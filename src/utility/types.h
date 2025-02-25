#pragma once

#include <functional>
#include <optional>

#include <boost/core/demangle.hpp>

template<typename T>
std::string type_to_string() {
    return boost::core::demangle(typeid(T).name());
}

template<typename T>
using optional_vector_ref = std::optional<std::reference_wrapper<std::vector<T>>>;
