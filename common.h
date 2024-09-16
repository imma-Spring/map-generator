#pragma once

#include <stdlib.h>

#ifndef LERP
#define LERP(a, b, t) ((a) + ((b - a) * (t)))
#endif // !LERP

#ifndef MAP
#define MAP(val, min1, max1, min2, max2)                                       \
  ((float)min2 + ((float)max2 - (float)min2) * (((float)val - (float)min1) /   \
                                                ((float)max1 - (float)min1)))
#endif // !MAP

#ifndef RAND_IN_RANGE
#define RAND_IN_RANGE(max, min) ((float)(rand()) / RAND_MAX) * (max - min) + min
#endif // DEBUG

#ifndef _VARIABLES
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SIZE_MODIFIER 30
#define N_LAYERS 10
#define MAX_ITERATIONS 100
#define N_START_POINTS 2
#define MOVE_SPEED 0.01f
#define _VARIABLES
#endif // !_VARIABLES

typedef struct {
  float x, y, z;
  float nx, ny, nz;
} Vector;
