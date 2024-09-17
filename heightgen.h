#pragma once

#include "common.h"
#include "open-simplex-noise.h"

void heightMapGen(float heightMap[WINDOW_WIDTH][WINDOW_HEIGHT], struct osn_context *ctx);
