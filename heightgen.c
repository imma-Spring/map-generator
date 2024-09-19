#include "heightgen.h"
#include "common.h"
#include "open-simplex-noise.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float function(float x) {
  return exp(-(pow(x, 5))); /*return 1.0f / (10.f + x);*/
}
static void genGradients(Vector **gradients, float **heightMap,
                         double amplitude, double frequency, Vector offset,
                         struct osn_context *ctx) {
  float delta = (0.00001);
  for (size_t x = 0; x < WINDOW_WIDTH; ++x) {
    for (size_t y = 0; y < WINDOW_HEIGHT; ++y) {
      float newX = (x + offset.x) * SCALE / frequency;
      float newY = (y + offset.y) * SCALE / frequency;
      float p1 = open_simplex_noise2(ctx, newX, newY) * amplitude;
      float px = open_simplex_noise2(ctx, newX + delta, newY) * amplitude;
      float py = open_simplex_noise2(ctx, newX, newY + delta) * amplitude;
      float dx = (px + amplitude) / 2 - (p1 + amplitude) / 2;
      float dy = (py + amplitude) / 2 - (p1 + amplitude) / 2;
      gradients[x][y].x += dx / delta;
      gradients[x][y].y += dy / delta;
      float grad = sqrt(gradients[x][y].x * gradients[x][y].x +
                        gradients[x][y].y * gradients[x][y].y);
      (heightMap)[x][y] += p1 * function(grad);
    }
  }
}

static void init_vector_array(Vector ***arr) {
  *arr = (Vector **)calloc(WINDOW_WIDTH, sizeof(Vector *));
  if (*arr == NULL) {
    fprintf(stderr, "Memory allocation failed for gradients array.\n");
    exit(EXIT_FAILURE);
  }
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    (*arr)[i] = (Vector *)calloc(WINDOW_HEIGHT, sizeof(Vector));
    if ((*arr)[i] == NULL) {
      fprintf(stderr, "Memory allocation failed for gradients row %zu.\n", i);
      exit(EXIT_FAILURE);
    }
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      (*arr)[i][j].x = 0;
      (*arr)[i][j].y = 0;
    }
  }
}

void heightMapGen(float **heightMap, struct osn_context *ctx) {
  printf("0\n");
  Vector **gradients;
  printf("1\n");
  init_vector_array(&gradients);
  printf("2\n");
  double amplitude = 1;
  double frequency = 1;
  for (size_t o = 0; o < OCTAVES; ++o) {
    Vector offset = {RAND_IN_RANGE(-10000, 10000),
                     RAND_IN_RANGE(-10000, 10000)};
    printf("%zu\n", o);
    genGradients(gradients, heightMap, amplitude, frequency, offset, ctx);
    printf("%zu\n", o);
    amplitude *= PERSISTENCE;
    frequency *= LACUNARITY;
  }
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    free(gradients[i]);
  }
  free(gradients);
}
