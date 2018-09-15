#include <stdio.h>
#include <math.h>

#include "Debug.hpp"
#include "Game.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Road.hpp"
#include "Type.hpp"

B32 IsValidJunctionCornerIndex(Junction* junction, I32 cornerIndex)
{
	Assert(junction->roadN > 0);
	B32 result = false;
	if (junction->roadN == 1)
		result = (cornerIndex == 0 || cornerIndex == 1);
	else if (junction->roadN >= 2)
		result = (cornerIndex >= 0 && cornerIndex < junction->roadN);
	else
		InvalidCodePath;
	return result;
}

I32 GetClockwiseJunctionCornerIndexDistance(Junction* junction, I32 startCornerIndex, I32 endCornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));
	Assert(junction->roadN >= 1);

	I32 distance = 0;
	if (junction->roadN == 1) 
	{
		if (startCornerIndex == endCornerIndex)
			distance = 0;
		else
			distance = 1;
	}
	else if (junction->roadN >= 2) 
	{
		distance = endCornerIndex - startCornerIndex;
		if (distance < 0)
			distance += junction->roadN;
	} 
	else 
	{
		InvalidCodePath;
	}

	Assert(distance >= 0 || distance <= junction->roadN);
	return distance;
}

I32 GetCounterClockwiseJunctionCornerIndexDistance(Junction* junction, I32 startCornerIndex, I32 endCornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, startCornerIndex));
	Assert(IsValidJunctionCornerIndex(junction, endCornerIndex));
	Assert(junction->roadN >= 1);

	I32 distance = 0;
	if (junction->roadN == 1) 
	{
		if (startCornerIndex == endCornerIndex)
			distance = 0;
		else
			distance = 1;
	} 
	else if (junction->roadN >= 2) 
	{
		distance = startCornerIndex - endCornerIndex;
		if (distance < 0)
			distance += junction->roadN;
	} 
	else 
	{
		InvalidCodePath;
	}

	Assert(distance >= 0 || distance <= junction->roadN);
	return distance;
}

I32 GetRoadOutLeftJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	I32 leftCornerIndex = 0;
	if (junction->roadN == 1)
		leftCornerIndex = 0;
	else if (junction->roadN >= 2)
		leftCornerIndex = GetJunctionRoadIndex(junction, road);
	else
		InvalidCodePath;
	return leftCornerIndex;
}

I32 GetRoadOutRightJunctionCornerIndex(Junction* junction, Road* road)
{
	I32 leftCornerIndex = GetRoadOutLeftJunctionCornerIndex(junction, road);
	I32 rightCornerIndex = GetNextJunctionCornerIndex(junction, leftCornerIndex);
	return rightCornerIndex;
}

I32 GetRoadLeftSidewalkJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	I32 cornerIndex = 0;
	if (junction->roadN == 1) 
	{
		Assert(junction->roads[0] == road);
		if (road->junction1 == junction)
			cornerIndex = 0;
		else if (road->junction2 == junction)
			cornerIndex = 1;
		else
			InvalidCodePath;
	} 
	else if (junction->roadN >= 2) 
	{
		I32 roadIndex = GetJunctionRoadIndex(junction, road);
		if (road->junction1 == junction)
			cornerIndex = roadIndex;
		else if (road->junction2 == junction)
			cornerIndex = GetNextJunctionCornerIndex(junction, roadIndex);
		else
			InvalidCodePath;
	}
	else 
	{
		InvalidCodePath;
	}
	return cornerIndex;
}

I32 GetRoadRightSidewalkJunctionCornerIndex(Junction* junction, Road* road)
{
	Assert(junction->roadN >= 1);
	I32 cornerIndex = 0;
	if (junction->roadN == 1) 
	{
		Assert(junction->roads[0] == road);
		if (road->junction1 == junction)
			cornerIndex = 1;
		else if (road->junction2 == junction)
			cornerIndex = 0;
		else
			InvalidCodePath;
	} 
	else if (junction->roadN >= 2) 
	{
		I32 roadIndex = GetJunctionRoadIndex(junction, road);
		if (road->junction1 == junction)
			cornerIndex = GetNextJunctionCornerIndex(junction, roadIndex);
		else if (road->junction2 == junction)
			cornerIndex = roadIndex;
		else
			InvalidCodePath;
	} 
	else 
	{
		InvalidCodePath;
	}
	return cornerIndex;
}

I32 GetRoadSidewalkJunctionCornerIndex(Junction* junction, Road* road, I32 sidewalkIndex)
{
	I32 cornerIndex = 0;
	if (sidewalkIndex == LeftRoadSidewalkIndex)
		cornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	else if (sidewalkIndex == RightRoadSidewalkIndex)
		cornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	else
		InvalidCodePath;
	return cornerIndex;
}

V2 GetRoadLeftSidewalkJunctionCorner(Junction* junction, Road* road)
{
	I32 cornerIndex = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	V2 result = GetJunctionCorner(junction, cornerIndex);
	return result;
}

V2 GetRoadRightSidewalkJunctionCorner(Junction* junction, Road* road)
{
	I32 cornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	V2 result = GetJunctionCorner(junction, cornerIndex);
	return result;
}

static B32 IsValidJunctionRoadIndex(Junction* junction, I32 index)
{
	B32 isValid = (index >= 0 && index < junction->roadN);
	return isValid;
}

I32 GetRoadJunctionCornerSidewalkIndex(Junction* junction, Road* road, I32 cornerIndex)
{
	Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
	I32 sidewalkIndex = 0;
	I32 leftRoadCornerIndex  = GetRoadLeftSidewalkJunctionCornerIndex(junction, road);
	I32 rightRoadCornerIndex = GetRoadRightSidewalkJunctionCornerIndex(junction, road);
	if (cornerIndex == leftRoadCornerIndex)
		sidewalkIndex = LeftRoadSidewalkIndex;
	else if (cornerIndex == rightRoadCornerIndex)
		sidewalkIndex = RightRoadSidewalkIndex;
	else
		InvalidCodePath;
	return sidewalkIndex;
}

I32 GetPreviousJunctionCornerIndex(Junction* junction, I32 cornerIndex)
{
	Assert(junction->roadN >= 1);
	I32 previousIndex = 0;
	if (junction->roadN == 1) 
	{
		if (cornerIndex == 1)
			previousIndex = 0;
		else if (cornerIndex == 0)
			previousIndex = 1;
		else
			InvalidCodePath;
	} 
	else if (junction->roadN >= 2) 
	{
		Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
		if (cornerIndex == 0)
			previousIndex = junction->roadN - 1;
		else
			previousIndex = cornerIndex - 1;
	}
	return previousIndex;
}

I32 GetNextJunctionCornerIndex(Junction* junction, I32 cornerIndex)
{
	Assert(junction->roadN >= 1);
	I32 nextIndex = 0;
	if (junction->roadN == 1) 
	{
		if (cornerIndex == 1)
			nextIndex = 0;
		else if (cornerIndex == 0)
			nextIndex = 1;
		else
			InvalidCodePath;
	} 
	else if (junction->roadN >= 2) 
	{
		Assert(IsValidJunctionRoadIndex(junction, cornerIndex))
		if (cornerIndex == junction->roadN - 1)
			nextIndex = 0;
		else
			nextIndex = cornerIndex + 1;
	}
	return nextIndex;
}

I32 GetRandomJunctionCornerIndex(Junction* junction)
{
	Assert(junction->roadN > 0);
	I32 maxIndex = 0;
	if (junction->roadN == 1)
		maxIndex = 1;
	else if (junction->roadN >= 2)
		maxIndex = junction->roadN - 1;
	else
		InvalidCodePath;
	I32 cornerIndex = IntRandom(0, maxIndex);
	return cornerIndex;
}

I32 GetJunctionRoadIndex(Junction* junction, Road* road)
{
	I32 index = 0;
	B32 found = false;
	for (I32 i = 0; i < junction->roadN; ++i) 
	{
		if (junction->roads[i] == road) 
		{
			index = i;
			found = true;
		}
	}
	Assert(found);
	return index;
}

V4 GetRoadLeavePoint(Junction* junction, Road* road)
{
	V4 result = {};
	I32 roadIndex = GetJunctionRoadIndex(junction, road);
	F32 stopDistance = junction->stopDistances[roadIndex];
	if (road->junction1 == junction) 
	{
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position  = FromRoadCoord1(road, +stopDistance, -LaneWidth * 0.5f);
	} 
	else if (road->junction2 == junction) 
	{
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position  = FromRoadCoord2(road, -stopDistance, +LaneWidth * 0.5f);
	} 
	else 
	{
		InvalidCodePath;
	}
	return result;
}

V4 GetRoadEnterPoint(Junction* junction, Road* road)
{
	V4 result = {};
	I32 roadIndex = GetJunctionRoadIndex(junction, road);
	F32 stopDistance = junction->stopDistances[roadIndex];
	if (road->junction1 == junction) 
	{
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position  = FromRoadCoord1(road, +stopDistance, +LaneWidth * 0.5f);
	} 
	else 
	{
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position  = FromRoadCoord2(road, -stopDistance, -LaneWidth * 0.5f);
	}
	return result;
}

// TODO: introduce road angle and use it for calculations
F32 DistanceSquareFromRoad(Road* road, V2 point)
{
	V2 closest = ClosestRoadPoint(road, point);
	F32 result = DistanceSquare(point, closest);
	return result;
}

V2 ClosestRoadPoint(Road* road, V2 point)
{
	V2 result = {};
	V2 resultRoadCoord = {};
	V2 pointRoadCoord = ToRoadCoord(road, point);

	F32 roadLength = RoadLength(road);
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

V2 ClosestLanePoint(Road* road, I32 laneIndex, V2 point)
{
	V2 pointRoadCoord = ToRoadCoord(road, point);
	if (laneIndex > 0)
		pointRoadCoord.y = +LaneWidth * 0.5f;
	else
		pointRoadCoord.y = -LaneWidth * 0.5f;
	V2 result = FromRoadCoord(road, pointRoadCoord);
	return result;
}

B32 IsPointOnRoad(V2 point, Road* road)
{
	V2 roadCoord = ToRoadCoord(road, point);
	B32 result = false;
	F32 roadLength = RoadLength(road);
	if (IsBetween(roadCoord.x, 0.0f, roadLength)) 
	{
		if (IsBetween(roadCoord.y, -LaneWidth, +LaneWidth))
			result = true;
	}
	return result;
}

B32 IsPointOnRoadSidewalk(V2 point, Road* road)
{
	V2 pointRoadCoord = ToRoadCoord(road, point);

	F32 roadLength = RoadLength(road);
	B32 result = IsBetween(pointRoadCoord.x, 0.0f, roadLength) 
		       && IsBetween(Abs(pointRoadCoord.y), LaneWidth, LaneWidth + SidewalkWidth);

	return result;
}

void HighlightRoad(Canvas canvas, Road* road, V4 color)
{
	F32 side = LaneWidth + SidewalkWidth;
	F32 length = RoadLength(road);
	V2 rBL = MakePoint(0.0f,   -side);
	V2 rBR = MakePoint(0.0f,   +side);
	V2 rTL = MakePoint(length, -side);
	V2 rTR = MakePoint(length, +side);

	V2 pBL = FromRoadCoord(road, rBL);
	V2 pBR = FromRoadCoord(road, rBR);
	V2 pTL = FromRoadCoord(road, rTL);
	V2 pTR = FromRoadCoord(road, rTR);

	Quad quad = {pBR, pBL, pTL, pTR};
	DrawQuad(canvas, quad, color);
}

I32 LaneIndex(Road* road, V2 point)
{
	I32 result = 0;
	B32 turnsRight = TurnsRight(road->endPoint1, road->endPoint2, point);

	if (turnsRight)
		result = 1;
	else
		result = -1;

	return result;
}

V2 LaneDirection(Road* road, I32 laneIndex)
{
	V2 result = {};

	if (laneIndex == 1)
		result = PointDirection(road->endPoint1, road->endPoint2);
	else if (laneIndex == -1)
		result = PointDirection(road->endPoint2, road->endPoint1);
	
	return result;
}

static V4 GetRoadLaneEnterPoint(Road* road, I32 laneIndex) 
{
	V4 result = {};

	if (laneIndex == 1) 
	{
		result.direction = PointDirection(road->endPoint1, road->endPoint2);
		result.position = FromRoadCoord1(road, 0.0f, +LaneWidth * 0.5f);
	} 
	else 
	{
		result.direction = PointDirection(road->endPoint2, road->endPoint1);
		result.position = FromRoadCoord2(road, 0.0f, -LaneWidth * 0.5f);
	}

	return result;
}

// TODO: can DistanceOnLane and TurnPointFromLane be merged?
F32 DistanceOnLane(Road* road, I32 laneIndex, V2 point)
{
	V4 startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	V2 vector = (point - startPoint.position);

	V2 parallelVector = ParallelVector(vector, startPoint.direction);

	F32 length = VectorLength(parallelVector);
	return length;
}

V4 TurnPointFromLane(Road* road, I32 laneIndex, V2 point)
{
	V4 result = {};

	V4 startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	V2 vector = (point - startPoint.position);

	V2 parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = (startPoint.position + parallelVector) - ((LaneWidth * 0.5f) * startPoint.direction);
	result.direction = startPoint.direction;

	return result;
}

V4 TurnPointToLane(Road* road, I32 laneIndex, V2 point)
{
	V4 result = {};

	V4 startPoint = GetRoadLaneEnterPoint(road, laneIndex);
	V2 vector = (point - startPoint.position);

	V2 parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = (startPoint.position + parallelVector) + ((LaneWidth * 0.5f) * startPoint.direction);
	result.direction = startPoint.direction;

	return result;
}

void HighlightRoadSidewalk(Canvas canvas, Road* road, V4 color)
{
	F32 closeSide = LaneWidth;
	F32 farSide   = LaneWidth + SidewalkWidth;
	Quad leftSidewalkQuad = 
	{
		FromRoadCoord1(road, 0.0f, -closeSide),
		FromRoadCoord1(road, 0.0f, -farSide),
		FromRoadCoord2(road, 0.0f, -farSide),
		FromRoadCoord2(road, 0.0f, -closeSide)
	};
	Quad rightSidewalkQuad = 
	{
		FromRoadCoord1(road, 0.0f, +closeSide),
		FromRoadCoord1(road, 0.0f, +farSide),
		FromRoadCoord2(road, 0.0f, +farSide),
		FromRoadCoord2(road, 0.0f, +closeSide)
	};
	DrawQuad(canvas, leftSidewalkQuad, color);
	DrawQuad(canvas, rightSidewalkQuad, color);
}

void DrawCrossing(Canvas canvas, Road* road)
{
	V4 stepColor = MakeColor(1.0f, 1.0f, 1.0f);
	F32 stepDistance = -LaneWidth;
	B32 drawStep = true;

	F32 crossingSide1 = road->crossingDistance - CrossingWidth * 0.5f;
	F32 crossingSide2 = road->crossingDistance + CrossingWidth * 0.5f;

	while (stepDistance < LaneWidth) 
	{
		F32 newStepDistance = stepDistance + CrossingStepLength;
		if (newStepDistance > LaneWidth)
			newStepDistance = LaneWidth;

		V4 drawColor = {};
		if (drawStep)
			drawColor = RoadStripeColor;
		else
			drawColor = RoadColor;
		
		V2 point1 = FromRoadCoord1(road, crossingSide1, newStepDistance);
		V2 point2 = FromRoadCoord1(road, crossingSide1, stepDistance);
		V2 point3 = FromRoadCoord1(road, crossingSide2, stepDistance);
		V2 point4 = FromRoadCoord1(road, crossingSide2, newStepDistance);
		Quad quad = {point1, point2, point3, point4};
		DrawQuad(canvas, quad, drawColor);

		drawStep = !drawStep;
		stepDistance = newStepDistance;
	}
}

void DrawTexturedCrossing(Canvas canvas, Road* road, Texture roadTexture, Texture stripeTexture)
{
	F32 stepDistance = -LaneWidth;
	B32 drawStep = true;

	F32 crossingSide1 = road->crossingDistance - CrossingWidth * 0.5f;
	F32 crossingSide2 = road->crossingDistance + CrossingWidth * 0.5f;

	while (stepDistance < LaneWidth) 
	{
		F32 newStepDistance = stepDistance + CrossingStepLength;
		if (newStepDistance > LaneWidth)
			newStepDistance = LaneWidth;

		Texture drawTexture = {};
		if (drawStep)
			drawTexture = stripeTexture;
		else
			drawTexture = roadTexture;
		
		V2 point1 = FromRoadCoord1(road, crossingSide1, newStepDistance);
		V2 point2 = FromRoadCoord1(road, crossingSide1, stepDistance);
		V2 point3 = FromRoadCoord1(road, crossingSide2, stepDistance);
		V2 point4 = FromRoadCoord1(road, crossingSide2, newStepDistance);
		Quad quad = {point1, point2, point3, point4};
		DrawWorldTextureQuad(canvas, quad, drawTexture);

		drawStep = !drawStep;
		stepDistance = newStepDistance;
	}
}

F32 RoadLength(Road* road)
{
	return Distance(road->endPoint1, road->endPoint2);
}

static F32 GetRoadStopDistance(Road* road, Junction* junction)
{
	B32 foundRoad = false;
	F32 result = 0.0f;
	for (I32 i = 0; i < junction->roadN; ++i) 
	{
		if (junction->roads[i] == road) 
		{
			result = junction->stopDistances[i];
			foundRoad = true;
		}
	}
	Assert(foundRoad);
	return result;
}

void GenerateCrossing(Road* road)
{
	F32 roadLength = RoadLength(road);
	F32 stopDistance1 = GetRoadStopDistance(road, road->junction1);
	F32 stopDistance2 = GetRoadStopDistance(road, road->junction2);
	F32 minCrossingDistance = stopDistance1 + (CrossingWidth * 0.5f);
	F32 maxCrossingDistance = roadLength - stopDistance2 - (CrossingWidth * 0.5f);
	Assert(minCrossingDistance < maxCrossingDistance);
	road->crossingDistance = RandomBetween(minCrossingDistance, maxCrossingDistance);
}

B32 IsPointOnCrossing(V2 point, Road* road)
{
	V2 pointRoadCoord = ToRoadCoord(road, point);

	B32 result = (
		(Abs(pointRoadCoord.x - road->crossingDistance) <= CrossingWidth * 0.5f) &&
		(Abs(pointRoadCoord.y) <= LaneWidth)
	);

	return result;
}

I32 RoadSidewalkIndex(Road* road, V2 point)
{
	I32 result = 0;
	V2 pointRoadCoord = ToRoadCoord(road, point);
	if (pointRoadCoord.y < -LaneWidth)
		result = -1;
	else if (pointRoadCoord.y > LaneWidth)
		result = 1;

	return result;
}

V2 FromRoadCoord1(Road* road, F32 along, F32 side)
{
	V2 roadDirection = PointDirection(road->endPoint1, road->endPoint2);
	V2 sideDirection = TurnVectorToRight(roadDirection);

	V2 addPoint = (along * roadDirection) + (side * sideDirection);
	V2 result = (road->endPoint1 + addPoint);
	return result;
}

V2 FromRoadCoord2(Road* road, F32 along, F32 side)
{
	V2 roadDirection = PointDirection(road->endPoint1, road->endPoint2);
	V2 sideDirection = TurnVectorToRight(roadDirection);

	V2 addPoint = (along * roadDirection) + (side * sideDirection);
	V2 result = (road->endPoint2 + addPoint);
	return result;
}

V2 FromRoadCoord(Road* road, V2 roadCoord1) 
{
	V2 result = FromRoadCoord1(road, roadCoord1.x, roadCoord1.y);
	return result;
}

V2 ToRoadCoord(Road* road, V2 point)
{
	V2 roadDirection = PointDirection(road->endPoint1, road->endPoint2);	
	V2 xy = (point - road->endPoint1);
	V2 result = XYToBase(xy, roadDirection);
	return result;
}

void DrawRoad(Canvas canvas, Road* road)
{
	DrawLine(canvas, road->endPoint1, road->endPoint2, RoadColor, 2.0f * LaneWidth);
	DrawLine(canvas, road->endPoint1, road->endPoint2, RoadStripeColor, RoadStripeWidth);
	DrawCrossing(canvas, road);
}

void DrawTexturedRoad(Canvas canvas, Road* road, Texture roadTexture, Texture stripeTexture)
{
	DrawWorldTextureLine(canvas, road->endPoint1, road->endPoint2, 2.0f * LaneWidth, roadTexture);
	DrawWorldTextureLine(canvas, road->endPoint1, road->endPoint2, RoadStripeWidth, stripeTexture);
	DrawTexturedCrossing(canvas, road, roadTexture, stripeTexture);
}

void DrawRoadSidewalk(Canvas canvas, Road* road)
{
	V2 side1 = FromRoadCoord1(road, 0.0f, -LaneWidth - SidewalkWidth * 0.5f);
	V2 side2 = FromRoadCoord2(road, 0.0f, -LaneWidth - SidewalkWidth * 0.5f);
	DrawLine(canvas, side1, side2, SidewalkColor, SidewalkWidth);

	side1 = FromRoadCoord1(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	side2 = FromRoadCoord2(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	DrawLine(canvas, side1, side2, SidewalkColor, SidewalkWidth);
}

void DrawTexturedRoadSidewalk(Canvas canvas, Road* road, Texture sidewalkTexture)
{
	V2 side1 = FromRoadCoord1(road, 0.0f, -LaneWidth - SidewalkWidth * 0.5f);
	V2 side2 = FromRoadCoord2(road, 0.0f, -LaneWidth - SidewalkWidth * 0.5f);
	DrawWorldTextureLine(canvas, side1, side2, SidewalkWidth, sidewalkTexture);

	side1 = FromRoadCoord1(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	side2 = FromRoadCoord2(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	DrawWorldTextureLine(canvas, side1, side2, SidewalkWidth, sidewalkTexture);
}

// Junction
V2 OtherRoadPoint(Junction* junction, Road* road)
{
	V2 point = {};
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

	I32 i = junction->roadN - 1;
	while (i > 0) 
	{
		V2 prevPoint = OtherRoadPoint(junction, junction->roads[i - 1]);
		V2 thisPoint = OtherRoadPoint(junction, junction->roads[i]);	
		F32 prevAngle = LineAngle(junction->position, prevPoint);
		F32 thisAngle = LineAngle(junction->position, thisPoint);
		if (prevAngle > thisAngle) 
		{
			Road* tmpRoad = junction->roads[i];
			junction->roads[i] = junction->roads[i - 1];
			junction->roads[i - 1] = tmpRoad;
			--i;
		} 
		else 
		{
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

static I32 GetPreviousJunctionRoadIndex(Junction* junction, I32 roadIndex)
{
	Assert(junction->roadN >= 2);
	I32 result = 0;
	if (roadIndex == 0)
		result = junction->roadN - 1;
	else
		result = roadIndex - 1;
	return result;
}

static I32 GetNextJunctionRoadIndex(Junction* junction, I32 roadIndex)
{
	Assert(junction->roadN >= 2);
	I32 result = 0;
	if (roadIndex == junction->roadN - 1)
		result = 0;
	else
		result = roadIndex + 1;
	return result;
}

F32 GetRoadAngleAtJunction(Junction* junction, I32 roadIndex)
{
	Assert(junction->roadN >= 1);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	F32 angle = 0.0f;
	Road* road = junction->roads[roadIndex];
	if (road->junction1 == junction)
		angle = LineAngle(road->endPoint1, road->endPoint2);
	else if (road->junction2 == junction)
		angle = LineAngle(road->endPoint2, road->endPoint1);
	else
		InvalidCodePath;
	return angle;
}

static V2 FromLeaveJunctionRoadCoord(Junction* junction, I32 roadIndex, F32 distanceAlong, F32 distanceSide)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Road* road = junction->roads[roadIndex];
	V2 result = {};
	if (road->junction1 == junction)
		result = FromRoadCoord1(road, +distanceAlong, +distanceSide);
	else if (road->junction2 == junction)
		result = FromRoadCoord2(road, -distanceAlong, -distanceSide);
	else
		InvalidCodePath;
	return result;
}

static V2 ToLeaveJunctionRoadCoord(Junction* junction, I32 roadIndex, V2 point)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Road* road = junction->roads[roadIndex];
	V2 result = {};
	V2 roadCoord = ToRoadCoord(road, point);
	if (road->junction1 == junction) 
	{
		result = roadCoord;
	} 
	else if (road->junction2 == junction) 
	{
		result.x = RoadLength(road) - roadCoord.x;
		result.y = -roadCoord.y;
	} 
	else 
	{
		InvalidCodePath;
	}
	return result;
}

static Line GetLineParallelToRoad(Road* road, F32 side)
{
	Line line = {};
	line.p1 = FromRoadCoord1(road, 0.0f, side);
	line.p2 = FromRoadCoord2(road, 0.0f, side);
	return line;
}

static Line GetLineParallelToIncomingRoad(Junction* junction, Road* road, F32 side)
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

static V2 GetRoadIntersectionAtJunction(Junction* junction, I32 leftRoadIndex, I32 rightRoadIndex, F32 side)
{
	Assert(IsValidJunctionRoadIndex(junction, leftRoadIndex));
	Assert(IsValidJunctionRoadIndex(junction, rightRoadIndex));
	Assert(leftRoadIndex != rightRoadIndex);

	F32 leftAngle = GetRoadAngleAtJunction(junction, leftRoadIndex);
	F32 rightAngle = GetRoadAngleAtJunction(junction, rightRoadIndex);
	if (rightAngle < leftAngle)
		rightAngle += 2 * PI;

	F32 intersectionAngle = (leftAngle + rightAngle) * 0.5f;
	F32 halfAngle = (rightAngle - leftAngle) * 0.5f;

	F32 sinHalfAngle = sinf(halfAngle);
	if (sinHalfAngle == 0.0f) 
	{
		InvalidCodePath;
		sinHalfAngle = 1.0f;
	}
	F32 intersectionDistance = Abs(side / sinHalfAngle);
	V2 intersection = {};
	intersection.x = junction->position.x + intersectionDistance * cosf(intersectionAngle);
	intersection.y = junction->position.y + intersectionDistance * sinf(intersectionAngle);
	return intersection;
}

static V2 GetRightRoadIntersectionAtJunction(Junction* junction, I32 roadIndex, F32 side)
{
	Assert(junction->roadN >= 2);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	I32 nextRoadIndex = GetNextJunctionRoadIndex(junction, roadIndex);
	V2 intersection = GetRoadIntersectionAtJunction(junction, roadIndex, nextRoadIndex, side);
	return intersection;
}

static V2 GetLeftRoadIntersectionAtJunction(Junction* junction, I32 roadIndex, F32 side)
{
	Assert(junction->roadN >= 2);
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	I32 previousRoadIndex = GetPreviousJunctionRoadIndex(junction, roadIndex);
	V2 intersection = GetRoadIntersectionAtJunction(junction, previousRoadIndex, roadIndex, side);
	return intersection;
}

V2 GetJunctionCorner(Junction* junction, I32 cornerIndex)
{
	V2 result = {};
	Assert(junction->roadN >= 1);
	F32 radius = LaneWidth + SidewalkWidth * 0.5f;
	if (junction->roadN == 1) 
	{
		if (cornerIndex == 0)
			result = FromLeaveJunctionRoadCoord(junction, 0, -radius, -radius);
		else if (cornerIndex == 1)
			result = FromLeaveJunctionRoadCoord(junction, 0, -radius, +radius);
		else
			InvalidCodePath;
	} 
	else if (junction->roadN >= 2) 
	{
		I32 roadIndex = cornerIndex;
		Assert(IsValidJunctionRoadIndex(junction, roadIndex));
		result = GetLeftRoadIntersectionAtJunction(junction, roadIndex, radius);
	}
	return result;
}

I32 GetClosestJunctionCornerIndex(Junction* junction, V2 point)
{
	Assert(junction->roadN >= 1);
	I32 cornerIndex = InvalidJunctionCornerIndex;
	if (junction->roadN == 1) 
	{
		V2 roadCoord = ToLeaveJunctionRoadCoord(junction, 0, point);
		if (roadCoord.y < 0.0f)
			cornerIndex = 0;
		else
			cornerIndex = 1;
	} 
	else if (junction->roadN >= 2) 
	{
		F32 pointAngle = LineAngle(junction->position, point);
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			I32 next = GetNextJunctionRoadIndex(junction, i);
			F32 thisAngle = GetRoadAngleAtJunction(junction, i);
			F32 nextAngle = GetRoadAngleAtJunction(junction, next);
			if (IsAngleBetween(thisAngle, pointAngle, nextAngle)) 
			{
				cornerIndex = next;
				break;
			}
		}
	}
	Assert(IsValidJunctionCornerIndex(junction, cornerIndex));
	return cornerIndex;
}

V2 GetClosestJunction(Junction* junction, V2 point)
{
	I32 cornerIndex = GetClosestJunctionCornerIndex(junction, point);
	V2 corner = GetJunctionCorner(junction, cornerIndex);
	return corner;
}

I32 RandomQuarterIndex()
{
	return 1 + (rand() % 4);
}

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road)
{
	TrafficLight* trafficLight = 0;
	for (I32 i = 0; i < junction->roadN; ++i) 
	{
		if (junction->roads[i] == road)
			trafficLight = &junction->trafficLights[i];
	}
	return trafficLight;
}

static Poly16 GetJunctionPolyForRoad(Junction* junction, I32 roadIndex, F32 side)
{
	Assert(IsValidJunctionRoadIndex(junction, roadIndex));
	Assert(junction->roadN >= 1);
	Poly16 poly = {};
	F32 MinExtraDistance = 0.001f;
	if (junction->roadN == 1) 
	{
		Road* road = junction->roads[0];
		if (road->junction1 == junction) 
		{
			Poly16Add(&poly, FromRoadCoord1(road, +side, +side));
			Poly16Add(&poly, FromRoadCoord1(road, -side, +side));
			Poly16Add(&poly, FromRoadCoord1(road, -side, -side));
			Poly16Add(&poly, FromRoadCoord1(road, +side, -side));
		} 
		else if (road->junction2 == junction) 
		{
			Poly16Add(&poly, FromRoadCoord2(road, -side, -side));
			Poly16Add(&poly, FromRoadCoord2(road, +side, -side));
			Poly16Add(&poly, FromRoadCoord2(road, +side, +side));
			Poly16Add(&poly, FromRoadCoord2(road, -side, +side));
		} 
		else 
		{
			InvalidCodePath;
		}
	}
	else if (junction->roadN == 2) 
	{
		Road* road = junction->roads[roadIndex];
		V2 leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, roadIndex, side);
		V2 rightIntersection = GetRightRoadIntersectionAtJunction(junction, roadIndex, side);

		V2 leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, roadIndex, leftIntersection);
		V2 rightRoadCoord = ToLeaveJunctionRoadCoord(junction, roadIndex, rightIntersection);

		F32 leftSideDistance = leftRoadCoord.x;
		F32 rightSideDistance = rightRoadCoord.x;

		Poly16Add(&poly, junction->position);
		Poly16Add(&poly, leftIntersection);

		if (leftSideDistance < LaneWidth - MinExtraDistance) 
		{
			V2 extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, LaneWidth, -side);
			Poly16Add(&poly, extraPoint);
		}
		if (rightSideDistance < LaneWidth - MinExtraDistance) 
		{
			V2 extraPoint = FromLeaveJunctionRoadCoord(junction, roadIndex, LaneWidth, +side);
			Poly16Add(&poly, extraPoint);
		}

		Poly16Add(&poly, rightIntersection);
	} 
	else if (junction->roadN >= 3) 
	{
		Road* road = junction->roads[roadIndex];
		V2 leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, roadIndex, side);
		V2 rightIntersection = GetRightRoadIntersectionAtJunction(junction, roadIndex, side);

		V2 leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, roadIndex, leftIntersection);
		V2 rightRoadCoord = ToLeaveJunctionRoadCoord(junction, roadIndex, rightIntersection);
	
		F32 leftSideDistance  = leftRoadCoord.x;
		F32 rightSideDistance = rightRoadCoord.x;

		Poly16Add(&poly, junction->position);
		Poly16Add(&poly, leftIntersection);

		if (Abs(leftSideDistance - rightSideDistance) >= MinExtraDistance) 
		{
			V2 extraPoint = {};
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

B32 IsPointOnJunction(V2 point, Junction* junction)
{
	B32 result = false;
	if (junction->roadN == 0) 
	{
		F32 x = junction->position.x;
		F32 y = junction->position.y;
		F32 left   = x - LaneWidth;
		F32 right  = x + LaneWidth;
		F32 top    = y - LaneWidth;
		F32 bottom = y + LaneWidth;
		result = ((left <= point.x && point.x <= right) && (top <= point.y && point.y <= bottom));
	} 
	else 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			if (IsPointInPoly(point, poly.points, poly.pointN)) 
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

B32 IsPointOnJunctionSidewalk(V2 point, Junction* junction)
{
	B32 isOnSidewalk = false;
	B32 isOnJunction = IsPointOnJunction(point, junction);
	if (isOnJunction) 
	{
		isOnSidewalk = false;
	} 
	else 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
			if (IsPointInPoly(point, poly.points, poly.pointN)) 
			{
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

static void UpdateTrafficLight(TrafficLight* trafficLight, F32 seconds)
{
	if (trafficLight->color == TrafficLightGreen) 
	{
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) 
		{
			trafficLight->timeLeft += TrafficLightYellowTime;
			trafficLight->color = TrafficLightYellow;
		}
	}
	else if (trafficLight->color == TrafficLightYellow) 
	{
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) 
		{
			trafficLight->color = TrafficLightRed;
		}
	}
}

static void DrawTrafficLight(Canvas canvas, TrafficLight* trafficLight)
{
	V4 drawColor = {};
	switch (trafficLight->color) 
	{
		case TrafficLightGreen:
			drawColor = {0.0f, 1.0f, 0.0f}; 
			break;
		case TrafficLightYellow:
			drawColor = {1.0f, 1.0f, 0.0f};
			break;
		case TrafficLightRed:
			drawColor = {1.0f, 0.0f, 0.0f};
			break;
		default:
			DebugBreak();
			break;
	}

	V2 position = trafficLight->position;
	F32 radius = TrafficLightRadius;
	DrawRect(
		canvas,
		position.x - radius, position.x + radius,
		position.y - radius, position.y + radius,
		drawColor
	);
}

void InitTrafficLights(Junction* junction)
{
	if (junction->roadN >= 3) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Road* road = junction->roads[i];
			TrafficLight* trafficLight = junction->trafficLights + i;
			F32 stopDistance = junction->stopDistances[i];
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

void UpdateTrafficLights(Junction* junction, F32 seconds)
{
	if (junction->roadN >= 3) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			TrafficLight* trafficLight = &junction->trafficLights[i];
			if (trafficLight->color != TrafficLightRed) 
			{
				UpdateTrafficLight(trafficLight, seconds);
				if (trafficLight->color == TrafficLightRed) 
				{
					I32 next = i + 1;
					if (next >= junction->roadN)
						next = 0;
					TrafficLight* nextTrafficLight = &junction->trafficLights[next];
					StartTrafficLight(nextTrafficLight);
				}		
			}
		}
	}
}

void DrawTrafficLights(Canvas canvas, Junction* junction)
{
	if (junction->roadN >= 3) 
	{
		for (I32 i = 0; i < junction->roadN; ++i)
			DrawTrafficLight(canvas, junction->trafficLights + i);
	}
}

void CalculateStopDistances(Junction* junction)
{
	Assert(junction->roadN >= 1);
	if (junction->roadN == 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i)
			junction->stopDistances[i] = LaneWidth;
	} 
	else if (junction->roadN == 2) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			F32 stopDistance = 0.0f;
			Road* road = junction->roads[i];
			V2 leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, i, LaneWidth);
			V2 rightIntersection = GetRightRoadIntersectionAtJunction(junction, i, LaneWidth);

			V2 leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, i, leftIntersection);
			V2 rightRoadCoord = ToLeaveJunctionRoadCoord(junction, i, rightIntersection);

			F32 leftSideDistance = leftRoadCoord.x;
			F32 rightSideDistance = rightRoadCoord.x;
			junction->stopDistances[i] = Max3(leftSideDistance, rightSideDistance, LaneWidth);
		}
	} 
	else 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			F32 stopDistance = 0.0f;
			Road* road = junction->roads[i];
			V2 rightIntersection = GetRightRoadIntersectionAtJunction(junction, i, LaneWidth);
			V2 leftIntersection  = GetLeftRoadIntersectionAtJunction(junction, i, LaneWidth);

			V2 leftRoadCoord  = ToLeaveJunctionRoadCoord(junction, i, leftIntersection);
			V2 rightRoadCoord = ToLeaveJunctionRoadCoord(junction, i, rightIntersection);
	
			F32 leftSideDistance  = leftRoadCoord.x;
			F32 rightSideDistance = rightRoadCoord.x;
			junction->stopDistances[i] = Max2(leftSideDistance, rightSideDistance);
		}
	}
}

void HighlightJunctionSidewalk(Canvas canvas, Junction* junction, V4 color)
{
	Assert(junction->roadN >= 1);
	for (I32 i = 0; i < junction->roadN; ++i) 
	{
		Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
		DrawPoly(canvas, poly.points, poly.pointN, color);
	}
	DrawJunction(canvas, junction);
}

void HighlightJunction(Canvas canvas, Junction* junction, V4 color)
{
	if (junction->roadN >= 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			DrawPoly(canvas, poly.points, poly.pointN, color);
		}
	} 
	else 
	{
		DrawJunctionPlaceholder(canvas, junction, color);
	}
}

void DrawJunctionSidewalk(Canvas canvas, Junction* junction)
{
	if (junction->roadN >= 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
			DrawPoly(canvas, poly.points, poly.pointN, SidewalkColor);
		}
	}
}

void DrawTexturedJunctionSidewalk(Canvas canvas, Junction* junction, Texture sidewalkTexture)
{
	if (junction->roadN >= 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth + SidewalkWidth);
			DrawWorldTexturePoly(canvas, poly.points, poly.pointN, sidewalkTexture);
		}
	}
}

static void DrawJunctionStripes(Canvas canvas, Junction* junction)
{
	if (junction->roadN <= 2) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Road* road = junction->roads[i];
			F32 stopDistance = junction->stopDistances[i];
			V2 roadEnterPoint = {};
			if (road->junction1 == junction)
				roadEnterPoint = FromRoadCoord1(road, +stopDistance, 0.0f);
			else if (road->junction2 == junction)
				roadEnterPoint = FromRoadCoord2(road, -stopDistance, 0.0f);
			else
				InvalidCodePath;
			DrawLine(canvas, junction->position, roadEnterPoint, RoadStripeColor, RoadStripeWidth);
		}
	} 
	else 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Road* road = junction->roads[i];
			F32 stopDistance = junction->stopDistances[i];
			V2 point1 = {};
			V2 point2 = {};
			if (road->junction1 == junction) 
			{
				point1 = FromRoadCoord1(road, +stopDistance, 0.0f);
				point2 = FromRoadCoord1(road, +stopDistance, -LaneWidth);
			} 
			else 
			{
				point1 = FromRoadCoord2(road, -stopDistance, 0.0f);
				point2 = FromRoadCoord2(road, -stopDistance, +LaneWidth);
			}
			DrawLine(canvas, point1, point2, RoadStripeColor, RoadStripeWidth);
		}
	}
}

static void DrawTexturedJunctionStripes(Canvas canvas, Junction* junction, Texture stripeTexture)
{
	if (junction->roadN <= 2) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Road* road = junction->roads[i];
			F32 stopDistance = junction->stopDistances[i];
			V2 roadEnterPoint = {};
			if (road->junction1 == junction)
				roadEnterPoint = FromRoadCoord1(road, +stopDistance, 0.0f);
			else if (road->junction2 == junction)
				roadEnterPoint = FromRoadCoord2(road, -stopDistance, 0.0f);
			else
				InvalidCodePath;

			DrawWorldTextureLine(canvas, junction->position, roadEnterPoint, RoadStripeWidth, stripeTexture);
		}
	} 
	else 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Road* road = junction->roads[i];
			F32 stopDistance = junction->stopDistances[i];
			V2 point1 = {};
			V2 point2 = {};
			if (road->junction1 == junction) 
			{
				point1 = FromRoadCoord1(road, +stopDistance, 0.0f);
				point2 = FromRoadCoord1(road, +stopDistance, -LaneWidth);
			}
			else 
			{
				point1 = FromRoadCoord2(road, -stopDistance, 0.0f);
				point2 = FromRoadCoord2(road, -stopDistance, +LaneWidth);
			}
			DrawWorldTextureLine(canvas, point1, point2, RoadStripeWidth, stripeTexture);
		}
	}
}

void DrawJunctionPlaceholder(Canvas canvas, Junction* junction, V4 color)
{
	V2 position = junction->position;
	F32 side = LaneWidth + SidewalkWidth;
	DrawRect(
		canvas,
		position.x - side, position.x + side,
		position.y - side, position.y + side,
		color
	);

	side = LaneWidth;
	DrawRect(
		canvas,
		position.x - side, position.x + side,
		position.y - side, position.y + side,
		color
	);
}

void DrawJunction(Canvas canvas, Junction* junction)
{
	if (junction->roadN >= 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			DrawPoly(canvas, poly.points, poly.pointN, RoadColor);
		}
		DrawJunctionStripes(canvas, junction);
	} 
	else 
	{
		DrawJunctionPlaceholder(canvas, junction, RoadColor);
	}
}

void DrawTexturedJunction(Canvas canvas, Junction* junction, Texture roadTexture, Texture stripeTexture)
{
	if (junction->roadN >= 1) 
	{
		for (I32 i = 0; i < junction->roadN; ++i) 
		{
			Poly16 poly = GetJunctionPolyForRoad(junction, i, LaneWidth);
			DrawWorldTexturePoly(canvas, poly.points, poly.pointN, roadTexture);
		}
		DrawTexturedJunctionStripes(canvas, junction, stripeTexture);
	} 
	else 
	{
		DrawJunctionPlaceholder(canvas, junction, RoadColor);
	}
}