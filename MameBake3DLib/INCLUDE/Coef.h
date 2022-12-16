#ifndef COEFH
#define		COEFH

#include <Windows.h>

//#include <d3dcommon.h>
//#include <dxgi.h>
//#include <d3d10_1.h>
//#include <d3d10.h>
//#include <d3dcompiler.h>
//#include <d3dx10.h>


#include <usercoef.h>

#include <vector>
#include <map>
#include <string>

class CModel;
class CMQOMaterial;

#define ALIGNED		_declspec(align(16))
#define	u_long	unsigned long

#define EDITRANGEHISTORYNUM	10000

#define COLIGROUPNUM	10

#define PATH_LENG	2048
#define MAXMOTIONNUM	100
#define MAXMODELNUM		100
#define MAXRENUM		100

// * 100����OpenFile()�֐��̃X�^�b�N�T�C�Y���傫�����Čx�����o��̂� * 32�ɕύX
//#define MULTIPATH	(MAX_PATH * 100)
#define MULTIPATH	(MAX_PATH * 32)

#define COL_CONE_INDEX		0
#define COL_CAPSULE_INDEX	1
#define COL_SPHERE_INDEX	2
#define COL_BOX_INDEX		3
#define COL_MAX				4

#define CGP_GROUND	1
#define CGP_CHARA	2
#define COLOF_G		CGP_CHARA
#define COLOF_CHARA_0	(CGP_GROUND | CGP_CHARA)
#define COLOF_CHARA_1	CGP_CHARA

#define UNDOMAX 999

//bone���Ƃ�RIG�z��
#define MAXRIGNUM	10
#define MAXRIGELEMNUM	5


enum
{
	//CQuaternion::Q2EulXYZusingMat()

	ROTORDER_XYZ,
	ROTORDER_YZX,
	ROTORDER_ZXY,
	ROTORDER_XZY,
	ROTORDER_YXZ,
	ROTORDER_ZYX,
	ROTORDER_MAX
};

enum
{
	BGCOL_BLACK,
	BGCOL_WHITE,
	BGCOL_BLUE,
	BGCOL_GREEN,
	BGCOL_RED,
	BGCOL_GRAY,
	BGCOL_MAX
};


enum
{
	INITMP_ROTTRA,
	INITMP_ROT,
	INITMP_TRA,
	INITMP_SCALE,
	INITMP_MAX
};

enum
{
	//for bit mask operation
	SYMROOTBONE_SAMEORG = 0,
	SYMROOTBONE_SYMDIR = 1,
	SYMROOTBONE_SYMPOS = 2
};


enum tag_befeulkind
{
	BEFEUL_ZERO,
	BEFEUL_BEFFRAME,
	BEFEUL_DIRECT
};


enum tag_axiskind
{
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,
	AXIS_MAX = 3
};


//Bone : m_upkind
enum {
	UPVEC_NONE,
	UPVEC_X,
	UPVEC_Y,
	UPVEC_Z,
	UPVEC_MAX
};

//for bvhelem
enum {
	CHANEL_XPOS,
	CHANEL_YPOS,
	CHANEL_ZPOS,
	CHANEL_ZROT,
	CHANEL_XROT,
	CHANEL_YROT,
	CHANEL_MAX
};


enum {
	FB_NORMAL,
	FB_BUNKI_PAR,
	FB_BUNKI_CHIL,
	FB_ROOT,
	FB_ENDJOINT,
	FB_MAX
};

enum {
	INVAL_ONLYDEFAULT,
	INVAL_ALL,
	INVAL_MAX
};

enum {
	OBJFROM_MQO,
	OBJFROM_FBX,
	OBJFROM_MAX
};


//BoneTriangle
enum {
	BT_PARENT,
	BT_CHILD,
	BT_3RD,
	BT_MAX
};

enum {
	MKIND_ROT,
	MKIND_MV,
	MKIND_SC,
	MKIND_MAX
};

enum {
	FUGOU_ZERO,
	FUGOU_PLUS,
	FUGOU_MINUS
};

enum {
	BONEAXIS_CURRENT = 0,
	BONEAXIS_PARENT = 1,
	BONEAXIS_GLOBAL = 2,
	BONEAXIS_MAX
};

typedef struct tag_cpinfo
{
	WCHAR fbxname[MAX_PATH];
	WCHAR motionname[MAX_PATH];
	double startframe;
	double framenum;
	int bvhtype;//0:undef, 1-144:bvh1 - bvh144, -1:bvh_other
	int importance;//0:undef, 1:tiny, 2:alittle, 3:normal, 4:noticed, 5:imortant, 6:very important
	WCHAR comment[32];//WCHAR * 31�����܂ŁB�R�Q�����ڂ͏I�[�L��
	void Init() {
		ZeroMemory(fbxname, sizeof(WCHAR) * MAX_PATH);
		ZeroMemory(motionname, sizeof(WCHAR) * MAX_PATH);
		startframe = 0.0;
		framenum = 0.0;
		bvhtype = 0;
		importance = 0;
		ZeroMemory(comment, sizeof(WCHAR) * 32);
	};
}CPMOTINFO;


typedef struct tag_historyelem
{
	FILETIME filetime;
	WCHAR wfilename[MAX_PATH];
	int hascpinfo;//cpinfo�ɃZ�b�g�����ꍇ1, �Z�b�g���Ă��Ȃ��ꍇ0
	CPMOTINFO cpinfo;
	bool operator< (const tag_historyelem& right) const {
		LONG lRet = CompareFileTime(&filetime, &(right.filetime));
		if (lRet < 0) {
			return true;
		}
		else if (lRet > 0) {
			return false;
		}
		else {
			return (std::wstring(wfilename) < std::wstring(right.wfilename));
		}
	};
	void Init() {
		ZeroMemory(&filetime, sizeof(FILETIME));
		ZeroMemory(wfilename, sizeof(WCHAR) * MAX_PATH);
		hascpinfo = 0;
		cpinfo.Init();
	};
}HISTORYELEM;

typedef struct tag_anglelimmit
{
	int limitoff[AXIS_MAX];
	int via180flag[AXIS_MAX];// if flag is 1, movable range is "lower --> -180(+180) --> upper"

	int boneaxiskind;//BONEAXIS_CURRENT or BONEAXIS_PARENT or BONEAXIS_GLOBAL

	//from -180 to 180 : the case lower -180 and upper 180, limit is off.
	int lower[AXIS_MAX];
	int upper[AXIS_MAX];

	bool applyeul[AXIS_MAX];
	float chkeul[AXIS_MAX];
}ANGLELIMIT;

typedef struct tag_rigtrans
{
	int axiskind;
	float applyrate;// from -100.0f to 100.0f
	int enable;//enable 1, disable 0
}RIGTRANS;

typedef struct tag_rigelem
{
	int rigrigboneno;
	int rigrigno;
	int boneno;
	RIGTRANS transuv[2];
}RIGELEM;

typedef struct tag_customrig
{
	int useflag;//0 : free, 1 : rental, 2 : valid and in use
	int rigno;//CUSTOMRIG��z��Ŏ����̂��߂�CUSTOMRIG��index
	int rigboneno;
	int elemnum;
	WCHAR rigname[256];
	RIGELEM rigelem[MAXRIGELEMNUM];
	int dispaxis;//����p�I�u�W�F�N�g�\���ꏊ�i���j ver1.0.0.19����
	int disporder;//����p�I�u�W�F�N�g�\���ꏊ�i�Ԗځj ver1.0.0.19����
	bool posinverse;//����p�I�u�W�F�N�g�ʒu�}�C�i�X�ʒu�t���O�@ver1.0.0.19����
}CUSTOMRIG;

typedef struct tag_hinfo
{
	float minh;
	float maxh;
	float height;
}HINFO;

typedef struct tag_reinfo
{
	char filename[MAX_PATH];
	float btgscale;
}REINFO;

typedef struct tag_boneinfluence
{
	DWORD Bone;
    DWORD numInfluences;
    DWORD *vertices;
    FLOAT *weights;
}BONEINFLUENCE;


typedef struct tag_tlelem
{
	int menuindex;
	int motionid;
}TLELEM;

typedef struct tag_modelelem
{
	CModel* modelptr;
	std::vector<TLELEM> tlarray;
	int motmenuindex;
	std::map<int, int> lineno2boneno;
	std::map<int, int> boneno2lineno;
}MODELELEM;


typedef struct tag_xmliobuf
{
	char* buf;
	int bufleng;
	int pos;
	int isend;
}XMLIOBUF;

enum {
	XMLIO_LOAD,
	XMLIO_WRITE,
	XMLIO_MAX
};

#define XMLIOLINELEN	1024


//enum {
//	PICK_NONE,
//	PICK_CENTER,
//	PICK_X,
//	PICK_Y,
//	PICK_Z,
//	PICK_CAMMOVE,
//	PICK_CAMROT,
//	PICK_CAMDIST,
//	PICK_SPA_X,
//	PICK_SPA_Y,
//	PICK_SPA_Z,
//	PICK_MAX
//};
enum {
	PICK_NONE,
	PICK_CENTER,
	PICK_X,
	PICK_Y,
	PICK_Z,
	PICK_CAMROT,
	PICK_CAMMOVE,
	PICK_CAMDIST,
	PICK_SPA_X,
	PICK_SPA_Y,
	PICK_SPA_Z,
	PICK_UNDO,
	PICK_REDO,
	PICK_MAX
};


//enum {
//	SPR_CAM_I = 0,
//	SPR_CAM_KAI = 1,
//	SPR_CAM_KAKU = 2,
//	SPR_CAM_MAX
//};
enum {
	SPR_CAM_KAI = 0,
	SPR_CAM_I = 1,
	SPR_CAM_KAKU = 2,
	SPR_CAM_MAX
};


typedef struct tag_motinfo
{
	char motname[256];
	WCHAR wfilename[MAX_PATH];
	char engmotname[256];
	int motid;
	double frameleng;
	double curframe;
	double speed;
	int loopflag;
}MOTINFO;


#define INFSCOPEMAX	10
#define INFNUMMAX	4

typedef struct tag_infelem
{
	int boneno;
	int kind;//CALCMODE_*
	float userrate;//
	float orginf;//CALCMODE_*�Ōv�Z�����l�B
	float dispinf;//�@orginf[] * userrate[]�Anormalizeflag���P�̂Ƃ��́A���K������B
	int isadditive;//�e���x�̐��K�������B
}INFELEM;


typedef struct rgbdat
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
} RGBDAT;

typedef struct rgb3f
{
	float r;
	float g;
	float b;
} RGB3F;

typedef struct tag_argbfdat
{
	float a;
	float r;
	float g;
	float b;
} ARGBF;

// material mode
enum {
	MMODE_VERTEX,
	MMODE_FACE,
	MMODE_MAX
};

typedef struct tag_infdata
{
	int m_infnum;
	INFELEM m_infelem[INFNUMMAX];
} INFDATA;

typedef struct tag_chkalpha
{
	int alphanum;
	int notalphanum;
} CHKALPHA;


typedef struct tag_mqobuf
{
	HANDLE hfile;
	unsigned char* buf;
	DWORD bufleng;
	DWORD pos;
	int isend;
} MQOBUF;


enum
{
	MATERIAL_DIFFUSE,
	MATERIAL_SPECULAR,
	MATERIAL_AMBIENT,
	MATERIAL_EMISSIVE,
	MATERIAL_POWER,
	MATERIAL_MAX
};


typedef struct tag_materialblock
{
	int materialno;
	int startface;
	int endface;
	CMQOMaterial* mqomat;
} MATERIALBLOCK;

typedef struct tag_dirtymat
{
	int materialno;
	int* dirtyflag;
} DIRTYMAT;

typedef struct tag_vcoldata
{
	int vertno;
	__int64 vcol;
} VCOLDATA;

enum {
	SYMMTYPE_NONE,
	SYMMTYPE_L,
	SYMMTYPE_R,
	SYMMTYPE_M,
	SYMMTYPE_MAX
};


typedef struct tag_infelemheader
{
	int infnum;
	int normalizeflag;
	int symaxis;
	float symdist;
}INFELEMHEADER;

enum {
	MIKOBONE_NONE,
	MIKOBONE_NORMAL,
	MIKOBONE_FLOAT,
	MIKOBONE_NULL,
	MIKOBONE_ILLEAGAL,
	MIKOBONE_MAX
};

enum {
	FBXBONE_NONE,
	FBXBONE_NORMAL,
	FBXBONE_NULL,
	FBXBONE_ILLEAGAL,
	FBXBONE_MAX
};

enum {
	MIKODEF_NONE,
	MIKODEF_SDEF,
	MIKODEF_BDEF,
	MIKODEF_NODEF,
	MIKODEF_MAX
};

enum {
	MIKOBLEND_SKINNING,
	MIKOBLEND_MIX,
	MIKOBLEND_MAX
};

enum {
	PARSMODE_PARS,
	PARSMODE_ORTHO,
	PARSMODE_MAX
};

typedef struct tag_tvertex {
	float pos[4]; 
	float tex[2];
} TVERTEX;

typedef struct tag_tlvertex {
	float pos[4];
	float diffuse[4];
	float tex[2];
} TLVERTEX;



typedef struct tag_pm3inf
{
	float weight[4];
	int boneindex[4];
}PM3INF;

typedef struct tag_brushstate
{
	int motionbrush_method;
	int wallscrapingikflag;
	bool limitdegflag;
	int brushmirrorUflag;
	int brushmirrorVflag;
	int ifmirrorVDiv2flag;
	int brushrepeats;

	void Init()
	{
		motionbrush_method = 0;
		wallscrapingikflag = 0;
		limitdegflag = false;
		brushmirrorUflag = 0;
		brushmirrorVflag = 0;
		ifmirrorVDiv2flag = 0;
		brushrepeats = 1;
	};
}BRUSHSTATE;


// error code
// d3dapp.h ����ړ��B

#ifndef D3DAPPERR_NODIRECT3D
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c

#endif

typedef struct tag_errormes {
	int errorcode;
	DWORD type;
	char* mesptr;
} ERRORMES;

//interpolation
enum {
	INTERPOLATION_SLERP,
	INTERPOLATION_SQUAD,
	INTERPOLATION_MAX
};

//user defined window message

#define WM_USER_DISPLAY		WM_USER


////////////////////////////////
#define BITMASK_0	0x00000001
#define BITMASK_1	0x00000002
#define BITMASK_2	0x00000004
#define BITMASK_3	0x00000008

#define BITMASK_4	0x00000010
#define BITMASK_5	0x00000020
#define BITMASK_6	0x00000040
#define BITMASK_7	0x00000080

#define BITMASK_8	0x00000100
#define BITMASK_9	0x00000200
#define BITMASK_10	0x00000400
#define BITMASK_11	0x00000800

#define BITMASK_12	0x00001000
#define BITMASK_13	0x00002000
#define BITMASK_14	0x00004000
#define BITMASK_15	0x00008000

#define BITMASK_16	0x00010000
#define BITMASK_17	0x00020000
#define BITMASK_18	0x00040000
#define BITMASK_19	0x00080000

#define BITMASK_20	0x00100000
#define BITMASK_21	0x00200000
#define BITMASK_22	0x00400000
#define BITMASK_23	0x00800000

#define BITMASK_24	0x01000000
#define BITMASK_25	0x02000000
#define BITMASK_26	0x04000000
#define BITMASK_27	0x08000000

#define BITMASK_28	0x10000000
#define BITMASK_29	0x20000000
#define BITMASK_30	0x40000000
#define BITMASK_31	0x80000000

////////////////////////////////
#define D3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)
#define D3DFVF_TLVERTEX2 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)

#define D3DFVF_VERTEX	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

#define D3DFVF_LVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)


//		mottype : motno �̐�����@�B
enum _motiontype {
	MOTIONTYPENONE,

	MOTION_STOP, // motionno �Œ�B
	MOTION_CLAMP, // motionno ���@max�ɒB������A���̂܂܂����ŌŒ�B
	MOTION_ROUND, //		�ŏ��ɖ߂�
	MOTION_INV,	//			�t�����ɐi�ށB
	MOTION_JUMP,
	//MOTION_BIV, // ������������A�o�C�u���[�V�������ǉ��B 

	MOTIONTYPEMAX
};



////////////////////


#ifndef COEFHSTRING

#else

/***
	char strskel[ SKEL_MAX ][30] = {
		"TOPOFJOINT",
		"TORSO",
		"LEFT_HIP",
		"LEFT_KNEE",
		"LEFT_FOOT",
		"RIGHT_HIP",
		"RIGHT_KNEE",
		"RIGHT_FOOT",
		"NECK",
		"HEAD",
		"LEFT_SHOULDER",
		"LEFT_ELBOW",
		"LEFT_HAND",
		"RIGHT_SHOULDER",
		"RIGHT_ELBOW",
		"RIGHT_HAND"
	};

	char strconvskel[ SKEL_MAX ][30] = {
		"00TOP_OF_JOINT",
		"01SKEL_TORSO",
		"02SKEL_LEFT_HIP",
		"03SKEL_LEFT_KNEE",
		"04SKEL_LEFT_FOOT",
		"05SKEL_RIGHT_HIP",
		"06SKEL_RIGHT_KNEE",
		"07SKEL_RIGHT_FOOT",
		"08SKEL_NECK",
		"09SKEL_HEAD",
		"10SKEL_LEFT_SHOULDER",
		"11SKEL_LEFT_ELBOW",
		"12SKEL_LEFT_HAND",
		"13SKEL_RIGHT_SHOULDER",
		"14SKEL_RIGHT_ELBOW",
		"15SKEL_RIGHT_HAND"
	};
***/

#endif

#endif