#ifndef POLYMESH3H
#define POLYMESH3H

#include <coef.h>
#include <D3DX9.h>

#include <crtdbg.h>
//#include <infelem.h>

#include <map>
using namespace std;

class CMQOObject;
class CMQOMaterial;
class CMQOFace;
class CInfBone;
class CInfScope;
class CBone;

class CPolyMesh3
{
public:
	CPolyMesh3();
	~CPolyMesh3();

	int CreatePM3( int pointnum, int facenum, float facet, D3DXVECTOR3* pointptr, CMQOFace* faceptr, 
		map<int,CMQOMaterial*>& srcmat, D3DXMATRIX multmat );
	
	int SetIndexBuf();	
	int InvIndexBuf();

	int ChkAlphaNum( map<int,CMQOMaterial*>& srcmat );
	int CalcBound();

	int CalcInfNoSkin( CBone* applybone );
	int CalcInf( CMQOObject* thisobj, map<int, CBone*>& srcbone );

	int MultScale( D3DXVECTOR3 srcscale, D3DXVECTOR3 srctra );

private:
	void InitParams();
	void DestroyObjs();

	int MultVert( D3DXMATRIX multmat );
	int CreateN3PFromMQOFace( N3P* n3pptr, int* numptr );

	int CalcOrgNormal();
	int CalcNormal( D3DXVECTOR3* newn, D3DXVECTOR3* curp, D3DXVECTOR3* aftp1, D3DXVECTOR3* aftp2 );
	int Vec3Cross( D3DXVECTOR3* pOut, D3DXVECTOR3* pV1, D3DXVECTOR3* pV2 );
	int Vec3Normalize( D3DXVECTOR3* retvec, D3DXVECTOR3* srcvec );

	int SetSMFace();
	int AddSmFace( N3P* n3p1, N3P* n3p2 );
	int CalcSMNormal();

	int SetOptV( PM3DISPV* optv, int* pleng, int* matnum, map<int,CMQOMaterial*>& srcmat );


public:
/***
typedef struct tag_verface
{
	int			faceno;
	int			orgfaceno;
	int			materialno;
	D3DXVECTOR3	facenormal;
}PERFACE;

typedef struct tag_pervert
{
	int				indexno;//3角の順番
	int				vno;
	int				uvnum;
	D3DXVECTOR2		uv[2];
	int				vcolflag;
	DWORD			vcol;
	D3DXVECTOR3 smnormal;

	int				createflag;
		//頂点を作成しない場合０
		//UV, VCOL, Materialnoの違いにより作成する場合は１を足す
		//normalにより作成する場合は２を足す
}PERVERT;

typedef struct tag_n3sm
{
	int smfacenum;
	void** ppsmface;//N3Pへのポインタの配列
}N3SM;

typedef struct tag_n3p
{
	PERFACE*	perface;
	PERVERT*	pervert;
	N3SM*		n3sm;
}N3P;//n*3


n3pをvno, materialno, uv, vcolでソートしたn3psortを作る。
n3psortを頭からチェックし、まったく同じものが無ければcreateflagに１を足す。

n3psortでvnoが等しいものの法線をチェックしスムージングするものが見つかったらppsmfaceに追加する。
全部終わったら
n3psortのvnoが等しいものの*ppsmfaceをチェックし、自分が始めて出現したときまたは出現しなかったときかつcreateflagに２が足されていないとき
createflagに２を足す。

n3smからsmnormalを計算する。

n3psortをmaterialno, faceno, indexnoでソートしたn3pafterを作る。

n3pafterのcreateflagを見て０以外のものからPM3OPTVを作る。

typedef struct tag_pm3optv
{
	int orgvno;
	int orgfaceno;
	D3DXVECTOR3 pos;
	int materialno;
	D3DXVECTOR3 normal;
	int				uvnum;
	D3DXVECTOR2		uv[2];
	int				vcolflag;
	DWORD		vcol;
}PM3OPTV;


***/

	int m_orgpointnum;
	int m_orgfacenum;
	int m_facenum;//三角分割後の面数

	float m_facet;
	CMQOFace* m_mqoface;//外部メモリ
	D3DXVECTOR3* m_pointbuf;//外部メモリ
	N3P* m_n3p;
	CInfBone* m_infbone;//orgpointnum長

///////
	int m_optleng;
	int m_optmatnum;
//	PM3OPTV* m_optv;
	PM3DISPV* m_dispv;
	int*	m_dispindex;
	int m_createoptflag;

	MATERIALBLOCK* m_matblock;
	MODELBOUND	m_bound;

	CHKALPHA chkalpha;
};



#endif