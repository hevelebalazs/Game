#pragma once

#include "Bitmap.hpp"
#include "Human.hpp"
#include "Intersection.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "Renderer.hpp"

struct AutoHuman {
	Human human;

	Intersection* onIntersection;

	PathNode* moveNode;
	Intersection* moveTargetIntersection;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;

	bool dead;
};

inline void InitAutoHumanMovement(AutoHuman* autoHuman) {
	Human* human = &autoHuman->human;

	float moveDistance = Distance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / human->moveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

inline void MoveAutoHumanToIntersection(AutoHuman* autoHuman, Intersection* intersection, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	Human* human = &autoHuman->human;

	MapElem targetElem = IntersectionSidewalkElem(autoHuman->onIntersection);
	MapElem nextElem = IntersectionSidewalkElem(intersection);

	autoHuman->moveNode = ConnectPedestrianElems(human->map, targetElem, human->position, nextElem,
												 arena, tmpArena, pathPool);

	if (autoHuman->moveNode) {
		// autoHuman->moveStartPoint = StartNodePoint(autoHuman->moveNode);
		autoHuman->moveStartPoint.position = human->position;
		DirectedPoint humanPosition = {};
		humanPosition.position = human->position;
		autoHuman->moveEndPoint = NextNodePoint(autoHuman->moveNode, humanPosition);

		InitAutoHumanMovement(autoHuman);

		autoHuman->moveTargetIntersection = intersection;
	}
}

inline void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	if (autoHuman->dead)
		return;

	Human* human = &autoHuman->human;

	if (autoHuman->moveTargetIntersection) {
		// TODO: should there be a limit on the interation number?
		while (seconds > 0.0f) {
			PathNode* moveNode = autoHuman->moveNode;

			if (!moveNode) {
				autoHuman->onIntersection = autoHuman->moveTargetIntersection;
				autoHuman->moveTargetIntersection = 0;
				break;
			}

			if (moveNode) {
				autoHuman->moveSeconds += seconds;

				if (autoHuman->moveSeconds >= autoHuman->moveTotalSeconds) {
					autoHuman->moveStartPoint = autoHuman->moveEndPoint;

					seconds = (autoHuman->moveSeconds - autoHuman->moveTotalSeconds);

					if (IsNodeEndPoint(moveNode, autoHuman->moveStartPoint)) {
						moveNode = moveNode->next;

						FreePathNode(autoHuman->moveNode, pathPool);
						autoHuman->moveNode = moveNode;

						if (!moveNode)
							continue;
					}
					else {
						autoHuman->moveEndPoint = NextNodePoint(moveNode, autoHuman->moveStartPoint);

						InitAutoHumanMovement(autoHuman);
					}
				}
				else {
					seconds = 0.0f;

					float endRatio = (autoHuman->moveSeconds / autoHuman->moveTotalSeconds);
					float startRatio = (1.0f - endRatio);

					DirectedPoint point = {};
					point.position = PointSum(
						PointProd(startRatio, autoHuman->moveStartPoint.position),
						PointProd(endRatio, autoHuman->moveEndPoint.position)
					);
					MoveHuman(human, point);
				}
			}
		}
	}
	else {
		Intersection* targetIntersection = RandomIntersection(*human->map);
		MoveAutoHumanToIntersection(autoHuman, targetIntersection, arena, tmpArena, pathPool);
	}
}

inline void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool) {
	autoHuman->dead = true;
	if (autoHuman->moveNode) {
		FreePath(autoHuman->moveNode, pathPool);
		autoHuman->moveNode = 0;
	}
}

// TODO: make this work with a Human instead of an AutoHuman
inline void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool) {
	Human* human = &autoHuman->human;
	if (human->healthPoints > 0) {
		human->healthPoints--;
		if (human->healthPoints == 0)
			KillAutoHuman(autoHuman, pathPool);
	}
}

inline void DrawAutoHuman(Renderer renderer, AutoHuman* autoHuman) {
	if (autoHuman->dead) {
		// TODO: this messes up other things that use random
		SeedRandom((int)autoHuman->human.position.x + (int)autoHuman->human.position.y);
		for (int i = 0; i < 5; ++i) {
			float x = autoHuman->human.position.x + RandomBetween(-2.0f, 2.0f);
			float y = autoHuman->human.position.y + RandomBetween(-2.0f, 2.0f);

			float side = 0.5f;
			float left   = x - side * 0.5f;
			float right  = x + side * 0.5f;
			float top    = y - side * 0.5f;
			float bottom = y + side * 0.5f;

			// TODO: make this a global variable?
			Color bloodColor = Color{1.0f, 0.0f, 0.0f};

			DrawRect(renderer, top, left, bottom, right, bloodColor);
		}
		InitRandom();
	}
	else {
		DrawHuman(renderer, autoHuman->human);
	}
}