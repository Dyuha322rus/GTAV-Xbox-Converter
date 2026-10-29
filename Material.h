#pragma once
#include "RenderWare.h"
#include "CRenderWare.h"
#include <Windows.h>
#include <vector>
#include <d3d9.h>
#include "Rage.Math.h"
#include "Hash.h"
unsigned int GetAlignedSize(char *str) // get string length aligned to 4
{
	return (strlen(str) + 4) & ~3;
}

class CTexture
{
public:
	struct{
		unsigned char filtering;
		unsigned char uAddressing : 4;
		unsigned char vAddressing : 4;
		unsigned char generateMipMaps : 1;
		unsigned char padding;
		char nameAligned[32];
	} _StreamData;
	unsigned int nameLength;

	void Set(char *name)
	{
		if (strlen(name) > 31){
			char nameHash[30];
			sprintf(nameHash, "%X", GetHashKey(name, 0));
			strcpy(name, nameHash);
		}
		strncpy(_StreamData.nameAligned, name, 30);
		_StreamData.nameAligned[30] = '\0';
		nameLength = GetAlignedSize(_StreamData.nameAligned);
		_StreamData.filtering = rwFILTERLINEARMIPLINEAR;
		_StreamData.uAddressing = rwTEXTUREADDRESSCLAMP;
		_StreamData.vAddressing = rwTEXTUREADDRESSCLAMP;
		_StreamData.generateMipMaps = 0;
	}

	bool Write(RwStream *stream)
	{
		unsigned int null = 0;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_TEXTURE, nameLength + 4 + 28 + 12 + 12, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &_StreamData, 4))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRING, nameLength, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, _StreamData.nameAligned, nameLength))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRING, 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &null, 4))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
			return false;
		return true;
	}

	unsigned int GetStreamSize()
	{
		return nameLength + 28 + 4 + 12 + 12;
	}
};

class CMaterial
{
public:
	struct{
		unsigned int flags;
		RwRGBA color;
		unsigned int unused;
		unsigned int  textured;
		RwSurfaceProperties surfaceProps;
	} _StreamData;

	char shaderName[100];

	unsigned int shaderHash;

	unsigned int matID;
	unsigned int numTris;

	CTexture texture;

	void Set(char *name, RwRGBA& color, unsigned int hash, unsigned int id = 0)
	{
		shaderHash = hash;
		if(name[0] != '\0')
			_StreamData.textured = 1;
		else
			_StreamData.textured = 0;
		_StreamData.color = color;
		if (hash == GetHashKey("trees_lod", 0) || hash == GetHashKey("trees_lod_tnt", 0) || hash == GetHashKey("trees_lod2d", 0) || hash == GetHashKey("trees_lod2", 0))
			_StreamData.color.alpha = 0;
		_StreamData.flags = 0;
		_StreamData.unused = 0;
		_StreamData.surfaceProps.specular = 0.0f;
		_StreamData.surfaceProps.ambient = 1.0f;
		_StreamData.surfaceProps.diffuse = 1.0f;
		texture.Set(name);
		matID = id;
		numTris = 0;
	}

	bool Write(RwStream *stream)
	{
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_MATERIAL, GetStreamSize(), 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 28, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &_StreamData, 28))
			return false;
		if(_StreamData.textured)
		{
			if(!texture.Write(stream))
				return false;
		}
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
			return false;
		return true;
	}

	unsigned int GetStreamSize()
	{
		unsigned int size = 40 + 12;
		if(_StreamData.textured)
			size += 12 + texture.GetStreamSize();
		return size;
	}
};