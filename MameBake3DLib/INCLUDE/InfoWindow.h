#pragma once
#ifndef INFOWINDOWH
#define INFOWINDOWH

#include <stdio.h>

#include <windows.h>
#include <wchar.h>
#include <coef.h>

#include <OrgWindow.h>

#define INFOWINDOWLINEW	7000
#define INFOWINDOWLINEH	4

class CInfoWindow
{
public:
	CInfoWindow();
	~CInfoWindow();

	int CreateInfoWindow(HWND srcparentwnd, int srcposx, int srcposy, int srcwidth, int srcheight);
	int OutputInfo(WCHAR* lpFormat, ...);


	void UpdateWindow();
	void OnPaint();

private:
	void InitParams();
	void DestroyObjs();

private:
	HWND m_hWnd;
	HWND m_hParentWnd;
	OrgWinGUI::HDCMaster* m_hdcM;
	WCHAR m_stroutput[INFOWINDOWLINEH][INFOWINDOWLINEW];
	int m_outputindex;
	bool m_isfirstoutput;
};

#endif