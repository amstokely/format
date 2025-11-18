#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <cctype>
#include <algorithm>

// ============================================================
// Token Definitions
// ============================================================

enum class TokenKind : uint8_t {
    Identifier,
    Keyword,
    Number,
    Operator,
    Comma,
    Colon,
    Semicolon,
    LParen,
    RParen,
    Percent,
    StringLiteral,
    Comment,
    Continuation,
    Whitespace,
    Newline,
    EndOfFile,
    Unknown
};

struct Token {
    TokenKind kind;
    std::string text;
    int line;
    int column;
};

// ============================================================
// FAST ASCII HELPERS
// ============================================================

inline static bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

inline static bool is_alpha(char c) noexcept {
    c |= 32; // force lowercase bit
    return c >= 'a' && c <= 'z';
}

inline static bool is_alnum_or_underscore(char c) noexcept {
    return is_alpha(c) || is_digit(c) || c == '_';
}

inline static bool is_space(char c) noexcept {
    return c == ' ' || c == '\t';
}


// ============================================================
// Fortran Tokenizer
// ============================================================

class FortranTokenizer {
public:
    explicit FortranTokenizer(std::string_view src)
        : m_source(src), m_pos(0), m_line(1), m_col(1) {}

    [[nodiscard]] std::vector<Token> tokenize() {
        std::vector<Token> out;
        out.reserve(m_source.size() / 4);

        while (true) {
            Token t = next_token();

            if (t.kind != TokenKind::Whitespace) {
                if (is_unary_sign_merge(out, t)) {
                    Token& sign = out.back();  // "+" or "-"

                    sign.kind = TokenKind::Number;
                    sign.text.reserve(sign.text.size() + t.text.size());
                    sign.text += t.text;       // merge "+1"
                } else {
                    out.push_back(std::move(t));
                }
            }

            m_prev_kind = t.kind;
            m_tokens_empty = false;

            if (t.kind == TokenKind::EndOfFile)
                break;
        }

        return out;
    }

private:
    std::string_view m_source;
    size_t m_pos;
    int m_line, m_col;
    TokenKind m_prev_kind = TokenKind::Unknown;
    bool m_tokens_empty = true;

    // ============================================================
    // BASIC CHAR ACCESS
    // ============================================================

    [[nodiscard]] char peek() const noexcept {
        return (m_pos < m_source.size()) ? m_source[m_pos] : '\0';
    }

    char get() noexcept {
        char c = peek();
        if (c != '\0') {
            ++m_pos;
            ++m_col;
        }
        return c;
    }

    Token make(TokenKind k, int line, int col, size_t start, size_t len) const {
        return Token{k, std::string(m_source.data() + start, len), line, col};
    }


    // ============================================================
    // UNARY SIGN MERGE LOGIC
    // ============================================================

    inline static bool is_unary_sign_merge(
        const std::vector<Token>& toks,
        const Token& current)
    {
        if (current.kind != TokenKind::Number)
            return false;

        if (toks.size() < 2)
            return false;

        const Token& sign = toks[toks.size() - 1];
        const Token& prev = toks[toks.size() - 2];

        bool sign_ok =
            sign.kind == TokenKind::Operator &&
            sign.text.size() == 1 &&
            (sign.text[0] == '+' || sign.text[0] == '-');

        if (!sign_ok) return false;

        // avoid merging 1 - -1 → incorrect
        if (prev.kind == TokenKind::Number || prev.kind == TokenKind::Identifier)
            return false;

        return true;
    }


    // ============================================================
    // TOKEN DISPATCH
    // ============================================================

    Token next_token() {
        int line = m_line;
        int col  = m_col;
        char c   = peek();

        if (c == '\0')
            return {TokenKind::EndOfFile, "", line, col};

        if (is_space(c))    return lex_whitespace(line, col);
        if (c == '\n')      return lex_newline(line, col);
        if (c == '!')       return lex_comment(line, col);
        if (c == '&')       return lex_continuation(line, col);
        if (c == '\'' || c == '"') return lex_string_literal(line, col);
        if (is_alpha(c))    return lex_identifier_or_keyword(line, col);
        if (is_digit(c))    return lex_number(line, col);

        if (auto t = lex_punctuation(line, col); t.kind != TokenKind::Unknown)
            return t;

        if (auto t = lex_operator(line, col); t.kind != TokenKind::Unknown)
            return t;

        return lex_unknown(line, col);
    }


    // ============================================================
    // TOKEN LEXERS
    // ============================================================

    Token lex_whitespace(int line, int col) {
        size_t start = m_pos;
        while (is_space(peek())) get();
        return make(TokenKind::Whitespace, line, col, start, m_pos - start);
    }

    Token lex_newline(int line, int col) {
        get();
        ++m_line; m_col = 1;
        return {TokenKind::Newline, "\n", line, col};
    }

    Token lex_comment(int line, int col) {
        size_t start = m_pos;
        while (peek() != '\n' && peek() != '\0') get();
        return make(TokenKind::Comment, line, col, start, m_pos - start);
    }

    Token lex_continuation(int line, int col) {
        get();
        return {TokenKind::Continuation, "&", line, col};
    }

    Token lex_string_literal(int line, int col) {
        char quote = get();
        size_t start = m_pos - 1;

        while (peek() != '\0') {
            if (get() == quote) break;
        }

        return make(TokenKind::StringLiteral, line, col, start, m_pos - start);
    }

    Token lex_identifier_or_keyword(int line, int col) {
        size_t start = m_pos;

        while (is_alnum_or_underscore(peek()))
            get();

        std::string_view v(m_source.data() + start, m_pos - start);

        if (is_keyword(v))
            return {TokenKind::Keyword, std::string(v), line, col};

        return {TokenKind::Identifier, std::string(v), line, col};
    }

    Token lex_number(int line, int col) {
        size_t start = m_pos;

        if (peek() == '+' || peek() == '-') get();

        // integer part
        while (is_digit(peek())) get();

        // decimal part
        if (peek() == '.') {
            get();
            while (is_digit(peek())) get();
        }

        // exponent
        char e = peek();
        if (e == 'e' || e == 'E' || e == 'd' || e == 'D') {
            get();
            if (peek() == '+' || peek() == '-') get();
            while (is_digit(peek())) get();
        }

        return make(TokenKind::Number, line, col, start, m_pos - start);
    }

    Token lex_punctuation(int line, int col) {
        char c = peek();
        switch (c) {
            case ',': get(); return {TokenKind::Comma, ",", line, col};
            case ':': get(); return {TokenKind::Colon, ":", line, col};
            case ';': get(); return {TokenKind::Semicolon, ";", line, col};
            case '(': get(); return {TokenKind::LParen, "(", line, col};
            case ')': get(); return {TokenKind::RParen, ")", line, col};
            case '%': get(); return {TokenKind::Percent, "%", line, col};
        }
        return {TokenKind::Unknown, "", line, col};
    }

    Token lex_operator(int line, int col) {
        static constexpr std::array two_ops{
            ">=", "<=", "/=", "==", "**"
        };

        if (m_pos + 1 < m_source.size()) {
            std::string_view two(m_source.data() + m_pos, 2);
            for (auto op : two_ops) {
                if (two == op) {
                    m_pos += 2;
                    m_col += 2;
                    return {TokenKind::Operator, std::string(op), line, col};
                }
            }
        }

        char c = peek();
        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '<' || c == '>') {
            get();
            return {TokenKind::Operator, std::string(1, c), line, col};
        }

        return {TokenKind::Unknown, "", line, col};
    }

    Token lex_unknown(int line, int col) {
        char c = get();
        return {TokenKind::Unknown, std::string(1, c), line, col};
    }


    // ============================================================
    // Keyword Matcher
    // ============================================================

    static bool is_keyword(std::string_view s) noexcept {
        static constexpr std::array keywords{
            "program","end","contains","module","end module","abstract",
            "abstract interface","interface","end interface","subroutine",
            "end subroutine","call","function","end function","select",
            "select case","end select","case","do","enddo","end do","if",
            "then","else","else if","endif","end if","use","print","implicit",
            "none","integer","real","double","precision","logical", "recursive",
            "type", "pure"
        };

        std::string lower;
        lower.reserve(s.size());
        for (char c : s) lower.push_back(std::tolower((unsigned char)c));

        return std::find(keywords.begin(), keywords.end(),
                         std::string_view(lower)) != keywords.end();
    }
};
