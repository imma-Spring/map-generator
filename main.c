#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "colors.h"
#include "open-simplex-noise.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void drawMap(struct osn_context *ctx) {
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
  for (int y = 0; y < WINDOW_HEIGHT; ++y) {
    for (int x = 0; x < WINDOW_WIDTH; ++x) {
      float value = (open_simplex_noise2(ctx, x * 0.01, y * 0.01) + 1) / 2.0f;
      glColor3f(value, value, value);
      glVertex2i(x, y);
    }
  }
  glEnd();
}

int main() {
  struct osn_context *ctx;
  open_simplex_noise(1, &ctx);
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

  glfwMakeContextCurrent(window);
  glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

  while (!glfwWindowShouldClose(window)) {
    drawMap(ctx);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  open_simplex_noise_free(ctx);
  return 0;
}
