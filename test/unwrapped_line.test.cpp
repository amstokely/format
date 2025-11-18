#include <ut.hpp>
#include "tokenizer.hpp"
#include "unwrapped_line.hpp"
#include <array>
#include <format>
#include <iostream>
#include <ranges>
#include <tuple>


using namespace boost::ut;
using namespace boost::ut::bdd;

int main() {
    // ------------------------------------------------------------
    // Helper: tokenize & parse
    // ------------------------------------------------------------
    auto parse = [](const std::string_view src) {
        FortranTokenizer tz(src);
        const auto tokens = tz.tokenize();
        const UnwrappedLineParser parser(tokens);
        return parser.parse();
    };

    // ------------------------------------------------------------
    // Multiple simple statements
    // ------------------------------------------------------------
    "multiple simple statements"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "x=1" << std::endl;
        src_stream << "y=2" << std::endl;
        src_stream << "z=3" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 4_i);
    };
    "statement with inline comment"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "x=1 !x equals 1" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 2_i);
    };

    "statement_with_line_break"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "subroutine foo(&" << std::endl;
        src_stream << " a, b)" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 2_i);
    };
    "statement_with_space_line_break"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "subroutine foo(& " << std::endl;
        src_stream << " a, b)" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 2_i);
    };
    "statement_with_multiple_line_breaks"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "subroutine foo(&" << std::endl;
        src_stream << "    a, b &" << std::endl;
        src_stream << ")" << std::endl;
        src_stream << "end subroutine" << std::endl;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 3_i);
    };
    "statement_with_single_token"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "program";;
        const auto src = src_stream.str();
        const auto lines = parse(src);
        expect(lines.size() == 1_i);
    };
    "empty_statement"_test = [parse] {
        std::stringstream src_stream;
        src_stream << "";;
        constexpr std::vector<Token> tokens;
        const UnwrappedLineParser parser(tokens);
        const auto lines = parser.parse();
        expect(lines.size() == 1_i);
    };
};
