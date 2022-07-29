#ifndef MODELH
#define MODELH


//class ID3D11Device;

//#include <d3dx9.h>
#include <wchar.h>
#include <string>
#include <map>

#include <Coef.h>
#include <OrgWindow.h>

#include <ChaVecCalc.h>
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

#include <fbxsdk.h>


#include <UndoMotion.h>

using namespace std;

class CMQOMaterial;
class CMQOObject;
class CMQOFace;
class CBone;
class CMySprite;
class CMotionPoint;
class CQuaternion;
class CBtObject;
class CRigidElem;
class CEditRange;

typedef struct funcmpparams
{
	int slotno;
	FbxMesh* fbxmesh;
	CModel* pmodel;
	FbxNode* jointnode;
	FbxNode* linknode;
	int framestart;
	int frameend;
	CBone* curbone;
	int animno;
	int motid;
	double animleng;
	FbxCluster* cluster;
	FbxPose* pPose;
	FbxAMatrix globalcurrentpos;
}FUNCMPPARAMS;


typedef struct tag_newmpelem
{
	CBone* boneptr;
	CMotionPoint* mpptr;
}NEWMPELEM;

typedef struct tag_fbxobj
{
	FbxNode* node;
	FbxMesh* mesh;
}FBXOBJ;


typedef struct tag_physikrec
{
	double time;
	CBone* pbone;
	ChaMatrix btmat;
}PHYSIKREC;

#define MAXPHYSIKRECCNT		(60 * 60)

class CModel
{
public:

/**
 * @fn
 * CModel
 * @breaf �R���X�g���N�^
 * @return �Ȃ��B
 */
	CModel();

/**
 * @fn
 * ~CModel
 * @breaf �f�X�g���N�^�B
 * @return �f�X�g���N�^�B
 */
	~CModel();

/**
 * @fn
 * LoadMQO
 * @breaf ���^�Z�R�C�A�ō쐬�����RD�f�[�^�t�@�C��*.mqo��ǂݍ��ށB
 * @param (ID3D11Device* pdev) IN Direct3D�̃f�o�C�X�B
 * @param (WCHAR* wfile) IN mqo�t�@�C���̃t���p�X�B
 * @param (WCHAR* modelfolder) IN FBX�����o���̍ۂɎg�p���郂�f���t�H���_�[���B�t�@�C��������g���q����������̂ɒʂ��ԍ���t�������̂��f�t�H���g�B
 * @param (float srcmult) IN �ǂݍ��ݔ{���B
 * @param (int ismedia) IN SDKmisc.cpp��DXUTFindDXSDKMediaFileCch�ŒT���f�B���N�g�����̃t�@�C�����ǂ����Ƃ����t���O�B
 * @param (int texpool = 0) IN �e�N�X�`�����쐬����ꏊ�B
 * @return ����������O�B
 * @detail texpool�����ɂ̓f�t�H���g�l������̂ŏȗ��\�B
 */
	int LoadMQO( ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, WCHAR* wfile, WCHAR* modelfolder, float srcmult, int ismedia, int texpool = 0 );
	
/**
 * @fn
 * LoadFBX
 * @breaf FBX�t�@�C����ǂݍ��ށB
 * @param (int skipdefref) IN �f�t�H���g���̐ݒ�𗘗p���X�L�b�v���邩�ǂ����̃t���O�Bcha�t�@�C������Ăяo���Ƃ���ref�t�@�C��������̂łP�BFBX�P�̂œǂݍ��ނƂ��͂O�B
 * @param (ID3D11Device* pdev) IN Direct3D��Device�B
 * @param (WCHAR* wfile) IN FBX�t�@�C���̃t���p�X�B
 * @param (WCHAR* modelfolder) IN FBX�����o�����Ɏg�p����FBX�t�@�C��������t�H���_�̖��O�Bcha�t�@�C��������t�H���_�̒���FBX������t�H���_�̖��O�ƂȂ�B
 * @param (float srcmult) IN �ǂݍ��ݔ{���B
 * @param (FbxManager* psdk) IN FBXSDK�̃}�l�[�W���B
 * @param (FbxImporter** ppimporter) IN FBXSDK�̃C���|�[�^�[�B
 * @param (FbxScene** ppscene) IN FBXSDK�̃V�[���B
 * @param (int forcenewaxisflag) �ߓn���t�@�C���̃t���O�B
 * @return ����������O�B
 */
	int LoadFBX( int skipdefref, ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, WCHAR* wfile, WCHAR* modelfolder, float srcmult, FbxManager* psdk, FbxImporter** ppimporter, FbxScene** ppscene, int forcenewaxisflag, BOOL motioncachebatchflag);

/**
 * @fn
 * LoadFBXAnim
 * @breaf FBX�̃A�j���[�V������ǂݍ��ށB
 * @param (FbxManager* psdk) IN FBXSDK�̃}�l�[�W���B
 * @param (FbxImporter* pimporter) IN FBXSDK�̃C���|�[�^�[�B
 * @param (FbxScene* pscene) IN FBXSDK�̃V�[���B
 * @param (int (*tlfunc)( int srcmotid )) IN �^�C�����C���������p�̊֐��ւ̃|�C���^�B
 * @return ����������O�B
 * @detail LoadFBX�Ăяo��������������ł��̊֐����Ăяo���B
 */
	int LoadFBXAnim( FbxManager* psdk, FbxImporter* pimporter, FbxScene* pscene, int (*tlfunc)( int srcmotid ), BOOL motioncachebatchflag);


/**
 * @fn
 * OnRender
 * @breaf ���f���f�[�^��`�悷��B
 * @param (ID3D11Device* pdev) IN Direct3D��Device�B
 * @param (int lightflag) IN �ƌ����������邩�ǂ����̃t���O�B
 * @param (ChaVector4 diffusemult) IN �f�B�t���[�Y(�g�U��)�ɏ�Z����RGBA�̒l�B���C�g��ς��Ȃ��Ă����邳��ς��邱�Ƃ��o����B
 * @param (int btflag = 0) IN bullet�̃V�~�����[�V�������ł��邩�ǂ����̃t���O�B
 * @return ����������O�B
 */
	int OnRender(ID3D11DeviceContext* pd3dImmediateContext, int lightflag, ChaVector4 diffusemult, int btflag = 0 );

	int RenderRefArrow(ID3D11DeviceContext* pd3dImmediateContext, CBone* boneptr, ChaVector4 diffusemult, int refmult, std::vector<ChaVector3> vecbonepos);
/**
 * @fn
 * RenderBoneMark
 * @breaf �{�[���}�[�N�ƃW���C���g�}�[�N�ƍ��̌`���\������B
 * @param (ID3D11Device* pdev) IN Direct3D��Device�B
 * @param (CModel* bmarkptr) IN �{�[���}�[�N�̃��f���f�[�^�B
 * @param (CMySprite* bcircleptr) IN �W���C���g�����̕\����Sprite�B
 * @param (int selboneno) IN �I�𒆂̃{�[����ID�B
 * @param (int skiptopbonemark) IN ��Ԑe����̃{�[����\�����Ȃ��t���O�B
 * @return ����������O�B
 */
	int RenderBoneMark(ID3D11DeviceContext* pd3dImmediateContext, CModel* bmarkptr, CMySprite* bcircleptr, int selboneno, int skiptopbonemark = 0 );


	void RenderCapsuleReq(ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto);

	void RenderBoneCircleReq(ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto, CMySprite* bcircleptr);

/**
 * @fn
 * GetModelBound
 * @breaf �o�E���_���[�f�[�^���擾����B
 * @param (MODELBOUND* dstb) OUT �o�E���_���[�f�[�^�i���f�����͂����ލŏ��͈̓f�[�^�j���Z�b�g�����B
 * @return ����������O�B
 */
	int GetModelBound( MODELBOUND* dstb );

/**
 * @fn
 * MakeObjectName
 * @breaf �I�u�W�F�N�g�̖��O����CMQOObject���������邽�߂̃f�[�^���Z�b�g����B
 * @return ����������O�B
 */
	int MakeObjectName();

/**
 * @fn
 * MakePolyMesh3
 * @breaf ���^�Z�R�C�A����ǂݍ��񂾃f�[�^�̕\���p�f�[�^���쐬����B
 * @return ����������O�B
 */
	int MakePolyMesh3();

/**
 * @fn
 * MakePolyMesh4
 * @breaf FBX����ǂݍ��񂾃f�[�^�̕\���p�f�[�^���쐬����B
 * @return ����������O�B
 */
	int MakePolyMesh4();

/**
 * @fn
 * MakeExtLine
 * @breaf ���^�Z�R�C�A��FBX����ǂݍ��񂾃f�[�^�̐����\���p�̃f�[�^���쐬����B
 * @return ����������O�B
 */
	int MakeExtLine();
	
/**
 * @fn
 * MakeDispObj
 * @breaf DirectX�̕`��p�o�b�t�@�̃��b�p�f�[�^���쐬����B
 * @return ����������O�B
 */	
	int MakeDispObj();

/**
 * @fn
 * UpdateMatrix
 * @breaf �A�j���[�V�����f�[�^��K�p����B���݂̎��Ԃ̎p�����Z�b�g����B���[�t�A�j��������΃��[�t�A�j�����K�p����B
 * @param (ChaMatrix* wmat) IN ���[���h�ϊ��s��B
 * @param (ChaMatrix* vpmat) IN View * Projection�ϊ��s��B
 * @return ����������O�B
 */
	int UpdateMatrix( ChaMatrix* wmat, ChaMatrix* vpmat );
	int HierarchyRouteUpdateMatrix(CBone* srcbone, ChaMatrix* wmat, ChaMatrix* vpmat);
	int UpdateLimitedWM(int srcmotid, double srcframe);
	int ClearLimitedWM(int srcmotid, double srcframe);

/**
 * @fn
 * SetShaderConst
 * @breaf �V�F�[�_�[�̒萔���Z�b�g����B�A�j���[�V�����A���C�g�A�e�N�X�`���Ȃǂ̃V�F�[�_�[�萔��Direct3D�̕`�施�߂��Ăяo���O�ɃZ�b�g���Ă����B
 * @param (CMQOObject* srcobj) IN �`��Ώۂ�CMQOObject�B
 * @param (int btflag = 0) IN bullet�̃V�~�����[�V���������ǂ����̃t���O�B
 * @return ����������O�B
 */
	int SetShaderConst( CMQOObject* srcobj, int btflag = 0 );

/**
 * @fn
 * FillTimeLine
 * @breaf �^�C�����C���Ƀ{�[���̕��A�s��ǉ�����B
 * @param (OrgWinGUI::OWP_Timeline& timeline) OUT �^�C�����C���B
 * @param (map<int, int>& lineno2boneno) OUT �^�C�����C���̍s�ԍ�����{�[��ID���������邽�߂�map�B
 * @param (map<int, int>& boneno2lineno) OUT �{�[��ID����^�C�����C���̍s�ԍ����������邽�߂�map�B
 * @return ����������O�B
 */
	int FillTimeLine( OrgWinGUI::OWP_Timeline& timeline, map<int, int>& lineno2boneno, map<int, int>& boneno2lineno );

/**
 * @fn
 * AddMotion
 * @breaf �A�j���[�V�����ێ��p�̃G���g���[���쐬����B
 * @param (char* srcname) IN �A�j���[�V�����̖��O�B
 * @param (WCHAR* wfilename) IN �A�j���[�V�����t�@�C�������o�����̂��߂̖��O�B
 * @param (double srcleng) IN �A�j���[�V�����̒����B
 * @param (int* dstid) OUT �A�j���[�V�����̑I���Ɏg�p����ID���Z�b�g�����B
 * @return ����������O�B
 */
	int AddMotion( char* srcname, WCHAR* wfilename, double srcleng, int* dstid );

/**
 * @fn
 * SetCurrentMotion
 * @breaf ���݂̃��[�V����ID���w�肷��B
 * @param (int srcmotid) IN ���[�V����ID�B
 * @return ����������O�B
 */
	int SetCurrentMotion( int srcmotid );

/**
 * @fn
 * SetMotionFrame
 * @breaf �Đ����郂�[�V�����̃t���[�����w�肷��B
 * @param (double srcframe) IN �t���[���i���ԁj���w�肷��B
 * @return ����������O�B
 */
	int SetMotionFrame( double srcframe );


/**
 * @fn
 * GetMotionFrame
 * @breaf ���݂̃��[�V�����̍Đ��t���[�����擾����B
 * @param (double* dstframe) OUT �Đ��t���[�����Z�b�g�����B
 * @return ����������O�B
 */
	int GetMotionFrame( double* dstframe );

/**
 * @fn
 * SetMotionSpeed
 * @breaf ���[�V�����̍Đ��X�s�[�h(60fps�ɑ΂��Ă̔{��)���w�肷��B
 * @param (double srcspeed) IN �Đ��X�s�[�h�B
 * @return ����������O�B
 */
	int SetMotionSpeed( double srcspeed );

/**
 * @fn
 * GetMotionSpeed
 * @breaf ���[�V�����̍Đ��X�s�[�h���擾����B
 * @param (double* dstspeed) OUT �Đ��X�s�[�h���Z�b�g�����B
 * @return ����������O�B
 */
	int GetMotionSpeed( double* dstspeed );

/**
 * @fn
 * DeleteMotion
 * @breaf ���[�V�������폜����B
 * @param (int motid) IN �폜���郂�[�V������ID�B
 * @return ����������O�B
 * @detail �p���̃L�[�����łȂ��A���[�V�����̃G���g���[���폜����B
 */
	int DeleteMotion( int motid );


/**
 * @fn
 * GetSymBoneNo
 * @breaf ���E�Ώ̐ݒ�̖��O�̃{�[����ID���擾����B
 * @param (int srcboneno) IN ���̃{�[����ID�B
 * @param (int* dstboneno) OUT ���E�Ώ̐ݒ�̃{�[����ID���Z�b�g�����B 
 * @param (int* existptr) OUT ���E�Ώ̐ݒ�̃{�[����������΂P�A������΂O���Z�b�g�����B
 * @return ����������O�B
 */
	int GetSymBoneNo( int srcboneno, int* dstboneno, int* existptr );


/**
 * @fn
 * PickBone
 * @breaf �w�肵����ʂ̂QD���W�ɋ߂��{�[�����擾����B
 * @param (PICKINFO* pickinfo) INOUT �擾���ւ̃|�C���^�B
 * @return ����������O�B
 * @detail pickinfo�ɃE�C���h�E�̃T�C�Y�ƃN���b�N�����QD���W�ƃ{�[���Ƃ̍ő勗�����w�肵�ČĂяo���B
 */
	int PickBone( PICKINFO* pickinfo );

/**
 * @fn
 * IKRotate
 * @breaf �I���{�[���̑I���t���[���ɑ΂���IK�Ŏp������]����B
 * @param (CEditRange* erptr) IN �I���t���[�������w�肷��B
 * @param (int srcboneno) IN �I���{�[����ID���w�肷��B
 * @param (ChaVector3 targetpos) IN �I���{�[���̖ڕW���W���w�肷��B
 * @param (int maxlevel) IN IK�v�Z�ŉ��K�w�e�܂ł����̂ڂ邩���w�肷��B
 * @return ����������O�B
 * @detail MameBake3D�ɂ����ẮA�}�j�s�����[�^�̒����̉��F���h���b�O�������ɌĂ΂��B
 */
	int IKRotate( CEditRange* erptr, int srcboneno, ChaVector3 targetpos, int maxlevel );


	int PhysicsRot(CEditRange* erptr, int srcboneno, ChaVector3 targetpos, int maxlevel);
	int PhysicsMV(CEditRange* erptr, int srcboneno, ChaVector3 diffvec);

	int CreatePhysicsPosConstraint(CBone* srcbone);
	int DestroyPhysicsPosConstraint(CBone* srcbone);
	int CreatePhysicsPosConstraintAll();
	int DestroyPhysicsPosConstraintAll();
	int CreatePhysicsPosConstraintUpper(CBone* srcbone);
	int DestroyPhysicsPosConstraintUpper(CBone* srcbone);
	int CreatePhysicsPosConstraintLower(CBone* srcbone);
	int DestroyPhysicsPosConstraintLower(CBone* srcbone);
	
	int Mass0_All(bool setflag);
	int Mass0_Upper(bool setflag, CBone* srcbone);
	int Mass0_Lower(bool setflag, CBone* srcbone);

	int SetMass0(CBone* srcbone);
	int RestoreMass(CBone* srcbone);

	int SetKinematicTmpLower(CBone* srcbone, bool srcflag);
	
/**
 * @fn
 * IKRotateAxisDelta
 * @breaf �I���{�[���̑I���t���[���ɑ΂��Ď��w���IK��]������B
 * @param (CEditRange* erptr) IN �I���t���[�������w�肷��B
 * @param (int axiskind) IN ��]�����w�肷��BPICK_X, PICK_Y, PICK_Z�̂����ꂩ�B
 * @param (int srcboneno) IN �I���{�[����ID���w�肷��B
 * @param (float delta) IN ��]�p�x���w�肷��B
 * @param (int maxlevel) IN IK�v�Z�ŉ��K�w�e�܂ł����̂ڂ邩���w�肷��B
 * @param (int ikcnt) IN �}�E�X�Ńh���b�O���J�n���Ă��牽�񂱂̊֐������s�������B
 * @return ����������O�B
 * @detail MameBake3D�ɂ����ẮA�}�j�s�����[�^�̃����O�܂��͋����h���b�O�������ɌĂ΂��B
 */
	int IKRotateAxisDelta( CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat );
	int TwistBoneAxisDelta(CEditRange* erptr, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat);


	int PhysicsRotAxisDelta(CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat);
	int SetDofRotAxis(int srcaxiskind);


/**
 * @fn
 * RotateXDelta
 * @breaf �{�[�����Ɋւ��Ďp�����˂���B
 * @param (CEditRange* erptr) IN �I���t���[�������w�肷��B
 * @param (int srcboneno) IN �I���{�[����ID���w�肷��B
 * @param (float delta) IN ��]�p�x���w�肷��B
 * @return ����������O�B
 * @detail MameBake3D�ɂ����ẮA�}�j�s�����[�^�̃����O�܂��͋��Ń{�[�����Ɋւ��ĉ�]����Ƃ��ɌĂ΂��B
 */
	int RotateXDelta( CEditRange* erptr, int srcboneno, float delta );

/**
 * @fn
 * FKRotate
 * @breaf �I���{�[���̑I���t���[���ɑ΂���FK�Ŏw�肵����������]����B
 * @param (double srcframe) IN �I���t���[�������w�肷��B
 * @param (int srcboneno) IN �I���{�[����ID���w�肷��B
 * @param (CQuaternion rotq) IN ��]��\���N�H�[�^�j�I���B
 * @return ����������O�B
 */
	int FKRotate(int reqflag, CBone* bvhbone, int traflag, ChaVector3 traanim, double srcframe, int srcboneno, CQuaternion srcq, int setmatflag = 0, ChaMatrix* psetmat = 0);
	//int FKRotate(double srcframe, int srcboneno, ChaMatrix srcmat);

/**
 * @fn
 * FKBoneTra
 * @breaf �I���{�[���̑I���t���[���ɑ΂���FK�Ŏw�肵���������ړ�����B
 * @param (CEditRange* erptr) IN �I���t���[�������w�肷��B
 * @param (int srcboneno) IN �I���{�[����ID���w�肷��B
 * @param (ChaVector3 addtra) IN �ړ����̃x�N�g���B
 * @return ����������O�B
 */
	int FKBoneTra( int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 addtra );

	int FKBoneTraAxis(int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float delta);


	int FKBoneScale(int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 scalevec);

	int FKBoneScaleAxis(int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float scaleval);


/**
 * @fn
 * CollisionNoBoneObj_Mouse
 * @breaf ��ʂ̂QD�̈ʒu�ɃI�u�W�F�N�g�����邩�ǂ������ׂ�B
 * @param (PICKINFO* pickinfo) INOUT ��ʂ̂QD���W�ƍő勗�����w�肷��B
 * @param (char* objnameptr) IN ���ׂ����I�u�W�F�N�g�̖��O���w�肷��B
 * @return �QD�ʒu�ɃI�u�W�F�N�g������΂P�A������΂O��Ԃ��B
 */
	int CollisionNoBoneObj_Mouse( PICKINFO* pickinfo, char* objnameptr );

/**
 * @fn
 * TransformBone
 * @breaf �{�[���̍��W��ϊ��v�Z����B
 * @param (int winx) IN �RD�\���E�C���h�E�̕�
 * @param (int winy) IN �RD�\���E�C���h�E�̍���
 * @param (int srcboneno) IN �{�[����ID�B
 * @param (ChaVector3* worldptr) OUT ���[���h���W�n�̕ϊ����ʁB
 * @param (ChaVector3* screenptr) OUT *worldptr�ɍX��View Projection��K�p�������W�B
 * @param (ChaVector3* dispptr) OUT *screenptr��-WindowSize/2����+WindowSize/2�܂ł̂QD���W�ɂ������́B
 * @return ����������O�B
 * @detail CBone��UpdateMatrix���Ă΂ꂽ��ŌĂяo����邱�Ƃ�z�肵�Ă���B(CBone::m_childworld���g�p���Ă���B)
 */
	int TransformBone( int winx, int winy, int srcboneno, ChaVector3* worldptr, ChaVector3* screenptr, ChaVector3* dispptr );

/**
 * @fn
 * ChangeMotFrameLeng
 * @breaf ���[�V�����̃t���[������ύX����B
 * @param (int motid) IN ���[�V������ID�B
 * @param (double srcleng) IN �ύX��̃t���[�����B
 * @return ����������O�B
 * @detail �͈͊O�̃L�[�͍폜�����B
 */
	int ChangeMotFrameLeng( int motid, double srcleng );

/**
 * @fn
 * AdvanceTime
 * @breaf �o�ߎ��Ԃ����ɁA���[�V�����̎��ɍĐ�����t���[�����v�Z����B
 * @param (int previewflag) IN �v���r���[��Ԃ��w��B
 * @param (double difftime) IN �O��`�悵�Ă���̎��Ԃ��w��B
 * @param (double* nextframeptr) OUT ���ɕ`�悷��t���[���B
 * @param (int* endflagptr) OUT �J��Ԃ��Đ��ł͂Ȃ����[�V�����̍ŏI�t���[���ɒB�������ǂ����B
 * @param (int* loopstartflag) �J��Ԃ��ɂ��ŏ��̃t���[���ɖ߂������ǂ����B
 * @param (int srcmotid) IN ���[�V������ID�B
 * @return ����������O�B
 * @detail previewflag�͒�~���O�A�ʏ�Đ��P�Abullet�����V�~�����[�V�����S�Abullet���O�h�[���V�~�����[�V�����T�B
 */
	int AdvanceTime(int onefps, CEditRange srcrange, int previewflag, double difftime, double* nextframeptr, int* endflagptr, int* loopstartflag, int srcmotid);

/**
 * @fn
 * MakeEnglishName
 * @breaf �t�@�C�������o��(���FBX�t�@�C��)�̂��߂Ɋe�햼�O���p��\�L�ɂ���B
 * @return ����������O�B
 */
	int MakeEnglishName();

/**
 * @fn
 * AddDefMaterial
 * @breaf �f�t�H���g�̍ގ����쐬����B
 * @return ����������O�B
 * @detail ���^�Z�R�C�A�t�@�C���ɂ͍ގ����܂܂�Ă��Ȃ��ꍇ������B�ގ����P���Ȃ��ƕ`�掞�ɃG���[�ɂȂ�̂Ńf�t�H���g�̍ގ����쐬����B
 */
	int AddDefMaterial();

/**
 * @fn
 * CreateBtObject
 * @breaf bullet�V�~�����[�V�����p�̃I�u�W�F�N�g���쐬����B
 * @param (int onfirstcreate) IN ��ԍŏ��̌Ăяo�����ɂ����P���w�肷��B
 * @return ����������O�B
 * @detail �V�~�����[�V�����J�n�̂��тɌĂԁB��ԍŏ��̌Ăяo������onfirstcreate���P�ɂ���B
 */
	int CreateBtObject( int onfirstcreate );

/**
 * @fn
 * SetBtMotion
 * @breaf bullet�V�~�����[�V�������ʂ�K�p����B
 * @param (int rgdollflag) IN ���O�h�[�����ɂP�B
 * @param (double srcframe) IN ���[�V�����̃t���[���B
 * @param (ChaMatrix* wmat) IN ���[���h�ϊ��s��B
 * @param (ChaMatrix* vpmat) IN ViewProj�ϊ��s��B
 * @param (double difftime) IN �O��̕`�悩��̎��ԁB
 * @return ����������O�B
 * @detail bullet�V�~�����[�V�������ɂ́ACModel::Motion2Bt-->BPWorld::clientMoveAndDisplay-->CModel::SetBtMotion�Ƃ�������ŌĂяo���B
 */
	int SetBtMotion(CBone* srcbone, int rgdollflag, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat );

/**
 * @fn
 * SetBtImpulse
 * @breaf �ݒ肳��Ă����C���p���X��K�p����B
 * @return ����������O�B
 */
	int SetBtImpulse();

/**
 * @fn
 * SetImp
 * @breaf �{�[���ɗ^����C���p���X��ݒ肷��B
 * @param (int srcboneno) IN �{�[����ID�B
 * @param (int kind) IN X�����w�莞�O�AY�w�莞�P�AZ�w�莞�Q�B
 * @param (float srcval) IN �C���p���X�̑傫���B
 * @return ����������O�B
 */
	int SetImp( int srcboneno, int kind, float srcval );

/**
 * @fn
 * SetDispFlag
 * @breaf �\�����邩�ǂ����̃t���O��ݒ肷��B
 * @param (char* objname) IN �I�u�W�F�N�g�̖��O���w�肷��B
 * @param (int flag) IN �\������Ƃ��P�A���Ȃ��Ƃ��O�B
 * @return ����������O�B
 * @detail �\������Ƃ��P�A���Ȃ��Ƃ��O�B
 */
	int SetDispFlag( char* objname, int flag );

/**
 * @fn
 * FindBtObject
 * @breaf �q���W���C���g�̖��O�ɑΉ�����CBtObject����������B
 * @param (int srcboneno) IN �q���W���C���g�̖��O�B
 * @return CBtObject* �������ʁB
 */
	CBtObject* FindBtObject( int srcboneno );


	int FillUpEmptyMotion(int motid);


	int SetAllSphrateData(int gid, int rgdindex, float srcval);
	int SetAllBoxzrateData(int gid, int rgdindex, float srcval);
	int SetAllSkipflagData(int gid, int rgdindex, bool srcval);
	int SetAllForbidrotData(int gid, int rgdindex, bool srcval);
	int SetAllColtypeData(int gid, int rgdindex, int srcval);
	int SetAllBtforceData(int rgdindex, bool srcval);
	int SetAllDampAnimData( int gid, int rgdindex, float valL, float valA );
	int SetAllBtgData( int gid, int reindex, float btg );
	int SetAllImpulseData( int gid, float impx, float impy, float impz );
	int SetAllKData( int gid, int reindex, int srclk, int srcak, float srccuslk, float srccusak );
	int SetAllRestData( int gid, int reindex, float rest, float fric );
	int SetAllMassData( int gid, int reindex, float srcmass );
	int SetAllMassDataByBoneLeng(int gid, int reindex, float srcmass);

	int SetAllDmpData( int gid, int reindex, float ldmp, float admp );
	int EnableAllRigidElem(int srcrgdindex);
	int DisableAllRigidElem(int srcrgdindex);

	int SetColTypeAll(int reindex, int srctype);

	int Motion2Bt( int firstflag, double nextframe, ChaMatrix* mW, ChaMatrix* mVP, int selectboneno );
	int SetRagdollKinFlag(int selectbone, int physicsmvkind = 0);
	int SetKinematicFlag();
	int CreateRigidElem();
	int SetCurrentRigidElem( int curindex );
	REINFO GetCurrentRigidElemInfo(int* retindex);
	void CreateRigidElemReq( CBone* curbone, int reflag, string rename, int impflag, string impname );
	int SetBtEquilibriumPoint();
	int SetBtEquilibriumPointReq( CBtObject* srcbto );
	void SetDofRotAxisReq(CBtObject* srcbto, int srcaxiskind);


	int MultDispObj( ChaVector3 srcmult, ChaVector3 srctra );
	MOTINFO* GetRgdMorphInfo();

	int SetColiIDtoGroup( CRigidElem* curre );
	int ResetBt();

	int InitUndoMotion( int saveflag );
	int SaveUndoMotion( int curboneno, int curbaseno, CEditRange* srcer, double srcapplyrate );
	int RollBackUndoMotion( int redoflag, int* curboneno, int* curbaseno, double* dststartframe, double* dstendframe, double* dstapplyframe );

	int AddBoneMotMark( OrgWinGUI::OWP_Timeline* owpTimeline, int curboneno, int curlineno, double startframe, double endframe, int flag );

	int MotionID2Index( int motid );
	FbxAnimLayer* GetAnimLayer( int motid );
	float GetTargetWeight( int motid, double dframe, double timescale, CMQOObject* baseobj, std::string targetname );


	int SetFirstFrameBonePos(HINFO* phinfo);

	int RecalcBoneAxisX(CBone* srcbone);

	int CalcBoneEul(int srcmotid);
	void CalcBoneEulReq(CBone* curbone, int srcmotid, double srcframe);

	int RigControl(int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig);
	int PhysicsRigControl(int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig);

	int DbgDump();

	CBone* GetSymPosBone(CBone* srcbone);

	int InterpolateBetweenSelection(double srcstartframe, double srcendframe);

	int BulletSimulationStop();
	int BulletSimulationStart();
	int ApplyBtToMotion();
	void CalcBtAxismat(int onfirstcreate);
	void CalcRigidElem();
	void SetBtKinFlagReq(CBtObject* srcbto, int oncreateflag);
	void SetKinematicFlagReq(CBtObject* srcbto);

	void ResetMotionCache();

	void EnableRotChildren(CBone* srcbone, bool srcflag);
	void EnableRotChildrenReq(CBone* srcbone, bool srcflag);

	void DestroyScene();


	void PhysIKRec(double srcrectime);
	void PhysIKRecReq(CBone* srcbone, double srcrectime);
	void ApplyPhysIkRec();
	void ApplyPhysIkRecReq(CBone* srcbone, double srcframe, double srcrectime);

	int ResetAngleLimit(int srcval);
	int AngleLimitReplace180to170();
	int AdditiveCurrentToAngleLimit();

	bool ChkBoneHasRig(CBone* srcbone);

private:
	int InitParams();
	int DestroyObjs();
	int CreateMaterialTexture(ID3D11DeviceContext* pd3dImmediateContext);


	int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb );

	int DestroyMaterial();
	int DestroyObject();
	int DestroyAncObj();
	int DestroyAllMotionInfo();

	//void MakeBoneReq( CBone* parentbone, CMQOFace* curface, ChaVector3* pointptr, int broflag, int* errcntptr );

	int SetShapeNoReq( CMQOFace** ppface, int facenum, int searchp, int shapeno, int* setfacenum);
	int SetFaceOfShape( CMQOFace** ppface, int facenum, int shapeno, CMQOFace** ppface2, int setfacenum );

	int CreateBtConstraint();
	void CreateBtConstraintReq( CBtObject* curbto );
	//void CreateBtConstraintReq(CBone* curbone);

	void CreateIndexedMotionPointReq(CBone* srcbone, int srcmotid, double srcanimleng);

	//void UpdateMatrixReq( int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat, 
	//	ChaMatrix* parmat, CQuaternion* parq, CBone* srcbone, int broflag );

	void FillTimelineReq( OrgWinGUI::OWP_Timeline& timeline, CBone* curbone, int* linenoptr, 
		map<int, int>& lineno2boneno, map<int, int>& boneno2lineno, int broflag );

	void SetSelectFlagReq( CBone* boneptr, int broflag );
	int CalcMouseLocalRay( PICKINFO* pickinfo, ChaVector3* startptr, ChaVector3* dirptr );
	CBone* GetCalcRootBone( CBone* firstbone, int maxlevel );
	//void CalcXTransformMatrixReq( CBone* srcbone, ChaMatrix parenttra, float mult );


	//int InitFBXManager( FbxManager** ppSdkManager, FbxImporter** ppImporter, FbxScene** ppScene, char* utfname );
	int CreateFBXMeshReq( FbxNode* pNode );
	int CreateFBXShape( FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep );

	CMQOObject* GetFBXMesh(FbxNode* pNode, FbxNodeAttribute *pAttrib );
	int GetFBXShape(FbxMesh* pMesh, CMQOObject* curobj, FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep );
	//int ComputeShapeDeformation(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );
	//int ComputeShapeDeformation2(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );

	int SetMQOMaterial( CMQOMaterial* newmqomat, FbxSurfaceMaterial* material );
	void CreateFBXBoneReq(FbxScene* pScene, FbxNode* pNode, FbxNode* parnode );
	void CreateExtendBoneReq(CBone* srcbone);
	int GetFBXBone(FbxScene* pScene, FbxNodeAttribute::EType type, FbxNodeAttribute *pAttrib, FbxNode* curnode, FbxNode* parnode );


	int CreateFBXAnim( FbxScene* pScene, FbxNode* prootnode, BOOL motioncachebatchflag );
	void CreateFBXAnimReq( int animno, FbxScene* pScene, FbxPose* pPose, FbxNode* pNode, int motid, double animleng );
	int GetFBXAnim( int animno, FbxScene* pScene, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute *pAttrib, int motid, double animleng );
	void CreateFBXSkinReq( FbxNode* pNode );
	int GetFBXSkin( FbxNodeAttribute *pAttrib, FbxNode* pNode );

	void InitMpScaleReq(CBone* curbone, int srcmotid, double srcframe);
	int CorrectFbxScaleAnim(int animno, FbxScene* pScene, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute* pAttrib, int motid, double animleng);


	int DestroyFBXSDK();
	int GetShapeWeight(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj );
	float GetFbxTargetWeight(FbxNode* pbaseNode, FbxMesh* pbaseMesh, std::string targetname, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* baseobj );
	
	int SetDefaultBonePos();


	//SetDefaultBonePosReq�͊֐�����Fbx��t���� FbxFile.h, FbxFile.cpp�Ɉړ��ɂȂ�܂����B�����N�G���[�̊֌W�ŁB
	//void SetDefaultBonePosReq( CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix pParentGlobalPosition );



	void FillUpEmptyKeyReq( int motid, double animleng, CBone* curbone, CBone* parentbone );

	int SetMaterialName();

	int DestroyBtObject();
	void DestroyBtObjectReq( CBtObject* curbt );
	void CreateBtObjectReq( CBtObject* parbt, CBone* parentbone, CBone* curbone );

	void CalcBtAxismatReq( CBone* curbone, int onfirstcreate );
	void CalcRigidElemReq(CBone* curbone);
	void SetBtMotionReq( CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat );
	void SetBtMotionPostLowerReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat, int kinematicadjustflag);
	void SetBtMotionPostUpperReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void SetBtMotionMass0BottomUpReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void FindAndSetKinematicReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);//Kinematic�Ƃ����łȂ��Ƃ���̋��ڂ�T���Ă݂�������LowerReq�Őe�s����Z�b�g����B
	void SetBtMotionKinematicLowerReq(CBtObject* curbto, ChaMatrix oldparentmat, ChaMatrix newparentmat);
	void AdjustBtMatToParent(CBone* curbone, CBone* childbone, int adjustrot = 0);
	void AdjustBtMatToCurrent(CBone* curbone);
	void AdjustBtMatToChild(CBone* curbone, CBone* childbone, int adjustrot = 0);
	void InitBtMatTraAnimReq(CBtObject* curbto);

	void BtMat2BtObjReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void RecalcConstraintFrameABReq(CBtObject* curbto);

	void FindBtObjectReq( CBtObject* srcbto, int srcboneno, CBtObject** ppret );
	void SetImpulseDataReq( int gid, CBone* srcbone, ChaVector3 srcimp );
	void SetBtImpulseReq( CBone* srcbone );
	
	void SetSphrateDataReq(int gid, int rgdindex, CBone* srcbone, float srcval);
	void SetBoxzrateDataReq(int gid, int rgdindex, CBone* srcbone, float srcval);
	void SetSkipflagDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval);
	void SetForbidrotDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval);
	void SetColtypeDataReq(int gid, int rgdindex, CBone* srcbone, int srcval);
	void SetBtforceDataReq(int rgdindex, CBone* srcbone, bool srcval);
	void SetDampAnimDataReq( int gid, int rgdindex, CBone* srcbone, float valL, float valA );
	void SetBtgDataReq( int gid, int reindex, CBone* srcbone, float btg );
	void SetKDataReq( int gid, int reindex, CBone* srcbone, int srclk, int srcak, float srccuslk, float srccusak );
	void SetRestDataReq( int gid, int reindex, CBone* srcbone, float rest, float fric );
	void SetDmpDataReq( int gid, int reindex, CBone* srcbone, float ldmp, float admp );
	void SetMassDataReq( int gid, int reindex, CBone* srcbone, float srcmass );
	void SetMassDataByBoneLengReq(int gid, int reindex, CBone* srcbone, float srcmass);
	void SetColTypeReq(int reindex, CBone* srcbone, int srctype);

	//void SetBtKinFlagReq(CBtObject* srcbto, int oncreateflag);
	void Motion2BtReq( CBtObject* srcbto );
	//void SetBtGravityReq( CBtObject* srcbto );
	void SetRagdollKinFlagReq( CBtObject* srcbto, int selectbone, int physicsmvkind = 0 );
	//void CreateBtConnectReq( CBone* curbone );
	void SetColiIDReq( CBone* srcbone, CRigidElem* srcre );
	void EnableAllRigidElemReq(CBone* srcbone, int srcrgdindex);
	void DisableAllRigidElemReq(CBone* srcbone, int srcrgdindex);

	int DampAnim( MOTINFO* rgdmorphinfo );

	void ResetBtReq( CBtObject* curbto );

	int GetValidUndoID( int* rbundoid );
	int GetValidRedoID( int* rbundoid );

	int SetBefEditMat( CEditRange* erptr, CBone* curbone, int maxlevel );
	int SetBefEditMatFK( CEditRange* erptr, CBone* curbone );

	int AdjustBoneTra( CEditRange* erptr, CBone* lastpar );

	void SetFirstFrameBonePosReq(CBone* srcbone, int srcmotid, HINFO* phinfo);
	void InterpolateBetweenSelectionReq(CBone* srcbone, double srcstartframe, double srcendframe);


	int DbgDumpBoneReq(CBone* boneptr, int broflag);
	void DumpBtObjectReq(CBtObject* srcbto, int srcdepth);
	void DumpBtConstraintReq(CBtObject* srcbto, int srcdepth);
	FbxPose* GetBindPose();

	void CreatePhysicsPosConstraintReq(CBone* srcbone, int forceflag = 0);
	void DestroyPhysicsPosConstraintReq(CBone* srcbone, int forceflag = 0);
	void SetMass0Req(CBone* srcbone, bool forceflag);
	void RestoreMassReq(CBone* srcbone);
	void SetKinematicTmpLowerReq(CBone* srcbone, bool srcflag);

	void PhysicsMVReq(CBone* srcbone, ChaVector3 diffvec);
	int WithConstraint(CBone* srcbone);
	void BulletSimulationStopReq(CBtObject* srcbto);
	void BulletSimulationStartReq(CBtObject* srcbto);
	void ApplyBtToMotionReq(CBone* srcbone);

	void SetCurrentRigidElemReq(CBone* srcbone, std::string curname);

	//int GetFreeThreadIndex();
	//void WaitAllTheadOfGetFbxAnim();

	//static unsigned __stdcall ThreadFunc_MP1(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP2(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP3(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP4(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP5(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP6(void* pArguments);

public: //accesser
	FbxManager* GetFBXSDK(){
		return m_psdk;
	};
	int GetModelNo(){ return m_modelno; };
	void SetModelNo( int srcno ){ m_modelno = srcno; };

	bool GetModelDisp(){ return m_modeldisp; };
	void SetModelDisp( bool srcflag ){ m_modeldisp = srcflag; };

	float GetLoadMult(){ return m_loadmult; };
	void SetLoadMult( float srcmult ){ m_loadmult = srcmult; };

	const WCHAR* GetFileName(){ return m_filename; };
	const WCHAR* GetDirName(){ return m_dirname; };
	const WCHAR* GetModelFolder(){ return m_modelfolder; };
	const char* GetDefaultReName(){ return m_defaultrename; };
	const char* GetDefaultImpName(){ return m_defaultimpname; };

	int GetMqoObjectSize(){
		return (int)m_object.size();
	};
	CMQOObject* GetMqoObject( int srcobjno ){
		map<int, CMQOObject*>::iterator itrobj;
		itrobj = m_object.find(srcobjno);
		if (itrobj != m_object.end()){
			return itrobj->second;
		}
		else{
			return 0;
		}
		//return m_object[ srcobjno ];
	};
	std::map<int,CMQOObject*>::iterator GetMqoObjectBegin(){
		return m_object.begin();
	};
	std::map<int,CMQOObject*>::iterator GetMqoObjectEnd(){
		return m_object.end();
	};
	void SetMqoObject( int srcindex, CMQOObject* srcobj ){
		m_object[ srcindex ] = srcobj;
	};

	CBone* GetBoneByName( string srcname ){
		map<string, CBone*>::iterator itrbone;
		itrbone = m_bonename.find(srcname);
		if (itrbone != m_bonename.end()){
			return itrbone->second;
		}
		else{
			return 0;
		}
		//return m_bonename[ srcname ];
	};

	CBone* GetBoneByWName(WCHAR* srcname){
		if (!srcname){
			return 0;
		}
		size_t namelen = wcslen(srcname);
		if ((namelen > 0) && (namelen < 256)){
			char mbname[1024] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, srcname, -1, mbname, 1024, NULL, NULL);
			return GetBoneByName(mbname);
		}
		else{
			return 0;
		}
	};

	int GetBoneListSize(){
		return (int)m_bonelist.size();
	};
	std::map<int,CBone*>::iterator GetBoneListBegin(){
		return m_bonelist.begin();
	};
	std::map<int,CBone*>::iterator GetBoneListEnd(){
		return m_bonelist.end();
	};
	CBone* GetBoneByID( int srcid ){
		map<int, CBone*>::iterator itrbone;
		itrbone = m_bonelist.find(srcid);
		if (itrbone != m_bonelist.end()){
			return itrbone->second;
		}
		else{
			return 0;
		}
		//return m_bonelist[ srcid ];
	};
	CBone* GetBoneByZeroBaseIndex(int srcindex) {
		int bonenum = (int)m_bonelist.size();
		if ((srcindex < 0) || (srcindex >= bonenum)) {
			return 0;
		}
		std::map<int, CBone*>::iterator itrbone;
		int curindex = 0;
		itrbone = m_bonelist.begin();
		while (curindex < srcindex) {
			itrbone++;
			if (itrbone == m_bonelist.end()) {
				return 0;
			}
			curindex++;
		}
		if (itrbone != m_bonelist.end()) {
			return itrbone->second;
		}
		else {
			return 0;
		}
	};



	CBone* GetTopBone(){
		return m_topbone;
	};

	CBtObject* GetTopBt(){
		return m_topbt;
	};

	float GetBtGScale(int srcindex){
		if ((srcindex >= 0) && (srcindex < (int)m_rigideleminfo.size())){
			return m_rigideleminfo[srcindex].btgscale;
		}
		else{
			_ASSERT(0);
			return 0.0f;
		}
	};
	void SetBtGScale( float srcval, int srcindex ){
		if ((srcindex >= 0) && (srcindex < (int)m_rigideleminfo.size())){
			m_rigideleminfo[srcindex].btgscale = srcval;
		}
		else{
			_ASSERT(0);
		}
	};

	int GetMotInfoSize(){
		return (int)m_motinfo.size();
	};
	MOTINFO* GetMotInfo( int srcid ){//motid��1����
		//return m_motinfo[srcid - 1];

		//DeleteMotion���ɗv�f��erase����̂�id - 1���z��̃C���f�b�N�X�ɂȂ�Ƃ͌���Ȃ�//2021/08/26
		int miindex;
		miindex = MotionID2Index(srcid);
		if (miindex >= 0) {
			return m_motinfo[miindex];
		}
		else {
			return 0;
		}
	};
	std::map<int,MOTINFO*>::iterator GetMotInfoBegin(){
		return m_motinfo.begin();
	};
	std::map<int,MOTINFO*>::iterator GetMotInfoEnd(){
		return m_motinfo.end();
	};

	MOTINFO* GetCurMotInfo(){
		return m_curmotinfo;
	};
	void SetCurMotInfo( MOTINFO* srcinfo ){
		m_curmotinfo = srcinfo;
	};

	CRigidElem* GetRigidElem(int srcboneno);
	CRigidElem* GetRgdRigidElem(int srcrgdindex, int srcboneno);

	int GetRigidElemInfoSize(){
		return (int)m_rigideleminfo.size();
	};
	REINFO GetRigidElemInfo( int srcindex ){
		return m_rigideleminfo[ srcindex ];
	};
	void PushBackRigidElemInfo( REINFO srcinfo ){
		m_rigideleminfo.push_back( srcinfo );
	};
	void SetRigidElemInfo( int srcindex, REINFO srcinfo ){
		m_rigideleminfo[ srcindex ] = srcinfo;
	};

	int GetImpInfoSize(){
		return (int)m_impinfo.size();
	};
	string GetImpInfo( int srcindex ){
		return m_impinfo[ srcindex ];
	};
	string GetCurImpName(){
		string curimpname = m_impinfo[m_curimpindex];
		return curimpname;
	};
	void PushBackImpInfo( std::string srcname )
	{
		m_impinfo.push_back( srcname );
	};


	int GetCurReIndex(){
		return m_curreindex;
	};

	int GetCurImpIndex(){
		return m_curimpindex;
	};
	void SetCurImpIndex( int srcindex ){
		m_curimpindex = srcindex;
	};


	int GetRgdIndex(){
		return m_rgdindex;
	};
	void SetRgdIndex( int srcindex ){
		m_rgdindex = srcindex;
	};

	int GetRgdMorphIndex(){
		return m_rgdmorphid;
	};
	void SetRgdMorphIndex( int srcindex ){
		m_rgdmorphid = srcindex;
	};

	float GetTmpMotSpeed(){
		return m_tmpmotspeed;
	};

	int GetMQOMaterialSize(){
		return (int)m_material.size();
	};
	CMQOMaterial* GetMQOMaterial( int srcindex ){
		return m_material[ srcindex ];
	};
	std::map<int,CMQOMaterial*>::iterator GetMQOMaterialBegin(){
		return m_material.begin();
	};
	std::map<int,CMQOMaterial*>::iterator GetMQOMaterialEnd(){
		return m_material.end();
	};

	void SetMQOMaterial( int srcindex, CMQOMaterial* srcmat ){
		m_material[ srcindex ] = srcmat;
	};

	CMQOMaterial* GetMQOMaterialByName( string srcname ){
		return m_materialname[ srcname ];
	};
	std::map<std::string,CMQOMaterial*>::iterator GetMQOMaterialNameBegin(){
		return m_materialname.begin();
	};
	std::map<std::string,CMQOMaterial*>::iterator GetMQOMaterialNameEnd(){
		return m_materialname.end();
	};


	FbxScene* GetScene(){
		return m_pscene;
	};

	FbxString* GetAnimStackName( int srcindex ){
		return mAnimStackNameArray[ srcindex ];
	};

	int GetBtCnt(){
		return m_btcnt;
	};
	void PlusPlusBtCnt(){
		m_btcnt++;
	};
	void ZeroBtCnt(){
		m_btcnt = 0;
	}

	void SetBtWorld( btDynamicsWorld* srcworld ){
		m_btWorld = srcworld;
	};

	int GetOldAxisFlagAtLoading()
	{
		return m_oldaxis_atloading;
	};
	void SetOldAxisFlagAtLoading(int srcflag)
	{
		m_oldaxis_atloading = srcflag;
	};

	CMQOObject* GetObjectByName(string strname)
	{
		map<string, CMQOObject*>::iterator itrobjname;
		itrobjname = m_objectname.find(strname);
		if (itrobjname == m_objectname.end()){
			return 0;
		}
		else{
			return itrobjname->second;
		}
	};

	bool GetCreateBtFlag()
	{
		return m_createbtflag;
	};
	void SetCreateBtFlag(bool srcflag)
	{
		m_createbtflag = srcflag;
	};
	bool GetLoadedFlag()
	{
		return m_loadedflag;
	};
	void SetLoadedFlag(bool srcflag)
	{
		m_loadedflag = srcflag;
	};
	int GetInitAxisMatX(){
		return m_initaxismatx;
	};
	void SetInitAxisMatX(int srcval){
		m_initaxismatx = srcval;
	};

	void SetModelPosition(ChaVector3 srcpos){
		m_modelposition = srcpos;
	};
	ChaVector3 GetModelPosition(){
		return m_modelposition;
	};
	void SetWorldMatFromCamera(ChaMatrix srcmat){
		m_worldmat = srcmat;
		m_worldmat._41 = m_modelposition.x;
		m_worldmat._42 = m_modelposition.y;
		m_worldmat._43 = m_modelposition.z;
	};
	ChaMatrix GetWorldMat()
	{
		return m_worldmat;
	};

	int GetFbxComment(char* dstcomment, int dstlen)
	{
		if (!dstcomment) {
			_ASSERT(0);
			return 1;
		}
		if ((dstlen < 0) || (dstlen > MAX_PATH)) {
			_ASSERT(0);
			return 1;
		}
		strcpy_s(dstcomment, dstlen, m_fbxcomment.Buffer());
		return 0;
	}

	FbxNode* GetBoneNode(CBone* curbone) {
		if (!curbone) {
			return 0;
		}
		else {
			FbxNode* pNode = m_bone2node[curbone];
			return pNode;
		}
	}

	int GetHasBindPose() {
		return m_hasbindpose;
	}
	void SetHasBindPose(int srcval) {
		m_hasbindpose = srcval;
	}

public:
	//CRITICAL_SECTION m_CritSection_GetGP;
	//FUNCMPPARAMS* m_armpparams[6];
	//HANDLE m_arhthread[6];

	bool m_loadedflag;//�����̓ǂݍ��ݏ������I��������true;
	bool m_modeldisp;//�\�����邩�ǂ���
	bool m_createbtflag;//CreateBtObject��ǂ񂾂��Ƃ������true�B

private:
	
	int m_physicsikcnt;
	int m_initaxismatx;
	int m_modelno;//���f����ID
	float m_loadmult;//�\���{��
	int m_oldaxis_atloading;//FBX�ǂݍ��ݎ��ɋ��f�[�^�݊��`�F�b�N�{�b�N�X�Ƀ`�F�b�N�����Ă������ǂ����B
	int m_hasbindpose;


	WCHAR m_filename[MAX_PATH];//���f���t�@�C�����A�t���p�X�B
	WCHAR m_dirname[MAX_PATH];//���f���t�@�C���̃f�B���N�g���̃p�X�B�t�@�C�����̎�O�܂ŁB������\\�����B
	WCHAR m_modelfolder[MAX_PATH];//���f���t�@�C���������Ă���f�B���N�g���̖��O(�p�X�ł͂Ȃ�)�B*.cha�t�@�C��������f�B���N�g���̒���FBX�������Ă���f�B���N�g���̖��O�B
	char m_defaultrename[MAX_PATH];//RigidEelem�t�@�C��*.ref�̃f�t�H���g�̃t�@�C�����B
	char m_defaultimpname[MAX_PATH];//�C���p���X�t�@�C��*.imp�̃f�t�H���g�̃t�@�C�����B

	map<int, CMQOObject*> m_object;//�I�u�W�F�N�g�B�ʂ̌��t�ł����ƂRD���f���ɂ����閼�O���t�����Ă���p�[�c�╔�i�B
	map<int, CBone*> m_bonelist;//�{�[�����{�[��ID���猟���ł���悤�ɂ���map�B
	map<string, CBone*> m_bonename;//�{�[���𖼑O���猟���ł���悤�ɂ���map�B

	CBone* m_topbone;//��Ԑe�̃{�[���B
	CBtObject* m_topbt;//��Ԑe��bullet���̃I�u�W�F�N�g�B
	//float m_btgscale;//bullet�̏d�͂Ɋ|���Z����X�P�[���B--> m_rigideleminfo��btgscale�Ɉړ��B

	map<int, MOTINFO*> m_motinfo;//���[�V�����̃v���p�e�B�����[�V����ID���猟���ł���悤�ɂ���map�B
	MOTINFO* m_curmotinfo;//m_motinfo�̒��̌��ݍĐ�����MOTINFO�ւ̃|�C���^�B

	vector<REINFO> m_rigideleminfo;//���̐ݒ�t�@�C��*.ref�̃t�@�C������vector�B
	vector<string> m_impinfo;//�C���p���X�t�@�C��*.imp�̃t�@�C������vector�B

	int m_curreindex;//���ݗL���ɂȂ��Ă��鍄�̃t�@�C���̃C���f�b�N�X�B���̃t�@�C���͕����ǂݍ��ނ��Ƃ��o���āA�J�����g��ݒ�ł���B
	int m_curimpindex;//���ݗL���ɂȂ��Ă���C���p���X�t�@�C���̃C���f�b�N�X�B�C���p���X�t�@�C���͕����ǂݍ��ނ��Ƃ��o���A�J�����g��ݒ�ł���B
	int m_rgdindex;//���O�h�[���V�~�����[�V�����p�̍��̐ݒ�t�@�C���̃C���f�b�N�X�B
	int m_rgdmorphid;//���O�h�[�����Ɏg�p����A�j���[�V�����ւ̃C���f�b�N�X�B���O�h�[�����ɂ̓��[�t�A�j����ʏ�A�j�����ƕς��邱�Ƃ��������߂��̐ݒ肪���݂���B�Ⴆ�΂����̃��[�t�A�j�����Đ������肷��B

	float m_tmpmotspeed;//���[�V�����Đ��{���̈ꎞ�ۑ��p�B

	//polymesh3�p�̃}�e���A���Bpolymesh4��mqoobject��mqomaterial���g�p����B
	map<int, CMQOMaterial*> m_material;
	map<string, CMQOMaterial*> m_materialname;

	FbxScene* m_pscene;//FBX SDK�̃V�[���ւ̃|�C���^�BCModel���ŃA���P�[�g�B
	FbxArray<FbxString*> mAnimStackNameArray;//�A�j���[�V��������ۑ�����FBX�`���̃f�[�^�B

	btDynamicsWorld* m_btWorld;//bullet�̃V�~�����[�V�����p�I�u�W�F�N�g�ւ̃|�C���^�B
	int m_btcnt;//bullet�̃V�~�����[�V�����������t���[�������L�^����B����̃V�~�����[�V�������ɈقȂ鏈�������邽�߂ɕK�v�ł���B


//�ȉ��Aprivate�����[�J�����炵���Q�Ƃ��Ȃ��f�[�^������A�N�Z�b�T�[�������B
	ID3D11Device* m_pdev;//�O���������BDirect3D��Device�B
	FbxManager* m_psdk;//�O���������BFBX SDK�̃}�l�[�W���B
	FbxImporter* m_pimporter;//FBX SDK�̃C���|�[�^�[�BCModel���ŃA���P�[�g�B
	FbxString m_fbxcomment;
	WCHAR m_fbxfullname[MAX_PATH];
	bool m_useegpfile;

	ChaMatrix m_matWorld;//���[���h�ϊ��s��B
	ChaMatrix m_matVP;//View * Projection �ϊ��s��B

	map<CMQOObject*, FBXOBJ*> m_fbxobj;//FbxNode�̃��b�p�[�N���X��CMQOObject�Ƃ�map�B
	map<string, CMQOObject*> m_objectname;//CMQOObject�𖼑O�Ō������邽�߂�map�B


	int (*m_tlFunc)( int srcmotid );//Main.cpp�Ɏ��Ԃ�����^�C�����C���������p�̊֐��ւ̃|�C���^�B�f�[�^�ǂݍ��ݎ���CModel���珉�����֐����ĂԁB

	mutable FbxTime mStart, mStop, mFrameTime, mFrameTime2;//Fbx�ł̎��ԕ\���B�A�j���[�V�����̎���(�t���[��)�w��ȂǂɎg�p�B

	map<CBone*, FbxNode*> m_bone2node;//�{�[����FbxNode�̑Ή��\�BFbxNode��FBX�t�@�C�����̃I�u�W�F�N�g�̊K�w�����ǂ�ۂȂǂɗ��p����B
	map<CBone*,CBone*> m_rigidbone;//���̂P�̓{�[���P�ɑΉ����Ă���B�{�[���͐e�̃W���C���g�Ǝq���̃W���C���g����Ȃ�B�W���C���g�ƃ{�[���͓����悤�ɌĂԂ��Ƃ�����B���̂̐e�{�[�����q���{�[�����炯�񂳂����邱�ƂɎg�p����B

	int m_texpool;//Direct3D�̃e�N�X�`���쐬�v�[���i�ꏊ�j�B�V�X�e�����������r�f�I���������}�l�[�W�h���I�ԁB�ʏ��0�Ńr�f�I���������w�肷��B
	ChaVector3 m_ikrotaxis;//IK, FK�Ń{�[����]���邽�߂̉�]�����ꎞ�I�ɕۑ�����B
	CUndoMotion m_undomotion[ UNDOMAX ];//�A���h�D�[�@�\�̂��߂�CUndoMotion�̔z��BCUndoMotion�̂P�̃C���X�^���X�͂P�t���[�����̃��[�V������ۑ�����B
	int m_undoid;//�A���h�D�[�p�f�[�^�������O�o�b�t�@�Ŏg�p���邽�߂̌��݈ʒu�ւ̃C���f�b�N�X�B

	ChaMatrix m_worldmat;
	ChaVector3 m_modelposition;

	std::vector<PHYSIKREC> m_physikrec0;
	std::vector<PHYSIKREC> m_physikrec;
	double m_phyikrectime;

};

#endif