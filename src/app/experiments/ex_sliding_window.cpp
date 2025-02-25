#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>

#include "CLI11.hpp"

#include "app/experiments/utility/cli_options.h"
#include "app/experiments/utility/cli_validators.h"
#include "app/experiments/utility/data_generation.h"
#include "datastructure/persistence_context.h"
#include "datastructure/persistence_diagram.h"
#include "persistence_defs.h"
#include "utility/random.h"
#include "utility/stats.h"
#include "utility/timer.h"

std::ofstream output_file;

#ifdef SLIDING_WINDOW_LOCAL
    #include "app/experiments/sliding_window_local.h"
#elif defined SLIDING_WINDOW_TOPOLOGICAL
    #include "app/experiments/sliding_window_topological.h"
#endif

int main(int argc, char** argv) {

    unsigned long seed = random_seed();
    size_t num_slides = 10;
    std::array<size_t, 3> window_size_limits;
    std::array<size_t, 3> step_size_limits = default_window_step;
    bool run_gudhi = false;
    bool run_persistence1d = false;
    std::string generator_args = "rw:0";
    std::string output_file_name;

    CLI::App app("Sliding Window Experiments");

    add_seed_option(app, seed);
    add_gudhi_flag(app, run_gudhi);
    add_persistence1d_flag(app, run_persistence1d);
    add_gen_args_option(app, generator_args);
    add_output_file_option(app, output_file_name);
    app.add_option("-n,--num_slides",
                   num_slides,
                   "Number of slides")
        ->check(CLI::PositiveNumber);
    app.add_option("-w,--window-step",
                   step_size_limits,
                   "How many items to advance with each slide");
    app.add_option("window_size",
                   window_size_limits,
                   "Size of the sliding window")
       ->required();

    CLI11_PARSE(app, argc, argv);

    const auto min_window_size = window_size_limits[0];
    const auto step_window_size = window_size_limits[1];
    const auto max_window_size = window_size_limits[2];
    if (min_window_size < 2 || step_window_size == 0 || max_window_size < min_window_size) {
        std::cerr << "window_size needs to be of the form min step max, with min >= 2, step >= 1 and max >= min.\n";
        std::cerr << app.help() << std::endl;
        return 1;
    }
    const auto min_step_size = step_size_limits[0];
    const auto step_step_size = step_size_limits[1];
    const auto max_step_size = step_size_limits[2];
    if (min_step_size < min_allowed_step_size || step_step_size == 0 || max_step_size < min_step_size) {
        std::cerr << "step_size needs to be of the form min step max, with min >= 2, step >= 1 and max >= min.\n";
        std::cerr << app.help() << std::endl;
        return 1;
    }

    if (output_file_name != "") {
        output_file.open(output_file_name);
        if (!output_file.is_open()) {
            std::cerr << "Failed to open " << output_file_name << "\n";
            return 1;
        }
    }

    random_number_generator rng{seed};

    const auto [gen_name, gen_param_string] = split_generator_args(generator_args);

    std::cout << "# Sliding window via " <<
        #ifdef SLIDING_WINDOW_LOCAL
            " local operations"
        #elif defined SLIDING_WINDOW_TOPOLOGICAL
            " topological operations"
        #endif
        << "\n";
    for (auto window_size = min_window_size; window_size <= max_window_size; window_size += step_window_size) {
        for (auto step_size = min_step_size; step_size <= max_step_size; step_size += step_step_size) {
            if (gen_name == random_walk_generator<>::get_name()) {
                sliding_window<random_walk_generator<decltype(rng)>>(num_slides, window_size, step_size, {rng, gen_param_string}, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                sliding_window<gaussian_random_walk_generator<decltype(rng)>>(num_slides, window_size, step_size, {rng, gen_param_string}, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                sliding_window<sum_quasi_periodic_generator<decltype(rng)>>(num_slides, window_size, step_size, {rng, gen_param_string}, run_gudhi, run_persistence1d);
            }
        }
    }

    return 0;
}
