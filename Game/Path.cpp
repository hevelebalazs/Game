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
	node.elem.type = MapElemType::ROAD;
	node.elem.road = road;
	return node;
}

static PathNode IntersectionNode(Intersection* intersection) {
	PathNode node = {};
	node.elem.type = MapElemType::INTERSECTION;
	node.elem.intersection = intersection;
	return node;
}

static PathNode BuildingNode(Building* building) {
	PathNode node = {};
	node.elem.type = MapElemType::BUILDING;
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
	while (building->connectElem.type == MapElemType::BUILDING) {
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

	if (elemStart.type == MapElemType::ROAD) {
		AddRoadToHelper(map, elemStart.road, -1, helper);
	}
	else if (elemStart.type == MapElemType::INTERSECTION) {
		AddIntersectionToHelper(map, elemStart.intersection, -1, helper);
	}

	if (elemStart.address == elemEnd.address) {
		PushElem(path, elemStart);
		return;
	}

	Road* roadEnd = 0;
	if (elemEnd.type == MapElemType::ROAD) roadEnd = elemEnd.road;

	Intersection* intersectionEnd = 0;
	if (elemEnd.type == MapElemType::INTERSECTION) intersectionEnd = elemEnd.intersection;

	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemType::ROAD) {
			Road* road = elem.road;

			AddIntersectionToHelper(map, road->intersection1, i, helper);
			if (intersectionEnd && road->intersection1 == intersectionEnd) break;

			AddIntersectionToHelper(map, road->intersection2, i, helper);
			if (intersectionEnd && road->intersection2 == intersectionEnd) break;
		}
		else if (elem.type == MapElemType::INTERSECTION) {
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
	while (building->connectElem.type == MapElemType::BUILDING) {
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

	if (elemStart.type == MapElemType::BUILDING && elemEnd.type == MapElemType::BUILDING) {
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

			return path;
		}
	}

	MapElem roadElemStart = {};
	if (elemStart.type == MapElemType::BUILDING) {
		PushFromBuildingToRoadElem(&path, elemStart.building);

		roadElemStart = GetConnectRoadElem(elemStart.building);
	}
	else {
		roadElemStart = elemStart;
	}

	MapElem roadElemEnd = {};
	if (elemEnd.type == MapElemType::BUILDING) {
		roadElemEnd = GetConnectRoadElem(elemEnd.building);
	}
	else {
		roadElemEnd = elemEnd;
	}

	PushConnectRoadElems(&path, map, roadElemStart, roadElemEnd, helper);

	if (elemEnd.type == MapElemType::BUILDING) {
		PushFromRoadElemToBuilding(&path, elemEnd.building);
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

DirectedPoint PathNode::StartPoint() {
	Point position = {};
	Point direction = {};

	if (elem.type == MapElemType::BUILDING) {
		Building* building = elem.building;

		position = building->connectPointClose;
		direction = PointDirection(building->connectPointClose, building->connectPointFar);
	}
	else if (elem.type == MapElemType::INTERSECTION) {
		position = elem.intersection->coordinate;
		// TODO: add direction
	}

	DirectedPoint result = {};
	result.position = position;
	result.direction = direction;
	return result;
}

DirectedPoint PathNode::NextPoint(DirectedPoint startPoint) {
	Point position = {};
	Point direction = {};

	Point startPosition = startPoint.position;

	if (elem.type == MapElemType::BUILDING) {
		Building* building = elem.building;

		if (!next) {
			position = building->connectPointClose;
			direction = PointDirection(building->connectPointFar, building->connectPointClose);
		}
		else if (next->elem.type == MapElemType::BUILDING) {
			// TODO: add direction
			Building* nextBuilding = next->elem.building;

			if (elem.building->connectElem.building == nextBuilding) {
				if (startPosition == building->connectPointClose) {
					position = building->connectPointFar;
				}
				else {
					position = NextPointAroundBuilding(building, startPosition, building->connectPointClose);
				}
			}
			else if (nextBuilding->connectElem.building == building) {
				if (startPosition == building->connectPointFar || startPosition == building->connectPointFarShow) {
					position = building->connectPointClose;
				}
				else {
					position = NextPointAroundBuilding(building, startPosition, nextBuilding->connectPointFar);
				}
			}
		}
		else if (next->elem.type == MapElemType::ROAD) {
			Road* road = next->elem.road;

			if (startPosition == building->connectPointClose) {
				position = building->connectPointFarShow;
				direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
			}
			else if (startPosition == building->connectPointFarShow) {
				Point roadDirection = {};

				PathNode* nextNext = next->next;
				if (nextNext) {
					MapElem nextNextElem = nextNext->elem;
					Point nextNextPoint = {};
					if (nextNextElem.type == MapElemType::BUILDING) {
						nextNextPoint = nextNextElem.building->connectPointFar;
					}
					else if (nextNextElem.type == MapElemType::INTERSECTION) {
						nextNextPoint = nextNextElem.intersection->coordinate;
					}

					roadDirection = PointDirection(building->connectPointFar, nextNextPoint);
				}

				position = building->connectPointFar + (road->width * 0.5f * roadDirection);
				direction = roadDirection;
			}
			else {
				// TODO: add direction
				position = NextPointAroundBuilding(building, startPosition, building->connectPointClose);
			}
		}
		else if (next->elem.type == MapElemType::INTERSECTION) {
			if (startPosition == building->connectPointClose) {
				position = building->connectPointFarShow;
				direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
			}
			else {
				// TODO: add direction
				position = NextPointAroundBuilding(building, startPosition, building->connectPointClose);
			}
		}
	}
	else if (elem.type == MapElemType::ROAD) {
		Road* road = elem.road;

		if (!next) {
			// NOTE: a path should not end in a road
		}
		else if (next->elem.type == MapElemType::BUILDING) {
			Building* building = next->elem.building;

			// NOTE: the previous point was on the same road, so it has the same direction
			Point comparePoint = building->connectPointFar - (startPoint.direction * road->width * 0.5f);

			if (startPosition == comparePoint) {
				position = building->connectPointFarShow;
				direction = PointDirection(building->connectPointFarShow, building->connectPointClose);
			}
			else {
				position = comparePoint;
				direction = startPoint.direction;
			}
		}
		else if (next->elem.type == MapElemType::INTERSECTION) {
			Intersection* intersection = next->elem.intersection;

			if (intersection == road->intersection1) {
				position = road->endPoint1;

				// TODO: add an angle to Road structure so this does not have to be recalculated?
				direction = PointDirection(road->endPoint2, road->endPoint1);
			}
			else if (intersection == road->intersection2) {
				position = road->endPoint2;
				direction = PointDirection(road->endPoint1, road->endPoint2);
			}
			else {
				position = next->elem.intersection->coordinate;
			}
		}
	}
	else if (elem.type == MapElemType::INTERSECTION) {
		Intersection* intersection = elem.intersection;

		if (next && next->elem.type == MapElemType::BUILDING) {
			Building* nextBuilding = next->elem.building;

			position = nextBuilding->connectPointFarShow;
			direction = PointDirection(nextBuilding->connectPointFarShow, nextBuilding->connectPointClose);
		}
		else if (next && next->elem.type == MapElemType::ROAD) {
			Road* road = next->elem.road;

			if (road->intersection1 == intersection) {
				position = road->endPoint1;
				direction = PointDirection(road->endPoint1, road->endPoint2);
			}
			else if (road->intersection2 == intersection) {
				position = road->endPoint2;
				direction = PointDirection(road->endPoint2, road->endPoint1);
			}
			else {
				position = intersection->coordinate;
			}
		}
		else {
			position = intersection->coordinate;
		}
	}

	DirectedPoint result = {};
	result.position = position;
	result.direction = direction;

	return result;
}

bool PathNode::IsEndPoint(DirectedPoint point) {
	Point position = point.position;
	Point direction = point.direction;

	bool result = false;

	if (elem.type == MapElemType::BUILDING) {
		Building* building = elem.building;

		if (!next) {
			result = (position == building->connectPointClose);
		}
		else if (next->elem.type == MapElemType::BUILDING) {
			Building* nextBuilding = next->elem.building;

			if (building->connectElem.type == MapElemType::BUILDING && building->connectElem.building == nextBuilding) {
				result = (position == building->connectPointFar);
			}
			else if (nextBuilding->connectElem.type == MapElemType::BUILDING && nextBuilding->connectElem.building == building) {
				result = (position == nextBuilding->connectPointFar);
			}
		}
		else if (next->elem.type == MapElemType::ROAD) {
			Road* road = next->elem.road;

			Point roadDirection = {};

			PathNode* nextNext = next->next;
			if (nextNext) {
				MapElem nextNextElem = nextNext->elem;
				Point nextNextPoint = {};
				if (nextNextElem.type == MapElemType::BUILDING) {
					nextNextPoint = nextNextElem.building->connectPointFar;
				}
				else if (nextNextElem.type == MapElemType::INTERSECTION) {
					nextNextPoint = nextNextElem.intersection->coordinate;
				}

				roadDirection = PointDirection(building->connectPointFar, nextNextPoint);
			}

			Point comparePoint = building->connectPointFar + (road->width * 0.5f * roadDirection);
			result = (position == comparePoint);
		}
		else if (next->elem.type == MapElemType::INTERSECTION) {
			result = (position == building->connectPointFarShow);
		}
	}
	else if (elem.type == MapElemType::ROAD) {
		Road* road = elem.road;

		if (!next) {
			// NOTE: path should not end with a road
		}
		else if (next->elem.type == MapElemType::BUILDING) {
			Building* building = next->elem.building;

			result = (position == building->connectPointFarShow);
		}
		else if (next->elem.type == MapElemType::INTERSECTION) {
			Intersection* intersection = next->elem.intersection;

			if (intersection == road->intersection1) result = (position == road->endPoint1);
			else if (intersection == road->intersection2) result = (position == road->endPoint2);
			else result = (position == intersection->coordinate);
		}
	}
	else if (elem.type == MapElemType::INTERSECTION) {
		Intersection* intersection = elem.intersection;

		if (next && next->elem.type == MapElemType::BUILDING) {
			Building* building = next->elem.building;

			result = (position == building->connectPointFarShow);
		}
		else if (next && next->elem.type == MapElemType::ROAD) {
			Road* road = next->elem.road;

			if (road->intersection1 == intersection) result = (position == road->endPoint1);
			else if (road->intersection2 == intersection) result = (position == road->endPoint2);
			else result = (position == intersection->coordinate);
		}
		else {
			result = (position == intersection->coordinate);
		}
	}

	return result;
}

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth) {
	if (path->nodeCount > 0) {
		PathNode* node = &path->nodes[0];
		DirectedPoint point = node->StartPoint();

		while (node) {
			if (node->IsEndPoint(point)) {
				node = node->next;
			}
			else {
				DirectedPoint nextPoint = node->NextPoint(point);

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
		DirectedPoint point = node->StartPoint();

		while (node) {
			if (node->IsEndPoint(point)) {
				node = node->next;
			}
			else {
				DirectedPoint nextPoint = node->NextPoint(point);
				bezier4 = TurnBezier4(point, nextPoint);
				DrawBezier4(bezier4, renderer, color, lineWidth, 3);

				point = nextPoint;
			}
		}
	}
}