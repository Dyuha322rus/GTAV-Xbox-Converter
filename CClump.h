#pragma once

#include "RenderWare.h"
#include "CRenderWare.h"
#include <Windows.h>
#include <vector>
#include <d3d9.h>
#include "Rage.Math.h"

class CVertexStream
{
public:
	unsigned int vertexBuffer;
	unsigned int offset;
	unsigned int stride;
	unsigned short geometryFlags;
	unsigned char managed;
	unsigned char dynamicLock;
};

class CMesh_NativeData
{
public:
	unsigned int numIndex; // Number of indices. Needed for reinstancing, not for rendering
	unsigned int minVert; // The vertex index where the meshes vertices begin. Needed for reinstancing, not for rendering
	unsigned int material;
	unsigned int vertexAlpha; // The prelight data contains alpha
	unsigned int vertexShader;
	unsigned int baseIndex; // Needed if we share the vertex buffer
	unsigned int numVertices; // Number of vertices in the vertex buffer
	unsigned int startIndex; // Location in the index array to start reading vertices
	unsigned int numPrimitives; // Number of primitives to render
};

class CMeshHeader
{
public:
	unsigned int flags;
	unsigned int numMeshes;
	unsigned int totalIndicesInMesh;
};

class CMesh
{
public:
	unsigned int numIndices;
	unsigned int material;
	unsigned int *indices;
	unsigned int indexCounter;
};

class CDeclarationElement
{
public:
	unsigned short stream;
	unsigned short offset;
	unsigned char type;
	unsigned char method;
	unsigned char usage;
	unsigned char usageIndex;
};

#define RES_ENTRY_HEADER_SIZE 0x40
#define MESH_SIZE 0x24
#define DECLARATION_SIZE 8
#define STREAM_SIZE 16
#include "ModelReader.h"

class CRageGeometry
{
public:
	unsigned int shaderId;
	grcVertexDeclaration rageDeclaration;
	unsigned int vertexCount;
	unsigned int indexCount;
	unsigned int facesCount;
	unsigned int indicesPerFace;
	unsigned short *indexBuffer;
	union
	{
		void *vertexBuffer;
		unsigned int *vertexInt;
	};
};

#include "Material.h"

unsigned __int32 halfToFloatI(unsigned __int16 y)
{
     register int s = (y >> 15) & 0x00000001;
     register int e = (y >> 10) & 0x0000001f;
     register int m =  y        & 0x000003ff;
     
     if (e == 0)
     {
         if (m == 0) // Plus or minus zero
             return s << 31;

         else // Denormalized number -- renormalize it
         {
             while (!(m & 0x00000400))
             {
                 m <<= 1;
                 e -=  1;
             }
     
             e += 1;
             m &= ~0x00000400;
         }
     }
     else if (e == 31)
     {
         if (m == 0) // Inf
             return (s << 31) | 0x7f800000;
         else // NaN
             return (s << 31) | 0x7f800000 | (m << 13);
     }
     
     e = e + (127 - 15);
     m = m << 13;
     
     return (s << 31) | (e << 23) | m;
}

float halfToFloat(unsigned __int16 y)
{
             union { float f; unsigned __int32 i; } v;
             v.i = halfToFloatI(y);
             return v.f;
}

struct DEC3N
{
	int x : 10;
	int y : 10;
	int z : 10;
	int w : 2;

	Vector3 Unpack()
	{
		return Vector3((float)(x) / 511.0f, (float)(y) / 511.0f, (float)(z) / 511.0f);
	}

	void SwapData()
	{
		SwapEndian(*(unsigned int *)this);
	}
};

struct FLOAT16_2
{
	unsigned short x;
	unsigned short y;

	Vector2 Unpack()
	{
		return Vector2(halfToFloat(x), halfToFloat(y));
	}

	void SwapData()
	{
		SwapEndian(x);
		SwapEndian(y);
	}
};

struct COLOR
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	DWORD Unpack()
	{
		return *(DWORD *)this;
	}

	void SwapData()
	{
		unsigned char temp = r;
		r = a;
		a = temp;
		temp = g;
		g = b;
		b = temp;
	}
};

class CModelInstance
{
public:
	struct{
		unsigned short flags;
		unsigned char numTexCoords;
		unsigned char nativeFlags;
		unsigned int numFaces;
		unsigned int numVertices;
		unsigned int numMorphTargets;
		struct{
			float x;
			float y;
			float z;
			float r;
			unsigned int hasPositions;
			unsigned int hasNormals;
		} MorphTarget;
		struct{
			unsigned int frameId;
			unsigned int geometryId;
			unsigned int unk5;
			unsigned int unk0;
		} Atomic;
	} _StreamData;

	bool hasPositions;
	bool hasNormals;
	bool hasColors;
	unsigned char numTexCoords;
	unsigned int totalNumIndex;
	unsigned int totalNumVertex;
	unsigned int totalNumFaces;

	Triangle *triangles;
	Vector3 *positions;
	Vector3 *normals;
	Vector2 *texCoords[8];
	DWORD *colors;

	CMeshHeader meshHeader;
	std::vector<CMesh> meshes;

	// WDR DATA
	BoundBox bbox;
	BoundSphere bsphere;
	std::vector<CMaterial> materials;
	std::vector<CRageGeometry> geometries;

	void BuildMeshes(FILE *f)
	{
		//char temp[200];
		//unsigned int a, c;
		// get the general geometry info
		hasPositions = false;
		hasNormals = false;
		hasColors = false;
		numTexCoords = 0;
		totalNumIndex = 0;
		totalNumVertex = 0;
		unsigned int indexCount = 0;
		unsigned int triCount = 0;
		Vector4 dummyVec(0.0f, 0.0f, 0.0f, 0.0f);
		unsigned char maxNumTexCoords = 0;
		CMesh mesh;
		for(size_t i = 0; i < geometries.size(); i++)
		{
			if(geometries[i].rageDeclaration.usedElements.m_bPosition)
				hasPositions = true;
			if(geometries[i].rageDeclaration.usedElements.m_bNormal)
				hasNormals = true;
			if(geometries[i].rageDeclaration.usedElements.m_bColor)
				hasColors = true;
			if(geometries[i].rageDeclaration.usedElements.m_bTexCoord8)
				maxNumTexCoords = 8;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord7)
				maxNumTexCoords = 7;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord6)
				maxNumTexCoords = 6;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord5)
				maxNumTexCoords = 5;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord4)
				maxNumTexCoords = 4;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord3)
				maxNumTexCoords = 3;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord2)
				maxNumTexCoords = 2;
			else if(geometries[i].rageDeclaration.usedElements.m_bTexCoord1)
				maxNumTexCoords = 1;
			if(maxNumTexCoords > numTexCoords)
				numTexCoords = maxNumTexCoords;
			meshes.push_back(mesh);
			meshes[i].indices = new unsigned int[geometries[i].indexCount];
			for(UINT j = 0; j < geometries[i].indexCount; j++)
				meshes[i].indices[j] = geometries[i].indexBuffer[j] + indexCount;
			indexCount += geometries[i].vertexCount;
			meshes[i].material = geometries[i].shaderId;
			meshes[i].numIndices = geometries[i].indexCount;
			totalNumIndex += geometries[i].indexCount;
			totalNumVertex += geometries[i].vertexCount;
		}
		//sprintf(temp, "Positions: %d Normals: %d Colors: %d NumTexCoors: %d TotalIndices: %d TotalVertices: %d\n",
		//	hasPositions, hasNormals, hasColors, numTexCoords, totalNumIndex, totalNumVertex);
		//fputs(temp, f);
		// allocate the data
		if(hasPositions)
			positions = new Vector3[totalNumVertex];
		if(hasNormals)
			normals = new Vector3[totalNumVertex];
		if(hasColors)
		{
			colors = new DWORD[totalNumVertex];
			memset(colors, 0xFF, totalNumVertex * 4);
		}
		for(int i = 0; i < numTexCoords; i++)
			texCoords[i] = new Vector2[totalNumVertex];

		void *vertex;
		unsigned int offset;
		unsigned int vertCounter = 0;
		// split the vertex data

		// print elements size
		//sprintf(temp, "Sizes: Position %d Normal %d Color %d TexCoord %d\n", 
		//	hasPositions? gElementSizes[geometries[0].rageDeclaration.elementTypes.m_nPositionType] : 0,
		//	hasNormals? gElementSizes[geometries[0].rageDeclaration.elementTypes.m_nNormalType] : 0,
		//	hasColors? gElementSizes[geometries[0].rageDeclaration.elementTypes.m_nColorType] : 0,
		//	numTexCoords > 0? gElementSizes[geometries[0].rageDeclaration.elementTypes.m_nTexCoord1Type] : 0);
		//fputs(temp, f);
		DEC3N dec3n;
		FLOAT16_2 float16_2;
		COLOR color;

		for (UINT i = 0; i < geometries.size(); i++)
		{
			if(!geometries[i].rageDeclaration.storeNormalsDataFirst)
			{
				// go through all geometry vertices
				for (UINT j = 0; j < geometries[i].vertexCount; j++)
				{
					vertex = (char *)geometries[i].vertexBuffer + j * geometries[i].rageDeclaration.totalSize;
					offset = 0;
					if(geometries[i].rageDeclaration.usedElements.m_bPosition)
					{
						positions[vertCounter + j] = *(Vector3 *)((char *)vertex + offset);
						SwapEndian(positions[vertCounter + j]);
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nPositionType];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bBlendWeight)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nBlendWeightType];
					if(geometries[i].rageDeclaration.usedElements.m_bBlendIndices)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nBlendIndicesType];
					if(geometries[i].rageDeclaration.usedElements.m_bNormal)
					{
						dec3n = *(DEC3N *)((char *)vertex + offset);
						dec3n.SwapData();
						normals[vertCounter + j] = dec3n.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nNormalType];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bColor)
					{
						color = *(COLOR *)((char *)vertex + offset);
						color.SwapData();
						colors[vertCounter + j] = color.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nColorType];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bSpecularColor)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nSpecularColorType];
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord1)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[0][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord1Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord2)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[1][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord2Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord3)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[2][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord3Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord4)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[3][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord4Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord5)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[4][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord5Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord6)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[5][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord6Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord7)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[6][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord7Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord8)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[7][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord8Type];
					}
					// not needed
					/*if(geometries[i].rageDeclaration.usedElements.m_bTangent)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTangentType];
					if(geometries[i].rageDeclaration.usedElements.m_bTangent)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTangentType]; */
				}
			}
			else
			{
				// go through all geometry vertices
				for (UINT j = 0; j < geometries[i].vertexCount; j++)
				{
					vertex = (char *)geometries[i].vertexBuffer + j * geometries[i].rageDeclaration.totalSize;
					offset = 0;
					if(geometries[i].rageDeclaration.usedElements.m_bPosition)
					{
						positions[vertCounter + j] = *(Vector3 *)((char *)vertex + offset);
						SwapEndian(positions[vertCounter + j]);
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nPositionType];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bNormal)
					{
						dec3n = *(DEC3N *)((char *)vertex + offset);
						dec3n.SwapData();
						normals[vertCounter + j] = dec3n.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nNormalType];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTangent)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTangentType];
					if (geometries[i].rageDeclaration.usedElements.m_bBinormal)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nBinormalType];
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord1)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[0][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord1Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord2)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[1][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord2Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord3)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[2][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord3Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord4)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[3][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord4Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord5)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[4][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord5Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord6)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[5][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord6Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord7)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[6][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord7Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bTexCoord8)
					{
						float16_2 = *(FLOAT16_2 *)((char *)vertex + offset);
						float16_2.SwapData();
						texCoords[7][vertCounter + j] = float16_2.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nTexCoord8Type];
					}
					if(geometries[i].rageDeclaration.usedElements.m_bBlendWeight)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nBlendWeightType];
					if(geometries[i].rageDeclaration.usedElements.m_bBlendIndices)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nBlendIndicesType];
					if(geometries[i].rageDeclaration.usedElements.m_bColor)
					{
						color = *(COLOR *)((char *)vertex + offset);
						color.SwapData();
						colors[vertCounter + j] = color.Unpack();
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nColorType];
					}
					// not needed
					/*if(geometries[i].rageDeclaration.usedElements.m_bSpecularColor)
						offset += gElementSizes[geometries[i].rageDeclaration.elementTypes.m_nSpecularColorType];*/
				}
			}
			vertCounter += geometries[i].vertexCount;
		}
		// build triangles
		if(totalNumIndex % 3)
			printf("\nIndices error\n");
		totalNumFaces = totalNumIndex / 3;
		triangles = new Triangle[totalNumFaces];
		indexCount = 0;
		triCount = 0;
		for (UINT i = 0; i < geometries.size(); i++)
		{
			for (UINT j = 0; j < (geometries[i].indexCount / 3); j++)
			{
				triangles[triCount].c = geometries[i].indexBuffer[j * 3] + indexCount;
				triangles[triCount].b = geometries[i].indexBuffer[j * 3 + 1] + indexCount;
				triangles[triCount].a = geometries[i].indexBuffer[j * 3 + 2] + indexCount;
				triangles[triCount].d = geometries[i].shaderId;
				triCount++;
			}
			indexCount += geometries[i].vertexCount;
		}
		// flip faces in meshes
		//for(int i = 0; i < meshes.size(); i++)
		//{
		//	for(int j = 0; j < meshes[i].numIndices / 3; j++)
		//	{
		//		a = meshes[i].indices[j * 3];
		//		c = meshes[i].indices[j * 3 + 2];
		//		meshes[i].indices[j * 3] = c;
		//		meshes[i].indices[j * 3 + 2] = a;
		//	}
		//}

		meshHeader.totalIndicesInMesh = totalNumIndex;
		meshHeader.flags = 0;
		meshHeader.numMeshes = meshes.size();
		
		//FILE *ff = fopen("test.txt", "at");
		//fputs("\n\nNEW MODEL\n", ff);
		//fputs("\nTriangles:\n", ff);
		//for(int i = 0; i < totalNumFaces; i++)
		//{
		//	sprintf(temp, "%d %d %d %d\n", triangles[i].a, triangles[i].b, triangles[i].c , triangles[i].d);
		//	fputs(temp, ff);
		//}
		//fclose(ff);

		//fputs("\nMeshes:\n", f);
		//for(int i = 0; i < meshes.size(); i++)
		//{
		//	fputs("\nMesh:\n", f);
		//	for(int j = 0; j < meshes[i].numIndices / 3; j++)
		//	{
		//		sprintf(temp, "%d %d %d\n", meshes[i].indices[j * 3], meshes[i].indices[j * 3 + 1], meshes[i].indices[j * 3 + 2]);
		//		fputs(temp, f);
		//	}
		//}

		//if(hasPositions)
		//{
		//	fputs("\nPositions:\n", f);
		//	for(int i = 0; i < totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f %f\n", positions[i].x, positions[i].y, positions[i].z);
		//		fputs(temp, f);
		//	}
		//}
		//if(hasNormals)
		//{
		//	fputs("\nNormals:\n", f);
		//	for(int i = 0; i < totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f %f\n", normals[i].x, normals[i].y, normals[i].z);
		//		fputs(temp, f);
		//	}
		//}
		//if(hasColors)
		//{
		//	fputs("\nColors:\n", f);
		//	for(int i = 0; i < totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%X\n", colors[i]);
		//		fputs(temp, f);
		//	}
		//}
		//if(numTexCoords > 0)
		//{
		//	fputs("\nTex Coors:\n", f);
		//	for(int i = 0; i < totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f\n", texCoords[0][i].x, texCoords[0][i].y);
		//		fputs(temp, f);
		//	}
		//}
	}

	// extensions

	// std::list<C2dfxEntry> effects;

	bool Write(RwStream *stream)
	{
		signed int *matData;
		unsigned int numMaterials;
		unsigned int materialsSize = 0;
		unsigned int meshesSize = 0;
		numMaterials = materials.size();
		for (UINT i = 0; i < numMaterials; i++)
			materialsSize += materials[i].GetStreamSize() + 12;
		// total size
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_GEOMETRY, GetStreamSize(), 0x36003, 0xFFFF))
			return false;
		// geometry base info size
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 16 + hasColors * (4 * totalNumVertex) + 
			hasNormals * (12 * totalNumVertex) + hasPositions * (12 * totalNumVertex) + 24 + 8 * (totalNumIndex / 3) + 
			8 * numTexCoords * totalNumVertex, 0x36003, 0xFFFF))
		{
			return false;
		}
		//printf("Geometry struct size %d\n", 16 + hasColors * (4 * totalNumVertex + 0) + 
		//	hasNormals * (12 * totalNumVertex) + hasPositions * (12 * totalNumVertex) + 24 + 8 * (totalNumIndex / 3) + 
		//	8 * numTexCoords * totalNumVertex);
		_StreamData.flags = 0;
		if(hasPositions)
			_StreamData.flags |= rpGEOMETRYPOSITIONS;
		if(hasNormals)
			_StreamData.flags |= rpGEOMETRYNORMALS;
		if(hasColors)
			_StreamData.flags |= rpGEOMETRYPRELIT;
		_StreamData.numTexCoords = numTexCoords;
		if(numTexCoords == 1)
			_StreamData.flags |= rpGEOMETRYTEXTURED;
		else if(numTexCoords == 2)
		{
			_StreamData.flags |= rpGEOMETRYTEXTURED;
			_StreamData.flags |= rpGEOMETRYTEXTURED2;
		}
		_StreamData.nativeFlags = 0;
		_StreamData.numMorphTargets = 1;
		_StreamData.numVertices = totalNumVertex;
		_StreamData.numFaces = totalNumIndex / 3;
		if(!gtaRwStreamWrite(stream, &_StreamData, 16))
			return false;
		if(hasColors && !gtaRwStreamWrite(stream, colors, 4 * totalNumVertex))
			return false;
		for(int i = 0; i < numTexCoords; i++)
		{
			if(!gtaRwStreamWrite(stream, texCoords[i], 8 * totalNumVertex))
				return false;
		}
		if(!gtaRwStreamWrite(stream, triangles, 8 * (totalNumIndex / 3)))
			return false;
		_StreamData.MorphTarget.x = bsphere.center.x;
		_StreamData.MorphTarget.y = bsphere.center.y;
		_StreamData.MorphTarget.z = bsphere.center.z;
		_StreamData.MorphTarget.r = bsphere.radius;
		_StreamData.MorphTarget.hasPositions = hasPositions;
		_StreamData.MorphTarget.hasNormals = hasNormals;
		if(!gtaRwStreamWrite(stream, &_StreamData.MorphTarget, 24))
			return false;
		if(hasPositions && !gtaRwStreamWrite(stream, positions, 12 * totalNumVertex))
			return false;
		if(hasNormals && !gtaRwStreamWrite(stream, normals, 12 * totalNumVertex))
			return false;

		// write materials
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_MATLIST, 4 * numMaterials + 16 + materialsSize, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 4 * numMaterials + 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &numMaterials, 4))
			return false;
		if(numMaterials > 0)
		{
			matData = new signed int[numMaterials];
			memset(matData, 0xFF, 4 * numMaterials);
			if(!gtaRwStreamWrite(stream, matData, 4 * numMaterials))
			{
				delete[] matData;
				return false;
			}
			delete[] matData;
			for (UINT i = 0; i < numMaterials; i++)
			{
				if(!materials[i].Write(stream))
					return false;
			}
		}
		for (UINT i = 0; i < meshes.size(); i++)
			meshesSize += 8 + 4 * meshes[i].numIndices;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 24 + meshesSize, 0x36003, 0xFFFF))
			return false;
		// Write Bin Mesh data
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, 0x50E, 12 + meshesSize, 0x36003, 0xFFFF))
			return false;
		meshHeader.flags = 0;
		meshHeader.numMeshes = meshes.size();
		meshHeader.totalIndicesInMesh = totalNumIndex;
		if(!gtaRwStreamWrite(stream, &meshHeader, 12))
			return false;
		for (UINT i = 0; i < meshes.size(); i++)
		{
			if(!gtaRwStreamWrite(stream, &meshes[i], 8))
				return false;
			if(!gtaRwStreamWrite(stream, meshes[i].indices, 4 * meshes[i].numIndices))
				return false;
		}
		return true;
	}

	unsigned int GetStreamSize()
	{
		unsigned int materialsSize = 0;
		unsigned int meshesSize = 0;
		for (UINT i = 0; i < meshes.size(); i++)
			meshesSize += 8 + 4 * meshes[i].numIndices;
		for (UINT i = 0; i < materials.size(); i++)
			materialsSize += materials[i].GetStreamSize() + 12;
		return 36 + 16 + hasColors * (4 * totalNumVertex) + 
			hasNormals * (12 * totalNumVertex) + hasPositions * (12 * totalNumVertex) + 24 + 8 * (totalNumIndex / 3) + 
			8 * numTexCoords * totalNumVertex + 4 * materials.size() + 16 + materialsSize + 12 + 12 + meshesSize;
	}
};

#include "Skeleton.h"

class CClump;

class CFrame
{
public:
	Vector3 rotation[3];
	Vector3 position;
	signed int parentFrame;
	unsigned int matrixFlags;

	unsigned short boneIndex;

	std::vector<CMaterial> *clumpMaterials;

	unsigned int GetMaterialIndexFromId(unsigned int id)
	{
		for (UINT mat = 0; mat < atomics[0].materials.size(); mat++)
		{
			if(atomics[0].materials[mat].matID == id)
				return mat;
		}
		return 0;
	}

	RwMatrix GetMatrix()
	{
		RwMatrix m;
		m.flags = m.pad1 = m.pad2 = m.pad3 = 0;
		m.right.x = rotation[0].x;
		m.right.y = rotation[0].y;
		m.right.z = rotation[0].z;
		m.up.x = rotation[1].x;
		m.up.y = rotation[1].y;
		m.up.z = rotation[1].z;
		m.at.x = rotation[2].x;
		m.at.y = rotation[2].y;
		m.at.z = rotation[2].z;
		m.pos.x = position.x;
		m.pos.y = position.y;
		m.pos.z = position.z;
		return m;
	}

	char name[24];

	std::vector<CModelInstance> atomics;

	CSkeletonBone bone;

	void BuildSkeleton(FILE *file)
	{
		char temp[200];
		unsigned int a, c;
		// get the general geometry info
		atomics[0].hasPositions = bone.format.position;
		atomics[0].hasNormals = bone.format.normal;
		atomics[0].hasColors = bone.format.color; // проверить что в других Build() не убрана запись цвета
		atomics[0].numTexCoords = bone.format.numTexCoords;
		atomics[0].totalNumIndex = bone.triangles.size() * 3;
		atomics[0].totalNumVertex = bone.triangles.size() * 3;
		unsigned int triCount = 0;
		Vector4 dummyVec(0.0f, 0.0f, 0.0f, 0.0f);

		CVertex *newVerts = new CVertex[bone.triangles.size() * 3];
		// build vertex data
		unsigned int indCount = 0;
		for (UINT j = 0; j < bone.triangles.size(); j++)
		{
			newVerts[indCount] = bone.triangles[j].vertices[0];
			newVerts[indCount].vertId = indCount;
			newVerts[indCount].isAssociated = false;
			newVerts[indCount + 1] = bone.triangles[j].vertices[1];
			newVerts[indCount + 1].vertId = indCount + 1;
			newVerts[indCount + 1].isAssociated = false;
			newVerts[indCount + 2] = bone.triangles[j].vertices[2];
			newVerts[indCount + 2].vertId = indCount + 2;
			newVerts[indCount + 2].isAssociated = false;
			indCount += 3;
		}
		// now go through vertices and associate them
		unsigned int numUniqueVerts = 0;
		for (UINT j = 0; j < bone.triangles.size() * 3; j++)
		{
			for(int i = j - 1; i >= 0; i--)
			{
				if(newVerts[j].IsEqual(newVerts[i], *clumpMaterials))
				{
					newVerts[j].vertId = newVerts[i].vertId;
					newVerts[j].isAssociated = true;
					break;
				}
			}
			if(newVerts[j].isAssociated)
				continue;
			newVerts[j].vertId = numUniqueVerts;
			numUniqueVerts++;
		}
		atomics[0].totalNumVertex = numUniqueVerts;
		
		//sprintf(temp, "Positions: %d Normals: %d Colors: %d NumTexCoors: %d TotalIndices: %d TotalVertices: %d\n",
		//	hasPositions, hasNormals, hasColors, numTexCoords, totalNumIndex, totalNumVertex);
		//fputs(temp, f);
		// allocate the data
		if(atomics[0].hasPositions)
			atomics[0].positions = new Vector3[atomics[0].totalNumVertex];
		if(atomics[0].hasNormals)
			atomics[0].normals = new Vector3[atomics[0].totalNumVertex];
		atomics[0].hasColors = true;
		if(atomics[0].hasColors)
		{
			atomics[0].colors = new DWORD[atomics[0].totalNumVertex];
			memset(atomics[0].colors, 0xFF, atomics[0].totalNumVertex * 4);
		}
		for(int i = 0; i < atomics[0].numTexCoords; i++)
			atomics[0].texCoords[i] = new Vector2[atomics[0].totalNumVertex];

		RwRGBA clr;

		if(bone.format.position)
		{
			indCount = 0;
			for(int j = 0; j < bone.triangles.size() * 3; j++)
			{
				if(!newVerts[j].isAssociated)
				{
					atomics[0].positions[indCount] = newVerts[j].position;
					indCount++;
				}
			}
		}

		if(bone.format.normal)
		{
			indCount = 0;
			for(int j = 0; j < bone.triangles.size() * 3; j++)
			{
				if(!newVerts[j].isAssociated)
				{
					atomics[0].normals[indCount] = newVerts[j].normal;
					indCount++;
				}
			}
		}

		if(bone.format.color)
		{
			indCount = 0;
			for(int j = 0; j < bone.triangles.size() * 3; j++)
			{
				if(!newVerts[j].isAssociated)
				{
					atomics[0].colors[indCount] = newVerts[j].intColor;
					indCount++;
				}
			}
		}

		for(int tx = 0; tx < bone.format.numTexCoords; tx++)
		{
			indCount = 0;
			for(int j = 0; j < bone.triangles.size() * 3; j++)
			{
				if(!newVerts[j].isAssociated)
				{
					atomics[0].texCoords[tx][indCount] = newVerts[j].texcoords[tx];
					indCount++;
				}
			}
		}
		
		// build triangles
		if(atomics[0].totalNumIndex % 3)
			printf("\nIndices error\n");
		atomics[0].totalNumFaces = atomics[0].totalNumIndex / 3;
		atomics[0].triangles = new Triangle[atomics[0].totalNumFaces];

		// flip faces in meshes
		//for(int i = 0; i < meshes.size(); i++)
		//{
		//	for(int j = 0; j < meshes[i].numIndices / 3; j++)
		//	{
		//		a = meshes[i].indices[j * 3];
		//		c = meshes[i].indices[j * 3 + 2];
		//		meshes[i].indices[j * 3] = c;
		//		meshes[i].indices[j * 3 + 2] = a;
		//	}
		//}

		indCount = 0;
		for(int j = 0; j < bone.triangles.size(); j++)
		{
			atomics[0].triangles[j].a = newVerts[indCount].vertId;
			atomics[0].triangles[j].b = newVerts[indCount + 1].vertId;
			atomics[0].triangles[j].c = newVerts[indCount + 2].vertId;
			atomics[0].triangles[j].d = GetMaterialIndexFromId(bone.triangles[j].meshId); // TMP!!!
			indCount += 3;
		}

		// now build meshes

		atomics[0].meshHeader.totalIndicesInMesh = atomics[0].totalNumIndex;
		atomics[0].meshHeader.flags = 0;
		atomics[0].meshHeader.numMeshes = atomics[0].materials.size();

		CMesh mesh;
		// create meshes
		for(int mtl = 0; mtl < atomics[0].materials.size(); mtl++)
		{
			atomics[0].meshes.push_back(mesh);
			atomics[0].meshes[mtl].material = mtl;
			atomics[0].meshes[mtl].numIndices = 0;
			atomics[0].meshes[mtl].indexCounter = 0;
		}
		// count triangles in mesh
		for(int j = 0; j < bone.triangles.size(); j++)
			atomics[0].meshes[atomics[0].triangles[j].d].numIndices += 3;
		// allocate indices data
		for(int mtl = 0; mtl < atomics[0].meshes.size(); mtl++)
			atomics[0].meshes[mtl].indices = new unsigned int[atomics[0].meshes[mtl].numIndices];
		for(int mtl = 0; mtl < atomics[0].meshes.size(); mtl++)
		// fill indices data
		indCount = 0;
		for(int j = 0; j < bone.triangles.size(); j++)
		{
			atomics[0].meshes[atomics[0].triangles[j].d].indices[atomics[0].meshes[atomics[0].triangles[j].d].indexCounter] = newVerts[indCount].vertId;
			atomics[0].meshes[atomics[0].triangles[j].d].indices[atomics[0].meshes[atomics[0].triangles[j].d].indexCounter + 1] = newVerts[indCount + 1].vertId;
			atomics[0].meshes[atomics[0].triangles[j].d].indices[atomics[0].meshes[atomics[0].triangles[j].d].indexCounter + 2] = newVerts[indCount + 2].vertId;
			atomics[0].meshes[atomics[0].triangles[j].d].indexCounter += 3;
			indCount += 3;
		}

		delete[] newVerts;
		//FILE *f = fopen("test.txt", "at");
		//fputs("\n\nNEW MODEL\n", f);
		//sprintf(temp, "\nTriangles: %d\n", atomics[0].totalNumFaces);
		//fputs(temp, f);
		//for(int i = 0; i < atomics[0].totalNumFaces; i++)
		//{
		//	sprintf(temp, "%d %d %d %d\n", atomics[0].triangles[i].a, atomics[0].triangles[i].b, atomics[0].triangles[i].c , atomics[0].triangles[i].d);
		//	fputs(temp, f);
		//}
		//
		//fputs("\nMeshes:\n", f);
		//for(int i = 0; i < atomics[0].meshes.size(); i++)
		//{
		//	fputs("\nMesh:\n", f);
		//	for(int j = 0; j < atomics[0].meshes[i].numIndices / 3; j++)
		//	{
		//		sprintf(temp, "%d %d %d\n", atomics[0].meshes[i].indices[j * 3], atomics[0].meshes[i].indices[j * 3 + 1], atomics[0].meshes[i].indices[j * 3 + 2]);
		//		fputs(temp, f);
		//	}
		//}
		//
		//if(atomics[0].hasPositions)
		//{
		//	fputs("\nPositions:\n", f);
		//	for(int i = 0; i < atomics[0].totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f %f\n", atomics[0].positions[i].x, atomics[0].positions[i].y, atomics[0].positions[i].z);
		//		fputs(temp, f);
		//	}
		//}
		//if(atomics[0].hasNormals)
		//{
		//	fputs("\nNormals:\n", f);
		//	for(int i = 0; i < atomics[0].totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f %f\n", atomics[0].normals[i].x, atomics[0].normals[i].y, atomics[0].normals[i].z);
		//		fputs(temp, f);
		//	}
		//}
		//if(atomics[0].hasColors)
		//{
		//	fputs("\nColors:\n", f);
		//	for(int i = 0; i < atomics[0].totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%X\n", atomics[0].colors[i]);
		//		fputs(temp, f);
		//	}
		//}
		//if(atomics[0].numTexCoords > 0)
		//{
		//	fputs("\nTex Coors:\n", f);
		//	for(int i = 0; i < atomics[0].totalNumVertex; i++)
		//	{
		//		sprintf(temp, "%f %f\n", atomics[0].texCoords[0][i].x, atomics[0].texCoords[0][i].y);
		//		fputs(temp, f);
		//	}
		//}
		//fclose(f);
	}
};

class CClump
{
public:
	struct
	{
		unsigned int numAtomics;
		unsigned int numLights;
		unsigned int numCameras;
	} _StreamData;

	std::vector<CMaterial> materials;

	std::vector<CFrame> frames;

	unsigned int GetBoneIndexById(unsigned int id)
	{
		for(int i = 0; i < frames.size(); i++)
		{
			if(id == frames[i].bone.boneId)
				return i;
		}
		return 0;
	}

	unsigned int GetFrameIdFromBoneId(unsigned short index)
	{
		for(int fr = 0; fr < frames.size(); fr++)
		{
			if(frames[fr].boneIndex == index)
				return fr;
		}
		return 0;
	}

	unsigned int GetAtomicGeometryId(unsigned int geometry)
	{
		unsigned int count = 0;
		for(int i = 0; i < geometry; i++)
		{
			if(frames[i].bone.triangles.size() > 0)
				count++;
		}
		return count;
	}

	bool Write(RwStream *stream)
	{
		unsigned int extSize = 0;
		_StreamData.numAtomics = 0;
		unsigned int numFrames = frames.size();
		unsigned int geometriesSize = 0;
		for(int i = 0; i < numFrames; i++)
		{
			extSize += 12 + 12 + strlen(frames[i].name);
			_StreamData.numAtomics += frames[i].atomics.size();
		}
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
				geometriesSize += 12 + frames[i].atomics[j].GetStreamSize();
		}
		// clump
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_CLUMP, 16 + geometriesSize + 12 + 12 + 52 * _StreamData.numAtomics +
			24 + 12 + numFrames * 56 + 16 + extSize, 
			0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 12, 0x36003, 0xFFFF))
			return false;
		_StreamData.numLights = 0;
		_StreamData.numCameras = 0;
		if(!gtaRwStreamWrite(stream, &this->_StreamData, 12))
			return false;
		// frame list
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_FRAMELIST, numFrames * 56 + 16 + extSize, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 56 * numFrames + 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &numFrames, 4))
			return false;
		for(int i = 0; i < numFrames; i++)
		{
			frames[i].rotation[0] = Vector3(1.0f, 0.0f, 0.0f);
			frames[i].rotation[1] = Vector3(0.0f, 1.0f, 0.0f);
			frames[i].rotation[2] = Vector3(0.0f, 0.0f, 1.0f);
			if(!gtaRwStreamWrite(stream, &frames[i].rotation[0].x, 56))
				return false;
		}
		for(int i = 0; i < numFrames; i++)
		{
			if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 12 + strlen(frames[i].name), 0x36003, 0xFFFF))
				return false;
			if(!gtaRwStreamWriteVersionedChunkHeader(stream, 0x253F2FE, strlen(frames[i].name), 0x36003, 0xFFFF))
				return false;
			if(!gtaRwStreamWrite(stream, frames[i].name, strlen(frames[i].name)))
				return false;
		}
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_GEOMETRYLIST, geometriesSize + 16, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &_StreamData.numAtomics, 4))
			return false;
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
				frames[i].atomics[j].Write(stream);
		}
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
			{
				frames[i].atomics[j]._StreamData.Atomic.frameId = i;
				frames[i].atomics[j]._StreamData.Atomic.geometryId = i;
				frames[i].atomics[j]._StreamData.Atomic.unk5 = 5;
				frames[i].atomics[j]._StreamData.Atomic.unk0 = 0;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_ATOMIC, 40, 0x36003, 0xFFFF))
					return false;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 16, 0x36003, 0xFFFF))
					return false;
				if(!gtaRwStreamWrite(stream, &frames[i].atomics[j]._StreamData.Atomic, 16))
					return false;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
					return false;
			}
		}
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
			return false;
		return true;
	};

		bool WriteWft(RwStream *stream)
	{
		unsigned int extSize = 0;
		_StreamData.numAtomics = 0;
		unsigned int numFrames = frames.size();
		unsigned int geometriesSize = 0;
		for(int i = 0; i < numFrames; i++)
		{
			extSize += 12 + 12 + strlen(frames[i].name);
			_StreamData.numAtomics += frames[i].atomics.size();
		}
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
				geometriesSize += 12 + frames[i].atomics[j].GetStreamSize();
		}
		// clump
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_CLUMP, 16 + geometriesSize + 12 + 12 + 52 * _StreamData.numAtomics +
			24 + 12 + numFrames * 56 + 16 + extSize, 
			0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 12, 0x36003, 0xFFFF))
			return false;
		_StreamData.numLights = 0;
		_StreamData.numCameras = 0;
		if(!gtaRwStreamWrite(stream, &this->_StreamData, 12))
			return false;
		// frame list
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_FRAMELIST, numFrames * 56 + 16 + extSize, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 56 * numFrames + 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &numFrames, 4))
			return false;
		for(int i = 0; i < numFrames; i++)
		{
			if(!gtaRwStreamWrite(stream, &frames[i].rotation[0].x, 56))
				return false;
		}
		for(int i = 0; i < numFrames; i++)
		{
			if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 12 + strlen(frames[i].name), 0x36003, 0xFFFF))
				return false;
			if(!gtaRwStreamWriteVersionedChunkHeader(stream, 0x253F2FE, strlen(frames[i].name), 0x36003, 0xFFFF))
				return false;
			if(!gtaRwStreamWrite(stream, frames[i].name, strlen(frames[i].name)))
				return false;
		}
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_GEOMETRYLIST, geometriesSize + 16, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 4, 0x36003, 0xFFFF))
			return false;
		if(!gtaRwStreamWrite(stream, &_StreamData.numAtomics, 4))
			return false;
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
				frames[i].atomics[j].Write(stream);
		}
		for(int i = 0; i < numFrames; i++)
		{
			for(int j = 0; j < frames[i].atomics.size(); j++)
			{
				frames[i].atomics[j]._StreamData.Atomic.frameId = i;
				frames[i].atomics[j]._StreamData.Atomic.geometryId = GetAtomicGeometryId(i);
				frames[i].atomics[j]._StreamData.Atomic.unk5 = 5;
				frames[i].atomics[j]._StreamData.Atomic.unk0 = 0;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_ATOMIC, 40, 0x36003, 0xFFFF))
					return false;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_STRUCT, 16, 0x36003, 0xFFFF))
					return false;
				if(!gtaRwStreamWrite(stream, &frames[i].atomics[j]._StreamData.Atomic, 16))
					return false;
				if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
					return false;
			}
		}
		if(!gtaRwStreamWriteVersionedChunkHeader(stream, rwID_EXTENSION, 0, 0x36003, 0xFFFF))
			return false;
		return true;
	};
};