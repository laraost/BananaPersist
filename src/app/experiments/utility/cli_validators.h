#pragma once

#include "CLI11.hpp"
#include "app/experiments/utility/data_generation.h"
#include <string>

// Similar to CLI's built-in range validator, but for open intervals.
// This code is basically copied from the CLI11 code for the Range validator.
class open_interval : public CLI::Validator {
public:
    template<typename T>
    open_interval(T min_val, T max_val, const std::string& validator_name = std::string{}) {
        if (validator_name.empty()) {
            std::stringstream out;
            out << CLI::detail::type_name<T>() << " in (" << min_val << ", " << max_val << ")";
            description(out.str());
        }

        func_ = [min_val, max_val](std::string& input) {
            using CLI::detail::lexical_cast;
            T val;
            bool converted = lexical_cast(input, val);
            if (!converted || (val <= min_val || val >= max_val)) {
                std::stringstream out;
                out << "Value " << input << " not in interval (";
                out << min_val << ", " << max_val << ")";
                return out.str();
            }
            return std::string{};
        };
    }

    // Produce an open interval (0, max_val).
    template <typename T>
    explicit open_interval(T max_val, const std::string& validator_name = std::string{})
        : open_interval(static_cast<T>(0), max_val, validator_name) {}

};

class gen_param_string : public CLI::Validator {
public:
    gen_param_string(const std::string& validator_name = std::string{}) {
        if (validator_name.empty()) {
            std::stringstream out;
            out << "string in format 'generator-name:arguments";
            description(out.str());
        }

        func_ = [](std::string& input) {
            const auto pos = input.find(':');
            if (pos == std::string::npos) {
                std::stringstream out;
                out << "Invalid generator argument string '" <<  input << "' (no separating ':')";
                return out.str();
            }
            const auto gen_name = input.substr(0, pos);
            if (gen_name != bananas::random_walk_generator<>::get_name()) {
                std::stringstream out;
                out << "Unknown generator name '" << gen_name << "'";
                return out.str();
            }
            return std::string{};
        };
    }

};
