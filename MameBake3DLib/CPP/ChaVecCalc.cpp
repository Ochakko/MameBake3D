#include "stdafx.h"
#include <windows.h>
#include <math.h>


//#ifdef CONVD3DX11
//#include <d3dcommon.h>
//#include <dxgi.h>
//#include <d3d10_1.h>
//#include <d3d10.h>
//#include <d3dcompiler.h>
//#include <d3dx10.h>
//#endif

#define CHACALCCPP
#include <ChaVecCalc.h>


#include "../Examples/CommonInterfaces/CommonExampleInterface.h"
#include "../Examples/CommonInterfaces/CommonGUIHelperInterface.h"
#include "../BTSRC/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "../BTSRC/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "../BTSRC/BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h"


#include "../BTSRC/LinearMath/btTransform.h"
#include "../BTSRC/LinearMath/btHashMap.h"
//#include "../Examples/MultiThreadedDemo/CommonRigidBodyMTBase.h"
//#include "../Examples/CommonInterfaces/CommonMultiBodyBase.h"

#include "../Examples/CommonInterfaces/CommonRigidBodyBase.h"
#include "../Examples/MultiThreadedDemo/CommonRigidBodyMTBase.h"


#include "../BTSRC/btBulletDynamicsCommon.h"
#include "../BTSRC/LinearMath/btIDebugDraw.h"

#include <math.h>

#include <iostream>
#include <vector>
#include <algorithm>



#include <crtdbg.h>

extern bool g_wmatDirectSetFlag;//!!!!!!!!!!!!




//########################################################################################################
// 	ここから　//https://lxjk.github.io/2020/02/07/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained-JP.html
//########################################################################################################
#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)

// for row major matrix
// we use __m128 to represent 2x2 matrix as A = | A0  A1 |
//                                              | A2  A3 |
// 2x2 row major Matrix multiply A*B
__forceinline __m128 Mat2Mul(__m128 vec1, __m128 vec2)
{
	return
		_mm_add_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 0, 3, 0, 3)),
			_mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}
// 2x2 row major Matrix adjugate multiply (A#)*B
__forceinline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3, 3, 0, 0), vec2),
			_mm_mul_ps(VecSwizzle(vec1, 1, 1, 2, 2), VecSwizzle(vec2, 2, 3, 0, 1)));

}
// 2x2 row major Matrix multiply adjugate A*(B#)
__forceinline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 3, 0, 3, 0)),
			_mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}
//########################################################################################################
// 	ここまで　//https://lxjk.github.io/2020/02/07/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained-JP.html
//########################################################################################################


//###########################################################################################################
// ここから　https://www.isus.jp/games/using-simd-technologies-on-intel-architecture-to-speed-up-game-code/
//###########################################################################################################
#define SHUFFLE_PARAM(x, y, z, w) ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
//同じ要素の値を同時に計算するため、4 つの同じ単精度浮動小数点値を 1 つの __m128 データに設定します。

#define _mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))


#define _mm_replicate_x_ps(v) _mm_shuffle_ps((v), (v), SHUFFLE_PARAM(0, 0, 0, 0))

#define _mm_replicate_y_ps(v) _mm_shuffle_ps((v), (v), SHUFFLE_PARAM(1, 1, 1, 1))

#define _mm_replicate_z_ps(v) _mm_shuffle_ps((v), (v), SHUFFLE_PARAM(2, 2, 2, 2))

#define _mm_replicate_w_ps(v) _mm_shuffle_ps((v), (v), SHUFFLE_PARAM(3, 3, 3, 3))
//###########################################################################################################
// ここまで　https://www.isus.jp/games/using-simd-technologies-on-intel-architecture-to-speed-up-game-code/
//###########################################################################################################


								//BoneProp

double vecDotVec(ChaVector3* vec1, ChaVector3* vec2)
{
	return ((double)vec1->x * (double)vec2->x + (double)vec1->y * (double)vec2->y + (double)vec1->z * (double)vec2->z);
}

double lengthVec(ChaVector3* vec)
{
	double mag;
	double leng;

	mag = (double)vec->x * (double)vec->x + (double)vec->y * (double)vec->y + (double)vec->z * (double)vec->z;
	if (mag == 0.0) {
		leng = 0.0;
	}
	else {
		leng = sqrt(mag);
	}
	return leng;
}

double aCos(double dot)
{
	if (dot > 1.0)
		dot = 1.0;
	else if (dot < -1.0)
		dot = -1.0;

	double rad;
	rad = acos(dot);

	double degree;
	degree = (rad * PAI2DEG);

	return degree;
}

int vec3RotateY(ChaVector3* dstvec, double deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion iniq;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ret = dirq.SetRotationXYZ(&iniq, 0, deg, 0);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	ChaVector3 tmpsrcvec = *srcvec;//dstvecとsrcvecが同一ポインタの場合に対応
	
	ChaVector3TransformCoord(dstvec, &tmpsrcvec, &dirm);
	//dstvec->x = (float)((double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12]);
	//dstvec->y = (float)((double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13]);
	//dstvec->z = (float)((double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14]);

	return 0;
}
int vec3RotateX(ChaVector3* dstvec, double deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion iniq;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ret = dirq.SetRotationXYZ(&iniq, deg, 0, 0);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	ChaVector3 tmpsrcvec = *srcvec;//dstvecとsrcvecが同一ポインタの場合に対応

	ChaVector3TransformCoord(dstvec, &tmpsrcvec, &dirm);
	//dstvec->x = (float)((double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12]);
	//dstvec->y = (float)((double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13]);
	//dstvec->z = (float)((double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14]);


	return 0;
}
int vec3RotateZ(ChaVector3* dstvec, float deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion iniq;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ret = dirq.SetRotationXYZ(&iniq, 0, 0, deg);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	ChaVector3 tmpsrcvec = *srcvec;//dstvecとsrcvecが同一ポインタの場合に対応

	ChaVector3TransformCoord(dstvec, &tmpsrcvec, &dirm);
	//dstvec->x = (float)((double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12]);
	//dstvec->y = (float)((double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13]);
	//dstvec->z = (float)((double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14]);

	return 0;
}


//int qToEulerAxis(CQuaternion axisQ, CQuaternion* srcq, ChaVector3* Euler)
//{
//
//	int ret;
//	CQuaternion invaxisQ, EQ;
//	ret = axisQ.inv(&invaxisQ);
//	_ASSERT(!ret);
//	EQ = invaxisQ * *srcq * axisQ;
//
//
//	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
//	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
//	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);
//
//	ChaVector3 targetVec, shadowVec;
//	ChaVector3 tmpVec;
//
//	EQ.Rotate(&targetVec, axisZVec);
//	shadowVec.x = (float)vecDotVec(&targetVec, &axisXVec);
//	shadowVec.y = 0.0f;
//	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler->y = 90.0f;
//	}
//	else {
//		Euler->y = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) < 0.0) {
//		Euler->y = -Euler->y;
//	}
//
//	/***
//	E3DMultQVec targetQ,0.0,0.0,1.0,targetVec.0,targetVec.1,targetVec.2
//	shadowVec.0= vecDotVec(targetVec,axisXVec)
//	shadowVec.1= 0.0
//	shadowVec.2= vecDotVec(targetVec,axisZVec)
//	if( lengthVec(shadowVec) == 0 ){
//	eulerY= 90.0
//	}else{
//	E3DACos ( vecDotVec(shadowVec,axisZVec)/ lengthVec(shadowVec) ), eulerY		//Y軸回転度数
//	}
//	if( vecDotVec(shadowVec,axisXVec) < 0.0 ){ eulerY= -eulerY }				//右回りに修正
//	***/
//	vec3RotateY(&tmpVec, -Euler->y, &targetVec);
//	shadowVec.x = 0.0f;
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler->x = 90.0f;
//	}
//	else {
//		Euler->x = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisYVec) > 0.0) {
//		Euler->x = -Euler->x;
//	}
//	/***
//	E3DVec3RotateY targetVec.0,targetVec.1,targetVec.2,(-eulerY),shadowVec.0,shadowVec.1,shadowVec.2
//	if( lengthVec(shadowVec) == 0 ){
//	eulerX= 90.0
//	}else{
//	E3DACos ( vecDotVec(shadowVec,axisZVec)/ lengthVec(shadowVec) ), eulerX		//X軸回転度数
//	}
//	if( vecDotVec(shadowVec,axisYVec) > 0.0 ){ eulerX= -eulerX }				//右回りに修正
//	***/
//
//	EQ.Rotate(&targetVec, axisYVec);
//	vec3RotateY(&tmpVec, -Euler->y, &targetVec);
//	targetVec = tmpVec;
//	vec3RotateX(&tmpVec, -Euler->x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = 0.0f;
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler->z = 90.0f;
//	}
//	else {
//		Euler->z = (float)aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) > 0.0) {
//		Euler->z = -Euler->z;
//	}
//	/***
//	E3DMultQVec targetQ,0.0,1.0,0.0,targetVec.0,targetVec.1,targetVec.2
//	E3DVec3RotateY (targetVec.0+0.0),(targetVec.1+0.0),(targetVec.2+0.0),(-eulerY),targetVec.0,targetVec.1,targetVec.2
//	E3DVec3RotateY targetVec.2,targetVec.0,targetVec.1,(-eulerX),shadowVec.2,shadowVec.0,shadowVec.1
//	　　----> 引数がZ,X,YになっているためRotateXと同じ！！！
//
//	  if( lengthVec(shadowVec) == 0 ){
//	  eulerZ= 90.0
//	  }else{
//	  E3DACos ( vecDotVec(shadowVec,axisYVec)/ lengthVec(shadowVec) ), eulerZ		//Z軸回転度数
//	  }
//	  if( vecDotVec(shadowVec,axisXVec) > 0.0 ){ eulerZ= -eulerZ }				//右回りに修正
//
//	  ***/
//	return 0;
//}



int GetRound(float srcval)
{
	if (srcval > 0.0f) {
		return (int)(srcval + 0.5f);
	}
	else {
		return (int)(srcval - 0.5f);
	}
}

//int modifyEuler(ChaVector3* eulerA, ChaVector3* eulerB)
//{
//
//	//オイラー角Aの値をオイラー角Bの値に近い表示に修正
//	float tmpX1, tmpY1, tmpZ1;
//	float tmpX2, tmpY2, tmpZ2;
//	float s1, s2;
//
//	//予想される角度1
//	tmpX1 = eulerA->x + 360.0f * GetRound((eulerB->x - eulerA->x) / 360.0f);
//	tmpY1 = eulerA->y + 360.0f * GetRound((eulerB->y - eulerA->y) / 360.0f);
//	tmpZ1 = eulerA->z + 360.0f * GetRound((eulerB->z - eulerA->z) / 360.0f);
//
//	//予想される角度2
//	tmpX2 = 180.0f - eulerA->x + 360.0f * GetRound((eulerB->x + eulerA->x - 180.0f) / 360.0f);
//	tmpY2 = eulerA->y + 180.0f + 360.0f * GetRound((eulerB->y - eulerA->y - 180.0f) / 360.0f);
//	tmpZ2 = eulerA->z + 180.0f + 360.0f * GetRound((eulerB->z - eulerA->z - 180.0f) / 360.0f);
//
//	//角度変化の大きさ
//	s1 = (eulerB->x - tmpX1) * (eulerB->x - tmpX1) + (eulerB->y - tmpY1) * (eulerB->y - tmpY1) + (eulerB->z - tmpZ1) * (eulerB->z - tmpZ1);
//	s2 = (eulerB->x - tmpX2) * (eulerB->x - tmpX2) + (eulerB->y - tmpY2) * (eulerB->y - tmpY2) + (eulerB->z - tmpZ2) * (eulerB->z - tmpZ2);
//
//	//変化の少ない方に修正
//	if (s1 < s2) {
//		eulerA->x = tmpX1; eulerA->y = tmpY1; eulerA->z = tmpZ1;
//	}
//	else {
//		eulerA->x = tmpX2; eulerA->y = tmpY2; eulerA->z = tmpZ2;
//	}
//
//	return 0;
//}

int IsInitRot(ChaMatrix srcmat)
{
	int retval = 0;

	float d11, d12, d13;
	float d21, d22, d23;
	float d31, d32, d33;

	d11 = srcmat.data[0] - 1.0f;
	d12 = srcmat.data[1] - 0.0f;
	d13 = srcmat.data[2] - 0.0f;

	d21 = srcmat.data[4] - 0.0f;
	d22 = srcmat.data[5] - 1.0f;
	d23 = srcmat.data[6] - 0.0f;

	d31 = srcmat.data[8] - 0.0f;
	d32 = srcmat.data[9] - 0.0f;
	d33 = srcmat.data[10] - 1.0f;

	float dmin = 0.000001f;

	if ((fabs(d11) <= dmin) && (fabs(d12) <= dmin) && (fabs(d13) <= dmin) &&
		(fabs(d21) <= dmin) && (fabs(d22) <= dmin) && (fabs(d23) <= dmin) &&
		(fabs(d31) <= dmin) && (fabs(d32) <= dmin) && (fabs(d33) <= dmin)) {
		retval = 1;
	}

	return retval;
}


int IsSameMat(ChaMatrix srcmat1, ChaMatrix srcmat2)
{

	int retval = 0;
	ChaMatrix diffmat;
	diffmat = srcmat1 - srcmat2;

	float dmin = 0.000001f;

	if ((fabs(diffmat.data[0]) <= dmin) && (fabs(diffmat.data[1]) <= dmin) && (fabs(diffmat.data[2]) <= dmin) && (fabs(diffmat.data[3]) <= dmin) &&
		(fabs(diffmat.data[4]) <= dmin) && (fabs(diffmat.data[5]) <= dmin) && (fabs(diffmat.data[6]) <= dmin) && (fabs(diffmat.data[7]) <= dmin) &&
		(fabs(diffmat.data[8]) <= dmin) && (fabs(diffmat.data[9]) <= dmin) && (fabs(diffmat.data[10]) <= dmin) && (fabs(diffmat.data[11]) <= dmin) &&
		(fabs(diffmat.data[12]) <= dmin) && (fabs(diffmat.data[13]) <= dmin) && (fabs(diffmat.data[14]) <= dmin) && (fabs(diffmat.data[15]) <= dmin)
		) {
		retval = 1;
	}

	return retval;
}

int IsSameEul(ChaVector3 srceul1, ChaVector3 srceul2)
{
	int retval = 0;

	ChaVector3 diffeul;
	diffeul = srceul1 - srceul2;

	//float dmin = 0.000015f;
	float dmin = 0.00010f;//(degree)

	if ((fabs(diffeul.x) <= dmin) && (fabs(diffeul.y) <= dmin) && (fabs(diffeul.z) <= dmin)) {
		retval = 1;
	}

	return retval;
}

void SetAngleLimitOff(ANGLELIMIT* dstal)
{
	if (dstal) {
		int axiskind;
		for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++) {
			if ((dstal->lower[axiskind] == -180) && (dstal->upper[axiskind] == 180)) {
				dstal->limitoff[axiskind] = 1;
			}
			else {
				dstal->limitoff[axiskind] = 0;
			}
		}
	}
}

void InitAngleLimit(ANGLELIMIT* dstal)
{
	dstal->boneaxiskind = BONEAXIS_CURRENT;

	if (dstal) {
		int axiskind;
		for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++) {
			dstal->limitoff[axiskind] = 0;
			dstal->via180flag[axiskind] = 0;
			dstal->lower[axiskind] = -180;
			dstal->upper[axiskind] = 180;
		}
		SetAngleLimitOff(dstal);
	}
}



bool IsTimeEqual(double srctime1, double srctime2)
{
	double difftime = srctime1 - srctime2;
	if ((difftime >= -0.0001) && (difftime <= 0.0001)) {
		return true;
	}
	else {
		return false;
	}
}

double VecLength(ChaVector3 srcvec)
{
	double tmpval = (double)srcvec.x * (double)srcvec.x + (double)srcvec.y * (double)srcvec.y + (double)srcvec.z * (double)srcvec.z;
	if (tmpval > 0.0) {
		return sqrt(tmpval);
	}
	else {
		return 0.0;
	}
}



void GetSRTMatrix(ChaMatrix srcmat, ChaVector3* svecptr, ChaMatrix* rmatptr, ChaVector3* tvecptr)
{
	*svecptr = ChaVector3(1.0f, 1.0f, 1.0f);
	ChaMatrixIdentity(rmatptr);
	*tvecptr = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaMatrix tmpmat1 = srcmat;

	tvecptr->x = tmpmat1.data[12];
	tvecptr->y = tmpmat1.data[13];
	tvecptr->z = tmpmat1.data[14];

	tmpmat1.data[12] = 0.0f;
	tmpmat1.data[13] = 0.0f;
	tmpmat1.data[14] = 0.0f;

	ChaVector3 vec1, vec2, vec3;
	vec1.x = tmpmat1.data[0];
	vec1.y = tmpmat1.data[1];
	vec1.z = tmpmat1.data[2];

	vec2.x = tmpmat1.data[4];
	vec2.y = tmpmat1.data[5];
	vec2.z = tmpmat1.data[6];

	vec3.x = tmpmat1.data[8];
	vec3.y = tmpmat1.data[9];
	vec3.z = tmpmat1.data[10];

	double len1, len2, len3;
	len1 = VecLength(vec1);
	len2 = VecLength(vec2);
	len3 = VecLength(vec3);

	//次のコメントアウトコードでは縮小出来ない
	//if ((fabs(len1) - 1.0) < 0.0001) {
	//	len1 = 1.0;//誤差で急に大きさが変わるのを防止
	//}
	//if (fabs(len1) < 0.0001) {
	//	len1 = 0.0001;//0scale禁止
	//}
	//if ((fabs(len2) - 1.0) < 0.0001) {
	//	len2 = 1.0;//誤差で急に大きさが変わるのを防止
	//}
	//if (fabs(len2) < 0.0001) {
	//	len2 = 0.0001;//0scale禁止
	//}
	//if ((fabs(len3) - 1.0) < 0.0001) {
	//	len3 = 1.0;//誤差で急に大きさが変わるのを防止
	//}
	//if (fabs(len3) < 0.0001) {
	//	len3 = 0.0001;//0scale禁止
	//}


	//2021/12/01
	if (((len1 - 1.0) > 0.0) && ((len1 - 1.0) < 0.00001)) {
		len1 = 1.0;//誤差で急に大きさが変わるのを防止
	}
	if (len1 < 0.00001) {
		len1 = 0.00001;//0scale禁止
	}
	if (((len2 - 1.0) > 0.0) && ((len2 - 1.0) < 0.00001)) {
		len2 = 1.0;//誤差で急に大きさが変わるのを防止
	}
	if (len2 < 0.00001) {
		len2 = 0.00001;//0scale禁止
	}
	if (((len3 - 1.0) > 0.0) && ((len3 - 1.0) < 0.00001)) {
		len3 = 1.0;//誤差で急に大きさが変わるのを防止
	}
	if (len3 < 0.00001) {
		len3 = 0.00001;//0scale禁止
	}


	svecptr->x = (float)len1;
	svecptr->y = (float)len2;
	svecptr->z = (float)len3;

	if (len1 != 0.0) {
		rmatptr->data[0] = (float)((double)tmpmat1.data[0] / len1);
		rmatptr->data[1] = (float)((double)tmpmat1.data[1] / len1);
		rmatptr->data[2] = (float)((double)tmpmat1.data[2] / len1);
	}
	else {
		rmatptr->data[0] = 1.0f;
		rmatptr->data[1] = 0.0f;
		rmatptr->data[2] = 0.0f;
	}

	if (len2 != 0.0f) {
		rmatptr->data[4] = (float)((double)tmpmat1.data[4] / len2);
		rmatptr->data[5] = (float)((double)tmpmat1.data[5] / len2);
		rmatptr->data[6] = (float)((double)tmpmat1.data[6] / len2);
	}
	else {
		rmatptr->data[4] = 0.0f;
		rmatptr->data[5] = 1.0f;
		rmatptr->data[6] = 0.0f;
	}

	if (len3 != 0.0f) {
		rmatptr->data[8] = (float)((double)tmpmat1.data[8] / len3);
		rmatptr->data[9] = (float)((double)tmpmat1.data[9] / len3);
		rmatptr->data[10] = (float)((double)tmpmat1.data[10] / len3);
	}
	else {
		rmatptr->data[8] = 0.0f;
		rmatptr->data[9] = 0.0f;
		rmatptr->data[10] = 1.0f;
	}


}

void GetSRTMatrix2(ChaMatrix srcmat, ChaMatrix* smatptr, ChaMatrix* rmatptr, ChaMatrix* tmatptr)
{
	if (!smatptr || !rmatptr || !tmatptr) {
		return;
	}

	ChaMatrix smat, rmat, tmat;
	ChaMatrixIdentity(&smat);
	ChaMatrixIdentity(&rmat);
	ChaMatrixIdentity(&tmat);

	ChaVector3 svec = ChaVector3(0.0, 0.0, 0.0);
	ChaVector3 tvec = ChaVector3(0.0, 0.0, 0.0);

	GetSRTMatrix(srcmat, &svec, &rmat, &tvec);

	ChaMatrixScaling(&smat, svec.x, svec.y, svec.z);
	ChaMatrixTranslation(&tmat, tvec.x, tvec.y, tvec.z);

	*smatptr = smat;
	*rmatptr = rmat;
	*tmatptr = tmat;
}

ChaMatrix GetS0RTMatrix(ChaMatrix srcmat) 
{
	//拡大縮小を初期化したRT行列を返す
	ChaMatrix retm;
	ChaVector3 svec, tvec;
	ChaMatrix rmat, tmat;
	GetSRTMatrix(srcmat, &svec, &rmat, &tvec);

	retm = srcmat;

	if (svec.x != 0.0f) {
		retm.data[0] = (float)((double)retm.data[0] / (double)svec.x);
		retm.data[1] = (float)((double)retm.data[1] / (double)svec.x);
		retm.data[2] = (float)((double)retm.data[2] / (double)svec.x);
	}
	else {
		retm.data[0] = 1.0f;
		retm.data[1] = 0.0f;
		retm.data[2] = 0.0f;
	}

	if (svec.y != 0.0f) {
		retm.data[4] = (float)((double)retm.data[4] / (double)svec.y);
		retm.data[5] = (float)((double)retm.data[5] / (double)svec.y);
		retm.data[6] = (float)((double)retm.data[6] / (double)svec.y);
	}
	else {
		retm.data[4] = 0.0f;
		retm.data[5] = 1.0f;
		retm.data[6] = 0.0f;
	}

	if (svec.z != 0.0f) {
		retm.data[8] = (float)((double)retm.data[8] / (double)svec.z);
		retm.data[9] = (float)((double)retm.data[9] / (double)svec.z);
		retm.data[10] = (float)((double)retm.data[10] / (double)svec.z);
	}
	else {
		retm.data[8] = 0.0f;
		retm.data[9] = 0.0f;
		retm.data[10] = 1.0f;
	}

	return retm;
}




ChaMatrix TransZeroMat(ChaMatrix srcmat)
{
	ChaMatrix retmat;
	retmat = srcmat;
	retmat.data[12] = 0.0f;
	retmat.data[13] = 0.0f;
	retmat.data[14] = 0.0f;

	return retmat;
}

ChaMatrix ChaMatrixFromFbxAMatrix(FbxAMatrix srcmat)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	retmat.data[0] = (float)srcmat.Get(0, 0);
	retmat.data[1] = (float)srcmat.Get(0, 1);
	retmat.data[2] = (float)srcmat.Get(0, 2);
	retmat.data[3] = (float)srcmat.Get(0, 3);

	retmat.data[4] = (float)srcmat.Get(1, 0);
	retmat.data[5] = (float)srcmat.Get(1, 1);
	retmat.data[6] = (float)srcmat.Get(1, 2);
	retmat.data[7] = (float)srcmat.Get(1, 3);

	retmat.data[8] = (float)srcmat.Get(2, 0);
	retmat.data[9] = (float)srcmat.Get(2, 1);
	retmat.data[10] = (float)srcmat.Get(2, 2);
	retmat.data[11] = (float)srcmat.Get(2, 3);

	retmat.data[12] = (float)srcmat.Get(3, 0);
	retmat.data[13] = (float)srcmat.Get(3, 1);
	retmat.data[14] = (float)srcmat.Get(3, 2);
	retmat.data[15] = (float)srcmat.Get(3, 3);

	return retmat;

}

ChaMatrix ChaMatrixFromBtMat3x3(btMatrix3x3* srcmat3x3)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (!srcmat3x3) {
		return retmat;
	}

	btVector3 tmpcol[3];
	int colno;
	for (colno = 0; colno < 3; colno++) {
		tmpcol[colno] = srcmat3x3->getColumn(colno);
		//tmpcol[colno] = worldmat.getRow( colno );
	}

	retmat.data[0] = tmpcol[0].x();
	retmat.data[1] = tmpcol[0].y();
	retmat.data[2] = tmpcol[0].z();

	retmat.data[4] = tmpcol[1].x();
	retmat.data[5] = tmpcol[1].y();
	retmat.data[6] = tmpcol[1].z();

	retmat.data[8] = tmpcol[2].x();
	retmat.data[9] = tmpcol[2].y();
	retmat.data[10] = tmpcol[2].z();

	return retmat;
}

ChaMatrix ChaMatrixFromBtTransform(btMatrix3x3* srcmat3x3, btVector3* srcpivot)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	btVector3 tmpcol[3];
	int colno;
	for (colno = 0; colno < 3; colno++) {
		tmpcol[colno] = srcmat3x3->getColumn(colno);
		//tmpcol[colno] = worldmat.getRow( colno );
	}

	retmat.data[0] = tmpcol[0].x();
	retmat.data[1] = tmpcol[0].y();
	retmat.data[2] = tmpcol[0].z();

	retmat.data[4] = tmpcol[1].x();
	retmat.data[5] = tmpcol[1].y();
	retmat.data[6] = tmpcol[1].z();

	retmat.data[8] = tmpcol[2].x();
	retmat.data[9] = tmpcol[2].y();
	retmat.data[10] = tmpcol[2].z();

	retmat.data[12] = srcpivot->x();
	retmat.data[13] = srcpivot->y();
	retmat.data[14] = srcpivot->z();

	return retmat;
}

ChaMatrix ChaMatrixInv(ChaMatrix srcmat)
{
	ChaMatrix retmat;
	ChaMatrixInverse(&retmat, NULL, &srcmat);
	return retmat;
}



//ChaVector2

ChaVector2::ChaVector2()
{
	x = 0.0f;
	y = 0.0f;
}

ChaVector2::ChaVector2(float srcx, float srcy)
{
	x = srcx;
	y = srcy;
}

ChaVector2::~ChaVector2()
{

}


ChaVector2 ChaVector2::operator= (ChaVector2 v) { this->x = v.x; this->y = v.y;; return *this; };
ChaVector2 ChaVector2::operator* (float srcw) const { return ChaVector2((float)((double)this->x * (double)srcw), (float)((double)this->y * (double)srcw)); }
ChaVector2 &ChaVector2::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaVector2 ChaVector2::operator/ (float srcw) const { if (srcw != 0.0f) { return ChaVector2((float)((double)this->x / (double)srcw), (float)((double)this->y / (double)srcw)); } else { return ChaVector2(0.0f, 0.0f); } }
ChaVector2 &ChaVector2::operator/= (float srcw) { if (srcw != 0.0f) { *this = *this / srcw; return *this; } else { this->x = 0.0f; this->y = 0.0f; return *this; } }
ChaVector2 ChaVector2::operator* (double srcw) const { return ChaVector2((float)((double)this->x * srcw), (float)((double)this->y * srcw)); }
ChaVector2 &ChaVector2::operator*= (double srcw) { *this = *this * srcw; return *this; }
ChaVector2 ChaVector2::operator/ (double srcw) const { if (srcw != 0.0) { return ChaVector2((float)((double)this->x / srcw), (float)((double)this->y / srcw)); } else { return ChaVector2(0.0f, 0.0f); } }
ChaVector2 &ChaVector2::operator/= (double srcw) { if (srcw != 0.0) { *this = *this / srcw; return *this; } else { this->x = 0.0f; this->y = 0.0f; return *this; } }
ChaVector2 ChaVector2::operator+ (const ChaVector2 &v) const { return ChaVector2(x + v.x, y + v.y); }
ChaVector2 &ChaVector2::operator+= (const ChaVector2 &v) { *this = *this + v; return *this; }
ChaVector2 ChaVector2::operator- (const ChaVector2 &v) const { return ChaVector2(x - v.x, y - v.y); }
ChaVector2 &ChaVector2::operator-= (const ChaVector2 &v) { *this = *this - v; return *this; }

ChaVector2 ChaVector2::operator- () const { return *this * -1.0; }



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
ChaVector3::ChaVector3(DirectX::XMVECTOR v)
{
	x = v.m128_f32[0];
	y = v.m128_f32[1];
	z = v.m128_f32[2];
}

ChaVector3::~ChaVector3()
{

}


ChaVector3 ChaVector3::operator= (ChaVector3 v) { this->x = v.x; this->y = v.y; this->z = v.z; return *this; };
ChaVector3 ChaVector3::operator* (float srcw) const { return ChaVector3((float)((double)this->x * (double)srcw), (float)((double)this->y * (double)srcw), (float)((double)this->z * (double)srcw)); }
ChaVector3 &ChaVector3::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaVector3 ChaVector3::operator/ (float srcw) const { if (srcw != 0.0f) { return ChaVector3((float)((double)this->x / (double)srcw), (float)((double)this->y / (double)srcw), (float)((double)this->z / (double)srcw)); } else { return ChaVector3(0.0f, 0.0f, 0.0f); } }
ChaVector3 &ChaVector3::operator/= (float srcw) { *this = *this / srcw; return *this; }
ChaVector3 ChaVector3::operator* (double srcw) const { return ChaVector3((float)((double)this->x * srcw), (float)((double)this->y * srcw), (float)((double)this->z * srcw)); }
ChaVector3& ChaVector3::operator*= (double srcw) { *this = *this * srcw; return *this; }
ChaVector3 ChaVector3::operator/ (double srcw) const { if (srcw != 0.0) { return ChaVector3((float)((double)this->x / srcw), (float)((double)this->y / srcw), (float)((double)this->z / srcw)); } else { return ChaVector3(0.0f, 0.0f, 0.0f); } }
ChaVector3& ChaVector3::operator/= (double srcw) { *this = *this / srcw; return *this; }
ChaVector3 ChaVector3::operator+ (const ChaVector3 &v) const { return ChaVector3(x + v.x, y + v.y, z + v.z); }
ChaVector3 &ChaVector3::operator+= (const ChaVector3 &v) { *this = *this + v; return *this; }
ChaVector3 ChaVector3::operator- (const ChaVector3 &v) const { return ChaVector3(x - v.x, y - v.y, z - v.z); }
ChaVector3 &ChaVector3::operator-= (const ChaVector3 &v) { *this = *this - v; return *this; }

ChaVector3 ChaVector3::operator- () const { return *this * -1.0; }


ChaMatrix ChaVector3::MakeTraMat()
{
	ChaMatrix retmat;
	retmat.SetIdentity();
	retmat.data[12] = x;
	retmat.data[13] = y;
	retmat.data[14] = z;
	return retmat;
}
ChaMatrix ChaVector3::MakeInvTraMat()
{
	ChaMatrix retmat;
	retmat.SetIdentity();
	retmat.data[12] = -x;
	retmat.data[13] = -y;
	retmat.data[14] = -z;
	return retmat;
}
ChaMatrix ChaVector3::MakeXYZRotMat(CQuaternion* srcaxisq)
{
	ChaMatrix retmat;
	retmat.SetIdentity();

	CQuaternion rotq;
	rotq.SetRotationXYZ(srcaxisq, *this);
	
	retmat = rotq.MakeRotMatX();
	return retmat;
}
ChaMatrix ChaVector3::MakeScaleMat()
{
	ChaMatrix retmat;
	retmat.SetIdentity();

	retmat.data[0] = x;
	retmat.data[5] = y;
	retmat.data[10] = z;
	return retmat;
}




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
ChaVector4 ChaVector4::operator* (float srcw) const { return ChaVector4((float)((double)this->x * (double)srcw), (float)((double)this->y * (double)srcw), (float)((double)this->z * (double)srcw), (float)((double)this->w * (double)srcw)); }
ChaVector4 &ChaVector4::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaVector4 ChaVector4::operator/ (float srcw) const { if (srcw != 0.0f) { return ChaVector4((float)((double)this->x / (double)srcw), (float)((double)this->y / (double)srcw), (float)((double)this->z / (double)srcw), (float)((double)this->w / (double)srcw)); } else { return ChaVector4(0.0f, 0.0f, 0.0f, 0.0f); } }
ChaVector4 &ChaVector4::operator/= (float srcw) { *this = *this / srcw; return *this; }
ChaVector4 ChaVector4::operator* (double srcw) const { return ChaVector4((float)((double)this->x * srcw), (float)((double)this->y * srcw), (float)((double)this->z * srcw), (float)((double)this->w * srcw)); }
ChaVector4& ChaVector4::operator*= (double srcw) { *this = *this * srcw; return *this; }
ChaVector4 ChaVector4::operator/ (double srcw) const { if (srcw != 0.0) { return ChaVector4((float)((double)this->x / srcw), (float)((double)this->y / srcw), (float)((double)this->z / srcw), (float)((double)this->w / srcw)); } else { return ChaVector4(0.0f, 0.0f, 0.0f, 0.0f); } }
ChaVector4& ChaVector4::operator/= (double srcw) { *this = *this / srcw; return *this; }
ChaVector4 ChaVector4::operator+ (const ChaVector4 &v) const { return ChaVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
ChaVector4 &ChaVector4::operator+= (const ChaVector4 &v) { *this = *this + v; return *this; }
ChaVector4 ChaVector4::operator- (const ChaVector4 &v) const { return ChaVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
ChaVector4 &ChaVector4::operator-= (const ChaVector4 &v) { *this = *this - v; return *this; }

ChaVector4 ChaVector4::operator- () const { return *this * -1.0; }


ChaMatrix::ChaMatrix()
{
	data[0] = 1.0f;
	data[1] = 0.0f;
	data[2] = 0.0f;
	data[3] = 0.0f;

	data[4] = 0.0f;
	data[5] = 1.0f;
	data[6] = 0.0f;
	data[7] = 0.0f;

	data[8] = 0.0f;
	data[9] = 0.0f;
	data[10] = 1.0f;
	data[11] = 0.0f;

	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;

	//_11 = 1.0f;
	//_12 = 0.0f;
	//_13 = 0.0f;
	//data[3] = 0.0f;

	//data[4] = 0.0f;
	//data[5] = 1.0f;
	//data[6] = 0.0f;
	//data[7] = 0.0f;

	//data[8] = 0.0f;
	//data[9] = 0.0f;
	//data[10] = 1.0f;
	//data[11] = 0.0f;

	//data[12] = 0.0f;
	//data[13] = 0.0f;
	//data[14] = 0.0f;
	//data[15] = 1.0f;

}

ChaMatrix::ChaMatrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
{
	data[0] = m11;
	data[1] = m12;
	data[2] = m13;
	data[3] = m14;

	data[4] = m21;
	data[5] = m22;
	data[6] = m23;
	data[7] = m24;

	data[8] = m31;
	data[9] = m32;
	data[10] = m33;
	data[11] = m34;

	data[12] = m41;
	data[13] = m42;
	data[14] = m43;
	data[15] = m44;

}

void ChaMatrix::SetIdentity()
{
	ChaMatrixIdentity(this);
}

FbxAMatrix ChaMatrix::FBXAMATRIX()
{
	FbxAMatrix retmat;
	retmat.SetIdentity();
	retmat.SetRow(0, FbxVector4(data[0], data[1], data[2], data[3]));
	retmat.SetRow(1, FbxVector4(data[4], data[5], data[6], data[7]));
	retmat.SetRow(2, FbxVector4(data[8], data[9], data[10], data[11]));
	retmat.SetRow(3, FbxVector4(data[12], data[13], data[14], data[15]));
	return retmat;
}

void ChaMatrix::SetTranslation(ChaVector3 srctra)
{
	//初期化しない

	data[12] = srctra.x;
	data[13] = srctra.y;
	data[14] = srctra.z;
}
void ChaMatrix::SetXYZRotation(CQuaternion* srcaxisq, ChaVector3 srceul)
{
	//初期化しない
	ChaMatrix rotmat = srceul.MakeXYZRotMat(srcaxisq);
	SetBasis(rotmat);
}
void ChaMatrix::SetXYZRotation(CQuaternion* srcaxisq, CQuaternion srcq)
{
	//初期化しない

	CQuaternion EQ;
	if (srcaxisq) {
		EQ = srcaxisq->inverse() * srcq * *srcaxisq;
	}
	else {
		EQ = srcq;
	}

	ChaMatrix rotmat = srcq.MakeRotMatX();
	SetBasis(rotmat);

}
void ChaMatrix::SetScale(ChaVector3 srcscale)
{
	//初期化しない

	data[0] = srcscale.x;
	data[5] = srcscale.y;
	data[10] = srcscale.z;

}
void ChaMatrix::SetBasis(ChaMatrix srcmat)
{
	//初期化しない　copy3x3
	data[0] = srcmat.data[0];
	data[1] = srcmat.data[1];
	data[2] = srcmat.data[2];

	data[4] = srcmat.data[4];
	data[5] = srcmat.data[5];
	data[6] = srcmat.data[6];

	data[8] = srcmat.data[8];
	data[9] = srcmat.data[9];
	data[10] = srcmat.data[10];
}

#ifdef CONVD3DX11
ChaMatrix::ChaMatrix(DirectX::XMMATRIX m)
{
	data[0] = m.r[0].m128_f32[0];
	data[1] = m.r[0].m128_f32[1];
	data[2] = m.r[0].m128_f32[2];
	data[3] = m.r[0].m128_f32[3];

	data[4] = m.r[1].m128_f32[0];
	data[5] = m.r[1].m128_f32[1];
	data[6] = m.r[1].m128_f32[2];
	data[7] = m.r[1].m128_f32[3];

	data[8] = m.r[2].m128_f32[0];
	data[9] = m.r[2].m128_f32[1];
	data[10] = m.r[2].m128_f32[2];
	data[11] = m.r[2].m128_f32[3];

	data[12] = m.r[3].m128_f32[0];
	data[13] = m.r[3].m128_f32[1];
	data[14] = m.r[3].m128_f32[2];
	data[15] = m.r[3].m128_f32[3];

	//_11 = m.r[0].m128_f32[0];
	//_12 = m.r[1].m128_f32[0];
	//_13 = m.r[2].m128_f32[0];
	//data[3] = m.r[3].m128_f32[0];

	//data[4] = m.r[0].m128_f32[1];
	//data[5] = m.r[1].m128_f32[1];
	//data[6] = m.r[2].m128_f32[1];
	//data[7] = m.r[3].m128_f32[1];

	//data[8] = m.r[0].m128_f32[2];
	//data[9] = m.r[1].m128_f32[2];
	//data[10] = m.r[2].m128_f32[2];
	//data[11] = m.r[3].m128_f32[2];

	//data[12] = m.r[0].m128_f32[3];
	//data[13] = m.r[1].m128_f32[3];
	//data[14] = m.r[2].m128_f32[3];
	//data[15] = m.r[3].m128_f32[3];

};
#endif


ChaMatrix::~ChaMatrix()
{


}



ChaMatrix ChaMatrix::operator= (ChaMatrix m) { 

	//this->data[0] = m.data[0]; 
	//this->data[1] = m.data[1]; 
	//this->data[2] = m.data[2];
	//this->data[3] = m.data[3];
	//
	//this->data[4] = m.data[4];
	//this->data[5] = m.data[5];
	//this->data[6] = m.data[6];
	//this->data[7] = m.data[7];

	//this->data[8] = m.data[8];
	//this->data[9] = m.data[9];
	//this->data[10] = m.data[10];
	//this->data[11] = m.data[11];

	//this->data[12] = m.data[12];
	//this->data[13] = m.data[13];
	//this->data[14] = m.data[14];
	//this->data[15] = m.data[15];

	this->mVec[0] = m.mVec[0];
	this->mVec[1] = m.mVec[1];
	this->mVec[2] = m.mVec[2];
	this->mVec[3] = m.mVec[3];

	return *this;
};



ChaMatrix ChaMatrix::operator* (float srcw) const {
	ChaMatrix retmat;
	__m128 multcoef = _mm_setr_ps(srcw, srcw, srcw, srcw);
	retmat.mVec[0] = _mm_mul_ps(multcoef, mVec[0]);
	retmat.mVec[1] = _mm_mul_ps(multcoef, mVec[1]);
	retmat.mVec[2] = _mm_mul_ps(multcoef, mVec[2]);
	retmat.mVec[3] = _mm_mul_ps(multcoef, mVec[3]);
	return retmat;

	//return ChaMatrix((float)((double)this->data[0] * (double)srcw), (float)((double)this->data[1] * (double)srcw), (float)((double)this->data[2] * (double)srcw), (float)((double)this->data[3] * (double)srcw),
	//	(float)((double)this->data[4] * (double)srcw), (float)((double)this->data[5] * (double)srcw), (float)((double)this->data[6] * (double)srcw), (float)((double)this->data[7] * (double)srcw),
	//	(float)((double)this->data[8] * (double)srcw), (float)((double)this->data[9] * (double)srcw), (float)((double)this->data[10] * (double)srcw), (float)((double)this->data[11] * (double)srcw),
	//	(float)((double)this->data[12] * (double)srcw), (float)((double)this->data[13] * (double)srcw), (float)((double)this->data[14] * (double)srcw), (float)((double)this->data[15] * (double)srcw));
}
ChaMatrix &ChaMatrix::operator*= (float srcw) { *this = *this * srcw; return *this; }
ChaMatrix ChaMatrix::operator/ (float srcw) const {
	ChaMatrix retmat;

	if (fabs(srcw) >= 1e-5) {
		const __m128 inverseOne = _mm_setr_ps(1.f, 1.f, 1.f, 1.f);
		__m128 divcoef = _mm_setr_ps(srcw, srcw, srcw, srcw);
		__m128 multcoef = _mm_div_ps(inverseOne, divcoef);
		retmat.mVec[0] = _mm_mul_ps(multcoef, mVec[0]);
		retmat.mVec[1] = _mm_mul_ps(multcoef, mVec[1]);
		retmat.mVec[2] = _mm_mul_ps(multcoef, mVec[2]);
		retmat.mVec[3] = _mm_mul_ps(multcoef, mVec[3]);
		return retmat;
	}
	else {
		ChaMatrixIdentity(&retmat);
		return retmat;
	}

	//if (srcw != 0.0f) {
	//	return ChaMatrix((float)((double)this->data[0] / (double)srcw), (float)((double)this->data[1] / (double)srcw), (float)((double)this->data[2] / (double)srcw), (float)((double)this->data[3] / (double)srcw),
	//		(float)((double)this->data[4] / (double)srcw), (float)((double)this->data[5] / (double)srcw), (float)((double)this->data[6] / (double)srcw), (float)((double)this->data[7] / (double)srcw),
	//		(float)((double)this->data[8] / (double)srcw), (float)((double)this->data[9] / (double)srcw), (float)((double)this->data[10] / (double)srcw), (float)((double)this->data[11] / (double)srcw),
	//		(float)((double)this->data[12] / (double)srcw), (float)((double)this->data[13] / (double)srcw), (float)((double)this->data[14] / (double)srcw), (float)((double)this->data[15] / (double)srcw));
	//}
	//else {
	//	ChaMatrix retmat;
	//	ChaMatrixIdentity(&retmat);
	//	return retmat;
	//}
}
ChaMatrix &ChaMatrix::operator/= (float srcw) { *this = *this / srcw; return *this; }
ChaMatrix ChaMatrix::operator* (double srcw) const {
	return ChaMatrix((float)((double)this->data[0] * srcw), (float)((double)this->data[1] * srcw), (float)((double)this->data[2] * srcw), (float)((double)this->data[3] * srcw),
		(float)((double)this->data[4] * srcw), (float)((double)this->data[5] * srcw), (float)((double)this->data[6] * srcw), (float)((double)this->data[7] * srcw),
		(float)((double)this->data[8] * srcw), (float)((double)this->data[9] * srcw), (float)((double)this->data[10] * srcw), (float)((double)this->data[11] * srcw),
		(float)((double)this->data[12] * srcw), (float)((double)this->data[13] * srcw), (float)((double)this->data[14] * srcw), (float)((double)this->data[15] * srcw));
}
ChaMatrix& ChaMatrix::operator*= (double srcw) { *this = *this * srcw; return *this; }
ChaMatrix ChaMatrix::operator/ (double srcw) const {
	if (srcw != 0.0) {
		return ChaMatrix((float)((double)this->data[0] / srcw), (float)((double)this->data[1] / srcw), (float)((double)this->data[2] / srcw), (float)((double)this->data[3] / srcw),
			(float)((double)this->data[4] / srcw), (float)((double)this->data[5] / srcw), (float)((double)this->data[6] / srcw), (float)((double)this->data[7] / srcw),
			(float)((double)this->data[8] / srcw), (float)((double)this->data[9] / srcw), (float)((double)this->data[10] / srcw), (float)((double)this->data[11] / srcw),
			(float)((double)this->data[12] / srcw), (float)((double)this->data[13] / srcw), (float)((double)this->data[14] / srcw), (float)((double)this->data[15] / srcw));
	}
	else {
		ChaMatrix retmat;
		ChaMatrixIdentity(&retmat);
		return retmat;
	}
}
ChaMatrix& ChaMatrix::operator/= (double srcw) { *this = *this / srcw; return *this; }



ChaMatrix ChaMatrix::operator+ (const ChaMatrix &m) const {
	ChaMatrix retmat;
	retmat.mVec[0] = _mm_add_ps(m.mVec[0], mVec[0]);
	retmat.mVec[1] = _mm_add_ps(m.mVec[1], mVec[1]);
	retmat.mVec[2] = _mm_add_ps(m.mVec[2], mVec[2]);
	retmat.mVec[3] = _mm_add_ps(m.mVec[3], mVec[3]);
	return retmat;

	//return ChaMatrix(
	//	data[0] + m.data[0], data[1] + m.data[1], data[2] + m.data[2], data[3] + m.data[3],
	//	data[4] + m.data[4], data[5] + m.data[5], data[6] + m.data[6], data[7] + m.data[7],
	//	data[8] + m.data[8], data[9] + m.data[9], data[10] + m.data[10], data[11] + m.data[11],
	//	data[12] + m.data[12], data[13] + m.data[13], data[14] + m.data[14], data[15] + m.data[15]
	//	);
}
ChaMatrix &ChaMatrix::operator+= (const ChaMatrix &m) { *this = *this + m; return *this; }

ChaMatrix ChaMatrix::operator- (const ChaMatrix &m) const { 
	ChaMatrix retmat;
	retmat.mVec[0] = _mm_sub_ps(mVec[0], m.mVec[0]);
	retmat.mVec[1] = _mm_sub_ps(mVec[1], m.mVec[1]);
	retmat.mVec[2] = _mm_sub_ps(mVec[2], m.mVec[2]);
	retmat.mVec[3] = _mm_sub_ps(mVec[3], m.mVec[3]);
	return retmat;

	//return ChaMatrix(
	//data[0] - m.data[0], data[1] - m.data[1], data[2] - m.data[2], data[3] - m.data[3],
	//data[4] - m.data[4], data[5] - m.data[5], data[6] - m.data[6], data[7] - m.data[7],
	//data[8] - m.data[8], data[9] - m.data[9], data[10] - m.data[10], data[11] - m.data[11],
	//data[12] - m.data[12], data[13] - m.data[13], data[14] - m.data[14], data[15] - m.data[15]
	//); 
}
ChaMatrix &ChaMatrix::operator-= (const ChaMatrix &m) { *this = *this - m; return *this; }


ChaMatrix ChaMatrix::operator* (const ChaMatrix& m) const {
	ChaMatrix res;

	__m128  xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	
	//xmm4 = _mm_loadu_ps(&m.data[0]);
	//xmm5 = _mm_loadu_ps(&m.data[4]);
	//xmm6 = _mm_loadu_ps(&m.data[8]);
	//xmm7 = _mm_loadu_ps(&m.data[12]);
	xmm4 = m.mVec[0];
	xmm5 = m.mVec[1];
	xmm6 = m.mVec[2];
	xmm7 = m.mVec[3];

	// column0
	xmm0 = _mm_load1_ps(&data[0]);
	xmm1 = _mm_load1_ps(&data[1]);
	xmm2 = _mm_load1_ps(&data[2]);
	xmm3 = _mm_load1_ps(&data[3]);
	
	//xmm0 = _mm_mul_ps(xmm0, xmm4);
	//xmm1 = _mm_mul_ps(xmm1, xmm5);
	//xmm2 = _mm_mul_ps(xmm2, xmm6);
	//xmm3 = _mm_mul_ps(xmm3, xmm7);
	//
	//xmm0 = _mm_add_ps(xmm0, xmm1);
	//xmm2 = _mm_add_ps(xmm2, xmm3);
	//xmm0 = _mm_add_ps(xmm0, xmm2);
	//_mm_storeu_ps(&res.data[0], xmm0);

	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm1 = _mm_madd_ps(xmm1, xmm5, xmm0);
	xmm2 = _mm_madd_ps(xmm2, xmm6, xmm1);
	xmm3 = _mm_madd_ps(xmm3, xmm7, xmm2);

	res.mVec[0] = xmm3;

	
	// column1
	xmm0 = _mm_load1_ps(&data[4]);
	xmm1 = _mm_load1_ps(&data[5]);
	xmm2 = _mm_load1_ps(&data[6]);
	xmm3 = _mm_load1_ps(&data[7]);
	
	//xmm0 = _mm_mul_ps(xmm0, xmm4);
	//xmm1 = _mm_mul_ps(xmm1, xmm5);
	//xmm2 = _mm_mul_ps(xmm2, xmm6);
	//xmm3 = _mm_mul_ps(xmm3, xmm7);
	//
	//xmm0 = _mm_add_ps(xmm0, xmm1);
	//xmm2 = _mm_add_ps(xmm2, xmm3);
	//xmm0 = _mm_add_ps(xmm0, xmm2);
	//
	//_mm_storeu_ps(&res.data[4], xmm0);

	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm1 = _mm_madd_ps(xmm1, xmm5, xmm0);
	xmm2 = _mm_madd_ps(xmm2, xmm6, xmm1);
	xmm3 = _mm_madd_ps(xmm3, xmm7, xmm2);

	res.mVec[1] = xmm3;


	// column2
	xmm0 = _mm_load1_ps(&data[8]);
	xmm1 = _mm_load1_ps(&data[9]);
	xmm2 = _mm_load1_ps(&data[10]);
	xmm3 = _mm_load1_ps(&data[11]);
	
	//xmm0 = _mm_mul_ps(xmm0, xmm4);
	//xmm1 = _mm_mul_ps(xmm1, xmm5);
	//xmm2 = _mm_mul_ps(xmm2, xmm6);
	//xmm3 = _mm_mul_ps(xmm3, xmm7);
	//
	//xmm0 = _mm_add_ps(xmm0, xmm1);
	//xmm2 = _mm_add_ps(xmm2, xmm3);
	//xmm0 = _mm_add_ps(xmm0, xmm2);
	//
	//_mm_storeu_ps(&res.data[8], xmm0);

	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm1 = _mm_madd_ps(xmm1, xmm5, xmm0);
	xmm2 = _mm_madd_ps(xmm2, xmm6, xmm1);
	xmm3 = _mm_madd_ps(xmm3, xmm7, xmm2);

	res.mVec[2] = xmm3;



	// column3
	xmm0 = _mm_load1_ps(&data[12]);
	xmm1 = _mm_load1_ps(&data[13]);
	xmm2 = _mm_load1_ps(&data[14]);
	xmm3 = _mm_load1_ps(&data[15]);
	
	//xmm0 = _mm_mul_ps(xmm0, xmm4);
	//xmm1 = _mm_mul_ps(xmm1, xmm5);
	//xmm2 = _mm_mul_ps(xmm2, xmm6);
	//xmm3 = _mm_mul_ps(xmm3, xmm7);
	//
	//xmm0 = _mm_add_ps(xmm0, xmm1);
	//xmm2 = _mm_add_ps(xmm2, xmm3);
	//xmm0 = _mm_add_ps(xmm0, xmm2);
	//
	//_mm_storeu_ps(&res.data[12], xmm0);

	xmm0 = _mm_mul_ps(xmm0, xmm4);
	xmm1 = _mm_madd_ps(xmm1, xmm5, xmm0);
	xmm2 = _mm_madd_ps(xmm2, xmm6, xmm1);
	xmm3 = _mm_madd_ps(xmm3, xmm7, xmm2);

	res.mVec[3] = xmm3;

	return res;
}


ChaMatrix &ChaMatrix::operator*= (const ChaMatrix &m) { *this = *this * m; return *this; }
ChaMatrix ChaMatrix::operator- () const { return *this * -1.0; }


CQuaternion::CQuaternion()
{
	InitParams();
}

CQuaternion::CQuaternion(float srcw, float srcx, float srcy, float srcz)
{
	w = srcw;
	x = srcx;
	y = srcy;
	z = srcz;
}

void CQuaternion::InitParams()
{
	w = 1.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}


CQuaternion::~CQuaternion()
{

}

int CQuaternion::SetParams(float srcw, float srcx, float srcy, float srcz)
{
	w = srcw;
	x = srcx;
	y = srcy;
	z = srcz;
	return 0;
}
#ifdef CONVD3DX11
int CQuaternion::SetParams(DirectX::XMFLOAT4 srcxq)
{
	w = srcxq.w;
	x = srcxq.x;
	y = srcxq.y;
	z = srcxq.z;
	return 0;
}
#endif

float CQuaternion::QuaternionLimitPhai(float srcphai) 
{
	//#######################################################
	//srcphaiを-180度から180度に直してからクォータニオンの設定をする
	//#######################################################

	float retphai = 0.0f;
	float tmpphai = srcphai;
	int dbgcnt = 0;

	if (srcphai > 0.0f) {
		//retphai = srcphai - (float)((int)(srcphai / (2.0f * (float)PAI))) * (2.0f * (float)PAI);
		//retphai = srcphai - (float)(((int)(srcphai / (float)PAI)) + 1) * (float)PAI;

		//for example : 190 --> -170
		while ((tmpphai >= -(float)PAI) && (tmpphai <= (float)PAI)) {
			tmpphai -= (2.0 * (float)PAI);
			dbgcnt++;
			if (dbgcnt >= 100) {
				tmpphai = srcphai;
				break;
			}
		}

		return tmpphai;
	}
	else if (srcphai < 0.0f) {
		//retphai = srcphai + (float)(((int)(srcphai / (float)PAI)) + 1) * (float)PAI;

		//for example : -190 --> 170

		while ((tmpphai >= -(float)PAI) && (tmpphai <= (float)PAI)) {
			tmpphai += (2.0 * (float)PAI);
			dbgcnt++;
			if (dbgcnt >= 100) {
				tmpphai = srcphai;
				break;
			}
		}

		return tmpphai;
	}
	else {
		return 0.0f;
	}
}

double CQuaternion::QuaternionLimitPhai(double srcphai)
{
	double retphai = 0.0;

	if (srcphai > 0.0) {
		retphai = srcphai - (double)((int)(srcphai / (2.0 * PAI))) * (2.0 * PAI);
		return retphai;
	}
	else if (srcphai < 0.0f) {
		retphai = srcphai + (double)((int)(srcphai / (2.0 * PAI))) * (2.0 * PAI);
		return retphai;
	}
	else {
		return 0.0;
	}
}

int CQuaternion::SetAxisAndRot(ChaVector3 srcaxis, float phai)
{
	double dblphai = phai;
	SetAxisAndRot(srcaxis, dblphai);

	//float phai2;
	//float cos_phai2, sin_phai2;

	//phai2 = QuaternionLimitPhai(phai) * 0.5f;
	//cos_phai2 = cosf(phai2);
	//sin_phai2 = sinf(phai2);

	//w = cos_phai2;
	//x = srcaxis.x * sin_phai2;
	//y = srcaxis.y * sin_phai2;
	//z = srcaxis.z * sin_phai2;

	return 0;
}
int CQuaternion::SetAxisAndRot(ChaVector3 srcaxis, double phai)
{
	double phai2;
	double cos_phai2, sin_phai2;

	phai2 = QuaternionLimitPhai(phai) * 0.5;
	cos_phai2 = cos(phai2);
	sin_phai2 = sin(phai2);

	w = (float)cos_phai2;
	x = (float)((double)srcaxis.x * sin_phai2);
	y = (float)((double)srcaxis.y * sin_phai2);
	z = (float)((double)srcaxis.z * sin_phai2);

	return 0;
}

//int CQuaternion::GetAxisAndRot( ChaVector3* axisvecptr, float* frad )
//{
//	D3DXQUATERNION tempq;
//
//	tempq.x = x;
//	tempq.y = y;
//	tempq.z = z;
//	tempq.w = w;
//
//	D3DXQuaternionToAxisAngle( &tempq, &(axisvecptr->D3DX()), frad );
//	
//	ChaVector3Normalize( axisvecptr, axisvecptr );
//
//	return 0;
//}


int CQuaternion::SetRotationXYZ(CQuaternion* axisq, ChaVector3 srcdeg)
{
	// X軸、Y軸、Z軸の順番で、回転する、クォータニオンをセットする。

	CQuaternion axisQ, invaxisQ;
	if (axisq) {
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
	}
	else {
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}


	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;


	double phaix, phaiy, phaiz;
	phaix = QuaternionLimitPhai((double)srcdeg.x * DEG2PAI);
	phaiy = QuaternionLimitPhai((double)srcdeg.y * DEG2PAI);
	phaiz = QuaternionLimitPhai((double)srcdeg.z * DEG2PAI);

	cosx = (float)cos(phaix * 0.5);
	sinx = (float)sin(phaix * 0.5);
	cosy = (float)cos(phaiy * 0.5);
	siny = (float)sin(phaiy * 0.5);
	cosz = (float)cos(phaiz * 0.5);
	sinz = (float)sin(phaiz * 0.5);

	qx.SetParams(cosx, sinx, 0.0f, 0.0f);
	qy.SetParams(cosy, 0.0f, siny, 0.0f);
	qz.SetParams(cosz, 0.0f, 0.0f, sinz);

	//q = axisQ * qy * qx * qz * invaxisQ;
	q = axisQ * qz * qy * qx * invaxisQ;


	*this = q;

	return 0;
}

int CQuaternion::SetRotationXYZ(CQuaternion* axisq, double degx, double degy, double degz)
{
	// X軸、Y軸、Z軸の順番で、回転する、クォータニオンをセットする。
	CQuaternion axisQ, invaxisQ;
	if (axisq) {
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
	}
	else {
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}

	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;

	double phaix, phaiy, phaiz;
	phaix = QuaternionLimitPhai(degx * DEG2PAI);
	phaiy = QuaternionLimitPhai(degy * DEG2PAI);
	phaiz = QuaternionLimitPhai(degz * DEG2PAI);

	cosx = (float)cos(phaix * 0.5);
	sinx = (float)sin(phaix * 0.5);
	cosy = (float)cos(phaiy * 0.5);
	siny = (float)sin(phaiy * 0.5);
	cosz = (float)cos(phaiz * 0.5);
	sinz = (float)sin(phaiz * 0.5);

	qx.SetParams(cosx, sinx, 0.0f, 0.0f);
	qy.SetParams(cosy, 0.0f, siny, 0.0f);
	qz.SetParams(cosz, 0.0f, 0.0f, sinz);

	//q = axisQ * qy * qx * qz * invaxisQ;
	q = axisQ * qz * qy * qx * invaxisQ;

	*this = q;

	return 0;
}

int CQuaternion::SetRotationZXY(CQuaternion* axisq, ChaVector3 srcdeg)
{
	// Z軸、X軸、Y軸の順番で、回転する、クォータニオンをセットする。

	CQuaternion axisQ, invaxisQ;
	if (axisq) {
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
	}
	else {
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}


	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;

	double phaix, phaiy, phaiz;
	phaix = QuaternionLimitPhai((double)srcdeg.x * DEG2PAI);
	phaiy = QuaternionLimitPhai((double)srcdeg.y * DEG2PAI);
	phaiz = QuaternionLimitPhai((double)srcdeg.z * DEG2PAI);

	cosx = (float)cos(phaix * 0.5);
	sinx = (float)sin(phaix * 0.5);
	cosy = (float)cos(phaiy * 0.5);
	siny = (float)sin(phaiy * 0.5);
	cosz = (float)cos(phaiz * 0.5);
	sinz = (float)sin(phaiz * 0.5);


	qx.SetParams(cosx, sinx, 0.0f, 0.0f);
	qy.SetParams(cosy, 0.0f, siny, 0.0f);
	qz.SetParams(cosz, 0.0f, 0.0f, sinz);

	q = axisQ * qy * qx * qz * invaxisQ;


	*this = q;

	return 0;
}

int CQuaternion::SetRotationZXY(CQuaternion* axisq, double degx, double degy, double degz)
{
	// Z軸、X軸、Y軸の順番で、回転する、クォータニオンをセットする。
	CQuaternion axisQ, invaxisQ;
	if (axisq) {
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
	}
	else {
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}

	CQuaternion q, qx, qy, qz;
	float cosx, sinx, cosy, siny, cosz, sinz;


	double phaix, phaiy, phaiz;
	phaix = QuaternionLimitPhai(degx * DEG2PAI);
	phaiy = QuaternionLimitPhai(degy * DEG2PAI);
	phaiz = QuaternionLimitPhai(degz * DEG2PAI);

	cosx = (float)cos(phaix * 0.5);
	sinx = (float)sin(phaix * 0.5);
	cosy = (float)cos(phaiy * 0.5);
	siny = (float)sin(phaiy * 0.5);
	cosz = (float)cos(phaiz * 0.5);
	sinz = (float)sin(phaiz * 0.5);

	qx.SetParams(cosx, sinx, 0.0f, 0.0f);
	qy.SetParams(cosy, 0.0f, siny, 0.0f);
	qz.SetParams(cosz, 0.0f, 0.0f, sinz);

	q = axisQ * qy * qx * qz * invaxisQ;

	*this = q;

	return 0;
}


CQuaternion CQuaternion::operator= (CQuaternion q) { this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; return *this; };
CQuaternion CQuaternion::operator* (float srcw) const { return CQuaternion((float)((double)this->w * (double)srcw), (float)((double)this->x * (double)srcw), (float)((double)this->y * (double)srcw), (float)((double)this->z * (double)srcw)); }
CQuaternion &CQuaternion::operator*= (float srcw) { *this = *this * srcw; return *this; }
CQuaternion CQuaternion::operator/ (float srcw) const { if (srcw != 0.0f) { return CQuaternion((float)((double)this->w / (double)srcw), (float)((double)this->x / (double)srcw), (float)((double)this->y / (double)srcw), (float)((double)this->z / (double)srcw)); } else { return CQuaternion(1.0f, 0.0f, 0.0f, 0.0f); } }
CQuaternion &CQuaternion::operator/= (float srcw) { *this = *this / srcw; return *this; }
CQuaternion CQuaternion::operator* (double srcw) const { return CQuaternion((float)((double)this->w * srcw), (float)((double)this->x * srcw), (float)((double)this->y * srcw), (float)((double)this->z * srcw)); }
CQuaternion& CQuaternion::operator*= (double srcw) { *this = *this * srcw; return *this; }
CQuaternion CQuaternion::operator/ (double srcw) const { if (srcw != 0.0) { return CQuaternion((float)((double)this->w / srcw), (float)((double)this->x / srcw), (float)((double)this->y / srcw), (float)((double)this->z / srcw)); } else { return CQuaternion(1.0f, 0.0f, 0.0f, 0.0f); } }
CQuaternion& CQuaternion::operator/= (double srcw) { *this = *this / srcw; return *this; }
CQuaternion CQuaternion::operator+ (const CQuaternion &q) const { return CQuaternion(w + q.w, x + q.x, y + q.y, z + q.z); }
CQuaternion &CQuaternion::operator+= (const CQuaternion &q) { *this = *this + q; return *this; }
CQuaternion CQuaternion::operator- (const CQuaternion &q) const { return CQuaternion(w - q.w, x - q.x, y - q.y, z - q.z); }
CQuaternion &CQuaternion::operator-= (const CQuaternion &q) { *this = *this - q; return *this; }
CQuaternion CQuaternion::operator* (const CQuaternion &q) const {
	//return CQuaternion(
	//	w * q.w - x * q.x - y * q.y - z * q.z,
	//	w * q.x + q.w * x + y * q.z - z * q.y,
	//	w * q.y + q.w * y + z * q.x - x * q.z,
	//	w * q.z + q.w * z + x * q.y - y * q.x ).normalize();
	double tmpx, tmpy, tmpz, tmpw;
	tmpw = (double)w * (double)q.w - (double)x * (double)q.x - (double)y * (double)q.y - (double)z * (double)q.z;
	tmpx = (double)w * (double)q.x + (double)q.w * (double)x + (double)y * (double)q.z - (double)z * (double)q.y;
	tmpy = (double)w * (double)q.y + (double)q.w * (double)y + (double)z * (double)q.x - (double)x * (double)q.z;
	tmpz = (double)w * (double)q.z + (double)q.w * (double)z + (double)x * (double)q.y - (double)y * (double)q.x;
	CQuaternion retq;
	retq.x = (float)tmpx;
	retq.y = (float)tmpy;
	retq.z = (float)tmpz;
	retq.w = (float)tmpw;
	return retq.normalize();
}
CQuaternion &CQuaternion::operator*= (const CQuaternion &q) { *this = *this * q; return *this; }
CQuaternion CQuaternion::operator- () const { return *this * -1.0; }
CQuaternion CQuaternion::normalize() {
	//float mag = w*w+x*x+y*y+z*z;
	//if( mag != 0.0f )
	//	return (*this)*(1.0f/(float)::sqrt(mag));
	//else
	//	return CQuaternion( 1.0f, 0.0f, 0.0f, 0.0f );

	double mag = (double)w * (double)w + (double)x * (double)x + (double)y * (double)y + (double)z * (double)z;
	if (mag != 0.0) {
		double divval = ::sqrt(mag);
		double tmpx;
		double tmpy;
		double tmpz;
		double tmpw;
		if (divval != 0.0) {
			tmpx = (double)x / divval;
			tmpy = (double)y / divval;
			tmpz = (double)z / divval;
			tmpw = (double)w / divval;
		}
		else {
			tmpx = 0.0;
			tmpy = 0.0;
			tmpz = 0.0;
			tmpw = 1.0;
		}
		this->x = (float)tmpx;
		this->y = (float)tmpy;
		this->z = (float)tmpz;
		this->w = (float)tmpw;
		return *this;
	}
	else {
		return CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
	}
}


ChaMatrix CQuaternion::MakeRotMatX()
{
	normalize();

	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	float data[16];
	ZeroMemory(data, sizeof(float) * 16);

	//転置
	data[0] = (float)(1.0 - 2.0 * ((double)y * (double)y + (double)z * (double)z));
	data[4] = (float)(2.0 * ((double)x * (double)y - (double)z * (double)w));
	data[8] = (float)(2.0 * ((double)z * (double)x + (double)w * (double)y));

	data[1] = (float)(2.0 * ((double)x * (double)y + (double)z * (double)w));
	data[5] = (float)(1.0 - 2.0 * ((double)z * (double)z + (double)x * (double)x));
	data[9] = (float)(2.0 * ((double)y * (double)z - (double)w * (double)x));

	data[2] = (float)(2.0 * ((double)z * (double)x - (double)w * (double)y));
	data[6] = (float)(2.0 * ((double)y * (double)z + (double)x * (double)w));
	data[10] = (float)(1.0 - 2.0 * ((double)y * (double)y + (double)x * (double)x));

	retmat.data[0] = (float)data[0];
	retmat.data[4] = (float)data[4];
	retmat.data[8] = (float)data[8];

	retmat.data[1] = (float)data[1];
	retmat.data[5] = (float)data[5];
	retmat.data[9] = (float)data[9];

	retmat.data[2] = (float)data[2];
	retmat.data[6] = (float)data[6];
	retmat.data[10] = (float)data[10];


	/*
	retmat.data[0] = 1.0 - 2.0 * (y * y + z * z);
	retmat._12 = 2.0 * (x * y - z * w);
	retmat._13 = 2.0 * (z * x + w * y);

	retmat.data[4] = 2.0 * (x * y + z * w);
	retmat.data[5] = 1.0 - 2.0 * (z * z + x * x);
	retmat.data[6] = 2.0 * (y * z - w * x);

	retmat.data[8] = 2.0 * (z * x - w * y);
	retmat.data[9] = 2.0 * (y * z + x * w);
	retmat.data[10] = 1.0 - 2.0 * (y * y + x * x);
	*/

	/*
	D3DXQUATERNION qx;
	qx.x = x;
	qx.y = y;
	qx.z = z;
	qx.w = w;
	ChaMatrixRotationQuaternion(&retmat, &qx);
	*/
	return retmat;
}


void CQuaternion::RotationMatrix(ChaMatrix srcmat)
{
	//転置前バージョン

	//CQuaternionは gpar * par * curの順で掛ける系
	//ChaMatrixは cur * par * gparの順で掛ける系
	//CQuaternionの時に転置してChaMatrixのときにそのままで計算が合う。

	CQuaternion tmpq;

	//スケールに関して正規化した回転からQuaternionを求める。そのためにSRTに分解する。
	ChaVector3 svec, tvec;
	ChaMatrix rmat;
	GetSRTMatrix(srcmat, &svec, &rmat, &tvec);

	float m[4][4];

	m[0][0] = rmat.data[0];
	m[0][1] = rmat.data[1];
	m[0][2] = rmat.data[2];
	m[0][3] = rmat.data[3];
	m[1][0] = rmat.data[4];
	m[1][1] = rmat.data[5];
	m[1][2] = rmat.data[6];
	m[1][3] = rmat.data[7];
	m[2][0] = rmat.data[8];
	m[2][1] = rmat.data[9];
	m[2][2] = rmat.data[10];
	m[2][3] = rmat.data[11];
	m[3][0] = rmat.data[12];
	m[3][1] = rmat.data[13];
	m[3][2] = rmat.data[14];
	m[3][3] = rmat.data[15];

	int i, maxi;
	FLOAT maxdiag;
	double S, trace;

	trace = (double)m[0][0] + (double)m[1][1] + (double)m[2][2] + 1.0;
	if (trace > 0.0f)
	{
		tmpq.x = (float)(((double)m[1][2] - (double)m[2][1]) / (2.0 * sqrt(trace)));
		tmpq.y = (float)(((double)m[2][0] - (double)m[0][2]) / (2.0 * sqrt(trace)));
		tmpq.z = (float)(((double)m[0][1] - (double)m[1][0]) / (2.0 * sqrt(trace)));
		tmpq.w = (float)(sqrt(trace) / 2.0);
		*this = tmpq;
		return;
	}
	maxi = 0;
	maxdiag = m[0][0];
	for (i = 1; i<3; i++)
	{
		if (m[i][i] > maxdiag)
		{
			maxi = i;
			maxdiag = m[i][i];
		}
	}
	switch (maxi)
	{
	case 0:
		S = 2.0 * sqrt(1.0 + (double)m[0][0] - (double)m[1][1] - (double)m[2][2]);
		if (S != 0.0) {
			tmpq.x = (float)(0.25 * S);
			tmpq.y = (float)(((double)m[0][1] + (double)m[1][0]) / S);
			tmpq.z = (float)(((double)m[0][2] + (double)m[2][0]) / S);
			tmpq.w = (float)(((double)m[1][2] - (double)m[2][1]) / S);
		}
		else {
			tmpq = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case 1:
		S = 2.0 * sqrt(1.0 + (double)m[1][1] - (double)m[0][0] - (double)m[2][2]);
		if (S != 0.0) {
			tmpq.x = (float)(((double)m[0][1] + (double)m[1][0]) / S);
			tmpq.y = (float)(0.25 * S);
			tmpq.z = (float)(((double)m[1][2] + (double)m[2][1]) / S);
			tmpq.w = (float)(((double)m[2][0] - (double)m[0][2]) / S);
		}
		else {
			tmpq = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case 2:
		S = 2.0 * sqrt(1.0 + (double)m[2][2] - (double)m[0][0] - (double)m[1][1]);
		if (S != 0.0) {
			tmpq.x = (float)(((double)m[0][2] + (double)m[2][0]) / S);
			tmpq.y = (float)(((double)m[1][2] + (double)m[2][1]) / S);
			tmpq.z = (float)(0.25 * S);
			tmpq.w = (float)(((double)m[0][1] - (double)m[1][0]) / S);
		}
		else {
			tmpq = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	}
	*this = tmpq;


	//ChaMatrix tmpmat;
	//tmpmat = srcmat;
	//tmpmat.data[12] = 0.0f;
	//tmpmat.data[13] = 0.0f;
	//tmpmat.data[14] = 0.0f;

	//D3DXQUATERNION qx;
	//D3DXQuaternionRotationMatrix(&qx, &tmpmat);
	//SetParams(qx);
}

/*
void CQuaternion::RotationMatrix(ChaMatrix srcmat)
{
//転置後バージョン

CQuaternion tmpq;

//スケールに関して正規化した回転からQuaternionを求める。そのためにSRTに分解する。
ChaVector3 svec, tvec;
ChaMatrix rmat;
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

//float CQuaternion::DotProduct(CQuaternion srcq)
//{
//	float dot;
//	dot = w * srcq.w +
//		x * srcq.x +
//		y * srcq.y +
//		z * srcq.z;
//	return dot;
//}
double CQuaternion::DotProduct(CQuaternion srcq)
{
	double dot;
	dot = (double)w * (double)srcq.w +
		(double)x * (double)srcq.x +
		(double)y * (double)srcq.y +
		(double)z * (double)srcq.z;
	return dot;
}



double CQuaternion::CalcRad(CQuaternion srcq)
{
	double dot;
	double retrad;
	dot = this->DotProduct(srcq);

	//!!!!!!!!!!　注意　!!!!!!!!!!!!
	//!!!! dot が１より微妙に大きい値のとき、kakuには、無効な値(-1.#IN00)が入ってしまう。
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (dot > 1.0)
		dot = 1.0;
	if (dot < -1.0)
		dot = -1.0;
	retrad = acos(dot);

	return retrad;
}

int CQuaternion::Slerp2(CQuaternion endq, double t, CQuaternion* dstq)
{
	dstq->SetParams(1.0f, 0.0f, 0.0f, 0.0f);

	if (t == 1.0) {
		*dstq = endq;
		return 0;//!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	else if (t == 0.0) {
		*dstq = *this;
		return 0;//!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	double kaku;
	kaku = this->CalcRad(endq);

	if (kaku > (PI * 0.5)) {
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		endq = -endq;
		kaku = this->CalcRad(endq);
		_ASSERT(kaku <= (PI * 0.5));
	}

	// sin( kaku ) == 0.0 付近を調整。
	//180度にはならないので（ならないようにするので）０度のみケア
	int kaku0flag = 0;
	if ((kaku <= 1e-4) && (kaku >= -1e-4)) {
		kaku0flag = 1;
	}

	CQuaternion tmpq;
	tmpq.SetParams(w, x, y, z);

	double alpha, beta;
	if (kaku0flag == 0) {
		alpha = sin(kaku * (1.0 - t)) / sin(kaku);
		beta = sin(kaku * t) / sin(kaku);

		dstq->x = (float)((double)tmpq.x * alpha + (double)endq.x * beta);
		dstq->y = (float)((double)tmpq.y * alpha + (double)endq.y * beta);
		dstq->z = (float)((double)tmpq.z * alpha + (double)endq.z * beta);
		dstq->w = (float)((double)tmpq.w * alpha + (double)endq.w * beta);
		//		retq = tmpq * alpha + endq * beta;

	}
	else {
		*dstq = tmpq;
	}
	return 0;
}

CQuaternion CQuaternion::Slerp(CQuaternion endq, int framenum, int frameno)
{
	CQuaternion retq;
	retq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);

	double kaku;
	kaku = this->CalcRad(endq);

	if (kaku > (PI * 0.5)) {
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		endq = -endq;
		kaku = this->CalcRad(endq);
		_ASSERT(kaku <= (PI * 0.5));
	}

	// sin( kaku ) == 0.0 付近を調整。
	//180度にはならないので（ならないようにするので）０度のみケア
	int kaku0flag = 0;
	if ((kaku <= 1e-4) && (kaku >= -1e-4)) {
		kaku0flag = 1;
		//DbgOut( "Quaternion : Slerp : kaku0flag 1 : dot %f, kaku %f\n", dot, kaku );
	}


	double t = (double)frameno / (double)framenum;
	double alpha, beta;
	if (kaku0flag == 0) {
		alpha = sin(kaku * (1.0 - t)) / sin(kaku);
		beta = sin(kaku * t) / sin(kaku);

		retq = *this * alpha + endq * beta;
	}
	else {
		retq = *this;
		//DbgOut( "MotionInfo : FillUpMotionPoint 2: frame %d, %f, %f, %f, %f\n",
		//	   frameno, startq.w, startq.x, startq.y, startq.z );
	}

	return retq;
}

#ifdef CONVD3DX11
//int CQuaternion::Squad(CQuaternion q0, CQuaternion q1, CQuaternion q2, CQuaternion q3, float t)
//{
//
//	CQuaternion iq0, iq1, iq2, iq3;
//	D3DXQUATERNION qx0, qx1, qx2, qx3;
//	D3DXQUATERNION ax, bx, cx, resx;
//
//	q0.inv(&iq0);
//	q1.inv(&iq1);
//	q2.inv(&iq2);
//	q3.inv(&iq3);
//
//	Q2X(&qx0, iq0);
//	Q2X(&qx1, iq1);
//	Q2X(&qx2, iq2);
//	Q2X(&qx3, iq3);
//
//	D3DXQuaternionSquadSetup(&ax, &bx, &cx, &qx0, &qx1, &qx2, &qx3);
//	D3DXQuaternionSquad(&resx, &qx1, &ax, &bx, &cx, t);
//
//	D3DXQUATERNION iresx;
//	D3DXQuaternionInverse(&iresx, &resx);
//
//	this->x = iresx.x;
//	this->y = iresx.y;
//	this->z = iresx.z;
//	this->w = iresx.w;
//
//
//	return 0;
//}
int CQuaternion::Q2X(DirectX::XMFLOAT4* dstx)
{
	dstx->x = x;
	dstx->y = y;
	dstx->z = z;
	dstx->w = w;

	return 0;
}

int CQuaternion::Q2X(DirectX::XMFLOAT4* dstx, CQuaternion srcq)
{
	dstx->x = srcq.x;
	dstx->y = srcq.y;
	dstx->z = srcq.z;
	dstx->w = srcq.w;

	return 0;
}
#endif

int CQuaternion::inv(CQuaternion* dstq)
{

	dstq->w = w;
	dstq->x = -x;
	dstq->y = -y;
	dstq->z = -z;

	*dstq = dstq->normalize();

	return 0;
}

CQuaternion CQuaternion::inverse()
{
	CQuaternion retq;
	this->inv(&retq);
	return retq;
}

int CQuaternion::RotationArc(ChaVector3 srcvec0, ChaVector3 srcvec1)
{
	//srcvec0, srcvec1は、normalizeされているとする。

	ChaVector3 c;
	ChaVector3Cross(&c, (const ChaVector3*)&srcvec0, (const ChaVector3*)&srcvec1);
	double d;
	d = ChaVector3DotDbl(&srcvec0, &srcvec1);
	double mags = (1.0 + d) * 2.0;
	double s;
	if (mags != 0.0) {
		s = sqrt(mags);
		if (s != 0.0) {
			x = (float)((double)c.x / s);
			y = (float)((double)c.y / s);
			z = (float)((double)c.z / s);
			w = (float)(s / 2.0);
		}
		else {
			x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f;
		}
	}
	else {
		x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f;
	}
	
	return 0;
}


int CQuaternion::Rotate(ChaVector3* dstvec, ChaVector3 srcvec)
{
	ChaMatrix mat;

	mat = MakeRotMatX();

	ChaVector3TransformCoord(dstvec, &srcvec, &mat);

	return 0;
}

//int CQuaternion::QuaternionToAxisAngle( ChaVector3* dstaxis, float* dstrad )
//{
//	D3DXQUATERNION xq;
//
//	int ret;
//	ret = Q2X( &xq );
//	_ASSERT( !ret );
//
//	D3DXQuaternionToAxisAngle( &xq, &(dstaxis->D3DX()), dstrad );
//
//	return 0;
//}

int CQuaternion::transpose(CQuaternion* dstq)
{
	ChaMatrix matx;

	matx = MakeRotMatX();

	ChaMatrix tmatx;
	ChaMatrixTranspose(&tmatx, &matx);

	dstq->RotationMatrix(tmatx);

	return 0;
}

ChaMatrix CQuaternion::CalcSymX2()
{
	CQuaternion tmpq;
	tmpq = *this;
	tmpq.x *= -1.0;
	//tmpq.y *= -1.0f;
	//tmpq.z *= -1.0f;
	tmpq.w *= -1.0;

	return tmpq.MakeRotMatX();
}


int CQuaternion::CalcSym(CQuaternion* dstq)
{
	CQuaternion tmpq;
	tmpq = *this;
	tmpq.x *= -1.0;
	//tmpq.y *= -1.0f;
	//tmpq.z *= -1.0f;
	tmpq.w *= -1.0;

	*dstq = tmpq;

	return 0;
}

double CQuaternion::vecDotVec(ChaVector3* vec1, ChaVector3* vec2)
{
	double tmpval = (double)vec1->x * (double)vec2->x + (double)vec1->y * (double)vec2->y + (double)vec1->z * (double)vec2->z;
	return tmpval;
}

double CQuaternion::lengthVec(ChaVector3* vec)
{
	double mag;
	double leng;

	mag = (double)vec->x * (double)vec->x + (double)vec->y * (double)vec->y + (double)vec->z * (double)vec->z;
	if (mag == 0.0) {
		leng = 0.0;
	}
	else {
		leng = sqrt(mag);
	}
	return leng;
}

double CQuaternion::aCos(double dot)
{
	if (dot > 1.0)
		dot = 1.0;
	else if (dot < -1.0)
		dot = -1.0;

	double rad;
	rad = acos(dot);

	double degree;
	degree = (rad * PAI2DEG);

	return degree;
}

int CQuaternion::vec3RotateY(ChaVector3* dstvec, double deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ChaVector3 tmpsrcvec = *srcvec;


	ret = dirq.SetRotationXYZ(0, 0, deg, 0);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	double tmpx, tmpy, tmpz;
	tmpx = (double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12];
	tmpy = (double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13];
	tmpz = (double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14];

	dstvec->x = (float)tmpx;
	dstvec->y = (float)tmpy;
	dstvec->z = (float)tmpz;

	return 0;
}
int CQuaternion::vec3RotateX(ChaVector3* dstvec, double deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ChaVector3 tmpsrcvec = *srcvec;

	ret = dirq.SetRotationXYZ(0, deg, 0, 0);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	double tmpx, tmpy, tmpz;

	tmpx = (double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12];
	tmpy = (double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13];
	tmpz = (double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14];

	dstvec->x = (float)tmpx;
	dstvec->y = (float)tmpy;
	dstvec->z = (float)tmpz;


	return 0;
}
int CQuaternion::vec3RotateZ(ChaVector3* dstvec, double deg, ChaVector3* srcvec)
{

	int ret;
	CQuaternion dirq;
	ChaMatrix	dirm;

	ChaVector3 tmpsrcvec = *srcvec;

	ret = dirq.SetRotationXYZ(0, 0, 0, deg);
	_ASSERT(!ret);
	dirm = dirq.MakeRotMatX();

	double tmpx, tmpy, tmpz;

	tmpx = (double)dirm.data[0] * (double)tmpsrcvec.x + (double)dirm.data[4] * (double)tmpsrcvec.y + (double)dirm.data[8] * (double)tmpsrcvec.z + (double)dirm.data[12];
	tmpy = (double)dirm.data[1] * (double)tmpsrcvec.x + (double)dirm.data[5] * (double)tmpsrcvec.y + (double)dirm.data[9] * (double)tmpsrcvec.z + (double)dirm.data[13];
	tmpz = (double)dirm.data[2] * (double)tmpsrcvec.x + (double)dirm.data[6] * (double)tmpsrcvec.y + (double)dirm.data[10] * (double)tmpsrcvec.z + (double)dirm.data[14];

	dstvec->x = (float)tmpx;
	dstvec->y = (float)tmpy;
	dstvec->z = (float)tmpz;

	return 0;
}

//int CQuaternion::Q2EulBt(ChaVector3* reteul)
//{
//	ChaVector3 Euler;
//
//
//	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
//	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
//	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);
//
//	ChaVector3 targetVec, shadowVec;
//	ChaVector3 tmpVec;
//
//	Rotate(&targetVec, axisZVec);
//	shadowVec.x = 0.0f;
//	shadowVec.y = (float)vecDotVec(&targetVec, &axisYVec);
//	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.x = 90.0f;
//	}
//	else {
//		Euler.x = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) > 0.0) {
//		Euler.x = -Euler.x;
//	}
//
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = 0.0f;
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.y = 90.0f;
//	}
//	else {
//		Euler.y = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	//if( vecDotVec( &shadowVec, &axisXVec ) < 0.0f ){
//	if (vecDotVec(&shadowVec, &axisXVec) > 0.0) {
//		Euler.y = -Euler.y;
//	}
//
//
//	Rotate(&targetVec, axisYVec);
//	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
//	targetVec = tmpVec;
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = 0.0f;
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.z = 90.0f;
//	}
//	else {
//		Euler.z = (float)aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
//	}
//	//if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
//	if (vecDotVec(&shadowVec, &axisXVec) < 0.0) {
//		Euler.z = -Euler.z;
//	}
//
//
//	*reteul = Euler;
//
//	return 0;
//}
//
//int CQuaternion::Q2EulZXY(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul)
//{
//	return Q2Eul(axisq, befeul, reteul);
//}
//
//int CQuaternion::Q2EulYXZ(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul)
//{
//
//	CQuaternion axisQ, invaxisQ, EQ;
//	if (axisq) {
//		axisQ = *axisq;
//		axisQ.inv(&invaxisQ);
//		EQ = invaxisQ * *this * axisQ;
//	}
//	else {
//		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		EQ = *this;
//	}
//
//	ChaVector3 Euler;
//
//
//	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
//	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
//	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);
//
//	ChaVector3 targetVec, shadowVec;
//	ChaVector3 tmpVec;
//
//	EQ.Rotate(&targetVec, axisYVec);
//	shadowVec.x = (float)vecDotVec(&targetVec, &axisXVec);
//	shadowVec.y = (float)vecDotVec(&targetVec, &axisYVec);
//	shadowVec.z = 0.0f;
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.z = 90.0f;
//	}
//	else {
//		Euler.z = (float)aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) > 0.0) {
//		Euler.z = -Euler.z;
//	}
//
//	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
//	shadowVec.x = 0.0f;
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.x = 90.0f;
//	}
//	else {
//		Euler.x = (float)aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisZVec) < 0.0) {
//		Euler.x = -Euler.x;
//	}
//
//
//	EQ.Rotate(&targetVec, axisZVec);
//	vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
//	targetVec = tmpVec;
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = 0.0f;
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.y = 90.0f;
//	}
//	else {
//		Euler.y = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) < 0.0) {
//		Euler.y = -Euler.y;
//	}
//
//	ModifyEuler(&Euler, &befeul);
//	*reteul = Euler;
//
//	return 0;
//}


int CQuaternion::Q2EulXYZ(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul, int isfirstbone, int isendbone, int notmodifyflag)
{

	CQuaternion axisQ, invaxisQ, EQ;
	if (axisq) {
		axisQ = *axisq;
		axisQ.inv(&invaxisQ);
		EQ = invaxisQ * *this * axisQ;
	}
	else {
		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		EQ = *this;
	}

	ChaVector3 Euler;

	CQuaternion iniq;

	CQuaternion QinvZ;
	CQuaternion EQinvZ;
	ChaVector3 EinvZ;
	
	CQuaternion QinvY;
	CQuaternion EQinvYZ;
	ChaVector3 EinvY;

	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);

	ChaVector3 targetVec, shadowVec;
	ChaVector3 tmpVec;

	EQ.Rotate(&targetVec, axisXVec);
	shadowVec.x = (float)vecDotVec(&targetVec, &axisXVec);
	shadowVec.y = (float)vecDotVec(&targetVec, &axisYVec);
	shadowVec.z = 0.0f;
	if (lengthVec(&shadowVec) <= 1e-5) {
		Euler.z = 90.0f;
	}
	else {
		Euler.z = (float)aCos(vecDotVec(&shadowVec, &axisXVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) < 0.0) {
		Euler.z = -Euler.z;
	}
	//if (vecDotVec(&shadowVec, &axisYVec) > 0.0f) {
	//	Euler.z = -Euler.z;
	//}

	EinvZ = ChaVector3(0.0f, 0.0f, -Euler.z);
	QinvZ.SetRotationXYZ(&iniq, EinvZ);
	EQinvZ = QinvZ * EQ;
	EQinvZ.Rotate(&targetVec, axisXVec);
	shadowVec.x = (float)vecDotVec(&targetVec, &axisXVec);
	shadowVec.y = 0.0f;
	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);

	//vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	//shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
	//shadowVec.y = 0.0f;
	//shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) <= 1e-5) {
		Euler.y = 90.0f;
	}
	else {
		Euler.y = (float)aCos(vecDotVec(&shadowVec, &axisXVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisZVec) > 0.0) {
		Euler.y = -Euler.y;
	}
	//if (vecDotVec(&shadowVec, &axisZVec) < 0.0f) {
	//	Euler.y = -Euler.y;
	//}


	EinvY = ChaVector3(0.0f, -Euler.y, 0.0f);
	QinvY.SetRotationXYZ(&iniq, EinvY);
	EQinvYZ = QinvY * QinvZ * EQ;
	EQinvYZ.Rotate(&targetVec, axisZVec);
	shadowVec.x = 0.0f;
	shadowVec.y = (float)vecDotVec(&targetVec, &axisYVec);
	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);

	//EQ.Rotate(&targetVec, axisZVec);
	//vec3RotateZ(&tmpVec, -Euler.z, &targetVec);
	//targetVec = tmpVec;
	//vec3RotateY(&tmpVec, -Euler.y, &targetVec);
	//shadowVec.x = 0.0f;
	//shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
	//shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
	if (lengthVec(&shadowVec) <= 1e-5) {
		Euler.x = 90.0f;
	}
	else {
		Euler.x = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
	}
	if (vecDotVec(&shadowVec, &axisYVec) > 0.0) {
		Euler.x = -Euler.x;
	}
	//if (vecDotVec(&shadowVec, &axisYVec) < 0.0f) {
	//	Euler.x = -Euler.x;
	//}

	//ModifyEuler(&Euler, &befeul);
	//ModifyEulerXYZ(&Euler, &befeul, isfirstbone, isendbone, notmodifyflag);//10027 CommentOut. 処理が重いわりにたまにしか役に立たないので。しばらくコメントアウト。
	*reteul = Euler;

	return 0;
}



//int CQuaternion::Q2Eul(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul)
//{
//
//	CQuaternion axisQ, invaxisQ, EQ;
//	if (axisq) {
//		axisQ = *axisq;
//		axisQ.inv(&invaxisQ);
//		EQ = invaxisQ * *this * axisQ;
//	}
//	else {
//		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		EQ = *this;
//	}
//
//	ChaVector3 Euler;
//
//
//	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
//	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
//	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);
//
//	ChaVector3 targetVec, shadowVec;
//	ChaVector3 tmpVec;
//
//	EQ.Rotate(&targetVec, axisZVec);
//	shadowVec.x = (float)vecDotVec(&targetVec, &axisXVec);
//	shadowVec.y = 0.0f;
//	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.y = 90.0f;
//	}
//	else {
//		Euler.y = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) < 0.0) {
//		Euler.y = -Euler.y;
//	}
//
//	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
//	shadowVec.x = 0.0f;
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.x = 90.0f;
//	}
//	else {
//		Euler.x = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisYVec) > 0.0) {
//		Euler.x = -Euler.x;
//	}
//
//
//	EQ.Rotate(&targetVec, axisYVec);
//	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
//	targetVec = tmpVec;
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = 0.0f;
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.z = 90.0f;
//	}
//	else {
//		Euler.z = (float)aCos(vecDotVec(&shadowVec, &axisYVec) / lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) > 0.0) {
//		Euler.z = -Euler.z;
//	}
//
//	ModifyEuler(&Euler, &befeul);
//	*reteul = Euler;
//
//	return 0;
//}
//
//int CQuaternion::Q2EulZYX(int needmodifyflag, CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul)
//{
//	CQuaternion axisQ, invaxisQ, EQ;
//	if (axisq) {
//		axisQ = *axisq;
//		axisQ.inv(&invaxisQ);
//		EQ = invaxisQ * *this * axisQ;
//	}
//	else {
//		axisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		invaxisQ.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		EQ = *this;
//	}
//
//	ChaVector3 Euler;
//
//
//	ChaVector3 axisXVec(1.0f, 0.0f, 0.0f);
//	ChaVector3 axisYVec(0.0f, 1.0f, 0.0f);
//	ChaVector3 axisZVec(0.0f, 0.0f, 1.0f);
//
//	ChaVector3 targetVec, shadowVec;
//	ChaVector3 tmpVec;
//
//	EQ.Rotate(&targetVec, axisZVec);
//	shadowVec.x = 0.0f;
//	shadowVec.y = (float)vecDotVec(&targetVec, &axisYVec);
//	shadowVec.z = (float)vecDotVec(&targetVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.x = 90.0f;
//	}
//	else {
//		Euler.x = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / (double)lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisYVec) > 0.0) {
//		Euler.x = -Euler.x;
//	}
//
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = 0;
//	shadowVec.z = (float)vecDotVec(&tmpVec, &axisZVec);
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.y = 90.0f;
//	}
//	else {
//		Euler.y = (float)aCos(vecDotVec(&shadowVec, &axisZVec) / (double)lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisXVec) < 0.0) {
//		Euler.y = -Euler.y;
//	}
//
//
//	EQ.Rotate(&targetVec, axisXVec);
//	vec3RotateX(&tmpVec, -Euler.x, &targetVec);
//	targetVec = tmpVec;
//	vec3RotateY(&tmpVec, -Euler.y, &targetVec);
//	shadowVec.x = (float)vecDotVec(&tmpVec, &axisXVec);
//	shadowVec.y = (float)vecDotVec(&tmpVec, &axisYVec);
//	shadowVec.z = 0.0f;
//	if (lengthVec(&shadowVec) == 0.0) {
//		Euler.z = 90.0f;
//	}
//	else {
//		Euler.z = (float)aCos(vecDotVec(&shadowVec, &axisXVec) / (double)lengthVec(&shadowVec));
//	}
//	if (vecDotVec(&shadowVec, &axisYVec) < 0.0) {
//		Euler.z = -Euler.z;
//	}
//
//	if (needmodifyflag == 1) {
//		ModifyEuler(&Euler, &befeul);
//	}
//	*reteul = Euler;
//
//	return 0;
//}
//
//
//
//int CQuaternion::ModifyEuler(ChaVector3* eulerA, ChaVector3* eulerB)
//{
//
//	//オイラー角Aの値をオイラー角Bの値に近い表示に修正
//	double tmpX1, tmpY1, tmpZ1;
//	double tmpX2, tmpY2, tmpZ2;
//	double s1, s2;
//
//	//予想される角度1
//	tmpX1 = eulerA->x + 360.0 * GetRound((float)(((double)eulerB->x - (double)eulerA->x) / 360.0));
//	tmpY1 = eulerA->y + 360.0 * GetRound((float)(((double)eulerB->y - (double)eulerA->y) / 360.0));
//	tmpZ1 = eulerA->z + 360.0 * GetRound((float)(((double)eulerB->z - (double)eulerA->z) / 360.0));
//
//	//予想される角度2
//	//クォータニオンは１８０°で一回転する。
//	//横軸が２シータ、縦軸がsin2シータ、cos2シータのグラフにおいて、newシータ　=　180 + oldシータの値は等しい。
//	//tmp2の角度はクォータニオンにおいて等しい姿勢を取るオイラー角である。
//	//この場合、３つの軸のうち１つだけの軸の角度の符号(ここではX軸)が反転する。
//	//ということだと思う。テストすると合っている。
//	tmpX2 = 180.0 - eulerA->x + 360.0 * GetRound((float)(((double)eulerB->x + (double)eulerA->x - 180.0) / 360.0));
//	tmpY2 = eulerA->y + 180.0 + 360.0 * GetRound((float)(((double)eulerB->y - (double)eulerA->y - 180.0) / 360.0));
//	tmpZ2 = eulerA->z + 180.0 + 360.0 * GetRound((float)(((double)eulerB->z - (double)eulerA->z - 180.0) / 360.0));
//
//
//	//角度変化の大きさ
//	s1 = ((double)eulerB->x - tmpX1) * ((double)eulerB->x - tmpX1) + ((double)eulerB->y - tmpY1) * ((double)eulerB->y - tmpY1) + ((double)eulerB->z - tmpZ1) * ((double)eulerB->z - tmpZ1);
//	s2 = ((double)eulerB->x - tmpX2) * ((double)eulerB->x - tmpX2) + ((double)eulerB->y - tmpY2) * ((double)eulerB->y - tmpY2) + ((double)eulerB->z - tmpZ2) * ((double)eulerB->z - tmpZ2);
//
//	//変化の少ない方に修正
//	if (s1 < s2) {
//		eulerA->x = (float)tmpX1; eulerA->y = (float)tmpY1; eulerA->z = (float)tmpZ1;
//	}
//	else {
//		eulerA->x = (float)tmpX2; eulerA->y = (float)tmpY2; eulerA->z = (float)tmpZ2;
//	}
//
//	return 0;
//}

BOOL IsValidNewEul(ChaVector3 srcneweul, ChaVector3 srcbefeul)
{
	//とりあえず
	return TRUE;
}


int CQuaternion::ModifyEulerXYZ(ChaVector3* eulerA, ChaVector3* eulerB, int isfirstbone, int isendbone, int notmodifyflag)
{

	//オイラー角Aの値をオイラー角Bの値に近い表示に修正
	double tmpX0, tmpY0, tmpZ0;
	double tmpX1, tmpY1, tmpZ1;
	double tmpX2, tmpY2, tmpZ2;
	double tmpX3, tmpY3, tmpZ3;
	double tmpX4, tmpY4, tmpZ4;
	//double tmpX5, tmpY5, tmpZ5;
	double s0, s1, s2, s3, s4;// , s5;
	//double newX, newY, newZ;

	tmpX0 = eulerA->x;
	tmpY0 = eulerA->y;
	tmpZ0 = eulerA->z;

	//予想される角度1
	tmpX1 = (double)eulerA->x + 360.0 * GetRound((float)(((double)eulerB->x - (double)eulerA->x) / 360.0));
	tmpY1 = (double)eulerA->y + 360.0 * GetRound((float)(((double)eulerB->y - (double)eulerA->y) / 360.0));
	tmpZ1 = (double)eulerA->z + 360.0 * GetRound((float)(((double)eulerB->z - (double)eulerA->z) / 360.0));


	//##########################################################################################
	// ModifyEuler いままでの試行錯誤のまとめ
	//####################################
	// 
	// 
	// 	notmodifyflag == 1//!!!! bvh-->fbx書き出し時にはmodifyeulerで裏返りチェックをするが、それ以外の時は２重に処理しないように裏返りチェックをしない
	// 
	// 
	// 
	// 試行錯誤時の症状と数学を思い出し、結局次のようになった.
	// ノイズが乗っているときにも今までで一番きれいなオイラーグラフになった.つま先に関しても改善.
	// 
	//座標系合わせ。軸の向きが座標系に合うように１８０度回転チェック。座標系を合わせるにはbefeulに近づければ良い.
	// 
	// 
	// クォータニオンからオイラー角を計算するとき、ボーン軸に関して１８０度ねじれるようなクオータニオンのオイラー角と同じオイラー角になる.
	// 例えばZが１８０度回転した時、XとYは-X, -Yになる.(180 - X)では無かった.
	// 裏返ったオイラー角の方がbefeulに近い場合を検出してオイラー角を裏返返す処理をする.
	// 
	// 
	//##########################################################################################


	//X + 180のとき
	tmpX2 = tmpX0 + 180.0;
	if (tmpX2 > 180.0) {
		tmpX2 -= 360.0;
	}
	tmpY2 = -tmpY0;
	tmpZ2 = -tmpZ0;
	//tmpY2 = 180.0 - tmpY0;
	//tmpZ2 = 180.0 - tmpZ0;

	//Y + 180のとき
	tmpY3 = tmpY0 + 180.0;
	if (tmpY3 > 180.0) {
		tmpY3 -= 360.0;
	}
	tmpX3 = -tmpX0;
	tmpZ3 = -tmpZ0;
	//tmpX3 = 180.0 - tmpX0;
	//tmpZ3 = 180.0 - tmpZ0;

	//Z + 180のとき
	tmpZ4 = tmpZ0 + 180.0;
	if (tmpZ4 > 180.0) {
		tmpZ4 -= 360.0;
	}
	tmpX4 = -tmpX0;
	tmpY4 = -tmpY0;
	//tmpX4 = 180.0 - tmpX0;
	//tmpY4 = 180.0 - tmpY0;



	//角度変化の大きさ
	s0 = ((double)eulerB->x - tmpX0) * ((double)eulerB->x - tmpX0) + ((double)eulerB->y - tmpY0) * ((double)eulerB->y - tmpY0) + ((double)eulerB->z - tmpZ0) * ((double)eulerB->z - tmpZ0);
	s1 = ((double)eulerB->x - tmpX1) * ((double)eulerB->x - tmpX1) + ((double)eulerB->y - tmpY1) * ((double)eulerB->y - tmpY1) + ((double)eulerB->z - tmpZ1) * ((double)eulerB->z - tmpZ1);
	s2 = ((double)eulerB->x - tmpX2) * ((double)eulerB->x - tmpX2) + ((double)eulerB->y - tmpY2) * ((double)eulerB->y - tmpY2) + ((double)eulerB->z - tmpZ2) * ((double)eulerB->z - tmpZ2);
	s3 = ((double)eulerB->x - tmpX3) * ((double)eulerB->x - tmpX3) + ((double)eulerB->y - tmpY3) * ((double)eulerB->y - tmpY3) + ((double)eulerB->z - tmpZ3) * ((double)eulerB->z - tmpZ3);
	s4 = ((double)eulerB->x - tmpX4) * ((double)eulerB->x - tmpX4) + ((double)eulerB->y - tmpY4) * ((double)eulerB->y - tmpY4) + ((double)eulerB->z - tmpZ4) * ((double)eulerB->z - tmpZ4);

	typedef struct tag_chkeul
	{
		double s;
		int index;
		double X, Y, Z;
		bool operator<(const tag_chkeul& right) const {
			return s == right.s ? index < right.index : s < right.s;
		}
	}CHKEUL;

	std::vector<struct tag_chkeul> vecchkeul;
	CHKEUL tmpchkeul;
	tmpchkeul.s = s0;
	tmpchkeul.index = 0;
	tmpchkeul.X = tmpX0;
	tmpchkeul.Y = tmpY0;
	tmpchkeul.Z = tmpZ0;
	vecchkeul.push_back(tmpchkeul);
	tmpchkeul.s = s1;
	tmpchkeul.index = 1;
	tmpchkeul.X = tmpX1;
	tmpchkeul.Y = tmpY1;
	tmpchkeul.Z = tmpZ1;
	vecchkeul.push_back(tmpchkeul);
	tmpchkeul.s = s2;
	tmpchkeul.index = 2;
	tmpchkeul.X = tmpX2;
	tmpchkeul.Y = tmpY2;
	tmpchkeul.Z = tmpZ2;
	vecchkeul.push_back(tmpchkeul);
	tmpchkeul.s = s3;
	tmpchkeul.index = 3;
	tmpchkeul.X = tmpX3;
	tmpchkeul.Y = tmpY3;
	tmpchkeul.Z = tmpZ3;
	vecchkeul.push_back(tmpchkeul);
	tmpchkeul.s = s4;
	tmpchkeul.index = 4;
	tmpchkeul.X = tmpX4;
	tmpchkeul.Y = tmpY4;
	tmpchkeul.Z = tmpZ4;
	vecchkeul.push_back(tmpchkeul);

	std::sort(vecchkeul.begin(), vecchkeul.end());

	CHKEUL mineul = vecchkeul[0];

	if ((notmodifyflag == 0) && (isfirstbone == 0)) {

		eulerA->x = (float)mineul.X; eulerA->y = (float)mineul.Y; eulerA->z = (float)mineul.Z;

	}
	else {
		//if (s0 <= s1) {
			eulerA->x = (float)tmpX0; eulerA->y = (float)tmpY0; eulerA->z = (float)tmpZ0;
		//}
		//else {
		//	eulerA->x = (float)tmpX1; eulerA->y = (float)tmpY1; eulerA->z = (float)tmpZ1;
		//}
	}

	return 0;
}

/*
int CQuaternion::ModifyEuler( ChaVector3* eulerA, ChaVector3* eulerB )
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
int CQuaternion::GetRound(float srcval)
{
	if (srcval > 0.0f) {
		return (int)(srcval + 0.5);
	}
	else {
		return (int)(srcval - 0.5);
	}
}
//
int CQuaternion::CalcFBXEulXYZ(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul, int isfirstbone, int isendbone, int notmodifyflag)
{

	int noise[4] = { 0, 1, 0, -1 };
	static int dbgcnt = 0;

	ChaVector3 tmpeul(0.0f, 0.0f, 0.0f);
	if (IsInit() == 0) {
		//Q2Eul(axisq, befeul, &tmpeul);
		Q2EulXYZ(axisq, befeul, &tmpeul, isfirstbone, isendbone, notmodifyflag);
		//Q2EulYXZ(axisq, befeul, &tmpeul);
		//Q2EulZXY(axisq, befeul, &tmpeul);
	}
	else {
		//FBX書き出しの際にアニメーションに「ある程度の大きさの変化」がないとキーが省略されてしまう。
		//ノイズを乗せることでアニメーションがなくてもキーが省略されないようになる。
		//ノイズを乗せるのは１つのボーンで良い。
		if (isfirstbone == 1) {
			tmpeul.x = (float)(noise[dbgcnt % 4]) / 100.0f;//!!!!!!!!
		}
		else {
			tmpeul.x = 0.0f;
		}
		tmpeul.y = 0.0f;
		tmpeul.z = 0.0f;
	}
	*reteul = tmpeul;

	dbgcnt++;

	return 0;
}

//int CQuaternion::CalcFBXEulZXY(CQuaternion* axisq, ChaVector3 befeul, ChaVector3* reteul, int isfirstbone)
//{
//
//	int noise[4] = { 0, 1, 0, -1 };
//	static int dbgcnt = 0;
//
//	ChaVector3 tmpeul(0.0f, 0.0f, 0.0f);
//	if (IsInit() == 0) {
//		//Q2Eul(axisq, befeul, &tmpeul);
//		//Q2EulXYZ(axisq, befeul, &tmpeul);
//		//Q2EulYXZ(axisq, befeul, &tmpeul);
//		Q2EulZXY(axisq, befeul, &tmpeul);
//	}
//	else {
//		//FBX書き出しの際にアニメーションに「ある程度の大きさの変化」がないとキーが省略されてしまう。
//		//ノイズを乗せることでアニメーションがなくてもキーが省略されないようになる。
//		//ノイズを乗せるのは１つのボーンで良い。
//		if (isfirstbone == 1) {
//			tmpeul.x = (float)(noise[dbgcnt % 4]) / 100.0f;//!!!!!!!!
//		}
//		else {
//			tmpeul.x = 0.0f;
//		}
//		tmpeul.y = 0.0f;
//		tmpeul.z = 0.0f;
//	}
//	*reteul = tmpeul;
//
//	dbgcnt++;
//
//	return 0;
//}

int CQuaternion::IsInit()
{
	float dx, dy, dz, dw;
	dx = x - 0.0f;
	dy = y - 0.0f;
	dz = z - 0.0f;
	dw = w - 1.0f;

	if ((fabs(dx) <= 0.000001f) && (fabs(dy) <= 0.000001f) && (fabs(dz) <= 0.000001f) && (fabs(dw) <= 0.000001f)) {
		return 1;
	}
	else {
		return 0;
	}
}

int CQuaternion::InOrder(CQuaternion* srcdstq)
{
	double kaku;
	kaku = CalcRad(*srcdstq);
	if (kaku > (PI * 0.5)) {
		//片方を-qにすれば、(PI * 0.5f)より小さくなる。（最短コースをたどれる）
		CQuaternion tmpq = -*srcdstq;
		*srcdstq = tmpq;
	}

	return 0;
}


void CQuaternion::MakeFromD3DXMat(ChaMatrix eulmat)
{
	this->RotationMatrix(eulmat);
}



//global

CQuaternion QMakeFromBtMat3x3(btMatrix3x3* eulmat)
{
	CQuaternion retq;
	retq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	if (!eulmat) {
		return retq;
	}

	btVector3 tmpcol[3];
	int colno;
	for (colno = 0; colno < 3; colno++) {
		tmpcol[colno] = eulmat->getColumn(colno);
	}

	ChaMatrix xmat;
	ChaMatrixIdentity(&xmat);

	xmat.data[0] = tmpcol[0].x();
	xmat.data[1] = tmpcol[0].y();
	xmat.data[2] = tmpcol[0].z();

	xmat.data[4] = tmpcol[1].x();
	xmat.data[5] = tmpcol[1].y();
	xmat.data[6] = tmpcol[1].z();

	xmat.data[8] = tmpcol[2].x();
	xmat.data[9] = tmpcol[2].y();
	xmat.data[10] = tmpcol[2].z();

	retq.RotationMatrix(xmat);
	return retq;

}



void ChaMatrixIdentity(ChaMatrix* pdst)
{
	if (!pdst){
		return;
	}
	pdst->data[0] = 1.0f;
	pdst->data[1] = 0.0f;
	pdst->data[2] = 0.0f;
	pdst->data[3] = 0.0f;

	pdst->data[4] = 0.0f;
	pdst->data[5] = 1.0f;
	pdst->data[6] = 0.0f;
	pdst->data[7] = 0.0f;

	pdst->data[8] = 0.0f;
	pdst->data[9] = 0.0f;
	pdst->data[10] = 1.0f;
	pdst->data[11] = 0.0f;

	pdst->data[12] = 0.0f;
	pdst->data[13] = 0.0f;
	pdst->data[14] = 0.0f;
	pdst->data[15] = 1.0f;
}

void ChaMatrixNormalizeRot(ChaMatrix* pdst)
{
	if (!pdst) {
		return;
	}

	ChaMatrix srcmat = *pdst;

	ChaVector3 vecx, vecy, vecz;
	vecx.x = srcmat.data[0];
	vecx.y = srcmat.data[1];
	vecx.z = srcmat.data[2];

	vecy.x = srcmat.data[4];
	vecy.y = srcmat.data[5];
	vecy.z = srcmat.data[6];

	vecz.x = srcmat.data[8];
	vecz.y = srcmat.data[9];
	vecz.z = srcmat.data[10];

	ChaVector3Normalize(&vecx, &vecx);
	ChaVector3Normalize(&vecy, &vecy);
	ChaVector3Normalize(&vecz, &vecz);

	pdst->data[0] = vecx.x;
	pdst->data[1] = vecx.y;
	pdst->data[2] = vecx.z;

	pdst->data[4] = vecy.x;
	pdst->data[5] = vecy.y;
	pdst->data[6] = vecy.z;

	pdst->data[8] = vecz.x;
	pdst->data[9] = vecz.y;
	pdst->data[10] = vecz.z;


}

void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, const ChaMatrix* psrc)
{
	//https://lxjk.github.io/2020/02/07/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained-JP.html

	if (!pdst || !psrc) {
		return;
	}

	ChaMatrix res;
	// use block matrix method
	// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or detA in code) is determinant, tr(A) is trace

	// sub matrices
	__m128 A = VecShuffle_0101(psrc->mVec[0], psrc->mVec[1]);
	__m128 B = VecShuffle_2323(psrc->mVec[0], psrc->mVec[1]);
	__m128 C = VecShuffle_0101(psrc->mVec[2], psrc->mVec[3]);
	__m128 D = VecShuffle_2323(psrc->mVec[2], psrc->mVec[3]);

#if 0
	__m128 detA = _mm_set1_ps(psrc->m[0][0] * psrc->m[1][1] - psrc->m[0][1] * psrc->m[1][0]);
	__m128 detB = _mm_set1_ps(psrc->m[0][2] * psrc->m[1][3] - psrc->m[0][3] * psrc->m[1][2]);
	__m128 detC = _mm_set1_ps(psrc->m[2][0] * psrc->m[3][1] - psrc->m[2][1] * psrc->m[3][0]);
	__m128 detD = _mm_set1_ps(psrc->m[2][2] * psrc->m[3][3] - psrc->m[2][3] * psrc->m[3][2]);
#else
	// determinant as (|A| |B| |C| |D|)
	__m128 detSub = _mm_sub_ps(
		_mm_mul_ps(VecShuffle(psrc->mVec[0], psrc->mVec[2], 0, 2, 0, 2), VecShuffle(psrc->mVec[1], psrc->mVec[3], 1, 3, 1, 3)),
		_mm_mul_ps(VecShuffle(psrc->mVec[0], psrc->mVec[2], 1, 3, 1, 3), VecShuffle(psrc->mVec[1], psrc->mVec[3], 0, 2, 0, 2))
	);
	__m128 detA = VecSwizzle1(detSub, 0);
	__m128 detB = VecSwizzle1(detSub, 1);
	__m128 detC = VecSwizzle1(detSub, 2);
	__m128 detD = VecSwizzle1(detSub, 3);
#endif

	// let iM = 1/|M| * | X  Y |
	//                  | Z  W |

	// D#C
	__m128 D_C = Mat2AdjMul(D, C);
	// A#B
	__m128 A_B = Mat2AdjMul(A, B);
	// X# = |D|A - B(D#C)
	__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	// W# = |A|D - C(A#B)
	__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	// |M| = |A|*|D| + ... (continue later)
	__m128 detM = _mm_mul_ps(detA, detD);

	// Y# = |B|C - D(A#B)#
	__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	// Z# = |C|B - A(D#C)#
	__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));

	// |M| = |A|*|D| + |B|*|C| ... (continue later)
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	// tr((A#B)(D#C))
	__m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
	detM = _mm_sub_ps(detM, tr);


	float checkdetM4[4];
	_mm_store_ps(checkdetM4, detM);
	if (fabs(checkdetM4[3]) >= 1e-5) {
		const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
		// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
		__m128 rDetM = _mm_div_ps(adjSignMask, detM);

		X_ = _mm_mul_ps(X_, rDetM);
		Y_ = _mm_mul_ps(Y_, rDetM);
		Z_ = _mm_mul_ps(Z_, rDetM);
		W_ = _mm_mul_ps(W_, rDetM);

		// apply adjugate and store, here we combine adjugate shuffle and store shuffle
		res.mVec[0] = VecShuffle(X_, Y_, 3, 1, 3, 1);
		res.mVec[1] = VecShuffle(X_, Y_, 2, 0, 2, 0);
		res.mVec[2] = VecShuffle(Z_, W_, 3, 1, 3, 1);
		res.mVec[3] = VecShuffle(Z_, W_, 2, 0, 2, 0);

		*pdst = res;
	}
	else {
		*pdst = *psrc;
	}

}

//void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, const ChaMatrix* psrc)
//{
//	if (!pdst || !psrc) {
//		return;
//	}
//
//	ChaMatrix res;
//	double detA;
//	double a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41, a42, a43, a44;
//	double b11, b12, b13, b14, b21, b22, b23, b24, b31, b32, b33, b34, b41, b42, b43, b44;
//
//	a11 = psrc->_11;
//	a12 = psrc->_12;
//	a13 = psrc->_13;
//	a14 = psrc->_14;
//
//	a21 = psrc->_21;
//	a22 = psrc->_22;
//	a23 = psrc->_23;
//	a24 = psrc->_24;
//
//	a31 = psrc->_31;
//	a32 = psrc->_32;
//	a33 = psrc->_33;
//	a34 = psrc->_34;
//
//	a41 = psrc->_41;
//	a42 = psrc->_42;
//	a43 = psrc->_43;
//	a44 = psrc->_44;
//
//	detA = (a11 * a22 * a33 * a44) + (a11 * a23 * a34 * a42) + (a11 * a24 * a32 * a43)
//		+ (a12 * a21 * a34 * a43) + (a12 * a23 * a31 * a44) + (a12 * a24 * a33 * a41)
//		+ (a13 * a21 * a32 * a44) + (a13 * a22 * a34 * a41) + (a13 * a24 * a31 * a42)
//		+ (a14 * a21 * a33 * a42) + (a14 * a22 * a31 * a43) + (a14 * a23 * a32 * a41)
//		- (a11 * a22 * a34 * a43) - (a11 * a23 * a32 * a44) - (a11 * a24 * a33 * a42)
//		- (a12 * a21 * a33 * a44) - (a12 * a23 * a34 * a41) - (a12 * a24 * a31 * a43)
//		- (a13 * a21 * a34 * a42) - (a13 * a22 * a31 * a44) - (a13 * a24 * a32 * a41)
//		- (a14 * a21 * a32 * a43) - (a14 * a22 * a33 * a41) - (a14 * a23 * a31 * a42);
//
//	if (pdet) {
//		*pdet = (float)detA;
//	}
//
//
//	if (detA == 0.0) {
//		*pdst = *psrc;
//		return;//!!!!!!!!!!!!!!!!!!!!!!!!!!
//	}
//
//
//
//
//	b11 = (a22 * a33 * a44) + (a23 * a34 * a42) + (a24 * a32 * a43) - (a22 * a34 * a43) - (a23 * a32 * a44) - (a24 * a33 * a42);
//	b12 = (a12 * a34 * a43) + (a13 * a32 * a44) + (a14 * a33 * a42) - (a12 * a33 * a44) - (a13 * a34 * a42) - (a14 * a32 * a43);
//	b13 = (a12 * a23 * a44) + (a13 * a24 * a42) + (a14 * a22 * a43) - (a12 * a24 * a43) - (a13 * a22 * a44) - (a14 * a23 * a42);
//	b14 = (a12 * a24 * a33) + (a13 * a22 * a34) + (a14 * a23 * a32) - (a12 * a23 * a34) - (a13 * a24 * a32) - (a14 * a22 * a33);
//
//	b21 = (a21 * a34 * a43) + (a23 * a31 * a44) + (a24 * a33 * a41) - (a21 * a33 * a44) - (a23 * a34 * a41) - (a24 * a31 * a43);
//	b22 = (a11 * a33 * a44) + (a13 * a34 * a41) + (a14 * a31 * a43) - (a11 * a34 * a43) - (a13 * a31 * a44) - (a14 * a33 * a41);
//	b23 = (a11 * a24 * a43) + (a13 * a21 * a44) + (a14 * a23 * a41) - (a11 * a23 * a44) - (a13 * a24 * a41) - (a14 * a21 * a43);
//	b24 = (a11 * a23 * a34) + (a13 * a24 * a31) + (a14 * a21 * a33) - (a11 * a24 * a33) - (a13 * a21 * a34) - (a14 * a23 * a31);
//
//	b31 = (a21 * a32 * a44) + (a22 * a34 * a41) + (a24 * a31 * a42) - (a21 * a34 * a42) - (a22 * a31 * a44) - (a24 * a32 * a41);
//	b32 = (a11 * a34 * a42) + (a12 * a31 * a44) + (a14 * a32 * a41) - (a11 * a32 * a44) - (a12 * a34 * a41) - (a14 * a31 * a42);
//	b33 = (a11 * a22 * a44) + (a12 * a24 * a41) + (a14 * a21 * a42) - (a11 * a24 * a42) - (a12 * a21 * a44) - (a14 * a22 * a41);
//	b34 = (a11 * a24 * a32) + (a12 * a21 * a34) + (a14 * a22 * a31) - (a11 * a22 * a34) - (a12 * a24 * a31) - (a14 * a21 * a32);
//
//	b41 = (a21 * a33 * a42) + (a22 * a31 * a43) + (a23 * a32 * a41) - (a21 * a32 * a43) - (a22 * a33 * a41) - (a23 * a31 * a42);
//	b42 = (a11 * a32 * a43) + (a12 * a33 * a41) + (a13 * a31 * a42) - (a11 * a33 * a42) - (a12 * a31 * a43) - (a13 * a32 * a41);
//	b43 = (a11 * a23 * a42) + (a12 * a21 * a43) + (a13 * a22 * a41) - (a11 * a22 * a43) - (a12 * a23 * a41) - (a13 * a21 * a42);
//	b44 = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a11 * a23 * a32) - (a12 * a21 * a33) - (a13 * a22 * a31);
//
//	res._11 = (float)(b11 / detA);
//	res._12 = (float)(b12 / detA);
//	res._13 = (float)(b13 / detA);
//	res._14 = (float)(b14 / detA);
//
//	res._21 = (float)(b21 / detA);
//	res._22 = (float)(b22 / detA);
//	res._23 = (float)(b23 / detA);
//	res._24 = (float)(b24 / detA);
//
//	res._31 = (float)(b31 / detA);
//	res._32 = (float)(b32 / detA);
//	res._33 = (float)(b33 / detA);
//	res._34 = (float)(b34 / detA);
//
//	res._41 = (float)(b41 / detA);
//	res._42 = (float)(b42 / detA);
//	res._43 = (float)(b43 / detA);
//	res._44 = (float)(b44 / detA);
//
//	*pdst = res;
//}


void ChaMatrixTranslation(ChaMatrix* pdst, float srcx, float srcy, float srcz)
{
	if (!pdst){
		return;
	}

	pdst->data[12] = srcx;
	pdst->data[13] = srcy;
	pdst->data[14] = srcz;
}

void ChaMatrixTranspose(ChaMatrix* pdst, ChaMatrix* psrc)
{
	pdst->data[0] = psrc->data[0];
	pdst->data[1] = psrc->data[4];
	pdst->data[2] = psrc->data[8];
	pdst->data[3] = psrc->data[12];

	pdst->data[4] = psrc->data[1];
	pdst->data[5] = psrc->data[5];
	pdst->data[6] = psrc->data[9];
	pdst->data[7] = psrc->data[13];

	pdst->data[8] = psrc->data[2];
	pdst->data[9] = psrc->data[6];
	pdst->data[10] = psrc->data[10];
	pdst->data[11] = psrc->data[14];

	pdst->data[12] = psrc->data[3];
	pdst->data[13] = psrc->data[7];
	pdst->data[14] = psrc->data[11];
	pdst->data[15] = psrc->data[15];

	//float m[4][4];
	//m[0][0] = psrc->data[0];
	//m[0][1] = psrc->data[1];
	//m[0][2] = psrc->data[2];
	//m[0][3] = psrc->data[3];
	//m[1][0] = psrc->data[4];
	//m[1][1] = psrc->data[5];
	//m[1][2] = psrc->data[6];
	//m[1][3] = psrc->data[7];
	//m[2][0] = psrc->data[8];
	//m[2][1] = psrc->data[9];
	//m[2][2] = psrc->data[10];
	//m[2][3] = psrc->data[11];
	//m[3][0] = psrc->data[12];
	//m[3][1] = psrc->data[13];
	//m[3][2] = psrc->data[14];
	//m[3][3] = psrc->data[15];

	//pdst->data[0] = m[0][0];
	//pdst->data[1] = m[1][0];
	//pdst->data[2] = m[2][0];
	//pdst->data[3] = m[3][0];

	//pdst->data[4] = m[0][1];
	//pdst->data[5] = m[1][1];
	//pdst->data[6] = m[2][1];
	//pdst->data[7] = m[3][1];

	//pdst->data[8] = m[0][2];
	//pdst->data[9] = m[1][2];
	//pdst->data[10] = m[2][2];
	//pdst->data[11] = m[3][2];

	//pdst->data[12] = m[0][3];
	//pdst->data[13] = m[1][3];
	//pdst->data[14] = m[2][3];
	//pdst->data[15] = m[3][3];

}

double ChaVector3LengthDbl(ChaVector3* v)
{
	if (!v) {
		return 0.0f;
	}

	double leng;
	double mag;
	mag = (double)v->x * (double)v->x + (double)v->y * (double)v->y + (double)v->z * (double)v->z;
	if (mag != 0.0) {
		leng = sqrt(mag);
	}
	else {
		leng = 0.0;
	}
	return leng;

}

double ChaVector3DotDbl(const ChaVector3* psrc1, const ChaVector3* psrc2)
{
	if (!psrc1 || !psrc2) {
		return 0.0f;
	}

	double retval = (double)psrc1->x * (double)psrc2->x + (double)psrc1->y * (double)psrc2->y + (double)psrc1->z * (double)psrc2->z;

	return retval;
}


//double ChaVector3LengthDbl(ChaVector3* v)
//{
//	if (!v){
//		return 0.0f;
//	}
//
//	double leng;
//	double mag;
//	mag = (double)v->x * (double)v->x + (double)v->y * (double)v->y + (double)v->z * (double)v->z;
//	if (mag != 0.0f){
//		leng = (float)sqrt(mag);
//	}
//	else{
//		leng = 0.0;
//	}
//	return leng;
//
//}

void ChaVector3Normalize(ChaVector3* pdst, const ChaVector3* psrc){
	if (!pdst || !psrc){
		return;
	}

	__m128 srcx = _mm_load1_ps(&psrc->x);
	__m128 srcy = _mm_load1_ps(&psrc->y);
	__m128 srcz = _mm_load1_ps(&psrc->z);

	__m128 mulx = _mm_mul_ps(srcx, srcx);
	__m128 maddxy = _mm_madd_ps(srcy, srcy, mulx);
	__m128 maddxyz = _mm_madd_ps(srcz, srcz, maddxy);

	float check[4];
	_mm_store_ps(check, maddxyz);
	if (check[0] >= 1e-7) {
		__m128 invsqrt = _mm_rsqrt_ps(maddxyz);//平方根の逆数

		__m128 srcxyz1 = _mm_setr_ps(psrc->x, psrc->y, psrc->z, 0.0f);
		__m128 mresult = _mm_mul_ps(srcxyz1, invsqrt);

		float result[4];
		_mm_store_ps(result, mresult);

		pdst->x = result[0];
		pdst->y = result[1];
		pdst->z = result[2];
	}
	else {
		//*pdst = ChaVector3(0.0f, 0.0f, 0.0f);
		pdst->x = psrc->x;
		pdst->y = psrc->y;
		pdst->z = psrc->z;
	}

	//ChaVector3 src = *psrc;
	//double mag = (double)src.x * (double)src.x + (double)src.y * (double)src.y + (double)src.z * (double)src.z;
	//if (mag != 0.0){
	//	double divval = ::sqrt(mag);
	//	if (divval != 0.0) {
	//		double tmpx = src.x / divval;
	//		double tmpy = src.y / divval;
	//		double tmpz = src.z / divval;
	//		pdst->x = (float)tmpx;
	//		pdst->y = (float)tmpy;
	//		pdst->z = (float)tmpz;
	//	}
	//	else {
	//		pdst->x = src.x;
	//		pdst->y = src.y;
	//		pdst->z = src.z;
	//	}
	//}
	//else{
	//	//*pdst = ChaVector3(0.0f, 0.0f, 0.0f);
	//	pdst->x = src.x;
	//	pdst->y = src.y;
	//	pdst->z = src.z;
	//}
}

float ChaVector3Dot(const ChaVector3* psrc1, const ChaVector3* psrc2)
{
	if (!psrc1 || !psrc2){
		return 0.0f;
	}

	return (psrc1->x * psrc2->x + psrc1->y * psrc2->y + psrc1->z * psrc2->z);

}

void ChaVector3Cross(ChaVector3* pdst, const ChaVector3* psrc1, const ChaVector3* psrc2)
{
	if (!pdst || !psrc1 || !psrc2){
		return;
	}

	//ChaVector3 v1, v2;
	//v1 = *psrc1;
	//v2 = *psrc2;

	//pdst->x = v1.y * v2.z - v1.z * v2.y;
	//pdst->y = v1.z * v2.x - v1.x * v2.z;
	//pdst->z = v1.x * v2.y - v1.y * v2.x;
	pdst->x = (float)((double)psrc1->y * (double)psrc2->z - (double)psrc1->z * (double)psrc2->y);
	pdst->y = (float)((double)psrc1->z * (double)psrc2->x - (double)psrc1->x * (double)psrc2->z);
	pdst->z = (float)((double)psrc1->x * (double)psrc2->y - (double)psrc1->y * (double)psrc2->x);
}


void ChaVector3TransformCoord(ChaVector3* dstvec, ChaVector3* srcvec, ChaMatrix* srcmat)
{
	if (!dstvec || !srcvec || !srcmat){
		return;
	}

	//float src4fl[4] = { srcvec->x, srcvec->y, srcvec->z, 1.0f };
	//__m128 ma_col_0 = _mm_load_ps(src4fl);
	const __m128 ma_col_0 = _mm_setr_ps(srcvec->x, srcvec->y, srcvec->z, 1.0f);


	__m128 resultX;
	__m128 resultXY;
	__m128 resultXYZ;
	__m128 resultXYZW;
	resultX = _mm_mul_ps(srcmat->mVec[0], _mm_replicate_x_ps(ma_col_0));
	resultXY = _mm_madd_ps(srcmat->mVec[1], _mm_replicate_y_ps(ma_col_0), resultX);
	resultXYZ = _mm_madd_ps(srcmat->mVec[2], _mm_replicate_z_ps(ma_col_0), resultXY);
	resultXYZW = _mm_madd_ps(srcmat->mVec[3], _mm_replicate_w_ps(ma_col_0), resultXYZ);


	//x/tmpw, y/tmpw, z/tmpw, w/tmpw
	float checkw4[4];
	_mm_store_ps(checkw4, resultXYZW);
	if (fabs(checkw4[3]) >= 1e-5) {
		const __m128 inverseOne = _mm_setr_ps(1.f, 1.f, 1.f, 1.f);
		const __m128 resultw = VecSwizzle1(resultXYZW, 3);
		__m128 rDetM = _mm_div_ps(inverseOne, resultw);
		resultXYZW = _mm_mul_ps(resultXYZW, rDetM);


		float resultfl4[4];
		_mm_store_ps(resultfl4, resultXYZW);

		dstvec->x = resultfl4[0];
		dstvec->y = resultfl4[1];
		dstvec->z = resultfl4[2];
	}
	else {
		//dstvec->x = 0.0f;
		//dstvec->y = 0.0f;
		//dstvec->z = 0.0f;
		*dstvec = *srcvec;
	}

	//tmpx = (double)srcmat->_11 * (double)srcvec->x + (double)srcmat->data[4] * (double)srcvec->y + (double)srcmat->data[8] * (double)srcvec->z + (double)srcmat->data[12];
	//tmpy = (double)srcmat->_12 * (double)srcvec->x + (double)srcmat->data[5] * (double)srcvec->y + (double)srcmat->data[9] * (double)srcvec->z + (double)srcmat->data[13];
	//tmpz = (double)srcmat->_13 * (double)srcvec->x + (double)srcmat->data[6] * (double)srcvec->y + (double)srcmat->data[10] * (double)srcvec->z + (double)srcmat->data[14];
	//tmpw = (double)srcmat->data[3] * (double)srcvec->x + (double)srcmat->data[7] * (double)srcvec->y + (double)srcmat->data[11] * (double)srcvec->z + (double)srcmat->data[15];

	//if (tmpw != 0.0){
	//	dstvec->x = (float)(tmpx / tmpw);
	//	dstvec->y = (float)(tmpy / tmpw);
	//	dstvec->z = (float)(tmpz / tmpw);
	//}
	//else{
	//	//dstvec->x = 0.0f;
	//	//dstvec->y = 0.0f;
	//	//dstvec->z = 0.0f;
	//	*dstvec = *srcvec;
	//}
}

/*
D3DXVECTOR3 *WINAPI D3DXVec3TransformNormal(D3DXVECTOR3 *pOut,
CONST D3DXVECTOR3 *pV,
CONST D3DXMATRIX *pM
);
パラメータ

pOut
[in, out] 演算結果である D3DXVECTOR3 構造体へのポインタ。
pV
[in] 処理の基になる D3DXVECTOR3 構造体へのポインタ。
pM
[in] 処理の基になる D3DXMATRIX 構造体へのポインタ。
戻り値

トランスフォームされたベクトルの D3DXVECTOR3 構造体へのポインタ。

注意

この関数は、ベクトル pV のベクトル法線(x, y, z, 0) を行列 pM でトランスフォームする。
*/
ChaVector3* ChaVector3TransformNormal(ChaVector3 *dstvec, const ChaVector3* srcvec, const ChaMatrix* srcmat)
{
	if (!dstvec || !srcvec || !srcmat) {
		return NULL;
	}

	double tmpx, tmpy, tmpz, tmpw;
	tmpx = (double)srcmat->data[0] * (double)srcvec->x + (double)srcmat->data[4] * (double)srcvec->y + (double)srcmat->data[8] * (double)srcvec->z;// +(double)srcmat->data[12];
	tmpy = (double)srcmat->data[1] * (double)srcvec->x + (double)srcmat->data[5] * (double)srcvec->y + (double)srcmat->data[9] * (double)srcvec->z;// + (double)srcmat->data[13];
	tmpz = (double)srcmat->data[2] * (double)srcvec->x + (double)srcmat->data[6] * (double)srcvec->y + (double)srcmat->data[10] * (double)srcvec->z;// + (double)srcmat->data[14];
	tmpw = (double)srcmat->data[3] * (double)srcvec->x + (double)srcmat->data[7] * (double)srcvec->y + (double)srcmat->data[11] * (double)srcvec->z;// + (double)srcmat->data[15];

	if (tmpw != 0.0) {
		dstvec->x = (float)(tmpx / tmpw);
		dstvec->y = (float)(tmpy / tmpw);
		dstvec->z = (float)(tmpz / tmpw);
	}
	else {
		//dstvec->x = 0.0f;
		//dstvec->y = 0.0f;
		//dstvec->z = 0.0f;
		*dstvec = *srcvec;
	}

	return dstvec;
}



double ChaVector3LengthSq(ChaVector3* psrc)
{
	if (!psrc) {
		return 0.0f;
	}

	double mag = (double)psrc->x * (double)psrc->x + (double)psrc->y * (double)psrc->y + (double)psrc->z * (double)psrc->z;
	return mag;

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

	pdst->data[0] = srcx;
	pdst->data[5] = srcy;
	pdst->data[10] = srcz;
}

void ChaMatrixLookAtRH(ChaMatrix* dstviewmat, ChaVector3* camEye, ChaVector3* camtar, ChaVector3* camUpVec)
{
/*
zaxis = normal(Eye - At)
xaxis = normal(cross(Up, zaxis))
yaxis = cross(zaxis, xaxis)

xaxis.x           yaxis.x           zaxis.x          0
xaxis.y           yaxis.y           zaxis.y          0
xaxis.z           yaxis.z           zaxis.z          0
-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
*/
	ChaVector3 zaxis;
	ChaVector3 yaxis;
	ChaVector3 xaxis;

	zaxis = *camEye - *camtar;
	ChaVector3Normalize(&zaxis, &zaxis);

	ChaVector3Cross(&xaxis, (const ChaVector3*)camUpVec, (const ChaVector3*)&zaxis);
	ChaVector3Normalize(&xaxis, &xaxis);

	ChaVector3Cross(&yaxis, (const ChaVector3*)&zaxis, (const ChaVector3*)&xaxis);
	ChaVector3Normalize(&yaxis, &yaxis);


	float trax, tray, traz;
	trax = -ChaVector3Dot(&xaxis, camEye);
	tray = -ChaVector3Dot(&yaxis, camEye);
	traz = -ChaVector3Dot(&zaxis, camEye);


	dstviewmat->data[0] = xaxis.x;
	dstviewmat->data[4] = xaxis.y;
	dstviewmat->data[8] = xaxis.z;
	dstviewmat->data[12] = trax;

	dstviewmat->data[1] = yaxis.x;
	dstviewmat->data[5] = yaxis.y;
	dstviewmat->data[9] = yaxis.z;
	dstviewmat->data[13] = tray;

	dstviewmat->data[2] = zaxis.x;
	dstviewmat->data[6] = zaxis.y;
	dstviewmat->data[10] = zaxis.z;
	dstviewmat->data[14] = traz;

	dstviewmat->data[3] = 0.0f;
	dstviewmat->data[7] = 0.0f;
	dstviewmat->data[11] = 0.0f;
	dstviewmat->data[15] = 1.0f;


}

/*
D3DXMATRIX *D3DXMatrixOrthoOffCenterRH(          D3DXMATRIX *pOut,
FLOAT l,
FLOAT r,
FLOAT t,
FLOAT b,
FLOAT zn,
FLOAT zf
);

2/(r-l)      0            0           0
0            2/(t-b)      0           0
0            0            1/(zn-zf)   0
(l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1
*/

ChaMatrix* ChaMatrixOrthoOffCenterRH(ChaMatrix* pOut, float l, float r, float t, float b, float zn, float zf) 
{
	if (!pOut) {
		return NULL;
	}

	if ((r == l) || (t == b) || (zn == zf)) {
		ChaMatrixIdentity(pOut);
		return pOut;
	}


	pOut->data[0] = (float)(2.0 / ((double)r - (double)l));
	pOut->data[1] = 0.0f;
	pOut->data[2] = 0.0f;
	pOut->data[3] = 0.0f;

	pOut->data[4] = 0.0f;
	pOut->data[5] = (float)(2.0 / ((double)t - (double)b));
	pOut->data[6] = 0.0f;
	pOut->data[7] = 0.0f;

	pOut->data[8] = 0.0f;
	pOut->data[9] = 0.0f;
	pOut->data[10] = (float)(1.0 / ((double)zn - (double)zf));
	pOut->data[11] = 0.0f;

	pOut->data[12] = (float)(((double)l + (double)r) / ((double)l - (double)r));
	pOut->data[13] = (float)(((double)t + (double)b) / ((double)b - (double)t));
	pOut->data[14] = (float)((double)zn / ((double)zn - (double)zf));
	pOut->data[15] = 1.0f;

	return pOut;
}


/*
D3DXMATRIX *D3DXMatrixPerspectiveFovRH(          D3DXMATRIX *pOut,
FLOAT fovY,
FLOAT Aspect,
FLOAT zn,
FLOAT zf
);

w       0       0                0
0       h       0                0
0       0       zf/(zn-zf)      -1
0       0       zn*zf/(zn-zf)    0
where:
h is the view space height. It is calculated from
h = cot(fovY/2);

w is the view space width. It is calculated from
w = h / Aspect.
*/

ChaMatrix* ChaMatrixPerspectiveFovRH(ChaMatrix* pOut, float fovY, float Aspect, float zn, float zf)
{
	if (!pOut) {
		return NULL;
	}
	if (zn == zf) {
		ChaMatrixIdentity(pOut);
		return pOut;
	}


	double h, w;
	double c, s;
	c = cos((double)fovY / 2.0);
	s = sin((double)fovY / 2.0);
	if ((s != 0.0) && (Aspect != 0.0f)) {
		h = c / s;
		w = h / (double)Aspect;

		pOut->data[0] = (float)w;
		pOut->data[1] = 0.0f;
		pOut->data[2] = 0.0f;
		pOut->data[3] = 0.0f;

		pOut->data[4] = 0;
		pOut->data[5] = (float)h;
		pOut->data[6] = 0.0f;
		pOut->data[7] = 0.0f;

		pOut->data[8] = 0.0f;
		pOut->data[9] = 0.0f;
		pOut->data[10] = (float)((double)zf / ((double)zn - (double)zf));
		pOut->data[11] = -1.0f;

		pOut->data[12] = 0.0f;
		pOut->data[13] = 0.0f;
		pOut->data[14] = (float)((double)zn * (double)zf / ((double)zn - (double)zf));
		pOut->data[15] = 0.0f;
	}
	else {
		ChaMatrixIdentity(pOut);
	}

	return pOut;

}



const ChaMatrix* ChaMatrixRotationQuaternion(ChaMatrix* dstmat, CQuaternion* srcq)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	if (!dstmat || !srcq) {
		return NULL;
	}

	retmat = srcq->MakeRotMatX();
	*dstmat = retmat;
	return (const ChaMatrix*)dstmat;
}

/*
pOut
[in, out] 演算結果である D3DXMATRIX 構造体へのポインタ。
Yaw
[in] y 軸を中心とするヨー(ラジアン単位)。
Pitch
[in] x 軸を中心とするピッチ(ラジアン単位)。
Roll
[in] z 軸を中心とするロール(ラジアン単位)。
戻り値

指定されたヨー･ピッチ･ロールを持つ D3DXMATRIX 構造体へのポインタ。

注意

この関数の戻り値は、pOut パラメータの戻り値と同じである。したがって、D3DXMatrixRotationYawPitchRoll 関数を別の関数の引数として使える。

トランスフォームの順序は、最初にロール、次にピッチ、最後にヨーである。これは、オブジェクトのローカル座標軸を基準として、z 軸の周囲での回転、x 軸の周囲での回転、y 軸の周囲での回転と同じになる。
*/
ChaMatrix* ChaMatrixRotationYawPitchRoll(ChaMatrix* pOut, float srcyaw, float srcpitch, float srcroll)
{
	if (!pOut) {
		return NULL;
	}

	ChaVector3 axisy = ChaVector3(0.0f, 1.0f, 0.0f);
	ChaVector3 axisx = ChaVector3(1.0f, 0.0f, 0.0f);
	ChaVector3 axisz = ChaVector3(0.0f, 0.0f, 1.0f);

	CQuaternion qyaw, qpitch, qroll;
	qyaw.SetAxisAndRot(axisy, srcyaw);
	qpitch.SetAxisAndRot(axisx, srcpitch);
	qroll.SetAxisAndRot(axisz, srcroll);

	CQuaternion qrot;
	qrot = qyaw * qpitch * qroll;

	*pOut = qrot.MakeRotMatX();
	return pOut;
}

ChaMatrix* ChaMatrixRotationX(ChaMatrix* pOut, float srcrad)
{
	if (!pOut) {
		return NULL;
	}

	ChaVector3 axisx = ChaVector3(1.0f, 0.0f, 0.0f);
	CQuaternion qx;
	qx.SetAxisAndRot(axisx, srcrad);

	*pOut = qx.MakeRotMatX();
	return pOut;
}

ChaMatrix* ChaMatrixRotationY(ChaMatrix* pOut, float srcrad)
{
	if (!pOut) {
		return NULL;
	}

	ChaVector3 axisy = ChaVector3(0.0f, 1.0f, 0.0f);
	CQuaternion qy;
	qy.SetAxisAndRot(axisy, srcrad);

	*pOut = qy.MakeRotMatX();
	return pOut;
}

ChaMatrix* ChaMatrixRotationZ(ChaMatrix* pOut, float srcrad)
{
	if (!pOut) {
		return NULL;
	}

	ChaVector3 axisz = ChaVector3(0.0f, 0.0f, 1.0f);
	CQuaternion qz;
	qz.SetAxisAndRot(axisz, srcrad);

	*pOut = qz.MakeRotMatX();
	return pOut;
}

void CQuaternionIdentity(CQuaternion* dstq)
{
	if (!dstq) {
		return;
	}
	dstq->x = 0.0f;
	dstq->y = 0.0f;
	dstq->z = 0.0f;
	dstq->w = 1.0f;

}

CQuaternion CQuaternionInv(CQuaternion srcq)
{
	CQuaternion invq;
	srcq.inv(&invq);
	return invq;
}


ChaMatrix MakeRotMatFromChaMatrix(ChaMatrix srcmat)
{
	CQuaternion tmpq;
	tmpq.MakeFromD3DXMat(srcmat);
	return tmpq.MakeRotMatX();
}

ChaMatrix ChaMatrixTranspose(ChaMatrix srcmat)
{
	ChaMatrix tmpmat = srcmat;
	ChaMatrix retmat;


	retmat.data[0] = tmpmat.data[0];
	retmat.data[1] = tmpmat.data[4];
	retmat.data[2] = tmpmat.data[8];
	retmat.data[3] = tmpmat.data[12];

	retmat.data[4] = tmpmat.data[1];
	retmat.data[5] = tmpmat.data[5];
	retmat.data[6] = tmpmat.data[9];
	retmat.data[7] = tmpmat.data[13];

	retmat.data[8] = tmpmat.data[2];
	retmat.data[9] = tmpmat.data[6];
	retmat.data[10] = tmpmat.data[10];
	retmat.data[11] = tmpmat.data[14];

	retmat.data[12] = tmpmat.data[3];
	retmat.data[13] = tmpmat.data[7];
	retmat.data[14] = tmpmat.data[11];
	retmat.data[15] = tmpmat.data[15];


	return retmat;
}





#ifdef CONVD3DX11
DirectX::XMFLOAT2 ChaVector2::D3DX()
{
	DirectX::XMFLOAT2 retv;
	retv.x = x;
	retv.y = y;
	return retv;
}

DirectX::XMFLOAT3 ChaVector3::D3DX()
{
	DirectX::XMFLOAT3 retv;
	retv.x = x;
	retv.y = y;
	retv.z = z;
	return retv;
}
DirectX::XMVECTOR ChaVector3::XMVECTOR(float w)
{
	DirectX::XMVECTOR retv;
	retv.m128_f32[0] = x;
	retv.m128_f32[1] = y;
	retv.m128_f32[2] = z;
	retv.m128_f32[3] = w;
	return retv;
}


DirectX::XMFLOAT4 ChaVector4::D3DX()
{
	DirectX::XMFLOAT4 retv;
	retv.x = x;
	retv.y = y;
	retv.z = z;
	retv.w = w;
	return retv;
}


ChaMatrix ChaMatrix::operator= (DirectX::XMMATRIX m) {
	this->data[0] = m.r[0].m128_f32[0];
	this->data[1] = m.r[0].m128_f32[1];
	this->data[2] = m.r[0].m128_f32[2];
	this->data[3] = m.r[0].m128_f32[3];

	this->data[4] = m.r[1].m128_f32[0];
	this->data[5] = m.r[1].m128_f32[1];
	this->data[6] = m.r[1].m128_f32[2];
	this->data[7] = m.r[1].m128_f32[3];

	this->data[8] = m.r[2].m128_f32[0];
	this->data[9] = m.r[2].m128_f32[1];
	this->data[10] = m.r[2].m128_f32[2];
	this->data[11] = m.r[2].m128_f32[3];

	this->data[12] = m.r[3].m128_f32[0];
	this->data[13] = m.r[3].m128_f32[1];
	this->data[14] = m.r[3].m128_f32[2];
	this->data[15] = m.r[3].m128_f32[3];

	//this->_11 = m.r[0].m128_f32[0];
	//this->_12 = m.r[1].m128_f32[0];
	//this->_13 = m.r[2].m128_f32[0];
	//this->data[3] = m.r[3].m128_f32[0];

	//this->data[4] = m.r[0].m128_f32[1];
	//this->data[5] = m.r[1].m128_f32[1];
	//this->data[6] = m.r[2].m128_f32[1];
	//this->data[7] = m.r[3].m128_f32[1];

	//this->data[8] = m.r[0].m128_f32[2];
	//this->data[9] = m.r[1].m128_f32[2];
	//this->data[10] = m.r[2].m128_f32[2];
	//this->data[11] = m.r[3].m128_f32[2];

	//this->data[12] = m.r[0].m128_f32[3];
	//this->data[13] = m.r[1].m128_f32[3];
	//this->data[14] = m.r[2].m128_f32[3];
	//this->data[15] = m.r[3].m128_f32[3];


	return *this;
};

DirectX::XMMATRIX ChaMatrix::D3DX()
{
	DirectX::XMMATRIX retm;
	retm.r[0].m128_f32[0] = data[0];
	retm.r[0].m128_f32[1] = data[1];
	retm.r[0].m128_f32[2] = data[2];
	retm.r[0].m128_f32[3] = data[3];

	retm.r[1].m128_f32[0] = data[4];
	retm.r[1].m128_f32[1] = data[5];
	retm.r[1].m128_f32[2] = data[6];
	retm.r[1].m128_f32[3] = data[7];

	retm.r[2].m128_f32[0] = data[8];
	retm.r[2].m128_f32[1] = data[9];
	retm.r[2].m128_f32[2] = data[10];
	retm.r[2].m128_f32[3] = data[11];

	retm.r[3].m128_f32[0] = data[12];
	retm.r[3].m128_f32[1] = data[13];
	retm.r[3].m128_f32[2] = data[14];
	retm.r[3].m128_f32[3] = data[15];

	//retm.r[0].m128_f32[0] = _11;
	//retm.r[1].m128_f32[0] = data[1];
	//retm.r[2].m128_f32[0] = _13;
	//retm.r[3].m128_f32[0] = data[3];

	//retm.r[0].m128_f32[1] = data[4];
	//retm.r[2].m128_f32[1] = data[5];
	//retm.r[3].m128_f32[1] = data[6];
	//retm.r[4].m128_f32[1] = data[7];

	//retm.r[0].m128_f32[2] = data[8];
	//retm.r[1].m128_f32[2] = data[9];
	//retm.r[2].m128_f32[2] = data[10];
	//retm.r[3].m128_f32[2] = data[11];

	//retm.r[0].m128_f32[3] = data[12];
	//retm.r[1].m128_f32[3] = data[13];
	//retm.r[2].m128_f32[3] = data[14];
	//retm.r[3].m128_f32[3] = data[15];

	return retm;
}
#endif


N3P::N3P() {
	InitParams();
}
N3P::~N3P()
{
	if (perface) {
		free(perface);
		perface = 0;
	}
	if (pervert) {
		free(pervert);
		pervert = 0;
	}
	if (n3sm) {
		delete n3sm;
		n3sm = 0;
	}
}
void N3P::InitParams()
{
	perface = 0;
	pervert = 0;
	n3sm = 0;
}

N3SM::N3SM()
{
	InitParams();
}
N3SM::~N3SM()
{
	if (ppsmface) {
		free(ppsmface);
		ppsmface = 0;
	}
	smfacenum = 0;
}
void N3SM::InitParams()
{
	smfacenum = 0;
	ppsmface = 0;
}




