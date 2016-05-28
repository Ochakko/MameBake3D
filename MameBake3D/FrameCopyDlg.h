// InfluenceDlg.h : CFrameCopyDlg �̐錾

#ifndef __FRAMECOPYDLG_H_
#define __FRAMECOPYDLG_H_

#include "resource.h"       // ���C�� �V���{��
#include <atlbase.h>
#include <atlhost.h>

#include "dlgid.h"

#include <map>
#include <vector>

#define FRAMECOPYLISTLENG	1024
#define FCSLOTNUM			10
#define SLOTNAMELEN			32

class CModel;
class CBone;


/////////////////////////////////////////////////////////////////////////////
// CFrameCopyDlg
class CFrameCopyDlg : 
	public CAxDialogImpl<CFrameCopyDlg>
{
public:
	CFrameCopyDlg();
	~CFrameCopyDlg();

	int SetModel( CModel* srcmodel );

	enum { IDD = IDD_FRAMECOPYDLG };

BEGIN_MSG_MAP(CFrameCopyDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	NOTIFY_HANDLER(IDN_TREE1, TVN_SELCHANGED, OnSelchangedTree1)
	COMMAND_ID_HANDLER(IDN_ADD, OnAdd)
	COMMAND_ID_HANDLER(IDN_DELETE, OnDelete)
	COMMAND_ID_HANDLER(IDN_ALLDELETE, OnAllDelete)

	COMMAND_ID_HANDLER(IDN_ADD2, OnAdd2)
	COMMAND_ID_HANDLER(IDN_DELETE2, OnDelete2)
	COMMAND_ID_HANDLER(IDN_ALLDELETE2, OnAllDelete2)

	COMMAND_ID_HANDLER(IDC_SLOTCOMBO, OnSelCombo)

END_MSG_MAP()
// �n���h���̃v���g�^�C�v:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelchangedTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAllDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnAdd2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAllDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSelCombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
	int InitParams();
	int DestroyObjs();
	int SetupDlg( CModel* srcmodel );

	int FillTree();
	void AddBoneToTree( CBone* srcbone, int addbroflag, int addtolast );
	HTREEITEM TVAdd( HTREEITEM parentTI, WCHAR* srcname, int srcno, int imageno, int selectno, int addtolast );

	void CreateImageList();
	int ParamsToDlg();

	int SetTree2ListReq( int validorinvalid, int srcno, int addbroflag );

	int CreateCombo();

public:
	std::vector<CBone*> GetCpVec()
	{
		return m_cpvec;
	}

private:
	int m_samemodelflag;

	CWindow m_dlg_wnd;
	CWindow m_tree_wnd;
	CWindow m_list_wnd;
	CWindow m_list2_wnd;
	CWindow m_combo_wnd;
	CWindow m_slotname_wnd;

	HIMAGELIST m_hImageList;
	int m_iImage;
	int m_iSelect;

	//HTREEITEM* m_TI;
	std::map<int, HTREEITEM> m_timap;
	HTREEITEM m_selecteditem;
	int m_selectedno;
	
	CModel* m_model;

public:
// ���[�U�[���w�肵���Atree�̐擪�ԍ����i�[����B
	int m_slotno;

	WCHAR m_slotname[FCSLOTNUM][SLOTNAMELEN];

	int m_influencenum[FCSLOTNUM];
	int m_influencelist[FCSLOTNUM][FRAMECOPYLISTLENG];

	int m_ignorenum[FCSLOTNUM];
	int m_ignorelist[FCSLOTNUM][FRAMECOPYLISTLENG];

// ���[�U�[���w�肵��tree�S�̂��i�[�i�q�����܂ށB�j	�����́As2shd_leng, �Y������ꍇ�́A�P����ȊO�͂O
	std::map<int, CBone*> m_validelemmap;
	std::map<int, CBone*> m_invalidelemmap;
	std::vector<CBone*> m_cpvec;

};

#endif //__FRAMECOPYDLG_H_
