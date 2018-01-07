#ifndef QUATERNIONH

#define QUATERNIONH

//#include <D3DX9.h>
#include <ChaVecCalc.h>
#include <coef.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"


#ifdef QUATERNIONCPP
void D3DVec3TransformCoord(ChaVector3* dstvec, ChaVector3* srcvec, ChaMatrix* srcmat);
#else
extern void D3DVec3TransformCoord(ChaVector3* dstvec, ChaVector3* srcvec, ChaMatrix* srcmat);
#endif

class CQuaternion
{
public:
	CQuaternion();
	CQuaternion( float srcw, float srcx, float srcy, float srcz );
	~CQuaternion();
	int SetParams( float srcw, float srcx, float srcy, float srcz );
	int SetParams( D3DXQUATERNION srcxq );

	int SetAxisAndRot( ChaVector3 srcaxis, float phai );
	int SetAxisAndRot( ChaVector3 srcaxis, double phai );

	int SetRotation( CQuaternion* axisq, ChaVector3 srcdeg );
	int SetRotation( CQuaternion* axisq, double degx, double degy, double degz );
	int GetAxisAndRot( ChaVector3* axisvecptr, float* frad );
	int QuaternionToAxisAngle( ChaVector3* dstaxis, float* dstrad );
	int CalcFBXEul( CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul, int isfirstbone );
	int CalcFBXEulZXY(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul, int isfirstbone);
	int IsInit();

	void MakeFromBtMat3x3(btMatrix3x3 eulmat);
	void MakeFromD3DXMat(ChaMatrix eulmat);

	//CQuaternion operator= (const CQuaternion &q) const;
	CQuaternion CQuaternion::operator= (CQuaternion q);
	CQuaternion operator* (float srcw) const;
	CQuaternion &operator*= (float srcw);
	CQuaternion operator/ (float srcw) const;
	CQuaternion &operator/= (float srcw);
	CQuaternion operator+ (const CQuaternion &q) const;
	CQuaternion &operator+= (const CQuaternion &q);
	CQuaternion operator- (const CQuaternion &q) const;
	CQuaternion &operator-= (const CQuaternion &q);
	CQuaternion operator* (const CQuaternion &q) const;
	CQuaternion &operator*= (const CQuaternion &q);
	CQuaternion operator/ (const CQuaternion &q) const;
	CQuaternion &operator/= (const CQuaternion &q);
	CQuaternion operator- () const;
	//CQuaternion inv () const;
	CQuaternion normalize ();
	
	int CQuaternion::inv( CQuaternion* dstq ); 
	
	bool CQuaternion::operator== (const CQuaternion &q) const { return w == q.w && x == q.x && y == q.y && z == q.z; }
	bool CQuaternion::operator!= (const CQuaternion &q) const { return !(*this == q); }

	float DotProduct( CQuaternion srcq );
	float CalcRad( CQuaternion srcq );

	CQuaternion Slerp( CQuaternion endq, int framenum, int frameno );
	int Slerp2( CQuaternion endq, double srcrate, CQuaternion* dstq );

	int Squad( CQuaternion q0, CQuaternion q1, CQuaternion q2, CQuaternion q3, float t );
	
	ChaMatrix MakeRotMatX();

	int RotationArc( ChaVector3 srcvec0, ChaVector3 srcvec1 );
	int Rotate( ChaVector3* dstvec, ChaVector3 srcvec );

	int Q2X( D3DXQUATERNION* dstx );
	int Q2X( D3DXQUATERNION* dstx, CQuaternion srcq );

	int transpose( CQuaternion* dstq );

	int CalcSym( CQuaternion* dstq );
	ChaMatrix CalcSymX2();


	//ç∂ÇÀÇ∂
	int Q2EulZXY(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul);
	int Q2EulYXZ(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul);
	int Q2EulXYZ(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul);//bulletÇ‡XYZÇÃèá
	int Q2EulZYX(int needmodifyflag, CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul);


	int Q2Eul( CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul );
	int Q2EulBt( ChaVector3* reteul );

	void RotationMatrix(ChaMatrix srcmat);

	//inout : srcdstq
	int InOrder(CQuaternion* srcdstq);

private:

	float vecDotVec( ChaVector3* vec1, ChaVector3* vec2 );
	float lengthVec( ChaVector3* vec );
	float aCos( float dot );
	int vec3RotateY( ChaVector3* dstvec, float deg, ChaVector3* srcvec );
	int vec3RotateX( ChaVector3* dstvec, float deg, ChaVector3* srcvec );
	int vec3RotateZ( ChaVector3* dstvec, float deg, ChaVector3* srcvec );

	int ModifyEuler( ChaVector3* eulerA, ChaVector3* eulerB );
	int GetRound( float srcval );	


public:
	float x, y, z, w;
};
#endif