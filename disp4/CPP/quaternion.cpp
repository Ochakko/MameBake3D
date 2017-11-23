#include <windows.h>
#include <math.h>
#include <quaternion.h>
#include <BoneProp.h>
#include <crtdbg.h>


CQuaternion::CQuaternion()
{
	w = 1.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

CQuaternion::CQuaternion( float srcw, float srcx, float srcy, float srcz )
{
	w = srcw;
	x = srcx;
	y = srcy;
	z = srcz;
}
CQuaternion::~CQuaternion()
{

}

int CQuaternion::SetParams( float srcw, float srcx, float srcy, float srcz )
{
	w = srcw;
	x = srcx;
	y = srcy;
	z = srcz;
	return 0;
}
int CQuaternion::SetParams( D3DXQUATERNION srcxq )
{
	w = srcxq.w;
	x = srcxq.x;
	y = srcxq.y;
	z = srcxq.z;
	return 0;
}


int CQuaternion::SetAxisAndRot( D3DXVECTOR3 srcaxis, float phai )
{
	float phai2;
	float cos_phai2, sin_phai2;

	phai2 = phai * 0.5f;
	cos_phai2 = cosf( phai2 );
	sin_phai2 = sinf( phai2 );

	w = cos_phai2;
	x = srcaxis.x * sin_phai2;
	y = srcaxis.y * sin_phai2;
	z = srcaxis.z * sin_phai2;

	return 0;
}
int CQuaternion::SetAxisAndRot( D3DXVECTOR3 srcaxis, double phai )
{
	double phai2;
	double cos_phai2, sin_phai2;

	phai2 = phai * 0.5;
	cos_phai2 = cos( phai2 );
	sin_phai2 = sin( phai2 );

	w = (float)cos_phai2;
	x = (float)( srcaxis.x * sin_phai2 );
	y = (float)( srcaxis.y * sin_phai2 );
	z = (float)( srcaxis.z * sin_phai2 );

	return 0;
}

int CQuaternion::GetAxisAndRot( D3DXVECTOR3* axisvecptr, float* frad )
{
	D3DXQUATERNION tempq;

	tempq.x = x;
	tempq.y = y;
	tempq.z = z;
	tempq.w = w;

	D3DXQuaternionToAxisAngle( &tempq, axisvecptr, frad );
	
	D3DXVec3Normalize( axisvecptr, axisvecptr );

	return 0;
}


int CQuaternion::SetRotation( CQuaternion* axisq, D3DXVECTOR3 srcdeg )
{
	// Z軸、X軸、Y軸の順番で、回転する、クォータニオンをセットする。

	CQuaternion axisQ, invaxisQ;
	if( axisq ){
		axisQ = *axisq;
		axisQ.inv( &invaxisQ );
	}else{
		axisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		invaxisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
	}


	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;
	float fDeg2Pai = (float)DEG2PAI;

	cosx = (float)cos( srcdeg.x * 0.5f * fDeg2Pai );
	sinx = (float)sin( srcdeg.x * 0.5f * fDeg2Pai );
	cosy = (float)cos( srcdeg.y * 0.5f * fDeg2Pai );
	siny = (float)sin( srcdeg.y * 0.5f * fDeg2Pai );
	cosz = (float)cos( srcdeg.z * 0.5f * fDeg2Pai );
	sinz = (float)sin( srcdeg.z * 0.5f * fDeg2Pai );

	qx.SetParams( cosx, sinx, 0.0f, 0.0f );
	qy.SetParams( cosy, 0.0f, siny, 0.0f );
	qz.SetParams( cosz, 0.0f, 0.0f, sinz );

	q = axisQ * qy * qx * qz * invaxisQ;


	*this = q;

	return 0;
}

int CQuaternion::SetRotation( CQuaternion* axisq, double degx, double degy, double degz )
{
	// Z軸、X軸、Y軸の順番で、回転する、クォータニオンをセットする。
	CQuaternion axisQ, invaxisQ;
	if( axisq ){
		axisQ = *axisq;
		axisQ.inv( &invaxisQ );
	}else{
		axisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		invaxisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
	}

	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;

	cosx = (float)cos( degx * 0.5 * DEG2PAI );
	sinx = (float)sin( degx * 0.5 * DEG2PAI );
	cosy = (float)cos( degy * 0.5 * DEG2PAI );
	siny = (float)sin( degy * 0.5 * DEG2PAI );
	cosz = (float)cos( degz * 0.5 * DEG2PAI );
	sinz = (float)sin( degz * 0.5 * DEG2PAI );

	qx.SetParams( cosx, sinx, 0.0f, 0.0f );
	qy.SetParams( cosy, 0.0f, siny, 0.0f );
	qz.SetParams( cosz, 0.0f, 0.0f, sinz );

	q = axisQ * qy * qx * qz * invaxisQ;

	*this = q;

	return 0;
}



CQuaternion CQuaternion::operator= (CQuaternion q) { this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; return *this; };
CQuaternion CQuaternion::operator* (float srcw) const { return CQuaternion(this->w * srcw, this->x * srcw, this->y * srcw, this->z * srcw); }
CQuaternion &CQuaternion::operator*= (float srcw) { *this = *this * srcw; return *this; }
CQuaternion CQuaternion::operator/ (float srcw) const { return CQuaternion(this->w / srcw, this->x / srcw, this->y / srcw, this->z / srcw); }
CQuaternion &CQuaternion::operator/= (float srcw) { *this = *this / srcw; return *this; }
CQuaternion CQuaternion::operator+ (const CQuaternion &q) const { return CQuaternion(w + q.w, x + q.x, y + q.y, z + q.z); }
CQuaternion &CQuaternion::operator+= (const CQuaternion &q) { *this = *this + q; return *this; }
CQuaternion CQuaternion::operator- (const CQuaternion &q) const { return CQuaternion(w - q.w, x - q.x, y - q.y, z - q.z); }
CQuaternion &CQuaternion::operator-= (const CQuaternion &q) { *this = *this - q; return *this; }
CQuaternion CQuaternion::operator* (const CQuaternion &q) const {
	return CQuaternion(
		w * q.w - x * q.x - y * q.y - z * q.z,
		w * q.x + q.w * x + y * q.z - z * q.y,
		w * q.y + q.w * y + z * q.x - x * q.z,
		w * q.z + q.w * z + x * q.y - y * q.x ).normalize();
}
CQuaternion &CQuaternion::operator*= (const CQuaternion &q) { *this = *this * q; return *this; }
CQuaternion CQuaternion::operator- () const { return *this * -1.0f; }
CQuaternion CQuaternion::normalize () const { 
	float mag = w*w+x*x+y*y+z*z;
	if( mag != 0.0f )
		return (*this)*(1.0f/(float)::sqrt(mag));
	else
		return CQuaternion( 1.0f, 0.0f, 0.0f, 0.0f );
}


D3DXMATRIX CQuaternion::MakeRotMatX()
{
	normalize();

	D3DXMATRIX retmat;
	D3DXMatrixIdentity(&retmat);

	//転置
	retmat._11 = 1.0 - 2.0 * (y * y + z * z);
	retmat._21 = 2.0 * (x * y - z * w);
	retmat._31 = 2.0 * (z * x + w * y);

	retmat._12 = 2.0 * (x * y + z * w);
	retmat._22 = 1.0 - 2.0 * (z * z + x * x);
	retmat._32 = 2.0 * (y * z - w * x);

	retmat._13 = 2.0 * (z * x - w * y);
	retmat._23 = 2.0 * (y * z + x * w);
	retmat._33 = 1.0 - 2.0 * (y * y + x * x);

	/*
	retmat._11 = 1.0 - 2.0 * (y * y + z * z);
	retmat._12 = 2.0 * (x * y - z * w);
	retmat._13 = 2.0 * (z * x + w * y);

	retmat._21 = 2.0 * (x * y + z * w);
	retmat._22 = 1.0 - 2.0 * (z * z + x * x);
	retmat._23 = 2.0 * (y * z - w * x);

	retmat._31 = 2.0 * (z * x - w * y);
	retmat._32 = 2.0 * (y * z + x * w);
	retmat._33 = 1.0 - 2.0 * (y * y + x * x);
	*/
	
	/*
	D3DXQUATERNION qx;
	qx.x = x;
	qx.y = y;
	qx.z = z;
	qx.w = w;
	D3DXMatrixRotationQuaternion(&retmat, &qx);
	*/
	return retmat;
}


void CQuaternion::RotationMatrix(D3DXMATRIX srcmat)
{
	//転置前バージョン

	//CQuaternionは gpar * par * curの順で掛ける系
	//D3DXMATRIXは cur * par * gparの順で掛ける系
	//CQuaternionの時に転置してD3DXMATRIXのときにそのままで計算が合う。
	
	CQuaternion tmpq;

	//スケールに関して正規化した回転からQuaternionを求める。そのためにSRTに分解する。
	D3DXVECTOR3 svec, tvec;
	D3DXMATRIX rmat;
	GetSRTMatrix(srcmat, &svec, &rmat, &tvec);

	int i, maxi;
	FLOAT maxdiag, S, trace;
	
	trace = rmat.m[0][0] + rmat.m[1][1] + rmat.m[2][2] + 1.0f;
	if (trace > 0.0f)
	{
		tmpq.x = (rmat.m[1][2] - rmat.m[2][1]) / (2.0f * sqrt(trace));
		tmpq.y = (rmat.m[2][0] - rmat.m[0][2]) / (2.0f * sqrt(trace));
		tmpq.z = (rmat.m[0][1] - rmat.m[1][0]) / (2.0f * sqrt(trace));
		tmpq.w = sqrt(trace) / 2.0f;
		*this = tmpq;
		return;
	}
	maxi = 0;
	maxdiag = rmat.m[0][0];
	for (i = 1; i<3; i++)
	{
		if (rmat.m[i][i] > maxdiag)
		{
			maxi = i;
			maxdiag = rmat.m[i][i];
		}
	}
	switch (maxi)
	{
	case 0:
		S = 2.0f * sqrt(1.0f + rmat.m[0][0] - rmat.m[1][1] - rmat.m[2][2]);
		tmpq.x = 0.25f * S;
		tmpq.y = (rmat.m[0][1] + rmat.m[1][0]) / S;
		tmpq.z = (rmat.m[0][2] + rmat.m[2][0]) / S;
		tmpq.w = (rmat.m[1][2] - rmat.m[2][1]) / S;
		break;
	case 1:
		S = 2.0f * sqrt(1.0f + rmat.m[1][1] - rmat.m[0][0] - rmat.m[2][2]);
		tmpq.x = (rmat.m[0][1] + rmat.m[1][0]) / S;
		tmpq.y = 0.25f * S;
		tmpq.z = (rmat.m[1][2] + rmat.m[2][1]) / S;
		tmpq.w = (rmat.m[2][0] - rmat.m[0][2]) / S;
		break;
	case 2:
		S = 2.0f * sqrt(1.0f + rmat.m[2][2] - rmat.m[0][0] - rmat.m[1][1]);
		tmpq.x = (rmat.m[0][2] + rmat.m[2][0]) / S;
		tmpq.y = (rmat.m[1][2] + rmat.m[2][1]) / S;
		tmpq.z = 0.25f * S;
		tmpq.w = (rmat.m[0][1] - rmat.m[1][0]) / S;
		break;
	}
	*this = tmpq;

	
	//D3DXMATRIX tmpmat;
	//tmpmat = srcmat;
	//tmpmat._41 = 0.0f;
	//tmpmat._42 = 0.0f;
	//tmpmat._43 = 0.0f;

	//D3DXQUATERNION qx;
	//D3DXQuaternionRotationMatrix(&qx, &tmpmat);
	//SetParams(qx);
}

/*
void CQuaternion::RotationMatrix(D3DXMATRIX srcmat)
{
//転置後バージョン

CQuaternion tmpq;

//スケールに関して正規化した回転からQuaternionを求める。そのためにSRTに分解する。
D3DXVECTOR3 svec, tvec;
D3DXMATRIX rmat;
GetSRTMatrix(srcmat, &svec, &rmat, &tvec);

int i, maxi;
FLOAT maxdiag, S, trace;

trace = rmat.m[0][0] + rmat.m[1][1] + rmat.m[2][2] + 1.0f;
if (trace > 0.0f)
{
tmpq.x = (rmat.m[2][1] - rmat.m[1][2]) / (2.0f * sqrt(trace));
tmpq.y = (rmat.m[0][2] - rmat.m[2][0]) / (2.0f * sqrt(trace));
tmpq.z = (rmat.m[1][0] - rmat.m[0][1]) / (2.0f * sqrt(trace));
tmpq.w = sqrt(trace) / 2.0f;
*this = tmpq;
return;
}
maxi = 0;
maxdiag = rmat.m[0][0];
for (i = 1; i<3; i++)
{
if (rmat.m[i][i] > maxdiag)
{
maxi = i;
maxdiag = rmat.m[i][i];
}
}
switch (maxi)
{
case 0:
S = 2.0f * sqrt(1.0f + rmat.m[0][0] - rmat.m[1][1] - rmat.m[2][2]);
tmpq.x = 0.25f * S;
tmpq.y = (rmat.m[1][0] + rmat.m[0][1]) / S;
tmpq.z = (rmat.m[2][0] + rmat.m[0][2]) / S;
tmpq.w = (rmat.m[2][1] - rmat.m[1][2]) / S;
break;
case 1:
S = 2.0f * sqrt(1.0f + rmat.m[1][1] - rmat.m[0][0] - rmat.m[2][2]);
tmpq.x = (rmat.m[1][0] + rmat.m[0][1]) / S;
tmpq.y = 0.25f * S;
tmpq.z = (rmat.m[2][1] + rmat.m[1][2]) / S;
tmpq.w = (rmat.m[0][2] - rmat.m[2][0]) / S;
break;
case 2:
S = 2.0f * sqrt(1.0f + rmat.m[2][2] - rmat.m[0][0] - rmat.m[1][1]);
tmpq.x = (rmat.m[2][0] + rmat.m[0][2]) / S;
tmpq.y = (rmat.m[2][1] + rmat.m[1][2]) / S;
tmpq.z = 0.25f * S;
tmpq.w = (rmat.m[1][0] - rmat.m[0][1]) / S;
break;
}
*this = tmpq;

}
*/

float CQuaternion::DotProduct( CQuaternion srcq )
{
	float dot;
	dot = w * srcq.w + 
		x * srcq.x + 
		y * srcq.y +
		z * srcq.z;
	return dot;
}

float CQuaternion::CalcRad( CQuaternion srcq )
{
	float dot, retrad;
	dot = this->DotProduct( srcq );

	//!!!!!!!!!!　注意　!!!!!!!!!!!!
	//!!!! dot が１より微妙に大きい値のとき、kakuには、無効な値(-1.#IN00)が入ってしまう。
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if( dot > 1.0f )
		dot = 1.0f;
	if( dot < -1.0f )
		dot = -1.0f;
	retrad = (float)acos( dot );

	return retrad;
}

int CQuaternion::Slerp2( CQuaternion endq, double t, CQuaternion* dstq )
{
	dstq->SetParams( 1.0f, 0.0f, 0.0f, 0.0f );

	if( t == 1.0 ){
		*dstq = endq;
		return 0;//!!!!!!!!!!!!!!!!!!!!!!!!!
	}else if( t == 0.0 ){
		*dstq = *this;
		return 0;//!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	float kaku;
	kaku = this->CalcRad( endq );

	if( kaku > (PI * 0.5f) ){
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		endq = -endq;
		kaku = this->CalcRad( endq );
		_ASSERT( kaku <= (PI * 0.5f) );
	}

	// sin( kaku ) == 0.0 付近を調整。
	//180度にはならないので（ならないようにするので）０度のみケア
	int kaku0flag = 0;
	if( (kaku <= 1e-4) && (kaku >= -1e-4) ){
		kaku0flag = 1;
	}

	CQuaternion tmpq;
	tmpq.SetParams( w, x, y, z );

	float alpha, beta;
	if( kaku0flag == 0 ){
		alpha = (float)sin( kaku * (1.0f - t) ) / (float)sin( kaku );
		beta = (float)sin( kaku * t ) / (float)sin( kaku );

		dstq->x = tmpq.x * alpha + endq.x * beta;
		dstq->y = tmpq.y * alpha + endq.y * beta;
		dstq->z = tmpq.z * alpha + endq.z * beta;
		dstq->w = tmpq.w * alpha + endq.w * beta;
//		retq = tmpq * alpha + endq * beta;

	}else{
		*dstq = tmpq;
	}
	return 0;
}

CQuaternion CQuaternion::Slerp( CQuaternion endq, int framenum, int frameno )
{
	CQuaternion retq;
	retq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );

	float kaku;
	kaku = this->CalcRad( endq );

	if( kaku > (PI * 0.5f) ){
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		endq = -endq;
		kaku = this->CalcRad( endq );
		_ASSERT( kaku <= (PI * 0.5f) );
	}

	// sin( kaku ) == 0.0 付近を調整。
	//180度にはならないので（ならないようにするので）０度のみケア
	int kaku0flag = 0;
	if( (kaku <= 1e-4) && (kaku >= -1e-4) ){
		kaku0flag = 1;
//DbgOut( "Quaternion : Slerp : kaku0flag 1 : dot %f, kaku %f\n", dot, kaku );
	}


	float t = (float)frameno / (float)framenum;
	float alpha, beta;
	if( kaku0flag == 0 ){
		alpha = (float)sin( kaku * (1.0f - t) ) / (float)sin( kaku );
		beta = (float)sin( kaku * t ) / (float)sin( kaku );

		retq = *this * alpha + endq * beta;
	}else{
		retq = *this;
//DbgOut( "MotionInfo : FillUpMotionPoint 2: frame %d, %f, %f, %f, %f\n",
//	   frameno, startq.w, startq.x, startq.y, startq.z );
	}
		
	return retq;
}


int CQuaternion::Squad( CQuaternion q0, CQuaternion q1, CQuaternion q2, CQuaternion q3, float t )
{

	CQuaternion iq0, iq1, iq2, iq3;
	D3DXQUATERNION qx0, qx1, qx2, qx3;
	D3DXQUATERNION ax, bx, cx, resx;

	q0.inv( &iq0 );
	q1.inv( &iq1 );
	q2.inv( &iq2 );
	q3.inv( &iq3 );

	Q2X( &qx0, iq0 );
	Q2X( &qx1, iq1 );
	Q2X( &qx2, iq2 );
	Q2X( &qx3, iq3 );

	D3DXQuaternionSquadSetup( &ax, &bx, &cx, &qx0, &qx1, &qx2, &qx3 );
	D3DXQuaternionSquad( &resx, &qx1, &ax, &bx, &cx, t );

	D3DXQUATERNION iresx;
	D3DXQuaternionInverse( &iresx, &resx );

	this->x = iresx.x;
	this->y = iresx.y;
	this->z = iresx.z;
	this->w = iresx.w;


	return 0;
}


int CQuaternion::Q2X( D3DXQUATERNION* dstx )
{
	dstx->x = x;
	dstx->y = y;
	dstx->z = z;
	dstx->w = w;
	
	return 0;
}

int CQuaternion::Q2X( D3DXQUATERNION* dstx, CQuaternion srcq )
{
	dstx->x = srcq.x;
	dstx->y = srcq.y;
	dstx->z = srcq.z;
	dstx->w = srcq.w;
	
	return 0;
}

int CQuaternion::inv( CQuaternion* dstq ) 
{

	dstq->w = w;	
	dstq->x = -x;
	dstq->y = -y;
	dstq->z = -z;

	*dstq = dstq->normalize();

	return 0;
}

int CQuaternion::RotationArc( D3DXVECTOR3 srcvec0, D3DXVECTOR3 srcvec1 )
{
	//srcvec0, srcvec1は、normalizeされているとする。

	D3DXVECTOR3 c;
	D3DXVec3Cross( &c, &srcvec0, &srcvec1 );
	float d;
	d = D3DXVec3Dot( &srcvec0, &srcvec1 );
	float s;
	s = (float)sqrt( (1 + d) * 2.0f );

	x = c.x / s;
	y = c.y / s;
	z = c.z / s;
	w = s / 2.0f;

	return 0;
}


int CQuaternion::Rotate( D3DXVECTOR3* dstvec, D3DXVECTOR3 srcvec )
{
	D3DXMATRIX mat;

	mat = MakeRotMatX();

	D3DXVec3TransformCoord( dstvec, &srcvec, &mat );
	
	return 0;
}

int CQuaternion::QuaternionToAxisAngle( D3DXVECTOR3* dstaxis, float* dstrad )
{
	D3DXQUATERNION xq;

	int ret;
	ret = Q2X( &xq );
	_ASSERT( !ret );

	D3DXQuaternionToAxisAngle( &xq, dstaxis, dstrad );

	return 0;
}

int CQuaternion::transpose( CQuaternion* dstq )
{
	D3DXMATRIX matx;

	matx = MakeRotMatX();

	D3DXMATRIX tmatx;
	D3DXMatrixTranspose( &tmatx, &matx );

	dstq->RotationMatrix(tmatx);

	return 0;
}

D3DXMATRIX CQuaternion::CalcSymX2()
{
	CQuaternion tmpq;
	tmpq = *this;
	tmpq.x *= -1.0f;
	//tmpq.y *= -1.0f;
	//tmpq.z *= -1.0f;
	tmpq.w *= -1.0f;

	return tmpq.MakeRotMatX();
}


int CQuaternion::CalcSym( CQuaternion* dstq )
{
	CQuaternion tmpq;
	tmpq = *this;
	tmpq.x *= -1.0f;
	//tmpq.y *= -1.0f;
	//tmpq.z *= -1.0f;
	tmpq.w *= -1.0f;

	*dstq = tmpq;

	return 0;
}

float CQuaternion::vecDotVec( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 )
{
	return ( vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z );
}

float CQuaternion::lengthVec( D3DXVECTOR3* vec )
{
	double mag;
	float leng;
	
	mag = vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
	if( mag == 0.0 ){
		leng = 0.0f;
	}else{
		leng = (float)sqrt( mag );
	}
	return leng;
}

float CQuaternion::aCos( float dot )
{
	if( dot > 1.0f )
		dot = 1.0f;
	else if( dot < -1.0f )
		dot = -1.0f;

	double rad;
	rad = acos( dot );

	float degree;
	degree = (float)( rad * PAI2DEG );

	return degree;
}

int CQuaternion::vec3RotateY( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( 0, 0, deg, 0 );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}
int CQuaternion::vec3RotateX( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( 0, deg, 0, 0 );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}
int CQuaternion::vec3RotateZ( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( 0, 0, 0, deg );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}

int CQuaternion::Q2EulBt( D3DXVECTOR3* reteul )
{
	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec( 1.0f, 0.0f, 0.0f );
	D3DXVECTOR3 axisYVec( 0.0f, 1.0f, 0.0f );
	D3DXVECTOR3 axisZVec( 0.0f, 0.0f, 1.0f );

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	Rotate( &targetVec, axisZVec );
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec( &targetVec, &axisYVec );
	shadowVec.z = vecDotVec( &targetVec, &axisZVec );
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler.x = 90.0f;
	}else{
		Euler.x = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );		
	}
	if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
		Euler.x = -Euler.x;
	}

	vec3RotateX( &tmpVec, -Euler.x, &targetVec );
	shadowVec.x = vecDotVec( &tmpVec, &axisXVec );
	shadowVec.y = 0.0f;
	shadowVec.z = vecDotVec( &tmpVec, &axisZVec );
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler.y = 90.0f;
	}else{
		Euler.y = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );
	}
	//if( vecDotVec( &shadowVec, &axisXVec ) < 0.0f ){
	if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
		Euler.y = -Euler.y;
	}


	Rotate( &targetVec, axisYVec );
	vec3RotateY( &tmpVec, -Euler.y, &targetVec );
	targetVec = tmpVec;
	vec3RotateX( &tmpVec, -Euler.x, &targetVec );
	shadowVec.x = vecDotVec( &tmpVec, &axisXVec );
	shadowVec.y = vecDotVec( &tmpVec, &axisYVec );
	shadowVec.z = 0.0f;
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler.z = 90.0f;
	}else{
		Euler.z = aCos( vecDotVec( &shadowVec, &axisYVec ) / lengthVec( &shadowVec ) );
	}
	//if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
	if( vecDotVec( &shadowVec, &axisXVec ) < 0.0f ){
		Euler.z = -Euler.z;
	}


	*reteul = Euler;

	return 0;
}

int CQuaternion::Q2EulZXY(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul)
{
	return Q2Eul(axisq, befeul, reteul);
}

int CQuaternion::Q2EulYXZ(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul)
{

	CQuaternion axisQ, invaxisQ, EQ;
	if (axisq){
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
		EQ = invaxisQ * *this * axisQ;
	}
	else{
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		EQ = *this;
	}

	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 axisYVec(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 axisZVec(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	EQ.Rotate(&targetVec, axisYVec);
	shadowVec.x = vecDotVec(&targetVec, &axisXVec);
	shadowVec.y = vecDotVec(&targetVec, &axisYVec);
	shadowVec.z = 0.0f;
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.z = 90.0f;
	}
	else{
		Euler.z = aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisXVec) > 0.0f){
		Euler.z = -Euler.z;
	}

	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec(&tmpVec, &axisYVec);
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.x = 90.0f;
	}
	else{
		Euler.x = aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisZVec) < 0.0f){
		Euler.x = -Euler.x;
	}


	EQ.Rotate(&targetVec, axisZVec);
	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	targetVec = tmpVec;
	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
	shadowVec.x = vecDotVec(&tmpVec, &axisXVec);
	shadowVec.y = 0.0f;
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.y = 90.0f;
	}
	else{
		Euler.y = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisXVec) < 0.0f){
		Euler.y = -Euler.y;
	}

	ModifyEuler(&Euler, &befeul);
	*reteul = Euler;

	return 0;
}


int CQuaternion::Q2EulXYZ(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul)
{

	CQuaternion axisQ, invaxisQ, EQ;
	if (axisq){
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
		EQ = invaxisQ * *this * axisQ;
	}
	else{
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		EQ = *this;
	}

	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 axisYVec(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 axisZVec(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	EQ.Rotate(&targetVec, axisXVec);
	shadowVec.x = vecDotVec(&targetVec, &axisXVec);
	shadowVec.y = vecDotVec(&targetVec, &axisYVec);
	shadowVec.z = 0.0f;
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.z = 90.0f;
	}
	else{
		Euler.z = aCos(vecDotVec(&shadowVec, &axisXVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) < 0.0f){
		Euler.z = -Euler.z;
	}

	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	shadowVec.x = vecDotVec(&tmpVec, &axisXVec);
	shadowVec.y = 0.0f;
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.y = 90.0f;
	}
	else{
		Euler.y = aCos(vecDotVec(&shadowVec, &axisXVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisZVec) > 0.0f){
		Euler.y = -Euler.y;
	}


	EQ.Rotate(&targetVec, axisZVec);
	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	targetVec = tmpVec;
	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec(&tmpVec, &axisYVec);
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.x = 90.0f;
	}
	else{
		Euler.x = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) > 0.0f){
		Euler.x = -Euler.x;
	}

	ModifyEuler(&Euler, &befeul);
	*reteul = Euler;

	return 0;
}



int CQuaternion::Q2Eul(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul)
{

	CQuaternion axisQ, invaxisQ, EQ;
	if (axisq){
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
		EQ = invaxisQ * *this * axisQ;
	}
	else{
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		EQ = *this;
	}

	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 axisYVec(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 axisZVec(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	EQ.Rotate(&targetVec, axisZVec);
	shadowVec.x = vecDotVec(&targetVec, &axisXVec);
	shadowVec.y = 0.0f;
	shadowVec.z = vecDotVec(&targetVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.y = 90.0f;
	}
	else{
		Euler.y = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisXVec) < 0.0f){
		Euler.y = -Euler.y;
	}

	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec(&tmpVec, &axisYVec);
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.x = 90.0f;
	}
	else{
		Euler.x = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) > 0.0f){
		Euler.x = -Euler.x;
	}


	EQ.Rotate(&targetVec, axisYVec);
	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
	targetVec = tmpVec;
	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
	shadowVec.x = vecDotVec(&tmpVec, &axisXVec);
	shadowVec.y = vecDotVec(&tmpVec, &axisYVec);
	shadowVec.z = 0.0f;
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.z = 90.0f;
	}
	else{
		Euler.z = aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisXVec) > 0.0f){
		Euler.z = -Euler.z;
	}

	ModifyEuler(&Euler, &befeul);
	*reteul = Euler;

	return 0;
}

//bullet角度０の軸を調整
int CQuaternion::Q2EulZYXbt(int needmodifyflag, CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul)
{

	CQuaternion axisQ, invaxisQ, EQ;
	if (axisq){
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
		EQ = invaxisQ * *this * axisQ;
	}
	else{
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		EQ = *this;
	}

	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 axisYVec(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 axisZVec(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	EQ.Rotate(&targetVec, axisZVec);
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec(&targetVec, &axisYVec);
	shadowVec.z = vecDotVec(&targetVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.x = 90.0f;
	}
	else{
		Euler.x = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) > 0.0f){
		Euler.x = -Euler.x;
	}

	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
	shadowVec.x = vecDotVec(&tmpVec, &axisXVec);
	shadowVec.y = 0;
	shadowVec.z = vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.y = 90.0f;
	}
	else{
		Euler.y = aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisXVec) < 0.0f){
		Euler.y = -Euler.y;
	}


	EQ.Rotate(&targetVec, axisXVec);
	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
	targetVec = tmpVec;
	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
	shadowVec.x = vecDotVec(&tmpVec, &axisXVec);
	shadowVec.y = vecDotVec(&tmpVec, &axisYVec);
	shadowVec.z = 0.0f;
	if (lengthVec(&shadowVec) == 0.0f){
		Euler.z = 90.0f;
	}
	else{
		Euler.z = aCos(vecDotVec(&shadowVec, &axisXVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) < 0.0f){
		Euler.z = -Euler.z;
	}

	if (needmodifyflag == 1){
		ModifyEuler(&Euler, &befeul);
	}
	*reteul = Euler;

	return 0;
}



int CQuaternion::ModifyEuler(D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB)
{

	//オイラー角Aの値をオイラー角Bの値に近い表示に修正
	float tmpX1, tmpY1, tmpZ1;
	float tmpX2, tmpY2, tmpZ2;
	float s1, s2;

	//予想される角度1
	tmpX1 = eulerA->x + 360.0f * GetRound((eulerB->x - eulerA->x) / 360.0f);
	tmpY1 = eulerA->y + 360.0f * GetRound((eulerB->y - eulerA->y) / 360.0f);
	tmpZ1 = eulerA->z + 360.0f * GetRound((eulerB->z - eulerA->z) / 360.0f);

	//予想される角度2
		//クォータニオンは１８０°で一回転する。
		//横軸が２シータ、縦軸がsin2シータ、cos2シータのグラフにおいて、newシータ　=　180 + oldシータの値は等しい。
		//tmp2の角度はクォータニオンにおいて等しい姿勢を取るオイラー角である。
		//この場合、３つの軸のうち１つだけの軸の角度の符号(ここではX軸)が反転する。
		//ということだと思う。テストすると合っている。
	tmpX2 = 180.0f - eulerA->x + 360.0f * GetRound((eulerB->x + eulerA->x - 180.0f) / 360.0f);
	tmpY2 = eulerA->y + 180.0f + 360.0f * GetRound((eulerB->y - eulerA->y - 180.0f) / 360.0f);
	tmpZ2 = eulerA->z + 180.0f + 360.0f * GetRound((eulerB->z - eulerA->z - 180.0f) / 360.0f);


	//角度変化の大きさ
	s1 = (eulerB->x - tmpX1) * (eulerB->x - tmpX1) + (eulerB->y - tmpY1) * (eulerB->y - tmpY1) + (eulerB->z - tmpZ1) * (eulerB->z - tmpZ1);
	s2 = (eulerB->x - tmpX2) * (eulerB->x - tmpX2) + (eulerB->y - tmpY2) * (eulerB->y - tmpY2) + (eulerB->z - tmpZ2) * (eulerB->z - tmpZ2);

	//変化の少ない方に修正
	if (s1 < s2){
		eulerA->x = tmpX1; eulerA->y = tmpY1; eulerA->z = tmpZ1;
	}
	else{
		eulerA->x = tmpX2; eulerA->y = tmpY2; eulerA->z = tmpZ2;
	}

	return 0;
}


/*
int CQuaternion::ModifyEuler( D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB )
{

	//オイラー角Aの値をオイラー角Bの値に近い表示に修正
	float tmpX1, tmpY1, tmpZ1;
	float tmpX2, tmpY2, tmpZ2;
	float s1, s2;

	//予想される角度1
	tmpX1 = eulerA->x + 360.0f * GetRound( (eulerB->x - eulerA->x) / 360.0f );
	tmpY1 = eulerA->y + 360.0f * GetRound( (eulerB->y - eulerA->y) / 360.0f );
	tmpZ1 = eulerA->z + 360.0f * GetRound( (eulerB->z - eulerA->z) / 360.0f );

	//予想される角度2
	tmpX2 = 180.0f - eulerA->x + 360.0f * GetRound( (eulerB->x + eulerA->x - 180.0f) / 360.0f );
	tmpY2 = eulerA->y + 180.0f + 360.0f * GetRound( (eulerB->y - eulerA->y - 180.0f) / 360.0f );
	tmpZ2 = eulerA->z + 180.0f + 360.0f * GetRound( (eulerB->z - eulerA->z - 180.0f) / 360.0f );

	//角度変化の大きさ
	s1 = (eulerB->x - tmpX1) * (eulerB->x - tmpX1) + (eulerB->y - tmpY1) * (eulerB->y - tmpY1) + (eulerB->z - tmpZ1) * (eulerB->z - tmpZ1);
	s2 = (eulerB->x - tmpX2) * (eulerB->x - tmpX2) + (eulerB->y - tmpY2) * (eulerB->y - tmpY2) + (eulerB->z - tmpZ2) * (eulerB->z - tmpZ2);

	//変化の少ない方に修正
	if( s1 < s2 ){
		eulerA->x = tmpX1; eulerA->y = tmpY1; eulerA->z = tmpZ1;
	}else{
		eulerA->x = tmpX2; eulerA->y = tmpY2; eulerA->z = tmpZ2;
	}

	return 0;
}
*/
int CQuaternion::GetRound( float srcval )
{
	if( srcval > 0.0f ){
		return (int)( srcval + 0.5f );
	}else{
		return (int)( srcval - 0.5f );
	}
}

int CQuaternion::CalcFBXEul( CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul, int isfirstbone )
{

	int noise[4] = { 0, 1, 0, -1 };
	static int dbgcnt = 0;

	D3DXVECTOR3 tmpeul( 0.0f, 0.0f, 0.0f );
	if (IsInit() == 0){
		//Q2Eul(axisq, befeul, &tmpeul);
		Q2EulXYZ(axisq, befeul, &tmpeul);
		//Q2EulYXZ(axisq, befeul, &tmpeul);
		//Q2EulZXY(axisq, befeul, &tmpeul);
	}
	else{
		//FBX書き出しの際にアニメーションに「ある程度の大きさの変化」がないとキーが省略されてしまう。
		//ノイズを乗せることでアニメーションがなくてもキーが省略されないようになる。
		//ノイズを乗せるのは１つのボーンで良い。
		if (isfirstbone == 1){
			tmpeul.x = (float)(noise[dbgcnt % 4]) / 100.0f;//!!!!!!!!
		}
		else{
			tmpeul.x = 0.0f;
		}
		tmpeul.y = 0.0f;
		tmpeul.z = 0.0f;
	}
	*reteul = tmpeul;

	dbgcnt++;

	return 0;
}

int CQuaternion::CalcFBXEulZXY(CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul, int isfirstbone)
{

	int noise[4] = { 0, 1, 0, -1 };
	static int dbgcnt = 0;

	D3DXVECTOR3 tmpeul(0.0f, 0.0f, 0.0f);
	if (IsInit() == 0){
		//Q2Eul(axisq, befeul, &tmpeul);
		//Q2EulXYZ(axisq, befeul, &tmpeul);
		//Q2EulYXZ(axisq, befeul, &tmpeul);
		Q2EulZXY(axisq, befeul, &tmpeul);
	}
	else{
		//FBX書き出しの際にアニメーションに「ある程度の大きさの変化」がないとキーが省略されてしまう。
		//ノイズを乗せることでアニメーションがなくてもキーが省略されないようになる。
		//ノイズを乗せるのは１つのボーンで良い。
		if (isfirstbone == 1){
			tmpeul.x = (float)(noise[dbgcnt % 4]) / 100.0f;//!!!!!!!!
		}
		else{
			tmpeul.x = 0.0f;
		}
		tmpeul.y = 0.0f;
		tmpeul.z = 0.0f;
	}
	*reteul = tmpeul;

	dbgcnt++;

	return 0;
}

int CQuaternion::IsInit()
{
	float dx, dy, dz, dw;
	dx = x - 0.0f;
	dy = y - 0.0f;
	dz = z - 0.0f;
	dw = w - 1.0f;

	if ((fabs(dx) <= 0.000001f) && (fabs(dy) <= 0.000001f) && (fabs(dz) <= 0.000001f) && (fabs(dw) <= 0.000001f)){
		return 1;
	}
	else{
		return 0;
	}
}

int CQuaternion::InOrder(CQuaternion* srcdstq)
{
	float kaku;
	kaku = CalcRad(*srcdstq);
	if (kaku > (PI * 0.5f)){
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		CQuaternion tmpq = -*srcdstq;
		*srcdstq = tmpq;
	}

	return 0;
}

void CQuaternion::MakeFromBtMat3x3(btMatrix3x3 eulmat)
{
	btVector3 tmpcol[3];
	int colno;
	for (colno = 0; colno < 3; colno++){
		tmpcol[colno] = eulmat.getColumn(colno);
	}

	D3DXMATRIX xmat;
	D3DXMatrixIdentity(&xmat);

	xmat._11 = tmpcol[0].x();
	xmat._12 = tmpcol[0].y();
	xmat._13 = tmpcol[0].z();

	xmat._21 = tmpcol[1].x();
	xmat._22 = tmpcol[1].y();
	xmat._23 = tmpcol[1].z();

	xmat._31 = tmpcol[2].x();
	xmat._32 = tmpcol[2].y();
	xmat._33 = tmpcol[2].z();

	this->RotationMatrix(xmat);

}
