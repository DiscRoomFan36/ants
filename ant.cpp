
#include "ints.h"

#include "common.h"
#include "raylib_extentions.h"

#include "cells.cpp"
#include "noise.cpp"


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


// returns a pointer to a static array, of the three points making up the ants vision cone
Vector2 *AntVisionCone(Ant ant) {
    static Vector2 points[3];

    points[0] = ant.position;

    // in units, how far the vision cone extends
    #define ANT_VISION_DISTANCE 3
    // in RAD, the angle of the vision cone
    #define ANT_VISION_CONE_ANGLE (PI / 3)

    // Vector2 unit = Vector2Normalize(ant->velocity);
    float heading = Vector2VectorToAngle(ant.velocity);

    points[1] = ant.position + (Vector2AngleToVector(heading - (ANT_VISION_CONE_ANGLE/2)) * ANT_VISION_DISTANCE);
    points[2] = ant.position + (Vector2AngleToVector(heading + (ANT_VISION_CONE_ANGLE/2)) * ANT_VISION_DISTANCE);

    return points;
}


/* Go code i wrote once
// https://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
func Draw_Triangle[T Vector.Number](img *Image, p1, p2, p3 Vector.Vector2[T], c Color) {
	v1 := Vector.Transform[T, int](p1)
	v2 := Vector.Transform[T, int](p2)
	v3 := Vector.Transform[T, int](p3)

	sortVerticesAscendingByY := func() {
		// bubble sort, i wish go had a better way to do this, like passing a function into the sort interface, (v1, v2, v3 used to be an array)
		if v1.Y > v2.Y {
			v1, v2 = v2, v1
		}
		if v2.Y > v3.Y {
			v2, v3 = v3, v2
		}
		if v1.Y > v2.Y {
			v1, v2 = v2, v1
		}

		if !(v1.Y <= v2.Y && v2.Y <= v3.Y) {
			log.Fatalf("Did not sort vertex's correctly, got %v, %v, %v\n", v1, v2, v3)
		}
	}
	sortVerticesAscendingByY()

	draw_line := func(x0, x1, y int) {
		if !(0 <= y && y < img.Height) {
			return
		}
		for x := max(min(x0, x1), 0); x < min(max(x0, x1), img.Width); x++ {
			img.put_pixel(x, y, c)
		}
	}
	fillBottomFlatTriangle := func(v1, v2, v3 Vector.Vector2[int]) {
		inv_slope_1 := float32(v2.X-v1.X) / float32(v2.Y-v1.Y)
		inv_slope_2 := float32(v3.X-v1.X) / float32(v3.Y-v1.Y)

		cur_x_1 := float32(v1.X)
		cur_x_2 := float32(v1.X)

		for scan_line_Y := v1.Y; scan_line_Y <= v2.Y; scan_line_Y++ {
			draw_line(int(cur_x_1), int(cur_x_2), scan_line_Y)
			cur_x_1 += inv_slope_1
			cur_x_2 += inv_slope_2
		}
	}
	fillTopFlatTriangle := func(v1, v2, v3 Vector.Vector2[int]) {
		inv_slope_1 := float32(v3.X-v1.X) / float32(v3.Y-v1.Y)
		inv_slope_2 := float32(v3.X-v2.X) / float32(v3.Y-v2.Y)

		cur_x_1 := float32(v3.X)
		cur_x_2 := float32(v3.X)

		for scan_line_Y := v3.Y; scan_line_Y > v1.Y; scan_line_Y-- {
			draw_line(int(cur_x_1), int(cur_x_2), scan_line_Y)
			cur_x_1 -= inv_slope_1
			cur_x_2 -= inv_slope_2
		}
	}

	if v2.Y == v3.Y {
		fillBottomFlatTriangle(v1, v2, v3)
	} else if v1.Y == v2.Y {
		fillTopFlatTriangle(v1, v2, v3)
	} else {

		// I did some rearranging here
		v4 := Vector.Vector2[int]{
			// X: int(float32(v1.X) + float32(v2.Y-v1.Y)/float32(v3.Y-v1.Y)*float32(v3.X-v1.X)),
			X: (((v2.Y - v1.Y) * (v3.X - v1.X)) + (v1.X * (v3.Y - v1.Y))) / (v3.Y - v1.Y),
			Y: v2.Y,
		}

		fillBottomFlatTriangle(v1, v2, v4)
		fillTopFlatTriangle(v2, v4, v3)
	}
}
*/

Vector2 AntCalculatePheromoneDirection(Map *map, Ant ant) {
    Vector2 *cone = AntVisionCone(ant);

    // get all squares intersecting with this


    return Vector2();
}


