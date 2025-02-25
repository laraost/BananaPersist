#include <array>
#include <gtest/gtest.h>

#include "datastructure/banana_tree.h"
#include "datastructure/interval.h"
#include "datastructure/list_item.h"
#include "utility/recycling_object_pool.h"

using namespace bananas;

TEST(IntervalTest, CorrectForwardIteration) {
    std::array<list_item, 5> item_array = {
        list_item{1, 0},
        list_item{2, 1},
        list_item{3, 0.2},
        list_item{4, 1.2},
        list_item{5, 0.3}
    };
    list_item::link(item_array[0], item_array[1]);
    list_item::link(item_array[1], item_array[2]);
    list_item::link(item_array[2], item_array[3]);
    list_item::link(item_array[3], item_array[4]);
    recycling_object_pool<up_tree_node> up_node_pool;
    recycling_object_pool<down_tree_node> down_node_pool;
    auto the_interval = interval{up_node_pool, down_node_pool, &item_array[0], &item_array[4]};

    size_t index = 0;
    for (auto &item: the_interval) {
        EXPECT_EQ(&item, &item_array[index]);
        index++;
    }
}

TEST(IntervalTest, CorrectBackwardIteration) {
    std::array<list_item, 5> item_array = {
        list_item{1, 0},
        list_item{2, 1},
        list_item{3, 0.2},
        list_item{4, 1.2},
        list_item{5, 0.3}
    };
    list_item::link(item_array[0], item_array[1]);
    list_item::link(item_array[1], item_array[2]);
    list_item::link(item_array[2], item_array[3]);
    list_item::link(item_array[3], item_array[4]);
    recycling_object_pool<up_tree_node> up_node_pool;
    recycling_object_pool<down_tree_node> down_node_pool;
    auto the_interval = interval{up_node_pool, down_node_pool, &item_array[0], &item_array[4]};

    size_t index = 4;
    for (auto begin = the_interval.rbegin(); begin != the_interval.rend(); ++begin) {
        EXPECT_EQ(&(*begin), &item_array[index]);
        --index;
    }
}

TEST(IntervalTest, IteratorsCompare) {
    std::array<list_item, 5> item_array = {
        list_item{1, 0},
        list_item{2, 1},
        list_item{3, 0.2},
        list_item{4, 1.2},
        list_item{5, 0.3}
    };
    list_item::link(item_array[0], item_array[1]);
    list_item::link(item_array[1], item_array[2]);
    list_item::link(item_array[2], item_array[3]);
    list_item::link(item_array[3], item_array[4]);
    recycling_object_pool<up_tree_node> up_node_pool;
    recycling_object_pool<down_tree_node> down_node_pool;
    auto the_interval = interval{up_node_pool, down_node_pool, &item_array[0], &item_array[4]};

    auto forward_begin = the_interval.begin();
    auto backward_begin = the_interval.rbegin();

    EXPECT_NE(forward_begin, backward_begin);

    EXPECT_EQ(forward_begin, interval::iterator_to(item_array[0]));
    EXPECT_NE(forward_begin, interval::r_iterator_to(item_array[0]));

    EXPECT_NE(backward_begin, interval::iterator_to(item_array[4]));
    EXPECT_EQ(backward_begin, interval::r_iterator_to(item_array[4]));

}