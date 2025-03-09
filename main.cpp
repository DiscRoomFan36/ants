#include <stdlib.h>
#include <stdio.h>

// these are system librarys for me
#include <raylib.h>
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


#define NUM_ANTS 10

// in pixels
#define SPAWNER_RADIUS 40
// in pixels
#define ANT_RADIUS 10
// in pixels, relative to size of ant image, TODO
#define ANT_SCALE 0.35

// Image from Freeimages.com
#define ANT_ART "Ant_clip_art_small.png"


// TODO change pixels per second to meters per second
// in pixels per second
#define ANT_SPEED 100

// how much the previous velocity is negated every second.
// aka it add a backwards vector to the acceleration
#define ANT_DRAG 0.9

// vary the heading a little bit, in % of total circle
#define ANT_HEADING_VARIANCE 50


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
    ant_spawner.position.x = WIDTH  / 2;
    ant_spawner.position.y = HEIGHT / 2;


    // TODO make spawner spawn
    for (size_t i = 0; i < NUM_ANTS; i++) {
        Ant ant = {};
        ant.position = {randf() * WIDTH, randf() * HEIGHT};
        ant.velocity = Vector2Unit() * ANT_SPEED;
        ant.noise = new_noise_generator();

        da_append(&ant_spawner, ant);
    }


    // setup window
    InitWindow(WIDTH, HEIGHT, "Ants");
    SetTargetFPS(60);


    // setup draw stuff
    Texture ant_texture = LoadTexture(ANT_ART);


    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // update simulation!
        for (size_t i = 0; i < ant_spawner.count; i++) {
            Ant *ant = &ant_spawner.items[i];

            float random_noise = get_noise(&ant->noise, dt);

            float t = dt;
            // acceleration
            Vector2 a = Vector2Zero();
            Vector2 s0 = ant->position;
            Vector2 u = ant->velocity;

            // repel ants from the edge
            if (s0.x < 0)      { a.x += ANT_SPEED; }
            if (s0.x > WIDTH)  { a.x -= ANT_SPEED; }
            if (s0.y < 0)      { a.y += ANT_SPEED; }
            if (s0.y > HEIGHT) { a.y -= ANT_SPEED; }

            // give the ants some movement in the direction their already going, 
            float heading = atan2(u.y, u.x);
            heading += random_noise * PI * (ANT_HEADING_VARIANCE/100);
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


        // draw debug text
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
