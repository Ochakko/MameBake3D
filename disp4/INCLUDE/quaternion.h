#ifndef QUATERNIONH

#define QUATERNIONH

#include <D3DX9.h>
#include <coef.h>

class CQuaternion
{
public:
	CQuaternion();
	CQuaternion( float srcw, float srcx, float srcy, float srcz );
	~CQuaternion();
	int SetParams( float srcw, float srcx, float srcy, float srcz );
	int SetParams( D3DXQUATERNION srcxq );

	int SetAxisAndRot( D3DXVECTOR3 srcaxis, float phai );
	int SetAxisAndRot( D3DXVECTOR3 srcaxis, double phai );

	int SetRotation( CQuaternion* axisq, D3DXVECTOR3 srcdeg );
	int SetRotation( CQuaternion* axisq, double degx, double degy, double degz );
	int GetAxisAndRot( D3DXVECTOR3* axisvecptr, float* frad );
	int QuaternionToAxisAngle( D3DXVECTOR3* dstaxis, float* dstrad );
	int CalcFBXEul( CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul, int isfirstbone );
	int CalcFBXEulZXY(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul, int isfirstbone);
	int IsInit();



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
	CQuaternion normalize () const;
	
	int CQuaternion::inv( CQuaternion* dstq ); 
	
	bool CQuaternion::operator== (const CQuaternion &q) const { return w == q.w && x == q.x && y == q.y && z == q.z; }
	bool CQuaternion::operator!= (const CQuaternion &q) const { return !(*this == q); }

	float DotProduct( CQuaternion srcq );
	float CalcRad( CQuaternion srcq );

	CQuaternion Slerp( CQuaternion endq, int framenum, int frameno );
	int Slerp2( CQuaternion endq, double srcrate, CQuaternion* dstq );

	int Squad( CQuaternion q0, CQuaternion q1, CQuaternion q2, CQuaternion q3, float t );
	
	D3DXMATRIX MakeRotMatX();

	int RotationArc( D3DXVECTOR3 srcvec0, D3DXVECTOR3 srcvec1 );
	int Rotate( D3DXVECTOR3* dstvec, D3DXVECTOR3 srcvec );

	int Q2X( D3DXQUATERNION* dstx );
	int Q2X( D3DXQUATERNION* dstx, CQuaternion srcq );

	int transpose( CQuaternion* dstq );

	int CalcSym( CQuaternion* dstq );
	D3DXMATRIX CalcSymX2();


	//ç∂ÇÀÇ∂
	int Q2EulZXY(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul);
	int Q2EulYXZ(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul);
	int Q2EulXYZ(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul);
	int Q2EulZYXbt(int needmodifyflag, CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul);


	int Q2Eul( CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul );
	int Q2EulBt( D3DXVECTOR3* reteul );

	void RotationMatrix(D3DXMATRIX srcmat);

	//inout : srcdstq
	int InOrder(CQuaternion* srcdstq);

private:

	float vecDotVec( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 );
	float lengthVec( D3DXVECTOR3* vec );
	float aCos( float dot );
	int vec3RotateY( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );
	int vec3RotateX( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );
	int vec3RotateZ( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec );

	int ModifyEuler( D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB );
	int GetRound( float srcval );	


public:
	float x, y, z, w;
};
#endif