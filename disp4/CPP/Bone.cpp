#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <Model.h>
#include <mqomaterial.h>
#include <mqoobject.h>

#define DBGH
#include <dbg.h>

#include <Bone.h>
#include <MQOFace.h>

#include <InfScope.h>
#include <MotionPoint.h>

#include <VecMath.h>

#include <RigidElem.h>
#include <EngName.h>
#include <BoneProp.h>

using namespace std;
using namespace OrgWinGUI;


map<CModel*,int> g_bonecntmap;
extern WCHAR g_basedir[MAX_PATH];
extern int g_boneaxis;
extern bool g_limitdegflag;
extern bool g_wmatDirectSetFlag;
extern bool g_underRetargetFlag;
extern int g_previewFlag;

CBone::CBone( CModel* parmodel ) : m_curmp(), m_axisq()
{
	InitParams();

	m_parmodel = parmodel;
	//_ASSERT(m_parmodel);

	map<CModel*,int>::iterator itrcnt;
	itrcnt = g_bonecntmap.find( m_parmodel );
	if( itrcnt == g_bonecntmap.end() ){
		g_bonecntmap[ m_parmodel ] = 0;
	}

	int curno = g_bonecntmap[ m_parmodel ]; 
	m_boneno = curno;
	g_bonecntmap[ m_parmodel ] = m_boneno + 1;

	m_firstcalcrigid = true;

}

CBone::~CBone()
{
	DestroyObjs();
}

int CBone::InitParams()
{
	m_excludemv = 0;
	m_mass0 = 0;
	m_posconstraint = 0;
	ZeroMemory(m_coldisp, sizeof(CModel*)* COL_MAX);

	D3DXMatrixIdentity(&m_tmpsymmat);

	D3DXMatrixIdentity(&m_btmat);
	D3DXMatrixIdentity(&m_befbtmat);
	m_setbtflag = 0;


	m_btparpos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_btchilpos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXMatrixIdentity(&m_btdiffmat);

	m_initcustomrigflag = 0;
	//InitCustomRig();//<-- after set name

	InitAngleLimit();
	m_upkind = UPVEC_NONE;
	m_motmark.clear();

	m_parmodel = 0;
	m_validflag = 1;
	m_rigidelem.clear();
	m_btobject.clear();
	m_btkinflag = 1;
	m_btforce = 0;

	m_globalpos.SetIdentity();
	D3DXMatrixIdentity( &m_invfirstmat );
	D3DXMatrixIdentity( &m_firstmat );
	D3DXMatrixIdentity( &m_startmat2 );
	D3DXMatrixIdentity( &m_axismat_par );
	D3DXMatrixIdentity(&m_initmat);
	D3DXMatrixIdentity(&m_invinitmat);
	D3DXMatrixIdentity(&m_tmpmat);
	D3DXMatrixIdentity(&m_firstaxismatX);
	D3DXMatrixIdentity(&m_firstaxismatZ);

	m_boneno = 0;
	m_topboneflag = 0;
	ZeroMemory( m_bonename, sizeof( char ) * 256 );
	ZeroMemory( m_wbonename, sizeof( WCHAR ) * 256 );
	ZeroMemory( m_engbonename, sizeof( char ) * 256 );

	m_childworld = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_childscreen = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	m_parent = 0;
	m_child = 0;
	m_brother = 0;

	m_selectflag = 0;

	m_getanimflag = 0;

	m_type = FBXBONE_NONE;

	D3DXMatrixIdentity( &m_nodemat );

	m_jointwpos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_jointfpos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_oldjointfpos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	D3DXMatrixIdentity( &m_laxismat );
	D3DXMatrixIdentity( &m_gaxismatXpar );

	m_remap.clear();
	m_impmap.clear();

	m_tmpq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);

	m_firstframebonepos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_posefoundflag = false;


	return 0;
}

int CBone::InitCustomRig()
{
	int rigno;
	for (rigno = 0; rigno < MAXRIGNUM; rigno++){
		::InitCustomRig(&(m_customrig[rigno]), this, rigno);
	}
	return 0;
}

void CBone::InitAngleLimit()
{
	::InitAngleLimit(&m_anglelimit);
}



int CBone::DestroyObjs()
{
	int colindex;
	for (colindex = 0; colindex < COL_MAX; colindex++){
		CModel* curcol = m_coldisp[colindex];
		if (curcol){
			delete curcol;
			m_coldisp[colindex] = 0;
		}
	}



	m_motmark.clear();

	map<int, CMotionPoint*>::iterator itrmp;
	for( itrmp = m_motionkey.begin(); itrmp != m_motionkey.end(); itrmp++ ){
		CMotionPoint* topkey = itrmp->second;
		if( topkey ){
			CMotionPoint* curmp = topkey;
			CMotionPoint* nextmp = 0;
			while( curmp ){
				nextmp = curmp->GetNext();

				delete curmp;

				curmp = nextmp;
			}
		}
	}

	m_motionkey.clear();

	
	map<string, std::map<CBone*, CRigidElem*>>::iterator itrmap;
	for( itrmap = m_remap.begin(); itrmap != m_remap.end(); itrmap++ ){
		map<CBone*, CRigidElem*> &curmap = itrmap->second;
		map<CBone*, CRigidElem*>::iterator itrre;
		for( itrre = curmap.begin(); itrre != curmap.end(); itrre++ ){
			CRigidElem* curre = itrre->second;
			delete curre;
		}
		curmap.clear();
	}
	m_remap.clear();
	m_impmap.clear();

	m_rigidelem.clear();

	return 0;
}


int CBone::AddChild( CBone* childptr )
{
	if( !m_child ){
		m_child = childptr;
		m_child->m_parent = this;
	}
	else{
		CBone* broptr = m_child;
		if (broptr){
			while (broptr->m_brother){
				broptr = broptr->m_brother;
			}
			broptr->m_brother = childptr;
			broptr->m_brother->m_parent = this;//!!!!!!!!
		}
	}

	return 0;
}


int CBone::UpdateMatrix( int srcmotid, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat )
{
	int existflag = 0;

	if (g_previewFlag != 5){


		if (srcframe >= 0.0){
			CallF(CalcFBXMotion(srcmotid, srcframe, &m_curmp, &existflag), return 1);
			D3DXMATRIX tmpmat = m_curmp.GetWorldMat();// **wmat;
			m_curmp.SetWorldMat(tmpmat);

			D3DXVECTOR3 jpos = GetJointFPos();
			D3DXVec3TransformCoord(&m_childworld, &jpos, &m_curmp.GetWorldMat());

			D3DXMATRIX wvpmat = m_curmp.GetWorldMat() * *vpmat;

			D3DXVec3TransformCoord(&m_childscreen, &m_childworld, vpmat);
			//D3DXVec3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
		}
		else{
			m_curmp.InitParams();
			m_curmp.SetWorldMat(*wmat);
		}
	}
	else{
		//RagdollIK時のボーン選択対策
		D3DXVECTOR3 jpos = GetJointFPos();

		D3DXMATRIX wmat, wvpmat;
		if (m_parent){
			wmat = m_parent->GetBtMat();
		}
		else{
			wmat = GetBtMat();
		}
		wvpmat = wmat * *vpmat;

		//D3DXVec3TransformCoord(&m_childworld, &jpos, &(GetBtMat()));
		D3DXVec3TransformCoord(&m_childscreen, &m_childworld, vpmat);

		D3DXVec3TransformCoord(&m_childworld, &jpos, &wmat);
		//D3DXVec3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);

	}
	return 0;
}


CMotionPoint* CBone::AddMotionPoint(int srcmotid, double srcframe, int* existptr)
{
	CMotionPoint* newmp = 0;
	CMotionPoint* pbef = 0;
	CMotionPoint* pnext = 0;
	CallF(GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, existptr), return 0);

	if (*existptr){
		pbef->SetFrame(srcframe);
		newmp = pbef;
	}
	else{
		newmp = new CMotionPoint();
		if (!newmp){
			_ASSERT(0);
			return 0;
		}
		newmp->SetFrame(srcframe);

		if (pbef){
			CallF(pbef->AddToNext(newmp), return 0);
		}
		else{
			m_motionkey[srcmotid] = newmp;
			if (pnext){
				newmp->SetNext(pnext);
			}
		}
	}

	return newmp;
}


int CBone::CalcFBXMotion( int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr )
{
	CMotionPoint* befptr = 0;
	CMotionPoint* nextptr = 0;
	CallF( GetBefNextMP( srcmotid, srcframe, &befptr, &nextptr, existptr ), return 1 );
	CallF( CalcFBXFrame( srcframe, befptr, nextptr, *existptr, dstmpptr ), return 1 );

	return 0;
}


int CBone::GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr )
{
	CMotionPoint* pbef = 0;
	CMotionPoint* pcur = m_motionkey[srcmotid];

	*existptr = 0;

	while( pcur ){

		if( (pcur->GetFrame() >= srcframe - 0.0001) && (pcur->GetFrame() <= srcframe + 0.0001) ){
			*existptr = 1;
			pbef = pcur;
			break;
		}else if( pcur->GetFrame() > srcframe ){
			*existptr = 0;
			break;
		}else{
			pbef = pcur;
			pcur = pcur->GetNext();
		}
	}
	*ppbef = pbef;

	if( *existptr ){
		*ppnext = pbef->GetNext();
	}else{
		*ppnext = pcur;
	}

	return 0;
}

int CBone::CalcFBXFrame( double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr )
{

	if( existflag == 1 ){
		*dstmpptr = *befptr;
		return 0;
	}else if( !befptr ){
		dstmpptr->InitParams();
		dstmpptr->SetFrame( srcframe );
		return 0;
	}else if( !nextptr ){
		*dstmpptr = *befptr;
		dstmpptr->SetFrame( srcframe );
		return 0;
	}else{
		double diffframe = nextptr->GetFrame() - befptr->GetFrame();
		_ASSERT( diffframe != 0.0 );
		double t = ( srcframe - befptr->GetFrame() ) / diffframe;

		D3DXMATRIX tmpmat = befptr->GetWorldMat() + (float)t * ( nextptr->GetWorldMat() - befptr->GetWorldMat() );
		dstmpptr->SetWorldMat( tmpmat );
		dstmpptr->SetFrame( srcframe );

		dstmpptr->SetPrev( befptr );
		dstmpptr->SetNext( nextptr );

		return 0;
	}
}

int CBone::DeleteMotion( int srcmotid )
{
	map<int, CMotionPoint*>::iterator itrmp;
	itrmp = m_motionkey.find( srcmotid );
	if( itrmp != m_motionkey.end() ){
		CMotionPoint* topkey = itrmp->second;
		if( topkey ){
			CMotionPoint* curmp = topkey;
			CMotionPoint* nextmp = 0;
			while( curmp ){
				nextmp = curmp->GetNext();

				delete curmp;

				curmp = nextmp;
			}
		}
	}

	m_motionkey.erase( itrmp );

	return 0;
}



int CBone::DeleteMPOutOfRange( int motid, double srcleng )
{
	CMotionPoint* curmp = m_motionkey[ motid ];

	while( curmp ){
		CMotionPoint* nextmp = curmp->GetNext();

		if( curmp->GetFrame() > srcleng ){
			curmp->LeaveFromChain( motid, this );
			delete curmp;
		}
		curmp = nextmp;
	}

	return 0;
}


int CBone::SetName( char* srcname )
{
	strcpy_s( m_bonename, 256, srcname );
	TermJointRepeats(m_bonename);

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_bonename, 256, m_wbonename, 256 );

	if (m_initcustomrigflag == 0){
		InitCustomRig();
		m_initcustomrigflag = 1;
	}

	return 0;
}

int CBone::CalcAxisMatZ( D3DXVECTOR3* curpos, D3DXVECTOR3* chilpos )
{
	if( *curpos == *chilpos ){
		D3DXMatrixIdentity( &m_laxismat );
		m_axisq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		return 0;
	}

	D3DXVECTOR3 startpos, endpos, upvec;

	D3DXVECTOR3 vecx0, vecy0, vecz0;
	D3DXVECTOR3 vecx1, vecy1, vecz1;

	startpos = *curpos;
	endpos = *chilpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	D3DXVECTOR3 bonevec;
	bonevec = endpos - startpos;
	D3DXVec3Normalize( &bonevec, &bonevec );

	if( (bonevec.x != 0.0f) || (bonevec.y != 0.0f) ){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
	}else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
	}

	vecz1 = bonevec;
		
	D3DXVec3Cross( &vecx1, &upvec, &vecz1 );
	D3DXVec3Normalize( &vecx1, &vecx1 );

	D3DXVec3Cross( &vecy1, &vecz1, &vecx1 );
	D3DXVec3Normalize( &vecy1, &vecy1 );


	D3DXMatrixIdentity( &m_laxismat );
	m_laxismat._11 = vecx1.x;
	m_laxismat._12 = vecx1.y;
	m_laxismat._13 = vecx1.z;

	m_laxismat._21 = vecy1.x;
	m_laxismat._22 = vecy1.y;
	m_laxismat._23 = vecy1.z;

	m_laxismat._31 = vecz1.x;
	m_laxismat._32 = vecz1.y;
	m_laxismat._33 = vecz1.z;

	m_axisq.RotationMatrix(m_laxismat);

	return 0;
}

int CBone::CalcAxisMatX()
{
	D3DXVECTOR3 curpos;
	D3DXVECTOR3 chilpos;

	if (m_parent){
		D3DXVec3TransformCoord(&curpos, &(m_parent->GetJointFPos()), &(m_parent->m_startmat2));
		D3DXVec3TransformCoord(&chilpos, &(GetJointFPos()), &m_startmat2);

		CalcAxisMatX_aft(curpos, chilpos, &m_gaxismatXpar);
	}

	return 0;
}


int CBone::CalcAxisMatX_aft(D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* dstmat)
{
	D3DXMATRIX retmat;
	D3DXMatrixIdentity(&retmat);
	if (curpos == chilpos){
		*dstmat = retmat;
		return 0;
	}

	D3DXVECTOR3 startpos, endpos, upvec;

	D3DXVECTOR3 vecx0, vecy0, vecz0;
	D3DXVECTOR3 vecx1, vecy1, vecz1;

	startpos = curpos;
	endpos = chilpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	D3DXVECTOR3 bonevec;
	bonevec = endpos - startpos;
	D3DXVec3Normalize(&bonevec, &bonevec);

	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.y) >= 0.000001f)){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
		m_upkind = UPVEC_Z;//vecx1-->X(bone), vecy1-->Y, vecz1-->Z
	}
	else{
		upvec.x = 0.0f;
		upvec.y = 1.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_Y;//vecx1-->X(bone), vecy1-->Z, vecz1-->Y
	}

	vecx1 = bonevec;

	D3DXVec3Cross(&vecy1, &upvec, &vecx1);
	D3DXVec3Normalize(&vecy1, &vecy1);

	D3DXVec3Cross(&vecz1, &vecx1, &vecy1);
	D3DXVec3Normalize(&vecz1, &vecz1);


	retmat._11 = vecx1.x;
	retmat._12 = vecx1.y;
	retmat._13 = vecx1.z;

	retmat._21 = vecy1.x;
	retmat._22 = vecy1.y;
	retmat._23 = vecy1.z;

	retmat._31 = vecz1.x;
	retmat._32 = vecz1.y;
	retmat._33 = vecz1.z;

	*dstmat = retmat;
	return 0;
}

int CBone::CalcAxisMatZ_aft(D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* dstmat)
{
	D3DXMATRIX retmat;
	D3DXMatrixIdentity(&retmat);
	if (curpos == chilpos){
		*dstmat = retmat;
		return 0;
	}

	D3DXVECTOR3 startpos, endpos, upvec;

	D3DXVECTOR3 vecx0, vecy0, vecz0;
	D3DXVECTOR3 vecx1, vecy1, vecz1;

	startpos = curpos;
	endpos = chilpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	D3DXVECTOR3 bonevec;
	bonevec = endpos - startpos;
	D3DXVec3Normalize(&bonevec, &bonevec);

	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.z) >= 0.000001f)){
		upvec.x = 0.0f;
		upvec.y = 1.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_Y;//vecx1-->X, vecy1-->Y, vecz1-->Z(bone)
	}
	else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_X;//vecx1-->Y, vecy1-->X, vecz1-->Z(bone)
	}

	vecz1 = bonevec;

	//D3DXVec3Cross(&vecx1, &vecz1, &upvec);
	D3DXVec3Cross(&vecx1, &upvec, &vecz1);
	D3DXVec3Normalize(&vecx1, &vecx1);

	D3DXVec3Cross(&vecy1, &vecz1, &vecx1);
	D3DXVec3Normalize(&vecy1, &vecy1);


	retmat._11 = vecx1.x;
	retmat._12 = vecx1.y;
	retmat._13 = vecx1.z;

	retmat._21 = vecy1.x;
	retmat._22 = vecy1.y;
	retmat._23 = vecy1.z;

	retmat._31 = vecz1.x;
	retmat._32 = vecz1.y;
	retmat._33 = vecz1.z;

	*dstmat = retmat;

	return 0;
}



int CBone::CalcAxisMatY( CBone* chilbone, D3DXMATRIX* dstmat )
{

	D3DXVECTOR3 curpos;
	D3DXVECTOR3 chilpos;

	D3DXVec3TransformCoord(&curpos, &(GetJointFPos()), &(m_curmp.GetWorldMat()));
	//D3DXVec3TransformCoord(&chilpos, &(chilbone->GetJointFPos()), &(chilbone->m_curmp.GetWorldMat()));
	D3DXVec3TransformCoord(&chilpos, &(chilbone->GetJointFPos()), &(m_curmp.GetWorldMat()));

	D3DXVECTOR3 diff = curpos - chilpos;
	float leng;
	leng = D3DXVec3Length( &diff );

	if( leng <= 0.00001f ){
		D3DXMatrixIdentity( dstmat );
		return 0;
	}

	D3DXVECTOR3 startpos, endpos, upvec;

	D3DXVECTOR3 vecx0, vecy0, vecz0;
	D3DXVECTOR3 vecx1, vecy1, vecz1;

	startpos = curpos;
	endpos = chilpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;//!!!!!!!!!!!!!!!!!!
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	D3DXVECTOR3 bonevec;
	bonevec = endpos - startpos;
	D3DXVec3Normalize( &bonevec, &bonevec );

	if( (bonevec.x != 0.0f) || (bonevec.y != 0.0f) ){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
	}else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
	}

	vecy1 = bonevec;
		
	D3DXVec3Cross( &vecx1, &vecy1, &upvec );

	int illeagalflag = 0;
	float crleng = D3DXVec3Length( &vecx1 );
	if( crleng < 0.000001f ){
		illeagalflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	D3DXVec3Normalize( &vecx1, &vecx1 );

	D3DXVec3Cross( &vecz1, &vecx1, &vecy1 );
	D3DXVec3Normalize( &vecy1, &vecy1 );

	D3DXQUATERNION tmpxq;

	D3DXMatrixIdentity( dstmat );
	if( illeagalflag == 0 ){
		dstmat->_11 = vecx1.x;
		dstmat->_12 = vecx1.y;
		dstmat->_13 = vecx1.z;

		dstmat->_21 = vecy1.x;
		dstmat->_22 = vecy1.y;
		dstmat->_23 = vecy1.z;

		dstmat->_31 = vecz1.x;
		dstmat->_32 = vecz1.y;
		dstmat->_33 = vecz1.z;
	}

	return 0;
}

int CBone::CalcRigidElemParams( CBone* chilbone, int setstartflag )
{


	CRigidElem* curre = m_rigidelem[ chilbone ];
	if( !curre ){
		_ASSERT( 0 );
		return 0;
	}

	//_ASSERT( colptr );
	_ASSERT( chilbone );

	CModel* curcoldisp = m_coldisp[curre->GetColtype()];
	_ASSERT( curcoldisp );


	D3DXMATRIX bmmat;
	D3DXMatrixIdentity( &bmmat );

	D3DXVECTOR3 aftbonepos;
	D3DXVECTOR3 aftchilpos;
	if (g_previewFlag != 5){
		D3DXVec3TransformCoord(&aftbonepos, &(GetJointFPos()), &(m_curmp.GetWorldMat()));
		D3DXVec3TransformCoord(&aftchilpos, &(chilbone->GetJointFPos()), &(m_curmp.GetWorldMat()));
	}
	else{
		if (setstartflag == 1){
			D3DXVec3TransformCoord(&aftbonepos, &(GetJointFPos()), &(m_curmp.GetWorldMat()));
			D3DXVec3TransformCoord(&aftchilpos, &(chilbone->GetJointFPos()), &(m_curmp.GetWorldMat()));
		}
		else{
			if (GetParent()){
				D3DXVec3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetBtMat()));
				D3DXVec3TransformCoord(&aftchilpos, &(chilbone->GetJointFPos()), &(GetBtMat()));
			}
			else{
				D3DXVec3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetBtMat()));
				D3DXVec3TransformCoord(&aftchilpos, &(chilbone->GetJointFPos()), &(chilbone->GetBtMat()));
			}
		}
	}
	CalcAxisMatZ( &aftbonepos, &aftchilpos );
	CalcAxisMatY( chilbone, &bmmat );			
	D3DXVECTOR3 diffvec = aftchilpos - aftbonepos;
	float diffleng = D3DXVec3Length( &diffvec );

	float cylileng = curre->GetCylileng();
	float sphr = curre->GetSphr();
	float boxz = curre->GetBoxz();

	if ((setstartflag == 1) || (m_firstcalcrigid == true)){
		if (curre->GetColtype() == COL_CAPSULE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				if (strcmp(curobj->GetName(), "cylinder") == 0){
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 0, &cylileng), return 1);
				}
				else if (strcmp(curobj->GetName(), "sph_ue") == 0){
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 1, &sphr), return 1);
				}
				else{
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 2, 0), return 1);
				}
			}
		}
		else if (curre->GetColtype() == COL_CONE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtCone(curre, diffleng, &cylileng, &sphr), return 1);
			}
		}
		else if (curre->GetColtype() == COL_SPHERE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtSphere(curre, diffleng, &cylileng, &sphr), return 1);
			}
		}
		else if (curre->GetColtype() == COL_BOX_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtBox(curre, diffleng, &cylileng, &sphr, &boxz), return 1);

				DbgOut(L"bonecpp : calcrigidelemparams : BOX : cylileng %f, sphr %f, boxz %f\r\n", cylileng, sphr, boxz);

			}
		}
		else{
			_ASSERT(0);
			return 1;
		}
	}

	//bmmat._41 = ( aftbonepos.x + aftchilpos.x ) * 0.5f;
	//bmmat._42 = ( aftbonepos.y + aftchilpos.y ) * 0.5f;
	//bmmat._43 = ( aftbonepos.z + aftchilpos.z ) * 0.5f;

	bmmat._41 = aftbonepos.x;
	bmmat._42 = aftbonepos.y;
	bmmat._43 = aftbonepos.z;


	curre->SetCapsulemat( bmmat );
	curre->SetCylileng( cylileng );
	curre->SetSphr( sphr );
	curre->SetBoxz( boxz );

	if( setstartflag == 1 ){
		curre->SetFirstcapsulemat( curre->GetCapsulemat() );
	}


	m_firstcalcrigid = false;

	return 0;
}

void CBone::SetStartMat2Req()
{
	SetStartMat2(m_curmp.GetWorldMat());

	if (m_child){
		m_child->SetStartMat2Req();
	}
	if (m_brother){
		m_brother->SetStartMat2Req();
	}
}

int CBone::CalcAxisMat( int firstflag, float delta )
{
	if( firstflag == 1 ){
		SetStartMat2Req();
		
		CalcAxisMatX();
	}

	if( delta != 0.0f ){
		CQuaternion multq;
		D3DXVECTOR3 gparpos, gchilpos, gbonevec;
		if (m_parent){
			D3DXVec3TransformCoord(&gparpos, &(m_parent->GetJointFPos()), &(m_parent->m_startmat2));
			D3DXVec3TransformCoord(&gchilpos, &(GetJointFPos()), &m_startmat2);
			gbonevec = gchilpos - gparpos;
			D3DXVec3Normalize(&gbonevec, &gbonevec);
		}
		else{
			_ASSERT(0);
			return 0;
		}


		multq.SetAxisAndRot( gbonevec, delta );
		D3DXMATRIX multmat = multq.MakeRotMatX();
		m_gaxismatXpar = m_gaxismatXpar * multmat;
	}

	D3DXMATRIX invpar;
	D3DXMatrixInverse( &invpar, NULL, &m_gaxismatXpar );

	CalcLocalAxisMat( m_startmat2, m_gaxismatXpar, m_gaxismatYpar );

	return 0;
}

int CBone::CalcLocalAxisMat( D3DXMATRIX motmat, D3DXMATRIX axismatpar, D3DXMATRIX gaxisy )
{
	D3DXMATRIX startpar0 = axismatpar;
	startpar0._41 = 0.0f;
	startpar0._42 = 0.0f;
	startpar0._43 = 0.0f;

	D3DXMATRIX starty = gaxisy;
	starty._41 = 0.0f;
	starty._42 = 0.0f;
	starty._43 = 0.0f;

	D3DXMATRIX motmat0 = motmat;
	motmat0._41 = 0.0f;
	motmat0._42 = 0.0f;
	motmat0._43 = 0.0f;

	D3DXMATRIX invmotmat;
	D3DXMatrixInverse( &invmotmat, NULL, &motmat0 );

	m_axismat_par = startpar0 * invmotmat;

	return 0;
}

int CBone::CreateRigidElem( CBone* parbone, int reflag, std::string rename, int impflag, std::string impname )
{
	if (!parbone){
		return 0;
	}

	if( reflag ){
		map<string, map<CBone*, CRigidElem*>>::iterator findremap;
		findremap = parbone->FindRigidElemOfMap( rename );
		if( findremap != parbone->GetRigidElemOfMapEnd() ){
			//map<CBone*, CRigidElem*> curmap = findremap->second;
	
	DbgOut( L"CreateRigidElem : map exist : parbone %s, curbone %s\r\n", parbone->m_wbonename, m_wbonename );

			CRigidElem* chkre = findremap->second[this];
			if( chkre ){
	DbgOut( L"CreateRigidElem : chkre return !!!\r\n" );
				return 0;
			}

			findremap->second[this] = new CRigidElem();
			if (!findremap->second[this]){
				_ASSERT( 0 );
				return 1;
			}

			findremap->second[this]->SetBone(parbone);
			findremap->second[this]->SetEndbone(this);

			SetGroupNoByName(findremap->second[this], this);

		}else{
			map<CBone*, CRigidElem*> curmap;

	DbgOut( L"CreateRigidElem : map [not] exist : curbone %s, chilbone %s\r\n", parbone->m_wbonename, m_wbonename );

			curmap[this] = new CRigidElem();
			if( !curmap[this] ){
				_ASSERT( 0 );
				return 1;
			}

			curmap[this]->SetBone(parbone);
			curmap[this]->SetEndbone(this);

			parbone->m_remap[rename] = curmap;

			SetGroupNoByName(curmap[this], this);

			//_ASSERT( 0 );
		}
	}

//////////////
	if( impflag ){
		map<string, map<CBone*, D3DXVECTOR3>>::iterator findimpmap;
		findimpmap = parbone->FindImpMap(impname);
		if (findimpmap != parbone->GetImpMapEnd()){
			map<CBone*, D3DXVECTOR3>::iterator itrimp;
			itrimp = findimpmap->second.find(this);
			if (itrimp != findimpmap->second.end()){
				return 0;
			}
			findimpmap->second[this] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		}else{
			map<CBone*, D3DXVECTOR3> curmap;

			curmap[this] = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			parbone->m_impmap[impname] = curmap;
		}
	}
	return 0;
}

int CBone::SetGroupNoByName( CRigidElem* curre, CBone* chilbone )
{
	char* groupmark = strstr( chilbone->m_bonename, "_G_" );
	if( groupmark ){
		char* numstart = groupmark + 3;
		char* numend = strstr( numstart, "_" );
		if( numend ){
			int numleng = (int)(numend - numstart);
			if( (numleng > 0) && (numleng <= 2) ){
				char strnum[5];
				ZeroMemory( strnum, sizeof( char ) * 5 );
				strncpy_s( strnum, 5, numstart, numleng );
				int gno = (int)atoi( strnum );
				if( (gno >= 1) && (gno <= COLIGROUPNUM) ){
					curre->SetGroupid( gno ); 
				}
			}
		}
	}
	int cmpbt = strncmp( chilbone->m_bonename, "BT_", 3 );
	if( cmpbt == 0 ){
		m_btforce = 1;
	}

	return 0;
}

int CBone::SetCurrentRigidElem( std::string curname )
{
	m_rigidelem.clear();

	if( !m_child ){
		return 0;
	}

	map<string, map<CBone*, CRigidElem*>>::iterator itrmap;
	itrmap = m_remap.find( curname );
	if( itrmap == m_remap.end() ){
		_ASSERT( 0 );
		return 1;
	}

	m_rigidelem = itrmap->second;

	return 0;
}


CMotionPoint* CBone::AddBoneTraReq( CMotionPoint* parmp, int srcmotid, double srcframe, D3DXVECTOR3 srctra )
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !curmp || !existflag ){
		_ASSERT( 0 );
		return 0;
	}


	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	if( parmp ){
		D3DXMATRIX invbefpar;
		D3DXMatrixInverse( &invbefpar, NULL, &parmp->GetBefWorldMat() );
		D3DXMATRIX tmpmat = curmp->GetWorldMat() * invbefpar * parmp->GetWorldMat();
		g_wmatDirectSetFlag = true;
		SetWorldMat( 0, srcmotid, srcframe, tmpmat );
		g_wmatDirectSetFlag = false;
	}
	else{
		D3DXMATRIX tramat;
		D3DXMatrixTranslation( &tramat, srctra.x, srctra.y, srctra.z );
		D3DXMATRIX tmpmat = curmp->GetWorldMat() * tramat;
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, tmpmat);
		g_wmatDirectSetFlag = false;
	}

	curmp->SetAbsMat( curmp->GetWorldMat() );

	if (m_child){
		m_child->AddBoneTraReq(curmp, srcmotid, srcframe, srctra);
	}
	if (m_brother && parmp){
		m_brother->AddBoneTraReq(parmp, srcmotid, srcframe, srctra);
	}

	return curmp;
}


CMotionPoint* CBone::PasteRotReq( int srcmotid, double srcframe, double dstframe )
{
	//src : srcmp srcparmp
	//dst : curmp parmp

	int existflag0 = 0;
	CMotionPoint* srcmp = AddMotionPoint( srcmotid, srcframe, &existflag0 );
	if( !existflag0 || !srcmp ){
		_ASSERT( 0 );
		return 0;
	}

	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, dstframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}
	
	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	curmp->SetWorldMat( srcmp->GetWorldMat() );
	curmp->SetAbsMat( srcmp->GetAbsMat() );


	//オイラー角初期化
	D3DXVECTOR3 cureul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	int paraxsiflag = 1;
	int isfirstbone = 0;
	cureul = CalcLocalEulZXY(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);
	SetLocalEul(srcmotid, srcframe, cureul);


	if( m_child ){
		m_child->PasteRotReq( srcmotid, srcframe, dstframe );
	}
	if( m_brother ){
		m_brother->PasteRotReq( srcmotid, srcframe, dstframe );
	}
	return curmp;

}


CMotionPoint* CBone::RotBoneQReq(CMotionPoint* parmp, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone, D3DXVECTOR3 traanim, int setmatflag, D3DXMATRIX* psetmat)
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}
	

	if (parmp){
		//再帰から呼び出し
		D3DXMATRIX befparmat = parmp->GetBefWorldMat();
		D3DXMATRIX newparmat = parmp->GetWorldMat();
		//if ((g_underRetargetFlag == true) || (IsSameMat(befparmat, newparmat) == 0)){
			D3DXMATRIX invbefpar;
			D3DXMatrixInverse(&invbefpar, NULL, &befparmat);
			D3DXMATRIX tmpmat = curmp->GetWorldMat() * invbefpar * newparmat;
			g_wmatDirectSetFlag = true;
			SetWorldMat(0, srcmotid, srcframe, tmpmat);
			g_wmatDirectSetFlag = false;
			//}
		//else{
			//parmatに変化がないときは変更しない。
		//	curmp->SetBefWorldMat( curmp->GetWorldMat() );
		//}
		if (bvhbone){
			if (m_child){
				bvhbone->SetTmpMat(curmp->GetWorldMat());
			}
			else{
				bvhbone->SetTmpMat(newparmat);
			}
		}
	}
	else{
		//初回呼び出し
		D3DXMATRIX tramat;
		D3DXMatrixIdentity(&tramat);
		D3DXMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);

		if (m_child){
			if (setmatflag == 0){
				D3DXVECTOR3 rotcenter;// = m_childworld;
				D3DXVec3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));

				D3DXMATRIX befrot, aftrot;
				D3DXMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
				D3DXMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
				D3DXMATRIX rotmat = befrot * rotq.MakeRotMatX() * aftrot;
				D3DXMATRIX tmpmat = curmp->GetWorldMat() * rotmat * tramat;
				//directflagまたはunderRetargetFlagがないときはtramat成分は無視され、SetWorldMatFromEul中でbone::CalcLocalTraAnimの値が適用される。
				SetWorldMat(0, srcmotid, srcframe, tmpmat);
				if (bvhbone){
					bvhbone->SetTmpMat(tmpmat);
				}
			}
			else{
				D3DXMATRIX tmpmat = *psetmat;
				g_wmatDirectSetFlag = true;
				SetWorldMat(0, srcmotid, srcframe, tmpmat);
				g_wmatDirectSetFlag = false;
				if (bvhbone){
					bvhbone->SetTmpMat(tmpmat);
				}
			}
		}
		else{
			D3DXVECTOR3 rotcenter;// = m_childworld;
			D3DXVec3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));

			D3DXMATRIX befrot, aftrot;
			D3DXMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
			D3DXMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
			D3DXMATRIX rotmat = befrot * rotq.MakeRotMatX() * aftrot;
			D3DXMATRIX tmpmat = curmp->GetWorldMat() * rotmat * tramat;
			g_wmatDirectSetFlag = true;//!!!!!!!!
			SetWorldMat(0, srcmotid, srcframe, tmpmat);
			g_wmatDirectSetFlag = false;//!!!!!!!
			if (bvhbone){
				bvhbone->SetTmpMat(tmpmat);
			}
		}
	}


	curmp->SetAbsMat(curmp->GetWorldMat());

	if (m_child && curmp){
		m_child->RotBoneQReq(curmp, srcmotid, srcframe, rotq);
	}
	if (m_brother && parmp){
		m_brother->RotBoneQReq(parmp, srcmotid, srcframe, rotq);
	}
	return curmp;
}

CMotionPoint* CBone::RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, D3DXMATRIX srcmat)
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint(srcmotid, srcframe, &existflag);
	if (!existflag || !curmp){
		_ASSERT(0);
		return 0;
	}

	if (parmp){
		//parentの行列をセット !!!!!!!!!
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, parmp->GetWorldMat());
		g_wmatDirectSetFlag = false;
	} else{
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, srcmat);
		g_wmatDirectSetFlag = false;
	}

	curmp->SetAbsMat(curmp->GetWorldMat());

	return curmp;
}


CMotionPoint* CBone::SetAbsMatReq( int broflag, int srcmotid, double srcframe, double firstframe )
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}

	int existflag2 = 0;
	CMotionPoint* firstmp = AddMotionPoint( srcmotid, firstframe, &existflag2 );
	if( !existflag2 || !firstmp ){
		_ASSERT( 0 );
		return 0;
	}

	g_wmatDirectSetFlag = true;
	SetWorldMat(0, srcmotid, srcframe, firstmp->GetAbsMat());
	g_wmatDirectSetFlag = false;

	if( m_child ){
		m_child->SetAbsMatReq( 1, srcmotid, srcframe, firstframe );
	}
	if( m_brother && broflag ){
		m_brother->SetAbsMatReq( 1, srcmotid, srcframe, firstframe );
	}
	return curmp;
}

int CBone::DestroyMotionKey( int srcmotid )
{
	CMotionPoint* curmp = m_motionkey[ srcmotid ];
	while( curmp ){
		CMotionPoint* nextmp = curmp->GetNext();
		delete curmp;
		curmp = nextmp;
	}

	m_motionkey[ srcmotid ] = NULL;

	return 0;
}


int CBone::AddBoneMarkIfNot( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double curframe, int flag )
{
	map<double, int> curmark;
	map<int, map<double, int>>::iterator itrcur;
	itrcur = m_motmark.find( motid );
	if( itrcur == m_motmark.end() ){
		curmark.clear();
	}else{
		curmark = itrcur->second;
	}
	map<double, int>::iterator itrmark;
	itrmark = curmark.find( curframe );
	if( itrmark == curmark.end() ){
		curmark[ curframe ] = flag;
		m_motmark[ motid ] = curmark;
	}

	return 0;
}

int CBone::DelBoneMarkRange( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe )
{
	map<int, map<double, int>>::iterator itrcur;
	itrcur = m_motmark.find( motid );
	if( itrcur == m_motmark.end() ){
		return 0;
	}

	map<double, int> curmark;
	curmark = itrcur->second;

	double frame;
	for( frame = (startframe + 1.0); frame <= (endframe - 1.0); frame += 1.0 ){
		map<double, int>::iterator itrfind;
		itrfind = curmark.find( frame );
		if( itrfind != curmark.end() ){
			curmark.erase( itrfind );
		}
	}

	m_motmark[ motid ] = curmark;

	return 0;
}

int CBone::AddBoneMotMark( int motid, OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag )
{
	if( startframe != endframe ){
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, startframe, flag );
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, endframe, flag );
		DelBoneMarkRange( motid, owpTimeline, curlineno, startframe, endframe );
	}else{
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, startframe, flag );
	}

	return 0;
}

int CBone::CalcLocalInfo( int motid, double frameno, CMotionPoint* pdstmp )
{

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(motid, frameno);
	if( m_parent ){
		if( pcurmp ){
			pparmp = m_parent->GetMotionPoint(motid, frameno);
			if( pparmp ){
				CMotionPoint setmp;
				D3DXMATRIX invpar = pparmp->GetInvWorldMat();
				D3DXMATRIX localmat = pcurmp->GetWorldMat() * invpar;
				//pcurmp->CalcQandTra(localmat, this);
				setmp.CalcQandTra(localmat, this);

				int inirotcur, inirotpar;
				inirotcur = IsInitRot(pcurmp->GetWorldMat());
				inirotpar = IsInitRot(pparmp->GetWorldMat());
				if (inirotcur && inirotpar){
					CQuaternion iniq;
					iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
					//pcurmp->SetQ(iniq);
					setmp.SetQ(iniq);
				}

				*pdstmp = setmp;
			}else{
				CMotionPoint inimp;
				*pdstmp = inimp;
				_ASSERT( 0 );
				return 0;
			}
		}else{
			CMotionPoint inimp;
			*pdstmp = inimp;

			//_ASSERT( 0 );
			return 0;
		}
	}else{
		if( pcurmp ){
			CMotionPoint setmp;
			D3DXMATRIX localmat = pcurmp->GetWorldMat();
			setmp.CalcQandTra( localmat, this );

			int inirotcur;
			inirotcur = IsInitRot(pcurmp->GetWorldMat());
			if (inirotcur ){
				CQuaternion iniq;
				iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
				setmp.SetQ(iniq);
			}

			*pdstmp = setmp;

		}else{
			CMotionPoint inimp;
			*pdstmp = inimp;

			//_ASSERT( 0 );
			return 0;
		}
	}

	return 0;
}

int CBone::CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp)
{

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(motid, frameno);
	if (m_parent){
		if (pcurmp){
			pparmp = GetMotionPoint(motid, frameno);
			if (pparmp){
				D3DXMATRIX invpar = pparmp->GetInvWorldMat();
				D3DXMATRIX invinitmat = GetInvInitMat();
				D3DXMATRIX localmat = invinitmat * pcurmp->GetWorldMat() * invpar;//world == init * local * parだからlocalを計算するには、invinit * world * invpar。
				pcurmp->CalcQandTra(localmat, this);
			}
			else{
				_ASSERT(0);
				return 0;
			}
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}
	else{
		if (pcurmp){
			D3DXMATRIX invinitmat = GetInvInitMat();
			D3DXMATRIX localmat = invinitmat * pcurmp->GetWorldMat();
			pcurmp->CalcQandTra(localmat, this);
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}

	*pdstmp = *pcurmp;

	return 0;
}


int CBone::GetBoneNum()
{
	int retnum = 0;

	if( !m_child ){
		return 0;
	}else{
		retnum++;
	}

	CBone* cbro = m_child->m_brother;
	while( cbro ){
		retnum++;
		cbro = cbro->m_brother;
	}

	return retnum;
}

int CBone::CalcFirstFrameBonePos(D3DXMATRIX srcmat)
{
	D3DXVECTOR3 jpos = GetJointFPos();
	D3DXVec3TransformCoord(&m_firstframebonepos, &jpos, &srcmat);

	//if ((m_firstframebonepos.x == 0.0f) && (m_firstframebonepos.y == 0.0f) && (m_firstframebonepos.z == 0.0f)){
	//	_ASSERT(0);
	//}
	return 0;
}

void CBone::CalcFirstAxisMatX()
{
	D3DXVECTOR3 curpos;
	D3DXVECTOR3 chilpos;

	if (m_parent){
		CalcAxisMatX_aft(m_parent->GetJointFPos(), GetJointFPos(), &m_firstaxismatX);
	}

}
void CBone::CalcFirstAxisMatZ()
{
	D3DXVECTOR3 curpos;
	D3DXVECTOR3 chilpos;

	if (m_parent){
		CalcAxisMatZ_aft(m_parent->GetJointFPos(), GetJointFPos(), &m_firstaxismatZ);
	}

}

int CBone::CalcBoneDepth()
{
	int retdepth = 0;
	CBone* curbone = this;
	if (curbone){
		while (curbone->GetParent()){
			retdepth++;
			curbone = curbone->GetParent();
		}
	}
	return retdepth;
}

D3DXVECTOR3 CBone::GetJointFPos()
{
	
	if (m_parmodel->GetOldAxisFlagAtLoading() == 0){
		return m_jointfpos;
	}
	else{
		return m_oldjointfpos;
	}
}
void CBone::SetJointFPos(D3DXVECTOR3 srcpos)
{ 
	m_jointfpos = srcpos; 
}
void CBone::SetOldJointFPos(D3DXVECTOR3 srcpos){
	m_oldjointfpos = srcpos;
}


D3DXVECTOR3 CBone::CalcLocalEulZXY(int axiskind, int srcmotid, double srcframe, enum tag_befeulkind befeulkind, int isfirstbone, D3DXVECTOR3* directbefeul)
{
	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)

	D3DXVECTOR3 cureul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 befeul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	const WCHAR* bonename = GetWBoneName();
	if (wcscmp(bonename, L"RootNode") == 0){
		return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
	}

	if (befeulkind == BEFEUL_BEFFRAME){
		//1つ前のフレームのEULはすでに計算されていると仮定する。
		double befframe;
		befframe = srcframe - 1.0;
		if (befframe >= -0.0001){
			CMotionPoint* befmp;
			befmp = GetMotionPoint(srcmotid, befframe);
			if (befmp){
				befeul = befmp->GetLocalEul();
			}
		}
	}
	else if ((befeulkind == BEFEUL_DIRECT) && directbefeul){
		befeul = *directbefeul;
	}

	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);//local!!!
	
	D3DXMATRIX axismat;
	CQuaternion axisq;
	int multworld = 0;//local!!!
	axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	axisq.RotationMatrix(axismat);

	if (axiskind == -1){
		if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
			tmpmp.GetQ().CalcFBXEulZXY(&axisq, befeul, &cureul, isfirstbone);
		}
		else{
			tmpmp.GetQ().CalcFBXEulZXY(0, befeul, &cureul, isfirstbone);
		}
	}
	else if (axiskind != BONEAXIS_GLOBAL){
		tmpmp.GetQ().CalcFBXEulZXY(&axisq, befeul, &cureul, isfirstbone);
	}
	else{
		tmpmp.GetQ().CalcFBXEulZXY(0, befeul, &cureul, isfirstbone);
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		D3DXVECTOR3 oldeul = curmp->GetLocalEul();
		if (IsSameEul(oldeul, cureul) == 0){
			return cureul;
		}
		else{
			return oldeul;
		}
	}
	else{
		return cureul;
	}
}

D3DXMATRIX CBone::CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe)
{
	D3DXMATRIX selm;
	D3DXMatrixIdentity(&selm);

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, srcframe);
	if (!pcurmp){
		//_ASSERT(0);
		return selm;
	}
	if (m_parent){
		pparmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (!pparmp){
			_ASSERT(0);
			return selm;
		}
	}

	D3DXMATRIX worldmat, parworldmat;
	if (pcurmp){
		if (g_previewFlag != 5){
			worldmat = pcurmp->GetWorldMat();
		}
		else{
			worldmat = GetBtMat();
		}
	}
	else{
		D3DXMatrixIdentity(&worldmat);
	}
	if (pparmp){
		if (g_previewFlag != 5){
			parworldmat = pparmp->GetWorldMat();
		}
		else{
			parworldmat = m_parent->GetBtMat();
		}
	}
	else{
		D3DXMatrixIdentity(&parworldmat);
	}


	if (anglelimitaxisflag == 0){
		if (g_boneaxis == 2){
			//global axis
			D3DXMatrixIdentity(&selm);
		}
		else if (g_boneaxis == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								D3DXMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							D3DXMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
			}
		}
		else if (g_boneaxis == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								D3DXMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							D3DXMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			D3DXMatrixIdentity(&selm);
		}
	}
	else{
		if (m_anglelimit.boneaxiskind == 2){
			//global axis
			D3DXMatrixIdentity(&selm);
		}
		else if (m_anglelimit.boneaxiskind == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								D3DXMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							D3DXMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (multworld == 1){
						selm = GetNodeMat() * worldmat;
					}
					else{
						selm = GetNodeMat();
					}
				}
			}
			else{
				//endjoint
				if (multworld == 1){
					selm = worldmat;
				}
				else{
					D3DXMatrixIdentity(&selm);
				}
			}
		}
		else if (m_anglelimit.boneaxiskind == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								D3DXMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							D3DXMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						D3DXMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			D3DXMatrixIdentity(&selm);
		}
	}

	if (settraflag == 0){
		selm._41 = 0.0f;
		selm._42 = 0.0f;
		selm._43 = 0.0f;
	}
	else{
		D3DXVECTOR3 aftjpos;
		D3DXVec3TransformCoord(&aftjpos, &(GetJointFPos()), &worldmat);

		selm._41 = aftjpos.x;
		selm._42 = aftjpos.y;
		selm._43 = aftjpos.z;
	}


	return selm;
}


int CBone::SetWorldMatFromEul(int inittraflag, int setchildflag, D3DXVECTOR3 srceul, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	if (!m_child){
		return 0;
	}

	D3DXMATRIX axismat;
	CQuaternion axisq;
	int multworld = 0;//local!!!
	axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	axisq.RotationMatrix(axismat);

	CQuaternion newrot;
	if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
		newrot.SetRotation(&axisq, srceul);
	}
	else{
		newrot.SetRotation(0, srceul);
	}


	QuaternionInOrder(srcmotid, srcframe, &newrot);


	D3DXMATRIX newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	D3DXMatrixIdentity(&befrotmat);
	D3DXMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	D3DXMatrixIdentity(&aftrotmat);
	D3DXMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	if (inittraflag == 0){
		D3DXVECTOR3 traanim = CalcLocalTraAnim(srcmotid, srcframe);
		D3DXMATRIX tramat;
		D3DXMatrixIdentity(&tramat);
		D3DXMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);
		newlocalmat = newlocalmat * tramat;
	}

	D3DXMATRIX newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			D3DXMATRIX parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		curmp->SetLocalEul(srceul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}

int CBone::SetWorldMatFromQAndTra(int setchildflag, CQuaternion axisq, CQuaternion srcq, D3DXVECTOR3 srctra, int srcmotid, double srcframe)
{
	if (!m_child){
		return 0;
	}

	CQuaternion invaxisq;
	axisq.inv(&invaxisq);
	CQuaternion newrot = invaxisq * srcq * axisq;

	D3DXMATRIX newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	D3DXMatrixIdentity(&befrotmat);
	D3DXMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	D3DXMatrixIdentity(&aftrotmat);
	D3DXMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	D3DXMATRIX tramat;
	D3DXMatrixIdentity(&tramat);
	D3DXMatrixTranslation(&tramat, srctra.x, srctra.y, srctra.z);
	newlocalmat = newlocalmat * tramat;


	D3DXMATRIX newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			D3DXMATRIX parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		D3DXVECTOR3 neweul = CalcLocalEulZXY(-1, srcmotid, srcframe, BEFEUL_ZERO, 0);
		curmp->SetLocalEul(neweul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;




}


int CBone::SetWorldMatFromEulAndTra(int setchildflag, D3DXVECTOR3 srceul, D3DXVECTOR3 srctra, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	if (!m_child){
		return 0;
	}

	D3DXMATRIX axismat;
	CQuaternion axisq;
	int multworld = 0;//local!!!
	axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	axisq.RotationMatrix(axismat);
	CQuaternion invaxisq;
	axisq.inv(&invaxisq);

	CQuaternion newrot;
	if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
		newrot.SetRotation(&axisq, srceul);
	}
	else{
		newrot.SetRotation(0, srceul);
	}

	D3DXMATRIX newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	D3DXMatrixIdentity(&befrotmat);
	D3DXMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	D3DXMatrixIdentity(&aftrotmat);
	D3DXMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	D3DXMATRIX tramat;
	D3DXMatrixIdentity(&tramat);
	D3DXMatrixTranslation(&tramat, srctra.x, srctra.y, srctra.z);
	newlocalmat = newlocalmat * tramat;


	D3DXMATRIX newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			D3DXMATRIX parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		curmp->SetLocalEul(srceul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}


int CBone::SetLocalEul(int srcmotid, double srcframe, D3DXVECTOR3 srceul)
{
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (!curmp){
		_ASSERT(0);
		return 1;
	}

	curmp->SetLocalEul(srceul);

	return 0;

}

void CBone::SetWorldMat(int setchildflag, int srcmotid, double srcframe, D3DXMATRIX srcmat)
{
	//if pose is change, return 1 else return 0
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (!curmp){
		return;
	}

	if ((g_wmatDirectSetFlag == false) && (g_underRetargetFlag == false)){
		D3DXMATRIX saveworldmat;
		saveworldmat = curmp->GetWorldMat();

		D3DXVECTOR3 oldeul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		int paraxsiflag = 1;
		int isfirstbone = 0;
		oldeul = CalcLocalEulZXY(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);


		curmp->SetWorldMat(srcmat);//tmp time
		D3DXVECTOR3 neweul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		neweul = CalcLocalEulZXY(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);

		curmp->SetWorldMat(saveworldmat);

		int ismovable = ChkMovableEul(neweul);
		if (ismovable == 1){
			if (IsSameEul(oldeul, neweul) == 0){
				int inittraflag0 = 0;
				SetWorldMatFromEul(inittraflag0, setchildflag, neweul, srcmotid, srcframe);//setchildflag有り!!!!
			}
			else{
				curmp->SetBefWorldMat(curmp->GetWorldMat());
			}
		}
		else{
			curmp->SetBefWorldMat(curmp->GetWorldMat());
		}
	}
	else{
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(srcmat);

		D3DXVECTOR3 neweul = CalcLocalEulZXY(-1, srcmotid, srcframe, BEFEUL_ZERO, 0);
		curmp->SetLocalEul(neweul);
	}
	/*
	if (setchildflag){
		if (GetChild()){
			CQuaternion dummyq;
			GetChild()->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
		}
	}
	*/
}

int CBone::ChkMovableEul(D3DXVECTOR3 srceul)
{
	if (g_limitdegflag == false){
		return 1;//movable
	}

	int dontmove = 0;
	int axiskind;

	float chkval[3];
	chkval[0] = srceul.x;
	chkval[1] = srceul.y;
	chkval[2] = srceul.z;

	for (axiskind = AXIS_X; axiskind <= AXIS_Z; axiskind++){
		if (m_anglelimit.via180flag[axiskind] == 0){
			if ((m_anglelimit.lower[axiskind] > (int)chkval[axiskind]) || (m_anglelimit.upper[axiskind] < (int)chkval[axiskind])){
				dontmove++;
			}
		}
		else{
			if ((m_anglelimit.lower[axiskind] <= (int)chkval[axiskind]) && (m_anglelimit.upper[axiskind] >= (int)chkval[axiskind])){
				dontmove++;
			}
		}
	}

	if (dontmove != 0){
		return 0;
	}
	else{
		return 1;//movable
	}
}


float CBone::LimitAngle(enum tag_axiskind srckind, float srcval)
{
	SetAngleLimitOff(&m_anglelimit);
	if (m_anglelimit.limitoff[srckind] == 1){
		return srcval;
	}
	else{
		int newval;
		newval = min((int)srcval, m_anglelimit.upper[srckind]);
		newval = max(newval, m_anglelimit.lower[srckind]);
		return (float)newval;
	}
}


D3DXVECTOR3 CBone::LimitEul(D3DXVECTOR3 srceul)
{
	D3DXVECTOR3 reteul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	reteul.x = LimitAngle(AXIS_X, srceul.x);
	reteul.y = LimitAngle(AXIS_Y, srceul.y);
	reteul.z = LimitAngle(AXIS_Z, srceul.z);

	return reteul;
}

ANGLELIMIT CBone::GetAngleLimit()
{
	::SetAngleLimitOff(&m_anglelimit);
	return m_anglelimit;
};
void CBone::SetAngleLimit(ANGLELIMIT srclimit)
{
	m_anglelimit = srclimit;

	int axiskind;
	for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++){
		if (m_anglelimit.lower[axiskind] < -180){
			m_anglelimit.lower[axiskind] = -180;
		}
		else if (m_anglelimit.lower[axiskind] > 180){
			m_anglelimit.lower[axiskind] = 180;
		}

		if (m_anglelimit.upper[axiskind] < -180){
			m_anglelimit.upper[axiskind] = -180;
		}
		else if (m_anglelimit.upper[axiskind] > 180){
			m_anglelimit.upper[axiskind] = 180;
		}

		if (m_anglelimit.lower[axiskind] > m_anglelimit.upper[axiskind]){
			_ASSERT(0);
			//swap
			int tmpval = m_anglelimit.lower[axiskind];
			m_anglelimit.lower[axiskind] = m_anglelimit.upper[axiskind];
			m_anglelimit.upper[axiskind] = tmpval;
		}
	}
	SetAngleLimitOff(&m_anglelimit);
};


int CBone::GetFreeCustomRigNo()
{
	int rigno;
	int findrigno = -1;
	for (rigno = 0; rigno < MAXRIGNUM; rigno++){
		if (m_customrig[rigno].useflag == 0){//0 : free mark
			findrigno = rigno;
			break;
		}
	}
	
	if (findrigno == -1){
		//freeがなかった場合、rentalから
		for (rigno = 0; rigno < MAXRIGNUM; rigno++){
			if (m_customrig[rigno].useflag == 1){//1 : rental mark
				findrigno = rigno;
				break;
			}
		}
	}

	return findrigno;
}
CUSTOMRIG CBone::GetFreeCustomRig()
{
	int freerigno = GetFreeCustomRigNo();
	if ((freerigno >= 0) && (freerigno < MAXRIGNUM)){
		m_customrig[freerigno].useflag = 1;//1 : rental mark
		//_ASSERT(0);
		return m_customrig[freerigno];
	}
	else{
		_ASSERT(0);
		CUSTOMRIG dummycr;
		::InitCustomRig(&dummycr, 0, 0);
		return dummycr;
	}
}

CUSTOMRIG CBone::GetCustomRig(int rigno)
{
	if ((rigno >= 0) && (rigno < MAXRIGNUM)){
		return m_customrig[rigno];
	}
	else{
		_ASSERT(0);
		CUSTOMRIG dummycr;
		::InitCustomRig(&dummycr, 0, 0);
		return dummycr;
	}
}
void CBone::SetCustomRig(CUSTOMRIG srccr)
{
	int isvalid = IsValidCustomRig(m_parmodel, srccr, this);
	if (isvalid == 1){
		m_customrig[srccr.rigno] = srccr;
		m_customrig[srccr.rigno].useflag = 2;//2 : valid mark
	}
}

D3DXMATRIX CBone::CalcSymXMat(int srcmotid, double srcframe)
{
	return CalcSymXMat2(srcmotid, srcframe, SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS);
}

D3DXMATRIX CBone::CalcSymXMat2(int srcmotid, double srcframe, int symrootmode)
{
	/*
	enum
	{
		//for bit mask operation
		SYMROOTBONE_SAMEORG = 0,
		SYMROOTBONE_SYMDIR = 1,
		SYMROOTBONE_SYMPOS = 2
	};
	*/

	D3DXMATRIX directsetmat;
	D3DXMatrixIdentity(&directsetmat);

	int rotcenterflag1 = 1;
	if (GetParent()){
		directsetmat = CalcLocalSymRotMat(rotcenterflag1, srcmotid, srcframe);
	}
	else{
		//root bone
		if (symrootmode == SYMROOTBONE_SAMEORG){
			directsetmat = GetWorldMat(srcmotid, srcframe);
			return directsetmat;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		else if(symrootmode & SYMROOTBONE_SYMDIR){
			directsetmat = CalcLocalSymRotMat(rotcenterflag1, srcmotid, srcframe);
		}
		else{
			directsetmat = CalcLocalRotMat(rotcenterflag1, srcmotid, srcframe);
		}
	}

////tra anim
	D3DXVECTOR3 curanimtra = CalcLocalTraAnim(srcmotid, srcframe);

	if (GetParent()){
		directsetmat._41 += -curanimtra.x;//inv signe
		directsetmat._42 += curanimtra.y;
		directsetmat._43 += curanimtra.z;
	}
	else{
		//root bone
		if (symrootmode & SYMROOTBONE_SYMPOS){
			directsetmat._41 += -curanimtra.x;//inv signe
			directsetmat._42 += curanimtra.y;
			directsetmat._43 += curanimtra.z;
		}
		else{
			directsetmat._41 += curanimtra.x;//same signe
			directsetmat._42 += curanimtra.y;
			directsetmat._43 += curanimtra.z;
		}
	}

	return directsetmat;
}


D3DXMATRIX CBone::GetWorldMat(int srcmotid, double srcframe)
{
	D3DXMATRIX curmat;
	D3DXMatrixIdentity(&curmat);
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		curmat = curmp->GetWorldMat();
	}
	return curmat;
}

D3DXMATRIX CBone::CalcLocalRotMat(int rotcenterflag, int srcmotid, double srcframe)
{
	D3DXMATRIX curmat;
	curmat = GetWorldMat(srcmotid, srcframe);
	CMotionPoint curlocalmp;
	CalcLocalInfo(srcmotid, srcframe, &curlocalmp);
	D3DXMATRIX currotmat;
	currotmat = curlocalmp.GetQ().MakeRotMatX();

	currotmat._41 = 0.0f;
	currotmat._42 = 0.0f;
	currotmat._43 = 0.0f;

	if (rotcenterflag == 1){
		D3DXMATRIX befrotmat, aftrotmat;
		D3DXMatrixIdentity(&befrotmat);
		D3DXMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
		D3DXMatrixIdentity(&aftrotmat);
		D3DXMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
		currotmat = befrotmat * currotmat * aftrotmat;
	}

	return currotmat;
}


D3DXMATRIX CBone::CalcLocalSymRotMat(int rotcenterflag, int srcmotid, double srcframe)
{
	D3DXMATRIX retmat;

	int symboneno = 0;
	int existflag = 0;
	m_parmodel->GetSymBoneNo(GetBoneNo(), &symboneno, &existflag);
	if (symboneno >= 0){
		CBone* symbone = m_parmodel->GetBoneByID(symboneno);
		_ASSERT(symbone);
		if (symbone){
			//if (symbone == this){
			//	WCHAR dbgmes[1024];
			//	swprintf_s(dbgmes, 1024, L"CalcLocalSymRotMat : frame %lf : samebone : this[%s]--sym[%s]", srcframe, GetWBoneName(), symbone->GetWBoneName());
			//	::MessageBox(NULL, dbgmes, L"check", MB_OK);
			//}

			CMotionPoint symlocalmp;
			symbone->CalcLocalInfo(srcmotid, srcframe, &symlocalmp);
			retmat = symlocalmp.GetQ().CalcSymX2();

			retmat._41 = 0.0f;
			retmat._42 = 0.0f;
			retmat._43 = 0.0f;

			if (rotcenterflag == 1){
				D3DXMATRIX befrotmat, aftrotmat;
				D3DXMatrixIdentity(&befrotmat);
				D3DXMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
				D3DXMatrixIdentity(&aftrotmat);
				D3DXMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
				retmat = befrotmat * retmat * aftrotmat;
			}
		}
		else{
			retmat = CalcLocalRotMat(rotcenterflag, srcmotid, srcframe);
			_ASSERT(0);
		}
	}
	else{
		retmat = CalcLocalRotMat(rotcenterflag, srcmotid, srcframe);
		_ASSERT(0);
	}

	return retmat;
}

D3DXVECTOR3 CBone::CalcLocalTraAnim(int srcmotid, double srcframe)
{

	D3DXMATRIX curmat;
	curmat = GetWorldMat(srcmotid, srcframe);

	int rotcenterflag1 = 1;
	D3DXMATRIX curlocalrotmat, invcurlocalrotmat;
	curlocalrotmat = CalcLocalRotMat(rotcenterflag1, srcmotid, srcframe);
	D3DXMatrixInverse(&invcurlocalrotmat, NULL, &curlocalrotmat);
	D3DXMATRIX parmat, invparmat;
	D3DXMatrixIdentity(&parmat);
	invparmat = parmat;
	if (GetParent()){
		parmat = GetParent()->GetWorldMat(srcmotid, srcframe);
		D3DXMatrixInverse(&invparmat, NULL, &parmat);
	}

	D3DXMATRIX curmvmat;
	curmvmat = invcurlocalrotmat * curmat * invparmat;

	D3DXVECTOR3 zeropos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 curanimtra;
	D3DXVec3TransformCoord(&curanimtra, &zeropos, &curmvmat);

	return curanimtra;
}


int CBone::PasteMotionPoint(int srcmotid, double srcframe, CMotionPoint srcmp)
{
	CMotionPoint* newmp = 0;
	newmp = GetMotionPoint(srcmotid, srcframe);
	if (newmp){
		D3DXMATRIX setmat = srcmp.GetWorldMat();

		CBone* parbone = GetParent();
		if (parbone){
			CMotionPoint* parmp = parbone->GetMotionPoint(srcmotid, srcframe);
			if (parmp){
				setmat = setmat * parmp->GetWorldMat();
			}
		}

		int setmatflag1 = 1;
		CQuaternion dummyq;
		D3DXVECTOR3 dummytra = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		g_underRetargetFlag = true;
		RotBoneQReq(0, srcmotid, srcframe, dummyq, 0, dummytra, setmatflag1, &setmat);
		g_underRetargetFlag = false;

		//オイラー角初期化
		D3DXVECTOR3 cureul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		int paraxsiflag = 1;
		int isfirstbone = 0;
		cureul = CalcLocalEulZXY(paraxsiflag, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);
		SetLocalEul(srcmotid, srcframe, cureul);

	}

	return 0;
}

D3DXVECTOR3 CBone::CalcFBXEul(int srcmotid, double srcframe, D3DXVECTOR3* befeulptr)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);
	int isfirstbone;
	if (GetParent()){
		isfirstbone = 0;
	}
	else{
		isfirstbone = 1;
	}

	D3DXVECTOR3 befeul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 cureul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	if (befeulptr){
		befeul = *befeulptr;
	}

	//tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);
	tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);

	return cureul;

}
D3DXVECTOR3 CBone::CalcFBXEulZXY(int srcmotid, double srcframe, D3DXVECTOR3* befeulptr)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);
	int isfirstbone;
	if (GetParent()){
		isfirstbone = 0;
	}
	else{
		isfirstbone = 1;
	}

	D3DXVECTOR3 befeul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 cureul = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	if (befeulptr){
		befeul = *befeulptr;
	}

	//tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);
	tmpmp.GetQ().CalcFBXEulZXY(0, befeul, &cureul, isfirstbone);

	return cureul;

}
D3DXVECTOR3 CBone::CalcFBXTra(int srcmotid, double srcframe)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);

	D3DXVECTOR3 orgtra;
	CBone* parbone = GetParent();
	if (parbone){
		orgtra = GetJointFPos() - parbone->GetJointFPos();
	}
	else{
		orgtra = GetJointFPos();
	}

	D3DXVECTOR3 fbxtra = orgtra + tmpmp.GetTra();
	return fbxtra;

}

int CBone::QuaternionInOrder(int srcmotid, double srcframe, CQuaternion* srcdstq)
{
	CQuaternion beflocalq;
	CMotionPoint befmp;
	double befframe = srcframe - 1.0;
	if (befframe >= 0.0001){
		CalcLocalInfo(srcmotid, befframe, &befmp);
		beflocalq = befmp.GetQ();
	}

	beflocalq.InOrder(srcdstq);

	return 0;

}

/*
int CBone::CalcNewBtMat(CRigidElem* srcre, CBone* chilbone, D3DXMATRIX* dstmat, D3DXVECTOR3* dstpos)
{
	D3DXMatrixIdentity(dstmat);
	*dstpos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if (!chilbone || !dstmat || !dstpos){
		return 1;
	}

	D3DXVECTOR3 jointfpos;
	D3DXMATRIX firstworld;
	D3DXMATRIX invfirstworld;
	D3DXMATRIX curworld;
	D3DXMATRIX befworld;
	D3DXMATRIX invbefworld;
	D3DXMATRIX diffworld;
	D3DXVECTOR3 rigidcenter;
	D3DXMATRIX multmat;
	D3DXMATRIX tramat;


	firstworld = GetStartMat2();
	D3DXMatrixInverse(&invfirstworld, NULL, &firstworld);


	//current
	if (GetBtKinFlag() == 1){
		diffworld = invfirstworld * GetCurMp().GetWorldMat();
		tramat = GetCurMp().GetWorldMat();
	}
	else{
		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
		if (GetCurMp().GetBtFlag() == 0){
			diffworld = invfirstworld * GetCurMp().GetWorldMat();
			tramat = GetCurMp().GetWorldMat();
		}
		else{
			//ここでのBtMatは一回前の姿勢。
			diffworld = invfirstworld * GetCurMp().GetBtMat();
			tramat = GetCurMp().GetBtMat();
		}
	}
	jointfpos = GetJointFPos();
	D3DXVec3TransformCoord(&m_btparpos, &jointfpos, &tramat);

	//child
	jointfpos = chilbone->GetJointFPos();
	D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &tramat);



	multmat = srcre->GetFirstcapsulemat() * diffworld;
	//rigidcenter = (m_btparpos + m_btchilpos) * 0.5f;
	rigidcenter = m_btparpos;


	*dstmat = multmat;
	*dstpos = rigidcenter;

	return 0;
}
*/

int CBone::CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* chilbone, D3DXMATRIX* dstmat, D3DXVECTOR3* dstpos)
{
	D3DXMatrixIdentity(dstmat);
	*dstpos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	if (!chilbone || !dstmat || !dstpos){
		return 1;
	}

	D3DXVECTOR3 jointfpos;
	D3DXMATRIX firstworld;
	D3DXMATRIX invfirstworld;
	D3DXMATRIX curworld;
	D3DXMATRIX befworld;
	D3DXMATRIX invbefworld;
	D3DXMATRIX diffworld;
	D3DXVECTOR3 rigidcenter;
	D3DXMATRIX multmat;
	D3DXMATRIX tramat;


	firstworld = GetStartMat2();
	D3DXMatrixInverse(&invfirstworld, NULL, &firstworld);

	D3DXMATRIX befbtmat;
	if (GetBtFlag() == 0){
		//再起処理中のまだ未セットの状態の場合
		befbtmat = GetBtMat();
	}
	else{
		//再起処理中のすでにセットした状態の場合
		befbtmat = GetBefBtMat();
	}

	//current
	if (GetBtKinFlag() == 1){
		diffworld = invfirstworld * GetCurMp().GetWorldMat();
		tramat = GetCurMp().GetWorldMat();

		jointfpos = GetJointFPos();
		D3DXVec3TransformCoord(&m_btparpos, &jointfpos, &tramat);
		jointfpos = chilbone->GetJointFPos();
		D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &tramat);

	}
	else{
		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
		if (srcmodel->GetBtCnt() == 0){
			diffworld = invfirstworld * GetCurMp().GetWorldMat();
			tramat = GetCurMp().GetWorldMat();

			jointfpos = GetJointFPos();
			D3DXVec3TransformCoord(&m_btparpos, &jointfpos, &tramat);
			jointfpos = chilbone->GetJointFPos();
			D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &tramat);
		}
		else{
			if (GetParent() && (GetParent()->GetBtKinFlag() == 1)){
				//ここでのBtMatは一回前の姿勢。

				//BtMatにアニメーションの移動成分のみを掛けたものを新しい姿勢行列として子供ジョイント位置を計算してシミュレーションに使用する。
				curworld = GetCurMp().GetWorldMat();
				befworld = GetCurMp().GetBefWorldMat();

				D3DXVECTOR3 befparpos, curparpos;
				jointfpos = GetJointFPos();
				D3DXVec3TransformCoord(&befparpos, &jointfpos, &befworld);
				D3DXVec3TransformCoord(&curparpos, &jointfpos, &curworld);
				D3DXVECTOR3 diffmv = curparpos - befparpos;

				D3DXMATRIX diffmvmat;
				D3DXMatrixIdentity(&diffmvmat);
				D3DXMatrixTranslation(&diffmvmat, diffmv.x, diffmv.y, diffmv.z);

				D3DXMatrixInverse(&invbefworld, NULL, &befworld);
				D3DXMATRIX newtramat = befbtmat * diffmvmat;

				diffworld = invfirstworld * newtramat;

				m_btparpos = curparpos;
				jointfpos = chilbone->GetJointFPos();
				//D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &befbtmat);
				D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &newtramat);
			}
			else{
				diffworld = invfirstworld * befbtmat;
				tramat = befbtmat;

				jointfpos = GetJointFPos();
				D3DXVec3TransformCoord(&m_btparpos, &jointfpos, &tramat);
				jointfpos = chilbone->GetJointFPos();
				D3DXVec3TransformCoord(&m_btchilpos, &jointfpos, &tramat);
			}
		}
	}

	multmat = srcre->GetFirstcapsulemat() * diffworld;
	rigidcenter = (m_btparpos + m_btchilpos) * 0.5f;

	*dstmat = multmat;
	*dstpos = rigidcenter;

	return 0;
}


D3DXVECTOR3 CBone::GetChildWorld(){
	if (g_previewFlag != 5){
		D3DXVec3TransformCoord(&m_childworld, &m_jointfpos, &m_curmp.GetWorldMat());
	}
	else{
		D3DXMATRIX wmat;
		if (m_parent){
			wmat = m_parent->GetBtMat();
		}
		else{
			wmat = GetBtMat();
		}

		//D3DXVec3TransformCoord(&m_childworld, &m_jointfpos, &(GetBtMat()));
		D3DXVec3TransformCoord(&m_childworld, &m_jointfpos, &wmat);
	}
	return m_childworld;
};

int CBone::LoadCapsuleShape(LPDIRECT3DDEVICE9 pdev)
{
	WCHAR wfilename[MAX_PATH];
	WCHAR mpath[MAX_PATH];

	wcscpy_s(mpath, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(mpath, TEXT('\\'));
	if (lasten){
		*lasten = 0L;
		last2en = wcsrchr(mpath, TEXT('\\'));
		if (last2en){
			*last2en = 0L;
			wcscat_s(mpath, MAX_PATH, L"\\Media\\MameMedia");
		}
	}

	m_coldisp[COL_CONE_INDEX] = new CModel();
	if (!m_coldisp[COL_CONE_INDEX]){
		_ASSERT(0);
		return 1;
	}

	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"cone_dirY.mqo");
	CallF(m_coldisp[COL_CONE_INDEX]->LoadMQO(pdev, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_CONE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_CAPSULE_INDEX] = new CModel();
	if (!m_coldisp[COL_CAPSULE_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"capsule_dirY.mqo");
	CallF(m_coldisp[COL_CAPSULE_INDEX]->LoadMQO(pdev, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_CAPSULE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_SPHERE_INDEX] = new CModel();
	if (!m_coldisp[COL_SPHERE_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"sphere_dirY.mqo");
	CallF(m_coldisp[COL_SPHERE_INDEX]->LoadMQO(pdev, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_SPHERE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_BOX_INDEX] = new CModel();
	if (!m_coldisp[COL_BOX_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"box.mqo");
	CallF(m_coldisp[COL_BOX_INDEX]->LoadMQO(pdev, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_BOX_INDEX]->MakeDispObj(), return 1);

	return 0;
}

CModel* CBone::GetCurColDisp(CBone* childbone)
{
	CRigidElem* curre = m_rigidelem[childbone];
	if (!curre){
		_ASSERT(0);
		return 0;
	}

	//_ASSERT(colptr);
	_ASSERT(childbone);

	CModel* curcoldisp = m_coldisp[curre->GetColtype()];
	_ASSERT(curcoldisp);

	return curcoldisp;
}
