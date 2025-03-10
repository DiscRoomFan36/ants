#include <stdlib.h>
#include <stdio.h>

// these are system librarys for me
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "ints.h"
// #include "hashmap.h"
#include "dynamic_array.h"

#include "noise.cpp"



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


// number of ants the spawner spawns
#define SPAWNER_MAX_ANTS 400

// in units
#define SPAWNER_RADIUS 2.5
// in units
#define ANT_RADIUS 0.2
// relative to size of ant image, just a random number
#define ANT_SCALE (ANT_RADIUS/25.0f)

// Image from Freeimages.com
#define ANT_ART "Ant_clip_art_small.png"


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
    Ant *items;
    u64 count;
    u64 capacity;

    // in units, where the spawner is located at
    Vector2 position;
    // in seconds
    float last_spawn_time;
} Ant_Spawner;


#define VEC2_Fmt "(%.2f, %.2f)"
#define VEC2_Arg(vec2) vec2.x, vec2.y
// Example: printf("my_vec = "VEC2_Fmt"\n", VEC2_Arg(my_vec));

// angle in RAD to Unit vector
Vector2 Vector2AngleToVector(float angle) {
    return {cosf(angle), sinf(angle)};
}

// returns a random unit vector
Vector2 Vector2Unit(void) {
    return Vector2AngleToVector(randf() * 2 * PI);
}

inline Rectangle operator * (const Rectangle& lhs, const float& rhs) {
    Rectangle result = {lhs.x * rhs, lhs.y * rhs, lhs.width * rhs, lhs.height * rhs};
    return result;
}

// draw a texture centered at position, with scale, rotation (in RAD) and tint
void DrawTextureAt(Texture texture, Vector2 position, float scale, float rotation, Color tint) {
    Rectangle sourceRec  = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest_Rec = {
        position.x,
        position.y,
        (float)texture.width*scale,
        (float)texture.height*scale,
    };
    Vector2 origin = {texture.width  * 0.5f * scale,
                      texture.height * 0.5f * scale};

    rotation *= RAD2DEG; // convert to DEG

    DrawTexturePro(texture, sourceRec, dest_Rec, origin, rotation, tint);
}


int main(void) {
    // setup environment

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
        // (WIDTH/2) * PIXELS_TO_UNITS,
        // (HEIGHT/2) * PIXELS_TO_UNITS,
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
    Texture ant_texture = LoadTexture(ANT_ART);

    bool paused = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        // if the dt gets too big, dont freak out, just use the normal step
        if (dt > 0.25) dt = 1.0f/60.0f;

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (paused) dt = 0;

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


        // spawn an ant maybe
        ant_spawner.last_spawn_time += dt;
        while (ant_spawner.last_spawn_time > TIME_TO_SPAWN) {
            ant_spawner.last_spawn_time -= TIME_TO_SPAWN;

            if (ant_spawner.count >= SPAWNER_MAX_ANTS) continue;

            // spawn this ant around the spawner
            float spawn_angle = randf() * 2 * PI;
            Vector2 spawn_vector = Vector2AngleToVector(spawn_angle);
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

            float random_noise = get_noise(&ant->noise, dt);

            float t = dt;
            // acceleration
            Vector2 a = Vector2Zero();
            Vector2 s0 = ant->position;
            Vector2 u = ant->velocity;

            // repel ants from the edge
            if (s0.x                  < bounding_box.x)      { a.x += ANT_SPEED; }
            if (s0.x + bounding_box.x > bounding_box.width)  { a.x -= ANT_SPEED; }
            if (s0.y                  < bounding_box.y)      { a.y += ANT_SPEED; }
            if (s0.y + bounding_box.y > bounding_box.height) { a.y -= ANT_SPEED; }

            // give the ants some movement in the direction their already going, 
            float heading = atan2(u.y, u.x);
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

                // Draw ants
                for (size_t i = 0; i < ant_spawner.count; i++) {
                    Ant *ant = &ant_spawner.items[i];

                    DrawCircleV(ant->position * UNITS_TO_PIXELS, ANT_RADIUS * UNITS_TO_PIXELS, RED);
                    // shows where it would be 1 sec in future, with no acc
                    DrawLineV(ant->position * UNITS_TO_PIXELS, (ant->position + ant->velocity) * UNITS_TO_PIXELS, BLUE);

                    float rotation = atan2(ant->velocity.y, ant->velocity.x);
                    rotation += PI/2; // extra 90 for rotating image

                    DrawTextureAt(ant_texture, ant->position * UNITS_TO_PIXELS, ANT_SCALE * UNITS_TO_PIXELS, rotation, RED);
                }


                // draw spawner
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS       * UNITS_TO_PIXELS, YELLOW);
                DrawCircleV(ant_spawner.position * UNITS_TO_PIXELS, SPAWNER_RADIUS * 0.9 * UNITS_TO_PIXELS, ORANGE);

            EndMode2D();


            // draw debug text
            char text_buf[64];
            sprintf(text_buf, "Num Ants %zu", ant_spawner.count);
            int text_width = MeasureText(text_buf, FONT_SIZE);
            DrawText(text_buf, WIDTH - text_width - 10, 10, FONT_SIZE, GREEN);
            // for (size_t i = 0; i < ant_spawner.count; i++) {
            //     Ant *ant = &ant_spawner.items[i];

            //     #define FONT_SIZE 25
            //     char text_buf[64];
            //     sprintf(text_buf, "Ant %zu: "VEC2_Fmt"", i, VEC2_Arg(ant->velocity));
            //     int text_width = MeasureText(text_buf, FONT_SIZE);
            //     DrawText(text_buf, WIDTH - text_width - 10, 10 + i*FONT_SIZE, FONT_SIZE, BLACK);
            // }


            DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadTexture(ant_texture);
    CloseWindow();

    da_free(&ant_spawner);

    return 0;
}
