#pragma once

#include "datastructure/banana_tree_sign_template.h"
#include "datastructure/banana_tree.h"

namespace bananas {

namespace internal {

// Recursively test if `node` and descendants of `node` are left of `reference`.
SIGN_TEMPLATE
bool descendants_less_rec(const banana_tree_node<sign> *node, const banana_tree_node<sign> *reference) {
    if (node->is_leaf()) {
        return *(node->get_item()) < *(reference->get_item());
    }
    if (*(node->get_item()) >= *(reference->get_item())) {
        return false;
    }
    if (!descendants_less_rec(node->get_in(), reference)) {
        return false;
    }
    if (!descendants_less_rec(node->get_mid(), reference)) {
        return false;
    }
    if (!descendants_less_rec(node->get_down(), reference)) {
        return false;
    }
    return true;
}

// Recursively test if `node` and descendants of `node` are right of `reference`.
SIGN_TEMPLATE
bool descendants_greater_rec(const banana_tree_node<sign> *node, const banana_tree_node<sign> *reference) {
    if (node->is_leaf()) {
        return *(node->get_item()) > *(reference->get_item());
    }
    if (*(node->get_item()) <= *(reference->get_item())) {
        return false;
    }
    if (!descendants_less_rec(node->get_in(), reference)) {
        return false;
    }
    if (!descendants_less_rec(node->get_mid(), reference)) {
        return false;
    }
    if (!descendants_less_rec(node->get_down(), reference)) {
        return false;
    }
    return true;
}

} // End of namespace `internal`

SIGN_TEMPLATE
bool test_invariant_1(const banana_tree_node<sign> *max_node) {
    massert(max_node->is_internal(), "Attempted to test invariant 1 for a node that's not internal.");
    const auto& max_item = *(max_node->get_item());
    const auto& birth_item = *(max_node->get_birth()->get_item());
    bool in_is_good = false;
    bool mid_is_good = false;
    bool down_is_good = false;
    if (birth_item < max_item) {
        in_is_good = internal::descendants_less_rec(max_node->get_in(), max_node);
        mid_is_good = internal::descendants_less_rec(max_node->get_mid(), max_node);
        down_is_good = internal::descendants_greater_rec(max_node->get_down(), max_node);
    } else {
        in_is_good = internal::descendants_greater_rec(max_node->get_in(), max_node);
        mid_is_good = internal::descendants_greater_rec(max_node->get_mid(), max_node);
        down_is_good = internal::descendants_less_rec(max_node->get_down(), max_node);
    }
    return in_is_good && mid_is_good && down_is_good;
}

SIGN_TEMPLATE
bool test_invariant_2(const banana_tree_node<sign>* min_node) {
    massert(min_node->is_leaf(), "Attempted to test invariant 2 for an internal node.");
    return min_node->get_value() > min_node->get_death()->get_low()->get_value();
}

SIGN_TEMPLATE
bool test_invariant_3(const banana_tree_node<sign>* max_node) {
    massert(max_node->is_internal(), "Attempted to test invariant 3 for a node that's not internal.");
    massert(!max_node->is_special_root(), "Attempted to test invariant 3 for the special root.");
    bool value_condition = max_node->get_up()->get_value() > max_node->get_value() &&
                           max_node->get_value() > max_node->get_down()->get_value();
    bool order_condition = false;
    const auto& up_item = *max_node->get_up()->get_item();
    const auto& max_item = *max_node->get_item();
    const auto& down_item = *max_node->get_down()->get_item();
    if (max_node->get_up()->get_in() != max_node) {
        order_condition = (up_item < max_item && max_item < down_item) ||
                          (down_item < max_item && max_item < up_item);
    } else {
        order_condition = (up_item < max_item && down_item < max_item) ||
                          (up_item > max_item && down_item > max_item);
    }
    return value_condition && order_condition;
}

// Test whether the trails beginning at `max_node` are ordered correctly, as required by the uniqueness conditions.
SIGN_TEMPLATE
bool test_trail_order(const banana_tree_node<sign>* max_node) {
    massert(max_node->is_internal(), "Expected an internal node as input.");
    const auto* const birth = max_node->get_birth();
    const auto* mid = birth->get_mid();
    bool mid_is_ok = true;
    while (mid != max_node) {
        mid_is_ok &= list_item::is_between(*mid->get_item(), *mid->get_down()->get_item(), *mid->get_up()->get_item());
        mid_is_ok &= mid->get_value() > mid->get_down()->get_value() && mid->get_up()->get_value() > mid->get_value();
        mid = mid->get_up();
    }
    const auto* in = birth->get_in();
    bool in_is_ok = true;
    while (in != max_node) {
        if (in != max_node->get_in()) {
            in_is_ok &= list_item::is_between(*in->get_item(), *in->get_down()->get_item(), *in->get_up()->get_item());
        } else {
            in_is_ok &= list_item::is_between(*birth->get_item(), *in->get_item(), *max_node->get_item());
        }
        in_is_ok &= in->get_value() > in->get_down()->get_value() && in->get_up()->get_value() > in->get_value();
        in = in->get_up();
    }
    return mid_is_ok && in_is_ok;
}

// Test whether mid and in pointers of `max_node` match the mid and in pointers of its birth,
// i.e., that following down-pointers from mid leads to `max_node->get_birth()`.
SIGN_TEMPLATE
bool test_trail_pointer_match(const banana_tree_node<sign>* max_node) {
    massert(max_node->is_internal(), "Expected an internal node as input.");
    const auto* const birth = max_node->get_birth();
    bool mid_matches = false;
    bool in_matches = false;
    if (max_node->get_mid() == birth) {
        mid_matches = (birth->get_mid() == max_node);
    } else {
        const auto* mid_node = max_node->get_mid();
        while (mid_node != nullptr && mid_node->get_down() != birth) {
            mid_node = mid_node->get_down();
        }
        massert(mid_node != nullptr, "Ran into a disconnected trail.");
        mid_matches = (birth->get_mid() == mid_node);
    }
    if (max_node->get_in() == birth) {
        in_matches = (birth->get_in() == max_node);
    } else {
        const auto* in_node = max_node->get_in();
        while (in_node != nullptr && in_node->get_down() != birth) {
            in_node = in_node->get_down();
        }
        massert(in_node != nullptr, "Ran into a disconnected trail.");
        in_matches = (birth->get_in() == in_node);
    }
    return mid_matches && in_matches;
}

} // End of namespace `bananas`
