#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/format_util.h"
#include "utility/types.h"

namespace bananas {

class interval;
class list_item;

class persistence_context_impl;

class persistence_context {

public:
    persistence_context();
    ~persistence_context();

    interval* new_interval(const std::vector<function_value_type> &values,
                           const optional_vector_ref<list_item*> &item_vector = std::nullopt,
                           const interval_order_type initial_order = 0);

    void change_value(interval* interval, list_item* item, function_value_type new_value);

    list_item* insert_item(interval* interval, interval_order_type order);
    list_item* insert_item_right_of(interval* interval, list_item* item);
    list_item* insert_right_endpoint(interval* interval, interval_order_type order_offset, function_value_type value);
    list_item* insert_left_endpoint(interval* interval, interval_order_type order_offset, function_value_type value);

    void delete_item(interval* interval, list_item* item);
    void delete_right_endpoint(interval* interval);
    void delete_left_endpoint(interval* interval);

    // Cut the given `interval` to the right of `cut_item`.
    // Returns a pair of interval pointers,
    // where the `first` points to the left interval
    // and `second` points to the right interval.
    std::pair<interval*, interval*> cut_interval(interval* interval, list_item* cut_item);

    void glue_intervals(interval* left_interval, interval* right_interval);

    void delete_interval(interval* interval);

    void compute_persistence_diagram(persistence_diagram &diagram) const;

    void analyse_all_intervals(multirow_csv_writer& writer) const;

    void print_memory_stats(std::ostream &stream) const;
    void print_memory_stats(csv_writer &writer) const;

    // Access to item properties
    bool is_non_critical(list_item* item) const;
    bool is_maximum(list_item* item) const;
    bool is_minimum(list_item* item) const;
    std::string criticality_as_string(list_item* item) const;

    // Access to interval properties
    interval_order_type get_global_max_order(interval* interval) const;
    function_value_type get_global_max_value(interval* interval) const;
    interval_order_type get_global_min_order(interval* interval) const;
    function_value_type get_global_min_value(interval* interval) const;

    size_t get_num_intervals() const;

    // Sanity checks
    bool validate_num_items(interval* interval) const;

private:
    std::unique_ptr<persistence_context_impl> pimpl;

};

}
