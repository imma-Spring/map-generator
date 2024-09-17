#pragma once
#include "common.h"

typedef struct {
  int **erosionBrushIndicies;
  float **erosionBrushWeights;
  size_t *lengths;
} Erosion;

void erode_init(Erosion *erosion);
void free_erode(Erosion *erosion);
void erode(Erosion *erosion, float map[WINDOW_WIDTH * WINDOW_HEIGHT],
           int numIteration);
