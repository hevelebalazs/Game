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

B32 IsHumanCrossedByLine(Human* human, Line line);
void DrawPoliceRadius(Canvas canvas, Human* human, F32 radius);

void MoveHuman(Human* human, V4 point);
void DrawHuman(Canvas canvas, Human human);
