#ifndef TEXBANKH
#define TEXBANKH

//#include <d3dx9.h>
#include <ChaVecCalc.h>
#include <wchar.h>
#include <map>
using namespace std;

class CTexElem;

class CTexBank
{
public:
	CTexBank( LPDIRECT3DDEVICE9 pdev );
	~CTexBank();

	int AddTex( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DCOLOR srccol, int* dstid );

	int Invalidate( int invalmode );
	int Restore();

	int DestroyTex( int srcid );

public:
	//accesser
	CTexElem* GetTexElem( int srcindex ){
		return m_texmap[ srcindex ];
	};
	void GetTexElem2( std::map<int,CTexElem*>& dstmap ){
		dstmap = m_texmap;
	};

private:
	int InitParams();
	int DestroyObjs();

	CTexElem* ExistTex( WCHAR* srcpath, WCHAR* srcname, int srctransparent );

private:
	LPDIRECT3DDEVICE9 m_pdev;
	map<int, CTexElem*> m_texmap;
};
#endif

