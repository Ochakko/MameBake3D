/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2007 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btGeneric6DofSpringConstraint.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btTransformUtil.h"

#include <Windows.h>

btGeneric6DofSpringConstraint::btGeneric6DofSpringConstraint(btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameInA, const btTransform& frameInB ,bool useLinearReferenceFrameA)
	: btGeneric6DofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
{
    init();
}


btGeneric6DofSpringConstraint::btGeneric6DofSpringConstraint(btRigidBody& rbB, const btTransform& frameInB, bool useLinearReferenceFrameB)
        : btGeneric6DofConstraint(rbB, frameInB, useLinearReferenceFrameB)
{
    init();
}


void btGeneric6DofSpringConstraint::init()
{
	m_objectType = D6_SPRING_CONSTRAINT_TYPE;

	for(int i = 0; i < 6; i++)
	{
		m_springEnabled[i] = false;
		m_equilibriumPoint[i] = btScalar(0.f);
		m_springStiffness[i] = btScalar(0.f);
		m_springDamping[i] = btScalar(1.f);
	}

	m_savedcnt = 0;
	memset((void*)m_saveStopErp, 0, sizeof(btScalar) * 6);
	memset((void*)m_saveStopCfm, 0, sizeof(btScalar) * 6);

}


void btGeneric6DofSpringConstraint::enableSpring(int index, bool onOff)
{
	btAssert((index >= 0) && (index < 6));
	m_springEnabled[index] = onOff;
	if(index < 3)
	{
		m_linearLimits.m_enableMotor[index] = onOff;
	}
	else
	{
		m_angularLimits[index - 3].m_enableMotor = onOff;
	}
}



void btGeneric6DofSpringConstraint::setStiffness(int index, btScalar stiffness)
{
	btAssert((index >= 0) && (index < 6));
	m_springStiffness[index] = stiffness;
}


void btGeneric6DofSpringConstraint::setDamping(int index, btScalar damping)
{
	btAssert((index >= 0) && (index < 6));
	m_springDamping[index] = damping;
}


void btGeneric6DofSpringConstraint::setEquilibriumPoint()
{
	calculateTransforms();
	int i;

	for( i = 0; i < 3; i++)
	{
		m_equilibriumPoint[i] = m_calculatedLinearDiff[i];
	}
	for(i = 0; i < 3; i++)
	{
		m_equilibriumPoint[i + 3] = m_calculatedAxisAngleDiff[i];
	}
}



void btGeneric6DofSpringConstraint::setEquilibriumPoint(int index)
{
	btAssert((index >= 0) && (index < 6));
	calculateTransforms();
	if(index < 3)
	{
		m_equilibriumPoint[index] = m_calculatedLinearDiff[index];
	}
	else
	{
		m_equilibriumPoint[index] = m_calculatedAxisAngleDiff[index - 3];
	}
}

void btGeneric6DofSpringConstraint::setEquilibriumPoint(int index, btScalar val)
{
	btAssert((index >= 0) && (index < 6));
	m_equilibriumPoint[index] = val;
}


void btGeneric6DofSpringConstraint::internalUpdateSprings(btConstraintInfo2* info)
{
	// it is assumed that calculateTransforms() have been called before this call
	int i;
	btVector3 relVel = m_rbB.getLinearVelocity() - m_rbA.getLinearVelocity();
	for(i = 0; i < 3; i++)
	{
		if(m_springEnabled[i])
		{
			// get current position of constraint
			btScalar currPos = m_calculatedLinearDiff[i];
			// calculate difference
			btScalar delta = currPos - m_equilibriumPoint[i];
			// spring force is (delta * m_stiffness) according to Hooke's Law
			btScalar force = delta * m_springStiffness[i];
			btScalar velFactor = info->fps * m_springDamping[i] / btScalar(info->m_numIterations);
			m_linearLimits.m_targetVelocity[i] =  velFactor * force;
			m_linearLimits.m_maxMotorForce[i] =  btFabs(force) / info->fps;
		}
	}
	for(i = 0; i < 3; i++)
	{
		if(m_springEnabled[i + 3])
		{
			// get current position of constraint
			btScalar currPos = m_calculatedAxisAngleDiff[i];
			// calculate difference
			btScalar delta = currPos - m_equilibriumPoint[i+3];
			// spring force is (-delta * m_stiffness) according to Hooke's Law
			btScalar force = -delta * m_springStiffness[i+3];
			btScalar velFactor = info->fps * m_springDamping[i+3] / btScalar(info->m_numIterations);
			m_angularLimits[i].m_targetVelocity = velFactor * force;
			m_angularLimits[i].m_maxMotorForce = btFabs(force) / info->fps;
		}
	}
}


void btGeneric6DofSpringConstraint::getInfo2(btConstraintInfo2* info)
{
	// this will be called by constraint solver at the constraint setup stage
	// set current motor parameters
	internalUpdateSprings(info);
	// do the rest of job for constraint setup
	btGeneric6DofConstraint::getInfo2(info);
}


void btGeneric6DofSpringConstraint::setAxis(const btVector3& axis1,const btVector3& axis2)
{
	btVector3 zAxis = axis1.normalized();
	btVector3 yAxis = axis2.normalized();
	btVector3 xAxis = yAxis.cross(zAxis); // we want right coordinate system

	btTransform frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setValue(	xAxis[0], yAxis[0], zAxis[0],	
                                xAxis[1], yAxis[1], zAxis[1],
                                xAxis[2], yAxis[2], zAxis[2]);

	// now get constraint frame in local coordinate systems
	m_frameInA = m_rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = m_rbB.getCenterOfMassTransform().inverse() * frameInW;

  calculateTransforms();
}



void btGeneric6DofSpringConstraint::calculateTransforms()
{
	calculateTransforms(m_rbA.getCenterOfMassTransform(),m_rbB.getCenterOfMassTransform());
}

void btGeneric6DofSpringConstraint::calculateTransforms(const btTransform& transA,const btTransform& transB)
{


	m_calculatedTransformA = transA * m_frameInA;
	m_calculatedTransformB = transB * m_frameInB;
	calculateLinearInfo();

	if( m_savedcnt == 1 ){
		int i;
		for( i = 0; i < 6; i++ ){
			m_saveStopErp[i] = getParam( BT_CONSTRAINT_STOP_ERP, i );
			m_saveStopCfm[i] = getParam( BT_CONSTRAINT_STOP_CFM, i );
			enableSpring( i, false );
			setParam( BT_CONSTRAINT_STOP_ERP, 1.0, i );
			setParam( BT_CONSTRAINT_STOP_CFM, 0.2, i );
		}

		m_savedcnt = 2;
	}else if( m_savedcnt == 2 ){
		int i;
		for( i = 0; i < 6; i++ ){
			//setParam( BT_CONSTRAINT_STOP_ERP, m_saveStopErp[i], i );
			//setParam( BT_CONSTRAINT_STOP_CFM, m_saveStopCfm[i], i );
			setParam( BT_CONSTRAINT_STOP_ERP, 0.0, i );
			setParam( BT_CONSTRAINT_STOP_CFM, 0.2, i );
			//enableSpring( i, true );
		}

		m_savedcnt = 0;
	}


	calculateAngleInfo();
	if(m_useOffsetForConstraintFrame)
	{	//  get weight factors depending on masses
		btScalar miA = getRigidBodyA().getInvMass();
		btScalar miB = getRigidBodyB().getInvMass();
		m_hasStaticBody = (miA < SIMD_EPSILON) || (miB < SIMD_EPSILON);
		btScalar miS = miA + miB;
		if(miS > btScalar(0.f))
		{
			m_factA = miB / miS;
		}
		else 
		{
			m_factA = btScalar(0.5f);
		}
		m_factB = btScalar(1.0f) - m_factA;
	}
}
