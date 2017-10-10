#include "GridMapCreator.h"
#include <stdio.h>
#include <time.h>

Map GridMapCreator::generateGrid() {
	Map newMap;
	newMap.width = width;
	newMap.height = height;
	srand(time(0));
	if (width == 0 || height == 0)
		printf("Map.generate: width and height must be greater than 0!");
	int rowCount = height / intersectionDistance;
	int colCount = width / intersectionDistance;
	newMap.intersections = getIntersections(rowCount, colCount);
	newMap.intersectionCount = colCount * rowCount;
	newMap.roads = getRoads(rowCount, colCount, newMap.intersections);
	newMap.roadCount = createdRoadCount;
	return newMap;
}

int getIntersectionIndex(int row, int col, int colCount) {
	return row * colCount + col;
}

Intersection* GridMapCreator::getIntersections(int rowCount, int colCount) {
	int intersectionCount = colCount * rowCount;
	Intersection* intersections = new Intersection[intersectionCount];
	for (int row = 0; row < rowCount; ++row)
		for (int col = 0; col < colCount; ++col) {
			Point currentIntersectionCoordinate = { col * intersectionDistance, row * intersectionDistance };
			intersections[getIntersectionIndex(row, col, colCount)].coordinate = currentIntersectionCoordinate;
		}
	return intersections;
}

bool isRoadNeeded(int roadCount, int neededRoadCount) {
	return rand() % roadCount < neededRoadCount;
}

Road* GridMapCreator::getRoads(int rowCount, int colCount, Intersection* intersections) {
	int remainingRoadCount = colCount * (rowCount - 1) + rowCount * (colCount - 1);
	int neededRoadCount = remainingRoadCount / 2;
	Road* roads = new Road[neededRoadCount];

	int roadWidth = intersectionDistance / 10;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount - 1; ++col) {
			if (isRoadNeeded(remainingRoadCount, neededRoadCount))
			{
				Intersection* leftIntersection = &intersections[getIntersectionIndex(row, col, colCount)];
				Intersection* rightIntersection = &intersections[getIntersectionIndex(row, col + 1, colCount)];
				roads[createdRoadCount] = createGridHorizontalRoad(roadWidth, leftIntersection, rightIntersection);
				leftIntersection->rightRoad = &roads[createdRoadCount];
				rightIntersection->leftRoad = &roads[createdRoadCount];
				--neededRoadCount;
				++createdRoadCount;
			}
			--remainingRoadCount;
		}
	}

	for (int row = 0; row < rowCount - 1; ++row) {
		for (int col = 0; col < colCount; ++col) {
			if (isRoadNeeded(remainingRoadCount, neededRoadCount))
			{
				Intersection* topIntersection = &intersections[getIntersectionIndex(row, col, colCount)];
				Intersection* bottomIntersection = &intersections[getIntersectionIndex(row + 1, col, colCount)];
				roads[createdRoadCount] =  createGridVerticalRoad(roadWidth, topIntersection, bottomIntersection);
				topIntersection->bottomRoad = &roads[createdRoadCount];
				bottomIntersection->topRoad = &roads[createdRoadCount];
				--neededRoadCount;
				++createdRoadCount;
			}
			--remainingRoadCount;
		}
	}
	return roads;
}

Road GridMapCreator::createGridHorizontalRoad(int roadWidth, Intersection* leftIntersection, Intersection* rightIntersection) {
	Point leftPoint = { leftIntersection->coordinate.x + roadWidth / 2, leftIntersection->coordinate.y };
	Point rightPoint = { rightIntersection->coordinate.x - roadWidth / 2, leftIntersection->coordinate.y };
	Road newRoad = { leftPoint, rightPoint, roadWidth };
	return newRoad;
}

Road GridMapCreator::createGridVerticalRoad(int roadWidth, Intersection* topIntersection, Intersection* bottomIntersection) {
	
	Point topPoint = { topIntersection->coordinate.x, topIntersection->coordinate.y + roadWidth / 2 };
	Point bottomPoint = { bottomIntersection->coordinate.x, bottomIntersection->coordinate.y - roadWidth / 2 };
	Road newRoad = { topPoint, bottomPoint, roadWidth };
	return newRoad;
}