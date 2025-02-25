#include <cstdlib>
#include <iostream>

#include "utility/errors.h"

void massert_func(const char* expression_string,
                  bool expression,
                  const char* filename,
                  int linenumber,
                  const char* message) {
    if (!expression) {
        std::cerr << "Assertion failed: " << message << "\n"
                  << "Expected " << expression_string << "\n"
                  << "in file " << filename << ", line " << linenumber << "\n"; 
        abort();
    }
}