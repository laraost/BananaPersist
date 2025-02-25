#include <memory>
#include <unordered_set>
#include <utility>

#include "datastructure/banana_tree.h"
#include "datastructure/interval.h"
#include "datastructure/list_item.h"
#include "datastructure/persistence_context.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/format_util.h"
#include "utility/recycling_object_pool.h"
#include "utility/stats.h"
#include "utility/types.h"

namespace bananas {

class persistence_context_impl {

public:
    interval* new_interval(const std::vector<function_value_type> &values, const optional_vector_ref<list_item*> &item_vector,
                           const interval_order_type initial_order) {
        massert(values.size() >= 2, "An interval needs at least two items");

        // Turn the list of values into a list of `list_item`s.
        auto *left_endpoint = allocate_item(initial_order, values[0]);
        if (item_vector.has_value()) {
            item_vector->get().push_back(left_endpoint);
        }
        auto *prev_item = left_endpoint;
        for (size_t idx = 1; idx < values.size(); ++idx) {
            auto *new_item = allocate_item(initial_order + idx, values[idx]);
            if (item_vector.has_value()) {
                item_vector->get().push_back(new_item);
            }
            list_item::link(*prev_item, *new_item);
            prev_item = new_item;
        }

        auto* new_interval = interval_pool.construct(up_tree_node_pool, down_tree_node_pool, std::make_pair(left_endpoint, prev_item));
        interval_ptr_set.insert(new_interval);
        return new_interval;
    }

    void change_value(interval* interval, list_item* item, function_value_type new_value) {
        interval->update_value(item, new_value);
    }

    list_item* insert_item(interval* interval, interval_order_type order) {
        return interval->insert_item(order, list_item_pool);
    }
    list_item* insert_item_right_of(interval* interval, list_item* item) {
        return interval->insert_item_to_right_of(item, list_item_pool);
    }
    list_item* insert_right_endpoint(interval* interval, interval_order_type order_offset, function_value_type value) {
        return interval->insert_right_endpoint(value, order_offset, list_item_pool);
    }
    list_item* insert_left_endpoint(interval* interval, interval_order_type order_offset, function_value_type value) {
        return interval->insert_left_endpoint(value, order_offset, list_item_pool);
    }

    void delete_item(interval* interval, list_item* item) {
        if (item == interval->get_right_endpoint()) {
            interval->delete_right_endpoint();
        } else if (item == interval->get_left_endpoint()) {
            interval->delete_left_endpoint();
        } else {
            interval->delete_internal_item(item);
        }
        list_item_pool.free(item);
    }
    void delete_right_endpoint(interval* interval) {
        auto* deleted_item = interval->delete_right_endpoint();
        list_item_pool.free(deleted_item);
    }
    void delete_left_endpoint(interval* interval) {
        auto* deleted_item = interval->delete_left_endpoint();
        list_item_pool.free(deleted_item);
    }

    void delete_interval(interval* interval) {
        auto* left_endpoint = interval->get_left_endpoint();
        interval_pool.free(interval);
        interval_ptr_set.erase(interval);
        for (auto* item_ptr = left_endpoint; item_ptr != nullptr; item_ptr = item_ptr->right_neighbor()) {
            list_item_pool.free(item_ptr);
        }
    }

    std::pair<interval*, interval*> cut_interval(interval* interval, list_item* cut_item) {
        massert(!cut_item->is_right_endpoint(), "");
        auto* new_interval = interval_pool.construct(interval->cut(cut_item, list_item_pool));
        interval_ptr_set.insert(new_interval);
        if (*new_interval->get_left_endpoint() < *interval->get_left_endpoint()) {
            return {new_interval, interval};
        } else {
            return {interval, new_interval};
        }
    }

    void glue_intervals(interval* left_interval, interval* right_interval) {
        massert(left_interval != right_interval, "Cannot glue an interval to itself.");
        massert(*(left_interval->get_right_endpoint()) < *(right_interval->get_left_endpoint()),
                "Expected `left_interval` to actually be to the left of `right_interval`.");
        interval::glue(*left_interval, *right_interval);
        delete_interval(right_interval);
    }

    void compute_persistence_diagram(persistence_diagram &diagram) const {
        diagram.clear_diagrams();
        for (auto* ival: interval_ptr_set) {
            ival->compute_persistence_diagram(diagram);
        }
    }

    void analyse_all_intervals(multirow_csv_writer& writer) const {
        for (auto* interval: interval_ptr_set) {
            writer.new_row();
            interval->compute_statistics();
            interval->print_statistics(writer);
        }
    }

    size_t get_num_intervals() const {
        return interval_ptr_set.size();
    }

    void print_memory_stats(std::ostream &stream) const {
        csv_writer writer;
        print_memory_stats(writer);
        writer.write_to_stream_and_reset(stream);
    }

    void print_memory_stats(csv_writer &writer) const {
        writer << std::make_pair("allocs_list_item_pool", list_item_pool.get_number_of_allocations())
               << std::make_pair("allocs_up_node_pool", up_tree_node_pool.get_number_of_allocations())
               << std::make_pair("allocs_down_node_pool", down_tree_node_pool.get_number_of_allocations())
               << std::make_pair("allocs_interval_pool", interval_pool.get_number_of_allocations())
               << std::make_pair("recycled_list_items", list_item_pool.get_number_of_recyclings())
               << std::make_pair("recycled_up_nodes", up_tree_node_pool.get_number_of_recyclings())
               << std::make_pair("recycled_down_nodes", down_tree_node_pool.get_number_of_recyclings());
    }

private:
    recycling_object_pool<list_item> list_item_pool;
    recycling_object_pool<banana_tree_node<1>> up_tree_node_pool;
    recycling_object_pool<banana_tree_node<-1>> down_tree_node_pool;
    recycling_object_pool<interval> interval_pool;

    std::unordered_set<interval*> interval_ptr_set;

    // Private methods

    list_item* allocate_item(const function_value_type &value) {
        return list_item_pool.construct(value);
    }
    list_item* allocate_item(const interval_order_type &order, const function_value_type &value) {
        return list_item_pool.construct(order, value);
    }

};

}

using namespace bananas;

persistence_context::persistence_context() : pimpl(std::make_unique<persistence_context_impl>()) {}

persistence_context::~persistence_context() = default;

interval* persistence_context::new_interval(const std::vector<function_value_type> &values,
                                            const optional_vector_ref<list_item*> &item_vector,
                                            const interval_order_type initial_order) {
    return pimpl->new_interval(values, item_vector, initial_order);
}

void persistence_context::change_value(interval* interval,
                                       list_item* item,
                                       function_value_type new_value) {
    pimpl->change_value(interval, item, new_value);
}

list_item* persistence_context::insert_item(interval* interval, interval_order_type order) {
    return pimpl->insert_item(interval, order);
}

list_item* persistence_context::insert_item_right_of(interval* interval, list_item* item) {
    return pimpl->insert_item_right_of(interval, item);
}
list_item* persistence_context::insert_right_endpoint(interval* interval, interval_order_type order_offset, function_value_type value) {
    return pimpl->insert_right_endpoint(interval, order_offset, value);
}
list_item* persistence_context::insert_left_endpoint(interval* interval, interval_order_type order_offset, function_value_type value) {
    return pimpl->insert_left_endpoint(interval, order_offset, value);
}

void persistence_context::delete_item(interval* interval, list_item* item) {
    pimpl->delete_item(interval, item);
}

void persistence_context::delete_right_endpoint(interval* interval) {
    pimpl->delete_right_endpoint(interval);
}

void persistence_context::delete_left_endpoint(interval* interval) {
    pimpl->delete_left_endpoint(interval);
}

std::pair<interval*, interval*> persistence_context::cut_interval(interval* interval, list_item* cut_item) {
    return pimpl->cut_interval(interval, cut_item);
}

void persistence_context::glue_intervals(interval* left_interval, interval* right_interval) {
    pimpl->glue_intervals(left_interval, right_interval);
}

void persistence_context::delete_interval(interval* interval) {
    pimpl->delete_interval(interval);
}

void persistence_context::compute_persistence_diagram(persistence_diagram &diagram) const {
    pimpl->compute_persistence_diagram(diagram);
}

void persistence_context::analyse_all_intervals(multirow_csv_writer& writer) const {
    pimpl->analyse_all_intervals(writer);
}

void persistence_context::print_memory_stats(std::ostream &stream) const {
    pimpl->print_memory_stats(stream);
}
void persistence_context::print_memory_stats(csv_writer &writer) const {
    pimpl->print_memory_stats(writer);
}

bool persistence_context::is_non_critical(list_item* item) const {
    return item->is_noncritical<1>();
}
bool persistence_context::is_maximum(list_item* item) const {
    return item->is_maximum<1>();
}
bool persistence_context::is_minimum(list_item* item) const {
    return item->is_minimum<1>();
}

std::string persistence_context::criticality_as_string(list_item* item) const {
    if (item->is_noncritical<1>()) {
        return "nc";
    } else if (item->is_maximum<1>() || item->is_down_type<1>()) {
        return "max";
    } else {
        return "min";
    }
}

interval_order_type persistence_context::get_global_max_order(interval* interval) const {
    return interval->get_up_tree().get_global_max()->get_interval_order();
}

function_value_type persistence_context::get_global_max_value(interval* interval) const {
    return interval->get_up_tree().get_global_max()->value<1>();
}

interval_order_type persistence_context::get_global_min_order(interval* interval) const {
    return interval->get_down_tree().get_global_max()->get_interval_order();
}

function_value_type persistence_context::get_global_min_value(interval* interval) const {
    return interval->get_down_tree().get_global_max()->value<1>();
}

size_t persistence_context::get_num_intervals() const {
    return pimpl->get_num_intervals();
}

bool persistence_context::validate_num_items(interval* interval) const {
    std::vector<list_item*> critical_items;
    for (auto& item: interval->critical_items()) {
        critical_items.push_back(&item);
    }
    size_t count_up = 0;
    for (auto* node: interval->get_up_tree().string()) {
        if (!node->is_hook()) {
            count_up++;
        }
    }
    size_t count_down = 0;
    for (auto* node: interval->get_down_tree().string()) {
        if (!node->is_hook()) {
            count_down++;
        }
    }
    bool up_success = count_up == critical_items.size();
    bool down_success = count_down == critical_items.size();
    if (!up_success) {
        DEBUG_MSG("Number of nodes in the up tree does not match number of critical items: " << count_up << " vs. " << critical_items.size());
    }
    if (!down_success) {
        DEBUG_MSG("Number of nodes in the down tree does not match number of critical items: " << count_down << " vs. " << critical_items.size());
    }
    return up_success && down_success;
}
