#include "gtest/gtest.h"
#include <iostream>
#include <limits>

#include "datastructure/banana_tree.h"
#include "datastructure/dictionary.h"
#include "datastructure/interval.h"
#include "example_trees/paper_tree.h"
#include "persistence_defs.h"
#include "utility/recycling_object_pool.h"
#include "validation.h"

using namespace bananas;

constexpr static size_t c=0,d=1,e=2,f=3,g=4,h=5,i=6,j=7,k=8,l=9,m=10,n=11,o=12;
constexpr static size_t p=13,q=14,r=15,s=16,t=17,u=18,v=19,w=20,x=21,y=22,z=23;

// Two intervals, left interval ends in a down-type item, right interval begins with a down-type item.
class GlueIntervalsDownDown : public ::testing::Test {

protected:
    constexpr static double fc=6,fd=2,fe=12,ff=5,fg=8,fh=4,fi=7,fj=1,fk=11,fl=9,fm=10,fn=3,fo=13;
    constexpr static double fp=13.1,fq=1.1,fr=15.1,fs=2.1,ft=17.1,fu=0.1,fv=19.1,fw=14.1,fx=16.1,fy=22,fz=20.1;

    inline GlueIntervalsDownDown() :
        left_items{list_item{c, fc}, list_item{d,fd}, list_item{e,fe}, list_item{f,ff},
                   list_item{g, fg}, list_item{h,fh}, list_item{i,fi}, list_item{j,fj},
                   list_item{k, fk}, list_item{l,fl}, list_item{m,fm}, list_item{n,fn},
                   list_item{o, fo}
        },
        right_items{list_item{p,fp}, list_item{q,fq}, list_item{r,fr}, list_item{s,fs},
                    list_item{t,ft}, list_item{u,fu}, list_item{v,fv}, list_item{w,fw},
                    list_item{x,fx}, list_item{y,fy}, list_item{z,fz}},
        left_interval(up_node_pool, down_node_pool),
        right_interval(up_node_pool, down_node_pool),
        starts_with_up(left_items[1].value<1>() > left_items[0].value<1>()),
        global_max(&right_items[y-p]),
        global_min(&right_items[u-p])
    {
        list_item::link(left_items[c], left_items[d]);
        list_item::link(left_items[d], left_items[e]);
        list_item::link(left_items[e], left_items[f]);
        list_item::link(left_items[f], left_items[g]);
        list_item::link(left_items[g], left_items[h]);
        list_item::link(left_items[h], left_items[i]);
        list_item::link(left_items[i], left_items[j]);
        list_item::link(left_items[j], left_items[k]);
        list_item::link(left_items[k], left_items[l]);
        list_item::link(left_items[l], left_items[m]);
        list_item::link(left_items[m], left_items[n]);
        list_item::link(left_items[n], left_items[o]);
        left_interval.construct(&left_items[c], &left_items[o]);

        list_item::link(right_items[p-p], right_items[q-p]);
        list_item::link(right_items[q-p], right_items[r-p]);
        list_item::link(right_items[r-p], right_items[s-p]);
        list_item::link(right_items[s-p], right_items[t-p]);
        list_item::link(right_items[t-p], right_items[u-p]);
        list_item::link(right_items[u-p], right_items[v-p]);
        list_item::link(right_items[v-p], right_items[w-p]);
        list_item::link(right_items[w-p], right_items[x-p]);
        list_item::link(right_items[x-p], right_items[y-p]);
        list_item::link(right_items[y-p], right_items[z-p]);
        right_interval.construct(&right_items[p-p], &right_items[z-p]);
    }

    void shift_right_items(function_value_type offset) {
        for (auto &item: right_items) {
            item.assign_value(item.value<1>() + offset);
        }
        for (auto &item: left_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
        for (auto &item: right_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
    }

    // An item-pool for use in cutting. This is placed here to make sure that items are destructed after search trees.
    recycling_object_pool<list_item> item_pool;
    recycling_object_pool<banana_tree_node<1>> up_node_pool;
    recycling_object_pool<banana_tree_node<-1>> down_node_pool;
    std::array<list_item, o+1> left_items;
    std::array<list_item, z-p+1> right_items;
    interval left_interval;
    interval right_interval;
    const bool starts_with_up;
    const list_item* global_max;
    const list_item* global_min;
};

// Two intervals, left interval ends in an up-type item, right interval begins with a down-type item.
class GlueIntervalsUpDown : public ::testing::Test {

protected:
    constexpr static double fc=20,fd=24,fe=14,ff=21,fg=18,fh=22,fi=19,fj=25,fk=15,fl=17,fm=16,fn=23,fo=13;
    constexpr static double fp=13.1,fq=1.1,fr=15.1,fs=2.1,ft=17.1,fu=0.1,fv=19.1,fw=14.1,fx=16.1,fy=22.1,fz=20.1;

    inline GlueIntervalsUpDown() :
        left_items{list_item{c, fc}, list_item{d,fd}, list_item{e,fe}, list_item{f,ff},
                   list_item{g, fg}, list_item{h,fh}, list_item{i,fi}, list_item{j,fj},
                   list_item{k, fk}, list_item{l,fl}, list_item{m,fm}, list_item{n,fn},
                   list_item{o, fo}
        },
        right_items{list_item{p,fp}, list_item{q,fq}, list_item{r,fr}, list_item{s,fs},
                    list_item{t,ft}, list_item{u,fu}, list_item{v,fv}, list_item{w,fw},
                    list_item{x,fx}, list_item{y,fy}, list_item{z,fz}},
        left_interval(up_node_pool, down_node_pool),
        right_interval(up_node_pool, down_node_pool),
        starts_with_up(left_items[1].value<1>() > left_items[0].value<1>()),
        global_max(&left_items[j-c]),
        global_min(&right_items[u-p])
    {
        list_item::link(left_items[c], left_items[d]);
        list_item::link(left_items[d], left_items[e]);
        list_item::link(left_items[e], left_items[f]);
        list_item::link(left_items[f], left_items[g]);
        list_item::link(left_items[g], left_items[h]);
        list_item::link(left_items[h], left_items[i]);
        list_item::link(left_items[i], left_items[j]);
        list_item::link(left_items[j], left_items[k]);
        list_item::link(left_items[k], left_items[l]);
        list_item::link(left_items[l], left_items[m]);
        list_item::link(left_items[m], left_items[n]);
        list_item::link(left_items[n], left_items[o]);
        left_interval.construct(&left_items[c], &left_items[o]);

        list_item::link(right_items[p-p], right_items[q-p]);
        list_item::link(right_items[q-p], right_items[r-p]);
        list_item::link(right_items[r-p], right_items[s-p]);
        list_item::link(right_items[s-p], right_items[t-p]);
        list_item::link(right_items[t-p], right_items[u-p]);
        list_item::link(right_items[u-p], right_items[v-p]);
        list_item::link(right_items[v-p], right_items[w-p]);
        list_item::link(right_items[w-p], right_items[x-p]);
        list_item::link(right_items[x-p], right_items[y-p]);
        list_item::link(right_items[y-p], right_items[z-p]);
        right_interval.construct(&right_items[p-p], &right_items[z-p]);
    }

    void shift_right_items(function_value_type offset) {
        for (auto &item: right_items) {
            item.assign_value(item.value<1>() + offset);
        }
        for (auto &item: left_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
        for (auto &item: right_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
    }

    // An item-pool for use in cutting. This is placed here to make sure that items are destructed after search trees.
    recycling_object_pool<list_item> item_pool;
    recycling_object_pool<banana_tree_node<1>> up_node_pool;
    recycling_object_pool<banana_tree_node<-1>> down_node_pool;
    std::array<list_item, o+1> left_items;
    std::array<list_item, z-p+1> right_items;
    interval left_interval;
    interval right_interval;
    const bool starts_with_up;
    const list_item* global_max;
    const list_item* global_min;
};

// Two intervals, left interval ends in a down-type item, right interval begins with an up-type item.
class GlueIntervalsDownUp : public ::testing::Test {

protected:
    constexpr static double fc=6,fd=2,fe=12,ff=5,fg=8,fh=4,fi=7,fj=1,fk=11,fl=9,fm=10,fn=3,fo=13;
    constexpr static double fp=12.1,fq=13.1,fr=5.1,fs=12.2,ft=7.1,fu=20.1,fv=9.1,fw=24.1,fx=16.1,fy=22.1,fz=20.1;

    inline GlueIntervalsDownUp() :
        left_items{list_item{c, fc}, list_item{d,fd}, list_item{e,fe}, list_item{f,ff},
                   list_item{g, fg}, list_item{h,fh}, list_item{i,fi}, list_item{j,fj},
                   list_item{k, fk}, list_item{l,fl}, list_item{m,fm}, list_item{n,fn},
                   list_item{o, fo}
        },
        right_items{list_item{p,fp}, list_item{q,fq}, list_item{r,fr}, list_item{s,fs},
                    list_item{t,ft}, list_item{u,fu}, list_item{v,fv}, list_item{w,fw},
                    list_item{x,fx}, list_item{y,fy}, list_item{z,fz}},
        left_interval(up_node_pool, down_node_pool),
        right_interval(up_node_pool, down_node_pool),
        starts_with_up(left_items[1].value<1>() > left_items[0].value<1>()),
        global_max(&right_items[w-p]),
        global_min(&left_items[j-c])
    {
        list_item::link(left_items[c], left_items[d]);
        list_item::link(left_items[d], left_items[e]);
        list_item::link(left_items[e], left_items[f]);
        list_item::link(left_items[f], left_items[g]);
        list_item::link(left_items[g], left_items[h]);
        list_item::link(left_items[h], left_items[i]);
        list_item::link(left_items[i], left_items[j]);
        list_item::link(left_items[j], left_items[k]);
        list_item::link(left_items[k], left_items[l]);
        list_item::link(left_items[l], left_items[m]);
        list_item::link(left_items[m], left_items[n]);
        list_item::link(left_items[n], left_items[o]);
        left_interval.construct(&left_items[c], &left_items[o]);

        list_item::link(right_items[p-p], right_items[q-p]);
        list_item::link(right_items[q-p], right_items[r-p]);
        list_item::link(right_items[r-p], right_items[s-p]);
        list_item::link(right_items[s-p], right_items[t-p]);
        list_item::link(right_items[t-p], right_items[u-p]);
        list_item::link(right_items[u-p], right_items[v-p]);
        list_item::link(right_items[v-p], right_items[w-p]);
        list_item::link(right_items[w-p], right_items[x-p]);
        list_item::link(right_items[x-p], right_items[y-p]);
        list_item::link(right_items[y-p], right_items[z-p]);
        right_interval.construct(&right_items[p-p], &right_items[z-p]);
    }

    void shift_right_items(function_value_type offset) {
        for (auto &item: right_items) {
            item.assign_value(item.value<1>() + offset);
        }
        for (auto &item: left_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
        for (auto &item: right_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
    }

    // An item-pool for use in cutting. This is placed here to make sure that items are destructed after search trees.
    recycling_object_pool<list_item> item_pool;
    recycling_object_pool<banana_tree_node<1>> up_node_pool;
    recycling_object_pool<banana_tree_node<-1>> down_node_pool;
    std::array<list_item, o+1> left_items;
    std::array<list_item, z-p+1> right_items;
    interval left_interval;
    interval right_interval;
    const bool starts_with_up;
    const list_item* global_max;
    const list_item* global_min;
};

// Two intervals, left interval ends in an up-type item, right interval begins with an up-type item.
class GlueIntervalsUpUp : public ::testing::Test {

protected:
    constexpr static double fc=20,fd=24,fe=14,ff=21,fg=18,fh=22,fi=19,fj=25,fk=15,fl=17,fm=16,fn=23,fo=13;
    constexpr static double fp=12.1,fq=13.1,fr=5.1,fs=11.1,ft=7.1,fu=20.1,fv=9.1,fw=24.1,fx=16.1,fy=22.1,fz=21.1;

    inline GlueIntervalsUpUp() :
        left_items{list_item{c, fc}, list_item{d,fd}, list_item{e,fe}, list_item{f,ff},
                   list_item{g, fg}, list_item{h,fh}, list_item{i,fi}, list_item{j,fj},
                   list_item{k, fk}, list_item{l,fl}, list_item{m,fm}, list_item{n,fn},
                   list_item{o, fo}
        },
        right_items{list_item{p,fp}, list_item{q,fq}, list_item{r,fr}, list_item{s,fs},
                    list_item{t,ft}, list_item{u,fu}, list_item{v,fv}, list_item{w,fw},
                    list_item{x,fx}, list_item{y,fy}, list_item{z,fz}},
        left_interval(up_node_pool, down_node_pool),
        right_interval(up_node_pool, down_node_pool),
        starts_with_up(left_items[1].value<1>() > left_items[0].value<1>()),
        global_max(&left_items[j-c]),
        global_min(&right_items[r-p])
    {
        list_item::link(left_items[c], left_items[d]);
        list_item::link(left_items[d], left_items[e]);
        list_item::link(left_items[e], left_items[f]);
        list_item::link(left_items[f], left_items[g]);
        list_item::link(left_items[g], left_items[h]);
        list_item::link(left_items[h], left_items[i]);
        list_item::link(left_items[i], left_items[j]);
        list_item::link(left_items[j], left_items[k]);
        list_item::link(left_items[k], left_items[l]);
        list_item::link(left_items[l], left_items[m]);
        list_item::link(left_items[m], left_items[n]);
        list_item::link(left_items[n], left_items[o]);
        left_interval.construct(&left_items[c], &left_items[o]);

        list_item::link(right_items[p-p], right_items[q-p]);
        list_item::link(right_items[q-p], right_items[r-p]);
        list_item::link(right_items[r-p], right_items[s-p]);
        list_item::link(right_items[s-p], right_items[t-p]);
        list_item::link(right_items[t-p], right_items[u-p]);
        list_item::link(right_items[u-p], right_items[v-p]);
        list_item::link(right_items[v-p], right_items[w-p]);
        list_item::link(right_items[w-p], right_items[x-p]);
        list_item::link(right_items[x-p], right_items[y-p]);
        list_item::link(right_items[y-p], right_items[z-p]);
        right_interval.construct(&right_items[p-p], &right_items[z-p]);
    }

    void shift_right_items(function_value_type offset) {
        for (auto &item: right_items) {
            item.assign_value(item.value<1>() + offset);
        }
        for (auto &item: left_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
        for (auto &item: right_items) {
            if (item.value<1>() > global_max->value<1>()) {
                global_max = &item;
            }
            if (item.value<1>() < global_min->value<1>()) {
                global_min = &item;
            }
        }
    }

    // An item-pool for use in cutting. This is placed here to make sure that items are destructed after search trees.
    recycling_object_pool<list_item> item_pool;
    recycling_object_pool<banana_tree_node<1>> up_node_pool;
    recycling_object_pool<banana_tree_node<-1>> down_node_pool;
    std::array<list_item, o+1> left_items;
    std::array<list_item, z-p+1> right_items;
    interval left_interval;
    interval right_interval;
    const bool starts_with_up;
    const list_item* global_max;
    const list_item* global_min;
};

TEST_F(GlueIntervalsDownDown, LeftBelowRight) {
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsDownDown, LeftAboveRight) {
    shift_right_items(-1);
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsUpDown, LeftBelowRight) {
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsUpDown, LeftAboveRight) {
    shift_right_items(-1);
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsDownUp, LeftBelowRight) {
    shift_right_items(1);
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsDownUp, LeftAboveRight) {
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsUpUp, LeftBelowRight) {
    shift_right_items(1);
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}

TEST_F(GlueIntervalsUpUp, LeftAboveRight) {
    interval::glue(left_interval, right_interval);
    auto crit_iter = left_interval.critical_items();
    validate_string_order(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end(), !starts_with_up);
    validate_string_order(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end(), starts_with_up);
    EXPECT_EQ(left_interval.get_up_tree().get_global_max(), global_max)
        << "Global max is " << left_interval.get_up_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_up_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_max->get_interval_order()
        << " with value " << global_max->value<1>() << "\n";
    EXPECT_EQ(left_interval.get_down_tree().get_global_max(), global_min)
        << "Global max is " << left_interval.get_down_tree().get_global_max()->get_interval_order()
        << " with value " << left_interval.get_down_tree().get_global_max()->value<1>()
        << "\nbut should be " << global_min->get_interval_order()
        << " with value " << global_min->value<1>() << "\n";
    validate_spine_labels(left_interval.get_up_tree(), crit_iter.begin(), crit_iter.end());
    validate_spine_labels(left_interval.get_down_tree(), crit_iter.begin(), crit_iter.end());
}




//
// Testing helpers for cutting
//

list_item cut_item(list_item* item) {
    return {(item->get_interval_order() + item->right_neighbor()->get_interval_order()) / 2,
            (item->value<1>() + item->right_neighbor()->value<1>()) / 2};
}

TEST_F(PaperUpTreeTest, SmallestBananaFG) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    auto cut = cut_item(&items[f]);
    auto result = up_tree.smallest_banana(cut, min_dict, max_dict);
    EXPECT_EQ(result.get_min<1>(), &items[f])
        << "Expected " << items[f].get_interval_order() << "\n"
        << "but got " << result.get_min<1>()->get_interval_order();
    EXPECT_EQ(result.get_max<1>(), &items[g])
        << "Expected " << items[g].get_interval_order() << "\n"
        << "but got " << result.get_max<1>()->get_interval_order();
}

TEST_F(PaperUpTreeTest, SmallestBananaHI) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    auto cut = cut_item(&items[h]);
    auto result = up_tree.smallest_banana(cut, min_dict, max_dict);
    EXPECT_EQ(result.get_min<1>(), &items[h])
        << "Expected " << items[h].get_interval_order() << "\n"
        << "but got " << result.get_min<1>()->get_interval_order();
    EXPECT_EQ(result.get_max<1>(), &items[i])
        << "Expected " << items[i].get_interval_order() << "\n"
        << "but got " << result.get_max<1>()->get_interval_order();
}

TEST_F(PaperUpTreeTest, SmallestBananaJSpecial) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    auto cut = cut_item(&items[i]);
    auto result = up_tree.smallest_banana(cut, min_dict, max_dict);
    EXPECT_EQ(result.get_min<1>(), &items[j])
        << "Expected " << items[j].get_interval_order() << "\n"
        << "but got " << result.get_min<1>()->get_interval_order();
    EXPECT_EQ(result.get_max<1>(), special_root->get_item())
        << "Expected " << special_root->get_item()->get_interval_order() << "\n"
        << "but got " << result.get_max<1>()->get_interval_order();
}

TEST_F(PaperUpTreeTest, SmallestBananaNK) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    auto cut = cut_item(&items[m]);
    auto result = up_tree.smallest_banana(cut, min_dict, max_dict);
    EXPECT_EQ(result.get_min<1>(), &items[n])
        << "Expected " << items[n].get_interval_order() << "\n"
        << "but got " << result.get_min<1>()->get_interval_order();
    EXPECT_EQ(result.get_max<1>(), &items[k])
        << "Expected " << items[k].get_interval_order() << "\n"
        << "but got " << result.get_max<1>()->get_interval_order();
}

TEST_F(PaperUpTreeTest, LoadStacksLM) {
    items[l].assign_value(8.9);
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    internal::banana_stack<1> L_stack, M_stack, R_stack;
    auto cut = cut_item(&items[k]);
    up_tree.load_stacks(cut,
                        up_tree.smallest_banana(cut, min_dict, max_dict),
                        L_stack,
                        M_stack,
                        R_stack);
    EXPECT_TRUE(L_stack.empty());
    ASSERT_FALSE(M_stack.empty());
    ASSERT_FALSE(R_stack.empty());

    auto r1 = R_stack.top(); R_stack.pop();
    EXPECT_ITEM_EQ(r1.get_min<1>(), &items[l]);
    EXPECT_ITEM_EQ(r1.get_max<1>(), &items[m]);
    EXPECT_TRUE(R_stack.empty());

    auto m1 = M_stack.top(); M_stack.pop();
    ASSERT_FALSE(M_stack.empty());
    auto m2 = M_stack.top(); M_stack.pop();
    EXPECT_TRUE(M_stack.empty());
    EXPECT_ITEM_EQ(m1.get_min<1>(), &items[j]);
    EXPECT_ITEM_EQ(m1.get_max<1>(), special_root->get_item());
    EXPECT_ITEM_EQ(m2.get_min<1>(), &items[n]);
    EXPECT_ITEM_EQ(m2.get_max<1>(), &items[k]);
}

TEST_F(PaperUpTreeTest, LoadStacksHI) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    internal::banana_stack<1> L_stack, M_stack, R_stack;
    auto cut = cut_item(&items[g]);
    up_tree.load_stacks(cut,
                        up_tree.smallest_banana(cut, min_dict, max_dict),
                        L_stack,
                        M_stack,
                        R_stack);
    EXPECT_TRUE(L_stack.empty());
    EXPECT_TRUE(M_stack.empty());
    EXPECT_FALSE(R_stack.empty());

    auto r1 = R_stack.top(); R_stack.pop();
    ASSERT_FALSE(R_stack.empty());
    auto r2 = R_stack.top(); R_stack.pop();
    EXPECT_TRUE(R_stack.empty());
    EXPECT_ITEM_EQ(r1.get_min<1>(), &items[j]);
    EXPECT_ITEM_EQ(r1.get_max<1>(), special_root->get_item());
    EXPECT_ITEM_EQ(r2.get_min<1>(), &items[h]);
    EXPECT_ITEM_EQ(r2.get_max<1>(), &items[i]);
}

TEST_F(PaperUpTreeTest, AddsMissingBananaDE) {
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    internal::banana_stack<1> L_stack, M_stack, R_stack;
    internal::banana_stack<-1> L_inv_stack, R_inv_stack; // These are empty, so we don't load them here.
    auto cut = cut_item(&items[g]);
    up_tree.load_stacks(cut,
                        up_tree.smallest_banana(cut, min_dict, max_dict),
                        L_stack,
                        M_stack,
                        R_stack);
    EXPECT_TRUE(L_stack.empty());
    EXPECT_TRUE(M_stack.empty());
    EXPECT_FALSE(R_stack.empty());

    auto stack_opt = internal::add_missing_short_wave_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack, cut.value<1>());
    EXPECT_TRUE(stack_opt.has_value());
    EXPECT_TRUE(internal::holds_stack(stack_opt.value(), L_inv_stack));

    EXPECT_FALSE(L_inv_stack.empty());
    auto L_inv_top = L_inv_stack.top();
    EXPECT_EQ(L_inv_top.get_min<1>(), &items[d]);
    EXPECT_EQ(L_inv_top.get_max<1>(), &items[e]);
}

TEST_F(PaperUpTreeTest, DoesNotAddMissingBananaDEIfDTooHigh) {
    items[d].assign_value(10);
    items[c].assign_value(11);
    min_dictionary min_dict;
    max_dictionary max_dict;
    for (auto &item: items) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_dict.insert_item(item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_dict.insert_item(item);
        }
    }
    internal::banana_stack<1> L_stack, M_stack, R_stack;
    internal::banana_stack<-1> L_inv_stack, R_inv_stack; // These are empty, so we don't load them here.
    auto cut = cut_item(&items[g]);
    up_tree.load_stacks(cut,
                        up_tree.smallest_banana(cut, min_dict, max_dict),
                        L_stack,
                        M_stack,
                        R_stack);
    EXPECT_TRUE(L_stack.empty());
    EXPECT_TRUE(M_stack.empty());
    EXPECT_FALSE(R_stack.empty());

    auto stack_opt = internal::add_missing_short_wave_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack, cut.value<1>());
    EXPECT_FALSE(stack_opt.has_value());
    EXPECT_TRUE(L_inv_stack.empty());
}

TEST(StackPoppingTest, PopsInCorrectOrder) {
    std::array<list_item, 7> items{
        list_item{0, 5}, // 0 // up 0
        list_item{1, 6}, // 1
        list_item{2, 1}, // 2
        list_item{3, 4}, // 3
        list_item{4, 7}, // 4 // dn 0
        list_item{5, 2}, // 5
        list_item{6, 3}  // 6
    };
    list_item inf_item{-std::numeric_limits<function_value_type>::infinity()};
    internal::banana_stack<1> Lup, Mup, Rup;
    internal::banana_stack<-1> Ldn, Rdn;
    Lup.push({&inf_item, &items[0]});
    Lup.push({&inf_item, &items[1]});
    Mup.push({&inf_item, &items[2]});
    Rup.push({&inf_item, &items[3]});
    Ldn.push({&items[4], &inf_item});
    Rdn.push({&items[5], &inf_item});
    Rdn.push({&items[6], &inf_item});

    auto opt_1 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_1.has_value());
    auto pop_1 = internal::top_of_var_stack<1>(opt_1.value());
    EXPECT_ITEM_EQ(pop_1.get_max<1>(), &items[4]);
    EXPECT_EQ(pop_1.get_max<1>()->value<1>(), 7);
    EXPECT_TRUE(internal::holds_stack(opt_1.value(), Ldn));
    EXPECT_FALSE(internal::holds_stack(opt_1.value(), Rdn));
    EXPECT_FALSE(internal::holds_stack(opt_1.value(), Mup));
    internal::pop_from_var_stack(opt_1.value());

    auto opt_2 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_2.has_value());
    auto pop_2 = internal::top_of_var_stack<1>(opt_2.value());
    EXPECT_ITEM_EQ(pop_2.get_max<1>(), &items[1]);
    EXPECT_EQ(pop_2.get_max<1>()->value<1>(), 6);
    internal::pop_from_var_stack(opt_2.value());

    auto opt_3 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_3.has_value());
    auto pop_3 = internal::top_of_var_stack<1>(opt_3.value());
    EXPECT_ITEM_EQ(pop_3.get_max<1>(), &items[0]);
    EXPECT_EQ(pop_3.get_max<1>()->value<1>(), 5);
    internal::pop_from_var_stack(opt_3.value());

    auto opt_4 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_4.has_value());
    auto pop_4 = internal::top_of_var_stack<1>(opt_4.value());
    EXPECT_ITEM_EQ(pop_4.get_max<1>(), &items[3]);
    EXPECT_EQ(pop_4.get_max<1>()->value<1>(), 4);
    internal::pop_from_var_stack(opt_4.value());

    auto opt_5 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_5.has_value());
    auto pop_5 = internal::top_of_var_stack<1>(opt_5.value());
    EXPECT_ITEM_EQ(pop_5.get_max<1>(), &items[6]);
    EXPECT_EQ(pop_5.get_max<1>()->value<1>(), 3);
    internal::pop_from_var_stack(opt_5.value());

    auto opt_6 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_6.has_value());
    auto pop_6 = internal::top_of_var_stack<1>(opt_6.value());
    EXPECT_ITEM_EQ(pop_6.get_max<1>(), &items[5]);
    EXPECT_EQ(pop_6.get_max<1>()->value<1>(), 2);
    internal::pop_from_var_stack(opt_6.value());

    auto opt_7 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_TRUE(opt_7.has_value());
    auto pop_7 = internal::top_of_var_stack<1>(opt_7.value());
    EXPECT_ITEM_EQ(pop_7.get_max<1>(), &items[2]);
    EXPECT_EQ(pop_7.get_max<1>()->value<1>(), 1);
    internal::pop_from_var_stack(opt_7.value());

    auto opt_8 = internal::top_banana(Lup, Mup, Rup, Ldn, Rdn);
    EXPECT_FALSE(opt_8.has_value());
}

//
// Testing cutting
//

TEST_F(PaperUpTreeTest, CutsHI) {
    internal::banana_stack<1> Lup, Mup, Rup;
    internal::banana_stack<-1> Ldn, Rdn;
    Mup.push({&items[h], &items[i]});
    Rup.push({&items[j], up_tree.get_special_root()->get_item()});

    list_item cut_item{(items[h].get_interval_order() + items[i].get_interval_order()) / 2,
                       (items[h].value<1>() + items[i].value<1>()) / 2};

    banana_tree<1> other_tree{up_node_pool};
    other_tree.initialize_empty_cut_tree(true);

    items[h].cut_right();

    up_tree.cut(cut_item, items[h], items[i], other_tree, Lup, Mup, Rup, Ldn, Rdn);

    auto* this_left_hook = up_tree.get_left_hook();
    auto* this_right_hook = up_tree.get_right_hook();
    auto* other_left_hook = other_tree.get_left_hook();
    auto* other_special_root = other_tree.get_special_root();

    //
    // Testing nodes in `other_tree`
    //
    EXPECT_NODE_EQ(nodes[c]->get_in(), other_left_hook);
    EXPECT_NODE_EQ(nodes[c]->get_mid(), other_left_hook);
    EXPECT_NODE_EQ(nodes[c]->get_up(), other_special_root);
    EXPECT_NODE_EQ(nodes[c]->get_down(), nodes[d]);
    EXPECT_NODE_EQ(nodes[c]->get_low(), nodes[d]);

    EXPECT_NODE_EQ(nodes[d]->get_in(), nodes[c]);
    EXPECT_NODE_EQ(nodes[d]->get_mid(), nodes[e]);
    EXPECT_NODE_EQ(nodes[d]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[d]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[d]->get_low(), nodes[d]);
    EXPECT_NODE_EQ(nodes[d]->get_death(), other_special_root);

    EXPECT_NODE_EQ(nodes[e]->get_in(), nodes[h]);
    EXPECT_NODE_EQ(nodes[e]->get_mid(), nodes[g]);
    EXPECT_NODE_EQ(nodes[e]->get_up(), other_special_root);
    EXPECT_NODE_EQ(nodes[e]->get_down(), nodes[d]);
    EXPECT_NODE_EQ(nodes[e]->get_low(), nodes[d]);

    EXPECT_NODE_EQ(nodes[f]->get_in(), nodes[g]);
    EXPECT_NODE_EQ(nodes[f]->get_mid(), nodes[g]);
    EXPECT_NODE_EQ(nodes[f]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[f]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[f]->get_low(), nodes[f]);
    EXPECT_NODE_EQ(nodes[f]->get_death(), nodes[g]);

    EXPECT_NODE_EQ(nodes[g]->get_in(), nodes[f]);
    EXPECT_NODE_EQ(nodes[g]->get_mid(), nodes[f]);
    EXPECT_NODE_EQ(nodes[g]->get_up(), nodes[e]);
    EXPECT_NODE_EQ(nodes[g]->get_down(), nodes[h]);
    EXPECT_NODE_EQ(nodes[g]->get_low(), nodes[h]);

    EXPECT_NODE_EQ(nodes[h]->get_in(), nodes[e]);
    EXPECT_NODE_EQ(nodes[h]->get_mid(), nodes[g]);
    EXPECT_NODE_EQ(nodes[h]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[h]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[h]->get_low(), nodes[h]);
    EXPECT_NODE_EQ(nodes[h]->get_death(), nodes[e]);

    EXPECT_TRUE(other_special_root->is_special_root());
    EXPECT_NODE_EQ(other_special_root->get_in(), nodes[c]);
    EXPECT_NODE_EQ(other_special_root->get_mid(), nodes[e]);
    EXPECT_NODE_EQ(other_special_root->get_low(), nodes[d]);

    EXPECT_NODE_EQ(other_left_hook->get_in(), nodes[c]);
    EXPECT_NODE_EQ(other_left_hook->get_mid(), nodes[c]);
    EXPECT_NODE_EQ(other_left_hook->get_death(), nodes[c]);


    //
    // Testing nodes in `up_tree`
    //
    EXPECT_NODE_EQ(nodes[i]->get_in(), this_left_hook);
    EXPECT_NODE_EQ(nodes[i]->get_mid(), this_left_hook);
    EXPECT_NODE_EQ(nodes[i]->get_up(), special_root);
    EXPECT_NODE_EQ(nodes[i]->get_down(), nodes[j]);
    EXPECT_NODE_EQ(nodes[i]->get_low(), nodes[j]);

    EXPECT_NODE_EQ(nodes[j]->get_in(), nodes[i]);
    EXPECT_NODE_EQ(nodes[j]->get_mid(), nodes[k]);
    EXPECT_NODE_EQ(nodes[j]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[j]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[j]->get_low(), nodes[j]);

    EXPECT_NODE_EQ(nodes[k]->get_in(), nodes[n]);
    EXPECT_NODE_EQ(nodes[k]->get_mid(), nodes[m]);
    EXPECT_NODE_EQ(nodes[k]->get_up(), nodes[o]);
    EXPECT_NODE_EQ(nodes[k]->get_down(), nodes[j]);
    EXPECT_NODE_EQ(nodes[k]->get_low(), nodes[j]);

    EXPECT_NODE_EQ(nodes[l]->get_in(), nodes[m]);
    EXPECT_NODE_EQ(nodes[l]->get_mid(), nodes[m]);
    EXPECT_NODE_EQ(nodes[l]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[l]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[l]->get_low(), nodes[l]);

    EXPECT_NODE_EQ(nodes[m]->get_in(), nodes[l]);
    EXPECT_NODE_EQ(nodes[m]->get_mid(), nodes[l]);
    EXPECT_NODE_EQ(nodes[m]->get_up(), nodes[k]);
    EXPECT_NODE_EQ(nodes[m]->get_down(), nodes[n]);
    EXPECT_NODE_EQ(nodes[m]->get_low(), nodes[n]);

    EXPECT_NODE_EQ(nodes[n]->get_in(), nodes[k]);
    EXPECT_NODE_EQ(nodes[n]->get_mid(), nodes[m]);
    EXPECT_NODE_EQ(nodes[n]->get_up(), nullptr);
    EXPECT_NODE_EQ(nodes[n]->get_down(), nullptr);
    EXPECT_NODE_EQ(nodes[n]->get_low(), nodes[n]);

    EXPECT_NODE_EQ(nodes[o]->get_in(), this_right_hook);
    EXPECT_NODE_EQ(nodes[o]->get_mid(), this_right_hook);
    EXPECT_NODE_EQ(nodes[o]->get_up(), special_root);
    EXPECT_NODE_EQ(nodes[o]->get_down(), nodes[k]);
    EXPECT_NODE_EQ(nodes[o]->get_low(), nodes[j]);

    EXPECT_TRUE(special_root->is_special_root());
    EXPECT_NODE_EQ(special_root->get_in(), nodes[i]);
    EXPECT_NODE_EQ(special_root->get_mid(), nodes[o]);
    EXPECT_NODE_EQ(special_root->get_low(), nodes[j]);

    EXPECT_NODE_EQ(this_left_hook->get_in(), nodes[i]);
    EXPECT_NODE_EQ(this_left_hook->get_mid(), nodes[i]);
    EXPECT_NODE_EQ(this_left_hook->get_death(), nodes[i]);

    EXPECT_NODE_EQ(this_right_hook->get_in(), nodes[o]);
    EXPECT_NODE_EQ(this_right_hook->get_mid(), nodes[o]);
    EXPECT_NODE_EQ(this_right_hook->get_death(), nodes[o]);

    validate_string_order(other_tree, items.begin(), items.begin() + 6, true);
    validate_string_order(up_tree, items.begin() + 6, items.end(), true);
}

//
// Testing cutting
//

TEST_F(GlueIntervalsDownDown, LeftBelowRightThenCut) {
    interval::glue(left_interval, right_interval);
    // Unfortunately, since we cut `left_interval` in the left spine, `left_interval` holds the right interval after cutting.
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    [[maybe_unused]] list_item* left_of_cut = new_left_interval.get_up_tree().get_right_endpoint();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    [[maybe_unused]] list_item* right_of_cut = new_right_interval.get_up_tree().get_left_endpoint();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    [[maybe_unused]] list_item* up_left_special_root = new_left_interval.get_up_tree().get_special_root()->get_item();
    EXPECT_EQ(up_left_special_root->get_interval_order(), std::numeric_limits<interval_order_type>::infinity());
    [[maybe_unused]] list_item* up_right_special_root = new_right_interval.get_up_tree().get_special_root()->get_item();
    EXPECT_EQ(up_right_special_root->get_interval_order(), std::numeric_limits<interval_order_type>::infinity());
    [[maybe_unused]] list_item* up_left_left_hook = new_left_interval.get_up_tree().get_left_hook()->get_item();
    [[maybe_unused]] list_item* up_left_right_hook = new_left_interval.get_up_tree().get_right_hook()->get_item();

    EXPECT_TRUE(left_of_cut->is_down_type<1>());
    EXPECT_TRUE(right_of_cut->is_up_type<1>());
    EXPECT_NE(up_left_left_hook, nullptr);
    EXPECT_NE(up_left_right_hook, nullptr);

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_EQ(up_left_special_root->get_node<1>(), new_left_interval.get_up_tree().get_special_root());
    EXPECT_EQ(up_left_left_hook->get_node<1>(), new_left_interval.get_up_tree().get_left_hook());
    EXPECT_EQ(up_left_right_hook->get_node<1>(), new_left_interval.get_up_tree().get_right_hook());

    EXPECT_TRUE(up_left_special_root->get_node<1>()->is_special_root());
    EXPECT_TRUE(up_right_special_root->get_node<1>()->is_special_root());

    // Test structure of left up-tree
    EXPECT_EQ(&left_items[c], new_left_interval.get_up_tree().get_left_endpoint());
    EXPECT_EQ(left_items[o].get_node<1>(), nullptr) << "  Expected `o` to not be represented.";
    EXPECT_EQ(left_of_cut, new_left_interval.get_up_tree().get_right_endpoint());
    std::array<list_item*, 13> left_validation_items;
    for (size_t idx = c; idx < o; ++idx) {
        left_validation_items[idx] = &left_items[idx];
    }
    left_validation_items[o] = left_of_cut;
    validate_paper_up_tree(left_validation_items, up_left_special_root, up_left_left_hook, up_left_right_hook);

    // Test structure of right up-tree
    EXPECT_EQ   (right_of_cut, new_right_interval.get_up_tree().get_left_endpoint());
    EXPECT_up   (1, right_of_cut, nullptr);
    EXPECT_down (1, right_of_cut, nullptr);
    EXPECT_in   (1, right_of_cut, &right_items[p-p]);
    EXPECT_mid  (1, right_of_cut, &right_items[p-p]);
    EXPECT_low  (1, right_of_cut, right_of_cut);
    EXPECT_death(1, right_of_cut, &right_items[p-p]);

    EXPECT_up   (1, &right_items[p-p], &right_items[t-p]);
    EXPECT_down (1, &right_items[p-p], &right_items[q-p]);
    EXPECT_in   (1, &right_items[p-p], right_of_cut);
    EXPECT_mid  (1, &right_items[p-p], right_of_cut);
    EXPECT_low  (1, &right_items[p-p], &right_items[q-p]);
    EXPECT_death(1, &right_items[p-p], nullptr);

    EXPECT_up   (1, &right_items[q-p], nullptr);
    EXPECT_down (1, &right_items[q-p], nullptr);
    EXPECT_in   (1, &right_items[q-p], &right_items[p-p]);
    EXPECT_mid  (1, &right_items[q-p], &right_items[r-p]);
    EXPECT_low  (1, &right_items[q-p], &right_items[q-p]);
    EXPECT_death(1, &right_items[q-p], &right_items[t-p]);

    EXPECT_up   (1, &right_items[r-p], &right_items[t-p]);
    EXPECT_down (1, &right_items[r-p], &right_items[q-p]);
    EXPECT_in   (1, &right_items[r-p], &right_items[s-p]);
    EXPECT_mid  (1, &right_items[r-p], &right_items[s-p]);
    EXPECT_low  (1, &right_items[r-p], &right_items[q-p]);
    EXPECT_death(1, &right_items[r-p], nullptr);

    EXPECT_up   (1, &right_items[s-p], nullptr);
    EXPECT_down (1, &right_items[s-p], nullptr);
    EXPECT_in   (1, &right_items[s-p], &right_items[r-p]);
    EXPECT_mid  (1, &right_items[s-p], &right_items[r-p]);
    EXPECT_low  (1, &right_items[s-p], &right_items[s-p]);
    EXPECT_death(1, &right_items[s-p], &right_items[r-p]);

    EXPECT_up   (1, &right_items[t-p], up_right_special_root);
    EXPECT_down (1, &right_items[t-p], &right_items[u-p]);
    EXPECT_in   (1, &right_items[t-p], &right_items[p-p]);
    EXPECT_mid  (1, &right_items[t-p], &right_items[r-p]);
    EXPECT_low  (1, &right_items[t-p], &right_items[u-p]);
    EXPECT_death(1, &right_items[t-p], nullptr);

    EXPECT_up   (1, &right_items[u-p], nullptr);
    EXPECT_down (1, &right_items[u-p], nullptr);
    EXPECT_in   (1, &right_items[u-p], &right_items[t-p]);
    EXPECT_mid  (1, &right_items[u-p], &right_items[v-p]);
    EXPECT_low  (1, &right_items[u-p], &right_items[u-p]);
    EXPECT_death(1, &right_items[u-p], up_right_special_root);

    EXPECT_up   (1, &right_items[v-p], &right_items[y-p]);
    EXPECT_down (1, &right_items[v-p], &right_items[u-p]);
    EXPECT_in   (1, &right_items[v-p], &right_items[w-p]);
    EXPECT_mid  (1, &right_items[v-p], &right_items[w-p]);
    EXPECT_low  (1, &right_items[v-p], &right_items[u-p]);
    EXPECT_death(1, &right_items[v-p], nullptr);

    EXPECT_up   (1, &right_items[w-p], nullptr);
    EXPECT_down (1, &right_items[w-p], nullptr);
    EXPECT_in   (1, &right_items[w-p], &right_items[v-p]);
    EXPECT_mid  (1, &right_items[w-p], &right_items[v-p]);
    EXPECT_low  (1, &right_items[w-p], &right_items[w-p]);
    EXPECT_death(1, &right_items[w-p], &right_items[v-p]);

    EXPECT_up   (1, &right_items[y-p], up_right_special_root);
    EXPECT_down (1, &right_items[y-p], &right_items[v-p]);
    EXPECT_in   (1, &right_items[y-p], &right_items[z-p]);
    EXPECT_mid  (1, &right_items[y-p], &right_items[z-p]);
    EXPECT_low  (1, &right_items[y-p], &right_items[u-p]);
    EXPECT_death(1, &right_items[y-p], nullptr);

    EXPECT_EQ   (&right_items[z-p], new_right_interval.get_up_tree().get_right_endpoint());
    EXPECT_up   (1, &right_items[z-p], nullptr);
    EXPECT_down (1, &right_items[z-p], nullptr);
    EXPECT_in   (1, &right_items[z-p], &right_items[y-p]);
    EXPECT_mid  (1, &right_items[z-p], &right_items[y-p]);
    EXPECT_low  (1, &right_items[z-p], &right_items[z-p]);
    EXPECT_death(1, &right_items[z-p], &right_items[y-p]);

    EXPECT_up   (1, up_right_special_root, nullptr);
    EXPECT_down (1, up_right_special_root, nullptr);
    EXPECT_in   (1, up_right_special_root, &right_items[t-p]);
    EXPECT_mid  (1, up_right_special_root, &right_items[y-p]);
    EXPECT_low  (1, up_right_special_root, &right_items[u-p]);
    EXPECT_death(1, up_right_special_root, nullptr);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}


TEST_F(GlueIntervalsDownDown, LeftAboveRightThenCut) {
    shift_right_items(-1);
    interval::glue(left_interval, right_interval);
    // Unfortunately, since we cut `left_interval` in the left spine, `left_interval` holds the right interval after cutting.
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_up_type<1>());
    EXPECT_EQ(new_left_interval.get_up_tree().get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_down_type<1>());
    EXPECT_EQ(new_right_interval.get_up_tree().get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsDownUp, LeftBelowRightThenCut) {
    shift_right_items(1);
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;
    

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_down_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_up_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    auto* up_left_special_root = new_left_interval.get_up_tree().get_special_root();
    auto* up_right_special_root = new_right_interval.get_up_tree().get_special_root();
    auto* down_left_special_root = new_left_interval.get_down_tree().get_special_root();
    auto* down_right_special_root = new_right_interval.get_down_tree().get_special_root();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(up_left_special_root->is_special_root());
    EXPECT_TRUE(up_right_special_root->is_special_root());
    EXPECT_TRUE(down_left_special_root->is_special_root());
    EXPECT_TRUE(down_right_special_root->is_special_root());

    EXPECT_NE(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_NE(up_of<1>(&left_items[e]), up_right_special_root);
    ASSERT_NE(up_of<1>(&right_items[y-p]), up_left_special_root);
    EXPECT_in(1, up_right_special_root->get_item(), &right_items[q-p]);
    EXPECT_mid(1, up_right_special_root->get_item(), &right_items[w-p]);
    EXPECT_up(1, &right_items[q-p], up_right_special_root->get_item());
    EXPECT_up(1, &right_items[w-p], up_right_special_root->get_item());

    std::array<list_item*, 13> left_validation_items;
    for (size_t idx = c; idx < o; ++idx) {
        left_validation_items[idx] = &left_items[idx];
    }
    left_validation_items[o] = left_of_cut;
    validate_paper_up_tree(left_validation_items, new_left_interval.get_up_tree().get_special_root()->get_item(),
                           new_left_interval.get_up_tree().get_left_hook()->get_item(),
                           new_left_interval.get_up_tree().get_right_hook()->get_item());

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsDownUp, LeftAboveRightThenCut) {
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_up_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_down_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    EXPECT_NE(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_NE(new_right_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_right_hook(), nullptr);

    std::array<list_item*, 13> left_validation_items;
    for (size_t idx = c; idx <= o; ++idx) {
        left_validation_items[idx] = &left_items[idx];
    }
    validate_paper_up_tree(left_validation_items, new_left_interval.get_up_tree().get_special_root()->get_item(),
                           new_left_interval.get_up_tree().get_left_hook()->get_item(),
                           left_of_cut);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsUpDown, LeftBelowRightThenCut) {
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_down_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_up_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    EXPECT_EQ(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_EQ(new_right_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_right_hook(), nullptr);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsUpDown, LeftAboveRightThenCut) {
    shift_right_items(-1);
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_up_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_down_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    EXPECT_EQ(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_NE(new_right_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_right_hook(), nullptr);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsUpUp, LeftBelowRightThenCut) {
    shift_right_items(1);
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_down_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_up_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    EXPECT_EQ(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_EQ(new_right_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_right_hook(), nullptr);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}

TEST_F(GlueIntervalsUpUp, LeftAboveRightThenCut) {
    interval::glue(left_interval, right_interval);
    auto new_left_interval = left_interval.cut(&left_items.back(), item_pool);
    auto& new_right_interval = left_interval;

    auto* left_of_cut = left_items[o].right_neighbor();
    EXPECT_EQ(left_of_cut->get_interval_order(), (2*12.0 + 13.0)/3);
    EXPECT_TRUE(left_of_cut->is_up_type<1>());
    EXPECT_EQ(new_left_interval.get_right_endpoint(), left_of_cut)
        << "Found right endpoint of left interval to be " << new_left_interval.get_up_tree().get_right_endpoint()->get_interval_order();
    auto* right_of_cut = right_items[p-p].left_neighbor();
    EXPECT_EQ(right_of_cut->get_interval_order(), (12.0 + 2*13.0)/3);
    EXPECT_TRUE(right_of_cut->is_down_type<1>());
    EXPECT_EQ(new_right_interval.get_left_endpoint(), right_of_cut)
        << "Found left endpoint of right interval to be " << new_right_interval.get_up_tree().get_left_endpoint()->get_interval_order();

    EXPECT_NE(new_left_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_up_tree().get_global_max(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_global_max(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_global_max(), nullptr);

    EXPECT_TRUE(new_left_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_up_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_left_interval.get_down_tree().get_special_root()->is_special_root());
    EXPECT_TRUE(new_right_interval.get_down_tree().get_special_root()->is_special_root());

    EXPECT_EQ(new_left_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_left_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_left_interval.get_down_tree().get_right_hook(), nullptr);

    EXPECT_NE(new_right_interval.get_up_tree().get_left_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_up_tree().get_right_hook(), nullptr);
    EXPECT_EQ(new_right_interval.get_down_tree().get_left_hook(), nullptr);
    EXPECT_NE(new_right_interval.get_down_tree().get_right_hook(), nullptr);

    auto left_crit_iter = new_left_interval.critical_items();
    validate_string_order(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end(), false);
    validate_spine_labels(new_left_interval.get_up_tree(), left_crit_iter.begin(), left_crit_iter.end());
    validate_string_order(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end(), true);
    validate_spine_labels(new_left_interval.get_down_tree(), left_crit_iter.begin(), left_crit_iter.end());
    auto right_crit_iter = new_right_interval.critical_items();
    validate_string_order(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end(), true);
    validate_spine_labels(new_right_interval.get_up_tree(), right_crit_iter.begin(), right_crit_iter.end());
    validate_string_order(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end(), false);
    validate_spine_labels(new_right_interval.get_down_tree(), right_crit_iter.begin(), right_crit_iter.end());
}
