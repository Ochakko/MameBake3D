#ifndef TEXELEMH
#define TEXELEMH

#include <d3dx9.h>
#include <wchar.h>

class CTexElem
{
public:
	CTexElem();
	~CTexElem();

	int CreateTexData( LPDIRECT3DDEVICE9 pdev );
	int InvalidateTexData();

private:
	int InitParams();
	int DestroyObjs();

public:
	LPDIRECT3DTEXTURE9 GetPTex(){
		return m_ptex;
	};

	int GetID(){
		return m_id;
	};
	void SetID( int srcval ){
		m_id = srcval;
	};

	const WCHAR* GetName(){
		return m_name;
	};
	void SetName( WCHAR* srcval ){
		wcscpy_s( m_name, MAX_PATH, srcval );
	};

	const WCHAR* GetPath(){
		return m_path;
	};
	void SetPath( WCHAR* srcval ){
		wcscpy_s( m_path, MAX_PATH, srcval );
	};

	void SetTransparent( int srcval ){
		m_transparent = srcval;
	};
	int GetTransparent(){
		return m_transparent;
	};

	int GetPool(){
		return m_pool;
	};
	void SetPool( int srcval ){
		m_pool = srcval;
	};

	void SetTransCol( D3DCOLOR srcval ){
		m_transcol = srcval;
	};

private:
	int m_id;
	WCHAR m_name[ MAX_PATH ];
	WCHAR m_path[ MAX_PATH ];
	int m_transparent;
	int m_pool;
	int m_orgheight, m_orgwidth;
	int m_height, m_width;
	D3DCOLOR m_transcol;
	LPDIRECT3DTEXTURE9 m_ptex;

};

#endif