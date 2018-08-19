#include "PhysicsLab.hpp"

#include "../Bitmap.hpp"
#include "../Car.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"

struct PhysicsLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	V2 walls[4];
	I32 wallN;

	PlayerCar car;
};
PhysicsLabState gPhysicsLabState;

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

static LRESULT CALLBACK PhysicsLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	PhysicsLabState* labState = &gPhysicsLabState;
	PlayerCar* car = &labState->car;
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
					car->engineForce = 5.0f;
					break;
				case 'S':
					car->engineForce = -5.0f;
					break;
				case 'A':
					car->turnInput = -0.5f;
					break;
				case 'D':
					car->turnInput = +0.5f;
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

	PlayerCar* car = &labState->car;
	car->car.angle = PI * 0.5f;
	car->car.width = 2.3f;
	car->car.length = 5.0f;

	car->inertia = 0.0f;
	for (int i = 0; i < 4; ++i) {
		V2 corner = GetCarCorner(&car->car, i);
		V2 radius = corner - car->car.position;
		F32 radiusLength = VectorLength(radius);
		car->inertia += CarCornerMass * (radiusLength * radiusLength);
	}

	PhysicsLabResize(labState, windowWidth, windowHeight);
}

static void PhysicsLabUpdate(PhysicsLabState* labState, V2 mouse)
{
	PlayerCar* car = &labState->car;

	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = car->car.position;
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

	// float seconds = 0.05f;
	float seconds = 1.0f / 60.0f;

	PlayerCar oldCar = *car;

	V4 carColor = MakeColor(0.5f, 0.0f, 0.0f);
	V2 oldCorner0 = GetCarCorner(&oldCar.car, 0);
	V2 oldCorner1 = GetCarCorner(&oldCar.car, 1);
	V2 oldCorner2 = GetCarCorner(&oldCar.car, 2);
	V2 oldCorner3 = GetCarCorner(&oldCar.car, 3);
	DrawQuadPoints(canvas, oldCorner0, oldCorner1, oldCorner2, oldCorner3, carColor);

	UpdatePlayerCarWithoutCollision(car, seconds);
	/*
	CollisionInfo hit = {};
	hit = GetCarPolyCollisionInfo(&oldCar.car, &car->car, labState->walls, labState->wallN);
	UpdatePlayerCarCollision(car, &oldCar, seconds, hit);

	hit = GetCarPolyCollisionInfo(&oldCar.car, &car->car, labState->walls, labState->wallN);
	if (hit.count > 0) {
		car->velocity = MakeVector(0.0f, 0.0f);
		car->car.position = oldCar.car.position;

		car->angularVelocity = 0.0f;
		car->car.angle = oldCar.car.angle;
	}
	*/
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

// TODO: Add wheels?
// TODO: Update car controls!
// TODO: Pull out common lab functions!