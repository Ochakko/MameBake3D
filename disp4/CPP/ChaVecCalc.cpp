#include <windows.h>
#include <math.h>

#define CHACALCCPP
#include <ChaVecCalc.h>
#include <Quaternion.h>

#include <crtdbg.h>


ChaVector3::ChaVector3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

ChaVector3::ChaVector3(float srcx, float srcy, float srcz)
{
	x = srcx;
	y = srcy;
	z = srcz;
}

ChaVector3::~ChaVector3()
{

}


ChaVector3 ChaVector3::operator= (ChaVector3 v) { this->x = v.x; this->y = v.y; this->z = v.z; return *this; };
ChaVector3 ChaVector3::operator* (float srcw) const { return ChaVector3(this->x * srcw, this->y * srcw, this->z * srcw); }
ChaVector3 &ChaVector3::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaVector3 ChaVector3::operator/ (float srcw) const { return ChaVector3(this->x / srcw, this->y / srcw, this->z / srcw); }
ChaVector3 &ChaVector3::operator/= (float srcw) { *this = *this / srcw; return *this; }
ChaVector3 ChaVector3::operator+ (const ChaVector3 &v) const { return ChaVector3(x + v.x, y + v.y, z + v.z); }
ChaVector3 &ChaVector3::operator+= (const ChaVector3 &v) { *this = *this + v; return *this; }
ChaVector3 ChaVector3::operator- (const ChaVector3 &v) const { return ChaVector3(x - v.x, y - v.y, z - v.z); }
ChaVector3 &ChaVector3::operator-= (const ChaVector3 &v) { *this = *this - v; return *this; }

ChaVector3 ChaVector3::operator- () const { return *this * -1.0f; }



ChaVector4::ChaVector4()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}

ChaVector4::ChaVector4(float srcx, float srcy, float srcz, float srcw)
{
	x = srcx;
	y = srcy;
	z = srcz;
	w = srcw;
}

ChaVector4::~ChaVector4()
{

}


ChaVector4 ChaVector4::operator= (ChaVector4 v) { this->x = v.x; this->y = v.y; this->z = v.z; this->w = v.w; return *this; };
ChaVector4 ChaVector4::operator* (float srcw) const { return ChaVector4(this->x * srcw, this->y * srcw, this->z * srcw, this->w * srcw); }
ChaVector4 &ChaVector4::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaVector4 ChaVector4::operator/ (float srcw) const { return ChaVector4(this->x / srcw, this->y / srcw, this->z / srcw, this->w / srcw); }
ChaVector4 &ChaVector4::operator/= (float srcw) { *this = *this / srcw; return *this; }
ChaVector4 ChaVector4::operator+ (const ChaVector4 &v) const { return ChaVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
ChaVector4 &ChaVector4::operator+= (const ChaVector4 &v) { *this = *this + v; return *this; }
ChaVector4 ChaVector4::operator- (const ChaVector4 &v) const { return ChaVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
ChaVector4 &ChaVector4::operator-= (const ChaVector4 &v) { *this = *this - v; return *this; }

ChaVector4 ChaVector4::operator- () const { return *this * -1.0f; }


ChaMatrix::ChaMatrix()
{
	_11 = 1.0f;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;

	_21 = 0.0f;
	_22 = 1.0f;
	_23 = 0.0f;
	_24 = 0.0f;

	_31 = 0.0f;
	_32 = 0.0f;
	_33 = 1.0f;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;

}

ChaMatrix::ChaMatrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
{
	_11 = m11;
	_12 = m12;
	_13 = m13;
	_14 = m14;

	_21 = m21;
	_22 = m22;
	_23 = m23;
	_24 = m24;

	_31 = m31;
	_32 = m32;
	_33 = m33;
	_34 = m34;

	_41 = m41;
	_42 = m42;
	_43 = m43;
	_44 = m44;

}

ChaMatrix::~ChaMatrix()
{


}



ChaMatrix ChaMatrix::operator= (ChaMatrix m) { 
	this->_11 = m._11; 
	this->_12 = m._12; 
	this->_13 = m._13;
	this->_14 = m._14;
	
	this->_21 = m._21;
	this->_22 = m._22;
	this->_23 = m._23;
	this->_24 = m._24;

	this->_31 = m._31;
	this->_32 = m._32;
	this->_33 = m._33;
	this->_34 = m._34;

	this->_41 = m._41;
	this->_42 = m._42;
	this->_43 = m._43;
	this->_44 = m._44;

	return *this;
};



ChaMatrix ChaMatrix::operator* (float srcw) const { return ChaMatrix(this->_11 * srcw, this->_12 * srcw, this->_13 * srcw, this->_14 * srcw,
	this->_21 * srcw, this->_22 * srcw, this->_23 * srcw, this->_24 * srcw,
	this->_31 * srcw, this->_32 * srcw, this->_33 * srcw, this->_34 * srcw,
	this->_41 * srcw, this->_42 * srcw, this->_43 * srcw, this->_44 * srcw);
}
ChaMatrix &ChaMatrix::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaMatrix ChaMatrix::operator/ (float srcw) const { 
	return ChaMatrix(this->_11 / srcw, this->_12 / srcw, this->_13 / srcw, this->_14 / srcw,
		this->_21 / srcw, this->_22 / srcw, this->_23 / srcw, this->_24 / srcw,
		this->_31 / srcw, this->_32 / srcw, this->_33 / srcw, this->_34 / srcw,
		this->_41 / srcw, this->_42 / srcw, this->_43 / srcw, this->_44 / srcw);
}
ChaMatrix &ChaMatrix::operator/= (float srcw) { *this = *this / srcw; return *this; }

ChaMatrix ChaMatrix::operator+ (const ChaMatrix &m) const { 
	return ChaMatrix(
		_11 + m._11, _12 + m._12, _13 + m._13, _14 + m._14,
		_21 + m._21, _22 + m._22, _23 + m._23, _24 + m._24,
		_31 + m._31, _32 + m._32, _33 + m._33, _34 + m._34,
		_41 + m._41, _42 + m._42, _43 + m._43, _44 + m._44
		);
}
ChaMatrix &ChaMatrix::operator+= (const ChaMatrix &m) { *this = *this + m; return *this; }

ChaMatrix ChaMatrix::operator- (const ChaMatrix &m) const { 
	return ChaMatrix(
	_11 - m._11, _12 - m._12, _13 - m._13, _14 - m._14,
	_21 - m._21, _22 - m._22, _23 - m._23, _24 - m._24,
	_31 - m._31, _32 - m._32, _33 - m._33, _34 - m._34,
	_41 - m._41, _42 - m._42, _43 - m._43, _44 - m._44
	); }
ChaMatrix &ChaMatrix::operator-= (const ChaMatrix &m) { *this = *this - m; return *this; }


ChaMatrix ChaMatrix::operator* (const ChaMatrix &m) const {
	//*this * m
	ChaMatrix res;
	res._11 = m._11 * _11 + m._21 * _12 + m._31 * _13 + m._41 * _14;
	res._21 = m._11 * _21 + m._21 * _22 + m._31 * _23 + m._41 * _24;
	res._31 = m._11 * _31 + m._21 * _32 + m._31 * _33 + m._41 * _34;
	res._41 = m._11 * _41 + m._21 * _42 + m._31 * _43 + m._41 * _44;

	res._12 = m._12 * _11 + m._22 * _12 + m._32 * _13 + m._42 * _14;
	res._22 = m._12 * _21 + m._22 * _22 + m._32 * _23 + m._42 * _24;
	res._32 = m._12 * _31 + m._22 * _32 + m._32 * _33 + m._42 * _34;
	res._42 = m._12 * _41 + m._22 * _42 + m._32 * _43 + m._42 * _44;

	res._13 = m._13 * _11 + m._23 * _12 + m._33 * _13 + m._43 * _14;
	res._23 = m._13 * _21 + m._23 * _22 + m._33 * _23 + m._43 * _24;
	res._33 = m._13 * _31 + m._23 * _32 + m._33 * _33 + m._43 * _34;
	res._43 = m._13 * _41 + m._23 * _42 + m._33 * _43 + m._43 * _44;

	res._14 = m._14 * _11 + m._24 * _12 + m._34 * _13 + m._44 * _14;
	res._24 = m._14 * _21 + m._24 * _22 + m._34 * _23 + m._44 * _24;
	res._34 = m._14 * _31 + m._24 * _32 + m._34 * _33 + m._44 * _34;
	res._44 = m._14 * _41 + m._24 * _42 + m._34 * _43 + m._44 * _44;

	return res;
}

ChaMatrix &ChaMatrix::operator*= (const ChaMatrix &m) { *this = *this * m; return *this; }
ChaMatrix ChaMatrix::operator- () const { return *this * -1.0f; }

void ChaMatrixIdentity(ChaMatrix* pdst)
{
	if (!pdst){
		return;
	}
	pdst->_11 = 1.0f;
	pdst->_12 = 0.0f;
	pdst->_13 = 0.0f;
	pdst->_14 = 0.0f;

	pdst->_21 = 0.0f;
	pdst->_22 = 1.0f;
	pdst->_23 = 0.0f;
	pdst->_24 = 0.0f;

	pdst->_31 = 0.0f;
	pdst->_32 = 0.0f;
	pdst->_33 = 1.0f;
	pdst->_34 = 0.0f;

	pdst->_41 = 0.0f;
	pdst->_42 = 0.0f;
	pdst->_43 = 0.0f;
	pdst->_44 = 1.0f;
}

void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, ChaMatrix* psrc)
{
	if (!pdst || !psrc){
		return;
	}

	ChaMatrix res;
	double detA;
	float a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41, a42, a43, a44;
	float b11, b12, b13, b14, b21, b22, b23, b24, b31, b32, b33, b34, b41, b42, b43, b44;

	a11 = psrc->_11;
	a12 = psrc->_12;
	a13 = psrc->_13;
	a14 = psrc->_14;

	a21 = psrc->_21;
	a22 = psrc->_22;
	a23 = psrc->_23;
	a24 = psrc->_24;

	a31 = psrc->_31;
	a32 = psrc->_32;
	a33 = psrc->_33;
	a34 = psrc->_34;

	a41 = psrc->_41;
	a42 = psrc->_42;
	a43 = psrc->_43;
	a44 = psrc->_44;

	detA = (a11 * a22 * a33 * a44) + (a11 * a23 * a34 * a42) + (a11 * a24 * a32 * a43)
		+ (a12 * a21 * a34 * a43) + (a12 * a23 * a31 * a44) + (a12 * a24 * a33 * a41)
		+ (a13 * a21 * a32 * a44) + (a13 * a22 * a34 * a41) + (a13 * a24 * a31 * a42)
		+ (a14 * a21 * a33 * a42) + (a14 * a22 * a31 * a43) + (a14 * a23 * a32 * a41)
		- (a11 * a22 * a34 * a43) - (a11 * a23 * a32 * a44) - (a11 * a24 * a33 * a42)
		- (a12 * a21 * a33 * a44) - (a12 * a23 * a34 * a41) - (a12 * a24 * a31 * a43)
		- (a13 * a21 * a34 * a42) - (a13 * a22 * a31 * a44) - (a13 * a24 * a32 * a41)
		- (a14 * a21 * a32 * a43) - (a14 * a22 * a33 * a41) - (a14 * a23 * a31 * a42);

	if (pdet){
		*pdet = (float)detA;
	}


	if (detA == 0.0){
		*pdst = *psrc;
		return;//!!!!!!!!!!!!!!!!!!!!!!!!!!
	}




	b11 = (a22 * a33 * a44) + (a23 * a34 * a42) + (a24 * a32 * a43) - (a22 * a34 * a43) - (a23 * a32 * a44) - (a24 * a33 * a42);
	b12 = (a12 * a34 * a43) + (a13 * a32 * a44) + (a14 * a33 * a42) - (a12 * a33 * a44) - (a13 * a34 * a42) - (a14 * a32 * a43);
	b13 = (a12 * a23 * a44) + (a13 * a24 * a42) + (a14 * a22 * a43) - (a12 * a24 * a43) - (a13 * a22 * a44) - (a14 * a23 * a42);
	b14 = (a12 * a24 * a33) + (a13 * a22 * a34) + (a14 * a23 * a32) - (a12 * a23 * a34) - (a13 * a24 * a32) - (a14 * a22 * a33);

	b21 = (a21 * a34 * a43) + (a23 * a31 * a44) + (a24 * a33 * a41) - (a21 * a33 * a44) - (a23 * a34 * a41) - (a24 * a31 * a43);
	b22 = (a11 * a33 * a44) + (a13 * a34 * a41) + (a14 * a31 * a43) - (a11 * a34 * a43) - (a13 * a31 * a44) - (a14 * a33 * a41);
	b23 = (a11 * a24 * a43) + (a13 * a21 * a44) + (a14 * a23 * a41) - (a11 * a23 * a44) - (a13 * a24 * a41) - (a14 * a21 * a43);
	b24 = (a11 * a23 * a34) + (a13 * a24 * a31) + (a14 * a21 * a33) - (a11 * a24 * a33) - (a13 * a21 * a34) - (a14 * a23 * a31);

	b31 = (a21 * a32 * a44) + (a22 * a34 * a41) + (a24 * a31 * a42) - (a21 * a34 * a42) - (a22 * a31 * a44) - (a24 * a32 * a41);
	b32 = (a11 * a34 * a42) + (a12 * a31 * a44) + (a14 * a32 * a41) - (a11 * a32 * a44) - (a12 * a34 * a41) - (a14 * a31 * a42);
	b33 = (a11 * a22 * a44) + (a12 * a24 * a41) + (a14 * a21 * a42) - (a11 * a24 * a42) - (a12 * a21 * a44) - (a14 * a22 * a41);
	b34 = (a11 * a24 * a32) + (a12 * a21 * a34) + (a14 * a22 * a31) - (a11 * a22 * a34) - (a12 * a24 * a31) - (a14 * a21 * a32);

	b41 = (a21 * a33 * a42) + (a22 * a31 * a43) + (a23 * a32 * a41) - (a21 * a32 * a43) - (a22 * a33 * a41) - (a23 * a31 * a42);
	b42 = (a11 * a32 * a43) + (a12 * a33 * a41) + (a13 * a31 * a42) - (a11 * a33 * a42) - (a12 * a31 * a43) - (a13 * a32 * a41);
	b43 = (a11 * a23 * a42) + (a12 * a21 * a43) + (a13 * a22 * a41) - (a11 * a22 * a43) - (a12 * a23 * a41) - (a13 * a21 * a42);
	b44 = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a11 * a23 * a32) - (a12 * a21 * a33) - (a13 * a22 * a31);

	res._11 = b11 / detA;
	res._12 = b12 / detA;
	res._13 = b13 / detA;
	res._14 = b14 / detA;

	res._21 = b21 / detA;
	res._22 = b22 / detA;
	res._23 = b23 / detA;
	res._24 = b24 / detA;

	res._31 = b31 / detA;
	res._32 = b32 / detA;
	res._33 = b33 / detA;
	res._34 = b34 / detA;

	res._41 = b41 / detA;
	res._42 = b42 / detA;
	res._43 = b43 / detA;
	res._44 = b44 / detA;

	*pdst = res;
}

void ChaMatrixTranslation(ChaMatrix* pdst, float srcx, float srcy, float srcz)
{
	if (!pdst){
		return;
	}

	pdst->_41 = srcx;
	pdst->_42 = srcy;
	pdst->_43 = srcz;
}

void ChaMatrixTranspose(ChaMatrix* pdst, ChaMatrix* psrc)
{
	float m[4][4];

	m[0][0] = psrc->_11;
	m[0][1] = psrc->_12;
	m[0][2] = psrc->_13;
	m[0][3] = psrc->_14;
	m[1][0] = psrc->_21;
	m[1][1] = psrc->_22;
	m[1][2] = psrc->_23;
	m[1][3] = psrc->_24;
	m[2][0] = psrc->_31;
	m[2][1] = psrc->_32;
	m[2][2] = psrc->_33;
	m[2][3] = psrc->_34;
	m[3][0] = psrc->_41;
	m[3][1] = psrc->_42;
	m[3][2] = psrc->_43;
	m[3][3] = psrc->_44;

	pdst->_11 = m[0][0];
	pdst->_12 = m[1][0];
	pdst->_13 = m[2][0];
	pdst->_14 = m[3][0];

	pdst->_21 = m[0][1];
	pdst->_22 = m[1][1];
	pdst->_23 = m[2][1];
	pdst->_24 = m[3][1];

	pdst->_31 = m[0][2];
	pdst->_32 = m[1][2];
	pdst->_33 = m[2][2];
	pdst->_34 = m[3][2];

	pdst->_41 = m[0][3];
	pdst->_42 = m[1][3];
	pdst->_43 = m[2][3];
	pdst->_44 = m[3][3];

}
float ChaVector3Length(ChaVector3* v)
{
	if (!v){
		return 0.0f;
	}

	float leng;
	double mag;
	mag = v->x * v->x + v->y * v->y + v->z * v->z;
	if (mag != 0.0f){
		leng = (float)sqrt(mag);
	}
	else{
		leng = 0.0f;
	}
	return leng;

}

void ChaVector3Normalize(ChaVector3* pdst, ChaVector3* psrc){
	if (!pdst || !psrc){
		return;
	}

	ChaVector3 src = *psrc;

	double mag = src.x * src.x + src.y * src.y + src.z * src.z;
	if (mag != 0.0){
		double divval = ::sqrt(mag);
		double tmpx = src.x / divval;
		double tmpy = src.y / divval;
		double tmpz = src.z / divval;
		pdst->x = (float)tmpx;
		pdst->y = (float)tmpy;
		pdst->z = (float)tmpz;
	}
	else{
		*pdst = ChaVector3(0.0f, 0.0f, 0.0f);
	}
}

float ChaVector3Dot(ChaVector3* psrc1, ChaVector3* psrc2)
{
	if (!psrc1 || !psrc2){
		return 0.0f;
	}

	return (psrc1->x * psrc2->x + psrc1->y * psrc2->y + psrc1->z * psrc2->z);

}

void ChaVector3Cross(ChaVector3* pdst, ChaVector3* psrc1, ChaVector3* psrc2)
{
	if (!pdst || !psrc1 || !psrc2){
		return;
	}

	ChaVector3 v1, v2;
	v1 = *psrc1;
	v2 = *psrc2;

	pdst->x = v1.y * v2.z - v1.z * v2.y;
	pdst->y = v1.z * v2.x - v1.x * v2.z;
	pdst->z = v1.x * v2.y - v1.y * v2.x;
}


void ChaVector3TransformCoord(ChaVector3* dstvec, ChaVector3* srcvec, ChaMatrix* srcmat)
{
	if (!dstvec || !srcvec || !srcmat){
		return;
	}

	double tmpx, tmpy, tmpz, tmpw;
	tmpx = srcmat->_11 * srcvec->x + srcmat->_21 * srcvec->y + srcmat->_31 * srcvec->z + srcmat->_41;
	tmpy = srcmat->_12 * srcvec->x + srcmat->_22 * srcvec->y + srcmat->_32 * srcvec->z + srcmat->_42;
	tmpz = srcmat->_13 * srcvec->x + srcmat->_23 * srcvec->y + srcmat->_33 * srcvec->z + srcmat->_43;
	tmpw = srcmat->_14 * srcvec->x + srcmat->_24 * srcvec->y + srcmat->_34 * srcvec->z + srcmat->_44;

	if (tmpw != 0.0){
		dstvec->x = tmpx / tmpw;
		dstvec->y = tmpy / tmpw;
		dstvec->z = tmpz / tmpw;
	}
	else{
		dstvec->x = 0.0f;
		dstvec->y = 0.0f;
		dstvec->z = 0.0f;
	}
}


void ChaMatrixRotationAxis(ChaMatrix* pdst, ChaVector3* srcaxis, float srcrad)
{
	if (!pdst){
		return;
	}
	CQuaternion q;
	q.SetAxisAndRot(*srcaxis, srcrad);

	*pdst = q.MakeRotMatX();

}

void ChaMatrixScaling(ChaMatrix* pdst, float srcx, float srcy, float srcz)
{
	if (!pdst){
		return;
	}

	pdst->_11 = srcx;
	pdst->_22 = srcy;
	pdst->_33 = srcz;
}



#ifdef CONVD3DX9
D3DXVECTOR3 ChaVector3::D3DX()
{
	D3DXVECTOR3 retv;
	retv.x = x;
	retv.y = y;
	retv.z = z;
	return retv;
}

D3DXVECTOR4 ChaVector4::D3DX()
{
	D3DXVECTOR4 retv;
	retv.x = x;
	retv.y = y;
	retv.z = z;
	retv.w = w;
	return retv;
}


ChaMatrix ChaMatrix::operator= (D3DXMATRIX m) {
	this->_11 = m._11;
	this->_12 = m._12;
	this->_13 = m._13;
	this->_14 = m._14;

	this->_21 = m._21;
	this->_22 = m._22;
	this->_23 = m._23;
	this->_24 = m._24;

	this->_31 = m._31;
	this->_32 = m._32;
	this->_33 = m._33;
	this->_34 = m._34;

	this->_41 = m._41;
	this->_42 = m._42;
	this->_43 = m._43;
	this->_44 = m._44;

	return *this;
};

D3DXMATRIX ChaMatrix::D3DX()
{
	D3DXMATRIX retm;
	retm._11 = _11;
	retm._12 = _12;
	retm._13 = _13;
	retm._14 = _14;

	retm._21 = _21;
	retm._22 = _22;
	retm._23 = _23;
	retm._24 = _24;

	retm._31 = _31;
	retm._32 = _32;
	retm._33 = _33;
	retm._34 = _34;

	retm._41 = _41;
	retm._42 = _42;
	retm._43 = _43;
	retm._44 = _44;

	return retm;
}
#endif
