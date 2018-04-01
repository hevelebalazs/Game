#include "Map.h"

struct MemArena;

Map CreateGridMap(float width, float height, float junctionDistance, MemArena* arena, MemArena* tmpArena);
