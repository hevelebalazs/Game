#include "GridMap.hpp"

static void GenerateGridMapJunctions(Map* map, int junctionRowN, int junctionColN)
{
	float left = 0.0f;
	float top  = 0.0f;

	float junctionX = left;
	float junctionY = top;

	map->junctionCount = (junctionRowN * junctionColN);
	float JunctionGridDistance = MinimumJunctionDistance * 1.5f;
	float MaxJunctionDistanceFromOrigin = MinimumJunctionDistance * 0.5f;
	for (int row = 0; row < junctionRowN; ++row) {
		for (int col = 0; col < junctionColN; ++col) {
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
	int row;
	int col;
};

enum JunctionGridDirection {
	JunctionGridUp,
	JunctionGridRight,
	JunctionGridDown,
	JunctionGridLeft,
	JunctionGridDirectionN
};

static JunctionGridDirection GetRandomJunctionGridDirection()
{
	JunctionGridDirection result = (JunctionGridDirection)IntRandom(0, JunctionGridDirectionN - 1);
	return result;
}

static JunctionGridPosition GetNextJunctionGridPosition(JunctionGridPosition startPosition, JunctionGridDirection direction)
{
	JunctionGridPosition endPosition = startPosition;

	if (direction == JunctionGridUp)
		endPosition.row--;
	else if (direction == JunctionGridDown)
		endPosition.row++;

	if (direction == JunctionGridLeft)
		endPosition.col--;
	else if (direction == JunctionGridRight)
		endPosition.col++;

	return endPosition;
}

static bool AreJunctionsConnected(Junction* junction1, Junction* junction2)
{
	bool areConnected = false;
	for (int i = 0; i < junction1->roadN; ++i) {
		Road* road = junction1->roads[i];
		if (road->junction1 == junction1 && road->junction2 == junction2)
			areConnected = true;
		else if (road->junction1 == junction2 && road->junction2 == junction1)
			areConnected = true;

		if (areConnected)
			break;
	}
	return areConnected;
}

static void GenerateGridMapRoads(Map* map, int junctionRowN, int junctionColN, int roadN, MemArena* tmpArena)
{
	JunctionGridPosition* connectedJunctionPositions = ArenaPushArray(tmpArena, JunctionGridPosition, map->junctionCount);
	int connectedJunctionN = 0;

	JunctionGridPosition startPosition = {};
	startPosition.row = IntRandom(0, junctionRowN - 1);
	startPosition.col = IntRandom(0, junctionColN - 1);
	connectedJunctionPositions[connectedJunctionN] = startPosition;
	connectedJunctionN++;

	map->roadCount = roadN;

	int createdRoadN = 0;
	while (1) {
		int positionIndex = IntRandom(0, connectedJunctionN - 1);
		JunctionGridPosition junctionPosition = connectedJunctionPositions[positionIndex];
		JunctionGridDirection direction = GetRandomJunctionGridDirection();
		while (1) {
			Junction* junction = map->junctions + (junctionColN * junctionPosition.row) + junctionPosition.col;
			JunctionGridPosition nextPosition = GetNextJunctionGridPosition(junctionPosition, direction);
			Assert(junctionPosition.row != nextPosition.row || junctionPosition.col != nextPosition.col);
			if (nextPosition.row < 0 || nextPosition.row >= junctionRowN)
				break;
			if (nextPosition.col < 0 || nextPosition.col >= junctionColN)
				break;

			Junction* nextJunction = map->junctions + (junctionColN * nextPosition.row) + nextPosition.col;
			if (AreJunctionsConnected(junction, nextJunction))
				break;

			bool isNextJunctionConnected = (nextJunction->roadN > 0);
			if (!isNextJunctionConnected) {
				connectedJunctionPositions[connectedJunctionN] = nextPosition;
				connectedJunctionN++;
			}

			ConnectJunctions(junction, nextJunction, map->roads + createdRoadN);
			createdRoadN++;
			if (createdRoadN == roadN)
				break;

			junction = nextJunction;
			junctionPosition = nextPosition;

			if (isNextJunctionConnected) {
				if (IntRandom(0, 2) == 0)
					break;
			} else {
				if (IntRandom(0, 5) == 0)
					break;
			}
		}

		if (createdRoadN == roadN)
			break;
	}

	ArenaPopTo(tmpArena, connectedJunctionPositions);
}

static void ReindexJunction(Junction* oldJunction, Junction* newJunction)
{
	for (int i = 0; i < oldJunction->roadN; ++i) {
		Road* road = oldJunction->roads[i];
		if (road->junction1 == oldJunction)
			road->junction1 = newJunction;
		else if (road->junction2 == oldJunction)
			road->junction2 = newJunction;
		else
			InvalidCodePath;
	}
}

static void RemoveEmptyJunctions(Map* map)
{
	int newJunctionN = 0;
	for (int i = 0; i < map->junctionCount; ++i) {
		Junction* junction = map->junctions + i;
		if (junction->roadN > 0) {
			Junction* newJunction = map->junctions + newJunctionN;
			*newJunction = *junction;
			ReindexJunction(junction, newJunction);
			newJunctionN++;
		}
	}

	map->junctionCount = newJunctionN;
}

static void InitJunctions(Map* map)
{
	for (int i = 0; i < map->junctionCount; ++i) {
		Junction* junction = map->junctions + i;
		if (junction->roadN > 0) {
			CalculateStopDistances(junction);
			InitTrafficLights(junction);
		}
	}
}

static void GenerateCrossings(Map* map)
{
	for (int i = 0; i < map->roadCount; ++i) {
		Road* road = map->roads + i;
		GenerateCrossing(road);
	}
}

void GenerateGridMap(Map* map, int junctionRowN, int junctionColN, int roadN, MemArena* tmpArena)
{
	Assert(map->junctions != 0);
	Assert(map->roads != 0);

	InitRandom();

	GenerateGridMapJunctions(map, junctionRowN, junctionColN);
	GenerateGridMapRoads(map, junctionRowN, junctionColN, roadN, tmpArena);

	RemoveEmptyJunctions(map);
	InitJunctions(map);
	GenerateCrossings(map);
}