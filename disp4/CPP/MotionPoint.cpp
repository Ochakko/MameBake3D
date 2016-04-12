#include <windows.h>
#include <MotionPoint.h>
#include <Bone.h>

#include <math.h>
#include <crtdbg.h>


CMotionPoint::CMotionPoint()
{
	InitParams();
}
CMotionPoint::~CMotionPoint()
{
	DestroyObjs();
}

int CMotionPoint::InitParams()
{
	m_setbtflag = 0;
	m_frame = 0.0;
	m_eul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_q.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
	m_tra = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_firstframetra = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	D3DXMatrixIdentity( &m_worldmat );
	D3DXMatrixIdentity( &m_btmat );
	D3DXMatrixIdentity( &m_absmat );
	D3DXMatrixIdentity( &m_befworldmat );
	D3DXMatrixIdentity( &m_befeditmat );

	m_prev = 0;
	m_next = 0;

	return 0;
}
int CMotionPoint::DestroyObjs()
{

	return 0;
}

int CMotionPoint::GetEul( D3DXVECTOR3* dsteul )
{
	*dsteul = m_eul;

	return 0;
}
int CMotionPoint::SetEul( CQuaternion* axisq, D3DXVECTOR3 srceul )
{
	m_eul = srceul;
	m_q.SetRotation( axisq, srceul );

	return 0;
}
int CMotionPoint::SetQ( CQuaternion* axisq, CQuaternion newq )
{
	m_q = newq;

	D3DXVECTOR3 befeul;
	if( m_prev ){
		befeul = m_prev->m_eul;
	}else{
		befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	}
	
	m_q.Q2Eul( axisq, befeul, &m_eul );

	return 0;
}


//int CMotionPoint::MakeMat( CBone* srcbone )
//{
//	D3DXMATRIX befrotmat, aftrotmat, rotmat, tramat;
//	D3DXMatrixIdentity( &befrotmat );
//	D3DXMatrixIdentity( &aftrotmat );
//	D3DXMatrixIdentity( &rotmat );
//	D3DXMatrixIdentity( &tramat );
//
//	befrotmat._41 = -srcbone->m_vertpos[ BT_PARENT ].x;
//	befrotmat._42 = -srcbone->m_vertpos[ BT_PARENT ].y;
//	befrotmat._43 = -srcbone->m_vertpos[ BT_PARENT ].z;
//
//	aftrotmat._41 = srcbone->m_vertpos[ BT_PARENT ].x;
//	aftrotmat._42 = srcbone->m_vertpos[ BT_PARENT ].y;
//	aftrotmat._43 = srcbone->m_vertpos[ BT_PARENT ].z;
//
//	rotmat = m_q.MakeRotMatX();
//
//	tramat._41 = m_tra.x;
//	tramat._42 = m_tra.y;
//	tramat._43 = m_tra.z;
//
//	m_mat = befrotmat * rotmat * aftrotmat * tramat;
//
//	return 0;
//}
//int CMotionPoint::MakeTotalMat( D3DXMATRIX* parmat, CQuaternion* parq, CBone* srcbone )
//{
//
//	MakeMat( srcbone );
//
//	m_totalmat = m_mat * *parmat;
//	m_totalq = *parq * m_q;
//
//	return 0;
//}

//int CMotionPoint::MakeWorldMat( D3DXMATRIX* wmat )
//{
//	m_worldmat = m_totalmat * *wmat;
//	
//
//	D3DXQUATERNION tmpxq;
//	D3DXQuaternionRotationMatrix( &tmpxq, wmat );
//	CQuaternion wq;
//	wq.SetParams( tmpxq );
//	m_worldq = wq * m_totalq;
//
//	return 0;
//}

//int CMotionPoint::UpdateMatrix( D3DXMATRIX* wmat, D3DXMATRIX* parmat, CQuaternion* parq, CBone* srcbone )
//{
//	MakeTotalMat( parmat, parq, srcbone );
//	MakeWorldMat( wmat );
//	return 0;
//}

int CMotionPoint::AddToPrev( CMotionPoint* addmp )
{
	CMotionPoint *saveprev, *savenext;
	saveprev = m_prev;
	savenext = m_next;

	addmp->m_prev = m_prev;
	addmp->m_next = this;

	m_prev = addmp;

	if( saveprev ){
		saveprev->m_next = addmp;
	}

	return 0;
}

int CMotionPoint::AddToNext( CMotionPoint* addmp )
{
	CMotionPoint *saveprev, *savenext;
	saveprev = m_prev;
	savenext = m_next;

	addmp->m_prev = this;
	addmp->m_next = savenext;

	m_next = addmp;

	if( savenext ){
		savenext->m_prev = addmp;
	}

	return 0;
}

int CMotionPoint::LeaveFromChain( int srcmotid, CBone* boneptr )
{
	CMotionPoint *saveprev, *savenext;
	saveprev = m_prev;
	savenext = m_next;

	m_prev = 0;
	m_next = 0;

	if( saveprev ){
		saveprev->m_next = savenext;
	}

	if( savenext ){
		savenext->m_prev = saveprev;
	}

	if( (srcmotid >= 0) && boneptr && !saveprev ){
		boneptr->SetMotionKey( srcmotid, savenext );
	}

	return 0;
}


int CMotionPoint::CopyMP( CMotionPoint* srcmp )
{
	m_frame = srcmp->m_frame;
	m_eul = srcmp->m_eul;
	m_tra = srcmp->m_tra;
	m_q = srcmp->m_q;
	m_worldmat = srcmp->m_worldmat;

	return 0;
}

int CMotionPoint::CalcQandTra( D3DXMATRIX srcmat, CBone* boneptr, float hrate )
{
	D3DXVECTOR3 aftpos;
	D3DXVec3TransformCoord( &aftpos, &boneptr->GetJointFPos(), &srcmat );
	m_tra = aftpos - boneptr->GetJointFPos();


	D3DXVECTOR3 srcbonepos = boneptr->GetFirstFrameBonePos() * hrate;
	D3DXVECTOR3 aftpos2;
	D3DXVec3TransformCoord(&aftpos2, &srcbonepos, &srcmat);
	m_firstframetra = aftpos - srcbonepos;
	//m_firstframetra = aftpos;


	D3DXMATRIX tmpmat = srcmat;
	tmpmat._41 = 0.0f;
	tmpmat._42 = 0.0f;
	tmpmat._43 = 0.0f;
	D3DXQUATERNION rotqx;
	D3DXQuaternionRotationMatrix( &rotqx, &tmpmat );
	m_q.x = rotqx.x;
	m_q.y = rotqx.y;
	m_q.z = rotqx.z;
	m_q.w = rotqx.w;

	return 0;
}


CMotionPoint CMotionPoint::operator= (CMotionPoint mp)
{
	m_setbtflag = mp.m_setbtflag;
	m_frame = mp.m_frame;
	m_eul = mp.m_eul;
	m_tra = mp.m_tra;

	m_q = mp.m_q;

	m_worldmat = mp.m_worldmat;//ワールド変換と親の影響を受けたマトリックス
	m_btmat = mp.m_btmat;

	m_prev = mp.m_prev;
	m_next = mp.m_next;

	m_befeditmat = mp.m_befeditmat;
	m_absmat = mp.m_absmat;


	return *this;
}



