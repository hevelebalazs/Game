#include "AutoHuman.h"

void AutoHuman::InitMovement() {
	float moveDistance = Point::CityDistance(moveStartPoint, moveEndPoint);

	moveTotalSeconds = (moveDistance / human.moveSpeed);
	moveSeconds = 0.0f;
}

void AutoHuman::MoveToBuilding(Building* building) {
	ClearPath(&movePath);

	// TODO: create functions to create these?
	MapElem targetElem = {};
	targetElem.type = MapElemType::BUILDING;
	targetElem.building = inBuilding;

	MapElem nextElem = {};
	nextElem.type = MapElemType::BUILDING;
	nextElem.building = building;

	movePath = ConnectElems(human.map, targetElem, nextElem, moveHelper);

	// TODO: can the path have 0 elements if the two buildings are the same?
	if (movePath.nodeCount == 0) {
		moveNode = 0;
	}
	else {
		moveNode = &movePath.nodes[0];

		moveStartPoint = moveNode->StartPoint();
		moveEndPoint = moveNode->NextPoint(moveStartPoint);

		InitMovement();

		moveTargetBuilding = building;
	}
}

void AutoHuman::Update(float seconds) {
	if (inBuilding && inBuilding->type == BuildingType_Red) {
		human.needRed -= 10.0f * seconds;
		if (human.needRed < 0.0f) human.needRed = 0.0f;
	}
	else {
		human.needRed += needRedSpeed * seconds;
		if (human.needRed > 100.0f) human.needRed = 100.0f;
	}

	if (inBuilding && inBuilding->type == BuildingType_Green) {
		human.needGreen -= 10.0f * seconds;
		if (human.needGreen < 0.0f) human.needGreen = 0.0f;
	}
	else {
		human.needGreen += needGreenSpeed * seconds;
		if (human.needGreen > 100.0f) human.needGreen = 100.0f;
	}

	if (inBuilding && inBuilding->type == BuildingType_Blue) {
		human.needBlue -= 10.0f * seconds;
		if (human.needBlue < 0.0f) human.needBlue = 0.0f;
	}
	else {
		human.needBlue += needBlueSpeed * seconds;
		if (human.needBlue > 100.0f) human.needBlue = 100.0f;
	}

	if (moveTargetBuilding) {
		// TODO: should there be a limit on the iteration number?
		while (seconds > 0.0f) {
			if (!moveNode) {
				inBuilding = moveTargetBuilding;
				moveTargetBuilding = 0;
				break;
			}

			if (moveNode) {
				moveSeconds += seconds;

				if (moveSeconds >= moveTotalSeconds) {
					seconds = moveSeconds - moveTotalSeconds;
					moveStartPoint = moveEndPoint;

					if (moveNode->IsEndPoint(moveEndPoint)) {
						moveNode = moveNode->next;

						if (!moveNode) continue;
					}
					else {
						moveEndPoint = moveNode->NextPoint(moveStartPoint);

						InitMovement();
					}
				}
				else {
					seconds = 0.0f;

					float moveRatio = 1.0f - (moveSeconds / moveTotalSeconds);

					human.position = (moveRatio)* moveStartPoint + (1.0f - moveRatio) * moveEndPoint;
				}
			}
		}
	}
	else {
		Building* targetBuilding = 0;

		if (human.needRed == 100.0f) {
			targetBuilding = human.map->GetClosestBuilding(human.position, BuildingType_Red);
		}
		else if (human.needGreen == 100.0f) {
			targetBuilding = human.map->GetClosestBuilding(human.position, BuildingType_Green);
		}
		else if (human.needBlue == 100.0f) {
			targetBuilding = human.map->GetClosestBuilding(human.position, BuildingType_Blue);
		}
		else if (human.needRed == 0.0f || human.needGreen == 0.0f || human.needBlue == 0.0f) {

			targetBuilding = human.map->GetRandomBuilding();
		}

		if (targetBuilding) MoveToBuilding(targetBuilding);
	}
}
