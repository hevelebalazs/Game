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
	Int8 *baseAddress;
	UInt32 usedSize;
	UInt32 maxSize;
};

static MemArena
func CreateMemArena(void *memory, UInt32 size)
{
	MemArena arena = {};
	arena.baseAddress = (Int8 *)memory;
	arena.maxSize = size;
	arena.usedSize = 0;
	return arena;
}

static void
func ArenaReset(MemArena *arena)
{
	arena->usedSize = 0;
}

static void *
func ArenaAlloc(MemArena *arena, UInt32 size)
{
	Assert(arena->usedSize + size <= arena->maxSize);
	Int8 *result = arena->baseAddress + arena->usedSize;
	arena->usedSize += size;
	return result;
}

static UInt32
func GetArenaSize(MemArena *arena)
{
	UInt32 result = arena->usedSize;
	return result;
}

static void
func SetArenaSize(MemArena *arena, UInt32 size)
{
	arena->usedSize = size;
}

static void
func ArenaPopTo(MemArena *arena, void *address)
{
	Assert(arena->baseAddress <= address);
	Assert(address < arena->baseAddress + arena->usedSize);
	arena->usedSize = (UInt32)((Int8 *)address - arena->baseAddress);
}

static Int8 *
func GetArenaTop(MemArena *arena)
{
	Int8 *top = arena->baseAddress + arena->usedSize;
	return top;
}

static Int8 *
func ArenaPushData(MemArena *arena, UInt32 size, void* data)
{
	Int8 *copyTo = (Int8*)ArenaAlloc(arena, size);
	Int8 *copyFrom = (Int8*)data;
	for(UInt32 index = 0; index < size; index++)
	{
		copyTo[index] = copyFrom[index];
	}

	return copyTo;
}

#define ArenaAllocType(arena, type) ((type *)ArenaAlloc((arena), sizeof(type)))
#define ArenaAllocArray(arena, type, size) ((type *)ArenaAlloc((arena), (size) * sizeof(type)))

/*
#define ArenaPush(arena, type, variable) {\
			type *address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
*/
#define ArenaPushVar(arena, variable) ArenaPushData(arena, sizeof(variable), &(variable))

static MemArena
func CreateSubArena(MemArena *arena, UInt32 size)
{
	MemArena result = {};
	result.baseAddress = (Int8 *)ArenaAlloc(arena, size);
	result.usedSize = 0;
	result.maxSize = size;
	return result;
}

#define GetRelativeAddress(absoluteAddress, base) ((UInt64)(absoluteAddress) - (UInt64)(base))
#define GetAbsoluteAddress(relativeAddress, base) ((UInt64)(relativeAddress) + (UInt64)(base))
