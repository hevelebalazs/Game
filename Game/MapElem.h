#pragma once

enum MapElemType {
	NONE,
	ROAD,
	INTERSECTION,
	BUILDING
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
