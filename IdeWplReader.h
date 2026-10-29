#pragma once
#include "ModelReader.h"
#include <vector>
#include "LogFile.h"
#include "FileSearch.h"
#include "Hash.h"

extern unsigned int gBaseId;
extern Vector3 MapMove;
struct HashedName
{
	unsigned int hash;
	char name[1024];
};
std::vector<HashedName> hashNames;

char* FindName(unsigned int hash) {
	char temp[1024];
	bool found=false;
	/*for (int i = 0; i < hashNames.size(); i++)
	{
		if (hashNames[i].hash == hash)
		{
			strcpy(temp, hashNames[i].name);
			found = true;
			break;
		}
	}*/
	//if (!found)
		sprintf(temp, "%X", hash);
	return temp;
}

char* FindName2(unsigned int hash) {
	char temp[1024];
	bool found = false;
	for (int i = 0; i < hashNames.size(); i++)
	{
	if (hashNames[i].hash == hash)
	{
	strcpy(temp, hashNames[i].name);
	found = true;
	break;
	}
	}
	if (!found)
		sprintf(temp, "%X", hash);
	return temp;
}

void AddToHashNames(char *filename)
{
	char temp[1024];
	HashedName hn;
	_splitpath(filename, NULL, NULL, temp, NULL);
	strcpy(hn.name, temp);
	hn.hash = GetHashKey(temp, 0);
	hashNames.push_back(hn);
}

struct XTYPMAP_Header{
	int _vmt;
	RSC_COLLECTION ptrToEnd;
	char someStaticBytes[19];
	char type;
	RSC_COLLECTION AdditionalHeader;
	RSC_COLLECTION ptr2;
	RSC_COLLECTION PtrToSectionPtrCollections;
	RSC_COLLECTION ptr4;
	RSC_COLLECTION ptr5;
	short unk0;
	short unk1;
	short SectionBlockCount;
	short unk2;
	char padding[32];
	void SwapData()
	{
		PtrToSectionPtrCollections.SwapData();
		SwapEndian(SectionBlockCount);
	}
};

struct RSC_BlockPtr
{
	int _vmt;
	int BlockByteSize;
	RSC_POINTER listOffset;
	int unk;
	void SwapData()
	{
		SwapEndian(_vmt);
		SwapEndian(BlockByteSize);
		SwapEndian(listOffset);
	}
};

struct XTYP_Obj{
	int unk1;//probably lod xdd hash not sure
	float DrawDistance;
	int unk2[2];
	Vector4 minAABB;
	Vector4 maxAABB;
	Vector4 unk3;
	float unk4;
	float unk5;
	unsigned int modelHash;
	unsigned int textuteHash;
	int unk6[4];
	int unk0[4];
	void SwapData()
	{
		SwapEndian(modelHash);
		SwapEndian(textuteHash);
		SwapEndian(DrawDistance);
	}
};

struct XMAP_Inst{
	int unk0;//probably lod xdd hash not sure
	unsigned int modelHash;
	int unk1[2];
	Vector4 pos;
	Vector4 rot;
	Vector2 unk2;
	int lodID;
	float DrawDist;
	int unk3[12];
	void SwapData()
	{
		SwapEndian(modelHash);
		SwapEndian(pos);
		SwapEndian(rot);
		SwapEndian(DrawDist);
		SwapEndian(lodID);
	}
};

class IdeObjectEntry
{
public:
	UINT id = -1;
	UINT modelHash = -1;
	UINT txdHash = -1;
	bool modelExists;
	bool txdExists;
	char modelName[100];
	char txdName[100];
	unsigned int drawDist;
	unsigned int flags;
	unsigned int flags2;

	IdeObjectEntry()
	{
		id = -1;
		modelName[0] = '\0';
		txdName[0] = '\0';
		drawDist = 0.0f;
		flags = 0;
		flags2 = 0;
	}

	IdeObjectEntry(char *name, char *txd, float drawdist, unsigned int flags_1, unsigned int flags_2, bool hasModel, bool hasTxd)
	{
		id = -1;
		strcpy(modelName, name);
		strcpy(txdName, txd);
		drawDist = drawdist;
		flags = flags_1;
		flags2 = flags_2;
		modelExists = hasModel;
		txdExists = hasTxd;
	}
	IdeObjectEntry(UINT ID, char *name, char *txd, float drawdist, unsigned int flags_1, bool hasModel, bool hasTxd)
	{
		id = ID;
		strcpy(modelName, name);
		strcpy(txdName, txd);
		drawDist = drawdist;
		flags = flags_1;
		modelExists = hasModel;
		txdExists = hasTxd;
	}
	IdeObjectEntry(UINT ID, UINT name, UINT txd, float drawdist, unsigned int flags_1, bool hasModel, bool hasTxd)
	{
		id = ID;
		modelHash = name;
		txdHash = txd;
		drawDist = drawdist;
		flags = flags_1;
		modelExists = hasModel;
		txdExists = hasTxd;
	}
};

class IdeTimeObjectEntry : public IdeObjectEntry
{
public:
	unsigned int nightFlag;

	IdeTimeObjectEntry(char *name, char *txd, float drawdist, unsigned int flags_1, unsigned int flags_2, unsigned int night_flag, bool hasModel, bool hasTxd)
	{
		strcpy(modelName, name);
		strcpy(txdName, txd);
		drawDist = drawdist;
		flags = flags_1;
		flags2 = flags_2;
		nightFlag = night_flag;
		modelExists = hasModel;
		txdExists = hasTxd;
	}
};

class IdeAnimObjectEntry : public IdeObjectEntry
{
public:
	char animName[20];

	IdeAnimObjectEntry(char *name, char *txd, float drawdist, unsigned int flags_1, unsigned int flags_2, char *anim, bool hasModel, bool hasTxd)
	{
		strcpy(modelName, name);
		strcpy(txdName, txd);
		drawDist = drawdist;
		flags = flags_1;
		flags2 = flags_2;
		strcpy(animName, anim);
		modelExists = hasModel;
		txdExists = hasTxd;
	}
};

class IdeTxdLink
{
public:
	char name1[24];
	char name2[24];
	bool name1Exist;
	bool name2Exist;

	IdeTxdLink(char *child, char *parent, bool hasName1, bool hasName2)
	{
		strcpy(name1, child);
		strcpy(name2, parent);
		name1Exist = hasName1;
		name2Exist = hasName2;
	}
};

class Ide2dfx
{
public:
	char modelName[20];
	RwV3d posn;
	unsigned int type;

	union{
		struct Light{
			bool coronarefl;
			unsigned char red;
			unsigned char green;
			unsigned char blue;
			unsigned char flags;
			unsigned char trigger;
			float coronaSize;
			float shadowSize;
			float pointlightRange;
		} light;
	};
};

std::vector<UINT> gDFFs;
bool gFileFound;
char gFileName[100];
char* gFileExt = ".dff";
char gFilePath[512];
#include <iostream>
#include <sstream>
void FindFileByExtCB(char *filename)
{
	char name[100];
	char ext[100];
	_splitpath(filename, NULL, NULL, name, ext);
	gFileFound = true;
	strcpy(gFilePath, filename);
	std::stringstream strValue;
	strValue << name;

	unsigned int hash;
	strValue >> hash;
	printf("\nfilename %i\n", hash);
	gDFFs.push_back(hash);
}
void FindNamedFileCB(char *filename)
{
	char name[100];
	_splitpath(filename, NULL, NULL, name, NULL);
	if(!strcmp(name, gFileName))
	{
		gFileFound = true;
		strcpy(gFilePath, filename);
	}
}

bool FindFileGlobal(char *name, char *ext, char *path = NULL)
{
	gFileFound = false;
	strcpy(gFileName, name);
	SearchFiles(ext, (LPSEARCHFUNC)FindNamedFileCB);
	if(gFileFound)
	{
		if(path)
			strcpy(path, gFilePath);
		return true;
	}
	return false;
}
bool FindFileGlobalByExt(char *ext, char *path = NULL)
{
	SearchFiles(ext, (LPSEARCHFUNC)FindFileByExtCB);
	return true;
}
class IdeFile
{
public:
	unsigned int baseId;
	char path[512];
	char directory[512];
	std::vector<IdeObjectEntry> objs;
	std::vector<IdeTimeObjectEntry> tobj;
	std::vector<IdeAnimObjectEntry> anim;
	std::vector<IdeTxdLink> txdp;
	std::vector<Ide2dfx> _2dfx;

	IdeFile()
	{
		baseId = 0;
	}

	void Clear()
	{
		objs.clear();
		tobj.clear();
		anim.clear();
		_2dfx.clear();
	}

	void SetBaseId(unsigned int id)
	{
		baseId = id;
	}

	unsigned int GetObjectsCount()
	{
		return objs.size() + tobj.size() + anim.size();
	}

	int GetObjectByName(char *name)
	{
		for(int j = 0; j < objs.size(); j++)
		{
			if(objs[j].modelExists && objs[j].txdExists && !strcmp(objs[j].modelName, name))
				return baseId + j;
		}
		for(int j = 0; j < tobj.size(); j++)
		{
			if(tobj[j].modelExists && tobj[j].txdExists && !strcmp(tobj[j].modelName, name))
				return baseId + objs.size() + j;
		}
		return -1;
	}

	bool ModelHasFile(char *name)
	{
		char temp[600];
		strcpy(temp, directory);
		strcat(temp, "\\");
		strcat(temp, name);
		strcat(temp, ".dff");
		FILE *file = fopen(temp, "rb");
		if(file)
		{
			fclose(file);
			return true;
		}
		else
		{
			// Search for this file!
			if(FindFileGlobal(name, "*.dff"))
				return true;
			return false;
		}
	}

	bool TxdHasFile(char *name)
	{
		char temp[600];
		strcpy(temp, directory);
		strcat(temp, "\\");
		strcat(temp, name);
		strcat(temp, ".txd");
		FILE *file = fopen(temp, "rb");
		if(file)
		{
			fclose(file);
			return true;
		}
		else
		{
			// Search for this file!
			if(FindFileGlobal(name, "*.txd"))
				return true;
			return false;
		}
	}

	bool InternalTxdToSimpleTxd(char *name)
	{
		char temp[600];
		char temp2[600];
		FILE *file2;
		strcpy(temp, name);
		strcat(temp, ".internal");
		if (FindFileGlobal(temp, "*.txd", temp2))
		{
			strcpy(temp, temp2);
			temp2[strlen(temp2) - 12] = '\0';
			strcat(temp2, "txd");
			file2 = fopen(temp2, "rb");
			if(file2)
			{
				fclose(file2);
				DeleteFile(temp2);
				LOG("   Deleted txd file %s\n", temp2);
			}
			rename(temp, temp2);
			LOG("   Renamed txd file %s to %s because related dff uses only this txd\n", temp, temp2);
			return true;
		}
		else
			return false;
	}

	bool MergeTxdWithInternal(char *txdName, char *modelName)
	{
		char temp[600];
		char globalPath[600];
		char parentTxdPath[600];
		char internalTxdPath[600];
		CTexDictionary modelTxd;
		CTexDictionary txd;
		CImage image;
		RwStream *stream;
		unsigned int length, version, numTextures, size, offset;
		NativeTexturePC texData;
		char *data;

		//strcpy(temp, directory);
		//strcat(temp, "\\");
		strcpy(temp, modelName);
		strcat(temp, ".internal");

		//FILE *modelTxdFile = fopen(temp, "rb");

		if (FindFileGlobal(temp, "*.txd", internalTxdPath))
		{
			if(!FindFileGlobal(txdName, "*.txd", parentTxdPath))
			{
				LOG("   Can't find %s TXD for merging internal %s TXD\n", txdName, modelName);
				return false;
			}
			stream = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, parentTxdPath);
			if(stream)
			{
				if(gtaRwStreamFindChunk(stream, rwID_TEXDICTIONARY, &length, &version) 
					&& gtaRwStreamFindChunk(stream, rwID_STRUCT, &length, &version))
				{
					if(gtaRwStreamRead(stream, &numTextures, 4))
					{
						for(int i = 0; i < numTextures; i++)
						{
							if(gtaRwStreamFindChunk(stream, rwID_TEXTURENATIVE, &length, &version) 
								&& gtaRwStreamFindChunk(stream, rwID_STRUCT, &length, &version))
							{
								if(gtaRwStreamRead(stream, &texData._TextureFormat, sizeof(NativeTexturePC::TextureFormat))
									&& gtaRwStreamRead(stream, &texData._RasterFormat, sizeof(NativeTexturePC::RasterFormat)))
								{
									strcpy(image.name, texData._TextureFormat.name);
									image.nameLength = strlen(image.name);
									image.width = texData._RasterFormat.width;
									image.height = texData._RasterFormat.height;
									image.format = (D3DFORMAT)texData._RasterFormat.d3dFormat;
									image.levels = texData._RasterFormat.numLevels;
									if(gtaRwStreamRead(stream, &size, 4))
									{
										image.stride = size / image.height;
										image.pixelsSize = image.stride * image.height;
										for(int j = 2; j <= image.levels; j++)
											image.pixelsSize += (image.stride * image.height) >> ((j - 1) * 2);
										image.pixels = new unsigned char[image.pixelsSize];
										if(gtaRwStreamRead(stream, image.pixels, size))
										{
											offset = size;
											for(int j = 2; j <= image.levels; j++)
											{
												if(!gtaRwStreamRead(stream, &size, 4) || !gtaRwStreamRead(stream, &image.pixels[offset], size))
													return false;
												offset += size;
											}
											// read extension
											if(gtaRwStreamFindChunk(stream, rwID_EXTENSION, &length, &version))
											{
												if(length > 0)
													gtaRwStreamSkip(stream, length);
											}
											if(txd.FindTexture(image.name))
											{
												LOG("   texture %s is already present in txd <temporary_txd>", image.name); 
												delete[] image.pixels;
											}
											else
												txd.images.push_back(image);
										}
									}
								}
							}
						}
						// end of txd
					}
				}
				gtaRwStreamClose(stream, NULL);
				// now open next txd
				stream = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, internalTxdPath);
				if(stream)
				{
					if(gtaRwStreamFindChunk(stream, rwID_TEXDICTIONARY, &length, &version) 
						&& gtaRwStreamFindChunk(stream, rwID_STRUCT, &length, &version))
					{
						if(gtaRwStreamRead(stream, &numTextures, 4))
						{
							for(int i = 0; i < numTextures; i++)
							{
								if(gtaRwStreamFindChunk(stream, rwID_TEXTURENATIVE, &length, &version) 
									&& gtaRwStreamFindChunk(stream, rwID_STRUCT, &length, &version))
								{
									if(gtaRwStreamRead(stream, &texData._TextureFormat, sizeof(NativeTexturePC::TextureFormat))
										&& gtaRwStreamRead(stream, &texData._RasterFormat, sizeof(NativeTexturePC::RasterFormat)))
									{
										strcpy(image.name, texData._TextureFormat.name);
										image.nameLength = strlen(image.name);
										image.width = texData._RasterFormat.width;
										image.height = texData._RasterFormat.height;
										image.format = (D3DFORMAT)texData._RasterFormat.d3dFormat;
										image.levels = texData._RasterFormat.numLevels;
										if(gtaRwStreamRead(stream, &size, 4))
										{
											image.stride = size / image.height;
											image.pixelsSize = image.stride * image.height;
											for(int j = 2; j <= image.levels; j++)
												image.pixelsSize += (image.stride * image.height) >> ((j - 1) * 2);
											image.pixels = new unsigned char[image.pixelsSize];
											if(gtaRwStreamRead(stream, image.pixels, size))
											{
												offset = size;
												for(int j = 2; j <= image.levels; j++)
												{
													if(!gtaRwStreamRead(stream, &size, 4) || !gtaRwStreamRead(stream, &image.pixels[offset], size))
														return false;
													offset += size;
												}
												// read extension
												if(gtaRwStreamFindChunk(stream, rwID_EXTENSION, &length, &version))
												{
													if(length > 0)
														gtaRwStreamSkip(stream, length);
												}
												if(txd.FindTexture(image.name))
												{
													LOG("   texture %s is already present in txd <temporary_txd>", image.name); 
													delete[] image.pixels;
												}
												else
													txd.images.push_back(image);
											}
										}
									}
								}
							}
							// end of txd
						}
					}
					gtaRwStreamClose(stream, NULL);
					DeleteFile(internalTxdPath);
					LOG("    Merged TXD %s", parentTxdPath);
					DeleteFile(parentTxdPath);
					txd.Build(parentTxdPath);
					LOG(" with TXD %s to TXD %s\n", internalTxdPath, parentTxdPath);
					return true;
				}
				LOG("     Can't open internal txd\n", txdName, modelName);
			}
			else
				LOG("     Can't find parent txd\n", txdName, modelName);
		}
		else
			LOG("     This model has no internal txd file txd - %s,model - %s\n", txdName, modelName);
		return false;
	}
	void ReadSA(char *filename)
	{
		char line[512];
		char temp[512];
		char temp2[512];

		// parameters
		char modelName[100];
		char txdName[100];
		char animName[100];
		float drawdist;
		unsigned int flags;
		float dummy[10];
		char lodName[100];

		unsigned int hash;

		Clear();

		LOG("Reading :: SA IDE :: %s\n", filename);

		strcpy(path, filename);
		strcpy(directory, filename);
		if (strrchr(directory, '\\')) strrchr(directory, '\\')[1] = '\0';
		FILE *file = fopen(filename, "rt");
		while (1)
		{
			if (feof(file))
				break;

			fgets(line, 512, file);

			if (line[0] == '#')
				continue;

			if (!strncmp(line, "objs", 4))
			{
				// read OBJS
				LOG(" :: OBJS\n", modelName);
				while (1)
				{
					if (feof(file))
						break;

					fgets(line, 512, file);

					if (line[0] == '#')
						continue;

					if (!strncmp(line, "end", 3))
						break;
					UINT id,mnHash,tnHash;
					sscanf(line, "%d, %X, %X", &id, &mnHash, &tnHash);
					sprintf(modelName, "%X\n", mnHash);
					sprintf(txdName, "%X\n", tnHash);
					strcpy(temp, modelName);
					memset(modelName, 0, 100);
					for (int i = 0; i < strlen(temp); i++)
					{
						if (temp[i] != ',')
							modelName[i] = temp[i];
					}

					strcpy(temp, txdName);
					memset(txdName, 0, 100);
					for (int i = 0; i < strlen(temp); i++)
					{
						if (temp[i] != ',')
							txdName[i] = temp[i];
					}
					objs.push_back(IdeObjectEntry(id, mnHash, tnHash, 300, 0, 1, 1));
				}
			}
		}
		fclose(file);
	}
	void ReadXTYP(char *filename)
	{
		char line[512];
		char temp[512];
		char temp2[512];

		// parameters
		char modelName[100];
		char txdName[100];
		char animName[100];
		unsigned int drawdist;
		unsigned int flags, flags2, nightFlag;
		float dummy[10];
		char lodName[100];

		unsigned int hash;

		Clear();

		LOG("Reading :: IDE :: %s\n", filename);

		strcpy(path, filename);
		strcpy(directory, filename);
		if (strrchr(directory, '\\')) strrchr(directory, '\\')[1] = '\0';
		CResource xtyp(filename);

		//FILE *file = fopen(filename, "rb");
		printf("%i",gDFFs.size());
		if (gDFFs.size() > 0){
			XTYPMAP_Header header;
			xtyp.ReadCpu(&header, sizeof(XTYPMAP_Header));
			header.SwapData();
			RSC_BlockPtr *SectionPtrs = new RSC_BlockPtr[header.SectionBlockCount];
			xtyp.ReadAtPointer(header.PtrToSectionPtrCollections.listOffset, SectionPtrs, sizeof(RSC_BlockPtr)*header.SectionBlockCount);
			for (int i = 0; i < header.SectionBlockCount; i++)
			{
				SectionPtrs[i].SwapData();
				if (SectionPtrs[i]._vmt == -2099839869){
					XTYP_Obj *objects = new XTYP_Obj[SectionPtrs[i].BlockByteSize/112];
					xtyp.ReadAtPointer(SectionPtrs[i].listOffset, objects, SectionPtrs[i].BlockByteSize);
					for (int k = 0; k < SectionPtrs[i].BlockByteSize / 112; k++)
					{
						objects[k].SwapData();
						sprintf(modelName, "%s", FindName(objects[k].modelHash));
						sprintf(txdName, "%s", FindName(objects[k].textuteHash));
						if (objects[k].textuteHash == 0)
						{
							if (InternalTxdToSimpleTxd(modelName))
								strcpy(txdName, modelName);
						}
						else
						{
							if (!MergeTxdWithInternal(txdName, modelName)){
								if (InternalTxdToSimpleTxd(modelName))
									strcpy(txdName, modelName);
							}
						}
						objs.push_back(IdeObjectEntry(modelName, txdName, objects[k].DrawDistance, 0, 0, ModelHasFile(modelName), TxdHasFile(txdName)));
					}
					DEL_ARRAY(objects);
				}
			}
			DEL_ARRAY(SectionPtrs);
		}
		/*if (gDFFs.size() > 0){
			for (int i = 0; i < gDFFs.size(); i++)
			{
				int currPos = xtyp.m_dwCpuCurrPos;
				while (xtyp.m_dwCpuCurrPos<xtyp.m_dwCpuDataSize-4)
				{
					unsigned int currHash;
					xtyp.ReadCpu(&currHash, 4);
					SwapEndian(currHash);
					if (gDFFs[i] == currHash)
					{
						int currPos0 = xtyp.m_dwCpuCurrPos;
						xtyp.m_dwCpuCurrPos -= 72;
						float ddist;
						xtyp.ReadCpu(&ddist, 4);
						SwapEndian(ddist);
						drawdist = ddist;
						//drawdist *= 3;
						/*if (drawdist > 300)
							drawdist = 300;
						xtyp.m_dwCpuCurrPos = currPos0;
						unsigned int texHash;
						xtyp.ReadCpu(&texHash, 4);
						SwapEndian(texHash);
						printf("\n%X,%X,%f\n", currHash, texHash, ddist);
						sprintf(modelName, "%X", currHash);
						sprintf(txdName, "%X", texHash);
						// check for internal txd
						//MergeTxdWithInternal(txdName, txdName);// check for hi-restex
						if (texHash==0)
						{
							if (InternalTxdToSimpleTxd(modelName))
								strcpy(txdName, modelName);
						}
						else
						{
							if (!MergeTxdWithInternal(txdName, modelName)){
								if (InternalTxdToSimpleTxd(modelName))
									strcpy(txdName, modelName);
							}
						}
						if (TxdHasFile(txdName) && drawdist>0 && drawdist<5000)
							objs.push_back(IdeObjectEntry(modelName, txdName, drawdist, 0, 0, 1, 1));
						//break;
					}
				}
				xtyp.m_dwCpuCurrPos = currPos;
			}
		}*/
		SetBaseId(gBaseId);
		gBaseId += GetObjectsCount();
		
		//fclose(file);
	}

	void Read(char *filename)
	{
		char line[512];
		char temp[512];
		char temp2[512];

		// parameters
		char modelName[100];
		char txdName[100];
		char animName[100];
		unsigned int drawdist;
		unsigned int flags, flags2, nightFlag;
		float dummy[10];
		char lodName[100];

		unsigned int hash;

		Clear();

		LOG("Reading :: IDE :: %s\n", filename);

		strcpy(path, filename);
		strcpy(directory, filename);
		if (strrchr(directory, '\\')) strrchr(directory, '\\')[1] = '\0';

		FILE *file = fopen(filename, "rt");
		while(1)
		{
			if(feof(file))
				break;

			fgets(line, 512, file);

			if(line[0] == '#')
				continue;

			if(!strncmp(line, "objs", 4))
			{
				// read OBJS
				LOG(" :: OBJS\n", modelName);
				while(1)
				{
					if(feof(file))
						break;

					fgets(line, 512, file);
					
					if(line[0] == '#')
						continue;

					if(!strncmp(line, "end", 3))
						break;

					if(sscanf(line, "%s %s %d %d %d", modelName, txdName, &drawdist, &flags, &flags2) != 5)
						flags2 = 0;

					// remove ','
					strcpy(temp, modelName);
					memset(modelName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							modelName[i] = temp[i];
					}

					strcpy(temp, txdName);
					memset(txdName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							txdName[i] = temp[i];
					}

					// check for internal txd
					if(!strcmp(txdName, "null"))
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						if(InternalTxdToSimpleTxd(modelName))
							strcpy(txdName, modelName);
					}
					else
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						MergeTxdWithInternal(txdName, modelName);
					}
					drawdist *= 3;
					if(drawdist > 300)
						drawdist = 300;
					if(ModelHasFile(modelName) && TxdHasFile(txdName))
						objs.push_back(IdeObjectEntry(modelName, txdName, drawdist, flags, flags2, 1, 1));
				}
			}
			else if(!strncmp(line, "tobj", 4))
			{
				// read TOBJ
				LOG(" :: TOBJ\n", modelName);
				while(1)
				{
					if(feof(file))
						break;

					fgets(line, 512, file);
					
					if(line[0] == '#')
						continue;

					if(!strncmp(line, "end", 3))
						break;

					sscanf(line, "%s %s %d %d %d %f %f %f %f %f %f %f %f %f %f %s %d", modelName, txdName, &drawdist, &flags, &flags2,
						&dummy[0], &dummy[1], &dummy[2], &dummy[3], &dummy[4], &dummy[5], &dummy[6], &dummy[7], &dummy[8], &dummy[9],
						lodName, &nightFlag);

					// remove ','
					strcpy(temp, modelName);
					memset(modelName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							modelName[i] = temp[i];
					}

					strcpy(temp, txdName);
					memset(txdName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							txdName[i] = temp[i];
					}

					// check for internal txd
					if(!strcmp(txdName, "null"))
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						if(InternalTxdToSimpleTxd(modelName))
							strcpy(txdName, modelName);
					}
					else
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						MergeTxdWithInternal(txdName, modelName);
					}
					drawdist *= 3;
					if(drawdist > 300)
						drawdist = 300;
					if(ModelHasFile(modelName) && TxdHasFile(txdName))
						tobj.push_back(IdeTimeObjectEntry(modelName, txdName, drawdist, flags, flags2, nightFlag, 1, 1));
				}
			}
			else if(!strncmp(line, "anim", 4))
			{
				// read ANIM
				LOG(" :: ANIM\n", modelName);
				while(1)
				{
					if(feof(file))
						break;

					fgets(line, 512, file);
					
					if(line[0] == '#')
						continue;

					if(!strncmp(line, "end", 3))
						break;

					sscanf(line, "%s %s %s %d %d %d", modelName, txdName, animName, &drawdist, &flags, &flags2);

					// remove ','
					strcpy(temp, modelName);
					memset(modelName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							modelName[i] = temp[i];
					}

					strcpy(temp, txdName);
					memset(txdName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							txdName[i] = temp[i];
					}

					strcpy(temp, animName);
					memset(animName, 0, 100);
					for(int i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							animName[i] = temp[i];
					}

					if(strlen(modelName) > 19)

					// check for internal txd
					if(!strcmp(txdName, "null"))
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						if(InternalTxdToSimpleTxd(modelName))
							strcpy(txdName, modelName);
					}
					else
					{
						hash = GetHashKey(modelName, 0);
						sprintf(modelName, "%X", hash);
						hash = GetHashKey(txdName, 0);
						sprintf(txdName, "%X", hash);
						MergeTxdWithInternal(txdName, modelName);
					}
					drawdist *= 3;
					if(drawdist > 300)
						drawdist = 300;
					if(ModelHasFile(modelName) && TxdHasFile(txdName))
						anim.push_back(IdeAnimObjectEntry(modelName, txdName, drawdist, flags, flags2, animName, 1, 1));
				}
			}
			else if(!strncmp(line, "txdp", 4))
			{
				// read TXDP
				LOG(" :: TXDP\n", modelName);
				while(1)
				{
					if(feof(file))
						break;

					fgets(line, 512, file);
					
					if(line[0] == '#')
						continue;

					if(!strncmp(line, "end", 3))
						break;

					sscanf(line, "%s", temp);
					// remove ','
					strcpy(temp2, temp);
					memset(modelName, 0, 100);
					for (UINT i = 0; i < strlen(temp2); i++)
					{
						if(temp2[i] == ',')
							temp[i] = ' ';
						else
							temp[i] = temp2[i];
					}

					sscanf(temp, "%s %s", modelName, txdName);

					hash = GetHashKey(modelName, 0);
					sprintf(modelName, "%X", hash);
					hash = GetHashKey(txdName, 0);
					sprintf(txdName, "%X", hash);
					if(TxdHasFile(modelName) && TxdHasFile(txdName))
						txdp.push_back(IdeTxdLink(modelName, txdName, 1, 1));
				}
			}
			else if(!strncmp(line, "2dfx", 4))
			{
				// read OBJS
				LOG(" :: 2DFX\n", modelName);
				while(1)
				{
					if(feof(file))
						break;

					fgets(line, 512, file);
					
					if(line[0] == '#')
						continue;

					if(!strncmp(line, "end", 3))
						break;

					Ide2dfx fx;
					fx.type = 0;
					unsigned int /*r, g, b, stype, fl,*/ _2dfxType;
					//char shadowName[32];

					strcpy(temp, line);
					memset(line, 0, 512);
					for (UINT i = 0; i < strlen(temp); i++)
					{
						if(temp[i] != ',')
							line[i] = temp[i];
						else
							line[i] = ' ';
					}

					sscanf_s(line, "%s %f %f %f %d", modelName, &fx.posn.x, &fx.posn.y, &fx.posn.z, &_2dfxType);

					
					switch(_2dfxType)
					{
					/*case 0:
						sscanf(line, 
							"%*s %*f %*f %*f %*d %*f %*f %*f %*f %*d %d %d %d %*d %*s %s %*f %*f %f %*f %*f %d %*f %*f %d %*f %*f %f %*f %*f %*f %f", 
							&r, &g, &b, shadowName, &fx.light.coronaSize, &stype, &fl, &fx.light.pointlightRange, &fx.light.shadowSize);

						//sprintf(temp, "rgb %d %d %d shadow %s stype %d fl %d range %f dist %f cor %f", 
						//	r, g, b, shadowName, stype, fl, fx.light.pointlightRange, fx.light.shadowSize, fx.light.coronaSize);
						//MessageBox(0, temp, 0, 0);

						if(!strncmp(shadowName, "\"\"", 2))
							fx.light.shadowSize = 0.0f;
						char hashedName[16];
						unsigned int hashed = GetHashKey(modelName, 0);
						sprintf(fx.modelName, "%X", hashed);
						fx.light.red = r;
						fx.light.green = g;
						fx.light.blue = b;
						fx.light.trigger = 0;
						// flags
						fx.light.flags = 0;
						if(fx.light.coronaSize == 0.0f)
							fx.light.flags |= 8; // WITHOUT_CORONA
						if(fl & 0x20)
							fx.light.flags |= 32; // AT_DAY
						if(fl & 0x40)
							fx.light.flags |= 64; // AT_NIGHT
						
						if(fl & 0x1)
							fx.light.trigger = 1;
						if(fl & 0x2)
							fx.light.trigger = 1;
						if(fl & 0x4)
							fx.light.trigger = 4;
						if(fl & 0x8)
							fx.light.trigger = 5;
						if(fl & 0x10)
							fx.light.trigger = 11;
						if(fl & 0x100)
							fx.light.flags |= 512;
						if(fl & 0x200)
							fx.light.trigger = 3;
						if(fl & 0x2000)
							fx.light.trigger = 4;
						if(fl & 0x4000)
							fx.light.trigger = 512;
						if(fl & 0x100000)
							fx.light.coronarefl = false;
						else
							fx.light.coronarefl = true;
						
						if(stype & 1)
						{
							fx.light.trigger = 11;
							fx.light.flags |= 128;
						}
						if(stype & 2)
						{
							fx.light.trigger = 11;
							fx.light.flags |= 128;
						}
						if(stype & 4)
							fx.light.trigger = 3;
						
						_2dfx.push_back(fx);
						break;*/
					}
				}
			}
		}
		SetBaseId(gBaseId);
		gBaseId += GetObjectsCount();
		fclose(file);
	}

	void Write()
	{
		//char temp[600];
		char line[512];
		char name[64];

		strcpy(&path[strlen(path) - 3], "ide");
		_splitpath(path, NULL, NULL, name, NULL);
		if (strrchr(path, '\\')) strrchr(path, '\\')[0] = '\0';
		if (strrchr(path, '\\')) strrchr(path, '\\')[1] = '\0';
		//strcat(path, "maps\\");
		strcat(path, name);
		strcat(path, ".ide");
		FILE *file = fopen(path, "wt");
		
		fputs("# This file was generated with IV2SAconv tool. <c> DK22Pac\n", file);

		// Flush all data to SA's IDE

		// Write OBJS
		fputs("objs\n", file);
		for (UINT i = 0; i < objs.size(); i++)
		{
			if(objs[i].modelExists && objs[i].txdExists)
				sprintf(line, "%d, %s, %s, %d, %d\n", baseId + i, objs[i].modelName, objs[i].txdName, objs[i].drawDist, 0);
			else
				sprintf(line, "#%d, %s, %s, %d, %d\n", baseId + i, objs[i].modelName, objs[i].txdName, objs[i].drawDist, 0);
			fputs(line, file);
		}
		fputs("end\n", file);

		/*// Write TOBJ
		fputs("tobj\n", file);
		for(int i = 0; i < tobj.size(); i++)
		{
			if(tobj[i].modelExists && tobj[i].txdExists)
				sprintf(line, "%d, %s, %s, %d, %d, %d, %d\n", baseId + i + objs.size(), tobj[i].modelName, tobj[i].txdName, tobj[i].drawDist, 0, 21, 7);
			else
				sprintf(line, "#%d, %s, %s, %d, %d, %d, %d\n", baseId + i + objs.size(), tobj[i].modelName, tobj[i].txdName, tobj[i].drawDist, 0, 21, 7);
			fputs(line, file);
		}
		fputs("end\n", file);

		// Don't write ANIM atm

		// Write 2DFX
		fputs("2dfx\n", file);
		for(int i = 0; i < _2dfx.size(); i++)
		{
			// if model exists
			int modelId = GetObjectByName(_2dfx[i].modelName);
			if(modelId != -1)
			{
				sprintf(line, "%d, %f, %f, %f, 0, %u, %u, %u, 200, \"coronastar\", \"shad_exp\", 300.0, %f, %f, %f, 128, %u, %d, 0, %u, 0, 0, 0, 100\n", 
					modelId, _2dfx[i].posn.x, _2dfx[i].posn.y, _2dfx[i].posn.z, _2dfx[i].light.red, _2dfx[i].light.green, _2dfx[i].light.blue,
					_2dfx[i].light.pointlightRange, _2dfx[i].light.coronaSize, _2dfx[i].light.shadowSize, _2dfx[i].light.trigger, _2dfx[i].light.coronarefl,
					_2dfx[i].light.flags);
				fputs(line, file);
			}
		}
		fputs("end\n", file);

		// Write TXDP
		fputs("txdp\n", file);
		for(int i = 0; i < txdp.size(); i++)
		{
			//if(TxdHasFile(txdp[i].name1) && TxdHasFile(txdp[i].name2))
				sprintf(line, "%s,%s\n", txdp[i].name1, txdp[i].name2);
			//else
			//	sprintf(line, "#%s,%s\n", txdp[i].name1, txdp[i].name2);
			fputs(line, file);
		}
		fputs("end", file);*/

		fclose(file);
	}
};

#include <list>

class IdeStore
{
public:

	static std::list<IdeFile> files;

	static void IdeFileReadCB(char *filename)
	{
		char name[100];
		_splitpath(filename, NULL, NULL, name, NULL);
		printf("\nReading %s...", name);
		IdeFile file;
		file.Read(filename);
		files.push_back(file);
	}
	static void SAIdeFileReadCB(char *filename)
	{
		char name[100];
		_splitpath(filename, NULL, NULL, name, NULL);
		printf("\nReading %s...", name);
		IdeFile file;
		file.ReadSA(filename);
		files.push_back(file);
	}
	static void XtypFileReadCB(char *filename)
	{
		char name[100];
		_splitpath(filename, NULL, NULL, name, NULL);
		printf("\nReading %s...", name);
		IdeFile file;
		file.ReadXTYP(filename);
		files.push_back(file);
	}
	
	static void FlushFiles()
	{
		for (auto i = files.begin(); i != files.end(); i++){
			if ((*i).objs.size() > 0)
				(*i).Write();
		}
	}
};

std::list<IdeFile> IdeStore::files;

struct WplHeaderStreamData
{
	unsigned int version;
	unsigned int numInstances;
	unsigned int unk1;
	unsigned int numGarages;
	unsigned int numCars;
};

struct WplInstanceStreamData
{
	Vector3 position;
	Vector4 rotation;
	unsigned int modelHash;
	unsigned int flags;
	int lodIndex;
	unsigned int unk[2];
};

class WplInstance
{
public:
	Vector3 position;
	Vector4 rotation;
	unsigned int flags;
	char modelName[20];
	char originalName[100];

	// SA thing
	unsigned int ideNumber;
	int lodID;

	WplInstance(Vector3 _position, Vector4 _rotation, unsigned int _flags, char *name, int modelId)
	{
		position = _position;
		rotation = _rotation;
		flags = _flags;
		strcpy(modelName, name);
		ideNumber = modelId;
		lodID = -1;
	}
	WplInstance(Vector3 _position, Vector4 _rotation, unsigned int _flags, char *name, int modelId, int LodID)
	{
		position = _position;
		rotation = _rotation;
		flags = _flags;
		strcpy(modelName, name);
		ideNumber = modelId;
		lodID = LodID;
	}
};

class WplFile
{
public:
	char path[512];
	char directory[512];
	char name[64];
	std::vector<WplInstance> instances;

	void Clear()
	{
		instances.clear();
	}

	int FindModelByHash(unsigned int hash, char *outName)
	{
		sprintf(outName, "%s", FindName(hash));
		for(auto i = IdeStore::files.begin(); i != IdeStore::files.end(); i++)
		{
			for (UINT j = 0; j < (*i).objs.size(); j++)
			{
				if ((*i).objs[j].id == -1){
					if ((*i).objs[j].modelExists && (*i).objs[j].txdExists && !strcmp((*i).objs[j].modelName, outName))
					{
						strcpy(outName, (*i).objs[j].modelName);
						return (*i).baseId + j;
					}
				}
				else
				{
					if ((*i).objs[j].modelHash == hash)
					{
						//strcpy(outName, (*i).objs[j].modelName);
						return (*i).objs[j].id;
					}
				}
			}
			for (UINT j = 0; j < (*i).tobj.size(); j++)
			{
				if((*i).tobj[j].modelExists && (*i).tobj[j].txdExists && !strcmp((*i).tobj[j].modelName, outName))
				{
					strcpy(outName, (*i).tobj[j].modelName);
					return (*i).baseId + (*i).objs.size() + j;
				}
			}
		}
		return -1;
	}

	void ReadXMAP(char *filename)
	{
		//char line[512];
		char modelName[512];
		//unsigned int id;

		WplInstanceStreamData inst;
		//WplHeaderStreamData header;

		// Clear();

		LOG("Reading :: WPL :: %s\n", filename);

		strcpy(path, filename);
		strcpy(directory, filename);
		if (strrchr(directory, '\\')) strrchr(directory, '\\')[1] = '\0';

		CResource xmap(filename);
		XTYPMAP_Header header;
		xmap.ReadCpu(&header, sizeof(XTYPMAP_Header));
		header.SwapData();
		RSC_BlockPtr *SectionPtrs = new RSC_BlockPtr[header.SectionBlockCount];
		xmap.ReadAtPointer(header.PtrToSectionPtrCollections.listOffset, SectionPtrs, sizeof(RSC_BlockPtr)*header.SectionBlockCount);
		for (int i = 0; i < header.SectionBlockCount; i++)
		{
			SectionPtrs[i].SwapData();
			if (SectionPtrs[i]._vmt == -833612669)
			{
				XMAP_Inst *objects = new XMAP_Inst[SectionPtrs[i].BlockByteSize / 112];
				xmap.ReadAtPointer(SectionPtrs[i].listOffset, objects, SectionPtrs[i].BlockByteSize);
				for (int k = 0; k < SectionPtrs[i].BlockByteSize / 112; k++)
				{
					objects[k].SwapData();
					int id = FindModelByHash(objects[k].modelHash, modelName);
					Vector3 pos = Vector3(objects[k].pos.x, objects[k].pos.y, objects[k].pos.z);
					instances.push_back(WplInstance(pos, objects[k].rot, 0, modelName, id, objects[k].lodID));
				}
				DEL_ARRAY(objects);
			}
			else if (SectionPtrs[i]._vmt == -749125722)
			{
				RSC_POINTER ptrToHashes = SectionPtrs[i].listOffset;
				ptrToHashes.offset += 8;
				unsigned int hash;
				xmap.ReadAtPointer(ptrToHashes, &hash, 4);
				SwapEndian(hash);
				LOG("%s have parent - %s\n", filename, FindName2(hash));
			}
		}
		DEL_ARRAY(SectionPtrs);
		/*int currPos = xmap.m_dwCpuCurrPos;
		while (xmap.m_dwCpuCurrPos<xmap.m_dwCpuDataSize - 4)
		{
			unsigned int currHash;
			xmap.ReadCpu(&currHash, 4);
			SwapEndian(currHash);
			int id = FindModelByHash(currHash, modelName);
			if (id != -1)
			{
				Vector3 pos;
				Vector4 rot;
				xmap.m_dwCpuCurrPos += 8;
				xmap.ReadCpu(&pos.x, 4);
				SwapEndian(pos.x);
				xmap.ReadCpu(&pos.y, 4);
				SwapEndian(pos.y);
				xmap.ReadCpu(&pos.z, 4);
				SwapEndian(pos.z);
				xmap.m_dwCpuCurrPos += 4;
				xmap.ReadCpu(&rot.x, 4);
				SwapEndian(rot.x);
				xmap.ReadCpu(&rot.y, 4);
				SwapEndian(rot.y);
				xmap.ReadCpu(&rot.z, 4);
				SwapEndian(rot.z);
				xmap.ReadCpu(&rot.w, 4);
				SwapEndian(rot.w);
				if (pos.x<20000 || pos.x > -20000||
					pos.y<20000 || pos.y > -20000||
					pos.x<20000 || pos.x > -20000)
					instances.push_back(WplInstance(pos, rot, 0, modelName, id));
			}
		}
		xmap.m_dwCpuCurrPos = currPos;*/
	}

	void Read(char *filename)
	{
		//char line[512];
		char modelName[512];
		unsigned int id;

		WplInstanceStreamData inst;
		WplHeaderStreamData header;

		// Clear();

		LOG("Reading :: WPL :: %s\n", filename);

		strcpy(path, filename);
		strcpy(directory, filename);
		if (strrchr(directory, '\\')) strrchr(directory, '\\')[1] = '\0';

		FILE *file = fopen(filename, "rb");
		fread(&header, sizeof(WplHeaderStreamData), 1, file);
		fseek(file, 68, SEEK_SET);
		for (UINT i = 0; i < header.numInstances; i++)
		{
			fread(&inst, 48, 1, file);
			id = FindModelByHash(inst.modelHash, modelName);
			if(id == -1)
			{
				LOG("  Can't find model %s\n", modelName);
			}
			instances.push_back(WplInstance(inst.position, inst.rotation, inst.flags, modelName, id));
		}
		fclose(file);
	}
	struct BnryIPL{
		char header[4];
		int instCount;
		int unkCount0;
		int unkCount1;
		int unkCount2;
		int unkCount3;
		int unkCount4;
		int instOffset;
		int instSize;
		int unkOffset0;
		int unkSize0;
		int unkOffset1;
		int unkSize1;
		int unkOffset2;
		int unkSize2;
		int unkOffset3;
		int unkSize3;
		int unkOffset4;
		int unkSize4;
	};
	void Write()
	{
		//char temp[600];
		char line[512];
		char bnrypath[512];
		if (strrchr(path, '\\')) strrchr(path, '\\')[0] = '\0';
		if (strrchr(path, '\\')) strrchr(path, '\\')[1] = '\0';
		//strcat(path, "maps\\");
		strcat(path, name);
		strcpy(bnrypath, path);
		strcat(bnrypath, "_bnry.ipl");
		strcat(path, ".ipl");
		FILE *filebnry = fopen(bnrypath, "wb");
		FILE *file = fopen(path, "wt");
		BnryIPL iplHeader;
		//fputs("# This file was generated with IV2SAconv tool. <c> DK22Pac\n", file);
		//memset(&iplHeader, NULL, sizeof(BnryIPL));
		strcpy(iplHeader.header,"bnry");
		iplHeader.instCount = instances.size();
		iplHeader.instOffset = sizeof(BnryIPL);
		// Flush all data to SA's IPL
		fwrite(&iplHeader, sizeof(BnryIPL), 1, filebnry);
		// Write instances
		fputs("inst\n", file);
		for (UINT i = 0; i < instances.size(); i++)
		{
			if(instances[i].ideNumber != -1)
			{
				sprintf(line, "%d, %s, %d, %f, %f, %f, %f, %f, %f, %f, %d\n", instances[i].ideNumber, instances[i].modelName,
					0, instances[i].position.x + MapMove.x, instances[i].position.y + MapMove.y, instances[i].position.z + MapMove.z, instances[i].rotation.x,
					instances[i].rotation.y, instances[i].rotation.z, instances[i].rotation.w, -1);
				fwrite(&instances[i].position.x, sizeof(float), 1, filebnry);
				fwrite(&instances[i].position.y, sizeof(float), 1, filebnry);
				fwrite(&instances[i].position.z, sizeof(float), 1, filebnry);
				fwrite(&instances[i].rotation.x, sizeof(float), 1, filebnry);
				fwrite(&instances[i].rotation.y, sizeof(float), 1, filebnry);
				fwrite(&instances[i].rotation.z, sizeof(float), 1, filebnry);
				fwrite(&instances[i].rotation.w, sizeof(float), 1, filebnry);
				fwrite(&instances[i].ideNumber, sizeof(int), 1, filebnry);
				int nul = 0;
				int invone = -1;
				fwrite(&nul, sizeof(int), 1, filebnry);
				fwrite(&invone, sizeof(int), 1, filebnry);
				//fputs(line, file);
			}
			else
			{
				sprintf(line, "#%d\n#%d, %s, %d, %f, %f, %f, %f, %f, %f, %f, %d\n", instances[i].ideNumber, instances[i].modelName,
					0, instances[i].position.x + MapMove.x, instances[i].position.y + MapMove.y, instances[i].position.z + MapMove.z, instances[i].rotation.x,
					instances[i].rotation.y, instances[i].rotation.z, instances[i].rotation.w, -1);
			}
			fputs(line, file);
		}
		fputs("end", file);

		fclose(file);
		fclose(filebnry);
	}
};

class WplStore
{
	public:

	static std::list<WplFile> files;

	static void WplFileReadCB(char *filename)
	{
		WplFile file;
		// filename to name
		char name[64];
		_splitpath(filename, NULL, NULL, name, NULL);
		if(strstr(name, "_stream"))
			strstr(name, "_stream")[0] = '\0';
		if(strstr(name, "_strbig"))
			strstr(name, "_strbig")[0] = '\0';
		WplFile *pFile = GetFileByName(name);
		if(pFile)
		{
			pFile->Read(filename);
		}
		else
		{
			strcpy(file.name, name);
			file.Read(filename);
			files.push_back(file);
		}
	}
	static void XMapFileReadCB(char *filename)
	{
		WplFile file;
		// filename to name
		char name[64];
		_splitpath(filename, NULL, NULL, name, NULL);
		if (strstr(name, "_stream"))
			strstr(name, "_stream")[0] = '\0';
		if (strstr(name, "_strbig"))
			strstr(name, "_strbig")[0] = '\0';
		WplFile *pFile = GetFileByName(name);
		if (pFile)
		{
			pFile->ReadXMAP(filename);
		}
		else
		{
			strcpy(file.name, name);
			file.ReadXMAP(filename);
			files.push_back(file);
		}
	}
	static void FlushFiles()
	{
		for (auto i = files.begin(); i != files.end(); i++){
			if ((*i).instances.size()>0)
				(*i).Write();
		}
	}

	static WplFile *GetFileByName(char *name)
	{
		for(auto i = files.begin(); i != files.end(); i++)
		{
			if(!strcmp((*i).name, name))
				return &(*i);
		}
		return NULL;
	}
};

std::list<WplFile> WplStore::files;