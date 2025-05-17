
#include <stdlib.h>
#include <stdio.h>


#include "defines.h"
#include "common.h"
#include "raylib_extentions.h"


#include "ints.h"
#include "dynamic_array.h"


#include "noise.h"
#include "cells.h"
#include "ant.h"


local void key_toggle_setting(bool *setting, int key) {
    if (IsKeyPressed(key)) *setting = !(*setting);
}


int main(void) {
    // setup environment

    Ant_Spawner ant_spawner = {};

    // make sure this is in whole units
    ant_spawner.bounding_box = {
        0, 0,
        (int) (WIDTH  * PIXELS_TO_UNITS),
        (int) (HEIGHT * PIXELS_TO_UNITS),
    },

    // just put it in the center
    ant_spawner.position = {
        ant_spawner.bounding_box.width  / 2,
        ant_spawner.bounding_box.height / 2,
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
    bool debug_draw_ant_calculate_pheromone_direction = false;

    while (!WindowShouldClose()) {
        f32 dt = GetFrameTime();
        // if the dt gets too big, dont freak out, just use the normal step
        if (dt > 0.25) dt = 1.0f/60.0f;


        { // key toggles
            key_toggle_setting(&paused, KEY_SPACE);
            if (paused) dt = 0;

            key_toggle_setting(&move_spawner_with_mouse, KEY_M);
            if (move_spawner_with_mouse) ant_spawner.position = GetScreenToWorld2D(GetMousePosition(), camera) * PIXELS_TO_UNITS;

            key_toggle_setting(&debug_draw_ants, KEY_N);
            key_toggle_setting(&debug_draw_ant_positions, KEY_B);
            key_toggle_setting(&debug_draw_ant_calculate_pheromone_direction, KEY_V);
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


        // handle pheromone evaporation
        for (u64 i = 0; i < ant_spawner.pheromone_map.chunks.count; i++) {
            Chunk *chunk = &ant_spawner.pheromone_map.chunks.items[i];
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

            if (ant_spawner.ant_array.count >= SPAWNER_MAX_ANTS) continue;

            // spawn this ant around the spawner
            Vector2 spawn_vector = Vector2AngleToVector(randf() * 2 * PI);
            Ant ant = {
                .position = ant_spawner.position + (spawn_vector * (SPAWNER_RADIUS*1.001)), // a tiny bit extra so they dont get removed,
                .velocity = spawn_vector * ANT_SPEED,
                .noise = new_noise_generator(),
            };

            da_append(&ant_spawner.ant_array, ant);
        }

        update_ants(&ant_spawner, dt);


        BeginDrawing();
            ClearBackground(GRAY);

            BeginMode2D(camera);

                Rectangle pixel_bb = ant_spawner.bounding_box * UNITS_TO_PIXELS;

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
                for (u64 i = 0; i < ant_spawner.pheromone_map.chunks.count; i++) {
                    Chunk chunk = ant_spawner.pheromone_map.chunks.items[i];
                    Vector2i position = ant_spawner.pheromone_map.positions.items[i];

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
                for (size_t i = 0; i < ant_spawner.ant_array.count; i++) {
                    Ant *ant = &ant_spawner.ant_array.items[i];

                    if (debug_draw_ants) {
                        DrawCircleV(ant->position * UNITS_TO_PIXELS, ANT_RADIUS * UNITS_TO_PIXELS, RED);
                        // shows where it would be 1 sec in future, with no acc
                        DrawLineV(ant->position * UNITS_TO_PIXELS, (ant->position + ant->velocity) * UNITS_TO_PIXELS, BLUE);

                    }
                    if (debug_draw_ant_calculate_pheromone_direction) {
                        Vector2 force = ant_calculate_pheromone_direction(&ant_spawner.pheromone_map, *ant, true);
                        DrawLineV(ant->position * UNITS_TO_PIXELS, (ant->position + force) * UNITS_TO_PIXELS, GRAY);
                    }


                    float rotation = Vector2VectorToAngle(ant->velocity);
                    rotation += PI/2; // extra 90 for rotating image

                    DrawTextureAt(ant_texture, ant->position * UNITS_TO_PIXELS, ANT_SCALE * UNITS_TO_PIXELS, rotation, RED);
                }


                // draw spawner
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS       * UNITS_TO_PIXELS, YELLOW);
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS * 0.9 * UNITS_TO_PIXELS, ORANGE);

            EndMode2D();


            // draw debug text
            const char *text = TextFormat("Num Ants %zu", ant_spawner.ant_array.count);
            int text_width = MeasureText(text, FONT_SIZE);
            DrawText(text, WIDTH - text_width - 10, 10, FONT_SIZE, GREEN);

            if (debug_draw_ant_positions) {
                for (u64 i = 0; i < MIN(ant_spawner.ant_array.count, (u64)10); i++) {
                    Ant *ant = &ant_spawner.ant_array.items[i];

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

    da_free(&ant_spawner.ant_array);

    return 0;
}
