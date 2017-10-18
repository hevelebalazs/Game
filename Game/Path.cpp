#include "Path.h"

void addIntersectionPathHelper(Map map, Intersection *intersection, int sourceIndex, IntersectionPathHelper *pathHelper) {
	int intersectionIndex = (int)(intersection - map.intersections);

	if (!pathHelper->isHelper[intersectionIndex]) {
		pathHelper->isHelper[intersectionIndex] = 1;
		pathHelper->indexes[pathHelper->count++] = intersectionIndex;
		pathHelper->source[intersectionIndex] = sourceIndex;
	}
}

static void buildUpPathHelper(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper) {
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

		if (intersection1->topRoad) {
			Intersection *intersection2 = intersection1->topRoad->otherIntersection(intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);

			if (intersection2 == finish) break;
		}

		if (intersection1->bottomRoad) {
			Intersection *intersection2 = intersection1->bottomRoad->otherIntersection(intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);

			if (intersection2 == finish) break;
		}

		if (intersection1->leftRoad) {
			Intersection *intersection2 = intersection1->leftRoad->otherIntersection(intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);

			if (intersection2 == finish) break;

		}

		if (intersection1->rightRoad) {
			Intersection *intersection2 = intersection1->rightRoad->otherIntersection(intersection1);
			addIntersectionPathHelper(map, intersection2, intersection1Index, pathHelper);

			if (intersection2 == finish) break;
		}
	}
}

Road *nextRoadOnPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper) {
	Intersection *nextIntersection = nextIntersectionOnPath(map, start, finish, pathHelper);

	Road *road = 0;

	if (start->leftRoad && start->leftRoad->otherIntersection(start) == nextIntersection) {
		road = start->leftRoad;
	}
	else if (start->rightRoad && start->rightRoad->otherIntersection(start) == nextIntersection) {
		road = start->rightRoad;
	}
	else if (start->topRoad && start->topRoad->otherIntersection(start) == nextIntersection) {
		road = start->topRoad;
	}
	else if (start->bottomRoad && start->bottomRoad->otherIntersection(start) == nextIntersection) {
		road = start->bottomRoad;
	}

	return road;
}

Intersection *nextIntersectionOnPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper) {
	buildUpPathHelper(map, start, finish, pathHelper);

	int startIndex = (int)(start - map.intersections);
	int intersectionIndex = pathHelper->indexes[pathHelper->count - 1];

	while (pathHelper->source[intersectionIndex] != startIndex) {
		intersectionIndex = pathHelper->source[intersectionIndex];
	}

	return &map.intersections[intersectionIndex];
}

IntersectionPath findConnectingPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper) {
	buildUpPathHelper(map, start, finish, pathHelper);

	IntersectionPath result = {};

	int intersectionIndex = pathHelper->indexes[pathHelper->count - 1];
	Intersection *intersection = &map.intersections[intersectionIndex];

	if (intersection == finish) {
		int count = 0;
		while (intersectionIndex > -1) {
			pathHelper->indexes[count++] = intersectionIndex;
			intersectionIndex = pathHelper->source[intersectionIndex];
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

void drawIntersectionPath(IntersectionPath path, Bitmap bitmap, float pathWidth) {
	for (int i = 1; i < path.intersectionCount; ++i) {
		Intersection *prevIntersection = path.intersections[i - 1];
		Intersection *thisIntersection = path.intersections[i];

		Point prevCenter = prevIntersection->coordinate;
		Point thisCenter = thisIntersection->coordinate;

		Color color = { 1.0f, 0.5f, 0.0f };

		if (prevCenter.x == thisCenter.x) {
			bitmap.drawRect(
				prevCenter.y, prevCenter.x - (pathWidth / 2.0f),
				thisCenter.y, thisCenter.x + (pathWidth / 2.0f),
				color
			);
		}
		else {
			bitmap.drawRect(
				prevCenter.y - (pathWidth / 2.0f), prevCenter.x,
				thisCenter.y + (pathWidth / 2.0f), thisCenter.x,
				color
			);
		}
	}
}