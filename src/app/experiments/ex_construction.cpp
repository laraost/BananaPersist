#include <chrono>
#include <iostream>
#include <utility>

#include "CLI11.hpp"

#include "gudhi/Persistence_on_a_line.h"
#include "persistence1d/persistence1d.hpp"

#include "app/experiments/utility/cli_options.h"
#include "app/experiments/utility/data_generation.h"
#include "datastructure/persistence_context.h"
#include "persistence_defs.h"
#include "utility/format_util.h"
#include "utility/random.h"
#include "utility/timer.h"

using namespace bananas;

std::ofstream output_file;

template<typename Generator>
void construct_experiment(size_t num_items,
                          size_t num_reps,
                          const typename Generator::parameters& generator_params,
                          bool run_gudhi,
                          bool run_persistence1d) {
    std::vector<function_value_type> values;
    csv_writer writer;
    multirow_csv_writer structure_writer;

    for (size_t rep = 0; rep < num_reps; ++ rep) {
        std::cout << "> rep " << rep << "\n";
        writer << std::make_pair("num_items", num_items);

        values.clear();
        Generator generator{generator_params};
        generator(values, num_items);
        generator.write_parameters(writer);

        Timer<std::chrono::nanoseconds> timer;

        persistence_context context;
        timer.restart();
        auto* const the_interval = context.new_interval(values);
        auto construction_time_banana = timer.elapsed();

        writer << std::make_pair("time", construction_time_banana);

        if (run_gudhi) {
            timer.restart();
            Gudhi::persistent_cohomology
                 ::compute_persistence_of_function_on_line(values,
                                                           [](function_value_type, function_value_type) {});
            auto construction_time_gudhi = timer.elapsed();
            writer << std::make_pair("time_gudhi", construction_time_gudhi);
        }
        if (run_persistence1d) {
            p1d::Persistence1D p1d;
            timer.restart();
            p1d.RunPersistence(values);
            auto construction_time_p1d = timer.elapsed();
            writer << std::make_pair("time_p1d", construction_time_p1d);
        }
        context.print_memory_stats(writer);

        writer << std::make_pair("global_max_pos", context.get_global_max_order(the_interval))
               << std::make_pair("global_max_value", context.get_global_max_value(the_interval))
               << std::make_pair("global_min_pos", context.get_global_min_order(the_interval))
               << std::make_pair("global_min_value", context.get_global_min_value(the_interval));

        writer.write_to_stream_and_reset(std::cout);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(rep) + "-" + generator_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, rep == 0);
        }
    }
}

int main(int argc, char** argv) {

    unsigned long seed = random_seed();
    size_t num_reps = 1;
    std::array<size_t, 3> num_item_limits;
    bool run_gudhi = false;
    bool run_persistence1d = false;
    std::string generator_args = "rw:0";
    std::string output_file_name;

    CLI::App app("Construction Experiments");

    add_seed_option(app, seed);
    add_num_reps_option(app, num_reps);
    add_num_items_option(app, num_item_limits)
        ->required();
    add_gen_args_option(app, generator_args);
    add_gudhi_flag(app, run_gudhi);
    add_persistence1d_flag(app, run_persistence1d);
    add_output_file_option(app, output_file_name);

    CLI11_PARSE(app, argc, argv);

    if (num_item_limits[0] < 2 || num_item_limits[1] == 0 || num_item_limits[2] < num_item_limits[0]) {
        std::cerr << "num_items needs to be of the form min step max, with min >= 2, step >= 1 and max > min.\n";
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

    const auto min_num_items = num_item_limits[0];
    const auto step_num_items = num_item_limits[1];
    const auto max_num_items = num_item_limits[2];
    random_number_generator rng{seed};

    const auto [gen_name, gen_param_string] = split_generator_args(generator_args);

    std::cout << "# Using generator " << gen_name << " with parameters " << gen_param_string << "\n";

    std::cout << "# Constructing a random walk.\n";
    for (auto num_items = min_num_items; num_items <= max_num_items; num_items += step_num_items) {
        if (gen_name == random_walk_generator<>::get_name()) {
            construct_experiment<random_walk_generator<decltype(rng)>>(num_items, num_reps, {rng, gen_param_string}, run_gudhi, run_persistence1d);
        } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
            construct_experiment<gaussian_random_walk_generator<decltype(rng)>>(num_items, num_reps, {rng, gen_param_string}, run_gudhi, run_persistence1d);
        } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
            construct_experiment<sum_quasi_periodic_generator<decltype(rng)>>(num_items, num_reps, {rng, gen_param_string}, run_gudhi, run_persistence1d);
        } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
            construct_experiment<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, num_reps, {rng, gen_param_string}, run_gudhi, run_persistence1d);
        }
        std::cout << "--\n";
    }

    return 0;

}
