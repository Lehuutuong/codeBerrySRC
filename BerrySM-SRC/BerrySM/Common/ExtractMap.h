// ExtractMap.h

#pragma once

class CExtractMap
{
public:
	CExtractMap(void);
	virtual ~CExtractMap(void);

private:
	BYTE*		m_infoMap;
	BOOLEAN*	m_pMapPassableInfo;
	CHAR		m_szMapRootPath[MAX_PATH];
	WORD		m_wMapNumber;
	WORD		m_wStartX;
	WORD		m_wStartY;
	WORD		m_wEndX;
	WORD		m_wEndY;
	WORD		m_wCountX;
	WORD		m_wCountY;
	DWORD		m_dwStartXPos;
	DWORD		m_dwStartYPos;
	DWORD		m_dwEndXPos;
	DWORD		m_dwEndYPos;

	BOOL		isInMap(WORD x, WORD y);
	BYTE		accessTile(WORD x, WORD y);
	BYTE		accessOriginalTile(WORD x, WORD y);
	BOOL		isPassable(WORD x, WORD y);
	BOOL		isPassable(WORD x, WORD y, BYTE heading);
	void		ParseMapInfo();
	BOOL		IsWall(int nCurPosX, int nCurPosY, int nDir);
	void		SafeDelete();
	void		SafeNew(DWORD dwCount);
	void		GetXY(LPSTR szFilePath);
	

public:
	BOOL		WriteBitmapFile(char *szFileName, BYTE* lpBuff, int nWidth, int nHeight, USHORT nBitCount);
	BOOL		Initialize();
	BOOL		ExtractMap(WORD wMapNumber);
};