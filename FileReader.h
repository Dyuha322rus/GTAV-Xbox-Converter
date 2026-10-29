#pragma once

#include <stdio.h>
#include <Windows.h>
// GTA IV RSC file

#define RSC_CPU_DATA 5
#define RSC_GPU_DATA 6

struct RSC_POINTER
{
	unsigned int offset : 28;
	unsigned int dataType : 4;
};

struct RSC_VECTOR4
{
	float x, y, z, w;
};

struct RSC_HEADER
{
	unsigned int fourcc;
	unsigned int unk1;
	unsigned int sysFlags;
	unsigned int gfxFlags;
	
	unsigned int SwapEndian32(unsigned int value)
	{
		char tmp[4];
		tmp[3] =  ((char *)&value)[0];
		tmp[2] =  ((char *)&value)[1];
		tmp[1] =  ((char *)&value)[2];
		tmp[0] =  ((char *)&value)[3];
		unsigned int result;
		memmove(&result, tmp, 4);
		return result;
	}

	int Reverse4Bits(int num)
	{
		return (num >> 3) | ((num >> 1) & 2) | ((num & 2) << 1) | ((num & 1) << 3);
	}
	
	int GetSizeFromFlag(unsigned int flag, int baseSize)
	{
		baseSize <<= (int)(flag & 0xf);
		int size = (int)((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * baseSize);
		for (int i = 0; i < 4; ++i)
		{
			size += (((flag >> (24 + i)) & 1) == 1) ? (baseSize >> (1 + i)) : 0;
		}
		return size;
	}
	
	int GetSizeFromSystemFlag(unsigned int flag, int mode)
	{
		if (mode == 1)
		{
			// PS3
			return GetSizeFromFlag(flag, 0x1000);
		}
		else
		{
			// XBOX 360
			return GetSizeFromFlag(flag, 0x2000);
		}
	}
	
	int GetSizeFromGraphicsFlag(unsigned int flag, int mode)
	{
		if (mode == 1)
		{
			// PS3
			return GetSizeFromFlag(flag, 0x1580);
		}
		else
		{
			// XBOX 360
			return GetSizeFromFlag(flag, 0x2000);
		}
	}
	
	int GetResourceVersionFromFlags(unsigned int systemFlag, unsigned int graphicsFlag)
	{
		return (int)(((graphicsFlag >> 28) & 0xF) | (((systemFlag >> 28) & 0xF) << 4));
	}

	unsigned int cpuSize() { return GetSizeFromSystemFlag(sysFlags, 0); }
	unsigned int gpuSize() { return GetSizeFromGraphicsFlag(gfxFlags, 0); }
};

class CResource : public RSC_HEADER
{
public:
	unsigned int m_dwCpuDataSize;
	unsigned int m_dwGpuDataSize;
	unsigned char *m_pCpuData;
	unsigned char *m_pGpuData;
	unsigned int m_dwCpuCurrPos;
	unsigned int m_dwGpuCurrPos;
	unsigned int m_dwSize;

	CResource(const char *filename)
	{
		FILE *file = fopen(filename, "rb");
		fseek(file, 0, SEEK_END);
		unsigned int fsize = ftell(file)  - sizeof(RSC_HEADER);
		unsigned char *fileData = new unsigned char[fsize];
		fseek(file, 0, SEEK_SET);
		fread(this, sizeof(RSC_HEADER), 1, file);
		sysFlags = SwapEndian32(sysFlags);
		gfxFlags = SwapEndian32(gfxFlags);
		m_dwCpuDataSize = cpuSize();
		m_dwGpuDataSize = gpuSize();
		m_dwSize = m_dwCpuDataSize + m_dwGpuDataSize;
		m_pCpuData = new unsigned char[m_dwCpuDataSize + m_dwGpuDataSize];
		m_pGpuData = &m_pCpuData[m_dwCpuDataSize];

		//printf("\nResource (XDR) Sys size: %d Gfx size: %d\n", m_dwCpuDataSize, m_dwGpuDataSize);

		fread(m_pCpuData, fsize, 1, file);
		fclose(file);

		char fname[512];
		strcpy(fname, filename);
		strcpy(&fname[strlen(fname) - 4], ".sys");
		FILE *sys = fopen(fname, "wb");
		fwrite(m_pCpuData, m_dwCpuDataSize, 1, sys);
		fclose(sys);
		DeleteFile(fname);
		strcpy(&fname[strlen(fname) - 4], ".gfx");
		FILE *gfx = fopen(fname, "wb");
		fwrite(m_pGpuData, m_dwGpuDataSize, 1, gfx);
		fclose(gfx);
		DeleteFile(fname);
		m_dwCpuCurrPos = 0;
		m_dwGpuCurrPos = 0;
	}

	~CResource() { delete[] m_pCpuData; }

	void ReadCpu(void *data, unsigned int size, bool setPos = true)
	{
		memcpy(data, &m_pCpuData[m_dwCpuCurrPos], size);
		if(setPos)
			m_dwCpuCurrPos += size;
	}

	unsigned char ReadCpuChar(bool setPos = true)
	{
		unsigned char result;
		memcpy(&result, &m_pCpuData[m_dwCpuCurrPos], 1);
		if(setPos)
			m_dwCpuCurrPos += 1;
		return result;
	}

	unsigned short ReadCpuShort(bool setPos = true)
	{
		unsigned short result;
		memcpy(&result, &m_pCpuData[m_dwCpuCurrPos], 2);
		if(setPos)
			m_dwCpuCurrPos += 2;
		return result;
	}

	unsigned short ReadCpuInt(bool setPos = true)
	{
		unsigned short result;
		memcpy(&result, &m_pCpuData[m_dwCpuCurrPos], 4);
		if(setPos)
			m_dwCpuCurrPos += 4;
		return result;
	}

	RSC_POINTER ReadCpuPointer(bool setPos = true)
	{
		RSC_POINTER result;
		memcpy(&result, &m_pCpuData[m_dwCpuCurrPos], 4);
		if(setPos)
			m_dwCpuCurrPos += 4;
		return result;
	}

	void ReadAtPointer(RSC_POINTER pointer, void *data, unsigned int size, bool setPos = false)
	{
		if(pointer.dataType != RSC_CPU_DATA && pointer.dataType != RSC_GPU_DATA)
		{
			MessageBox(0, "Incorrect pointer", "CResource::ReadAtPointer()", MB_ICONWARNING|MB_OK);
			return;
		}
		memcpy(data, pointer.dataType == RSC_CPU_DATA? m_pCpuData + pointer.offset : m_pGpuData + pointer.offset, size);
		if(setPos)
			pointer.dataType == RSC_CPU_DATA? m_dwCpuCurrPos : m_dwGpuCurrPos = pointer.offset + size;
	}

	bool IsPointerValid(RSC_POINTER pointer, bool showMessage = false)
	{
		if((pointer.dataType != RSC_CPU_DATA && pointer.dataType != RSC_GPU_DATA) || (pointer.offset == 0) || (pointer.offset > m_dwSize))
		{
			if(showMessage)
				MessageBox(0, "Not a valid pointer", "CResource::IsPointerValid()", MB_ICONWARNING|MB_OK);
			return false;
		}
		return true;
	}

	unsigned int ReadStringAtPointer(RSC_POINTER pointer, char *str, unsigned int maxSize, bool setPos = false)
	{
		unsigned char *pFileStr = pointer.dataType == RSC_CPU_DATA? m_pCpuData + pointer.offset : m_pGpuData + pointer.offset;
		if(pointer.offset > m_dwSize)
		{
			return 0;
		}
		unsigned int l;
		for(l = 0; l < maxSize; l++)
		{
			str[l] = pFileStr[l];
			if(!pFileStr[l])
				break;
			if(l == (maxSize - 1))
			{
				pFileStr[l] = 0;
				break;
			}
		}
		if(setPos)
			pointer.dataType == RSC_CPU_DATA? m_dwCpuCurrPos : m_dwGpuCurrPos = pointer.offset + l + 1;
		return l + 1;
	}

	void ReadGpu(void *data, unsigned int size, bool setPos = true)
	{
		memcpy(data, &m_pGpuData[m_dwGpuCurrPos], size);
		if(setPos)
			m_dwGpuCurrPos += size;
	}

	unsigned char ReadGpuChar(bool setPos = true)
	{
		unsigned char result;
		memcpy(&result, &m_pGpuData[m_dwGpuCurrPos], 1);
		if(setPos)
			m_dwGpuCurrPos += 1;
		return result;
	}

	unsigned short ReadGpuShort(bool setPos = true)
	{
		unsigned short result;
		memcpy(&result, &m_pGpuData[m_dwGpuCurrPos], 2);
		if(setPos)
			m_dwGpuCurrPos += 2;
		return result;
	}

	unsigned int ReadGpuInt(bool setPos = true)
	{
		unsigned int result;
		memcpy(&result, &m_pGpuData[m_dwGpuCurrPos], 4);
		if(setPos)
			m_dwGpuCurrPos += 4;
		return result;
	}

	RSC_POINTER ReadGpuPointer(bool setPos = true)
	{
		RSC_POINTER result;
		memcpy(&result, &m_pGpuData[m_dwGpuCurrPos], 4);
		if(setPos)
			m_dwCpuCurrPos += 4;
		return result;
	}

	void SkipCpu(unsigned int size)
	{
		m_dwCpuCurrPos += size;
	}

	void SkipGpu(unsigned int size)
	{
		m_dwGpuCurrPos += size;
	}

	void SeekCpu(unsigned int posn)
	{
		m_dwCpuCurrPos = posn;
	}

	void SeekGpu(unsigned int posn)
	{
		m_dwGpuCurrPos = posn;
	}
};