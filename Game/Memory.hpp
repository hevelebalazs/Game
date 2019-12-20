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
	Int8 *base_address;
	UInt32 used_size;
	UInt32 max_size;
};

static MemArena
func CreateMemArena(void *memory, UInt32 size)
{
	MemArena arena = {};
	arena.base_address = (Int8 *)memory;
	arena.max_size = size;
	arena.used_size = 0;
	return arena;
}

static void
func ArenaReset(MemArena *arena)
{
	arena->used_size = 0;
}

static void *
func ArenaAlloc(MemArena *arena, UInt32 size)
{
	Assert(arena->used_size + size <= arena->max_size);
	Int8 *result = arena->base_address + arena->used_size;
	arena->used_size += size;
	return result;
}

static UInt32
func GetArenaSize(MemArena *arena)
{
	UInt32 result = arena->used_size;
	return result;
}

static void
func SetArenaSize(MemArena *arena, UInt32 size)
{
	arena->used_size = size;
}

static void
func ArenaPopTo(MemArena *arena, void *address)
{
	Assert(arena->base_address <= address);
	Assert(address < arena->base_address + arena->used_size);
	arena->used_size = (UInt32)((Int8 *)address - arena->base_address);
}

static Int8 *
func GetArenaTop(MemArena *arena)
{
	Int8 *top = arena->base_address + arena->used_size;
	return top;
}

static Int8 *
func ArenaPushData(MemArena *arena, UInt32 size, void* data)
{
	Int8 *copy_to = (Int8*)ArenaAlloc(arena, size);
	Int8 *copy_from = (Int8*)data;
	for(UInt32 index = 0; index < size; index++)
	{
		copy_to[index] = copy_from[index];
	}

	return copy_to;
}

static Bool32
func ArenaContainsAddress(MemArena *arena, void *address)
{
	Int8 *bottom = arena->base_address;
	Int8 *top = arena->base_address + arena->used_size;
	Int8 *test_address = (Int8 *)address;
	Bool32 is_in_arena = (bottom <= test_address && test_address <= top);
	return is_in_arena;
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
	result.base_address = (Int8 *)ArenaAlloc(arena, size);
	result.used_size = 0;
	result.max_size = size;
	return result;
}

#define GetRelativeAddress(absolute_address, base) ((UInt64)(absolute_address) - (UInt64)(base))
#define GetAbsoluteAddress(relative_address, base) ((UInt64)(relative_address) + (UInt64)(base))
