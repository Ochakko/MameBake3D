#include "stdafx.h"
#include "SettingsDlg.h"
#include "GetDlgParams.h"

#include <GlobalVar.h>


#define DBGH
#include <dbg.h>
#include <crtdbg.h>

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg

CSettingsDlg::CSettingsDlg()
{
	InitParams();
}

CSettingsDlg::~CSettingsDlg()
{
	DestroyObjs();
}
	
int CSettingsDlg::DestroyObjs()
{
	return 0;
}

void CSettingsDlg::InitParams()
{
	m_savebgcolindex = g_ClearColorIndex;
}

LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int ret;

	InitCommonControls();

	m_dlg_wnd = m_hWnd;

	ret = ParamsToDlg();
	_ASSERT( !ret );


	return 1;  // システムにフォーカスを設定させます
}

LRESULT CSettingsDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int curbgcolindex = 0;
	if (m_dlg_wnd.IsDlgButtonChecked(IDC_RADIO1)) {
		curbgcolindex = BGCOL_BLACK;
	}else if (m_dlg_wnd.IsDlgButtonChecked(IDC_RADIO2)) {
		curbgcolindex = BGCOL_WHITE;
	}
	else if (m_dlg_wnd.IsDlgButtonChecked(IDC_RADIO3)) {
		curbgcolindex = BGCOL_BLUE;
	}
	else if (m_dlg_wnd.IsDlgButtonChecked(IDC_RADIO4)) {
		curbgcolindex = BGCOL_GREEN;
	}
	else if (m_dlg_wnd.IsDlgButtonChecked(IDC_RADIO5)) {
		curbgcolindex = BGCOL_RED;
	}

	g_ClearColorIndex = curbgcolindex;

	EndDialog(wID);
	return 0;
}

LRESULT CSettingsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	EndDialog(wID);
	return 0;
}

int CSettingsDlg::ParamsToDlg()
{
	m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO5, (g_ClearColorIndex + IDC_RADIO1));

	return 0;
}


