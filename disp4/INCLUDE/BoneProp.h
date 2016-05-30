#ifndef BONEPROPH
#define BONEPROPH

class CQuaternion;

#include <d3dx9.h>
#include <Coef.h>

class CBone;
class CModel;

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

#endif
