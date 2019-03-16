#pragma once

#include "Debug.hpp"
#include "Type.hpp"

#define AddLine(string, value) {string = string + value + "\n";}
#define OneLineString(memory, maxSize, line) {StartString(memory, maxSize) + line;}

struct String
{
	Int8* buffer;
	Int32 bufferSize;
	Int32 usedSize;
};

static void CloseString(String* string);

static Bool32 func StringIsTerminated(Int8* string, Int32 length)
{
	Bool32 isTerminated = false;
	for(Int32 i = 0; i < length; i++)
	{
		if(string[i] == 0)
		{
			isTerminated = true;
			break;
		}
	}
	return isTerminated;
}

static void func StringCopy(Int8* from, Int8* to, Int32 maxSize)
{
	Bool32 terminated = false;
	for(Int32 i = 0; i < maxSize; i++)
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

String func StartString(Int8* buffer, Int32 bufferSize)
{
	Assert(bufferSize > 0);
	String string = {};
	string.buffer = buffer;
	string.bufferSize = bufferSize;
	string.usedSize = 0;
	return string;
}

static void func AddChar(String* string, Int8 value)
{
	Assert(string->usedSize + 1 < string->bufferSize);
	string->buffer[string->usedSize] = value;
	string->usedSize++;

	CloseString(string);
}

static Int32 func GetNumberOfDigits(Int32 value)
{
	Assert(value > 0);
	Int32 result = 0;
	while(value > 0)
	{
		result++;
		value = value / 10;
	}
	return result;
}

static Int32 func GetLastDigit(Int32 value)
{
	Assert(value >= 0);
	Int32 result = value % 10;
	return result;
}

Int32 CutLastDigit(Int32 value)
{
	Assert(value > 0);
	Int32 result = value / 10;
	return result;
}

static void func AddInt(String* string, Int32 value)
{
	if(value == 0)
	{
		AddChar(string, '0');
	}
	else
	{
		Int32 digitN = GetNumberOfDigits(value);
		Int32 position = string->usedSize + digitN - 1;
		Assert(position < string->bufferSize);
		while(value > 0)
		{
			Assert(position >= string->usedSize);
			string->buffer[position] = Int8('0' + GetLastDigit(value));
			position--;
			value = CutLastDigit(value);
		}
		string->usedSize += digitN;
	}

	CloseString(string);
}

static void func AddFloat(String* string, Real32 value)
{
	Assert(value >= 0.0f);
	AddInt(string, (Int32)value);
	AddChar(string, '.');
	
	Int8 digit = 0;

	digit = (Int32)(value * 10) % 10;
	Assert(IsIntBetween(digit, 0, 9));
	AddChar(string, '0' + digit);
	value = value * 10.0f;

	digit = (Int32)(value * 10) % 10;
	Assert(IsIntBetween(digit, 0, 9));
	AddChar(string, '0' + digit);
	value = value * 10.0f;
}

static void func AddText(String* string, Int8* text)
{
	for(Int32 i = 0; text[i]; i++)
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

static String func operator+(String string, Int8 value)
{
	String result = string;
	AddChar(&result, value);
	return result;
}

static String func operator+(String string, Int32 value)
{
	String result = string;
	AddInt(&result, value);
	return result;
}

static String func operator+(String string, Real32 value)
{
	String result = string;
	AddFloat(&result, value);
	return result;
}

static String func operator+(String string, Int8* text)
{
	String result = string;
	AddText(&result, text);
	return result;
}

static Int32 func GetNumberOfLines(String string)
{
	Int32 lineN = 0;

	for(Int32 i = 0; i < string.usedSize; i++)
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