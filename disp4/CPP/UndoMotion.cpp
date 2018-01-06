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
#include <quaternion.h>
//#include <VecMath.h>
#include <ChaVecCalc.h>

#include <string>


using namespace std;

//#include <MorphKey.h>


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
			delete mpptr;
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


int CUndoMotion::SaveUndoMotion( CModel* pmodel, int curboneno, int curbaseno )
{
	if( !pmodel->GetCurMotInfo() ){
		return 0;
	}

	if( pmodel->GetCurMotInfo()->motid < 0 ){
		return 0;
	}
	if( pmodel->GetBoneListSize()<= 0 ){
		return 0;
	}



	//ClearData();

	int curmotid = pmodel->GetCurMotInfo()->motid;

	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->GetBoneListBegin(); itrbone != pmodel->GetBoneListEnd(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );
		if (curbone){
			CMotionPoint* firstmp = curbone->GetMotionKey(curmotid);

			CMotionPoint* undofirstmp = 0;
			map<CBone*, CMotionPoint*>::iterator itrbone2mp;
			itrbone2mp = m_bone2mp.find(curbone);
			if (itrbone2mp != m_bone2mp.end()){
				undofirstmp = itrbone2mp->second;
			}

			if (firstmp){
				if (!undofirstmp){
					undofirstmp = new CMotionPoint();
					if (!undofirstmp){
						_ASSERT(0);
						return 1;
					}
					m_bone2mp[curbone] = undofirstmp;
				}

				undofirstmp->CopyMP(firstmp);
				undofirstmp->SetUndoValidFlag(1);

				CMotionPoint* curmp = firstmp->GetNext();
				CMotionPoint* befundomp = undofirstmp;
				CMotionPoint* newundomp = befundomp->GetNext();
				while (curmp){
					if (!newundomp){
						newundomp = new CMotionPoint();
						if (!newundomp){
							_ASSERT(0);
							return 1;
						}
						befundomp->AddToNext(newundomp);
					}

					newundomp->CopyMP(curmp);
					newundomp->SetUndoValidFlag(1);

					befundomp = newundomp;
					curmp = curmp->GetNext();
					newundomp = befundomp->GetNext();
				}

				while (newundomp){
					newundomp->SetUndoValidFlag(0);
					newundomp = newundomp->GetNext();
				}
			}
			else{
				if (undofirstmp){
					CMotionPoint* newundomp = undofirstmp;
					while (newundomp){
						newundomp->SetUndoValidFlag(0);
						newundomp = newundomp->GetNext();
					}
				}
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
	MoveMemory( &m_savemotinfo, pmodel->GetCurMotInfo(), sizeof( MOTINFO ) );

	m_curboneno = curboneno;
	m_curbaseno = curbaseno;

	m_validflag = 1;

	return 0;
}
int CUndoMotion::RollBackMotion( CModel* pmodel, int* curboneno, int* curbaseno )
{
	if( m_validflag != 1 ){
		_ASSERT( 0 );
		return 1;
	}

	int setmotid = m_savemotinfo.motid;
	MOTINFO* chkmotinfo = pmodel->GetMotInfo( setmotid );
	if( !chkmotinfo ){
		_ASSERT( 0 );
		return 1;
	}
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

		if (curbone){
			CMotionPoint* srcmp = m_bone2mp[curbone];
			CMotionPoint* dstmp = curbone->GetMotionKey(setmotid);
			while (srcmp && (srcmp->GetUndoValidFlag() == 1) && dstmp){
				dstmp->CopyMP(srcmp);

				srcmp = srcmp->GetNext();
				dstmp = dstmp->GetNext();
			}

			//curbone->SetMotionKey(setmotid, undofirstmp);
			curbone->SetMotMarkOfMap2(setmotid, m_bonemotmark[curbone]);
		}
	}


	MoveMemory( chkmotinfo, &m_savemotinfo, sizeof( MOTINFO ) );
	pmodel->SetCurMotInfo( chkmotinfo );

	*curboneno = m_curboneno;
	*curbaseno = m_curbaseno;

	return 0;
}

