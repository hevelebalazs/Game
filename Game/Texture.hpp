#pragma once

#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Texture 
{
	I32 side; // NOTE: power of two
	I32 logSide;
	U32* memory;
};

Texture CopyTexture(Texture* texture);
void Swap2(U32* i1, U32* i2);
void Swap4(U32* i1, U32* i2, U32* i3, U32* i4); 
U32* TextureAddress(Texture* texture, I32 row, I32 col);
U32 TextureValue(Texture* texture, I32 row, I32 col);
void RotateTextureUpsideDown(Texture* texture);
void RotateTextureLeft(Texture* texture);
void RotateTextureRight(Texture* texture);
Texture RoofTexture(I32 logSide);
Texture GrassTexture(I32 logSide, MemArena* tmpArena);
Texture RandomGreyTexture(I32 logSide, I32 minRatio, I32 maxRatio);
U32 TextureColorCodeInt(Texture texture, I32 row, I32 col);
V4 TextureColorInt(Texture texture, I32 row, I32 col);
V4 TextureColor(Texture texture, F32 x, F32 y);
U32 TextureColorCode(Texture texture, F32 x, F32 y);
U32 ColorCodeLerp(U32 colorCode1, U8 ratio, U32 colorCode2);
U32 TextureColorCode(Texture texture, I32 x, U8 subx, I32 y, U8 suby);