#pragma once

enum MapElemType {
	MapElemNone,
	MapElemRoad,
	MapElemIntersection,
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

inline MapElem IntersectionElem(Intersection* intersection) {
	MapElem result = {};
	result.type = MapElemIntersection;
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

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color);