#pragma once

#include "Building.hpp"
#include "Draw.hpp"
#include "Map.hpp"
#include "Math.hpp"
#include "Path.hpp"
#include "Type.hpp"

#define HumanRadius			0.25f
#define HumanColor			MakeColor(1.0f, 0.0f, 0.0f)
#define MaxHealthPoints		3
#define HealthPointPadding	0.1f
#define	FullHealthColor		MakeColor(1.0f, 0.0f, 0.0f)
#define EmptyHealthColor	MakeColor(0.0f, 0.0f, 0.0f)

struct Human 
{
	Map* map;
	Building* inBuilding;

	B32 isPolice;

	V2 position;
	F32 moveSpeed;

	I32 healthPoints;
};

static B32 IsHumanCrossedByLine(Human* human, Line line)
{
	F32 left   = (human->position.x - HumanRadius);
	F32 right  = (human->position.x + HumanRadius);
	F32 top    = (human->position.y - HumanRadius);
	F32 bottom = (human->position.y + HumanRadius);

	if (line.x1 < left && line.x2 < left)
		return false;
	if (line.x1 > right && line.x2 > right)
		return false;
	if (line.y1 < top && line.y2 < top)
		return false;
	if (line.y1 > bottom && line.y2 > bottom)
		return false;

	V2 topLeft     = MakePoint(left, top);
	V2 topRight    = MakePoint(right, top);
	V2 bottomLeft  = MakePoint(left, bottom);
	V2 bottomRight = MakePoint(right, bottom);

	if (DoLinesCross(line.p1, line.p2, topLeft, topRight))
		return true;
	else if (DoLinesCross(line.p1, line.p2, topRight, bottomRight))
		return true;
	else if (DoLinesCross(line.p1, line.p2, bottomRight, bottomLeft))
		return true;
	else if (DoLinesCross(line.p1, line.p2, bottomLeft, topLeft))
		return true;
	else
		return false;
}

static void DrawPoliceRadius(Canvas canvas, Human* human, F32 radius)
{
	V4 color = MakeColor(0.0f, 0.0f, 1.0f);
	V2 position = human->position;
	F32 left   = position.x - radius;
	F32 right  = position.x + radius;
	F32 top    = position.y - radius;
	F32 bottom = position.y + radius;
	DrawRectOutline(canvas, top, left, bottom, right, color);
}

static void MoveHuman(Human* human, V4 point)
{
	human->position = point.position;
}

static void DrawHealthPoints(Canvas canvas, Human* human)
{
	I32 healthPoints = MaxHealthPoints;

	F32 healthPointRadius = (2.0f * HumanRadius + HealthPointPadding) / ((F32)MaxHealthPoints);
	healthPointRadius -= HealthPointPadding;

	F32 top    = human->position.y + HumanRadius + HealthPointPadding;
	F32 bottom = top + healthPointRadius;
	F32 left   = human->position.x - HumanRadius;
	for (I32 i = 0; i < MaxHealthPoints; ++i) 
	{
		F32 right = left + healthPointRadius;

		V4 color = {};
		if (i < human->healthPoints)
			color = FullHealthColor;
		else
			color = EmptyHealthColor;

		DrawRect(canvas, left, right, top, bottom, color);

		left = right + HealthPointPadding;
	}
}

static void DrawHuman(Canvas canvas, Human human)
{
	F32 radius = HumanRadius;
	V2 position = human.position;

	V4 color = {};
	if (human.healthPoints == 0)
		color = MakeColor(0.0f, 0.0f, 0.0f);
	else if (human.isPolice)
		color = MakeColor(0.0f, 0.0f, 1.0f);
	else
		color = MakeColor(1.0f, 1.0f, 1.0f);

	DrawRect(
		canvas,
		position.x - radius, position.x + radius,
		position.y - radius, position.y + radius,
		color
	);

	if (human.healthPoints < MaxHealthPoints)
		DrawHealthPoints(canvas, &human);
}