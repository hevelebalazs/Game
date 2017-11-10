#include "GridMapCreator.h"
#include <math.h>
#include <time.h>

struct GridPosition {
	int row;
	int col;
};

enum GridMapDirection {
	GRIDMAP_LEFT,
	GRIDMAP_RIGHT,
	GRIDMAP_UP,
	GRIDMAP_DOWN
};

struct BuildArea {
	float left;
	float right;
	float top;
	float bottom;
};

// TODO: move this to a math file?
static float RandomBetween(float left, float right) {
	return (left)+(right - left) * ((float)rand() / (float)RAND_MAX);
}

static void GenerateBuildings(Map* map, BuildArea area, float buildingPadding, float minBuildingSide, float maxBuildingSide) {
	area.left += RandomBetween(0.0f, buildingPadding);
	area.right -= RandomBetween(0.0f, buildingPadding);
	area.top += RandomBetween(0.0f, buildingPadding);
	area.bottom -= RandomBetween(0.0f, buildingPadding);

	float areaWidth = area.right - area.left;
	float areaHeight = area.bottom - area.top;

	if (areaWidth < buildingPadding) return;
	if (areaHeight < buildingPadding) return;

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
		Building *building = &map->buildings[map->buildingCount];
		*building = {};

		building->left = area.left;
		building->right = area.right;
		building->top = area.top;
		building->bottom = area.bottom;

		building->color = Color{ 0.0f, 0.0f, 0.0f };

		map->buildingCount++;
	}
}

static void ConnectIntersections(Intersection* intersection1, Intersection* intersection2, Road* road, float roadWidth) {
	if (intersection1->coordinate.y == intersection2->coordinate.y) {
		Intersection* left;
		Intersection* right;

		if (intersection1->coordinate.x < intersection2->coordinate.x) {
			left = intersection1;
			right = intersection2;
		}
		else {
			left = intersection2;
			right = intersection1;
		}

		road->endPoint1.x = left->coordinate.x + roadWidth / 2.0f;
		road->endPoint1.y = left->coordinate.y;

		road->endPoint2.x = right->coordinate.x - roadWidth / 2.0f;
		road->endPoint2.y = right->coordinate.y;

		left->rightRoad = road;
		right->leftRoad = road;

		road->intersection1 = left;
		road->intersection2 = right;
	}
	else {
		Intersection* top;
		Intersection* bottom;

		if (intersection1->coordinate.y < intersection2->coordinate.y) {
			top = intersection1;
			bottom = intersection2;
		}
		else {
			top = intersection2;
			bottom = intersection1;
		}

		road->endPoint1.x = top->coordinate.x;
		road->endPoint1.y = top->coordinate.y + roadWidth / 2.0f;

		road->endPoint2.x = bottom->coordinate.x;
		road->endPoint2.y = bottom->coordinate.y - roadWidth / 2.0f;

		bottom->topRoad = road;
		top->bottomRoad = road;

		road->intersection1 = top;
		road->intersection2 = bottom;
	}

	road->width = roadWidth;
}

Map CreateGridMap(float width, float height, float intersectionDistance) {
	Map map;

	map.width = width;
	map.height = height;

	int colCount = (int)(width / intersectionDistance);
	int rowCount = (int)(height / intersectionDistance);

	float rightOffset = width - ((float)(colCount - 1) * intersectionDistance);
	float bottomOffset = height - ((float)(rowCount - 1) * intersectionDistance);

	Point leftTop = { rightOffset / 2.0f, bottomOffset / 2.0f };

	int intersectionCount = colCount * rowCount;

	map.intersections = new Intersection[intersectionCount];
	map.intersectionCount = intersectionCount;

	int intersectionIndex = 0;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount; ++col) {
			map.intersections[intersectionIndex].coordinate = 
				leftTop + (Point{ (float)col, (float)row } * (float)intersectionDistance);

			++intersectionIndex;
		}
	}

	GridPosition* connectedPositions= new GridPosition[intersectionCount];
	int connectedCount = 0;

	int maxRoadCount = colCount * (rowCount - 1) + (colCount - 1) * rowCount;
	int roadCount = maxRoadCount / 2;
	int createdRoadCount = 0;

	map.roads = new Road[roadCount];
	map.roadCount = roadCount;

	srand((unsigned int)time(0));

	float roadWidth = intersectionDistance / 5;
	
	int startRow = rand() % rowCount;
	int startCol = rand() % colCount;

	connectedPositions[connectedCount++] = { startRow, startCol };

	while (createdRoadCount < roadCount) {
		GridPosition startPosition = connectedPositions[rand() % connectedCount];
		Intersection* startIntersection =
			&map.intersections[startPosition.row * colCount + startPosition.col];

		int direction = rand() % 4;

		while (createdRoadCount < roadCount) {
			bool createRoad = true;

			GridPosition endPosition = startPosition;

			if (direction == GRIDMAP_RIGHT) {
				if (startIntersection->rightRoad) createRoad = false;
				endPosition.col++;
			}
			else if (direction == GRIDMAP_LEFT) {
				if (startIntersection->leftRoad) createRoad = false;
				endPosition.col--;
			}
			else if (direction == GRIDMAP_DOWN) {
				if (startIntersection->bottomRoad) createRoad = false;
				endPosition.row++;
			}
			else if (direction == GRIDMAP_UP) {
				if (startIntersection->topRoad) createRoad = false;
				endPosition.row--;
			}

			if (endPosition.col < 0 || endPosition.col >= colCount) createRoad = false;
			if (endPosition.row < 0 || endPosition.row >= rowCount) createRoad = false;

			if (!createRoad) break;

			Intersection *endIntersection =
				&map.intersections[endPosition.row * colCount + endPosition.col];

			bool endConnected;

			if (!endIntersection->bottomRoad && !endIntersection->topRoad &&
				!endIntersection->leftRoad && !endIntersection->rightRoad) {
				endConnected = false;
				connectedPositions[connectedCount++] = { endPosition.row, endPosition.col };
			}
			else {
				endConnected = true;
			}

			ConnectIntersections(startIntersection, endIntersection, &map.roads[createdRoadCount], roadWidth);
			createdRoadCount++;

			startIntersection = endIntersection;
			startPosition = endPosition;
			
			if (endConnected) {
				if (rand() % 2 < 1) break;
			}
			else {
				if (rand() % 5 < 1) break;
			}
		}
	}

	delete[] connectedPositions;

	int maxAreaCount = (rowCount + 1) * (colCount + 1);
	map.buildings = new Building[4 * maxAreaCount];

	map.buildingCount = 0;

	float buildingPadding = intersectionDistance / 10.0f;

	BuildArea** gridAreas = new BuildArea *[maxAreaCount];

	BuildArea* buildAreas = new BuildArea[maxAreaCount];
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
			if (row > 0) areaAbove = gridAreas[(row - 1) * (colCount + 1) + (col - 1)];

			Intersection* topLeftIntersection = 0;
			if (row > 0 && col > 0) topLeftIntersection = &map.intersections[(row - 1) * colCount + (col - 1)];

			Intersection* topRightIntersection = 0;
			if (row > 0 && col < colCount) topRightIntersection = &map.intersections[(row - 1) * colCount + (col)];

			Intersection* bottomLeftIntersection = 0;
			if (row < rowCount && col > 0) bottomLeftIntersection = &map.intersections[(row)* colCount + (col - 1)];

			Intersection* bottomRightIntersection = 0;
			if (row < rowCount && col < colCount) bottomRightIntersection = &map.intersections[(row)* colCount + (col)];

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
			else if(isNewBuilding) {
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

	for (int i = 0; i < buildAreaCount; ++i) {
		GenerateBuildings(&map, buildAreas[i], buildingPadding, intersectionDistance / 4.0f, intersectionDistance * 2.0f);
	}

	delete[] gridAreas;
	delete[] buildAreas;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];

		Point center = {};
		center.x = (building->left + building->right) * 0.5f;
		center.y = (building->top + building->bottom) * 0.5f;

		Road* closestRoad = map.ClosestRoad(center);
		if (closestRoad) {
			building->connectPointFar = closestRoad->ClosestPoint(center);
		} 
		else {
			building->connectPointFar = center;
		}
	}

	Building::connectRoadWidth = roadWidth / 5.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];

		Point center = {};
		center.x = (building->left + building->right) * 0.5f;
		center.y = (building->top + building->bottom) * 0.5f;

		Road* closestRoad = map.ClosestRoad(center);
		if (closestRoad) {
			building->connectPointFar = closestRoad->ClosestPoint(center);
			building->connectPointFarShow = building->connectPointFar;

			building->connectRoad = closestRoad;

			building->connectPointClose = center;
			if (closestRoad->endPoint1.x == closestRoad->endPoint2.x) {
				building->connectPointClose.y = building->connectPointFar.y;

				if (closestRoad->endPoint1.x < center.x) {
					building->connectPointClose.x = building->left;
					building->connectPointFarShow.x += closestRoad->width * 0.5f;
				}
				else {
					building->connectPointClose.x = building->right;
					building->connectPointFarShow.x -= closestRoad->width * 0.5f;
				}
			}
			else if (closestRoad->endPoint1.y == closestRoad->endPoint2.y) {
				building->connectPointClose.x = building->connectPointFar.x;

				if (closestRoad->endPoint1.y < center.y) {
					building->connectPointClose.y = building->top;
					building->connectPointFarShow.y += closestRoad->width * 0.5f;
				}
				else {
					building->connectPointClose.y = building->bottom;
					building->connectPointFarShow.y -= closestRoad->width * 0.5f;
				}
			}
		} else {
			building->connectPointFar = center;
			building->connectPointFarShow = center;
			building->connectPointClose = center;
		}

		Building* crossedBuilding = map.CrossedBuilding(building->connectPointClose, building->connectPointFar, building);

		if (crossedBuilding) {
			crossedBuilding->roadAround = true;

			building->connectPointFar = crossedBuilding->ClosestCrossPoint(building->connectPointClose, building->connectPointFar);
			building->connectPointFarShow = building->connectPointFar;

			building->connectRoad = 0;
			building->connectBuilding = crossedBuilding;
		}
	}

	int realIntersectionCount = 0;
	for (int i = 0; i < intersectionCount; ++i) {
		Intersection* oldIntersection = &map.intersections[i];
		Intersection* newIntersection = &map.intersections[realIntersectionCount];
		int isIntersectionReal = 0;
		
		if (oldIntersection->leftRoad) {
			if (oldIntersection->leftRoad->intersection1 == oldIntersection) {
				oldIntersection->leftRoad->intersection1 = newIntersection;
			}
			else {
				oldIntersection->leftRoad->intersection2 = newIntersection;
			}
			isIntersectionReal = 1;
		}

		if (oldIntersection->rightRoad) {
			if (oldIntersection->rightRoad->intersection1 == oldIntersection) {
				oldIntersection->rightRoad->intersection1 = newIntersection;
			}
			else {
				oldIntersection->rightRoad->intersection2 = newIntersection;
			}
			isIntersectionReal = 1;
		}

		if (oldIntersection->topRoad) {
			if (oldIntersection->topRoad->intersection1 == oldIntersection) {
				oldIntersection->topRoad->intersection1 = newIntersection;
			}
			else {
				oldIntersection->topRoad->intersection2 = newIntersection;
			}
			isIntersectionReal = 1;
		}

		if (oldIntersection->bottomRoad) {
			if (oldIntersection->bottomRoad->intersection1 == oldIntersection) {
				oldIntersection->bottomRoad->intersection1 = newIntersection;
			}
			else {
				oldIntersection->bottomRoad->intersection2 = newIntersection;
			}
			isIntersectionReal = 1;
		}

		if (isIntersectionReal) {
			*newIntersection = *oldIntersection;
			realIntersectionCount++;
		}
	}

	map.intersectionCount = realIntersectionCount;

	return map;
}