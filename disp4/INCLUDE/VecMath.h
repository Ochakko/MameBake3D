#ifndef VECMATHH
#define VECMATHH

#include <d3dx9.h>

float DXVec3Dot( D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 );
int DXVec3Cross( D3DXVECTOR3* outvec, D3DXVECTOR3* vec1, D3DXVECTOR3* vec2 );
float DXVec3Length( D3DXVECTOR3* v );
int DXVec3Normalize( D3DXVECTOR3* outv, D3DXVECTOR3* inv );

#endif