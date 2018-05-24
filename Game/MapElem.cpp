#include "Building.h"
#include "MapElem.h"
#include "Renderer.h"
#include "Road.h"

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color) {
	if (mapElem.type == MapElemBuilding)
		HighlightBuilding(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemBuildingConnector)
		HighlightBuildingConnector(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemJunction)
		HighlightJunction(renderer, mapElem.junction, color);
	else if (mapElem.type == MapElemJunctionSidewalk) 
		HighlightJunctionSidewalk(renderer, mapElem.junction, color);
	else if (mapElem.type == MapElemRoad)
		HighlightRoad(renderer, mapElem.road, color);
	else if (mapElem.type == MapElemRoadSidewalk)
		HighlightRoadSidewalk(renderer, mapElem.road, color);
}
