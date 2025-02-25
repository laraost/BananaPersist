#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

#include "persistence_defs.h"
#include "utility/debug.h"
#include "utility/errors.h"
#include "utility/format_util.h"
#include "utility/random.h"

namespace bananas {

template<typename T>
std::vector<T> logspace(double min, double max, size_t target_size) {
    std::vector<T> result;
    result.reserve(target_size);
    double q = std::pow(max/min, 1/((double)target_size - 1.0));
    for (size_t i = 0; i < target_size; ++i) {
        result.push_back(static_cast<T>(min * std::pow(q, i)));
    }
    return result;
}

inline std::pair<std::string, std::string> split_generator_args(const std::string& input) {
    auto pos = input.find(':');
    return {input.substr(0, pos), input.substr(pos+1)};
}

template<typename rng_type = random_number_generator<>>
struct random_walk_generator {

    constexpr static bool has_state = true;
    static std::string get_name() { return "rw"; }

    struct parameters {
        rng_type& rng;
        function_value_type bias = 0;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng),
            bias(std::stod(args)) {
        }

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "b" + std::to_string(bias);
        }
    };

    random_walk_generator(const parameters& params) : rng(params.rng), bias(params.bias) {}

    void operator()(std::vector<function_value_type> &values,
                           size_t target_size) {
        values.reserve(target_size);    
        while (values.size() < target_size) {
            values.push_back(next_value());
            last_value = values.back();
        }
    }

    function_value_type next_value() {
        last_value = last_value + rng.template next_real<function_value_type>(-1, 1) + bias;
        return last_value;
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name())
               << std::make_pair("seed", rng.get_seed())
               << std::make_pair("bias", bias);
    }

private:
    rng_type& rng;
    function_value_type last_value = 0;
    function_value_type bias = 0;

};

template<typename rng_type = random_number_generator<>>
struct gaussian_random_walk_generator {

    constexpr static bool has_state = true;
    static std::string get_name() { return "grw"; }

    struct parameters {
        rng_type& rng;
        function_value_type mean = 0;
        function_value_type sd = 1;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng),
            mean(std::stod(args)) {
            const auto semi_pos = args.find(';');
            if (semi_pos == std::string::npos) {
                sd = 1;
            } else {
                sd = std::stod(args.substr(semi_pos + 1));
            }
        }

        parameters(rng_type& rng, function_value_type mean, function_value_type sd) :
            rng(rng),
            mean(mean),
            sd(sd) {}

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "m" + std::to_string(mean) +
                "s" + std::to_string(sd);
        }

    };

    gaussian_random_walk_generator(const parameters& params) : rng(params.rng), mean(params.mean), sd(params.sd) {}

    void operator()(std::vector<function_value_type>& values,
                    size_t target_size) {
        values.reserve(target_size);
        while (values.size() < target_size) {
            values.push_back(next_value());
            last_value = values.back();
        }
    }

    function_value_type next_value() {
        last_value = last_value + rng.template next_normal_real<function_value_type>(mean, sd);
        return last_value;
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name())
               << std::make_pair("seed", rng.get_seed())
               << std::make_pair("grw_mean", mean)
               << std::make_pair("grw_sd", sd);
    }

private:
    rng_type& rng;
    const function_value_type mean;
    const function_value_type sd;

    function_value_type last_value = 0;

};

template<typename rng_type = random_number_generator<>>
struct sum_quasi_periodic_generator {

    constexpr static bool has_state = true;
    static std::string get_name() { return "sqp"; }

    struct parameters {
        rng_type& rng;
        function_value_type period = 100;
        function_value_type amplitude = 1;
        function_value_type mean = 0;
        function_value_type sd = 1;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng) {
            if (!args.empty()) {
                period = std::stod(args);
                auto semi_pos = args.find(';');
                if (semi_pos != std::string::npos) {
                    amplitude = std::stod(args.substr(semi_pos + 1));
                    semi_pos = args.find(';', semi_pos + 1);
                    if (semi_pos != std::string::npos) {
                        mean = std::stod(args.substr(semi_pos + 1));
                        semi_pos = args.find(';', semi_pos + 1);
                        if (semi_pos != std::string::npos) {
                            sd = std::stod(args.substr(semi_pos + 1));
                        }
                    }
                }
            }
        }

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "m" + std::to_string(mean) +
                "s" + std::to_string(sd) +
                "p" + std::to_string(period) +
                "a" + std::to_string(amplitude);
        }
    };

    sum_quasi_periodic_generator(const parameters& params) :
        rng(params.rng),
        period(params.period),
        amplitude(params.amplitude),
        mean(params.mean),
        sd(params.sd) {}

    void operator()(std::vector<function_value_type>& values,
                    size_t target_size) {
        values.reserve(target_size);
        while (values.size() < target_size) {
            values.push_back(next_value());
        }
    }

    function_value_type next_value() {
        last_value = last_value + rng.template next_normal_real<function_value_type>(mean, sd)
                                + amplitude * std::sin(x * 2 * std::numbers::pi / period);
        x++;
        return last_value;
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name());
        writer << std::make_pair("sqp_period", period)
               << std::make_pair("sqp_amplitude", amplitude)
               << std::make_pair("sqp_mean", mean)
               << std::make_pair("sqp_sd", sd);
    }

private:
    rng_type& rng;
    const function_value_type period;
    const function_value_type amplitude;
    const function_value_type mean;
    const function_value_type sd;

    long x = 0;
    function_value_type last_value = 0;
};

template<typename rng_type = random_number_generator<>>
struct modulating_quasi_periodic_generator {

    constexpr static bool has_state = false;
    static std::string get_name() { return "mqp"; }

    struct parameters {
        rng_type& rng;
        function_value_type num_periods = 5.5;
        function_value_type amplitude = 1;
        function_value_type sd = 1;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng) {
            if (!args.empty()) {
                num_periods = std::stod(args);
                auto semi_pos = args.find(';');
                if (semi_pos != std::string::npos) {
                    amplitude = std::stod(args.substr(semi_pos + 1));
                    semi_pos = args.find(';', semi_pos + 1);
                    if (semi_pos != std::string::npos) {
                        sd = std::stod(args.substr(semi_pos + 1));
                    }
                }
            }
        }

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "s" + std::to_string(sd) +
                "p" + std::to_string(num_periods) +
                "a" + std::to_string(amplitude);
        }
    };

    modulating_quasi_periodic_generator(const parameters& params) :
        rng(params.rng),
        num_periods(params.num_periods),
        amplitude(params.amplitude),
        sd(params.sd) {}

    void operator()(std::vector<function_value_type>& values,
                    size_t target_size) {
        values.reserve(target_size);
        const auto period = target_size/num_periods;
        while (values.size() < target_size) {
            const auto mean = amplitude * std::sin(x*2 * std::numbers::pi/period);
            last_value = last_value + rng.template next_normal_real<function_value_type>(mean, sd);
            x++;
            values.push_back(last_value);
        }
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name())
               << std::make_pair("seed", rng.get_seed())
               << std::make_pair("mqp_period", num_periods)
               << std::make_pair("mqp_amplitude", amplitude)
               << std::make_pair("mqp_sd", sd);
    }

private:
    rng_type& rng;
    const function_value_type num_periods;
    const function_value_type amplitude;
    const function_value_type sd;

    long x = 0;
    function_value_type last_value = 0;
};

template<typename rng_type = random_number_generator<>>
struct local_worst_case_generator {

    constexpr static bool has_state = false;
    static std::string get_name() { return "local-wc"; }

    struct parameters {
        rng_type& rng;
        function_value_type noise_amount = 0;
        function_value_type mean = 0;
        function_value_type sd = 1;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng) {
            if (!args.empty()) {
                noise_amount = std::stod(args);
                auto semi_pos = args.find(';');
                if (semi_pos != std::string::npos) {
                    mean = std::stod(args.substr(semi_pos + 1));
                    semi_pos = args.find(';', semi_pos + 1);
                    if (semi_pos != std::string::npos) {
                        sd = std::stod(args.substr(semi_pos + 1));
                    }
                }
            }
        }

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "n" + std::to_string(noise_amount) +
                "m" + std::to_string(mean) +
                "s" + std::to_string(sd);
        }
    };

    inline local_worst_case_generator(const parameters& params) :
        rng(params.rng),
        noise_amount(params.noise_amount),
        mean(params.mean),
        sd(params.sd),
        grw_generator({rng, mean, sd}) {}

    void operator()(std::vector<function_value_type> &values,
                    size_t target_size) {
        massert(target_size >= 6, "Need at least 6 items for worst case for local maintenance.");
        massert(target_size % 2 == 0, "Need an even number of items in worst case for local maintenance.");
        values.reserve(target_size);
        values.push_back(static_cast<function_value_type>(target_size));
        values.push_back(0.5);
        values.push_back(-0.5);
        int value = 1;
        while (values.size() < target_size - 1) {
            values.push_back(static_cast<function_value_type>(-value));
            values.push_back(static_cast<function_value_type>(value));
            value++;
        }
        values.push_back(-static_cast<function_value_type>(target_size));
        if (noise_amount != 0) {
            std::vector<function_value_type> noise;
            noise.reserve(target_size);
            grw_generator(noise, target_size);
            for (size_t i = 0; i < target_size; ++i) {
                values[i] = (1 - noise_amount) * values[i] + target_size * noise_amount * noise[i];
            }
        }
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name())
               << std::make_pair("seed", rng.get_seed())
               << std::make_pair("lwc_noise", noise_amount)
               << std::make_pair("grw_mean", mean)
               << std::make_pair("grw_sd", sd);
    }
    
private:
    rng_type& rng;
    const function_value_type noise_amount;
    const function_value_type mean;
    const function_value_type sd;
    gaussian_random_walk_generator<rng_type> grw_generator;

};

template<bool two_stage, typename rng_type = random_number_generator<>>
struct topological_worst_case_generator {

    constexpr static bool has_state = false;
    static std::string get_name() { return two_stage ? "glue-wc" : "cut-wc"; }

    struct parameters {
        rng_type& rng;
        function_value_type noise_amount = 0;
        function_value_type mean = 0;
        function_value_type sd = 1;

        parameters(rng_type& rng) : rng(rng) {}

        parameters(rng_type& rng, const std::string& args) : rng(rng) {
            if (!args.empty()) {
                noise_amount = std::stod(args);
                auto semi_pos = args.find(';');
                if (semi_pos != std::string::npos) {
                    mean = std::stod(args.substr(semi_pos + 1));
                    semi_pos = args.find(';', semi_pos + 1);
                    if (semi_pos != std::string::npos) {
                        sd = std::stod(args.substr(semi_pos + 1));
                    }
                }
            }
        }

        std::string to_string() const {
            return "seed" + std::to_string(rng.get_seed()) +
                "n" + std::to_string(noise_amount) +
                "m" + std::to_string(mean) +
                "s" + std::to_string(sd);
        }
    };

    inline topological_worst_case_generator(const parameters& params) :
        rng(params.rng),
        noise_amount(params.noise_amount),
        mean(params.mean),
        sd(params.sd),
        grw_generator({rng, mean, sd}) {}

    void operator()(std::vector<function_value_type> &values,
                    size_t target_size) {
        massert(target_size >= 4, "Need to have at least 4 items for topological worst-case.");
        const size_t stage_size = two_stage ? target_size : target_size / 2;
        if (decrease) {
            long value = 1;
            for (size_t i = 0; i < stage_size; i += 2) {
                values.push_back(static_cast<function_value_type>(-value));
                values.push_back(static_cast<function_value_type>(value));
                value++;
            }
            std::reverse(values.begin(), values.begin() + stage_size);
        }
        if (!two_stage || !decrease) {
            decrease = false;
            long value = 1;
            auto size_offset = two_stage ? values.size() : 0;
            while (values.size() < target_size  - 1 + size_offset) {
                values.push_back(static_cast<function_value_type>(value) + 0.1);
                values.push_back(static_cast<function_value_type>(-value) + 0.1);
                value++;
            }
            values.push_back(static_cast<function_value_type>(value) + 0.1);
        }
        if (noise_amount != 0) {
            auto scale = stage_size/2;
            if (two_stage && !decrease) {
                scale = (target_size - 1)/2;
            }
            for (size_t i = 0; i < values.size(); ++i) {
                values[i] = (1 - noise_amount) * values[i] + scale * noise_amount * grw_generator.next_value();
            }
        }
        decrease = false;
    }

    void write_parameters(csv_writer& writer) {
        writer << std::make_pair("gen", get_name())
               << std::make_pair("seed", rng.get_seed())
               << std::make_pair("lwc_noise", noise_amount)
               << std::make_pair("grw_mean", mean)
               << std::make_pair("grw_sd", sd);
    }

private:
    rng_type& rng;
    const function_value_type noise_amount;
    const function_value_type mean;
    const function_value_type sd;
    gaussian_random_walk_generator<rng_type> grw_generator;

    bool decrease = true;
};

}
