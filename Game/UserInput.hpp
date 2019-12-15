#pragma once

#include <Windows.h>

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct UserInput
{
	Bool32 is_key_down[256];
	Int32 key_toggle_count[256];
	IntVec2 mouse_pixel_position;
};

static void
func ResetKeyToggleCounts(UserInput *user_input)
{
	for(Int32 i = 0; i < 256; i++)
	{
		user_input->key_toggle_count[i] = 0;
	}
}

static Bool32
func IsKeyDown(UserInput *user_input, UInt8 key_code)
{
	Bool32 is_down = user_input->is_key_down[key_code];
	return is_down;
}

static Bool32
func WasKeyToggled(UserInput *user_input, UInt8 key_code)
{
	Bool32 was_toggled = (user_input->key_toggle_count[key_code] > 0);
	return was_toggled;
}

static Bool32
func WasKeyPressed(UserInput *user_input, UInt8 key_code)
{
	Bool32 was_pressed = IsKeyDown(user_input, key_code) && WasKeyToggled(user_input, key_code);
	return was_pressed;
}

static Bool32
func WasKeyReleased(UserInput *user_input, UInt8 key_code)
{
	Bool32 was_released = !IsKeyDown(user_input, key_code) && WasKeyToggled(user_input, key_code);
	return was_released;
}