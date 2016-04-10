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

	int CopyMP( CMotionPoint* srcmp );

	int CalcQandTra( D3DXMATRIX srcmat, CBone* boneptr );
	int SetFirstFrameBaseMat(D3DXMATRIX srcfirstmat);



	CMotionPoint CMotionPoint::operator= (CMotionPoint mp);

public:
	//accesser
	int GetBtFlag(){ return m_setbtflag; };
	void SetBtFlag( int srcflag ){ m_setbtflag = srcflag; };

	double GetFrame(){ return m_frame; };
	void SetFrame( double srcframe ){ m_frame = srcframe; };

	D3DXVECTOR3 GetEul(){ return m_eul; };
	void SetEul( D3DXVECTOR3 srceul ){ m_eul = srceul; };

	D3DXVECTOR3 GetTra(){ return m_tra; };
	void SetTra( D3DXVECTOR3 srctra ){ m_tra = srctra; };

	D3DXVECTOR3 GetFirstFrameTra(){ return m_firstframetra; };

	CQuaternion GetQ(){ return m_q; };
	void SetQ( CQuaternion srcq ){ m_q = srcq; };

	D3DXMATRIX GetWorldMat(){ return m_worldmat; };
	void SetWorldMat( D3DXMATRIX srcmat ){ m_worldmat = srcmat; };
	D3DXMATRIX GetInvWorldMat(){ D3DXMATRIX invmat; D3DXMatrixInverse(&invmat, NULL, &m_worldmat); return invmat; };

	D3DXMATRIX GetBtMat(){ return m_btmat; };
	void SetBtMat( D3DXMATRIX srcmat ){ m_btmat = srcmat; };

	D3DXMATRIX GetBefWorldMat(){ return m_befworldmat; };
	void SetBefWorldMat( D3DXMATRIX srcmat ){ m_befworldmat = srcmat; };

	D3DXMATRIX GetBefEditMat(){ return m_befeditmat; };
	void SetBefEditMat( D3DXMATRIX srcmat ){ m_befeditmat = srcmat; };

	D3DXMATRIX GetAbsMat(){ return m_absmat; };
	void SetAbsMat( D3DXMATRIX srcmat ){ m_absmat = srcmat; };

	CMotionPoint* GetPrev(){ return m_prev; };
	void SetPrev( CMotionPoint* srcprev ){ m_prev = srcprev; };

	CMotionPoint* GetNext(){ return m_next; };
	void SetNext( CMotionPoint* srcnext ){ m_next = srcnext; };


private:
	int DestroyObjs();

private:
	int m_setbtflag;
	double m_frame;
	D3DXVECTOR3 m_eul;
	D3DXVECTOR3 m_tra;
	D3DXVECTOR3 m_firstframetra;

	CQuaternion m_q;

	D3DXMATRIX m_worldmat;//ワールド変換と親の影響を受けたマトリックス
	D3DXMATRIX m_btmat;

	D3DXMATRIX m_befworldmat;
	D3DXMATRIX m_befeditmat;

	D3DXMATRIX m_absmat;


	CMotionPoint* m_prev;
	CMotionPoint* m_next;

};


#endif