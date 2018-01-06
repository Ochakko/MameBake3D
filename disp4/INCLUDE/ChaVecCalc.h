#ifndef ChaCalcH
#define ChaCalcH

#include <d3dx9.h>


class ChaMatrix
{
public:
	ChaMatrix();
	ChaMatrix::ChaMatrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
	ChaMatrix::ChaMatrix(D3DXMATRIX m){
		_11 = m._11;
		_12 = m._12;
		_13 = m._13;
		_14 = m._14;
			
		_21 = m._21;
		_22 = m._22;
		_23 = m._23;
		_24 = m._24;

		_31 = m._31;
		_32 = m._32;
		_33 = m._33;
		_34 = m._34;

		_41 = m._41;
		_42 = m._42;
		_43 = m._43;
		_44 = m._44;
	};
	~ChaMatrix();

	ChaMatrix ChaMatrix::operator= (ChaMatrix m);
	ChaMatrix ChaMatrix::operator= (D3DXMATRIX m);
	ChaMatrix operator* (float srcw) const;
	ChaMatrix &operator*= (float srcw);
	ChaMatrix operator/ (float srcw) const;
	ChaMatrix &operator/= (float srcw);
	ChaMatrix operator+ (const ChaMatrix &m) const;
	ChaMatrix &operator+= (const ChaMatrix &m);
	ChaMatrix operator- (const ChaMatrix &m) const;
	ChaMatrix &operator-= (const ChaMatrix &m);
	ChaMatrix operator* (const ChaMatrix &m) const;
	ChaMatrix &operator*= (const ChaMatrix &m);
	ChaMatrix operator/ (const ChaMatrix &m) const;
	ChaMatrix &operator/= (const ChaMatrix &m);
	ChaMatrix operator- () const;

	bool ChaMatrix::operator== (const ChaMatrix &m) const {
		bool result;
		result = _11 == m._11 && _12 == m._12 && _13 == m._13 && _14 == m._14 &&
			_21 == m._21 && _22 == m._22 && _23 == m._23 && _24 == m._24 &&
			_31 == m._31 && _32 == m._32 && _33 == m._33 && _34 == m._34 &&
			_41 == m._41 && _42 == m._42 && _43 == m._43 && _44 == m._44;
		return result;
	};
	bool ChaMatrix::operator!= (const ChaMatrix &m) const {
		return !(*this == m);
	};

	D3DXMATRIX D3DX();

public:
	float _11;
	float _12;
	float _13;
	float _14;

	float _21;
	float _22;
	float _23;
	float _24;

	float _31;
	float _32;
	float _33;
	float _34;

	float _41;
	float _42;
	float _43;
	float _44;

};


class ChaVector3
{
public:

	ChaVector3();
	ChaVector3(float srcx, float srcy, float srcz);
	~ChaVector3();

	ChaVector3 ChaVector3::operator= (ChaVector3 v);
	ChaVector3 operator* (float srcw) const;
	ChaVector3 &operator*= (float srcw);
	ChaVector3 operator/ (float srcw) const;
	ChaVector3 &operator/= (float srcw);
	ChaVector3 operator+ (const ChaVector3 &v) const;
	ChaVector3 &operator+= (const ChaVector3 &v);
	ChaVector3 operator- (const ChaVector3 &v) const;
	ChaVector3 &operator-= (const ChaVector3 &v);
	ChaVector3 operator* (const ChaVector3 &v) const;
	ChaVector3 &operator*= (const ChaVector3 &v);
	//ChaVector3 operator/ (const ChaVector3 &q) const;
	//ChaVector3 &operator/= (const ChaVector3 &q);
	ChaVector3 operator- () const;

	bool operator== (const ChaVector3 &v) const { return x == v.x && y == v.y && z == v.z; };
	bool operator!= (const ChaVector3 &v) const { return !(*this == v); };


	D3DXVECTOR3 D3DX();

public:
	float x;
	float y;
	float z;

};



#ifdef CHACALCCPP
float ChaVector3Length(ChaVector3* psrc);
void ChaVector3Normalize(ChaVector3* pdst, ChaVector3* psrc);
float ChaVector3Dot(ChaVector3* psrc1, ChaVector3* psrc2);
void ChaVector3Cross(ChaVector3* pdst, ChaVector3* psrc1, ChaVector3* psrc2);
void ChaVector3TransformCoord(ChaVector3* pdst, ChaVector3* psrc, ChaMatrix* pmat);

void ChaMatrixIdentity(ChaMatrix* pdst);
void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, ChaMatrix* psrc);
void ChaMatrixTranslation(ChaMatrix* pdst, float srcx, float srcy, float srcz);
void ChaMatrixTranspose(ChaMatrix* pdst, ChaMatrix* psrc);
void ChaMatrixRotationAxis(ChaMatrix* pdst, ChaVector3* srcaxis, float srcrad);
void ChaMatrixScaling(ChaMatrix* pdst, float srcx, float srcy, float srcz);
#else
extern float ChaVector3Length(ChaVector3* psrc);
extern void ChaVector3Normalize(ChaVector3* pdst, ChaVector3* psrc);
extern float ChaVector3Dot(ChaVector3* psrc1, ChaVector3* psrc2);
extern void ChaVector3Cross(ChaVector3* pdst, ChaVector3* psrc1, ChaVector3* psrc2);
extern void ChaVector3TransformCoord(ChaVector3* pdst, ChaVector3* psrc, ChaMatrix* pmat);

extern void ChaMatrixIdentity(ChaMatrix* pdst);
extern void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, ChaMatrix* psrc);
extern void ChaMatrixTranslation(ChaMatrix* pdst, float srcx, float srcy, float srcz);
extern void ChaMatrixTranspose(ChaMatrix* pdst, ChaMatrix* psrc);
extern void ChaMatrixRotationAxis(ChaMatrix* pdst, ChaVector3* srcaxis, float srcrad);
extern void ChaMatrixScaling(ChaMatrix* pdst, float srcx, float srcy, float srcz);
#endif



#endif



