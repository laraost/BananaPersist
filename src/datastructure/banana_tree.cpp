#include <limits>

#include "algorithms/banana_tree_algorithms.h"
#include "datastructure/banana_tree.h"
#include "datastructure/banana_tree_sign_template.h"
#include "datastructure/list_item.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/errors.h"
#include "utility/recycling_object_pool.h"

using namespace bananas;

//
// `banana_tree_node` implementation
//

SIGN_TEMPLATE
banana_tree_node<sign>::banana_tree_node(list_item* item) : item(item) {
    massert(item != nullptr, "Node needs a non-null item");
    item->assign_node(this);
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
void banana_tree_node<sign>::set_pointers(self* up_ptr,
                                          self* down_ptr,
                                          self* in_ptr,
                                          self* mid_ptr,
                                          self* low_ptr,
                                          self* death_ptr) {
    up = up_ptr;
    down = down_ptr;
    in = in_ptr;
    mid = mid_ptr;
    low = low_ptr;
    death = death_ptr;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::replace_item(list_item* new_item) {
    massert(new_item->get_node<sign>() == nullptr, "Expected `new_item` to not have a node");
    this->item->template assign_node<sign>(nullptr);
    new_item->assign_node<sign>(this);
    this->item = new_item;
}

SIGN_TEMPLATE
bool banana_tree_node<sign>::is_on_in_trail() const {
    // `low` points to the same node -> minimum
    // `death` of `low` points to the same node -> special root
    // In both cases `this` is not on any trail.
    if (low == this || low->get_death() == this) {
        return false;
    }
    // A node is on the in-trail if `low` lies between the node and the upper end (`low->death`) of the banana.
    return list_item::is_between(*low->get_item(), *low->get_death()->get_item(), *this->get_item());
}

SIGN_TEMPLATE
bool banana_tree_node<sign>::is_on_mid_trail() const {
    // `low` points to the same node -> minimum
    // `death` of `low` points to the same node -> special root
    // In both cases `this` is not on any trail.
    if (low == this || low->get_death() == this) {
        return false;
    }
    // A node is on the mid-trail if it lies between `low` and the upper end (`low->death`) of the banana
    return list_item::is_between(*this->get_item(), *low->get_item(), *low->get_death()->get_item());
}

//
// Private methods of `banana_tree_node`
//

SIGN_TEMPLATE
void banana_tree_node<sign>::set_in_or_up(self* node) {
    if (this->low == this) {
        in = node;
    } else {
        up = node;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::set_mid_or_up(self* node) {
    if (this->low == this) {
        mid = node;
    } else {
        up = node;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::set_in_or_down(self* node, bool set_in) {
    if (set_in) {
        in = node;
    } else {
        down = node;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::set_mid_or_down(self* node, bool set_mid) {
    if (set_mid) {
        mid = node;
    } else {
        down = node;
    }
}

//
// `banana_tree` implementation
//

SIGN_TEMPLATE
banana_tree<sign>::banana_tree(node_pool_type &node_pool) :
        node_pool(node_pool),
        left_hook_item(0),
        right_hook_item(0),
        special_root_item(std::numeric_limits<interval_order_type>::infinity(),
                          sign*std::numeric_limits<function_value_type>::infinity()) {}

SIGN_TEMPLATE
banana_tree<sign>::banana_tree(node_pool_type &node_pool,
                         list_item* left_endpoint,
                         list_item* right_endpoint) :
        node_pool(node_pool),
        left_hook_item(0),
        right_hook_item(0),
        special_root_item(std::numeric_limits<interval_order_type>::infinity(),
                          sign*std::numeric_limits<function_value_type>::infinity()),
        left_endpoint(left_endpoint),
        right_endpoint(right_endpoint) {
    construct(left_endpoint, right_endpoint);
}

SIGN_TEMPLATE
banana_tree<sign>::banana_tree(banana_tree &&other) : node_pool(other.node_pool),
                                                      left_hook_item(std::move(other.left_hook_item)),
                                                      right_hook_item(std::move(other.right_hook_item)),
                                                      special_root_item(std::move(other.special_root_item)),
                                                      global_max(other.global_max),
                                                      left_endpoint(other.left_endpoint),
                                                      right_endpoint(other.right_endpoint) {
    other.global_max = nullptr;
    other.left_endpoint = nullptr;
    other.right_endpoint = nullptr;
    massert(other.left_hook_item.template get_node<sign>() == nullptr, "Expected no left hook node in moved-from banana tree.");
    massert(other.right_hook_item.template get_node<sign>() == nullptr, "Expected no right hook node in moved-from banana tree.");
    massert(other.special_root_item.template get_node<sign>() == nullptr, "Expected no special root node in moved-from banana tree.");
}

SIGN_TEMPLATE
banana_tree<sign>::~banana_tree() {
    if (get_special_root() == nullptr) {
        // No speical root means no banana tree to destroy
        return;
    }
    std::vector<list_item*> items_to_free;
    map_banana_dfs(*this, [&items_to_free](const_node_ptr_type min_node,
                                           const_node_ptr_type max_node,
                                           int, int) {
        items_to_free.push_back(min_node->get_item());
        items_to_free.push_back(max_node->get_item());
    });
//    for (auto* node: this->string()) {
//        items_to_free.push_back(node->get_item());
//    }
    for (auto* item: items_to_free) {
        free_node(item);
    }
//    // The special root is not visited by the string iterator...
//    free_node(&special_root_item);
}

SIGN_TEMPLATE
void banana_tree<sign>::construct(list_item* left_endpoint, list_item* right_endpoint) {
    massert(left_endpoint->right_neighbor() != nullptr, "Need at least two items to construct a banana tree");

    TIME_BEGIN(construct);

    this->left_endpoint = left_endpoint;
    this->right_endpoint = right_endpoint;
    assign_hook_value_and_order<true>(left_endpoint);
    assign_hook_value_and_order<false>(right_endpoint);
    construct_impl(left_endpoint, right_endpoint);
    initialize_spline_labels();

    TIME_END(construct, sign);

    massert(global_max != nullptr, "Expected a global maximum to be assigned during construction");
}

SIGN_TEMPLATE
banana_tree<sign>::walk_iterator_pair banana_tree<sign>::walk() const {
    return {special_root_item.get_node<sign>()};
}

SIGN_TEMPLATE
banana_tree<sign>::string_iterator_pair banana_tree<sign>::string() const {
    auto leftmost_node = left_endpoint->get_node<sign>();
    if (leftmost_node->get_low() != leftmost_node) {
        leftmost_node = left_hook_item.get_node<sign>();
    }
    return {leftmost_node, special_root_item.get_node<sign>()};
}

SIGN_TEMPLATE
void banana_tree<sign>::print(std::ostream &stream) const {
    stream << (sign == 1 ? "up" : "down") << "-tree:\n";
    for (auto [min, max]: walk()) {
        stream << "  Banana (" << min->get_item()->get_interval_order() << ", " << max->get_item()->get_interval_order()
               << " with low = " << max->get_low()->get_item()->get_interval_order() << "\n";
    }
}

SIGN_TEMPLATE
banana_tree<sign>::node_ptr_type banana_tree<sign>::allocate_node(list_item* item) {
    return node_pool.construct(item);
}

SIGN_TEMPLATE
void banana_tree<sign>::free_node(list_item* item) {
    free_node(item->get_node<sign>());
}

SIGN_TEMPLATE
void banana_tree<sign>::free_node(node_ptr_type node) {
    node->item->template assign_node<sign>(nullptr);
    node_pool.free(node);
}

SIGN_TEMPLATE
template<bool left>
void banana_tree<sign>::assign_hook_value_and_order(list_item* endpoint) {
    constexpr interval_order_type offset = 0.1;
    if constexpr (left) {
        massert(endpoint->is_left_endpoint(), "Expected a left endpoint.");
        left_hook_item.assign_value(add_tiniest_offset< -sign>(endpoint->value<1>()));
        left_hook_item.assign_order(endpoint->get_interval_order() - offset);
    } else {
        massert(endpoint->is_right_endpoint(), "Expected a right endpoint.");
        right_hook_item.assign_value(add_tiniest_offset< -sign>(endpoint->value<1>()));
        right_hook_item.assign_order(endpoint->get_interval_order() + offset);
    }
}

SIGN_TEMPLATE
template<bool left>
void banana_tree<sign>::assign_hook_value_and_order() {
    massert(!left || left_hook_item.get_node<sign>() != nullptr,
            "Need a left hook node to update the left hook's value without a specified endpoint.");
    massert(left || right_hook_item.get_node<sign>() != nullptr,
            "Need a right hook node to update the right hook's value without a specified endpoint.");
    list_item* endpoint = left ? left_hook_item.get_node<sign>()->death->item 
                               : right_hook_item.get_node<sign>()->death->item;
    assign_hook_value_and_order<left>(endpoint);
}

// `banana_tree` and `banana_tree_node` instantiation
namespace bananas {
    template class banana_tree_node<1>;
    template class banana_tree_node<-1>;

    template class banana_tree<1>;
    template class banana_tree<-1>;

    template void banana_tree<1>::assign_hook_value_and_order<true>(list_item*);
    template void banana_tree<1>::assign_hook_value_and_order<false>(list_item*);
    template void banana_tree<1>::assign_hook_value_and_order<true>();
    template void banana_tree<1>::assign_hook_value_and_order<false>();
    template void banana_tree< -1>::assign_hook_value_and_order<true>(list_item*);
    template void banana_tree< -1>::assign_hook_value_and_order<false>(list_item*);
    template void banana_tree< -1>::assign_hook_value_and_order<true>();
    template void banana_tree< -1>::assign_hook_value_and_order<false>();
}

//
// `persistence_data_structure` implementation
//

persistence_data_structure::persistence_data_structure(
            recycling_object_pool<up_tree_node> &up_tree_node_pool,
            recycling_object_pool<down_tree_node> &down_tree_node_pool) :
        up_tree(up_tree_node_pool),
        down_tree(down_tree_node_pool) {}

persistence_data_structure::persistence_data_structure(recycling_object_pool<up_tree_node> &up_tree_node_pool,
                                                       recycling_object_pool<down_tree_node> &down_tree_node_pool,
                                                       list_item* left_endpoint,
                                                       list_item* right_endpoint) :
        up_tree(up_tree_node_pool, left_endpoint, right_endpoint),
        down_tree(down_tree_node_pool, left_endpoint, right_endpoint) {}

void persistence_data_structure::construct(list_item* left_endpoint, list_item* right_endpoint) {
    up_tree.construct(left_endpoint, right_endpoint);
    down_tree.construct(left_endpoint, right_endpoint);
}

void persistence_data_structure::extract_persistence_diagram(persistence_diagram &dgm) const {
    using persistence_diagram::diagram_type::essential;
    using persistence_diagram::diagram_type::ordinary;
    using persistence_diagram::diagram_type::relative;

    map_banana_dfs(up_tree, [this, &dgm](const up_tree_node* min_node,
                                         const up_tree_node* max_node,
                                         int, int) {
        if (min_node == up_tree.get_left_hook() || min_node == up_tree.get_right_hook()) {
            return;
        }
        if (max_node == up_tree.get_special_root()) {
            dgm.add_pair<essential>(min_node->get_item(),
                                    up_tree.get_global_max());
        } else {
            dgm.add_pair<ordinary>(min_node->get_item(),
                                   max_node->get_item());
            dgm.add_arrow(min_node->get_item(), max_node->get_low()->get_item());
        }
    });
    map_banana_dfs(down_tree, [this, &dgm](const down_tree_node* min_node,
                                           const down_tree_node* max_node,
                                           int, int) {
        if (min_node == down_tree.get_left_hook() || min_node == down_tree.get_right_hook()) {
            return;
        }
        dgm.add_pair<relative>(min_node->get_item(),
                               max_node->get_item());
        if (max_node != down_tree.get_special_root()) {
            dgm.add_arrow(min_node->get_item(), max_node->get_low()->get_item());
        }
    });
}

const banana_tree<1>& persistence_data_structure::get_up_tree() const {
    return up_tree;
}

const banana_tree< -1>& persistence_data_structure::get_down_tree() const {
    return down_tree;
}

const up_tree_node* persistence_data_structure::get_up_tree_special_root() const {
    return up_tree.get_special_root();
}

const down_tree_node* persistence_data_structure::get_down_tree_special_root() const {
    return down_tree.get_special_root();
}

const list_item* persistence_data_structure::get_global_max() const {
    return up_tree.get_global_max();
}

const list_item* persistence_data_structure::get_global_min() const {
    return down_tree.get_global_max();
}
