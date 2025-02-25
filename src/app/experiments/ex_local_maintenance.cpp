#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
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

struct random_internal_item_selector {
    template<typename RNG>
    size_t operator()(size_t num_items, RNG &rng) {
        return rng.template next_int<size_t>(1, num_items - 2);
    } 

    std::string get_name() const {
        return "internal items";
    }
};
struct random_endpoint_selector {
    template<typename RNG>
    size_t operator()(size_t num_items, RNG &rng) {
        return rng.template next_int<size_t>(0, 1)*(num_items - 1);
    }

    std::string get_name() const {
        return "endpoints";
    }
};
struct random_item_selector {
    template<typename RNG>
    size_t operator()(size_t num_items, RNG &rng) {
        return rng.template next_int<size_t>(0, num_items - 1);
    } 

    std::string get_name() const {
        return "all items";
    }
};
struct worst_case_selector {
    template<typename RNG>
    size_t operator()(size_t, RNG&) {
        return 2;
    }

    std::string get_name() const {
        return "worst case";
    }
};

template<typename Selector, typename Generator, typename RNG>
void local_maintenance(size_t num_items,
                       const typename Generator::parameters& gen_params,
                       RNG& rng,
                       unsigned int num_reps,
                       std::pair<function_value_type, function_value_type> change_bounds,
                       int num_divisions,
                       bool run_gudhi,
                       bool run_persistence1d) {
    std::vector<function_value_type> values;
    std::vector<list_item*> item_ptrs;

    const auto change_range = change_bounds.second - change_bounds.first;

    Timer<std::chrono::nanoseconds> timer;
    csv_writer writer;
    multirow_csv_writer structure_writer;

    for (unsigned int rep = 0; rep < num_reps; ++rep) {

        values.clear();
        Generator generator{gen_params};
        generator(values, num_items);

        auto index = Selector{}(num_items, rng);

        for (int div = 0; div < num_divisions; ++div) {
            std::cout << "> rep " << rep << "." << div << "\n";

            writer << std::make_pair("num_items", num_items)
                   << std::make_pair("num_reps", num_reps)
                   << std::make_pair("change_min", change_bounds.first)
                   << std::make_pair("change_max", change_bounds.second)
                   << std::make_pair("rep", rep)
                   << std::make_pair("div", div);
            generator.write_parameters(writer);

            persistence_context context;
            item_ptrs.clear();
            auto* the_interval = context.new_interval(values, {std::ref(item_ptrs)});

            auto* item_to_change = item_ptrs[index];
            const auto change = change_bounds.first + change_range * (function_value_type)div/((function_value_type)num_divisions - 1.0);

            auto criticality_before = context.criticality_as_string(item_to_change);
            persistence_diagram pd_before, pd_after;
            context.compute_persistence_diagram(pd_before);

            if (output_file.is_open()) {
                structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                      "." + std::to_string(rep) +
                                                                      "." + std::to_string(div) +
                                                                      ".pre-" + gen_params.to_string()));
                context.analyse_all_intervals(structure_writer);
                structure_writer.write_to_stream_and_reset(output_file, rep == 0);
            }

            const auto original_value = values[index];
            values[index] = values[index] + change;

            persistence_stats.reset();
            dictionary_stats.reset();
            timer.restart();
            context.change_value(the_interval, item_to_change, values[index]);
            const auto change_time = timer.elapsed();

            auto criticality_after = context.criticality_as_string(item_to_change);
            context.compute_persistence_diagram(pd_after);

            auto pd_diff = persistence_diagram::symmetric_difference(pd_before, pd_after);

            writer << std::make_pair("index", index)
                   << std::make_pair("change", change)
                   << std::make_pair("time", change_time)
                   << std::make_pair("criticality_change", criticality_before + "->" + criticality_after)
                   << std::make_pair("diff_points", pd_diff.points)
                   << std::make_pair("diff_arrows", pd_diff.arrows);

            if (output_file.is_open()) {
                structure_writer.on_every_row(std::make_pair("stamp", std::to_string(num_items) +
                                                                      "." + std::to_string(rep) +
                                                                      "." + std::to_string(div) +
                                                                      ".post-" + gen_params.to_string()));
                context.analyse_all_intervals(structure_writer);
                structure_writer.write_to_stream_and_reset(output_file, false);
            }

            if (run_gudhi) {
                timer.restart();
                Gudhi::persistent_cohomology::compute_persistence_of_function_on_line(values, [](auto, auto) {});
                const auto change_time_gudhi = timer.elapsed();
                writer << std::make_pair("time_gudhi", change_time_gudhi);
            }
            if (run_persistence1d) {
                p1d::Persistence1D p1d;
                timer.restart();
                p1d.RunPersistence(values);
                const auto change_time_p1d = timer.elapsed();
                writer << std::make_pair("time_p1d", change_time_p1d);
            }

            persistence_stats.write_statistics<std::chrono::nanoseconds>(writer);
            persistence_stats.reset();
            dictionary_stats.write_statistics<std::chrono::nanoseconds>(writer);
            dictionary_stats.reset();
            writer.write_to_stream_and_reset(std::cout);

            // Reset the data structure
            values[index] = original_value;
            context.change_value(the_interval, item_to_change, values[index]);
        }
    }
}

template<typename Generator, typename RNG>
void internal_local_maintenance(size_t num_items,
                                const typename Generator::parameters& gen_params,
                                RNG& rng,
                                unsigned int num_reps,
                                function_value_type magnitude,
                                int num_divisions,
                                bool run_gudhi,
                                bool run_persistence1d) {
    local_maintenance<random_internal_item_selector, Generator>(
        num_items, gen_params, rng, num_reps, {-magnitude, magnitude}, num_divisions, run_gudhi, run_persistence1d
    );
}

template<typename Generator, typename RNG>
void endpoint_local_maintenance(size_t num_items,
                                const typename Generator::parameters& gen_params,
                                RNG& rng,
                                unsigned int num_reps,
                                function_value_type magnitude,
                                int num_divisions,
                                bool run_gudhi,
                                bool run_persistence1d) {
    local_maintenance<random_endpoint_selector, Generator>(
        num_items, gen_params, rng, num_reps, {-magnitude, magnitude}, num_divisions, run_gudhi, run_persistence1d
    );
}

template<typename Generator, typename RNG>
void random_item_local_maintenance(size_t num_items,
                                   const typename Generator::parameters& gen_params,
                                   RNG& rng,
                                   unsigned int num_reps,
                                   function_value_type magnitude,
                                   int num_divisions,
                                   bool run_gudhi,
                                   bool run_persistence1d) {
    local_maintenance<random_item_selector, Generator>(
        num_items, gen_params, rng, num_reps, {-magnitude, magnitude}, num_divisions, run_gudhi, run_persistence1d
    );
}

template<typename RNG>
void worst_case_local_maintenance(size_t num_items,
                                  const typename local_worst_case_generator<RNG>::parameters& gen_params,
                                  RNG& rng,
                                  unsigned int num_reps,
                                  bool run_gudhi,
                                  bool run_persistence1d) {
    local_maintenance<worst_case_selector, local_worst_case_generator<RNG>>(
        num_items, gen_params, rng, num_reps, {1.1, static_cast<function_value_type>(num_items) + 0.6}, 2, run_gudhi, run_persistence1d
    );
}

int main(int argc, char** argv) {

    std::array<size_t, 3> num_item_limits;
    unsigned long seed = random_seed();
    size_t num_reps;
    function_value_type magnitude;
    int num_divisions = 2;
    bool run_gudhi = false;
    bool run_persistence1d = false;
    std::string generator_args = "rw:0";
    std::string output_file_name;

    CLI::App app("Construction Experiments");

    add_num_items_option(app, num_item_limits) -> required();
    add_seed_option(app, seed);
    add_num_reps_option(app, num_reps);
    add_gudhi_flag(app, run_gudhi);
    add_persistence1d_flag(app, run_persistence1d);
    auto* gen_opt = add_gen_args_option(app, generator_args);
    add_output_file_option(app, output_file_name);
    app.add_option("-m,--magnitude",
                   magnitude,
                   "Perform value changes in the interval [-m,m]")
        ->default_val(1)
        ->check(CLI::PositiveNumber);
    app.add_option("-d,--divisions",
                   num_divisions,
                   "How many value changes to perform in [-m,m]")
        ->check(CLI::Range(2, std::numeric_limits<int>::max()));

    auto* internal_app = app.add_subcommand("internal", "Updates to internal items");
    auto* endpoint_app = app.add_subcommand("endpoint", "Updates to endpoints");
    auto* random_item_app = app.add_subcommand("random", "Updates to random items, including internal ones and endpoints");
    auto* worst_case_app = app.add_subcommand("worst-case", "Linear time anticancellation and linear number of interchanges");
    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);

    if (num_item_limits[0] < 2 || num_item_limits[1] == 0 || num_item_limits[2] < num_item_limits[0]) {
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

    const auto min_num_items = num_item_limits[0];
    const auto step_num_items = num_item_limits[1];
    const auto max_num_items = num_item_limits[2];
    random_number_generator rng{seed};

    const auto [gen_name, gen_param_string] = split_generator_args(generator_args);

    auto logspace_items = logspace<size_t>(min_num_items, max_num_items, step_num_items);

    if (app.got_subcommand(internal_app)) {
        std::cout << "# Local maintenance under change to internal items.\n";
        for (auto num_items: logspace_items) {
            if (gen_name == random_walk_generator<>::get_name()) {
                internal_local_maintenance<random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                internal_local_maintenance<gaussian_random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                internal_local_maintenance<sum_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
                internal_local_maintenance<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            }
            std::cout << "--\n";
        }
    }
    if (app.got_subcommand(endpoint_app)) {
        std::cout << "# Local maintenance under change to endpoints.\n";
        for (auto num_items: logspace_items) {
            if (gen_name == random_walk_generator<>::get_name()) {
                endpoint_local_maintenance<random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                endpoint_local_maintenance<gaussian_random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                endpoint_local_maintenance<sum_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
                endpoint_local_maintenance<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            }
            std::cout << "--\n";
        }
    }
    if (app.got_subcommand(random_item_app)) {
        std::cout << "# Local maintenance under change to any item.\n";
        for (auto num_items: logspace_items) {
            if (gen_name == random_walk_generator<>::get_name()) {
                random_item_local_maintenance<random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
                random_item_local_maintenance<gaussian_random_walk_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
                random_item_local_maintenance<sum_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
                random_item_local_maintenance<modulating_quasi_periodic_generator<decltype(rng)>>(num_items, {rng, gen_param_string}, rng, num_reps, magnitude, num_divisions, run_gudhi, run_persistence1d);
            }
            std::cout << "--\n";
        }
    }
    if (app.got_subcommand(worst_case_app)) {
        if (*gen_opt && gen_name != local_worst_case_generator<>::get_name()) {
            std::cerr << "worst-case app requires local-wc generator.\n";
            return 1;
        }
        if (min_num_items < 6) {
            std::cerr << "Need at least 6 items for local maintenance worst-case.\n";
            return 1;
        } 
        std::cout << "#Worst case for local maintenance.\n";
        for (auto num_items: logspace_items) {
            if (num_items % 2 == 1) {
                num_items++;
            }
            worst_case_local_maintenance(num_items, {rng, gen_param_string}, rng, num_reps, run_gudhi, run_persistence1d);
            std::cout << "--\n";
        }
    }

    return 0;

}
