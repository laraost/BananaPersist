#include <fstream>

#include "CLI11.hpp"

#include "app/experiments/utility/cli_options.h"
#include "app/experiments/utility/data_generation.h"
#include "persistence_defs.h"
#include "utility/random.h"

using namespace bananas;

int main(int argc, char** argv) {

    size_t num_items;
    unsigned long seed = random_seed();
    std::string generator_args = "rw:0";
    std::string output_file_name;

    CLI::App app("Generate Data");

    add_num_items_option(app, num_items)->required();
    add_seed_option(app, seed);
    add_gen_args_option(app, generator_args);
    add_output_file_option(app, output_file_name);
    
    CLI11_PARSE(app, argc, argv);

    std::ofstream output_file;
    std::ostream* output_stream = &std::cout;
    
    if (output_file_name != "") {
        output_file.open(output_file_name);
        if (!output_file.is_open()) {
            std::cerr << "Failed to open " << output_file_name << "\n";
            return 1;
        }
        output_stream = &output_file;
    }

    const auto [gen_name, gen_param_string] = split_generator_args(generator_args);

    std::vector<function_value_type> values;

    if (gen_name == random_walk_generator<>::get_name()) {
        random_number_generator rng{seed};
        random_walk_generator<> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == gaussian_random_walk_generator<>::get_name()) {
        random_number_generator rng{seed};
        gaussian_random_walk_generator<> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == sum_quasi_periodic_generator<>::get_name()) {
        random_number_generator rng{seed};
        sum_quasi_periodic_generator<> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == modulating_quasi_periodic_generator<>::get_name()) {
        random_number_generator rng{seed};
        modulating_quasi_periodic_generator<> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == local_worst_case_generator<>::get_name()) {
        if (num_items < 6) {
            std::cerr << "Need at least 6 items for local maintenance worst-case\n";
            return 1;
        }
        random_number_generator rng{seed};
        local_worst_case_generator<> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == topological_worst_case_generator<false>::get_name()) {
        if (num_items < 4) {
            std::cerr << "Need at least 4 items for topological maintenance worst-case\n";
            return 1;
        }
        if (num_items % 4 != 0) {
            std::cerr << "Number of items needs to be a multiple of 4 for topological maintenance worst-case.\n";
            return 1;
        }
        random_number_generator rng{seed};
        topological_worst_case_generator<false> gen{{rng, gen_param_string}};
        gen(values, num_items);
    } else if (gen_name == topological_worst_case_generator<true>::get_name()) {
        if (num_items < 4) {
            std::cerr << "Need at least 4 items for topological maintenance worst-case\n";
            return 1;
        }
        if (num_items % 4 != 0) {
            std::cerr << "Number of items needs to be a multiple of 4 for topological maintenance worst-case.\n";
            return 1;
        }
        random_number_generator rng{seed};
        topological_worst_case_generator<true> gen{{rng, gen_param_string}};
        std::vector<function_value_type> tmp_vector_left, tmp_vector_right;
        gen(tmp_vector_left, num_items/2);
        gen(tmp_vector_right, num_items - num_items/2);
        values.insert(values.end(), tmp_vector_left.begin(), tmp_vector_left.end());
        values.insert(values.end(), tmp_vector_right.begin(), tmp_vector_right.end());
    }

    *output_stream << "x, y\n";
    for (size_t idx = 0; idx < values.size(); ++idx) {
        *output_stream << idx << ", " << values[idx] << "\n";
    }

    return 0;
}
