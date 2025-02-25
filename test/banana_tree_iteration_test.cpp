#include <array>
#include <gtest/gtest.h>

#include "datastructure/banana_tree.h"

#include "datastructure/list_item.h"
#include "example_trees/paper_tree.h"
#include "utility/recycling_object_pool.h"
#include "validation.h"

using namespace bananas;

TEST(BananaTreeIteration, WalkIteratorOrdersCorrectly) {
    // We use a single dummy item to associate with all nodes,
    // since nodes need to be associated with some item.
    // Since the items are irrelevant for this test, this doesn't cause issues.
    list_item dummy{0};
    up_tree_node node_a{&dummy};
    up_tree_node node_a_m{&dummy};
    up_tree_node node_b{&dummy};
    up_tree_node node_b_m{&dummy};
    up_tree_node node_c{&dummy};
    up_tree_node node_c_m{&dummy};
    up_tree_node node_d{&dummy};
    up_tree_node node_d_m{&dummy};
    up_tree_node node_e{&dummy};
    up_tree_node node_e_m{&dummy};
    up_tree_node node_f{&dummy};
    up_tree_node node_f_m{&dummy};

    node_a.set_pointers(nullptr, nullptr, &node_c, &node_e, &node_a_m, nullptr);
    node_b.set_pointers(&node_c, &node_a_m, &node_b_m, &node_b_m, &node_a_m, nullptr);
    node_c.set_pointers(&node_a, &node_b, &node_d, &node_c_m, &node_a_m, nullptr);
    node_d.set_pointers(&node_c, &node_c_m, &node_d_m, &node_d_m, &node_c_m, nullptr);
    node_e.set_pointers(&node_a, &node_a_m, &node_f, &node_e_m, &node_a_m, nullptr);
    node_f.set_pointers(&node_e, &node_e_m, &node_f_m, &node_f_m, &node_e_m, nullptr);
    node_a_m.set_pointers(nullptr, nullptr, &node_b, &node_e, &node_a_m, &node_a);
    node_b_m.set_pointers(nullptr, nullptr, &node_b, &node_b, &node_b_m, &node_b);
    node_c_m.set_pointers(nullptr, nullptr, &node_d, &node_c, &node_c_m, &node_c);
    node_d_m.set_pointers(nullptr, nullptr, &node_d, &node_d, &node_d_m, &node_d);
    node_e_m.set_pointers(nullptr, nullptr, &node_f, &node_e, &node_e_m, &node_e);
    node_f_m.set_pointers(nullptr, nullptr, &node_f, &node_f, &node_f_m, &node_f);

    std::vector<std::pair<up_tree_node*, up_tree_node*>> bananas;
    auto iter_pair = banana_tree<1>::walk_iterator_pair{&node_a};
    for (auto &banana: iter_pair) {
        bananas.push_back(banana);
    }

    ASSERT_EQ(bananas.size(), 6);
    EXPECT_EQ(bananas[0], std::pair(&node_a_m, &node_a));
    EXPECT_EQ(bananas[1], std::pair(&node_b_m, &node_b));
    EXPECT_EQ(bananas[2], std::pair(&node_c_m, &node_c));
    EXPECT_EQ(bananas[3], std::pair(&node_d_m, &node_d));
    EXPECT_EQ(bananas[4], std::pair(&node_e_m, &node_e));
    EXPECT_EQ(bananas[5], std::pair(&node_f_m, &node_f));
}

TEST_F(PaperUpTreeTest, WalksPaperExampleUpTreeCorrectly) {
    std::vector<std::pair<up_tree_node*, up_tree_node*>> bananas;
    for (auto &banana: up_tree.walk()) {
        bananas.push_back(banana);
    }

    ASSERT_EQ(bananas.size(), 8);
    EXPECT_EQ(bananas[0].first,  nodes[j]);
    EXPECT_EQ(bananas[0].second, special_root);

    EXPECT_EQ(bananas[1].first,  nodes[h]);
    EXPECT_EQ(bananas[1].second, nodes[i]);

    EXPECT_EQ(bananas[2].first,  nodes[f]);
    EXPECT_EQ(bananas[2].second, nodes[g]);

    EXPECT_EQ(bananas[3].first,  nodes[d]);
    EXPECT_EQ(bananas[3].second, nodes[e]);

    EXPECT_EQ(bananas[4].first,  left_hook);
    EXPECT_EQ(bananas[4].second, nodes[c]);

    EXPECT_EQ(bananas[5].first,  nodes[n]);
    EXPECT_EQ(bananas[5].second, nodes[k]);

    EXPECT_EQ(bananas[6].first,  nodes[l]);
    EXPECT_EQ(bananas[6].second, nodes[m]);

    EXPECT_EQ(bananas[7].first,  right_hook);
    EXPECT_EQ(bananas[7].second, nodes[o]);
}

TEST_F(PaperDownTreeTest, WalksPaperExampleDownTreeCorrectly) {
    std::vector<std::pair<down_tree_node*, down_tree_node*>> bananas;
    for (auto &banana: down_tree.walk()) {
        bananas.push_back(banana);
    }

    ASSERT_EQ(bananas.size(), 7);
    EXPECT_EQ(bananas[0].first,  nodes[o]);
    EXPECT_EQ(bananas[0].second, special_root);

    EXPECT_EQ(bananas[1].first,  nodes[k]);
    EXPECT_EQ(bananas[1].second, nodes[n]);

    EXPECT_EQ(bananas[2].first,  nodes[m]);
    EXPECT_EQ(bananas[2].second, nodes[l]);

    EXPECT_EQ(bananas[3].first,  nodes[e]);
    EXPECT_EQ(bananas[3].second, nodes[j]);

    EXPECT_EQ(bananas[4].first,  nodes[c]);
    EXPECT_EQ(bananas[4].second, nodes[d]);

    EXPECT_EQ(bananas[5].first,  nodes[g]);
    EXPECT_EQ(bananas[5].second, nodes[f]);

    EXPECT_EQ(bananas[6].first,  nodes[i]);
    EXPECT_EQ(bananas[6].second, nodes[h]);
}

TEST_F(PaperUpTreeTest, StringIteratorOrdersNodesCorrectly) {
    validate_string_order(up_tree, items.begin(), items.end(), true);
}

TEST_F(PaperUpTreeTestWithExtraWiggle, StringIteratorOrdersNodesCorrectly) {
    validate_string_order(up_tree, items.begin(), items.end(), true);
}

TEST_F(PaperDownTreeTest, StringIteratorOrdersNodesCorrectly) {
    validate_string_order(down_tree, items.begin(), items.end(), false);
}

TEST(ModifiedPaperInterval, StringIteratorOrdersNodesCorrectly) {
    constexpr size_t c = 0, d = 1, e = 2, f = 3, g = 4, h = 5, j = 6, l = 7, m = 8, n = 9, o = 10;
    std::array<list_item, 11> items = {
        list_item{c, 6}, list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
        list_item{g, 8}, list_item{h, 4}, list_item{j, 12.5}, list_item{l, 9},
        list_item{m, 10}, list_item{n, 3}, list_item{o, 13}
    };
    list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
    list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
    list_item::link(items[g], items[h]); list_item::link(items[h], items[j]);
    list_item::link(items[j], items[l]); list_item::link(items[l], items[m]);
    list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);

    recycling_object_pool<up_tree_node> up_node_pool;
    auto up_tree = banana_tree<1>{up_node_pool, &items[c], &items[o]};

    validate_string_order(up_tree, items.begin(), items.end(), true);
}
