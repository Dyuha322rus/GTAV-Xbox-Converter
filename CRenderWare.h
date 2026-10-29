#pragma once

#include "RenderWare.h"

bool gtaRwErrorSet(char *message, ...);
RwStream *gtaRwStreamOpen(RwStreamType type, RwStreamAccessType accessType, const void *pData);
bool gtaRwStreamClose(RwStream * stream, void *pData = NULL);
unsigned int gtaRwStreamRead(RwStream *stream, void *buffer, unsigned int length);
RwStream *gtaRwStreamSkip(RwStream *stream, unsigned int offset);
bool gtaChunkIsComplex(RwChunkHeaderInfo *chunkHeaderInfo);
bool gtaRwStreamReadChunkHeader(RwStream *stream, unsigned int *pType, unsigned int *pLength, unsigned int *pVersion, unsigned int *pBuild);
bool gtaRwStreamFindChunk(RwStream *stream, unsigned int type, unsigned int *lengthOut, unsigned int *versionOut);
RwPluginRegistry * gtaRwPluginRegistryInitObject(RwPluginRegistry *registry, void *object);
RwPluginRegistry *gtaRwPluginRegistryDeInitObject(RwPluginRegistry *registry, void *object);
RpClump *gtaRpClumpCreate();
RpClump *gtaRpClumpStreamRead(RwStream *stream);
RwStream *gtaRwStreamReadReal(RwStream *stream, float *reals, unsigned int numBytes);
RwStream *gtaRwStreamWrite(RwStream *stream, const void *buffer, unsigned int length);
RwStream *gtaRwStreamWriteVersionedChunkHeader(RwStream *stream, unsigned int type, unsigned int size, 
													   unsigned int version, unsigned short buildNum);
RwStream *gtaRwStreamWriteInt32(RwStream *stream, unsigned int *integers, unsigned int length);