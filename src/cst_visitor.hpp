#ifndef FORMAT_CST_VISITOR_HPP
#define FORMAT_CST_VISITOR_HPP
#include <iostream>

#include "cst_node.hpp"
#include "kinds.hpp"
#include <memory>
#include <vector>


struct CSTVisitor {
    virtual ~CSTVisitor() = default;
    virtual void on_enter(const CSTNode& node) {}
    virtual void on_exit(const CSTNode& node) {}
    virtual void on_node(const CSTNode& node) {}
};

struct BlockNode {
    std::shared_ptr<CSTNode> begin_node;
    std::shared_ptr<CSTNode> end_node;
    BlockNode* parent{nullptr};
    std::vector<std::unique_ptr<BlockNode>> children;
};

struct BlockTreeBuilder : public CSTVisitor {
    std::unique_ptr<BlockNode> root = std::make_unique<BlockNode>();
    BlockNode* current = nullptr;

    BlockTreeBuilder() { current = root.get(); }

    static bool begins_block(NodeKind k) {
        using NK = NodeKind;
        return (k == NK::Program || k == NK::Module || k == NK::Subroutine ||
                k == NK::Function || k == NK::Interface || k == NK::IfConstruct ||
                k == NK::Do || k == NK::SelectCase || k == NK::Type);
    }

    static bool ends_block(NodeKind k) {
        return k == NodeKind::EndProgram ||
               k == NodeKind::EndModule ||
               k == NodeKind::EndSubroutine ||
               k == NodeKind::EndFunction ||
               k == NodeKind::EndInterface ||
               k == NodeKind::EndIf ||
               k == NodeKind::EndDo ||
               k == NodeKind::EndSelect ||
               k == NodeKind::EndType;
    }

    void on_node(const CSTNode& node) override {
        using NK = NodeKind;

        // -------- begin block --------
        if (begins_block(node.kind)) {
            if (!current->begin_node) {
                current->begin_node = std::make_shared<CSTNode>(node);
            } else {
                auto child = std::make_unique<BlockNode>();
                child->parent = current;
                child->begin_node = std::make_shared<CSTNode>(node);
                auto num_children = current->children.size();
                current->children.push_back(std::move(child));
                current = current->children.back().get();
            }
            on_enter(node);
        }

        // -------- end block --------
        else if (ends_block(node.kind)) {
            if (!current->end_node) {
                current->end_node = std::make_shared<CSTNode>(node);
            } else {
                // end of nested block; bubble back
                if (current->parent) {
                    current = current->parent;
                    current->end_node = std::make_shared<CSTNode>(node);
                }
            }
            on_exit(node);
        }
    }
};



#endif //FORMAT_CST_VISITOR_HPP
