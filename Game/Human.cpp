#include "Human.h"

float Human::moveSpeed = 50.0f;

void Human::InitMovement() {
	float moveDistance = Point::CityDistance(moveStartPoint, moveEndPoint);

	moveTotalSeconds = (moveDistance / moveSpeed);
	moveSeconds = 0.0f;
}

void Human::Update(float seconds) {
	// TODO: should there be a limit on the iteration number?
	while (seconds > 0.0f) {
		if (!moveNode) {
			Building* nextBuilding = map->GetRandomBuilding();
			ClearPath(&movePath);

			// TODO: create functions to create these?
			MapElem targetElem = {};
			targetElem.type = MapElemType::BUILDING;
			targetElem.building = moveTargetBuilding;

			MapElem nextElem = {};
			nextElem.type = MapElemType::BUILDING;
			nextElem.building = nextBuilding;

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

				moveTargetBuilding = nextBuilding;
			}
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

				position = (moveRatio) * moveStartPoint + (1.0f - moveRatio) * moveEndPoint;
			}
		}
	}

	if (moveEndPoint.x == 0.0f && moveEndPoint.y == 0.0f) throw 1;
}

void Human::Draw(Renderer renderer) {
	float radius = 5.0f;
	Color color = Color{1.0f, 1.0f, 1.0f};

	renderer.DrawRect(
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		color
	);
}