#include <ut.hpp>
#include "tokens.hpp"

using namespace boost::ut;
using namespace boost::ut::bdd;

int main() {
    "Tokens"_test = [] {

        given("an empty Tokens container") = [] {
            Tokens t;

            then("it is empty and has size 0") = [&] {
                expect(t.empty());
                expect(t.size() == 0_u);
            };

            then("first_token_is returns false") = [&] {
                expect(!t.first_token_is("foo"));
            };

            when("a token is added") = [&] {
                t.push_back(Token{TokenKind::Keyword, "program"});

                then("size becomes 1 and it is no longer empty") = [&] {
                    expect(!t.empty());
                    expect(t.size() == 1_u);
                };

                then("front and back both refer to the same token") = [&] {
                    expect(t.front().text == "program");
                    expect(t.back().text == "program");
                };

                then("first_token_is matches the first token") = [&] {
                    expect(t.first_token_is("program"));
                    expect(!t.first_token_is("module"));
                };

                then("first_token_is_any returns true if the first token matches one of the provided strings") = [&] {
                    using SV = std::vector<std::string>;
                    expect(t.first_token_is_any<SV>({"foo", "bar", "program"}));
                    expect(!t.first_token_is_any<SV>({"foo", "bar", "p"}));
                };

                when("a second token is added") = [&] {
                    t.push_back(Token{TokenKind::Keyword, "module"});

                    then("size becomes 2") = [&] {
                        expect(t.size() == 2_u);
                    };

                    then("operator[] returns the correct tokens") = [&] {
                        expect(t[0].text == "program");
                        expect(t[1].text == "module");
                    };

                    then("back returns the last token") = [&] {
                        expect(t.back().text == "module");
                    };

                    then("first_token_is still checks only the first position") = [&] {
                        expect(t.first_token_is("program"));
                        expect(!t.first_token_is("module"));
                    };

                    then("contains_token finds the provided text in any token") = [&] {
                        expect(t.contains_token("program"));
                        expect(t.contains_token("module"));
                        expect(!t.contains_token("subroutine"));
                    };

                    then("contains_token_sequence matches adjacent token sequences") = [&] {
                        using SV = std::vector<std::string>;

                        expect(t.contains_token_sequence(SV{"program"}));
                        expect(t.contains_token_sequence(SV{"module"}));
                        expect(t.contains_token_sequence(SV{"program", "module"}));

                        expect(!t.contains_token_sequence(SV{"module", "program"}));
                        expect(!t.contains_token_sequence(SV{"if", "then"}));
                        expect(!t.contains_token_sequence(SV{"program", "subroutine"}));
                    };

                    then("iteration yields tokens in the correct order") = [&] {
                        std::vector<std::string> texts;
                        for (auto &tk : t)
                            texts.push_back(std::string(tk.text));

                        expect(texts.size() == 2_u);
                        expect(texts[0] == "program");
                        expect(texts[1] == "module");
                    };
                };
            };
        };
    };
}
