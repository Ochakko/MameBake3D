#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <MySprite.h>

#define DBGH
#include <dbg.h>

#include <TexBank.h>
#include <TexElem.h>

#include <map>
#include <algorithm>
#include <iostream>
#include <iterator>
using namespace std;

extern CTexBank* g_texbank;
extern WCHAR g_basedir[ MAX_PATH ];

extern ID3DXEffect*		g_pEffect;
extern D3DXHANDLE g_hRenderSprite;

extern D3DXHANDLE g_hdiffuse;
extern D3DXHANDLE g_hMeshTexture;


CMySprite::CMySprite( LPDIRECT3DDEVICE9 pdev )
{
	InitParams();
	m_pdev = pdev;
}
CMySprite::~CMySprite()
{
	DestroyObjs();
}

int CMySprite::InitParams()
{
	m_pdev = 0;
	m_texid = -1;
	m_decl = 0;

	m_pos = ChaVector3( 0.0f, 0.0f, 0.0f );
	m_size = D3DXVECTOR2( 1.0f, 1.0f );
	m_col = ChaVector4( 1.0f, 1.0f, 1.0f, 1.0f );

	m_v[0].pos = ChaVector4( -1.0f, +1.0f, 0.0f, 1.0f );
	m_v[1].pos = ChaVector4( +1.0f, +1.0f, 0.0f, 1.0f );
	m_v[2].pos = ChaVector4( +1.0f, -1.0f, 0.0f, 1.0f );
	m_v[3].pos = ChaVector4( -1.0f, -1.0f, 0.0f, 1.0f );

	m_v[0].uv = D3DXVECTOR2( 0.0f, 0.0f );
	m_v[1].uv = D3DXVECTOR2( 1.0f, 0.0f );
	m_v[2].uv = D3DXVECTOR2( 1.0f, 1.0f );
	m_v[3].uv = D3DXVECTOR2( 0.0f, 1.0f );

	return 0;
}
int CMySprite::DestroyObjs()
{
	if( m_decl ){
		m_decl->Release();
		m_decl = 0;
	}
	return 0;
}

int CMySprite::CreateDecl()
{
	D3DVERTEXELEMENT9 decl[] = {
		//pos[4]
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		//uv
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()

	};

	HRESULT hr;
	hr = m_pdev->CreateVertexDeclaration( decl, &m_decl );
	if( hr != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}

int CMySprite::Create( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DCOLOR srccol )
{
	CallF( CreateDecl(), return 1 );

	if( !g_texbank ){
		g_texbank = new CTexBank( m_pdev );
		_ASSERT( g_texbank );
	}

	g_texbank->AddTex( srcpath, srcname, srctransparent, srcpool, srccol, &m_texid );
	if( m_texid < 0 ){
		_ASSERT( 0 );
		return 1;
	}


	return 0;
}
int CMySprite::SetPos( ChaVector3 srcpos )
{
	m_pos = srcpos;

	return 0;
}
int CMySprite::SetSize( D3DXVECTOR2 srcsize )
{
	m_size = srcsize;

	return 0;
}
int CMySprite::SetColor( ChaVector4 srccol )
{
	m_col = srccol;
	return 0;
}
int CMySprite::OnRender( LPDIRECT3DTEXTURE9 ptex )
{
	SPRITEV renderv[4];
	int vno;
	for( vno = 0; vno < 4; vno++ ){
		renderv[vno].uv = m_v[vno].uv;
	}

	float maxx, minx, maxy, miny;
	maxx = m_pos.x + 0.5f * m_size.x;
	minx = m_pos.x - 0.5f * m_size.x;
	maxy = m_pos.y + 0.5f * m_size.y;
	miny = m_pos.y - 0.5f * m_size.y;

	renderv[0].pos = ChaVector4( minx, maxy, m_pos.z, 1.0f );
	renderv[1].pos = ChaVector4( maxx, maxy, m_pos.z, 1.0f );
	renderv[2].pos = ChaVector4( maxx, miny, m_pos.z, 1.0f );
	renderv[3].pos = ChaVector4( minx, miny, m_pos.z, 1.0f );

	HRESULT hr;
	hr = g_pEffect->SetValue( g_hdiffuse, &m_col, sizeof( ChaVector4 ) );
	_ASSERT( !hr );


	LPDIRECT3DTEXTURE9 disptex = 0;
	if( ptex == 0 ){
		if( m_texid >= 0 ){
			CTexElem* findtex = g_texbank->GetTexElem( m_texid );
			if( findtex ){
				disptex = findtex->GetPTex();
				_ASSERT( disptex );
			}else{
				_ASSERT( 0 );
			}
		}else{
			_ASSERT( 0 );
			disptex = 0;
		}
	}else{
		disptex = ptex;
	}
	_ASSERT( disptex );
	hr = g_pEffect->SetTexture( g_hMeshTexture, disptex );
	_ASSERT( !hr );


	hr = g_pEffect->SetTechnique( g_hRenderSprite );
	_ASSERT( hr == D3D_OK );
	hr = g_pEffect->Begin( NULL, 0 );
	_ASSERT( hr == D3D_OK );

	hr = g_pEffect->BeginPass( 0 );
	_ASSERT( hr == D3D_OK );

	hr = m_pdev->SetVertexDeclaration( m_decl );
	_ASSERT( hr == D3D_OK );

	hr = m_pdev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN
					, 2, renderv, sizeof( SPRITEV ) );
	_ASSERT( hr == D3D_OK );

	hr = g_pEffect->EndPass();
	_ASSERT( hr == D3D_OK );

	hr = g_pEffect->End();
	_ASSERT( hr == D3D_OK );


	return 0;
}

