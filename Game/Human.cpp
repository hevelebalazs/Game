#include "Human.h"

float Human::moveSpeed = 50.0f;

void Human::InitMovement() {
	float moveDistance = Point::CityDistance(moveStartPoint, moveEndPoint);

	moveTotalSeconds = (moveDistance / moveSpeed);
	moveSeconds = 0.0f;
}

void Human::MoveToBuilding(Building* building) {
	ClearPath(&movePath);

	// TODO: create functions to create these?
	MapElem targetElem = {};
	targetElem.type = MapElemType::BUILDING;
	targetElem.building = inBuilding;

	MapElem nextElem = {};
	nextElem.type = MapElemType::BUILDING;
	nextElem.building = building;

	movePath = ConnectElems(map, targetElem, nextElem, moveHelper);

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

void Human::Update(float seconds) {
	if (inBuilding && inBuilding->type == BuildingType::RED) {
		needRed -= 10.0f * seconds;
		if (needRed < 0.0f) needRed = 0.0f;
	}
	else {
		needRed += needRedSpeed * seconds;
		if (needRed > 100.0f) needRed = 100.0f;
	}

	if (inBuilding && inBuilding->type == BuildingType::GREEN) {
		needGreen -= 10.0f * seconds;
		if (needGreen < 0.0f) needGreen = 0.0f;
	}
	else {
		needGreen += needGreenSpeed * seconds;
		if (needGreen > 100.0f) needGreen = 100.0f;
	}

	if (inBuilding && inBuilding->type == BuildingType::BLUE) {
		needBlue -= 10.0f * seconds;
		if (needBlue < 0.0f) needBlue = 0.0f;
	}
	else {
		needBlue += needBlueSpeed * seconds;
		if (needBlue > 100.0f) needBlue = 100.0f;
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

					position = (moveRatio)* moveStartPoint + (1.0f - moveRatio) * moveEndPoint;
				}
			}
		}
	}
	else {
		Building* targetBuilding = 0;

		if (needRed == 100.0f) {
			targetBuilding = map->GetClosestBuilding(position, BuildingType::RED);
		}
		else if (needGreen == 100.0f) {
			targetBuilding = map->GetClosestBuilding(position, BuildingType::GREEN);
		}
		else if (needBlue == 100.0f) {
			targetBuilding = map->GetClosestBuilding(position, BuildingType::BLUE);
		}
		else if (needRed == 0.0f || needGreen == 0.0f || needBlue == 0.0f) {
			
			targetBuilding = map->GetRandomBuilding();
		}

		if (targetBuilding) MoveToBuilding(targetBuilding);
	}
}

void Human::Draw(Renderer renderer) {
	float radius = 5.0f;

	Color color = Color {
		needRed / 100.0f,
		needGreen / 100.0f,
		needBlue / 100.0f
	};

	renderer.DrawRect(
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		color
	);
}