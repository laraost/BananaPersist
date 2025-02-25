#pragma once

#include "datastructure/banana_tree.h"
#include "datastructure/interval.h"
#include "utility/recycling_object_pool.h"
#include <array>
#include <gtest/gtest.h>

#include "validation.h"

using namespace bananas;

class PaperUpTreeTest : public ::testing::Test {

    protected:
        constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6,j=7,k=8,l=9,m=10,n=11,o=12;

        inline PaperUpTreeTest() :
            items{list_item{c, 6},  list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
                  list_item{g, 8},  list_item{h, 4}, list_item{i, 7},  list_item{j, 1},
                  list_item{k, 11}, list_item{l, 9}, list_item{m, 10}, list_item{n, 3},
                  list_item{o, 13}},
            up_tree(up_node_pool)
        {
            list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
            list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
            list_item::link(items[g], items[h]); list_item::link(items[h], items[i]);
            list_item::link(items[i], items[j]); list_item::link(items[j], items[k]);
            list_item::link(items[k], items[l]); list_item::link(items[l], items[m]);
            list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);
            up_tree.construct(&items[c], &items[items.size() - 1]);
            for (size_t i = 0; i < items.size(); ++i) {
                nodes[i] = items[i].get_node<1>();
            }
            left_hook = up_tree.get_left_hook();
            right_hook = up_tree.get_right_hook();
            special_root = up_tree.get_special_root();
        }

        // A pool for allocating additional `list_item`s
        bananas::recycling_object_pool<bananas::list_item> item_pool;
        bananas::recycling_object_pool<bananas::banana_tree_node<1>> up_node_pool;
        std::array<list_item, 13> items;
        std::array<up_tree_node*, 13> nodes;
        banana_tree<1> up_tree;
        const up_tree_node* left_hook;
        const up_tree_node* right_hook;
        const up_tree_node* special_root;

};

class PaperUpTreeTestWithExtraWiggle : public ::testing::Test {

    protected:
        constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6, wmin=7, wmax=8,j=9,k=10,l=11,m=12,n=13,o=14;

        inline PaperUpTreeTestWithExtraWiggle() :
            items{list_item{c, 6},  list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
                  list_item{g, 8},  list_item{h, 4}, list_item{i, 7},
                  list_item{wmin, 1.5}, list_item{wmax, 2.5},
                  list_item{j, 1},
                  list_item{k, 11}, list_item{l, 9}, list_item{m, 10}, list_item{n, 3},
                  list_item{o, 13}},
            up_tree(up_node_pool)
        {
            list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
            list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
            list_item::link(items[g], items[h]); list_item::link(items[h], items[i]);
            list_item::link(items[i], items[wmin]); list_item::link(items[wmin], items[wmax]);
            list_item::link(items[wmax], items[j]); list_item::link(items[j], items[k]);
            list_item::link(items[k], items[l]); list_item::link(items[l], items[m]);
            list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);
            up_tree.construct(&items[c], &items[items.size() - 1]);
            for (size_t i = 0; i < items.size(); ++i) {
                nodes[i] = items[i].get_node<1>();
            }
            left_hook = up_tree.get_left_hook();
            right_hook = up_tree.get_right_hook();
            special_root = up_tree.get_special_root();
        }


        bananas::recycling_object_pool<bananas::banana_tree_node<1>> up_node_pool;
        std::array<list_item, 15> items;
        std::array<up_tree_node*, 15> nodes;
        banana_tree<1> up_tree;
        const up_tree_node* left_hook;
        const up_tree_node* right_hook;
        const up_tree_node* special_root;

};

class PaperDownTreeTest : public ::testing::Test {

    protected:
        constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6,j=7,k=8,l=9,m=10,n=11,o=12;

        inline PaperDownTreeTest() :
            items{list_item{c, 6},  list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
                  list_item{g, 8},  list_item{h, 4}, list_item{i, 7},  list_item{j, 1},
                  list_item{k, 11}, list_item{l, 9}, list_item{m, 10}, list_item{n, 3},
                  list_item{o, 13}},
            down_tree(down_node_pool)
        {
            list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
            list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
            list_item::link(items[g], items[h]); list_item::link(items[h], items[i]);
            list_item::link(items[i], items[j]); list_item::link(items[j], items[k]);
            list_item::link(items[k], items[l]); list_item::link(items[l], items[m]);
            list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);
            down_tree.construct(&items[c], &items[items.size() - 1]);
            for (size_t i = 0; i < items.size(); ++i) {
                nodes[i] = items[i].get_node<-1>();
            }
            special_root = down_tree.get_special_root();
        }
        
        bananas::recycling_object_pool<bananas::banana_tree_node<-1>> down_node_pool;
        std::array<list_item, 13> items;
        std::array<down_tree_node*, 13> nodes;
        banana_tree<-1> down_tree;
        const down_tree_node* special_root;

};

class PaperTreePairTest : public ::testing::Test {

    protected:
        constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6,j=7,k=8,l=9,m=10,n=11,o=12;

        inline PaperTreePairTest() :
            persistence(up_node_pool, down_node_pool),
            items{list_item{c, 6},  list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
                  list_item{g, 8},  list_item{h, 4}, list_item{i, 7},  list_item{j, 1},
                  list_item{k, 11}, list_item{l, 9}, list_item{m, 10}, list_item{n, 3},
                  list_item{o, 13}}
        {
            list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
            list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
            list_item::link(items[g], items[h]); list_item::link(items[h], items[i]);
            list_item::link(items[i], items[j]); list_item::link(items[j], items[k]);
            list_item::link(items[k], items[l]); list_item::link(items[l], items[m]);
            list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);
            persistence.construct(&items[c], &items[items.size() - 1]);
            for (size_t i = 0; i < items.size(); ++i) {
                up_nodes[i] = items[i].get_node<1>();
                down_nodes[i] = items[i].get_node< -1>();
            }
            up_special_root = persistence.get_up_tree_special_root();
            down_special_root = persistence.get_down_tree_special_root();
        }

        // A pool for allocating additional `list_item`s
        bananas::recycling_object_pool<bananas::list_item> item_pool;
        bananas::recycling_object_pool<bananas::banana_tree_node<1>> up_node_pool;
        bananas::recycling_object_pool<bananas::banana_tree_node< -1>> down_node_pool;
        persistence_data_structure persistence;
        std::array<list_item, 13> items;
        std::array<up_tree_node*, 13> up_nodes;
        std::array<down_tree_node*, 13> down_nodes;
        const up_tree_node* up_special_root;
        const down_tree_node* down_special_root;
};

class PaperIntervalTest : public ::testing::Test {

    protected:
        constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6,j=7,k=8,l=9,m=10,n=11,o=12;

        inline PaperIntervalTest() :
            items{list_item{c, 6},  list_item{d, 2}, list_item{e, 12}, list_item{f, 5},
                  list_item{g, 8},  list_item{h, 4}, list_item{i, 7},  list_item{j, 1},
                  list_item{k, 11}, list_item{l, 9}, list_item{m, 10}, list_item{n, 3},
                  list_item{o, 13}},
            interval(up_node_pool, down_node_pool)
        {
            list_item::link(items[c], items[d]); list_item::link(items[d], items[e]);
            list_item::link(items[e], items[f]); list_item::link(items[f], items[g]);
            list_item::link(items[g], items[h]); list_item::link(items[h], items[i]);
            list_item::link(items[i], items[j]); list_item::link(items[j], items[k]);
            list_item::link(items[k], items[l]); list_item::link(items[l], items[m]);
            list_item::link(items[m], items[n]); list_item::link(items[n], items[o]);
            interval.construct(&items[c], &items[items.size() - 1]);
            for (size_t i = 0; i < items.size(); ++i) {
                up_nodes[i] = items[i].get_node<1>();
                down_nodes[i] = items[i].get_node< -1>();
            }
        }

        recycling_object_pool<list_item> item_pool;
        bananas::recycling_object_pool<bananas::banana_tree_node<1>> up_node_pool;
        bananas::recycling_object_pool<bananas::banana_tree_node< -1>> down_node_pool;
        std::array<list_item, 13> items;
        bananas::interval interval;
        std::array<up_tree_node*, 13> up_nodes;
        std::array<down_tree_node*, 13> down_nodes;

};

inline void validate_paper_up_tree(std::array<list_item*, 13> &items,
                                   list_item* special_root_item,
                                   list_item* left_hook_item,
                                   list_item* right_hook_item) {
    const size_t c=0, d=1, e=2, f=3, g=4, h=5, i=6, j=7, k=8, l=9, m=10, n=11, o=12;
    EXPECT_up   (1, items[c], items[e]);
    EXPECT_down (1, items[c], items[d]);
    EXPECT_in   (1, items[c], left_hook_item);
    EXPECT_mid  (1, items[c], left_hook_item);
    EXPECT_low  (1, items[c], items[d]);
    EXPECT_death(1, items[c], nullptr);

    EXPECT_up   (1, items[d], nullptr);
    EXPECT_down (1, items[d], nullptr);
    EXPECT_in   (1, items[d], items[c]);
    EXPECT_mid  (1, items[d], items[e]);
    EXPECT_low  (1, items[d], items[d]);
    EXPECT_death(1, items[d], items[e]);

    EXPECT_up   (1, items[e], special_root_item);
    EXPECT_down (1, items[e], items[g]);
    EXPECT_in   (1, items[e], items[c]);
    EXPECT_mid  (1, items[e], items[d]);
    EXPECT_low  (1, items[e], items[j]);
    EXPECT_death(1, items[e], nullptr);

    EXPECT_up   (1, items[f], nullptr);
    EXPECT_down (1, items[f], nullptr);
    EXPECT_in   (1, items[f], items[g]);
    EXPECT_mid  (1, items[f], items[g]);
    EXPECT_low  (1, items[f], items[f]);
    EXPECT_death(1, items[f], items[g]);

    EXPECT_up   (1, items[g], items[e]);
    EXPECT_down (1, items[g], items[i]);
    EXPECT_in   (1, items[g], items[f]);
    EXPECT_mid  (1, items[g], items[f]);
    EXPECT_low  (1, items[g], items[j]);
    EXPECT_death(1, items[g], nullptr);

    EXPECT_up   (1, items[h], nullptr);
    EXPECT_down (1, items[h], nullptr);
    EXPECT_in   (1, items[h], items[i]);
    EXPECT_mid  (1, items[h], items[i]);
    EXPECT_low  (1, items[h], items[h]);
    EXPECT_death(1, items[h], items[i]);

    EXPECT_up   (1, items[i], items[g]);
    EXPECT_down (1, items[i], items[j]);
    EXPECT_in   (1, items[i], items[h]);
    EXPECT_mid  (1, items[i], items[h]);
    EXPECT_low  (1, items[i], items[j]);
    EXPECT_death(1, items[i], nullptr);

    EXPECT_up   (1, items[j], nullptr);
    EXPECT_down (1, items[j], nullptr);
    EXPECT_in   (1, items[j], items[i]);
    EXPECT_mid  (1, items[j], items[k]);
    EXPECT_low  (1, items[j], items[j]);
    EXPECT_death(1, items[j], special_root_item);

    EXPECT_up   (1, items[k], items[o]);
    EXPECT_down (1, items[k], items[j]);
    EXPECT_in   (1, items[k], items[n]);
    EXPECT_mid  (1, items[k], items[m]);
    EXPECT_low  (1, items[k], items[j]);
    EXPECT_death(1, items[k], nullptr);

    EXPECT_up   (1, items[l], nullptr);
    EXPECT_down (1, items[l], nullptr);
    EXPECT_in   (1, items[l], items[m]);
    EXPECT_mid  (1, items[l], items[m]);
    EXPECT_low  (1, items[l], items[l]);
    EXPECT_death(1, items[l], items[m]);

    EXPECT_up   (1, items[m], items[k]);
    EXPECT_down (1, items[m], items[n]);
    EXPECT_in   (1, items[m], items[l]);
    EXPECT_mid  (1, items[m], items[l]);
    EXPECT_low  (1, items[m], items[n]);
    EXPECT_death(1, items[m], nullptr);

    EXPECT_up   (1, items[n], nullptr);
    EXPECT_down (1, items[n], nullptr);
    EXPECT_in   (1, items[n], items[k]);
    EXPECT_mid  (1, items[n], items[m]);
    EXPECT_low  (1, items[n], items[n]);
    EXPECT_death(1, items[n], items[k]);

    EXPECT_up   (1, items[o], special_root_item);
    EXPECT_down (1, items[o], items[k]);
    EXPECT_in   (1, items[o], right_hook_item);
    EXPECT_mid  (1, items[o], right_hook_item);
    EXPECT_low  (1, items[o], items[j]);
    EXPECT_death(1, items[o], nullptr);
}
