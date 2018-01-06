#pragma once

enum MapElemType {
	MapElemNone,
	MapElemRoad,
	MapElemRoadSidewalk,
	MapElemCrossing,
	MapElemIntersection,
	MapElemIntersectionSidewalk,
	MapElemBuilding,
	MapElemBuildingConnector
};

struct Building;
struct Color;
struct Intersection;
struct Renderer;
struct Road;

struct MapElem {
	MapElemType type;

	union {
		Building* building;
		Intersection* intersection;
		Road* road;
		void* address;
	};
};

inline MapElem RoadElem(Road* road) {
	MapElem result = {};
	result.type = MapElemRoad;
	result.road = road;

	return result;
}

inline MapElem RoadSidewalkElem(Road* road) {
	MapElem result = {};
	result.type = MapElemRoadSidewalk;
	result.road = road;

	return result;
}

inline MapElem CrossingElem(Road* road) {
	MapElem result = {};
	result.type = MapElemCrossing;
	result.road = road;

	return result;
}

inline MapElem IntersectionElem(Intersection* intersection) {
	MapElem result = {};
	result.type = MapElemIntersection;
	result.intersection = intersection;

	return result;
}

inline MapElem IntersectionSidewalkElem(Intersection* intersection) {
	MapElem result = {};
	result.type = MapElemIntersectionSidewalk;
	result.intersection = intersection;

	return result;
}

inline MapElem BuildingElem(Building* building) {
	MapElem result = {};
	result.type = MapElemBuilding;
	result.building = building;

	return result;
}

inline MapElem BuildingConnectorElem(Building* building) {
	MapElem result = {};
	result.type = MapElemBuildingConnector;
	result.building = building;

	return result;
}

inline bool MapElemEqual(MapElem elem1, MapElem elem2) {
	if (elem1.type != elem2.type) 
		return false;
	else if (elem1.type == MapElemNone) 
		return true;
	else
		return (elem1.address == elem2.address);
}

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color);