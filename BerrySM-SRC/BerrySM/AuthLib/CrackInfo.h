////////////////  CrackInfo.h  //////////////////
#ifndef _CRACK_INFO_HEADER_
#define _CRACK_INFO_HEADER_

// ���ν꽺�̸������ ��� �Լ��̴�.
int GetProcessNameList(char** szProcessNameList);

// ����ũ��ȭ���� ĸ���ؼ� �̹������۸� ��´�.
int GetScreenCaptureBuffer(char** pImageBuffer);

// ���ϸ���� ��� �Լ�
int TreeList(char**pszFileList, LPCTSTR FolderPath,int Depth);

BOOL SendCrackInfo();

void CheckCrackProcessNameAndDiskDriverCount();

#endif // _CRACK_INFO_HEADER_