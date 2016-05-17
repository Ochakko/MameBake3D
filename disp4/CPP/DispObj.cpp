#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <DispObj.h>

#include <mqomaterial.h>

#include <polymesh3.h>
#include <polymesh4.h>
#include <ExtLine.h>

#include <coef.h>

#define DBGH
#include <dbg.h>

#include <TexBank.h>
#include <TexElem.h>

#include <InfBone.h>

#include <map>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace std;

extern int g_dbgflag;
extern CTexBank* g_texbank;

extern ID3DXEffect*		g_pEffect;
extern D3DXHANDLE g_hRenderBoneL0;
extern D3DXHANDLE g_hRenderBoneL1;
extern D3DXHANDLE g_hRenderBoneL2;
extern D3DXHANDLE g_hRenderBoneL3;
extern D3DXHANDLE g_hRenderNoBoneL0;
extern D3DXHANDLE g_hRenderNoBoneL1;
extern D3DXHANDLE g_hRenderNoBoneL2;
extern D3DXHANDLE g_hRenderNoBoneL3;
extern D3DXHANDLE g_hRenderLine;

extern D3DXHANDLE g_hdiffuse;
extern D3DXHANDLE g_hambient;
extern D3DXHANDLE g_hspecular;
extern D3DXHANDLE g_hpower;
extern D3DXHANDLE g_hemissive;
extern D3DXHANDLE g_hMeshTexture;

extern int	g_nNumActiveLights;


CDispObj::CDispObj()
{
	InitParams();
}
CDispObj::~CDispObj()
{
	DestroyObjs();
}
int CDispObj::InitParams()
{
	m_hasbone = 0;

	m_pdev = 0;
	m_pm3 = 0;//ŠO•”ƒƒ‚ƒŠ
	m_pm4 = 0;

	m_declbone = 0;
	m_declnobone = 0;
	m_declline = 0;

    m_VB = 0;
	m_InfB = 0;
	m_IB = 0;

	return 0;
}
int CDispObj::DestroyObjs()
{
	if( m_declbone ){
		m_declbone->Release();
		m_declbone = 0;
	}
	if( m_declnobone ){
		m_declnobone->Release();
		m_declnobone = 0;
	}
	if( m_declline ){
		m_declline->Release();
		m_declline = 0;
	}

	if( m_VB ){
		m_VB->Release();
		m_VB = 0;
	}

	if( m_InfB ){
		m_InfB->Release();
		m_InfB = 0;
	}

	if( m_IB ){
		m_IB->Release();
		m_IB = 0;
	}

	return 0;
}

int CDispObj::CreateDispObj( LPDIRECT3DDEVICE9 pdev, CPolyMesh3* pm3, int hasbone )
{
	DestroyObjs();

	m_hasbone = hasbone;

	m_pdev = pdev;
	m_pm3 = pm3;

	CallF( CreateDecl(), return 1 );
	CallF( CreateVBandIB(), return 1 );

	return 0;
}
int CDispObj::CreateDispObj( LPDIRECT3DDEVICE9 pdev, CPolyMesh4* pm4, int hasbone )
{
	DestroyObjs();

	m_hasbone = hasbone;

	m_pdev = pdev;
	m_pm4 = pm4;

	CallF( CreateDecl(), return 1 );
	CallF( CreateVBandIB(), return 1 );

	return 0;
}

int CDispObj::CreateDispObj( LPDIRECT3DDEVICE9 pdev, CExtLine* extline )
{
	DestroyObjs();

	m_hasbone = 0;

	m_pdev = pdev;
	m_extline = extline;

	CallF( CreateDecl(), return 1 );
	CallF( CreateVBandIBLine(), return 1 );

	return 0;
}


int CDispObj::CreateDecl()
{
	D3DVERTEXELEMENT9 declbone[] = {
		//pos[4]
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		//normal[3]
		{ 0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },

		//uv
		{ 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		//weight[4]
		{ 1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },

		//boneindex[4]
		{ 1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },


		D3DDECL_END()

	};
	D3DVERTEXELEMENT9 declnobone[] = {
		//pos[4]
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		//normal[3]
		{ 0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },

		//uv
		{ 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		D3DDECL_END()

	};
	D3DVERTEXELEMENT9 declline[] = {
		//pos[4]
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		D3DDECL_END()

	};


	HRESULT hr;
	hr = m_pdev->CreateVertexDeclaration( declbone, &m_declbone );
	if( hr != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}
	hr = m_pdev->CreateVertexDeclaration( declnobone, &m_declnobone );
	if( hr != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}
	hr = m_pdev->CreateVertexDeclaration( declline, &m_declline );
	if( hr != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}
int CDispObj::CreateVBandIB()
{
	HRESULT hr;

	UINT elemleng, infleng;
	DWORD curFVF;

	elemleng = sizeof( PM3DISPV );
	infleng = sizeof( PM3INF );
	curFVF = 0;

	int pmvleng, pmfleng;
	CInfBone* pmib;
	PM3DISPV* pmdv;
	if( m_pm3 ){
		pmvleng = m_pm3->GetOptLeng();
		pmfleng = m_pm3->GetFaceNum();
		pmib = m_pm3->GetInfBone();
		pmdv = m_pm3->GetDispV();
	}else if( m_pm4 ){
		pmvleng = m_pm4->GetOptLeng();
		pmfleng = m_pm4->GetFaceNum();
		pmib = 0;
		pmdv = m_pm4->GetPm3Disp();
	}


	hr = m_pdev->CreateVertexBuffer( pmvleng * elemleng,
		D3DUSAGE_WRITEONLY, curFVF,
		D3DPOOL_MANAGED,
		//D3DPOOL_DEFAULT,
		&m_VB, NULL );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	if( m_hasbone ){
		hr = m_pdev->CreateVertexBuffer( pmvleng * infleng,
			D3DUSAGE_WRITEONLY, curFVF,
			D3DPOOL_MANAGED,
			//D3DPOOL_DEFAULT,
			&m_InfB, NULL );
		if( FAILED(hr) ){
			_ASSERT( 0 );
			return 1;
		}
	}
	hr = m_pdev->CreateIndexBuffer( pmfleng * 3 * sizeof(int),
		0,
		D3DFMT_INDEX32, 
		D3DPOOL_MANAGED,
		&m_IB, NULL );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}

//////////
	PM3DISPV* pv;
	hr = m_VB->Lock( 0, sizeof( PM3DISPV ) * pmvleng, (void**)&pv, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	memcpy( pv, pmdv, sizeof( PM3DISPV ) * pmvleng );
	m_VB->Unlock();


	if( m_hasbone ){
		PM3INF* pi;
		hr = m_InfB->Lock( 0, sizeof( PM3INF ) * pmvleng, (void**)&pi, 0 );
		if( FAILED(hr) ){
			_ASSERT( 0 );
			return 1;
		}
		if( m_pm4 ){
			memcpy( pi, m_pm4->GetPm3Inf(), sizeof( PM3INF ) * pmvleng );
		}else{
			_ASSERT( 0 );
		}
		m_InfB->Unlock();
	}

	int* pIndices;
	hr = m_IB->Lock( 0, pmfleng * sizeof(int), (void**)&pIndices, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	if( m_pm3 ){
		memcpy( pIndices, m_pm3->GetDispIndex(), m_pm3->GetFaceNum() * 3 * sizeof( int ) );
	}else if( m_pm4 ){
		memcpy( pIndices, m_pm4->GetDispIndex(), m_pm4->GetFaceNum() * 3 * sizeof( int ) );
	}
	m_IB->Unlock();

	return 0;
}

int CDispObj::CreateVBandIBLine()
{
	HRESULT hr;

	UINT elemleng;
	DWORD curFVF;

	elemleng = sizeof( EXTLINEV );
	curFVF = 0;

	hr = m_pdev->CreateVertexBuffer( m_extline->m_linenum * 2 * elemleng,
		D3DUSAGE_WRITEONLY, curFVF,
		D3DPOOL_MANAGED,
		//D3DPOOL_DEFAULT,
		&m_VB, NULL );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	hr = m_pdev->CreateIndexBuffer( m_extline->m_linenum * 2 * sizeof(int),
		0,
		D3DFMT_INDEX32, 
		D3DPOOL_MANAGED,
		&m_IB, NULL );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}

//////////
	EXTLINEV* lv;
	hr = m_VB->Lock( 0, sizeof( EXTLINEV ) * m_extline->m_linenum * 2, (void**)&lv, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	memcpy( lv, m_extline->m_linev, sizeof( EXTLINEV ) * m_extline->m_linenum * 2 );
	m_VB->Unlock();

	int* pIndices;
	hr = m_IB->Lock( 0, m_extline->m_linenum * 2 * sizeof(int), (void**)&pIndices, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	int i;
	for( i = 0; i < (m_extline->m_linenum * 2); i++ ){
		*( pIndices + i ) = i;
	}
	m_IB->Unlock();

	return 0;
}



int CDispObj::RenderNormal( CMQOMaterial* rmaterial, int lightflag, D3DXVECTOR4 diffusemult )
{
	if( !m_pm3 && !m_pm4 ){
		return 0;
	}
	
	HRESULT hr;
	
	CMQOMaterial* curmat = rmaterial;
	_ASSERT( curmat );

	D3DXVECTOR4 diffuse;
	D3DXVECTOR4 curdif4f = curmat->GetDif4F();
	diffuse.w = curdif4f.w * diffusemult.w;
	diffuse.x = curdif4f.x * diffusemult.x;
	diffuse.y = curdif4f.y * diffusemult.y;
	diffuse.z = curdif4f.z * diffusemult.z;


	//if( diffuse.w <= 0.99999f ){
	//	m_pdev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	//}else{
		m_pdev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	//}

//diffuse = D3DXVECTOR4( 0.6f, 0.6f, 0.6f, 1.0f );

	hr = g_pEffect->SetValue( g_hdiffuse, &diffuse, sizeof( D3DXVECTOR4 ) );
	_ASSERT( !hr );
	hr = g_pEffect->SetValue( g_hambient, &(curmat->GetAmb3F()), sizeof( D3DXVECTOR3 ) );
	_ASSERT( !hr );
	hr = g_pEffect->SetValue( g_hspecular, &(curmat->GetSpc3F()), sizeof( D3DXVECTOR3 ) );
	_ASSERT( !hr );
	hr = g_pEffect->SetFloat( g_hpower, curmat->GetPower() );
	_ASSERT( !hr );
	hr = g_pEffect->SetValue( g_hemissive, &(curmat->GetEmi3F()), sizeof( D3DXVECTOR3 ) );
	_ASSERT( !hr );

	if( m_hasbone ){
		if( lightflag != 0 ){
			switch( g_nNumActiveLights ){
			case 1:
				hr = g_pEffect->SetTechnique( g_hRenderBoneL1 );
				_ASSERT( !hr );
				break;
			case 2:
				hr = g_pEffect->SetTechnique( g_hRenderBoneL2 );
				_ASSERT( !hr );
				break;
			case 3:
				hr = g_pEffect->SetTechnique( g_hRenderBoneL3 );
				_ASSERT( !hr );
				break;
			default:
				_ASSERT( 0 );
				hr = g_pEffect->SetTechnique( g_hRenderBoneL1 );
				_ASSERT( !hr );
				break;
			}
		}else{
			hr = g_pEffect->SetTechnique( g_hRenderBoneL0 );
			_ASSERT( !hr );
		}
	}else{
		if( lightflag != 0 ){
			switch( g_nNumActiveLights ){
			case 1:
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
				_ASSERT( !hr );
				break;
			case 2:
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL2 );
				_ASSERT( !hr );
				break;
			case 3:
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL3 );
				_ASSERT( !hr );
				break;
			default:
				_ASSERT( 0 );
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
				_ASSERT( !hr );
				break;
			}
		}else{
			hr = g_pEffect->SetTechnique( g_hRenderNoBoneL0 );
			_ASSERT( !hr );
		}
	}

	LPDIRECT3DTEXTURE9 disptex = 0;

	if( curmat->GetTexID() >= 0 ){
		CTexElem* findtex = g_texbank->GetTexElem( curmat->GetTexID() );
		if( findtex ){
			disptex = findtex->GetPTex();
			_ASSERT( disptex );
		}else{
			disptex = 0;
		}
	}else{
		disptex = 0;
	}


	int passno;
	if( disptex ){
		passno = 0;
	}else{
		passno = 1;
	}

	hr = g_pEffect->SetTexture( g_hMeshTexture, disptex );
	_ASSERT( !hr );
		
/////////////
	HRESULT hres;
	UINT cPasses;
	hres = g_pEffect->Begin(&cPasses, 0);
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	hres = g_pEffect->BeginPass( passno );
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}
	int curnumprim;
	if( m_pm3 ){
		curnumprim = m_pm3->GetFaceNum();
	}else if( m_pm4 ){
		curnumprim = m_pm4->GetFaceNum();
	}else{
		_ASSERT( 0 );
		return 1;
	}

	if( m_hasbone ){
		m_pdev->SetVertexDeclaration( m_declbone );
		m_pdev->SetStreamSource( 0, m_VB, 0, sizeof(PM3DISPV) );
		m_pdev->SetStreamSource( 1, m_InfB, 0, sizeof(PM3INF) );
		m_pdev->SetIndices( m_IB );
	}else{
		m_pdev->SetVertexDeclaration( m_declnobone );
		m_pdev->SetStreamSource( 0, m_VB, 0, sizeof(PM3DISPV) );
		m_pdev->SetIndices( m_IB );
	}

	int rendervnum;
	if( m_pm3 ){
		rendervnum = m_pm3->GetOptLeng();
	}else if( m_pm4 ){
		rendervnum = m_pm4->GetOptLeng();
	}
	hres = m_pdev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										0,
										rendervnum,
										0,
										curnumprim
										);

	hres = g_pEffect->EndPass();
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	hres = g_pEffect->End();
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}


	return 0;
}

int CDispObj::RenderNormalPM3( int lightflag, D3DXVECTOR4 diffusemult )
{
	if( !m_pm3 ){
		return 0;
	}
	if( m_pm3->GetCreateOptFlag() == 0 ){
		return 0;
	}



	HRESULT hr;
	int blno;
	for( blno = 0; blno < m_pm3->GetOptMatNum(); blno++ ){
		MATERIALBLOCK* currb = m_pm3->GetMatBlock() + blno;

		CMQOMaterial* curmat;
			curmat = currb->mqomat;
		if( !curmat ){
			_ASSERT( 0 );
			return 1;
		}

		D3DXVECTOR4 diffuse;
		D3DXVECTOR4 curdif4f = curmat->GetDif4F();
		diffuse.w = curdif4f.w * diffusemult.w;
		diffuse.x = curdif4f.x * diffusemult.x;
		diffuse.y = curdif4f.y * diffusemult.y;
		diffuse.z = curdif4f.z * diffusemult.z;

		hr = g_pEffect->SetValue( g_hdiffuse, &diffuse, sizeof( D3DXVECTOR4 ) );
		_ASSERT( !hr );
		hr = g_pEffect->SetValue( g_hambient, &(curmat->GetAmb3F()), sizeof( D3DXVECTOR3 ) );
		_ASSERT( !hr );
		hr = g_pEffect->SetValue( g_hspecular, &(curmat->GetSpc3F()), sizeof( D3DXVECTOR3 ) );
		_ASSERT( !hr );
		hr = g_pEffect->SetFloat( g_hpower, curmat->GetPower() );
		_ASSERT( !hr );
		hr = g_pEffect->SetValue( g_hemissive, &(curmat->GetEmi3F()), sizeof( D3DXVECTOR3 ) );
		_ASSERT( !hr );


		//if( diffuse.w <= 0.99999f ){
		//	m_pdev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		//}else{
			m_pdev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		//}

/***
		if( m_hasbone ){
			if( lightflag != 0 ){
				switch( g_nNumActiveLights ){
				case 1:
					hr = g_pEffect->SetTechnique( g_hRenderBoneL1 );
					_ASSERT( !hr );
					break;
				case 2:
					hr = g_pEffect->SetTechnique( g_hRenderBoneL2 );
					_ASSERT( !hr );
					break;
				case 3:
					hr = g_pEffect->SetTechnique( g_hRenderBoneL3 );
					_ASSERT( !hr );
					break;
				default:
					_ASSERT( 0 );
					hr = g_pEffect->SetTechnique( g_hRenderBoneL1 );
					_ASSERT( !hr );
					break;
				}
			}else{
				hr = g_pEffect->SetTechnique( g_hRenderBoneL0 );
				_ASSERT( !hr );
			}
		}else{
			if( lightflag != 0 ){
				switch( g_nNumActiveLights ){
				case 1:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
					_ASSERT( !hr );
					break;
				case 2:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL2 );
					_ASSERT( !hr );
					break;
				case 3:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL3 );
					_ASSERT( !hr );
					break;
				default:
					_ASSERT( 0 );
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
					_ASSERT( !hr );
					break;
				}
			}else{
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL0 );
				_ASSERT( !hr );
			}
		}
***/
			if( lightflag != 0 ){
				switch( g_nNumActiveLights ){
				case 1:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
					_ASSERT( !hr );
					break;
				case 2:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL2 );
					_ASSERT( !hr );
					break;
				case 3:
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL3 );
					_ASSERT( !hr );
					break;
				default:
					_ASSERT( 0 );
					hr = g_pEffect->SetTechnique( g_hRenderNoBoneL1 );
					_ASSERT( !hr );
					break;
				}
			}else{
				hr = g_pEffect->SetTechnique( g_hRenderNoBoneL0 );
				_ASSERT( !hr );
			}


		LPDIRECT3DTEXTURE9 disptex = 0;
		if( curmat->GetTexID() >= 0 ){
			CTexElem* findtex = g_texbank->GetTexElem( curmat->GetTexID() );
			if( findtex ){
				disptex = findtex->GetPTex();
			}else{
				disptex = 0;
			}
		}else{
			disptex = 0;
		}

		int passno;
		if( disptex ){
			passno = 0;
		}else{
			passno = 1;
		}

		hr = g_pEffect->SetTexture( g_hMeshTexture, disptex );
		_ASSERT( !hr );
		
	/////////////
		HRESULT hres;
		UINT cPasses;
		hres = g_pEffect->Begin(&cPasses, 0);
		if( hres != D3D_OK ){
			_ASSERT( 0 );
			return 1;
		}

		hres = g_pEffect->BeginPass( passno );
		if( hres != D3D_OK ){
			_ASSERT( 0 );
			return 1;
		}
		int curnumprim;
		curnumprim = currb->endface - currb->startface + 1;
		if( m_hasbone ){
			m_pdev->SetVertexDeclaration( m_declbone );
			m_pdev->SetStreamSource( 0, m_VB, 0, sizeof(PM3DISPV) );
			m_pdev->SetStreamSource( 1, m_InfB, 0, sizeof(PM3INF) );
			m_pdev->SetIndices( m_IB );
		}else{
			m_pdev->SetVertexDeclaration( m_declnobone );
			m_pdev->SetStreamSource( 0, m_VB, 0, sizeof(PM3DISPV) );
			m_pdev->SetIndices( m_IB );
		}

		hres = m_pdev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
											0,
											0,
											m_pm3->GetOptLeng(),
											currb->startface * 3,
											curnumprim
											);

		hres = g_pEffect->EndPass();
		if( hres != D3D_OK ){
			_ASSERT( 0 );
			return 1;
		}

		hres = g_pEffect->End();
		if( hres != D3D_OK ){
			_ASSERT( 0 );
			return 1;
		}
	}

	return 0;
}


int CDispObj::RenderLine( D3DXVECTOR4 diffusemult )
{
	if( !m_extline ){
		return 0;
	}
	if( m_extline->m_linenum <= 0 ){
		return 0;
	}

	HRESULT hr;

	D3DXVECTOR4 diffuse;
	diffuse.w = m_extline->m_color.w * diffusemult.w;
	diffuse.x = m_extline->m_color.x * diffusemult.x;
	diffuse.y = m_extline->m_color.y * diffusemult.y;
	diffuse.z = m_extline->m_color.z * diffusemult.z;

	hr = g_pEffect->SetValue( g_hdiffuse, &diffuse, sizeof( D3DXVECTOR4 ) );
	_ASSERT( !hr );

	hr = g_pEffect->SetTechnique( g_hRenderLine );
	_ASSERT( !hr );
		
	/////////////
	HRESULT hres;
	UINT cPasses;
	hres = g_pEffect->Begin(&cPasses, 0);
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	hres = g_pEffect->BeginPass( 0 );
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}
	int curnumprim;
	curnumprim = m_extline->m_linenum;
	m_pdev->SetVertexDeclaration( m_declline );
	m_pdev->SetStreamSource( 0, m_VB, 0, sizeof(EXTLINEV) );
	m_pdev->SetIndices( m_IB );
	hres = m_pdev->DrawIndexedPrimitive( D3DPT_LINELIST,
										0,
										0,
										m_extline->m_linenum * 2,
										0,
										curnumprim
										);

	hres = g_pEffect->EndPass();
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	hres = g_pEffect->End();
	if( hres != D3D_OK ){
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}

int CDispObj::CopyDispV( CPolyMesh4* pm4 )
{
	m_pm4 = pm4;

	if( !m_VB || !pm4->GetPm3Disp() ){
		_ASSERT( 0 );
		return 1;
	}

	HRESULT hr;
	PM3DISPV* pv;
	hr = m_VB->Lock( 0, sizeof( PM3DISPV ) * m_pm4->GetOptLeng(), (void**)&pv, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	memcpy( pv, m_pm4->GetPm3Disp(), sizeof( PM3DISPV ) * m_pm4->GetOptLeng() );
	m_VB->Unlock();

	return 0;
}

int CDispObj::CopyDispV( CPolyMesh3* pm3 )
{

	m_pm3 = pm3;

	if( !m_VB || !pm3->GetDispV() ){
		_ASSERT( 0 );
		return 1;
	}

	HRESULT hr;
	PM3DISPV* pv;
	hr = m_VB->Lock( 0, sizeof( PM3DISPV ) * m_pm3->GetOptLeng(), (void**)&pv, 0 );
	if( FAILED(hr) ){
		_ASSERT( 0 );
		return 1;
	}
	memcpy( pv, m_pm3->GetDispV(), sizeof( PM3DISPV ) * m_pm3->GetOptLeng() );
	m_VB->Unlock();

	return 0;
}

