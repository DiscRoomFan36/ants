
#include "ints.h"

#include "defines.h"
#include "common.h"
#include "raylib_extentions.h"

#include "cells.cpp"
#include "noise.cpp"


typedef struct Ant {
    // in units
    Vector2 position;
    // in units per second
    Vector2 velocity;

    // TODO should this be here? all the ants want some noise,
    // but one per ant? and on every single struct? Hmm...
    // maybe only make so Generator, and randomly give them out with batching
    NoiseGenerator noise;
} Ant;

// thing that spawns ants, up to NUM_ANTS or something
typedef struct Ant_Spawner {
    struct {
        Ant *items;
        u64 count;
        u64 capacity;
    } ant_array;

    // in units, where the spawner is located at
    Vector2 position;
    // in seconds
    float last_spawn_time;
} Ant_Spawner;


// never returns a value over 2
inline float pheromone_activator_function(float x) {
    if (x <= 0) return 0;
    if (x <= 1) return x;
    return (-1/x) + 2;
}


// for now just makes a force that directs the ant away from clusters of pheromone
Vector2 ant_calculate_pheromone_direction(Map *map, Ant ant, bool debug_draw = false) {
    // the cone was a bad idea, instead just get every-thing within a circle radius

    #define ANT_VISION_RADIUS 3

    Vector2 force = Vector2Zero();

    int start_x = floorf(ant.position.x - ANT_VISION_RADIUS);
    int   end_x = ceilf (ant.position.x + ANT_VISION_RADIUS);

    int start_y = floorf(ant.position.y - ANT_VISION_RADIUS);
    int   end_y = ceilf (ant.position.y + ANT_VISION_RADIUS);

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            Vector2 pos = {(float)x, (float)y};
            Vector2 center = pos + (Vector2){0.5, 0.5};

            float dist_sqr = Vector2DistanceSqr(ant.position, center);
            if (!(dist_sqr < ANT_VISION_RADIUS*ANT_VISION_RADIUS)) continue;

            if (debug_draw) {
                Color color = ColorAlpha(ORANGE, 0.3);
                DrawRectangleV(pos * UNITS_TO_PIXELS, {UNITS_TO_PIXELS, UNITS_TO_PIXELS}, color);
            }

            Cell *cell = get_cell_at_or_null(map, center);
            if (!cell) continue;

            float active = pheromone_activator_function(cell->pheromone_level);
            if (!active) continue;

            float inv_dist = ANT_VISION_RADIUS - sqrtf(dist_sqr); // 1 / sqrtf(dist_sqr);
            Vector2 direction = Vector2Normalize(center - ant.position);

            force -= (direction * active * inv_dist);
        }
    }

    if (debug_draw) {
        DrawCircleLinesV(ant.position * UNITS_TO_PIXELS, ANT_VISION_RADIUS*UNITS_TO_PIXELS, BLUE);
    }

    return force;
}




void update_ants(Ant_Spawner *spawner) {
    assert(False && "TODO");
}

void render_ants(Ant_Spawner *spawner) {
    assert(False && "TODO");
}
