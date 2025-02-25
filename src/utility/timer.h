#pragma once

#include <chrono>

// A simple timer based on std::chrono's clocks
// 'T' is 
template<typename T, typename clock_type = std::chrono::high_resolution_clock>
class Timer {
public:
        using time_point_type = typename clock_type::time_point;
        using duration_type = T;
        using time_rep = typename T::rep;

private:
        static constexpr auto default_timeout = std::chrono::duration_cast<T>(std::chrono::seconds{10});

public:
        Timer(time_rep timeout = default_timeout.count()) : timeout(timeout) {
                last_time_point = clock.now();
        }

        // Restarts the timer and returns a 'time_point' representing the current time
        time_point_type restart() {
                last_time_point = clock.now();
                return last_time_point;
        }

        // Returns a 'time_point' representing the current time
        time_point_type now() const {
                return clock.now();
        }

        // Computes the time elapsed since the last call to 'restart()'
        duration_type elapsed() const {
                return std::chrono::duration_cast<T>(now() - last_time_point);
        }

        // Check if the elapsed time exceeds the timeout.
        bool timed_out() const {
                return elapsed() > timeout;
        }

        static time_rep from_seconds(long seconds) {
                return std::chrono::duration_cast<T>(std::chrono::seconds{seconds}).count();
        }

private:
        clock_type clock;
        time_point_type last_time_point;
        const time_rep timeout;

};

using default_timer = Timer<std::chrono::milliseconds>;
