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



// TODO refactor for this
// 10 meters for every pixel, to be changed when you zoom the camera or something
#define METER_TO_PIXEL 0.1
#define PIXEL_TO_METER 10

// TODO
// check out the camera Stuct
// Camera2D camera = {};


#define FACTOR 60
#define WIDTH  (16 * FACTOR)
#define HEIGHT ( 9 * FACTOR)

// obviously in pixels
#define FONT_SIZE 25


// number of ants the spawner spawns
#define SPAWNER_MAX_ANTS 400

// in pixels
#define SPAWNER_RADIUS 40
// in pixels
#define ANT_RADIUS 10
// in pixels, relative to size of ant image, TODO
#define ANT_SCALE 0.35

// Image from Freeimages.com
#define ANT_ART "Ant_clip_art_small.png"


// How many ants the spawner spawns per second
#define SPAWNER_ANTS_PER_SECOND 5
#define TIME_TO_SPAWN (1.0f / SPAWNER_ANTS_PER_SECOND)

// TODO change pixels per second to meters per second
// in pixels per second
#define ANT_SPEED 100

// how much the previous velocity is negated every second.
// aka it add a backwards vector to the acceleration
#define ANT_DRAG 0.9

// vary the heading a little bit, in % of total circle
#define ANT_HEADING_VARIANCE 50

// in pixels, from the edge
#define BOUNDING_BOX_PADDING 20


typedef struct Ant {
    Vector2 position;
    Vector2 velocity;

    // TODO should this be here? all the ants want some noise,
    // but one per ant? and on every single struct? Hmm...
    // maybe only make so Generator, and  randomly give them out with batching
    NoiseGenerator noise;
} Ant;

// thing that spawns ants, up to NUM_ANTS or something
typedef struct Ant_Spawner {
    Ant *items;
    u64 count;
    u64 capacity;

    // where the spawner is located at
    Vector2 position;
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

// draw a texture centered at position, with scale, rotation and tint
// rotation in RAD
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
    Ant_Spawner ant_spawner = {};
    ant_spawner.position = {WIDTH  / 2, HEIGHT / 2};

    Rectangle bounding_box = {
        BOUNDING_BOX_PADDING,
        BOUNDING_BOX_PADDING,
        WIDTH - BOUNDING_BOX_PADDING*2,
        HEIGHT - BOUNDING_BOX_PADDING*2
    };


    // setup window
    InitWindow(WIDTH, HEIGHT, "Ants");
    SetTargetFPS(60);

    Camera2D camera = {
        .offset = {0, 0},
        .rotation = 0,
        .target = {0, 0},
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

                // draw bounding box
                DrawRectangleRoundedLines(bounding_box, 0.1, 1, GOLD);
                // TODO draw line grid... where the pheromones are going to be,
                // but only in the camera view / bounding box?

                // this kinda works, dont know how though
                // rlPushMatrix();
                //     rlTranslatef(0, 25*50, 0);
                //     rlRotatef(90, 1, 0, 0);
                //     DrawGrid(100, 50);
                // rlPopMatrix();

                Color line_color = ColorAlpha(WHITE, 0.25);
                for (float i = bounding_box.x; i < bounding_box.x + bounding_box.width; i += 40) {
                    if (i == bounding_box.x) continue;
                    DrawLine(i, bounding_box.y, i, bounding_box.y + bounding_box.height, line_color);
                }
                for (float i = bounding_box.y; i < bounding_box.y + bounding_box.height; i += 40) {
                    if (i == bounding_box.y) continue;
                    DrawLine(bounding_box.x, i, bounding_box.x + bounding_box.width, i, line_color);
                }

                // Draw ants
                for (size_t i = 0; i < ant_spawner.count; i++) {
                    Ant *ant = &ant_spawner.items[i];

                    DrawCircleV(ant->position, ANT_RADIUS, RED);
                    // shows where it would be 1 sec in future, with no acc
                    DrawLineV(ant->position, ant->position + ant->velocity, BLUE);

                    float rotation = atan2(ant->velocity.y, ant->velocity.x);
                    rotation += PI/2; // extra 90 for rotating image

                    DrawTextureAt(ant_texture, ant->position, ANT_SCALE, rotation, RED);
                }


                // draw spawner
                DrawCircleV(ant_spawner.position, SPAWNER_RADIUS,     YELLOW);
                DrawCircleV(ant_spawner.position, SPAWNER_RADIUS*0.9, ORANGE);

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
