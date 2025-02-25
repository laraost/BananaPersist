#pragma once

#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

namespace bananas {

#ifdef USE_FALLBACK_CHRONO_OPERATOR

template<std::intmax_t Num, std::intmax_t Denom>
std::ostream& operator<<(std::ostream& stream, const std::ratio<Num, Denom>&) {
    stream << "?";
    return stream;
}

template<>
inline std::ostream& operator<<(std::ostream& stream, const std::nano&) {
    stream << "n";
    return stream;
}

template<>
inline std::ostream& operator<<(std::ostream& stream, const std::micro&) {
    stream << "µ";
    return stream;
}

template<>
inline std::ostream& operator<<(std::ostream& stream, const std::milli&) {
    stream << "m";
    return stream;
}

template<>
inline std::ostream& operator<<(std::ostream& stream, const std::ratio<1,1>&) {
    return stream;
}

template<typename Rep, typename Period>
std::ostream& operator<<(std::ostream& stream, const std::chrono::duration<Rep, Period>& d) {
    stream << d.count() << Period{} << "s";
    return stream;
}
#endif

inline void print_color(std::ostream &stream, const std::string &text, int cc) {
    stream << "\033[1;" << cc << "m" << text << "\033[0m";
}

inline void print_green(std::ostream &stream, const std::string &text) {
    print_color(stream, text, 32);
}

inline void print_red(std::ostream &stream, const std::string &text) {
    print_color(stream, text, 31);
}

template<typename T>
std::ostream& operator<<(std::ostream& str, const std::array<T, 2>& vars) {
    str << vars[1] << ", " << vars[0];
    return str;
}

template<typename T>
void write_csv_line(std::ostream& stream, const T& t) {
    stream << t;
}

template<typename T1, typename... T2>
void write_csv_line(std::ostream& stream, const T1& head, const T2& ...tail) {
    stream << head << ", ";
    write_csv_line(stream, tail...);
}

class csv_writer {

public:
    constexpr static char separator[] = ", ";

    template<typename T>
    csv_writer& operator<<(std::pair<const char*, T> name_value_pair) {
        if (!empty) {
            header_stream << separator;
            value_stream << separator;
        }
        header_stream << name_value_pair.first;
        value_stream << name_value_pair.second;
        empty = false;
        return *this;
    }

    template<typename T>
    csv_writer& operator<<(std::pair<const char*, std::array<T, 2>> name_signed_value_pair) {
        if (!empty) {
            header_stream << separator;
            value_stream << separator;
        }
        header_stream << name_signed_value_pair.first << "_up" << separator
                      << name_signed_value_pair.first << "_down";
        value_stream << name_signed_value_pair.second[1] << separator
                     << name_signed_value_pair.second[0];
        empty = false;
        return *this;
    }

    void write_to_stream_and_reset(std::ostream &stream, bool write_header = true) {
        if (write_header) {
            stream << header_prefix << header_stream.str() << "\n";
        }
        stream << value_stream.str() << "\n";
        header_stream.str("");
        value_stream.str("");
        empty = true;
    }

    void set_header_prefix(const std::string &string) {
        header_prefix = string;
    }

private:
    bool empty = true;
    std::stringstream header_stream;
    std::stringstream value_stream;

    std::string header_prefix = "** ";

};

class multirow_csv_writer {

public:

    multirow_csv_writer& on_every_row(std::pair<const char*, std::string> name_value_pair) {
        every_row_prefix = name_value_pair;
        return *this;
    }

    multirow_csv_writer& new_row() {
        writers.push_back({});
        writers.back() << every_row_prefix;
        return *this;
    }

    template<typename T>
    multirow_csv_writer& operator<<(std::pair<const char*, T> name_value_pair) {
        if (writers.empty()) {
            new_row();
        }
        writers.back() << name_value_pair;
        return *this;
    }

    void write_to_stream_and_reset(std::ostream &stream, bool write_header = true) {
        if (writers.empty()) {
            return;
        }
        writers[0].write_to_stream_and_reset(stream, write_header);
        for (size_t idx = 1; idx < writers.size(); ++idx) {
            writers[idx].write_to_stream_and_reset(stream, false);
        }
        writers.clear();
    }
    
private:
    std::vector<csv_writer> writers;
    std::pair<const char*, std::string> every_row_prefix;

};

}
