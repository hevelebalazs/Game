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

static void ConnectIntersections(Intersection* intersection1, Intersection* intersection2, Road* road, float roadWidth) {
	if (intersection1->position.y == intersection2->position.y) {
		Intersection* left;
		Intersection* right;

		if (intersection1->position.x < intersection2->position.x) {
			left = intersection1;
			right = intersection2;
		}
		else {
			left = intersection2;
			right = intersection1;
		}

		road->endPoint1.x = left->position.x + roadWidth / 2.0f;
		road->endPoint1.y = left->position.y;

		road->endPoint2.x = right->position.x - roadWidth / 2.0f;
		road->endPoint2.y = right->position.y;

		left->rightRoad = road;
		right->leftRoad = road;

		road->intersection1 = left;
		road->intersection2 = right;
	}
	else {
		Intersection* top;
		Intersection* bottom;

		if (intersection1->position.y < intersection2->position.y) {
			top = intersection1;
			bottom = intersection2;
		}
		else {
			top = intersection2;
			bottom = intersection1;
		}

		road->endPoint1.x = top->position.x;
		road->endPoint1.y = top->position.y + roadWidth / 2.0f;

		road->endPoint2.x = bottom->position.x;
		road->endPoint2.y = bottom->position.y - roadWidth / 2.0f;

		bottom->topRoad = road;
		top->bottomRoad = road;

		road->intersection1 = top;
		road->intersection2 = bottom;
	}

	road->width = roadWidth;
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

static void ReindexRoad(Road* road, Intersection* oldIntersection, Intersection* newIntersection) {
	if (road->intersection1 == oldIntersection) 
		road->intersection1 = newIntersection;

	if (road->intersection2 == oldIntersection) 
		road->intersection2 = newIntersection;
}

static void RemoveIntersection(Map* map, Intersection* intersection) {
	Intersection* oldIntersection = &map->intersections[map->intersectionCount - 1];
	map->intersectionCount--;

	*intersection = *oldIntersection;

	if (intersection->leftRoad)   
		ReindexRoad(intersection->leftRoad,   oldIntersection, intersection);

	if (intersection->rightRoad)  
		ReindexRoad(intersection->rightRoad,  oldIntersection, intersection);

	if (intersection->topRoad)  
		ReindexRoad(intersection->topRoad,    oldIntersection, intersection);

	if (intersection->bottomRoad)
		ReindexRoad(intersection->bottomRoad, oldIntersection, intersection);
}

static void ReindexIntersection(Intersection* intersection, Road* oldRoad, Road* road) {
	if (intersection->leftRoad == oldRoad) 
		intersection->leftRoad = road;

	if (intersection->rightRoad == oldRoad) 
		intersection->rightRoad = road;

	if (intersection->topRoad == oldRoad) 
		intersection->topRoad = road;

	if (intersection->bottomRoad == oldRoad) 
		intersection->bottomRoad = road;
}

static void RemoveRoad(Map* map, Road* road) {
	Road* oldRoad = &map->roads[map->roadCount - 1];
	map->roadCount--;

	*road = *oldRoad;

	if (road->intersection1) 
		ReindexIntersection(road->intersection1, oldRoad, road);

	if (road->intersection2) 
		ReindexIntersection(road->intersection2, oldRoad, road);
}

// TODO: separate this into smaller functions?
// TODO: sometimes a building is not connected to anything, find out why this is
Map CreateGridMap(float width, float height, float intersectionDistance, MemArena* arena, MemArena* tmpArena) {
	Map map = {};
	map.width = width;
	map.height = height;

	int colCount = (int)(width / intersectionDistance);
	int rowCount = (int)(height / intersectionDistance);

	float rightOffset = width - ((float)(colCount - 1) * intersectionDistance);
	float bottomOffset = height - ((float)(rowCount - 1) * intersectionDistance);

	Point leftTop = {rightOffset / 2.0f, bottomOffset / 2.0f};

	int intersectionCount = colCount * rowCount;

	map.intersections = ArenaPushArray(arena, Intersection, intersectionCount);
	map.intersectionCount = intersectionCount;

	int intersectionIndex = 0;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount; ++col) {
			map.intersections[intersectionIndex] = Intersection{};
			map.intersections[intersectionIndex].position = PointSum(
				leftTop,
				PointProd((float)intersectionDistance, Point{(float)col, (float)row})
			);

			++intersectionIndex;
		}
	}

	GridPosition* connectedPositions = ArenaPushArray(tmpArena, GridPosition, intersectionCount);
	int connectedCount = 0;

	int maxRoadCount = colCount * (rowCount - 1) + (colCount - 1) * rowCount;
	int roadCount = maxRoadCount / 2;
	int createdRoadCount = 0;

	map.roads = ArenaPushArray(arena, Road, roadCount);
	map.roadCount = roadCount;

	InitRandom();

	float roadWidth = intersectionDistance / 5.0f;

	int startRow = rand() % rowCount;
	int startCol = rand() % colCount;

	connectedPositions[connectedCount++] = {startRow, startCol};

	while (createdRoadCount < roadCount) {
		GridPosition startPosition = connectedPositions[rand() % connectedCount];
		Intersection* startIntersection =
			&map.intersections[startPosition.row * colCount + startPosition.col];

		int direction = rand() % 4;

		while (createdRoadCount < roadCount) {
			bool createRoad = true;

			GridPosition endPosition = startPosition;

			if (direction == GridMapRight) {
				if (startIntersection->rightRoad) 
					createRoad = false;
				endPosition.col++;
			}
			else if (direction == GridMapLeft) {
				if (startIntersection->leftRoad) 
					createRoad = false;
				endPosition.col--;
			}
			else if (direction == GridMapDown) {
				if (startIntersection->bottomRoad) 
					createRoad = false;
				endPosition.row++;
			}
			else if (direction == GridMapUp) {
				if (startIntersection->topRoad) 
					createRoad = false;
				endPosition.row--;
			}

			if (endPosition.col < 0 || endPosition.col >= colCount) 
				createRoad = false;

			if (endPosition.row < 0 || endPosition.row >= rowCount) 
				createRoad = false;

			if (!createRoad) 
				break;

			Intersection *endIntersection = &map.intersections[endPosition.row * colCount + endPosition.col];

			bool endConnected;

			if (!endIntersection->bottomRoad && !endIntersection->topRoad &&
				!endIntersection->leftRoad && !endIntersection->rightRoad) {
				endConnected = false;
				connectedPositions[connectedCount++] = {endPosition.row, endPosition.col};
			}
			else {
				endConnected = true;
			}

			ConnectIntersections(startIntersection, endIntersection, &map.roads[createdRoadCount], roadWidth);
			createdRoadCount++;

			startIntersection = endIntersection;
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

	float buildingPadding = intersectionDistance / 10.0f;

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

		newArea.top = ((float)(row - 1) * intersectionDistance) + buildingPadding + leftTop.y;
		newArea.bottom = ((float)row * intersectionDistance) - buildingPadding + leftTop.y;

		for (int col = 0; col <= colCount; ++col) {
			BuildArea* areaAbove = 0;
			if (row > 0) 
				areaAbove = gridAreas[(row - 1) * (colCount + 1) + (col - 1)];

			Intersection* topLeftIntersection = 0;
			if (row > 0 && col > 0) 
				topLeftIntersection = &map.intersections[(row - 1) * colCount + (col - 1)];

			Intersection* topRightIntersection = 0;
			if (row > 0 && col < colCount) 
				topRightIntersection = &map.intersections[(row - 1) * colCount + (col)];

			Intersection* bottomLeftIntersection = 0;
			if (row < rowCount && col > 0) 
				bottomLeftIntersection = &map.intersections[(row) * colCount + (col - 1)];

			Intersection* bottomRightIntersection = 0;
			if (row < rowCount && col < colCount) 
				bottomRightIntersection = &map.intersections[(row) * colCount + (col)];

			bool roadOnLeft = (topLeftIntersection != 0) && (topLeftIntersection->bottomRoad != 0);
			bool roadOnRight = (topRightIntersection != 0) && (topRightIntersection->bottomRoad != 0);
			bool roadOnTop = (topLeftIntersection != 0) && (topLeftIntersection->rightRoad != 0);
			bool roadOnBottom = (bottomLeftIntersection != 0) && (bottomLeftIntersection->rightRoad != 0);

			bool createArea = false;

			bool isNearRoad = (roadOnLeft || roadOnRight || roadOnTop || roadOnBottom);

			if (isNearRoad) {
				if (isNewBuilding) {
					if (roadOnLeft) {
						createArea = true;
						isNewBuilding = false;
					}
					else {
						newArea.right += intersectionDistance;

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
					areaAbove->bottom += intersectionDistance;

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
				newArea.left = ((float)(col - 1) * intersectionDistance) + buildingPadding + leftTop.x;
				newArea.right = ((float)col * intersectionDistance) - buildingPadding + leftTop.x;

				roadAbove = roadOnTop;
			}
		}
	}

	for (int i = 0; i < buildAreaCount; ++i)
		GenerateBuildings(&map, buildAreas[i], buildingPadding, intersectionDistance * 0.25f, intersectionDistance * 1.0f);

	ArenaPopTo(tmpArena, gridAreas);

	int i = 0;
	while (i < map.intersectionCount) {
		Intersection* intersection = &map.intersections[i];

		int roadCount = 0;

		if (intersection->leftRoad)   roadCount++;
		if (intersection->rightRoad)  roadCount++;
		if (intersection->topRoad)    roadCount++;
		if (intersection->bottomRoad) roadCount++;

		if (roadCount == 0) {
			RemoveIntersection(&map, intersection);
		}
		else if (roadCount == 2 && intersection->leftRoad && intersection->rightRoad) {
			Road* leftRoad = intersection->leftRoad;
			Road* rightRoad = intersection->rightRoad;
			Intersection* leftIntersection = 0;
			Intersection* rightIntersection = 0;

			if (leftRoad->intersection1 == intersection)
				leftIntersection = leftRoad->intersection2;
			else 
				leftIntersection = leftRoad->intersection1;

			if (rightRoad->intersection1 == intersection) 
				rightIntersection = rightRoad->intersection2;
			else 
				rightIntersection = rightRoad->intersection1;

			ConnectIntersections(leftIntersection, rightIntersection, rightRoad, roadWidth);
			RemoveRoad(&map, leftRoad);
			RemoveIntersection(&map, intersection);
		}
		else if (roadCount == 2 && intersection->topRoad && intersection->bottomRoad) {
			Road* topRoad = intersection->topRoad;
			Road* bottomRoad = intersection->bottomRoad;
			Intersection* topIntersection = 0;
			Intersection* bottomIntersection = 0;

			if (topRoad->intersection1 == intersection) 
				topIntersection = topRoad->intersection2;
			else 
				topIntersection = topRoad->intersection1;

			if (bottomRoad->intersection1 == intersection) 
				bottomIntersection = bottomRoad->intersection2;
			else 
				bottomIntersection = bottomRoad->intersection2;

			ConnectIntersections(topIntersection, bottomIntersection, bottomRoad, roadWidth);
			RemoveRoad(&map, topRoad);
			RemoveIntersection(&map, intersection);
		}
		else {
			i++;
		}
	}

	for (int i = 0; i < map.intersectionCount; ++i) {
		Intersection* intersection = &map.intersections[i];

		InitTrafficLights(intersection);
	}

	connectRoadWidth = roadWidth / 5.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];

		Point center = {};
		center.x = (building->left + building->right) * 0.5f;
		center.y = (building->top + building->bottom) * 0.5f;

		MapElem closestElem = ClosestRoadOrIntersection(map, center);
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