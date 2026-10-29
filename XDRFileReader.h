#pragma once
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
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
class CXDRFileReader
{
	static void GetNames(LPSTR path);
	static RSC_DRAWABLE GetDrawable(CResource file);
public:
	static void ReadXDR(LPSTR path);
	static LPSTR m_sCurrModelName;
	static LPSTR m_sCurrCollName;
	static LPSTR m_sCurrFilePath;
	static LPSTR m_sCurrModelHash;
	static UINT m_nCurrModelHash;
	static bool m_bDebug;
};

