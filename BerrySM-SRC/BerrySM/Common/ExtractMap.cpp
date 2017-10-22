// ExtractMap.cpp

#include "StdAfx.h"
#include "ExtractMap.h"
#include <io.h>
#include "../common/log.h"
#include <stdio.h>
#ifdef _DEBUG
#undef THIS_FILE
static CHAR THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BITFLAG_IS_IMPASSABLE 128
//#define WRITEMAPIMPASSIBLEINFO
extern BOOLEAN  *g_pMapPassableInfo;
extern UINT		*g_pRoadBuffer;
extern BYTE		*g_pMapBuffer;

extern int		g_nMapWidth;
extern int		g_nMapHeight;
extern int		g_nMapStartX;
extern int		g_nMapStartY;
extern int		g_nMapEndX;
extern int		g_nMapEndY;

CExtractMap::CExtractMap(void)
{
	m_wMapNumber = 0;
	m_wStartX = 0;
	m_wStartY = 0;
	m_wEndX = 0;
	m_wEndY = 0;
	m_wCountX = 0;
	m_wCountY = 0;
	m_dwStartXPos = 0x0FFFF;
	m_dwStartYPos = 0x0FFFF;
	m_dwEndXPos = 0;
	m_dwEndYPos = 0;
}

CExtractMap::~CExtractMap(void)
{
	SafeDelete();
}

BOOL CExtractMap::Initialize()
{
	CHAR szLinExePath[MAX_PATH] = {0};
	HKEY	hKey;
	DWORD	dwType, dwLen = sizeof(szLinExePath);

	RtlZeroMemory(m_szMapRootPath, sizeof(m_szMapRootPath));

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\NC Soft\\Lineage", 0, STANDARD_RIGHTS_READ|KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) 
		return FALSE;

	if (RegQueryValueExA(hKey, "ExecutePath", NULL, &dwType, (LPBYTE)szLinExePath, &dwLen) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);

	char *p = strstr(szLinExePath, "lineage.exe");
	if(p == NULL)
		return FALSE;
	*p = NULL;
	strcat_s(szLinExePath, "map");

	strcpy_s(m_szMapRootPath, _countof(m_szMapRootPath), szLinExePath);

	return TRUE;
}

// get map impassible info from map info
BOOL CExtractMap::isInMap(WORD x, WORD y)
{
	if (m_wMapNumber == 4 && (x < 32520 || y < 32070 || (y < 32190 && x < 33950))) {
			return false;
	}
	return (m_wStartX <= x && x <= m_wEndX && m_wStartY <= y && y <= m_wEndY);
}

BYTE CExtractMap::accessTile(WORD x, WORD y)
{
	if (!isInMap(x, y)) {
		return 0;
	}
	else {
		return m_infoMap[(y-m_wStartY)*m_wCountX + (x-m_wStartX)];
	}
}

BYTE CExtractMap::accessOriginalTile(WORD x, WORD y)
{
	return	accessTile(x , y) & (~BITFLAG_IS_IMPASSABLE);
}

BOOL CExtractMap::isPassable(WORD x, WORD y) {

	return isPassable(x, y - 1, 4) || isPassable(x + 1, y, 6)
		|| isPassable(x, y + 1, 0) || isPassable(x - 1, y, 2);
}

BOOL CExtractMap::isPassable(WORD x, WORD y, BYTE heading) {		
	BYTE tile1, tile2, tile3, tile4;
	
	tile1 = accessTile(x, y);	

	switch (heading) {
		case 0: tile2 = accessTile(x, y - 1); break;
		case 1: tile2 = accessTile(x + 1, y - 1); break;
		case 2: tile2 = accessTile(x + 1, y); break;
		case 3: tile2 = accessTile(x + 1, y + 1); break;
		case 4: tile2 = accessTile(x, y + 1); break;
		case 5: tile2 = accessTile(x - 1, y + 1); break;
		case 6: tile2 = accessTile(x - 1, y); break;
		case 7: tile2 = accessTile(x - 1, y - 1); break;
		default: return false;
	}

	if ((tile2 & BITFLAG_IS_IMPASSABLE) == BITFLAG_IS_IMPASSABLE) {
		return false;
	}

	switch (heading) {
		case 0:{ return (tile1 & 0x02) == 0x02; }
		case 1:{ tile3 = accessTile(x, y - 1); tile4 = accessTile(x + 1, y); return (tile3 & 0x01) == 0x01 || (tile4 & 0x02) == 0x02; }
		case 2:{ return (tile1 & 0x01) == 0x01; }
		case 3:{ tile3 = accessTile(x, y + 1); return (tile3 & 0x01) == 0x01; }
		case 4:{ return (tile2 & 0x02) == 0x02; }
		case 5:{ return (tile2 & 0x01) == 0x01 || (tile2 & 0x02) == 0x02; }
		case 6:{ return (tile2 & 0x01) == 0x01; }
		case 7:{ tile3 = accessTile(x - 1, y); return (tile3 & 0x02) == 0x02; }
		default:break;
	}
	return false; 
}

BOOL CExtractMap::IsWall(int nCurPosX, int nCurPosY, int nDir)
{
	int nX = nCurPosX;
	int nY = nCurPosY;

	nX = GetLinPos(nX);
	typedef DWORD (__cdecl *_IsWall)(int, int, int);
	DWORD dwRet;
	__try
	{
		dwRet = ((_IsWall)PTR_FUNC_WALL)(nX, nY, nDir);
	}
	__except(1)
	{
		//PrintLog("[EM] PRT_FUNC_WALL error(%d, %d, %d)", nCurPosX - m_wStartX, nCurPosY - m_wStartY, nDir);
		return TRUE;
	}

	return dwRet & 0xFF;
}


void CExtractMap::ParseMapInfo()
{
#ifdef WRITEMAPIMPASSIBLEINFO
	CHAR szTest[MAX_PATH] = {0};
	CHAR szLogBuf[MAX_PATH] = {0};
	FILE *fp;

	sprintf_s(szTest, sizeof(szTest), "c:\\%d.test.dat", m_wMapNumber);
	fp = fopen(szTest, "wb");
	if (!fp) {
		PrintOptionalLog(L"ExtractMap : failed to open map test file!");
	}
#endif // WRITEMAPIMPASSIBLEINFO

	for (WORD x = m_wStartX; x <= m_wEndX; x++) {
		for (WORD y = m_wStartY; y <= m_wEndY; y++) {
			if (!isPassable(x, y)) {
				m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX)] = FALSE;
#ifdef WRITEMAPIMPASSIBLEINFO
					sprintf_s(szLogBuf, sizeof(szLogBuf), "%d,%d\xD\n", x, y);
					fwrite(szLogBuf, sizeof(CHAR), strlen(szLogBuf), fp);
#endif // WRITEMAPIMPASSIBLEINFO
			}
		}
	}

// 	for (WORD x = m_wStartX; x <= m_wEndX; x++) {
// 		for (WORD y = m_wStartY; y <= m_wEndY; y++) {
// 			if (m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX)]) {
// 				//PrintLog("[EM] (%d, %d)", x - m_wStartX, y - m_wStartY);
// 				if(y - m_wStartY - 1 >= 0 && m_pMapPassableInfo[(y - m_wStartY - 1) * m_wCountX + (x - m_wStartX)] && IsWall(x, y, 0) )
// 					m_pMapPassableInfo[(y - m_wStartY - 1) * m_wCountX + (x - m_wStartX)] = FALSE;
// 				
// 				if(y - m_wStartY + 1 < g_nMapHeight && m_pMapPassableInfo[(y - m_wStartY + 1) * m_wCountX + (x - m_wStartX)] && IsWall(x, y, 4))
// 					m_pMapPassableInfo[(y - m_wStartY + 1) * m_wCountX + (x - m_wStartX)] = FALSE;
// 				
// 				if(x - m_wStartX - 1 >= 0 && m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX - 1)] && IsWall(x, y, 6))
// 					m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX - 1)] = FALSE;
// 				
// 				if(x - m_wStartX + 1 < g_nMapHeight && m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX + 1)] && IsWall(x, y, 2))
// 					m_pMapPassableInfo[(y - m_wStartY) * m_wCountX + (x - m_wStartX + 1)] = FALSE;
// 				
// 			}
// 		}
// 	}
#ifdef WRITEMAPIMPASSIBLEINFO
	Sleep(2000);
	fclose(fp);
#endif // WRITEMAPIMPASSIBLEINFO
}
// end get map impassible info from map info

// get map info
void CExtractMap::SafeDelete()
{
	if (m_infoMap != NULL) {
		delete [] m_infoMap;
		m_infoMap = NULL;
	}
}

void CExtractMap::SafeNew(DWORD dwCount)
{
	SafeDelete();
	m_infoMap = new BYTE[dwCount];
	RtlZeroMemory(m_infoMap, dwCount);
}

CHAR myUpper(CHAR myLower)
{
	char result;
	if ((myLower>='a')&&(myLower<='z'))
		result=myLower-'a'+'A';
	else
		result=myLower;
	return result;
}

void CExtractMap::GetXY(LPSTR szFilePath)
{
	int len,i;
	int xOff,yOff;
	CHAR ch;	
	
	if ((len = strlen(szFilePath)) != 12)
		return;
	len -= 4;	
	szFilePath[len]=0;
	
	for (i=0;i<8;i++)
	{
		ch = toupper(szFilePath[i]);
		if ((ch>='0')&&(ch<='9')) continue;
		if ((ch<'A')||(ch>'F')) return;
	}
	DWORD XPos = 0;
	DWORD YPos = 0;
	xOff = 0;
	yOff = 0;

	for (i=0;i<4;i++)
	{		
		ch = toupper(szFilePath[i]);
		if (ch >= 'A'&& ch <= 'F')
			xOff = ch - '7';
		else
			xOff = ch - '0';
		XPos += xOff;
		if (i != 3)
			XPos <<= 4;		
	}
	for (i=4;i<8;i++)
	{
		ch = toupper(szFilePath[i]);
		if (ch >= 'A'&& ch <= 'F')
			yOff = ch - '7';
		else
			yOff = ch - '0';
		YPos += yOff;
		if (i != 7)
			YPos <<= 4;		
	}		
 		
	if (XPos > m_dwEndXPos) m_dwEndXPos = XPos;
	if (YPos > m_dwEndYPos) m_dwEndYPos = YPos;
	if (XPos < m_dwStartXPos) m_dwStartXPos = XPos;
	if (YPos < m_dwStartYPos) m_dwStartYPos = YPos;	
}

BOOL CExtractMap::WriteBitmapFile(char *szFileName, BYTE* lpBuff, int nWidth, int nHeight, USHORT nBitCount)
{
	WriteLog("[EM]쓰기시작");
	int nRowBytes = 0;
	int lpGapBuff[4];
	int nGap;
	int i;
	FILE *fp;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	if (nBitCount != 1 && nBitCount != 8 && nBitCount != 24) 
		return FALSE;	

	fopen_s(&fp, szFileName, "wb");
	if (fp == NULL) return FALSE;

	bfh.bfType = 19778;
	//bfh.bfSize	
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	//bfh.bfOffBits

	bih.biWidth = nWidth;
	bih.biHeight = nHeight;	
	bih.biBitCount = nBitCount;
	//bih.biClrImportant;
	//bih.biClrUsed;
	bih.biCompression = 0;
	bih.biPlanes = 1;
	bih.biSize = 40;
	//bih.biSizeImage
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;

	if (nBitCount == 1)
	{
		BYTE ind[8];		

		nRowBytes = (((nWidth + 7) / 8) + 3) / 4 * 4;
		nGap = nRowBytes - nWidth;

		bfh.bfSize = 62 + nRowBytes * nHeight;
		bfh.bfOffBits = 62;
		bih.biSizeImage = nRowBytes * nHeight;
		bih.biClrImportant = 0;
		bih.biClrUsed = 0;

		memset(ind, 0, 4);
		memset(ind + 4, 255, 4);

		fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
		fwrite(ind, sizeof(BYTE), 8, fp);

		BYTE *lpBitBuff = new BYTE[nRowBytes];

		for (i=0; i<nHeight; i++)
		{			
			register int j, k;
			int n=0;
			for(j=0; j<nRowBytes; j++)
			{
				BYTE nByte=0, nBit;
				for(k=0; k<8; k++)
				{
					if (n>=nWidth) break;
					nBit=(lpBuff[i*nWidth+n]) ? 1 : 0;
					nByte<<=1;
					nByte|=nBit;					
					n++;
				}
				nByte<<=(8-k);
				lpBitBuff[j]=nByte;
			}
			fwrite(lpBitBuff, sizeof(BYTE), nRowBytes, fp);
		}
		delete[] lpBitBuff;		
	}

	if (nBitCount == 8)
	{
		nRowBytes = (nWidth+3)/4*4;
		nGap = nRowBytes - nWidth;

		bfh.bfSize = 1078 + nRowBytes * nHeight;
		bfh.bfOffBits = 1078;
		bih.biSizeImage = nRowBytes * nHeight;
		bih.biClrImportant = 256;
		bih.biClrUsed = 256;

		BYTE ind[1024];
		for (i=0; i<1024; i++)
			ind[i]=i/4;

		fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
		fwrite(ind, sizeof(BYTE), 1024, fp);

		
// 		for(i = 0; i < nHeight; i ++)
// 			for(int j = 0; j < nWidth; j ++)
// 				lpBuff[i * nWidth + j] *= 255;

		for (i= nHeight - 1; i >= 0; i--)
		{
			for(int j = 0; j < nWidth; j ++)
			{
				BYTE byVal = 0;
				byVal = 255 * lpBuff[i * nWidth + j];
				fwrite(&byVal, 1, 1, fp);
			}
			
			fwrite(lpGapBuff, sizeof(BYTE), nGap, fp);
		}

	}
	if (nBitCount == 24)
	{
		nRowBytes = (nWidth*3+3)/4*4;
		nGap = nRowBytes - nWidth*3;

		bfh.bfSize = 54 + nRowBytes * nHeight;
		bfh.bfOffBits = 54;
		bih.biSizeImage = nRowBytes * nHeight;
		bih.biClrImportant = 0;
		bih.biClrUsed = 0;

		fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

		for (i=0; i<nHeight; i++)
		{
			fwrite(lpBuff+i*nWidth*3, sizeof(BYTE), nWidth*3, fp);
			fwrite(lpGapBuff, sizeof(BYTE), nGap, fp);
		}
	}

	fclose(fp);
	WriteLog("[EM]쓰기성공");
	return TRUE;
}


BOOL CExtractMap::ExtractMap(WORD wMapNumber)
{
	m_infoMap = NULL;
	m_wMapNumber = wMapNumber;
	
	m_dwStartXPos = 0x0FFFF;
	m_dwStartYPos = 0x0FFFF;
	m_dwEndXPos = 0;
	m_dwEndYPos = 0;
	
	char szMapDir[MAX_PATH] = {0};
	char szMapFile[MAX_PATH] = {0};
	sprintf_s(szMapDir, sizeof(szMapDir), "%s\\%d\\", m_szMapRootPath, wMapNumber);

	_finddata_t findData;
	int hFind;
	sprintf_s(szMapFile, sizeof(szMapFile), "%s*.s??", szMapDir);
	hFind = _findfirst(szMapFile, &findData);
	if (hFind != -1) {		
		if ((strcmp(findData.name,".") != 0) && (strcmp(findData.name,"..") != 0)) {

			GetXY(findData.name);
		//	PrintLog(findData.name);
		}
		while (_findnext(hFind, &findData) == 0) {
			if ((strcmp(findData.name,".") != 0) && (strcmp(findData.name,"..") != 0)) {
				GetXY(findData.name);
		//		PrintLog(findData.name);
			}
		}		
	}
	else {
		_findclose(hFind);
		WriteLog("[EM]맵파일을 찾지 못했습니다! (%s)", szMapDir);
		return FALSE;
	}
	_findclose(hFind);

	m_wStartX = (WORD)((m_dwStartXPos - 0x7E00) << 6);
	m_wStartY = (WORD)((m_dwStartYPos - 0x7E00) << 6);

	m_wCountX = (WORD)((m_dwEndXPos - m_dwStartXPos + 1) << 6);
	m_wCountY = (WORD)((m_dwEndYPos - m_dwStartYPos + 1) << 6);

	m_wEndX   = (WORD)(m_wStartX + m_wCountX - 1);
	m_wEndY   = (WORD)(m_wStartY + m_wCountY - 1);
	
	
	SafeNew(m_wCountX*m_wCountY);
	
	g_nMapWidth = m_wCountX;
	g_nMapHeight = m_wCountY;

	if(g_pMapPassableInfo)
		free(g_pMapPassableInfo);
	g_pMapPassableInfo = (BOOLEAN *)malloc(g_nMapWidth * g_nMapHeight);
	
	if(g_pMapBuffer)
		free(g_pMapBuffer);
	g_pMapBuffer = (BYTE *)malloc(g_nMapWidth * g_nMapHeight);
	
	
	if(g_pRoadBuffer)
		free(g_pRoadBuffer);
	g_pRoadBuffer = (UINT *)malloc(g_nMapWidth * g_nMapHeight * sizeof(UINT));

	memset(g_pMapPassableInfo, 1, g_nMapWidth * g_nMapHeight);
	memset(g_pRoadBuffer, 0, g_nMapWidth * g_nMapHeight);
	memset(g_pMapBuffer, 0, g_nMapWidth * g_nMapHeight);
	
	m_pMapPassableInfo = g_pMapPassableInfo;
	g_nMapStartX = m_wStartX;
	g_nMapStartY = m_wStartY;
	g_nMapEndX = m_wEndX;
	g_nMapEndY = m_wEndY;

// 	PrintLog("[맵]g_pMapPassableInfo = %x", (DWORD)g_pMapPassableInfo);
// 	PrintLog("[맵]g_pRoadBuffer = %x", (DWORD)g_pRoadBuffer);
// 	PrintLog("[맵]g_pMapBuffer = %x", (DWORD)g_pMapBuffer);
// 	PrintLog("[맵]시작위치(%d, %d)", m_wStartX, m_wStartY);
// 	PrintLog("[맵]width = %d, height = %d", g_nMapWidth, g_nMapHeight);

	CHAR szFileName[MAX_PATH] = {0};
	FILE *fp;
	DWORD xNum,yNum;

	BYTE var2A9, var29D, var28D, byFlag;
	DWORD k,offset;

	for (DWORD dwYPos = m_dwStartYPos; dwYPos <= m_dwEndYPos; dwYPos++) {
		for (DWORD dwXPos = m_dwStartXPos; dwXPos <= m_dwEndXPos; dwXPos++) {
			sprintf_s(szFileName, sizeof(szFileName), "%s%4x%4x.s32", szMapDir, dwXPos, dwYPos);
			fopen_s(&fp, szFileName,"rb");
			if (fp == NULL) {
				sprintf_s(szFileName, sizeof(szFileName), "%s%4x%4x.seg", szMapDir, dwXPos, dwYPos);
				fopen_s(&fp, szFileName,"rb");
				if (fp == NULL) {
					for (int y = 0; y < 0x40; y++) {
						for (int x = 0; x < 0x40; x++)
						{
							xNum = dwXPos - m_dwStartXPos;
							yNum = dwYPos - m_dwStartYPos;
							xNum = (DWORD)(xNum<<6) + x;
							yNum = (DWORD)(yNum<<6) + y;
							m_infoMap[yNum * m_wCountX + xNum] = 0x80;
						}
					}
				}
				else {
					fseek(fp, 0x4000, SEEK_SET);					
					fscanf_s(fp,"%c", &var2A9);
					k = var2A9*4 + 0x4002;
					fscanf_s(fp, "%c", &var2A9);
					offset = (DWORD)(var2A9 << 10) + k;
					fseek(fp, offset, SEEK_SET);

					for(int y = 0; y < 0x40; y++) {
						for (int x = 0; x <0x40; x++) {
							fscanf_s(fp, "%c", &var2A9);							
							fscanf_s(fp, "%c", &var29D);
							byFlag = 0;

							if (var2A9 & 4)	{
								byFlag = 0x10;
							}
							else if (var2A9 & 8) {
								byFlag = 0x20;
							}

							if (!(var2A9 & 1)) {
								byFlag += 10;								
							}
							else if (var2A9 & 0x40) {
								byFlag += 8;								
							}

							if (!(var29D & 1)) {
								byFlag += 5;								
							}
							else if (var29D & 0x40) {
								byFlag += 4;
							}

							xNum = dwXPos - m_dwStartXPos;
							yNum = dwYPos - m_dwStartYPos;
							xNum = (DWORD)(xNum << 6) + x;
							yNum = (DWORD)(yNum << 6) + y;
							m_infoMap[yNum * m_wCountX + xNum] = byFlag;
						}						
					}
					fclose(fp);
				}				
			} // end first if fp == 0
			else { // .S32
				fseek(fp, 0x8000, SEEK_SET);
				fscanf_s(fp, "%c", &var2A9);
				k = var2A9*6 + 0x8002;
				fscanf_s(fp, "%c", &var2A9);
				offset = var2A9 * 3;
				offset = (DWORD)(offset<<9) + k;
				fseek(fp, offset, SEEK_SET);

				for(int y=0; y < 0x40; y++) {
					for(int x = 0; x < 0x40; x++) {
						fscanf_s(fp, "%c", &var2A9);						
						fscanf_s(fp, "%c", &var28D);
						fscanf_s(fp, "%c", &var29D);
						fscanf_s(fp, "%c", &var28D);
						byFlag = 0;

						if (var2A9 & 4)	{
							byFlag = 0x10;
						}
						else if (var2A9 & 8) {
							byFlag = 0x20;
						}

						if (!(var2A9 & 1)) {
							byFlag += 10;								
						}
						else if (var2A9 & 0x40) {
							byFlag += 8;								
						}

						if (!(var29D & 1)) {
							byFlag += 5;								
						}
						else if (var29D & 0x40) {
							byFlag += 4;
						}

						xNum = dwXPos - m_dwStartXPos;
						yNum = dwYPos - m_dwStartYPos;
						xNum = (DWORD)(xNum << 6) + x;
						yNum = (DWORD)(yNum << 6) + y;
						m_infoMap[yNum * m_wCountX + xNum] = byFlag;
					}
				}
				fclose(fp);
			}
		} 
	}

#ifdef	WRITEHEADER
	FILE *fp_header;
	CHAR strBuf[MAX_PATH] = {0};

	fp_header = fopen("c:\header.dat", "wb");
	if (fp_header) {
		sprintf(strBuf,"unsigned char _map_%i_[%i][%i]={",m_wMapNumber, m_wCountY, m_wCountX);
		fprintf(fp_header,"%s",strBuf);

		for(int y=0;y<m_wCountY;y++)
		{
			fprintf(fp_header,"{");

			for (int x=0;x<m_wCountX;x++)
			{
				fprintf(fp_header,"%u",m_infoMap[y*m_wCountX+x]);
				if (x==m_wCountX-1)
					fprintf(fp_header,"}\n");
				else
					fprintf(fp_header,",");
			}
			if (y==m_wCountY-1)
				fprintf(fp_header,"};\n");
			else
				fprintf(fp_header,",");	
		}
	}
#endif // WRITEHEADER

	ParseMapInfo();

	if(wMapNumber == 9101)
	{
		int nX, nY;
		nX = 32789 + 2 - g_nMapStartX;
		nY = 32809 - g_nMapStartY;
		g_pMapPassableInfo[nY * g_nMapWidth + nX] = FALSE;
		
		WriteLog("[EM] nx = %d, ny = %d", nX, nY);
		nX = 32789 + 3 - g_nMapStartX;
		g_pMapPassableInfo[nY * g_nMapWidth + nX] = FALSE;
		WriteLog("[EM] nx = %d, ny = %d", nX, nY);

		nX = 32789 + 13 - g_nMapStartX;
		g_pMapPassableInfo[nY * g_nMapWidth + nX] = FALSE;
		WriteLog("[EM] nx = %d, ny = %d", nX, nY);

		nX = 32789 + 14 - g_nMapStartX;
		g_pMapPassableInfo[nY * g_nMapWidth + nX] = FALSE;
		WriteLog("[EM] nx = %d, ny = %d", nX, nY);
	}

	if(wMapNumber == 0)
	{
		g_pMapPassableInfo[183 * g_nMapWidth + 366] = FALSE;
		g_pMapPassableInfo[184 * g_nMapWidth + 365] = FALSE;
		g_pMapPassableInfo[190 * g_nMapWidth + 365] = FALSE;
		g_pMapPassableInfo[192 * g_nMapWidth + 365] = FALSE;
		g_pMapPassableInfo[194 * g_nMapWidth + 365] = FALSE;
		g_pMapPassableInfo[173 * g_nMapWidth + 349] = FALSE;
	}
	SafeDelete();
//  	if(WriteBitmapFile("c:\\mapDungeon.bmp", m_pMapPassableInfo, g_nMapWidth, g_nMapHeight, 8))
// 		PrintLog("[맵]파일쓰기성공");
// 	else
// 		PrintLog("[맵]파일쓰기실패");
	
	WriteLog("[EM]초기화완료");
	return TRUE;
}
// end get map info