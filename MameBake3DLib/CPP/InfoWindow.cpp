#include "stdafx.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <windows.h>

#include <InfoWindow.h>

#include <crtdbg.h>
#define DBGH
#include <dbg.h>


LRESULT CALLBACK InfoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int InitializeInfoWindow(CREATESTRUCT* createWindowArgs);

static CInfoWindow* s_contextwin = 0;

CInfoWindow::CInfoWindow()
{
	InitParams();
}
CInfoWindow::~CInfoWindow()
{
	DestroyObjs();
}
void CInfoWindow::InitParams()
{
	s_contextwin = 0;
	m_isfirstoutput = true;

	m_hWnd = NULL;
	m_hParentWnd = NULL;
	m_hdcM = 0;

	ZeroMemory(&(m_stroutput[0][0]), sizeof(WCHAR) * INFOWINDOWLINEW * INFOWINDOWLINEH);
	m_outputindex = 0;

}
void CInfoWindow::DestroyObjs()
{
	if (m_hdcM) {
		delete m_hdcM;
		m_hdcM = 0;
	}
}


int CInfoWindow::CreateInfoWindow(HWND srcparentwnd, int srcposx, int srcposy, int srcwidth, int srcheight)
{
	//ウィンドウクラスを登録
	WNDCLASSEX wcex;
	ZeroMemory((LPVOID)&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = InfoWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"InfoWindow_";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);



	//ウィンドウ作成
	int cxframe = GetSystemMetrics(SM_CXFRAME);
	int cyframe = GetSystemMetrics(SM_CYFRAME);
	//if (istopmost) {
	//	hWnd = CreateWindowEx(//WS_EX_TOOLWINDOW|WS_EX_TOPMOST,szclassName,title,WS_POPUP,
	//		WS_EX_LEFT | WS_EX_TOPMOST, szclassName, title, WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
	//		pos.x, pos.y,
	//		size.x, size.y,
	//		hWndParent, NULL, hInstance, NULL);
	//}
	//else {
	m_hWnd = CreateWindowEx(
		WS_EX_LEFT, L"InfoWindow_", L"InfoWindow", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_THICKFRAME,
		srcposx, srcposy,
		srcwidth, srcheight - 2 * cyframe,
		srcparentwnd, NULL, (HINSTANCE)GetModuleHandle(NULL), NULL);
	//}
	//SetParent(s_infownd, s_mainhwnd);

	if (m_hWnd) {
		m_hdcM = new OrgWinGUI::HDCMaster();
		if (m_hdcM) {
			m_hdcM->setHWnd(m_hWnd);

			//ウィンドウ表示
			ShowWindow(m_hWnd, SW_SHOW);

			s_contextwin = this;

			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		return 1;
	}
}

int CInfoWindow::OutputInfo(WCHAR* lpFormat, ...)
{
	if (!m_hWnd) {
		return 0;
	}
	if (!IsWindow(m_hWnd)) {
		return 0;
	}

	int ret;
	va_list Marker;
	unsigned long wleng, writeleng;
	WCHAR outchar[INFOWINDOWLINEW];

	ZeroMemory(outchar, sizeof(WCHAR) * INFOWINDOWLINEW);

	va_start(Marker, lpFormat);
	ret = vswprintf_s(outchar, INFOWINDOWLINEW, lpFormat, Marker);
	va_end(Marker);

	if (ret < 0)
		return 1;

	if (!m_isfirstoutput) {
		m_outputindex++;
		if (m_outputindex >= INFOWINDOWLINEH) {
			m_outputindex = 0;
		}
	}

	//wleng = (unsigned long)wcslen(outchar);
	wcscpy_s(&(m_stroutput[m_outputindex][0]), INFOWINDOWLINEW, outchar);

	m_isfirstoutput = false;

	//WriteFile(dbgfile, outchar, sizeof(WCHAR) * wleng, &writeleng, NULL);

	return 0;
}

void CInfoWindow::UpdateWindow()
{
	if (m_hWnd && IsWindow(m_hWnd)) {
		RECT clirect;
		GetClientRect(m_hWnd, &clirect);
		InvalidateRect(m_hWnd, &clirect, true);
		::UpdateWindow(m_hWnd);
	}
}

void CInfoWindow::OnPaint()
{
	if (m_hWnd && IsWindow(m_hWnd)) {

		RECT clirect;
		GetClientRect(m_hWnd, &clirect);

		m_hdcM->beginPaint();
		m_hdcM->setPenAndBrush(RGB(70, 50, 70), RGB(70, 50, 70));
		Rectangle(m_hdcM->hDC, 0, 0, clirect.right, clirect.bottom);
		m_hdcM->setFont(12, _T("ＭＳ ゴシック"));
		SetTextColor(m_hdcM->hDC, RGB(255, 255, 255));
		
		//TextOut(m_hdcM->hDC, 10, 2, outchar, (int)wcslen(outchar));

		//INFOWINDOWLINEH行分、古い順に描画する
		int outputno;
		int curindex = m_outputindex + INFOWINDOWLINEH;
		while(curindex >= INFOWINDOWLINEH) {
			curindex -= INFOWINDOWLINEH;
		}
		if (curindex >= 0) {
			for (outputno = 0; outputno < INFOWINDOWLINEH; outputno++) {
				TextOut(m_hdcM->hDC, 5, 5 + 15 * outputno, &(m_stroutput[curindex][0]), (int)wcslen(&(m_stroutput[curindex][0])));
				curindex--;
				if (curindex < 0) {
					curindex = INFOWINDOWLINEH - 1;
				}
			}
		}
		m_hdcM->endPaint();
	}
}

LRESULT CALLBACK InfoWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
		//case WM_TIMER:
		//	OnTimerFunc(wParam);
		//	break;
	case WM_DESTROY:
		//PostQuitMessage(0);
		return 0;
		break;
	case WM_CREATE:
		return InitializeInfoWindow((CREATESTRUCT*)lParam);
		break;
		//case WM_COMMAND:
		//{
		//	if ((menuid >= 59900) && (menuid <= (59900 + MAXMOTIONNUM))) {
		//		ActivatePanel(0);
		//		int selindex = menuid - 59900;
		//		OnAnimMenu(selindex);
		//		ActivatePanel(1);
		//		return 0;
		//	}
		//}
		//break;
	case WM_PAINT:				//描画
		if (s_contextwin) {
			s_contextwin->OnPaint();
		}
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

int InitializeInfoWindow(CREATESTRUCT* createWindowArgs)
{
	//TCHAR message[1024];
	//int messageResult;
	//wsprintf(message,
	//	TEXT("ウィンドウクラス:%s\nタイトル:%s\nウィンドウを生成しますか？"),
	//	createWindowArgs->lpszClass, createWindowArgs->lpszName
	//);

	//messageResult = MessageBox(NULL, message, TEXT("確認"), MB_YESNO | MB_ICONINFORMATION);

	//if (messageResult == IDNO)
	//	return -1;
	return 0;
}
