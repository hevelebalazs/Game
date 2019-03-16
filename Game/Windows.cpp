#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Type.hpp"

#include "Lab/CombatLab.hpp"
#include "Lab/TextLab.hpp"
#include "Lab/ThreadLab.hpp"
#include "Lab/WorldLab.hpp"

I32 CALLBACK func WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, I32 cmdShow)
{
	WorldLab(instance);
	return 0;
}