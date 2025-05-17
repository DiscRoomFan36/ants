
#include "ant.h"

#include "dynamic_array.h"
#include "ints.h"

#include "defines.h"
#include "common.h"
#include "raylib_extentions.h"

#include "cells.h"
#include "noise.h"



// never returns a value over 2
local inline float pheromone_activator_function(float x) {
    if (x <= 0) return 0;
    if (x <= 1) return x;
    return (-1/x) + 2;
}


// for now just makes a force that directs the ant away from clusters of pheromone
Vector2 ant_calculate_pheromone_direction(Map *map, Ant ant, bool debug_draw) {
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



void update_ants(Ant_Spawner *spawner, f32 dt) {
    // update ants!
    for (size_t i = 0; i < spawner->ant_array.count; i++) {
        Ant *ant = &spawner->ant_array.items[i];

        // check if the ant is within the spawner, to remove it.
        if (Vector2DistanceSqr(ant->position, spawner->position) < SPAWNER_RADIUS*SPAWNER_RADIUS) {
            // remove the ant with STAMP and remove, dec i
            da_stamp_and_remove(&spawner->ant_array, i);
            i -= 1;
            continue;
        }

        // -------------------------------------
        //               Move Ant
        // -------------------------------------
        {
            // THINK is this the place to put this? its only used in one place?
            // but... im trying to remove ant stuff from the inner parts...
            float random_noise = get_noise(&ant->noise, dt);

            float t = dt;
            Vector2 a = Vector2Zero();
            Vector2 s0 = ant->position;
            Vector2 u = ant->velocity;

            // repel ants from the edge
            if (s0.x                           < spawner->bounding_box.x)      { a.x += ANT_SPEED; }
            if (s0.x + spawner->bounding_box.x > spawner->bounding_box.width)  { a.x -= ANT_SPEED; }
            if (s0.y                           < spawner->bounding_box.y)      { a.y += ANT_SPEED; }
            if (s0.y + spawner->bounding_box.y > spawner->bounding_box.height) { a.y -= ANT_SPEED; }

            // give the ants some movement in the direction their already going,
            float heading = Vector2VectorToAngle(u);
            heading += random_noise * PI * (ANT_HEADING_VARIANCE/100.0f);
            a += Vector2AngleToVector(heading) * ANT_SPEED;

            // add some drag force!
            a -= (u * ANT_DRAG);

            // run away from pheromones!
            a += ant_calculate_pheromone_direction(&spawner->pheromone_map, *ant);

            // v = u + at
            Vector2 v = u + (a*t);
            // s = ut + (1/2)at^2
            Vector2 s = (u*t) + (a * (0.5*t*t));

            ant->position += s;
            ant->velocity = v;
        }


        // -------------------------------------
        //           Pheromone stuff
        // -------------------------------------

        // the cell the ant is over.
        Cell *cell = get_cell_at(&spawner->pheromone_map, ant->position);
        cell->pheromone_level += PHEROMONE_PER_SECOND * dt;
        // TODO do we cap this? or just let the decay do its thing

        // now the hard part... we need to get all the pheromones in a vision cone
        // Vector2 d = ant_calculate_pheromone_direction(&pheromone_map, *ant);
    }
}
