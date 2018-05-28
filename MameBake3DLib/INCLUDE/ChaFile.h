#ifndef CHAFILEH
#define CHAFILEH

//#include <d3dx9.h>
#include <ChaVecCalc.h>
#include <coef.h>
#include <XMLIO.h>

#include <vector>
#include <string>
#include <map>

class CModel;
class BPWorld;

class CChaFile : public CXMLIO
{
public:
	CChaFile();
	virtual ~CChaFile();

	int WriteChaFile( BPWorld* srcbpw, WCHAR* projdir, WCHAR* projname, std::vector<MODELELEM>& srcmodelindex, float srcmotspeed );
	int LoadChaFile( WCHAR* strpath, CModel* (*srcfbxfunc)( int skipdefref, int inittimelineflag ), int (*srcReffunc)(), int (*srcImpFunc)(), int (*srcGcoFunc)(), 
		int (*srcReMenu)( int selindex1, int callbymenu1 ), int (*srcRgdMenu)( int selindex2, int callbymenu2 ), int (*srcMorphMenu)( int selindex3 ), int (*srcImpMenu)( int selindex4 ) );

private:
	virtual int InitParams();
	virtual int DestroyObjs();

	int WriteFileInfo();
	int WriteChara( MODELELEM* srcme, WCHAR* projname );

	//int CheckFileVersion( XMLIOBUF* xmliobuf );
	int ReadProjectInfo( XMLIOBUF* xmliobuf, int* charanumptr );
	int ReadChara( XMLIOBUF* xmliobuf );
	//int ReadMotion( XMLIOBUF* xmliobuf, WCHAR* modelfolder, CModel* modelptr );
	int ReadWall(XMLIOBUF* xmliobuf);


private:
	std::vector<MODELELEM> m_modelindex;
	WCHAR m_newdirname[MAX_PATH];

	std::map<int, MOTINFO*> m_motinfo;
	WCHAR m_wloaddir[MAX_PATH];
	char m_mloaddir[MAX_PATH];


	CModel* (*m_FbxFunc)( int skipdefref, int inittimelineflag );
	int (*m_RefFunc)();
	int (*m_ImpFunc)();
	int (*m_GcoFunc)();
	int (*m_ReMenu)( int selindex, int callbymenu );
	int (*m_RgdMenu)( int selindex, int callbymenu );
	int (*m_MorphMenu)( int selindex );
	int (*m_ImpMenu)( int selindex );

	float m_motspeed;
};

#endif