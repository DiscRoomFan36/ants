
#ifndef CELLS_H_
#define CELLS_H_


#include "ints.h"

#include "raylib_extentions.h"


typedef struct Cell {
    float pheromone_level;
} Cell;


#define CHUNK_X 8
#define CHUNK_Y 8
static_assert(CHUNK_X <= UINT8_MAX + 1);
static_assert(CHUNK_Y <= UINT8_MAX + 1);

// TODO Kinda want to make this a template...
typedef struct Chunk {
    Cell cells[CHUNK_X * CHUNK_Y];
} Chunk;

typedef struct Chunk_Array {
    Chunk *items;
    u64 count;
    u64 capacity;
} Chunk_Array;


typedef struct Vector2i_Array {
    Vector2i *items;
    u64 count;
    u64 capacity;
} Vector2i_Array;


typedef struct Map {
    Chunk_Array chunks;         // paired array
    Vector2i_Array positions;   // paired array
} Map;



// TODO should these functions accept float vectors? dose it matter?

// returns a pointer to a cell, this might be moved in memory at some point so keep a look out.
Cell *get_cell_at_or_null(Map *map, Vector2 position);

Cell *get_cell_at(Map *map, Vector2 position);

#endif // CELLS_H_
