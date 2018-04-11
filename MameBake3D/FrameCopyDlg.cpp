#include "stdafx.h"
// FrameCopyDlg.cpp : CFrameCopyDlg のインプリメンテーション
#include "FrameCopyDlg.h"
#include <Model.h>
#include <Bone.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CFrameCopyDlg

CFrameCopyDlg::CFrameCopyDlg()
{
	m_samemodelflag = 0;
	InitParams();
}

int CFrameCopyDlg::InitParams()
{

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

		ZeroMemory( m_influencenum, sizeof( int ) * FCSLOTNUM );
		ZeroMemory( &(m_influencelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );

		ZeroMemory( m_ignorenum, sizeof( int ) * FCSLOTNUM );
		ZeroMemory( &(m_ignorelist[0][0]), sizeof( int ) * FCSLOTNUM * FRAMECOPYLISTLENG );
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
	return 1;  // システムにフォーカスを設定させます
}

LRESULT CFrameCopyDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//外部クラスから、wNotifyCode = 999 で呼び出されることがある。（ダイアログは表示されていない状態）


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
	CBone* parbone = srcbone->GetParent();
	if( parbone ){
		parTI = m_timap[ parbone->GetBoneNo() ];
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
		swprintf_s( addstr, 2048, L"%d : %s", m_influencelist[m_slotno][listno2], bonename );

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
