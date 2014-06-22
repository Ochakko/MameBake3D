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