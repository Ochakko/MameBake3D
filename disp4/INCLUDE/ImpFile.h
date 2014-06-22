#ifndef IMPFILEH
#define IMPFILEH

#include <d3dx9.h>
#include <coef.h>
#include <XMLIO.h>

#include <map>
#include <string>

class CModel;
class CMQOObject;
class CBone;
class CRigidElem;

class CImpFile : public CXMLIO
{
public:
	CImpFile();
	virtual ~CImpFile();

	int WriteImpFile( WCHAR* strpath, CModel* srcmodel );
	int LoadImpFile( WCHAR* strpath, CModel* srcmodel );
//	int LoadImpFileFromPnd( CPmCipherDll* cipher, int qubindex, CModel* srcmodel, int* newid );

private:
	virtual int InitParams();
	virtual int DestroyObjs();

	void WriteImpReq( CBone* srcbone );
	int WriteImp( CBone* srcbone );

	int ReadBone( XMLIOBUF* xmliobuf );
	int ReadRE( XMLIOBUF* xmliobuf, CBone* srcbone );

public:
	CModel* m_model;
	std::string m_strimp;
};

#endif