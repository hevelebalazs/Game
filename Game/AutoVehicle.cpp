// TODO: rename "Vehicle" to "Car" because it sounds cooler?
#include "AutoVehicle.h"
#include "Geometry.h"
#include "MapElem.h"

void AutoVehicle::InitMovement() {
	moveBezier4 = TurnBezier4(moveStartPoint, moveEndPoint);

	// TODO: is this distance close enough?
	float moveDistance = PointDistance(moveStartPoint.position, moveEndPoint.position);

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
				bool stop = false;

				if (moveNode->IsEndPoint(moveEndPoint)) {
					PathNode* nextNode = moveNode->next;

					if (nextNode) {
						MapElem moveElem = moveNode->elem;
						MapElem nextElem = nextNode->elem;

						if (moveElem.type == MapElemType::ROAD && nextElem.type == MapElemType::INTERSECTION) {
							Road* road = moveElem.road;
							Intersection* intersection = nextElem.intersection;
							TrafficLight* trafficLight = 0;

							if (intersection->leftRoad == road)        trafficLight = &intersection->leftTrafficLight;
							else if (intersection->rightRoad == road)  trafficLight = &intersection->rightTrafficLight;
							else if (intersection->topRoad == road)    trafficLight = &intersection->topTrafficLight;
							else if (intersection->bottomRoad == road) trafficLight = &intersection->bottomTrafficLight;

							if ((trafficLight && trafficLight->color == TrafficLight_Red) || 
								(trafficLight && trafficLight->color == TrafficLight_Yellow)
							) {
								stop = true;
							}
						}
					}
				}

				if (stop) {
					// TODO: use distance square here?
					float distanceLeft = PointDistance(vehicle.position, moveEndPoint.position);

					// TODO: introduce a "stopDistance" variable?
					if (distanceLeft < vehicle.length * 0.5f) {
						break;
					}
				}

				moveSeconds += seconds;

				if (moveSeconds >= moveTotalSeconds) {
					moveStartPoint = moveEndPoint;

					seconds = moveSeconds - moveTotalSeconds;

					if (moveNode->IsEndPoint(moveStartPoint)) {
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

					float moveRatio = (moveSeconds / moveTotalSeconds);

					DirectedPoint position = Bezier4DirectedPoint(moveBezier4, moveRatio);
					vehicle.MoveTo(position);
				}
			}
		}
	}
	else {
		Building* targetBuilding = vehicle.map->GetRandomBuilding();
		MoveToBuilding(targetBuilding);
	}
}