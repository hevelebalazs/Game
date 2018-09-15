#pragma once

#include "Bezier.hpp"
#include "Building.hpp"
#include "Draw.hpp"
#include "Map.hpp"
#include "Memory.hpp"
#include "Road.hpp"
#include "Type.hpp"

struct PathNode 
{
	// TODO: create a MapSubElem struct?
	MapElem elem;
	I32 subElemIndex;

	PathNode* next;
};

struct PathPool 
{
	PathNode* nodes;
	I32 nodeCount;
	I32 maxNodeCount;
	PathNode* firstFreeNode;
};


struct Path 
{
	I32 nodeCount;
	PathNode* nodes;
	MemArena* arena;
};

static void PushNode(Path* path, PathNode node)
{
	ArenaPushType(path->arena, PathNode);
	path->nodes[path->nodeCount] = node;
	path->nodeCount++;
}

static PathNode ElemNode(MapElem elem)
{
	PathNode node = {};
	node.elem = elem;
	return node;
}

static void PushElem(Path* path, MapElem elem)
{
	PathNode node = ElemNode(elem);
	PushNode(path, node);
}

static PathNode RoadNode(Road* road)
{
	PathNode node = {};
	node.elem = GetRoadElem(road);
	node.next = 0;
	return node;
}

static PathNode JunctionNode(Junction* junction)
{
	PathNode node = {};
	node.elem = GetJunctionElem(junction);
	node.next = 0;
	return node;
}

static PathNode BuildingNode(Building* building)
{
	PathNode node = {};
	node.elem = GetBuildingElem(building);
	node.next = 0;
	return node;
}

static void PushRoad(Path* path, Road* road)
{
	PathNode node = RoadNode(road);
	PushNode(path, node);
}

static void PushJunction(Path* path, Junction* junction)
{
	PathNode node = JunctionNode(junction);
	PushNode(path, node);
}

static void PushBuilding(Path* path, Building* building)
{
	PathNode node = BuildingNode(building);
	PushNode(path, node);
}

static void InvertSegment(Path* path, I32 startIndex, I32 endIndex)
{
	while (startIndex < endIndex) 
	{
		PathNode tmpNode = path->nodes[startIndex];
		path->nodes[startIndex] = path->nodes[endIndex];
		path->nodes[endIndex] = tmpNode;

		++startIndex;
		--endIndex;
	}
}

struct PathHelper 
{
	PathNode* nodes;
	I32 nodeCount;

	I32* isJunctionHelper;
	I32* isRoadHelper;
	I32* sourceIndex;
};

static void PushFromBuildingToRoadElem(Path* path, Building* building)
{
	// TODO: Update this in a Building project!
}

static void PushFromRoadElemToBuilding(Path* path, Building *building)
{
	I32 startIndex = path->nodeCount;
	PushFromBuildingToRoadElem(path, building);

	I32 endIndex = path->nodeCount - 1;
	InvertSegment(path, startIndex, endIndex);
}

static void AddRoadToHelper(Map* map, Road* road, I32 sourceIndex, PathHelper* pathHelper)
{
	I32 roadIndex = (I32)(road - map->roads);

	if (pathHelper->isRoadHelper[roadIndex] == 0) 
	{
		pathHelper->isRoadHelper[roadIndex] = 1;

		pathHelper->nodes[pathHelper->nodeCount] = RoadNode(road);
		pathHelper->sourceIndex[pathHelper->nodeCount] = sourceIndex;
		pathHelper->nodeCount++;
	}
}

static void AddJunctionToHelper(Map* map, Junction* junction, I32 sourceIndex, PathHelper* pathHelper)
{
	I32 junctionIndex = (I32)(junction - map->junctions);

	if (pathHelper->isJunctionHelper[junctionIndex] == 0) 
	{
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

static void ConnectRoadElemsHelper(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper)
{
	ClearPathHelper(helper);
	if (elemStart.address == elemEnd.address) 
	{
		helper->nodes[helper->nodeCount].elem = elemStart;
		helper->nodes[helper->nodeCount].next = 0;
		helper->sourceIndex[helper->nodeCount] = -1;
		helper->nodeCount++;
		return;
	}

	for (I32 i = 0; i < map->junctionN; ++i) 
		helper->isJunctionHelper[i] = 0;
	for (I32 i = 0; i < map->roadN; ++i)
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

	B32 foundElemEnd = false;
	for (I32 i = 0; i < helper->nodeCount; ++i) 
	{
		PathNode node = helper->nodes[i];
		MapElem elem = node.elem;

		if (elem.type == MapElemRoad) 
		{
			Road* road = elem.road;

			AddJunctionToHelper(map, road->junction1, i, helper);
			if (junctionEnd && road->junction1 == junctionEnd) 
			{
				foundElemEnd = true;
				break;
			}

			AddJunctionToHelper(map, road->junction2, i, helper);
			if (junctionEnd && road->junction2 == junctionEnd) 
			{
				foundElemEnd = true;
				break;
			}
		} 
		else if (elem.type == MapElemJunction) 
		{
			Junction* junction = elem.junction;
			B32 roadEndFound = false;
			for (I32 j = 0; j < junction->roadN; ++j) 
			{
				Road* road = junction->roads[j];
				AddRoadToHelper(map, road, i, helper);
				if (roadEnd && road == roadEnd) 
				{
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

static void PushConnectRoadElems(Path* path, Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper)
{
	ConnectRoadElemsHelper(map, elemStart, elemEnd, helper);

	I32 startIndex = path->nodeCount;
	I32 nodeIndex = helper->nodeCount - 1;
	while (nodeIndex > -1) 
	{
		PathNode node = helper->nodes[nodeIndex];
		PushNode(path, node);
		nodeIndex = helper->sourceIndex[nodeIndex];
	}

	I32 endIndex = path->nodeCount - 1;

	InvertSegment(path, startIndex, endIndex);
}

static MapElem SidewalkElemToRoadElem(MapElem sidewalkElem)
{
	MapElem roadElem = sidewalkElem;

	if (sidewalkElem.type == MapElemRoadSidewalk)
		roadElem.type = MapElemRoad;
	else if (sidewalkElem.type == MapElemJunctionSidewalk)
		roadElem.type = MapElemJunction;

	return roadElem;
}

static MapElem RoadElemToSidewalkElem(MapElem roadElem)
{
	MapElem sidewalkElem = roadElem;

	if (roadElem.type == MapElemRoad)
		sidewalkElem.type = MapElemRoadSidewalk;
	else if (roadElem.type == MapElemJunction)
		sidewalkElem.type = MapElemJunctionSidewalk;

	return sidewalkElem;
}

// TODO: create a LaneIndex enum
static void PushRoadSidewalk(Path* path, Road* road, I32 laneIndex)
{
	PathNode node = {};
	node.elem.type = MapElemRoadSidewalk;
	node.elem.road = road;
	node.subElemIndex = laneIndex;
	PushNode(path, node);
}

static void PushJunctionSidewalk(Path* path, Junction* junction, I32 cornerIndex)
{
	PathNode node = {};
	node.elem.type = MapElemJunctionSidewalk;
	node.elem.junction = junction;
	node.subElemIndex = cornerIndex;
	PushNode(path, node);
}

static void PushCrossRoad(Path* path, Road* road, V2 startPoint, Junction* junction, I32 quarterIndex)
{
	if (road) 
	{
		I32 endLaneIndex = -LaneIndex(road, startPoint);
		PushJunctionSidewalk(path, junction, quarterIndex);
		PushRoadSidewalk(path, road, endLaneIndex);
	}
}

struct PathAroundJunction 
{
	Junction* junction;
	I32 startCornerIndex;
	I32 endCornerIndex;
	B32 goClockwise;
};

static PathAroundJunction GetPathAroundJunction(MapElem elem, MapElem nextElem, I32 startCornerIndex, I32 endSubElemIndex)
{
	PathAroundJunction result = {};
	Assert(elem.type == MapElemJunctionSidewalk);
	Junction* junction = elem.junction;
	result.junction = junction;
	result.startCornerIndex = startCornerIndex;

	I32 cornerIndex = startCornerIndex;
	I32 leftTargetCornerIndex = 0;
	I32 rightTargetCornerIndex = 0;
	if (nextElem.type == MapElemNone) 
	{
		Assert(IsValidJunctionCornerIndex(junction, endSubElemIndex));
		leftTargetCornerIndex = endSubElemIndex;
		rightTargetCornerIndex = endSubElemIndex;
	} 
	else if (nextElem.type == MapElemRoadSidewalk) 
	{
		Road* road = nextElem.road;
		leftTargetCornerIndex = GetRoadOutLeftJunctionCornerIndex(junction, road);
		rightTargetCornerIndex = GetRoadOutRightJunctionCornerIndex(junction, road);
	} 
	else 
	{
		InvalidCodePath;
	}

	I32 clockwiseDistance = GetClockwiseJunctionCornerIndexDistance(junction, cornerIndex, leftTargetCornerIndex);
	I32 counterClockwiseDistance = GetCounterClockwiseJunctionCornerIndexDistance(junction, cornerIndex, rightTargetCornerIndex);
	if (clockwiseDistance < counterClockwiseDistance) 
	{
		result.goClockwise = true;
		result.endCornerIndex = leftTargetCornerIndex;
	}
	else 
	{
		result.goClockwise = false;
		result.endCornerIndex = rightTargetCornerIndex;
	}
	return result;
}

static I32 GetPathAroundJunctionIndexDistance(PathAroundJunction pathAroundJunction)
{
	Junction* junction = pathAroundJunction.junction;
	I32 startCornerIndex = pathAroundJunction.startCornerIndex;
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	I32 endCornerIndex = pathAroundJunction.endCornerIndex;
	I32 indexDistance = 0;
	if (pathAroundJunction.goClockwise)
		indexDistance = GetClockwiseJunctionCornerIndexDistance(junction, startCornerIndex, endCornerIndex);
	else
		indexDistance = GetCounterClockwiseJunctionCornerIndexDistance(junction, startCornerIndex, endCornerIndex);
	return indexDistance;
}

static void PushPathAroundJunction(Path* path, PathAroundJunction pathAroundJunction)
{
	Junction* junction = pathAroundJunction.junction;
	I32 startCornerIndex = pathAroundJunction.startCornerIndex;
	I32 endCornerIndex = pathAroundJunction.endCornerIndex;
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));

	I32 cornerIndex = startCornerIndex;
	PushJunctionSidewalk(path, junction, cornerIndex);

	if (junction->roadN == 1) 
	{
		if (cornerIndex != endCornerIndex)
			PushJunctionSidewalk(path, junction, endCornerIndex);
	} 
	else if (junction->roadN >= 2) 
	{
		if (pathAroundJunction.goClockwise) 
		{
			while (cornerIndex != endCornerIndex) 
			{
				I32 nextCornerIndex = GetNextJunctionCornerIndex(junction, cornerIndex);
				Road* roadToCross = junction->roads[cornerIndex];
				I32 startSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, cornerIndex);
				I32 endSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, nextCornerIndex);
				PushRoadSidewalk(path, roadToCross, startSidewalkIndex);
				PushRoadSidewalk(path, roadToCross, endSidewalkIndex);
				PushJunctionSidewalk(path, junction, nextCornerIndex);
				cornerIndex = nextCornerIndex;
			}
		} 
		else 
		{
			while (cornerIndex != endCornerIndex) 
			{
				I32 previousCornerIndex = GetPreviousJunctionCornerIndex(junction, cornerIndex);
				Road* roadToCross = junction->roads[previousCornerIndex];
				I32 startSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, cornerIndex);
				I32 endSidewalkIndex = GetRoadJunctionCornerSidewalkIndex(junction, roadToCross, previousCornerIndex);
				PushRoadSidewalk(path, roadToCross, startSidewalkIndex);
				PushRoadSidewalk(path, roadToCross, endSidewalkIndex);
				PushJunctionSidewalk(path, junction, previousCornerIndex);
				cornerIndex = previousCornerIndex;
			}
		}
	} 
	else 
	{
		InvalidCodePath;
	}
}

static void PushConnectSidewalkElems(Path* path, Map* map, MapElem startElem, I32 startSubIndex,
									 MapElem endElem, I32 endSubIndex, PathHelper* helper) 
{
	MapElem startRoadElem = SidewalkElemToRoadElem(startElem);
	MapElem endRoadElem = SidewalkElemToRoadElem(endElem);

	// NOTE: the start and end elems are passed in a reverse order so that iteration will be easier
	// TODO: make ConnectRoadElemsHelper swap the elements by default?
	ConnectRoadElemsHelper(map, endRoadElem, startRoadElem, helper);

	I32 subIndex = startSubIndex;
	I32 nodeIndex = helper->nodeCount - 1;
	while (nodeIndex > -1) 
	{
		PathNode roadNode = helper->nodes[nodeIndex];
		MapElem elem = RoadElemToSidewalkElem(roadNode.elem);

		I32 nextIndex = helper->sourceIndex[nodeIndex];
		PathNode nextRoadNode = {};
		MapElem nextElem = {};
		if (nextIndex > -1) 
		{
			nextRoadNode = helper->nodes[nextIndex];
			nextElem = RoadElemToSidewalkElem(nextRoadNode.elem);
		}

		MapElem nextNextElem = {};
		if (nextIndex > -1) 
		{
			I32 nextNextIndex = helper->sourceIndex[nextIndex];
			if (nextNextIndex > -1) 
			{
				PathNode nextNextRoadNode = helper->nodes[nextNextIndex];
				nextNextElem = RoadElemToSidewalkElem(nextNextRoadNode.elem);
			}
		}

		if (elem.type == MapElemJunctionSidewalk) 
		{
			Junction* junction = elem.junction;
			I32 cornerIndex = subIndex;
			PathAroundJunction pathAroundJunction = GetPathAroundJunction(elem, nextElem, cornerIndex, endSubIndex);
			PushPathAroundJunction(path, pathAroundJunction);

			if (nextElem.type == MapElemRoadSidewalk) 
			{
				Road* road = nextElem.road;
				I32 leftRoadCornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
				I32 rightRoadCornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
				I32 endCornerIndex = pathAroundJunction.endCornerIndex;
				if (endCornerIndex == leftRoadCornerIndex)
					subIndex = LeftRoadSidewalkIndex;
				else if (endCornerIndex == rightRoadCornerIndex)
					subIndex = RightRoadSidewalkIndex;
				else
					InvalidCodePath;
			}
		} 
		else if (elem.type == MapElemRoadSidewalk) 
		{
			Road* road = elem.road;
			if (nextElem.type == MapElemJunctionSidewalk) 
			{
				Junction* junction = nextElem.junction;

				I32 sidewalkIndexWithoutCrossing = subIndex;
				I32 sidewalkIndexWithCrossing = 0;
				if (sidewalkIndexWithoutCrossing == LeftRoadSidewalkIndex)
					sidewalkIndexWithCrossing = RightRoadSidewalkIndex;
				else if (sidewalkIndexWithoutCrossing == RightRoadSidewalkIndex)
					sidewalkIndexWithCrossing = LeftRoadSidewalkIndex;
				else
					InvalidCodePath;

				I32 cornerIndexWithoutCrossing = GetRoadSidewalkJunctionCornerIndex(junction, road, sidewalkIndexWithoutCrossing);
				I32 cornerIndexWithCrossing    = GetRoadSidewalkJunctionCornerIndex(junction, road, sidewalkIndexWithCrossing);

				PathAroundJunction pathWithoutCrossing = GetPathAroundJunction(nextElem, nextNextElem, cornerIndexWithoutCrossing, endSubIndex);
				PathAroundJunction pathWithCrossing    = GetPathAroundJunction(nextElem, nextNextElem, cornerIndexWithCrossing, endSubIndex);

				I32 distanceWithoutCrossing = GetPathAroundJunctionIndexDistance(pathWithoutCrossing);
				I32 distanceWithCrossing    = GetPathAroundJunctionIndexDistance(pathWithCrossing);
				
				if (distanceWithCrossing < distanceWithoutCrossing) 
				{
					PushRoadSidewalk(path, road, sidewalkIndexWithoutCrossing);
					PushRoadSidewalk(path, road, sidewalkIndexWithCrossing);
					subIndex = cornerIndexWithCrossing;
				} 
				else 
				{
					PushRoadSidewalk(path, road, sidewalkIndexWithoutCrossing);
					subIndex = cornerIndexWithoutCrossing;
				}
			} 
			else 
			{
				InvalidCodePath;
			}
		} 
		else 
		{
			InvalidCodePath;
		}

		nodeIndex = nextIndex;
	}
}

static Building* CommonAncestor(Building* building1, Building* building2)
{
	Building* building = 0;
	// TODO: Update this in a Building project!
	return building;
}

static void PushDownTheTree(Path* path, Building* buildingStart, Building* buildingEnd)
{
	// TODO: Update this in a Building project!
}

static void PushUpTheTree(Path* path, Building* buildingStart, Building* buildingEnd)
{
	// TODO: Update this in a Building project!
}

static MapElem GetConnectRoadElem(Building* building)
{
	MapElem result = {};
	// TODO: Update this in a Building project!
	return result;
}

static PathNode* GetFreePathNode(PathPool* pathPool)
{
	PathNode* result = 0;

	if (pathPool->firstFreeNode) 
	{
		result = pathPool->firstFreeNode;
		pathPool->firstFreeNode = pathPool->firstFreeNode->next;
	} 
	else if (pathPool->nodeCount < pathPool->maxNodeCount) 
	{
		result = &pathPool->nodes[pathPool->nodeCount];
		pathPool->nodeCount++;
	} 
	else 
	{
		InvalidCodePath;
	}

	return result;
}

static PathNode* PushPathToPool(Path path, PathPool* pathPool)
{
	PathNode* result = 0;
	PathNode* previous = 0;

	for (I32 i = 0; i < path.nodeCount; ++i) 
	{
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

static void ResetPathPool(PathPool* pathPool)
{
	pathPool->firstFreeNode = 0;
	pathPool->nodeCount = 0;
}

static PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool)
{
	PathNode* result = GetFreePathNode(pathPool);

	result->elem = elem;
	result->next = firstNode;

	return result;
}

static PathHelper PathHelperForMap(Map* map, MemArena* arena)
{
	PathHelper helper = {};
	helper.nodes = ArenaPushArray(arena, PathNode, map->junctionN + map->roadN);
	helper.isJunctionHelper = ArenaPushArray(arena, I32, map->junctionN);
	helper.isRoadHelper = ArenaPushArray(arena, I32, map->roadN);
	helper.sourceIndex = ArenaPushArray(arena, I32, map->junctionN + map->roadN);

	return helper;
}

// TODO: rename this to CarPath?
static PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* tmpArena, PathPool* pathPool)
{
	if (elemStart.type == MapElemRoadSidewalk || elemStart.type == MapElemJunctionSidewalk) 
		return 0;
	if (elemEnd.type == MapElemRoadSidewalk || elemEnd.type == MapElemJunctionSidewalk)
		return 0;

	PathHelper helper = PathHelperForMap(map, tmpArena);

	Path path = {};
	path.arena = tmpArena;
	path.nodes = ArenaPushArray(path.arena, PathNode, 0);

	B32 finished = false;

	if (elemStart.address == elemEnd.address) 
	{
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

		if (commonAncestor) 
		{
			if (commonAncestor == buildingEnd) 
			{
				PushDownTheTree(&path, buildingStart, buildingEnd);
			} 
			else if (commonAncestor == buildingStart) 
			{
				PushUpTheTree(&path, buildingStart, buildingEnd);
			} 
			else 
			{
				PushDownTheTree(&path, buildingStart, commonAncestor);
				// NOTE: making sure commonAncestor does not get into the path twice
				path.nodeCount--;
				PushUpTheTree(&path, commonAncestor, buildingEnd);
			}

			finished = true;
		}
	}
	
	if (!finished) 
	{
		for (I32 i = 0; i < map->roadN; ++i)
			helper.isRoadHelper[i] = 0;

		for (I32 i = 0; i < map->junctionN; ++i)
			helper.isJunctionHelper[i] = 0;

		MapElem roadElemStart = {};
		if (elemStart.type == MapElemBuilding) 
		{
			Building* startBuilding = elemStart.building;

			PushFromBuildingToRoadElem(&path, startBuilding);
			roadElemStart = GetConnectRoadElem(startBuilding);

			if (roadElemStart.type == MapElemRoad) 
				startConnectRoad = roadElemStart.road;
		} 
		else 
		{
			roadElemStart = elemStart;
		}

		MapElem roadElemEnd = {};
		if (elemEnd.type == MapElemBuilding) 
		{
			Building* endBuilding = elemEnd.building;

			roadElemEnd = GetConnectRoadElem(endBuilding);

			if (roadElemEnd.type == MapElemRoad) 
				endConnectRoad = roadElemEnd.road;
		} 
		else 
		{
			roadElemEnd = elemEnd;
		}

		if (startConnectRoad && endConnectRoad && startConnectRoad == endConnectRoad) 
		{
			I32 startLaneIndex = LaneIndex(startConnectRoad, elemStart.building->connectPointFarShow);
			I32 endLaneIndex = LaneIndex(endConnectRoad, elemEnd.building->connectPointFarShow);

			if (startLaneIndex == endLaneIndex) 
			{
				F32 startDistance = DistanceOnLane(startConnectRoad, startLaneIndex, elemStart.building->connectPointFarShow);
				F32 endDistance = DistanceOnLane(endConnectRoad, endLaneIndex, elemEnd.building->connectPointFarShow);

				if (startDistance < endDistance) 
				{
					startConnectRoad = 0;
					endConnectRoad = 0;
				}
			}
		}

		if (startConnectRoad) 
		{
			Building* startBuilding = elemStart.building;

			I32 laneIndex = LaneIndex(startConnectRoad, startBuilding->connectPointFarShow);

			Junction* startJunction = 0;
			if (laneIndex == 1)
				startJunction = startConnectRoad->junction2;
			else if (laneIndex == -1)
				startJunction = startConnectRoad->junction1;

			if (startJunction) 
			{
				startConnectRoad = startConnectRoad;
				roadElemStart.type = MapElemJunction;
				roadElemStart.junction = startJunction;
			}
		}

		if (endConnectRoad) 
		{
			Building* endBuilding = elemEnd.building;

			I32 laneIndex = LaneIndex(endConnectRoad, endBuilding->connectPointFarShow);
				
			Junction* endJunction = 0;
			if (laneIndex == 1)
				endJunction = endConnectRoad->junction1;
			else if (laneIndex == -1)
				endJunction = endConnectRoad->junction2;

			if (endJunction) 
			{
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
static PathNode* ConnectPedestrianElems(Map* map, MapElem startElem, I32 startSubIndex, MapElem endElem, I32 endSubIndex,
							   MemArena* arena, PathPool* pathPool) 
{
	Assert(startElem.type == MapElemJunctionSidewalk || startElem.type == MapElemRoadSidewalk || startElem.type == MapElemCrossing);
	Assert(endElem.type == MapElemJunctionSidewalk || endElem.type == MapElemRoadSidewalk || endElem.type == MapElemCrossing);

	PathHelper helper = PathHelperForMap(map, arena);
	// TODO: create a ResetPathHelper function?
	//       or move this to where the helper is actually used?
	for (I32 i = 0; i < map->roadN; ++i)
		helper.isRoadHelper[i] = 0;

	for (I32 i = 0; i < map->junctionN; ++i)
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

static void FreePathNode(PathNode* node, PathPool* pathPool)
{
	if (node) 
	{
		node->next = pathPool->firstFreeNode;
		pathPool->firstFreeNode = node;
	}
}

static void FreePath(PathNode* firstNode, PathPool* pathPool)
{
	if (firstNode) 
	{
		PathNode* lastNode = firstNode;

		while (lastNode && lastNode->next)
			lastNode = lastNode->next;

		lastNode->next = pathPool->firstFreeNode;
		pathPool->firstFreeNode = firstNode;
	}
}

static V2 NextPointAroundBuilding(Building* building, V2 startPoint, V2 targetPoint)
{
	V2 result = {};
	if (startPoint.x == building->left && startPoint.y < building->bottom) 
	{
		if (targetPoint.x == building->left && targetPoint.y > startPoint.y)
			result = targetPoint;
		else
			result = MakePoint(building->left, building->bottom);
	} 
	else if (startPoint.y == building->bottom && startPoint.x < building->right) 
	{
		if (targetPoint.y == building->bottom && targetPoint.x > startPoint.x)
			result = targetPoint;
		else
			result = MakePoint(building->right, building->bottom);
	} 
	else if (startPoint.x == building->right && startPoint.y > building->top) 
	{
		if (targetPoint.x == building->right && targetPoint.y < startPoint.y)
			result = targetPoint;
		else
			result = MakePoint(building->right, building->top);
	} 
	else if (startPoint.y == building->top && startPoint.x > building->left) 
	{
		if (targetPoint.y == building->top && targetPoint.x < startPoint.x)
			result = targetPoint;
		else
			result = MakePoint(building->left, building->top);
	} 
	else 
	{
		result = targetPoint;
	}

	return result;
}

static V4 StartNodePoint(PathNode* node)
{
	V2 position = {};
	V2 direction = {};

	MapElem elem = node->elem;
	if (elem.type == MapElemBuilding) 
	{
		Building* building = elem.building;

		position = building->connectPointClose;
		direction = PointDirection(building->connectPointClose, building->connectPointFar);
	} 
	else if (elem.type == MapElemJunction) 
	{
		position = elem.junction->position;
	} 
	else if (elem.type == MapElemJunctionSidewalk) 
	{
		Junction* junction = elem.junction;
		I32 cornerIndex = node->subElemIndex;
		position = GetJunctionCorner(junction, cornerIndex);
	} 
	else 
	{
		InvalidCodePath;
	}

	V4 result = {};
	result.position = position;
	result.direction = direction;
	return result;
}

static V4 NextFromBuildingToNothing(V4 startPoint, Building* building)
{
	V4 result = {};

	if (startPoint.position == building->connectPointFar || startPoint.position == building->connectPointFarShow) 
	{
		result.position = building->connectPointClose;
		result.direction = PointDirection(building->connectPointFar, building->connectPointClose);
	}
	else 
	{
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

static B32 EndFromBuildingToNothing(V4 point, Building* building)
{
	B32 result = (point.position == building->connectPointClose);
	return result;
}

static V4 NextFromBuildingToBuilding(V4 startPoint, Building* building, Building* nextBuilding)
{
	V4 result = {};

	// TODO: Update this in a Building project!

	return result;
}

static B32 EndFromBuildingToBuilding(V4 point, Building* building, Building* nextBuilding)
{
	B32 result = false;

	// TODO: Update this in a Building project!

	return result;
}

static V4 NextFromBuildingToRoad(V4 startPoint, Building* building, Road* road)
{
	V4 result = {};

	if (startPoint.position == building->connectPointClose) 
	{
		result.position = building->connectPointFarShow;
		result.direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
	} 
	else 
	{
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

static B32 EndFromBuildingToRoad(V4 point, Building* building, Road* road)
{
	B32 result = (point.position == building->connectPointFarShow);
	return result;
}

static V4 NextFromBuildingToJunction(V4 startPoint, Building* building, Junction* junction)
{
	V4 result = {};

	if (startPoint.position == building->connectPointClose) 
	{
		result.position = building->connectPointFarShow;
		result.direction = PointDirection(building->connectPointClose, building->connectPointFarShow);
	} 
	else 
	{
		// TODO: add direction
		result.position = NextPointAroundBuilding(building, startPoint.position, building->connectPointClose);
	}

	return result;
}

static B32 EndFromBuildingToJunction(V4 point, Building* building, Junction* junction)
{
	B32 result = (point.position == building->connectPointFarShow);
	return result;
}

// TODO: Update this when buildings are added back!
static V4 NextFromRoadToBuilding(V4 startPoint, Road* road, Building* building)
{
	V4 result = {};
	return result;
}

// TODO: Update this when buildings are added back!
static B32 EndFromRoadToBuilding(V4 point, Road* road, Building* building)
{
	B32 result = true;
	return result;
}

static V4 NextFromRoadToJunction(V4 startPoint, Road* road, Junction* junction)
{
	V4 result = GetRoadLeavePoint(junction, road);
	return result;
}

static B32 EndFromRoadToJunction(V4 point, Road* road, Junction* junction)
{
	V4 endPoint = {};
	endPoint = GetRoadLeavePoint(junction, road);

	B32 result = (point.position == endPoint.position);
	return result;
}

static V4 NextFromJunctionToNothing(V4 startPoint, Junction* junction)
{
	V4 result = {};
	result.position = junction->position;

	return result;
}

static B32 EndFromJunctionToNothing(V4 point, Junction* junction)
{
	B32 result = (point.position == junction->position);
	return result;
}

static V4 NextFromJunctionToBuilding(V4 startPoint, Junction* junction, Building* building)
{
	V4 result = {};

	result.position = building->connectPointFarShow;
	result.direction = PointDirection(building->connectPointFarShow, building->connectPointClose);

	return result;
}

static B32 EndFromJunctionToBuilding(V4 point, Junction* junction, Building* building)
{
	B32 result = (point.position == building->connectPointFarShow);
	return result;
}

static V4 NextFromJunctionToRoad(V4 startPoint, Junction* junction, Road* road)
{
	V4 result = GetRoadEnterPoint(junction, road);
	return result;
}

static B32 EndFromJunctionToRoad(V4 point, Junction* junction, Road* road)
{
	V4 endPoint = GetRoadEnterPoint(junction, road);
	B32 result = (point.position == endPoint.position);
	return result;
}

static V4 NextFromRoadSidewalkToJunctionSidewalk(V4 startPoint, Road* road, Junction* junction, I32 junctionCornerIndex)
{
	V4 result = {};
	V2 corner = GetJunctionCorner(junction, junctionCornerIndex);
	result.position = corner;
	return result;
}

static B32 EndFromRoadSidewalkToJunctionSidewalk(V4 point, Road* road, Junction* junction, I32 junctionCornerIndex) 
{
	V2 corner = GetJunctionCorner(junction, junctionCornerIndex);
	B32 result = (point.position == corner);
	return result;
}

static V4 NextFromRoadSidewalkToRoadSidewalk(V4 point, Road* road, Road* nextRoad, I32 sidewalkIndex)
{
	Assert(road == nextRoad);
	Assert(sidewalkIndex == LeftRoadSidewalkIndex || sidewalkIndex == RightRoadSidewalkIndex);
	V4 result = {};
	V2 roadCoord = ToRoadCoord(road, point.position);
	B32 isAtCrossing = false;
	if (Abs(roadCoord.x - road->crossingDistance) <= SidewalkWidth * 0.5f) 
	{
		F32 along = roadCoord.x;
		F32 side = 0.0f;
		if (sidewalkIndex == LeftRoadSidewalkIndex)
			side = -(LaneWidth + SidewalkWidth * 0.5f);
		else if (sidewalkIndex == RightRoadSidewalkIndex)
			side = +(LaneWidth + SidewalkWidth * 0.5f);
		else
			InvalidCodePath;
		result.position = FromRoadCoord1(road, along, side);
	} 
	else 
	{
		F32 along = road->crossingDistance;
		F32 side = 0.0f;
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

static V4 NextFromRoadSidewalkToRoadSidewalk(V4 point, Road* road, Road* nextRoad)
{
	V4 result = {};

	V2 pointRoadCoord = ToRoadCoord(road, point.position);
	V2 nextRoadCoord = ToRoadCoord(road, nextRoad->endPoint1);

	I32 pointLaneIndex = 1;
	if (pointRoadCoord.y < 0.0f)
		pointLaneIndex = -1;

	I32 nextRoadLaneIndex = 1;
	if (nextRoadCoord.y < 0.0f)
		nextRoadLaneIndex = -1;

	if (pointLaneIndex == nextRoadLaneIndex) 
	{
		F32 roadLength = RoadLength(road);
		if (nextRoadCoord.x < roadLength * 0.5f)
			pointRoadCoord.x = SidewalkWidth * 0.5f;
		else
			pointRoadCoord.x = roadLength - SidewalkWidth * 0.5f;
	} 
	else 
	{
		if ((pointRoadCoord.x >= road->crossingDistance - CrossingWidth * 0.5f) 
			&& (pointRoadCoord.x <= road->crossingDistance + CrossingWidth * 0.5f))
			pointRoadCoord.y = nextRoadLaneIndex * (LaneWidth + SidewalkWidth * 0.5f); 
		else
			pointRoadCoord.x = road->crossingDistance;
	}

	result.position = FromRoadCoord(road, pointRoadCoord);
	return result;
}

static B32 EndFromRoadSidewalkToRoadSidewalk(V4 point, Road* road, Road* nextRoad, I32 sidewalkIndex)
{
	B32 result = false;
	Assert(road == nextRoad);
	Assert(sidewalkIndex == LeftRoadSidewalkIndex || sidewalkIndex == RightRoadSidewalkIndex);
	V2 roadCoord = ToRoadCoord(road, point.position);
	F32 crossingDistance = road->crossingDistance;
	if (Abs(roadCoord.x - crossingDistance) <= CrossingWidth * 0.5f) 
	{
		if (sidewalkIndex == LeftRoadSidewalkIndex && roadCoord.y < 0.0f)
			result = true;
		else if (sidewalkIndex == RightRoadSidewalkIndex && roadCoord.y > 0.0f)
			result = true;
	}
	return result;
}

static V4 NextFromJunctionSidewalkToNothing(V4 startPoint, Junction* junction)
{
	V4 result = startPoint;
	return result;
}

static B32 EndFromJunctionSidewalkToNothing(V4 point, Junction* junction)
{
	B32 result = true;
	return result;
}

static V4 NextFromJunctionSidewalkToJunctionSidewalk(V4 startPoint, Junction* junction, Junction* nextJunction, I32 cornerIndex)
{
	V4 result = {};
	Assert(junction == nextJunction);
	result.position = GetJunctionCorner(junction, cornerIndex);
	return result;
}

static B32 EndFromJunctionSidewalkToJunctionSidewalk(V4 point, Junction* junction, Junction* nextJunction, I32 cornerIndex)
{
	Assert(junction == nextJunction);
	V2 corner = GetJunctionCorner(junction, cornerIndex);
	B32 result = (point.position == corner);
	return result;
}

// TODO: make this work with indices instead of floating-point comparison
static V4 NextNodePoint(PathNode* node, V4 startPoint)
{
	V4 result = {};

	MapElem elem = node->elem;
	PathNode* next = node->next;
	MapElem nextElem = {};
	if (next)
		nextElem = next->elem;

	switch(elem.type) 
	{
		case MapElemBuilding:
		{
			Building* building = elem.building;
			switch (nextElem.type) 
			{
				case MapElemNone:
					result = NextFromBuildingToNothing(startPoint, building);
					break;
				case MapElemBuilding:
					result = NextFromBuildingToBuilding(startPoint, building, nextElem.building);
					break;
				case MapElemRoad:
					result = NextFromBuildingToRoad(startPoint, building, nextElem.road);
					break;
				case MapElemJunction:
					result = NextFromBuildingToJunction(startPoint, building, nextElem.junction);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemRoad:
		{
			Road* road = elem.road;
			switch (nextElem.type) 
			{
				case MapElemBuilding:
					result = NextFromRoadToBuilding(startPoint, road, nextElem.building);
					break;
				case MapElemJunction:
					result = NextFromRoadToJunction(startPoint, road, nextElem.junction);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemJunction:
		{
			Junction* junction = elem.junction;
			switch (nextElem.type) 
			{
				case MapElemNone:
					result = NextFromJunctionToNothing(startPoint, junction);
					break;
				case MapElemBuilding:
					result = NextFromJunctionToBuilding(startPoint, junction, nextElem.building);
					break;
				case MapElemRoad:
					result = NextFromJunctionToRoad(startPoint, junction, nextElem.road);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemRoadSidewalk:
		{
			Road* road = elem.road;
			switch(nextElem.type) 
			{
				case MapElemJunctionSidewalk:
					result = NextFromRoadSidewalkToJunctionSidewalk(startPoint, road, nextElem.junction, next->subElemIndex);
					break;
				case MapElemRoadSidewalk:
					result = NextFromRoadSidewalkToRoadSidewalk(startPoint, road, nextElem.road, next->subElemIndex);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemJunctionSidewalk:
		{
			Junction* junction = elem.junction;
			switch(nextElem.type) 
			{
				case MapElemNone:
					result = NextFromJunctionSidewalkToNothing(startPoint, junction);
					break;
				case MapElemRoadSidewalk:
					result = startPoint;
					break;
				case MapElemJunctionSidewalk: 
					result = NextFromJunctionSidewalkToJunctionSidewalk(startPoint, junction, nextElem.junction, next->subElemIndex);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	return result;
}

static B32 IsNodeEndPoint(PathNode* node, V4 point)
{
	B32 result = false;

	MapElem elem = node->elem;
	PathNode* next = node->next;
	MapElem nextElem = {};
	if (next)
		nextElem = next->elem;

	switch (elem.type) 
	{
		case MapElemBuilding: 
		{
			Building* building = elem.building;
			switch (nextElem.type) 
			{
				case MapElemNone:
					result = EndFromBuildingToNothing(point, building);
					break;
				case MapElemBuilding:
					result = EndFromBuildingToBuilding(point, building, nextElem.building);
					break;
				case MapElemRoad:
					result = EndFromBuildingToRoad(point, building, nextElem.road);
					break;
				case MapElemJunction:
					result = EndFromBuildingToJunction(point, building, nextElem.junction);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemRoad: 
		{
			Road* road = elem.road;
			switch (nextElem.type) 
			{
				case MapElemNone:
					DebugBreak();
					break;
				case MapElemBuilding:
					result = EndFromRoadToBuilding(point, road, nextElem.building);
					break;
				case MapElemJunction:
					result = EndFromRoadToJunction(point, road, nextElem.junction);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemJunction: 
		{
			Junction* junction = elem.junction;
			switch (nextElem.type) 
			{
				case MapElemNone:
					result = EndFromJunctionToNothing(point, junction);
					break;
				case MapElemBuilding:
					result = EndFromJunctionToBuilding(point, junction, nextElem.building);
					break;
				case MapElemRoad:
					result = EndFromJunctionToRoad(point, junction, nextElem.road);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemRoadSidewalk: 
		{
			Road* road = elem.road;
			switch (nextElem.type) 
			{
				case MapElemJunctionSidewalk:
					result = EndFromRoadSidewalkToJunctionSidewalk(point, road, nextElem.junction, next->subElemIndex);
					break;
				case MapElemRoadSidewalk:
					result = EndFromRoadSidewalkToRoadSidewalk(point, road, nextElem.road, next->subElemIndex);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		case MapElemJunctionSidewalk: 
		{
			Junction* junction = elem.junction;
			switch (nextElem.type) 
			{
				case MapElemNone:
				case MapElemRoadSidewalk:
					result = true;
					break;
				case MapElemJunctionSidewalk:
					result = EndFromJunctionSidewalkToJunctionSidewalk(point, junction, nextElem.junction, next->subElemIndex);
					break;
				default:
					DebugBreak();
			}
			break;
		}
		default: 
		{
			DebugBreak();
			break;
		}
	}

	return result;
}

static void DrawPath(Canvas canvas, PathNode* firstNode, V4 color, F32 lineWidth)
{
	PathNode* node = firstNode;

	if (node) 
	{
		V4 point = StartNodePoint(node);

		while (node) 
		{
			if (IsNodeEndPoint(node, point)) 
			{
				node = node->next;
			} 
			else 
			{
				V4 nextPoint = NextNodePoint(node, point);

				DrawLine(canvas, point.position, nextPoint.position, color, lineWidth);

				point = nextPoint;
			}
		}
	}
}

static void DrawBezierPathFromPoint(Canvas canvas, PathNode* firstNode, V4 startPoint, V4 color, F32 lineWidth)
{
	PathNode* node = firstNode;
	Bezier4 bezier4 = {};

	if (node) 
	{
		V4 point = startPoint;

		while (node) 
		{
			if (IsNodeEndPoint(node, point)) 
			{
				node = node->next;
			} 
			else 
			{
				V4 nextPoint = NextNodePoint(node, point);
				Assert(nextPoint.position.x != 0.0f || nextPoint.position.y != 0.0f);
				bezier4 = TurnBezier4(point, nextPoint);
				DrawBezier4(canvas, bezier4, color, lineWidth, 5);
				point = nextPoint;
			}
		}
	}
}

static void DrawBezierPath(Canvas canvas, PathNode* firstNode, V4 color, F32 lineWidth)
{
	PathNode* node = firstNode;
	Bezier4 bezier4 = {};

	if (node) 
	{
		V4 startPoint = StartNodePoint(node);
		DrawBezierPathFromPoint(canvas, firstNode, startPoint, color, lineWidth);
	}
}