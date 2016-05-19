//#include "stdafx.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include <coef.h>
#include <crtdbg.h>

#include <quaternion.h>

#define DBGH
#include <dbg.h>

float vecDotVec( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 )
{
	return ( vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z );
}

float lengthVec( D3DXVECTOR3* vec )
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

float aCos( float dot )
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

int vec3RotateY( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion iniq;
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( &iniq, 0, deg, 0 );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}
int vec3RotateX( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion iniq; 
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( &iniq, deg, 0, 0 );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}
int vec3RotateZ( D3DXVECTOR3* dstvec, float deg, D3DXVECTOR3* srcvec )
{

	int ret;
	CQuaternion iniq;
	CQuaternion dirq;
	D3DXMATRIX	dirm;

	ret = dirq.SetRotation( &iniq, 0, 0, deg );
	_ASSERT( !ret );
	dirm = dirq.MakeRotMatX();

	dstvec->x = dirm._11 * srcvec->x + dirm._21 * srcvec->y + dirm._31 * srcvec->z + dirm._41;
	dstvec->y = dirm._12 * srcvec->x + dirm._22 * srcvec->y + dirm._32 * srcvec->z + dirm._42;
	dstvec->z = dirm._13 * srcvec->x + dirm._23 * srcvec->y + dirm._33 * srcvec->z + dirm._43;

	return 0;
}


int qToEulerAxis( CQuaternion axisQ, CQuaternion* srcq, D3DXVECTOR3* Euler )
{

	int ret;
	CQuaternion invaxisQ, EQ;
	ret = axisQ.inv( &invaxisQ );
	_ASSERT( !ret );
	EQ = invaxisQ * *srcq * axisQ;


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
		Euler->y = 90.0f;
	}else{
		Euler->y = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );		
	}
	if( vecDotVec( &shadowVec, &axisXVec ) < 0.0f ){
		Euler->y = -Euler->y;
	}

/***
	E3DMultQVec targetQ,0.0,0.0,1.0,targetVec.0,targetVec.1,targetVec.2
	shadowVec.0= vecDotVec(targetVec,axisXVec)
	shadowVec.1= 0.0
	shadowVec.2= vecDotVec(targetVec,axisZVec)
	if( lengthVec(shadowVec) == 0 ){
		eulerY= 90.0
	}else{
		E3DACos ( vecDotVec(shadowVec,axisZVec)/ lengthVec(shadowVec) ), eulerY		//Yé≤âÒì]ìxêî
	}
	if( vecDotVec(shadowVec,axisXVec) < 0.0 ){ eulerY= -eulerY }				//âEâÒÇËÇ…èCê≥
***/
	vec3RotateY( &tmpVec, -Euler->y, &targetVec );
	shadowVec.x = 0.0f;
	shadowVec.y = vecDotVec( &tmpVec, &axisYVec );
	shadowVec.z = vecDotVec( &tmpVec, &axisZVec );
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler->x = 90.0f;
	}else{
		Euler->x = aCos( vecDotVec( &shadowVec, &axisZVec ) / lengthVec( &shadowVec ) );
	}
	if( vecDotVec( &shadowVec, &axisYVec ) > 0.0f ){
		Euler->x = -Euler->x;
	}
/***
	E3DVec3RotateY targetVec.0,targetVec.1,targetVec.2,(-eulerY),shadowVec.0,shadowVec.1,shadowVec.2
	if( lengthVec(shadowVec) == 0 ){
		eulerX= 90.0
	}else{
		E3DACos ( vecDotVec(shadowVec,axisZVec)/ lengthVec(shadowVec) ), eulerX		//Xé≤âÒì]ìxêî
	}
	if( vecDotVec(shadowVec,axisYVec) > 0.0 ){ eulerX= -eulerX }				//âEâÒÇËÇ…èCê≥
***/

	EQ.Rotate( &targetVec, axisYVec );
	vec3RotateY( &tmpVec, -Euler->y, &targetVec );
	targetVec = tmpVec;
	vec3RotateX( &tmpVec, -Euler->x, &targetVec );
	shadowVec.x = vecDotVec( &tmpVec, &axisXVec );
	shadowVec.y = vecDotVec( &tmpVec, &axisYVec );
	shadowVec.z = 0.0f;
	if( lengthVec( &shadowVec ) == 0.0f ){
		Euler->z = 90.0f;
	}else{
		Euler->z = aCos( vecDotVec( &shadowVec, &axisYVec ) / lengthVec( &shadowVec ) );
	}
	if( vecDotVec( &shadowVec, &axisXVec ) > 0.0f ){
		Euler->z = -Euler->z;
	}
/***
	E3DMultQVec targetQ,0.0,1.0,0.0,targetVec.0,targetVec.1,targetVec.2
	E3DVec3RotateY (targetVec.0+0.0),(targetVec.1+0.0),(targetVec.2+0.0),(-eulerY),targetVec.0,targetVec.1,targetVec.2
	E3DVec3RotateY targetVec.2,targetVec.0,targetVec.1,(-eulerX),shadowVec.2,shadowVec.0,shadowVec.1
	Å@Å@----> à¯êîÇ™Z,X,YÇ…Ç»Ç¡ÇƒÇ¢ÇÈÇΩÇﬂRotateXÇ∆ìØÇ∂ÅIÅIÅI
	
	if( lengthVec(shadowVec) == 0 ){
		eulerZ= 90.0
	}else{
		E3DACos ( vecDotVec(shadowVec,axisYVec)/ lengthVec(shadowVec) ), eulerZ		//Zé≤âÒì]ìxêî
	}
	if( vecDotVec(shadowVec,axisXVec) > 0.0 ){ eulerZ= -eulerZ }				//âEâÒÇËÇ…èCê≥

***/
	return 0;
}



int GetRound( float srcval )
{
	if( srcval > 0.0f ){
		return (int)( srcval + 0.5f );
	}else{
		return (int)( srcval - 0.5f );
	}
}

int modifyEuler( D3DXVECTOR3* eulerA, D3DXVECTOR3* eulerB )
{

	//ÉIÉCÉâÅ[äpAÇÃílÇÉIÉCÉâÅ[äpBÇÃílÇ…ãﬂÇ¢ï\é¶Ç…èCê≥
	float tmpX1, tmpY1, tmpZ1;
	float tmpX2, tmpY2, tmpZ2;
	float s1, s2;

	//ó\ëzÇ≥ÇÍÇÈäpìx1
	tmpX1 = eulerA->x + 360.0f * GetRound( (eulerB->x - eulerA->x) / 360.0f );
	tmpY1 = eulerA->y + 360.0f * GetRound( (eulerB->y - eulerA->y) / 360.0f );
	tmpZ1 = eulerA->z + 360.0f * GetRound( (eulerB->z - eulerA->z) / 360.0f );

	//ó\ëzÇ≥ÇÍÇÈäpìx2
	tmpX2 = 180.0f - eulerA->x + 360.0f * GetRound( (eulerB->x + eulerA->x - 180.0f) / 360.0f );
	tmpY2 = eulerA->y + 180.0f + 360.0f * GetRound( (eulerB->y - eulerA->y - 180.0f) / 360.0f );
	tmpZ2 = eulerA->z + 180.0f + 360.0f * GetRound( (eulerB->z - eulerA->z - 180.0f) / 360.0f );

	//äpìxïœâªÇÃëÂÇ´Ç≥
	s1 = (eulerB->x - tmpX1) * (eulerB->x - tmpX1) + (eulerB->y - tmpY1) * (eulerB->y - tmpY1) + (eulerB->z - tmpZ1) * (eulerB->z - tmpZ1);
	s2 = (eulerB->x - tmpX2) * (eulerB->x - tmpX2) + (eulerB->y - tmpY2) * (eulerB->y - tmpY2) + (eulerB->z - tmpZ2) * (eulerB->z - tmpZ2);

	//ïœâªÇÃè≠Ç»Ç¢ï˚Ç…èCê≥
	if( s1 < s2 ){
		eulerA->x = tmpX1; eulerA->y = tmpY1; eulerA->z = tmpZ1;
	}else{
		eulerA->x = tmpX2; eulerA->y = tmpY2; eulerA->z = tmpZ2;
	}

	return 0;
}

int IsInitRot(D3DXMATRIX srcmat)
{
	int retval = 0;

	float d11, d12, d13;
	float d21, d22, d23;
	float d31, d32, d33;

	d11 = srcmat._11 - 1.0f;
	d12 = srcmat._12 - 0.0f;
	d13 = srcmat._13 - 0.0f;

	d21 = srcmat._21 - 0.0f;
	d22 = srcmat._22 - 1.0f;
	d23 = srcmat._23 - 0.0f;

	d31 = srcmat._11 - 0.0f;
	d32 = srcmat._12 - 0.0f;
	d33 = srcmat._13 - 1.0f;

	float dmin = 0.000001f;

	if ((fabs(d11) <= dmin) && (fabs(d12) <= dmin) && (fabs(d13) <= dmin) &&
		(fabs(d21) <= dmin) && (fabs(d22) <= dmin) && (fabs(d23) <= dmin) &&
		(fabs(d31) <= dmin) && (fabs(d32) <= dmin) && (fabs(d33) <= dmin)){
		retval = 1;
	}

	return retval;
}


int IsSameMat(D3DXMATRIX srcmat1, D3DXMATRIX srcmat2)
{
	int retval = 0;
	D3DXMATRIX diffmat;
	diffmat = srcmat1 - srcmat2;

	float dmin = 0.000001f;

	if ((fabs(diffmat._11) <= dmin) && (fabs(diffmat._12) <= dmin) && (fabs(diffmat._13) <= dmin) && (fabs(diffmat._14) <= dmin) &&
		(fabs(diffmat._21) <= dmin) && (fabs(diffmat._22) <= dmin) && (fabs(diffmat._23) <= dmin) && (fabs(diffmat._24) <= dmin) &&
		(fabs(diffmat._31) <= dmin) && (fabs(diffmat._32) <= dmin) && (fabs(diffmat._33) <= dmin) && (fabs(diffmat._34) <= dmin) && 
		(fabs(diffmat._41) <= dmin) && (fabs(diffmat._42) <= dmin) && (fabs(diffmat._43) <= dmin) && (fabs(diffmat._44) <= dmin)
		){
		retval = 1;
	}

	return retval;
}

int IsSameEul(D3DXVECTOR3 srceul1, D3DXVECTOR3 srceul2)
{
	int retval = 0;

	D3DXVECTOR3 diffeul;
	diffeul = srceul1 - srceul2;

	//float dmin = 0.000015f;
	float dmin = 0.00010f;//(degree)

	if ((fabs(diffeul.x) <= dmin) && (fabs(diffeul.y) <= dmin) && (fabs(diffeul.z) <= dmin)){
		retval = 1;
	}

	return retval;
}

