#pragma once

enum MapElemType {
	MapElemNone,
	MapElemRoad,
	MapElemIntersection,
	MapElemBuilding
};

struct Building;
struct Intersection;
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
