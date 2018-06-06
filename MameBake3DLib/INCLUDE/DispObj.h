#ifndef DISPOBJH
#define DISPOBJH

//// Direct3D10 includes
//#include <d3dcommon.h>
//#include <dxgi.h>
//#include <d3d10_1.h>
//#include <d3d10.h>
//#include <d3dcompiler.h>
//#include <d3dx10.h>

#include <ChaVecCalc.h>

class CMQOMaterial;
class CPolyMesh3;
class CPolyMesh4;
class CExtLine;

class CDispObj
{
public:

/**
 * @fn
 * CDispObj
 * @breaf �R���X�g���N�^�B
 * @return �Ȃ��B
 */
	CDispObj();

/**
 * @fn
 * ~CDispObj
 * @breaf �f�X�g���N�^�B
 * @return �Ȃ��B
 */
	~CDispObj();

/**
 * @fn
 * CreateDispObj
 * @breaf ���^�Z�R�C�A����̃f�[�^�p�̕\���p�I�u�W�F�N�g���쐬����B
 * @param (ID3D10Device* pdev) IN Direct3D�̃f�o�C�X�B
 * @param (CPolyMesh3* pm3) IN �`��f�[�^�ւ̃|�C���^�B
 * @param (int hasbone) IN �{�[���ό`�L�̏ꍇ�͂P�A�����̏ꍇ�͂O�BMameBake3D�ł̓��^�Z�R�C�A�f�[�^�̓{�[�������B
 * @return ����������O�B
 */
	int CreateDispObj( ID3D10Device* pdev, CPolyMesh3* pm3, int hasbone );


/**
 * @fn
 * CreateDispObj
 * @breaf FBX����̃f�[�^�p�̕\���p�I�u�W�F�N�g���쐬����B
 * @param (ID3D10Device* pdev) IN Direct3D�̃f�o�C�X�B
 * @param (CPolyMesh4* pm4) IN �`��f�[�^�ւ̃|�C���^�B
 * @param (int hasbone) IN �{�[���ό`�L�̏ꍇ�͂P�A�����̏ꍇ�͂O�B * @return �߂�l�̐���
 * @return ����������O�B
 */
	int CreateDispObj( ID3D10Device* pdev, CPolyMesh4* pm4, int hasbone );

/**
 * @fn
 * CreateDispObj
 * @breaf �����f�[�^�p�̕\���p�I�u�W�F�N�g���쐬����B
 * @param (ID3D10Device* pdev) IN Direct3D�̃f�o�C�X�B
 * @param (CExtLine* extline) IN ���̌`��f�[�^�ւ̃|�C���^�B
 * @return ����������O�B
 */
	int CreateDispObj( ID3D10Device* pdev, CExtLine* extline );


/**
 * @fn
 * RenderNormal
 * @breaf �ʏ�̃x�[�V�b�N�ȕ��@�łRD�`�������B
 * @param (CMQOMaterial* rmaterial) IN �ގ��ւ̃|�C���^�B
 * @param (int lightflag) IN �ƌ���������Ƃ��͂P�A���Ȃ��Ƃ��͂O�B
 * @param (ChaVector4 diffusemult) IN �f�B�t���[�Y�F�Ɋ|���Z����䗦�B
 * @return ����������O�B
 */
	int RenderNormal( CMQOMaterial* rmaterial, int lightflag, ChaVector4 diffusemult );

/**
 * @fn
 * RenderNormalPM3
 * @breaf �ʏ�̃x�[�V�b�N�ȕ��@�Ń��^�Z�R�C�A�f�[�^���RD�\������B
 * @param (int lightflag) IN �ƌ���������Ƃ��͂P�A���Ȃ��Ƃ��͂O�B
 * @param (ChaVector4 diffusemult) IN �f�B�t���[�Y�F�Ɋ|���Z����䗦�B
 * @return ����������O�B
 * @detail FBX�f�[�^�͂P�I�u�W�F�N�g�ɂ��P�}�e���A��(�ގ�)�����A���^�Z�R�C�A�f�[�^�͂P�I�u�W�F�N�g�ɕ����}�e���A�����ݒ肳��Ă��邱�Ƃ������B
 */
	int RenderNormalPM3( int lightflag, ChaVector4 diffusemult );

/**
 * @fn
 * RenderLine
 * @breaf �����f�[�^��\������B
 * @param (ChaVector4 diffusemult) IN �f�B�t���[�Y�F�Ɋ|���Z����䗦�B
 * @return ����������O�B
 */
	int RenderLine( ChaVector4 diffusemult );

/**
 * @fn
 * CopyDispV
 * @breaf FBX�̒��_�f�[�^��\���o�b�t�@�ɃR�s�[����B
 * @param (CPolyMesh4* pm4) IN FBX�̌`��f�[�^�B
 * @return ����������O�B
 */
	int CopyDispV( CPolyMesh4* pm4 );

/**
 * @fn
 * CopyDispV
 * @breaf ���^�Z�R�C�A�̒��_�f�[�^��\���o�b�t�@�ɃR�s�[����B
 * @param (CPolyMesh3* pm3) IN ���^�Z�R�C�A�̌`��f�[�^�B
 * @param (������) �����̐���
 * @return ����������O�B
 */
	int CopyDispV( CPolyMesh3* pm3 );


	void SetScale(ChaVector3 srcscale, ChaVector3 srcoffset)
	{
		m_scale = srcscale;
		m_scaleoffset = srcoffset;
	};

private:

/**
 * @fn
 * InitParams
 * @breaf �����o������������B
 * @return �O�B
 */
	int InitParams();

/**
 * @fn
 * DestroyObjs
 * @breaf �A���P�[�g�������������J������B
 * @return �O�B
 */
	int DestroyObjs();

/**
 * @fn
 * CreateDecl
 * @breaf �\���p���_�f�[�^�̃t�H�[�}�b�g(�錾)���쐬����B
 * @return ����������O�B
 */
	int CreateDecl();

/**
 * @fn
 * CreateVBandIB
 * @breaf �RD�\���p�̒��_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@���쐬����B
 * @return ����������O�B
 */
	int CreateVBandIB();

/**
 * @fn
 * CreateVBandIBLine
 * @breaf �����p�̒��_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@���쐬����B
 * @return ����������O�B
 */
	int CreateVBandIBLine();

private:
	int m_hasbone;//�{�[���ό`�p�̃I�u�W�F�N�g�ł���Ƃ��P�A����ȊO�̎��͂O�B

	ID3D10Device* m_pdev;//�O���������ADirect3D�̃f�o�C�X�B
	CPolyMesh3* m_pm3;//�O���������A���^�Z�R�C�A�t�@�C������쐬�����RD�f�[�^�B
	CPolyMesh4* m_pm4;//�O���������AFBX�t�@�C������쐬�����RD�f�[�^�B
	CExtLine* m_extline;//�O���������A���f�[�^�B


	/*
	extern ID3D10EffectTechnique* g_hRenderBoneL0;
	extern ID3D10EffectTechnique* g_hRenderBoneL1;
	extern ID3D10EffectTechnique* g_hRenderBoneL2;
	extern ID3D10EffectTechnique* g_hRenderBoneL3;
	extern ID3D10EffectTechnique* g_hRenderNoBoneL0;
	extern ID3D10EffectTechnique* g_hRenderNoBoneL1;
	extern ID3D10EffectTechnique* g_hRenderNoBoneL2;
	extern ID3D10EffectTechnique* g_hRenderNoBoneL3;
	extern ID3D10EffectTechnique* g_hRenderLine;
	*/


	D3D10_BUFFER_DESC m_BufferDescBone;
	D3D10_BUFFER_DESC m_BufferDescNoBone;
	D3D10_BUFFER_DESC m_BufferDescInf;
	D3D10_BUFFER_DESC m_BufferDescLine;

	ID3D10InputLayout* m_layoutBoneL0;
	ID3D10InputLayout* m_layoutBoneL1;
	ID3D10InputLayout* m_layoutBoneL2;
	ID3D10InputLayout* m_layoutBoneL3;
	ID3D10InputLayout* m_layoutNoBoneL0;
	ID3D10InputLayout* m_layoutNoBoneL1;
	ID3D10InputLayout* m_layoutNoBoneL2;
	ID3D10InputLayout* m_layoutNoBoneL3;
	ID3D10InputLayout* m_layoutLine;

    ID3D10Buffer* m_VB;//�\���p���_�o�b�t�@�B
	ID3D10Buffer* m_InfB;//�\���p�{�[���e���x�o�b�t�@�B
	ID3D10Buffer* m_IB;//�\���p�O�p�̃C���f�b�N�X�o�b�t�@�B


	ChaVector3 m_scale;
	ChaVector3 m_scaleoffset;

};



#endif