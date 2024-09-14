#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "colors.h"
#include "opensimplex2s.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void drawMap() {
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
  for (int y = 0; y < WINDOW_HEIGHT; ++y) {
    for (int x = 0; x < WINDOW_WIDTH; ++x) {
      if (x < WINDOW_WIDTH / 2 && y < WINDOW_HEIGHT / 2) {
        glColor3f(0.0f, 0.0f, 0.0f);
      } else {
        glColor3f(1.0f, 1.0f, 1.0f);
      }
      glVertex2i(x, y);
    }
  }
  glEnd();
}

int main() {
  opensimplex_init();
  printf("%f", opensimplex2s_noise2(1, 0.1, 0.1));
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Checkerboard", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to open window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

  while (!glfwWindowShouldClose(window)) {
    drawMap();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  opensimple_free();
  return 0;
}
