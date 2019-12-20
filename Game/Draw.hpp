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
	V2 center;
	V2 screen_pixel_size;

	R32 unit_in_pixels;
	R32 target_unit_in_pixels;
};

struct Canvas 
{
	Bitmap bitmap;
	Camera* camera;
	GlyphData* glyph_data;
};

static I32
func UnitXtoPixel(Camera* camera, R32 unit_x)
{
	I32 pixel_x = Floor((camera->screen_pixel_size.x * 0.5f) + ((unit_x - camera->center.x) * camera->unit_in_pixels));
	return pixel_x;
}

static I32
func UnitYtoPixel(Camera* camera, R32 unit_y)
{
	I32 pixel_y = Floor((camera->screen_pixel_size.y * 0.5f) + ((unit_y - camera->center.y) * camera->unit_in_pixels));
	return pixel_y;
}

static V2
func UnitToPixel(Camera* camera, V2 unit)
{
	R32 x = (R32)UnitXtoPixel(camera, unit.x);
	R32 y = (R32)UnitYtoPixel(camera, unit.y);
	V2 result = MakePoint(x, y);
	return result;
}

static void
func ResizeCamera(Camera* camera, I32 width, I32 height)
{
	camera->screen_pixel_size.x = (R32)width;
	camera->screen_pixel_size.y = (R32)height;
}

static U32*
func GetPixelAddress(Bitmap bitmap, I32 row, I32 col)
{
	U32* address = bitmap.memory + row * bitmap.width + col;
	return address;
}

static U32
func GetPixel(Bitmap bitmap, I32 row, I32 col)
{
	U32* pixel_address = GetPixelAddress(bitmap, row, col);
	return *pixel_address;
}

static void
func SetPixel(Bitmap bitmap, I32 row, I32 col, U32 color_code)
{
	U32* pixel_address = GetPixelAddress(bitmap, row, col);
	*pixel_address = color_code;
}

static void
func SetPixelCheck(Bitmap bitmap, I32 row, I32 col, I32 color_code)
{
	if((row >= 0 && row < bitmap.height) && (col >= 0 && col < bitmap.width))
	{
		SetPixel(bitmap, row, col, color_code);
	}
}

static V4
func ColorProd(V4 color1, V4 color2)
{
	V4 result = {};
	result.red   = color1.red   * color2.red;
	result.green = color1.green * color2.green;
	result.blue  = color1.blue  * color2.blue;
	return result;
}

static void
func ApplyBitmapMask(Bitmap bitmap, Bitmap mask)
{
	U32* pixel = bitmap.memory;
	U32* mask_pixel = mask.memory;

	for(I32 row = 0; row < bitmap.height; row++) 
	{
		for(I32 col = 0; col < bitmap.width; col++) 
		{
			V4 color1 = GetColorFromColorCode(*pixel);
			V4 color2 = GetColorFromColorCode(*mask_pixel);
			V4 color = ColorProd(color1, color2);
			U32 code = GetColorCode(color);
			*pixel = code;

			pixel++;
			mask_pixel++;
		}
	}
}

struct PixelPosition
{
	I32 row;
	I32 col;
};

static void
func SmoothZoom(Camera* camera, R32 pixel_per_unit)
{
	camera->target_unit_in_pixels = pixel_per_unit;
}

#define PixelPerUnitChangeSpeed 10.0f

static void
func UpdateCamera(Camera* camera, R32 seconds)
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

static R32
func GetUnitDistanceInPixel(Camera* camera, R32 unit_distance)
{
	R32 pixel_distance = unit_distance * camera->unit_in_pixels;
	return pixel_distance;
}

static R32
func GetPixelDistanceInUnit(Camera* camera, R32 pixel_distance)
{
	Assert(camera->unit_in_pixels > 0.0f);
	R32 unit_distance = pixel_distance / camera->unit_in_pixels;
	return unit_distance;
}

static R32
func PixelToUnitX(Camera* camera, R32 pixel_x)
{
	R32 pixel_in_units = Invert(camera->unit_in_pixels);
	R32 unit_x = camera->center.x + (pixel_x - camera->screen_pixel_size.x * 0.5f) * pixel_in_units;
	return unit_x;
}

static R32
func PixelToUnitY(Camera* camera, R32 pixel_y) 
{
	R32 pixel_in_units = Invert(camera->unit_in_pixels);
	R32 unit_y = camera->center.y + (pixel_y - camera->screen_pixel_size.y * 0.5f) * pixel_in_units;
	return unit_y;
}

static V2
func PixelToUnit(Camera* camera, IV2 pixel)
{
	V2 result = {};
	result.x = PixelToUnitX(camera, (R32)pixel.col);
	result.y = PixelToUnitY(camera, (R32)pixel.row);
	return result;
}

static V2
func PixelToUnit(Camera* camera, V2 pixel)
{
	V2 result = {};
	result.x = PixelToUnitX(camera, pixel.x);
	result.y = PixelToUnitY(camera, pixel.y);
	return result;
}

static R32
func CameraLeftSide(Camera* camera)
{
	R32 left_side = PixelToUnitX(camera, 0);
	return left_side;
}

static R32
func CameraRightSide(Camera* camera)
{
	R32 right_side = PixelToUnitX(camera, camera->screen_pixel_size.x - 1);
	return right_side;
}

static R32
func CameraTopSide(Camera* camera)
{
	R32 top_side = PixelToUnitY(camera, 0);
	return top_side;
}

static R32
func CameraBottomSide(Camera* camera)
{
	R32 bottom_side = PixelToUnitY(camera, camera->screen_pixel_size.y - 1);
	return bottom_side;
}

static V2
func GetCameraTopLeftCorner(Camera* camera)
{
	V2 corner = {};
	corner.x = CameraLeftSide(camera);
	corner.y = CameraTopSide(camera);
	return corner;
}

static V2
func GetCameraBottomRightCorner(Camera* camera)
{
	V2 corner = {};
	corner.x = CameraRightSide(camera);
	corner.y = CameraBottomSide(camera);
	return corner;
}

static void
func ClearScreen(Canvas* canvas, V4 color)
{
	U32 color_code = GetColorCode(color);

	Bitmap bitmap = canvas->bitmap;
	U32 *pixel = bitmap.memory;
	for(I32 row = 0; row < bitmap.height; row++) 
	{
		for(I32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

struct BresenhamContext 
{
	I32 x1, y1;
	I32 x2, y2;
	I32 abs_x, abs_y;
	I32 add_x, add_y;
	I32 error, error2;
};

static BresenhamContext
func BresenhamInitPixel(V2 pixel_point1, V2 pixel_point2)
{
	BresenhamContext context = {};

	context.x1 = (I32)pixel_point1.x;
	context.y1 = (I32)pixel_point1.y;
	context.x2 = (I32)pixel_point2.x;
	context.y2 = (I32)pixel_point2.y;

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
func BresenhamInitUnit(Canvas* canvas, V2 point1, V2 point2)
{
	Camera* camera = canvas->camera;
	V2 pixel_point1 = UnitToPixel(camera, point1);
	V2 pixel_point2 = UnitToPixel(camera, point2);
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
func Bresenham(Canvas* canvas, V2 point1, V2 point2, V4 color)
{
	Bitmap bitmap = canvas->bitmap;
	U32 color_code = GetColorCode(color);

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

static V2
func LineAtX(V2 line_point1, V2 line_point2, R32 x)
{
	R32 minX = Min2(line_point1.x, line_point2.x);
	R32 maxX = Max2(line_point1.x, line_point2.x);
	Assert((minX <= x) && (x <= maxX));

	if(line_point1.x == line_point2.x)
	{
		return line_point1;
	}

	R32 alpha = (x - line_point1.x) / (line_point2.x - line_point1.x);
	V2 result = {};
	result.x = x;
	result.y = line_point1.y + alpha * (line_point2.y - line_point1.y);
	return result;
}

static V2
func LineAtY(V2 line_point1, V2 line_point2, R32 y)
{
	R32 minY = Min2(line_point1.y, line_point2.y);
	R32 maxY = Max2(line_point1.y, line_point2.y);
	Assert((minY <= y) && (y <= maxY));
	
	if(line_point1.y == line_point2.y)
	{
		return line_point1;
	}

	R32 alpha = (y - line_point1.y) / (line_point2.y - line_point1.y);
	V2 result = {};
	result.x = line_point1.x + alpha * (line_point2.x - line_point1.x);
	result.y = y;
	return result;
}

static void
func DrawHorizontalTrapezoid(Canvas* canvas, V2 top_left, V2 top_right, V2 bottom_left, V2 bottom_right, V4 color)
{
	Camera* camera = canvas->camera;
	R32 camera_top    = CameraTopSide(camera);
	R32 camera_bottom = CameraBottomSide(camera);

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
	U32 color_code = GetColorCode(color);

	BresenhamContext left_line = BresenhamInitUnit(canvas, top_left, bottom_left);
	BresenhamContext right_line = BresenhamInitUnit(canvas, top_right, bottom_right);

	I32 top = UnitYtoPixel(camera, top_left.y);
	I32 bottom = UnitYtoPixel(camera, bottom_left.y);
	if(bottom < top)
	{
		return;
	}

	for(I32 row = top; row <= bottom; row++) 
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

		I32 left = left_line.x1;
		I32 right = right_line.x1;

		if(left < 0) 
		{
			left = 0;
		}
		if(right > bitmap.width - 1)
		{
			right = bitmap.width - 1;
		}

		U32 *pixel = GetPixelAddress(bitmap, row, left);
		for(I32 col = left; col <= right; col++) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

static void
func DrawVerticalTrapezoid(Canvas* canvas, V2 top_left, V2 top_right, V2 bottom_left, V2 bottom_right, V4 color)
{
	Camera* camera = canvas->camera;
	R32 camera_left  = CameraLeftSide(camera);
	R32 camera_right = CameraRightSide(camera);

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
	U32 color_code = GetColorCode(color);

	BresenhamContext top_line = BresenhamInitUnit(canvas, top_left, top_right);
	BresenhamContext bottom_line = BresenhamInitUnit(canvas, bottom_left, bottom_right);

	I32 left = UnitXtoPixel(camera, top_left.x);
	I32 right = UnitXtoPixel(camera, top_right.x);
	if(right < left)
	{
		return;
	}

	for(I32 col = left; col <= right; col++) 
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

		I32 top = top_line.y1;
		I32 bottom = bottom_line.y1;

		if(top < 0) 
		{
			top = 0;
		}

		if(bottom > bitmap.height - 1)
		{
			bottom = bitmap.height - 1;
		}

		U32* pixel = GetPixelAddress(bitmap, top, col);
		for(I32 row = top; row <= bottom; row++) 
		{
			*pixel = color_code;
			pixel += bitmap.width;
		}
	}
}

static void
func DrawCircle(Canvas *canvas, V2 center, R32 radius, V4 color)
{
	U32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;

	I32 center_x_pixel = UnitXtoPixel(camera, center.x);
	I32 center_y_pixel = UnitYtoPixel(camera, center.y);

	I32 left_pixel   = UnitXtoPixel(camera, center.x - radius);
	I32 right_pixel  = UnitXtoPixel(camera, center.x + radius);
	I32 top_pixel    = UnitYtoPixel(camera, center.y - radius);
	I32 bottom_pixel = UnitYtoPixel(camera, center.y + radius);

	if(top_pixel > bottom_pixel)
	{
		IntSwap(&top_pixel, &bottom_pixel);
	}

	if(left_pixel > right_pixel)
	{
		IntSwap(&left_pixel, &right_pixel);
	}

	I32 pixel_radius = (I32)(radius * camera->unit_in_pixels);
	I32 pixel_radius_square = pixel_radius * pixel_radius;

	Bitmap bitmap = canvas->bitmap;
	top_pixel    = IntMax2(top_pixel, 0);
	bottom_pixel = IntMin2(bottom_pixel, bitmap.height - 1);
	left_pixel   = IntMax2(left_pixel, 0);
	right_pixel  = IntMin2(right_pixel, bitmap.width - 1);

	for(I32 row = top_pixel; row < bottom_pixel; row++)
	{
		for(I32 col = left_pixel; col < right_pixel; col++)
		{
			U32 *pixel = bitmap.memory + row * bitmap.width + col;
			I32 pixel_distance_square = IntSquare(row - center_y_pixel) + IntSquare(col - center_x_pixel);
			if(pixel_distance_square <= pixel_radius_square)
			{
				*pixel = color_code;
			}
		}
	}
}

static void
func DrawCircleOutline(Canvas *canvas, V2 center, R32 radius, V4 color)
{
	I32 line_n = 20;
	R32 angle_advance = (2.0f * PI) / R32(line_n);
	R32 angle = 0.0f;
	for(I32 i = 0; i < line_n; i++)
	{
		R32 angle1 = angle;
		R32 angle2 = angle + angle_advance;
		V2 point1 = center + radius * RotationVector(angle1);
		V2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);

		angle += angle_advance;
	}
}

static void
func DrawRectLRTB(Canvas* canvas, R32 left, R32 right, R32 top, R32 bottom, V4 color)
{
	U32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;
	I32 top_pixel    = UnitYtoPixel(camera, top);
	I32 left_pixel   = UnitXtoPixel(camera, left);
	I32 bottom_pixel = UnitYtoPixel(camera, bottom);
	I32 right_pixel  = UnitXtoPixel(camera, right);

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

	for(I32 row = top_pixel; row < bottom_pixel; row++) 
	{
		for(I32 col = left_pixel; col < right_pixel; col++) 
		{
			U32 *pixel = bitmap.memory + row * bitmap.width + col;
			*pixel = color_code;
		}
	}
}

static void
func DrawRect(Canvas* canvas, Rect rect, V4 color)
{
	DrawRectLRTB(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void
func DrawGridLine(Canvas* canvas, V2 point1, V2 point2, V4 color, R32 line_width)
{
	R32 left   = 0.0f;
	R32 right  = 0.0f;
	R32 top    = 0.0f;
	R32 bottom = 0.0f;

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
func DrawQuad(Canvas *canvas, Quad quad, V4 color)
{
	U32 color_code = GetColorCode(color);

	V2 *points = quad.points;
	Camera *camera = canvas->camera;
	for(I32 i = 0; i < 4; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	I32 min_x = bitmap.width;
	I32 min_y = bitmap.height;
	I32 max_x = 0;
	I32 max_y = 0;

	for(I32 i = 0; i < 4; i++) 
	{
		I32 point_x = (I32)points[i].x;
		I32 point_y = (I32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	U32 *pixel = 0;
	for(I32 row = min_y; row < max_y; ++row) 
	{
		for(I32 col = min_x; col < max_x; ++col) 
		{
			V2 test_point = MakePoint(R32(col), R32(row));

			B32 draw_point = true;

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
				pixel = (U32 *)bitmap.memory + row * bitmap.width + col;
				*pixel = color_code;
			}
		}
	}
}

static void
func DrawLine(Canvas* canvas, V2 point1, V2 point2, V4 color, R32 line_width)
{
	V2 direction = PointDirection(point2, point1);

	R32 tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	R32 half_line_width = line_width * 0.5f;
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

	R32 pixel_in_units = Invert(camera->unit_in_pixels);

	R32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	R32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	R32 add_x = pixel_in_units;
	R32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
	I32 and_val = (texture.side - 1);

	U32 *pixel = bitmap.memory;

	I32 start_int_x = ((I32)start_x) & and_val;
	I32 start_int_y = ((I32)start_y) & and_val;

	U32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);

	R32 y = start_y;
	I32 prev_int_y = ((I32)y) & and_val;

	for(I32 row = 0; row < bitmap.height; row++) 
	{
		R32 x = start_x;
		I32 prev_int_x = (((I32)x) & and_val);
		U32 *texture_pixel = texture_row;

		for(I32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = *texture_pixel;
			pixel++;

			x += add_x;
			I32 int_x = ((I32)x) & and_val;
			texture_pixel += (int_x - prev_int_x);
			prev_int_x = int_x;
		}

		y += add_y;
		I32 int_y = ((I32)y) & and_val;
		texture_row += (int_y - prev_int_y) << texture.log_side;
		prev_int_y = int_y;
	}
}

static void
func DrawWorldTextureQuad(Canvas *canvas, Quad quad, Texture texture)
{
    Bitmap bitmap = canvas->bitmap;
	Camera *camera = canvas->camera;
    
    V2 *points = quad.points;
    for(I32 i = 0; i < 4; i++)
	{
        points[i] = UnitToPixel(camera, points[i]);
	}

	I32 min_x = bitmap.width;
	I32 min_y = bitmap.height;
	I32 max_x = 0;
	I32 max_y = 0;
   
	for(I32 i = 0; i < 4; i++) 
	{
		I32 point_x = (I32)points[i].x;
        I32 point_y = (I32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);
    
	R32 pixel_in_units = Invert(camera->unit_in_pixels);

	R32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	R32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	R32 add_x = pixel_in_units;
	R32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
    
    start_x += (min_x * add_x);
    start_y += (min_y * add_y);
    
    I32 and_val = (texture.side - 1);

	I32 start_int_x = ((I32)start_x) & and_val;
	I32 start_int_y = ((I32)start_y) & and_val;

	U32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);
	U32 *pixel_row = bitmap.memory + (bitmap.width * min_y) + (min_x);

	R32 y = start_y;
	I32 prev_int_y = ((I32)y) & and_val;

	for(I32 row = min_y; row < max_y; row++) 
	{
        R32 x = start_x;
		I32 prev_int_x = ((I32)x & and_val);
		U32 *texture_pixel = texture_row;
		U32 *pixel = pixel_row;
    
		for(I32 col = min_x; col < max_x; col++) 
		{
			V2 test_point = {};
            test_point.x = (R32)col;
            test_point.y = (R32)row;

			B32 draw_point = true;

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
			I32 int_x = ((I32)x) & and_val;
			texture_pixel += (int_x - prev_int_x);
			prev_int_x = int_x;
		}
        
        y += add_y;
		I32 int_y = ((I32)y) & and_val;
		texture_row += (int_y - prev_int_y) << texture.log_side;
		pixel_row += (bitmap.width);
		prev_int_y = int_y;
	}
}

static void
func DrawWorldTextureLine(Canvas *canvas, V2 point1, V2 point2, R32 line_width, Texture texture)
{
	V2 direction = PointDirection(point2, point1);
	V2 turned_direction = TurnVectorToRight(direction);
	
	R32 half_line_width = line_width * 0.5f;
	Quad quad = {};
	V2 point_product = (half_line_width * turned_direction);
	quad.points[0] = (point1 - point_product);
	quad.points[1] = (point1 + point_product);
	quad.points[2] = (point2 + point_product);
	quad.points[3] = (point2 - point_product);
	DrawWorldTextureQuad(canvas, quad, texture);
}

static void
func DrawRectLRTBOutline(Canvas *canvas, R32 left, R32 right, R32 top, R32 bottom, V4 color)
{
	V2 top_left     = MakePoint(left,  top);
	V2 top_right    = MakePoint(right, top);
	V2 bottom_left  = MakePoint(left,  bottom);
	V2 bottom_right = MakePoint(right, bottom);

	Bresenham(canvas, top_left,     top_right,    color);
	Bresenham(canvas, top_right,    bottom_right, color);
	Bresenham(canvas, bottom_right, bottom_left,  color);
	Bresenham(canvas, bottom_left,  top_left,     color);
}

static void
func DrawRectOutline(Canvas *canvas, Rect rect, V4 color)
{
	DrawRectLRTBOutline(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void
func WorldTextureRect(Canvas *canvas, R32 left, R32 right, R32 top, R32 bottom, Texture texture)
{
	Camera *camera = canvas->camera;
	I32 top_pixel =    UnitYtoPixel(camera, top);
	I32 left_pixel =   UnitXtoPixel(camera, left);
	I32 bottom_pixel = UnitYtoPixel(camera, bottom);
	I32 right_pixel =  UnitXtoPixel(camera, right);

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

	R32 texture_zoom = 16.0f;

	V2 top_left_pixel = MakePoint((R32)left_pixel, (R32)top_pixel);
	V2 top_left_unit = PixelToUnit(camera, top_left_pixel);
	// NOTE: optimization stuff
	top_left_unit.x *= texture_zoom;
	top_left_unit.y *= texture_zoom;

	R32 unit_per_pixel = Invert(camera->unit_in_pixels);
	// NOTE: optimization stuff
	unit_per_pixel *= texture_zoom;

	I32 sub_unit_per_pixel = (I32)(unit_per_pixel * 255.0f);

	U32 *top_left = bitmap.memory + top_pixel * bitmap.width + left_pixel;

	// TODO: try to optimize this code
	V2 world_unit = top_left_unit;
	I32 leftx  = ((I32)world_unit.x) & (texture.side - 1);
	I32 topy   = ((I32)world_unit.y) & (texture.side - 1);

	I32 leftsubx = (I32)((world_unit.x - Floor(world_unit.x)) * 255.0f);
	I32 topsuby = (I32)((world_unit.y - Floor(world_unit.y)) * 255.0f);

	I32 x = leftx;
	I32 y = topy;

	I32 subx = leftsubx;
	I32 suby = topsuby;

	for(I32 row = top_pixel; row < bottom_pixel; row++) 
	{
		x = leftx;
		subx = leftsubx;

		U32 *pixel = top_left;

		for(I32 col = left_pixel; col < right_pixel; col++) 
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
func WorldTextureGridLine(Canvas *canvas, V2 point1, V2 point2, R32 width, Texture texture)
{
	R32 left   = Min2(point1.x, point2.x);
	R32 right  = Max2(point1.x, point2.x);
	R32 top    = Min2(point1.y, point2.y);
	R32 bottom = Max2(point1.y, point2.y);

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
func DrawPolyOutline(Canvas *canvas, V2 *points, I32 point_n, V4 color)
{
	I32 prev = point_n - 1;
	for(I32 i = 0; i < point_n; i++) 
	{
		Bresenham(canvas, points[prev], points[i], color);
		prev = i;
	}
}

static void
func DrawPoly(Canvas *canvas, V2 *points, I32 point_n, V4 color)
{
	U32 color_code = GetColorCode(color);

	Camera *camera = canvas->camera;
	for(I32 i = 0; i < point_n; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	I32 min_x = bitmap.width;
	I32 min_y = bitmap.height;
	I32 max_x = 0;
	I32 max_y = 0;

	for(I32 i = 0; i < point_n; i++) 
	{
		I32 point_x = (I32)points[i].x;
		I32 point_y = (I32)points[i].y;

		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	U32 *pixel = 0;
	for(I32 row = min_y; row < max_y; row++) 
	{
		for(I32 col = min_x; col < max_x; col++) 
		{
			V2 test_point = MakePoint(R32(col), R32(row));

			B32 draw_point = true;

			I32 prev = point_n - 1;
			for(I32 i = 0; i < point_n; i++) 
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
				pixel = (U32 *)bitmap.memory + row * bitmap.width + col;
				*pixel = color_code;
			}
		}
	}

	for(I32 i = 0; i < point_n; ++i)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void
func DrawWorldTexturePoly(Canvas *canvas, V2 *points, I32 point_n, Texture texture)
{
	Bitmap bitmap = canvas->bitmap;
	Camera *camera = canvas->camera;

	I32 min_x = bitmap.width;
	I32 min_y = bitmap.height;
	I32 max_x = 0;
	I32 max_y = 0;

	for(I32 i = 0; i < point_n; i++) 
	{
		V2 point_in_pixels = UnitToPixel(camera, points[i]);
		I32 point_x = I32(point_in_pixels.x);
		I32 point_y = I32(point_in_pixels.y);
		min_x = IntMin2(min_x, point_x);
		max_x = IntMax2(max_x, point_x);
		min_y = IntMin2(min_y, point_y);
		max_y = IntMax2(max_y, point_y);
	}

	min_x = IntMax2(min_x, 0);
	max_x = IntMin2(max_x, bitmap.width - 1);
	min_y = IntMax2(min_y, 0);
	max_y = IntMin2(max_y, bitmap.height - 1);

	R32 pixel_in_units = Invert(camera->unit_in_pixels);
    
    R32 start_x = camera->center.x - (camera->screen_pixel_size.x * 0.5f * pixel_in_units);
	R32 start_y = camera->center.y - (camera->screen_pixel_size.y * 0.5f * pixel_in_units);
	start_x *= WorldTextureScale;
	start_y *= WorldTextureScale;

	R32 add_x = pixel_in_units;
	R32 add_y = pixel_in_units;
	add_x *= WorldTextureScale;
	add_y *= WorldTextureScale;
    
    start_x += (min_x * add_x);
    start_y += (min_y * add_y);
    
    I32 and_val = (texture.side - 1);

	I32 start_int_x = ((I32)start_x) & and_val;
	I32 start_int_y = ((I32)start_y) & and_val;

	U32 *texture_row = texture.memory + (start_int_y << texture.log_side) + (start_int_x);
	U32 *pixel_row = bitmap.memory + (bitmap.width * min_y) + (min_x);

	R32 y = start_y;
	I32 prev_int_y = (I32(y)) & and_val;

	U32 *pixel = 0;
	for(I32 row = min_y; row <= max_y; row++) 
	{
        R32 x = start_x;
		I32 prev_int_x = (I32(x) & and_val);
		U32 *texture_pixel = texture_row;
		U32 *pixel = pixel_row;
    
		for(I32 col = min_x; col <= max_x; col++) 
		{
            V2 test_point = {};
            test_point.x = R32(col);
            test_point.y = R32(row);

			B32 draw_point = true;

			I32 prev = point_n - 1;
			for(I32 i = 0; i < point_n; i++) 
			{
				V2 prev_point = UnitToPixel(camera, points[prev]);
				V2 this_point = UnitToPixel(camera, points[i]);
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
            I32 int_x = ((I32)x) & and_val;
            texture_pixel += (int_x - prev_int_x);
            prev_int_x = int_x;
		}
        
        y += add_y;
        I32 int_y = ((I32)y) & and_val;
        texture_row += (int_y - prev_int_y) << texture.log_side;
        pixel_row += (bitmap.width);
        prev_int_y = int_y;
	}

	for(I32 i = 0; i < point_n; i++)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void
func DrawQuadPoints(Canvas *canvas, V2 point1, V2 point2, V2 point3, V2 point4, V4 color)
{
	Quad quad = MakeQuad(point1, point2, point3, point4);
	DrawQuad(canvas, quad, color);
}

static void
func DrawBitmap(Canvas *canvas, Bitmap *bitmap, R32 left, R32 top)
{
	Camera *camera = canvas->camera;
	I32 pixel_left = UnitXtoPixel(camera, left);
	I32 pixel_top  = UnitYtoPixel(camera, top);
	CopyBitmap(bitmap, &canvas->bitmap, pixel_left, pixel_top);
}

static R32
func GetTextHeight(Canvas *canvas, I8 *text)
{
	Camera *camera = canvas->camera;
	Assert(camera->unit_in_pixels > 0.0f);
	R32 height = (R32)TextHeightInPixels / camera->unit_in_pixels;
	return height;
}

static R32
func GetTextWidth(Canvas *canvas, I8 *text)
{
	Assert(canvas->glyph_data != 0);
	R32 pixel_width = GetTextPixelWidth(text, canvas->glyph_data);

	Camera *camera = canvas->camera;
	Assert(camera->unit_in_pixels > 0.0f);
	R32 width = pixel_width / camera->unit_in_pixels;
	return width;
}

static void
func DrawTextLine(Canvas *canvas, I8 *text, R32 base_line_y, R32 left, V4 text_color)
{
	Assert(canvas->glyph_data != 0);
	I32 left_pixel = UnitXtoPixel(canvas->camera, left);
	I32 base_line_y_pixel = UnitYtoPixel(canvas->camera, base_line_y);
	DrawBitmapTextLine(&canvas->bitmap, text, canvas->glyph_data, left_pixel, base_line_y_pixel, text_color);
}

static void
func DrawTextLineXCentered(Canvas *canvas, I8 *text, R32 base_line_y, R32 center_x, V4 text_color)
{
	Assert(canvas->glyph_data != 0);
	R32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}

static void
func DrawTextLineXYCentered(Canvas *canvas, I8 *text, R32 center_y, R32 center_x, V4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Camera *camera = canvas->camera;

	R32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	R32 text_height = GetPixelDistanceInUnit(camera, TextHeightInPixels);
	R32 text_top = center_x - text_height * 0.5f;
	R32 base_line_y = text_top + GetPixelDistanceInUnit(camera, TextPixelsAboveBaseLine);
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}

static void
func DrawTextLineBottomXCentered(Canvas *canvas, I8 *text, R32 bottom, R32 center_x, V4 text_color)
{
	Assert(canvas->glyph_data != 0);
	Camera *camera = canvas->camera;

	R32 left = center_x - GetTextWidth(canvas, text) * 0.5f;
	R32 text_height = GetPixelDistanceInUnit(camera, TextHeightInPixels);
	R32 text_top = bottom - text_height;
	R32 base_line_y = text_top + GetPixelDistanceInUnit(camera, TextPixelsAboveBaseLine);
	DrawTextLine(canvas, text, base_line_y, left, text_color);
}