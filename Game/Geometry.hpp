#pragma once

#include "Math.hpp"
#include "Type.hpp"

union Line 
{
	struct
	{
		V2 p1;
		V2 p2;
	};

	struct 
	{
		F32 x1, y1;
		F32 x2, y2;
	};
};

struct Poly16 
{
	V2 points[16];
	I32 pointN;
};

void Poly16Add(Poly16* poly, V2 point);

struct Quad 
{
	V2 points[4];
};

V2 MakePoint(F32 x, F32 y);
V2 MakeVector(F32 x, F32 y);
Quad MakeQuad(V2 point1, V2 point2, V2 point3, V2 point4);

F32 DistanceSquare(V2 point1, V2 point2);
F32 CityDistance(V2 point1, V2 point2);
F32 Distance(V2 point1, V2 point2);

F32 VectorLength(V2 vector);
F32 VectorAngle(V2 vector);

F32 LineAngle(V2 startPoint, V2 endPoint);
F32 NormalizeAngle(F32 angle);
B32 IsAngleBetween(F32 minAngle, F32 angle, F32 maxAngle);

V2 RotationVector(F32 angle);
V2 PointDirection(V2 startPoint, V2 endPoint);
B32 TurnsRight(V2 point1, V2 point2, V2 point3);

B32 DoLinesCross(V2 line11, V2 line12, V2 line21, V2 line22);
V2 LineIntersection(V2 line11, V2 line12, V2 line21, V2 line22);
V2 LineIntersection(Line line1, Line line2);

F32 DotProduct(V2 vector1, V2 vector2);
V2 NormalVector(V2 vector);
V2 ParallelVector(V2 vector, V2 base);

B32 IsPointInPoly(V2 point, V2* points, I32 pointN);
B32 IsPointInQuad(Quad quad, V2 point);

V2 TurnVectorToRight(V2 vector);
V2 XYToBase(V2 point, V2 baseUnit);
