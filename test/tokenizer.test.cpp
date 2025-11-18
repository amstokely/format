#include <ut.hpp>
#include "tokenizer.hpp"

using namespace boost::ut;
using namespace boost::ut::bdd;

// small helpers
auto has = [](auto kind, std::string_view text = {}) {
    return [=](const Token &t) {
        if (t.kind != kind) return false;
        return text.empty() ? true : t.text == text;
    };
};

auto exists = [](auto &&tokens, auto pred) {
    return std::ranges::any_of(tokens, pred);
};

auto count_tokens = [](auto &&tokens, auto pred) {
    return std::ranges::count_if(tokens, pred);
};

auto has_text = [](auto &&tokens, std::string_view text) {
    return std::ranges::any_of(tokens, [&](auto &x) { return x.text == text; });
};

struct TokenTestCase {
    std::string src;
    std::string text;
    TokenKind kind;
};

int main() {
    //
    // ------------------------------------------------------------
    // Keywords
    // ------------------------------------------------------------
    //
    "keywords"_test = [] {
        const std::vector<TokenTestCase> test_cases = {
            {"program end do implicit none integer", "program", TokenKind::Keyword},
            {"program end do implicit none integer", "end", TokenKind::Keyword},
            {"program end do implicit none integer", "do", TokenKind::Keyword},
            {"program end do implicit none integer", "implicit", TokenKind::Keyword},
            {"program end do implicit none integer", "none", TokenKind::Keyword},
            {"program end do implicit none integer", "integer", TokenKind::Keyword},
        };

        for (const auto &test_case: test_cases) {
            FortranTokenizer tz(test_case.src);
            const auto tokens = tz.tokenize();
            expect(exists(tokens, has(test_case.kind, test_case.text)));
        }
    };

    //
    // ------------------------------------------------------------
    // Identifiers and Numbers
    // ------------------------------------------------------------
    //

    "identifiers"_test = [] {
        const std::vector<TokenTestCase> test_cases = {
            {"foo bar123 x_9", "foo", TokenKind::Identifier}, {"foo bar123 x_9", "bar123", TokenKind::Identifier},
            {"foo bar123 x_9", "x_9", TokenKind::Identifier},
        };

        for (const auto &test_case: test_cases) {
            FortranTokenizer tz(test_case.src);
            const auto tokens = tz.tokenize();
            expect(exists(tokens, has(test_case.kind, test_case.text)));
        }
    };


    "numbers"_test = [] {
        const std::vector<TokenTestCase> unsigned_test_cases = {
            {"x = 42", "42", TokenKind::Number}, {"x = 3.14", "3.14", TokenKind::Number},
        };
        for (const auto &test_case: unsigned_test_cases) {
            FortranTokenizer tz(test_case.src);
            const auto tokens = tz.tokenize();
            expect(exists(tokens, has(test_case.kind, test_case.text)));
        }
        const std::vector<TokenTestCase> signed_test_cases = {
            {"x = +1", "+1", TokenKind::Number}, {"x = +1.1", "+1.1", TokenKind::Number},
            {"x = -2", "-2", TokenKind::Number}, {"x = -2.2", "-2.2", TokenKind::Number},
            {"x = - 3", "-3", TokenKind::Number}, {"x = - 3.3", "-3.3", TokenKind::Number},
            {"x =-4", "-4", TokenKind::Number}, {"x=-4.4", "-4.4", TokenKind::Number},
            {"x = 4 * (-5)", "-5", TokenKind::Number}, {"x= 4.0 * (-5.5)", "-5.5", TokenKind::Number},
            {"x = 4 * -6", "-6", TokenKind::Number}, {"x= 4.0 * -6.6", "-6.6", TokenKind::Number},
        };
        for (const auto &test_case: signed_test_cases) {
            FortranTokenizer tz(test_case.src);
            const auto tokens = tz.tokenize();
            expect(exists(tokens, has(test_case.kind, test_case.text)));
        }
    };
    //
    // ------------------------------------------------------------
    // Operators (+ - * / = **)
    // ------------------------------------------------------------
    //
    "operators"_test = [] {
        const std::vector<TokenTestCase> test_cases = {
            {"x = 1 + 2", "+", TokenKind::Operator},
             {"x = 1 - 2", "-",  TokenKind::Operator},
            {"x = 1 * 2", "*", TokenKind::Operator}, {"x = 1 / 2", "/", TokenKind::Operator},
            {"x = 1", "=", TokenKind::Operator}, {"x = 1 ** 2", "**", TokenKind::Operator},
        };

        for (const auto &tc: test_cases) {
            FortranTokenizer tz(tc.src);
            const auto tokens = tz.tokenize();
            expect(exists(tokens, has(tc.kind, tc.text)));
        }
    };


    //
    // ------------------------------------------------------------
    // Comments
    // ------------------------------------------------------------
    //
    "comments"_test = [] {
        std::string src = "! this is a comment\n";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::Comment, "! this is a comment")));
        expect(exists(t, has(TokenKind::Newline)));
    };

    //
    // ------------------------------------------------------------
    // Continuation Lines
    // ------------------------------------------------------------
    //
    "continuation line"_test = [] {
        std::string src = "x = a &\n& + b\n";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(count_tokens(t, has(TokenKind::Continuation)) == 2);
    };

    //
    // ------------------------------------------------------------
    // String Literals
    // ------------------------------------------------------------
    //
    "string literals"_test = [] {
        std::string src = "'hello' \"world\"";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::StringLiteral, "'hello'")));
        expect(exists(t, has(TokenKind::StringLiteral, "\"world\"")));
    };

    //
    // ------------------------------------------------------------
    // Whitespace + Newlines
    // ------------------------------------------------------------
    //
    "newlines and whitespace"_test = [] {
        std::string src = "a\n  b";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::Identifier, "a")));
        expect(exists(t, has(TokenKind::Newline)));
        expect(!exists(t, has(TokenKind::Whitespace, "  ")));
        expect(exists(t, has(TokenKind::Identifier, "b")));
    };

    //
    // ------------------------------------------------------------
    // Line/Column Positions
    // ------------------------------------------------------------
    //
    "line and column positions"_test = [] {
        std::string src = "a\n  b\nc";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        // find token "a"
        const auto it = std::ranges::find_if(t, has(TokenKind::Identifier, "a"));
        expect(it != t.end());
        expect(it->line == 1_i);
        expect(it->column == 1_i);

        // find token "b"
        const auto it2 = std::ranges::find_if(t, has(TokenKind::Identifier, "b"));
        expect(it2 != t.end());
        expect(it2->line == 2);
        expect(it2->column == 3);
    };

    //
    // ------------------------------------------------------------
    // Subroutine Support
    // ------------------------------------------------------------
    //
    "subroutine syntax"_test = [] {
        std::string src = R"(
subroutine foo(a, b)
    call foo(3)
end subroutine foo
)";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::Keyword, "subroutine")));
        expect(exists(t, has(TokenKind::Keyword, "call")));
        expect(exists(t, has(TokenKind::Keyword, "end")));
        expect(exists(t, has(TokenKind::Identifier, "foo")));
        expect(exists(t, has(TokenKind::Identifier, "a")));
        expect(exists(t, has(TokenKind::Identifier, "b")));
        expect(exists(t, has(TokenKind::Number, "3")));
    };

    //
    // ------------------------------------------------------------
    // IF / ELSE / END IF support
    // ------------------------------------------------------------
    //
    "if-else constructs"_test = [] {
        std::string src = R"(
if (a > b) then
    print *, a
else if (b > 0) then
    print *, b
end if
)";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::Keyword, "if")));
        expect(exists(t, has(TokenKind::Keyword, "then")));
        expect(exists(t, has(TokenKind::Keyword, "else")));

        // at least two "if" keywords ("if", "else if")
        expect(count_tokens(t, has(TokenKind::Keyword, "if")) >= 2);

        // end + if pair
        expect(exists(t, has(TokenKind::Keyword, "end")));

        // conditionals
        expect(exists(t, has(TokenKind::Identifier, "a")));
        expect(exists(t, has(TokenKind::Operator, ">")));
        expect(exists(t, has(TokenKind::Identifier, "b")));

        expect(exists(t, has(TokenKind::Number, "0")));
    };

    //
    // ------------------------------------------------------------
    // Full Program Parse (Integration Test)
    // ------------------------------------------------------------
    //
    "full sample program"_test = [] {
        const std::string src = R"(
program main
implicit none
integer i
do i = 1, 10
    print *, i+1
end do
end program
)";
        FortranTokenizer tz(src);
        auto t = tz.tokenize();

        expect(exists(t, has(TokenKind::Keyword, "program")));
        expect(exists(t, has(TokenKind::Keyword, "implicit")));
        expect(exists(t, has(TokenKind::Keyword, "integer")));
        expect(exists(t, has(TokenKind::Keyword, "print")));
        expect(exists(t, has(TokenKind::Identifier, "i")));
        expect(exists(t, has(TokenKind::Number, "1")));
        expect(count_tokens(t, has(TokenKind::Keyword, "end")) >= 2);
    };
}
