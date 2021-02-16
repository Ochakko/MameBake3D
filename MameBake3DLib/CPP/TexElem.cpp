#include "stdafx.h"
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

//#include <d3dcommon.h>
//#include <dxgi.h>
//#include <d3d10_1.h>
//#include <d3d10.h>
//#include <d3dcompiler.h>
//#include <d3dx10.h>

#include "WICTextureLoader.h"

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
	//m_pool = 0;
	m_pool = 0;
	m_orgheight = 0;
	m_orgwidth = 0;
	m_height = 0;
	m_width = 0;
//m_transcol = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
m_ptex = 0;
m_ResView = 0;
return 0;
}
int CTexElem::DestroyObjs()
{
	if (m_ptex) {
		m_ptex->Release();
		m_ptex = 0;
	}
	if (m_ResView) {
		m_ResView->Release();
		m_ResView = 0;
	}

	InitParams();

	return 0;
}


int CTexElem::CreateTexData(ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext)
{
	if (m_ptex) {
		return 0;
	}
	if (m_ResView) {
		return 0;
	}

	int ret;
	if (m_path[0] != 0) {
		ret = SetCurrentDirectory(m_path);
		//_ASSERT( ret );
	}

	int miplevels = 0;
	//int mipfilter = D3DX_FILTER_TRIANGLE;

	HRESULT hr = DirectX::CreateWICTextureFromFile(
		pdev,
		pd3dImmediateContext,
		m_name,
		&m_ptex,
		&m_ResView
		);
	if (FAILED(hr)) {
		DbgOut(L"TexElem : CreateTexData : CreateTextureFromFileEx error!!! %x, path : %s, name : %s\r\n", hr, m_path, m_name);
		_ASSERT( 0 );
		return -1;
	}

	m_width = m_orgwidth;
	m_height = m_orgheight;

	return 0;

}
//int CTexElem::CreateTexData(ID3D11Device* pdev)
//{
//	if (m_ptex) {
//		return 0;
//	}
//	if (m_ResView) {
//		return 0;
//	}
//
//	int ret;
//	if (m_path[0] != 0) {
//		ret = SetCurrentDirectory(m_path);
//		//_ASSERT( ret );
//	}
//
//	int miplevels = 0;
//	//int mipfilter = D3DX_FILTER_TRIANGLE;
//
//	HRESULT hr;
//	ID3D11Resource* newresource = 0;
//	hr = D3DX10CreateTextureFromFileW(pdev, m_name, NULL, NULL, &newresource, NULL);
//
//	//hr = D3DXCreateTextureFromFileEx( pdev, m_name, 
//	//						D3DX_DEFAULT, D3DX_DEFAULT, 
//	//						miplevels,//!!!!!!! 
//	//						0, D3DFMT_A8R8G8B8, 
//	//						(D3DPOOL)m_pool, 
//	//						D3DX_FILTER_TRIANGLE,// | D3DX_FILTER_MIRROR, 
//	//						//D3DX_FILTER_POINT | D3DX_FILTER_MIRROR,
//	//						mipfilter,//!!!!!!! 
//	//						m_transcol, NULL, NULL, &newtex );
//	if (SUCCEEDED(hr)) {
//		D3DX10_IMAGE_INFO info1;
//		HRESULT hrsize;
//		hrsize = D3DX10GetImageInfoFromFile(m_name, NULL, &info1, NULL);
//		if (SUCCEEDED(hrsize)) {
//			m_orgwidth = info1.Width;
//			m_orgheight = info1.Height;
//		}
//		else {
//			DbgOut(L"texelem : CreateTexData : GetImageInfoFromFile error !!! skip size setting %s\r\n", m_name);
//			_ASSERT(0);
//		}
//	}
//	else {
//		DbgOut(L"TexElem : CreateTexData : CreateTextureFromFileEx error!!! %x, path : %s, name : %s\r\n", hr, m_path, m_name);
//		//_ASSERT( 0 );
//		m_ptex = 0;
//		m_ResView = 0;
//		return 0;
//	}
//
//	m_ptex = newresource;//!!!!!!!!!!!!!
//	pdev->CreateShaderResourceView(m_ptex, 0, &m_ResView);
//
//	if (newresource) {
//		//D3DSURFACE_DESC desc;
//		//hr = newtex->GetLevelDesc( 0, &desc );
//		//if(FAILED(hr)){
//		//	DbgOut( L"texelem : CreateTexData : newtex GetLevelDesc error !!!\r\n" );
//		//	_ASSERT( 0 );
//		//	return 1;
//		//}
//		//m_width = desc.Width;
//		//m_height = desc.Height;
//
//		m_width = m_orgwidth;
//		m_height = m_orgheight;
//	}
//	else {
//		DbgOut(L"texbank : CreateTexData : newtex NULL warning !!! %s\r\n", m_name);
//	}
//
//	return 0;
//}
int CTexElem::InvalidateTexData()
{
	if (m_ptex) {
		m_ptex->Release();
		m_ptex = 0;
	}
	if (m_ResView) {
		m_ResView->Release();
		m_ResView = 0;
	}

	return 0;
}


