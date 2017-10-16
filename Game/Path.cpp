#include "Path.h"

Intersection *otherIntersection(Road *road, Intersection *intersection) {
	if (road->intersection1 == intersection) return road->intersection2;
	else return road->intersection1;
}

void addIntersectionPathHelper(Map map, Intersection *intersection, int sourceIndex, IntersectionPathHelper *pathHelper) {
	int intersectionIndex = (int)(intersection - map.intersections);

	if (!pathHelper->isHelper[intersectionIndex]) {
		pathHelper->isHelper[intersectionIndex] = 1;
		pathHelper->indexes[pathHelper->count++] = intersectionIndex;
		pathHelper->source[intersectionIndex] = sourceIndex;
	}
}

IntersectionPath findConnectingPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper) {
	for (int i = 0; i < map.intersectionCount; ++i) pathHelper->isHelper[i] = 0;

	int startIndex = (int)(start - map.intersections);
	pathHelper->count = 0;
	pathHelper->indexes[pathHelper->count++] = startIndex;
	pathHelper->isHelper[startIndex] = 1;
	pathHelper->source[startIndex] = -1;

	int intersection1Index = 0;
	Intersection *intersection1 = 0;

	for (int i = 0; i < pathHelper->count; ++i) {
		intersection1Index = pathHelper->indexes[i];
		intersection1 = &map.intersections[intersection1Index];

		if (intersection1 == finish) break;

		if (intersection1->topRoad) {
			Intersection *intersection2 = otherIntersection(intersection1->topRoad, intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);
		}

		if (intersection1->bottomRoad) {
			Intersection *intersection2 = otherIntersection(intersection1->bottomRoad, intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);
		}

		if (intersection1->leftRoad) {
			Intersection *intersection2 = otherIntersection(intersection1->leftRoad, intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);
		}

		if (intersection1->rightRoad) {
			Intersection *intersection2 = otherIntersection(intersection1->rightRoad, intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);
		}
	}

	IntersectionPath result = {};

	if (intersection1 == finish) {
		int count = 0;
		while (intersection1Index > -1) {
			pathHelper->indexes[count++] = intersection1Index;
			intersection1Index = pathHelper->source[intersection1Index];
		}

		result.intersectionCount = count;
		result.intersections = new Intersection*[count];

		for (int i = 0; i < count; ++i) {
			int index = pathHelper->indexes[i];
			result.intersections[i] = &map.intersections[index];
		}
	}

	return result;
}

void drawIntersectionPath(IntersectionPath path, Bitmap bitmap) {
	for (int i = 1; i < path.intersectionCount; ++i) {
		Intersection *prevIntersection = path.intersections[i - 1];
		Intersection *thisIntersection = path.intersections[i];

		Point prevCenter = prevIntersection->coordinate;
		Point thisCenter = thisIntersection->coordinate;

		Color color = { 1.0f, 0.5f, 0.0f };

		float lineWidth = 5.0f;

		if (prevCenter.x == thisCenter.x) {
			bitmap.drawRect(
				(int)(prevCenter.y), (int)(prevCenter.x - lineWidth / 2),
				(int)(thisCenter.y), (int)(thisCenter.x + lineWidth / 2),
				color
			);
		}
		else {
			bitmap.drawRect(
				(int)(prevCenter.y - lineWidth / 2), (int)(prevCenter.x),
				(int)(thisCenter.y + lineWidth / 2), (int)(thisCenter.x),
				color
			);
		}
	}
}