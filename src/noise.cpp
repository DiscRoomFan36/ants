
#ifndef NOISE_CPP_
#define NOISE_CPP_


#include "common.h"

float clamp(float x, float lowerlimit = 0.0f, float upperlimit = 1.0f) {
    if (x < lowerlimit) return lowerlimit;
    if (x > upperlimit) return upperlimit;
    return x;
}

float smoothstep(float edge0, float edge1, float x) {
    // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0));

    return x * x * (3.0f - 2.0f * x);
}

typedef struct NoiseGenerator {
    float prev, next;
    float t;
} NoiseGenerator;

NoiseGenerator new_noise_generator(void) {
    NoiseGenerator result = {};
    result.prev = randf() * 2 - 1;
    result.next = randf() * 2 - 1;
    result.t = 0;
    return result;
}

// gets some random noise in the range -1 .. 1
float get_noise(NoiseGenerator *noise, float dt) {
    noise->t += dt;

    if (noise->t > 2) {
        noise->prev = randf() * 2 - 1;
        noise->next = randf() * 2 - 1;
        while (noise->t > 0) noise->t -= 1;

    } else if (noise->t > 1) {
        // advance the noise generator
        noise->prev = noise->next;
        noise->next = randf() * 2 - 1;
        noise->t -= 1;
    }

    return noise->prev + smoothstep(0, 1, noise->t) * (noise->next - noise->prev);
}


#endif // NOISE_CPP_
