#ifndef INFSCOPEH
#define INFSCOPEH

#include <coef.h>
#include <d3dx9.h>

class CMQOObject;
class CMQOFace;
class CMQOMaterial;
class CBone;

class CInfScope
{
public:
	CInfScope();
	~CInfScope();

	int CalcCenter();
	int CheckInCnt( D3DXVECTOR3* vptr, int* incntptr, int* justptr );

private:
	int InitParams();
	int DestroyObjs();

public:
	int m_serialno;
	int m_materialno;
	int m_validflag;

	int m_facenum;
	CMQOFace** m_ppface;

	int m_vnum;
	D3DXVECTOR3* m_pvert;

	int m_applyboneno;
	CMQOObject* m_targetobj;
	CBone* m_tmpappbone;

	D3DXVECTOR3 m_minv;
	D3DXVECTOR3 m_maxv;
	D3DXVECTOR3 m_center;

	int m_settminmax;
	D3DXVECTOR3 m_tminv;
	D3DXVECTOR3 m_tmaxv;
	D3DXVECTOR3 m_tcenter;
	float m_tmaxdist;

};

#endif