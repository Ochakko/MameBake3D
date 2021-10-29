#include "stdafx.h"
#include "CopyHistoryDlg.h"
#include "GetDlgParams.h"

#include <GlobalVar.h>


#define DBGH
#include <dbg.h>
#include <crtdbg.h>

/////////////////////////////////////////////////////////////////////////////
// CCopyHistoryDlg

CCopyHistoryDlg::CCopyHistoryDlg()
{
	InitParams();
}

CCopyHistoryDlg::~CCopyHistoryDlg()
{
	DestroyObjs();
}
	
int CCopyHistoryDlg::DestroyObjs()
{
	return 0;
}

void CCopyHistoryDlg::InitParams()
{
	m_ischeckedmostrecent = true;
	ZeroMemory(m_selectname, sizeof(WCHAR) * MAX_PATH);

	m_namenum = 0;
	m_copyhistory.clear();


	m_ctrlid[0] = IDC_RADIO1;
	m_ctrlid[1] = IDC_RADIO2;
	m_ctrlid[2] = IDC_RADIO3;
	m_ctrlid[3] = IDC_RADIO4;
	m_ctrlid[4] = IDC_RADIO5;
	m_ctrlid[5] = IDC_RADIO6;
	m_ctrlid[6] = IDC_RADIO7;
	m_ctrlid[7] = IDC_RADIO8;
	m_ctrlid[8] = IDC_RADIO9;
	m_ctrlid[9] = IDC_RADIO10;
	m_ctrlid[10] = IDC_RADIO11;
	m_ctrlid[11] = IDC_RADIO12;
	m_ctrlid[12] = IDC_RADIO13;
	m_ctrlid[13] = IDC_RADIO14;
	m_ctrlid[14] = IDC_RADIO15;
	m_ctrlid[15] = IDC_RADIO16;
	m_ctrlid[16] = IDC_RADIO17;
	m_ctrlid[17] = IDC_RADIO18;
	m_ctrlid[18] = IDC_RADIO19;
	m_ctrlid[19] = IDC_RADIO20;

}

LRESULT CCopyHistoryDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int ret;

	InitCommonControls();

	m_dlg_wnd = m_hWnd;

	ret = ParamsToDlg();
	_ASSERT( !ret );


	return 1;  // システムにフォーカスを設定させます
}

LRESULT CCopyHistoryDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int selectedno = 0;//チェックされていない場合(あり得ないが)、一番最初
	int nameno;
	for (nameno = 0; nameno < m_namenum; nameno++) {
		if (m_dlg_wnd.IsDlgButtonChecked(m_ctrlid[nameno])) {
			selectedno = nameno;
			break;
		}
	}

	if ((selectedno >= 0) && (selectedno < m_namenum)) {
		wcscpy_s(m_selectname, MAX_PATH, m_copyhistory[selectedno].wfilename);
		m_selectname[MAX_PATH - 1] = 0L;
	}

	m_ischeckedmostrecent = m_dlg_wnd.IsDlgButtonChecked(IDC_CHECK1);


	EndDialog(wID);
	return 0;
}

LRESULT CCopyHistoryDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	EndDialog(wID);
	return 0;
}

int CCopyHistoryDlg::ParamsToDlg()
{
	//m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO5, (g_ClearColorIndex + IDC_RADIO1));
	
	bool ischeck = false;
	int nameno;
	for (nameno = 0; nameno < m_namenum; nameno++) {
		WCHAR dispname[MAX_PATH] = { 0L };
		WCHAR* pen;
		pen = wcsrchr(m_copyhistory[nameno].wfilename, TEXT('_'));
		if (pen) {
			wcscpy_s(dispname, MAX_PATH, pen + 1);
		}
		else {
			wcscpy_s(dispname, MAX_PATH, m_copyhistory[nameno].wfilename);
		}

		m_dlg_wnd.SetDlgItemTextW(m_ctrlid[nameno], dispname);


		if ((m_selectname[0] != 0L) && (wcscmp(m_selectname, m_copyhistory[nameno].wfilename) == 0)) {
			m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO20, m_ctrlid[nameno]);
			ischeck = true;
		}
	}
	int nameno2;
	for (nameno2 = m_namenum; nameno2 < 20; nameno2++) {
		CWindow curwnd2 = m_dlg_wnd.GetDlgItem(m_ctrlid[nameno2]);
		if (curwnd2.IsWindow()) {
			curwnd2.SetWindowTextW(L"no more.");
		}
	}
	if (ischeck == false) {
		m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO20, IDC_RADIO1);
	}


	if (m_ischeckedmostrecent) {
		m_dlg_wnd.CheckDlgButton(IDC_CHECK1, BST_CHECKED);
	}
	else {
		m_dlg_wnd.CheckDlgButton(IDC_CHECK1, BST_UNCHECKED);
	}

	return 0;
}

int CCopyHistoryDlg::SetNames(std::vector<HISTORYELEM>& copyhistory)
{
	m_copyhistory = copyhistory;

	m_namenum = min(20, m_copyhistory.size());
	
	//ParamsToDlg();//表示されていないときにはm_hWndがNULLの場合がある

	return 0;


}


