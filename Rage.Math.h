#pragma once

#include <float.h>
#include <math.h>

struct Vector2
{
	float x, y;

	Vector2(float a, float b)
	{
		x = a; y = b;
	}

	Vector2()
	{
		x = y = 0.0f;
	}

	bool operator==(const Vector2& v) const
    {
        return ( ( fabs ( x - v.x ) < FLT_EPSILON ) &&
                 ( fabs ( y - v.y ) < FLT_EPSILON ) );
	}
};

struct Vector3
{
	float x, y, z;

	Vector3()
	{
		x = y = z = 0.0f;
	}

	Vector3(float a, float b, float c)
	{
		x = a; y = b; z = c;
	}

	float DistanceTo(Vector3 &vec)
	{
		return sqrtf(powf(vec.x - x, 2.0f) + powf(vec.y - y, 2.0f) + powf(vec.z - z, 2.0f));
	}

	bool operator==(const Vector3& v) const
    {
        return ( ( fabs ( x - v.x ) < FLT_EPSILON ) &&
                 ( fabs ( y - v.y ) < FLT_EPSILON ) &&
                 ( fabs ( z - v.z ) < FLT_EPSILON ) );
	}
};

struct Vector4
{
	 float x, y, z, w;

	 Vector4(float a, float b, float c, float d)
	{
		x = a; y = b; z = c; w = d;
	}

	Vector4()
	{
		x = y = z = w = 0.0f;
	}
};

struct Triangle
{
	unsigned short a, b, d, c;
};

struct BoundSphere
{
	Vector3 center;
	float radius;

	BoundSphere()
	{
		center.x = center.y = center.z = 0.0f;
		radius = 1.0f;
	}

	BoundSphere(float x, float y, float z, float fRadius)
	{
		center.x = x;
		center.y = y;
		center.z = z;
		radius = fRadius;
	}

	BoundSphere(Vector3 &vCenter, float fRadius)
	{
		center = vCenter;
		radius = fRadius;
	}

	BoundSphere(Vector3 *vCenter, float fRadius)
	{
		center = *vCenter;
		radius = fRadius;
	}
};

struct BoundBox
{
	Vector3 aabbMin;
	Vector3 aabbMax;

	BoundBox()
	{
		aabbMin.x = aabbMin.y = aabbMin.z = -1.0f;
		aabbMax.x = aabbMax.y = aabbMax.z = 1.0f;
	}

	void Set(Vector3 &min, Vector3 &max)
	{
		aabbMin = min;
		aabbMax = max;
	}

	BoundBox(Vector3 &min, Vector3 &max)
	{
		Set(min, max);
	}

	BoundBox(Vector3 *min, Vector3 *max)
	{
		Set(*min, *max);
	}

	void ToSphere(BoundSphere *sphere) // BoundBox::ToSphere
	{
		sphere->radius = aabbMin.DistanceTo(aabbMax) / 2.0f;
		sphere->center.x = (aabbMin.x + aabbMax.x) / 2.0f;
		sphere->center.y = (aabbMin.y + aabbMax.y) / 2.0f;
		sphere->center.z = (aabbMin.z + aabbMax.z) / 2.0f;
	}
};