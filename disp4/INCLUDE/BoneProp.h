#ifndef BONEPROPH
#define BONEPROPH

class CQuaternion;

//#include <d3dx9.h>
#include <Coef.h>
#include <ChaVecCalc.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"


class CBone;
class CModel;

#ifdef BONEPROPCPP
int qToEulerAxis( CQuaternion axisQ, CQuaternion* srcq, ChaVector3* Euler );
int modifyEuler( ChaVector3* eulerA, ChaVector3* eulerB );// new, old

float vecDotVec( ChaVector3* vec1, ChaVector3* vec2 );
float lengthVec( ChaVector3* vec );
float aCos( float dot );
int vec3RotateY( ChaVector3* dstvec, float deg, ChaVector3* srcvec );
int vec3RotateX( ChaVector3* dstvec, float deg, ChaVector3* srcvec );
int vec3RotateZ( ChaVector3* dstvec, float deg, ChaVector3* srcvec );

int GetRound( float srcval );
int IsInitRot(ChaMatrix srcmat);
int IsSameMat(ChaMatrix srcmat1, ChaMatrix srcmat2);
int IsSameEul(ChaVector3 srceul1, ChaVector3 srceul2);

void InitAngleLimit(ANGLELIMIT* dstal);
void SetAngleLimitOff(ANGLELIMIT* dstal);

void InitCustomRig(CUSTOMRIG* dstcr, CBone* parbone, int rigno);
int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parbone);
void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* chilbone);

bool IsTimeEqual(double srctime1, double srctime2);

float VecLength(ChaVector3 srcvec);
void GetSRTMatrix(ChaMatrix srcmat, ChaVector3* svecptr, ChaMatrix* rmatptr, ChaVector3* tvecptr);

ChaMatrix TransZeroMat(ChaMatrix srcmat);
ChaMatrix ChaMatrixFromBtMat3x3(btMatrix3x3 srcmat3x3);
ChaMatrix ChaMatrixFromBtTransform(btMatrix3x3 srcmat3x3, btVector3 srcpipot);
ChaMatrix ChaMatrixInv(ChaMatrix srcmat);

#else
extern int qToEulerAxis(CQuaternion axisQ, CQuaternion* srcq, ChaVector3* Euler);
extern int modifyEuler(ChaVector3* eulerA, ChaVector3* eulerB);// new, old

extern float vecDotVec(ChaVector3* vec1, ChaVector3* vec2);
extern float lengthVec(ChaVector3* vec);
extern float aCos(float dot);
extern int vec3RotateY(ChaVector3* dstvec, float deg, ChaVector3* srcvec);
extern int vec3RotateX(ChaVector3* dstvec, float deg, ChaVector3* srcvec);
extern int vec3RotateZ(ChaVector3* dstvec, float deg, ChaVector3* srcvec);

extern int GetRound(float srcval);
extern int IsInitRot(ChaMatrix srcmat);
extern int IsSameMat(ChaMatrix srcmat1, ChaMatrix srcmat2);
extern int IsSameEul(ChaVector3 srceul1, ChaVector3 srceul2);

extern void InitAngleLimit(ANGLELIMIT* dstal);
extern void SetAngleLimitOff(ANGLELIMIT* dstal);

extern void InitCustomRig(CUSTOMRIG* dstcr, CBone* parbone, int rigno);
extern int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parbone);
extern void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* chilbone);

extern bool IsTimeEqual(double srctime1, double srctime2);

extern float VecLength(ChaVector3 srcvec);
extern void GetSRTMatrix(ChaMatrix srcmat, ChaVector3* svecptr, ChaMatrix* rmatptr, ChaVector3* tvecptr);

extern ChaMatrix TransZeroMat(ChaMatrix srcmat);
extern ChaMatrix ChaMatrixFromBtMat3x3(btMatrix3x3 srcmat3x3);
extern ChaMatrix ChaMatrixFromBtTransform(btMatrix3x3 srcmat3x3, btVector3 srcpipot);
extern ChaMatrix ChaMatrixInv(ChaMatrix srcmat);

#endif
#endif
