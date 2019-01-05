#pragma once

#include "Map.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Road.hpp"

#define JunctionGridDistance			(MinimumJunctionDistance * 1.5f)
#define MaxJunctionDistanceFromOrigin	(MinimumJunctionDistance * 0.5f)

static void func GenerateGridMapJunctions(Map* map, I32 junctionRowN, I32 junctionColN)
{
	F32 left = -(junctionRowN * JunctionGridDistance) / 2;
	F32 top  = -(junctionColN * JunctionGridDistance) / 2;

	F32 junctionX = left;
	F32 junctionY = top;

	map->junctionN = (junctionRowN * junctionColN);
	for (I32 row = 0; row < junctionRowN; ++row)
	{
		for (I32 col = 0; col < junctionColN; ++col)
		{
			Junction* junction = map->junctions + (row * junctionColN) + col;
			junction->position.x = junctionX + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
			junction->position.y = junctionY + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
			junction->roadN = 0;
			junctionX += JunctionGridDistance;
		}
		junctionY += JunctionGridDistance;
		junctionX = left + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
	}
}

struct JunctionGridPosition {
	I32 row;
	I32 col;
};

enum JunctionGridDirection {
	JunctionGridUp,
	JunctionGridRight,
	JunctionGridDown,
	JunctionGridLeft,
	JunctionGridDirectionN
};

static JunctionGridDirection func GetRandomJunctionGridDirection()
{
	JunctionGridDirection result = (JunctionGridDirection)IntRandom(0, JunctionGridDirectionN - 1);
	return result;
}

static JunctionGridPosition func GetNextJunctionGridPosition(JunctionGridPosition startPosition, JunctionGridDirection direction)
{
	JunctionGridPosition endPosition = startPosition;

	if (direction == JunctionGridUp)
	{
		endPosition.row--;
	}
	else if (direction == JunctionGridDown)
	{
		endPosition.row++;
	}

	if (direction == JunctionGridLeft)
	{
		endPosition.col--;
	}
	else if (direction == JunctionGridRight)
	{
		endPosition.col++;
	}

	return endPosition;
}

static B32 func AreJunctionsConnected(Junction* junction1, Junction* junction2)
{
	B32 areConnected = false;
	for (I32 i = 0; i < junction1->roadN; ++i)
	{
		Road* road = junction1->roads[i];
		if (road->junction1 == junction1 && road->junction2 == junction2)
		{
			areConnected = true;
		}
		else if (road->junction1 == junction2 && road->junction2 == junction1)
		{
			areConnected = true;
		}

		if (areConnected)
		{
			break;
		}
	}
	return areConnected;
}

static void func GenerateGridMapRoads(Map* map, I32 junctionRowN, I32 junctionColN, I32 roadN, MemArena* tmpArena)
{
	JunctionGridPosition* connectedJunctionPositions = ArenaPushArray(tmpArena, JunctionGridPosition, map->junctionN);
	I32 connectedJunctionN = 0;

	JunctionGridPosition startPosition = {};
	startPosition.row = IntRandom(0, junctionRowN - 1);
	startPosition.col = IntRandom(0, junctionColN - 1);
	connectedJunctionPositions[connectedJunctionN] = startPosition;
	connectedJunctionN++;

	map->roadN = roadN;

	I32 createdRoadN = 0;
	while (1)
	{
		I32 positionIndex = IntRandom(0, connectedJunctionN - 1);
		JunctionGridPosition junctionPosition = connectedJunctionPositions[positionIndex];
		JunctionGridDirection direction = GetRandomJunctionGridDirection();
		while (1)
		{
			Junction* junction = map->junctions + (junctionColN * junctionPosition.row) + junctionPosition.col;
			JunctionGridPosition nextPosition = GetNextJunctionGridPosition(junctionPosition, direction);
			Assert(junctionPosition.row != nextPosition.row || junctionPosition.col != nextPosition.col);
			if (nextPosition.row < 0 || nextPosition.row >= junctionRowN)
			{
				break;
			}
			if (nextPosition.col < 0 || nextPosition.col >= junctionColN)
			{
				break;
			}

			Junction* nextJunction = map->junctions + (junctionColN * nextPosition.row) + nextPosition.col;
			if (AreJunctionsConnected(junction, nextJunction))
			{
				break;
			}

			B32 isNextJunctionConnected = (nextJunction->roadN > 0);
			if (!isNextJunctionConnected)
			{
				connectedJunctionPositions[connectedJunctionN] = nextPosition;
				connectedJunctionN++;
			}

			ConnectJunctions(junction, nextJunction, map->roads + createdRoadN);
			createdRoadN++;
			if (createdRoadN == roadN)
			{
				break;
			}

			junction = nextJunction;
			junctionPosition = nextPosition;

			if (isNextJunctionConnected)
			{
				if (IntRandom(0, 2) == 0)
				{
					break;
				}
			} 
			else 
			{
				if (IntRandom(0, 5) == 0)
				{
					break;
				}
			}
		}

		if (createdRoadN == roadN)
		{
			break;
		}
	}

	ArenaPopTo(tmpArena, connectedJunctionPositions);
}

static void func ReindexJunction(Junction* oldJunction, Junction* newJunction)
{
	for (I32 i = 0; i < oldJunction->roadN; ++i)
	{
		Road* road = oldJunction->roads[i];
		if (road->junction1 == oldJunction)
		{
			road->junction1 = newJunction;
		}
		else if (road->junction2 == oldJunction)
		{
			road->junction2 = newJunction;
		}
		else
		{
			InvalidCodePath;
		}
	}
}

static void func RemoveEmptyJunctions(Map* map)
{
	I32 newJunctionN = 0;
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		Junction* junction = map->junctions + i;
		if (junction->roadN > 0)
		{
			Junction* newJunction = map->junctions + newJunctionN;
			*newJunction = *junction;
			ReindexJunction(junction, newJunction);
			newJunctionN++;
		}
	}

	map->junctionN = newJunctionN;
}

static void func InitJunctions(Map* map)
{
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		Junction* junction = map->junctions + i;
		if (junction->roadN > 0)
		{
			CalculateStopDistances(junction);
			InitTrafficLights(junction);
		}
	}
}

static void func GenerateCrossings(Map* map)
{
	for (I32 i = 0; i < map->roadN; ++i)
	{
		Road* road = map->roads + i;
		GenerateCrossing(road);
	}
}

static void func GenerateGridMap(Map* map, I32 junctionRowN, I32 junctionColN, I32 roadN, MemArena* tmpArena)
{
	Assert(map->junctions != 0);
	Assert(map->roads != 0);

	map->left   = -((junctionColN + 2) * JunctionGridDistance) * 0.5f;
	map->right  = +((junctionColN + 2) * JunctionGridDistance) * 0.5f;
	map->top    = -((junctionRowN + 2) * JunctionGridDistance) * 0.5f;
	map->bottom = +((junctionRowN + 2) * JunctionGridDistance) * 0.5f;

	map->tileRowN = Floor((map->right - map->left) / (MapTileSide));
	map->tileColN = Floor((map->bottom - map->top) / (MapTileSide));

	InitRandom();

	GenerateGridMapJunctions(map, junctionRowN, junctionColN);
	GenerateGridMapRoads(map, junctionRowN, junctionColN, roadN, tmpArena);

	RemoveEmptyJunctions(map);
	InitJunctions(map);
	GenerateCrossings(map);
}