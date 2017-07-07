#pragma once

#ifdef PAJLADA_SETTINGS_DEBUG

#include <iostream>

#define PS_DEBUG(x)                  \
    do {                             \
        std::cout << x << std::endl; \
    } while (0);
#else
#define PS_DEBUG(x)
#endif
