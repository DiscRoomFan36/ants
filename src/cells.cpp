
#ifndef CELLS_CPP_
#define CELLS_CPP_


#include "cells.h"

#include <stdlib.h>

#include "dynamic_array.h"


// rounds a float down towards -inf
local constexpr s32 int_floor(float x) {
    s32 i = (s32) x;
    if (x < 0 && i != x) i--;
    return i;
}
static_assert(int_floor(-0.5) == -1);


// rounds the resulting div towards 0
local constexpr s32 div_down(int x, int y) {
    if (x >= 0) return x / y;
    else        return (x - y + 1) / y;
}
static_assert(div_down(-3, 2) == -2);


Cell *get_cell_at_or_null(Map *map, Vector2 position) {
    // find the exact cell
    s32 x = int_floor(position.x);
    s32 y = int_floor(position.y);

    s32 chunk_x = div_down(x, CHUNK_X);
    s32 chunk_y = div_down(y, CHUNK_Y);
    u32 cell_x = ((x % CHUNK_X) + CHUNK_X) % CHUNK_X; // positive modulo
    u32 cell_y = ((y % CHUNK_Y) + CHUNK_Y) % CHUNK_Y; // positive modulo

    Chunk *found_chunk = NULL;
    for (size_t i = 0; i < map->positions.count; i++) {
        Vector2i position = map->positions.items[i];

        if (position.x == chunk_x && position.y == chunk_y) {
            found_chunk = &map->chunks.items[i];
            break;
        }
    }

    if (!found_chunk) {
        return NULL;
    }

    return &found_chunk->cells[cell_x + cell_y * CHUNK_X];
}

Cell *get_cell_at(Map *map, Vector2 position) {
    Cell *cell_or_null = get_cell_at_or_null(map, position);
    if (cell_or_null) return cell_or_null;

    s32 x = int_floor(position.x);
    s32 y = int_floor(position.y);

    s32 chunk_x = div_down(x, CHUNK_X);
    s32 chunk_y = div_down(y, CHUNK_Y);
    u32 cell_x = ((x % CHUNK_X) + CHUNK_X) % CHUNK_X; // positive modulo
    u32 cell_y = ((y % CHUNK_Y) + CHUNK_Y) % CHUNK_Y; // positive modulo

    Chunk new_chunk = {};
    Vector2i new_position = {chunk_x, chunk_y};

    da_append(&map->chunks, new_chunk);
    da_append(&map->positions, new_position);

    return &map->chunks.items[map->chunks.count-1].cells[cell_x + cell_y * CHUNK_X];
}


#endif // CELLS_CPP_
