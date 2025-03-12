
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
Vector2 Vector2AngleToVector(float angle) {
    return {cosf(angle), sinf(angle)};
}

float Vector2VectorToAngle(Vector2 v) {
    return atan2(v.y, v.x);
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


typedef struct Vector2i {
    s32 x, y;
} Vector2i;

// inline Vector2i operator * (const Vector2i& lhs, const s32& rhs) {
//     return {lhs.x * rhs, lhs.y * rhs};
// }


#endif // RAYLIB_EXTENSIONS_H_
