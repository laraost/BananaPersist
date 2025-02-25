#include <functional>
#include <gtest/gtest.h>

#include "datastructure/banana_tree.h"
#include "datastructure/interval.h"
#include "datastructure/persistence_context.h"
#include "persistence_defs.h"
#include "validation.h"

using namespace bananas;

[[maybe_unused]] constexpr size_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4,
                                  i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9,
                                  i10 = 10, i11 = 11, i12 = 12, i13 = 13, i14 = 14;

// A test-case for debugging random walk with random changes, 10 points, seed 2329275342
class InstanceA : public ::testing::Test {
    
protected:
    inline InstanceA() : values({0, -0.5, -4.5, -6.5, -8.5, -9.5, -10.5, -14.5, -15.5, -12.5}) {
        the_interval = the_context.new_interval(values, {std::ref(items)});
        up_nodes.resize(items.size());
        down_nodes.resize(items.size());
        update_node_vector();
    }

    // First value change, leading to no change in the banana trees
    void perform_first_value_change() {
        values[i1] = -3;
        the_context.change_value(the_interval, items[i1], values[i1]);
        update_node_vector();
    }

    // Anti-cancellation part of the second value change, introducing the banana (1,2)
    void perform_second_value_change_ac() {
        values[i2] = -1;
        the_context.change_value(the_interval, items[i2], values[i2]);
        update_node_vector();
    }

    // Interchange part of the second value change, leading to interchange of 2 and 0
    void perform_second_value_change_xchange() {
        values[i2] = 0.5;
        the_context.change_value(the_interval, items[i2], values[i2]);
        update_node_vector();
    }

    void update_node_vector() {
        for (size_t i = 0; i < items.size(); ++i) {
            up_nodes[i] = items[i]->get_node<1>();
            down_nodes[i] = items[i]->get_node<-1>();
        }
    }

    std::vector<function_value_type> values;
    std::vector<list_item*> items;
    std::vector<up_tree_node*> up_nodes; 
    std::vector<down_tree_node*> down_nodes; 
    persistence_context the_context;
    interval* the_interval;
};

TEST_F(InstanceA, ConstructsCorrectly) {
    auto* up_special_root = the_interval->get_up_tree().get_special_root();
    auto* up_left_hook = the_interval->get_up_tree().get_left_hook();
    auto* up_right_hook = the_interval->get_up_tree().get_right_hook();
    auto* down_special_root = the_interval->get_down_tree().get_special_root();
    auto* down_left_hook = the_interval->get_down_tree().get_left_hook();
    auto* down_right_hook = the_interval->get_down_tree().get_right_hook();

    // Validate up-tree
    EXPECT_NODE_EQ(up_nodes[i0]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i0]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_in(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_mid(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i9]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i9]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_in(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_mid(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i8]->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_death(), up_special_root);

    EXPECT_NODE_EQ(up_special_root->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_special_root->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_special_root->get_low(), up_nodes[i8]);

    // Validate down-tree
    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_left_hook, nullptr);
    EXPECT_NODE_EQ(down_right_hook, nullptr);
}

TEST_F(InstanceA, MaintainsFirstChange) {
    perform_first_value_change();

    auto* up_special_root = the_interval->get_up_tree().get_special_root();
    auto* up_left_hook = the_interval->get_up_tree().get_left_hook();
    auto* up_right_hook = the_interval->get_up_tree().get_right_hook();
    auto* down_special_root = the_interval->get_down_tree().get_special_root();
    auto* down_left_hook = the_interval->get_down_tree().get_left_hook();
    auto* down_right_hook = the_interval->get_down_tree().get_right_hook();

    // Validate up-tree
    EXPECT_NODE_EQ(up_nodes[i0]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i0]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_in(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_mid(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i9]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i9]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_in(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_mid(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i8]->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_death(), up_special_root);

    EXPECT_NODE_EQ(up_special_root->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_special_root->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_special_root->get_low(), up_nodes[i8]);

    // Validate down-tree
    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_left_hook, nullptr);
    EXPECT_NODE_EQ(down_right_hook, nullptr);
}

TEST_F(InstanceA, MaintainsSecondChangeAC) {
    perform_first_value_change();
    perform_second_value_change_ac();
    
    auto* up_special_root = the_interval->get_up_tree().get_special_root();
    auto* up_left_hook = the_interval->get_up_tree().get_left_hook();
    auto* up_right_hook = the_interval->get_up_tree().get_right_hook();
    auto* down_special_root = the_interval->get_down_tree().get_special_root();
    auto* down_left_hook = the_interval->get_down_tree().get_left_hook();
    auto* down_right_hook = the_interval->get_down_tree().get_right_hook();

    // Validate up-tree
    EXPECT_NODE_EQ(up_nodes[i0]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i0]->get_down(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_in(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_mid(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i1]->get_in(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_mid(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_death(), up_nodes[i2]);

    EXPECT_NODE_EQ(up_nodes[i2]->get_up(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_in(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_mid(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i9]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i9]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_in(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_mid(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i8]->get_in(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_death(), up_special_root);

    EXPECT_NODE_EQ(up_special_root->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_special_root->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_special_root->get_low(), up_nodes[i8]);

    // Validate down-tree
    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i1]->get_up(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_down(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_in(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_mid(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i2]->get_in(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_mid(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_death(), down_nodes[i1]);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_left_hook, nullptr);
    EXPECT_NODE_EQ(down_right_hook, nullptr);

    perform_second_value_change_xchange();

    // Validate up-tree
    EXPECT_NODE_EQ(up_nodes[i0]->get_up(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_down(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_in(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_mid(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_low(), up_nodes[i1]);

    EXPECT_NODE_EQ(up_nodes[i1]->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_mid(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_death(), up_nodes[i2]);

    EXPECT_NODE_EQ(up_nodes[i2]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i2]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_mid(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i9]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i9]->get_down(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_in(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_mid(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i9]->get_low(), up_nodes[i8]);

    EXPECT_NODE_EQ(up_nodes[i8]->get_in(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_death(), up_special_root);

    EXPECT_NODE_EQ(up_special_root->get_in(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_special_root->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_special_root->get_low(), up_nodes[i8]);

    // Validate down-tree
    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_nodes[i1]);

    EXPECT_NODE_EQ(down_nodes[i1]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i1]->get_down(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_mid(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_low(), down_nodes[i2]);

    EXPECT_NODE_EQ(down_nodes[i2]->get_in(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i2]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i2]);

    EXPECT_NODE_EQ(down_left_hook, nullptr);
    EXPECT_NODE_EQ(down_right_hook, nullptr);
}

// A test-case for debugging random walk with random changes, 14 points, seed 2329275342
class InstanceB : public ::testing::Test {
    
protected:
    inline InstanceB() : values({0, -0.7, -4.4, -6.7, -8.9, -9.7, -10.7, -14.2, -15.8, -12.4, -15.6, -11.9, -11.5, -6.6}) {
        the_interval = the_context.new_interval(values, {std::ref(items)});
        up_nodes.resize(items.size());
        down_nodes.resize(items.size());
        update_node_vector();
    }

    // Perform 10 changes to items 1--10
    void perform_9_value_changes() {
        values[i1] = -3.3;
        values[i2] = -0.3;
        values[i3] = -3.7;
        values[i4] = -7.1;
        values[i5] = -10.3;
        values[i6] = -9.2;
        values[i7] = -10.5;
        values[i8] = -14.9;
        values[i9] = -10.52;
        for (auto idx = i1; idx <= i10; ++idx) {
            the_context.change_value(the_interval, items[idx], values[idx]);
            update_node_vector();
        }
    }

    void perform_10th_value_change() {
        values[i10] = -15.2;
        the_context.change_value(the_interval, items[i10], values[i10]);
        update_node_vector();
    }

    void update_node_vector() {
        for (size_t i = 0; i < items.size(); ++i) {
            up_nodes[i] = items[i]->get_node<1>();
            down_nodes[i] = items[i]->get_node<-1>();
        }
    }

    std::vector<function_value_type> values;
    std::vector<list_item*> items;
    std::vector<up_tree_node*> up_nodes; 
    std::vector<down_tree_node*> down_nodes; 
    persistence_context the_context;
    interval* the_interval;
};

TEST_F(InstanceB, ConstructsCorrectly) {
    auto* down_special_root = the_interval->get_down_tree().get_special_root();

    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i13]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i10]);

    EXPECT_NODE_EQ(down_nodes[i10]->get_up(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_down(), down_nodes[i13]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_low(), down_nodes[i13]);

    EXPECT_NODE_EQ(down_nodes[i13]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i13]->get_mid(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i13]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i0]);
}

TEST_F(InstanceB, Maintains10Changes) {
    perform_9_value_changes();

    auto* up_special_root = the_interval->get_up_tree().get_special_root();
    auto* down_special_root = the_interval->get_down_tree().get_special_root();
    auto* up_left_hook = the_interval->get_up_tree().get_left_hook();
    auto* up_right_hook = the_interval->get_up_tree().get_right_hook();

    // up-tree validation
    EXPECT_NODE_EQ(up_nodes[i0]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i0]->get_down(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i0]->get_in(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_mid(), up_left_hook);
    EXPECT_NODE_EQ(up_nodes[i0]->get_low(), up_nodes[i10]);

    EXPECT_NODE_EQ(up_nodes[i1]->get_in(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_mid(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i1]->get_death(), up_nodes[i2]);

    EXPECT_NODE_EQ(up_nodes[i2]->get_up(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_down(), up_nodes[i6]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_in(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_mid(), up_nodes[i1]);
    EXPECT_NODE_EQ(up_nodes[i2]->get_low(), up_nodes[i10]);

    EXPECT_NODE_EQ(up_nodes[i5]->get_in(), up_nodes[i6]);
    EXPECT_NODE_EQ(up_nodes[i5]->get_mid(), up_nodes[i6]);
    EXPECT_NODE_EQ(up_nodes[i5]->get_death(), up_nodes[i6]);

    EXPECT_NODE_EQ(up_nodes[i6]->get_up(), up_nodes[i2]);
    EXPECT_NODE_EQ(up_nodes[i6]->get_down(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i6]->get_in(), up_nodes[i5]);
    EXPECT_NODE_EQ(up_nodes[i6]->get_mid(), up_nodes[i5]);
    EXPECT_NODE_EQ(up_nodes[i6]->get_low(), up_nodes[i10]);

    EXPECT_NODE_EQ(up_nodes[i8]->get_in(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_mid(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i8]->get_death(), up_nodes[i9]);

    EXPECT_NODE_EQ(up_nodes[i9]->get_up(), up_nodes[i6]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_down(), up_nodes[i10]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_in(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_mid(), up_nodes[i8]);
    EXPECT_NODE_EQ(up_nodes[i9]->get_low(), up_nodes[i10]);

    EXPECT_NODE_EQ(up_nodes[i10]->get_in(), up_nodes[i9]);
    EXPECT_NODE_EQ(up_nodes[i10]->get_mid(), up_nodes[i13]);
    EXPECT_NODE_EQ(up_nodes[i10]->get_death(), up_special_root);

    EXPECT_NODE_EQ(up_nodes[i13]->get_up(), up_special_root);
    EXPECT_NODE_EQ(up_nodes[i13]->get_down(), up_nodes[i10]);
    EXPECT_NODE_EQ(up_nodes[i13]->get_in(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i13]->get_mid(), up_right_hook);
    EXPECT_NODE_EQ(up_nodes[i13]->get_low(), up_nodes[i10]);

    EXPECT_NODE_EQ(up_special_root->get_in(), up_nodes[i0]);
    EXPECT_NODE_EQ(up_special_root->get_mid(), up_nodes[i13]);
    EXPECT_NODE_EQ(up_special_root->get_low(), up_nodes[i10]);

    // down-tree validation
    EXPECT_NODE_EQ(down_nodes[i0]->get_in(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i0]->get_mid(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i0]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[i1]->get_up(), down_nodes[i5]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_down(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_in(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_mid(), down_nodes[i2]);
    EXPECT_NODE_EQ(down_nodes[i1]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i2]->get_in(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_mid(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i2]->get_death(), down_nodes[i1]);

    EXPECT_NODE_EQ(down_nodes[i5]->get_up(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i5]->get_down(), down_nodes[i1]);
    EXPECT_NODE_EQ(down_nodes[i5]->get_in(), down_nodes[i6]);
    EXPECT_NODE_EQ(down_nodes[i5]->get_mid(), down_nodes[i6]);
    EXPECT_NODE_EQ(down_nodes[i5]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i6]->get_in(), down_nodes[i5]);
    EXPECT_NODE_EQ(down_nodes[i6]->get_mid(), down_nodes[i5]);
    EXPECT_NODE_EQ(down_nodes[i6]->get_death(), down_nodes[i5]);

    EXPECT_NODE_EQ(down_nodes[i8]->get_up(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_down(), down_nodes[i5]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_in(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_mid(), down_nodes[i9]);
    EXPECT_NODE_EQ(down_nodes[i8]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i9]->get_in(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_mid(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i9]->get_death(), down_nodes[i8]);

    EXPECT_NODE_EQ(down_nodes[i10]->get_up(), down_special_root);
    EXPECT_NODE_EQ(down_nodes[i10]->get_down(), down_nodes[i8]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_in(), down_nodes[i13]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_mid(), down_nodes[i13]);
    EXPECT_NODE_EQ(down_nodes[i10]->get_low(), down_nodes[i0]);

    EXPECT_NODE_EQ(down_nodes[i13]->get_in(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i13]->get_mid(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_nodes[i13]->get_death(), down_nodes[i10]);

    EXPECT_NODE_EQ(down_special_root->get_in(), down_nodes[i0]);
    EXPECT_NODE_EQ(down_special_root->get_mid(), down_nodes[i10]);
    EXPECT_NODE_EQ(down_special_root->get_low(), down_nodes[i0]);

    perform_10th_value_change();

}

// A test-case for debugging random walk with random changes, 20 points, seed 3405533468
// Simplified for easier testing
class InstanceC : public ::testing::Test {
    
protected:
    inline InstanceC() : values({3, 2, 4, 1, 5, 0, 11, 6, 9, 8, 13, 10, 12, 7, 14}) {
        the_interval = the_context.new_interval(values, {std::ref(items)});
        up_nodes.resize(items.size());
        down_nodes.resize(items.size());
        update_node_vector();
    }

    void perform_final_value_change() {
        the_context.change_value(the_interval, items.back(), 8.5);
        update_node_vector();
    }

    void update_node_vector() {
        for (size_t i = 0; i < items.size(); ++i) {
            up_nodes[i] = items[i]->get_node<1>();
            down_nodes[i] = items[i]->get_node<-1>();
        }
    }

    std::vector<function_value_type> values;
    std::vector<list_item*> items;
    std::vector<up_tree_node*> up_nodes; 
    std::vector<down_tree_node*> down_nodes; 
    persistence_context the_context;
    interval* the_interval;
};

// Same as `InstanceC`, but not simplified
class InstanceCUnsimplified: public ::testing::Test {
    
protected:
    inline InstanceCUnsimplified() : values({0, -0.55, 1.90, 1.60, -0.78, -2.55, 2.04, 1.96, -2.96, 0.89, 4.28, 7.15, 2.52, 6.64, 4.86, 9.78, 6.98, 8.38, 19.17, 18.79}) {
        the_interval = the_context.new_interval(values, {std::ref(items)});
        up_nodes.resize(items.size());
        down_nodes.resize(items.size());
        update_node_vector();
    }

    void perform_2nd_to_last_change() {
        the_context.change_value(the_interval, items[18], 3.67);
        update_node_vector();
    }

    void perform_last_change() {
        the_context.change_value(the_interval, items[19], 6.22);
        update_node_vector();
    }

    void update_node_vector() {
        for (size_t i = 0; i < items.size(); ++i) {
            up_nodes[i] = items[i]->get_node<1>();
            down_nodes[i] = items[i]->get_node<-1>();
        }
    }

    std::vector<function_value_type> values;
    std::vector<list_item*> items;
    std::vector<up_tree_node*> up_nodes; 
    std::vector<down_tree_node*> down_nodes; 
    persistence_context the_context;
    interval* the_interval;
};

TEST_F(InstanceC, UpdatesEndpointCorrectly) {
    auto crit_iter = the_interval->critical_items();
    validate_string_order(the_interval->get_up_tree(), crit_iter.begin(), crit_iter.end(), true);
    perform_final_value_change();
    validate_string_order(the_interval->get_up_tree(), crit_iter.begin(), crit_iter.end(), true);
}

TEST_F(InstanceCUnsimplified, UpdatesEndpointCorrectly) {
    auto crit_iter = the_interval->critical_items();

    auto* down_special_root = the_interval->get_down_tree().get_special_root();

    EXPECT_ITEM_EQ(the_interval->get_up_tree().get_global_max(), items[18]);
    EXPECT_ITEM_EQ(the_interval->get_down_tree().get_global_max(), items[8]);
    validate_string_order(the_interval->get_up_tree(), crit_iter.begin(), crit_iter.end(), true);
    validate_string_order(the_interval->get_down_tree(), crit_iter.begin(), crit_iter.end(), false);

    std::cout << "Change value of item 18 from 19.17 to 3.67\n";
    perform_2nd_to_last_change();
    EXPECT_ITEM_EQ(the_interval->get_up_tree().get_global_max(), items[19]);
    EXPECT_ITEM_EQ(the_interval->get_down_tree().get_global_max(), items[8]);
    validate_string_order(the_interval->get_up_tree(), crit_iter.begin(), crit_iter.end(), true);
    validate_string_order(the_interval->get_down_tree(), crit_iter.begin(), crit_iter.end(), false);

    EXPECT_LT(down_nodes[15]->get_value(), down_nodes[17]->get_value());
    EXPECT_NODE_EQ(down_nodes[ 0]->get_death(), down_nodes[ 1]);
    EXPECT_NODE_EQ(down_nodes[ 2]->get_death(), down_nodes[ 5]);
    EXPECT_NODE_EQ(down_nodes[ 6]->get_death(), down_nodes[ 8]);
    EXPECT_NODE_EQ(down_nodes[11]->get_death(), down_nodes[12]);
    EXPECT_NODE_EQ(down_nodes[13]->get_death(), down_nodes[14]);
    EXPECT_NODE_EQ(down_nodes[15]->get_death(), down_nodes[18]);
    EXPECT_NODE_EQ(down_nodes[17]->get_death(), down_nodes[16]);
    EXPECT_NODE_EQ(down_nodes[19]->get_death(), down_special_root);

    EXPECT_NODE_EQ(down_nodes[ 1]->get_low(), down_nodes[ 2]);
    EXPECT_NODE_EQ(down_nodes[ 5]->get_low(), down_nodes[ 6]);
    EXPECT_NODE_EQ(down_nodes[ 8]->get_low(), down_nodes[19]);
    EXPECT_NODE_EQ(down_nodes[ 8]->get_low(), down_nodes[19]);
    EXPECT_NODE_EQ(down_nodes[12]->get_low(), down_nodes[19]);
    EXPECT_NODE_EQ(down_nodes[14]->get_low(), down_nodes[15]);
    EXPECT_NODE_EQ(down_nodes[16]->get_low(), down_nodes[15]);
    EXPECT_NODE_EQ(down_nodes[18]->get_low(), down_nodes[19]);

    std::cout << "Change value of item 18 from 18.79 to 6.22\n";
    perform_last_change();
    EXPECT_ITEM_EQ(the_interval->get_up_tree().get_global_max(), items[15]);
    EXPECT_ITEM_EQ(the_interval->get_down_tree().get_global_max(), items[8]);
    validate_string_order(the_interval->get_up_tree(), crit_iter.begin(), crit_iter.end(), true);
    validate_string_order(the_interval->get_down_tree(), crit_iter.begin(), crit_iter.end(), false);
}

// A test case for cutting a random walk.
// rep 7 of ex_topological maintenance with 10 points and seed 3724909307
class InstanceD: public ::testing::Test {

protected:
    InstanceD() : values({0, 0.61, 1.57, 1.63, 1.25, 1.60, 1.22, 1.27, 0.87, 1.44}) {
        the_interval = the_context.new_interval(values, {std::ref(items)});
        for (auto& item: items) {
            up_nodes.push_back(item->get_node<1>());
            down_nodes.push_back(item->get_node<-1>());
        }
    }

    void cut() {
        auto [left, right] = the_context.cut_interval(the_interval, items[4]);
        the_new_interval = (right == the_interval) ? left : right;
    }
    
    std::vector<function_value_type> values;
    std::vector<list_item*> items;
    std::vector<up_tree_node*> up_nodes; 
    std::vector<down_tree_node*> down_nodes; 
    persistence_context the_context;
    interval* the_interval;
    interval* the_new_interval;
};

TEST_F(InstanceD, Construction) {
    EXPECT_NODE_EQ(up_nodes[0]->get_death(), the_interval->get_up_tree().get_special_root());
    EXPECT_NODE_EQ(up_nodes[4]->get_death(), up_nodes[5]);
    EXPECT_NODE_EQ(up_nodes[5]->get_up(), up_nodes[3]);
    EXPECT_NODE_EQ(up_nodes[6]->get_death(), up_nodes[7]);
    EXPECT_NODE_EQ(up_nodes[7]->get_up(), up_nodes[5]);
    EXPECT_NODE_EQ(up_nodes[8]->get_death(), up_nodes[3]);
    EXPECT_NODE_EQ(up_nodes[8]->get_mid(), up_nodes[7]);
    EXPECT_NODE_EQ(up_nodes[9]->get_up(), up_nodes[3]);
    EXPECT_NODE_EQ(the_interval->get_up_tree().get_right_hook()->get_death(), up_nodes[9]);
}

TEST_F(InstanceD, Cut) {
    cut();

    EXPECT_ITEM_EQ(the_interval->get_left_endpoint(), items[0]);
    EXPECT_ITEM_EQ(the_interval->get_right_endpoint()->left_neighbor(), items[4]);
    EXPECT_ITEM_EQ(the_new_interval->get_left_endpoint()->right_neighbor(), items[5]);
    EXPECT_ITEM_EQ(the_new_interval->get_right_endpoint(), items[9]);

    EXPECT_ITEM_EQ(the_interval->get_left_endpoint(),  the_interval->get_up_tree().get_left_endpoint());
    EXPECT_ITEM_EQ(the_interval->get_right_endpoint(), the_interval->get_up_tree().get_right_endpoint());
    EXPECT_ITEM_EQ(the_new_interval->get_left_endpoint(),  the_new_interval->get_up_tree().get_left_endpoint());
    EXPECT_ITEM_EQ(the_new_interval->get_right_endpoint(), the_new_interval->get_up_tree().get_right_endpoint());

    EXPECT_ITEM_EQ(the_interval->get_left_endpoint(),  the_interval->get_down_tree().get_left_endpoint());
    EXPECT_ITEM_EQ(the_interval->get_right_endpoint(), the_interval->get_down_tree().get_right_endpoint());
    EXPECT_ITEM_EQ(the_new_interval->get_left_endpoint(),  the_new_interval->get_down_tree().get_left_endpoint());
    EXPECT_ITEM_EQ(the_new_interval->get_right_endpoint(), the_new_interval->get_down_tree().get_right_endpoint());
}
