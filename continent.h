#pragma once

#include "common.h"
#include "open-simplex-noise.h"

void generateVoronoiNoise(float map[WINDOW_WIDTH][WINDOW_HEIGHT],
                             Vector layerPoints[], const float index,
                             const size_t length, struct osn_context *ctx,
                             const float bias_scale, const float rate);
void relaxPoints(Vector layerPoints[], const size_t length);
