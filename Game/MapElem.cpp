#include "Building.h"
#include "Intersection.h"
#include "MapElem.h"
#include "Renderer.h"
#include "Road.h"

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color) {
	if (mapElem.type == MapElemBuilding)
		HighlightBuilding(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemBuildingConnector)
		HighlightBuildingConnector(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemIntersection)
		HighlightIntersection(renderer, *mapElem.intersection, color);
	else if (mapElem.type == MapElemIntersectionSidewalk) 
		HighlightIntersectionSidewalk(renderer, *mapElem.intersection, color);
	else if (mapElem.type == MapElemRoad)
		HighlightRoad(renderer, *mapElem.road, color);
	else if (mapElem.type == MapElemRoadSidewalk)
		HighlightRoadSidewalk(renderer, *mapElem.road, color);
}
