#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <UndoMotion.h>

#include <Model.h>
#include <Bone.h>

#define DBGH
#include <dbg.h>

#include <MotionPoint.h>
//#include <quaternion.h>
//#include <VecMath.h>
#include <ChaVecCalc.h>
#include <EditRange.h>


#include <string>


using namespace std;

//#include <MorphKey.h>


extern LONG g_bvh2fbxbatchflag;
//extern LONG g_motioncachebatchflag;
extern LONG g_retargetbatchflag;



CUndoMotion::CUndoMotion()
{
	InitParams();
}
CUndoMotion::~CUndoMotion()
{
	DestroyObjs();
}
int CUndoMotion::InitParams()
{
	m_validflag = 0;
	ZeroMemory( &m_savemotinfo, sizeof( MOTINFO ) );
	m_savemotinfo.motid = -1;

	m_bone2mp.clear();
	m_base2mk.clear();

	m_curboneno = -1;
	m_curbaseno = -1;

	m_bonemotmark.clear();

	m_keynum = 1;
	m_startframe = 1.0;
	m_endframe = 1.0;
	m_applyrate = 50.0;

	m_brushstate.Init();

	return 0;
}
int CUndoMotion::ClearData()
{
	DestroyObjs();
	InitParams();

	return 0;
}

int CUndoMotion::DestroyObjs()
{
	map<CBone*, CMotionPoint*>::iterator itrb2mp;
	for( itrb2mp = m_bone2mp.begin(); itrb2mp != m_bone2mp.end(); itrb2mp++ ){
		CMotionPoint* mpptr = itrb2mp->second;
		CMotionPoint* nextmp = 0;
		while( mpptr ){
			nextmp = mpptr->GetNext();
			//delete mpptr;
			CMotionPoint::InvalidateMotionPoint(mpptr);
			mpptr = nextmp;
		}
	}
	m_bone2mp.clear();

/***
	map<CMQOObject*, CMorphKey*>::iterator itrb2mk;
	for( itrb2mk = m_base2mk.begin(); itrb2mk != m_base2mk.end(); itrb2mk++ ){
		CMorphKey* mkptr = itrb2mk->second;
		CMorphKey* nextmk = 0;
		while( mkptr ){
			nextmk = mkptr->m_next;
			delete mkptr;
			mkptr = nextmk;
		}
	}
	m_base2mk.clear();
***/

	m_bonemotmark.clear();

	return 0;
}


int CUndoMotion::SaveUndoMotion( CModel* pmodel, int curboneno, int curbaseno, CEditRange* srcer, double srcapplyrate, BRUSHSTATE srcbrushstate)
{
	if (!pmodel) {
		return 2;
	}

	if( !pmodel->GetCurMotInfo() ){
		return 2;
	}

	if( pmodel->GetCurMotInfo()->motid < 0 ){
		return 2;
	}
	if( pmodel->GetBoneListSize()<= 0 ){
		return 2;
	}

	if (!srcer) {
		return 2;
	}


	m_brushstate = srcbrushstate;


	//if (g_bvh2fbxbatchflag || g_motioncachebatchflag || g_retargetbatchflag) {
	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) && (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) && (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) && (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		return 2;
	}


	//ClearData();

	int curmotid = pmodel->GetCurMotInfo()->motid;

	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->GetBoneListBegin(); itrbone != pmodel->GetBoneListEnd(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );
		if (curbone){

			//###################
			// first MotionPoint
			//###################

			CMotionPoint* firstsrcmp = curbone->GetMotionPoint(curmotid, srcer->GetStartFrame());
			//CMotionPoint* firstsrcmp = curbone->GetMotionPoint(curmotid, 0.0);
			if (!firstsrcmp) {
				//_ASSERT(0);
				return 2;
			}
			CMotionPoint* firstundomp = 0;
			map<CBone*, CMotionPoint*>::iterator itrbone2mp;
			itrbone2mp = m_bone2mp.find(curbone);
			if (itrbone2mp != m_bone2mp.end()) {
				firstundomp = itrbone2mp->second;
			}
			if (!firstundomp) {
				firstundomp = CMotionPoint::GetNewMP();
				if (!firstundomp) {
					_ASSERT(0);
					SetValidFlag(0);
					return 1;
				}
				m_bone2mp[curbone] = firstundomp;
			}
			firstundomp->CopyMP(firstsrcmp);
			firstundomp->SetUndoValidFlag(1);

			//######################
			// followed MotionPoint
			//######################

			CMotionPoint* befundomp = firstundomp;

			double currenttime;
			CMotionPoint* srcmp = firstsrcmp;
			CMotionPoint* undomp = firstundomp;
			for (currenttime = (srcer->GetStartFrame() + 1.0); currenttime <= srcer->GetEndFrame(); currenttime += 1.0) {
			//for (currenttime = 1.0; currenttime < (pmodel->GetCurMotInfo()->frameleng - 1.0); currenttime += 1.0) {
				srcmp = curbone->GetMotionPoint(curmotid, currenttime);
				undomp = 0;
				if (srcmp && befundomp) {
					undomp = befundomp->GetNext();
					if (!undomp) {
						undomp = CMotionPoint::GetNewMP();
						if (!undomp) {
							_ASSERT(0);
							SetValidFlag(0);
							return 1;
						}
						befundomp->AddToNext(undomp);
					}
					undomp->CopyMP(srcmp);
					undomp->SetUndoValidFlag(1);
				}
				else {
					_ASSERT(0);
					SetValidFlag(0);
					return 1;
				}

				befundomp = undomp;
			}

			undomp = undomp->GetNext();
			while (undomp){
				undomp->SetUndoValidFlag(0);
				undomp = undomp->GetNext();
			}



			map<double, int> tmpmap;
			curbone->GetMotMarkOfMap2(curmotid, tmpmap);
			if ((int)tmpmap.size() > 0){
				(m_bonemotmark[curbone]).clear();
				m_bonemotmark[curbone] = tmpmap;
			}
			else{
				(m_bonemotmark[curbone]).clear();
			}
		}
	}

/***
	map<int, CMQOObject*>::iterator itrbase;
	for( itrbase = pmodel->m_mbaseobject.begin(); itrbase != pmodel->m_mbaseobject.end(); itrbase++ ){
		CMQOObject* curbase = itrbase->second;
		_ASSERT( curbase );
		CMorphKey* firstmk = curbase->m_morphkey[ curmotid ];
		if( firstmk ){
			CMorphKey* undofirstmk = new CMorphKey( curbase );
			if( !undofirstmk ){
				_ASSERT( 0 );
				return 1;
			}

			undofirstmk->CopyMotion( firstmk );

			CMorphKey* curmk = firstmk->m_next;
			CMorphKey* befundomk = undofirstmk;
			while( curmk ){
				CMorphKey* newundomk = new CMorphKey( curbase );
				if( !newundomk ){
					_ASSERT( 0 );
					return 1;
				}
				newundomk->CopyMotion( curmk );

				befundomk->AddToNext( newundomk );

				befundomk = newundomk;
				curmk = curmk->m_next;
			}
			m_base2mk[ curbase ] = undofirstmk;
		}
	}
***/
	::MoveMemory( &m_savemotinfo, pmodel->GetCurMotInfo(), sizeof( MOTINFO ) );

	m_curboneno = curboneno;
	m_curbaseno = curbaseno;

	if (srcer) {
		double tmpapplyframe;
		srcer->GetRange(&m_keynum, &m_startframe, &m_endframe, &tmpapplyframe);
		m_applyrate = srcapplyrate;
	}
	else {
		m_keynum = 1;
		m_startframe = 1.0;
		m_endframe = 1.0;
		m_applyrate = 50.0;
	}

	m_validflag = 1;

	return 0;
}
int CUndoMotion::RollBackMotion( CModel* pmodel, int* curboneno, int* curbaseno, double* dststartframe, double* dstendframe, double* dstapplyrate, BRUSHSTATE* dstbrushstate)
{
	if( m_validflag != 1 ){
		_ASSERT( 0 );
		return 2;
	}
	if (!pmodel) {
		_ASSERT(0);
		return 2;
	}
	if (!curboneno) {
		_ASSERT(0);
		return 2;
	}
	if (!curbaseno) {
		_ASSERT(0);
		return 2;
	}
	if (!dststartframe)
	{
		_ASSERT(0);
		return 2;
	}
	if (!dstendframe) {
		_ASSERT(0);
		return 2;
	}
	if (!dstapplyrate) {
		_ASSERT(0);
		return 2;
	}
	if (!dstbrushstate) {
		_ASSERT(0);
		return 2;
	}

	*dstbrushstate = m_brushstate;

	int setmotid = m_savemotinfo.motid;
	MOTINFO* chkmotinfo = pmodel->GetMotInfo( setmotid );
	if( !chkmotinfo ){
		_ASSERT( 0 );
		SetValidFlag(0);//!!!!!!!!!!!!!!!
		return 1;
	}

	//::MoveMemory(chkmotinfo, &m_savemotinfo, sizeof(MOTINFO));
	//pmodel->SetCurMotInfo(chkmotinfo);
	//pmodel->SetCurrentMotion(setmotid);

	/*
/////// destroy
	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->GetBoneListBegin(); itrbone != pmodel->GetBoneListEnd(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );
		if (curbone){
			curbone->DestroyMotionKey(setmotid);
			curbone->ClearMotMarkOfMap2(setmotid);
		}
	}
	*/

///////// set
	map<int, CBone*>::iterator itrbone;
	for (itrbone = pmodel->GetBoneListBegin(); itrbone != pmodel->GetBoneListEnd(); itrbone++){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );

		if (curbone) {
			CMotionPoint* srcmp = m_bone2mp[curbone];
			while (srcmp && (srcmp->GetUndoValidFlag() == 1)) {
				double srcframe = srcmp->GetFrame();
				CMotionPoint* dstmp = curbone->GetMotionPoint(setmotid, srcframe);
				if (dstmp) {
					dstmp->CopyMP(srcmp);
				}

				srcmp = srcmp->GetNext();
			}
			curbone->SetMotMarkOfMap2(setmotid, m_bonemotmark[curbone]);
		}

		//if (curbone){
		//	CMotionPoint* srcmp = m_bone2mp[curbone];
		//	CMotionPoint* dstmp = curbone->GetMotionKey(setmotid);
		//	while (srcmp && (srcmp->GetUndoValidFlag() == 1) && dstmp){
		//		dstmp->CopyMP(srcmp);

		//		srcmp = srcmp->GetNext();
		//		dstmp = dstmp->GetNext();
		//	}

		//	//curbone->SetMotionKey(setmotid, undofirstmp);
		//	curbone->SetMotMarkOfMap2(setmotid, m_bonemotmark[curbone]);
		//}
	}


	MoveMemory( chkmotinfo, &m_savemotinfo, sizeof( MOTINFO ) );
	pmodel->SetCurMotInfo( chkmotinfo );


	*curboneno = m_curboneno;
	*curbaseno = m_curbaseno;


	*dststartframe = m_startframe;
	*dstendframe = m_endframe;
	*dstapplyrate = m_applyrate;

	return 0;
}

