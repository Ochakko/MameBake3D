#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <search.h>

#include <windows.h>

#define	DBGH
#include <dbg.h>
#include <crtdbg.h>


#include <TexBank.h>
#include <TexElem.h>

#include <algorithm>
#include <iostream>
#include <iterator>

extern WCHAR g_basedir[ MAX_PATH ];

CTexBank::CTexBank( ID3D10Device* pdev )
{
	InitParams();
	m_pdev = pdev;
}

CTexBank::~CTexBank()
{
	DestroyObjs();
}

int CTexBank::InitParams()
{
	m_pdev = 0;
	return 0;
}
int CTexBank::DestroyObjs()
{
	Invalidate( INVAL_ALL );

	map<int,CTexElem*>::iterator itr;
	for( itr = m_texmap.begin(); itr != m_texmap.end(); itr++ ){
		CTexElem* delte = itr->second;
		if( delte ){
			delete delte;
		}
	}

	m_texmap.clear();

	return 0;
}

CTexElem* CTexBank::ExistTex( WCHAR* srcpath, WCHAR* srcname, int srctransparent )
{
	map<int,CTexElem*>::iterator finditr = m_texmap.end();

	map<int,CTexElem*>::iterator itr;
	for( itr = m_texmap.begin(); itr != m_texmap.end(); itr++ ){
		CTexElem* curelem = itr->second;
		if( curelem ){
			int cmpname, cmppath;
			cmpname = wcscmp( srcname, curelem->GetName() );
			if( cmpname == 0 ){
				cmppath = wcscmp( srcpath, curelem->GetPath() );
				if( cmppath == 0 ){
					if( srctransparent == curelem->GetTransparent() ){
						return curelem;
					}
				}
			}
		}
	}

	return 0;
}

int CTexBank::AddTex( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DXCOLOR* srccol, int* dstid )
{
	*dstid = -1;

	CTexElem* sameelem = 0;
	sameelem = ExistTex( srcpath, srcname, srctransparent );
	if( sameelem ){
		*dstid = sameelem->GetID();
		return 2;
	}

	CTexElem* newelem;
	newelem = new CTexElem();
	if( !newelem ){
		_ASSERT( 0 );
		return 1;
	}
	newelem->SetName( srcname );
	newelem->SetPath( srcpath );
	newelem->SetTransparent( srctransparent );
	newelem->SetPool( srcpool );
	if (srccol) {
		newelem->SetTransCol(*srccol);
	}
	CallF( newelem->CreateTexData( m_pdev ), return 1 );

	m_texmap[ newelem->GetID() ] = newelem;
	*dstid = newelem->GetID();

	return 0;
}

int CTexBank::Invalidate( int invalmode )
{
	map<int,CTexElem*>::iterator itr;
	for( itr = m_texmap.begin(); itr != m_texmap.end(); itr++ ){
		CTexElem* delelem = itr->second;
		if( delelem && (invalmode == INVAL_ALL) || ((invalmode == INVAL_ONLYDEFAULT) && (delelem->GetPool() == 0)) ){
			delelem->InvalidateTexData();
		}
	}

	return 0;
}
int CTexBank::Restore()
{
	map<int,CTexElem*>::iterator itr;
	for( itr = m_texmap.begin(); itr != m_texmap.end(); itr++ ){
		CTexElem* telem = itr->second;
		if( telem ){
			CallF( telem->CreateTexData( m_pdev ), return 1 );
		}
	}

	return 0;
}
int CTexBank::DestroyTex( int srcid )
{
	map<int,CTexElem*>::iterator finditr;
	finditr = m_texmap.find( srcid );
	if( finditr == m_texmap.end() ){
		return 0;
	}

	if( finditr->second ){
		delete finditr->second;
	}
	m_texmap.erase( finditr );

	return 0;
}

