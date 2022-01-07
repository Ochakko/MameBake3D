// InfluenceDlg.h : CCopyHistoryDlg の宣言

#ifndef __CCopyHistoryDlg_H_
#define __CCopyHistoryDlg_H_

//#include "usealt.h"

#include <coef.h>

#include <atlbase.h>
#include <atlhost.h>

#include "resource.h"       // メイン シンボル

#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// CCopyHistoryDlg
class CCopyHistoryDlg : 
	public CAxDialogImpl<CCopyHistoryDlg>
{
public:
	CCopyHistoryDlg();
	~CCopyHistoryDlg();

	enum { IDD = IDD_COPYHISTORYDLG };

BEGIN_MSG_MAP(CCopyHistoryDlg)
	//MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_CHECK1, OnCheckMostRecent)
	COMMAND_ID_HANDLER(IDC_BUTTON3, OnSearch)
	COMMAND_ID_HANDLER(IDC_BUTTON6, OnDelete1)
	COMMAND_ID_HANDLER(IDC_BUTTON16, OnDelete2)
	COMMAND_ID_HANDLER(IDC_BUTTON17, OnDelete3)
	COMMAND_ID_HANDLER(IDC_BUTTON18, OnDelete4)
	COMMAND_ID_HANDLER(IDC_BUTTON19, OnDelete5)
	COMMAND_ID_HANDLER(IDC_BUTTON20, OnDelete6)
	COMMAND_ID_HANDLER(IDC_BUTTON21, OnDelete7)
	COMMAND_ID_HANDLER(IDC_BUTTON22, OnDelete8)
	COMMAND_ID_HANDLER(IDC_BUTTON23, OnDelete9)
	COMMAND_ID_HANDLER(IDC_BUTTON24, OnDelete10)
	COMMAND_ID_HANDLER(IDC_RADIO1, OnRadio1)
	COMMAND_ID_HANDLER(IDC_RADIO2, OnRadio2)
	COMMAND_ID_HANDLER(IDC_RADIO3, OnRadio3)
	COMMAND_ID_HANDLER(IDC_RADIO4, OnRadio4)
	COMMAND_ID_HANDLER(IDC_RADIO5, OnRadio5)
	COMMAND_ID_HANDLER(IDC_RADIO6, OnRadio6)
	COMMAND_ID_HANDLER(IDC_RADIO7, OnRadio7)
	COMMAND_ID_HANDLER(IDC_RADIO8, OnRadio8)
	COMMAND_ID_HANDLER(IDC_RADIO9, OnRadio9)
	COMMAND_ID_HANDLER(IDC_RADIO10, OnRadio10)
	COMMAND_ID_HANDLER(IDC_CHECK1, OnChkRecent)
END_MSG_MAP()
// ハンドラのプロトタイプ:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	//LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCheckMostRecent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnDelete1(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete4(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete5(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete6(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete7(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete8(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete9(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete10(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnRadio1(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio4(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio5(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio6(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio7(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio8(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio9(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRadio10(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnChkRecent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	int SetNames(std::vector<HISTORYELEM>& copyhistory);

	bool IsCheckedMostRecent()
	{
		return m_ischeckedmostrecent;
	};

	int GetSelectedFileName(WCHAR* dstfilename) {
		*dstfilename = 0L;
		m_selectname[MAX_PATH - 1] = 0L;

		if (m_selectname[0] != 0L) {
			wcscpy_s(dstfilename, MAX_PATH, m_selectname);
			return 0;
		}
		else {
			return 1;
		}
	};

	bool GetCreatedFlag()
	{
		return m_createdflag;
	};

private:
	void InitParams();
	int DestroyObjs();
	int ParamsToDlg();

	LRESULT OnDelete(int delid);
	LRESULT OnRadio(int radioid);
	void SetEnableCtrls();

private:
	bool m_createdflag;
	CWindow m_dlg_wnd;

	int m_namenum;
	bool m_ischeckedmostrecent;
	WCHAR m_selectname[MAX_PATH];

	std::vector<HISTORYELEM> m_copyhistory;
	std::vector<HISTORYELEM> m_savecopyhistory;

	UINT m_ctrlid[10];
	UINT m_startframeid[10];
	UINT m_framenumid[10];
	UINT m_bvhtypeid[10];
	UINT m_importanceid[10];
	UINT m_commentid[10];

	UINT m_text1[10];
	UINT m_text2[10];

	WCHAR m_strimportance[7][32];

	std::vector<std::wstring> m_strcombo_fbxname;
	std::vector<std::wstring> m_strcombo_motionname;
	std::vector<int> m_strcombo_bvhtype;


	bool m_initsearchcomboflag;

};

#endif //__ColiIDDlg_H_
