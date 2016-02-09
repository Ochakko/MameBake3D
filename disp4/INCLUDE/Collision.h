#ifndef COLLISIONH
#define COLLISIONH

#include <d3dx9.h>

#ifdef COLLISIONCPP

/**
 * @fn
 * ChkRay
 * @breaf ���������O�p�`�ƌ���邩�ǂ����𔻒肷��B
 * @param (int allowrev) IN ���ʂ����肷��ꍇ�P�B
 * @param (int i1) IN �O�p�̂P�ڂ̒��_�̃C���f�b�N�X�Bpointbuf�ւ̃C���f�b�N�X�B
 * @param (int i2) IN �O�p�̂Q�ڂ̒��_�̃C���f�b�N�X�Bpointbuf�ւ̃C���f�b�N�X�B
 * @param (int i3) IN �O�p�̂R�ڂ̒��_�̃C���f�b�N�X�Bpointbuf�ւ̃C���f�b�N�X�B
 * @param (D3DXVECTOR3* pointbuf) IN ���_�̔z��B�O�p�̍��W���܂܂�Ă���B
 * @param (D3DXVECTOR3 startpos) IN �������̎n�_�B
 * @param (D3DXVECTOR3 dir) IN �������̌����B
 * @param (float justval) IN �������̎n�_�ƖʂƂ̍ŏ������B���̋������߂��ꍇ�͖ʏ�̓_�Ƃ݂Ȃ��B
 * @param (int* justptr) IN �������̎n�_�Ɩʂ̋�����justval��菬�����ꍇ�͐��̐������Z�b�g�����B
 * @return �����ꍇ�͂P�A�����łȂ���΂O��Ԃ��B
 */
	int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );

/**
 * @fn
 * CalcShadowToPlane
 * @breaf ����_����ʂւ̖@���̑��̍��W�����߂�B
 * @param (D3DXVECTOR3 srcpos) IN ���̓_�̍��W�B
 * @param (D3DXVECTOR3 planedir) IN �ʂ̖@���B
 * @param (D3DXVECTOR3 planepos) IN �ʏ�̂P�_�B
 * @param (D3DXVECTOR3* shadowptr) OUT �@���̑��̍��W�B
 * @return ����������O�B
 */
	int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );

#else
	extern int ChkRay( int allowrev, int i1, int i2, int i3, D3DXVECTOR3* pointbuf, D3DXVECTOR3 startpos, D3DXVECTOR3 dir, float justval, int* justptr );
	extern int CalcShadowToPlane( D3DXVECTOR3 srcpos, D3DXVECTOR3 planedir, D3DXVECTOR3 planepos, D3DXVECTOR3* shadowptr );
#endif

#endif
