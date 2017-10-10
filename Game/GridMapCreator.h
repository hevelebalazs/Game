#include "Map.h"

struct GridMapCreator {
	float width = 0;
	float height = 0;
	float intersectionDistance;
	int createdRoadCount = 0;

	Map generateGrid();
	Road createGridHorizontalRoad(int roadWidth, Intersection* leftIntersection, Intersection* rightIntersection);
	Road createGridVerticalRoad(int roadWidth, Intersection* topIntersection, Intersection* bottomIntersection);
	Intersection* getIntersections(int rowCount, int colCount);
	Road* getRoads(int rowCount, int colCount, Intersection* intersections);
};