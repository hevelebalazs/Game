#include "GridMapCreator.h"
#include <math.h>
#include <time.h>

struct GridPosition {
	int row;
	int col;
};

static void connectIntersections(Intersection *intersection1, Intersection *intersection2, Road *road, float roadWidth) {
	if (intersection1->coordinate.y == intersection2->coordinate.y) {
		Intersection *left, *right;

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
		Intersection *top, *bottom;

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

enum GridMapDirection {
	GRIDMAP_LEFT,
	GRIDMAP_RIGHT,
	GRIDMAP_UP,
	GRIDMAP_DOWN
};

Map createGridMap(float width, float height, float intersectionDistance) {
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

	GridPosition *connectedPositions= new GridPosition[intersectionCount];
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

			connectIntersections(startIntersection, endIntersection, &map.roads[createdRoadCount], roadWidth);
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

	int maxBuildingCount = (rowCount + 1) * (colCount + 1);
	map.buildings = new Building[maxBuildingCount];

	map.buildingCount = 0;

	float buildingPadding = intersectionDistance / 5.0f;

	Building **gridBuilding = new Building*[maxBuildingCount];

	for (int i = 0; i < maxBuildingCount; ++i) gridBuilding[i] = 0;

	for (int row = 0; row <= rowCount; ++row) {
		bool isNewBuilding = false;
		Building newBuilding = {};
		bool roadAbove = false;

		newBuilding.top = ((float)(row - 1) * intersectionDistance) + buildingPadding + leftTop.y;
		newBuilding.bottom = ((float)row * intersectionDistance) - buildingPadding + leftTop.y;

		for (int col = 0; col <= colCount; ++col) {
			Intersection *topLeftIntersection = 0;
			if (row > 0 && col > 0) topLeftIntersection = &map.intersections[(row - 1) * colCount + (col - 1)];

			Intersection *topRightIntersection = 0;
			if (row > 0 && col < colCount) topRightIntersection = &map.intersections[(row - 1) * colCount + (col)];

			Intersection *bottomLeftIntersection = 0;
			if (row < rowCount && col > 0) bottomLeftIntersection = &map.intersections[(row)* colCount + (col - 1)];

			Intersection *bottomRightIntersection = 0;
			if (row < rowCount && col < colCount) bottomRightIntersection = &map.intersections[(row)* colCount + (col)];

			bool roadOnLeft = (topLeftIntersection != 0) && (topLeftIntersection->bottomRoad != 0);
			bool roadOnRight = (topRightIntersection != 0) && (topRightIntersection->bottomRoad != 0);
			bool roadOnTop = (topLeftIntersection != 0) && (topLeftIntersection->rightRoad != 0);
			bool roadOnBottom = (bottomLeftIntersection != 0) && (bottomLeftIntersection->rightRoad != 0);

			bool createBuilding = false;

			bool isNearRoad = (roadOnLeft || roadOnRight || roadOnTop || roadOnBottom);

			if (isNearRoad) {
				if (isNewBuilding) {
					if (roadOnLeft) {
						createBuilding = true;
						isNewBuilding = false;
					}
					else {
						newBuilding.right += intersectionDistance;

						roadAbove |= roadOnTop;
					}
				}
			}
			else if(isNewBuilding) {
				createBuilding = true;
			}

			if (createBuilding) {
				Building *buildingAbove = 0;
				if (row != 0) buildingAbove = gridBuilding[(row - 1) * (colCount - 1) + (col)];

				if (!roadAbove && buildingAbove && 
					buildingAbove->left == newBuilding.left && 
					buildingAbove->right == newBuilding.right
				) {
					buildingAbove->bottom += intersectionDistance;

					gridBuilding[(row) * (colCount - 1) + (col)] = buildingAbove;
				}
				else {
					newBuilding.color = Color{ 0.0f, 0.0f, 0.0f };
					map.buildings[map.buildingCount] = newBuilding;

					gridBuilding[(row) * (colCount - 1) + (col)] = &map.buildings[map.buildingCount];

					map.buildingCount++;
				}

				isNewBuilding = false;
			}

			if (isNearRoad && !isNewBuilding) {
				isNewBuilding = true;
				newBuilding.left = ((float)(col - 1) * intersectionDistance) + buildingPadding + leftTop.x;
				newBuilding.right = ((float)col * intersectionDistance) - buildingPadding + leftTop.x;

				roadAbove = roadOnTop;
			}
		}
	}

	delete[] gridBuilding;

	int realIntersectionCount = 0;
	for (int i = 0; i < intersectionCount; ++i) {
		Intersection *oldIntersection = &map.intersections[i];
		Intersection *newIntersection = &map.intersections[realIntersectionCount];
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