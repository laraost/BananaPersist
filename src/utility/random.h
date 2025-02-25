#pragma once

#include <random>

inline unsigned long random_seed() {
    return std::random_device{}();
}

template<typename engine_type = std::mt19937>
class random_number_generator {

public:
    random_number_generator(typename engine_type::result_type seed = random_seed()) : engine(seed), initial_seed(seed) {}

    // Generate a random real number in the interval `[min, max)`.
    template<typename F>
    F next_real(F min, F max) {
        return std::uniform_real_distribution<F>{min, max}(engine);
    }

    template<typename F>
    F next_normal_real(F mean, F sd) {
        return std::normal_distribution<F>{mean, sd}(engine);
    }

    template<typename I>
    I next_int(I min, I max) {
        return std::uniform_int_distribution<I>{min, max}(engine);
    }

    auto get_seed() {
        return initial_seed;
    }

private:
    engine_type engine;
    typename engine_type::result_type initial_seed;

};
