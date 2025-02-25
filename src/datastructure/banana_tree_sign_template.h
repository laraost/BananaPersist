#include "persistence_defs.h"

#define SIGN_TEMPLATE \
    template<int sign> \
        requires sign_integral<decltype(sign), sign>

