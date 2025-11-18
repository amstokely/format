#include <ut.hpp>
#include "tokenizer.hpp"
#include "unwrapped_line.hpp"
#include "cst.hpp"
#include "cst_visitor.hpp"   // BlockTreeBuilder

using namespace boost::ut;
using bdd::given;
using bdd::when;
using bdd::then;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static std::vector<UnwrappedLine> unwrap(const std::string &src) {
    FortranTokenizer tz(src);
    const auto tokens = tz.tokenize();
    const UnwrappedLineParser parser(tokens);
    return parser.parse();
}

static std::vector<CSTNode>
build_cst_with(BlockTreeBuilder &visitor, const std::vector<UnwrappedLine> &lines)
{
    return build_cst(lines, &visitor);
}

// -----------------------------------------------------------------------------
// TEST SUITE
// -----------------------------------------------------------------------------

int main() {

    // =========================================================================
    // 1. FIRST BLOCK GOES INTO ROOT->BEGIN_NODE
    // =========================================================================
    "block tree: first block stored in root begin/end"_test = [] {
        given("a program with a subroutine") = [] {
            const std::string src =
                "program foo\n"
                "contains\n"
                "subroutine foo(a,b)\n"
                "integer :: a, b\n"
                "end subroutine foo\n"
                "end program foo\n";

            const auto lines = unwrap(src);
            BlockTreeBuilder visitor;

            when("the CST is built") = [&] {
                build_cst_with(visitor, lines);
            };
            then("The root holds the program block.") = [&] {
                expect(visitor.root->begin_node->kind == NodeKind::Program);
                expect(visitor.root->end_node->kind   == NodeKind::EndProgram);
            };

            then("The first root child is the foo subroutine block.") = [&] {
                expect(visitor.root->children.at(0)->begin_node->kind == NodeKind::Subroutine);
                expect(visitor.root->children.at(0)->end_node->kind   == NodeKind::EndSubroutine);
            };

            then("root has one child") = [&] {
                expect(visitor.root->children.size() == 1_ul);
            };
        };
    };


    // =========================================================================
    // 2. NESTED BLOCKS: program → subroutine → if
    // =========================================================================
    "block tree: nested blocks"_test = [] {
        given("a subroutine containing an if-construct") = [] {
            const std::string src =
                "subroutine bar\n"
                "integer :: x = 5\n"
                "if (x == 5) then\n"
                "x = x + 1\n"
                "end if\n"
                "end subroutine bar\n";

            const auto lines = unwrap(src);
            BlockTreeBuilder visitor;

            when("the CST is built") = [&] {
                build_cst_with(visitor, lines);
            };

            then("root has the first block: Subroutine") = [&] {
                expect(visitor.root->begin_node->kind == NodeKind::Subroutine);
                expect(visitor.root->end_node->kind   == NodeKind::EndSubroutine);
            };

            then("Subroutine has one child: the IfConstruct") = [&] {
                auto *SUB = visitor.root.get();
                expect(SUB->children.size() == 1_ul);

                auto *IF = SUB->children[0].get();
                expect(IF->begin_node->kind == NodeKind::IfConstruct);
                expect(IF->end_node->kind   == NodeKind::EndIf);
            };
        };
    };


    // =========================================================================
    // 3. SIBLING BLOCKS: subroutine a, then subroutine b
    // =========================================================================
    "block tree: sibling blocks"_test = [] {
        given("two subroutines in sequence") = [] {
            const std::string src =
                "subroutine a\n"
                "end subroutine a\n"
                "subroutine b\n"
                "end subroutine b\n";

            const auto lines = unwrap(src);
            BlockTreeBuilder visitor;

            when("CST is built") = [&] {
                build_cst_with(visitor, lines);
            };

            then("root begin/end = first subroutine") = [&] {
                expect(visitor.root->begin_node->kind == NodeKind::Subroutine);
                expect(visitor.root->end_node->kind   == NodeKind::EndSubroutine);
            };

            then("second subroutine is in root->children[0]") = [&] {
                expect(visitor.root->children.size() == 1_ul);

                auto *S2 = visitor.root->children[0].get();
                expect(S2->begin_node->kind == NodeKind::Subroutine);
                expect(S2->end_node->kind   == NodeKind::EndSubroutine);
            };
        };
    };


    // =========================================================================
    // 4. DEEP NESTING: if → do → select case
    // =========================================================================
    "block tree: deep nesting"_test = [] {
        given("if/do/select nesting") = [] {
            const std::string src =
                "if (a == b) then\n"
                "  do i = 1, 10\n"
                "    select case (i)\n"
                "    case (1)\n"
                "    a = 2\n"
                "    end select\n"
                "  end do\n"
                "end if\n";

            const auto lines = unwrap(src);
            BlockTreeBuilder visitor;

            when("the CST is built") = [&] {
                build_cst_with(visitor, lines);
            };

            then("root begin/end is IfConstruct") = [&] {
                expect(visitor.root->begin_node->kind == NodeKind::IfConstruct);
                expect(visitor.root->end_node->kind   == NodeKind::EndIf);
            };

            then("IfConstruct has one child: Do") = [&] {
                auto *IF = visitor.root.get();
                expect(IF->children.size() == 1_ul);

                auto *DO = IF->children[0].get();
                expect(DO->begin_node->kind == NodeKind::Do);
                expect(DO->end_node->kind   == NodeKind::EndDo);
            };

            then("Do has one child: SelectCase") = [&] {
                auto *DO = visitor.root->children[0].get();
                expect(DO->children.size() == 1_ul);

                auto *SEL = DO->children[0].get();
                expect(SEL->begin_node->kind == NodeKind::SelectCase);
                expect(SEL->end_node->kind   == NodeKind::EndSelect);
            };
        };
    };


    // =========================================================================
    // 5. TYPE BLOCK
    // =========================================================================
    "block tree: type block"_test = [] {
        given("a type definition") = [] {
            const std::string src =
                "type :: my_type\n"
                "integer :: x\n"
                "end type my_type\n";

            const auto lines = unwrap(src);
            BlockTreeBuilder visitor;

            when("CST is built") = [&] {
                build_cst_with(visitor, lines);
            };

            then("root begin/end is Type block") = [&] {
                expect(visitor.root->begin_node->kind == NodeKind::Type);
                expect(visitor.root->end_node->kind   == NodeKind::EndType);
            };

            then("type has no children") = [&] {
                expect(visitor.root->children.size() == 0_ul);
            };
        };
    };

    return 0;
}

