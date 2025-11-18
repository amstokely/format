#include <ut.hpp>
#include "tokenizer.hpp"
#include "unwrapped_line.hpp"
#include <iostream>
#include <ranges>
#include "cst.hpp"


using namespace boost::ut;
using namespace boost::ut::bdd;

int main() {
    // ------------------------------------------------------------
    // Helper: tokenize & parse
    // ------------------------------------------------------------
    auto parse = [](const std::string_view src) {
        FortranTokenizer tz(src);
        auto tokens = tz.tokenize();
        UnwrappedLineParser parser(tokens);
        return parser.parse();
    };

    const auto get_node = [](const int node_index, const std::vector<CSTNode>& cst) {
        return cst.at(node_index);
    };

    // ------------------------------------------------------------
    // Multiple simple statements
    // ------------------------------------------------------------
    "assignment"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "x=1" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(cst.front().kind == NodeKind::Assignment);
    };
    "declaration"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "integer :: x" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(cst.front().kind == NodeKind::Declaration);
    };
    "program"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "program main" << std::endl;
        src_stream << "end program" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Program);
        expect(get_node(1, cst).kind == NodeKind::EndProgram);
    };
    "module"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "module main" << std::endl;
        src_stream << "end module" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Module);
        expect(get_node(1, cst).kind == NodeKind::EndModule);
    };
    "function"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "function main" << std::endl;
        src_stream << "end function" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Function);
        expect(get_node(1, cst).kind == NodeKind::EndFunction);
    };
    "type function"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "integer function main" << std::endl;
        src_stream << "end function" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Function);
        expect(get_node(1, cst).kind == NodeKind::EndFunction);
    };
    "pure function"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "pure function main" << std::endl;
        src_stream << "end function" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Function);
        expect(get_node(1, cst).kind == NodeKind::EndFunction);
    };
    "pure type function"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "pure integer function main" << std::endl;
        src_stream << "end function" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Function);
        expect(get_node(1, cst).kind == NodeKind::EndFunction);
    };
    "subroutine"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "subroutine main" << std::endl;
        src_stream << "end subroutine" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Subroutine);
        expect(get_node(1, cst).kind == NodeKind::EndSubroutine);
    };
    "recursive subroutine"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "recursive subroutine main" << std::endl;
        src_stream << "end subroutine" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Subroutine);
        expect(get_node(1, cst).kind == NodeKind::EndSubroutine);
    };
    "simple type"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "type :: foo" << std::endl;
        src_stream << "end type" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Type);
        expect(get_node(1, cst).kind == NodeKind::EndType);
    };
    "bind c type"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "type, bind(C) :: foo" << std::endl;
        src_stream << "end type" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Type);
        expect(get_node(1, cst).kind == NodeKind::EndType);
    };
    "abstract type"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "type, abstract :: foo" << std::endl;
        src_stream << "end type" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Type);
        expect(get_node(1, cst).kind == NodeKind::EndType);
    };
    "child type"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "type, extends(bar) :: foo" << std::endl;
        src_stream << "end type" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Type);
        expect(get_node(1, cst).kind == NodeKind::EndType);
    };
    "type argument"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "type(foo) :: bar" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind != NodeKind::Type);
    };
    "interface"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "interface main" << std::endl;
        src_stream << "end interface" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Interface);
        expect(get_node(1, cst).kind == NodeKind::EndInterface);
    };
    "abstract interface"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "abstract interface main" << std::endl;
        src_stream << "end interface" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Interface);
        expect(get_node(1, cst).kind == NodeKind::EndInterface);
    };
    "use"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "use iso_c_bindings" << std::endl;
        src_stream << "use iso_c_bindings, only: c_ptr" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Use);
        expect(get_node(1, cst).kind == NodeKind::Use);
    };
    "call"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "call foo()" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Call);
    };
    "if"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "if (x > 5) then" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::IfConstruct);
    };
    "end if"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "end if" << std::endl;
        src_stream << "endif" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::EndIf);
        expect(get_node(1, cst).kind == NodeKind::EndIf);
    };
    "else if"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "else if" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::ElseIf);
    };
    "else"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "else" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Else);
    };
    "endif"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "endif" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::EndIf);
    };
    "do"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "do i, 5" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Do);
    };
    "end do"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "end do" << std::endl;
        src_stream << "enddo" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::EndDo);
        expect(get_node(1, cst).kind == NodeKind::EndDo);
    };
    "select case"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "select case(expr)" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::SelectCase);
    };
    "case"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "case (5)" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Case);
    };
    "end select"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "end select" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::EndSelect);
    };
    "comment"_test = [parse, get_node] {
        std::stringstream src_stream;
        src_stream << "! This is a comment" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        const auto cst = build_cst(lines);
        expect(get_node(0, cst).kind == NodeKind::Comment);
    };
};