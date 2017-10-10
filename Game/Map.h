#include "Intersection.h"

struct Map
{
	Intersection* intersections;
	int intersectionCount = 0;
	Road* roads;
	int roadCount = 0;
	float width = 0;
	float height = 0;

	void draw(HDC context);
};

