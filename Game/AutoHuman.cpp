#include "AutoHuman.h"
#include "Geometry.h"

static void InitAutoHumanMovement(AutoHuman* autoHuman) {
	float moveDistance = CityDistance(autoHuman->moveStartPoint.position, autoHuman->moveEndPoint.position);

	autoHuman->moveTotalSeconds = (moveDistance / autoHuman->human.moveSpeed);
	autoHuman->moveSeconds = 0.0f;
}

void MoveAutoHumanToBuilding(AutoHuman* autoHuman, Building* building) {
	ClearPath(&autoHuman->movePath);

	// TODO: create functions to create these?
	MapElem targetElem = {};
	targetElem.type = MapElemBuilding;
	targetElem.building = autoHuman->inBuilding;

	MapElem nextElem = {};
	nextElem.type = MapElemBuilding;
	nextElem.building = building;

	autoHuman->movePath = ConnectElems(autoHuman->human.map, targetElem, nextElem, autoHuman->moveHelper);

	// TODO: can the path have 0 elements if the two buildings are the same?
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

	if (autoHuman->inBuilding && autoHuman->inBuilding->type == BuildingType_Red) {
		human->needRed -= 10.0f * seconds;
		if (human->needRed < 0.0f) human->needRed = 0.0f;
	}
	else {
		human->needRed += autoHuman->needRedSpeed * seconds;
		if (human->needRed > 100.0f) human->needRed = 100.0f;
	}

	if (autoHuman->inBuilding && autoHuman->inBuilding->type == BuildingType_Green) {
		human->needGreen -= 10.0f * seconds;
		if (human->needGreen < 0.0f) human->needGreen = 0.0f;
	}
	else {
		human->needGreen += autoHuman->needGreenSpeed * seconds;
		if (human->needGreen > 100.0f) human->needGreen = 100.0f;
	}

	if (autoHuman->inBuilding && autoHuman->inBuilding->type == BuildingType_Blue) {
		human->needBlue -= 10.0f * seconds;
		if (human->needBlue < 0.0f) human->needBlue = 0.0f;
	}
	else {
		human->needBlue += autoHuman->needBlueSpeed * seconds;
		if (human->needBlue > 100.0f) human->needBlue = 100.0f;
	}

	if (autoHuman->moveTargetBuilding) {
		PathNode* moveNode = autoHuman->moveNode;

		// TODO: should there be a limit on the iteration number?
		while (seconds > 0.0f) {
			if (!moveNode) {
				autoHuman->inBuilding = autoHuman->moveTargetBuilding;
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

						if (!moveNode) continue;
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

		if (human->needRed == 100.0f) {
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingType_Red);
		}
		else if (human->needGreen == 100.0f) {
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingType_Green);
		}
		else if (human->needBlue == 100.0f) {
			targetBuilding = ClosestBuilding(*human->map, human->position, BuildingType_Blue);
		}
		else if (human->needRed == 0.0f || human->needGreen == 0.0f || human->needBlue == 0.0f) {

			targetBuilding = RandomBuilding(*human->map);
		}

		if (targetBuilding) MoveAutoHumanToBuilding(autoHuman, targetBuilding);
	}
}
