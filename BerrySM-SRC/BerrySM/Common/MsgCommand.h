// MsgCommand.h: interface for the CMsgCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGCOMMAND_H__AD4C1E84_0662_4F4B_A81E_4B44B0153E14__INCLUDED_)
#define AFX_MSGCOMMAND_H__AD4C1E84_0662_4F4B_A81E_4B44B0153E14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsgCommand  
{
public:
	CMsgCommand();
	virtual ~CMsgCommand();

	static void MouseDown(HWND hWnd, int x, int y);
	static void MouseUp(HWND hWnd, int x, int y);
	static void MouseClick(HWND hWnd, int x, int y);
	static void MouseDblClick(HWND hWnd, int x, int y);
	static void ShiftMouseClick(HWND hWnd, int x, int y);

	static void  SysKeyPress(HWND hWnd, BYTE bKey);
	static void	 KeyPress(HWND hWnd, BYTE bKey, BYTE bKeyChar);
	static void	 KeyDown(HWND hWnd, BYTE bKey);
	static void	 KeyUp(HWND hWnd, BYTE bKey);
};

#endif // !defined(AFX_MSGCOMMAND_H__AD4C1E84_0662_4F4B_A81E_4B44B0153E14__INCLUDED_)
