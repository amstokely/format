#ifndef FORMAT_UNWRAPPED_LINE_HPP
#define FORMAT_UNWRAPPED_LINE_HPP

#include "tokenizer.hpp"
#include "tokens.hpp"
#include <vector>
#include <ranges>


struct UnwrappedLine {
    Tokens tokens;
};

class UnwrappedLineParser {
public:
    explicit UnwrappedLineParser(const std::vector<Token> &tokens)
        : m_tokens(tokens) {
    }

    [[nodiscard]] std::vector<UnwrappedLine> parse() const {
        std::vector<UnwrappedLine> lines;
        lines.emplace_back();

        const std::size_t num_tokens = m_tokens.size();
        if (num_tokens == 0) return lines;
        if (num_tokens == 1) {
            lines.back().tokens.push_back(m_tokens[0]);
            return lines;
        }


        auto head = m_tokens | std::views::take(num_tokens - 1);
        auto tail = m_tokens | std::views::drop(1);

        bool skip_next_newline = false;

        for (auto [cur, next]: std::views::zip(head, tail)) {
            if (skip_next_newline) {
                if (cur.kind == TokenKind::Newline) {
                    skip_next_newline = false;
                    continue;
                }
                skip_next_newline = false;
            }

            if (is_continuation_pair(cur, next)) {
                lines.back().tokens.push_back(cur);
                skip_next_newline = true;
                continue;
            }
            if (cur.kind != TokenKind::Whitespace) lines.back().tokens.push_back(cur);

            if (cur.kind == TokenKind::Newline) {
                lines.emplace_back();
            }
        }
        return lines;
    }

private:
    [[nodiscard]] static bool is_continuation_pair(const Token &a, const Token &b) noexcept {
        return a.kind == TokenKind::Continuation && b.kind == TokenKind::Newline;
    }

private:
    const std::vector<Token> &m_tokens;
};

#endif // FORMAT_UNWRAPPED_LINE_HPP
