#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>

#include "CLI11.hpp"

#include "app/experiments/utility/cli_options.h"
#include "datastructure/persistence_context.h"
#include "datastructure/persistence_diagram.h"
#include "gudhi/Persistence_on_a_line.h"
#include "persistence_defs.h"
#include "utility/errors.h"
#include "utility/format_util.h"
#include "utility/random.h"
#include "utility/stats.h"
#include "utility/timer.h"

using namespace bananas;

std::ofstream output_file;

std::vector<function_value_type> read_value_from_stream(std::istream &stream) {
    std::vector<function_value_type> values;
    function_value_type value;
    while (stream >> value) {
        if (values.empty() || value != values.back()) {
            values.push_back(value);
        }
    }
    return values;
}

template<typename rng_type>
void structure_experiment(size_t num_reps, bool run_gudhi, rng_type& rng, function_value_type noise_amount) {
    std::vector<function_value_type> values = read_value_from_stream(std::cin);
    std::vector<function_value_type> diff_values = values;
    std::sort(diff_values.begin(), diff_values.end());
    auto min_diff = std::numeric_limits<function_value_type>::infinity();
    for (size_t i = 0; i < diff_values.size() - 1; ++i) {
        if (diff_values[i+1] - diff_values[i] != 0) {
            min_diff = std::min(min_diff, std::abs(diff_values[i+1] - diff_values[i]));
        }
    }
    massert(min_diff > 0, "Expected smallest non-negative difference to be non-negative.");
    const auto noise_scale = min_diff*noise_amount;
    DEBUG_MSG("Adding noise scaled to " << noise_scale << " with min_diff = " << min_diff);
    for (auto& val: values) {
        val += noise_scale*rng.next_real(-0.5,0.5);
    }

    csv_writer writer;
    multirow_csv_writer structure_writer;

    for (size_t rep = 0; rep < num_reps; ++rep) {
        std::cout << "> rep " << rep << "\n";
        writer << std::make_pair("num_items", values.size())
               << std::make_pair("rep", rep);

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
        context.print_memory_stats(writer);

        writer << std::make_pair("global_max_pos", context.get_global_max_order(the_interval))
               << std::make_pair("global_max_value", context.get_global_max_value(the_interval))
               << std::make_pair("global_min_pos", context.get_global_min_order(the_interval))
               << std::make_pair("global_min_value", context.get_global_min_value(the_interval));

        writer.write_to_stream_and_reset(std::cout);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(rep)));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, rep == 0);
        }
    }
}





int main(int argc, char** argv) {

    unsigned long seed = random_seed();
    size_t num_reps = 1;
    bool run_gudhi = false;
    std::string output_file_name;
    function_value_type noise_amount = 1e-2;

    CLI::App app("Experiments on Time Series");

    add_num_reps_option(app, num_reps);
    add_gudhi_flag(app, run_gudhi);
    add_output_file_option(app, output_file_name);
    add_seed_option(app, seed);
    app.add_option("-r,--random-range",
                   noise_amount,
                   "Scale of noise relative to (max-min); default is 1e-5.");
    
    auto* construct_app = app.add_subcommand("construct", "Construct banana trees.");

    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);
    
    if (output_file_name != "") {
        output_file.open(output_file_name);
        if (!output_file.is_open()) {
            std::cerr << "Failed to open " << output_file_name << "\n";
            return 1;
        }
    }

    random_number_generator<> rng{seed};

    if (app.got_subcommand(construct_app)) {
        std::cout << "# Constructing banana trees.\n";
        structure_experiment(num_reps, run_gudhi, rng, noise_amount);
        std::cout << "--\n";
    }

    return 0;
}
