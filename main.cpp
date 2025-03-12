#include <stdlib.h>
#include <stdio.h>

#include "ints.h"
// #include "hashmap.h"
#include "dynamic_array.h"

#include "common.h"
#include "raylib_extentions.h"

#include "noise.cpp"
#include "cells.cpp"
#include "ant.cpp"



// 10 units for every pixel, and just let the camera do its own thing
#define UNITS_TO_PIXELS 10
#define PIXELS_TO_UNITS (1.0f/UNITS_TO_PIXELS)


#define FACTOR 60
// in pixels
#define WIDTH  (16 * FACTOR)
// in pixels
#define HEIGHT ( 9 * FACTOR)

// in pixels
#define FONT_SIZE 25

// Image from Freeimages.com
#define ANT_ART_PATH "Ant_clip_art_small.png"


// in units
#define SPAWNER_RADIUS 2.5
// in units
#define ANT_RADIUS 0.2
// relative to size of ant image, just a random number
#define ANT_SCALE (ANT_RADIUS/25.0f)


// number of ants the spawner spawns
#define SPAWNER_MAX_ANTS 400

// How many ants the spawner spawns per second
#define SPAWNER_ANTS_PER_SECOND 5
#define TIME_TO_SPAWN (1.0f / SPAWNER_ANTS_PER_SECOND)

// in units per second
#define ANT_SPEED 3

// how much the previous velocity is negated every second.
// aka it add a backwards vector to the acceleration
#define ANT_DRAG 0.9

// vary the heading a little bit, in % of total circle
#define ANT_HEADING_VARIANCE 50

// TODO change the camera to fit this, not the box
// in pixels, from the edge
#define BOUNDING_BOX_PADDING 20

// how many seconds until pheromone is half the amount it was
#define PHEROMONE_HALF_LIFE_TIME 3
#define PHEROMONE_DECAY_EIGENVALUE (1.0f/PHEROMONE_HALF_LIFE_TIME)
// how much pheromone is placed per second
#define PHEROMONE_PER_SECOND 1




void key_toggle_setting(bool *setting, int key) {
    if (IsKeyPressed(key)) *setting = !(*setting);
}


int main(void) {
    // setup environment

    Map pheromone_map = {};

    // make sure this is in whole units
    Rectangle bounding_box = {
        0, 0,
        (int) (WIDTH  * PIXELS_TO_UNITS),
        (int) (HEIGHT * PIXELS_TO_UNITS),
    };

    Ant_Spawner ant_spawner = {};
    ant_spawner.position = {
        bounding_box.width  / 2,
        bounding_box.height / 2,
    };


    // setup window
    InitWindow(WIDTH, HEIGHT, "Ants");
    SetTargetFPS(60);

    Camera2D camera = {
        .offset = {0, 0},
        .target = {0, 0},
        .rotation = 0,
        .zoom = 1,
    };


    // setup draw stuff
    Texture ant_texture = LoadTexture(ANT_ART_PATH);

    // key toggles
    bool paused = false;
    bool move_spawner_with_mouse = false;
    bool debug_draw_ants = false;
    bool debug_draw_ant_positions = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        // if the dt gets too big, dont freak out, just use the normal step
        if (dt > 0.25) dt = 1.0f/60.0f;


        { // key toggles
            key_toggle_setting(&paused, KEY_SPACE);
            if (paused) dt = 0;

            key_toggle_setting(&move_spawner_with_mouse, KEY_M);
            if (move_spawner_with_mouse) ant_spawner.position = GetScreenToWorld2D(GetMousePosition(), camera) * PIXELS_TO_UNITS;

            key_toggle_setting(&debug_draw_ants, KEY_N);
            key_toggle_setting(&debug_draw_ant_positions, KEY_B);
        }


        { // Taken from https://www.raylib.com/examples/core/loader.html?name=core_2d_camera_mouse_zoom
            // handle mouse dragging
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 delta = GetMouseDelta();
                delta *= -1.0f/camera.zoom;
                camera.target += delta;
            }

            // Zoom based on mouse wheel
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                // Get the world point that is under the mouse
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                // Set the offset to where the mouse is
                camera.offset = GetMousePosition();

                // Set the target to match, so that the camera maps the world space point 
                // under the cursor to the screen space point under the cursor at any zoom
                camera.target = mouseWorldPos;

                // Zoom increment
                float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
                if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
                camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
            }
        }


        // handle pheromone
        for (u64 i = 0; i < pheromone_map.chunks.count; i++) {
            Chunk *chunk = &pheromone_map.chunks.items[i];
            for (u64 j = 0; j < CHUNK_X*CHUNK_Y; j++) {
                Cell *cell = &chunk->cells[j];

                if (!cell->pheromone_level) continue;

                // https://en.wikipedia.org/wiki/Exponential_decay#Solution_of_the_differential_equation
                // N(t) = N(0) * c^(-decay * t)
                cell->pheromone_level *= expf(-PHEROMONE_DECAY_EIGENVALUE * dt);
                if (cell->pheromone_level < 0.00001) cell->pheromone_level = 0;
            }
        }


        // spawn an ant maybe
        ant_spawner.last_spawn_time += dt;
        while (ant_spawner.last_spawn_time > TIME_TO_SPAWN) {
            ant_spawner.last_spawn_time -= TIME_TO_SPAWN;

            if (ant_spawner.count >= SPAWNER_MAX_ANTS) continue;

            // spawn this ant around the spawner
            Vector2 spawn_vector = Vector2AngleToVector(randf() * 2 * PI);
            Ant ant = {
                .position = ant_spawner.position + (spawn_vector * (SPAWNER_RADIUS*1.001)), // a tiny bit extra so they dont get removed,
                .velocity = spawn_vector * ANT_SPEED,
                .noise = new_noise_generator(),
            };

            da_append(&ant_spawner, ant);
        }

        // update ants!
        for (size_t i = 0; i < ant_spawner.count; i++) {
            Ant *ant = &ant_spawner.items[i];

            // check if the ant is within the spawner, to remove it.
            if (Vector2DistanceSqr(ant->position, ant_spawner.position) < SPAWNER_RADIUS*SPAWNER_RADIUS) {
                // remove the ant with STAMP and remove, dec i
                da_stamp_and_remove(&ant_spawner, i);
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
                if (s0.x                  < bounding_box.x)      { a.x += ANT_SPEED; }
                if (s0.x + bounding_box.x > bounding_box.width)  { a.x -= ANT_SPEED; }
                if (s0.y                  < bounding_box.y)      { a.y += ANT_SPEED; }
                if (s0.y + bounding_box.y > bounding_box.height) { a.y -= ANT_SPEED; }

                // give the ants some movement in the direction their already going,
                float heading = Vector2VectorToAngle(u);
                heading += random_noise * PI * (ANT_HEADING_VARIANCE/100.0f);
                a += Vector2AngleToVector(heading) * ANT_SPEED;

                // add some drag force!
                a -= (u * ANT_DRAG);

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
            Cell *cell = get_cell_at(&pheromone_map, ant->position);
            cell->pheromone_level += PHEROMONE_PER_SECOND * dt;
            // TODO do we cap this? or just let the decay do its thing

            // now the hard part... we need to get all the pheromones in a vision cone

        }


        BeginDrawing();
            ClearBackground(GRAY);

            BeginMode2D(camera);

                Rectangle pixel_bb = bounding_box * UNITS_TO_PIXELS;

                // draw bounding box and grid
                DrawRectangleRoundedLines(pixel_bb, 0.1, 1, GOLD);
                Color line_color = ColorAlpha(WHITE, 0.25);
                for (int i = pixel_bb.x + UNITS_TO_PIXELS; i < pixel_bb.x + pixel_bb.width; i += UNITS_TO_PIXELS) {
                    if (i == pixel_bb.x) continue;
                    DrawLine(i, pixel_bb.y, i, pixel_bb.y + pixel_bb.height, line_color);
                }
                for (int i = pixel_bb.y + UNITS_TO_PIXELS; i < pixel_bb.y + pixel_bb.height; i += UNITS_TO_PIXELS) {
                    if (i == pixel_bb.y) continue;
                    DrawLine(pixel_bb.x, i, pixel_bb.x + pixel_bb.width, i, line_color);
                }

                // Draw Pheromones
                for (u64 i = 0; i < pheromone_map.chunks.count; i++) {
                    Chunk chunk = pheromone_map.chunks.items[i];
                    Vector2i position = pheromone_map.positions.items[i];

                    for (u64 j = 0; j < CHUNK_X*CHUNK_Y; j++) {
                        Cell cell = chunk.cells[j];

                        if (!cell.pheromone_level) continue;

                        u8 x = j % CHUNK_X;
                        u8 y = j / CHUNK_X;

                        Vector2i cell_position = {position.x*CHUNK_X + x, position.y*CHUNK_Y + y};
                        Vector2 cell_center = {cell_position.x + 0.5f, cell_position.y + 0.5f};

                        Color color = ColorAlpha(RED, cell.pheromone_level);
                        DrawCircleV(cell_center * UNITS_TO_PIXELS, 0.5 * UNITS_TO_PIXELS, color);
                    }
                }


                // Draw ants
                for (size_t i = 0; i < ant_spawner.count; i++) {
                    Ant *ant = &ant_spawner.items[i];

                    if (debug_draw_ants) {
                        DrawCircleV(ant->position * UNITS_TO_PIXELS, ANT_RADIUS * UNITS_TO_PIXELS, RED);
                        // shows where it would be 1 sec in future, with no acc
                        DrawLineV(ant->position * UNITS_TO_PIXELS, (ant->position + ant->velocity) * UNITS_TO_PIXELS, BLUE);

                        Vector2 *cone = AntVisionCone(*ant);
                        { // sort the points counter clockwise
                            // 1. find the lowest y (bc way the screen renders)
                            int low_y = 0;
                            for (size_t i = 1; i < 3; i++) {
                                if (cone[i].y < cone[low_y].y) low_y = i;
                            }
                            // 2. move lowest to front
                            if (low_y) {
                                Vector2 tmp = cone[0];
                                cone[0] = cone[low_y];
                                cone[low_y] = tmp;
                            }
                            // 3. now swap the one with the lowest x
                            if (cone[2].x < cone[1].x) {
                                Vector2 tmp = cone[1];
                                cone[1] = cone[2];
                                cone[2] = tmp;
                            }
                        }

                        for (size_t j = 0; j < 3; j++) cone[j] *= UNITS_TO_PIXELS;
                        DrawTriangle(cone[0], cone[1], cone[2], RED);
                    }

                    float rotation = Vector2VectorToAngle(ant->velocity);
                    rotation += PI/2; // extra 90 for rotating image

                    DrawTextureAt(ant_texture, ant->position * UNITS_TO_PIXELS, ANT_SCALE * UNITS_TO_PIXELS, rotation, RED);

                }


                // draw spawner
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS       * UNITS_TO_PIXELS, YELLOW);
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS * 0.9 * UNITS_TO_PIXELS, ORANGE);

            EndMode2D();

            // {
            //     Vector2 p1 = {WIDTH/2, 0};
            //     Vector2 p2 = {0, HEIGHT};
            //     Vector2 p3 = {WIDTH, HEIGHT};

            //     DrawTriangle(p3, p1, p2, RED);
            // }


            // draw debug text
            const char *text = TextFormat("Num Ants %zu", ant_spawner.count);
            int text_width = MeasureText(text, FONT_SIZE);
            DrawText(text, WIDTH - text_width - 10, 10, FONT_SIZE, GREEN);

            if (debug_draw_ant_positions) {
                for (size_t i = 0; i < MIN(ant_spawner.count, 10); i++) {
                    Ant *ant = &ant_spawner.items[i];

                    const char *text = TextFormat("Ant: " VEC2_Fmt, VEC2_Arg(ant->position));
                    int text_width = MeasureText(text, FONT_SIZE);
                    DrawText(text, WIDTH - text_width - 10, 10 + (i+1)*FONT_SIZE, FONT_SIZE, GREEN);
                }
            }


            DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadTexture(ant_texture);
    CloseWindow();

    da_free(&ant_spawner);

    return 0;
}
