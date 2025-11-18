#ifndef FORMAT_TOKENS_HPP
#define FORMAT_TOKENS_HPP
#include "tokenizer.hpp"
#include <ranges>

class Tokens {
    std::vector<Token> m_data;

public:
    // Modifiers
    void push_back(const Token &token) noexcept {
        m_data.push_back(token);
    }

    // Iteration
    auto begin() noexcept { return m_data.begin(); }
    auto end() noexcept { return m_data.end(); }
    [[nodiscard]] auto begin() const noexcept { return m_data.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_data.end(); }

    // Element access: operator[]
    Token &operator[](std::size_t i) noexcept {
        return m_data[i];
    }

    const Token &operator[](std::size_t i) const noexcept {
        return m_data[i];
    }

    // Element access: front
    Token &front() noexcept { return m_data.front(); }
    [[nodiscard]] const Token &front() const noexcept { return m_data.front(); }

    // Element access: back
    Token &back() noexcept { return m_data.back(); }
    [[nodiscard]] const Token &back() const noexcept { return m_data.back(); }

    [[nodiscard]] size_t size() const noexcept { return m_data.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

    // First token equals exactly one string
    [[nodiscard]] bool first_token_is(const std::string &text) const noexcept {
        return !m_data.empty() && m_data[0].text == text;
    }

    // First token equals any of a set of provided strings
    template<typename Range>
    [[nodiscard]] bool first_token_is_any(const Range &texts) const noexcept {
        if (m_data.empty()) return false;

        const auto &first = m_data[0].text;

        return std::ranges::any_of(texts, [&](const auto &s) {
            return first == s;
        });
    }

    // Check if any token matches the provided text
    [[nodiscard]] bool contains_token(const std::string &text) const noexcept {
        for (const auto &t : m_data) {
            if (t.text == text)
                return true;
        }
        return false;
    }

    // ----------------------------------------------------------------------
    // NEW: contains_token_sequence
    // Checks for an adjacent ordered sequence of token text values
    // ----------------------------------------------------------------------
    template<typename Range>
    [[nodiscard]] bool contains_token_sequence(const Range &seq) const noexcept {
        const std::size_t n = m_data.size();
        const std::size_t m = std::size(seq);

        if (m == 0 || m > n)
            return false;

        // Slide a window of length m across the tokens
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
