#include "stdafx.h"
// FrameCopyDlg.cpp : CFrameCopyDlg のインプリメンテーション
#include "FrameCopyDlg.h"
#include <Model.h>
#include <Bone.h>

#define DBGH
#include <dbg.h>

using namespace std;

//extern
extern void OnDSUpdate();
//extern void OnDSMouseHereApeal();
//extern LONG g_undertrackingRMenu;
//extern LONG g_underApealingMouseHere;



/////////////////////////////////////////////////////////////////////////////
// CFrameCopyDlg

CFrameCopyDlg::CFrameCopyDlg()
{
	m_samemodelflag = 0;
	InitParams();
}

int CFrameCopyDlg::InitParams()
{
	m_inittimerflag = false;
	m_timerid = 339;


	ZeroMemory(m_tmpmqopath, sizeof(WCHAR) * MAX_PATH);

	m_hImageList = 0;
	m_iImage = 0;
	m_iSelect = 0;

	m_timap.clear();
	m_selecteditem = 0;
	m_selectedno = 0;

	if( m_samemodelflag == 0 ){
		m_model = 0;

		m_slotno = 0;

		int slotno;
		for( slotno = 0; slotno < FCSLOTNUM; slotno++ ){
			swprintf_s( &(m_slotname[slotno][0]), SLOTNAMELEN, L"Slot %d", slotno );
		}

		::ZeroMemory( m_influencenum, sizeof( int ) * FCSLOTNUM );
		::ZeroMemory( &(m_influencelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );

		::ZeroMemory( m_ignorenum, sizeof( int ) * FCSLOTNUM );
		::ZeroMemory( &(m_ignorelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );
	}

	m_validelemmap.clear();
	m_invalidelemmap.clear();
	m_cpvec.clear();

	m_samemodelflag = 0;

	return 0;
}

int CFrameCopyDlg::SetModel( CModel* srcmodel )
{
	if( m_model == srcmodel ){
		m_samemodelflag = 1;
	}else{
		m_samemodelflag = 0;
	}
	m_model = srcmodel;
	return 0;
}


CFrameCopyDlg::~CFrameCopyDlg()
{
	DestroyObjs();
}
	
int CFrameCopyDlg::DestroyObjs()
{
	//KillTimer(m_timerid);

	if( m_hImageList ){
		ImageList_Destroy( m_hImageList );
	}

	m_timap.clear();

	m_validelemmap.clear();
	m_invalidelemmap.clear();
	m_cpvec.clear();

	return 0;
}

int CFrameCopyDlg::SetupDlg( CModel* srcmodel )
{
	int ret;

	DestroyObjs();
	InitParams();

	m_model = srcmodel;

	m_dlg_wnd = m_hWnd;
	m_tree_wnd = GetDlgItem( IDN_TREE1 );
	_ASSERT( m_tree_wnd );
	m_list_wnd = GetDlgItem( IDN_LIST1 );
	_ASSERT( m_list_wnd );
	m_list2_wnd = GetDlgItem( IDN_LIST2 );
	_ASSERT( m_list2_wnd );
	m_combo_wnd = GetDlgItem( IDC_SLOTCOMBO );
	_ASSERT( m_combo_wnd );
	m_slotname_wnd = GetDlgItem( IDC_SLOTNAME );
	_ASSERT( m_slotname_wnd );

	CreateImageList();

	//SetTimer(m_timerid, 20);

	if( m_model->GetTopBone() ){
		ret = FillTree();
		if( ret ){
			_ASSERT( 0 );
			return 1;
		}

		CreateCombo();

		ret = ParamsToDlg();
		_ASSERT( !ret );
	}

	return 0;
}

LRESULT CFrameCopyDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//InitCommonControls();
	SetupDlg( m_model );
	StartTimer();
	return 1;  // システムにフォーカスを設定させます
}

LRESULT CFrameCopyDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//外部クラスから、wNotifyCode = 999 で呼び出されることがある。（ダイアログは表示されていない状態）
	
	EndTimer();

	GetDlgItemTextW( IDC_SLOTNAME, &(m_slotname[m_slotno][0]), SLOTNAMELEN );

	int bonenum = m_model->GetBoneListSize();

	m_validelemmap.clear();
	m_invalidelemmap.clear();
	m_cpvec.clear();

	int i, validno;
	for( i = 0; i < m_influencenum[m_slotno]; i++ ){
		validno = m_influencelist[m_slotno][ i ];
		SetTree2ListReq( 0, validno, 0 );
	}

	int j, invalidno;
	for( j = 0; j < m_ignorenum[m_slotno]; j++ ){
		invalidno = m_ignorelist[m_slotno][ j ];
		SetTree2ListReq( 1, invalidno, 0 );
	}

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_model->GetBoneListBegin(); itrbone != m_model->GetBoneListEnd(); itrbone++ ){
		int chkboneno = itrbone->first;
		CBone* chkbone = itrbone->second;
		if (chkbone){
			CBone* valbone = m_validelemmap[chkboneno];
			CBone* invalbone = m_invalidelemmap[chkboneno];

			if (valbone && !invalbone){
				m_cpvec.push_back(chkbone);
			}
		}
	}

	EndDialog(wID);

	return 0;
}

LRESULT CFrameCopyDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndTimer();

	EndDialog(wID);
	return 0;
}

int CFrameCopyDlg::FillTree()
{
	//m_selecteditem = hRoot;

	int shdnum;
	shdnum = m_model->GetBoneListSize();

	m_timap.clear();

	m_selectedno = 0;
	m_selecteditem = 0;
	AddBoneToTree( m_model->GetTopBone(), 1, 1 );


	map<int, HTREEITEM>::iterator itrtree;
	for( itrtree = m_timap.begin(); itrtree != m_timap.end(); itrtree++ ){
		HTREEITEM* curti = &(itrtree->second);
		TreeView_Expand( m_tree_wnd, *curti, TVE_EXPAND );
	}
	TreeView_SelectSetFirstVisible( m_tree_wnd, m_selecteditem );

	return 0;
}
void CFrameCopyDlg::AddBoneToTree( CBone* srcbone, int addbroflag, int addtolast )
{
	//addtolastは初回のTVAddのみを制御する。
	//( CShdElemのTree構造と同期するため。)

	HTREEITEM parTI;
	CBone* parentbone = srcbone->GetParent();
	if( parentbone ){
		parTI = m_timap[ parentbone->GetBoneNo() ];
	}else{
		parTI = TreeView_GetRoot( m_tree_wnd );
	}

	HTREEITEM newTI;
	newTI = TVAdd( parTI, (WCHAR*)srcbone->GetWBoneName(), srcbone->GetBoneNo(), m_iImage, m_iSelect, addtolast );
	if( !newTI ){
		_ASSERT( 0 );
		return;
	}

	m_timap[ srcbone->GetBoneNo() ] = newTI;
	if( m_selectedno == 0 ){
		m_selecteditem = newTI;
		m_selectedno = srcbone->GetBoneNo();
		TreeView_SelectItem( m_tree_wnd, m_selecteditem );
	}

///////////
	if( srcbone->GetChild() ){
		AddBoneToTree( srcbone->GetChild(), 1, 1 );
	}

	///////////
	if( addbroflag ){
		if( srcbone->GetBrother() ){
			AddBoneToTree( srcbone->GetBrother(), 1, 1 );
		}
	}

}
HTREEITEM CFrameCopyDlg::TVAdd( HTREEITEM parentTI, WCHAR* srcname, int srcno, int imageno, int selectno, int addtolast )
{
	//ここのDbg文は取り除かない。
//	DbgOut( "FrameCopyDlg : TVAdd : srcname %s, srcno %d, imageno %d, selectno %d, addtolast %d\n",
//		srcname, srcno, imageno, selectno, addtolast );

	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.pszText = srcname;

	tvi.mask |= TVIF_PARAM;
	tvi.lParam = (LPARAM)srcno;

	if( imageno != -1 ){
		tvi.mask |= TVIF_IMAGE;
		tvi.iImage = imageno;
	}

	if( selectno != -1 ){
		tvi.mask |= TVIF_SELECTEDIMAGE;
		tvi.iSelectedImage = selectno;
	}

	TVINSERTSTRUCT tvins;
	tvins.item = tvi;

	if( addtolast )
		tvins.hInsertAfter = TVI_LAST;
	else
		tvins.hInsertAfter = TVI_FIRST;

	tvins.hParent = parentTI;

	return TreeView_InsertItem( m_tree_wnd, &tvins );

}

void CFrameCopyDlg::CreateImageList()
{
	m_tree_wnd.SetWindowLong( GWL_STYLE, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS
		);

	HBITMAP hBitmap;
	m_hImageList = ImageList_Create( 16, 16, ILC_COLOR, 2, 10 );
	
	hBitmap = LoadBitmap( (HINSTANCE)GetModuleHandle(NULL),
		MAKEINTRESOURCE( IDB_BONE_IMAGE ) );
	if( hBitmap == NULL ){
		_ASSERT( 0 );
	}
	m_iImage = ImageList_Add( m_hImageList, hBitmap, (HBITMAP)0 );
	if( m_iImage == -1 ){
		_ASSERT( 0 );
	}
	DeleteObject( hBitmap );

	hBitmap = LoadBitmap( (HINSTANCE)GetModuleHandle(NULL),
		MAKEINTRESOURCE( IDB_BONE_SELECT ) );
	if( hBitmap == NULL ){
		_ASSERT( 0 );
	}
	m_iSelect = ImageList_Add( m_hImageList, hBitmap, (HBITMAP)0 );
	if( m_iSelect == -1 ){
		_ASSERT( 0 );
	}
	DeleteObject( hBitmap );

	HIMAGELIST retIL;
	retIL = TreeView_SetImageList( m_tree_wnd,
		m_hImageList, TVSIL_NORMAL );	
}
int CFrameCopyDlg::ParamsToDlg()
{
	m_slotname_wnd.SetWindowTextW( &(m_slotname[ m_slotno ][0]) );

	m_list_wnd.SendMessage( LB_RESETCONTENT, 0, 0 );
	
	int listno;
	for( listno = 0; listno < m_influencenum[m_slotno]; listno++ ){
		LRESULT lres;

		const WCHAR* bonename = m_model->GetBoneByID( m_influencelist[m_slotno][listno] )->GetWBoneName();
		WCHAR addstr[2048];
		ZeroMemory( addstr, sizeof( WCHAR ) * 2048 );
		swprintf_s( addstr, 2048, L"%d : %s", m_influencelist[m_slotno][listno], bonename );

		lres = m_list_wnd.SendMessage( LB_ADDSTRING, 0, (LPARAM)addstr );
		if( (lres == LB_ERR) || (lres == LB_ERRSPACE) ){
			_ASSERT( 0 );
			return 1;
		}
	}

	///////////////
	
	m_list2_wnd.SendMessage( LB_RESETCONTENT, 0, 0 );
	
	int listno2;
	for( listno2 = 0; listno2 < m_ignorenum[m_slotno]; listno2++ ){
		LRESULT lres;

		const WCHAR* bonename = m_model->GetBoneByID( m_ignorelist[m_slotno][listno2] )->GetWBoneName();
		WCHAR addstr[2048];
		ZeroMemory( addstr, sizeof( WCHAR ) * 2048 );
		//swprintf_s( addstr, 2048, L"%d : %s", m_influencelist[m_slotno][listno2], bonename );
		swprintf_s(addstr, 2048, L"%d : %s", m_ignorelist[m_slotno][listno2], bonename);

		lres = m_list2_wnd.SendMessage( LB_ADDSTRING, 0, (LPARAM)addstr );
		if( (lres == LB_ERR) || (lres == LB_ERRSPACE) ){
			_ASSERT( 0 );
			return 1;
		}
	}

	return 0;
}

LRESULT CFrameCopyDlg::OnSelchangedTree1(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	// TODO : ｺﾝﾄﾛｰﾙの通知ﾊﾝﾄﾞﾗ用のｺｰﾄﾞを追加してください。
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
	TVITEM tvi = pnmtv->itemNew;

	m_selecteditem = tvi.hItem;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	if( !TreeView_GetItem( m_tree_wnd, &tvi ) )
		return 0;

	m_selectedno = (int)(tvi.lParam);

	return 0;
}

LRESULT CFrameCopyDlg::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( m_selectedno == 0 )
		return 0;

	if( m_influencenum[m_slotno] >= FRAMECOPYLISTLENG ){
		MessageBox( L"これ以上、指定できません。", L"バッファ不足エラー", MB_OK );
		return 0;
	}

	int listno;
	int findflag = 0;
	for( listno = 0; listno < m_influencenum[m_slotno]; listno++ ){
		if( m_influencelist[m_slotno][listno] == m_selectedno ){
			findflag = 1;
			break;
		}
	}
	if( findflag == 1 )
		return 0;

	m_influencelist[m_slotno][m_influencenum[m_slotno]] = m_selectedno;
	(m_influencenum[m_slotno])++;

	LRESULT lres;

	const WCHAR* bonename = m_model->GetBoneByID( m_selectedno )->GetWBoneName();
	WCHAR addstr[2048];
	ZeroMemory( addstr, sizeof( WCHAR ) * 2048 );
	swprintf_s( addstr, 2048, L"%d : %s", m_selectedno, bonename );

	lres = m_list_wnd.SendMessage( LB_ADDSTRING, 0, (LPARAM)addstr );
	if( lres == LB_ERR ){
		_ASSERT( 0 );
		return -1;
	}

	return 0;
}

LRESULT CFrameCopyDlg::OnAdd2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( m_selectedno == 0 )
		return 0;

	if( m_ignorenum[m_slotno] >= FRAMECOPYLISTLENG ){
		MessageBox( L"これ以上、指定できません。", L"バッファ不足エラー", MB_OK );
		return 0;
	}

	int listno;
	int findflag = 0;
	for( listno = 0; listno < m_ignorenum[m_slotno]; listno++ ){
		if( m_ignorelist[m_slotno][listno] == m_selectedno ){
			findflag = 1;
			break;
		}
	}
	if( findflag == 1 )
		return 0;

	m_ignorelist[m_slotno][m_ignorenum[m_slotno]] = m_selectedno;
	(m_ignorenum[m_slotno])++;

	int ret;

	const WCHAR* bonename = m_model->GetBoneByID( m_selectedno )->GetWBoneName();
	WCHAR addstr[2048];
	ZeroMemory( addstr, sizeof( WCHAR ) * 2048 );
	swprintf_s( addstr, 2048, L"%d : %s", m_selectedno, bonename );

	ret = (int)m_list2_wnd.SendMessage( LB_ADDSTRING, 0, (LPARAM)addstr );
	if( ret == LB_ERR ){
		_ASSERT( 0 );
		return -1;
	}

	return 0;
}

LRESULT CFrameCopyDlg::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int selindex;
	selindex = (int)m_list_wnd.SendMessage( LB_GETCURSEL, 0, 0 );
	if( selindex == LB_ERR ){
		//_ASSERT( 0 );
		//return -1;
		
		//何も選択していないときもLB_ERRが返る
		return 0;
	}
	if( (selindex < 0) || (selindex >= m_influencenum[m_slotno]) ){
		_ASSERT( 0 );
		return -1;
	}

	int ret;
	ret = (int)m_list_wnd.SendMessage( LB_DELETESTRING, (WPARAM)selindex, 0 );
	if( ret == LB_ERR ){
		_ASSERT( 0 );
		return 0;
	}

	int listno;
	for( listno = selindex; listno < (m_influencenum[m_slotno] - 1); listno++ ){
		m_influencelist[m_slotno][listno] = m_influencelist[m_slotno][listno + 1];
	}
	(m_influencenum[m_slotno])--;


	return 0;
}

LRESULT CFrameCopyDlg::OnDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int selindex;
	selindex = (int)m_list2_wnd.SendMessage( LB_GETCURSEL, 0, 0 );
	if( selindex == LB_ERR ){
		//_ASSERT( 0 );
		//return -1;
		
		//何も選択していないときもLB_ERRが返る
		return 0;
	}
	if( (selindex < 0) || (selindex >= m_ignorenum[m_slotno]) ){
		_ASSERT( 0 );
		return -1;
	}

	int ret;
	ret = (int)m_list2_wnd.SendMessage( LB_DELETESTRING, (WPARAM)selindex, 0 );
	if( ret == LB_ERR ){
		_ASSERT( 0 );
		return 0;
	}

	int listno;
	for( listno = selindex; listno < (m_ignorenum[m_slotno] - 1); listno++ ){
		m_ignorelist[m_slotno][listno] = m_ignorelist[m_slotno][listno + 1];
	}
	(m_ignorenum[m_slotno])--;


	return 0;
}

LRESULT CFrameCopyDlg::OnAllDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_list_wnd.SendMessage( LB_RESETCONTENT, 0, 0 );
	m_influencenum[m_slotno] = 0;
	ZeroMemory( &(m_influencelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );

	return 0;
}

LRESULT CFrameCopyDlg::OnAllDelete2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_list2_wnd.SendMessage( LB_RESETCONTENT, 0, 0 );
	m_ignorenum[m_slotno] = 0;
	ZeroMemory( &(m_ignorelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );

	return 0;
}

int CFrameCopyDlg::SetTree2ListReq( int validorinvalid, int srcno, int addbroflag )
{

	CBone* curbone = 0;
	int bonenum = m_model->GetBoneListSize();

	if( (srcno >= 0) && (srcno < bonenum) ){
	
		curbone = m_model->GetBoneByID( srcno );
		if( curbone ){
			if( validorinvalid == 0 ){
				m_validelemmap[ srcno ] = curbone;
			}else{
				m_invalidelemmap[ srcno ] = curbone;
			}
		}
	}else{
		_ASSERT( 0 );
		return 1;
	}

	_ASSERT( curbone );

	if( curbone->GetChild() ){
		SetTree2ListReq( validorinvalid, curbone->GetChild()->GetBoneNo(), 1 );
	}

	if( addbroflag == 1 ){
		if( curbone->GetBrother() ){
			SetTree2ListReq( validorinvalid, curbone->GetBrother()->GetBoneNo(), 1 );
		}
	}

	return 0;
}

int CFrameCopyDlg::CreateCombo()
{
	m_combo_wnd.SendMessage( CB_RESETCONTENT, 0, 0 );

	int selno;
	for( selno = 0; selno < FCSLOTNUM; selno++ ){
		m_combo_wnd.SendMessage( CB_ADDSTRING, 0, (LPARAM)&(m_slotname[selno]) );
	}
	m_slotno = 0;
	m_combo_wnd.SendMessage( CB_SETCURSEL, 0, 0 );

	return 0;
}


LRESULT CFrameCopyDlg::OnSelCombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItemTextW( IDC_SLOTNAME, &(m_slotname[m_slotno][0]), SLOTNAMELEN );

	int combono;
	combono = (int)m_combo_wnd.SendMessage( CB_GETCURSEL, 0, 0 );
	if( combono == CB_ERR )
		return 0;

	CreateCombo();

	m_slotno = combono;
	m_combo_wnd.SendMessage( CB_SETCURSEL, m_slotno, 0 );

	ParamsToDlg();

	return 0;
}

LRESULT CFrameCopyDlg::OnTimer(UINT, WPARAM, LPARAM, BOOL&)
{
	OnDSUpdate();
	return TRUE;
}


LRESULT CFrameCopyDlg::OnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LoadTBOFile();
	return 0;
}
LRESULT CFrameCopyDlg::OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	WriteTBOFile();
	return 0;
}


int CFrameCopyDlg::WriteTBOFile()
{
	if (!m_model) {
		return 0;
	}
	if (!m_model->GetCurMotInfo()) {
		return 0;
	}

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = m_dlg_wnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"TargetBone(*.tbo)\0*.tbo\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = m_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	//s_getfilenamehwnd = 0;
	//s_getfilenametreeview = 0;
	//HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
	//	WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	//InterlockedExchange(&g_undertrackingRMenu, 1);

	if (GetOpenFileNameW(&ofn) == IDOK) {
		HANDLE hfile = CreateFile(m_tmpmqopath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hfile == INVALID_HANDLE_VALUE) {
			DbgOut(L"TBOFile : WriteTBOFile : file open error !!!\n");
			_ASSERT(0);
			return 1;
		}

		char TBOheader[256];
		::ZeroMemory(TBOheader, sizeof(char) * 256);
		strcpy_s(TBOheader, 256, "MB3DTargetBoneFile ver1.0.0.8");
		DWORD wleng = 0;
		WriteFile(hfile, TBOheader, sizeof(char) * 256, &wleng, NULL);
		if (wleng != (sizeof(char) * 256)) {
			_ASSERT(0);
			return 1;
		}

		//WCHAR m_slotname[FCSLOTNUM][SLOTNAMELEN];
		wleng = 0;
		WriteFile(hfile, m_slotname, sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN, &wleng, NULL);
		if (wleng != (sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN)) {
			_ASSERT(0);
			return 1;
		}

		//int m_influencenum[FCSLOTNUM];
		wleng = 0;
		WriteFile(hfile, m_influencenum, sizeof(int) * FCSLOTNUM, &wleng, NULL);
		if (wleng != (sizeof(int) * FCSLOTNUM)) {
			_ASSERT(0);
			return 1;
		}


		//int m_influencelist[FCSLOTNUM][FRAMECOPYLISTLENG];
		wleng = 0;
		WriteFile(hfile, m_influencelist, sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG, &wleng, NULL);
		if (wleng != (sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG)) {
			_ASSERT(0);
			return 1;
		}

		//int m_ignorenum[FCSLOTNUM];
		wleng = 0;
		WriteFile(hfile, m_ignorenum, sizeof(int) * FCSLOTNUM, &wleng, NULL);
		if (wleng != (sizeof(int) * FCSLOTNUM)) {
			_ASSERT(0);
			return 1;
		}
		//int m_ignorelist[FCSLOTNUM][FRAMECOPYLISTLENG];
		wleng = 0;
		WriteFile(hfile, m_ignorelist, sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG, &wleng, NULL);
		if (wleng != (sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG)) {
			_ASSERT(0);
			return 1;
		}


		FlushFileBuffers(hfile);
		CloseHandle(hfile);
	}

	//InterlockedExchange(&g_undertrackingRMenu, 0);
	//UnhookWinEvent(hhook);
	//s_getfilenamehwnd = 0;
	//s_getfilenametreeview = 0;

	return 0;
}


bool CFrameCopyDlg::ValidateTBOFile(char* dstTBOheader, char* srcbuf, DWORD bufleng)
{
	if (!dstTBOheader || !srcbuf || (bufleng <= 0)) {
		_ASSERT(0);
		return false;
	}

	if (bufleng <= (sizeof(char) * 256)) {
		_ASSERT(0);
		return false;
	}

	MoveMemory(dstTBOheader, srcbuf, sizeof(char) * 256);

	//typedef struct tag_CPTheader
	//{
	//	char magicstr[32];//EvaluateGlobalPosition
	//	char version[16];
	//	char fbxdate[256];
	//	int animno;
	//	int jointnum;
	//	int framenum;
	//	int reserved;
	//}CPTHEADER;

	int magicstrlen;
	magicstrlen = strlen(dstTBOheader);
	if ((magicstrlen <= 0) || (magicstrlen >= 256)) {
		_ASSERT(0);
		return false;
	}
	int cmp;
	cmp = strcmp(dstTBOheader, "MB3DTargetBoneFile ver1.0.0.8");
	if (cmp != 0) {
		_ASSERT(0);
		return false;
	}

	DWORD datasize;
	datasize = (bufleng - sizeof(char) * 256);
	DWORD chksize;
	chksize = (sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN) +
		(sizeof(int) * FCSLOTNUM) +
		(sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG) +
		(sizeof(int) * FCSLOTNUM) +
		(sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG);

	if (datasize != chksize) {
		_ASSERT(0);
		return false;
	}


	return true;
}


bool CFrameCopyDlg::LoadTBOFile()
{
	if (!m_model) {
		_ASSERT(0);
		return false;
	}
	if (!m_model->GetCurMotInfo()) {
		_ASSERT(0);
		return false;
	}

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = m_dlg_wnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"TargetBone(*.tbo)\0*.tbo\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = m_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	//s_getfilenamehwnd = 0;
	//s_getfilenametreeview = 0;
	//HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
	//	WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	//InterlockedExchange(&g_undertrackingRMenu, 1);

	if (GetOpenFileNameW(&ofn) == IDOK) {
		HANDLE hfile;
		hfile = CreateFile(m_tmpmqopath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hfile == INVALID_HANDLE_VALUE) {
			_ASSERT(0);
			return false;
		}

		DWORD sizehigh;
		DWORD bufleng;
		bufleng = GetFileSize(hfile, &sizehigh);
		if (bufleng <= 0) {
			_ASSERT(0);
			return false;
		}
		if (sizehigh != 0) {
			_ASSERT(0);
			return false;
		}
		char* newbuf;
		newbuf = (char*)malloc(sizeof(char) * bufleng);//bufleng + 1
		if (!newbuf) {
			_ASSERT(0);
			return false;
		}
		ZeroMemory(newbuf, sizeof(char) * bufleng);
		DWORD rleng, readleng;
		rleng = bufleng;
		BOOL bsuccess;
		bsuccess = ReadFile(hfile, (void*)newbuf, rleng, &readleng, NULL);
		if (!bsuccess || (rleng != readleng)) {
			_ASSERT(0);
			CloseHandle(hfile);
			if (!newbuf) {
				_ASSERT(0);
				return false;
			}
			return false;
		}


		char TBOheader[256];
		ZeroMemory(TBOheader, sizeof(char) * 256);
		bool isvalid;
		isvalid = ValidateTBOFile(TBOheader, newbuf, bufleng);
		if (!isvalid) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}


		DWORD curpos;
		curpos = sizeof(char) * 256;

		//WCHAR m_slotname[FCSLOTNUM][SLOTNAMELEN];
		if ((curpos + sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		MoveMemory(m_slotname, newbuf + curpos, sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN);
		curpos += sizeof(WCHAR) * FCSLOTNUM * SLOTNAMELEN;


		//int m_influencenum[FCSLOTNUM];
		if ((curpos + sizeof(int) * FCSLOTNUM) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		MoveMemory(m_influencenum, newbuf + curpos, sizeof(int) * FCSLOTNUM);
		curpos += sizeof(int) * FCSLOTNUM;


		//int m_influencelist[FCSLOTNUM][FRAMECOPYLISTLENG];
		if ((curpos + sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		MoveMemory(m_influencelist, newbuf + curpos, sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG);
		curpos += sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG;

		//int m_ignorenum[FCSLOTNUM];
		if ((curpos + sizeof(int) * FCSLOTNUM) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		MoveMemory(m_ignorenum, newbuf + curpos, sizeof(int) * FCSLOTNUM);
		curpos += sizeof(int) * FCSLOTNUM;

		//int m_ignorelist[FCSLOTNUM][FRAMECOPYLISTLENG];
		if ((curpos + sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		MoveMemory(m_ignorelist, newbuf + curpos, sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG);
		curpos += sizeof(int) * FCSLOTNUM * FRAMECOPYLISTLENG;

		ParamsToDlg();

		CloseHandle(hfile);
	}

	//InterlockedExchange(&g_undertrackingRMenu, 0);
	//UnhookWinEvent(hhook);
	//s_getfilenamehwnd = 0;
	//s_getfilenametreeview = 0;

	return true;
}
