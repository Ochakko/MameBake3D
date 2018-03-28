// FilterDlg.cpp : CFilterDlg のインプリメンテーション
//#include "stdafx.h"

#include "FilterDlg.h"
#include <Commdlg.h>
#include <FilterType.h>

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg

CFilterDlg::CFilterDlg()
	: m_filtertype_wnd(0)
{
	m_filtertype = AVGF_NONE;
	m_filtersize = avgfsize[0];
}

CFilterDlg::~CFilterDlg()
{
}

LRESULT CFilterDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetWnd();
	SetCombo();
	ParamsToDlg();

	return 1;  // システムにフォーカスを設定させます
}

LRESULT CFilterDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
///
	int combono;
	combono = (int)SendMessage( m_filtertype_wnd, CB_GETCURSEL, 0, 0);
	if( combono == CB_ERR ){
		return 0;
	}
	m_filtertype = combono;

	combono = (int)SendMessage( m_filtersize_wnd, CB_GETCURSEL, 0, 0);
	if( combono == CB_ERR ){
		return 0;
	}
	m_filtersize = combono;


	EndDialog(wID);

	return 0;
}

LRESULT CFilterDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

void CFilterDlg::SetWnd()
{
	m_filtertype_wnd = GetDlgItem( IDC_FILTERTYPE );
	m_filtersize_wnd = GetDlgItem( IDC_FILTERSIZE );
}

int CFilterDlg::ParamsToDlg()
{	
	SendMessage( m_filtertype_wnd, CB_SETCURSEL, m_filtertype, 0);
	SendMessage( m_filtersize_wnd, CB_SETCURSEL, m_filtersize, 0);

	return 0;
}

int CFilterDlg::SetCombo()
{
	int fno;
	for( fno = 0; fno < AVGF_MAX; fno++ ){
		SendMessage( m_filtertype_wnd, CB_ADDSTRING, 0, (LPARAM)&stravgf[fno][0]);
	}
	SendMessage( m_filtertype_wnd, CB_SETCURSEL, 0, 0);

	for( int i=0; i<10; i++ ){
		SendMessage( m_filtersize_wnd, CB_ADDSTRING, 0, (LPARAM)&stravgfsize[i][0]);
	}
	SendMessage( m_filtersize_wnd, CB_SETCURSEL, 0, 0);

	return 0;
}
