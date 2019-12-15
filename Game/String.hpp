#pragma once

#include "Debug.hpp"
#include "Type.hpp"

#define AddLine(string, value) {string = string + value + "\n";}
#define OneLineString(memory, max_size, line) {StartString(memory, max_size) + line;}

struct String
{
	Int8 *buffer;
	Int32 buffer_size;
	Int32 used_size;
};

static void CloseString(String *string);

static Bool32
func StringIsTerminated(Int8 *string, Int32 length)
{
	Bool32 is_terminated = false;
	for(Int32 i = 0; i < length; i++)
	{
		if(string[i] == 0)
		{
			is_terminated = true;
			break;
		}
	}
	return is_terminated;
}

static void
func StringCopy(Int8 *from, Int8 *to, Int32 max_size)
{
	Bool32 terminated = false;
	for(Int32 i = 0; i < max_size; i++)
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

static String
func StartString(Int8 *buffer, Int32 buffer_size)
{
	Assert(buffer_size > 0);
	String string = {};
	string.buffer = buffer;
	string.buffer_size = buffer_size;
	string.used_size = 0;
	return string;
}

static void
func AddChar(String *string, Int8 value)
{
	Assert(string->used_size + 1 < string->buffer_size);
	string->buffer[string->used_size] = value;
	string->used_size++;

	CloseString(string);
}

static Int32
func GetNumberOfDigits(Int32 value)
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

static Int32
func GetLastDigit(Int32 value)
{
	Assert(value >= 0);
	Int32 result = value % 10;
	return result;
}

static Int32 
func CutLastDigit(Int32 value)
{
	Assert(value > 0);
	Int32 result = value / 10;
	return result;
}

static void
func AddInt(String *string, Int32 value)
{
	if(value == 0)
	{
		AddChar(string, '0');
	}
	else
	{
		Int32 digit_n = GetNumberOfDigits(value);
		Int32 position = string->used_size + digit_n - 1;
		Assert(position < string->buffer_size);
		while(value > 0)
		{
			Assert(position >= string->used_size);
			string->buffer[position] = Int8('0' + GetLastDigit(value));
			position--;
			value = CutLastDigit(value);
		}
		string->used_size += digit_n;
	}

	CloseString(string);
}

static void
func AddFloat(String *string, Real32 value)
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

static void
func AddText(String *string, Int8* text)
{
	for(Int32 i = 0; text[i]; i++)
	{
		AddChar(string, text[i]);
	}

	CloseString(string);
}

static void
func CloseString(String *string)
{
	Assert(string->used_size + 1 < string->buffer_size);
	string->buffer[string->used_size] = 0;
}

static String
func operator+(String string, Int8 value)
{
	String result = string;
	AddChar(&result, value);
	return result;
}

static String
func operator+(String string, Int32 value)
{
	String result = string;
	AddInt(&result, value);
	return result;
}

static String
func operator+(String string, Real32 value)
{
	String result = string;
	AddFloat(&result, value);
	return result;
}

static String
func operator+(String string, Int8* text)
{
	String result = string;
	AddText(&result, text);
	return result;
}

static Int32
func GetNumberOfLines(String string)
{
	Int32 line_n = 0;

	for(Int32 i = 0; i < string.used_size; i++)
	{
		if(string.buffer[i] == '\n')
		{
			line_n++;
		}
	}

	if(string.used_size > 0 && string.buffer[string.used_size - 1] != '\n')
	{
		line_n++;
	}

	Assert(line_n >= 0);
	return line_n;
}

// TODO: Handle AddInt when the value is negative!
// TODO: Handle AddInt when the value is 0!