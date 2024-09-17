#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colors.h"
#include "common.h"
#include "continent.h"
#include "erosion.h"
#include "open-simplex-noise.h"

void normalizeMap(float map[WINDOW_WIDTH][WINDOW_HEIGHT], float *min,
                  float *max) {
  *min = FLT_MAX;
  *max = -FLT_MAX;

  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      if (map[i][j] > *max)
        *max = map[i][j];
      else if (map[i][j] < *min)
        *min = map[i][j];
    }
  }

  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      map[i][j] = MAP(map[i][j], *min, *max, 0, 1);
    }
  }
}

void drawMap(struct osn_context *ctx, float map[WINDOW_WIDTH][WINDOW_HEIGHT]) {
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
  for (int y = 0; y < WINDOW_HEIGHT; ++y) {
    for (int x = 0; x < WINDOW_WIDTH; ++x) {
      float value = map[x][y];
      glColor3f(value, value, value);
      glVertex2i(x, y);
    }
  }
  glEnd();
}

void oneDimensionalArrayToTwoDimensional(
    float oneDimensionalArray[WINDOW_WIDTH * WINDOW_HEIGHT],
    float twoDimensionalArray[WINDOW_WIDTH][WINDOW_HEIGHT]) {
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      twoDimensionalArray[i][j] = oneDimensionalArray[i + j * WINDOW_WIDTH];
    }
  }
}

void twoDimensionalArrayToOneDimensionalArray(
    float oneDimensionalArray[WINDOW_WIDTH * WINDOW_HEIGHT],
    float twoDimensionalArray[WINDOW_WIDTH][WINDOW_HEIGHT]) {
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      oneDimensionalArray[i + j * WINDOW_WIDTH] = twoDimensionalArray[i][j];
    }
  }
}

int main() {
  srand(SEED);
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  GLFWwindow *window =
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Checkerboard", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to open window\n");
    glfwTerminate();
    return -1;
  }

  Vector **points = (Vector **)calloc(N_LAYERS, sizeof(Vector *));
  if (points == NULL) {
    fprintf(stderr, "Memory allocation failed for points array.\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < N_LAYERS; ++i) {
    Vector *p = (Vector *)calloc(N_START_POINTS + i, sizeof(Vector));
    if (p == NULL) {
      fprintf(stderr, "Memmory allocation failed for layer %zu.\n", i);
      for (size_t k = 0; k < i; ++k) {
        free(points[k]);
      }
      free(points);
      exit(EXIT_FAILURE);
    }

    for (size_t j = 0; j < N_START_POINTS + i; ++j) {
      float x = (float)(rand() % (WINDOW_WIDTH));
      float y = (float)(rand() % (WINDOW_HEIGHT));
      Vector v;
      v.x = x;
      v.y = y;
      printf("%f, %f", x, y);
      p[j] = v;
    }
    points[i] = p;
  }

  int currentIteration = 0;
  float min = FLT_MIN;
  float max = FLT_MAX;
  float bias_scale = 0.0001;
  float rate = 1;
  Erosion erosion;
  erode_init(&erosion);
  struct osn_context *ctx;
  open_simplex_noise(1, &ctx);

  float map[WINDOW_WIDTH][WINDOW_HEIGHT];
  float tempMap[WINDOW_WIDTH][WINDOW_HEIGHT];
  float m[WINDOW_WIDTH * WINDOW_HEIGHT];
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      map[i][j] = 0;
    }
  }

  glfwMakeContextCurrent(window);
  glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

  while (!glfwWindowShouldClose(window)) {
    if (currentIteration < MAX_ITERATIONS) {
      for (size_t i = 0; i < N_LAYERS; ++i) {
        if (currentIteration % (i + 1) == 0) {
          relaxPoints(points[i], N_START_POINTS + i);
          generateVoronoiNoise(map, points[i], i + 1, N_START_POINTS + i, ctx,
                               bias_scale, rate);
        }
      }
      normalizeMap(map, &min, &max);
      twoDimensionalArrayToOneDimensionalArray(m, map);
      erode(&erosion, m, 1000);
      oneDimensionalArrayToTwoDimensional(m, map);
      for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
        for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
          map[i][j] = MAP(map[i][j], 0, 1, min, max);
        }
      }
      currentIteration++;
      printf("%d\n", currentIteration);
    }
    for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
      for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
        tempMap[i][j] = map[i][j];
      }
    }
    normalizeMap(tempMap, &max, &min);
    drawMap(ctx, tempMap);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  open_simplex_noise_free(ctx);
  for (size_t i = 0; i < N_LAYERS; ++i) {
    free(points[i]);
  }
  free(points);
  free_erode(&erosion);
  return 0;
}
