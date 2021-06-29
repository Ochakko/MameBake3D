// LocateMBDlg.h : CLocateMBDlg ÇÃêÈåæ

#ifndef __LocateMBDlg_H_
#define __LocateMBDlg_H_

#include "resource.h"       // ÉÅÉCÉì ÉVÉìÉ{Éã
#include <atlbase.h>
#include <atlhost.h>

/////////////////////////////////////////////////////////////////////////////
// CLocateMBDlg
class CLocateMBDlg : 
	public CAxDialogImpl<CLocateMBDlg>
{
public:
	CLocateMBDlg();
	~CLocateMBDlg();

	enum { IDD = IDD_LOCATEMBDLG };


BEGIN_MSG_MAP(CLocateMBDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
	void SetWnd();
	int ParamsToDlg();

public:
	//int m_cmdshow;
	BOOL m_w2;
	BOOL m_h2;

private:

	CWindow m_dlg_wnd;
	CWindow m_w2_wnd;
	CWindow m_h2_wnd;
};

#endif //__LocateMBDlg_H_
