#include "GridMap.h"
#include "Math.h"
#include "Memory.h"

struct GridPosition {
	int row;
	int col;
};

enum GridMapDirection {
	GridMapLeft,
	GridMapRight,
	GridMapUp,
	GridMapDown
};

struct BuildArea {
	float left;
	float right;
	float top;
	float bottom;
};

static void GenerateBuildings(Map* map, BuildArea area, float buildingPadding, float minBuildingSide, float maxBuildingSide) {
	area.left += RandomBetween(0.0f, buildingPadding);
	area.right -= RandomBetween(0.0f, buildingPadding);
	area.top += RandomBetween(0.0f, buildingPadding);
	area.bottom -= RandomBetween(0.0f, buildingPadding);

	float areaWidth = area.right - area.left;
	float areaHeight = area.bottom - area.top;

	if (areaWidth < buildingPadding) 
		return;

	if (areaHeight < buildingPadding) 
		return;

	if ((areaWidth > maxBuildingSide) || 
		((areaWidth >= buildingPadding + 2 * minBuildingSide) && (RandomBetween(0.0f, 1.0f) < 0.5f))
	) {
		BuildArea areaLeft = area;
		areaLeft.right = ((area.left + area.right) / 2.0f);
		
		BuildArea areaRight = area;
		areaRight.left = ((area.left + area.right) / 2.0f);

		GenerateBuildings(map, areaLeft, buildingPadding, minBuildingSide, maxBuildingSide);
		GenerateBuildings(map, areaRight, buildingPadding, minBuildingSide, maxBuildingSide);
	}
	else if ((areaHeight > maxBuildingSide) ||
		((areaHeight >= buildingPadding + 2 * minBuildingSide) && (RandomBetween(0.0f, 1.0f) < 0.5f))
	){
		BuildArea areaTop = area;
		areaTop.bottom = ((area.top + area.bottom) / 2.0f);

		BuildArea areaBottom = area;
		areaBottom.top = ((area.top + area.bottom) / 2.0f);

		GenerateBuildings(map, areaTop, buildingPadding, minBuildingSide, maxBuildingSide);
		GenerateBuildings(map, areaBottom, buildingPadding, minBuildingSide, maxBuildingSide);
	}
	else {
		Building* building = &map->buildings[map->buildingCount];
		*building = {};

		building->left = area.left;
		building->right = area.right;
		building->top = area.top;
		building->bottom = area.bottom;

		building->height = RandomBetween(10.0f, 30.0f);

		map->buildingCount++;
	}
}

static void ConnectJunctions(Junction* junction1, Junction* junction2, Road* road) {
	if (junction1->position.y == junction2->position.y) {
		Junction* left;
		Junction* right;

		if (junction1->position.x < junction2->position.x) {
			left = junction1;
			right = junction2;
		}
		else {
			left = junction2;
			right = junction1;
		}

		road->endPoint1.x = left->position.x + LaneWidth;
		road->endPoint1.y = left->position.y;

		road->endPoint2.x = right->position.x - LaneWidth;
		road->endPoint2.y = right->position.y;

		left->rightRoad = road;
		right->leftRoad = road;

		road->junction1 = left;
		road->junction2 = right;
	}
	else {
		Junction* top;
		Junction* bottom;

		if (junction1->position.y < junction2->position.y) {
			top = junction1;
			bottom = junction2;
		}
		else {
			top = junction2;
			bottom = junction1;
		}

		road->endPoint1.x = top->position.x;
		road->endPoint1.y = top->position.y + LaneWidth;

		road->endPoint2.x = bottom->position.x;
		road->endPoint2.y = bottom->position.y - LaneWidth;

		bottom->topRoad = road;
		top->bottomRoad = road;

		road->junction1 = top;
		road->junction2 = bottom;
	}
}

// TODO: can the recursion cause any performance or memory issue?
// TODO: a stack overflow has happened here, find out the issue
static void CalculateTreeHeight(Building* building) {
	if (building->connectTreeHeight > 0)
		return;

	if (building->connectElem.type == MapElemBuilding) {
		Building* connectBuilding = building->connectElem.building;

		CalculateTreeHeight(connectBuilding);

		building->connectTreeHeight = connectBuilding->connectTreeHeight + 1;
	}
	else {
		building->connectTreeHeight = 1;
	}
}

static void ReindexRoad(Road* road, Junction* oldJunction, Junction* newJunction) {
	if (road->junction1 == oldJunction) 
		road->junction1 = newJunction;

	if (road->junction2 == oldJunction) 
		road->junction2 = newJunction;
}

static void RemoveJunction(Map* map, Junction* junction) {
	Junction* oldJunction = &map->junctions[map->junctionCount - 1];
	map->junctionCount--;

	*junction = *oldJunction;

	if (junction->leftRoad)   
		ReindexRoad(junction->leftRoad,   oldJunction, junction);

	if (junction->rightRoad)  
		ReindexRoad(junction->rightRoad,  oldJunction, junction);

	if (junction->topRoad)  
		ReindexRoad(junction->topRoad,    oldJunction, junction);

	if (junction->bottomRoad)
		ReindexRoad(junction->bottomRoad, oldJunction, junction);
}

static void ReindexJunction(Junction* junction, Road* oldRoad, Road* road) {
	if (junction->leftRoad == oldRoad) 
		junction->leftRoad = road;

	if (junction->rightRoad == oldRoad) 
		junction->rightRoad = road;

	if (junction->topRoad == oldRoad) 
		junction->topRoad = road;

	if (junction->bottomRoad == oldRoad) 
		junction->bottomRoad = road;
}

static void RemoveRoad(Map* map, Road* road) {
	Road* oldRoad = &map->roads[map->roadCount - 1];
	map->roadCount--;

	*road = *oldRoad;

	if (road->junction1) 
		ReindexJunction(road->junction1, oldRoad, road);

	if (road->junction2) 
		ReindexJunction(road->junction2, oldRoad, road);
}

// TODO: separate this into smaller functions?
// TODO: sometimes a building is not connected to anything, find out why this is
Map CreateGridMap(float width, float height, float junctionDistance, MemArena* arena, MemArena* tmpArena) {
	Map map = {};
	map.width = width;
	map.height = height;

	int colCount = (int)(width / junctionDistance);
	int rowCount = (int)(height / junctionDistance);

	float rightOffset = width - ((float)(colCount - 1) * junctionDistance);
	float bottomOffset = height - ((float)(rowCount - 1) * junctionDistance);

	Point leftTop = {rightOffset / 2.0f, bottomOffset / 2.0f};

	int junctionCount = colCount * rowCount;

	map.junctions = ArenaPushArray(arena, Junction, junctionCount);
	map.junctionCount = junctionCount;

	int junctionIndex = 0;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount; ++col) {
			map.junctions[junctionIndex] = Junction{};
			map.junctions[junctionIndex].position = PointSum(
				leftTop,
				PointProd((float)junctionDistance, Point{(float)col, (float)row})
			);

			++junctionIndex;
		}
	}

	GridPosition* connectedPositions = ArenaPushArray(tmpArena, GridPosition, junctionCount);
	int connectedCount = 0;

	int maxRoadCount = colCount * (rowCount - 1) + (colCount - 1) * rowCount;
	int roadCount = maxRoadCount / 2;
	int createdRoadCount = 0;

	map.roads = ArenaPushArray(arena, Road, roadCount);
	map.roadCount = roadCount;

	InitRandom();

	int startRow = rand() % rowCount;
	int startCol = rand() % colCount;

	connectedPositions[connectedCount++] = {startRow, startCol};

	while (createdRoadCount < roadCount) {
		GridPosition startPosition = connectedPositions[rand() % connectedCount];
		Junction* startJunction =
			&map.junctions[startPosition.row * colCount + startPosition.col];

		int direction = rand() % 4;

		while (createdRoadCount < roadCount) {
			bool createRoad = true;

			GridPosition endPosition = startPosition;

			if (direction == GridMapRight) {
				if (startJunction->rightRoad) 
					createRoad = false;
				endPosition.col++;
			}
			else if (direction == GridMapLeft) {
				if (startJunction->leftRoad) 
					createRoad = false;
				endPosition.col--;
			}
			else if (direction == GridMapDown) {
				if (startJunction->bottomRoad) 
					createRoad = false;
				endPosition.row++;
			}
			else if (direction == GridMapUp) {
				if (startJunction->topRoad) 
					createRoad = false;
				endPosition.row--;
			}

			if (endPosition.col < 0 || endPosition.col >= colCount) 
				createRoad = false;

			if (endPosition.row < 0 || endPosition.row >= rowCount) 
				createRoad = false;

			if (!createRoad) 
				break;

			Junction *endJunction = &map.junctions[endPosition.row * colCount + endPosition.col];

			bool endConnected;

			if (!endJunction->bottomRoad && !endJunction->topRoad &&
				!endJunction->leftRoad && !endJunction->rightRoad) {
				endConnected = false;
				connectedPositions[connectedCount++] = {endPosition.row, endPosition.col};
			}
			else {
				endConnected = true;
			}

			ConnectJunctions(startJunction, endJunction, &map.roads[createdRoadCount]);
			createdRoadCount++;

			startJunction = endJunction;
			startPosition = endPosition;

			if (endConnected) {
				if (rand() % 2 < 1) 
					break;
			}
			else {
				if (rand() % 5 < 1) 
					break;
			}
		}
	}

	for (int i = 0; i < map.roadCount; ++i) {
		Road* road = &map.roads[i];
		GenerateCrossing(road);
	}

	ArenaPopTo(tmpArena, connectedPositions);

	int maxAreaCount = (rowCount + 1) * (colCount + 1);
	map.buildings = ArenaPushArray(arena, Building, 4 * maxAreaCount);

	map.buildingCount = 0;

	float buildingPadding = junctionDistance / 10.0f;

	BuildArea** gridAreas = ArenaPushArray(tmpArena, BuildArea*, maxAreaCount);
	BuildArea* buildAreas = ArenaPushArray(tmpArena, BuildArea, maxAreaCount);
	int buildAreaCount = 0;

	for (int i = 0; i < maxAreaCount; ++i) {
		gridAreas[i] = 0;
	}

	for (int row = 0; row <= rowCount; ++row) {
		bool isNewBuilding = false;
		BuildArea newArea = {};
		bool roadAbove = false;

		newArea.top = ((float)(row - 1) * junctionDistance) + buildingPadding + leftTop.y;
		newArea.bottom = ((float)row * junctionDistance) - buildingPadding + leftTop.y;

		for (int col = 0; col <= colCount; ++col) {
			BuildArea* areaAbove = 0;
			if (row > 0) 
				areaAbove = gridAreas[(row - 1) * (colCount + 1) + (col - 1)];

			Junction* topLeftJunction = 0;
			if (row > 0 && col > 0) 
				topLeftJunction = &map.junctions[(row - 1) * colCount + (col - 1)];

			Junction* topRightJunction = 0;
			if (row > 0 && col < colCount) 
				topRightJunction = &map.junctions[(row - 1) * colCount + (col)];

			Junction* bottomLeftJunction = 0;
			if (row < rowCount && col > 0) 
				bottomLeftJunction = &map.junctions[(row) * colCount + (col - 1)];

			Junction* bottomRightJunction = 0;
			if (row < rowCount && col < colCount) 
				bottomRightJunction = &map.junctions[(row) * colCount + (col)];

			bool roadOnLeft = (topLeftJunction != 0) && (topLeftJunction->bottomRoad != 0);
			bool roadOnRight = (topRightJunction != 0) && (topRightJunction->bottomRoad != 0);
			bool roadOnTop = (topLeftJunction != 0) && (topLeftJunction->rightRoad != 0);
			bool roadOnBottom = (bottomLeftJunction != 0) && (bottomLeftJunction->rightRoad != 0);

			bool createArea = false;

			bool isNearRoad = (roadOnLeft || roadOnRight || roadOnTop || roadOnBottom);

			if (isNearRoad) {
				if (isNewBuilding) {
					if (roadOnLeft) {
						createArea = true;
						isNewBuilding = false;
					}
					else {
						newArea.right += junctionDistance;

						roadAbove |= roadOnTop;
					}
				}
			}
			else if (isNewBuilding) {
				createArea = true;
			}

			if (createArea) {
				if (!roadAbove && areaAbove &&
					areaAbove->left == newArea.left &&
					areaAbove->right == newArea.right
					) {
					areaAbove->bottom += junctionDistance;

					gridAreas[(row) * (colCount + 1) + (col - 1)] = areaAbove;
				}
				else {
					buildAreas[buildAreaCount] = newArea;
					gridAreas[(row) * (colCount + 1) + (col - 1)] = &buildAreas[buildAreaCount];
					buildAreaCount++;
				}

				isNewBuilding = false;
			}

			if (isNearRoad && !isNewBuilding) {
				isNewBuilding = true;
				newArea.left = ((float)(col - 1) * junctionDistance) + buildingPadding + leftTop.x;
				newArea.right = ((float)col * junctionDistance) - buildingPadding + leftTop.x;

				roadAbove = roadOnTop;
			}
		}
	}

	for (int i = 0; i < buildAreaCount; ++i)
		GenerateBuildings(&map, buildAreas[i], buildingPadding, junctionDistance * 0.25f, junctionDistance * 1.0f);

	ArenaPopTo(tmpArena, gridAreas);

	int i = 0;
	while (i < map.junctionCount) {
		Junction* junction = &map.junctions[i];

		int roadCount = 0;

		if (junction->leftRoad)   roadCount++;
		if (junction->rightRoad)  roadCount++;
		if (junction->topRoad)    roadCount++;
		if (junction->bottomRoad) roadCount++;

		if (roadCount == 0) {
			RemoveJunction(&map, junction);
		}
		else if (roadCount == 2 && junction->leftRoad && junction->rightRoad) {
			Road* leftRoad = junction->leftRoad;
			Road* rightRoad = junction->rightRoad;
			Junction* leftJunction = 0;
			Junction* rightJunction = 0;

			if (leftRoad->junction1 == junction)
				leftJunction = leftRoad->junction2;
			else 
				leftJunction = leftRoad->junction1;

			if (rightRoad->junction1 == junction) 
				rightJunction = rightRoad->junction2;
			else 
				rightJunction = rightRoad->junction1;

			ConnectJunctions(leftJunction, rightJunction, rightRoad);
			RemoveRoad(&map, leftRoad);
			RemoveJunction(&map, junction);
		}
		else if (roadCount == 2 && junction->topRoad && junction->bottomRoad) {
			Road* topRoad = junction->topRoad;
			Road* bottomRoad = junction->bottomRoad;
			Junction* topJunction = 0;
			Junction* bottomJunction = 0;

			if (topRoad->junction1 == junction) 
				topJunction = topRoad->junction2;
			else 
				topJunction = topRoad->junction1;

			if (bottomRoad->junction1 == junction) 
				bottomJunction = bottomRoad->junction2;
			else 
				bottomJunction = bottomRoad->junction2;

			ConnectJunctions(topJunction, bottomJunction, bottomRoad);
			RemoveRoad(&map, topRoad);
			RemoveJunction(&map, junction);
		}
		else {
			i++;
		}
	}

	for (int i = 0; i < map.junctionCount; ++i) {
		Junction* junction = &map.junctions[i];

		InitTrafficLights(junction);
	}

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];

		Point center = {};
		center.x = (building->left + building->right) * 0.5f;
		center.y = (building->top + building->bottom) * 0.5f;

		MapElem closestElem = ClosestRoadOrJunction(map, center);
		ConnectBuildingToElem(building, closestElem);

		Building* crossedBuilding = ClosestCrossedBuilding(map, building->connectPointClose, building->connectPointFar, building);
		if (crossedBuilding) {
			crossedBuilding->roadAround = 1;

			MapElem elem = BuildingElem(crossedBuilding);

			ConnectBuildingToElem(building, elem);
		}
	}

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];

		if (building->connectTreeHeight == 0)
			CalculateTreeHeight(building);

		building->type = (BuildingType)(rand() % 4);

		GenerateBuildingInside(building, arena, tmpArena);
	}

	return map;
}