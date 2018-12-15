//
// Created by vladimirlesk on 14.12.18.
//

#include "util.h"

#define DEFINE_MIN_T(T) \
    T Min_ ##T(T a, T b) { \
        return a > b ? a : b; \
    }
    
    
DEFINE_MIN_T(size_t)
