
#ifndef COMMON_H_
#define COMMON_H_


#include <stdlib.h>

// Return a random float between 0 and 1
float randf(void) {
    return (float) rand() / (float) RAND_MAX;
}

// #define MAX(a,b) \
//    ({ __typeof__ (a) _a = (a); \
//        __typeof__ (b) _b = (b); \
//      _a > _b ? _a : _b; })

// #define MIN(a,b) \
//      ({ __typeof__ (a) _a = (a); \
//          __typeof__ (b) _b = (b); \
//        _a < _b ? _a : _b; })

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


#endif // COMMON_H_
