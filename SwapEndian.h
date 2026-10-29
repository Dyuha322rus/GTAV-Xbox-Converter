#pragma once

#include <Windows.h>
#include "Rage.Math.h"

void SwapEndian(unsigned int &value)
{
	char tmp[4];
	tmp[3] =  ((char *)&value)[0];
	tmp[2] =  ((char *)&value)[1];
	tmp[1] =  ((char *)&value)[2];
	tmp[0] =  ((char *)&value)[3];
	memmove(&value, tmp, 4);
}

void SwapEndian(DWORD &value)
{
	char tmp[4];
	tmp[3] =  ((char *)&value)[0];
	tmp[2] =  ((char *)&value)[1];
	tmp[1] =  ((char *)&value)[2];
	tmp[0] =  ((char *)&value)[3];
	memmove(&value, tmp, 4);
}

void SwapEndian(int &value)
{
	char tmp[4];
	tmp[3] =  ((char *)&value)[0];
	tmp[2] =  ((char *)&value)[1];
	tmp[1] =  ((char *)&value)[2];
	tmp[0] =  ((char *)&value)[3];
	memmove(&value, tmp, 4);
}

void SwapEndian(float &value)
{
	char tmp[4];
	tmp[3] =  ((char *)&value)[0];
	tmp[2] =  ((char *)&value)[1];
	tmp[1] =  ((char *)&value)[2];
	tmp[0] =  ((char *)&value)[3];
	memmove(&value, tmp, 4);
}

void SwapEndian(Vector4 &value)
{
	SwapEndian(value.x);
	SwapEndian(value.y);
	SwapEndian(value.z);
	SwapEndian(value.w);
}

void SwapEndian(Vector3 &value)
{
	SwapEndian(value.x);
	SwapEndian(value.y);
	SwapEndian(value.z);
}

void SwapEndian(Vector2 &value)
{
	SwapEndian(value.x);
	SwapEndian(value.y);
}

void SwapEndian(RSC_VECTOR4 &value)
{
	SwapEndian(value.x);
	SwapEndian(value.y);
	SwapEndian(value.z);
	SwapEndian(value.w);
}

void SwapEndian(RSC_POINTER &value)
{
	char tmp[4];
	tmp[3] =  ((char *)&value)[0];
	tmp[2] =  ((char *)&value)[1];
	tmp[1] =  ((char *)&value)[2];
	tmp[0] =  ((char *)&value)[3];
	memmove(&value, tmp, 4);
}

void SwapEndian(unsigned short &value)
{
	char tmp[2];
	tmp[1] =  ((char *)&value)[0];
	tmp[0] =  ((char *)&value)[1];
	memmove(&value, tmp, 2);
}

void SwapEndian(short &value)
{
	char tmp[2];
	tmp[1] =  ((char *)&value)[0];
	tmp[0] =  ((char *)&value)[1];
	memmove(&value, tmp, 2);
}