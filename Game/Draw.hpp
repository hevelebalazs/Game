#pragma once

#include "Bitmap.hpp"
#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Text.hpp"
#include "Texture.hpp"
#include "Type.hpp"

struct Camera
{
	Vec2 center;
	Vec2 screen_pixel_size;

	Real32 unit_in_pixels;
	Real32 target_unit_in_pixels;
};

struct Canvas 
{
	Bitmap bitmap;
	Camera* camera;
	GlyphData* glyph_data;
};

static Int32
func UnitXtoPixel(Camera* camera, Real32 unit_x)
{
	Int32 pixel_x = Floor((camera->screen_pixel_size.x * 0.5f) + ((unit_x - camera->center.x) * camera->unit_in_pixels));
	return pixel_x;
}

static Int32
func UnitYtoPixel(Camera* camera, Real32 unit_y)
{
	Int32 pixel_y = Floor((camera->screen_pixel_size.y * 0.5f) + ((unit_y - camera->center.y) * camera->unit_in_pixels));
	return pixel_y;
}

static Vec2
func UnitToPixel(Camera* camera, Vec2 unit)
{
	Real32 x = (Real32)UnitXtoPixel(camera, unit.x);
	Real32 y = (Real32)UnitYtoPixel(camera, unit.y);
	Vec2 result = MakePoint(x, y);
	return result;
}

static void
func ResizeCamera(Camera* camera, Int32 width, Int32 height)
{
	camera->screen_pixel_size.x = (Real32)width;
	camera->screen_pixel_size.y = (Real32)height;
}

static UInt32*
func GetPixelAddress(Bitmap bitmap, Int32 row, Int32 col)
{
	UInt32* address = bitmap.memory + row * bitmap.width + col;
	return address;
}

static UInt32
func GetPixel(Bitmap bitmap, Int32 row, Int32 col)
{
	UInt32* pixel_address = GetPixelAddress(bitmap, row, col);
	return *pixel_address;
}

static void
func SetPixel(Bitmap bitmap, Int32 row, Int32 col, UInt32 color_code)
{
	UInt32* pixel_address = GetPixelAddress(bitmap, row, col);
	*pixel_address = color_code;
}

static void
func SetPixelCheck(Bitmap bitmap, Int32 row, Int32 col, Int32 color_code)
{
	if((row >= 0 && row < bitmap.height) && (col >= 0 && col < bitmap.width))
	{
		SetPixel(bitmap, row, col, color_code);
	}
}

static Vec4
func ColorProd(Vec4 color1, Vec4 color2)
{
	Vec4 result = {};
	result.red   = color1.red   * color2.red;
	result.green = color1.green * color2.green;
	result.blue  = color1.blue  * color2.blue;
	return result;
}

static void
func ApplyBitmapMask(Bitmap bitmap, Bitmap mask)
{
	UInt32* pixel = bitmap.memory;
	UInt32* mask_pixel = mask.memory;

	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			Vec4 color1 = GetColorFromColorCode(*pixel);
			Vec4 color2 = GetColorFromColorCode(*mask_pixel);
			Vec4 color = ColorProd(color1, color2);
			UInt32 code = GetColorCode(color);
			*pixel = code;

			pixel++;
			mask_pixel++;
		}
	}
}

struct PixelPosition
{
	Int32 row;
	Int32 col;
};

static void
func SmoothZoom(Camera* camera, Real32 pixel_per_unit)
{
	camera->target_unit_in_pixels = pixel_per_unit;
}

#define PixelPerUnitChangeSpeed 10.0f

static void
func UpdateCamera(Camera* camera, Real32 seconds)
{
	if(camera->unit_in_pixels != camera->target_unit_in_pixels) 
	{
		if(camera->unit_in_pixels < camera->target_unit_in_pixels)
		{
			camera->unit_in_pixels += seconds * PixelPerUnitChangeSpeed;
			if(camera->unit_in_pixels > camera->target_unit_in_pixels)
			{
				camera->unit_in_pixels = camera->target_unit_in_pixels;
			}
		} 
		else 
		{
			camera->unit_in_pixels -= seconds * PixelPerUnitChangeSpeed;
			if(camera->unit_in_pixels < camera->target_unit_in_pixels)
			{
				camera->unit_in_pixels = camera->target_unit_in_pixels;
			}
		}
	}
}

static Real32
func GetUnitDistanceInPixel(Camera* camera, Real32 unit_distance)
{
	Real32 pixel_distance = unit_distance * camera->unit_in_pixels;
	return pixel_distance;
}

static Real32
func GetPixelDistanceInUnit(Camera* camera, Real32 pixel_distance)
{
	Assert(camera->unit_in_pixels > 0.0f);
	Real32 unit_distance = pixel_distance / camera->unit_in_pixels;
	return unit_distance;
}

static Real32
func PixelToUnitX(Camera* camera, Real32 pixel_x)
{
	Real32 pixel_in_units = Invert(camera->unit_in_pixels);
	Real32 unit_x = camera->center.x + (pixel_x - camera->screen_pixel_size.x * 0.5f) * pixel_in_units;
	return unit_x;
}

static Real32
func PixelToUnitY(Camera* camera, Real32 pixel_y) 
{
	Real32 pixel_in_units = Invert(camera->unit_in_pixels);
	Real32 unit_y = camera->center.y + (pixel_y - camera->screen_pixel_size.y * 0.5f) * pixel_in_units;
	return unit_y;
}

static Vec2
func PixelToUnit(Camera* camera, IntVec2 pixel)
{
	Vec2 result = {};
	result.x = PixelToUnitX(camera, (Real32)pixel.col);
	result.y = PixelToUnitY(camera, (Real32)pixel.row);
	return result;
}

static Vec2
func PixelToUnit(Camera* camera, Vec2 pixel)
{
	Vec2 result = {};
	result.x = PixelToUnitX(camera, pixel.x);
	result.y = PixelToUnitY(camera, pixel.y);
	return result;
}

static Real32
func CameraLeftSide(Camera* camera)
{
	Real32 left_side = PixelToUnitX(camera, 0);
	return left_side;
}

static Real32
func CameraRightSide(Camera* camera)
{
	Real32 right_side = PixelToUnitX(camera, camera->screen_pixel_size.x - 1);
	return right_side;
}

static Real32
func CameraTopSide(Camera* camera)
{
	Real32 top_side = PixelToUnitY(camera, 0);
	return top_side;
}

static Real32
func CameraBottomSide(Camera* camera)
{
	Real32 bottom_side = PixelToUnitY(camera, camera->screen_pixel_size.y - 1);
	return bottom_side;
}

static Vec2
func GetCameraTopLeftCorner(Camera* camera)
{
	Vec2 corner = {};
	corner.x = CameraLeftSide(camera);
	corner.y = CameraTopSide(camera);
	return corner;
}

static Vec2
func GetCameraBottomRightCorner(Camera* camera)
{
	Vec2 corner = {};
	corner.x = CameraRightSide(camera);
	corner.y = CameraBottomSide(camera);
	return corner;
}

static void
func ClearScreen(Canvas* canvas, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);

	Bitmap bitmap = canvas->bitmap;
	UInt32 *pixel = bitmap.memory;
	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

struct BresenhamContext 
{
	Int32 x1, y1;
	Int32 x2, y2;
	Int32 abs_x, abs_y;
	Int32 add_x, add_y;
	Int32 error, error2;
};

static BresenhamContext
func BresenhamInitPixel(Vec2 pixel_point1, Vec2 pixel_point2)
{
	BresenhamContext context = {};

	context.x1 = (Int32)pixel_point1.x;
	context.y1 = (Int32)pixel_point1.y;
	context.x2 = (Int32)pixel_point2.x;
	context.y2 = (Int32)pixel_point2.y;

	context.abs_x = IntAbs(context.x1 - context.x2);
	context.abs_y = IntAbs(context.y1 - context.y2);

	context.add_x = 1;
	if(context.x1 > context.x2)
	{
		context.add_x = -1;
	}

	context.add_y = 1;
	if(context.y1 > context.y2)
	{
		context.add_y = -1;
	}

	context.error = 0;
	if(context.abs_x > context.abs_y)
	{
		context.error = context.abs_x / 2;
	}
	else
	{
		context.error = -context.abs_y / 2;
	}

	context.error2 = 0;

	return context;
}

static BresenhamContext
func BresenhamInitUnit(Canvas* canvas, Vec2 point1, Vec2 point2)
{
	Camera* camera = canvas->camera;
	Vec2 pixel_point1 = UnitToPixel(camera, point1);
	Vec2 pixel_point2 = UnitToPixel(camera, point2);
	BresenhamContext context = BresenhamInitPixel(pixel_point1, pixel_point2);
	return context;
}

static void
func BresenhamAdvance(BresenhamContext* context)
{
	context->error2 = context->error;
	if(context->error2 > -context->abs_x)
	{
		context->error -= context->abs_y;
		context->x1 += context->add_x;
	}
	if(context->error2 < context->abs_y) 
	{
		context->error += context->abs_x;
		context->y1 += context->add_y;
	}
}

static void
func Bresenham(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color)
{
	Bitmap bitmap = canvas->bitmap;
	UInt32 color_code = GetColorCode(color);

	BresenhamContext context = BresenhamInitUnit(canvas, point1, point2);
	while(1) 
	{
		SetPixelCheck(bitmap, context.y1, context.x1, color_code);

		if(context.x1 == context.x2 && context.y1 == context.y2)
		{
			break;
		}

		BresenhamAdvance(&context);
	}
}

static Vec2
func LineAtX(Vec2 line_point1, Vec2 line_point2, Real32 x)
{
	Real32 minX = Min2(line_point1.x, line_point2.x);
	Real32 maxX = Max2(line_point1.x, line_point2.x);
	Assert((minX <= x) && (x <= maxX));

	if(line_point1.x == line_point2.x)
	{
		return line_point1;
	}

	Real32 alpha = (x - line_point1.x) / (line_point2.x - line_point1.x);
	Vec2 result = {};
	result.x = x;
	result.y = line_point1.y + alpha * (line_point2.y - line_point1.y);
	return result;
}

static Vec2
func LineAtY(Vec2 line_point1, Vec2 line_point2, Real32 y)
{
	Real32 minY = Min2(line_point1.y, line_point2.y);
	Real32 maxY = Max2(line_point1.y, line_point2.y);
	Assert((minY <= y) && (y <= maxY));
	
	if(line_point1.y == line_point2.y)
	{
		return line_point1;
	}

	Real32 alpha = (y - line_point1.y) / (line_point2.y - line_point1.y);
	Vec2 result = {};
	result.x = line_point1.x + alpha * (line_point2.x - line_point1.x);
	result.y = y;
	return result;
}

static void
func DrawHorizontalTrapezoid(Canvas* canvas, Vec2 top_left, Vec2 top_right, Vec2 bottom_left, Vec2 bottom_right, Vec4 color)
{
	Camera* camera = canvas->camera;
	Real32 camera_top    = CameraTopSide(camera);
	Real32 camera_bottom = CameraBottomSide(camera);

	if(top_left.y < camera_top) 
	{
		if(bottom_left.y < camera_top)
		{
			return;
		}

		top_left = LineAtY(top_left, bottom_left, camera_top);
	}
	if(top_right.y < camera_top) 
	{
		if(bottom_right.y < camera_top)
		{
			return;
		}

		top_right = LineAtY(top_right, bottom_right, camera_top);
	}
	if(bottom_left.y > camera_bottom) 
	{
		if(top_left.y > camera_bottom)
		{
			return;
		}

		bottom_left = LineAtY(top_left, bottom_left, camera_bottom);
	}
	if(bottom_right.y > camera_bottom) 
	{
		if(top_right.y > camera_bottom)
		{
			return;
		}

		bottom_right = LineAtY(top_right, bottom_right, camera_bottom);
	}

	Bitmap bitmap = canvas->bitmap;
	UInt32 color_code = GetColorCode(color);

	BresenhamContext left_line = BresenhamInitUnit(canvas, top_left, bottom_left);
	BresenhamContext right_line = BresenhamInitUnit(canvas, top_right, bottom_right);

	Int32 top = UnitYtoPixel(camera, top_left.y);
	Int32 bottom = UnitYtoPixel(camera, bottom_left.y);
	if(bottom < top)
	{
		return;
	}

	for(Int32 row = top; row <= bottom; row++) 
	{
		while(left_line.y1 < row)
		{
			BresenhamAdvance(&left_line);
		}

		while(right_line.y1 < row)
		{
			BresenhamAdvance(&right_line);
		}

		if((row < 0) || (row > bitmap.height - 1))
		{
			continue;
		}

		Int32 left = left_line.x1;
		Int32 right = right_line.x1;

		if(left < 0) 
		{
			left = 0;
		}
		if(right > bitmap.width - 1)
		{
			right = bitmap.width - 1;
		}

		UInt32 *pixel = GetPixelAddress(bitmap, row, left);
		for(Int32 col = left; col <= right; col++) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

static void
func DrawVerticalTrapezoid(Canvas* canvas, Vec2 top_left, Vec2 top_right, Vec2 bottom_left, Vec2 bottom_right, Vec4 color)
{
	Camera* camera = canvas->camera;
	Real32 camera_left  = CameraLeftSide(camera);
	Real32 camera_right = CameraRightSide(camera);

	if(top_left.x < camera_left) 
	{
		if(top_right.x < camera_left)
		{
			return;
		}

		top_left = LineAtX(top_left, top_right, camera_left);
	}
	if(top_right.x > camera_right) 
	{
		if(top_left.x > camera_right)
		{
			return;
		}

		top_right = LineAtX(top_left, top_right, camera_right);
	}
	if(bottom_left.x < camera_left) 
	{
		if(bottom_right.x < camera_left)
		{
			return;
		}

		bottom_left = LineAtX(bottom_left, bottom_right, camera_left);
	}
	if(bottom_right.x > camera_right) 
	{
		if(bottom_left.x > camera_right)
		{
			return;
		}

		bottom_right = LineAtX(bottom_left, bottom_right, camera_right);
	}

	Bitmap bitmap = canvas->bitmap;
	UInt32 color_code = GetColorCode(color);

	BresenhamContext top_line = BresenhamInitUnit(canvas, top_left, top_right);
	BresenhamContext bottom_line = BresenhamInitUnit(canvas, bottom_left, bottom_right);

	Int32 left = UnitXtoPixel(camera, top_left.x);
	Int32 right = UnitXtoPixel(camera, top_right.x);
	if(right < left)
	{
		return;
	}

	for(Int32 col = left; col <= right; col++) 
	{
		while(top_line.x1 < col)
		{
			BresenhamAdvance(&top_line);
		}

		while(bottom_line.x1 < col)
		{
			BresenhamAdvance(&bottom_line);
		}

		if((col < 0) || (col > bitmap.width - 1))
		{
			continue;
		}

		Int32 top = top_line.y1;
		Int32 bottom = bottom_line.y1;

		if(top < 0) 
		{
			top = 0;
		}

		if(bottom > bitmap.height - 1)
		{
			bottom = bitmap.height - 1;
		}

		UInt32* pixel = GetPixelAddress(bitmap, top, col);
		for(Int32 row = top; row <= bottom; row++) 
		{
			*pixel = color_code;
			pixel += bitmap.width;
		}
	}
}

static void
func DrawCircle(Canvas *canvas, Vec2 center, Real32 radius, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;

	Int32 center_x_pixel = UnitXtoPixel(camera, center.x);
	Int32 center_y_pixel = UnitYtoPixel(camera, center.y);

	Int32 left_pixel   = UnitXtoPixel(camera, center.x - radius);
	Int32 right_pixel  = UnitXtoPixel(camera, center.x + radius);
	Int32 top_pixel    = UnitYtoPixel(camera, center.y - radius);
	Int32 bottom_pixel = UnitYtoPixel(camera, center.y + radius);

	if(top_pixel > bottom_pixel)
	{
		IntSwap(&top_pixel, &bottom_pixel);
	}

	if(left_pixel > right_pixel)
	{
		IntSwap(&left_pixel, &right_pixel);
	}

	Int32 pixel_radius = (Int32)(radius * camera->unit_in_pixels);
	Int32 pixel_radius_square = pixel_radius * pixel_radius;

	Bitmap bitmap = canvas->bitmap;
	top_pixel    = IntMax2(top_pixel, 0);
	bottom_pixel = IntMin2(bottom_pixel, bitmap.height - 1);
	left_pixel   = IntMax2(left_pixel, 0);
	right_pixel  = IntMin2(right_pixel, bitmap.width - 1);

	for(Int32 row = top_pixel; row < bottom_pixel; row++)
	{
		for(Int32 col = left_pixel; col < right_pixel; col++)
		{
			UInt32 *pixel = bitmap.memory + row * bitmap.width + col;
			Int32 pixel_distance_square = IntSquare(row - center_y_pixel) + IntSquare(col - center_x_pixel);
			if(pixel_distance_square <= pixel_radius_square)
			{
				*pixel = color_code;
			}
		}
	}
}

static void
func DrawCircleOutline(Canvas *canvas, Vec2 center, Real32 radius, Vec4 color)
{
	Int32 line_n = 20;
	Real32 angle_advance = (2.0f * PI) / Real32(line_n);
	Real32 angle = 0.0f;
	for(Int32 i = 0; i < line_n; i++)
	{
		Real32 angle1 = angle;
		Real32 angle2 = angle + angle_advance;
		Vec2 point1 = center + radius * RotationVector(angle1);
		Vec2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);

		angle += angle_advance;
	}
}

static void
func DrawRectLRTB(Canvas* canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;
	Int32 top_pixel    = UnitYtoPixel(camera, top);
	Int32 left_pixel   = UnitXtoPixel(camera, left);
	Int32 bottom_pixel = UnitYtoPixel(camera, bottom);
	Int32 right_pixel  = UnitXtoPixel(camera, right);

	if(top_pixel > bottom_pixel)
	{
		IntSwap(&top_pixel, &bottom_pixel);
	}

	if(left_pixel > right_pixel)
	{
		IntSwap(&left_pixel, &right_pixel);
	}

	Bitmap bitmap = canvas->bitmap;
	top_pixel    = IntMax2(top_pixel, 0);
	bottom_pixel = IntMin2(bottom_pixel, bitmap.height - 1);
	left_pixel   = IntMax2(left_pixel, 0);
	right_pixel  = IntMin2(right_pixel, bitmap.width - 1);

	for(Int32 row = top_pixel; row < bottom_pixel; row++) 
	{
		for(Int32 col = left_pixel; col < right_pixel; col++) 
		{
			UInt32 *pixel = bitmap.memory + row * bitmap.width + col;
			*pixel = color_code;
		}
	}
}

static void
func DrawRect(Canvas* canvas, Rect rect, Vec4 color)
{
	DrawRectLRTB(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void
func DrawGridLine(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color, Real32 line_width)
{
	Real32 left   = 0.0f;
	Real32 right  = 0.0f;
	Real32 top    = 0.0f;
	Real32 bottom = 0.0f;

	if(point1.x == point2.x) 
	{
		left  = point1.x - line_width * 0.5f;
		right = point1.x + line_width * 0.5f;

		top    = Min2(point1.y, point2.y);
		bottom = Max2(point1.y, point2.y);
	} 
	else if(point1.y == point2.y)
	{
		top    = point1.y - line_width * 0.5f;
		bottom = point1.y + line_width * 0.5f;

		left  = Min2(point1.x, point2.x);
		right = Max2(point1.x, point2.x);
	}

	DrawRectLRTB(canvas, left, right, top, bottom, color);
}

static void
func DrawQuad(Canvas *canvas, Quad quad, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);

	Vec2 *points = quad.points;
	Camera *camera = canvas->camera;
	for(Int32 i = 0; i < 4; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	Int32 min_x = bitmap.width;
	Int32 min_y = bitmap.height;
	Int32 max_x = 0;
	Int32 max_y = 0;

	for(Int32 i = 0; i < 4; i++) 
	{
		Int32 point_x = (Int32)points[i].x;
		Int32 point_y = (Int32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	UInt32 *pixel = 0;
	for(Int32 row = min_y; row < max_y; ++row) 
	{
		for(Int32 col = min_x; col < max_x; ++col) 
		{
			Vec2 test_point = MakePoint(Real32(col), Real32(row));

			Bool32 draw_point = true;

			if(!TurnsRight(points[0], points[1], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[1], points[2], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[2], points[3], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[3], points[0], test_point))
			{
				draw_point = false;
			}

			if(draw_point) 
			{
				pixel = (UInt32 *)bitmap.memory + row * bitmap.width + col;
				*pixel = color_code;
			}
		}
	}
}

static void
func DrawLine(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color, Real32 line_width)
{
	Vec2 direction = PointDirection(point2, point1);

	Real32 tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	Real32 half_line_width = line_width * 0.5f;
	Quad quad = {};
	quad.points[0] = point1 - (half_line_width * direction);
	quad.points[1] = point1 + (half_line_width * direction);
	quad.points[2] = point2 + (half_line_width * direction);
	quad.points[3] = point2 - (half_line_width * direction);
	DrawQuad(canvas, quad, color);
}

#define WorldTextureScale 20.0f

static void
func FillScreenWithWorldTexture(Canvas* canvas, Texture texture)
{
	Bitmap bitmap = canvas->bitmap;
	Camera* camera = canvas->camera;

	Real32 pixel_in_units = Invert(camera->unit_in_pixels);

	Real32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	Real32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	Real32 add_x = pixel_in_units;
	Real32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
	Int32 and_val = (texture.side - 1);

	UInt32 *pixel = bitmap.memory;

	Int32 start_int_x = ((Int32)start_x) & and_val;
	Int32 start_int_y = ((Int32)start_y) & and_val;

	UInt32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);

	Real32 y = start_y;
	Int32 prev_int_y = ((Int32)y) & and_val;

	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		Real32 x = start_x;
		Int32 prev_int_x = (((Int32)x) & and_val);
		UInt32 *texture_pixel = texture_row;

		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = *texture_pixel;
			pixel++;

			x += add_x;
			Int32 int_x = ((Int32)x) & and_val;
			texture_pixel += (int_x - prev_int_x);
			prev_int_x = int_x;
		}

		y += add_y;
		Int32 int_y = ((Int32)y) & and_val;
		texture_row += (int_y - prev_int_y) << texture.log_side;
		prev_int_y = int_y;
	}
}

static void
func DrawWorldTextureQuad(Canvas *canvas, Quad quad, Texture texture)
{
    Bitmap bitmap = canvas->bitmap;
	Camera *camera = canvas->camera;
    
    Vec2 *points = quad.points;
    for(Int32 i = 0; i < 4; i++)
	{
        points[i] = UnitToPixel(camera, points[i]);
	}

	Int32 min_x = bitmap.width;
	Int32 min_y = bitmap.height;
	Int32 max_x = 0;
	Int32 max_y = 0;
   
	for(Int32 i = 0; i < 4; i++) 
	{
		Int32 point_x = (Int32)points[i].x;
        Int32 point_y = (Int32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);
    
	Real32 pixel_in_units = Invert(camera->unit_in_pixels);

	Real32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	Real32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	Real32 add_x = pixel_in_units;
	Real32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
    
    start_x += (min_x * add_x);
    start_y += (min_y * add_y);
    
    Int32 and_val = (texture.side - 1);

	Int32 start_int_x = ((Int32)start_x) & and_val;
	Int32 start_int_y = ((Int32)start_y) & and_val;

	UInt32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);
	UInt32 *pixel_row = bitmap.memory + (bitmap.width * min_y) + (min_x);

	Real32 y = start_y;
	Int32 prev_int_y = ((Int32)y) & and_val;

	for(Int32 row = min_y; row < max_y; row++) 
	{
        Real32 x = start_x;
		Int32 prev_int_x = ((Int32)x & and_val);
		UInt32 *texture_pixel = texture_row;
		UInt32 *pixel = pixel_row;
    
		for(Int32 col = min_x; col < max_x; col++) 
		{
			Vec2 test_point = {};
            test_point.x = (Real32)col;
            test_point.y = (Real32)row;

			Bool32 draw_point = true;

			if(!TurnsRight(points[0], points[1], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[1], points[2], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[2], points[3], test_point))
			{
				draw_point = false;
			}
			else if(!TurnsRight(points[3], points[0], test_point))
			{
				draw_point = false;
			}

			if(draw_point)
			{
                *pixel = *texture_pixel;
			}
            
            pixel++;
            
            x += add_x;
			Int32 int_x = ((Int32)x) & and_val;
			texture_pixel += (int_x - prev_int_x);
			prev_int_x = int_x;
		}
        
        y += add_y;
		Int32 int_y = ((Int32)y) & and_val;
		texture_row += (int_y - prev_int_y) << texture.log_side;
		pixel_row += (bitmap.width);
		prev_int_y = int_y;
	}
}

static void
func DrawWorldTextureLine(Canvas *canvas, Vec2 point1, Vec2 point2, Real32 line_width, Texture texture)
{
	Vec2 direction = PointDirection(point2, point1);
	Vec2 turned_direction = TurnVectorToRight(direction);
	
	Real32 half_line_width = line_width * 0.5f;
	Quad quad = {};
	Vec2 point_product = (half_line_width * turned_direction);
	quad.points[0] = (point1 - point_product);
	quad.points[1] = (point1 + point_product);
	quad.points[2] = (point2 + point_product);
	quad.points[3] = (point2 - point_product);
	DrawWorldTextureQuad(canvas, quad, texture);
}

static void
func DrawRectLRTBOutline(Canvas *canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Vec4 color)
{
	Vec2 top_left     = MakePoint(left,  top);
	Vec2 top_right    = MakePoint(right, top);
	Vec2 bottom_left  = MakePoint(left,  bottom);
	Vec2 bottom_right = MakePoint(right, bottom);

	Bresenham(canvas, top_left,     top_right,    color);
	Bresenham(canvas, top_right,    bottom_right, color);
	Bresenham(canvas, bottom_right, bottom_left,  color);
	Bresenham(canvas, bottom_left,  top_left,     color);
}

static void
func DrawRectOutline(Canvas *canvas, Rect rect, Vec4 color)
{
	DrawRectLRTBOutline(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void
func WorldTextureRect(Canvas *canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Texture texture)
{
	Camera *camera = canvas->camera;
	Int32 top_pixel =    UnitYtoPixel(camera, top);
	Int32 left_pixel =   UnitXtoPixel(camera, left);
	Int32 bottom_pixel = UnitYtoPixel(camera, bottom);
	Int32 right_pixel =  UnitXtoPixel(camera, right);

	if(top_pixel > bottom_pixel)
	{
		IntSwap(&top_pixel, &bottom_pixel);
	}

	if(left_pixel > right_pixel)
	{
		IntSwap(&left_pixel, &right_pixel);
	}

	Bitmap bitmap = canvas->bitmap;

	top_pixel    = IntMax2(top_pixel, 0);
	bottom_pixel = IntMin2(bottom_pixel, bitmap.height - 1);
	left_pixel   = IntMax2(left_pixel, 0);
	right_pixel  = IntMin2(right_pixel, bitmap.width - 1);

	Real32 texture_zoom = 16.0f;

	Vec2 top_left_pixel = MakePoint((Real32)left_pixel, (Real32)top_pixel);
	Vec2 top_left_unit = PixelToUnit(camera, top_left_pixel);
	// NOTE: optimization stuff
	top_left_unit.x *= texture_zoom;
	top_left_unit.y *= texture_zoom;

	Real32 unit_per_pixel = Invert(camera->unit_in_pixels);
	// NOTE: optimization stuff
	unit_per_pixel *= texture_zoom;

	Int32 sub_unit_per_pixel = (Int32)(unit_per_pixel * 255.0f);

	UInt32 *top_left = bitmap.memory + top_pixel * bitmap.width + left_pixel;

	// TODO: try to optimize this code
	Vec2 world_unit = top_left_unit;
	Int32 leftx  = ((Int32)world_unit.x) & (texture.side - 1);
	Int32 topy   = ((Int32)world_unit.y) & (texture.side - 1);

	Int32 leftsubx = (Int32)((world_unit.x - Floor(world_unit.x)) * 255.0f);
	Int32 topsuby = (Int32)((world_unit.y - Floor(world_unit.y)) * 255.0f);

	Int32 x = leftx;
	Int32 y = topy;

	Int32 subx = leftsubx;
	Int32 suby = topsuby;

	for(Int32 row = top_pixel; row < bottom_pixel; row++) 
	{
		x = leftx;
		subx = leftsubx;

		UInt32 *pixel = top_left;

		for(Int32 col = left_pixel; col < right_pixel; col++) 
		{
			*pixel = TextureColorCodeInt(texture, y, x);
			//*pixel = TextureColorCode(texture, x, subx, y, suby);
			pixel++;

			subx += sub_unit_per_pixel;
			if(subx > 0xFF) 
			{
				x = ((x + (subx >> 8)) & (texture.side - 1));
				subx = (subx & 0xFF);
			}
		}

		suby += sub_unit_per_pixel;
		if(suby > 0xFF) 
		{
			y = ((y + (suby >> 8)) & (texture.side - 1));
			suby = (suby & 0xFF);
		}

		top_left += bitmap.width;
	}
}

static void
func WorldTextureGridLine(Canvas *canvas, Vec2 point1, Vec2 point2, Real32 width, Texture texture)
{
	Real32 left   = Min2(point1.x, point2.x);
	Real32 right  = Max2(point1.x, point2.x);
	Real32 top    = Min2(point1.y, point2.y);
	Real32 bottom = Max2(point1.y, point2.y);

	// TODO: this Assert was triggered, check what's happening
	Assert((left == right) || (top == bottom));
	if(left == right) 
	{
		left  -= width * 0.5f;
		right += width * 0.5f;
	} 
	else if(top == bottom) 
	{
		top    -= width * 0.5f;
		bottom += width * 0.5f;
	}

	WorldTextureRect(canvas, left, right, top, bottom, texture);
}

static void
func DrawPolyOutline(Canvas *canvas, Vec2 *points, Int32 point_n, Vec4 color)
{
	Int32 prev = point_n - 1;
	for(Int32 i = 0; i < point_n; i++) 
	{
		Bresenham(canvas, points[prev], points[i], color);
		prev = i;
	}
}

static void
func DrawPoly(Canvas *canvas, Vec2 *points, Int32 point_n, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;
	for(Int32 i = 0; i < point_n; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	Int32 min_x = bitmap.width;
	Int32 min_y = bitmap.height;
	Int32 max_x = 0;
	Int32 max_y = 0;

	for(Int32 i = 0; i < point_n; i++) 
	{
		Int32 point_x = (Int32)points[i].x;
		Int32 point_y = (Int32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	UInt32 *pixel = 0;
	for(Int32 row = min_y; row < max_y; row++) 
	{
		for(Int32 col = min_x; col < max_x; col++) 
		{
			Vec2 test_point = MakePoint(Real32(col), Real32(row));

			Bool32 draw_point = true;

			Int32 prev = point_n - 1;
			for(Int32 i = 0; i < point_n; i++) 
			{
				// TODO: is using cross product faster than these calls?
				if(!TurnsRight(points[prev], points[i], test_point)) 
				{
					draw_point = false;
					break;
				}
				prev = i;
			}

			if(draw_point) 
			{
				pixel = (UInt32 *)bitmap.memory + row * bitmap.width + col;
				*pixel = color_code;
			}
		}
	}

	for(Int32 i = 0; i < point_n; ++i)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void
func DrawWorldTexturePoly(Canvas *canvas, Vec2 *points, Int32 point_n, Texture texture)
{
	Bitmap bitmap = canvas->bitmap;
	Camera *camera = canvas->camera;

	Int32 min_x = bitmap.width;
	Int32 min_y = bitmap.height;
	Int32 max_x = 0;
	Int32 max_y = 0;

	for(Int32 i = 0; i < point_n; i++) 
	{
		Vec2 point_in_pixels = UnitToPixel(camera, points[i]);
		Int32 point_x = Int32(point_in_pixels.x);
		Int32 point_y = Int32(point_in_pixels.y);
		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	Real32 pixel_in_units = Invert(camera->unit_in_pixels);
    
    Real32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	Real32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	Real32 add_x = pixel_in_units;
	Real32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
    
    start_x += (min_x * add_x);
    start_y += (min_y * add_y);
    
    Int32 and_val = (texture.side - 1);

	Int32 start_int_x = ((Int32)start_x) & and_val;
	Int32 start_int_y = ((Int32)start_y) & and_val;

	UInt32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);
	UInt32 *pixel_row = bitmap.memory + (bitmap.width * min_y) + (min_x);

	Real32 y = start_y;
	Int32 prev_int_y = (Int32(y)) & and_val;

	UInt32 *pixel = 0;
	for(Int32 row = min_y; row <= max_y; row++) 
	{
        Real32 x = start_x;
		Int32 prev_int_x = (Int32(x) & and_val);
		UInt32 *texture_pixel = texture_row;
		UInt32 *pixel = pixel_row;
    
		for(Int32 col = min_x; col <= max_x; col++) 
		{
            Vec2 test_point = {};
            test_point.x = Real32(col);
            test_point.y = Real32(row);

			Bool32 draw_point = true;

			Int32 prev = point_n - 1;
			for(Int32 i = 0; i < point_n; i++) 
			{
				Vec2 prev_point = UnitToPixel(camera, points[prev]);
				Vec2 this_point = UnitToPixel(camera, points[i]);
				if(!TurnsRight(prev_point, this_point, test_point)) 
				{
					draw_point = false;
					break;
				}
				prev = i;
			}

			if(draw_point)
			{
                *pixel = *texture_pixel;
			}
                
            pixel++;
            x += add_x;
            Int32 int_x = ((Int32)x) & and_val;
            texture_pixel += (int_x - prev_int_x);
            prev_int_x = int_x;
		}
        
        y += add_y;
        Int32 int_y = ((Int32)y) & and_val;
        texture_row += (int_y - prev_int_y) << texture.log_side;
        pixel_row += (bitmap.width);
        prev_int_y = int_y;
	}

	for(Int32 i = 0; i < point_n; i++)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void
func DrawQuadPoints(Canvas *canvas, Vec2 point1, Vec2 point2, Vec2 point3, Vec2 point4, Vec4 color)
{
	Quad quad = MakeQuad(point1, point2, point3, point4);
	DrawQuad(canvas, quad, color);
}

static void
func DrawBitmap(Canvas *canvas, Bitmap *bitmap, Real32 left, Real32 top)
{
	Camera *camera = canvas->camera;
	Int32 pixel_left = UnitXtoPixel(camera, left);
	Int32 pixel_top  = UnitYtoPixel(camera, top);
	CopyBitmap(bitmap, &canvas->bitmap, pixel_left, pixel_top);
}

static Real32
func GetTextHeight(Canvas *canvas, Int8 *text)
{
	Camera *camera = canvas->camera;
	Assert(camera->unit_in_pixels > 0.0f);
	Real32 height = (Real32)TextHeightInPixels / camera->unit_in_pixels;
	return height;
}

static Real32
func GetTextWidth(Canvas *canvas, Int8 *text)
{
	Assert(canvas->glyph_data != 0);
	Real32 pixel_width = GetTextPixelWidth(text, canvas->glyph_data);

	Camera *camera = canvas->camera;
	Assert(camera->unit_in_pixels > 0.0f);
	Real32 width = pixel_width / camera->unit_in_pixels;
	return width;
}

static void
func DrawTextLine(Canvas *canvas, Int8 *text, Real32 base_line_y, Real32 left, Vec4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Int32 left_pixel = UnitXtoPixel(canvas->camera, left);
	Int32 base_line_y_pixel = UnitYtoPixel(canvas->camera, base_line_y);
	DrawBitmapTextLine(&canvas->bitmap, text, canvas->glyph_data, left_pixel, base_line_y_pixel, text_color);
}

static void
func DrawTextLineXCentered(Canvas *canvas, Int8 *text, Real32 base_line_y, Real32 center_x, Vec4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Real32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}

static void
func DrawTextLineXYCentered(Canvas *canvas, Int8 *text, Real32 center_y, Real32 center_x, Vec4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Camera *camera = canvas->camera;

	Real32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	Real32 text_height = GetPixelDistanceInUnit(camera, TextHeightInPixels);
	Real32 text_top = center_x - text_height * 0.5f;
	Real32 base_line_y = text_top + GetPixelDistanceInUnit(camera, TextPixelsAboveBaseLine);
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}

static void
func DrawTextLineBottomXCentered(Canvas *canvas, Int8 *text, Real32 bottom, Real32 center_x, Vec4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Camera *camera = canvas->camera;

	Real32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	Real32 text_height = GetPixelDistanceInUnit(camera, TextHeightInPixels);
	Real32 text_top = bottom - text_height;
	Real32 base_line_y = text_top + GetPixelDistanceInUnit(camera, TextPixelsAboveBaseLine);
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}