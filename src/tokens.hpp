#ifndef FORMAT_TOKENS_HPP
#define FORMAT_TOKENS_HPP
#include "tokenizer.hpp"
#include <ranges>

class Tokens {
    std::vector<Token> m_data;

public:
    Tokens() = default;
    void push_back(const Token &token) noexcept {
        m_data.push_back(token);
    }

    auto begin() noexcept { return m_data.begin(); }
    auto end() noexcept { return m_data.end(); }
    [[nodiscard]] auto begin() const noexcept { return m_data.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_data.end(); }

    Token &operator[](std::size_t i) noexcept {
        return m_data[i];
    }

    const Token &operator[](std::size_t i) const noexcept {
        return m_data[i];
    }

    Token &front() noexcept { return m_data.front(); }
    [[nodiscard]] const Token &front() const noexcept { return m_data.front(); }

    Token &back() noexcept { return m_data.back(); }
    [[nodiscard]] const Token &back() const noexcept { return m_data.back(); }

    [[nodiscard]] size_t size() const noexcept { return m_data.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

    [[nodiscard]] bool first_token_is(const std::string &text) const noexcept {
        return !m_data.empty() && m_data[0].text == text;
    }

    template<typename Range>
    [[nodiscard]] bool first_token_is_any(const Range &texts) const noexcept {
        if (m_data.empty()) return false;

        const auto &first = m_data[0].text;

        return std::ranges::any_of(texts, [&](const auto &s) {
            return first == s;
        });
    }

    [[nodiscard]] bool contains_token(const std::string &text) const noexcept {
        return std::ranges::any_of(m_data, [&](const auto &s) {
            if (s.text == text) return true;
            return false;
        });
    }

    template<typename Range>
    [[nodiscard]] bool contains_token_sequence(const Range &seq) const noexcept {
        const std::size_t n = m_data.size();
        const std::size_t m = std::size(seq);

        if (m == 0 || m > n)
            return false;

        for (std::size_t i = 0; i <= n - m; ++i) {
            bool match = true;

            for (std::size_t j = 0; j < m; ++j) {
                if (m_data[i + j].text != seq[j]) {
                    match = false;
                    break;
                }
            }

            if (match)
                return true;
        }

        return false;
    }
};

#endif // FORMAT_TOKENS_HPP
