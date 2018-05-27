#pragma once

#include "Bitmap.h"
#include "Math.h"
#include "Point.h"
#include "Renderer.h"

#define JunctionMaxRoadN 4

extern Color RoadColor;
extern float LaneWidth;

extern Color RoadStripeColor;
extern float RoadStripeWidth;

extern Color SidewalkColor;
extern float SidewalkWidth;
extern float CrossingWidth;
extern float CrossingStepLength;

extern float JunctionRadius;
extern int   InvalidJunctionCornerIndex;
extern float MinimumJunctionDistance;

extern float TrafficLightRadius;
extern float TrafficLightSwitchTime;
extern float TrafficLightYellowTime;

extern int   LeftRoadSidewalkIndex;
extern int   RightRoadSidewalkIndex;

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

struct Junction {
	Point position;

	int roadN;
	// NOTE: roads are in clockwise order!
	Road* roads[JunctionMaxRoadN];
	TrafficLight trafficLights[JunctionMaxRoadN];
	float stopDistances[JunctionMaxRoadN];
};

// Road
float RoadLength(Road* road);
void GenerateCrossing(Road* road);

DirectedPoint GetRoadLeavePoint(Junction* junction, Road* road);
DirectedPoint GetRoadEnterPoint(Junction* junction, Road* road);

float DistanceSquareFromRoad(Road* road, Point point);
Point ClosestRoadPoint(Road* road, Point point);
Point ClosestLanePoint(Road* road, int laneIndex, Point point);

bool IsPointOnRoad(Point point, Road* road);
bool IsPointOnRoadSidewalk(Point point, Road* road);
bool IsPointOnCrossing(Point point, Road* road);

int LaneIndex(Road* road, Point point);
Point LaneDirection(Road* road, int laneIndex);

int RoadSidewalkIndex(Road* road, Point point);

float DistanceOnLane(Road* road, int laneIndex, Point point);
DirectedPoint TurnPointFromLane(Road* road, int laneIndex, Point point);
DirectedPoint TurnPointToLane(Road* road, int laneIndex, Point point);

// Road coordinate system
Point FromRoadCoord1(Road* road, float along, float side);
Point FromRoadCoord2(Road* road, float along, float side);
Point FromRoadCoord(Road* road, Point roadCoord1);
Point ToRoadCoord(Road* road, Point point);

// Road graphics
void HighlightRoadSidewalk(Renderer renderer, Road* road, Color color);
void HighlightRoad(Renderer renderer, Road* road, Color color);
void DrawCrossing(Renderer renderer, Road* road);
void DrawRoad(Renderer renderer, Road* road);
void DrawRoadSidewalk(Renderer renderer, Road* road);

// Junction
bool IsValidJunctionCornerIndex(Junction* junction, int cornerIndex);

int GetClockwiseJunctionCornerIndexDistance(Junction* junction, int startCornerIndex, int endCornerIndex);
int GetCounterClockwiseJunctionCornerIndexDistance(Junction* junction, int startCornerIndex, int endCornerIndex);

int GetRoadOutLeftJunctionCornerIndex(Junction* junction, Road* road);
int GetRoadOutRightJunctionCornerIndex(Junction* junction, Road* road);

int GetRoadLeftSidewalkJunctionCornerIndex(Junction* junction, Road* road);
int GetRoadRightSidewalkJunctionCornerIndex(Junction* junction, Road* road);
int GetRoadSidewalkJunctionCornerIndex(Junction* junction, Road* road, int sidewalkIndex);
Point GetRoadLeftSidewalkJunctionCorner(Junction* junction, Road* road);
Point GetRoadRightSidewalkJunctionCorner(Junction* junction, Road* road);

int GetRoadJunctionCornerSidewalkIndex(Junction* junction, Road* road, int cornerIndex);

int GetPreviousJunctionCornerIndex(Junction* junction, int cornerIndex);
int GetNextJunctionCornerIndex(Junction* junction, int cornerIndex);
int GetRandomJunctionCornerIndex(Junction* junction);
int GetJunctionRoadIndex(Junction* junction, Road* road);

Point OtherRoadPoint(Junction* junction, Road* road);
void AddRoad(Junction* junction, Road* road);
void ConnectJunctions(Junction* junction1, Junction* junction2, Road* road);

Point GetJunctionCorner(Junction* junction, int cornerIndex);
int GetClosestJunctionCornerIndex(Junction* junction, Point point);
Point GetClosestJunction(Junction* junction, Point point);

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road);
bool IsPointOnJunction(Point point, Junction* junction);
bool IsPointOnJunctionSidewalk(Point point, Junction* junction);

void InitTrafficLights(Junction* junction);
void UpdateTrafficLights(Junction* junction, float seconds);
void DrawTrafficLights(Renderer renderer, Junction* junction);

void CalculateStopDistances(Junction* junction);
void HighlightJunctionSidewalk(Renderer renderer, Junction* junction, Color color);
void HighlightJunction(Renderer renderer, Junction* junction, Color color);

void DrawJunctionPlaceholder(Renderer renderer, Junction* junction, Color color);
void DrawJunctionSidewalk(Renderer renderer, Junction* junction);
void DrawJunction(Renderer renderer, Junction* junction);