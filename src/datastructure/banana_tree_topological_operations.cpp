#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include "datastructure/banana_tree.h"
#include "datastructure/banana_tree_sign_template.h"
#include "datastructure/banana_tree_validation.h"
#include "datastructure/dictionary.h"
#include "datastructure/list_item.h"
#include "persistence_defs.h"
#include "utility/debug.h"
#include "utility/stats.h"

using namespace bananas;

//
// Implementation of `banana_stack`
//

SIGN_TEMPLATE
void internal::banana_stack<sign>::push(const banana_type &banana) {
    stack.push_back(banana);
    top_iter = stack.rbegin();
}

SIGN_TEMPLATE
void internal::banana_stack<sign>::pop() {
    massert(!empty(), "Attempted to pop from an already empty stack.");
    ++top_iter;
}

SIGN_TEMPLATE
void internal::banana_stack<sign>::actually_pop() {
    massert(!stack.empty(), "The stack has no bananas at all. There's nothing to pop, not even actually, really, totally.");
    stack.pop_back();
    reset_top();
}

SIGN_TEMPLATE
internal::banana_stack<sign>::banana_type& internal::banana_stack<sign>::top() const {
    massert(top_iter != stack.rend(), "Attempted to access top of empty stack.");
    return *top_iter;
}

SIGN_TEMPLATE
void internal::banana_stack<sign>::reset_top() {
    top_iter = stack.rbegin();
}

SIGN_TEMPLATE
bool internal::banana_stack<sign>::empty() const {
    return top_iter == stack.rend();
}

SIGN_TEMPLATE
std::optional<internal::stack_variant> internal::top_banana(banana_stack<sign> &L_stack,
                                                            banana_stack<sign> &M_stack,
                                                            banana_stack<sign> &R_stack,
                                                            banana_stack<-sign> &L_inv_stack,
                                                            banana_stack<-sign> &R_inv_stack) {
    std::optional<internal::stack_variant> result;
    function_value_type top_value = -std::numeric_limits<function_value_type>::infinity();
    for (auto &stack: {&L_stack, &M_stack, &R_stack}) {
        if (!stack->empty()) {
            auto top_max_val = stack->top().template get_max<sign>()->template value<sign>();
            if (top_max_val > top_value) {
                result.emplace(std::ref(*stack));
                top_value = top_max_val;
            }
        }
    }
    for (auto &stack: {&L_inv_stack, &R_inv_stack}) {
        if (!stack->empty()) {
            auto top_max_val = stack->top().template get_max<sign>()->template value<sign>();
            if (top_max_val > top_value) {
                result.emplace(std::ref(*stack));
                top_value = top_max_val;
            }
        }
    }
    return result;
}

void internal::pop_from_var_stack(internal::stack_variant &var) {
    std::visit([](auto&& stack_arg) { stack_arg.get().pop(); }, var);
}

void internal::actually_pop_from_var_stack(internal::stack_variant &var) {
    std::visit([](auto&& stack_arg) { stack_arg.get().actually_pop(); }, var);
}

SIGN_TEMPLATE
internal::item_pair<sign> internal::top_of_var_stack(internal::stack_variant &var) {
    auto [min, max] = std::visit([](auto&& stack_arg) {
        auto top = stack_arg.get().top();
        return std::make_pair(top.template get_min<sign>(), top.template get_max<sign>());
    }, var);
    return {min, max};
}

SIGN_TEMPLATE
bool internal::holds_stack(const internal::stack_variant &var, const banana_stack<sign> &stack) {
    using ref_wrap_type = std::reference_wrapper<std::remove_cvref_t<decltype(stack)>>;
    if (!holds_alternative<ref_wrap_type>(var)) {
        return false;
    }
    auto val = std::get<ref_wrap_type>(var);
    return &(val.get()) == &stack;
}

SIGN_TEMPLATE
std::optional<internal::stack_variant> internal::add_missing_short_wave_banana(internal::banana_stack<sign>& L_stack,
                                                                               internal::banana_stack<sign>& M_stack,
                                                                               internal::banana_stack<sign>& R_stack,
                                                                               internal::banana_stack<-sign>& L_inv_stack,
                                                                               internal::banana_stack<-sign>& R_inv_stack,
                                                                               function_value_type cut_value) {
    auto top_banana = internal::top_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack).value();
    massert(internal::holds_stack(top_banana, L_stack) ||
            internal::holds_stack(top_banana, M_stack) ||
            internal::holds_stack(top_banana, R_stack),
            "Expected the topmost banana in L, M, R.");
    auto* highest_max = internal::top_of_var_stack<sign>(top_banana).template get_max<sign>()->template get_node<sign>();
    // If the top banana is cut in the mid-panel and is not the special banana,
    // then there exists no short-wave window with the cut in its out-panel
    if (!highest_max->is_special_root() && internal::holds_stack(top_banana, M_stack)) {
        return {};
    }
    if (highest_max->is_special_root()) {
        return add_missing_short_wave_banana_sr(top_banana, M_stack, R_stack, L_inv_stack, R_inv_stack, cut_value);
    }
    const bool cuts_left = highest_max->is_on_left_spine();
    auto* next_on_spine = highest_max->get_in();
    if (next_on_spine->is_internal()) {
        list_item* mup_top_min = nullptr;
        list_item* dn_top_min = nullptr;
        if (!M_stack.empty()) {
            mup_top_min = M_stack.top().template get_min<sign>();
        }
        if (cuts_left && !L_inv_stack.empty()) {
            dn_top_min = L_inv_stack.top().template get_min<sign>();
        } else if (!cuts_left && !R_inv_stack.empty()) {
            dn_top_min = R_inv_stack.top().template get_min<sign>();
        }
        auto mup_min_value = mup_top_min != nullptr ? mup_top_min->value<sign>() : std::numeric_limits<function_value_type>::infinity();
        auto dn_min_value = dn_top_min != nullptr ? dn_top_min->value<sign>() : std::numeric_limits<function_value_type>::infinity();
        auto birth_val = next_on_spine->get_birth()->get_value();
        if (birth_val < mup_min_value && birth_val < dn_min_value && birth_val < cut_value) {
            if (cuts_left) {
                L_inv_stack.push({next_on_spine->get_item(), next_on_spine->get_birth()->get_item()});
                return {std::ref(L_inv_stack)};
            } else {
                R_inv_stack.push({next_on_spine->get_item(), next_on_spine->get_birth()->get_item()});
                return {std::ref(R_inv_stack)};
            }
        }
    }
    return {};
}
SIGN_TEMPLATE
std::optional<internal::stack_variant> internal::add_missing_short_wave_banana_sr(internal::stack_variant &top_banana,
                                                                                  internal::banana_stack<sign>& M_stack,
                                                                                  internal::banana_stack<sign>& R_stack,
                                                                                  internal::banana_stack<-sign>& L_inv_stack,
                                                                                  internal::banana_stack<-sign>& R_inv_stack,
                                                                                  function_value_type cut_value) {
    auto* top_max_node = internal::top_of_var_stack<sign>(top_banana).template get_max<sign>()->template get_node<sign>();
    massert(top_max_node->is_special_root(),
            "Called the function for the special case involving the special root, but the top banana doesn't have the special root as max.");
    massert(internal::holds_stack(top_banana, M_stack) ||
            internal::holds_stack(top_banana, R_stack),
            "Can't have a special banana with in-panel on the right.");
    const bool cuts_left = internal::holds_stack(top_banana, R_stack);
    auto* next_on_spine = cuts_left ? top_max_node->get_in() : top_max_node->get_mid();
    if (next_on_spine->is_internal()) {
        list_item* mup_top_min = nullptr;
        list_item* dn_top_min = nullptr;
        if (internal::holds_stack(top_banana, M_stack)) {
            M_stack.pop();
        }
        if (!M_stack.empty()) {
            mup_top_min = M_stack.top().template get_min<sign>();
        }
        M_stack.reset_top();
        if (cuts_left && !L_inv_stack.empty()) {
            dn_top_min = L_inv_stack.top().template get_min<sign>();
        } else if (!cuts_left && !R_inv_stack.empty()) {
            dn_top_min = R_inv_stack.top().template get_min<sign>();
        }
        auto mup_min_value = mup_top_min != nullptr ? mup_top_min->value<sign>() : std::numeric_limits<function_value_type>::infinity();
        auto dn_min_value = dn_top_min != nullptr ? dn_top_min->value<sign>() : std::numeric_limits<function_value_type>::infinity();
        auto birth_val = next_on_spine->get_birth()->get_value();
        if (birth_val < mup_min_value && birth_val < dn_min_value && birth_val < cut_value) {
            if (cuts_left) {
                L_inv_stack.push({next_on_spine->get_item(), next_on_spine->get_birth()->get_item()});
                return {std::ref(L_inv_stack)};
            } else {
                R_inv_stack.push({next_on_spine->get_item(), next_on_spine->get_birth()->get_item()});
                return {std::ref(R_inv_stack)};
            }
        }
    }
    return {};
}

namespace bananas {
    template class internal::banana_stack<1>;
    template class internal::banana_stack<-1>;

    template std::optional<internal::stack_variant> internal::top_banana<1>(banana_stack<1> &,
                                                                            banana_stack<1> &,
                                                                            banana_stack<1> &,
                                                                            banana_stack<-1> &,
                                                                            banana_stack<-1> &);
    template std::optional<internal::stack_variant> internal::top_banana<-1>(banana_stack<-1> &,
                                                                             banana_stack<-1> &,
                                                                             banana_stack<-1> &,
                                                                             banana_stack<1> &,
                                                                             banana_stack<1> &);
    template internal::item_pair<1> internal::top_of_var_stack<1>(internal::stack_variant &);
    template internal::item_pair<-1> internal::top_of_var_stack<-1>(internal::stack_variant &);
    template bool internal::holds_stack<1>(const internal::stack_variant &, const banana_stack<1> &);
    template bool internal::holds_stack<-1>(const internal::stack_variant &, const banana_stack<-1> &);
    template std::optional<internal::stack_variant> internal::add_missing_short_wave_banana<1>(internal::banana_stack<1> &,
                                                                                               internal::banana_stack<1> &,
                                                                                               internal::banana_stack<1> &,
                                                                                               internal::banana_stack<-1> &,
                                                                                               internal::banana_stack<-1> &,
                                                                                               function_value_type);
    template std::optional<internal::stack_variant> internal::add_missing_short_wave_banana<-1>(internal::banana_stack<-1> &,
                                                                                                internal::banana_stack<-1> &,
                                                                                                internal::banana_stack<-1> &,
                                                                                                internal::banana_stack<1> &,
                                                                                                internal::banana_stack<1> &,
                                                                                                function_value_type);
    template std::optional<internal::stack_variant> internal::add_missing_short_wave_banana_sr<1>(internal::stack_variant &,
                                                                                                  internal::banana_stack<1> &,
                                                                                                  internal::banana_stack<1> &,
                                                                                                  internal::banana_stack<-1> &,
                                                                                                  internal::banana_stack<-1> &,
                                                                                                  function_value_type);
    template std::optional<internal::stack_variant> internal::add_missing_short_wave_banana_sr<-1>(internal::stack_variant &,
                                                                                                   internal::banana_stack<-1> &,
                                                                                                   internal::banana_stack<-1> &,
                                                                                                   internal::banana_stack<1> &,
                                                                                                   internal::banana_stack<1> &,
                                                                                                   function_value_type);
}

//
// Implementation of the gluing algorithm in `banana_tree`
//

SIGN_TEMPLATE
void banana_tree<sign>::glue_to_right(tree_type &right_tree, signed_min_dictionary<sign> &min_dict) {
    massert(*right_endpoint < *right_tree.left_endpoint,
            "Expected items of `right_tree` to be to the right of the tree it's being glued to.");

    TIME_BEGIN(glue_preprocess);
    // Ensure up/down assumption:
    // One interval ends in an up-type item, the other ends in a down-type item
    // and the up-type item has lower value than the down-type item.
    auto [left_glue_node, right_glue_node] = prepare_gluing_to_right(right_tree, min_dict);
    auto *left_special_root = get_special_root();
    const auto *right_special_root = right_tree.get_special_root();

    // detect dummy node
    auto *dummy_node = left_glue_node->get_birth()->is_hook() ? left_glue_node->get_birth() : right_glue_node->get_birth();

    auto *left_max = left_glue_node->is_leaf() ? left_glue_node->death : left_glue_node;
    auto *right_max = right_glue_node->is_leaf() ? right_glue_node->death : right_glue_node;

    // Move the left special root to negative infinity, i.e., to the left of the interval.
    // This ensures that the right spine of the left tree consists only of in-trails.
    // We need to swap the in-trail and mid-trail for consistency
    left_special_root->item->assign_order(-std::numeric_limits<interval_order_type>::infinity());
    std::swap(left_special_root->in, left_special_root->mid);
    std::swap(left_special_root->low->in, left_special_root->low->mid);

    TIME_END(glue_preprocess, sign);

    // Whether the termination condition is true
    // If `terminate_left == true`, then the left tree is empty
    // If `terminate_right == true`, then the right tree is empty
    bool terminate_left = false;
    bool terminate_right = false;
    do {
        auto [candidate_max, other_max] = (left_max->get_value() < right_max->get_value())
            ? std::make_pair(left_max, right_max)
            : std::make_pair(right_max, left_max);
        auto* min_low = candidate_max->low; 
        auto* min_bth = candidate_max->get_birth() != dummy_node ? candidate_max->get_birth() : other_max->get_birth(); 

        min_bth->spine_label = internal::spine_pos::not_on_spine;

        massert(min_low->is_leaf(), "Expected `min_low` to be a leaf.");
        massert(min_bth->is_leaf(), "Expected `min_bth` to be a leaf.");
        massert(!min_bth->is_hook(), "`min_bth` is set to not be the dummy node for a very good reason.");
        massert(candidate_max->is_internal(), "Expected `candidate_max` to be an internal node.");
        massert(candidate_max != min_bth, "This should never happen, since `min_bth` is a leaf and `candidate_max` is internal.");
        // Use `-inf` as value of the low-pointer of special roots.
        // This matches the paper in that the low-pointer of special roots is defined to
        // point to a dummy with value -inf just for gluing the trees.
        auto low_val = candidate_max->is_special_root() ?
                                -std::numeric_limits<function_value_type>::infinity() :
                                min_low->get_value();
        // Note: we compare using `>=` instead of `>` in order to not get scares
        // when candidate_max is a special root and not paired with a hook
        if (min_bth->get_value() >= low_val) {
            if (min_bth == candidate_max->get_birth()) {
                undo_injury(candidate_max, other_max->get_birth());
            } else {
                undo_fatality(candidate_max->get_birth(), candidate_max, min_bth);
            }
        } else {
            massert(candidate_max->get_birth() == dummy_node,
                    "Expected scare to involve a maximum paired with the dummy.");
            undo_scare(candidate_max->get_birth());
        }
        if (candidate_max == left_max) {
            left_max = left_max->low->death;
        } else {
            right_max = right_max->low->death;
        }

        // Set terminate_left to true if the left special root is only connected to the dummy
        terminate_left = (left_special_root->in == left_special_root->mid) && (left_special_root->in == dummy_node);
        // Same for terminate_right
        terminate_right = (right_special_root->in == right_special_root->mid) && (right_special_root->in == dummy_node);
    } while (!terminate_left && !terminate_right);

    TIME_BEGIN(glue_postprocess);

    if (terminate_left) {
        // Left tree is empty -> need to move right tree to the left
        ensure_glued_tree_is_this(right_tree.special_root_item.template get_node<sign>(), dummy_node);
    }
    
    // Discard empty tree: first the dummy, then the special root.
    free_node(dummy_node);
    free_node(right_tree.special_root_item.template get_node<sign>());

    // Ensure that the hooks are items in `this`
    // The left hook must have come from the left tree, so `left_hook_item` remains `left_hook_item`
    // The right hook comes from the right tree, so `this->right_hook_item` must replace the item representing the right hook
    auto* right_hook_node = right_tree.right_hook_item.template get_node<sign>();
    if (right_hook_node != nullptr) {
        right_hook_node->replace_item(&(this->right_hook_item));
        assign_hook_value_and_order<false>();
    }

    // Reset the left special root to positive infinity,
    // and swap the in-trail and mid-trail if necessary
    // to ensure that the left trail is the in-trail and the right trail is the mid-trail.
    left_special_root->item->assign_order(std::numeric_limits<interval_order_type>::infinity());
    if (list_item::is_between(*left_special_root->in->item, *left_special_root->mid->item, *left_special_root->item)) {
        std::swap(left_special_root->in, left_special_root->mid);
        std::swap(left_special_root->low->in, left_special_root->low->mid);
    }

    // update the global max
    if (this->global_max->template value<sign>() < right_tree.global_max->template value<sign>()) {
        global_max = right_tree.global_max;
    }
    
    right_endpoint = right_tree.right_endpoint;

    TIME_END(glue_postprocess, sign);
    
    massert(left_special_root->is_special_root(), "Expected the special root of the glued tree to be a special root.");
}

SIGN_TEMPLATE
void banana_tree<sign>::undo_injury(node_ptr_type max_node,
                                    node_ptr_type other_min_node) {
    massert(max_node->is_internal(), "Expected `max_node` to be internal.");
    massert(other_min_node->is_leaf(), "Expected `other_min_node` to be a leaf.");
    massert(other_min_node->is_hook(), "Expected a hook as `other_min_node`.");

    PERSISTENCE_STAT(undo_injury, sign);
    TIME_BEGIN(undo_injury);

    // Find where to cut mid-trail of `other_min_node` and update low-pointers along the way
    const auto cut_value = max_node->get_value();
    auto* max_birth = max_node->get_birth();
    auto* cut_node = other_min_node->mid;
    if (cut_node->get_value() > cut_value) {
        max_node->spine_label = internal::spine_pos::not_on_spine;
        return;
    }
    while (cut_node->get_value() < cut_value) {
        cut_node->low = max_birth;
        
        massert(list_item::is_between(*max_node->get_birth()->get_item(), *cut_node->get_item(), *max_node->get_item()),
                "Expected `cut_node` to belong to an in-trail.");
        massert(list_item::is_between(*cut_node->get_item(), *cut_node->get_birth()->get_item(), *max_node->get_item()),
                "Expected `cut_node` to be between its birth and the death of its new trail.");

        cut_node = cut_node->up;
    }
    auto* node_below_cut = cut_node == other_min_node->death ? cut_node->mid : cut_node->down;
    auto* low_end_of_cut_trail = other_min_node->mid;

    // Remove the section of trail between `low_end_of_cut_trail` and `node_below_cut`
    cut_node->set_mid_or_down(other_min_node, cut_node == other_min_node->death);
    other_min_node->mid = cut_node;

    // Insert the section of trail below `max_node`
    auto* insert_point = max_node->in;
    max_node->in = node_below_cut;
    node_below_cut->up = max_node;
    insert_point->set_in_or_up(low_end_of_cut_trail);
    low_end_of_cut_trail->down = insert_point;

    if (!cut_node->is_special_root()) {
        max_node->spine_label = internal::spine_pos::not_on_spine;
    }

    TIME_END(undo_injury, sign);

    massert(test_trail_order(max_node), "Expected trails of `max_node` to be ordered according to uniqueness condition.");
}

SIGN_TEMPLATE
void banana_tree<sign>::undo_fatality(node_ptr_type min_node,
                                      node_ptr_type max_node,
                                      node_ptr_type other_min_node) {
    massert(min_node->is_leaf(), "Expected `min_node` to be a leaf.");
    massert(min_node->is_hook(), "Expected a hook as `min_node`.");
    massert(max_node->is_internal(), "Expected `max_node` to be internal.");
    massert(min_node->get_death() == max_node, "Expected `min_node` to be paired with `max_node`.");
    massert(other_min_node->is_leaf(), "Expected `other_min_node` to be a leaf.");
    massert(max_node->get_in()->is_leaf(), "Expected `max_node` to have an empty in-trail.");
    massert(max_node->get_value() > other_min_node->get_value(), "undo_fatality would pair a minimum with a maximum of lower value.");

    PERSISTENCE_STAT(undo_fatality, sign);
    TIME_BEGIN(undo_fatality);

    // If one of the maxima is a special root, we need to fix its low-pointer in the end.
    // Test for special-rootness now, then set low to birth later.
    const bool max_is_special_root = max_node->is_special_root();
    const bool other_mins_death_is_special_root = other_min_node->get_death()->is_special_root();

    // find the place where to cut the in- and mid-trails of `other_min_node`
    const auto cut_value = max_node->get_value();
    auto* top_of_in = other_min_node->death->in;
    while (top_of_in->get_value() >= cut_value) {
        top_of_in = top_of_in->down;
    }
    auto* top_of_mid = other_min_node->death->mid;
    bool cuts_mid_below_special_root = other_min_node->death->is_special_root();
    while (top_of_mid->get_value() >= cut_value) {
        cuts_mid_below_special_root = top_of_mid->is_special_root();
        top_of_mid = top_of_mid->down;
    }
    massert(top_of_in->low == other_min_node, "Expected `top_of_in` to be on trail to `other_min_node`.");
    massert(top_of_mid->low == other_min_node, "Expected `top_of_mid` to be on trail to `other_min_node`.");
    // swap `min_node` and `other_min_node` (along with the pieces of trail attached to `other_min_node`)
    fatality_swap(top_of_in, top_of_mid, min_node);
    massert(test_trail_pointer_match(max_node), "Mismatched trails after fatality swap.");

    if (!cuts_mid_below_special_root) {
        max_node->spine_label = internal::spine_pos::not_on_spine;
    }

    // If the max_node is a special root, then its low-pointer changes,
    // since for special roots `low == get_birth()`.
    if (max_is_special_root) {
        max_node->low = max_node->get_birth();
    }
    if (other_mins_death_is_special_root) {
        min_node->death->low = min_node;
    }

    TIME_END(undo_fatality, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::undo_scare(node_ptr_type min_node) {
    PERSISTENCE_STAT(undo_scare, sign);
    TIME_BEGIN(undo_scare);

    auto* const max_node = min_node->death;
    massert(!max_node->is_special_root(), "The special banana does not experience a scare.");
    massert(max_node->is_internal(), "Expected `max_node` to be internal.");
    massert(min_node->is_leaf(), "Expected `min_node` to be a leaf.");
    massert(min_node->is_hook(), "Expected a hook as minimum in `undo_scare`");
    massert(max_node->get_low() != min_node, "Expected `min_node` to drop below another node.");
    min_node->item->assign_value(add_tiniest_offset< -sign>(max_node->get_low()->get_item()->template value<1>()));
    min_node->min_interchange_below(max_node->low);
    // This is a bit of a hack, since min_interchange assumes the usual "in-trail of special root is left",
    // but we changed this for the left tree during gluing.
    // This means we have to fix it now.
    // Basically, if `max_node` has a special root at `up`, then it's on a spine,
    // and we assign left/right based on its position relative to the special bananas lower end.
    // If `max_node` does not have a special root at `up` it will end up in a mid-panel after the interchange,
    // so it won't be on any spine.
    if (max_node->get_up()->is_special_root()) {
        massert(max_node->is_on_spine(), "Expected node with special root as up to be on a spine.");
        if (*(max_node->get_item()) < *(max_node->get_low()->get_item())) {
            max_node->spine_label = internal::spine_pos::on_left_spine;
        } else {
            max_node->spine_label = internal::spine_pos::on_right_spine;
        }
    }

    TIME_END(undo_scare, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::fatality_swap(node_ptr_type top_of_in,
                                      node_ptr_type top_of_mid,
                                      node_ptr_type node) {
    massert(top_of_in->get_low() == top_of_mid->get_low(),
            "Expected `top_of_in` and `top_of_mid` to belong to the same banana.");
    massert(node->is_leaf(), "Expected to swap with a leaf.");
    massert(node->is_hook(), "Excepted to swap with a hook/dummy node.");
    auto* const above_top_of_in = top_of_in->is_internal()
                                        ? top_of_in->up
                                        : top_of_in->in;
    const bool moves_full_in_trail = (above_top_of_in->in == top_of_in);

    auto* const above_top_of_mid = top_of_mid->is_internal()
                                        ? top_of_mid->up
                                        : top_of_mid->mid;
    const bool moves_full_mid_trail = (above_top_of_mid->mid == top_of_mid);

    auto* const node_in = node->in;
    // Since `node` is a hook, its in-trail is always empty.
    // For details see the correctness proof in the paper.
    massert(node_in == node->death, "Expected the in-trail of the swapped minimum to be empty.");
    const bool node_has_empty_in = true; 
    auto* const node_mid = node->mid;
    const bool node_has_empty_mid = (node_mid == node->death);

    // First, update low-pointers on trails beginning at `node`.
    // The in-trail is empty (`node_in == node->death`), so we only iterate over the mid-trail
    for (auto* it = node_mid; it != node->get_death(); it = it->up) {
        it->low = top_of_in->low;
    }

    // Trails towards `top_of_in` and `top_of_mid` change from in to mid and mid to in, respectively.
    // Hence we update the `in` and `mid` pointer of their lower end.
    // This has to be done first, otherwise, we miss assignment of some pointers
    std::swap(top_of_in->low->in, top_of_in->low->mid);
    // The in-trail below `top_of_in` attaches to the mid-trail below `node->death`
    // The mid-trail below `top_of_mid` attaches to the in-trail below `node->death`
    // Connect `top_of_in` to `node_mid`
    top_of_in->set_mid_or_up(node_mid);
    node_mid->set_mid_or_down(top_of_in, node_has_empty_mid);
    // Connect `top_of_mid` to `node_in`
    top_of_mid->set_in_or_up(node_in);
    node_in->set_in_or_down(top_of_mid, node_has_empty_in);

    // Connect `node` to `above_top_of_in`
    node->in = above_top_of_in;
    above_top_of_in->set_in_or_down(node, moves_full_in_trail);
    // Connect `node` to `above_top_of_mid`
    node->mid = above_top_of_mid;
    above_top_of_mid->set_mid_or_down(node, moves_full_mid_trail);

    // Update the death pointers of the swapped leaves
    std::swap(top_of_in->low->death, node->death);

    // Now again, update the low-pointers on trails beginning at `node`
    for (auto* it = node->mid; it != node->get_death(); it = it->up) {
        it->low = node;
    }
    for (auto* it = node->in; it != node->get_death(); it = it->up) {
        it->low = node;
    }
}

SIGN_TEMPLATE
std::pair<banana_tree_node<sign>*, banana_tree_node<sign>*>
        banana_tree<sign>::prepare_gluing_to_right(tree_type &right_tree,
                                                   signed_min_dictionary<sign> &min_dict) {
    auto * const end_of_left = right_endpoint;
    auto * const end_of_right = right_tree.left_endpoint;
    auto left_value = end_of_left->value<sign>();
    auto right_value = end_of_right->template value<sign>();
    auto *left_node = end_of_left->get_node<sign>();
    auto *right_node = end_of_right->template get_node<sign>();

    auto *left_tree_glue_node = left_node;
    auto *right_tree_glue_node = right_node;

    const bool left_ends_in_down = end_of_left->is_down_type<sign>();
    const bool left_ends_in_up = end_of_left->is_up_type<sign>();
    const bool right_begins_with_down = end_of_right->template is_down_type<sign>();
    const bool right_begins_with_up = end_of_right->template is_up_type<sign>();

    if (left_ends_in_down && right_begins_with_down) {
        // remove the down-type with smaller value (including its hook)
        if (left_value < right_value) {
            left_tree_glue_node = min_dict.previous_item(*end_of_left)->template get_node<sign>();
            massert(left_tree_glue_node != nullptr, "Expected to find another minimum to the left.");
            remove_node_with_hook(left_node);
        } else {
            right_tree_glue_node = min_dict.next_item(*end_of_left)->template get_node<sign>();
            massert(right_tree_glue_node != nullptr, "Expected to find another minimum to the right.");
            remove_node_with_hook(right_node);
        }
    } else if (left_ends_in_up && right_begins_with_up) {
        // replace the up-type with greater value by a hook
        if (left_value > right_value) {
            left_tree_glue_node = turn_node_into_hook<false>(left_node);
        } else {
            right_tree_glue_node = right_tree.turn_node_into_hook<true>(right_node);
        }
    } else if (left_ends_in_up && left_value > right_value) {
        // replace end_of_left by a hook
        left_tree_glue_node = turn_node_into_hook<false>(left_node);
        // remove right_node
        right_tree_glue_node = min_dict.next_item(*end_of_left)->template get_node<sign>();
        massert(right_tree_glue_node != nullptr, "Expected to find another minimum to the right.");
        remove_node_with_hook(right_node);
    } else if (right_begins_with_up && right_value > left_value) {
        // replace end_of_right by a hook
        right_tree_glue_node = right_tree.turn_node_into_hook<true>(right_node);
        // remove left_node
        left_tree_glue_node = min_dict.previous_item(*end_of_left)->template get_node<sign>();
        massert(left_tree_glue_node != nullptr, "Expected to find another minimum to the left.");
        remove_node_with_hook(left_node);
    }
    // We may have removed the global max, so update it now.
    this->update_global_max();
    right_tree.update_global_max();
    // If none of the four cases above occur, then
    //  - one of the two endpoints is an up-type item
    //  - the other is a down-type item
    //  - and the up-type item has a lower value than the down-type item
    // as required by the gluing algorithm.
    return {left_tree_glue_node, right_tree_glue_node};
}

SIGN_TEMPLATE
void banana_tree<sign>::remove_node_with_hook(node_ptr_type node_with_hook) {
    massert(!node_with_hook->is_special_root(), "Can't remove a special root.");
    massert(node_with_hook->is_internal(), "Expected to remove an internal node.");
    massert(node_with_hook->has_empty_banana(), "Trails need to be empty.");
    massert(node_with_hook->get_in()->is_hook(), "Expected the node to be removed to be paired with a hook.");
    auto* birth = node_with_hook->get_birth();
    node_with_hook->unlink_from_trail();
    free_node(node_with_hook);
    free_node(birth);
}

SIGN_TEMPLATE
template<bool left>
banana_tree<sign>::node_ptr_type banana_tree<sign>::turn_node_into_hook(node_ptr_type new_hook_node) {
    auto * const endpoint = left ? left_endpoint : right_endpoint;
    assign_hook_value_and_order<left>(endpoint);
    new_hook_node->replace_item(left ? &left_hook_item : &right_hook_item);
    return new_hook_node->death;
}

SIGN_TEMPLATE
void banana_tree<sign>::ensure_glued_tree_is_this(node_ptr_type other_special_root, node_ptr_type dummy_node) {
    auto* this_special_root = special_root_item.get_node<sign>();

    massert(this_special_root->has_empty_banana(), "Expected the left special root to have empty trails.");
    massert(this_special_root->get_birth()->is_hook(), "Expected the left special root to have a hook as the lower end.");

    this_special_root->in = other_special_root->in;
    this_special_root->mid = other_special_root->mid;
    this_special_root->low = other_special_root->low;
    this_special_root->death = this_special_root->up = this_special_root->down = nullptr;
    this_special_root->in->set_in_or_up(this_special_root);
    this_special_root->mid->set_mid_or_up(this_special_root);
    this_special_root->low->death = this_special_root;

    other_special_root->in = dummy_node;
    other_special_root->mid = dummy_node;
    other_special_root->low = dummy_node;
    dummy_node->in = other_special_root;
    dummy_node->mid = other_special_root;
}

//
// Implementation of the cutting algorithm in `banana_tree`
//

SIGN_TEMPLATE
internal::item_pair<sign> banana_tree<sign>::smallest_banana(const list_item& virtual_item,
                                                             signed_min_dictionary<sign> &min_dict,
                                                             signed_max_dictionary<sign> &max_dict) const {
    auto left_min_it = min_dict.previous_item(virtual_item);
    auto right_min_it = min_dict.next_item(virtual_item);
    auto left_max_it = max_dict.previous_item(virtual_item);
    auto right_max_it = max_dict.next_item(virtual_item);
    massert(left_min_it != min_dict.end() && left_max_it != max_dict.end(), "Expected both a max and a min to the left of `item`.");
    massert(right_min_it != min_dict.end() && right_max_it != max_dict.end(), "Expected both a max and a min to the right of `item`.");
    auto& left_min = *left_min_it;
    auto& right_min = *right_min_it;
    auto& left_max = *left_max_it;
    auto& right_max = *right_max_it;
    node_ptr_type node_a;
    node_ptr_type node_b;
    bool compare_less;
    // TODO: handle the following cases:
    //   - one side has a min, but no max
    //   - one side has a max, but no min
    //   - one side has a min, but no max and the other side has a max, but no min
    // These are all valid cutting cases and should not be ignored
    if (list_item::is_between(left_max, left_min, virtual_item)) {
        node_a = right_min.template get_node<sign>();
        node_b = left_max.template get_node<sign>();
        compare_less = false;
    } else {
        node_a = left_min.template get_node<sign>();
        node_b = right_max.template get_node<sign>();
        compare_less = true;
    }
    massert(node_b->is_internal(), "Expected a maximum, but got a leaf.");
    massert(node_a->is_leaf(), "Expected a minimum, but didn't get a leaf.");
    node_ptr_type node_q;
    node_ptr_type node_r;
    auto compare = compare_less ? [](list_item* a, list_item *b) { return *a < *b; }
                                : [](list_item* a, list_item *b) { return *a > *b; };
    if (compare(node_b->get_down()->get_item(), node_b->get_item())) {
        node_q = node_b->low->death;
        node_r = node_b->down;
    } else {
        node_q = node_b;
        node_r = node_b->mid;
    }
    while (node_r != node_a && virtual_item.value<sign>() < node_r->get_value()) {
        node_q = node_r;
        node_r = node_r->in;
    }
    massert(node_q->is_internal(), "Should terminate with `node_q` as an internal node.");
    return {node_q->get_birth()->item, node_q->item};
}

SIGN_TEMPLATE
void banana_tree<sign>::load_stacks(const list_item& virtual_item,
                                    const internal::item_pair<sign>& smallest_banana,
                                    internal::banana_stack<sign> &L_stack,
                                    internal::banana_stack<sign> &M_stack,
                                    internal::banana_stack<sign> &R_stack) const {
    TIME_BEGIN(load_stacks);

    auto* node_p = smallest_banana.template get_min<sign>()->template get_node<sign>();
    auto* node_q = smallest_banana.template get_max<sign>()->template get_node<sign>();
    while (true) {
        if (*(node_p->get_item()) < virtual_item && *(node_q->get_item()) < virtual_item) {
            L_stack.push({node_p->item, node_q->item});
        } else if (*(node_p->get_item()) < virtual_item && *(node_q->get_item()) > virtual_item) {
            M_stack.push({node_p->item, node_q->item});
        } else if (*(node_p->get_item()) > virtual_item && *(node_q->get_item()) < virtual_item) {
            M_stack.push({node_p->item, node_q->item});
        } else { // if (*(node_p->get_item()) > virtual_item && *(node_q->get_item()) > virtual_item) {
            R_stack.push({node_p->item, node_q->item});
        }
        if (node_q->is_on_spine()) {
            break;
        }
        node_p = node_q->low;
        node_q = node_p->death;
    }

    TIME_END(load_stacks, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::initialize_empty_cut_tree(bool left) {
    auto* special_root_node = allocate_node(&special_root_item);
    node_ptr_type hook_node;
    if (left) {
        special_root_item.assign_order(-std::numeric_limits<interval_order_type>::infinity());
        allocate_node(&right_hook_item);
        hook_node = right_hook_item.template get_node<sign>();
        hook_node->spine_label = internal::spine_pos::on_right_spine;
    } else {
        allocate_node(&left_hook_item);
        hook_node = left_hook_item.template get_node<sign>();
        hook_node->spine_label = internal::spine_pos::on_left_spine;
    }
    special_root_node->set_pointers(nullptr, nullptr,
                                    hook_node, hook_node,
                                    hook_node, nullptr);
    hook_node->set_pointers(nullptr, nullptr,
                                  special_root_node, special_root_node,
                                  hook_node, special_root_node);
}

// Helper function for `cut`: assign to `dummy_item` a value just below the topmost minimum on the stacks.
SIGN_TEMPLATE
void assign_dummy_value(list_item* dummy_item,
                        internal::banana_stack<sign> &L_stack,
                        internal::banana_stack<sign> &M_stack,
                        internal::banana_stack<sign> &R_stack,
                        internal::banana_stack<-sign> &L_inv_stack,
                        internal::banana_stack<-sign> &R_inv_stack) {
    auto top_banana = internal::top_banana(L_stack,
                                           M_stack,
                                           R_stack,
                                           L_inv_stack,
                                           R_inv_stack).value();
    auto* lowest_min = internal::top_of_var_stack<sign>(top_banana).template get_min<sign>();
    dummy_item->assign_value(add_tiniest_offset<-sign>(lowest_min->template value<1>()));
}

SIGN_TEMPLATE
bool banana_tree<sign>::cut(list_item& cut_item,
                            list_item& left_of_cut,
                            list_item& right_of_cut,
                            tree_type &other_tree,
                            internal::banana_stack<sign> &L_stack,
                            internal::banana_stack<sign> &M_stack,
                            internal::banana_stack<sign> &R_stack,
                            internal::banana_stack<-sign> &L_inv_stack,
                            internal::banana_stack<-sign> &R_inv_stack) {
    TIME_BEGIN(cut_preprocess);

    // If the top banana on L_inv_stack (or R_inv_stack) has a node on the spine in the opposite tree,
    // then this banana is not a banana in this tree.
    // We pop these bananas from their stack since we don't need to process them here.
    // They are placed back on the stack at the very end of this function.
    // TODO: check that this always works
    std::optional<internal::item_pair<-sign>> L_inv_top;
    std::optional<internal::item_pair<-sign>> R_inv_top;
    if (!L_inv_stack.empty() && L_inv_stack.top().template get_max<-sign>()->template get_node<-sign>()->is_on_spine()) {
        L_inv_top = L_inv_stack.top();
        L_inv_stack.actually_pop();
    }
    if (!R_inv_stack.empty() && R_inv_stack.top().template get_max<-sign>()->template get_node<-sign>()->is_on_spine()) {
        R_inv_top = R_inv_stack.top();
        R_inv_stack.actually_pop();
    }
    auto modified_stack_opt = internal::add_missing_short_wave_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack, cut_item.value<sign>());

    // First determine whether we cut left or right.
    auto top_banana_var = internal::top_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack).value();
    auto top_banana = internal::top_of_var_stack<sign>(top_banana_var);
    auto top_max_node = top_banana.template get_max<sign>()->template get_node<sign>();
    bool cuts_left;
    if (top_max_node->is_special_root()) {
        DEBUG_MSG("top_max_node " << top_max_node->item->get_interval_order() << " is a special root.");
        DEBUG_MSG("  Comparing cut_item " << cut_item.get_interval_order() << " to global min " << top_max_node->get_low()->item->get_interval_order());
        // Always need to cut left: trivially, if we cut in the in-panel;
        // if we cut in the mid-panel then we need to move the global minimum and all that's attached to the left
        cuts_left = true;
    } else if (top_max_node->is_on_left_spine()) {
        DEBUG_MSG("top_max_node " << top_max_node->item->get_interval_order() << " is on the left spine.");
        cuts_left = true;
    } else if (top_max_node->is_on_right_spine()) {
        DEBUG_MSG("top_max_node " << top_max_node->item->get_interval_order() << " is on the right spine.");
        cuts_left = false;
    } else {
        __builtin_unreachable();
    }

    DEBUG_MSG("Cutting tree of sign " << sign << " on the " << (cuts_left ? "left" : "right"));

    // Now make sure that the trees are initialized and update their endpoints.
    other_tree.initialize_empty_cut_tree(cuts_left);
    assign_dummy_value(other_tree.get_special_root()->low->item, L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack);
    if (cuts_left) {
        other_tree.left_endpoint = this->left_endpoint;
        other_tree.right_endpoint = &left_of_cut;
        this->left_endpoint = &right_of_cut;
    } else {
        other_tree.left_endpoint = &right_of_cut;
        other_tree.right_endpoint = this->right_endpoint;
        this->right_endpoint = &left_of_cut;
    }

    if (left_of_cut.is_up_type<sign>()) {
        left_of_cut.get_node<sign>()->spine_label = internal::spine_pos::on_right_spine;
    } else {
        right_of_cut.get_node<sign>()->spine_label = internal::spine_pos::on_left_spine;
    }

    TIME_END(cut_preprocess, sign);

    // Now do the actual splitting
    auto* dummy_node = other_tree.get_special_root()->get_birth();
    cut_loop(cut_item, dummy_node, L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack);

    TIME_BEGIN(cut_postprocess);

    // Ensure correct position of the special root of `other_tree`
    // and correct in-/mid-trails in special banana of `other_tree`
    other_tree.fix_special_root_after_cut(cuts_left);
    massert(this->get_special_root()->is_special_root(), "Expected the special root to be a special root, but it's not.");
    // Assign order of `dummy_node`/its item.
    // Reassign hook nodes to hook items in left/right tree.
    update_hooks_after_cut(other_tree, left_of_cut, right_of_cut, dummy_node, cuts_left);

    update_global_max();

    // Clean up the stack we modified in the beginning.
    if (modified_stack_opt.has_value()) {
        internal::actually_pop_from_var_stack(modified_stack_opt.value());
    }
    if (L_inv_top.has_value()) {
        L_inv_stack.push(L_inv_top.value());
    }
    if (R_inv_top.has_value()) {
        R_inv_stack.push(R_inv_top.value());
    }

    TIME_END(cut_postprocess, sign);

    return cuts_left;
}

SIGN_TEMPLATE
void banana_tree<sign>::cut_loop(list_item& cut_item,
                                 node_ptr_type dummy_node,
                                 internal::banana_stack<sign> &L_stack,
                                 internal::banana_stack<sign> &M_stack,
                                 internal::banana_stack<sign> &R_stack,
                                 internal::banana_stack<-sign> &L_inv_stack,
                                 internal::banana_stack<-sign> &R_inv_stack) {
    while (true) {
        auto stack_opt = internal::top_banana(L_stack, M_stack, R_stack, L_inv_stack, R_inv_stack);
        if (!stack_opt.has_value()) {
            break;
        }
        auto& stack_var = stack_opt.value();
        auto item_pair = internal::top_of_var_stack<sign>(stack_var);
        auto* min_node = item_pair.template get_min<sign>()->template get_node<sign>();
        auto* max_node = item_pair.template get_max<sign>()->template get_node<sign>();
        internal::pop_from_var_stack(stack_var);
        if (internal::holds_stack(stack_var, L_stack)) {
            DEBUG_MSG("do_injury (L) with min " << min_node->item->get_interval_order() << " and max " << max_node->item->get_interval_order());
            max_node->spine_label = internal::spine_pos::on_right_spine;
            do_injury(cut_item, max_node, dummy_node);
        } else if (internal::holds_stack(stack_var, M_stack)) {
            if (cut_item < *(max_node->get_item())) {
                max_node->spine_label = internal::spine_pos::on_left_spine;
            } else {
                max_node->spine_label = internal::spine_pos::on_right_spine;
            }
            DEBUG_MSG("do_fatality with min " << min_node->item->get_interval_order() << " and max " << max_node->item->get_interval_order());
            do_fatality(cut_item, min_node, max_node, dummy_node);
        } else if (internal::holds_stack(stack_var, R_stack)) {
            DEBUG_MSG("do_injury (R) with min " << min_node->item->get_interval_order() << " and max " << max_node->item->get_interval_order());
            max_node->spine_label = internal::spine_pos::on_left_spine;
            do_injury(cut_item, max_node, dummy_node);
        } else if (internal::holds_stack(stack_var, L_inv_stack)) {
            DEBUG_MSG("do_scare (L) with min " << min_node->item->get_interval_order() << " and max " << max_node->item->get_interval_order());
            do_scare(min_node, dummy_node);
        } else if (internal::holds_stack(stack_var, R_inv_stack)) {
            DEBUG_MSG("do_scare (R) with min " << min_node->item->get_interval_order() << " and max " << max_node->item->get_interval_order());
            do_scare(min_node, dummy_node);
        }
    }
}

SIGN_TEMPLATE
void banana_tree<sign>::fix_special_root_after_cut(bool cuts_left) {
    if (cuts_left) {
        massert(special_root_item.get_interval_order() == -std::numeric_limits<interval_order_type>::infinity(),
                "Expected special root to be at negative infinity when `cuts_left == true`.");
        special_root_item.assign_order(std::numeric_limits<interval_order_type>::infinity());
        auto* special_root_node = get_special_root();
        std::swap(special_root_node->in, special_root_node->mid);
        std::swap(special_root_node->low->in, special_root_node->low->mid);
    } else {
        massert(special_root_item.get_interval_order() == std::numeric_limits<interval_order_type>::infinity(),
                "Expected special root to already be at infinity when `cuts_left == false`.");
    }
    auto* special_root_node = get_special_root();
    special_root_node->in->spine_label = internal::spine_pos::on_left_spine;
    special_root_node->mid->spine_label = internal::spine_pos::on_right_spine;
    update_global_max();
    massert(global_max != nullptr, "Expected to have a global max.");
    massert(get_special_root()->is_special_root(), "Expected the special root to be a special root, but it's not.");
}

SIGN_TEMPLATE
void banana_tree<sign>::update_hooks_after_cut(banana_tree& other_tree,
                                               list_item& left_of_cut,
                                               list_item& right_of_cut,
                                               node_ptr_type dummy_node,
                                               bool cuts_left) {
    if (cuts_left) {
        if (this->get_left_hook() != nullptr) {
            massert(other_tree.left_hook_item.template get_node<sign>() == nullptr,
                    "Expected new tree's left hook item not to have a node assigned when cutting in the left spine.");
            this->left_hook_item.template get_node<sign>()->replace_item(&other_tree.left_hook_item);
            other_tree.assign_hook_value_and_order<true>();
            massert(this->get_left_hook() == nullptr,
                    "Expected the old tree to no longer have a left hook after cutting the left spine and before reassigning the dummy.");
        }
        if (right_of_cut.is_maximum<sign>() || right_of_cut.is_down_type<sign>()) {
            // New down-type is not in `other_tree`, so we need to move the dummy
            dummy_node->replace_item(&this->left_hook_item);
            dummy_node->spine_label = internal::spine_pos::on_left_spine;
            this->assign_hook_value_and_order<true>();
        } else {
            massert(other_tree.get_right_hook() == dummy_node,
                    "Expected the right hook of the new tree to be assigned to the dummy"
                    "when the maximum is on the left of the (left) cut.");
            other_tree.assign_hook_value_and_order<false>(); 
            dummy_node->spine_label = internal::spine_pos::on_right_spine;
        }
    } else {
        if (this->get_right_hook() != nullptr) {
            massert(other_tree.right_hook_item.template get_node<sign>() == nullptr,
                    "Expected new tree's right hook item not to have a node assigned when cutting in the right spine.");
            this->right_hook_item.template get_node<sign>()->replace_item(&other_tree.right_hook_item);
            other_tree.assign_hook_value_and_order<false>();
            massert(this->get_right_hook() == nullptr,
                    "Expected the old tree to no longer have a right hook after cutting the left spine and before reassigning the dummy.");
        }
        if (left_of_cut.is_maximum<sign>() || left_of_cut.is_down_type<sign>()) {
            dummy_node->replace_item(&this->right_hook_item);
            dummy_node->spine_label = internal::spine_pos::on_right_spine;
            this->assign_hook_value_and_order<false>();
        } else {
            massert(other_tree.get_left_hook() == dummy_node,
                    "Expected the left hook of the new tree to be assigned to the dummy"
                    "when the maximum is on the right of the (right) cut.");
            other_tree.assign_hook_value_and_order<true>(); 
            dummy_node->spine_label = internal::spine_pos::on_left_spine;
        }
    }
}

SIGN_TEMPLATE
void banana_tree<sign>::do_injury(list_item& cut_item,
                                  node_ptr_type max_node,
                                  node_ptr_type dummy_node) {
    massert(max_node->is_internal(), "Expected an internal node for a maximum.");
    massert(dummy_node->is_hook(), "Expected a dummy.");

    PERSISTENCE_STAT(do_injury, sign);
    TIME_BEGIN(do_injury);

    if (!list_item::is_between(cut_item, *max_node->in->item, *max_node->item)) {
        TIME_END(do_injury, sign);
        return;
    }
    massert(max_node->in != max_node->get_birth(),
            "Expected injury on non-empty trail.");

    auto* top_of_in = max_node->in;
    auto* node_below_cut = max_node->in;
    while (list_item::is_between(cut_item, *node_below_cut->item, *max_node->item)) {
        // This node is going to be moved to a new trail, so update the low pointer now.
        node_below_cut->low = dummy_node;
        node_below_cut = node_below_cut->down;
    }
    auto* node_above_cut = node_below_cut->is_leaf() ? node_below_cut->in : node_below_cut->up;
    massert(node_above_cut != max_node->get_birth(),
            "Expected min and max of injured banana on the same side of the cut.");

    // Close the old trail
    max_node->in = node_below_cut;
    node_below_cut->set_in_or_up(max_node);

    // Insert the section between `top_of_in` and `node_above_cut` into
    // the mid-trail between `dummy_node->death` (specifically `dummy_node->mid`) and `dummy_node`
    dummy_node->mid->set_mid_or_down(top_of_in, dummy_node->get_mid() == dummy_node->get_death());
    top_of_in->up = dummy_node->mid;
    dummy_node->mid = node_above_cut;
    node_above_cut->down = dummy_node;

    update_dummy_position_in_cut(dummy_node);

    TIME_END(do_injury, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::do_fatality(list_item& cut_item, node_ptr_type min_node, node_ptr_type max_node, node_ptr_type dummy_node) {
    massert(min_node->is_leaf(), "Expected a leaf for a minimum.");
    massert(max_node->is_internal(), "Expected an internal node for a maximum.");
    massert(min_node->get_death() == max_node, "Expected min and max to be paired.");
    massert(list_item::is_between(cut_item, *min_node->item, *max_node->item), "Expected `cut_item` in mid-panel.");
    massert(dummy_node->is_hook(), "Expected a dummy.");

    PERSISTENCE_STAT(do_fatality, sign);
    TIME_BEGIN(do_fatality);

    // `dummy_node->death` or `max_node` may be special roots,
    // in which case their low-pointers need to be updated.
    // Its easiest to detect this here, before doing anything else
    if (dummy_node->death->is_special_root()) {
        DEBUG_MSG("do_fatality with special root as death of dummy_node. Order is "
                  << dummy_node->death->get_item()->get_interval_order());
        dummy_node->death->low = min_node;
    }
    if (max_node->is_special_root()) {
        DEBUG_MSG("do_fatality with special root as max_node. Order is "
                  << max_node->get_item()->get_interval_order());
        massert(max_node->in->low == max_node->mid->low, "do_fatality on an invalid banana.");
        max_node->low = dummy_node;
    }

    // Select top of moved in-trail and top of moved mid-trail,
    // also update the low pointers of the unmoved nodes on the mid-trail.
    auto* top_of_in = max_node->in;
    auto* above_top_of_mid = max_node;
    auto* top_of_mid = max_node->mid;
    while (list_item::is_between(*top_of_mid->item, cut_item, *max_node->item)) {
        top_of_mid->low = dummy_node;
        above_top_of_mid = top_of_mid;
        top_of_mid = top_of_mid->down;
    }
    // These pointers need to be swapped, since the in-trail becomes a mid-trail and vice versa
    std::swap(min_node->in, min_node->mid);
    // Connect top_of_in to dummy_node->mid; connect top_of_mid to dummy_node->in
    dummy_node->mid->set_mid_or_down(top_of_in, dummy_node->mid == dummy_node->death);
    top_of_in->set_mid_or_up(dummy_node->mid);
    dummy_node->in->set_in_or_down(top_of_mid, true); // dummy_node->in == dummy_node->death
    top_of_mid->set_in_or_up(dummy_node->in);
    
    // Make dummy_node max_node->in and connect it to above_top_of_mid
    dummy_node->in = max_node;
    max_node->in = dummy_node;
    dummy_node->mid = above_top_of_mid;
    above_top_of_mid->set_mid_or_down(dummy_node, above_top_of_mid == max_node);

    // Update death of min_node and dummy_node
    std::swap(dummy_node->death, min_node->death);

    // Update low pointers of nodes above top_of_in and top_of_mid to point to min_node
    // But the in-trail above top_of_mid is empty, since the in-trail above dummy was empty.
    // Hence we only update nodes above top_of_in, i.e., those on the mid-trail.
    auto* up_node = top_of_in == min_node ? top_of_in->mid : top_of_in->up;
    while (up_node != min_node->death) {
        DEBUG_MSG("Updating a low pointer on the mid trail of min_node");
        up_node->low = min_node;
        up_node = up_node->up;
    }

    update_dummy_position_in_cut(dummy_node);

    TIME_END(do_fatality, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::do_scare(node_ptr_type min_node, node_ptr_type dummy_node) {
    massert(min_node->is_leaf(), "Expected a leaf for a minimum.");
    massert(dummy_node->is_hook(), "Expected a dummy.");

    PERSISTENCE_STAT(do_scare, sign);
    TIME_BEGIN(do_scare);

    dummy_node->item->assign_value(add_tiniest_offset<sign>(min_node->item->template value<1>())); // TODO: check this again
    min_node->min_interchange_below(dummy_node);

    TIME_END(do_scare, sign);
}

SIGN_TEMPLATE
void banana_tree<sign>::update_dummy_position_in_cut(node_ptr_type dummy_node) {
    bool is_left_end = true;
    if (dummy_node->death != dummy_node->mid) {
        is_left_end = *(dummy_node->mid->item) < *(dummy_node->mid->up->item);
    } else {
        is_left_end = *(dummy_node->death->item) < *(dummy_node->death->low->item);
    }
    if (is_left_end) {
        dummy_node->item->assign_order(add_tiniest_offset<-1>(dummy_node->mid->item->get_interval_order()));
    } else {
        dummy_node->item->assign_order(add_tiniest_offset<1>(dummy_node->mid->item->get_interval_order()));
    }
}

//
// Explicit specialization
//
namespace bananas {
    template class banana_tree<1>;
    template class banana_tree< -1>;

    template banana_tree<1>::node_ptr_type banana_tree<1>::turn_node_into_hook<true>(node_ptr_type);
    template banana_tree<1>::node_ptr_type banana_tree<1>::turn_node_into_hook<false>(node_ptr_type);
    template banana_tree<-1>::node_ptr_type banana_tree<-1>::turn_node_into_hook<true>(node_ptr_type);
    template banana_tree<-1>::node_ptr_type banana_tree<-1>::turn_node_into_hook<false>(node_ptr_type);
}



void persistence_data_structure::glue_to_right(persistence_data_structure &right_persistence,
                                               min_dictionary &min_dict,
                                               max_dictionary &max_dict) {
    up_tree.glue_to_right(right_persistence.up_tree, min_dict);
    down_tree.glue_to_right(right_persistence.down_tree, max_dict);
}

persistence_data_structure persistence_data_structure::cut(list_item& left_of_cut,
                                                           list_item& right_of_cut,
                                                           min_dictionary &min_dict,
                                                           max_dictionary &max_dict) {
    persistence_data_structure other_persistence(up_tree.node_pool, down_tree.node_pool);
    
    list_item cut_item{(left_of_cut.get_interval_order() + right_of_cut.get_interval_order()) / 2,
                       (left_of_cut.value<1>() + right_of_cut.value<1>()) / 2};

    massert((left_of_cut.is_minimum<1>() && right_of_cut.is_maximum<1>()) ||
            (left_of_cut.is_maximum<1>() && right_of_cut.is_minimum<1>()),
            "Need a minimum and a maximum on either side of the cut.");
    massert(left_of_cut.right_neighbor() == &right_of_cut, "Expected `left_of_cut` and `right_of_cut` to be neighbors");
    
    if (left_of_cut.is_minimum<1>()) {
        anticancel(min_dict, max_dict, {&left_of_cut, &right_of_cut});
    } else {
        anticancel(min_dict, max_dict, {&right_of_cut, &left_of_cut});
    }
    auto smallest_up_banana = left_of_cut.is_minimum<1>()
            ? internal::item_pair<1>{&left_of_cut, &right_of_cut}
            : internal::item_pair<1>{&right_of_cut, &left_of_cut};
    auto smallest_dn_banana = left_of_cut.is_minimum<1>()
            ? internal::item_pair<-1>{&right_of_cut, &left_of_cut}
            : internal::item_pair<-1>{&left_of_cut, &right_of_cut};

    left_of_cut.cut_right();

    internal::banana_stack<1> Lup_stack, Mup_stack, Rup_stack;
    internal::banana_stack<-1> Ldn_stack, Mdn_stack, Rdn_stack;
    up_tree.load_stacks(cut_item, smallest_up_banana, Lup_stack, Mup_stack, Rup_stack);
    down_tree.load_stacks(cut_item, smallest_dn_banana, Ldn_stack, Mdn_stack, Rdn_stack);
    const bool up_cuts_left = up_tree.cut(cut_item, left_of_cut, right_of_cut,
                                          other_persistence.up_tree,
                                          Lup_stack, Mup_stack, Rup_stack, Ldn_stack, Rdn_stack);
    Lup_stack.reset_top(); Mup_stack.reset_top(); Rup_stack.reset_top();
    Ldn_stack.reset_top(); Mdn_stack.reset_top(); Rdn_stack.reset_top();
    const bool down_cuts_left = down_tree.cut(cut_item, left_of_cut, right_of_cut,
                                              other_persistence.down_tree,
                                              Ldn_stack, Mdn_stack, Rdn_stack, Lup_stack, Rup_stack);
    if (up_cuts_left != down_cuts_left) {
        swap(down_tree, other_persistence.down_tree);
    }

    return other_persistence;
}
