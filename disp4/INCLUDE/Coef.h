#ifndef COEFH
#define		COEFH

#include <D3DX9.h>
#include <usercoef.h>

#include <vector>
#include <map>

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
#define MULTIPATH	(MAX_PATH * 100)

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

#define UNDOMAX 200

//boneごとのRIG配列長
#define MAXRIGNUM	10
#define MAXRIGELEMNUM	5

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
	BEFEUL_BEFFRAME
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

typedef struct tag_anglelimmit
{
	int limitoff[AXIS_MAX];
	int via180flag[AXIS_MAX];// if flag is 1, movable range is "lower --> -180(+180) --> upper"

	int boneaxiskind;//BONEAXIS_CURRENT or BONEAXIS_PARENT or BONEAXIS_GLOBAL

	//from -180 to 180 : the case lower -180 and upper 180, limit is off.
	int lower[AXIS_MAX];
	int upper[AXIS_MAX];
}ANGLELIMIT;

typedef struct tag_rigtrans
{
	int axiskind;
	float applyrate;// from -100.0f to 100.0f
}RIGTRANS;

typedef struct tag_rigelem
{
	int boneno;
	RIGTRANS transuv[2];
}RIGELEM;

typedef struct tag_customrig
{
	int useflag;//0 : free, 1 : rental, 2 : valid and in use
	int rigno;//CUSTOMRIGを配列で持つ側のためのCUSTOMRIGのindex
	int rigboneno;
	int elemnum;
	WCHAR rigname[256];
	RIGELEM rigelem[MAXRIGELEMNUM];
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


enum {
	PICK_NONE,
	PICK_CENTER,
	PICK_X,
	PICK_Y,
	PICK_Z,
	PICK_CAMMOVE,
	PICK_CAMROT,
	PICK_CAMDIST,
	PICK_SPA_X,
	PICK_SPA_Y,
	PICK_SPA_Z,
	PICK_MAX
};

enum {
	SPR_CAM_I = 0,
	SPR_CAM_KAI = 1,
	SPR_CAM_KAKU = 2,
	SPR_CAM_MAX
};

typedef struct tag_pickinfo
{
	int buttonflag;//WM_LBUTTONDOWN-->PICK_L, WM_RBUTTONDOWN-->PICK_R, WM_MBUTTONDOWN-->PICK_M。押していないとき-->PICK_NONE。
	//以下、buttonflagがPICK_NONE以外の時に意味を持つ。
	POINT mousepos;
	POINT mousebefpos;
	POINT clickpos;
	D3DXVECTOR2 diffmouse;
	D3DXVECTOR2 firstdiff;
	int winx;
	int winy;
	int pickrange;
	int pickobjno;
	D3DXVECTOR3 objscreen;
	D3DXVECTOR3 objworld;
}PICKINFO;

typedef struct tag_cpmot
{
	int boneno;
	double frame;
	D3DXVECTOR3 eul;
	D3DXVECTOR3 tra;
}CPMOT;

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

typedef struct tag_texv
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 uv;
}TEXV;

#define INFSCOPEMAX	10
#define INFNUMMAX	4

typedef struct tag_infelem
{
	int boneno;
	int kind;//CALCMODE_*
	float userrate;//
	float orginf;//CALCMODE_*で計算した値。
	float dispinf;//　orginf[] * userrate[]、normalizeflagが１のときは、正規化する。
	int isadditive;//影響度の正規化無し。
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

typedef struct tag_spritev {
	D3DXVECTOR4 pos;
	D3DXVECTOR2 uv;
} SPRITEV;


typedef struct tag_verface
{
	int			faceno;
	int			orgfaceno;
	int			materialno;
	D3DXVECTOR3	facenormal;
}PERFACE;

typedef struct tag_pervert
{
	int				indexno;//3角の順番
	int				vno;
	int				uvnum;
	D3DXVECTOR2		uv[2];
	int				vcolflag;
	DWORD			vcol;
	D3DXVECTOR3 smnormal;

	int				createflag;
		//頂点を作成しない場合０
		//UV, VCOL, Materialnoの違いにより作成する場合は１を足す
		//normalにより作成する場合は２を足す

}PERVERT;

typedef struct tag_n3sm
{
	int smfacenum;
	void** ppsmface;//N3Pのポインタの配列
}N3SM;

typedef struct tag_n3p
{
	PERFACE*	perface;
	PERVERT*	pervert;
	N3SM*		n3sm;
}N3P;//n*3

typedef struct tag_pm3optv
{
	int orgvno;
	int orgfaceno;
	D3DXVECTOR3 pos;
	int materialno;
	D3DXVECTOR3 normal;
	int				uvnum;
	D3DXVECTOR2		uv[2];
	int				vcolflag;
	DWORD			vcol;
}PM3OPTV;

typedef struct tag_pm3dispv
{
	D3DXVECTOR4		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		uv;
}PM3DISPV;

typedef struct tag_pm3inf
{
	float weight[4];
	float boneindex[4];
}PM3INF;

typedef struct tag_extlinev
{
	D3DXVECTOR4 pos;
}EXTLINEV;


typedef struct tag_modelbound
{
	D3DXVECTOR3 min;
	D3DXVECTOR3 max;
	D3DXVECTOR3 center;
	float		r;
}MODELBOUND;

// error code
// d3dapp.h から移動。

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


//		mottype : motno の制御方法。
enum _motiontype {
	MOTIONTYPENONE,

	MOTION_STOP, // motionno 固定。
	MOTION_CLAMP, // motionno が　maxに達したら、そのままそこで固定。
	MOTION_ROUND, //		最初に戻る
	MOTION_INV,	//			逆方向に進む。
	MOTION_JUMP,
	//MOTION_BIV, // 落ち着いたら、バイブレーションも追加。 

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