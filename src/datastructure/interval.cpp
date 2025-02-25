#include <algorithm>
#include <limits>
#include <ostream>
#include <random>

#include "algorithms/banana_tree_algorithms.h"
#include "datastructure/dictionary.h"
#include "datastructure/interval.h"
#include "datastructure/banana_tree.h"
#include "datastructure/list_item.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/iterator.h"
#include "utility/recycling_object_pool.h"

using namespace bananas;

interval::interval(recycling_object_pool<up_tree_node> & up_tree_node_pool,
                   recycling_object_pool<down_tree_node> &down_tree_node_pool) :
        persistence(up_tree_node_pool, down_tree_node_pool) {}

interval::interval(recycling_object_pool<up_tree_node> & up_tree_node_pool,
                   recycling_object_pool<down_tree_node> &down_tree_node_pool,
                   list_item* left_endpoint,
                   list_item* right_endpoint) :
        persistence(up_tree_node_pool, down_tree_node_pool),
        left_endpoint(left_endpoint),
        right_endpoint(right_endpoint) {
    construct(left_endpoint, right_endpoint);
}

interval::interval(recycling_object_pool<up_tree_node> & up_tree_node_pool,
                   recycling_object_pool<down_tree_node> &down_tree_node_pool,
                   std::pair<list_item*, list_item*> endpoints) : interval(up_tree_node_pool,
                                                                           down_tree_node_pool,
                                                                           endpoints.first,
                                                                           endpoints.second) {}

interval::interval(interval&& ival) : persistence(std::move(ival.persistence)),
                                      interval_stats(std::move(ival.interval_stats)),
                                      min_dict(std::move(ival.min_dict)),
                                      max_dict(std::move(ival.max_dict)),
                                      nc_dict(std::move(ival.nc_dict)),
                                      left_endpoint(ival.left_endpoint),
                                      right_endpoint(ival.right_endpoint) {}

interval::interval(persistence_data_structure &&pds) : persistence(std::move(pds)) {
    left_endpoint = persistence.get_up_tree().get_left_endpoint();
    right_endpoint = persistence.get_up_tree().get_right_endpoint();
}

void interval::construct(list_item* left_endpoint, list_item* right_endpoint) {
    this->left_endpoint = left_endpoint;
    this->right_endpoint = right_endpoint;
    persistence.construct(left_endpoint, right_endpoint);
    insert_into_dicts();
}

void interval::insert_into_dicts() {
    // TODO: this is a dirty, evil hack. This gets us somewhat balanced trees, but is stupid.
    // Constructing directly from a sorted range should also work (see Option 2 below),
    // but this doesn't seem to work properly for splay trees.
    // Consider using this shuffling option with splay trees and Option 2 for AVL trees or treaps

#ifdef SPLAY_SEARCH_TREE
    std::vector<list_item*> items;
    for (auto &item: *this) {
        items.push_back(&item);
    }
    std::mt19937 g(std::random_device{}());
    std::shuffle(items.begin(), items.end(), g);
    for (auto &item: items) {
        if (item->is_minimum<1>() || item->is_up_type<1>()) {
            min_dict.insert_item(*item);
        } else if (item->is_maximum<1>() || item->is_down_type<1>()) {
            max_dict.insert_item(*item);
        } else {
            nc_dict.insert_item(*item);
        }
    }
#else
    // Option 2:
    // directly constructing the dictionaries from sorted ranges.
    // This doesn't seem to work with splay trees.
    std::vector<list_item*> min_items, nc_items, max_items;
    for (auto &item: *this) {
        if (item.is_minimum<1>() || item.is_up_type<1>()) {
            min_items.push_back(&item);
        } else if (item.is_maximum<1>() || item.is_down_type<1>()) {
            max_items.push_back(&item);
        } else {
            nc_items.push_back(&item);
        }
    }
    min_dict = min_dictionary{pointer_range_adapter{min_items.begin()}, pointer_range_adapter{min_items.end()}};
    max_dict = max_dictionary{pointer_range_adapter{max_items.begin()}, pointer_range_adapter{max_items.end()}};
    nc_dict = nc_dictionary{pointer_range_adapter{nc_items.begin()}, pointer_range_adapter{nc_items.end()}};
#endif
}

void interval::update_value(list_item* item, function_value_type value) {
    if (item->value<1>() == value) {
        return;
    }
    if (item->is_endpoint()) {
        update_value_of_endpoint(item, value);
    } else if (item->is_noncritical<1>()) {
        update_non_critical_value(item, value);
    } else {
        update_critical_value(item, value);
    }
}

list_item* interval::insert_item(interval_order_type order, recycling_object_pool<list_item> &item_pool) {
    massert(left_endpoint->get_interval_order() < order && order < right_endpoint->get_interval_order(),
            "Expected to insert a non-endpoint item.");

    auto new_item = item_pool.construct(order, 0.0);
    auto prev_min_it = min_dict.previous_item(*new_item);
    auto prev_max_it = max_dict.previous_item(*new_item);
    auto prev_nc_it = nc_dict.previous_item(*new_item);
    massert(prev_min_it != min_dict.end() || prev_max_it != max_dict.end() || prev_nc_it != nc_dict.end(),
            "Expected an item in one of the three dictionaries.");
    auto prev_min_order = prev_min_it == min_dict.end() ? -std::numeric_limits<interval_order_type>::infinity() : prev_min_it->get_interval_order();
    auto prev_max_order = prev_max_it == max_dict.end() ? -std::numeric_limits<interval_order_type>::infinity() : prev_max_it->get_interval_order();
    auto prev_nc_order =  prev_nc_it == nc_dict.end()  ? -std::numeric_limits<interval_order_type>::infinity() : prev_nc_it->get_interval_order();

    list_item* left_neighbor_item = nullptr;
    if (prev_nc_order > prev_min_order && prev_nc_order > prev_max_order) {
        left_neighbor_item = &*prev_nc_it;
    } else if (prev_min_order > prev_nc_order && prev_min_order > prev_max_order) {
        left_neighbor_item = &*prev_min_it;
    } else {
        left_neighbor_item = &*prev_max_it;
    }
    auto* right_neighbor_item = left_neighbor_item->right_neighbor();
    left_neighbor_item->cut_right();
    list_item::link(*left_neighbor_item, *new_item);
    list_item::link(*new_item, *right_neighbor_item);
    new_item->interpolate_neighbors();
    nc_dict.insert_item(*new_item);
    return new_item; 
}

list_item* interval::insert_item_to_right_of(list_item* item, recycling_object_pool<list_item> &item_pool) {
    massert(item->right_neighbor() != nullptr, "Expected to insert a non-endpoint item.");

    auto new_order = (item->get_interval_order() + item->right_neighbor()->get_interval_order()) / 2.0;
    auto* new_item  = item_pool.construct(new_order, 0.0);
    auto* new_right_neighbor = item->right_neighbor();
    item->cut_right();
    list_item::link(*item, *new_item);
    list_item::link(*new_item, *new_right_neighbor);
    new_item->interpolate_neighbors();
    nc_dict.insert_item(*new_item);
    return new_item;
}

list_item* interval::insert_right_endpoint(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool) {
    return insert_endpoint_impl<false>(value, offset, item_pool);
}

list_item* interval::insert_left_endpoint(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool) {
    return insert_endpoint_impl<true>(value, -offset, item_pool);
}

template<bool insert_left>
list_item* interval::insert_endpoint_impl(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool) {
    auto* old_endpoint = insert_left ? left_endpoint : right_endpoint;
    auto old_endpoint_value = old_endpoint->value<1>();
    auto was_down = old_endpoint->is_down_type<1>();
    auto temp_value = was_down ? add_tiniest_offset<1>(old_endpoint_value) : add_tiniest_offset<-1>(old_endpoint_value);
    auto new_item = item_pool.construct(old_endpoint->get_interval_order() + offset, temp_value);
    if constexpr (insert_left) {
        list_item::link(*new_item, *old_endpoint);
    } else {
        list_item::link(*old_endpoint, *new_item);
    }
    // Move/Add the old and new endpoint to the appropriate dictionaries.
    // The old endpoint becomes non-critical, so goes into nc_dict.
    // The new endpoint takes on the criticality of the old endpoint.
    if (was_down) {
        move_in_dictionaries(old_endpoint, max_dict, nc_dict);
        max_dict.insert_item(*new_item);
    } else {
        move_in_dictionaries(old_endpoint, min_dict, nc_dict);
        min_dict.insert_item(*new_item);
    }
    if constexpr (insert_left) {
        persistence.replace_left_endpoint(new_item);
    } else {
        persistence.replace_right_endpoint(new_item);
    }
    update_value_of_endpoint(new_item, value);
    if constexpr (insert_left) {
        left_endpoint = new_item;
    } else {
        right_endpoint = new_item;
    }
    return new_item;
}
namespace bananas {
    template list_item* interval::insert_endpoint_impl<true>(function_value_type, interval_order_type, recycling_object_pool<list_item>&);
    template list_item* interval::insert_endpoint_impl<false>(function_value_type, interval_order_type, recycling_object_pool<list_item>&);
}

void interval::delete_internal_item(list_item* item) {
    massert(item->is_internal(), "Expected an internal item.");
    auto left_neighbor = item->left_neighbor();
    auto right_neighbor = item->right_neighbor();
    if (item->is_critical<1>()) {
        auto left_neighbor_value = left_neighbor->value<1>();
        auto right_neighbor_value = right_neighbor->value<1>();
        update_critical_value(item, (left_neighbor_value + right_neighbor_value) / 2.0);
    }
    massert(item->is_noncritical<1>(), "Expected a non-critical item after forcing it to be non-criticial.");
    nc_dict.erase_item(*item);
    left_neighbor->cut_right();
    right_neighbor->cut_left();
    list_item::link(*left_neighbor, *right_neighbor);
}

list_item* interval::delete_right_endpoint() {
    return delete_endpoint_impl<false>();
}

list_item* interval::delete_left_endpoint() {
    return delete_endpoint_impl<true>();
}

template<bool left>
list_item* interval::delete_endpoint_impl() {
    auto* old_endpoint = left ? left_endpoint : right_endpoint;
    auto* new_endpoint = left ? old_endpoint->right_neighbor() : old_endpoint->left_neighbor();
    massert(new_endpoint->is_internal(), "Expected at least three items when deleting an endpoint.");
    auto* next_neighbor = left ? new_endpoint->right_neighbor() : new_endpoint->left_neighbor();
    const auto is_down = new_endpoint->value<1>() > next_neighbor->value<1>();
    auto temp_value = is_down ? add_tiniest_offset<1>(new_endpoint->value<1>())
                              : add_tiniest_offset<-1>(new_endpoint->value<1>());
    update_value_of_endpoint(old_endpoint, temp_value);
    if (old_endpoint->is_down_type<1>()) {
        max_dict.erase_item(*old_endpoint);
    } else {
        min_dict.erase_item(*old_endpoint);
    }
    massert(new_endpoint->is_noncritical<1>(), "Expected the new endpoint to be non-critical before deleting the old endpoint.");
    if constexpr (left) {
        new_endpoint->cut_left();
    } else {
        new_endpoint->cut_right();
    }
    if (is_down) {
        move_in_dictionaries(new_endpoint, nc_dict, max_dict);
    } else {
        move_in_dictionaries(new_endpoint, nc_dict, min_dict);
    }
    if constexpr (left) {
        persistence.replace_left_endpoint(new_endpoint);
        left_endpoint = new_endpoint;
    } else {
        persistence.replace_right_endpoint(new_endpoint);
        right_endpoint = new_endpoint;
    }
    return old_endpoint;

}
namespace bananas {
    template list_item* interval::delete_endpoint_impl<true>();
    template list_item* interval::delete_endpoint_impl<false>();
}

//
// Private methods related to value changes
//

void interval::update_non_critical_value(list_item* item, function_value_type value) {
    auto left_val = item->left_neighbor()->value<1>();
    auto right_val = item->right_neighbor()->value<1>();
    if ((left_val < value && value < right_val) ||
            (left_val > value && value > right_val)) {
        // `item` remains non-critical
        item->assign_value(value);
        return;
    }
    if (value > item->value<1>()) {
        increase_non_critical_value(item, value);
    } else {
        decrease_non_critical_value(item, value);
    }
}

void interval::increase_non_critical_value(list_item* item, function_value_type value) {
    massert(item->is_noncritical<1>(), "Expected `item` to be non-critical.");
    massert(nc_dict.contains(*item), "Expected `item` to be in the dictionary of non-critical items.");
    massert(value > item->value<1>(), "Expected the item's value to increase.");
    massert(value > item->right_neighbor()->value<1>() && value > item->left_neighbor()->value<1>(),
            "Expected `item` to become critical.");
    auto* high_neighbor = item->high_neighbor();
    if (high_neighbor->is_maximum<1>() || high_neighbor->is_down_type<1>()) {
        // Max slide
        item->assign_value(value);
        persistence.max_slide(high_neighbor, item);
        persistence.on_increase_value_of_maximum(item);
        // TODO: this can be simplified to swapping the items in the dictionaries
        if (high_neighbor->is_endpoint()) {
            move_in_dictionaries(high_neighbor, max_dict, min_dict);
        } else {
            move_in_dictionaries(high_neighbor, max_dict, nc_dict);
        }
        move_in_dictionaries(item, nc_dict, max_dict);
    } else {
        // Anticancellation where `item` becomes a maximum and `high_neighbor` a minimum.
        item->assign_value(add_tiniest_offset<1>(high_neighbor->value<1>()));
        persistence.anticancel(min_dict, max_dict, {high_neighbor, item});

        item->assign_value(value);
        persistence.on_increase_value_of_maximum(item);
        move_in_dictionaries(item, nc_dict, max_dict);
        move_in_dictionaries(high_neighbor, nc_dict, min_dict);
    }
}

void interval::decrease_non_critical_value(list_item* item, function_value_type value) {
    massert(item->is_noncritical<1>(), "Expected `item` to be non-critical.");
    massert(nc_dict.contains(*item), "Expected `item` to be in the dictionary of non-critical items.");
    massert(value < item->value<1>(), "Expected the item's value to decrease.");
    massert(value < item->right_neighbor()->value<1>() && value < item->left_neighbor()->value<1>(),
            "Expected `item` to become critical.");
    auto* low_neighbor = item->low_neighbor();
    if (low_neighbor->is_minimum<1>() || low_neighbor->is_up_type<1>()) {
        // Min slide
        item->assign_value(value);
        persistence.min_slide(low_neighbor, item);
        persistence.on_decrease_value_of_minimum(item);
        if (low_neighbor->is_endpoint()) {
            move_in_dictionaries(low_neighbor, min_dict, max_dict);
        } else {
            move_in_dictionaries(low_neighbor, min_dict, nc_dict);
        }
        move_in_dictionaries(item, nc_dict, min_dict);
    } else {
        // Anticancellation where `item` becomes a minimum and `low_neighbor` a maximum
        item->assign_value(add_tiniest_offset<-1>(low_neighbor->value<1>()));
        persistence.anticancel(min_dict, max_dict, {item, low_neighbor});
        
        item->assign_value(value);
        persistence.on_decrease_value_of_minimum(item);
        move_in_dictionaries(item, nc_dict, min_dict);
        move_in_dictionaries(low_neighbor, nc_dict, max_dict);
    }
}

void interval::update_critical_value(list_item* item, function_value_type value) {
    massert(item->is_internal(), "Attempting to update critical value of endpoint.");
    auto value_increased = value > item->value<1>();
    // TODO: on increase of minima/decrease of maxima might want to split this into "make non-critical" and "update_non_critical_value"
    if (value_increased) {
        if (item->is_maximum<1>()) {
            // Criticality doesn't change and we can go directly to the new value
            item->assign_value(value);
            persistence.on_increase_value_of_maximum(item);
        } else {
            increase_minimum(item, value);
        }
    } else {
        if (item->is_maximum<1>()) {
            decrease_maximum(item, value);
        } else {
            item->assign_value(value);
            persistence.on_decrease_value_of_minimum(item);
        }
    }
}

void interval::increase_minimum(list_item* item, function_value_type value) {
    massert(item->is_minimum<1>(), "Expected a minimum for `item`");
    if (item->left_neighbor()->value<1>() > value &&
            item->right_neighbor()->value<1>() > value) {
        // Criticality doesn't change
        item->assign_value(value);
        persistence.on_increase_value_of_minimum(item);
        massert(item->is_minimum<1>(), "Expected `item` to remain a minimum.");
    } else {
        auto* low_neighbor = item->low_neighbor();
        auto need_to_slide = low_neighbor->is_noncritical<1>();
        // Criticality changes from minimum to something else
        // Split the update into two:
        // 1. make the item non-critical
        // 2. update the non-critical item
        item->assign_value(add_tiniest_offset< -1>(low_neighbor->value<1>()));
        persistence.on_increase_value_of_minimum(item);
        if (need_to_slide) {
            item->assign_value(add_tiniest_offset<1>(low_neighbor->value<1>()));
            persistence.min_slide(item, low_neighbor);
            item->interpolate_neighbors();
            move_in_dictionaries(item, min_dict, nc_dict);
            move_in_dictionaries(low_neighbor, nc_dict, min_dict);
        } else if (low_neighbor->is_internal()) {
            persistence.cancel(item, low_neighbor);
            item->interpolate_neighbors();
            move_in_dictionaries(item, min_dict, nc_dict);
            move_in_dictionaries(low_neighbor, max_dict, nc_dict);
        } else {
            persistence.cancel_min_with_endpoint(item, low_neighbor);
            item->interpolate_neighbors();
            // update dictionaries: `low_neighbor` max_dict -> min_dict; `item` min_dict -> nc_dict
            move_in_dictionaries(low_neighbor, max_dict, min_dict);
            move_in_dictionaries(item, min_dict, nc_dict);
        }
        update_non_critical_value(item, value);
    }
}

void interval::decrease_maximum(list_item* item, function_value_type value) {
    massert(item->is_maximum<1>(), "Expected a maximum for `item`");
    if (item->left_neighbor()->value<1>() < value &&
            item->right_neighbor()->value<1>() < value) {
        // `item` remains a maximum
        item->assign_value(value);
        persistence.on_decrease_value_of_maximum(item);
        massert(item->is_maximum<1>(), "Expected `item` to remain a maximum.");
    } else {
        auto* high_neighbor = item->high_neighbor();
        auto need_to_slide  = high_neighbor->is_noncritical<1>();
        // `item` becomes non-critical or a minimum
        item->assign_value(add_tiniest_offset<1>(high_neighbor->value<1>()));
        persistence.on_decrease_value_of_maximum(item);
        if (need_to_slide) {
            item->assign_value(add_tiniest_offset<-1>(high_neighbor->value<1>()));
            persistence.max_slide(item, high_neighbor);
            item->interpolate_neighbors();
            move_in_dictionaries(item, max_dict, nc_dict);
            move_in_dictionaries(high_neighbor, nc_dict, max_dict);
        } else if (high_neighbor->is_internal()) {
            persistence.cancel(high_neighbor, item);
            item->interpolate_neighbors();
            move_in_dictionaries(item, max_dict, nc_dict);
            move_in_dictionaries(high_neighbor, min_dict, nc_dict);
        } else {
            persistence.cancel_max_with_endpoint(item, high_neighbor);
            item->interpolate_neighbors();
            // update dictionaries: `high_neighbor` min_dict -> max_dict; `item` max_dict -> nc_dict
            move_in_dictionaries(high_neighbor, min_dict, max_dict);
            move_in_dictionaries(item, max_dict, nc_dict);
        }
        update_non_critical_value(item, value);
    }
}

void interval::update_value_of_endpoint(list_item* item, function_value_type value) {
    massert(item->is_endpoint(),
            "Changing value of an endpoint, but the given item isn't an endpoint.");
    auto value_increased = value > item->value<1>();
    auto is_left = item->is_left_endpoint();
    auto* neighbor_item = is_left ? item->right_neighbor() :
                                   item->left_neighbor();
    auto neighbor_value = neighbor_item->value<1>();
    if (value_increased) {
        if (item->is_down_type<1>()) {
            // No change in criticality
            item->assign_value(value);
            persistence.on_increase_value_of_maximum(item);
        } else {
            if (value > neighbor_value) {
                // `item` was an up-type item and becomes an down-type item
                // First, move `item` as close as possible to its neighbor.
                // Need to `add_tiniest_offset` twice to accomodate for the hook.
                item->assign_value(add_tiniest_offset<-1>(add_tiniest_offset<-1>(neighbor_value)));
                persistence.on_increase_value_of_minimum(item);
                // Now turn it into a down-type item
                item->assign_value(value);
                persistence.change_up_to_down(item, neighbor_item);
                persistence.on_increase_value_of_maximum(item);
                // TODO: update dictionaries: `item` min_dict -> max_dict;
                // for `neighbor_item:`
                // if `neighbor_item->is_non_critical` then max_dict -> nc_dict
                // else `neighbor_item->is_minimum` and nc_dict -> min_dict
                move_in_dictionaries(item, min_dict, max_dict);
                if (neighbor_item->is_minimum<1>()) {
                    move_in_dictionaries(neighbor_item, nc_dict, min_dict);
                } else {
                    massert(neighbor_item->is_noncritical<1>(),
                            "Expected neighbor of updated endpoint to be non-critical if not a minimum.");
                    move_in_dictionaries(neighbor_item, max_dict, nc_dict);
                }
            } else {
                // No change in criticality; `item` is still an up-type item
                item->assign_value(value);
                persistence.on_increase_value_of_minimum(item);
            }
        }
    } else {
        if (item->is_down_type<1>()) {
            if (value < neighbor_value) {
                // `item` was a down-type item and becomes an up-type item
                // First, move `item` as close as possible to its neighbor.
                // Need to add tiniest offset twice to accomodate for the hook.
                item->assign_value(add_tiniest_offset<1>(add_tiniest_offset<1>(neighbor_value)));
                persistence.on_decrease_value_of_maximum(item);
                // Now turn it into an up-type item
                item->assign_value(value);
                persistence.change_down_to_up(item, neighbor_item);
                persistence.on_decrease_value_of_minimum(item);
                // TODO: update dictionaries: `item` max_dict -> min_dict:
                // for `neighbor_item`:
                // if `neighbor_item->is_non_critical` then min_dict -> nc_dict
                // else `neighbor_item->is_maximum` and nc_dict -> max_dict
                move_in_dictionaries(item, max_dict, min_dict);
                if (neighbor_item->is_maximum<1>()) {
                    move_in_dictionaries(neighbor_item, nc_dict, max_dict);
                } else {
                    massert(neighbor_item->is_noncritical<1>(),
                            "Expected neighbor of updated endpoint to be non-critical if not a minimum.");
                    move_in_dictionaries(neighbor_item, min_dict, nc_dict);
                }
            } else {
                // No change in criticality; `item` is still a down-type item
                item->assign_value(value);
                persistence.on_decrease_value_of_maximum(item);
            }
        } else {
            // No change in criticality
            item->assign_value(value);
            persistence.on_decrease_value_of_minimum(item);
        }
    }
}

//
// Implementation of topological maintenance operations
//

interval& interval::glue(interval& left_interval, interval& right_interval) {
    massert(*right_interval.left_endpoint > *left_interval.right_endpoint, \
            "Expected the items of `right_interval` to be to the right of the items of `left_interval`.");
    
    // Glue the dictionaries
    left_interval.max_dict.join(right_interval.max_dict);
    left_interval.min_dict.join(right_interval.min_dict);
    left_interval.nc_dict.join(right_interval.nc_dict);

    // Glue the persistence data structure
    left_interval.persistence.glue_to_right(right_interval.persistence,
                                            left_interval.min_dict, left_interval.max_dict);
    
    auto* const endpoint_l = left_interval.right_endpoint;
    auto* const endpoint_r = right_interval.left_endpoint;

    // Glue the list (`left_interval.right_endpoint` gets right neighbor `right_interval.left_endpoint`)
    list_item::link(*left_interval.right_endpoint, *right_interval.left_endpoint);
    left_interval.right_endpoint = right_interval.right_endpoint;

    right_interval.left_endpoint = nullptr;
    right_interval.right_endpoint = nullptr;

    // Update items in the dictionary
    update_dicts_on_glue(endpoint_l,
                         endpoint_r,
                         left_interval.min_dict,
                         left_interval.max_dict,
                         left_interval.nc_dict);

    return left_interval;
}

void interval::update_dicts_on_glue(list_item* endpoint_l,
                                    list_item* endpoint_r,
                                    min_dictionary &min_dict,
                                    max_dictionary &max_dict,
                                    nc_dictionary &nc_dict) {
    const bool l_is_down = endpoint_l->value<1>() > endpoint_l->left_neighbor()->value<1>(); // endpoint_l->is_down_type<1>();
    const bool r_is_down = endpoint_r->value<1>() > endpoint_r->right_neighbor()->value<1>(); //endpoint_r->is_down_type<1>();
    if (l_is_down && r_is_down) {
        if (endpoint_l->value<1>() > endpoint_r->value<1>()) {
            // `endpoint_l` remains critical, `endpoint_r` becomes non-critical
            max_dict.erase_item(*endpoint_r);
            nc_dict.insert_item(*endpoint_r);
        } else {
            // `endpoint_l` becomes non-critical, `endpoint_r` becomes critical
            max_dict.erase_item(*endpoint_l);
            nc_dict.insert_item(*endpoint_l);
        }
    } else if (l_is_down && !r_is_down) {
        if (endpoint_l->value<1>() < endpoint_r->value<1>()) {
            // Both become non-critical
            max_dict.erase_item(*endpoint_l);
            min_dict.erase_item(*endpoint_r);
            nc_dict.insert_item(*endpoint_l);
            nc_dict.insert_item(*endpoint_r);
        }
    } else if (!l_is_down && r_is_down) {
        if (endpoint_l->value<1>() > endpoint_r->value<1>()) {
            // Both become non-critical
            min_dict.erase_item(*endpoint_l);
            max_dict.erase_item(*endpoint_r);
            nc_dict.insert_item(*endpoint_l);
            nc_dict.insert_item(*endpoint_r);
        }
    } else if (!l_is_down && !r_is_down) {
        if (endpoint_l->value<1>() > endpoint_r->value<1>()) {
            // `endpoint_l` becomes non-critical
            min_dict.erase_item(*endpoint_l);
            nc_dict.insert_item(*endpoint_l);
        } else {
            // `endpoint_r` becomes non-critical
            min_dict.erase_item(*endpoint_r);
            nc_dict.insert_item(*endpoint_r);
        }
    }
}

// TODO: need an item pool in order to allocate new items.
interval interval::cut(list_item* cut_item, recycling_object_pool<list_item>& item_pool) {
    massert(cut_item->right_neighbor() != nullptr, "Expected `cut_item` to have a right neighbor.");
    massert(!cut_item->right_neighbor()->is_endpoint(), "Expected to cut away from an endpoint.");

    // create new items
    auto* left_of_cut = item_pool.construct((2*cut_item->get_interval_order() + cut_item->right_neighbor()->get_interval_order()) / 3,
                                            (cut_item->value<1>() + cut_item->right_neighbor()->value<1>()) / 2);
    auto* right_of_cut = item_pool.construct((cut_item->get_interval_order() + 2*cut_item->right_neighbor()->get_interval_order()) / 3,
                                             (cut_item->value<1>() + cut_item->right_neighbor()->value<1>()) / 2);
    // insert `left_of_cut`, `right_of_cut` into the list of items
    auto* right_neighbor = cut_item->right_neighbor();
    cut_item->cut_right();
    list_item::link(*cut_item, *left_of_cut);
    list_item::link(*left_of_cut, *right_of_cut);
    list_item::link(*right_of_cut, *right_neighbor);
    if (cut_item->value<1>() < right_neighbor->value<1>()) {
        // Value increases from left to right, so we make `left_of_cut` a maximum and `right_of_cut` a minimum.
        left_of_cut->assign_value(add_tiniest_offset<1>(left_of_cut->value<1>()));
        right_of_cut->assign_value(add_tiniest_offset<-1>(right_of_cut->value<1>()));
        max_dict.insert_item(*left_of_cut);
        min_dict.insert_item(*right_of_cut);
    } else {
        // Value decreases from left to right, so we make `left_of_cut` a minimum and `right_of_cut` a maximum.
        left_of_cut->assign_value(add_tiniest_offset<-1>(left_of_cut->value<1>()));
        right_of_cut->assign_value(add_tiniest_offset<1>(right_of_cut->value<1>()));
        min_dict.insert_item(*left_of_cut);
        max_dict.insert_item(*right_of_cut);
    }
    // Initialize the new interval by cutting the persistence data structure.
    // This also cuts the link between `left_of_cut` and `right_of_cut`.
    interval new_interval(persistence.cut(*left_of_cut, *right_of_cut, min_dict, max_dict));

    // split dictionaries and update endpoints
    if (new_interval.left_endpoint == left_endpoint) {
        // The new interval is the left interval
        min_dict.cut_left(*right_of_cut, new_interval.min_dict);
        max_dict.cut_left(*right_of_cut, new_interval.max_dict);
        nc_dict.cut_left(*right_of_cut, new_interval.nc_dict);
        left_endpoint = right_of_cut;
        massert(new_interval.right_endpoint == left_of_cut, "Expected endpoints of new interval to be updated already.");
    } else {
        // The new interval is the right interval
        min_dict.cut_right(*right_of_cut, new_interval.min_dict);
        max_dict.cut_right(*right_of_cut, new_interval.max_dict);
        nc_dict.cut_right(*right_of_cut, new_interval.nc_dict);
        right_endpoint = left_of_cut;
        massert(new_interval.left_endpoint == right_of_cut, "Expected endpoints of new interval to be updated already.");
    }

    return new_interval;
}

void interval::compute_persistence_diagram(persistence_diagram& diagram) const {
    persistence.extract_persistence_diagram(diagram);
}

//
//
//

const banana_tree<1>& interval::get_up_tree() const {
    return persistence.get_up_tree();
}

const banana_tree< -1>& interval::get_down_tree() const {
    return persistence.get_down_tree();
}

list_item* interval::get_left_endpoint() const {
    return left_endpoint;
}

list_item* interval::get_right_endpoint() const {
    return right_endpoint;
}

//
// Methods related to analysis
//

void interval::compute_statistics() {
    interval_stats.reset();
    for ([[maybe_unused]] const auto& item: *this) {
        interval_stats.increment_count(interval_statistics::idx_num_items);
        if (item.left_neighbor() != nullptr) {
            interval_stats.add_variation(item.left_neighbor()->value<1>(), item.value<1>());
        }
}
    for (auto* end : {left_endpoint, right_endpoint}) {
        if (end->is_down_type<1>()) {
            interval_stats.increment_count(interval_statistics::idx_num_hooks);
        }
    }
    analyze_banana_trees();
}

void interval::print_statistics(multirow_csv_writer& writer) {
    interval_stats.print(writer);
}

void interval::analyze_banana_trees() {
    using up_node_t = const up_tree_node*;
    using down_node_t = const down_tree_node*;
    map_banana_dfs(persistence.get_up_tree(),
                   [this](up_node_t min,
                          up_node_t max,
                          int nesting_depth,
                          int node_depth)
    {
        // Count nodes in the up-tree: a minimum and a maximum for each banana.
        interval_stats.increment_count(interval_statistics::idx_num_nodes);
        interval_stats.increment_count(interval_statistics::idx_num_nodes);
        // If the current banana is empty, then we have a leaf window.
        if (min->has_empty_banana()) {
            interval_stats.new_dist_value(interval_statistics::idx_nesting_depth, 1, nesting_depth);
            interval_stats.increment_count(interval_statistics::idx_leaf_bananas_up);
        }
        // Depth of the current maximum.
        interval_stats.new_dist_value(interval_statistics::idx_node_depth, 1, node_depth);
        // Compute trail lengths
        int length = 0;
        map_in_trail(max, [&length](up_node_t) { length++; });
        interval_stats.new_dist_value(interval_statistics::idx_length_of_in_trail, 1, length);
        length = 0;
        map_mid_trail(max, [&length](up_node_t) { length++; });
        interval_stats.new_dist_value(interval_statistics::idx_length_of_mid_trail, 1, length);
    });
    map_banana_dfs(persistence.get_down_tree(),
                   [this](down_node_t min,
                          down_node_t max,
                          int nesting_depth,
                          int node_depth)
    {
        if (min->has_empty_banana()) {
            interval_stats.new_dist_value(interval_statistics::idx_nesting_depth, -1, nesting_depth);
            interval_stats.increment_count(interval_statistics::idx_leaf_bananas_down);
        }
        interval_stats.new_dist_value(interval_statistics::idx_node_depth, -1, node_depth);
        int length = 0;
        map_in_trail(max, [&length](down_node_t) { length++; });
        interval_stats.new_dist_value(interval_statistics::idx_length_of_in_trail, -1, length);
        length = 0;
        map_mid_trail(max, [&length](down_node_t) { length++; });
        interval_stats.new_dist_value(interval_statistics::idx_length_of_mid_trail, -1, length);
    });
    auto* node_up = persistence.get_up_tree().get_special_root();
    while (!node_up->get_in()->is_leaf()) {
        interval_stats.increment_count(interval_statistics::idx_short_wave_left_up);
        node_up = node_up->get_in();
    }
    node_up = persistence.get_up_tree().get_special_root()->get_mid();
    if (!node_up->is_leaf()) {
        interval_stats.increment_count(interval_statistics::idx_short_wave_right_up);
        while (!node_up->get_in()->is_leaf()) {
            interval_stats.increment_count(interval_statistics::idx_short_wave_right_up);
            node_up = node_up->get_in();   
        }
    }
    auto* node_down = persistence.get_down_tree().get_special_root();
    while (!node_down->get_in()->is_leaf()) {
        interval_stats.increment_count(interval_statistics::idx_short_wave_left_down);
        node_down = node_down->get_in();
    }
    node_down = persistence.get_down_tree().get_special_root()->get_mid();
    if (!node_down->is_leaf()) {
        interval_stats.increment_count(interval_statistics::idx_short_wave_right_down);
        while (!node_down->get_in()->is_leaf()) {
            interval_stats.increment_count(interval_statistics::idx_short_wave_right_down);
            node_down = node_down->get_in();   
        }
    }
}

//
// Methods related to iteration
//

interval_iterator interval::begin() {
    return {left_endpoint, list_item::direction::right};
}

interval_iterator interval::end() {
    return {nullptr, list_item::direction::right};
}

interval_iterator interval::rbegin() {
    return {right_endpoint, list_item::direction::left};
}

interval_iterator interval::rend() {
    return {nullptr, list_item::direction::left};
}

interval_iterator interval::iterator_to(list_item &item) {
    return {&item, list_item::direction::right};
}

interval_iterator interval::r_iterator_to(list_item &item) {
    return {&item, list_item::direction::left};
}

interval::critical_item_iter_pair interval::critical_items() {
    return critical_item_iter_pair{left_endpoint, right_endpoint};
}

//
// Implementation of helper for `interval_critical_iterator`
//

interval_critical_iterator interval::critical_item_iter_pair::begin() {
    return {left_endpoint, list_item::direction::right};
}

interval_critical_iterator interval::critical_item_iter_pair::end() {
    return {nullptr, list_item::direction::right};
}

interval_critical_iterator interval::critical_item_iter_pair::rbegin() {
    return {right_endpoint, list_item::direction::left};
}

interval_critical_iterator interval::critical_item_iter_pair::rend() {
    return {nullptr, list_item::direction::left};
}

//
// `interval_iterator` implementation
//

interval_iterator::interval_iterator(list_item* start,
                                     list_item::direction dir) : pointed_item(start),
                                                                 dir(dir) {}

interval_iterator& interval_iterator::operator++() {
    pointed_item = pointed_item->neighbor(dir);
    return *this;
}

interval_iterator interval_iterator::operator++(int) {
    return {pointed_item->neighbor(dir), dir};
}

interval_iterator::reference interval_iterator::operator*() const {
    return *pointed_item;
}

interval_iterator::pointer interval_iterator::operator->() const {
    return pointed_item;
}

bool bananas::operator==(const interval_iterator &a, const interval_iterator &b) {
    return a.pointed_item == b.pointed_item && a.dir == b.dir;
}

bool bananas::operator!=(const interval_iterator &a, const interval_iterator &b) {
    return a.pointed_item != b.pointed_item || a.dir != b.dir;
}


//
// `interval_critical_iterator` implementation
//

interval_critical_iterator::interval_critical_iterator(
        list_item* start,
        list_item::direction dir) : pointed_item(start),
                                    dir(dir) {}

interval_critical_iterator& interval_critical_iterator::operator++() {
    do {
        pointed_item = pointed_item->neighbor(dir);
    } while (pointed_item != nullptr && pointed_item->is_noncritical<1>());
    return *this;
}

interval_critical_iterator interval_critical_iterator::operator++(int) {
    auto* next_item = pointed_item;
    do {
        next_item = next_item->neighbor(dir);
    } while (next_item != nullptr && next_item->is_noncritical<1>());
    return {next_item, dir};
}

interval_critical_iterator::reference interval_critical_iterator::operator*() const {
    return *pointed_item;
}

interval_critical_iterator::pointer interval_critical_iterator::operator->() const {
    return pointed_item;
}

bool bananas::operator==(const interval_critical_iterator &a, const interval_critical_iterator &b) {
    return a.pointed_item == b.pointed_item && a.dir == b.dir;
}

bool bananas::operator!=(const interval_critical_iterator &a, const interval_critical_iterator &b) {
    return !(a == b);
}



interval_id_t interval_statistics::next_interval_id = 0;

void interval_statistics::print(multirow_csv_writer& writer) {
    writer << std::make_pair("id", interval_id)
           << std::make_pair("items", counts[idx_num_items])
           << std::make_pair("nodes", counts[idx_num_nodes])
           << std::make_pair("hooks", counts[idx_num_hooks])
           << std::make_pair("short_wave_left_up", counts[idx_short_wave_left_up])
           << std::make_pair("short_wave_right_up", counts[idx_short_wave_right_up])
           << std::make_pair("short_wave_left_down", counts[idx_short_wave_left_down])
           << std::make_pair("short_wave_right_down", counts[idx_short_wave_right_down])
           << std::make_pair("leaf_bananas_up", counts[idx_leaf_bananas_up])
           << std::make_pair("leaf_bananas_down", counts[idx_leaf_bananas_down])
           << std::make_pair("total_var", total_variation);
    distributions[idx_length_of_in_trail].print_with_name(writer, "length_in");
    distributions[idx_length_of_mid_trail].print_with_name(writer, "length_mid");
    distributions[idx_nesting_depth].print_with_name(writer, "nesting_depth");
    distributions[idx_node_depth].print_with_name(writer, "node_depth");
}



