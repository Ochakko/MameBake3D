//#include <stdafx.h> //É_É~Å[

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


#include <TexElem.h>

static int s_alloccnt = 0;

CTexElem::CTexElem()
{
	InitParams();
	s_alloccnt++;
	m_id = s_alloccnt;
}

CTexElem::~CTexElem()
{
	DestroyObjs();
}

int CTexElem::InitParams()
{
	m_id = -1;
	ZeroMemory( m_name, sizeof( WCHAR ) * MAX_PATH );
	ZeroMemory( m_path, sizeof( WCHAR ) * MAX_PATH );
	m_transparent = 0;
	m_pool = D3DPOOL_DEFAULT;
	m_orgheight = 0;
	m_orgwidth = 0;
	m_height = 0;
	m_width = 0;
	m_transcol = 0;
	m_ptex = 0;

	return 0;
}
int CTexElem::DestroyObjs()
{
	if( m_ptex ){
		m_ptex->Release();
		m_ptex = 0;
	}
	InitParams();

	return 0;
}

int CTexElem::CreateTexData( LPDIRECT3DDEVICE9 pdev )
{
	if( m_ptex ){
		return 0;
	}

	int ret;
	if( m_path[0] != 0 ){
		ret = SetCurrentDirectory( m_path );
		//_ASSERT( ret );
	}

	int miplevels = 0;
	int mipfilter = D3DX_FILTER_TRIANGLE;

	HRESULT hr;
	LPDIRECT3DTEXTURE9 newtex = 0;
	hr = D3DXCreateTextureFromFileEx( pdev, m_name, 
							D3DX_DEFAULT, D3DX_DEFAULT, 
							miplevels,//!!!!!!! 
							0, D3DFMT_A8R8G8B8, 
							(D3DPOOL)m_pool, 
							D3DX_FILTER_TRIANGLE,// | D3DX_FILTER_MIRROR, 
							//D3DX_FILTER_POINT | D3DX_FILTER_MIRROR,
							mipfilter,//!!!!!!! 
							m_transcol, NULL, NULL, &newtex );
	if( hr == D3D_OK ){
		D3DXIMAGE_INFO info1;
		HRESULT hrsize;
		hrsize = D3DXGetImageInfoFromFile( m_name, &info1 );
		if( hrsize == D3D_OK ){
			m_orgwidth = info1.Width;
			m_orgheight = info1.Height;
		}else{
			DbgOut( L"texelem : CreateTexData : GetImageInfoFromFile error !!! skip size setting %s\r\n", m_name );
			_ASSERT( 0 );
		}
	}else{
//		_ASSERT( 0 );
		m_ptex = 0;
		return 0;
	}

	m_ptex = newtex;//!!!!!!!!!!!!!

	if( newtex ){
		D3DSURFACE_DESC desc;
		hr = newtex->GetLevelDesc( 0, &desc );
		if( hr != D3D_OK ){
			DbgOut( L"texelem : CreateTexData : newtex GetLevelDesc error !!!\r\n" );
			_ASSERT( 0 );
			return 1;
		}
		m_width = desc.Width;
		m_height = desc.Height;
	}else{
		DbgOut( L"texbank : CreateTexData : newtex NULL warning !!! %s\r\n", m_name );
	}

	return 0;
}
int CTexElem::InvalidateTexData()
{
	if( m_ptex ){
		m_ptex->Release();
		m_ptex = 0;
	}

	return 0;
}


