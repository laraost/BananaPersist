#include "datastructure/list_item.h"
#include "datastructure/banana_tree.h"
#include "persistence_defs.h"
#include "utility/errors.h"

using namespace bananas;

list_item::list_item(const function_value_type &function_value) : list_item(0, function_value) {}
list_item::list_item(const interval_order_type &x_order,
                     const function_value_type &function_value) : order(x_order), function_value(function_value) {}

list_item::list_item(list_item &&other) :
    neighbors(std::move(other.neighbors)),
    order(std::move(other.order)),
    function_value(std::move(other.order))
{
    if (other.up_node != nullptr) {
        other.up_node->replace_item(this);
    }
    if (other.down_node != nullptr) {
        other.down_node->replace_item(this);
    }
}

list_item::list_item_ptr list_item::left_neighbor() const {
    return neighbors[left_idx];
}

list_item::list_item_ptr list_item::right_neighbor() const {
    return neighbors[right_idx];
}

list_item::list_item_ptr list_item::neighbor(direction dir) const {
    return neighbors[to_index(dir)];
}

list_item::list_item_ptr list_item::low_neighbor() const {
    massert(this->is_internal(), "Expected `this` to be an internal item");
    return left_neighbor()->value<1>() < right_neighbor()->value<1>() ? left_neighbor() :
                                                                        right_neighbor();
}

list_item::list_item_ptr list_item::high_neighbor() const {
    massert(this->is_internal(), "Expected `this` to be an internal item");
    return left_neighbor()->value<1>() > right_neighbor()->value<1>() ? left_neighbor() :
                                                                        right_neighbor();
}

bool list_item::is_left_endpoint() const {
    return neighbors[left_idx] == nullptr;
}

bool list_item::is_right_endpoint() const {
    return neighbors[right_idx] == nullptr;
}

bool list_item::is_endpoint() const {
    return is_left_endpoint() || is_right_endpoint();
}

bool list_item::is_internal() const {
    return !is_endpoint();
}

list_item::list_item_ptr list_item::cut_left() {
    massert(neighbors[to_index(direction::left)] != nullptr, "Cutting left requires a left neighbor.");
    return cut<direction::left>();
}

list_item::list_item_ptr list_item::cut_right() {
    massert(neighbors[to_index(direction::right)] != nullptr, "Cutting right requires a right neighbor.");
    return cut<direction::right>();
}

void list_item::link(list_item &left, list_item &right) {
    massert(left.neighbors[right_idx] == nullptr, "Link requires the left item to not have a right neighbor.");
    massert(right.neighbors[left_idx] == nullptr, "Link requires the right item to not have a left neighbor.");
    left.neighbors[right_idx] = &right;
    right.neighbors[left_idx] = &left;
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
function_value_type list_item::value() const {
    return sign*function_value;
}
template function_value_type list_item::value<1>() const;
template function_value_type list_item::value< -1>() const;

interval_order_type list_item::get_interval_order() const {
    return order;
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_maximum() const {
    if (!is_internal()) {
        return false;
    }
    return neighbors[to_index(direction::left)]->value<sign>() < value<sign>()
        && neighbors[to_index(direction::right)]->value<sign>() < value<sign>();
}
template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_noncritical() const {
    return is_internal() && !list_item::is_maximum<sign>() && !list_item::is_minimum<sign>();
}
template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_minimum() const {
    if (!is_internal()) {
        return false;
    }
    return neighbors[to_index(direction::left)]->value<sign>() > value<sign>()
        && neighbors[to_index(direction::right)]->value<sign>() > value<sign>();
}
template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_up_type() const {
    return (is_left_endpoint() && right_neighbor()->value<sign>() > value<sign>()) ||
           (is_right_endpoint() && left_neighbor()->value<sign>() > value<sign>());
}
template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_down_type() const {
    return (is_left_endpoint() && right_neighbor()->value<sign>() < value<sign>()) ||
           (is_right_endpoint() && left_neighbor()->value<sign>() < value<sign>());
}
template<int sign>
    requires sign_integral<decltype(sign), sign>
bool list_item::is_critical() const {
    return is_up_type<sign>() || is_maximum<sign>() || is_minimum<sign>();
};

// Instantiation of criticality checks
template bool list_item::is_maximum<1>() const;
template bool list_item::is_maximum<-1>() const;
template bool list_item::is_noncritical<1>() const;
template bool list_item::is_noncritical<-1>() const;
template bool list_item::is_minimum<1>() const;
template bool list_item::is_minimum<-1>() const;
template bool list_item::is_up_type<1>() const;
template bool list_item::is_up_type<-1>() const;
template bool list_item::is_down_type<1>() const;
template bool list_item::is_down_type<-1>() const;
template bool list_item::is_critical<1>() const;
template bool list_item::is_critical<-1>() const;

void list_item::assign_order(const interval_order_type &value) {
    this->order = value;
}

void list_item::assign_value(const function_value_type &value) {
    this->function_value = value;
}

void list_item::swap_order_and_value(list_item& other_item) {
    using std::swap;
    swap(order, other_item.order);
    swap(function_value, other_item.function_value);
}

void list_item::interpolate_neighbors() {
    massert(is_internal(), "Cannot interpolate between the neighbors of an endpoint.");
    auto left_value = left_neighbor()->value<1>();
    auto right_value = right_neighbor()->value<1>();
    function_value = (left_value + right_value)/2.0;
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
void list_item::assign_node(banana_tree_node<sign> *node) {
    if constexpr (sign == 1) {
        up_node = node;
    } else {
        down_node = node;
    }
}
template void list_item::assign_node(banana_tree_node<1> *node);
template void list_item::assign_node(banana_tree_node<-1> *node);

template<int sign>
    requires sign_integral<decltype(sign), sign>
void list_item::swap_node_with_item(list_item& other_item) {
    auto* this_node = get_node<sign>();
    auto* other_node = other_item.get_node<sign>();
    if (this_node == nullptr && other_node != nullptr) {
        other_node->replace_item(this);
    } else if (this_node != nullptr && other_node == nullptr) {
        this_node->replace_item(&other_item);
    } else if (this_node != nullptr && other_node != nullptr){
        assign_node<sign>(nullptr);
        other_item.assign_node<sign>(nullptr);
        this_node->replace_item(&other_item);
        other_node->replace_item(this);
        other_item.assign_node(this_node);
    }
}
template void list_item::swap_node_with_item<1>(list_item&);
template void list_item::swap_node_with_item<-1>(list_item&);

template<int sign>
    requires sign_integral<decltype(sign), sign>
banana_tree_node<sign>* list_item::get_node() const {
    if constexpr (sign == 1) {
        return up_node;
    } else {
        return down_node;
    }
}
template banana_tree_node<1>* list_item::get_node() const;
template banana_tree_node<-1>* list_item::get_node() const;

bool list_item::is_between(const list_item& q, const list_item& a, const list_item &b) {
    return (a < q && q < b) || (a > q && q > b);
}

//
// `list_item`: private methods
//

template<list_item::direction side>
list_item::list_item_ptr list_item::cut() {
    auto* result = neighbors[to_index(side)];
    neighbors[to_index(side)] = nullptr;
    result->neighbors[to_index(other_side(side))] = nullptr;
    return result;
}

bool bananas::operator<(const list_item& a, const list_item& b) {
    return a.order < b.order;
}
bool bananas::operator<=(const list_item& a, const list_item& b) {
    return a.order <= b.order;
}
bool bananas::operator>(const list_item& a, const list_item& b) {
    return a.order > b.order;
}
bool bananas::operator>=(const list_item& a, const list_item& b) {
    return a.order >= b.order;
}
bool bananas::operator==(const list_item& a, const list_item& b) {
    return a.order == b.order;
}
bool bananas::operator!=(const list_item& a, const list_item& b) {
    return a.order != b.order;
}
