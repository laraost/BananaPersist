#pragma once

#include <array>
#include <cstddef>
#include <limits>

#include "CLI11.hpp"

inline CLI::Option* add_seed_option(CLI::App& app, unsigned long& seed) {
    return app.add_option("-s,--seed",
                          seed,
                          "The seed for the random number generator");
}

inline CLI::Option* add_num_reps_option(CLI::App& app, size_t& num_reps) {
    return app.add_option("-n,--num_reps",
                          num_reps,
                          "Number of repetitions")
        ->default_val(1)
        ->check(CLI::PositiveNumber);
}

inline CLI::Option* add_num_items_option(CLI::App& app, size_t& num_items) {
    return app.add_option("num_items",
                          num_items,
                          "Number of items")
        ->check(CLI::Range(size_t{2}, std::numeric_limits<size_t>::max()));
}

inline CLI::Option* add_num_items_option(CLI::App& app, std::array<size_t, 3>& num_item_limits) {
    return app.add_option("num_items",
                          num_item_limits,
                          "Number of items in the form 'min step max'");
}

inline CLI::Option* add_logspace_num_items_option(CLI::App& app, std::array<size_t, 3>& num_item_limits) {
    return app.add_option("num_items",
                          num_item_limits,
                          "Number of items in the form 'min number_of_steps max'; uniformly spaced on a logarithmic scale");
}

inline CLI::Option* add_gen_args_option(CLI::App& app, std::string& generator_args) {
    return app.add_option("--gen-args",
                          generator_args,
                          "The generator to use and its arguments in the format 'name:arguments'");
}

inline CLI::Option* add_gudhi_flag(CLI::App& app, bool& run_gudhi) {
    return app.add_flag("-g,--gudhi",
                        run_gudhi,
                        "Also measure the time to run Gudhi");
}

inline CLI::Option* add_persistence1d_flag(CLI::App& app, bool& run_persistence1d) {
    return app.add_flag("-p,--persistence1d",
                        run_persistence1d,
                        "Also measure the time to run Persistence1D");
}

inline CLI::Option* add_output_file_option(CLI::App& app, std::string& output_file) {
    return app.add_option("-o,--output",
                          output_file,
                          "Output file for structure information");
}
