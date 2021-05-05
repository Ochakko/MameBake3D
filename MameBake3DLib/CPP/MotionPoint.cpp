#include "stdafx.h"
#include <windows.h>
#include <MotionPoint.h>
#include <Bone.h>

//#include <BoneProp.h>

#include <math.h>
#include <crtdbg.h>


static std::vector<CMotionPoint*> s_mppool;//allocate MPPOOLBLKLEN motoinpoints at onse and pool 


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
	//not use at allocated
	m_useflag = 0;//0: not use, 1: in use
	m_indexofpool = 0;
	m_allocheadflag = 0;//1: head pointer at allocated


	m_localmatflag = 0;
	m_undovalidflag = 0;
	m_frame = 0.0;
	m_q.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
	m_tra = ChaVector3( 0.0f, 0.0f, 0.0f );
	m_firstframetra = ChaVector3(0.0f, 0.0f, 0.0f);

	m_localeul = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaMatrixIdentity( &m_worldmat );
	ChaMatrixIdentity( &m_absmat );
	ChaMatrixIdentity( &m_befworldmat );
	ChaMatrixIdentity( &m_befeditmat );

	m_prev = 0;
	m_next = 0;

	return 0;
}
int CMotionPoint::DestroyObjs()
{

	return 0;
}

void CMotionPoint::SetLocalEul(ChaVector3 neweul)
{
	if (IsSameEul(neweul, m_localeul) == 0){
		m_localeul = neweul;
	}
};

/*
int CMotionPoint::GetEul( ChaVector3* dsteul )
{
	*dsteul = m_eul;

	return 0;
}
int CMotionPoint::SetEul( CQuaternion* axisq, ChaVector3 srceul )
{
	m_eul = srceul;
	m_q.SetRotation( axisq, srceul );

	return 0;
}
int CMotionPoint::SetQ( CQuaternion* axisq, CQuaternion newq )
{
	m_q = newq;

	ChaVector3 befeul;
	if( m_prev ){
		befeul = m_prev->m_eul;
	}else{
		befeul = ChaVector3( 0.0f, 0.0f, 0.0f );
	}
	
	m_q.Q2Eul( axisq, befeul, &m_eul );

	return 0;
}
*/

//int CMotionPoint::MakeMat( CBone* srcbone )
//{
//	ChaMatrix befrotmat, aftrotmat, rotmat, tramat;
//	ChaMatrixIdentity( &befrotmat );
//	ChaMatrixIdentity( &aftrotmat );
//	ChaMatrixIdentity( &rotmat );
//	ChaMatrixIdentity( &tramat );
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
//int CMotionPoint::MakeTotalMat( ChaMatrix* parmat, CQuaternion* parq, CBone* srcbone )
//{
//
//	MakeMat( srcbone );
//
//	m_totalmat = m_mat * *parmat;
//	m_totalq = *parq * m_q;
//
//	return 0;
//}

//int CMotionPoint::MakeWorldMat( ChaMatrix* wmat )
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

//int CMotionPoint::UpdateMatrix( ChaMatrix* wmat, ChaMatrix* parmat, CQuaternion* parq, CBone* srcbone )
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
	//m_eul = srcmp->m_eul;
	m_tra = srcmp->m_tra;
	m_q = srcmp->m_q;
	SetWorldMat(srcmp->m_worldmat);
	m_localeul = srcmp->m_localeul;
	m_absmat = srcmp->m_absmat;
	m_localmatflag = srcmp->m_localmatflag;

	return 0;
}

int CMotionPoint::CalcQandTra( ChaMatrix srcmat, CBone* boneptr, float hrate )
{
	if (boneptr){
		ChaVector3 aftpos;
		ChaVector3TransformCoord(&aftpos, &boneptr->GetJointFPos(), &srcmat);
		m_tra = aftpos - boneptr->GetJointFPos();


		ChaVector3 srcbonepos = boneptr->GetFirstFrameBonePos() * hrate;
		ChaVector3 aftpos2;
		ChaVector3TransformCoord(&aftpos2, &srcbonepos, &srcmat);
		m_firstframetra = aftpos - srcbonepos;
		//m_firstframetra = aftpos;
	}
	else{
		m_tra = ChaVector3(0.0f, 0.0f, 0.0f);
		m_firstframetra = ChaVector3(0.0f, 0.0f, 0.0f);
	}


	m_q.RotationMatrix(srcmat);

	return 0;
}


CMotionPoint CMotionPoint::operator= (CMotionPoint mp)
{
	m_localmatflag = mp.m_localmatflag;
	m_frame = mp.m_frame;
	//m_eul = mp.m_eul;
	m_tra = mp.m_tra;

	m_q = mp.m_q;
	m_localeul = mp.m_localeul;

	//SetWorldMat(mp.m_worldmat);
	m_worldmat = mp.m_worldmat;
	m_befworldmat = mp.m_befworldmat;//!!!!!!!!!!
	

	m_prev = mp.m_prev;
	m_next = mp.m_next;

	m_befeditmat = mp.m_befeditmat;
	m_absmat = mp.m_absmat;


	return *this;
}



//static func
CMotionPoint* CMotionPoint::GetNewMP()
{
	static int s_befheadno = -1;
	static int s_befelemno = -1;

	int curpoollen;
	curpoollen = s_mppool.size();


	if ((s_befheadno != (s_mppool.size() - 1)) || (s_befelemno != (MPPOOLBLKLEN - 1))) {//前回リリースしたポインタが最後尾ではない場合

		//前回リリースしたポインタの次のメンバーをチェックして未使用だったらリリース
		int chkheadno;
		chkheadno = s_befheadno;
		int chkelemno;
		chkelemno = s_befelemno + 1;
		if ((chkheadno >= 0) && (chkheadno >= curpoollen) && (chkelemno >= MPPOOLBLKLEN)) {
			chkelemno = 0;
			chkheadno++;
		}
		if ((chkheadno >= 0) && (chkheadno < curpoollen) && (chkelemno >= 0) && (chkelemno < MPPOOLBLKLEN)) {
			CMotionPoint* curmphead = s_mppool[chkheadno];
			if (curmphead) {
				CMotionPoint* chkmp;
				chkmp = curmphead + chkelemno;
				if (chkmp) {
					if (chkmp->GetUseFlag() == 0) {
						int saveindex = chkmp->GetIndexOfPool();
						int saveallochead = chkmp->IsAllocHead();
						chkmp->InitParams();
						chkmp->SetUseFlag(1);
						chkmp->SetIndexOfPool(saveindex);
						chkmp->SetIsAllocHead(saveallochead);

						s_befheadno = chkheadno;
						s_befelemno = chkelemno;
						return chkmp;
					}
				}
			}
		}

		//if ((chkheadno >= 0) && (chkheadno < curpoollen)) {
			//プールを先頭から検索して未使用がみつかればそれをリリース
		int mpno;
		for (mpno = 0; mpno < curpoollen; mpno++) {
			CMotionPoint* curmphead = s_mppool[mpno];
			if (curmphead) {
				int elemno;
				for (elemno = 0; elemno < MPPOOLBLKLEN; elemno++) {
					CMotionPoint* curmp;
					curmp = curmphead + elemno;
					if (curmp->GetUseFlag() == 0) {
						int saveindex = curmp->GetIndexOfPool();
						int saveallochead = curmp->IsAllocHead();
						curmp->InitParams();
						curmp->SetUseFlag(1);
						curmp->SetIndexOfPool(saveindex);
						curmp->SetIsAllocHead(saveallochead);

						s_befheadno = mpno;
						s_befelemno = elemno;
						return curmp;
					}
				}
			}
		}
		//}
	}

	//前回リリースしたポインタが最後尾または
	//未使用MPがpoolに無かった場合、アロケートしてアロケートした先頭のポインタをリリース
	CMotionPoint* allocmp;
	allocmp = new CMotionPoint[MPPOOLBLKLEN];
	if (!allocmp) {
		_ASSERT(0);

		s_befheadno = -1;
		s_befelemno = -1;

		return 0;
	}
	int allocno;
	for (allocno = 0; allocno < MPPOOLBLKLEN; allocno++) {
		CMotionPoint* curallocmp = allocmp + allocno;
		if (curallocmp) {
			int indexofpool = curpoollen + allocno;
			curallocmp->InitParams();
			curallocmp->SetUseFlag(0);
			curallocmp->SetIndexOfPool(indexofpool);

			if (allocno == 0) {
				curallocmp->SetIsAllocHead(1);
			}
			else {
				curallocmp->SetIsAllocHead(0);
			}
		}
		else {
			_ASSERT(0);

			s_befheadno = -1;
			s_befelemno = -1;

			return 0;
		}
	}
	s_mppool.push_back(allocmp);//allocate block(アロケート時の先頭ポインタ)を格納

	allocmp->SetUseFlag(1);


	s_befheadno = s_mppool.size() - 1;
	s_befelemno = 0;

	return allocmp;
}

//static func
void CMotionPoint::InvalidateMotionPoint(CMotionPoint* srcmp)
{
	if (!srcmp) {
		_ASSERT(0);
		return;
	}

	int saveindex = srcmp->GetIndexOfPool();
	int saveallochead = srcmp->IsAllocHead();

	srcmp->InitParams();
	srcmp->SetUseFlag(0);
	srcmp->SetIsAllocHead(saveallochead);
	srcmp->SetIndexOfPool(saveindex);
}

//static func
void CMotionPoint::InitMotionPoints()
{
	s_mppool.clear();
}

//static func
void CMotionPoint::DestroyMotionPoints() {
	int mpallocnum = s_mppool.size();
	int mpno;
	for (mpno = 0; mpno < mpallocnum; mpno++) {
		CMotionPoint* delmp;
		delmp = s_mppool[mpno];
		//if (delmp && (delmp->IsAllocHead() == 1)) {
		if (delmp) {
			delete[] delmp;
		}
	}
	s_mppool.clear();
}
