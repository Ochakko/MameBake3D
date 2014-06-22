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
	int				indexno;//3�p�̏���
	int				vno;
	int				uvnum;
	D3DXVECTOR2		uv[2];
	int				vcolflag;
	DWORD			vcol;
	D3DXVECTOR3 smnormal;

	int				createflag;
		//���_���쐬���Ȃ��ꍇ�O
		//UV, VCOL, Materialno�̈Ⴂ�ɂ��쐬����ꍇ�͂P�𑫂�
		//normal�ɂ��쐬����ꍇ�͂Q�𑫂�
}PERVERT;

typedef struct tag_n3sm
{
	int smfacenum;
	void** ppsmface;//N3P�ւ̃|�C���^�̔z��
}N3SM;

typedef struct tag_n3p
{
	PERFACE*	perface;
	PERVERT*	pervert;
	N3SM*		n3sm;
}N3P;//n*3


n3p��vno, materialno, uv, vcol�Ń\�[�g����n3psort�����B
n3psort�𓪂���`�F�b�N���A�܂������������̂��������createflag�ɂP�𑫂��B

n3psort��vno�����������̖̂@�����`�F�b�N���X���[�W���O������̂�����������ppsmface�ɒǉ�����B
�S���I�������
n3psort��vno�����������̂�*ppsmface���`�F�b�N���A�������n�߂ďo�������Ƃ��܂��͏o�����Ȃ������Ƃ�����createflag�ɂQ��������Ă��Ȃ��Ƃ�
createflag�ɂQ�𑫂��B

n3sm����smnormal���v�Z����B

n3psort��materialno, faceno, indexno�Ń\�[�g����n3pafter�����B

n3pafter��createflag�����ĂO�ȊO�̂��̂���PM3OPTV�����B

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
	int m_facenum;//�O�p������̖ʐ�

	float m_facet;
	CMQOFace* m_mqoface;//�O��������
	D3DXVECTOR3* m_pointbuf;//�O��������
	N3P* m_n3p;
	CInfBone* m_infbone;//orgpointnum��

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