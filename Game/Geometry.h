#pragma once

#include "Point.h"

float PointDistance(Point point1, Point point2);

// TODO: is creating a Line struct a good idea?
bool TurnsRight(Point point1, Point point2, Point point3);
bool DoLinesCross(Point line11, Point line12, Point line21, Point line22);
Point LineIntersection(Point line11, Point line12, Point line21, Point line22);

float DotProduct(Point vector1, Point vector2);
Point NormalVector(Point vector);
Point ParallelVector(Point vector, Point base);