#pragma once

#include <array>
#include <chrono>
#include <fstream>
#include <ranges>
#include <utility>
#include <vector>

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
using std::size_t;

extern std::ofstream output_file;

constexpr size_t min_allowed_step_size = 1;

constexpr std::array<size_t, 3> default_window_step = {1, 1, 1};

template<typename Generator>
inline void sliding_window(size_t num_slides,
                           size_t window_size,
                           size_t step_size,
                           const typename Generator::parameters& gen_params,
                           bool run_gudhi,
                           bool run_persistence1d) {

    std::vector<function_value_type> values;
    using iter_t = decltype(values)::iterator;
    Generator generator{gen_params};
    generator(values, window_size);
    values.reserve(window_size + num_slides * step_size);

    persistence_context context;
    auto* const the_interval = context.new_interval(values);

    Timer<std::chrono::nanoseconds> timer;
    csv_writer writer;
    multirow_csv_writer structure_writer;

    std::array<persistence_diagram, 2> persistence_diagrams{persistence_diagram{}, persistence_diagram{}};
    persistence_diagram* pd_before = &persistence_diagrams[0];
    persistence_diagram* pd_after = &persistence_diagrams[1];

    context.compute_persistence_diagram(*pd_before);

    if (output_file.is_open()) {
        structure_writer.on_every_row(std::make_pair("stamp", std::to_string(window_size) +
                                                              "." + std::to_string(step_size) +
                                                              ".0-" + gen_params.to_string()));
        context.analyse_all_intervals(structure_writer);
        structure_writer.write_to_stream_and_reset(output_file);
    }

    persistence_stats.reset();
    dictionary_stats.reset();

    for (size_t slide = 1; slide <= num_slides; ++slide) {
        std::cout << "> rep " << slide << "\n";

        writer << std::make_pair("window_size", window_size)
               << std::make_pair("step_size", step_size)
               << std::make_pair("method", "local");
        generator.write_parameters(writer);

        timer.restart();
        for (size_t step = 0; step < step_size; ++step) {
            context.delete_left_endpoint(the_interval);
        }
        for (size_t step = 0; step < step_size; ++step) {
            values.push_back(generator.next_value());
            context.insert_right_endpoint(the_interval, 1, values.back());
        }
        auto slide_time = timer.elapsed();

        context.compute_persistence_diagram(*pd_after);
        auto pd_diff = persistence_diagram::symmetric_difference(*pd_before, *pd_after);
        std::swap(pd_before, pd_after);
        pd_after->clear_diagrams();

        writer << std::make_pair("time", slide_time)
               << std::make_pair("diff_points", pd_diff.points)
               << std::make_pair("diff_arrows", pd_diff.arrows);

        if (output_file.is_open()) {
            structure_writer.on_every_row(std::make_pair("stamp", std::to_string(window_size) +
                                                                  "." + std::to_string(step_size) +
                                                                  "." + std::to_string(slide) +
                                                                  "-" + gen_params.to_string()));
            context.analyse_all_intervals(structure_writer);
            structure_writer.write_to_stream_and_reset(output_file, false);
        }

        if (run_gudhi) {
            timer.restart();
            Gudhi::persistent_cohomology
                 ::compute_persistence_of_function_on_line(std::ranges::subrange<iter_t>(values.begin() + (values.size() - window_size),
                                                                                         values.end()),
                                                           [] (auto, auto) {});
            const auto slide_time_gudhi = timer.elapsed();
            writer << std::make_pair("time_gudhi", slide_time_gudhi);
        }
        if (run_persistence1d) {
            auto window_range = std::ranges::subrange<iter_t>(values.begin() + (values.size() - window_size),
                                                              values.end());
            std::vector<function_value_type> window_values{window_range.begin(), window_range.end()};
            p1d::Persistence1D p1d;
            timer.restart();
            p1d.RunPersistence(window_values);
            const auto slide_time_p1d = timer.elapsed();
            writer << std::make_pair("time_p1d", slide_time_p1d);
        }

        persistence_stats.write_statistics(writer);
        persistence_stats.reset();
        dictionary_stats.write_statistics(writer);
        dictionary_stats.reset();

        context.print_memory_stats(writer);

        writer.write_to_stream_and_reset(std::cout);
    }

}
