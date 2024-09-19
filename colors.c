#include "colors.h"
#include "common.h"
#include <stdlib.h>

Color colors[31];

void addColors() {
  colors[0] = color(203, 189, 147);
  colors[1] = color(198, 194, 145);
  colors[2] = color(192, 199, 145);
  colors[3] = color(184, 200, 137);
  colors[4] = color(178, 205, 137);
  colors[5] = color(173, 211, 135);
  colors[6] = color(163, 217, 129);
  colors[7] = color(152, 223, 129);
  colors[8] = color(140, 230, 110);
  colors[9] = color(130, 240, 75);
  colors[10] = color(124, 252, 0);
  colors[11] = color(115, 240, 0);
  colors[12] = color(100, 228, 0);
  colors[13] = color(90, 208, 20);
  colors[14] = color(76, 187, 23);
  colors[15] = color(65, 180, 23);
  colors[16] = color(59, 167, 23);
  colors[17] = color(53, 150, 35);
  colors[18] = color(85, 130, 102);
  colors[19] = color(120, 102, 115);
  colors[20] = color(136, 141, 140);
  colors[21] = color(145, 150, 145);
  colors[22] = color(159, 159, 159);
  colors[23] = color(169, 169, 169);
  colors[24] = color(178, 178, 178);
  colors[25] = color(189, 189, 189);
  colors[26] = color(200, 200, 200);
  colors[27] = color(211, 211, 211);
  colors[28] = color(225, 225, 225);
  colors[29] = color(242, 242, 242);
  colors[30] = color(255, 255, 255);
}

Color color(int r, int g, int b) {
  Color color;
  color.r = r / 255.0f;
  color.g = g / 255.0f;
  color.b = b / 255.0f;
  return color;
}

void initializeHeight(float **heights) {
  size_t length = 31;
  *heights = (float *)calloc(length, sizeof(float));
  for (size_t i = 0; i < length; i++) {
    (*heights)[i] = (float)(i) / (length);
  }
}

static size_t getIndex(float *heights, float value) {
  size_t length = (sizeof(&heights) / sizeof(float));
  for (size_t i = 0; i < 31; ++i) {
    if (value >= heights[i] && (i == 30 || value < heights[i + 1])) {
      return i;
    }
  }
  return 30;
}

Color getColor(float *heights, float value, float sealevel) {
  if (value < sealevel) {
    value = MAP(value, 0, sealevel, -1, 0);
  } else {
    value = MAP(value, sealevel, 1, 0, 1);
  }
  if (value < 0) {
    Color rgb = color(0, 0, 255);
    return rgb;
  }
  Color c = colors[getIndex(heights, value)];
  return c;
}
