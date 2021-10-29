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
END_MSG_MAP()
// ハンドラのプロトタイプ:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

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

private:
	CWindow m_dlg_wnd;

	int m_namenum;
	bool m_ischeckedmostrecent;
	WCHAR m_selectname[MAX_PATH];

	std::vector<HISTORYELEM> m_copyhistory;

	UINT m_ctrlid[20];

};

#endif //__ColiIDDlg_H_
