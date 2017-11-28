#include "Intersection.h"

float TrafficLight::radius = 2.0f;
float TrafficLight::switchTime = 3.0f;

void TrafficLight::Start() {
	color = TrafficLight_Green;
	timeLeft = TrafficLight::switchTime;
}

void TrafficLight::Update(float seconds) {
	if (color == TrafficLight_Green) {
		timeLeft -= seconds;

		if (timeLeft < 0.0f) color = TrafficLight_Red;
	}
}

void TrafficLight::Draw(Renderer renderer) {
	Color drawColor = {};

	if (color == TrafficLight_Green)    drawColor = {0.0f, 1.0f, 0.0f};
	else if (color == TrafficLight_Red) drawColor = {1.0f, 0.0f, 0.0f};

	renderer.DrawRect(
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		drawColor
	);
}

void Intersection::InitTrafficLights() {
	int roadCount = 0;
	if (leftRoad)   roadCount++;
	if (rightRoad)  roadCount++;
	if (topRoad)    roadCount++;
	if (bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	float roadWidth = this->GetRoadWidth();

	if (leftRoad)   leftTrafficLight.position   = {coordinate.x - roadWidth * 0.5f,  coordinate.y + roadWidth * 0.25f};
	if (rightRoad)  rightTrafficLight.position  = {coordinate.x + roadWidth * 0.5f,  coordinate.y - roadWidth * 0.25f};
	if (topRoad)    topTrafficLight.position    = {coordinate.x - roadWidth * 0.25f, coordinate.y - roadWidth * 0.5f};
	if (bottomRoad) bottomTrafficLight.position = {coordinate.x + roadWidth * 0.25f, coordinate.y + roadWidth * 0.5f};

	if (leftRoad)   leftTrafficLight.color   = TrafficLight_Red;
	if (rightRoad)  rightTrafficLight.color  = TrafficLight_Red;
	if (topRoad)    topTrafficLight.color    = TrafficLight_Red;
	if (bottomRoad) bottomTrafficLight.color = TrafficLight_Red;

	if (leftRoad)        leftTrafficLight.Start();
	else if (topRoad)    topTrafficLight.Start();
	else if (rightRoad)  rightTrafficLight.Start();
	else if (bottomRoad) bottomTrafficLight.Start();
}

void Intersection::UpdateTrafficLights(float seconds) {
	int roadCount = 0;
	if (leftRoad)   roadCount++;
	if (rightRoad)  roadCount++;
	if (topRoad)    roadCount++;
	if (bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	// TODO: this is total nuts, update this to arrays as soon as possible!
	if (leftRoad && leftTrafficLight.color == TrafficLight_Green) {
		leftTrafficLight.Update(seconds);
		if (leftTrafficLight.color == TrafficLight_Red) {
			if (topRoad)         topTrafficLight.Start();
			else if (rightRoad)  rightTrafficLight.Start();
			else if (bottomRoad) bottomTrafficLight.Start();
		}
	}
	else if (topRoad && topTrafficLight.color == TrafficLight_Green) {
		topTrafficLight.Update(seconds);
		if (topTrafficLight.color == TrafficLight_Red) {
			if (rightRoad)       rightTrafficLight.Start();
			else if (bottomRoad) bottomTrafficLight.Start();
			else if (leftRoad)   leftTrafficLight.Start();
		}
	}
	else if (rightRoad && rightTrafficLight.color == TrafficLight_Green) {
		rightTrafficLight.Update(seconds);
		if (rightTrafficLight.color == TrafficLight_Red) {
			if (bottomRoad)    bottomTrafficLight.Start();
			else if (leftRoad) leftTrafficLight.Start();
			else if (topRoad)  topTrafficLight.Start();
		}
	}
	else if (bottomRoad && bottomTrafficLight.color == TrafficLight_Green) {
		bottomTrafficLight.Update(seconds);
		if (bottomTrafficLight.color == TrafficLight_Red) {
			if (leftRoad)       leftTrafficLight.Start();
			else if (topRoad)   topTrafficLight.Start();
			else if (rightRoad) rightTrafficLight.Start();
		}
	}
}

void Intersection::DrawTrafficLights(Renderer renderer) {
	int roadCount = 0;
	if (leftRoad)   roadCount++;
	if (rightRoad)  roadCount++;
	if (topRoad)    roadCount++;
	if (bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	if (leftRoad)   leftTrafficLight.Draw(renderer);
	if (rightRoad)  rightTrafficLight.Draw(renderer);
	if (topRoad)    topTrafficLight.Draw(renderer);
	if (bottomRoad) bottomTrafficLight.Draw(renderer);
}

float Intersection::GetRoadWidth() {
	if (leftRoad) return leftRoad->width;
	if (rightRoad) return rightRoad->width;
	if (topRoad) return topRoad->width;
	if (bottomRoad) return bottomRoad->width;

	return 0.0f;
}

void Intersection::Highlight(Renderer renderer, Color color) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth = GetRoadWidth();

		float stripeWidth = roadWidth / 10.0f;
		float top = coordinate.y - (roadWidth / 2.0f);
		float bottom = coordinate.y + (roadWidth / 2.0f);
		float left = coordinate.x - (roadWidth / 2.0f);
		float right = coordinate.x + (roadWidth / 2.0f);

		renderer.DrawRect(top, left, bottom, right, color);
	}
}

void Intersection::Draw(Renderer renderer) {
	float roadWidth = GetRoadWidth();

	float top = coordinate.y - (roadWidth / 2.0f);
	float bottom = coordinate.y + (roadWidth / 2.0f);
	float left = coordinate.x - (roadWidth / 2.0f);
	float right = coordinate.x + (roadWidth / 2.0f);

	float midX = (left + right) / 2;
	float midY = (top + bottom) / 2;

	Color color = { 0.5f, 0.5f, 0.5f };
	renderer.DrawRect(top, left, bottom, right, color);

	float stripeWidth = roadWidth / 20.0f;
	Color stripeColor = { 1.0f, 1.0f, 1.0f };

	int roadCount = 0;

	if (topRoad) roadCount++;
	if (leftRoad) roadCount++;
	if (bottomRoad) roadCount++;
	if (rightRoad) roadCount++;

	if (roadCount > 2) {
		if (topRoad) {
			renderer.DrawRect(top, left, top + stripeWidth, right, stripeColor);
		}
		if (leftRoad) {
			renderer.DrawRect(top, left, bottom, left + stripeWidth, stripeColor);
		}
		if (bottomRoad) {
			renderer.DrawRect(bottom - stripeWidth, left, bottom, right, stripeColor);
		}
		if (rightRoad) {
			renderer.DrawRect(top, right - stripeWidth, bottom, right, stripeColor);
		}
	}
	else {
		if (topRoad) {
			renderer.DrawRect(top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (leftRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (bottomRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (rightRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
		}
	}
}