#pragma once
#include "FileReader.h"

#define RSC_TXD_GTAIV 8
#define RSC_TXD_MP3  11

struct RSC_TXD
{
	int _vmt;
	RSC_POINTER dwOffsetMapOffset;
	RSC_POINTER dwHashTableOffset;
	char _f0[8];
	unsigned short dwTextureCount;
	unsigned short dwTextureCount2;
	RSC_POINTER texturesPtrs;
	unsigned short dwTextureCount3;
	unsigned short dwTextureCount4;

	void SwapData()
	{
		SwapEndian(dwTextureCount);
		SwapEndian(dwTextureCount2);
		SwapEndian(dwTextureCount3);
		SwapEndian(dwTextureCount4);
		SwapEndian(texturesPtrs);
	}
};
struct XTDHeader{
	int _vmt;
	RSC_POINTER dwOffsetMapOffset;
	RSC_POINTER dwHashTableOffset;
	DWORD _fC;
	DWORD _f10;
	DWORD dwTextureCount;
	DWORD dwTextureCount2;
	RSC_POINTER dwTextureListOffset;
	DWORD dwTextureCount3;
	DWORD dwTextureCount4;
};
class RSC_TEXTURE
{
	static unsigned int vtable;
public:
	virtual RSC_POINTER GetName() = 0;
	virtual unsigned short GetWidth() = 0;
	virtual unsigned short GetHeight() = 0;
	virtual unsigned int GetFormat() = 0;
	virtual unsigned short GetStride() = 0;
	virtual unsigned char GetLevels() = 0;
	virtual RSC_POINTER GetPixelData() = 0;


	void SaveVtable()
	{
		vtable = *(unsigned int *)this;
	}

	void ResetVtable()
	{
		*(unsigned int *)this = vtable;
	}
};

unsigned int RSC_TEXTURE::vtable;

// GTA IV texture
class RSC_TEXTURE_8 : public RSC_TEXTURE
{
	char _1[16];
	RSC_POINTER pName;
	char _2[4];
	unsigned short width;
	unsigned short height;
	unsigned int format;
	unsigned short stride;
	unsigned char type;
	unsigned char levels;
	char _3[32];
	RSC_POINTER pPixelData;
public:
	virtual RSC_POINTER GetName() { return pName; }
	virtual unsigned short GetWidth() { return width; }
	virtual unsigned short GetHeight() { return height; }
	virtual unsigned int GetFormat() { return format; }
	virtual unsigned short GetStride() { return stride; }
	virtual unsigned char GetLevels() { return levels; }
	virtual RSC_POINTER GetPixelData() { return pPixelData; }
};

// Max Payne 3 texture
class RSC_TEXTURE_11 : public RSC_TEXTURE
{
	char _1[20];
	RSC_POINTER pName;
	int _2;
	unsigned short width;
	unsigned short height;
	short _3;
	unsigned short stride;
	unsigned int format;
	unsigned char flags;
	unsigned char levels;
	char _4[2];
	char _5[32];
	RSC_POINTER pPixelData;
public:
	virtual RSC_POINTER GetName() { return pName; }
	virtual unsigned short GetWidth() { return width; }
	virtual unsigned short GetHeight() { return height; }
	virtual unsigned int GetFormat() { return format; }
	virtual unsigned short GetStride() { return stride; }
	virtual unsigned char GetLevels() { return levels; }
	virtual RSC_POINTER GetPixelData() { return pPixelData; }
};

static_assert(sizeof(RSC_TEXTURE_11) == 0x54, "ERR");

// GTA V Texture
class RSC_TEXTURE_V : public RSC_TEXTURE
{
public:
	unsigned int _f4;
	unsigned int _f8;
	unsigned int _fC;
	unsigned int _f10;
	unsigned int _f14;
	unsigned int _f18;
	unsigned int _f1C;
	RSC_POINTER pName;
	unsigned int _f24;
	unsigned int _f28;
	unsigned short _f2C;
	unsigned short _f2E;
	RSC_POINTER _f30;
	RSC_POINTER pD3DTexture;
	unsigned short width;
	unsigned short height;
	unsigned int levels;

	virtual RSC_POINTER GetName() { return pName; }
	virtual unsigned short GetWidth() { return width; }
	virtual unsigned short GetHeight() { return height; }
	virtual unsigned int GetFormat() { return D3DFMT_DXT1; }
	virtual unsigned short GetStride() { return width; }
	virtual unsigned char GetLevels() { return 0; }
	virtual RSC_POINTER GetPixelData() { return pD3DTexture; }

	void SwapData()
	{
		SwapEndian(pName);
		SwapEndian(width);
		SwapEndian(height);
		SwapEndian(levels);
		SwapEndian(pD3DTexture);
		SwapEndian(_f2C);
		SwapEndian(_f2E);
	}
};

static_assert(sizeof(RSC_TEXTURE_V) == 0x40, "ERR");

union RSC_UNION_4
{
	unsigned int uint;
	RSC_POINTER ptr;
};

enum eD3DXenonTextureType
{
	D3D_XENON_L8   = 0x02,
	D3D_XENON_G8R8 = 0x4A,
	D3D_XENON_DXT1 = 0x52,
	D3D_XENON_DXT3 = 0x52,
	D3D_XENON_DXT5 = 0x54,
	D3D_XENON_ATI2 = 0x71,
	D3D_XENON_ATI1 = 0x7B,
	D3D_XENON_CTX1 = 0x7C,
	D3D_XENON_ARGB = 0x86
};

struct RSC_D3DTEXTURE
{
	union{
		struct
		{

			unsigned int cFlags : 28;
			unsigned int cType : 4;
			unsigned int nRefCount;
			unsigned int Fence;
			unsigned int ReadFence;
			unsigned int Identifier;
			unsigned int BaseFlush;
			unsigned int MipFlush;
			unsigned int bTiled : 1;
			unsigned int Pitch : 9;
			unsigned int unk0 : 1;
			unsigned int unk1 : 2;
			unsigned int ClampZ : 3;
			unsigned int ClampY : 3;
			unsigned int ClampX : 3;
			unsigned int SignW : 2;
			unsigned int SignZ : 2;
			unsigned int SignY : 2;
			unsigned int SignX : 2;
			unsigned int Type : 2;
			unsigned int BaseAddress : 20;
			unsigned int ClampPolicy : 1;    // GPUCLAMPPOLICY
			unsigned int Stacked : 1;    // BOOL
			unsigned int RequestSize : 2;    // GPUREQUESTSIZE
			unsigned int Endian : 2;    // GPUENDIAN
			unsigned int DataFormat : 6;    // GPUTEXTUREFORMAT
			unsigned int Size;
			unsigned int BorderSize : 1;    // DWORD
			unsigned int unk2 : 3;
			unsigned int AnisoFilter : 3;    // GPUANISOFILTER
			unsigned int MipFilter : 2;    // GPUMIPFILTER
			unsigned int MinFilter : 2;    // GPUMINMAGFILTER
			unsigned int MagFilter : 2;    // GPUMINMAGFILTER
			int     ExpAdjust : 6;    // int
			unsigned int SwizzleW : 3;    // GPUSWIZZLE
			unsigned int SwizzleZ : 3;    // GPUSWIZZLE
			unsigned int SwizzleY : 3;    // GPUSWIZZLE
			unsigned int SwizzleX : 3;    // GPUSWIZZLE
			unsigned int NumFormat : 1;    // GPUNUMFORMAT        
			// DWORD 11:
			int     GradExpAdjustV : 5;    // int
			int     GradExpAdjustH : 5;    // int
			int     LODBias : 10;   // int
			unsigned int MinAnisoWalk : 1;    // BOOL
			unsigned int MagAnisoWalk : 1;    // BOOL
			unsigned int MaxMipLevel : 4;    // DWORD
			unsigned int MinMipLevel : 4;    // DWORD
			unsigned int VolMinFilter : 1;    // GPUMINMAGFILTER
			unsigned int VolMagFilter : 1;    // GPUMINMAGFILTER        
			// DWORD 12:
			unsigned int MipAddress : 20;
			unsigned int PackedMips : 1;    // BOOL
			unsigned int   Dimension : 2;
			int     AnisoBias : 4;    // int
			unsigned int   TriClamp : 2;    // GPUTRICLAMP
			unsigned int   ForceBCWToMax : 1;    // BOOL
			unsigned int   BorderColor : 2;
		};
		struct
		{
			RSC_UNION_4 dw[13];
		};
	};
};
/*
struct   D3DBaseTexture {
    // DWORD 0:
    DWORD   Common_Flags:28;
    DWORD   Common_Type: 4;
    // DWORD 1:
    DWORD   ReferenceCount;
    // DWORD 2:
    DWORD   Fence;
    // DWORD 3:
    DWORD   ReadFence;
    // DWORD 4:
    DWORD   Identifier;
    // DWORD 5:
    DWORD   BaseFlush;
    // DWORD 6:
    DWORD   MipFlush; 
    // DWORD 7:
    DWORD   Tiled                    : 1;    // BOOL
    DWORD   Pitch                    : 9;    // DWORD
    DWORD                            : 1;
    DWORD                            : 2;
    DWORD   ClampZ                   : 3;    // GPUCLAMP
    DWORD   ClampY                   : 3;    // GPUCLAMP
    DWORD   ClampX                   : 3;    // GPUCLAMP
    DWORD   SignW                    : 2;    // GPUSIGN
    DWORD   SignZ                    : 2;    // GPUSIGN
    DWORD   SignY                    : 2;    // GPUSIGN
    DWORD   SignX                    : 2;    // GPUSIGN
    DWORD   Type                     : 2;    // GPUCONSTANTTYPE
    // DWORD 8:
    DWORD   BaseAddress              : 20;   // DWORD        
    DWORD   ClampPolicy              : 1;    // GPUCLAMPPOLICY
    DWORD   Stacked                  : 1;    // BOOL
    DWORD   RequestSize              : 2;    // GPUREQUESTSIZE
    DWORD   Endian                   : 2;    // GPUENDIAN
    DWORD   DataFormat               : 6;    // GPUTEXTUREFORMAT
    // DWORD 9:
    union {
        GPUTEXTURESIZE_1D   OneD;
        GPUTEXTURESIZE_2D   TwoD;
        GPUTEXTURESIZE_3D   ThreeD;
        GPUTEXTURESIZE_STACK   Stack;
    } Size;
    // DWORD 10:
    DWORD   BorderSize               : 1;    // DWORD
    DWORD                            : 3;
    DWORD   AnisoFilter              : 3;    // GPUANISOFILTER
    DWORD   MipFilter                : 2;    // GPUMIPFILTER
    DWORD   MinFilter                : 2;    // GPUMINMAGFILTER
    DWORD   MagFilter                : 2;    // GPUMINMAGFILTER
    int     ExpAdjust                : 6;    // int
    DWORD   SwizzleW                 : 3;    // GPUSWIZZLE
    DWORD   SwizzleZ                 : 3;    // GPUSWIZZLE
    DWORD   SwizzleY                 : 3;    // GPUSWIZZLE
    DWORD   SwizzleX                 : 3;    // GPUSWIZZLE
    DWORD   NumFormat                : 1;    // GPUNUMFORMAT        
    // DWORD 11:
    int     GradExpAdjustV           : 5;    // int
    int     GradExpAdjustH           : 5;    // int
    int     LODBias                   : 10;   // int
    DWORD   MinAnisoWalk             : 1;    // BOOL
    DWORD   MagAnisoWalk             : 1;    // BOOL
    DWORD   MaxMipLevel               : 4;    // DWORD
    DWORD   MinMipLevel               : 4;    // DWORD
    DWORD   VolMinFilter             : 1;    // GPUMINMAGFILTER
    DWORD   VolMagFilter             : 1;    // GPUMINMAGFILTER        
    // DWORD 12:
    DWORD   MipAddress               : 20 <read=dataaddress_read>;   // DWORD
    DWORD   PackedMips               : 1;    // BOOL
    DWORD   Dimension                 : 2 <read=gpudimension_read>;    // GPUDIMENSION
    int     AnisoBias                 : 4;    // int
    DWORD   TriClamp                 : 2;    // GPUTRICLAMP
    DWORD   ForceBCWToMax             : 1;    // BOOL
    DWORD   BorderColor               : 2;    // GPUBORDERCOLOR
};*/