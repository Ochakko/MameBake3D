#ifndef POLYMESH4H
#define POLYMESH4H

#include <coef.h>
#include <D3DX9.h>

#include <crtdbg.h>

#include <map>
using namespace std;

class CMQOObject;
class CMQOMaterial;
class CMQOFace;
class CInfBone;
//class CInfScope;
class CBone;

class CPolyMesh4
{
public:
	CPolyMesh4();
	~CPolyMesh4();

	int CreatePM4( int pointnum, int facenum, int normalleng, int uvleng, D3DXVECTOR3* pointptr, D3DXVECTOR3* nptr, D3DXVECTOR2* uvptr, CMQOFace* faceptr, map<int,CMQOMaterial*>& srcmat );
	
	int ChkAlphaNum( map<int,CMQOMaterial*>& srcmat );
	int CalcBound();

	int SetPm3Inf( CMQOObject* srcobj );
	int UpdateMorphBuffer( D3DXVECTOR3* mpoint );

	int DumpInfBone( CMQOObject* srcobj, map<int,CBone*>& srcbonelist );
	int SetPm3InfNoSkin( LPDIRECT3DDEVICE9 pdev, CMQOObject* srcobj, int clusterno, map<int,CBone*>& srcbonelist );

private:
	void InitParams();
	void DestroyObjs();

	int SetTriFace( CMQOFace* faceptr, int* numptr );
	int SetOptV( PM3DISPV* optv, int* pleng, int* matnum, map<int,CMQOMaterial*>& srcmat );

public:
	//accesser
	int GetOrgPointNum(){
		return m_orgpointnum;
	};

	int GetOrgFaceNum(){
		return m_orgfacenum;
	};

	int GetFaceNum(){
		return m_facenum;
	};

	float GetFacet(){
		return m_facet;
	};

	CInfBone* GetInfBone(){
		return m_infbone;
	};

	PM3DISPV* GetPm3Disp(){
		return m_dispv;
	};

	PM3INF* GetPm3Inf(){
		return m_pm3inf;
	};

	int* GetDispIndex(){
		return m_dispindex;
	};

	int* GetOrgIndex(){
		return m_orgindex;
	};

	int GetCreateOptFlag(){
		return m_createoptflag;
	};

	MODELBOUND GetBound(){
		return m_bound;
	};

	int GetOptLeng(){
		return m_optleng;
	};

private:
	int m_orgpointnum;
	int m_orgfacenum;
	int m_facenum;//三角分割後の面数

	float m_facet;
	int m_normalleng;
	int m_uvleng;

	CInfBone* m_infbone;//orgpointnum長

	int m_optleng;
	PM3DISPV* m_dispv;
	PM3INF* m_pm3inf;
	int*	m_dispindex;
	int*	m_orgindex;
	int m_createoptflag;

	MODELBOUND	m_bound;


//以下、クラス外からアクセスしないのでアクセッサー無し。
	CMQOFace* m_mqoface;//外部メモリ
	D3DXVECTOR3* m_pointbuf;//外部メモリ
	D3DXVECTOR3* m_normal;//外部メモリ
	D3DXVECTOR2* m_uvbuf;//外部メモリ
	CMQOFace* m_triface;

	CHKALPHA chkalpha;
};



#endif