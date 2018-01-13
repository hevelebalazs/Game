#pragma once

#include <windows.h>

#include "Debug.h"

// TODO: is having a #define better than a function for this?
void Assert(bool value) {
	if (!value)
		DebugBreak ();
}