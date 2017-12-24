#include "Map.h"

struct MemArena;

Map CreateGridMap(float width, float height, float intersectionDistance, MemArena* arena, MemArena* tmpArena);
