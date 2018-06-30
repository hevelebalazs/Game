#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Type.hpp"

#define JunctionMaxRoadN			4

#define RoadColor					GetColor(0.5f, 0.5f, 0.5f)
#define LaneWidth					4.0f

#define RoadStripeColor				GetColor(1.0f, 1.0f, 1.0f)
#define RoadStripeWidth				0.25f

#define SidewalkColor				GetColor(0.4f, 0.4f, 0.4f)
#define SidewalkWidth				2.0f
#define CrossingWidth				5.0f
#define CrossingStepLength			(0.2f * LaneWidth)

#define JunctionRadius				(2.0f * LaneWidth)
#define InvalidJunctionCornerIndex	-1
#define MinimumJunctionDistance		(10.0f * LaneWidth)

#define	TrafficLightRadius			0.5f
#define TrafficLightSwitchTime		4.0f
#define TrafficLightYellowTime		2.0f

#define LeftRoadSidewalkIndex		-1
#define RightRoadSidewalkIndex		+1

struct Road;
struct Junction;

struct Road {
	V2 endPoint1;
	V2 endPoint2;

	Junction* junction1;
	Junction* junction2;

	F32 crossingDistance;
};

enum TrafficLightColor {
	TrafficLightNone,
	TrafficLightRed,
	TrafficLightYellow,
	TrafficLightGreen
};

struct TrafficLight {
	V2 position;
	TrafficLightColor color;
	F32 timeLeft;
};

struct Junction {
	V2 position;

	I32 roadN;
	// NOTE: roads are in clockwise order!
	Road* roads[JunctionMaxRoadN];
	TrafficLight trafficLights[JunctionMaxRoadN];
	F32 stopDistances[JunctionMaxRoadN];
};

// Road
F32 RoadLength(Road* road);
void GenerateCrossing(Road* road);

V4 GetRoadLeavePoint(Junction* junction, Road* road);
V4 GetRoadEnterPoint(Junction* junction, Road* road);

F32 DistanceSquareFromRoad(Road* road, V2 point);
V2 ClosestRoadPoint(Road* road, V2 point);
V2 ClosestLanePoint(Road* road, I32 laneIndex, V2 point);

B32 IsPointOnRoad(V2 point, Road* road);
B32 IsPointOnRoadSidewalk(V2 point, Road* road);
B32 IsPointOnCrossing(V2 point, Road* road);

I32 LaneIndex(Road* road, V2 point);
V2 LaneDirection(Road* road, I32 laneIndex);

I32 RoadSidewalkIndex(Road* road, V2 point);

F32 DistanceOnLane(Road* road, I32 laneIndex, V2 point);
V4 TurnPointFromLane(Road* road, I32 laneIndex, V2 point);
V4 TurnPointToLane(Road* road, I32 laneIndex, V2 point);

// Road coordinate system
V2 FromRoadCoord1(Road* road, F32 along, F32 side);
V2 FromRoadCoord2(Road* road, F32 along, F32 side);
V2 FromRoadCoord(Road* road, V2 roadCoord1);
V2 ToRoadCoord(Road* road, V2 point);

// Road graphics
void HighlightRoadSidewalk(Canvas canvas, Road* road, V4 color);
void HighlightRoad(Canvas canvas, Road* road, V4 color);
void DrawCrossing(Canvas canvas, Road* road);
void DrawTexturedCrossing(Canvas canvas, Road* road, Texture roadTexture, Texture stripeTexture);
void DrawRoad(Canvas canvas, Road* road);
void DrawTexturedRoad(Canvas canvas, Road* road, Texture roadTexure, Texture stripeTexture);
void DrawRoadSidewalk(Canvas canvas, Road* road);
void DrawTexturedRoadSidewalk(Canvas canvas, Road* road, Texture sidewalkTexture);

// Junction
B32 IsValidJunctionCornerIndex(Junction* junction, I32 cornerIndex);

I32 GetClockwiseJunctionCornerIndexDistance(Junction* junction, I32 startCornerIndex, I32 endCornerIndex);
I32 GetCounterClockwiseJunctionCornerIndexDistance(Junction* junction, I32 startCornerIndex, I32 endCornerIndex);

I32 GetRoadOutLeftJunctionCornerIndex(Junction* junction, Road* road);
I32 GetRoadOutRightJunctionCornerIndex(Junction* junction, Road* road);

I32 GetRoadLeftSidewalkJunctionCornerIndex(Junction* junction, Road* road);
I32 GetRoadRightSidewalkJunctionCornerIndex(Junction* junction, Road* road);
I32 GetRoadSidewalkJunctionCornerIndex(Junction* junction, Road* road, I32 sidewalkIndex);
V2 GetRoadLeftSidewalkJunctionCorner(Junction* junction, Road* road);
V2 GetRoadRightSidewalkJunctionCorner(Junction* junction, Road* road);

I32 GetRoadJunctionCornerSidewalkIndex(Junction* junction, Road* road, I32 cornerIndex);

I32 GetPreviousJunctionCornerIndex(Junction* junction, I32 cornerIndex);
I32 GetNextJunctionCornerIndex(Junction* junction, I32 cornerIndex);
I32 GetRandomJunctionCornerIndex(Junction* junction);
I32 GetJunctionRoadIndex(Junction* junction, Road* road);

V2 OtherRoadPoint(Junction* junction, Road* road);
void AddRoad(Junction* junction, Road* road);
void ConnectJunctions(Junction* junction1, Junction* junction2, Road* road);

V2 GetJunctionCorner(Junction* junction, I32 cornerIndex);
I32 GetClosestJunctionCornerIndex(Junction* junction, V2 point);
V2 GetClosestJunction(Junction* junction, V2 point);

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road);
B32 IsPointOnJunction(V2 point, Junction* junction);
B32 IsPointOnJunctionSidewalk(V2 point, Junction* junction);

void InitTrafficLights(Junction* junction);
void UpdateTrafficLights(Junction* junction, F32 seconds);
void DrawTrafficLights(Canvas canvas, Junction* junction);

void CalculateStopDistances(Junction* junction);
void HighlightJunctionSidewalk(Canvas canvas, Junction* junction, V4 color);
void HighlightJunction(Canvas canvas, Junction* junction, V4 color);

void DrawJunctionPlaceholder(Canvas canvas, Junction* junction, V4 color);
void DrawJunctionSidewalk(Canvas canvas, Junction* junction);
void DrawTexturedJunctionSidewalk(Canvas canvas, Junction* junction, Texture sidewalkTexture);
void DrawJunction(Canvas canvas, Junction* junction);
void DrawTexturedJunction(Canvas canvas, Junction* junction, Texture roadTexture, Texture stripeTexture);
