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
#include "heightgen.h"
#include "open-simplex-noise.h"

void normalizeMap(float **map, float *min, float *max) {
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

float getSealevel(float **map) {
  float lowerBound = 0;
  float upperBound = 1.0f;
  float sealevel = 0;
  int totalCells = WINDOW_WIDTH * WINDOW_HEIGHT;
  while (upperBound - lowerBound > 0.001f) {
    sealevel = (lowerBound + upperBound) / 2.0f;
    int n = 0;

    for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
      bool skip = false;
      for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
        if (map[i][j] < sealevel) {
          n++;
        }
      }
    }
    float percentage = (float)(n) / totalCells;

    printf("Sealevel: %.3f, Percentage: %.3f, n: %d, lowerBound: %.3f, "
           "upperBound: %.3f\n",
           sealevel, percentage, n, lowerBound, upperBound);

    if (percentage < WATER_THRESHOLD) {
      lowerBound = sealevel;
    } else {
      upperBound = sealevel;
    }
  }

  return sealevel;
}

void drawMap(struct osn_context *ctx, float **map, float *heights,
             float sealevel) {
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
  for (int y = 0; y < WINDOW_HEIGHT; ++y) {
    for (int x = 0; x < WINDOW_WIDTH; ++x) {
      float value = map[x][y];
      Color rgb = getColor(heights, value, sealevel);
      glColor3f(rgb.r, rgb.g, rgb.b);
      glVertex2i(x, y);
    }
  }
  glEnd();
}

void oneDimensionalArrayToTwoDimensional(float *oneDimensionalArray,
                                         float **twoDimensionalArray) {
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      twoDimensionalArray[i][j] = oneDimensionalArray[i + j * WINDOW_WIDTH];
    }
  }
}

void twoDimensionalArrayToOneDimensionalArray(float *oneDimensionalArray,
                                              float **twoDimensionalArray) {
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      oneDimensionalArray[i + j * WINDOW_WIDTH] = twoDimensionalArray[i][j];
    }
  }
}

float *init1DArray(size_t size) {
  float *array = (float *)calloc(size, sizeof(float));
  for (size_t i = 0; i < size; ++i) {
    array[i] = 0;
  }
  return array;
}

float **init2DArray() {
  float **array =
      (float **)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(float *));
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    array[i] = init1DArray(WINDOW_HEIGHT);
  }
  return array;
}

void free2DArray(float **array) {
  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    free(array[i]);
  }
  free(array);
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
      p[j] = v;
    }
    points[i] = p;
  }

  int currentIteration = 0;
  float min = FLT_MIN;
  float max = FLT_MAX;
  float bias_scale = 0.0001;
  float rate = 1;
  float sealevel = 0.5;
  Erosion erosion;

  erode_init(&erosion);
  struct osn_context *ctx;
  open_simplex_noise(SEED, &ctx);

  float **map = init2DArray();
  float **tempMap = init2DArray();
  float **heightMap = init2DArray();
  float *m = init1DArray(WINDOW_WIDTH * WINDOW_HEIGHT);
  float *heights;
  initializeHeight(&heights);
  addColors();

  for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
    for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
      map[i][j] = 0;
      heightMap[i][j] = 0;
    }
  }
  heightMapGen(heightMap, ctx);
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
      if (true) {
        for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
          for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
            map[i][j] += 10 * heightMap[i][j];
          }
        }
      }
      normalizeMap(map, &min, &max);
      twoDimensionalArrayToOneDimensionalArray(m, map);
      erode(&erosion, m, 200000, sealevel);
      oneDimensionalArrayToTwoDimensional(m, map);
      for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
        for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
          map[i][j] = MAP(map[i][j], 0, 1, min, max);
        }
      }
      currentIteration++;
      printf("%d\n", currentIteration);
    } else if (currentIteration == MAX_ITERATIONS) {
      normalizeMap(map, &min, &max);
      twoDimensionalArrayToOneDimensionalArray(m, map);
      erode(&erosion, m, 2000000, sealevel);
      oneDimensionalArrayToTwoDimensional(m, map);
      for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
        for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
          map[i][j] = MAP(map[i][j], 0, 1, min, max);
        }
      }
      currentIteration++;
    }
    for (size_t i = 0; i < WINDOW_WIDTH; ++i) {
      for (size_t j = 0; j < WINDOW_HEIGHT; ++j) {
        tempMap[i][j] = map[i][j];
      }
    }
    normalizeMap(tempMap, &max, &min);
    if (currentIteration % 10 == 0) {
      sealevel = getSealevel(tempMap);
      printf("%f\n", sealevel);
    }
    drawMap(ctx, tempMap, heights, sealevel);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  open_simplex_noise_free(ctx);
  for (size_t i = 0; i < N_LAYERS; ++i) {
    free(points[i]);
  }
  free(heights);
  free(points);
  free_erode(&erosion);
  free2DArray(map);
  free2DArray(tempMap);
  free2DArray(heightMap);
  free(m);
  return 0;
}
