//
// This file implements the construction algorithm for the banana tree.
//
#include "datastructure/banana_tree.h"
#include "datastructure/banana_tree_sign_template.h"
#include "persistence_defs.h"
#include "utility/errors.h"

using namespace bananas;

template<int sign>
    requires sign_integral<decltype(sign), sign>
void banana_tree<sign>::construct_impl(list_item *left_endpoint,
                                       list_item *right_endpoint) {
    massert(left_endpoint->right_neighbor() != nullptr, "Need at least two items to construct a banana tree");

    TIME_BEGIN(construct_prepare);

    // Extract critical items from the list of all items
    auto construction_item_pool = recycling_object_pool<construction_item>{};
    auto prev_item = construction_item_pool.construct(nullptr, nullptr, left_endpoint);
    this->allocate_node(left_endpoint);
    auto left_c_endpoint = prev_item;
    auto right_c_endpoint = prev_item;
    // Begin with the assumption that the left endpoint is the global maximum. We will update this in the loop.
    global_max = left_endpoint;
    for (auto *begin = left_endpoint->right_neighbor(); begin != nullptr; begin = begin->right_neighbor()) {
        if (begin->is_endpoint() || begin->is_critical<sign>()) {
            if (begin->is_maximum<sign>() || begin->is_down_type<sign>()) {
                if (begin->value<sign>() > global_max->value<sign>()) {
                    global_max = begin;
                }
            }

            auto new_item = construction_item_pool.construct(prev_item, nullptr, begin);
            prev_item->next = new_item;
            prev_item = new_item;
            right_c_endpoint = new_item;

            this->allocate_node(new_item->stored_item);
        }
    }
    massert(global_max != nullptr, "Expected to find a global maximum during construction.");
    // Add hooks if necessary
    bool added_left_hook = false;
    bool added_right_hook = false;
    if (left_endpoint->is_down_type<sign>()) {
        auto hook_left = construction_item_pool.construct(nullptr, left_c_endpoint, &this->left_hook_item);
        list_item::link(this->left_hook_item, *left_endpoint);
        left_c_endpoint->prev = hook_left;
        left_c_endpoint = hook_left;

        this->allocate_node(hook_left->stored_item);
        added_left_hook = true;
    }
    if (right_endpoint->is_down_type<sign>()) {
        auto hook_right = construction_item_pool.construct(right_c_endpoint, nullptr, &this->right_hook_item);
        list_item::link(*right_endpoint, this->right_hook_item);
        right_c_endpoint->next = hook_right;
        right_c_endpoint = hook_right;

        this->allocate_node(hook_right->stored_item);
        added_right_hook = true;
    }
    // Add the additional "fake" item on the left (which ensures that the stack never empties)
    auto fake_left_item = list_item{sign*std::numeric_limits<function_value_type>::infinity()};
    list_item::link(fake_left_item, *(left_c_endpoint->stored_item));
    auto fake_left = construction_item{nullptr, left_c_endpoint, &fake_left_item};

    // Add the item on the right (which becomes the special root)
    // The fake item on the right end of the interval becomes the tree's special root
    auto fake_right = construction_item{right_c_endpoint, nullptr, &this->special_root_item};
    list_item::link(*(right_c_endpoint->stored_item), this->special_root_item);
    right_c_endpoint->next = &fake_right;
    right_c_endpoint = &fake_right;
    this->allocate_node(fake_left.stored_item);
    this->allocate_node(fake_right.stored_item);

    TIME_END(construct_prepare, sign);
    TIME_BEGIN(construct_loop);

    using stack_pair = min_max_pair<construction_item*>;
    std::vector<stack_pair> the_stack;
    the_stack.push_back({&fake_left, &fake_left});

    // Make the `down`-pointer of the fake left node point somewhere
    fake_left.stored_item->template get_node<sign>()->down =
        fake_left.next->stored_item->template get_node<sign>();

    construction_item* A = nullptr; // We initialize this to be null; see the paper for why it will be initialized in time.
    for (auto j = left_c_endpoint; j != nullptr; j = j->next) {
        if (j->stored_item->template is_minimum<sign>()) {
            A = j;
        } else if (j->stored_item->template is_maximum<sign>() ||
                   j->stored_item->template is_down_type<sign>()) {
            construction_item *b = nullptr;
            while (j->get_value() > the_stack.back().max->get_value()) {
                auto top = the_stack.back();
                construction_item* a = top.min;
                b = top.max;
                the_stack.pop_back();
                if (A->get_value() < a->get_value()) {
                    fix_banana(a, b);
                } else {
                    attach_below_on_right(b, j);
                    fix_banana(A, b);
                    A = a;
                }
            }
            b = the_stack.back().max;
            attach_below_on_left(j, b);
            the_stack.push_back({A, j});
            if (j == right_c_endpoint) {
                fix_banana(A, j);
            }
        }
    }

    TIME_END(construct_loop, sign);
    TIME_BEGIN(construct_cleanup);

    // Remove the fake left item and its node
    free_node(&fake_left_item);
    fake_left_item.cut_right();
    // remove the special root item from the linked list...
    special_root_item.cut_left();
    // ...and clean up the pointers of its associated node
    auto special_root = special_root_item.get_node<sign>();
    special_root->up = nullptr;
    special_root->down = nullptr;
    // The special root's `low`-pointer is set to its birth, such that the special banana can be identified.
    special_root->low = special_root->get_birth();
    // remove left hook and right hook from the list, if they were added
    if (added_left_hook) {
        left_hook_item.cut_right();
    }
    if (added_right_hook) {
        right_hook_item.cut_left();
    }

    TIME_END(construct_cleanup, sign);
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
void banana_tree<sign>::attach_below_on_left(construction_item* j,
                                             construction_item* b) {
    // `banana_tree_node`s storing the items `j` and `b`
    auto J = j->stored_item->template get_node<sign>();
    auto B = b->stored_item->template get_node<sign>();

    J->up = B;
    J->in = B->down;
    J->mid = j->prev->stored_item->template get_node<sign>();
    if (j->next != nullptr) {
        J->down = j->next->stored_item->template get_node<sign>();
    }
    B->down = J;
    J->in->up = J;
    J->mid->up = J;
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
void banana_tree<sign>::attach_below_on_right(construction_item* b,
                                              construction_item* j) {
    // `banana_tree_node`s storing the items `j` and `b`
    auto J = j->stored_item->template get_node<sign>();
    auto B = b->stored_item->template get_node<sign>();
    
    B->up->down = B->in;
    B->in->up = B->up;
    B->up = J;
    B->in = j->prev->stored_item->template get_node<sign>();
    auto aux = B->down; //
    B->down = B->mid;   // Swap B->down and B->mid
    B->mid = aux;       //
    B->in->up = B;
    j->prev = b;
}

template<int sign>
    requires sign_integral<decltype(sign), sign>
void banana_tree<sign>::fix_banana(construction_item* a,
                                   construction_item* b) {
    // `banana_tree_node`s storing the items `a` and `b` 
    auto A = a->stored_item->template get_node<sign>();
    auto B = b->stored_item->template get_node<sign>();

    auto Q = B;
    auto P = B->in;
    while(P != A) {
        P->low = A;
        Q = P;
        P = P->down;
    }
    A->in = Q;

    Q = B;
    P = B->mid;
    while (P != A) {
        P->low = A;
        Q = P;
        P = P->down;
    }
    A->mid = Q;

    A->low = A;
    A->death = B;
    A->up = nullptr;
    A->down = nullptr;
}

SIGN_TEMPLATE
void banana_tree<sign>::initialize_spline_labels() {
    auto* special_root_node = special_root_item.get_node<sign>();
    special_root_node->spine_label = internal::spine_pos::on_both_spines;
    auto node_left = special_root_node->in;
    while (true) {
        node_left->spine_label = internal::spine_pos::on_left_spine;
        if (node_left->is_leaf()) {
            break;
        }
        node_left = node_left->in;
    }
    auto node_right = special_root_node->mid;
    massert(list_item::is_between(*node_right->item, *node_left->item, special_root_item),
            "Expected `in` of the special root to be on the left spine and `mid` to be on the right spine.");
    while (true) {
        node_right->spine_label = internal::spine_pos::on_right_spine;
        if (node_right->is_leaf()) {
            break;
        }
        node_right = node_right->in;
    }
}


// `banana_tree` instantiation
namespace bananas {
    template class banana_tree<1>;
    template class banana_tree<-1>;
}
