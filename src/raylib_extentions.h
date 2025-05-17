
// raylib and some helpers I made

#ifndef RAYLIB_EXTENSIONS_H_
#define RAYLIB_EXTENSIONS_H_


// these are system librarys for me
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "common.h"


#define VEC2_Fmt "(%.2f, %.2f)"
#define VEC2_Arg(vec2) vec2.x, vec2.y
// Example: printf("my_vec = "VEC2_Fmt"\n", VEC2_Arg(my_vec));


// angle in RAD to Unit vector
Vector2 Vector2AngleToVector(float angle);

float Vector2VectorToAngle(Vector2 v);

// returns a random unit vector
Vector2 Vector2Unit(void);

Rectangle operator * (const Rectangle& lhs, const float& rhs);

// draw a texture centered at position, with scale, rotation (in RAD) and tint
void DrawTextureAt(Texture texture, Vector2 position, float scale, float rotation, Color tint);


typedef struct Vector2i {
    s32 x, y;
} Vector2i;

// Vector2i operator * (const Vector2i& lhs, const s32& rhs) {
//     return {lhs.x * rhs, lhs.y * rhs};
// }


#endif // RAYLIB_EXTENSIONS_H_
