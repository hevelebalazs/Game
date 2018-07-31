#include "PhysicsLab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"

struct PhysicsLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	V2 carPosition;
	V2 carVelocity;
	V2 carAcceleration;

	F32 angle;
	F32 angularVelocity;
	F32 angularAcceleration;

	I32 pushCornerIndex;
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

static void PushCar(PhysicsLabState* labState, V2 point, V2 force)
{
	labState->carVelocity = labState->carVelocity + force;

	V2 radius = point - labState->carPosition;
	V2 radiusPerp = TurnVectorToRight(radius);
	F32 angularVelocity = DotProduct(radiusPerp, force) * 0.01f;
	labState->angularVelocity += angularVelocity;
}

static V2 GetCorner(PhysicsLabState* labState, I32 cornerIndex)
{
	F32 carWidth = 6.0f;
	F32 carLength = 10.0f;

	F32 halfWidth = carWidth * 0.5f;
	F32 halfLength = carLength * 0.5f;

	F32 angle = labState->angle;
	V2 rotationUpDown = RotationVector(angle);
	V2 rotationLeftRight = RotationVector(angle + PI * 0.5f);
	V2 center = labState->carPosition;
	V2 frontCenter = center + (halfLength * rotationUpDown);
	V2 backCenter = center - (halfLength * rotationUpDown);

	V2 result = {};
	if (cornerIndex == 1)
		result = frontCenter - (halfWidth * rotationLeftRight);
	else if (cornerIndex == 2)
		result = frontCenter + (halfWidth * rotationLeftRight);
	else if (cornerIndex == 3)
		result = backCenter  + (halfWidth * rotationLeftRight);
	else if (cornerIndex == 4)
		result = backCenter  - (halfWidth * rotationLeftRight);
	else
		DebugBreak();

	return result;
}

static LRESULT CALLBACK PhysicsLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	PhysicsLabState* labState = &gPhysicsLabState;
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
		case WM_LBUTTONDOWN: {
			if (labState->pushCornerIndex != 0) {
				V2 mouse = GetMousePosition(&labState->camera, window);
				V2 corner = GetCorner(labState, labState->pushCornerIndex);
				V2 force = (mouse - corner);
				PushCar(labState, corner, force);
				labState->pushCornerIndex = 0;
			}
			break;
		}
		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case '1':
					labState->pushCornerIndex = 1;
					break;
				case '2':
					labState->pushCornerIndex = 2;
					break;
				case '3':
					labState->pushCornerIndex = 3;
					break;
				case '4':
					labState->pushCornerIndex = 4;
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

	labState->angle = PI * 0.5f;

	PhysicsLabResize(labState, windowWidth, windowHeight);
}

static void PhysicsLabUpdate(PhysicsLabState* labState, V2 mouse)
{
	float seconds = 0.05f;

	F32 resistanceConstant = 0.5f;
	V2 resistanceForce = ((-resistanceConstant) * labState->carVelocity);
	labState->carAcceleration = resistanceForce;

	labState->carVelocity = labState->carVelocity + (seconds * labState->carAcceleration);
	labState->carPosition = labState->carPosition + (seconds * labState->carVelocity);

	F32 angularResistanceConstant = 0.5f;
	F32 angularResistance = ((-angularResistanceConstant) * labState->angularVelocity);
	labState->angularAcceleration = angularResistance;

	labState->angularVelocity = labState->angularVelocity + (seconds * labState->angularAcceleration);
	labState->angle = labState->angle + (seconds * labState->angularVelocity);

	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = labState->carPosition;
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

	V4 carColor = MakeColor(0.4f, 0.0f, 0.0f);

	V2 p1 = GetCorner(labState, 1);
	V2 p2 = GetCorner(labState, 2);
	V2 p3 = GetCorner(labState, 3);
	V2 p4 = GetCorner(labState, 4);
	DrawQuadPoints(canvas, p1, p2, p3, p4, carColor);

	V4 forceColor = {0.0f, 0.5f, 0.0f};
	if (labState->pushCornerIndex != 0) {
		V2 corner = GetCorner(labState, labState->pushCornerIndex);
		Bresenham(canvas, corner, mouse, forceColor);
	}
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

// TODO: Pull out common lab functions!