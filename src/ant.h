
#ifndef ANT_H_
#define ANT_H_


#include "noise.h"
#include "cells.h"

#include "raylib_extentions.h"

#include "ints.h"

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

// thing that spawns ants, and 
typedef struct Ant_Spawner {
    struct {
        Ant *items   = NULL;
        u64 count    = 0;
        u64 capacity = 0;
    } ant_array;

    // in units, where the spawner is located at
    Vector2 position;
    // in seconds
    f32 last_spawn_time = 0;

    // the area the ands can live in.
    Rectangle bounding_box;
    Map pheromone_map;
} Ant_Spawner;



// for now just makes a force that directs the ant away from clusters of pheromone
Vector2 ant_calculate_pheromone_direction(Map *map, Ant ant, bool debug_draw = false);

void update_ants(Ant_Spawner *spawner, f32 dt);


#endif // ANT_H_

