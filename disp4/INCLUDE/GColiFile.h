#ifndef GCOLIFILEH
#define GCOLIFILEH

#include <d3dx9.h>
#include <coef.h>
#include <XMLIO.h>

#include <map>
#include <string>

class BPWorld;
class CModel;

class CGColiFile : public CXMLIO
{
public:
	CGColiFile();
	virtual ~CGColiFile();

	int WriteGColiFile( WCHAR* strpath, BPWorld* srcbpw );
	int LoadGColiFile( CModel* srcmodel, WCHAR* strpath, BPWorld* srcbpw );

private:
	virtual int InitParams();
	virtual int DestroyObjs();

	int WriteColiID();
	int ReadColiID( XMLIOBUF* xmliobuf );

public:
	BPWorld* m_bpw;
	CModel* m_model;
};

#endif