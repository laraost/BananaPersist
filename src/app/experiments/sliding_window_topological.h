#pragma once

#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <ranges>
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

constexpr size_t min_allowed_step_size = 2;

constexpr std::array<size_t, 3> default_window_step = {2, 1, 2};

template<typename Generator>
inline void sliding_window(size_t num_slides,
                           size_t window_size,
                           size_t step_size,
                           const typename Generator::parameters& gen_params,
                           bool run_gudhi,
                           bool run_persistence1d) {

    std::vector<function_value_type> values;
    std::vector<function_value_type> all_values;
    using iter_t = decltype(all_values)::iterator;

    Generator generator{gen_params};
    generator(values, window_size);
    if (run_gudhi || run_persistence1d) {
        all_values.insert(all_values.end(), values.begin(), values.end());
    }

    persistence_context context;
    std::vector<list_item*> item_ptrs;
    auto* window_interval = context.new_interval(values, std::ref(item_ptrs));

    Timer<std::chrono::nanoseconds> timer;
    Timer<std::chrono::nanoseconds> sub_timer;
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

    for (size_t slide = 0; slide < num_slides; ++slide) {
        std::cout << "> rep " << slide << "\n";

        writer << std::make_pair("window_size", window_size)
               << std::make_pair("step_size", step_size)
               << std::make_pair("method", "topological");
        generator.write_parameters(writer);

        values.clear();
        generator(values, step_size);
        if (run_gudhi || run_persistence1d) {
            all_values.insert(all_values.end(), values.begin(), values.end());
        }

        timer.restart();
        auto start_timestamp = timer.now();
        // Construct new interval
        auto* new_interval = context.new_interval(values, std::ref(item_ptrs), window_size * (slide + 1));
        auto post_construct_timestamp = timer.now();
        // Remove old items
        auto ival_pair = context.cut_interval(window_interval, item_ptrs[step_size-1]);
        item_ptrs.erase(item_ptrs.begin(), item_ptrs.begin() + step_size);
        auto* left_interval = ival_pair.first;
        context.delete_interval(left_interval);
        auto post_remove_timestamp = timer.now();
        // Add new items
        window_interval = ival_pair.second;
        context.glue_intervals(window_interval, new_interval);
        auto post_slide_timestamp = timer.now();

        massert(item_ptrs.size() == window_size, "There should be exactly one item pointer for each item in the window.");
        massert(context.get_num_intervals() == 1, "Expected exactly one interval.");

        auto slide_time = post_slide_timestamp - start_timestamp;
        auto construct_new_time = post_construct_timestamp - start_timestamp;
        auto remove_old_time = post_remove_timestamp - post_construct_timestamp;
        auto append_new_time = post_slide_timestamp - post_remove_timestamp;

        context.compute_persistence_diagram(*pd_after);
        auto pd_diff = persistence_diagram::symmetric_difference(*pd_before, *pd_after);
        std::swap(pd_before, pd_after);
        pd_after->clear_diagrams();

        writer << std::make_pair("time", slide_time)
               << std::make_pair("construct_new_time", construct_new_time)
               << std::make_pair("remove_old_time", remove_old_time)
               << std::make_pair("append_new_time", append_new_time)
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
                 ::compute_persistence_of_function_on_line(std::ranges::subrange<iter_t>(all_values.begin() + (all_values.size() - window_size),
                                                                                         all_values.end()),
                                                           [] (auto, auto) {});
            const auto slide_time_gudhi = timer.elapsed();
            writer << std::make_pair("time_gudhi", slide_time_gudhi);
        }
        if (run_persistence1d) {
            auto window_range = std::ranges::subrange<iter_t>(all_values.begin() + (all_values.size() - window_size),
                                                              all_values.end());
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
