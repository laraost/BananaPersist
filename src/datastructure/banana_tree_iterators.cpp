#include "datastructure/banana_tree.h"
#include "datastructure/banana_tree_sign_template.h"
#include "persistence_defs.h"
#include "utility/errors.h"

using namespace bananas;

//
// Implementation of `walk_iterator`
//

template<typename N>
walk_iterator<N>::walk_iterator(node_ptr_type at_node) : banana_stack(), current_node(at_node) {
}

template<typename N>
walk_iterator<N>::walk_iterator(const value_type &initial_banana) :
        current_node(initial_banana.first) {
    banana_stack.push_back(initial_banana);
}

template<typename N>
walk_iterator<N>& walk_iterator<N>::operator++() {
    massert(!banana_stack.empty(), "Attempting to increment an invalid iterator.");
    auto* birth = banana_stack.back().first;
    auto* death = banana_stack.back().second;

    // if current_node == birth and birth->in != death, set current_node to birth->in, push and return
    // if current_node == death->in, then set current_node to birth->mid
    // else set current_node to current_node->up
    if (current_node == birth && current_node != death->get_in()) {
        current_node = birth->in;
        banana_stack.push_back({current_node->get_birth(), current_node});
        current_node = current_node->get_birth();
        return *this;
    }
    if (current_node == death->get_in()) {
        current_node = birth->mid;
    } else {
        current_node = current_node->up;
    }

    // if current_node == death then pop, this->operator++ and return
    // else push and return
    if (current_node == death) {
        banana_stack.pop_back();
        if (!banana_stack.empty()) {
            ++(*this);
        }
    } else {
        banana_stack.push_back({current_node->get_birth(), current_node});
        current_node = current_node->get_birth();
    }

    return *this;
}

template<typename N>
walk_iterator<N>::reference walk_iterator<N>::operator*() {
    return banana_stack.back();
}

template<typename N>
walk_iterator<N>::pointer walk_iterator<N>::operator->() {
    return &(banana_stack.back());
}

SIGN_TEMPLATE
walk_iterator<banana_tree_node<sign>> banana_tree<sign>::walk_iterator_pair::begin() {
    return walk_iterator<node_type>({initial_max->get_birth(), initial_max});
}

SIGN_TEMPLATE
walk_iterator<banana_tree_node<sign>> banana_tree<sign>::walk_iterator_pair::end() {
    return walk_iterator<node_type>(initial_max);
}

SIGN_TEMPLATE
banana_tree<sign>::walk_iterator_pair::walk_iterator_pair(node_ptr_type initial_max) :
        initial_max(initial_max) {}

template<typename node_type>
bool bananas::operator!=(const walk_iterator<node_type> &a,
                         const walk_iterator<node_type> &b) {
    return a.current_node != b.current_node || a.banana_stack.size() != b.banana_stack.size();
}


//
// Implementation of `string_iterator`
//

template<typename N>
string_iterator<N>::string_iterator(const_node_ptr_type previous_node,
                                    const_node_ptr_type start) : previous_node(previous_node),
                                                                 current_node(start) {}

template<typename N>
string_iterator<N>& string_iterator<N>::operator++() {
//    massert(!(current_node->get_birth() == current_node->get_low() &&
//                previous_node == current_node->get_mid()),
//            "Attempting to iterate from the special root");
    if (current_node->is_special_root()) {
        // Don't move from the special root
        if (previous_node == current_node->get_mid()) {
            return *this;
        }
        // `current_node` is the special root.
        // `previous_node != current_node->get_mid()` by the above assertion.
        previous_node = current_node;
        current_node = current_node->get_in();
    }
    if (current_node->is_leaf()) {
        if (previous_node == current_node->get_in()) {
            previous_node = current_node;
            current_node = current_node->get_mid();
        } else {
            previous_node = current_node;
            current_node = current_node->get_in();
        }
    } else if (previous_node == current_node->get_mid() && previous_node != current_node->get_in()) {
        // Coming from the mid-panel 
        previous_node = current_node;
        current_node = current_node->get_down();
    } else if (previous_node == current_node->get_mid() && previous_node == current_node->get_in()) {
        // Empty banana
        if (*previous_node->get_item() < *current_node->get_item()) {
            previous_node = current_node;
            current_node = current_node->get_down();
        } else {
            previous_node = current_node;
            current_node = current_node->get_up();
        }
    } else if (previous_node == current_node->get_in() && previous_node != current_node->get_mid()) {
        // We've seen the mid-trail before the in-trail, so `current_node`'s trail goes upwards.
        previous_node = current_node;
        current_node = current_node->get_up();
    } else if (previous_node == current_node->get_down()) {
        // We're walking up, so we continue on the mid-trail
        previous_node = current_node;
        current_node = current_node->get_mid();
    } else if (previous_node == current_node->get_up()) {
        previous_node = current_node;
        current_node = current_node->get_in();
    } 
    // Skip the first occurence of an internal node if its banana lies on the left
    auto the_birth = current_node->get_birth()->get_item();
    if (*previous_node->get_item() < *the_birth && *the_birth < *current_node->get_item()) {
        massert(current_node->get_low() != current_node, "Expected an internal node, but found a leaf.");
        this->operator++();
        return *this;
    }
    // Skip internal nodes when returning upwards and their banana lies on the right.
    if (current_node->get_low() != current_node && *previous_node->get_item() > *current_node->get_item()) {
        this->operator++();
        return *this;
    }

    return *this;
}

template<typename N>
string_iterator<N>::reference string_iterator<N>::operator*() {
    return current_node;
}

template<typename N>
string_iterator<N>::pointer string_iterator<N>::operator->() {
    return &current_node;
}

SIGN_TEMPLATE
banana_tree<sign>::string_iterator_pair::string_iterator_pair(node_ptr_type left_node,
                                                              node_ptr_type special_root) :
        left_node(left_node),
        special_root(special_root) {
    massert(special_root->get_birth() == special_root->get_low(),
            "Expected string-iteration to begin with a special root.");
}

SIGN_TEMPLATE
string_iterator<banana_tree_node<sign>> banana_tree<sign>::string_iterator_pair::begin() {
    return {left_node->get_in(), left_node};
}

SIGN_TEMPLATE
string_iterator<banana_tree_node<sign>> banana_tree<sign>::string_iterator_pair::end() {
    return {special_root->get_mid(), special_root};
}

template<typename node_type>
bool bananas::operator==(const string_iterator<node_type> &a, const string_iterator<node_type> &b) {
    return a.previous_node == b.previous_node && a.current_node == b.current_node;
}

template<typename node_type>
bool bananas::operator!=(const string_iterator<node_type> &a, const string_iterator<node_type> &b) {
    return !(a == b);
}



// `banana_tree` instantiation
namespace bananas {
    template class banana_tree<1>;
    template class banana_tree<-1>;

    template class walk_iterator<up_tree_node>;
    template class walk_iterator<down_tree_node>;

    template class string_iterator<up_tree_node>;
    template class string_iterator<down_tree_node>;

    template bool operator!=(const walk_iterator<up_tree_node> &a,
                             const walk_iterator<up_tree_node> &b);
    template bool operator!=(const walk_iterator<down_tree_node> &a,
                             const walk_iterator<down_tree_node> &b);

    template bool operator==(const string_iterator<up_tree_node> &a,
                             const string_iterator<up_tree_node> &b);
    template bool operator==(const string_iterator<down_tree_node> &a,
                             const string_iterator<down_tree_node> &b);
    template bool operator!=(const string_iterator<up_tree_node> &a,
                             const string_iterator<up_tree_node> &b);
    template bool operator!=(const string_iterator<down_tree_node> &a,
                             const string_iterator<down_tree_node> &b);
}
