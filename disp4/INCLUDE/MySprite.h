#ifndef MYSPRITEH
#define MYSPRITEH

#include <d3dx9.h>
#include <Coef.h>


class CMySprite
{
public:
	CMySprite( LPDIRECT3DDEVICE9 pdev );
	~CMySprite();

	int Create( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DCOLOR srccol );
	int CreateDecl();
	int SetPos( D3DXVECTOR3 srcpos );
	int SetSize( D3DXVECTOR2 srcsize );
	int SetColor( D3DXVECTOR4 srccol );
	int OnRender( LPDIRECT3DTEXTURE9 ptex = 0 );

private:
	int InitParams();
	int DestroyObjs();


public:
	LPDIRECT3DDEVICE9 m_pdev;
	int m_texid;
	SPRITEV m_v[4];
	IDirect3DVertexDeclaration9* m_decl;

	D3DXVECTOR3 m_pos;
	D3DXVECTOR2 m_size;
	D3DXVECTOR4 m_col;
};

#endif