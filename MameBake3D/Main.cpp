/*
2016/04/22　説明文修正
[IKについて]
MameBake3DのIK方法は相対IK(SlerpIK)と絶対IK(AbsIK)がある。

相対IKとは
姿勢適用ボーンの姿勢の変化分(編集前後での変化分)を
フレームに関して補間し、それを編集前の姿勢に掛け算する方法である。

絶対IKとは
姿勢適用ボーンの姿勢をそのまま選択フレームに反映する方法である。
絶対IKで部分的に動かす場合、適用した部分だけ絶対的な姿勢になるので親の位置や回転の影響も受けなくなる。
体のつながった部分に絶対IKを使用して編集すると、体が全体移動した際などに部分がちぎれるので注意してください。
絶対IKの使いどころとしては、ロケットパンチなどのワンスキンではない部位へのグローバル座標系でのアニメーションです。


MameBake3Dはデフォルトで相対IKです。

メインウインドウのSlerpIKを切るチェックボックスをオンにすると
適用分の姿勢の補間が切れる。

メインウインドウの絶対IKをオンにすると
絶対IKになる。
*/


#include "useatl.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#include <DXUT.h>
#include <DXUTcamera.h>
#include <DXUTgui.h>
#include <DXUTsettingsdlg.h>
#include <SDKmisc.h>
#include "resource.h"

#include <Coef.h>
#include <Model.h>
#include <TexBank.h>
#include <Bone.h>
#include <MySprite.h>
#include <mqoobject.h>

#include <OrgWindow.h>

#define DBGH
#include <dbg.h>

#include <shlobj.h> //shell
#include <Commdlg.h>
#include <ChaVecCalc.h>

#include <mqomaterial.h>

#include <ChaFile.h>
#include <ImpFile.h>
#include <RigidElemFile.h>
#include <RigidElem.h>

#include <BtObject.h>
#include <fbxsdk.h>

#include <BPWorld.h>
#include <GlutStuff.h>
#include <GLDebugDrawer.h>
#include <btBulletDynamicsCommon.h>

#include "ColiIDDlg.h"
#include "GColiIDDlg.h"
#include "RegistDlg.h"
#include <GColiFile.h>

#include <math.h>
#include <stdio.h>

#include <EditRange.h>
#include "FrameCopyDlg.h"

#include <BVHFile.h>
#include "FBXFile.h"

#include <BntFile.h>

#include <Model.h>
#include "RMenuMain.h"
//#include <BoneProp.h>
#include <lmtFile.h>
#include <RigFile.h>
#include <MotFilter.h>

typedef struct tag_spaxis
{
	CMySprite* sprite;
	POINT dispcenter;
}SPAXIS, SPCAM, SPELEM;


ID3D10DepthStencilState *g_pDSStateZCmp = 0;
ID3D10DepthStencilState *g_pDSStateZCmpAlways = 0;
ID3D10ShaderResourceView* g_presview = 0;

int	g_numthread = 3;
double g_btcalccnt = 3.0;
int g_dbgloadcnt = 0;
double g_calcfps = 60.0;

extern map<CModel*,int> g_bonecntmap;
extern int gNumIslands;
extern void InitCustomRig(CUSTOMRIG* dstcr, CBone* parbone, int rigno);
extern int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parbone);
//void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* chilbone);
extern int IsValidRigElem(CModel* srcmodel, RIGELEM srcrigelem);

extern void DXUTSetOverrideSize(int srcw, int srch);


//ChaMatrix s_selectmat;//for display manipulator
//ChaMatrix s_ikselectmat;//for ik, fk
ChaMatrix s_selectmat;//for display manipulator
ChaMatrix s_ikselectmat;//for ik, fk
bool g_selecttolastFlag = false;
bool g_underselecttolast = false;
bool g_undereditrange = false;

bool g_limitdegflag = true;
bool g_wmatDirectSetFlag = false;
bool g_underRetargetFlag = false;

static int s_onragdollik = 0;
static int s_physicskind = 0;

static CMQOMaterial* s_matred = 0;// = s_select->GetMQOMaterialByName("matred");
static CMQOMaterial* s_ringred = 0;// = s_select->GetMQOMaterialByName("ringred");
static CMQOMaterial* s_matblue = 0;// = s_select->GetMQOMaterialByName("matblue");
static CMQOMaterial* s_ringblue = 0;// = s_select->GetMQOMaterialByName("ringblue");
static CMQOMaterial* s_matgreen = 0;// = s_select->GetMQOMaterialByName("matgreen");
static CMQOMaterial* s_ringgreen = 0;// = s_select->GetMQOMaterialByName("ringgreen");
static CMQOMaterial* s_matyellow = 0;// = s_select->GetMQOMaterialByName("matyellow");
static ChaVector4 s_matredmat;
static ChaVector4 s_ringredmat;
static ChaVector4 s_matbluemat;
static ChaVector4 s_ringbluemat;
static ChaVector4 s_matgreenmat;
static ChaVector4 s_ringgreenmat;
static ChaVector4 s_matyellowmat;
static ChaVector4 s_ringyellowmat;

static bool s_dispanglelimit = false;
static HWND s_anglelimitdlg = 0;
static ANGLELIMIT s_anglelimit;
static CBone* s_anglelimitbone = 0;

static HWND s_rotaxisdlg = 0;
static int s_rotaxiskind = AXIS_X;
static float s_rotaxisdeg = 0.0f;

static HWND s_customrigdlg = 0;
static CUSTOMRIG s_customrig;
static CUSTOMRIG s_ikcustomrig;
static CBone* s_customrigbone = 0;
static int s_customrigno = 0;
static map<int, int> s_customrigmenuindex;

static int s_forcenewaxis = 0;
static int s_doneinit = 0;
static int s_underselectingframe = 0;
static double s_buttonselectstart = 0.0;
static double s_buttonselectend = 0.0;
static int s_buttonselecttothelast = 0;

static float s_selectscale = 1.0f;
static int s_sethipstra = 0;
static CFrameCopyDlg s_selbonedlg;
static int s_allmodelbone = 0;

//float g_initcamdist = 10.0f;
//static float s_projnear = 0.01f;
float g_initcamdist = 50.0f;
static float s_projnear = 0.001f;
static float s_fAspectRatio = 1.0f;
static float s_cammvstep = 100.0f;
static int s_editmotionflag = -1;

static WCHAR s_strcurrent[256] = L"Move To Current Frame";
static WCHAR s_streditrange[256] = L"Drag Edit Range";
static WCHAR s_strmark[256] = L"Mark Key Line";

float g_impscale = 1.0f;
float g_initmass = 1.0f;

ChaVector3 g_camEye = ChaVector3( 0.0f, 0.0f, 0.0f );
ChaVector3 g_camtargetpos = ChaVector3( 0.0f, 0.0f, 0.0f );


//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.2f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1e8 };
//float g_a_kval[3] = { powf( 10.0f, 0.0f ), powf( 10.0f, 0.3f ), 1e8 };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1000.0f };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.27f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.3f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1000.0f };

float g_l_kval[3] = { 1.0f, powf( 10.0f, 2.61f ), 2000.0f };//
float g_a_kval[3] = { 0.1f, powf( 10.0f, 0.3f ), 70.0f };//
float g_initcuslk = 1e2;
//float g_initcuslk = 2000.0f;
//float g_initcuslk = 100.0f;
float g_initcusak = 70.0f;


//float g_l_dmp = 0.75f;
//float g_a_dmp = 0.50f;

float g_l_dmp = 0.50f;
float g_a_dmp = 0.50f;


bool g_controlkey = false;
bool g_shiftkey = false;
bool g_ctrlshiftkeyformb = false;//ForMiddleButton
static int s_akeycnt = 0;
static int s_dkeycnt = 0;
static int s_1keycnt = 0;

static bool s_dispsampleui = true;


static double s_erp = 1.0;
//static float s_erp = 0.99f;
//static float s_erp = 0.75f;
//static double s_erp = 0.5;
//static float s_erp = 0.2f;
//static float s_impup = 0.0f;


int g_previewFlag = 0;			// プレビューフラグ
static int s_savepreviewFlag = 0;
double s_btstartframe = 0.0;

static FbxManager* s_psdk = 0;
static BPWorld* s_bpWorld = 0;
static btDynamicsWorld* s_btWorld = 0;

using namespace OrgWinGUI;

static ChaMatrix s_inimat;
static double s_time = 0.0;
static double s_difftime = 0.0;
static int s_ikkind = 0;
static int s_iklevel = 1;

static PICKINFO s_pickinfo;
static vector<TLELEM> s_tlarray;
static int s_curmotmenuindex = -1;
static int s_curreindex = -1;
static int s_rgdindex = -1;

static HWND		s_mainwnd = 0;
static HMENU	s_mainmenu = 0;
static HMENU	s_animmenu = 0;
static HMENU	s_morphmenu = 0;
static HMENU	s_modelmenu = 0;
static HMENU	s_remenu = 0;
static HMENU	s_rgdmenu = 0;
static HMENU	s_impmenu = 0;
static int s_filterindex = 1;

//static int s_mainwidth = 600;
//static int s_mainheight = 620;
static int s_mainwidth = 800;
//static int s_mainheight = 820;
static int s_mainheight = 600;
static int s_bufwidth = 800;
static int s_bufheight = 600;

static ID3D10Device* s_pdev = 0;

static CModel* s_model = NULL;
static CModel* s_select = NULL;
static CModel* s_bmark = NULL;
//static CModel* s_coldisp[ COL_MAX ];
static CModel* s_ground = NULL;
static CModel* s_gplane = NULL;
static CModel* s_rigmark = NULL;

//static CModel* s_dummytri = NULL;
static CMySprite* s_bcircle = 0;
static CMySprite* s_kinsprite = 0;

static int s_fbxbunki = 1;


//static ChaMatrix s_matWorld;
//static ChaMatrix s_matProj;
////static ChaMatrix s_matW, s_matVP;
//static ChaMatrix s_matVP;
//static ChaMatrix s_matView;
static ChaMatrix s_matWorld;
static ChaMatrix s_matProj;
static ChaMatrix s_matVP;
static ChaMatrix s_matView;
static ChaVector3 s_camUpVec = ChaVector3(0.00001f, 1.0f, 0.0f);
static float s_camdist = g_initcamdist;

static int s_curmotid = -1;
static int s_curboneno = -1;
static int s_saveboneno = -1;
static int s_curbaseno = -1;
static int s_ikcnt = 0;
static ChaMatrix s_selm = ChaMatrix( 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);

static OrgWindow* s_timelineWnd = 0;
static OWP_Timeline* s_owpTimeline = 0;
static OWP_PlayerButton* s_owpPlayerButton = 0;

static OrgWindow* s_LtimelineWnd = 0;
static OWP_Timeline* s_owpLTimeline = 0;


static OrgWindow* s_dmpanimWnd = 0;
static OWP_CheckBox* s_dmpgroupcheck = 0;
static OWP_Label* s_dmpanimLlabel = 0;
static OWP_Slider* s_dmpanimLSlider = 0;
static OWP_Label* s_dmpanimAlabel = 0;
static OWP_Slider* s_dmpanimASlider = 0;
static OWP_Button* s_dmpanimB = 0;

static OrgWindow* s_rigidWnd = 0;
static OWP_CheckBox* s_groupcheck = 0;
static OWP_Slider* s_shprateSlider = 0;
static OWP_Slider* s_boxzSlider = 0;
static OWP_Slider* s_massSlider = 0;
static OWP_Button* s_massB = 0;
static OWP_CheckBox* s_rigidskip = 0;
static OWP_CheckBox* s_forbidrot = 0;
static OWP_Label* s_shplabel = 0;
static OWP_Label* s_boxzlabel = 0;
static OWP_RadioButton* s_colradio = 0;
static OWP_RadioButton* s_lkradio = 0;
static OWP_RadioButton* s_akradio = 0;
static OWP_Slider* s_ldmpSlider = 0;
static OWP_Slider* s_admpSlider = 0;
static OWP_Label* s_massSLlabel = 0;
static OWP_Label* s_namelabel = 0;
static OWP_Label* s_lenglabel = 0;
static OWP_Label* s_ldmplabel = 0;
static OWP_Label* s_admplabel = 0;
static OWP_Button* s_kB = 0;
static OWP_Button* s_restB = 0;
static OWP_Button* s_dmpB = 0;
static OWP_Button* s_groupB = 0;
static OWP_Button* s_gcoliB = 0;
static OWP_Button* s_allrigidenableB = 0;
static OWP_Button* s_allrigiddisableB = 0;


static OWP_Slider* s_lkSlider = 0;
static OWP_Label* s_lklabel = 0;
static OWP_Slider* s_akSlider = 0;
static OWP_Label* s_aklabel = 0;

static OWP_Slider* s_restSlider = 0;
static OWP_Label* s_restlabel = 0;
static OWP_Slider* s_fricSlider = 0;
static OWP_Label* s_friclabel = 0;

static OWP_Slider* s_btgSlider = 0;
static OWP_Label* s_btglabel = 0;
static OWP_Slider* s_btgscSlider = 0;
static OWP_Label* s_btgsclabel = 0;
static OWP_Button* s_btgB = 0;
static OWP_CheckBox* s_btforce = 0;

static OrgWindow* s_impWnd = 0;
static OWP_CheckBox* s_impgroupcheck = 0;
static OWP_Slider* s_impxSlider = 0;
static OWP_Slider* s_impySlider = 0;
static OWP_Slider* s_impzSlider = 0;
static OWP_Slider* s_impscaleSlider = 0;
static OWP_Label* s_impxlabel = 0;
static OWP_Label* s_impylabel = 0;
static OWP_Label* s_impzlabel = 0;
static OWP_Label* s_impscalelabel = 0;
static OWP_Button* s_impallB = 0;


static OrgWindow* s_gpWnd = 0;
static OWP_Slider* s_ghSlider = 0;
static OWP_Slider* s_gsizexSlider = 0;
static OWP_Slider* s_gsizezSlider = 0;
static OWP_Label* s_ghlabel = 0;
static OWP_Label* s_gsizexlabel = 0;
static OWP_Label* s_gsizezlabel = 0;
static OWP_CheckBox* s_gpdisp = 0;
static OWP_Slider* s_grestSlider = 0;
static OWP_Label* s_grestlabel = 0;
static OWP_Slider* s_gfricSlider = 0;
static OWP_Label* s_gfriclabel = 0;

static OrgWindow* s_toolWnd = 0;		
static OWP_Button* s_toolCopyB = 0;
static OWP_Button* s_toolSymCopyB = 0;
static OWP_Button* s_toolCutB = 0;
static OWP_Button* s_toolPasteB = 0;
static OWP_Button* s_toolDeleteB = 0;
static OWP_Button* s_toolMotPropB = 0;
static OWP_Button* s_toolMarkB = 0;
static OWP_Button* s_toolSelBoneB = 0;
static OWP_Button* s_toolInitMPB = 0;
static OWP_Button* s_toolFilterB = 0;
static OWP_Button* s_toolInterpolateB = 0;

#define CONVBONEMAX		256
static OrgWindow* s_convboneWnd = 0;
static OWP_ScrollWnd* s_convboneSCWnd = 0;
static int s_convbonenum = 0;
static OWP_Button* s_cbselmodel = 0;
static OWP_Button* s_cbselbvh = 0;
static OWP_Label* s_convbonemidashi[2];
static OWP_Label* s_modelbone[CONVBONEMAX];
static OWP_Button* s_bvhbone[CONVBONEMAX];
static OWP_Separator* s_convbonesp = 0;
static OWP_Button* s_convboneconvert = 0;
static CModel* s_convbone_model = 0;
static CModel* s_convbone_bvh = 0;
static CBone* s_modelbone_bone[CONVBONEMAX];
static CBone* s_bvhbone_bone[CONVBONEMAX];
static map<CBone*, CBone*> s_convbonemap;


static OrgWindow* s_layerWnd = 0;
static OWP_LayerTable* s_owpLayerTable = 0;

static bool s_closeFlag = false;			// 終了フラグ
static bool s_closetoolFlag = false;
static bool s_closeobjFlag = false;
static bool s_closemodelFlag = false;
static bool s_closeconvboneFlag = false;
static bool s_DcloseFlag = false;
static bool s_RcloseFlag = false;
static bool s_IcloseFlag = false;
static bool s_GcloseFlag = false;
static bool s_copyFlag = false;			// コピーフラグ
static bool s_symcopyFlag = false;
static bool s_undersymcopyFlag = false;
static bool s_cutFlag = false;			// カットフラグ
static bool s_pasteFlag = false;			// ペーストフラグ
static bool s_cursorFlag = false;			// カーソル移動フラグ
static bool s_selectFlag = false;			// キー選択フラグ
static bool s_keyShiftFlag = false;		// キー移動フラグ
static bool s_deleteFlag = false;		// キー削除フラグ
static bool s_motpropFlag = false;
static bool s_markFlag = false;
static bool s_selboneFlag = false;
static bool s_initmpFlag = false;
static bool s_filterFlag = false;
static bool s_interpolateFlag = false;

static bool s_firstkeyFlag = false;
static bool s_lastkeyFlag = false;
static bool s_btresetFlag = false;

static bool s_LcloseFlag = false;
static bool s_LnextkeyFlag = false;
static bool s_LbefkeyFlag = false;
static bool s_LcursorFlag = false;			// カーソル移動フラグ
static bool s_LstartFlag = false;
static bool s_LstopFlag = false;

static bool s_timelineRUpFlag = false;
static bool s_timelinembuttonFlag = false;
static int s_mbuttoncnt = 1;
static bool s_timelinewheelFlag = false;

static bool s_prevrangeFlag = false;
static bool s_nextrangeFlag = false;

static bool s_dispmw = true;
static bool s_disptool = true;
//static bool s_dispobj = true;
static bool s_dispobj = false;
static bool s_dispmodel = false;//!!!!!!!!!!!!!!!!!
static bool s_dispground = true;
static bool s_dispselect = true;
//static bool s_displightarrow = true;
static bool s_dispconvbone = false;


static bool s_Ldispmw = true;

static double s_keyShiftTime = 0.0;			// キー移動量
static list<KeyInfo> s_copyKeyInfoList;	// コピーされたキー情報リスト

//static multimap<CBone*, CMotionPoint> s_copymotmap;
typedef struct tag_cpelem
{
	CBone* bone;
	CMotionPoint mp;
}CPELEM;
static vector<CPELEM> s_copymotmap;

static list<KeyInfo> s_deletedKeyInfoList;	// 削除されたキー情報リスト
static list<KeyInfo> s_selectKeyInfoList;	// コピーされたキー情報リスト

static CEditRange s_editrange;
static CEditRange* s_editrangehistory = 0;
static int s_editrangehistoryno = 0;
static int s_editrangesetindex = 0;
static CEditRange s_previewrange;
static SPAXIS s_spaxis[3];
static SPCAM s_spcam[3];
static SPELEM s_sprig[2];//inactive, active
static SPELEM s_spbt;
static int s_oprigflag = 0;

typedef struct tag_modelpanel
{
	OrgWindow* panel;
	OWP_RadioButton* radiobutton;
	OWP_Separator* separator;
	vector<OWP_CheckBox*> checkvec;
	int modelindex;
}MODELPANEL;

static MODELPANEL s_modelpanel;

static map<int, int> s_lineno2boneno;
static map<int, int> s_boneno2lineno;

static vector<MODELELEM> s_modelindex;
static MODELBOUND	s_totalmb;
static int s_curmodelmenuindex = -1;

static WCHAR s_tmpmotname[256] = {0L};
static double s_tmpmotframeleng = 100.0f;
//static double s_tmpmotframeleng = 5640.0f;
static int s_tmpmotloop = 1;

static WCHAR s_projectname[64] = {0L};
static WCHAR s_projectdir[MAX_PATH] = {0L};
static WCHAR s_chapath[MAX_PATH] = {0};
static WCHAR s_chasavename[64] = {0};
static WCHAR s_chasavedir[MAX_PATH] = {0};

static WCHAR s_REname[MAX_PATH] = {0L};
static WCHAR s_Impname[MAX_PATH] = {0L};
static WCHAR s_Gconame[MAX_PATH] = {0L};

static int s_camtargetflag = 0;
int g_applyendflag = 0;
int g_slerpoffflag = 0;
int g_absikflag = 0;
int g_bonemarkflag = 1;
int g_pseudolocalflag = 1;
int g_boneaxis = 1;
CDXUTCheckBox* s_CamTargetCheckBox = 0;
//CDXUTCheckBox* s_LightCheckBox = 0;
CDXUTCheckBox* s_ApplyEndCheckBox = 0;
CDXUTCheckBox* s_SlerpOffCheckBox = 0;
CDXUTCheckBox* s_AbsIKCheckBox = 0;
CDXUTCheckBox* s_BoneMarkCheckBox = 0;
CDXUTCheckBox* s_PseudoLocalCheckBox = 0;
CDXUTCheckBox* s_LimitDegCheckBox = 0;

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

ChaVector3 g_vCenter( 0.0f, 0.0f, 0.0f );
CTexBank*	g_texbank = 0;

float g_tmpmqomult = 1.0f;
WCHAR g_tmpmqopath[MULTIPATH] = {0L};
float g_tmpbvhfilter = 100.0f;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DX10Font*                  g_pFont = NULL;         // Font for drawing text
ID3DX10Sprite*                g_pSprite = NULL;       // Sprite for batching draw text calls
CDXUTTextHelper*              g_pTxtHelper = NULL;

bool                        g_bShowHelp = true;     // If true, it renders the UI control text
CModelViewerCamera          g_Camera;               // A model viewing camera
ID3D10Effect*                g_pEffect = NULL;       // D3DX effect interface
//ID3DXMesh*                  g_pMesh = NULL;         // Mesh object

//IDirect3DTexture10*          g_pMeshTexture = NULL;  // Mesh texture
LPD3DXFONT                  g_pFont9 = NULL;         // Font for drawing text


CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
//CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D UI
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
bool                        g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader
//ChaMatrix               g_mCenterWorld;
ChaMatrix               g_mCenterWorld;

#define MAX_LIGHTS 3
CDXUTDirectionWidget g_LightControl[MAX_LIGHTS];
float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;

double						g_dspeed = 3.0;
//double						g_dspeed = 0.0;

ID3D10EffectTechnique* g_hRenderBoneL0 = 0;
ID3D10EffectTechnique* g_hRenderBoneL1 = 0;
ID3D10EffectTechnique* g_hRenderBoneL2 = 0;
ID3D10EffectTechnique* g_hRenderBoneL3 = 0;
ID3D10EffectTechnique* g_hRenderNoBoneL0 = 0;
ID3D10EffectTechnique* g_hRenderNoBoneL1 = 0;
ID3D10EffectTechnique* g_hRenderNoBoneL2 = 0;
ID3D10EffectTechnique* g_hRenderNoBoneL3 = 0;
ID3D10EffectTechnique* g_hRenderLine = 0;
ID3D10EffectTechnique* g_hRenderSprite = 0;

ID3D10EffectMatrixVariable* g_hm4x4Mat = 0;
ID3D10EffectMatrixVariable* g_hmWorld = 0;
ID3D10EffectMatrixVariable* g_hmVP = 0;

ID3D10EffectVectorVariable* g_hEyePos = 0;
ID3D10EffectScalarVariable* g_hnNumLight = 0;
ID3D10EffectVectorVariable* g_hLightDir = 0;
ID3D10EffectVectorVariable* g_hLightDiffuse = 0;
ID3D10EffectVectorVariable* g_hLightAmbient = 0;
ID3D10EffectVectorVariable* g_hSpriteOffset = 0;
ID3D10EffectVectorVariable* g_hSpriteScale = 0;
ID3D10EffectVectorVariable* g_hPm3Scale = 0;
ID3D10EffectVectorVariable* g_hPm3Offset = 0;


ID3D10EffectVectorVariable* g_hdiffuse = 0;
ID3D10EffectVectorVariable* g_hambient = 0;
ID3D10EffectVectorVariable* g_hspecular = 0;
ID3D10EffectScalarVariable* g_hpower = 0;
ID3D10EffectVectorVariable* g_hemissive = 0;
ID3D10EffectShaderResourceVariable* g_hMeshTexture = 0;

BYTE g_keybuf[256];
BYTE g_savekeybuf[256];

WCHAR g_basedir[ MAX_PATH ] = {0};

float g_ikfirst = 1.0f;
float g_ikrate = 1.0f;
int g_applyrate = 50;
float g_physicsmvrate = 1.0f;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define ID_RMENU_PHYSICSCONSTRAINT	(ID_RMENU_0 - 100)
#define ID_RMENU_MASS0			(ID_RMENU_PHYSICSCONSTRAINT + 1)
#define ID_RMENU_EXCLUDE_MV		(ID_RMENU_PHYSICSCONSTRAINT + 2)

#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_ENABLE_PRESHADER    5
#define IDC_NUM_LIGHTS          6
#define IDC_NUM_LIGHTS_STATIC   7
#define IDC_ACTIVE_LIGHT        8
#define IDC_LIGHT_SCALE         9
#define IDC_LIGHT_SCALE_STATIC  10

#define IDC_COMBO_BONE			11
#define IDC_FK_XP				12
#define IDC_FK_XM				13
#define IDC_FK_YP				14
#define IDC_FK_YM				15
#define IDC_FK_ZP				16
#define IDC_FK_ZM				17

#define IDC_T_XP				18
#define IDC_T_XM				19
#define IDC_T_YP				20
#define IDC_T_YM				21
#define IDC_T_ZP				22
#define IDC_T_ZM				23

#define IDC_SPEED_STATIC		24
#define IDC_SPEED				25

#define IDC_CAMTARGET			26

#define IDC_COMBO_EDITMODE		27
#define IDC_IK_ROT				28
#define IDC_IK_MV				29
#define IDC_IK_LIGHT			30
#define IDC_BT_RIGID			31
#define IDC_BT_IMP				32
#define IDC_BT_GP				33
#define IDC_BT_DAMP				34

#define IDC_COMBO_IKLEVEL		35

#define IDC_LIGHT_DISP			36

#define IDC_SL_IKFIRST			37
#define IDC_SL_IKRATE			38
#define IDC_STATIC_IKFIRST		39
#define IDC_STATIC_IKRATE		40

#define IDC_SLERP_OFF			41
#define IDC_ABS_IK				42

#define IDC_SL_APPLYRATE			43
#define IDC_STATIC_APPLYRATE		44

#define IDC_APPLY_TO_THEEND			45
#define IDC_BMARK					46
#define IDC_PSEUDOLOCAL				47

#define IDC_COMBO_BONEAXIS			48
#define IDC_LIMITDEG				49

#define IDC_STATIC_BTCALCCNT		50
#define IDC_BTCALCCNT				51
#define IDC_STATIC_ERP				52
#define IDC_ERP						53

#define IDC_PHYSICS_IK				54
#define IDC_BTSTART					55
#define IDC_PHYSICS_MV_IK				56
#define IDC_PHYSICS_MV_SLIDER			57
#define IDC_STATIC_PHYSICS_MV_SLIDER	58
//#define IDC_APPLY_BT				59
#define IDC_STOP_BT					60

#define IDC_SL_NUMTHREAD			61
#define IDC_STATIC_NUMTHREAD		62

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
//bool CALLBACK IsDeviceAcceptable( D3DCAPS10* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
//                                  void* pUserContext );
//bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
//HRESULT CALLBACK OnCreateDevice( ID3D10Device* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//                                 void* pUserContext );
//HRESULT CALLBACK OnResetDevice( ID3D10Device* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//                                void* pUserContext );
//void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
//void CALLBACK OnFrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
//LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
//void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
//void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
//void CALLBACK OnLostDevice( void* pUserContext );
//void CALLBACK OnDestroyDevice( void* pUserContext );

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

//bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
//	bool bWindowed, void* pUserContext) {
//	return false;
//};
//HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice,
//	const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
//{
//	return -1;
//};
//HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//	void* pUserContext) 
//{
//	return -1;
//};
//void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
//	void* pUserContext)
//{
//	return;
//};
//void CALLBACK OnD3D9LostDevice(void* pUserContext)
//{
//	return;
//};
//void CALLBACK OnD3D9DestroyDevice(void* pUserContext)
//{
//	return;
//};
bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
	bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice,
	const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
	void* pUserContext);
void CALLBACK OnD3D9LostDevice(void* pUserContext);
void CALLBACK OnD3D9DestroyDevice(void* pUserContext);


bool CALLBACK IsD3D10DeviceAcceptable(UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D10CreateDevice(ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D10ResizedSwapChain(ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D10ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D10DestroyDevice(void* pUserContext);
void CALLBACK OnD3D10FrameRender(ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext);


LRESULT CALLBACK OpenMqoDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MotPropDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OpenBvhDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SaveChaDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ExportXDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK SaveREDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK SaveImpDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK SaveGcoDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK CheckAxisTypeProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK AngleLimitDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK RotAxisDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK CustomRigDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);



void InitApp();
HRESULT LoadMesh( ID3D10Device* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void RenderText( double fTime );


static void OnUserFrameMove(double fTime, float fElapsedTime);
static int RollbackCurBoneNo();

static void AutoCameraTarget();

static int CreateUtDialog();
static int CreateTimelineWnd();
static int CreateLongTimelineWnd();
static int CreateDmpAnimWnd();
static int CreateRigidWnd();
static int CreateImpulseWnd();
static int CreateGPlaneWnd();
static int CreateToolWnd();
static int CreateLayerWnd();

static int OnFrameKeyboard();
static int OnFrameUtCheckBox();
static int OnFramePreviewStop();
static int OnFramePreviewNormal(double* pnextframe, double* pdifftime);
static int OnFramePreviewBt(double* pnextframe, double* pdifftime);
static int OnFramePreviewRagdoll(double* pnextframe, double* pdifftime);
static int OnFrameCloseFlag();
static int OnFrameTimeLineWnd();
static int OnFrameMouseButton();
static int OnFrameToolWnd();
static int OnFramePlayButton();
static int OnFrameUndo();
static int OnFrameUpdateGround();
static int OnFrameInitBtWorld();
static int ToggleRig();
static void UpdateBtSimu(double nextframe, CModel* curmodel);

static int OnRenderSetShaderConst();
static int OnRenderModel();
static int OnRenderGround();
static int OnRenderBoneMark();
static int OnRenderSelect();
static int OnRenderSprite();
static int OnRenderUtDialog(float fElapsedTime);

static int PasteMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe);
static int PasteNotMvParMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe);
static int PasteMotionPointJustInTerm(double copyStartTime, double copyEndTime, double startframe, double endframe);
static int PasteMotionPointAfterCopyEnd(double copyStartTime, double copyEndTime, double startframe, double endframe);

static int ChangeCurrentBone();

static int InitCurMotion(int selectflag, double expandmotion);
static CRigidElem* GetCurRe();
static CRigidElem* GetCurRgdRe();

static int OpenChaFile();
CModel* OpenMQOFile();
CModel* OpenFBXFile( int skipdefref );
static int OpenREFile();
static int OpenImpFile();
static int OpenGcoFile();

int OnDelMotion( int delindex );
int OnAddMotion( int srcmotid );

static int StartBt( int flag, int btcntzero );
static int GetShaderHandle();
static int SetBaseDir();

static int OpenFile();
static int BVH2FBX();
static int SaveProject();
static int SaveREFile();
static int SaveImpFile();
static int SaveGcoFile();
static int ExportFBXFile();

static void refreshTimeline( OWP_Timeline& timeline );
static int AddBoneTra( int kind, float srctra );
static int AddBoneTra2( ChaVector3 diffvec );
static int AddBoneTraPhysics(ChaVector3 diffvec);

static int DispMotionWindow();
static int DispToolWindow();
static int DispObjPanel();
static int DispModelPanel();
static int DispConvBoneWindow();
static int DispAngleLimitDlg();
static int DispRotAxisDlg();
static int DispCustomRigDlg(int rigno);
static int BoneRClick(int srcboneno);

//CustomRigDlg
static int Bone2CustomRig(int rigno);
static int CustomRig2Bone();
static int GetCustomRigRateVal(HWND hDlgWnd, int resid, float* dstptr);
static int CustomRig2Dlg(HWND hDlgWnd);
static int SetCustomRigDlgLevel(HWND hDlgWnd, int levelnum);
static int SetRigRigCombo(HWND hDlgWnd, int elemno);
static int CheckRigRigCombo(HWND hDlgWnd, int elemno);
static int EnableRigAxisUV(HWND hDlgWnd);

//angle limit dlg
static int Bone2AngleLimit();
static int AngleLimit2Bone();
static int AngleLimit2Dlg(HWND hDlgWnd);
static int InitAngleLimitSlider(HWND hDlgWnd, int slresid, int txtresid, int srclimit);
static int GetAngleLimitSliderVal(HWND hDlgWnd, int slresid, int txtresid, int* dstptr);

static int InitRotAxis();
static int RotAxis(HWND hDlgWnd);

static int EraseKeyList();
static int DestroyTimeLine( int dellist );
static int AddTimeLine( int newmotid );
static int AddMotion( WCHAR* wfilename, double srcleng = 0.0 );
static int OnAnimMenu( int selindex, int saveundoflag = 1 );
static int OnRgdMorphMenu( int selindex );
static int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb );
static int OnSetMotSpeed();

static int OnModelMenu( int selindex, int callbymenu );
static int OnREMenu( int selindex, int callbymenu );
static int OnRgdMenu( int selindex, int callbymenu );
static int OnImpMenu( int selindex );
static int OnDelModel( int delindex );
static int OnDelAllModel();
static int refreshModelPanel();
static int RenderSelectMark(int renderflag);
static int RenderSelectFunc();
static int RenderRigMarkFunc();
static int SetSelectState();

static int CreateModelPanel();
static int DestroyModelPanel();
static int CreateConvBoneWnd();
static int DestroyConvBoneWnd();
static int SetConvBoneModel();
static int SetConvBoneBvh();
static int SetConvBone( int cbno );
static int ConvBoneConvert();
static void ConvBoneConvertReq(CBone* modelbone, double srcframe, CBone* befbvhbone, float hrate);
static int ConvBoneRotation(int selfflag, CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate);


static int CalcTargetPos( ChaVector3* dstpos );
static int CalcPickRay( ChaVector3* start3d, ChaVector3* end3d );
static void ActivatePanel( int state );
static int SetCamera6Angle();

static int SetSelectCol();
static int RigidElem2WndParam();
static int SetRigidLeng();
static int SetImpWndParams();
static int SetGpWndParams();
static int SetDmpWndParams();

static int SetSpAxisParams();
static int PickSpAxis( POINT srcpos );
static int SetSpCamParams();
static int PickSpCam(POINT srcpos);
static int SetSpRigParams();
static int PickSpRig(POINT srcpos);
static int SetSpBtParams();
static int PickSpBt(POINT srcpos);


static int InsertCopyMP(CBone* curbone, double curframe);
static void InsertCopyMPReq(CBone* curbone, double curframe);
static int InsertSymMP(CBone* curbone, double curframe, int symrootmode);
static void InsertSymMPReq(CBone* curbone, double curframe, int symrootmode);

static int InitMpFromTool();
static int InitMP( CBone* curbone, double curframe );
static void InitMPReq(CBone* curbone, double curframe);
static int InitMpByEul(int initmode, CBone* curbone, int srcmotid, double srcframe);
static void InitMpByEulReq(int initmode, CBone* curbone, int srcmotid, double srcframe);

static int AdjustBoneTra( CBone* curbone, double frame );
static int CreateTimeLineMark( int topboneno = -1 );
static void CreateMarkReq( int curboneno, int broflag );
static int SetLTimelineMark( int curboneno );
static int SetTimelineMark();

static int ExportBntFile();

static const int s_appindex = 1;


static int s_registflag = 0;
static HKEY s_hkey;
static int RegistKey();
static int IsRegist();


static int OnTimeLineCursor(int mbuttonflag, double newframe);
static int OnTimeLineButtonSelectFromSelectStartEnd(int tothelastflag);
static int OnTimeLineSelectFormSelectedKey();
static int OnTimeLineMButtonDown(bool ctrlshiftflag);
static int OnTimeLineWheel();
static int AddEditRangeHistory();
static int RollBackEditRange(int prevrangeFlag, int nextrangeFlag);
static int RecalcBoneAxisX(CBone* srcbone);

static int GetSymRootMode();





int RegistKey()
{
/*
	CRegistDlg dlg;
	dlg.DoModal();

	if( strcmp( s_appkey[ s_appindex ], dlg.m_regkey ) == 0 ){
		if( s_registflag == 0 ){
			LONG lret;
			DWORD dwret;
			lret = RegCreateKeyExA( HKEY_CURRENT_USER, "Software\\OchakkoLab\\MameBake3D_0003", 0, "",
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &s_hkey, &dwret );
			if( dwret == REG_CREATED_NEW_KEY ){
				lret = RegSetValueExA( s_hkey, "registkey", 0, REG_SZ, (LPBYTE)(dlg.m_regkey), sizeof(char) * 36 );
				if( lret != ERROR_SUCCESS ){
					::MessageBoxA( NULL, "エラー　：　レジストに失敗しました。", "レジストエラー", MB_OK );
				}
				RegCloseKey( s_hkey );
				::MessageBoxA( NULL, "成功　：　レジストに成功しました。", "レジスト成功", MB_OK );
			}else{
				::MessageBoxA( NULL, "エラー　：　レジストに失敗しました。", "レジストエラー", MB_OK );
			}
		}
	}else{
		::MessageBoxA( NULL, "エラー　：　不正なレジストキーです。", "レジストエラー", MB_OK );
	}
*/
	return 0;
}

int IsRegist()
{
	s_registflag = 1;
//	s_registflag = 0;

/*
	LONG lret;
	lret = RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\OchakkoLab\\MameBake3D_0003", 0, KEY_ALL_ACCESS, &s_hkey );
	if( lret == ERROR_SUCCESS ){
		DWORD dwtype;
		char strkey[37] = {0};
		DWORD dwsize = 37;
		lret = RegQueryValueExA( s_hkey, "registkey", NULL, &dwtype, (LPBYTE)strkey, &dwsize );
		if( lret == ERROR_SUCCESS ){
			if( dwtype == REG_SZ ){
				if( strncmp( strkey, s_appkey[ s_appindex ], 36 ) == 0 ){
					s_registflag = 1;
				}
			}
		}
		RegCloseKey( s_hkey );
	}
*/

	return 0;
}



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	OpenDbgFile();
//_CrtSetBreakAlloc(787);
//_CrtSetBreakAlloc(2806);
//_CrtSetBreakAlloc(758);
//_CrtSetBreakAlloc(469193);
//_CrtSetBreakAlloc(5557505);
//_CrtSetBreakAlloc(5557507);
//_CrtSetBreakAlloc(1750750);
//_CrtSetBreakAlloc(602);

//_CrtSetBreakAlloc(65234);
//_CrtSetBreakAlloc(1526483);


	SetBaseDir();
	
	
	s_copyKeyInfoList.clear();	// コピーされたキー情報リスト
	s_copymotmap.clear();
	s_deletedKeyInfoList.clear();	// 削除されたキー情報リスト
	s_selectKeyInfoList.clear();	// コピーされたキー情報リスト


	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);

	//DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	//DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
	//DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
	//DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender);
	//DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
	//DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);
	DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	DXUTSetCallbackD3D9DeviceCreated(NULL);
	DXUTSetCallbackD3D9DeviceReset(NULL);
	DXUTSetCallbackD3D9FrameRender(NULL);
	DXUTSetCallbackD3D9DeviceLost(NULL);
	DXUTSetCallbackD3D9DeviceDestroyed(NULL);


	DXUTSetCallbackD3D10DeviceAcceptable(IsD3D10DeviceAcceptable);
	DXUTSetCallbackD3D10DeviceCreated(OnD3D10CreateDevice);
	DXUTSetCallbackD3D10SwapChainResized(OnD3D10ResizedSwapChain);
	DXUTSetCallbackD3D10FrameRender(OnD3D10FrameRender);
	DXUTSetCallbackD3D10SwapChainReleasing(OnD3D10ReleasingSwapChain);
	DXUTSetCallbackD3D10DeviceDestroyed(OnD3D10DestroyDevice);




	s_doneinit = 0;
    InitApp();
	if (s_doneinit != 1){
		_ASSERT(0);
		return 1;
	}

	//IsRegist();


	s_mainmenu = LoadMenuW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDR_MENU1 ) );
	if( s_mainmenu == NULL ){
		_ASSERT( 0 );
		return 1;
	}



    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
	HRESULT hr;
	// Initialize DXUT and create the desired Win32 window and Direct3D 
	// device for the application. Calling each of these functions is optional, but they
	// allow you to set several options which control the behavior of the framework.
	//DXUTInit(true, true); // Parse the command line and show msgboxes
	//DXUTSetHotkeyHandling(true, true, true);
	//DXUTCreateWindow(L"MameBake3D", 0, 0, s_mainmenu);
	//DXUTCreateDevice(true, s_mainwidth, s_mainheight);
	//s_mainwnd = DXUTGetHWND();
	//_ASSERT(s_mainwnd);
	//ShowWindow(s_mainwnd, SW_SHOW);
	//SetWindowPos(s_mainwnd, HWND_TOP, 450, 0, s_mainwidth, s_mainheight, SWP_NOSIZE);


	//InitApp();
	//DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	//DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
	//DXUTCreateWindow(L"BasicHLSL10");
	//DXUTCreateDevice(true, 640, 480);
	//DXUTMainLoop(); // Enter into the DXUT render loop

	hr = DXUTInit( true, true ); // Parse the command line and show msgboxes
	if (FAILED(hr)) {
		_ASSERT(0);
		return 1;
	}
	// Show the cursor and clip it when in full screen
	//DXUTSetCursorSettings(true, true);
	DXUTSetCursorSettings(false, false);

	//DXUTSetHotkeyHandling( true, true, true );
	DXUTSetHotkeyHandling(false, false, false);

	DXUTSetOverrideSize(s_mainwidth, s_mainheight);
	//GetDXUTState().SetOverrideWidth(s_mainwidth);
	//GetDXUTState().SetOverrideHeight(s_mainwidth);

	HICON appicon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
	hr = DXUTCreateWindow(L"MameBake3D", 0, appicon, s_mainmenu, 450, 0);
	if (FAILED(hr)) {
		_ASSERT(0);
		return 1;
	}

	s_mainwnd = DXUTGetHWND();
	_ASSERT(s_mainwnd);
	RECT clientrect;
	GetClientRect(s_mainwnd, &clientrect);
	s_bufwidth = clientrect.right;
	s_bufheight = clientrect.bottom;

	hr = DXUTCreateDevice(true);
	if (FAILED(hr)) {
		_ASSERT(0);
		return 1;
	}
	s_mainwnd = DXUTGetHWND();
	_ASSERT(s_mainwnd);
	RECT clientrect2;
	GetClientRect(s_mainwnd, &clientrect2);


	//RECT clientrect;
	//GetClientRect(s_mainwnd, &clientrect);
	//s_mainwidth = clientrect.right;
	//s_mainheight = clientrect.bottom;
	//ShowWindow( s_mainwnd, SW_SHOW );
	//SetWindowPos( s_mainwnd, HWND_TOP, 450, 0, s_mainwidth, s_mainheight, SWP_NOSIZE ); 

	//animmenu
	HMENU motmenu;
	motmenu = GetSubMenu( s_mainmenu, 2 );
	s_animmenu = GetSubMenu( motmenu, 3 );
	_ASSERT( s_animmenu );

	HMENU mdlmenu = GetSubMenu( s_mainmenu, 3 );
	s_modelmenu = GetSubMenu( mdlmenu, 3 );
	_ASSERT( s_modelmenu );

	//編集メニュー　4

	s_remenu = GetSubMenu( s_mainmenu, 5 );
	_ASSERT( s_remenu );

	s_rgdmenu = GetSubMenu( s_mainmenu, 6 );
	_ASSERT( s_rgdmenu );

	s_morphmenu = GetSubMenu( s_mainmenu, 7 );
	_ASSERT( s_morphmenu );

	s_impmenu = GetSubMenu( s_mainmenu, 8 );
	_ASSERT( s_impmenu );


	//CallF( InitializeSdkObjects(), return 1 );

	s_psdk = FbxManager::Create();
	if (!s_psdk)
	{
		_ASSERT(0);
		return 1;
	}
	FbxIOSettings * ios = FbxIOSettings::Create(s_psdk, IOSROOT);
	s_psdk->SetIOSettings(ios);
	// Load plugins from the executable directory
	FbxString lPath = FbxGetApplicationDirectory();
#if defined(KARCH_ENV_WIN)
	FbxString lExtension = "dll";
#elif defined(KARCH_ENV_MACOSX)
	FbxString lExtension = "dylib";
#elif defined(KARCH_ENV_LINUX)
	FbxString lExtension = "so";
#endif
	s_psdk->LoadPluginsDirectory(lPath.Buffer(), "dll");



	// Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	s_customrigbone = 0;
	s_customrigdlg = 0;

	s_underselectingframe = 0;
	s_buttonselectstart = 0.0;
	s_buttonselectend = 0.0;

	s_editrangehistoryno = 0;
	s_editrangesetindex = 0;
	s_editrangehistory = new CEditRange[EDITRANGEHISTORYNUM];
	if (!s_editrangehistory){
		_ASSERT(0);
		return;
	}
	int erhno;
	for (erhno = 0; erhno < EDITRANGEHISTORYNUM; erhno++){
		(s_editrangehistory + erhno)->Clear();
	}

	ChaMatrixIdentity(&s_selectmat);
	ChaMatrixIdentity(&s_ikselectmat);

	int cbno;
	for (cbno = 0; cbno < CONVBONEMAX; cbno++){
		s_modelbone[cbno] = 0;
		s_bvhbone[cbno] = 0;
		s_modelbone_bone[cbno] = 0;
		s_bvhbone_bone[cbno] = 0;
	}
	s_convbonemidashi[0] = 0;
	s_convbonemidashi[1] = 0;
	s_convbonemap.clear();


	ZeroMemory(s_spaxis, sizeof( SPAXIS ) * 3);
	ZeroMemory(s_spcam, sizeof(SPCAM) * 3);
	ZeroMemory(s_sprig, sizeof(SPELEM) * 2);
	ZeroMemory(&s_spbt, sizeof(SPELEM));

	g_bonecntmap.clear();

	ChaMatrixIdentity( &s_inimat );

    g_bEnablePreshader = true;

    for( int i = 0; i < MAX_LIGHTS; i++ ){
        g_LightControl[i].SetLightDirection( ChaVector3( sinf( D3DX_PI * 2 * ( MAX_LIGHTS - i - 1 ) / MAX_LIGHTS - D3DX_PI / 6 ),
                                                          0, -cosf( D3DX_PI * 2 * ( MAX_LIGHTS - i - 1 ) / MAX_LIGHTS - D3DX_PI / 6 ) ) );
	}

    g_nActiveLight = 0;
	g_nNumActiveLights = 1;
    g_fLightScale = 1.0f;

	CreateUtDialog();
	CreateTimelineWnd();
	CreateLongTimelineWnd();
	CreateDmpAnimWnd();
	CreateRigidWnd();
	CreateImpulseWnd();
	CreateGPlaneWnd();
	CreateToolWnd();
	CreateLayerWnd();
//////////
	ZeroMemory( &s_pickinfo, sizeof( PICKINFO ) );

	s_modelpanel.panel = 0;
	s_modelpanel.radiobutton = 0;
	s_modelpanel.separator = 0;
	s_modelpanel.checkvec.clear();
	s_modelpanel.modelindex = -1;


	{
		s_bpWorld = new BPWorld(NULL, s_matWorld, "BtPiyo", // ウィンドウのタイトル
			460, 460,         // ウィンドウの幅と高さ [pixels]
			NULL);    // モニタリング用関数へのポインタ  
		_ASSERT(s_bpWorld);


		s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->setTimeStep(0.015);// seconds
		s_bpWorld->setGlobalERP(s_erp);// ERP
		//s_bpWorld->start();// ウィンドウを表示して，シミュレーションを開始する
		s_btWorld = s_bpWorld->getDynamicsWorld();
		s_bpWorld->setNumThread(g_numthread);
	}

	s_doneinit = 1;
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	if (DXUT_D3D9_DEVICE == pDeviceSettings->ver)
	{
		D3DCAPS9 Caps;
		IDirect3D9* pD3D = DXUTGetD3D9Object();
		pD3D->GetDeviceCaps(pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps);

		// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
		// then switch to SWVP.
		if ((Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
			Caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
		{
			pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// Debugging vertex shaders requires either REF or software vertex processing 
		// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
		if (pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF)
		{
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
			pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
#endif
#ifdef DEBUG_PS
		pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
	}
	else
	{
		// Uncomment this to get debug information from D3D10
		//pDeviceSettings->d3d10.CreateFlags |= D3D10_CREATE_DEVICE_DEBUG;
	}

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if ((DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF) ||
			(DXUT_D3D10_DEVICE == pDeviceSettings->ver &&
				pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE))
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}

	return true;
}



//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable(UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
 //   // No fallback defined by this app, so reject any device that 
 //   // doesn't support at least ps2.0
 //   if( pCaps->PixelShaderVersion < D3DPS_VERSION( 3, 0 ) )
 //       return false;

	//if( pCaps->MaxVertexIndex <= 0x0000FFFF ){
	//	return false;
	//}

 //   // Skip backbuffer formats that don't support alpha blending
 //   IDirect3D10* pD3D = DXUTGetD3D10Object();
 //   if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
 //                                        AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
 //                                        D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
 //       return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice(ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;

	s_pdev = pd3dDevice;

	hr = g_DialogResourceManager.OnD3D10CreateDevice(pd3dDevice);
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}
	//hr = g_SettingsDlg.OnD3D10CreateDevice(pd3dDevice);
	//if (FAILED(hr)) {
	//	_ASSERT(0);
	//	return hr;
	//}
	hr = D3DX10CreateFont(pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"MS ゴシック", &g_pFont);
		//L"Arial", &g_pFont10);
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}

	hr = D3DX10CreateSprite(pd3dDevice, 512, &g_pSprite);
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}
	g_pTxtHelper = new CDXUTTextHelper(NULL, NULL, g_pFont, g_pSprite, 15);

	s_totalmb.center = ChaVector3(0.0f, 0.0f, 0.0f);
	s_totalmb.max = ChaVector3(5.0f, 5.0f, 5.0f);
	s_totalmb.min = ChaVector3(-5.0f, -5.0f, -5.0f);
	s_totalmb.r = ChaVector3Length(&s_totalmb.max);
	g_vCenter = s_totalmb.center;
	float fObjectRadius = s_totalmb.r;
	//ChaMatrixTranslation( &g_mCenterWorld, -g_vCenter.x, -g_vCenter.y, -g_vCenter.z );

	hr = CDXUTDirectionWidget::StaticOnD3D10CreateDevice(pd3dDevice);
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}
	for (int i = 0; i < MAX_LIGHTS; i++)
		g_LightControl[i].SetRadius(fObjectRadius);

//	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
//#if defined( DEBUG ) || defined( _DEBUG )
//	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
//	// Setting this flag improves the shader debugging experience, but still allows 
//	// the shaders to be optimized and to run exactly the way they will run in 
//	// the release configuration of this program.
//	dwShaderFlags |= D3D10_SHADER_DEBUG;
//#endif

	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// Preshaders are parts of the shader that the effect system pulls out of the 
	// shader and runs on the host CPU. They should be used if you are GPU limited. 
	// The D3DXSHADER_NO_PRESHADER flag disables preshaders.
	if (!g_bEnablePreshader)
		dwShaderFlags |= D3DXSHADER_NO_PRESHADER;


	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	hr = DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"..\\Media\\Shader\\Ochakko.fx");
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}
	//ID3D10Blob*	l_pBlob_Effect = NULL;
	//ID3D10Blob*	l_pBlob_Errors = NULL;
	//hr = D3DX10CompileEffectFromFile(str, NULL, NULL,
	//	D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
	//	&l_pBlob_Effect, &l_pBlob_Errors);
	//if (FAILED(hr)) {
	//	LPVOID l_pError = NULL;
	//	if (l_pBlob_Errors)
	//	{
	//		l_pError = l_pBlob_Errors->GetBufferPointer();
	//		// then cast to a char* to see it in the locals window
	//	}

	//	LPVOID l_pErrorEffect = NULL;
	//	if (l_pBlob_Effect)
	//	{
	//		l_pErrorEffect = l_pBlob_Effect->GetBufferPointer();
	//		// then cast to a char* to see it in the locals window
	//	}

	//	_ASSERT(0);
	//	return hr;
	//}


	/*
	HRESULT D3DX10CreateEffectFromFile(
	  LPCTSTR pFileName,
	  CONST D3D10_SHADER_MACRO *pDefines,
	  ID3D10Include *pInclude,
	  LPCSTR pProfile,
	  UINT HLSLFlags,
	  UINT FXFlags,
	  ID3D10Device *pDevice,
	  ID3D10EffectPool *pEffectPool,
	  ID3DX10ThreadPump *pPump,
	  ID3D10Effect **ppEffect,
	  ID3D10Blob **ppErrors,
	  HRESULT *pHResult
	);*/

	//D3DX10CreateEffectFromFile(str, NULL, NULL, "fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL,
	//	NULL, &g_pEffect10, NULL, NULL);


	ID3D10Blob*	l_pBlob_Errors = NULL;
	hr = D3DX10CreateEffectFromFile(str, NULL, NULL,
		"fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL, NULL,
		&g_pEffect, &l_pBlob_Errors, NULL);
	if (FAILED(hr)) {
		LPVOID l_pError = NULL;
		if (l_pBlob_Errors)
		{
			l_pError = l_pBlob_Errors->GetBufferPointer();
			// then cast to a char* to see it in the locals window
		}
		_ASSERT(0);
		return hr;
	}


	CallF(GetShaderHandle(), return 1);


	// Setup the camera's view parameters
	//	g_Camera.SetProjParams( D3DX_PI / 4, 1.0f, g_initnear, 4.0f * g_initcamdist );
	g_Camera.SetProjParams(D3DX_PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist);


	ChaVector3 vecEye(0.0f, 0.0f, g_initcamdist);
	ChaVector3 vecAt(0.0f, 0.0f, -0.0f);
	g_Camera.SetViewParams(&vecEye, &vecAt);
	g_Camera.SetRadius(fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 6.0f);


	s_camdist = g_initcamdist;
	g_camEye = ChaVector3(0.0f, fObjectRadius * 0.5f, g_initcamdist);
	g_camtargetpos = ChaVector3(0.0f, fObjectRadius * 0.5f, -0.0f);
	ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

	if (!g_texbank) {
		g_texbank = new CTexBank(s_pdev);
		if (!g_texbank) {
			_ASSERT(0);
			return 0;
		}
	}

	s_select = new CModel();
	if (!s_select) {
		_ASSERT(0);
		return 1;
	}
	CallF(s_select->LoadMQO(s_pdev, L"..\\Media\\MameMedia\\select_2.mqo", 0, 1.0f, 0), return 1);
	CallF(s_select->MakeDispObj(), return 1);

	s_matred = s_select->GetMQOMaterialByName("matred");
	_ASSERT(s_matred);
	s_ringred = s_select->GetMQOMaterialByName("ringred");
	_ASSERT(s_ringred);
	s_matblue = s_select->GetMQOMaterialByName("matblue");
	_ASSERT(s_matblue);
	s_ringblue = s_select->GetMQOMaterialByName("ringblue");
	_ASSERT(s_ringblue);
	s_matgreen = s_select->GetMQOMaterialByName("matgreen");
	_ASSERT(s_matgreen);
	s_ringgreen = s_select->GetMQOMaterialByName("ringgreen");
	_ASSERT(s_ringgreen);
	s_matyellow = s_select->GetMQOMaterialByName("matyellow");
	_ASSERT(s_matyellow);

	s_matredmat = s_matred->GetDif4F();
	s_ringredmat = s_ringred->GetDif4F();
	s_matbluemat = s_matblue->GetDif4F();
	s_ringbluemat = s_ringblue->GetDif4F();
	s_matgreenmat = s_matgreen->GetDif4F();
	s_ringgreenmat = s_ringgreen->GetDif4F();
	s_matyellowmat = s_matyellow->GetDif4F();


	s_rigmark = new CModel();
	if (!s_rigmark) {
		_ASSERT(0);
		return 1;
	}
	CallF(s_rigmark->LoadMQO(s_pdev, L"..\\Media\\MameMedia\\rigmark.mqo", 0, 1.0f, 0), return 1);
	CallF(s_rigmark->MakeDispObj(), return 1);


	s_bmark = new CModel();
	if (!s_bmark) {
		_ASSERT(0);
		return 1;
	}
	CallF(s_bmark->LoadMQO(s_pdev, L"..\\Media\\MameMedia\\bonemark.mqo", 0, 1.0f, 0), return 1);
	CallF(s_bmark->MakeDispObj(), return 1);




	s_ground = new CModel();
	if (!s_ground) {
		_ASSERT(0);
		return 1;
	}
	CallF(s_ground->LoadMQO(s_pdev, L"..\\Media\\MameMedia\\ground2.mqo", 0, 1.0f, 0), return 1);
	CallF(s_ground->MakeDispObj(), return 1);

	s_gplane = new CModel();
	if (!s_gplane) {
		_ASSERT(0);
		return 1;
	}
	CallF(s_gplane->LoadMQO(s_pdev, L"..\\Media\\MameMedia\\gplane.mqo", 0, 1.0f, 0, D3DPOOL_MANAGED), return 1);
	CallF(s_gplane->MakeDispObj(), return 1);
	ChaVector3 tra(0.0f, 0.0, 0.0f);
	ChaVector3 mult(5.0f, 1.0f, 5.0f);
	CallF(s_gplane->MultDispObj(mult, tra), return 1);


	s_bcircle = new CMySprite(s_pdev);
	if (!s_bcircle) {
		_ASSERT(0);
		return 1;
	}

	WCHAR path[MAX_PATH];
	wcscpy_s(path, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(path, TEXT('\\'));
	*lasten = 0L;
	last2en = wcsrchr(path, TEXT('\\'));
	*last2en = 0L;
	wcscat_s(path, MAX_PATH, L"\\Media\\MameMedia\\");
	CallF(s_bcircle->Create(path, L"bonecircle.dds", 0, D3DPOOL_MANAGED, 0), return 1);

	///////
	WCHAR mpath[MAX_PATH];
	wcscpy_s(mpath, MAX_PATH, g_basedir);
	lasten = 0;
	last2en = 0;
	lasten = wcsrchr(mpath, TEXT('\\'));
	*lasten = 0L;
	last2en = wcsrchr(mpath, TEXT('\\'));
	*last2en = 0L;
	wcscat_s(mpath, MAX_PATH, L"\\Media\\MameMedia\\");

	s_spaxis[0].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[0].sprite);
	CallF(s_spaxis[0].sprite->Create(mpath, L"X.png", 0, D3DPOOL_MANAGED, 0), return 1);
	s_spaxis[1].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[1].sprite);
	CallF(s_spaxis[1].sprite->Create(mpath, L"Y.png", 0, D3DPOOL_MANAGED, 0), return 1);
	s_spaxis[2].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[2].sprite);
	CallF(s_spaxis[2].sprite->Create(mpath, L"Z.png", 0, D3DPOOL_MANAGED, 0), return 1);


	s_spcam[SPR_CAM_I].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_I].sprite);
	CallF(s_spcam[SPR_CAM_I].sprite->Create(mpath, L"cam_i.png", 0, D3DPOOL_MANAGED, 0), return 1);
	s_spcam[SPR_CAM_KAI].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_KAI].sprite);
	CallF(s_spcam[SPR_CAM_KAI].sprite->Create(mpath, L"cam_kai.png", 0, D3DPOOL_MANAGED, 0), return 1);
	s_spcam[SPR_CAM_KAKU].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_KAKU].sprite);
	CallF(s_spcam[SPR_CAM_KAKU].sprite->Create(mpath, L"cam_kaku.png", 0, D3DPOOL_MANAGED, 0), return 1);

	s_sprig[0].sprite = new CMySprite(s_pdev);
	_ASSERT(s_sprig[0].sprite);
	CallF(s_sprig[0].sprite->Create(mpath, L"ToggleRig.png", 0, D3DPOOL_MANAGED, 0), return 1);
	s_sprig[1].sprite = new CMySprite(s_pdev);
	_ASSERT(s_sprig[1].sprite);
	CallF(s_sprig[1].sprite->Create(mpath, L"ToggleRigActive.png", 0, D3DPOOL_MANAGED, 0), return 1);

	s_spbt.sprite = new CMySprite(s_pdev);
	_ASSERT(s_spbt.sprite);
	CallF(s_spbt.sprite->Create(mpath, L"BtApply.png", 0, D3DPOOL_MANAGED, 0), return 1);

	///////
	//s_pdev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//s_pdev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	//s_pdev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//s_pdev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//s_pdev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//s_pdev->SetRenderState(D3DRS_ALPHAREF, 0x00);
	//s_pdev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);


	D3D10_DEPTH_STENCIL_DESC dsDescNormal;
	// Depth test parameters
	dsDescNormal.DepthEnable = true;
	dsDescNormal.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	dsDescNormal.DepthFunc = D3D10_COMPARISON_LESS;
	// Stencil test parameters
	dsDescNormal.StencilEnable = true;
	dsDescNormal.StencilReadMask = 0xFF;
	dsDescNormal.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDescNormal.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDescNormal.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
	dsDescNormal.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDescNormal.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	dsDescNormal.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDescNormal.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
	dsDescNormal.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDescNormal.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	// Create depth stencil state
	//ID3D10DepthStencilState * pDSState;
	s_pdev->CreateDepthStencilState(&dsDescNormal, &g_pDSStateZCmp);


	D3D10_DEPTH_STENCIL_DESC dsDescZCmpAlways;
	// Depth test parameters
	dsDescZCmpAlways.DepthEnable = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	dsDescZCmpAlways.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	dsDescZCmpAlways.DepthFunc = D3D10_COMPARISON_LESS;
	// Stencil test parameters
	dsDescZCmpAlways.StencilEnable = true;
	dsDescZCmpAlways.StencilReadMask = 0xFF;
	dsDescZCmpAlways.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDescZCmpAlways.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDescZCmpAlways.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
	dsDescZCmpAlways.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDescZCmpAlways.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	dsDescZCmpAlways.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDescZCmpAlways.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
	dsDescZCmpAlways.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDescZCmpAlways.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	// Create depth stencil state
	//ID3D10DepthStencilState * pDSState;
	s_pdev->CreateDepthStencilState(&dsDescZCmpAlways, &g_pDSStateZCmpAlways);


	s_pdev->OMSetDepthStencilState(g_pDSStateZCmp, 1);


	return S_OK;
}


////--------------------------------------------------------------------------------------
//// This callback function will be called immediately after the Direct3D device has been 
//// reset, which will happen after a lost device scenario. This is the best location to 
//// create 0 resources since these resources need to be reloaded whenever 
//// the device is lost. Resources created here should be released in the OnLostDevice 
//// callback. 
////--------------------------------------------------------------------------------------
//HRESULT CALLBACK OnResetDevice( ID3D10Device* pd3dDevice,
//                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
//{
//    HRESULT hr;
//
//    V_RETURN( g_DialogResourceManager.OnD3D10ResetDevice() );
//    V_RETURN( g_SettingsDlg.OnD3D10ResetDevice() );
//
//    // Create a sprite to help batch calls when drawing many lines of text
//    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite ) );
//
//    if( g_pFont )
//        V_RETURN( g_pFont->OnResetDevice() );
//    if( g_pEffect )
//        V_RETURN( g_pEffect->OnResetDevice() );
//
//
//	if( g_texbank ){
//		CallF( g_texbank->Invalidate( INVAL_ONLYDEFAULT ), return 1 );
//		CallF( g_texbank->Restore(), return 1 );
//	}
//
//    for( int i = 0; i < MAX_LIGHTS; i++ )
//        g_LightControl[i].OnD3D10ResetDevice( pBackBufferSurfaceDesc );
//
//    // Setup the camera's projection parameters
//    s_fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
////    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, g_initnear, 4.0f * g_initcamdist );
//	g_Camera.SetProjParams( D3DX_PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
//
//    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
//	g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );
//
//	s_mainwidth = pBackBufferSurfaceDesc->Width;
//	s_mainheight = pBackBufferSurfaceDesc->Height;
//
//    //g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 550 );
//	//g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
//    //g_SampleUI.SetSize( 170, 750 );
//
//	g_SampleUI.SetLocation( 0, 0 );
//	g_SampleUI.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
//	
//	SetSpAxisParams();
//	SetSpCamParams();
//	SetSpRigParams();
//	SetSpBtParams();
//
//    return S_OK;
//}

//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10ResizedSwapChain(ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D10ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	//V_RETURN(g_SettingsDlg.OnD3D10ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	s_fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
//    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, g_initnear, 4.0f * g_initcamdist );
	g_Camera.SetProjParams( D3DX_PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
	
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );
	
	s_mainwidth = pBackBufferSurfaceDesc->Width;
	s_mainheight = pBackBufferSurfaceDesc->Height;
	
	
	g_SampleUI.SetLocation(0, 0);
	g_SampleUI.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);


	SetSpAxisParams();
	SetSpCamParams();
	SetSpRigParams();
	SetSpBtParams();
	
	//g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	//g_HUD.SetSize(170, 170);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D10ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice(void* pUserContext)
{
	if (g_pDSStateZCmp) {
		g_pDSStateZCmp->Release();
		g_pDSStateZCmp = 0;
	}
	if (g_pDSStateZCmpAlways) {
		g_pDSStateZCmpAlways->Release();
		g_pDSStateZCmpAlways = 0;
	}

	g_DialogResourceManager.OnD3D10DestroyDevice();
	//g_SettingsDlg.OnD3D10DestroyDevice();
	CDXUTDirectionWidget::StaticOnD3D10DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	SAFE_DELETE(g_pTxtHelper);
	SAFE_RELEASE(g_pFont);
	SAFE_RELEASE(g_pSprite);
	SAFE_RELEASE(g_pEffect);
	//SAFE_RELEASE(g_pVertexLayout);
	//g_Mesh10.Destroy();

	if (s_editrangehistory) {
		delete[] s_editrangehistory;
		s_editrangehistory = 0;
	}
	s_editrangehistoryno = 0;

	if (s_anglelimitdlg) {
		DestroyWindow(s_anglelimitdlg);
		s_anglelimitdlg = 0;
	}
	if (s_rotaxisdlg) {
		DestroyWindow(s_rotaxisdlg);
		s_rotaxisdlg = 0;
	}
	if (s_customrigdlg) {
		DestroyWindow(s_customrigdlg);
		s_customrigdlg = 0;
	}
	s_oprigflag = 0;
	s_customrigbone = 0;

	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel) {
			//FbxScene* pscene = curmodel->GetScene();
			//if (pscene){
			//	pscene->Destroy();
			//}

			delete curmodel;
		}
	}
	s_modelindex.clear();
	s_model = 0;

	if (s_select) {
		delete s_select;
		s_select = 0;
	}
	if (s_rigmark) {
		delete s_rigmark;
		s_rigmark = 0;
	}
	if (s_ground) {
		delete s_ground;
		s_ground = 0;
	}
	if (s_gplane) {
		delete s_gplane;
		s_gplane = 0;
	}
	if (s_bmark) {
		delete s_bmark;
		s_bmark = 0;
	}


	if (s_bcircle) {
		delete s_bcircle;
		s_bcircle = 0;
	}
	if (s_kinsprite) {
		delete s_kinsprite;
		s_kinsprite = 0;
	}

	if (g_texbank) {
		delete g_texbank;
		g_texbank = 0;
	}

	if (s_mainmenu) {
		DestroyMenu(s_mainmenu);
		s_mainmenu = 0;
	}

	DestroyTimeLine(1);

	if (s_timelineWnd) {
		delete s_timelineWnd;
		s_timelineWnd = 0;
	}
	if (s_LtimelineWnd) {
		delete s_LtimelineWnd;
		s_LtimelineWnd = 0;
	}

	if (s_toolWnd) {
		delete s_toolWnd;
		s_toolWnd = 0;
	}

	if (s_toolCopyB) {
		delete s_toolCopyB;
		s_toolCopyB = 0;
	}
	if (s_toolSymCopyB) {
		delete s_toolSymCopyB;
		s_toolSymCopyB = 0;
	}
	if (s_toolCutB) {
		delete s_toolCutB;
		s_toolCutB = 0;
	}
	if (s_toolPasteB) {
		delete s_toolPasteB;
		s_toolPasteB = 0;
	}
	if (s_toolDeleteB) {
		delete s_toolDeleteB;
		s_toolDeleteB = 0;
	}
	if (s_toolMotPropB) {
		delete s_toolMotPropB;
		s_toolMotPropB = 0;
	}
	if (s_toolMarkB) {
		delete s_toolMarkB;
		s_toolMarkB = 0;
	}
	if (s_toolSelBoneB) {
		delete s_toolSelBoneB;
		s_toolSelBoneB = 0;
	}
	if (s_toolInitMPB) {
		delete s_toolInitMPB;
		s_toolInitMPB = 0;
	}
	if (s_toolFilterB) {
		delete s_toolFilterB;
		s_toolFilterB = 0;
	}
	if (s_toolInterpolateB) {
		delete s_toolInterpolateB;
		s_toolInterpolateB = 0;
	}

	if (s_owpTimeline) {
		delete s_owpTimeline;
		s_owpTimeline = 0;
	}
	if (s_owpPlayerButton) {
		delete s_owpPlayerButton;
		s_owpPlayerButton = 0;
	}
	if (s_owpLTimeline) {
		delete s_owpLTimeline;
		s_owpLTimeline = 0;
	}

	if (s_layerWnd) {
		delete s_layerWnd;
		s_layerWnd = 0;
	}
	if (s_owpLayerTable) {
		delete s_owpLayerTable;
		s_owpLayerTable = 0;
	}

	if (s_shprateSlider) {
		delete s_shprateSlider;
		s_shprateSlider = 0;
	}
	if (s_boxzSlider) {
		delete s_boxzSlider;
		s_boxzSlider = 0;
	}
	if (s_boxzlabel) {
		delete s_boxzlabel;
		s_boxzlabel = 0;
	}
	if (s_massSlider) {
		delete s_massSlider;
		s_massSlider = 0;
	}
	if (s_massSLlabel) {
		delete s_massSLlabel;
		s_massSLlabel = 0;
	}
	if (s_massB) {
		delete s_massB;
		s_massB = 0;
	}
	if (s_namelabel) {
		delete s_namelabel;
		s_namelabel = 0;
	}
	if (s_lenglabel) {
		delete s_lenglabel;
		s_lenglabel = 0;
	}
	if (s_rigidskip) {
		delete s_rigidskip;
		s_rigidskip = 0;
	}
	if (s_forbidrot) {
		delete s_forbidrot;
		s_forbidrot = 0;
	}

	if (s_allrigidenableB) {
		delete s_allrigidenableB;
		s_allrigidenableB = 0;
	}
	if (s_allrigiddisableB) {
		delete s_allrigiddisableB;
		s_allrigiddisableB = 0;
	}


	if (s_shplabel) {
		delete s_shplabel;
		s_shplabel = 0;
	}
	if (s_colradio) {
		delete s_colradio;
		s_colradio = 0;
	}
	if (s_lkradio) {
		delete s_lkradio;
		s_lkradio = 0;
	}
	if (s_lkSlider) {
		delete s_lkSlider;
		s_lkSlider = 0;
	}
	if (s_lklabel) {
		delete s_lklabel;
		s_lklabel = 0;
	}
	if (s_akradio) {
		delete s_akradio;
		s_akradio = 0;
	}
	if (s_akSlider) {
		delete s_akSlider;
		s_akSlider = 0;
	}
	if (s_aklabel) {
		delete s_aklabel;
		s_aklabel = 0;
	}
	if (s_restSlider) {
		delete s_restSlider;
		s_restSlider = 0;
	}
	if (s_restlabel) {
		delete s_restlabel;
		s_restlabel = 0;
	}
	if (s_fricSlider) {
		delete s_fricSlider;
		s_fricSlider = 0;
	}
	if (s_friclabel) {
		delete s_friclabel;
		s_friclabel = 0;
	}
	if (s_ldmpSlider) {
		delete s_ldmpSlider;
		s_ldmpSlider = 0;
	}
	if (s_admpSlider) {
		delete s_admpSlider;
		s_admpSlider = 0;
	}
	if (s_kB) {
		delete s_kB;
		s_kB = 0;
	}
	if (s_restB) {
		delete s_restB;
		s_restB = 0;
	}
	if (s_ldmplabel) {
		delete s_ldmplabel;
		s_ldmplabel = 0;
	}
	if (s_admplabel) {
		delete s_admplabel;
		s_admplabel = 0;
	}
	if (s_dmpB) {
		delete s_dmpB;
		s_dmpB = 0;
	}
	if (s_groupB) {
		delete s_groupB;
		s_groupB = 0;
	}
	if (s_gcoliB) {
		delete s_gcoliB;
		s_gcoliB = 0;
	}
	if (s_btgSlider) {
		delete s_btgSlider;
		s_btgSlider = 0;
	}
	if (s_btglabel) {
		delete s_btglabel;
		s_btglabel = 0;
	}
	if (s_btgscSlider) {
		delete s_btgscSlider;
		s_btgscSlider = 0;
	}
	if (s_btgsclabel) {
		delete s_btgsclabel;
		s_btgsclabel = 0;
	}
	if (s_btforce) {
		delete s_btforce;
		s_btforce = 0;
	}
	if (s_groupcheck) {
		delete s_groupcheck;
		s_groupcheck = 0;
	}
	if (s_btgB) {
		delete s_btgB;
		s_btgB = 0;
	}
	if (s_rigidWnd) {
		delete s_rigidWnd;
		s_rigidWnd = 0;
	}

	if (s_dmpgroupcheck) {
		delete s_dmpgroupcheck;
		s_dmpgroupcheck = 0;
	}
	if (s_dmpanimLlabel) {
		delete s_dmpanimLlabel;
		s_dmpanimLlabel = 0;
	}
	if (s_dmpanimLSlider) {
		delete s_dmpanimLSlider;
		s_dmpanimLSlider = 0;
	}
	if (s_dmpanimAlabel) {
		delete s_dmpanimAlabel;
		s_dmpanimAlabel = 0;
	}
	if (s_dmpanimASlider) {
		delete s_dmpanimASlider;
		s_dmpanimASlider = 0;
	}
	if (s_dmpanimB) {
		delete s_dmpanimB;
		s_dmpanimB = 0;
	}
	if (s_dmpanimWnd) {
		delete s_dmpanimWnd;
		s_dmpanimWnd = 0;
	}


	if (s_impgroupcheck) {
		delete s_impgroupcheck;
		s_impgroupcheck = 0;
	}
	if (s_impzSlider) {
		delete s_impzSlider;
		s_impzSlider = 0;
	}
	if (s_impySlider) {
		delete s_impySlider;
		s_impySlider = 0;
	}
	if (s_impxSlider) {
		delete s_impxSlider;
		s_impxSlider = 0;
	}
	if (s_impzlabel) {
		delete s_impzlabel;
		s_impzlabel = 0;
	}
	if (s_impylabel) {
		delete s_impylabel;
		s_impylabel = 0;
	}
	if (s_impxlabel) {
		delete s_impxlabel;
		s_impxlabel = 0;
	}
	if (s_impscaleSlider) {
		delete s_impscaleSlider;
		s_impscaleSlider = 0;
	}
	if (s_impscalelabel) {
		delete s_impscalelabel;
		s_impscalelabel = 0;
	}
	if (s_impallB) {
		delete s_impallB;
		s_impallB = 0;
	}
	if (s_impWnd) {
		delete s_impWnd;
		s_impWnd = 0;
	}
	if (s_gpWnd) {
		delete s_gpWnd;
		s_gpWnd = 0;
	}
	if (s_ghSlider) {
		delete s_ghSlider;
		s_ghSlider = 0;
	}
	if (s_gsizexSlider) {
		delete s_gsizexSlider;
		s_gsizexSlider = 0;
	}
	if (s_gsizezSlider) {
		delete s_gsizezSlider;
		s_gsizezSlider = 0;
	}
	if (s_ghlabel) {
		delete s_ghlabel;
		s_ghlabel = 0;
	}
	if (s_gsizexlabel) {
		delete s_gsizexlabel;
		s_gsizexlabel = 0;
	}
	if (s_gsizezlabel) {
		delete s_gsizezlabel;
		s_gsizezlabel = 0;
	}
	if (s_gpdisp) {
		delete s_gpdisp;
		s_gpdisp = 0;
	}
	if (s_grestSlider) {
		delete s_grestSlider;
		s_grestSlider = 0;
	}
	if (s_grestlabel) {
		delete s_grestlabel;
		s_grestlabel = 0;
	}
	if (s_gfricSlider) {
		delete s_gfricSlider;
		s_gfricSlider = 0;
	}
	if (s_gfriclabel) {
		delete s_gfriclabel;
		s_gfriclabel = 0;
	}

	//static SPAXIS s_spaxis[3];
	int spno;
	for (spno = 0; spno < 3; spno++) {
		CMySprite* cursp = s_spaxis[spno].sprite;
		if (cursp) {
			delete cursp;
		}
		s_spaxis[spno].sprite = 0;
	}

	int spcno;
	for (spcno = 0; spcno < 3; spcno++) {
		CMySprite* curspc = s_spcam[spcno].sprite;
		if (curspc) {
			delete curspc;
		}
		s_spcam[spcno].sprite = 0;
	}


	int sprno;
	for (sprno = 0; sprno < 2; sprno++) {
		CMySprite* cursp = s_sprig[sprno].sprite;
		if (cursp) {
			delete cursp;
		}
		s_sprig[sprno].sprite = 0;
	}

	CMySprite* cursp = s_spbt.sprite;
	if (cursp) {
		delete cursp;
	}
	s_spbt.sprite = 0;



	DestroyModelPanel();
	DestroyConvBoneWnd();

	//DestroySdkObjects();

	if (s_psdk) {
		s_psdk->Destroy();
		s_psdk = 0;
	}

	if (s_bpWorld) {
		delete s_bpWorld;
		s_bpWorld = 0;
	}


}



//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	int callfromik = 0;
	OnUserFrameMove(fTime, fElapsedTime);
}

void OnUserFrameMove(double fTime, float fElapsedTime)
{

	static double savetime = 0.0;
	static int capcnt = 0;

	WCHAR sz[100];
	swprintf_s(sz, 100, L"スレッド数 : %d(%d) 個", g_numthread, gNumIslands);
	g_SampleUI.GetStatic(IDC_STATIC_NUMTHREAD)->SetText(sz);


	OnFrameUtCheckBox();
	SetCamera6Angle();
	AutoCameraTarget();

	OnFrameKeyboard();
	OnFrameTimeLineWnd();
	OnFramePlayButton();
	OnFrameMouseButton();

	s_time = fTime;
	g_Camera.FrameMove(fElapsedTime);
	double difftime = fTime - savetime;

	s_matWorld = ChaMatrix(*g_Camera.GetWorldMatrix());
	s_matProj = ChaMatrix(*g_Camera.GetProjMatrix());
	s_matWorld._41 = 0.0f;
	s_matWorld._42 = 0.0f;
	s_matWorld._43 = 0.0f;
	//s_matW = s_matWorld;
	s_matVP = s_matView * s_matProj;

	int modelno;
	int modelnum = (int)s_modelindex.size();
	for (modelno = 0; modelno < modelnum; modelno++){
		s_modelindex[modelno].modelptr->SetWorldMatFromCamera(s_matWorld);
	}

	double nextframe = 0.0;
	if (g_previewFlag){
		if (s_model && s_model->GetCurMotInfo()){
			if (g_previewFlag <= 3){
				OnFramePreviewNormal(&nextframe, &difftime);
			}
			else if (g_previewFlag == 4){//BTの物理
				OnFramePreviewBt(&nextframe, &difftime);
			}
			else if (g_previewFlag == 5){//ラグドール
				OnFramePreviewRagdoll(&nextframe, &difftime);
			}
			OnTimeLineCursor(0, 0.0);
		}
		else{
			g_previewFlag = 0;
		}
	}
	else{
		OnFramePreviewStop();
	}
	s_difftime = difftime;
	savetime = fTime;

	OnFrameCloseFlag();
	OnFrameToolWnd();
	OnFrameUndo();

	OnFrameUpdateGround();
	OnFrameInitBtWorld();


	s_savepreviewFlag = g_previewFlag;
}

void InsertCopyMPReq(CBone* curbone, double curframe)
{
	if (curbone){
		InsertCopyMP(curbone, curframe);

		if (curbone->GetChild()){
			InsertCopyMPReq(curbone->GetChild(), curframe);
		}
		if (curbone->GetBrother()){
			InsertCopyMPReq(curbone->GetBrother(), curframe);
		}
	}
}

int InsertCopyMP( CBone* curbone, double curframe )
{
	/*
	CMotionPoint* pcurmp = 0;
	pcurmp = curbone->GetMotionPoint(s_model->GetCurMotInfo()->motid, curframe);
	if(pcurmp){
		CPELEM cpelem;
		ZeroMemory(&cpelem, sizeof(CPELEM));
		cpelem.bone = curbone;
		cpelem.mp.SetFrame(curframe);
		cpelem.mp.SetWorldMat(pcurmp->GetWorldMat());
		cpelem.mp.SetLocalMatFlag(0);//!!!!!!!!!!
		s_copymotmap.push_back(cpelem);
	}
	*/
	int rotcenterflag1 = 1;
	ChaMatrix localmat = curbone->CalcLocalRotMat(rotcenterflag1, s_model->GetCurMotInfo()->motid, curframe);
	ChaVector3 curanimtra = curbone->CalcLocalTraAnim(s_model->GetCurMotInfo()->motid, curframe);
	localmat._41 += curanimtra.x;
	localmat._42 += curanimtra.y;
	localmat._43 += curanimtra.z;

	CPELEM cpelem;
	ZeroMemory(&cpelem, sizeof(CPELEM));
	cpelem.bone = curbone;
	cpelem.mp.SetFrame(curframe);
	cpelem.mp.SetWorldMat(localmat);
	cpelem.mp.SetLocalMatFlag(1);//!!!!!!!!!!
	s_copymotmap.push_back(cpelem);

	return 0;
}

void InsertSymMPReq(CBone* curbone, double curframe, int symrootmode)
{
	if (curbone){
		InsertSymMP(curbone, curframe, symrootmode);

		if (curbone->GetChild()){
			InsertSymMPReq(curbone->GetChild(), curframe, symrootmode);
		}
		if (curbone->GetBrother()){
			InsertSymMPReq(curbone->GetBrother(), curframe, symrootmode);
		}
	}
}
int InsertSymMP(CBone* curbone, double curframe, int symrootmode)
{
	ChaMatrix symmat = curbone->CalcSymXMat2(s_model->GetCurMotInfo()->motid, curframe, symrootmode);

	CPELEM cpelem;
	ZeroMemory(&cpelem, sizeof(CPELEM));
	cpelem.bone = curbone;
	cpelem.mp.SetFrame(curframe);
	cpelem.mp.SetWorldMat(symmat);
	cpelem.mp.SetLocalMatFlag(1);//!!!!!!!!!!
	s_copymotmap.push_back(cpelem);

	return 0;
}

int InitMP( CBone* curbone, double curframe )
{
	CMotionPoint* pcurmp = 0;
	pcurmp = curbone->GetMotionPoint(s_model->GetCurMotInfo()->motid, curframe);

	if(pcurmp){

		//pcurmp->SetBefWorldMat(pcurmp->GetWorldMat());

		ChaMatrix xmat = curbone->GetFirstMat();
		pcurmp->SetWorldMat(xmat);
		curbone->SetInitMat(xmat);

	}else{
		CMotionPoint* curmp3 = 0;
		int existflag3 = 0;
		curmp3 = curbone->AddMotionPoint(s_model->GetCurMotInfo()->motid, curframe, &existflag3);
		if (!curmp3){
			_ASSERT(0);
			return 1;
		}
		ChaMatrix xmat = curbone->GetFirstMat();
		curmp3->SetWorldMat(xmat);
		curbone->SetInitMat(xmat);
		//_ASSERT( 0 );
	}

	//オイラー角初期化
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	int paraxsiflag = 1;
	int isfirstbone = 0;
	cureul = curbone->CalcLocalEulZXY(paraxsiflag, s_model->GetCurMotInfo()->motid, curframe, BEFEUL_ZERO, isfirstbone);
	curbone->SetLocalEul(s_model->GetCurMotInfo()->motid, curframe, cureul);

	return 0;
}

int AdjustBoneTra( CBone* curbone, double curframe )
{
	CMotionPoint* pcurmp = 0;
	pcurmp = curbone->GetMotionPoint(s_model->GetCurMotInfo()->motid, curframe);
	if(pcurmp){
		ChaVector3 orgpos;
		ChaVector3TransformCoord( &orgpos, &(curbone->GetJointFPos()), &(pcurmp->GetBefWorldMat()) );

		ChaVector3 newpos;
		ChaVector3TransformCoord( &newpos, &(curbone->GetJointFPos()), &(pcurmp->GetWorldMat()) );

		ChaVector3 diffpos;
		diffpos = orgpos - newpos;

		CEditRange tmper;
		KeyInfo tmpki;
		tmpki.time = curframe;
		list<KeyInfo> tmplist;
		tmplist.push_back( tmpki );
		tmper.SetRange( tmplist, curframe );
		s_model->FKBoneTra( 0, &tmper, curbone->GetBoneNo(), diffpos );
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    //if( g_SettingsDlg.IsActive() )
    //{
    //    g_SettingsDlg.OnRender( fElapsedTime );
    //    return;
    //}

    //// Clear the render target and the zbuffer 
    //V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
    //                      0 ) );


	// Clear the render target and depth stencil
	float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };
	ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
	pd3dDevice->ClearRenderTargetView(pRTV, ClearColor);
	ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
	pd3dDevice->ClearDepthStencilView(pDSV, D3D10_CLEAR_DEPTH, 1.0, 0);




	ChaMatrix mWorldViewProjection;
	ChaVector3 vLightDir[MAX_LIGHTS];
	ChaVector4 vLightDiffuse[MAX_LIGHTS];
	ChaMatrix mWorld;
	ChaMatrix mView;
	ChaMatrix mProj;

	// Get the projection & view matrix from the camera class
//	mWorld = g_mCenterMesh * *g_Camera.GetWorldMatrix();
	mWorld = *g_Camera.GetWorldMatrix();
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();

	mWorldViewProjection = mWorld * mView * mProj;

	// Render the light arrow so the user can visually see the light dir
	for (int i = 0; i < g_nNumActiveLights; i++)
	{
		D3DXCOLOR arrowColor = (i == g_nActiveLight) ? D3DXCOLOR(1, 1, 0, 1) : D3DXCOLOR(1, 1, 1, 1);
		//V(g_LightControl[i].OnRender10(arrowColor, &mView, &mProj, g_Camera.GetEyePt()));
		vLightDir[i] = g_LightControl[i].GetLightDirection();
		vLightDiffuse[i] = ChaVector4(1, 1, 1, 1) * g_fLightScale;
	}

	//V(g_pLightDir->SetRawValue(vLightDir, 0, sizeof(ChaVector3) * MAX_LIGHTS));
	//V(g_pLightDiffuse->SetFloatVectorArray((float*)vLightDiffuse, 0, MAX_LIGHTS));
	//V(g_pmWorldViewProjection->SetMatrix((float*)&mWorldViewProjection));
	//V(g_pmWorld->SetMatrix((float*)&mWorld));
	//V(g_pfTime->SetFloat((float)fTime));
	//V(g_pnNumLights->SetInt(g_nNumActiveLights));

	//// Render the scene with this technique as defined in the .fx file
	//ID3D10EffectTechnique* pRenderTechnique;
	//switch (g_nNumActiveLights)
	//{
	//case 1:
	//	pRenderTechnique = g_pTechRenderSceneWithTexture1Light;
	//	break;
	//case 2:
	//	pRenderTechnique = g_pTechRenderSceneWithTexture2Light;
	//	break;
	//case 3:
	//	pRenderTechnique = g_pTechRenderSceneWithTexture3Light;
	//	break;
	//default:
	//	pRenderTechnique = g_pTechRenderSceneWithTexture1Light;
	//	break;
	//}



	//s_pdev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	//s_pdev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	//s_pdev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	OnRenderSetShaderConst();

	OnRenderModel();
	OnRenderGround();
	OnRenderBoneMark();
	OnRenderSelect();
	//OnRenderUtDialog(fElapsedTime);
	if (s_dispsampleui) {//ctrl + 1 (one) key --> toggle
		OnRenderSprite();
	}

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	////g_HUD.OnRender(fElapsedTime);
	if (s_dispsampleui) {//ctrl + 1 (one) key --> toggle
		g_SampleUI.OnRender(fElapsedTime);
	}
	RenderText(fTime);
	DXUT_EndPerfEvent();

}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DX10Font interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText( double fTime )
{
	static double s_savetime = 0.0;
	double g_calcfps;
	if (fTime != s_savetime) {
		g_calcfps = 1.0 / (fTime - s_savetime);
	}
	else {
		g_calcfps = 100.0;
	}

    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work fine however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves perf.
    //CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );
	//CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 18 );

    // Output statistics
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    //g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    //g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    g_pTxtHelper->DrawFormattedTextLine( L"fps : %0.2f fTime: %0.1f, preview %d, btcanccnt %.1f, ERP %.5f", g_calcfps, fTime, g_previewFlag, g_btcalccnt, s_erp );

	//int tmpnum;
	//double tmpstart, tmpend, tmpapply;
	//s_editrange.GetRange( &tmpnum, &tmpstart, &tmpend, &tmpapply );
    //g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    //g_pTxtHelper->DrawFormattedTextLine( L"Select Frame : selnum %d, start %.3f, end %.3f, apply %.3f", tmpnum, tmpstart, tmpend, tmpapply );

	//if (s_model && (s_curboneno >= 0)){
	//	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	//	if (curbone){
	//		MOTINFO* curmi = s_model->GetCurMotInfo();
	//		if (curmi){
	//			const WCHAR* wbonename = curbone->GetWBoneName();
	//			ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//			int paraxsiflag = 1;
	//			int isfirstbone = 0;
	//			cureul = curbone->CalcLocalEulZXY(paraxsiflag, curmi->motid, curmi->curframe, BEFEUL_ZERO, isfirstbone);
	//			ChaVector3 curtra = ChaVector3(0.0f, 0.0f, 0.0f);
	//			curtra = curbone->CalcLocalTraAnim(curmi->motid, curmi->curframe);
	//			//curbone->SetLocalEul(curmi->motid, curmi->curframe, cureul);
	//			g_pTxtHelper->DrawFormattedTextLine(L"selected bone : %s", wbonename);
	//			g_pTxtHelper->DrawFormattedTextLine(L"selected bone eul : (%.6f, %.6f, %.6f)",cureul.x, cureul.y, cureul.z);
	//			g_pTxtHelper->DrawFormattedTextLine(L"selected bone tra : (%.6f, %.6f, %.6f)", curtra.x, curtra.y, curtra.z);
	//		}
	//	}
	//}

/***
    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D10BackBufferSurfaceDesc();
        g_pTxtHelper->SetInsertionPos( 2, pd3dsdBackBuffer->Height - 15 * 6 );
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Controls:" );

        g_pTxtHelper->SetInsertionPos( 20, pd3dsdBackBuffer->Height - 15 * 5 );
        g_pTxtHelper->DrawTextLine( L"Rotate model: Left mouse button\n"
                                L"Rotate light: Right mouse button\n"
                                L"Rotate camera: Middle mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n" );

        g_pTxtHelper->SetInsertionPos( 250, pd3dsdBackBuffer->Height - 15 * 5 );
        g_pTxtHelper->DrawTextLine( L"Hide help: F1\n"
                                L"Quit: ESC\n" );
    }
    else
    {
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Press F1 for help" );
    }
***/
    g_pTxtHelper->End();


	s_savetime = fTime;
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
//	DbgOut( L"msgproc!!! %d, %d\r\n", uMsg, WM_LBUTTONDOWN );
	//if(s_anglelimitdlg && IsDialogMessage(s_anglelimitdlg, &msg))

	// Always allow dialog resource manager calls to handle global messages
	// so GUI state is updated correctly
	//g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) {
		//_ASSERT(0);
		return 0;
	}

	//if( g_SettingsDlg.IsActive() )
	//{
	//    g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
	//    //return 0;
	//}

	// Give the dialogs a chance to handle the message first
	g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) {
		//_ASSERT(0);
		return 0;
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	//g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);


	if( uMsg == WM_COMMAND ){

		WORD menuid;
		menuid = LOWORD( wParam );

		if( (menuid >= 59900) && (menuid <= (59900 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 59900;
			OnAnimMenu( selindex );
			ActivatePanel( 1 );
			return 0;
		}else if( (menuid >= 61000) && (menuid <= (61000 + MAXMODELNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 61000;
			OnModelMenu( selindex, 1 );
			ActivatePanel( 1 );
			return 0;
		}else if( (menuid >= 62000) && (menuid <= (62000 + MAXRENUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 62000;
			OnREMenu( selindex, 1 );
			ActivatePanel( 1 );
			return 0;
		}else if( (menuid >= 63000) && (menuid <= (63000 + MAXRENUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 63000;
			OnRgdMenu( selindex, 1 );
			ActivatePanel( 1 );
			return 0;
		}else if( (menuid >= 64000) && (menuid <= (64000 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 64000;
			OnRgdMorphMenu( selindex );
			ActivatePanel( 1 );
			return 0;
		}else if( (menuid >= 64500) && (menuid <= (64500 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 64500;
			OnImpMenu( selindex );
			ActivatePanel( 1 );
			return 0;
		}else{
			switch( menuid ){
			case ID_40047:
				// "編集・変換"
				// "ボーン軸をZに再計算"
				ActivatePanel(0);
				RecalcBoneAxisX(0);
				ActivatePanel(1);
				return 0;
				break;
			case 55544:
				ActivatePanel( 0 );
				RegistKey();
				ActivatePanel( 1 );
				return 0;
				break;
			case ID_FILE_EXPORTBNT:
				ActivatePanel( 0 );
				ExportBntFile();
				ActivatePanel( 1 );
				return 0;
				break;
			case ID_FILE_OPEN40001:
				ActivatePanel( 0 );
				OpenFile();
				ActivatePanel( 1 );
				return 0;
				break;
			case ID_FILE_BVH2FBX:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					BVH2FBX();
					ActivatePanel( 1 );
				}
				return 0;
				break;
			case ID_SAVE_FBX40039:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					ExportFBXFile();
					ActivatePanel( 1 );
				}
				break;
			case ID_SAVEPROJ_40035:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveProject();
					ActivatePanel( 1 );
				}
				break;
			case ID_RESAVE_40028:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveREFile();
					ActivatePanel( 1 );
				}
				return 0;
				break;
			case ID_IMPSAVE_40030:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveImpFile();
					ActivatePanel( 1 );
				}
				return 0;
				break;
			case ID_SAVEGCOLI_40033:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveGcoFile();
					ActivatePanel( 1 );
				}
				return 0;
				break;
			case ID_DISPMW40002:
				DispMotionWindow();
				return 0;
				break;
			case 4007:
				DispToolWindow();
				return 0;
				break;
			case 40012:
				DispObjPanel();
				return 0;
				break;
			case ID_40048:
				DispConvBoneWindow();
				return 0;
				break;
			case ID_40049:
				DispAngleLimitDlg();
				return 0;
				break;
			case ID_40050:
				DispRotAxisDlg();
				return 0;
				break;
			case ID_DISPMODELPANEL:
				DispModelPanel();
				return 0;
				break;
			case ID_DISPGROUND:
				s_dispground = !s_dispground;
				return 0;
				break;
			case ID_NEWMOT:
				AddMotion( 0 );
				InitCurMotion(0, 0);
				return 0;
				break;
			case ID_DELCURMOT:
				OnDelMotion( s_curmotmenuindex );
				return 0;
				break;
			case ID_DELMODEL:
				OnDelModel( s_curmodelmenuindex );
				return 0;
				break;
			case ID_DELALLMODEL:
				OnDelAllModel();
				return 0;
				break;
			default:
				break;
			}
		}
	}else if( uMsg == WM_MOUSEWHEEL ){
		/*
		int delta;
		delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (s_underselectingframe == 1){
			OnTimeLineButtonSelectFromSelectStartEnd((double)delta, 0);
		}
		else{
			OnTimeLineCursor();
		}
		*/
		/*
		if( g_keybuf[VK_CONTROL] & 0x80 ){
			float radstep = 0.5f * (float)DEG2PAI;
			float delta;

			int curdelta;
			curdelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if( curdelta >= 0 ){
				delta = radstep;
			}else{
				delta = -1.0f * radstep;
			}

			if( s_model && s_model->GetTopBt() ){
//s_model->CalcBtAxismat( delta );//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
		}
		*/
	}else if( (uMsg == WM_LBUTTONDOWN) || (uMsg == WM_LBUTTONDBLCLK) ){
		if (s_curboneno >= 0){
			s_saveboneno = s_curboneno;
		}
		s_curboneno = -1;

		s_ikcnt = 0;
		SetCapture( s_mainwnd );
		POINT ptCursor;
		GetCursorPos( &ptCursor );
		::ScreenToClient( s_mainwnd, &ptCursor );
		s_pickinfo.clickpos = ptCursor;
		s_pickinfo.mousepos = ptCursor;
		s_pickinfo.mousebefpos = ptCursor;
		s_pickinfo.diffmouse = ChaVector2( 0.0f, 0.0f );
		s_pickinfo.firstdiff = ChaVector2( 0.0f, 0.0f );

		s_pickinfo.winx = (int)DXUTGetWindowWidth();
		s_pickinfo.winy = (int)DXUTGetWindowHeight();
		s_pickinfo.pickrange = 6;

		s_pickinfo.pickobjno = -1;

		int spckind = PickSpCam(ptCursor);
		if (spckind != 0){
			s_pickinfo.buttonflag = spckind;
			s_pickinfo.pickobjno = -1;
			RollbackCurBoneNo();
		}else if (s_model){
			int spakind = PickSpAxis( ptCursor );
			int pickrigflag = PickSpRig(ptCursor);
			if ((spakind != 0) && (s_saveboneno >= 0)){
				RollbackCurBoneNo();
				s_pickinfo.buttonflag = spakind;
				s_pickinfo.pickobjno = s_curboneno;
			} else if (pickrigflag == 1){
				RollbackCurBoneNo();
				ToggleRig();
			}else{
				if (s_oprigflag == 0){
					if (g_shiftkey == false){
						CallF(s_model->PickBone(&s_pickinfo), return 1);
					}
					if (s_pickinfo.pickobjno >= 0){
						s_curboneno = s_pickinfo.pickobjno;

						if (s_owpTimeline){
							s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
						}

						ChangeCurrentBone();

						if (s_model->GetInitAxisMatX() == 0){
							s_owpLTimeline->setCurrentTime(0.0, false);
							s_model->SetMotionFrame(0.0);
							s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);


							//ここでAxisMatXの初期化
							s_model->CreateBtObject(1);
							s_model->CalcBtAxismat(2);//2
							s_model->SetInitAxisMatX(1);
						}

						s_pickinfo.buttonflag = PICK_CENTER;//!!!!!!!!!!!!!

						s_pickinfo.firstdiff.x = (float)s_pickinfo.clickpos.x - s_pickinfo.objscreen.x;
						s_pickinfo.firstdiff.y = (float)s_pickinfo.clickpos.y - s_pickinfo.objscreen.y;

					}
					else{
						//if (g_previewFlag != 5){
							if (s_dispselect){
								int colliobjx, colliobjy, colliobjz, colliringx, colliringy, colliringz;
								colliobjx = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "objX");
								colliobjy = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "objY");
								colliobjz = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "objZ");
								if (s_ikkind == 0){
									colliringx = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "ringX");
									colliringy = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "ringY");
									colliringz = s_select->CollisionNoBoneObj_Mouse(&s_pickinfo, "ringZ");
								}
								else{
									colliringx = 0;
									colliringy = 0;
									colliringz = 0;
								}

								if (colliobjx || colliringx || colliobjy || colliringy || colliobjz || colliringz){
									RollbackCurBoneNo();
									s_pickinfo.pickobjno = s_curboneno;
								}

								if (colliobjx || colliringx){
									s_pickinfo.buttonflag = PICK_X;
								}
								else if (colliobjy || colliringy){
									s_pickinfo.buttonflag = PICK_Y;
								}
								else if (colliobjz || colliringz){
									s_pickinfo.buttonflag = PICK_Z;
								}
								else{
									ZeroMemory(&s_pickinfo, sizeof(PICKINFO));
								}
							}
							else{
								ZeroMemory(&s_pickinfo, sizeof(PICKINFO));
								s_pickinfo.pickobjno = -1;
							}
						//}
						//else{
						//	ZeroMemory(&s_pickinfo, sizeof(PICKINFO));
						//}
					}
				}
				else{
					int colliobj;
					colliobj = s_rigmark->CollisionNoBoneObj_Mouse(&s_pickinfo, "obj1");
					if (colliobj){
						RollbackCurBoneNo();
						s_pickinfo.buttonflag = PICK_CENTER;
						s_pickinfo.pickobjno = s_curboneno;
						//_ASSERT(0);
					}
					else{
						ZeroMemory(&s_pickinfo, sizeof(PICKINFO));
						s_pickinfo.pickobjno = -1;
					}
				}
				//if( s_owpLTimeline && s_model && s_model->GetCurMotInfo() ){
				//	int curlineno = s_owpLTimeline->getCurrentLine();
				//	if( curlineno == 1 ){
				//		s_editrange.SetRange( s_owpLTimeline->getSelectedKey(), s_owpTimeline->getCurrentTime() );
				//	}
				//}
			}
		}else{
			ZeroMemory( &s_pickinfo, sizeof( PICKINFO ) );
			s_pickinfo.pickobjno = -1;
		}

		ChaMatrixIdentity(&s_ikselectmat);
		if( s_model && (s_curboneno >= 0) ){
			CBone* curbone = s_model->GetBoneByID( s_curboneno );
			_ASSERT( curbone );
			if (curbone){
				s_saveboneno = s_curboneno;

				if (s_camtargetflag){
					g_camtargetpos = curbone->GetChildWorld();
				}
				MOTINFO* curmi = s_model->GetCurMotInfo();
				if (curmi){
					int multworld = 1;
					//s_ikselectmat = curbone->CalcManipulatorMatrix(0, 0, multworld, curmi->motid, curmi->curframe);//curmotinfo!!!
					s_ikselectmat = s_selm;
					s_ikselectmat._41 = 0.0f;
					s_ikselectmat._42 = 0.0f;
					s_ikselectmat._43 = 0.0f;
				}
			}
		}

		g_Camera.SetViewParams(&g_camEye, &g_camtargetpos);
		ChaVector3 diffv = g_camEye - g_camtargetpos;
		s_camdist = ChaVector3Length( &diffv );


		if (s_model && (s_pickinfo.pickobjno >= 0) && (g_previewFlag == 5)){
			if (PickSpBt(ptCursor) == 0){
				StartBt(1, 1);
				//s_model->BulletSimulationStart();
			}
			else{
				if (s_model){
					s_model->BulletSimulationStop();
					g_previewFlag = 0;
					s_model->ApplyBtToMotion();
				}
			}
		}


		if (s_model && (s_curboneno >= 0)){
			SetRigidLeng();
			SetImpWndParams();
			SetDmpWndParams();
			RigidElem2WndParam();
		}

	}else if( uMsg == WM_MBUTTONDOWN){

	}
	else if (uMsg == WM_MOUSEMOVE){
		if (s_pickinfo.buttonflag == PICK_CENTER){
			if (s_model){
				if (g_previewFlag == 0){

					s_pickinfo.mousebefpos = s_pickinfo.mousepos;
					POINT ptCursor;
					GetCursorPos(&ptCursor);
					::ScreenToClient(s_mainwnd, &ptCursor);
					s_pickinfo.mousepos = ptCursor;

					ChaVector3 tmpsc;
					s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

					if (s_model){
						if (s_oprigflag == 0){
							ChaVector3 targetpos(0.0f, 0.0f, 0.0f);
							CallF(CalcTargetPos(&targetpos), return 1);
							if (s_ikkind == 0){
								s_editmotionflag = s_model->IKRotate(&s_editrange, s_pickinfo.pickobjno, targetpos, s_iklevel);
							}
							else if (s_ikkind == 1){
								ChaVector3 diffvec = targetpos - s_pickinfo.objworld;
								AddBoneTra2(diffvec);
								s_editmotionflag = s_curboneno;
							}
						}
						else{
							if (s_customrigbone){
								float deltau = (float)(s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) * 0.5f;
								float deltav = (float)(s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y) * 0.5f;
								if (g_controlkey == true){
									deltau *= 0.250f;
									deltav *= 0.250f;
								}

								s_ikcustomrig = s_customrigbone->GetCustomRig(s_customrigno);

								s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 0, deltau, s_ikcustomrig);
								s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
								s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 1, deltav, s_ikcustomrig);
								s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
								s_editmotionflag = s_curboneno;
							}
						}
						s_ikcnt++;
					}
				}
				else if (g_previewFlag == 5){
					if (s_model){
						//if (s_onragdollik == 0){
						//	StartBt(1, 1);
						//}
						s_onragdollik = 1;
					}
				}
			}
		}
		else if ((s_pickinfo.buttonflag == PICK_X) || (s_pickinfo.buttonflag == PICK_Y) || (s_pickinfo.buttonflag == PICK_Z)){
			if (s_model){
				if (g_previewFlag == 0){
					s_pickinfo.mousebefpos = s_pickinfo.mousepos;
					POINT ptCursor;
					GetCursorPos(&ptCursor);
					::ScreenToClient(s_mainwnd, &ptCursor);
					s_pickinfo.mousepos = ptCursor;

					ChaVector3 tmpsc;
					s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

					if (g_previewFlag == 0){
						float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.5f;
						if (g_controlkey == true){
							deltax *= 0.250f;
						}
						if (s_ikkind == 0){
							s_editmotionflag = s_model->IKRotateAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, s_iklevel, s_ikcnt, s_ikselectmat);
						}
						else{
							AddBoneTra(s_pickinfo.buttonflag - PICK_X, deltax * 0.1f);
							s_editmotionflag = s_curboneno;
						}
						s_ikcnt++;
					}
				}
				else if (g_previewFlag == 5){
					if (s_model){
						s_onragdollik = 2;
					}
				}
			}
		}
		else if ((s_pickinfo.buttonflag == PICK_SPA_X) || (s_pickinfo.buttonflag == PICK_SPA_Y) || (s_pickinfo.buttonflag == PICK_SPA_Z)){
			if (s_model){
				if (g_previewFlag == 0){
					s_pickinfo.buttonflag = s_pickinfo.buttonflag - PICK_SPA_X + PICK_X;

					s_pickinfo.mousebefpos = s_pickinfo.mousepos;
					POINT ptCursor;
					GetCursorPos(&ptCursor);
					::ScreenToClient(s_mainwnd, &ptCursor);
					s_pickinfo.mousepos = ptCursor;

					ChaVector3 tmpsc;
					s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

					if (g_previewFlag == 0){
						float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.5f;
						if (g_controlkey == true){
							deltax *= 0.250f;
						}

						if (s_ikkind == 0){
							s_editmotionflag = s_model->IKRotateAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, s_iklevel, s_ikcnt, s_ikselectmat);
						}
						else{
							AddBoneTra(s_pickinfo.buttonflag - PICK_X, deltax * 0.1f);
							s_editmotionflag = s_curboneno;
						}
						s_ikcnt++;
					}
				}
				else if (g_previewFlag == 5){
					if (s_model){
						s_onragdollik = 3;
					}
				}
			}
		}
		else if (s_pickinfo.buttonflag == PICK_CAMMOVE){

			s_pickinfo.mousebefpos = s_pickinfo.mousepos;
			POINT ptCursor;
			GetCursorPos(&ptCursor);
			::ScreenToClient(s_mainwnd, &ptCursor);
			s_pickinfo.mousepos = ptCursor;

			ChaVector3 cammv;
			cammv.x = ((float)s_pickinfo.mousepos.x - (float)s_pickinfo.mousebefpos.x) / (float)s_pickinfo.winx * -s_cammvstep;
			cammv.y = ((float)s_pickinfo.mousepos.y - (float)s_pickinfo.mousebefpos.y) / (float)s_pickinfo.winy * s_cammvstep;
			cammv.z = 0.0f;
			if (g_controlkey == true){
				cammv *= 0.250f;
			}


			ChaMatrix matview;
			ChaVector3 weye, wat;
			matview = s_matView;
			weye = g_camEye;
			wat = g_camtargetpos;

			ChaVector3 cameye, camat;
			ChaVector3TransformCoord(&cameye, &weye, &matview);
			ChaVector3TransformCoord(&camat, &wat, &matview);

			ChaVector3 aftcameye, aftcamat;
			aftcameye = cameye + cammv;
			aftcamat = camat + cammv;

			ChaMatrix invmatview;
			ChaMatrixInverse(&invmatview, NULL, &matview);

			ChaVector3 neweye, newat;
			ChaVector3TransformCoord(&neweye, &aftcameye, &invmatview);
			ChaVector3TransformCoord(&newat, &aftcamat, &invmatview);

			g_Camera.SetViewParams(&neweye, &newat);



			g_camEye = neweye;
			g_camtargetpos = newat;
			ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			ChaVector3 diffv;
			diffv = neweye - newat;
			s_camdist = ChaVector3Length(&diffv);


		}
		else if (s_pickinfo.buttonflag == PICK_CAMROT){
			s_pickinfo.mousebefpos = s_pickinfo.mousepos;
			POINT ptCursor;
			GetCursorPos(&ptCursor);
			::ScreenToClient(s_mainwnd, &ptCursor);
			s_pickinfo.mousepos = ptCursor;

			float roty, rotxz;
			rotxz = -((float)s_pickinfo.mousepos.x - (float)s_pickinfo.mousebefpos.x) / (float)s_pickinfo.winx * 250.0f;
			roty = ((float)s_pickinfo.mousepos.y - (float)s_pickinfo.mousebefpos.y) / (float)s_pickinfo.winy * 250.0f;
			if (g_controlkey == true){
				rotxz *= 0.250f;
				roty *= 0.250f;
			}

			ChaMatrix matview;
			ChaVector3 weye, wat;
			weye = g_camEye;
			wat = g_camtargetpos;

			ChaVector3 viewvec, upvec, rotaxisy, rotaxisxz;
			viewvec = wat - weye;
			ChaVector3Normalize(&viewvec, &viewvec);
			upvec = ChaVector3(0.000001f, 1.0f, 0.0f);

			float chkdot;
			chkdot = ChaVector3Dot(&viewvec, &upvec);
			if (fabs(chkdot) < 0.99965f){
				ChaVector3Cross(&rotaxisxz, (const ChaVector3*)&upvec, (const ChaVector3*)&viewvec);
				ChaVector3Normalize(&rotaxisxz, &rotaxisxz);

				ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
				ChaVector3Normalize(&rotaxisy, &rotaxisy);
			}
			else if (chkdot >= 0.99965f){
				rotaxisxz = upvec;
				ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
				ChaVector3Normalize(&rotaxisy, &rotaxisy);
				if (roty < 0.0f){
					roty = 0.0f;
				}
				else{
				}
			}
			else{
				rotaxisxz = upvec;
				ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
				ChaVector3Normalize(&rotaxisy, &rotaxisy);
				if (roty > 0.0f){
					roty = 0.0f;
				}
				else{
					//rotyだけ回す。
				}
			}


			if (s_model && (s_curboneno >= 0) && s_camtargetflag){
				CBone* curbone = s_model->GetBoneByID(s_curboneno);
				_ASSERT(curbone);
				if (curbone){
					g_camtargetpos = curbone->GetChildWorld();
				}
			}


			ChaMatrix befrotmat, rotmaty, rotmatxz, aftrotmat;
			ChaMatrixTranslation(&befrotmat, -g_camtargetpos.x, -g_camtargetpos.y, -g_camtargetpos.z);
			ChaMatrixTranslation(&aftrotmat, g_camtargetpos.x, g_camtargetpos.y, g_camtargetpos.z);
			ChaMatrixRotationAxis(&rotmaty, &rotaxisy, rotxz * (float)DEG2PAI);
			ChaMatrixRotationAxis(&rotmatxz, &rotaxisxz, roty * (float)DEG2PAI);

			ChaMatrix mat;
			mat = befrotmat * rotmatxz * rotmaty * aftrotmat;
			ChaVector3 neweye;
			ChaVector3TransformCoord(&neweye, &weye, &mat);

			float chkdot2;
			ChaVector3 newviewvec = weye - neweye;
			ChaVector3Normalize(&newviewvec, &newviewvec);
			chkdot2 = ChaVector3Dot(&newviewvec, &upvec);
			if (fabs(chkdot2) < 0.99965f){
				ChaVector3Cross(&rotaxisxz, (const ChaVector3*)&upvec, (const ChaVector3*)&viewvec);
				ChaVector3Normalize(&rotaxisxz, &rotaxisxz);

				ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
				ChaVector3Normalize(&rotaxisy, &rotaxisy);
			}
			else{
				roty = 0.0f;
				rotaxisxz = upvec;
				ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
				ChaVector3Normalize(&rotaxisy, &rotaxisy);
			}
			ChaMatrixRotationAxis(&rotmaty, &rotaxisy, rotxz * (float)DEG2PAI);
			ChaMatrixRotationAxis(&rotmatxz, &rotaxisxz, roty * (float)DEG2PAI);
			mat = befrotmat * rotmatxz * rotmaty * aftrotmat;
			ChaVector3TransformCoord(&neweye, &weye, &mat);

			g_Camera.SetViewParams(&neweye, &g_camtargetpos);

			g_camEye = neweye;
			ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			ChaVector3 diffv;
			diffv = neweye - g_camtargetpos;
			s_camdist = ChaVector3Length(&diffv);


		}else if (s_pickinfo.buttonflag == PICK_CAMDIST){
			s_pickinfo.mousebefpos = s_pickinfo.mousepos;
			POINT ptCursor;
			GetCursorPos(&ptCursor);
			::ScreenToClient(s_mainwnd, &ptCursor);
			s_pickinfo.mousepos = ptCursor;

			float deltadist = (float)(s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y) * 0.5f;
			//float mdelta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
			//float deltadist = mdelta * s_camdist * 0.0010f;
			if (g_controlkey == true){
				deltadist *= 0.250f;
			}

			s_camdist += deltadist;
			if (s_camdist < 0.0001f){
				s_camdist = 0.0001f;
			}

			ChaVector3 camvec = g_camEye - g_camtargetpos;
			ChaVector3Normalize(&camvec, &camvec);
			g_camEye = g_camtargetpos + camvec * s_camdist;
			ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		}

	}else if( uMsg == WM_LBUTTONUP ){
		if (s_model && (s_onragdollik != 0)){
			s_model->BulletSimulationStop();
			//s_model->SetBtKinFlagReq(s_model->GetTopBt(), 1);
		}

		ReleaseCapture();
		s_pickinfo.buttonflag = 0;
		s_ikcnt = 0;
		s_onragdollik = 0;

		if( s_editmotionflag >= 0 ){
			if( s_model ){
				CreateTimeLineMark();
				SetLTimelineMark( s_curboneno );
				s_model->SaveUndoMotion( s_curboneno, s_curbaseno );
			}
			s_editmotionflag = -1;
		}

	}else if( uMsg == WM_RBUTTONDOWN ){
		
		BoneRClick(-1);

	}else if( uMsg == WM_RBUTTONUP ){
		//ReleaseCapture();
		s_pickinfo.buttonflag = 0;
	}else if( uMsg == WM_MBUTTONUP ){
		ReleaseCapture();
		s_pickinfo.buttonflag = 0;
	}


	/*
	if( uMsg == WM_LBUTTONDOWN ){
		g_Camera.HandleMessages( hWnd, WM_RBUTTONDOWN, wParam, lParam );
		if( s_ikkind == 2 ){
			g_LightControl[g_nActiveLight].HandleMessages( hWnd, WM_RBUTTONDOWN, wParam, lParam );
		}
	}else if( uMsg == WM_LBUTTONDBLCLK ){
	}else if( uMsg == WM_LBUTTONUP ){
		g_Camera.HandleMessages( hWnd, WM_RBUTTONUP, wParam, lParam );
		if( s_ikkind == 2 ){
			g_LightControl[g_nActiveLight].HandleMessages( hWnd, WM_RBUTTONUP, wParam, lParam );
		}
	}else if( uMsg == WM_RBUTTONDOWN ){
	}else if( uMsg == WM_RBUTTONUP ){
	}else if( uMsg == WM_RBUTTONDBLCLK ){
	}else if( uMsg == WM_MBUTTONDOWN ){
	}else if( uMsg == WM_MBUTTONUP ){
	}else if( uMsg == WM_MOUSEWHEEL ){
		//if( (g_keybuf[VK_CONTROL] & 0x80) == 0 ){
		//	float mdelta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
		//	//deltadist = mdelta * s_camdist * 0.00010f;
		//	deltadist = mdelta * s_camdist * 0.0010f;
		//
		//	s_camdist += deltadist;
		//	if( s_camdist < 0.0001f ){
		//		s_camdist = 0.0001f;
		//	}
		//
		//	ChaVector3 camvec = g_camEye - g_camtargetpos;
		//	ChaVector3Normalize( &camvec, &camvec );
		//	g_camEye = g_camtargetpos + s_camdist * camvec;
		//	ChaMatrixLookAtRH( &s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec );
		//}
	}else{
		g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
		if( s_ikkind == 2 ){
			g_LightControl[g_nActiveLight].HandleMessages( hWnd, uMsg, wParam, lParam );
		}
	}
	*/


	if( uMsg == WM_ACTIVATE ){
		if( wParam == 1 ){
			DbgOut( L"%f, activate wparam 1\r\n", s_time );
			ActivatePanel( 1 );
		}
	}
	if( uMsg == WM_SYSCOMMAND ){
		switch( wParam ){
		case SC_CLOSE:
			DbgOut( L"%f, syscommand close\r\n", s_time );
			break;
		case SC_MINIMIZE:
			DbgOut( L"%f, syscommand minimize\r\n", s_time );
			ActivatePanel( 0 );
			break;
		case SC_MAXIMIZE:
			DbgOut( L"%f, syscommand maximize\r\n", s_time );
			DefWindowProc( s_mainwnd, uMsg, wParam, lParam );
			ActivatePanel( 1 );
			return 1;//!!!!!!!!!!!!!
			break;
		}
	}


    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
//void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
        }
    }
}

int RollbackCurBoneNo()
{
	if ((s_curboneno < 0) && (s_saveboneno >= 0)){
		s_curboneno = s_saveboneno;
	}
	return 0;
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{

    CDXUTComboBox* pComboBox;
	ChaVector3 cureul, neweul;
	int tmpboneno;
	CBone* tmpbone;
	WCHAR sz[100];
	ChaVector3 weye;
	float trastep = s_totalmb.r * 0.05f;
	int modelnum = (int)s_modelindex.size();
	int modelno;
	int tmpikindex;

    switch( nControlID )
    {
  //      case IDC_ACTIVE_LIGHT:
  //          if( !g_LightControl[g_nActiveLight].IsBeingDragged() )
  //          {
  //              g_nActiveLight++;
  //              g_nActiveLight %= g_nNumActiveLights;
  //          }
  //          break;
		//
  //      case IDC_NUM_LIGHTS:
  //          if( !g_LightControl[g_nActiveLight].IsBeingDragged() )
  //          {
  //              WCHAR sz[100];
  //              swprintf_s( sz, 100, L"# Lights: %d", g_SampleUI.GetSlider( IDC_NUM_LIGHTS )->GetValue() );
  //              g_SampleUI.GetStatic( IDC_NUM_LIGHTS_STATIC )->SetText( sz );

  //              g_nNumActiveLights = g_SampleUI.GetSlider( IDC_NUM_LIGHTS )->GetValue();
  //              g_nActiveLight %= g_nNumActiveLights;
  //          }
  //          break;
		case IDC_BTSTART:
			StartBt(0, 1);
			break;
		case IDC_PHYSICS_IK:
			s_physicskind = 0;
			StartBt(1, 1);
			break;
		case IDC_PHYSICS_MV_IK:
			s_physicskind = 1;
			StartBt(1, 1);
			break;
		case IDC_PHYSICS_MV_SLIDER:
			RollbackCurBoneNo();
			g_physicsmvrate = (float)(g_SampleUI.GetSlider(IDC_PHYSICS_MV_SLIDER)->GetValue()) * 0.01f;
			swprintf_s(sz, 100, L"Physics Edit Rate : %.3f", g_physicsmvrate);
			g_SampleUI.GetStatic(IDC_STATIC_PHYSICS_MV_SLIDER)->SetText(sz);
			break;
		//case IDC_APPLY_BT:
		//	if (s_model){
		//		s_model->BulletSimulationStop();
		//		g_previewFlag = 0;
		//		s_model->ApplyBtToMotion();
		//	}
		//	break;
		case IDC_STOP_BT:
			g_previewFlag = 0;
			break;

        case IDC_LIGHT_SCALE:
			RollbackCurBoneNo();
			g_fLightScale = (float)(g_SampleUI.GetSlider(IDC_LIGHT_SCALE)->GetValue() * 0.10f);

            swprintf_s( sz, 100, L"Light scale: %0.2f", g_fLightScale );
            g_SampleUI.GetStatic( IDC_LIGHT_SCALE_STATIC )->SetText( sz );
            break;

        case IDC_SL_IKFIRST:
			RollbackCurBoneNo();
			g_ikfirst = (float)(g_SampleUI.GetSlider(IDC_SL_IKFIRST)->GetValue()) * 0.04f;
		    swprintf_s( sz, 100, L"IK 次数の係数 : %f", g_ikfirst );
            g_SampleUI.GetStatic( IDC_STATIC_IKFIRST )->SetText( sz );
            break;
        case IDC_SL_IKRATE:
			RollbackCurBoneNo();
			g_ikrate = (float)(g_SampleUI.GetSlider(IDC_SL_IKRATE)->GetValue()) * 0.01f;
		    swprintf_s( sz, 100, L"IK 伝達係数 : %f", g_ikrate );
            g_SampleUI.GetStatic( IDC_STATIC_IKRATE )->SetText( sz );
            break;
		case IDC_SL_NUMTHREAD:
			RollbackCurBoneNo();
			g_numthread = (int)(g_SampleUI.GetSlider(IDC_SL_NUMTHREAD)->GetValue());
			swprintf_s(sz, 100, L"スレッド数 : %d(%d) 個", g_numthread, gNumIslands);
			g_SampleUI.GetStatic(IDC_STATIC_NUMTHREAD)->SetText(sz);
			s_bpWorld->setNumThread(g_numthread);
			break;
		case IDC_SL_APPLYRATE:
			RollbackCurBoneNo();
			g_applyrate = g_SampleUI.GetSlider(IDC_SL_APPLYRATE)->GetValue();
		    swprintf_s( sz, 100, L"姿勢適用位置 : %d ％", g_applyrate );
            g_SampleUI.GetStatic( IDC_STATIC_APPLYRATE )->SetText( sz );
			CEditRange::SetApplyRate((double)g_applyrate);
			OnTimeLineSelectFormSelectedKey();
            break;

        case IDC_SPEED:
			RollbackCurBoneNo();
			g_dspeed = (float)(g_SampleUI.GetSlider(IDC_SPEED)->GetValue() * 0.010f);
			for( modelno = 0; modelno < modelnum; modelno++ ){
				s_modelindex[modelno].modelptr->SetMotionSpeed( g_dspeed );
			}

            swprintf_s( sz, 100, L"Motion Speed: %0.4f", g_dspeed );
            g_SampleUI.GetStatic( IDC_SPEED_STATIC )->SetText( sz );
            break;

		case IDC_BTCALCCNT:
			RollbackCurBoneNo();
			g_btcalccnt = (double)(g_SampleUI.GetSlider(IDC_BTCALCCNT)->GetValue());

			swprintf_s(sz, 100, L"BT CalcCnt: %0.2f", g_btcalccnt);
			g_SampleUI.GetStatic(IDC_STATIC_BTCALCCNT)->SetText(sz);
			break;

		case IDC_ERP:
			RollbackCurBoneNo();
			s_erp = (double)(g_SampleUI.GetSlider(IDC_ERP)->GetValue() * 0.0002);

			swprintf_s(sz, 100, L"BT ERP: %0.5f", s_erp);
			g_SampleUI.GetStatic(IDC_STATIC_ERP)->SetText(sz);
			break;

		case IDC_COMBO_BONEAXIS:
			RollbackCurBoneNo();
			if (s_model){
				pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_BONEAXIS);
				g_boneaxis = (int)PtrToUlong(pComboBox->GetSelectedData());
			}
			break;
		case IDC_COMBO_BONE:
			RollbackCurBoneNo();
			if (s_model){
				pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
				tmpboneno = (int)PtrToUlong( pComboBox->GetSelectedData() );
				tmpbone = s_model->GetBoneByID( tmpboneno );
				if( tmpbone ){
					s_curboneno = tmpboneno;
				}

				if( s_curboneno >= 0 ){
					int lineno = s_boneno2lineno[ s_curboneno ];
					if( lineno >= 0 ){
						s_owpTimeline->setCurrentLine( lineno, true );					
					}
				}
			}
			break;

		case IDC_COMBO_EDITMODE:
			if( s_model ){
				RollbackCurBoneNo();
				pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_EDITMODE);
				tmpikindex = (int)PtrToUlong( pComboBox->GetSelectedData() );
				switch( tmpikindex ){
				case IDC_IK_ROT:
					s_ikkind = 0;
					break;
				case IDC_IK_MV:
					s_ikkind = 1;
					break;
				//case IDC_IK_LIGHT:
				//	s_ikkind = 2;
				//	s_displightarrow = true;
				//	if (s_LightCheckBox){
				//		s_LightCheckBox->SetChecked(true);
				//	}
				//	break;
				case IDC_BT_RIGID:
					if( s_model && (s_curboneno >= 0) ){
						
						s_model->SetCurrentRigidElem(s_curreindex);

						//CallF(s_model->CreateBtObject(0), return);
						CallF(s_model->CreateBtObject(1), return);

						s_ikkind = 3;
						s_rigidWnd->setVisible( 1 );
						SetRigidLeng();
						RigidElem2WndParam();
					}
					break;
				case IDC_BT_IMP:
					if( s_model && (s_curboneno >= 0) ){
						CallF(s_model->CreateBtObject(0), return);

						s_ikkind = 4;
						s_impWnd->setVisible( 1 );

						SetImpWndParams();
						s_impWnd->callRewrite();
					}
					break;
				case IDC_BT_GP:
					if( s_bpWorld ){
						CallF(s_model->CreateBtObject(0), return);

						s_ikkind = 5;
						s_gpWnd->setVisible( 1 );

						SetGpWndParams();
						s_gpWnd->callRewrite();
					}
					break;
				case IDC_BT_DAMP:
					if( s_bpWorld ){
						CallF(s_model->CreateBtObject(0), return);

						s_ikkind = 6;
						s_dmpanimWnd->setVisible( 1 );

						SetDmpWndParams();
						s_dmpanimWnd->callRewrite();
					}
					break;
				}
			}
			break;

		case IDC_COMBO_IKLEVEL:
			RollbackCurBoneNo();
			pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_IKLEVEL);
			s_iklevel = (int)PtrToUlong( pComboBox->GetSelectedData() );
			break;
		case IDC_CAMTARGET:
			RollbackCurBoneNo();
			AutoCameraTarget();

			break;
		case IDC_APPLY_TO_THEEND:
			RollbackCurBoneNo();
			break;
		case IDC_SLERP_OFF:
			RollbackCurBoneNo();
			break;
		case IDC_ABS_IK:
			RollbackCurBoneNo();
			break;
		case IDC_PSEUDOLOCAL:
			RollbackCurBoneNo();
			break;
		case IDC_LIMITDEG:
			RollbackCurBoneNo();
			break;

		default:
			break;
	
	}

}


////--------------------------------------------------------------------------------------
//// This callback function will be called immediately after the Direct3D device has 
//// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
//// in the OnResetDevice callback should be released here, which generally includes all 
//// 0 resources. See the "Lost Devices" section of the documentation for 
//// information about lost devices.
////--------------------------------------------------------------------------------------
//void CALLBACK OnLostDevice( void* pUserContext )
//{
//    g_DialogResourceManager.OnD3D10LostDevice();
//    g_SettingsDlg.OnD3D10LostDevice();
//    CDXUTDirectionWidget::StaticOnD3D10LostDevice();
//    if( g_pFont )
//        g_pFont->OnLostDevice();
//    if( g_pEffect )
//        g_pEffect->OnLostDevice();
//    SAFE_RELEASE( g_pSprite );
//
//	if( g_texbank ){
//		CallF( g_texbank->Invalidate( INVAL_ONLYDEFAULT ), return );
//	}
//
//}
//

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
//void CALLBACK OnDestroyDevice( void* pUserContext )
//{
//    g_DialogResourceManager.OnD3D10DestroyDevice();
//    g_SettingsDlg.OnD3D10DestroyDevice();
//    CDXUTDirectionWidget::StaticOnD3D10DestroyDevice();
//    SAFE_RELEASE( g_pEffect );
//    SAFE_RELEASE( g_pFont );
//
//	if (s_editrangehistory){
//		delete[] s_editrangehistory;
//		s_editrangehistory = 0;
//	}
//	s_editrangehistoryno = 0;
//
//	if (s_anglelimitdlg){
//		DestroyWindow(s_anglelimitdlg);
//		s_anglelimitdlg = 0;
//	}
//	if (s_rotaxisdlg){
//		DestroyWindow(s_rotaxisdlg);
//		s_rotaxisdlg = 0;
//	}
//	if (s_customrigdlg){
//		DestroyWindow(s_customrigdlg);
//		s_customrigdlg = 0;
//	}
//	s_oprigflag = 0;
//	s_customrigbone = 0;
//
//	vector<MODELELEM>::iterator itrmodel;
//	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
//		CModel* curmodel = itrmodel->modelptr;
//		if( curmodel ){
//			//FbxScene* pscene = curmodel->GetScene();
//			//if (pscene){
//			//	pscene->Destroy();
//			//}
//
//			delete curmodel;
//		}
//	}
//	s_modelindex.clear();
//	s_model = 0;
//
//	if( s_select ){
//		delete s_select;
//		s_select = 0;
//	}
//	if (s_rigmark){
//		delete s_rigmark;
//		s_rigmark = 0;
//	}
//	if (s_ground){
//		delete s_ground;
//		s_ground = 0;
//	}
//	if( s_gplane ){
//		delete s_gplane;
//		s_gplane = 0;
//	}
//	if( s_bmark ){
//		delete s_bmark;
//		s_bmark = 0;
//	}
//
//
//	if( s_bcircle ){
//		delete s_bcircle;
//		s_bcircle = 0;
//	}
//	if( s_kinsprite ){
//		delete s_kinsprite;
//		s_kinsprite = 0;
//	}
//
//	if( g_texbank ){
//		delete g_texbank;
//		g_texbank = 0;
//	}
//
//	if( s_mainmenu ){
//		DestroyMenu( s_mainmenu );
//		s_mainmenu = 0;
//	}
//
//	DestroyTimeLine( 1 );
//
//	if( s_timelineWnd ){
//		delete s_timelineWnd;
//		s_timelineWnd = 0;
//	}
//	if( s_LtimelineWnd ){
//		delete s_LtimelineWnd;
//		s_LtimelineWnd = 0;
//	}
//
//	if( s_toolWnd ){
//		delete s_toolWnd;
//		s_toolWnd = 0;
//	}
//
//	if( s_toolCopyB ){
//		delete s_toolCopyB;
//		s_toolCopyB = 0;
//	}
//	if( s_toolSymCopyB ){
//		delete s_toolSymCopyB;
//		s_toolSymCopyB = 0;
//	}
//	if( s_toolCutB ){
//		delete s_toolCutB;
//		s_toolCutB = 0;
//	}
//	if( s_toolPasteB ){
//		delete s_toolPasteB;
//		s_toolPasteB = 0;
//	}
//	if( s_toolDeleteB ){
//		delete s_toolDeleteB;
//		s_toolDeleteB = 0;
//	}
//	if( s_toolMotPropB ){
//		delete s_toolMotPropB;
//		s_toolMotPropB = 0;
//	}
//	if( s_toolMarkB ){
//		delete s_toolMarkB;
//		s_toolMarkB = 0;
//	}
//	if( s_toolSelBoneB ){
//		delete s_toolSelBoneB;
//		s_toolSelBoneB = 0;
//	}
//	if( s_toolInitMPB ){
//		delete s_toolInitMPB;
//		s_toolInitMPB = 0;
//	}
//	if (s_toolFilterB){
//		delete s_toolFilterB;
//		s_toolFilterB = 0;
//	}
//	if (s_toolInterpolateB){
//		delete s_toolInterpolateB;
//		s_toolInterpolateB = 0;
//	}
//
//	if( s_owpTimeline ){
//		delete s_owpTimeline;
//		s_owpTimeline = 0;
//	}
//	if( s_owpPlayerButton ){
//		delete s_owpPlayerButton;
//		s_owpPlayerButton = 0;
//	}
//	if( s_owpLTimeline ){
//		delete s_owpLTimeline;
//		s_owpLTimeline = 0;
//	}
//
//	if( s_layerWnd ){
//		delete s_layerWnd;
//		s_layerWnd = 0;
//	}
//	if( s_owpLayerTable ){
//		delete s_owpLayerTable;
//		s_owpLayerTable = 0;
//	}
//
//	if( s_shprateSlider ){
//		delete s_shprateSlider;
//		s_shprateSlider = 0;
//	}
//	if( s_boxzSlider ){
//		delete s_boxzSlider;
//		s_boxzSlider = 0;
//	}
//	if( s_boxzlabel ){
//		delete s_boxzlabel;
//		s_boxzlabel = 0;
//	}
//	if( s_massSlider ){
//		delete s_massSlider;
//		s_massSlider = 0;
//	}
//	if( s_massSLlabel ){
//		delete s_massSLlabel;
//		s_massSLlabel = 0;
//	}
//	if( s_massB ){
//		delete s_massB;
//		s_massB = 0;
//	}
//	if( s_namelabel ){
//		delete s_namelabel;
//		s_namelabel = 0;
//	}
//	if( s_lenglabel ){
//		delete s_lenglabel;
//		s_lenglabel = 0;
//	}
//	if( s_rigidskip ){
//		delete s_rigidskip;
//		s_rigidskip = 0;
//	}
//	if (s_forbidrot){
//		delete s_forbidrot;
//		s_forbidrot = 0;
//	}
//
//	if (s_allrigidenableB){
//		delete s_allrigidenableB;
//		s_allrigidenableB = 0;
//	}
//	if (s_allrigiddisableB){
//		delete s_allrigiddisableB;
//		s_allrigiddisableB = 0;
//	}
//
//
//	if( s_shplabel ){
//		delete s_shplabel;
//		s_shplabel = 0;
//	}
//	if( s_colradio ){
//		delete s_colradio;
//		s_colradio = 0;
//	}
//	if( s_lkradio ){
//		delete s_lkradio;
//		s_lkradio = 0;
//	}
//	if( s_lkSlider ){
//		delete s_lkSlider;
//		s_lkSlider = 0;
//	}
//	if( s_lklabel ){
//		delete s_lklabel;
//		s_lklabel = 0;
//	}
//	if( s_akradio ){
//		delete s_akradio;
//		s_akradio = 0;
//	}
//	if( s_akSlider ){
//		delete s_akSlider;
//		s_akSlider = 0;
//	}
//	if( s_aklabel ){
//		delete s_aklabel;
//		s_aklabel = 0;
//	}
//	if( s_restSlider ){
//		delete s_restSlider;
//		s_restSlider = 0;
//	}
//	if( s_restlabel ){
//		delete s_restlabel;
//		s_restlabel = 0;
//	}
//	if( s_fricSlider ){
//		delete s_fricSlider;
//		s_fricSlider = 0;
//	}
//	if( s_friclabel ){
//		delete s_friclabel;
//		s_friclabel = 0;
//	}
//	if( s_ldmpSlider ){
//		delete s_ldmpSlider;
//		s_ldmpSlider = 0;
//	}
//	if( s_admpSlider ){
//		delete s_admpSlider;
//		s_admpSlider = 0;
//	}
//	if( s_kB ){
//		delete s_kB;
//		s_kB = 0;
//	}
//	if( s_restB ){
//		delete s_restB;
//		s_restB = 0;
//	}
//	if( s_ldmplabel ){
//		delete s_ldmplabel;
//		s_ldmplabel = 0;
//	}
//	if( s_admplabel ){
//		delete s_admplabel;
//		s_admplabel = 0;
//	}
//	if( s_dmpB ){
//		delete s_dmpB;
//		s_dmpB = 0;
//	}
//	if( s_groupB ){
//		delete s_groupB;
//		s_groupB = 0;
//	}
//	if( s_gcoliB ){
//		delete s_gcoliB;
//		s_gcoliB = 0;
//	}
//	if( s_btgSlider ){
//		delete s_btgSlider;
//		s_btgSlider = 0;
//	}
//	if( s_btglabel ){
//		delete s_btglabel;
//		s_btglabel = 0;
//	}
//	if( s_btgscSlider ){
//		delete s_btgscSlider;
//		s_btgscSlider = 0;
//	}
//	if( s_btgsclabel ){
//		delete s_btgsclabel;
//		s_btgsclabel = 0;
//	}
//	if( s_btforce ){
//		delete s_btforce;
//		s_btforce = 0;
//	}
//	if( s_groupcheck ){
//		delete s_groupcheck;
//		s_groupcheck = 0;
//	}
//	if( s_btgB ){
//		delete s_btgB;
//		s_btgB = 0;
//	}
//	if( s_rigidWnd ){
//		delete s_rigidWnd;
//		s_rigidWnd = 0;
//	}
//
//	if( s_dmpgroupcheck ){
//		delete s_dmpgroupcheck;
//		s_dmpgroupcheck = 0;
//	}
//	if( s_dmpanimLlabel ){
//		delete s_dmpanimLlabel;
//		s_dmpanimLlabel = 0;
//	}
//	if( s_dmpanimLSlider ){
//		delete s_dmpanimLSlider;
//		s_dmpanimLSlider = 0;
//	}
//	if( s_dmpanimAlabel ){
//		delete s_dmpanimAlabel;
//		s_dmpanimAlabel = 0;
//	}
//	if( s_dmpanimASlider ){
//		delete s_dmpanimASlider;
//		s_dmpanimASlider = 0;
//	}
//	if( s_dmpanimB ){
//		delete s_dmpanimB;
//		s_dmpanimB = 0;
//	}
//	if( s_dmpanimWnd ){
//		delete s_dmpanimWnd;
//		s_dmpanimWnd = 0;
//	}
//
//
//	if( s_impgroupcheck ){
//		delete s_impgroupcheck;
//		s_impgroupcheck = 0;
//	}
//	if( s_impzSlider ){
//		delete s_impzSlider;
//		s_impzSlider = 0;
//	}
//	if( s_impySlider ){
//		delete s_impySlider;
//		s_impySlider = 0;
//	}
//	if( s_impxSlider ){
//		delete s_impxSlider;
//		s_impxSlider = 0;
//	}
//	if( s_impzlabel ){
//		delete s_impzlabel;
//		s_impzlabel = 0;
//	}
//	if( s_impylabel ){
//		delete s_impylabel;
//		s_impylabel = 0;
//	}
//	if( s_impxlabel ){
//		delete s_impxlabel;
//		s_impxlabel = 0;
//	}
//	if( s_impscaleSlider ){
//		delete s_impscaleSlider;
//		s_impscaleSlider = 0;
//	}
//	if( s_impscalelabel ){
//		delete s_impscalelabel;
//		s_impscalelabel = 0;
//	}
//	if( s_impallB ){
//		delete s_impallB;
//		s_impallB = 0;
//	}
//	if( s_impWnd ){
//		delete s_impWnd;
//		s_impWnd = 0;
//	}
//	if( s_gpWnd ){
//		delete s_gpWnd;
//		s_gpWnd = 0;
//	}
//	if( s_ghSlider ){
//		delete s_ghSlider;
//		s_ghSlider = 0;
//	}
//	if( s_gsizexSlider ){
//		delete s_gsizexSlider;
//		s_gsizexSlider = 0;
//	}
//	if( s_gsizezSlider ){
//		delete s_gsizezSlider;
//		s_gsizezSlider = 0;
//	}
//	if( s_ghlabel ){
//		delete s_ghlabel;
//		s_ghlabel = 0;
//	}
//	if( s_gsizexlabel ){
//		delete s_gsizexlabel;
//		s_gsizexlabel = 0;
//	}
//	if( s_gsizezlabel ){
//		delete s_gsizezlabel;
//		s_gsizezlabel = 0;
//	}
//	if( s_gpdisp ){
//		delete s_gpdisp;
//		s_gpdisp = 0;
//	}
//	if( s_grestSlider ){
//		delete s_grestSlider;
//		s_grestSlider = 0;
//	}
//	if( s_grestlabel ){
//		delete s_grestlabel;
//		s_grestlabel = 0;
//	}
//	if( s_gfricSlider ){
//		delete s_gfricSlider;
//		s_gfricSlider = 0;
//	}
//	if( s_gfriclabel ){
//		delete s_gfriclabel;
//		s_gfriclabel = 0;
//	}
//
////static SPAXIS s_spaxis[3];
//	int spno;
//	for( spno = 0; spno < 3; spno++ ){
//		CMySprite* cursp = s_spaxis[spno].sprite;
//		if( cursp ){
//			delete cursp;
//		}
//		s_spaxis[spno].sprite = 0;
//	}
//
//	int spcno;
//	for (spcno = 0; spcno < 3; spcno++){
//		CMySprite* curspc = s_spcam[spcno].sprite;
//		if (curspc){
//			delete curspc;
//		}
//		s_spcam[spcno].sprite = 0;
//	}
//
//
//	int sprno;
//	for (sprno = 0; sprno < 2; sprno++){
//		CMySprite* cursp = s_sprig[sprno].sprite;
//		if (cursp){
//			delete cursp;
//		}
//		s_sprig[sprno].sprite = 0;
//	}
//
//	CMySprite* cursp = s_spbt.sprite;
//	if (cursp){
//		delete cursp;
//	}
//	s_spbt.sprite = 0;
//
//
//
//	DestroyModelPanel();
//	DestroyConvBoneWnd();
//
//	//DestroySdkObjects();
//
//	if( s_psdk ){
//		s_psdk->Destroy();
//		s_psdk = 0;
//	}
//
//	if( s_bpWorld ){
//		delete s_bpWorld;
//		s_bpWorld = 0;
//	}
//}

int DestroyTimeLine( int dellist )
{
	if( dellist ){
		EraseKeyList();
	}

	s_tlarray.clear();
	
	return 0;
}

int GetShaderHandle()
{
	if( !g_pEffect ){
		_ASSERT( 0 );
		return 1;
	}

	g_hRenderBoneL0 = g_pEffect->GetTechniqueByName( "RenderBoneL0" );
	_ASSERT( g_hRenderBoneL0 );
	g_hRenderBoneL1 = g_pEffect->GetTechniqueByName( "RenderBoneL1" );
	_ASSERT( g_hRenderBoneL1 );
	g_hRenderBoneL2 = g_pEffect->GetTechniqueByName( "RenderBoneL2" );
	_ASSERT( g_hRenderBoneL2 );
	g_hRenderBoneL3 = g_pEffect->GetTechniqueByName( "RenderBoneL3" );
	_ASSERT( g_hRenderBoneL3 );

	g_hRenderNoBoneL0 = g_pEffect->GetTechniqueByName( "RenderNoBoneL0" );
	_ASSERT( g_hRenderNoBoneL0 );
	g_hRenderNoBoneL1 = g_pEffect->GetTechniqueByName( "RenderNoBoneL1" );
	_ASSERT( g_hRenderNoBoneL1 );
	g_hRenderNoBoneL2 = g_pEffect->GetTechniqueByName( "RenderNoBoneL2" );
	_ASSERT( g_hRenderNoBoneL2 );
	g_hRenderNoBoneL3 = g_pEffect->GetTechniqueByName( "RenderNoBoneL3" );
	_ASSERT( g_hRenderNoBoneL3 );

	g_hRenderLine = g_pEffect->GetTechniqueByName( "RenderLine" );
	_ASSERT( g_hRenderLine );
	g_hRenderSprite = g_pEffect->GetTechniqueByName( "RenderSprite" );
	_ASSERT( g_hRenderSprite );



	g_hm4x4Mat = g_pEffect->GetVariableByName("g_m4x4Mat")->AsMatrix();
	_ASSERT( g_hm4x4Mat );
	g_hmWorld = g_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
	_ASSERT( g_hmWorld );
	g_hmVP = g_pEffect->GetVariableByName("g_mVP")->AsMatrix();
	_ASSERT( g_hmVP );
	g_hEyePos = g_pEffect->GetVariableByName("g_EyePos")->AsVector();
	_ASSERT( g_hEyePos );


//	g_hnNumLight = g_pEffect->GetVariableByName("g_nNumLights" );
//	_ASSERT( g_hnNumLight );
	g_hLightDir = g_pEffect->GetVariableByName("g_LightDir")->AsVector();
	_ASSERT( g_hLightDir );
	g_hLightDiffuse = g_pEffect->GetVariableByName("g_LightDiffuse")->AsVector();
	_ASSERT( g_hLightDiffuse );
//	g_hLightAmbient = g_pEffect->GetVariableByName("g_LightAmbient" );
//	_ASSERT( g_hLightAmbient );
	g_hSpriteOffset = g_pEffect->GetVariableByName("g_spriteoffset")->AsVector();
	_ASSERT(g_hSpriteOffset);
	g_hSpriteScale = g_pEffect->GetVariableByName("g_spritescale")->AsVector();
	_ASSERT(g_hSpriteScale);

	g_hPm3Offset = g_pEffect->GetVariableByName("g_pm3offset")->AsVector();
	_ASSERT(g_hPm3Offset);
	g_hPm3Scale = g_pEffect->GetVariableByName("g_pm3scale")->AsVector();
	_ASSERT(g_hPm3Scale);

	g_hdiffuse = g_pEffect->GetVariableByName("g_diffuse")->AsVector();
	_ASSERT( g_hdiffuse );
	g_hambient = g_pEffect->GetVariableByName("g_ambient")->AsVector();
	_ASSERT( g_hambient );
	g_hspecular = g_pEffect->GetVariableByName("g_specular")->AsVector();
	_ASSERT( g_hspecular );
	g_hpower = g_pEffect->GetVariableByName("g_power")->AsScalar();
	_ASSERT( g_hpower );
	g_hemissive = g_pEffect->GetVariableByName("g_emissive")->AsVector();
	_ASSERT( g_hemissive );
	g_hMeshTexture = g_pEffect->GetVariableByName("g_MeshTexture")->AsShaderResource();
	_ASSERT( g_hMeshTexture );


/*
// Obtain variables
g_ptxDiffuse = g_pEffect10->GetVariableByName("g_MeshTexture")->AsShaderResource();
g_pLightDir = g_pEffect10->GetVariableByName("g_LightDir")->AsVector();
g_pLightDiffuse = g_pEffect10->GetVariableByName("g_LightDiffuse")->AsVector();
g_pmWorldViewProjection = g_pEffect10->GetVariableByName("g_mWorldViewProjection")->AsMatrix();
g_pmWorld = g_pEffect10->GetVariableByName("g_mWorld")->AsMatrix();
g_pfTime = g_pEffect10->GetVariableByName("g_fTime")->AsScalar();
g_pMaterialAmbientColor = g_pEffect10->GetVariableByName("g_MaterialAmbientColor")->AsVector();
g_pMaterialDiffuseColor = g_pEffect10->GetVariableByName("g_MaterialDiffuseColor")->AsVector();
g_pnNumLights = g_pEffect10->GetVariableByName("g_nNumLights")->AsScalar();

// Create our vertex input layout
const D3D10_INPUT_ELEMENT_DESC layout[] =
{
{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
};

D3D10_PASS_DESC PassDesc;
V_RETURN(g_pTechRenderSceneWithTexture1Light->GetPassByIndex(0)->GetDesc(&PassDesc));
V_RETURN(pd3dDevice->CreateInputLayout(layout, 3, PassDesc.pIAInputSignature,
PassDesc.IAInputSignatureSize, &g_pVertexLayout));

// Load the mesh
V_RETURN(g_Mesh10.Create(pd3dDevice, L"tiny\\tiny.sdkmesh", true));

// Set effect variables as needed
D3DXCOLOR colorMtrlDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
D3DXCOLOR colorMtrlAmbient(0.35f, 0.35f, 0.35f, 0);
V_RETURN(g_pMaterialAmbientColor->SetFloatVector((float*)&colorMtrlAmbient));
V_RETURN(g_pMaterialDiffuseColor->SetFloatVector((float*)&colorMtrlDiffuse));

*/

	return 0;
}

int SetBaseDir()
{
	WCHAR filename[MAX_PATH] = { 0 };
	WCHAR* endptr = 0;

	int ret;
	ret = GetModuleFileNameW(NULL, filename, MAX_PATH);
	if (ret == 0){
		_ASSERT(0);
		return 1;
	}


	WCHAR* lasten = NULL;
	lasten = wcsrchr(filename, TEXT('\\'));
	if (!lasten){
		_ASSERT(0);
		DbgOut(L"SetMediaDir : strrchr error !!!\n");
		return 1;
	}

	//*.exeの頭
	*lasten = 0;

	WCHAR* last2en = 0;
	WCHAR* last3en = 0;
	WCHAR* last4en = 0;
	last2en = wcsrchr(filename, TEXT('\\'));//Release, Debugの頭の円かどうか調べる
	if (last2en){
		*last2en = 0;
		last3en = wcsrchr(filename, TEXT('\\'));//Release, Debugかどうか調べるフォルダの前にフォルダがあるかどうか
		if (last3en){
			if ((wcscmp(last2en + 1, L"debug") == 0) ||
				(wcscmp(last2en + 1, L"Debug") == 0) ||
				(wcscmp(last2en + 1, L"DEBUG") == 0) ||
				(wcscmp(last2en + 1, L"release") == 0) ||
				(wcscmp(last2en + 1, L"Release") == 0) ||
				(wcscmp(last2en + 1, L"RELEASE") == 0)
				){

				*last3en = 0;
				last4en = wcsrchr(filename, TEXT('\\'));//x64かどうか調べるフォルダの前にフォルダがあるかどうか
				if (last4en){
					if (wcscmp(last3en + 1, L"x64") != 0){
						*last3en = TEXT('\\');
					}
				}
			}
			else{
				*last2en = TEXT('\\');
			}
		}
	}

	int leng;
	ZeroMemory(g_basedir, sizeof(WCHAR)* MAX_PATH);
	wcscpy_s(g_basedir, MAX_PATH, filename);
	leng = wcslen(g_basedir);
	if (wcscmp(g_basedir + leng - 1, L"\\") != 0){
		wcscat_s(g_basedir, MAX_PATH, L"\\");
	}


	DbgOut(L"SetBaseDir : %s\r\n", g_basedir);

	return 0;
}

int OpenGcoFile()
{
	if( !s_bpWorld ){
		return 0;
	}
	if( !s_gplane ){
		return 0;
	}

	if( g_tmpmqopath[0] == 0 ){
		return 0;
	}

	CGColiFile gcofile;
	CallF( gcofile.LoadGColiFile( s_gplane, g_tmpmqopath, s_bpWorld ), return 1 );

	s_bpWorld->RemakeG();
	ChaVector3 tra( 0.0f, 0.0f, 0.0f );
	ChaVector3 mult( s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y );
	CallF( s_gplane->MultDispObj( mult, tra ), return 1 );

	return 0;
}


int OpenImpFile()
{
	if( !s_model ){
		return 0;
	}
	if( !s_model->GetTopBone() ){
		return 0;
	}

	if( g_tmpmqopath[0] == 0 ){
		return 0;
	}

	CImpFile impfile;
	CallF( impfile.LoadImpFile( g_tmpmqopath, s_model ), return 1 );

	int impnum = s_model->GetImpInfoSize();
	if( impnum > 0 ){
		OnImpMenu( impnum - 1 );
	}else{
		OnImpMenu( -1 );
	}

	return 0;
}


int OpenREFile()
{
	if( !s_model ){
		return 0;
	}
	if( !s_model->GetTopBone() ){
		return 0;
	}

	if( g_tmpmqopath[0] == 0 ){
		return 0;
	}

	if( s_model->GetRigidElemInfoSize() >= (MAXRENUM - 1) ){
		::MessageBox( s_mainwnd, L"これ以上読み込めません。", L"制限数オーバー(９９個まで)", MB_OK );
		return 0;
	}

	CRigidElemFile refile;
	CallF( refile.LoadRigidElemFile( g_tmpmqopath, s_model ), return 1 );


	int renum = s_model->GetRigidElemInfoSize();
	if( renum > 0 ){
		OnREMenu( renum - 1, 0 );
		OnRgdMenu( renum - 1, 0 );
	}else{
		OnREMenu( -1, 0 );
		OnRgdMenu( -1, 0 );
	}

	return 0;
}

int SaveGcoFile()
{
	if( !s_bpWorld ){
		return 0;
	}

	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_SAVEGCODLG ), 
		s_mainwnd, (DLGPROC)SaveGcoDlgProc );
	if( (dlgret != IDOK) || !s_Gconame[0] ){
		return 0;
	}

	CGColiFile gcofile;
	CallF( gcofile.WriteGColiFile( s_Gconame, s_bpWorld ), return 1 );

	return 0;


}

int SaveImpFile()
{
	if( !s_model ){
		return 0;
	}
	if( !s_model->GetTopBone() ){
		return 0;
	}
	if( s_rgdindex < 0 ){
		::MessageBox( s_mainwnd, L"ラグドール設定のimpulseしか保存できません。\nラグドール設定してから再試行してください。", L"準備エラー", MB_OK );
		return 0;
	}


	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_SAVEIMPDLG ), 
		s_mainwnd, (DLGPROC)SaveImpDlgProc );
	if( (dlgret != IDOK) || !s_Impname[0] ){
		return 0;
	}

	CImpFile impfile;
	CallF( impfile.WriteImpFile( s_Impname, s_model ), return 1 );

	int impnum = s_model->GetImpInfoSize();
	if( impnum > 0 ){
		OnImpMenu( impnum - 1 );
	}else{
		OnImpMenu( -1 );
	}

	return 0;


}

int SaveREFile()
{
	if( !s_model ){
		return 0;
	}
	if( !s_model->GetTopBone() ){
		return 0;
	}

	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_SAVEREDLG ), 
		s_mainwnd, (DLGPROC)SaveREDlgProc );
	if( (dlgret != IDOK) || !s_REname[0] ){
		return 0;
	}

	CRigidElemFile refile;
	CallF( refile.WriteRigidElemFile( s_REname, s_model, s_model->GetCurReIndex() ), return 1 );

	return 0;
}


int BVH2FBX()
{

	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_OPENMQODLG ), 
		s_mainwnd, (DLGPROC)OpenMqoDlgProc );
	if( (dlgret != IDOK) || !g_tmpmqopath[0] ){
		return 0;
	}


	WCHAR savepath[MULTIPATH];
	MoveMemory( savepath, g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );


	
	//bvhファイルを読み込む
	CBVHFile bvhfile;
	int ret;
	ret = bvhfile.LoadBVHFile( s_mainwnd, g_tmpmqopath, g_tmpmqomult );
	if( ret ){
		_ASSERT( 0 );
		return 1;
	}


	//FBXファイルに書き出す
	char fbxpath[MAX_PATH] = {0};
	WideCharToMultiByte( CP_UTF8, 0, g_tmpmqopath, -1, fbxpath, MAX_PATH, NULL, NULL );
	strcat_s( fbxpath, MAX_PATH, ".fbx" );
	CallF( BVH2FBXFile( s_psdk, &bvhfile, fbxpath ), return 1 );


	return 0;
}

int OpenFile()
{
	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_OPENMQODLG ), 
		s_mainwnd, (DLGPROC)OpenMqoDlgProc );
	if( (dlgret != IDOK) || !g_tmpmqopath[0] ){
		return 0;
	}


	WCHAR savepath[MULTIPATH];
	MoveMemory( savepath, g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );

	int leng;
	int namecnt = 0;
	leng = (int)wcslen( savepath );
	WCHAR* topchar = savepath + leng + 1;
	if( *topchar == TEXT( '\0' ) ){
		WCHAR* extptr = 0;
		extptr = wcsrchr( g_tmpmqopath, TEXT( '.' ) );
		if( !extptr ){
			return 0;
		}
		int cmpcha, cmpfbx, cmpmqo, cmpref, cmpimp, cmpgco;
		cmpcha = wcscmp( extptr, L".cha" );
		cmpfbx = wcscmp( extptr, L".fbx" );
		cmpmqo = wcscmp( extptr, L".mqo" );
		cmpref = wcscmp( extptr, L".ref" );
		cmpimp = wcscmp( extptr, L".imp" );
		cmpgco = wcscmp( extptr, L".gco" );
		if( cmpcha == 0 ){
			OpenChaFile();
			s_filterindex = 1;
		}else if( cmpfbx == 0 ){
			OpenFBXFile( 0 );
			s_filterindex = 1;
		}else if( cmpmqo == 0 ){
			OpenMQOFile();
			s_filterindex = 1;
		}else if( cmpref == 0 ){
			OpenREFile();
			s_filterindex = 2;
		}else if( cmpimp == 0 ){
			OpenImpFile();
			s_filterindex = 3;
		}else if( cmpgco == 0 ){
			OpenGcoFile();
			s_filterindex = 4;
		}

	}else{
		int leng2;
		while( *topchar != TEXT( '\0' ) ){
			swprintf_s( g_tmpmqopath, MULTIPATH, L"%s\\%s", savepath, topchar );

			WCHAR* extptr = 0;
			extptr = wcsrchr( g_tmpmqopath, TEXT( '.' ) );
			if( !extptr ){
				return 0;
			}
			int cmpfbx, cmpmqo;
			cmpfbx = wcscmp( extptr, L".fbx" );
			cmpmqo = wcscmp( extptr, L".mqo" );
			if( cmpfbx == 0 ){
				OpenFBXFile( 0 );
				s_filterindex = 1;
			}else if( cmpmqo == 0 ){
				OpenMQOFile();
				s_filterindex = 1;
			}
			leng2 = (int)wcslen( topchar );
			topchar = topchar + leng2 + 1;
			namecnt++;
		}
	}

	return 0;
}

CModel* OpenMQOFile()
{
	static int modelcnt = 0;
	WCHAR modelname[MAX_PATH] = {0L};
	WCHAR* lasten;
	lasten = wcsrchr( g_tmpmqopath, TEXT('\\') );
	if( !lasten ){
		_ASSERT( 0 );
		return 0;
	}
	wcscpy_s( modelname, MAX_PATH, lasten + 1 );
	WCHAR* extptr;
	extptr = wcsrchr( modelname, TEXT('.') );
	if( !extptr ){
		_ASSERT( 0 );
		return 0;
	}
	*extptr = 0L;
	WCHAR modelfolder[MAX_PATH] = {0L};
	swprintf_s( modelfolder, MAX_PATH, L"%s_%d", modelname, modelcnt );
	modelcnt++;


	if( !g_texbank ){
		g_texbank = new CTexBank( s_pdev );
		if( !g_texbank ){
			_ASSERT( 0 );
			return 0;
		}
	}
	if( s_model && (s_curmodelmenuindex >= 0) && (s_modelindex.empty() == 0) ){
		s_modelindex[s_curmodelmenuindex].tlarray = s_tlarray;
		s_modelindex[s_curmodelmenuindex].lineno2boneno = s_lineno2boneno;
		s_modelindex[s_curmodelmenuindex].boneno2lineno = s_boneno2lineno;
	}

	DestroyTimeLine( 1 );

    // Load the mesh
	CModel* newmodel;
	newmodel = new CModel();
	if( !newmodel ){
		_ASSERT( 0 );
		return 0;
	}
	int ret;
	ret = newmodel->LoadMQO( s_pdev, g_tmpmqopath, modelfolder, g_tmpmqomult, 0 );
	if( ret ){
		delete newmodel;
		if( s_owpTimeline ){
			refreshTimeline( *s_owpTimeline );
		}
		return 0;
	}else{
		s_model = newmodel;
	}
	CallF( s_model->MakeDispObj(), return 0 );
	CallF( s_model->DbgDump(), return 0 );

	int mindex;
	mindex = (int)s_modelindex.size();
	MODELELEM modelelem;
	modelelem.modelptr = s_model;
	modelelem.motmenuindex = 0;
	s_modelindex.push_back( modelelem );

    CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	pComboBox->RemoveAllItems();

	map<int, CBone*>::iterator itrbone;
	for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
		ULONG boneno = (ULONG)itrbone->first;
		CBone* curbone = itrbone->second;
		if( curbone && (boneno >= 0) ){
			char* nameptr = (char*)curbone->GetBoneName();
			WCHAR wname[256];
			ZeroMemory( wname, sizeof( WCHAR ) * 256 );
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
			pComboBox->AddItem( wname, ULongToPtr( boneno ) );
		}
	}

	s_totalmb.center = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.max = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.min = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.r = 0.0f;

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		MODELBOUND mb;
		curmodel->GetModelBound( &mb );
		AddModelBound( &s_totalmb, &mb );
	}

    FLOAT fObjectRadius;
	g_vCenter = s_totalmb.center;
	fObjectRadius = s_totalmb.r;

    //ChaMatrixTranslation( &g_mCenterWorld, -g_vCenter.x, -g_vCenter.y, -g_vCenter.z );

    for( int i = 0; i < MAX_LIGHTS; i++ )
		g_LightControl[i].SetRadius( fObjectRadius );


    ChaVector3 vecEye( 0.0f, 0.0f, g_initcamdist );
    ChaVector3 vecAt ( 0.0f, 0.0f, -0.0f );
	g_Camera.SetViewParams(&vecEye, &vecAt);
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

	s_camdist = g_initcamdist;
	g_camEye = ChaVector3( 0.0f, fObjectRadius * 0.5f, g_initcamdist );
	g_camtargetpos = ChaVector3( 0.0f, fObjectRadius * 0.5f, -0.0f );
	ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

	CallF( AddMotion( 0 ), return 0 );
	InitCurMotion(0, 0);

	s_modelindex[ mindex ].tlarray = s_tlarray;
	s_modelindex[ mindex ].lineno2boneno = s_lineno2boneno;
	s_modelindex[ mindex ].boneno2lineno = s_boneno2lineno;

//	CallF( OnModelMenu( mindex, 0 ), return 0 );

	CallF( CreateModelPanel(), return 0 );

	return newmodel;
}

CModel* OpenFBXFile( int skipdefref )
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;


	//int dlgret;
	//dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHECKAXISTYPE),
	//	s_mainwnd, (DLGPROC)CheckAxisTypeProc);
	//if (dlgret != IDOK){
	//	return 0;
	//}

	g_camtargetpos = ChaVector3( 0.0f, 0.0f, 0.0f );
	

	static int modelcnt = 0;
	WCHAR modelname[MAX_PATH] = {0L};
	WCHAR* lasten;
	lasten = wcsrchr( g_tmpmqopath, TEXT('\\') );
	if( !lasten ){
		_ASSERT( 0 );
		return 0;
	}
	wcscpy_s( modelname, MAX_PATH, lasten + 1 );
	WCHAR* extptr;
	extptr = wcsrchr( modelname, TEXT('.') );
	if( !extptr ){
		_ASSERT( 0 );
		return 0;
	}
	*extptr = 0L;
	WCHAR modelfolder[MAX_PATH] = {0L};
	swprintf_s( modelfolder, MAX_PATH, L"%s_%d", modelname, modelcnt );
	modelcnt++;


	if( !g_texbank ){
		g_texbank = new CTexBank( s_pdev );
		if( !g_texbank ){
			_ASSERT( 0 );
			return 0;
		}
	}
	if( s_model && (s_curmodelmenuindex >= 0) && (s_modelindex.empty() == 0) ){
		s_modelindex[s_curmodelmenuindex].tlarray = s_tlarray;
		s_modelindex[s_curmodelmenuindex].lineno2boneno = s_lineno2boneno;
		s_modelindex[s_curmodelmenuindex].boneno2lineno = s_boneno2lineno;
	}

	DestroyTimeLine( 1 );

    // Load the mesh
	CModel* newmodel;
	newmodel = new CModel();
	if( !newmodel ){
		_ASSERT( 0 );
		return 0;
	}

	_ASSERT(s_btWorld);
	newmodel->SetBtWorld( s_btWorld );
	FbxScene* pScene = 0;
	FbxImporter* pImporter = 0;

	int ret;
	ret = newmodel->LoadFBX(skipdefref, s_pdev, g_tmpmqopath, modelfolder, g_tmpmqomult, s_psdk, &pImporter, &pScene, s_forcenewaxis);
	if( ret ){
		_ASSERT( 0 );
		delete newmodel;
		if( s_owpTimeline ){
			refreshTimeline( *s_owpTimeline );
		}
		return 0;
	}else{
		s_model = newmodel;
	}

	CallF( s_model->MakeDispObj(), return 0 );


	int mindex;
	mindex = (int)s_modelindex.size();
	MODELELEM modelelem;
	modelelem.modelptr = s_model;
	modelelem.motmenuindex = 0;
	s_modelindex.push_back( modelelem );

    CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	pComboBox->RemoveAllItems();

	map<int, CBone*>::iterator itrbone;
	for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
		ULONG boneno = (ULONG)itrbone->first;
		CBone* curbone = itrbone->second;
		if( curbone && (boneno >= 0) ){
			char* nameptr = (char*)curbone->GetBoneName();
			WCHAR wname[256];
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
			pComboBox->AddItem( wname, ULongToPtr( boneno ) );
		}
	}

	s_totalmb.center = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.max = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.min = ChaVector3( 0.0f, 0.0f, 0.0f );
	s_totalmb.r = 0.0f;

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		MODELBOUND mb;
		curmodel->GetModelBound( &mb );
		if( mb.r != 0.0f ){
			AddModelBound( &s_totalmb, &mb );
		}
	}

    FLOAT fObjectRadius;
	g_vCenter = s_totalmb.center;
	fObjectRadius = s_totalmb.r;
	if( fObjectRadius < 0.1f ){
		fObjectRadius = 10.0f;
	}

	s_cammvstep = fObjectRadius;

DbgOut( L"fbx : totalmb : r %f, center (%f, %f, %f)\r\n",
	s_totalmb.r, s_totalmb.center.x, s_totalmb.center.y, s_totalmb.center.z );


	s_projnear = fObjectRadius * 0.01f;
	g_initcamdist = fObjectRadius * 3.0f;
	g_Camera.SetProjParams( D3DX_PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
	
	
    for( int i = 0; i < MAX_LIGHTS; i++ )
		g_LightControl[i].SetRadius( fObjectRadius );


    ChaVector3 vecEye( 0.0f, 0.0f, g_initcamdist );
    ChaVector3 vecAt ( 0.0f, 0.0f, -0.0f );
	g_Camera.SetViewParams(&vecEye, &vecAt);
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 6.0f );

	s_camdist = fObjectRadius * 4.0f;
	g_camEye = ChaVector3( 0.0f, fObjectRadius * 0.5f, fObjectRadius * 4.0f );
	g_camtargetpos = ChaVector3( 0.0f, fObjectRadius * 0.5f, -0.0f );
	ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);


	s_modelindex[ mindex ].tlarray = s_tlarray;
	s_modelindex[ mindex ].lineno2boneno = s_lineno2boneno;
	s_modelindex[ mindex ].boneno2lineno = s_boneno2lineno;


	CallF( OnModelMenu( mindex, 0 ), return 0 );

	CallF( CreateModelPanel(), return 0 );

	CallF( s_model->LoadFBXAnim( s_psdk, pImporter, pScene, OnAddMotion ), return 0 );
	if( (int)s_modelindex.size() >= 2 )
		_ASSERT( 0 );


	//if( s_model->GetMotInfoSize() >= 2 ){
	//	OnDelMotion( 0 );//初期状態のダミーモーションを削除
	//}

	int motnum = s_model->GetMotInfoSize();
	if (motnum == 0){
		CallF(AddMotion(0), return 0);
		s_modelindex[mindex].tlarray = s_tlarray;
		s_modelindex[mindex].lineno2boneno = s_lineno2boneno;
		s_modelindex[mindex].boneno2lineno = s_boneno2lineno;

		InitCurMotion(0, 0);
	}


	OnRgdMorphMenu( 0 );

//	SetCapture( s_mainwnd );

	s_curmotid = s_model->GetCurMotInfo()->motid;


	if (s_model->GetOldAxisFlagAtLoading() == 0){
		CLmtFile lmtfile;
		WCHAR lmtname[MAX_PATH];
		swprintf_s(lmtname, MAX_PATH, L"%s.lmt", g_tmpmqopath);
		lmtfile.LoadLmtFile(lmtname, s_model);

		WCHAR rigname[MAX_PATH] = { 0L };
		swprintf_s(rigname, MAX_PATH, L"%s.rig", g_tmpmqopath);
		CRigFile rigfile;
		rigfile.LoadRigFile(rigname, s_model);
	}


	s_model->CalcBoneEul(-1);

	s_model->SetLoadedFlag(true);


	s_model->SetMotionSpeed(g_dspeed);


	CallF(s_model->DbgDump(), return 0);

	g_dbgloadcnt++;

	return newmodel;
}

int InitCurMotion(int selectflag, double expandmotion)
{
	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (curmi){
		//CallF(s_model->FillUpEmptyMotion(curmi->motid), return 0);
		CBone* topbone = s_model->GetTopBone();
		if (topbone){
			double motleng = curmi->frameleng;
			//_ASSERT(0);

			if (selectflag == 1){//called from tool panel : init selected time range
				list<KeyInfo>::iterator itrcp;
				for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
					double curframe = itrcp->time;
					if (topbone){
						s_model->SetMotionFrame(curframe);
						InitMPReq(topbone, curframe);
					}
				}
			}
			else if (expandmotion > 0){//モーション長を長くした際に、長くなった分の初期化をする
				double oldframeleng = expandmotion;
				double frame;
				for (frame = oldframeleng; frame < motleng; frame += 1.0){
					if (topbone){
						s_model->SetMotionFrame(frame);
						InitMPReq(topbone, frame);
					}
				}
			}
			else{
				double frame;
				for (frame = 0.0; frame < motleng; frame += 1.0){
					if (topbone){
						s_model->SetMotionFrame(frame);
						InitMPReq(topbone, frame);
					}
				}
			}
		}
	}
	else{
		_ASSERT(0);
	}
	return 0;
};

int AddTimeLine( int newmotid )
{
	//EraseKeyList();

	if( !s_owpTimeline && s_model && (s_model->GetBoneListSize() > 0) ){
		//OWP_Timeline* owpTimeline = 0;
		//タイムラインのGUIパーツを生成
		s_owpTimeline= new OWP_Timeline( L"testmotion", 100.0, 4.0 );

		// カーソル移動時のイベントリスナーに
		// カーソル移動フラグcursorFlagをオンにするラムダ関数を登録する
		s_owpTimeline->setCursorListener([](){ s_cursorFlag = true; });

		// キー選択時のイベントリスナーに
		// キー選択フラグselectFlagをオンにするラムダ関数を登録する
		//s_owpTimeline->setSelectListener([](){ s_selectFlag = true; });//LTimelineへ移動

		s_owpTimeline->setMouseRUpListener([]() {s_timelineRUpFlag = true; });

		// キー移動時のイベントリスナーに
		// キー移動フラグkeyShiftFlagをオンにして、キー移動量をコピーするラムダ関数を登録する
		s_owpTimeline->setKeyShiftListener([](){
			s_keyShiftFlag = true;
			s_keyShiftTime = s_owpTimeline->getShiftKeyTime();
		});

		// キー削除時のイベントリスナーに
		// 削除されたキー情報をスタックするラムダ関数を登録する
		s_owpTimeline->setKeyDeleteListener([](const KeyInfo &keyInfo){
			//s_deletedKeyInfoList.push_back(keyInfo);
		});


		//ウィンドウにタイムラインを関連付ける
		s_timelineWnd->addParts(*s_owpTimeline);



//		s_owpTimeline->timeSize = 4.0;
//		s_owpTimeline->callRewrite();						//再描画
//		s_owpTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開
	}

	if( s_owpTimeline && (s_model->GetBoneListSize() > 0) ){
		int nextindex;
		nextindex = (int)s_tlarray.size();

		TLELEM newtle;
		newtle.menuindex = nextindex;
		newtle.motionid = newmotid;
		s_tlarray.push_back( newtle );

		CallF( s_model->SetCurrentMotion( newmotid ), return 1 );
	}

	if( !s_owpLTimeline && s_model ){
		s_owpLTimeline= new OWP_Timeline( L"EditRangeTimeLine" );
		s_LtimelineWnd->addParts(*s_owpLTimeline);//playerbuttonより後
		s_owpLTimeline->setCursorListener([]() { s_LcursorFlag = true; });
		s_owpLTimeline->setSelectListener([]() { s_selectFlag = true; });
		s_owpLTimeline->setMouseMDownListener([]() {
			s_timelinembuttonFlag = true;
			if (s_mbuttoncnt == 0) {
				s_mbuttoncnt = 1;
			}
			else {
				s_mbuttoncnt = 0;
			}
		});
		s_owpLTimeline->setMouseWheelListener([]() {
			s_timelinewheelFlag = true;
		});

	}

	//タイムラインのキーを設定
	if( s_owpTimeline ){
		refreshTimeline(*s_owpTimeline);
	}

	s_owpLTimeline->selectClear();

	return 0;
}

//タイムラインにモーションデータのキーを設定する
void refreshTimeline(OWP_Timeline& timeline){

	//時刻幅を設定
	if( s_model && (s_model->GetCurMotInfo()) ){
		timeline.setMaxTime( s_model->GetCurMotInfo()->frameleng );

		s_owpLTimeline->deleteKey();
		s_owpLTimeline->deleteLine();

		s_owpLTimeline->newLine(0, 0, s_strcurrent);
		//s_owpLTimeline->newKey( s_strcurrent, 0.0, 0 );
		s_owpLTimeline->newLine(0, 0, s_streditrange);
		s_owpLTimeline->newLine(0, 0, s_strmark);
		//s_owpLTimeline->newKey( s_strmark, 0.0, 0 );

		//s_owpLTimeline->setMaxTime( s_model->m_curmotinfo->frameleng - 1.0 );
		s_owpLTimeline->setMaxTime( s_model->GetCurMotInfo()->frameleng );//左端の１マスを選んだ状態がフレーム０を選んだ状態だから　-1 しない。

		int itime;
		for( itime = 0; itime < (int)s_model->GetCurMotInfo()->frameleng; itime++ ){
			s_owpLTimeline->newKey( s_streditrange, (double)itime, 0 );
		}
	}

	//すべての行をクリア
	timeline.deleteLine();

	s_lineno2boneno.clear();
	s_boneno2lineno.clear();

	if( s_model && s_model->GetTopBone() ){
		CallF( s_model->FillTimeLine( timeline, s_lineno2boneno, s_boneno2lineno ), return );
	}else{
		WCHAR label[256];
		swprintf_s(label, 256, L"dummy%d", 0);
		timeline.newLine(0, 0, label);
	}

	//選択時刻を設定
	timeline.setCurrentLine( 0 );
	s_owpLTimeline->setCurrentTime( 0.0, false );
	//timeline.setCurrentTime(0.0);


    CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	pComboBox->RemoveAllItems();
	if( s_model ){
		map<int, CBone*>::iterator itrbone;
		for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
			ULONG boneno = (ULONG)itrbone->first;
			CBone* curbone = itrbone->second;
			if( curbone && (boneno >= 0) ){
				char* nameptr = (char*)curbone->GetBoneName();
				WCHAR wname[256];
				MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
				pComboBox->AddItem( wname, ULongToPtr( boneno ) );
			}
		}
	}
}


int ConvBoneRotation(int selfflag, CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate)
{
	if (selfflag && !bvhbone){
		_ASSERT(0);
		return 0;
	}
	if ((selfflag == 0) && !befbvhbone){
		_ASSERT(0);
		return 0;
	}

	if (!s_convbone_model || !s_convbone_bvh || !srcbone){
		_ASSERT(0);
		return 0;
	}

	static ChaMatrix s_firsthipmat;
	static ChaMatrix s_invfirsthipmat;


	MOTINFO* bvhmi = s_convbone_bvh->GetMotInfoBegin()->second;
	int bvhmotid = bvhmi->motid;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	CMotionPoint bvhmp;
	if (bvhbone){
		CMotionPoint* pbvhmp = 0;
		pbvhmp = bvhbone->GetMotionPoint(bvhmotid, srcframe);
		if (pbvhmp){
			bvhmp = *pbvhmp;
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}
	else{
		CMotionPoint* pbvhmp = 0;
		pbvhmp = befbvhbone->GetMotionPoint(bvhmotid, srcframe);
		if (pbvhmp){
			bvhmp = *pbvhmp;
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}


	MOTINFO* modelmi = s_convbone_model->GetCurMotInfo();
	if (modelmi){
		int modelmotid = modelmi->motid;
		CMotionPoint modelmp;
		CMotionPoint* pmodelmp = 0;
		pmodelmp = srcbone->GetMotionPoint(modelmotid, srcframe);
		if (pmodelmp){
			modelmp = *pmodelmp;
		}
		else{
			_ASSERT(0);
			return 0;
		}

		CMotionPoint modelparmp;
		CMotionPoint* pmodelparmp = 0;
		if (srcbone->GetParent()){
			pmodelparmp = srcbone->GetParent()->GetMotionPoint(modelmotid, srcframe);
			if (pmodelparmp){
				modelparmp = *pmodelparmp;
			}
		}


		s_curboneno = srcbone->GetBoneNo();


		CQuaternion rotq;
		ChaVector3 traanim;

		if (bvhbone){
			ChaMatrix curbvhmat;
			ChaMatrix bvhmat;
			bvhmat = bvhmp.GetWorldMat();

			ChaMatrix modelinit, invmodelinit;
			modelinit = modelmp.GetWorldMat();
			invmodelinit = modelmp.GetInvWorldMat();

			if (srcbone == s_model->GetTopBone()){//モデル側の最初のボーンの処理時
				s_firsthipmat = bvhmp.GetWorldMat();
				s_firsthipmat._41 = 0.0f;
				s_firsthipmat._42 = 0.0f;
				s_firsthipmat._43 = 0.0f;
				ChaMatrixInverse(&s_invfirsthipmat, NULL, &s_firsthipmat);
				s_invfirsthipmat._41 = 0.0f;
				s_invfirsthipmat._42 = 0.0f;
				s_invfirsthipmat._43 = 0.0f;
			}


			//curbvhmat = bvhbone->GetInvFirstMat() * invmodelinit * bvhmat;
			//curbvhmat = bvhbone->GetInvFirstMat() * s_invfirsthipmat * invmodelinit * bvhmat;
			curbvhmat = s_invfirsthipmat * bvhbone->GetInvFirstMat() * s_firsthipmat * invmodelinit * bvhmat;//初期姿勢の変換にbvhの全体回転s_firsthipmatを考慮する。

			CMotionPoint curbvhrotmp;
			curbvhrotmp.CalcQandTra(curbvhmat, bvhbone);
			rotq = curbvhrotmp.GetQ();

			traanim = bvhbone->CalcLocalTraAnim(bvhmotid, srcframe);
			if (!bvhbone->GetParent()){
				ChaVector3 bvhbonepos = bvhbone->GetJointFPos();
				ChaVector3 firstframebonepos = bvhbone->GetFirstFrameBonePos();
				ChaVector3 firstdiff = firstframebonepos - bvhbonepos;
				traanim -= firstdiff;
			}
			traanim = traanim * hrate;

		}
		else{
			rotq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			traanim = ChaVector3(0.0f, 0.0f, 0.0f);
		}

		if (bvhbone){
			s_model->FKRotate(1, bvhbone, 1, traanim, srcframe, s_curboneno, rotq);
		}
		else{
			s_model->FKRotate(0, befbvhbone, 0, traanim, srcframe, s_curboneno, rotq);
		}
	}

	return 0;
}

int AddBoneTra2( ChaVector3 diffvec )
{
	if( !s_model || (s_curboneno < 0) && !s_model->GetTopBone() ){
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID( s_curboneno  );
	if( !curbone ){
		_ASSERT( 0 );
		return 0;
	}

	s_model->FKBoneTra( 0, &s_editrange, s_curboneno, diffvec );

	s_editmotionflag = s_curboneno;

	return 0;
}

/*
int ImpulseBonePhysics(ChaVector3 diffvec)
{
	if (!s_model || (s_curboneno < 0) && !s_model->GetTopBone()){
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (!curbone){
		_ASSERT(0);
		return 0;
	}

	s_model->ImpulseBoneRagdoll(0, &s_editrange, s_curboneno, diffvec);

	s_editmotionflag = s_curboneno;

	return 0;
}
*/

int AddBoneTra( int kind, float srctra )
{
	if( !s_model || (s_curboneno < 0) && !s_model->GetTopBone() ){
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( !curbone ){
		_ASSERT( 0 );
		return 0;
	}

	s_model->FKBoneTraAxis(0, &s_editrange, s_curboneno, kind, srctra);


	s_editmotionflag = s_curboneno;

	return 0;
}

int DispMotionWindow()
{
	if( !s_timelineWnd ){
		return 0;
	}

	if( s_dispmw ){
		s_timelineWnd->setVisible( false );
		s_LtimelineWnd->setVisible(false);
		s_dispmw = false;
	}else{
		s_timelineWnd->setVisible( true );
		s_LtimelineWnd->setVisible(true);
		s_dispmw = true;
	}

	return 0;
}
int DispToolWindow()
{
	if( !s_toolWnd ){
		return 0;
	}

	if( s_disptool ){
		s_toolWnd->setVisible( false );
		s_disptool = false;
	}else{
		s_toolWnd->setVisible( true );
		s_disptool = true;
	}

	return 0;
}
int DispObjPanel()
{
	if( !s_layerWnd ){
		return 0;
	}

	if( s_dispobj ){
		s_layerWnd->setVisible( false );
		s_dispobj = false;
	}else{
		s_layerWnd->setVisible( true );
		s_dispobj = true;
	}

	return 0;
}
int DispModelPanel()
{
	if( !s_modelpanel.panel ){
		return 0;
	}

	if( s_dispmodel ){
		s_modelpanel.panel->setVisible( false );
		s_dispmodel = false;
	}else{
		s_modelpanel.panel->setVisible( true );
		s_dispmodel = true;
	}

	return 0;
}

int DispConvBoneWindow()
{
	if (!s_model){
		return 0;
	}

	CreateConvBoneWnd();

	if (!s_convboneWnd){
		return 0;
	}

	if (s_dispconvbone){
		s_convboneWnd->setVisible(false);
		s_dispconvbone = false;
	}
	else{
		s_convboneWnd->setVisible(true);
		s_dispconvbone = true;
	}
	return 0;
}


int EraseKeyList()
{
	s_copyKeyInfoList.clear();
	s_copymotmap.clear();
	s_selectKeyInfoList.clear();
	s_deletedKeyInfoList.clear();

	return 0;
}

int AddMotion( WCHAR* wfilename, double srcmotleng )
{
	int motnum = (int)s_tlarray.size();
	if( motnum >= MAXMOTIONNUM ){
		MessageBox( s_mainwnd, L"これ以上モーションを読み込めません。", L"モーション数が多すぎます。", MB_OK );
		return 0;
	}

	char motionname[256];
	ZeroMemory( motionname, 256 );
	SYSTEMTIME systime;
	GetLocalTime( &systime );
	sprintf_s( motionname, 256, "motion_%d_%d_%d_%d_%d_%d_%d",
		systime.wYear,
		systime.wMonth,
		systime.wDay,
		systime.wHour,
		systime.wMinute,
		systime.wSecond,
		systime.wMilliseconds
	);

	int newmotid = -1;
	double motleng;
	if (srcmotleng == 0.0){
		motleng = 100.0;
	}
	else{
		motleng = srcmotleng;
	}
	CallF( s_model->AddMotion( motionname, wfilename, motleng, &newmotid ), return 1 );
	//_ASSERT(0);

	CallF( AddTimeLine( newmotid ), return 1 );

	int selindex = (int)s_tlarray.size() - 1;
	CallF( OnAnimMenu( selindex ), return 1 );


	return 0;
}

int OnRgdMorphMenu( int selindex )
{
	s_model->SetRgdMorphIndex( selindex );

	if( selindex < 0 ){
		return 0;//!!!!!!!!!
	}

	_ASSERT( s_morphmenu );

	int iAnimSet, cAnimSets;
	cAnimSets = GetMenuItemCount( s_morphmenu );
	for (iAnimSet = 0; iAnimSet < cAnimSets; iAnimSet++)
	{
		RemoveMenu(s_morphmenu, 0, MF_BYPOSITION);
	}

	if( !s_model || !s_owpTimeline ){
		return 0;//!!!!!!!!!!!!!!!!!!
	}


	cAnimSets = (int)s_tlarray.size();

	if( cAnimSets <= 0 ){
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	char* szName;
	WCHAR wname[256];
	for(iAnimSet = 0; iAnimSet < cAnimSets; iAnimSet++)
	{
		int motid;
		motid = s_tlarray[iAnimSet].motionid;

		szName = s_model->GetMotInfo( motid )->motname;
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szName, 256, wname, 256 );

		if (szName != NULL)
			AppendMenu(s_morphmenu, MF_STRING, 64000 + iAnimSet, wname);
		else
			AppendMenu(s_morphmenu, MF_STRING, 64000 + iAnimSet, L"<No Animation Name>");
	}

	if( cAnimSets > 0 ){
		CheckMenuItem(s_mainmenu, 64000 + selindex, MF_CHECKED);
	}

	return 0;
}


int OnAnimMenu( int selindex, int saveundoflag )
{
	s_curmotmenuindex = selindex;

	if( selindex < 0 ){
		return 0;//!!!!!!!!!
	}

	_ASSERT( s_animmenu );

	int iAnimSet, cAnimSets;
	cAnimSets = GetMenuItemCount( s_animmenu );
	for (iAnimSet = 0; iAnimSet < cAnimSets; iAnimSet++)
	{
		RemoveMenu(s_animmenu, 0, MF_BYPOSITION);
	}


	if( !s_model || !s_owpTimeline ){
		s_curmotid = -1;
		return 0;//!!!!!!!!!!!!!!!!!!
	}

	cAnimSets = (int)s_tlarray.size();

	if( cAnimSets <= 0 ){
		if( s_owpTimeline ){
			refreshTimeline(*s_owpTimeline);
		}
		s_curmotid = -1;
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	char* szName;
	WCHAR wname[256];
	for(iAnimSet = 0; iAnimSet < cAnimSets; iAnimSet++)
	{
		int motid;
		motid = s_tlarray[iAnimSet].motionid;
		szName = s_model->GetMotInfo( motid )->motname;
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szName, 256, wname, 256 );

		if (szName != NULL)
			AppendMenu(s_animmenu, MF_STRING, 59900 + iAnimSet, wname);
		else
			AppendMenu(s_animmenu, MF_STRING, 59900 + iAnimSet, L"<No Animation Name>");
	}

	if( cAnimSets > 0 ){
		CheckMenuItem(s_mainmenu, 59900 + selindex, MF_CHECKED);

		int selmotid;
		selmotid = s_tlarray[ selindex ].motionid;
		if( selmotid > 0 ){
			CallF( s_model->SetCurrentMotion( selmotid ), return 1 );
			//EraseKeyList();
			s_owpTimeline->setCurrentLine( 0 );
			s_owpTimeline->setCurrentTime( 0.0 );
			s_curmotid = selmotid;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
	}

	if( s_owpTimeline ){
		//タイムラインのキーを設定
		refreshTimeline(*s_owpTimeline);
		s_owpTimeline->setCurrentTime( 0.0 );
	}

	if( saveundoflag == 1 ){
		if( s_model ){
			s_model->SaveUndoMotion( s_curboneno, s_curbaseno );
		}
	}else{
		if( s_model ){
			double curframe = s_model->GetCurMotInfo()->curframe;
			s_owpLTimeline->setCurrentTime( curframe, false );
		}
	}

	s_owpLTimeline->selectClear();

	return 0;
}

int OnModelMenu( int selindex, int callbymenu )
{
	s_customrigbone = 0;
	
	if (s_anglelimitdlg){
		DestroyWindow(s_anglelimitdlg);
		s_anglelimitdlg = 0;
	}
	if (s_rotaxisdlg){
		DestroyWindow(s_rotaxisdlg);
		s_rotaxisdlg = 0;
	}
	if (s_customrigdlg){
		DestroyWindow(s_customrigdlg);
		s_customrigdlg = 0;
	}
	s_oprigflag = 0;

	if( callbymenu == 1 ){
		if( s_model && (s_curmodelmenuindex >= 0) && (s_modelindex.empty() == 0) ){
			s_modelindex[s_curmodelmenuindex].tlarray = s_tlarray;
			s_modelindex[s_curmodelmenuindex].lineno2boneno = s_lineno2boneno;
			s_modelindex[s_curmodelmenuindex].boneno2lineno = s_boneno2lineno;
		}
	}

	s_curmodelmenuindex = selindex;

	_ASSERT( s_modelmenu );
	int iMdlSet, cMdlSets;
	cMdlSets = GetMenuItemCount( s_modelmenu );
	for (iMdlSet = 0; iMdlSet < cMdlSets; iMdlSet++)
	{
		RemoveMenu(s_modelmenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		s_model = 0;
		s_curboneno = -1;
		if( s_owpTimeline ){
			refreshTimeline(*s_owpTimeline);
		}
		refreshModelPanel();
		return 0;//!!!!!!!!!
	}

	cMdlSets = (int)s_modelindex.size();
	if( cMdlSets <= 0 ){
		s_model = 0;
		if( s_owpTimeline ){
			refreshTimeline(*s_owpTimeline);
		}
		refreshModelPanel();
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	WCHAR* wname;
	for(iMdlSet = 0; iMdlSet < cMdlSets; iMdlSet++)
	{
		wname = (WCHAR*)s_modelindex[iMdlSet].modelptr->GetFileName();

		if( *wname != 0 )
			AppendMenu(s_modelmenu, MF_STRING, 61000 + iMdlSet, wname);
		else
			AppendMenu(s_modelmenu, MF_STRING, 61000 + iMdlSet, L"<No Model Name>");
	}

	if( cMdlSets > 0 ){
		CheckMenuItem( s_mainmenu, 61000 + selindex, MF_CHECKED );

		s_model = s_modelindex[ selindex ].modelptr;
		s_tlarray = s_modelindex[ selindex ].tlarray;
		s_curmotmenuindex = s_modelindex[ selindex ].motmenuindex;
		s_lineno2boneno = s_modelindex[ selindex ].lineno2boneno;
		s_boneno2lineno = s_modelindex[ selindex ].boneno2lineno;

		OnAnimMenu( s_curmotmenuindex );
	}


	if( s_model ){
		if( s_model->GetRigidElemInfoSize() > 0 ){
			CallF( OnREMenu( 0, 0 ), return 0 );
			CallF( OnRgdMenu( 0, 0 ), return 0 );
			CallF( OnImpMenu( 0 ), return 0 );
		}else{
			CallF( OnREMenu( -1, 0 ), return 0 );
			CallF( OnRgdMenu( -1, 0 ), return 0 );
			CallF( OnImpMenu( -1 ), return 0 );
		}
	}else{
		CallF( OnREMenu( -1, 0 ), return 0 );
		CallF( OnRgdMenu( -1, 0 ), return 0 );
		CallF( OnImpMenu( -1 ), return 0 );
	}


	refreshModelPanel();

	g_SampleUI.GetSlider( IDC_SPEED )->SetValue( (int)( g_dspeed * 100.0f ) );
	WCHAR sz[100];
	swprintf_s( sz, 100, L"Motion Speed: %0.4f", g_dspeed );
    g_SampleUI.GetStatic( IDC_SPEED_STATIC )->SetText( sz );


	return 0;
}

int OnREMenu( int selindex, int callbymenu )
{
	s_curreindex = selindex;

	_ASSERT( s_remenu );
	int iReSet, cReSets;
	cReSets = GetMenuItemCount( s_remenu );
	for (iReSet = 0; iReSet < cReSets; iReSet++)
	{
		RemoveMenu(s_remenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		AppendMenu(s_remenu, MF_STRING, 62000, L"読み込んでいません" );
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetRigidElemInfoSize();
	if( cReSets <= 0 ){
		s_curreindex = -1;
		AppendMenu(s_remenu, MF_STRING, 62000, L"読み込んでいません" );
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	WCHAR wname[MAX_PATH];
	for(iReSet = 0; iReSet < cReSets; iReSet++)
	{
		ZeroMemory( wname, sizeof( WCHAR ) * MAX_PATH );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, s_model->GetRigidElemInfo( iReSet ).filename, MAX_PATH, wname, MAX_PATH );
		AppendMenu(s_remenu, MF_STRING, 62000 + iReSet, wname);
DbgOut( L"OnREMenu : addmenu %s\r\n", wname );
	}

	if( cReSets > 0 ){
		CheckMenuItem( s_mainmenu, 62000 + selindex, MF_CHECKED );
	}

	CallF( s_model->SetCurrentRigidElem( s_curreindex ), return 1 );

	RigidElem2WndParam();
	
	return 0;
}

int OnRgdMenu( int selindex, int callbymenu )
{
	if( s_model ){
		s_model->SetRgdIndex( selindex );
	}
	s_rgdindex = selindex;

	_ASSERT( s_rgdmenu );
	int iReSet, cReSets;
	cReSets = GetMenuItemCount( s_rgdmenu );
	for (iReSet = 0; iReSet < cReSets; iReSet++)
	{
		RemoveMenu(s_rgdmenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		AppendMenu(s_rgdmenu, MF_STRING, 63000, L"読み込んでいません" );
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetRigidElemInfoSize();
	if( cReSets <= 0 ){
		s_rgdindex = -1;
		AppendMenu(s_rgdmenu, MF_STRING, 63000, L"読み込んでいません" );
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	WCHAR wname[MAX_PATH];
	for(iReSet = 0; iReSet < cReSets; iReSet++)
	{
		ZeroMemory( wname, sizeof( WCHAR ) * MAX_PATH );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, s_model->GetRigidElemInfo( iReSet ).filename, MAX_PATH, wname, MAX_PATH );
		AppendMenu(s_rgdmenu, MF_STRING, 63000 + iReSet, wname);
	}

	if( cReSets > 0 ){
		CheckMenuItem( s_mainmenu, 63000 + selindex, MF_CHECKED );
	}

//	CallF( s_model->SetCurrentRigidElem( s_curreindex ), return 1 );

	return 0;
}
int OnImpMenu( int selindex )
{
	if( s_model ){
		s_model->SetCurImpIndex( selindex );
	}

	_ASSERT( s_impmenu );
	int iReSet, cReSets;
	cReSets = GetMenuItemCount( s_impmenu );
	for (iReSet = 0; iReSet < cReSets; iReSet++)
	{
		RemoveMenu(s_impmenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		AppendMenu(s_impmenu, MF_STRING, 64500, L"読み込んでいません" );
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetImpInfoSize();
	if( cReSets <= 0 ){
		s_model->SetCurImpIndex( 0 );
		AppendMenu(s_impmenu, MF_STRING, 64500, L"読み込んでいません" );
		return 0;//!!!!!!!!!!!!!!!!!!!
	}

	WCHAR wname[MAX_PATH];
	for(iReSet = 0; iReSet < cReSets; iReSet++)
	{
		ZeroMemory( wname, sizeof( WCHAR ) * MAX_PATH );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, s_model->GetImpInfo( iReSet ).c_str(), -1, wname, MAX_PATH );
		AppendMenu(s_impmenu, MF_STRING, 64500 + iReSet, wname);
	}

	if( cReSets > 0 ){
		CheckMenuItem( s_mainmenu, 64500 + selindex, MF_CHECKED );
	}

	return 0;
}

int OnDelMotion( int delmenuindex )
{
	int tlnum = (int)s_tlarray.size();
	if( (tlnum <= 0) || (delmenuindex < 0) || (delmenuindex >= tlnum) ){
		return 0;
	}

	int delmotid = s_tlarray[ delmenuindex ].motionid;
	CallF( s_model->DeleteMotion( delmotid ), return 1 );


	int tlno;
	for( tlno = delmenuindex; tlno < (tlnum - 1); tlno++ ){
		s_tlarray[ tlno ] = s_tlarray[ tlno + 1 ];
	}
	s_tlarray.pop_back();

	int newtlnum = (int)s_tlarray.size();
	if( newtlnum == 0 ){
		AddMotion( 0 );
		InitCurMotion(0, 0);
	}else{
		OnAnimMenu( 0 );
	}

	return 0;
}

int OnDelModel( int delmenuindex )
{
	int mdlnum = (int)s_modelindex.size();
	if( (mdlnum <= 0) || (delmenuindex < 0) || (delmenuindex >= mdlnum) ){
		return 0;
	}

	if (mdlnum == 1){
		OnDelAllModel();//psdk rootnode初期化
		return 0;
	}


	CModel* delmodel = s_modelindex[ delmenuindex ].modelptr;
	if( delmodel ){
		map<CModel*, int>::iterator itrbonecnt;
		itrbonecnt = g_bonecntmap.find(delmodel);
		if (itrbonecnt != g_bonecntmap.end()){
			g_bonecntmap.erase(itrbonecnt);
		}

		FbxScene* pscene = delmodel->GetScene();
		if (pscene){
			pscene->Destroy();
		}
		delete delmodel;
	}

	int mdlno;
	for( mdlno = delmenuindex; mdlno < (mdlnum - 1); mdlno++ ){
		s_modelindex[ mdlno ].tlarray.clear();
		s_modelindex[ mdlno ].boneno2lineno.clear();
		s_modelindex[ mdlno ].lineno2boneno.clear();

		s_modelindex[ mdlno ] = s_modelindex[ mdlno + 1 ];
	}
	s_modelindex.pop_back();

	if( s_modelindex.empty() ){
		s_curboneno = -1;
		s_model = 0;
		s_curmodelmenuindex = -1;
		s_tlarray.clear();
		s_curmotmenuindex = -1;
		s_lineno2boneno.clear();
		s_boneno2lineno.clear();
	}else{
		s_curboneno = -1;
		s_model = s_modelindex[ 0 ].modelptr;
		s_tlarray = s_modelindex[ 0 ].tlarray;
		s_curmotmenuindex = s_modelindex[ 0 ].motmenuindex;
		s_lineno2boneno = s_modelindex[ 0 ].lineno2boneno;
		s_boneno2lineno = s_modelindex[ 0 ].boneno2lineno;
	}

	OnModelMenu( 0, 0 );

	CreateModelPanel();

	return 0;
}

int OnDelAllModel()
{
	int mdlnum = (int)s_modelindex.size();
	if( mdlnum <= 0 ){
		return 0;
	}

	g_bonecntmap.clear();

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* delmodel = itrmodel->modelptr;
		if( delmodel ){
			FbxScene* pscene = delmodel->GetScene();
			if (pscene){
				pscene->Destroy();
			}
			delete delmodel;
		}
		itrmodel->tlarray.clear();
		itrmodel->boneno2lineno.clear();
		itrmodel->lineno2boneno.clear();
	}

	s_modelindex.clear();

	s_curboneno = -1;
	s_model = 0;
	s_curmodelmenuindex = -1;
	s_tlarray.clear();
	s_curmotmenuindex = -1;
	s_lineno2boneno.clear();
	s_boneno2lineno.clear();

	OnModelMenu( -1, 0 );

	CreateModelPanel();

	return 0;
}



int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb )
{
	ChaVector3 newmin = mb->min;
	ChaVector3 newmax = mb->max;

	if( newmin.x > addmb->min.x ){
		newmin.x = addmb->min.x;
	}
	if( newmin.y > addmb->min.y ){
		newmin.y = addmb->min.y;
	}
	if( newmin.z > addmb->min.z ){
		newmin.z = addmb->min.z;
	}

	if( newmax.x < addmb->max.x ){
		newmax.x = addmb->max.x;
	}
	if( newmax.y < addmb->max.y ){
		newmax.y = addmb->max.y;
	}
	if( newmax.z < addmb->max.z ){
		newmax.z = addmb->max.z;
	}

	mb->center = ( newmin + newmax ) * 0.5f;
	mb->min = newmin;
	mb->max = newmax;

	ChaVector3 diff;
	diff = mb->center - newmin;
	mb->r = ChaVector3Length( &diff );

	return 0;
}

int refreshModelPanel()
{
	//すべての行をクリア
	s_owpLayerTable->deleteLine();

	WCHAR label[256];
	int objnum = 0;

	if( s_model ){
		objnum = s_model->GetMqoObjectSize();
	}else{
		objnum = 0;
	}
	
	if( objnum > 0 ){
		map<int, CMQOObject*>::iterator itrobj;
		for( itrobj = s_model->GetMqoObjectBegin(); itrobj != s_model->GetMqoObjectEnd(); itrobj++ ){
			CMQOObject* curobj = itrobj->second;
			WCHAR wname[256];
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)curobj->GetName(), 256, wname, 256 );
			s_owpLayerTable->newLine( wname, (void*)curobj );
			s_owpLayerTable->setVisible( wname, (curobj->GetDispFlag() > 0), true );
		}
	}else{
		wcscpy_s( label, 256, L"dummy name" );
		s_owpLayerTable->newLine( label, 0 );
	}

	if( s_modelpanel.radiobutton && ((int)s_modelindex.size() > 0) && (s_curmodelmenuindex >= 0) ){
		if( s_curmodelmenuindex >= 0 ){
			s_modelpanel.modelindex = s_curmodelmenuindex;
			s_modelpanel.radiobutton->setSelectIndex( s_modelpanel.modelindex );
		}
	}

	return 0;
}

int RenderSelectMark(int renderflag)
{
	if( s_curboneno < 0 ){
		return 0;
	}
	if (!s_model){
		return 0;
	}
	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (!curmi){
		return 0;
	}

	CBone* curboneptr = s_model->GetBoneByID( s_curboneno );
	if (curboneptr){
		if (s_onragdollik == 0){
			int multworld = 1;
			s_selm = curboneptr->CalcManipulatorMatrix(0, 0, multworld, curmi->motid, curmi->curframe);
		}

		ChaVector3 orgpos = curboneptr->GetJointFPos();
		ChaVector3 bonepos = curboneptr->GetChildWorld();

		ChaVector3 cam0, cam1;
		ChaMatrix mwv = s_model->GetWorldMat() * s_matView;
		ChaVector3TransformCoord( &cam0, &orgpos, &mwv );
		cam1 = cam0 + ChaVector3( 1.0f, 0.0f, 0.0f );

		ChaVector3 sc0, sc1;
		ChaVector3TransformCoord( &sc0, &cam0, &s_matProj );
		ChaVector3TransformCoord( &sc1, &cam1, &s_matProj );
		float lineleng = (sc0.x - sc1.x) * (sc0.x - sc1.x) + (sc0.y - sc1.y) * (sc0.y - sc1.y);
		if( lineleng > 0.00001f ){
			lineleng = sqrt( lineleng );
			s_selectscale = 0.0020f / lineleng;
			if ((s_oprigflag == 1) && (curboneptr == s_customrigbone)){
				s_selectscale *= 0.25f;
			}
		}
		else{
			//s_selectscaleの計算はしない。s_selectscaleは前回の計算値を使用。
		}
		ChaMatrix scalemat;
		ChaMatrixIdentity( &scalemat );
		ChaMatrixScaling(&scalemat, s_selectscale, s_selectscale, s_selectscale);

		s_selectmat = scalemat * s_selm;
		s_selectmat._41 = bonepos.x;
		s_selectmat._42 = bonepos.y;
		s_selectmat._43 = bonepos.z;

		if (renderflag){
			g_hmVP->SetMatrix((float*)&s_matVP);
			//g_pEffect->SetMatrix(g_hmVP, &(s_matVP.D3DX()));
			g_hmWorld->SetMatrix((float*)&s_selectmat);
			//g_pEffect->SetMatrix(g_hmWorld, &(s_selectmat.D3DX()));

			if (s_oprigflag == 0){
				RenderSelectFunc();
			}
			else{
				if (curboneptr == s_customrigbone){
					RenderRigMarkFunc();
				}
				else{
					RenderSelectFunc();
				}
			}
			//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			s_pdev->OMSetDepthStencilState(g_pDSStateZCmp, 1);

		}
	}

	return 0;
}

int RenderSelectFunc()
{
	s_select->UpdateMatrix(&s_selectmat, &s_matVP);
	//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	s_pdev->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
	if (s_dispselect){
		int lightflag = 1;
		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		s_select->OnRender(s_pdev, lightflag, diffusemult);
	}
	s_pdev->OMSetDepthStencilState(g_pDSStateZCmp, 1);

	return 0;

}

int RenderRigMarkFunc()
{
	s_rigmark->UpdateMatrix(&s_selectmat, &s_matVP);
	//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	s_pdev->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
	int lightflag = 1;
	ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
	s_rigmark->OnRender(s_pdev, lightflag, diffusemult);
	//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	s_pdev->OMSetDepthStencilState(g_pDSStateZCmp, 1);

	return 0;
}
int CalcTargetPos( ChaVector3* dstpos )
{
	ChaVector3 start3d, end3d;
	CalcPickRay( &start3d, &end3d );

	//カメラの面とレイとの交点(targetpos)を求める。
	ChaVector3 sb, se, n;
	sb = s_pickinfo.objworld - start3d;
	se = end3d - start3d;
	n = g_camtargetpos - g_camEye;

	float t;
	t = ChaVector3Dot( &sb, &n ) / ChaVector3Dot( &se, &n );

	*dstpos = start3d * (1.0f - t) + end3d * t;
	return 0;
}

int CalcPickRay( ChaVector3* startptr, ChaVector3* endptr )
{
	s_pickinfo.diffmouse.x = (float)( s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x );
	s_pickinfo.diffmouse.y = (float)( s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y );

	ChaVector3 mousesc;
	mousesc.x = s_pickinfo.objscreen.x + s_pickinfo.diffmouse.x;
	mousesc.y = s_pickinfo.objscreen.y + s_pickinfo.diffmouse.y;
	mousesc.z = s_pickinfo.objscreen.z;

	ChaVector3 startsc, endsc;
	float rayx, rayy;
	rayx = mousesc.x / (s_pickinfo.winx / 2.0f) - 1.0f;
	rayy = 1.0f - mousesc.y / (s_pickinfo.winy / 2.0f);

	startsc = ChaVector3( rayx, rayy, 0.0f );
	endsc = ChaVector3( rayx, rayy, 1.0f );
	
    ChaMatrix mView;
    ChaMatrix mProj;
    mProj = *g_Camera.GetProjMatrix();
    mView = s_matView;
	ChaMatrix mVP, invmVP;
	mVP = mView * mProj;
	ChaMatrixInverse( &invmVP, NULL, &mVP );

	ChaVector3TransformCoord( startptr, &startsc, &invmVP );
	ChaVector3TransformCoord( endptr, &endsc, &invmVP );

	return 0;
}


LRESULT CALLBACK OpenMqoDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	
	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlgWnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"project(*.cha)chara(*.fbx)\0*.cha;*.fbx\0Rigid(*.ref)\0*.ref\0Impulse(*.imp)\0*.imp\0Ground(*.gco)\0*.gco\0BVH(*.bvh)\0*.bvh\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = s_filterindex;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt =NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	WCHAR strmult[256];
	wcscpy_s( strmult, 256, L"1.000" );
	
	switch (msg) {
        case WM_INITDIALOG:
			g_tmpmqomult = 1.0f;
			ZeroMemory( g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );
			SetDlgItemText( hDlgWnd, IDC_MULT, strmult );
			SetDlgItemText( hDlgWnd, IDC_FILEPATH, L"参照ボタンを押してファイルを指定してください。" );
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_MULT, strmult, 256 );
					g_tmpmqomult = (float)_wtof( strmult );
					//GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );

                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
				case IDC_REFMQO:
					if( GetOpenFileNameW(&ofn) == IDOK ){
						SetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath );
					}
					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;
}


LRESULT CALLBACK OpenBvhDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlgWnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"BVH(*.bvh)\0*.bvh\0chara(*.fbx)\0*.fbx\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	WCHAR strmult[256];
	wcscpy_s(strmult, 256, L"1.000");
	WCHAR strfilter[256];
	wcscpy_s(strfilter, 256, L"100.0");

	switch (msg) {
	case WM_INITDIALOG:
		g_tmpmqomult = 1.0f;
		ZeroMemory(g_tmpmqopath, sizeof(WCHAR)* MULTIPATH);
		SetDlgItemText(hDlgWnd, IDC_MULT, strmult);
		SetDlgItemText(hDlgWnd, IDC_EDITFILTER, strfilter);
		SetDlgItemText(hDlgWnd, IDC_FILEPATH, L"参照ボタンを押してファイルを指定してください。");
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			GetDlgItemText(hDlgWnd, IDC_MULT, strmult, 256);
			g_tmpmqomult = (float)_wtof(strmult);
			//GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );
			GetDlgItemText(hDlgWnd, IDC_EDITFILTER, strfilter, 256);
			g_tmpbvhfilter = (float)_wtof(strfilter);

			EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
		case IDC_REFMQO:
			if (GetOpenFileNameW(&ofn) == IDOK){
				SetDlgItemText(hDlgWnd, IDC_FILEPATH, g_tmpmqopath);
			}
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK CheckAxisTypeProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlgWnd, IDC_FILENAME, g_tmpmqopath);
		SendMessage(GetDlgItem(hDlgWnd, IDC_AXISCHECK1), BM_SETCHECK, (WPARAM)0, 0L);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			if (IsDlgButtonChecked(hDlgWnd, IDC_AXISCHECK1) == BST_CHECKED){
				s_forcenewaxis = 1;
			}
			else{
				s_forcenewaxis = 0;
			}

			EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;

}


LRESULT CALLBACK MotPropDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

//static WCHAR s_tmpmotname[256] = {0L};
//static double s_tmpmotframeleng = 100.0f;
//static int s_tmpmotloop = 0;

	WCHAR strframeleng[256];

	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetCurMotInfo() ){
				MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, 
					s_model->GetCurMotInfo()->motname, 256, s_tmpmotname, 256 );
				SetDlgItemText( hDlgWnd, IDC_MOTNAME, s_tmpmotname );

				s_tmpmotframeleng = s_model->GetCurMotInfo()->frameleng;
				swprintf_s( strframeleng, 256, L"%f", s_tmpmotframeleng );
				SetDlgItemText( hDlgWnd, IDC_FRAMELENG, strframeleng );

				s_tmpmotloop = s_model->GetCurMotInfo()->loopflag;
				SendMessage( GetDlgItem( hDlgWnd, IDC_LOOP ), BM_SETCHECK, (WPARAM)s_tmpmotloop, 0L);

			}
			//SetDlgItemText( hDlgWnd, IDC_MULT, strmult );
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_MOTNAME, s_tmpmotname, 256 );
					
					GetDlgItemText( hDlgWnd, IDC_FRAMELENG, strframeleng, 256 );
					s_tmpmotframeleng = (float)_wtof( strframeleng );

					if( IsDlgButtonChecked( hDlgWnd, IDC_LOOP ) == BST_CHECKED ){  
                        s_tmpmotloop = 1;
                    }else{ 
						s_tmpmotloop = 0;
					}

                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;

}

LRESULT CALLBACK SaveGcoDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR buf[MAX_PATH];
	ZeroMemory( buf, sizeof( WCHAR ) * MAX_PATH );
	if( s_Gconame[0] != 0 ){
		wcscpy_s( buf, MAX_PATH, s_Gconame );
	}

	OPENFILENAME ofn;
	buf[0] = 0L;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"Ground File(*.gco)\0*.gco\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"gco";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;


	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_Gconame );
			}
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_Gconame, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
					if( GetSaveFileNameW(&ofn) == IDOK ){
						SetDlgItemText( hDlgWnd, IDC_FILENAME, buf );
					}
					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;

}

LRESULT CALLBACK SaveImpDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR buf[MAX_PATH];
	ZeroMemory( buf, sizeof( WCHAR ) * MAX_PATH );
	if( s_Impname[0] != 0 ){
		wcscpy_s( buf, MAX_PATH, s_Impname );
	}

	OPENFILENAME ofn;
	buf[0] = 0L;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"Impulse File(*.imp)\0*.imp\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"imp";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;


	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_Impname );
			}
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_Impname, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
					if( GetSaveFileNameW(&ofn) == IDOK ){
						SetDlgItemText( hDlgWnd, IDC_FILENAME, buf );
					}
					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;

}


LRESULT CALLBACK SaveREDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR buf[MAX_PATH];
	ZeroMemory( buf, sizeof( WCHAR ) * MAX_PATH );
	if( s_REname[0] != 0 ){
		wcscpy_s( buf, MAX_PATH, s_REname );
	}

	OPENFILENAME ofn;
	buf[0] = 0L;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"RigidElem File(*.ref)\0*.ref\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"ref";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;


	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_REname );
			}
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_REname, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
					if( GetSaveFileNameW(&ofn) == IDOK ){
						SetDlgItemText( hDlgWnd, IDC_FILENAME, buf );
					}
					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;

}


LRESULT CALLBACK ExportXDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR buf[MAX_PATH];
	OPENFILENAME ofn;
	buf[0] = 0L;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"x File (*.x)\0*.x\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt =NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	WCHAR strmult[256];
	wcscpy_s( strmult, 256, L"1.000" );
	
	switch (msg) {
        case WM_INITDIALOG:
			g_tmpmqomult = 1.0f;
			ZeroMemory( g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );
			SetDlgItemText( hDlgWnd, IDC_MULT, strmult );
			SetDlgItemText( hDlgWnd, IDC_FILEPATH, L"参照ボタンを押してファイルを指定してください。" );
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_MULT, strmult, 256 );
					g_tmpmqomult = (float)_wtof( strmult );
					GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
				case IDC_REFX:
					if( GetSaveFileNameW(&ofn) == IDOK ){
						SetDlgItemText( hDlgWnd, IDC_FILEPATH, buf );
					}
					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;
}



void ActivatePanel( int state )
{
	if( state == 1 ){
		if( s_dispmw ){
			s_timelineWnd->setVisible( false );
			s_timelineWnd->setVisible( true );
		}
		if( s_disptool ){
			s_toolWnd->setVisible( true );
		}
		if( s_dispobj ){
			s_layerWnd->setVisible( true );
		}
		if( s_dispmodel && s_modelpanel.panel ){
			s_modelpanel.panel->setVisible( true );
		}
	}else if( state == 0 ){
		s_timelineWnd->setVisible( false );
		s_toolWnd->setVisible( false );
		s_layerWnd->setVisible( false );
		if( s_modelpanel.panel ){
			s_modelpanel.panel->setVisible( false );
		}
	}
}


int DestroyModelPanel()
{
	if( s_modelpanel.panel ){
		s_modelpanel.panel->setVisible( false );
		delete s_modelpanel.panel;
		s_modelpanel.panel = 0;
	}
	if( s_modelpanel.radiobutton ){
		delete s_modelpanel.radiobutton;
		s_modelpanel.radiobutton = 0;
	}
	if( s_modelpanel.separator ){
		delete s_modelpanel.separator;
		s_modelpanel.separator = 0;
	}
	if( !(s_modelpanel.checkvec.empty()) ){
		int checknum = (int)s_modelpanel.checkvec.size();
		int checkno;
		for( checkno = 0; checkno < checknum; checkno++ ){
			delete s_modelpanel.checkvec[checkno];
		}
	}
	s_modelpanel.checkvec.clear();
	
	s_modelpanel.modelindex = -1;

	return 0;
}

int CreateModelPanel()
{
	DestroyModelPanel();

	int modelnum = (int)s_modelindex.size();
	if( modelnum <= 0 ){
		return 0;
	}
	
	int classcnt = 0;
	WCHAR clsname[256];
	swprintf_s( clsname, 256, L"ModelPanel%d", classcnt );

	s_modelpanel.panel = new OrgWindow(
		1,
		clsname,		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(900, 0),		//位置
		WindowSize(200,100),	//サイズ
		L"ModelPanel",	//タイトル
		NULL,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		70,50,70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_modelpanel.panel->setSizeMin(WindowSize(150,150));		// 最小サイズを設定

	int modelcnt;
	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		CModel* curmodel = s_modelindex[modelcnt].modelptr;
		_ASSERT( curmodel );

		if( modelcnt == 0 ){
			s_modelpanel.radiobutton = new OWP_RadioButton( curmodel->GetFileName() );
		}else{
			s_modelpanel.radiobutton->addLine( curmodel->GetFileName() );
		}
	}

	s_modelpanel.separator =  new OWP_Separator();									// セパレータ1（境界線による横方向2分割）


	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		CModel* curmodel = s_modelindex[modelcnt].modelptr;
		OWP_CheckBox *owpCheckBox= new OWP_CheckBox( L"表示/非表示", curmodel->GetModelDisp() );
		s_modelpanel.checkvec.push_back( owpCheckBox );
	}

	s_modelpanel.panel->addParts( *(s_modelpanel.separator) );

	s_modelpanel.separator->addParts1( *(s_modelpanel.radiobutton) );

	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		OWP_CheckBox* curcb = s_modelpanel.checkvec[modelcnt];
		s_modelpanel.separator->addParts2( *curcb );
	}

	s_modelpanel.modelindex = s_curmodelmenuindex;
	s_modelpanel.radiobutton->setSelectIndex( s_modelpanel.modelindex );

	s_modelpanel.panel->setVisible( 0 );//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

////////////
	s_modelpanel.panel->setCloseListener( [](){ s_closemodelFlag = true; } );

	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		s_modelpanel.checkvec[modelcnt]->setButtonListener( [modelcnt](){
			CModel* curmodel = s_modelindex[modelcnt].modelptr;
			curmodel->SetModelDisp( s_modelpanel.checkvec[modelcnt]->getValue() );
		} );
	}

	s_modelpanel.radiobutton->setSelectListener( [](){
		s_modelpanel.modelindex = s_modelpanel.radiobutton->getSelectIndex();
		OnModelMenu( s_modelpanel.modelindex, 1 );
	} );

	return 0;
}

int DestroyConvBoneWnd()
{
	s_convbone_model = 0;
	s_convbone_bvh = 0;
	s_convbonemap.clear();

	if (s_convboneWnd){
		s_convboneWnd->setVisible(false);
		delete s_convboneWnd;
		s_convboneWnd = 0;
	}
	if (s_convboneSCWnd){
		delete s_convboneSCWnd;
		s_convboneSCWnd = 0;
	}

	int cbno;
	for (cbno = 0; cbno < CONVBONEMAX; cbno++){
		if (s_modelbone[cbno]){
			delete s_modelbone[cbno];
			s_modelbone[cbno] = 0;
		}
		if (s_bvhbone[cbno]){
			delete s_bvhbone[cbno];
			s_bvhbone[cbno] = 0;
		}
		s_modelbone_bone[cbno] = 0;
		s_bvhbone_bone[cbno] = 0;
	}

	if (s_cbselmodel){
		delete s_cbselmodel;
		s_cbselmodel = 0;
	}
	if (s_cbselbvh){
		delete s_cbselbvh;
		s_cbselbvh = 0;
	}

	if (s_convboneconvert){
		delete s_convboneconvert;
		s_convboneconvert = 0;
	}

	if (s_convbonesp){
		delete s_convbonesp;
		s_convbonesp = 0;
	}

	if (s_convbonemidashi[0]){
		delete s_convbonemidashi[0];
		s_convbonemidashi[0] = 0;
	}
	if (s_convbonemidashi[1]){
		delete s_convbonemidashi[1];
		s_convbonemidashi[1] = 0;
	}

	s_convbonenum = 0;

	return 0;
}


int CreateConvBoneWnd()
{
	DestroyConvBoneWnd();

	s_convbonenum = s_model->GetBoneListSize();
	if (s_convbonenum >= CONVBONEMAX){
		_ASSERT(0);
		return 1;
	}

	s_convboneWnd = new OrgWindow(
		0,
		L"convbone0",		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(900, 200),		//位置
		WindowSize(200, 100),	//サイズ
		L"ConvBoneWnd",	//タイトル
		NULL,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_convboneWnd->setSizeMin(WindowSize(150, 150));		// 最小サイズを設定

	s_convboneSCWnd = new OWP_ScrollWnd(L"ConvBoneScWnd");
	s_convboneSCWnd->setLineDataSize(s_convbonenum + 4);

	s_convboneWnd->addParts(*s_convboneSCWnd);


	WCHAR bvhbonename[MAX_PATH];
	int cbno = 0;
	map<int, CBone*>::iterator itrbone;
	for (itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++){
		CBone* curbone = itrbone->second;
		if (curbone){
			const WCHAR* wbonename = curbone->GetWBoneName();
			_ASSERT(wbonename);
			s_modelbone[cbno] = new OWP_Label(wbonename);
			_ASSERT(s_modelbone[cbno]);
			s_modelbone_bone[cbno] = curbone;

			swprintf_s(bvhbonename, MAX_PATH, L"未設定_%03d", cbno);
			s_bvhbone[cbno] = new OWP_Button(bvhbonename);
			_ASSERT(s_bvhbone[cbno]);
			s_bvhbone_bone[cbno] = 0;

			s_convbonemap[curbone] = 0;

			DbgOut(L"convbone %d : (%s,  %s)\n", cbno, wbonename, bvhbonename);

			cbno++;
		}
	}
	DbgOut(L"\n\n");
	_ASSERT(cbno == s_convbonenum);


	s_convbonesp = new OWP_Separator(true);									// セパレータ1（境界線による横方向2分割）

	s_cbselmodel = new OWP_Button(L"モデルデータ選択ボタン");
	s_cbselbvh = new OWP_Button(L"bvh選択ボタン");
	s_convboneconvert = new OWP_Button(L"コンバートボタン");
	s_convbonemidashi[0] = new OWP_Label(L"骨入り形状側");
	s_convbonemidashi[1] = new OWP_Label(L"モーション側");

	s_convboneSCWnd->addParts(*s_convbonesp);


	s_convbonesp->addParts1(*s_convbonemidashi[0]);
	s_convbonesp->addParts1(*s_cbselmodel);
	s_convbonesp->addParts2(*s_convbonemidashi[1]);
	s_convbonesp->addParts2(*s_cbselbvh);


	for (cbno = 0; cbno < s_convbonenum; cbno++){
		s_convbonesp->addParts1(*s_modelbone[cbno]);
		s_convbonesp->addParts2(*s_bvhbone[cbno]);
	}

	s_convbonesp->addParts1(*s_convboneconvert);

	s_convboneWnd->setVisible(0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	////////////
	s_convboneWnd->setCloseListener([](){ s_closeconvboneFlag = true; });


	s_cbselmodel->setButtonListener([](){
		SetConvBoneModel();
		s_convboneWnd->callRewrite();
	});
	s_cbselbvh->setButtonListener([](){
		SetConvBoneBvh();
		s_convboneWnd->callRewrite();
	});
	for (cbno = 0; cbno < s_convbonenum; cbno++){
		s_bvhbone[cbno]->setButtonListener([cbno](){
			SetConvBone(cbno);
			//CModel* curmodel = s_modelindex[modelcnt].modelptr;
			//curmodel->SetModelDisp(s_modelpanel.checkvec[modelcnt]->getValue());
			s_convboneWnd->callRewrite();
		});
	}
	s_convboneconvert->setButtonListener([](){
		ConvBoneConvert();
	});


	return 0;
}

int SetConvBoneModel()
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	HWND parwnd;
	parwnd = s_convboneWnd->getHWnd();

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd);
	if (ret){
		return 1;
	}

	HMENU submenu = rmenu->GetSubMenu();

	int menunum;
	menunum = GetMenuItemCount(submenu);
	int menuno;
	for (menuno = 0; menuno < menunum; menuno++)
	{
		RemoveMenu(submenu, 0, MF_BYPOSITION);
	}

	int modelno;
	for (modelno = 0; modelno < modelnum; modelno++){
		CModel* curmodel = s_modelindex[modelno].modelptr;
		if (curmodel){
			const WCHAR* modelname = curmodel->GetFileName();
			if (modelname){
				int setmenuid = ID_RMENU_0 + modelno;
				AppendMenu(submenu, MF_STRING, setmenuid, modelname);
			}
		}
	}


	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	if ((menuid >= ID_RMENU_0) && (menuid < (ID_RMENU_0 + modelnum))){
		int modelindex = menuid - ID_RMENU_0;
		s_convbone_model = s_modelindex[modelindex].modelptr;

		WCHAR strmes[1024];
		if (!s_convbone_model){
			swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
			::MessageBox(NULL, strmes, L"check", MB_OK);
		}
		else{
			swprintf_s(strmes, 1024, L"%s", s_convbone_model->GetFileName());
			s_cbselmodel->setName(strmes);
		}
	}


	rmenu->Destroy();
	delete rmenu;


	return 0;
}
int SetConvBoneBvh()
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	HWND parwnd;
	parwnd = s_convboneWnd->getHWnd();

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd);
	if (ret){
		return 1;
	}

	HMENU submenu = rmenu->GetSubMenu();

	int menunum;
	menunum = GetMenuItemCount(submenu);
	int menuno;
	for (menuno = 0; menuno < menunum; menuno++)
	{
		RemoveMenu(submenu, 0, MF_BYPOSITION);
	}

	int modelno;
	for (modelno = 0; modelno < modelnum; modelno++){
		CModel* curmodel = s_modelindex[modelno].modelptr;
		if (curmodel){
			const WCHAR* modelname = curmodel->GetFileName();
			if (modelname){
				int setmenuid = ID_RMENU_0 + modelno;
				AppendMenu(submenu, MF_STRING, setmenuid, modelname);
			}
		}
	}


	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	if ((menuid >= ID_RMENU_0) && (menuid < (ID_RMENU_0 + modelnum))){
		int modelindex = menuid - ID_RMENU_0;
		s_convbone_bvh = s_modelindex[modelindex].modelptr;

		WCHAR strmes[1024];
		if (!s_convbone_bvh){
			swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
			::MessageBox(NULL, strmes, L"check", MB_OK);
		}
		else{
			swprintf_s(strmes, 1024, L"%s", s_convbone_bvh->GetFileName());
			s_cbselbvh->setName(strmes);
		}
	}


	rmenu->Destroy();
	delete rmenu;


	return 0;
}
int SetConvBone( int cbno )
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	if (!s_convbone_model || !s_convbone_bvh){
		return 0;
	}

	HWND parwnd;
	parwnd = s_convboneWnd->getHWnd();

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd);
	if (ret){
		return 1;
	}

	HMENU submenu = rmenu->GetSubMenu();

	int menunum;
	menunum = GetMenuItemCount(submenu);
	int menuno;
	for (menuno = 0; menuno < menunum; menuno++)
	{
		RemoveMenu(submenu, 0, MF_BYPOSITION);
	}

	int setmenuid0 = ID_RMENU_0 + 0;
	AppendMenu(submenu, MF_STRING, setmenuid0, L"未設定");

	int maxboneno = 0;
	map<int, CBone*>::iterator itrbone;
	for (itrbone = s_convbone_bvh->GetBoneListBegin(); itrbone != s_convbone_bvh->GetBoneListEnd(); itrbone++){
		CBone* curbone = itrbone->second;
		if (curbone){
			int boneno = curbone->GetBoneNo();
			int setmenuid = ID_RMENU_0 + boneno + 1;
			AppendMenu(submenu, MF_STRING, setmenuid, curbone->GetWBoneName());
			if (boneno > maxboneno){
				maxboneno = boneno;
			}
		}
	}

	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + maxboneno + 1))){
		if (menuid == (ID_RMENU_0 + 0)){
			//未設定
			s_bvhbone_bone[cbno] = 0;
			CBone* modelbone = s_modelbone_bone[cbno];
			_ASSERT(modelbone);
			if (modelbone){
				s_convbonemap[modelbone] = 0;
			}
			s_bvhbone[cbno]->setName(L"未設定");
		}
		else{
			int boneno = menuid - ID_RMENU_0 - 1;
			CBone* curbone = s_convbone_bvh->GetBoneByID(boneno);
			WCHAR strmes[1024];
			if (!curbone){
				s_bvhbone_bone[cbno] = 0;
				CBone* modelbone = s_modelbone_bone[cbno];
				_ASSERT(modelbone);
				if (modelbone){
					s_convbonemap[modelbone] = 0;
				}
				s_bvhbone[cbno]->setName(L"未設定");

				swprintf_s(strmes, 1024, L"convbone : sel bvh bone : curbone NULL !!!");
				::MessageBox(NULL, strmes, L"check", MB_OK);
			}
			else{
				swprintf_s(strmes, 1024, L"%s", curbone->GetWBoneName());
				s_bvhbone[cbno]->setName(strmes);
				s_bvhbone_bone[cbno] = curbone;

				CBone* modelbone = s_modelbone_bone[cbno];
				if (modelbone){
					s_convbonemap[modelbone] = curbone;
				}
			}
		}
	}

	rmenu->Destroy();
	delete rmenu;

	return 0;
}

int ConvBoneConvert()
{
	if (!s_convbone_model || !s_convbone_bvh){
		return 0;
	}
	if (s_model != s_convbone_model){
		::MessageBox(NULL, L"モデル選択メニューで形状側のモデルを選択してから再試行してください。", L"エラー", MB_OK);
		return 1;
	}

	g_underRetargetFlag = true;//!!!!!!!!!!!!

	MOTINFO* bvhmi = s_convbone_bvh->GetMotInfoBegin()->second;
	double motleng = bvhmi->frameleng;
	AddMotion(0, motleng);
	InitCurMotion(0, 0);


	MOTINFO* modelmi = s_convbone_model->GetCurMotInfo();
	CBone* modelbone = s_convbone_model->GetTopBone();


	HINFO bvhhi;
	bvhhi.minh = 1e7;
	bvhhi.maxh = -1e7;
	bvhhi.height = 0.0f;
	s_convbone_bvh->SetFirstFrameBonePos(&bvhhi);

	HINFO modelhi;
	modelhi.minh = 1e7;
	modelhi.maxh = -1e7;
	modelhi.height = 0.0f;
	s_convbone_model->SetFirstFrameBonePos(&modelhi);

	float hrate;
	if (bvhhi.height != 0.0f){
		hrate = modelhi.height / bvhhi.height;
	}
	else{
		hrate = 0.0f;
		_ASSERT(0);
	}

	double frame;
	for (frame = 0.0; frame < motleng; frame += 1.0){
		s_sethipstra = 0;

		if (modelbone){
			s_model->SetMotionFrame(frame);
			CBone* befbvhbone = s_convbone_bvh->GetTopBone();
			if (befbvhbone){
				ConvBoneConvertReq(modelbone, frame, befbvhbone, hrate);
			}
		}
	}

	s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);

	g_underRetargetFlag = false;//!!!!!!!!!!!!

	::MessageBox(NULL, L"コンバートが終了しました。", L"完了", MB_OK);

	return 0;
}

void InitMPReq(CBone* curbone, double curframe)
{
	if (!curbone){
		return;
	}

	InitMP(curbone, curframe);

	if (curbone->GetChild()){
		InitMPReq(curbone->GetChild(), curframe);
	}
	if (curbone->GetBrother()){
		InitMPReq(curbone->GetBrother(), curframe);
	}
}


void ConvBoneConvertReq(CBone* modelbone, double srcframe, CBone* befbvhbone, float hrate)
{
	if (!modelbone){
		_ASSERT(0);
		return;
	}

	CBone* bvhbone = s_convbonemap[modelbone];
	if (bvhbone){
		ConvBoneRotation(1, modelbone, bvhbone, srcframe, befbvhbone, hrate);
	}
	else{
		ConvBoneRotation(0, modelbone, 0, srcframe, befbvhbone, hrate);
	}


	if (modelbone->GetChild()){
		if (bvhbone){
			ConvBoneConvertReq(modelbone->GetChild(), srcframe, bvhbone, hrate);
		}
		else{
			ConvBoneConvertReq(modelbone->GetChild(), srcframe, befbvhbone, hrate);
		}
	}
	if (modelbone->GetBrother()){
		//if (bvhbone){
		//	ConvBoneConvertReq(modelbone->GetBrother(), srcframe, bvhbone, hrate);
		//}
		//else{
		ConvBoneConvertReq(modelbone->GetBrother(), srcframe, befbvhbone, hrate);
		//}
	}

}


int SetCamera6Angle()
{

	ChaVector3 weye, wdiff;
	weye = g_camEye;
	wdiff = g_camtargetpos - weye;
	float camdist = ChaVector3Length( &wdiff );

	ChaVector3 neweye;
	float delta = 0.10f;

	if( g_keybuf[ VK_F1 ] & 0x80 ){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z - camdist;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);

		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

	}else if( g_keybuf[ VK_F2 ] & 0x80 ){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z + camdist;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);
		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F3] & 0x80){
		neweye.x = g_camtargetpos.x - camdist;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);
		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F4] & 0x80){
		neweye.x = g_camtargetpos.x + camdist;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);
		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F5] & 0x80){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y + camdist;
		neweye.z = g_camtargetpos.z + delta;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);
		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F6] & 0x80){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y - camdist;
		neweye.z = g_camtargetpos.z - delta;

		g_Camera.SetViewParams(&neweye, &g_camtargetpos);
		g_camEye = neweye;
		ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}

	ChaVector3 diffv;
	diffv = g_camEye - g_camtargetpos;
	s_camdist = ChaVector3Length( &diffv );

	return 0;
}

int OnAddMotion( int srcmotid )
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;

	MOTINFO* newmotinfo = s_model->GetMotInfo( srcmotid );


	CallF( AddTimeLine( srcmotid ), return 1 );

	int selindex = (int)s_tlarray.size() - 1;
	CallF( OnAnimMenu( selindex ), return 1 );

	return 0;

}


int StartBt(int flag, int btcntzero)
{
	if (!s_model){
		return 0;
	}


	int resetflag = 0;
	int createflag = 0;


	if ((flag == 0) && (g_previewFlag != 4)){
		//F9キー
		g_previewFlag = 4;
		createflag = 1;
	}
	else if (flag == 1){
		//F10キー
		g_previewFlag = 5;
		createflag = 1;
	}
	else if (flag == 2){
		//spaceキー
		if (g_previewFlag == 4){
			resetflag = 1;
		}
		else if (g_previewFlag == 5){
			//resetflag = 1;
		}
	}
	else{
		g_previewFlag = 0;
	}



	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		//CModel* curmodel = s_model;
		if (curmodel){
			//if ((flag == 0) && (g_previewFlag != 4)){
				//F9キー
				if (btcntzero == 1){
					curmodel->ZeroBtCnt();
					curmodel->SetCreateBtFlag(false);
				}
			//}
			//else if (flag == 1){
			//	//F10キー
			//	if (btcntzero == 1){
			//		curmodel->ZeroBtCnt();
			//		curmodel->SetCreateBtFlag(false);
			//	}
			//}


			double curframe;
			if (resetflag == 1){
				curframe = s_owpTimeline->getCurrentTime();
				//curmodel->GetMotionFrame(&curframe);
			}
			else{
				s_previewrange = s_editrange;
				double rangestart;
				if (s_previewrange.IsSameStartAndEnd()) {
					//rangestart = 1.0;
					curframe = 1.0;
				}
				else {
					//rangestart = s_previewrange.GetStartFrame();
					curframe = s_previewrange.GetStartFrame();
				}
				//curframe = 1.0;//!!!!!!!!!!
				s_owpLTimeline->setCurrentTime(curframe, false);
			}

			if ((g_previewFlag == 4) || (g_previewFlag == 5)){

				if (g_previewFlag == 4){
					curmodel->SetCurrentRigidElem(s_curreindex);//s_curreindexをmodelごとに持つ必要あり！！！

					s_btWorld->setGravity(btVector3(0.0, -9.8, 0.0)); // 重力加速度の設定
					s_bpWorld->setGlobalERP(s_erp);// ERP



					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 20.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 30.0);


					curmodel->SetMotionFrame(curframe);

					vector<MODELELEM>::iterator itrmodel;
					for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
						CModel* curmodel = itrmodel->modelptr;
						if (curmodel){
							curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
						}
					}

					//curmodel->SetCurrentRigidElem(s_curreindex);//s_curreindexをmodelごとに持つ必要あり！！！reの内容を変えてから呼ぶ
					//s_curreindex = 1;
					curmodel->SetMotionSpeed(g_dspeed);
				}
				else if (g_previewFlag == 5){
					curmodel->SetCurrentRigidElem(s_rgdindex);//s_rgdindexをmodelごとに持つ必要あり！！！

					s_btWorld->setGravity(btVector3(0.0, 0.0, 0.0)); // 重力加速度の設定

				//ラグドールの時のERPは決め打ち
					//s_bpWorld->setGlobalERP(0.0);// ERP
					//s_bpWorld->setGlobalERP(1.0);// ERP
					//s_bpWorld->setGlobalERP(0.2);// ERP
					//s_bpWorld->setGlobalERP(0.001);// ERP
					//s_bpWorld->setGlobalERP(1.0e-8);// ERP
				
				
					s_bpWorld->setGlobalERP(0.00020);// ERP
					//s_bpWorld->setGlobalERP(s_erp);// ERP



					curmodel->SetMotionFrame(curframe);



					curmodel->SetColTypeAll(s_rgdindex, COL_CONE_INDEX);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!


				//ラグドールの時のバネは決め打ち
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1e4, 10.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 230.0, 30.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 600.0, 60.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 600.0, 30.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 600.0, 10.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 400.0, 10.0);
				
					//s_model->SetAllMassData(-1, s_rgdindex, 1e-9);
					//s_model->SetAllMassData(-1, s_rgdindex, 0.5);
					//s_model->SetAllMassData(-1, s_rgdindex, 1.0);
					//s_model->SetAllMassData(-1, s_rgdindex, 10.0);


					if (s_physicskind == 0){
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 30.0);
						curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 20.0);
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1600.0, 20.0);
					}
					else{
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 60.0);
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 2000.0, 60.0);
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 10000.0, 60.0);
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 13000.0, 200.0);
						//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 40000.0, 100.0);
						curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 30.0);
					}

					//s_model->SetAllMassData(-1, s_rgdindex, 100.0);
					//s_model->SetAllMassData(-1, s_rgdindex, 30.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 30.0);
					curmodel->SetAllMassDataByBoneLeng(-1, s_rgdindex, 30.0);

					curmodel->SetMotionSpeed(g_dspeed);
				}

				s_btstartframe = curframe;

				//CallF(curmodel->CreateBtObject(s_coldisp, 0), return 1);
				CallF(curmodel->CreateBtObject(1), return 1);

			
				//if( g_previewFlag == 4 ){

					curmodel->BulletSimulationStart();


					//s_bpWorld->clientResetScene();
					//if( s_model ){
					//	s_model->ResetBt();
					//}
					//int firstflag = 1;
					//s_model->Motion2Bt(firstflag, s_coldisp, s_btstartframe, &s_matW, &s_matVP);
					//int rgdollflag = 0;
					//double difftime = 0.0;
					//s_model->SetBtMotion(rgdollflag, s_btstartframe, &s_matW, &s_matVP);
					//s_model->ResetBt();
					UpdateBtSimu(curframe, curmodel);
				//}
			
				//if( g_previewFlag == 5 ){
				//	s_model->SetBtImpulse();
				//}

			
				if (curmodel->GetRgdMorphIndex() >= 0){
					MOTINFO* morphmi = curmodel->GetRgdMorphInfo();
					if (morphmi){
						//morphmi->curframe = 0.0;
						morphmi->curframe = s_btstartframe;
					}
				}

			}
		}
	}
	return 0;
}

int RigidElem2WndParam()
{
	if( s_curboneno < 0 ){
		return 0;
	}


	//ダイアログの数値はメニューで選択中のもの
	s_model->SetCurrentRigidElem(s_curreindex);


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		//int kinflag = curbone->m_btforce;

		CBone* parbone = curbone->GetParent();
		if( parbone ){
			int kinflag = parbone->GetBtForce();
			s_btforce->setValue( (bool)kinflag );
			
			CRigidElem* curre = parbone->GetRigidElem( curbone );
			if( curre ){
				float rate = (float)curre->GetSphrate();
				float boxz = (float)curre->GetBoxzrate();
				float mass = (float)curre->GetMass();
				bool skipflag = !((bool)curre->GetSkipflag());
				int colindex = curre->GetColtype();
				int lkindex = curre->GetLKindex();
				int akindex = curre->GetAKindex();
				float ldmp = curre->GetLDamping();
				float admp = curre->GetADamping();
				float cuslk = curre->GetCusLk();
				float cusak = curre->GetCusAk();
				float rest = curre->GetRestitution();
				float fric = curre->GetFriction();
				float btg = curre->GetBtg();
				int forbidrot = curre->GetForbidRotFlag();

				s_shprateSlider->setValue( rate );
				s_boxzSlider->setValue( boxz );
				s_massSlider->setValue( mass );
				s_rigidskip->setValue( skipflag );
				s_forbidrot->setValue(forbidrot);
				s_colradio->setSelectIndex( colindex );
				s_lkradio->setSelectIndex( lkindex );
				s_akradio->setSelectIndex( akindex );
				s_ldmpSlider->setValue( ldmp );
				s_admpSlider->setValue( admp );
				s_lkSlider->setValue( cuslk );
				s_akSlider->setValue( cusak );
				s_restSlider->setValue( rest );
				s_fricSlider->setValue( fric );
				s_btgSlider->setValue( btg );
			}
			else{
				//rigid elemが作成されていないとき
				s_shprateSlider->setValue(1.0);
				s_boxzSlider->setValue(1.0);
				s_massSlider->setValue(1.0);
				s_rigidskip->setValue(0);
				s_forbidrot->setValue(0);
				s_colradio->setSelectIndex(0);
				s_lkradio->setSelectIndex(0);
				s_akradio->setSelectIndex(0);
				s_ldmpSlider->setValue(g_l_dmp);
				s_admpSlider->setValue(g_a_dmp);
				s_lkSlider->setValue(g_initcuslk);
				s_akSlider->setValue(g_initcusak);
				s_restSlider->setValue(0.0);
				s_fricSlider->setValue(0.0);
				s_btgSlider->setValue(9.0);
			}
		}
		else{
			//rigid elemが作成されていないとき
			s_shprateSlider->setValue(1.0);
			s_boxzSlider->setValue(1.0);
			s_massSlider->setValue(1.0);
			s_rigidskip->setValue(0);
			s_forbidrot->setValue(0);
			s_colradio->setSelectIndex(0);
			s_lkradio->setSelectIndex(0);
			s_akradio->setSelectIndex(0);
			s_ldmpSlider->setValue(g_l_dmp);
			s_admpSlider->setValue(g_a_dmp);
			s_lkSlider->setValue(g_initcuslk);
			s_akSlider->setValue(g_initcusak);
			s_restSlider->setValue(0.0);
			s_fricSlider->setValue(0.0);
			s_btgSlider->setValue(9.0);
		}
		s_namelabel->setName( (WCHAR*)curbone->GetWBoneName() );
	}else{
		WCHAR noname[256];
		wcscpy_s( noname, 256, L"ボーン名：not selected" );
		s_namelabel->setName( noname );
	}
	if( s_model ){
		s_btgscSlider->setValue( s_model->GetBtGScale(s_model->GetCurReIndex()) );
	}

	s_rigidWnd->callRewrite();



	//再生中、シミュレーション中への対応。元の状態に戻す。
	if (g_previewFlag != 5){
		s_model->SetCurrentRigidElem(s_curreindex);
	}
	else{
		s_model->SetCurrentRigidElem(s_rgdindex);
	}



	return 0;
}

int SetRigidLeng()
{
	if( s_curboneno < 0 ){
		return 0;
	}
	if( !s_model ){
		return 0;
	}
	if( !s_model->GetTopBt() ){
		return 0;
	}

	CBtObject* curbto = s_model->FindBtObject( s_curboneno );
	if( curbto ){
		WCHAR curlabel[512];
		swprintf_s( curlabel, 512, L"ボーン名：%s", curbto->GetEndBone()->GetWBoneName() );
		if( s_namelabel ){
			s_namelabel->setName( curlabel );
		}

		WCHAR curlabel2[256];
		swprintf_s( curlabel2, 256, L"ボーン長:%.3f[m]", curbto->GetBoneLeng() );
		if( s_lenglabel ){
			s_lenglabel->setName( curlabel2 );
		}
	}else{
		WCHAR curlabel[512];
		wcscpy_s( curlabel, 512, L"ボーン名：not selected" );
		if( s_namelabel ){
			s_namelabel->setName( curlabel );
		}

		WCHAR curlabel2[256];
		wcscpy_s( curlabel2, 256, L"ボーン長：not selected" );
		if( s_lenglabel ){
			s_lenglabel->setName( curlabel2 );
		}
	}
	return 0;
}

int SetImpWndParams()
{
	if( s_curboneno < 0 ){
		return 0;
	}
	if( !s_model ){
		return 0;
	}
	if( s_rgdindex < 0 ){
		return 0;
	}


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		CBone* parbone = curbone->GetParent();
		if( parbone ){
			ChaVector3 setimp( 0.0f, 0.0f, 0.0f );


			int impnum = parbone->GetImpMapSize();
			if( (s_model->GetCurImpIndex() >= 0) && (s_model->GetCurImpIndex() < impnum) ){
				string curimpname = s_model->GetImpInfo( s_model->GetCurImpIndex() );
				setimp = parbone->GetImpMap( curimpname, curbone );
			}
			else{
				//_ASSERT(0);
			}

			if( s_impzSlider ){
				s_impzSlider->setValue( setimp.z );
			}
			if( s_impySlider ){
				s_impySlider->setValue( setimp.y );
			}
			if( s_impxSlider ){
				s_impxSlider->setValue( setimp.x );
			}
			if( s_impscaleSlider ){
				s_impscaleSlider->setValue( g_impscale );
			}
		}
	}

	s_impWnd->callRewrite();

	return 0;
}

int SetDmpWndParams()
{
	if( s_curboneno < 0 ){
		return 0;
	}
	if( !s_model ){
		return 0;
	}
	if( s_rgdindex < 0 ){
		return 0;
	}


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		CBone* parbone = curbone->GetParent();
		if( parbone ){
			char* filename = s_model->GetRigidElemInfo( s_rgdindex ).filename;
			CRigidElem* curre = parbone->GetRigidElemOfMap( filename, curbone );
			if( curre ){
				if( s_dmpanimLSlider ){
					s_dmpanimLSlider->setValue( curre->GetDampanimL() );
				}
				if( s_dmpanimASlider ){
					s_dmpanimASlider->setValue( curre->GetDampanimA() );
				}
			}
		}
	}

	s_dmpanimWnd->callRewrite();

	return 0;
}


int SetGpWndParams()
{
	if( !s_bpWorld ){
		return 0;
	}

	if( s_ghSlider ){
		s_ghSlider->setValue( s_bpWorld->m_gplaneh );
	}
	if( s_gsizexSlider ){
		s_gsizexSlider->setValue( s_bpWorld->m_gplanesize.x );
	}
	if( s_gsizezSlider ){
		s_gsizezSlider->setValue( s_bpWorld->m_gplanesize.y );
	}
	if( s_gpdisp ){
		s_gpdisp->setValue( (bool)s_bpWorld->m_gplanedisp );
	}

	if( s_grestSlider ){
		s_grestSlider->setValue( s_bpWorld->m_restitution );
	}
	if( s_gfricSlider ){
		s_gfricSlider->setValue( s_bpWorld->m_friction );
	}

	s_gpWnd->callRewrite();

	return 0;
}

int SaveProject()
{
	if( !s_bpWorld ){
		return 0;
	}

	if( s_modelindex.empty() ){
		return 0;
	}

	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_SAVECHADLG ), 
		s_mainwnd, (DLGPROC)SaveChaDlgProc );
	if( (dlgret != IDOK) || !s_projectname[0] || !s_projectdir[0] ){
		return 0;
	}


	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			s_owpLTimeline->setCurrentTime(0.0, false);
			curmodel->SetMotionFrame(0.0);
			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);

			//ここでAxisMatXの初期化
			curmodel->CreateBtObject(1);
			curmodel->CalcBtAxismat(2);//2
			curmodel->SetInitAxisMatX(1);
		}
	}



	CChaFile chafile;
	CallF( chafile.WriteChaFile( s_bpWorld, s_projectdir, s_projectname, s_modelindex, (float)g_dspeed ), return 1 );

	return 0;
}


LRESULT CALLBACK SaveChaDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

//	static WCHAR s_projectname[64] = {0L};
//	static WCHAR s_projectdir[MAX_PATH] = {0L};

	BROWSEINFO bi;
	LPITEMIDLIST curlpidl = 0;
	WCHAR dispname[MAX_PATH] = {0L};
	WCHAR selectname[MAX_PATH] = {0L};
	int iImage = 0;

	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetCurMotInfo() ){
				if( s_chasavename[0] ){
					SetDlgItemText( hDlgWnd, IDC_PROJNAME, s_chasavename );
				}
				if( s_chasavedir[0] ){
					SetDlgItemText( hDlgWnd, IDC_DIRNAME, s_chasavedir );
				}else{
					if( s_projectdir[0] ){
						SetDlgItemText( hDlgWnd, IDC_DIRNAME, s_projectdir );
					}else{
						LPITEMIDLIST pidl;

						HWND hWnd = NULL;

						IMalloc *pMalloc;
						SHGetMalloc( &pMalloc );

						if( SUCCEEDED(SHGetSpecialFolderLocation( s_mainwnd, CSIDL_DESKTOPDIRECTORY, &pidl )) )
						{ 
							// パスに変換する
							SHGetPathFromIDList( pidl, s_projectdir );
							// 取得したIDLを解放する (CoTaskMemFreeでも可)
							pMalloc->Free( pidl );              
							SetDlgItemText( hDlgWnd, IDC_DIRNAME, s_projectdir );
						}
						pMalloc->Release();
					}
				}
			}
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_PROJNAME, s_projectname, 64 );
					GetDlgItemText( hDlgWnd, IDC_DIRNAME, s_projectdir, MAX_PATH );
					wcscpy_s( s_chasavename, 64, s_projectname );
					wcscpy_s( s_chasavedir, MAX_PATH, s_projectdir );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					s_projectname[0] = 0L;
					s_projectdir[0] = 0L;
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
				case IDC_REFDIR:
					bi.hwndOwner = hDlgWnd;
					bi.pidlRoot = NULL;//!!!!!!!
					bi.pszDisplayName = dispname;
					bi.lpszTitle = L"保存フォルダを選択してください。";
					//bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = NULL;
					bi.lParam = 0;
					bi.iImage = iImage;

					curlpidl = SHBrowseForFolder( &bi );
					if( curlpidl ){
						//::MessageBox( m_hWnd, dispname, "フォルダー名", MB_OK );

						BOOL bret;
						bret = SHGetPathFromIDList( curlpidl, selectname );
						if( bret == FALSE ){
							_ASSERT( 0 );
							if( curlpidl )
								CoTaskMemFree( curlpidl );
							return 1;
						}

						if( curlpidl )
							CoTaskMemFree( curlpidl );

						wcscpy_s( s_projectdir, MAX_PATH, selectname );
						SetDlgItemText( hDlgWnd, IDC_DIRNAME, s_projectdir );
					}

					break;
				default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;

}

int OpenChaFile()
{
	WCHAR* lasten = 0;
	lasten = wcsrchr( g_tmpmqopath, TEXT( '\\' ) );
	if( lasten ){
		int leng = (int)wcslen( lasten + 1 );
		if( leng < 64 ){
			wcscpy_s( s_chasavename, 64, lasten + 1 );
			WCHAR* peri = wcsrchr( s_chasavename, TEXT( '.' ) );
			if( peri ){
				*peri = 0L;
			}
		}
	}


	WCHAR tmpdir[MAX_PATH];
	wcscpy_s( tmpdir, MAX_PATH, g_tmpmqopath );
	WCHAR* lastend = 0;
	lastend = wcsrchr( tmpdir, TEXT( '\\' ) );
	if( lastend ){
		*lastend = 0L;
		WCHAR* lastend2 = 0;
		lastend2 = wcsrchr( tmpdir, TEXT( '\\' ) );
		if( lastend2 ){
			*lastend2 = 0L;
			int dirleng = (int)( lastend2 - tmpdir );
			if( dirleng < MAX_PATH ){
				wcscpy_s( s_chasavedir, MAX_PATH, tmpdir );
			}
		}
	}

	if( !s_bpWorld ){
		//ChaMatrix inimat;
		//ChaMatrixIdentity( &inimat );
		//s_bpWorld = new BPWorld(NULL, inimat, "BtPiyo", // ウィンドウのタイトル
		//				460, 460,         // ウィンドウの幅と高さ [pixels]
		//				NULL);    // モニタリング用関数へのポインタ  
		//_ASSERT( s_bpWorld );

		s_bpWorld = new BPWorld(NULL, s_matWorld, "BtPiyo", // ウィンドウのタイトル
			460, 460,         // ウィンドウの幅と高さ [pixels]
			NULL);    // モニタリング用関数へのポインタ  
		_ASSERT(s_bpWorld);


		s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->setTimeStep(0.015);// seconds
		s_bpWorld->setGlobalERP(s_erp);// ERP
									   //s_bpWorld->start();// ウィンドウを表示して，シミュレーションを開始する
		s_btWorld = s_bpWorld->getDynamicsWorld();
		s_bpWorld->setNumThread(g_numthread);

	}

	HCURSOR oldcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	CChaFile chafile;
	CallF( chafile.LoadChaFile( g_tmpmqopath, OpenFBXFile, OpenREFile, OpenImpFile, OpenGcoFile, OnREMenu, OnRgdMenu, OnRgdMorphMenu, OnImpMenu ), return 1 );

	SetCursor(oldcursor);


	return 0;
}

int OnSetMotSpeed()
{
	s_model->GetCurMotInfo()->speed = s_model->GetTmpMotSpeed();//!!!!!!!!!!!!!!!!!!!
	g_dspeed = s_model->GetTmpMotSpeed();

	g_SampleUI.GetSlider( IDC_SPEED )->SetValue( (int)( g_dspeed * 100.0f ) );
	WCHAR sz[100];
	swprintf_s( sz, 100, L"Motion Speed: %0.4f", g_dspeed );
    g_SampleUI.GetStatic( IDC_SPEED_STATIC )->SetText( sz );

	return 0;
}

CRigidElem* GetCurRe()
{
	CRigidElem* retre = 0;

	if( s_model && (s_curboneno >= 0) && (s_curreindex >= 0) ){
		CBone* curbone = s_model->GetBoneByID( s_curboneno );
		if( curbone ){
			CBone* parbone = curbone->GetParent();
			if( parbone ){
				char* filename = s_model->GetRigidElemInfo( s_curreindex ).filename;
				CRigidElem* curre = parbone->GetRigidElemOfMap( filename, curbone );
				if( curre ){
					retre = curre;
				}else{
					retre = 0;
				}
			}else{
				retre = 0;
			}
		}else{
			retre = 0;
		}
	}else{
		retre = 0;
	}

	return retre;
}

CRigidElem* GetCurRgdRe()
{
	CRigidElem* retre = 0;

	if( s_model && (s_curboneno >= 0) && (s_rgdindex >= 0) ){
		CBone* curbone = s_model->GetBoneByID( s_curboneno );
		if( curbone ){
			CBone* parbone = curbone->GetParent();
			if( parbone ){
				char* filename = s_model->GetRigidElemInfo( s_rgdindex ).filename;
				CRigidElem* curre = parbone->GetRigidElemOfMap( filename, curbone );
				if( curre ){
					retre = curre;
				}else{
					retre = 0;
				}
			}else{
				retre = 0;
			}
		}else{
			retre = 0;
		}
	}else{
		retre = 0;
	}

	return retre;
}

int SetSpAxisParams()
{
	if( !(s_spaxis[0].sprite) ){
		return 0;
	}

	float spawidth = 32.0f;
	int spashift = 12;
	s_spaxis[0].dispcenter.x = (int)( s_mainwidth * 0.57f );
	s_spaxis[0].dispcenter.y = (int)( 30.0f * ( (float)s_mainheight / 620.0 ) );
	spashift = (int)( (float)spashift * ( (float)s_mainwidth / 600.0 ) ); 

	s_spaxis[1].dispcenter.x = s_spaxis[0].dispcenter.x + (int)( spawidth ) + spashift;
	s_spaxis[1].dispcenter.y = s_spaxis[0].dispcenter.y;

	s_spaxis[2].dispcenter.x = s_spaxis[1].dispcenter.x + (int)( spawidth ) + spashift;
	s_spaxis[2].dispcenter.y = s_spaxis[0].dispcenter.y;

	int spacnt;
	for( spacnt = 0; spacnt < 3; spacnt++ ){
		ChaVector3 disppos;
		disppos.x = (float)( s_spaxis[spacnt].dispcenter.x ) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)( s_spaxis[spacnt].dispcenter.y ) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2( spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f );
		CallF( s_spaxis[spacnt].sprite->SetPos( disppos ), return 1 );
		CallF( s_spaxis[spacnt].sprite->SetSize( dispsize ), return 1 );
	}

	return 0;

}

int SetSpCamParams()
{
	if (!(s_spcam[SPR_CAM_I].sprite) || !(s_spcam[SPR_CAM_KAI].sprite) || !(s_spcam[SPR_CAM_KAKU].sprite)){
		return 0;
	}

	float spawidth = 32.0f;
	int spashift = 12;
	s_spcam[0].dispcenter.x = (int)(s_mainwidth * 0.57f);
	s_spcam[0].dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0)) + (int(spawidth * 1.5f));
	spashift = (int)((float)spashift * ((float)s_mainwidth / 600.0));

	s_spcam[1].dispcenter.x = s_spcam[0].dispcenter.x + (int)(spawidth)+spashift;
	s_spcam[1].dispcenter.y = s_spcam[0].dispcenter.y;

	s_spcam[2].dispcenter.x = s_spcam[1].dispcenter.x + (int)(spawidth)+spashift;
	s_spcam[2].dispcenter.y = s_spcam[0].dispcenter.y;

	int spacnt;
	for (spacnt = 0; spacnt < 3; spacnt++){
		ChaVector3 disppos;
		disppos.x = (float)(s_spcam[spacnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spcam[spacnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
		CallF(s_spcam[spacnt].sprite->SetPos(disppos), return 1);
		CallF(s_spcam[spacnt].sprite->SetSize(dispsize), return 1);
	}

	return 0;

}

int SetSpRigParams()
{
	if (!(s_sprig[0].sprite) || !(s_sprig[1].sprite)){
		return 0;
	}

	float spawidth = 32.0f;
	int spashift = 12;
	spashift = (int)((float)spashift * ((float)s_mainwidth / 600.0));
	s_sprig[0].dispcenter.x = (int)(s_mainwidth * 0.57f) + ((int)(spawidth)+spashift) * 3;
	s_sprig[0].dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0));// +(int(spawidth * 1.5f) * 2);

	s_sprig[1].dispcenter = s_sprig[0].dispcenter;


	ChaVector3 disppos;
	disppos.x = (float)(s_sprig[0].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_sprig[0].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
	CallF(s_sprig[0].sprite->SetPos(disppos), return 1);
	CallF(s_sprig[0].sprite->SetSize(dispsize), return 1);
	CallF(s_sprig[1].sprite->SetPos(disppos), return 1);
	CallF(s_sprig[1].sprite->SetSize(dispsize), return 1);

	return 0;

}

int SetSpBtParams()
{
	if (!s_spbt.sprite){
		return 0;
	}

	float spawidth = 32.0f;
	int spashift = 12;
	spashift = (int)((float)spashift * ((float)s_mainwidth / 600.0));
	s_spbt.dispcenter.x = (int)(s_mainwidth * 0.57f) + ((int)(spawidth)+spashift) * 3;
	s_spbt.dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0)) + (int(spawidth * 1.5f));// *2);


	ChaVector3 disppos;
	disppos.x = (float)(s_spbt.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_spbt.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
	CallF(s_spbt.sprite->SetPos(disppos), return 1);
	CallF(s_spbt.sprite->SetSize(dispsize), return 1);

	return 0;

}



int PickSpAxis( POINT srcpos )
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	int starty = s_spaxis[0].dispcenter.y - 16;
	int endy = starty + 32;

	if( (srcpos.y >= starty) && (srcpos.y <= endy) ){
		int spacnt;
		for( spacnt = 0; spacnt < 3; spacnt++ ){
			int startx = s_spaxis[spacnt].dispcenter.x - 16;
			int endx = startx + 32;

			if( (srcpos.x >= startx) && (srcpos.x <= endx) ){
				switch( spacnt ){
				case 0:
					kind = PICK_SPA_X;
					break;
				case 1:
					kind = PICK_SPA_Y;
					break;
				case 2:
					kind = PICK_SPA_Z;
					break;
				}
				break;
			}
		}
	}


//DbgOut( L"pickspaxis : kind %d, mouse (%d, %d), starty %d, endy %d\r\n",
//	kind, srcpos.x, srcpos.y, starty, endy );
//int spacnt;
//for( spacnt = 0; spacnt < 3; spacnt++ ){
//	DbgOut( L"\tspa %d : startx %d, endx %d\r\n", spacnt, s_spaxis[spacnt].dispcenter.x, s_spaxis[spacnt].dispcenter.x + 32 );
//}

	return kind;
}

int PickSpCam(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	int starty = s_spcam[SPR_CAM_I].dispcenter.y - 16;
	int endy = starty + 32;

	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
		int spacnt;
		for (spacnt = 0; spacnt < 3; spacnt++){
			int startx = s_spcam[spacnt].dispcenter.x - 16;
			int endx = startx + 32;

			if ((srcpos.x >= startx) && (srcpos.x <= endx)){
				switch (spacnt){
				case 0:
					kind = PICK_CAMMOVE;
					break;
				case 1:
					kind = PICK_CAMROT;
					break;
				case 2:
					kind = PICK_CAMDIST;
					break;
				}
				break;
			}
		}
	}

	//DbgOut( L"pickspaxis : kind %d, mouse (%d, %d), starty %d, endy %d\r\n",
	//	kind, srcpos.x, srcpos.y, starty, endy );
	//int spacnt;
	//for( spacnt = 0; spacnt < 3; spacnt++ ){
	//	DbgOut( L"\tspa %d : startx %d, endx %d\r\n", spacnt, s_spaxis[spacnt].dispcenter.x, s_spaxis[spacnt].dispcenter.x + 32 );
	//}

	return kind;
}


int PickSpRig(POINT srcpos)
{
	int pickflag = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	if (s_sprig[0].sprite == 0){
		return 0;
	}

	int starty = s_sprig[0].dispcenter.y - 16;
	int endy = starty + 32;

	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
		int startx = s_sprig[0].dispcenter.x - 16;
		int endx = startx + 32;

		if ((srcpos.x >= startx) && (srcpos.x <= endx)){
			pickflag = 1;
		}
	}

	return pickflag;
}


int PickSpBt(POINT srcpos)
{
	int pickflag = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	if (s_spbt.sprite == 0){
		return 0;
	}

	int starty = s_spbt.dispcenter.y - 16;
	int endy = starty + 32;

	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
		int startx = s_spbt.dispcenter.x - 16;
		int endx = startx + 32;

		if ((srcpos.x >= startx) && (srcpos.x <= endx)){
			pickflag = 1;
		}
	}

	return pickflag;
}

int SetSelectState()
{
	//if( !s_select || !s_model || g_previewFlag ){
	if (!s_select || !s_model || (g_previewFlag == 4)){
		return 0;
	}

	if( s_ikkind == 0 ){
		s_select->SetDispFlag( "ringX", 1 );
		s_select->SetDispFlag( "ringY", 1 );
		s_select->SetDispFlag( "ringZ", 1 );
	}else if( s_ikkind == 1 ){
		s_select->SetDispFlag( "ringX", 0 );
		s_select->SetDispFlag( "ringY", 0 );
		s_select->SetDispFlag( "ringZ", 0 );
	}
////////
	PICKINFO pickinfo;
	ZeroMemory( &pickinfo, sizeof( PICKINFO ) );
	POINT ptCursor;
	GetCursorPos( &ptCursor );
	::ScreenToClient( s_mainwnd, &ptCursor );
	pickinfo.clickpos = ptCursor;
	pickinfo.mousepos = ptCursor;
	pickinfo.mousebefpos = ptCursor;
	pickinfo.diffmouse = ChaVector2( 0.0f, 0.0f );

	pickinfo.winx = (int)DXUTGetWindowWidth();
	pickinfo.winy = (int)DXUTGetWindowHeight();
	pickinfo.pickrange = 6;
	pickinfo.buttonflag = 0;

	//pickinfo.pickobjno = s_curboneno;
	pickinfo.pickobjno = -1;

	int spakind = PickSpAxis( ptCursor );
	int spckind = PickSpCam(ptCursor);
	if (spakind != 0){
		pickinfo.pickobjno = s_curboneno;
		pickinfo.buttonflag = spakind;
	} else if (spckind != 0){
		pickinfo.pickobjno = s_curboneno;
		pickinfo.buttonflag = spckind;
	}else{
	
		if( g_shiftkey == false ){
			CallF( s_model->PickBone( &pickinfo ), return 1 );
		}

		if( pickinfo.pickobjno >= 0 ){
			pickinfo.buttonflag = PICK_CENTER;//!!!!!!!!!!!!!
		}else{
			if( s_dispselect ){
				int colliobjx, colliobjy, colliobjz, colliringx, colliringy, colliringz;
				colliobjx = 0;
				colliobjy = 0;
				colliobjz = 0;
				colliringx = 0;
				colliringy = 0;
				colliringz = 0;
						
				colliobjx = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "objX" );
				colliobjy = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "objY" );
				colliobjz = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "objZ" );
				if( s_ikkind == 0 ){
					colliringx = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "ringX" );
					colliringy = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "ringY" );
					colliringz = s_select->CollisionNoBoneObj_Mouse( &pickinfo, "ringZ" );
				}else{
					colliringx = 0;
					colliringy = 0;
					colliringz = 0;
				}
				if( colliobjx || colliringx || colliobjy || colliringy || colliobjz || colliringz ){
					pickinfo.pickobjno = s_curboneno;
				}

				if( colliobjx || colliringx ){
					pickinfo.buttonflag = PICK_X;
				}else if( colliobjy || colliringy ){
					pickinfo.buttonflag = PICK_Y;
				}
				else if (colliobjz || colliringz){
					pickinfo.buttonflag = PICK_Z;
				}else{
					ZeroMemory( &pickinfo, sizeof( PICKINFO ) );
					pickinfo.pickobjno = -1;
				}
			}else{
				ZeroMemory( &pickinfo, sizeof( PICKINFO ) );
				pickinfo.pickobjno = -1;
			}
		}
	}

	if( s_pickinfo.buttonflag != 0 ){
		return 0;//!!!!!!!!!!!!!!!!!!!!!!!
	}

	float hirate = 1.0f;
	float lowrate = 0.6f;

	float hia = 0.3f;
	float lowa = 0.3f;

	if (s_matred && s_ringred && s_matblue && s_ringblue && s_matgreen && s_ringgreen && s_matyellow){
		if( (pickinfo.pickobjno >= 0) && (s_curboneno == pickinfo.pickobjno) ){
			
			if( (pickinfo.buttonflag == PICK_X) || (pickinfo.buttonflag == PICK_SPA_X) ){//red
				s_matred->SetDif4F(s_matredmat * hirate);
				s_ringred->SetDif4F(s_ringredmat * hirate);
				s_matred->SetDif4FW(hia);
				s_ringred->SetDif4FW(hia);

				s_matblue->SetDif4F(s_matbluemat * lowrate);
				s_ringblue->SetDif4F(s_ringbluemat * lowrate);
				s_matblue->SetDif4FW(lowa);
				s_ringblue->SetDif4FW(lowa);

				s_matgreen->SetDif4F(s_matgreenmat * lowrate);
				s_ringgreen->SetDif4F(s_ringgreenmat * lowrate);
				s_matgreen->SetDif4FW(lowa);
				s_ringgreen->SetDif4FW(lowa);

				s_matyellow->SetDif4F(s_matyellowmat * lowrate);
				s_matyellow->SetDif4FW(lowa);
			}else if( (pickinfo.buttonflag == PICK_Y) || (pickinfo.buttonflag == PICK_SPA_Y) ){//green
				s_matred->SetDif4F(s_matredmat * lowrate);
				s_ringred->SetDif4F(s_ringredmat * lowrate);
				s_matred->SetDif4FW(lowa);
				s_ringred->SetDif4FW(lowa);

				s_matblue->SetDif4F(s_matbluemat * lowrate);
				s_ringblue->SetDif4F(s_ringbluemat * lowrate);
				s_matblue->SetDif4FW(lowa);
				s_ringblue->SetDif4FW(lowa);

				s_matgreen->SetDif4F(s_matgreenmat * hirate);
				s_ringgreen->SetDif4F(s_ringgreenmat * hirate);
				s_matgreen->SetDif4FW(hia);
				s_ringgreen->SetDif4FW(hia);

				s_matyellow->SetDif4F(s_matyellowmat * lowrate);
				s_matyellow->SetDif4FW(lowa);

			}else if( (pickinfo.buttonflag == PICK_Z) || (pickinfo.buttonflag == PICK_SPA_Z) ){//blue
				s_matred->SetDif4F(s_matredmat * lowrate);
				s_ringred->SetDif4F(s_ringredmat * lowrate);
				s_matred->SetDif4FW(lowa);
				s_ringred->SetDif4FW(lowa);

				s_matblue->SetDif4F(s_matbluemat * hirate);
				s_ringblue->SetDif4F(s_ringbluemat * hirate);
				s_matblue->SetDif4FW(hia);
				s_ringblue->SetDif4FW(hia);

				s_matgreen->SetDif4F(s_matgreenmat * lowrate);
				s_ringgreen->SetDif4F(s_ringgreenmat * lowrate);
				s_matgreen->SetDif4FW(lowa);
				s_ringgreen->SetDif4FW(lowa);

				s_matyellow->SetDif4F(s_matyellowmat * lowrate);
				s_matyellow->SetDif4FW(lowa);

			}else if( pickinfo.buttonflag == PICK_CENTER ){//yellow
				s_matred->SetDif4F(s_matredmat * lowrate);
				s_ringred->SetDif4F(s_ringredmat * lowrate);
				s_matred->SetDif4FW(lowa);
				s_ringred->SetDif4FW(lowa);

				s_matblue->SetDif4F(s_matbluemat * lowrate);
				s_ringblue->SetDif4F(s_ringbluemat * lowrate);
				s_matblue->SetDif4FW(lowa);
				s_ringblue->SetDif4FW(lowa);

				s_matgreen->SetDif4F(s_matgreenmat * lowrate);
				s_ringgreen->SetDif4F(s_ringgreenmat * lowrate);
				s_matgreen->SetDif4FW(lowa);
				s_ringgreen->SetDif4FW(lowa);

				s_matyellow->SetDif4F(s_matyellowmat * hirate);
				s_matyellow->SetDif4FW(hia);
			}
		}else{
			s_matred->SetDif4F(s_matredmat * lowrate);
			s_ringred->SetDif4F(s_ringredmat * lowrate);
			s_matred->SetDif4FW(lowa);
			s_ringred->SetDif4FW(lowa);

			s_matblue->SetDif4F(s_matbluemat * lowrate);
			s_ringblue->SetDif4F(s_ringbluemat * lowrate);
			s_matblue->SetDif4FW(lowa);
			s_ringblue->SetDif4FW(lowa);

			s_matgreen->SetDif4F(s_matgreenmat * lowrate);
			s_ringgreen->SetDif4F(s_ringgreenmat * lowrate);
			s_matgreen->SetDif4FW(lowa);
			s_ringgreen->SetDif4FW(lowa);

			s_matyellow->SetDif4F(s_matyellowmat * lowrate);
			s_matyellow->SetDif4FW(lowa);
		}
	}

	return 0;
}

int CreateTimeLineMark( int topboneno )
{
	if( s_model && s_owpTimeline && s_owpLTimeline ){
		if( topboneno < 0 ){
			CreateMarkReq( s_editmotionflag, 0 );
		}else{
			CreateMarkReq( topboneno, 0 );
		}

		SetTimelineMark();

		s_owpTimeline->callRewrite();
		s_owpTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開

		s_owpLTimeline->callRewrite();
		s_owpLTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開
	}
	return 0;
}

int SetTimelineMark()
{
	if( !s_model || !s_owpTimeline || !s_owpLTimeline ){
		_ASSERT( 0 );
		return 0;
	}


	s_owpTimeline->setRewriteOnChangeFlag(false);		//再描画要求を再開


//	s_owpTimeline->deleteKey();

	map<int,CBone*>::iterator itrbone;
	for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			int curlineno = s_boneno2lineno[ curbone->GetBoneNo() ];
			if( curlineno >= 0 ){
				double curframe;
				for( curframe = 0.0; curframe < s_model->GetCurMotInfo()->frameleng; curframe += 1.0 ){
					KeyInfo chkki = s_owpTimeline->ExistKey( curlineno, curframe );
					if( chkki.lineIndex >= 0 ){
						s_owpTimeline->deleteKey( curlineno, curframe );
					}
				}
			}
		}
	}

	for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			int curlineno = s_boneno2lineno[ curbone->GetBoneNo() ];
			if( curlineno >= 0 ){
				map<int, map<double, int>>::iterator itrcur;
				itrcur = curbone->FindMotMarkOfMap( s_model->GetCurMotInfo()->motid );
				if( itrcur != curbone->GetMotMarkOfMapBegin() ){
					map<double, int>::iterator itrmark;
					for( itrmark = itrcur->second.begin(); itrmark != itrcur->second.end(); itrmark++ ){
						double curframe = itrmark->first;
						s_owpTimeline->newKey( curbone->GetWBoneName(), curframe, 0 );
					}
				}
			}
		}
	}

	s_owpTimeline->callRewrite();
	s_owpTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開

	return 0;
}


void CreateMarkReq( int curboneno, int broflag )
{
	if( curboneno < 0 ){
		return;
	}
	CBone* curbone = s_model->GetBoneByID( curboneno );
	if( curbone ){
		int curlineno = s_boneno2lineno[ curboneno ];
		if( curlineno >= 0 ){
			int keynum;
			double startframe, endframe;
			s_editrange.GetRange( &keynum, &startframe, &endframe );
			s_model->AddBoneMotMark( s_owpTimeline, curboneno, curlineno, startframe, endframe, 1 );
		}
	}

	if( curbone->GetChild() ){
		CreateMarkReq( curbone->GetChild()->GetBoneNo(), 1 );
	}
	if( broflag && curbone->GetBrother() ){
		CreateMarkReq( curbone->GetBrother()->GetBoneNo(), 1 );
	}
}

int SetLTimelineMark( int curboneno )
{
	if( (curboneno >= 0) && s_model && s_owpTimeline && s_owpLTimeline ){
		CBone* curbone = s_model->GetBoneByID( curboneno );
		if( curbone ){
			int curlineno = s_boneno2lineno[ curboneno ];
			if( curlineno >= 0 ){
				s_owpLTimeline->deleteLine( 2 );

				WCHAR markname[256] = {0L};
				swprintf_s( markname, 256, L"Mark:%s", curbone->GetWBoneName() );
				s_owpLTimeline->newLine(0, 0, markname);

				double frame;
				for( frame = 0.0; frame < s_model->GetCurMotInfo()->frameleng; frame += 1.0 ){
					KeyInfo chkki = s_owpTimeline->ExistKey( curlineno, frame );
					if( chkki.lineIndex >= 0 ){
						s_owpLTimeline->newKey( markname, frame, 0 );
					}
				}
			}
		}
	}

	s_owpLTimeline->callRewrite();
	s_owpTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開

	return 0;
}

int ExportFBXFile()
{
	if( !s_model || !s_owpLTimeline ){
		_ASSERT( 0 );
		return 0;
	}

	g_previewFlag = 0; 
	s_owpLTimeline->setCurrentTime( 0.0, false );

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel && curmodel->GetCurMotInfo() ){
			curmodel->SetMotionFrame( 0.0 );
		}
	}

	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel ){
			curmodel->UpdateMatrix( &curmodel->GetWorldMat(), &s_matVP );
		}
	}

	WCHAR filename[MAX_PATH]={0L};
	OPENFILENAME ofn1;
	ofn1.lStructSize = sizeof(OPENFILENAME);
	ofn1.hwndOwner = s_mainwnd;
	ofn1.hInstance = 0;
	ofn1.lpstrFilter = L"FBX file(*.fbx)\0";
	ofn1.lpstrCustomFilter = NULL;
	ofn1.nMaxCustFilter = 0;
	ofn1.nFilterIndex = 0;
	ofn1.lpstrFile = filename;
	ofn1.nMaxFile = MAX_PATH;
	ofn1.lpstrFileTitle = NULL;
	ofn1.nMaxFileTitle = 0;
	ofn1.lpstrInitialDir = NULL;
	ofn1.lpstrTitle = NULL;
	ofn1.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn1.nFileOffset = 0;
	ofn1.nFileExtension = 0;
	ofn1.lpstrDefExt = L"fbx";
	ofn1.lCustData = NULL;
	ofn1.lpfnHook = NULL;
	ofn1.lpTemplateName = NULL;

	if( GetSaveFileNameW(&ofn1) != IDOK ){
		return 0;
	}
	
	char fbxpath[MAX_PATH] = {0};
	WideCharToMultiByte( CP_UTF8, 0, filename, -1, fbxpath, MAX_PATH, NULL, NULL );	


	{
		s_owpLTimeline->setCurrentTime(0.0, false);
		s_model->SetMotionFrame(0.0);
		s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);

		//ここでAxisMatXの初期化
		s_model->CreateBtObject(1);
		s_model->CalcBtAxismat(2);//2
		s_model->SetInitAxisMatX(1);
	}


	//CallF( WriteFBXFile( s_model, fbxpath, s_dummytri, mb, g_tmpmqomult, s_fbxbunki ), return 1 );
	CallF( WriteFBXFile( s_psdk, s_model, fbxpath ), return 1 );


	if (s_model->GetOldAxisFlagAtLoading() == 0){
		WCHAR lmtname[MAX_PATH] = { 0L };
		swprintf_s(lmtname, MAX_PATH, L"%s.lmt", filename);
		CLmtFile lmtfile;
		lmtfile.WriteLmtFile(lmtname, s_model);

		WCHAR rigname[MAX_PATH] = { 0L };
		swprintf_s(rigname, MAX_PATH, L"%s.rig", filename);
		CRigFile rigfile;
		rigfile.WriteRigFile(rigname, s_model);

	}


	return 0;
}

int ExportBntFile()
{
	if( !s_model || !s_owpLTimeline ){
		_ASSERT( 0 );
		return 0;
	}

	g_previewFlag = 0; 
	s_owpLTimeline->setCurrentTime( 0.0, false );

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel && curmodel->GetCurMotInfo() ){
			curmodel->SetMotionFrame( 0.0 );
		}
	}

	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel ){
			curmodel->UpdateMatrix( &curmodel->GetWorldMat(), &s_matVP );
		}
	}


	WCHAR filename[MAX_PATH]={0L};
	OPENFILENAME ofn1;
	ofn1.lStructSize = sizeof(OPENFILENAME);
	ofn1.hwndOwner = s_mainwnd;
	ofn1.hInstance = 0;
	ofn1.lpstrFilter = L"BNT file(*.bnt)\0";
	ofn1.lpstrCustomFilter = NULL;
	ofn1.nMaxCustFilter = 0;
	ofn1.nFilterIndex = 0;
	ofn1.lpstrFile = filename;
	ofn1.nMaxFile = MAX_PATH;
	ofn1.lpstrFileTitle = NULL;
	ofn1.nMaxFileTitle = 0;
	ofn1.lpstrInitialDir = NULL;
	ofn1.lpstrTitle = NULL;
	ofn1.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn1.nFileOffset = 0;
	ofn1.nFileExtension = 0;
	ofn1.lpstrDefExt = L"bnt";
	ofn1.lCustData = NULL;
	ofn1.lpfnHook = NULL;
	ofn1.lpTemplateName = NULL;

	if( GetSaveFileNameW(&ofn1) != IDOK ){
		return 0;
	}
	
	//char fbxpath[MAX_PATH] = {0};
	//WideCharToMultiByte( CP_UTF8, 0, filename, -1, fbxpath, MAX_PATH, NULL, NULL );	

	if( s_modelindex.empty() ){
		_ASSERT( 0 );
		return 0;
	}
	MODELELEM wme = s_modelindex[0];
	CModel* curmodel = wme.modelptr;
	if( !curmodel ){
		return 0;
	}

	CBntFile bntfile;
	CallF( bntfile.WriteBntFile( filename, wme ), return 1 );

	return 0;
}





int AddEditRangeHistory()
{
	static int s_historycnt = 0;

	if ((s_editrangehistoryno < 0) || (s_editrangehistoryno >= EDITRANGEHISTORYNUM)){
		_ASSERT(0);
		s_editrangehistoryno = 0;
	}
	if ((s_editrangesetindex < 0) || (s_editrangesetindex >= EDITRANGEHISTORYNUM)){
		_ASSERT(0);
		s_editrangesetindex = 0;
	}

	if (s_editrange.IsSameStartAndEnd()){
		//_ASSERT(0);
		return 0;
	}


	int findflag = 0;
	int erhno;
	for (erhno = 0; erhno < EDITRANGEHISTORYNUM; erhno++){
		if (*(s_editrangehistory + erhno) == s_editrange){
			findflag++;
			break;
		}
	}

	if (findflag == 0){
		s_editrangesetindex += 1;
		if (s_editrangesetindex >= EDITRANGEHISTORYNUM){
			s_editrangesetindex = 0;
		}

		*(s_editrangehistory + s_editrangesetindex) = s_editrange;
		(s_editrangehistory + s_editrangesetindex)->SetSetFlag(1);
		(s_editrangehistory + s_editrangesetindex)->SetSetCnt(s_historycnt);
		s_historycnt++;

		s_editrangehistoryno = s_editrangesetindex;
	}
	return 0;
}

int RollBackEditRange(int prevrangeFlag, int nextrangeFlag)
{
	if ((s_editrangehistoryno < 0) || (s_editrangehistoryno >= EDITRANGEHISTORYNUM)){
		_ASSERT(0);
		s_editrangehistoryno = 0;
	}

	int findindex = -1;
	int erhcnt;
	int curindex = s_editrangehistoryno;

	if (prevrangeFlag && (s_editrange.IsSameStartAndEnd())){
		//prevボタンのとき　範囲が解除されている場合は現状復帰のためインデックスはそのまま
		if ((s_editrangehistory + curindex)->GetSetFlag() == 1){
			findindex = curindex;
		}
	}
	else{
		for (erhcnt = 0; erhcnt < EDITRANGEHISTORYNUM; erhcnt++){
			if (prevrangeFlag){
				curindex -= 1;
			}
			else if (nextrangeFlag){
				curindex += 1;
			}
			else{
				_ASSERT(0);
				break;
			}
			if (curindex < 0){
				curindex = EDITRANGEHISTORYNUM - 1;
			}
			if (curindex >= EDITRANGEHISTORYNUM){
				curindex = 0;
			}
			if ((s_editrangehistory + curindex)->GetSetFlag() == 1){
				findindex = curindex;
				break;
			}
		}
	}

	if (findindex >= 0){
		s_editrange = *(s_editrangehistory + findindex);
		s_editrangehistoryno = findindex;
	}

	return 0;
}


int RecalcBoneAxisX(CBone* srcbone)
{
	if (s_model && (s_model->GetOldAxisFlagAtLoading() == 1)){
		::MessageBox(s_mainwnd, L"旧型データを新型データにしてから(保存しなおして読み込んでから)\n実行しなおしてください。", L"データタイプエラー", MB_OK);
		return 0;
	}

	s_model->RecalcBoneAxisX(srcbone);

	return 0;
}



int DispAngleLimitDlg()
{
	if (s_anglelimitdlg){
		//already opened
		return 0;
	}
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	if (s_model->GetOldAxisFlagAtLoading() == 1){
		::MessageBox(s_mainwnd, L"座標軸が設定してあるデータでのみ機能します。\nFBXファイルを保存しなおしてから再試行してください。", L"データタイプエラー", MB_OK);
		return 0;
	}


	Bone2AngleLimit();

	/*
	int dlgret;
	dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANGLELIMITDLG),
		s_mainwnd, (DLGPROC)AngleLimitDlgProc);
	if (dlgret != IDOK){
		return 0;
	}
	*/
	s_anglelimitdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANGLELIMITDLG), s_mainwnd, (DLGPROC)AngleLimitDlgProc);
	if (!s_anglelimitdlg){
		_ASSERT(0);
		return 1;
	}
	ShowWindow(s_anglelimitdlg, SW_SHOW);
	UpdateWindow(s_anglelimitdlg);

	//AngleLimit2Bone();

	return 0;
}

int Bone2AngleLimit()
{
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	s_anglelimitbone = s_model->GetBoneByID(s_curboneno);
	if (s_anglelimitbone){
		s_anglelimit = s_anglelimitbone->GetAngleLimit();
	}
	else{
		_ASSERT(0);
		InitAngleLimit(&s_anglelimit);
	}

	return 0;
}

int AngleLimit2Bone()
{
	if (!s_model){
		return 0;
	}
	if (!s_anglelimitbone){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	if (s_anglelimitbone){
		s_anglelimitbone->SetAngleLimit(s_anglelimit);
	}
	else{
		_ASSERT(0);
	}

	return 0;
}
int InitAngleLimitSlider(HWND hDlgWnd, int slresid, int txtresid, int srclimit)
{
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)-180);
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)180);

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_CLEARTICS, 0, 0);
	int tickcnt;
	for (tickcnt = 0; tickcnt <= 36; tickcnt++){
		int tickval = -180 + 10 * tickcnt;
		SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETTIC, 1, (LPARAM)(LONG)tickval);
	}

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)srclimit);

	WCHAR strval[256] = {0L};
	swprintf_s(strval, 256, L"%d", srclimit);
	SetDlgItemText(hDlgWnd, txtresid, (LPCWSTR)strval);

	return 0;
}

int GetAngleLimitSliderVal(HWND hDlgWnd, int slresid, int txtresid, int* dstptr)
{
	int curval = (int)SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_GETPOS, 0, 0);
	*dstptr = curval;
	WCHAR strval[256];
	swprintf_s(strval, 256, L"%d", curval);
	SetDlgItemText(hDlgWnd, txtresid, (LPCWSTR)strval);

	return 0;
}

int AngleLimit2Dlg(HWND hDlgWnd)
{
	if (s_anglelimitbone){
		SetDlgItemText(hDlgWnd, IDC_BONENAME, (LPCWSTR)s_anglelimitbone->GetWBoneName());


		SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_RESETCONTENT, 0, 0);
		WCHAR strcombo[256];
		wcscpy_s(strcombo, 256, L"CurrentBoneAxis");
		SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_ADDSTRING, 0, (LPARAM)strcombo);
		wcscpy_s(strcombo, 256, L"ParentBoneAxis");
		SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_ADDSTRING, 0, (LPARAM)strcombo);
		wcscpy_s(strcombo, 256, L"GlobalBoneAxis");
		SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_ADDSTRING, 0, (LPARAM)strcombo);
		SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_SETCURSEL, s_anglelimit.boneaxiskind, 0);


		InitAngleLimitSlider(hDlgWnd, IDC_SLXL, IDC_XLVAL, s_anglelimit.lower[AXIS_X]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLXU, IDC_XUVAL, s_anglelimit.upper[AXIS_X]);

		InitAngleLimitSlider(hDlgWnd, IDC_SLYL, IDC_YLVAL, s_anglelimit.lower[AXIS_Y]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLYU, IDC_YUVAL, s_anglelimit.upper[AXIS_Y]);

		InitAngleLimitSlider(hDlgWnd, IDC_SLZL, IDC_ZLVAL, s_anglelimit.lower[AXIS_Z]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLZU, IDC_ZUVAL, s_anglelimit.upper[AXIS_Z]);

		if (s_anglelimit.via180flag[0] == 1){
			CheckDlgButton(hDlgWnd, IDC_CHECKX, BST_CHECKED);
		}
		else{
			CheckDlgButton(hDlgWnd, IDC_CHECKX, BST_UNCHECKED);
		}
		if (s_anglelimit.via180flag[1] == 1){
			CheckDlgButton(hDlgWnd, IDC_CHECKY, BST_CHECKED);
		}
		else{
			CheckDlgButton(hDlgWnd, IDC_CHECKY, BST_UNCHECKED);
		}
		if (s_anglelimit.via180flag[2] == 1){
			CheckDlgButton(hDlgWnd, IDC_CHECKZ, BST_CHECKED);
		}
		else{
			CheckDlgButton(hDlgWnd, IDC_CHECKZ, BST_UNCHECKED);
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}

LRESULT CALLBACK AngleLimitDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			AngleLimit2Dlg(hDlgWnd);
			return FALSE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_BONEAXIS:
			{
				int combono = (int)SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_GETCURSEL, 0, 0);
				if ((combono >= BONEAXIS_CURRENT) && (combono <= BONEAXIS_GLOBAL)){
					s_anglelimit.boneaxiskind = combono;
				}
			}
			break;
		case IDOK:
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKX) == BST_CHECKED){
				s_anglelimit.via180flag[AXIS_X] = 1;
			}
			else{
				s_anglelimit.via180flag[AXIS_X] = 0;
			}
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKY) == BST_CHECKED){
				s_anglelimit.via180flag[AXIS_Y] = 1;
			}
			else{
				s_anglelimit.via180flag[AXIS_Y] = 0;
			}
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKZ) == BST_CHECKED){
				s_anglelimit.via180flag[AXIS_Z] = 1;
			}
			else{
				s_anglelimit.via180flag[AXIS_Z] = 0;
			}

			AngleLimit2Bone();
			
			//読み込みなおし：lowerとupperは大小関係で入れ替わることがあるため適用後読み込みなおす。
			Bone2AngleLimit();
			//AngleLimit2Dlg(s_anglelimitdlg);
			ChangeCurrentBone();

			//EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			//EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			return FALSE;
		}
		break;
	case WM_CLOSE:
		if (s_anglelimitdlg){
			DestroyWindow(s_anglelimitdlg);
			s_anglelimitdlg = 0;
		}
		break;
	case WM_HSCROLL:
		{
			HWND ctrlwnd;
			ctrlwnd = (HWND)lp;

			if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLXL)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLXL, IDC_XLVAL, &(s_anglelimit.lower[AXIS_X]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLXU)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLXU, IDC_XUVAL, &(s_anglelimit.upper[AXIS_X]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLYL)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLYL, IDC_YLVAL, &(s_anglelimit.lower[AXIS_Y]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLYU)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLYU, IDC_YUVAL, &(s_anglelimit.upper[AXIS_Y]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLZL)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLZL, IDC_ZLVAL, &(s_anglelimit.lower[AXIS_Z]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLZU)){
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLZU, IDC_ZUVAL, &(s_anglelimit.upper[AXIS_Z]));
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;

}

int DispRotAxisDlg()
{
	if (s_rotaxisdlg){
		//already opened
		return 0;
	}
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	if (s_model->GetOldAxisFlagAtLoading() == 1){
		::MessageBox(s_mainwnd, L"座標軸が設定してあるデータでのみ機能します。\nFBXファイルを保存しなおしてから再試行してください。", L"データタイプエラー", MB_OK);
		return 0;
	}


	s_rotaxisdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ROTAXISDLG), s_mainwnd, (DLGPROC)RotAxisDlgProc);
	if (!s_rotaxisdlg){
		_ASSERT(0);
		return 1;
	}
	ShowWindow(s_rotaxisdlg, SW_SHOW);
	UpdateWindow(s_rotaxisdlg);

	return 0;
}

int InitRotAxis()
{
	if (!s_rotaxisdlg){
		return 0;
	}
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (curbone){
		RecalcBoneAxisX(curbone);
	}

	return 0;
}

int RotAxis(HWND hDlgWnd)
{
	if (!s_rotaxisdlg){
		return 0;
	}
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (!curbone){
		return 0;
	}

	UINT checkflagX;
	checkflagX = IsDlgButtonChecked(hDlgWnd, IDC_RADIO1);
	if (checkflagX == BST_CHECKED){
		s_rotaxiskind = AXIS_X;
	}
	else{
		UINT checkflagY;
		checkflagY = IsDlgButtonChecked(hDlgWnd, IDC_RADIO2);
		if (checkflagY == BST_CHECKED){
			s_rotaxiskind = AXIS_Y;
		}
		else{
			UINT checkflagZ;
			checkflagZ = IsDlgButtonChecked(hDlgWnd, IDC_RADIO3);
			if (checkflagZ == BST_CHECKED){
				s_rotaxiskind = AXIS_Z;
			}
		}
	}

	WCHAR strdeg[256] = { 0L };
	GetWindowText(GetDlgItem(hDlgWnd, IDC_EDITDEG), strdeg, 256);
	unsigned int len = (unsigned int)wcslen(strdeg);
	//_ASSERT(0);
	if ((len > 0) && (len < 256)){
		s_rotaxisdeg = (float)_wtof(strdeg);
		//_ASSERT(0);
		if ((s_rotaxisdeg >= -360.0f) && (s_rotaxisdeg <= 360.0f)){
			float rotrad = s_rotaxisdeg * (float)DEG2PAI;
			ChaVector3 axis0;
			CQuaternion rotq;
			if (s_rotaxiskind == AXIS_X){
				axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
				rotq.SetAxisAndRot(axis0, rotrad);
			}
			else if (s_rotaxiskind == AXIS_Y){
				axis0 = ChaVector3(0.0f, 1.0f, 0.0f);
				rotq.SetAxisAndRot(axis0, rotrad);
			}
			else if (s_rotaxiskind == AXIS_Z){
				axis0 = ChaVector3(0.0f, 0.0f, 1.0f);
				rotq.SetAxisAndRot(axis0, rotrad);
			}
			else{
				_ASSERT(0);
				return 1;
			}

			ChaMatrix nodemat = curbone->GetNodeMat();
			CQuaternion noderot;
			noderot.RotationMatrix(nodemat);
			CQuaternion invnoderot;
			noderot.inv(&invnoderot);

			ChaMatrix newnodemat;
			newnodemat = nodemat * invnoderot.MakeRotMatX() * rotq.MakeRotMatX() * noderot.MakeRotMatX();

			ChaVector3 bonepos = curbone->GetJointFPos();
			newnodemat._41 = bonepos.x;
			newnodemat._42 = bonepos.y;
			newnodemat._43 = bonepos.z;

			curbone->SetNodeMat(newnodemat);

			MOTINFO* mi = s_model->GetCurMotInfo();
			if (mi){
				s_model->CalcBoneEul(mi->motid);
			}
			//WCHAR strmes[256];
			//swprintf_s(strmes, 256, L"rotaxis %d, rotdeg %.3f", s_rotaxiskind, s_rotaxisdeg);
			//::MessageBox(hDlgWnd, strmes, L"Check!!!", MB_OK);
		}
	}

	return 0;
}

LRESULT CALLBACK RotAxisDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		CheckRadioButton(hDlgWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
		s_rotaxiskind = AXIS_X;
		WCHAR strdeg[256];
		swprintf_s(strdeg, 256, L"0");
		SetWindowText(GetDlgItem(hDlgWnd, IDC_EDITDEG), strdeg);

		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		if (curbone){
			SetWindowText(GetDlgItem(hDlgWnd, IDC_BONENAME), curbone->GetWBoneName());
		}
		return FALSE;
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			//EndDialog(hDlgWnd, IDOK);
			if (s_rotaxisdlg){
				DestroyWindow(s_rotaxisdlg);
				s_rotaxisdlg = 0;
			}
			break;
		case IDCANCEL:
			//EndDialog(hDlgWnd, IDCANCEL);
			if (s_rotaxisdlg){
				DestroyWindow(s_rotaxisdlg);
				s_rotaxisdlg = 0;
			}
			break;
		case IDC_INITROT:
			InitRotAxis();
			break;
		case IDC_ROTATE:
			RotAxis(hDlgWnd);
			break;
		default:
			return FALSE;
		}
		break;
	case WM_CLOSE:
		if (s_rotaxisdlg){
			DestroyWindow(s_rotaxisdlg);
			s_rotaxisdlg = 0;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;

}


int ChangeCurrentBone()
{
	if (s_model){
		CDXUTComboBox* pComboBox;
		pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_BONE);
		CBone* pBone;
		pBone = s_model->GetBoneByID(s_curboneno);
		if (pBone){
			pComboBox->SetSelectedByData(ULongToPtr(s_curboneno));
		}

		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		if (curbone){
			CDXUTComboBox* pComboBox3 = g_SampleUI.GetComboBox(IDC_COMBO_BONEAXIS);
			//ANGLELIMIT anglelimit = curbone->GetAngleLimit();
			//pComboBox3->SetSelectedByData(ULongToPtr(anglelimit.boneaxiskind));
			//g_boneaxis = anglelimit.boneaxiskind;
			g_boneaxis = (int)PtrToUlong(pComboBox3->GetSelectedData());
		}

		SetRigidLeng();
		SetImpWndParams();
		SetDmpWndParams();
		RigidElem2WndParam();
		if (s_anglelimitdlg){
			Bone2AngleLimit();
			AngleLimit2Dlg(s_anglelimitdlg);
		}
	}
	return 0;
}

int OnFrameKeyboard()
{
	MoveMemory(g_savekeybuf, g_keybuf, sizeof(BYTE) * 256);


	GetKeyboardState((PBYTE)g_keybuf);
	if (g_keybuf[VK_SHIFT] & 0x80){
		s_dispselect = false;
	}
	else{
		s_dispselect = true;
	}
	if ((g_keybuf[VK_F9] & 0x80) && ((g_savekeybuf[VK_F9] & 0x80) == 0)){
		StartBt(0, 1);
	}
	if ((g_keybuf[VK_F10] & 0x80) && ((g_savekeybuf[VK_F10] & 0x80) == 0)){
		StartBt(1, 1);
	}
	if ((g_keybuf[' '] & 0x80) && ((g_savekeybuf[' '] & 0x80) == 0)){
		if (s_bpWorld && s_model){
			StartBt(2, 1);
		}
	}
	if (g_keybuf[VK_CONTROL] & 0x80){
		g_controlkey = true;
	}
	else{
		g_controlkey = false;
	}
	if (g_keybuf[VK_SHIFT] & 0x80){
		g_shiftkey = true;
	}
	else{
		g_shiftkey = false;
	}


	/*
	if (g_controlkey == false){
		if (g_keybuf[VK_ADD] & 0x80){
			g_btcalccnt += 1.0;
			Sleep(200);
		}
		else if (g_keybuf[VK_SUBTRACT] & 0x80){
			if (g_btcalccnt >= 2.0){
				g_btcalccnt -= 1.0;
				Sleep(200);
			}
		}
	}
	else{
		if (g_keybuf[VK_ADD] & 0x80){
			if (s_erp <= 0.9){
				s_erp += 0.1;
				Sleep(200);
			}
		}
		else if (g_keybuf[VK_SUBTRACT] & 0x80){
			if (s_erp >= 0.1){
				s_erp -= 0.1;
				Sleep(200);
			}
		}
	}
	*/

	if (g_ctrlshiftkeyformb == false){
		if ((g_keybuf[VK_CONTROL] & 0x80) && (g_keybuf[VK_SHIFT] & 0x80)){
			if (((g_savekeybuf[VK_CONTROL] & 0x80) == 0) || ((g_savekeybuf[VK_SHIFT] & 0x80) == 0)){
				g_ctrlshiftkeyformb = true;
				//reset g_ctrlshiftkeyformb at the place of calling OnTimelineMButtonDown
			}
		}
	}

	if (g_keybuf['A'] & 0x80){
		s_akeycnt++;
	}
	else{
		s_akeycnt = 0;
	}
	if (g_keybuf['D'] & 0x80){
		s_dkeycnt++;
	}
	else{
		s_dkeycnt = 0;
	}

	if (g_keybuf['1'] & 0x80) {//num
		s_1keycnt++;
	}
	else {
		s_1keycnt = 0;
	}



	/////// all model bone
	if (s_model && g_controlkey && (g_keybuf['A'] & 0x80) && !(g_savekeybuf['A'] & 0x80)){
		s_allmodelbone = !s_allmodelbone;
	}

	if (g_controlkey && (s_1keycnt == 1)) {
		s_dispsampleui = !s_dispsampleui;
	}


	return 0;
}

int OnFrameUtCheckBox()
{
	g_applyendflag = (int)s_ApplyEndCheckBox->GetChecked();
	g_slerpoffflag = (int)s_SlerpOffCheckBox->GetChecked();
	g_absikflag = (int)s_AbsIKCheckBox->GetChecked();
	g_bonemarkflag = (int)s_BoneMarkCheckBox->GetChecked();
	g_pseudolocalflag = (int)s_PseudoLocalCheckBox->GetChecked();
	g_limitdegflag = (int)s_LimitDegCheckBox->GetChecked();

	return 0;
}

int OnFramePreviewStop()
{

	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		}
	}

	return 0;
}

int OnFramePreviewNormal(double* pnextframe, double* pdifftime)
{
	if (g_previewFlag != 0){
		if (s_savepreviewFlag == 0){
			//preview start frame
			s_previewrange = s_editrange;
			double rangestart;
			if (s_previewrange.IsSameStartAndEnd()){
				rangestart = 1.0;
			}
			else{
				rangestart = s_previewrange.GetStartFrame();
			}
			s_model->SetMotionFrame(rangestart);
			*pdifftime = 0.0;
		}
	}

	int endflag = 0;
	int loopstartflag = 0;
	s_model->AdvanceTime(s_previewrange, g_previewFlag, *pdifftime, pnextframe, &endflag, &loopstartflag, -1);
	if (endflag == 1){
		g_previewFlag = 0;
	}

	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel && curmodel->GetCurMotInfo()){
			curmodel->SetMotionFrame(*pnextframe);
		}
	}
	s_owpLTimeline->setCurrentTime(*pnextframe, false);



	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		}
	}


	return 0;
}

int OnFramePreviewBt(double* pnextframe, double* pdifftime)
{
	//int endflag = 0;

	static double rangestart = 1.0;//!!!!!!!!


	if (!s_model){
		return 0;
	}

	if (g_previewFlag != 0) {
		if (s_savepreviewFlag == 0) {
			//preview start frame
			s_previewrange = s_editrange;
			if (s_previewrange.IsSameStartAndEnd()) {
				rangestart = 1.0;
			}
			else {
				rangestart = s_previewrange.GetStartFrame();
			}
			s_model->SetMotionFrame(rangestart);
			*pdifftime = 0.0;
		}
	}

	int endflag = 0;
	int loopstartflag = 0;
	s_model->AdvanceTime(s_previewrange, g_previewFlag, *pdifftime, pnextframe, &endflag, &loopstartflag, -1);
	s_owpLTimeline->setCurrentTime(*pnextframe, false);
	if (endflag == 1) {
		g_previewFlag = 0;
		_ASSERT(0);

	}

	//CModel* curmodel = s_model;
	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			curmodel->SetMotionFrame(*pnextframe);
			if ((IsTimeEqual(*pnextframe, rangestart)) || (loopstartflag == 1)){
				curmodel->ZeroBtCnt();
			}

			//if ((curmodel->GetBtCnt() == 0) || (curmodel->GetBtCnt() == 1) || (curmodel->GetBtCnt() == 2)){
			if (curmodel->GetBtCnt() == 0){
				StartBt(2, 0);
			}

			//UpdateBtSimu(*pnextframe, curmodel);

			int firstflag = 0;
			if (s_savepreviewFlag != g_previewFlag){
				firstflag = 1;
			}
			if (curmodel && curmodel->GetCurMotInfo()){
				curmodel->Motion2Bt(firstflag, *pnextframe, &curmodel->GetWorldMat(), &s_matVP, s_curboneno);
			}

			curmodel->PlusPlusBtCnt();
		}
	}

	s_bpWorld->clientMoveAndDisplay();

	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			if (curmodel && curmodel->GetCurMotInfo()){
				curmodel->SetBtMotion(0, *pnextframe, &curmodel->GetWorldMat(), &s_matVP);
			}
		}
	}

	return 0;
}

void UpdateBtSimu(double nextframe, CModel* curmodel)
{
	if (!curmodel){
		return;
	}
	int firstflag = 0;
	if (s_savepreviewFlag != g_previewFlag){
		firstflag = 1;
	}
	if (curmodel && curmodel->GetCurMotInfo()){
		curmodel->Motion2Bt(firstflag, nextframe, &curmodel->GetWorldMat(), &s_matVP, s_curboneno);
	}
	//s_bpWorld->setTimeStep(1.0f / 120.0f);// seconds
	s_bpWorld->clientMoveAndDisplay();
	if (curmodel && curmodel->GetCurMotInfo()){
		curmodel->SetBtMotion(0, nextframe, &curmodel->GetWorldMat(), &s_matVP);
	}
}

int OnFramePreviewRagdoll(double* pnextframe, double* pdifftime)
{
	static int s_underikflag = 0;
	static int s_befunderikflag = 0;

	int endflag = 0;

	if (!s_model){
		return 0;
	}
	CModel* curmodel = s_model;


	if (curmodel && curmodel->GetCurMotInfo()){
		//if (s_onragdollik != 0){
		//	if (s_underikflag == 0){
		//		curmodel->BulletSimulationStop();
		//		s_underikflag = 1;
		//	}
		//}
		curmodel->SetRagdollKinFlag(s_curboneno, s_physicskind);
	}

	if (s_onragdollik != 0){
		s_pickinfo.mousebefpos = s_pickinfo.mousepos;
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_mainwnd, &ptCursor);
		s_pickinfo.mousepos = ptCursor;

		ChaVector3 tmpsc;
		curmodel->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

		//RagdollIK時には時間０（とりあえず）
		//s_editrange.SetRangeOne(1.0);

		if (s_oprigflag == 0){//Rig操作ではないとき
			ChaVector3 targetpos(0.0f, 0.0f, 0.0f);
			CallF(CalcTargetPos(&targetpos), return 1);

			s_model->SetDofRotAxis(s_pickinfo.buttonflag);//!!!!!!!!!!!!!!!!!!!!!!!


			if (s_physicskind == 0){
				if (s_onragdollik == 1){
					int ikmaxlevel = 0;

					curmodel->PhysicsRot(&s_editrange, s_pickinfo.pickobjno, targetpos, ikmaxlevel);
				}
				else if ((s_onragdollik == 2) || (s_onragdollik == 3)){
					float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.5f;
					if (g_controlkey == true){
						deltax *= 0.250f;
					}
					s_editmotionflag = s_model->PhysicsRotAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, s_iklevel, s_ikcnt, s_ikselectmat);
					s_ikcnt++;
				}

				//if (s_curboneno <= 0){
				//	::MessageBoxA(NULL, "OnFramePreviewRagdoll : curboneno error", "check", MB_OK);
				//}

			}
			else{
				int ikmaxlevel = 0;
				ChaVector3 diffvec = targetpos - s_pickinfo.objworld;
				curmodel->PhysicsMV(&s_editrange, s_pickinfo.pickobjno, diffvec);
			}
		}


		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		CBone* parbone = curbone->GetParent();
		if (parbone){
			s_editmotionflag = parbone->GetBoneNo();
		}
		else{
			s_editmotionflag = s_curboneno;
		}
		s_ikcnt++;


	}
	//else{
	//	if (s_underikflag == 1){
	//		curmodel->BulletSimulationStop();
	//		s_underikflag = 0;
	//	}
	//}

	/*
	if (s_oprigflag == 0){
	ChaVector3 targetpos(0.0f, 0.0f, 0.0f);
	CallF(CalcTargetPos(&targetpos), return 1);
	if (s_ikkind == 0){
	s_editmotionflag = s_model->IKRotate(&s_editrange, s_pickinfo.pickobjno, targetpos, s_iklevel);
	}
	else if (s_ikkind == 1){
	ChaVector3 diffvec = targetpos - s_pickinfo.objworld;
	AddBoneTra2(diffvec);
	s_editmotionflag = s_curboneno;
	}
	}
	else{
	if (s_customrigbone){
	float deltau = (float)(s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) * 0.5f;
	float deltav = (float)(s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y) * 0.5f;
	if (g_controlkey == true){
	deltau *= 0.250f;
	deltav *= 0.250f;
	}

	s_ikcustomrig = s_customrigbone->GetCustomRig(s_customrigno);

	s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 0, deltau, s_ikcustomrig);
	s_model->UpdateMatrix(&s_matW, &s_matVP);
	s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 1, deltav, s_ikcustomrig);
	s_model->UpdateMatrix(&s_matW, &s_matVP);
	s_editmotionflag = s_curboneno;
	}
	}
	*/


	
	//RadgollIK時には時間０（とりあえず）
	//curmodel->SetMotionFrame(0.0);
	//*pnextframe = 0.0;//!!!!!!!!!!!!!!!!
	curmodel->SetMotionFrame(s_editrange.GetStartFrame());
	*pnextframe = s_editrange.GetStartFrame();//!!!!!!!!!!!!!!!


	s_bpWorld->clientMoveAndDisplay();

	if (curmodel && curmodel->GetCurMotInfo()){
		curmodel->SetBtMotion(1, *pnextframe, &curmodel->GetWorldMat(), &s_matVP);
		curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		curmodel->PlusPlusBtCnt();
	}
	

	s_befunderikflag = s_underikflag;

	return 0;
}

int OnFrameCloseFlag()
{
	// 終了フラグを確認

	if (s_closeFlag){
		s_closeFlag = false;
		s_dispmw = false;
		if (s_timelineWnd){
			s_timelineWnd->setVisible(0);
		}

	}
	if (s_LcloseFlag){
		s_LcloseFlag = false;
		s_Ldispmw = false;
		if (s_LtimelineWnd){
			s_LtimelineWnd->setVisible(0);
		}

	}
	if (s_closetoolFlag){
		s_closetoolFlag = false;
		s_disptool = false;
		if (s_toolWnd){
			s_toolWnd->setVisible(0);
		}
	}
	if (s_closeobjFlag){
		s_closeobjFlag = false;
		s_dispobj = false;
		if (s_layerWnd){
			s_layerWnd->setVisible(0);
		}
	}
	if (s_closemodelFlag){
		s_closemodelFlag = false;
		s_dispmodel = false;
		if (s_modelpanel.panel){
			s_modelpanel.panel->setVisible(0);
		}
	}
	if (s_closeconvboneFlag){
		s_closeconvboneFlag = false;
		s_dispconvbone = false;
		if (s_convboneWnd){
			s_convboneWnd->setVisible(false);
		}
	}
	if (s_DcloseFlag){
		s_DcloseFlag = false;
		s_dmpanimWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(s_erp);
		}
		if (s_model){
			CallF(s_model->CreateBtObject(0), return 1);
		}
	}
	if (s_RcloseFlag){
		s_RcloseFlag = false;
		s_rigidWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(s_erp);
		}
		if (s_model){
			CallF(s_model->CreateBtObject(0), return 1);
		}
	}
	if (s_IcloseFlag){
		s_IcloseFlag = false;
		s_impWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(s_erp);
		}
		if (s_model){
			CallF(s_model->CreateBtObject(0), return 1);
		}
	}
	if (s_GcloseFlag){
		s_GcloseFlag = false;
		s_gpWnd->setVisible(0);
	}

	return 0;
}

int GetCurrentBoneFromTimeline(int* dstboneno)
{
	if (s_model && s_owpTimeline){
		int curlineno = s_owpTimeline->getCurrentLine();// 選択行
		if (curlineno >= 0){
			*dstboneno = s_lineno2boneno[curlineno];
			SetLTimelineMark(s_curboneno);
			ChangeCurrentBone();
		}
		else{
			*dstboneno = -1;
		}
	}
	return 0;
}


int OnFrameTimeLineWnd()
{
	// カーソル移動フラグを確認 //////////////////////////////////////////////////

	if (s_cursorFlag){
		s_cursorFlag = false;

		GetCurrentBoneFromTimeline(&s_curboneno);

		// カーソル位置をコンソールに出力
		if (s_owpTimeline && s_model && s_model->GetCurMotInfo()){
			if (g_previewFlag == 0){
				double curframe = s_owpTimeline->getCurrentTime();// 選択時刻

				vector<MODELELEM>::iterator itrmodel;
				for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
					CModel* curmodel = itrmodel->modelptr;
					if (curmodel && curmodel->GetCurMotInfo()){
						curmodel->SetMotionFrame(curframe);
					}
				}
			}
		}
		//DbgOut( L"cursor : lineno %d, boneno %d, frame %f\r\n", curlineno, s_curboneno, s_curframe );
	}

	if (s_LcursorFlag) {
		static int s_cnt = 0;
		s_cnt++;

		s_LcursorFlag = false;

		if (s_underselectingframe == 0) {
			//これがないとモーション停止ボタンを押した後にselect表示されない。
			s_buttonselectstart = s_editrange.GetStartFrame();
			s_buttonselectend = s_editrange.GetEndFrame();
			OnTimeLineButtonSelectFromSelectStartEnd(0);
		}

		s_underselectingframe = 1;
		OnTimeLineCursor(0, 0.0);

		if (g_previewFlag != 0) {
			//これがないとモーション再生中にselectが表示されない。
			OnTimeLineButtonSelectFromSelectStartEnd(0);
		}
	}

	// キー移動フラグを確認 ///////////////////////////////////////////////////////////
	if (s_keyShiftFlag){
		s_keyShiftFlag = false;
	}

	return 0;
}

int OnFrameMouseButton()
{
	if (s_timelinembuttonFlag || g_ctrlshiftkeyformb){
		s_timelinembuttonFlag = false;
		OnTimeLineMButtonDown(g_ctrlshiftkeyformb);
		g_ctrlshiftkeyformb = false;
	}
	if (s_timelinewheelFlag || (s_underselectingframe && ((g_keybuf['A'] & 0x80) || (g_keybuf['D'] & 0x80)))){
		s_timelinewheelFlag = false;
		OnTimeLineWheel();
	}

	if (s_timelineRUpFlag){//s_timelineWnd
		s_timelineRUpFlag = false;

		GetCurrentBoneFromTimeline(&s_curboneno);
		if (s_curboneno > 0){
			BoneRClick(s_curboneno);
		}
	}

	return 0;
}

int OnFrameToolWnd()
{
	//操作対象ボーンはs_selbonedlg::GetCpVec()にて取得。

	if (s_selboneFlag){
		s_selboneFlag = false;
		if (s_model && s_owpTimeline && s_owpLTimeline){
			s_selbonedlg.SetModel(s_model);
			s_selbonedlg.DoModal();
		}
	}

	if (s_markFlag){
		s_markFlag = false;

		if (s_model && s_owpTimeline && s_owpLTimeline){
			double curltime = s_owpLTimeline->getCurrentTime();
			KeyInfo ki = s_owpLTimeline->ExistKey(2, curltime);
			if (ki.lineIndex < 0){
				s_owpLTimeline->newKey(s_strmark, curltime, 0);
			}
		}
	}

	if (s_initmpFlag){
		s_initmpFlag = false;

		InitMpFromTool();
	}

	if (s_interpolateFlag){
		s_interpolateFlag = false;

		if (s_model && s_owpTimeline && s_owpLTimeline && s_model->GetCurMotInfo()){
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno);

			int keynum;
			double startframe, endframe, applyframe;
			s_editrange.Clear();
			s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
			s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);

			s_model->InterpolateBetweenSelection(startframe, endframe);

			s_model->SaveUndoMotion(s_curboneno, s_curbaseno);
		}

	}

	if (s_copyFlag){
		s_copyFlag = false;
		s_undersymcopyFlag = false;

		if (s_model && s_owpTimeline && s_owpLTimeline && s_model->GetCurMotInfo()){
			s_copymotmap.clear();
			s_copyKeyInfoList.clear();
			s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();

			list<KeyInfo>::iterator itrcp;
			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
				double curframe = itrcp->time;
				InsertCopyMPReq(s_model->GetTopBone(), curframe);
			}

			s_model->SaveUndoMotion(s_curboneno, s_curbaseno);
		}
	}

	if (s_symcopyFlag){
		s_symcopyFlag = false;
		s_undersymcopyFlag = true;

		if (s_model && s_owpTimeline && s_owpLTimeline && s_model->GetCurMotInfo()){
			int symrootmode = GetSymRootMode();

			s_copymotmap.clear();
			s_copyKeyInfoList.clear();
			s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();

			list<KeyInfo>::iterator itrcp;
			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
				double curframe = itrcp->time;
				InsertSymMPReq(s_model->GetTopBone(), curframe, symrootmode);
			}
		}
	}


	if (s_pasteFlag){
		s_pasteFlag = false;

		if (s_model && s_owpTimeline && s_model->GetCurMotInfo() && !s_copymotmap.empty()){

			int cpnum = (int)s_selbonedlg.m_cpvec.size();
			int keynum = 0;
			double startframe, endframe, applyframe;

			double pastestartframe = 0.0;
			s_editrange.Clear();
			if (s_model && s_model->GetCurMotInfo()){
				if (s_owpTimeline && s_owpLTimeline){
					s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
					s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);
				}
			}

			//double curmaxframe = s_model->m_curmotinfo->frameleng;

			//コピーされたキーの先頭時刻を求める
			double copyStartTime = DBL_MAX;
			double copyEndTime = 0;
			vector<CPELEM>::iterator itrcp;
			for (itrcp = s_copymotmap.begin(); itrcp != s_copymotmap.end(); itrcp++){
				if (itrcp->mp.GetFrame() <= copyStartTime){
					copyStartTime = itrcp->mp.GetFrame();
				}
				if (itrcp->mp.GetFrame() >= copyEndTime){
					copyEndTime = itrcp->mp.GetFrame();
				}
			}

			if (keynum >= 0){
				if (keynum == 0){
					double motleng = s_model->GetCurMotInfo()->frameleng - 1;
					double srcendframe = min(motleng, startframe + (copyEndTime - copyStartTime));
					srcendframe = max(srcendframe, 0.0);
					PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, srcendframe);
				}
				else{
					if (keynum <= (int)(copyEndTime - copyStartTime + 1.0 + 0.1)){
						PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, endframe);
					}else{
						PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, endframe);

						//コピー元の最終フレームの姿勢をコピー先の残りのフレームにペースト
						PasteMotionPointAfterCopyEnd(copyStartTime, copyEndTime, startframe, endframe);
					}
				}
			}
		}
		s_model->SaveUndoMotion(s_curboneno, s_curbaseno);
	}

	if (s_motpropFlag){
		s_motpropFlag = false;

		if (s_model && s_model->GetCurMotInfo()){
			int dlgret;
			dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MOTPROPDLG),
				s_mainwnd, (DLGPROC)MotPropDlgProc);
			if ((dlgret == IDOK) && s_tmpmotname[0]){
				WideCharToMultiByte(CP_ACP, 0, s_tmpmotname, -1, s_model->GetCurMotInfo()->motname, 256, NULL, NULL);
				//s_model->m_curmotinfo->frameleng = s_tmpmotframeleng;
				s_model->GetCurMotInfo()->loopflag = s_tmpmotloop;
				double oldframeleng = s_model->GetCurMotInfo()->frameleng;

				s_owpTimeline->setMaxTime(s_tmpmotframeleng);
				s_model->ChangeMotFrameLeng(s_model->GetCurMotInfo()->motid, s_tmpmotframeleng);//はみ出たmpも削除
				InitCurMotion(0, oldframeleng);

				//メニュー書き換え, timeline update
				OnAnimMenu(s_curmotmenuindex);
			}
		}
	}

	/*
	if ((g_keybuf['E'] & 0x80) && ((g_savekeybuf['E'] & 0x80) == 0)){
	if (s_model && (s_curboneno >= 0)){
	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (curbone){
	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (curmi){
	const WCHAR* wbonename = curbone->GetWBoneName();
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	int paraxsiflag = 1;
	int isfirstbone = 0;
	cureul = curbone->CalcLocalEulZXY(paraxsiflag, curmi->motid, curmi->curframe, BEFEUL_ZERO, isfirstbone);
	curbone->SetWorldMatFromEul(1, cureul, curmi->motid, curmi->curframe);
	::MessageBox(NULL, L"SetWorldMatFromEul", L"Check", MB_OK);
	}
	}
	}
	}
	*/

	if (s_filterFlag == true){
		s_filterFlag = false;

		if (s_model){
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno);
		}

		s_editrange.Clear();
		if (s_model && s_model->GetCurMotInfo()){
			if (s_owpTimeline && s_owpLTimeline){
				s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
				int keynum;
				double startframe, endframe, applyframe;
				s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);

				if (keynum >= 2){
					CMotFilter motfilter;
					motfilter.Filter(s_model, s_model->GetCurMotInfo()->motid, (int)startframe, (int)endframe);

					if (s_model){
						s_model->SaveUndoMotion(s_curboneno, s_curbaseno);
					}
				}
				else{
					::MessageBox(s_mainwnd, L"フレーム範囲を指定してから再試行してください。", L"選択エラー", MB_OK);
				}
			}
		}


	}



	if (s_deleteFlag){
		s_deleteFlag = false;
		/***
		if( s_model && s_owpTimeline && s_model->m_curmotinfo){
		s_owpTimeline->deleteKey();
		//motionpointのdeleteはdelete Listenerでする。
		s_model->SaveUndoMotion( s_curboneno, s_curbaseno );
		}
		***/
	}


	// 削除されたキー情報のスタックを確認 ////////////////////////////////////////////
	for (; s_deletedKeyInfoList.begin() != s_deletedKeyInfoList.end();
		s_deletedKeyInfoList.pop_front()){
		/***
		CMotionPoint* mpptr = (CMotionPoint*)( s_deletedKeyInfoList.begin()->object );
		CBone* boneptr = s_model->m_bonelist[ s_lineno2boneno[ s_deletedKeyInfoList.begin()->lineIndex ] ];
		mpptr->LeaveFromChain( s_model->m_curmotinfo->motid, boneptr );
		//_ASSERT( 0 );
		delete mpptr;
		***/
	}

	return 0;
}

int PasteMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe)
{
	int cpnum = (int)s_selbonedlg.m_cpvec.size();

	int docopyflag = 0;
	int hasNotMvParFlag = 1;
	ChaMatrix notmvparmat;
	ChaMatrixIdentity(&notmvparmat);
	if (cpnum != 0){
		//selected bone at selbonedlg
		int cpno;
		for (cpno = 0; cpno < cpnum; cpno++){
			CBone* chkbone = s_selbonedlg.m_cpvec[cpno];
			if (chkbone == srcbone){
				docopyflag = 1;

				CBone* parbone = srcbone->GetParent();
				if (parbone){
					int cpno2;
					for (cpno2 = 0; cpno2 < cpnum; cpno2++){
						CBone* chkparbone = s_selbonedlg.m_cpvec[cpno2];
						if (chkparbone == parbone){
							hasNotMvParFlag = 0;
						}
					}
				}
				else{
					hasNotMvParFlag = 0;
				}

				break;
			}
			else{
				hasNotMvParFlag = 0;
			}
		}
	}
	else{
		docopyflag = 1;// all bone
		hasNotMvParFlag = 0;
	}

	if (srcbone && (docopyflag == 1)){
		_ASSERT(s_model->GetCurMotInfo());
		int curmotid = s_model->GetCurMotInfo()->motid;
		srcbone->PasteMotionPoint(curmotid, newframe, srcmp);
	}

	return 0;
}

int PasteNotMvParMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe)
{
	int cpnum = (int)s_selbonedlg.m_cpvec.size();


	int docopyflag = 0;
	int hasNotMvParFlag = 1;
	ChaMatrix notmvparmat;
	ChaMatrixIdentity(&notmvparmat);

	if (cpnum != 0){
		//selected bone at selbonedlg
		int cpno;
		for (cpno = 0; cpno < cpnum; cpno++){
			CBone* chkbone = s_selbonedlg.m_cpvec[cpno];
			if (chkbone == srcbone){
				docopyflag = 1;

				CBone* parbone = srcbone->GetParent();
				if (parbone){
					int cpno2;
					for (cpno2 = 0; cpno2 < cpnum; cpno2++){
						CBone* chkparbone = s_selbonedlg.m_cpvec[cpno2];
						if (chkparbone == parbone){
							hasNotMvParFlag = 0;
						}
					}
				}
				else{
					hasNotMvParFlag = 0;
				}

				break;
			}
			else{
				hasNotMvParFlag = 0;
			}
		}
	}
	else{
		docopyflag = 1;// all bone
		hasNotMvParFlag = 0;
	}


	if (srcbone && (docopyflag == 1)){
		CMotionPoint* newmp = 0;
		_ASSERT(s_model->GetCurMotInfo());
		int curmotid = s_model->GetCurMotInfo()->motid;
		newmp = srcbone->GetMotionPoint(curmotid, newframe);
		if (newmp){
			if (hasNotMvParFlag == 1){
				CBone* parbone = srcbone->GetParent();
				if (parbone){
					CMotionPoint* parmp = parbone->GetMotionPoint(curmotid, newframe);
					int setmatflag1 = 1;
					CQuaternion dummyq;
					ChaVector3 dummytra = ChaVector3(0.0f, 0.0f, 0.0f);

					parmp->SetBefWorldMat(parmp->GetWorldMat());
					srcbone->RotBoneQReq(parmp, curmotid, newframe, dummyq, 0, dummytra);
					_ASSERT(0);
				}
			}
		}
	}

	return 0;
}

int PasteMotionPointJustInTerm(double copyStartTime, double copyEndTime, double startframe, double endframe)
{
	int cpnum = (int)s_selbonedlg.m_cpvec.size();

	vector<CPELEM>::iterator itrcp;
	for (itrcp = s_copymotmap.begin(); itrcp != s_copymotmap.end(); itrcp++){
		CBone* srcbone = itrcp->bone;
		if (srcbone){
			CMotionPoint srcmp = itrcp->mp;
			double newframe = (double)((int)(srcmp.GetFrame() - copyStartTime + startframe + 0.1));//!!!!!!!!!!!!!!!!!!
			if ((newframe >= startframe) && (newframe <= endframe)){
				PasteMotionPoint(srcbone, srcmp, newframe);
			}
		}
	}

	//移動しないボーンのための処理
	for (itrcp = s_copymotmap.begin(); itrcp != s_copymotmap.end(); itrcp++){
		CBone* srcbone = itrcp->bone;
		if (srcbone){
			CMotionPoint srcmp = itrcp->mp;
			double newframe = (double)((int)(srcmp.GetFrame() - copyStartTime + startframe + 0.1));//!!!!!!!!!!!!!!!!!!
			if ((newframe >= startframe) && (newframe <= endframe)){
				PasteNotMvParMotionPoint(srcbone, srcmp, newframe);
			}
		}
	}

	return 0;
}

int PasteMotionPointAfterCopyEnd(double copyStartTime, double copyEndTime, double startframe, double endframe)
{
	vector<CPELEM>::iterator itrcp;

	double newframe;
	for (newframe = (double)((int)(copyEndTime - copyStartTime + startframe + 0.1)); newframe <= endframe; newframe += 1.0){
		for (itrcp = s_copymotmap.begin(); itrcp != s_copymotmap.end(); itrcp++){
			CBone* srcbone = itrcp->bone;
			if (srcbone){
				CMotionPoint srcmp = itrcp->mp;
				if (itrcp->mp.GetFrame() == copyEndTime){
					PasteMotionPoint(srcbone, srcmp, newframe);
				}
			}
		}
	}

	//移動しないボーンのための処理
	for (newframe = (double)((int)(copyEndTime - copyStartTime + startframe + 0.1)); newframe <= endframe; newframe += 1.0){
		for (itrcp = s_copymotmap.begin(); itrcp != s_copymotmap.end(); itrcp++){
			CBone* srcbone = itrcp->bone;
			if (srcbone){
				CMotionPoint srcmp = itrcp->mp;
				if (itrcp->mp.GetFrame() == copyEndTime){
					PasteNotMvParMotionPoint(srcbone, srcmp, newframe);
				}
			}
		}
	}

	return 0;
}

int OnFramePlayButton()
{
	if (s_firstkeyFlag){
		//先頭フレームへ
		s_firstkeyFlag = false;
		g_previewFlag = 0;
		if (s_owpTimeline){
			s_owpLTimeline->setCurrentTime(1.0, false);
		}
	}
	if (s_lastkeyFlag){
		//最終フレームへ
		s_lastkeyFlag = false;
		g_previewFlag = 0;
		if (s_model){
			if (s_owpTimeline){
				MOTINFO* curmi = s_model->GetCurMotInfo();
				if (curmi){
					double lastframe = max(0, s_model->GetCurMotInfo()->frameleng - 1.0);
					s_owpLTimeline->setCurrentTime(lastframe, false);
				}
			}
		}
	}
	return 0;
}


int OnFrameUndo()
{
	///////////// undo
	if (s_model && g_controlkey && (g_keybuf['Z'] & 0x80) && !(g_savekeybuf['Z'] & 0x80)){

		if (g_keybuf[VK_SHIFT] & 0x80){
			s_model->RollBackUndoMotion(1, &s_curboneno, &s_curbaseno);//!!!!!!!!!!!
		}
		else{
			s_model->RollBackUndoMotion(0, &s_curboneno, &s_curbaseno);//!!!!!!!!!!!

		}

		//s_copyKeyInfoList.clear();
		//s_deletedKeyInfoList.clear();
		//s_selectKeyInfoList.clear();

		if (s_model->GetCurMotInfo()->motid != s_curmotid){
			int chkcnt = 0;
			int findflag = 0;
			map<int, MOTINFO*>::iterator itrmi;
			for (itrmi = s_model->GetMotInfoBegin(); itrmi != s_model->GetMotInfoEnd(); itrmi++){
				MOTINFO* curmi = itrmi->second;
				_ASSERT(curmi);
				if (curmi == s_model->GetCurMotInfo()){
					findflag = 1;
					break;
				}
				chkcnt++;
			}

			if (findflag == 1){
				int selindex;
				selindex = chkcnt;
				OnAnimMenu(selindex, 0);
			}
		}
		else{
			//メニュー書き換え, timeline update
			OnAnimMenu(s_curmotmenuindex, 0);
		}

		int curlineno = s_boneno2lineno[s_curboneno];
		if (s_owpTimeline){
			s_owpTimeline->setCurrentLine(curlineno, true);
		}

		SetTimelineMark();
		SetLTimelineMark(s_curboneno);

		Sleep(500);
	}

	return 0;
}

int OnFrameUpdateGround()
{

	if (s_ground){
		s_ground->UpdateMatrix(&s_ground->GetWorldMat(), &s_matVP);
	}

	if (s_gplane && s_bpWorld && s_bpWorld->m_rigidbodyG){
		ChaMatrix gpmat = s_inimat;
		gpmat._42 = s_bpWorld->m_gplaneh;
		s_gplane->UpdateMatrix(&gpmat, &s_matVP);
	}
	return 0;
}

int OnFrameInitBtWorld()
{
	if (!s_model || !s_btWorld){
		return 0;
	}

	CModel* curmodel = s_model;


	//vector<MODELELEM>::iterator itrmodel;
	//for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		//CModel* curmodel = itrmodel->modelptr;
		//if (curmodel && (curmodel->GetLoadedFlag() == true) && (curmodel->GetCreateBtFlag() == false)){
			curmodel->SetBtWorld(s_btWorld);
		//	CallF(curmodel->CreateBtObject(s_coldisp, 1), return 1);
		//}

		//curmodel->PlusPlusBtCnt();

	//}
	return 0;
}


int CreateUtDialog()
{

	// Initialize dialogs
	//g_SettingsDlg.Init(&g_DialogResourceManager);
	g_SampleUI.Init(&g_DialogResourceManager);

	int iY;
	g_SampleUI.SetCallback(OnGUIEvent); 
	iY = 60;

	int ctrlh = 25;
	int addh = ctrlh + 2;

	int ctrlxlen = 120;
	int checkboxxlen = 120;

	WCHAR sz[100];


	//iY += 24;
	swprintf_s(sz, 100, L"Light scale: %0.2f", g_fLightScale);
	g_SampleUI.AddStatic(IDC_LIGHT_SCALE_STATIC, sz, 35, iY, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_LIGHT_SCALE, 50, iY += addh, 100, ctrlh, 0, 20, (int)(g_fLightScale * 10.0f));

	g_SampleUI.AddCheckBox(IDC_BMARK, L"ボーンを表示する", 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_BoneMarkCheckBox);

	/***
	swprintf_s( sz, 100, L"# Lights: %d", g_nNumActiveLights );
	g_SampleUI.AddStatic( IDC_NUM_LIGHTS_STATIC, sz, 35, iY += addh, ctrlxlen, ctrlh );
	g_SampleUI.AddSlider( IDC_NUM_LIGHTS, 50, iY += addh, 100, ctrlh, 1, MAX_LIGHTS, g_nNumActiveLights );

	//iY += 24;
	swprintf_s( sz, 100, L"Light scale: %0.2f", g_fLightScale );
	g_SampleUI.AddStatic( IDC_LIGHT_SCALE_STATIC, sz, 35, iY += addh, ctrlxlen, ctrlh );
	g_SampleUI.AddSlider( IDC_LIGHT_SCALE, 50, iY += addh, 100, ctrlh, 0, 20, ( int )( g_fLightScale * 10.0f ) );

	//iY += 24;
	g_SampleUI.AddButton( IDC_ACTIVE_LIGHT, L"Change active light (K)", 35, iY += addh, ctrlxlen, ctrlh, 'K' );

	g_SampleUI.AddCheckBox( IDC_LIGHT_DISP, L"ライト矢印を表示する", 25, iY += addh, 450, 16, true, 0U, false, &s_LightCheckBox );

	iY += addh;
	***/
	//iY += 24;
	g_SampleUI.AddComboBox(IDC_COMBO_BONEAXIS, 35, iY += addh, ctrlxlen, ctrlh);
	CDXUTComboBox* pComboBox3 = g_SampleUI.GetComboBox(IDC_COMBO_BONEAXIS);
	pComboBox3->RemoveAllItems();
	WCHAR straxis[256];
	ULONG boneaxisindex;
	swprintf_s(straxis, 256, L"CurrentBoneAxis");
	boneaxisindex = 0;
	pComboBox3->AddItem(straxis, ULongToPtr(boneaxisindex));
	swprintf_s(straxis, 256, L"ParentBoneAxis");
	boneaxisindex = 1;
	pComboBox3->AddItem(straxis, ULongToPtr(boneaxisindex));
	swprintf_s(straxis, 256, L"GlobalBoneAxis");
	boneaxisindex = 2;
	pComboBox3->AddItem(straxis, ULongToPtr(boneaxisindex));
	pComboBox3->SetSelectedByData(ULongToPtr(1L));


	g_SampleUI.AddComboBox(IDC_COMBO_BONE, 35, iY += addh, ctrlxlen, ctrlh);

	/***
	g_SampleUI.AddButton( IDC_FK_XP, L"Rot X+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_FK_XM, L"Rot X-", 100, iY, 60, ctrlh );
	g_SampleUI.AddButton( IDC_FK_YP, L"Rot Y+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_FK_YM, L"Rot Y-", 100, iY, 60, ctrlh );
	g_SampleUI.AddButton( IDC_FK_ZP, L"Rot Z+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_FK_ZM, L"Rot Z-",100, iY, 60, ctrlh );
	***/
	/***
	iY += addh;

	g_SampleUI.AddButton( IDC_T_XP, L"Tra X+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_T_XM, L"Tra X-", 100, iY, 60, ctrlh );
	g_SampleUI.AddButton( IDC_T_YP, L"Tra Y+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_T_YM, L"Tra Y-", 100, iY, 60, ctrlh );
	g_SampleUI.AddButton( IDC_T_ZP, L"Tra Z+", 35, iY += addh, 60, ctrlh );
	g_SampleUI.AddButton( IDC_T_ZM, L"Tra Z-",100, iY, 60, ctrlh );
	***/
	//iY += addh;


	g_SampleUI.AddCheckBox(IDC_CAMTARGET, L"選択部を注視点にする", 25, iY += addh, ctrlxlen, 16, false, 0U, false, &s_CamTargetCheckBox);


	//iY += addh;

	g_SampleUI.AddComboBox(IDC_COMBO_IKLEVEL, 35, iY += addh, ctrlxlen, ctrlh);
	CDXUTComboBox* pComboBox0 = g_SampleUI.GetComboBox(IDC_COMBO_IKLEVEL);
	pComboBox0->RemoveAllItems();
	int level;
	for (level = 0; level < 15; level++){
		ULONG levelval = (ULONG)level;
		WCHAR strlevel[256];
		swprintf_s(strlevel, 256, L"%02d", level);
		pComboBox0->AddItem(strlevel, ULongToPtr(levelval));
	}
	pComboBox0->SetSelectedByData(ULongToPtr(1));


	g_SampleUI.AddComboBox(IDC_COMBO_EDITMODE, 35, iY += addh, ctrlxlen, ctrlh);
	CDXUTComboBox* pComboBox1 = g_SampleUI.GetComboBox(IDC_COMBO_EDITMODE);
	pComboBox1->RemoveAllItems();
	pComboBox1->AddItem(L"IK回転", ULongToPtr(IDC_IK_ROT));
	pComboBox1->AddItem(L"IK移動", ULongToPtr(IDC_IK_MV));
	//pComboBox1->AddItem( L"ライト回転", ULongToPtr( IDC_IK_LIGHT ) );
	pComboBox1->AddItem(L"剛体設定", ULongToPtr(IDC_BT_RIGID));
	pComboBox1->AddItem(L"インパルス", ULongToPtr(IDC_BT_IMP));
	pComboBox1->AddItem(L"物理地面", ULongToPtr(IDC_BT_GP));
	pComboBox1->AddItem(L"減衰率アニメ", ULongToPtr(IDC_BT_DAMP));

	pComboBox1->SetSelectedByData(ULongToPtr(0));


	swprintf_s(sz, 100, L"姿勢適用位置 : %d ％", g_applyrate);
	g_SampleUI.AddStatic(IDC_STATIC_APPLYRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_SL_APPLYRATE, 50, iY += addh, 100, ctrlh, 0, 100, g_applyrate);
	CEditRange::SetApplyRate(g_applyrate);


	//swprintf_s( sz, 100, L"IK First Rate : %f", g_ikfirst );
	swprintf_s(sz, 100, L"IK 次数の係数 : %f", g_ikfirst);
	g_SampleUI.AddStatic(IDC_STATIC_IKFIRST, sz, 35, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_SL_IKFIRST, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikfirst * 25.0f));

	swprintf_s(sz, 100, L"IK 伝達係数 : %f", g_ikrate);
	g_SampleUI.AddStatic(IDC_STATIC_IKRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_SL_IKRATE, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikrate * 100.0f));

	g_SampleUI.AddCheckBox(IDC_APPLY_TO_THEEND, L"最終フレームまで適用する。", 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_ApplyEndCheckBox);
	g_SampleUI.AddCheckBox(IDC_SLERP_OFF, L"SlerpIKをオフにする", 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_SlerpOffCheckBox);
	g_SampleUI.AddCheckBox(IDC_ABS_IK, L"絶対IKをオンにする", 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_AbsIKCheckBox);
	g_SampleUI.AddCheckBox(IDC_PSEUDOLOCAL, L"PseudoLocal(疑似ローカル)", 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_PseudoLocalCheckBox);
	g_SampleUI.AddCheckBox(IDC_LIMITDEG, L"回転角度制限をする", 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_LimitDegCheckBox);


	//Left Bottom
	iY = s_mainheight - 210;
	int startx = s_mainwidth / 2 - 180;

	swprintf_s(sz, 100, L"スレッド数 : %d(%d) 個", g_numthread, gNumIslands);
	g_SampleUI.AddStatic(IDC_STATIC_NUMTHREAD, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_SL_NUMTHREAD, startx, iY += addh, 100, ctrlh, 1, 64, g_numthread);


	//Right Bottom
	iY = s_mainheight - 210;
	startx = s_mainwidth - 120;

	swprintf_s(sz, 100, L"BT CalcCnt: %0.2f", g_btcalccnt);
	g_SampleUI.AddStatic(IDC_STATIC_BTCALCCNT, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_BTCALCCNT, startx, iY += addh, 100, ctrlh, 1, 100, (int)(g_btcalccnt));

	swprintf_s(sz, 100, L"BT ERP: %0.5f", s_erp);
	g_SampleUI.AddStatic(IDC_STATIC_ERP, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_ERP, startx, iY += addh, 100, ctrlh, 0, 5000, (int)(s_erp * 5000.0 + 0.4));

	

	//Center Bottom
	iY = s_mainheight - 210;
	startx = s_mainwidth / 2 - 50;

	swprintf_s(sz, 100, L"Motion Speed: %0.2f", g_dspeed);
	g_SampleUI.AddStatic(IDC_SPEED_STATIC, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_SPEED, startx, iY += addh, 100, ctrlh, 0, 700, (int)(g_dspeed * 100.0f));

	iY += 10;
	g_SampleUI.AddButton(IDC_BTSTART, L"BT start", startx, iY += addh, 100, ctrlh);
	iY += 5;
	g_SampleUI.AddButton(IDC_STOP_BT, L"STOP BT", startx, iY += addh, 100, ctrlh);


	//CenterRight Bottom
	iY = s_mainheight - 210;
	startx = s_mainwidth / 2 - 50 + 130 ;

	swprintf_s(sz, 100, L"Physics Edit Rate : %.3f", g_physicsmvrate);
	g_SampleUI.AddStatic(IDC_STATIC_PHYSICS_MV_SLIDER, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddSlider(IDC_PHYSICS_MV_SLIDER, startx, iY += addh, 100, ctrlh, 0, 100, (int)(g_physicsmvrate * 100.0f));

	iY += 10;
	g_SampleUI.AddButton(IDC_PHYSICS_IK, L"Physics Rot start", startx, iY += addh, 100, ctrlh);
	iY += 5;
	g_SampleUI.AddButton(IDC_PHYSICS_MV_IK, L"Physics MV start", startx, iY += addh, 100, ctrlh);
	//iY += 5;
	//g_SampleUI.AddButton(IDC_APPLY_BT, L"Apply BT", startx, iY += addh, 100, ctrlh);


	return 0;

}

int CreateTimelineWnd()
{
	s_timelineWnd = new OrgWindow(
		0,
		L"TimeLine",				//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(50, 0),		//位置
		WindowSize(400, 630),	//サイズ
		//WindowSize(150,540),	//サイズ
		L"TimeLine",				//タイトル
		s_mainwnd,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		70, 50, 70);				//カラー


	// ウィンドウの閉じるボタンのイベントリスナーに
	// 終了フラグcloseFlagをオンにするラムダ関数を登録する
	s_timelineWnd->setCloseListener([]() { s_closeFlag = true; });


	// ウィンドウのキーボードイベントリスナーに
	// コピー/カット/ペーストフラグcopyFlag/cutFlag/pasteFlagをオンにするラムダ関数を登録する
	// コピー等のキーボードを使用する処理はキーボードイベントリスナーを使用しなくても
	// メインループ内でマイフレームキー状態を監視することで作成可能である。
	s_timelineWnd->setKeyboardEventListener([](const KeyboardEvent &e){
		if (e.ctrlKey && !e.repeat && e.onDown){
			switch (e.keyCode){
			case 'C':
				s_copyFlag = true;
				break;
			case 'B':
				s_symcopyFlag = true;
				break;
			case 'X':
				s_cutFlag = true;
				break;
			case 'V':
				s_pasteFlag = true;
				break;
			case 'P':
				g_previewFlag = 1;
				break;
			case 'S':
				g_previewFlag = 0;
				break;
			case 'D':
				s_deleteFlag = true;
				break;
			default:
				break;
			}
		}
	});

	return 0;
}

int CreateLongTimelineWnd()
{

	//////////
	///////// Long Timeline
	s_LtimelineWnd = new OrgWindow(
		0,
		L"EditRangeTimeLine",				//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos( 250, 825 ),		//位置
		WindowPos(200, 645),		//位置
		WindowSize(1050, 120),	//サイズ
		L"EditRangeTimeLine",				//タイトル
		s_mainwnd,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		70, 50, 70);				//カラー

	/////////
	s_owpPlayerButton = new OWP_PlayerButton;
	s_owpPlayerButton->setButtonSize(20);
	s_LtimelineWnd->addParts(*s_owpPlayerButton);//owp_timelineより前

	s_owpPlayerButton->setFrontPlayButtonListener([]() { s_LstartFlag = true; g_previewFlag = 1; });
	s_owpPlayerButton->setBackPlayButtonListener([](){  s_LstartFlag = true; g_previewFlag = -1; });
	s_owpPlayerButton->setFrontStepButtonListener([](){ s_LstartFlag = true; s_lastkeyFlag = true; });
	s_owpPlayerButton->setBackStepButtonListener([](){  s_LstartFlag = true; s_firstkeyFlag = true; });
	s_owpPlayerButton->setStopButtonListener([]() {  s_LstopFlag = true; g_previewFlag = 0; });
	s_owpPlayerButton->setResetButtonListener([](){ if (s_owpLTimeline){ s_LstopFlag = true; g_previewFlag = 0; s_owpLTimeline->setCurrentTime(1.0, false); } });
	s_owpPlayerButton->setSelectToLastButtonListener([](){  g_underselecttolast = true; g_selecttolastFlag = true; });
	s_owpPlayerButton->setBtResetButtonListener([](){  s_btresetFlag = true; });
	s_owpPlayerButton->setPrevRangeButtonListener([](){  g_undereditrange = true; s_prevrangeFlag = true; });
	s_owpPlayerButton->setNextRangeButtonListener([](){  g_undereditrange = true; s_nextrangeFlag = true; });


	//###################################
	//s_owpLTimelineの関連ラムダをコメントとしてコピペ
	//###################################
	//s_owpLTimeline->setCursorListener([]() { s_LcursorFlag = true; });
	//s_owpLTimeline->setSelectListener([]() { s_selectFlag = true; });
	//s_owpLTimeline->setMouseMDownListener([]() {
	//	s_timelinembuttonFlag = true;
	//	if (s_mbuttoncnt == 0) {
	//		s_mbuttoncnt = 1;
	//	}
	//	else {
	//		s_mbuttoncnt = 0;
	//	}
	//});
	//s_owpLTimeline->setMouseWheelListener([]() {
	//	s_timelinewheelFlag = true;
	//});

	//####################################
	//s_LtimelineWndのラムダ　s_owpLTimelineではない。
	//####################################
	s_LtimelineWnd->setSizeMin(OrgWinGUI::WindowSize(100, 100));
	s_LtimelineWnd->setCloseListener([](){ s_LcloseFlag = true; });
	s_LtimelineWnd->setLUpListener([](){
		if (g_previewFlag == 0){
			if (s_prevrangeFlag || s_nextrangeFlag){
				RollBackEditRange(s_prevrangeFlag, s_nextrangeFlag);
				s_buttonselectstart = s_editrange.GetStartFrame();
				s_buttonselectend = s_editrange.GetEndFrame();

				s_underselectingframe = 0;
				OnTimeLineButtonSelectFromSelectStartEnd(0);
			}
			else if (g_selecttolastFlag == false){

				if (!s_LstopFlag) {
					if (s_selectFlag) {
						s_selectFlag = false;
						s_selectKeyInfoList.clear();
						s_selectKeyInfoList = s_owpLTimeline->getSelectedKey();
						s_editrange.SetRange(s_selectKeyInfoList, s_owpLTimeline->getCurrentTime());
						s_buttonselectstart = s_editrange.GetStartFrame();
						s_buttonselectend = s_editrange.GetEndFrame();
						s_underselectingframe = 0;
						//_ASSERT(0);
					}
					else {
						s_buttonselectstart = s_owpLTimeline->getCurrentTime();
						s_buttonselectend = s_owpLTimeline->getCurrentTime();
						s_underselectingframe = 0;
						//_ASSERT(0);
					}
					OnTimeLineButtonSelectFromSelectStartEnd(0);
				}
				else {
					//停止ボタンが押されたとき
					//_ASSERT(0);
					s_buttonselectstart = s_editrange.GetStartFrame();
					s_buttonselectend = s_editrange.GetEndFrame();
					s_underselectingframe = 0;
					//_ASSERT(0);
					OnTimeLineButtonSelectFromSelectStartEnd(0);
					//_ASSERT(0);
				}

			}
			else{
				//ToTheLastFrame
				OnTimeLineButtonSelectFromSelectStartEnd(1);
			}
		}
		else {
			//再生ボタンが押されたとき
			//_ASSERT(0);
			s_buttonselectstart = s_editrange.GetStartFrame();
			s_buttonselectend = s_editrange.GetEndFrame();
			s_underselectingframe = 0;
			//_ASSERT(0);

			OnTimeLineButtonSelectFromSelectStartEnd(0);

		}

		s_LstartFlag = false;
		s_LstopFlag = false;
		g_selecttolastFlag = false;
		s_prevrangeFlag = false;
		s_nextrangeFlag = false;


	});

	return 0;
}

int CreateDmpAnimWnd()
{

	/////////
	s_dmpanimWnd = new OrgWindow(
		1,
		_T("dampAnimWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(0, 400),		//位置
		//WindowSize(450,880),		//サイズ
		WindowSize(500, 120),		//サイズ
		_T("減衰率アニメウィンドウ"),	//タイトル
		s_mainwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_dmpgroupcheck = new OWP_CheckBox(L"全部設定ボタンで同グループ剛体に設定する。", 0);
	s_dmpanimLlabel = new OWP_Label(L"１フレームあたりの位置ばね減衰率の減少率");
	s_dmpanimLSlider = new OWP_Slider(0.0, 1.0, 0.0);
	s_dmpanimAlabel = new OWP_Label(L"１フレームあたりの角度ばね減衰率の減少率");
	s_dmpanimASlider = new OWP_Slider(0.0, 1.0, 0.0);
	s_dmpanimB = new OWP_Button(L"全ての剛体に設定");

	int slw2 = 500;
	s_dmpanimLSlider->setSize(WindowSize(slw2, 40));
	s_dmpanimASlider->setSize(WindowSize(slw2, 40));

	s_dmpanimWnd->addParts(*s_dmpgroupcheck);
	s_dmpanimWnd->addParts(*s_dmpanimLlabel);
	s_dmpanimWnd->addParts(*s_dmpanimLSlider);
	s_dmpanimWnd->addParts(*s_dmpanimAlabel);
	s_dmpanimWnd->addParts(*s_dmpanimASlider);
	s_dmpanimWnd->addParts(*s_dmpanimB);

	s_dmpanimWnd->setCloseListener([](){ s_DcloseFlag = true; });

	s_dmpanimLSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRgdRe();
		if (curre){
			float val = (float)s_dmpanimLSlider->getValue();
			curre->SetDampanimL(val);
		}
		s_dmpanimWnd->callRewrite();						//再描画
	});
	s_dmpanimASlider->setCursorListener([](){
		CRigidElem* curre = GetCurRgdRe();
		if (curre){
			float val = (float)s_dmpanimASlider->getValue();
			curre->SetDampanimA(val);
		}
		s_dmpanimWnd->callRewrite();						//再描画
	});
	s_dmpanimB->setButtonListener([](){
		if (s_model && (s_rgdindex >= 0)){
			float valL = (float)s_dmpanimLSlider->getValue();
			float valA = (float)s_dmpanimASlider->getValue();
			int chkg = (int)s_dmpgroupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRgdRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllDampAnimData(gid, s_rgdindex, valL, valA);
		}
	});

	return 0;
}

int CreateRigidWnd()
{

	/////////
	s_rigidWnd = new OrgWindow(
		1,
		_T("RigidWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(100, 200),		//位置
		//WindowSize(450,880),		//サイズ
		//WindowSize(450,680),		//サイズ
		//WindowSize(450, 760),		//サイズ
		WindowSize(450, 780),		//サイズ
		_T("剛体設定ウィンドウ"),	//タイトル
		s_mainwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_groupcheck = new OWP_CheckBox(L"全部設定ボタンで同グループ剛体に設定する。", 0);
	s_shprateSlider = new OWP_Slider(0.6, 20.0, 0.0);
	s_boxzSlider = new OWP_Slider(0.6, 20.0, 0.0);
	s_massSlider = new OWP_Slider(g_initmass, 30.0, 0.0);
	s_massB = new OWP_Button(L"全剛体に質量設定");
	s_rigidskip = new OWP_CheckBox(L"有効/無効", 1);
	s_forbidrot = new OWP_CheckBox(L"回転禁止", 0);
	s_allrigidenableB = new OWP_Button(L"全ての剛体を有効にする");
	s_allrigiddisableB = new OWP_Button(L"全ての剛体以外を無効にする");
	s_btgSlider = new OWP_Slider(-1.0, 1.0, -1.0);
	s_btgscSlider = new OWP_Slider(10.0, 100.0, 0.0);
	s_btgB = new OWP_Button(L"全ての剛体にBT重力設定");
	s_btforce = new OWP_CheckBox(L"BTシミュ剛体にする", 0);

	s_shplabel = new OWP_Label(L"剛体の太さ");
	s_boxzlabel = new OWP_Label(L"直方体の奥行き");
	s_massSLlabel = new OWP_Label(L"剛体の質量");
	s_btglabel = new OWP_Label(L"BT剛体の重力");
	s_btgsclabel = new OWP_Label(L"BT剛体の重力のスケール");

	s_namelabel = new OWP_Label(L"ボーン名:????");
	s_lenglabel = new OWP_Label(L"ボーン長:*****[m]");


	s_kB = new OWP_Button(L"全ての剛体にばねパラメータ設定");
	s_restB = new OWP_Button(L"全ての剛体に弾性と摩擦設定");

	s_colradio = new OWP_RadioButton(L"コーン形状");
	s_colradio->addLine(L"カプセル形状");
	s_colradio->addLine(L"球形状");
	s_colradio->addLine(L"直方体形状");

	s_lkradio = new OWP_RadioButton(L"[位置ばね]へなへな");
	s_lkradio->addLine(L"[位置ばね]結構ゆるい");
	s_lkradio->addLine(L"[位置ばね]普通こんなもんだと思う");
	s_lkradio->addLine(L"[位置ばね]カスタム値");

	//s_lkSlider = new OWP_Slider(g_initcuslk, 1e6, 1e4);//60000
	//s_lkSlider = new OWP_Slider(g_initcuslk, 1e10, 1e8);//60000
	s_lkSlider = new OWP_Slider(g_initcuslk, 1e4, 1e2);//60000
	s_lklabel = new OWP_Label(L"位置ばね カスタム値");

	s_akradio = new OWP_RadioButton(L"[角度ばね]へなへな");
	s_akradio->addLine(L"[角度ばね]結構ゆるい");
	s_akradio->addLine(L"[角度ばね]普通こんなもんだと思う");
	s_akradio->addLine(L"[角度ばね]カスタム値");

	//s_akSlider = new OWP_Slider(g_initcusak, 6000.0f, 0.0f);//300
	//s_akSlider = new OWP_Slider(g_initcusak, 30.0f, 0.0f);//300
	//s_akSlider = new OWP_Slider(g_initcusak, 3000.0f, 30.0f);//300
	s_akSlider = new OWP_Slider(g_initcusak, 3000.0f, 10.0f);//300
	s_aklabel = new OWP_Label(L"角度ばね カスタム値");

	s_restSlider = new OWP_Slider(0.5f, 1.0f, 0.0f);
	s_restlabel = new OWP_Label(L"剛体の弾性");
	s_fricSlider = new OWP_Slider(0.5f, 1.0f, 0.0f);
	s_friclabel = new OWP_Label(L"剛体の摩擦");


	s_ldmplabel = new OWP_Label(L"[位置ばね]減衰率");
	s_admplabel = new OWP_Label(L"[角度ばね]減衰率");
	s_ldmpSlider = new OWP_Slider(g_l_dmp, 1.0, 0.0);
	s_admpSlider = new OWP_Slider(g_a_dmp, 1.0, 0.0);
	s_dmpB = new OWP_Button(L"全剛体に減衰率設定");
	s_groupB = new OWP_Button(L"剛体の衝突グループID設定");
	s_gcoliB = new OWP_Button(L"地面の衝突グループID設定");

	int slw = 350;

	s_shprateSlider->setSize(WindowSize(slw, 40));
	s_boxzSlider->setSize(WindowSize(slw, 40));
	s_massSlider->setSize(WindowSize(slw, 40));
	s_btgSlider->setSize(WindowSize(slw, 40));
	s_btgscSlider->setSize(WindowSize(slw, 40));
	s_ldmpSlider->setSize(WindowSize(slw, 40));
	s_admpSlider->setSize(WindowSize(slw, 40));
	s_lkSlider->setSize(WindowSize(slw, 40));
	s_akSlider->setSize(WindowSize(slw, 40));
	s_restSlider->setSize(WindowSize(slw, 40));
	s_fricSlider->setSize(WindowSize(slw, 40));

	s_rigidWnd->addParts(*s_namelabel);
	s_rigidWnd->addParts(*s_groupcheck);
	s_rigidWnd->addParts(*s_shplabel);
	s_rigidWnd->addParts(*s_shprateSlider);
	s_rigidWnd->addParts(*s_boxzlabel);
	s_rigidWnd->addParts(*s_boxzSlider);
	s_rigidWnd->addParts(*s_massSLlabel);
	s_rigidWnd->addParts(*s_massSlider);
	s_rigidWnd->addParts(*s_massB);
	s_rigidWnd->addParts(*s_lenglabel);
	s_rigidWnd->addParts(*s_rigidskip);
	s_rigidWnd->addParts(*s_forbidrot);
	s_rigidWnd->addParts(*s_allrigidenableB);
	s_rigidWnd->addParts(*s_allrigiddisableB);

	s_rigidWnd->addParts(*s_colradio);

	s_rigidWnd->addParts(*s_lkradio);
	s_rigidWnd->addParts(*s_lklabel);
	s_rigidWnd->addParts(*s_lkSlider);
	s_rigidWnd->addParts(*s_akradio);
	s_rigidWnd->addParts(*s_aklabel);
	s_rigidWnd->addParts(*s_akSlider);
	s_rigidWnd->addParts(*s_kB);

	s_rigidWnd->addParts(*s_restlabel);
	s_rigidWnd->addParts(*s_restSlider);
	s_rigidWnd->addParts(*s_friclabel);
	s_rigidWnd->addParts(*s_fricSlider);
	s_rigidWnd->addParts(*s_restB);


	s_rigidWnd->addParts(*s_ldmplabel);
	s_rigidWnd->addParts(*s_ldmpSlider);
	s_rigidWnd->addParts(*s_admplabel);
	s_rigidWnd->addParts(*s_admpSlider);
	s_rigidWnd->addParts(*s_dmpB);

	s_rigidWnd->addParts(*s_btglabel);
	s_rigidWnd->addParts(*s_btgSlider);
	s_rigidWnd->addParts(*s_btgB);
	s_rigidWnd->addParts(*s_btgsclabel);
	s_rigidWnd->addParts(*s_btgscSlider);
	s_rigidWnd->addParts(*s_btforce);

	s_rigidWnd->addParts(*s_groupB);
	s_rigidWnd->addParts(*s_gcoliB);


	s_rigidWnd->setCloseListener([](){ s_RcloseFlag = true; });

	s_shprateSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_shprateSlider->getValue();
			curre->SetSphrate(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_boxzSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_boxzSlider->getValue();
			curre->SetBoxzrate(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_massSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_massSlider->getValue();
			curre->SetMass(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_ldmpSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_ldmpSlider->getValue();
			curre->SetLDamping(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_admpSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_admpSlider->getValue();
			curre->SetADamping(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_lkSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_lkSlider->getValue();
			curre->SetCusLk(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_akSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_akSlider->getValue();
			curre->SetCusAk(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});


	s_restSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_restSlider->getValue();
			curre->SetRestitution(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_fricSlider->setCursorListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			float val = (float)s_fricSlider->getValue();
			curre->SetFriction(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});


	s_rigidskip->setButtonListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			bool validflag = s_rigidskip->getValue();
			if (validflag == false){
				curre->SetSkipflag(1);
			}
			else{
				curre->SetSkipflag(0);
			}
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_forbidrot->setButtonListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			bool validflag = s_forbidrot->getValue();
			if (validflag == false){
				curre->SetForbidRotFlag(0);
			}
			else{
				curre->SetForbidRotFlag(1);
			}
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_allrigidenableB->setButtonListener([](){
		if (s_model){
			s_model->EnableAllRigidElem(s_curreindex);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_allrigiddisableB->setButtonListener([](){
		if (s_model){
			s_model->DisableAllRigidElem(s_curreindex);
		}
		s_rigidWnd->callRewrite();						//再描画
	});


	s_btforce->setButtonListener([](){
		if (s_model && (s_curboneno >= 0)){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				CBone* parbone = curbone->GetParent();
				if (parbone){
					bool kinflag = s_btforce->getValue();
					if (kinflag == false){
						parbone->SetBtForce(0);
					}
					else{
						parbone->SetBtForce(1);
					}
				}
			}
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_btgSlider->setCursorListener([](){
		float btg = (float)s_btgSlider->getValue();
		CRigidElem* curre = GetCurRe();
		if (curre){
			curre->SetBtg(btg);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_btgscSlider->setCursorListener([](){
		float btgsc = (float)s_btgscSlider->getValue();
		if (s_model && (s_curreindex >= 0)){
			REINFO tmpinfo = s_model->GetRigidElemInfo(s_curreindex);
			tmpinfo.btgscale = btgsc;
			s_model->SetRigidElemInfo(s_curreindex, tmpinfo);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_colradio->setSelectListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			int val = s_colradio->getSelectIndex();
			curre->SetColtype(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_lkradio->setSelectListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			int val = s_lkradio->getSelectIndex();
			curre->SetLKindex(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});
	s_akradio->setSelectListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			int val = s_akradio->getSelectIndex();
			curre->SetAKindex(val);
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_kB->setButtonListener([](){
		if (s_model){
			int lindex = s_lkradio->getSelectIndex();
			int aindex = s_akradio->getSelectIndex();
			float cuslk = (float)s_lkSlider->getValue();
			float cusak = (float)s_akSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllKData(gid, s_curreindex, lindex, aindex, cuslk, cusak);
		}
	});
	s_restB->setButtonListener([](){
		if (s_model){
			float rest = (float)s_restSlider->getValue();
			float fric = (float)s_fricSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllRestData(gid, s_curreindex, rest, fric);
		}
	});
	s_dmpB->setButtonListener([](){
		if (s_model){
			float ldmp = (float)s_ldmpSlider->getValue();
			float admp = (float)s_admpSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllDmpData(gid, s_curreindex, ldmp, admp);
		}
	});

	s_groupB->setButtonListener([](){
		CRigidElem* curre = GetCurRe();
		if (curre){
			CColiIDDlg dlg(curre);
			dlg.DoModal();

			if (dlg.m_setgroup == 1){
				if (s_model){
					s_model->SetColiIDtoGroup(curre);
				}
			}
		}
	});
	s_gcoliB->setButtonListener([](){
		if (s_bpWorld){
			CGColiIDDlg dlg(s_bpWorld->m_coliids, s_bpWorld->m_myselfflag);
			int dlgret = (int)dlg.DoModal();
			if (dlgret == IDOK){
				s_bpWorld->m_coliids = dlg.m_coliids;
				s_bpWorld->m_myselfflag = dlg.m_myself;
				s_bpWorld->RemakeG();
			}
		}
	});

	s_massB->setButtonListener([](){
		if (s_model){
			float mass = (float)s_massSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllMassData(gid, s_curreindex, mass);
		}
		//		_ASSERT( 0 );
	});
	s_btgB->setButtonListener([](){
		if (s_model){
			float btg = (float)s_btgSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllBtgData(gid, s_curreindex, btg);
		}
	});


	s_rigidWnd->callRewrite();						//再描画

	return 0;
}

int CreateImpulseWnd()
{

	//////////
	s_impWnd = new OrgWindow(
		1,
		_T("ImpulseWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(400, 400),		//位置
		WindowSize(400, 200),		//サイズ
		//WindowSize(200,110),		//サイズ
		_T("剛体ウィンドウ"),	//タイトル
		s_mainwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_impgroupcheck = new OWP_CheckBox(L"全部に設定ボタンで同じ剛体グループに設定", 0);

	s_impxSlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impySlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impzSlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impscaleSlider = new OWP_Slider(1.0, 10.0, 0.0);
	s_impxlabel = new OWP_Label(L"インパルスのX");
	s_impylabel = new OWP_Label(L"インパルスのY");
	s_impzlabel = new OWP_Label(L"インパルスのZ");
	s_impscalelabel = new OWP_Label(L"インパルスのスケール");
	s_impallB = new OWP_Button(L"全ての剛体にインパルスをセット");

	int slw = 300;

	s_impzSlider->setSize(WindowSize(slw, 40));
	s_impySlider->setSize(WindowSize(slw, 40));

	s_impWnd->addParts(*s_impgroupcheck);
	s_impWnd->addParts(*s_impxlabel);
	s_impWnd->addParts(*s_impxSlider);
	s_impWnd->addParts(*s_impylabel);
	s_impWnd->addParts(*s_impySlider);
	s_impWnd->addParts(*s_impzlabel);
	s_impWnd->addParts(*s_impzSlider);
	s_impWnd->addParts(*s_impscalelabel);
	s_impWnd->addParts(*s_impscaleSlider);
	s_impWnd->addParts(*s_impallB);

	s_impWnd->setCloseListener([](){ s_IcloseFlag = true; });

	s_impzSlider->setCursorListener([](){
		float val = (float)s_impzSlider->getValue();
		if (s_model){
			s_model->SetImp(s_curboneno, 2, val);
		}
		s_impWnd->callRewrite();						//再描画
	});
	s_impySlider->setCursorListener([](){
		float val = (float)s_impySlider->getValue();
		if (s_model){
			s_model->SetImp(s_curboneno, 1, val);
		}
		s_impWnd->callRewrite();						//再描画
	});
	s_impxSlider->setCursorListener([](){
		float val = (float)s_impxSlider->getValue();
		if (s_model){
			s_model->SetImp(s_curboneno, 0, val);
		}
		s_impWnd->callRewrite();						//再描画
	});
	s_impscaleSlider->setCursorListener([](){
		float scale = (float)s_impscaleSlider->getValue();
		g_impscale = scale;
		s_impWnd->callRewrite();						//再描画
	});
	s_impallB->setButtonListener([](){
		if (s_model){
			float impx = (float)s_impxSlider->getValue();
			float impy = (float)s_impySlider->getValue();
			float impz = (float)s_impzSlider->getValue();
			int chkg = (int)s_impgroupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = GetCurRgdRe();
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllImpulseData(gid, impx, impy, impz);
		}
	});
	//////////

	return 0;
}

int CreateGPlaneWnd()
{

	//////////
	s_gpWnd = new OrgWindow(
		1,
		_T("GPlaneWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(400, 645),		//位置
		WindowSize(400, 320),		//サイズ
		//WindowSize(200,110),		//サイズ
		_T("物理地面ウィンドウ"),	//タイトル
		s_mainwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_ghSlider = new OWP_Slider(-1.5, 5.0, -15.0);
	s_gsizexSlider = new OWP_Slider(5.0, 50.0, -50.0);
	s_gsizezSlider = new OWP_Slider(5.0, 50.0, -50.0);
	s_ghlabel = new OWP_Label(L"物理地面の高さ");
	s_gsizexlabel = new OWP_Label(L"X方向のサイズ");
	s_gsizezlabel = new OWP_Label(L"Z方向のサイズ");
	s_gpdisp = new OWP_CheckBox(L"表示する", 1);

	s_grestSlider = new OWP_Slider(0.5, 1.0, 0.0);
	s_gfricSlider = new OWP_Slider(0.5, 1.0, 0.0);
	s_grestlabel = new OWP_Label(L"地面の弾性");
	s_gfriclabel = new OWP_Label(L"地面の摩擦");


	int slw = 300;

	s_ghSlider->setSize(WindowSize(slw, 40));
	s_gsizexSlider->setSize(WindowSize(slw, 40));
	s_gsizezSlider->setSize(WindowSize(slw, 40));
	s_grestSlider->setSize(WindowSize(slw, 40));
	s_gfricSlider->setSize(WindowSize(slw, 40));

	s_gpWnd->addParts(*s_ghlabel);
	s_gpWnd->addParts(*s_ghSlider);
	s_gpWnd->addParts(*s_gsizexlabel);
	s_gpWnd->addParts(*s_gsizexSlider);
	s_gpWnd->addParts(*s_gsizezlabel);
	s_gpWnd->addParts(*s_gsizezSlider);
	s_gpWnd->addParts(*s_gpdisp);

	s_gpWnd->addParts(*s_grestlabel);
	s_gpWnd->addParts(*s_grestSlider);
	s_gpWnd->addParts(*s_gfriclabel);
	s_gpWnd->addParts(*s_gfricSlider);

	s_gpWnd->setCloseListener([](){ s_GcloseFlag = true; });

	s_ghSlider->setCursorListener([](){
		if (s_bpWorld){
			s_bpWorld->m_gplaneh = (float)s_ghSlider->getValue();
			s_bpWorld->RemakeG();

			ChaVector3 tra(0.0f, 0.0f, 0.0f);
			ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
			CallF(s_gplane->MultDispObj(mult, tra), return);

			s_gpWnd->callRewrite();						//再描画
		}
	});
	s_gsizexSlider->setCursorListener([](){
		if (s_bpWorld && s_gplane){
			s_bpWorld->m_gplanesize.x = (float)s_gsizexSlider->getValue();

			ChaVector3 tra(0.0f, 0.0f, 0.0f);
			ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
			CallF(s_gplane->MultDispObj(mult, tra), return);
			s_gpWnd->callRewrite();						//再描画
		}
	});
	s_gsizezSlider->setCursorListener([](){
		if (s_bpWorld && s_gplane){
			s_bpWorld->m_gplanesize.y = (float)s_gsizezSlider->getValue();

			ChaVector3 tra(0.0f, 0.0f, 0.0f);
			ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
			CallF(s_gplane->MultDispObj(mult, tra), return);
			s_gpWnd->callRewrite();						//再描画
		}
	});
	s_gpdisp->setButtonListener([](){
		if (s_bpWorld){
			bool dispflag = s_gpdisp->getValue();
			s_bpWorld->m_gplanedisp = (int)dispflag;
			s_gpWnd->callRewrite();						//再描画
		}
	});
	s_grestSlider->setCursorListener([](){
		if (s_bpWorld && s_gplane){
			s_bpWorld->m_restitution = (float)s_grestSlider->getValue();
			s_bpWorld->RemakeG();

			s_gpWnd->callRewrite();						//再描画
		}
	});
	s_gfricSlider->setCursorListener([](){
		if (s_bpWorld && s_gplane){
			s_bpWorld->m_friction = (float)s_gfricSlider->getValue();
			s_bpWorld->RemakeG();

			s_gpWnd->callRewrite();						//再描画
		}
	});

	return 0;
}

int CreateToolWnd()
{

	/////////
	// ツールウィンドウを作成してボタン類を追加
	s_toolWnd = new OrgWindow(
		1,
		_T("ToolWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos(400, 580),		//位置
		WindowPos(50, 645),		//位置
		WindowSize(150, 10),		//サイズ
		_T("ツールウィンドウ"),	//タイトル
		s_timelineWnd->getHWnd(),	//親ウィンドウハンドル
		true,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		false);					//サイズ変更の可否

	s_toolSelBoneB = new OWP_Button(_T("コマンド対象ボーン"));
	s_toolCopyB = new OWP_Button(_T("コピー"));
	s_toolSymCopyB = new OWP_Button(_T("対称コピー"));
	//s_toolCutB = new OWP_Button(_T("カット"));
	s_toolPasteB = new OWP_Button(_T("ペースト"));
	s_toolInitMPB = new OWP_Button(_T("姿勢初期化"));
	//s_toolDeleteB = new OWP_Button(_T("削除"));
	//s_toolMarkB = new OWP_Button(_T("マーク作成"));
	s_toolMotPropB = new OWP_Button(_T("プロパティ"));
	s_toolFilterB = new OWP_Button(_T("平滑化"));
	s_toolInterpolateB = new OWP_Button(_T("補間"));

	s_toolWnd->addParts(*s_toolSelBoneB);
	s_toolWnd->addParts(*s_toolCopyB);
	s_toolWnd->addParts(*s_toolSymCopyB);
	s_toolWnd->addParts(*s_toolPasteB);
	s_toolWnd->addParts(*s_toolInitMPB);
	//s_toolWnd->addParts(*s_toolMarkB);
	s_toolWnd->addParts(*s_toolMotPropB);
	s_toolWnd->addParts(*s_toolFilterB);
	s_toolWnd->addParts(*s_toolInterpolateB);

	s_toolWnd->setCloseListener([](){ s_closetoolFlag = true; });
	s_toolCopyB->setButtonListener([](){ s_copyFlag = true; });
	s_toolSymCopyB->setButtonListener([](){ s_symcopyFlag = true; });
	s_toolPasteB->setButtonListener([](){ s_pasteFlag = true; });
	s_toolMotPropB->setButtonListener([](){ s_motpropFlag = true; });
	//s_toolMarkB->setButtonListener( [](){ s_markFlag = true; } );
	s_toolSelBoneB->setButtonListener([](){ s_selboneFlag = true; });
	s_toolInitMPB->setButtonListener([](){ s_initmpFlag = true; });
	s_toolFilterB->setButtonListener([](){ s_filterFlag = true; });
	s_toolInterpolateB->setButtonListener([](){ s_interpolateFlag = true; });

	return 0;

}

int CreateLayerWnd()
{
	////
	// ウィンドウを作成
	s_layerWnd = new OrgWindow(
		1,
		_T("LayerTool"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos(800, 500),		//位置
		WindowPos(250, 645),		//位置
		WindowSize(150, 200),		//サイズ
		_T("オブジェクトパネル"),	//タイトル
		NULL,					//親ウィンドウハンドル
		//true,					//表示・非表示状態
		false,					//表示・非表示状態
		70, 50, 70,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	// レイヤーウィンドウパーツを作成
	s_owpLayerTable = new OWP_LayerTable(_T("レイヤーテーブル"));
	WCHAR label[256];
	wcscpy_s(label, 256, L"dummy name");
	s_owpLayerTable->newLine(label, 0);

	// ウィンドウにウィンドウパーツを登録
	s_layerWnd->addParts(*s_owpLayerTable);


	s_layerWnd->setCloseListener([](){ s_closeobjFlag = true; });

	//レイヤーのカーソルリスナー
	s_owpLayerTable->setCursorListener([](){
		//_tprintf_s( _T("CurrentLayer: Index=%3d Name=%s\n"),
		//			owpLayerTable->getCurrentLine(),
		//			owpLayerTable->getCurrentLineName().c_str() );
	});

	//レイヤーの移動リスナー
	s_owpLayerTable->setLineShiftListener([](int from, int to){
		//_tprintf_s( _T("ShiftLayer: fromIndex=%3d toIndex=%3d\n"), from, to );
	});

	//レイヤーの可視状態変更リスナー
	s_owpLayerTable->setChangeVisibleListener([](int index){
		if (s_model){
			CMQOObject* curobj = (CMQOObject*)(s_owpLayerTable->getObj(index));
			if (curobj){
				if (s_owpLayerTable->getVisible(index)){
					curobj->SetDispFlag(1);
				}
				else{
					curobj->SetDispFlag(0);
				}
			}
		}
	});

	//レイヤーのロック状態変更リスナー
	s_owpLayerTable->setChangeLockListener([](int index){
		//if( owpLayerTable->getLock(index) ){
		//	_tprintf_s( _T("ChangeLock: Index=%3d Lock='True'  Name=%s\n"),
		//				index,
		//				owpLayerTable->getName(index).c_str() );
		//}else{
		//	_tprintf_s( _T("ChangeLock: Index=%3d Lock='False' Name=%s\n"),
		//				index,
		//				owpLayerTable->getName(index).c_str() );
		//}
	});

	//レイヤーのプロパティコールリスナー
	s_owpLayerTable->setCallPropertyListener([](int index){
		//_tprintf_s( _T("CallProperty: Index=%3d Name=%s\n"),
		//			index,
		//			owpLayerTable->getName(index).c_str() );
	});


	return 0;

}

int OnRenderModel()
{
	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;

		if (curmodel && curmodel->GetModelDisp()){
			int lightflag = 1;
			ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
			int btflag;
			if ((g_previewFlag != 4) && (g_previewFlag != 5)){
				btflag = 0;
			}
			else{
				btflag = 1;
			}
			curmodel->OnRender(s_pdev, lightflag, diffusemult, btflag);
		}
	}

	return 0;
}

int OnRenderGround()
{
	if (s_ground && s_dispground){
		g_hmWorld->SetMatrix((float*)&s_matWorld);
		//g_pEffect->SetMatrix(g_hmWorld, &(s_matWorld.D3DX()));

		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		s_ground->OnRender(s_pdev, 0, diffusemult);
	}
	if (s_gplane && s_bpWorld && s_bpWorld->m_gplanedisp){
		ChaMatrix gpmat = s_inimat;
		gpmat._42 = s_bpWorld->m_gplaneh;
		g_hmWorld->SetMatrix((float*)&gpmat);
		//g_pEffect->SetMatrix(g_hmWorld, &(gpmat.D3DX()));

		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		s_gplane->OnRender(s_pdev, 0, diffusemult);
	}

	return 0;
}

int OnRenderBoneMark()
{
	if (s_allmodelbone == 0){
		if ((g_previewFlag != 1) && (g_previewFlag != -1) && (g_previewFlag != 4)){
			if (s_model && s_model->GetModelDisp()){
				//if (s_ikkind >= 3){
					s_model->RenderBoneMark(s_pdev, s_bmark, s_bcircle, s_curboneno);
				//}
				//else{
				//	s_model->RenderBoneMark(s_pdev, s_bmark, s_bcircle, 0, s_curboneno);
				//}
			}
		}
	}
	else{
		vector<MODELELEM>::iterator itrme;
		for (itrme = s_modelindex.begin(); itrme != s_modelindex.end(); itrme++){
			MODELELEM curme = *itrme;
			CModel* curmodel = curme.modelptr;
			curmodel->RenderBoneMark(s_pdev, s_bmark, s_bcircle, 0, s_curboneno);
		}
	}

	return 0;
}
int OnRenderSelect()
{
	if ((g_previewFlag != 4) && (g_previewFlag != 5)){
		if (s_select && (s_curboneno >= 0) && (g_previewFlag == 0) && (s_model && s_model->GetModelDisp())){
			//SetSelectCol();
			SetSelectState();
			RenderSelectMark(1);
		}
	}
	//else if ((g_previewFlag == 5) && (s_oprigflag == 1)){
	else if (g_previewFlag == 5){
		if (s_select && (s_curboneno >= 0) && (s_model && s_model->GetModelDisp())){
			//SetSelectCol();
			SetSelectState();
			RenderSelectMark(1);
		}
	}

	return 0;
}

int OnRenderSprite()
{
	int spacnt;
	for (spacnt = 0; spacnt < 3; spacnt++){
		s_spaxis[spacnt].sprite->OnRender();
	}
	int spccnt;
	for (spccnt = 0; spccnt < 3; spccnt++){
		s_spcam[spccnt].sprite->OnRender();
	}

	s_sprig[s_oprigflag].sprite->OnRender();

	s_spbt.sprite->OnRender();


	return 0;
}

int OnRenderSetShaderConst()
{
	HRESULT hr;

	ChaVector3 vLightDir[MAX_LIGHTS];
	D3DXCOLOR vLightDiffuse[MAX_LIGHTS];

	// Get the projection & view matrix from the camera class
	g_hmVP->SetMatrix((float*)&s_matVP);
	//g_pEffect->SetMatrix(g_hmVP, &(s_matVP.D3DX()));
	////g_pEffect->SetMatrix(g_hmWorld, &s_matW);//CModelへ


	ChaVector3 lightdir0, nlightdir0;
	lightdir0 = g_camEye;
	ChaVector3Normalize(&nlightdir0, &lightdir0);
	g_LightControl[0].SetLightDirection(nlightdir0);

	// Render the light arrow so the user can visually see the light dir
	for (int i = 0; i < g_nNumActiveLights; i++)
	{
		//if( s_displightarrow ){
		//	D3DXCOLOR arrowColor = ( i == g_nActiveLight ) ? D3DXCOLOR( 1, 1, 0, 1 ) : D3DXCOLOR( 1, 1, 1, 1 );
		//	V( g_LightControl[i].OnRender10( arrowColor, &mView, &mProj, &g_camEye ) );
		//}
		vLightDir[i] = g_LightControl[i].GetLightDirection();
		vLightDiffuse[i] = g_fLightScale * D3DXCOLOR(1, 1, 1, 1);
	}
	ChaVector3 lightamb(1.0f, 1.0f, 1.0f);

	g_hLightDir->SetRawValue(vLightDir, 0, sizeof(ChaVector3) * MAX_LIGHTS);
	//V(g_pEffect->SetValue(g_hLightDir, vLightDir, sizeof(ChaVector3) * MAX_LIGHTS));
	g_hLightDiffuse->SetRawValue(vLightDiffuse, 0, sizeof(ChaVector4) * MAX_LIGHTS);
	//V(g_pEffect->SetValue(g_hLightDiffuse, vLightDiffuse, sizeof(ChaVector4) * MAX_LIGHTS));
	ChaVector3 eyept = g_camEye;
	g_hEyePos->SetRawValue(&eyept, 0, sizeof(ChaVector3));
	//V(g_pEffect->SetValue(g_hEyePos, &eyept, sizeof(ChaVector3)));

	return 0;
}

int OnRenderUtDialog(float fElapsedTime)
{
	if (g_previewFlag != 3){
		//g_HUD.OnRender( fElapsedTime );
		g_SampleUI.OnRender(fElapsedTime);
	}

	return 0;
}


int InitMpFromTool()
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model){
		return 0;
	}
	if (!s_owpTimeline || !s_owpLTimeline){
		return 0;
	}
	MOTINFO* mi = s_model->GetCurMotInfo();
	if (!mi){
		return 0;
	}


	HWND parwnd;
	parwnd = s_mainwnd;

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd);
	if (ret){
		return 1;
	}

	HMENU submenu = rmenu->GetSubMenu();
	POINT pt;
	GetCursorPos(&pt);


	CRMenuMain* rsubmenu[3];
	ZeroMemory(rsubmenu, sizeof(CRMenuMain*) * 3);

	int menunum;
	menunum = GetMenuItemCount(submenu);
	int menuno;
	for (menuno = 0; menuno < menunum; menuno++)
	{
		RemoveMenu(submenu, 0, MF_BYPOSITION);
	}
	s_customrigmenuindex.clear();

	int subnum = 3;
	int subsubnum = 3;
	int setmenuid;
	
	WCHAR strinitmpsub[3][20] = { L"全ボーン", L"選択ボーン１つ", L"選択ボーンと子供ボーン" };
	WCHAR strinitmpsubsub[3][20] = { L"回転と移動を初期化", L"回転を初期化", L"移動を初期化" };

	int subno;
	for (subno = 0; subno < 3; subno++){
		setmenuid = ID_RMENU_0 + subno * 3;

		rsubmenu[subno] = new CRMenuMain(IDR_RMENU);
		if (!rsubmenu[subno]){
			return 1;
		}
		ret = rsubmenu[subno]->CreatePopupMenu(parwnd, submenu, strinitmpsub[subno]);
		if (ret){
			return 1;
		}
		HMENU subsubmenu = rsubmenu[subno]->GetSubMenu();
		int subsubmenunum;
		subsubmenunum = GetMenuItemCount(subsubmenu);
		int subsubmenuno;
		for (subsubmenuno = 0; subsubmenuno < subsubmenunum; subsubmenuno++)
		{
			RemoveMenu(subsubmenu, 0, MF_BYPOSITION);
		}

		int subsubno;
		for (subsubno = 0; subsubno < 3; subsubno++){
			int subsubid = setmenuid + subsubno;
			AppendMenu(subsubmenu, MF_STRING, subsubid, strinitmpsubsub[subsubno]);

		}
	}

/////////////
	int initmode = -1;
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + 3 * 3))){
		int subid = (menuid - ID_RMENU_0) / 3;
		int initmode = (menuid - ID_RMENU_0) - subid * 3;

		s_copymotmap.clear();
		s_copyKeyInfoList.clear();
		s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();
		s_editrange.SetRange(s_copyKeyInfoList, s_owpTimeline->getCurrentTime());

		if (subid == 0){
			list<KeyInfo>::iterator itrcp;
			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
				double curframe = itrcp->time;
				CBone* topbone = s_model->GetTopBone();
				if (topbone){
					InitMpByEulReq(initmode, topbone, mi->motid, curframe);//topbone req
				}
			}
		}
		else if (subid == 1){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				list<KeyInfo>::iterator itrcp;
				for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
					double curframe = itrcp->time;
					InitMpByEul(initmode, curbone, mi->motid, curframe);//curbone
				}
			}
		}
		else if (subid == 2){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				list<KeyInfo>::iterator itrcp;
				for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
					double curframe = itrcp->time;
					InitMpByEulReq(initmode, curbone, mi->motid, curframe);//curbone req
				}
			}
		}
	}

	for (subno = 0; subno < 3; subno++){
		CRMenuMain* delsubmenu = rsubmenu[subno];
		if (delsubmenu){
			delete delsubmenu;
		}
	}

	rmenu->Destroy();
	delete rmenu;

	return 0;
}

int InitMpByEul(int initmode, CBone* curbone, int srcmotid, double srcframe)
{
	if (curbone){
		if (curbone->GetChild()){
			if (initmode == INITMP_ROTTRA){
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				int inittraflag1 = 1;
				int setchildflag1 = 1;
				curbone->SetWorldMatFromEul(inittraflag1, setchildflag1, cureul, srcmotid, srcframe);
			}
			else if (initmode == INITMP_ROT){
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				int inittraflag0 = 0;
				int setchildflag1 = 1;
				curbone->SetWorldMatFromEul(inittraflag0, setchildflag1, cureul, srcmotid, srcframe);
			}
			else if (initmode == INITMP_TRA){
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				int paraxsiflag1 = 1;
				int isfirstbone = 0;
				cureul = curbone->CalcLocalEulZXY(paraxsiflag1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);

				int inittraflag1 = 1;
				int setchildflag1 = 1;
				curbone->SetWorldMatFromEul(inittraflag1, setchildflag1, cureul, srcmotid, srcframe);
			}
		}
	}
	return 0;
}

void InitMpByEulReq(int initmode, CBone* curbone, int srcmotid, double srcframe)
{
	if (!curbone){
		return;
	}

	InitMpByEul(initmode, curbone, srcmotid, srcframe);

	if (curbone->GetChild()){
		InitMpByEulReq(initmode, curbone->GetChild(), srcmotid, srcframe);
	}
	if (curbone->GetBrother()){
		InitMpByEulReq(initmode, curbone->GetBrother(), srcmotid, srcframe);
	}
}

/// CustomRigDlg
int DispCustomRigDlg(int rigno)
{
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}
	if (s_model->GetOldAxisFlagAtLoading() == 1){
		::MessageBox(s_mainwnd, L"座標軸が設定してあるデータでのみ機能します。\nFBXファイルを保存しなおしてから再試行してください。", L"データタイプエラー", MB_OK);
		return 0;
	}

	Bone2CustomRig(rigno);

	if (!s_customrigdlg){
		s_customrigdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CUSTOMRIGDLG), s_mainwnd, (DLGPROC)CustomRigDlgProc);
		if (!s_customrigdlg){
			_ASSERT(0);
			return 1;
		}
	}
	else{
		CustomRig2Dlg(s_customrigdlg);
	}
	
	ShowWindow(s_customrigdlg, SW_SHOW);
	UpdateWindow(s_customrigdlg);
	
	s_oprigflag = 1;

	return 0;
}

int Bone2CustomRig(int rigno)
{
	if (!s_model){
		return 0;
	}
	if (s_curboneno < 0){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	s_customrigbone = s_model->GetBoneByID(s_curboneno);
	if (s_customrigbone){
		if ((rigno >= 0) && (rigno < MAXRIGNUM)){
			s_customrig = s_customrigbone->GetCustomRig(rigno);
		}
		else{
			s_customrig = s_customrigbone->GetFreeCustomRig();
		}
		if (s_customrig.rigboneno <= 0){
			_ASSERT(0);
		}
		s_customrigno = s_customrig.rigno;
	}
	else{
		_ASSERT(0);
		InitCustomRig(&s_customrig, 0, 0);
	}

	return 0;
}

int CustomRig2Bone()
{
	if (!s_model){
		return 0;
	}
	if (!s_customrigbone){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	if (s_customrigbone){
		int isvalid = IsValidCustomRig(s_model, s_customrig, s_customrigbone);
		if (isvalid == 0){
			::MessageBox(s_mainwnd, L"パラメータが不正です。", L"入力エラー", MB_OK);
			return 0;
		}
		s_customrigbone->SetCustomRig(s_customrig);
	}
	else{
		_ASSERT(0);
	}

	return 0;
}

int GetCustomRigRateVal(HWND hDlgWnd, int resid, float* dstptr)
{
	WCHAR strval[256] = { 0L };
	GetDlgItemText(hDlgWnd, resid, strval, 256);
	float tmpval;
	tmpval = (float)_wtof(strval);
	if ((tmpval < -100.0f) || (tmpval > 100.0f)){
		::MessageBox(hDlgWnd, L"倍率は-100.0から100.0までです。", L"倍率範囲エラー", MB_OK);
		*dstptr = 0.0f;
		return 1;
	}

	*dstptr = tmpval;
	return 0;
}

int CustomRig2Dlg(HWND hDlgWnd)
{
	if (s_customrigbone){
		SetDlgItemText(hDlgWnd, IDC_RIGNAME, (LPCWSTR)s_customrig.rigname);
		SetDlgItemText(hDlgWnd, IDC_RIGBONENAME, (LPCWSTR)s_customrigbone->GetWBoneName());

		int elemnum = s_customrig.elemnum;
		SendMessage(GetDlgItem(hDlgWnd, IDC_CHILNUM), CB_RESETCONTENT, 0, 0);

		WCHAR strcombo[256];
		WCHAR strval[256];

		int elemno;
		for (elemno = 0; elemno < MAXRIGELEMNUM; elemno++){
			swprintf_s(strcombo, 256, L"%d", elemno + 1);//from 1 to MAXRIGELEMNUM
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHILNUM), CB_ADDSTRING, 0, (LPARAM)strcombo);
		}
		if (elemnum < 1){
			_ASSERT(0);
			elemnum = 1;
			s_customrig.rigelem[0].boneno = s_customrigbone->GetBoneNo();
		}
		SendMessage(GetDlgItem(hDlgWnd, IDC_CHILNUM), CB_SETCURSEL, elemnum - 1, 0);


		int gpboxid[5] = {IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5};
		int axisuid[5] = {IDC_AXIS_U1, IDC_AXIS_U2, IDC_AXIS_U3, IDC_AXIS_U4, IDC_AXIS_U5};
		int axisvid[5] = {IDC_AXIS_V1, IDC_AXIS_V2, IDC_AXIS_V3, IDC_AXIS_V4, IDC_AXIS_V5};
		int rateuid[5] = {IDC_RATE_U1, IDC_RATE_U2, IDC_RATE_U3, IDC_RATE_U4, IDC_RATE_U5};
		int ratevid[5] = {IDC_RATE_V1, IDC_RATE_V2, IDC_RATE_V3, IDC_RATE_V4, IDC_RATE_V5};
		int enableuid[5] = { IDC_ENABLEU1, IDC_ENABLEU2, IDC_ENABLEU3, IDC_ENABLEU4, IDC_ENABLEU5};
		int enablevid[5] = { IDC_ENABLEV1, IDC_ENABLEV2, IDC_ENABLEV3, IDC_ENABLEV4, IDC_ENABLEV5 };
		int rigrigcomboid[5] = { IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO4, IDC_COMBO5 };


		for (elemno = 0; elemno < MAXRIGELEMNUM; elemno++){
			RIGELEM currigelem = s_customrig.rigelem[elemno];
			CBone* curbone = 0;
			if (elemno < elemnum){
				curbone = s_model->GetBoneByID(currigelem.boneno);
				if (curbone){
					SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)curbone->GetWBoneName());
				}
				else{
					_ASSERT(0);
					SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"未設定");
					//return 1;
				}
			}
			else{
				SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"未設定");
			}

			SetRigRigCombo(hDlgWnd, elemno);

			
			SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_RESETCONTENT, 0, 0);
			wcscpy_s(strcombo, 256, L"X");
			SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			wcscpy_s(strcombo, 256, L"Y");
			SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			wcscpy_s(strcombo, 256, L"Z");
			SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			if ((currigelem.transuv[0].axiskind >= AXIS_X) && (currigelem.transuv[0].axiskind <= AXIS_Z)){
				SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_SETCURSEL, currigelem.transuv[0].axiskind, 0);
			}
			swprintf_s(strval, 256, L"%f", currigelem.transuv[0].applyrate);
			SetDlgItemText(hDlgWnd, rateuid[elemno], (LPCWSTR)strval);


			SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_RESETCONTENT, 0, 0);
			wcscpy_s(strcombo, 256, L"X");
			SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			wcscpy_s(strcombo, 256, L"Y");
			SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			wcscpy_s(strcombo, 256, L"Z");
			SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
			if ((currigelem.transuv[0].axiskind >= AXIS_X) && (currigelem.transuv[0].axiskind <= AXIS_Z)){
				SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_SETCURSEL, currigelem.transuv[1].axiskind, 0);
			}
			swprintf_s(strval, 256, L"%f", currigelem.transuv[1].applyrate);
			SetDlgItemText(hDlgWnd, ratevid[elemno], (LPCWSTR)strval);

			//int enableuid[5] = { IDC_ENABLEU1, IDC_ENABLEU2, IDC_ENABLEU3, IDC_ENABLEU4, IDC_ENABLEU5 };
			//int enablevid[5] = { IDC_ENABLEV1, IDC_ENABLEV2, IDC_ENABLEV3, IDC_ENABLEV4, IDC_ENABLEV5 };
			if (currigelem.transuv[0].enable == 1){
				CheckDlgButton(hDlgWnd, enableuid[elemno], BST_CHECKED);
			}
			else{
				CheckDlgButton(hDlgWnd, enableuid[elemno], BST_UNCHECKED);
			}

			if (currigelem.transuv[1].enable == 1){
				CheckDlgButton(hDlgWnd, enablevid[elemno], BST_CHECKED);
			}
			else{
				CheckDlgButton(hDlgWnd, enablevid[elemno], BST_UNCHECKED);
			}

		}
		EnableRigAxisUV(hDlgWnd);
	}
	else{
		_ASSERT(0);
	}

	return 0;
}

int CheckRigRigCombo(HWND hDlgWnd, int elemno)
{
	//_ASSERT(0);
	//初期化
	int rigrigcomboid[5] = { IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO4, IDC_COMBO5 };
	int gpboxid[5] = { IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5 };
	s_customrig.rigelem[elemno].rigrigboneno = -1;
	s_customrig.rigelem[elemno].rigrigno = -1;
	CBone* levelbone = s_model->GetBoneByID(s_customrig.rigelem[elemno].boneno);
	if (levelbone){
		SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)levelbone->GetWBoneName());
	}
	else{
		SetDlgItemText(hDlgWnd, gpboxid[elemno], L"未設定");
	}


	//チェック　アンド　セット
	int combono;
	combono = (int)SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_GETCURSEL, 0, 0);
	s_customrig.rigelem[elemno].rigrigboneno = -1;
	s_customrig.rigelem[elemno].rigrigno = -1;
	if ((combono != 0) && (combono != CB_ERR)){
		WCHAR combolabel[256];
		ZeroMemory(combolabel, sizeof(WCHAR) * 256);
		SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_GETLBTEXT, combono, (LPARAM)combolabel);
//{
//	WCHAR strdbg[256];
//	swprintf_s(strdbg, 256, L"elemno %d, combolabel %s", elemno, combolabel);
//	::MessageBox(hDlgWnd, strdbg, L"combolabel", MB_OK);
//}
		size_t labellen = wcslen(combolabel);
		if ((labellen > 0) && (labellen < 256)){
			//format    [rigrigno]rigrigbonename[|]rigname
			if (combolabel[0] == L'['){
				WCHAR* prigrignoend = wcsstr(combolabel, L"]");
				if (prigrignoend){
					WCHAR strrigrigno[256];
					ZeroMemory(strrigrigno, sizeof(WCHAR) * 256);
					wcsncpy_s(strrigrigno, 256, combolabel + 1, (size_t)(prigrignoend - (combolabel + 1)));
					int rigrigno = _wtoi(strrigrigno);
//{
//	WCHAR strdbg[256];
//	swprintf_s(strdbg, 256, L"rigrigno %d", rigrigno);
//	::MessageBox(hDlgWnd, strdbg, L"rigrigno", MB_OK);
//}
					if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)){
						s_customrig.rigelem[elemno].rigrigno = rigrigno;

						WCHAR* pbonenameend = wcsstr(combolabel, L"[|]");
						if (pbonenameend){
							WCHAR rigrigbonename[256];
							ZeroMemory(rigrigbonename, sizeof(WCHAR) * 256);
							wcsncpy_s(rigrigbonename, 256, prigrignoend + 1, (size_t)(pbonenameend - (prigrignoend + 1)));
//{
//	WCHAR strdbg[256];
//	swprintf_s(strdbg, 256, L"rigrigbonename %s", rigrigbonename);
//	::MessageBox(hDlgWnd, strdbg, L"rigrigbonename", MB_OK);
//}
							CBone* rigrigbone = s_model->GetBoneByWName(rigrigbonename);
							if (rigrigbone){
								s_customrig.rigelem[elemno].rigrigboneno = rigrigbone->GetBoneNo();

//{
//	WCHAR strdbg[256];
//	swprintf_s(strdbg, 256, L"rigrigboneno %d", s_customrig.rigelem[elemno].rigrigboneno);
//	::MessageBox(hDlgWnd, strdbg, L"rigrigboneno", MB_OK);
//}
								int gpboxid[5] = { IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5 };
								SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)rigrigbone->GetWBoneName());
							}
						}
					}
				}
			}
		}
	}

	EnableRigAxisUV(hDlgWnd);

	return 0;

}



LRESULT CALLBACK CustomRigDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

	int rigrigcomboid[5] = { IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO4, IDC_COMBO5 };

	switch (msg) {
	case WM_INITDIALOG:
	{
		CustomRig2Dlg(hDlgWnd);
		return FALSE;
	}
	break;
	case WM_COMMAND:
		if (HIWORD(wp) == CBN_SELCHANGE){
			switch (LOWORD(wp)) {
			case IDC_COMBO1:
				CheckRigRigCombo(hDlgWnd, 0);
				break;
			case IDC_COMBO2:
				CheckRigRigCombo(hDlgWnd, 1);
				break;
			case IDC_COMBO3:
				CheckRigRigCombo(hDlgWnd, 2);
				break;
			case IDC_COMBO4:
				CheckRigRigCombo(hDlgWnd, 3);
				break;
			case IDC_COMBO5:
				CheckRigRigCombo(hDlgWnd, 4);
				break;
			case IDC_CHILNUM:
			{
				int combono = (int)SendMessage(GetDlgItem(hDlgWnd, IDC_CHILNUM), CB_GETCURSEL, 0, 0);
				if ((combono >= 0) && (combono < MAXRIGELEMNUM)){
					SetCustomRigDlgLevel(hDlgWnd, combono + 1);
				}
			}
			break;
			default:
				break;
			}
		}
		else{
			switch (LOWORD(wp)) {
			case IDOK:
			{
				WCHAR strrigname[256] = { 0L };
				GetDlgItemText(hDlgWnd, IDC_RIGNAME, strrigname, 256);
				wcscpy_s(s_customrig.rigname, 256, strrigname);

				int axisuid[5] = { IDC_AXIS_U1, IDC_AXIS_U2, IDC_AXIS_U3, IDC_AXIS_U4, IDC_AXIS_U5 };
				int axisvid[5] = { IDC_AXIS_V1, IDC_AXIS_V2, IDC_AXIS_V3, IDC_AXIS_V4, IDC_AXIS_V5 };
				int rateuid[5] = { IDC_RATE_U1, IDC_RATE_U2, IDC_RATE_U3, IDC_RATE_U4, IDC_RATE_U5 };
				int ratevid[5] = { IDC_RATE_V1, IDC_RATE_V2, IDC_RATE_V3, IDC_RATE_V4, IDC_RATE_V5 };
				int enableuid[5] = { IDC_ENABLEU1, IDC_ENABLEU2, IDC_ENABLEU3, IDC_ENABLEU4, IDC_ENABLEU5 };
				int enablevid[5] = { IDC_ENABLEV1, IDC_ENABLEV2, IDC_ENABLEV3, IDC_ENABLEV4, IDC_ENABLEV5 };

				int elemno;
				for (elemno = 0; elemno < s_customrig.elemnum; elemno++){

					CheckRigRigCombo(hDlgWnd, elemno);


					int combono = (int)SendMessage(GetDlgItem(hDlgWnd, axisuid[elemno]), CB_GETCURSEL, 0, 0);
					if ((combono >= AXIS_X) && (combono <= AXIS_Z)){
						s_customrig.rigelem[elemno].transuv[0].axiskind = combono;
					}
					combono = (int)SendMessage(GetDlgItem(hDlgWnd, axisvid[elemno]), CB_GETCURSEL, 0, 0);
					if ((combono >= AXIS_X) && (combono <= AXIS_Z)){
						s_customrig.rigelem[elemno].transuv[1].axiskind = combono;
					}

					int ret;
					float tmprate;
					ret = GetCustomRigRateVal(hDlgWnd, rateuid[elemno], &tmprate);
					if (ret){
						::MessageBox(hDlgWnd, L"横倍率パラメータが不正です。倍率は-100.0から100.0です。", L"入力エラー", MB_OK);
						return 0;
					}
					s_customrig.rigelem[elemno].transuv[0].applyrate = tmprate;

					ret = GetCustomRigRateVal(hDlgWnd, ratevid[elemno], &tmprate);
					if (ret){
						::MessageBox(hDlgWnd, L"縦倍率パラメータが不正です。倍率は-100.0から100.0です。", L"入力エラー", MB_OK);
						return 0;
					}
					s_customrig.rigelem[elemno].transuv[1].applyrate = tmprate;


					if (IsDlgButtonChecked(hDlgWnd, enableuid[elemno]) == BST_CHECKED){
						s_customrig.rigelem[elemno].transuv[0].enable = 1;
					}
					else{
						s_customrig.rigelem[elemno].transuv[0].enable = 0;
					}

					if (IsDlgButtonChecked(hDlgWnd, enablevid[elemno]) == BST_CHECKED){
						s_customrig.rigelem[elemno].transuv[1].enable = 1;
					}
					else{
						s_customrig.rigelem[elemno].transuv[1].enable = 0;
					}

				}


				int isvalid = IsValidCustomRig(s_model, s_customrig, s_customrigbone);
				if (isvalid == 0){
					::MessageBox(hDlgWnd, L"パラメータが不正です。", L"入力エラー", MB_OK);
					return 0;
				}

				CustomRig2Bone();

				//EndDialog(hDlgWnd, IDOK);
			}
			break;
			case IDCANCEL:
				//EndDialog(hDlgWnd, IDCANCEL);
				break;
			default:
				return FALSE;
			}
		}
		break;
	case WM_CLOSE:
		if (s_customrigdlg){
			DestroyWindow(s_customrigdlg);
			s_customrigdlg = 0;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;

}

int BoneRClick(int srcboneno)
{
	if (!s_model){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}

	if (srcboneno < 0){
		s_ikcnt = 0;
		//SetCapture(s_mainwnd);
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_mainwnd, &ptCursor);
		s_pickinfo.clickpos = ptCursor;
		s_pickinfo.mousepos = ptCursor;
		s_pickinfo.mousebefpos = ptCursor;
		s_pickinfo.diffmouse = ChaVector2(0.0f, 0.0f);
		s_pickinfo.firstdiff = ChaVector2(0.0f, 0.0f);

		s_pickinfo.winx = (int)DXUTGetWindowWidth();
		s_pickinfo.winy = (int)DXUTGetWindowHeight();
		s_pickinfo.pickrange = 6;

		s_pickinfo.pickobjno = -1;

		CallF(s_model->PickBone(&s_pickinfo), return 1);
		if (s_pickinfo.pickobjno >= 0){
			s_curboneno = s_pickinfo.pickobjno;
		}
	}

	if (s_curboneno > 0){
		if (s_owpTimeline){
			s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
		}

		ChangeCurrentBone();

		if (s_curboneno >= 0){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				HWND parwnd;
				parwnd = s_mainwnd;

				CRMenuMain* rmenu;
				rmenu = new CRMenuMain(IDR_RMENU);
				if (!rmenu){
					return 1;
				}
				int ret;
				ret = rmenu->Create(parwnd);
				if (ret){
					return 1;
				}

				HMENU submenu = rmenu->GetSubMenu();

				CRMenuMain* rsubmenu[MAXRIGNUM];
				ZeroMemory(rsubmenu, sizeof(CRMenuMain*) * MAXRIGNUM);


				int menunum;
				menunum = GetMenuItemCount(submenu);
				int menuno;
				for (menuno = 0; menuno < menunum; menuno++)
				{
					RemoveMenu(submenu, 0, MF_BYPOSITION);
				}
				s_customrigmenuindex.clear();

				if (curbone->GetPosConstraint() == 0){
					AppendMenu(submenu, MF_STRING, ID_RMENU_PHYSICSCONSTRAINT, L"Physics Pos Constraint設定");
				}
				else{
					AppendMenu(submenu, MF_STRING, ID_RMENU_PHYSICSCONSTRAINT, L"Physics Pos Constraint解除");
				}

				if (curbone->GetMass0() == 0){
					AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0, L"一時Mass0設定");
				}
				else{
					AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0, L"一時Mass0解除");
				}

				if (curbone->GetExcludeMv() == 0){
					AppendMenu(submenu, MF_STRING, ID_RMENU_EXCLUDE_MV, L"MV除外設定");
				}
				else{
					AppendMenu(submenu, MF_STRING, ID_RMENU_EXCLUDE_MV, L"MV除外解除");
				}


				AppendMenu(submenu, MF_STRING, ID_RMENU_0, L"新規Rig");
				int setmenuno = 1;
				int rigno;
				for (rigno = 0; rigno < MAXRIGNUM; rigno++){
					CUSTOMRIG currig = curbone->GetCustomRig(rigno);
					if (currig.useflag == 2){
						int setmenuid = ID_RMENU_0 + setmenuno;

						//AppendMenu(submenu, MF_STRING, setmenuid, currig.rigname);
						s_customrigmenuindex[setmenuno] = rigno;

						rsubmenu[rigno] = new CRMenuMain(IDR_RMENU);
						if (!rsubmenu[rigno]){
							return 1;
						}
						ret = rsubmenu[rigno]->CreatePopupMenu(parwnd, submenu, currig.rigname);
						if (ret){
							return 1;
						}
						HMENU subsubmenu = rsubmenu[rigno]->GetSubMenu();
						int subsubmenunum;
						subsubmenunum = GetMenuItemCount(subsubmenu);
						int subsubmenuno;
						for (subsubmenuno = 0; subsubmenuno < subsubmenunum; subsubmenuno++)
						{
							RemoveMenu(subsubmenu, 0, MF_BYPOSITION);
						}

						int subsubid1 = setmenuid + MAXRIGNUM;
						int subsubid2 = setmenuid + MAXRIGNUM * 2;
						AppendMenu(subsubmenu, MF_STRING, subsubid1, L"Rig設定");
						AppendMenu(subsubmenu, MF_STRING, subsubid2, L"Rig実行");

						setmenuno++;
					}
				}


				POINT pt;
				GetCursorPos(&pt);
				//::ScreenToClient(parwnd, &pt);

				int currigno = -1;
				int menuid;
				menuid = rmenu->TrackPopupMenu(pt);
				if (menuid == ID_RMENU_PHYSICSCONSTRAINT){
					//toggle
					if (curbone->GetPosConstraint() == 0){
						s_model->CreatePhysicsPosConstraint(curbone);
					}
					else{
						s_model->DestroyPhysicsPosConstraint(curbone);
					}
				}
				else if (menuid == ID_RMENU_MASS0){
					//toggle
					if (curbone->GetMass0() == 0){
						s_model->SetMass0(curbone);
					}
					else{
						s_model->RestoreMass(curbone);
					}
				}
				else if (menuid == ID_RMENU_EXCLUDE_MV){
					//toggle
					if (curbone->GetExcludeMv() == 0){
						curbone->SetExcludeMv(1);
					}
					else{
						curbone->SetExcludeMv(0);
					}
				}

				else if (menuid == ID_RMENU_0){
					//新規
					currigno = -1;
					DispCustomRigDlg(currigno);
				}
				else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 2))){
					//設定
					currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM)];
					DispCustomRigDlg(currigno);

				}
				else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM * 2)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 3))){
					//実行
					currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM * 2)];
					Bone2CustomRig(currigno);
					if (s_customrigbone){
						s_oprigflag = 1;
					}
				}
				
				for (rigno = 0; rigno < MAXRIGNUM; rigno++){
					CRMenuMain* curmenu = rsubmenu[rigno];
					if (curmenu){
						curmenu->Destroy();
						delete curmenu;
					}
				}

				rmenu->Destroy();
				delete rmenu;

			}
		}
	}

	return 0;
}

int EnableRigAxisUV(HWND hDlgWnd)
{
	int axisuid[5] = { IDC_AXIS_U1, IDC_AXIS_U2, IDC_AXIS_U3, IDC_AXIS_U4, IDC_AXIS_U5 };
	int axisvid[5] = { IDC_AXIS_V1, IDC_AXIS_V2, IDC_AXIS_V3, IDC_AXIS_V4, IDC_AXIS_V5 };
	int rigrigcomboid[5] = { IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO4, IDC_COMBO5 };

	int elemno;
	for (elemno = 0; elemno < s_customrig.elemnum; elemno++){
		RIGELEM currigelem = s_customrig.rigelem[elemno];
		if (currigelem.rigrigboneno >= 0){
			EnableWindow(GetDlgItem(hDlgWnd, axisuid[elemno]), false);
			EnableWindow(GetDlgItem(hDlgWnd, axisvid[elemno]), false);
		}
		else{
			EnableWindow(GetDlgItem(hDlgWnd, axisuid[elemno]), true);
			EnableWindow(GetDlgItem(hDlgWnd, axisvid[elemno]), true);
		}
	}

	return 0;
}

int SetRigRigCombo(HWND hDlgWnd, int elemno)
{
	int gpboxid[5] = { IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5 };
	int rigrigcomboid[5] = { IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO4, IDC_COMBO5 };

	WCHAR strcombo[256];

	SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"未設定");
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_RESETCONTENT, 0, 0);
	wcscpy_s(strcombo, 256, L"通常ボーン");
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_SETCURSEL, 0, 0);


	if (elemno < s_customrig.elemnum){
		RIGELEM currigelem = s_customrig.rigelem[elemno];
		int selrigrigcombono = 0;
		SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_RESETCONTENT, 0, 0);
		wcscpy_s(strcombo, 256, L"通常ボーン");
		SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
		int setcombono = 1;

		CBone* rigrigbone = s_model->GetBoneByID(currigelem.rigrigboneno);
		CBone* currigrigbone;
		map<int, CBone*>::iterator itrcurrigrigbone;
		for (itrcurrigrigbone = s_model->GetBoneListBegin(); itrcurrigrigbone != s_model->GetBoneListEnd(); itrcurrigrigbone++){
			currigrigbone = itrcurrigrigbone->second;
			if (currigrigbone){
				WCHAR rigrigbonename[256];
				ZeroMemory(rigrigbonename, sizeof(WCHAR) * 256);
				wcscpy_s(rigrigbonename, 256, currigrigbone->GetWBoneName());
				int rigrigno;
				for (rigrigno = 0; rigrigno < MAXRIGNUM; rigrigno++){
					CUSTOMRIG rigrig = currigrigbone->GetCustomRig(rigrigno);
					if (rigrig.useflag == 2){
						int isvalid = IsValidCustomRig(s_model, rigrig, currigrigbone);
						if (isvalid == 1){
							swprintf_s(strcombo, 256, L"[%d]%s[|]%s", rigrigno, rigrigbonename, rigrig.rigname);
							SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
							if (rigrigbone && (currigrigbone == rigrigbone) && (rigrigno == currigelem.rigrigno)){
								selrigrigcombono = setcombono;
								SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)rigrigbone->GetWBoneName());
							}
							setcombono++;
						}
					}
				}
			}
		}
		SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_SETCURSEL, selrigrigcombono, 0);
	}

	return 0;
}

int SetCustomRigDlgLevel(HWND hDlgWnd, int levelnum)
{
	if (!s_model){
		return 0;
	}
	if (!s_customrigbone){
		return 0;
	}
	if (!s_model->GetTopBone()){
		return 0;
	}
	if (!s_customrigdlg){
		return 0;
	}

	if ((levelnum >= 1) && (levelnum <= MAXRIGELEMNUM)){
		int gpboxid[5] = { IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5 };

		int parno = 1;
		CBone* parbone = s_customrigbone->GetParent();
		while (parbone && (parno < MAXRIGELEMNUM) && (parno < levelnum)){
			SetDlgItemText(s_customrigdlg, gpboxid[parno], (LPCWSTR)parbone->GetWBoneName());
			s_customrig.rigelem[parno].boneno = parbone->GetBoneNo();
			parbone = parbone->GetParent();
			parno++;
		}

		//int newlevelnum = parno;
		//s_customrig.elemnum = levelnum;
		//SendMessage(GetDlgItem(s_customrigdlg, IDC_CHILNUM), CB_SETCURSEL, newlevelnum - 1, 0);
		s_customrig.elemnum = levelnum;

		int elemno;
		for (elemno = 0; elemno < MAXRIGELEMNUM; elemno++){
			SetRigRigCombo(hDlgWnd, elemno);
		}
	}

	EnableRigAxisUV(hDlgWnd);


	return 0;
}

int ToggleRig()
{
	if (s_customrigbone){
		if (s_oprigflag == 0){
			s_oprigflag = 1;
			s_curboneno = s_customrigbone->GetBoneNo();
			s_pickinfo.buttonflag = PICK_CENTER;
		}
		else{
			s_oprigflag = 0;
			s_pickinfo.buttonflag = 0;

			if (s_customrigdlg){
				DestroyWindow(s_customrigdlg);
				s_customrigdlg = 0;
			}
		}
	}
	else{
		s_oprigflag = 0;
		if (s_customrigdlg){
			DestroyWindow(s_customrigdlg);
			s_customrigdlg = 0;
		}
		s_pickinfo.buttonflag = 0;
	}
	return 0;
}

int GetSymRootMode()
{
	/*
	enum
	{
		//for bit mask operation
		SYMROOTBONE_SAMEORG = 0,
		SYMROOTBONE_SYMDIR = 1,
		SYMROOTBONE_SYMPOS = 2
	};
	*/

	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 0;
	}
	int ret;
	ret = rmenu->Create(s_mainwnd);
	if (ret){
		return 0;
	}

	HMENU submenu = rmenu->GetSubMenu();

	int menunum;
	menunum = GetMenuItemCount(submenu);
	int menuno;
	for (menuno = 0; menuno < menunum; menuno++)
	{
		RemoveMenu(submenu, 0, MF_BYPOSITION);
	}


	int setmenuid;
	setmenuid = ID_RMENU_0;
	AppendMenu(submenu, MF_STRING, setmenuid, L"ルートボーンはコピー元と同じ");
	setmenuid = ID_RMENU_0 + 1;
	AppendMenu(submenu, MF_STRING, setmenuid, L"ルートボーンは位置と向き共対称");
	setmenuid = ID_RMENU_0 + 2;
	AppendMenu(submenu, MF_STRING, setmenuid, L"ルートボーンは向きだけ対称");
	setmenuid = ID_RMENU_0 + 3;
	AppendMenu(submenu, MF_STRING, setmenuid, L"ルートボーンは位置だけ対称");


	POINT pt;
	GetCursorPos(&pt);

	int retmode = 0;
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + 3))){
		switch(menuid){
		case (ID_RMENU_0) :
			retmode = SYMROOTBONE_SAMEORG;
			break;
		case (ID_RMENU_0 + 1) :
			retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
			break;
		case (ID_RMENU_0 + 2) :
			retmode = SYMROOTBONE_SYMDIR;
			break;
		case (ID_RMENU_0 + 3) :
			retmode = SYMROOTBONE_SYMPOS;
			break;
		default:
			retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
			break;
		}
	}

	rmenu->Destroy();
	delete rmenu;


	return retmode;
}

void AutoCameraTarget()
{
	s_camtargetflag = (int)s_CamTargetCheckBox->GetChecked();
	if (s_model && (s_curboneno >= 0) && s_camtargetflag){
		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		_ASSERT(curbone);
		if (curbone){
			g_camtargetpos = curbone->GetChildWorld();
			g_Camera.SetViewParams(&g_camEye, &g_camtargetpos);

			ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			ChaVector3 diffv;
			diffv = g_camEye - g_camtargetpos;
			s_camdist = ChaVector3Length(&diffv);
		}
	}
}


/////// for directx9

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	//// No fallback defined by this app, so reject any device that doesn't support at least ps2.0
	//if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	//	return false;

	//// Skip backbuffer formats that don't support alpha blending
	//IDirect3D9* pD3D = DXUTGetD3D9Object();
	//if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
	//	AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
	//	D3DRTYPE_TEXTURE, BackBufferFormat)))
	//	return false;

	//return true;
	return false;
}


//////////////////////////////////////////
int OnTimeLineSelectFormSelectedKey()
{
	s_editrange.Clear();
	if (s_model && s_model->GetCurMotInfo()) {
		if (s_owpTimeline && s_owpLTimeline) {
			s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
			int keynum;
			double startframe, endframe, applyframe;
			s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);

			if (s_underselectingframe != 0) {
				if (s_buttonselectstart <= s_buttonselectend) {
					s_owpLTimeline->setCurrentTime(endframe, false);
				}
				else {
					s_owpLTimeline->setCurrentTime(startframe, false);
				}
			}
			else {
				s_owpLTimeline->setCurrentTime(applyframe, false);
				AddEditRangeHistory();
			}
		}
	}

	return 0;
}

int OnTimeLineButtonSelectFromSelectStartEnd(int tothelastflag)
{
	s_buttonselecttothelast = tothelastflag;
	if (s_owpLTimeline) {
		s_owpLTimeline->selectClear(false);
		if (s_buttonselectstart != s_buttonselectend) {
			s_owpLTimeline->OnButtonSelect(s_buttonselectstart, s_buttonselectend, s_buttonselecttothelast);
		}
	}

	OnTimeLineSelectFormSelectedKey();

	return 0;
}

int OnTimeLineCursor(int mbuttonflag, double newframe)
{
	if (s_owpLTimeline && s_model && s_model->GetCurMotInfo()) {
		double curframe;
		if (mbuttonflag != 2) {
			curframe = s_owpLTimeline->getCurrentTime();// 選択時刻
			s_owpTimeline->setCurrentTime(curframe, false);
		}
		else {
			curframe = newframe;
			s_owpTimeline->setCurrentTime(curframe, false);
			s_owpLTimeline->setCurrentTime(curframe, false);
		}
	}

	return 0;
}

int OnTimeLineMButtonDown(bool ctrlshiftflag)
{
	//if (s_underselectingframe == 0){
	if (s_mbuttoncnt == 0) {
		if (ctrlshiftflag == false) {
			s_underselectingframe = 1;
		}
		else {
			s_underselectingframe = 2;
		}
		if (g_previewFlag == 0) {
			if (s_owpLTimeline) {
				s_buttonselectstart = s_owpLTimeline->getCurrentTime();
				s_buttonselectend = s_buttonselectstart;
				OnTimeLineCursor(1, 0.0);
			}
		}
	}
	else {
		s_underselectingframe = 0;
		OnTimeLineButtonSelectFromSelectStartEnd(0);
	}

	DbgOut(L"OnTimeLineMButtonDown : underselectingframe %d, start %lf, end %lf\r\n", s_underselectingframe, s_buttonselectstart, s_buttonselectend);

	return 0;
}


int OnTimeLineWheel()
{

	DbgOut(L"OnTimeLineWheel Called\r\n");

	if (s_owpLTimeline) {
		if ((s_underselectingframe == 1) || (s_underselectingframe == 2)) {
			int delta = 0;
			double delta2 = 0;

			int adkeyflag = 0;

			//A D key
			if (g_keybuf['A'] & 0x80) {
				adkeyflag = 1;
				if ((s_akeycnt % 5) == 0) {
					if (g_controlkey == false) {
						delta2 = -5;
					}
					else {
						delta2 = -1;
					}
				}
				else {
					delta2 = 0;
				}
			}
			else if (g_keybuf['D'] & 0x80) {
				adkeyflag = 1;
				if ((s_dkeycnt % 5) == 0) {
					if (g_controlkey == false) {
						delta2 = 5;
					}
					else {
						delta2 = 1;
					}
				}
				else {
					delta2 = 0;
				}
			}

			if (adkeyflag == 0) {//timelineのwheeldeltaはホイールを回していない間は更新されずに値が残るため、ホイールだけを扱うこと(キー処理中ではないこと)を明示的に確認する。
								 //マウス操作 MButton and Wheel, A D key
				delta = (int)(s_owpLTimeline->getMouseWheelDelta());
				if (g_controlkey == false) {
					delta2 = (double)delta / 20.0;
				}
				else {
					//delta2 = (double)delta / 100.0;//ctrlを押していたら[slowly]
					if (delta > 0) {
						delta2 = 1;
					}
					else if (delta < 0) {
						delta2 = -1;
					}
					else {
						delta2 = 0;
					}
				}
			}

			//timeline
			if (delta2 != 0.0) {
				s_buttonselectend += delta2;
				DbgOut(L"OnTimeLineWheel 0 : start %lf, end %lf, delta %lf\r\n", s_buttonselectstart, s_buttonselectend, delta2);

				OnTimeLineButtonSelectFromSelectStartEnd(0);
			}
		}
		else {
			DbgOut(L"OnTimeLineWheel 1 : start %lf, end %lf\r\n", s_buttonselectstart, s_buttonselectend);

			int delta = 0;
			double delta2 = 0;
			delta = (int)(s_owpLTimeline->getMouseWheelDelta());
			if (g_controlkey == false) {
				delta2 = (double)delta / 20.0;
			}
			else {
				//delta2 = (double)delta / 100.0;//ctrlを押していたら[slowly]
				if (delta > 0) {
					delta2 = 1;
				}
				else if (delta < 0) {
					delta2 = -1;
				}
				else {
					delta2 = 0;
				}
			}
			if (delta2 != 0.0) {
				double curframe = s_owpLTimeline->getCurrentTime();
				double newframe = curframe + delta2;
				OnTimeLineCursor(2, newframe);
			}

		}
	}

	return 0;
}

