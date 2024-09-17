#include "erosion.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
  float height;
  float gradientX;
  float gradientY;
} HeightAndGradient;

static void initalizeBrushIndicies(Erosion *erosion) {
  erosion->erosionBrushIndicies =
      (int **)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(int *));
  erosion->erosionBrushWeights =
      (float **)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(float *));
  erosion->lengths =
      (size_t *)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(size_t));

  int *xOffsets =
      (int *)calloc(EROSION_RADIUS * EROSION_RADIUS * 4, sizeof(int));
  int *yOffsets =
      (int *)calloc(EROSION_RADIUS * EROSION_RADIUS * 4, sizeof(int));
  float *weights =
      (float *)calloc(EROSION_RADIUS * EROSION_RADIUS * 4, sizeof(float));
  float weightSum = 0;
  int addIndex = 0;

  for (size_t i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i) {
    int centerX = i % WINDOW_WIDTH;
    int centerY = i / WINDOW_WIDTH;

    if (centerY <= EROSION_RADIUS ||
        centerY >= WINDOW_HEIGHT - EROSION_RADIUS ||
        centerX <= EROSION_RADIUS + 1 ||
        centerX >= WINDOW_WIDTH - EROSION_RADIUS) {
      weightSum = 0;
      addIndex = 0;

      for (int y = -EROSION_RADIUS; y <= EROSION_RADIUS; ++y) {
        for (int x = -EROSION_RADIUS; x <= EROSION_RADIUS; ++x) {
          float sqrDst = x * x + y * y;
          if (sqrDst < EROSION_RADIUS * EROSION_RADIUS) {
            int cordX = centerX + x;
            int cordY = centerY + y;

            if (cordX >= 0 && cordX < WINDOW_WIDTH && cordY >= 0 &&
                cordY < WINDOW_HEIGHT) {
              float weight = 1 - sqrt(sqrDst) / EROSION_RADIUS;
              weightSum += weight;
              weights[addIndex] = weight;
              xOffsets[addIndex] = x;
              yOffsets[addIndex] = y;
              addIndex++;
            }
          }
        }
      }
      int numEntries = addIndex;
      erosion->erosionBrushIndicies[i] = (int *)calloc(numEntries, sizeof(int));
      erosion->erosionBrushWeights[i] =
          (float *)calloc(numEntries, sizeof(float));
      erosion->lengths[i] = numEntries;

      for (size_t j = 0; j < numEntries; ++j) {
        erosion->erosionBrushIndicies[i][j] =
            (yOffsets[j] + centerY) * WINDOW_WIDTH + xOffsets[j] + centerX;
        erosion->erosionBrushWeights[i][j] = weights[j] / weightSum;
      }
    }
  }

  free(xOffsets);
  free(yOffsets);
  free(weights);
}

void erode_init(Erosion *erosion) { initalizeBrushIndicies(erosion); }

void free_erode(Erosion *erosion) {
  free(erosion->erosionBrushWeights);
  free(erosion->erosionBrushIndicies);
  free(erosion->lengths);
}

static HeightAndGradient
calculateHeightAndGradient(float map[WINDOW_WIDTH * WINDOW_HEIGHT], float posX,
                           float posY) {
  int cordX = (int)posX;
  int cordY = (int)posY;

  float x = posX - cordX;
  float y = posY - cordY;

  int nodeIndexNW = cordY * WINDOW_WIDTH + cordX;
  float heightNW = map[nodeIndexNW];
  float heightNE = map[nodeIndexNW + 1];
  float heightSW = map[nodeIndexNW + WINDOW_WIDTH];
  float heightSE = map[nodeIndexNW + WINDOW_WIDTH + 1];

  float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
  float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

  float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) +
                 heightSW * (1 - x) * y + heightSE * x * y;

  HeightAndGradient result = {height, gradientX, gradientY};
  return result;
}

void erode(Erosion *erosion, float map[WINDOW_WIDTH * WINDOW_HEIGHT],
           int numIterations) {

  for (size_t iteration = 0; iteration < numIterations; ++iteration) {
    float posX = RAND_IN_RANGE(0, WINDOW_WIDTH - 1);
    float posY = RAND_IN_RANGE(0, WINDOW_HEIGHT - 1);
    float dirX = 0, dirY = 0;
    float speed = INITAL_SPEED;
    float water = INITIAL_WATER_VOLUME;
    float sediment = 0;
    for (size_t lifetime = 0; lifetime < MAX_DROPLET_LIFETIME; ++lifetime) {
      int nodeX = (int)posX;
      int nodeY = (int)posY;
      int dropletIndex = nodeY * WINDOW_WIDTH + nodeX;

      float cellOffsetX = posX - nodeX;
      float cellOffsetY = posY - nodeY;

      HeightAndGradient heightAndGradient =
          calculateHeightAndGradient(map, posX, posY);

      dirX = dirX * INERTIA - heightAndGradient.gradientX * (1 - INERTIA);
      dirY = dirY * INERTIA - heightAndGradient.gradientY * (1 - INERTIA);

      float len = sqrt(dirX * dirX + dirY * dirY);
      if (len != 0) {
        dirX /= len;
        dirY /= len;
      }
      posX += dirX;
      posY += dirY;

      if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= WINDOW_WIDTH - 1 ||
          posY < 0 || posY >= WINDOW_HEIGHT - 1) {
        break;
      }

      float newHeight = calculateHeightAndGradient(map, posX, posY).height;
      float deltaHeight = newHeight - heightAndGradient.height;

      float sedimentCapcity =
          MAX(-deltaHeight * speed * water * SEDIMENT_CAPACITY_FACTOR,
              MIN_SEDIMENT_CAPACITY);

      if (sediment > sedimentCapcity || deltaHeight > 0) {
        float amountToDeposit =
            (deltaHeight > 0) ? MIN(deltaHeight, sediment)
                              : (sediment - sedimentCapcity) * DEPOSIT_SPEED;
        sediment -= amountToDeposit;
        map[dropletIndex] +=
            amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
        map[dropletIndex + 1] +=
            amountToDeposit * cellOffsetX * (1 - cellOffsetY);
        map[dropletIndex + WINDOW_WIDTH] +=
            amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
        map[dropletIndex + WINDOW_WIDTH + 1] +=
            amountToDeposit * cellOffsetX * cellOffsetY;
      } else {
        float amountToErode =
            MIN((sedimentCapcity - sediment) * ERODE_SPEED, -deltaHeight);
        for (size_t brushPointIndex = 0;
             brushPointIndex < erosion->lengths[dropletIndex];
             brushPointIndex++) {
          int nodeIndex =
              erosion->erosionBrushIndicies[dropletIndex][brushPointIndex];
          float weightErodeAmount =
              amountToErode *
              erosion->erosionBrushWeights[dropletIndex][brushPointIndex];
          float deltaSediment = (map[nodeIndex] < weightErodeAmount)
                                    ? map[nodeIndex]
                                    : weightErodeAmount;
          map[nodeIndex] -= deltaSediment;
          sediment += deltaSediment;
        }
      }

      speed = sqrt(speed * speed + deltaHeight * GRAVITY);
      water *= (1 - EVAPORATE_SPEED);
    }
  }
}
