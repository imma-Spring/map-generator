#include "continent.h"
#include "common.h"
#include "open-simplex-noise.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float distance(float x1, float y1, float x2, float y2) {
  float deltaX = x1 - x2;
  float deltaY = y1 - y2;
  return sqrt(deltaX * deltaX + deltaY * deltaY);
}

static int closestDist(float x, float y, Vector points[], size_t length) {
  float closestDist = FLT_MAX;
  int closestIndex = -1;
  for (size_t i = 0; i < length; ++i) {
    float d = distance(x, y, points[i].x, points[i].y);
    if (closestDist < d) {
      closestDist = d;
      closestIndex = i;
    }
  }
  return closestIndex;
}

static float max(float a, float b) { return a >= b ? a : b; }

static float min(float a, float b) { return a <= b ? a : b; }

void generateVoronoiNoise(float map[WINDOW_WIDTH][WINDOW_HEIGHT],
                          Vector layerPoints[], const float index,
                          const size_t length, struct osn_context *ctx,
                          const float bias_scale, const float rate) {
  printf("generateVoronoiNoise called for index: %f, length: %zu\n", index,
         length);
  Vector offset;
  offset.x = (((float)(rand()) / RAND_MAX) * 20000) - 10000;
  offset.y = (((float)(rand()) / RAND_MAX) * 20000) - 10000;

  for (size_t i = 0; i < length; ++i) {
    Vector point = layerPoints[i];
    float r = (SIZE_MODIFIER * N_LAYERS / index);
    int x0 = (int)max(0, point.x - r);
    int y0 = (int)max(0, point.y - r);
    int xf = (int)min(WINDOW_WIDTH - 1, point.x + r);
    int yf = (int)min(WINDOW_HEIGHT - 1, point.y + r);
    for (int x = x0; x <= xf; ++x) {
      for (int y = y0; y <= yf; ++y) {
        if (distance(x, y, point.x, point.y) > r)
          continue;
        // if (i != closestDist(x, y, layerPoints, length))
        //   continue;
        float noiseFactor = open_simplex_noise3(ctx, x * bias_scale + offset.x,
                                                y * bias_scale + offset.y, i) *
                            1.5;
        float inverDistanceValue =
            (r == 0) ? 0 : r - distance(x, y, point.x, point.y) / r;
        map[x][y] += ((inverDistanceValue + noiseFactor) * rate);
      }
    }
  }
}

void relaxPoints(Vector layerPoints[], const size_t length) {
  Vector *newPoints = (Vector *)calloc(length, sizeof(Vector));
  if (newPoints == NULL) {
    perror("Failed to allocate memory ofr newPoints");
    return;
  }
  int *counts = (int *)calloc(length, sizeof(int));
  if (counts == NULL) {
    perror("Failed to allocate memory for counts");
    free(newPoints);
    return;
  }

  for (size_t i = 0; i < length; ++i) {
    newPoints[i].x = 0;
    newPoints[i].y = 0;
    counts[i] = 0;
  }

  for (size_t x = 0; x < WINDOW_WIDTH; ++x) {
    for (size_t y = 0; y < WINDOW_HEIGHT; ++y) {
      float closestD = FLT_MAX;
      size_t closestIndex = 0;

      for (size_t i = 0; i < length; i++) {
        float d = distance(x, y, layerPoints[i].x, layerPoints[i].y);
        if (d < closestD) {
          closestD = d;
          closestIndex = i;
        }
      }

      newPoints[closestIndex].x += x;
      newPoints[closestIndex].y += y;
      counts[closestIndex]++;
    }
  }

  for (size_t i = 0; i < length; ++i) {
    if (counts > 0) {
      newPoints[i].x /= counts[i];
      newPoints[i].y /= counts[i];
    } else {
      newPoints[i].x = layerPoints[i].x;
      newPoints[i].y = layerPoints[i].y;
    }
    layerPoints[i].x =
        LERP(layerPoints[i].x, newPoints[i].x, MOVE_SPEED / length);
    layerPoints[i].y =
        LERP(layerPoints[i].y, newPoints[i].y, MOVE_SPEED / length);
  }
  free(newPoints);
  free(counts);
}
