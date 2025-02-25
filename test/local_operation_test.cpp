#include <cstddef>

#include "gtest/gtest.h"

#include "datastructure/banana_tree.h"
#include "datastructure/dictionary.h"
#include "datastructure/list_item.h"

#include "example_trees/paper_tree.h"
#include "persistence_defs.h"
#include "test/validation.h"
#include "utility/recycling_object_pool.h"

using namespace bananas;

enum class trail_position {
    internal,
    top_of_in,
    top_of_mid
};

//
//
// Testing interchanges of maxima
//
//

// A setting where two maxima internal to the same trail change positions
// Node j increasing above node q.
// The template argument `pos` parameterizes where q is in relation to the upper end of its trail:
//  - `pos == internal`: q is on the same trail as its parent
//  - `pos == top_of_in`: q is the top of its parent's in-trail
//  - `pos == top_of_mid`: q is the top of its parent's mid-trail
// These are the cases 1 and 2 illustrated in the paper. See there for more details.
template<trail_position pos>
class MaxInterchangeInternalTest : public ::testing::Test {

    protected:
        MaxInterchangeInternalTest() :
            low_item(0),
            high_item(10),
            item_p(2),
            item_q(3),
            item_i(1),
            item_j(4),
            node_low(&low_item),
            node_high(&high_item),
            node_p(&item_p),
            node_q(&item_q),
            node_i(&item_i),
            node_j(&item_j)
        {
            if constexpr (pos == trail_position::internal) {
                node_high.set_pointers(nullptr, &node_q, nullptr, nullptr, nullptr, nullptr);
            } else if constexpr (pos == trail_position::top_of_in) {
                node_high.set_pointers(nullptr, nullptr, &node_q, nullptr, nullptr, nullptr);
            } else {
                node_high.set_pointers(nullptr, nullptr, nullptr, &node_q, nullptr, nullptr);
            }
            node_low.set_pointers(&node_j, nullptr, nullptr, nullptr, nullptr, nullptr);
            node_p.set_pointers(nullptr, nullptr, &node_q, &node_q, &node_p, &node_q);
            node_q.set_pointers(&node_high, &node_j, &node_p, &node_p, nullptr, nullptr);
            node_i.set_pointers(nullptr, nullptr, &node_j, &node_j, &node_i, &node_j);
            node_j.set_pointers(&node_q, &node_low, &node_i, &node_i, nullptr, nullptr);
            list_item::link(high_item, item_p);
            list_item::link(item_p, item_q);
            list_item::link(item_q, item_i);
            list_item::link(item_i, item_j);
            list_item::link(item_j, low_item);
        }

        list_item low_item;
        list_item high_item;
        list_item item_p;
        list_item item_q;
        list_item item_i;
        list_item item_j;
        up_tree_node node_low;
        up_tree_node node_high;
        up_tree_node node_p;
        up_tree_node node_q;
        up_tree_node node_i;
        up_tree_node node_j;
};

using MaxInterchangeInternalTest1 = MaxInterchangeInternalTest<trail_position::internal>;
using MaxInterchangeInternalTest2 = MaxInterchangeInternalTest<trail_position::top_of_in>;
using MaxInterchangeInternalTest3 = MaxInterchangeInternalTest<trail_position::top_of_mid>;

// The upper ends of two parallel bananas internal to the same trail interchange
// without the pairing changing.
// The upper maximum is not the upper end of a trail.
// This is case 1 in the paper.
TEST_F(MaxInterchangeInternalTest1, InterchangesInternalWithoutPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(1);
    item_p.assign_value(2);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_down(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_i);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), &node_i);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_q);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_j);
}

// The upper ends of two parallel bananas internal to the same trail interchange
// without the pairing changing.
// The upper maximum is the upper end of an in-trail.
// This is case 1 in the paper.
TEST_F(MaxInterchangeInternalTest2, InterchangesInternalWithoutPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(1);
    item_p.assign_value(2);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_in(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_i);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), &node_i);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_q);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_j);
}

// The upper ends of two parallel bananas internal to the same trail interchange
// without the pairing changing.
// The upper maximum is the upper end of a mid-trail.
// This is case 1 in the paper.
TEST_F(MaxInterchangeInternalTest3, InterchangesInternalWithoutPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(1);
    item_p.assign_value(2);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_mid(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_i);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), &node_i);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_q);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_j);
}

// The upper ends of two parallel bananas internal to the same trail interchange
// with the pairing changing.
// The upper maximum is not the upper end of a trail.
// This is case 2 in the paper.
TEST_F(MaxInterchangeInternalTest1, InterchangesInternalWithPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(2);
    item_p.assign_value(1);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_down(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_p);
    EXPECT_EQ(node_q.get_in(), &node_i);
    EXPECT_EQ(node_q.get_mid(), &node_i);
    EXPECT_EQ(node_q.get_low(), &node_p);

    EXPECT_EQ(node_p.get_in(), &node_j);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_j);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_p);
    EXPECT_EQ(node_j.get_mid(), &node_q);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_q);
    EXPECT_EQ(node_i.get_death(), &node_q);
}

// The upper ends of two parallel bananas internal to the same trail interchange
// with the pairing changing.
// The upper maximum is the upper end of an in-trail.
// This is case 2 in the paper.
TEST_F(MaxInterchangeInternalTest2, InterchangesInternalWithPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(2);
    item_p.assign_value(1);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_in(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_p);
    EXPECT_EQ(node_q.get_in(), &node_i);
    EXPECT_EQ(node_q.get_mid(), &node_i);
    EXPECT_EQ(node_q.get_low(), &node_p);

    EXPECT_EQ(node_p.get_in(), &node_j);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_j);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_p);
    EXPECT_EQ(node_j.get_mid(), &node_q);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_q);
    EXPECT_EQ(node_i.get_death(), &node_q);
}

// The upper ends of two parallel bananas internal to the same trail interchange
// with the pairing changing.
// The upper maximum is the upper end of a mid-trail.
// This is case 2 in the paper.
TEST_F(MaxInterchangeInternalTest3, InterchangesInternalWithPairingChange) {
    // Ensure `f(i) < f(p)`
    item_i.assign_value(2);
    item_p.assign_value(1);

    node_j.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_mid(), &node_j);

    EXPECT_EQ(node_q.get_up(), &node_j);
    EXPECT_EQ(node_q.get_down(), &node_p);
    EXPECT_EQ(node_q.get_in(), &node_i);
    EXPECT_EQ(node_q.get_mid(), &node_i);
    EXPECT_EQ(node_q.get_low(), &node_p);

    EXPECT_EQ(node_p.get_in(), &node_j);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_j);

    EXPECT_EQ(node_j.get_up(), &node_high);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_p);
    EXPECT_EQ(node_j.get_mid(), &node_q);

    EXPECT_EQ(node_i.get_in(), &node_q);
    EXPECT_EQ(node_i.get_mid(), &node_q);
    EXPECT_EQ(node_i.get_death(), &node_q);
}




// A setting where two maxima change positions, where one maximum (`q`)
// is at the top of the in-trail of the other (`j`).
// See MaxInterchangeInternalTest for details on `pos`, but replace q by j.
// This is the case illustrated in the bottom left of Figure 7 of the paper.
template<trail_position pos>
class MaxInterchangeTopInTrailTest : public ::testing::Test {

    protected:
        MaxInterchangeTopInTrailTest() :
            low_item(0),
            high_item(10),
            item_p(2),
            item_q(5),
            item_i(1),
            item_j(4),
            node_low(&low_item),
            node_high(&high_item),
            node_p(&item_p),
            node_q(&item_q),
            node_i(&item_i),
            node_j(&item_j)
        {
            if constexpr (pos == trail_position::internal) {
                node_high.set_pointers(nullptr, &node_j, nullptr, nullptr, nullptr, nullptr);
            } else if constexpr (pos == trail_position::top_of_in) {
                node_high.set_pointers(nullptr, nullptr, &node_j, nullptr, nullptr, nullptr);
            } else {
                node_high.set_pointers(nullptr, nullptr, nullptr, &node_j, nullptr, nullptr);
            }
            node_low.set_pointers(&node_j, nullptr, nullptr, nullptr, nullptr, nullptr);
            node_p.set_pointers(nullptr, nullptr, &node_q, &node_q, &node_p, &node_q);
            node_q.set_pointers(&node_j, &node_i, &node_p, &node_p, &node_i, nullptr);
            node_i.set_pointers(nullptr, nullptr, &node_q, &node_j, &node_i, &node_j);
            node_j.set_pointers(&node_high, &node_low, &node_q, &node_i, nullptr, nullptr);
            list_item::link(high_item, item_p);
            list_item::link(item_p, item_q);
            list_item::link(item_q, item_i);
            list_item::link(item_i, item_j);
            list_item::link(item_j, low_item);
        }

        list_item low_item;
        list_item high_item;
        list_item item_p;
        list_item item_q;
        list_item item_i;
        list_item item_j;
        up_tree_node node_low;
        up_tree_node node_high;
        up_tree_node node_p;
        up_tree_node node_q;
        up_tree_node node_i;
        up_tree_node node_j;

};

using MaxInterchangeTopInTrailTest1 = MaxInterchangeTopInTrailTest<trail_position::internal>;
using MaxInterchangeTopInTrailTest2 = MaxInterchangeTopInTrailTest<trail_position::top_of_in>;
using MaxInterchangeTopInTrailTest3 = MaxInterchangeTopInTrailTest<trail_position::top_of_mid>;

TEST_F(MaxInterchangeTopInTrailTest1, InterchangesTopOfInTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_down(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

TEST_F(MaxInterchangeTopInTrailTest2, InterchangesTopOfInTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_in(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

TEST_F(MaxInterchangeTopInTrailTest3, InterchangesTopOfInTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_mid(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

// A setting where two maxima change positions, where one maximum (`q`)
// is at the top of the mid-trail of the other (`j`).
// See MaxInterchangeInternalTest for details on `pos`, but replace q by j.
// This is case illustrated in the bottom right of Figure 7 of the paper.
template<trail_position pos>
class MaxInterchangeTopMidTrailTest : public ::testing::Test {

    protected:
        MaxInterchangeTopMidTrailTest() :
            low_item(0),
            high_item(10),
            item_p(2),
            item_q(5),
            item_i(1),
            item_j(4),
            node_low(&low_item),
            node_high(&high_item),
            node_p(&item_p),
            node_q(&item_q),
            node_i(&item_i),
            node_j(&item_j)
        {
            if constexpr (pos == trail_position::internal) {
                node_high.set_pointers(nullptr, &node_j, nullptr, nullptr, nullptr, nullptr);
            } else if constexpr (pos == trail_position::top_of_in) {
                node_high.set_pointers(nullptr, nullptr, &node_j, nullptr, nullptr, nullptr);
            } else {
                node_high.set_pointers(nullptr, nullptr, nullptr, &node_j, nullptr, nullptr);
            }
            node_low.set_pointers(&node_j, nullptr, nullptr, nullptr, nullptr, nullptr);
            node_p.set_pointers(nullptr, nullptr, &node_j, &node_q, &node_p, &node_j);
            node_q.set_pointers(&node_j, &node_p, &node_i, &node_i, &node_p, nullptr);
            node_i.set_pointers(nullptr, nullptr, &node_q, &node_q, &node_i, &node_q);
            node_j.set_pointers(&node_high, &node_low, &node_p, &node_q, nullptr, nullptr);
            list_item::link(high_item, item_p);
            list_item::link(item_p, item_q);
            list_item::link(item_q, item_i);
            list_item::link(item_i, item_j);
            list_item::link(item_j, low_item);
        }

        list_item low_item;
        list_item high_item;
        list_item item_p;
        list_item item_q;
        list_item item_i;
        list_item item_j;
        up_tree_node node_low;
        up_tree_node node_high;
        up_tree_node node_p;
        up_tree_node node_q;
        up_tree_node node_i;
        up_tree_node node_j;

};

using MaxInterchangeTopMidTrailTest1 = MaxInterchangeTopMidTrailTest<trail_position::internal>;
using MaxInterchangeTopMidTrailTest2 = MaxInterchangeTopMidTrailTest<trail_position::top_of_in>;
using MaxInterchangeTopMidTrailTest3 = MaxInterchangeTopMidTrailTest<trail_position::top_of_mid>;

TEST_F(MaxInterchangeTopMidTrailTest1, InterchangesTopOfMidTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_down(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

TEST_F(MaxInterchangeTopMidTrailTest2, InterchangesTopOfMidTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_in(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

TEST_F(MaxInterchangeTopMidTrailTest3, InterchangesTopOfMidTrailCorrectly) {
    node_q.max_interchange_with_parent();

    EXPECT_EQ(node_high.get_mid(), &node_q);

    EXPECT_EQ(node_q.get_up(), &node_high);
    EXPECT_EQ(node_q.get_down(), &node_j);
    EXPECT_EQ(node_q.get_in(), &node_p);
    EXPECT_EQ(node_q.get_mid(), &node_p);
    EXPECT_EQ(node_q.get_low(), nullptr);

    EXPECT_EQ(node_p.get_in(), &node_q);
    EXPECT_EQ(node_p.get_mid(), &node_q);
    EXPECT_EQ(node_p.get_death(), &node_q);

    EXPECT_EQ(node_j.get_up(), &node_q);
    EXPECT_EQ(node_j.get_down(), &node_low);
    EXPECT_EQ(node_j.get_in(), &node_i);
    EXPECT_EQ(node_j.get_mid(), &node_i);

    EXPECT_EQ(node_i.get_in(), &node_j);
    EXPECT_EQ(node_i.get_mid(), &node_j);
    EXPECT_EQ(node_i.get_death(), &node_j);
}

//
//
// Testing interchanges of minima
//
//

TEST(MinInterchange, SmallestMinInterchangeInTrail) {
    auto item_a = list_item{0, 1};
    auto item_b = list_item{1, 2};
    auto item_c = list_item{2, 0};
    list_item::link(item_a, item_b);
    list_item::link(item_b, item_c);

    auto up_node_pool = recycling_object_pool<up_tree_node>{};
    auto up_tree = banana_tree<1>{up_node_pool, &item_a, &item_c};
    auto* node_a = item_a.get_node<1>();
    const auto* node_b = item_b.get_node<1>();
    auto* node_c = item_c.get_node<1>();
    const auto* special_root = up_tree.get_special_root();

    item_a.assign_value(-1);
    node_a->min_interchange_below(node_c);

    EXPECT_EQ(node_a->get_up(), nullptr);
    EXPECT_EQ(node_a->get_down(), nullptr);
    EXPECT_EQ(node_a->get_in(), special_root);
    EXPECT_EQ(node_a->get_mid(), node_b);
    EXPECT_EQ(node_a->get_low(), node_a);
    EXPECT_EQ(node_a->get_death(), special_root);

    EXPECT_EQ(node_b->get_up(), special_root);
    EXPECT_EQ(node_b->get_down(), node_a);
    EXPECT_EQ(node_b->get_in(), node_c);
    EXPECT_EQ(node_b->get_mid(), node_c);
    EXPECT_EQ(node_b->get_low(), node_a);
    EXPECT_EQ(node_b->get_death(), nullptr);

    EXPECT_EQ(node_c->get_up(), nullptr);
    EXPECT_EQ(node_c->get_down(), nullptr);
    EXPECT_EQ(node_c->get_in(), node_b);
    EXPECT_EQ(node_c->get_mid(), node_b);
    EXPECT_EQ(node_c->get_low(), node_c);
    EXPECT_EQ(node_c->get_death(), node_b);

    EXPECT_EQ(special_root->get_in(), node_a);
    EXPECT_EQ(special_root->get_mid(), node_b);
    EXPECT_EQ(special_root->get_low(), node_a);
}

TEST(MinInterchange, SmallestMinInterchangeMidTrail) {
    auto item_a = list_item{0, 0};
    auto item_b = list_item{1, 2};
    auto item_c = list_item{2, 1};
    list_item::link(item_a, item_b);
    list_item::link(item_b, item_c);

    auto up_node_pool = recycling_object_pool<up_tree_node>{};
    auto up_tree = banana_tree<1>{up_node_pool, &item_a, &item_c};
    auto* node_a = item_a.get_node<1>();
    const auto* node_b = item_b.get_node<1>();
    auto* node_c = item_c.get_node<1>();
    const auto* special_root = up_tree.get_special_root();

    item_c.assign_value(-1);
    node_c->min_interchange_below(node_a);

    EXPECT_EQ(node_a->get_up(), nullptr);
    EXPECT_EQ(node_a->get_down(), nullptr);
    EXPECT_EQ(node_a->get_in(), node_b);
    EXPECT_EQ(node_a->get_mid(), node_b);
    EXPECT_EQ(node_a->get_low(), node_a);
    EXPECT_EQ(node_a->get_death(), node_b);

    EXPECT_EQ(node_b->get_up(), special_root);
    EXPECT_EQ(node_b->get_down(), node_c);
    EXPECT_EQ(node_b->get_in(), node_a);
    EXPECT_EQ(node_b->get_mid(), node_a);
    EXPECT_EQ(node_b->get_low(), node_c);
    EXPECT_EQ(node_b->get_death(), nullptr);

    EXPECT_EQ(node_c->get_up(), nullptr);
    EXPECT_EQ(node_c->get_down(), nullptr);
    EXPECT_EQ(node_c->get_in(), node_b);
    EXPECT_EQ(node_c->get_mid(), special_root);
    EXPECT_EQ(node_c->get_low(), node_c);
    EXPECT_EQ(node_c->get_death(), special_root);

    EXPECT_EQ(special_root->get_in(), node_b);
    EXPECT_EQ(special_root->get_mid(), node_c);
    EXPECT_EQ(special_root->get_low(), node_c);
}

TEST_F(PaperUpTreeTest, MinInterchangeFHDoesNothing) {
    items[f].assign_value(items[h].value<1>() - 0.1);

    nodes[f]->min_interchange_below(nodes[h]);

    EXPECT_EQ(nodes[f]->get_death(), nodes[g]);
    EXPECT_EQ(nodes[h]->get_death(), nodes[i]);
    EXPECT_EQ(nodes[i]->get_up(), nodes[g]);
    EXPECT_EQ(nodes[g]->get_down(), nodes[i]);
    EXPECT_EQ(nodes[i]->get_low(), nodes[j]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[j]);

    expect_not_on_spine(nodes[f]);
    expect_not_on_spine(nodes[g]);
    expect_not_on_spine(nodes[h]);
    expect_not_on_spine(nodes[i]);
}

TEST_F(PaperUpTreeTest, MinInterchangeFJ) {
    items[f].assign_value(items[j].value<1>() - 0.1);

    nodes[f]->min_interchange_below(nodes[j]);

    EXPECT_EQ(nodes[f]->get_up(), nullptr);
    EXPECT_EQ(nodes[f]->get_down(), nullptr);
    EXPECT_EQ(nodes[f]->get_in(), nodes[e]);
    EXPECT_EQ(nodes[f]->get_mid(), nodes[g]);
    EXPECT_EQ(nodes[f]->get_low(), nodes[f]);
    EXPECT_EQ(nodes[f]->get_death(), special_root);
    expect_not_on_spine(nodes[f]);

    EXPECT_EQ(nodes[g]->get_up(), nodes[k]);
    EXPECT_EQ(nodes[g]->get_down(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_in(), nodes[j]);
    EXPECT_EQ(nodes[g]->get_mid(), nodes[i]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[f]);
    expect_not_on_spine(nodes[g]);

    EXPECT_EQ(nodes[j]->get_up(), nullptr);
    EXPECT_EQ(nodes[j]->get_down(), nullptr);
    EXPECT_EQ(nodes[j]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[j]->get_mid(), nodes[i]);
    EXPECT_EQ(nodes[j]->get_low(), nodes[j]);
    EXPECT_EQ(nodes[j]->get_death(), nodes[g]);
    expect_not_on_spine(nodes[j]);
    
    EXPECT_EQ(nodes[e]->get_low(), nodes[f]);
    EXPECT_EQ(nodes[e]->get_down(), nodes[f]);
    expect_left_spine(nodes[e]);

    EXPECT_EQ(nodes[k]->get_low(), nodes[f]);
    EXPECT_EQ(nodes[k]->get_down(), nodes[g]);
    EXPECT_FALSE(nodes[k]->is_on_spine());
    expect_not_on_spine(nodes[k]);

    EXPECT_EQ(nodes[o]->get_low(), nodes[f]);
    expect_right_spine(nodes[o]);

    EXPECT_EQ(special_root->get_low(), nodes[f]);
    expect_both_spines(special_root);
}

TEST_F(PaperUpTreeTest, MinInterchangeNJ) {
    items[n].assign_value(items[j].value<1>() - 0.1);

    nodes[n]->min_interchange_below(nodes[j]);

    EXPECT_EQ(nodes[n]->get_up(), nullptr);
    EXPECT_EQ(nodes[n]->get_down(), nullptr);
    EXPECT_EQ(nodes[n]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[n]->get_mid(), nodes[o]);
    EXPECT_EQ(nodes[n]->get_low(), nodes[n]);
    EXPECT_EQ(nodes[n]->get_death(), special_root);
    expect_not_on_spine(nodes[n]);

    EXPECT_EQ(nodes[k]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[k]->get_down(), nodes[m]);
    EXPECT_EQ(nodes[k]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[k]->get_mid(), nodes[j]);
    EXPECT_EQ(nodes[k]->get_low(), nodes[n]);
    expect_not_on_spine(nodes[k]);

    EXPECT_EQ(nodes[m]->get_up(), nodes[k]);
    EXPECT_EQ(nodes[m]->get_down(), nodes[n]);
    EXPECT_EQ(nodes[m]->get_low(), nodes[n]);
    expect_not_on_spine(nodes[m]);

    EXPECT_EQ(nodes[j]->get_mid(), nodes[k]);
    EXPECT_EQ(nodes[j]->get_death(), nodes[k]);
    expect_not_on_spine(nodes[j]);
    
    EXPECT_EQ(nodes[o]->get_down(), nodes[n]);
    EXPECT_EQ(nodes[o]->get_low(), nodes[n]);
    expect_right_spine(nodes[o]);

    EXPECT_EQ(nodes[e]->get_low(), nodes[n]);
    expect_left_spine(nodes[e]);

    EXPECT_EQ(special_root->get_low(), nodes[n]);
    EXPECT_TRUE(special_root->is_on_both_spines());
    expect_both_spines(special_root);
}

TEST_F(PaperUpTreeTest, MinInterchangeLN) {
    items[l].assign_value(items[n].value<1>() - 0.1);

    nodes[l]->min_interchange_below(nodes[n]);

    EXPECT_EQ(nodes[k]->get_down(), nodes[j]);
    EXPECT_EQ(nodes[k]->get_up(), nodes[o]);
    EXPECT_EQ(nodes[k]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[k]->get_mid(), nodes[l]);
    EXPECT_EQ(nodes[k]->get_low(), nodes[j]);
    expect_not_on_spine(nodes[k]);

    EXPECT_EQ(nodes[l]->get_down(), nullptr);
    EXPECT_EQ(nodes[l]->get_up(), nullptr);
    EXPECT_EQ(nodes[l]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[l]->get_mid(), nodes[k]);
    EXPECT_EQ(nodes[l]->get_death(), nodes[k]);
    EXPECT_EQ(nodes[l]->get_low(), nodes[l]);
    expect_not_on_spine(nodes[l]);

    EXPECT_EQ(nodes[m]->get_down(), nodes[l]);
    EXPECT_EQ(nodes[m]->get_up(), nodes[k]);
    EXPECT_EQ(nodes[m]->get_in(), nodes[n]);
    EXPECT_EQ(nodes[m]->get_mid(), nodes[n]);
    EXPECT_EQ(nodes[m]->get_low(), nodes[l]);
    EXPECT_FALSE(nodes[m]->is_on_spine());
    expect_not_on_spine(nodes[m]);

    EXPECT_EQ(nodes[n]->get_down(), nullptr);
    EXPECT_EQ(nodes[n]->get_up(), nullptr);
    EXPECT_EQ(nodes[n]->get_in(), nodes[m]);
    EXPECT_EQ(nodes[n]->get_mid(), nodes[m]);
    EXPECT_EQ(nodes[n]->get_death(), nodes[m]);
    EXPECT_EQ(nodes[n]->get_low(), nodes[n]);
    expect_not_on_spine(nodes[n]);
}

//
//
// Testing sequences of interchanges of maxima
//
//

TEST_F(PaperUpTreeTest, MaxIncreaseIAboveKInterchangesOnce) {
    items[i].assign_value(items[k].value<1>() + 0.1);

    up_tree.on_increase_value_of_maximum(&items[i]);

    EXPECT_EQ(nodes[i]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[i]->get_down(), nodes[j]);
    EXPECT_EQ(nodes[i]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[i]->get_mid(), nodes[h]);
    expect_not_on_spine(nodes[i]);

    EXPECT_EQ(nodes[g]->get_up(), nodes[i]);
    EXPECT_EQ(nodes[g]->get_down(), nodes[h]);
    EXPECT_EQ(nodes[g]->get_in(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_mid(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[h]);
    expect_not_on_spine(nodes[g]);

    EXPECT_EQ(nodes[h]->get_in(), nodes[g]);
    EXPECT_EQ(nodes[h]->get_mid(), nodes[i]);
    EXPECT_EQ(nodes[h]->get_death(), nodes[i]);
    expect_not_on_spine(nodes[h]);

    EXPECT_EQ(nodes[k]->get_up(), nodes[o]);
    EXPECT_EQ(nodes[k]->get_down(), nodes[j]);
    expect_not_on_spine(nodes[k]);
}

TEST_F(PaperUpTreeTest, MaxIncreaseIAboveOReplacesGlobalMax) {
    items[i].assign_value(items[o].value<1>() + 0.1);

    up_tree.on_increase_value_of_maximum(&items[i]);

    EXPECT_EQ(nodes[i]->get_up(), special_root);
    EXPECT_EQ(nodes[i]->get_down(), nodes[j]);
    EXPECT_EQ(nodes[i]->get_in(), nodes[c]);
    EXPECT_EQ(nodes[i]->get_mid(), nodes[e]);
    EXPECT_EQ(nodes[i]->get_low(), nodes[j]);
    expect_left_spine(nodes[i]);

    EXPECT_EQ(nodes[g]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[g]->get_down(), nodes[h]);
    EXPECT_EQ(nodes[g]->get_in(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_mid(), nodes[f]);
    EXPECT_EQ(nodes[g]->get_low(), nodes[h]);
    expect_not_on_spine(nodes[g]);

    EXPECT_EQ(nodes[e]->get_up(), nodes[i]);
    EXPECT_EQ(nodes[e]->get_down(), nodes[d]);
    EXPECT_EQ(nodes[e]->get_in(), nodes[h]);
    EXPECT_EQ(nodes[e]->get_mid(), nodes[g]);
    EXPECT_EQ(nodes[e]->get_low(), nodes[d]);
    expect_not_on_spine(nodes[e]);

    EXPECT_EQ(nodes[d]->get_in(), nodes[c]);
    EXPECT_EQ(nodes[d]->get_mid(), nodes[e]);
    EXPECT_EQ(nodes[d]->get_death(), nodes[i]);
    expect_not_on_spine(nodes[d]);

    EXPECT_EQ(nodes[h]->get_in(), nodes[e]);
    EXPECT_EQ(nodes[h]->get_mid(), nodes[g]);
    EXPECT_EQ(nodes[h]->get_death(), nodes[e]);
    expect_not_on_spine(nodes[h]);

    EXPECT_EQ(special_root->get_in(), nodes[i]);
    EXPECT_EQ(special_root->get_mid(), nodes[o]);
    expect_both_spines(special_root);

    expect_left_spine(nodes[c]);
    expect_right_spine(nodes[o]);

    EXPECT_EQ(up_tree.get_global_max(), &items[i]);
}

//
//
// Testing coupling of interchanges between up- and down-tree
//
//

TEST_F(PaperTreePairTest, MaxIncreaseIAboveO) {
    items[i].assign_value(items[o].value<1>() + 0.1);

    persistence.on_increase_value_of_maximum(&items[i]);

    // Assertions for the up-tree
    EXPECT_EQ(up_nodes[i]->get_up(), up_special_root);
    EXPECT_EQ(up_nodes[i]->get_down(), up_nodes[j]);
    EXPECT_EQ(up_nodes[i]->get_in(), up_nodes[c]);
    EXPECT_EQ(up_nodes[i]->get_mid(), up_nodes[e]);
    EXPECT_EQ(up_nodes[i]->get_low(), up_nodes[j]);
    expect_left_spine(up_nodes[i]);

    EXPECT_EQ(up_nodes[g]->get_up(), up_nodes[e]);
    EXPECT_EQ(up_nodes[g]->get_down(), up_nodes[h]);
    EXPECT_EQ(up_nodes[g]->get_in(), up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_mid(), up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_low(), up_nodes[h]);
    expect_not_on_spine(up_nodes[g]);

    EXPECT_EQ(up_nodes[e]->get_up(), up_nodes[i]);
    EXPECT_EQ(up_nodes[e]->get_down(), up_nodes[d]);
    EXPECT_EQ(up_nodes[e]->get_in(), up_nodes[h]);
    EXPECT_EQ(up_nodes[e]->get_mid(), up_nodes[g]);
    EXPECT_EQ(up_nodes[e]->get_low(), up_nodes[d]);
    expect_not_on_spine(up_nodes[e]);

    EXPECT_EQ(up_nodes[d]->get_in(), up_nodes[c]);
    EXPECT_EQ(up_nodes[d]->get_mid(), up_nodes[e]);
    EXPECT_EQ(up_nodes[d]->get_death(), up_nodes[i]);
    expect_not_on_spine(up_nodes[d]);

    EXPECT_EQ(up_nodes[h]->get_in(), up_nodes[e]);
    EXPECT_EQ(up_nodes[h]->get_mid(), up_nodes[g]);
    EXPECT_EQ(up_nodes[h]->get_death(), up_nodes[e]);
    expect_not_on_spine(up_nodes[h]);

    EXPECT_EQ(up_special_root->get_in(), up_nodes[i]);
    EXPECT_EQ(up_special_root->get_mid(), up_nodes[o]);
    expect_both_spines(up_special_root);

    expect_left_spine(up_nodes[c]);
    expect_right_spine(up_nodes[o]);

    EXPECT_EQ(persistence.get_global_max(), &items[i]);

    // Assertions for the down-tree
    EXPECT_EQ(down_nodes[j]->get_up(), down_special_root);
    EXPECT_EQ(down_nodes[j]->get_down(), down_nodes[i]);
    EXPECT_EQ(down_nodes[j]->get_in(), down_nodes[o]);
    EXPECT_EQ(down_nodes[j]->get_mid(), down_nodes[n]);
    EXPECT_EQ(down_nodes[j]->get_low(), down_nodes[i]);
    expect_right_spine(down_nodes[j]);

    EXPECT_EQ(down_nodes[d]->get_up(), down_special_root);
    EXPECT_EQ(down_nodes[d]->get_down(), down_nodes[h]);
    EXPECT_EQ(down_nodes[d]->get_in(), down_nodes[c]);
    EXPECT_EQ(down_nodes[d]->get_mid(), down_nodes[c]);
    EXPECT_EQ(down_nodes[d]->get_low(), down_nodes[i]);
    expect_left_spine(down_nodes[d]);

    EXPECT_EQ(down_nodes[h]->get_up(), down_nodes[d]);
    EXPECT_EQ(down_nodes[h]->get_down(), down_nodes[i]);
    EXPECT_EQ(down_nodes[h]->get_in(), down_nodes[e]);
    EXPECT_EQ(down_nodes[h]->get_mid(), down_nodes[f]);
    EXPECT_EQ(down_nodes[h]->get_low(), down_nodes[i]);
    expect_not_on_spine(down_nodes[h]);

    EXPECT_EQ(down_nodes[e]->get_in(), down_nodes[h]);
    EXPECT_EQ(down_nodes[e]->get_mid(), down_nodes[f]);
    EXPECT_EQ(down_nodes[e]->get_death(), down_nodes[h]);
    expect_not_on_spine(down_nodes[e]);

    EXPECT_EQ(down_special_root->get_in(), down_nodes[d]);
    EXPECT_EQ(down_special_root->get_mid(), down_nodes[j]);
    EXPECT_EQ(down_special_root->get_low(), down_nodes[i]);
    expect_both_spines(down_special_root);

    expect_not_on_spine(down_nodes[n]);

    expect_right_spine(down_nodes[o]);
    expect_left_spine(down_nodes[c]);

    EXPECT_EQ(persistence.get_global_min(), &items[j]);
}

TEST_F(PaperTreePairTest, MaxDecreaseEBelowC) {
    items[e].assign_value(items[c].value<1>() - 0.1);

    persistence.on_decrease_value_of_maximum(&items[e]);

    // Assertions for the up-tree
    EXPECT_EQ(up_nodes[g]->get_up(), up_special_root);
    EXPECT_EQ(up_nodes[g]->get_down(), up_nodes[i]);
    EXPECT_EQ(up_nodes[g]->get_in(), up_nodes[c]);
    EXPECT_EQ(up_nodes[g]->get_mid(), up_nodes[e]);
    EXPECT_EQ(up_nodes[g]->get_low(), up_nodes[j]);
    expect_left_spine(up_nodes[g]);

    EXPECT_EQ(up_nodes[e]->get_up(), up_nodes[g]);
    EXPECT_EQ(up_nodes[e]->get_down(), up_nodes[d]);
    EXPECT_EQ(up_nodes[e]->get_in(), up_nodes[f]);
    EXPECT_EQ(up_nodes[e]->get_mid(), up_nodes[f]);
    EXPECT_EQ(up_nodes[e]->get_low(), up_nodes[d]);
    expect_not_on_spine(up_nodes[e]);

    EXPECT_EQ(up_nodes[d]->get_in(), up_nodes[c]);
    EXPECT_EQ(up_nodes[d]->get_mid(), up_nodes[e]);
    EXPECT_EQ(up_nodes[d]->get_death(), up_nodes[g]);
    expect_not_on_spine(up_nodes[d]);

    EXPECT_EQ(up_special_root->get_in(), up_nodes[g]);
    EXPECT_EQ(up_special_root->get_mid(), up_nodes[o]);
    EXPECT_EQ(up_special_root->get_low(), up_nodes[j]);
    expect_both_spines(up_special_root);

    expect_left_spine(up_nodes[c]);
    expect_right_spine(up_nodes[o]);

    // Assertions for the down-tree
    EXPECT_EQ(down_nodes[j]->get_up(), down_special_root);
    EXPECT_EQ(down_nodes[j]->get_down(), down_nodes[n]);
    EXPECT_EQ(down_nodes[j]->get_in(), down_nodes[d]);
    EXPECT_EQ(down_nodes[j]->get_mid(), down_nodes[h]);
    EXPECT_EQ(down_nodes[j]->get_low(), down_nodes[o]);
    expect_left_spine(down_nodes[j]);
    
    EXPECT_EQ(down_nodes[f]->get_up(), down_nodes[d]);
    EXPECT_EQ(down_nodes[f]->get_down(), down_nodes[g]);
    EXPECT_EQ(down_nodes[f]->get_in(), down_nodes[e]);
    EXPECT_EQ(down_nodes[f]->get_mid(), down_nodes[e]);
    EXPECT_EQ(down_nodes[f]->get_low(), down_nodes[g]);
    expect_not_on_spine(down_nodes[f]);

    EXPECT_EQ(down_nodes[g]->get_in(), down_nodes[f]);
    EXPECT_EQ(down_nodes[g]->get_mid(), down_nodes[h]);
    EXPECT_EQ(down_nodes[g]->get_death(), down_nodes[j]);
    expect_not_on_spine(down_nodes[g]);

    EXPECT_EQ(down_nodes[e]->get_in(), down_nodes[f]);
    EXPECT_EQ(down_nodes[e]->get_mid(), down_nodes[f]);
    EXPECT_EQ(down_nodes[e]->get_death(), down_nodes[f]);
    expect_not_on_spine(down_nodes[e]);

    EXPECT_EQ(down_special_root->get_in(), down_nodes[j]);
    EXPECT_EQ(down_special_root->get_mid(), down_nodes[o]);
    EXPECT_EQ(down_special_root->get_low(), down_nodes[o]);
    expect_both_spines(down_special_root);

    expect_left_spine(down_nodes[c]);
    expect_left_spine(down_nodes[d]);

    expect_not_on_spine(down_nodes[n]);

    expect_right_spine(down_nodes[o]);
}

TEST_F(PaperTreePairTest, MaxDecreaseOBelowM) {
    items[o].assign_value(items[n].value<1>() + 0.1);

    persistence.on_decrease_value_of_maximum(&items[o]);

    // Assertions for the up-tree
    EXPECT_EQ(up_nodes[o]->get_up(), up_nodes[k]);
    EXPECT_EQ(up_nodes[o]->get_down(), up_nodes[n]);
    EXPECT_EQ(up_nodes[o]->get_low(), up_nodes[n]);
    expect_right_spine(up_nodes[o]);

    EXPECT_EQ(up_nodes[k]->get_up(), up_special_root);
    EXPECT_EQ(up_nodes[k]->get_down(), up_nodes[j]);
    EXPECT_EQ(up_nodes[k]->get_in(), up_nodes[o]);
    EXPECT_EQ(up_nodes[k]->get_mid(), up_nodes[m]);
    expect_right_spine(up_nodes[k]);

    EXPECT_EQ(up_special_root->get_mid(), up_nodes[k]);
    expect_both_spines(up_special_root);

    expect_left_spine(up_nodes[e]);
    expect_left_spine(up_nodes[c]);

    // Assertions for the down-tree
    EXPECT_EQ(down_nodes[j]->get_up(), down_special_root);
    EXPECT_EQ(down_nodes[j]->get_down(), down_nodes[h]);
    EXPECT_EQ(down_nodes[j]->get_in(), down_nodes[n]);
    EXPECT_EQ(down_nodes[j]->get_mid(), down_nodes[k]);
    EXPECT_EQ(down_nodes[j]->get_low(), down_nodes[e]);
    expect_right_spine(down_nodes[j]);

    EXPECT_EQ(down_nodes[n]->get_up(), down_nodes[j]);
    EXPECT_EQ(down_nodes[n]->get_down(), down_nodes[l]);
    EXPECT_EQ(down_nodes[n]->get_in(), down_nodes[o]);
    EXPECT_EQ(down_nodes[n]->get_mid(), down_nodes[o]);
    EXPECT_EQ(down_nodes[n]->get_low(), down_nodes[k]);
    expect_right_spine(down_nodes[n]);

    EXPECT_EQ(down_nodes[k]->get_in(), down_nodes[l]);
    EXPECT_EQ(down_nodes[k]->get_mid(), down_nodes[j]);
    EXPECT_EQ(down_nodes[k]->get_death(), down_nodes[j]);
    expect_not_on_spine(down_nodes[k]);

    EXPECT_EQ(down_special_root->get_low(), down_nodes[e]);
    EXPECT_EQ(down_special_root->get_in(), down_nodes[d]);
    EXPECT_EQ(down_special_root->get_mid(), down_nodes[j]);
    expect_both_spines(down_special_root);

    expect_left_spine(down_nodes[c]);
    expect_left_spine(down_nodes[d]);

    EXPECT_EQ(persistence.get_global_max(), &items[e]);
}

TEST_F(PaperTreePairTest, MinIncreaseJAboveF) {
    items[j].assign_value(items[f].value<1>() + 0.1);

    persistence.on_increase_value_of_minimum(&items[j]);

    // Assertions for the up-tree
    EXPECT_EQ(up_nodes[e]->get_up(),   up_nodes[o]);
    EXPECT_EQ(up_nodes[e]->get_down(), up_nodes[d]);
    EXPECT_EQ(up_nodes[e]->get_in(),   up_nodes[n]);
    EXPECT_EQ(up_nodes[e]->get_mid(),  up_nodes[k]);
    EXPECT_EQ(up_nodes[e]->get_low(),  up_nodes[d]);
    expect_not_on_spine(up_nodes[e]);

    EXPECT_EQ(up_nodes[k]->get_up(),   up_nodes[e]);
    EXPECT_EQ(up_nodes[k]->get_down(), up_nodes[m]);
    EXPECT_EQ(up_nodes[k]->get_in(),   up_nodes[g]);
    EXPECT_EQ(up_nodes[k]->get_mid(),  up_nodes[i]);
    EXPECT_EQ(up_nodes[k]->get_low(),  up_nodes[n]);
    expect_not_on_spine(up_nodes[k]);

    EXPECT_EQ(up_nodes[g]->get_up(),   up_nodes[k]);
    EXPECT_EQ(up_nodes[g]->get_down(), up_nodes[h]);
    EXPECT_EQ(up_nodes[g]->get_in(),   up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_mid(),  up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_low(),  up_nodes[h]);
    expect_not_on_spine(up_nodes[g]);

    EXPECT_EQ(up_nodes[i]->get_up(),   up_nodes[k]);
    EXPECT_EQ(up_nodes[i]->get_down(), up_nodes[h]);
    EXPECT_EQ(up_nodes[i]->get_in(),   up_nodes[j]);
    EXPECT_EQ(up_nodes[i]->get_mid(),  up_nodes[j]);
    EXPECT_EQ(up_nodes[i]->get_low(),  up_nodes[h]);
    expect_not_on_spine(up_nodes[i]);

    EXPECT_EQ(up_nodes[f]->get_in(),    up_nodes[g]);
    EXPECT_EQ(up_nodes[f]->get_mid(),   up_nodes[g]);
    EXPECT_EQ(up_nodes[f]->get_death(), up_nodes[g]);
    expect_not_on_spine(up_nodes[f]);

    EXPECT_EQ(up_nodes[j]->get_in(),    up_nodes[i]);
    EXPECT_EQ(up_nodes[j]->get_mid(),   up_nodes[i]);
    EXPECT_EQ(up_nodes[j]->get_death(), up_nodes[i]);
    expect_not_on_spine(up_nodes[j]);

    EXPECT_EQ(up_nodes[h]->get_in(),    up_nodes[g]);
    EXPECT_EQ(up_nodes[h]->get_mid(),   up_nodes[i]);
    EXPECT_EQ(up_nodes[h]->get_death(), up_nodes[k]);
    expect_not_on_spine(up_nodes[h]);

    EXPECT_EQ(up_nodes[n]->get_in(),    up_nodes[e]);
    EXPECT_EQ(up_nodes[n]->get_mid(),   up_nodes[m]);
    EXPECT_EQ(up_nodes[n]->get_death(), up_nodes[e]);
    expect_not_on_spine(up_nodes[n]);

    EXPECT_EQ(up_special_root->get_in(), up_nodes[c]);
    EXPECT_EQ(up_special_root->get_mid(), up_nodes[o]);
    EXPECT_EQ(up_special_root->get_low(), up_nodes[d]);
    expect_both_spines(up_special_root);

    expect_left_spine(up_nodes[c]);
    expect_right_spine(up_nodes[o]);


    // Assertions for the down-tree
    EXPECT_EQ(down_nodes[d]->get_up(),   down_special_root);
    EXPECT_EQ(down_nodes[d]->get_down(), down_nodes[n]);
    EXPECT_EQ(down_nodes[d]->get_in(),   down_nodes[c]);
    EXPECT_EQ(down_nodes[d]->get_mid(),  down_nodes[c]);
    EXPECT_EQ(down_nodes[d]->get_low(),  down_nodes[o]);
    expect_left_spine(down_nodes[d]);

    EXPECT_EQ(down_nodes[n]->get_up(),   down_nodes[d]);
    EXPECT_EQ(down_nodes[n]->get_down(), down_nodes[o]);
    EXPECT_EQ(down_nodes[n]->get_in(),   down_nodes[e]);
    EXPECT_EQ(down_nodes[n]->get_mid(),  down_nodes[h]);
    EXPECT_EQ(down_nodes[n]->get_low(),  down_nodes[o]);
    expect_not_on_spine(down_nodes[n]);

    EXPECT_EQ(down_nodes[h]->get_up(),   down_nodes[n]);
    EXPECT_EQ(down_nodes[h]->get_down(), down_nodes[f]);
    EXPECT_EQ(down_nodes[h]->get_in(),   down_nodes[l]);
    EXPECT_EQ(down_nodes[h]->get_mid(),  down_nodes[j]);
    EXPECT_EQ(down_nodes[h]->get_low(),  down_nodes[e]);
    expect_not_on_spine(down_nodes[h]);

    EXPECT_EQ(down_nodes[j]->get_up(),   down_nodes[h]);
    EXPECT_EQ(down_nodes[j]->get_down(), down_nodes[k]);
    EXPECT_EQ(down_nodes[j]->get_in(),   down_nodes[i]);
    EXPECT_EQ(down_nodes[j]->get_mid(),  down_nodes[i]);
    EXPECT_EQ(down_nodes[j]->get_low(),  down_nodes[k]);
    expect_not_on_spine(down_nodes[j]);

    EXPECT_EQ(down_nodes[l]->get_up(),   down_nodes[h]);
    EXPECT_EQ(down_nodes[l]->get_down(), down_nodes[k]);
    EXPECT_EQ(down_nodes[l]->get_in(),   down_nodes[m]);
    EXPECT_EQ(down_nodes[l]->get_mid(),  down_nodes[m]);
    EXPECT_EQ(down_nodes[l]->get_low(),  down_nodes[k]);
    expect_not_on_spine(down_nodes[l]);

    EXPECT_EQ(down_nodes[e]->get_in(),    down_nodes[n]);
    EXPECT_EQ(down_nodes[e]->get_mid(),   down_nodes[f]);
    EXPECT_EQ(down_nodes[e]->get_death(), down_nodes[n]);
    expect_not_on_spine(down_nodes[e]);

    EXPECT_EQ(down_nodes[k]->get_in(),    down_nodes[l]);
    EXPECT_EQ(down_nodes[k]->get_mid(),   down_nodes[j]);
    EXPECT_EQ(down_nodes[k]->get_death(), down_nodes[h]);
    expect_not_on_spine(down_nodes[k]);

    EXPECT_EQ(down_special_root->get_in(),  down_nodes[d]);
    EXPECT_EQ(down_special_root->get_mid(), down_nodes[o]);
    EXPECT_EQ(down_special_root->get_low(), down_nodes[o]);
    expect_both_spines(down_special_root);

    expect_left_spine(down_nodes[c]);
    expect_right_spine(down_nodes[o]);

    EXPECT_EQ(persistence.get_global_min(), &items[d]);
}

TEST_F(PaperTreePairTest, MinDecreaseLBelowJ) {
    items[l].assign_value(items[j].value<1>() - 0.1);

    persistence.on_decrease_value_of_minimum(&items[l]);

    // Assertions for the up-tree
    EXPECT_EQ(up_nodes[m]->get_up(),   up_nodes[o]);
    EXPECT_EQ(up_nodes[m]->get_down(), up_nodes[l]);
    EXPECT_EQ(up_nodes[m]->get_in(),   up_nodes[n]);
    EXPECT_EQ(up_nodes[m]->get_mid(),  up_nodes[n]);
    EXPECT_EQ(up_nodes[m]->get_low(),  up_nodes[l]);
    expect_not_on_spine(up_nodes[m]);

    EXPECT_EQ(up_nodes[k]->get_up(),   up_nodes[e]);
    EXPECT_EQ(up_nodes[k]->get_down(), up_nodes[l]);
    EXPECT_EQ(up_nodes[k]->get_in(),   up_nodes[g]);
    EXPECT_EQ(up_nodes[k]->get_mid(),  up_nodes[j]);
    EXPECT_EQ(up_nodes[k]->get_low(),  up_nodes[l]);
    expect_not_on_spine(up_nodes[k]);

    EXPECT_EQ(up_nodes[g]->get_up(),   up_nodes[k]);
    EXPECT_EQ(up_nodes[g]->get_down(), up_nodes[i]);
    EXPECT_EQ(up_nodes[g]->get_in(),   up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_mid(),  up_nodes[f]);
    EXPECT_EQ(up_nodes[g]->get_low(),  up_nodes[j]);
    expect_not_on_spine(up_nodes[g]);

    EXPECT_EQ(up_nodes[i]->get_up(),   up_nodes[g]);
    EXPECT_EQ(up_nodes[i]->get_down(), up_nodes[j]);
    EXPECT_EQ(up_nodes[i]->get_in(),   up_nodes[h]);
    EXPECT_EQ(up_nodes[i]->get_mid(),  up_nodes[h]);
    EXPECT_EQ(up_nodes[i]->get_low(),  up_nodes[j]);
    expect_not_on_spine(up_nodes[i]);

    EXPECT_EQ(up_nodes[f]->get_in(),    up_nodes[g]);
    EXPECT_EQ(up_nodes[f]->get_mid(),   up_nodes[g]);
    EXPECT_EQ(up_nodes[f]->get_death(), up_nodes[g]);
    expect_not_on_spine(up_nodes[f]);

    EXPECT_EQ(up_nodes[j]->get_in(),    up_nodes[i]);
    EXPECT_EQ(up_nodes[j]->get_mid(),   up_nodes[k]);
    EXPECT_EQ(up_nodes[j]->get_death(), up_nodes[k]);
    expect_not_on_spine(up_nodes[j]);

    EXPECT_EQ(up_nodes[h]->get_in(),    up_nodes[i]);
    EXPECT_EQ(up_nodes[h]->get_mid(),   up_nodes[i]);
    EXPECT_EQ(up_nodes[h]->get_death(), up_nodes[i]);
    expect_not_on_spine(up_nodes[h]);

    EXPECT_EQ(up_nodes[n]->get_in(),    up_nodes[m]);
    EXPECT_EQ(up_nodes[n]->get_mid(),   up_nodes[m]);
    EXPECT_EQ(up_nodes[n]->get_death(), up_nodes[m]);
    expect_not_on_spine(up_nodes[n]);

    EXPECT_EQ(up_special_root->get_in(), up_nodes[e]);
    EXPECT_EQ(up_special_root->get_mid(), up_nodes[o]);
    EXPECT_EQ(up_special_root->get_low(), up_nodes[l]);
    expect_both_spines(up_special_root);

    expect_left_spine(up_nodes[e]);
    expect_left_spine(up_nodes[c]);
    expect_right_spine(up_nodes[o]);

    // Assertions for the down-tree
    
    EXPECT_EQ(down_nodes[l]->get_up(),   down_special_root);
    EXPECT_EQ(down_nodes[l]->get_down(), down_nodes[n]);
    EXPECT_EQ(down_nodes[l]->get_in(),   down_nodes[d]);
    EXPECT_EQ(down_nodes[l]->get_mid(),  down_nodes[j]);
    EXPECT_EQ(down_nodes[l]->get_low(),  down_nodes[o]);
    expect_left_spine(down_nodes[l]);

    EXPECT_EQ(down_nodes[n]->get_up(),   down_nodes[l]);
    EXPECT_EQ(down_nodes[n]->get_down(), down_nodes[o]);
    EXPECT_EQ(down_nodes[n]->get_in(),   down_nodes[m]);
    EXPECT_EQ(down_nodes[n]->get_mid(),  down_nodes[m]);
    EXPECT_EQ(down_nodes[n]->get_low(),  down_nodes[o]);
    expect_not_on_spine(down_nodes[n]);

    EXPECT_EQ(down_nodes[h]->get_up(),   down_nodes[j]);
    EXPECT_EQ(down_nodes[h]->get_down(), down_nodes[f]);
    EXPECT_EQ(down_nodes[h]->get_in(),   down_nodes[i]);
    EXPECT_EQ(down_nodes[h]->get_mid(),  down_nodes[i]);
    EXPECT_EQ(down_nodes[h]->get_low(),  down_nodes[e]);
    expect_not_on_spine(down_nodes[h]);

    EXPECT_EQ(down_nodes[j]->get_up(),   down_nodes[l]);
    EXPECT_EQ(down_nodes[j]->get_down(), down_nodes[h]);
    EXPECT_EQ(down_nodes[j]->get_in(),   down_nodes[k]);
    EXPECT_EQ(down_nodes[j]->get_mid(),  down_nodes[k]);
    EXPECT_EQ(down_nodes[j]->get_low(),  down_nodes[e]);
    expect_not_on_spine(down_nodes[j]);

    EXPECT_EQ(down_nodes[f]->get_up(),   down_nodes[h]);
    EXPECT_EQ(down_nodes[f]->get_down(), down_nodes[e]);
    EXPECT_EQ(down_nodes[f]->get_in(),   down_nodes[g]);
    EXPECT_EQ(down_nodes[f]->get_mid(),  down_nodes[g]);
    EXPECT_EQ(down_nodes[f]->get_low(),  down_nodes[e]);
    expect_not_on_spine(down_nodes[f]);

    EXPECT_EQ(down_nodes[e]->get_in(),    down_nodes[d]);
    EXPECT_EQ(down_nodes[e]->get_mid(),   down_nodes[f]);
    EXPECT_EQ(down_nodes[e]->get_death(), down_nodes[l]);
    expect_not_on_spine(down_nodes[e]);

    EXPECT_EQ(down_nodes[k]->get_in(),    down_nodes[j]);
    EXPECT_EQ(down_nodes[k]->get_mid(),   down_nodes[j]);
    EXPECT_EQ(down_nodes[k]->get_death(), down_nodes[j]);
    expect_not_on_spine(down_nodes[k]);

    EXPECT_EQ(down_special_root->get_in(),  down_nodes[l]);
    EXPECT_EQ(down_special_root->get_mid(), down_nodes[o]);
    EXPECT_EQ(down_special_root->get_low(), down_nodes[o]);
    expect_both_spines(down_special_root);

    expect_left_spine(down_nodes[d]);
    expect_left_spine(down_nodes[c]);
    
    EXPECT_EQ(persistence.get_global_min(), &items[l]);

}


//
//
// Testing cancellation
//
//

TEST_F(PaperUpTreeTest, CancelGFRemovesGF) {
    up_tree.cancel_maximum(&items[g]);

    EXPECT_EQ(nodes[i]->get_up(), nodes[e]);
    EXPECT_EQ(nodes[e]->get_down(), nodes[i]);
}

TEST_F(PaperUpTreeTest, CancelMLRemovesML) {
    up_tree.cancel_maximum(&items[m]);

    EXPECT_EQ(nodes[k]->get_mid(), nodes[n]);
    EXPECT_EQ(nodes[n]->get_mid(), nodes[k]);
}

using PaperUpTreeDeathTest = PaperUpTreeTest;

TEST_F(PaperUpTreeDeathTest, CancelEFails) {
    EXPECT_DEATH(up_tree.cancel_maximum(&items[e]),
        "Cancelled banana may not have nested bananas.");
}

TEST_F(PaperUpTreeDeathTest, CancelDFails) {
    EXPECT_DEATH(up_tree.cancel_maximum(&items[d]),
        "Cancelled item has to be a maximum");
}


//
//
// Testing anti-cancellations
//
//

TEST_F(PaperUpTreeDeathTest, AnticancelMaxMinPairFails) {
    auto min_item = list_item(6.3, 1.5);
    auto max_item = list_item(6.6, 2.5);
    items[i].cut_right();
    list_item::link(items[i], min_item);
    list_item::link(min_item, max_item);
    list_item::link(max_item, items[j]);

    EXPECT_DEATH(up_tree.anticancel(&items[i], {&max_item, &min_item}),
        "First item of item pair has to be a minimum.");

    EXPECT_DEATH(up_tree.anticancel(&items[i], {&min_item, &min_item}),
        "Second item of item pair has to be a maximum.");
}

TEST_F(PaperUpTreeDeathTest, AnticancelNextToMinFails) {
    auto min_item = list_item(6.3, 1.5);
    auto max_item = list_item(6.6, 2.5);
    items[i].cut_right();
    list_item::link(items[i], min_item);
    list_item::link(min_item, max_item);
    list_item::link(max_item, items[j]);

    EXPECT_DEATH(up_tree.anticancel(&items[j], {&min_item, &max_item}),
                 "Expected anticancel next to a maximum or down-type item.");
}

TEST_F(PaperUpTreeTest, AnticancelSucceeds) {
    auto* min_item = item_pool.construct(6.3, 1.5);
    auto* max_item = item_pool.construct(6.6, 2.5);
    items[i].cut_right();
    list_item::link(items[i], *min_item);
    list_item::link(*min_item, *max_item);
    list_item::link(*max_item, items[j]);

    up_tree.anticancel(&items[i], {min_item, max_item});

    EXPECT_up   (1, max_item, &items[i]);
    EXPECT_down (1, max_item, &items[j]);
    EXPECT_in   (1, max_item, min_item);
    EXPECT_mid  (1, max_item, min_item);
    EXPECT_death(1, max_item, nullptr);
    EXPECT_low  (1, max_item, &items[j]);

    EXPECT_up   (1, &items[h], nullptr);
    EXPECT_down (1, &items[h], nullptr);
    EXPECT_in   (1, &items[h], &items[i]);
    EXPECT_mid  (1, &items[h], &items[i]);
    EXPECT_death(1, &items[h], &items[i]);
    EXPECT_low  (1, &items[h], &items[h]);

    EXPECT_up  (1, &items[i], &items[g]);   
    EXPECT_down(1, &items[i], max_item);
    EXPECT_in  (1, &items[i], &items[h]);
    EXPECT_mid (1, &items[i], &items[h]);
    EXPECT_low (1, &items[i], &items[j]);

    EXPECT_in  (1, &items[j], max_item);
    EXPECT_mid (1, &items[j], &items[k]);
    EXPECT_low (1, &items[j], &items[j]);

    EXPECT_up   (1, min_item, nullptr);
    EXPECT_down (1, min_item, nullptr);
    EXPECT_in   (1, min_item, max_item);
    EXPECT_mid  (1, min_item, max_item);
    EXPECT_death(1, min_item, max_item);
    EXPECT_low  (1, min_item, min_item);
}

TEST_F(PaperTreePairTest, AnticancelSucceeds) {
    auto* min_item = item_pool.construct(6.3, 1.5);
    auto* max_item = item_pool.construct(6.6, 2.5);
    items[i].cut_right();
    list_item::link(items[i], *min_item);
    list_item::link(*min_item, *max_item);
    list_item::link(*max_item, items[j]);

    auto min_dict = min_dictionary{};
    auto max_dict = max_dictionary{};
    min_dict.insert_item(items[d]); min_dict.insert_item(items[f]);
    min_dict.insert_item(items[h]); min_dict.insert_item(items[j]);
    min_dict.insert_item(items[l]); min_dict.insert_item(items[n]);

    max_dict.insert_item(items[c]); max_dict.insert_item(items[e]);
    max_dict.insert_item(items[g]); max_dict.insert_item(items[i]);
    max_dict.insert_item(items[k]); max_dict.insert_item(items[m]);
    max_dict.insert_item(items[o]);

    persistence.anticancel(min_dict, max_dict, {min_item, max_item});

    auto *up_max_node = max_item->template get_node<1>();
    auto *up_min_node = min_item->template get_node<1>();
    auto *down_max_node = min_item->template get_node<-1>();
    auto *down_min_node = max_item->template get_node<-1>();

    EXPECT_EQ(up_max_node->get_up(),   up_nodes[i]);
    EXPECT_EQ(up_max_node->get_down(), up_nodes[j]);
    EXPECT_EQ(up_max_node->get_in(),   up_min_node);
    EXPECT_EQ(up_max_node->get_mid(),  up_min_node);
    EXPECT_EQ(up_max_node->get_low(),  up_nodes[j]);

    EXPECT_EQ(up_min_node->get_in(),    up_max_node);
    EXPECT_EQ(up_min_node->get_mid(),   up_max_node);
    EXPECT_EQ(up_min_node->get_death(), up_max_node);
    EXPECT_EQ(up_min_node->get_low(), up_min_node);

    EXPECT_EQ(down_max_node->get_up(),   down_nodes[j]);
    EXPECT_EQ(down_max_node->get_down(), down_nodes[h]);
    EXPECT_EQ(down_max_node->get_in(),   down_min_node);
    EXPECT_EQ(down_max_node->get_mid(),  down_min_node);
    EXPECT_EQ(down_max_node->get_low(),  down_nodes[e]);

    EXPECT_EQ(down_min_node->get_in(),    down_max_node);
    EXPECT_EQ(down_min_node->get_mid(),   down_max_node);
    EXPECT_EQ(down_min_node->get_death(), down_max_node);
    EXPECT_EQ(down_min_node->get_low(), down_min_node);
}

//
// Testing sequences of local operations
//

void test_value_change(interval &interval, list_item* item, function_value_type value, bool skip_hook_in_up) {
    interval.update_value(item, value);

    auto critical_items = interval.critical_items();
    validate_string_order(interval.get_up_tree(), critical_items.begin(), critical_items.end(), skip_hook_in_up);
    validate_spine_labels(interval.get_up_tree(), critical_items.begin(), critical_items.end());
    validate_string_order(interval.get_down_tree(), critical_items.begin(), critical_items.end(), !skip_hook_in_up);
    validate_spine_labels(interval.get_down_tree(), critical_items.begin(), critical_items.end());
}

TEST_F(PaperIntervalTest, IncreasingDMaintainsOrder) {
    test_value_change(interval, &items[d], 12.5, false);
}

TEST_F(PaperIntervalTest, DecreasingEMaintainsOrder) {
    test_value_change(interval, &items[e], 0.5, true);
}

TEST_F(PaperIntervalTest, IncreasingFMaintainsOrder) {
    test_value_change(interval, &items[f], 11.5, true);
}

TEST_F(PaperIntervalTest, DecreasingGMaintainsOrder) {
    test_value_change(interval, &items[g], 0.5, true);
}

TEST_F(PaperIntervalTest, IncreasingHMaintainsOrder) {
    test_value_change(interval, &items[h], 12.5, true);
}

TEST_F(PaperIntervalTest, DecreasingIMaintainsOrder) {
    test_value_change(interval, &items[i], 0.5, true);
}

TEST_F(PaperIntervalTest, IncreasingJMaintainsOrder) {
    test_value_change(interval, &items[j], 12.5, true);
}

TEST_F(PaperIntervalTest, DecreasingKMaintainsOrder) {
    test_value_change(interval, &items[k], 0.5, true);
}

TEST_F(PaperIntervalTest, IncreasingLMaintainsOrder) {
    test_value_change(interval, &items[l], 11.5, true);
}

TEST_F(PaperIntervalTest, DecreasingMMaintainsOrder) {
    test_value_change(interval, &items[m], 0.5, true);
}

TEST_F(PaperIntervalTest, IncreasingNMaintainsOrder) {
    test_value_change(interval, &items[n], 13.5, true);
}

TEST_F(PaperIntervalTest, DecreasingCMaintainsOrder) {
    test_value_change(interval, &items[c], 0.5, false);
}

TEST_F(PaperIntervalTest, DecreasingAndIncreasingCMaintainsOrder) {
    test_value_change(interval, &items[c], 1.5, false);
    test_value_change(interval, &items[c], 6, true);
}

TEST_F(PaperIntervalTest, DecreasingOMaintainsOrder) {
    test_value_change(interval, &items[o], 0.5, true);
}

TEST_F(PaperIntervalTest, DecreasingAndIncreasingOMaintainsOrder) {
    test_value_change(interval, &items[o], 1.5, true);
    test_value_change(interval, &items[o], 13, true);
}

TEST_F(PaperIntervalTest, DecreasingGSlightlyDoesNotInterchange) {
    test_value_change(interval, &items[g], 7.9, true);
}



// Testing insertions of items

TEST_F(PaperIntervalTest, InsertAtPosition) {
    auto item_d_e = interval.insert_item(1.5, item_pool); // Next to a minimum
    EXPECT_TRUE(item_d_e->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_d_e->left_neighbor(), &items[d]);
    EXPECT_ITEM_EQ(items[d].right_neighbor(), item_d_e);
    EXPECT_ITEM_EQ(item_d_e->right_neighbor(), &items[e]);
    EXPECT_ITEM_EQ(items[e].left_neighbor(), item_d_e);

    auto item_g_h = interval.insert_item(4.5, item_pool); // Next to a maximum
    EXPECT_TRUE(item_g_h->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_g_h->left_neighbor(), &items[g]);
    EXPECT_ITEM_EQ(items[g].right_neighbor(), item_g_h);
    EXPECT_ITEM_EQ(item_g_h->right_neighbor(), &items[h]);
    EXPECT_ITEM_EQ(items[h].left_neighbor(), item_g_h);

    auto item_gh_h = interval.insert_item(4.75, item_pool); // Next to a non-critical item
    EXPECT_TRUE(item_gh_h->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_gh_h->left_neighbor(), item_g_h);
    EXPECT_ITEM_EQ(item_g_h->right_neighbor(), item_gh_h);
    EXPECT_ITEM_EQ(item_gh_h->right_neighbor(), &items[h]);
    EXPECT_ITEM_EQ(items[h].left_neighbor(), item_gh_h);
}

TEST_F(PaperIntervalTest, InsertNextToItem) {
    auto item_d_e = interval.insert_item_to_right_of(&items[d], item_pool); // Next to a minimum
    EXPECT_TRUE(item_d_e->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_d_e->left_neighbor(), &items[d]);
    EXPECT_ITEM_EQ(items[d].right_neighbor(), item_d_e);
    EXPECT_ITEM_EQ(item_d_e->right_neighbor(), &items[e]);
    EXPECT_ITEM_EQ(items[e].left_neighbor(), item_d_e);

    auto item_g_h = interval.insert_item_to_right_of(&items[g], item_pool); // Next to a maximum
    EXPECT_TRUE(item_g_h->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_g_h->left_neighbor(), &items[g]);
    EXPECT_ITEM_EQ(items[g].right_neighbor(), item_g_h);
    EXPECT_ITEM_EQ(item_g_h->right_neighbor(), &items[h]);
    EXPECT_ITEM_EQ(items[h].left_neighbor(), item_g_h);

    auto item_gh_h = interval.insert_item_to_right_of(item_g_h, item_pool); // Next to a non-critical item
    EXPECT_TRUE(item_gh_h->is_noncritical<1>());
    EXPECT_ITEM_EQ(item_gh_h->left_neighbor(), item_g_h);
    EXPECT_ITEM_EQ(item_g_h->right_neighbor(), item_gh_h);
    EXPECT_ITEM_EQ(item_gh_h->right_neighbor(), &items[h]);
    EXPECT_ITEM_EQ(items[h].left_neighbor(), item_gh_h);
}

TEST_F(PaperIntervalTest, InsertLeftEndpointWithoutCriticalityChange) {
    auto new_end = interval.insert_left_endpoint(6.1, 1, item_pool);
    EXPECT_TRUE(new_end->is_down_type<1>());
    EXPECT_TRUE(new_end->is_endpoint());
    EXPECT_TRUE(new_end->is_left_endpoint());
    EXPECT_ITEM_EQ(new_end->right_neighbor(), &items[c]);

    auto up_left_hook = interval.get_up_tree().get_left_hook()->get_item();

    // Check up-tree structure
    EXPECT_up(1, new_end, &items[e]);
    EXPECT_down(1, new_end, &items[d]);
    EXPECT_low(1, new_end, &items[d]);
    EXPECT_in(1, new_end, up_left_hook);
    EXPECT_mid(1, new_end, up_left_hook);
    EXPECT_in(1, &items[d], new_end);
    EXPECT_in(1, &items[e], new_end);

    // Check down-tree structure
    EXPECT_low(-1, new_end, new_end);
    EXPECT_death(-1, new_end, &items[d]);
    EXPECT_in(-1, new_end, &items[d]);
    EXPECT_mid(-1, new_end, &items[d]);
    EXPECT_in(-1, &items[d], new_end);
    EXPECT_mid(-1, &items[d], new_end);
}

TEST_F(PaperIntervalTest, InsertLeftEndpointWithCriticalityChange) {
    auto new_end = interval.insert_left_endpoint(5.9, 1, item_pool);
    EXPECT_TRUE(new_end->is_up_type<1>());
    EXPECT_TRUE(new_end->is_endpoint());
    EXPECT_TRUE(new_end->is_left_endpoint());
    EXPECT_ITEM_EQ(new_end->right_neighbor(), &items[c]);

    EXPECT_EQ(interval.get_up_tree().get_left_hook(), nullptr);
    auto down_left_hook = interval.get_down_tree().get_left_hook()->get_item();

    // Check up-tree structure
    EXPECT_low(1, new_end, new_end);
    EXPECT_death(1, new_end, &items[c]);
    EXPECT_in(1, new_end, &items[c]);
    EXPECT_mid(1, new_end, &items[c]);
    EXPECT_in(1, &items[c], new_end);
    EXPECT_mid(1, &items[c], new_end);

    // Check down-tree structure
    EXPECT_up(-1, new_end, &items[d]);
    EXPECT_down(-1, new_end, &items[c]);
    EXPECT_low(-1, new_end, &items[c]);
    EXPECT_in(-1, new_end, down_left_hook);
    EXPECT_mid(-1, new_end, down_left_hook);
    EXPECT_in(-1, &items[c], new_end);
    EXPECT_in(-1, &items[d], new_end);
}

TEST_F(PaperIntervalTest, InsertRightEndpointWithoutCriticalityChange) {
    auto new_end = interval.insert_right_endpoint(13.1, 1, item_pool);
    EXPECT_TRUE(new_end->is_down_type<1>());
    EXPECT_TRUE(new_end->is_endpoint());
    EXPECT_TRUE(new_end->is_right_endpoint());
    EXPECT_ITEM_EQ(new_end->left_neighbor(), &items[o]);

    auto up_right_hook = interval.get_up_tree().get_right_hook()->get_item();
    auto up_special_root = interval.get_up_tree().get_special_root()->get_item();
    auto down_special_root = interval.get_down_tree().get_special_root()->get_item();

    // Check up-tree structure
    EXPECT_up(1, new_end, up_special_root);
    EXPECT_down(1, new_end, &items[k]);
    EXPECT_low(1, new_end, &items[j]);
    EXPECT_in(1, new_end, up_right_hook);
    EXPECT_mid(1, new_end, up_right_hook);
    EXPECT_mid(1, up_special_root, new_end);

    // Check down-tree structure
    EXPECT_low(-1, new_end, new_end);
    EXPECT_death(-1, new_end, down_special_root);
    EXPECT_in(-1, new_end, &items[n]);
    EXPECT_mid(-1, new_end, down_special_root);
    EXPECT_mid(-1, down_special_root, new_end);
}

TEST_F(PaperIntervalTest, InsertRightEndpointWithCriticalityChange) {
    auto new_end = interval.insert_right_endpoint(12.9, 1, item_pool);
    EXPECT_TRUE(new_end->is_up_type<1>());
    EXPECT_TRUE(new_end->is_endpoint());
    EXPECT_TRUE(new_end->is_right_endpoint());
    EXPECT_ITEM_EQ(new_end->left_neighbor(), &items[o]);

    EXPECT_EQ(interval.get_up_tree().get_right_hook(), nullptr);
    auto down_right_hook = interval.get_down_tree().get_right_hook()->get_item();
    auto down_special_root = interval.get_down_tree().get_special_root()->get_item();

    // Check up-tree structure
    EXPECT_low(1, new_end, new_end);
    EXPECT_death(1, new_end, &items[o]);
    EXPECT_in(1, new_end, &items[o]);
    EXPECT_mid(1, new_end, &items[o]);
    EXPECT_in(1, &items[o], new_end);
    EXPECT_mid(1, &items[o], new_end);

    // Check down-tree structure
    EXPECT_up(-1, new_end, down_special_root);
    EXPECT_down(-1, new_end, &items[o]);
    EXPECT_low(-1, new_end, &items[o]);
    EXPECT_in(-1, new_end, down_right_hook);
    EXPECT_mid(-1, new_end, down_right_hook);
    EXPECT_mid(-1, &items[o], new_end);
    EXPECT_mid(-1, down_special_root, new_end);
}

TEST_F(PaperIntervalTest, DeleteMinimum) {
    interval.delete_internal_item(&items[f]);
    EXPECT_TRUE(items[g].is_noncritical<1>());
    EXPECT_up(1, &items[i], &items[e]);
    EXPECT_down(1, &items[e], &items[i]);
    EXPECT_ITEM_EQ(items[e].right_neighbor(), &items[g]);
    EXPECT_ITEM_EQ(items[g].left_neighbor(), &items[e]);
}

TEST_F(PaperIntervalTest, DeleteMaximum) {
    interval.delete_internal_item(&items[g]);
    EXPECT_TRUE(items[f].is_noncritical<1>());
    EXPECT_up(1, &items[i], &items[e]);
    EXPECT_down(1, &items[e], &items[i]);
    EXPECT_ITEM_EQ(items[f].right_neighbor(), &items[h]);
    EXPECT_ITEM_EQ(items[h].left_neighbor(), &items[f]);
}

TEST_F(PaperIntervalTest, DeleteNoncritical) {
    auto nc_item = interval.insert_item_to_right_of(&items[i], item_pool);
    interval.delete_internal_item(nc_item);
    EXPECT_in(1, &items[j], &items[i]);
    EXPECT_down(1, &items[i], &items[j]);
    EXPECT_ITEM_EQ(items[i].right_neighbor(), &items[j]);
    EXPECT_ITEM_EQ(items[j].left_neighbor(), &items[i]);
}

TEST_F(PaperIntervalTest, DeleteRightEndpoint) {
    interval.delete_right_endpoint();
    auto* up_special_root = interval.get_up_tree().get_special_root()->get_item();

    EXPECT_ITEM_EQ(interval.get_right_endpoint(), &items[n]);
    EXPECT_up(1, &items[k], up_special_root);
    EXPECT_mid(1, up_special_root, &items[k]);

    auto crit_iter = interval.critical_items();
    validate_string_order(interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), false);
}

TEST_F(PaperIntervalTest, DeleteLeftEndpoint) {
    interval.delete_left_endpoint();

    EXPECT_ITEM_EQ(interval.get_left_endpoint(), &items[d]);
    EXPECT_in(1, &items[d], &items[e]);
    EXPECT_in(1, &items[e], &items[d]);

    auto crit_iter = interval.critical_items();
    validate_string_order(interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), true);
}
