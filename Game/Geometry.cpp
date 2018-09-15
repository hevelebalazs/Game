#include <math.h>

#include "Debug.hpp"
#include "Geometry.hpp"
#include "Type.hpp"

// TODO: create a type that always represents a unit vector?

void Poly16Add(Poly16* poly, V2 point)
{
	Assert(poly->pointN < 16);
	poly->points[poly->pointN] = point;
	poly->pointN++;
}

V2 MakePoint(F32 x, F32 y)
{
	V2 point = {};
	point.x = x;
	point.y = y;
	return point;
}

V2 MakeVector(F32 x, F32 y)
{
	V2 result = MakePoint(x, y);
	return result;
}

Quad MakeQuad(V2 point1, V2 point2, V2 point3, V2 point4)
{
	Quad quad = {};
	quad.points[0] = point1;
	quad.points[1] = point2;
	quad.points[2] = point3;
	quad.points[3] = point4;
	return quad;
}

F32 DistanceSquare(V2 point1, V2 point2)
{
	return (point1.x - point2.x) * (point1.x - point2.x) +
		(point1.y - point2.y) * (point1.y - point2.y);
}

F32 CityDistance(V2 point1, V2 point2)
{
	return fabsf(point1.x - point2.x) + fabsf(point1.y - point2.y);
}

F32 Distance(V2 point1, V2 point2)
{
	F32 dx = (point1.x - point2.x);
	F32 dy = (point1.y - point2.y);

	return sqrtf((dx * dx) + (dy * dy));
}

F32 VectorLength(V2 vector)
{
	return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

F32 VectorAngle(V2 vector)
{
	return atan2f(vector.y, vector.x);
}

V2 RotationVector(F32 angle)
{
	V2 result = MakePoint(cosf(angle), sinf(angle));
	return result;
}

V2 PointDirection(V2 startPoint, V2 endPoint)
{
	V2 vector = {endPoint.x - startPoint.x, endPoint.y - startPoint.y};
	vector = NormalVector(vector);

	return vector;
}

// TODO: can this be merged with LineIntersection?
B32 DoLinesCross(V2 line11, V2 line12, V2 line21, V2 line22)
{
	B32 right1 = TurnsRight(line11, line21, line22);
	B32 right2 = TurnsRight(line12, line21, line22);
	if (right1 == right2) 
		return false;

	B32 right3 = TurnsRight(line21, line11, line12);
	B32 right4 = TurnsRight(line22, line11, line12);
	if (right3 == right4) 
		return false;

	return true;
}

static F32 Determinant(F32 a, F32 b, F32 c, F32 d)
{
	return (a * d) - (b * c);
}

V2 LineIntersection(V2 line11, V2 line12, V2 line21, V2 line22)
{
	F32 det1  = Determinant(line11.x, line11.y, line12.x, line12.y);
	F32 detX1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	F32 detY1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	F32 det2  = Determinant(line21.x, line21.y, line22.x, line22.y);
	F32 detX2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	F32 detY2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	F32 detXUp = Determinant(det1, detX1, det2, detX2);
	F32 detYUp = Determinant(det1, detY1, det2, detY2);

	detX1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	detY1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	detX2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	detY2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	F32 detDown = Determinant(detX1, detY1, detX2, detY2);

	Assert(detDown != 0.0f);

	V2 result = (1.0f / detDown) * MakePoint(detXUp, detYUp);
	return result;
}

V2 LineIntersection(Line line1, Line line2)
{
	V2 intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

B32 IsPointInPoly(V2 point, V2* points, I32 pointN)
{
	B32 isInside = false;
	if (pointN >= 3) 
	{
		isInside = true;
		I32 prev = pointN - 1;
		for (I32 i = 0; i < pointN; ++i) 
		{
			if (!TurnsRight(points[prev], points[i], point)) 
			{
				isInside = false;
				break;
			}
			prev = i;
		}
	}
	return isInside;
}

F32 DotProduct(V2 vector1, V2 vector2)
{
	return ((vector1.x * vector2.x) + (vector1.y * vector2.y));
}

V2 NormalVector(V2 vector)
{
	F32 length = VectorLength(vector);

	if (length == 0.0f) 
		return vector;

	V2 result = (1.0f / length) * vector;
	return result;
}

// TODO: create a version of this where base is unit length?
V2 ParallelVector(V2 vector, V2 base)
{
	V2 unitBase = NormalVector(base);
	V2 result = DotProduct(vector, unitBase) * unitBase;
	return result;
}