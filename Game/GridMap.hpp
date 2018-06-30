#include "Map.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Road.hpp"

#define JunctionGridDistance			(MinimumJunctionDistance * 1.5f)
#define MaxJunctionDistanceFromOrigin	(MinimumJunctionDistance * 0.5f)

void GenerateGridMap(Map* map, I32 junctionRowN, I32 junctionColN, I32 roadN, MemArena* tmpArena);