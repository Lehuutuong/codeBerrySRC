// MsgCommand.cpp: implementation of the CMsgCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MsgCommand.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMsgCommand::CMsgCommand()
{

}

CMsgCommand::~CMsgCommand()
{

}
void CMsgCommand::MouseUp(HWND hWnd, int x, int y)
{
	SendNotifyMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}
void CMsgCommand::MouseDown(HWND hWnd, int x, int y)
{
	SendNotifyMessage(hWnd, WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
}

void CMsgCommand::MouseClick(HWND hWnd, int x, int y)
{
	CMsgCommand::MouseDown(hWnd, x, y);
	CMsgCommand::MouseUp(hWnd, x, y);
}

void CMsgCommand::MouseDblClick(HWND hWnd, int x, int y)
{
	SendNotifyMessage(hWnd, WM_LBUTTONDBLCLK, 0, MAKELPARAM(x, y));
}

void CMsgCommand::ShiftMouseClick(HWND hWnd, int x, int y)
{
	SendNotifyMessage(hWnd, WM_LBUTTONDOWN, MK_SHIFT | MK_LBUTTON, MAKELPARAM(x, y));
	SendNotifyMessage(hWnd, WM_LBUTTONUP, MK_SHIFT, MAKELPARAM(x, y));
}

#define KEY_DELAY 100
void KEYUP(HWND hWnd, BYTE bKey)
{
	SendNotifyMessage(hWnd, WM_KEYUP, bKey, 0x80000000);
}
void KEYDOWN(HWND hWnd, BYTE bKey, BOOL bChar)
{
	SendNotifyMessage(hWnd, WM_KEYDOWN, bKey, 0x00000000);
	if (bChar)
		SendNotifyMessage(hWnd, WM_CHAR, bKey, 0x00000000);
}

#define KEYTYPING_EVENT(hWnd, bKey, bKeyChar)		\
	KEYDOWN(hWnd, (BYTE)(bKey), FALSE);	\
	SendNotifyMessage(hWnd, WM_CHAR, (BYTE)bKeyChar, 0x00000000);\
	Sleep(KEY_DELAY);					\
	KEYUP(hWnd, (BYTE)(bKey));		\
	Sleep(KEY_DELAY);

void CMsgCommand::SysKeyPress(HWND hWnd, BYTE bKey)
{
	SendNotifyMessage(hWnd, WM_KEYDOWN, bKey, 0x00000000);
	Sleep(KEY_DELAY);					
	SendNotifyMessage(hWnd, WM_KEYUP, bKey, 0x80000000);
	Sleep(KEY_DELAY);					
}

void CMsgCommand::KeyDown(HWND hWnd, BYTE bKey)
{
	KEYDOWN(hWnd, bKey, FALSE);
}

void CMsgCommand::KeyUp(HWND hWnd,BYTE bKey)
{
	KEYUP(hWnd, bKey);
}

void CMsgCommand::KeyPress(HWND hWnd,BYTE bKey,BYTE bKeyChar)
{
	KEYTYPING_EVENT(hWnd, bKey, bKeyChar);
}
