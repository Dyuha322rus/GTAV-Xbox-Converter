#include "ModelReader.h"
#include "CClump.h"
#include "FileReader.h"
#include "TextureReader.h"
#include "CTexDictionary.h"
#include "IdeWplReader.h"
#include "CollisionModel.h"
#include "FileSearch.h"
#include "Hash.h"
#include "Paths.h"
#include "Optimise.h"
#include "LogFile.h"
#include "ModelReader.h"
#include "CClump.h"
#define DEL_ARRAY(array) if (array){/*return;*/delete[] array;array = NULL;}
#define DEL(array) if (array){/*return;*/delete array;array = NULL; }

struct CShaderName
{
	unsigned int hash;
	char name[100];
};
bool bDebug = false;
std::vector<CShaderName> shaderNames;

char noName[] = "__UNKNOWN_SHADER__";

void GetShaders(char *fileName)
{
	CShaderName shader;
	_splitpath_s(fileName, NULL,0, NULL,0, shader.name,100, NULL,0);
	shader.hash = GetHashKey(shader.name, 0);
	shaderNames.push_back(shader);
}

char *GetShaderName(unsigned int hash)
{
	for (UINT i = 0; i < shaderNames.size(); i++)
	{
		if (shaderNames[i].hash == hash)
			return shaderNames[i].name;
	}
	return noName;
}

RSC_DRAWABLE XDrawableReadHeader(CResource *XdrFile)
{
	RSC_DRAWABLE Drawable;
	XdrFile->ReadCpu(&Drawable, sizeof(RSC_DRAWABLE));
	// Wiggle it!
	Drawable.SwapData();
	return Drawable;
}

RSC_DRAWABLE XDrawableReadHeaderAtPtr(CResource *XdrFile, RSC_POINTER Ptr)
{
	RSC_DRAWABLE Drawable;
	XdrFile->ReadAtPointer(Ptr, &Drawable, sizeof(RSC_DRAWABLE));
	// Wiggle it!
	Drawable.SwapData();
	return Drawable;
}

void XDrawableDebugHeader(RSC_DRAWABLE *Drawable){
	if (bDebug){
		printf("Reading Xenon Drawable Object\n");
		printf("Bounding Box Info:\n");
		printf("Min AABB: %f, %f, %f.\n", Drawable->aabbMin.x, Drawable->aabbMin.y, Drawable->aabbMin.z);
		printf("Max AABB: %f, %f, %f.\n", Drawable->aabbMax.x, Drawable->aabbMax.y, Drawable->aabbMax.z);
		printf("Center: %f, %f, %f.\n", Drawable->center.x, Drawable->center.y, Drawable->center.z);
		printf("Radius: %f.\n", Drawable->center.w);
	}
}
void XDrawableDebugModelInfo(RSC_MODEL *Model,int nId){
	if (bDebug){
		printf("Reading Xenon Drawable Object Model %i\n", nId);
		printf("Geometry Count: %i\n", Model->geometriesCount);
	}
}

RSC_COLLECTION XDrawableReadPtrCollectionAtPtr(CResource *XdrFile, RSC_POINTER Ptr)
{
	RSC_COLLECTION PtrCollection;
	XdrFile->ReadAtPointer(Ptr, &PtrCollection, sizeof(RSC_COLLECTION));
	PtrCollection.SwapData();
	return PtrCollection;
}

CFrame XDrawableToDrawableGetFrame(int nID,char* sName)
{
	CFrame Frame;

	Frame.parentFrame = nID == 0 ? -1 : 0;
	Frame.matrixFlags = 0;
	if (strlen(sName)<=23)
		strcpy_s(Frame.name, sName);
	else
		strcpy_s(Frame.name, "tomuchsymbols");
	if (nID != 0)
		sprintf_s(Frame.name, "%s%d", Frame.name, nID + 1);

	return Frame;
}

CModelInstance XDrawableToDrawableGetAtomicInfo(RSC_DRAWABLE *DrawableHeader)
{
	CModelInstance AtomicHeader;
	AtomicHeader.bbox.aabbMax = *(Vector3 *)&DrawableHeader->aabbMax;
	AtomicHeader.bbox.aabbMin = *(Vector3 *)&DrawableHeader->aabbMin;
	AtomicHeader.bsphere.center = *(Vector3 *)&DrawableHeader->center;
	AtomicHeader.bsphere.radius = DrawableHeader->center.w;
	return AtomicHeader;
}

CRageGeometry XDrawableToDrawableGetGeometry(CResource *XdrFile, RSC_GEOMETRY* XGeometry,UINT nShaderID){
	CRageGeometry outGeometry;
	RSC_INDEXBUFFER iBuffer;
	RSC_VERTEXBUFFER vBuffer;

	outGeometry.facesCount = XGeometry->faceCount;
	outGeometry.indexBuffer = new unsigned short[XGeometry->indexCount];

	XdrFile->ReadAtPointer(XGeometry->indexBuffers[0], &iBuffer, sizeof(RSC_INDEXBUFFER));
	iBuffer.SwapData();

	XdrFile->ReadAtPointer(iBuffer.indexData, outGeometry.indexBuffer, XGeometry->indexCount * 2);
	for (UINT k = 0; k < XGeometry->indexCount; k++)
		SwapEndian(outGeometry.indexBuffer[k]);

	outGeometry.indexCount = XGeometry->indexCount;
	outGeometry.indicesPerFace = XGeometry->indicesPerFace;
	XdrFile->ReadAtPointer(XGeometry->vertexBuffers[0], &vBuffer, sizeof(RSC_VERTEXBUFFER));
	vBuffer.SwapData();

	outGeometry.vertexBuffer = new unsigned char[vBuffer.vertexSize * XGeometry->vertexCount];
	XdrFile->ReadAtPointer(vBuffer.vertexData, outGeometry.vertexBuffer, vBuffer.vertexSize * XGeometry->vertexCount);

	XdrFile->ReadAtPointer(vBuffer.declaration, &outGeometry.rageDeclaration, sizeof(grcVertexDeclaration));
	outGeometry.rageDeclaration.SwapData();
	
	outGeometry.shaderId = nShaderID;
	outGeometry.vertexCount = XGeometry->vertexCount;
	return outGeometry;
}

CMaterial XDrawableToDrawableGetMaterial(CResource *XDRFile, RSC_TEXTURE_V *Texture, UINT xShaderNameHash){
	CMaterial outMat;
	if (Texture&&XDRFile){
		char temp[512];
		XDRFile->ReadStringAtPointer(Texture->GetName(), temp, 200);
		outMat.Set(temp, CColor(255, 255, 255, 255), xShaderNameHash);
	}
	else
		outMat.Set("\0", CColor(255, 255, 255, 255), xShaderNameHash);
	strcpy_s(outMat.shaderName, GetShaderName(xShaderNameHash));
	return outMat;
}

BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int GetGPUDataOffset(int offs){
	return ((offs >> 8) << 16) >> 8;
}

void DoTheSwizzle(unsigned char* img, int size){
	for (int i = 0; i < size; i += 4)
	{
		int tmp[2];
		tmp[0] = img[i];
		tmp[1] = img[i + 1];
		img[i] = img[i + 2];
		img[i + 1] = img[i + 3];
		img[i + 2] = tmp[0];
		img[i + 3] = tmp[1];
	}
}

void XDrawableConvertInternalTexDict(CResource *XDRFile, RSC_SHADERGOUP* shaderGroup, char *name){
	CTexDictionary txd;
	CImage image;
	RSC_TEXTURE_V *texRsc;
	RSC_TXD txdRsc;
	RSC_POINTER *texturesPtrs;
	char temp[1024];
	if (shaderGroup->texDictionary.dataType)
	{
		//FileLOG(f, " This file has internal TXD\n");
		// generate a name for internal txd
		strcat(name, ".internal.txd");
		LOG(" Working on internal TXD (%s)\n", name);
		texRsc = new RSC_TEXTURE_V;
		XDRFile->ReadAtPointer(shaderGroup->texDictionary, &txdRsc, sizeof(RSC_TXD));
		txdRsc.SwapData();
		//FileLOG(f, "  Num textures in TXD: %d\n", txdRsc.count);
		//wdrFile.IsPointerValid(txdRsc.texturesPtrs, true);
		texturesPtrs = new RSC_POINTER[txdRsc.dwTextureCount];
		LOG("  Total textures in txd: %d\n", txdRsc.dwTextureCount);
		if (txdRsc.dwTextureCount > 0)
		{
			XDRFile->ReadAtPointer(txdRsc.texturesPtrs, texturesPtrs, txdRsc.dwTextureCount * 4);
			for (int i = 0; i < txdRsc.dwTextureCount; i++)
			{
				texRsc->SaveVtable();
				SwapEndian(texturesPtrs[i]);
				XDRFile->ReadAtPointer(texturesPtrs[i], texRsc, sizeof(RSC_TEXTURE_V));
				texRsc->SwapData();
				texRsc->ResetVtable();
				XDRFile->ReadStringAtPointer(texRsc->GetName(), image.name, 200);
				//FileLOG(f, "   Texture %d %s Width %d Height %d Levels %d\n",
				//	i, image.name, texRsc->width, texRsc->height, texRsc->levels);
				image.nameLength = XDRFile->ReadStringAtPointer(texRsc->GetName(), image.name, 200);
				if (!strncmp(image.name, "pack:/", 6))
				{
					strcpy(temp, image.name);
					strcpy(image.name, &temp[6]);
					image.nameLength -= 6;
				}
				if (!strcmp(&image.name[image.nameLength - 5], ".dds"))
				{
					image.name[image.nameLength - 5] = '\0';
					image.nameLength -= 4;
				}
				image.format = (D3DFORMAT)texRsc->GetFormat();
				image.height = texRsc->GetHeight();
				image.width = texRsc->GetWidth();
				image.levels = texRsc->GetLevels();
				image.stride = image.width/2;
				// calculate pixelDataSize
				image.pixelsSize = image.stride * image.height;
				/*for (int j = 2; j <= image.levels; j++)
					image.pixelsSize += (image.stride * image.height) >> ((j - 1) * 2);*/
				image.pixels = new unsigned char[image.pixelsSize];
				int offset;
				texRsc->pD3DTexture.offset += 32;
				XDRFile->ReadAtPointer(texRsc->pD3DTexture, &offset, 4);
				SwapEndian(offset);
				offset = GetGPUDataOffset(offset);
				texRsc->pD3DTexture.dataType = RSC_GPU_DATA;
				texRsc->pD3DTexture.offset = offset;
				XDRFile->ReadAtPointer(texRsc->GetPixelData(), image.pixels, image.pixelsSize);
				DoTheSwizzle(image.pixels, image.pixelsSize);
				if (txd.FindTexture(image.name))
				{
					LOG("   texture %s is already present in txd %s", image.name, name);
					delete[] image.pixels;
				}
				else
					txd.images.push_back(image);
				printf("\n%s w %d h %d s %d levels %d dataSize %d", image.name, texRsc->GetWidth(),
					texRsc->GetHeight(), image.stride, texRsc->GetLevels(), image.pixelsSize);
			}
		}
		txd.Build(name);
		delete texRsc;
		delete[] texturesPtrs;
	}
}

void GenerateConvertCollision(char*colName, char* nameHash, CClump *clump, RSC_DRAWABLE *DrawableHeader,bool bEmpty){
	char colHelper[64];
	if (strrchr(colName, '\\')) strrchr(colName, '\\')[0] = '\0';
	strcat(colName, ".col");
	_splitpath(colName, NULL, NULL, colHelper, NULL);
	if (strrchr(colName, '\\')) strrchr(colName, '\\')[1] = '\0';
	strcat(colName, colHelper);
	strcat(colName, "\\");
	strcat(colName, colHelper);
	strcat(colName, ".col");

	unsigned int numGeoms = clump->frames[0].atomics[0].geometries.size();
	RpGeometryOptimised **optList = new RpGeometryOptimised *[numGeoms];
	unsigned int vertCounter = 0;
	unsigned int faceCounter = 0;
	unsigned int newFaceCount = 0;
	unsigned int newVertsCount = 0;

	// calc vertex count
	unsigned int totalVerts = 0;
	for (UINT i = 0; i < numGeoms; i++)
		totalVerts += clump->frames[0].atomics[0].geometries[i].vertexCount;

	for (UINT i = 0; i < numGeoms; i++)
	{
		if (clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod_tnt", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod2", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod2d", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_normal", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_normal_spec", 0)
			|| clump->frames[0].atomics[0].materials[clump->frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_tnt", 0)
			)
		{
			optList[i] = 0;
		}
		else
		{
			optList[i] = new RpGeometryOptimised(clump->frames[0].atomics[0].geometries[i].facesCount, clump->frames[0].atomics[0].geometries[i].vertexCount,
				clump->frames[0].atomics[0].meshes[i].indices, &clump->frames[0].atomics[0].positions[vertCounter], clump->frames[0].atomics[0].geometries[i].shaderId,
				vertCounter);
			if (totalVerts > 30000)
			{
				_rtGeometryOptimize((RpGeometry *)optList[i], 0.5f);
			}
			newFaceCount += optList[i]->numTriangles;
			newVertsCount += optList[i]->numVertices;
		}
		vertCounter += clump->frames[0].atomics[0].geometries[i].vertexCount;
		faceCounter += clump->frames[0].atomics[0].geometries[i].facesCount;
	}
	if (newFaceCount > 30000 || newVertsCount > 30000)
	{
		printf("ERROR: Too many faces/vertices in COL model");
	}

	if (newVertsCount < 30000 || !bEmpty){

		RwStream *col = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMAPPEND, colName);
		CollisionModel2 colmodel;
		colmodel.fourcc = '2LOC';
		colmodel.flags = 2;
		strcpy(colmodel.name, nameHash);
		colmodel.unused = 0;
		colmodel.modelIndex = 20000;
		colmodel.bbox.Set(*(Vector3 *)&DrawableHeader->aabbMin, *(Vector3 *)&DrawableHeader->aabbMax);
		/*colmodel.bbox.aabbMax.x -= drawable.center.x;
		colmodel.bbox.aabbMax.y -= drawable.center.y;
		colmodel.bbox.aabbMax.z -= drawable.center.z;
		colmodel.bbox.aabbMin.x -= drawable.center.x;
		colmodel.bbox.aabbMin.y -= drawable.center.y;
		colmodel.bbox.aabbMin.z -= drawable.center.z;*/
		colmodel.bsphere.center = *(Vector3 *)&DrawableHeader->center;
		colmodel.bsphere.radius = DrawableHeader->center.w;
		//colmodel.bsphere.center = Vector3(0,0,0);
		colmodel.numBoxes = 0;
		colmodel.numFaces = newFaceCount;
		colmodel.numSpheres = 0;
		colmodel.offsetBoxes = 0;
		colmodel.offsetLines = 0;
		colmodel.offsetSpheres = 0;
		colmodel.offsetTrianglePlanes = 0;
		unsigned int verticesSize = newVertsCount * 6;
		if (verticesSize % 4)
			verticesSize += 2;
		colmodel.size = sizeof(CollisionModel2)+colmodel.numFaces * 8 + verticesSize - 8;
		colmodel.offsetVertices = sizeof(CollisionModel2)-4;
		colmodel.offsetFaces = colmodel.offsetVertices + verticesSize;
		gtaRwStreamWrite(col, &colmodel, sizeof(CollisionModel2));
		ColVertex *colVerts = new ColVertex[newVertsCount];

		vertCounter = 0;
		for (UINT j = 0; j < clump->frames[0].atomics[0].geometries.size(); j++)
		{
			if (optList[j])
			{
				for (int i = 0; i < optList[j]->numVertices; i++)
				{
					colVerts[i + vertCounter].x = optList[j]->morphTarget->verts[i].x * 128;
					colVerts[i + vertCounter].y = optList[j]->morphTarget->verts[i].y * 128;
					colVerts[i + vertCounter].z = optList[j]->morphTarget->verts[i].z * 128;
				}
				vertCounter += optList[j]->numVertices;
			}
		}

		gtaRwStreamWrite(col, colVerts, verticesSize);
		DEL_ARRAY(colVerts)
			ColTriangle *colTris = new ColTriangle[colmodel.numFaces];
		char *matName;
		unsigned short matId;

		faceCounter = 0;
		vertCounter = 0;
		for (UINT j = 0; j < clump->frames[0].atomics[0].geometries.size(); j++)
		{
			if (optList[j])
			{
				for (int i = 0; i < optList[j]->numTriangles; i++)
				{
					colTris[i + faceCounter].a = optList[j]->triangles[i].vertIndex[2] + vertCounter;
					colTris[i + faceCounter].b = optList[j]->triangles[i].vertIndex[1] + vertCounter;
					colTris[i + faceCounter].c = optList[j]->triangles[i].vertIndex[0] + vertCounter;
					// try to match material...
					matId = optList[j]->triangles[i].matIndex;
					if (clump->frames[0].atomics[0].materials[matId]._StreamData.textured)
					{
						matName = clump->frames[0].atomics[0].materials[matId].texture._StreamData.nameAligned;
						if (strstr(matName, "tarmac"))
							colTris[i + faceCounter].material = 1;
						else if (strstr(matName, "grass"))
						{
							if (strstr(matName, "dirt"))
								colTris[i + faceCounter].material = 13;
							else
								colTris[i + faceCounter].material = 10;
						}
						else if (strstr(matName, "stone") || strstr(matName, "rock"))
							colTris[i + faceCounter].material = 35;
						else if (strstr(matName, "wood"))
						{
							if (strstr(matName, "step"))
								colTris[i + faceCounter].material = 73;
							else if (strstr(matName, "bench"))
								colTris[i + faceCounter].material = 70;
							else if (strstr(matName, "floor"))
								colTris[i + faceCounter].material = 22;
							else
								colTris[i + faceCounter].material = 43;
						}
						else if (strstr(matName, "gravel"))
							colTris[i + faceCounter].material = 6;
						else if (strstr(matName, "rubbish"))
							colTris[i + faceCounter].material = 139;
						else if (strstr(matName, "poster") || strstr(matName, "billboard"))
							colTris[i + faceCounter].material = 68;
						else if (strstr(matName, "pavement") || strstr(matName, "alleypath"))
							colTris[i + faceCounter].material = 4;
						else if (strstr(matName, "fence"))
							colTris[i + faceCounter].material = 55;
						else if (strstr(matName, "concrete"))
							colTris[i + faceCounter].material = 7;
						else if (strstr(matName, "girder"))
							colTris[i + faceCounter].material = 56;
						else if (strstr(matName, "gate"))
							colTris[i + faceCounter].material = 54;
						else if (strstr(matName, "tail"))
							colTris[i + faceCounter].material = 107;
						else if (strstr(matName, "glass"))
							colTris[i + faceCounter].material = 45;
						else if (strstr(matName, "metal") || strstr(matName, "alum"))
						{
							if (strstr(matName, "step"))
								colTris[i + faceCounter].material = 162;
							else if (strstr(matName, "floor"))
								colTris[i + faceCounter].material = 164;
							else
								colTris[i].material = 51;
						}
						else if (strstr(matName, "door"))
							colTris[i + faceCounter].material = 158;
						else if (strstr(matName, "rubber") || strstr(matName, "bball_court"))
							colTris[i + faceCounter].material = 67;
						else
							colTris[i + faceCounter].material = 0;
					}
					else
						colTris[i + faceCounter].material = 0;
					//
					colTris[i + faceCounter].lighting = 255;
				}
				faceCounter += optList[j]->numTriangles;
				vertCounter += optList[j]->numVertices;
			}
		}

		for (UINT i = 0; i < clump->frames[0].atomics[0].geometries.size(); i++)
			DEL(optList[i]);
		DEL_ARRAY(optList)

			gtaRwStreamWrite(col, colTris, colmodel.numFaces * 8);
		DEL_ARRAY(colTris)

			gtaRwStreamClose(col, NULL);
	}
	else
	{
		RwStream *col = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMAPPEND, colName);
		CollisionModel2 colmodel;
		colmodel.fourcc = '2LOC';
		colmodel.flags = 2;
		strcpy(colmodel.name, nameHash);
		colmodel.unused = 0;
		colmodel.modelIndex = 20000;
		colmodel.bbox.Set(*(Vector3 *)&DrawableHeader->aabbMin, *(Vector3 *)&DrawableHeader->aabbMax);
		/*colmodel.bbox.aabbMax.x -= drawable.center.x;
		colmodel.bbox.aabbMax.y -= drawable.center.y;
		colmodel.bbox.aabbMax.z -= drawable.center.z;
		colmodel.bbox.aabbMin.x -= drawable.center.x;
		colmodel.bbox.aabbMin.y -= drawable.center.y;
		colmodel.bbox.aabbMin.z -= drawable.center.z;*/
		colmodel.bsphere.center = *(Vector3 *)&DrawableHeader->center;
		colmodel.bsphere.radius = DrawableHeader->center.w;
		//colmodel.bsphere.center = Vector3(0,0,0);
		colmodel.numBoxes = 0;
		colmodel.numFaces = 0;
		colmodel.numSpheres = 0;
		colmodel.offsetBoxes = 0;
		colmodel.offsetLines = 0;
		colmodel.offsetSpheres = 0;
		colmodel.offsetTrianglePlanes = 0;
		colmodel.size = sizeof(CollisionModel2);
		colmodel.offsetVertices = 0;
		colmodel.offsetFaces = 0;
		gtaRwStreamWrite(col, &colmodel, sizeof(CollisionModel2));
		gtaRwStreamClose(col, NULL);
	}
}

void ConvertXDrawableSectionToDFF(CResource *XDRFile, RSC_DRAWABLE* DrawableHeader, char *nameHash, char *name, char *modelName, char *colName, char *fileName, bool bEmpty){
	RSC_COLLECTION ModelsCollection;
	RSC_POINTER* aModelPtrs;
	RSC_MODEL* aModels;
	RSC_POINTER * aGeometryPtrs;
	RSC_GEOMETRY * aGeometries;
	USHORT *aSMappings;
	CClump clump;

	RSC_SHADERGOUP ShaderGroup;
	RSC_POINTER * aShadersPtrs;
	RSC_SHADERPARAM *aShaderParams;
	RSC_SHADER Shader;
	RSC_TEXTURE_V Texture;
	for (int nLodLevel = 0; nLodLevel < 1; nLodLevel++)
	{
		if (DrawableHeader->pModelCollection[nLodLevel].dataType == 0)
			continue;
		ModelsCollection = XDrawableReadPtrCollectionAtPtr(XDRFile, DrawableHeader->pModelCollection[nLodLevel]);
		if (ModelsCollection.count <= 0)
			continue;
		aModelPtrs = new RSC_POINTER[ModelsCollection.count];
		aModels = new RSC_MODEL[ModelsCollection.count];
		XDRFile->ReadAtPointer(ModelsCollection.listOffset, aModelPtrs, 4 * ModelsCollection.count);
		for (int j = 0; j < ModelsCollection.count; j++)
			SwapEndian(aModelPtrs[j]);
		for (int i = 0; i < ModelsCollection.count; i++)
		{
			XDRFile->ReadAtPointer(aModelPtrs[i], &aModels[i], sizeof(RSC_MODEL));
			aModels[i].SwapData();
			XDrawableDebugModelInfo(&aModels[i], i);
			clump.frames.push_back(XDrawableToDrawableGetFrame(i, modelName));

			if (aModels[i].geometriesCount <= 0)
				continue;
			aGeometryPtrs = new RSC_POINTER[aModels[i].geometriesCount];
			aGeometries = new RSC_GEOMETRY[aModels[i].geometriesCount];

			XDRFile->ReadAtPointer(aModels[i].geometriesList, aGeometryPtrs, 4 * aModels[i].geometriesCount);
			for (int j = 0; j < aModels[i].geometriesCount; j++)
				SwapEndian(aGeometryPtrs[j]);
			clump.frames[i].atomics.push_back(XDrawableToDrawableGetAtomicInfo(DrawableHeader));

			aSMappings = new unsigned short[aModels[i].shaderMappingsCount];
			XDRFile->ReadAtPointer(aModels[i].shaderMappings, aSMappings, 2 * aModels[i].shaderMappingsCount);
			for (int j = 0; j < aModels[i].shaderMappingsCount; j++)
				SwapEndian(aSMappings[j]);

			for (int k = 0; k < aModels[i].geometriesCount; k++)
			{
				XDRFile->ReadAtPointer(aGeometryPtrs[k], &aGeometries[k], sizeof(RSC_GEOMETRY));
				aGeometries[k].SwapData();
				clump.frames[i].atomics[0].geometries.push_back(XDrawableToDrawableGetGeometry(XDRFile, &aGeometries[k], aSMappings[k]));
			}

			XDRFile->ReadAtPointer(DrawableHeader->pShaders, &ShaderGroup, sizeof(RSC_SHADERGOUP));
			ShaderGroup.SwapData();
			if (ShaderGroup.texDictionary.dataType)
			{
				char sMName[512];
				char dname[512];
				_splitpath(fileName, NULL, NULL, sMName, NULL);
				strcpy(dname, fileName);
				if (strrchr(dname, '\\')) strrchr(dname, '\\')[1] = '\0';
				strcat(dname, sMName);
				strcat(dname, "+xdr");
				if (!DirectoryExists(dname))
					LOG("%s\n", dname);
			}

			aShadersPtrs = new RSC_POINTER[ShaderGroup.numShaders];
			XDRFile->ReadAtPointer(ShaderGroup.shaders, aShadersPtrs, 4 * ShaderGroup.numShaders);
			for (int k = 0; k < ShaderGroup.numShaders; k++)
				SwapEndian(aShadersPtrs[k]);

			bool bHasMat = false;
			for (int mat = 0; mat < ShaderGroup.numShaders; mat++)
			{
				XDRFile->ReadAtPointer(aShadersPtrs[mat], &Shader, sizeof(RSC_SHADER));
				Shader.SwapData();
				aShaderParams = new RSC_SHADERPARAM[Shader.paramsCount];
				XDRFile->ReadAtPointer(Shader.shaderParams, aShaderParams, 8 * Shader.paramsCount);
				for (int param = 0; param < Shader.paramsCount; param++)
				{
					aShaderParams[param].SwapData();

					if (param == 0 && (aShaderParams[param].type == 0 || aShaderParams[param].type == 131072))
					{
						bHasMat = true;
						Texture.SaveVtable();
						if (aShaderParams[param].param.dataType && aShaderParams[param].param.offset)
						{
							// Read Texture
							XDRFile->ReadAtPointer(aShaderParams[param].param, &Texture, sizeof(RSC_TEXTURE_V));
							Texture.SwapData();

							Texture.ResetVtable();
							clump.frames[i].atomics[0].materials.push_back(XDrawableToDrawableGetMaterial(XDRFile, &Texture, Shader.nameHash));
						}
						else
							clump.frames[i].atomics[0].materials.push_back(XDrawableToDrawableGetMaterial(NULL, NULL, Shader.nameHash));
					}
				}
				DEL_ARRAY(aShaderParams);
			}

			if (!bHasMat || clump.frames[i].atomics[0].materials.size() <= 0 || clump.frames[i].atomics[0].materials.size()<clump.frames[i].atomics[0].meshHeader.numMeshes)
				clump.frames[i].atomics[0].materials.push_back(XDrawableToDrawableGetMaterial(NULL, NULL, -1));

			clump.frames[i].atomics[0].BuildMeshes(0);
			DEL_ARRAY(aGeometryPtrs);
			DEL_ARRAY(aGeometries);
			DEL_ARRAY(aSMappings);
			DEL_ARRAY(aShadersPtrs);
		}
		//XDrawableConvertInternalTexDict(XDRFile, &ShaderGroup, name);
		GenerateConvertCollision(colName, nameHash, &clump, DrawableHeader, bEmpty);
		strcat(name, (nLodLevel<4) ? ((nLodLevel<3) ? ((nLodLevel<2) ? "" : "+med") : "+low") : "+vlow");
		strcat(name, ".dff");
		RwStream *dff = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, name);
		clump.Write(dff);
		gtaRwStreamClose(dff, NULL);
		for (int i = 0; i < ModelsCollection.count; i++)
		{
			for (int j = 0; j < clump.frames[i].atomics[0].geometries.size(); j++)
			{
				DEL_ARRAY(clump.frames[i].atomics[0].geometries[j].indexBuffer);
				DEL_ARRAY(clump.frames[i].atomics[0].geometries[j].vertexBuffer);
			}
		}

		DEL_ARRAY(aModelPtrs);
		DEL_ARRAY(aModels);
	}
}
void XDrawableConvertToDFF(char *fileName)
{
	char nameHash[1024];
	char name[1024];
	char modelName[1024];
	char colName[1024];
	_splitpath(fileName, NULL, NULL, modelName, NULL);
	strcpy(name, fileName);
	strcpy(colName, fileName);
	if (strrchr(name, '\\')) strrchr(name, '\\')[1] = '\0';
	unsigned int hash = GetHashKey(modelName, 0);
	gDFFs.push_back(hash);
	sprintf(nameHash, "%X", hash);
	//sprintf(nameHash, "%s", modelName);
	strcat(name, nameHash);
	printf("%s\n",modelName);
	unsigned int len = strlen(name);
	CResource XDRFile(fileName);
	RSC_DRAWABLE DrawableHeader;
	DrawableHeader = XDrawableReadHeader(&XDRFile);
	XDrawableDebugHeader(&DrawableHeader);
	ConvertXDrawableSectionToDFF(&XDRFile, &DrawableHeader, nameHash, name, modelName, colName, fileName,false);
}


void XDrawableDictionaryConvertToDFF(char *fileName)
{
	char nameHash[1024];
	char name[1024];
	char modelName[1024];
	char colName[1024];
	_splitpath(fileName, NULL, NULL, modelName, NULL);
	strcpy(colName, fileName);
	printf("%s\n", modelName);

	CResource XDDFile(fileName);

	RSC_DRAWABLEDICTIONARY DrawableDictHeader;
	XDDFile.ReadCpu(&DrawableDictHeader, sizeof(RSC_DRAWABLEDICTIONARY));
	DrawableDictHeader.SwapData();
	RSC_POINTER* drawablePtrs = new RSC_POINTER[DrawableDictHeader.drawablePtrs.count];
	unsigned int* hashes = new unsigned int[DrawableDictHeader.drawablePtrs.count];
	XDDFile.ReadAtPointer(DrawableDictHeader.hashPtrs.listOffset, hashes, 4 * DrawableDictHeader.drawablePtrs.count);
	XDDFile.ReadAtPointer(DrawableDictHeader.drawablePtrs.listOffset, drawablePtrs, 4 * DrawableDictHeader.drawablePtrs.count);

	for (int i = 0; i < DrawableDictHeader.drawablePtrs.count; i++)
	{
		SwapEndian(hashes[i]);
		SwapEndian(drawablePtrs[i]);

		strcpy(name, fileName);
		if (strrchr(name, '\\')) strrchr(name, '\\')[1] = '\0';

		gDFFs.push_back(hashes[i]);
		sprintf(nameHash, "%X", hashes[i]);
		HashedName hn;
		strcpy(hn.name, nameHash);
		hn.hash = hashes[i];
		hashNames.push_back(hn);
		strcat(name, nameHash);

		RSC_DRAWABLE DrawableHeader;
		DrawableHeader = XDrawableReadHeaderAtPtr(&XDDFile, drawablePtrs[i]);
		XDrawableDebugHeader(&DrawableHeader);
		ConvertXDrawableSectionToDFF(&XDDFile, &DrawableHeader, nameHash, name, nameHash, colName, fileName,true);
	}
	DEL_ARRAY(drawablePtrs);
	DEL_ARRAY(hashes);
}