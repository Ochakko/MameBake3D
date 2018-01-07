#ifndef POLYMESH4H
#define POLYMESH4H

#include <coef.h>
//#include <D3DX9.h>
#include <ChaVecCalc.h>

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

	int CreatePM4( int pointnum, int facenum, int normalleng, int uvleng, ChaVector3* pointptr, ChaVector3* nptr, D3DXVECTOR2* uvptr, CMQOFace* faceptr, map<int,CMQOMaterial*>& srcmat );
	
	int ChkAlphaNum( map<int,CMQOMaterial*>& srcmat );
	int CalcBound();

	int SetPm3Inf( CMQOObject* srcobj );
	int UpdateMorphBuffer( ChaVector3* mpoint );

	int DumpInfBone( CMQOObject* srcobj, map<int,CBone*>& srcbonelist );
	int SetPm3InfNoSkin( LPDIRECT3DDEVICE9 pdev, CMQOObject* srcobj, int clusterno, map<int,CBone*>& srcbonelist );

private:
	void InitParams();
	void DestroyObjs();

	int SetTriFace( CMQOFace* faceptr, int* numptr );
	int SetOptV( PM3DISPV* optv, int* pleng, int* matnum, map<int,CMQOMaterial*>& srcmat );
	int SetLastValidVno();

public:
	//accesser
	int GetOrgPointNum(){
		//return m_orgpointnum;
		return (m_lastvalidvno + 1);
	};

	int GetOrgFaceNum(){
		return m_orgfacenum;
	};

	ChaVector3* GetOrgPointBuf()
	{
		return m_pointbuf;
	};

	ChaVector3* GetOrgNormal()
	{
		return m_normal;
	};

	D3DXVECTOR2* GetOrgUV()
	{
		return m_uvbuf;
	};
	int GetOrgUVLeng(){
		return m_uvleng;
	};

	CMQOFace* GetTriFace(){
		return m_triface;
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

	ChaVector3 GetNormalByControlPointNo(int vno);
	D3DXVECTOR2 GetUVByControlPointNo(int vno);

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
	int*	m_fbxindex;
	int m_createoptflag;

	MODELBOUND	m_bound;

	int*	m_dirtyflag;
	int		m_lastvalidvno;

	CMQOFace* m_mqoface;//外部メモリ
	ChaVector3* m_pointbuf;//外部メモリ
	ChaVector3* m_normal;//外部メモリ
	D3DXVECTOR2* m_uvbuf;//外部メモリ
	CMQOFace* m_triface;

	CHKALPHA chkalpha;
};



#endif