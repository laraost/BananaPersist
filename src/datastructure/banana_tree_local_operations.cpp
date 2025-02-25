#include <utility>

#include "datastructure/banana_tree.h"
#include "datastructure/banana_tree_sign_template.h"
#include "datastructure/dictionary.h"
#include "datastructure/list_item.h"
#include "persistence_defs.h"
#include "utility/errors.h"
#include "utility/stats.h"

using namespace bananas;

SIGN_TEMPLATE
void banana_tree_node<sign>::max_interchange_with_parent() {
    massert(up != nullptr, "Node has to have a parent for max interchange");
    massert(item->is_maximum<sign>() || item->is_down_type<sign>(), "Node has to represent a maximum");
    massert(is_internal(), "Node has to represent a maximum, i.e., be an internal node");
    massert(get_value() > up->get_value(), "Item of parent must have smaller value");

    PERSISTENCE_STAT(max_interchange, sign);
    TIME_BEGIN(max_interchange);

    const auto parent = up;
    if (parent->in == this) {
        nested_max_interchange_in_trail();
    } else if (parent->mid == this) {
        nested_max_interchange_mid_trail();
    } else {
        if (this->get_birth()->get_value() < parent->get_birth()->get_value()) {
            parallel_max_interchange_without_swap();
        } else {
            parallel_max_interchange_with_swap();
        }
    }

    TIME_END(max_interchange, sign);
}

SIGN_TEMPLATE
void banana_tree_node<sign>::min_interchange_below(self* other) {
    massert(this->is_leaf(), "Node has to be a leaf.");
    massert(other->is_leaf(), "Other node has to be a leaf.");
    massert(get_value() < other->get_value(),
            "This node's value must be below `other`'s value.");

    PERSISTENCE_STAT(min_interchange, sign);

    // Min-interchange has no effect if bananas are not nested
    if (other != death->low) {
        return;
    }

    TIME_BEGIN(min_interchange);

    // The node at which the windows starting at `this` and `other` merge.
    // This is `other->death` after the interchange.
    auto merge_death = this->death;
    // The node at which the merged window dies.
    // This is `this->death` after the interchange.
    auto high_death = other->death;
    bool is_on_in_trail = merge_death->is_on_in_trail();

    // Fix the new upper trail starting at `this` (except low-pointers)
    // and move `merge_death` onto its new trail by swapping its `down` and `mid`
    if (is_on_in_trail) {
        merge_death->merge_in_trail_to_up();

        std::swap(merge_death->down, merge_death->mid);
    } else {
        // Swap the in and mid trail of the smaller banana, such that they agree
        // with the trails with which they are merged.
        std::swap(merge_death->in, merge_death->mid);
        std::swap(in, mid);
        merge_death->merge_mid_trail_to_up();

        std::swap(merge_death->down, merge_death->in);
        std::swap(merge_death->in, merge_death->mid);
    }

    // Find the point at which to split the trail that originally doesn't contain `merge_death`
    auto below_split = is_on_in_trail ? high_death->mid : high_death->in;
    while (below_split->get_value() > merge_death->get_value()) {
        // Update low pointers.
        // Note that at this point `below_death` still lies on the trail
        // that will end up a trail of `this`.
        below_split->low = this;
        below_split = below_split->down;
    }
    // Get the node above the split.
    // This is `below_split->up` if `below_split` is an internal node
    // and `below_split->in` or `below_split->mid` otherwise.
    self* above_split;
    if (below_split->low == below_split) {
        massert(below_split == other, "We should run into the argument `other` here. This failing implies that the tree structure is incorrect.");
        // We choose the parent of `below_split` on the trail that does *not* contain `merge_death`.
        above_split = is_on_in_trail ? below_split->mid : below_split->in;
    } else {
        above_split = below_split->up;
    }
    // Actually split the trail and attach the top half to `merge_death`.
    if (above_split == high_death) {
        if (is_on_in_trail) {
            above_split->mid = merge_death;
        } else {
            above_split->in = merge_death;
        }
    } else {
        above_split->down = merge_death;
    }
    merge_death->up = above_split;
    // First, we have to make sure that `merge_death` is on the mid-trail
    // If `merge_death` was on the in-trail before, this implies that `other`'s mid- and in-trail have to be swapped.
    if (is_on_in_trail) {
        std::swap(other->in, other->mid);
    }
    // Make the bottom half the in-trail beginning at `merge_death`
    merge_death->in = below_split;
    below_split->set_in_or_up(merge_death);

    // Update low pointers of new in-trail
    auto iter_node = is_on_in_trail ? high_death->in : high_death->mid;
    while (iter_node->low != this) {
        iter_node->low = this;
        iter_node = iter_node->down;
    }

    // Update remaining pointers
    this->death = high_death;
    other->death = merge_death;
    merge_death->low = this;
    // Update the special root's `low` pointer if we are in the special banana
    if (high_death->low == other) {
        high_death->low = this;
    }
    // Update spine labels
    if (high_death->is_special_root()) {
        if (merge_death == high_death->get_in()) {
            merge_death->spine_label = internal::spine_pos::on_left_spine;
        } else if (merge_death == high_death->get_mid()) {
            merge_death->spine_label = internal::spine_pos::on_right_spine;
        } else {
            merge_death->spine_label = internal::spine_pos::not_on_spine;
        }
    } else {
        if (merge_death == high_death->get_in()) {
            merge_death->spine_label = high_death->spine_label;
        } else {
            merge_death->spine_label = internal::spine_pos::not_on_spine;
        }
    }

    TIME_END(min_interchange, sign);
}

//
// `banana_tree_node`, private methods
//

SIGN_TEMPLATE
void banana_tree_node<sign>::parallel_max_interchange_without_swap() {
    massert(up != nullptr, "Node has to have a parent for max interchange");
    massert(this->is_internal(), "Node in max interchange has to be internal");

    auto parent = this->up;
    parent->unlink_from_trail();
    insert_node_on_top_of_in(parent);
    this->spine_label = parent->spine_label;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::parallel_max_interchange_with_swap() {
    massert(up != nullptr, "Node has to have a parent for max interchange");
    massert(this->is_internal(), "Node in max interchange has to be internal");

    auto parent = this->up;
    swap_bananas_with_internal_node(parent);
    parent->unlink_from_trail();
    insert_node_on_top_of_mid(parent);
    std::swap(parent->in, parent->mid);
    auto parent_birth = parent->get_birth();
    std::swap(parent_birth->in, parent_birth->mid);
    std::swap(this->spine_label, parent->spine_label);
}

SIGN_TEMPLATE
void banana_tree_node<sign>::nested_max_interchange_in_trail() {
    massert(up != nullptr, "Node has to have a parent");
    massert(up->in == this, "Node has to be the top of its parents in-trail");

    auto parent = this->up;
    this->unlink_from_trail();
    this->insert_this_above(parent);
    if (parent->is_on_spine()) {
        parent->spine_label = internal::spine_pos::not_on_spine;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::nested_max_interchange_mid_trail() {
    massert(up != nullptr, "Node has to have a parent");
    massert(up->mid == this, "Node has to be the top of its parents mid-trail");

    auto parent = this->up;
    this->unlink_from_trail();
    this->insert_this_above(parent);
    this->swap_bananas_with_internal_node(parent);

    std::swap(parent->in, parent->mid);
    auto parent_birth = parent->get_birth();
    std::swap(parent_birth->in, parent_birth->mid);
    std::swap(this->spine_label, parent->spine_label);
}


SIGN_TEMPLATE
void banana_tree_node<sign>::unlink_from_trail() {
    massert(this->is_internal(), "Node has to be internal");

    auto the_up_node = up;
    auto the_down_node = down;

    if (this == the_up_node->in) {
        the_up_node->in = the_down_node;
    } else if (this == the_up_node->mid) {
        the_up_node->mid = the_down_node;
    } else {
        the_up_node->down = the_down_node;
    }

    if (this == the_down_node->in) {
        the_down_node->in = the_up_node;
    } else if (this == the_down_node->mid) {
        the_down_node->mid = the_up_node;
    } else {
        the_down_node->up = the_up_node;
    }

    up = down = nullptr;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_node_on_top_of_in(self* node) {
    massert(!this->is_leaf(), "Attempted to insert a node to the top of a trail, but `this` is a leaf.");
    auto the_in_node = in;

    node->up = this;
    node->down = the_in_node;
    node->low = the_in_node->low;

    the_in_node->set_in_or_up(node);
    this->in = node;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_node_on_top_of_mid(self* node) {
    massert(!this->is_leaf(), "Attempted to insert a node to the top of a trail, but `this` is a leaf.");
    auto the_mid_node = mid;

    node->up = this;
    node->down = the_mid_node;
    node->low = the_mid_node->low;

    the_mid_node->set_mid_or_up(node);
    this->mid = node;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_node_on_bottom_of_in(self* node) {
    massert(this->is_leaf(), "Cannot insert on botton of in of maximum.");
    auto the_in_node = in;

    if (the_in_node->in == this) {
        the_in_node->in = node;
    } else {
        the_in_node->down = node;
    }
    node->up = the_in_node;
    node->down = this;
    node->low = this;
    this->in = node;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_node_on_bottom_of_mid(self* node) {
    massert(this->is_leaf(), "Cannot insert on botton of mid of maximum.");
    auto the_mid_node = mid;

    if (the_mid_node->mid == this) {
        the_mid_node->mid = node;
    } else {
        the_mid_node->down = node;
    }
    node->up = the_mid_node;
    node->down = this;
    node->low = this;
    this->mid = node;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_this_above(self* node) {
    massert(node->is_internal(), "Node has to be internal to a trail");
    if (node == node->up->in) {
        node->up->insert_node_on_top_of_in(this);
    } else if (node == node->up->mid) {
        node->up->insert_node_on_top_of_mid(this);
    } else {
        auto parent = node->up;
        parent->down = this;
        this->down = node;
        node->up = this;
        this->up = parent;
        this->low = node->low;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::insert_this_below(self* node) {
    massert(node->is_internal(), "Node has to be internal to a trail.");
    if (node == node->down->in) {
        node->down->insert_node_on_bottom_of_in(this);
    } else if (node == node->down->mid) {
        node->down->insert_node_on_bottom_of_mid(this);
    } else {
        auto child = node->down;
        child->up = this;
        this->up = node;
        node->down = this;
        this->down = child;
        this->low = node->low;
    }
}

SIGN_TEMPLATE
void banana_tree_node<sign>::swap_bananas_with_internal_node(self* node) {
    swap_in_trail_with_internal_node(node);    
    swap_mid_trail_with_internal_node(node);
    this->get_birth()->death = this;
    node->get_birth()->death = node;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::swap_in_trail_with_internal_node(self* node) {
    auto this_in = this->in;
    auto node_in = node->in;
    if (this_in->in == this) {
        this_in->in = node;
    } else {
        this_in->up = node;
    }
    if (node_in->in == node) {
        node_in->in = this;
    } else {
        node_in->up = this;
    }
    node->in = this_in;
    this->in = node_in;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::swap_mid_trail_with_internal_node(self* node) {
    auto this_mid = this->mid;
    auto node_mid = node->mid;
    if (this_mid->mid == this) {
        this_mid->mid = node;
    } else {
        this_mid->up = node;
    }
    if (node_mid->mid == node) {
        node_mid->mid = this;
    } else {
        node_mid->up = this;
    }
    node->mid = this_mid;
    this->mid = node_mid;
}

SIGN_TEMPLATE
void banana_tree_node<sign>::merge_in_trail_to_up() {
    assert(this->is_on_in_trail());
    auto the_in = in;
    auto the_up = up;
    if (the_up->in == this) {
        the_up->in = the_in;
    } else {
        the_up->down = the_in;
    }
    the_in->set_in_or_up(the_up);
}

SIGN_TEMPLATE
void banana_tree_node<sign>::merge_mid_trail_to_up() {
    assert(this->is_on_mid_trail());
    auto the_mid = mid;
    auto the_up = up;
    if (the_up->mid == this) {
        the_up->mid = the_mid;
    } else {
        the_up->down = the_mid;
    }
    the_mid->set_mid_or_up(the_up);
}

//
// Implementation of public methods of `banana_tree` related to local operations
//

SIGN_TEMPLATE
void banana_tree<sign>::on_increase_value_of_maximum(list_item* item) {
    on_increase_value_of_maximum(item, [](node_ptr_type, node_ptr_type){});
}

SIGN_TEMPLATE
void banana_tree<sign>::on_decrease_value_of_maximum(list_item* item) {
    on_decrease_value_of_maximum(item, [](node_ptr_type, node_ptr_type){});
}

SIGN_TEMPLATE
void banana_tree<sign>::anticancel(list_item* next_max,
                                   const list_item_pair &item_pair) {
    massert(item_pair.min->is_minimum<sign>(),
            "First item of item pair has to be a minimum.");
    massert(item_pair.max->is_maximum<sign>(),
            "Second item of item pair has to be a maximum.");
    massert(next_max->is_maximum<sign>() || next_max->is_down_type<sign>(),
            "Expected anticancel next to a maximum or down-type item.");

    PERSISTENCE_STAT(anticancellation, sign);
    TIME_BEGIN(anticancellation);

    auto *new_min_node = allocate_node(item_pair.min);
    auto *new_max_node = allocate_node(item_pair.max);

    auto *next_max_node = next_max->get_node<sign>();
    auto *next_max_bth = next_max_node->get_birth();

    auto* node_q = list_item::is_between(*item_pair.min,
                                         *next_max,
                                         *next_max_bth->get_item())
            ? next_max_node->mid
            : next_max_node->down;
    while (node_q->get_value() > new_max_node->get_value()) {
        PERSISTENCE_STAT(anticancellation_iterations, sign);
        node_q = node_q->in;
    }
    if (node_q->is_leaf()) {
        if (node_q == next_max_bth) {
            node_q->insert_node_on_bottom_of_mid(new_max_node);
        } else {
            if (list_item::is_between(*item_pair.max, *node_q->get_item(), *node_q->get_death()->get_item())) {
                node_q->insert_node_on_bottom_of_mid(new_max_node);
            } else {
                node_q->insert_node_on_bottom_of_in(new_max_node);
            }
        }
    } else {
        new_max_node->insert_this_above(node_q);
    }
    new_max_node->in = new_min_node;
    new_max_node->mid = new_min_node;
    new_min_node->in = new_max_node;
    new_min_node->mid = new_max_node;
    new_min_node->death = new_max_node;
    new_min_node->low = new_min_node;

    TIME_END(anticancellation, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::max_slide(list_item* old_max, list_item* new_max) {
    massert(new_max->is_internal(), "Expected the new maximum to be an internal item.");
    massert(old_max->right_neighbor() == new_max || old_max->left_neighbor() == new_max,
            "Items involved in max-slide need to be neighbors.");
    massert(old_max->value<sign>() < new_max->value<sign>(),
            "Expected a max-slide, but new maximum is too low in value.");
    massert(old_max->get_node<sign>() != nullptr, "Old maximum has to be in the banana tree.");
    massert(new_max->get_node<sign>() == nullptr, "New maximum may not be in the banana tree.");

    PERSISTENCE_STAT(max_slide, sign);
    TIME_BEGIN(max_slide);

    auto* old_max_node = old_max->get_node<sign>();
    massert(old_max_node->is_internal(), "Old maximum has to be an internal node.");
    old_max_node->replace_item(new_max);

    if (old_max == global_max) {
        global_max = new_max;
    }

    if (old_max->is_right_endpoint()) {
        // `old_max_node` needs to go where the right hook is now
        // By assumption `new_max->value<sign>() > old_max->value<sign>()`
        // and since `old_max` is just above the hook in value,
        // we don't need to do any position adjustments.  // TODO: I hope^^
        auto right_hook_node = right_hook_item.get_node<sign>();
        right_hook_node->replace_item(old_max);
    } else if (old_max->is_left_endpoint()) {
        auto left_hook_node = left_hook_item.get_node<sign>();
        left_hook_node->replace_item(old_max);
    }

    TIME_END(max_slide, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::min_slide(list_item* old_min, list_item* new_min) {
    massert(new_min->is_internal(), "Expected the new minimum to be an internal item.");
    massert(old_min->right_neighbor() == new_min || old_min->left_neighbor() == new_min,
            "Items involved in min-slide need to be neighbors.");
    massert(old_min->value<sign>() > new_min->value<sign>(),
            "Expected a min-slide, but new minimum is too low in value.");
    massert(old_min->get_node<sign>() != nullptr, "Old minimum has to be in the banana tree.");
    massert(new_min->get_node<sign>() == nullptr, "New minimum may not be in the banana tree.");

    PERSISTENCE_STAT(min_slide, sign);
    TIME_BEGIN(min_slide);

    auto* old_min_node = old_min->get_node<sign>();
    massert(old_min_node->is_leaf(), "Old minimum has to be a leaf.");
    old_min_node->replace_item(new_min);

    if (old_min->is_endpoint()) {
        auto is_left = old_min->is_left_endpoint();
        auto hook_node = is_left ? allocate_node(&left_hook_item) :
                                   allocate_node(&right_hook_item);
        auto old_min_new_node = allocate_node(old_min);
        old_min_new_node->set_pointers(old_min_node->death,
                                       old_min_node,
                                       hook_node,
                                       hook_node,
                                       old_min_node,
                                       nullptr);
        hook_node->set_pointers(nullptr,
                                nullptr,
                                old_min_new_node,
                                old_min_new_node,
                                hook_node,
                                old_min_new_node);
        old_min_new_node->spine_label = old_min_node->spine_label;
        hook_node->spine_label = old_min_node->spine_label;
        old_min_node->spine_label = internal::spine_pos::not_on_spine;
        if (old_min_node->get_death()->is_special_root()) {
            if (*old_min > *new_min) {
                old_min_node->insert_node_on_bottom_of_mid(old_min_new_node);
            } else {
                old_min_node->insert_node_on_bottom_of_in(old_min_new_node);
            }
        } else {
            auto death_item = old_min_node->get_death()->get_item();
            if (list_item::is_between(*old_min, *death_item, *new_min)) {
                old_min_node->insert_node_on_bottom_of_mid(old_min_new_node);
            } else {
                massert(list_item::is_between(*new_min, *death_item, *old_min),
                        "Expected `old_min` in the in-panel of (`new_min`, `death(new_min)`), but it's somewhere else.");
                old_min_node->insert_node_on_bottom_of_in(old_min_new_node);
            }
        }
        // Note: `old_min_node->death->item` has higher value than `old_min`,
        // since it's the same item that killed `old_min` when it was still a minimum.
        // This means that `old_min_new_node` is in the correct position
        // on the bottom of the in-/mid-trail of `old_min_node`,
        // and there's nothing else to do
    }

    TIME_END(min_slide, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::cancel_maximum(list_item* item) {
    PERSISTENCE_STAT(cancellation, sign);
    TIME_BEGIN(cancellation);

    auto the_node = item->get_node<sign>();
    massert(the_node->is_internal(), "Cancelled item has to be a maximum/represented by an internal node.");
    massert(the_node->has_empty_banana(), "Cancelled banana may not have nested bananas.");

    the_node->unlink_from_trail();
    auto the_birth = the_node->in;
    free_node(the_node);
    free_node(the_birth);

    TIME_END(cancellation, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::cancel_max_with_endpoint(list_item* item, list_item* endpoint) {
    massert(item->is_internal(),
            "Expected `item` to be represented by an internal node.");
    massert(endpoint->is_endpoint(),
            "Expected `endpoint` to be an endpoint.");
    massert(endpoint->get_node<sign>()->is_leaf(),
            "Expected `endpoint` to be represented by a leaf.");

    PERSISTENCE_STAT(cancellation, sign);
    TIME_BEGIN(cancellation);

    auto* endpoint_node = endpoint->get_node<sign>();
    auto* item_node = item->get_node<sign>();
    endpoint->assign_node(item_node);
    item_node->item = endpoint;
    if (endpoint->is_left_endpoint()) {
        assign_hook_value_and_order<true>(endpoint);
        left_hook_item.assign_node(endpoint_node);
        endpoint_node->item = &left_hook_item;
    } else {
        assign_hook_value_and_order<false>(endpoint);
        right_hook_item.assign_node(endpoint_node);
        endpoint_node->item = &right_hook_item;
    }
    item->assign_node<sign>(nullptr);
    // If `item` was the global maximum then `endpoint` is the new global maximum,
    // since we assume that `item` and `endpoint` are sufficiently close in value.
    if (item == global_max) {
        global_max = endpoint;
    }

    TIME_END(cancellation, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::cancel_min_with_endpoint(list_item* item, list_item* endpoint) {
    massert(item->get_node<sign>()->is_leaf(),
            "Expected `item` to be represented by a leaf.");
    massert(endpoint->is_endpoint(),
            "Expected `endpoint` to be an endpoint");
    massert(endpoint->get_node<sign>()->is_internal(),
            "Expected endpoint to be represented by an internal node.");

    PERSISTENCE_STAT(cancellation, sign);
    TIME_BEGIN(cancellation);

    auto& hook_item = endpoint->is_left_endpoint() ? left_hook_item :
                                                     right_hook_item;
    auto* hook_node = hook_item.get_node<sign>();    
    auto* endpoint_node = endpoint->get_node<sign>();
    const auto endpoint_spine_label = endpoint_node->spine_label;
    massert(endpoint_node->get_birth() == hook_node,
            "Expected the endpoint to be paired with the hook.");
    endpoint_node->unlink_from_trail();
    free_node(endpoint_node);
    free_node(hook_node);
    
    auto* item_node = item->get_node<sign>();
    item_node->item = endpoint;
    item->assign_node<sign>(nullptr);
    endpoint->assign_node(item_node);
    item_node->spine_label = endpoint_spine_label;

    TIME_END(cancellation, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::replace_right_endpoint(list_item *new_endpoint) {
    auto endpoint_node = right_endpoint->get_node<sign>();
    endpoint_node->replace_item(new_endpoint);
    if (right_endpoint == global_max) {
        global_max = new_endpoint;
    }
    right_endpoint = new_endpoint;
    if (right_endpoint->is_down_type<sign>()) {
        assign_hook_value_and_order<false>();
    }
}

SIGN_TEMPLATE
void banana_tree<sign>::replace_left_endpoint(list_item *new_endpoint) {
    auto endpoint_node = left_endpoint->get_node<sign>();
    endpoint_node->replace_item(new_endpoint);
    if (left_endpoint == global_max) {
        global_max = new_endpoint;
    }
    left_endpoint = new_endpoint;
    if (left_endpoint->is_down_type<sign>()) {
        assign_hook_value_and_order<true>();
    }
}

//
// Implementation of private methods of `banana_tree` related to local operations
//

SIGN_TEMPLATE
template<typename T>
void banana_tree<sign>::on_increase_value_of_maximum(list_item* item, const T &callback) {
    TIME_BEGIN(max_increase);

    auto the_node = item->get_node<sign>();
    massert(the_node->is_internal(), "Expected the node to be internal when increasing the value of a maximum");
    // Update the hook, if needed
    if (item->is_right_endpoint()) {
        assign_hook_value_and_order<false>(item);
    }
    if (item->is_left_endpoint()) {
        assign_hook_value_and_order<true>(item);
    }
    auto the_parent = the_node->up;
    while (the_parent->get_value() < item->value<sign>()) {
        the_node->max_interchange_with_parent();

        callback(the_node, the_parent);

        the_parent = the_node->up;
    }
    // If the item becomes the global maximum, then the special root represents a new global maximum.
    // We use `callback` to notify the caller that this is happening,
    // and pretend it's an interchange with the node associated with the global maximum
    if (the_parent == get_special_root() && item->value<sign>() > global_max->value<sign>()) {
        callback(the_node, global_max->get_node<sign>());
    }
    update_global_max(item);

    TIME_END(max_increase, sign);
}

SIGN_TEMPLATE
template<typename T>
void banana_tree<sign>::on_decrease_value_of_maximum(list_item* item, const T &callback) {
    TIME_BEGIN(max_decrease);

    auto the_node = item->get_node<sign>();
    massert(the_node->is_internal(), "Expected the node to be internal when decreasing the value of a maximum");
    // Update the hook, if needed
    if (item->is_right_endpoint()) {
        assign_hook_value_and_order<false>(item);
    }
    if (item->is_left_endpoint()) {
        assign_hook_value_and_order<true>(item);
    }
    // Update the global max first. Candidates are:
    // - `special_root->in` if `the_node` is mid of the special root
    // - `special_root->mid` if `the_node` is in of the special root
    // - `the_node->in`, `the_node->mid`, `the_node->down`
    // In all cases we need to make sure that the item is actually a maximum/down-type item.
    if (item == global_max) {
        auto special_root = special_root_item.get_node<sign>();
        if (the_node == special_root->get_in()) {
            update_global_max(special_root->get_mid()->item);
        } else {
            update_global_max(special_root->get_in()->item);
        }
        auto down_item = the_node->down->item;
        update_global_max(down_item);
        if (the_node->get_birth()->get_item() != &right_hook_item &&
                the_node->get_birth()->get_item() != &left_hook_item) {
            for (auto next_lower_item: {the_node->in->item, the_node->mid->item}) {
                update_global_max(next_lower_item);
            }
        }
        // Use the callback to notify the caller of the "interchange" of this item and the new global max
        if (item != global_max) {
            callback(global_max->get_node<sign>(), the_node);
        }
    }
    // Now perform the interchanges with `down`, `in`, `mid`, whichever has largest value
    auto max_child_node = std::max({the_node->down, the_node->in, the_node->mid},
        [](const node_ptr_type &a, const node_ptr_type &b) {
            return a->get_value() < b->get_value();
        }
    );
    // The paper uses `f(q) > f(j+1)` as condition in the while-loop, where `q` is `max_child_node`
    // and `j+1` is the minimum with greatest value neighboring `item`.
    // This is equivalent to `q is a maximum`,
    // which again is equivalent to `q`'s low pointer not pointing to itself.
    // In other words: the loop terminates when `max_child_node` is no longer a maximum.
    while(max_child_node->get_low() != max_child_node && max_child_node->get_value() > the_node->get_value()) {
        massert(max_child_node->up == the_node, "Interchanged node should be a child of `item`'s node.");
        max_child_node->max_interchange_with_parent();

        callback(max_child_node, the_node);

        max_child_node = std::max({the_node->down, the_node->in, the_node->mid},
            [](const node_ptr_type &a, const node_ptr_type &b) {
                return a->get_value() < b->get_value();
            }
        );
    }

    TIME_END(max_decrease, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::update_global_max(list_item* item) {
    if ((item->is_down_type<sign>() || item->is_maximum<sign>()) &&
            item->value<sign>() > global_max->value<sign>()) {
        global_max = item;
    }
}

SIGN_TEMPLATE
void banana_tree<sign>::update_global_max() {
    auto* special_root_in = get_special_root()->get_in();
    auto* special_root_mid = get_special_root()->get_mid();
    if (special_root_in->get_value() > special_root_mid->get_value()) {
        global_max = special_root_in->item;
    } else {
        global_max = special_root_mid->item;
    }
}

//
// Implementation of local operations in `persistence_data_structure`
//

void persistence_data_structure::on_increase_value_of_maximum(list_item* max_item) {
    up_tree.on_increase_value_of_maximum(max_item, [](up_tree_node* above, up_tree_node* below) {
        auto *down_above = above->get_opposite_node();
        auto *down_below = below->get_opposite_node();
        // Flipping the sign changes the meaning of above and below:
        // in the down-tree `above` goes below `below`, even though `above` goes above `below` in the up-tree.
        down_above->min_interchange_below(down_below);
    });
}
void persistence_data_structure::on_decrease_value_of_maximum(list_item* max_item) {
    up_tree.on_decrease_value_of_maximum(max_item, [](up_tree_node* above, up_tree_node* below) {
        auto *down_above = above->get_opposite_node();
        auto *down_below = below->get_opposite_node();
        // Flipping the sign changes the meaning of above and below:
        // in the down-tree `above` goes below `below`, even though `above` goes above `below` in the up-tree.
        down_above->min_interchange_below(down_below);
    });
}
void persistence_data_structure::on_increase_value_of_minimum(list_item* min_item) {
    down_tree.on_decrease_value_of_maximum(min_item, [](down_tree_node* above, down_tree_node* below) {
        auto *up_above = above->get_opposite_node();
        auto *up_below = below->get_opposite_node();
        // Flipping the sign changes the meaning of above and below:
        // in the up-tree `above` goes below `below`, even though `above` goes above `below` in the down-tree.
        up_above->min_interchange_below(up_below);
    });
}
void persistence_data_structure::on_decrease_value_of_minimum(list_item* min_item) {
    down_tree.on_increase_value_of_maximum(min_item, [](down_tree_node* above, down_tree_node* below) {
        auto *up_above = above->get_opposite_node();
        auto *up_below = below->get_opposite_node();
        // Flipping the sign changes the meaning of above and below:
        // in the up-tree `above` goes below `below`, even though `above` goes above `below` in the down-tree.
        up_above->min_interchange_below(up_below);
    });
}

void persistence_data_structure::anticancel(min_dictionary &min_dict,
                                            max_dictionary &max_dict,
                                            const list_item_pair &new_items) {
    massert(new_items.min->left_neighbor() == new_items.max || new_items.min->right_neighbor() == new_items.max,
            "Anticancelled items have to be neighbors.");
    // Anticancellation in the up-tree
    TIME_BEGIN(anticancellation_dict);
    auto closest_max = max_dict.closest_item_on_opposite_side(*new_items.min, *new_items.max);
    auto closest_min = min_dict.closest_item_on_opposite_side(*new_items.max, *new_items.min);
    TIME_END(anticancellation_dict, 1);
    massert(closest_max != max_dict.end(), "Insertion at an endpoint is not an anticancellation.");
    massert(closest_min != min_dict.end(), "Insertion at an endpoint is not an anticancellation.");

    up_tree.anticancel(&(*closest_max), new_items);
    down_tree.anticancel(&(*closest_min), {new_items.max, new_items.min});
}

void persistence_data_structure::cancel(list_item* min_item, list_item* max_item) {
    up_tree.cancel_maximum(max_item);
    down_tree.cancel_maximum(min_item);
}

void persistence_data_structure::cancel_max_with_endpoint(list_item* item, list_item* endpoint) {
    up_tree.cancel_max_with_endpoint(item, endpoint);
    down_tree.cancel_min_with_endpoint(item, endpoint);
}

void persistence_data_structure::cancel_min_with_endpoint(list_item* item, list_item* endpoint) {
    up_tree.cancel_min_with_endpoint(item, endpoint);
    down_tree.cancel_max_with_endpoint(item, endpoint);
}

void persistence_data_structure::max_slide(list_item* old_max, list_item* new_max) {
    up_tree.max_slide(old_max, new_max);
    down_tree.min_slide(old_max, new_max);
}

void persistence_data_structure::min_slide(list_item* old_min, list_item* new_min) {
    up_tree.min_slide(old_min, new_min);
    down_tree.max_slide(old_min, new_min);
}

void persistence_data_structure::change_down_to_up(list_item* endpoint, list_item* neighbor) {
    if (neighbor->is_noncritical<1>()) {
        up_tree.cancel_min_with_endpoint(neighbor, endpoint);
        down_tree.cancel_max_with_endpoint(neighbor, endpoint);
    } else {
        up_tree.max_slide(endpoint, neighbor);
        down_tree.min_slide(endpoint, neighbor);
    }
}

void persistence_data_structure::change_up_to_down(list_item* endpoint, list_item* neighbor) {
    if (neighbor->is_noncritical<1>()) {
        up_tree.cancel_max_with_endpoint(neighbor, endpoint);
        down_tree.cancel_min_with_endpoint(neighbor, endpoint);
    } else {
        up_tree.min_slide(endpoint, neighbor);
        down_tree.max_slide(endpoint, neighbor);
    }
}

void persistence_data_structure::replace_right_endpoint(list_item *new_endpoint) {
    up_tree.replace_right_endpoint(new_endpoint);
    down_tree.replace_right_endpoint(new_endpoint);
}

void persistence_data_structure::replace_left_endpoint(list_item *new_endpoint) {
    up_tree.replace_left_endpoint(new_endpoint);
    down_tree.replace_left_endpoint(new_endpoint);
}

namespace bananas {
    template class banana_tree_node<1>;
    template class banana_tree_node< -1>;

    template class banana_tree<1>;
    template class banana_tree< -1>;
}
