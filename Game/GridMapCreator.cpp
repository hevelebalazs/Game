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

	int colCount = (int)(width / intersectionDistance) + 1;
	int rowCount = (int)(height / intersectionDistance) + 1;

	int intersectionCount = colCount * rowCount;

	map.intersections = new Intersection[intersectionCount];
	map.intersectionCount = intersectionCount;

	int intersectionIndex = 0;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount; ++col) {
			map.intersections[intersectionIndex].coordinate.x = col * intersectionDistance;
			map.intersections[intersectionIndex].coordinate.y = row * intersectionDistance;

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

	return map;
}