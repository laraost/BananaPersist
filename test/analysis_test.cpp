#include <gtest/gtest.h>

#include "algorithms/banana_tree_algorithms.h"
#include "example_trees/paper_tree.h"

using namespace bananas;

TEST_F(PaperUpTreeTest, MapDFSComputesCorrectNodeDepth) {
    std::unordered_map<const up_tree_node*, int> node_depth_map;
    map_banana_dfs(up_tree, [&node_depth_map](auto*, auto* max_node, int, int node_depth) {
        node_depth_map.insert({max_node, node_depth});
    });

    EXPECT_EQ(node_depth_map.size(), 8);
    EXPECT_EQ(node_depth_map.at(special_root), 0);
    EXPECT_EQ(node_depth_map.at(nodes[c]), 2);
    EXPECT_EQ(node_depth_map.at(nodes[e]), 1);
    EXPECT_EQ(node_depth_map.at(nodes[g]), 2);
    EXPECT_EQ(node_depth_map.at(nodes[i]), 3);
    EXPECT_EQ(node_depth_map.at(nodes[k]), 2);
    EXPECT_EQ(node_depth_map.at(nodes[m]), 3);
    EXPECT_EQ(node_depth_map.at(nodes[o]), 1);

}

TEST_F(PaperUpTreeTest, MapDFSComputesCorrectNestingDepth) {
    std::unordered_map<const up_tree_node*, int> nesting_depth_map;
    map_banana_dfs(up_tree, [&nesting_depth_map](auto*, auto* max_node, int nesting_depth, int) {
        nesting_depth_map.insert({max_node, nesting_depth});
    });

    EXPECT_EQ(nesting_depth_map.size(), 8);
    EXPECT_EQ(nesting_depth_map.at(special_root), 0);
    EXPECT_EQ(nesting_depth_map.at(nodes[c]), 2);
    EXPECT_EQ(nesting_depth_map.at(nodes[e]), 1);
    EXPECT_EQ(nesting_depth_map.at(nodes[g]), 1);
    EXPECT_EQ(nesting_depth_map.at(nodes[i]), 1);
    EXPECT_EQ(nesting_depth_map.at(nodes[k]), 1);
    EXPECT_EQ(nesting_depth_map.at(nodes[m]), 2);
    EXPECT_EQ(nesting_depth_map.at(nodes[o]), 1);

}
