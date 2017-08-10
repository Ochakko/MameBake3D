#ifndef BONEH
#define BONEH

#include <d3dx9.h>
#include <Coef.h>
#include <MotionPoint.h>
#include <quaternion.h>

#include <fbxsdk.h>

#include <map>
#include <string>

#include <orgwindow.h>
#include <Coef.h>

class CMQOFace;
class CMotionPoint;
class CRigidElem;
class CBtObject;
class CModel;

class CBone
{
public:

	D3DXVECTOR3 m_btparpos;//Motion2Bt���̃{�[���̈ʒu(���̍s��v�Z�p)
	D3DXVECTOR3 m_btchilpos;//Motion2Bt���̃{�[���̈ʒu(���̍s��v�Z�p)
	D3DXMATRIX m_btdiffmat;//Motion2Bt����btmat�̕ω���(���̍s��v�Z�p)

/**
 * @fn
 * CBone
 * @breaf CBone�̃R���X�g���N�^�B
 * @param (CModel* parmodel) IN �e�{�[���ւ̃|�C���^���w�肷��B
 * @return �Ȃ��B
 */
	CBone( CModel* parmodel );

/**
 * @fn
 * ~CBone
 * @breaf CBone�̃f�X�g���N�^�B
 * @return �Ȃ��B
 */
	~CBone();

/**
 * @fn 
 * SetName
 * @breaf �{�[���̖��O��ݒ肷��B
 * @param (char* srcname) IN �ݒ肵�����{�[���̖��O���w�肷��B
 * @return ����������O�B
 * @detail char�̃{�[���̖��O��WCHAR�ɕϊ�����ă{�[����UNICODE�����Z�b�g�����B
 */
	int SetName( char* srcname );

/**
 * @fn 
 * AddChild
 * @breaf �{�[���̊K�w�\����ݒ肷��B
 * @param (CBone* childptr) IN �q���ɂ���CBone�̃|�C���^���w�肷��B
 * @return ����������O�B
 * @detail �q�����w�肷�邱�ƂŊK�w��ݒ肷��B�q�������łȂ����q���̐e�̐ݒ������B
 */
	int AddChild( CBone* childptr );

/**
 * @fn 
 * UpdateMatrix
 * @breaf �{�[���̌��݂̎p�����v�Z����B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (D3DXMATRIX* wmat) IN ���[���h���W�n�̕ϊ��s����w�肷��B
 * @param (D3DXMATRIX* vpmat) IN �J�����v���W�F�N�V�������W�n�̕ϊ��s����w�肷��B
 * @return ����������O�B
 * @detail �w�胂�[�V�����̎w�莞�Ԃ̎p�����v�Z����B�O���[�o���Ȏp���̌v�Z�ł���B
 */
	int UpdateMatrix( int srcmotid, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat );

/**
 * @fn
 * AddMotionPoint
 * @breaf �w�胂�[�V�����̎w�莞�Ԃ�MotionPoint��ǉ�����B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (int* existptr) OUT �ǉ�����O����MotionPoint�����݂��Ă�����P���Z�b�g�����A����ȊO�̏ꍇ�͂O���Z�b�g�����B
 * @return ����������ǉ�����CMotionPoint�̃|�C���^�A���s���͂O�B
 * @detail �ǉ�����MotionPoint�̎p����Identity��Ԃł���B
 */
	CMotionPoint* AddMotionPoint( int srcmotid, double srcframe, int* existptr );


/**
 * @fn
 * CalcFBXMotion
 * @breaf �w�胂�[�V�����̎w�莞�Ԃ̃|�[�Y(CMotionPoint)���v�Z����B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[��(����)���w�肷��B
 * @param (CMotionPoint* dstmpptr) OUT �v�Z�����p�����Z�b�g�����B
 * @param (int* existptr) OUT �w�莞�Ԃ��傤�ǂ̃f�[�^������΂P�A�Ȃ���΂O�B�����ꍇ�͑O�ォ��v�Z����B
 * @return ����������O�B
 */
	int CalcFBXMotion( int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr );

/**
 * @fn
 * CalcFBXFrame
 * @breaf �w�莞�Ԃ̑O��̎p������w�莞�Ԃ̎p�����v�Z����B
 * @param (double srcframe) IN �w�莞�ԁB
 * @param (CMotionPoint* befptr) IN �w�莞�Ԃ̒��O�̎p���B
 * @param (CMotionPoint* nextptr) IN �w�莞�Ԃ̒���̎p���B
 * @param (int existflag) IN �w�莞�Ԃ��傤�ǂɎp���f�[�^������Ƃ��P�B���̎����傤�ǂ̂Ƃ��̎p����befptr�ɓ����Ă���B
 * @param (CMotionPoint* dstmpptr) OUT �w�莞�Ԃ̎p�����v�Z����ăZ�b�g�����B
 * @return ����������O�B
 * @detail �w�莞�Ԃ��傤�ǂɃf�[�^���������ꍇ�͂��ꂪ���ʂɂȂ�Bbefptr���Ȃ��ꍇ��nextptr�̎p�������ʂƂȂ�Bnextptr�������ꍇ��befptr�̎p�������ʂƂȂ�Bbef��next������ꍇ�͐��`��Ԃ���B
 */
	int CalcFBXFrame( double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr );

/**
 * @fn
 * GetBefNextMP
 * @breaf �w�胂�[�V�����̎w�莞�Ԃ̒��O�ƒ���̎p���f�[�^���擾����B
 * @param (int srcmotid) IN ���[�V����ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (CMotionPoint** ppbef) OUT ���O�̎p���f�[�^�B
 * @param (CMotionPoint** ppnext) OUT ����̎p���f�[�^�B
 * @param (int* existptr) OUT ���Ԃ��傤�ǂ̃f�[�^������Ƃ��ɂP�B
 * @return ����������O�B
 * @detail existptr�̓��e���P�̂Ƃ��A���傤�ǂ̎��Ԃ̎p����ppbef�ɃZ�b�g�����B
 */
	int GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr );

/**
 * @fn
 * DeleteMotion
 * @breaf �w�肵�����[�V����ID�̃��[�V�������폜����B
 * @param (int srcmotid) IN ���[�V������ID�B
 * @return ����������O�B
 * @detail �w�肵�����[�V�����̎p���f�[�^���폜����B
 */
	int DeleteMotion( int srcmotid );

/**
 * @fn
 * DeleteMPOutOfRange
 * @breaf �w�肵�����[�V�����̒������͂ݏo���Ă���(���Ԃ��͂ݏo���Ă���)�p���f�[�^���폜����B
 * @param (int motid) IN ���[�V������ID���w�肷��B
 * @param (double srcleng) IN ���[�V�����̒���(����)���w�肷��B
 * @return ����������O�B
 * @detail ���[�V�����̒�����ύX����Ƃ��ɔ͈͊O�̎p�����폜���邽�߂ɌĂ΂��B
 */
	int DeleteMPOutOfRange( int motid, double srcleng );

/**
 * @fn
 * CalcRigidElemParams
 * @breaf ���̕\���p�̃f�[�^�����̂̃p�����[�^�ɏ]���ăX�P�[�����邽�߂̕ϊ��s������߂ăX�P�[������B
 * @param (CBone* chilbone) IN�@���̂��w�肷�邽�߂̎q���{�[���B
 * @param (int setstartflag) IN�@���̃V�~�����[�V�����J�n���̌Ăяo�����ɂP���Z�b�g����B
 * @return ����������O�B
 * @detail ���̂��{�[���̈ʒu�ɕ\�����邽�߂ɁA���̕\���p�̌`����X�P�[�����邽�߂ɌĂԁB���̂̓{�[���̎q���W���C���g�ƂP�΂P�őΉ����邽�߁A�w���chilbone���g���B
 */
	int CalcRigidElemParams( CBone* chilbone, int setstartflag );

/**
 * @fn
 * CalcAxisMat
 * @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B������Ԃ�X���������Ă���Ɖ��肵�Čv�Z����B
 * @param (int firstflag) IN �ϊ��J�n�̍s���ۑ��������Ƃ��ɂP���w�肷��B
 * @param (float delta) IN ��]�p�x�����Ă�����B���݂͂O�ł����g�p���Ă��Ȃ��B
 * @return ����������O�B
 * @detail �v�Z�����f�[�^�́A�{�[���̈ʒu�Ƀ}�j�s�����[�^��\�����邽�߂̕ϊ��s��Ɏg�p����B���݂�CalcAxisMatZ�֐��Ń{�[���̕ϊ��s����v�Z���Ă���B
 */
	int CalcAxisMat( int firstflag, float delta );
	

/**
 * @fn
 * CalcAxisMatZ
 * @breaf �{�[���̐e�Ǝq���̈ʒu���w�肵�āA���̎��̕ϊ��s����v�Z����B
 * @param (D3DXVECTOR3* srccur) IN �{�[���̐e�̍��W���w�肷��B
 * @param (D3DXVECTOR3* srcchil) IN �{�[���̎q���̍��W���w�肷��B
 * @return ����������O�B
 * @detail ������Ԃ�Z���������Ă���Ɖ��肵�Čv�Z����B�{�[���̈ʒu�Ƀ}�j�s�����[�^��\�����邽�߂Ɏg�p����B
 */
	int CalcAxisMatZ( D3DXVECTOR3* srccur, D3DXVECTOR3* srcchil );


/**
 * @fn
 * CreateRigidElem
 * @breaf ���̃V�~�����[�V�����p�̃p�����[�^��ێ�����CRigidElem���쐬����B�w��{�[���Ɋւ���CRigidElem���쐬����B
 * @param (CBone* parbone) IN �������g�Ƃ����Ŏw�肷��e�W���C���g�Œ�`�����{�[���Ɋւ���CRigidElem���쐬����B
 * @param (int reflag) IN CRigidElem���쐬����ꍇ�ɂP�A���Ȃ��ꍇ�ɂO���w��B
 * @param (std::string rename) IN reflag���P�̂Ƃ��Aref�t�@�C���̃t�@�C�������w�肷��B
 * @param (int impflag) IN �C���p���X��^���邽�߂̐ݒ���쐬����ꍇ�ɂP�A���Ȃ��ꍇ�ɂO���w��B
 * @param (std::string impname) IN impflag���P�̂Ƃ��Aimp�t�@�C���̃t�@�C�������w�肷��B
 * @return ����������O�B
 * @detail �C���p���X�ݒ�f�[�^���쐬����B
 */
	//int CreateRigidElem( CBone* chil, int reflag, std::string rename, int impflag, std::string impname );
	int CreateRigidElem(CBone* parbone, int reflag, std::string rename, int impflag, std::string impname);


/**
 * @fn
 * SetCurrentRigidElem
 * @breaf �J�����g�ݒ�Ƃ��Ďg�p����RigidElem���Z�b�g����B
 * @param (std::string curname) �J�����g�ɂ�����ref�t�@�C���̖��O���w�肷��B
 * @return ����������O�B
 */
	int SetCurrentRigidElem( std::string curname );


/**
 * @fn
 * AddBoneTraReq
 * @breaf �{�[���̎p�����w��x�N�g�������ړ�����B�q�������ɍċA�I�Ɍv�Z����B
 * @param (CMotionPoint* parmp) IN �O���w�肵�����͎����̎p�����v�Z����B�O�ȊO�̐e���w�肵���ꍇ�͎q�������Ɏp���̕ύX��`���Ă����B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (D3DXVECTOR3 srctra) IN �ړ��x�N�g�����w�肷��B
 * @return �v�Z�����p�����i�[����CMotionPoint�̃|�C���^��Ԃ����ċA�֐��ł��邱�Ƃɒ��ӁB�|�C���^�̓`�F�C���ɃZ�b�g���ꂽ���̂ł���B
 * @detail �z�肵�Ă���g�����Ƃ��ẮA�O������̌Ăяo�����ɂ�parmp���O�ɂ���B���̊֐����ł̍ċA�Ăяo������parmp�ɐe���Z�b�g����B
 */
	CMotionPoint* AddBoneTraReq( CMotionPoint* parmp, int srcmotid, double srcframe, D3DXVECTOR3 srctra );

/**
 * @fn
 * RotBoneQReq
 * @breaf �{�[���̎p�����w��N�H�[�^�j�I���̕���]����B�q�������ɍċA�I�Ɍv�Z����B
 * @param (CMotionPoint* parmp) IN �O���w�肵�����͎����̎p�����v�Z����B�O�ȊO�̐e���w�肵���ꍇ�͎q�������Ɏp���̕ύX��`���Ă����B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (CQuaternion rotq) IN ��]�̂��߂̃N�H�[�^�j�I�����w�肷��B
 * @return �v�Z�����p�����i�[����CMotionPoint�̃|�C���^��Ԃ����ċA�֐��ł��邱�Ƃɒ��ӁB�|�C���^�̓`�F�C���ɃZ�b�g���ꂽ���̂ł���B
 * @detail �z�肵�Ă���g�����Ƃ��ẮA�O������̌Ăяo�����ɂ�parmp���O�ɂ���B���̊֐����ł̍ċA�Ăяo������parmp�ɐe���Z�b�g����B
 */
	CMotionPoint* RotBoneQReq(CMotionPoint* parmp, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, D3DXVECTOR3 traanim = D3DXVECTOR3(0.0f, 0.0f, 0.0f), int setmatflag = 0, D3DXMATRIX* psetmat = 0);

/**

*/
	CMotionPoint* RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, D3DXMATRIX srcmat);

/**
 * @fn
 * PasteRotReq
 * @breaf srcframe�̎p����dstframe�̎p���Ƀy�[�X�g����B�q�������ɍċA�I�ɐݒ肷��B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN �R�s�[���̃t���[���i���ԁj���w�肷��B
 * @param (double dstframe) IN �R�s�[��̃t���[�����w�肷��B
 * @return dst��CMotionPoint�̃|�C���^��Ԃ��B�������ċA�I�ɂł���B
 */
	CMotionPoint* PasteRotReq( int srcmotid, double srcframe, double dstframe );


/**
 * @fn
 * SetAbsMatReq
 * @breaf firstframe�̎p�������̂܂�srcframe�ɓK�p����B�q�������ɍċA�I�Ɏ��s����B
 * @param (int broflag) IN ���̃{�[���̌Z��{�[���ɑ΂��Ă��������������ꍇ�͂P���A���̑��̏ꍇ�͂O���w�肷��B
 * @param (int srcmotid) IN ���[�V������ID���w�肷��B
 * @param (double srcframe) IN �p����K�p�������t���[�����w�肷��B
 * @param (double firstframe) IN �ҏW�̊J�n�t���[�����w�肷��B
 * @return �ҏW���K�p���ꂽ�{�[����CMotionPoint�̃|�C���^���Ԃ����B�������ċA�I�ɂł���B
 * @detail ���̊֐��͐��IK�@�\�Ƃ��ČĂ΂��B���IK�Ƒ���IK�̐�����Main.cpp�̖`���̐�����ǂނ��ƁB
 */
	CMotionPoint* SetAbsMatReq( int broflag, int srcmotid, double srcframe, double firstframe );


/**
 * @fn
 * DeleteMotionKey
 * @breaf �w�肵�����[�V����ID�̃��[�V�����L�[���폜����B
 * @param (int srcmotid) IN ���[�V������ID�B
 * @return ����������O�B
 * @detail �w�肵�����[�V�����̎p���f�[�^���폜����B
 * @detail DeleteMotion���L�[����������Ƀ��[�V�����̃G���g���[���폜����̂ɑ΂��A
 * @detail DestroyMotionKey�̓L�[���폜������Ƀ��[�V�����̃G���g���[��NULL���Z�b�g����B
 */
	int DestroyMotionKey( int srcmotid );


/**
 * @fn
 * AddBoneMotMark
 * @breaf �p���ҏW�}�[�N��t����B
 * @param (int motid) IN ���[�V������ID���w�肷��B
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN �^�C�����C�����w�肷��B
 * @param (int curlineno) IN �^�C�����C���̃��C���ԍ����w�肷��B
 * @param (double startframe) IN �ҏW�J�n�t���[�����w�肷��B
 * @param (double endframe) IN �ҏW�I���t���[�����w�肷��B
 * @param (int flag) IN �}�[�N�ɃZ�b�g����t���O�B�ʏ�P���w��B
 * @return ����������O�B
 * @detail �p���ҏW�̊J�n���ƏI�����Ƀ}�[�N��t���A���łɂ��̊ԂɃ}�[�N������΂���������B
 */
	int AddBoneMotMark( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag );


/**
 * @fn
 * CalcLocalInfo
 * @breaf �w�胂�[�V�����̎w�莞�Ԃ̎p�����烍�[�J���ȉ�]�N�H�[�^�j�I���ƈړ��x�N�g�����v�Z����B
 * @param (int motid) IN ���[�V������ID���w�肷��B
 * @param (double frameno) IN ���[�V�����̃t���[���i���ԁj���w�肷��B
 * @param (CMotionPoint* pdstmp) OUT �v�Z���ʂ��󂯎��CMotionPoint�̃|�C���^���w�肷��B
 * @return ����������O�B
 */
	int CalcLocalInfo( int motid, double frameno, CMotionPoint* pdstmp );

	int CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp);
/**
 * @fn
 * GetBoneNum
 * @breaf ���̃{�[����e�Ƃ���{�[���̐����擾����B
 * @return �{�[���̐��B
 * @detail �q���Ǝq���̌Z��̐��̍��v��Ԃ��B
 */
	int GetBoneNum();

	/**
	* @fn
	* CalcAxisMatX_aft
	* @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B������Ԃ�X���������Ă���Ɖ��肵�Čv�Z����B
	* @param (D3DXVECTOR3 curpos) IN �{�[���̈ʒu�B
	* @param (D3DXVECTOR3 chilpos) IN �q���̃{�[���̈ʒu�B
	* @param (D3DXMATRIX* destmat) OUT �ϊ��s����i�[����f�[�^�ւ̃|�C���^�B
	* @return ����������O�B
	* @detail CalcAxisMatX����Ă΂��B
	*/
	int CalcAxisMatX_aft(D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* destmat);

	int CalcAxisMatZ_aft(D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* destmat);


	int CalcFirstFrameBonePos(D3DXMATRIX srcmat);

	int CalcBoneDepth();

	D3DXVECTOR3 CalcLocalEulZXY(int axiskind, int srcmotid, double srcframe, enum tag_befeulkind befeulkind, int isfirstbone, D3DXVECTOR3* directbefeul = 0);//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)
	D3DXMATRIX CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe);
	int SetWorldMatFromEul(int inittraflag, int setchildflag, D3DXVECTOR3 srceul, int srcmotid, double srcframe);
	int SetWorldMatFromEulAndTra(int setchildflag, D3DXVECTOR3 srceul, D3DXVECTOR3 srctra, int srcmotid, double srcframe);
	int SetWorldMatFromQAndTra(int setchildflag, CQuaternion axisq, CQuaternion srcq, D3DXVECTOR3 srctra, int srcmotid, double srcframe);
	int SetLocalEul(int srcmotid, double srcframe, D3DXVECTOR3 srceul);
	void SetWorldMat(int setchildflag, int srcmotid, double srcframe, D3DXMATRIX srcmat);
	D3DXMATRIX GetWorldMat(int srcmotid, double srcframe);
	D3DXVECTOR3 CalcLocalTraAnim(int srcmotid, double srcframe);
	D3DXMATRIX CalcLocalRotMat(int rotcenterflag, int srcmotid, double srcframe);
	D3DXMATRIX CalcLocalSymRotMat(int rotcenterflag, int srcmotid, double srcframe);
	D3DXMATRIX CalcSymXMat(int srcmotid, double srcframe);
	D3DXMATRIX CalcSymXMat2(int srcmotid, double srcframe, int symrootmode);
	int PasteMotionPoint(int srcmotid, double srcframe, CMotionPoint srcmp);

	D3DXVECTOR3 CalcFBXEul(int srcmotid, double srcframe, D3DXVECTOR3* befeulptr = 0);
	D3DXVECTOR3 CalcFBXTra(int srcmotid, double srcframe);
	int QuaternionInOrder(int srcmotid, double srcframe, CQuaternion* srcdstq);
	int CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* chilbone, D3DXMATRIX* dstmat, D3DXVECTOR3* dstpos);

	int LoadCapsuleShape(LPDIRECT3DDEVICE9 pdev);
private:

/**
 * @fn
 * InitParams
 * @breaf �N���X�����o�̏�����������B
 * @return �O�B
 */
	int InitParams();

/**
 * @fn
 * DestroyObjs
 * @breaf �N���X�ŃA���P�[�g�������������J������B
 * @return �O�B
 */
	int DestroyObjs();


/**
 * @fn
 * CalcAxisMatX
 * @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B�{�[���̎����v�Z����CalcAxisMatX_aft�ɓn���Čv�Z����B
 * @return ����������O�B
 * @detail CalcAxisMat����Ă΂��B
 */
	int CalcAxisMatX();

	int CalcAxisMatZ();


/**
 * @fn
 * CalcAxisMatY
 * @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B������Ԃ�Z���������Ă���Ɖ��肵�Čv�Z����B
 * @param (CBone* chilbone) IN �q���̃{�[���B
 * @param (D3DXMATRIX* dstmat) OUT �ϊ��s����i�[����f�[�^�ւ̃|�C���^�B
 * @return ����������O�B
 */
	int CalcAxisMatY( CBone* chilbone, D3DXMATRIX* dstmat );

/**
 * @fn
 * CalcLocalAxisMat
 * @breaf ���[�J���ȃ{�[�����̕ϊ��s����v�Z����B
 * @param (D3DXMATRIX motmat) IN �O���[�o���Ȏp���s��B
 * @param (D3DXMATRIX axismatpar) IN �O���[�o���ȃ{�[�����ϊ��s��BX����B
 * @param (D3DXMATRIX gaxisy) IN �O���[�o���ȃ{�[�����ϊ��s��BY����B
 * @return ����������O�B
 * @detail �v�Z���ʂ�GetAxisMatPar()�Ŏ擾����B
 */
	int CalcLocalAxisMat( D3DXMATRIX motmat, D3DXMATRIX axismatpar, D3DXMATRIX gaxisy );

/**
 * @fn
 * SetGroupNoByName
 * @breaf ���̂̂����蔻��p�̃O���[�v�ԍ���ݒ肷��B�{�[���̖��O���画�肵�Đݒ肷��B
 * @param (CRigidElem* curre) IN ���̂�CRigidElem�ւ̃|�C���^�B
 * @param (CBone* chilbone) IN �{�[����CBone�ւ̃|�C���^�B
 * @return ����������O�B
 * @detail BT_�����O�ɂ����̂ɂ����̗p�̃O���[�v�ԍ����^������B����ȊO�̃O���[�v�ԍ��́Aname_G_*** �`���̖��O�Ŏw��ł���B
 */
	int SetGroupNoByName( CRigidElem* curre, CBone* chilbone );

/**
 * @fn
 * AddBoneMarkIfNot
 * @breaf �{�[���̕ҏW�}�[�N��t����B
 * @param (int motid) IN ���[�V������ID���w�肷��B
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN �^�C�����C�����w�肷��B
 * @param (int curlineno) IN �^�C�����C���̃��C���ԍ����w�肷��B
 * @param (double curframe) IN ���[�V�����̃t���[�����w�肷��B
 * @param (int flag) IN �}�[�N�ɃZ�b�g����l�A�ʏ�͂P���w�肷��B
 * @return ����������O�B
 */
	int AddBoneMarkIfNot( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double curframe, int flag );


/**
 * @fn
 * DelBoneMarkRange
 * @breaf �w��͈͓��̕ҏW�}�[�N���폜����B
 * @param (int motid) IN ���[�V������ID���w�肷��B
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN �^�C�����C�����w�肷��B
 * @param (int curlineno) IN �^�C�����C���̃��C���ԍ����w�肷��B
 * @param (double startframe) IN �폜�J�n�t���[���B
 * @param (double endframe) IN �폜�I���t���[���B
 * @return �߂�l�̐���
 */
	int DelBoneMarkRange( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe );

	void SetStartMat2Req();
	void CalcFirstAxisMatX();
	void CalcFirstAxisMatZ();

	void InitAngleLimit();
	int ChkMovableEul(D3DXVECTOR3 srceul);
	float LimitAngle(enum tag_axiskind srckind, float srcval);
	D3DXVECTOR3 LimitEul(D3DXVECTOR3 srceul);
	int InitCustomRig();
	void CalcBtRootDiffMatFunc(CBone* srcbone);

public: //accesser
	int GetType(){ return m_type; };
	void SetType( int srctype ){ m_type = srctype; };

	int GetSelectFlag(){ return m_selectflag; };
	void SetSelectFlag( int srcflag ){ m_selectflag = srcflag; };

	int GetBoneCnt(){ return m_bonecnt; };
	void SetBoneCnt( int srccnt ){ m_bonecnt = srccnt; };

	int GetValidFlag(){ return m_validflag; };
	void SetValidFlag( int srcflag ){ m_validflag = srcflag; };

	int GetBoneNo(){ return m_boneno; };
	void SetBoneNo( int srcno ){ m_boneno = srcno; };

	int GetTopBoneFlag(){ return m_topboneflag; };
	void SetTopBoneFlag( int srcflag ){ m_topboneflag = srcflag; };

	const char* GetBoneName(){ return (const char*)m_bonename; };
	void SetBoneName( char* srcname ){ strcpy_s( m_bonename, 256, srcname ); };

	const WCHAR* GetWBoneName(){ return (const WCHAR*)m_wbonename; };
	void SetWBoneName( WCHAR* srcname ){ wcscpy_s( m_wbonename, 256, srcname ); };

	const char* GetEngBoneName(){ return (const char*)m_engbonename; };
	void SetEngBoneName( char* srcname ){ strcpy_s( m_engbonename, 256, srcname ); };

	D3DXVECTOR3 GetChildWorld();
	void SetChildWorld( D3DXVECTOR3 srcvec ){ m_childworld = srcvec; };

	D3DXVECTOR3 GetChildScreen(){ return m_childscreen; };
	void SetChildScreen( D3DXVECTOR3 srcvec ){ m_childscreen = srcvec; };

	int GetMotionKeySize(){ return (int)m_motionkey.size(); };
	CMotionPoint* GetMotionKey( int srccookie ){ return m_motionkey[ srccookie ]; };
	void SetMotionKey( int srccookie, CMotionPoint* srcmk ){ m_motionkey[ srccookie ] = srcmk; };

	CMotionPoint GetCurMp(){ return m_curmp; };
	void SetCurMp( CMotionPoint srcmp ){ m_curmp = srcmp; };

	//CMotionPoint GetBefMp(){ return m_befmp; };
	//void SetBefMp(CMotionPoint srcmp){ m_befmp = srcmp; };


	CQuaternion GetAxisQ(){ return m_axisq; };
	void SetAxisQ( CQuaternion srcq ){ m_axisq = srcq; };

	D3DXMATRIX GetLAxisMat(){ return m_laxismat; };
	D3DXMATRIX GetAxisMatPar(){ return m_axismat_par; };
	D3DXMATRIX GetInvAxisMatPar(){
		D3DXMATRIX invaxis;
		D3DXMatrixInverse(&invaxis, NULL, &m_axismat_par);
		return invaxis;
	};

	D3DXMATRIX GetStartMat2(){ return m_startmat2; };
	D3DXMATRIX GetInvStartMat2(){
		D3DXMATRIX retmat;
		D3DXMatrixInverse(&retmat, NULL, &m_startmat2);
		return retmat;
	};
	void SetStartMat2(D3DXMATRIX srcmat){ m_startmat2 = srcmat; };

	int GetGetAnimFlag(){ return m_getanimflag; };
	void SetGetAnimFlag( int srcflag ){ m_getanimflag = srcflag; };

	D3DXMATRIX GetNodeMat(){ return m_nodemat; };
	D3DXMATRIX GetInvNodeMat(){
		D3DXMATRIX retmat;
		D3DXMatrixInverse(&retmat, NULL, &m_nodemat);
		return retmat;
	};
	void SetNodeMat( D3DXMATRIX srcmat ){ m_nodemat = srcmat; };


	D3DXMATRIX GetFirstMat(){ return m_firstmat; };
	void SetFirstMat( D3DXMATRIX srcmat ){ m_firstmat = srcmat; };

	D3DXMATRIX GetInvFirstMat(){ D3DXMatrixInverse(&m_invfirstmat, NULL, &m_firstmat); return m_invfirstmat; };
	void SetInvFirstMat( D3DXMATRIX srcmat ){ m_invfirstmat = srcmat; };

	D3DXMATRIX GetInitMat(){ return m_initmat; };
	void SetInitMat(D3DXMATRIX srcmat){ m_initmat = srcmat; };

	D3DXMATRIX GetInvInitMat(){ D3DXMatrixInverse(&m_invinitmat, NULL, &m_initmat); return m_invinitmat; };
	void SetInvInitMat(D3DXMATRIX srcmat){ m_invinitmat = srcmat; };

	D3DXMATRIX GetTmpMat(){ return m_tmpmat; };
	void SetTmpMat(D3DXMATRIX srcmat){ m_tmpmat = srcmat; };

	CQuaternion GetTmpQ(){ return m_tmpq; };
	void SetTmpQ(CQuaternion srcq){ m_tmpq = srcq; };


	D3DXVECTOR3 GetJointFPos();
	void SetJointFPos(D3DXVECTOR3 srcpos);
	void SetOldJointFPos(D3DXVECTOR3 srcpos);

	D3DXVECTOR3 GetJointWPos(){ return m_jointwpos; };
	void SetJointWPos( D3DXVECTOR3 srcpos ){ m_jointwpos = srcpos; };

	FbxAMatrix GetGlobalPosMat(){ return m_globalpos; };
	void SetGlobalPosMat( FbxAMatrix srcmat ){ m_globalpos = srcmat; };

	int GetRigidElemSize(){ return (int)m_rigidelem.size(); };
	void GetRigidElemMap( std::map<CBone*, CRigidElem*>& dstmap ){
		dstmap = m_rigidelem;
	};
	CRigidElem* GetRigidElem( CBone* srcbone ){ return m_rigidelem[ srcbone ]; };
	std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapBegin(){
		return m_rigidelem.begin();
	};
	std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapEnd(){
		return m_rigidelem.end();
	};
	void SetRigidElem( CBone* srcbone, CRigidElem* srcre ){ m_rigidelem[ srcbone ] = srcre; };

	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator FindRigidElemOfMap(std::string srcname){
		return m_remap.find(srcname);
	};
	int GetReMapSize(){ return (int)m_remap.size(); };
	int GetReMapSize2(std::string srcstring){
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstring ];
		return (int)curmap.size();
	};
	CRigidElem* GetRigidElemOfMap( std::string srcstr, CBone* srcbone ){
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstr ];
		return curmap[ srcbone ];
	};
	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapBegin(){
		return m_remap.begin();
	};
	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapEnd(){
		return m_remap.end();
	};

	void SetRigidElemOfMap( std::string srcstr, CBone* srcbone, CRigidElem* srcre ){
		((m_remap[ srcstr ])[ srcbone ]) = srcre;
	};

	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator FindImpMap(std::string srcname){
		return m_impmap.find(srcname);
	};
	int GetImpMapSize(){ return (int)m_impmap.size(); };
	int GetImpMapSize2(std::string srcstring){
		std::map<CBone*, D3DXVECTOR3> curmap = m_impmap[ srcstring ];
		return (int)curmap.size();
	};
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator GetImpMapBegin(){
		return m_impmap.begin();
	};
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator GetImpMapEnd(){
		return m_impmap.end();
	};
	D3DXVECTOR3 GetImpMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, D3DXVECTOR3> curmap = itrimpmap->second;
			std::map<CBone*, D3DXVECTOR3>::iterator itrimp;
			itrimp = curmap.find(srcbone);
			if (itrimp != curmap.end()){
				return itrimp->second;
			}
			else{
				//_ASSERT(0);
				return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			_ASSERT(0);
			return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		}
	};
	void SetImpMap2( std::string srcstring, std::map<CBone*,D3DXVECTOR3>& srcmap ){
		m_impmap[ srcstring ] = srcmap;
	};
	void SetImpOfMap( std::string srcstr, CBone* srcbone, D3DXVECTOR3 srcimp ){
		std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, D3DXVECTOR3>::iterator itrsetmap;
			itrsetmap = itrimpmap->second.find(srcbone);
			if (itrsetmap != itrimpmap->second.end()){
				itrsetmap->second = srcimp;
			}
			else{
				itrimpmap->second[srcbone] = srcimp;
			}
		}
		else{
			std::map<CBone*, D3DXVECTOR3> newmap;
			newmap[srcbone] = srcimp;
			m_impmap[srcstr] = newmap;
		}
		//((m_impmap[ srcstr ])[ srcbone ]) = srcimp;
	};

	int GetBtKinFlag(){ return m_btkinflag; };
	void SetBtKinFlag( int srcflag ){ m_btkinflag = srcflag; };

	int GetBtForce(){ return m_btforce; };
	void SetBtForce( int srcval ){ m_btforce = srcval; };

	int GetBtObjectSize(){ return (int)m_btobject.size(); };
	CBtObject* GetBtObject( CBone* srcbone ){ return m_btobject[ srcbone ]; };
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapBegin(){
		return m_btobject.begin();
	};
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapEnd(){
		return m_btobject.end();
	};

	void SetBtObject( CBone* srcbone, CBtObject* srcbo ){ m_btobject[ srcbone ] = srcbo; };
	void ClearBtObject(){ m_btobject.clear(); };


	int GetMotMarkSize(){ return (int)m_motmark.size(); };
	int GetMotMarkSize2(int srcindex){
		std::map<double, int> curmap = m_motmark[ srcindex ];
		return (int)curmap.size();
	};
	int GetMotMarkOfMap( int srcindex, double srcframe ){
		std::map<double,int> curmap = m_motmark[ srcindex ];
		return curmap[ srcframe ];
	};
	void GetMotMarkOfMap2( int srcindex, std::map<double, int>& dstmap ){
		dstmap = m_motmark[ srcindex ];
	};
	std::map<int, std::map<double, int>>::iterator GetMotMarkOfMapBegin(){
		return m_motmark.begin();
	};
	std::map<int, std::map<double, int>>::iterator GetMotMarkOfMapEnd(){
		return m_motmark.end();
	};
	std::map<int, std::map<double, int>>::iterator FindMotMarkOfMap( int srcindex ){
		return m_motmark.find( srcindex );
	};
	void SetMotMarkOfMap( int srcindex, double srcframe, int srcmark ){
		((m_motmark[ srcindex ])[ srcframe ]) = srcmark;
	};
	void SetMotMarkOfMap2( int srcindex, std::map<double, int>& srcmap ){
		m_motmark[ srcindex ] = srcmap;
	};
	void ClearMotMarkOfMap2( int srcindex ){
		m_motmark[ srcindex ].clear();
	};

	D3DXVECTOR3 GetFirstFrameBonePos()
	{
		return m_firstframebonepos;
	};


	CModel* GetParModel(){ return m_parmodel; };
	void SetParModel( CModel* srcpar ){ m_parmodel = srcpar; };

	CBone* GetParent(){ return m_parent; };
	void SetParent( CBone* srcpar ){ m_parent = srcpar; };

	CBone* GetChild(){ return m_child; };
	void SetChild( CBone* srcchil ){ m_child = srcchil; };

	CBone* GetBrother(){ return m_brother; };
	void SetBrother( CBone* srcbro ){ m_brother = srcbro; };

	bool GetPositionFound(){
		return m_posefoundflag;
	};
	void SetPositionFound(bool foundflag){
		m_posefoundflag = foundflag;
	};

	float GetBoneLeng(){
		CBone* parbone = GetParent();
		if (parbone){
			D3DXVECTOR3 bonevec = GetJointFPos() - parbone->GetJointFPos();
			float boneleng = D3DXVec3Length(&bonevec);
			return boneleng;
		}
		else{
			return 0.0f;
		}
	};

	D3DXMATRIX GetFirstAxisMatX()
	{
		CalcFirstAxisMatX();
		return m_firstaxismatX;
	};
	D3DXMATRIX GetFirstAxisMatZ()
	{
		CalcFirstAxisMatZ();
		return m_firstaxismatZ;
	};

	CMotionPoint* GetMotionPoint(int srcmotid, double srcframe){
		//���݂���Ƃ������Ԃ��B
		CMotionPoint* pbef = 0;
		CMotionPoint* pnext = 0;
		int existflag = 0;
		GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, &existflag);
		if (existflag == 1){
			return pbef;
		}
		else{
			return 0;
		}
	};

	ANGLELIMIT GetAngleLimit();
	void SetAngleLimit(ANGLELIMIT srclimit);

	int GetFreeCustomRigNo();
	CUSTOMRIG CBone::GetFreeCustomRig();
	CUSTOMRIG GetCustomRig(int rigno);
	void SetCustomRig(CUSTOMRIG srccr);

	D3DXMATRIX GetTmpSymMat()
	{
		return m_tmpsymmat;
	};
	void SetTmpSymMat(D3DXMATRIX srcmat)
	{
		m_tmpsymmat = srcmat;
	};
	D3DXVECTOR3 GetBtParPos(){
		return m_btparpos;
	};
	D3DXVECTOR3 GetBtChilPos(){
		return m_btchilpos;
	};
	D3DXMATRIX GetBtDiffMat(){
		return m_btdiffmat;
	};


	D3DXMATRIX GetBefBtMat(){ return m_befbtmat; };
	void SetBefBtMat(D3DXMATRIX srcmat){ m_befbtmat = srcmat; };
	D3DXMATRIX GetBtMat(){ return m_btmat; };
	void SetBtMat(D3DXMATRIX srcmat){
		//if (GetBtFlag() == 0){
			SetBefBtMat(m_btmat);
		//}
		m_btmat = srcmat;
	};

	int GetBtFlag(){ return m_setbtflag; };
	void SetBtFlag(int srcflag){ m_setbtflag = srcflag; };

	CModel* GetCurColDisp(CBone* childbone);
	void SetFirstCalcRigid(bool srcflag){
		m_firstcalcrigid = srcflag;
	};

	void SetPosConstraint(int srcval){
		m_posconstraint = srcval;
	};
	int GetPosConstraint(){
		return m_posconstraint;
	};

	void SetMass0(int srcval){
		m_mass0 = srcval;
	};
	int GetMass0(){
		return m_mass0;
	};

	void SetExcludeMv(int srcval){
		m_excludemv = srcval;
	};
	int GetExcludeMv(){
		return m_excludemv;
	};


private:
	int m_posconstraint;
	int m_mass0;
	int m_excludemv;
	bool m_firstcalcrigid;
	int m_type;
	int m_selectflag;
	int m_bonecnt;
	int m_validflag;
	int m_boneno;
	int m_topboneflag;
	char m_bonename[256];
	WCHAR m_wbonename[256];
	char m_engbonename[256];

	int m_upkind;//m_gaxismatXpar�v�Z����upvec

	bool m_posefoundflag;//BindPose�̒��ɂ��̃{�[���̈ʒu��񂪂������ꍇtrue�B

	D3DXVECTOR3 m_childworld;//�{�[���̎q���̃��[�V�����s��K�p��̍��W�B
	D3DXVECTOR3 m_childscreen;//�{�[���̎q����WVP�K�p��̍��W�B

	std::map<int, CMotionPoint*> m_motionkey;//m_motionkey[ ���[�V����ID ]�Ń��[�V�����̍ŏ��̃t���[���̎p���ɃA�N�Z�X�ł���B
	CMotionPoint m_curmp;//���݂�WVP�K�p��̎p���f�[�^�B
	CMotionPoint m_befmp;//���O�̎p���f�[�^�B

	CQuaternion m_axisq;//�{�[���̎��̃N�H�[�^�j�I���\���B

	D3DXMATRIX m_laxismat;//Z�{�[����axismat
	D3DXMATRIX m_gaxismatXpar;//X�{�[���̃O���[�o����axismat
	D3DXMATRIX m_gaxismatYpar;//Y�{�[���̃O���[�o����axismat
	D3DXMATRIX m_axismat_par;//X�{�[���̃��[�J����axismat
	D3DXMATRIX m_firstaxismatX;//������Ԃł�X�{�[���̃O���[�o��axismat
	D3DXMATRIX m_firstaxismatZ;//������Ԃł�Z�{�[���̃O���[�o��axismat


	D3DXMATRIX m_startmat2;//���[���h�s���ۑ����Ă����Ƃ���B���̃V�~�����[�V�������n�߂�ۂȂǂɕۑ�����B

	int m_getanimflag;//FBX�t�@�C����ǂݍ��ލۂɃA�j���[�V������ǂݍ��񂾂�P�B
	D3DXMATRIX m_nodemat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	D3DXMATRIX m_firstmat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	D3DXMATRIX m_invfirstmat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	D3DXVECTOR3 m_jointfpos;//�W���C���g�̏����ʒu�B
	D3DXVECTOR3 m_oldjointfpos;//�W���C���g�����ʒu�i���f�[�^�݊��j
	D3DXVECTOR3 m_jointwpos;//FBX�ɃA�j���[�V�����������Ă��Ȃ����̃W���C���g�̏����ʒu�B
	FbxAMatrix m_globalpos;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	D3DXMATRIX m_initmat;
	D3DXMATRIX m_invinitmat;
	D3DXMATRIX m_tmpmat;//�ꎞ�g�p�ړI
	CQuaternion m_tmpq;
	D3DXMATRIX m_tmpsymmat;


	D3DXMATRIX m_btmat;
	D3DXMATRIX m_befbtmat;
	int m_setbtflag;


	D3DXVECTOR3 m_firstframebonepos;

	ANGLELIMIT m_anglelimit;

	int m_initcustomrigflag;
	CUSTOMRIG m_customrig[MAXRIGNUM];

	//CBone*�͎q���W���C���g�̃|�C���^�B�q���̐������G���g���[������B
	std::map<CBone*, CRigidElem*> m_rigidelem;


	//m_remap�́Ajoint�̖��O�ŃZ�b�g�����map<string,CRigidElem*>�ōςށB
	//���O�ň�ӂ�RigidElem��I�����邽�߂Ɏq���̃{�[���̖��O���g�p����B
	//���̂��ߎq���̃{�[���̖��O��CRigidElem�̃Z�b�g���g���B
	//m_impmap�ɂ��Ă����l�̂��Ƃ�������B
	std::map<std::string, std::map<CBone*, CRigidElem*>> m_remap;//map<�ݒ�t�@�C����, map<�q���{�[��, ���̐ݒ�>>
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>> m_impmap;//map<�ݒ�t�@�C����, map<�q���{�[��, �C���p���X�ݒ�>>


	int m_btkinflag;//bullet kinematic flag�B���̃V�~�����[�V�����̍����̃{�[�����Œ胂�[�V�����ɒǐ�����ۂ͂O���w�肷��B���̑��͂P�B
	int m_btforce;//bullet �����t���O�B���f���ɐݒ肳��Ă���btkinflag�ł���Bm_btkinflag�̓V�~�����[�V�����J�n�ƏI���ŕω����邪�A���̃t���O�͓ǂݍ��ݎ��ɐݒ肳�����̂ł���B
	std::map<CBone*,CBtObject*> m_btobject;//CBtObject��bullet�̍��̃I�u�W�F�N�g�����b�v�����N���X�ł���B�{�[����CBtObject��map�B

	//�^�C�����C���̃��[�V�����ҏW�}�[�N
	//�ŏ���int��motid�B����map��frameno�ƍX�V�t���O�B�X�V�t���O�͓ǂݍ��ݎ��̃}�[�N�͂O�A����Ȍ�̕ҏW�}�[�N�͂P�ɂ���\��B�F��ς��邩���B
	std::map<int, std::map<double,int>> m_motmark;

	CModel* m_coldisp[COL_MAX];

	CModel* m_parmodel;

	CBone* m_parent;
	CBone* m_child;
	CBone* m_brother;
};

#endif