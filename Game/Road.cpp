#include <stdio.h>
#include <math.h>

#include "Debug.hpp"
#include "Game.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Point.hpp"
#include "Road.hpp"

extern Color RoadColor			= {0.5f, 0.5f, 0.5f};
extern float LaneWidth			= 4.0f;

extern Color RoadStripeColor	= {1.0f, 1.0f, 1.0f};
extern float RoadStripeWidth	= 0.2f;

extern Color SidewalkColor		= {0.4f, 0.4f, 0.4f};
extern float SidewalkWidth		= 2.0f;
extern float CrossingWidth		= 5.0f;
extern float CrossingStepLength	= 0.2f * LaneWidth;

extern float JunctionRadius             = 2.0f * LaneWidth;
extern int   InvalidJunctionCornerIndex = -1;
extern float MinimumJunctionDistance    = 10.0f * LaneWidth;

extern float TrafficLightRadius     = 0.5f;
extern float TrafficLightSwitchTime = 4.0f;
extern float TrafficLightYellowTime = 2.0f;

extern int   LeftRoadSidewalkIndex  = -1;
extern int   RightRoadSidewalkIndex = +1;

bool IsValidJunctionCornerIndex(Junction* junction, int cornerIndex)
{
	Assert(junction->roadN > 0);
	bool result = false;
	if (junction->roadN == 1)
		result = (cornerIndex == 0 || cornerIndex == 1);
	else if (junction->roadN >= 2)
		result = (cornerIndex >= 0 && cornerIndex < junction->roadN);
	else
		InvalidCodePath;
	return result;
}

int GetClockwiseJunctionCornerIndexDistance(Junction* junction, int startCornerIndex, int endCornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));
	Assert(junction->roadN >= 1);

	int distance = 0;
	if (junction->roadN == 1) {
		if (startCornerIndex == endCornerIndex)
			distance = 0;
		else
			distance = 1;
	} else if (junction->roadN >= 2) {
		distance = endCornerIndex - startCornerIndex;
		if (distance < 0)
			distance += junction->roadN;
	} else {
		InvalidCodePath;
	}

	Assert(distance >= 0 || distance <= junction->roadN);
	return distance;
}

int GetCounterClockwiseJunctionCornerIndexDistance(Junction* junction, int startCornerIndex, int endCornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));
	Assert(junction->roadN >= 1);

	int distance = 0;
	if (junction->roadN == 1) {
		if (startCornerIndex == endCornerIndex)
			distance = 0;
		else
			distance = 1;
	} else if (junction->roadN >= 2) {
		distance = startCornerIndex - endCornerIndex;
		if (distance < 0)
			distance += junction->roadN;
	} else {
		InvalidCodePath;
	}

	Assert(distance >= 0 || distance <= junction->roadN);
	return distance;
}

int GetRoadOutLeftJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	int leftCornerIndex = 0;
	if (junction->roadN == 1)
		leftCornerIndex = 0;
	else if (junction->roadN >= 2)
		leftCornerIndex = GetJunctionRoadIndex(junction, road);
	else
		InvalidCodePath;
	return leftCornerIndex;
}

int GetRoadOutRightJunctionCornerIndex(Junction* junction, Road* road)
{
	int leftCornerIndex = GetRoadOutLeftJunctionCornerIndex(junction, road);
	int rightCornerIndex = GetNextJunctionCornerIndex(junction, leftCornerIndex);
	return rightCornerIndex;
}

int GetRoadLeftSidewalkJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	int cornerIndex = 0;
	if (junction->roadN == 1) {
		Assert(junction->roads[0] == road);
		if (road->junction1 == junction)
			cornerIndex = 0;
		else if (road->junction2 == junction)
			cornerIndex = 1;
		else
			InvalidCodePath;
	} else if (junction->roadN >= 2) {
		int roadIndex = GetJunctionRoadIndex(junction, road);
		if (road->junction1 == junction)
			cornerIndex = roadIndex;
		else if (road->junction2 == junction)
			cornerIndex = GetNextJunctionCornerIndex(junction, roadIndex);
		else
			InvalidCodePath;
	} else {
		InvalidCodePath;
	}
	return cornerIndex;
}

int GetRoadRightSidewalkJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	int cornerIndex = 0;
	if (junction->roadN == 1) {
		Assert(junction->roads[0] == road);
		if (road->junction1 == junction)
			cornerIndex = 1;
		else if (road->junction2 == junction)
			cornerIndex = 0;
		else
			InvalidCodePath;
	} else if (junction->roadN >= 2) {
		int roadIndex = GetJunctionRoadIndex(junction, road);
		if (road->junction1 == junction)
			cornerIndex = GetNextJunctionCornerIndex(junction, roadIndex);
		else if (road->junction2 == junction)
			cornerIndex = roadIndex;
		else
			InvalidCodePath;
	} else {
		InvalidCodePath;
	}
	return cornerIndex;
}

int GetRoadSidewalkJunctionCornerIndex(Junction* junction, Road* road, int sidewalkIndex)
{
	int cornerIndex = 0;
	if (sidewalkIndex == LeftRoadSidewalkIndex)
		cornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	else if (sidewalkIndex == RightRoadSidewalkIndex)
		cornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	else
		InvalidCodePath;
	return cornerIndex;
}

Point GetRoadLeftSidewalkJunctionCorner(Junction* junction, Road* road)
{
	int cornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	Point result = GetJunctionCorner(junction, cornerIndex);
	return result;
}

Point GetRoadRightSidewalkJunctionCorner(Junction* junction, Road* road)
{
	int cornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	Point result = GetJunctionCorner(junction, cornerIndex);
	return result;
}

static bool IsValidJunctionRoadIndex(Junction* junction, int index)
{
	bool isValid = (index >= 0 && index < junction->roadN);
	return isValid;
}

int GetRoadJunctionCornerSidewalkIndex(Junction* junction, Road* road, int cornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
	int sidewalkIndex = 0;
	int leftRoadCornerIndex  = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	int rightRoadCornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	if (cornerIndex == leftRoadCornerIndex)
		sidewalkIndex = LeftRoadSidewalkIndex;
	else if (cornerIndex == rightRoadCornerIndex)
		sidewalkIndex = RightRoadSidewalkIndex;
	else
		InvalidCodePath;
	return sidewalkIndex;
}

int GetPreviousJunctionCornerIndex(Junction* junction, int cornerIndex)
{
	Assert(junction->roadN >= 1);
	int previousIndex = 0;
	if (junction->roadN == 1) {
		if (cornerIndex == 1)
			previousIndex = 0;
		else if (cornerIndex == 0)
			previousIndex = 1;
		else
			InvalidCodePath;
	} else if (junction->roadN >= 2) {
		Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
		if (cornerIndex == 0)
			previousIndex = junction->roadN - 1;
		else
			previousIndex = cornerIndex - 1;
	}
	return previousIndex;
}

int GetNextJunctionCornerIndex(Junction* junction, int cornerIndex)
{
	Assert(junction->roadN >= 1);
	int nextIndex = 0;
	if (junction->roadN == 1) {
		if (cornerIndex == 1)
			nextIndex = 0;
		else if (cornerIndex == 0)
			nextIndex = 1;
		else
			InvalidCodePath;
	} else if (junction->roadN >= 2) {
		Assert(IsValidJunctionRoadIndex(junction, cornerIndex))
		if (cornerIndex == junction->roadN - 1)
			nextIndex = 0;
		else
			nextIndex = cornerIndex + 1;
	}
	return nextIndex;
}

int GetRandomJunctionCornerIndex(Junction* junction)
{
	Assert(junction->roadN > 0);
	int maxIndex = 0;
	if (junction->roadN == 1)
		maxIndex = 1;
	else if (junction->roadN >= 2)
		maxIndex = junction->roadN - 1;
	else
		InvalidCodePath;
	int cornerIndex = IntRandom(0, maxIndex);
	return cornerIndex;
}

int GetJunctionRoadIndex(Junction* junction, Road* road)
{
	int index = 0;
	bool found = false;
	for (int i = 0; i < junction->roadN; ++i) {
		if (junction->roads[i] == road) {
			index = i;
			found = true;
		}
	}
	Assert(found);
	return index;
}

DirectedPoint GetRoadLeavePoint(Junction* junction, Road* road)
{
	DirectedPoint result = {};
	int roadIndex = GetJunctionRoadIndex(junction, road);
	float stopDistance = junction->stopDistances[roadIndex];
	if (road->junction1 == junction) {
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position  = FromRoadCoord1(road, +stopDistance, -LaneWidth * 0.5f);
	} else if (road->junction2 == junction) {
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position  = FromRoadCoord2(road, -stopDistance, +LaneWidth * 0.5f);
	} else {
		InvalidCodePath;
	}
	return result;
}

DirectedPoint GetRoadEnterPoint(Junction* junction, Road* road)
{
	DirectedPoint result = {};
	int roadIndex = GetJunctionRoadIndex(junction, road);
	float stopDistance = junction->stopDistances[roadIndex];
	if (road->junction1 == junction) {
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position  = FromRoadCoord1(road, +stopDistance, +LaneWidth * 0.5f);
	} else {
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position  = FromRoadCoord2(road, -stopDistance, -LaneWidth * 0.5f);
	}
	return result;
}

// TODO: introduce road angle and use it for calculations
float DistanceSquareFromRoad(Road* road, Point point)
{
	Point closest = ClosestRoadPoint(road, point);
	float result = DistanceSquare(point, closest);
	return result;
}

Point ClosestRoadPoint(Road* road, Point point)
{
	Point result = {};
	Point resultRoadCoord = {};
	Point pointRoadCoord = ToRoadCoord(road, point);

	float roadLength = RoadLength(road);
	if (pointRoadCoord.x < 0.0f)
		resultRoadCoord.x = 0.0f;
	else if (pointRoadCoord.x < roadLength)
		resultRoadCoord.x = pointRoadCoord.x;
	else
		resultRoadCoord.x = roadLength;

	resultRoadCoord.y = 0.0f;

	result = FromRoadCoord(road, resultRoadCoord);
	return result;
}

Point ClosestLanePoint(Road* road, int laneIndex, Point point)
{
	Point pointRoadCoord = ToRoadCoord(road, point);
	if (laneIndex > 0)
		pointRoadCoord.y = +LaneWidth * 0.5f;
	else
		pointRoadCoord.y = -LaneWidth * 0.5f;
	Point result = FromRoadCoord(road, pointRoadCoord);
	return result;
}

bool IsPointOnRoad(Point point, Road* road)
{
	Point roadCoord = ToRoadCoord(road, point);
	bool result = false;
	float roadLength = RoadLength(road);
	if (IsBetween(roadCoord.x, 0.0f, roadLength)) {
		if (IsBetween(roadCoord.y, -LaneWidth, +LaneWidth))
			result = true;
	}
	return result;
}

bool IsPointOnRoadSidewalk(Point point, Road* road)
{
	Point pointRoadCoord = ToRoadCoord(road, point);

	float roadLength = RoadLength(road);
	bool result = IsBetween(pointRoadCoord.x, 0.0f, roadLength) 
		       && IsBetween(Abs(pointRoadCoord.y), LaneWidth, LaneWidth + SidewalkWidth);

	return result;
}

void HighlightRoad(Renderer renderer, Road* road, Color color)
{
	float side = LaneWidth + SidewalkWidth;
	float length = RoadLength(road);
	Point rBL = Point{0.0f,   -side};
	Point rBR = Point{0.0f,   +side};
	Point rTL = Point{length, -side};
	Point rTR = Point{length, +side};

	Point pBL = FromRoadCoord(road, rBL);
	Point pBR = FromRoadCoord(road, rBR);
	Point pTL = FromRoadCoord(road, rTL);
	Point pTR = FromRoadCoord(road, rTR);

	Quad quad = {pBR, pBL, pTL, pTR};
	DrawQuad(renderer, quad, color);
}

int LaneIndex(Road* road, Point point)
{
	int result = 0;
	bool turnsRight = TurnsRight(road->endPoint1, road->endPoint2, point);

	if (turnsRight)
		result = 1;
	else
		result = -1;

	return result;
}

Point LaneDirection(Road* road, int laneIndex)
{
	Point result = {};

	if (laneIndex == 1)
		result = PointDirection(road->endPoint1, road->endPoint2);
	else if (laneIndex == -1)
		result = PointDirection(road->endPoint2, road->endPoint1);
	
	return result;
}

static DirectedPoint GetRoadLaneEnterPoint(Road* road, int laneIndex) {
	DirectedPoint result = {};

	if (laneIndex == 1) {
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position = FromRoadCoord1(road, 0.0f, +LaneWidth * 0.5f);
	} else {
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position = FromRoadCoord2(road, 0.0f, -LaneWidth * 0.5f);
	}

	return result;
}

// TODO: can DistanceOnLane and TurnPointFromLane be merged?
float DistanceOnLane(Road* road, int laneIndex, Point point)
{
	DirectedPoint startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	float length = VectorLength(parallelVector);

	return length;
}

DirectedPoint TurnPointFromLane(Road* road, int laneIndex, Point point)
{
	DirectedPoint result = {};

	DirectedPoint startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointDiff(
		PointSum(startPoint.position, parallelVector),
		PointProd(LaneWidth * 0.5f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

DirectedPoint TurnPointToLane(Road* road, int laneIndex, Point point)
{
		DirectedPoint result = {};

	DirectedPoint startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointSum(
		PointSum(startPoint.position, parallelVector),
		PointProd(LaneWidth * 0.5f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

void HighlightRoadSidewalk(Renderer renderer, Road* road, Color color)
{
	float closeSide = LaneWidth;
	float farSide   = LaneWidth + SidewalkWidth;
	Quad leftSidewalkQuad = {
		FromRoadCoord1(road, 0.0f, -closeSide),
		FromRoadCoord1(road, 0.0f, -farSide),
		FromRoadCoord2(road, 0.0f, -farSide),
		FromRoadCoord2(road, 0.0f, -closeSide)
	};
	Quad rightSidewalkQuad = {
		FromRoadCoord1(road, 0.0f, +closeSide),
		FromRoadCoord1(road, 0.0f, +farSide),
		FromRoadCoord2(road, 0.0f, +farSide),
		FromRoadCoord2(road, 0.0f, +closeSide)
	};
	DrawQuad(renderer, leftSidewalkQuad, color);
	DrawQuad(renderer, rightSidewalkQuad, color);
}

void DrawCrossing(Renderer renderer, Road* road)
{
	Color stepColor = Color{1.0f, 1.0f, 1.0f};
	float stepDistance = -LaneWidth;
	bool drawStep = true;

	float crossingSide1 = road->crossingDistance - CrossingWidth * 0.5f;
	float crossingSide2 = road->crossingDistance + CrossingWidth * 0.5f;

	while (stepDistance < LaneWidth) {
		float newStepDistance = stepDistance + CrossingStepLength;
		if (newStepDistance > LaneWidth)
			newStepDistance = LaneWidth;

		Color drawColor = {};
		if (drawStep)
			drawColor = RoadStripeColor;
		else
			drawColor = RoadColor;
		
		Point point1 = FromRoadCoord1(road, crossingSide1, newStepDistance);
		Point point2 = FromRoadCoord1(road, crossingSide1, stepDistance);
		Point point3 = FromRoadCoord1(road, crossingSide2, stepDistance);
		Point point4 = FromRoadCoord1(road, crossingSide2, newStepDistance);
		Quad quad = {point1, point2, point3, point4};
		DrawQuad(renderer, quad, drawColor);

		drawStep = !drawStep;
		stepDistance = newStepDistance;
	}
}

float RoadLength(Road* road)
{
	return Distance(road->endPoint1, road->endPoint2);
}

static float GetRoadStopDistance(Road* road, Junction* junction)
{
	bool foundRoad = false;
	float result = 0.0f;
	for (int i = 0; i < junction->roadN; ++i) {
		if (junction->roads[i] == road) {
			result = junction->stopDistances[i];
			foundRoad = true;
		}
	}
	Assert(foundRoad);
	return result;
}

void GenerateCrossing(Road* road)
{
	float roadLength = RoadLength(road);
	float stopDistance1 = GetRoadStopDistance(road, road->junction1);
	float stopDistance2 = GetRoadStopDistance(road, road->junction2);
	float minCrossingDistance = stopDistance1 + (CrossingWidth * 0.5f);
	float maxCrossingDistance = roadLength - stopDistance2 - (CrossingWidth * 0.5f);
	Assert(minCrossingDistance < maxCrossingDistance);
	road->crossingDistance = RandomBetween(minCrossingDistance, maxCrossingDistance);
}

bool IsPointOnCrossing(Point point, Road* road)
{
	Point pointRoadCoord = ToRoadCoord(road, point);

	bool result = (
		(Abs(pointRoadCoord.x - road->crossingDistance) <= CrossingWidth * 0.5f) &&
		(Abs(pointRoadCoord.y) <= LaneWidth)
	);

	return result;
}

int RoadSidewalkIndex(Road* road, Point point)
{
	int result = 0;
	Point pointRoadCoord = ToRoadCoord(road, point);
	if (pointRoadCoord.y < -LaneWidth)
		result = -1;
	else if (pointRoadCoord.y > LaneWidth)
		result = 1;

	return result;
}

Point FromRoadCoord1(Road* road, float along, float side)
{
	Point roadDirection = PointDirection(road->endPoint1, road->endPoint2);
	Point sideDirection = TurnVectorToRight(roadDirection);

	Point addPoint = PointSum(
		PointProd(along, roadDirection),
		PointProd(side, sideDirection)
	);

	Point result = PointSum(road->endPoint1, addPoint);
	return result;
}

Point FromRoadCoord2(Road* road, float along, float side)
{
	Point roadDirection = PointDirection(road->endPoint1, road->endPoint2);
	Point sideDirection = TurnVectorToRight(roadDirection);

	Point addPoint = PointSum(
		PointProd(along, roadDirection),
		PointProd(side, sideDirection)
	);

	Point result = PointSum(road->endPoint2, addPoint);
	return result;
}

Point FromRoadCoord(Road* road, Point roadCoord1) 
{
	Point result = FromRoadCoord1(road, roadCoord1.x, roadCoord1.y);
	return result;
}

Point ToRoadCoord(Road* road, Point point)
{
	Point roadDirection = PointDirection(road->endPoint1, road->endPoint2);	
	Point xy = PointDiff(point, road->endPoint1);
	Point result = XYToBase(xy, roadDirection);
	return result;
}

void DrawRoad(Renderer renderer, Road* road)
{
	DrawLine(renderer, road->endPoint1, road->endPoint2, RoadColor, 2.0f * LaneWidth);
	DrawLine(renderer, road->endPoint1, road->endPoint2, RoadStripeColor, RoadStripeWidth);
	DrawCrossing(renderer, road);
}

void DrawRoadSidewalk(Renderer renderer, Road* road)
{
	Point side1 = FromRoadCoord1(road, 0.0, -LaneWidth - SidewalkWidth * 0.5f);
	Point side2 = FromRoadCoord2(road, 0.0, -LaneWidth - SidewalkWidth * 0.5f);
	DrawLine(renderer, side1, side2, SidewalkColor, SidewalkWidth);

	side1 = FromRoadCoord1(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	side2 = FromRoadCoord2(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	DrawLine(renderer, side1, side2, SidewalkColor, SidewalkWidth);
}

// Junction
Point OtherRoadPoint(Junction* junction, Road* road)
{
	Point point = {};
	if (road->junction1 == junction)
		point = road->endPoint2;
	else if (road->junction2 == junction)
		point = road->endPoint1;
	return point;
}

void AddRoad(Junction* junction, Road* road)
{
	Assert(junction->roadN < JunctionMaxRoadN);
	junction->roads[junction->roadN] = road;
	junction->roadN++;

	int i = junction->roadN - 1;
	while (i > 0) {
		Point prevPoint = OtherRoadPoint(junction, junction->roads[i - 1]);
		Point thisPoint = OtherRoadPoint(junction, junction->roads[i]);	
		float prevAngle = LineAngle(junction->position, prevPoint);
		float thisAngle = LineAngle(junction->position, thisPoint);
		if (prevAngle > thisAngle) {
			Road* tmpRoad = junction->roads[i];
			junction->roads[i] = junction->roads[i - 1];
			junction->roads[i - 1] = tmpRoad;
			--i;
		} else {
			break;	
		}
	}
}

void ConnectJunctions(Junction* junction1, Junction* junction2, Road* road) 
{
	Assert(junction1 != junction2);
	road->junction1 = junction1;
	road->junction2 = junction2;
	road->endPoint1 = road->junction1->position;
	road->endPoint2 = road->junction2->position;
	AddRoad(junction1, road);
	AddRoad(junction2, road);
}

static int GetPreviousJunctionRoadIndex(Junction* junction, int roadIndex)
{
	Assert(junction->roadN >= 2);
	int result = 0;
	if (roadIndex == 0)
		result = junction->roadN - 1;
	else
		result = roadIndex - 1;
	return result;
}

static int GetNextJunctionRoadIndex(Junction* junction, int roadIndex)
{
	Assert(junction->roadN >= 2);
	int result = 0;
	if (roadIndex == junction->roadN - 1)
		result = 0;
	else
		result = roadIndex + 1;
	return result;
}

float GetRoadAngleAtJunction(Junction* junction, int roadIndex)
{
	Assert(junction->roadN >= 1);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	float angle = 0.0f;
	Road* road = junction->roads[roadIndex];
	if (road->junction1 == junction)
		angle = LineAngle(road->endPoint1, road->endPoint2);
	else if (road->junction2 == junction)
		angle = LineAngle(road->endPoint2, road->endPoint1);
	else
		InvalidCodePath;
	return angle;
}

static Point FromLeaveJunctionRoadCoord(Junction* junction, int roadIndex, float distanceAlong, float distanceSide)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Road* road = junction->roads[roadIndex];
	Point result = {};
	if (road->junction1 == junction)
		result = FromRoadCoord1(road, +distanceAlong, +distanceSide);
	else if (road->junction2 == junction)
		result = FromRoadCoord2(road, -distanceAlong, -distanceSide);
	else
		InvalidCodePath;
	return result;
}

static Point ToLeaveJunctionRoadCoord(Junction* junction, int roadIndex, Point point)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Road* road = junction->roads[roadIndex];
	Point result = {};
	Point roadCoord = ToRoadCoord(road, point);
	if (road->junction1 == junction) {
		result = roadCoord;
	} else if (road->junction2 == junction) {
		result.x = RoadLength(road) - roadCoord.x;
		result.y = -roadCoord.y;
	} else {
		InvalidCodePath;
	}
	return result;
}

static Line GetLineParallelToRoad(Road* road, float side)
{
	Line line = {};
	line.p1 = FromRoadCoord1(road, 0.0f, side);
	line.p2 = FromRoadCoord2(road, 0.0f, side);
	return line;
}

static Line GetLineParallelToIncomingRoad(Junction* junction, Road* road, float side)
{
	Line line = {};
	if (road->junction2 == junction)
		line = GetLineParallelToRoad(road, side);
	else if (road->junction1 == junction)
		line = GetLineParallelToRoad(road, -side);
	else
		InvalidCodePath;
	return line;
}

static Point GetRoadIntersectionAtJunction(Junction* junction, int leftRoadIndex, int rightRoadIndex, float side)
{
	Assert(IsValidJunctionRoadIndex(junction, leftRoadIndex));
	Assert(IsValidJunctionRoadIndex(junction, rightRoadIndex));
	Assert(leftRoadIndex != rightRoadIndex);

	float leftAngle = GetRoadAngleAtJunction(junction, leftRoadIndex);
	float rightAngle = GetRoadAngleAtJunction(junction, rightRoadIndex);
	if (rightAngle < leftAngle)
		rightAngle += 2 * PI;

	float intersectionAngle = (leftAngle + rightAngle) * 0.5f;
	float halfAngle = (rightAngle - leftAngle) * 0.5f;

	float sinHalfAngle = sinf(halfAngle);
	if (sinHalfAngle == 0.0f) {
		InvalidCodePath;
		sinHalfAngle = 1.0f;
	}
	float intersectionDistance = Abs(side / sinHalfAngle);
	Point intersection = {};
	intersection.x = junction->position.x + intersectionDistance * cosf(intersectionAngle);
	intersection.y = junction->position.y + intersectionDistance * sinf(intersectionAngle);
	return intersection;
}

static Point GetRightRoadIntersectionAtJunction(Junction* junction, int roadIndex, float side)
{
	Assert(junction->roadN >= 2);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	int nextRoadIndex = GetNextJunctionRoadIndex(junction, roadIndex);
	Point intersection = GetRoadIntersectionAtJunction(junction, roadIndex, nextRoadIndex, side);
	return intersection;
}

static Point GetLeftRoadIntersectionAtJunction(Junction* junction, int roadIndex, float side)
{
	Assert(junction->roadN >= 2);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	int previousRoadIndex = GetPreviousJunctionRoadIndex(junction, roadIndex);
	Point intersection = GetRoadIntersectionAtJunction(junction, previousRoadIndex, roadIndex, side);
	return intersection;
}

Point GetJunctionCorner(Junction* junction, int cornerIndex)
{
	Point result = {};
	Assert(junction->roadN >= 1);
	float radius = LaneWidth + SidewalkWidth * 0.5f;
	if (junction->roadN == 1) {
		if (cornerIndex == 0)
			result = FromLeaveJunctionRoadCoord(junction, 0, -radius, -radius);
		else if (cornerIndex == 1)
			result = FromLeaveJunctionRoadCoord(junction, 0, -radius, +radius);
		else
			InvalidCodePath;
	} else if (junction->roadN >= 2) {
		int roadIndex = cornerIndex;
		Assert(IsValidJunctionRoadIndex(junction, roadIndex));
		result = GetLeftRoadIntersectionAtJunction(junction, roadIndex, radius);
	}
	return result;
}

int GetClosestJunctionCornerIndex(Junction* junction, Point point)
{
	Assert(junction->roadN >= 1);
	int cornerIndex = InvalidJunctionCornerIndex;
	if (junction->roadN == 1) {
		Point roadCoord = ToLeaveJunctionRoadCoord(junction, 0, point);
		if (roadCoord.y < 0.0f)
			cornerIndex = 0;
		else
			cornerIndex = 1;
	} else if (junction->roadN >= 2) {
		float pointAngle = LineAngle(junction->position, point);
		for (int i = 0; i < junction->roadN; ++i) {
			int next = GetNextJunctionRoadIndex(junction, i);
			float thisAngle = GetRoadAngleAtJunction(junction, i);
			float nextAngle = GetRoadAngleAtJunction(junction, next);
			if (IsAngleBetween(thisAngle, pointAngle, nextAngle)) {
				cornerIndex = next;
				break;
			}
		}
	}
	Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
	return cornerIndex;
}

Point GetClosestJunction(Junction* junction, Point point)
{
	int cornerIndex = GetClosestJunctionCornerIndex(junction, point);
	Point corner = GetJunctionCorner(junction, cornerIndex);
	return corner;
}

int RandomQuarterIndex()
{
	return 1 + (rand() % 4);
}

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road)
{
	TrafficLight* trafficLight = 0;
	for (int i = 0; i < junction->roadN; ++i) {
		if (junction->roads[i] == road)
			trafficLight = &junction->trafficLights[i];
	}
	return trafficLight;
}

static Poly16 GetJunctionPolyForRoad(Junction* junction, int roadIndex, float side)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Assert(junction->roadN >= 1);
	Poly16 poly = {};
	float MinExtraDistance = 0.001f;
	if (junction->roadN == 1) {
		Road* road = junction->roads[0];
		if (road->junction1 == junction) {
			Poly16Add(&poly, FromRoadCoord1(road, +side, +side));
			Poly16Add(&poly, FromRoadCoord1(road, -side, +side));
			Poly16Add(&poly, FromRoadCoord1(road, -side, -side));
			Poly16Add(&poly, FromRoadCoord1(road, +side, -side));
		} else if (road->junction2 == junction) {
			Poly16Add(&poly, FromRoadCoord2(road, -side, -side));
			Poly16Add(&poly, FromRoadCoord2(road, +side, -side));
			Poly16Add(&poly, FromRoadCoord2(road, +side, +side));
			Poly16Add(&poly, FromRoadCoord2(road, -side, +side));
		} else {
			InvalidCodePath;
		}
	} else if (junction->roadN == 2) {
		Road* road = junction->roads[roadIndex];
		Point leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, roadIndex, side);
		Point rightIntersection = GetRightRoadIntersectionAtJunction(junction, roadIndex, side);

		Point leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, roadIndex, leftIntersection);
		Point rightRoadCoord = ToLeaveJunctionRoadCoord(junction, roadIndex, rightIntersection);

		float leftSideDistance = leftRoadCoord.x;
		float rightSideDistance = rightRoadCoord.x;

		Poly16Add(&poly, junction->position);
		Poly16Add(&poly, leftIntersection);

		if (leftSideDistance < LaneWidth - MinExtraDistance) {
			Point extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, LaneWidth, -side);
			Poly16Add(&poly, extraPoint);
		}
		if (rightSideDistance < LaneWidth - MinExtraDistance) {
			Point extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, LaneWidth, +side);
			Poly16Add(&poly, extraPoint);
		}

		Poly16Add(&poly, rightIntersection);
	} else if (junction->roadN >= 3) {
		Road* road = junction->roads[roadIndex];
		Point leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, roadIndex, side);
		Point rightIntersection = GetRightRoadIntersectionAtJunction(junction, roadIndex, side);

		Point leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, roadIndex, leftIntersection);
		Point rightRoadCoord = ToLeaveJunctionRoadCoord(junction, roadIndex, rightIntersection);
	
		float leftSideDistance  = leftRoadCoord.x;
		float rightSideDistance = rightRoadCoord.x;

		Poly16Add(&poly, junction->position);
		Poly16Add(&poly, leftIntersection);

		if (Abs(leftSideDistance - rightSideDistance) >= MinExtraDistance) {
			Point extraPoint = {};
			if (leftSideDistance < rightSideDistance)
				extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, rightSideDistance, -side);
			else
				extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, leftSideDistance, +side);
			Poly16Add(&poly, extraPoint);
		}

		Poly16Add(&poly, rightIntersection);
	}
	return poly;
}

bool IsPointOnJunction(Point point, Junction* junction)
{
	bool result = false;
	if (junction->roadN == 0) {
		float x = junction->position.x;
		float y = junction->position.y;
		float left   = x - LaneWidth;
		float right  = x + LaneWidth;
		float top    = y - LaneWidth;
		float bottom = y + LaneWidth;
		result = ((left <= point.x && point.x <= right) && (top <= point.y && point.y <= bottom));
	} else {
		for (int i = 0; i < junction->roadN; ++i) {
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			if (IsPointInPoly(point, poly.points, poly.pointN)) {
				result = true;
				break;
			}
		}
	}
	return result;
}

bool IsPointOnJunctionSidewalk(Point point, Junction* junction)
{
	bool isOnSidewalk = false;
	bool isOnJunction = IsPointOnJunction(point, junction);
	if (isOnJunction) {
		isOnSidewalk = false;
	} else {
		for (int i = 0; i < junction->roadN; ++i) {
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
			if (IsPointInPoly(point, poly.points, poly.pointN)) {
				isOnSidewalk = true;
				break;
			}
		}
	}
	return isOnSidewalk;
}

static void StartTrafficLight(TrafficLight* trafficLight)
{
	trafficLight->color = TrafficLightGreen;
	trafficLight->timeLeft = TrafficLightSwitchTime;
}

static void UpdateTrafficLight(TrafficLight* trafficLight, float seconds)
{
	if (trafficLight->color == TrafficLightGreen) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->timeLeft += TrafficLightYellowTime;
			trafficLight->color = TrafficLightYellow;
		}
	}
	else if (trafficLight->color == TrafficLightYellow) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->color = TrafficLightRed;
		}
	}
}

static void DrawTrafficLight(Renderer renderer, TrafficLight* trafficLight)
{
	Color drawColor = {};
	switch (trafficLight->color) {
		case TrafficLightGreen: {
			drawColor = {0.0f, 1.0f, 0.0f}; 
			break;
		}
		case TrafficLightYellow: {
			drawColor = {1.0f, 1.0f, 0.0f};
			break;
		}
		case TrafficLightRed: {
			drawColor = {1.0f, 0.0f, 0.0f};
			break;
		}
	}

	Point position = trafficLight->position;
	float radius = TrafficLightRadius;
	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		drawColor
	);
}

void InitTrafficLights(Junction* junction)
{
	if (junction->roadN >= 3) {
		for (int i = 0; i < junction->roadN; ++i) {
			Road* road = junction->roads[i];
			TrafficLight* trafficLight = junction->trafficLights + i;
			float stopDistance = junction->stopDistances[i];
			trafficLight->color = TrafficLightRed;
			if (road->junction1 == junction)
				trafficLight->position = FromRoadCoord1(road, +stopDistance, -0.5f * LaneWidth);
			else if (road->junction2 == junction)
				trafficLight->position = FromRoadCoord2(road, -stopDistance, +0.5f * LaneWidth);
			else
				InvalidCodePath;
		}

		StartTrafficLight(junction->trafficLights + 0);
	}
}

void UpdateTrafficLights(Junction* junction, float seconds)
{
	if (junction->roadN >= 3) {
		for (int i = 0; i < junction->roadN; ++i) {
			TrafficLight* trafficLight = &junction->trafficLights[i];
			if (trafficLight->color != TrafficLightRed) {
				UpdateTrafficLight(trafficLight, seconds);
				if (trafficLight->color == TrafficLightRed) {
					int next = i + 1;
					if (next >= junction->roadN)
						next = 0;
					TrafficLight* nextTrafficLight = &junction->trafficLights[next];
					StartTrafficLight(nextTrafficLight);
				}		
			}
		}
	}
}

void DrawTrafficLights(Renderer renderer, Junction* junction)
{
	if (junction->roadN >= 3) {
		for (int i = 0; i < junction->roadN; ++i)
			DrawTrafficLight(renderer, junction->trafficLights + i);
	}
}

void CalculateStopDistances(Junction* junction)
{
	Assert(junction->roadN >= 1);
	if (junction->roadN == 1) {
		for (int i = 0; i < junction->roadN; ++i)
			junction->stopDistances[i] = LaneWidth;
	} else if (junction->roadN == 2) {
		for (int i = 0; i < junction->roadN; ++i) {
			float stopDistance = 0.0f;
			Road* road = junction->roads[i];
			Point leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, i, LaneWidth);
			Point rightIntersection = GetRightRoadIntersectionAtJunction(junction, i, LaneWidth);

			Point leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, i, leftIntersection);
			Point rightRoadCoord = ToLeaveJunctionRoadCoord(junction, i, rightIntersection);

			float leftSideDistance = leftRoadCoord.x;
			float rightSideDistance = rightRoadCoord.x;
			junction->stopDistances[i] = Max3(leftSideDistance, rightSideDistance, LaneWidth);
		}
	} else {
		for (int i = 0; i < junction->roadN; ++i) {
			float stopDistance = 0.0f;
			Road* road = junction->roads[i];
			Point rightIntersection = GetRightRoadIntersectionAtJunction(junction, i, LaneWidth);
			Point leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, i, LaneWidth);

			Point leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, i, leftIntersection);
			Point rightRoadCoord = ToLeaveJunctionRoadCoord(junction, i, rightIntersection);
	
			float leftSideDistance  = leftRoadCoord.x;
			float rightSideDistance = rightRoadCoord.x;
			junction->stopDistances[i] = Max2(leftSideDistance, rightSideDistance);
		}
	}
}

void HighlightJunctionSidewalk(Renderer renderer, Junction* junction, Color color)
{
	Assert(junction->roadN >= 1);
	for (int i = 0; i < junction->roadN; ++i) {
		Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
		DrawPoly(renderer, poly.points, poly.pointN, color);
	}
	DrawJunction(renderer, junction);
}

void HighlightJunction(Renderer renderer, Junction* junction, Color color)
{
	if (junction->roadN >= 1) {
		for (int i = 0; i < junction->roadN; ++i) {
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			DrawPoly(renderer, poly.points, poly.pointN, color);
		}
	} else {
		DrawJunctionPlaceholder(renderer, junction, color);
	}
}

void DrawJunctionSidewalk(Renderer renderer, Junction* junction)
{
	if (junction->roadN >= 1) {
		for (int i = 0; i < junction->roadN; ++i) {
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
			DrawPoly(renderer, poly.points, poly.pointN, SidewalkColor);
		}
	}
}

static void DrawJunctionStripes(Renderer renderer, Junction* junction)
{
	if (junction->roadN <= 2) {
		for (int i = 0; i < junction->roadN; ++i) {
			Road* road = junction->roads[i];
			float stopDistance = junction->stopDistances[i];
			Point roadEnterPoint = {};
			if (road->junction1 == junction)
				roadEnterPoint = FromRoadCoord1(road, +stopDistance, 0.0f);
			else if (road->junction2 == junction)
				roadEnterPoint = FromRoadCoord2(road, -stopDistance, 0.0f);
			else
				InvalidCodePath;
			DrawLine(renderer, junction->position, roadEnterPoint, RoadStripeColor, RoadStripeWidth);
		}
	} else {
		for (int i = 0; i < junction->roadN; ++i) {
			Road* road = junction->roads[i];
			float stopDistance = junction->stopDistances[i];
			Point point1 = {};
			Point point2 = {};
			if (road->junction1 == junction) {
				point1 = FromRoadCoord1(road, +stopDistance, 0.0f);
				point2 = FromRoadCoord1(road, +stopDistance, -LaneWidth);
			} else {
				point1 = FromRoadCoord2(road, -stopDistance, 0.0f);
				point2 = FromRoadCoord2(road, -stopDistance, +LaneWidth);
			}
			DrawLine(renderer, point1, point2, RoadStripeColor, RoadStripeWidth);
		}
	}
}

void DrawJunctionPlaceholder(Renderer renderer, Junction* junction, Color color)
{
	Point position = junction->position;
	float side = LaneWidth + SidewalkWidth;
	DrawRect(
		renderer,
		position.y - side, position.x - side,
		position.y + side, position.x + side,
		color
	);

	side = LaneWidth;
	DrawRect(
		renderer,
		position.y - side, position.x - side,
		position.y + side, position.x + side,
		color
	);
}

void DrawJunction(Renderer renderer, Junction* junction)
{
	if (junction->roadN >= 1) {
		for (int i = 0; i < junction->roadN; ++i) {
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			DrawPoly(renderer, poly.points, poly.pointN, RoadColor);
		}
		DrawJunctionStripes(renderer, junction);
	} else {
		DrawJunctionPlaceholder(renderer, junction, RoadColor);
	}
}
