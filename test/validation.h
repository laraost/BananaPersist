#pragma once

#include <boost/core/demangle.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "datastructure/banana_tree.h"

// Generic version; expects a `banana_tree_node*` Turn a node-pointer into a string.
template<typename ptr_type>
std::string ptr_to_string(ptr_type ptr) { return "node for " + std::to_string(ptr->get_item()->get_interval_order()); }
// Turn an item into a string
template<>
inline std::string ptr_to_string(bananas::list_item* ptr) { return "item " + std::to_string(ptr->get_interval_order()); }
template<>
inline std::string ptr_to_string(const bananas::list_item* ptr) { return "item " + std::to_string(ptr->get_interval_order()); }
// Turn a nullptr_t object into a string.
template<>
inline std::string ptr_to_string(std::nullptr_t /*ptr*/) { return "nullptr"; }

template<typename ptr_type_a, typename ptr_type_b>
std::string expect_message(ptr_type_a to_test, ptr_type_b to_expect) {
    return "  Expected " + ptr_to_string(to_expect) + " but got " + ptr_to_string(to_test);
}

// This macro wraps `EXPECT_EQ`, such that the two given nodes can be compared
// and the original expressions passed into the macro are preserved in the output on failure.
// Also outputs the nodes by printing their item order.
// If one of the arguments is a null-pointer, but not of `std::nullptr_t`, this will fail.
#define EXPECT_NODE_EQ(to_test, to_expect) \
        EXPECT_EQ(to_test, to_expect) << expect_message(to_test, to_expect)

template<int sign>
auto node_of(bananas::list_item* item_ptr) {
    return item_ptr->template get_node<sign>();
}
template<int sign>
std::nullptr_t node_of(std::nullptr_t) {
    return nullptr;
}
#define DEF_NODE_ACCESSOR(ptr_name) \
    template<int sign, typename item_ptr_type> \
    auto ptr_name##_of(item_ptr_type item_ptr) { \
        return item_ptr->template get_node<sign>()->get_##ptr_name(); \
    } \
    template<int sign> \
    std::nullptr_t ptr_name##_of(std::nullptr_t) { \
        return nullptr; \
    }
DEF_NODE_ACCESSOR(up)
DEF_NODE_ACCESSOR(down)
DEF_NODE_ACCESSOR(in)
DEF_NODE_ACCESSOR(mid)
DEF_NODE_ACCESSOR(low)
DEF_NODE_ACCESSOR(death)

// This macro wraps `EXPECT_EQ` with the aim of being used for items.
// It ends up being the same as `EXPECT_NODE_EQ`, but using a different naming scheme would be weird.
#define EXPECT_ITEM_EQ(to_test, to_expect) \
        EXPECT_EQ(to_test, to_expect) << expect_message(to_test, to_expect)
// Expect `to_expect` at the up-pointer of the node associated with `to_test` and the given `sign`.
// Compares `to_test->get_node<sign>()->get_up() == to_expect->get_node<sign>()`.
#define EXPECT_up(sign, to_test, to_expect) \
        EXPECT_EQ(up_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(up_of<sign>(to_test), node_of<sign>(to_expect))
// Same as `EXPECT_up`, but for down-pointers
#define EXPECT_down(sign, to_test, to_expect) \
        EXPECT_EQ(down_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(down_of<sign>(to_test), node_of<sign>(to_expect))
// Same as `EXPECT_up`, but for in-pointers
#define EXPECT_in(sign, to_test, to_expect) \
        EXPECT_EQ(in_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(in_of<sign>(to_test), node_of<sign>(to_expect))
// Same as `EXPECT_up`, but for mid-pointers
#define EXPECT_mid(sign, to_test, to_expect) \
        EXPECT_EQ(mid_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(mid_of<sign>(to_test), node_of<sign>(to_expect))
// Same as `EXPECT_up`, but for low-pointers
#define EXPECT_low(sign, to_test, to_expect) \
        EXPECT_EQ(low_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(low_of<sign>(to_test), node_of<sign>(to_expect))
// Same as `EXPECT_up`, but for death-pointers
#define EXPECT_death(sign, to_test, to_expect) \
        EXPECT_EQ(death_of<sign>(to_test), node_of<sign>(to_expect)) << expect_message(death_of<sign>(to_test), node_of<sign>(to_expect))

template<typename T>
std::string demangle_type() {
    return boost::core::demangle(typeid(T).name());
}

// Validate that the string-order of `banana_tree` increases along the interval and
// matches the order of critical items specified by the range `[begin_items, end_items)`.
// If `skip_hook` is true, the first node output by the string order iterator is skipped
// in comparisons of the item order.
template<typename tree_type, typename iterator_type>
void validate_string_order(tree_type &banana_tree,
                           const iterator_type &begin_items,
                           const iterator_type &end_items,
                           bool skip_hook) {
    std::vector<const typename tree_type::node_type*> visited_nodes;
    for (const auto* node: banana_tree.string()) {
        if (!node->is_special_root()) {
            if (!visited_nodes.empty()) {
                ASSERT_NE(visited_nodes.back()->get_item(), node->get_item())
                    << "  Error: "
                    << ptr_to_string(node->get_item())
                    << " is visited twice.";
                ASSERT_LT(*(visited_nodes.back()->get_item()),
                          *(node->get_item()))
                    << "when validating a tree of type "
                    << demangle_type<tree_type>() << ";\n"
                    << "  comparison "
                    << visited_nodes.back()->get_item()->get_interval_order() << " < "
                    << node->get_item()->get_interval_order()
                    << " failed,\n"
                    << "  for items with values "
                    << visited_nodes.back()->get_value() << " and " << node->get_value();
            }
            visited_nodes.push_back(node);
        }
    }

    size_t idx = skip_hook ? 1 : 0;
    for (auto it = begin_items; it != end_items; ++it, ++idx) {
        EXPECT_ITEM_EQ(visited_nodes[idx]->get_item(), &(*it));
    }
}

// Expect `node` to be on the left spine, but not on the right spine.
template<typename node_type>
void expect_left_spine(node_type *node) {
    EXPECT_TRUE(node->is_on_left_spine())
        << "Incorrect spine label for "
        << node->get_item()->get_interval_order() << "\n"
        << " of type " << demangle_type<node_type>();
    EXPECT_FALSE(node->is_on_both_spines())
        << "Non-special root "
        << node->get_item()->get_interval_order()
        << " is on both spines.\n"
        << " Node is of type " << demangle_type<node_type>();
}

// Expect `node` to be on the right spine, but not on the left spine.
template<typename node_type>
void expect_right_spine(node_type *node) {
    EXPECT_TRUE(node->is_on_right_spine())
        << "Incorrect spine label for "
        << node->get_item()->get_interval_order() << "\n"
        << " of type " << demangle_type<node_type>();
    EXPECT_FALSE(node->is_on_both_spines())
        << "Non-special root "
        << node->get_item()->get_interval_order()
        << " is on both spines.\n"
        << " Node is of type " << demangle_type<node_type>();
}

// Expect `node` to not be on any spine.
template<typename node_type>
void expect_not_on_spine(node_type *node) {
    EXPECT_FALSE(node->is_on_spine())
        << "Incorrect spine label for "
        << node->get_item()->get_interval_order() << "\n"
        << " of type " << demangle_type<node_type>();
}

// Expect `node` to be on both the left and right spine.
// Since only the special root may satisfy this,
// also expect `node` to be a special root.
template<typename node_type>
void expect_both_spines(node_type *node) {
    EXPECT_TRUE(node->is_on_both_spines())
        << "Expected special root to be on both spines.\n"
        << " Node is of type " << demangle_type<node_type>();
    EXPECT_TRUE(node->is_special_root())
        << "Expected node on both spines to be a special root.\n"
        << " Node is of type " << demangle_type<node_type>();
}

template<typename tree_type, typename iterator_type>
void validate_spine_labels(tree_type &tree,
                           const iterator_type &begin_items,
                           const iterator_type &end_items) {
    auto* special_root = tree.get_special_root();
    std::unordered_set<bananas::list_item*> left_spine_items;
    std::unordered_set<bananas::list_item*> right_spine_items;
    auto* left_node = special_root->get_in();
    while (true) {
        expect_left_spine(left_node);
        left_spine_items.insert(left_node->get_item());
        if (left_node->is_leaf()) {
            break;
        }
        left_node = left_node->get_in();
    }
    auto* right_node = special_root->get_mid();
    while (true) {
        expect_right_spine(right_node);
        right_spine_items.insert(right_node->get_item());
        if (right_node->is_leaf()) {
            break;
        }
        right_node = right_node->get_in();
    }

    for (auto it = begin_items; it != end_items; ++it) {
        auto* node = it->template get_node<tree_type::sign_v>();
        EXPECT_FALSE(node->is_on_both_spines())
            << "Found non special-root "
            << node->get_item()->get_interval_order()
            << " with label `on_both_spines`\n"
            << " in tree of type " << demangle_type<tree_type>();
        if (node->is_on_left_spine()) {
            EXPECT_TRUE(left_spine_items.contains(&(*it)))
                << "Found node "
                << node->get_item()->get_interval_order()
                << " with label `on_left_spine` although it is not `in*(special_root)`\n"
                << " in tree of type " << demangle_type<tree_type>();
        } else if (node->is_on_right_spine()) {
            EXPECT_TRUE(right_spine_items.contains(&(*it)))
                << "Found node "
                << node->get_item()->get_interval_order()
                << " with label `on_right_spine` although it is not `in*(mid(special_root))`\n"
                << " in tree of type " << demangle_type<tree_type>();
        } else if (!node->is_on_spine()) {
            EXPECT_FALSE(left_spine_items.contains(&(*it)))
                << "Found node "
                << node->get_item()->get_interval_order()
                << " with label `not_on_spine` although it is on the left spine\n"
                << " in tree of type " << demangle_type<tree_type>();
            EXPECT_FALSE(right_spine_items.contains(&(*it)))
                << "Found node "
                << node->get_item()->get_interval_order()
                << " with label `not_on_spine` although it is on the right spine\n"
                << " in tree of type " << demangle_type<tree_type>();
        }
    }
}
