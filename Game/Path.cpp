#include "Bezier.hpp"
#include "Geometry.hpp"
#include "Memory.hpp"
#include "Path.hpp"

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
	node.elem = GetRoadElem(road);
	node.next = 0;
	return node;
}

static inline PathNode JunctionNode(Junction* junction) {
	PathNode node = {};
	node.elem = GetJunctionElem(junction);
	node.next = 0;
	return node;
}

static inline PathNode BuildingNode(Building* building) {
	PathNode node = {};
	node.elem = GetBuildingElem(building);
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
	// TODO: Update this in a Building project!
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

static void ClearPathHelper(PathHelper* helper) 
{
	helper->nodeCount = 0;
}

static inline void ConnectRoadElemsHelper(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper) {
	ClearPathHelper(helper);
	if (elemStart.address == elemEnd.address) {
		helper->nodes[helper->nodeCount].elem = elemStart;
		helper->nodes[helper->nodeCount].next = 0;
		helper->sourceIndex[helper->nodeCount] = -1;
		helper->nodeCount++;
		return;
	}

	for (int i = 0; i < map->junctionN; ++i) 
		helper->isJunctionHelper[i] = 0;
	for (int i = 0; i < map->roadN; ++i)
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

	bool foundElemEnd = false;
	for (int i = 0; i < helper->nodeCount; ++i) {
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemRoad) {
			Road* road = elem.road;

			AddJunctionToHelper(map, road->junction1, i, helper);
			if (junctionEnd && road->junction1 == junctionEnd) {
				foundElemEnd = true;
				break;
			}

			AddJunctionToHelper(map, road->junction2, i, helper);
			if (junctionEnd && road->junction2 == junctionEnd) {
				foundElemEnd = true;
				break;
			}
		} else if (elem.type == MapElemJunction) {
			Junction* junction = elem.junction;
			bool roadEndFound = false;
			for (int j = 0; j < junction->roadN; ++j) {
				Road* road = junction->roads[j];
				AddRoadToHelper(map, road, i, helper);
				if (roadEnd && road == roadEnd) {
					foundElemEnd = true;
					roadEndFound = true;
					break;
				}
			}
			if (roadEndFound)
				break;
		}
	}

	if (!foundElemEnd)
		ClearPathHelper(helper);
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

static void PushJunctionSidewalk(Path* path, Junction* junction, int cornerIndex) {
	PathNode node = {};
	node.elem.type = MapElemJunctionSidewalk;
	node.elem.junction = junction;
	node.subElemIndex = cornerIndex;
	PushNode(path, node);
}

static void PushCrossRoad(Path* path, Road* road, Point startPoint, Junction* junction, int quarterIndex) {
	if (road) {
		int endLaneIndex = -LaneIndex(road, startPoint);
		PushJunctionSidewalk(path, junction, quarterIndex);
		PushRoadSidewalk(path, road, endLaneIndex);
	}
}

struct PathAroundJunction {
	Junction* junction;
	int startCornerIndex;
	int endCornerIndex;
	bool goClockwise;
};

static PathAroundJunction GetPathAroundJunction(MapElem elem, MapElem nextElem, int startCornerIndex, int endSubElemIndex)
{
	PathAroundJunction result = {};
	Assert(elem.type == MapElemJunctionSidewalk);
	Junction* junction = elem.junction;
	result.junction = junction;
	result.startCornerIndex = startCornerIndex;

	int cornerIndex = startCornerIndex;
	int leftTargetCornerIndex = 0;
	int rightTargetCornerIndex = 0;
	if (nextElem.type == MapElemNone) {
		Assert(IsValidJunctionCornerIndex(junction, endSubElemIndex));
		leftTargetCornerIndex = endSubElemIndex;
		rightTargetCornerIndex = endSubElemIndex;
	} else if (nextElem.type == MapElemRoadSidewalk) {
		Road* road = nextElem.road;
		leftTargetCornerIndex = GetRoadOutLeftJunctionCornerIndex(junction, road);
		rightTargetCornerIndex = GetRoadOutRightJunctionCornerIndex(junction, road);
	} else {
		InvalidCodePath;
	}

	int clockwiseDistance = GetClockwiseJunctionCornerIndexDistance(junction, cornerIndex, leftTargetCornerIndex);
	int counterClockwiseDistance = GetCounterClockwiseJunctionCornerIndexDistance(junction, cornerIndex, rightTargetCornerIndex);
	if (clockwiseDistance < counterClockwiseDistance) {
		result.goClockwise = true;
		result.endCornerIndex = leftTargetCornerIndex;
	} else {
		result.goClockwise = false;
		result.endCornerIndex = rightTargetCornerIndex;
	}
	return result;
}

static int GetPathAroundJunctionIndexDistance(PathAroundJunction pathAroundJunction)
{
	Junction* junction = pathAroundJunction.junction;
	int startCornerIndex = pathAroundJunction.startCornerIndex;
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	int endCornerIndex = pathAroundJunction.endCornerIndex;
	int indexDistance = 0;
	if (pathAroundJunction.goClockwise)
		indexDistance = GetClockwiseJunctionCornerIndexDistance(junction, startCornerIndex, endCornerIndex);
	else
		indexDistance = GetCounterClockwiseJunctionCornerIndexDistance(junction, startCornerIndex, endCornerIndex);
	return indexDistance;
}

static void PushPathAroundJunction(Path* path, PathAroundJunction pathAroundJunction)
{
	Junction* junction = pathAroundJunction.junction;
	int startCornerIndex = pathAroundJunction.startCornerIndex;
	int endCornerIndex = pathAroundJunction.endCornerIndex;
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));

	int cornerIndex = startCornerIndex;
	PushJunctionSidewalk(path, junction, cornerIndex);

	if (junction->roadN == 1) {
		if (cornerIndex != endCornerIndex)
			PushJunctionSidewalk(path, junction, endCornerIndex);
	} else if (junction->roadN >= 2) {
		if (pathAroundJunction.goClockwise) {
			while (cornerIndex != endCornerIndex) {
				int nextCornerIndex = GetNextJunctionCornerIndex(junction, cornerIndex);
				Road* roadToCross = junction->roads[cornerIndex];
				int startSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, cornerIndex);
				int endSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, nextCornerIndex);
				PushRoadSidewalk(path, roadToCross, startSidewalkIndex);
				PushRoadSidewalk(path, roadToCross, endSidewalkIndex);
				PushJunctionSidewalk(path, junction, nextCornerIndex);
				cornerIndex = nextCornerIndex;
			}
		} else {
			while (cornerIndex != endCornerIndex) {
				int previousCornerIndex = GetPreviousJunctionCornerIndex(junction, cornerIndex);
				Road* roadToCross = junction->roads[previousCornerIndex];
				int startSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, cornerIndex);
				int endSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, previousCornerIndex);
				PushRoadSidewalk(path, roadToCross, startSidewalkIndex);
				PushRoadSidewalk(path, roadToCross, endSidewalkIndex);
				PushJunctionSidewalk(path, junction, previousCornerIndex);
				cornerIndex = previousCornerIndex;
			}
		}
	} else {
		InvalidCodePath;
	}
}

static void PushConnectSidewalkElems(Path* path, Map* map, MapElem startElem, int startSubIndex,
									 MapElem endElem, int endSubIndex, PathHelper* helper) 
{
	MapElem startRoadElem = SidewalkElemToRoadElem(startElem);
	MapElem endRoadElem = SidewalkElemToRoadElem(endElem);

	// NOTE: the start and end elems are passed in a reverse order so that iteration will be easier
	// TODO: make ConnectRoadElemsHelper swap the elements by default?
	ConnectRoadElemsHelper(map, endRoadElem, startRoadElem, helper);

	int subIndex = startSubIndex;
	int nodeIndex = helper->nodeCount - 1;
	while (nodeIndex > -1) {
		PathNode roadNode = helper->nodes[nodeIndex];
		MapElem elem = RoadElemToSidewalkElem(roadNode.elem);

		int nextIndex = helper->sourceIndex[nodeIndex];
		PathNode nextRoadNode = {};
		MapElem nextElem = {};
		if (nextIndex > -1) {
			nextRoadNode = helper->nodes[nextIndex];
			nextElem = RoadElemToSidewalkElem(nextRoadNode.elem);
		}

		MapElem nextNextElem = {};
		if (nextIndex > -1) {
			int nextNextIndex = helper->sourceIndex[nextIndex];
			if (nextNextIndex > -1) {
				PathNode nextNextRoadNode = helper->nodes[nextNextIndex];
				nextNextElem = RoadElemToSidewalkElem(nextNextRoadNode.elem);
			}
		}

		if (elem.type == MapElemJunctionSidewalk) {
			Junction* junction = elem.junction;
			int cornerIndex = subIndex;
			PathAroundJunction pathAroundJunction = GetPathAroundJunction(elem, nextElem, cornerIndex, endSubIndex);
			PushPathAroundJunction(path, pathAroundJunction);

			if (nextElem.type == MapElemRoadSidewalk) {
				Road* road = nextElem.road;
				int leftRoadCornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
				int rightRoadCornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
				int endCornerIndex = pathAroundJunction.endCornerIndex;
				if (endCornerIndex == leftRoadCornerIndex)
					subIndex = LeftRoadSidewalkIndex;
				else if (endCornerIndex == rightRoadCornerIndex)
					subIndex = RightRoadSidewalkIndex;
				else
					InvalidCodePath;
			}
		} else if (elem.type == MapElemRoadSidewalk) {
			Road* road = elem.road;
			if (nextElem.type == MapElemJunctionSidewalk) {
				Junction* junction = nextElem.junction;

				int sidewalkIndexWithoutCrossing = subIndex;
				int sidewalkIndexWithCrossing = 0;
				if (sidewalkIndexWithoutCrossing == LeftRoadSidewalkIndex)
					sidewalkIndexWithCrossing = RightRoadSidewalkIndex;
				else if (sidewalkIndexWithoutCrossing == RightRoadSidewalkIndex)
					sidewalkIndexWithCrossing = LeftRoadSidewalkIndex;
				else
					InvalidCodePath;

				int cornerIndexWithoutCrossing = GetRoadSidewalkJunctionCornerIndex(junction, road, sidewalkIndexWithoutCrossing);
				int cornerIndexWithCrossing    = GetRoadSidewalkJunctionCornerIndex(junction, road, sidewalkIndexWithCrossing);

				PathAroundJunction pathWithoutCrossing = GetPathAroundJunction(nextElem, nextNextElem, cornerIndexWithoutCrossing, endSubIndex);
				PathAroundJunction pathWithCrossing    = GetPathAroundJunction(nextElem, nextNextElem, cornerIndexWithCrossing, endSubIndex);

				int distanceWithoutCrossing = GetPathAroundJunctionIndexDistance(pathWithoutCrossing);
				int distanceWithCrossing    = GetPathAroundJunctionIndexDistance(pathWithCrossing);
				
				if (distanceWithCrossing < distanceWithoutCrossing) {
					PushRoadSidewalk(path, road, sidewalkIndexWithoutCrossing);
					PushRoadSidewalk(path, road, sidewalkIndexWithCrossing);
					subIndex = cornerIndexWithCrossing;
				} else {
					PushRoadSidewalk(path, road, sidewalkIndexWithoutCrossing);
					subIndex = cornerIndexWithoutCrossing;
				}
			} else {
				InvalidCodePath;
			}
		} else {
			InvalidCodePath;
		}

		nodeIndex = nextIndex;
	}
}

Building* CommonAncestor(Building* building1, Building* building2)
{
	Building* building = 0;
	// TODO: Update this in a Building project!
	return building;
}

static void PushDownTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	// TODO: Update this in a Building project!
}

static void PushUpTheTree(Path* path, Building* buildingStart, Building* buildingEnd) {
	// TODO: Update this in a Building project!
}

MapElem GetConnectRoadElem(Building* building) {
	MapElem result = {};
	// TODO: Update this in a Building project!
	return result;
}

static PathNode* GetFreePathNode(PathPool* pathPool) {
	PathNode* result = 0;

	if (pathPool->firstFreeNode) {
		result = pathPool->firstFreeNode;
		pathPool->firstFreeNode = pathPool->firstFreeNode->next;
	} else if (pathPool->nodeCount < pathPool->maxNodeCount) {
		result = &pathPool->nodes[pathPool->nodeCount];
		pathPool->nodeCount++;
	} else {
		InvalidCodePath;
	}

	return result;
}

static PathNode* PushPathToPool(Path path, PathPool* pathPool) {
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

void ResetPathPool(PathPool* pathPool)
{
	pathPool->firstFreeNode = 0;
	pathPool->nodeCount = 0;
}

PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool) {
	PathNode* result = GetFreePathNode(pathPool);

	result->elem = elem;
	result->next = firstNode;

	return result;
}

static inline PathHelper PathHelperForMap(Map* map, MemArena* arena) {
	PathHelper helper = {};
	helper.nodes = ArenaPushArray(arena, PathNode, map->junctionN + map->roadN);
	helper.isJunctionHelper = ArenaPushArray(arena, int, map->junctionN);
	helper.isRoadHelper = ArenaPushArray(arena, int, map->roadN);
	helper.sourceIndex = ArenaPushArray(arena, int, map->junctionN + map->roadN);

	return helper;
}

// TODO: rename this to CarPath?
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* tmpArena, PathPool* pathPool) {
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
			} else if (commonAncestor == buildingStart) {
				PushUpTheTree(&path, buildingStart, buildingEnd);
			} else {
				PushDownTheTree(&path, buildingStart, commonAncestor);
				// NOTE: making sure commonAncestor does not get into the path twice
				path.nodeCount--;
				PushUpTheTree(&path, commonAncestor, buildingEnd);
			}

			finished = true;
		}
	}
	
	if (!finished) {
		for (int i = 0; i < map->roadN; ++i)
			helper.isRoadHelper[i] = 0;

		for (int i = 0; i < map->junctionN; ++i)
			helper.isJunctionHelper[i] = 0;

		MapElem roadElemStart = {};
		if (elemStart.type == MapElemBuilding) {
			Building* startBuilding = elemStart.building;

			PushFromBuildingToRoadElem(&path, startBuilding);
			roadElemStart = GetConnectRoadElem(startBuilding);

			if (roadElemStart.type == MapElemRoad) 
				startConnectRoad = roadElemStart.road;
		} else {
			roadElemStart = elemStart;
		}

		MapElem roadElemEnd = {};
		if (elemEnd.type == MapElemBuilding) {
			Building* endBuilding = elemEnd.building;

			roadElemEnd = GetConnectRoadElem(endBuilding);

			if (roadElemEnd.type == MapElemRoad) 
				endConnectRoad = roadElemEnd.road;
		} else {
			roadElemEnd = elemEnd;
		}

		if (startConnectRoad && endConnectRoad && startConnectRoad == endConnectRoad) {
			int startLaneIndex = LaneIndex(startConnectRoad, elemStart.building->connectPointFarShow);
			int endLaneIndex = LaneIndex(endConnectRoad, elemEnd.building->connectPointFarShow);

			if (startLaneIndex == endLaneIndex) {
				float startDistance = DistanceOnLane(startConnectRoad, startLaneIndex, elemStart.building->connectPointFarShow);
				float endDistance = DistanceOnLane(endConnectRoad, endLaneIndex, elemEnd.building->connectPointFarShow);

				if (startDistance < endDistance) {
					startConnectRoad = 0;
					endConnectRoad = 0;
				}
			}
		}

		if (startConnectRoad) {
			Building* startBuilding = elemStart.building;

			int laneIndex = LaneIndex(startConnectRoad, startBuilding->connectPointFarShow);

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

			int laneIndex = LaneIndex(endConnectRoad, endBuilding->connectPointFarShow);
				
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

	PathNode* firstNode = PushPathToPool(path, pathPool);

	ArenaPopTo(tmpArena, helper.nodes);

	return firstNode;
}

// TODO: remove arena from argument list?
// TODO: rename this to PedestrianPath?
PathNode* ConnectPedestrianElems(Map* map, MapElem startElem, int startSubIndex, MapElem endElem, int endSubIndex,
							   MemArena* arena, PathPool* pathPool) 
{
	Assert(startElem.type == MapElemJunctionSidewalk || startElem.type == MapElemRoadSidewalk || startElem.type == MapElemCrossing);
	Assert(endElem.type == MapElemJunctionSidewalk || endElem.type == MapElemRoadSidewalk || endElem.type == MapElemCrossing);

	PathHelper helper = PathHelperForMap(map, arena);
	// TODO: create a ResetPathHelper function?
	//       or move this to where the helper is actually used?
	for (int i = 0; i < map->roadN; ++i)
		helper.isRoadHelper[i] = 0;

	for (int i = 0; i < map->junctionN; ++i)
		helper.isJunctionHelper[i] = 0;

	Path path = {};
	path.arena = arena;
	path.nodes = ArenaPushArray(path.arena, PathNode, 0);

	MapElem startSidewalkElem = startElem;
	if (startElem.type == MapElemCrossing)
		startSidewalkElem = GetRoadSidewalkElem(startElem.road);

	MapElem endSidewalkElem = endElem;
	if (endElem.type == MapElemCrossing)
		endSidewalkElem = GetRoadSidewalkElem(endElem.road);

	PushConnectSidewalkElems(&path, map, startSidewalkElem, startSubIndex, endSidewalkElem, endSubIndex, &helper);
	PathNode* firstNode = PushPathToPool(path, pathPool);
	ArenaPopTo(arena, helper.nodes);

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
		} else {
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
		} else {
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
	} else if (startPoint.y == building->bottom && startPoint.x < building->right) {
		if (targetPoint.y == building->bottom && targetPoint.x > startPoint.x)
			return targetPoint;
		else
			return Point{building->right, building->bottom};
	} else if (startPoint.x == building->right && startPoint.y > building->top) {
		if (targetPoint.x == building->right && targetPoint.y < startPoint.y)
			return targetPoint;
		else
			return Point{building->right, building->top};
	} else if (startPoint.y == building->top && startPoint.x > building->left) {
		if (targetPoint.y == building->top && targetPoint.x < startPoint.x)
			return targetPoint;
		else
			return Point{building->left, building->top};
	} else {
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
	} else if (elem.type == MapElemJunction) {
		position = elem.junction->position;
	} else if (elem.type == MapElemJunctionSidewalk) {
		Junction* junction = elem.junction;
		int cornerIndex = node->subElemIndex;
		position = GetJunctionCorner(junction, cornerIndex);
	} else {
		InvalidCodePath;
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
	} else {
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

	// TODO: Update this in a Building project!

	return result;
}

bool EndFromBuildingToBuilding(DirectedPoint point, Building* building, Building* nextBuilding) {
	bool result = false;

	// TODO: Update this in a Building project!

	return result;
}

DirectedPoint NextFromBuildingToRoad(DirectedPoint startPoint, Building* building, Road* road) {
	DirectedPoint result = {};

	if (PointEqual(startPoint.position, building->connectPointClose)) {
		result.position = building->connectPointFarShow;
		result.direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
	} else {
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
	} else {
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

bool EndFromBuildingToJunction(DirectedPoint point, Building* building, Junction* junction) {
	bool result = PointEqual(point.position, building->connectPointFarShow);
	return result;
}

// TODO: Update this when buildings are added back!
DirectedPoint NextFromRoadToBuilding(DirectedPoint startPoint, Road* road, Building* building) {
	DirectedPoint result = {};
	return result;
}

// TODO: Update this when buildings are added back!
bool EndFromRoadToBuilding(DirectedPoint point, Road* road, Building* building) {
	bool result = true;
	return result;
}

DirectedPoint NextFromRoadToJunction(DirectedPoint startPoint, Road* road, Junction* junction) {
	DirectedPoint result = GetRoadLeavePoint(junction, road);
	return result;
}

bool EndFromRoadToJunction(DirectedPoint point, Road* road, Junction* junction) {
	bool result = false;

	DirectedPoint endPoint = {};
	endPoint = GetRoadLeavePoint(junction, road);

	result = PointEqual(point.position, endPoint.position);

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
	DirectedPoint result = GetRoadEnterPoint(junction, road);
	return result;
}

bool EndFromJunctionToRoad(DirectedPoint point, Junction* junction, Road* road) {
	DirectedPoint endPoint = GetRoadEnterPoint(junction, road);
	bool result = PointEqual(point.position, endPoint.position);
	return result;
}

static DirectedPoint NextFromRoadSidewalkToJunctionSidewalk(DirectedPoint startPoint, Road* road, Junction* junction, int junctionCornerIndex)
{
	DirectedPoint result = {};
	Point corner = GetJunctionCorner(junction, junctionCornerIndex);
	result.position = corner;
	return result;
}

static bool EndFromRoadSidewalkToJunctionSidewalk(DirectedPoint point, Road* road, Junction* junction, int junctionCornerIndex) 
{
	bool result = false;
	Point corner = GetJunctionCorner(junction, junctionCornerIndex);
	result = PointEqual(point.position, corner);
	return result;
}

static DirectedPoint NextFromRoadSidewalkToRoadSidewalk(DirectedPoint point, Road* road, Road* nextRoad, int sidewalkIndex)
{
	Assert(road == nextRoad);
	Assert(sidewalkIndex == LeftRoadSidewalkIndex || sidewalkIndex == RightRoadSidewalkIndex);
	DirectedPoint result = {};
	Point roadCoord = ToRoadCoord(road, point.position);
	bool isAtCrossing = false;
	if (Abs(roadCoord.x - road->crossingDistance) <= SidewalkWidth * 0.5f) {
		float along = roadCoord.x;
		float side = 0.0f;
		if (sidewalkIndex == LeftRoadSidewalkIndex)
			side = -(LaneWidth + SidewalkWidth * 0.5f);
		else if (sidewalkIndex == RightRoadSidewalkIndex)
			side = +(LaneWidth + SidewalkWidth * 0.5f);
		else
			InvalidCodePath;
		result.position = FromRoadCoord1(road, along, side);
	} else {
		float along = road->crossingDistance;
		float side = 0.0f;
		if (sidewalkIndex == LeftRoadSidewalkIndex)
			side = +(LaneWidth + SidewalkWidth * 0.5f);
		else if (sidewalkIndex == RightRoadSidewalkIndex)
			side = -(LaneWidth + SidewalkWidth * 0.5f);
		else
			InvalidCodePath;
		result.position = FromRoadCoord1(road, along, side);
	}
	return result;
}

static DirectedPoint NextFromRoadSidewalkToRoadSidewalk(DirectedPoint point, Road* road, Road* nextRoad) {
	DirectedPoint result = {};

	Point pointRoadCoord = ToRoadCoord(road, point.position);
	Point nextRoadCoord = ToRoadCoord(road, nextRoad->endPoint1);

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
	} else {
		if ((pointRoadCoord.x >= road->crossingDistance - CrossingWidth * 0.5f) 
			&& (pointRoadCoord.x <= road->crossingDistance + CrossingWidth * 0.5f))
			pointRoadCoord.y = nextRoadLaneIndex * (LaneWidth + SidewalkWidth * 0.5f); 
		else
			pointRoadCoord.x = road->crossingDistance;
	}

	result.position = FromRoadCoord(road, pointRoadCoord);
	return result;
}

static bool EndFromRoadSidewalkToRoadSidewalk(DirectedPoint point, Road* road, Road* nextRoad, int sidewalkIndex)
{
	bool result = false;
	Assert(road == nextRoad);
	Assert(sidewalkIndex == LeftRoadSidewalkIndex || sidewalkIndex == RightRoadSidewalkIndex);
	Point roadCoord = ToRoadCoord(road, point.position);
	float crossingDistance = road->crossingDistance;
	if (Abs(roadCoord.x - crossingDistance) <= CrossingWidth * 0.5f) {
		if (sidewalkIndex == LeftRoadSidewalkIndex && roadCoord.y < 0.0f)
			result = true;
		else if (sidewalkIndex == RightRoadSidewalkIndex && roadCoord.y > 0.0f)
			result = true;
	}
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

static DirectedPoint NextFromJunctionSidewalkToJunctionSidewalk(DirectedPoint startPoint, Junction* junction, Junction* nextJunction, int cornerIndex)
{
	DirectedPoint result = {};
	Assert(junction == nextJunction);
	result.position = GetJunctionCorner(junction, cornerIndex);
	return result;
}

static bool EndFromJunctionSidewalkToJunctionSidewalk(DirectedPoint point, Junction* junction, Junction* nextJunction, int cornerIndex)
{
	bool result = false;
	Assert(junction == nextJunction);
	Point corner = GetJunctionCorner(junction, cornerIndex);
	result = PointEqual(point.position, corner);
	return result;
}

// TODO: make this work with indexes instead of floating-point comparison
DirectedPoint NextNodePoint(PathNode* node, DirectedPoint startPoint) {
	DirectedPoint result = {};

	MapElem elem = node->elem;
	PathNode* next = node->next;
	MapElem nextElem = {};
	if (next)
		nextElem = next->elem;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;
		if (nextElem.type == MapElemNone)
			result = NextFromBuildingToNothing(startPoint, building);
		else if (nextElem.type == MapElemBuilding)
			result = NextFromBuildingToBuilding(startPoint, building, nextElem.building);
		else if (nextElem.type == MapElemRoad)
			result = NextFromBuildingToRoad(startPoint, building, nextElem.road);
		else if (nextElem.type == MapElemJunction)
			result = NextFromBuildingToJunction(startPoint, building, nextElem.junction);
		else
			InvalidCodePath;
	} else if (elem.type == MapElemRoad) {
		Road* road = elem.road;
		if (nextElem.type == MapElemBuilding)
			result = NextFromRoadToBuilding(startPoint, road, nextElem.building);
		else if (nextElem.type == MapElemJunction)
			result = NextFromRoadToJunction(startPoint, road, nextElem.junction);
		else
			InvalidCodePath;
	} else if (elem.type == MapElemJunction) {
		Junction* junction = elem.junction;
		if (nextElem.type == MapElemNone)
			result = NextFromJunctionToNothing(startPoint, junction);
		else if (nextElem.type == MapElemBuilding)
			result = NextFromJunctionToBuilding(startPoint, junction, nextElem.building);
		else if (nextElem.type == MapElemRoad)
			result = NextFromJunctionToRoad(startPoint, junction, nextElem.road);
		else
			InvalidCodePath;
	} else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;
		if (nextElem.type == MapElemJunctionSidewalk)
			result = NextFromRoadSidewalkToJunctionSidewalk(startPoint, road, nextElem.junction, next->subElemIndex);
		else if (nextElem.type == MapElemRoadSidewalk)
			result = NextFromRoadSidewalkToRoadSidewalk(startPoint, road, nextElem.road, next->subElemIndex);
		else
			InvalidCodePath;
	} else if (elem.type == MapElemJunctionSidewalk) {
		Junction* junction = elem.junction;
		if (nextElem.type == MapElemRoadSidewalk)
			result = startPoint;
		else if (nextElem.type == MapElemJunctionSidewalk)
			result = NextFromJunctionSidewalkToJunctionSidewalk(startPoint, junction, nextElem.junction, next->subElemIndex);
	} else {
		InvalidCodePath;
	}

	return result;
}

bool IsNodeEndPoint(PathNode* node, DirectedPoint point) {
	bool result = false;

	MapElem elem = node->elem;
	PathNode* next = node->next;
	MapElem nextElem = {};
	if (next)
		nextElem = next->elem;

	if (elem.type == MapElemBuilding) {
		Building* building = elem.building;
		if (nextElem.type == MapElemNone)
			result = EndFromBuildingToNothing(point, building);
		else if (nextElem.type == MapElemBuilding)
			result = EndFromBuildingToBuilding(point, building, nextElem.building);
		else if (nextElem.type == MapElemRoad)
			result = EndFromBuildingToRoad(point, building, nextElem.road);
		else if (nextElem.type == MapElemJunction)
			result = EndFromBuildingToJunction(point, building, nextElem.junction);
	} else if (elem.type == MapElemRoad) {
		Road* road = elem.road;
		if (nextElem.type == MapElemNone) 
			InvalidCodePath;
		else if (nextElem.type == MapElemBuilding)
			result = EndFromRoadToBuilding(point, road, nextElem.building);
		else if (nextElem.type == MapElemJunction)
			result = EndFromRoadToJunction(point, road, nextElem.junction);
	} else if (elem.type == MapElemJunction) {
		Junction* junction = elem.junction;
		if (nextElem.type == MapElemNone)
			result = EndFromJunctionToNothing(point, junction);
		else if (nextElem.type == MapElemBuilding)
			result = EndFromJunctionToBuilding(point, junction, nextElem.building);
		else if (nextElem.type == MapElemRoad)
			result = EndFromJunctionToRoad(point, junction, nextElem.road);
	} else if (elem.type == MapElemRoadSidewalk) {
		Road* road = elem.road;
		if (nextElem.type == MapElemJunctionSidewalk)
			result = EndFromRoadSidewalkToJunctionSidewalk(point, road, nextElem.junction, next->subElemIndex);
		else if (nextElem.type == MapElemRoadSidewalk)
			result = EndFromRoadSidewalkToRoadSidewalk(point, road, nextElem.road, next->subElemIndex);
		else
			InvalidCodePath;
	} else if (elem.type == MapElemJunctionSidewalk) {
		Junction* junction = elem.junction;
		if (nextElem.type == MapElemNone)
			result = true;
		else if (nextElem.type == MapElemRoadSidewalk)
			result = true;
		else if (nextElem.type == MapElemJunctionSidewalk)
			result = EndFromJunctionSidewalkToJunctionSidewalk(point, junction, nextElem.junction, next->subElemIndex);
		else
			InvalidCodePath;
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
			} else {
				DirectedPoint nextPoint = NextNodePoint(node, point);

				DrawLine(renderer, point.position, nextPoint.position, color, lineWidth);

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
			} else {
				DirectedPoint nextPoint = NextNodePoint(node, point);
				Assert(nextPoint.position.x != 0.0f || nextPoint.position.y != 0.0f);
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