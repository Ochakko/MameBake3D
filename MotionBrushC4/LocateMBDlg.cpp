#include "pch.h"
// LocateMBDlg.cpp : CLocateMBDlg のインプリメンテーション
//#include "stdafx.h"

#include "LocateMBDlg.h"
#include <Commdlg.h>


/////////////////////////////////////////////////////////////////////////////
// CLocateMBDlg

CLocateMBDlg::CLocateMBDlg()
{
	m_w2 = TRUE;
	m_h2 = TRUE;
}

CLocateMBDlg::~CLocateMBDlg()
{
}


LRESULT CLocateMBDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	SetWnd();
	ParamsToDlg();

#define MAINMENUAIMBARH		32

	int totalwndwidth = (1216 + 450) * 2;
	int totalwndheight = (950 - MAINMENUAIMBARH) * 2;

	int dlgposx = 0;
	int dlgposy = 0;

	HWND desktopwnd;
	desktopwnd = ::GetDesktopWindow();
	if (desktopwnd) {
		RECT desktoprect;
		::GetClientRect(desktopwnd, &desktoprect);
		//if ((desktoprect.right >= (totalwndwidth * 2)) && (desktoprect.bottom >= ((totalwndheight - MAINMENUAIMBARH) * 2))) {
		//}

		dlgposx = (desktoprect.left + desktoprect.right) / 2;
		dlgposy = (desktoprect.top + desktoprect.bottom) / 2;
	}
	::MoveWindow(m_dlg_wnd, dlgposx, dlgposy, 157 * 2, 96 * 2, TRUE);

	return 1;  // システムにフォーカスを設定させます
}

LRESULT CLocateMBDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_w2 = (BOOL)IsDlgButtonChecked(IDC_CHECK1);
	m_h2 = (BOOL)IsDlgButtonChecked(IDC_CHECK2);

	EndDialog(wID);

	return 0;
}

LRESULT CLocateMBDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

void CLocateMBDlg::SetWnd()
{
	m_dlg_wnd = this->m_hWnd;

	m_w2_wnd = GetDlgItem( IDC_CHECK1 );
	m_h2_wnd = GetDlgItem( IDC_CHECK2 );
}

int CLocateMBDlg::ParamsToDlg()
{	
	m_dlg_wnd.CheckDlgButton(IDC_CHECK1, (UINT)m_w2);
	m_dlg_wnd.CheckDlgButton(IDC_CHECK2, (UINT)m_h2);

	return 0;
}
