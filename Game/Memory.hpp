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
	I8 *base_address;
	U32 used_size;
	U32 max_size;
};

static MemArena
func CreateMemArena(void *memory, U32 size)
{
	MemArena arena = {};
	arena.base_address = (I8 *)memory;
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
func ArenaAlloc(MemArena *arena, U32 size)
{
	Assert(arena->used_size + size <= arena->max_size);
	I8 *result = arena->base_address + arena->used_size;
	arena->used_size += size;
	return result;
}

static U32
func GetArenaSize(MemArena *arena)
{
	U32 result = arena->used_size;
	return result;
}

static void
func SetArenaSize(MemArena *arena, U32 size)
{
	arena->used_size = size;
}

static void
func ArenaPopTo(MemArena *arena, void *address)
{
	Assert(arena->base_address <= address);
	Assert(address < arena->base_address + arena->used_size);
	arena->used_size = (U32)((I8 *)address - arena->base_address);
}

static I8 *
func GetArenaTop(MemArena *arena)
{
	I8 *top = arena->base_address + arena->used_size;
	return top;
}

static I8 *
func ArenaPushData(MemArena *arena, U32 size, void* data)
{
	I8 *copy_to = (I8*)ArenaAlloc(arena, size);
	I8 *copy_from = (I8*)data;
	for(U32 index = 0; index < size; index++)
	{
		copy_to[index] = copy_from[index];
	}

	return copy_to;
}

#define ArenaAllocType(arena, type) ((type *)ArenaAlloc((arena), sizeof(type))
#define ArenaAllocArray(arena, type, size) ((type *)ArenaAlloc((arena), (size) * sizeof(type)))

/*
#define ArenaPush(arena, type, variable) {\
			type *address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
*/
#define ArenaPushVar(arena, variable) ArenaPushData(arena, sizeof(variable), &(variable))

static MemArena
func CreateSubArena(MemArena *arena, U32 size)
{
	MemArena result = {};
	result.base_address = (I8 *)ArenaAlloc(arena, size);
	result.used_size = 0;
	result.max_size = size;
	return result;
}

#define GetRelativeAddress(absolute_address, base) ((U64)(absolute_address) - (U64)(base))
#define GetAbsoluteAddress(relative_address, base) ((U64)(relative_address) + (U64)(base))
