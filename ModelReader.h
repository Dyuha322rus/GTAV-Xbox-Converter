#pragma once
#include "FileReader.h"

#include "SwapEndian.h"

#include "Rage.Math.h"

#include "RenderWare.h"
class CColor : public RwRGBA
{
public:
	CColor()
	{
		red = green = blue = alpha = 255;
	}

	CColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		red = r;
		green = g;
		blue = b;
		alpha = a;
	}
};

struct rageD3D9VertexFlags
{
	unsigned int m_bPosition : 1;
	unsigned int m_bBlendWeight : 1;
	unsigned int m_bBlendIndices : 1;
	unsigned int m_bNormal : 1;
	unsigned int m_bColor : 1;
	unsigned int m_bSpecularColor : 1;
	unsigned int m_bTexCoord1 : 1;
	unsigned int m_bTexCoord2 : 1;
	unsigned int m_bTexCoord3 : 1;
	unsigned int m_bTexCoord4 : 1;
	unsigned int m_bTexCoord5 : 1;
	unsigned int m_bTexCoord6 : 1;
	unsigned int m_bTexCoord7 : 1;
	unsigned int m_bTexCoord8 : 1;
	unsigned int m_bTangent : 1;
	unsigned int m_bBinormal : 1;

	void SwapData()
	{
		char tmp[4];
		tmp[3] =  ((char *)this)[0];
		tmp[2] =  ((char *)this)[1];
		tmp[1] =  ((char *)this)[2];
		tmp[0] =  ((char *)this)[3];
		memmove(this, tmp, 4);
	}
};

struct rageD3D9VertexElementTypes
{
	unsigned int m_nPositionType : 4;
	unsigned int m_nBlendWeightType : 4;
	unsigned int m_nBlendIndicesType : 4;
	unsigned int m_nNormalType : 4;
	unsigned int m_nColorType : 4;
	unsigned int m_nSpecularColorType : 4;
	unsigned int m_nTexCoord1Type : 4;
	unsigned int m_nTexCoord2Type : 4;
	unsigned int m_nTexCoord3Type : 4;
	unsigned int m_nTexCoord4Type : 4;
	unsigned int m_nTexCoord5Type : 4;
	unsigned int m_nTexCoord6Type : 4;
	unsigned int m_nTexCoord7Type : 4;
	unsigned int m_nTexCoord8Type : 4;
	unsigned int m_nTangentType : 4;
	unsigned int m_nBinormalType : 4;

	void SwapData()
	{
		char tmp[8];
		tmp[7] =  ((char *)this)[0];
		tmp[6] =  ((char *)this)[1];
		tmp[5] =  ((char *)this)[2];
		tmp[4] =  ((char *)this)[3];
		tmp[3] =  ((char *)this)[4];
		tmp[2] =  ((char *)this)[5];
		tmp[1] =  ((char *)this)[6];
		tmp[0] =  ((char *)this)[7];
		memmove(this, tmp, 8);
	}
};

struct grcVertexDeclaration // Xenon
{
	rageD3D9VertexFlags usedElements;
	unsigned char totalSize;
	unsigned char _f6;
	unsigned char storeNormalsDataFirst;
	unsigned char numElements;
	rageD3D9VertexElementTypes elementTypes;

	void SwapData()
	{
		usedElements.SwapData();
		elementTypes.SwapData();
	}
};

enum eElementType
{
	ELEMENTTYPE_FLOAT16,
	ELEMENTTYPE_FLOAT16_2,
	ELEMENTTYPE_FLOAT16_3,
	ELEMENTTYPE_FLOAT16_4,
	ELEMENTTYPE_FLOAT1,
	ELEMENTTYPE_FLOAT2,
	ELEMENTTYPE_FLOAT3,
	ELEMENTTYPE_FLOAT4,
	ELEMENTTYPE_UBYTE4,
	ELEMENTTYPE_D3DCOLOR,
	ELEMENTTYPE_DEC3N,
	ELEMENTTYPE_NOTAVAILABLE
};

unsigned int gElementSizes[16] = {2, 4, 6, 8, 4, 8, 12, 16, 4, 4, 4, 0, 0, 0, 0, 0};

struct RSC_COLLECTION
{
	RSC_POINTER listOffset;
	unsigned short count, size;

	void SwapData()
	{
		SwapEndian(listOffset);
		SwapEndian(count);
		SwapEndian(size);
	}
};

struct RSC_MODEL
{
	unsigned int vtablle;
	RSC_POINTER geometriesList;
	unsigned short geometriesCount;
	unsigned short geometriesSize;
	RSC_POINTER _fC;
	RSC_POINTER shaderMappings;
	unsigned char _f14;
	unsigned char isSkinned;
	unsigned char _f16;
	unsigned char _f17;
	unsigned char _f18;
	unsigned char _f19;
	unsigned short shaderMappingsCount;

	void SwapData()
	{
		SwapEndian(geometriesList);
		SwapEndian(geometriesCount);
		SwapEndian(geometriesSize);
		SwapEndian(shaderMappings);
		SwapEndian(shaderMappingsCount);
	}

};

struct RSC_GEOMETRY
{
	unsigned char _f0[12];
	RSC_POINTER vertexBuffers[4];
	RSC_POINTER indexBuffers[4];
	unsigned int indexCount;
	unsigned int faceCount;
	unsigned short vertexCount;
	unsigned short indicesPerFace;
	RSC_POINTER boneMapping;
	unsigned short vertexStride;
	unsigned short boneCount;

	void SwapData()
	{
		SwapEndian(vertexBuffers[0]);
		SwapEndian(indexBuffers[0]);
		SwapEndian(indexCount);
		SwapEndian(faceCount);
		SwapEndian(vertexCount);
		SwapEndian(indicesPerFace);
		SwapEndian(vertexStride);
		SwapEndian(boneMapping);
		SwapEndian(boneCount);
	}
};

struct RSC_GEOMETRY_POINTERS
{
	RSC_POINTER _f0[20];

	void SwapData()
	{
		for(int i = 0; i < 20; i++)
		{
			SwapEndian(_f0[i]);
		}
	}
};

struct RSC_DRAWABLE
{
	unsigned int vtable;
	RSC_POINTER blockMap;
	RSC_POINTER pShaders;
	RSC_POINTER pSkeleton;
	RSC_VECTOR4 center;
	RSC_VECTOR4 aabbMin;
	RSC_VECTOR4 aabbMax;
	RSC_POINTER pModelCollection[4];
	RSC_VECTOR4 point_max;
	unsigned short _unk1;
	unsigned short _unk2;
	unsigned int _unk3[4];
	unsigned int _unk4;
	RSC_POINTER _unk5;
	RSC_POINTER title;

	void SwapData()
	{
		SwapEndian(pShaders);
		SwapEndian(pSkeleton);
		SwapEndian(center);
		SwapEndian(aabbMin);
		SwapEndian(aabbMax);
		SwapEndian(pModelCollection[0]);
		SwapEndian(pModelCollection[1]);
		SwapEndian(pModelCollection[2]);
		SwapEndian(pModelCollection[3]);
	}
};

struct RSC_DRAWABLEDICTIONARY
{
	unsigned int vtable;
	RSC_POINTER blockMap;
	int nParentDict;
	int nUsageCount;
	RSC_COLLECTION hashPtrs;
	RSC_COLLECTION drawablePtrs;
	void SwapData()
	{
		hashPtrs.SwapData();
		drawablePtrs.SwapData();
	}
};

struct RSC_INDEXBUFFER
{
	unsigned int _f0;
	unsigned int indexCount;
	RSC_POINTER indexData;

	void SwapData()
	{
		SwapEndian(indexCount);
		SwapEndian(indexData);
	}
};

struct RSC_VERTEXBUFFER
{
	unsigned int _f0;
	unsigned short vertexSize;
	unsigned char locked;
	unsigned char _f7;
	RSC_POINTER lockedData;
	unsigned int vertexCount;
	RSC_POINTER vertexData;
	unsigned int _f14;
	RSC_POINTER declaration;
	
	void SwapData()
	{
		SwapEndian(vertexSize);
		SwapEndian(lockedData);
		SwapEndian(vertexCount);
		SwapEndian(vertexData);
		SwapEndian(declaration);
	}
};

struct RSC_SHADERGOUP
{
	unsigned int _f0;
	RSC_POINTER texDictionary;
	RSC_POINTER shaders;
	unsigned short numShaders;

	void SwapData()
	{
		SwapEndian(texDictionary);
		SwapEndian(shaders);
		SwapEndian(numShaders);
	}
};

struct RSC_SHADER
{
	RSC_POINTER shaderParams;
	unsigned int nameHash;
	unsigned char paramsCount;
	unsigned char _f6;
	unsigned short _f7;
	unsigned short paramsDataSize;
	unsigned short _fA;
	unsigned int spsNameHash;
	unsigned int _f10;
	unsigned int _f14;
	unsigned int _f18;
	unsigned int _f1C;

	void SwapData()
	{
		SwapEndian(shaderParams);
		SwapEndian(nameHash);
		SwapEndian(_f7);
		SwapEndian(paramsDataSize);
		SwapEndian(spsNameHash);
		SwapEndian(_f10);
		SwapEndian(_f14);
		SwapEndian(_f1C);
	}
};

struct RSC_SHADERPARAM
{
	unsigned int type;
	RSC_POINTER param;

	void SwapData()
	{
		SwapEndian(type);
		SwapEndian(param);
	}
};

struct RSC_FRAGCHILD
{
	unsigned int vtable;
	float pristineMass;
	float damagedMass;
	unsigned char groupId;
	unsigned char pad;
	unsigned short boneIndex;
	RSC_VECTOR4 matrix1[4];
	RSC_VECTOR4 matrix2[4];
	RSC_POINTER drawable;

	void SwapData()
	{
		SwapEndian(drawable);
		SwapEndian(boneIndex);
	};
};

#pragma pack(push, 1)
struct RSC_FRAGTYPE
{
	unsigned int _unk1;
	RSC_POINTER unkPtr;
	unsigned int _unk2[6];
	RSC_POINTER drawable;
	RSC_POINTER ptrs[5];
	RSC_POINTER name; // "pack:/<modelname>"
	unsigned int _fUnk[22];
	RSC_POINTER fragData; // RSC_FRAG_DATA

	void SwapData()
	{
		SwapEndian(drawable);
		SwapEndian(name);
		SwapEndian(fragData);
	}
};
#pragma pack(pop)

struct RSC_FRAG_DATA
{
    unsigned int vtable;
    int _f4;
    RSC_POINTER childsData;

	void SwapData()
	{
		SwapEndian(childsData);
	}
};

#pragma pack(push, 1)
struct RSC_CHILDS_DATA
{
    unsigned int vtable;
    unsigned int _unk1[2];
    float MaxInertiaValueNormalized;
    float MaxInertiaValue;
    unsigned int _unk2[2];
    RSC_POINTER _unk3;
    RSC_VECTOR4 vectors[9];
    RSC_POINTER _unk4;
    RSC_POINTER _unk5;
    RSC_POINTER childsPtrs;
    RSC_POINTER _unk6;
    unsigned int _unk7; // empty ptr?
    RSC_POINTER _unk8[4];
    unsigned int _unk9[3];
    unsigned char _unk10;
    unsigned char groupsCount;
    unsigned char childsCount;
    unsigned char _unk11;

	void SwapData()
	{
		SwapEndian(childsPtrs);
	}
};
#pragma pack(pop)

struct RSC_SKELETON_V_POINTERS
{
	RSC_POINTER _f0[50];

	void SwapData()
	{
		for(int i = 0; i < 50; i++)
		{
			SwapEndian(_f0[i]);
		}
	}
};

struct RSC_SKELETON_V_WORDS
{
	WORD _f0[100];

	void SwapData()
	{
		for(int i = 0; i < 100; i++)
		{
			SwapEndian(_f0[i]);
		}
	}
};

struct RSC_SKELETON_V_BYTES
{
	BYTE _f0[200];
};

struct RSC_BONE_V
{
	unsigned int _f0;
	unsigned int _f4;
	unsigned int _f8;
	unsigned int _fC;
	unsigned int _f10;
	unsigned int _f14;
	unsigned int _f18;
	unsigned int _f1C;
	unsigned int _f20;
	unsigned int _f24;
	unsigned int _f28;
	unsigned int _f2C;
	short boneId; // -1 if main bone
	short parentBoneId; // -1 if main bone
	RSC_POINTER name;
	unsigned int _f38;
	unsigned short boneIndex;
	unsigned short boneNumber;

	void SwapData()
	{
		SwapEndian(name);
		SwapEndian(boneId);
		SwapEndian(parentBoneId);
		SwapEndian(boneIndex);
		SwapEndian(boneNumber);
	}
};

struct RSC_BONE_INTS
{
	unsigned int _f0[16]; // 13 = name

	void SwapData()
	{
		for(int i = 0; i < 16; i++)
		{
			SwapEndian(_f0[i]);
		}
	}
};

struct RSC_BONE_WORDS
{
	unsigned short _f0[32];

	void SwapData()
	{
		for(int i = 0; i < 32; i++)
		{
			SwapEndian(_f0[i]);
		}
	}
};

struct RSC_BONE_BYTES
{
	unsigned char _f0[64]; // 13 = name
};

struct RSC_SKELETON_V
{
	unsigned int _f0; // CRC? static 2020968960
	unsigned int _f4; // static 0
	RSC_POINTER _f8;
	unsigned short _fC;
	unsigned short boneRelatedDataSize; // same as boneCount
	unsigned int _f10; // static 1
	RSC_POINTER bones; // crBone *
	RSC_POINTER transform; // D3DMATRIX *
	RSC_POINTER transformInverted; // D3DMATRIX *
	RSC_POINTER parentBoneIndices; // unsigned short *
	RSC_POINTER _f24;
	unsigned int _f28; // static 0
	unsigned int _f2C; // crc?
	unsigned int _f30; // crc?
	unsigned short _f34;
	unsigned char _f36;
	unsigned char _f37;
	unsigned short _f38; // static 1
	unsigned short boneCount;
	unsigned short boneRotCount;
	unsigned short _f3E;
	unsigned int _f40;

	void SwapData()
	{
		SwapEndian(_f0);
		SwapEndian(_f4);
		SwapEndian(_f8);
		SwapEndian(_fC);
		SwapEndian(boneRelatedDataSize);
		SwapEndian(_f10);
		SwapEndian(_f10);
		SwapEndian(bones);
		SwapEndian(transform);
		SwapEndian(transformInverted);
		SwapEndian(parentBoneIndices);
		SwapEndian(_f24);
		SwapEndian(_f28);
		SwapEndian(_f2C);
		SwapEndian(_f30);
		SwapEndian(_f34);
		SwapEndian(_f38);
		SwapEndian(boneCount);
		SwapEndian(boneRotCount);
		SwapEndian(_f3E);
		SwapEndian(_f40);
	}
	/*
	RSC_POINTER parentBoneIndices;
	RSC_POINTER boneWorldOrient;
	RSC_POINTER boneWorldOrientInverted;
	RSC_POINTER boneLocalTransforms;
	unsigned short boneCount;
	unsigned short transLockCount;
	unsigned short rotLockCount;
	unsigned short scaleLockCount;
	unsigned int flags;
	RSC_POINTER boneIdMappingsPtrs;
	unsigned short boneIdMappingsCount;
	unsigned short boneIdMappingsSize;*/
};

struct RSC_SKELETON
{
	RSC_POINTER bones;
	RSC_POINTER parentBoneIndices;
	RSC_POINTER boneWorldOrient;
	RSC_POINTER boneWorldOrientInverted;
	RSC_POINTER boneLocalTransforms;
	unsigned short boneCount;
	unsigned short transLockCount;
	unsigned short rotLockCount;
	unsigned short scaleLockCount;
	unsigned int flags;
	RSC_POINTER boneIdMappingsPtrs;
	unsigned short boneIdMappingsCount;
	unsigned short boneIdMappingsSize;

	void SwapData()
	{
		SwapEndian(bones);
		SwapEndian(parentBoneIndices);
		SwapEndian(boneWorldOrient);
		SwapEndian(boneWorldOrientInverted);
		SwapEndian(boneLocalTransforms);
		SwapEndian(boneCount);
		SwapEndian(transLockCount);
		SwapEndian(rotLockCount);
		SwapEndian(scaleLockCount);
		SwapEndian(boneIdMappingsPtrs);
		SwapEndian(boneIdMappingsCount);
		SwapEndian(boneIdMappingsSize);
	}
};

struct RSC_BONEMAPPING
{
	unsigned short boneId;
	unsigned short number;

	void SwapData()
	{
		SwapEndian(boneId);
		SwapEndian(number);
	}
};

struct RSC_BONE
{
	RSC_POINTER name;
	unsigned int flags;
	RSC_POINTER next;
	RSC_POINTER child;
	RSC_POINTER parent;
	char _f4[0xCC];

	void SwapData()
	{
		SwapEndian(name);
		SwapEndian(flags);
		SwapEndian(next);
		SwapEndian(child);
		SwapEndian(parent);
	}
};

struct RSC_MATRIX : public RwMatrix
{
	void SwapData()
	{
		SwapEndian(pos.x);
		SwapEndian(pos.y);
		SwapEndian(pos.z);
		SwapEndian(flags);
		SwapEndian(right.x);
		SwapEndian(right.y);
		SwapEndian(right.z);
		SwapEndian(pad1);
		SwapEndian(at.x);
		SwapEndian(at.y);
		SwapEndian(at.z);
		SwapEndian(pad2);
		SwapEndian(up.x);
		SwapEndian(up.y);
		SwapEndian(up.z);
		SwapEndian(pad3);
	}
};

class RSC_TEXTURE_REF
{
public:
	unsigned int _f0;
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
	unsigned int _f30;
	RSC_POINTER pD3DTexture;
	unsigned short width;
	unsigned short height;
	unsigned int levels;

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