#ifndef BONEPROPH
#define BONEPROPH

class CQuaternion;

#include <d3dx9.h>
#include <Coef.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"


class CBone;
class CModel;

#ifdef BONEPROPCPP
int qToEulerAxis( CQuaternion axisQ, CQuaternion* srcq, D3DXVECTOR3* Euler );
int modifyEuler( D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB );// new, old

float vecDotVec( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 );
float lengthVec( D3DXVECTOR3* vec );
float aCos( float dot );
int vec3RotateY( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );
int vec3RotateX( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );
int vec3RotateZ( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );

int GetRound( float srcval );
int IsInitRot(D3DXMATRIX srcmat);
int IsSameMat(D3DXMATRIX srcmat1, D3DXMATRIX srcmat2);
int IsSameEul(D3DXVECTOR3 srceul1, D3DXVECTOR3 srceul2);

void InitAngleLimit(ANGLELIMIT* dstal);
void SetAngleLimitOff(ANGLELIMIT* dstal);

void InitCustomRig(CUSTOMRIG* dstcr, CBone* parbone, int rigno);
int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parbone);
void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* chilbone);

bool IsTimeEqual(double srctime1, double srctime2);

float VecLength(D3DXVECTOR3 srcvec);
void GetSRTMatrix(D3DXMATRIX srcmat, D3DXVECTOR3* svecptr, D3DXMATRIX* rmatptr, D3DXVECTOR3* tvecptr);

D3DXMATRIX TransZeroMat(D3DXMATRIX srcmat);
D3DXMATRIX D3DXMatrixFromBtMat3x3(btMatrix3x3 srcmat3x3);
D3DXMATRIX D3DXMatrixFromBtTransform(btMatrix3x3 srcmat3x3, btVector3 srcpipot);
D3DXMATRIX D3DXMatrixInv(D3DXMATRIX srcmat);

#else
extern int qToEulerAxis(CQuaternion axisQ, CQuaternion* srcq, D3DXVECTOR3* Euler);
extern int modifyEuler(D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB);// new, old

extern float vecDotVec(D3DXVECTOR3* vec1, D3DXVECTOR3* vec2);
extern float lengthVec(D3DXVECTOR3* vec);
extern float aCos(float dot);
extern int vec3RotateY(D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec);
extern int vec3RotateX(D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec);
extern int vec3RotateZ(D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec);

extern int GetRound(float srcval);
extern int IsInitRot(D3DXMATRIX srcmat);
extern int IsSameMat(D3DXMATRIX srcmat1, D3DXMATRIX srcmat2);
extern int IsSameEul(D3DXVECTOR3 srceul1, D3DXVECTOR3 srceul2);

extern void InitAngleLimit(ANGLELIMIT* dstal);
extern void SetAngleLimitOff(ANGLELIMIT* dstal);

extern void InitCustomRig(CUSTOMRIG* dstcr, CBone* parbone, int rigno);
extern int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parbone);
extern void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* chilbone);

extern bool IsTimeEqual(double srctime1, double srctime2);

extern float VecLength(D3DXVECTOR3 srcvec);
extern void GetSRTMatrix(D3DXMATRIX srcmat, D3DXVECTOR3* svecptr, D3DXMATRIX* rmatptr, D3DXVECTOR3* tvecptr);

extern D3DXMATRIX TransZeroMat(D3DXMATRIX srcmat);
extern D3DXMATRIX D3DXMatrixFromBtMat3x3(btMatrix3x3 srcmat3x3);
extern D3DXMATRIX D3DXMatrixFromBtTransform(btMatrix3x3 srcmat3x3, btVector3 srcpipot);
extern D3DXMATRIX D3DXMatrixInv(D3DXMATRIX srcmat);

#endif
#endif
