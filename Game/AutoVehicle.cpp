// TODO: rename "Vehicle" to "Car" because it sounds cooler?
#include "AutoVehicle.h"
#include "Geometry.h"
#include "Memory.h"
#include "MapElem.h"

void InitAutoVehicleMovement(AutoVehicle* autoVehicle) {
	autoVehicle->moveBezier4 = TurnBezier4(autoVehicle->moveStartPoint, autoVehicle->moveEndPoint);

	// TODO: is this distance close enough?
	float moveDistance = Distance(autoVehicle->moveStartPoint.position, autoVehicle->moveEndPoint.position);

	autoVehicle->moveTotalSeconds = (moveDistance / autoVehicle->vehicle.maxSpeed);
	autoVehicle->moveSeconds = 0.0f;
}

void MoveAutoVehicleToBuilding(AutoVehicle* autoVehicle, Building* building, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	MapElem targetElem = BuildingElem(autoVehicle->inBuilding);
	MapElem nextElem = BuildingElem(building);

	autoVehicle->moveNode = ConnectElems(autoVehicle->vehicle.map, targetElem, nextElem, arena, tmpArena, pathPool);

	if (autoVehicle->moveNode) {
		autoVehicle->moveStartPoint = StartNodePoint(autoVehicle->moveNode);
		autoVehicle->moveEndPoint = NextNodePoint(autoVehicle->moveNode, autoVehicle->moveStartPoint);

		InitAutoVehicleMovement(autoVehicle);

		autoVehicle->moveTargetBuilding = building;
	}
}

void UpdateAutoVehicle(AutoVehicle* autoVehicle, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool) {
	Vehicle* vehicle = &autoVehicle->vehicle;

	if (autoVehicle->moveTargetBuilding) {
		// TODO: should there be a limit on the iteration number?
		while (seconds > 0.0f) {
			PathNode* moveNode = autoVehicle->moveNode;

			if (!moveNode) {
				autoVehicle->inBuilding = autoVehicle->moveTargetBuilding;
				autoVehicle->moveTargetBuilding = 0;
				break;
			}

			if (moveNode) {
				bool stop = false;

				if (IsNodeEndPoint(moveNode, autoVehicle->moveEndPoint)) {
					PathNode* nextNode = moveNode->next;

					if (nextNode) {
						MapElem moveElem = moveNode->elem;
						MapElem nextElem = nextNode->elem;

						if (moveElem.type == MapElemRoad && nextElem.type == MapElemIntersection) {
							Road* road = moveElem.road;
							Intersection* intersection = nextElem.intersection;
							TrafficLight* trafficLight = 0;

							if (intersection->leftRoad == road)
								trafficLight = &intersection->leftTrafficLight;
							else if (intersection->rightRoad == road)
								trafficLight = &intersection->rightTrafficLight;
							else if (intersection->topRoad == road)
								trafficLight = &intersection->topTrafficLight;
							else if (intersection->bottomRoad == road)
								trafficLight = &intersection->bottomTrafficLight;

							if (trafficLight && (trafficLight->color == TrafficLightRed || trafficLight->color == TrafficLightYellow))
								stop = true;
						}
					}
				}

				if (stop) {
					// TODO: use distance square here?
					float distanceLeft = Distance(vehicle->position, autoVehicle->moveEndPoint.position);

					// TODO: introduce a "stopDistance" variable?
					if (distanceLeft < vehicle->length * 0.5f)
						break;
				}

				autoVehicle->moveSeconds += seconds;

				if (autoVehicle->moveSeconds >= autoVehicle->moveTotalSeconds) {
					autoVehicle->moveStartPoint = autoVehicle->moveEndPoint;

					seconds = autoVehicle->moveSeconds - autoVehicle->moveTotalSeconds;

					if (IsNodeEndPoint(moveNode, autoVehicle->moveStartPoint)) {
						moveNode = moveNode->next;

						FreePathNode(autoVehicle->moveNode, pathPool);
						autoVehicle->moveNode = moveNode;

						if (!moveNode)
							continue;
					}
					else {
						autoVehicle->moveEndPoint = NextNodePoint(moveNode, autoVehicle->moveStartPoint);

						InitAutoVehicleMovement(autoVehicle);
					}
				}
				else {
					seconds = 0.0f;

					float moveRatio = (autoVehicle->moveSeconds / autoVehicle->moveTotalSeconds);

					DirectedPoint position = Bezier4DirectedPoint(autoVehicle->moveBezier4, moveRatio);
					MoveVehicle(vehicle, position);
				}
			}
		}
	}
	else {
		Building* targetBuilding = RandomBuilding(*vehicle->map);
		MoveAutoVehicleToBuilding(autoVehicle, targetBuilding, arena, tmpArena, pathPool);
	}
}