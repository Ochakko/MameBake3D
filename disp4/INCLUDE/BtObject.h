#ifndef BTOBJECTH
#define BTOBJECTH

#include <d3dx9.h>
#include <wchar.h>
#include <Coef.h>
#include <string>
#include <map>
#include <vector>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

class CBone;
class CRigidElem;

class CBtObject
{
public:
	CBtObject( CBtObject* parbt, btDynamicsWorld* btWorld );
	~CBtObject();

	int CreateObject( CBtObject* parbt, CBone* parbone, CBone* curbone, CBone* chilbone );
	int CreateBtConstraint();
	int SetBtMotion( D3DXMATRIX* wmat, D3DXMATRIX* vpmat );
	int CalcMass();
	int Motion2Bt();
private:
	int InitParams();
	int DestroyObjs();

	btRigidBody* localCreateRigidBody( CRigidElem* curre, const btTransform& startTransform, btCollisionShape* shape );
	int AddChild( CBtObject* addbt );

	int CalcConstraintTransform( int chilflag, CRigidElem* curre, CBtObject* curbto, btTransform& dstmat );

public:
	btDynamicsWorld* m_btWorld;

	int m_topflag;
	float m_boneleng;
	D3DXMATRIX m_transmat;
	D3DXMATRIX m_par2cen;
	D3DXMATRIX m_cen2parY;

	CBone* m_bone;
	CBone* m_parbone;
	CBone* m_endbone;
	btCollisionShape* m_colshape;
	btRigidBody* m_rigidbody;
	std::vector<btTypedConstraint*> m_constraint;
	btGeneric6DofSpringConstraint* m_dofC;

	CBtObject* m_parbt;
	std::vector<CBtObject*> m_chilbt;

	float m_constzrad;
	int m_connectflag;

	btVector3 m_curpivot;
	btVector3 m_chilpivot;

	btTransform m_FrameA;
	btTransform m_FrameB;

	int m_staticflag;
	D3DXMATRIX m_xworld;
};


#endif