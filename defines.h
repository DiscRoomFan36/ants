
#ifndef DEFINES_H_
#define DEFINES_H_



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



#endif // DEFINES_H_
