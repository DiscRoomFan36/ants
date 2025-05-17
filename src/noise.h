
#ifndef NOISE_H_
#define NOISE_H_


typedef struct NoiseGenerator {
    float prev, next;
    float t;
} NoiseGenerator;


NoiseGenerator new_noise_generator(void);

// gets some random noise in the range -1 .. 1
float get_noise(NoiseGenerator *noise, float dt);

#endif // NOISE_H_
