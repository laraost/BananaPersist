#include <array>
#include <gtest/gtest.h>

#include "datastructure/banana_tree.h"
#include "datastructure/list_item.h"
#include "utility/recycling_object_pool.h"

#include "example_trees/paper_tree.h"
#include "validation.h"

using namespace bananas;

TEST(BananaTreeConstruction, SimpleUpTreeConstruction) {
    recycling_object_pool<banana_tree_node<1>> up_node_pool;
    auto item_0 = list_item{0.0, 0.0};
    auto item_1 = list_item{1.0, 1.0};
    list_item::link(item_0, item_1);

    auto tree = banana_tree<1>(up_node_pool,
                               &item_0,
                               &item_1);
    
    const auto *item_0_node = item_0.get_node<1>();
    const auto *item_1_node = item_1.get_node<1>();
    const auto *special_root = tree.get_special_root();
    const auto *left_hook = tree.get_left_hook();
    const auto *right_hook = tree.get_right_hook();
    const auto *global_max = tree.get_global_max();

    EXPECT_EQ(item_0_node->get_low(), item_0_node);
    EXPECT_EQ(item_0_node->get_death(), special_root);
    EXPECT_EQ(item_0_node->get_in(), special_root);
    EXPECT_EQ(item_0_node->get_mid(), item_1_node);
    expect_left_spine(item_0_node);
    
    EXPECT_EQ(right_hook->get_low(), right_hook);
    EXPECT_EQ(right_hook->get_death(), item_1_node);
    expect_right_spine(right_hook);

    EXPECT_EQ(item_1_node->get_low(), item_0_node);
    EXPECT_EQ(item_1_node->get_down(), item_0_node);
    EXPECT_EQ(item_1_node->get_up(), special_root);
    EXPECT_EQ(item_1_node->get_birth(), right_hook);
    EXPECT_TRUE(item_1_node->has_empty_banana());
    expect_right_spine(item_1_node);

    EXPECT_EQ(special_root->get_in(), item_0_node);
    EXPECT_EQ(special_root->get_mid(), item_1_node);
    expect_both_spines(special_root);

    EXPECT_EQ(left_hook, nullptr);

    EXPECT_EQ(global_max, &item_1);
}

TEST(BananaTreeConstruction, SimpleDownTreeConstruction) {
    recycling_object_pool<banana_tree_node<-1>> up_node_pool;
    auto item_0 = list_item{0.0, 0.0};
    auto item_1 = list_item{1.0, 1.0};
    list_item::link(item_0, item_1);

    auto tree = banana_tree<-1>(up_node_pool,
                               &item_0,
                               &item_1);
    
    const auto *item_0_node = item_0.get_node<-1>();
    const auto *item_1_node = item_1.get_node<-1>();
    const auto *special_root = tree.get_special_root();
    const auto *left_hook = tree.get_left_hook();
    const auto *right_hook = tree.get_right_hook();
    const auto *global_max = tree.get_global_max();

    EXPECT_EQ(item_0_node->get_low(), item_1_node);
    EXPECT_EQ(item_0_node->get_up(), special_root);
    EXPECT_EQ(item_0_node->get_down(), item_1_node);
    EXPECT_EQ(item_0_node->get_in(), left_hook);
    EXPECT_EQ(item_0_node->get_mid(), left_hook);
    EXPECT_TRUE(item_0_node->has_empty_banana());
    expect_left_spine(item_0_node);
    
    EXPECT_EQ(left_hook->get_low(), left_hook);
    EXPECT_EQ(left_hook->get_death(), item_0_node);
    expect_left_spine(left_hook);

    EXPECT_EQ(item_1_node->get_low(), item_1_node);
    EXPECT_EQ(item_1_node->get_in(), item_0_node);
    EXPECT_EQ(item_1_node->get_mid(), special_root);
    expect_right_spine(item_1_node);

    EXPECT_EQ(special_root->get_in(), item_0_node);
    EXPECT_EQ(special_root->get_mid(), item_1_node);
    expect_both_spines(special_root);

    EXPECT_EQ(right_hook, nullptr);

    EXPECT_EQ(global_max, &item_0);
}

// Testing that the example map from the paper is turned into the correct tree
TEST_F(PaperUpTreeTest, PaperExampleUpTreeConstructsCorrectly) {
    EXPECT_EQ(up_tree.get_global_max(), &items[o]);

    EXPECT_EQ(nodes[c]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[c]->get_down(), nodes[d]);
    EXPECT_EQ(nodes[c]->get_in(), left_hook);
    EXPECT_EQ(nodes[c]->get_mid(), left_hook);
    EXPECT_EQ(nodes[c]->get_low(), nodes[d]);
    EXPECT_TRUE(nodes[c]->is_on_in_trail());
    expect_left_spine(nodes[c]);

    EXPECT_EQ(nodes[d]->get_up(), nullptr);
    EXPECT_EQ(nodes[d]->get_down(), nullptr);
    EXPECT_EQ(nodes[d]->get_in(), nodes[c]);
    EXPECT_EQ(nodes[d]->get_mid(), nodes[e]);
    EXPECT_EQ(nodes[d]->get_low(), nodes[d]);
    EXPECT_EQ(nodes[d]->get_death(), nodes[e]);
    expect_not_on_spine(nodes[d]);

    EXPECT_EQ(nodes[e]->get_up(), special_root);
    EXPECT_EQ(nodes[e]->get_down(), nodes[g]);
    EXPECT_EQ(nodes[e]->get_in(), nodes[c]);
    EXPECT_EQ(nodes[e]->get_mid(), nodes[d]);
    EXPECT_EQ(nodes[e]->get_low(), nodes[j]);
    EXPECT_TRUE(nodes[e]->is_on_in_trail());
    expect_left_spine(nodes[e]);

    EXPECT_EQ(nodes[f]->get_up(), nullptr);
    EXPECT_EQ(nodes[f]->get_down(), nullptr);
    EXPECT_EQ(nodes[f]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[f]->get_mid(), nodes[g]);
    EXPECT_EQ(nodes[f]->get_low(), nodes[f]);
    EXPECT_EQ(nodes[f]->get_death(), nodes[g]);
    expect_not_on_spine(nodes[f]);

    EXPECT_EQ(nodes[g]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[g]->get_down(), nodes[i]);
    EXPECT_EQ(nodes[g]->get_in(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_mid(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[j]);
    EXPECT_TRUE(nodes[g]->is_on_in_trail());
    expect_not_on_spine(nodes[g]);

    EXPECT_EQ(nodes[h]->get_up(), nullptr);
    EXPECT_EQ(nodes[h]->get_down(), nullptr);
    EXPECT_EQ(nodes[h]->get_in(), nodes[i]);
    EXPECT_EQ(nodes[h]->get_mid(), nodes[i]);
    EXPECT_EQ(nodes[h]->get_low(), nodes[h]);
    EXPECT_EQ(nodes[h]->get_death(), nodes[i]);
    expect_not_on_spine(nodes[h]);

    EXPECT_EQ(nodes[i]->get_up(), nodes[g]);
    EXPECT_EQ(nodes[i]->get_down(), nodes[j]);
    EXPECT_EQ(nodes[i]->get_in(), nodes[h]);
    EXPECT_EQ(nodes[i]->get_mid(), nodes[h]);
    EXPECT_EQ(nodes[i]->get_low(), nodes[j]);
    EXPECT_TRUE(nodes[i]->is_on_in_trail());
    expect_not_on_spine(nodes[i]);

    EXPECT_EQ(nodes[j]->get_up(), nullptr);
    EXPECT_EQ(nodes[j]->get_down(), nullptr);
    EXPECT_EQ(nodes[j]->get_in(), nodes[i]);
    EXPECT_EQ(nodes[j]->get_mid(), nodes[k]);
    EXPECT_EQ(nodes[j]->get_low(), nodes[j]);
    EXPECT_EQ(nodes[j]->get_death(), special_root);
    expect_not_on_spine(nodes[j]);

    EXPECT_EQ(nodes[k]->get_up(), nodes[o]);
    EXPECT_EQ(nodes[k]->get_down(), nodes[j]);
    EXPECT_EQ(nodes[k]->get_in(), nodes[n]);
    EXPECT_EQ(nodes[k]->get_mid(), nodes[m]);
    EXPECT_EQ(nodes[k]->get_low(), nodes[j]);
    EXPECT_TRUE(nodes[k]->is_on_mid_trail());
    expect_not_on_spine(nodes[k]);

    EXPECT_EQ(nodes[l]->get_up(), nullptr);
    EXPECT_EQ(nodes[l]->get_down(), nullptr);
    EXPECT_EQ(nodes[l]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[l]->get_mid(), nodes[m]);
    EXPECT_EQ(nodes[l]->get_low(), nodes[l]);
    EXPECT_EQ(nodes[l]->get_death(), nodes[m]);
    expect_not_on_spine(nodes[l]);

    EXPECT_EQ(nodes[m]->get_up(), nodes[k]);
    EXPECT_EQ(nodes[m]->get_down(), nodes[n]);
    EXPECT_EQ(nodes[m]->get_in(), nodes[l]);
    EXPECT_EQ(nodes[m]->get_mid(), nodes[l]);
    EXPECT_EQ(nodes[m]->get_low(), nodes[n]);
    EXPECT_TRUE(nodes[m]->is_on_mid_trail());
    expect_not_on_spine(nodes[m]);

    EXPECT_EQ(nodes[n]->get_up(), nullptr);
    EXPECT_EQ(nodes[n]->get_down(), nullptr);
    EXPECT_EQ(nodes[n]->get_in(), nodes[k]);
    EXPECT_EQ(nodes[n]->get_mid(), nodes[m]);
    EXPECT_EQ(nodes[n]->get_low(), nodes[n]);
    EXPECT_EQ(nodes[n]->get_death(), nodes[k]);
    expect_not_on_spine(nodes[n]);

    EXPECT_EQ(nodes[o]->get_up(), special_root);
    EXPECT_EQ(nodes[o]->get_down(), nodes[k]);
    EXPECT_EQ(nodes[o]->get_in(), right_hook);
    EXPECT_EQ(nodes[o]->get_mid(), right_hook);
    EXPECT_EQ(nodes[o]->get_low(), nodes[j]);
    EXPECT_TRUE(nodes[o]->is_on_mid_trail());
    expect_right_spine(nodes[o]);

    EXPECT_EQ(left_hook->get_in(), nodes[c]);
    EXPECT_EQ(left_hook->get_mid(), nodes[c]);
    EXPECT_EQ(left_hook->get_low(), left_hook);
    EXPECT_EQ(left_hook->get_death(), nodes[c]);
    expect_left_spine(left_hook);

    EXPECT_EQ(right_hook->get_in(), nodes[o]);
    EXPECT_EQ(right_hook->get_mid(), nodes[o]);
    EXPECT_EQ(right_hook->get_low(), right_hook);
    EXPECT_EQ(right_hook->get_death(), nodes[o]);
    expect_right_spine(right_hook);

    EXPECT_EQ(special_root->get_in(), nodes[e]);
    EXPECT_EQ(special_root->get_mid(), nodes[o]);
    EXPECT_EQ(special_root->get_low(), nodes[j]);
    expect_both_spines(special_root);
}

// Testing that the example map from the paper is turned into the correct tree
TEST_F(PaperDownTreeTest, PaperExampleDownTreeConstructsCorrectly) {
    EXPECT_EQ(down_tree.get_global_max(), &items[j]);

    EXPECT_EQ(nodes[c]->get_up(), nullptr);
    EXPECT_EQ(nodes[c]->get_down(), nullptr);
    EXPECT_EQ(nodes[c]->get_in(), nodes[d]);
    EXPECT_EQ(nodes[c]->get_mid(), nodes[d]);
    EXPECT_EQ(nodes[c]->get_low(), nodes[c]);
    EXPECT_EQ(nodes[c]->get_death(), nodes[d]);
    expect_left_spine(nodes[c]);

    EXPECT_EQ(nodes[d]->get_up(), nodes[j]);
    EXPECT_EQ(nodes[d]->get_down(), nodes[e]);
    EXPECT_EQ(nodes[d]->get_in(), nodes[c]);
    EXPECT_EQ(nodes[d]->get_mid(), nodes[c]);
    EXPECT_EQ(nodes[d]->get_low(), nodes[e]);
    EXPECT_TRUE(nodes[d]->is_on_in_trail());
    expect_left_spine(nodes[d]);

    EXPECT_EQ(nodes[e]->get_up(), nullptr);
    EXPECT_EQ(nodes[e]->get_down(), nullptr);
    EXPECT_EQ(nodes[e]->get_in(), nodes[d]);
    EXPECT_EQ(nodes[e]->get_mid(), nodes[f]);
    EXPECT_EQ(nodes[e]->get_low(), nodes[e]);
    EXPECT_EQ(nodes[e]->get_death(), nodes[j]);
    expect_not_on_spine(nodes[e]);

    EXPECT_EQ(nodes[f]->get_up(), nodes[h]);
    EXPECT_EQ(nodes[f]->get_down(), nodes[e]);
    EXPECT_EQ(nodes[f]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[f]->get_mid(), nodes[g]);
    EXPECT_EQ(nodes[f]->get_low(), nodes[e]);
    EXPECT_TRUE(nodes[f]->is_on_mid_trail());
    expect_not_on_spine(nodes[f]);

    EXPECT_EQ(nodes[g]->get_up(), nullptr);
    EXPECT_EQ(nodes[g]->get_down(), nullptr);
    EXPECT_EQ(nodes[g]->get_in(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_mid(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[g]);
    EXPECT_EQ(nodes[g]->get_death(), nodes[f]);
    expect_not_on_spine(nodes[g]);

    EXPECT_EQ(nodes[h]->get_up(), nodes[j]);
    EXPECT_EQ(nodes[h]->get_down(), nodes[f]);
    EXPECT_EQ(nodes[h]->get_in(), nodes[i]);
    EXPECT_EQ(nodes[h]->get_mid(), nodes[i]);
    EXPECT_EQ(nodes[h]->get_low(), nodes[e]);
    EXPECT_TRUE(nodes[h]->is_on_mid_trail());
    expect_not_on_spine(nodes[h]);

    EXPECT_EQ(nodes[i]->get_up(), nullptr);
    EXPECT_EQ(nodes[i]->get_down(), nullptr);
    EXPECT_EQ(nodes[i]->get_in(), nodes[h]);
    EXPECT_EQ(nodes[i]->get_mid(), nodes[h]);
    EXPECT_EQ(nodes[i]->get_low(), nodes[i]);
    EXPECT_EQ(nodes[i]->get_death(), nodes[h]);
    expect_not_on_spine(nodes[i]);

    EXPECT_EQ(nodes[j]->get_up(), special_root);
    EXPECT_EQ(nodes[j]->get_down(), nodes[n]);
    EXPECT_EQ(nodes[j]->get_in(), nodes[d]);
    EXPECT_EQ(nodes[j]->get_mid(), nodes[h]);
    EXPECT_EQ(nodes[j]->get_low(), nodes[o]);
    EXPECT_TRUE(nodes[j]->is_on_in_trail());
    expect_left_spine(nodes[j]);

    EXPECT_EQ(nodes[k]->get_up(), nullptr);
    EXPECT_EQ(nodes[k]->get_down(), nullptr);
    EXPECT_EQ(nodes[k]->get_in(), nodes[n]);
    EXPECT_EQ(nodes[k]->get_mid(), nodes[l]);
    EXPECT_EQ(nodes[k]->get_low(), nodes[k]);
    EXPECT_EQ(nodes[k]->get_death(), nodes[n]);
    expect_not_on_spine(nodes[k]);

    EXPECT_EQ(nodes[l]->get_up(), nodes[n]);
    EXPECT_EQ(nodes[l]->get_down(), nodes[k]);
    EXPECT_EQ(nodes[l]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[l]->get_mid(), nodes[m]);
    EXPECT_EQ(nodes[l]->get_low(), nodes[k]);
    EXPECT_TRUE(nodes[l]->is_on_mid_trail());
    expect_not_on_spine(nodes[l]);

    EXPECT_EQ(nodes[m]->get_up(), nullptr);
    EXPECT_EQ(nodes[m]->get_down(), nullptr);
    EXPECT_EQ(nodes[m]->get_in(), nodes[l]);
    EXPECT_EQ(nodes[m]->get_mid(), nodes[l]);
    EXPECT_EQ(nodes[m]->get_low(), nodes[m]);
    EXPECT_EQ(nodes[m]->get_death(), nodes[l]);
    expect_not_on_spine(nodes[m]);

    EXPECT_EQ(nodes[n]->get_up(), nodes[j]);
    EXPECT_EQ(nodes[n]->get_down(), nodes[o]);
    EXPECT_EQ(nodes[n]->get_in(), nodes[k]);
    EXPECT_EQ(nodes[n]->get_mid(), nodes[l]);
    EXPECT_EQ(nodes[n]->get_low(), nodes[o]);
    EXPECT_TRUE(nodes[n]->is_on_in_trail());
    expect_not_on_spine(nodes[n]);

    EXPECT_EQ(nodes[o]->get_up(), nullptr);
    EXPECT_EQ(nodes[o]->get_down(), nullptr);
    EXPECT_EQ(nodes[o]->get_in(), nodes[n]);
    EXPECT_EQ(nodes[o]->get_mid(), special_root);
    EXPECT_EQ(nodes[o]->get_low(), nodes[o]);
    EXPECT_EQ(nodes[o]->get_death(), special_root);
    expect_right_spine(nodes[o]);

    EXPECT_EQ(special_root->get_in(), nodes[j]);
    EXPECT_EQ(special_root->get_mid(), nodes[o]);
    EXPECT_EQ(special_root->get_low(), nodes[o]);
    expect_both_spines(special_root);
}
