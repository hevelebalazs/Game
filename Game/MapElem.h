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

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color);