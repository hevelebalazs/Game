#pragma once

#include <stdlib.h>

#include "Debug.hpp"
#include "Type.hpp"

#define Byte (8)
#define KiloByte (1024 * Byte)
#define MegaByte (1024 * KiloByte)
#define GigaByte (1024 * MegaByte)

struct MemArena 
{
	Int8* baseAddress;
	UInt32 usedSize;
	UInt32 maxSize;
};

static MemArena func CreateMemArena(void* memory, UInt32 size)
{
	MemArena arena = {};
	arena.baseAddress = (Int8*)memory;
	arena.maxSize = size;
	arena.usedSize = 0;
	return arena;
}

static MemArena func CreateMemArena(UInt32 maxSize)
{
	MemArena result = {};
	result.baseAddress = new Int8[maxSize];
	result.maxSize = maxSize;
	result.usedSize = 0;

	return result;
}

static void func ArenaReset(MemArena* arena)
{
	arena->usedSize = 0;
}

static void* func ArenaAlloc(MemArena* arena, UInt32 size)
{
	Assert(arena->usedSize + size <= arena->maxSize);
	if(arena->usedSize + size <= arena->maxSize) 
	{
		Int8* result = arena->baseAddress + arena->usedSize;
		arena->usedSize += size;
		return result;
	}
	else
	{
		return 0;
	}
}

static UInt32 func GetArenaSize(MemArena* arena)
{
	UInt32 result = arena->usedSize;
	return result;
}

static void func SetArenaSize(MemArena* arena, UInt32 size)
{
	arena->usedSize = size;
}

// TODO: only pop if the address is smaller than the current one?
static void func ArenaPopTo(MemArena* arena, void* address)
{
	arena->usedSize = (UInt32)((Int8*)address - arena->baseAddress);
}

static Int8* func GetArenaTop(MemArena* arena)
{
	Int8* top = arena->baseAddress + arena->usedSize;
	return top;
}

#define ArenaPushType(arena, type) ((type*)ArenaAlloc((arena), sizeof(type)))
#define ArenaPushArray(arena, type, size) ((type*)ArenaAlloc((arena), (size) * sizeof(type)))
#define ArenaPush(arena, type, variable) {\
			type* address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
#define ArenaPushVar(arena, variable) {  \
			Int8* address = (Int8*)ArenaAlloc((arena), sizeof(variable)); \
		    memcpy((address), &(variable), sizeof(variable)); }