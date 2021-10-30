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
	
	m_startframeid[0] = IDC_TEXT_STARTFRAME;
	m_startframeid[1] = IDC_TEXT_STARTFRAME2;
	m_startframeid[2] = IDC_TEXT_STARTFRAME3;
	m_startframeid[3] = IDC_TEXT_STARTFRAME4;
	m_startframeid[4] = IDC_TEXT_STARTFRAME5;
	m_startframeid[5] = IDC_TEXT_STARTFRAME6;
	m_startframeid[6] = IDC_TEXT_STARTFRAME7;
	m_startframeid[7] = IDC_TEXT_STARTFRAME8;
	m_startframeid[8] = IDC_TEXT_STARTFRAME9;
	m_startframeid[9] = IDC_TEXT_STARTFRAME10;

	m_framenumid[0] = IDC_TEXT_FRAMENUM;
	m_framenumid[1] = IDC_TEXT_FRAMENUM2;
	m_framenumid[2] = IDC_TEXT_FRAMENUM3;
	m_framenumid[3] = IDC_TEXT_FRAMENUM4;
	m_framenumid[4] = IDC_TEXT_FRAMENUM5;
	m_framenumid[5] = IDC_TEXT_FRAMENUM6;
	m_framenumid[6] = IDC_TEXT_FRAMENUM7;
	m_framenumid[7] = IDC_TEXT_FRAMENUM8;
	m_framenumid[8] = IDC_TEXT_FRAMENUM9;
	m_framenumid[9] = IDC_TEXT_FRAMENUM10;

	m_bvhtypeid[0] = IDC_TEXT_BVHTYPE;
	m_bvhtypeid[1] = IDC_TEXT_BVHTYPE2;
	m_bvhtypeid[2] = IDC_TEXT_BVHTYPE3;
	m_bvhtypeid[3] = IDC_TEXT_BVHTYPE4;
	m_bvhtypeid[4] = IDC_TEXT_BVHTYPE5;
	m_bvhtypeid[5] = IDC_TEXT_BVHTYPE6;
	m_bvhtypeid[6] = IDC_TEXT_BVHTYPE7;
	m_bvhtypeid[7] = IDC_TEXT_BVHTYPE8;
	m_bvhtypeid[8] = IDC_TEXT_BVHTYPE9;
	m_bvhtypeid[9] = IDC_TEXT_BVHTYPE10;

	m_importanceid[0] = IDC_TEXT_IMPORTANCE;
	m_importanceid[1] = IDC_TEXT_IMPORTANCE2;
	m_importanceid[2] = IDC_TEXT_IMPORTANCE3;
	m_importanceid[3] = IDC_TEXT_IMPORTANCE4;
	m_importanceid[4] = IDC_TEXT_IMPORTANCE5;
	m_importanceid[5] = IDC_TEXT_IMPORTANCE6;
	m_importanceid[6] = IDC_TEXT_IMPORTANCE7;
	m_importanceid[7] = IDC_TEXT_IMPORTANCE8;
	m_importanceid[8] = IDC_TEXT_IMPORTANCE9;
	m_importanceid[9] = IDC_TEXT_IMPORTANCE10;

	m_commentid[0] = IDC_TEXT_COMMENT1;
	m_commentid[1] = IDC_TEXT_COMMENT2;
	m_commentid[2] = IDC_TEXT_COMMENT3;
	m_commentid[3] = IDC_TEXT_COMMENT4;
	m_commentid[4] = IDC_TEXT_COMMENT5;
	m_commentid[5] = IDC_TEXT_COMMENT6;
	m_commentid[6] = IDC_TEXT_COMMENT7;
	m_commentid[7] = IDC_TEXT_COMMENT8;
	m_commentid[8] = IDC_TEXT_COMMENT9;
	m_commentid[9] = IDC_TEXT_COMMENT10;

	ZeroMemory(m_strimportance, sizeof(WCHAR) * 32 * 6);
	wcscpy_s(m_strimportance[0], 32, L"Undef.");
	wcscpy_s(m_strimportance[1], 32, L"Tiny.");
	wcscpy_s(m_strimportance[2], 32, L"ALittle.");
	wcscpy_s(m_strimportance[3], 32, L"Normal.");
	wcscpy_s(m_strimportance[4], 32, L"Noticed.");
	wcscpy_s(m_strimportance[5], 32, L"Important.");
	wcscpy_s(m_strimportance[6], 32, L"VeryImportant.");

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
		if (m_dlg_wnd.IsDlgButtonChecked(m_ctrlid[nameno]) && (m_copyhistory[nameno].hascpinfo == 1)) {
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
		wcscpy_s(dispname, MAX_PATH, m_copyhistory[nameno].cpinfo.fbxname);
		wcscat_s(dispname, MAX_PATH, L"_");
		wcscat_s(dispname, MAX_PATH, m_copyhistory[nameno].cpinfo.motionname);
		m_dlg_wnd.SetDlgItemTextW(m_ctrlid[nameno], dispname);

		if (m_copyhistory[nameno].hascpinfo == 1) {
			WCHAR textstartframe[MAX_PATH] = { 0L };
			swprintf_s(textstartframe, MAX_PATH, L"%.0lf", m_copyhistory[nameno].cpinfo.startframe);
			m_dlg_wnd.SetDlgItemTextW(m_startframeid[nameno], textstartframe);

			WCHAR textframenum[MAX_PATH] = { 0L };
			swprintf_s(textframenum, MAX_PATH, L"%.0lf", m_copyhistory[nameno].cpinfo.framenum);
			m_dlg_wnd.SetDlgItemTextW(m_framenumid[nameno], textframenum);

			int curbvhtype = m_copyhistory[nameno].cpinfo.bvhtype;
			WCHAR textbvhtype[MAX_PATH] = { 0L };
			if ((curbvhtype >= 1) && (curbvhtype <= 144)) {
				swprintf_s(textbvhtype, MAX_PATH, L"bvh_%03d", m_copyhistory[nameno].cpinfo.bvhtype);
			}
			else {
				swprintf_s(textbvhtype, MAX_PATH, L"bvh_Undef");
			}
			m_dlg_wnd.SetDlgItemTextW(m_bvhtypeid[nameno], textbvhtype);


			int curimportance = m_copyhistory[nameno].cpinfo.importance;
			WCHAR textimportance[MAX_PATH] = { 0L };
			if ((curimportance >= 0) && (curimportance <= 6)) {
				swprintf_s(textimportance, MAX_PATH, m_strimportance[curimportance]);
			}
			else {
				swprintf_s(textimportance, MAX_PATH, L"Undef.");
			}
			m_dlg_wnd.SetDlgItemTextW(m_importanceid[nameno], textimportance);


			m_copyhistory[nameno].cpinfo.comment[32 - 1] = 0L;
			m_dlg_wnd.SetDlgItemTextW(m_commentid[nameno], m_copyhistory[nameno].cpinfo.comment);


			if ((m_selectname[0] != 0L) && (wcscmp(m_selectname, m_copyhistory[nameno].wfilename) == 0)) {
				m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO10, m_ctrlid[nameno]);
				ischeck = true;
			}
		}
		else {
			//m_dlg_wnd.SetDlgItemTextW(m_ctrlid[nameno], L"Invalid.");
			m_dlg_wnd.SetDlgItemTextW(m_startframeid[nameno], L"Invalid.");
			m_dlg_wnd.SetDlgItemTextW(m_framenumid[nameno], L"Invalid.");
			m_dlg_wnd.SetDlgItemTextW(m_bvhtypeid[nameno], L"Invalid.");
			m_dlg_wnd.SetDlgItemTextW(m_importanceid[nameno], L"Invalid.");
			m_dlg_wnd.SetDlgItemTextW(m_commentid[nameno], L"Invalid.");
		}
	}
	int nameno2;
	for (nameno2 = m_namenum; nameno2 < 10; nameno2++) {
		m_dlg_wnd.SetDlgItemTextW(m_ctrlid[nameno2], L"no more.");
		m_dlg_wnd.SetDlgItemTextW(m_startframeid[nameno2], L"no more.");
		m_dlg_wnd.SetDlgItemTextW(m_framenumid[nameno2], L"no more.");
		m_dlg_wnd.SetDlgItemTextW(m_bvhtypeid[nameno2], L"no more.");
		m_dlg_wnd.SetDlgItemTextW(m_importanceid[nameno2], L"no more.");
		m_dlg_wnd.SetDlgItemTextW(m_commentid[nameno2], L"no more.");
	}
	if (ischeck == false) {
		m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO10, IDC_RADIO1);
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

	m_namenum = min(10, m_copyhistory.size());
	
	//ParamsToDlg();//表示されていないときにはm_hWndがNULLの場合がある

	return 0;


}

LRESULT CCopyHistoryDlg::OnSelFbxName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}
LRESULT CCopyHistoryDlg::OnSelMotionName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}
LRESULT CCopyHistoryDlg::OnSelBvhType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}
LRESULT CCopyHistoryDlg::OnSelImportance(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}
LRESULT CCopyHistoryDlg::OnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return 0;
}

LRESULT CCopyHistoryDlg::OnDelete1(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(0);
}
LRESULT CCopyHistoryDlg::OnDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(1);
}
LRESULT CCopyHistoryDlg::OnDelete3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(2);
}
LRESULT CCopyHistoryDlg::OnDelete4(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(3);
}
LRESULT CCopyHistoryDlg::OnDelete5(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(4);
}
LRESULT CCopyHistoryDlg::OnDelete6(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(5);
}
LRESULT CCopyHistoryDlg::OnDelete7(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(6);
}
LRESULT CCopyHistoryDlg::OnDelete8(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(7);
}
LRESULT CCopyHistoryDlg::OnDelete9(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(8);
}
LRESULT CCopyHistoryDlg::OnDelete10(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnDelete(9);
}

LRESULT CCopyHistoryDlg::OnDelete(int delid)
{
	if ((delid < 0) || (delid >= m_namenum)) {
		return 0;
	}

	WCHAR delcpt[MAX_PATH] = { 0L };
	wcscpy_s(delcpt, MAX_PATH, m_copyhistory[delid].wfilename);
	delcpt[MAX_PATH - 1] = 0L;
	if (delcpt[0] != 0L) {
		BOOL bexist;
		bexist = PathFileExists(delcpt);
		if (bexist) {
			DeleteFileW(delcpt);
		}
	}

	if (m_copyhistory[delid].hascpinfo == 1) {
		WCHAR delcpi[MAX_PATH] = { 0L };
		wcscpy_s(delcpi, MAX_PATH, delcpt);
		delcpi[MAX_PATH - 1] = 0L;
		WCHAR* pdot = wcsrchr(delcpi, TEXT('.'));
		if (pdot) {
			*pdot = 0L;
			wcscat_s(delcpi, MAX_PATH, L".cpi");
			BOOL bexist;
			bexist = PathFileExists(delcpi);
			if (bexist) {
				DeleteFileW(delcpi);
			}
		}
	}

	wcscpy_s(m_copyhistory[delid].wfilename, MAX_PATH, L"deleted.");
	m_copyhistory[delid].hascpinfo = 0;
	wcscpy_s(m_copyhistory[delid].cpinfo.fbxname, MAX_PATH, L"deleted.");
	wcscpy_s(m_copyhistory[delid].cpinfo.motionname, MAX_PATH, L"deleted.");

	ParamsToDlg();

	return 0;
}



