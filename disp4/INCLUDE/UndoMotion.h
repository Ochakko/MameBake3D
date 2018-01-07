#ifndef UNDOMOTIONH
#define UNDOMOTIONH

//#include <d3dx9.h>
#include <ChaVecCalc.h>
#include <wchar.h>
#include <Coef.h>
#include <string>
#include <map>


class CModel;
class CBone;
class CMotionPoint;
class CMQOObject;
class CMorphKey;

class CUndoMotion
{
public:
	CUndoMotion();
	~CUndoMotion();

	int ClearData();
	int SaveUndoMotion( CModel* pmodel, int curboneno, int curbaseno );
	int RollBackMotion( CModel* pmodel, int* curboneno, int* curbaseno );

private:
	int InitParams();
	int DestroyObjs();

public:
	//accesser
	MOTINFO GetSaveMotInfo(){
		return m_savemotinfo;
	};

	int GetValidFlag(){
		return m_validflag;
	};
	void SetValidFlag( int srcval ){
		m_validflag = srcval;
	};

private:
	int m_validflag;
	MOTINFO m_savemotinfo;
	std::map<CBone*, CMotionPoint*> m_bone2mp;
	std::map<CMQOObject*, CMorphKey*> m_base2mk;
	std::map<CBone*, std::map<double, int>> m_bonemotmark;

	int m_curboneno;
	int m_curbaseno;
};


#endif