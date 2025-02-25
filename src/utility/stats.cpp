#include "utility/stats.h"
#include <chrono>
#include <ostream>

namespace bananas {

persistence_statistics persistence_stats{};
dictionary_statistics dictionary_stats{};

namespace time {

clock_type::time_point time_now() {
    return clock_type::now();
}

duration_type time_diff(clock_type::time_point begin, clock_type::time_point end) {
    return std::chrono::duration_cast<duration_type>(end-begin);
}

} // End of namespace time

} // End of namespace bananas
