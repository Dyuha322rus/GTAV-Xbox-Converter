#pragma once

#include <vector>
#include <d3d9.h>
#include "CRenderWare.h"
#include "LogFile.h"
#include "Hash.h"
#pragma pack(push, 1)
struct NativeTexturePC
{
	struct TextureFormat{
		unsigned int platformId;
		unsigned int filterMode : 8;
		unsigned int uAddressing : 4;
		unsigned int vAddressing : 4;
		char name[32];
		char maskName[32];
	} _TextureFormat;	
	struct RasterFormat{
		unsigned int rasterFormat;
		union
		{
			unsigned int d3dFormat;
			unsigned int hasAlpha;
		};
		unsigned short width;
		unsigned short height;
		unsigned char stride;
		unsigned char numLevels;
		unsigned char rasterType;
		union {
			unsigned char compression; // GTA 3 & VC
			struct { // SA
				unsigned char alpha : 1;
				unsigned char cubeTexture : 1;
				unsigned char autoMipMaps : 1;
				unsigned char compressed : 1;
			};
		};
	} _RasterFormat;	
	//unsigned int imageSize;	
	//unsigned char *rasterPalette;
	//unsigned char *pixelData;
};

#define GAME_3 0
#define GAME_VC 1
#define GAME_SA 2

extern unsigned int uAddressing = rwTEXTUREADDRESSWRAP, vAddressing = rwTEXTUREADDRESSWRAP, 
	filterMode = rwFILTERLINEAR, filterMipMode = rwFILTERLINEARMIPLINEAR, anysotropy = 0, skipNormals = 0, skipBump = 0, skipSpec = 0, gameVersion = 
	GAME_SA, chunkVersion = 0x36003;

struct TxdPC
{
	unsigned short numTextures;
	unsigned char deviceId;
	unsigned char _zero;
};

class CImage
{
public:
	char name[200];
	unsigned int nameLength;
	unsigned int width;
	unsigned int height;
	unsigned int levels;
	unsigned int stride;
	D3DFORMAT format;
	unsigned char *pixels;
	unsigned int pixelsSize;
	unsigned char depth;
	unsigned int SizeInTxd()
	{
		unsigned int size = 100 + pixelsSize + levels * 4;
		if(anysotropy && gameVersion != GAME_3)
			size += 16;
		return size;
	}

	unsigned int StructSizeInTxd()
	{
		return 100 + pixelsSize + levels * 4;
	}

	void Write(RwStream *stream, char *fileName)
	{
		bool hasAlpha = false;
		if(format == D3DFMT_X8R8G8B8)
		{
		//	pixelsSize = (stride - stride / 4) * height;
		//	for(int j = 2; j <= levels; j++)
		//		pixelsSize += ((stride - stride / 4) * height) >> ((j - 1) * 2);
		}
		NativeTexturePC texData; unsigned int size, offset;// int counter;
		memset(&texData, 0, sizeof(texData));
		printf("\n%i\n", pixelsSize);
		if(gtaRwStreamWriteVersionedChunkHeader(stream, rwID_TEXTURENATIVE, SizeInTxd() + 12, chunkVersion, 0xFFFF)
			&& gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, StructSizeInTxd() - 12, chunkVersion, 0xFFFF))
		{
			if(gameVersion == GAME_SA)
				texData._TextureFormat.platformId = rwID_PCD3D9;
			else
				texData._TextureFormat.platformId = rwID_PCD3D8;
			if(levels > 1)
				texData._TextureFormat.filterMode = filterMipMode;
			else
				texData._TextureFormat.filterMode = filterMode;
			texData._TextureFormat.uAddressing = uAddressing;
			texData._TextureFormat.vAddressing = vAddressing;
			if(format == D3DFMT_DXT3 || format == D3DFMT_DXT5 || format == D3DFMT_A8B8G8R8)
			{
				hasAlpha = true;
				if(gameVersion == GAME_SA)
					texData._RasterFormat.alpha = true;
				else
					texData._RasterFormat.hasAlpha = true;
			}

			if(nameLength > 31)
			{
				char nameHash[30];
				sprintf(nameHash, "%X", GetHashKey(name, 0));
				strcpy(name, nameHash);
				
				if(hasAlpha)
				{
					strncpy(texData._TextureFormat.name, nameHash, 30);
					strncpy(texData._TextureFormat.maskName, nameHash, 30);
					texData._TextureFormat.maskName[30] = 'a';
					texData._TextureFormat.maskName[31] = '\0';
					LOG("%s: texture \"%s\" has too long name. Renamed to %s\n", fileName, name, 
						texData._TextureFormat.name);
				}
				else
				{
					strncpy(texData._TextureFormat.name, name, 30);
					texData._TextureFormat.maskName[30] = '\0';
					LOG("%s: texture \"%s\" has too long name. Renamed to %s\n", fileName, name, 
						texData._TextureFormat.name);
				}
			}
			else
			{
				strcpy(texData._TextureFormat.name, name);
				if(hasAlpha)
				{
					strcpy(texData._TextureFormat.maskName, name);
					texData._TextureFormat.maskName[nameLength - 1] = 'a';
					texData._TextureFormat.maskName[nameLength] = '\0';
				}
			}
			if(gtaRwStreamWrite(stream, &texData._TextureFormat, sizeof(NativeTexturePC::TextureFormat)))
			{
				if(format == D3DFMT_DXT1 || format == D3DFMT_DXT2 || format == D3DFMT_DXT3 || format == D3DFMT_DXT4 || format == D3DFMT_DXT5)
				{
					if(gameVersion == GAME_SA)
						texData._RasterFormat.compressed = true;
					else
					{
						switch(format)
						{
						case D3DFMT_DXT1:
							texData._RasterFormat.compression = 1;
							break;
						case D3DFMT_DXT2:
							texData._RasterFormat.compression = 2;
							break;
						case D3DFMT_DXT3:
							texData._RasterFormat.compression = 3;
							break;
						case D3DFMT_DXT4:
							texData._RasterFormat.compression = 4;
							break;
						case D3DFMT_DXT5:
							texData._RasterFormat.compression = 5;
						}
					}
				}
				if(gameVersion == GAME_SA)
					texData._RasterFormat.d3dFormat = format;
				texData._RasterFormat.height = height;
				texData._RasterFormat.numLevels = levels;
				texData._RasterFormat.width = width;
				if(format == D3DFMT_X8R8G8B8)
				{
					//size = stride * height;
					//for(int j = 2; j <= levels; j++)
					//	size += (stride * height) >> ((j - 1) * 2);
					//counter = 0;
					//for(int i = 0; i < size; i++)
					//{
					//	if(i % 4)
					//	{
					//		pixels[counter] = pixels[i];
					//		counter++;
					//	}
					//}
					//stride -= (stride / 4);
				}
				texData._RasterFormat.stride = 0;
				texData._RasterFormat.rasterType = rwRASTERTYPETEXTURE;
				switch(format)
				{
				case D3DFMT_L8:
					texData._RasterFormat.rasterFormat = rwRASTERFORMATLUM8;
					break;
				case D3DFMT_X8R8G8B8:
					texData._RasterFormat.rasterFormat = rwRASTERFORMAT888;
					break;
				case D3DFMT_A8R8G8B8:
				case D3DFMT_DXT1:
				case D3DFMT_DXT3:
				case D3DFMT_DXT5:
					texData._RasterFormat.rasterFormat = rwRASTERFORMAT8888;
					break;
				default:
					texData._RasterFormat.rasterFormat = rwRASTERFORMATDEFAULT;
				}
				if(levels > 2)
					texData._RasterFormat.rasterFormat |= rwRASTERFORMATMIPMAP;
				if(gtaRwStreamWrite(stream, &texData._RasterFormat, sizeof(NativeTexturePC::RasterFormat)))
				{
					size = stride * height;
					if(gtaRwStreamWrite(stream, &size, 4) && gtaRwStreamWrite(stream, pixels, size))
					{
						offset = size;
						
						for (UINT j = 2; j <= levels; j++)
						{
							size = (stride * height) >> ((j - 1) * 2);
							if(!gtaRwStreamWrite(stream, &size, 4) || !gtaRwStreamWrite(stream, &pixels[offset], size))
								break;
							offset += size;
						}
						if(anysotropy && gameVersion != GAME_3)
						{
							gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 16, chunkVersion, 0xFFFF);
							gtaRwStreamWriteVersionedChunkHeader(stream, 0x127, 4, chunkVersion, 0xFFFF);
							gtaRwStreamWriteInt32(stream, &anysotropy, 4);
						}
						else
							gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, chunkVersion, 0xFFFF);
					}
				}
			}
		}
	}
};

class CTexDictionary
{
public:
	std::vector<CImage> images;

	CImage *FindTexture(char *name)
	{
		for (UINT i = 0; i < images.size(); i++)
		{
			if(!strcmp(images[i].name, name))
				return &images[i];
		}
		return NULL;
	}

	void Build(char *fileName)
	{
		TxdPC txdData;
		unsigned int size = 28;
		RwStream *stream = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, fileName);
		for(auto i = images.begin(); i != images.end(); ++i)
			size += (*i).SizeInTxd();
		if(gtaRwStreamWriteVersionedChunkHeader(stream, rwID_TEXDICTIONARY, size, chunkVersion, 0xFFFF)
			&& gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 4, chunkVersion, 0xFFFF))
		{
			txdData.deviceId = 0;
			txdData._zero = 0;
			txdData.numTextures = images.size();
			if(gtaRwStreamWrite(stream, &txdData, 4))
			{
				for(auto i = images.begin(); i != images.end(); ++i)
				{
					//printf("%s\n", (*i).name);
					(*i).Write(stream, fileName);
					delete[] (*i).pixels;
				}
			}
			gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, chunkVersion, 0xFFFF);
		}
		gtaRwStreamClose(stream, NULL);
	}
};