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


#include <GlobalVar.h>

#include <BtObject.h>

#include <Model.h>
#include <Bone.h>
//#include <quaternion.h>
#include <RigidElem.h>


//#include <BoneProp.h>

#define DBGH
#include <dbg.h>

using namespace std;
/*
extern float g_miscale;
extern float g_l_kval[3];
extern float g_a_kval[3];
extern int g_previewFlag;			// プレビューフラグ
*/
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
	ChaMatrixIdentity(&m_firstTransformMatX);
	m_firstTransform.setIdentity();

	m_btpos = ChaVector3(0.0f, 0.0f, 0.0f);

	m_connectflag = 0;
	m_constzrad = 0.0f;
	ChaMatrixIdentity( &m_transmat );
	ChaMatrixIdentity( &m_xworld );
	m_btWorld = 0;

	m_topflag = 0;
	m_parentbone = 0;
	m_endbone = 0;
	m_bone = 0;
	m_colshape = 0;
	m_rigidbody = 0;
	m_constraint.clear();

	m_gz_colshape = 0;
	m_gz_rigidbody = 0;
	ZeroMemory(&m_gz_constraint, sizeof(CONSTRAINTELEM));



	m_parbt = 0;
	m_chilbt.clear();

	m_curpivot.setValue( 0.0f, 0.0f, 0.0f );
	m_chilpivot.setValue( 0.0f, 0.0f, 0.0f );

	m_FrameA.setIdentity();
	m_FrameB.setIdentity();
	m_firstTransformMat.setIdentity();//bto->GetRigidBody()のCreateBtObject時のWorldTransform->getBasis


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

	DestroyPhysicsPosConstraint();


	m_chilbt.clear();

	return 0;
}

void CBtObject::DestroyGZObj()
{
	btGeneric6DofSpringConstraint* constptr = m_gz_constraint.constraint;
	if (constptr){
		m_btWorld->removeConstraint(constptr);
		delete constptr;
	}
	ZeroMemory(&m_gz_constraint, sizeof(CONSTRAINTELEM));

	if (m_gz_rigidbody){
		if (m_gz_rigidbody->getMotionState()){
			delete m_gz_rigidbody->getMotionState();
		}
		m_btWorld->removeRigidBody(m_gz_rigidbody);
		delete m_gz_rigidbody;
		m_gz_rigidbody = 0;
	}

	if (m_gz_colshape){
		delete m_gz_colshape;
		m_gz_colshape = 0;
	}
}


btRigidBody* CBtObject::localCreateRigidBody( CRigidElem* curre, const btTransform& startTransform, btCollisionShape* shape )
{
	_ASSERT( shape );

	btRigidBody* body = 0;

	if (curre){
		//bool isDynamic = (curre->GetMass() != 0.f);
		bool isDynamic = true;
		btVector3 localInertia(0, 0, 0);
		//btVector3 localInertia(0, -m_boneleng * 0.5f, 0);
		if (isDynamic)
			shape->calculateLocalInertia(curre->GetMass(), localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(curre->GetMass(), myMotionState, shape, localInertia);
		body = new btRigidBody(rbInfo);


		//body->setRestitution(curre->GetRestitution());
		//body->setFriction(curre->GetFriction());
		body->setRestitution(0.0);
		body->setFriction(1.0);
		//if (g_previewFlag != 5){
			body->setDamping(0.3, 0.6);
		//}else{
		//	body->setDamping(0.3, 1.0);
		//}


		int myid = curre->GetGroupid();
		//int coliid = curre->GetColiID();
		int coliid;
		if (g_previewFlag == 5){
			coliid = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		else{
			coliid = curre->GetColiID();
		}
		m_btWorld->addRigidBody(body, myid, coliid);
	}
	else{
		bool isDynamic = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		btScalar setmass = 0.0;

		btVector3 localInertia(0, 0, 0);
		//if (isDynamic)
			shape->calculateLocalInertia(setmass, localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(setmass, myMotionState, shape, localInertia);
		body = new btRigidBody(rbInfo);

		body->setRestitution(0.0);
		body->setFriction(1.0);
		//body->setDamping(0.3, 0.6);

		int myid = 999;
		int coliid = 0;
		m_btWorld->addRigidBody(body, myid, coliid);

	}
	return body;
}
int CBtObject::AddChild( CBtObject* addbt )
{
	m_chilbt.push_back( addbt );
	return 0;
}

int CBtObject::CreateObject( CBtObject* parbt, CBone* parentbone, CBone* curbone, CBone* childbone )
{
	m_bone = curbone;
	m_parentbone = parentbone;
	m_parbt = parbt;
	m_endbone = childbone;

	if( !m_bone ){
		return 0;
	}
	if( !m_endbone ){
		return 0;
	}

	m_bone->SetFirstCalcRigid(true);
	m_endbone->SetFirstCalcRigid(true);


	CRigidElem* curre = m_bone->GetRigidElem( childbone );
	if( !curre ){
		_ASSERT( 0 );
		return 1;
	}

	if( curre && (curre->GetSkipflag() == 1) ){
		return 0;
	}

	ChaVector3 centerA, parentposA, childposA, aftparentposA, aftchildposA;
	parentposA = m_bone->GetJointFPos();
	ChaVector3TransformCoord(&aftparentposA, &parentposA, &m_bone->GetCurrentZeroFrameMat());
	childposA = m_endbone->GetJointFPos();
	ChaVector3TransformCoord(&aftchildposA, &childposA, &m_endbone->GetCurrentZeroFrameMat());
	ChaVector3 diffA = childposA - parentposA;
	m_boneleng = ChaVector3Length(&diffA);

	float h, r, z;
	//max : boneleng 0 対策
	r = max(0.0001f, curre->GetSphr());// * 0.95f;
	h = max(0.0001f, curre->GetCylileng());// *0.70f;//!!!!!!!!!!!!!
	z = max(0.0001f, curre->GetBoxz());

	//double lengrate = 1.0;
	//double lengrate = 0.95;

	double lengrate;
	double rrate;

	if (g_previewFlag == 5){
		//lengrate = 0.90;
		//rrate = 0.30;
		//rrate = 0.1;

		lengrate = 0.90;
		rrate = 0.050;
	}
	else{
		lengrate = 0.90;
		rrate = 0.90;
	}


	if( curre->GetColtype() == COL_CAPSULE_INDEX ){
		m_colshape = new btCapsuleShape(btScalar(r * rrate), btScalar(h * lengrate));//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_CONE_INDEX ){
		m_colshape = new btConeShape(btScalar(r * rrate), btScalar(h * lengrate));
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_SPHERE_INDEX ){
		m_colshape = new btSphereShape(btScalar(r * rrate));
		_ASSERT( m_colshape );
	}else if( curre->GetColtype() == COL_BOX_INDEX ){
		m_colshape = new btBoxShape(btVector3(r * rrate, h * lengrate, z * rrate));
		_ASSERT( m_colshape );
	}else{
		_ASSERT( 0 );
		return 1;
	}


//	if( m_boneleng < 0.00001f ){
//		_ASSERT( 0 );
//	}


	ChaMatrix startrot = curre->GetCapsulemat(1);
	//ChaMatrix startrot = m_bone->CalcManipulatorPostureMatrix(0, 0, 1);

	//m_transmat = startrot;


	CQuaternion startrotq;
	startrotq.RotationMatrix(startrot);

	btScalar qx = startrotq.x;
	btScalar qy = startrotq.y;
	btScalar qz = startrotq.z;
	btScalar qw = startrotq.w;
	btQuaternion btq( qx, qy, qz, qw );


	centerA = ( aftparentposA + aftchildposA ) * 0.5f;
	//centerA = aftparentposA;
	btVector3 btv( btScalar( centerA.x ), btScalar( centerA.y ), btScalar( centerA.z ) );

	btTransform transform;
	transform.setIdentity();
	transform.setRotation( btq );
	transform.setOrigin( btv );


	//-0.374995, 0.249996, 0.000000
	ChaMatrixIdentity( &m_cen2parY );
	m_cen2parY._41 = 0.0f;
	//m_cen2parY._42 = -m_boneleng * 0.5f;
	m_cen2parY._42 = 0.0f;
	m_cen2parY._43 = 0.0f;
//	m_cen2parY._41 = 0.0f - -0.374995f;
//	m_cen2parY._42 = -m_boneleng * 0.5f - 0.249996f;
//	m_cen2parY._43 = 0.0f;


	ChaVector3 partocen = centerA - aftparentposA;
	ChaMatrixIdentity( &m_par2cen );
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
	
	m_firstTransform = worldtra;
	m_firstTransformMat = worldmat;//bto->GetRigidBody()のCreateBtObject時のWorldTransform->getBasis
	SetFirstTransformMatX(ChaMatrixFromBtTransform(&worldtra.getBasis(), &worldtra.getOrigin()));

	btVector3 tmpcol[3];
	int colno;
	for( colno = 0; colno < 3; colno++ ){
		tmpcol[colno] = worldmat.getColumn( colno );
//		tmpcol[colno] = worldmat.getRow( colno );
	}

	ChaMatrixIdentity( &m_xworld );

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




/*
int CBtObject::CalcConstraintTransform( int chilflag, CRigidElem* curre, CBtObject* curbto, btTransform& dsttra )
{
	dsttra.setIdentity();

	if (!m_rigidbody){
		return 1;
	}

	ChaVector3 parentposA, childposA, aftparentposA, aftchildposA;
	parentposA = curbto->m_bone->GetJointFPos();
	ChaVector3TransformCoord( &aftparentposA, &parentposA, &curbto->m_bone->GetStartMat2() );
	childposA = curbto->m_endbone->GetJointFPos();
	ChaVector3TransformCoord( &aftchildposA, &childposA, &curbto->m_endbone->GetStartMat2() );

	ChaVector2 dirxy, ndirxy;
	dirxy.x = aftchildposA.x - aftparentposA.x;
	dirxy.y = aftchildposA.y - aftparentposA.y;
	float lengxy = D3DXVec2Length( &dirxy );
	D3DXVec2Normalize( &ndirxy, &dirxy );


	ChaVector2 basex( 1.0f, 0.0f );
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


	dsttra.getBasis().setEulerZYX(0.0f, 0.0f, m_constzrad);
	//dsttra.getBasis().setEulerZYX(m_constzrad, 0.0f, 0.0f);
	//dsttra.getBasis().setEulerZYX(0.0, 0.0f, 0.0f);

	btTransform rigidtra = curbto->m_rigidbody->getWorldTransform();
	btTransform invtra = rigidtra.inverse();
	//btVector3 localpivot;
	if( chilflag == 0 ){
		m_curpivot = invtra( btVector3( aftchildposA.x, aftchildposA.y, aftchildposA.z ) );
		//m_curpivot = btVector3( 0.0f, 0.5f * curbto->m_boneleng, 0.0f );
	}else{
		m_curpivot = invtra( btVector3( aftparentposA.x, aftparentposA.y, aftparentposA.z ) );
		//m_curpivot = btVector3( 0.0f, -0.5f * curbto->m_boneleng, 0.0f );
	}
	dsttra.setOrigin( m_curpivot );

	return 0;
}
*/

int CBtObject::CalcConstraintTransform(int chilflag, CRigidElem* curre, CBtObject* curbto, btTransform& dsttra)
{
	dsttra.setIdentity();

	if (!m_rigidbody){
		return 1;
	}

	ChaMatrix transmatx;
	ChaMatrixIdentity(&transmatx);
	int setstartflag = 1;

	curbto->m_bone->CalcAxisMatX(0, curbto->m_endbone, &transmatx, setstartflag);

	CQuaternion rotq;
	rotq.RotationMatrix(transmatx);
	CQuaternion invrotq;
	rotq.inv(&invrotq);

	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 eul = ChaVector3(0.0f, 0.0f, 0.0f);
	invrotq.Q2EulXYZ(0, befeul, &eul);
	//rotq.Q2EulZYX(0, 0, befeul, &eul);

	dsttra.getBasis().setEulerZYX(eul.x * PAI / 180.0, eul.y * PAI / 180.0, eul.z * PAI / 180.0);

	btTransform rigidtra = curbto->m_rigidbody->getWorldTransform();
	btTransform invtra = rigidtra.inverse();

	ChaVector3 parentposA, childposA, aftparentposA, aftchildposA;
	parentposA = curbto->m_bone->GetJointFPos();
	ChaVector3TransformCoord(&aftparentposA, &parentposA, &curbto->m_bone->GetCurrentZeroFrameMat());
	childposA = curbto->m_endbone->GetJointFPos();
	ChaVector3TransformCoord(&aftchildposA, &childposA, &curbto->m_endbone->GetCurrentZeroFrameMat());
	if (chilflag == 0){
		m_curpivot = invtra(btVector3(aftchildposA.x, aftchildposA.y, aftchildposA.z));
		//m_curpivot = btVector3( 0.0f, 0.5f * curbto->m_boneleng, 0.0f );
	}
	else{
		m_curpivot = invtra(btVector3(aftparentposA.x, aftparentposA.y, aftparentposA.z));
		//m_curpivot = btVector3( 0.0f, -0.5f * curbto->m_boneleng, 0.0f );
	}


	dsttra.setOrigin(m_curpivot);

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
		//CalcConstraintTransform(1, tmpre, chilbto, m_FrameB);

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
			//dofC = new btConeTwistConstraint( *m_rigidbody, *(chilbto->m_rigidbody), m_FrameA, m_FrameB, true );
			dofC = new btGeneric6DofSpringConstraint(*m_rigidbody, *(chilbto->m_rigidbody), m_FrameA, m_FrameB, true);
			_ASSERT( dofC );
			if (dofC){
				btTransform worldtra;
				m_rigidbody->getMotionState()->getWorldTransform(worldtra);
				btMatrix3x3 worldmat = worldtra.getBasis();
				btVector3 worldpos = worldtra.getOrigin();

				//##dofC->setLinearLowerLimit(btVector3(worldpos.x() + lmin, worldpos.y() + lmin, worldpos.z() + lmin));
				//##dofC->setLinearUpperLimit(btVector3(worldpos.x() + lmax, worldpos.y() + lmax, worldpos.z() + lmax));

				//dofC->setBreakingImpulseThreshold(FLT_MAX);
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
				ANGLELIMIT anglelimit = chilbto->m_bone->GetAngleLimit();


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
					//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
					dofC->enableSpring(dofid, true);

					//dofC->setEquilibriumPoint(dofid);
				}
				for (dofid = 3; dofid < 6; dofid++){
					if (a_kindex <= 2){
						dofC->setStiffness(dofid, g_a_kval[a_kindex]);
					}
					else{
						dofC->setStiffness(dofid, a_cusk);
					}
					dofC->setDamping(dofid, a_damping);
					//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
					dofC->enableSpring(dofid, true);

					//dofC->setEquilibriumPoint(dofid);
				}				

				//dofC->setDamping(a_damping);

				//dofC->setDamping(1.0);


				CONSTRAINTELEM addelem;
				addelem.constraint = dofC;
				addelem.centerbone = m_endbone;
				addelem.childbto = chilbto;
				m_constraint.push_back(addelem);

				DbgOut(L"CreateBtConstraint (aft) : curbto %s---%s, chilbto %s---%s\r\n",
					m_bone->GetWBoneName(), m_endbone->GetWBoneName(),
					chilbto->m_bone->GetWBoneName(), chilbto->m_endbone->GetWBoneName());


				dofC->setEquilibriumPoint();
				//tmpre = m_bone->GetRigidElem(m_endbone);
				////ChaMatrix axismatrix = m_endbone->GetStartMat2();
				//ChaMatrix axismatrix = tmpre->GetFirstcapsulemat();
				//btVector3 axisZ;
				//btVector3 axisX;
				//axisZ = btVector3(axismatrix._13, axismatrix._23, axismatrix._33);
				//axisX = btVector3(axismatrix._11, axismatrix._21, axismatrix._31);
				////axisZ = btVector3(axismatrix._31, axismatrix._32, axismatrix._33);
				////axisX = btVector3(axismatrix._11, axismatrix._12, axismatrix._13);
				//dofC->setAxis(axisZ, axisX);


				//m_btWorld->addConstraint(dofC, false);
				m_btWorld->addConstraint(dofC, true);
				//m_btWorld->addConstraint((btTypedConstraint*)dofC, false);//!!!!!!!!!!!! disable collision between linked bodies
				//m_btWorld->addConstraint((btTypedConstraint*)dofC, true);//!!!!!!!!!!!! disable collision between linked bodies
				//m_dofC = dofC;

				//dofC->setEquilibriumPoint();//!!!!!!!!!!!!tmp disable


				if (forbidrotflag == 0) {
					dofC->setAngularLowerLimit(btVector3(angPAI, angPAI2, angPAI));
					dofC->setAngularUpperLimit(btVector3(-angPAI, -angPAI2, -angPAI));
				}
				else {
					//dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
					//dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
					dofC->calculateTransforms();
					btScalar currentx = dofC->getAngle(0);
					btScalar currenty = dofC->getAngle(1);
					btScalar currentz = dofC->getAngle(2);
					dofC->setAngularLowerLimit(btVector3(currentx - 0.5 * (float)DEG2PAI, currenty - 0.5 * (float)DEG2PAI, currentz - 0.5 * (float)DEG2PAI));
					dofC->setAngularUpperLimit(btVector3(currentx + 0.5 * (float)DEG2PAI, currenty + 0.5 * (float)DEG2PAI, currentz + 0.5 * (float)DEG2PAI));

				}

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

int CBtObject::SetDofRotAxis(int srcaxiskind)
{


	/*
	float angPAI4, angPAI2, angPAI, ang5;
	angPAI4 = 45.0f * (float)DEG2PAI;
	angPAI2 = 90.0f * (float)DEG2PAI;
	angPAI = 180.0f * (float)DEG2PAI;
	ang5 = 5.0f * (float)DEG2PAI;

	int constraintnum = m_constraint.size();
	int constno;
	for (constno = 0; constno < constraintnum; constno++){
		btConeTwistConstraint* dofC = m_constraint[constno].constraint;
		if (dofC){
			if (srcaxiskind == PICK_CENTER){
				dofC->setLimit(angPAI, angPAI, angPAI2);
			}
			else if (srcaxiskind == PICK_X){
				dofC->setLimit(angPAI, 0.0, 0.0);
			}
			else if (srcaxiskind == PICK_Y){
				dofC->setLimit(0.0, 0.0, angPAI2);
			}
			else if (srcaxiskind == PICK_Z){
				dofC->setLimit(0.0, angPAI, 0.0);

			}
		}
	}

	//btConeTwistConstraint* gzdofC = m_gz_constraint.constraint;
	//if (gzdofC){
		//if (srcaxiskind == PICK_CENTER){
		//gzdofC->setLimit(angPAI, angPAI, angPAI);
		//}
		//else if (srcaxiskind == PICK_X){
		//	gzdofC->setLimit(angPAI, 0.0, 0.0);
		//}
		//else if (srcaxiskind == PICK_Y){
		//	gzdofC->setLimit(0.0, 0.0, angPAI2);
		//}
		//else if (srcaxiskind == PICK_Z){
		//	gzdofC->setLimit(0.0, angPAI, 0.0);
		//}
	//}
	*/

	return 0;
}


int CBtObject::SetEquilibriumPoint(int lflag, int aflag)
{

	int constraintnum = m_constraint.size();
	int constno;
	for (constno = 0; constno < constraintnum; constno++){
		btGeneric6DofSpringConstraint* dofC = m_constraint[constno].constraint;
		dofC->setEquilibriumPoint();
	}


	/*
	float angPAI = 180.0f * (float)DEG2PAI;
	float angPAI2 = 90.0f * (float)DEG2PAI;
	float angPAI4 = 45.0f * (float)DEG2PAI;
	float ang5 = 5.0f * (float)DEG2PAI;

	int constraintnum = m_constraint.size();
	int constno;
	for (constno = 0; constno < constraintnum; constno++){
		btConeTwistConstraint* dofC = m_constraint[constno].constraint;
		if (dofC){
			dofC->setLimit(angPAI, angPAI, angPAI);
		}
	}
	*/


	float angPAI4, angPAI2, angPAI, ang5;
	angPAI4 = 45.0f * (float)DEG2PAI;
	angPAI2 = 90.0f * (float)DEG2PAI;
	angPAI = 180.0f * (float)DEG2PAI;
	ang5 = 5.0f * (float)DEG2PAI;

	////int constraintnum = m_constraint.size();
	////int constno;
	//for (constno = 0; constno < constraintnum; constno++){
	//	btGeneric6DofSpringConstraint* dofC = m_constraint[constno].constraint;
	//	if (dofC){
	//		//dofC->setLimit(angPAI, angPAI, angPAI, 0.8, 0.0, 0.5);
	//		//dofC->setLimit(angPAI, angPAI, angPAI2);
	//		//dofC->setLimit(angPAI2, angPAI2, angPAI2);
	//		//dofC->setLimit(angPAI2, angPAI2, angPAI4, 0.8, 0.0, 0.5);
	//		//dofC->setLimit(angPAI2, angPAI2, angPAI4, 0.8, 0.3, 0.8);
	//		//dofC->setLimit(angPAI2, angPAI2, angPAI4);
	//		if (g_previewFlag != 5){
	//			dofC->setLimit(angPAI4, angPAI4, angPAI4);
	//		}
	//		else{
	//			dofC->setLimit(angPAI, angPAI, angPAI);
	//		}
	//	}
	//}

	for (constno = 0; constno < constraintnum; constno++){
		btGeneric6DofSpringConstraint* dofC = m_constraint[constno].constraint;
		CBtObject* childbto = m_constraint[constno].childbto;
		if (childbto && childbto->m_bone){
			ANGLELIMIT anglelimit = childbto->m_bone->GetAngleLimit();
			int forbidrotflag = childbto->m_bone->GetRigidElem(childbto->m_endbone)->GetForbidRotFlag();

			//if (forbidrotflag == 0){
			//	dofC->setAngularLowerLimit(btVector3(angPAI, angPAI2, angPAI));
			//	dofC->setAngularUpperLimit(btVector3(-angPAI, -angPAI2, -angPAI));
			//}
			//else{
			//	dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
			//	dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
			//}

			if (forbidrotflag == 0){
				//XYZ
				dofC->setAngularLowerLimit(btVector3(anglelimit.lower[0] * PAI / 180.0f, anglelimit.lower[1] * PAI / 180.0f, anglelimit.lower[2] * PAI / 180.0f));
				dofC->setAngularUpperLimit(btVector3(anglelimit.upper[0] * PAI / 180.0f, anglelimit.upper[1] * PAI / 180.0f, anglelimit.upper[2] * PAI / 180.0f));
			}
			else{
				dofC->calculateTransforms();
				btScalar currentx = dofC->getAngle(0);
				btScalar currenty = dofC->getAngle(1);
				btScalar currentz = dofC->getAngle(2);
				dofC->setAngularLowerLimit(btVector3(currentx - 1.0 * (float)DEG2PAI, currenty - 1.0 * (float)DEG2PAI, currentz - 1.0 * (float)DEG2PAI));
				dofC->setAngularUpperLimit(btVector3(currentx + 1.0 * (float)DEG2PAI, currenty + 1.0 * (float)DEG2PAI, currentz + 1.0 * (float)DEG2PAI));

				//dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
				//dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
			}
		}
	}







	return 0;
}

int CBtObject::Motion2Bt(CModel* srcmodel)
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
		ChaMatrix newrotmat;
		ChaVector3 newrigidpos;
		GetBone()->CalcNewBtMat(srcmodel, curre, GetEndBone(), &newrotmat, &newrigidpos);

		CQuaternion tmpq;
		tmpq.RotationMatrix(newrotmat);
		btQuaternion btrotq( tmpq.x, tmpq.y, tmpq.z, tmpq.w );

		btTransform worldtra;
		worldtra.setIdentity();
		worldtra.setRotation(btrotq);
		worldtra.setOrigin(btVector3(newrigidpos.x, newrigidpos.y, newrigidpos.z));

		m_rigidbody->getMotionState()->setWorldTransform( worldtra );

		m_btpos = ChaVector3(newrigidpos.x, newrigidpos.y, newrigidpos.z);

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
	for (colno = 0; colno < 3; colno++){
		tmpcol[colno] = worldmat.getColumn(colno);
		//		tmpcol[colno] = worldmat.getRow( colno );
	}

	ChaMatrix newxworld;
	ChaMatrixIdentity(&newxworld);
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

	ChaMatrix invxworld;
	ChaMatrixInverse( &invxworld, NULL, &m_xworld );

	ChaMatrix diffxworld;
	diffxworld = invxworld * newxworld;

	//CMotionPoint curmp;
	//curmp = m_bone->GetCurMp();
	m_bone->SetBtMat(m_bone->GetStartMat2() * diffxworld);
	//curmp.SetBtMat(newxworld);
	//m_bone->GetCurMp().SetBtMat(m_bone->GetStartMat2() * diffxworld);
	m_bone->SetBtFlag(1);
	//m_bone->SetCurMp(curmp);


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
			ChaMatrix invstart;
			ChaMatrixInverse( &invstart, NULL, &(m_parbt->m_bone->GetStartMat2()) );
			ChaMatrix diffmat;
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

int CBtObject::SetCapsuleBtMotion(CRigidElem* srcre)
{
	if (m_topflag == 1){
		return 0;
	}
	if (!m_rigidbody){
		return 0;
	}
	if (!m_rigidbody->getMotionState()){
		_ASSERT(0);
		return 0;
	}

	btTransform worldtra;
	m_rigidbody->getMotionState()->getWorldTransform(worldtra);
	btMatrix3x3 worldmat = worldtra.getBasis();
	btVector3 worldpos = worldtra.getOrigin();
	btVector3 tmpcol[3];//行列のカラム表現。
	int colno;
	for (colno = 0; colno < 3; colno++){
		tmpcol[colno] = worldmat.getColumn(colno);
		//		tmpcol[colno] = worldmat.getRow( colno );
	}

	ChaMatrix newxworld;
	ChaMatrixIdentity(&newxworld);
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

	ChaMatrix invxworld;
	ChaMatrixInverse(&invxworld, NULL, &m_xworld);
	//invxworld = m_bone->GetCurrentZeroFrameInvMat();

	ChaMatrix diffxworld;
	diffxworld = invxworld * newxworld;

	//CMotionPoint curmp;
	//curmp = m_bone->GetCurMp();
	//curmp.SetBtMat(m_bone->GetStartMat2() * diffxworld);
	//curmp.SetBtFlag(1);
	//m_bone->SetCurMp(curmp);

	//ChaMatrix newcapsulemat;
	//newcapsulemat = srcre->GetBindcapsulemat() * diffxworld;
	//newcapsulemat = m_bone->CalcManipulatorPostureMatrix(0, 1, 1);
	//srcre->SetCapsulemat(newcapsulemat);


	return 0;
}

int CBtObject::CreatePhysicsPosConstraint()
{


	DestroyGZObj();


//////////////
////////////// mass zero rigidbody
	float h, r, z;
	//max : boneleng 0 対策
	r = 1.0f;
	h = 1.0f;
	z = 1.0f;

	m_gz_colshape = new btSphereShape(btScalar(r));

	ChaVector3 endpos, aftendpos;
	endpos = m_endbone->GetJointFPos();
	ChaMatrix endmat;
	if (g_previewFlag == 5){

		endmat = m_endbone->GetBtMat();
	}
	else{
		endmat = m_endbone->GetCurMp().GetWorldMat();
	}
	ChaVector3TransformCoord(&aftendpos, &endpos, &endmat);

	btVector3 btv(btScalar(aftendpos.x), btScalar(aftendpos.y), btScalar(aftendpos.z));

	btTransform transform;
	transform.setIdentity();
	//transform.setRotation(btq);
	transform.setOrigin(btv);

	m_gz_rigidbody = localCreateRigidBody(0, transform, m_gz_colshape);
	_ASSERT(m_gz_rigidbody);
	if (!m_gz_rigidbody){
		_ASSERT(0);
		return 1;
	}
//////////////////
////////////////// constraint
	if (m_topflag == 1){
		return 0;
	}
	_ASSERT(m_btWorld);
	_ASSERT(m_bone);

	if (!m_endbone){
		return 1;
	}
	if (!m_gz_rigidbody){
		return 1;
	}


	btTransform FrameA;//剛体設定時のA側変換行列。
	btTransform FrameB;//剛体設定時のB側変換行列。
	FrameA.setIdentity();
	FrameB.setIdentity();

	//CRigidElem* tmpre;
	//tmpre = m_bone->GetRigidElem(m_endbone);
	//_ASSERT(tmpre);
	//CalcConstraintTransform(0, tmpre, this, m_FrameA);
	//tmpre = chilbto->m_bone->GetRigidElem(chilbto->m_endbone);
	//_ASSERT(tmpre);
	//CalcConstraintTransform(1, tmpre, chilbto, m_FrameB);


	float angPAI2, angPAI;
	angPAI2 = 90.0f * (float)DEG2PAI;
	angPAI = 180.0f * (float)DEG2PAI;

	float lmax, lmin;
	lmin = -10000.0f;
	lmax = 10000.0f;

	btGeneric6DofSpringConstraint* dofC = 0;
	dofC = new btGeneric6DofSpringConstraint(*m_rigidbody, *m_gz_rigidbody, m_FrameA, m_FrameB, true);
	//dofC = new btConeTwistConstraint(*m_rigidbody, *m_gz_rigidbody, m_FrameA, m_FrameB);
	_ASSERT(dofC);
	if (dofC){
		int dofid;
		for (dofid = 0; dofid < 3; dofid++){
			//dofC->setEquilibriumPoint(dofid);
			dofC->enableSpring(dofid, true);
		}
		for (dofid = 3; dofid < 6; dofid++){
			//dofC->setEquilibriumPoint(dofid);
			dofC->enableSpring(dofid, true);
		}

		m_gz_constraint.constraint = dofC;//!!!!!!!!!!!!!!!!!!!!!!
		m_gz_constraint.centerbone = m_endbone;
		m_gz_constraint.childbto = NULL;

		//m_btWorld->addConstraint(dofC, true);
		//m_btWorld->addConstraint((btTypedConstraint*)dofC, false);//!!!!!!!!!!!! disable collision between linked bodies
		m_btWorld->addConstraint((btTypedConstraint*)dofC, true);//!!!!!!!!!!!! disable collision between linked bodies
		//m_dofC = dofC;

		dofC->setEquilibriumPoint();
	}


	return 0;
}

int CBtObject::DestroyPhysicsPosConstraint()
{
	DestroyGZObj();
	return 0;
}
