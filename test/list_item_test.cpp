#include <gtest/gtest.h>

#include "datastructure/list_item.h"

using namespace bananas;

TEST(ListItem, CorrectNeighborsAfterLink) {
    auto item_1 = list_item(5.0);
    auto item_2 = list_item(5.0);

    list_item::link(item_1, item_2);
    EXPECT_EQ(item_1.left_neighbor(),  nullptr);
    EXPECT_EQ(item_1.right_neighbor(), &item_2);
    EXPECT_EQ(item_2.left_neighbor(),  &item_1);
    EXPECT_EQ(item_2.right_neighbor(), nullptr);
}

TEST(ListItem, CutsRightCorrectly) {
    auto item_1 = list_item(5.0);
    auto item_2 = list_item(5.0);
    list_item::link(item_1, item_2);

    auto cut_neighbor_1 = item_1.cut_right();

    EXPECT_EQ(item_1.right_neighbor(), nullptr);
    EXPECT_EQ(item_2.left_neighbor(),  nullptr);

    EXPECT_EQ(cut_neighbor_1, &item_2);
}

TEST(ListItem, CutsLeftCorrectly) {
    auto item_1 = list_item(5.0);
    auto item_2 = list_item(5.0);
    list_item::link(item_1, item_2);

    auto cut_neighbor_2 = item_2.cut_left();

    EXPECT_EQ(item_1.right_neighbor(), nullptr);
    EXPECT_EQ(item_2.left_neighbor(),  nullptr);

    EXPECT_EQ(cut_neighbor_2, &item_1);
}

TEST(ListItem, IdentifiesEndpoints) {
    auto item_1 = list_item(5.0);
    auto item_2 = list_item(5.0);
    auto item_3 = list_item(5.0);
    list_item::link(item_1, item_2);
    list_item::link(item_2, item_3);

    EXPECT_TRUE(item_1.is_left_endpoint());
    EXPECT_FALSE(item_1.is_right_endpoint());
    EXPECT_TRUE(item_1.is_endpoint());
    EXPECT_FALSE(item_1.is_internal());
    
    EXPECT_TRUE(item_2.is_internal());
    EXPECT_FALSE(item_2.is_left_endpoint());
    EXPECT_FALSE(item_2.is_right_endpoint());
    EXPECT_FALSE(item_2.is_endpoint());

    EXPECT_FALSE(item_3.is_left_endpoint());
    EXPECT_TRUE(item_3.is_right_endpoint());
    EXPECT_TRUE(item_3.is_endpoint());
    EXPECT_FALSE(item_3.is_internal());
}

TEST(ListItem, IdentifiesCriticality) {
    //                       ...
    //         /c\           ...
    //        /   \d\        ...
    //  a\   /       \       ...
    //    \b/         \e     ...
    //
    auto item_a = list_item(1.0);
    auto item_b = list_item(0.0);
    auto item_c = list_item(3.0);
    auto item_d = list_item(2.0);
    auto item_e = list_item(0.0);
    list_item::link(item_a, item_b);
    list_item::link(item_b, item_c);
    list_item::link(item_c, item_d);
    list_item::link(item_d, item_e);

    EXPECT_FALSE(item_a.is_maximum<1>());
    EXPECT_FALSE(item_a.is_noncritical<1>());
    EXPECT_FALSE(item_a.is_minimum<1>());
    EXPECT_FALSE(item_a.is_maximum<-1>());
    EXPECT_FALSE(item_a.is_noncritical<-1>());
    EXPECT_FALSE(item_a.is_minimum<-1>());
    EXPECT_TRUE(item_a.is_down_type<1>());
    EXPECT_TRUE(item_a.is_up_type<-1>());
    EXPECT_FALSE(item_a.is_down_type<-1>());
    EXPECT_FALSE(item_a.is_up_type<1>());
    EXPECT_FALSE(item_a.is_critical<1>());
    EXPECT_TRUE(item_a.is_critical<-1>());

    EXPECT_FALSE(item_e.is_maximum<1>());
    EXPECT_FALSE(item_e.is_noncritical<1>());
    EXPECT_FALSE(item_e.is_minimum<1>());
    EXPECT_FALSE(item_e.is_maximum<-1>());
    EXPECT_FALSE(item_e.is_noncritical<-1>());
    EXPECT_FALSE(item_e.is_minimum<-1>());
    EXPECT_TRUE(item_e.is_down_type<-1>());
    EXPECT_TRUE(item_e.is_up_type<1>());
    EXPECT_FALSE(item_e.is_down_type<1>());
    EXPECT_FALSE(item_e.is_up_type<-1>());
    EXPECT_TRUE(item_e.is_critical<1>());
    EXPECT_FALSE(item_e.is_critical<-1>());

    EXPECT_FALSE(item_b.is_maximum<1>());
    EXPECT_FALSE(item_b.is_noncritical<1>());
    EXPECT_TRUE(item_b.is_minimum<1>());
    EXPECT_TRUE(item_b.is_maximum<-1>());
    EXPECT_FALSE(item_b.is_noncritical<-1>());
    EXPECT_FALSE(item_b.is_minimum<-1>());
    EXPECT_FALSE(item_b.is_down_type<1>());
    EXPECT_FALSE(item_b.is_down_type<-1>());
    EXPECT_FALSE(item_b.is_up_type<1>());
    EXPECT_FALSE(item_b.is_up_type<-1>());
    EXPECT_TRUE(item_b.is_critical<1>());
    EXPECT_TRUE(item_b.is_critical<-1>());

    EXPECT_TRUE(item_c.is_maximum<1>());
    EXPECT_FALSE(item_c.is_noncritical<1>());
    EXPECT_FALSE(item_c.is_minimum<1>());
    EXPECT_FALSE(item_c.is_maximum<-1>());
    EXPECT_FALSE(item_c.is_noncritical<-1>());
    EXPECT_TRUE(item_c.is_minimum<-1>());
    EXPECT_TRUE(item_c.is_critical<1>());
    EXPECT_TRUE(item_c.is_critical<-1>());

    EXPECT_FALSE(item_d.is_maximum<1>());
    EXPECT_TRUE(item_d.is_noncritical<1>());
    EXPECT_FALSE(item_d.is_minimum<1>());
    EXPECT_FALSE(item_d.is_maximum<-1>());
    EXPECT_TRUE(item_d.is_noncritical<-1>());
    EXPECT_FALSE(item_d.is_minimum<-1>());
    EXPECT_FALSE(item_d.is_critical<1>());
    EXPECT_FALSE(item_d.is_critical<-1>());
}
