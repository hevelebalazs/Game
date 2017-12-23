#include "AutoHuman.h"
#include "Geometry.h"

static void InitAutoHumanMovement(AutoHuman* autoHuman) {
	float moveDistance = CityDistance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / humanMoveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

void MoveAutoHumanToBuilding(AutoHuman* autoHuman, Building* building) {
	ClearPath(&autoHuman->movePath);

	MapElem targetElem = BuildingElem(autoHuman->human.inBuilding);
	MapElem nextElem = BuildingElem(building);

	autoHuman->movePath = ConnectElems(autoHuman->human.map, targetElem, nextElem, autoHuman->moveHelper);

	if (autoHuman->movePath.nodeCount == 0) {
		autoHuman->moveNode = 0;
	}
	else {
		autoHuman->moveNode = &autoHuman->movePath.nodes[0];

		autoHuman->moveStartPoint = StartNodePoint(autoHuman->moveNode);
		autoHuman->moveEndPoint = NextNodePoint(autoHuman->moveNode, autoHuman->moveStartPoint);

		InitAutoHumanMovement(autoHuman);

		autoHuman->moveTargetBuilding = building;
	}
}

void UpdateAutoHuman(AutoHuman* autoHuman, float seconds) {
	Human* human = &autoHuman->human;

	if (human->inBuilding && human->inBuilding->type == BuildingRed) {
		human->needRed -= 10.0f * seconds;
		if (human->needRed < 0.0f) 
			human->needRed = 0.0f;
	}
	else {
		human->needRed += autoHuman->needRedSpeed * seconds;
		if (human->needRed > 100.0f) 
			human->needRed = 100.0f;
	}

	if (human->inBuilding && human->inBuilding->type == BuildingGreen) {
		human->needGreen -= 10.0f * seconds;
		if (human->needGreen < 0.0f) 
			human->needGreen = 0.0f;
	}
	else {
		human->needGreen += autoHuman->needGreenSpeed * seconds;
		if (human->needGreen > 100.0f) 
			human->needGreen = 100.0f;
	}

	if (human->inBuilding && human->inBuilding->type == BuildingBlue) {
		human->needBlue -= 10.0f * seconds;
		if (human->needBlue < 0.0f) 
			human->needBlue = 0.0f;
	}
	else {
		human->needBlue += autoHuman->needBlueSpeed * seconds;
		if (human->needBlue > 100.0f) 
			human->needBlue = 100.0f;
	}

	if (autoHuman->moveTargetBuilding) {
		PathNode* moveNode = autoHuman->moveNode;

		// TODO: should there be a limit on the iteration number?
		while (seconds > 0.0f) {
			if (!moveNode) {
				human->inBuilding = autoHuman->moveTargetBuilding;
				autoHuman->moveTargetBuilding = 0;
				break;
			}

			if (moveNode) {
				autoHuman->moveSeconds += seconds;

				if (autoHuman->moveSeconds >= autoHuman->moveTotalSeconds) {
					seconds = autoHuman->moveSeconds - autoHuman->moveTotalSeconds;
					autoHuman->moveStartPoint = autoHuman->moveEndPoint;

					if (IsNodeEndPoint(autoHuman->moveNode, autoHuman->moveEndPoint)) {
						autoHuman->moveNode = moveNode->next;
						moveNode = autoHuman->moveNode;

						if (!moveNode) 
							continue;
					}
					else {
						autoHuman->moveEndPoint = NextNodePoint(autoHuman->moveNode, autoHuman->moveStartPoint);

						InitAutoHumanMovement(autoHuman);
					}
				}
				else {
					seconds = 0.0f;

					float moveRatio = 1.0f - (autoHuman->moveSeconds / autoHuman->moveTotalSeconds);

					autoHuman->human.position = PointSum(
						PointProd(moveRatio, autoHuman->moveStartPoint.position), 
                        PointProd(1.0f - moveRatio, autoHuman->moveEndPoint.position)
					);
				}
			}
		}
	}
	else {
		Building* targetBuilding = 0;

		if (human->needRed == 100.0f)
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingRed);
		else if (human->needGreen == 100.0f)
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingGreen);
		else if (human->needBlue == 100.0f)
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingBlue);
		else if (human->needRed == 0.0f || human->needGreen == 0.0f || human->needBlue == 0.0f)
			targetBuilding = RandomBuilding(*human->map);

		if (targetBuilding) 
			MoveAutoHumanToBuilding(autoHuman, targetBuilding);
	}
}
