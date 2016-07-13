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

#include <BtObject.h>

#include <Bone.h>
#include <quaternion.h>
#include <RigidElem.h>

#define DBGH
#include <dbg.h>

using namespace std;

extern float g_miscale;
extern float g_l_kval[3];
extern float g_a_kval[3];
extern int g_previewFlag;			// プレビューフラグ

CBtObject::CBtObject( CBtObject* parbt, btDynamicsWorld* btWorld )
{
	InitParams();
	//if( parbt ){
	//	parbt->AddChild( this );
	//}
	m_btWorld = btWorld;
}

CBtObject::~CBtObject()
{
	DestroyObjs();
}

int CBtObject::InitParams()
{
	m_connectflag = 0;
	m_constzrad = 0.0f;
	D3DXMatrixIdentity( &m_transmat );
	D3DXMatrixIdentity( &m_xworld );
	m_btWorld = 0;

	m_topflag = 0;
	m_parbone = 0;
	m_endbone = 0;
	m_bone = 0;
	m_colshape = 0;
	m_rigidbody = 0;
	m_constraint.clear();

	m_parbt = 0;
	m_chilbt.clear();

	m_curpivot.setValue( 0.0f, 0.0f, 0.0f );
	m_chilpivot.setValue( 0.0f, 0.0f, 0.0f );

	m_FrameA.setIdentity();
	m_FrameB.setIdentity();


	return 0;
}
int CBtObject::DestroyObjs()
{
	int chilno;
	for( chilno = 0; chilno < (int)m_constraint.size(); chilno++ ){
		//btTypedConstraint* constptr = m_constraint[ chilno ];
		btGeneric6DofSpringConstraint* constptr = m_constraint[chilno].constraint;
		if( constptr ){
			m_btWorld->removeConstraint( constptr );
			delete constptr;
		}
	}
	m_constraint.clear();

	if( m_rigidbody ){
		if( m_rigidbody->getMotionState() ){
			delete m_rigidbody->getMotionState();
		}
		m_btWorld->removeRigidBody( m_rigidbody );
		delete m_rigidbody;
		m_rigidbody = 0;
	}

	if( m_colshape ){
		delete m_colshape;
		m_colshape = 0;
	}

	m_chilbt.clear();

	return 0;
}

btRigidBody* CBtObject::localCreateRigidBody( CRigidElem* curre, const btTransform& startTransform, btCollisionShape* shape )
{
	_ASSERT( shape );

	bool isDynamic = (curre->GetMass() != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia( curre->GetMass(), localInertia );

	btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
		
	btRigidBody::btRigidBodyConstructionInfo rbInfo( curre->GetMass(), myMotionState, shape, localInertia );
	btRigidBody* body = new btRigidBody( rbInfo );

	body->setRestitution( curre->GetRestitution() );
	body->setFriction( curre->GetFriction() );

	int myid = curre->GetGroupid();
	int coliid = curre->GetColiID();
	m_btWorld->addRigidBody(body, myid, coliid);

	return body;
}
int CBtObject::AddChild( CBtObject* addbt )
{
	m_chilbt.push_back( addbt );
	return 0;
}

int CBtObject::CreateObject( CBtObject* parbt, CBone* parbone, CBone* curbone, CBone* chilbone )
{
	m_bone = curbone;
	m_parbone = parbone;
	m_parbt = parbt;
	m_endbone = chilbone;

	if( !m_bone ){
		return 0;
	}
	if( !m_endbone ){
		return 0;
	}

	CRigidElem* curre = m_bone->GetRigidElem( chilbone );
	if( !curre ){
		_ASSERT( 0 );
		return 1;
	}

	if( curre && (curre->GetSkipflag() == 1) ){
		return 0;
	}

	D3DXVECTOR3 centerA, parposA, chilposA, aftparposA, aftchilposA;
	parposA = m_bone->GetJointFPos();
	D3DXVec3TransformCoord(&aftparposA, &parposA, &m_bone->GetStartMat2());
	chilposA = m_endbone->GetJointFPos();
	D3DXVec3TransformCoord(&aftchilposA, &chilposA, &m_endbone->GetStartMat2());
	D3DXVECTOR3 diffA = chilposA - parposA;
	m_boneleng = D3DXVec3Length(&diffA);

	float h, r, z;
	//max : boneleng 0 対策
	r = max(0.0001f, curre->GetSphr());// * 0.95f;
	h = max(0.0001f, curre->GetCylileng());// *0.70f;//!!!!!!!!!!!!!
	z = max(0.0001f, curre->GetBoxz());

	if( curre->GetColtype() == COL_CAPSULE_INDEX ){
		m_colshape = new btCapsuleShape( btScalar( r ), btScalar( h ) );
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_CONE_INDEX ){
		m_colshape = new btConeShape( btScalar( r ), btScalar( h ) );
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_SPHERE_INDEX ){
		m_colshape = new btSphereShape( btScalar( r ) );
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_BOX_INDEX ){
		m_colshape = new btBoxShape( btVector3( r, h, z ) );
		_ASSERT( m_colshape );
	}else{
		_ASSERT( 0 );
		return 1;
	}


//	if( m_boneleng < 0.00001f ){
//		_ASSERT( 0 );
//	}


	D3DXMATRIX startrot = curre->GetCapsulemat();
	//m_transmat = startrot;


	CQuaternion startrotq;
	startrotq.RotationMatrix(startrot);

	btScalar qx = startrotq.x;
	btScalar qy = startrotq.y;
	btScalar qz = startrotq.z;
	btScalar qw = startrotq.w;
	btQuaternion btq( qx, qy, qz, qw ); 


	centerA = ( aftparposA + aftchilposA ) * 0.5f;
	btVector3 btv( btScalar( centerA.x ), btScalar( centerA.y ), btScalar( centerA.z ) );

	btTransform transform;
	transform.setIdentity();
	transform.setRotation( btq );
	transform.setOrigin( btv );


	//-0.374995, 0.249996, 0.000000
	D3DXMatrixIdentity( &m_cen2parY );
	m_cen2parY._41 = 0.0f;
	m_cen2parY._42 = -m_boneleng * 0.5f;
	m_cen2parY._43 = 0.0f;
//	m_cen2parY._41 = 0.0f - -0.374995f;
//	m_cen2parY._42 = -m_boneleng * 0.5f - 0.249996f;
//	m_cen2parY._43 = 0.0f;


	D3DXVECTOR3 partocen = centerA - aftparposA;
	D3DXMatrixIdentity( &m_par2cen );
	m_par2cen._41 = partocen.x;
	m_par2cen._42 = partocen.y;
	m_par2cen._43 = partocen.z;

	m_transmat = startrot;
	m_transmat._41 = centerA.x;
	m_transmat._42 = centerA.y;
	m_transmat._43 = centerA.z;

//m_transmat = curre->m_capsulemat;


	m_rigidbody = localCreateRigidBody( curre, transform, m_colshape );
	_ASSERT( m_rigidbody );
	if (!m_rigidbody){
		_ASSERT(0);
		return 1;
	}

	btTransform worldtra;
	m_rigidbody->getMotionState()->getWorldTransform( worldtra );
	btMatrix3x3 worldmat = worldtra.getBasis();
	btVector3 worldpos = worldtra.getOrigin();
	btVector3 tmpcol[3];
	int colno;
	for( colno = 0; colno < 3; colno++ ){
		tmpcol[colno] = worldmat.getColumn( colno );
//		tmpcol[colno] = worldmat.getRow( colno );
	}

	D3DXMatrixIdentity( &m_xworld );

	m_xworld._11 = tmpcol[0].x();
	m_xworld._12 = tmpcol[0].y();
	m_xworld._13 = tmpcol[0].z();

	m_xworld._21 = tmpcol[1].x();
	m_xworld._22 = tmpcol[1].y();
	m_xworld._23 = tmpcol[1].z();

	m_xworld._31 = tmpcol[2].x();
	m_xworld._32 = tmpcol[2].y();
	m_xworld._33 = tmpcol[2].z();

	m_xworld._41 = worldpos.x();
	m_xworld._42 = worldpos.y();
	m_xworld._43 = worldpos.z();


	return 0;
}

int CBtObject::CalcConstraintTransform( int chilflag, CRigidElem* curre, CBtObject* curbto, btTransform& dsttra )
{
	dsttra.setIdentity();

	if (!m_rigidbody){
		return 1;
	}

	D3DXVECTOR3 parposA, chilposA, aftparposA, aftchilposA;
	parposA = curbto->m_bone->GetJointFPos();
	D3DXVec3TransformCoord( &aftparposA, &parposA, &curbto->m_bone->GetStartMat2() );
	chilposA = curbto->m_endbone->GetJointFPos();
	D3DXVec3TransformCoord( &aftchilposA, &chilposA, &curbto->m_endbone->GetStartMat2() );

	D3DXVECTOR2 dirxy, ndirxy;
	dirxy.x = aftchilposA.x - aftparposA.x;
	dirxy.y = aftchilposA.y - aftparposA.y;
	float lengxy = D3DXVec2Length( &dirxy );
	D3DXVec2Normalize( &ndirxy, &dirxy );

/***
	D3DXMATRIX startrot = curre->m_capsulemat;
	startrot._41 = 0.0f;
	startrot._42 = 0.0f;
	startrot._43 = 0.0f;
	D3DXQUATERNION xq;
	D3DXQuaternionRotationMatrix( &xq, &startrot );
	CQuaternion cq;
	cq.SetParams( xq );

	D3DXVECTOR3 eul;
	cq.Q2EulBt( &eul );
	dsttra.getBasis().setEulerZYX( eul.x, eul.y, eul.z );
***/

	D3DXVECTOR2 basex( 1.0f, 0.0f );
	float dotx;
	dotx = D3DXVec2Dot( &basex, &ndirxy );
	if( dotx > 1.0f ){
		dotx = 1.0f;
	}
	if( dotx < -1.0f ){
		dotx = -1.0f;
	}

	float calcacos = (float)acos( dotx );
	if( fabs( calcacos ) <= 0.1f * (float)DEG2PAI ){
		m_constzrad = -90.0f * (float)DEG2PAI;
	}else{
		if( dirxy.y >= 0.0f ){
			m_constzrad = -calcacos;
		}else{
			m_constzrad = calcacos;
		}
	}

	if( lengxy < 0.2f ){
		//_ASSERT( 0 );
		m_constzrad = -90.0f * (float)DEG2PAI;
	}

/***
	if( m_boneleng < 0.0001f ){
		CBtObject* parbt = curbto->m_parbt;
		if( parbt ){
			m_constzrad = parbt->m_constzrad;
		}else{
			m_constzrad = -90.0f * (float)DEG2PAI;
		}
	}
***/
/***
	float diffzrad = 0.0f;
	CBtObject* parbt = curbto->m_parbt;
	if( parbt ){
		diffzrad = fabs( rad - parbt->m_constzrad );
	}
	if( diffzrad >= 176.0f * (float)DEG2PAI ){
		_ASSERT( parbt );
		m_constzrad = parbt->m_constzrad;	
	}else{
		m_constzrad = rad;
	}
***/

	dsttra.getBasis().setEulerZYX( 0.0f, 0.0f, m_constzrad );

	btTransform rigidtra = curbto->m_rigidbody->getWorldTransform();
	btTransform invtra = rigidtra.inverse();
	//btVector3 localpivot;
	if( chilflag == 0 ){
		m_curpivot = invtra( btVector3( aftchilposA.x, aftchilposA.y, aftchilposA.z ) );
		//m_curpivot = btVector3( 0.0f, 0.5f * curbto->m_boneleng, 0.0f );
	}else{
		m_curpivot = invtra( btVector3( aftparposA.x, aftparposA.y, aftparposA.z ) );
		//m_curpivot = btVector3( 0.0f, -0.5f * curbto->m_boneleng, 0.0f );
	}
	dsttra.setOrigin( m_curpivot );

	return 0;
}

int CBtObject::CreateBtConstraint()
{
	if( m_topflag == 1 ){
		return 0;
	}
	_ASSERT( m_btWorld );
	_ASSERT( m_bone );

	if( !m_endbone ){
		return 1;
	}
	if (!m_rigidbody){
		return 1;
	}

	int chilno;
	for (chilno = 0; chilno < (int)m_chilbt.size(); chilno++){
		CBtObject* chilbto = m_chilbt[ chilno ];
		if( !chilbto ){
			continue;
		}

		m_FrameA.setIdentity();
		m_FrameB.setIdentity();

		CRigidElem* tmpre;
		tmpre = m_bone->GetRigidElem( m_endbone );
		_ASSERT( tmpre );
		CalcConstraintTransform( 0, tmpre, this, m_FrameA );
		tmpre = chilbto->m_bone->GetRigidElem( chilbto->m_endbone );
		_ASSERT( tmpre );
		CalcConstraintTransform( 1, tmpre, chilbto, m_FrameB );

		if( m_rigidbody && chilbto->m_rigidbody ){

DbgOut( L"CreateBtConstraint (bef) : curbto %s---%s, chilbto %s---%s\r\n", 
	   m_bone->GetWBoneName(), m_endbone->GetWBoneName(),
	   chilbto->m_bone->GetWBoneName(), chilbto->m_endbone->GetWBoneName() );

			float angPAI2, angPAI;
			angPAI2 = 90.0f * (float)DEG2PAI;
			angPAI = 180.0f * (float)DEG2PAI;

			float lmax, lmin;
			lmin = -10000.0f;
			lmax = 10000.0f;

			btGeneric6DofSpringConstraint* dofC = 0;
			dofC = new btGeneric6DofSpringConstraint( *m_rigidbody, *(chilbto->m_rigidbody), m_FrameA, m_FrameB, true );
			_ASSERT( dofC );
			if (dofC){
				btTransform worldtra;
				m_rigidbody->getMotionState()->getWorldTransform(worldtra);
				btMatrix3x3 worldmat = worldtra.getBasis();
				btVector3 worldpos = worldtra.getOrigin();

				dofC->setLinearLowerLimit(btVector3(worldpos.x() + lmin, worldpos.y() + lmin, worldpos.z() + lmin));
				dofC->setLinearUpperLimit(btVector3(worldpos.x() + lmax, worldpos.y() + lmax, worldpos.z() + lmax));




				dofC->setBreakingImpulseThreshold(FLT_MAX);
				//dofC->setBreakingImpulseThreshold( 1e9 );
				//dofC->setBreakingImpulseThreshold(0.0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//dofC->setBreakingImpulseThreshold(1e7);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

				int l_kindex = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetLKindex();
				int a_kindex = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetAKindex();
				float l_damping = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetLDamping();
				float a_damping = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetADamping();
				float l_cusk = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetCusLk();
				float a_cusk = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetCusAk();
				int forbidrotflag = chilbto->m_bone->GetRigidElem(chilbto->m_endbone)->GetForbidRotFlag();

				if (forbidrotflag == 0){
					dofC->setAngularLowerLimit(btVector3(angPAI, angPAI2, angPAI));
					dofC->setAngularUpperLimit(btVector3(-angPAI, -angPAI2, -angPAI));
				} else{
					dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
					dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
				}

				int dofid;
				/*
				for (dofid = 0; dofid < 3; dofid++){
					dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
				}
				for (dofid = 3; dofid < 6; dofid++){
					dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
				}
				*/
				for (dofid = 0; dofid < 3; dofid++){
					if( l_kindex <= 2 ){
						dofC->setStiffness( dofid, g_l_kval[ l_kindex ] );
					}else{
						dofC->setStiffness( dofid, l_cusk );
					}
					//dofC->setStiffness(dofid, 1.0e12);
					dofC->setDamping(dofid, l_damping);
					dofC->setEquilibriumPoint(dofid);

					//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
					dofC->enableSpring(dofid, true);
				}
				for (dofid = 3; dofid < 6; dofid++){
					if (a_kindex <= 2){
						dofC->setStiffness(dofid, g_a_kval[a_kindex]);
					}
					else{
						dofC->setStiffness(dofid, a_cusk);
					}
					dofC->setDamping(dofid, a_damping);
					dofC->setEquilibriumPoint(dofid);

					dofC->enableSpring(dofid, true);
					//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
				}

				CONSTRAINTELEM addelem;
				addelem.constraint = dofC;
				addelem.centerbone = m_endbone;
				addelem.childbto = chilbto;
				m_constraint.push_back(addelem);

				DbgOut(L"CreateBtConstraint (aft) : curbto %s---%s, chilbto %s---%s\r\n",
					m_bone->GetWBoneName(), m_endbone->GetWBoneName(),
					chilbto->m_bone->GetWBoneName(), chilbto->m_endbone->GetWBoneName());

				//m_btWorld->addConstraint(dofC, true);
				//m_btWorld->addConstraint((btTypedConstraint*)dofC, false);//!!!!!!!!!!!! disable collision between linked bodies
				m_btWorld->addConstraint((btTypedConstraint*)dofC, true);//!!!!!!!!!!!! disable collision between linked bodies
				//m_dofC = dofC;

				dofC->setEquilibriumPoint();//!!!!!!!!!!!!tmp disable
			}
		}
	}

	return 0;
}

int CBtObject::EnableSpring(bool angleflag, bool linearflag)
{
	int constraintnum = m_constraint.size();
	int constno;
	for (constno = 0; constno < constraintnum; constno++){
		btGeneric6DofSpringConstraint* curconst = m_constraint[constno].constraint;
		if (curconst){
			int dofid;
			for (dofid = 0; dofid < 3; dofid++){
				curconst->enableSpring(dofid, linearflag);
			}
			for (dofid = 3; dofid < 6; dofid++){
				curconst->enableSpring(dofid, angleflag);
			}
		}
	}

	return 0;
}


int CBtObject::SetEquilibriumPoint( int lflag, int aflag )
{
	int chilno;
	for (chilno = 0; chilno < (int)m_constraint.size(); chilno++){
		btGeneric6DofSpringConstraint* constptr = m_constraint[chilno].constraint;
		CBone* centerbone = m_constraint[chilno].centerbone;
		if (constptr && centerbone){
			D3DXVECTOR3 centerpos = centerbone->GetBtParPos();

			DbgOut(L"checkbt !!! : btobject : SetEquilibriumPoint : chilno %d, centerbone %s, centerpos (%.4f, %.4f, %.4f)\r\n",
				chilno, centerbone->GetWBoneName(), centerpos.x, centerpos.y, centerpos.z);

			//float lmin = -0.15f;
			//float lmax = 0.15f;
			float lmin = -100.0f;//
			float lmax = 100.0f;
			//float lmin = -10.0f;//少し　移動すると　破たん
			//float lmax = 10.0f;


			constptr->setLinearLowerLimit(btVector3(centerpos.x + lmin, centerpos.y + lmin, centerpos.z + lmin));
			constptr->setLinearUpperLimit(btVector3(centerpos.x + lmax, centerpos.y + lmax, centerpos.z + lmax));

			int dofid;
			if (lflag == 1){
				for (dofid = 0; dofid < 3; dofid++){//角度のみ
					constptr->setEquilibriumPoint(dofid);
				}
			}
			if (aflag == 1){
				for (dofid = 3; dofid < 6; dofid++){//角度のみ
					constptr->setEquilibriumPoint(dofid);
				}
			}
		}
	}
	return 0;
}

int CBtObject::Motion2Bt()
{
	if( m_topflag == 1 ){
		return 0;
	}
	if( !m_bone ){
		return 0;
	}
	if( !m_rigidbody ){
		return 0;
	}

	if( !m_rigidbody->getMotionState() ){
		_ASSERT( 0 );
		return 0;
	}
	if (!GetBone() || !GetEndBone()){
		return 0;
	}


	CRigidElem* curre = m_bone->GetRigidElem( m_endbone );
	if( curre ){
		D3DXMATRIX newrotmat;
		D3DXVECTOR3 newrigidpos;
		GetBone()->CalcNewBtMat(curre, GetEndBone(), &newrotmat, &newrigidpos);
		/*
		D3DXMATRIX invfirstworld;
		D3DXMatrixInverse( &invfirstworld, NULL, &m_bone->GetStartMat2() );
		D3DXMATRIX diffworld;
		diffworld = invfirstworld * m_bone->GetCurMp().GetWorldMat();

		D3DXMATRIX multmat = curre->GetFirstcapsulemat() * diffworld;

		D3DXVECTOR3 rigidcenter;
		D3DXVECTOR3 aftcurpos, aftchilpos;
		D3DXVec3TransformCoord( &aftcurpos, &m_bone->GetJointFPos(), &(m_bone->GetCurMp().GetWorldMat()) );
		D3DXVec3TransformCoord( &aftchilpos, &m_endbone->GetJointFPos(), &(m_endbone->GetCurMp().GetWorldMat()) );
		rigidcenter = ( aftcurpos + aftchilpos ) * 0.5f;
		

		D3DXMATRIX newcapsulemat;
		newcapsulemat = newrotmat;
		newcapsulemat._41 = 0.0f;
		newcapsulemat._42 = 0.0f;
		newcapsulemat._43 = 0.0f;
		*/

		CQuaternion tmpq;
		tmpq.RotationMatrix(newrotmat);
		btQuaternion btrotq( tmpq.x, tmpq.y, tmpq.z, tmpq.w );

		btTransform worldtra;
		worldtra.setIdentity();
		worldtra.setRotation( btrotq );
		worldtra.setOrigin(btVector3(newrigidpos.x, newrigidpos.y, newrigidpos.z));

		m_rigidbody->getMotionState()->setWorldTransform( worldtra );
	}else{
		_ASSERT( 0 );
	}

	return 0;
}

int CBtObject::SetBtMotion()
{
	if( m_topflag == 1 ){
		return 0;
	}
	if( !m_rigidbody ){
		return 0;
	}
	if( !m_rigidbody->getMotionState() ){
		_ASSERT( 0 );
		return 0;
	}

	btTransform worldtra;
	m_rigidbody->getMotionState()->getWorldTransform( worldtra );
	btMatrix3x3 worldmat = worldtra.getBasis();
	btVector3 worldpos = worldtra.getOrigin();
	btVector3 tmpcol[3];//行列のカラム表現。
	int colno;
	for( colno = 0; colno < 3; colno++ ){
		tmpcol[colno] = worldmat.getColumn( colno );
	}

	D3DXMATRIX newxworld;
	D3DXMatrixIdentity( &newxworld );

	newxworld._11 = tmpcol[0].x();
	newxworld._12 = tmpcol[0].y();
	newxworld._13 = tmpcol[0].z();

	newxworld._21 = tmpcol[1].x();
	newxworld._22 = tmpcol[1].y();
	newxworld._23 = tmpcol[1].z();

	newxworld._31 = tmpcol[2].x();
	newxworld._32 = tmpcol[2].y();
	newxworld._33 = tmpcol[2].z();

	newxworld._41 = worldpos.x();
	newxworld._42 = worldpos.y();
	newxworld._43 = worldpos.z();

	D3DXMATRIX invxworld;
	D3DXMatrixInverse( &invxworld, NULL, &m_xworld );

	D3DXMATRIX diffxworld;
	diffxworld = invxworld * newxworld;

	CMotionPoint curmp;
	curmp = m_bone->GetCurMp();
	curmp.SetBtMat(m_bone->GetStartMat2() * diffxworld);
	curmp.SetBtFlag(1);
	m_bone->SetCurMp(curmp);


	/*
	
	//boneleng 0 対策はCreateObjectの剛体のサイズを決めるところで最小値を設定することにしたので、以下はコメントアウト。

	if( m_boneleng > 0.00001f ){
		//ボーンの親と子供が同位置ではない場合。
		//剛体シミュレーション開始時の行列に、開始時からの変化分を掛ける。
		curmp = m_bone->GetCurMp();
		curmp.SetBtMat( m_bone->GetStartMat2() * diffxworld );
		m_bone->SetCurMp( curmp );
	}else{
		//ボーンの親と子供が同位置の場合。
		if( m_parbt->m_bone ){
			//親側ボーンの親ボーンがある場合。
			//剛体シミュレーション開始時の行列に、親ボーンの変化分を掛ける。
			D3DXMATRIX invstart;
			D3DXMatrixInverse( &invstart, NULL, &(m_parbt->m_bone->GetStartMat2()) );
			D3DXMATRIX diffmat;
			diffmat = invstart * m_parbt->m_bone->GetCurMp().GetBtMat();
			curmp = m_bone->GetCurMp();
			curmp.SetBtMat( m_bone->GetStartMat2() * diffmat );
			m_bone->SetCurMp( curmp );
		}else{
			//剛体シミュレーション行列のままとする。
			curmp = m_bone->GetCurMp();
			curmp.SetBtMat( m_bone->GetStartMat2() );
			m_bone->SetCurMp( curmp );
		}
	}

	curmp = m_bone->GetCurMp();
	curmp.SetBtFlag( 1 );
	m_bone->SetCurMp( curmp );
	*/

	return 0;
}

