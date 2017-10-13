#include "Map.h"
#include <stdio.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include "GridPosition.h"
using namespace std;

Map generateGrid(float width, float height, float intersectionDistance);
vector<Intersection> generateIntersections(float intersectionDistance, int rowCount, int colCount);
vector<Road> generateRoads(float intersectionDistance, int rowCount, int colCount, vector<Intersection>* intersections, int maxRoadCount);
GridPosition getRandomGridPosition(vector<GridPosition> gridPositions);
vector<Road> generateRoadLine(float roadWidth, int rowCount, int colCount, vector<Intersection>* intersections, vector<GridPosition> connectedGridPositions, GridPosition startGridPosition, GridPosition lineDirection);
bool isLineStoped(GridPosition currentGridPosition, vector<GridPosition> connectedGridPositions);
bool isValidGridPosition(GridPosition gridPosition, int rowCount, int colCount);
bool hasRoad(GridPosition gridPosition1, GridPosition gridPosition2, vector<Intersection> intersections, int colCount);
bool hasRoad(Intersection intersection1, Intersection intersection2);
bool hasVerticalRoad(Intersection intersection1, Intersection intersection2);
bool hasHorizontalRoad(Intersection intersection1, Intersection intersection2);
Road createGridRoad(float roadWidth, Intersection* intersection1, Intersection* intersection2);
Road createGridHorizontalRoad(float roadWidth, Intersection* leftIntersection, Intersection* rightIntersection);
Road createGridVerticalRoad(float roadWidth, Intersection* topIntersection, Intersection* bottomIntersection);
int getIntersectionIndex(GridPosition position, int colCount);
int getIntersectionIndex(int row, int col, int colCount);
GridPosition createNewLineDirection();