#pragma once

#include <boost/intrusive/avl_set_hook.hpp>
#include <boost/intrusive/bs_set_hook.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <cmath>
#include <limits>
#include <type_traits>

using interval_order_type = double;
using function_value_type = double;

using interval_id_t = int;

template<typename T, T value>
concept sign_integral = (std::is_integral_v<T> &&
                        (value == 1 || value == -1));

template<typename T>
inline T next_larger(T t) {
    return std::nextafter(t, std::numeric_limits<T>::infinity());
}

template<typename T>
inline T next_smaller(T t) {
    return std::nextafter(t, -std::numeric_limits<T>::infinity());
}

// Get the value closest to `t` in the direction of `sign`.
template<int sign>
    requires sign_integral<decltype(sign), sign>
inline function_value_type add_tiniest_offset(function_value_type t) {
    if constexpr (sign == 1) {
        return next_larger(t);
    } else {
        return next_smaller(t);
    }
}

template<typename T>
struct min_max_pair {
    T min;
    T max;
};

// Definitions related to search trees and other datastructures

// The hook for intrusive `set`-type data structures, i.e., for insertion into the dictionaries.
// There is a clash of nomenclature here: the data structures we call dictionaries in the paper
// are called sets in Boost and the STL.
// TODO: configure `link_mode` based on build type: safe for debug; normal for release
#ifdef AVL_SEARCH_TREE
    using default_set_member_hook_type = boost::intrusive::avl_set_member_hook<>;
#elif defined SPLAY_SEARCH_TREE
    using default_set_member_hook_type = boost::intrusive::bs_set_member_hook<>;
#endif
