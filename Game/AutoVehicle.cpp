// TODO: rename "Vehicle" to "Car" because it sounds cooler?
#include "AutoVehicle.h"
#include "MapElem.h"

void AutoVehicle::InitMovement() {
	float moveDistance = Point::CityDistance(moveStartPoint, moveEndPoint);

	moveTotalSeconds = (moveDistance / vehicle.maxSpeed);
	moveSeconds = 0.0f;
}

void AutoVehicle::MoveToBuilding(Building* building) {
	ClearPath(&movePath);

	// TODO: create functions to create these?
	MapElem targetElem = {};
	targetElem.type = MapElemType::BUILDING;
	targetElem.building = inBuilding;

	MapElem nextElem = {};
	nextElem.type = MapElemType::BUILDING;
	nextElem.building = building;

	movePath = ConnectElems(vehicle.map, targetElem, nextElem, moveHelper);

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

void AutoVehicle::Update(float seconds) {
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
						PathNode* nextNode = moveNode->next;

						if (nextNode) {
							MapElem nextElem = nextNode->elem;
							MapElem moveElem = moveNode->elem;

							if (moveElem.type == MapElemType::ROAD && nextElem.type == MapElemType::INTERSECTION) {
								Road* road = moveElem.road;
								Intersection* intersection = nextElem.intersection;
								TrafficLight* trafficLight = 0;

								if (intersection->leftRoad == road)        trafficLight = &intersection->leftTrafficLight;
								else if (intersection->rightRoad == road)  trafficLight = &intersection->rightTrafficLight;
								else if (intersection->topRoad == road)    trafficLight = &intersection->topTrafficLight;
								else if (intersection->bottomRoad == road) trafficLight = &intersection->bottomTrafficLight;

								if (trafficLight && trafficLight->color == TrafficLight_Red) {
									moveSeconds = 0.0f;
									moveTotalSeconds = 0.0f;
									break;
								}
							}
						}

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

					vehicle.position = (moveRatio)* moveStartPoint + (1.0f - moveRatio) * moveEndPoint;
				}
			}
		}
	}
	else {
		Building* targetBuilding = vehicle.map->GetRandomBuilding();
		MoveToBuilding(targetBuilding);
	}
}