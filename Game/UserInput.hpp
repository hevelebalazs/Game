#pragma once

#include <Windows.h>

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct UserInput
{
	Bool32 isKeyDown[256];
	Int32 keyToggleCount[256];
	IntVec2 mousePixelPosition;
};

static void
func ResetKeyToggleCounts(UserInput* userInput)
{
	for(Int32 i = 0; i < 256; i++)
	{
		userInput->keyToggleCount[i] = 0;
	}
}

static Bool32
func IsKeyDown(UserInput* userInput, UInt8 keyCode)
{
	Bool32 isDown = userInput->isKeyDown[keyCode];
	return isDown;
}

static Bool32
func WasKeyToggled(UserInput* userInput, UInt8 keyCode)
{
	Bool32 wasToggled = (userInput->keyToggleCount[keyCode] > 0);
	return wasToggled;
}

static Bool32
func WasKeyPressed(UserInput* userInput, UInt8 keyCode)
{
	Bool32 wasPressed = IsKeyDown(userInput, keyCode) && WasKeyToggled(userInput, keyCode);
	return wasPressed;
}

static Bool32
func WasKeyReleased(UserInput* userInput, UInt8 keyCode)
{
	Bool32 wasReleased = !IsKeyDown(userInput, keyCode) && WasKeyToggled(userInput, keyCode);
	return wasReleased;
}