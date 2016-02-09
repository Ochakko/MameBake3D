#include <windows.h>
#include <quaternion.h>
#include <math.h>
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
		return CQuaternion( 1e6, 1e6, 1e6, 1e6 );
}


D3DXMATRIX CQuaternion::MakeRotMatX()
{
	float dat00, dat01, dat02;
	float dat10, dat11, dat12;
	float dat20, dat21, dat22;

	dat00 = w * w + x * x - y * y - z * z;
	dat01 = 2.0f * ( x * y + w * z );
	dat02 = 2.0f * ( x * z - w * y );

	dat10 = 2.0f * ( x * y - w * z );
	dat11 = w * w - x * x + y * y - z * z;
	dat12 = 2.0f * ( y * z + w * x );

	dat20 = 2.0f * ( x * z + w * y );
	dat21 = 2.0f * ( y * z - w * x );
	dat22 = w * w - x * x - y * y + z * z;

	D3DXMATRIX retmat(
		dat00, dat01, dat02, 0.0f,
		dat10, dat11, dat12, 0.0f,
		dat20, dat21, dat22, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );

	return retmat;
}


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

	D3DXQUATERNION qx;
	D3DXQuaternionRotationMatrix( &qx, &tmatx );

	dstq->x = qx.x;	
	dstq->y = qx.y;	
	dstq->z = qx.z;	
	dstq->w = qx.w;	

	return 0;
}

D3DXMATRIX CQuaternion::CalcSymX( D3DXMATRIX srcmat )
{
	int deginvflag = 1;

	D3DXMATRIX mat0 = srcmat;
	mat0._41 = 0.0f;
	mat0._42 = 0.0f;
	mat0._43 = 0.0f;

	D3DXQUATERNION qx0;
	D3DXQuaternionRotationMatrix( &qx0, &mat0 );

	D3DXVECTOR3 symmaxis;
	float symmangle;
	D3DXQuaternionToAxisAngle( &qx0, &symmaxis, &symmangle );		
	if( deginvflag != 0 ){
		symmangle *= -1.0f;//!!!
	}

	if( (symmaxis.x == 0.0f) && (symmaxis.y == 0.0f) && (symmaxis.z == 0.0f) ){
		return mat0;//!!!!!!!!!!!!
	}
	if( (symmangle >= -100.0f) && (symmangle <= 100.0f) ){
		symmaxis.x *= -1.0f;

		D3DXQUATERNION newxq;
		D3DXQuaternionRotationAxis( &newxq, &symmaxis, symmangle );

		w = newxq.w;
		x = newxq.x;
		y = newxq.y;
		z = newxq.z;

		return MakeRotMatX();
	}else{
		//symmangleが非数値の時のフォロー
		return mat0;
	}
}



int CQuaternion::CalcSym( CQuaternion* dstq )
{
	int deginvflag = 1;

	D3DXQUATERNION symmxq;
	symmxq.w = w;
	symmxq.x = x;
	symmxq.y = y;
	symmxq.z = z;

	D3DXVECTOR3 symmaxis;
	float symmangle;

	D3DXQuaternionToAxisAngle( &symmxq, &symmaxis, &symmangle );		

	if( deginvflag != 0 ){
		symmangle *= -1.0f;//!!!
	}

	symmaxis.x *= -1.0f;

	D3DXQUATERNION newxq;
	D3DXQuaternionRotationAxis( &newxq, &symmaxis, symmangle );

	dstq->w = newxq.w;
	dstq->x = newxq.x;
	dstq->y = newxq.y;
	dstq->z = newxq.z;

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


int CQuaternion::Q2Eul( CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul )
{

	CQuaternion axisQ, invaxisQ, EQ;
	if( axisq ){
		axisQ = *axisq;
		axisQ.inv( &invaxisQ );
		EQ = invaxisQ * *this * axisQ;
	}else{
		axisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		invaxisQ.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		EQ = *this;
	}

	D3DXVECTOR3 Euler;


	D3DXVECTOR3 axisXVec( 1.0f, 0.0f, 0.0f );
	D3DXVECTOR3 axisYVec( 0.0f, 1.0f, 0.0f );
	D3DXVECTOR3 axisZVec( 0.0f, 0.0f, 1.0f );

	D3DXVECTOR3 targetVec, shadowVec;
	D3DXVECTOR3 tmpVec;

	EQ.Rotate( &targetVec, axisZVec );
	shadowVec.x = vecDotVec( &targetVec, &axisXVec );
	shadowVec.y = 0.0f;
	shadowVec.z = vecDotVec( &targetVec, &axisZVec );
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler.y = 90.0f;
	}else{
		Euler.y = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );		
	}
	if( vecDotVec( &shadowVec, &axisXVec ) < 0.0f ){
		Euler.y = -Euler.y;
	}

	vec3RotateY( &tmpVec, -Euler.y, &targetVec );
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec( &tmpVec, &axisYVec );
	shadowVec.z = vecDotVec( &tmpVec, &axisZVec );
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler.x = 90.0f;
	}else{
		Euler.x = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );
	}
	if( vecDotVec( &shadowVec, &axisYVec ) > 0.0f ){
		Euler.x = -Euler.x;
	}


	EQ.Rotate( &targetVec, axisYVec );
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
	if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
		Euler.z = -Euler.z;
	}

	ModifyEuler( &Euler, &befeul );
	*reteul = Euler;

	return 0;
}


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

int CQuaternion::GetRound( float srcval )
{
	if( srcval > 0.0f ){
		return (int)( srcval + 0.5f );
	}else{
		return (int)( srcval - 0.5f );
	}
}

int CQuaternion::CalcFBXEul( CQuaternion* axisq, D3DXVECTOR3 befeul, D3DXVECTOR3* reteul )
{

/***
	D3DXMATRIX rightmat;
	rightmat = MakeRotMatX();

	rightmat._31 *= -1;
	rightmat._32 *= -1;
	rightmat._34 *= -1;
	rightmat._13 *= -1;
	rightmat._23 *= -1;
	rightmat._43 *= -1;

	D3DXQUATERNION rqx;
	D3DXQuaternionRotationMatrix( &rqx, &rightmat );


	CQuaternion rq;
	rq.x = rqx.x;
	rq.y = rqx.y;
	rq.z = rqx.z;
	rq.w = rqx.w;

//	qToEulerAxis( axisq, &rq, reteul );
//	modifyEuler( reteul, &befeul );

	D3DXVECTOR3 tmpeul( 0.0f, 0.0f, 0.0f );
	rq.Q2Eul( 0, befeul, &tmpeul );
	*reteul = tmpeul;
***/

	D3DXVECTOR3 tmpeul( 0.0f, 0.0f, 0.0f );
	Q2Eul( 0, befeul, &tmpeul );
	*reteul = tmpeul;

	return 0;
}

