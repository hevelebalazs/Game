#pragma once

#include <Windows.h>

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct UserInput
{
	B32 is_key_down[256];
	I32 key_toggle_count[256];
	IV2 mouse_pixel_position;
};

static void
func ResetKeyToggleCounts(UserInput *user_input)
{
	for(I32 i = 0; i < 256; i++)
	{
		user_input->key_toggle_count[i] = 0;
	}
}

static B32
func IsKeyDown(UserInput *user_input, U8 key_code)
{
	B32 is_down = user_input->is_key_down[key_code];
	return is_down;
}

static B32
func WasKeyToggled(UserInput *user_input, U8 key_code)
{
	B32 was_toggled = (user_input->key_toggle_count[key_code] > 0);
	return was_toggled;
}

static B32
func WasKeyPressed(UserInput *user_input, U8 key_code)
{
	B32 was_pressed = IsKeyDown(user_input, key_code) && WasKeyToggled(user_input, key_code);
	return was_pressed;
}

static B32
func WasKeyReleased(UserInput *user_input, U8 key_code)
{
	B32 was_released = !IsKeyDown(user_input, key_code) && WasKeyToggled(user_input, key_code);
	return was_released;
}