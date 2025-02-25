#pragma once

#ifndef NDEBUG
    #include <iostream>
    #define DEBUG_MSG(message) std::cout << message << "\n"
#else
    #define DEBUG_MSG(message) do {} while(false)
#endif
