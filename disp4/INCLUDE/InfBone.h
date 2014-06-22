#ifndef INFBONEH
#define INFBONEH

#include <coef.h>
#include <map>

class CMQOObject;

class CInfBone
{
public:
	CInfBone();
	~CInfBone();

	int InitParams();

	int ExistBone( CMQOObject* srcobj, int srcboneno );
	int AddInfElem( CMQOObject* srcobj, INFELEM srcie );
	int NormalizeInf( CMQOObject* srcobj );

private:
	int DestroyObjs();
	int InitElem( INFDATA* infptr );

public:
	std::map<CMQOObject*, INFDATA*> m_infdata;
};

#endif

