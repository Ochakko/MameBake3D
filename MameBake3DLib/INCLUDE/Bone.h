#ifndef BONEH
#define BONEH

//class ID3D11Device;


//#include <d3dx9.h>
//#include <quaternion.h>

#include <fbxsdk.h>

#include <map>
#include <string>

#include <Coef.h>

#include <orgwindow.h>

#include <ChaVecCalc.h>

#include <MotionPoint.h>


#define BONEPOOLBLKLEN	256


class CMQOFace;
class CMotionPoint;
class CRigidElem;
class CBtObject;
class CModel;

class CBone
{
public:

	ChaVector3 m_btparentpos;//Motion2Bt���̃{�[���̈ʒu(���̍s��v�Z�p)
	ChaVector3 m_btchildpos;//Motion2Bt���̃{�[���̈ʒu(���̍s��v�Z�p)
	ChaMatrix m_btdiffmat;//Motion2Bt����btmat�̕ω���(���̍s��v�Z�p)


	CBone() {
		InitializeCriticalSection(&m_CritSection_AddMP);
		InitializeCriticalSection(&m_CritSection_GetBefNext);
		InitParams();
	};

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
 * @param (ChaMatrix* wmat) IN ���[���h���W�n�̕ϊ��s����w�肷��B
 * @param (ChaMatrix* vpmat) IN �J�����v���W�F�N�V�������W�n�̕ϊ��s����w�肷��B
 * @return ����������O�B
 * @detail �w�胂�[�V�����̎w�莞�Ԃ̎p�����v�Z����B�O���[�o���Ȏp���̌v�Z�ł���B
 */
	int UpdateMatrix( int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat );

	int UpdateMatrixFromEul(int srcmotid, double srcframe, ChaVector3 neweul, ChaMatrix* wmat, ChaMatrix* vpmat);

	int UpdateLimitedWorldMat(int srcmotid, double srcframe);

	
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
	int GetCalclatedLimitedWM(int srcmotid, double srcframe, ChaMatrix* plimitedworldmat, CMotionPoint** pporgbefmp = 0);

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
	int GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr, bool onaddmotion = false );

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
 * @param (CBone* childbone) IN�@���̂��w�肷�邽�߂̎q���{�[���B
 * @param (int setstartflag) IN�@���̃V�~�����[�V�����J�n���̌Ăяo�����ɂP���Z�b�g����B
 * @return ����������O�B
 * @detail ���̂��{�[���̈ʒu�ɕ\�����邽�߂ɁA���̕\���p�̌`����X�P�[�����邽�߂ɌĂԁB���̂̓{�[���̎q���W���C���g�ƂP�΂P�őΉ����邽�߁A�w���childbone���g���B
 */
	int CalcRigidElemParams( CBone* childbone, int setstartflag );

/**
 * @fn
 * CalcAxisMat
 * @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B������Ԃ�X���������Ă���Ɖ��肵�Čv�Z����B
 * @param (int firstflag) IN �ϊ��J�n�̍s���ۑ��������Ƃ��ɂP���w�肷��B
 * @param (float delta) IN ��]�p�x�����Ă�����B���݂͂O�ł����g�p���Ă��Ȃ��B
 * @return ����������O�B
 * @detail �v�Z�����f�[�^�́A�{�[���̈ʒu�Ƀ}�j�s�����[�^��\�����邽�߂̕ϊ��s��Ɏg�p����B���݂�CalcAxisMatZ�֐��Ń{�[���̕ϊ��s����v�Z���Ă���B
 */
	//int CalcAxisMat( int firstflag, float delta );
	float CalcAxisMatX(int bindflag, CBone* childbone, ChaMatrix* dstmat, int setstartflag);


/**
 * @fn
 * CalcAxisMatZ
 * @breaf �{�[���̐e�Ǝq���̈ʒu���w�肵�āA���̎��̕ϊ��s����v�Z����B
 * @param (ChaVector3* srccur) IN �{�[���̐e�̍��W���w�肷��B
 * @param (ChaVector3* srcchil) IN �{�[���̎q���̍��W���w�肷��B
 * @return ����������O�B
 * @detail ������Ԃ�Z���������Ă���Ɖ��肵�Čv�Z����B�{�[���̈ʒu�Ƀ}�j�s�����[�^��\�����邽�߂Ɏg�p����B
 */
	int CalcAxisMatZ( ChaVector3* srccur, ChaVector3* srcchil );


/**
 * @fn
 * CreateRigidElem
 * @breaf ���̃V�~�����[�V�����p�̃p�����[�^��ێ�����CRigidElem���쐬����B�w��{�[���Ɋւ���CRigidElem���쐬����B
 * @param (CBone* parentbone) IN �������g�Ƃ����Ŏw�肷��e�W���C���g�Œ�`�����{�[���Ɋւ���CRigidElem���쐬����B
 * @param (int reflag) IN CRigidElem���쐬����ꍇ�ɂP�A���Ȃ��ꍇ�ɂO���w��B
 * @param (std::string rename) IN reflag���P�̂Ƃ��Aref�t�@�C���̃t�@�C�������w�肷��B
 * @param (int impflag) IN �C���p���X��^���邽�߂̐ݒ���쐬����ꍇ�ɂP�A���Ȃ��ꍇ�ɂO���w��B
 * @param (std::string impname) IN impflag���P�̂Ƃ��Aimp�t�@�C���̃t�@�C�������w�肷��B
 * @return ����������O�B
 * @detail �C���p���X�ݒ�f�[�^���쐬����B
 */
	//int CreateRigidElem( CBone* chil, int reflag, std::string rename, int impflag, std::string impname );
	int CreateRigidElem(CBone* parentbone, int reflag, std::string rename, int impflag, std::string impname);


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
 * @param (ChaVector3 srctra) IN �ړ��x�N�g�����w�肷��B
 * @return �v�Z�����p�����i�[����CMotionPoint�̃|�C���^��Ԃ����ċA�֐��ł��邱�Ƃɒ��ӁB�|�C���^�̓`�F�C���ɃZ�b�g���ꂽ���̂ł���B
 * @detail �z�肵�Ă���g�����Ƃ��ẮA�O������̌Ăяo�����ɂ�parmp���O�ɂ���B���̊֐����ł̍ċA�Ăяo������parmp�ɐe���Z�b�g����B
 */
	CMotionPoint* AddBoneTraReq(CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srctra);


	CMotionPoint* AddBoneScaleReq(CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srcscale);

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
	CMotionPoint* RotBoneQReq(bool infooutflag, CBone* parentbone, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, ChaVector3 traanim = ChaVector3(0.0f, 0.0f, 0.0f), int setmatflag = 0, ChaMatrix* psetmat = 0);

	//CMotionPoint* RotBoneQCurrentReq(bool infooutflag, CBone* parbone, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, ChaVector3 traanim = ChaVector3(0.0f, 0.0f, 0.0f), int setmatflag = 0, ChaMatrix* psetmat = 0);

/**

*/
	CMotionPoint* RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, ChaMatrix srcmat);

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
	int CalcCurrentLocalInfo(CMotionPoint* pdstmp);
	int CalcBtLocalInfo(CMotionPoint* pdstmp);

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
	* @param (ChaVector3 curpos) IN �{�[���̈ʒu�B
	* @param (ChaVector3 childpos) IN �q���̃{�[���̈ʒu�B
	* @param (ChaMatrix* destmat) OUT �ϊ��s����i�[����f�[�^�ւ̃|�C���^�B
	* @return ����������O�B
	* @detail CalcAxisMatX����Ă΂��B
	*/
	//int CalcAxisMatX_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* destmat);

	int CalcAxisMatZ_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* destmat);


	int CalcFirstFrameBonePos(ChaMatrix srcmat);

	int CalcBoneDepth();

	ChaVector3 CalcLocalEulXYZ(int axiskind, int srcmotid, double srcframe, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)
	ChaVector3 CalcCurrentLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);
	ChaVector3 CalcBtLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);

	ChaMatrix CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe);
	//ChaMatrix CalcManipulatorPostureMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe);
	ChaMatrix CalcManipulatorPostureMatrix(int calccapsuleflag, int anglelimitaxisflag, int settraflag, int multworld, int calczeroframe);
	int SetWorldMatFromEul(int inittraflag, int setchildflag, ChaVector3 srceul, int srcmotid, double srcframe, int initscaleflag = 0);
	int SetBtWorldMatFromEul(int setchildflag, ChaVector3 srceul);
	ChaMatrix CalcWorldMatFromEul(int inittraflag, int setchildflag, ChaVector3 srceul, ChaVector3 befeul, int srcmotid, double srcframe, int initscaleflag);//initscaleflag = 1 : default
	int SetWorldMatFromEulAndScaleAndTra(int inittraflag, int setchildflag, ChaVector3 srceul, ChaVector3 srcscale, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetWorldMatFromEulAndTra(int setchildflag, ChaVector3 srceul, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetWorldMatFromQAndTra(int setchildflag, CQuaternion axisq, CQuaternion srcq, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetLocalEul(int srcmotid, double srcframe, ChaVector3 srceul);
	ChaVector3 GetLocalEul(int srcmotid, double srcframe);
	int SetWorldMat(bool infooutflag, int setchildflag, int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck = 0);
	ChaMatrix GetWorldMat(int srcmotid, double srcframe);
	ChaVector3 CalcLocalTraAnim(int srcmotid, double srcframe);
	ChaVector3 CalcLocalScaleAnim(int srcmotid, double srcframe);
	ChaMatrix CalcLocalScaleRotMat(int rotcenterflag, int srcmotid, double srcframe);
	ChaMatrix CalcLocalSymScaleRotMat(int rotcenterflag, int srcmotid, double srcframe);
	ChaVector3 CalcLocalSymScaleVec(int srcmotid, double srcframe);
	ChaVector3 CalcLocalSymTraAnim(int srcmotid, double srcframe);
	ChaMatrix CalcSymXMat(int srcmotid, double srcframe);
	ChaMatrix CalcSymXMat2(int srcmotid, double srcframe, int symrootmode);
	int PasteMotionPoint(int srcmotid, double srcframe, CMotionPoint srcmp);

	ChaVector3 CalcFBXEulXYZ(int srcnotmodifyflag, int srcmotid, double srcframe, ChaVector3* befeulptr = 0);
	//ChaVector3 CalcFBXEulZXY(int srcmotid, double srcframe, ChaVector3* befeulptr = 0);
	ChaVector3 CalcFBXTra(int srcmotid, double srcframe);
	int QuaternionInOrder(int srcmotid, double srcframe, CQuaternion* srcdstq);
	int CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos);

	int LoadCapsuleShape(ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext);

	int ChkMovableEul(ChaVector3 srceul);


	int SetCurrentMotion(int srcmotid, double animleng);
	void ResetMotionCache();

	ChaMatrix GetCurrentZeroFrameMat(int updateflag);//current motion��frame 0��worldmat
	ChaMatrix GetCurrentZeroFrameInvMat(int updateflag);


	static CBone* GetNewBone(CModel* parmodel);
	static void InvalidateBone(CBone* srcmp);
	static void InitBones();
	static void DestroyBones();
	static void OnDelModel(CModel* parmodel);

	ChaMatrix CalcParentGlobalMat(int srcmotid, double srcframe);

	void InitAddLimitQAll();
	void RotQAddLimitQAll(int srcmotid, double srcframe);


	int CreateIndexedMotionPoint(int srcmotid, double animleng);

	int ResizeIndexedMotionPoint(int srcmotid, double animleng);
	void ResizeIndexedMotionPointReq(int srcmotid, double animleng);

	ChaVector3 LimitEul(ChaVector3 srceul);
	void SetBefWorldMatReq(int srcmotid, double srcframe);

	int ResetAngleLimit(int srcval);
	int AngleLimitReplace180to170();
	int AdditiveCurrentToAngleLimit();
	int AdditiveToAngleLimit(ChaVector3 cureul);

private:

/**
 * @fn
 * InitParams
 * @breaf �N���X�����o�̏�����������B
 * @return �O�B
 */
	int InitParams();
	int InitParamsForReUse(CModel* srcparmodel);

	int SetParams(CModel* parmodel);//�R���X�g���N�^��InitParams�ł͑���Ȃ�����

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
	//int CalcAxisMatX();


	int CalcAxisMatZ();


/**
 * @fn
 * CalcAxisMatY
 * @breaf �{�[���̎��̂��߂̕ϊ��s����v�Z����B������Ԃ�Z���������Ă���Ɖ��肵�Čv�Z����B
 * @param (CBone* childbone) IN �q���̃{�[���B
 * @param (ChaMatrix* dstmat) OUT �ϊ��s����i�[����f�[�^�ւ̃|�C���^�B
 * @return ����������O�B
 */
	int CalcAxisMatY( CBone* childbone, ChaMatrix* dstmat );

/**
 * @fn
 * CalcLocalAxisMat
 * @breaf ���[�J���ȃ{�[�����̕ϊ��s����v�Z����B
 * @param (ChaMatrix motmat) IN �O���[�o���Ȏp���s��B
 * @param (ChaMatrix axismatpar) IN �O���[�o���ȃ{�[�����ϊ��s��BX����B
 * @param (ChaMatrix gaxisy) IN �O���[�o���ȃ{�[�����ϊ��s��BY����B
 * @return ����������O�B
 * @detail �v�Z���ʂ�GetAxisMatPar()�Ŏ擾����B
 */
	int CalcLocalAxisMat( ChaMatrix motmat, ChaMatrix axismatpar, ChaMatrix gaxisy );

/**
 * @fn
 * SetGroupNoByName
 * @breaf ���̂̂����蔻��p�̃O���[�v�ԍ���ݒ肷��B�{�[���̖��O���画�肵�Đݒ肷��B
 * @param (CRigidElem* curre) IN ���̂�CRigidElem�ւ̃|�C���^�B
 * @param (CBone* childbone) IN �{�[����CBone�ւ̃|�C���^�B
 * @return ����������O�B
 * @detail BT_�����O�ɂ����̂ɂ����̗p�̃O���[�v�ԍ����^������B����ȊO�̃O���[�v�ԍ��́Aname_G_*** �`���̖��O�Ŏw��ł���B
 */
	int SetGroupNoByName( CRigidElem* curre, CBone* childbone );

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
	//void CalcFirstAxisMatX();
	void CalcFirstAxisMatZ();

	void InitAngleLimit();
	float LimitAngle(enum tag_axiskind srckind, float srcval);
	int InitCustomRig();
	void CalcBtRootDiffMatFunc(CBone* srcbone);

	ChaMatrix GetCurrentZeroFrameMatFunc(int updateflag, int inverseflag);

	void CalcParentGlobalMatReq(ChaMatrix* dstmat, CBone* srcbone, int srcmotid, double srcframe);


	void InitAddLimitQReq(CBone* srcbone);
	void RotQAddLimitQReq(CBone* srcbone, int srcmotid, double srcframe);


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

	ChaVector3 GetChildWorld();
	void SetChildWorld( ChaVector3 srcvec ){ m_childworld = srcvec; };

	ChaVector3 GetChildScreen(){ return m_childscreen; };
	void SetChildScreen( ChaVector3 srcvec ){ m_childscreen = srcvec; };

	int GetMotionKeySize(){ return (int)m_motionkey.size(); };
	CMotionPoint* GetMotionKey( int srccookie ){
		if ((srccookie >= 1) && (srccookie <= m_motionkey.size())) {
			return m_motionkey[srccookie - 1];
		}
		else {
			//_ASSERT(0);//OnAddMotion�̂Ƃ��ɂ͗L�蓾��
			return 0;
		}
	};
	void SetMotionKey( int srccookie, CMotionPoint* srcmk ){ 
		if ((srccookie >= 1) && (srccookie <= m_motionkey.size())) {
			m_motionkey[srccookie - 1] = srcmk;
		}
		else {
			_ASSERT(0);
		}
	};

	CMotionPoint GetCurMp(){ return m_curmp; };
	void SetCurMp( CMotionPoint srcmp ){ m_curmp = srcmp; };

	//CMotionPoint GetBefMp(){ return m_befmp; };
	//void SetBefMp(CMotionPoint srcmp){ m_befmp = srcmp; };


	CQuaternion GetAxisQ(){ return m_axisq; };
	void SetAxisQ( CQuaternion srcq ){ m_axisq = srcq; };

	ChaMatrix GetLAxisMat(){ return m_laxismat; };
	ChaMatrix GetAxisMatPar(){ return m_axismat_par; };
	ChaMatrix GetInvAxisMatPar(){
		ChaMatrix invaxis;
		ChaMatrixInverse(&invaxis, NULL, &m_axismat_par);
		return invaxis;
	};

	ChaMatrix GetStartMat2(){ return m_startmat2; };
	ChaMatrix GetInvStartMat2(){
		ChaMatrix retmat;
		ChaMatrixInverse(&retmat, NULL, &m_startmat2);
		return retmat;
	};
	void SetStartMat2(ChaMatrix srcmat){ m_startmat2 = srcmat; };

	int GetGetAnimFlag(){ return m_getanimflag; };
	void SetGetAnimFlag( int srcflag ){ m_getanimflag = srcflag; };

	ChaMatrix GetNodeMat(){ return m_nodemat; };
	ChaMatrix GetInvNodeMat(){
		ChaMatrix retmat;
		ChaMatrixInverse(&retmat, NULL, &m_nodemat);
		return retmat;
	};
	void SetNodeMat( ChaMatrix srcmat ){ m_nodemat = srcmat; };


	ChaMatrix GetFirstMat(){ return m_firstmat; };
	void SetFirstMat( ChaMatrix srcmat ){ m_firstmat = srcmat; };

	ChaMatrix GetInvFirstMat(){ ChaMatrixInverse(&m_invfirstmat, NULL, &m_firstmat); return m_invfirstmat; };
	void SetInvFirstMat( ChaMatrix srcmat ){ m_invfirstmat = srcmat; };

	ChaMatrix GetInitMat(){ return m_initmat; };
	void SetInitMat(ChaMatrix srcmat){ m_initmat = srcmat; };

	ChaMatrix GetInvInitMat(){ ChaMatrixInverse(&m_invinitmat, NULL, &m_initmat); return m_invinitmat; };
	void SetInvInitMat(ChaMatrix srcmat){ m_invinitmat = srcmat; };

	ChaMatrix GetTmpMat(){ return m_tmpmat; };
	void SetTmpMat(ChaMatrix srcmat){ m_tmpmat = srcmat; };

	CQuaternion GetTmpQ(){ return m_tmpq; };
	void SetTmpQ(CQuaternion srcq){ m_tmpq = srcq; };


	ChaVector3 GetJointFPos();
	void SetJointFPos(ChaVector3 srcpos);
	void SetOldJointFPos(ChaVector3 srcpos);

	ChaVector3 GetJointWPos(){ return m_jointwpos; };
	void SetJointWPos( ChaVector3 srcpos ){ m_jointwpos = srcpos; };

	FbxAMatrix GetGlobalPosMat(){ return m_globalpos; };
	void SetGlobalPosMat( FbxAMatrix srcmat ){ m_globalpos = srcmat; };

	int GetRigidElemSize(){
		std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
		itrremap = m_remap.find(m_rigidelemname);
		if (itrremap != m_remap.end()){
			return (int)itrremap->second.size();
		}
		else{
			return 0;
		}
	};
		
	CRigidElem* GetRigidElem( CBone* srcbone ){ 
		return GetRigidElemOfMap(m_rigidelemname, srcbone);
	};
	//std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapBegin(){
	//	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
	//	itrremap = m_remap.find(m_rigidelemname);
	//	if (itrremap != m_remap.end()){
	//		return itrremap->second.begin();
	//	}
	//	else{
	//		return itrremap->second.end();
	//	}
	//};
	//std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapEnd(){
	//	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
	//	itrremap = m_remap.find(m_rigidelemname);
	//	return itrremap->second.end();
	//};
	void SetRigidElem( CBone* srcbone, CRigidElem* srcre ){ 
		SetRigidElemOfMap(m_rigidelemname, srcbone, srcre);
	};

	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator FindRigidElemOfMap(std::string srcname){
	//	return m_remap.find(srcname);
	//};
	int GetReMapSize(){ return (int)m_remap.size(); };
	int GetReMapSize2(std::string srcstring){
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstring ];
		return (int)curmap.size();
	};
	CRigidElem* GetRigidElemOfMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
		itrremap = m_remap.find(srcstr);
		if (itrremap != m_remap.end()){
			//itrremap->second[srcbone];
			std::map<CBone*, CRigidElem*>::iterator itrmap2;
			itrmap2 = itrremap->second.find(srcbone);
			if (itrmap2 != itrremap->second.end()){
				return itrmap2->second;
			}
			else{
				return 0;
			}
		}
		else{
			return 0;
		}
	};
	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapBegin(){
	//	return m_remap.begin();
	//};
	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapEnd(){
	//	return m_remap.end();
	//};

	void SetRigidElemOfMap(std::string srcstr, CBone* srcbone, CRigidElem* srcre);

	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator FindImpMap(std::string srcname){
		return m_impmap.find(srcname);
	};
	int GetImpMapSize(){ return (int)m_impmap.size(); };
	int GetImpMapSize2(std::string srcstring){
		std::map<CBone*, ChaVector3> curmap = m_impmap[ srcstring ];
		return (int)curmap.size();
	};
	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator GetImpMapBegin(){
		return m_impmap.begin();
	};
	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator GetImpMapEnd(){
		return m_impmap.end();
	};
	ChaVector3 GetImpMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, ChaVector3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, ChaVector3> curmap = itrimpmap->second;
			std::map<CBone*, ChaVector3>::iterator itrimp;
			itrimp = curmap.find(srcbone);
			if (itrimp != curmap.end()){
				return itrimp->second;
			}
			else{
				//_ASSERT(0);
				return ChaVector3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			_ASSERT(0);
			return ChaVector3(0.0f, 0.0f, 0.0f);
		}
	};
	void SetImpMap2( std::string srcstring, std::map<CBone*,ChaVector3>& srcmap ){
		m_impmap[ srcstring ] = srcmap;
	};
	void SetImpOfMap( std::string srcstr, CBone* srcbone, ChaVector3 srcimp ){
		std::map<std::string, std::map<CBone*, ChaVector3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, ChaVector3>::iterator itrsetmap;
			itrsetmap = itrimpmap->second.find(srcbone);
			if (itrsetmap != itrimpmap->second.end()){
				itrsetmap->second = srcimp;
			}
			else{
				itrimpmap->second[srcbone] = srcimp;
			}
		}
		else{
			std::map<CBone*, ChaVector3> newmap;
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

	ChaVector3 GetFirstFrameBonePos()
	{
		return m_firstframebonepos;
	};

	ChaVector3 GetWorldPos(int srcmotid, double srcframe);
	ChaMatrix GetLimitedWorldMat(int srcmotid, double srcframe, ChaVector3* dstneweul = 0);
	ChaMatrix GetCurrentLimitedWorldMat();


	CModel* GetParModel(){ return m_parmodel; };
	//void SetParModel( CModel* srcpar ){ m_parmodel = srcpar; };//parmodel���Ƃ�m_boneno�ɒ��ӁI�I�I

	CBone* GetParent(){ return m_parent; };
	void SetParent( CBone* srcpar ){ m_parent = srcpar; };

	CBone* GetChild(){ return m_child; };
	void SetChild( CBone* srcchil ){ m_child = srcchil; };

	CBone* GetBrother(){ return m_brother; };
	void SetBrother( CBone* srcbro ){ m_brother = srcbro; };
	CBone* GetSister() {
		CBone* parbone = GetParent();
		if (parbone) {
			CBone* firstbrobone = parbone->GetChild();
			CBone* nextbone = firstbrobone;
			while (nextbone) {
				if (nextbone) {
					if (nextbone->GetBrother() == this) {
						return nextbone;//!!!!!!!!!!!!
					}
					nextbone = nextbone->GetBrother();
				}
				else {
					return 0;
				}
			}
			return 0;
		}
		else {
			return 0;
		}
		return 0;
	};
	bool IsBranchBone()
	{
		CBone* chksister = GetSister();
		CBone* chkbrother = GetBrother();
		if (chksister || chkbrother) {
			return true;
		}
		else {
			return false;
		}
	}
	CBone* GetUpperBranchBone() {
		CBone* curbone = GetParent();
		while (curbone) {
			if (curbone->IsBranchBone()) {
				return curbone;
			}
			curbone = curbone->GetParent();
		}
		return 0;
	};
	CBone* GetLowerBranchBone() {
		CBone* curbone = GetChild();
		while (curbone) {
			if (curbone->IsBranchBone()) {
				return curbone;
			}
			curbone = curbone->GetChild();
		}
		return 0;
	}
	

	bool GetPositionFound(){
		return m_posefoundflag;
	};
	void SetPositionFound(bool foundflag){
		m_posefoundflag = foundflag;
	};

	double GetBoneLeng(){
		CBone* parentbone = GetParent();
		if (parentbone){
			ChaVector3 bonevec = GetJointFPos() - parentbone->GetJointFPos();
			double boneleng = ChaVector3LengthDbl(&bonevec);
			return boneleng;
		}
		else{
			return 0.0;
		}
	};
	/*
	ChaMatrix GetFirstAxisMatX()
	{
		CalcFirstAxisMatX();
		return m_firstaxismatX;
	};
	*/
	ChaMatrix GetFirstAxisMatZ()
	{
		CalcFirstAxisMatZ();
		return m_firstaxismatZ;
	};

	CMotionPoint* GetMotionPoint(int srcmotid, double srcframe, bool onaddmotion = false){
		//���݂���Ƃ������Ԃ��B
		CMotionPoint* pbef = 0;
		CMotionPoint* pnext = 0;
		int existflag = 0;
		GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, &existflag, onaddmotion);
		if (existflag == 1){
			return pbef;
		}
		else{
			return 0;
		}
	};

	ANGLELIMIT GetAngleLimit(int getchkflag);
	void SetAngleLimit(ANGLELIMIT srclimit);

	int GetFreeCustomRigNo();
	CUSTOMRIG CBone::GetFreeCustomRig();
	CUSTOMRIG GetCustomRig(int rigno);
	void SetCustomRig(CUSTOMRIG srccr);

	ChaMatrix GetTmpSymMat()
	{
		return m_tmpsymmat;
	};
	void SetTmpSymMat(ChaMatrix srcmat)
	{
		m_tmpsymmat = srcmat;
	};
	ChaVector3 GetBtparentpos(){
		return m_btparentpos;
	};
	ChaVector3 GetBtchildpos(){
		return m_btchildpos;
	};
	ChaMatrix GetBtDiffMat(){
		return m_btdiffmat;
	};


	ChaMatrix GetBefBtMat(){ return m_befbtmat; };
	void SetBefBtMat(ChaMatrix srcmat){ m_befbtmat = srcmat; };
	ChaMatrix GetBtMat(){ return m_btmat; };
	ChaMatrix GetInvBtMat(){ ChaMatrix retmat; ChaMatrixInverse(&retmat, NULL, &m_btmat); return retmat; };
	void SetBtMat(ChaMatrix srcmat){
		//if (GetBtFlag() == 0){
			SetBefBtMat(m_btmat);
		//}
		m_btmat = srcmat;
	};

	int GetBtFlag(){ return m_setbtflag; };
	void SetBtFlag(int srcflag){ m_setbtflag = srcflag; };

	CModel* GetCurColDisp(CBone* childbone);
	CModel* GetColDisp(CBone* childbone, int srcindex);
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
	void SetCurMotID(int srcmotid);
	int GetCurMotID()
	{
		return m_curmotid;
	};
	void SetTmpKinematic(bool srcflag)
	{
		m_tmpkinematic = srcflag;
	};
	bool GetTmpKinematic()
	{
		return m_tmpkinematic;
	};
	

	int GetUseFlag()
	{
		return m_useflag;
	};
	void SetUseFlag(int srcflag)
	{
		m_useflag = srcflag;
	};
	int GetIndexOfPool()
	{
		return m_indexofpool;
	};
	void SetIndexOfPool(int srcindex)
	{
		m_indexofpool = srcindex;
	};
	int IsAllocHead()
	{
		return m_allocheadflag;
	};
	void SetIsAllocHead(int srcflag)
	{
		m_allocheadflag = srcflag;
	};


	void SetExtendFlag(bool srcflag) {
		m_extendflag = srcflag;
	};
	bool GetExtendFlag() {
		return m_extendflag;
	};

	void InitAddLimitQ()
	{
		m_addlimitq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}
	void Add2AddLimitQ(CQuaternion srcq);
	CQuaternion GetAddLimitQ()
	{
		return m_addlimitq;
	}

private:
	CRITICAL_SECTION m_CritSection_GetBefNext;
	CRITICAL_SECTION m_CritSection_AddMP;

	int m_useflag;//0: not use, 1: in use
	int m_indexofpool;//index of pool vector
	int m_allocheadflag;//1: head pointer at allocated


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
	int m_curmotid;

	int m_upkind;//m_gaxismatXpar�v�Z����upvec

	bool m_posefoundflag;//BindPose�̒��ɂ��̃{�[���̈ʒu��񂪂������ꍇtrue�B

	ChaVector3 m_childworld;//�{�[���̎q���̃��[�V�����s��K�p��̍��W�B
	ChaVector3 m_childscreen;//�{�[���̎q����WVP�K�p��̍��W�B




	std::map<int, CMotionPoint*> m_motionkey;//m_motionkey[ ���[�V����ID ]�Ń��[�V�����̍ŏ��̃t���[���̎p���ɃA�N�Z�X�ł���B
	CMotionPoint m_curmp;//���݂�WVP�K�p��̎p���f�[�^�B
	CMotionPoint m_befmp;//���O�̎p���f�[�^�B
	CMotionPoint* m_cachebefmp[MAXMOTIONNUM + 1];//motid���Ƃ̃L���b�V��

	//std::vector<CMotionPoint*> m_indexedmp;
	std::map<int, std::vector<CMotionPoint*>> m_indexedmotionpoint;
	std::map<int, bool> m_initindexedmotionpoint;
	CMotionPoint m_dummymp;

	CQuaternion m_axisq;//�{�[���̎��̃N�H�[�^�j�I���\���B

	ChaMatrix m_laxismat;//Z�{�[����axismat
	//ChaMatrix m_gaxismatXpar;//X�{�[���̃O���[�o����axismat
	ChaMatrix m_gaxismatYpar;//Y�{�[���̃O���[�o����axismat
	ChaMatrix m_axismat_par;//X�{�[���̃��[�J����axismat
	//ChaMatrix m_firstaxismatX;//������Ԃł�X�{�[���̃O���[�o��axismat
	ChaMatrix m_firstaxismatZ;//������Ԃł�Z�{�[���̃O���[�o��axismat


	ChaMatrix m_startmat2;//���[���h�s���ۑ����Ă����Ƃ���B���̃V�~�����[�V�������n�߂�ۂȂǂɕۑ�����B

	int m_getanimflag;//FBX�t�@�C����ǂݍ��ލۂɃA�j���[�V������ǂݍ��񂾂�P�B
	ChaMatrix m_nodemat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	ChaMatrix m_firstmat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	ChaMatrix m_invfirstmat;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	ChaVector3 m_jointfpos;//�W���C���g�̏����ʒu�B
	ChaVector3 m_oldjointfpos;//�W���C���g�����ʒu�i���f�[�^�݊��j
	ChaVector3 m_jointwpos;//FBX�ɃA�j���[�V�����������Ă��Ȃ����̃W���C���g�̏����ʒu�B
	FbxAMatrix m_globalpos;//�W���C���g�̏����ʒu���v�Z����Ƃ��Ɏg�p����BFBX�ǂݍ��ݎ��ɃZ�b�g���Ďg�p����B
	ChaMatrix m_initmat;
	ChaMatrix m_invinitmat;
	ChaMatrix m_tmpmat;//�ꎞ�g�p�ړI
	CQuaternion m_tmpq;
	ChaMatrix m_tmpsymmat;

	CQuaternion m_addlimitq;

	ChaMatrix m_btmat;
	ChaMatrix m_befbtmat;
	int m_setbtflag;


	ChaVector3 m_firstframebonepos;

	ANGLELIMIT m_anglelimit;

	int m_initcustomrigflag;
	CUSTOMRIG m_customrig[MAXRIGNUM];

	//CBone*�͎q���W���C���g�̃|�C���^�B�q���̐������G���g���[������B
	//std::map<CBone*, CRigidElem*> m_rigidelem;
	std::string m_rigidelemname;


	//m_remap�́Ajoint�̖��O�ŃZ�b�g�����map<string,CRigidElem*>�ōςށB
	//���O�ň�ӂ�RigidElem��I�����邽�߂Ɏq���̃{�[���̖��O���g�p����B
	//���̂��ߎq���̃{�[���̖��O��CRigidElem�̃Z�b�g���g���B
	//m_impmap�ɂ��Ă����l�̂��Ƃ�������B
	std::map<std::string, std::map<CBone*, CRigidElem*>> m_remap;//map<�ݒ�t�@�C����, map<�q���{�[��, ���̐ݒ�>>
	std::map<std::string, std::map<CBone*, ChaVector3>> m_impmap;//map<�ݒ�t�@�C����, map<�q���{�[��, �C���p���X�ݒ�>>


	int m_btkinflag;//bullet kinematic flag�B���̃V�~�����[�V�����̍����̃{�[�����Œ胂�[�V�����ɒǐ�����ۂ͂O���w�肷��B���̑��͂P�B
	int m_btforce;//bullet �����t���O�B���f���ɐݒ肳��Ă���btkinflag�ł���Bm_btkinflag�̓V�~�����[�V�����J�n�ƏI���ŕω����邪�A���̃t���O�͓ǂݍ��ݎ��ɐݒ肳�����̂ł���B
	std::map<CBone*,CBtObject*> m_btobject;//CBtObject��bullet�̍��̃I�u�W�F�N�g�����b�v�����N���X�ł���B�{�[����CBtObject��map�B

	//�^�C�����C���̃��[�V�����ҏW�}�[�N
	//�ŏ���int��motid�B����map��frameno�ƍX�V�t���O�B�X�V�t���O�͓ǂݍ��ݎ��̃}�[�N�͂O�A����Ȍ�̕ҏW�}�[�N�͂P�ɂ���\��B�F��ς��邩���B
	std::map<int, std::map<double,int>> m_motmark;

	CModel* m_coldisp[COL_MAX];

	CModel* m_parmodel;

	int m_firstgetflag;//GetCurrentZeroFrameMat�p
	ChaMatrix m_firstgetmatrix;//GetCurrentZeroFrameMat�p
	ChaMatrix m_invfirstgetmatrix;//GetCurrentZeroFrameMat�p

	bool m_tmpkinematic;

	bool m_extendflag;

	double m_befupdatetime;


	CBone* m_parent;
	CBone* m_child;
	CBone* m_brother;
};

#endif