#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#define COLLISIONCPP
#include <Collision.h>

#define DBGH
#include <dbg.h>

#include <VecMath.h>

int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr )
{
	D3DXVECTOR3 v1;
	v1 = startpos;

	D3DXVECTOR3 v;
	v = dir;

	D3DXVECTOR3 e;
	DXVec3Normalize( &e, &v );

	D3DXVECTOR3 s, t;
	s = *( pointbuf + i2 ) - *( pointbuf + i1 );
	t = *( pointbuf + i3 ) - *( pointbuf + i1 );
	D3DXVECTOR3 abc;
	DXVec3Cross( &abc, &s, &t );
	DXVec3Normalize( &abc, &abc );

	float d;
	d = -DXVec3Dot( &abc, (pointbuf + i1) );

	float dotface = DXVec3Dot( &abc, &e );
	if( dotface == 0.0f ){
//		_ASSERT( 0 );
		return 0;
	}
	if( (allowrev == 0) && (dotface < 0.0f) ){
		//— –Ê‚Í“–‚½‚ç‚È‚¢
		return 0;
	}

	float k;
	k = -( (DXVec3Dot( &abc, &v1 ) + d) / DXVec3Dot( &abc, &e ) );
	if( fabs( k ) <= justval ){
		(*justptr)++;
		return 0;
	}
	if( k < 0.0f ){
		return 0;
	}

	D3DXVECTOR3 q;
	q = v1 + k * e;

	D3DXVECTOR3 g0, g1, cA, cB, cC;
	
	g1 = *(pointbuf + i2) - *(pointbuf + i1);
	g0 = q - *(pointbuf + i1);
	DXVec3Cross( &cA, &g0, &g1 );
	DXVec3Normalize( &cA, &cA );

	g1 = *(pointbuf + i3) - *(pointbuf + i2);
	g0 = q - *(pointbuf + i2);
	DXVec3Cross( &cB, &g0, &g1 );
	DXVec3Normalize( &cB, &cB );

	g1 = *(pointbuf + i1) - *(pointbuf + i3);
	g0 = q - *(pointbuf + i3);
	DXVec3Cross( &cC, &g0, &g1 );
	DXVec3Normalize( &cC, &cC );


	float dota, dotb, dotc;
	dota = DXVec3Dot( &abc, &cA );
	dotb = DXVec3Dot( &abc, &cB );
	dotc = DXVec3Dot( &abc, &cC );

	int zeroflag;
	zeroflag = (fabs(dota) < 0.05f) && (fabs(dotb) < 0.05f) && (fabs(dotc) < 0.05f);
	if( zeroflag != 0 ){
		//(*justptr)++;
		return 1;
	}

	if( ((dota <= -0.50f) && (dotb <= -0.50f) && (dotc <= -0.50f)) ||
		((dota >= 0.50f) && (dotb >= 0.50f) && (dotc >= 0.50f)) ){
		return 1;
	}else{
		return 0;
	}

}
int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr )
{
	//–Ê‚ÆƒŒƒC‚Æ‚ÌŒð“_(shadow)‚ð‹‚ß‚éB
	//s-->start, e-->end, b-->point on plane, n-->plane dir

	if (!shadowptr){
		_ASSERT(0);
		return 1;
	}

	D3DXVECTOR3 sb, se, n;
	sb = planepos - srcpos;
	se = (srcpos + 100.0f * planedir) - srcpos;
	n = planedir;

	float t;
	t = D3DXVec3Dot( &sb, &n ) / D3DXVec3Dot( &se, &n );

	*shadowptr = ( 1.0f - t ) * srcpos + t * (srcpos + 100.0f * planedir);
	return 0;
}