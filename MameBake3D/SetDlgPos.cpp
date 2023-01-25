#include "stdafx.h"

#include <windows.h>

#define DECSETDLGPOSH
#include "SetDlgPos.h"

#define DBGH
#include <dbg.h>
#include <crtdbg.h>

using namespace std;


int SetDlgPosDesktopCenter(HWND hDlgWnd, HWND hWndInsertAfter)
{
	int posx = 0;
	int posy = 0;

	//POINT ptCursor;
	//GetCursorPos(&ptCursor);
	////::ScreenToClient(s_3dwnd, &ptCursor);

	HWND desktopwnd;
	desktopwnd = ::GetDesktopWindow();
	if (desktopwnd) {
		RECT desktoprect;
		::GetClientRect(desktopwnd, &desktoprect);
		posx = (desktoprect.left + desktoprect.right) / 2;
		posy = (desktoprect.top + desktoprect.bottom) / 2;
	}
	else {
		posx = 0;
		posy = 0;
	}

	SetWindowPos(hDlgWnd, hWndInsertAfter, posx, posy, 0, 0, SWP_NOSIZE);

	//RECT dlgrect;
	//GetWindowRect(fgwnd, &dlgrect);
	//SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
	


	//2023/01/25
	//この関数は　指定しているウインドウがフォアグランドになったときにも呼ばれる
	//機能のボタンを押した際　マウスがアップする前に呼ばれることがある
	//マウスを動かすと　ボタンが機能しないことがあるので　SetCursorPosはコメントアウト
	//SetCursorPos(posx + 25, posy + 10);


	return 0;
}

int SetDlgPosCursorPos(HWND hDlgWnd, HWND hWndInsertAfter)
{
	int posx = 0;
	int posy = 0;

	POINT ptCursor;
	GetCursorPos(&ptCursor);
	////::ScreenToClient(s_3dwnd, &ptCursor);

	posx = ptCursor.x;
	posy = ptCursor.y;

	SetWindowPos(hDlgWnd, hWndInsertAfter, posx, posy, 0, 0, SWP_NOSIZE);

	return 0;
}



