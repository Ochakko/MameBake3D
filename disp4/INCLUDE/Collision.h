#ifndef COLLISIONH
#define COLLISIONH

#include <d3dx9.h>

#ifdef COLLISIONCPP
	int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );
	int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );
#else
	extern int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );
	extern int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );
#endif

#endif
