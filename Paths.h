#pragma once

#include "CRenderWare.h"
#include <iostream>

#pragma pack(push, 4)
class CPathNode
{
public:
	CPathNode *m_pPrev;
	CPathNode **m_ppToThisNodeInNextNode;
	class CompressedVector {
	public:
		short x, y, z;
	} m_posn;
	unsigned short m_wSearchList;
	short  m_wBaseLinkId;
	short  m_wAreaId;
	short  m_wNodeId;
	unsigned char m_nPathWidth;
	unsigned char m_nNodeType;
	/*
	m_dwFlags[0] & 0xF => m_nNumLinks
	m_dwFlags[2] >> 4  => m_nBahaviourType
	*/
	unsigned int m_nNumLinks : 4;
	unsigned int m_nTrafficLevel : 2;
	unsigned int m_bRoadBlocks : 1;
	unsigned int m_bWaterNode : 1;

	unsigned int m_bEmergencyVehiclesOnly : 1;
	unsigned int unk1 : 1;
	unsigned int m_bDontWander : 1;
	unsigned int unk2 : 1;
	unsigned int m_bNotHighway : 1;
	unsigned int m_bHighway : 1;
	unsigned int unk3 : 1;
	unsigned int unk4 : 1;

	unsigned int m_nSpawnProbability : 4;
	unsigned int m_nBehaviourType : 4; // 1 - roadblock
	                                   // 2 - parking node
};
#pragma pack(pop)

static_assert(sizeof(CPathNode) == 28, "Fail");

#pragma pack(push, 1)
class CNodeAddress
{
public:
	short m_wRegion;
	short m_wNodeId;
};
#pragma pack(pop)

#pragma pack(push, 1)
class CNodeCarLinkAddress
{
public:
	short m_wNodeId : 10;
	short m_wAreaId : 6;
};
#pragma pack(pop)

static_assert(sizeof(CNodeCarLinkAddress) == 2, "Fail");

#pragma pack(push, 1)
class CCarPathLink
{
public:
	short x;
	short y;
	CNodeAddress m_pathNodeAddress;
	char dirX;
	char dirY;
	unsigned char m_nPathNodeWidth;

	unsigned char m_nNumLeftLanes : 3;
	unsigned char m_nNumRightLanes : 3;
	unsigned char m_bTrafficLightDirection : 1;
	unsigned char unk1 : 1;

	unsigned short m_nTrafficLightState : 2;
	unsigned short m_bTrainCrossing : 1;
};
#pragma pack(pop)

static_assert(sizeof(CCarPathLink) == 14, "Fail");

class CPathIntersectionFlags
{
public:
	unsigned char m_bRoadCross : 1;
	unsigned char m_bPedTrafficLight : 1;
};

static_assert(sizeof(CPathIntersectionFlags) == 1, "Fail");

class CPathReaderWriter
{
public:

	static void DumpSANodeFile(char *filename)
	{
		unsigned int numNodes, numVehicleNodes, numPedPaths, numCarPathLinks, numLinkingAddresses;
		RwStream *nodefile = gtaRwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
		
		gtaRwStreamRead(nodefile, &numNodes, 4);
		gtaRwStreamRead(nodefile, &numVehicleNodes, 4);
		gtaRwStreamRead(nodefile, &numPedPaths, 4);
		gtaRwStreamRead(nodefile, &numCarPathLinks, 4);
		gtaRwStreamRead(nodefile, &numLinkingAddresses, 4);
		
		CPathNode *nodes = new CPathNode[numNodes];
		CCarPathLink *carLinks = new CCarPathLink[numCarPathLinks];
		CNodeAddress *nodeLinks = new CNodeAddress[numLinkingAddresses];
		CNodeCarLinkAddress *nodeCarLinks = new CNodeCarLinkAddress[numLinkingAddresses];
		unsigned char *nodeLinksLengths = new unsigned char[numLinkingAddresses];
		CPathIntersectionFlags *intsFlags = new CPathIntersectionFlags[numLinkingAddresses];

		gtaRwStreamRead(nodefile, nodes, numNodes * sizeof(CPathNode));
		gtaRwStreamRead(nodefile, carLinks, numCarPathLinks * sizeof(CCarPathLink));
		gtaRwStreamRead(nodefile, nodeLinks, numLinkingAddresses * sizeof(CNodeAddress));
		gtaRwStreamSkip(nodefile, 768);
		gtaRwStreamRead(nodefile, nodeCarLinks, numLinkingAddresses * sizeof(CNodeCarLinkAddress));
		gtaRwStreamRead(nodefile, nodeLinksLengths, numLinkingAddresses);
		//gtaRwStreamSkip(nodefile, 192);
		//gtaRwStreamRead(nodefile, intsFlags, numLinkingAddresses *  sizeof(CPathIntersectionFlags));
		gtaRwStreamClose(nodefile);

		char temp[256];
		strcpy(temp, filename);
		strcpy(&temp[strlen(temp) - 4], "_decompiled.txt");
		FILE *file = fopen(temp, "wt");
		sprintf(temp, "TotalNumNodes %d\nNumVehicleNodes + NumPedNodes = %d + %d = %d\nNumCarPathLinks %d\nNumLinkingAddresses %d\n\nPathNodes\n{\n",
			numNodes, numVehicleNodes, numPedPaths, numVehicleNodes + numPedPaths, numCarPathLinks, numLinkingAddresses);
		fputs(temp, file);
		for (UINT i = 0; i < numNodes; i++)
		{
			if(nodes[i].m_bWaterNode)
			{
				sprintf(temp, "    %d (%d) WATER posn: %.2f %.2f %.2f numLinks %d linksOffset %d\n", nodes[i].m_wNodeId,
					nodes[i].m_wAreaId, (float)nodes[i].m_posn.x / 8.0f, (float)nodes[i].m_posn.y / 8.0f, (float)nodes[i].m_posn.z / 8.0f,
					nodes[i].m_nNumLinks, nodes[i].m_wBaseLinkId);
			}
			else if(nodes[i].m_nNodeType == 1)
			{
				sprintf(temp, "    %d (%d) VEHICLE posn: %.2f %.2f %.2f numLinks %d linksOffset %d\n", nodes[i].m_wNodeId,
					nodes[i].m_wAreaId, (float)nodes[i].m_posn.x / 8.0f, (float)nodes[i].m_posn.y / 8.0f, (float)nodes[i].m_posn.z / 8.0f,
					nodes[i].m_nNumLinks, nodes[i].m_wBaseLinkId);
			}
			else
			{
				sprintf(temp, "    %d (%d) PED posn: %.2f %.2f %.2f numLinks %d linksOffset %d\n", nodes[i].m_wNodeId,
					nodes[i].m_wAreaId, (float)nodes[i].m_posn.x / 8.0f, (float)nodes[i].m_posn.y / 8.0f, (float)nodes[i].m_posn.z / 8.0f,
					nodes[i].m_nNumLinks, nodes[i].m_wBaseLinkId);
			}
			fputs(temp, file);
		}
		fputs("}\nNodeLinks\n{\n", file);
		for (UINT i = 0; i < numLinkingAddresses; i++)
		{
			sprintf(temp, "    %d area %d node %d\n", i, nodeLinks[i].m_wRegion, nodeLinks[i].m_wNodeId);
			fputs(temp, file);
		}
		fputs("}\nCarPathLinks\n{\n", file);
		for (UINT i = 0; i < numCarPathLinks; i++)
		{
			sprintf(temp, "    area %d node %d x %.2f y %.2f\n", carLinks[i].m_pathNodeAddress.m_wRegion, carLinks[i].m_pathNodeAddress.m_wNodeId,
				(float)carLinks[i].x / 8.0f, (float)carLinks[i].y / 8.0f);
			fputs(temp, file);
		}
		fputs("}\nCarPathLinksLinkingAddresses\n{\n", file);
		for (UINT i = 0; i < numLinkingAddresses; i++)
		{
			sprintf(temp, "    %d area %d node %d\n", i, nodeCarLinks[i].m_wAreaId, nodeCarLinks[i].m_wNodeId);
			fputs(temp, file);
		}
		fputs("}", file);
		fclose(file);

		delete[] nodes;
		delete[] carLinks;
		delete[] nodeLinks;
		delete[] nodeCarLinks;
		delete[] nodeLinksLengths;
		delete[] intsFlags;
	}
};