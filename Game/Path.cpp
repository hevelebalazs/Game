#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "Bezier.h"
#include "Geometry.h"
#include "Path.h"

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
	if (node.elem.type > 4) throw 1;

	// TODO: create a resizable template array
	//       or should this be preallocated?
	path->nodes = (PathNode*)realloc(path->nodes, (path->nodeCount + 1) * sizeof(PathNode));
	path->nodes[path->nodeCount] = node;
	path->nodeCount++;
}

static PathNode ElemNode(MapElem elem) {
	PathNode node = {};
	node.elem = elem;
	return node;
}

static void PushElem(Path* path, MapElem elem) {
	PathNode node = ElemNode(elem);

	PushNode(path, node);
}

static PathNode RoadNode(Road* road) {
	PathNode node = {};
	node.elem.type = MapElemRoad;
	node.elem.road = road;
	return node;
}

static PathNode IntersectionNode(Intersection* intersection) {
	PathNode node = {};
	node.elem.type = MapElemIntersection;
	node.elem.intersection = intersection;
	return node;
}

static PathNode BuildingNode(Building* building) {
	PathNode node = {};
	node.elem.type = MapElemBuilding;
	node.elem.building = building;
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

static void InvertSegment(Path* path, int startIndex, int endIndex) {
	while (startIndex < endIndex) {
		PathNode tmpNode = path->nodes[startIndex];
		path->nodes[startIndex] = path->nodes[endIndex];
		path->nodes[endIndex] = tmpNode;

		++startIndex;
		--endIndex;
	}
}

static void PushFromBuildingToRoadElem(Path* path, Building* building) {
	while (building->connectElem.type == MapElemBuilding) {
		PushBuilding(path, building);
		building = building->connectElem.building;
	}

	PushBuilding(path, building);
}

static void PushFromRoadElemToBuilding(Path* path, Building *building) {
	int startIndex = path->nodeCount;
	PushFromBuildingToRoadElem(path, building);
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

static void PushConnectRoadElems(Path* path, Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper) {
	for (int i = 0; i < map->intersectionCount; ++i) helper->isIntersectionHelper[i] = 0;
	for (int i = 0; i < map->roadCount; ++i) helper->isRoadHelper[i] = 0;

	helper->nodeCount = 0;

	if (elemStart.type == MapElemRoad) {
		AddRoadToHelper(map, elemStart.road, -1, helper);
	}
	else if (elemStart.type == MapElemIntersection) {
		AddIntersectionToHelper(map, elemStart.intersection, -1, helper);
	}

	if (elemStart.address == elemEnd.address) {
		PushElem(path, elemStart);
		return;
	}

	Road* roadEnd = 0;
	if (elemEnd.type == MapElemRoad) roadEnd = elemEnd.road;

	Intersection* intersectionEnd = 0;
	if (elemEnd.type == MapElemIntersection) intersectionEnd = elemEnd.intersection;

	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemRoad) {
			Road* road = elem.road;

			AddIntersectionToHelper(map, road->intersection1, i, helper);
			if (intersectionEnd && road->intersection1 == intersectionEnd) break;

			AddIntersectionToHelper(map, road->intersection2, i, helper);
			if (intersectionEnd && road->intersection2 == intersectionEnd) break;
		}
		else if (elem.type == MapElemIntersection) {
			Intersection* intersection = elem.intersection;

			if (intersection->leftRoad) {
				AddRoadToHelper(map, intersection->leftRoad, i, helper);
				if (roadEnd && intersection->leftRoad == roadEnd) break;
			}

			if (intersection->rightRoad) {
				AddRoadToHelper(map, intersection->rightRoad, i, helper);
				if (roadEnd && intersection->rightRoad == roadEnd) break;
			}

			if (intersection->topRoad) {
				AddRoadToHelper(map, intersection->topRoad, i, helper);
				if (roadEnd && intersection->topRoad == roadEnd) break;
			}

			if (intersection->bottomRoad) {
				AddRoadToHelper(map, intersection->bottomRoad, i, helper);
				if (roadEnd && intersection->bottomRoad == roadEnd) break;
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
			building1 = building1->connectElem.building;
		}
		if (building2->connectTreeHeight > building1->connectTreeHeight) {
			building2 = building2->connectElem.building;
		}
	}

	while (building1 && building2 && building1->connectTreeHeight > 1 && building2->connectTreeHeight > 1) {
		if (building1 == building2) return building1;

		building1 = building1->connectElem.building;
		building2 = building2->connectElem.building;
	}

	if (building1 == building2) return building1;
	else return 0;

	return 0;
}

static void PushDownTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	while (buildingStart != buildingEnd) {
		PushBuilding(path, buildingStart);

		buildingStart = buildingStart->connectElem.building;
	}

	PushBuilding(path, buildingEnd);
}

static void PushUpTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	int startIndex = path->nodeCount;

	PushDownTheTree(path, buildingEnd, buildingStart);

	int endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);
}

MapElem GetConnectRoadElem(Building* building) {
	while (building->connectElem.type == MapElemBuilding) {
		building = building->connectElem.building;
	}

	return building->connectElem;
}

Path ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper) {
	Path path = EmptyPath();

	if (elemStart.address == elemEnd.address) {
		PushElem(&path, elemStart);

		return path;
	}

	bool finished = false;

	if (elemStart.type == MapElemBuilding && elemEnd.type == MapElemBuilding) {
		Building* buildingStart = elemStart.building;
		Building* buildingEnd = elemEnd.building;

		// NOTE: if the buildingss have a common ancestor in the connection tree,
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

			finished = true;
		}
	}
	
	if (!finished) {
		MapElem roadElemStart = {};
		if (elemStart.type == MapElemBuilding) {
			PushFromBuildingToRoadElem(&path, elemStart.building);

			roadElemStart = GetConnectRoadElem(elemStart.building);
		}
		else {
			roadElemStart = elemStart;
		}

		MapElem roadElemEnd = {};
		if (elemEnd.type == MapElemBuilding) {
			roadElemEnd = GetConnectRoadElem(elemEnd.building);
		}
		else {
			roadElemEnd = elemEnd;
		}

		PushConnectRoadElems(&path, map, roadElemStart, roadElemEnd, helper);

		if (elemEnd.type == MapElemBuilding) {
			PushFromRoadElemToBuilding(&path, elemEnd.building);
		}
	}

	if (path.nodeCount == 0) return path;
	
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

	DrawRect(
		renderer,
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

DirectedPoint StartNodePoint(PathNode* node) {
	Point position = {};
	Point direction = {};

	MapElem elem = node->elem;
	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;

		position = building->connectPointClose;
		direction = PointDirection(building->connectPointClose, building->connectPointFar);
	}
	else if (elem.type == MapElemIntersection) {
		position = elem.intersection->coordinate;
		// TODO: add direction
	}

	DirectedPoint result = {};
	result.position = position;
	result.direction = direction;
	return result;
}

DirectedPoint NextFromBuildingToNothing(DirectedPoint startPoint, Building* building) {
	DirectedPoint result = {};

	if (PointEqual(startPoint.position, building->connectPointFar) || PointEqual(startPoint.position, building->connectPointFarShow)) {
		result.position = building->connectPointClose;
		result.direction = PointDirection(building->connectPointFar, building->connectPointClose);
	}
	else {
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

bool EndFromBuildingToNothing(DirectedPoint point, Building* building) {
	bool result = PointEqual(point.position, building->connectPointClose);
	return result;
}

DirectedPoint NextFromBuildingToBuilding(DirectedPoint startPoint, Building* building, Building* nextBuilding) {
	DirectedPoint result = {};

	// TODO: add direction
	if (building->connectElem.type == MapElemBuilding && building->connectElem.building == nextBuilding) {
		if (PointEqual(startPoint.position, building->connectPointClose)) {
			result.position = building->connectPointFar;
		}
		else {
			result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
		}
	}
	else if (nextBuilding->connectElem.type == MapElemBuilding && nextBuilding->connectElem.building == building) {
		if (PointEqual(startPoint.position, building->connectPointFar) || PointEqual(startPoint.position, building->connectPointFarShow)) {
			result.position = building->connectPointClose;
		}
		else {
			result.position = NextPointAroundBuilding(building, startPoint.position, nextBuilding->connectPointFar);
		}
	}

	return result;
}

bool EndFromBuildingToBuilding(DirectedPoint point, Building* building, Building* nextBuilding) {
	bool result = false;

	if (building->connectElem.type == MapElemBuilding && building->connectElem.building == nextBuilding) {
		result = PointEqual(point.position, building->connectPointFar);
	}
	else if (nextBuilding->connectElem.type == MapElemBuilding && nextBuilding->connectElem.building == building) {
		result = PointEqual(point.position, nextBuilding->connectPointFar);
	}

	return result;
}

DirectedPoint NextFromBuildingToRoad(DirectedPoint startPoint, Building* building, Road* road) {
	DirectedPoint result = {};

	if (PointEqual(startPoint.position, building->connectPointClose)) {
		result.position = building->connectPointFarShow;
		result.direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
	}
	else {
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

bool EndFromBuildingToRoad(DirectedPoint point, Building* building, Road* road) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromBuildingToIntersection(DirectedPoint startPoint, Building* building, Intersection* intersection) {
	DirectedPoint result = {};

	if (PointEqual(startPoint.position, building->connectPointClose)) {
		result.position = building->connectPointFarShow;
		result.direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
	}
	else {
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

bool EndFromBuildingToIntersection(DirectedPoint point, Building* building, Intersection* intersection) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromRoadToBuilding(DirectedPoint startPoint, Road* road, Building* building) {
	DirectedPoint result = {};
	result.position = building->connectPointFarShow;
	result.direction = PointDirection(building->connectPointFarShow, building->connectPointClose);

	return result;
}

bool EndFromRoadToBuilding(DirectedPoint point, Road* road, Building* building) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromRoadToIntersection(DirectedPoint startPoint, Road* road, Intersection* intersection) {
	DirectedPoint result = {};

	if (intersection == road->intersection1) {
		result.position = road->endPoint1;

		// TODO: add an angle to Road structure so this does not have to be recalculated?
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
	}
	else if (intersection == road->intersection2) {
		result.position = road->endPoint2;
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
	}
	else {
		result.position = intersection->coordinate;
	}

	return result;
}

bool EndFromRoadToIntersection(DirectedPoint point, Road* road, Intersection* intersection) {
	bool result = false;

	if (intersection == road->intersection1) result = PointEqual(point.position, road->endPoint1);
	else if (intersection == road->intersection2) result = PointEqual(point.position, road->endPoint2);
	else result = PointEqual(point.position, intersection->coordinate);

	return result;
}

DirectedPoint NextFromIntersectionToBuilding(DirectedPoint startPoint, Intersection* intersection, Building* building) {
	DirectedPoint result = {};

	result.position = building->connectPointFarShow;
	result.direction = PointDirection(building->connectPointFarShow, building->connectPointClose);

	return result;
}

bool EndFromIntersectionToBuilding(DirectedPoint point, Intersection* intersection, Building* building) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromIntersectionToRoad(DirectedPoint startPoint, Intersection* intersection, Road* road) {
	DirectedPoint result = {};

	if (road->intersection1 == intersection) {
		result.position = road->endPoint1;
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
	}
	else if (road->intersection2 == intersection) {
		result.position = road->endPoint2;
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
	}
	else {
		result.position = intersection->coordinate;
	}

	return result;
}

bool EndFromIntersectionToRoad(DirectedPoint point, Intersection* intersection, Road* road) {
	bool result = false;

	if (road->intersection1 == intersection) result = PointEqual(point.position, road->endPoint1);
	else if (road->intersection2 == intersection) result = PointEqual(point.position, road->endPoint2);
	else result = PointEqual(point.position, intersection->coordinate);

	return result;
}

// TODO: make this work with indexes instead of floating-point comparison
DirectedPoint NextNodePoint(PathNode* node, DirectedPoint startPoint) {
	DirectedPoint result = {};

	MapElem elem = node->elem;
	PathNode* next = node->next;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;

		if (!next)                                       result = NextFromBuildingToNothing(startPoint, building);
		else if (next->elem.type == MapElemBuilding)     result = NextFromBuildingToBuilding(startPoint, building, next->elem.building);
		else if (next->elem.type == MapElemRoad)         result = NextFromBuildingToRoad(startPoint, building, next->elem.road);
		else if (next->elem.type == MapElemIntersection) result = NextFromBuildingToIntersection(startPoint, building, next->elem.intersection);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) ; // NOTE: path should not end in a road
		else if (next->elem.type == MapElemBuilding)     result = NextFromRoadToBuilding(startPoint, road, next->elem.building);
		else if (next->elem.type == MapElemIntersection) result = NextFromRoadToIntersection(startPoint, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersection) {
		Intersection* intersection = elem.intersection;

		if (next && next->elem.type == MapElemBuilding)  result = NextFromIntersectionToBuilding(startPoint, intersection, next->elem.building);
		else if (next && next->elem.type == MapElemRoad) result = NextFromIntersectionToRoad(startPoint, intersection, next->elem.road);
		else result.position = elem.intersection->coordinate;
	}

	return result;
}

bool IsNodeEndPoint(PathNode* node, DirectedPoint point) {
	bool result = false;

	MapElem elem = node->elem;
	PathNode* next = node->next;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;

		if (!next)                                       result = EndFromBuildingToNothing(point, building);
		else if (next->elem.type == MapElemBuilding)     result = EndFromBuildingToBuilding(point, building, next->elem.building);
		else if (next->elem.type == MapElemRoad)         result = EndFromBuildingToRoad(point, building, next->elem.road);
		else if (next->elem.type == MapElemIntersection) result = EndFromBuildingToIntersection(point, building, next->elem.intersection);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) ; // NOTE: path should not end with a road
		else if (next->elem.type == MapElemBuilding)     result = EndFromRoadToBuilding(point, road, next->elem.building);
		else if (next->elem.type == MapElemIntersection) result = EndFromRoadToIntersection(point, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersection) {
		Intersection* intersection = elem.intersection;

		if (next && next->elem.type == MapElemBuilding)  result = EndFromIntersectionToBuilding(point, intersection, next->elem.building);
		else if (next && next->elem.type == MapElemRoad) result = EndFromIntersectionToRoad(point, intersection, next->elem.road);
	}

	return result;
}

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth) {
	if (path->nodeCount > 0) {
		PathNode* node = &path->nodes[0];
		DirectedPoint point = StartNodePoint(node);

		while (node) {
			if (IsNodeEndPoint(node, point)) {
				node = node->next;
			}
			else {
				DirectedPoint nextPoint = NextNodePoint(node, point);

				DrawGridLine(renderer, point.position, nextPoint.position, color, lineWidth);

				point = nextPoint;
			}
		}
	}
}

void DrawBezierPath(Path* path, Renderer renderer, Color color, float lineWidth) {
	Bezier4 bezier4 = {};
	if (path->nodeCount > 0) {
		PathNode* node = &path->nodes[0];
		DirectedPoint point = StartNodePoint(node);

		while (node) {
			if (IsNodeEndPoint(node, point)) {
				node = node->next;
			}
			else {
				DirectedPoint nextPoint = NextNodePoint(node, point);
				bezier4 = TurnBezier4(point, nextPoint);
				DrawBezier4(bezier4, renderer, color, lineWidth, 5);

				point = nextPoint;
			}
		}
	}
}