#include "PhysicsLab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"

#define CarCornerMass 250000
#define CarMass (4 * CarCornerMass)

struct PhysicsLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	F32 engineForce;
	F32 turnInput;

	V2 position;
	V2 velocity;
	V2 acceleration;

	F32 angle;
	F32 angularVelocity;
	F32 angularAcceleration;
	F32 inertia;

	V2 wallPoint1;
	V2 wallPoint2;
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

static V2 GetCorner(PhysicsLabState* labState, I32 cornerIndex)
{
	Assert(IsIntBetween(cornerIndex, 0, 3));
	F32 carWidth = 6.0f;
	F32 carLength = 10.0f;

	F32 halfWidth = carWidth * 0.5f;
	F32 halfLength = carLength * 0.5f;

	F32 angle = labState->angle;
	V2 rotationUpDown = RotationVector(angle);
	V2 rotationLeftRight = RotationVector(angle + PI * 0.5f);
	V2 center = labState->position;
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

static V2 GetCornerVelocity(PhysicsLabState* labState, int cornerIndex)
{
	Assert(IsIntBetween(cornerIndex, 0, 3));
	V2 velocity = labState->velocity;
	V2 corner = GetCorner(labState, cornerIndex);
	V2 cornerToCenter = (corner - labState->position);
	V2 turnVelocity = labState->angularVelocity * TurnVectorToRight(cornerToCenter);
	velocity = velocity + turnVelocity;
	return velocity;
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
		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case 'W':
					labState->engineForce = 1.0f;
					break;
				case 'S':
					labState->engineForce = -1.0f;
					break;
				case 'A':
					labState->turnInput = -0.1f;
					break;
				case 'D':
					labState->turnInput = +0.1f;
					break;
			}
			break;
		}
		case WM_KEYUP: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case 'W':
				case 'S':
					labState->engineForce = 0.0f;
					break;
				case 'A':
				case 'D':
					labState->turnInput = 0.0f;
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

	labState->wallPoint1 = MakePoint(-50.0f, -15.0f);
	labState->wallPoint2 = MakePoint(+50.0f, -15.0f);

	labState->inertia = 0.0f;
	for (int i = 0; i < 4; ++i) {
		V2 corner = GetCorner(labState, i);
		V2 radius = corner - labState->position;
		F32 radiusLength = VectorLength(radius);
		labState->inertia += CarCornerMass * (radiusLength * radiusLength);
	}

	PhysicsLabResize(labState, windowWidth, windowHeight);
}

static void PhysicsLabUpdate(PhysicsLabState* labState, V2 mouse)
{
	float seconds = 0.05f;

	V2 oldP[4] = {};
	for (int i = 0; i < 4; ++i)
		oldP[i] = GetCorner(labState, i);

	V2 direction = RotationVector(labState->angle);
	V2 tractionForce = ((labState->engineForce) * CarMass * direction);

	F32 dragConstant = 0.05f;
	F32 speed = VectorLength(labState->velocity);
	V2 drag = ((-dragConstant * speed) * CarMass * labState->velocity);

	F32 sideResistanceConstant = 1.0f;
	F32 sideAngle = labState->angle + PI * 0.5f;
	V2 sideVector = RotationVector(sideAngle);
	V2 sideResistance = (-sideResistanceConstant) * CarMass * DotProduct(labState->velocity, sideVector) * sideVector;

	V2 oldPosition = labState->position;
	labState->acceleration = Invert(CarMass) *  (tractionForce + drag + sideResistance);
	labState->velocity = labState->velocity + (seconds * labState->acceleration);
	labState->position = labState->position + (seconds * labState->velocity);

	F32 angularResistanceConstant = 0.5f;
	F32 angularResistance = ((-angularResistanceConstant) * labState->inertia * labState->angularVelocity);
	F32 turn = labState->inertia * labState->turnInput;
	labState->angularAcceleration = Invert(labState->inertia) * (turn + angularResistance);

	F32 oldAngle = labState->angle;
	labState->angularVelocity = labState->angularVelocity + (seconds * labState->angularAcceleration);
	labState->angle = labState->angle + (seconds * labState->angularVelocity);

	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = labState->position;
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
	DrawQuadPoints(canvas, oldP[0], oldP[1], oldP[2], oldP[3], carColor);
	
	V2 newP[4] = {};
	for (int i = 0; i < 4; ++i)
		newP[i] = GetCorner(labState, i);

	B32 hitWall = false;
	I32 hitCornerIndex = 0;
	I32 hitCount = 0;

	V2 wall1 = labState->wallPoint1;
	V2 wall2 = labState->wallPoint2;
	for (int i = 0; i < 4; ++i) {
		if (DoLinesCross(wall1, wall2, oldP[i], newP[i])) {
			hitWall = true;
			hitCornerIndex = i;
			hitCount++;
		}
	}

	if (hitCount > 1) {
		labState->velocity.y = 0.0f;
		labState->position = oldPosition + seconds * labState->velocity;

		labState->angularVelocity = 0.0f;
		labState->angle = oldAngle;
	}

	F32 eps = 1e-5f;
	if (Abs(labState->angularVelocity) < eps)
		labState->angularVelocity = 0.0f;

	if (hitWall && hitCount == 1) {
		V2 n = MakeVector(0.0f, 1.0f);
		F32 e = 0.5f;
		V2 v = GetCornerVelocity(labState, hitCornerIndex);
		V2 p = GetCorner(labState, hitCornerIndex);
		V2 r = p - labState->position;
		V2 rt = TurnVectorToRight(r);

		F32 up = -(1.0f + e) * DotProduct(v, n);
		F32 down = DotProduct(n, n) * Invert(CarMass) +
			Square(DotProduct(rt, n)) * Invert(labState->inertia);
		F32 j = up / down;

		labState->velocity = labState->velocity + (j / CarMass) * n;
		labState->angularVelocity = labState->angularVelocity + DotProduct(rt, j * n) / labState->inertia;

		labState->position = oldPosition + seconds * labState->velocity;
		labState->angle = oldAngle + seconds * labState->angularVelocity;

		V2 oldCorner = oldP[hitCornerIndex];
		V2 newCorner = GetCorner(labState, hitCornerIndex);
	}

	/*
	TODO: Check why this happens!
	for (int i = 0; i < 4; ++i) {
		V2 oldCorner = oldP[i];
		V2 newCorner = GetCorner(labState, i);
		Assert(!DoLinesCross(wall1, wall2, oldCorner, newCorner));
	}
	*/

	V4 wallColor = MakeColor(1.0f, 1.0f, 1.0f);
	F32 wallWidth = 0.2f;
	DrawLine(canvas, labState->wallPoint1, labState->wallPoint2, wallColor, wallWidth);
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