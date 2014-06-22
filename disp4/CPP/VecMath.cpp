#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <windows.h>

#define	DBGH
#include <dbg.h>

#include <crtdbg.h>

#include <VecMath.h>



float DXVec3Dot( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 )
{
	float retval;
	retval = vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
	return retval;
}
int DXVec3Cross( D3DXVECTOR3* outvec, D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 )
{
	D3DXVECTOR3 v1, v2;
	v1 = *vec1;
	v2 = *vec2;

	outvec->x = v1.y * v2.z - v1.z * v2.y;
	outvec->y = v1.z * v2.x - v1.x * v2.z;
	outvec->z = v1.x * v2.y - v1.y * v2.x;
	return 0;
}

float DXVec3Length( D3DXVECTOR3* v )
{
	float leng;
	float mag;
	mag = v->x * v->x + v->y * v->y + v->z * v->z;
	if( mag != 0.0f ){
		leng = sqrtf( mag );
	}else{
		leng = 0.0f;
	}
	return leng;
}

int DXVec3Normalize( D3DXVECTOR3* outv, D3DXVECTOR3* inv )
{
	D3DXVECTOR3 tmpv = *inv;
	float leng = DXVec3Length( &tmpv );
	if( leng != 0.0f ){
		*outv = tmpv / leng;
	}else{
		*outv = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	}
	return 0;
}
