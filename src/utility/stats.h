#pragma once

#include <array>
#include <chrono>
#include <iomanip>
#include <limits>
#include <ostream>

#include "datastructure/banana_tree_sign_template.h"
#include "persistence_defs.h"
#include "utility/format_util.h"

namespace bananas {

namespace detail {

using count_type = long;

constexpr inline size_t sign_to_index(int sign) {
    return (sign + 1) / 2;
}

template<int w1, int w2, typename T>
void write_var_inline(std::ostream& stream, const std::string& name, const std::array<T, 2> &vars) {
    stream << std::setw(w1) << name << ", "
           << std::setw(w2) << vars[0] << ", "
           << std::setw(w2) << vars[1] << "\n";
}

} // End of namespace detail

namespace time {
using clock_type = std::chrono::high_resolution_clock;
using duration_type = std::chrono::nanoseconds;
using duration_rep = duration_type::rep;

clock_type::time_point time_now();

duration_type time_diff(clock_type::time_point begin, clock_type::time_point end);

template<typename out_duration, typename in_duration>
std::array<out_duration, 2> convert_times(std::array<in_duration, 2> &times) {
    return {std::chrono::duration_cast<out_duration>(times[0]),
            std::chrono::duration_cast<out_duration>(times[1])};
}

}

#define COUNT_VAR(name) \
    count_##name
#define COUNT_MAX_VAR(name) \
    count_max_##name
#define COUNT_MIN_VAR(name) \
    count_min_##name

#define BEGIN_INCREMENT_FUNCTION(name) \
    SIGN_TEMPLATE \
    void increment_##name()
#define INCREMENT_FUNCTION_BODY(name) \
        COUNT_VAR(name)[detail::sign_to_index(sign)]++;
#define INCREMENT_FUNCTION(name) \
    BEGIN_INCREMENT_FUNCTION(name) { \
        INCREMENT_FUNCTION_BODY(name) \
    }
#define INCREMENT_MIN_MAX_FUNCTION(name) \
    BEGIN_INCREMENT_FUNCTION(name) { \
        INCREMENT_FUNCTION_BODY(name) \
        COUNT_MAX_VAR(name)[detail::sign_to_index(sign)] = \
            std::max(COUNT_MAX_VAR(name)[detail::sign_to_index(sign)], \
                     COUNT_VAR(name)[detail::sign_to_index(sign)]); \
    }

#define BEGIN_DECREMENT_FUNCTION(name) \
    SIGN_TEMPLATE \
    void decrement_##name()
#define DECREMENT_FUNCTION_BODY(name) \
        COUNT_VAR(name)[detail::sign_to_index(sign)]--;
#define DECREMENT_FUNCTION(name) \
    BEGIN_DECREMENT_FUNCTION(name) { \
        DECREMENT_FUNCTION_BODY(name) \
    }
#define DECREMENT_MIN_MAX_FUNCTION(name) \
    BEGIN_DECREMENT_FUNCTION(name) { \
        DECREMENT_FUNCTION_BODY(name) \
        COUNT_MIN_VAR(name)[detail::sign_to_index(sign)] = \
            std::min(COUNT_MIN_VAR(name)[detail::sign_to_index(sign)], \
                     COUNT_VAR(name)[detail::sign_to_index(sign)]); \
    }
#define RESET_COUNT_VAR_FUNC(name) \
    void reset_count_##name() { COUNT_VAR(name) = {0, 0}; }
#define RESET_COUNT_MIN_MAX_VAR_FUNC(name) \
    void reset_count_##name() { \
        COUNT_VAR(name) = {0, 0}; \
        COUNT_MIN_VAR(name) = {std::numeric_limits<detail::count_type>::max(), std::numeric_limits<detail::count_type>::max()}; \
        COUNT_MAX_VAR(name) = {std::numeric_limits<detail::count_type>::min(), std::numeric_limits<detail::count_type>::min()}; \
    }

#define DIST_SUM_VAR(name) \
    dist_sum_##name
#define DIST_MIN_VAR(name) \
    dist_min_##name
#define DIST_MAX_VAR(name) \
    dist_max_##name
#define DIST_FUNCTION(name) \
    SIGN_TEMPLATE \
    void new_##name(detail::count_type value) { \
        DIST_SUM_VAR(name)[detail::sign_to_index(sign)] += value; \
        DIST_MIN_VAR(name)[detail::sign_to_index(sign)] = std::min(DIST_MIN_VAR(name)[detail::sign_to_index(sign)], value); \
        DIST_MAX_VAR(name)[detail::sign_to_index(sign)] = std::max(DIST_MAX_VAR(name)[detail::sign_to_index(sign)], value); \
    }
#define RESET_DIST_FUNC(name) \
    void reset_dist_##name() { \
        DIST_SUM_VAR(name) = {0,0}; \
        DIST_MIN_VAR(name) = {std::numeric_limits<detail::count_type>::max(), std::numeric_limits<detail::count_type>::max()}; \
        DIST_MAX_VAR(name) = {std::numeric_limits<detail::count_type>::min(), std::numeric_limits<detail::count_type>::min()}; \
    }

#define TIME_VAR(name) \
    time_##name##_sum

#define TIME_FUNCTION(name) \
    SIGN_TEMPLATE \
    void time_##name(time::duration_type t) { \
        TIME_VAR(name)[detail::sign_to_index(sign)] += t; \
    }
#define RESET_TIME_FUNC(name) \
    void reset_time_##name() { TIME_VAR(name) = {time::duration_type{}, time::duration_type{}}; }

#define DEF_COUNT_VAR_AND_FUNC(name) \
    public: INCREMENT_FUNCTION(name) \
            DECREMENT_FUNCTION(name) \
            RESET_COUNT_VAR_FUNC(name) \
    private: std::array<detail::count_type, 2> COUNT_VAR(name) = {0, 0}

#define DEF_COUNT_MIN_MAX_VAR_AND_FUNC(name) \
    public: INCREMENT_FUNCTION(name) \
            DECREMENT_FUNCTION(name) \
            RESET_COUNT_MIN_MAX_VAR_FUNC(name) \
    private: std::array<detail::count_type, 2> COUNT_VAR(name) = {0, 0}; \
             std::array<detail::count_type, 2> COUNT_MAX_VAR(name) = {std::numeric_limits<detail::count_type>::min(), \
                                                                      std::numeric_limits<detail::count_type>::min()}; \
             std::array<detail::count_type, 2> COUNT_MIN_VAR(name) = {std::numeric_limits<detail::count_type>::max(), \
                                                                      std::numeric_limits<detail::count_type>::max()}

#define DEF_DIST_VAR_AND_FUNC(name) \
    public: DIST_FUNCTION(name) \
            RESET_DIST_FUNC(name) \
    private: std::array<detail::count_type, 2> DIST_SUM_VAR(name) = {0, 0}; \
             std::array<detail::count_type, 2> DIST_MAX_VAR(name) = {std::numeric_limits<detail::count_type>::min(), \
                                                                      std::numeric_limits<detail::count_type>::min()}; \
             std::array<detail::count_type, 2> DIST_MIN_VAR(name) = {std::numeric_limits<detail::count_type>::max(), \
                                                                      std::numeric_limits<detail::count_type>::max()}

#define DEF_TIME_VAR_AND_FUNC(name) \
    public: TIME_FUNCTION(name) \
            RESET_TIME_FUNC(name) \
    private: std::array<time::duration_type, 2> TIME_VAR(name)

#define PRINT_COUNT_VAR(stream, w1, w2, name) \
    detail::write_var_inline<w1, w2>(stream, #name, COUNT_VAR(name))
#define PRINT_TIME_VAR(stream, w1, w2, name, duration) \
    detail::write_var_inline<w1, w2>(stream, #name, time::convert_times<duration>(TIME_VAR(name)))

class persistence_statistics {
    DEF_COUNT_VAR_AND_FUNC(max_interchange);
    DEF_COUNT_VAR_AND_FUNC(min_interchange);
    DEF_COUNT_VAR_AND_FUNC(min_slide);
    DEF_COUNT_VAR_AND_FUNC(max_slide);
    DEF_COUNT_VAR_AND_FUNC(cancellation);
    DEF_COUNT_VAR_AND_FUNC(anticancellation);
    DEF_COUNT_VAR_AND_FUNC(anticancellation_iterations);
    DEF_COUNT_VAR_AND_FUNC(do_injury);
    DEF_COUNT_VAR_AND_FUNC(do_fatality);
    DEF_COUNT_VAR_AND_FUNC(do_scare);
    DEF_COUNT_VAR_AND_FUNC(undo_injury);
    DEF_COUNT_VAR_AND_FUNC(undo_fatality);
    DEF_COUNT_VAR_AND_FUNC(undo_scare);

    DEF_TIME_VAR_AND_FUNC(max_interchange);
    DEF_TIME_VAR_AND_FUNC(min_interchange);
    DEF_TIME_VAR_AND_FUNC(min_slide);
    DEF_TIME_VAR_AND_FUNC(max_slide);
    DEF_TIME_VAR_AND_FUNC(cancellation);
    DEF_TIME_VAR_AND_FUNC(anticancellation);
    DEF_TIME_VAR_AND_FUNC(max_increase);
    DEF_TIME_VAR_AND_FUNC(max_decrease);
    DEF_TIME_VAR_AND_FUNC(anticancellation_dict);
    DEF_TIME_VAR_AND_FUNC(do_injury);
    DEF_TIME_VAR_AND_FUNC(do_fatality);
    DEF_TIME_VAR_AND_FUNC(do_scare);
    DEF_TIME_VAR_AND_FUNC(undo_injury);
    DEF_TIME_VAR_AND_FUNC(undo_fatality);
    DEF_TIME_VAR_AND_FUNC(undo_scare);
    DEF_TIME_VAR_AND_FUNC(load_stacks);
    DEF_TIME_VAR_AND_FUNC(cut_preprocess);
    DEF_TIME_VAR_AND_FUNC(cut_postprocess);
    DEF_TIME_VAR_AND_FUNC(glue_preprocess);
    DEF_TIME_VAR_AND_FUNC(glue_postprocess);
    DEF_TIME_VAR_AND_FUNC(construct);
    DEF_TIME_VAR_AND_FUNC(construct_prepare);
    DEF_TIME_VAR_AND_FUNC(construct_loop);
    DEF_TIME_VAR_AND_FUNC(construct_cleanup);

public:
    template<typename duration = std::chrono::duration<double, std::milli>>
    void write_statistics(std::ostream& stream) {
        csv_writer writer;
        write_statistics(writer);
        writer.write_to_stream_and_reset(stream);
    }
    template<typename duration = std::chrono::duration<double, std::milli>>
    void write_statistics(csv_writer& writer) {
        writer << std::make_pair("max_xchange", COUNT_VAR(max_interchange))
               << std::make_pair("min_xchange", COUNT_VAR(min_interchange))
               << std::make_pair("min_slide", COUNT_VAR(min_slide))
               << std::make_pair("max_slide", COUNT_VAR(max_slide))
               << std::make_pair("cancel", COUNT_VAR(cancellation))
               << std::make_pair("anticancel", COUNT_VAR(anticancellation))
               << std::make_pair("anticancel_iterations", COUNT_VAR(anticancellation_iterations))
               << std::make_pair("do_injury", COUNT_VAR(do_injury))
               << std::make_pair("do_fatality", COUNT_VAR(do_fatality))
               << std::make_pair("do_scare", COUNT_VAR(do_scare))
               << std::make_pair("undo_injury", COUNT_VAR(undo_injury))
               << std::make_pair("undo_fatality", COUNT_VAR(undo_fatality))
               << std::make_pair("undo_scare", COUNT_VAR(undo_scare))
               << std::make_pair("time_max_xchange", time::convert_times<duration>(TIME_VAR(max_interchange)))
               << std::make_pair("time_min_xchange", time::convert_times<duration>(TIME_VAR(min_interchange)))
               << std::make_pair("time_min_slide", time::convert_times<duration>(TIME_VAR(min_slide)))
               << std::make_pair("time_max_slide", time::convert_times<duration>(TIME_VAR(max_slide)))
               << std::make_pair("time_cancel", time::convert_times<duration>(TIME_VAR(cancellation)))
               << std::make_pair("time_anticancel", time::convert_times<duration>(TIME_VAR(anticancellation)))
               << std::make_pair("time_max_increase", time::convert_times<duration>(TIME_VAR(max_increase)))
               << std::make_pair("time_max_decrease", time::convert_times<duration>(TIME_VAR(max_decrease)))
               << std::make_pair("time_anticancel_dict",
                                 std::chrono::duration_cast<duration>(TIME_VAR(anticancellation_dict)[detail::sign_to_index(1)]))
               << std::make_pair("time_do_injury", time::convert_times<duration>(TIME_VAR(do_injury)))
               << std::make_pair("time_do_fatality", time::convert_times<duration>(TIME_VAR(do_fatality)))
               << std::make_pair("time_do_scare", time::convert_times<duration>(TIME_VAR(do_scare)))
               << std::make_pair("time_undo_injury", time::convert_times<duration>(TIME_VAR(undo_injury)))
               << std::make_pair("time_undo_fatality", time::convert_times<duration>(TIME_VAR(undo_fatality)))
               << std::make_pair("time_undo_scare", time::convert_times<duration>(TIME_VAR(undo_scare)))
               << std::make_pair("time_load_stacks", time::convert_times<duration>(TIME_VAR(load_stacks)))
               << std::make_pair("time_cut_preprocess", time::convert_times<duration>(TIME_VAR(cut_preprocess)))
               << std::make_pair("time_cut_postprocess", time::convert_times<duration>(TIME_VAR(cut_postprocess)))
               << std::make_pair("time_glue_preprocess", time::convert_times<duration>(TIME_VAR(glue_preprocess)))
               << std::make_pair("time_glue_postprocess", time::convert_times<duration>(TIME_VAR(glue_postprocess)))
               << std::make_pair("time_construct", time::convert_times<duration>(TIME_VAR(construct)))
               << std::make_pair("time_construct_prepare", time::convert_times<duration>(TIME_VAR(construct_prepare)))
               << std::make_pair("time_construct_loop", time::convert_times<duration>(TIME_VAR(construct_loop)))
               << std::make_pair("time_construct_cleanup", time::convert_times<duration>(TIME_VAR(construct_cleanup)));
    }

    void reset() {
        reset_count_max_interchange();
        reset_count_min_interchange();
        reset_count_min_slide();
        reset_count_max_slide();
        reset_count_cancellation();
        reset_count_anticancellation();
        reset_count_anticancellation_iterations();
        reset_count_do_injury();
        reset_count_do_fatality();
        reset_count_do_scare();
        reset_count_undo_injury();
        reset_count_undo_fatality();
        reset_count_undo_scare();
        reset_time_max_interchange();
        reset_time_min_interchange();
        reset_time_min_slide();
        reset_time_max_slide();
        reset_time_cancellation();
        reset_time_anticancellation();
        reset_time_max_increase();
        reset_time_max_decrease();
        reset_time_anticancellation_dict();
        reset_time_do_injury();
        reset_time_do_fatality();
        reset_time_do_scare();
        reset_time_undo_injury();
        reset_time_undo_fatality();
        reset_time_undo_scare();
        reset_time_load_stacks();
        reset_time_cut_preprocess();
        reset_time_cut_postprocess();
        reset_time_glue_preprocess();
        reset_time_glue_postprocess();
        reset_time_construct();
        reset_time_construct_prepare();
        reset_time_construct_loop();
        reset_time_construct_cleanup();
    }
};

extern persistence_statistics persistence_stats; 

#define PERSISTENCE_STAT(name, sign) persistence_stats.increment_##name<sign>()
#define PERSISTENCE_STAT_DEC(name, sign) persistence_stats.decrement_##name<sign>()

#define TIME_STAT(name, sign, val) persistence_stats.time_##name<sign>(val)
#define TIME_BEGIN(name) const auto time_begin_v_##name = time::time_now()
#define TIME_END(name, sign) const auto time_end_v_##name = time::time_now(); \
                             TIME_STAT(name, sign, time::time_diff(time_begin_v_##name, time_end_v_##name))

class dictionary_statistics {
    DEF_TIME_VAR_AND_FUNC(contains);
    DEF_TIME_VAR_AND_FUNC(insert);
    DEF_TIME_VAR_AND_FUNC(erase);
    DEF_TIME_VAR_AND_FUNC(next);
    DEF_TIME_VAR_AND_FUNC(previous);
    DEF_TIME_VAR_AND_FUNC(join);
    DEF_TIME_VAR_AND_FUNC(cut);

public:
    template<typename duration = std::chrono::duration<double, std::milli>>
    void write_statistics(std::ostream &stream) const {
        csv_writer writer;
        write_statistics<duration>(writer);
        writer.write_to_stream_and_reset(stream);
    }
    template<typename duration = std::chrono::duration<long, std::nano>>
    void write_statistics(csv_writer& writer) const {
        writer << std::make_pair("time_contains",
                                 std::chrono::duration_cast<duration>(TIME_VAR(contains)[detail::sign_to_index(1)]))
               << std::make_pair("time_insert",
                                 std::chrono::duration_cast<duration>(TIME_VAR(insert)[detail::sign_to_index(1)]))
               << std::make_pair("time_erase",
                                 std::chrono::duration_cast<duration>(TIME_VAR(erase)[detail::sign_to_index(1)]))
               << std::make_pair("time_next",
                                 std::chrono::duration_cast<duration>(TIME_VAR(next)[detail::sign_to_index(1)]))
               << std::make_pair("time_previous",
                                 std::chrono::duration_cast<duration>(TIME_VAR(previous)[detail::sign_to_index(1)]))
               << std::make_pair("time_join",
                                 std::chrono::duration_cast<duration>(TIME_VAR(join)[detail::sign_to_index(1)]))
               << std::make_pair("time_cut",
                                 std::chrono::duration_cast<duration>(TIME_VAR(cut)[detail::sign_to_index(1)]));
    }

    void reset() {
        reset_time_contains();
        reset_time_insert();
        reset_time_erase();
        reset_time_next();
        reset_time_previous();
        reset_time_join();
        reset_time_cut();
    }
};

extern dictionary_statistics dictionary_stats;

#define DICT_TIME_STAT(name, val) dictionary_stats.time_##name<1>(val);
#define DICT_TIME_BEGIN(name) const auto time_begin_v_##name = time::time_now();
#define DICT_TIME_END(name) const auto time_end_v_##name = time::time_now(); \
                            DICT_TIME_STAT(name, time::time_diff(time_begin_v_##name, time_end_v_##name))

} // End of namespace bananas
