#include "AutoHuman.hpp"
#include "Type.hpp"

void InitAutoHumanMovement(AutoHuman* autoHuman)
{
	Human* human = &autoHuman->human;

	F32 moveDistance = Distance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / human->moveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

void MoveAutoHumanToJunction(AutoHuman* autoHuman, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	Human* human = &autoHuman->human;

	MapElem targetElem = GetJunctionSidewalkElem(autoHuman->onJunction);
	MapElem nextElem = GetJunctionSidewalkElem(junction);
	I32 startCornerIndex = GetClosestJunctionCornerIndex(autoHuman->onJunction, autoHuman->human.position);
	I32 endCornerIndex = GetRandomJunctionCornerIndex(junction);

	autoHuman->moveNode = ConnectPedestrianElems(human->map, targetElem, startCornerIndex, nextElem, endCornerIndex,
												 tmpArena, pathPool);

	if (autoHuman->moveNode) {
		// autoHuman->moveStartPoint = StartNodePoint(autoHuman->moveNode);
		autoHuman->moveStartPoint.position = human->position;
		V4 humanPosition = {};
		humanPosition.position = human->position;
		autoHuman->moveEndPoint = NextNodePoint(autoHuman->moveNode, humanPosition);

		InitAutoHumanMovement(autoHuman);

		autoHuman->moveTargetJunction = junction;
	}
}

void UpdateAutoHuman(AutoHuman* autoHuman, F32 seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	if (autoHuman->dead)
		return;

	Human* human = &autoHuman->human;

	if (autoHuman->moveTargetJunction) {
		// TODO: should there be a limit on the interation number?
		while (seconds > 0.0f) {
			PathNode* moveNode = autoHuman->moveNode;

			if (!moveNode) {
				autoHuman->onJunction = autoHuman->moveTargetJunction;
				autoHuman->moveTargetJunction = 0;
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
					} else {
						autoHuman->moveEndPoint = NextNodePoint(moveNode, autoHuman->moveStartPoint);

						InitAutoHumanMovement(autoHuman);
					}
				} else {
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
	} else {
		Junction* targetJunction = GetRandomJunction(human->map);
		MoveAutoHumanToJunction(autoHuman, targetJunction, arena, tmpArena, pathPool);
	}
}

void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool)
{
	autoHuman->dead = true;
	if (autoHuman->moveNode) {
		FreePath(autoHuman->moveNode, pathPool);
		autoHuman->moveNode = 0;
	}
}

// TODO: make this work with a Human instead of an AutoHuman
void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool)
{
	Human* human = &autoHuman->human;
	if (human->healthPoints > 0) {
		human->healthPoints--;
		if (human->healthPoints == 0)
			KillAutoHuman(autoHuman, pathPool);
	}
}