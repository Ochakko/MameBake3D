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
#include <VecMath.h>

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
			nextmp = mpptr->m_next;
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
	if( !pmodel->m_curmotinfo ){
		return 0;
	}

	if( (pmodel->m_curmotinfo->motid < 0) || pmodel->m_bonelist.empty() ){
		return 0;
	}

	ClearData();

	int curmotid = pmodel->m_curmotinfo->motid;

	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->m_bonelist.begin(); itrbone != pmodel->m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );
		CMotionPoint* firstmp = curbone->m_motionkey[ curmotid ];
		if( firstmp ){
			CMotionPoint* undofirstmp = new CMotionPoint();
			if( !undofirstmp ){
				_ASSERT( 0 );
				return 1;
			}

			undofirstmp->CopyMP( firstmp );

			CMotionPoint* curmp = firstmp->m_next;
			CMotionPoint* befundomp = undofirstmp;
			while( curmp ){
				CMotionPoint* newundomp = new CMotionPoint();
				if( !newundomp ){
					_ASSERT( 0 );
					return 1;
				}
				newundomp->CopyMP( curmp );
				befundomp->AddToNext( newundomp );

				befundomp = newundomp;
				curmp = curmp->m_next;
			}
			m_bone2mp[ curbone ] = undofirstmp;
		}

		map<int, map<double, int>>::iterator itrcur;
		itrcur = curbone->m_motmark.find( curmotid );
		if( itrcur != curbone->m_motmark.end() ){
			map<double, int> curmark;
			curmark = itrcur->second;
			(m_bonemotmark[ curbone ]).clear();
			m_bonemotmark[ curbone ] = curmark;
		}else{
			(m_bonemotmark[ curbone ]).clear();
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
	MoveMemory( &m_savemotinfo, pmodel->m_curmotinfo, sizeof( MOTINFO ) );

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
	MOTINFO* chkmotinfo = pmodel->m_motinfo[ setmotid ];
	if( !chkmotinfo ){
		_ASSERT( 0 );
		return 1;
	}
/////// destroy
	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->m_bonelist.begin(); itrbone != pmodel->m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );
		curbone->DestroyMotionKey( setmotid );

		(curbone->m_motmark[ setmotid ]).clear();
	}
/***
	map<int, CMQOObject*>::iterator itrbase;
	for( itrbase = pmodel->m_mbaseobject.begin(); itrbase != pmodel->m_mbaseobject.end(); itrbase++ ){
		CMQOObject* curbase = itrbase->second;
		_ASSERT( curbase );
		curbase->DestroyMorphKey( setmotid );
	}
***/
///////// set
	for( itrbone = pmodel->m_bonelist.begin(); itrbone != pmodel->m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );

		CMotionPoint* firstmp = m_bone2mp[ curbone ];
		if( firstmp ){
			CMotionPoint* undofirstmp = new CMotionPoint();
			if( !undofirstmp ){
				_ASSERT( 0 );
				return 1;
			}
			undofirstmp->CopyMP( firstmp );

			CMotionPoint* mpptr = firstmp->m_next;
			CMotionPoint* befundomp = undofirstmp;
			while( mpptr ){
				CMotionPoint* newundomp = new CMotionPoint();
				if( !newundomp ){
					_ASSERT( 0 );
					return 1;
				}
				newundomp->CopyMP( mpptr );
				befundomp->AddToNext( newundomp );

				befundomp = newundomp;
				mpptr = mpptr->m_next;
			}

			curbone->m_motionkey[ setmotid ] = undofirstmp;
		}

		curbone->m_motmark[ setmotid ] = m_bonemotmark[ curbone ];

	}

/***
	for( itrbase = pmodel->m_mbaseobject.begin(); itrbase != pmodel->m_mbaseobject.end(); itrbase++ ){
		CMQOObject* curbase = itrbase->second;
		_ASSERT( curbase );

		CMorphKey* firstmk = m_base2mk[ curbase ];
		if( firstmk ){
			CMorphKey* undofirstmk = new CMorphKey( curbase );
			if( !undofirstmk ){
				_ASSERT( 0 );
				return 1;
			}
			undofirstmk->CopyMotion( firstmk );

			CMorphKey* mkptr = firstmk->m_next;
			CMorphKey* befundomk = undofirstmk;
			while( mkptr ){
				CMorphKey* newundomk = new CMorphKey( curbase );
				if( !newundomk ){
					_ASSERT( 0 );
					return 1;
				}
				newundomk->CopyMotion( mkptr );
				befundomk->AddToNext( newundomk );

				befundomk = newundomk;
				mkptr = mkptr->m_next;
			}

			curbase->m_morphkey[ setmotid ] = undofirstmk;
		}
	}
***/
	MoveMemory( chkmotinfo, &m_savemotinfo, sizeof( MOTINFO ) );
	pmodel->m_curmotinfo = chkmotinfo;

	*curboneno = m_curboneno;
	*curbaseno = m_curbaseno;

	return 0;
}

