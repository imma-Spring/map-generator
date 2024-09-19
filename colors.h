#pragma once
#include "common.h"

typedef struct {
  float r, g, b;
} Color;

void addColors();
Color color(int r, int g, int b);
void initializeHeight(float **heights);
Color getColor(float *heights, float value, float sealevel);
