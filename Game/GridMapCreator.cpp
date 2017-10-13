#include "GridMapCreator.h"

GridPosition lineDirections[4] = { { 1, 0 },{ 0, 1 },{ -1, 0 },{ 0, -1 } };

Map generateGrid(float width, float height, float intersectionDistance) {
	Map newMap;
	newMap.width = width;
	newMap.height = height;
	srand(time(0));
	int rowCount = height / intersectionDistance;
	int colCount = width / intersectionDistance;
	vector<Intersection> newIntersections = generateIntersections(intersectionDistance, rowCount, colCount);
	vector<Road> newRoads = generateRoads(intersectionDistance, rowCount, colCount, &newIntersections, 10);
	newMap.intersections = new Intersection[newIntersections.size()];
	memcpy(newMap.intersections, newIntersections.data(), newIntersections.size() * sizeof(newIntersections));
	newMap.intersectionCount = newIntersections.size();
	newMap.roads = new Road[newRoads.size()];
	memcpy(newMap.roads, newRoads.data(), newRoads.size() * sizeof(Road));
	newMap.roadCount = newRoads.size();
	return newMap;
}

vector<Intersection> generateIntersections(float intersectionDistance, int rowCount, int colCount) {
	int intersectionCount = colCount * rowCount;
	vector<Intersection> intersections;
	for (int row = 0; row < rowCount; ++row)
		for (int col = 0; col < colCount; ++col) {
			Point currentIntersectionCoordinate = { col * intersectionDistance, row * intersectionDistance };
			intersections.push_back({ currentIntersectionCoordinate });
		}
	return intersections;
}

vector<Road> generateRoads(float intersectionDistance, int rowCount, int colCount, vector<Intersection>* intersections, int maxRoadCount) {
	vector<Road> newRoads;
	vector<GridPosition> connectedGridPositions;
	float roadWidth = intersectionDistance / 10;
	connectedGridPositions.push_back({ rand() % colCount, rand() % rowCount });
	while (newRoads.size() < maxRoadCount) {
		GridPosition startPosition = getRandomGridPosition(connectedGridPositions);
		vector<Road> newRoadLine = generateRoadLine(roadWidth, rowCount, colCount, intersections, connectedGridPositions, startPosition, createNewLineDirection());
		newRoads.insert(newRoads.end(), newRoadLine.begin(), newRoadLine.end());
	}
	return newRoads;
}

GridPosition getRandomGridPosition(vector<GridPosition> gridPositions) {
	if (gridPositions.size() == 0)
		throw new exception("The gridPositions must contains elements!");
	else if (gridPositions.size() == 1)
		return gridPositions[0];
	else {
		int randomIndex = rand() % (gridPositions.size() - 1);
		return gridPositions[randomIndex];
	}
}

vector<Road> generateRoadLine(float roadWidth, int rowCount, int colCount, vector<Intersection>* intersections, vector<GridPosition> connectedGridPositions, GridPosition startGridPosition, GridPosition lineDirection) {
	vector<Road> newRoads;
	vector<GridPosition> newConnectedGridPositions;
	GridPosition currentGridPosition = startGridPosition;
	GridPosition secondGridPosition = currentGridPosition + lineDirection;
	while (isValidGridPosition(secondGridPosition, rowCount, colCount) && !isLineStoped(secondGridPosition, connectedGridPositions) && !hasRoad(currentGridPosition, secondGridPosition, *intersections, colCount)) {
		Intersection* currentIntersection = &intersections->at(getIntersectionIndex(currentGridPosition, colCount));
		Intersection* secondIntersection = &intersections->at(getIntersectionIndex(secondGridPosition, colCount));
		newRoads.push_back(createGridRoad(roadWidth, currentIntersection, secondIntersection));
		newConnectedGridPositions.push_back(secondGridPosition);
		currentGridPosition = secondGridPosition;
		secondGridPosition = currentGridPosition + lineDirection;
	}
	connectedGridPositions.insert(connectedGridPositions.end(), newConnectedGridPositions.begin(), newConnectedGridPositions.end());
	return newRoads;
}

bool isValidGridPosition(GridPosition position, int rowCount, int colCount) {
	return position.col >= 0 && position.col < colCount && position.row >= 0 && position.row < rowCount;
}

bool isLineStoped(GridPosition currentGridPosition, vector<GridPosition> connectedGridPositions) {
	if (find(connectedGridPositions.begin(), connectedGridPositions.end(), currentGridPosition) != connectedGridPositions.end()) {
		bool stopInIntersection = rand() % 2;
		return stopInIntersection;
	}
	else
		return false;
}

bool hasRoad(GridPosition gridPosition1, GridPosition gridPosition2, vector<Intersection> intersections, int colCount) {
	return hasRoad(intersections[getIntersectionIndex(gridPosition1, colCount)], intersections[getIntersectionIndex(gridPosition2, colCount)]);
}

bool hasRoad(Intersection intersection1, Intersection intersection2) {
	if (intersection1.coordinate.x == intersection2.coordinate.x)
		if (intersection1.coordinate.y > intersection2.coordinate.y)
			return hasVerticalRoad(intersection2, intersection1);
		else
			return hasVerticalRoad(intersection1, intersection2);
	else if (intersection1.coordinate.y == intersection2.coordinate.y)
		if (intersection1.coordinate.x > intersection2.coordinate.x)
			return hasHorizontalRoad(intersection1, intersection2);
		else
			return hasHorizontalRoad(intersection2, intersection2);
	else
		return false;
}

bool hasVerticalRoad(Intersection intersection1, Intersection intersection2) {
	return intersection1.bottomRoad != NULL && intersection2.topRoad != NULL && intersection1.bottomRoad == intersection2.topRoad;
}

bool hasHorizontalRoad(Intersection intersection1, Intersection intersection2) {
	return intersection1.rightRoad != NULL && intersection2.leftRoad != NULL && intersection1.leftRoad == intersection2.rightRoad;
}

Road createGridRoad(float roadWidth, Intersection* intersection1, Intersection* intersection2) {
	if (intersection1->coordinate.x == intersection2->coordinate.x)
		if (intersection1->coordinate.y > intersection2->coordinate.y)
			return createGridHorizontalRoad(roadWidth, intersection1, intersection2);
		else
			return createGridHorizontalRoad(roadWidth, intersection2, intersection1);
	else if (intersection1->coordinate.y == intersection2->coordinate.y)
		if (intersection1->coordinate.x > intersection2->coordinate.x)
			return createGridVerticalRoad(roadWidth, intersection1, intersection2);
		else
			return createGridVerticalRoad(roadWidth, intersection2, intersection1);
	else
		throw new exception("Bad intersection coords!");
}

Road createGridHorizontalRoad(float roadWidth, Intersection* topIntersection, Intersection* bottomIntersection) {
	Point topPoint = { topIntersection->coordinate.x, topIntersection->coordinate.y - roadWidth / 2 };
	Point bottomPoint = { bottomIntersection->coordinate.x, bottomIntersection->coordinate.y + roadWidth / 2 };
	Road newRoad = { topPoint, bottomPoint, roadWidth };
	topIntersection->bottomRoad = &newRoad;
	bottomIntersection->topRoad = &newRoad;
	return newRoad;
}

Road createGridVerticalRoad(float roadWidth, Intersection* leftIntersection, Intersection* rightIntersection) {
	Point leftPoint = { leftIntersection->coordinate.x - roadWidth / 2, leftIntersection->coordinate.y };
	Point rightPoint = { rightIntersection->coordinate.x + roadWidth / 2, leftIntersection->coordinate.y };
	Road newRoad = { leftPoint, rightPoint, roadWidth };
	leftIntersection->rightRoad = &newRoad;
	rightIntersection->leftRoad = &newRoad;
	return newRoad;
}

int getIntersectionIndex(GridPosition position, int colCount) {
	return getIntersectionIndex(position.row, position.col, colCount);
}

int getIntersectionIndex(int row, int col, int colCount) {
	return row * colCount + col;
}

GridPosition createNewLineDirection() {
	return lineDirections[rand() % 4];
}

