#pragma once

#include "datastructure/banana_tree.h"
#include "persistence_defs.h"

namespace bananas {

template<int sign, typename Visitor>
    requires sign_integral<decltype(sign), sign>
void map_in_trail(const banana_tree_node<sign>* max_node, Visitor visitor) {
    massert(max_node->is_internal(), "Expected to start iterating trails at a maximum.");
    auto* in = max_node->get_in();
    const auto* birth = max_node->get_birth();
    while (in != birth) {
        visitor(in);
        in = in->get_down();
    }
}

template<int sign, typename Visitor>
    requires sign_integral<decltype(sign), sign>
void map_mid_trail(const banana_tree_node<sign>* max_node, Visitor visitor) {
    massert(max_node->is_internal(), "Expected to start iterating trails at a maximum.");
    auto* mid = max_node->get_mid();
    const auto* birth = max_node->get_birth();
    while (mid != birth) {
        visitor(mid);
        mid = mid->get_down();
    }
}

// Applies `visitor` to each banana (node pair) in the tree `b`,
// iterating over bananas in DFS-manner.
// `visitor` should take four arguments:
// two `banana_tree<sign>::const_node_ptr_type` and two `int`,
// where the two pointers are the minimum and maximum of the current banana, respectively,
// the first `int` is the number of "ancestor bananas" of the current banana,
// the second `int` is the node depth.
template<int sign, typename Visitor>
    requires sign_integral<decltype(sign), sign>
void map_banana_dfs(const banana_tree<sign> &b, Visitor visitor) {
    using node_ptr_t = banana_tree<sign>::const_node_ptr_type;
    std::vector<std::tuple<node_ptr_t, int, int>> stack;
    stack.push_back(std::make_tuple(b.get_special_root(), 0, 0));
    while (!stack.empty()) {
        auto [current, nesting_depth, node_depth] = stack.back();
        stack.pop_back();
        visitor(current->get_birth(), current, nesting_depth, node_depth);
        auto in_node_depth = node_depth;
        auto mid_node_depth = node_depth;
        map_in_trail(current, [&stack, &nesting_depth, &in_node_depth](node_ptr_t node) {
            ++in_node_depth;
            stack.push_back(std::make_tuple(node, nesting_depth+1, in_node_depth));
        });
        map_mid_trail(current, [&stack, &nesting_depth, &mid_node_depth](node_ptr_t node) {
            ++mid_node_depth;
            stack.push_back(std::make_tuple(node, nesting_depth+1, mid_node_depth));
        });
    }
}

} // End of namespace `bananas`
