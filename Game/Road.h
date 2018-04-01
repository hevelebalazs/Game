#pragma once

#include "Bitmap.h"
#include "Math.h"
#include "Point.h"
#include "Renderer.h"
#include "Texture.h"

extern Color RoadColor;
extern float LaneWidth;

extern Color RoadStripeColor;
extern float RoadStripeWidth;

extern Color SidewalkColor;
extern float SidewalkWidth;
extern float CrossingWidth;

extern float TrafficLightRadius;
extern float TrafficLightSwitchTime;
extern float TrafficLightYellowTime;

struct GameAssets;
struct Road;
struct Junction;

struct Road {
	Point endPoint1;
	Point endPoint2;

	Junction* junction1;
	Junction* junction2;

	float crossingDistance;
};

enum TrafficLightColor {
	TrafficLightNone,
	TrafficLightRed,
	TrafficLightYellow,
	TrafficLightGreen
};

struct TrafficLight {
	Point position;
	TrafficLightColor color;
	float timeLeft;
};

// TODO: name this QuarterIndex
enum {
	QuarterNone,
	QuarterTopLeft,
	QuarterTopRight,
	QuarterBottomLeft,
	QuarterBottomRight
};

struct Junction {
	Point position;

	Road* roads[4];
	TrafficLight trafficLights[4];
	// TODO: create an array of these
	Road* leftRoad = 0;
	Road* rightRoad = 0;
	Road* topRoad = 0;
	Road* bottomRoad = 0;

	// TODO: create an array of these, each should correspond to a road
	TrafficLight leftTrafficLight;
	TrafficLight rightTrafficLight;
	TrafficLight topTrafficLight;
	TrafficLight bottomTrafficLight;
};

// Road
float RoadLength(Road* road);
void GenerateCrossing(Road* road);

DirectedPoint RoadLeavePoint(Road road, int endPointIndex);
DirectedPoint RoadEnterPoint(Road road, int endPointIndex);

float DistanceSquareFromRoad(Road road, Point point);
Point ClosestRoadPoint(Road road, Point point);
Point ClosestLanePoint(Road road, int laneIndex, Point point);

bool IsPointOnRoad(Point point, Road road);
bool IsPointOnRoadSide(Point point, Road road);
bool IsPointOnRoadSidewalk(Point point, Road road);
bool IsPointOnCrossing(Point point, Road road);

int LaneIndex(Road road, Point point);
Point LaneDirection(Road road, int laneIndex);

int RoadSidewalkIndex(Road road, Point point);

float DistanceOnLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point);

// Road coordinate system
Point FromRoadCoord1(Road road, float along, float side);
Point FromRoadCoord2(Road road, float along, float side);
Point FromRoadCoord(Road road, Point roadCoord1);
Point ToRoadCoord(Road road, Point point);

// Road graphics
void HighlightRoadSidewalk(Renderer renderer, Road road, Color color);
void HighlightRoad(Renderer renderer, Road road, Color color);
void DrawRoad(Renderer renderer, Road road, GameAssets* assets);
void DrawRoad(Renderer renderer, Road road);

// Junction
int RandomQuarterIndex();

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road);
bool IsPointOnJunction(Point point, Junction junction);
bool IsPointOnJunctionSidewalk(Point point, Junction junction);

void InitTrafficLights(Junction* junction);
void UpdateTrafficLights(Junction* junction, float seconds);
void DrawTrafficLights(Renderer renderer, Junction junction);

void HighlightJunctionSidewalk(Renderer renderer, Junction junction, Color color);
void HighlightJunction(Renderer renderer, Junction junction, Color color);
void DrawJunction(Renderer renderer, Junction junction, GameAssets* assets);
void DrawJunction(Renderer renderer, Junction junction);

int QuarterIndex(Junction* junction, Point point);
Point JunctionSidewalkCorner(Junction* junction, int quarterIndex);