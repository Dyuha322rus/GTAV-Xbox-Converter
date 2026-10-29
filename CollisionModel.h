#pragma once

#include "Rage.Math.h"

#pragma pack(push, 1)
struct CollisionModel2
{
	unsigned int fourcc;
	unsigned int size;
	char name[20];
	short unused;
	short modelIndex;
	BoundBox bbox;
	BoundSphere bsphere;
	unsigned short numSpheres;
	unsigned short numBoxes;
	unsigned int numFaces;
	unsigned int flags;
	unsigned int offsetSpheres;
	unsigned int offsetBoxes;
	unsigned int offsetLines;
	unsigned int offsetVertices;
	unsigned int offsetFaces;
	unsigned int offsetTrianglePlanes;
};

struct ColVertex
{
	short x, y, z;
};

struct ColTriangle
{
	unsigned short a, b, c;
	unsigned char material, lighting;
};
#pragma pack(pop)

static_assert(sizeof(CollisionModel2) == 108, "ERR");