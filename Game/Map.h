#include "Intersection.h"
struct Map
{
	Intersection* intersections;
	int intersectionCount = 0;
	Road* roads;
	int roadCount = 0;
	float width = 0;
	float height = 0;

	int TMProadCount = 0;

	void generateGrid(float intersectionDistance);
	void createGridHorizontalRoad(int row, int col, int colCount, int roadWidth);
	void createGridVerticalRoad(int row, int col, int colCount, int roadWidth);
	void initIntersections(int rowCount, int colCount, float intersectionDistance);
	void initRoads(int rowCount, int colCount, float intersectionDistance);
	void draw(HDC context);
};

