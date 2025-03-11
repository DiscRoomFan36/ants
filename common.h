
#ifndef COMMON_H_
#define COMMON_H_


#include <stdlib.h>

// Return a random float between 0 and 1
float randf(void) {
    return (float) rand() / (float) RAND_MAX;
}


#endif // COMMON_H_
