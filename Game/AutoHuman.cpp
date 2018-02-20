#include "AutoHuman.h"

void InitAutoHumanMovement(AutoHuman* autoHuman) {
	Human* human = &autoHuman->human;

	float moveDistance = Distance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / human->moveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

void MoveAutoHumanToIntersection(AutoHuman* autoHuman, Intersection* intersection, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
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

void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
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

void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool) {
	autoHuman->dead = true;
	if (autoHuman->moveNode) {
		FreePath(autoHuman->moveNode, pathPool);
		autoHuman->moveNode = 0;
	}
}

// TODO: make this work with a Human instead of an AutoHuman
void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool) {
	Human* human = &autoHuman->human;
	if (human->healthPoints > 0) {
		human->healthPoints--;
		if (human->healthPoints == 0)
			KillAutoHuman(autoHuman, pathPool);
	}
}