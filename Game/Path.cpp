#include "Path.h"

#include <stdlib.h>

#include <stdio.h>
#include <windows.h>

// TODO: should the helper be attached to the map?
PathHelper PathHelperForMap(Map* map) {
	PathHelper helper;
	helper.nodes = new PathNode[map->intersectionCount + map->roadCount];
	helper.isIntersectionHelper = new int[map->intersectionCount];
	helper.isRoadHelper = new int[map->roadCount];
	helper.sourceIndex = new int[map->intersectionCount + map->roadCount];

	return helper;
}

static Path EmptyPath() {
	Path path = {};
	return path;
}

static void PushNode(Path* path, PathNode node) {
	// TODO: create a resizable template array
	//       or should this be preallocated?
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

static void PushFromRoadToRoad(Path* path, Map* map, Road* roadStart, Road* roadEnd, PathHelper* helper) {
	int maxNodeCount = (map->roadCount + map->intersectionCount);

	for (int i = 0; i < map->intersectionCount; ++i) helper->isIntersectionHelper[i] = 0;
	for (int i = 0; i < map->roadCount; ++i) helper->isRoadHelper[i] = 0;

	helper->nodeCount = 0;
	helper->sourceIndex[helper->nodeCount] = -1;
	helper->nodes[helper->nodeCount] = RoadNode(roadStart);
	helper->nodeCount++;

	if (roadStart == roadEnd) {
		PushRoad(path, roadStart);
		return;
	}

	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];

		if (node.type == PATH_NODE_ROAD) {
			Road* road = node.road;

			AddIntersectionToHelper(map, road->intersection1, i, helper);
			AddIntersectionToHelper(map, road->intersection2, i, helper);
		}
		else if (node.type == PATH_NODE_INTERSECTION) {
			Intersection* intersection = node.intersection;

			if (intersection->leftRoad) {
				AddRoadToHelper(map, intersection->leftRoad, i, helper);

				if (intersection->leftRoad == roadEnd) break;
			}

			if (intersection->rightRoad) {
				AddRoadToHelper(map, intersection->rightRoad, i, helper);

				if (intersection->rightRoad == roadEnd) break;
			}

			if (intersection->topRoad) {
				AddRoadToHelper(map, intersection->topRoad, i, helper);

				if (intersection->topRoad == roadEnd) break;
			}

			if (intersection->bottomRoad) {
				AddRoadToHelper(map, intersection->bottomRoad, i, helper);

				if (intersection->bottomRoad == roadEnd) break;
			}
		}
	}

	int startIndex = path->nodeCount;

	int nodeIndex = helper->nodeCount - 1;
	while (nodeIndex > -1) {
		PathNode node = helper->nodes[nodeIndex];

		PushNode(path, node);

		nodeIndex = helper->sourceIndex[nodeIndex];
	}

	int endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);
}

Building* CommonAncestor(Building* building1, Building* building2) {
	while (building1->connectTreeHeight != building2->connectTreeHeight) {
		if (building1->connectTreeHeight > building2->connectTreeHeight) {
			building1 = building1->connectBuilding;
		}
		if (building2->connectTreeHeight > building1->connectTreeHeight) {
			building2 = building2->connectBuilding;
		}
	}

	while (building1 || building2) {
		if (building1 == building2) return building1;

		building1 = building1->connectBuilding;
		building2 = building2->connectBuilding;
	}

	return 0;
}

static void PushDownTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	while (buildingStart != buildingEnd) {
		PushBuilding(path, buildingStart);

		buildingStart = buildingStart->connectBuilding;
	}

	PushBuilding(path, buildingEnd);
}

static void PushUpTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	int startIndex = path->nodeCount;

	PushDownTheTree(path, buildingEnd, buildingStart);

	int endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);
}

Path ConnectBuildings(Map* map, Building* buildingStart, Building* buildingEnd, PathHelper* helper) {
	Path path = EmptyPath();

	// NOTE: if the buildings have a common ancestor in the connection tree,
	//       there is no need to go out to the road
	Building* commonAncestor = CommonAncestor(buildingStart, buildingEnd);

	if (commonAncestor) {
		if (commonAncestor == buildingEnd) {
			PushDownTheTree(&path, buildingStart, buildingEnd);
		}
		else if (commonAncestor == buildingStart) {
			PushUpTheTree(&path, buildingStart, buildingEnd);
		}
		else {
			PushDownTheTree(&path, buildingStart, commonAncestor);
			// NOTE: making sure commonAncestor does not get into the path twice
			path.nodeCount--;
			PushUpTheTree(&path, commonAncestor, buildingEnd);
		}
	}
	else {
		PushFromBuildingToRoad(&path, buildingStart);

		Road* roadStart = buildingStart->GetConnectedRoad();
		Road* roadEnd = buildingEnd->GetConnectedRoad();

		PushFromRoadToRoad(&path, map, roadStart, roadEnd, helper);

		PushFromRoadToBuilding(&path, buildingEnd);
	}

	for (int i = 0; i < path.nodeCount - 1; ++i) {
		path.nodes[i].next = &path.nodes[i + 1];
	}

	path.nodes[path.nodeCount - 1].next = 0;

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

		if (point1.y < point2.y) {
			point1.y -= width * 0.5f;
			point2.y += width * 0.5f;
		}
		else {
			point1.y += width * 0.5f;
			point2.y -= width * 0.5f;
		}
	}
	
	if (point1.y == point2.y) {
		point1.y -= width * 0.5f;
		point2.y += width * 0.5f;

		if (point1.x < point2.x) {
			point1.x -= width * 0.5f;
			point2.x += width * 0.5f;
		}
		else {
			point1.x += width * 0.5f;
			point2.x -= width * 0.5f;
		}
	}

	renderer.DrawRect(
		point1.y, point1.x,
		point2.y, point2.x,
		color
	);
}

Point NextPointAroundBuilding(Building* building, Point startPoint, Point targetPoint) {
	if (startPoint.x == building->left && startPoint.y < building->bottom) {
		if (targetPoint.x == building->left && targetPoint.y > startPoint.y) return targetPoint;
		else return Point{building->left, building->bottom};
	}
	else if (startPoint.y == building->bottom && startPoint.x < building->right) {
		if (targetPoint.y == building->bottom && targetPoint.x > startPoint.x) return targetPoint;
		else return Point{building->right, building->bottom};
	}
	else if (startPoint.x == building->right && startPoint.y > building->top) {
		if (targetPoint.x == building->right && targetPoint.y < startPoint.y) return targetPoint;
		else return Point{building->right, building->top};
	}
	else if (startPoint.y == building->top && startPoint.x > building->left) {
		if (targetPoint.y == building->top && targetPoint.x < startPoint.x) return targetPoint;
		else return Point{building->left, building->top};
	}
	else {
		return targetPoint;
	}
}

Point PathNode::StartPoint() {
	Point point = {};

	if (type == PATH_NODE_BUILDING) {
		point = building->connectPointClose;
	}

	return point;
}

Point PathNode::NextPoint(Point startPoint) {
	Point nextPoint = {};

	if (type == PATH_NODE_BUILDING) {
		if (!next) {
			nextPoint = building->connectPointClose;
		}
		else if (next->type == PATH_NODE_BUILDING) {
			Building* nextBuilding = next->building;

			if (building->connectBuilding == nextBuilding) {
				if (startPoint == building->connectPointClose) {
					nextPoint = building->connectPointFar;
				}
				else {
					nextPoint = NextPointAroundBuilding(building, startPoint, building->connectPointClose);
				}
			}
			else if (nextBuilding->connectBuilding == building) {
				if (startPoint == building->connectPointFar) {
					nextPoint = building->connectPointClose;
				}
				else {
					nextPoint = NextPointAroundBuilding(building, startPoint, nextBuilding->connectPointFar);
				}
			}
		}
		else if (next->type == PATH_NODE_ROAD) {
			if (startPoint == building->connectPointClose) {
				nextPoint = building->connectPointFar;
			}
			else {
				nextPoint = NextPointAroundBuilding(building, startPoint, building->connectPointClose);
			}
		}
		else if (next->type == PATH_NODE_INTERSECTION) {
			// TODO: implement this if buildings are connected to intersections
		}
	}
	else if (type == PATH_NODE_ROAD) {
		if (!next) {
			// NOTE: a path should not end in a road
		}
		else if (next->type == PATH_NODE_BUILDING) {
			nextPoint = next->building->connectPointFar;
		}
		else if (next->type == PATH_NODE_INTERSECTION) {
			nextPoint = next->intersection->coordinate;
		}
	}
	else if (type == PATH_NODE_INTERSECTION) {
		nextPoint = intersection->coordinate;

		if (next && next->type == PATH_NODE_BUILDING) {
			// TODO: implement this if buildings are connected to intersections
		}
	}

	return nextPoint;
}

bool PathNode::IsEndPoint(Point point) {
	bool result = false;

	if (type == PATH_NODE_BUILDING) {
		if (!next) {
			result = (point == building->connectPointClose);
		}
		else if (next->type == PATH_NODE_BUILDING) {
			Building* nextBuilding = next->building;

			if (building->connectBuilding == nextBuilding) result = (point == building->connectPointFar);
			else if (nextBuilding->connectBuilding == building) result = (point == nextBuilding->connectPointFar);
		}
		else if (next->type == PATH_NODE_ROAD) {
			result = (point == building->connectPointFar);
		}
	}
	else if (type == PATH_NODE_ROAD) {
		if (!next) {
			// NOTE: path should not end with a road
		}
		else if (next->type == PATH_NODE_BUILDING) {
			result = (point == next->building->connectPointFar);
		}
		else if (next->type == PATH_NODE_INTERSECTION) {
			result = (point == next->intersection->coordinate);
		}
	}
	else if (type == PATH_NODE_INTERSECTION) {
		result = (point == intersection->coordinate);

		if (next && next->type == PATH_NODE_BUILDING) {
			// TODO: implement this if buildings are connected to intersections
		}
	}

	return result;
}

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth) {
	if (path->nodeCount > 0) {
		PathNode* node = &path->nodes[0];
		Point point = node->StartPoint();

		while (node) {
			if (node->IsEndPoint(point)) {
				node = node->next;
			}
			else {
				Point nextPoint = node->NextPoint(point);

				DrawGridLine(renderer, point, nextPoint, color, lineWidth);

				point = nextPoint;
			}
		}
	}
}