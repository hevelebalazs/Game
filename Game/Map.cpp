#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Map::draw(HDC context) {
	HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
	RECT rect = { 0, 0, width, height };

	FillRect(context, &rect, brush);

	for (int i = 0; i < roadCount; ++i)
		roads[i].draw(context);
}

void Map::generateGrid(float intersectionDistance) {
	srand(time(0));
	if (width == 0 || height == 0)
		printf("Map.generate: width and height must be greater than 0!");
	int rowCount = height / intersectionDistance;
	int colCount = width / intersectionDistance;
	initIntersections(rowCount, colCount, intersectionDistance);
	initRoads(rowCount, colCount, intersectionDistance);
}

int getIntersectionIndex(int row, int col, int colCount) {
	return row * colCount + col;
}

void Map::initIntersections(int rowCount, int colCount, float intersectionDistance) {
	intersectionCount = colCount * rowCount;
	intersections = new Intersection[intersectionCount];
	for (int row = 0; row < rowCount; ++row)
		for (int col = 0; col < colCount; ++col) {
			Point currentIntersectionCoordinate = { col * intersectionDistance, row * intersectionDistance };
			intersections[getIntersectionIndex(row, col, colCount)].coordinate = currentIntersectionCoordinate;
		}
}

bool isRoadNeeded(int roadCount, int neededRoadCount) {
	return rand() % roadCount < neededRoadCount;
}

void Map::initRoads(int rowCount, int colCount, float intersectionDistance) {
	int remainingRoadCount = colCount * (rowCount - 1) + rowCount * (colCount - 1);
	int neededRoadCount = remainingRoadCount / 2;
	roads = new Road[neededRoadCount];

	TMProadCount = neededRoadCount;

	int roadWidth = intersectionDistance / 10;
	for (int row = 0; row < rowCount; ++row) {
		for (int col = 0; col < colCount - 1; ++col) {
			if (isRoadNeeded(remainingRoadCount, neededRoadCount))
			{
				createGridHorizontalRoad(row, col, colCount, roadWidth);
				--neededRoadCount;
				++roadCount;
			}
			--remainingRoadCount;
		}
	}

	for (int row = 0; row < rowCount - 1; ++row) {
		for (int col = 0; col < colCount; ++col) {
			if (isRoadNeeded(remainingRoadCount, neededRoadCount))
			{
				createGridVerticalRoad(row, col, colCount, roadWidth);
				--neededRoadCount;
				++roadCount;
			}
			--remainingRoadCount;
		}
	}
}

void Map::createGridHorizontalRoad(int row, int col, int colCount, int roadWidth) {
	Intersection* leftIntersection = &intersections[getIntersectionIndex(row, col, colCount)];
	Intersection* rightIntersection = &intersections[getIntersectionIndex(row, col + 1, colCount)];
	Point leftPoint = { leftIntersection->coordinate.x + roadWidth / 2, leftIntersection->coordinate.y };
	Point rightPoint = { rightIntersection->coordinate.x - roadWidth / 2, leftIntersection->coordinate.y };
	Road newRoad = { leftPoint, rightPoint, roadWidth };
	roads[roadCount] = newRoad;
	leftIntersection->rightRoad = &roads[roadCount];
	rightIntersection->leftRoad = &roads[roadCount];
}

void Map::createGridVerticalRoad(int row, int col, int colCount, int roadWidth) {
	Intersection* topIntersection = &intersections[getIntersectionIndex(row, col, colCount)];
	Intersection* bottomIntersection = &intersections[getIntersectionIndex(row + 1, col, colCount)];
	Point topPoint = { topIntersection->coordinate.x, topIntersection->coordinate.y + roadWidth / 2 };
	Point bottomPoint = { bottomIntersection->coordinate.x, bottomIntersection->coordinate.y - roadWidth / 2};
	Road newRoad = { topPoint, bottomPoint, roadWidth };
	roads[roadCount] = newRoad;
	topIntersection->bottomRoad = &roads[roadCount];
	bottomIntersection->topRoad = &roads[roadCount];
}