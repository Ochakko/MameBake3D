// FilterDlg.h : CFilterDlg ÇÃêÈåæ

#ifndef __FilterDlg_H_
#define __FilterDlg_H_

#include "../../MameBake3D/resource.h"       // ÉÅÉCÉì ÉVÉìÉ{Éã
#include <atlbase.h>
#include <atlhost.h>

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg
class CFilterDlg : 
	public CAxDialogImpl<CFilterDlg>
{
public:
	CFilterDlg();
	~CFilterDlg();

	enum { IDD = IDD_FILTERDLG };

BEGIN_MSG_MAP(CFilterDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	int GetFilterType()
	{
		return m_filtertype;
	};
	int GetFilterSize()
	{
		return m_filtersize;
	};

private:
	void SetWnd();
	int SetCombo();
	int ParamsToDlg();

public:
	int m_cmdshow;
	int m_filtertype;
	int m_filtersize;

private:
	CWindow m_filtertype_wnd;
	CWindow m_filtersize_wnd;
};

#endif //__FilterDlg_H_
