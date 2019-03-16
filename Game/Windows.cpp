#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Type.hpp"

#include "Lab/CombatLab.hpp"
#include "Lab/TextLab.hpp"
#include "Lab/ThreadLab.hpp"

Int32 CALLBACK func WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, Int32 cmdShow)
{
	int x = 10;
	CombatLab(instance);
	return 0;
}