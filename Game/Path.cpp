#include "Path.h"

// TODO: make this library work with pre-allocated memory
#include <stdlib.h>

#include <stdio.h>
#include <windows.h>

static Path EmptyPath() {
	Path path = {};
	return path;
}

static void PushNode(Path* path, PathNode node) {
	// TODO: create a resizable template array
	path->nodes = (PathNode*)realloc(path->nodes, (path->nodeCount + 1) * sizeof(PathNode));
	path->nodes[path->nodeCount] = node;
	path->nodeCount++;
}

static PathNode RoadNode(Road* road) {
	PathNode node = {};
	node.type = PATH_NODE_ROAD;
	node.road = road;
	return node;
}

static PathNode IntersectionNode(Intersection* intersection) {
	PathNode node = {};
	node.type = PATH_NODE_INTERSECTION;
	node.intersection = intersection;
	return node;
}

static PathNode BuildingNode(Building* building) {
	PathNode node = {};
	node.type = PATH_NODE_BUILDING;
	node.building = building;
	return node;
}

static void PushRoad(Path* path, Road* road) {
	PathNode node = RoadNode(road);

	PushNode(path, node);
}

static void PushIntersection(Path* path, Intersection* intersection) {
	PathNode node = IntersectionNode(intersection);

	PushNode(path, node);
}

static void PushBuilding(Path* path, Building* building) {
	PathNode node = BuildingNode(building);

	PushNode(path, node);
}

static void PushFromBuildingToRoad(Path* path, Building* building) {
	while (building->connectBuilding) {
		PushBuilding(path, building);
		building = building->connectBuilding;
	}

	PushBuilding(path, building);
}

static void InvertSegment(Path* path, int startIndex, int endIndex) {
	while (startIndex < endIndex) {
		PathNode tmpNode = path->nodes[startIndex];
		path->nodes[startIndex] = path->nodes[endIndex];
		path->nodes[endIndex] = tmpNode;

		++startIndex;
		--endIndex;
	}
}

static void PushFromRoadToBuilding(Path* path, Building *building) {
	int startIndex = path->nodeCount;
	PushFromBuildingToRoad(path, building);
	int endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);
}

struct PathHelper {
	PathNode* nodes;
	int nodeCount;

	int* isIntersectionHelper;
	int* isRoadHelper;
	int* sourceIndex;
};

static void AddRoadToHelper(Map* map, Road* road, int sourceIndex, PathHelper* pathHelper) {
	int roadIndex = (int)(road - map->roads);

	if (pathHelper->isRoadHelper[roadIndex] == 0) {
		pathHelper->isRoadHelper[roadIndex] = 1;

		pathHelper->nodes[pathHelper->nodeCount] = RoadNode(road);
		pathHelper->sourceIndex[pathHelper->nodeCount] = sourceIndex;
		pathHelper->nodeCount++;
	}
}

static void AddIntersectionToHelper(Map* map, Intersection* intersection, int sourceIndex, PathHelper* pathHelper) {
	int intersectionIndex = (int)(intersection - map->intersections);

	if (pathHelper->isIntersectionHelper[intersectionIndex] == 0) {
		pathHelper->isIntersectionHelper[intersectionIndex] = 1;

		pathHelper->nodes[pathHelper->nodeCount] = IntersectionNode(intersection);
		pathHelper->sourceIndex[pathHelper->nodeCount] = sourceIndex;
		pathHelper->nodeCount++;
	}
}

static void PushFromRoadToRoad(Path* path, Map* map, Road* roadStart, Road* roadEnd) {
	int maxNodeCount = (map->roadCount + map->intersectionCount);

	// TODO: make sure the memory for these arrays is always pre-allocated
	PathHelper helper;
	helper.nodes = new PathNode[maxNodeCount];
	helper.isIntersectionHelper = new int[map->intersectionCount];
	helper.isRoadHelper = new int[map->roadCount];
	helper.sourceIndex = new int[maxNodeCount];

	for (int i = 0; i < map->intersectionCount; ++i) helper.isIntersectionHelper[i] = 0;
	for (int i = 0; i < map->roadCount; ++i) helper.isRoadHelper[i] = 0;

	helper.nodeCount = 0;
	helper.sourceIndex[helper.nodeCount] = -1;
	helper.nodes[helper.nodeCount] = RoadNode(roadStart);
	helper.nodeCount++;

	for (int i = 0; i < helper.nodeCount; ++i) {
		PathNode node = helper.nodes[i];

		if (node.type == PATH_NODE_ROAD) {
			Road* road = node.road;

			AddIntersectionToHelper(map, road->intersection1, i, &helper);
			AddIntersectionToHelper(map, road->intersection2, i, &helper);
		}
		else if (node.type == PATH_NODE_INTERSECTION) {
			Intersection* intersection = node.intersection;

			if (intersection->leftRoad) {
				AddRoadToHelper(map, intersection->leftRoad, i, &helper);

				if (intersection->leftRoad == roadEnd) break;
			}

			if (intersection->rightRoad) {
				AddRoadToHelper(map, intersection->rightRoad, i, &helper);

				if (intersection->rightRoad == roadEnd) break;
			}

			if (intersection->topRoad) {
				AddRoadToHelper(map, intersection->topRoad, i, &helper);

				if (intersection->topRoad == roadEnd) break;
			}

			if (intersection->bottomRoad) {
				AddRoadToHelper(map, intersection->bottomRoad, i, &helper);

				if (intersection->bottomRoad == roadEnd) break;
			}
		}
	}

	int startIndex = path->nodeCount;

	int nodeIndex = helper.nodeCount - 1;
	while (nodeIndex > -1) {
		PathNode node = helper.nodes[nodeIndex];

		PushNode(path, node);

		nodeIndex = helper.sourceIndex[nodeIndex];
	}

	int endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);

	delete[] helper.nodes;
	delete[] helper.isIntersectionHelper;
	delete[] helper.isRoadHelper;
}

Path ConnectBuildings(Map* map, Building* buildingStart, Building* buildingEnd) {
	Path path = EmptyPath();

	PushFromBuildingToRoad(&path, buildingStart);

	Road* roadStart = buildingStart->GetConnectedRoad();
	Road* roadEnd = buildingEnd->GetConnectedRoad();

	PushFromRoadToRoad(&path, map, roadStart, roadEnd);

	PushFromRoadToBuilding(&path, buildingEnd);

	return path;
}

void ClearPath(Path* path) {
	free(path->nodes);
	path->nodes = 0;
	path->nodeCount = 0;
}

static void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float width) {
	if (point1.x == point2.x) {
		point1.x -= width * 0.5f;
		point2.x += width * 0.5f;
	}
	
	if (point1.y == point2.y) {
		point1.y -= width * 0.5f;
		point2.y += width * 0.5f;
	}

	renderer.DrawRect(
		point1.y, point1.x,
		point2.y, point2.x,
		color
	);
}

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth) {
	Point prevPoint = {};

	int intersectionCount = 0;

	for (int i = 0; i < path->nodeCount; ++i) {
		PathNode *thisNode = &path->nodes[i];
		PathNode *nextNode = 0;

		Point thisPoint = prevPoint;

		if (i < path->nodeCount - 1) nextNode = &path->nodes[i + 1];

		if (thisNode->type == PATH_NODE_BUILDING) {
			Building* building = thisNode->building;
			DrawGridLine(renderer, building->connectPointClose, building->connectPointFar, color, lineWidth);

			thisPoint = building->connectPointFar;
		}
		else if (thisNode->type == PATH_NODE_INTERSECTION) {
			Intersection* intersection = thisNode->intersection;

			thisPoint = intersection->coordinate;
		}
		else if (thisNode->type == PATH_NODE_ROAD) {

		}

		if (i > 0) {
			DrawGridLine(renderer, prevPoint, thisPoint, color, lineWidth);
		}

		prevPoint = thisPoint;
	}

	char log[256];
	sprintf_s(log, "Number of intersections: %i\n", intersectionCount);
	OutputDebugStringA(log);
}