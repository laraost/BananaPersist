#include <algorithm>
#include <boost/intrusive/splaytree_algorithms.hpp>
#include <numeric>
#include <random>
#include <gtest/gtest.h>
#include <vector>

#include "datastructure/dictionary.h"
#include "persistence_defs.h"

using namespace bananas;

std::vector<double> init_random_order(size_t num, double init_val) {
    std::vector<double> result(num);
    std::iota(result.begin(), result.end(), init_val);
    std::shuffle(result.begin(), result.end(), std::mt19937(std::random_device()()));
    return result;
}

std::vector<list_item> init_item_vector(const std::vector<double> &order) {
    std::vector<list_item> result;
    for (auto& o: order) {
        result.emplace_back(o, 0);
    }
    return result;
}

TEST(SplayTree, JoinsCorrectly) {
    using tree_type = internal::item_splay_tree;

    auto left_order = init_random_order(10, 0);
    auto right_order = init_random_order(10, 20);
    auto left_items = init_item_vector(left_order);
    auto right_items = init_item_vector(right_order);

    tree_type left_tree(left_items.begin(), left_items.end());
    tree_type right_tree(right_items.begin(), right_items.end());

    internal::bulk_algorithms<tree_type>::join(left_tree, right_tree);

    EXPECT_TRUE(right_tree.empty());

    std::vector<interval_order_type> orders_sorted;
    for (auto& item: left_tree) {
        orders_sorted.push_back(item.get_interval_order());
    }
    EXPECT_EQ(orders_sorted.size(), left_items.size() + right_items.size());
    for (size_t idx = 1; idx < orders_sorted.size(); ++idx) {
        EXPECT_GT(orders_sorted[idx], orders_sorted[idx - 1]);
    }
}

TEST(SplayTree, JoinsEmptyLeftTreeWithRightTree) {
    using tree_type = internal::item_splay_tree;

    auto right_order = init_random_order(16, 10);
    std::vector<list_item> right_items = init_item_vector(right_order);

    tree_type left_tree;
    tree_type right_tree(right_items.begin(), right_items.end());

    internal::bulk_algorithms<tree_type>::join(left_tree, right_tree);

    EXPECT_TRUE(right_tree.empty());
}

TEST(SplayTree, JoinsEmptyTrees) {
    using tree_type = internal::item_splay_tree;
    tree_type left_tree;
    tree_type right_tree;

    internal::bulk_algorithms<tree_type>::join(left_tree, right_tree);

    EXPECT_TRUE(left_tree.empty());
    EXPECT_TRUE(right_tree.empty());
}

TEST(SplayTree, SplitsCorrectlyRight) {
    using tree_type = internal::item_splay_tree;

    int num_items = 59;
    auto order_vec = init_random_order(num_items, 0);
    auto items = init_item_vector(order_vec);
    interval_order_type split_pos = 26;
    list_item split_item{split_pos, 0};

    tree_type tree;
    for (auto &item: items) {
        tree.insert(item);
    }
    tree_type new_right_tree;

    internal::bulk_algorithms<tree_type>::cut_to_right(tree, new_right_tree, split_item);
    
    std::vector<interval_order_type> left_orders_sorted;
    std::vector<interval_order_type> right_orders_sorted;
    for (auto& item: tree) {
        left_orders_sorted.push_back(item.get_interval_order());
    }
    for (auto& item: new_right_tree) {
        right_orders_sorted.push_back(item.get_interval_order());
    }
    EXPECT_NE(left_orders_sorted.back(), split_pos);
    EXPECT_EQ(right_orders_sorted.front(), split_pos);
    for (size_t idx = 0; idx < left_orders_sorted.size() - 1; ++idx) {
        EXPECT_GT(left_orders_sorted[idx+1], left_orders_sorted[idx]);
        EXPECT_LT(left_orders_sorted[idx], split_pos);
    }
    for (size_t idx = 0; idx < right_orders_sorted.size() - 1; ++idx) {
        EXPECT_GT(right_orders_sorted[idx+1], right_orders_sorted[idx]);
        EXPECT_GE(right_orders_sorted[idx], split_pos);
    }
    EXPECT_GT(right_orders_sorted.back(), split_pos);
}

TEST(SplayTree, SplitsCorrectlyLeft) {
    using tree_type = internal::item_splay_tree;

    int num_items = 59;
    auto order_vec = init_random_order(num_items, 0);
    auto items = init_item_vector(order_vec);
    interval_order_type split_pos = 26;
    list_item split_item{split_pos, 0};

    tree_type tree;
    for (auto &item: items) {
        tree.insert(item);
    }
    tree_type new_left_tree;

    internal::bulk_algorithms<tree_type>::cut_to_left(new_left_tree, tree, split_item);
    
    std::vector<interval_order_type> left_orders_sorted;
    std::vector<interval_order_type> right_orders_sorted;
    for (auto& item: new_left_tree) {
        left_orders_sorted.push_back(item.get_interval_order());
    }
    for (auto& item: tree) {
        right_orders_sorted.push_back(item.get_interval_order());
    }
    EXPECT_NE(left_orders_sorted.back(), split_pos);
    EXPECT_EQ(right_orders_sorted.front(), split_pos);
    for (size_t idx = 0; idx < left_orders_sorted.size() - 1; ++idx) {
        EXPECT_GT(left_orders_sorted[idx+1], left_orders_sorted[idx]);
        EXPECT_LT(left_orders_sorted[idx], split_pos);
    }
    for (size_t idx = 0; idx < right_orders_sorted.size() - 1; ++idx) {
        EXPECT_GT(right_orders_sorted[idx+1], right_orders_sorted[idx]);
        EXPECT_GE(right_orders_sorted[idx], split_pos);
    }
    EXPECT_GT(right_orders_sorted.back(), split_pos);
}
