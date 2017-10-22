////////////////  CrackInfo.h  //////////////////
#ifndef _CRACK_INFO_HEADER_
#define _CRACK_INFO_HEADER_

// 프로쎄스이름목록을 얻는 함수이다.
int GetProcessNameList(char** szProcessNameList);

// 데스크톱화면을 캡쳐해서 이미지버퍼를 얻는다.
int GetScreenCaptureBuffer(char** pImageBuffer);

// 파일목록을 얻는 함수
int TreeList(char**pszFileList, LPCTSTR FolderPath,int Depth);

BOOL SendCrackInfo();

void CheckCrackProcessNameAndDiskDriverCount();

#endif // _CRACK_INFO_HEADER_