// main.cpp : Defines the entry point for the console application.
#define DEL_ARRAY(array) if (array){/*return;*/delete[] array;array = NULL;}
#define DEL(array) if (array){/*return;*/delete array;array = NULL; }
#define CRNLIB_PIXEL_FMT_FOURCC(a, b, c, d) ((a) | ((b) << 8U) | ((c) << 16U) | ((d) << 24U))
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d10.h>
#include <d3dcompiler.h>

enum eGameVersion
{
	GAME_SA,
	GAME_3,
	GAME_VC
};
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
#include "WDRConverter.h"
IDirect3D9* pD3D9;
IDirect3DDevice9* pD3D9Dev;
struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};
typedef struct {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;


bool GetPixelDataFromDDS(char* filename, CImage *image){
	char name[128];
	_splitpath(filename, NULL, NULL, name, NULL);
	strcpy(image->name, name);
	image->nameLength = strlen(image->name);
	printf("%s",filename);
	IDirect3DTexture9* tex;
	D3DXCreateTextureFromFile(pD3D9Dev, filename, &tex);
	tex->GenerateMipSubLevels();
	D3DXSaveTextureToFile(filename, D3DXIFF_DDS, tex, NULL);
	tex->Release();
	tex = NULL;
	FILE* dds = fopen(filename, "rb");
	DWORD magic;
	DDS_HEADER hdr;
	fread(&magic, 4, 1, dds);
	if (magic == 0x20534444){
		fread(&hdr, sizeof(DDS_HEADER), 1, dds);
		image->format = (D3DFORMAT)hdr.ddspf.dwFourCC;
		if (image->format == D3DFMT_DXT1 || image->format == D3DFMT_DXT5 || image->format == D3DFMT_DXT2 || image->format == D3DFMT_DXT3 || image->format == D3DFMT_DXT4 || image->format == D3DFMT_A8R8G8B8)
			;
		else
		{
			fclose(dds);
			return false;
		}
			
		//printf("%i", image->format);
		image->height = hdr.dwHeight;
		image->width = hdr.dwWidth;
		image->levels = hdr.dwMipMapCount+1;
		int blockSize = (image->format == D3DFMT_DXT1) ? 8 : 16;
		int pitch = max(1, ((image->width + 3) / 4))*blockSize;
		if (image->format == D3DFMT_DXT1)
			image->stride = image->width/2;
		else if (image->format == D3DFMT_A8R8G8B8)
			image->stride = image->width* 4;
		else
			image->stride = image->width;
		if (image->format == D3DFMT_DXT1)
			image->pixelsSize = (image->width*image->height) / 2;
		else if (image->format == D3DFMT_A8R8G8B8)
			image->pixelsSize = image->width*image->height*4;
		else
			image->pixelsSize = image->width*image->height;
		for (UINT j = 2; j <= image->levels; j++)
			image->pixelsSize += (image->stride * image->height) >> ((j - 1) * 2);
		//printf("\n%i\n", image->pixelsSize);
		image->pixels = new unsigned char[image->pixelsSize];
		fseek(dds, 128, 0);
		fread(image->pixels, 1, image->pixelsSize, dds);
		
	}
	else
	{
		printf("wrong dds file: %s", filename);
		fclose(dds);
		return false;
	}
	fclose(dds);
	return true;
}

std::vector<CImage> g_TmpImages;
void ConvertToImage(char* filename){
	CImage img;
	if (GetPixelDataFromDDS(filename, &img))
		g_TmpImages.push_back(img);
}


unsigned int gModelDetailLevel;

void ConvertToTxd(char *fileName);
void ConvertToDff(char *fileName);
void ConvertWftToDff(char *fileName);


unsigned int gBaseId;
Vector3 MapMove;
int gbUseColMaterials = 1;

char gOutFolderName[64];
void DeleteCB(char *filename)
{
	DeleteFile(filename);
}

FILE *filelist;

void DumpFileIde(char *path)
{
	char filename[64];
	_splitpath(path, NULL, NULL, filename, NULL);
	fprintf(filelist, "IDE LEVELS\\GTA5\\%s\\%s.IDE\n", gOutFolderName, filename);
}

void DumpFileImg(char *path)
{
	char filename[64];
	_splitpath(path, NULL, NULL, filename, NULL);
	fprintf(filelist, "IMG LEVELS\\GTA5\\%s\\%s.IMG\n", gOutFolderName, filename);
}

void DumpFileIpl(char *path)
{
	char filename[64];
	_splitpath(path, NULL, NULL, filename, NULL);
	fprintf(filelist, "IPL LEVELS\\GTA5\\%s\\%s.IPL\n", gOutFolderName, filename);
}

void DisasmShader(char *path){
	char filename[64];
	_splitpath(path, NULL, NULL, filename, NULL);
	FILE* pShader = fopen(path,"rb");
	byte* pShaderCode;
	fseek(pShader, 0, SEEK_END);
	unsigned int fsize = ftell(pShader);
	fseek(pShader, 0, SEEK_SET);
	fread(&pShaderCode, 1, fsize, pShader);
	fclose(pShader);
	ID3DBlob* pDisassembly = 0;
	D3DDisassemble(&pShaderCode, fsize,
		D3D_DISASM_ENABLE_COLOR_CODE, 0, &pDisassembly);

	if (pDisassembly)
	{
		char pDisassemblyPath[512];
		strcpy(pDisassemblyPath, filename);
		strcat(pDisassemblyPath, "_disassembly.html");
		FILE* pFile = 0;
		if (fopen_s(&pFile, pDisassemblyPath, "w") == 0)
		{
			fwrite(pDisassembly->GetBufferPointer(), pDisassembly->GetBufferSize(), 1, pFile);
			fclose(pFile);
		}

		pDisassembly->Release();
	}
}

bool gDeleteIVfilesAfterConverting;
FILE *dbgFile;
bool gbConvertTextures;
bool gbConvertModels;
bool gbConvertPlacements;
bool gbConvertXTYPs;
D3DPRESENT_PARAMETERS d3dpp;

void InitD3D9()
{
	// Init Present Params.
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferWidth = 8;
	d3dpp.BackBufferHeight = 8;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	// Init D3D9 and Device.
	HWND hWnd = GetForegroundWindow();
	pD3D9 = Direct3DCreate9(DIRECT3D_VERSION);
	pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pD3D9Dev);
}

void GetGameVersion(){
	char temp[64];
	GetPrivateProfileString("GAME", "Target", "GTASA", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "GTA3"))
	{
		gameVersion = GAME_3;
		chunkVersion = 0x33002;
	}
	else if (!strcmp(temp, "GTAVC"))
	{
		gameVersion = GAME_VC;
		chunkVersion = 0x34003;
	}
	else
	{
		gameVersion = GAME_SA;
		chunkVersion = 0x36003;
	}
	LOG("Building option: game %s\n", temp);
}

void InitTexModelSettings(){
	char temp[64];
	GetPrivateProfileString("TEXTURES PROPERTIES", "FilterMode", "rwFILTERLINEAR", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "rwFILTERNEAREST"))
		filterMode = rwFILTERNEAREST;
	else
		filterMode = rwFILTERLINEAR;
	GetPrivateProfileString("TEXTURES PROPERTIES", "FilterMipMode", "rwFILTERLINEARMIPLINEAR", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "rwFILTERMIPNEAREST"))
		filterMipMode = rwFILTERMIPNEAREST;
	else if (!strcmp(temp, "rwFILTERMIPLINEAR"))
		filterMipMode = rwFILTERMIPLINEAR;
	else if (!strcmp(temp, "rwFILTERLINEARMIPNEAREST"))
		filterMipMode = rwFILTERLINEARMIPNEAREST;
	else
		filterMipMode = rwFILTERLINEARMIPLINEAR;
	GetPrivateProfileString("TEXTURES PROPERTIES", "Addressing.U", "rwTEXTUREADDRESSWRAP", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "rwTEXTUREADDRESSBORDER"))
		uAddressing = rwTEXTUREADDRESSBORDER;
	else if (!strcmp(temp, "rwTEXTUREADDRESSMIRROR"))
		uAddressing = rwTEXTUREADDRESSMIRROR;
	else if (!strcmp(temp, "rwTEXTUREADDRESSCLAMP"))
		uAddressing = rwTEXTUREADDRESSCLAMP;
	else
		uAddressing = rwTEXTUREADDRESSWRAP;
	GetPrivateProfileString("TEXTURES PROPERTIES", "Addressing.V", "rwTEXTUREADDRESSWRAP", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "rwTEXTUREADDRESSBORDER"))
		vAddressing = rwTEXTUREADDRESSBORDER;
	else if (!strcmp(temp, "rwTEXTUREADDRESSMIRROR"))
		vAddressing = rwTEXTUREADDRESSMIRROR;
	else if (!strcmp(temp, "rwTEXTUREADDRESSCLAMP"))
		vAddressing = rwTEXTUREADDRESSCLAMP;
	else
		vAddressing = rwTEXTUREADDRESSWRAP;
	anysotropy = GetPrivateProfileInt("TEXTURES PROPERTIES", "Anisotropy", 0, ".\\_WTD2TXD_SETTINGS.INI");
	GetPrivateProfileString("SKIPPING OPTIONS", "SkipNormalMapTextures", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		skipNormals = TRUE;
	else
		skipNormals = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "SkipBumpMapTextures", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		skipBump = TRUE;
	else
		skipBump = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "SkipSpecularMapTextures", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		skipSpec = TRUE;
	else
		skipSpec = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "ConvertTextures", "TRUE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		gbConvertTextures = TRUE;
	else
		gbConvertTextures = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "ConvertModels", "TRUE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		gbConvertModels = TRUE;
	else
		gbConvertModels = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "ConvertXTYPs", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		gbConvertXTYPs = TRUE;
	else
		gbConvertXTYPs = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "ConvertPlacements", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		gbConvertPlacements = TRUE;
	else
		gbConvertPlacements = FALSE;
	GetPrivateProfileString("SKIPPING OPTIONS", "ConvertColMats", "FALSE", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	if (!strcmp(temp, "TRUE"))
		gbUseColMaterials = TRUE;
	else
		gbUseColMaterials = FALSE;
}

int main()
{
	char temp[64];

	SetConsoleTitle("GTAIV2SA (.WDR.WTD.IDE > .DFF.TXD.COL.IDE) gen by DK22Pac");

	printf("Conversion started...\n");

	dbgFile = fopen("_IV2SACONV_LOG.txt", "wt");
	// Init stuff.
	InitD3D9();
	OptimiseInit();
	GetGameVersion();
	InitTexModelSettings();
	gBaseId = GetPrivateProfileInt("GENRL", "gBaseId", 6000, ".\\_IDE_SETTINGS.INI");
	GetPrivateProfileString("GENRL", "gMapMoveX", "0.0", temp, 64,".\\_WTD2TXD_SETTINGS.INI");
	MapMove.x = atof(temp);
	GetPrivateProfileString("GENRL", "gMapMoveY", "0.0", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	MapMove.y = atof(temp);
	GetPrivateProfileString("GENRL", "gMapMoveZ", "0.0", temp, 64, ".\\_WTD2TXD_SETTINGS.INI");
	MapMove.z = atof(temp);
	GetPrivateProfileString("GENRL", "gOutFolderName", "hollywood", gOutFolderName, 64, ".\\_IDE_SETTINGS.INI");
	gModelDetailLevel = GetPrivateProfileInt("GENRL", "gModelDetailLevel", 0, ".\\_WFT_SETTINGS.INI");
	if(gModelDetailLevel > 3)
		gModelDetailLevel = 0;
	gDeleteIVfilesAfterConverting = (bool)GetPrivateProfileInt("GENRL", "gDeleteIVfilesAfterConverting", 0, ".\\_GENERAL_SETTINGS.INI");


	if (gbConvertTextures)
		SearchFiles("*.txd", (LPSEARCHFUNC)DeleteCB);
	if (gbConvertModels){
		SearchFiles("*.dff", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.col", (LPSEARCHFUNC)DeleteCB);
	}
	SearchFiles("*.matlist", (LPSEARCHFUNC)DeleteCB);
	if (gbConvertPlacements){
		if (gbConvertXTYPs)
			SearchFiles("*.ide", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.ipl", (LPSEARCHFUNC)DeleteCB);
	}
	SearchFiles("*.xdr", (LPSEARCHFUNC)AddToHashNames);
	SearchFiles("*.xtd", (LPSEARCHFUNC)AddToHashNames);
	SearchFiles("*.xmap", (LPSEARCHFUNC)AddToHashNames);
	SearchFiles("*.xtyp", (LPSEARCHFUNC)AddToHashNames);
	if (gbConvertModels){
		SearchFiles("*.xdr", (LPSEARCHFUNC)XDrawableConvertToDFF);
		SearchFiles("*.xdd", (LPSEARCHFUNC)XDrawableDictionaryConvertToDFF);
	}
	if (gbConvertTextures)
		SearchFiles("*.xtd", (LPSEARCHFUNC)ConvertToTxd);
	if (gbConvertPlacements){
		if (gbConvertXTYPs){
			printf("\nScanning XTYP...");
			SearchFiles("*.xtyp", (LPSEARCHFUNC)IdeStore::XtypFileReadCB);
			SearchFiles("*.xmap", (LPSEARCHFUNC)IdeStore::XtypFileReadCB);
		}
		else
		{
			printf("\nScanning IDE...");
			SearchFiles("*.ide", (LPSEARCHFUNC)IdeStore::SAIdeFileReadCB);
		}
		printf("\nScanning XMAP...");
		SearchFiles("*.xmap", (LPSEARCHFUNC)WplStore::XMapFileReadCB);
		SearchFiles("*.xtyp", (LPSEARCHFUNC)WplStore::XMapFileReadCB);
		if (gbConvertXTYPs){
			printf("\nWriting IDE...");
			IdeStore::FlushFiles();
		}
		printf("\nWriting IPL...");
		WplStore::FlushFiles();
	}
	filelist = fopen("filelist.txt", "wt");
	fputs("# IMGs\n\n", filelist);
	SearchFiles("*.img", (LPSEARCHFUNC)DumpFileImg);
	if (gbConvertXTYPs)
	{
		fputs("\n# IDEs\n\n", filelist);
		SearchFiles("*.ide", (LPSEARCHFUNC)DumpFileIde);
	}
	fputs("\n# IPLs\n\n", filelist);
	SearchFiles("*.ipl", (LPSEARCHFUNC)DumpFileIpl);
	fclose(filelist);
	
	if(gDeleteIVfilesAfterConverting)
	{
		printf("\nDeleting WDR, WTD, WPL, WDE, MATLIST files...");
		SearchFiles("*.wtd", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.wdr", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.wft", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.wde", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.wpl", (LPSEARCHFUNC)DeleteCB);
		SearchFiles("*.matlist", (LPSEARCHFUNC)DeleteCB);
	}
	printf("\nConversion finished.");

	// paths - temporary
	//SearchFiles("*.dat", (LPSEARCHFUNC)CPathReaderWriter::DumpSANodeFile);

	fclose(dbgFile);
	getch();
	return 0;
}

#include "ModelReader.h"
#include "CClump.h"

#define TEXTURESAMPLER_HASH 726757629
#define TEXTURESAMPLERLAYER0_HASH 3576369631
#define PARAM_DIFFUSE_SAMPLER 0

char *GetVertexElementType(unsigned char type)
{
	switch (type)
	{
	case ELEMENTTYPE_FLOAT16:
		return "FLOAT16";
	case ELEMENTTYPE_FLOAT16_2:
		return "FLOAT16_2";
	case ELEMENTTYPE_FLOAT16_3:
		return "FLOAT16_3";
	case ELEMENTTYPE_FLOAT16_4:
		return "FLOAT16_4";
	case ELEMENTTYPE_FLOAT1:
		return "FLOAT1";
	case ELEMENTTYPE_FLOAT2:
		return "FLOAT2";
	case ELEMENTTYPE_FLOAT3:
		return "FLOAT3";
	case ELEMENTTYPE_FLOAT4:
		return "FLOAT4";
	case ELEMENTTYPE_UBYTE4:
		return "UBYTE4";
	case ELEMENTTYPE_D3DCOLOR:
		return "D3DCOLOR";
	case ELEMENTTYPE_DEC3N:
		return "DEC3N";
	}
	return "UNKNOWN";
}

void ConvertToDff(char *fileName)
{
	printf("\nMODEL %s\n", fileName);

	char nameHash[20];
	char temp[512];
	char name[512];
	//char type[64];
	char modelName[100];
	char colName[512];
	_splitpath(fileName, NULL, NULL, modelName, NULL);
	strcpy(name, fileName);
	strcpy(colName, fileName);
	//strcpy(colName, fileName);
	if (strrchr(name, '\\')) strrchr(name, '\\')[1] = '\0';
	unsigned int hash = GetHashKey(modelName, 0);
	//LOG("\n %s = %X\n", modelName, hash);
	gDFFs.push_back(hash);
	sprintf(nameHash, "%X", hash);
	strcat(name, nameHash);
	strcat(name, ".xdr");
	//FILE *f = fopen(name, "wt");
	unsigned int len = strlen(name);

	CResource wdrFile(fileName);
	printf("wdr opend\n");
	RSC_DRAWABLE drawable;
	wdrFile.ReadCpu(&drawable, sizeof(RSC_DRAWABLE));
	drawable.SwapData();

	//FileLOG(f, "Dimensions min %f %f %f max %f %f %f center %f %f %f\n", drawable.aabbMin.x, drawable.aabbMin.y, drawable.aabbMin.z, 
	//	drawable.aabbMax.x, drawable.aabbMax.y, drawable.aabbMax.z, drawable.center.x, drawable.center.y, drawable.center.z);
	RSC_COLLECTION modelsCollection;
	wdrFile.ReadAtPointer(drawable.pModelCollection[0], &modelsCollection, sizeof(RSC_COLLECTION));
	modelsCollection.SwapData();
	if (modelsCollection.count > 0)
	{
	//FileLOG(f, "Models count: %d\n", modelsCollection.count);
	RSC_POINTER * pointers;
	RSC_POINTER * geometryPointers;
	RSC_GEOMETRY * geometries;
	RSC_MODEL * models;
	RSC_INDEXBUFFER iBuffer;
	RSC_VERTEXBUFFER vBuffer;
	CFrame frame;
	CClump clump;
	CModelInstance model;
	CMaterial material;
	CRageGeometry geometry;
	RSC_SHADERGOUP shaderGroup;
	RSC_POINTER * shadersPtrs;
	RSC_SHADERPARAM *shaderParams;
	RSC_SHADER shader;
	RSC_TEXTURE_V tex;
	//RSC_TEXTURE_V *texRsc;
	//RSC_TXD txdRsc;
	//RSC_POINTER *texturesPtrs;
	//CTexDictionary txd;
	//CImage image;
	//unsigned int dataSize;
	unsigned short *shaderMappings;
	printf("models count:%i\n", modelsCollection.count);
	
		pointers = new RSC_POINTER[modelsCollection.count];
		models = new RSC_MODEL[modelsCollection.count];
		wdrFile.ReadAtPointer(modelsCollection.listOffset, pointers, 4 * modelsCollection.count);
		for (int j = 0; j < modelsCollection.count; j++)
			SwapEndian(pointers[j]);
		// for all frames
		for (int i = 0; i < modelsCollection.count; i++)
		{
			wdrFile.ReadAtPointer(pointers[i], &models[i], sizeof(RSC_MODEL));
			models[i].SwapData();
			frame.parentFrame = i == 0 ? -1 : 0;
			frame.matrixFlags = 0;
			// set the frame name
			strcpy(frame.name, modelName);
			if (i != 0)
			{
				sprintf(temp, "%d", i + 1);
				strcat(frame.name, temp);
			}
			//FileLOG(f, "Frame %s", frame.name);
			clump.frames.push_back(frame);
			//FileLOG(f, " (%d geometries)\n", models[i].geometriesCount);
			if (models[i].geometriesCount > 0)
			{
				geometryPointers = new RSC_POINTER[models[i].geometriesCount];
				geometries = new RSC_GEOMETRY[models[i].geometriesCount];
				wdrFile.ReadAtPointer(models[i].geometriesList, geometryPointers, 4 * models[i].geometriesCount);
				for (int j = 0; j < models[i].geometriesCount; j++)
					SwapEndian(geometryPointers[j]);
				model.bbox.aabbMax = *(Vector3 *)&drawable.aabbMax;
				model.bbox.aabbMin = *(Vector3 *)&drawable.aabbMin;
				model.bbox.ToSphere(&model.bsphere);
				clump.frames[i].atomics.push_back(model);

				shaderMappings = new unsigned short[models[i].shaderMappingsCount];
				wdrFile.ReadAtPointer(models[i].shaderMappings, shaderMappings, 2 * models[i].shaderMappingsCount);
				for (int j = 0; j < models[i].shaderMappingsCount; j++)
					SwapEndian(shaderMappings[j]);
				//FileLOG(f, " NumShaderMappings %d\n", models[i].shaderMappingsCount);
				// for all meshes
				for (int j = 0; j < models[i].geometriesCount; j++)
				{
					wdrFile.ReadAtPointer(geometryPointers[j], &geometries[j], sizeof(RSC_GEOMETRY));
					geometries[j].SwapData();
					//FileLOG(f, "  Geometry %d indices: %d vertices: %d faces: %d\n", j, 
					//	geometries[j].indexCount, geometries[j].vertexCount, geometries[j].faceCount);
					geometry.facesCount = geometries[j].faceCount;
					geometry.indexBuffer = new unsigned short[geometries[j].indexCount];
					wdrFile.ReadAtPointer(geometries[j].indexBuffers[0], &iBuffer, sizeof(RSC_INDEXBUFFER));
					iBuffer.SwapData();
					wdrFile.ReadAtPointer(iBuffer.indexData, geometry.indexBuffer, geometries[j].indexCount * 2);
					//fputs("   Indices:\n", f);
					for (UINT k = 0; k < geometries[j].indexCount; k++)
					{
						SwapEndian(geometry.indexBuffer[k]);
						//if(!((k + 1) % 3))
						//{
						//	sprintf(temp, "    %d %d %d\n", geometry.indexBuffer[k - 2], geometry.indexBuffer[k - 1], geometry.indexBuffer[k]);
						//	fputs(temp, f);
						//}	
					}
					geometry.indexCount = geometries[j].indexCount;
					geometry.indicesPerFace = geometries[j].indicesPerFace;
					wdrFile.ReadAtPointer(geometries[j].vertexBuffers[0], &vBuffer, sizeof(RSC_VERTEXBUFFER));
					vBuffer.SwapData();
					//FileLOG(f, "    VBuffer: %d vertices stride %d\n    ShaderId: %d\n", vBuffer.vertexCount, vBuffer.vertexSize, shaderMappings[j]);
					geometry.vertexBuffer = new unsigned char[vBuffer.vertexSize * geometries[j].vertexCount];
					wdrFile.ReadAtPointer(vBuffer.vertexData, geometry.vertexBuffer, vBuffer.vertexSize * geometries[j].vertexCount);

					wdrFile.ReadAtPointer(vBuffer.declaration, &geometry.rageDeclaration, sizeof(grcVertexDeclaration));
					geometry.rageDeclaration.SwapData();
					/*FileLOG(f, "     grcVertexDeclaration\n       UsedElements: %d (size %d) StoreNormalsFirst: %d\n", geometry.rageDeclaration.numElements, geometry.rageDeclaration.totalSize, geometry.rageDeclaration.storeNormalsDataFirst);
					if(geometry.rageDeclaration.usedElements.m_bPosition)
					FileLOG(f, "        POSITION (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nPositionType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nPositionType]);
					if(geometry.rageDeclaration.usedElements.m_bNormal)
					FileLOG(f, "        NORMAL (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nNormalType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nNormalType]);
					if(geometry.rageDeclaration.usedElements.m_bColor)
					FileLOG(f, "        COLOR (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nColorType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nColorType]);
					if(geometry.rageDeclaration.usedElements.m_bTangent)
					FileLOG(f, "        TANGENT (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTangentType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTangentType]);
					if(geometry.rageDeclaration.usedElements.m_bBinormal)
					FileLOG(f, "        BINORMAL (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nBinormalType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nBinormalType]);
					if(geometry.rageDeclaration.usedElements.m_bBlendIndices)
					FileLOG(f, "        BLEND_INDICES (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nBlendIndicesType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nBlendIndicesType]);
					if(geometry.rageDeclaration.usedElements.m_bBlendWeight)
					FileLOG(f, "        BLEND_WEIGHT (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nBlendWeightType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nBlendWeightType]);
					if(geometry.rageDeclaration.usedElements.m_bSpecularColor)
					FileLOG(f, "        SPECULAR_COLOR (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nSpecularColorType), gElementSizes[geometry.rageDeclaration.elementTypes.m_nSpecularColorType]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord1)
					FileLOG(f, "        TEXCOORD0 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord1Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord1Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord2)
					FileLOG(f, "        TEXCOORD1 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord2Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord2Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord3)
					FileLOG(f, "        TEXCOORD2 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord3Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord3Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord4)
					FileLOG(f, "        TEXCOORD3 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord4Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord4Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord5)
					FileLOG(f, "        TEXCOORD4 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord5Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord5Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord6)
					FileLOG(f, "        TEXCOORD5 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord6Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord6Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord7)
					FileLOG(f, "        TEXCOORD6 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord7Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord7Type]);
					if(geometry.rageDeclaration.usedElements.m_bTexCoord8)
					FileLOG(f, "        TEXCOORD7 (%s - %d)\n", GetVertexElementType(geometry.rageDeclaration.elementTypes.m_nTexCoord8Type), gElementSizes[geometry.rageDeclaration.elementTypes.m_nTexCoord8Type]);
					*/geometry.shaderId = shaderMappings[j];
					geometry.vertexCount = geometries[j].vertexCount;
					clump.frames[i].atomics[0].geometries.push_back(geometry);
				}
				// materials
				wdrFile.ReadAtPointer(drawable.pShaders, &shaderGroup, sizeof(RSC_SHADERGOUP));
				shaderGroup.SwapData();
				//FileLOG(f, " ShaderGroup (%d shaders)\n", shaderGroup.numShaders);
				shadersPtrs = new RSC_POINTER[shaderGroup.numShaders];
				wdrFile.ReadAtPointer(shaderGroup.shaders, shadersPtrs, 4 * shaderGroup.numShaders);
				for (int k = 0; k < shaderGroup.numShaders; k++)
					SwapEndian(shadersPtrs[k]);
				bool hasMat = false;
				for (int mat = 0; mat < shaderGroup.numShaders; mat++)
				{
					wdrFile.ReadAtPointer(shadersPtrs[mat], &shader, sizeof(RSC_SHADER));
					shader.SwapData();
					//FileLOG(f, "  Shader %d name %s\n", mat, GetShaderName(shader.nameHash));
					shaderParams = new RSC_SHADERPARAM[shader.paramsCount];
					wdrFile.ReadAtPointer(shader.shaderParams, shaderParams, 8 * shader.paramsCount);
					for (int param = 0; param < shader.paramsCount; param++)
					{
						shaderParams[param].SwapData();
						//printf("     Param %d pParam %u Type %d\n", param, shaderParams[param].param, shaderParams[param].type);
						if (param == 0&&(shaderParams[param].type == PARAM_DIFFUSE_SAMPLER || shaderParams[param].type == 131072))
						{
							hasMat = true;
							tex.SaveVtable();
							//printf("has mat\n");
							if (shaderParams[param].param.dataType && shaderParams[param].param.offset)
							{
								// Read Texture
								wdrFile.ReadAtPointer(shaderParams[param].param, &tex, sizeof(RSC_TEXTURE_V));
								tex.SwapData();
								tex.ResetVtable();
								wdrFile.ReadStringAtPointer(tex.GetName(), temp, 200);
								material.Set(temp, CColor(255, 255, 255, 255), shader.nameHash);
								strcpy(material.shaderName, GetShaderName(shader.nameHash));
								clump.frames[i].atomics[0].materials.push_back(material);
							}
							else
							{
								//printf("No tex\n");
								material.Set("\0", CColor(255, 255, 255, 255), shader.nameHash);
								strcpy(material.shaderName, GetShaderName(shader.nameHash));
								clump.frames[i].atomics[0].materials.push_back(material);
								//FileLOG(f, "      Texture \"NULL\"\n");
							}
						}
					}
					DEL_ARRAY(shaderParams)
				}

				if (!hasMat || clump.frames[i].atomics[0].materials.size() <= 0 || clump.frames[i].atomics[0].materials.size()<clump.frames[i].atomics[0].meshHeader.numMeshes)
				{
					material.Set("\0", CColor(255, 255, 255, 255), 0);
					strcpy(material.shaderName, "NO_SHADER");
					clump.frames[i].atomics[0].materials.push_back(material);
				}

				clump.frames[i].atomics[0].BuildMeshes(0);
				DEL_ARRAY(geometryPointers)
				DEL_ARRAY(geometries)
				DEL_ARRAY(shaderMappings)
				DEL_ARRAY(shadersPtrs)
			}
		}

		wdrFile.ReadAtPointer(drawable.pShaders, &shaderGroup, sizeof(RSC_SHADERGOUP));
		shaderGroup.SwapData();
		if (shaderGroup.texDictionary.dataType)
		{
			char sMName[512];
			char dname[512];
			_splitpath(fileName, NULL, NULL, sMName, NULL);
			strcpy(dname, fileName);
			if (strrchr(dname, '\\')) strrchr(dname, '\\')[1] = '\0';
			strcat(dname, sMName);
			strcat(dname, "+hidr");
			if (!DirectoryExists(dname))
				LOG("%s\n", dname);
		}
		// Internal TXD
		/*
		CTexDictionary txd;
		CImage image;

		wdrFile.ReadAtPointer(drawable.pShaders, &shaderGroup, sizeof(RSC_SHADERGOUP));
		shaderGroup.SwapData();
		if(shaderGroup.texDictionary.dataType)
		{
		//FileLOG(f, " This file has internal TXD\n");
		// generate a name for internal txd
		strcpy(&name[len - 3], "internal.txd");
		LOG(" Working on internal TXD (%s)\n", name);
		texRsc = new RSC_TEXTURE_V;
		wdrFile.ReadAtPointer(shaderGroup.texDictionary, &txdRsc, sizeof(RSC_TXD));
		txdRsc.SwapData();
		//FileLOG(f, "  Num textures in TXD: %d\n", txdRsc.count);
		//wdrFile.IsPointerValid(txdRsc.texturesPtrs, true);
		texturesPtrs = new RSC_POINTER[txdRsc.count];
		LOG("  Total textures in txd: %d\n", txdRsc.count);
		if(txdRsc.count > 0)
		{
		wdrFile.ReadAtPointer(txdRsc.texturesPtrs, texturesPtrs, txdRsc.count * 4);
		for(int i = 0; i < txdRsc.count; i++)
		{
		texRsc->SaveVtable();
		SwapEndian(texturesPtrs[i]);
		wdrFile.ReadAtPointer(texturesPtrs[i], texRsc, sizeof(RSC_TEXTURE_V));
		texRsc->SwapData();
		texRsc->ResetVtable();
		wdrFile.ReadStringAtPointer(texRsc->GetName(), image.name, 200);
		//FileLOG(f, "   Texture %d %s Width %d Height %d Levels %d\n",
		//	i, image.name, texRsc->width, texRsc->height, texRsc->levels);
		/*
		image.nameLength = wdrFile.ReadStringAtPointer(texRsc->GetName(), image.name, 200);
		if(!strncmp(image.name, "pack:/", 6))
		{
		strcpy(temp, image.name);
		strcpy(image.name, &temp[6]);
		image.nameLength -= 6;
		}
		if(!strcmp(&image.name[image.nameLength - 5], ".dds"))
		{
		image.name[image.nameLength - 5] = '\0';
		image.nameLength -= 4;
		}
		if(skipNormals)
		{
		if(!strcmp(&image.name[image.nameLength - 5], "norm") || !strcmp(&image.name[image.nameLength - 5], "_nrm")
		|| !strcmp(&image.name[image.nameLength - 3], "_n"))
		{
		LOG("%s: skipped normal texture %s\n", name, image.name);
		continue;
		}
		}
		if(skipBump)
		{
		if(!strcmp(&image.name[image.nameLength - 3], "_b") || !strcmp(&image.name[image.nameLength - 5], "bump")
		|| !strcmp(&image.name[image.nameLength - 8], "bump_01"))
		{
		LOG("%s: skipped bump texture %s\n", name, image.name);
		continue;
		}
		}
		if(skipSpec)
		{
		if(!strcmp(&image.name[image.nameLength - 3], "_s") || !strcmp(&image.name[image.nameLength - 5], "spec"))
		{
		LOG("%s: skipped specular texture %s\n", name, image.name);
		continue;
		}
		}
		image.format = (D3DFORMAT)texRsc->GetFormat();
		image.height = texRsc->GetHeight();
		image.width = texRsc->GetWidth();
		image.levels = texRsc->GetLevels();
		image.stride = texRsc->GetStride();
		// calculate pixelDataSize
		image.pixelsSize = image.stride * image.height;
		for(int j = 2; j <= image.levels; j++)
		image.pixelsSize += (image.stride * image.height) >> ((j - 1) * 2);
		image.pixels = new unsigned char[image.pixelsSize];
		wdrFile.ReadAtPointer(texRsc->GetPixelData(), image.pixels, image.pixelsSize);
		if(txd.FindTexture(image.name))
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
		//txd.Build(name);
		delete texRsc;
		delete[] texturesPtrs;
		}
		*/

		// create collision model

#define MAGIC_NUMBER 30000
#define GTA_TREES_SHADER 2025232660

		char colHelper[64];
		if (strrchr(colName, '\\')) strrchr(colName, '\\')[0] = '\0';
		strcat(colName, ".col");
		_splitpath(colName, NULL, NULL, colHelper, NULL);
		if (strrchr(colName, '\\')) strrchr(colName, '\\')[1] = '\0';
		strcat(colName, colHelper);
		strcat(colName, "\\");
		strcat(colName, colHelper);
		strcat(colName, ".col");

		unsigned int numGeoms = clump.frames[0].atomics[0].geometries.size();
		RpGeometryOptimised **optList = new RpGeometryOptimised *[numGeoms];
		unsigned int vertCounter = 0;
		unsigned int faceCounter = 0;
		unsigned int newFaceCount = 0;
		unsigned int newVertsCount = 0;

		// calc vertex count
		unsigned int totalVerts = 0;
		for (UINT i = 0; i < numGeoms; i++)
			totalVerts += clump.frames[0].atomics[0].geometries[i].vertexCount;
	
		for (UINT i = 0; i < numGeoms; i++)
		{
			if (clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod_tnt", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod2", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_lod2d", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_normal", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_normal_spec", 0)
				|| clump.frames[0].atomics[0].materials[clump.frames[0].atomics[0].meshes[i].material].shaderHash == GetHashKey("trees_tnt", 0)
				)
			{
				optList[i] = 0;
			}
			else
			{
				optList[i] = new RpGeometryOptimised(clump.frames[0].atomics[0].geometries[i].facesCount, clump.frames[0].atomics[0].geometries[i].vertexCount,
					clump.frames[0].atomics[0].meshes[i].indices, &clump.frames[0].atomics[0].positions[vertCounter], clump.frames[0].atomics[0].geometries[i].shaderId,
					vertCounter);
				if(totalVerts > MAGIC_NUMBER)
				{
					_rtGeometryOptimize((RpGeometry *)optList[i], 0.5f);
				}
				newFaceCount += optList[i]->numTriangles;
				newVertsCount += optList[i]->numVertices;
			}
			vertCounter += clump.frames[0].atomics[0].geometries[i].vertexCount;
			faceCounter += clump.frames[0].atomics[0].geometries[i].facesCount;
		}
		if(newFaceCount > 30000 || newVertsCount > 30000)
		{
			printf("ERROR: Too many faces/vertices in COL model");
			//MessageBox(0, "ERROR: Too many faces/vertices in COL model", "IV2SAconv", MB_OK | MB_ICONWARNING);
		}

		if (newVertsCount < 30000){

			RwStream *col = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMAPPEND, colName);
			CollisionModel2 colmodel;
			colmodel.fourcc = '2LOC';
			colmodel.flags = 2;
			strcpy(colmodel.name, nameHash);
			colmodel.unused = 0;
			colmodel.modelIndex = 20000;
			colmodel.bbox.Set(*(Vector3 *)&drawable.aabbMin, *(Vector3 *)&drawable.aabbMax);
			colmodel.bbox.aabbMax.x -= drawable.center.x;
			colmodel.bbox.aabbMax.y -= drawable.center.y;
			colmodel.bbox.aabbMax.z -= drawable.center.z;
			colmodel.bbox.aabbMin.x -= drawable.center.x;
			colmodel.bbox.aabbMin.y -= drawable.center.y;
			colmodel.bbox.aabbMin.z -= drawable.center.z;
			colmodel.bbox.ToSphere(&colmodel.bsphere);
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
			for (UINT j = 0; j < clump.frames[0].atomics[0].geometries.size(); j++)
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
			for (UINT j = 0; j < clump.frames[0].atomics[0].geometries.size(); j++)
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
						if (clump.frames[0].atomics[0].materials[matId]._StreamData.textured&&gbUseColMaterials)
						{
							matName = clump.frames[0].atomics[0].materials[matId].texture._StreamData.nameAligned;
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

			for (UINT i = 0; i < clump.frames[0].atomics[0].geometries.size(); i++)
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
			colmodel.bbox.Set(*(Vector3 *)&drawable.aabbMin, *(Vector3 *)&drawable.aabbMax);
			colmodel.bbox.aabbMax.x -= drawable.center.x;
			colmodel.bbox.aabbMax.y -= drawable.center.y;
			colmodel.bbox.aabbMax.z -= drawable.center.z;
			colmodel.bbox.aabbMin.x -= drawable.center.x;
			colmodel.bbox.aabbMin.y -= drawable.center.y;
			colmodel.bbox.aabbMin.z -= drawable.center.z;
			colmodel.bbox.ToSphere(&colmodel.bsphere);
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

		/*
		RwStream *col = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMAPPEND, colName);
		CollisionModel2 colmodel;
		colmodel.fourcc = '2LOC';
		colmodel.flags = 2;
		strcpy(colmodel.name, nameHash);
		colmodel.unused = 0;
		colmodel.modelIndex = 20000;
		colmodel.bbox.Set(*(Vector3 *)&drawable.aabbMin, *(Vector3 *)&drawable.aabbMax);
		colmodel.bbox.ToSphere(&colmodel.bsphere);
		colmodel.numBoxes = 0;
		colmodel.numFaces = clump.frames[0].atomics[0].totalNumFaces;
		colmodel.numSpheres = 0;
		colmodel.offsetBoxes = 0;
		colmodel.offsetLines = 0;
		colmodel.offsetSpheres = 0;
		colmodel.offsetTrianglePlanes = 0;
		unsigned int verticesSize = clump.frames[0].atomics[0].totalNumVertex * 6;
		if(verticesSize % 4)
			verticesSize += 2;
		colmodel.size = sizeof(CollisionModel2) + colmodel.numFaces * 8 + verticesSize - 8;
		colmodel.offsetVertices = sizeof(CollisionModel2) - 4;
		colmodel.offsetFaces = colmodel.offsetVertices + verticesSize;
		gtaRwStreamWrite(col, &colmodel, sizeof(CollisionModel2));
		ColVertex *colVerts = new ColVertex[clump.frames[0].atomics[0].totalNumVertex];
		for(int i = 0; i < clump.frames[0].atomics[0].totalNumVertex; i++)
		{
			colVerts[i].x = clump.frames[0].atomics[0].positions[i].x * 128.0f;
			colVerts[i].y = clump.frames[0].atomics[0].positions[i].y * 128.0f;
			colVerts[i].z = clump.frames[0].atomics[0].positions[i].z * 128.0f;
		}
		gtaRwStreamWrite(col, colVerts, verticesSize);
		delete[] colVerts;
		ColTriangle *colTris = new ColTriangle[colmodel.numFaces];
		char *matName;
		unsigned short matId;
		for(int i = 0; i < colmodel.numFaces; i++)
		{
			colTris[i].a = clump.frames[0].atomics[0].triangles[i].c;
			colTris[i].b = clump.frames[0].atomics[0].triangles[i].b;
			colTris[i].c = clump.frames[0].atomics[0].triangles[i].a;
			// try to match material...
			matId = clump.frames[0].atomics[0].triangles[i].d;
			if(clump.frames[0].atomics[0].materials[matId]._StreamData.textured)
			{
				matName = clump.frames[0].atomics[0].materials[matId].texture._StreamData.nameAligned;
				if(strstr(matName, "tarmac"))
					colTris[i].material = 1;
				else if(strstr(matName, "grass"))
				{
					if(strstr(matName, "dirt"))
						colTris[i].material = 13;
					else
						colTris[i].material = 10;
				}
				else if(strstr(matName, "stone") || strstr(matName, "rock"))
					colTris[i].material = 35;
				else if(strstr(matName, "wood"))
				{
					if(strstr(matName, "step"))
						colTris[i].material = 73;
					else if(strstr(matName, "bench"))
						colTris[i].material = 70;
					else if(strstr(matName, "floor"))
						colTris[i].material = 22;
					else
						colTris[i].material = 43;
				}
				else if(strstr(matName, "gravel"))
					colTris[i].material = 6;
				else if(strstr(matName, "rubbish"))
					colTris[i].material = 139;
				else if(strstr(matName, "poster") || strstr(matName, "billboard"))
					colTris[i].material = 68;
				else if(strstr(matName, "pavement") || strstr(matName, "alleypath"))
					colTris[i].material = 4;
				else if(strstr(matName, "fence"))
					colTris[i].material = 55;
				else if(strstr(matName, "concrete"))
					colTris[i].material = 7;
				else if(strstr(matName, "girder"))
					colTris[i].material = 56;
				else if(strstr(matName, "gate"))
					colTris[i].material = 54;
				else if(strstr(matName, "traintracks"))
					colTris[i].material = 178;
				else if(strstr(matName, "tail"))
					colTris[i].material = 107;
				else if(strstr(matName, "glass"))
					colTris[i].material = 45;
				else if(strstr(matName, "metal") || strstr(matName, "alum"))
				{
					if(strstr(matName, "step"))
						colTris[i].material = 162;
					else if(strstr(matName, "floor"))
						colTris[i].material = 164;
					else
						colTris[i].material = 51;
				}
				else if(strstr(matName, "door"))
					colTris[i].material = 158;
				else if(strstr(matName, "rubber") || strstr(matName, "bball_court"))
					colTris[i].material = 67;
				else
					colTris[i].material = 0;
			}
			else
				colTris[i].material = 0;
			//
			colTris[i].lighting = 255;
		}
		gtaRwStreamWrite(col, colTris, colmodel.numFaces * 8);
		delete[] colTris;

		gtaRwStreamClose(col, NULL);*/

		strcpy(&name[len - 3], "dff");
		RwStream *dff = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, name);
		clump.Write(dff);
		gtaRwStreamClose(dff, NULL);
		printf("%i - mat count\n", clump.frames[0].atomics.size());
		for (int i = 0; i < modelsCollection.count; i++)
		{
			printf("%i - g count\n", models[i].geometriesCount);
			for (int j = 0; j < models[i].geometriesCount; j++)
			{
				DEL_ARRAY(clump.frames[i].atomics[0].geometries[j].indexBuffer)
				DEL_ARRAY(clump.frames[i].atomics[0].geometries[j].vertexBuffer)
			}
		}
		//fclose(f);
		DEL_ARRAY(pointers)
		printf("ptrs deleted\n");
		DEL_ARRAY(models)
			/*for (int i = 0; i < clump.frames.size(); i++)
			{
			for (int j = 0; j < clump.frames[i].atomics.size(); j++)
			{
			clump.frames[i].atomics[j].meshes.clear();
			clump.frames[i].atomics[j].geometries.clear();
			}
			clump.frames[i].atomics.clear();
			//clump.frames[i].clumpMaterials.clear();
			}
			clump.frames.clear();*/
		/*clump.frames.push_back(CFrame());
		for (int i = 0; i < clump.frames.size(); i++)
		{
			clump.frames[i].atomics.push_back(CModelInstance());
			//DEL_ARRAY(clump.frames[i].clumpMaterials)
			for (int k = 0; k < clump.frames[i].atomics.size(); k++)
			{
				clump.frames[i].atomics[k].geometries.push_back(CRageGeometry());
				clump.frames[i].atomics[k].materials.push_back(CMaterial());
				clump.frames[i].atomics[k].meshes.push_back(CMesh());
			}
		}
		clump.materials.push_back(CMaterial());*/
		printf("models deleted\n");
	}
	printf("models deleted2\n");
}

DWORD Xbox360ConvertTextureAdress(int x, int y, int w, int TexelPitch){
	DWORD alignedWidth, logBpp, Macro, Micro, Offset;
	alignedWidth = (w + 31) & !31;
	logBpp = (TexelPitch >> 2) + ((TexelPitch >> 1) >> (TexelPitch >> 2));
	Macro = ((x >> 5) + (y >> 5)*(alignedWidth >> 5)) << (logBpp + 7);
	Micro = (((x & 7) + ((y & 6) << 2)) << logBpp);
	Offset = Macro + ((Micro & !15) << 1) + (Micro & 15) + ((y & 8) <<(3 + logBpp)) + ((y & 1) << 4);
	return (((Offset & !511) << 3) + ((Offset & 448) << 2) + (Offset & 63) + ((y & 16) << 7) + (((((y & 8) >> 2) + (x >> 3)) & 3) << 6)) >> logBpp;
}

void ConvertToTxd(char *fileName)
{
	char name[128];
	char nametmp[128];
	char nametmphi[128];
	bool isIntenal = false;
	char txdName[100];
	_splitpath(fileName, NULL, NULL, txdName, NULL);
	strcpy(name, fileName);
	if (strrchr(name, '\\')) strrchr(name, '\\')[1] = '\0';
	strcat(name, txdName);
	if (!strcmp(&txdName[strlen(txdName) - 5], "+hidr")){
		isIntenal = true;
	}
	else if (!strcmp(&txdName[strlen(txdName) - 4], "+xdr"))
	{
		isIntenal = true;
	}
	else if (!strcmp(&txdName[strlen(txdName) - 3], "+hi"))
	{
		isIntenal = true;
	}
	strcpy(nametmp, name);
	
	strcpy(nametmphi, name);
	strcat(nametmphi, "+hi\\*.dds");
	LOG("%s\n", nametmphi);
	strcat(nametmp, "\\*.dds");
	g_TmpImages.clear();

	SearchFiles(nametmphi, (LPSEARCHFUNC)ConvertToImage);
	SearchFiles(nametmp, (LPSEARCHFUNC)ConvertToImage);
	
	if (strrchr(name, '\\')) strrchr(name, '\\')[1] = '\0';
	int alignementSize = strlen(txdName);
	if (!strcmp(&txdName[strlen(txdName) - 5], "+hidr")){
		txdName[strlen(txdName) - 5] = '\0';
	}
	else if (!strcmp(&txdName[strlen(txdName) - 4], "+xdr"))
	{
		txdName[strlen(txdName) - 4] = '\0';
	}
	else if (!strcmp(&txdName[strlen(txdName) - 3], "+hi"))
	{
		txdName[strlen(txdName) - 3] = '\0';
	}
	unsigned int hash = GetHashKey(txdName, 0);
	LOG("\n%s=%X\n", txdName, hash);
	sprintf(txdName, "%X", hash);
	strcat(name, txdName);
	if (isIntenal) strcat(name, ".internal");
	strcat(name, ".txd");
	CTexDictionary txd;
	for (UINT i = 0; i < g_TmpImages.size(); i++){
		if (txd.FindTexture(g_TmpImages[i].name))
		{
			LOG("   texture %s is already present in txd %s", g_TmpImages[i].name, name);
			delete[] g_TmpImages[i].pixels;
		}
		else
			txd.images.push_back(g_TmpImages[i]);
	}
	txd.Build(name);
}