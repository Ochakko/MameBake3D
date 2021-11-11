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
	m_createdflag = false;

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

	m_text1[0] = IDC_STATIC1_1;
	m_text1[1] = IDC_STATIC2_1;
	m_text1[2] = IDC_STATIC3_1;
	m_text1[3] = IDC_STATIC4_1;
	m_text1[4] = IDC_STATIC5_1;
	m_text1[5] = IDC_STATIC6_1;
	m_text1[6] = IDC_STATIC7_1;
	m_text1[7] = IDC_STATIC8_1;
	m_text1[8] = IDC_STATIC9_1;
	m_text1[9] = IDC_STATIC10_1;

	m_text2[0] = IDC_STATIC1_2;
	m_text2[1] = IDC_STATIC2_2;
	m_text2[2] = IDC_STATIC3_2;
	m_text2[3] = IDC_STATIC4_2;
	m_text2[4] = IDC_STATIC5_2;
	m_text2[5] = IDC_STATIC6_2;
	m_text2[6] = IDC_STATIC7_2;
	m_text2[7] = IDC_STATIC8_2;
	m_text2[8] = IDC_STATIC9_2;
	m_text2[9] = IDC_STATIC10_2;


	ZeroMemory(m_strimportance, sizeof(WCHAR) * 32 * 6);
	wcscpy_s(m_strimportance[0], 32, L"Undef.");
	wcscpy_s(m_strimportance[1], 32, L"Tiny.");
	wcscpy_s(m_strimportance[2], 32, L"ALittle.");
	wcscpy_s(m_strimportance[3], 32, L"Normal.");
	wcscpy_s(m_strimportance[4], 32, L"Noticed.");
	wcscpy_s(m_strimportance[5], 32, L"Important.");
	wcscpy_s(m_strimportance[6], 32, L"VeryImportant.");


	m_strcombo_fbxname.clear();
	m_strcombo_motionname.clear();
	m_strcombo_bvhtype.clear();

	m_initsearchcomboflag = false;

}

//LRESULT CCopyHistoryDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	int ret;
//
//	InitCommonControls();
//
//	m_dlg_wnd = m_hWnd;
//
//	ret = ParamsToDlg();
//	_ASSERT( !ret );
//
//
//	return 1;  // システムにフォーカスを設定させます
//}

LRESULT CCopyHistoryDlg::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//int ret;
	
	InitCommonControls();
	
	//m_dlg_wnd = m_hWnd;
	
	//ret = ParamsToDlg();
	//_ASSERT( !ret );
	
	return 0;
}
LRESULT CCopyHistoryDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
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
		m_selectname[MAX_PATH - 1] = 0L;
		wcscpy_s(m_selectname, MAX_PATH, m_copyhistory[selectedno].wfilename);
		m_selectname[MAX_PATH - 1] = 0L;
	}

	m_ischeckedmostrecent = m_dlg_wnd.IsDlgButtonChecked(IDC_CHECK1);


	m_initsearchcomboflag = false;
	//EndDialog(wID);
	//DestroyWindow();
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CCopyHistoryDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_initsearchcomboflag = false;
	//EndDialog(wID);
	//DestroyWindow();
	ShowWindow(SW_HIDE);
	return 0;
}

int CCopyHistoryDlg::ParamsToDlg()
{
	//m_dlg_wnd.CheckRadioButton(IDC_RADIO1, IDC_RADIO5, (g_ClearColorIndex + IDC_RADIO1));

	m_dlg_wnd = m_hWnd;

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

	SetEnableCtrls();

	if (m_initsearchcomboflag == false) {
		//ctrls for seasrch
		m_strcombo_fbxname.clear();
		m_strcombo_motionname.clear();
		m_strcombo_bvhtype.clear();

		int srcnum = m_savecopyhistory.size();//検索候補はsavecopyhistoryから取得する
		int srcno;
		for (srcno = 0; srcno < srcnum; srcno++) {
			if (m_savecopyhistory[srcno].hascpinfo == 1) {
				wstring curfbxname = m_savecopyhistory[srcno].cpinfo.fbxname;
				bool foundfbxname = false;
				std::vector<std::wstring>::iterator itrfbxname;
				for (itrfbxname = m_strcombo_fbxname.begin(); itrfbxname != m_strcombo_fbxname.end(); itrfbxname++) {
					if (curfbxname == *itrfbxname) {
						foundfbxname = true;
						break;
					}
				}
				if (foundfbxname == false) {
					m_strcombo_fbxname.push_back(curfbxname);
				}


				wstring curmotionname = m_savecopyhistory[srcno].cpinfo.motionname;
				bool foundmotionname = false;
				std::vector<std::wstring>::iterator itrmotionname;
				for (itrmotionname = m_strcombo_motionname.begin(); itrmotionname != m_strcombo_motionname.end(); itrmotionname++) {
					if (curmotionname == *itrmotionname) {
						foundmotionname = true;
						break;
					}
				}
				if (foundmotionname == false) {
					m_strcombo_motionname.push_back(curmotionname);
				}


				int curbvhtype = m_savecopyhistory[srcno].cpinfo.bvhtype;
				bool foundbvhtype = false;
				std::vector<int>::iterator itrbvhtype;
				for (itrbvhtype = m_strcombo_bvhtype.begin(); itrbvhtype != m_strcombo_bvhtype.end(); itrbvhtype++) {
					if (curbvhtype == *itrbvhtype) {
						foundbvhtype = true;
						break;
					}
				}
				if (foundbvhtype == false) {
					m_strcombo_bvhtype.push_back(curbvhtype);
				}
			}
		}

		CWindow combofbxnamewnd = m_dlg_wnd.GetDlgItem(IDC_COMBO1);
		if (combofbxnamewnd.IsWindow()) {
			combofbxnamewnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combofbxnamewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combofbxnamewnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int fbxnamenum = m_strcombo_fbxname.size();
			int fbxnameno;
			for (fbxnameno = 0; fbxnameno < fbxnamenum; fbxnameno++) {
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR) * MAX_PATH);
				wcscpy_s(tempchar, MAX_PATH, m_strcombo_fbxname[fbxnameno].c_str());

				combofbxnamewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combofbxnamewnd.SendMessage(CB_SETITEMDATA, (fbxnameno + 1), (LPARAM)(fbxnameno + 1));
			}
			combofbxnamewnd.SendMessage(CB_SETCURSEL, 0, 0);
		}

		CWindow combomotionnamewnd = m_dlg_wnd.GetDlgItem(IDC_COMBO2);
		if (combomotionnamewnd.IsWindow()) {
			combomotionnamewnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combomotionnamewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combomotionnamewnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int motionnamenum = m_strcombo_motionname.size();
			int motionnameno;
			for (motionnameno = 0; motionnameno < motionnamenum; motionnameno++) {
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR) * MAX_PATH);
				wcscpy_s(tempchar, MAX_PATH, m_strcombo_motionname[motionnameno].c_str());

				combomotionnamewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combomotionnamewnd.SendMessage(CB_SETITEMDATA, (motionnameno + 1), (LPARAM)(motionnameno + 1));
			}
			combomotionnamewnd.SendMessage(CB_SETCURSEL, 0, 0);
		}

		CWindow combobvhtypewnd = m_dlg_wnd.GetDlgItem(IDC_COMBO3);
		if (combobvhtypewnd.IsWindow()) {
			combobvhtypewnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combobvhtypewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combobvhtypewnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int bvhtypenum = m_strcombo_bvhtype.size();
			int bvhtypeno;
			for (bvhtypeno = 0; bvhtypeno < bvhtypenum; bvhtypeno++) {
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR) * MAX_PATH);
				swprintf_s(tempchar, MAX_PATH, L"bvh_%03d", m_strcombo_bvhtype[bvhtypeno]);

				combobvhtypewnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combobvhtypewnd.SendMessage(CB_SETITEMDATA, (bvhtypeno + 1), (LPARAM)(bvhtypeno + 1));
			}
			combobvhtypewnd.SendMessage(CB_SETCURSEL, 0, 0);
		}


		CWindow combostartyearwnd = m_dlg_wnd.GetDlgItem(IDC_STARTYEAR);
		CWindow comboendyearwnd = m_dlg_wnd.GetDlgItem(IDC_ENDYEAR);
		if (combostartyearwnd.IsWindow() && comboendyearwnd.IsWindow()) {

			combostartyearwnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combostartyearwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combostartyearwnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);
			comboendyearwnd.SendMessage(CB_RESETCONTENT, 0, 0);
			comboendyearwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			comboendyearwnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int comboyearnum = 4;
			int comboyearno;
			for (comboyearno = 0; comboyearno < comboyearnum; comboyearno++) {
				int curyear = 2021 + comboyearno;
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR)* MAX_PATH);
				swprintf_s(tempchar, MAX_PATH, L"%d", curyear);

				combostartyearwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combostartyearwnd.SendMessage(CB_SETITEMDATA, (comboyearno + 1), (LPARAM)curyear);
				comboendyearwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				comboendyearwnd.SendMessage(CB_SETITEMDATA, (comboyearno + 1), (LPARAM)curyear);
			}
			combostartyearwnd.SendMessage(CB_SETCURSEL, 0, 0);
			comboendyearwnd.SendMessage(CB_SETCURSEL, 0, 0);
		}


		CWindow combostartmonthwnd = m_dlg_wnd.GetDlgItem(IDC_STARTMONTH);
		CWindow comboendmonthwnd = m_dlg_wnd.GetDlgItem(IDC_ENDMONTH);
		if (combostartmonthwnd.IsWindow() && comboendmonthwnd.IsWindow()) {

			combostartmonthwnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combostartmonthwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combostartmonthwnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);
			comboendmonthwnd.SendMessage(CB_RESETCONTENT, 0, 0);
			comboendmonthwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			comboendmonthwnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int combomonthnum = 12;
			int combomonthno;
			for (combomonthno = 1; combomonthno <= combomonthnum; combomonthno++) {
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR) * MAX_PATH);
				swprintf_s(tempchar, MAX_PATH, L"%d", combomonthno);

				combostartmonthwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combostartmonthwnd.SendMessage(CB_SETITEMDATA, combomonthno, (LPARAM)combomonthno);
				comboendmonthwnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				comboendmonthwnd.SendMessage(CB_SETITEMDATA, combomonthno, (LPARAM)combomonthno);
			}
			combostartmonthwnd.SendMessage(CB_SETCURSEL, 0, 0);
			comboendmonthwnd.SendMessage(CB_SETCURSEL, 0, 0);
		}


		CWindow combostartdaywnd = m_dlg_wnd.GetDlgItem(IDC_STARTDAY);
		CWindow comboenddaywnd = m_dlg_wnd.GetDlgItem(IDC_ENDDAY);
		if (combostartdaywnd.IsWindow() && comboenddaywnd.IsWindow()) {

			combostartdaywnd.SendMessage(CB_RESETCONTENT, 0, 0);
			combostartdaywnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			combostartdaywnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);
			comboenddaywnd.SendMessage(CB_RESETCONTENT, 0, 0);
			comboenddaywnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)L"----");
			comboenddaywnd.SendMessage(CB_SETITEMDATA, 0, (LPARAM)0);

			int combodaynum = 31;
			int combodayno;
			for (combodayno = 1; combodayno <= combodaynum; combodayno++) {
				WCHAR tempchar[MAX_PATH];
				ZeroMemory(tempchar, sizeof(WCHAR) * MAX_PATH);
				swprintf_s(tempchar, MAX_PATH, L"%d", combodayno);

				combostartdaywnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				combostartdaywnd.SendMessage(CB_SETITEMDATA, combodayno, (LPARAM)combodayno);
				comboenddaywnd.SendMessage(CB_ADDSTRING, 0, (LPARAM)tempchar);
				comboenddaywnd.SendMessage(CB_SETITEMDATA, combodayno, (LPARAM)combodayno);
			}
			combostartdaywnd.SendMessage(CB_SETCURSEL, 0, 0);
			comboenddaywnd.SendMessage(CB_SETCURSEL, 0, 0);
		}

		m_initsearchcomboflag = true;
	}


	return 0;
}

void CCopyHistoryDlg::SetEnableCtrls()
{

	BOOL srcenable;
	if (m_dlg_wnd.IsDlgButtonChecked(IDC_CHECK1)) {
		m_ischeckedmostrecent = true;
		srcenable = FALSE;
	}
	else {
		m_ischeckedmostrecent = false;
		srcenable = TRUE;
	}

	int nameno;
	for (nameno = 0; nameno < 10; nameno++) {
		CWindow radiobutton = m_dlg_wnd.GetDlgItem(m_ctrlid[nameno]);
		if (radiobutton.IsWindow()) {
			radiobutton.EnableWindow(srcenable);
		}
		CWindow startframewnd = m_dlg_wnd.GetDlgItem(m_startframeid[nameno]);
		if (startframewnd.IsWindow()) {
			startframewnd.EnableWindow(srcenable);
		}
		CWindow framenumwnd = m_dlg_wnd.GetDlgItem(m_framenumid[nameno]);
		if (framenumwnd.IsWindow()) {
			framenumwnd.EnableWindow(srcenable);
		}
		CWindow bvhtypewnd = m_dlg_wnd.GetDlgItem(m_bvhtypeid[nameno]);
		if (bvhtypewnd.IsWindow()) {
			bvhtypewnd.EnableWindow(srcenable);
		}
		CWindow importancewnd = m_dlg_wnd.GetDlgItem(m_importanceid[nameno]);
		if (importancewnd.IsWindow()) {
			importancewnd.EnableWindow(srcenable);
		}
		CWindow commentwnd = m_dlg_wnd.GetDlgItem(m_commentid[nameno]);
		if (commentwnd.IsWindow()) {
			commentwnd.EnableWindow(srcenable);
		}

		CWindow text1wnd = m_dlg_wnd.GetDlgItem(m_text1[nameno]);
		if (text1wnd.IsWindow()) {
			text1wnd.EnableWindow(srcenable);
		}
		CWindow text2wnd = m_dlg_wnd.GetDlgItem(m_text2[nameno]);
		if (text2wnd.IsWindow()) {
			text2wnd.EnableWindow(srcenable);
		}
	}
}

LRESULT CCopyHistoryDlg::OnCheckMostRecent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetEnableCtrls();
	return 0;
}


int CCopyHistoryDlg::SetNames(std::vector<HISTORYELEM>& copyhistory)
{
	m_copyhistory = copyhistory;
	m_savecopyhistory = copyhistory;

	m_namenum = min(10, m_copyhistory.size());
	
	ParamsToDlg();//表示されていないときにはm_hWndがNULLの場合があるのでShowWindowの後でSetNamesを呼ぶ


	m_createdflag = true;

	return 0;


}

LRESULT CCopyHistoryDlg::OnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bool findfbxnameflag = false;
	WCHAR findfbxname[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_COMBO1, findfbxname, MAX_PATH);
	if (wcscmp(findfbxname, L"----") != 0) {
		findfbxnameflag = true;
	}

	bool findmotionnameflag = false;
	WCHAR findmotionname[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_COMBO2, findmotionname, MAX_PATH);
	if (wcscmp(findmotionname, L"----") != 0) {
		findmotionnameflag = true;
	}

	bool findbvhtypeflag = false;
	WCHAR findbvhtype[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_COMBO3, findbvhtype, MAX_PATH);
	if (wcscmp(findbvhtype, L"----") != 0) {
		findbvhtypeflag = true;
	}

	bool findstartyearflag = false;
	WCHAR findstartyear[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_STARTYEAR, findstartyear, MAX_PATH);
	if (wcscmp(findstartyear, L"----") != 0) {
		findstartyearflag = true;
	}
	bool findendyearflag = false;
	WCHAR findendyear[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_ENDYEAR, findendyear, MAX_PATH);
	if (wcscmp(findendyear, L"----") != 0) {
		findendyearflag = true;
	}

	bool findstartmonthflag = false;
	WCHAR findstartmonth[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_STARTMONTH, findstartmonth, MAX_PATH);
	if (wcscmp(findstartmonth, L"----") != 0) {
		findstartmonthflag = true;
	}
	bool findendmonthflag = false;
	WCHAR findendmonth[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_ENDMONTH, findendmonth, MAX_PATH);
	if (wcscmp(findendmonth, L"----") != 0) {
		findendmonthflag = true;
	}

	bool findstartdayflag = false;
	WCHAR findstartday[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_STARTDAY, findstartday, MAX_PATH);
	if (wcscmp(findstartday, L"----") != 0) {
		findstartdayflag = true;
	}
	bool findenddayflag = false;
	WCHAR findendday[MAX_PATH] = { 0L };
	m_dlg_wnd.GetDlgItemTextW(IDC_ENDDAY, findendday, MAX_PATH);
	if (wcscmp(findendday, L"----") != 0) {
		findenddayflag = true;
	}


	bool finddateflag;
	if (findstartyearflag && findendyearflag && findstartmonthflag && findendmonthflag && findstartdayflag && findenddayflag) {
		finddateflag = true;
	}
	else {
		finddateflag = false;
	}


	//search fbxname
	std::vector<HISTORYELEM> copyhistory1 = m_savecopyhistory;
	std::vector<HISTORYELEM> copyhistory2;
	copyhistory2.clear();
	{
		int namenum1 = copyhistory1.size();
		if (findfbxnameflag) {
			int nameno1;
			for (nameno1 = 0; nameno1 < namenum1; nameno1++) {
				if (copyhistory1[nameno1].hascpinfo == 1) {
					if (wcscmp(copyhistory1[nameno1].cpinfo.fbxname, findfbxname) == 0) {
						copyhistory2.push_back(copyhistory1[nameno1]);
					}
				}
			}
		}
		else {
			copyhistory2 = copyhistory1;
		}
	}

	//search motionname
	std::vector<HISTORYELEM> copyhistory3;
	copyhistory3.clear();
	{
		int namenum1 = copyhistory2.size();
		if (findmotionnameflag) {
			int nameno1;
			for (nameno1 = 0; nameno1 < namenum1; nameno1++) {
				if (copyhistory2[nameno1].hascpinfo == 1) {
					if (wcscmp(copyhistory2[nameno1].cpinfo.motionname, findmotionname) == 0) {
						copyhistory3.push_back(copyhistory2[nameno1]);
					}
				}
			}
		}
		else {
			copyhistory3 = copyhistory2;
		}
	}

	//search bvhtype
	std::vector<HISTORYELEM> copyhistory4;
	copyhistory4.clear();
	{
		int namenum1 = copyhistory3.size();
		if (findbvhtypeflag) {
			int nameno1;
			for (nameno1 = 0; nameno1 < namenum1; nameno1++) {
				if (copyhistory3[nameno1].hascpinfo == 1) {
					WCHAR srcbvhtype[MAX_PATH] = { 0L };
					swprintf_s(srcbvhtype, MAX_PATH, L"bvh_%03d", copyhistory3[nameno1].cpinfo.bvhtype);
					if (wcscmp(srcbvhtype, findbvhtype) == 0) {
						copyhistory4.push_back(copyhistory3[nameno1]);
					}
				}
			}
		}
		else {
			copyhistory4 = copyhistory3;
		}
	}


	//search date
	std::vector<HISTORYELEM> copyhistory5;
	copyhistory5.clear();
	{
		int namenum1 = copyhistory4.size();
		if (finddateflag) {
			int nameno1;
			for (nameno1 = 0; nameno1 < namenum1; nameno1++) {
				if (copyhistory4[nameno1].hascpinfo == 1) {
					FILETIME srcfiletime = copyhistory4[nameno1].filetime;
					FILETIME startfiletime;
					FILETIME endfiletime;
					SYSTEMTIME ststart;
					SYSTEMTIME stend;
					ZeroMemory(&ststart, sizeof(SYSTEMTIME));
					ZeroMemory(&stend, sizeof(SYSTEMTIME));
					ststart.wYear = _wtol(findstartyear);
					ststart.wMonth = _wtol(findstartmonth);
					ststart.wDay = _wtol(findstartday);
					stend.wYear = _wtol(findendyear);
					stend.wMonth = _wtol(findendmonth);
					stend.wDay = _wtol(findendday);
					stend.wHour = 23;
					stend.wMinute = 59;
					stend.wSecond = 59;
					stend.wMilliseconds = 999;
					SystemTimeToFileTime(&ststart, &startfiletime);
					SystemTimeToFileTime(&stend, &endfiletime);

					if ((CompareFileTime(&srcfiletime, &startfiletime) >= 0) && (CompareFileTime(&srcfiletime, &endfiletime) <= 0)) {
						copyhistory5.push_back(copyhistory4[nameno1]);
					}
				}
			}
		}
		else {
			copyhistory5 = copyhistory4;
		}
	}




	m_copyhistory = copyhistory5;
	m_namenum = min(10, m_copyhistory.size());

	ParamsToDlg();



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


LRESULT CCopyHistoryDlg::OnRadio1(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(0);
}
LRESULT CCopyHistoryDlg::OnRadio2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(1);
}
LRESULT CCopyHistoryDlg::OnRadio3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(2);
}
LRESULT CCopyHistoryDlg::OnRadio4(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(3);
}
LRESULT CCopyHistoryDlg::OnRadio5(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(4);
}
LRESULT CCopyHistoryDlg::OnRadio6(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(5);
}
LRESULT CCopyHistoryDlg::OnRadio7(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(6);
}
LRESULT CCopyHistoryDlg::OnRadio8(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(7);
}
LRESULT CCopyHistoryDlg::OnRadio9(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(8);
}
LRESULT CCopyHistoryDlg::OnRadio10(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return OnRadio(9);
}

LRESULT CCopyHistoryDlg::OnRadio(int radioid)
{
	if ((radioid < 0) || (radioid >= m_namenum)) {
		return 0;
	}

	if (m_namenum != m_copyhistory.size()) {
		_ASSERT(0);
		return 1;
	}

	m_selectname[MAX_PATH - 1] = 0L;
	wcscpy_s(m_selectname, MAX_PATH, m_copyhistory[radioid].wfilename);
	m_selectname[MAX_PATH - 1] = 0L;

	return 0;
}
