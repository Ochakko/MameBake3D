#ifndef COLLISIONH
#define COLLISIONH

#include <d3dx9.h>

#ifdef COLLISIONCPP

/**
 * @fn
 * ChkRay
 * @breaf 半直線が三角形と交わるかどうかを判定する。
 * @param (int allowrev) IN 裏面も判定する場合１。
 * @param (int i1) IN 三角の１つ目の頂点のインデックス。pointbufへのインデックス。
 * @param (int i2) IN 三角の２つ目の頂点のインデックス。pointbufへのインデックス。
 * @param (int i3) IN 三角の３つ目の頂点のインデックス。pointbufへのインデックス。
 * @param (D3DXVECTOR3* pointbuf) IN 頂点の配列。三角の座標が含まれている。
 * @param (D3DXVECTOR3 startpos) IN 半直線の始点。
 * @param (D3DXVECTOR3 dir) IN 半直線の向き。
 * @param (float justval) IN 半直線の始点と面との最少距離。この距離より近い場合は面上の点とみなす。
 * @param (int* justptr) IN 半直線の始点と面の距離がjustvalより小さい場合は正の整数がセットされる。
 * @return 交わる場合は１、そうでなければ０を返す。
 */
	int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );

/**
 * @fn
 * CalcShadowToPlane
 * @breaf ある点から面への法線の足の座標を求める。
 * @param (D3DXVECTOR3 srcpos) IN 元の点の座標。
 * @param (D3DXVECTOR3 planedir) IN 面の法線。
 * @param (D3DXVECTOR3 planepos) IN 面上の１点。
 * @param (D3DXVECTOR3* shadowptr) OUT 法線の足の座標。
 * @return 成功したら０。
 */
	int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );

#else
	extern int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );
	extern int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );
#endif

#endif
