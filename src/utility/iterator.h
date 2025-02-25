#pragma once

#include <iterator>
#include <type_traits>

namespace bananas {

template<typename Iter>
    requires std::is_pointer_v<typename Iter::value_type>
class pointer_range_adapter {
    using iter_t = Iter;

public:
    using value_type = std::remove_pointer_t<typename iter_t::value_type>;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;

    pointer_range_adapter(const Iter& it) : current_it(it) {}

    pointer_range_adapter& operator++() {
        current_it++;
        return *this;
    }
    pointer_range_adapter operator++(int) {
        auto next_it = current_it;
        next_it++;
        return {next_it};
    }
    reference operator*() {
        return **current_it;
    }
    pointer operator->() {
        return *current_it;
    }

private:
    Iter current_it;

    friend bool operator==(const pointer_range_adapter& a, const pointer_range_adapter& b) {
        return a.current_it == b.current_it;
    }
    friend bool operator!=(const pointer_range_adapter& a, const pointer_range_adapter& b) {
        return a.current_it != b.current_it;
    }

};

} // End of namespace `bananas`
