#pragma once

#include "Draw.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Type.hpp"

void LightSector(Canvas canvas, V2 center, F32 minDistance, F32 maxDistance, F32 minAngle, F32 maxAngle, F32 baseBrightness);
void LightCircle(Canvas canvas, V2 center, F32 seeDistance, F32 baseBrightness = 1.0f);