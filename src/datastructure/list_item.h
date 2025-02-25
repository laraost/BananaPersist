#pragma once

#include <array>

#include "persistence_defs.h"

namespace bananas {

    template<int sign>
        requires sign_integral<decltype(sign), sign>
    class banana_tree_node;

    class list_item {
        public:
            using list_item_ptr = list_item*;

            enum class direction {
                left = 0,
                right = 1
            };
            constexpr static size_t left_idx = 0;
            constexpr static size_t right_idx = 1;

            inline constexpr static direction other_side(const direction side) {
                return static_cast<direction>(1 - static_cast<unsigned>(side));
            }

        private:
            inline constexpr static size_t to_index(const direction dir) {
                return static_cast<size_t>(dir);
            }

        public:
            list_item(const function_value_type &function_value);
            list_item(const interval_order_type &x_order,
                      const function_value_type &function_value);

            list_item(const list_item&) = delete;

            list_item(list_item &&other);
        
            [[nodiscard]] list_item_ptr left_neighbor() const;
            [[nodiscard]] list_item_ptr right_neighbor() const;
            [[nodiscard]] list_item_ptr neighbor(direction dir) const;

            // Obtain a pointer to the neighbor with lower function value
            // This is only defined for internal items, i.e., if `this->is_internal() == true`.
            [[nodiscard]] list_item_ptr low_neighbor() const;
            // Obtain a pointer to the neighbor with higher function value
            // This is only defined for internal items, i.e., if `this->is_internal() == true`.
            [[nodiscard]] list_item_ptr high_neighbor() const;

            [[nodiscard]] bool is_left_endpoint() const;
            [[nodiscard]] bool is_right_endpoint() const;
            [[nodiscard]] bool is_endpoint() const;
            [[nodiscard]] bool is_internal() const;

            list_item_ptr cut_left();
            list_item_ptr cut_right();

            static void link(list_item &left, list_item &right);

            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] function_value_type value() const;

            [[nodiscard]] interval_order_type get_interval_order() const;

            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_maximum() const;
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_noncritical() const;
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_minimum() const;
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_up_type() const;
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_down_type() const;
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            [[nodiscard]] bool is_critical() const;

            void assign_order(const interval_order_type &value);
            void assign_value(const function_value_type &value);

            void swap_order_and_value(list_item& other_item);

            // Assign to this item the value that's the average of the left and right neighbor's values.
            // Assumes that `this` is an internal item.
            void interpolate_neighbors();

            // Assign `node` as this item's node in the up/down-tree as determined by `sign`.
            // This does not update the node itself.
            template<int sign>
                requires sign_integral<decltype(sign), sign>
            void assign_node(banana_tree_node<sign>* node);

            template<int sign>
                requires sign_integral<decltype(sign), sign>
            void swap_node_with_item(list_item& other_item);

            template<int sign>
                requires sign_integral<decltype(sign), sign>
            banana_tree_node<sign>* get_node() const;

            // Test if item `q` lies in the interval delimited by `a` and `b`.
            // If `a < b`, returns true if $q \in (a,b)$;
            // if `a > b`, returns true if $q \in (b,a)$.`
            static bool is_between(const list_item& q, const list_item& a, const list_item &b);

            // The hook for the intrusive search tree
            default_set_member_hook_type search_tree_hook;

        private:
            std::array<list_item_ptr, 2> neighbors{nullptr, nullptr};

            interval_order_type order;
            function_value_type function_value;

            banana_tree_node<1>* up_node = nullptr;
            banana_tree_node<-1>* down_node = nullptr;

            // Comparison by interval order
            friend bool operator<(const list_item& a, const list_item& b);
            friend bool operator<=(const list_item& a, const list_item& b);
            friend bool operator>(const list_item& a, const list_item& b);
            friend bool operator>=(const list_item& a, const list_item& b);
            friend bool operator==(const list_item& a, const list_item& b);
            friend bool operator!=(const list_item& a, const list_item& b);

            // Private methods

            // Implementation of `cut_left` and `cut_right`
            template<direction side>
            list_item_ptr cut();

    };

    bool operator<(const list_item& a, const list_item& b);
    bool operator<=(const list_item& a, const list_item& b);
    bool operator>(const list_item& a, const list_item& b);
    bool operator>=(const list_item& a, const list_item& b);
    bool operator==(const list_item& a, const list_item& b);
    bool operator!=(const list_item& a, const list_item& b);

    using list_item_pair = min_max_pair<list_item*>; 

}
