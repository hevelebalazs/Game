#pragma once

#include "Debug.hpp"
#include "Type.hpp"

#define AddLine(string, value) {string = string + value + "\n";}
#define OneLineString(memory, maxSize, line) {StartString(memory, maxSize) + line;}

struct String
{
	I8* buffer;
	I32 bufferSize;
	I32 usedSize;
};

static void CloseString(String* string);

static B32 func StringIsTerminated(I8* string, I32 length)
{
	B32 isTerminated = false;
	for(I32 i = 0; i < length; i++)
	{
		if(string[i] == 0)
		{
			isTerminated = true;
			break;
		}
	}
	return isTerminated;
}

static void func StringCopy(I8* from, I8* to, I32 maxSize)
{
	B32 terminated = false;
	for(I32 i = 0; i < maxSize; i++)
	{
		to[i] = from[i];
		if(from[i] == 0)
		{
			terminated = true;
			break;
		}
	}
	Assert(terminated);
}

String func StartString(I8* buffer, I32 bufferSize)
{
	Assert(bufferSize > 0);
	String string = {};
	string.buffer = buffer;
	string.bufferSize = bufferSize;
	string.usedSize = 0;
	return string;
}

static void func AddChar(String* string, I8 value)
{
	Assert(string->usedSize + 1 < string->bufferSize);
	string->buffer[string->usedSize] = value;
	string->usedSize++;

	CloseString(string);
}

static I32 func GetNumberOfDigits(I32 value)
{
	Assert(value > 0);
	I32 result = 0;
	while(value > 0)
	{
		result++;
		value = value / 10;
	}
	return result;
}

static I32 func GetLastDigit(I32 value)
{
	Assert(value >= 0);
	I32 result = value % 10;
	return result;
}

I32 CutLastDigit(I32 value)
{
	Assert(value > 0);
	I32 result = value / 10;
	return result;
}

static void func AddInt(String* string, I32 value)
{
	if(value == 0)
	{
		AddChar(string, '0');
	}
	else
	{
		I32 digitN = GetNumberOfDigits(value);
		I32 position = string->usedSize + digitN - 1;
		Assert(position < string->bufferSize);
		while(value > 0)
		{
			Assert(position >= string->usedSize);
			string->buffer[position] = I8('0' + GetLastDigit(value));
			position--;
			value = CutLastDigit(value);
		}
		string->usedSize += digitN;
	}

	CloseString(string);
}

static void func AddFloat(String* string, F32 value)
{
	Assert(value >= 0.0f);
	AddInt(string, I32(value));
	AddChar(string, '.');
	
	I8 digit = 0;

	digit = I32(value * 10) % 10;
	Assert(IsIntBetween(digit, 0, 9));
	AddChar(string, '0' + digit);
	value = value * 10.0f;

	digit = I32(value * 10) % 10;
	Assert(IsIntBetween(digit, 0, 9));
	AddChar(string, '0' + digit);
	value = value * 10.0f;
}

static void func AddText(String* string, I8* text)
{
	for(I32 i = 0; text[i]; i++)
	{
		AddChar(string, text[i]);
	}

	CloseString(string);
}

static void func CloseString(String* string)
{
	Assert(string->usedSize + 1 < string->bufferSize);
	string->buffer[string->usedSize] = 0;
}

static String func operator+(String string, I8 value)
{
	String result = string;
	AddChar(&result, value);
	return result;
}

static String func operator+(String string, I32 value)
{
	String result = string;
	AddInt(&result, value);
	return result;
}

static String func operator+(String string, F32 value)
{
	String result = string;
	AddFloat(&result, value);
	return result;
}

static String func operator+(String string, I8* text)
{
	String result = string;
	AddText(&result, text);
	return result;
}

static I32 func GetNumberOfLines(String string)
{
	I32 lineN = 0;

	for(I32 i = 0; i < string.usedSize; i++)
	{
		if(string.buffer[i] == '\n')
		{
			lineN++;
		}
	}

	if(string.usedSize > 0 && string.buffer[string.usedSize - 1] != '\n')
	{
		lineN++;
	}

	Assert(lineN >= 0);
	return lineN;
}

// TODO: Handle AddInt when the value is negative!
// TODO: Handle AddInt when the value is 0!