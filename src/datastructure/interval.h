#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>

#include "datastructure/banana_tree.h"
#include "datastructure/dictionary.h"
#include "datastructure/list_item.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/format_util.h"
#include "utility/recycling_object_pool.h"
#include "utility/stats.h"

namespace bananas {

// Class for recording statistics of intervals and their banana trees.
// Potentially not thread-safe
class interval_statistics {

    struct dist_var {
        std::array<long, 2> min;
        std::array<long, 2> max;
        std::array<long, 2> total;

        dist_var() {
            reset();
        }

        void reset() {
            min = {std::numeric_limits<int>::max(),
                   std::numeric_limits<int>::max()};
            max = {std::numeric_limits<int>::min(),
                   std::numeric_limits<int>::min()};
            total = {0, 0};
        }

        void new_value(int sign, long value) {
            min[detail::sign_to_index(sign)] = std::min(min[detail::sign_to_index(sign)], value);
            max[detail::sign_to_index(sign)] = std::max(max[detail::sign_to_index(sign)], value);
            total[detail::sign_to_index(sign)] += value;
        }

        friend std::ostream& operator<<(std::ostream& stream, const dist_var& d) {
            stream << d.min << ", " << d.max << ", " << d.total;
            return stream;
        }

        void print_with_name(multirow_csv_writer& writer, const std::string& name) {
            writer << std::make_pair(("min_" + name).c_str(), min)
                   << std::make_pair(("max_" + name).c_str(), max)
                   << std::make_pair(("sum_" + name).c_str(), total);
        }
    };

public:
    enum count_idx {
        idx_num_items = 0,
        idx_num_hooks,
        idx_num_nodes,
        idx_short_wave_left_up,
        idx_short_wave_right_up,
        idx_short_wave_left_down,
        idx_short_wave_right_down,
        idx_leaf_bananas_up,
        idx_leaf_bananas_down,
        num_count
    };

    enum dist_idx {
        idx_length_of_in_trail = 0,
        idx_length_of_mid_trail,
        idx_nesting_depth,
        idx_node_depth,
        dist_count
    };

    inline interval_statistics() : interval_id(next_interval_id) {
        next_interval_id++;
        reset();
    }

    void increment_count(const count_idx& idx) {
        counts[idx]++;
    }
    void decrement_count(const count_idx& idx) {
        counts[idx]--;
    }

    void new_dist_value(const dist_idx& idx, int sign, int value) {
        distributions[idx].new_value(sign, value);
    }

    void add_variation(function_value_type a, function_value_type b) {
        total_variation += std::abs(a - b);
    }

private:
    static interval_id_t next_interval_id;

    interval_id_t interval_id;

    std::array<size_t, count_idx::num_count> counts;
    std::array<dist_var, dist_idx::dist_count> distributions;
    function_value_type total_variation = 0;

public:
    inline void reset() {
        for (auto &c: counts) {
            c = 0;
        }
        for (auto &d: distributions) {
            d.reset();
        }
        total_variation = 0;
    }

    void print(multirow_csv_writer& writer);

};



class interval_iterator;
class interval_critical_iterator;

class interval {

public:
    struct critical_item_iter_pair {

            interval_critical_iterator begin();
            interval_critical_iterator end();
            interval_critical_iterator rbegin();
            interval_critical_iterator rend();

        protected:
            critical_item_iter_pair(list_item* left_endpoint,
                                    list_item* right_endpoint) :
                    left_endpoint(left_endpoint),
                    right_endpoint(right_endpoint) {}

            list_item* left_endpoint;
            list_item* right_endpoint;

            friend class interval;

    };




    interval(recycling_object_pool<up_tree_node> &up_tree_node_pool,
             recycling_object_pool<down_tree_node> &down_tree_node_pool);
    interval(recycling_object_pool<up_tree_node> &up_tree_node_pool,
             recycling_object_pool<down_tree_node> &down_tree_node_pool,
             list_item* left_endpoint,
             list_item* right_endpoint);
    interval(recycling_object_pool<up_tree_node> &up_tree_node_pool,
             recycling_object_pool<down_tree_node> &down_tree_node_pool,
             std::pair<list_item*, list_item*> endpoints);

    interval(const interval&) = delete;

    interval(interval && ival);

    // Construct the interval from the list of items given by `left_endpoint` and `right_endpoint`.
    void construct(list_item* left_endpoint, list_item* right_endpoint);

private:
    void insert_into_dicts();

public:
    // Set the value of `item` to `value`.
    void update_value(list_item* item, function_value_type value);

    // Insert an item at the given position
    // Expects the new item to be between the left and right endpoints.
    // The function value of the new item is obtained by interpolating its neighbors.
    list_item* insert_item(interval_order_type order, recycling_object_pool<list_item> &item_pool);
    // Insert a new item between the given item and its right neighbor.
    // Expects that `item` is not the right endpoint of the interval.
    // The interval order and function value of the new item
    // are obtained by interpolation between its two neighbors.
    list_item* insert_item_to_right_of(list_item* item, recycling_object_pool<list_item> &item_pool);

    // Insert a new right endpoint with the given function value
    // The order of the new endpoint is that of the old endpoint plus the offset.
    list_item* insert_right_endpoint(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool);
    // Insert a new left endpoint with the given function value
    // The order of the new endpoint is that of the old endpoint minus the offset.
    list_item* insert_left_endpoint(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool);

private: 
    // Insert a new endpoint (left endpoint if `left == true`, right endpoint otherwise) with the given `value`.
    // The order of the new endpoint is the order of the old endpoint plus the offset (offset is always added independent of the template parameter).
    template<bool left>
    list_item* insert_endpoint_impl(function_value_type value, interval_order_type offset, recycling_object_pool<list_item> &item_pool);

public:
    // Delete the given non-endpoint item.
    // If it is critical, make it non-critical first.
    // Whether or not `item` is actually in this interval is not checked;
    // if it is not then horrible things may happen.
    void delete_internal_item(list_item* item);

    // Delete the right endpoint.
    // Returns a pointer to the deleted endpoint.
    // Expectes that there are at least two items left after deletion.
    list_item* delete_right_endpoint();
    // Delete the left endpoint.
    // Returns a pointer to the deleted endpoint.
    // Expectes that there are at least two items left after deletion.
    list_item* delete_left_endpoint();

private:
    template<bool left>
    list_item* delete_endpoint_impl();

public:
    //
    // Topological maintenance
    //

    // Glue the interval given by `right_interval` to the right of the interval
    // given by `left_interval`.
    // After the gluing `left_interval` represents the combined interval
    // and `right_interval` is empty.
    // Returns a reference to the glued interval.
    static interval& glue(interval &left_interval, interval &right_interval);

    // Cut `this` interval between `cut_item` and its right neighbor.
    // Returns the interval with items up to `cut_item` if `cut_item` is left of the global min
    // and with items to the right of `cut_item` if `cut_item` is right of the global min or the global min.
    // Two new items are inserted between `cut_item` and its right neighbor,
    // which become the endpoints of the new intervals.
    interval cut(list_item* cut_item, recycling_object_pool<list_item> &item_pool);

    void compute_persistence_diagram(persistence_diagram& diagram) const;

    //
    //
    //

    [[nodiscard]] const banana_tree<1>& get_up_tree() const;
    [[nodiscard]] const banana_tree< -1>& get_down_tree() const;

    [[nodiscard]] list_item* get_left_endpoint() const;
    [[nodiscard]] list_item* get_right_endpoint() const;

    //
    // Iteration over intervals.
    //

    interval_iterator begin();
    interval_iterator end();
    interval_iterator rbegin();
    interval_iterator rend();

    // Obtain a pair of iterators over critical items in the interval.
    critical_item_iter_pair critical_items();

    static interval_iterator iterator_to(list_item &item);
    static interval_iterator r_iterator_to(list_item &item);

    // Analysis
    
    void compute_statistics();

    void print_statistics(std::ostream &stream);

    void print_statistics(multirow_csv_writer& writer);

private:
    void analyze_banana_trees();
    
private:
    persistence_data_structure persistence;

    interval_statistics interval_stats;

    min_dictionary min_dict;
    max_dictionary max_dict;
    nc_dictionary nc_dict;

    list_item* left_endpoint;
    list_item* right_endpoint;

    // Private constructor for creating an interval wrapping around an existing `persistence_data_structure`,
    // intended for use in cut.
    // Note: `min_dict`, `max_dict`, `nc_dict` need to be initialized separately.
    interval(persistence_data_structure &&pds);

    void update_non_critical_value(list_item* item, function_value_type value);

    // Increase the value of `item` to the given value until `item` becomes critical.
    void increase_non_critical_value(list_item* item, function_value_type value);
    // Decrease the value of `item` to the given value until `item` becomes critical.
    void decrease_non_critical_value(list_item* item, function_value_type value);

    // Update the value of critical item `item` to the given value.
    void update_critical_value(list_item* item, function_value_type value);
    // Increase the value of `item` to the given value, where `item` is a minimum.
    void increase_minimum(list_item* item, function_value_type value);
    // Decrease the value of `item` to the given value, where `item` is a maximum.
    void decrease_maximum(list_item* item, function_value_type value);

    void update_value_of_endpoint(list_item* item, function_value_type value);

    // Move critical items that become non-critical upon gluing from `min_dict`/`max_dict` to `nc_dict`.
    // `endpoint_l` and `endpoint_r` are the "inner" endpoints of the left and right interval, respectively,
    // i.e., those endpoints that may become non-critical.
    // Quietly assumes that `min_dict`, `max_dict` and `nc_dict` are the merged dictionaries
    // containing items from `left_interval` and `right_interval`.
    static void update_dicts_on_glue(list_item* endpoint_l,
                                     list_item* endpoint_r,
                                     min_dictionary &min_dict,
                                     max_dictionary &max_dict,
                                     nc_dictionary &nc_dict);

    template<typename dict_a, typename dict_b>
    void move_in_dictionaries(list_item* item, dict_a &a, dict_b& b) {
        a.erase_item(*item);
        b.insert_item(*item);
    }

};

// An iterator over all items in an interval.
class interval_iterator {

public:
    using value_type = list_item;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;

protected:
    interval_iterator(list_item* start,
                      list_item::direction dir = list_item::direction::right);

public:
    interval_iterator& operator++();
    interval_iterator operator++(int);
    reference operator*() const;
    pointer operator->() const;

private:
    list_item* pointed_item;
    list_item::direction dir = list_item::direction::right;
    
    friend class interval;

    friend bool operator==(const interval_iterator &a, const interval_iterator &b);
    friend bool operator!=(const interval_iterator &a, const interval_iterator &b);

};

// An iterator over only critical items in an interval (including down-type items).
class interval_critical_iterator {

public:
    using value_type = list_item;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;

protected:
    interval_critical_iterator(list_item* start,
                               list_item::direction dir = list_item::direction::right);

public:
    interval_critical_iterator& operator++();
    interval_critical_iterator operator++(int);
    reference operator*() const;
    pointer operator->() const;

private:
    pointer pointed_item;
    list_item::direction dir = list_item::direction::right;
    
    friend class interval;

    friend bool operator==(const interval_critical_iterator &a,
                           const interval_critical_iterator &b);
    friend bool operator!=(const interval_critical_iterator &a,
                           const interval_critical_iterator &b);

};

bool operator==(const interval_iterator &a, const interval_iterator &b);
bool operator!=(const interval_iterator &a, const interval_iterator &b);

bool operator==(const interval_critical_iterator &a, const interval_critical_iterator &b);
bool operator!=(const interval_critical_iterator &a, const interval_critical_iterator &b);

}
