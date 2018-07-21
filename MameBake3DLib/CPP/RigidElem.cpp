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

#include <RigidElem.h>
//#include <Bone.h>
//#include <quaternion.h>

#define DBGH
#include <dbg.h>

using namespace std;
/*
extern float g_kinit;
extern float g_initmass;
extern float g_l_dmp;
extern float g_a_dmp;
extern float g_initcuslk;
extern float g_initcusak;
*/

CRigidElem::CRigidElem()
{
	InitParams();
}
CRigidElem::~CRigidElem()
{
	DestroyObjs();
}

int CRigidElem::InitParams()
{
	m_forbidrotflag = 0;

	m_dampanim_l = 0.0f;
	m_dampanim_a = 0.0f;

	m_boneleng = 0.0f;
	m_coltype = COL_CAPSULE_INDEX;
	m_skipflag = 1;

	ChaMatrixIdentity( &m_capsulemat );
	ChaMatrixIdentity(&m_firstcapsulemat);
	ChaMatrixIdentity(&m_firstworldmat);
	ChaMatrixIdentity(&m_bindcapsulemat);

	m_sphrate = 0.6f;
	m_boxzrate = 0.6f;

	m_cylileng = 0.0f;
	m_sphr = 0.0f;
	m_boxz = 0.0f;

	//m_impulse = ChaVector3( 0.0f, 0.0f, 0.0f );
	
	m_l_kindex = 2;
	m_a_kindex = 2;

	m_l_damping = g_l_dmp;
	m_a_damping = g_a_dmp;

	m_mass = g_initmass;

	m_bone = 0;
	m_endbone = 0;

	m_cus_lk = g_initcuslk;
	m_cus_ak = g_initcusak;


	m_groupid = 2;
	m_coliids.clear();
	m_coliids.push_back( 1 );
	m_myselfflag = 1;

	m_restitution = 0.5f;
	m_friction = 0.5f;

	m_btg = -0.09f;

	return 0;
}
int CRigidElem::DestroyObjs()
{
	m_coliids.clear();
	return 0;
}

int CRigidElem::GetColiID()
{
	int retid = 0;

	int idnum = (int)m_coliids.size();
	int ino;
	for( ino = 0; ino < idnum; ino++ ){
		int curid = 1 << (m_coliids[ ino ] - 1);
		retid |= curid;
	}

	if( m_myselfflag == 1 ){
		int curid = 1 << (m_groupid - 1);
		retid |= curid;
	}

	return retid;
}

float CRigidElem::GetBoneLeng()
{
	if (!m_bone || !m_endbone){
		m_boneleng = 0.0f;
		return 0.0f;
	}

	ChaVector3 centerA, parentposA, childposA, aftparentposA, aftchildposA;
	parentposA = m_bone->GetJointFPos();
	ChaVector3TransformCoord(&aftparentposA, &parentposA, &m_bone->GetInitMat());
	childposA = m_endbone->GetJointFPos();
	ChaVector3TransformCoord(&aftchildposA, &childposA, &m_endbone->GetInitMat());
	ChaVector3 diffA = childposA - parentposA;
	m_boneleng = ChaVector3Length(&diffA);

	return m_boneleng;

}


ChaMatrix CRigidElem::GetCapsulemat(int calczeroframe)
{
	int calccapsuleflag = 1;
	m_capsulemat = m_endbone->CalcManipulatorPostureMatrix(calccapsuleflag, 0, 1, 1, calczeroframe);
	return m_capsulemat;

}