#pragma once

#include "CRenderWare.h"
#include <Windows.h>
#include "Rage.Math.h"

extern "C" RpGeometry *_rtGeometryOptimize(RpGeometry *geometry, float distance);

extern "C" int _rwerror(unsigned int code)
{
	return 1;
}

extern "C" int RwErrorSet(unsigned int code)
{
	return 1;
}

extern "C" RpGeometry *RpGeometryLock(RpGeometry *geometry, int lockMode)
{
	return geometry;
}

extern "C" RpGeometry *RpGeometryUnlock(RpGeometry *geometry)
{
	return geometry;
}

extern "C" RpMaterial *_rpMaterialListGetMaterial(RpMaterialList *list, unsigned int id)
{
	return NULL;
}

extern "C" void RpMaterialDestroy(RpMaterial *material) { }

extern "C" void RpGeometryTriangleGetVertexIndices(RpGeometry *geometry, RpTriangle *triangle, unsigned short *a,  unsigned short *b,  unsigned short *c)
{
	if(a)
		*a = triangle->vertIndex[0];
	if(b)
		*b = triangle->vertIndex[1];
	if(c)
		*c = triangle->vertIndex[2];
}

extern "C" void RpGeometryTriangleSetVertexIndices(RpGeometry *geometry, RpTriangle *triangle, unsigned short a,  unsigned short b,  unsigned short c)
{
	triangle->vertIndex[0] = a;
	triangle->vertIndex[1] = b;
	triangle->vertIndex[2] = c;
}

extern "C" void RpClumpForAllAtomics(RpClump *clump, void *callback, void *data) { }

struct RwGlobalsDummy
{
	char dummy[0x108];
	void *rwMalloc;
	void *rwFree;
	void *rwRealloc;
};

void *rwmalloc(unsigned int size, unsigned int hint)
{
	return malloc(size);
}

void *rwrealloc(void *mem, unsigned int size, unsigned int hint)
{
	return realloc(mem, size);
}

void rwfree(void *mem, unsigned int hint)
{
	free(mem);
}

RwGlobalsDummy rwEngineDummy;

extern "C" RwGlobalsDummy *RwEngineInstance;

RwGlobalsDummy *RwEngineInstance;

void OptimiseInit()
{
	RwEngineInstance = &rwEngineDummy;
	RwEngineInstance->rwMalloc = rwmalloc;
	RwEngineInstance->rwRealloc = rwrealloc;
	RwEngineInstance->rwFree = rwfree;
}

struct RpGeometryOptimised
{
	RwObject            object;
	unsigned int        flags;
	unsigned short      lockedSinceLastInst;
	short               refCount;
	int                 numTriangles;
	int                 numVertices;
	int                 numMorphTargets;
	int                 numTexCoordSets;
	RpMaterialList      matList;
	RpTriangle         *triangles;
	RwRGBA             *preLitLum;
	RwTexCoords        *texCoords[8];
	RpMeshHeader       *mesh;
	RwResEntry         *repEntry;
	RpMorphTarget      *morphTarget;

	RpGeometryOptimised(unsigned int trisCount, unsigned int vertsCount, Triangle *tris, Vector3 *positions)
	{
		memset(&this->object, 0, sizeof(RpGeometryOptimised));
		morphTarget = new RpMorphTarget;
		numMorphTargets = 1;
		memset(morphTarget, 0, sizeof(RpMorphTarget));
		numTriangles = trisCount;
		numVertices = vertsCount;
		triangles = new RpTriangle[trisCount];
		for (UINT i = 0; i < trisCount; i++)
		{
			triangles[i].matIndex = tris[i].d;
			triangles[i].vertIndex[0] = tris[i].a;
			triangles[i].vertIndex[1] = tris[i].b;
			triangles[i].vertIndex[2] = tris[i].c;
		}
		flags |= rpGEOMETRYPOSITIONS;
		morphTarget->verts = new RwV3d[vertsCount];
		memcpy(morphTarget->verts, positions, vertsCount * sizeof(RwV3d));
	}

	RpGeometryOptimised(unsigned int trisCount, unsigned int vertsCount, unsigned int *tris, Vector3 *positions, unsigned short mat, unsigned int indexBase)
	{
		memset(&this->object, 0, sizeof(RpGeometryOptimised));
		morphTarget = new RpMorphTarget;
		numMorphTargets = 1;
		memset(morphTarget, 0, sizeof(RpMorphTarget));
		numTriangles = trisCount;
		numVertices = vertsCount;
		triangles = new RpTriangle[trisCount];
		for (UINT i = 0; i < trisCount; i++)
		{
			triangles[i].matIndex = mat;
			triangles[i].vertIndex[0] = tris[0] - indexBase;
			triangles[i].vertIndex[1] = tris[1] - indexBase;
			triangles[i].vertIndex[2] = tris[2] - indexBase;
			tris += 3;
		}
		flags |= rpGEOMETRYPOSITIONS;
		morphTarget->verts = new RwV3d[vertsCount];
		memcpy(morphTarget->verts, positions, vertsCount * sizeof(RwV3d));
	}

	~RpGeometryOptimised()
	{
		delete[] triangles;
		delete[] morphTarget->verts;
		delete morphTarget;
	}
};