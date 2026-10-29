#include "XDRFileReader.h"
bool CXDRFileReader::m_bDebug = false;
LPSTR CXDRFileReader::m_sCurrModelName;
LPSTR CXDRFileReader::m_sCurrFilePath;
LPSTR CXDRFileReader::m_sCurrModelHash;
LPSTR CXDRFileReader::m_sCurrCollName;
UINT CXDRFileReader::m_nCurrModelHash;

void CXDRFileReader::GetNames(LPSTR path){
	_splitpath(path, NULL, NULL, m_sCurrModelName, NULL);
	strcpy(m_sCurrFilePath, path);
	strcpy(m_sCurrCollName, path);
	if (strrchr(m_sCurrFilePath, '\\')) strrchr(m_sCurrFilePath, '\\')[1] = '\0';
	m_nCurrModelHash = GetHashKey(m_sCurrModelName, 0);
	gDFFs.push_back(m_nCurrModelHash);
	sprintf(m_sCurrModelHash, "%X", m_nCurrModelHash);
	strcat(m_sCurrFilePath, m_sCurrModelHash);
	strcat(m_sCurrFilePath, ".xdr");
}
RSC_DRAWABLE GetDrawable(CResource file){
	RSC_DRAWABLE Drawable;
	file.ReadCpu(&Drawable, sizeof(RSC_DRAWABLE));
	Drawable.SwapData();
	return Drawable;
}
void CXDRFileReader::ReadXDR(LPSTR path)
{
	if (m_bDebug)
		printf("\n Reading XDR Model at %s\n", path);
	GetNames(path);
	CResource XDRFile(path);
	RSC_DRAWABLE XDRDrawableHeader = GetDrawable(XDRFile);
	//FILE *f = fopen(name, "wt");
}
