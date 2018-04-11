#ifndef TEXBANKH
#define TEXBANKH

//#include <d3dx9.h>
#include <ChaVecCalc.h>
#include <wchar.h>
#include <map>
using namespace std;


//class ID3D10Device;
//class D3DXCOLOR;
class CTexElem;

class CTexBank
{
public:
	CTexBank( ID3D10Device* pdev );
	~CTexBank();

	int AddTex( WCHAR* srcpath, WCHAR* srcname, int srctransparent, int srcpool, D3DXCOLOR* srccol, int* dstid );

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
	ID3D10Device* m_pdev;
	map<int, CTexElem*> m_texmap;
};
#endif

