#ifndef MOTIONPOINTH
#define MOTIONPOINTH

#include <coef.h>
#include <quaternion.h>

class CBone;

class CMotionPoint
{
public:
	CMotionPoint();
	~CMotionPoint();

	int InitParams();

	int GetEul( D3DXVECTOR3* dsteul );
	int SetEul( CQuaternion* axisq, D3DXVECTOR3 srceul );
	int SetQ( CQuaternion* axisq, CQuaternion newq );

	int UpdateMatrix( D3DXMATRIX* wmat, D3DXMATRIX* parmat, CQuaternion* parq, CBone* srcbone );

	int AddToPrev( CMotionPoint* addmp );
	int AddToNext( CMotionPoint* addmp );
	int LeaveFromChain( int srcmotid = -1, CBone* boneptr = 0 );

	int MakeDispMat();
	int CopyMP( CMotionPoint* srcmp );

	int CalcQandTra( D3DXMATRIX srcmat, CBone* boneptr );

private:
	int DestroyObjs();

	int MakeMat( CBone* srcbone );
	int MakeTotalMat( D3DXMATRIX* parmat, CQuaternion* parq, CBone* srcbone );
	int MakeWorldMat( D3DXMATRIX* wmat );

public:
	int m_setbtflag;
	double m_frame;
	D3DXVECTOR3 m_eul;
	D3DXVECTOR3 m_tra;

	CQuaternion m_q;
	CQuaternion m_totalq;//親の影響を受けている回転情報
	CQuaternion m_worldq;//ワールドと親の影響を受けている回転情報

	D3DXMATRIX m_mat;//親の影響を受けていないマトリックス
	D3DXMATRIX m_totalmat;//親の影響を受けているマトリックス
	D3DXMATRIX m_worldmat;//ワールド変換と親の影響を受けたマトリックス
	D3DXMATRIX m_btmat;

	CQuaternion m_dispq;
	CQuaternion m_orderdispq;
	D3DXVECTOR3 m_disptra;
	D3DXMATRIX m_dispmat;

	D3DXMATRIX m_befworldmat;


	CMotionPoint* m_prev;
	CMotionPoint* m_next;

	D3DXMATRIX m_copywmat;

	D3DXMATRIX m_befeditmat;
	D3DXMATRIX m_absmat;
};


#endif