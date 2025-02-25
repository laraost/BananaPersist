#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <ranges>
#include <utility>

#include "CLI11.hpp"

#include "app/experiments/utility/cli_options.h"
#include "app/experiments/utility/cli_validators.h"
#include "app/experiments/utility/data_generation.h"
#include "datastructure/persistence_context.h"
#include "datastructure/persistence_diagram.h"
#include "gudhi/Persistence_on_a_line.h"
#include "persistence1d/persistence1d.hpp"
#include "persistence_defs.h"
#include "utility/format_util.h"
#include "utility/random.h"
#include "utility/stats.h"
#include "utility/timer.h"

using namespace bananas;

std::ofstream output_file;

template<typename Generator>
void cut_experiment(size_t num_items,
                    double cut_fraction,
                    const typename Generator::parameters& gen_params,
                    unsigned int num_reps,
                    bool run_gudhi,
                    bool run_persistence1d) {
    const size_t cut_index = static_cast<size_t>(cut_fraction * num_items) - 1;

    std::vector<function_value_type> values;
    using iter_t = std::vector<function_value_type>::iterator;
    std::vector<list_item*> item_ptrs;

    Timer<std::chrono::nanoseconds> timer;
    csv_writer writer;
    multirow_csv_writer structure_writer;

    for (unsigned int rep = 0; rep < num_reps; ++rep) {
        std::cout << "> rep " << rep << "\n";

        writer << std::make_pair("num_items", num_items)
               << std::make_pair("cut_fraction", cut_fraction)
               << std::make_pair("cut_index", cut_index);

        values.clear();
        Generator generator{gen_params};
        generator(values, num_items);
        generator.write_parameters(writer);
        
        persistence_diagram pd_before, pd_after;

        persistence_context context;
        item_ptrs.clear();
        auto* const the_interval = context.new_interval(values, {std::ref(item_ptrs)});

        const auto global_max_order = context.get_global_max_order(the_interval);
        const auto global_min_order = context.get_global_min_order(the_interval);
        const auto global_max_value = context.get_global_max_value(the_interval);
        const auto global_min_value = context.get_global_min_value(the_interval);

        context.compute_persistence_diagram(pd_before);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                  "." + std::to_string(rep) +
                                                                  ".pre-" + gen_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, rep == 0);
        }

        persistence_stats.reset();
        dictionary_stats.reset();
        timer.restart();
        context.cut_interval(the_interval, item_ptrs[cut_index]);
        const auto cut_time = timer.elapsed();

        context.compute_persistence_diagram(pd_after);
        const auto pd_diff = persistence_diagram::symmetric_difference(pd_before, pd_after);

        writer << std::make_pair("time", cut_time)
               << std::make_pair("diff_points", pd_diff.points)
               << std::make_pair("diff_arrows", pd_diff.arrows)
               << std::make_pair("global_max_pos", global_max_order)
               << std::make_pair("global_min_pos", global_min_order)
               << std::make_pair("global_max_value", global_max_value)
               << std::make_pair("global_min_value", global_min_value);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                  "." + std::to_string(rep) +
                                                                  ".post-" + gen_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, false);
        }

        if (run_gudhi) {
            timer.restart();
            Gudhi::persistent_cohomology::compute_persistence_of_function_on_line(std::ranges::subrange<iter_t>(values.begin(), values.begin() + cut_index + 1),
                                                                                  [] (auto, auto) {});
            auto gudhi_time_left = timer.elapsed();
            timer.restart();
            Gudhi::persistent_cohomology::compute_persistence_of_function_on_line(std::ranges::subrange<iter_t>(values.begin() + cut_index + 1, values.end()),
                                                                                  [] (auto, auto) {});
            auto gudhi_time_right = timer.elapsed();
            writer << std::make_pair("time_gudhi_left", gudhi_time_left)
                   << std::make_pair("time_gudhi_right", gudhi_time_right);
        }
        if (run_persistence1d) {
            p1d::Persistence1D p1d_left, p1d_right;
            auto left_range = std::ranges::subrange<iter_t>(values.begin(), values.begin() + cut_index + 1);
            auto right_range = std::ranges::subrange<iter_t>(values.begin() + cut_index + 1, values.end());
            std::vector<function_value_type> left_values{left_range.begin(), left_range.end()};
            std::vector<function_value_type> right_values{right_range.begin(), right_range.end()};

            timer.restart();
            p1d_left.RunPersistence(left_values);
            auto p1d_time_left = timer.elapsed();
            timer.restart();
            p1d_left.RunPersistence(right_values);
            auto p1d_time_right = timer.elapsed();

            writer << std::make_pair("time_p1d_left", p1d_time_left)
                   << std::make_pair("time_p1d_right", p1d_time_right);
        }

        persistence_stats.write_statistics<std::chrono::nanoseconds>(writer);
        dictionary_stats.write_statistics<std::chrono::nanoseconds>(writer);
        writer.write_to_stream_and_reset(std::cout);
    }
}

template<typename Generator>
void glue_experiment(size_t num_items,
                     double cut_fraction,
                     const typename Generator::parameters& gen_params,
                     unsigned int num_reps,
                     bool run_gudhi,
                     bool run_persistence1d) {
    const size_t cut_index = static_cast<size_t>(cut_fraction * num_items) - 1;

    std::vector<function_value_type> all_values;
    std::vector<function_value_type> values_left, values_right;
    std::vector<list_item*> item_ptrs_left, item_ptrs_right;

    Timer<std::chrono::nanoseconds> timer;
    csv_writer writer;
    multirow_csv_writer structure_writer;

    for (unsigned int rep = 0; rep < num_reps; ++rep) {
        std::cout << "> rep " << rep << "\n";

        writer << std::make_pair("num_items", num_items)
               << std::make_pair("cut_fraction", cut_fraction)
               << std::make_pair("cut_index", cut_index);

        values_left.clear();
        values_right.clear();
        Generator generator{gen_params};
        generator(all_values, num_items);
        generator.write_parameters(writer);
        values_left.insert(values_left.end(), all_values.begin(), all_values.begin() + cut_index + 1);
        values_right.insert(values_right.end(), all_values.begin() + cut_index + 1, all_values.end());
        
        persistence_diagram pd_before, pd_after;

        persistence_context context;
        item_ptrs_left.clear();
        item_ptrs_right.clear();
        auto* const the_left_interval = context.new_interval(values_left, {std::ref(item_ptrs_left)});
        auto* const the_right_interval = context.new_interval(values_right, {std::ref(item_ptrs_right)}, values_left.size());

        context.compute_persistence_diagram(pd_before);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                  "." + std::to_string(rep) +
                                                                  ".pre-" + gen_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, rep == 0);
        }

        persistence_stats.reset();
        dictionary_stats.reset();
        timer.restart();
        context.glue_intervals(the_left_interval, the_right_interval);
        const auto glue_time = timer.elapsed();

        context.compute_persistence_diagram(pd_after);
        const auto pd_diff = persistence_diagram::symmetric_difference(pd_before, pd_after);

        const auto global_max_order = context.get_global_max_order(the_left_interval);
        const auto global_min_order = context.get_global_min_order(the_left_interval);
        const auto global_max_value = context.get_global_max_value(the_left_interval);
        const auto global_min_value = context.get_global_min_value(the_left_interval);

        writer << std::make_pair("time", glue_time)
               << std::make_pair("diff_points", pd_diff.points)
               << std::make_pair("diff_arrows", pd_diff.arrows)
               << std::make_pair("global_max_pos", global_max_order)
               << std::make_pair("global_min_pos", global_min_order)
               << std::make_pair("global_max_value", global_max_value)
               << std::make_pair("global_min_value", global_min_value);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                  "." + std::to_string(rep) +
                                                                  ".post-" + gen_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, false);
        }

        if (run_gudhi) {
            timer.restart();
            Gudhi::persistent_cohomology::compute_persistence_of_function_on_line(all_values, [](auto, auto){});
            auto glue_time_gudhi = timer.elapsed();
            writer << std::make_pair("time_gudhi", glue_time_gudhi);
        }
        if (run_persistence1d) {
            p1d::Persistence1D p1d;
            timer.restart();
            p1d.RunPersistence(all_values);
            auto glue_time_p1d = timer.elapsed();
            writer << std::make_pair("time_p1d", glue_time_p1d);
        }

        persistence_stats.write_statistics<std::chrono::nanoseconds>(writer);
        dictionary_stats.write_statistics<std::chrono::nanoseconds>(writer);
        writer.write_to_stream_and_reset(std::cout);
    }
}



int main(int argc, char** argv) {

    unsigned long seed = random_seed();
    size_t num_reps = 1;
    std::array<size_t, 3> num_item_limits;
    double cut_fraction = 0.5;
    bool run_gudhi = false;
    bool run_persistence1d = false;
    std::string generator_args = "rw:0";
    std::string output_file_name;
    
    CLI::App app("Topological Maintenance Experiments");

    add_seed_option(app, seed);
    add_num_reps_option(app, num_reps);
    add_num_items_option(app, num_item_limits) -> required();
    add_gudhi_flag(app, run_gudhi);
    add_persistence1d_flag(app, run_persistence1d);
    auto* gen_opt = add_gen_args_option(app, generator_args);
    add_output_file_option(app, output_file_name);
    app.add_option("-c,--cut_fraction",
                   cut_fraction,
                   "Where to cut the interval.")
       ->check(open_interval(0.0, 1.0));

    auto* cut_app = app.add_subcommand("cut", "Cutting intervals");
    auto* glue_app = app.add_subcommand("glue", "Gluing intervals");
    auto* wc_cut_app = app.add_subcommand("wc-cut", "Linear-time case for cutting");
    auto* wc_glue_app = app.add_subcommand("wc-glue", "Linear-time case for gluing");

    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);

    const auto min_num_items = num_item_limits[0];
    const auto step_num_items = num_item_limits[1];
    const auto max_num_items = num_item_limits[2];
    if (min_num_items < 2 || step_num_items == 0 || max_num_items < min_num_items) {
        std::cerr << "num_items needs to be of the form min step max, with min >= 2, step >= 1 and max >= min.\n";
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

    auto logspace_items = logspace<size_t>(min_num_items, max_num_items, step_num_items);

    if (app.got_subcommand(cut_app)) {
        std::cout << "# Cutting a random walk.\n";
        for (auto num_items: logspace_items) {
            if (gen_name == random_walk_generator<>::get_name()) {
                cut_experiment<random_walk_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                cut_experiment<gaussian_random_walk_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                cut_experiment<sum_quasi_periodic_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
                cut_experiment<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            }
            std::cout << "--\n";
        }
    } else if (app.got_subcommand(glue_app)) {
        std::cout << "# Gluing two random walks.\n";
        for (auto num_items: logspace_items) {
            if (gen_name == random_walk_generator<>::get_name()) {
                glue_experiment<random_walk_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                glue_experiment<gaussian_random_walk_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                glue_experiment<sum_quasi_periodic_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
                glue_experiment<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, cut_fraction, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            }
            std::cout << "--\n";
        }
    } else if (app.got_subcommand(wc_cut_app)) {
        if (*gen_opt && gen_name != topological_worst_case_generator<false>::get_name()) {
            std::cerr << "wc-cut app requires cut-wc generator.\n";
            return 1;
        }
        if (min_num_items < 4) {
            std::cerr << "Need at least 4 items for topological worst case.\n";
            return 1;
        }
        std::cout << "# Linear-time case for cutting.\n";
        for (auto num_items: logspace_items) {
            // Need an odd number of items
            if (num_items % 2 == 0) {
                num_items++;
            }
            if (num_items % 4 != 1) {
                num_items += 2;
            }
            massert(num_items % 4 == 1, "Expected number of items to be a number divisible by 4 plus 1.");
            cut_experiment<topological_worst_case_generator<false, decltype(rng)>>(num_items, 0.5, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            std::cout << "--\n";
        }
    } else if (app.got_subcommand(wc_glue_app)) {
        if (*gen_opt && gen_name != topological_worst_case_generator<true>::get_name()) {
            std::cerr << "wc-glue app requires cut-wc generator.\n";
            return 1;
        }
        if (min_num_items < 4) {
            std::cerr << "Need at least 4 items for topological worst case.\n";
            return 1;
        }
        std::cout << "# Linear-time case for gluing.\n";
        for (auto num_items: logspace_items) {
            // Need an odd number of items
            if (num_items % 2 == 0) {
                num_items++;
            }
            if (num_items % 4 != 1) {
                num_items += 2;
            }
            massert(num_items % 4 == 1, "Expected number of items to be a number divisible by 4 plus 1.");
            glue_experiment<topological_worst_case_generator<true, decltype(rng)>>(num_items, 0.5, {rng, gen_param_string}, num_reps, run_gudhi, run_persistence1d);
            std::cout << "--\n";
        }
    }

    return 0;
}
