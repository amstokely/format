#ifndef FORMAT_CST_NODE_HPP
#define FORMAT_CST_NODE_HPP

#include "kinds.hpp"
#include "unwrapped_line.hpp"

struct CSTNode {
    NodeKind kind = NodeKind::Unknown;
    NodeKind prev_kind = NodeKind::Unknown;
    const UnwrappedLine *line = nullptr;
};

#endif //FORMAT_CST_NODE_HPP