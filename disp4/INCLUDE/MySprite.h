#ifndef MYSPRITEH
#define MYSPRITEH

//#include <d3dx9.h>
#include <ChaVecCalc.h>
#include <Coef.h>


class CMySprite
{
public:
	CMySprite( ID3D10Device* pdev );
	~CMySprite();

	int Create( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DXCOLOR* srccol );
	int CreateDecl();
	int SetPos( ChaVector3 srcpos );
	int SetSize( ChaVector2 srcsize );
	int SetColor( ChaVector4 srccol );
	int OnRender( ID3D10Resource* ptex = 0 );

private:
	int InitParams();
	int DestroyObjs();


private:
	ID3D10Device* m_pdev;
	int m_texid;
	SPRITEV m_v[6];

	D3D10_BUFFER_DESC m_BufferDesc;
	ID3D10InputLayout* m_layout;
	ID3D10Buffer* m_VB;//表示用頂点バッファ。

	ChaVector3 m_pos;
	ChaVector2 m_size;
	ChaVector4 m_col;
	ChaVector3 m_spriteoffset;
	ChaVector2 m_spritescale;
};

#endif