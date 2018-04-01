#include "Bezier.h"
#include "Geometry.h"
#include "Memory.h"
#include "Path.h"

struct Path {
	int nodeCount;
	PathNode* nodes;
	MemArena* arena;
};

static inline void PushNode(Path* path, PathNode node) {
	ArenaPushType(path->arena, PathNode);
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

static inline PathNode JunctionNode(Junction* junction) {
	PathNode node = {};
	node.elem = JunctionElem(junction);
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

static inline void PushJunction(Path* path, Junction* junction) {
	PathNode node = JunctionNode(junction);

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

	int* isJunctionHelper;
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

static void AddJunctionToHelper(Map* map, Junction* junction, int sourceIndex, PathHelper* pathHelper) {
	int junctionIndex = (int)(junction - map->junctions);

	if (pathHelper->isJunctionHelper[junctionIndex] == 0) {
		pathHelper->isJunctionHelper[junctionIndex] = 1;

		pathHelper->nodes[pathHelper->nodeCount] = JunctionNode(junction);
		pathHelper->sourceIndex[pathHelper->nodeCount] = sourceIndex;
		pathHelper->nodeCount++;
	}
}

static inline void ConnectRoadElemsHelper(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper) {
	helper->nodeCount = 0;

	if (elemStart.address == elemEnd.address) {
		helper->nodes[helper->nodeCount].elem = elemStart;
		helper->nodes[helper->nodeCount].next = 0;
		helper->sourceIndex[helper->nodeCount] = -1;
		helper->nodeCount++;
		return;
	}

	for (int i = 0; i < map->junctionCount; ++i) 
		helper->isJunctionHelper[i] = 0;
	for (int i = 0; i < map->roadCount; ++i)
		helper->isRoadHelper[i] = 0;

	if (elemStart.type == MapElemRoad)
		AddRoadToHelper(map, elemStart.road, -1, helper);
	else if (elemStart.type == MapElemJunction)
		AddJunctionToHelper(map, elemStart.junction, -1, helper);

	Road* roadEnd = 0;
	if (elemEnd.type == MapElemRoad) 
		roadEnd = elemEnd.road;

	Junction* junctionEnd = 0;
	if (elemEnd.type == MapElemJunction) 
		junctionEnd = elemEnd.junction;

	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemRoad) {
			Road* road = elem.road;

			AddJunctionToHelper(map, road->junction1, i, helper);
			if (junctionEnd && road->junction1 == junctionEnd)
				break;

			AddJunctionToHelper(map, road->junction2, i, helper);
			if (junctionEnd && road->junction2 == junctionEnd)
				break;
		}
		else if (elem.type == MapElemJunction) {
			Junction* junction = elem.junction;

			if (junction->leftRoad) {
				AddRoadToHelper(map, junction->leftRoad, i, helper);
				if (roadEnd && junction->leftRoad == roadEnd)
					break;
			}

			if (junction->rightRoad) {
				AddRoadToHelper(map, junction->rightRoad, i, helper);
				if (roadEnd && junction->rightRoad == roadEnd)
					break;
			}

			if (junction->topRoad) {
				AddRoadToHelper(map, junction->topRoad, i, helper);
				if (roadEnd && junction->topRoad == roadEnd)
					break;
			}

			if (junction->bottomRoad) {
				AddRoadToHelper(map, junction->bottomRoad, i, helper);
				if (roadEnd && junction->bottomRoad == roadEnd)
					break;
			}
		}
	}
}

static void PushConnectRoadElems(Path* path, Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper) {
	ConnectRoadElemsHelper(map, elemStart, elemEnd, helper);

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

static inline MapElem SidewalkElemToRoadElem(MapElem sidewalkElem) {
	MapElem roadElem = sidewalkElem;

	if (sidewalkElem.type == MapElemRoadSidewalk)
		roadElem.type = MapElemRoad;
	else if (sidewalkElem.type == MapElemJunctionSidewalk)
		roadElem.type = MapElemJunction;

	return roadElem;
}

static inline MapElem RoadElemToSidewalkElem(MapElem roadElem) {
	MapElem sidewalkElem = roadElem;

	if (roadElem.type == MapElemRoad)
		sidewalkElem.type = MapElemRoadSidewalk;
	else if (roadElem.type == MapElemJunction)
		sidewalkElem.type = MapElemJunctionSidewalk;

	return sidewalkElem;
}

// TODO: create a LaneIndex enum
static void PushRoadSidewalk(Path* path, Road* road, int laneIndex) {
	PathNode node = {};
	node.elem.type = MapElemRoadSidewalk;
	node.elem.road = road;
	node.subElemIndex = laneIndex;
	PushNode(path, node);
}

// TODO: create a QuarterIndex enum
static void PushJunctionSidewalk(Path* path, Junction* junction, int quarterIndex) {
	PathNode node = {};
	node.elem.type = MapElemJunctionSidewalk;
	node.elem.junction = junction;
	node.subElemIndex = quarterIndex;
	PushNode(path, node);
}

static void PushCrossRoad(Path* path, Road* road, Point startPoint, Junction* junction, int quarterIndex) {
	if (road) {
		int endLaneIndex = -LaneIndex(*road, startPoint);
		PushJunctionSidewalk(path, junction, quarterIndex);
		PushRoadSidewalk(path, road, endLaneIndex);
	}
}

static void PushConnectSidewalkElems(Path* path, Map* map, MapElem elemStart, Point startPoint, MapElem elemEnd, PathHelper* helper) {
	MapElem roadElemStart = SidewalkElemToRoadElem(elemStart);
	MapElem roadElemEnd = SidewalkElemToRoadElem(elemEnd);

	// NOTE: the start and end elems are passed in a reverse order so that iteration will be easier
	// TODO: make ConnectRoadElemsHelper swap the elements by default?
	ConnectRoadElemsHelper(map, roadElemEnd, roadElemStart, helper);

	Point point = startPoint;

	int nodeIndex = helper->nodeCount - 1;
	while (nodeIndex > -1) {
		PathNode roadNode = helper->nodes[nodeIndex];
		PathNode sidewalkNode = {};
		sidewalkNode.elem = RoadElemToSidewalkElem(roadNode.elem);
		MapElem elem = sidewalkNode.elem;

		int nextIndex = helper->sourceIndex[nodeIndex];
		PathNode nextRoadNode = {};
		PathNode nextSidewalkNode = {};
		MapElem nextElem = {};
		if (nextIndex > -1) {
			nextRoadNode = helper->nodes[nextIndex];
			nextSidewalkNode.elem = RoadElemToSidewalkElem(nextRoadNode.elem);
			nextElem = nextSidewalkNode.elem;
		}

		if (elem.type == MapElemJunctionSidewalk) {
			Junction* junction = elem.junction;

			if (nextElem.type == MapElemNone) {
				PushJunctionSidewalk(path, junction, 0);
			}
			else if (nextElem.type == MapElemRoadSidewalk) {
				Road* road = nextElem.road;

				int quarterIndex = QuarterIndex(elem.junction, point);

				bool leftRoad   = (junction->leftRoad == road);
				bool rightRoad  = (junction->rightRoad == road);
				bool topRoad    = (junction->topRoad == road);
				bool bottomRoad = (junction->bottomRoad == road);

				int endQuarterIndex = 0;
				if (quarterIndex == QuarterTopLeft) {
					if (topRoad || leftRoad) {
						endQuarterIndex = QuarterTopLeft;
					}
					else if (rightRoad) {
						PushCrossRoad(path, junction->topRoad, point, junction, QuarterTopLeft);
						endQuarterIndex = QuarterTopRight;
					}
					else if (bottomRoad) {
						PushCrossRoad(path, junction->leftRoad, point, junction, QuarterTopLeft);
						endQuarterIndex = QuarterBottomLeft;
					}
				}
				else if (quarterIndex == QuarterTopRight) {
					if (topRoad || rightRoad) {
						endQuarterIndex = QuarterTopRight;
					}
					else if (leftRoad) {
						PushCrossRoad(path, junction->topRoad, point, junction, QuarterTopRight);
						endQuarterIndex = QuarterTopLeft;
					}
					else if (bottomRoad) {
						PushCrossRoad(path, junction->rightRoad, point, junction, QuarterTopRight);
						endQuarterIndex = QuarterBottomRight;
					}
				}
				else if (quarterIndex == QuarterBottomLeft) {
					if (bottomRoad || leftRoad) {
						endQuarterIndex = QuarterBottomLeft;
					}
					else if (rightRoad) {
						PushCrossRoad(path, junction->bottomRoad, point, junction, QuarterBottomLeft);
						endQuarterIndex = QuarterBottomRight;
					}
					else if (topRoad) {
						PushCrossRoad(path, junction->leftRoad, point, junction, QuarterBottomLeft);
						endQuarterIndex = QuarterTopLeft;
					}
				}
				else if (quarterIndex == QuarterBottomRight) {
					if (bottomRoad || rightRoad) {
						endQuarterIndex = QuarterBottomRight;
					}
					else if (leftRoad) {
						PushCrossRoad(path, junction->bottomRoad, point, junction, QuarterBottomRight);
						endQuarterIndex = QuarterBottomLeft;
					}
					else if (topRoad) {
						PushCrossRoad(path, junction->rightRoad, point, junction, QuarterBottomRight);
						endQuarterIndex = QuarterTopRight;
					}
				}

				PushJunctionSidewalk(path, junction, endQuarterIndex);
				point = JunctionSidewalkCorner(junction, endQuarterIndex);
			}
		}
		else if (elem.type == MapElemRoadSidewalk) {
			Road* road = elem.road;

			if (nextElem.type == MapElemNone) {
				PushRoadSidewalk(path, road, 0);
			}
			else if (nextElem.type == MapElemJunctionSidewalk) {
				Junction* junction = nextElem.junction;
				Road* nextRoad = 0;

				int nextNextIndex = helper->sourceIndex[nextIndex];
				PathNode nextNextRoadNode = {};

				if (nextNextIndex > -1) {
					nextNextRoadNode = helper->nodes[nextNextIndex];

					if (nextNextRoadNode.elem.type == MapElemRoad)
						nextRoad = nextNextRoadNode.elem.road;
				}

				bool hasToTurn = false;
				if (junction && nextRoad) {
					hasToTurn |= ((road == junction->leftRoad) && (nextRoad != junction->rightRoad));
					hasToTurn |= ((road == junction->rightRoad) && (nextRoad != junction->leftRoad));
					hasToTurn |= ((road == junction->topRoad) && (nextRoad != junction->bottomRoad));
					hasToTurn |= ((road == junction->bottomRoad) && (nextRoad != junction->topRoad));
				}

				int endLaneIndex = 0;
				if (hasToTurn)
					endLaneIndex = LaneIndex(*road, nextRoad->endPoint1);
				else
					endLaneIndex = LaneIndex(*road, point);
			
				PushRoadSidewalk(path, road, endLaneIndex);
			
				float parallelDistance = (SidewalkWidth * 0.5f);
				float perpendicularDistance = (LaneWidth) + (SidewalkWidth * 0.5f);
				if (road->junction1 == junction) {
					if (endLaneIndex > 0)
						point = FromRoadCoord1(*road, parallelDistance, +perpendicularDistance);
					else
						point = FromRoadCoord1(*road, parallelDistance, -perpendicularDistance);
				}
				else if (road->junction2 == junction) {
					if (endLaneIndex > 0)
						point = FromRoadCoord2(*road, parallelDistance, +perpendicularDistance);
					else
						point = FromRoadCoord2(*road, parallelDistance, -perpendicularDistance);
				}
			}
		}

		nodeIndex = nextIndex;
	}
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

static PathNode* PushPathToPool(Path path, MemArena* arena, PathPool* pathPool) {
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
	helper.nodes = ArenaPushArray(arena, PathNode, map->junctionCount + map->roadCount);
	helper.isJunctionHelper = ArenaPushArray(arena, int, map->junctionCount);
	helper.isRoadHelper = ArenaPushArray(arena, int, map->roadCount);
	helper.sourceIndex = ArenaPushArray(arena, int, map->junctionCount + map->roadCount);

	return helper;
}

// TODO: rename this to VehiclePath?
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	if (elemStart.type == MapElemRoadSidewalk || elemStart.type == MapElemJunctionSidewalk) 
		return 0;
	if (elemEnd.type == MapElemRoadSidewalk || elemEnd.type == MapElemJunctionSidewalk)
		return 0;

	PathHelper helper = PathHelperForMap(map, tmpArena);

	Path path = {};
	path.arena = tmpArena;
	path.nodes = ArenaPushArray(path.arena, PathNode, 0);

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

		for (int i = 0; i < map->junctionCount; ++i)
			helper.isJunctionHelper[i] = 0;

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

			Junction* startJunction = 0;
			if (laneIndex == 1)
				startJunction = startConnectRoad->junction2;
			else if (laneIndex == -1)
				startJunction = startConnectRoad->junction1;

			if (startJunction) {
				startConnectRoad = startConnectRoad;
				roadElemStart.type = MapElemJunction;
				roadElemStart.junction = startJunction;
			}
		}

		if (endConnectRoad) {
			Building* endBuilding = elemEnd.building;

			int laneIndex = LaneIndex(*endConnectRoad, endBuilding->connectPointFarShow);
				
			Junction* endJunction = 0;
			if (laneIndex == 1)
				endJunction = endConnectRoad->junction1;
			else if (laneIndex == -1)
				endJunction = endConnectRoad->junction2;

			if (endJunction) {
				endConnectRoad = endConnectRoad;
				roadElemEnd.type = MapElemJunction;
				roadElemEnd.junction = endJunction;
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

	PathNode* firstNode = PushPathToPool(path, arena, pathPool);

	ArenaPopTo(tmpArena, helper.nodes);

	return firstNode;
}

// TODO: remove arena from argument list?
// TODO: rename this to PedestrianPath?
PathNode* ConnectPedestrianElems(Map* map, MapElem elemStart, Point startPoint, MapElem elemEnd, 
							   MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	// TODO: assert instead of these checks?
	if (elemStart.type != MapElemRoadSidewalk && elemStart.type != MapElemJunctionSidewalk && elemStart.type != MapElemCrossing) 
		return 0;
	if (elemEnd.type != MapElemRoadSidewalk && elemEnd.type != MapElemJunctionSidewalk && elemEnd.type != MapElemCrossing)
		return 0;

	PathHelper helper = PathHelperForMap(map, tmpArena);

	// TODO: create a ResetPathHelper function?
	//       or move this to where the helper is actually used?
	for (int i = 0; i < map->roadCount; ++i)
		helper.isRoadHelper[i] = 0;

	for (int i = 0; i < map->junctionCount; ++i)
		helper.isJunctionHelper[i] = 0;

	Path path = {};
	path.arena = tmpArena;
	path.nodes = ArenaPushArray(path.arena, PathNode, 0);

	MapElem sidewalkElemStart = elemStart;
	if (elemStart.type == MapElemCrossing)
		sidewalkElemStart = RoadSidewalkElem(elemStart.road);

	MapElem sidewalkElemEnd = elemEnd;
	if (elemEnd.type == MapElemCrossing)
		sidewalkElemEnd = RoadSidewalkElem(elemEnd.road);

	PushConnectSidewalkElems(&path, map, sidewalkElemStart, startPoint, sidewalkElemEnd, &helper);

	PathNode* firstNode = PushPathToPool(path, arena, pathPool);

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
	else if (elem.type == MapElemJunction || elem.type == MapElemJunctionSidewalk) {
		position = elem.junction->position;
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

DirectedPoint NextFromBuildingToJunction(DirectedPoint startPoint, Building* building, Junction* junction) {
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

bool EndFromBuildingToJunction(DirectedPoint point, Building* building, Junction* junction) {
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

DirectedPoint NextFromRoadToJunction(DirectedPoint startPoint, Road* road, Junction* junction) {
	DirectedPoint result = {};

	bool onRoadSide = IsPointOnRoadSide(startPoint.position, *road);

	if (onRoadSide) {
		int laneIndex = LaneIndex(*road, startPoint.position);
		result = TurnPointToLane(*road, laneIndex, startPoint.position);
	}
	else if (junction == road->junction1) {
		result = RoadLeavePoint(*road, 1);
	}
	else if (junction == road->junction2) {
		result = RoadLeavePoint(*road, 2);
	}
	else {
		result.position = junction->position;
	}

	return result;
}

bool EndFromRoadToJunction(DirectedPoint point, Road* road, Junction* junction) {
	bool result = false;

	Point endPoint = {};
	if (junction == road->junction1)
		endPoint = RoadLeavePoint(*road, 1).position;
	else if (junction == road->junction2)
		endPoint = RoadLeavePoint(*road, 2).position;
	else
		endPoint = junction->position;

	result = PointEqual(point.position, endPoint);

	return result;
}

DirectedPoint NextFromJunctionToNothing(DirectedPoint startPoint, Junction* junction) {
	DirectedPoint result = {};

	result.position = junction->position;

	return result;
}

bool EndFromJunctionToNothing(DirectedPoint point, Junction* junction) {
	bool result = PointEqual(point.position, junction->position);
	return result;
}

DirectedPoint NextFromJunctionToBuilding(DirectedPoint startPoint, Junction* junction, Building* building) {
	DirectedPoint result = {};

	result.position = building->connectPointFarShow;
	result.direction = PointDirection(building->connectPointFarShow, building->connectPointClose);

	return result;
}

bool EndFromJunctionToBuilding(DirectedPoint point, Junction* junction, Building* building) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

DirectedPoint NextFromJunctionToRoad(DirectedPoint startPoint, Junction* junction, Road* road) {
	DirectedPoint result = {};

	if (road->junction1 == junction)
		result = RoadEnterPoint(*road, 1);
	else if (road->junction2 == junction)
		result = RoadEnterPoint(*road, 2);
	else
		result.position = junction->position;

	return result;
}

bool EndFromJunctionToRoad(DirectedPoint point, Junction* junction, Road* road) {
	bool result = false;

	Point endPoint = {};
	if (road->junction1 == junction)
		endPoint = RoadEnterPoint(*road, 1).position;
	else if (road->junction2 == junction)
		endPoint = RoadEnterPoint(*road, 2).position;
	else
		endPoint = junction->position;

	result = PointEqual(point.position, endPoint);

	return result;
}

static DirectedPoint NextFromRoadSidewalkToJunctionSidewalk
						(DirectedPoint startPoint, Road* road, int endSidewalkIndex, Junction* junction) {
	DirectedPoint result = {};

	int sidewalkIndex = RoadSidewalkIndex(*road, startPoint.position);

	if (sidewalkIndex != endSidewalkIndex) {
		Point startRoadCoord = ToRoadCoord(*road, startPoint.position);
		Point resultRoadCoord = {};
		resultRoadCoord.x = road->crossingDistance;

		float perpendicularDistance = (LaneWidth) + (SidewalkWidth * 0.5f);
		if (Abs(startRoadCoord.x - road->crossingDistance) <= CrossingWidth * 0.5f)
			resultRoadCoord.y = (endSidewalkIndex * perpendicularDistance);
		else
			resultRoadCoord.y = (sidewalkIndex * perpendicularDistance);

		result.position = FromRoadCoord(*road, resultRoadCoord);
	}
	else {
		float roadLength = RoadLength(road);
		float parallelDistance = SidewalkWidth * 0.5f;
		float perpendicularDistance = LaneWidth + SidewalkWidth * 0.5f;

		if (sidewalkIndex > 0) {
			if (road->junction1 == junction)
				result.position = FromRoadCoord1(*road, parallelDistance, +perpendicularDistance);
			else if (road->junction2 == junction)
				result.position = FromRoadCoord2(*road, parallelDistance, +perpendicularDistance);
		}
		else if (sidewalkIndex < 0) {
			if (road->junction1 == junction)
				result.position = FromRoadCoord1(*road, parallelDistance, -perpendicularDistance);
			else if (road->junction2 == junction)
				result.position = FromRoadCoord2(*road, parallelDistance, -perpendicularDistance);
		}
	}

	return result;
}

static bool EndFromRoadSidewalkToJunctionSidewalk
				(DirectedPoint point, Road* road, int endLaneIndex, Junction* junction) {
	bool result = false;

	int laneIndex = LaneIndex(*road, point.position);
	if (laneIndex == endLaneIndex) {
		Point junctionRoadCoord = ToRoadCoord(*road, junction->position);
		Point pointRoadCoord = ToRoadCoord(*road, point.position);

		float roadLength = RoadLength(road);
		if (junctionRoadCoord.x < (roadLength * 0.5f))
			result = IsBetween(pointRoadCoord.x, 0.0f, SidewalkWidth);
		else
			result = IsBetween(pointRoadCoord.x, roadLength - SidewalkWidth, roadLength);
	}

	return result;
}

static DirectedPoint NextFromRoadSidewalkToRoadSidewalk(DirectedPoint point, Road* road, Road* nextRoad) {
	DirectedPoint result = {};

	Point pointRoadCoord = ToRoadCoord(*road, point.position);
	Point nextRoadCoord = ToRoadCoord(*road, nextRoad->endPoint1);

	int pointLaneIndex = 1;
	if (pointRoadCoord.y < 0.0f)
		pointLaneIndex = -1;

	int nextRoadLaneIndex = 1;
	if (nextRoadCoord.y < 0.0f)
		nextRoadLaneIndex = -1;

	if (pointLaneIndex == nextRoadLaneIndex) {
		float roadLength = RoadLength(road);
		if (nextRoadCoord.x < roadLength * 0.5f)
			pointRoadCoord.x = SidewalkWidth * 0.5f;
		else
			pointRoadCoord.x = roadLength - SidewalkWidth * 0.5f;
	}
	else {
		if ((pointRoadCoord.x >= road->crossingDistance - CrossingWidth * 0.5f) 
			&& (pointRoadCoord.x <= road->crossingDistance + CrossingWidth * 0.5f))
			pointRoadCoord.y = nextRoadLaneIndex * (LaneWidth + SidewalkWidth * 0.5f); 
		else
			pointRoadCoord.x = road->crossingDistance;
	}

	result.position = FromRoadCoord(*road, pointRoadCoord);

	return result;
}

static bool EndFromRoadSidewalkToRoadSidewalk(DirectedPoint point, Road* road, Road* nextRoad) {
	bool result = false;

	Point pointRoadCoord = ToRoadCoord(*road, point.position);
	Point nextRoadCoord = ToRoadCoord(*road, nextRoad->endPoint1);

	Point debug = FromRoadCoord(*road, pointRoadCoord);

	float roadLength = RoadLength(road);
	if (nextRoadCoord.x < roadLength * 0.5f)
		result = (pointRoadCoord.x < SidewalkWidth);
	else
		result = (pointRoadCoord.x > roadLength - SidewalkWidth);

	return result;
}

static inline DirectedPoint NextFromJunctionSidewalkToNothing(DirectedPoint startPoint, Junction* junction) {
	DirectedPoint result = startPoint;
	return result;
}

static inline bool EndFromJunctionSidewalkToNothing(DirectedPoint point, Junction* junction) {
	bool result = true;
	return result;
}

static DirectedPoint NextFromJunctionSidewalkToRoadSidewalk
						(DirectedPoint startPoint, Junction* junction, int endQuarterIndex, Road* road) {
	DirectedPoint result = {};
	int quarterIndex = endQuarterIndex;

	float roadWidth = 2.0f * LaneWidth;
	bool leftOut   = (startPoint.position.x < junction->position.x - roadWidth * 0.5f);
	bool rightOut  = (startPoint.position.x > junction->position.x + roadWidth * 0.5f);
	bool topOut    = (startPoint.position.y < junction->position.y - roadWidth * 0.5f);
	bool bottomOut = (startPoint.position.y > junction->position.y + roadWidth * 0.5f);

	if (endQuarterIndex == QuarterTopLeft) {
		if (topOut || leftOut)
			quarterIndex = QuarterTopLeft;
		else if (rightOut)
			quarterIndex = QuarterTopRight;
		else if (bottomOut)
			quarterIndex = QuarterBottomLeft;
	}
	else if (endQuarterIndex == QuarterTopRight) {
		if (topOut || rightOut)
			quarterIndex = QuarterTopRight;
		else if (leftOut)
			quarterIndex = QuarterTopLeft;
		else if (bottomOut)
			quarterIndex = QuarterBottomRight;
	}
	else if (endQuarterIndex == QuarterBottomLeft) {
		if (bottomOut || leftOut)
			quarterIndex = QuarterBottomLeft;
		else if (rightOut)
			quarterIndex = QuarterBottomRight;
		else if (topOut)
			quarterIndex = QuarterTopLeft;
	}
	else if (endQuarterIndex == QuarterBottomRight) {
		if (bottomOut || rightOut)
			quarterIndex = QuarterBottomRight;
		else if (leftOut)
			quarterIndex = QuarterBottomLeft;
		else if (topOut)
			quarterIndex = QuarterTopRight;
	}

	result.position = JunctionSidewalkCorner(junction, quarterIndex);
	return result;
}

static bool EndFromJunctionSidewalkToRoadSidewalk
						(DirectedPoint point, Junction* junction, int endQuarterIndex, Road* road) {
	int quarterIndex = QuarterIndex(junction, point.position);
	bool result = (quarterIndex == endQuarterIndex);

	if (result) {
		int debug = 5;
	}

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
		else if (next->elem.type == MapElemJunction)
			result = NextFromBuildingToJunction(startPoint, building, next->elem.junction);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) ; // NOTE: path should not end in a road
		else if (next->elem.type == MapElemBuilding)
			result = NextFromRoadToBuilding(startPoint, road, next->elem.building);
		else if (next->elem.type == MapElemJunction)
			result = NextFromRoadToJunction(startPoint, road, next->elem.junction);
	}
	else if (elem.type == MapElemJunction) {
		Junction* junction = elem.junction;

		if (!next)
			result = NextFromJunctionToNothing(startPoint, junction);
		else if (next->elem.type == MapElemBuilding)
			result = NextFromJunctionToBuilding(startPoint, junction, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = NextFromJunctionToRoad(startPoint, junction, next->elem.road);
		else
			result.position = elem.junction->position;
	}
	else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;
		int laneIndex = node->subElemIndex;

		if (!next) ; // NOTE: path should not end in a road
		else if (next->elem.type == MapElemJunctionSidewalk)
			result = NextFromRoadSidewalkToJunctionSidewalk(startPoint, road, laneIndex, next->elem.junction);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = NextFromRoadSidewalkToRoadSidewalk(startPoint, road, next->elem.road);
	}
	else if (elem.type == MapElemJunctionSidewalk) {
		Junction* junction = elem.junction;
		int quarterIndex = node->subElemIndex;

		if (!next)
			result = NextFromJunctionSidewalkToNothing(startPoint, junction);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = NextFromJunctionSidewalkToRoadSidewalk(startPoint, junction, quarterIndex, next->elem.road);
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
		else if (next->elem.type == MapElemJunction)
			result = EndFromBuildingToJunction(point, building, next->elem.junction);
	}
	else if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (!next) 
			; // NOTE: path should not end with a road
		else if (next->elem.type == MapElemBuilding)
			result = EndFromRoadToBuilding(point, road, next->elem.building);
		else if (next->elem.type == MapElemJunction)
			result = EndFromRoadToJunction(point, road, next->elem.junction);
	}
	else if (elem.type == MapElemJunction) {
		Junction* junction = elem.junction;

		if (!next)
			result = EndFromJunctionToNothing(point, junction);
		else if (next->elem.type == MapElemBuilding)
			result = EndFromJunctionToBuilding(point, junction, next->elem.building);
		else if (next->elem.type == MapElemRoad)
			result = EndFromJunctionToRoad(point, junction, next->elem.road);
	}
	else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;
		int laneIndex = node->subElemIndex;

		if (!next)
			; // NOTE: path should not end with a road
		else if (next->elem.type == MapElemJunctionSidewalk)
			result = EndFromRoadSidewalkToJunctionSidewalk(point, road, laneIndex, next->elem.junction);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = EndFromRoadSidewalkToRoadSidewalk(point, road, next->elem.road);
	}
	else if (elem.type == MapElemJunctionSidewalk) {
		Junction* junction = elem.junction;
		int quarterIndex = node->subElemIndex;

		if (!next)
			result = EndFromJunctionSidewalkToNothing(point, junction);
		else if (next->elem.type == MapElemRoadSidewalk)
			result = EndFromJunctionSidewalkToRoadSidewalk(point, junction, quarterIndex, next->elem.road);
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