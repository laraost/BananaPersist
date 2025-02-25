#pragma once

#include <cstdlib>
#include <ostream>
#include <type_traits>

#include <boost/intrusive/avl_set.hpp>
#include <boost/intrusive/avltree_algorithms.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/splay_set.hpp>
#include <boost/intrusive/splaytree_algorithms.hpp>

#include "datastructure/list_item.h"
#include "persistence_defs.h"
#include "utility/debug.h"
#include "utility/errors.h"
#include "utility/stats.h"

namespace bananas {

namespace internal {

using item_member_hook_option = boost::intrusive::member_hook<list_item,
                                                              default_set_member_hook_type,
                                                              &list_item::search_tree_hook>;

using item_avl_tree = boost::intrusive::avl_set<list_item,
                                                item_member_hook_option,
                                                boost::intrusive::constant_time_size<false>>; 
using item_splay_tree = boost::intrusive::splay_set<list_item,
                                                    item_member_hook_option,
                                                    boost::intrusive::constant_time_size<false>>;

// Template for defining "bulk" operations on search trees,
// i.e., splitting and joining.
template<typename T>
class bulk_algorithms {
    using tree_type = T;

public:
    // Join the `right_tree` to `left_tree`.
    // Expects that all items in `right_tree` are strictly greater than the items in `left_tree`.
    // After returning, `left_tree` contains the joined tree and `right_tree` is empty.
    static void join(tree_type &left_tree, tree_type &right_tree);

    // Cut `right_tree` such that every item greater or equal to `cut_item` ends up in `right_tree`
    // and other items end up in `left_tree`.
    // Expects `left_tree` to be empty.
    static void cut_to_left(tree_type& left_tree, tree_type& right_tree, const list_item &cut_item);

    // Cut `left_tree` such that every item greater or equal to `cut_item` ends up in `right_tree`
    // and other items end up in `left_tree`.
    // Expects `right_tree` to be empty.
    static void cut_to_right(tree_type& left_tree, tree_type& right_tree, const list_item &cut_item);
};

#ifdef AVL_SEARCH_TREE
// "Bulk" operations for AVL trees.
template<>
class bulk_algorithms<item_avl_tree> {
    using tree_type = item_avl_tree;
    using value_type = typename tree_type::value_type;
    using node_traits = typename tree_type::node_traits;
    using algos = boost::intrusive::avltree_algorithms<node_traits>;

public:
    static void join(tree_type& /*left_tree*/, tree_type& /*right_tree*/) {
        DEBUG_MSG("`join` is not implemented for AVL trees.");
        std::abort();
    }

    static void cut_to_left(tree_type& /*left_tree*/, tree_type& /*right_tree*/, const list_item& /*cut_item*/) {
        DEBUG_MSG("`cut_to_left` is not implemented for AVL trees.");
        std::abort();
    }
    static void cut_to_right(tree_type& /*left_tree*/, tree_type& /*right_tree*/, const list_item& /*cut_item*/) {
        DEBUG_MSG("`cut_to_right` is not implemented for AVL trees.");
        std::abort();
    }

};
#endif // End of `#ifdef AVL_SEARCH_TREE`

#ifdef SPLAY_SEARCH_TREE
// "Bulk" operations for splay trees.
template<>
class bulk_algorithms<item_splay_tree> {
    using tree_type = item_splay_tree;
    using value_type = typename tree_type::value_type;
    using node_traits = typename tree_type::node_traits;
    using algos = boost::intrusive::splaytree_algorithms<node_traits>;

public:
    static void join(tree_type &left_tree, tree_type &right_tree) {
        if (right_tree.empty()) {
            return;
        }
        if (left_tree.empty()) {
            // Make the right tree the left tree.
            node_traits::set_left(left_tree.header_ptr(), right_tree.header_ptr()->left_);
            node_traits::set_right(left_tree.header_ptr(), right_tree.header_ptr()->right_);
            node_traits::set_parent(left_tree.header_ptr(), right_tree.header_ptr()->parent_);
            node_traits::set_parent(left_tree.header_ptr()->parent_, left_tree.header_ptr()); 
            algos::init_header(right_tree.header_ptr());
            return;
        }
        massert(*left_tree.rbegin() < *right_tree.begin(),
                "Expected items in `right_tree` to be strictly greater than items in `left_tree`.");

        // Make the rightmost node of the left tree the root of the left tree
        auto *left_rightmost_node = left_tree.header_ptr()->right_;
        algos::splay_up(left_rightmost_node, left_tree.header_ptr());
        // Attach the right tree to the new root of the left tree
        node_traits::set_right(left_rightmost_node, right_tree.root().pointed_node());
        node_traits::set_parent(right_tree.root().pointed_node(), left_rightmost_node);
        // Update the header of the left subtree and reset the header of the right subtree
        node_traits::set_right(left_tree.header_ptr(), right_tree.header_ptr()->right_);
        algos::init_header(right_tree.header_ptr());
    }

    // Cut `right_tree` such that every item greater or equal to `cut_item` ends up in `right_tree`
    // and other items end up in `left_tree`.
    // Expects `left_tree` to be empty.
    static void cut_to_left(tree_type& left_tree, tree_type& right_tree, const list_item &cut_item) {
        massert(left_tree.empty(), "Expected an empty left tree.");
        if (right_tree.empty()) {
            return;
        }
        // Find the first element that's greater or equal to `cut_item`.
        // TODO: first test that lower_bound != end (no items to the right of the cut), then proceed.
        auto lb = right_tree.lower_bound(cut_item);
        if (lb == right_tree.end()) {
            // No items in the right tree; move everything to the left tree.
            right_tree.swap(left_tree);
            return;
        }
        auto* next_right = lb.pointed_node();
        algos::splay_up(next_right, right_tree.header_ptr());
        // Due to splaying this is now the root.
        massert(right_tree.root().pointed_node() == next_right,
                "Expected the looked up node to become the root.");
        
        // The left child of `next_right` becomes the root of the new left tree.
        auto* root_left = node_traits::get_left(next_right);
        if (root_left != nullptr) {
            // Set up the header of the left tree.
            // Parent points to the root, left points to the smallest element, right points to the largest element.
            node_traits::set_parent(left_tree.header_ptr(), root_left);
            node_traits::set_left(left_tree.header_ptr(), node_traits::get_left(right_tree.header_ptr()));
            node_traits::set_right(left_tree.header_ptr(), algos::maximum(root_left));
            node_traits::set_parent(root_left, left_tree.header_ptr());
        } else {
            // `next_right` has no left child, so the left tree is empty.
            algos::init_header(left_tree.header_ptr());   
        }
        
        // `next_right` is the leftmost element of `right_tree`, by definition.
        node_traits::set_parent(right_tree.header_ptr(), next_right);
        node_traits::set_left(right_tree.header_ptr(), next_right);
        node_traits::set_left(next_right, nullptr);
        node_traits::set_parent(next_right, right_tree.header_ptr());
    }

    // Cut `left_tree` such that every item greater or equal to `cut_item` ends up in `right_tree`
    // and other items end up in `left_tree`.
    // Expects `right_tree` to be empty.
    static void cut_to_right(tree_type& left_tree, tree_type& right_tree, const list_item &cut_item) {
        massert(right_tree.empty(), "Expected an empty right tree.");
        if (left_tree.empty()) {
            return;
        }
        auto lb = left_tree.lower_bound(cut_item);
        if (lb == left_tree.end()) {
            // No items in the right tree, so just stop.
            return;
        }
        auto* next_right = lb.pointed_node();
        algos::splay_up(next_right, left_tree.header_ptr());
        massert(left_tree.root().pointed_node() == next_right,
                "Expected the looked up node to become the root.");
        auto* rightmost = node_traits::get_right(left_tree.header_ptr());

        auto* root_left = node_traits::get_left(next_right);
        if (root_left != nullptr) {
            node_traits::set_parent(left_tree.header_ptr(), root_left);
            node_traits::set_right(left_tree.header_ptr(), algos::maximum(root_left));
            node_traits::set_parent(root_left, left_tree.header_ptr());
        } else {
            algos::init_header(left_tree.header_ptr());
        }

        node_traits::set_parent(right_tree.header_ptr(), next_right);
        node_traits::set_left(right_tree.header_ptr(), next_right);
        node_traits::set_right(right_tree.header_ptr(), rightmost);
        node_traits::set_left(next_right, nullptr);
        node_traits::set_parent(next_right, right_tree.header_ptr());
    }

};
#endif // end of `#ifdef SPLAY_SEARCH_TREE`

} // End of namespace `internal`

#ifdef AVL_SEARCH_TREE
using item_search_tree_type = internal::item_avl_tree;
#elif defined SPLAY_SEARCH_TREE
using item_search_tree_type = internal::item_splay_tree;
#endif

enum class item_storage_type {
minimum = -1,
non_critical = 0,
maximum = 1
};

template<item_storage_type storage_type>
class dictionary {
    using dictionary_type = dictionary<storage_type>;

    using value_type = item_search_tree_type::value_type;
    using key_type = item_search_tree_type::key_type;
    using iterator = item_search_tree_type::iterator;
    using const_iterator = item_search_tree_type::const_iterator;

public:

    dictionary() {}
    template<typename Iter>
    dictionary(Iter begin, Iter end) : search_tree(begin, end) {}

    iterator begin() noexcept {
        return search_tree.begin();
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return search_tree.begin();
    }

    iterator end() noexcept {
        return search_tree.end();
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return search_tree.end();
    }

    bool contains(value_type &item) {
        DICT_TIME_BEGIN(contains);
        const bool result = search_tree.find(item) != search_tree.end();
        DICT_TIME_END(contains);
        return result;
    }

    void insert_item(value_type &item) {
        if constexpr (storage_type == item_storage_type::minimum) {
            massert(item.is_minimum<1>() || item.is_up_type<1>(),
                    "Item inserted into dictionary with storage type `minimum` has to be a minimum or up-type.");
        } else if constexpr (storage_type == item_storage_type::maximum) {
            massert(item.is_maximum<1>() || item.is_down_type<1>(),
                    "Item inserted into dictionary with storage type `maximum` has to be a maximum or down-type.");
        } else {
            massert(item.is_noncritical<1>(), "Item inserted into dictionary with storage type `non_critical` has to be non-critical.");
        }
        DICT_TIME_BEGIN(insert);
        search_tree.insert(item);
        DICT_TIME_END(insert);
    }

    void erase_item(value_type &item) {
        massert(contains(item), "Expected to erase an item contained in the tree.");
        DICT_TIME_BEGIN(erase);
        search_tree.erase(search_tree.iterator_to(item));
        DICT_TIME_END(erase);
    }

    // Obtain an iterator to the first item strictly greater than `item`
    // or `end` if no such item exists.
    iterator next_item(const key_type &item) {
        DICT_TIME_BEGIN(next);
        return search_tree.upper_bound(item);
        DICT_TIME_END(next);
    }

    // Obtain an iterator to the last item strictly less than `item`
    // or `end` if no such item exists.
    iterator previous_item(const key_type &item) {
        DICT_TIME_BEGIN(previous);
        if (search_tree.empty()) {
            DICT_TIME_END(previous);
            return search_tree.end();
        }
        auto iter = search_tree.lower_bound(item);
        // `lower_bound` yields an item `i` with `i >= item`.
        // If `item` is in the tree, then `i == item` and we return the previous item.
        // Otherwise, `i > item` and thus `--i < item`.
        --iter;
        DICT_TIME_END(previous);
        return iter;
    }

    // Return an iterator to the item that's closest to `closest_to`,
    // but not on the same side of `closest_to` as `opposite_to`.
    // If `opposite_to < closest_to` this is equivalent to `next_item(closest_to)`;
    // if `opposite_to > closest_to` this is equivalent to `previous_item(closest_to)`.
    iterator closest_item_on_opposite_side(const key_type &closest_to, const key_type &opposite_to) {
        if (opposite_to < closest_to) {
            return next_item(closest_to);
        }
        return previous_item(closest_to);
    }

    void join(dictionary_type &right_dict) {
        DICT_TIME_BEGIN(join);
        internal::bulk_algorithms<item_search_tree_type>::join(search_tree, right_dict.search_tree);
        DICT_TIME_END(join);
    }

    void cut_right(const key_type &item, dictionary_type &new_right_dict) {
        DICT_TIME_BEGIN(cut);
        internal::bulk_algorithms<item_search_tree_type>::cut_to_right(search_tree, new_right_dict.search_tree, item);
        DICT_TIME_END(cut);
    }
    void cut_left(const key_type &item, dictionary_type &new_left_dict) {
        DICT_TIME_BEGIN(cut);
        internal::bulk_algorithms<item_search_tree_type>::cut_to_left(new_left_dict.search_tree, search_tree, item);
        DICT_TIME_END(cut);
    }

    void print(std::ostream &stream) {
        for (auto &item: search_tree) {
            stream << item.get_interval_order() << " ";
        }
    }

private:
    item_search_tree_type search_tree;

};

template<int sign>
    requires sign_integral<decltype(sign), sign>
using signed_min_dictionary = dictionary<static_cast<item_storage_type>(-sign)>;
template<int sign>
    requires sign_integral<decltype(sign), sign>
using signed_max_dictionary = dictionary<static_cast<item_storage_type>(sign)>;

using min_dictionary = signed_min_dictionary<1>;
using max_dictionary = signed_max_dictionary<1>;
using nc_dictionary = dictionary<item_storage_type::non_critical>;

} // End of namespace `bananas`
