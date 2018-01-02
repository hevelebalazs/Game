#include <stdlib.h>

#include "Bezier.h"
#include "Geometry.h"
#include "Memory.h"
#include "Path.h"

struct Path {
	int nodeCount;
	PathNode *nodes;
};

static inline void PushNode(Path* path, PathNode node) {
	// TODO: use arena for this as well?
	path->nodes = (PathNode*)realloc(path->nodes, (path->nodeCount + 1) * sizeof(PathNode));
	path->nodes[path->nodeCount] = node;
	path->nodeCount++;
}

static inline PathNode ElemNode(MapElem elem) {
	PathNode node = {};
	node.elem = elem;
	return node;
}

static inline void PushElem(Path* path, MapElem elem) {
	PathNode node = ElemNode(elem);

	PushNode(path, node);
}

static inline PathNode RoadNode(Road* road) {
	PathNode node = {};
	node.elem = RoadElem(road);
	node.next = 0;
	return node;
}

static inline PathNode IntersectionNode(Intersection* intersection) {
	PathNode node = {};
	node.elem = IntersectionElem(intersection);
	node.next = 0;
	return node;
}

static inline PathNode BuildingNode(Building* building) {
	PathNode node = {};
	node.elem = BuildingElem(building);
	node.next = 0;
	return node;
}

static inline void PushRoad(Path* path, Road* road) {
	PathNode node = RoadNode(road);

	PushNode(path, node);
}

static inline void PushIntersection(Path* path, Intersection* intersection) {
	PathNode node = IntersectionNode(intersection);

	PushNode(path, node);
}

static inline void PushBuilding(Path* path, Building* building) {
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

struct PathHelper {
	PathNode* nodes;
	int nodeCount;

	int* isIntersectionHelper;
	int* isRoadHelper;
	int* sourceIndex;
};

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
	for (int i = 0; i < map->intersectionCount; ++i) 
		helper->isIntersectionHelper[i] = 0;
	for (int i = 0; i < map->roadCount; ++i)
		helper->isRoadHelper[i] = 0;

	helper->nodeCount = 0;

	if (elemStart.type == MapElemRoad)
		AddRoadToHelper(map, elemStart.road, -1, helper);
	else if (elemStart.type == MapElemIntersection)
		AddIntersectionToHelper(map, elemStart.intersection, -1, helper);

	if (elemStart.address == elemEnd.address) {
		PushElem(path, elemStart);
		return;
	}

	Road* roadEnd = 0;
	if (elemEnd.type == MapElemRoad) 
		roadEnd = elemEnd.road;

	Intersection* intersectionEnd = 0;
	if (elemEnd.type == MapElemIntersection) 
		intersectionEnd = elemEnd.intersection;

	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemRoad) {
			Road* road = elem.road;

			AddIntersectionToHelper(map, road->intersection1, i, helper);
			if (intersectionEnd && road->intersection1 == intersectionEnd)
				break;

			AddIntersectionToHelper(map, road->intersection2, i, helper);
			if (intersectionEnd && road->intersection2 == intersectionEnd)
				break;
		}
		else if (elem.type == MapElemIntersection) {
			Intersection* intersection = elem.intersection;

			if (intersection->leftRoad) {
				AddRoadToHelper(map, intersection->leftRoad, i, helper);
				if (roadEnd && intersection->leftRoad == roadEnd)
					break;
			}

			if (intersection->rightRoad) {
				AddRoadToHelper(map, intersection->rightRoad, i, helper);
				if (roadEnd && intersection->rightRoad == roadEnd)
					break;
			}

			if (intersection->topRoad) {
				AddRoadToHelper(map, intersection->topRoad, i, helper);
				if (roadEnd && intersection->topRoad == roadEnd)
					break;
			}

			if (intersection->bottomRoad) {
				AddRoadToHelper(map, intersection->bottomRoad, i, helper);
				if (roadEnd && intersection->bottomRoad == roadEnd)
					break;
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
		if (building1->connectTreeHeight > building2->connectTreeHeight)
			building1 = building1->connectElem.building;
		if (building2->connectTreeHeight > building1->connectTreeHeight)
			building2 = building2->connectElem.building;
	}

	while (building1 && building2 && building1->connectTreeHeight > 1 && building2->connectTreeHeight > 1) {
		if (building1 == building2)
			return building1;

		building1 = building1->connectElem.building;
		building2 = building2->connectElem.building;
	}

	if (building1 == building2)
		return building1;
	else
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

static PathNode* GetFreePathNode(PathPool* pathPool) {
	PathNode* result = 0;

	if (pathPool->firstFreeNode) {
		result = pathPool->firstFreeNode;
		pathPool->firstFreeNode = pathPool->firstFreeNode->next;
	}
	else if (pathPool->nodeCount < pathPool->maxNodeCount) {
		result = &pathPool->nodes[pathPool->nodeCount];
		pathPool->nodeCount++;
	}
	else {
		// TODO: introduce asserts
	}

	return result;
}

static PathNode* PushPathToArena(Path path, MemArena* arena, PathPool* pathPool) {
	PathNode* result = 0;
	PathNode* previous = 0;

	for (int i = 0; i < path.nodeCount; ++i) {
		PathNode* pathNode = GetFreePathNode(pathPool);

		*pathNode = path.nodes[i];
		pathNode->next = 0;

		if (previous)
			previous->next = pathNode;

		if (!result)
			result = pathNode;

		previous = pathNode;
	}

	return result;
}

PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool) {
	PathNode* result = GetFreePathNode(pathPool);

	result->elem = elem;
	result->next = firstNode;

	return result;
}

static inline PathHelper PathHelperForMap(Map* map, MemArena* arena) {
	PathHelper helper = {};
	helper.nodes = ArenaPushArray(arena, PathNode, map->intersectionCount + map->roadCount);
	helper.isIntersectionHelper = ArenaPushArray(arena, int, map->intersectionCount);
	helper.isRoadHelper = ArenaPushArray(arena, int, map->roadCount);
	helper.sourceIndex = ArenaPushArray(arena, int, map->intersectionCount + map->roadCount);

	return helper;
}

// TODO: rename this to VehiclePath?
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	if (elemStart.type == MapElemRoadSidewalk || elemStart.type == MapElemIntersectionSidewalk) 
		return 0;
	if (elemEnd.type == MapElemRoadSidewalk || elemEnd.type == MapElemIntersectionSidewalk)
		return 0;

	Path path = {};

	PathHelper helper = PathHelperForMap(map, tmpArena);

	bool finished = false;

	if (elemStart.address == elemEnd.address) {
		PushElem(&path, elemStart);

		finished = true;
	}

	Road* endConnectRoad = 0;
	Road* startConnectRoad = 0;

	if (!finished && elemStart.type == MapElemBuilding && elemEnd.type == MapElemBuilding) {
		Building* buildingStart = elemStart.building;
		Building* buildingEnd = elemEnd.building;

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

			finished = true;
		}
	}
	
	if (!finished) {
		for (int i = 0; i < map->roadCount; ++i)
			helper.isRoadHelper[i] = 0;

		for (int i = 0; i < map->intersectionCount; ++i)
			helper.isIntersectionHelper[i] = 0;

		MapElem roadElemStart = {};
		if (elemStart.type == MapElemBuilding) {
			Building* startBuilding = elemStart.building;

			PushFromBuildingToRoadElem(&path, startBuilding);
			roadElemStart = GetConnectRoadElem(startBuilding);

			if (roadElemStart.type == MapElemRoad) 
				startConnectRoad = roadElemStart.road;
		}
		else {
			roadElemStart = elemStart;
		}

		MapElem roadElemEnd = {};
		if (elemEnd.type == MapElemBuilding) {
			Building* endBuilding = elemEnd.building;

			roadElemEnd = GetConnectRoadElem(endBuilding);

			if (roadElemEnd.type == MapElemRoad) 
				endConnectRoad = roadElemEnd.road;
		}
		else {
			roadElemEnd = elemEnd;
		}

		if (startConnectRoad && endConnectRoad && startConnectRoad == endConnectRoad) {
			int startLaneIndex = LaneIndex(*startConnectRoad, elemStart.building->connectPointFarShow);
			int endLaneIndex = LaneIndex(*endConnectRoad, elemEnd.building->connectPointFarShow);

			if (startLaneIndex == endLaneIndex) {
				float startDistance = DistanceOnLane(*startConnectRoad, startLaneIndex, elemStart.building->connectPointFarShow);
				float endDistance = DistanceOnLane(*endConnectRoad, endLaneIndex, elemEnd.building->connectPointFarShow);

				if (startDistance < endDistance) {
					startConnectRoad = 0;
					endConnectRoad = 0;
				}
			}
		}

		if (startConnectRoad) {
			Building* startBuilding = elemStart.building;

			int laneIndex = LaneIndex(*startConnectRoad, startBuilding->connectPointFarShow);

			Intersection* startIntersection = 0;
			if (laneIndex == 1)
				startIntersection = startConnectRoad->intersection2;
			else if (laneIndex == -1)
				startIntersection = startConnectRoad->intersection1;

			if (startIntersection) {
				startConnectRoad = startConnectRoad;
				roadElemStart.type = MapElemIntersection;
				roadElemStart.intersection = startIntersection;
			}
		}

		if (endConnectRoad) {
			Building* endBuilding = elemEnd.building;

			int laneIndex = LaneIndex(*endConnectRoad, endBuilding->connectPointFarShow);
				
			Intersection* endIntersection = 0;
			if (laneIndex == 1)
				endIntersection = endConnectRoad->intersection1;
			else if (laneIndex == -1)
				endIntersection = endConnectRoad->intersection2;

			if (endIntersection) {
				endConnectRoad = endConnectRoad;
				roadElemEnd.type = MapElemIntersection;
				roadElemEnd.intersection = endIntersection;
			}
		}

		if (startConnectRoad)
			PushRoad(&path, startConnectRoad);

		PushConnectRoadElems(&path, map, roadElemStart, roadElemEnd, &helper);

		if (endConnectRoad)
			PushRoad(&path, endConnectRoad);

		if (elemEnd.type == MapElemBuilding)
			PushFromRoadElemToBuilding(&path, elemEnd.building);
	}

	PathNode* firstNode = PushPathToArena(path, arena, pathPool);

	ArenaPopTo(tmpArena, helper.nodes);

	return firstNode;
}

// TODO: rename this to PedestrianPath?
PathNode* ConnectSidewalkElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	// TODO: assert instead of these checks?
	if (elemStart.type != MapElemRoadSidewalk && elemStart.type != MapElemIntersectionSidewalk) 
		return 0;
	if (elemEnd.type != MapElemRoadSidewalk && elemEnd.type != MapElemIntersectionSidewalk)
		return 0;

	Path path = {};

	MapElem roadElemStart = elemStart;
	if (elemStart.type == MapElemRoadSidewalk)
		roadElemStart.type = MapElemRoad;
	else if (elemStart.type == MapElemIntersectionSidewalk)
		roadElemStart.type = MapElemIntersection;

	MapElem roadElemEnd = elemEnd;
	if (elemEnd.type == MapElemRoadSidewalk)
		roadElemEnd.type = MapElemRoad;
	else if (elemEnd.type == MapElemIntersectionSidewalk)
		roadElemEnd.type = MapElemIntersection;

	PathHelper helper = PathHelperForMap(map, tmpArena);
	// TODO: create a ResetPathHelper function?
	for (int i = 0; i < map->roadCount; ++i)
		helper.isRoadHelper[i] = 0;

	for (int i = 0; i < map->intersectionCount; ++i)
		helper.isIntersectionHelper[i] = 0;

	PushConnectRoadElems(&path, map, roadElemStart, roadElemEnd, &helper);

	// NOTE: convert road elems to sidewalk elems
	for (int i = 0; i < path.nodeCount; ++i) {
		MapElem* elem = &path.nodes[i].elem;

		if (elem->type == MapElemRoad)
			elem->type = MapElemRoadSidewalk;
		else if (elem->type == MapElemIntersection)
			elem->type = MapElemIntersectionSidewalk;
	}

	PathNode* firstNode = PushPathToArena(path, arena, pathPool);

	ArenaPopTo(tmpArena, helper.nodes);

	return firstNode;
}

void FreePathNode(PathNode* node, PathPool* pathPool) {
	if (node) {
		node->next = pathPool->firstFreeNode;
		pathPool->firstFreeNode = node;
	}
}

void FreePath(PathNode* firstNode, PathPool* pathPool) {
	if (firstNode) {
		PathNode* lastNode = firstNode;

		while (lastNode && lastNode->next)
			lastNode = lastNode->next;

		lastNode->next = pathPool->firstFreeNode;
		pathPool->firstFreeNode = firstNode;
	}
}

// TODO: move this to Renderer.cpp?
static inline void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float width) {
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
		if (targetPoint.x == building->left && targetPoint.y > startPoint.y)
			return targetPoint;
		else
			return Point{building->left, building->bottom};
	}
	else if (startPoint.y == building->bottom && startPoint.x < building->right) {
		if (targetPoint.y == building->bottom && targetPoint.x > startPoint.x)
			return targetPoint;
		else
			return Point{building->right, building->bottom};
	}
	else if (startPoint.x == building->right && startPoint.y > building->top) {
		if (targetPoint.x == building->right && targetPoint.y < startPoint.y)
			return targetPoint;
		else
			return Point{building->right, building->top};
	}
	else if (startPoint.y == building->top && startPoint.x > building->left) {
		if (targetPoint.y == building->top && targetPoint.x < startPoint.x)
			return targetPoint;
		else
			return Point{building->left, building->top};
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
		position = elem.intersection->position;
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
		if (PointEqual(startPoint.position, building->connectPointClose))
			result.position = building->connectPointFar;
		else
			result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}
	else if (nextBuilding->connectElem.type == MapElemBuilding && nextBuilding->connectElem.building == building) {
		if (PointEqual(startPoint.position, building->connectPointFar) || PointEqual(startPoint.position, building->connectPointFarShow))
			result.position = building->connectPointClose;
		else
			result.position = NextPointAroundBuilding(building, startPoint.position, nextBuilding->connectPointFar);
	}

	return result;
}

bool EndFromBuildingToBuilding(DirectedPoint point, Building* building, Building* nextBuilding) {
	bool result = false;

	if (building->connectElem.type == MapElemBuilding && building->connectElem.building == nextBuilding)
		result = PointEqual(point.position, building->connectPointFar);
	else if (nextBuilding->connectElem.type == MapElemBuilding && nextBuilding->connectElem.building == building)
		result = PointEqual(point.position, nextBuilding->connectPointFar);

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

	bool onRoadSide = IsPointOnRoadSide(startPoint.position, *road);

	if (onRoadSide) {
		int laneIndex = LaneIndex(*road, startPoint.position);
		result = TurnPointToLane(*road, laneIndex, startPoint.position);
	}
	else {
		// TODO: save these values into the building's connection somehow?
		int laneIndex = LaneIndex(*road, building->connectPointFarShow);
		DirectedPoint turnPoint = TurnPointFromLane(*road, laneIndex, building->connectPointFarShow);
		float turnPointDistance = DistanceOnLane(*road, laneIndex, turnPoint.position);
		float startDistance = DistanceOnLane(*road, laneIndex, startPoint.position);

		if (startDistance > turnPointDistance || PointEqual(startPoint.position, turnPoint.position)) {
			result.position = building->connectPointFarShow;
			result.direction = PointDirection(building->connectPointFarShow, building->connectPointClose);
		}
		else {
			result = turnPoint;
		}
	}

	return result;
}

bool EndFromRoadToBuilding(DirectedPoint point, Road* road, Building* building) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromRoadToIntersection(DirectedPoint startPoint, Road* road, Intersection* intersection) {
	DirectedPoint result = {};

	bool onRoadSide = IsPointOnRoadSide(startPoint.position, *road);

	if (onRoadSide) {
		int laneIndex = LaneIndex(*road, startPoint.position);
		result = TurnPointToLane(*road, laneIndex, startPoint.position);
	}
	else if (intersection == road->intersection1) {
		result = RoadLeavePoint(*road, 1);
	}
	else if (intersection == road->intersection2) {
		result = RoadLeavePoint(*road, 2);
	}
	else {
		result.position = intersection->position;
	}

	return result;
}

bool EndFromRoadToIntersection(DirectedPoint point, Road* road, Intersection* intersection) {
	bool result = false;

	Point endPoint = {};
	if (intersection == road->intersection1)
		endPoint = RoadLeavePoint(*road, 1).position;
	else if (intersection == road->intersection2)
		endPoint = RoadLeavePoint(*road, 2).position;
	else
		endPoint = intersection->position;

	result = PointEqual(point.position, endPoint);

	return result;
}

DirectedPoint NextFromIntersectionToNothing(DirectedPoint startPoint, Intersection* intersection) {
	DirectedPoint result = {};

	result.position = intersection->position;

	return result;
}

bool EndFromIntersectionToNothing(DirectedPoint point, Intersection* intersection) {
	bool result = PointEqual(point.position, intersection->position);
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

	if (road->intersection1 == intersection)
		result = RoadEnterPoint(*road, 1);
	else if (road->intersection2 == intersection)
		result = RoadEnterPoint(*road, 2);
	else
		result.position = intersection->position;

	return result;
}

bool EndFromIntersectionToRoad(DirectedPoint point, Intersection* intersection, Road* road) {
	bool result = false;

	Point endPoint = {};
	if (road->intersection1 == intersection)
		endPoint = RoadEnterPoint(*road, 1).position;
	else if (road->intersection2 == intersection)
		endPoint = RoadEnterPoint(*road, 2).position;
	else
		endPoint = intersection->position;

	result = PointEqual(point.position, endPoint);

	return result;
}

static DirectedPoint NextFromRoadSidewalkToIntersectionSidewalk(DirectedPoint startPoint, Road* road, Intersection* intersection) {
	DirectedPoint result = {};

	int laneIndex = LaneIndex(*road, startPoint.position);

	float roadLength = RoadLength(road);
	float parallelDistance = sideWalkWidth * 0.5f;
	float perpendicularDistance = road->width * 0.5f + sideWalkWidth * 0.5f;

	if (laneIndex > 0) {
		if (road->intersection1 == intersection)
			result.position = XYFromPositiveRoadCoord(road, parallelDistance, perpendicularDistance);
		else if (road->intersection2 == intersection)
			result.position = XYFromNegativeRoadCoord(road, parallelDistance, -perpendicularDistance);
	}
	else if (laneIndex < 0) {
		if (road->intersection1 == intersection)
			result.position = XYFromPositiveRoadCoord(road, parallelDistance, -perpendicularDistance);
		else if (road->intersection2 == intersection)
			result.position = XYFromNegativeRoadCoord(road, parallelDistance, perpendicularDistance);
	}

	return result;
}

static bool EndFromRoadSidewalkToIntersectionSidewalk(DirectedPoint point, Road* road, Intersection* intersection) {
	DirectedPoint endPoint = NextFromRoadSidewalkToIntersectionSidewalk(point, road, intersection);

	bool result = PointEqual(point.position, endPoint.position);
	return result;
}

static inline DirectedPoint NextFromIntersectionSidewalkToNothing(DirectedPoint startPoint, Intersection* intersection) {
	DirectedPoint result = startPoint;
	return result;
}

static inline bool EndFromIntersectionSidewalkToNothing(DirectedPoint point, Intersection* intersection) {
	bool result = true;
	return true;
}

static DirectedPoint NextFromIntersectionSidewalkToRoadSidewalk(DirectedPoint startPoint, Intersection* intersection, Road* road) {
	DirectedPoint result = {};
	Point startPosition = startPoint.position;
	Point center = intersection->position;

	float roadWidth = GetIntersectionRoadWidth(*intersection);
	bool leftHalf   = (startPosition.x <  center.x);
	bool rightHalf  = (startPosition.x >= center.x);
	bool topHalf    = (startPosition.y <  center.y);
	bool bottomHalf = (startPosition.y >= center.y);

	bool leftOut   = (startPosition.x < (center.x - roadWidth * 0.5f));
	bool rightOut  = (startPosition.x > (center.x + roadWidth * 0.5f));
	bool topOut    = (startPosition.y < (center.y - roadWidth * 0.5f));
	bool bottomOut = (startPosition.y > (center.y + roadWidth * 0.5f));

	bool leftRoad   = (intersection->leftRoad   == road);
	bool rightRoad  = (intersection->rightRoad  == road);
	bool topRoad    = (intersection->topRoad    == road);
	bool bottomRoad = (intersection->bottomRoad == road);

	float cornerDistance = (roadWidth * 0.5f) + (sideWalkWidth * 0.5f);
	Point topLeft     = Point{center.x - cornerDistance, center.y - cornerDistance};
	Point topRight    = Point{center.x + cornerDistance, center.y - cornerDistance};
	Point bottomLeft  = Point{center.x - cornerDistance, center.y + cornerDistance};
	Point bottomRight = Point{center.x + cornerDistance, center.y + cornerDistance};

	if (topHalf && leftHalf) {
		if (topRoad || leftRoad) {
			result = startPoint;
		}
		else if (rightRoad) {
			if (topOut)
				result.position = topRight;
			else
				result.position = topLeft;
		}
		else if (bottomRoad) {
			if (leftOut)
				result.position = bottomLeft;
			else
				result.position = topLeft;
		}
	}
	else if (topHalf && rightHalf) {
		if (topRoad || rightRoad) {
			result = startPoint;
		}
		else if (leftRoad) {
			if (topOut)
				result.position = topLeft;
			else
				result.position = topRight;
		}
		else if (bottomRoad) {
			if (rightOut)
				result.position = bottomRight;
			else
				result.position = topRight;
		}
	}
	else if (bottomHalf && leftHalf) {
		if (bottomRoad || leftRoad) {
			result = startPoint;
		}
		else if (rightRoad) {
			if (bottomOut)
				result.position = bottomRight;
			else
				result.position = bottomLeft;
		}
		else if (topRoad) {
			if (leftOut)
				result.position = topLeft;
			else
				result.position = bottomLeft;
		}
	}
	else if (bottomHalf && rightHalf) {
		if (bottomRoad || rightRoad) {
			result = startPoint;
		}
		else if (leftRoad) {
			if (bottomOut)
				result.position = bottomLeft;
			else
				result.position = bottomRight;
		}
		else if (topRoad) {
			if (rightOut)
				result.position = topRight;
			else
				result.position = bottomRight;
		}
	}

	return result;
}

static bool EndFromIntersectionSidewalkToRoadSidewalk(DirectedPoint point, Intersection* intersection, Road* road) {
	bool result = false;
	Point center = intersection->position;
	Point position = point.position;

	if (intersection->leftRoad == road)
		result = (position.x < center.x);
	else if (intersection->rightRoad == road)
		result = (position.x >= center.x);
	else if (intersection->topRoad == road)
		result = (position.y < center.y);
	else if (intersection->bottomRoad == road)
		result = (position.y >= center.y);

	return result;
}

// TODO: make this work with indexes instead of floating-point comparison
DirectedPoint NextNodePoint(PathNode* node, DirectedPoint startPoint) {
	DirectedPoint result = {};

	MapElem elem = node->elem;
	PathNode* next = node->next;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;

		if (!next)
			result = NextFromBuildingToNothing(startPoint, building);
		else if (next->elem.type == MapElemBuilding)
			result = NextFromBuildingToBuilding(startPoint, building, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = NextFromBuildingToRoad(startPoint, building, next->elem.road);
		else if (next->elem.type == MapElemIntersection)
			result = NextFromBuildingToIntersection(startPoint, building, next->elem.intersection);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) ; // NOTE: path should not end in a road
		else if (next->elem.type == MapElemBuilding)
			result = NextFromRoadToBuilding(startPoint, road, next->elem.building);
		else if (next->elem.type == MapElemIntersection)
			result = NextFromRoadToIntersection(startPoint, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersection) {
		Intersection* intersection = elem.intersection;

		if (!next)
			result = NextFromIntersectionToNothing(startPoint, intersection);
		else if (next->elem.type == MapElemBuilding)
			result = NextFromIntersectionToBuilding(startPoint, intersection, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = NextFromIntersectionToRoad(startPoint, intersection, next->elem.road);
		else
			result.position = elem.intersection->position;
	}
	else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;

		if (!next) ; // NOTE: path should not end in a road
		else if (next->elem.type == MapElemIntersectionSidewalk)
			result = NextFromRoadSidewalkToIntersectionSidewalk(startPoint, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersectionSidewalk) {
		Intersection* intersection = elem.intersection;

		if (!next)
			result = NextFromIntersectionSidewalkToNothing(startPoint, intersection);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = NextFromIntersectionSidewalkToRoadSidewalk(startPoint, intersection, next->elem.road);
	}

	return result;
}

bool IsNodeEndPoint(PathNode* node, DirectedPoint point) {
	bool result = false;

	MapElem elem = node->elem;
	PathNode* next = node->next;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;

		if (!next)
			result = EndFromBuildingToNothing(point, building);
		else if (next->elem.type == MapElemBuilding)
			result = EndFromBuildingToBuilding(point, building, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = EndFromBuildingToRoad(point, building, next->elem.road);
		else if (next->elem.type == MapElemIntersection)
			result = EndFromBuildingToIntersection(point, building, next->elem.intersection);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) 
			; // NOTE: path should not end with a road
		else if (next->elem.type == MapElemBuilding)
			result = EndFromRoadToBuilding(point, road, next->elem.building);
		else if (next->elem.type == MapElemIntersection)
			result = EndFromRoadToIntersection(point, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersection) {
		Intersection* intersection = elem.intersection;

		if (!next)
			result = EndFromIntersectionToNothing(point, intersection);
		else if (next->elem.type == MapElemBuilding)
			result = EndFromIntersectionToBuilding(point, intersection, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = EndFromIntersectionToRoad(point, intersection, next->elem.road);
	}
	else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;

		if (!next)
			; // NOTE: path should not end with a road
		else if (next->elem.type == MapElemIntersectionSidewalk)
			result = EndFromRoadSidewalkToIntersectionSidewalk(point, road, next->elem.intersection);
	}
	else if (elem.type == MapElemIntersectionSidewalk) {
		Intersection* intersection = elem.intersection;

		if (!next)
			result = EndFromIntersectionSidewalkToNothing(point, intersection);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = EndFromIntersectionSidewalkToRoadSidewalk(point, intersection, next->elem.road);
	}

	return result;
}

void DrawPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth) {
	PathNode* node = firstNode;

	if (node) {
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

void DrawBezierPathFromPoint(Renderer renderer, PathNode* firstNode, DirectedPoint startPoint, Color color, float lineWidth) {
	PathNode* node = firstNode;
	Bezier4 bezier4 = {};

	if (node) {
		DirectedPoint point = startPoint;

		while (node) {
			if (IsNodeEndPoint(node, point)) {
				node = node->next;
			}
			else {
				DirectedPoint nextPoint = NextNodePoint(node, point);
				bezier4 = TurnBezier4(point, nextPoint);
				DrawBezier4(renderer, bezier4, color, lineWidth, 5);

				point = nextPoint;
			}
		}
	}
}

void DrawBezierPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth) {
	PathNode* node = firstNode;
	Bezier4 bezier4 = {};

	if (node) {
		DirectedPoint startPoint = StartNodePoint(node);
		DrawBezierPathFromPoint(renderer, firstNode, startPoint, color, lineWidth);
	}
}