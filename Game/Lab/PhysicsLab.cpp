#include "PhysicsLab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"

#define CarCornerMass 250000
#define CarMass (4 * CarCornerMass)

#define CarWidth  6.0f
#define CarLength 10.0f

struct CarState {
	V2 position;
	V2 velocity;
	V2 acceleration;

	F32 angle;
	F32 angularVelocity;
	F32 angularAcceleration;
	F32 inertia;

	F32 engineForce;
	F32 turnInput;
};

struct PhysicsLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	V2 walls[4];
	I32 wallN;

	CarState car;
};
PhysicsLabState gPhysicsLabState;

struct CollisionInfo {
	I32 count;
	V2 point;
	V2 normalVector;
};

CollisionInfo operator+(CollisionInfo hit1, CollisionInfo hit2)
{
	CollisionInfo hit = {};
	hit.count = hit1.count + hit2.count;
	if (hit2.count > 0) {
		hit.point = hit2.point;
		hit.normalVector = hit2.normalVector;
	} else {
		hit.point = hit1.point;
		hit.normalVector = hit1.normalVector;
	}
	return hit;
}

static V2 GetMousePosition(Camera* camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	V2 point = {};
	point.x = (F32)cursorPoint.x;
	point.y = (F32)cursorPoint.y;

	point = PixelToUnit(camera, point);

	return point;
}

static void PhysicsLabResize(PhysicsLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 10.0f;
}

static void PhysicsLabBlit(Canvas canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas.bitmap;
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
		          &bitmap.info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static V2 GetCorner(CarState* car, I32 cornerIndex)
{
	Assert(IsIntBetween(cornerIndex, 0, 3));

	F32 halfWidth = CarWidth * 0.5f;
	F32 halfLength = CarLength * 0.5f;

	F32 angle = car->angle;
	V2 rotationUpDown = RotationVector(angle);
	V2 rotationLeftRight = RotationVector(angle + PI * 0.5f);
	V2 center = car->position;
	V2 frontCenter = center + (halfLength * rotationUpDown);
	V2 backCenter = center - (halfLength * rotationUpDown);

	V2 result = {};
	switch (cornerIndex) {
		case 0:
			result = frontCenter - (halfWidth * rotationLeftRight);
			break;
		case 1:
			result = frontCenter + (halfWidth * rotationLeftRight);
			break;
		case 2:
			result = backCenter  + (halfWidth * rotationLeftRight);
			break;
		case 3:
			result = backCenter  - (halfWidth * rotationLeftRight);
			break;
		default:
			DebugBreak();
			break;
	}
	return result;
}

static V2 GetCarPointVelocity(CarState* car, V2 point)
{
	V2 velocity = car->velocity;
	V2 pointVector = (point - car->position);
	V2 turnVelocity = car->angularVelocity * TurnVectorToRight(pointVector);
	velocity = velocity + turnVelocity;
	return velocity;
}

static V2 GetCarRelativeCoordinates(CarState* car, V2 point)
{
	V2 result = {};
	V2 pointVector = (point - car->position);

	V2 frontDirection = RotationVector(car->angle);
	result.x = DotProduct(pointVector, frontDirection);

	V2 sideDirection = TurnVectorToRight(frontDirection);
	result.y = DotProduct(pointVector, sideDirection);
	return result;
}

static LRESULT CALLBACK PhysicsLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	PhysicsLabState* labState = &gPhysicsLabState;
	CarState* car = &labState->car;
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			PhysicsLabResize(labState, width, height);
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			PhysicsLabBlit(labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}
		case WM_SETCURSOR: {
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case 'W':
					car->engineForce = 1.0f;
					break;
				case 'S':
					car->engineForce = -1.0f;
					break;
				case 'A':
					car->turnInput = -0.1f;
					break;
				case 'D':
					car->turnInput = +0.1f;
					break;
			}
			break;
		}
		case WM_KEYUP: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case 'W':
				case 'S':
					car->engineForce = 0.0f;
					break;
				case 'A':
				case 'D':
					car->turnInput = 0.0f;
					break;
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE: {
			labState->running = false;
			break;
		}
		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}
	return result;
}

static void PhysicsLabInit(PhysicsLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;

	labState->wallN = 4;
	labState->walls[0] = MakePoint(10.0f,   0.0f);
	labState->walls[1] = MakePoint(60.0f,  -100.0f);
	labState->walls[2] = MakePoint(30.0f,  -25.0f);
	labState->walls[3] = MakePoint(110.0f, -50.0f);

	CarState* car = &labState->car;
	car->angle = PI * 0.5f;

	car->inertia = 0.0f;
	for (int i = 0; i < 4; ++i) {
		V2 corner = GetCorner(car, i);
		V2 radius = corner - car->position;
		F32 radiusLength = VectorLength(radius);
		car->inertia += CarCornerMass * (radiusLength * radiusLength);
	}

	PhysicsLabResize(labState, windowWidth, windowHeight);
}

static CollisionInfo GetCarLineCollisionInfo(CarState* oldCar, CarState* newCar, V2 point1, V2 point2)
{
	CollisionInfo hit = {};

	for (I32 i = 0; i < 4; ++i) {
		V2 oldCorner = GetCorner(oldCar, i);
		V2 newCorner = GetCorner(newCar, i);
		CollisionInfo hitCorner = {};
		if (DoLinesCross(point1, point2, oldCorner, newCorner)) {
			V2 wallAngle = PointDirection(point1, point2);
			hitCorner.normalVector = TurnVectorToRight(wallAngle);
			hitCorner.point = oldCorner;
			hitCorner.count++;
		}
		hit = hit + hitCorner;
	}

	return hit;
}

static CollisionInfo GetCarPointCollisionInfo(CarState* oldCar, CarState* newCar, V2 point)
{
	CollisionInfo hit = {};

	F32 halfLength = CarLength * 0.5f;
	F32 halfWidth  = CarWidth * 0.5f;
	V2 topLeft     = MakePoint(-halfLength, -halfWidth);
	V2 topRight    = MakePoint(-halfLength, +halfWidth); 
	V2 bottomLeft  = MakePoint(+halfLength, -halfWidth);
	V2 bottomRight = MakePoint(+halfLength, +halfWidth);

	V2 oldRelativePoint = GetCarRelativeCoordinates(oldCar, point);
	V2 newRelativePoint = GetCarRelativeCoordinates(newCar, point);

	if (DoLinesCross(oldRelativePoint, newRelativePoint, topLeft, topRight)) {
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle);
	}
	if (DoLinesCross(oldRelativePoint, newRelativePoint, bottomLeft, bottomRight)) {
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle);
	}
	if (DoLinesCross(oldRelativePoint, newRelativePoint, topLeft, bottomLeft)) {
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle + PI * 0.5f);
	}
	if (DoLinesCross(oldRelativePoint, newRelativePoint, topRight, bottomRight)) {
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle + PI * 0.5f);
	}

	return hit;
}

static CollisionInfo GetCarPolyCollisionInfo(CarState* oldCar, CarState* newCar, V2* points, I32 pointN)
{
	CollisionInfo hit = {};

	for (I32 i = 0; i < pointN; ++i) {
		I32 j = i + 1;
		if (j == pointN)
			j = 0;

		CollisionInfo hitLine = GetCarLineCollisionInfo(oldCar, newCar, points[i], points[j]);
		hit = hit + hitLine;

		CollisionInfo hitPoint = GetCarPointCollisionInfo(oldCar, newCar, points[i]);
		hit = hit + hitPoint;
	}

	return hit;
}

static void PhysicsLabUpdate(PhysicsLabState* labState, V2 mouse)
{
	float seconds = 0.05f;

	CarState* car = &labState->car;

	CarState oldCar = *car;

	V2 direction = RotationVector(car->angle);
	V2 tractionForce = ((car->engineForce) * CarMass * direction);

	F32 dragConstant = 0.05f;
	F32 speed = VectorLength(car->velocity);
	V2 drag = ((-dragConstant * speed) * CarMass * car->velocity);

	F32 sideResistanceConstant = 1.0f;
	F32 sideAngle = car->angle + PI * 0.5f;
	V2 sideVector = RotationVector(sideAngle);
	V2 sideResistance = (-sideResistanceConstant) * CarMass * DotProduct(car->velocity, sideVector) * sideVector;

	V2 oldPosition = car->position;
	car->acceleration = Invert(CarMass) *  (tractionForce + drag + sideResistance);
	car->velocity = car->velocity + (seconds * car->acceleration);
	car->position = car->position + (seconds * car->velocity);

	F32 angularResistanceConstant = 0.5f;
	F32 angularResistance = ((-angularResistanceConstant) * car->inertia * car->angularVelocity);
	F32 turn = car->inertia * car->turnInput;
	car->angularAcceleration = Invert(car->inertia) * (turn + angularResistance);

	F32 oldAngle = car->angle;
	car->angularVelocity = car->angularVelocity + (seconds * car->angularAcceleration);
	car->angle = car->angle + (seconds * car->angularVelocity);

	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = car->position;
	F32 left   = CameraLeftSide(camera);
	F32 right  = CameraRightSide(camera);
	F32 top    = CameraTopSide(camera);
	F32 bottom = CameraBottomSide(camera);

	I32 firstCol = Floor(left / gridDistance);
	I32 lastCol  = Floor(right / gridDistance);
	for (int col = firstCol; col <= lastCol; ++col) {
		F32 x = (col * gridDistance);
		V2 point1 = MakePoint(x, top);
		V2 point2 = MakePoint(x, bottom);
		Bresenham(canvas, point1, point2, gridColor);
	}

	I32 firstRow = Floor(top / gridDistance);
	I32 lastRow  = Floor(bottom / gridDistance);
	for (int row = firstRow; row <= lastRow; ++row) {
		F32 y = (row * gridDistance);
		V2 point1 = MakePoint(left, y);
		V2 point2 = MakePoint(right, y);
		Bresenham(canvas, point1, point2, gridColor);
	}

	V4 carColor = MakeColor(0.5f, 0.0f, 0.0f);
	V2 oldCorner0 = GetCorner(&oldCar, 0);
	V2 oldCorner1 = GetCorner(&oldCar, 1);
	V2 oldCorner2 = GetCorner(&oldCar, 2);
	V2 oldCorner3 = GetCorner(&oldCar, 3);
	DrawQuadPoints(canvas, oldCorner0, oldCorner1, oldCorner2, oldCorner3, carColor);

	CarState* newCar = &labState->car;
	CollisionInfo hit = GetCarPolyCollisionInfo(&oldCar, newCar, labState->walls, labState->wallN);

	if (hit.count > 1) {
		car->velocity = MakeVector(0.0f, 0.0f);
		car->position = oldPosition;

		car->angularVelocity = 0.0f;
		car->angle = oldAngle;
	}

	if (hit.count == 1) {
		V2 hitPointVelocity = GetCarPointVelocity(car, hit.point);
		V2 carCenter = car->position;

		F32 reflectionRatio = 0.5f;
		V2 turnForce = TurnVectorToRight(hit.point - carCenter);

		F32 up = -(1.0f + reflectionRatio) * DotProduct(hitPointVelocity, hit.normalVector);
		F32 down = DotProduct(hit.normalVector, hit.normalVector) * Invert(CarMass) +
			Square(DotProduct(turnForce, hit.normalVector)) * Invert(car->inertia);
		F32 forceRatio = up / down;

		car->velocity = car->velocity + (forceRatio / CarMass) * hit.normalVector;
		car->angularVelocity = car->angularVelocity + 
									DotProduct(turnForce, forceRatio * hit.normalVector) / car->inertia;

		car->position = oldPosition + seconds * car->velocity;
		car->angle = oldAngle + seconds * car->angularVelocity;
	}

	/*
	TODO: Check why this happens!
	for (int i = 0; i < 4; ++i) {
		V2 oldCorner = oldP[i];
		V2 newCorner = GetCorner(labState, i);
		Assert(!DoLinesCross(wall1, wall2, oldCorner, newCorner));
	}
	*/

	hit = GetCarPolyCollisionInfo(&oldCar, car, labState->walls, labState->wallN);
	if (hit.count > 0) {
		car->velocity = MakeVector(0.0f, 0.0f);
		car->position = oldPosition;

		car->angularVelocity = 0.0f;
		car->angle = oldAngle;
	}

	V4 wallColor = MakeColor(1.0f, 1.0f, 1.0f);
	F32 wallWidth = 0.2f;
	DrawPolyOutline(canvas, labState->walls, labState->wallN, wallColor);
}

void PhysicsLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = PhysicsLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "PhysicsLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"PhysicsLab",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		0
	);
	Assert(window != 0);

	PhysicsLabState* labState = &gPhysicsLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	PhysicsLabInit(labState, width, height);

	MSG message = {};
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		V2 mouse = GetMousePosition(&labState->camera, window);
		PhysicsLabUpdate(labState, mouse);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		PhysicsLabBlit(labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}

// TODO: Car crosses lines when going straight into it!
// TODO: Integrate physics to Car struct!
// TODO: Pull out common lab functions!