#ifndef FORMAT_CST_HPP
#define FORMAT_CST_HPP

#include <vector>
#include <string_view>
#include <algorithm>

#include "unwrapped_line.hpp"
#include "cst_visitor.hpp"

using StringVector = std::vector<std::string>;

// ============================================================================
// NodeKind: Classification of lines for CST building
// ============================================================================


inline bool starts_with_keyword(const UnwrappedLine &line, std::string_view kw) {
    return !line.tokens.empty() && line.tokens[0].kind == TokenKind::Keyword && line.tokens[0].text == kw;
}

inline bool has_second_keyword(const UnwrappedLine &line, std::string_view kw) {
    return line.tokens.size() > 1 && line.tokens[1].kind == TokenKind::Keyword && line.tokens[1].text == kw;
}

inline bool is_declaration_type_keyword(std::string_view text) {
    static constexpr std::string_view kinds[] = {"integer", "real", "logical", "double"};
    return std::ranges::any_of(kinds, [&](std::string_view k) { return k == text; });
}

inline bool is_fortran_declaration(const UnwrappedLine &line) {
    return !line.tokens.empty() && line.tokens[0].kind == TokenKind::Keyword && is_declaration_type_keyword(
               line.tokens[0].text);
}

inline bool is_assignment(const UnwrappedLine &line) {
    return line.tokens.contains_token("=");
}

// ============================================================================
// TYPE / CLASS construct detection
// ============================================================================

inline bool is_type_construct(const UnwrappedLine &line) {
    if (line.tokens.empty()) return false;
    if (line.tokens.contains_token("type") && !line.tokens.contains_token_sequence<StringVector>({"type", "("})) {
        return true;
    }
    return false;
}

// ============================================================================
// END <construct> detection
// ============================================================================

inline NodeKind classify_end_construct(const UnwrappedLine &line) {
    if (line.tokens.empty()) return NodeKind::Unknown;

    // Simple forms: endif, enddo
    if (line.tokens.first_token_is("endif")) return NodeKind::EndIf;
    if (line.tokens.first_token_is("enddo")) return NodeKind::EndDo;

    // All multiword forms begin with "end"
    if (!line.tokens.first_token_is("end")) return NodeKind::Unknown;

    if (has_second_keyword(line, "program")) return NodeKind::EndProgram;
    if (has_second_keyword(line, "module")) return NodeKind::EndModule;
    if (has_second_keyword(line, "subroutine")) return NodeKind::EndSubroutine;
    if (has_second_keyword(line, "function")) return NodeKind::EndFunction;
    if (has_second_keyword(line, "interface")) return NodeKind::EndInterface;
    if (has_second_keyword(line, "select")) return NodeKind::EndSelect;
    if (has_second_keyword(line, "do")) return NodeKind::EndDo;
    if (has_second_keyword(line, "if")) return NodeKind::EndIf;
    if (has_second_keyword(line, "type")) return NodeKind::EndType;

    return NodeKind::Unknown;
}

// ============================================================================
// classify() – central logic
// ============================================================================

inline NodeKind classify(const UnwrappedLine &line) {
    using K = TokenKind;

    if (line.tokens.empty()) return NodeKind::Blank;

    const Token &t0 = line.tokens.front();

    // Comments
    if (t0.kind == K::Comment) return NodeKind::Comment;

    // END <construct>
    if (starts_with_keyword(line, "end") || starts_with_keyword(line, "endif") || starts_with_keyword(line, "enddo")) {
        return classify_end_construct(line);
    }

    // module procedure special case
    if (line.tokens.contains_token_sequence<StringVector>({"module", "procedure"})) return NodeKind::Declaration;

    // TYPE constructs
    if (is_type_construct(line)) return NodeKind::Type;

    // Keyword-driven constructs
    if (t0.kind == K::Keyword) {
        // abstract interface
        if (t0.text == "abstract" && has_second_keyword(line, "interface")) return NodeKind::Interface;

        if (t0.text == "program") return NodeKind::Program;
        if (t0.text == "module") return NodeKind::Module;
        if (t0.text == "use") return NodeKind::Use;
        if (t0.text == "call") return NodeKind::Call;
        if (t0.text == "select") return NodeKind::SelectCase;
        if (t0.text == "case") return NodeKind::Case;
        if (t0.text == "interface") return NodeKind::Interface;
        if (t0.text == "do") return NodeKind::Do;

        // print behaves like CALL for formatting
        if (t0.text == "print") return NodeKind::Call;

        // FUNCTION / SUBROUTINE anywhere in line
        if (line.tokens.contains_token("function")) return NodeKind::Function;

        if (line.tokens.contains_token("subroutine")) return NodeKind::Subroutine;

        // IF / IF THEN
        if (t0.text == "if") {
            return line.tokens.contains_token("then") ? NodeKind::IfConstruct : NodeKind::If;
        }

        // ELSE / ELSE IF
        if (t0.text == "else") {
            if (line.tokens.size() > 1 && line.tokens[1].text == "if") return NodeKind::ElseIf;
            return NodeKind::Else;
        }
    }

    // Declarations
    if (is_fortran_declaration(line)) return NodeKind::Declaration;

    // Assignments
    if (is_assignment(line)) return NodeKind::Assignment;

    return NodeKind::Unknown;
}

// ============================================================================
// build_cst()
// ============================================================================

inline std::vector<CSTNode>
build_cst(const std::vector<UnwrappedLine> &lines,
          CSTVisitor* visitor = nullptr)
{
    std::vector<CSTNode> ast;
    ast.reserve(lines.size());

    NodeKind last_real = NodeKind::Unknown;

    for (const auto &line: lines) {
        CSTNode node;
        node.line = &line;
        node.kind = classify(line);
        node.prev_kind = last_real;

        if (node.kind != NodeKind::Blank &&
            node.kind != NodeKind::Unknown)
            last_real = node.kind;

        if (visitor) visitor->on_node(node);

        ast.push_back(node);
    }

    return ast;
}

#endif // FORMAT_CST_HPP
