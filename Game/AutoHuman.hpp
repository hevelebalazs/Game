#pragma once

#include "Draw.hpp"
#include "Human.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Type.hpp"

struct AutoHuman
{
	Human human;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	V4 moveStartPoint;
	V4 moveEndPoint;
	F32 moveTotalSeconds;
	F32 moveSeconds;

	B32 dead;
};

static void func InitAutoHumanMovement(AutoHuman* autoHuman)
{
	Human* human = &autoHuman->human;

	F32 moveDistance = Distance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / human->moveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

static void func MoveAutoHumanToJunction(AutoHuman* autoHuman, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	Human* human = &autoHuman->human;

	MapElem targetElem = GetJunctionSidewalkElem(autoHuman->onJunction);
	MapElem nextElem = GetJunctionSidewalkElem(junction);
	I32 startCornerIndex = GetClosestJunctionCornerIndex(autoHuman->onJunction, autoHuman->human.position);
	I32 endCornerIndex = GetRandomJunctionCornerIndex(junction);

	autoHuman->moveNode = ConnectPedestrianElems(human->map, targetElem, startCornerIndex, nextElem, endCornerIndex,
												 tmpArena, pathPool);

	if (autoHuman->moveNode)
	{
		// autoHuman->moveStartPoint = StartNodePoint(autoHuman->moveNode);
		autoHuman->moveStartPoint.position = human->position;
		V4 humanPosition = {};
		humanPosition.position = human->position;
		autoHuman->moveEndPoint = NextNodePoint(autoHuman->moveNode, humanPosition);

		InitAutoHumanMovement(autoHuman);

		autoHuman->moveTargetJunction = junction;
	}
}

static void func UpdateAutoHuman(AutoHuman* autoHuman, F32 seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	if (autoHuman->dead)
	{
		return;
	}

	Human* human = &autoHuman->human;

	if (autoHuman->moveTargetJunction)
	{
		// TODO: should there be a limit on the interation number?
		while (seconds > 0.0f)
		{
			PathNode* moveNode = autoHuman->moveNode;

			if (!moveNode)
			{
				autoHuman->onJunction = autoHuman->moveTargetJunction;
				autoHuman->moveTargetJunction = 0;
				break;
			}

			if (moveNode)
			{
				autoHuman->moveSeconds += seconds;

				if (autoHuman->moveSeconds >= autoHuman->moveTotalSeconds)
				{
					autoHuman->moveStartPoint = autoHuman->moveEndPoint;

					seconds = (autoHuman->moveSeconds - autoHuman->moveTotalSeconds);

					if (IsNodeEndPoint(moveNode, autoHuman->moveStartPoint))
					{
						moveNode = moveNode->next;

						FreePathNode(autoHuman->moveNode, pathPool);
						autoHuman->moveNode = moveNode;

						if (!moveNode)
						{
							continue;
						}
					}
					else
					{
						autoHuman->moveEndPoint = NextNodePoint(moveNode, autoHuman->moveStartPoint);

						InitAutoHumanMovement(autoHuman);
					}
				}
				else
				{
					seconds = 0.0f;

					F32 endRatio = (autoHuman->moveSeconds / autoHuman->moveTotalSeconds);
					F32 startRatio = (1.0f - endRatio);

					V4 point = {};
					point.position = (startRatio * autoHuman->moveStartPoint.position) +
									 (endRatio * autoHuman->moveEndPoint.position);

					MoveHuman(human, point);
				}
			}
		}
	} 
	else
	{
		Junction* targetJunction = GetRandomJunction(human->map);
		MoveAutoHumanToJunction(autoHuman, targetJunction, arena, tmpArena, pathPool);
	}
}

static void func KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool)
{
	autoHuman->dead = true;
	if (autoHuman->moveNode)
	{
		FreePath(autoHuman->moveNode, pathPool);
		autoHuman->moveNode = 0;
	}
}

// TODO: make this work with a Human instead of an AutoHuman
static void func DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool)
{
	Human* human = &autoHuman->human;
	if (human->healthPoints > 0)
	{
		human->healthPoints--;
		if (human->healthPoints == 0)
		{
			KillAutoHuman(autoHuman, pathPool);
		}
	}
}

static void func DrawAutoHuman(Canvas* canvas, AutoHuman* autoHuman) 
{
	if (autoHuman->dead) 
	{
		// TODO: this messes up other things that use random
		SeedRandom((I32)autoHuman->human.position.x + (I32)autoHuman->human.position.y);
		for (I32 i = 0; i < 5; ++i) 
		{
			F32 x = autoHuman->human.position.x + RandomBetween(-2.0f, 2.0f);
			F32 y = autoHuman->human.position.y + RandomBetween(-2.0f, 2.0f);

			F32 side = 0.5f;
			F32 left   = x - side * 0.5f;
			F32 right  = x + side * 0.5f;
			F32 top    = y - side * 0.5f;
			F32 bottom = y + side * 0.5f;

			// TODO: make this a global variable?
			V4 bloodColor = MakeColor(1.0f, 0.0f, 0.0f);
			DrawRectLRTB(canvas, left, right, top, bottom, bloodColor);
		}
		InitRandom();
	} 
	else 
	{
		DrawHuman(canvas, autoHuman->human);
	}
}
