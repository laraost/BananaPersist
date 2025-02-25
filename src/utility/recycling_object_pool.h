#pragma once

#include <cassert>
#include <type_traits>
#include <vector>

#include "boost/pool/object_pool.hpp"
#include "utility/debug.h"
#include "utility/types.h"

namespace bananas {

// An object pool that recycles its objects instead of deallocating.
// This is a wrapper around `boost::object_pool` to avoid the expensive deallocation
// that happens when `free` is called.
//
// `T` is the type of objects allocated from this pool.
// `UserAllocator` is the allocator used by the `boost::object_pool`.
template<typename T, typename UserAllocator = boost::default_user_allocator_new_delete>
class recycling_object_pool {
    using object_type = T;
    using pointer_type = object_type*;
    using reference_type = object_type&;
    using user_allocator_type = UserAllocator;

public:
    recycling_object_pool(const size_t arg_next_size = 32, const size_t arg_max_size = 0) :
        pool(arg_next_size, arg_max_size) {}

    // Construct an instance of type `T` with `Args` passed to the constructor.
    // Returns a pointer to the newly constructed instance.
    template<typename... Args>
    pointer_type construct(Args &&...args) {
        auto size_before = pool.get_next_size();
        if (free_objects.empty()) {
            auto* result = pool.construct(args...);
            // Note: counting allocations this way fails if the max size is reached, since then next_size does not change.
            number_of_allocations += static_cast<int>(size_before != pool.get_next_size());
            if (size_before != pool.get_next_size()) {
                DEBUG_MSG("New allocation in memory pool for " << type_to_string<T>() << ".");
            }
            return result;
        } 
        // If there is a free object lying around, recycle it.
        number_of_recyclings++;
        auto result = free_objects.back();
        free_objects.pop_back();
        return new(result) object_type{args...};
    }

    // Construct an instance of `T` via move construction.
    pointer_type construct(T &&temp) {
        auto size_before = pool.get_next_size();
        if (free_objects.empty()) {
            auto* result = pool.malloc();
            result = new(result) object_type(std::move(temp));
            // Note: counting allocations this way fails if the max size is reached, since then next_size does not change.
            number_of_allocations += static_cast<int>(size_before != pool.get_next_size());
            if (size_before != pool.get_next_size()) {
                DEBUG_MSG("New allocation in memory pool for " << type_to_string<T>() << ".");
            }
            return result;
        } 
        // If there is a free object lying around, recycle it.
        number_of_recyclings++;
        auto result = free_objects.back();
        free_objects.pop_back();
        return new(result) object_type{std::move(temp)};
    }

    // "Frees" the object at `ptr`.
    // Calls the destructor if `T` is not trivially destructible.
    void free(pointer_type ptr) {
        assert(ptr != nullptr); // Can't free something that doesn't exist
        // If the object has to be destructed, do that now.
        if constexpr (!std::is_trivially_destructible_v<T>) {
            ptr->~T();
        }
        free_objects.push_back(ptr);
    }

    int get_number_of_allocations() const {
        return number_of_allocations;
    }

    long get_number_of_recyclings() const {
        return number_of_recyclings;
    }

private:
    boost::object_pool<object_type, user_allocator_type> pool;
    std::vector<pointer_type> free_objects;
    int number_of_allocations = 0;
    long number_of_recyclings = 0;

};

} // End of namespace `bananas`
