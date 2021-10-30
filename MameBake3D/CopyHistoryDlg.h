// InfluenceDlg.h : CCopyHistoryDlg の宣言

#ifndef __CCopyHistoryDlg_H_
#define __CCopyHistoryDlg_H_

//#include "usealt.h"

#include <coef.h>

#include <atlbase.h>
#include <atlhost.h>

#include "resource.h"       // メイン シンボル

#include <vector>

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
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_BUTTON1, OnSelFbxName)
	COMMAND_ID_HANDLER(IDC_BUTTON2, OnSelMotionName)
	COMMAND_ID_HANDLER(IDC_BUTTON4, OnSelBvhType)
	COMMAND_ID_HANDLER(IDC_BUTTON5, OnSelImportance)
	COMMAND_ID_HANDLER(IDC_BUTTON3, OnSearch)
	COMMAND_ID_HANDLER(IDC_BUTTON6, OnDelete1)
	COMMAND_ID_HANDLER(IDC_BUTTON7, OnDelete2)
	COMMAND_ID_HANDLER(IDC_BUTTON8, OnDelete3)
	COMMAND_ID_HANDLER(IDC_BUTTON9, OnDelete4)
	COMMAND_ID_HANDLER(IDC_BUTTON10, OnDelete5)
	COMMAND_ID_HANDLER(IDC_BUTTON11, OnDelete6)
	COMMAND_ID_HANDLER(IDC_BUTTON12, OnDelete7)
	COMMAND_ID_HANDLER(IDC_BUTTON13, OnDelete8)
	COMMAND_ID_HANDLER(IDC_BUTTON14, OnDelete9)
	COMMAND_ID_HANDLER(IDC_BUTTON15, OnDelete10)
END_MSG_MAP()
// ハンドラのプロトタイプ:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSelFbxName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelMotionName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelBvhType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelImportance(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
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

private:
	void InitParams();
	int DestroyObjs();
	int ParamsToDlg();

	LRESULT OnDelete(int delid);

private:
	CWindow m_dlg_wnd;

	int m_namenum;
	bool m_ischeckedmostrecent;
	WCHAR m_selectname[MAX_PATH];

	std::vector<HISTORYELEM> m_copyhistory;

	UINT m_ctrlid[10];
	UINT m_startframeid[10];
	UINT m_framenumid[10];
	UINT m_bvhtypeid[10];
	UINT m_importanceid[10];
	UINT m_commentid[10];

	WCHAR m_strimportance[7][32];


};

#endif //__ColiIDDlg_H_
