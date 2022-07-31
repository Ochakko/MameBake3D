
#include "stdafx.h"
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


/*

//2021/02/24_1
開発の途中から
適切なパラメータ値を探すために
previewflag 5, 6の再生時にはパラメータを決め打ちして再生していた

そのためRigidParamdDialgで設定した値は
物理シミュレーション時には上書きされていた

今回、RigidParamsDialogを前面に出すことになって
previewflag 5 の再生時にはパラメータを決め打ちを止めた
よって挙動が変わるかもしれない

決め打ちの部分は
該当プログラム個所に
//決め打ち
というコメントをしておいたので
決め打ちパラメータ値を知りたい場合にはみてください

*/


/*
* 
* 2021/11/29
3DウインドウのPseudoLocalのチェックボックスは画面に出なくなりました
PseudoLocalIKは常にオンになります

3Dウインドウに新たにWallScrapingIKチェックボックスが加わりました
壁すりIKのオンとオフを指定するためのものです
壁すりIKとは
制限角度を越えてIK操作を行おうとした場合に
制限に達した成分以外は変化させるIKのことです
壁すりIKをオンにすることで制限角度下でも動かしやすいIK操作が可能です
言い方を変えると
壁すりIKをオフにするとXYZどれかの軸が制限値に達した場合にIKで動かすことが難しくなります

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

//#include <OrgWindow.h>
//InfoWindowでOrgWindowをincludeしている
#include <InfoWindow.h>

#define DBGH
#include <dbg.h>

#include <shlobj.h> //shell
#include <shlobj_core.h>
#include <objbase.h>
#include <Knownfolders.h>

#include <Commdlg.h>
//#include <ChaVecCalc.h>
#include <GlobalVar.h>

#include <mqomaterial.h>

#include <ChaFile.h>
#include <RetargetFile.h>
#include <ImpFile.h>
#include <RigidElemFile.h>
#include <RigidElem.h>
#include <MNLFile.h>
#include "IniFile.h"


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
#include "SettingsDlg.h"
#include "CopyHistoryDlg.h"
#include "CpInfoDlg.h"

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
#include "MotFilter.h"
#include <MotionPoint.h>

#include <EGPFile.h>

#include <Retarget.h>

#include "DSUpdateUnderTracking.h"
#include "PluginElem.h"

#include "SelectLSDlg.h"

#include <ThreadsUpdateMatrix.h>


//#include <uxtheme.h>
//#pragma ( lib, "UxTheme.lib" )

#include "..\Effects11\Inc\d3dx11effect.h"
#include "..\DS4HidInputLib\DSInput_dll.h"


#include <Windows.h>
#include <gdiplus.h>

#define WINDOWS_CLASS_NAME TEXT("OchakkoLab.MameBake3D.Window")

#define WM_USER_FOR_BATCH_PROGRESS	(WM_USER + 1)

#define MAXPLUGIN	255
#define OPENHISTORYMAXNUM	10


typedef struct tag_spaxis
{
	CMySprite* sprite;
	POINT dispcenter;
}SPAXIS, SPCAM, SPELEM;

typedef struct tag_spsw
{
	bool state;//ON : 1 or OFF : 0
	CMySprite* spriteON;
	CMySprite* spriteOFF;
	POINT dispcenter;
}SPGUISW;


//typedef struct tag_physikrec
//{
//	double time;
//	CBone* pbone;
//	ChaMatrix btmat;
//}PHYSIKREC;
//
//#define MAXPHYSIKRECCNT		(60 * 30)
//static std::vector<PHYSIKREC> s_physikrec0;
//static std::vector<PHYSIKREC> s_physikrec;
//static double s_phyikrectime = 0.0;
//static void PhysIKRec(double srcrectime);
//static void PhysIKRecReq(CBone* srcbone, double srcrectime);
//static void ApplyPhysIkRec();
//static void ApplyPhysIkRecReq(CBone* srcbone, double srcframe, double srcrectime);

static CThreadsUpdateMatrix s_tum;

static double s_rectime = 0.0;
static double s_reccnt = 0;

static int s_appcnt = 0;
static int s_launchbyc4 = 0;
static int s_launchc4diffx = 0;
static int s_launchc4diffy = 0;
static int s_onefps = 0;


static vector<wstring> s_bvh2fbxout;
static LONG s_bvh2fbxnum = 0;
static LONG s_bvh2fbxcnt = 0;
static LONG s_befbvh2fbxnum = 0;
static LONG s_befbvh2fbxcnt = 0;
LONG g_bvh2fbxbatchflag = 0;
HWND s_bvh2fbxbatchwnd = 0;
HANDLE s_bvh2fbxhandle1;
HANDLE s_bvh2fbxhandle2;

static LONG s_progressnum = 0;
static LONG s_progresscnt = 0;
static LONG s_progressmodelnum = 0;
static LONG s_progressmodelcnt = 0;
static LONG s_befprogressnum = 0;
static LONG s_befprogresscnt = 0;
static LONG s_befprogressmodelnum = 0;
static LONG s_befprogressmodelcnt = 0;

HWND s_progresswnd;


static vector<wstring> s_retargetout;
static LONG s_retargetnum = 0;
static LONG s_retargetcnt = 0;
static LONG s_befretargetnum = 0;
static LONG s_befretargetcnt = 0;
int s_saveretargetmodel = 0;
LONG g_retargetbatchflag;
HWND s_retargetbatchwnd;
HANDLE s_retargethandle1;
HANDLE s_retargethandle2;
static void WaitRetargetThreads();
static int s_convbone_model_batch_selindex;


LONG g_calclimitedwmflag;


static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR gdiplusToken;
Gdiplus::Image* g_mousehereimage = 0;
Gdiplus::Image* g_menuaimbarimage = 0;
int g_currentsubmenuid = 0;
POINT g_currentsubmenupos = { 0, 0 };
int g_submenuwidth = 32;
HWND g_filterdlghwnd = 0;

CRITICAL_SECTION g_CritSection_GetGP;

static int ClearLimitedWM();

static float CalcSelectScale(CBone* curboneptr);
static double CalcRefFrame();
static void ChangeCurDirFromMameMediaToTest();

static int OnPluginClose();
static int OnPluginPose();
static int s_onselectplugin = 0;
static CPluginElem* s_plugin = 0;

static void InitTimelineSelection();

static HWND GetOFWnd(POINT srcpoint);
static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
static HWND GetNearestEnumDist();
static BOOL CALLBACK EnumIDOKProc(HWND hwnd, LPARAM lParam);
static BOOL CALLBACK EnumTreeViewProc(HWND hwnd, LPARAM lParam);
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);

static void DSMessageBox(HWND srcparenthwnd, WCHAR* srcmessage, WCHAR* srctitle, LONG srcok);

/*
ID3D11DepthStencilState *g_pDSStateZCmp = 0;
ID3D11DepthStencilState *g_pDSStateZCmpAlways = 0;
ID3D11ShaderResourceView* g_presview = 0;

int	g_numthread = 3;
double g_btcalccnt = 3.0;
int g_dbgloadcnt = 0;
double g_calcfps = 60.0;

bool g_selecttolastFlag = false;
bool g_underselecttolast = false;
bool g_undereditrange = false;

bool g_limitdegflag = true;
bool g_wmatDirectSetFlag = false;
bool g_underRetargetFlag = false;

float g_impscale = 1.0f;
float g_mass = 1.0f;

ChaVector3 g_camEye = ChaVector3( 0.0f, 0.0f, 0.0f );
ChaVector3 g_camtargetpos = ChaVector3( 0.0f, 0.0f, 0.0f );

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

int g_previewFlag = 0;			// プレビューフラグ

int g_applyendflag = 0;
int g_slerpoffflag = 0;
int g_absikflag = 0;
int g_bonemarkflag = 1;
int g_pseudolocalflag = 1;
int g_boneaxis = 1;

CTexBank*	g_texbank = 0;

float g_tmpmqomult = 1.0f;
WCHAR g_tmpmqopath[MULTIPATH] = {0L};
float g_tmpbvhfilter = 100.0f;

ID3D11EffectTechnique* g_hRenderBoneL0 = 0;
ID3D11EffectTechnique* g_hRenderBoneL1 = 0;
ID3D11EffectTechnique* g_hRenderBoneL2 = 0;
ID3D11EffectTechnique* g_hRenderBoneL3 = 0;
ID3D11EffectTechnique* g_hRenderNoBoneL0 = 0;
ID3D11EffectTechnique* g_hRenderNoBoneL1 = 0;
ID3D11EffectTechnique* g_hRenderNoBoneL2 = 0;
ID3D11EffectTechnique* g_hRenderNoBoneL3 = 0;
ID3D11EffectTechnique* g_hRenderLine = 0;
ID3D11EffectTechnique* g_hRenderSprite = 0;

ID3D11EffectMatrixVariable* g_hm4x4Mat = 0;
ID3D11EffectMatrixVariable* g_hmWorld = 0;
ID3D11EffectMatrixVariable* g_hmVP = 0;

ID3D11EffectVectorVariable* g_hEyePos = 0;
ID3D11EffectScalarVariable* g_hnNumLight = 0;
ID3D11EffectVectorVariable* g_hLightDir = 0;
ID3D11EffectVectorVariable* g_hLightDiffuse = 0;
ID3D11EffectVectorVariable* g_hLightAmbient = 0;
ID3D11EffectVectorVariable* g_hSpriteOffset = 0;
ID3D11EffectVectorVariable* g_hSpriteScale = 0;
ID3D11EffectVectorVariable* g_hPm3Scale = 0;
ID3D11EffectVectorVariable* g_hPm3Offset = 0;


ID3D11EffectVectorVariable* g_hdiffuse = 0;
ID3D11EffectVectorVariable* g_hambient = 0;
ID3D11EffectVectorVariable* g_hspecular = 0;
ID3D11EffectScalarVariable* g_hpower = 0;
ID3D11EffectVectorVariable* g_hemissive = 0;
ID3D11EffectShaderResourceVariable* g_hMeshTexture = 0;

BYTE g_keybuf[256];
BYTE g_savekeybuf[256];

WCHAR g_basedir[ MAX_PATH ] = {0};

double						g_dspeed = 3.0;
//double						g_dspeed = 0.0;


float g_ikfirst = 1.0f;
float g_ikrate = 1.0f;
int g_applyrate = 50;
float g_physicsmvrate = 1.0f;


float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;

*/

enum {
	MB3D_WND_MAIN,
	MB3D_WND_3D,
	MB3D_WND_TREE,
	MB3D_WND_TOOL,
	MB3D_WND_TIMELINE,
	MB3D_WND_SIDE,
	MB3D_WND_MAX
};


static int s_savebonemarkflag = 1;
static int s_saverigidmarkflag = 1;

static WCHAR s_temppath[MAX_PATH] = { 0L };


static CDSUpdateUnderTracking* s_dsupdater = 0;
LONG g_undertrackingRMenu = 0;
LONG g_underApealingMouseHere = 0;

extern HANDLE g_hUnderTrackingThread;
extern DSManager manager;

bool g_enableDS = false;
static void InitDSValues();
static void GetDSValues();
static void DSColorAndVibration();
static void DSSelectWindowAndCtrl();//L1, square, triangle
static void DSSelectCharactor();//(L2 or R2) and L1
static void DSCrossButton(bool firstctrlselect);
static void DSCrossButtonSelectTree(bool firstctrlselect);
static void DSCrossButtonSelectUTGUI(bool firstctrlselect);
static void DSCrossButtonSelectToolCtrls(bool firstctrlselect);
static void DSCrossButtonSelectPlayerBtns(bool firstctrlselect);
static void DSCrossButtonSelectRigidCtrls(bool firstctrlselect);
static void DSCrossButtonSelectImpulseCtrls(bool firstctrlselect);
static void DSCrossButtonSelectGPCtrls(bool firstctrlselect);
static void DSCrossButtonSelectDampCtrls(bool firstctrlselect);
static void DSCrossButtonSelectRetargetCtrls(bool firstctrlselect);
static void DSCrossButtonSelectEulLimitCtrls(bool firstctrlselect);
static void DSOptionButtonRightClick();
static void DSR1ButtonSelectCurrentBone();//R1
static void DSR1ButtonSelectMotion();//(L2 or R2) and R1
static void DSAxisLMouseMove();
//static void DSAxisLSelectingPopupMenu();
static void DSAxisRMainMenuBar();
static void DSAimBarOK();
static void DSOButtonSelectedPopupMenu();
static void DSXButtonCancel();
static void DSL3R3ButtonMouseHere();


static void SelectNextWindow(int nextwndid);

static void SetMainWindowTitle();

static RECT s_rcmainwnd;
static RECT s_rc3dwnd;
static RECT s_rctreewnd;
static RECT s_rctoolwnd;
static RECT s_rcltwnd;
static RECT s_rcsidemenuwnd;
static RECT s_rcrigidwnd;
static RECT s_rcinfownd;
static RECT s_rcmainmenuaimbarwnd;
static void ChangeMouseSetCapture();
static void ChangeMouseReleaseCapture();


#define MB3D_DSBUTTONNUM	14
#define MB3D_DSAXISNUM		6
#define MB3D_DSAXISSRH		(0.70f)
static int s_curaimbarno = -1;
static int s_dsdeviceid = -1;
static int s_currentwndid = 0;
static HWND s_currenthwnd = 0;
static int s_restorewndid = 0;
static HWND s_restorehwnd = 0;
static POINT s_restorecursorpos;
static int s_currentctrlid = -1;
static HWND s_currentctrlhwnd = 0;
static int s_wmlbuttonup = 0;
//#define SUBMENUNUM	10
//static int g_currentsubmenuid = 0;//globalへ
static int s_currentsubmenuitemid = 0;
static HWND s_ofhwnd = 0;
static HWND s_messageboxhwnd = 0;
static int s_messageboxpushcnt = 0;
static HWND s_getfilenamehwnd = 0;
static HWND s_getfilenametreeview = 0;

static int s_getsym_retmode = 0;



typedef struct tag_enumdist
{
	HWND hwnd;
	float dist;
}ENUMDIST;
static std::vector<ENUMDIST> s_enumdist;

static int s_curdsutguikind = 0;
static int s_curdsutguino = 0;
static std::vector<CDXUTControl*> s_dsutgui0;
static std::vector<CDXUTControl*> s_dsutgui1;
static std::vector<CDXUTControl*> s_dsutgui2;
static std::vector<CDXUTControl*> s_dsutgui3;
static std::vector<UINT> s_dsutguiid0;
static std::vector<UINT> s_dsutguiid1;
static std::vector<UINT> s_dsutguiid2;
static std::vector<UINT> s_dsutguiid3;

static int s_curdstoolctrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dstoolctrls;

static int s_curdsplayerbtnno = 0;

static int s_curdsrigidctrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dsrigidctrls;

static int s_curdsimpulsectrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dsimpulsectrls;

static int s_curdsgpctrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dsgpctrls;

static int s_curdsdampctrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dsdampctrls;

static int s_curdsretargetctrlno = 0;
static std::vector<OrgWinGUI::OrgWindowParts*> s_dsretargetctrls;

static int s_curdseullimitctrlno = 0;
static std::vector<UINT> s_dseullimitctrls;


static int s_dsbuttondown[MB3D_DSBUTTONNUM];
static int s_bef_dsbuttondown[MB3D_DSBUTTONNUM];
static int s_dsbuttonup[MB3D_DSBUTTONNUM];
static int s_bef_dsbuttonup[MB3D_DSBUTTONNUM];
static float s_dsaxisvalue[MB3D_DSAXISNUM];
static float s_bef_dsaxisvalue[MB3D_DSAXISNUM];
static int s_dsaxisOverSrh[MB3D_DSAXISNUM];
static int s_bef_dsaxisOverSrh[MB3D_DSAXISNUM];
static int s_dsaxisMOverSrh[MB3D_DSAXISNUM];
static int s_bef_dsaxisMOverSrh[MB3D_DSAXISNUM];
static int s_dspushedOK = 0;
static int s_dspushedL3 = 0;
static int s_dspushedR3 = 0;
//static int s_dsmousewait = 0;

static HWND s_mqodlghwnd = 0;
static bool s_underframecopydlg = false;
static CColiIDDlg* s_pcolidlg = 0;
static bool s_undercolidlg = false;
static CGColiIDDlg* s_pgcolidlg = 0;
static bool s_undergcolidlg = false;
static HWND s_motpropdlghwnd = 0;
static HWND s_savechadlghwnd = 0;
static HWND s_bvhdlghwnd = 0;
static HWND s_saveredlghwnd = 0;
static HWND s_saveimpdlghwnd = 0;
static HWND s_savegcodlghwnd = 0;
static HWND s_rotzisdlghwnd = 0;
static HWND s_customrighwnd = 0;
static HWND s_exportxdlghwnd = 0;


static int s_opedelmodelcnt = -1;
static int s_opedelmotioncnt = -1;
static bool s_underdelmodel = false;
static bool s_underdelmotion = false;
static bool s_underselectmodel = false;
static bool s_underselectmotion = false;
static int s_opeselectmodelcnt = -1;
static int s_opeselectmotioncnt = -1;

static int s_underanglelimithscroll = 0;


static bool s_nowloading = true;
static void OnRenderNowLoading();

extern map<CModel*,int> g_bonecntmap;
extern int gNumIslands;
extern void InitCustomRig(CUSTOMRIG* dstcr, CBone* parentbone, int rigno);
extern int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parentbone);
//void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* childbone);
extern int IsValidRigElem(CModel* srcmodel, RIGELEM srcrigelem);

//extern void DXUTSetOverrideSize(int srcw, int srch);

extern void OrgWinGUI::InitEulKeys();
extern void OrgWinGUI::DestroyEulKeys();
extern void OrgWinGUI::InitKeys();
extern void OrgWinGUI::DestroyKeys();

extern HANDLE g_hEvent; //手動リセットイベント



CRITICAL_SECTION s_CritSection_LTimeline;


//ChaMatrix s_selectmat;//for display manipulator
//ChaMatrix s_ikselectmat;//for ik, fk
ChaMatrix s_selectmat;//for display manipulator
ChaMatrix s_selectmat_posture;//for display manipulator
ChaMatrix s_ikselectmat;//for ik, fk


static HWND s_mainhwnd = NULL;

static int s_onragdollik = 0;
static int s_physicskind = 0;
static int s_platemenukind = 0;
static int s_platemenuno = 1;


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

static CMQOMaterial* s_matrig = 0;
static ChaVector4 s_matrigmat;



static bool s_dispanglelimit = false;
static HWND s_anglelimitdlg = 0;
static ANGLELIMIT s_anglelimit;
static ANGLELIMIT s_anglelimitcopy;
static CBone* s_anglelimitbone = 0;
static bool s_beflimitdegflag = true;
static bool s_savelimitdegflag = true;


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
//static int s_underselectingframe = 0;//globalに変更
static double s_buttonselectstart = 0.0;
static double s_buttonselectend = 0.0;
static int s_buttonselecttothelast = 0;

static float s_selectscale = 1.0f;
//static int s_sethipstra = 0;
static CFrameCopyDlg s_selbonedlg;
static int s_allmodelbone = 0;

static std::vector<HISTORYELEM> s_cptfilename;
static CCopyHistoryDlg s_copyhistorydlg;

//float g_initcamdist = 10.0f;
//static float s_projnear = 0.01f;
float g_initcamdist = 50.0f;
//static float s_projnear = 0.001f;
//static float s_projnear = 1.0f;
static float s_projnear = 0.01f;
static float s_fAspectRatio = 1.0f;
static float s_cammvstep = 100.0f;
static int s_editmotionflag = -1;
static int s_tkeyflag = 0;

static WCHAR s_strcurrent[256] = L"Move To Current Frame";
static WCHAR s_streditrange[256] = L"Drag Edit Range";
static WCHAR s_strmark[256] = L"Mark Key Line";



//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.2f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1e8 };
//float g_a_kval[3] = { powf( 10.0f, 0.0f ), powf( 10.0f, 0.3f ), 1e8 };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1000.0f };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.27f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), powf( 10.0f, 3.3f ) };
//float g_l_kval[3] = { powf( 10.0f, 2.0f ), powf( 10.0f, 2.61f ), 1000.0f };


bool g_controlkey = false;
bool g_shiftkey = false;
bool g_ctrlshiftkeyformb = false;//ForMiddleButton
static int s_akeycnt = 0;
static int s_dkeycnt = 0;
static int s_1keycnt = 0;

static bool s_dispsampleui = true;
static HWND s_sampleuihwnd = 0;

//double g_erp = 1.0;
double g_erp = 0.8;
//static float g_erp = 0.99f;
//static float g_erp = 0.75f;
//static double g_erp = 0.5;
//static float g_erp = 0.2f;
//static float s_impup = 0.0f;


static int s_savepreviewFlag = 0;
double s_btstartframe = 0.0;
bool g_btsimurecflag = false;

static FbxManager* s_psdk = 0;
static BPWorld* s_bpWorld = 0;
static btDynamicsWorld* s_btWorld = 0;

using namespace OrgWinGUI;

static ChaMatrix s_inimat;
static double s_time = 0.0;
//static double s_difftime = 0.0;
static int s_ikkind = 0;

//PICKRANGEを大きくするとジョイントではなく疑似ボーンドラッグまで可能になるが、マニピュレータのリングのpickが難しくなる
#define PICKRANGE	16
static PICKINFO s_pickinfo;
static vector<TLELEM> s_tlarray;


//static int s_curmotmenuindex = -1;
//static int s_curreindex = -1;
//static int s_rgdindex = -1;
	//each character each param //2021/03/18 
static map<CModel*, int> s_motmenuindexmap;
static map<CModel*, int> s_reindexmap;
static map<CModel*, int> s_rgdindexmap;


static HWND		s_3dwnd = 0;
static HMENU	s_mainmenu = 0;
static HMENU	s_animmenu = 0;
static HMENU	s_morphmenu = 0;
static HMENU	s_modelmenu = 0;
static HMENU	s_remenu = 0;
static HMENU	s_rgdmenu = 0;
static HMENU	s_impmenu = 0;
static int s_filterindex = 1;
static HMENU	s_cursubmenu = 0;


#define MAINMENUAIMBARH		32


static int s_totalwndwidth = (1216 + 450);
static int s_totalwndheight = 950;
static int s_2ndposy = 600;

static int s_mainwidth = 800;
static int s_mainheight = (520 - MAINMENUAIMBARH);
static int s_bufwidth = 800;
static int s_bufheight = (520 - MAINMENUAIMBARH);

static int s_timelinewidth = 400;
static int s_timelineheight = s_2ndposy - MAINMENUAIMBARH;

static int s_longtimelinewidth = 970;
static int s_longtimelineheight = s_totalwndheight - s_2ndposy - MAINMENUAIMBARH - 18;

static int s_toolwidth = 230;
static int s_toolheight = 290;

static int s_infowinwidth = s_mainwidth;
static int s_infowinheight = s_2ndposy - s_mainheight - MAINMENUAIMBARH;

static int s_sidemenuwidth = 450;
static int s_sidemenuheight = MAINMENUAIMBARH;

static int s_sidewidth = s_sidemenuwidth;
static int s_sideheight = s_totalwndheight - MAINMENUAIMBARH - s_sidemenuheight - 28;

static int s_guibarX0 = 120;

static ID3D11Device* s_pdev = 0;

static CModel* s_model = NULL;
static CModel* s_select = NULL;
static CModel* s_select_posture = NULL;
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
static ChaMatrix s_selm_posture = ChaMatrix(0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);

static void OrgWindowListenMouse(bool srcflag);

static OrgWindow* s_timelineWnd = 0;
static OWP_Timeline* s_owpTimeline = 0;
static OWP_PlayerButton* s_owpPlayerButton = 0;
static OWP_CheckBoxA* s_parentcheck = 0;

static OrgWindow* s_LtimelineWnd = 0;
static OWP_Timeline* s_owpLTimeline = 0;
static OWP_EulerGraph* s_owpEulerGraph = 0;
static OWP_Separator* s_LTSeparator = 0;

static OrgWindow* s_dmpanimWnd = 0;
static OWP_CheckBoxA* s_dmpgroupcheck = 0;
static OWP_Label* s_dmpanimLlabel = 0;
static OWP_Slider* s_dmpanimLSlider = 0;
static OWP_Label* s_dmpanimAlabel = 0;
static OWP_Slider* s_dmpanimASlider = 0;
static OWP_Button* s_dmpanimB = 0;

static OrgWindow* s_sidemenuWnd = 0;
static OWP_Separator* s_sidemenusp = 0;
static OWP_Separator* s_sidemenusp1 = 0;
static OWP_Separator* s_sidemenusp2 = 0;
static OWP_Button* s_sidemenu_rigid = 0;
static OWP_Button* s_sidemenu_limiteul = 0;
static OWP_Button* s_sidemenu_copyhistory = 0;
static OWP_Button* s_sidemenu_retarget = 0;

static OrgWindow* s_mainmenuaimbarWnd = 0;
static OWP_Label* s_mainmenulabel = 0;


static OrgWindow* s_placefolderWnd = 0;
static OWP_Label* s_placefolderlabel_1 = 0;
static OWP_Label* s_placefolderlabel_2 = 0;
static OWP_Label* s_placefolderlabel_3 = 0;

static OrgWindow* s_rigidWnd = 0;
static OWP_CheckBoxA* s_groupcheck = 0;
static OWP_Slider* s_sphrateSlider = 0;
static OWP_Slider* s_boxzSlider = 0;
static OWP_Slider* s_massSlider = 0;
//static OWP_Separator* s_massSeparator = 0;
//static OWP_Separator* s_massSeparator1 = 0;
//static OWP_Separator* s_massSeparator2 = 0;
static OWP_Button* s_massB = 0;
static OWP_Button* s_thicknessB = 0;
static OWP_Button* s_depthB = 0;
static OWP_Label* s_massspacelabel = 0;
static OWP_CheckBoxA* s_rigidskip = 0;
static OWP_Button* s_skipB = 0;
static OWP_Separator* s_forbidSeparator = 0;
static OWP_Button* s_forbidB = 0;
static OWP_CheckBoxA* s_forbidrot = 0;
static OWP_Label* s_shplabel = 0;
static OWP_Label* s_boxzlabel = 0;
static OWP_Separator* s_colSeparator = 0;
static OWP_Button* s_colB = 0;
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
static OWP_Separator* s_validSeparator = 0;
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
static OWP_Separator* s_btforceSeparator = 0;
static OWP_CheckBoxA* s_btforce = 0;
static OWP_Button* s_btforceB = 0;

static OrgWindow* s_impWnd = 0;
static OWP_CheckBoxA* s_impgroupcheck = 0;
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
static OWP_CheckBoxA* s_gpdisp = 0;
static OWP_Slider* s_grestSlider = 0;
static OWP_Label* s_grestlabel = 0;
static OWP_Slider* s_gfricSlider = 0;
static OWP_Label* s_gfriclabel = 0;

static OrgWindow* s_toolWnd = 0;		
static OWP_Separator* s_toolSeparator = 0;
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
static OWP_Button* s_toolSelectCopyFileName = 0;


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
static CModel* s_convbone_model_batch = 0;
static CModel* s_convbone_bvh = 0;
static int s_maxboneno = 0;
static CBone* s_modelbone_bone[CONVBONEMAX];
static CBone* s_bvhbone_bone[CONVBONEMAX];
static map<CBone*, CBone*> s_convbonemap;
static int s_bvhbone_cbno = 0;
static OWP_Button* s_rtgfilesave = 0;
static OWP_Button* s_rtgfileload = 0;


static OrgWindow* s_layerWnd = 0;
static OWP_LayerTable* s_owpLayerTable = 0;

static bool s_closeFlag = false;			// 終了フラグ
static bool s_closetoolFlag = false;
static bool s_closeobjFlag = false;
static bool s_closemodelFlag = false;
static bool s_closemotionFlag = false;
static bool s_closeconvboneFlag = false;
static bool s_DcloseFlag = false;
static bool s_RcloseFlag = false;
static bool s_ScloseFlag = false;
static bool s_IcloseFlag = false;
static bool s_GcloseFlag = false;
static bool s_copyFlag = false;			// コピーフラグ
static bool s_selCopyHisotryFlag = false;
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

static int s_calclimitedwmState = 0;

static bool s_EcursorFlag = false;			// カーソル移動フラグ


static bool s_timelineRUpFlag = false;
static bool s_timelinembuttonFlag = false;
static int s_mbuttoncnt = 1;
static double s_mbuttonstart = 0.0;
static bool s_timelinewheelFlag = false;
static bool s_timelineshowposFlag = false;

static bool s_prevrangeFlag = false;
static bool s_nextrangeFlag = false;

static bool s_dispmw = true;
static bool s_disptool = true;
//static bool s_dispobj = true;
static bool s_dispobj = false;
static bool s_dispmodel = false;//!!!!!!!!!!!!!!!!!
static bool s_dispmotion = false;//!!!!!!!!!!!!!!!!!
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
	//ChaVector3 localscale;//mpのmatに掛け算しておく
}CPELEM2;
static vector<CPELEM2> s_copymotvec;
static vector<CPELEM2> s_pastemotvec;
static int WriteCPTFile(WCHAR* dstfilename);
static int WriteCPIFile(WCHAR* cptfilename);
static bool LoadCPTFile();
static int LoadCPIFile(HISTORYELEM* srcdstelem);

//static int WriteTBOFile();
//static bool LoadTBOFile();


static list<KeyInfo> s_deletedKeyInfoList;	// 削除されたキー情報リスト
static list<KeyInfo> s_selectKeyInfoList;	// コピーされたキー情報リスト

static CEditRange s_editrange;
static CEditRange* s_editrangehistory = 0;
static int s_editrangehistoryno = 0;
static int s_editrangesetindex = 0;
static CEditRange s_previewrange;



//ID_RMENU_0を足して使う
#define MENUOFFSET_SETCONVBONEMODEL		(100)
#define MENUOFFSET_SETCONVBONEBVH		(MENUOFFSET_SETCONVBONEMODEL + 100)
#define MENUOFFSET_SETCONVBONE			(MENUOFFSET_SETCONVBONEBVH + 100)
#define MENUOFFSET_INITMPFROMTOOL		(MENUOFFSET_SETCONVBONE + 500)
#define MENUOFFSET_BONERCLICK			(MENUOFFSET_INITMPFROMTOOL + 100)
#define MENUOFFSET_GETSYMROOTMODE		(MENUOFFSET_BONERCLICK + 100)


#define SPAXISNUM	3
//#define SPCAMNUM	3	//Coef.h : SPR_CAM_MAX



enum {
	SPRIG_INACTIVE,
	SPRIG_ACTIVE,
	SPRIGMAX
};
//#define SPRIGMAX	2

enum {
	SPPLATEMENUKIND_GUI,
	SPPLATEMENUKIND_RIGID,
	SPPLATEMENUKIND_RETARGET,
	SPPLATEMENUKINDNUM
};
enum {
	SPGUISW_SPRITEFK,
	SPGUISW_LEFT,
	SPGUISW_LEFT2ND,
	SPGUISW_BULLETPHYSICS,
	SPGUISW_PHYSICSIK,
	SPGUISWNUM
};
//#define SPGUISWNUM	5
enum {
	SPRIGIDTSW_RIGIDPARAMS,
	SPRIGIDSW_IMPULSE,
	SPRIGIDSW_GROUNDPLANE,
	SPRIGIDSW_DAMPANIM,
	SPRIGIDSWNUM
};
//#define SPRIGIDSWNUM	4
enum {
	SPRETARGETSW_RETARGET,
	SPRETARGETSW_LIMITEULER,
	SPRETARGETSWNUM
};
//#define SPRETARGETSWNUM	2
enum {
	SPAIMBAR_1,
	SPAIMBAR_2,
	SPAIMBAR_3,
	SPAIMBAR_4,
	SPAIMBAR_5,
	SPAIMBARNUM
};
//#define SPAIMBARNUM	5
enum {
	SPMENU_FILE,
	SPMENU_DISP,
	SPMENU_MOTION,
	SPMENU_MODEL,
	SPMENU_EDIT,
	SPMENU_SELRIGID,
	SPMENU_SELRGD,
	SPMENU_SELRGDMORPH,
	SPMENU_SELIMPULSE,
	SPMENU_HELP,
	SPMENU_MAX
};



#define SPPLAYERBUTTONNUM	16

static SPAXIS s_spaxis[SPAXISNUM];
static SPCAM s_spcam[SPR_CAM_MAX];
static SPELEM s_sprig[SPRIGMAX];//inactive, active
//static SPELEM s_spbt;
static SPELEM s_spret2prev;
static SPGUISW s_spguisw[SPGUISWNUM];
static SPGUISW s_sprigidsw[SPRIGIDSWNUM];
static SPGUISW s_spretargetsw[SPRETARGETSWNUM];
static bool s_firstmoveaimbar = true;
static SPGUISW s_spaimbar[SPAIMBARNUM];
static SPGUISW s_spmenuaimbar[SPMENU_MAX];
static int s_oprigflag = 0;
static SPGUISW s_spsel3d;
static SPELEM s_spmousehere;
static SPGUISW s_spikmodesw[3];
static SPGUISW s_sprefpos;
static SPELEM s_mousecenteron;


typedef struct tag_modelpanel
{
	OrgWindow* panel;
	OWP_RadioButton* radiobutton;
	OWP_Separator* separator;
	OWP_Separator* separator2;
	vector<OWP_CheckBoxA*> checkvec;
	vector<OWP_Button*> delbutton;
	int modelindex;
}MODELPANEL;
static MODELPANEL s_modelpanel;
static bool s_firstmodelpanelpos = true;
static WindowPos s_modelpanelpos;

typedef struct tag_motionpanel
{
	OrgWindow* panel;
	OWP_RadioButton* radiobutton;
	OWP_Separator* separator;
	vector<OWP_Button*> delbutton;
	int modelindex;
}MOTIONPANEL;
static MOTIONPANEL s_motionpanel;
static bool s_firstmotionpanelpos = true;
static WindowPos s_motionpanelpos;


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
CDXUTCheckBox* s_CamTargetCheckBox = 0;
//CDXUTCheckBox* s_LightCheckBox = 0;
CDXUTCheckBox* s_ApplyEndCheckBox = 0;
//CDXUTCheckBox* s_SlerpOffCheckBox = 0;
CDXUTCheckBox* s_AbsIKCheckBox = 0;
CDXUTCheckBox* s_BoneMarkCheckBox = 0;
CDXUTCheckBox* s_RigidMarkCheckBox = 0;
//CDXUTCheckBox* s_PseudoLocalCheckBox = 0;
CDXUTCheckBox* s_WallScrapingIKCheckBox = 0;
CDXUTCheckBox* s_LimitDegCheckBox = 0;
CDXUTCheckBox* s_BrushMirrorUCheckBox = 0;
CDXUTCheckBox* s_BrushMirrorVCheckBox = 0;
CDXUTCheckBox* s_IfMirrorVDiv2CheckBox = 0;



//Left
static CDXUTControl* s_ui_lightscale = 0;
static CDXUTControl* s_ui_dispbone = 0;
static CDXUTControl* s_ui_disprigid = 0;
static CDXUTControl* s_ui_boneaxis = 0;
static CDXUTControl* s_ui_bone = 0;
static CDXUTControl* s_ui_locktosel = 0;
static CDXUTControl* s_ui_iklevel = 0;
static CDXUTControl* s_ui_editmode = 0;
static CDXUTControl* s_ui_texapplyrate = 0;
static CDXUTControl* s_ui_slapplyrate = 0;
static CDXUTControl* s_ui_motionbrush = 0;
static CDXUTControl* s_ui_texikorder = 0;
static CDXUTControl* s_ui_slikorder = 0;
//static CDXUTControl* s_ui_texikrate = 0;
//static CDXUTControl* s_ui_slikrate = 0;
static CDXUTControl* s_ui_texref = 0;
static CDXUTControl* s_ui_slirefpos = 0;
static CDXUTControl* s_ui_slirefmult = 0;
static CDXUTControl* s_ui_applytotheend = 0;
static CDXUTControl* s_ui_slerpoff = 0;
static CDXUTControl* s_ui_absikon = 0;

static CDXUTControl* s_ui_texbrushrepeats = 0;
static CDXUTControl* s_ui_brushrepeats = 0;
static CDXUTControl* s_ui_brushmirroru = 0;
static CDXUTControl* s_ui_brushmirrorv = 0;
static CDXUTControl* s_ui_ifmirrorvdiv2 = 0;


//Left 2nd
static CDXUTControl* s_ui_texthreadnum = 0;
static CDXUTControl* s_ui_slthreadnum = 0;
//static CDXUTControl* s_ui_pseudolocal = 0;
static CDXUTControl* s_ui_wallscrapingik = 0;
static CDXUTControl* s_ui_limiteul = 0;
static CDXUTControl* s_ui_texspeed = 0;
static CDXUTControl* s_ui_speed = 0;


//Bullet
static CDXUTControl* s_ui_btstart = 0;
static CDXUTControl* s_ui_btrecstart = 0;
static CDXUTControl* s_ui_stopbt = 0;
static CDXUTControl* s_ui_texbtcalccnt = 0;
static CDXUTControl* s_ui_btcalccnt = 0;
static CDXUTControl* s_ui_texerp = 0;
static CDXUTControl* s_ui_erp = 0;


//PhysicsIK
static CDXUTControl* s_ui_texphysmv = 0;
static CDXUTControl* s_ui_slphysmv = 0;
static CDXUTControl* s_ui_physrotstart = 0;
static CDXUTControl* s_ui_physmvstart = 0;
static CDXUTControl* s_ui_physikstop = 0;

//static bool s_guivisible_left = true;
//static bool s_guivisible_left2nd = true;
//static bool s_guivisible_bullet = true;
//static bool s_guivisible_physicsik = true;

static void GUIGetNextMenu(POINT ptCursor, int srcmenukind, int* dstmenukind, int* dstplateno);
static void GUIMenuSetVisible(int srcmenukind, int srcplateno);

static void GUISetVisible(int srcplateno);
static void GUISetVisible_SpriteFK();
static void GUISetVisible_Left();
static void GUISetVisible_Left2nd();
static void GUISetVisible_Bullet();
static void GUISetVisible_PhysicsIK();
static void GUISetVisible_AimBar();

static void GUIRigidSetVisible(int srcplateno);
static void ShowRigidWnd(bool srcflag);
static void ShowImpulseWnd(bool srcflag);
static void ShowGroundWnd(bool srcflag);
static void ShowDampAnimWnd(bool srcflag);

static void GUIRetargetSetVisible(int srcplateno);
static void ShowRetargetWnd(bool srcflag);
static void ShowLimitEulerWnd(bool srcflag);


//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

bool g_4kresolution = false;

ChaVector3 g_vCenter( 0.0f, 0.0f, 0.0f );


std::vector<void*> g_eulpool;//allocate EULPOOLBLKLEN EulKey at onse and pool 
std::vector<void*> g_keypool;//allocate KEYPOOLBLKLEN Key at onse and pool 

void OnDSUpdate();
static void OnDSMouseHereApeal();
static void OnArrowKey();//DS関数でキーボードの矢印キーに対応


static void CalcTotalBound();


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
//ID3DX11Font*                  g_pFont = NULL;         // Font for drawing text
//ID3DX11Sprite*                g_pSprite = NULL;       // Sprite for batching draw text calls
CDXUTTextHelper*              g_pTxtHelper = NULL;

bool                        g_bShowHelp = true;     // If true, it renders the UI control text
CModelViewerCamera*          g_Camera = 0;// A model viewing camera
//ID3DX11Effect*                g_pEffect = NULL;       // D3DX effect interface
//ID3DXMesh*                  g_pMesh = NULL;         // Mesh object

//IDirect3DTexture10*          g_pMeshTexture = NULL;  // Mesh texture
//LPD3DXFONT                  g_pFont9 = NULL;         // Font for drawing text


CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
//CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D UI
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
bool                        g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader
//ChaMatrix               g_mCenterWorld;
ChaMatrix               g_mCenterWorld;


#define MAX_LIGHTS 3
CDXUTDirectionWidget g_LightControl[MAX_LIGHTS];

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define ID_RMENU_PHYSICSCONSTRAINT	(ID_RMENU_0 - 100)
#define ID_RMENU_MASS0			(ID_RMENU_PHYSICSCONSTRAINT + 1)
#define ID_RMENU_EXCLUDE_MV		(ID_RMENU_PHYSICSCONSTRAINT + 2)
#define ID_RMENU_FORBIDROT_ONE	(ID_RMENU_PHYSICSCONSTRAINT + 3)
#define ID_RMENU_ENABLEROT_ONE	(ID_RMENU_PHYSICSCONSTRAINT + 4)
#define ID_RMENU_FORBIDROT_CHILDREN	(ID_RMENU_PHYSICSCONSTRAINT + 5)
#define ID_RMENU_ENABLEROT_CHILDREN	(ID_RMENU_PHYSICSCONSTRAINT + 6)

#define ID_RMENU_MASS0_ON_ALL		(ID_RMENU_PHYSICSCONSTRAINT + 7)
#define ID_RMENU_MASS0_OFF_ALL		(ID_RMENU_PHYSICSCONSTRAINT + 8)
#define ID_RMENU_MASS0_ON_UPPER		(ID_RMENU_PHYSICSCONSTRAINT + 9)
#define ID_RMENU_MASS0_OFF_UPPER	(ID_RMENU_PHYSICSCONSTRAINT + 10)
#define ID_RMENU_MASS0_ON_LOWER		(ID_RMENU_PHYSICSCONSTRAINT + 11)
#define ID_RMENU_MASS0_OFF_LOWER	(ID_RMENU_PHYSICSCONSTRAINT + 12)

#define ID_RMENU_KINEMATIC_ON_LOWER	(ID_RMENU_PHYSICSCONSTRAINT + 13)
#define ID_RMENU_KINEMATIC_OFF_LOWER	(ID_RMENU_PHYSICSCONSTRAINT + 14)

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

#define IDC_COMBO_MOTIONBRUSH_METHOD	63
#define IDC_RMARK					64

#define IDC_PHYSICS_IK_STOP			65
#define IDC_BTRECSTART				66

#define IDC_STATIC_BRUSHREPEATS		67
#define IDC_SL_BRUSHREPEATS			68
#define IDC_BRUSH_MIRROR_U			69
#define IDC_BRUSH_MIRROR_V			70
#define IDC_BRUSH_MIRROR_V_DIV2		71

#define IDC_IK_SC					72

#define IDC_SL_REFPOS				73
#define IDC_SL_REFARROW				74
#define IDC_STATIC_REF				75
#define IDC_SL_REFMULT				76

#define IDC_WALLSCRAPINGIK			77



LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int InitializeMainWindow(CREATESTRUCT* createWindowArgs);
static HWND CreateMainWindow();
static HWND Create3DWnd();
static CInfoWindow* CreateInfoWnd();

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
//bool CALLBACK IsDeviceAcceptable( D3DCAPS10* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
//                                  void* pUserContext );
//bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
//HRESULT CALLBACK OnCreateDevice( ID3D11Device* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//                                 void* pUserContext );
//HRESULT CALLBACK OnResetDevice( ID3D11Device* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//                                void* pUserContext );
//void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
//void CALLBACK OnFrameRender( ID3D11Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
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
//bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
//	bool bWindowed, void* pUserContext);
//HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice,
//	const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
//HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
//	void* pUserContext);
//void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
//	void* pUserContext);
//void CALLBACK OnD3D9LostDevice(void* pUserContext);
//void CALLBACK OnD3D9DestroyDevice(void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

//bool CALLBACK IsD3D11DeviceAcceptable(UINT Adapter, UINT Output, D3D11_DRIVER_TYPE DeviceType,
//	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
//HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
//	void* pUserContext);
//HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
//	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
//void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
//void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
//void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);


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
LRESULT CALLBACK AboutDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK bvh2FbxBatchDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK RetargetBatchDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ProgressDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);


void InitApp();
int CheckResolution();

//HRESULT LoadMesh( ID3D11Device* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void RenderText(double fTime );
static int RetargetFile(char* fbxpath);

static int OnMouseMoveFunc();

static void OnUserFrameMove(double fTime, float fElapsedTime);
static int RollbackCurBoneNo();
static void PrepairUndo();
static int OnFrameUndo(bool fromds, int fromdskind);
static void OnGUIEventSpeed();
static int SetShowPosTime();

static void AutoCameraTarget();

static int CreateUtDialog();
static int CreateTimelineWnd();
static int CreateLongTimelineWnd();
static int CreateDmpAnimWnd();
static int CreateRigidWnd();
static int CreateSideMenuWnd();
static int CreateMainMenuAimBarWnd();
static int CreateImpulseWnd();
static int CreateGPlaneWnd();
static int CreateToolWnd();
static int CreateLayerWnd();
static int CreatePlaceFolderWnd();

static int OnFrameAngleLimit();
static int OnFrameKeyboard();
static int OnFrameUtCheckBox();
static int OnFramePreviewStop();
static int OnFramePreviewNormal(double* pnextframe, double* pdifftime);
static int OnFramePreviewBt(double* pnextframe, double* pdifftime);
int OnFramePreviewBtAftFunc(double nextframe, CModel* curmodel);
static int OnFramePreviewRagdoll(double* pnextframe, double* pdifftime);
static int OnFrameCloseFlag();
static int OnFrameTimeLineWnd();
static int OnFrameMouseButton();
static int OnFrameToolWnd();
static int OnFramePlayButton();
static int OnFrameStartPreview(double curtime, double* psavetime);
static int OnFrameBatchThread();
//static int OnFrame();
static int OnFrameUpdateGround();
static int OnFrameInitBtWorld();
static int ToggleRig();
static void UpdateBtSimu(double nextframe, CModel* curmodel);
static void SetKinematicToHand(CModel* srcmodel, bool srcflag);
static void SetKinematicToHandReq(CModel* srcmodel, CBone* srcbone, bool srcflag);
static void DispProgressCalcLimitedWM();


static int OnRenderSetShaderConst();
static int OnRenderModel(ID3D11DeviceContext* pd3dImmediateContext);
static int OnRenderRefPose(ID3D11DeviceContext* pd3dImmediateContext, CModel* curmodel);
static int OnRenderGround(ID3D11DeviceContext* pd3dImmediateContext);
static int OnRenderBoneMark(ID3D11DeviceContext* pd3dImmediateContext);
static int OnRenderSelect(ID3D11DeviceContext* pd3dImmediateContext);
static int OnRenderSprite(ID3D11DeviceContext* pd3dImmediateContext);
static int OnRenderUtDialog(ID3D11DeviceContext* pd3dImmediateContext, float fElapsedTime);

static int PasteMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe);
static int PasteNotMvParMotionPoint(CBone* srcbone, CMotionPoint srcmp, double newframe);
static int PasteMotionPointJustInTerm(double copyStartTime, double copyEndTime, double startframe, double endframe);
static int PasteMotionPointAfterCopyEnd(double copyStartTime, double copyEndTime, double startframe, double endframe);

static int ChangeCurrentBone();

static int InitCurMotion(int selectflag, double expandmotion);

static int OpenChaFile();
CModel* OpenMQOFile();
CModel* OpenFBXFile( bool dorefreshtl, int skipdefref, int inittimelineflag );
static int OpenREFile();
static int OpenImpFile();
static int OpenGcoFile();
static int OpenMNLFile();

int OnDelMotion( int delindex, bool ondelbutton = false );
int OnAddMotion( int srcmotid, bool dorefreshtl);

static int StartBt( CModel* curmodel, BOOL isfirstmodel, int flag, int btcntzero );
static int StopBt();
static int GetShaderHandle();
static int SetBaseDir();
static int LoadIniFile();
static int SaveIniFile();


static int OpenFile();
static int BVH2FBX();
static void FindF(std::vector<wstring>& out, const wstring& directory, const wstring& findext);
static int BVH2FBXBatch();
static int RetargetBatch();
static int CalcLimitedWorldMat();
static int SaveBatchHistory(WCHAR* selectname);
static int GetBatchHistoryDir(WCHAR* dstname, int dstlen);
static int Savebvh2FBXHistory(WCHAR* selectname);
static int GetbvhHistoryDir(std::vector<wstring>& dstvecopenfilename);
static int GetCPTFileName(std::vector<HISTORYELEM>& dstcptfilename);
static int SaveProject();
static int SaveREFile();
static int SaveImpFile();
static int SaveGcoFile();
static int ExportFBXFile();

static void refreshTimeline( OWP_Timeline& timeline ); 
static int refreshEulerGraph();
static int AddBoneTra( int kind, float srctra );
static int AddBoneTra2( ChaVector3 diffvec );
//static int AddBoneTraPhysics(ChaVector3 diffvec);

static int AddBoneScale(int kind, float srctra);
static int AddBoneScale2(ChaVector3 diffvec);

static int DispMotionWindow();
static int DispToolWindow();
static int DispObjPanel();
static int DispModelPanel();
static int DispMotionPanel();
//static int DispConvBoneWindow();
static int DispAngleLimitDlg();
static int DispRotAxisDlg();
static int DispCustomRigDlg(int rigno);
static int InvalidateCustomRig(int rigno);
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
static int Bone2AngleLimit(int setcheckflag);
static int AngleLimit2Bone();
static int AngleLimit2Dlg(HWND hDlgWnd);
static int InitAngleLimitSlider(HWND hDlgWnd, int slresid, int txtresid, int srclimit);
static int GetAngleLimitSliderVal(HWND hDlgWnd, int slresid, int txtresid, int* dstptr);
static int InitAngleLimitSliderFloat(HWND hDlgWnd, int slresid, int txtresid, float srclimit);
static int GetAngleLimitSliderValFloat(HWND hDlgWnd, int slresid, int txtresid, float* dstptr);

static int InitRotAxis();
static int RotAxis(HWND hDlgWnd);

static int EraseKeyList();
static int DestroyTimeLine( int dellist );
static int AddTimeLine( int newmotid, bool dorefreshtl );
static int AddMotion( WCHAR* wfilename, double srcleng = 0.0 );
static int OnAnimMenu( bool dorefreshflag, int selindex, int saveundoflag = 1 );
static int OnRgdMorphMenu( int selindex );
static int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb );
static int OnSetMotSpeed();

static int OnModelMenu( bool dorefreshtl, int selindex, int callbymenu );
static int SetTimelineHasRigFlag();
static int OnREMenu( int selindex, int callbymenu );
static int OnRgdMenu( int selindex, int callbymenu );
static int OnImpMenu( int selindex );
static int OnDelModel( int delindex, bool ondelbutton = false );
static int OnDelAllModel();
static int refreshModelPanel();
//static int refreshMotionPanel();
static int RenderSelectMark(ID3D11DeviceContext* pd3dImmediateContext, int renderflag);
static int RenderSelectFunc(ID3D11DeviceContext* pd3dImmediateContext);
static int RenderSelectPostureFunc(ID3D11DeviceContext* pd3dImmediateContext);
static int RenderRigMarkFunc(ID3D11DeviceContext* pd3dImmediateContext);
//static int SetSelectState(ID3D11DeviceContext* pd3dImmediateContext);


static int CreateModelPanel();
static int DestroyModelPanel();
static int CreateMotionPanel();
static int DestroyMotionPanel();
static int CreateConvBoneWnd();
static int DestroyConvBoneWnd();
static int SetConvBoneModel();
static int SetConvBoneBvh();
static int SetConvBone( int cbno );
static int SaveRetargetFile();
static int LoadRetargetFile(WCHAR* srcfilename);
static int SaveMotionNameListFile();
//static int LoadMotionNameListFile(WCHAR* srcfilename);
static int SetJointPair2ConvBoneWnd();


//static int ConvBoneConvert();//--> RetargetMotion()に改名
static int RetargetMotion();

//MameBake3DLibのRetarget.h, Retarget.cppへ移動
//static void ConvBoneConvertReq(CBone* modelbone, double srcframe, CBone* befbvhbone, float hrate);
//static int ConvBoneRotation(int selfflag, CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate);


static int CalcTargetPos( ChaVector3* dstpos );
static int CalcPickRay( ChaVector3* start3d, ChaVector3* end3d );
static void ActivatePanel( int state );
static int SetCamera6Angle();

//static int SetSelectCol();
static int RigidElem2WndParam();
static int SetRigidLeng();
static int SetImpWndParams();
static int SetGpWndParams();
static int SetDmpWndParams();

static int SetSpSel3DParams();
static int SetSpAimBarParams();
static int SetSpMenuAimBarParams();
static int SetSpAxisParams();
static int SetSpMouseCenterParams();
static int PickSpAxis( POINT srcpos );
static int SetSpGUISWParams();
static int PickSpGUISW(POINT srcpos);
static int SetSpRigidSWParams();
static int PickSpRigidSW(POINT srcpos);
static int PickSpRetargetSW(POINT srcpos);
static int SetSpRetargetSWParams();
static int SetSpCamParams();
static int PickSpCam(POINT srcpos);
static int SetSpRigParams();
static int PickSpRig(POINT srcpos);
static int PickRigBone(PICKINFO* ppickinfo);
static ChaMatrix CalcRigMat(CBone* curbone, int curmotid, double curframe, int dispaxis, int disporder, bool posinverse);


//static int SetSpBtParams();
//static int PickSpBt(POINT srcpos);
static int SetSpMouseHereParams();
static int SetSpIkModeSWParams();
static int PickSpIkModeSW(POINT srcpos);
static int SetSpRefPosSWParams();
static int PickSpRefPosSW(POINT srcpos);



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
static int CreateMotionBrush(double srcstart, double srcend, bool onrefresh);


static int ExportBntFile();

static const int s_appindex = 1;

static void AboutMotionBrush();
static int s_registflag = 1;//!!!!!!!!!!!
static HKEY s_hkey;
static int RegistKey();
static int IsRegist();

static int TimelineCursorToMotion();
int OnTimeLineCursor(int mbuttonflag, double newframe);
static int OnTimeLineButtonSelectFromSelectStartEnd(int tothelastflag);
static int OnTimeLineSelectFromSelectedKey();
static int OnTimeLineMButtonDown(bool ctrlshiftflag);
static int OnTimeLineWheel();
static int AddEditRangeHistory();
static int RollBackEditRange(int prevrangeFlag, int nextrangeFlag);
static int RecalcBoneAxisX(CBone* srcbone);
static void RecalcAxisX_All();

static int GetSymRootMode();

static int UpdateEditedEuler();


static std::wstring ReplaceString
(
	std::wstring String1  // 置き換え対象
	, std::wstring String2  // 検索対象
	, std::wstring String3  // 置き換える内容
);


std::wstring ReplaceString
(
	std::wstring String1  // 置き換え対象
	, std::wstring String2  // 検索対象
	, std::wstring String3  // 置き換える内容
)
{
	std::wstring::size_type  Pos(String1.find(String2));

	while (Pos != std::string::npos)
	{
		String1.replace(Pos, String2.length(), String3);
		Pos = String1.find(String2, Pos + String3.length());
	}

	return String1;
}


LRESULT CALLBACK AboutDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			return FALSE;
		}
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
		break;
	}
	return TRUE;
}


void AboutMotionBrush()
{
	int dlgret;
	dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1),
		s_3dwnd, (DLGPROC)AboutDlgProc);

}



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
//INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
//INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd )//SAL付き
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);


	//_CrtSetBreakAlloc(297);
	//_CrtSetBreakAlloc(303);
	//_CrtSetBreakAlloc(307);

//_CrtSetBreakAlloc(10309);
//_CrtSetBreakAlloc(10833);

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
	
	LoadIniFile();


	//s_appcntのセット。CheckResolution()よりも前
	s_appcnt = 0;
	s_launchbyc4 = 0;
	int    i;
	int    nArgs;
	//WCHAR  szBuf[256];
	LPWSTR* lplpszArgs;
	lplpszArgs = CommandLineToArgvW(GetCommandLine(), &nArgs);
	for (i = 0; i < nArgs; i++) {
		//wsprintf(szBuf, TEXT("%d番目の引数"), i + 1);
		//MessageBox(NULL, lplpszArgs[i], szBuf, MB_OK);
		if (wcscmp(lplpszArgs[i], L"-progno") == 0) {
			s_launchbyc4 = 1;
			if ((i + 1) < nArgs) {
				i++;
				WCHAR strprogno[MAX_PATH] = { 0L };
				wcscpy_s(strprogno, MAX_PATH, lplpszArgs[i]);
				s_appcnt = _wtoi(strprogno);
			}
		}else if(wcscmp(lplpszArgs[i], L"-diffx") == 0) {
			if ((i + 1) < nArgs) {
				i++;
				WCHAR strdiffx[MAX_PATH] = { 0L };
				wcscpy_s(strdiffx, MAX_PATH, lplpszArgs[i]);
				s_launchc4diffx = _wtoi(strdiffx);
			}
		}else if (wcscmp(lplpszArgs[i], L"-diffy") == 0) {
			if ((i + 1) < nArgs) {
				i++;
				WCHAR strdiffy[MAX_PATH] = { 0L };
				wcscpy_s(strdiffy, MAX_PATH, lplpszArgs[i]);
				s_launchc4diffy = _wtoi(strdiffy);
			}
		}
}

	LocalFree(lplpszArgs);


	OpenDbgFile(s_appcnt);


	//2K TV or 4K TV. Create*Window()よりも前
	int chkresult = CheckResolution();
	if (chkresult != 0) {//大小選択ダイアログでキャンセルボタンを押した場合はアプリ終了
		return 0;
	}



	s_copyKeyInfoList.clear();	// コピーされたキー情報リスト
	s_copymotvec.clear();
	s_pastemotvec.clear();
	s_deletedKeyInfoList.clear();	// 削除されたキー情報リスト
	s_selectKeyInfoList.clear();	// コピーされたキー情報リスト


	//DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);

	//DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	//DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
	//DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
	//DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender);
	//DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
	//DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);
	//DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	//DXUTSetCallbackD3D9DeviceCreated(NULL);
	//DXUTSetCallbackD3D9DeviceReset(NULL);
	//DXUTSetCallbackD3D9FrameRender(NULL);
	//DXUTSetCallbackD3D9DeviceLost(NULL);
	//DXUTSetCallbackD3D9DeviceDestroyed(NULL);


	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);


	s_mainhwnd = CreateMainWindow();
	if (s_mainhwnd == NULL) {
		_ASSERT(0);
		return 1;
	}

	::SetPriorityClass(GetModuleHandle(NULL), REALTIME_PRIORITY_CLASS);
	::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	//#############################
	//SetTreadAffinityはヒントでしかない
	//#############################
	//SYSTEM_INFO sys;
	//::GetSystemInfo(&sys);
	//DWORD cpunum = sys.dwNumberOfProcessors;
	//DWORD_PTR affinitymask = 0;
	//DWORD cpucnt;
	////for (cpucnt = 0; cpucnt < cpunum; cpucnt++) {
	////affinitymask |= (DWORD_PTR)(1 << (cpunum - 1));
	////}
	//DWORD_PTR befaffinity = ::SetThreadAffinityMask(GetCurrentThread(), affinitymask);
	//--------------------------------------------------------------------------------------
	// Limit the current thread to one processor (the current one). This ensures that timing code 
	// runs on only one processor, and will not suffer any ill effects from power management.
	// See "Game Timing and Multicore Processors" for more details
	//--------------------------------------------------------------------------------------
	//	void CDXUTTimer::LimitThreadAffinityToCurrentProc()



	s_doneinit = 0;
    InitApp();
	if (s_doneinit != 1){
		_ASSERT(0);
		return 1;
	}

	//IsRegist();





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
	//s_3dwnd = DXUTGetHWND();
	//_ASSERT(s_3dwnd);
	//ShowWindow(s_3dwnd, SW_SHOW);
	//SetWindowPos(s_3dwnd, HWND_TOP, 450, 0, s_mainwidth, s_mainheight, SWP_NOSIZE);


	//InitApp();
	//DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	//DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
	//DXUTCreateWindow(L"BasicHLSL10");
	//DXUTCreateDevice(true, 640, 480);
	//DXUTMainLoop(); // Enter into the DXUT render loop

	//s_mainhwnd = CreateMainWindow();
	//if (s_mainhwnd == NULL) {
	//	_ASSERT(0);
	//	return 1;
	//}


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

	g_Camera = new CModelViewerCamera();
	if (!g_Camera) {
		_ASSERT(0);
		return 1;
	}


	//DXUTSetOverrideSize(s_mainwidth, s_mainheight);//mac + VM Fusionの場合はここを実行する

	//GetDXUTState().SetOverrideWidth(s_mainwidth);
	//GetDXUTState().SetOverrideHeight(s_mainwidth);

/*
#define SPAXISNUM	3
//#define SPCAMNUM	3	//Coef.h : SPR_CAM_MAX
#define SPRIGMAX	2
#define SPGUISWNUM	5
#define SPRIGIDSWNUM	4
#define SPRETARGETSWNUM	2
#define SPAIMBARNUM	5
*/
	CreateUtDialog();
	int spgno;
	for (spgno = 0; spgno < SPGUISWNUM; spgno++) {
		GUISetVisible(spgno + 2);
	}



	if (!Create3DWnd()) {
		_ASSERT(0);
		return 1;
	}

	CreatePlaceFolderWnd();
	CreateTimelineWnd();
	CreateToolWnd();
	CreateLongTimelineWnd();
	CreateDmpAnimWnd();
	CreateRigidWnd();
	CreateImpulseWnd();
	CreateGPlaneWnd();
	CreateLayerWnd();
	CreateInfoWnd();
	CreateSideMenuWnd();
	CreateMainMenuAimBarWnd();	

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

	
	GUIMenuSetVisible(s_platemenukind, s_platemenuno);

	
	//if (!s_eventhook) {
	HRESULT hr1 = CoInitialize(NULL);
	if (FAILED(hr1)) {
		//すでに初期化済なだけでエラーリターンするのでそのまま続行する
		//_ASSERT(0);
		//return 1;
	}
	//s_eventhook = SetWinEventHook(
	//	//EVENT_SYSTEM_DIALOGSTART,
	//	//EVENT_SYSTEM_DIALOGSTART,
	//	//EVENT_SYSTEM_DIALOGEND,
	//	EVENT_MIN,
	//	EVENT_MAX,
	//	NULL,
	//	WinEventProc,
	//	//GetDlgItemInt(IDC_PROCESSID),
	//	//GetProcessId(GetModuleHandle(NULL)),
	//	//GetThreadId(g_hUnderTrackingThread),
	//	//GetThreadId(s_messageboxthread),
	//	0,
	//	0,
	//	WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
	//);
	//IntPtr hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, IntPtr.Zero,
	//	procDelegate, 0, 0, WINEVENT_OUTOFCONTEXT);

	//// MessageBox provides the necessary mesage loop that SetWinEventHook requires.
	//MessageBox.Show("Tracking focus, close message box to exit.");

	//UnhookWinEvent(hhook);

	//}


	//s_iktimerid = (int)::SetTimer(s_mainhwnd, s_iktimerid, 16, NULL);


	//if (s_mainhwnd) {
	//	SetCapture(s_mainhwnd);
	//}

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
int CheckResolution()
{
	g_4kresolution = false;

/*
//基準とする大きさ　mainwindowの大きさ
(1216 + 450) * 2, (950 - MAINMENUAIMBARH) * 2
*/

	if ((s_appcnt == 0) && (s_launchbyc4 == 0)) {

		HWND desktopwnd;
		desktopwnd = ::GetDesktopWindow();
		if (desktopwnd) {
			RECT desktoprect;
			::GetClientRect(desktopwnd, &desktoprect);
			if ((desktoprect.right >= (s_totalwndwidth * 2)) && (desktoprect.bottom >= ((s_totalwndheight - MAINMENUAIMBARH) * 2))) {

				CSelectLSDlg dlg;
				int dlgret = (int)dlg.DoModal();
				if (dlgret != IDOK) {
					return 1;//キャンセルボタンはアプリ終了
				}
				BOOL selectL = dlg.GetIsLarge();//4K可能の場合には大小を選択可能
				if (selectL == TRUE) {
					g_4kresolution = true;//!!!!!!!!!!!!!!!!


					s_totalwndwidth = (1216 + 450) * 2;
					s_totalwndheight = (950 - MAINMENUAIMBARH) * 2;
					s_2ndposy = 600 * 2;

					s_toolwidth = 230 * 2 - 60;
					//s_toolheight = 290 * 2;
					//s_toolheight = (s_totalwndheight - s_2ndposy - MAINMENUAIMBARH - 18) * 2;
					s_toolheight = s_totalwndheight - s_2ndposy - (MAINMENUAIMBARH + 18) * 2 + MAINMENUAIMBARH + 8;

					s_mainwidth = 800 * 2 + 340 + 450 - 64 + 60;
					s_mainheight = (520 * 2 - MAINMENUAIMBARH);

					//s_bufwidth = (800 * 2);
					s_bufwidth = 800 * 2 + 340 + 450 - 64 + 60;
					s_bufheight = (520 * 2 - MAINMENUAIMBARH);


					//s_timelinewidth = 400 * 2;
					s_timelinewidth = s_toolwidth;
					s_timelineheight = s_2ndposy - MAINMENUAIMBARH;

					//s_longtimelinewidth = 970 * 2;
					s_longtimelinewidth = s_mainwidth;
					//s_longtimelineheight = (s_totalwndheight - s_2ndposy - MAINMENUAIMBARH - 18) * 2;
					s_longtimelineheight = s_totalwndheight - s_2ndposy - (MAINMENUAIMBARH + 18) * 2 + MAINMENUAIMBARH + 8;

					s_infowinwidth = s_mainwidth;
					s_infowinheight = (s_2ndposy - s_mainheight - MAINMENUAIMBARH);

					//s_sidemenuwidth = 450 * 2 + 16;
					s_sidemenuwidth = 450 + 16 + 64 - 4;
					s_sidemenuheight = MAINMENUAIMBARH;

					s_sidewidth = s_sidemenuwidth;
					//s_sideheight = (s_totalwndheight - MAINMENUAIMBARH - s_sidemenuheight - 28) * 2 + MAINMENUAIMBARH;
					s_sideheight = s_totalwndheight - s_sidemenuheight - 28 * 2 - 4;


					//s_guibarX0 = s_mainwidth / 2 - 180 - 2 * 180 - 30;
					s_guibarX0 = s_mainwidth / 2 - 130 * 2;

				}
				else {
					g_4kresolution = false;
				}			
			}
		}
	}


	if (!g_4kresolution) {

		s_totalwndwidth = (1216 + 450);
		s_totalwndheight = 950;
		s_2ndposy = 600;

		s_toolwidth = 230;
		//s_toolheight = 290;
		s_toolheight = s_totalwndheight - s_2ndposy - MAINMENUAIMBARH - 28;

		s_mainwidth = 800 - 64;
		s_mainheight = (520 - MAINMENUAIMBARH);

		s_bufwidth = 800 - 64;
		s_bufheight = (520 - MAINMENUAIMBARH);


		s_timelinewidth = 400;
		s_timelineheight = s_2ndposy - MAINMENUAIMBARH;

		s_longtimelinewidth = 970 - 64;
		s_longtimelineheight = s_toolheight;


		s_infowinwidth = s_mainwidth;
		s_infowinheight = (s_2ndposy - s_mainheight - MAINMENUAIMBARH);

		s_sidemenuwidth = 450 + 64 - 4;
		s_sidemenuheight = MAINMENUAIMBARH;

		s_sidewidth = s_sidemenuwidth;
		s_sideheight = s_totalwndheight - MAINMENUAIMBARH - s_sidemenuheight - 28;

		s_guibarX0 = 120;

	}


	return 0;
}


void InitApp()
{
	InitializeCriticalSection(&s_CritSection_LTimeline);
	InitializeCriticalSection(&g_CritSection_GetGP);

	InitCommonControls();


	//////check
	//WCHAR strchk[256] = { 0L };
	//swprintf_s(strchk, 256, L"NULL == %p\nINVALID_HANDLE_VALUE == %p", NULL, INVALID_HANDLE_VALUE);
	//::MessageBox(NULL, strchk, L"check", MB_OK);


	s_totalmb.center = ChaVector3(0.0f, 0.0f, 0.0f);
	s_totalmb.max = ChaVector3(5.0f, 5.0f, 5.0f);
	s_totalmb.min = ChaVector3(-5.0f, -5.0f, -5.0f);
	s_totalmb.r = (float)ChaVector3LengthDbl(&s_totalmb.max);


	g_wallscrapingikflag = 0;

	s_ikkind = 0;
	

	s_beflimitdegflag = g_limitdegflag;
	s_savelimitdegflag = g_limitdegflag;



	s_progressnum = 0;
	s_progresscnt = 0;
	s_progressmodelnum = 0;
	s_progressmodelcnt = 0;
	s_befprogressnum = 0;
	s_befprogresscnt = 0;
	s_befprogressmodelnum = 0;
	s_befprogressmodelcnt = 0;


	//g_ClearColorIndex = 0;//inifileで読み込み
	//g_ClearColor[BGCOL_MAX][4] = {
	//	{0.0f, 0.0f, 0.0f, 1.0f},
	//	{1.0f, 1.0f, 1.0f, 1.0f},
	//	{0.0f, 0.0f, 1.0f, 1.0f},
	//	{0.0f, 0.5f, 0.25f, 1.0f},
	//	{1.0f, 0.5f, 0.5f, 1.0f}
	//};
	g_ClearColor[BGCOL_BLACK][0] = 0.0f;
	g_ClearColor[BGCOL_BLACK][1] = 0.0f;
	g_ClearColor[BGCOL_BLACK][2] = 0.0f;
	g_ClearColor[BGCOL_BLACK][3] = 1.0f;
	g_ClearColor[BGCOL_WHITE][0] = 1.0f;
	g_ClearColor[BGCOL_WHITE][1] = 1.0f;
	g_ClearColor[BGCOL_WHITE][2] = 1.0f;
	g_ClearColor[BGCOL_WHITE][3] = 1.0f;
	g_ClearColor[BGCOL_BLUE][0] = 0.0f;
	g_ClearColor[BGCOL_BLUE][1] = 0.0f;
	g_ClearColor[BGCOL_BLUE][2] = 1.0f;
	g_ClearColor[BGCOL_BLUE][3] = 1.0f;
	g_ClearColor[BGCOL_GREEN][0] = 0.0f;
	g_ClearColor[BGCOL_GREEN][1] = 0.5f;
	g_ClearColor[BGCOL_GREEN][2] = 0.25f;
	g_ClearColor[BGCOL_GREEN][3] = 1.0f;
	g_ClearColor[BGCOL_RED][0] = 1.0f;
	g_ClearColor[BGCOL_RED][1] = 0.5f;
	g_ClearColor[BGCOL_RED][2] = 0.5f;
	g_ClearColor[BGCOL_RED][3] = 1.0f;
	g_ClearColor[BGCOL_GRAY][0] = 0.5f;
	g_ClearColor[BGCOL_GRAY][1] = 0.5f;
	g_ClearColor[BGCOL_GRAY][2] = 0.5f;
	g_ClearColor[BGCOL_GRAY][3] = 1.0f;



	s_firstmodelpanelpos = true;
	s_modelpanelpos = WindowPos(0, 0);;
	s_firstmotionpanelpos = true;
	s_motionpanelpos = WindowPos(0, 0);;



	s_opedelmodelcnt = -1;
	s_opedelmotioncnt = -1;
	s_underdelmodel = false;
	s_underdelmotion = false;
	s_opeselectmodelcnt = -1;
	s_opeselectmotioncnt = -1;
	s_underselectmodel = false;
	s_underselectmotion = false;

	s_underanglelimithscroll = 0;

	s_dispanglelimit = false;
	s_dispsampleui = true;
	s_dispmw = true;
	s_disptool = true;
	s_dispobj = false;
	s_dispmodel = false;//!!!!!!!!!!!!!!!!! modelpanelのdispflag
	s_dispmotion = false;//!!!!!!!!!!!!!!!! motionpanelのdispflag
	s_dispground = true;
	s_dispselect = true;
	//s_displightarrow = true;
	s_dispconvbone = false;

	s_oprigflag = 0;

	s_tkeyflag = 0;//bone twist
	s_closeFlag = false;			// 終了フラグ
	s_closetoolFlag = false;
	s_closeobjFlag = false;
	s_closemodelFlag = false;
	s_closemotionFlag = false;
	s_closeconvboneFlag = false;
	s_DcloseFlag = false;
	s_RcloseFlag = false;
	s_ScloseFlag = false;
	s_IcloseFlag = false;
	s_GcloseFlag = false;
	s_copyFlag = false;			// コピーフラグ
	s_selCopyHisotryFlag = false;
	s_symcopyFlag = false;
	s_undersymcopyFlag = false;
	s_cutFlag = false;			// カットフラグ
	s_pasteFlag = false;			// ペーストフラグ
	s_cursorFlag = false;			// カーソル移動フラグ
	s_selectFlag = false;			// キー選択フラグ
	s_keyShiftFlag = false;		// キー移動フラグ
	s_deleteFlag = false;		// キー削除フラグ
	s_motpropFlag = false;
	s_markFlag = false;
	s_selboneFlag = false;
	s_initmpFlag = false;
	s_filterFlag = false;
	s_interpolateFlag = false;
	s_firstkeyFlag = false;
	s_lastkeyFlag = false;
	s_btresetFlag = false;
	s_LcloseFlag = false;
	s_LnextkeyFlag = false;
	s_LbefkeyFlag = false;
	s_LcursorFlag = false;			// カーソル移動フラグ
	s_LstartFlag = false;
	s_LstopFlag = false;
	s_EcursorFlag = false;			// カーソル移動フラグ
	s_timelineRUpFlag = false;
	s_timelinembuttonFlag = false;
	s_mbuttoncnt = 1;
	s_mbuttonstart = 0.0;
	s_timelinewheelFlag = false;
	s_timelineshowposFlag = false;
	s_prevrangeFlag = false;
	s_nextrangeFlag = false;

	s_calclimitedwmState = 0;

	s_temppath[0] = 0L;
	::GetTempPathW(MAX_PATH, s_temppath);
	_ASSERT(s_temppath[0]);
	s_cptfilename.clear();
	GetCPTFileName(s_cptfilename);//s_temppathセットより後。初回。


	InitDSValues();

	s_onselectplugin = 0;
	s_plugin = 0;

	s_onefps = 0;

	s_rectime = 0.0;
	s_reccnt = 0;
	g_btsimurecflag = false;

	s_bvh2fbxhandle1 = INVALID_HANDLE_VALUE;
	s_bvh2fbxhandle2 = INVALID_HANDLE_VALUE;
	s_bvh2fbxnum = 0;
	s_bvh2fbxcnt = 0;
	s_befbvh2fbxnum = 0;
	s_befbvh2fbxcnt = 0;
	g_bvh2fbxbatchflag = 0;
	s_bvh2fbxbatchwnd = 0;
	s_bvh2fbxout.clear();

	s_saveretargetmodel = 0;
	s_convbone_model_batch_selindex = 0;
	s_retargethandle1 = INVALID_HANDLE_VALUE;
	s_retargethandle2 = INVALID_HANDLE_VALUE;
	s_retargetnum = 0;
	s_retargetcnt = 0;
	s_befretargetnum = 0;
	s_befretargetcnt = 0;
	g_retargetbatchflag = 0;
	s_retargetbatchwnd = 0;
	s_retargetout.clear();

	g_calclimitedwmflag = 0;

	g_mousehereimage = 0;
	g_menuaimbarimage = 0;
	g_currentsubmenuid = 0;
	g_currentsubmenupos.x = 0;
	g_currentsubmenupos.y = 0;

	g_submenuwidth = 32;

	g_filterdlghwnd = 0;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	s_cursubmenu = 0;

	g_dsmousewait = 0;
	g_mouseherebmp = 0;
	g_tranbmp = 0;

	s_sampleuihwnd = 0;
	s_nowloading = true;

	s_getsym_retmode = 0;

	s_rcmainwnd.top = 0;
	s_rcmainwnd.left = 0;
	s_rcmainwnd.bottom = 0;
	s_rcmainwnd.right = 0;
	s_rc3dwnd.top = 0;
	s_rc3dwnd.left = 0;
	s_rc3dwnd.bottom = 0;
	s_rc3dwnd.right = 0;
	s_rctreewnd.top = 0;
	s_rctreewnd.left = 0;
	s_rctreewnd.bottom = 0;
	s_rctreewnd.right = 0;
	s_rctoolwnd.top = 0;
	s_rctoolwnd.left = 0;
	s_rctoolwnd.bottom = 0;
	s_rctoolwnd.right = 0;
	s_rcltwnd.top = 0;
	s_rcltwnd.left = 0;
	s_rcltwnd.bottom = 0;
	s_rcltwnd.right = 0;
	s_rcsidemenuwnd.top = 0;
	s_rcsidemenuwnd.left = 0;
	s_rcsidemenuwnd.bottom = 0;
	s_rcsidemenuwnd.right = 0;
	s_rcrigidwnd.top = 0;
	s_rcrigidwnd.left = 0;
	s_rcrigidwnd.bottom = 0;
	s_rcrigidwnd.right = 0;
	s_rcinfownd.top = 0;
	s_rcinfownd.left = 0;
	s_rcinfownd.bottom = 0;
	s_rcinfownd.right = 0;
	s_rcmainmenuaimbarwnd.top = 0;
	s_rcmainmenuaimbarwnd.left = 0;
	s_rcmainmenuaimbarwnd.bottom = 0;
	s_rcmainmenuaimbarwnd.right = 0;


	s_CamTargetCheckBox = 0;
	//s_LightCheckBox = 0;
	s_ApplyEndCheckBox = 0;
	//s_SlerpOffCheckBox = 0;
	s_AbsIKCheckBox = 0;
	s_BoneMarkCheckBox = 0;
	s_RigidMarkCheckBox = 0;
	//s_PseudoLocalCheckBox = 0;
	s_WallScrapingIKCheckBox = 0;
	s_LimitDegCheckBox = 0;
	s_BrushMirrorUCheckBox = 0;
	s_BrushMirrorVCheckBox = 0;
	s_IfMirrorVDiv2CheckBox = 0;

	//Left
	s_ui_lightscale = 0;
	s_ui_dispbone = 0;
	s_ui_disprigid = 0;
	s_ui_boneaxis = 0;
	s_ui_bone = 0;
	s_ui_locktosel = 0;
	s_ui_iklevel = 0;
	s_ui_editmode = 0;
	s_ui_texapplyrate = 0;
	s_ui_slapplyrate = 0;
	s_ui_motionbrush = 0;
	s_ui_texikorder = 0;
	s_ui_slikorder = 0;
	//s_ui_texikrate = 0;
	//s_ui_slikrate = 0;
	s_ui_texref = 0;
	s_ui_slirefpos = 0;
	s_ui_slirefmult = 0;
	s_ui_applytotheend = 0;
	s_ui_slerpoff = 0;
	s_ui_absikon = 0;
	s_ui_texbrushrepeats = 0;
	s_ui_brushrepeats = 0;
	s_ui_brushmirroru = 0;
	s_ui_brushmirrorv = 0;
	s_ui_ifmirrorvdiv2 = 0;

	//Left 2nd
	s_ui_texthreadnum = 0;
	s_ui_slthreadnum = 0;
	//s_ui_pseudolocal = 0;
	s_ui_wallscrapingik = 0;
	s_ui_limiteul = 0;
	s_ui_texspeed = 0;
	s_ui_speed = 0;
	//Bullet
	s_ui_btstart = 0;
	s_ui_btrecstart = 0;
	s_ui_stopbt = 0;
	s_ui_texbtcalccnt = 0;
	s_ui_btcalccnt = 0;
	s_ui_texerp = 0;
	s_ui_erp = 0;
	//PhysicsIK
	s_ui_texphysmv = 0;
	s_ui_slphysmv = 0;
	s_ui_physrotstart = 0;
	s_ui_physmvstart = 0;
	s_ui_physikstop = 0;


	
	bool bsuccess1 = false;
	bool bsuccess2 = false;
	if ((s_appcnt == 0) && (s_launchbyc4 == 0)) {//C4から起動時にはゲームパッド未対応。//2021/08/30
		bsuccess1 = StartDS4();
	}
	else {
		bsuccess1 = false;
	}
	if (bsuccess1) {
		bsuccess2 = GetController();
	}
	if (bsuccess1 && bsuccess2) {
		g_enableDS = true;
		s_dsdeviceid = 0;
		s_curaimbarno = 0;
	}
	else {
		g_enableDS = false;
		s_dsdeviceid = -1;
		s_curaimbarno = -1;
	}


	CRigidElem::InitRigidElems();
	CBone::InitBones();
	CMotionPoint::InitMotionPoints();
	InitEulKeys();
	InitKeys();

	s_motmenuindexmap.clear();
	s_reindexmap.clear();
	s_rgdindexmap.clear();

	s_anglelimitbone = 0;
	s_anglelimitdlg = 0;
	InitAngleLimit(&s_anglelimit);
	InitAngleLimit(&s_anglelimitcopy);

	s_rotaxisdlg = 0;

	g_motionbrush_method = 0;
	g_motionbrush_startframe = 0.0;
	g_motionbrush_endframe = 0.0;
	g_motionbrush_applyframe = 0.0;
	g_motionbrush_numframe = 0.0;
	g_motionbrush_frameleng = 0;
	g_motionbrush_value = 0;

	s_timelineWnd = 0;
	s_owpTimeline = 0;
	s_owpPlayerButton = 0;
	s_parentcheck = 0;
	s_LtimelineWnd = 0;
	s_owpLTimeline = 0;
	s_owpEulerGraph = 0;
	s_LTSeparator = 0;
	s_sidemenusp = 0;
	s_sidemenusp1 = 0;
	s_sidemenusp2 = 0;

	s_mainmenuaimbarWnd = 0;
	s_mainmenulabel = 0;

	s_toolWnd = 0;
	s_toolSeparator = 0;
	s_toolCopyB = 0;
	s_toolSymCopyB = 0;
	s_toolCutB = 0;
	s_toolPasteB = 0;
	s_toolDeleteB = 0;
	s_toolMotPropB = 0;
	s_toolMarkB = 0;
	s_toolSelBoneB = 0;
	s_toolInitMPB = 0;
	s_toolFilterB = 0;
	s_toolInterpolateB = 0;
	s_toolSelectCopyFileName = 0;

	s_customrigbone = 0;
	s_customrigdlg = 0;

	g_underselectingframe = 0;
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
	ChaMatrixIdentity(&s_selectmat_posture);
	ChaMatrixIdentity(&s_ikselectmat);

	s_convbone_model = 0;
	s_convbone_model_batch = 0;
	s_convbone_bvh = 0;
	s_maxboneno = 0;
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

	s_bvhbone_cbno = 0;


	ZeroMemory(s_spaxis, sizeof( SPAXIS ) * SPAXISNUM);
	ZeroMemory(s_spcam, sizeof(SPCAM) * SPR_CAM_MAX);
	ZeroMemory(s_sprig, sizeof(SPELEM) * SPRIGMAX);
	//ZeroMemory(&s_spbt, sizeof(SPELEM));
	ZeroMemory(&s_spmousehere, sizeof(SPELEM));
	ZeroMemory(&s_spret2prev, sizeof(SPELEM));
	ZeroMemory(&s_mousecenteron, sizeof(SPELEM));

	{
		ZeroMemory(&s_spaimbar, sizeof(SPGUISW) * SPAIMBARNUM);
		int spgno;
		for (spgno = 0; spgno < SPAIMBARNUM; spgno++) {
			s_spaimbar[spgno].state = false;
		}
	}
	{
		ZeroMemory(&s_spmenuaimbar, sizeof(SPGUISW) * SPMENU_MAX);
		int spgno;
		for (spgno = 0; spgno < SPMENU_MAX; spgno++) {
			s_spmenuaimbar[spgno].state = false;
		}
	}

	{
		ZeroMemory(&s_spsel3d, sizeof(SPGUISW));
		s_spsel3d.state = false;
	}

	{
		ZeroMemory(&s_spikmodesw, sizeof(SPGUISW) * 3);
		s_spikmodesw[0].state = true;
		s_spikmodesw[1].state = false;
		s_spikmodesw[2].state = false;
	}
	{
		ZeroMemory(&s_sprefpos, sizeof(SPGUISW));
		//s_sprefpos.state = true;
		s_sprefpos.state = false;//2021/11/22 ReferencePose Off by default
	}
	{
		ZeroMemory(&s_spguisw, sizeof(SPGUISW) * SPGUISWNUM);
		int spgno;
		for (spgno = 0; spgno < SPGUISWNUM; spgno++) {
			s_spguisw[spgno].state = true;//初回のGUISetVisibleで反転してfalseになる
		}
	}
	{
		ZeroMemory(&s_sprigidsw, sizeof(SPGUISW) * SPRIGIDSWNUM);
		int spgno;
		for (spgno = 0; spgno < SPRIGIDSWNUM; spgno++) {
			s_sprigidsw[spgno].state = false;
		}
		s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].state = true;
	}

	ZeroMemory(&s_spretargetsw, sizeof(SPGUISW) * SPRETARGETSWNUM);
	s_spretargetsw[SPRETARGETSW_RETARGET].state = false;
	s_spretargetsw[SPRETARGETSW_LIMITEULER].state = false;


	g_bonecntmap.clear();

	ChaMatrixIdentity( &s_inimat );

    g_bEnablePreshader = true;

    for( int i = 0; i < MAX_LIGHTS; i++ ){
        g_LightControl[i].SetLightDirection( ChaVector3( sinf( PI * 2 * ( MAX_LIGHTS - i - 1 ) / MAX_LIGHTS - PI / 6 ),
                                                          0, -cosf( PI * 2 * ( MAX_LIGHTS - i - 1 ) / MAX_LIGHTS - PI / 6 ) ).D3DX() );
	}

    g_nActiveLight = 0;
	g_nNumActiveLights = 1;
    g_fLightScale = 1.0f;

	//CreateUtDialog();



	//CThreadsUpdateMatrix
	s_tum.InitParams();
	s_tum.CreateThreads();



//////////
	ZeroMemory( &s_pickinfo, sizeof( PICKINFO ) );

	s_modelpanel.panel = 0;
	s_modelpanel.radiobutton = 0;
	s_modelpanel.separator = 0;
	s_modelpanel.separator2 = 0;
	s_modelpanel.checkvec.clear();
	s_modelpanel.delbutton.clear();
	s_modelpanel.modelindex = -1;

	s_motionpanel.panel = 0;
	s_motionpanel.radiobutton = 0;
	s_motionpanel.separator = 0;
	s_motionpanel.delbutton.clear();
	s_motionpanel.modelindex = -1;


	{
		s_bpWorld = new BPWorld(NULL, s_matWorld, "BtPiyo", // ウィンドウのタイトル
			460, 460,         // ウィンドウの幅と高さ [pixels]
			NULL);    // モニタリング用関数へのポインタ  
		_ASSERT(s_bpWorld);


		s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->enableFixedTimeStep(false);
		//s_bpWorld->setTimeStep(0.015);// seconds
		s_bpWorld->setGlobalERP(btScalar(g_erp));// ERP
		//s_bpWorld->start();// ウィンドウを表示して，シミュレーションを開始する
		s_btWorld = s_bpWorld->getDynamicsWorld();
		s_bpWorld->setNumThread(g_numthread);
	}


	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	InterlockedExchange(&g_underApealingMouseHere, (LONG)0);
	if (g_enableDS == true) {
		s_dsupdater = new CDSUpdateUnderTracking();
		if (s_dsupdater) {
			int isuccess = s_dsupdater->CreateDSUpdateUnderTracking(GetModuleHandle(NULL));
			if (isuccess != 0) {
				delete s_dsupdater;
				s_dsupdater = 0;
			}
		}
	}
	else {
		s_dsupdater = 0;
	}

	s_doneinit = 1;
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	// Uncomment this to get debug information from D3D11
	//pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;

	// For the first device created if its a REF device, optionally display a warning dialog box
	//static bool s_bFirstTime = true;
	//if (s_bFirstTime)
	//{
	//	s_bFirstTime = false;
	//	//if ((DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
	//	//	pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE))
	//	if ((pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE))
	//	{
	//		DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	//	}
	//}

	return true;
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	//V_RETURN(g_D3DSettingsDlg.OnD3D11CreateDevice(pd3dDevice));
	g_pTxtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 10);


	s_pdev = pd3dDevice;

	//hr = g_SettingsDlg.OnD3D11CreateDevice(pd3dDevice);
	//if (FAILED(hr)) {
	//	_ASSERT(0);
	//	return hr;
	//}
	//hr = D3DX10CreateFont(pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
	//	OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
	//	L"MS ゴシック", &g_pFont);
	//	//L"Arial", &g_pFont10);
	//if (FAILED(hr)) {
	//	_ASSERT(0);
	//	return hr;
	//}

	//hr = D3DX10CreateSprite(pd3dDevice, 512, &g_pSprite);
	//if (FAILED(hr)) {
	//	_ASSERT(0);
	//	return hr;
	//}
	////g_pTxtHelper = new CDXUTTextHelper(NULL, NULL, g_pFont, g_pSprite, 15);



	//DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;

	////	DWORD dwShaderFlags = D3D11_SHADER_ENABLE_STRICTNESS;
////#if defined( DEBUG ) || defined( _DEBUG )
////	// Set the D3D11_SHADER_DEBUG flag to embed debug information in the shaders.
////	// Setting this flag improves the shader debugging experience, but still allows 
////	// the shaders to be optimized and to run exactly the way they will run in 
////	// the release configuration of this program.
////	dwShaderFlags |= D3D11_SHADER_DEBUG;
////#endif
//
	//DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	//DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
//
//#if defined( DEBUG ) || defined( _DEBUG )
//	// Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
//	// Setting this flag improves the shader debugging experience, but still allows 
//	// the shaders to be optimized and to run exactly the way they will run in 
//	// the release configuration of this program.
//	dwShaderFlags |= D3DXSHADER_DEBUG;
//#endif
//
//#ifdef DEBUG_VS
//	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
//#endif
//#ifdef DEBUG_PS
//	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
//#endif
//
//	// Preshaders are parts of the shader that the effect system pulls out of the 
//	// shader and runs on the host CPU. They should be used if you are GPU limited. 
//	// The D3DXSHADER_NO_PRESHADER flag disables preshaders.
//	if (!g_bEnablePreshader)
//		dwShaderFlags |= D3DXSHADER_NO_PRESHADER;


	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	hr = DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"..\\Media\\Shader\\Ochakko.fx");
	//hr = DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"..\\Media\\Shader\\Ochakko11.fx");
	//hr = DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"..\\Media\\Shader\\Ochakko.fxo");
	if (FAILED(hr)) {
		_ASSERT(0);
		return hr;
	}
	//ID3D11Blob*	l_pBlob_Effect = NULL;
	//ID3D11Blob*	l_pBlob_Errors = NULL;
	//hr = D3DX10CompileEffectFromFile(str, NULL, NULL,
	//	D3D11_SHADER_ENABLE_STRICTNESS, 0, NULL,
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
	  CONST D3D11_SHADER_MACRO *pDefines,
	  ID3D11Include *pInclude,
	  LPCSTR pProfile,
	  UINT HLSLFlags,
	  UINT FXFlags,
	  ID3D11Device *pDevice,
	  ID3D11EffectPool *pEffectPool,
	  ID3DX10ThreadPump *pPump,
	  ID3D11Effect **ppEffect,
	  ID3D11Blob **ppErrors,
	  HRESULT *pHResult
	);*/

	//D3DX10CreateEffectFromFile(str, NULL, NULL, "fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL,
	//	NULL, &g_pEffect10, NULL, NULL);


	ID3D10Blob*	l_pBlob_Errors = NULL;
	//hr = D3DX11CreateEffectFromFile(str, NULL, NULL,
	//	"fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL, NULL,
	//	&g_pEffect, &l_pBlob_Errors, NULL);
	//hr = D3DX11CreateEffectFromFile(str, dwShaderFlags, pd3dDevice, &g_pEffect);
	//HRESULT WINAPI D3DX11CreateEffectFromFile(LPCWSTR pFileName, UINT FXFlags, ID3D11Device *pDevice, ID3DX11Effect **ppEffect)
	

//	//compile shader
//	ID3DBlob* errorBlob;
//	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
//
//#if defined _DEBUG || defined DEBUG
//	shaderFlags = D3DCOMPILE_DEBUG;
//#endif

	hr = D3DX11CompileEffectFromFile(str, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dwShaderFlags,
		0, pd3dDevice, &g_pEffect, &l_pBlob_Errors);
	////if (FAILED(hr))
	////{
	////	MessageBox(nullptr, (LPCWSTR)errorBlob->GetBufferPointer(), L"error", MB_OK);
	////	return hr;
	////}
	////m_pTechnique = m_pFx->GetTechniqueByName("ColorTech");
	////m_pFxWorldViewProj = m_pFx->GetVariableByName("gWorldViewProj")->AsMatrix();

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


	CalcTotalBound();


	CallF(GetShaderHandle(), return S_FALSE);



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
		return S_FALSE;
	}
	CallF(s_select->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\select_2.mqo", 0, 1.0f, 0), return S_FALSE);
	CallF(s_select->MakeDispObj(), return S_FALSE;);

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

	//s_matredmat = s_matred->GetDif4F();
	//s_ringredmat = s_ringred->GetDif4F();
	//s_matbluemat = s_matblue->GetDif4F();
	//s_ringbluemat = s_ringblue->GetDif4F();
	//s_matgreenmat = s_matgreen->GetDif4F();
	//s_ringgreenmat = s_ringgreen->GetDif4F();
	//s_matyellowmat = s_matyellow->GetDif4F();
	s_matredmat = ChaVector4(255.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
	s_ringredmat = ChaVector4(255.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
	s_matbluemat = ChaVector4(150.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	s_ringbluemat = ChaVector4(150.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	s_matgreenmat = ChaVector4(0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f);
	s_ringgreenmat = ChaVector4(0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f);
	s_matyellowmat = s_matyellow->GetDif4F();




	s_select_posture = new CModel();
	if (!s_select_posture) {
		_ASSERT(0);
		return S_FALSE;
	}
	CallF(s_select_posture->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\select_2_posture.mqo", 0, 1.0f, 0), return S_FALSE);
	CallF(s_select_posture->MakeDispObj(), return S_FALSE);


	s_rigmark = new CModel();
	if (!s_rigmark) {
		_ASSERT(0);
		return S_FALSE;
	}
	CallF(s_rigmark->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\rigmark.mqo", 0, 0.30f, 0), return S_FALSE);
	CallF(s_rigmark->MakeDispObj(), return S_FALSE);
	s_matrig = s_rigmark->GetMQOMaterialByName("mat1");
	_ASSERT(s_matrig);
	s_matrigmat = ChaVector4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f);


	s_bmark = new CModel();
	if (!s_bmark) {
		_ASSERT(0);
		return S_FALSE;
	}
	CallF(s_bmark->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\bonemark.mqo", 0, 1.0f, 0), return S_FALSE);
	CallF(s_bmark->MakeDispObj(), return S_FALSE);




	s_ground = new CModel();
	if (!s_ground) {
		_ASSERT(0);
		return S_FALSE;
	}
	CallF(s_ground->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\ground2.mqo", 0, 1.0f, 0), return S_FALSE);
	CallF(s_ground->MakeDispObj(), return S_FALSE);

	s_gplane = new CModel();
	if (!s_gplane) {
		_ASSERT(0);
		return S_FALSE;
	}
	CallF(s_gplane->LoadMQO(s_pdev, pd3dImmediateContext, L"..\\Media\\MameMedia\\gplane.mqo", 0, 1.0f, 0), return S_FALSE);
	CallF(s_gplane->MakeDispObj(), return S_FALSE);
	ChaVector3 tra(0.0f, 0.0, 0.0f);
	ChaVector3 mult(5.0f, 1.0f, 5.0f);
	CallF(s_gplane->MultDispObj(mult, tra), return S_FALSE);


	s_bcircle = new CMySprite(s_pdev);
	if (!s_bcircle) {
		_ASSERT(0);
		return S_FALSE;
	}

	WCHAR path[MAX_PATH];
	wcscpy_s(path, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(path, TEXT('\\'));
	if (!lasten) {
		_ASSERT(0);
		return S_FALSE;
	}
	*lasten = 0L;
	last2en = wcsrchr(path, TEXT('\\'));
	if (!last2en) {
		_ASSERT(0);
		return S_FALSE;
	}
	*last2en = 0L;
	wcscat_s(path, MAX_PATH, L"\\Media\\MameMedia\\");
	CallF(s_bcircle->Create(pd3dImmediateContext, path, L"bonecircle.dds", 0, 0), return S_FALSE);

	///////
	WCHAR mpath[MAX_PATH];
	wcscpy_s(mpath, MAX_PATH, g_basedir);
	lasten = 0;
	last2en = 0;
	lasten = wcsrchr(mpath, TEXT('\\'));
	if (!lasten) {
		_ASSERT(0);
		return S_FALSE;
	}
	*lasten = 0L;
	last2en = wcsrchr(mpath, TEXT('\\'));
	if (!last2en) {
		_ASSERT(0);
		return S_FALSE;
	}
	*last2en = 0L;
	wcscat_s(mpath, MAX_PATH, L"\\Media\\MameMedia\\");

	s_spaxis[0].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[0].sprite);
	CallF(s_spaxis[0].sprite->Create(pd3dImmediateContext, mpath, L"X.gif", 0, 0), return S_FALSE);
	s_spaxis[1].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[1].sprite);
	CallF(s_spaxis[1].sprite->Create(pd3dImmediateContext, mpath, L"Y.gif", 0, 0), return S_FALSE);
	s_spaxis[2].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spaxis[2].sprite);
	CallF(s_spaxis[2].sprite->Create(pd3dImmediateContext, mpath, L"Z.gif", 0, 0), return S_FALSE);

	//SpriteSwitch RefPos
	s_sprefpos.spriteON = new CMySprite(s_pdev);
	_ASSERT(s_sprefpos.spriteON);
	CallF(s_sprefpos.spriteON->Create(pd3dImmediateContext, mpath, L"RefPosON.gif", 0, 0), return S_FALSE);
	s_sprefpos.spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_sprefpos.spriteOFF);
	CallF(s_sprefpos.spriteOFF->Create(pd3dImmediateContext, mpath, L"RefPosOFF.gif", 0, 0), return S_FALSE);

	//SpriteSwitch IKMode
	s_spikmodesw[0].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[0].spriteON);
	CallF(s_spikmodesw[0].spriteON->Create(pd3dImmediateContext, mpath, L"IKRot2ON.gif", 0, 0), return S_FALSE);
	s_spikmodesw[0].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[0].spriteOFF);
	CallF(s_spikmodesw[0].spriteOFF->Create(pd3dImmediateContext, mpath, L"IKRot2OFF.gif", 0, 0), return S_FALSE);
	s_spikmodesw[1].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[1].spriteON);
	CallF(s_spikmodesw[1].spriteON->Create(pd3dImmediateContext, mpath, L"IKMove2ON.gif", 0, 0), return S_FALSE);
	s_spikmodesw[1].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[1].spriteOFF);
	CallF(s_spikmodesw[1].spriteOFF->Create(pd3dImmediateContext, mpath, L"IKMove2OFF.gif", 0, 0), return S_FALSE);
	s_spikmodesw[2].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[2].spriteON);
	CallF(s_spikmodesw[2].spriteON->Create(pd3dImmediateContext, mpath, L"IKScale2ON.gif", 0, 0), return S_FALSE);
	s_spikmodesw[2].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spikmodesw[2].spriteOFF);
	CallF(s_spikmodesw[2].spriteOFF->Create(pd3dImmediateContext, mpath, L"IKScale2OFF.gif", 0, 0), return S_FALSE);


	//SpriteSwitch ON
	s_spguisw[SPGUISW_SPRITEFK].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_SPRITEFK].spriteON);
	CallF(s_spguisw[SPGUISW_SPRITEFK].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_SpriteIK140ON.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_LEFT].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_LEFT].spriteON);
	CallF(s_spguisw[SPGUISW_LEFT].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_BasicSettings1140ON.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_LEFT2ND].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_LEFT2ND].spriteON);
	CallF(s_spguisw[SPGUISW_LEFT2ND].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_BasicSettings2140ON.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_BULLETPHYSICS].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_BULLETPHYSICS].spriteON);
	CallF(s_spguisw[SPGUISW_BULLETPHYSICS].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_BulletPhysics140ON.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_PHYSICSIK].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_PHYSICSIK].spriteON);
	CallF(s_spguisw[SPGUISW_PHYSICSIK].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_Experimental140ON.png", 0, 0), return S_FALSE);
	//SpriteSwitch OFF
	s_spguisw[SPGUISW_SPRITEFK].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_SPRITEFK].spriteOFF);
	CallF(s_spguisw[SPGUISW_SPRITEFK].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_SpriteIK140OFF.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_LEFT].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_LEFT].spriteOFF);
	CallF(s_spguisw[SPGUISW_LEFT].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_BasicSettings1140OFF.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_LEFT2ND].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_LEFT2ND].spriteOFF);
	CallF(s_spguisw[SPGUISW_LEFT2ND].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_BasicSettings2140OFF.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_BULLETPHYSICS].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_BULLETPHYSICS].spriteOFF);
	CallF(s_spguisw[SPGUISW_BULLETPHYSICS].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_BulletPhysics140OFF.png", 0, 0), return S_FALSE);
	s_spguisw[SPGUISW_PHYSICSIK].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spguisw[SPGUISW_PHYSICSIK].spriteOFF);
	CallF(s_spguisw[SPGUISW_PHYSICSIK].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_Experimental140OFF.png", 0, 0), return S_FALSE);


	//RigidSwitch ON
	s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteON);
	CallF(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuRigid140ON.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_IMPULSE].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_IMPULSE].spriteON);
	CallF(s_sprigidsw[SPRIGIDSW_IMPULSE].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuImpulse140ON.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteON);
	CallF(s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuGP140ON.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteON);
	CallF(s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuDamp140ON.png", 0, 0), return S_FALSE);
	//RigidSwitch OFF
	s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteOFF);
	CallF(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuRigid140OFF.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_IMPULSE].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_IMPULSE].spriteOFF);
	CallF(s_sprigidsw[SPRIGIDSW_IMPULSE].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuImpulse140OFF.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteOFF);
	CallF(s_sprigidsw[SPRIGIDSW_GROUNDPLANE].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuGP140OFF.png", 0, 0), return S_FALSE);
	s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteOFF);
	CallF(s_sprigidsw[SPRIGIDSW_DAMPANIM].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlate_menuDamp140OFF.png", 0, 0), return S_FALSE);

	s_spretargetsw[SPRETARGETSW_RETARGET].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spretargetsw[SPRETARGETSW_RETARGET].spriteON);
	CallF(s_spretargetsw[SPRETARGETSW_RETARGET].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlateRetarget140ON.png", 0, 0), return S_FALSE);
	s_spretargetsw[SPRETARGETSW_RETARGET].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spretargetsw[SPRETARGETSW_RETARGET].spriteOFF);
	CallF(s_spretargetsw[SPRETARGETSW_RETARGET].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlateRetarget140OFF.png", 0, 0), return S_FALSE);
	s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteON = new CMySprite(s_pdev);
	_ASSERT(s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteON);
	CallF(s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlateLimitEuler140ON.png", 0, 0), return S_FALSE);
	s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteOFF = new CMySprite(s_pdev);
	_ASSERT(s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteOFF);
	CallF(s_spretargetsw[SPRETARGETSW_LIMITEULER].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlateLimitEuler140OFF.png", 0, 0), return S_FALSE);

	{
		int aimno;
		for (aimno = 0; aimno < SPAIMBARNUM; aimno++) {
			s_spaimbar[aimno].spriteON = new CMySprite(s_pdev);
			_ASSERT(s_spaimbar[aimno].spriteON);
			CallF(s_spaimbar[aimno].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlateAim140ON.png", 0, 0), return S_FALSE);
			s_spaimbar[aimno].spriteOFF = new CMySprite(s_pdev);
			_ASSERT(s_spaimbar[aimno].spriteOFF);
			CallF(s_spaimbar[aimno].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlateAim140OFF.png", 0, 0), return S_FALSE);
		}
	}
	{
		int aimno;
		for (aimno = 0; aimno < SPMENU_MAX; aimno++) {
			s_spmenuaimbar[aimno].spriteON = new CMySprite(s_pdev);
			_ASSERT(s_spmenuaimbar[aimno].spriteON);
			CallF(s_spmenuaimbar[aimno].spriteON->Create(pd3dImmediateContext, mpath, L"GUIPlateAim140ON.png", 0, 0), return S_FALSE);
			s_spmenuaimbar[aimno].spriteOFF = new CMySprite(s_pdev);
			_ASSERT(s_spmenuaimbar[aimno].spriteOFF);
			CallF(s_spmenuaimbar[aimno].spriteOFF->Create(pd3dImmediateContext, mpath, L"GUIPlateAim140OFF.png", 0, 0), return S_FALSE);
		}
	}

	{
		s_spsel3d.spriteON = new CMySprite(s_pdev);
		_ASSERT(s_spsel3d.spriteON);
		CallF(s_spsel3d.spriteON->Create(pd3dImmediateContext, mpath, L"button101_Select.tif", 0, 0), return S_FALSE);
		s_spsel3d.spriteOFF = new CMySprite(s_pdev);
		_ASSERT(s_spsel3d.spriteOFF);
		CallF(s_spsel3d.spriteOFF->Create(pd3dImmediateContext, mpath, L"button101_UnSelect.tif", 0, 0), return S_FALSE);
	}



	s_spcam[SPR_CAM_I].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_I].sprite);
	CallF(s_spcam[SPR_CAM_I].sprite->Create(pd3dImmediateContext, mpath, L"cam_i.gif", 0, 0), return S_FALSE);
	s_spcam[SPR_CAM_KAI].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_KAI].sprite);
	CallF(s_spcam[SPR_CAM_KAI].sprite->Create(pd3dImmediateContext, mpath, L"cam_kai.gif", 0, 0), return S_FALSE);
	s_spcam[SPR_CAM_KAKU].sprite = new CMySprite(s_pdev);
	_ASSERT(s_spcam[SPR_CAM_KAKU].sprite);
	CallF(s_spcam[SPR_CAM_KAKU].sprite->Create(pd3dImmediateContext, mpath, L"cam_kaku.gif", 0, 0), return S_FALSE);

	s_sprig[SPRIG_INACTIVE].sprite = new CMySprite(s_pdev);
	_ASSERT(s_sprig[SPRIG_INACTIVE].sprite);
	CallF(s_sprig[SPRIG_INACTIVE].sprite->Create(pd3dImmediateContext, mpath, L"RigOFF.gif", 0, 0), return S_FALSE);
	s_sprig[SPRIG_ACTIVE].sprite = new CMySprite(s_pdev);
	_ASSERT(s_sprig[SPRIG_ACTIVE].sprite);
	CallF(s_sprig[SPRIG_ACTIVE].sprite->Create(pd3dImmediateContext, mpath, L"RigON.gif", 0, 0), return S_FALSE);

	//s_spbt.sprite = new CMySprite(s_pdev);
	//_ASSERT(s_spbt.sprite);
	//CallF(s_spbt.sprite->Create(pd3dImmediateContext, mpath, L"BtApply.png", 0, 0), return S_FALSE);

	s_spmousehere.sprite = new CMySprite(s_pdev);
	_ASSERT(s_spmousehere.sprite);
	CallF(s_spmousehere.sprite->Create(pd3dImmediateContext, mpath, L"img_l105.png", 0, 0), return S_FALSE);
	
	{
		WCHAR pngpath[MAX_PATH];
		swprintf_s(pngpath, MAX_PATH, L"%simg_l105.png", mpath);
		//swprintf_s(bmppath, MAX_PATH, L"%simg_l105.bmp", mpath);
		//g_mouseherebmp = (HBITMAP)::LoadImage(GetModuleHandle(NULL), bmppath, IMAGE_BITMAP, 52, 50, LR_LOADFROMFILE);
		//_ASSERT(g_mouseherebmp);
		//g_tranbmp = 0xFF000000;
		g_mousehereimage = new Gdiplus::Image(pngpath);
		_ASSERT(g_mousehereimage);
	}

	{
		WCHAR pngpath[MAX_PATH];
		swprintf_s(pngpath, MAX_PATH, L"%sMenuAimBar140.png", mpath);
		//swprintf_s(bmppath, MAX_PATH, L"%simg_l105.bmp", mpath);
		//g_mouseherebmp = (HBITMAP)::LoadImage(GetModuleHandle(NULL), bmppath, IMAGE_BITMAP, 52, 50, LR_LOADFROMFILE);
		//_ASSERT(g_mouseherebmp);
		//g_tranbmp = 0xFF000000;
		g_menuaimbarimage = new Gdiplus::Image(pngpath);
		_ASSERT(g_menuaimbarimage);
	}


	s_spret2prev.sprite = new CMySprite(s_pdev);
	_ASSERT(s_spret2prev.sprite);
	CallF(s_spret2prev.sprite->Create(pd3dImmediateContext, mpath, L"img_ret2prev.gif", 0, 0), return S_FALSE);

	s_mousecenteron.sprite = new CMySprite(s_pdev);
	_ASSERT(s_mousecenteron.sprite);
	CallF(s_mousecenteron.sprite->Create(pd3dImmediateContext, mpath, L"MouseCenterButtonON.png", 0, 0), return S_FALSE);

	///////
	//s_pdev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//s_pdev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	//s_pdev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//s_pdev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//s_pdev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//s_pdev->SetRenderState(D3DRS_ALPHAREF, 0x00);
	//s_pdev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(s_pdev->CreateBlendState(&blendDesc, &g_blendState)))
	{
		_ASSERT(0);
		return S_FALSE;
	}
	/*
	FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	g_context->OMSetBlendState(g_blendMode[BlendMode::NONE]->GetBlendState(), blendFactor, 0xffffffff);
	*/

	D3D11_DEPTH_STENCIL_DESC dsDescNormal;
	// Depth test parameters
	dsDescNormal.DepthEnable = true;
	dsDescNormal.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDescNormal.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	dsDescNormal.StencilEnable = true;
	dsDescNormal.StencilReadMask = 0xFF;
	dsDescNormal.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDescNormal.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescNormal.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDescNormal.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDescNormal.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	dsDescNormal.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescNormal.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDescNormal.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDescNormal.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	//ID3D11DepthStencilState * pDSState;
	s_pdev->CreateDepthStencilState(&dsDescNormal, &g_pDSStateZCmp);


	D3D11_DEPTH_STENCIL_DESC dsDescZCmpAlways;
	// Depth test parameters
	dsDescZCmpAlways.DepthEnable = false;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	dsDescZCmpAlways.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDescZCmpAlways.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	dsDescZCmpAlways.StencilEnable = true;
	dsDescZCmpAlways.StencilReadMask = 0xFF;
	dsDescZCmpAlways.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDescZCmpAlways.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescZCmpAlways.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDescZCmpAlways.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDescZCmpAlways.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	dsDescZCmpAlways.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescZCmpAlways.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDescZCmpAlways.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDescZCmpAlways.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	//ID3D11DepthStencilState * pDSState;
	s_pdev->CreateDepthStencilState(&dsDescZCmpAlways, &g_pDSStateZCmpAlways);


	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);




	WCHAR initialdir[MAX_PATH] = { 0L };
	wcscpy_s(initialdir, MAX_PATH, g_basedir);
	wcscat_s(initialdir, MAX_PATH, L"..\\Test\\");
	SetCurrentDirectoryW(initialdir);

	return S_OK;
}


////--------------------------------------------------------------------------------------
//// This callback function will be called immediately after the Direct3D device has been 
//// reset, which will happen after a lost device scenario. This is the best location to 
//// create 0 resources since these resources need to be reloaded whenever 
//// the device is lost. Resources created here should be released in the OnLostDevice 
//// callback. 
////--------------------------------------------------------------------------------------
//HRESULT CALLBACK OnResetDevice( ID3D11Device* pd3dDevice,
//                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
//{
//    HRESULT hr;
//
//    V_RETURN( g_DialogResourceManager.OnD3D11ResetDevice() );
//    V_RETURN( g_SettingsDlg.OnD3D11ResetDevice() );
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
//		CallF( g_texbank->Invalidate( INVAL_ONLYDEFAULT ), return S_FALSE );
//		CallF( g_texbank->Restore(), return 1 );
//	}
//
//    for( int i = 0; i < MAX_LIGHTS; i++ )
//        g_LightControl[i].OnD3D11ResetDevice( pBackBufferSurfaceDesc );
//
//    // Setup the camera's projection parameters
//    s_fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
////    g_Camera->SetProjParams( D3DX_PI / 4, fAspectRatio, g_initnear, 4.0f * g_initcamdist );
//	g_Camera->SetProjParams( D3DX_PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
//
//    g_Camera->SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
//	g_Camera->SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );
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
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	//V_RETURN(g_SettingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	s_fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
//    g_Camera->SetProjParams( D3DX_PI / 4, fAspectRatio, g_initnear, 4.0f * g_initcamdist );
	//g_Camera->SetProjParams( PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
	//g_Camera->SetProjParams(PI / 4, s_fAspectRatio, s_projnear, 100.0f * g_initcamdist);
	g_Camera->SetProjParams(PI / 4, s_fAspectRatio, s_projnear, 100.0f * g_initcamdist);

	g_Camera->SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera->SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );
	
	s_mainwidth = pBackBufferSurfaceDesc->Width;
	s_mainheight = pBackBufferSurfaceDesc->Height;
	
	
	g_SampleUI.SetLocation(0, 0);
	g_SampleUI.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	SetSpSel3DParams();
	SetSpAimBarParams();
	SetSpMenuAimBarParams();//CreateMainMenuAimBarWndよりも後
	SetSpAxisParams();
	SetSpGUISWParams();
	SetSpIkModeSWParams();
	SetSpRefPosSWParams();
	SetSpRigidSWParams();
	SetSpRetargetSWParams();
	SetSpCamParams();
	SetSpRigParams();
	//SetSpBtParams();
	SetSpMouseHereParams();
	SetSpMouseCenterParams();

	//g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	//g_HUD.SetSize(170, 170);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


int OnPluginClose()
{
	if (!s_plugin) {
		return 0;
	}

	int pluginno;
	for (pluginno = 0; pluginno < MAXPLUGIN; pluginno++) {
		if ((s_plugin + pluginno)->validflag == 1) {
			(s_plugin + pluginno)->CallOnClose();
		}
	}
	return 0;
}

int OnPluginPose()
{
	if (!s_plugin) {
		return 0;
	}

	//if (!m_shandler || (m_mhandler->m_kindnum <= 0)) {
	//	return 0;
	//}
	//int motid;
	//motid = g_motdlg->GetMotCookie();
	//if (motid < 0) {
	//	return 0;
	//}

	//int pluginno;
	//for (pluginno = 0; pluginno < MAXPLUGIN; pluginno++) {
	//	if ((m_plugin + pluginno)->validflag == 1) {
	//		(m_plugin + pluginno)->CallOnPose(motid);
	//	}
	//}

	return 0;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//DirectX11 : because of DX11 defferd destroy, some ref count will be alive.//!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	g_endappflag = 1;
	UNREFERENCED_PARAMETER(pUserContext);

	OrgWindowListenMouse(false);
	
	SaveIniFile();

	OnPluginClose();
	if (s_plugin) {
		delete[] s_plugin;
		s_plugin = 0;
	}

	//if (s_eventhook) {
		//UnhookWinEvent(s_eventhook);
		//s_eventhook = 0;
		CoUninitialize();
	//}



	if (s_dsupdater) {
		delete s_dsupdater;
		s_dsupdater = 0;
	}

	EndDS4();

	if (g_mousehereimage) {
		delete g_mousehereimage;
		g_mousehereimage = 0;
	}
	if (g_menuaimbarimage) {
		delete g_menuaimbarimage;
		g_menuaimbarimage = 0;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);


	if (g_mouseherebmp) {
		::DeleteObject(g_mouseherebmp);
		g_mouseherebmp = 0;
	}


	s_motmenuindexmap.clear();
	s_reindexmap.clear();
	s_rgdindexmap.clear();


	//::KillTimer(s_mainhwnd, s_iktimerid);
	/*
	g_DialogResourceManager.OnD3D11DestroyDevice();
	g_D3DSettingsDlg.OnD3D11DestroyDevice();
	//CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	SAFE_DELETE( g_pTxtHelper );

	g_Mesh11.Destroy();

	SAFE_RELEASE( g_pVertexLayout11 );
	SAFE_RELEASE( g_pVertexBuffer );
	SAFE_RELEASE( g_pIndexBuffer );
	SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pSamLinear );

	SAFE_RELEASE( g_pcbVSPerObject );
	SAFE_RELEASE( g_pcbPSPerObject );
	SAFE_RELEASE( g_pcbPSPerFrame );
	*/

	if (g_pDSStateZCmp) {
		g_pDSStateZCmp->Release();
		g_pDSStateZCmp = 0;
	}
	if (g_pDSStateZCmpAlways) {
		g_pDSStateZCmpAlways->Release();
		g_pDSStateZCmpAlways = 0;
	}

	g_DialogResourceManager.OnD3D11DestroyDevice();
	//g_SettingsDlg.OnD3D11DestroyDevice();
	CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	SAFE_DELETE(g_pTxtHelper);

	//SAFE_RELEASE(g_pFont);
	//SAFE_RELEASE(g_pSprite);
	//SAFE_RELEASE(g_pVertexLayout);
	//g_Mesh10.Destroy();

	SAFE_RELEASE(g_pEffect);

	SAFE_RELEASE(g_blendState);//!!!!!

	if (s_editrangehistory) {
		delete[] s_editrangehistory;
		s_editrangehistory = 0;
	}
	s_editrangehistoryno = 0;

	if (s_anglelimitdlg) {
		s_underanglelimithscroll = 0;
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
	
	if (s_copyhistorydlg.GetCreatedFlag() == true) {
		s_copyhistorydlg.DestroyWindow();
	}



	s_tum.DestroyObjs();



	CloseDbgFile();
	if (g_infownd) {
		delete g_infownd;
		g_infownd = 0;
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
			//curmodel->DestroyScene();

			delete curmodel;
		}
	}

	s_modelindex.clear();
	s_model = 0;




	if (s_select) {
		delete s_select;
		s_select = 0;
	}
	if (s_select_posture) {
		delete s_select_posture;
		s_select_posture = 0;
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
	if (s_toolSeparator) {
		delete s_toolSeparator;
		s_toolSeparator = 0;
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
	if (s_toolSelectCopyFileName) {
		delete s_toolSelectCopyFileName;
		s_toolSelectCopyFileName = 0;
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
	if (s_owpEulerGraph) {
		delete s_owpEulerGraph;
		s_owpEulerGraph = 0;
	}
	if (s_LTSeparator) {
		delete s_LTSeparator;
		s_LTSeparator = 0;
	}

	if (s_layerWnd) {
		delete s_layerWnd;
		s_layerWnd = 0;
	}
	if (s_owpLayerTable) {
		delete s_owpLayerTable;
		s_owpLayerTable = 0;
	}

	if (s_sphrateSlider) {
		delete s_sphrateSlider;
		s_sphrateSlider = 0;
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
	//if (s_massSeparator) {
	//	delete s_massSeparator;
	//	s_massSeparator = 0;
	//}
	//if (s_massSeparator1) {
	//	delete s_massSeparator1;
	//	s_massSeparator1 = 0;
	//}
	//if (s_massSeparator2) {
	//	delete s_massSeparator2;
	//	s_massSeparator2 = 0;
	//}
	if (s_massB) {
		delete s_massB;
		s_massB = 0;
	}
	if (s_thicknessB) {
		delete s_thicknessB;
		s_thicknessB = 0;
	}
	if (s_depthB) {
		delete s_depthB;
		s_depthB = 0;
	}
	if (s_massspacelabel) {
		delete s_massspacelabel;
		s_massspacelabel = 0;
	}
	if (s_namelabel) {
		delete s_namelabel;
		s_namelabel = 0;
	}
	if (s_lenglabel) {
		delete s_lenglabel;
		s_lenglabel = 0;
	}
	if (s_skipB) {
		delete s_skipB;
		s_skipB = 0;
	}
	if (s_rigidskip) {
		delete s_rigidskip;
		s_rigidskip = 0;
	}
	if (s_forbidSeparator) {
		delete s_forbidSeparator;
		s_forbidSeparator = 0;
	}
	if (s_forbidB) {
		delete s_forbidB;
		s_forbidB = 0;
	}
	if (s_forbidrot) {
		delete s_forbidrot;
		s_forbidrot = 0;
	}

	if (s_validSeparator) {
		delete s_validSeparator;
		s_validSeparator = 0;
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
	if (s_colSeparator) {
		delete s_colSeparator;
		s_colSeparator = 0;
	}
	if (s_colB) {
		delete s_colB;
		s_colB = 0;
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
	if (s_btforceSeparator) {
		delete s_btforceSeparator;
		s_btforceSeparator = 0;
	}
	if (s_btforce) {
		delete s_btforce;
		s_btforce = 0;
	}
	if (s_btforceB) {
		delete s_btforceB;
		s_btforceB = 0;
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

	if (s_placefolderlabel_1) {
		delete s_placefolderlabel_1;
		s_placefolderlabel_1 = 0;
	}
	if (s_placefolderlabel_2) {
		delete s_placefolderlabel_2;
		s_placefolderlabel_2 = 0;
	}
	if (s_placefolderlabel_3) {
		delete s_placefolderlabel_3;
		s_placefolderlabel_3 = 0;
	}
	if (s_placefolderWnd) {
		delete s_placefolderWnd;
		s_placefolderWnd = 0;
	}


	if (s_sidemenu_rigid) {
		delete s_sidemenu_rigid;
		s_sidemenu_rigid = 0;
	}
	if (s_sidemenu_limiteul) {
		delete s_sidemenu_limiteul;
		s_sidemenu_limiteul = 0;
	}
	if (s_sidemenu_copyhistory) {
		delete s_sidemenu_copyhistory;
		s_sidemenu_copyhistory = 0;
	}
	if (s_sidemenu_retarget) {
		delete s_sidemenu_retarget;
		s_sidemenu_retarget = 0;
	}
	if (s_sidemenusp) {
		delete s_sidemenusp;
		s_sidemenusp = 0;
	}
	if (s_sidemenusp1) {
		delete s_sidemenusp1;
		s_sidemenusp1 = 0;
	}
	if (s_sidemenusp2) {
		delete s_sidemenusp2;
		s_sidemenusp2 = 0;
	}
	if (s_sidemenu_rigid) {
		delete s_sidemenu_rigid;
		s_sidemenu_rigid = 0;
	}
	if (s_sidemenu_limiteul) {
		delete s_sidemenu_limiteul;
		s_sidemenu_limiteul = 0;
	}
	if (s_sidemenu_copyhistory) {
		delete s_sidemenu_copyhistory;
		s_sidemenu_copyhistory = 0;
	}
	if (s_sidemenu_retarget) {
		delete s_sidemenu_retarget;
		s_sidemenu_retarget = 0;
	}
	if (s_sidemenuWnd) {
		delete s_sidemenuWnd;
		s_sidemenuWnd = 0;
	}

	if (s_mainmenuaimbarWnd) {
		delete s_mainmenuaimbarWnd;
		s_mainmenuaimbarWnd = 0;
	}
	if (s_mainmenulabel) {
		delete s_mainmenulabel;
		s_mainmenulabel = 0;
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

	if (g_motionbrush_value) {
		free(g_motionbrush_value);
		g_motionbrush_value = 0;
	}

	{
		//static SPAXIS s_spaxis[3];
		int spno;
		for (spno = 0; spno < SPAXISNUM; spno++) {
			CMySprite* cursp = s_spaxis[spno].sprite;
			if (cursp) {
				delete cursp;
			}
			s_spaxis[spno].sprite = 0;
		}
	}

	{
		int spgno;
		for (spgno = 0; spgno < SPAIMBARNUM; spgno++) {
			CMySprite* curspgON = s_spaimbar[spgno].spriteON;
			if (curspgON) {
				delete curspgON;
			}
			s_spaimbar[spgno].spriteON = 0;

			CMySprite* curspgOFF = s_spaimbar[spgno].spriteOFF;
			if (curspgOFF) {
				delete curspgOFF;
			}
			s_spaimbar[spgno].spriteOFF = 0;
		}
	}
	{
		int spgno;
		for (spgno = 0; spgno < SPMENU_MAX; spgno++) {
			CMySprite* curspgON = s_spmenuaimbar[spgno].spriteON;
			if (curspgON) {
				delete curspgON;
			}
			s_spmenuaimbar[spgno].spriteON = 0;

			CMySprite* curspgOFF = s_spmenuaimbar[spgno].spriteOFF;
			if (curspgOFF) {
				delete curspgOFF;
			}
			s_spmenuaimbar[spgno].spriteOFF = 0;
		}
	}
	{
		CMySprite* curspgON = s_spsel3d.spriteON;
		if (curspgON) {
			delete curspgON;
		}
		s_spsel3d.spriteON = 0;

		CMySprite* curspgOFF = s_spsel3d.spriteOFF;
		if (curspgOFF) {
			delete curspgOFF;
		}
		s_spsel3d.spriteOFF = 0;
	}

	{
		int spgno;
		for (spgno = 0; spgno < 3; spgno++) {
			CMySprite* curspgON = s_spikmodesw[spgno].spriteON;
			if (curspgON) {
				delete curspgON;
			}
			s_spikmodesw[spgno].spriteON = 0;

			CMySprite* curspgOFF = s_spikmodesw[spgno].spriteOFF;
			if (curspgOFF) {
				delete curspgOFF;
			}
			s_spikmodesw[spgno].spriteOFF = 0;

		}
	}
	{
		CMySprite* curspgON = s_sprefpos.spriteON;
		if (curspgON) {
			delete curspgON;
		}
		s_sprefpos.spriteON = 0;

		CMySprite* curspgOFF = s_sprefpos.spriteOFF;
		if (curspgOFF) {
			delete curspgOFF;
		}
		s_sprefpos.spriteOFF = 0;
	}
	{
		int spgno;
		for (spgno = 0; spgno < SPGUISWNUM; spgno++) {
			CMySprite* curspgON = s_spguisw[spgno].spriteON;
			if (curspgON) {
				delete curspgON;
			}
			s_spguisw[spgno].spriteON = 0;

			CMySprite* curspgOFF = s_spguisw[spgno].spriteOFF;
			if (curspgOFF) {
				delete curspgOFF;
			}
			s_spguisw[spgno].spriteOFF = 0;

		}
	}
	{
		int spmno;
		for (spmno = 0; spmno < SPRIGIDSWNUM; spmno++) {
			CMySprite* curspmON = s_sprigidsw[spmno].spriteON;
			if (curspmON) {
				delete curspmON;
			}
			s_sprigidsw[spmno].spriteON = 0;

			CMySprite* curspmOFF = s_sprigidsw[spmno].spriteOFF;
			if (curspmOFF) {
				delete curspmOFF;
			}
			s_sprigidsw[spmno].spriteOFF = 0;

		}
	}
	{
		int sprno;
		for (sprno = 0; sprno < SPRETARGETSWNUM; sprno++) {
			if (s_spretargetsw[sprno].spriteON) {
				delete s_spretargetsw[sprno].spriteON;
			}
			s_spretargetsw[sprno].spriteON = 0;

			if (s_spretargetsw[sprno].spriteOFF) {
				delete s_spretargetsw[sprno].spriteOFF;
			}
			s_spretargetsw[sprno].spriteOFF = 0;
		}
	}
	{
		int spcno;
		for (spcno = 0; spcno < SPR_CAM_MAX; spcno++) {
			CMySprite* curspc = s_spcam[spcno].sprite;
			if (curspc) {
				delete curspc;
			}
			s_spcam[spcno].sprite = 0;
		}
	}
	{
		int sprigno;
		for (sprigno = 0; sprigno < SPRIGMAX; sprigno++) {
			CMySprite* cursp = s_sprig[sprigno].sprite;
			if (cursp) {
				delete cursp;
			}
			s_sprig[sprigno].sprite = 0;
		}
	}

	//CMySprite* cursp = s_spbt.sprite;
	//if (cursp) {
	//	delete cursp;
	//}
	//s_spbt.sprite = 0;

	CMySprite* curspmousehere = s_spmousehere.sprite;
	if (curspmousehere) {
		delete curspmousehere;
	}
	s_spmousehere.sprite = 0;


	CMySprite* curspret = s_spret2prev.sprite;
	if (curspret) {
		delete curspret;
	}
	s_spret2prev.sprite = 0;


	CMySprite* curspm = s_mousecenteron.sprite;
	if (curspm) {
		delete curspm;
	}
	s_mousecenteron.sprite = 0;


	if (g_Camera) {
		delete g_Camera;
		g_Camera = 0;
	}

	DestroyModelPanel();
	DestroyMotionPanel();
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


	CMotionPoint::DestroyMotionPoints();
	CBone::DestroyBones();
	CRigidElem::DestroyRigidElems();

	DestroyEulKeys();
	DestroyKeys();


	DeleteCriticalSection(&s_CritSection_LTimeline);
	DeleteCriticalSection(&g_CritSection_GetGP);

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

	//if (g_bvh2fbxbatchflag || g_motioncachebatchflag || g_retargetbatchflag) {
	//if((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)){
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		OnFrameBatchThread();
		return;
	}

	if (s_underdelmotion || s_underdelmodel || s_underselectmotion || s_underselectmodel) {
		OnFrameCloseFlag();
		OnFrameToolWnd();
	}
	else {
		WCHAR sz[100];
		swprintf_s(sz, 100, L"ThreadNum:%d(%d)", g_numthread, gNumIslands);
		g_SampleUI.GetStatic(IDC_STATIC_NUMTHREAD)->SetText(sz);


		if (g_undertrackingRMenu == 0) {
			OnDSUpdate();
		}

		OnFrameStartPreview(fTime, &savetime);


		OnFrameUtCheckBox();
		SetCamera6Angle();
		AutoCameraTarget();

		OnFrameKeyboard();

		if ((g_previewFlag == 0) && (s_savepreviewFlag != 0)) {
			s_cursorFlag = true;
		}
		OnFrameTimeLineWnd();


		OnFramePlayButton();
		OnFrameMouseButton();

		s_time = fTime;
		g_Camera->FrameMove(fElapsedTime);
		double difftime = fTime - savetime;

		s_matWorld = ChaMatrix(g_Camera->GetWorldMatrix());
		s_matProj = ChaMatrix(g_Camera->GetProjMatrix());
		s_matWorld._41 = 0.0f;
		s_matWorld._42 = 0.0f;
		s_matWorld._43 = 0.0f;
		//s_matW = s_matWorld;
		s_matVP = s_matView * s_matProj;

		int modelno;
		int modelnum = (int)s_modelindex.size();
		for (modelno = 0; modelno < modelnum; modelno++) {
			s_modelindex[modelno].modelptr->SetWorldMatFromCamera(s_matWorld);
		}

		double nextframe = 0.0;
		if (g_previewFlag) {
			if (s_model && s_model->GetCurMotInfo()) {
				if (g_previewFlag <= 3) {
					OnFramePreviewNormal(&nextframe, &difftime);
				}
				else if (g_previewFlag == 4) {//BTの物理
					OnFramePreviewBt(&nextframe, &difftime);
				}
				else if (g_previewFlag == 5) {//ラグドール
					OnFramePreviewRagdoll(&nextframe, &difftime);
				}
				OnTimeLineCursor(0, 0.0);
			}
			else {
				g_previewFlag = 0;
			}
		}
		else {
			OnFramePreviewStop();
		}
		//s_difftime = difftime;
		savetime = fTime;





		OnFrameCloseFlag();
		OnFrameToolWnd();
		OnFrameUndo(false, 0);

		OnFrameUpdateGround();
		OnFrameInitBtWorld();

		OnDSMouseHereApeal();


		//s_tum.WaitUpdateMatrix();
	}

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
		s_copymotvec.push_back(cpelem);
	}
	*/
	int rotcenterflag1 = 1;
	ChaMatrix localmat = curbone->CalcLocalScaleRotMat(rotcenterflag1, s_model->GetCurMotInfo()->motid, curframe);
	ChaVector3 curanimtra = curbone->CalcLocalTraAnim(s_model->GetCurMotInfo()->motid, curframe);
	ChaVector3 localscale = curbone->CalcLocalScaleAnim(s_model->GetCurMotInfo()->motid, curframe);
	
	localmat._41 += curanimtra.x;
	localmat._42 += curanimtra.y;
	localmat._43 += curanimtra.z;


	//localmat._11 *= localscale.x;
	//localmat._12 *= localscale.x;
	//localmat._13 *= localscale.x;
	//localmat._21 *= localscale.y;
	//localmat._22 *= localscale.y;
	//localmat._23 *= localscale.y;
	//localmat._31 *= localscale.z;
	//localmat._32 *= localscale.z;
	//localmat._33 *= localscale.z;

	CPELEM2 cpelem;
	ZeroMemory(&cpelem, sizeof(CPELEM2));
	cpelem.bone = curbone;
	cpelem.mp.SetFrame(curframe);
	cpelem.mp.SetWorldMat(localmat);
	cpelem.mp.SetLocalMatFlag(1);//!!!!!!!!!!
	s_copymotvec.push_back(cpelem);

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

	CPELEM2 cpelem;
	ZeroMemory(&cpelem, sizeof(CPELEM2));
	cpelem.bone = curbone;
	cpelem.mp.SetFrame(curframe);
	cpelem.mp.SetWorldMat(symmat);
	cpelem.mp.SetLocalMatFlag(1);//!!!!!!!!!!
	s_copymotvec.push_back(cpelem);

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
	//int isfirstbone = 0;
	cureul = curbone->CalcLocalEulXYZ(paraxsiflag, s_model->GetCurMotInfo()->motid, curframe, BEFEUL_ZERO);
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

void OnRenderNowLoading()
{
	/*
	HBRUSH GetSysColorBrush(
		int nIndex
	);

	　システムカラーのインデックスまたは定義されている定数名を指定します。
	　定義されているシステムカラーは以下の通りです。重複している定数もありますので、インデックスも表記しています。

	定数	 	内容
	COLOR_SCROLLBAR	0	スクロールバーの灰色の領域。
	COLOR_DESKTOP
	COLOR_BACKGROUND	1	デスクトップの色。
	COLOR_ACTIVECAPTION	2	アクティブなウィンドウのタイトルバーの色。
	グラデーションが有効な場合はグラデーションの左側の色。
	COLOR_INACTIVECAPTION	3	非アクティブなウィンドウのタイトルバーの色。
	グラデーションが有効な場合はグラデーションの左側の色。
	COLOR_MENU	4	メニューの背景色。
	COLOR_WINDOW	5	ウィンドウの背景色。
	COLOR_WINDOWFRAME	6	ウィンドウフレームの色。
	COLOR_MENUTEXT	7	メニュー内のテキストの色。
	COLOR_WINDOWTEXT	8	ウィンドウ内のテキストの色。
	COLOR_CAPTIONTEXT	9	キャプション、サイズボックス、スクロールバーの矢印ボックス内のテキストの色。
	COLOR_ACTIVEBORDER	10	アクティブなウィンドウの境界線の色。
	COLOR_INACTIVEBORDER	11	非アクティブなウィンドウの境界線の色。
	COLOR_APPWORKSPACE	12	MDIウィンドウの背景色。
	COLOR_HIGHLIGHT	13	コントロールで選択された時の反転色。
	COLOR_HIGHLIGHTTEXT	14	コントロールで選択された時のテキストの色。
	COLOR_3DFACE
	COLOR_BTNFACE	15	3D表示及びダイアログボックスの背景色。
	COLOR_3DSHADOW
	COLOR_BTNSHADOW	16	3D表示の影の色。
	COLOR_GRAYTEXT	17	無効のテキストの色。
	COLOR_BTNTEXT	18	プッシュボタンのテキストの色。
	COLOR_INACTIVECAPTIONTEXT	19	非アクティブウィンドウのキャプションのテキスト色。
	COLOR_3DHILIGHT
	COLOR_3DHIGHLIGHT
	COLOR_BTNHILIGHT
	COLOR_BTNHIGHLIGHT	20	3D表示のハイライト色。
	COLOR_3DDKSHADOW	21	3D表示の暗い影の色。
	COLOR_3DLIGHT	22	3D表示の光の色。
	COLOR_INFOTEXT	23	ツールチップのテキスト色。
	COLOR_INFOBK	24	ツールチップの背景色。
	-	25	不明
	COLOR_HOTLIGHT	26	ハイパーリンクされたアイテムの色。
	COLOR_GRADIENTACTIVECAPTION	27	アクティブウィンドウのタイトルバーのグラデーション右側の色。
	COLOR_GRADIENTINACTIVECAPTION	28	非アクティブウィンドウのタイトルバーのグラデーション右側の色。
	COLOR_MENUHILIGHT	29	ハイライトされたメニューアイテムの色。
	COLOR_MENUBAR	29	メニューバーの背景色。
	 */

	
	HDC hdc;
	RECT rect;
	HBRUSH brush;
	hdc = ::GetDC(s_3dwnd);
	::GetClientRect(s_3dwnd, &rect);
	brush = CreateSolidBrush(BLACK_BRUSH);
	//brush = CreateSolidBrush(RGB(64, 128, 256));
	//brush = ::GetSysColorBrush(COLOR_HIGHLIGHT);//黒と青でちらくつ
	//brush = ::GetSysColorBrush(COLOR_HIGHLIGHT);
	::FillRect(hdc, &rect, brush);
	::DeleteObject(brush);
	::ReleaseDC(s_3dwnd, hdc);
	
	return;

}

void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
		return;
	}

    //HRESULT hr;

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
	//float ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//float ClearColor[4] = { (64.0f / 255.0f), (128.0f / 255.0f), (255.0f / 255.0f), 1.0f };
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	//pd3dImmediateContext->ClearRenderTargetView(pRTV, s_ClearColor);
	if ((g_ClearColorIndex >= 0) && (g_ClearColorIndex < BGCOL_MAX)) {
		pd3dImmediateContext->ClearRenderTargetView(pRTV, g_ClearColor[g_ClearColorIndex]);
	}
	else {
		pd3dImmediateContext->ClearRenderTargetView(pRTV, ClearColor);
	}
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);




	ChaMatrix mWorldViewProjection;
	ChaVector3 vLightDir[MAX_LIGHTS];
	ChaVector4 vLightDiffuse[MAX_LIGHTS];
	ChaMatrix mWorld;
	ChaMatrix mView;
	ChaMatrix mProj;

	// Get the projection & view matrix from the camera class
//	mWorld = g_mCenterMesh * *g_Camera->GetWorldMatrix();
	mWorld = g_Camera->GetWorldMatrix();
	mProj = g_Camera->GetProjMatrix();
	mView = g_Camera->GetViewMatrix();

	mWorldViewProjection = mWorld * mView * mProj;

	// Render the light arrow so the user can visually see the light dir
	for (int i = 0; i < g_nNumActiveLights; i++)
	{
		DirectX::XMFLOAT4 arrowColor = (i == g_nActiveLight) ? DirectX::XMFLOAT4(1, 1, 0, 1) : DirectX::XMFLOAT4(1, 1, 1, 1);
		//V(g_LightControl[i].OnRender10(arrowColor, &mView, &mProj, g_Camera->GetEyePt()));
		vLightDir[i] = ChaVector3(g_LightControl[i].GetLightDirection());
		vLightDiffuse[i] = ChaVector4(1, 1, 1, 1) * g_fLightScale;
	}

	//V(g_pLightDir->SetRawValue(vLightDir, 0, sizeof(ChaVector3) * MAX_LIGHTS));
	//V(g_pLightDiffuse->SetFloatVectorArray((float*)vLightDiffuse, 0, MAX_LIGHTS));
	//V(g_pmWorldViewProjection->SetMatrix((float*)&mWorldViewProjection));
	//V(g_pmWorld->SetMatrix((float*)&mWorld));
	//V(g_pfTime->SetFloat((float)fTime));
	//V(g_pnNumLights->SetInt(g_nNumActiveLights));

	//// Render the scene with this technique as defined in the .fx file
	//ID3D11EffectTechnique* pRenderTechnique;
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

	//if ((InterlockedAdd(&g_retargetbatchflag, 0) == 0) && (InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 0) && (InterlockedAdd(&g_motioncachebatchflag, 0) == 0) &&
	if ((InterlockedAdd(&g_retargetbatchflag, 0) == 0) && (InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 0) && (InterlockedAdd(&g_calclimitedwmflag, 0) == 0) &&
		!s_underdelmodel && !s_underdelmotion && !s_underselectmodel && !s_underselectmotion) {
		OnRenderModel(pd3dImmediateContext);
		OnRenderGround(pd3dImmediateContext);
		OnRenderBoneMark(pd3dImmediateContext);
		OnRenderSelect(pd3dImmediateContext);
		//OnRenderUtDialog(fElapsedTime);
		if (s_dispsampleui) {//ctrl + 1 (one) key --> toggle
			OnRenderSprite(pd3dImmediateContext);
		}

		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
		////g_HUD.OnRender(fElapsedTime);
		if (s_dispsampleui) {//ctrl + 1 (one) key --> toggle
			g_SampleUI.OnRender(fElapsedTime);
		}
		RenderText(fTime);
		DXUT_EndPerfEvent();
	}
	else {
		OnRenderNowLoading();
	}
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DX10Font interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------

void RenderText( double fTime )
{
	static double s_savetime = 0.0;
	//double g_calcfps;
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
    //g_pTxtHelper->SetInsertionPos( 2, 0 );
	g_pTxtHelper->SetInsertionPos(175, 0);
    g_pTxtHelper->SetForegroundColor( DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) );
    //g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    //g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    g_pTxtHelper->SetForegroundColor(DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    //g_pTxtHelper->DrawFormattedTextLine( L"fps : %0.2f fTime: %0.1f, preview %d, btcanccnt %.1f, ERP %.5f", g_calcfps, fTime, g_previewFlag, g_btcalccnt, g_erp );
	//g_pTxtHelper->DrawFormattedTextLine(L"fps : %0.2f fTime: %0.1f, preview %d", g_calcfps, fTime, g_previewFlag);
	g_pTxtHelper->DrawFormattedTextLine(L"fps : %0.2f preview : %d mbuttoncnt %d", g_calcfps, g_previewFlag, s_mbuttoncnt);
	if (s_onefps == 1) {
		g_pTxtHelper->DrawFormattedTextLine(L" ");
		g_pTxtHelper->DrawFormattedTextLine(L"PlayerButton OneFpsMode : 1 fps");
	}
	else if (s_onefps == 2) {
		g_pTxtHelper->DrawFormattedTextLine(L" ");
		g_pTxtHelper->DrawFormattedTextLine(L"PlayerButton OneFpsMode : 2 fps");
	}

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
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D11BackBufferSurfaceDesc();
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

void PrepairUndo()
{
	if ((s_editmotionflag >= 0) || (g_btsimurecflag == true)) {
		if (s_model) {
			CreateTimeLineMark();
			SetLTimelineMark(s_curboneno);
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
		}
		s_editmotionflag = -1;
	}
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	if (!s_sampleuihwnd && hWnd && IsWindow(hWnd)) {
		s_sampleuihwnd = hWnd;
	}



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
	//g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) {
		//_ASSERT(0);
		return 0;
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	//g_Camera->HandleMessages(hWnd, uMsg, wParam, lParam);
	CBone* curbone = 0;
	CRigidElem* curre = 0;
	//if (g_retargetbatchflag == 0) {
	if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {
		//int maxboneno = 0;
		if (s_model && (s_curboneno >= 0)) {
			curbone = s_model->GetBoneByID(s_curboneno);
			curre = s_model->GetRigidElem(s_curboneno);
		}
		else {
			curbone = 0;
			curre = 0;
		}

		////if (g_retargetbatchflag == 0) {
		//if (s_model && s_convbone_bvh) {
		//	map<int, CBone*>::iterator itrbone;
		//	for (itrbone = s_convbone_bvh->GetBoneListBegin(); itrbone != s_convbone_bvh->GetBoneListEnd(); itrbone++) {
		//		CBone* curbone = itrbone->second;
		//		if (curbone) {
		//			int boneno = curbone->GetBoneNo();
		//			if (boneno > maxboneno) {
		//				maxboneno = boneno;
		//			}
		//		}
		//	}
		//}
		////}
	}

	if( uMsg == WM_COMMAND ){

		WORD menuid;
		menuid = LOWORD( wParam );
		int modelnum = (int)s_modelindex.size();

		if ((menuid >= (ID_RMENU_0 + MENUOFFSET_SETCONVBONEMODEL)) && (menuid < (ID_RMENU_0 + modelnum + MENUOFFSET_SETCONVBONEMODEL))) {
			int modelindex = menuid - ID_RMENU_0 - MENUOFFSET_SETCONVBONEMODEL;
			s_convbone_model = s_modelindex[modelindex].modelptr;

			WCHAR strmes[1024];
			if (!s_convbone_model) {
				swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
				::DSMessageBox(NULL, strmes, L"check!!!", MB_OK);
			}
			else {
				swprintf_s(strmes, 1024, L"%s", s_convbone_model->GetFileName());
				s_cbselmodel->setName(strmes);
			}
		}


		else if ((menuid >= (ID_RMENU_0 + MENUOFFSET_SETCONVBONEBVH)) && (menuid < (ID_RMENU_0 + modelnum + MENUOFFSET_SETCONVBONEBVH))) {
			int modelindex = menuid - ID_RMENU_0 - MENUOFFSET_SETCONVBONEBVH;
			s_convbone_bvh = s_modelindex[modelindex].modelptr;
			WCHAR strmes[1024];
			if (!s_convbone_bvh) {
				swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
				::DSMessageBox(NULL, strmes, L"check!!!", MB_OK);
				s_maxboneno = 0;
			}
			else {
				swprintf_s(strmes, 1024, L"%s", s_convbone_bvh->GetFileName());
				s_cbselbvh->setName(strmes);
				s_maxboneno = s_convbone_bvh->GetBoneListSize();
			}
		}


		else if ((menuid >= (ID_RMENU_0 + MENUOFFSET_SETCONVBONE)) && (menuid <= (ID_RMENU_0 + s_maxboneno + 1 + MENUOFFSET_SETCONVBONE))) {
			if (menuid == (ID_RMENU_0 + 0 + MENUOFFSET_SETCONVBONE)) {
				//未設定
				s_bvhbone_bone[s_bvhbone_cbno] = 0;
				CBone* modelbone = s_modelbone_bone[s_bvhbone_cbno];
				_ASSERT(modelbone);
				if (modelbone) {
					s_convbonemap[modelbone] = 0;
				}
				s_bvhbone[s_bvhbone_cbno]->setName(L"NotSet");
			}
			else {
				int boneno = menuid - ID_RMENU_0 - 1 - MENUOFFSET_SETCONVBONE;
				curbone = s_convbone_bvh->GetBoneByID(boneno);
				WCHAR strmes[1024];
				if (!curbone) {
					s_bvhbone_bone[s_bvhbone_cbno] = 0;
					CBone* modelbone = s_modelbone_bone[s_bvhbone_cbno];
					_ASSERT(modelbone);
					if (modelbone) {
						s_convbonemap[modelbone] = 0;
					}
					s_bvhbone[s_bvhbone_cbno]->setName(L"NotSet");

					swprintf_s(strmes, 1024, L"convbone : sel bvh bone : curbone NULL !!!");
					::DSMessageBox(NULL, strmes, L"check!!!", MB_OK);
				}
				else {
					swprintf_s(strmes, 1024, L"%s", curbone->GetWBoneName());
					s_bvhbone[s_bvhbone_cbno]->setName(strmes);
					s_bvhbone_bone[s_bvhbone_cbno] = curbone;

					CBone* modelbone = s_modelbone_bone[s_bvhbone_cbno];
					if (modelbone) {
						s_convbonemap[modelbone] = curbone;
					}
				}
			}
		}



		//else if ((menuid >= (ID_RMENU_0 + MENUOFFSET_INITMPFROMTOOL)) && (menuid <= (ID_RMENU_0 + 3 * 3 + MENUOFFSET_INITMPFROMTOOL))) {
		else if ((menuid >= (ID_RMENU_0 + MENUOFFSET_INITMPFROMTOOL)) && (menuid <= (ID_RMENU_0 + 3 * 4 + MENUOFFSET_INITMPFROMTOOL))) {//### 2022/07/04
			int subid = (menuid - ID_RMENU_0 - MENUOFFSET_INITMPFROMTOOL) / 4;//4 * 3 / 4 --> 0, 1, 2
			//int initmode = (menuid - ID_RMENU_0 - MENUOFFSET_INITMPFROMTOOL) - subid * 4;//0, 1, 2, 3
			int initmode = (menuid - ID_RMENU_0 - MENUOFFSET_INITMPFROMTOOL) % 4;//0, 1, 2, 3 //### 2022/07/04
			MOTINFO* mi = s_model->GetCurMotInfo();
			if (mi) {
				s_copymotvec.clear();
				s_copyKeyInfoList.clear();
				s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();
				s_editrange.SetRange(s_copyKeyInfoList, s_owpTimeline->getCurrentTime());

				if (subid == 0) {
					list<KeyInfo>::iterator itrcp;
					for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++) {
						double curframe = itrcp->time;
						CBone* topbone = s_model->GetTopBone();
						if (topbone) {
							InitMpByEulReq(initmode, topbone, mi->motid, curframe);//topbone req
						}
					}
				}
				else if (subid == 1) {
					curbone = s_model->GetBoneByID(s_curboneno);
					if (curbone) {
						list<KeyInfo>::iterator itrcp;
						for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++) {
							double curframe = itrcp->time;
							InitMpByEul(initmode, curbone, mi->motid, curframe);//curbone
						}
					}
				}
				else if (subid == 2) {
					curbone = s_model->GetBoneByID(s_curboneno);
					if (curbone) {
						list<KeyInfo>::iterator itrcp;
						for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++) {
							double curframe = itrcp->time;
							InitMpByEulReq(initmode, curbone, mi->motid, curframe);//curbone req
						}
					}
				}
			}
			UpdateEditedEuler();
		}




		else if (menuid == (ID_RMENU_PHYSICSCONSTRAINT + MENUOFFSET_BONERCLICK)) {
			//位置コンストレイントはMass0で実現する。
			////toggle
			//if (curbone->GetPosConstraint() == 0){
			//	s_model->CreatePhysicsPosConstraint(curbone);
			//}
			//else{
			//	s_model->DestroyPhysicsPosConstraint(curbone);
			//}
		}
		else if (menuid == (ID_RMENU_KINEMATIC_ON_LOWER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->SetKinematicTmpLower(curbone, true);
			}
		}
		else if (menuid == (ID_RMENU_KINEMATIC_OFF_LOWER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->SetKinematicTmpLower(curbone, false);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_ON_ALL + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_All(true);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_OFF_ALL + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_All(false);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_ON_UPPER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_Upper(true, curbone);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_OFF_UPPER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_Upper(false, curbone);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_ON_LOWER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_Lower(true, curbone);
			}
		}
		else if (menuid == (ID_RMENU_MASS0_OFF_LOWER + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				s_model->Mass0_Lower(false, curbone);
			}
		}
		else if (menuid == (ID_RMENU_MASS0 + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				//toggle
				if (curbone->GetMass0() == 0) {
					s_model->SetMass0(curbone);
				}
				else {
					s_model->RestoreMass(curbone);
				}
			}
		}
		else if (menuid == (ID_RMENU_EXCLUDE_MV + MENUOFFSET_BONERCLICK)) {
			if (curbone) {
				//toggle
				if (curbone->GetExcludeMv() == 0) {
					curbone->SetExcludeMv(1);
				}
				else {
					curbone->SetExcludeMv(0);
				}
			}
		}
		else if (menuid == (ID_RMENU_FORBIDROT_ONE + MENUOFFSET_BONERCLICK)) {
			if (curre) {
				curre->SetForbidRotFlag(1);
			}
		}
		else if (menuid == (ID_RMENU_ENABLEROT_ONE + MENUOFFSET_BONERCLICK)) {
			if (curre) {
				curre->SetForbidRotFlag(0);
			}
		}
		else if (menuid == (ID_RMENU_FORBIDROT_CHILDREN + MENUOFFSET_BONERCLICK)) {
			if (curre) {
				s_model->EnableRotChildren(curbone, false);
			}
		}
		else if (menuid == (ID_RMENU_ENABLEROT_CHILDREN + MENUOFFSET_BONERCLICK)) {
			if (curre) {
				s_model->EnableRotChildren(curbone, true);
			}
		}
		else if (menuid == (ID_RMENU_0 + MENUOFFSET_BONERCLICK)) {
			//新規
			GUIMenuSetVisible(-1, -1);
			int currigno = -1;
			DispCustomRigDlg(currigno);
		}
		else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM + MENUOFFSET_BONERCLICK)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 2 + MENUOFFSET_BONERCLICK))) {
			//設定
			GUIMenuSetVisible(-1, -1);
			int currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM) - MENUOFFSET_BONERCLICK];
			DispCustomRigDlg(currigno);
		}
		else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM * 2 + MENUOFFSET_BONERCLICK)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 3 + MENUOFFSET_BONERCLICK))) {
			//実行
			int currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM * 2) - MENUOFFSET_BONERCLICK];
			Bone2CustomRig(currigno);
			if (s_customrigbone) {
				s_oprigflag = 1;
			}
		}
		else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM * 3 + MENUOFFSET_BONERCLICK)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 4 + MENUOFFSET_BONERCLICK))) {
			//削除(無効化)
			int currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM * 3) - MENUOFFSET_BONERCLICK];
			InvalidateCustomRig(currigno);
		}


		if ((menuid >= (ID_RMENU_0 + MENUOFFSET_GETSYMROOTMODE)) && (menuid <= (ID_RMENU_0 + 3 + MENUOFFSET_GETSYMROOTMODE))) {
			switch (menuid) {
			case (ID_RMENU_0 + MENUOFFSET_GETSYMROOTMODE):
				s_getsym_retmode = SYMROOTBONE_SAMEORG;
				break;
			case (ID_RMENU_0 + 1 + MENUOFFSET_GETSYMROOTMODE):
				s_getsym_retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
				break;
			case (ID_RMENU_0 + 2 + MENUOFFSET_GETSYMROOTMODE):
				s_getsym_retmode = SYMROOTBONE_SYMDIR;
				break;
			case (ID_RMENU_0 + 3 + MENUOFFSET_GETSYMROOTMODE):
				s_getsym_retmode = SYMROOTBONE_SYMPOS;
				break;
			default:
				s_getsym_retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
				break;
			}
		}






		else if( (menuid >= 59900) && (menuid <= (59900 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 59900;
			OnAnimMenu( true, selindex );
			ActivatePanel( 1 );
			//return 0;
		}else if( (menuid >= 61000) && (menuid <= (61000 + MAXMODELNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 61000;
			OnModelMenu( true, selindex, 1 );
			ActivatePanel( 1 );
			//return 0;
		}else if( (menuid >= 62000) && (menuid <= (62000 + MAXRENUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 62000;
			OnREMenu( selindex, 1 );
			ActivatePanel( 1 );
			//return 0;
		}else if( (menuid >= 63000) && (menuid <= (63000 + MAXRENUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 63000;
			OnRgdMenu( selindex, 1 );
			ActivatePanel( 1 );
			//return 0;
		}else if( (menuid >= 64000) && (menuid <= (64000 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 64000;
			OnRgdMorphMenu( selindex );
			ActivatePanel( 1 );
			//return 0;
		}else if( (menuid >= 64500) && (menuid <= (64500 + MAXMOTIONNUM)) ){
			ActivatePanel( 0 );
			int selindex = menuid - 64500;
			OnImpMenu( selindex );
			ActivatePanel( 1 );
			//return 0;
		}else{
			switch( menuid ){
			case ID_40047:
				// "編集・変換"
				// "ボーン軸をXに再計算"
				ActivatePanel(0);
				//RecalcBoneAxisX(0);
				RecalcAxisX_All();
				ActivatePanel(1);
				//return 0;
				break;
			case 29800:
				ActivatePanel( 0 );
				//RegistKey();
				AboutMotionBrush();
				ActivatePanel( 1 );
				//return 0;
				break;
			case ID_FILE_EXPORTBNT:
				ActivatePanel( 0 );
				ExportBntFile();
				ActivatePanel( 1 );
				//return 0;
				break;
			case ID_FILE_OPEN40001:
				ActivatePanel( 0 );
				OpenFile();
				ActivatePanel( 1 );
				//return 0;
				break;
			case ID_FILE_BVH2FBX:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					BVH2FBX();
					ActivatePanel( 1 );
				}
				//return 0;
				break;
			case ID_FILE_BVH2FBXBATCH:
				if (s_registflag == 1) {
					ActivatePanel(0);
					BVH2FBXBatch();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_FILE_RETARGETBATCH:
				if (s_registflag == 1) {
					ActivatePanel(0);
					RetargetBatch();
					ActivatePanel(1);
				}
				//return 0;
				break;

/***
			case ID_SAVE_FBX40039:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					ExportFBXFile();
					ActivatePanel( 1 );
				}
				break;
***/
			case ID_SAVEPROJ_40035:
				//FBX Fileは201300で書き出し
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
				//return 0;
				break;
			case ID_IMPSAVE_40030:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveImpFile();
					ActivatePanel( 1 );
				}
				//return 0;
				break;
			case ID_SAVEGCOLI_40033:
				if( s_registflag == 1 ){
					ActivatePanel( 0 );
					SaveGcoFile();
					ActivatePanel( 1 );
				}
				//return 0;
				break;
			case ID_SAVE_MOTIONNAMELIST:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveMotionNameListFile();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_DISPMW40002:
				DispMotionWindow();
				//return 0;
				break;
			case 4007:
				DispToolWindow();
				//return 0;
				break;
			case 40012:
			{
				bool savedispflag = s_dispobj;
				s_dispobj = !savedispflag;
				DispObjPanel();
				//return 0;
			}
				break;
			case ID_40048:
				//DispConvBoneWindow();
				s_platemenukind = 2;
				GUIMenuSetVisible(s_platemenukind, 1);
				//return 0;
				break;
			case ID_40049:
				//DispAngleLimitDlg();
				s_platemenukind = 2;
				GUIMenuSetVisible(s_platemenukind, 2);
				//return 0;
				break;
			case ID_40050:
				DispRotAxisDlg();
				//return 0;
				break;
			case ID_DISPMODELPANEL:
			{
				bool savedispflag = s_dispmodel;
				s_dispmodel = !savedispflag;
				DispModelPanel();
				//return 0;
			}
				break;
			case ID_MOTIONPANEL:
			{
				bool savedispflag = s_dispmotion;
				s_dispmotion = !savedispflag;
				DispMotionPanel();
				//return 0;
			}
				break;
			case ID_SETTINGS:
			{
				CSettingsDlg dlg;
				dlg.DoModal();
			}
				break;

			case ID_DISPGROUND:
				s_dispground = !s_dispground;
				//return 0;
				break;
			case ID_NEWMOT:
				AddMotion( 0 );
				InitCurMotion(0, 0);
				//return 0;
				break;
			case ID_DELCURMOT:
				if (s_model) {
					OnDelMotion(s_motmenuindexmap[s_model]);
				}
				//return 0;
				break;
			case ID_DELMODEL:
				OnDelModel( s_curmodelmenuindex );
				//return 0;
				break;
			case ID_DELALLMODEL:
				OnDelAllModel();
				//return 0;
				break;
			default:
				break;
			}
		}
	}else if( uMsg == WM_MOUSEWHEEL ){
		if ((g_keybuf['T'] & 0x80) != 0) {
			if (s_model && (s_curboneno > 0)) {
				s_tkeyflag = 1;

				int delta;
				delta = GET_WHEEL_DELTA_WPARAM(wParam);
				s_editmotionflag = s_model->TwistBoneAxisDelta(&s_editrange, s_curboneno, (float)delta, g_iklevel, s_ikcnt, s_ikselectmat);
			}
		}

		/*
		int delta;
		delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (g_underselectingframe == 1){
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
		//s_curboneno = -1;

		s_ikcnt = 0;
		s_rectime = 0.0;
		s_reccnt = 0;

		//!!!!!!!!!!!!!!!!!!!!!!!!!!
		//DS deviceがあっても、マウスを併用する場合があるのでマウスのSetCaptureとReleaseCaptureは必要
		//!!!!!!!!!!!!!!!!!!!!!!!!!!

		//if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
			//DS deviceが無い場合
			SetCapture(s_3dwnd);
		//}
		//SetCapture( s_3dwnd );


		POINT ptCursor;
		GetCursorPos( &ptCursor );
		::ScreenToClient( s_3dwnd, &ptCursor );
		s_pickinfo.clickpos = ptCursor;
		s_pickinfo.mousepos = ptCursor;
		s_pickinfo.mousebefpos = ptCursor;
		s_pickinfo.diffmouse = ChaVector2( 0.0f, 0.0f );
		s_pickinfo.firstdiff = ChaVector2( 0.0f, 0.0f );

		s_pickinfo.winx = (int)DXUTGetWindowWidth();
		s_pickinfo.winy = (int)DXUTGetWindowHeight();
		s_pickinfo.pickrange = PICKRANGE;

		s_pickinfo.pickobjno = -1;

		int platemenukind = s_platemenukind;
		int nextplatemenukind = 0;
		int nextplateno = 0;
		

		//if (s_dispmodel && s_modelpanel.panel && s_modelpanel.separator) {
		//	
		//	//SetCapture(s_modelpanel.panel->getHWnd());//!!!!!!!!!!!!
		//	POINT tmppos;
		//	GetCursorPos(&tmppos);
		//	LPARAM panallparam;
		//	panallparam = (tmppos.y << 16) | tmppos.x;
		//	SendMessage(s_modelpanel.panel->getHWnd(), WM_LBUTTONDOWN, MK_LBUTTON, panallparam);
		//}

		//check and op rigflag : s_oprigflag turn to 1 when RClickRigMenu selected too.
		{
			//IK Mode
			int pickikmodeflag = 0;
			pickikmodeflag = PickSpIkModeSW(ptCursor);
			if (pickikmodeflag == 1) {
				s_ikkind = 0;
				s_spikmodesw[0].state = true;
				s_spikmodesw[1].state = false;
				s_spikmodesw[2].state = false;
				refreshEulerGraph();
			}
			else if (pickikmodeflag == 2) {
				s_ikkind = 1;
				s_spikmodesw[0].state = false;
				s_spikmodesw[1].state = true;
				s_spikmodesw[2].state = false;
				refreshEulerGraph();
			}
			else if (pickikmodeflag == 3) {
				s_ikkind = 2;
				s_spikmodesw[0].state = false;
				s_spikmodesw[1].state = false;
				s_spikmodesw[2].state = true;
				refreshEulerGraph();
			}
		}
		{
			//RefPos switch
			int pickrefposflag = 0;
			pickrefposflag = PickSpRefPosSW(ptCursor);
			if (pickrefposflag == 1) {
				s_sprefpos.state = !s_sprefpos.state;
			}
		}

		int oprigdoneflag = 0;
		int pickrigflag = 0;
		pickrigflag = PickSpRig(ptCursor);
		if (pickrigflag == 1) {


			//開いている設定ダイアログを閉じないと、設定ダイアログのrigboneと新たなrigboneが異なってしまい、Applyボタンで異なるリグを保存することがある
			if (s_customrigdlg) {
				DestroyWindow(s_customrigdlg);
				s_customrigdlg = 0;
			}


			if (s_oprigflag == 1) {
				
				//オンだったRigをオフにする
				RollbackCurBoneNo();
				ToggleRig();
				oprigdoneflag = 1;
				
				s_curboneno = -1;//Sprite Menuより後で。Rigid作成に選択済s_curbonenoが必要。
			}
			else {

				//オフだったリグをオンにする
				RollbackCurBoneNo();

				ToggleRig();
				oprigdoneflag = 1;
				//s_oprigflag == 1 のときのpickrigflag == 0　だから　rigはオン
				//s_curboneno はそのまま
			}
		}


		//menukind : from 0 to 4
		//plateno : from 1 to platenum
		GUIGetNextMenu(ptCursor, platemenukind, &nextplatemenukind, &nextplateno);
		if ((nextplatemenukind >= 0) && (nextplateno != 0)) {
			s_platemenukind = nextplatemenukind;
			s_platemenuno = nextplateno;
			GUIMenuSetVisible(s_platemenukind, nextplateno);
		}

		//s_curboneno = -1;//Sprite Menuより後で。Rigid作成に選択済s_curbonenoが必要。
		
		int spckind = 0;
		if (s_spguisw[SPGUISW_SPRITEFK].state && ((spckind = PickSpCam(ptCursor)) != 0)) {
			s_pickinfo.buttonflag = spckind;
			s_pickinfo.pickobjno = -1;
			RollbackCurBoneNo();
		}else if (s_model){
			int spakind = 0;
			//int pickrigflag = 0;
			if (s_spguisw[SPGUISW_SPRITEFK].state) {
				spakind = PickSpAxis(ptCursor);
				//pickrigflag = PickSpRig(ptCursor);
			}
			if ((spakind != 0) && (s_saveboneno >= 0)){
				RollbackCurBoneNo();
				s_pickinfo.buttonflag = spakind;
				s_pickinfo.pickobjno = s_curboneno;

			//} else if ((oprigdoneflag == 0) && (pickrigflag == 1)){
			//	RollbackCurBoneNo();
			//	ToggleRig();

			}else{
				if (s_oprigflag == 0){
					if (g_shiftkey == false){
						CallF(s_model->PickBone(&s_pickinfo), return 1);
					}
					if (s_pickinfo.pickobjno >= 0){
						s_curboneno = s_pickinfo.pickobjno;//!!!!!!!

						if (s_owpTimeline){
							s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
						}

						ChangeCurrentBone();

						
						//if (s_model->GetInitAxisMatX() == 0){//OnAnimMenuに移動
						//	s_owpLTimeline->setCurrentTime(0.0, true);
						//	s_owpEulerGraph->setCurrentTime(0.0, false);
						//	s_model->SetMotionFrame(0.0);
						//	s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
						//	//ここでAxisMatXの初期化
						//	s_model->CreateBtObject(1);
						//	s_model->CalcBtAxismat(2);//2
						//	s_model->SetInitAxisMatX(1);
						//}

						s_pickinfo.buttonflag = PICK_CENTER;//!!!!!!!!!!!!!

						//CModel::PickBone内でセット
						//s_pickinfo.firstdiff.x = (float)s_pickinfo.clickpos.x - s_pickinfo.objscreen.x;
						//s_pickinfo.firstdiff.y = (float)s_pickinfo.clickpos.y - s_pickinfo.objscreen.y;

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
					int pickrigboneno = PickRigBone(&s_pickinfo);
					if (pickrigboneno < 0) {
						s_curboneno = -1;
						s_customrigbone = 0;

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
			curbone = s_model->GetBoneByID( s_curboneno );
			_ASSERT( curbone );
			if (curbone){
				s_saveboneno = s_curboneno;

				if (s_camtargetflag){
					g_befcamtargetpos = g_camtargetpos;
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


		//pickでボーン選択が変わらなかったとき。モーションフレーム選択をした場合など。
		if ((s_curboneno <= 0) && (s_saveboneno > 0)) {
			s_curboneno = s_saveboneno;
		}


		g_Camera->SetViewParams(g_camEye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));//!!!!!!!!!!
		//g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));//!!!!!!!!!!
		ChaVector3 diffv = g_camEye - g_camtargetpos;
		s_camdist = (float)ChaVector3LengthDbl( &diffv );


		//if (s_model && (s_pickinfo.pickobjno >= 0) && (g_previewFlag == 5)){
		if (s_model && (g_previewFlag == 5)) {
			if ((s_pickinfo.pickobjno >= 0)){// && 
				//((s_spguisw[SPGUISW_SPRITEFK].state == false)) || (PickSpBt(ptCursor) == 0))){//物理IK中でジョイントをクリックしていて、Applyボタンを押していないとき
				StartBt(s_model, TRUE, 1, 1);
				//s_model->BulletSimulationStart();
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

		OnMouseMoveFunc();

	}else if( uMsg == WM_LBUTTONUP ){


		//if (s_dispmodel && s_modelpanel.panel && s_modelpanel.separator) {
		//	POINT tmppos;
		//	GetCursorPos(&tmppos);
		//	LPARAM panallparam;
		//	panallparam = (tmppos.y << 16) | tmppos.x;
		//	SendMessage(s_modelpanel.panel->getHWnd(), WM_LBUTTONUP, MK_LBUTTON, panallparam);
		//}


		//!!!!!!!!!!!!!!!!!!!!!!!!!!
		//DS deviceがあっても、マウスを併用することがあるのでマウスのSetCaptureとReleaseCaptureは必要
		//!!!!!!!!!!!!!!!!!!!!!!!!!!

		//if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
			//DS deviceが無い場合
			ReleaseCapture();
		//}
		//ReleaseCapture();
		s_wmlbuttonup = 1;


		if (s_model && (s_onragdollik != 0)){
			//s_model->BulletSimulationStop();
			//s_model->SetBtKinFlagReq(s_model->GetTopBt(), 1);
			
			s_model->BulletSimulationStop();
			g_previewFlag = 0;
			s_model->ApplyPhysIkRec();
		}

		UpdateEditedEuler();

		s_pickinfo.buttonflag = 0;
		s_ikcnt = 0;
		s_onragdollik = 0;


		PrepairUndo();


	}else if( uMsg == WM_RBUTTONDOWN ){
		
		BoneRClick(-1);

	}else if( uMsg == WM_RBUTTONUP ){
		//ReleaseCapture();
		s_pickinfo.buttonflag = 0;
	}else if( uMsg == WM_MBUTTONUP ){
		//ReleaseCapture();
		s_pickinfo.buttonflag = 0;
	}


	/*
	if( uMsg == WM_LBUTTONDOWN ){
		g_Camera->HandleMessages( hWnd, WM_RBUTTONDOWN, wParam, lParam );
		if( s_ikkind == 2 ){
			g_LightControl[g_nActiveLight].HandleMessages( hWnd, WM_RBUTTONDOWN, wParam, lParam );
		}
	}else if( uMsg == WM_LBUTTONDBLCLK ){
	}else if( uMsg == WM_LBUTTONUP ){
		g_Camera->HandleMessages( hWnd, WM_RBUTTONUP, wParam, lParam );
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
		g_Camera->HandleMessages( hWnd, uMsg, wParam, lParam );
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
			DefWindowProc( s_3dwnd, uMsg, wParam, lParam );
			ActivatePanel( 1 );
			//return 1;//!!!!!!!!!!!!!
			break;
		}
	}


	//if ((s_doneinit == 1) && (g_undertrackingRMenu == 1)) {
	//	OnDSUpdate();
	//}


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
	//int tmpboneno;
	//CBone* tmpbone;
	WCHAR sz[100];
	ChaVector3 weye;
	float trastep = s_totalmb.r * 0.05f;
	int modelnum = (int)s_modelindex.size();
	//int modelno;
	//int tmpikindex;

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
			s_calclimitedwmState = 101;
			break;
		case IDC_BTRECSTART:
		{
			if (g_motionbrush_numframe < 10) {
				WCHAR strmes[1024] = {0L};
				swprintf_s(strmes, 1024, L"複数フレームを選択してから再試行してください。\nRetry after selecting frames range.");
				::DSMessageBox(NULL, strmes, L"error!!!", MB_OK);
			}
			else {
				s_calclimitedwmState = 1001;
			}
		}
			break;
		case IDC_PHYSICS_IK:
			s_physicskind = 0;
			s_savelimitdegflag = g_limitdegflag;
			g_limitdegflag = false;
			if (s_LimitDegCheckBox) {
				s_LimitDegCheckBox->SetChecked(g_limitdegflag);
			}
			StartBt(s_model, TRUE, 1, 1);
			break;
		case IDC_PHYSICS_MV_IK:
			s_physicskind = 1;
			s_savelimitdegflag = g_limitdegflag;
			g_limitdegflag = false;
			if (s_LimitDegCheckBox) {
				s_LimitDegCheckBox->SetChecked(g_limitdegflag);
			}
			StartBt(s_model, TRUE, 1, 1);
			break;
		case IDC_PHYSICS_MV_SLIDER:
			RollbackCurBoneNo();
			g_physicsmvrate = (float)(g_SampleUI.GetSlider(IDC_PHYSICS_MV_SLIDER)->GetValue()) * 0.01f;
			swprintf_s(sz, 100, L"EditRate : %.3f", g_physicsmvrate);
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
		case IDC_PHYSICS_IK_STOP:
			StopBt();
			//g_limitdegflag = s_savelimitdegflag;//StopBt内で呼ぶ
			break;

        case IDC_LIGHT_SCALE:
			RollbackCurBoneNo();
			g_fLightScale = (float)(g_SampleUI.GetSlider(IDC_LIGHT_SCALE)->GetValue() * 0.10f);

            swprintf_s( sz, 100, L"Light : %0.2f", g_fLightScale );
            //g_SampleUI.GetStatic( IDC_LIGHT_SCALE_STATIC )->SetText( sz );
            break;

        case IDC_SL_IKFIRST:
			//RollbackCurBoneNo();
			//g_ikfirst = (float)(g_SampleUI.GetSlider(IDC_SL_IKFIRST)->GetValue()) * 0.04f;
		 //   swprintf_s( sz, 100, L"IK Order : %f", g_ikfirst );
   //         g_SampleUI.GetStatic( IDC_STATIC_IKFIRST )->SetText( sz );
            break;
   //     case IDC_SL_IKRATE:
			//RollbackCurBoneNo();
			//g_ikrate = (float)(g_SampleUI.GetSlider(IDC_SL_IKRATE)->GetValue()) * 0.01f;
		 //   swprintf_s( sz, 100, L"IK Trans : %f", g_ikrate );
   //         g_SampleUI.GetStatic( IDC_STATIC_IKRATE )->SetText( sz );
   //         break;
		case IDC_SL_REFPOS:
		{
			RollbackCurBoneNo();
			g_refpos = g_SampleUI.GetSlider(IDC_SL_REFPOS)->GetValue();
			double refframe = CalcRefFrame();
			if (refframe >= 0.0) {
				swprintf_s(sz, 100, L"RefPos : %d%% : %d", g_refpos, (int)refframe);
			}
			else {
				swprintf_s(sz, 100, L"RefPos : %d%%", g_refpos);
			}
			g_SampleUI.GetStatic(IDC_STATIC_REF)->SetText(sz);
			//CEditRange::SetApplyRate((double)g_refpos);
			//OnTimeLineSelectFromSelectedKey();
			//if (s_editmotionflag < 0) {
			//	int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
			//	if (result) {
			//		_ASSERT(0);
			//	}
			//}
		}
			break;
		case IDC_SL_REFMULT:
		{
			RollbackCurBoneNo();
			g_refmult = g_SampleUI.GetSlider(IDC_SL_REFMULT)->GetValue();
		}
		break;
		case IDC_SL_NUMTHREAD:
			RollbackCurBoneNo();
			g_numthread = (int)(g_SampleUI.GetSlider(IDC_SL_NUMTHREAD)->GetValue());
			swprintf_s(sz, 100, L"ThreadNum:%d(%d)", g_numthread, gNumIslands);
			g_SampleUI.GetStatic(IDC_STATIC_NUMTHREAD)->SetText(sz);
			s_bpWorld->setNumThread(g_numthread);
			break;
		case IDC_SL_BRUSHREPEATS:
			RollbackCurBoneNo();
			g_brushrepeats = (int)(g_SampleUI.GetSlider(IDC_SL_BRUSHREPEATS)->GetValue());
			swprintf_s(sz, 100, L"Brush Repeats : %d", g_brushrepeats);
			g_SampleUI.GetStatic(IDC_STATIC_BRUSHREPEATS)->SetText(sz);
			if (s_editmotionflag < 0) {
				int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
				if (result) {
					_ASSERT(0);
				}
			}
			break;
		case IDC_BRUSH_MIRROR_U:
			if (s_BrushMirrorUCheckBox) {
				g_brushmirrorUflag = (int)s_BrushMirrorUCheckBox->GetChecked();
				if (s_editmotionflag < 0) {
					int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
					if (result) {
						_ASSERT(0);
					}
				}
			}
			break;
		case IDC_BRUSH_MIRROR_V:
			if (s_BrushMirrorVCheckBox) {
				g_brushmirrorVflag = (int)s_BrushMirrorVCheckBox->GetChecked();
				if (s_editmotionflag < 0) {
					int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
					if (result) {
						_ASSERT(0);
					}
				}
			}
			break;
		case IDC_BRUSH_MIRROR_V_DIV2:
			if (s_IfMirrorVDiv2CheckBox) {
				g_ifmirrorVDiv2flag = (int)s_IfMirrorVDiv2CheckBox->GetChecked();
				if (s_editmotionflag < 0) {
					int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
					if (result) {
						_ASSERT(0);
					}
				}
			}
			break;
		case IDC_SL_APPLYRATE:
			RollbackCurBoneNo();
			g_applyrate = g_SampleUI.GetSlider(IDC_SL_APPLYRATE)->GetValue();
			CEditRange::SetApplyRate((double)g_applyrate);
			swprintf_s( sz, 100, L"TopPos : %d%% : %d", g_applyrate, (int)(s_editrange.GetApplyFrame()) );
            g_SampleUI.GetStatic( IDC_STATIC_APPLYRATE )->SetText( sz );
			OnTimeLineSelectFromSelectedKey();
			if (s_editmotionflag < 0) {
				int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
				if (result) {
					_ASSERT(0);
				}
			}
            break;

        case IDC_SPEED:
			OnGUIEventSpeed();
			break;
		case IDC_BTCALCCNT:
			RollbackCurBoneNo();
			g_btcalccnt = (double)(g_SampleUI.GetSlider(IDC_BTCALCCNT)->GetValue());

			swprintf_s(sz, 100, L"BT CalcCnt: %0.2f", g_btcalccnt);
			g_SampleUI.GetStatic(IDC_STATIC_BTCALCCNT)->SetText(sz);
			break;

		case IDC_ERP:
			RollbackCurBoneNo();
			g_erp = (double)(g_SampleUI.GetSlider(IDC_ERP)->GetValue() * 0.0002);

			swprintf_s(sz, 100, L"BT ERP: %0.5f", g_erp);
			g_SampleUI.GetStatic(IDC_STATIC_ERP)->SetText(sz);
			break;
		case IDC_COMBO_MOTIONBRUSH_METHOD:
			RollbackCurBoneNo();
			if (s_model) {
				pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_MOTIONBRUSH_METHOD);
				g_motionbrush_method = (int)PtrToUlong(pComboBox->GetSelectedData());
				if (s_editmotionflag < 0) {
					int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
					if (result) {
						_ASSERT(0);
					}
				}
			}
			break;
		case IDC_COMBO_BONEAXIS:
			RollbackCurBoneNo();
			if (s_model){
				pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_BONEAXIS);
				g_boneaxis = (int)PtrToUlong(pComboBox->GetSelectedData());

			}
			break;
		case IDC_COMBO_BONE:
			//RollbackCurBoneNo();
			//if (s_model){
			//	pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
			//	tmpboneno = (int)PtrToUlong( pComboBox->GetSelectedData() );
			//	tmpbone = s_model->GetBoneByID( tmpboneno );
			//	if( tmpbone ){
			//		s_curboneno = tmpboneno;
			//	}

			//	if( s_curboneno >= 0 ){
			//		int lineno = s_boneno2lineno[ s_curboneno ];
			//		if( lineno >= 0 ){
			//			s_owpTimeline->setCurrentLine( lineno, true );					
			//		}
			//	}
			//}
			break;

		//case IDC_COMBO_EDITMODE:
		//	if( s_model ){
		//		RollbackCurBoneNo();
		//		pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_EDITMODE);
		//		tmpikindex = (int)PtrToUlong(pComboBox->GetSelectedData());

		//		switch( tmpikindex ){
		//		case IDC_IK_ROT:
		//			s_ikkind = 0;
		//			refreshEulerGraph();
		//			break;
		//		case IDC_IK_MV:
		//			s_ikkind = 1;
		//			refreshEulerGraph();
		//			break;
		//		case IDC_IK_SC:
		//			s_ikkind = 2;
		//			refreshEulerGraph();
		//			break;
		//		//case IDC_IK_LIGHT:
		//		//	s_ikkind = 2;
		//		//	s_displightarrow = true;
		//		//	if (s_LightCheckBox){
		//		//		s_LightCheckBox->SetChecked(true);
		//		//	}
		//		//	break;
		//		case IDC_BT_RIGID:
		//			GUIMenuSetVisible(1, 1);
		//			break;
		//		case IDC_BT_IMP:
		//			GUIMenuSetVisible(1, 2);
		//			break;
		//		case IDC_BT_GP:
		//			GUIMenuSetVisible(1, 3);
		//			break;
		//		case IDC_BT_DAMP:
		//			GUIMenuSetVisible(1, 4);
		//			break;
		//		default:
		//			break;
		//		}
		//	}
		//	break;

		case IDC_COMBO_IKLEVEL:
			RollbackCurBoneNo();
			pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_IKLEVEL);
			g_iklevel = (int)PtrToUlong( pComboBox->GetSelectedData() );
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
//    g_DialogResourceManager.OnD3D11LostDevice();
//    g_SettingsDlg.OnD3D11LostDevice();
//    CDXUTDirectionWidget::StaticOnD3D11LostDevice();
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
//    g_DialogResourceManager.OnD3D11DestroyDevice();
//    g_SettingsDlg.OnD3D11DestroyDevice();
//    CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
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
	
	if (s_parentcheck) {
		delete s_parentcheck;
		s_parentcheck = 0;
	}

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
const D3D11_INPUT_ELEMENT_DESC layout[] =
{
{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

D3DX11_PASS_DESC PassDesc;
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

	unsigned int leng;
	ZeroMemory(g_basedir, sizeof(WCHAR)* MAX_PATH);
	wcscpy_s(g_basedir, MAX_PATH, filename);
	g_basedir[MAX_PATH - 1] = 0L;
	leng = (unsigned int)wcslen(g_basedir);
	if (wcscmp(g_basedir + leng - 1, L"\\") != 0){
		wcscat_s(g_basedir, MAX_PATH, L"\\");
	}


	DbgOut(L"SetBaseDir : %s\r\n", g_basedir);

	return 0;
}


int OpenMNLFile()
{
	if (!s_model) {
		return 0;
	}

	if (g_tmpmqopath[0] == 0) {
		return 0;
	}

	CMNLFile mnlfile;
	CallF(mnlfile.LoadMNLFile(g_tmpmqopath, s_model), return 1);

	CallF(OnAnimMenu(true, 0), return 1);

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
		::DSMessageBox( s_3dwnd, L"Overflow Loading(Limit under 99 files)", L"warning!!!", MB_OK );
		return 0;
	}

	CRigidElemFile refile;
	CallF( refile.LoadRigidElemFile( g_tmpmqopath, s_model ), return 1 );


	int renum = s_model->GetRigidElemInfoSize();
	if( renum > 0 ){
		
		//RgidElemFileのLoad時にしている
		//if (s_model && (renum == 1)) {//初回のref読み込み後にRigidElemを作成
		//	int chkret;
		//	chkret = s_model->CreateRigidElem();
		//	_ASSERT(!chkret);
		//}

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
		s_3dwnd, (DLGPROC)SaveGcoDlgProc );
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
	if( s_rgdindexmap[s_model] < 0 ){
		::DSMessageBox( s_3dwnd, L"Save Only RagdollImpulse.\nRetry after Setting of Ragdoll", L"error!!!", MB_OK );
		return 0;
	}


	int dlgret;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_SAVEIMPDLG ), 
		s_3dwnd, (DLGPROC)SaveImpDlgProc );
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
		s_3dwnd, (DLGPROC)SaveREDlgProc );
	if( (dlgret != IDOK) || !s_REname[0] ){
		return 0;
	}

	CRigidElemFile refile;
	CallF( refile.WriteRigidElemFile( s_REname, s_model, s_model->GetCurReIndex() ), return 1 );

	return 0;
}


void FindF(std::vector<wstring>& out, const wstring& directory, const wstring& findext)
{
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFile((directory + L"/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		return; /* No files found */
	}

	do
	{
		const wstring fileName = fileData.cFileName;
		const wstring fullFileName = directory + L"\\" + fileName;
		const bool isDirectory = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (fileName[0] == L'.')
			continue;

		if (!isDirectory)
		{
			//WCHAR pattern[20] = L".bvh";
			//WCHAR npattern[20] = L".bvh.";
			WCHAR pattern[20] = { 0L };
			WCHAR wpath[MAX_PATH] = { 0L };
			wcscpy_s(pattern, 20, findext.c_str());
			wcscpy_s(wpath, MAX_PATH, fullFileName.c_str());

			const WCHAR* pfind;
			pfind = wcsstr(wpath, pattern);
			if (pfind) {
				wpath[MAX_PATH - 1] = 0L;
				pattern[20 - 1] = 0L;
				int pathleng = (int)wcslen(wpath);
				int patternleng = (int)wcslen(pattern);
				if ((patternleng > 0) && (pathleng > 0) && (pathleng < MAX_PATH) && (pathleng > patternleng)) {
					WCHAR chkterm = *(pfind + patternleng);
					if (chkterm == 0L) {//patternの次の文字がNULLの場合
						out.push_back(fullFileName);
					}
				}
			}
		}
		else
		{
			FindF(out, fullFileName, findext);
		}

	} while (FindNextFile(dir, &fileData));

	FindClose(dir);

	return;
}


int CALLBACK BrowseCallbackProc(HWND   hWnd, UINT   uMsg, LPARAM lParam, LPARAM lpData) 
{
	WCHAR firstdir[MAX_PATH] = { 0L };

	switch (uMsg) {

		// 初期ディレクトリの設定
	case BFFM_INITIALIZED:
		firstdir[0] = 0L;
		GetBatchHistoryDir(firstdir, MAX_PATH);
		if(firstdir[0] != 0L){
			SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)firstdir);
			SendMessage(hWnd, BFFM_SETEXPANDED, (WPARAM)TRUE, (LPARAM)firstdir);
		}
		else {
			// ドキュメントが初期フォルダ
			if (lpData) {
				SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)lpData);
				SendMessage(hWnd, BFFM_SETEXPANDED, (WPARAM)TRUE, (LPARAM)lpData);
			}
		}
		//初期フォルダ（ドキュメント）を展開
		//SendMessage(hWnd, BFFM_SETEXPANDED, (WPARAM)TRUE, (LPARAM)lpData);
		break;

		// 無効なフォルダ名を入力された場合
	case BFFM_VALIDATEFAILED:
		MessageBox(NULL, L"無効なフォルダ名です", L"", MB_OK);
		return TRUE;
	}
	return FALSE;
}


//########### retarget batch　ここから

int RetargetFile(char* fbxpath)
{
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	char directorypath[MAX_PATH] = { 0 };
	ZeroMemory(directorypath, sizeof(char) * MAX_PATH);
	char* pfind;
	pfind = strrchr(fbxpath, '\\');
	if (pfind) {
		
		if (s_modelindex.size() > 0) {
			OnModelMenu(false, (int)s_modelindex.size() - 1, 1);
		}

		unsigned int dirpathlen;
		dirpathlen = (unsigned int)(pfind - fbxpath + 1);
		strncpy_s(directorypath, MAX_PATH, fbxpath, dirpathlen);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, fbxpath, MAX_PATH, g_tmpmqopath, MAX_PATH);
		CModel* newmodel = 0;
		newmodel = OpenFBXFile(false, 0, 1);
		if (newmodel) {
			//s_model = s_convbone_model_batch;


			OnModelMenu(false, s_convbone_model_batch_selindex, 1);
			s_model = s_convbone_model_batch;
			s_convbone_model = s_convbone_model_batch;
			s_convbone_bvh = newmodel;
			s_maxboneno = s_convbone_bvh->GetBoneListSize();

			WCHAR wretargetfilename[MAX_PATH] = { 0L };
			char retargetfilename[MAX_PATH] = { 0 };
			sprintf_s(retargetfilename, MAX_PATH, "%sretarget.rtg", directorypath);
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, retargetfilename, MAX_PATH, wretargetfilename, MAX_PATH);

			int result1;
			result1 = LoadRetargetFile(wretargetfilename);
			if (result1 == 0) {
				int result2;
				result2 = RetargetMotion();
				if (result2 != 0) {
					_ASSERT(0);
				}
			}

			//int modelindex = (int)s_modelindex.size() - 1;
			//OnDelModel(modelindex);
		}
	}
	return 0;
}


unsigned __stdcall ThreadFunc_CalcLimitedWM(LPVOID lpThreadParam)
{
	int modelcnt = 0;
	int modelnum = s_modelindex.size();


	InterlockedExchange(&s_progressmodelnum, modelnum);
	InterlockedExchange(&s_progressmodelcnt, 0);
	InterlockedExchange(&s_befprogressmodelnum, 0);
	InterlockedExchange(&s_befprogressmodelcnt, 0);


	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		modelcnt++;
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel) {
			MOTINFO* curmi;
			curmi = curmodel->GetCurMotInfo();
			if (!curmi) {
				continue;
			}

			InterlockedExchange(&s_progressmodelcnt, modelcnt);

			double frameleng = curmi->frameleng;
			InterlockedExchange(&s_progressnum, (LONG)frameleng);
			InterlockedExchange(&s_progresscnt, 0);
			if (s_progresswnd) {
				HWND hProg;
				hProg = GetDlgItem(s_progresswnd, IDC_PROGRESS1);
				if (hProg) {
					SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
					SendMessage(hProg, PBM_SETPOS, (WPARAM)0, 0);
					UpdateWindow(s_progresswnd);
				}
			}

			int dbgcnt = 0;
			double curframe;
			for (curframe = 0.0; curframe < frameleng; curframe += 1.0) {
				InterlockedExchange(&s_progresscnt, (LONG)curframe);

				curmodel->UpdateLimitedWM(curmi->motid, curframe);
			}

		}
	}

	if (s_progresswnd) {
		SendMessage(s_progresswnd, WM_CLOSE, 0, 0);
	}

	InterlockedExchange(&g_calclimitedwmflag, (LONG)2);//mark finished


	//##########################################
	//watch this thread at OnFrameStartPreview()
	//##########################################


	return 0;
}

int CalcLimitedWorldMat()
{
	if (!s_model) {
		return 0;
	}

	if (g_limitdegflag == 1) {

		if (InterlockedAdd(&g_calclimitedwmflag, 0) != 0) {//if already under calc, return 0.
			return 0;
		}


		InterlockedExchange(&g_calclimitedwmflag, (LONG)1);//mark started


		CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)ProgressDlgProc);
		RECT rect;
		GetWindowRect(s_LtimelineWnd->getHWnd(), &rect);
		SetWindowPos(s_progresswnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		ShowWindow(s_progresswnd, SW_SHOW);
		UpdateWindow(s_progresswnd);

		unsigned int threadaddr1 = 0;
		HANDLE hthread = (HANDLE)_beginthreadex(
			NULL, 0, &ThreadFunc_CalcLimitedWM,
			(void*)0,
			0, &threadaddr1);

		//WiatForしない場合には先に閉じてもOK
		if (hthread && (hthread != INVALID_HANDLE_VALUE)) {
			CloseHandle(hthread);
		}

	}
	else {

		InterlockedExchange(&g_calclimitedwmflag, (LONG)2);//mark finished
	}

	return 0;
}





unsigned __stdcall ThreadFunc_Retarget(LPVOID lpThreadParam)
{

	int outnum = (int)s_retargetout.size();

	InterlockedExchange(&s_retargetnum, (LONG)outnum);
	InterlockedExchange(&s_retargetcnt, 0);
	InterlockedExchange(&s_befretargetnum, 0);
	InterlockedExchange(&s_befretargetcnt, 0);

	int outcnt;
	for (outcnt = 0; outcnt < outnum; outcnt++) {
		//FBXファイル
		char fbxpath[MAX_PATH] = { 0 };
		WideCharToMultiByte(CP_UTF8, 0, s_retargetout[outcnt].c_str(), -1, fbxpath, MAX_PATH, NULL, NULL);
		//strcat_s(fbxpath, MAX_PATH, ".fbx");

		int result;
		result = RetargetFile(fbxpath);
		if (result == 0) {
			InterlockedExchange(&s_retargetcnt, (LONG)outcnt);
		}
		else {
			//InterlockedExchange(&g_retargetbatchflag, 0);
			break;
		}

		//if (g_retargetbatchflag != 1) {
		if (InterlockedAdd(&g_retargetbatchflag, 0) != 1) {
			break;
		}
	}
	
	InterlockedExchange(&g_retargetbatchflag, (LONG)3);


	//#########################################
	//watch thread state at OnFrameBatchThread
	//#########################################


	return 0;
}


int RetargetBatch()
{
	if (!s_model) {
		_ASSERT(0);
		return 0;
	}


	if (InterlockedAdd(&g_retargetbatchflag, 0) != 0) {//if already under calc, return 0.
		return 0;
	}


	s_savelimitdegflag = g_limitdegflag;
	g_limitdegflag = false;
	if (s_LimitDegCheckBox) {
		s_LimitDegCheckBox->SetChecked(g_limitdegflag);
	}


	//if (!s_convbone_model || !s_convbone_bvh) {
	//}
	//if (s_model != s_convbone_model) {
	//}

	s_convbone_model_batch = s_model;
	s_convbone_model_batch_selindex = s_curmodelmenuindex;


	s_saveretargetmodel = s_curmodelmenuindex;//終了時にOnModelMenuを呼ぶために保存


	BROWSEINFO bi;
	LPITEMIDLIST curlpidl = 0;
	WCHAR dispname[MAX_PATH] = { 0L };
	WCHAR selectname[MAX_PATH] = { 0L };
	int iImage = 0;

	//GetBatchHistoryDir(dispname, MAX_PATH);
	//if (selectname[0] != 0) {
	//	LPITEMIDLIST pidl;
	//	IMalloc* pMalloc;
	//	SHGetMalloc(&pMalloc);
	//	if (SUCCEEDED(SHGetSpecialFolderLocation(s_3dwnd, CSIDL_DESKTOPDIRECTORY, &pidl)))
	//	{
	//		// パスに変換する
	//		SHGetPathFromIDList(pidl, selectname);
	//		// 取得したIDLを解放する (CoTaskMemFreeでも可)
	//		pMalloc->Free(pidl);
	//		//SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir);
	//	}
	//	pMalloc->Release();
	//}

	bi.hwndOwner = s_3dwnd;
	bi.pidlRoot = NULL;//!!!!!!!
	bi.pszDisplayName = dispname;
	//bi.lpszTitle = L"保存フォルダを選択してください。";
	bi.lpszTitle = L"SelectDirectoryForBatch";
	//bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;// | BIF_NEWDIALOGSTYLE;//BIF_NEWDIALOGSTYLEを指定すると固まる　謎
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = 0;
	bi.iImage = iImage;

	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;
	HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
		WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

	curlpidl = SHBrowseForFolder(&bi);

	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	UnhookWinEvent(hhook);
	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;

	if (curlpidl) {
		//::DSMessageBox( m_hWnd, dispname, "フォルダー名", MB_OK );

		BOOL bret;
		bret = SHGetPathFromIDList(curlpidl, selectname);
		if (bret == FALSE) {
			_ASSERT(0);
			if (curlpidl)
				CoTaskMemFree(curlpidl);

			g_limitdegflag = s_savelimitdegflag;
			if (s_LimitDegCheckBox) {
				s_LimitDegCheckBox->SetChecked(g_limitdegflag);
			}

			return 1;
		}

		if (curlpidl)
			CoTaskMemFree(curlpidl);


		//selectname
		wstring target;
		string filenamepattern;
		//vector<wstring> out;
		wstring findext;

		s_retargetout.clear();
		target = selectname;
		findext = L".fbx";

		SaveBatchHistory(selectname);

		FindF(s_retargetout, target, findext);
		int outnum = (int)s_retargetout.size();
		InterlockedExchange(&s_retargetnum, (LONG)outnum);
		InterlockedExchange(&s_retargetcnt, 0);
		InterlockedExchange(&s_befretargetnum, 0);
		InterlockedExchange(&s_befretargetcnt, 0);

		if (outnum > 0)
		{
			InterlockedExchange(&g_retargetbatchflag, (LONG)1);

			CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)RetargetBatchDlgProc);
			RECT rect;
			GetWindowRect(s_LtimelineWnd->getHWnd(), &rect);
			SetWindowPos(s_retargetbatchwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
			ShowWindow(s_retargetbatchwnd, SW_SHOW);
			UpdateWindow(s_retargetbatchwnd);

			unsigned int threadaddr1 = 0;
			s_retargethandle1 = (HANDLE)_beginthreadex(
				NULL, 0, &ThreadFunc_Retarget,
				(void*)0,
				0, &threadaddr1);


			//WiatForしない場合には先に閉じてもOK
			if ((s_retargethandle1 != 0) && (s_retargethandle1 != INVALID_HANDLE_VALUE)) {
				CloseHandle(s_retargethandle1);
			}

		}
	}

	//InterlockedExchange(&g_retargetbatchflag, 0);//スレッドを立ててすぐに出ていくのでここではフラグはそのまま


	return 0;
}


//########## retarget batch ここまで


unsigned __stdcall ThreadFunc_Bvh2Fbx(LPVOID lpThreadParam)
{
	int outnum = (int)s_bvh2fbxout.size();
	int outcnt;

	InterlockedExchange(&s_progressnum, (LONG)outnum);
	InterlockedExchange(&s_progresscnt, (LONG)0);
	if (s_bvh2fbxbatchwnd) {
		HWND hProg;
		hProg = GetDlgItem(s_bvh2fbxbatchwnd, IDC_PROGRESS1);
		if (hProg) {
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
			SendMessage(hProg, PBM_SETPOS, (WPARAM)0, 0);
			UpdateWindow(s_bvh2fbxbatchwnd);
		}
	}


	for (outcnt = 0; outcnt < outnum; outcnt++) {

		InterlockedExchange(&s_progresscnt, (LONG)outcnt);

		//bvhファイルを読み込む
		CBVHFile* bvhfile = new CBVHFile();
		if (!bvhfile) {
			//g_bvh2fbxbatchflag = 0;
			break;
		}
		int ret;
		ret = bvhfile->LoadBVHFile(s_LtimelineWnd->getHWnd(), (wchar_t*)(s_bvh2fbxout[outcnt].c_str()), g_tmpmqomult);
		if (ret) {
			_ASSERT(0);
			if (bvhfile) {
				delete bvhfile;
				bvhfile = 0;
			}
			break;
		}

		//FBXファイルに書き出す
		char fbxpath[MAX_PATH] = { 0 };
		WideCharToMultiByte(CP_UTF8, 0, s_bvh2fbxout[outcnt].c_str(), -1, fbxpath, MAX_PATH, NULL, NULL);
		strcat_s(fbxpath, MAX_PATH, ".fbx");
		SYSTEMTIME localtime;
		GetLocalTime(&localtime);
		char fbxdate[MAX_PATH] = { 0L };
		sprintf_s(fbxdate, MAX_PATH, "CommentForEGP_%04u%02u%02u%02u%02u%02u",
			localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);

		int result;
		result = BVH2FBXFile(s_psdk, bvhfile, fbxpath, fbxdate);
		if (result == 0) {
			s_bvh2fbxcnt = outcnt;
		}
		else {
			if (bvhfile) {
				delete bvhfile;
				bvhfile = 0;
			}
			break;
		}

		if (bvhfile) {
			delete bvhfile;
			bvhfile = 0;
		}

		//if (g_bvh2fbxbatchflag != 1) {
		if (InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 1) {
			break;
		}
	}

	InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)3);

	//#########################################
	//watch thread state at OnFrameBatchThread
	//#########################################

	return 0;
}

//unsigned __stdcall ThreadFunc_Bvh2FbxDisp(LPVOID lpThreadParam)
//{
//
//	if (s_bvh2fbxbatchwnd != 0) {
//		SetWindowTextW(s_bvh2fbxbatchwnd, L"bvh to FBX Batch");
//	}
//
//	while (g_bvh2fbxbatchflag == 1) {
//		if (s_bvh2fbxbatchwnd != 0) {
//			SendMessage(s_bvh2fbxbatchwnd, WM_USER_FOR_BATCH_PROGRESS, 0, 0);
//			InvalidateRect(s_bvh2fbxbatchwnd, NULL, TRUE);
//			UpdateWindow(s_bvh2fbxbatchwnd);
//		}
//		Sleep(20);
//	}
//
//	return 0;
//}


int BVH2FBXBatch()
{
	if (InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) {//if already under calc, return 0.
		return 0;
	}

	BROWSEINFO bi;
	LPITEMIDLIST curlpidl = 0;
	WCHAR dispname[MAX_PATH] = { 0L };
	WCHAR selectname[MAX_PATH] = { 0L };
	int iImage = 0;

	//GetBatchHistoryDir(dispname, MAX_PATH);
	//if (selectname[0] != 0) {
	//	LPITEMIDLIST pidl;
	//	IMalloc* pMalloc;
	//	SHGetMalloc(&pMalloc);
	//	if (SUCCEEDED(SHGetSpecialFolderLocation(s_3dwnd, CSIDL_DESKTOPDIRECTORY, &pidl)))
	//	{
	//		// パスに変換する
	//		SHGetPathFromIDList(pidl, selectname);
	//		// 取得したIDLを解放する (CoTaskMemFreeでも可)
	//		pMalloc->Free(pidl);
	//		//SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir);
	//	}
	//	pMalloc->Release();
	//}

	bi.hwndOwner = s_3dwnd;
	bi.pidlRoot = NULL;//!!!!!!!
	bi.pszDisplayName = dispname;
	//bi.lpszTitle = L"保存フォルダを選択してください。";
	bi.lpszTitle = L"SelectDirectoryForBatch";
	//bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;// | BIF_NEWDIALOGSTYLE;//BIF_NEWDIALOGSTYLEを指定すると固まる　謎
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = 0;
	bi.iImage = iImage;

	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;
	HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
		WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

	curlpidl = SHBrowseForFolder(&bi);

	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	UnhookWinEvent(hhook);
	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;

	if (curlpidl) {
		//::DSMessageBox( m_hWnd, dispname, "フォルダー名", MB_OK );

		BOOL bret;
		bret = SHGetPathFromIDList(curlpidl, selectname);
		if (bret == FALSE) {
			_ASSERT(0);
			if (curlpidl)
				CoTaskMemFree(curlpidl);
			return 1;
		}

		if (curlpidl)
			CoTaskMemFree(curlpidl);


		//selectname
		wstring target;
		string filenamepattern;
		//vector<wstring> out;
		wstring findext;

		s_bvh2fbxout.clear();
		target = selectname;
		findext = L".bvh";

		SaveBatchHistory(selectname);

		FindF(s_bvh2fbxout, target, findext);
		int outnum = (int)s_bvh2fbxout.size();
		InterlockedExchange(&s_bvh2fbxnum, (LONG)outnum);
		InterlockedExchange(&s_bvh2fbxcnt, 0);
		InterlockedExchange(&s_befbvh2fbxnum, 0);
		InterlockedExchange(&s_befbvh2fbxcnt, 0);

		if (outnum > 0)
		{
	
			InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)1);//start thread


			CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)bvh2FbxBatchDlgProc);
			RECT rect;
			GetWindowRect(s_LtimelineWnd->getHWnd(), &rect);
			SetWindowPos(s_bvh2fbxbatchwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
			ShowWindow(s_bvh2fbxbatchwnd, SW_SHOW);
			UpdateWindow(s_bvh2fbxbatchwnd);

			unsigned int threadaddr1 = 0;
			s_bvh2fbxhandle1 = (HANDLE)_beginthreadex(
				NULL, 0, &ThreadFunc_Bvh2Fbx,
				(void*)0,
				0, &threadaddr1);

			//WiatForしない場合には先に閉じてもOK
			if (s_bvh2fbxhandle1 && (s_bvh2fbxhandle1 != INVALID_HANDLE_VALUE)) {
				CloseHandle(s_bvh2fbxhandle1);
			}
		}
	}


	return 0;
}

int BVH2FBX()
{

	int dlgret;
	s_filterindex = 5;
	dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPENMQODLG),
		s_3dwnd, (DLGPROC)OpenMqoDlgProc);
	if ((dlgret != IDOK) || !g_tmpmqopath[0]) {
		return 0;
	}


	WCHAR savepath[MULTIPATH];
	MoveMemory(savepath, g_tmpmqopath, sizeof(WCHAR) * MULTIPATH);


	//bvhファイルを読み込む
	CBVHFile bvhfile;
	int ret;
	ret = bvhfile.LoadBVHFile( s_3dwnd, g_tmpmqopath, g_tmpmqomult );
	if( ret ){
		_ASSERT( 0 );
		return 1;
	}

	Savebvh2FBXHistory(savepath);

	//FBXファイルに書き出す
	char fbxpath[MAX_PATH] = {0};
	WideCharToMultiByte( CP_UTF8, 0, g_tmpmqopath, -1, fbxpath, MAX_PATH, NULL, NULL );
	strcat_s( fbxpath, MAX_PATH, ".fbx" );
	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	char fbxdate[MAX_PATH] = { 0L };
	sprintf_s(fbxdate, MAX_PATH, "CommentForEGP_%04u%02u%02u%02u%02u%02u",
		localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
	CallF( BVH2FBXFile( s_psdk, &bvhfile, fbxpath, fbxdate ), return 1 );


	return 0;
}

int OpenFile()
{
	s_nowloading = true;


	//大きいフレーム一のまま小さいフレーム長のデータを読み込んだ時にエラーにならないように。
	InitTimelineSelection();



	//CurrentDirectoryがMameMediaになっていたときにはTestディレクトリに変える
	WCHAR curdir[MAX_PATH] = { 0L };
	ZeroMemory(curdir, sizeof(WCHAR) * MAX_PATH);
	GetCurrentDirectory(MAX_PATH, curdir);
	WCHAR* findpat = wcsstr(curdir, L"\\MameMedia");
	if (findpat) {
		WCHAR initialdir[MAX_PATH] = { 0L };
		wcscpy_s(initialdir, MAX_PATH, g_basedir);
		wcscat_s(initialdir, MAX_PATH, L"..\\Test\\");
		SetCurrentDirectoryW(initialdir);
	}



	int dlgret;
	s_filterindex = 1;
	dlgret = (int)DialogBoxW( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDD_OPENMQODLG ), 
		s_3dwnd, (DLGPROC)OpenMqoDlgProc );
	if( (dlgret != IDOK) || !g_tmpmqopath[0] ){
		s_nowloading = false;
		return 0;
	}

	WCHAR savepath[MULTIPATH];
	ZeroMemory(savepath, sizeof(WCHAR) * MULTIPATH);
	MoveMemory( savepath, g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );

	int leng;
	int namecnt = 0;
	savepath[MULTIPATH - 1] = 0L;
	leng = (int)wcslen( savepath );
	WCHAR* topchar = savepath + leng;
	if( *topchar == TEXT( '\0' ) ){
		WCHAR* extptr = 0;
		extptr = wcsrchr( g_tmpmqopath, TEXT( '.' ) );
		if( !extptr ){
			s_nowloading = false;
			return 0;
		}
		int result = 0;
		CModel* newmodel = 0;
		int cmpcha, cmpfbx, cmpmqo, cmpref, cmpimp, cmpgco, cmpmnl;
		cmpcha = wcscmp( extptr, L".cha" );
		cmpfbx = wcscmp( extptr, L".fbx" );
		cmpmqo = wcscmp( extptr, L".mqo" );
		cmpref = wcscmp( extptr, L".ref" );
		cmpimp = wcscmp( extptr, L".imp" );
		cmpgco = wcscmp( extptr, L".gco" );
		cmpmnl = wcscmp( extptr, L".mnl" );
		if( cmpcha == 0 ){
			result = OpenChaFile();
			s_filterindex = 1;
		}else if( cmpfbx == 0 ){
			if (s_modelindex.size() > 0) {
				OnModelMenu(false, (int)s_modelindex.size() - 1, 1);
			}
			newmodel = OpenFBXFile( true, 0, 1 );
			if (newmodel) {
				result = 0;
			}
			else {
				result = 1;
			}
			s_filterindex = 1;
		}else if( cmpmqo == 0 ){
			if (s_modelindex.size() > 0) {
				OnModelMenu(false, (int)s_modelindex.size() - 1, 1);
			}
			newmodel = OpenMQOFile();
			if (newmodel) {
				result = 0;
			}
			else {
				result = 1;
			}
			s_filterindex = 1;
		}else if( cmpref == 0 ){
			result = OpenREFile();
			s_filterindex = 2;
		}else if( cmpimp == 0 ){
			result = OpenImpFile();
			s_filterindex = 3;
		}else if( cmpgco == 0 ){
			result = OpenGcoFile();
			s_filterindex = 4;
		}
		else if (cmpmnl == 0) {
			result = OpenMNLFile();
			s_filterindex = 6;
		}

		if (result != 0) {
			WCHAR strerror[MAX_PATH * 2] = { 0L };
			swprintf_s(strerror, MAX_PATH * 2, L"%s の\n読み込みに失敗しました。", g_tmpmqopath);
			MessageBox(s_mainhwnd, strerror, L"エラー", MB_OK);
			s_nowloading = false;
			return 1;
		}

	}else{
		int leng2;
		while( *topchar != TEXT( '\0' ) ){
			savepath[MULTIPATH - 1] = 0L;
			leng2 = (int)wcslen(topchar);
			swprintf_s( g_tmpmqopath, MULTIPATH, L"%s\\%s", savepath, topchar );

			WCHAR* extptr = 0;
			extptr = wcsrchr( g_tmpmqopath, TEXT( '.' ) );
			if( !extptr ){
				s_nowloading = false;
				return 0;
			}
			int result = 0;
			CModel* newmodel = 0;
			int cmpfbx, cmpmqo;
			cmpfbx = wcscmp( extptr, L".fbx" );
			cmpmqo = wcscmp( extptr, L".mqo" );
			if( cmpfbx == 0 ){
				//WCHAR* nexttopchar = topchar + leng2 + 1;
				//if (*nexttopchar != TEXT('\0')) {
					if (s_modelindex.size() > 0) {
						OnModelMenu(false, (int)s_modelindex.size() - 1, 1);
					}
					newmodel = OpenFBXFile(true, 0, 1);
					if (newmodel) {
						result = 0;
					}
					else {
						result = 1;
					}
				//}
				//else {
				//	//最終のFBXに対してのみinittimelineをする
				//	OpenFBXFile(0, 1);
				//}
				s_filterindex = 1;
			}else if( cmpmqo == 0 ){
				if (s_modelindex.size() > 0) {
					OnModelMenu(false, (int)s_modelindex.size() - 1, 1);
				}
				newmodel = OpenMQOFile();
				if (newmodel) {
					result = 0;
				}
				else {
					result = 1;
				}
				s_filterindex = 1;
			}

			if (result != 0) {
				WCHAR strerror[MAX_PATH * 2] = { 0L };
				swprintf_s(strerror, MAX_PATH * 2, L"%s の\n読み込みに失敗しました。", g_tmpmqopath);
				MessageBox(s_mainhwnd, strerror, L"エラー", MB_OK);
				s_nowloading = false;
				return 1;
			}

			savepath[MULTIPATH - 1] = 0L;
			leng2 = (int)wcslen( topchar );
			topchar = topchar + leng2 + 1;
			namecnt++;
		}
	}

	s_nowloading = false;

	//ChangeCurrentBone();

	return 0;
}

CModel* OpenMQOFile()
{
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();


	//大きいフレーム一のまま小さいフレーム長のデータを読み込んだ時にエラーにならないように。
	InitTimelineSelection();


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
	ret = newmodel->LoadMQO( s_pdev, pd3dImmediateContext, g_tmpmqopath, modelfolder, g_tmpmqomult, 0 );
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

 //   CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	//pComboBox->RemoveAllItems();

	//map<int, CBone*>::iterator itrbone;
	//for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
	//	ULONG boneno = (ULONG)itrbone->first;
	//	CBone* curbone = itrbone->second;
	//	if( curbone && (boneno >= 0) ){
	//		char* nameptr = (char*)curbone->GetBoneName();
	//		WCHAR wname[256];
	//		ZeroMemory( wname, sizeof( WCHAR ) * 256 );
	//		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
	//		pComboBox->AddItem( wname, ULongToPtr( boneno ) );
	//	}
	//}

	//s_totalmb.center = ChaVector3( 0.0f, 0.0f, 0.0f );
	//s_totalmb.max = ChaVector3( 0.0f, 0.0f, 0.0f );
	//s_totalmb.min = ChaVector3( 0.0f, 0.0f, 0.0f );
	//s_totalmb.r = 0.0f;
	CalcTotalBound();


	CallF( AddMotion( 0 ), return 0 );
	InitCurMotion(0, 0);

	s_modelindex[ mindex ].tlarray = s_tlarray;
	s_modelindex[ mindex ].lineno2boneno = s_lineno2boneno;
	s_modelindex[ mindex ].boneno2lineno = s_boneno2lineno;

//	CallF( OnModelMenu( mindex, 0 ), return 0 );

	DispModelPanel();

	return newmodel;
}

void CalcTotalBound()
{

	//s_totalmb.center = ChaVector3(0.0f, 0.0f, 0.0f);
	//s_totalmb.max = ChaVector3(50.0f, 50.0f, 50.0f);
	//s_totalmb.min = ChaVector3(-50.0f, -50.0f, -50.0f);
	//s_totalmb.r = (float)ChaVector3LengthDbl(&s_totalmb.max);

	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		CModel* curmodel = itrmodel->modelptr;
		MODELBOUND mb;
		curmodel->GetModelBound(&mb);
		if (mb.r != 0.0f) {
			AddModelBound(&s_totalmb, &mb);
		}
	}

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	FLOAT fObjectRadius;
	g_vCenter = s_totalmb.center;
	fObjectRadius = s_totalmb.r;
	if (fObjectRadius < 0.1f) {
		fObjectRadius = 10.0f;
	}

	s_cammvstep = fObjectRadius;

	DbgOut(L"fbx : totalmb : r %f, center (%f, %f, %f)\r\n",
		s_totalmb.r, s_totalmb.center.x, s_totalmb.center.y, s_totalmb.center.z);


	s_projnear = fObjectRadius * 0.01f;
	g_initcamdist = fObjectRadius * 3.0f;
	//g_Camera->SetProjParams( PI / 4, s_fAspectRatio, s_projnear, 5.0f * g_initcamdist );
	g_Camera->SetProjParams(PI / 4, s_fAspectRatio, s_projnear, 100.0f * g_initcamdist);


	for (int i = 0; i < MAX_LIGHTS; i++)
		g_LightControl[i].SetRadius(fObjectRadius);


	ChaVector3 vecEye(0.0f, 0.0f, g_initcamdist);
	ChaVector3 vecAt(0.0f, 0.0f, -0.0f);
	g_Camera->SetViewParams(vecEye.XMVECTOR(1.0f), vecAt.XMVECTOR(1.0f));
	g_Camera->SetRadius(fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 6.0f);

	s_camdist = fObjectRadius * 4.0f;
	g_camEye = ChaVector3(0.0f, fObjectRadius * 0.5f, fObjectRadius * 4.0f);
	g_camtargetpos = ChaVector3(0.0f, fObjectRadius * 0.5f, -0.0f);
	g_befcamEye = g_camEye;
	g_befcamtargetpos = g_camtargetpos;
	//!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	g_Camera->SetViewParams(g_camEye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
	g_Camera->SetRadius(fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 6.0f);
	s_matView = g_Camera->GetViewMatrix();
	s_matProj = g_Camera->GetProjMatrix();

}


CModel* OpenFBXFile( bool dorefreshtl, int skipdefref, int inittimelineflag )
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();


	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}


	//大きいフレーム一のまま小さいフレーム長のデータを読み込んだ時にエラーにならないように。
	InitTimelineSelection();


	//int dlgret;
	//dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHECKAXISTYPE),
	//	s_3dwnd, (DLGPROC)CheckAxisTypeProc);
	//if (dlgret != IDOK){
	//	return 0;
	//}

	g_camtargetpos = ChaVector3( 0.0f, 0.0f, 0.0f );
	g_befcamtargetpos = g_camtargetpos;


	static int modelcnt = 0;

	WCHAR fbxpath0[MAX_PATH] = { 0L };
	wcscpy_s(fbxpath0, MAX_PATH, g_tmpmqopath);

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

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	DestroyTimeLine( 1 );

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

    // Load the mesh
	CModel* newmodel;
	newmodel = new CModel();
	if( !newmodel ){
		_ASSERT( 0 );
		return 0;
	}

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	_ASSERT(s_btWorld);
	newmodel->SetBtWorld( s_btWorld );
	FbxScene* pScene = 0;
	FbxImporter* pImporter = 0;
	//skipdefref FBX単体読み込みの場合にはdefault_ref.refは存在しない。その場合skipdefrefには０が代入され、CModel::LoadFBX内でdefault_ref.refの中でメモリからデフォルト値が設定される
	int ret;
	BOOL motioncachebatchflag = FALSE;
	ret = newmodel->LoadFBX(skipdefref, s_pdev, pd3dImmediateContext, g_tmpmqopath, modelfolder, g_tmpmqomult, s_psdk, &pImporter, &pScene, s_forcenewaxis, motioncachebatchflag);
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

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	CallF( s_model->MakeDispObj(), return 0 );

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	int mindex;
	mindex = (int)s_modelindex.size();
	MODELELEM modelelem;
	modelelem.modelptr = s_model;
	modelelem.motmenuindex = 0;
	s_modelindex.push_back( modelelem );

 //   CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	//pComboBox->RemoveAllItems();

	//map<int, CBone*>::iterator itrbone;
	//for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
	//	ULONG boneno = (ULONG)itrbone->first;
	//	CBone* curbone = itrbone->second;
	//	if( curbone && (boneno >= 0) ){
	//		char* nameptr = (char*)curbone->GetBoneName();
	//		WCHAR wname[256];
	//		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
	//		pComboBox->AddItem( wname, ULongToPtr( boneno ) );
	//	}
	//}

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}


	CalcTotalBound();

	s_modelindex[ mindex ].tlarray = s_tlarray;
	s_modelindex[ mindex ].lineno2boneno = s_lineno2boneno;
	s_modelindex[ mindex ].boneno2lineno = s_boneno2lineno;

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	CallF(OnModelMenu(dorefreshtl, mindex, 0), return 0);

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}


	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	//CallF( s_model->LoadFBXAnim( s_psdk, pImporter, pScene, OnAddMotion ), return 0 );
	CallF(s_model->LoadFBXAnim(s_psdk, pImporter, pScene, NULL, motioncachebatchflag), return 0);
	//if( (int)s_modelindex.size() >= 2 )
	//	_ASSERT( 0 );

//::MessageBox(s_mainhwnd, L"check 1", L"check!!!", MB_OK);

	CalcTotalBound();



	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	//if( s_model->GetMotInfoSize() >= 2 ){
	//	OnDelMotion( 0 );//初期状態のダミーモーションを削除
	//}

//::MessageBox(s_mainhwnd, L"check 2", L"check!!!", MB_OK);

	//OnAnimMenuでCreateRigidElemを呼ぶ前に、default_ref.refを読む
	if (skipdefref == 1) {//プロジェクトファイルから呼ばれて、かつ、default_ref.refが存在する場合
		if (s_model->GetMotInfoSize() > 0) {
			MOTINFO* firstmi = s_model->GetMotInfo(1);
			if (firstmi) {
				s_model->SetCurrentMotion(firstmi->motid);

				WCHAR savetmpmqopath[MAX_PATH];
				wcscpy_s(savetmpmqopath, MAX_PATH, g_tmpmqopath);
				WCHAR* lastenref;
				lastenref = wcsrchr(g_tmpmqopath, TEXT('\\'));
				if (lastenref) {
					*lastenref = 0L;
					wcscat_s(g_tmpmqopath, MAX_PATH, L"\\default_ref.ref");
					int chkret;
					chkret = OpenREFile();
					_ASSERT(chkret == 0);


					wcscpy_s(g_tmpmqopath, MAX_PATH, savetmpmqopath);
				}
			}
		}
	}

//::MessageBox(s_mainhwnd, L"check 3", L"check!!!", MB_OK);


	//if (inittimelineflag == 1)//inittimelineflag は 最後のキャラの時に１
	{
		int lastmotid = -1;
		int motnum = s_model->GetMotInfoSize();
		int motno;
		for (motno = 0; motno < motnum; motno++) {

//WCHAR strchk[256] = { 0L };
//swprintf_s(strchk, 256, L"check 3 : %d / %d", motno, motnum);
//::MessageBox(s_mainhwnd, strchk, L"check!!!", MB_OK);

			MOTINFO* curmi = s_model->GetMotInfo(motno + 1);
			if (curmi) {
				lastmotid = curmi->motid;
				s_model->SetCurrentMotion(lastmotid);
				//OnAddMotion(curmi->motid, (motno == 0));
				OnAddMotion(curmi->motid, (motno == (motnum - 1)));//最後のモーション!!!!!! 2021/08/19

				if (s_nowloading && s_3dwnd) {
					OnRenderNowLoading();
				}

			}
		}
		//s_model->SetCurrentMotion(lastmotid);
	}
	//}

//::MessageBox(s_mainhwnd, L"check 4", L"check!!!", MB_OK);

	int motnum = s_model->GetMotInfoSize();
	if (motnum == 0){
		CallF(AddMotion(0), return 0);
		s_modelindex[mindex].tlarray = s_tlarray;
		s_modelindex[mindex].lineno2boneno = s_lineno2boneno;
		s_modelindex[mindex].boneno2lineno = s_boneno2lineno;

		InitCurMotion(0, 0);
	}

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

//::MessageBox(s_mainhwnd, L"check 5", L"check!!!", MB_OK);


	OnRgdMorphMenu( 0 );

//	SetCapture( s_3dwnd );

	s_curmotid = s_model->GetCurMotInfo()->motid;

	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	if (s_model->GetOldAxisFlagAtLoading() == 0){
		CLmtFile lmtfile;
		WCHAR lmtname[MAX_PATH];
		swprintf_s(lmtname, MAX_PATH, L"%s.lmt", g_tmpmqopath);
		char fbxcomment[MAX_PATH] = { 0 };
		int chkretcomment;
		chkretcomment = s_model->GetFbxComment(fbxcomment, MAX_PATH);
		if (chkretcomment == 0) {
			int chkret1;
			chkret1 = lmtfile.LoadLmtFile(lmtname, s_model, fbxcomment);
			//_ASSERT(chkret1 == 0);
		}
		WCHAR rigname[MAX_PATH] = { 0L };
		swprintf_s(rigname, MAX_PATH, L"%s.rig", g_tmpmqopath);
		CRigFile rigfile;
		int chkret2;
		chkret2 = rigfile.LoadRigFile(rigname, s_model);
		//_ASSERT(chkret2 == 0);
	}

//::MessageBox(s_mainhwnd, L"check 6", L"check!!!", MB_OK);


	s_model->SetMotionSpeed(g_dspeed);

	DispModelPanel();


	//OnAnimMenuで呼ぶ
	//if (skipdefref == 0) {
	//	s_model->CreateBtObject(1);//初回
	//	s_model->CalcBoneEul(-1);
	//}




	CallF(s_model->DbgDump(), return 0);

	g_dbgloadcnt++;

	//s_model->DestroyScene();
	s_model->SetLoadedFlag(true);

	//ShowRigidWnd(true);

//::MessageBox(s_mainhwnd, L"check 7", L"check!!!", MB_OK);


	if (s_nowloading && s_3dwnd) {
		OnRenderNowLoading();
	}

	OrgWindowListenMouse(true);

//::MessageBox(s_mainhwnd, L"check 8", L"check!!!", MB_OK);


	SetTimelineHasRigFlag();


//############################
//############################

	//読み込み処理が成功してから履歴を保存する。fbxファイル。
	int savepathlen;
	fbxpath0[MAX_PATH - 1] = 0L;
	savepathlen = (int)wcslen(fbxpath0);
	if (savepathlen > 4) {
		WCHAR* pwext;
		pwext = fbxpath0 + ((size_t)savepathlen - 1) - 3;
		if (wcscmp(pwext, L".fbx") == 0) {
			SYSTEMTIME localtime;
			GetLocalTime(&localtime);
			WCHAR HistoryForOpeningProjectWithGamePad[MAX_PATH] = { 0L };
			swprintf_s(HistoryForOpeningProjectWithGamePad, MAX_PATH, L"%s\\MB3DOpenProj_%04u%02u%02u%02u%02u%02u.txt",
				s_temppath,
				localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
			HANDLE hfile;
			hfile = CreateFile(HistoryForOpeningProjectWithGamePad, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				//int pathlen;
				//pathlen = (int)wcslen(saveprojpath);
				//if ((pathlen > 0) && (pathlen < MAX_PATH)) {
				if ((savepathlen > 0) && (savepathlen < MAX_PATH)) {
					DWORD writelen = 0;
					WriteFile(hfile, fbxpath0, (savepathlen * sizeof(WCHAR)), &writelen, NULL);
					_ASSERT((savepathlen * sizeof(WCHAR)) == writelen);
				}
				CloseHandle(hfile);
			}
		}
	}





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

				if (topbone) {
					topbone->ResizeIndexedMotionPointReq(curmi->motid, motleng);
				}

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

int AddTimeLine( int newmotid, bool dorefreshtl )
{
	EnterCriticalSection(&s_CritSection_LTimeline);

	//EraseKeyList();
	if (s_model && s_model->GetBoneListSize() > 0) {

		if (!s_owpTimeline) {
			//OWP_Timeline* owpTimeline = 0;
			//タイムラインのGUIパーツを生成
			s_owpTimeline = new OWP_Timeline(L"testmotion", 100.0, 4.0);

			// カーソル移動時のイベントリスナーに
			// カーソル移動フラグcursorFlagをオンにするラムダ関数を登録する
			s_owpTimeline->setCursorListener([]() { 
				if (s_model) {
					s_cursorFlag = true;
				}
			});

			// キー選択時のイベントリスナーに
			// キー選択フラグselectFlagをオンにするラムダ関数を登録する
			//s_owpTimeline->setSelectListener([](){ s_selectFlag = true; });//LTimelineへ移動

			s_owpTimeline->setMouseRUpListener([]() {
				if (s_model) {
					s_timelineRUpFlag = true;
				}
			});

			// キー移動時のイベントリスナーに
			// キー移動フラグkeyShiftFlagをオンにして、キー移動量をコピーするラムダ関数を登録する
			s_owpTimeline->setKeyShiftListener([]() {
				if (s_model) {
					s_keyShiftFlag = true;
					s_keyShiftTime = s_owpTimeline->getShiftKeyTime();
				}
			});

			// キー削除時のイベントリスナーに
			// 削除されたキー情報をスタックするラムダ関数を登録する
			s_owpTimeline->setKeyDeleteListener([](const KeyInfo &keyInfo) {
				//s_deletedKeyInfoList.push_back(keyInfo);
			});


			//ウィンドウにタイムラインを関連付ける
			s_timelineWnd->addParts(*s_owpTimeline);



			//		s_owpTimeline->timeSize = 4.0;
			//		s_owpTimeline->callRewrite();						//再描画
			//		s_owpTimeline->setRewriteOnChangeFlag(true);		//再描画要求を再開
		}

		if (s_owpTimeline) {
			int nextindex;
			nextindex = (int)s_tlarray.size();

			TLELEM newtle;
			newtle.menuindex = nextindex;
			newtle.motionid = newmotid;
			s_tlarray.push_back(newtle);

			s_model->SetCurrentMotion(newmotid);
		}

		if (s_LtimelineWnd && s_owpPlayerButton) {
			if (!s_LTSeparator) {
				s_LTSeparator = new OWP_Separator(s_LtimelineWnd, false, 0.38, false);
				s_LtimelineWnd->addParts(*s_LTSeparator);

				if (s_owpLTimeline) {
					delete s_owpLTimeline;
					s_owpLTimeline = 0;
				}
				s_owpLTimeline = new OWP_Timeline(L"EditRangeTimeLine");
				//s_LtimelineWnd->addParts(*s_owpLTimeline);//playerbuttonより後
				s_LTSeparator->addParts1(*s_owpLTimeline);
				s_owpLTimeline->setCursorListener([]() { 
					if (s_model) {
						s_LcursorFlag = true;
					}
				});
				s_owpLTimeline->setSelectListener([]() { 
					if (s_model) {
						s_selectFlag = true;
					}
				});
				s_owpLTimeline->setMouseMDownListener([]() {
					if (s_model) {
						s_timelinembuttonFlag = true;
						//if (s_mbuttoncnt == 0) {
						//	s_mbuttoncnt = 1;
						//}
						//else {
						//	s_mbuttoncnt = 0;
						//}
					}
				});
				s_owpLTimeline->setMouseWheelListener([]() {
					if (s_model) {
						if ((g_keybuf['S'] & 0x80) == 0) {//Scroll の S
							s_timelinewheelFlag = true;
							s_timelineshowposFlag = false;
							s_LcursorFlag = true;
						}
						else {
							s_timelinewheelFlag = false;
							s_timelineshowposFlag = true;
						}
					}
				});


				if (s_parentcheck) {
					delete s_parentcheck;
					s_parentcheck = 0;
				}
				s_parentcheck = new OWP_CheckBoxA(L"ParentEuler", 1);//parentcheck ON by default
				//s_LtimelineWnd->addParts(*s_parentcheck);
				//s_LTSeparator->addParts2(*s_parentcheck);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!parentwindow libfbxdll error 2021/03/04 comment out tmp
				s_parentcheck->setButtonListener([]() { 
					if (s_model) {
						refreshEulerGraph();
					}
				});


				if (s_owpEulerGraph) {
					delete s_owpEulerGraph;
					s_owpEulerGraph = 0;
				}
				s_owpEulerGraph = new OWP_EulerGraph(L"EulerGraph");
				//s_LtimelineWnd->addParts(*s_owpEulerGraph);
				s_LTSeparator->addParts2(*s_owpEulerGraph);

				//OrgWinGUI::WindowSize graphsize = OrgWinGUI::WindowSize(s_LTSeparator->getSize().x - 8, 60);
				//s_owpEulerGraph->setSize(graphsize);
				//OrgWinGUI::WindowPos graphpos = OrgWinGUI::WindowPos(0, 16);
				//s_owpEulerGraph->setPos(graphpos);
				s_owpEulerGraph->setCursorListener([]() { 
					if (s_model) {
						s_LcursorFlag = true;
					}
				});
			}
		}

		//タイムラインのキーを設定
		if (s_owpTimeline) {
			if (dorefreshtl) {
				refreshTimeline(*s_owpTimeline);
			}
			s_owpLTimeline->selectClear();
		}

	}


	LeaveCriticalSection(&s_CritSection_LTimeline);

	return 0;
}


int UpdateEditedEuler()
{
	//オイラーグラフのキーを作成しなおす場合にはrefreshEulerGraph()


	//ツールボタンからも呼ぶ
	//if (s_pickinfo.buttonflag == 0) {
	//	return 0;
	//}

	if (!s_model || !s_owpLTimeline || !s_owpEulerGraph) {
		return 0;
	}

	//if (s_model && (s_model->GetLoadedFlag() == false)) {
	//	return 0;
	//}


	//選択状態がない場合にはtopboneのオイラーグラフを表示する。
	if (s_curboneno < 0) {
		CBone* topbone = s_model->GetTopBone();
		if (topbone) {
			s_curboneno = topbone->GetBoneNo();
		}
	}


	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (curbone) {
		//if (s_parentcheck) {
			//int check = (int)s_parentcheck->getValue();
			//if (check == 1) {
				CBone* parentbone = curbone->GetParent();
				//if ((s_ikkind == 0) && parentbone) {//!!!!!!!!!!!!
				if(parentbone){
					curbone = parentbone;
				}
			//}
		//}

		MOTINFO* curmi = s_model->GetCurMotInfo();
		if (curmi) {

			double curtime;
			float minval = 0.0f;
			float maxval = 0.0f;
			int minfirstflag, maxfirstflag;
			bool isset = false;

			//refreshEulerGraphは全範囲でminとmaxを設定。UpdateEditedEulerはstartframeからendframeまで。
			s_owpEulerGraph->getEulMinMax(&isset, &minval, &maxval);
			if (isset == true) {
				minfirstflag = 0;
				maxfirstflag = 0;
			}
			else {
				minfirstflag = 1;
				maxfirstflag = 1;
			}
			//minfirstflag = 1;
			//maxfirstflag = 1;


			//s_buttonselectstart = s_editrange.GetStartFrame();
			//s_buttonselectend = s_editrange.GetEndFrame();

			double startframe, endframe;
			startframe = s_buttonselectstart;
			endframe = s_buttonselectend;
			double firstframe;
			firstframe = max((startframe - 1.0), 0.0);

			ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
			int ret;
			ret = s_owpEulerGraph->getEuler(firstframe, &befeul);
			if (ret) {
				befeul = ChaVector3(0.0f, 0.0f, 0.0f);
			}

			for (curtime = startframe; curtime <= endframe; curtime += 1.0) {
				const WCHAR* wbonename = curbone->GetWBoneName();
				ChaVector3 orgeul = ChaVector3(0.0f, 0.0f, 0.0f);
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				//cureul = curbone->CalcFBXEul(curmi->motid, (double)curtime, &befeul);
				//befeul = cureul;//!!!!!!!

				CMotionPoint* curmp = curbone->GetMotionPoint(curmi->motid, (double)curtime);
				if (curmp) {
					if (s_ikkind == 0) {//回転
						curbone->GetLimitedWorldMat(curmi->motid, (double)curtime, &cureul);
					}
					else if(s_ikkind == 1){//移動
						cureul = curbone->CalcLocalTraAnim(curmi->motid, (double)curtime);
					}
					else if (s_ikkind == 2) {//スケール
						cureul = curbone->CalcLocalScaleAnim(curmi->motid, (double)curtime);
					}
				}
				else {
					cureul.x = 0.0;
					cureul.y = 0.0;
					cureul.z = 0.0;
					//befeul = cureul;//!!!!!!!
				}
				if ((curtime == 0.0) || (curtime == 1.0) || IsValidNewEul(cureul, befeul)) {
					befeul = cureul;
				}


				s_owpEulerGraph->setKey(_T("X"), (double)curtime, cureul.x);
				s_owpEulerGraph->setKey(_T("Y"), (double)curtime, cureul.y);
				s_owpEulerGraph->setKey(_T("Z"), (double)curtime, cureul.z);

				if (minfirstflag == 1) {
					minval = min(cureul.z, min(cureul.x, cureul.y));
					minfirstflag = 0;
				}
				if (minval > cureul.x) {
					minval = cureul.x;
				}
				if (minval > cureul.y) {
					minval = cureul.y;
				}
				if (minval > cureul.z) {
					minval = cureul.z;
				}

				if (maxfirstflag == 1) {
					maxval = max(cureul.z, max(cureul.x, cureul.y));
					maxfirstflag = 0;
				}
				if (maxval < cureul.x) {
					maxval = cureul.x;
				}
				if (maxval < cureul.y) {
					maxval = cureul.y;
				}
				if (maxval < cureul.z) {
					maxval = cureul.z;
				}

			}


			s_owpEulerGraph->setEulMinMax(s_ikkind, minval, maxval);

			if (g_motionbrush_value) {

				double scalemin, scalemax;
				if (minval != maxval) {
					scalemin = minval;
					scalemax = maxval;
				}
				else {
					//Eulerが全て０　例えば全フレームを選択してツールの姿勢初期化を実行した後など
					//仮のminとmaxを指定
					scalemin = minval;
					scalemax = maxval + 10.0;
				}

				unsigned int scaleindex;
				for (scaleindex = 0; scaleindex < curmi->frameleng; scaleindex++) {
					double curscalevalue;
					if ((scaleindex >= (int)g_motionbrush_startframe) && (scaleindex <= (int)g_motionbrush_endframe) && (scaleindex < (int)g_motionbrush_frameleng)) {
						curscalevalue = (double)(*(g_motionbrush_value + scaleindex));// *(scalemax - scalemin) + scalemin;
						curscalevalue = (curscalevalue * 0.5 + 0.5) * (scalemax - scalemin) + scalemin;
					}
					else {
						curscalevalue = 0.0;// *(scalemax - scalemin) + scalemin;
						curscalevalue = (curscalevalue * 0.5 + 0.5) * (scalemax - scalemin) + scalemin;
					}
					s_owpEulerGraph->setKey(_T("S"), (double)scaleindex, curscalevalue);//setkey
				}


			}

			//_ASSERT(0);
			s_owpEulerGraph->callRewrite();

		}
	}

	return 0;
}

int refreshEulerGraph()
{

	//オイラーグラフのキーを作成しなおさない場合はUpdateEditedEuler()

	if (!s_model || !s_owpLTimeline || !s_owpEulerGraph) {
		return 0;
	}

	//if (s_model && (s_model->GetLoadedFlag() == false)) {
	//	return;
	//}

	MOTINFO* curmotinfo = 0;
	if (s_model && ((curmotinfo = s_model->GetCurMotInfo()) != 0)) {

		int frameleng = (int)s_model->GetCurMotInfo()->frameleng;

		//if (!g_motionbrush_value || (g_motionbrush_frameleng != frameleng)) {
			int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
			if (result) {
				_ASSERT(0);
			}
		//}

		//int result = CreateMotionBrush(0, (double)(frameleng - 1), true);
		//_ASSERT(result == 0);

		s_owpEulerGraph->setDispScale(1.0);//倍率初期化
		s_owpEulerGraph->setDispOffset(0.0);//位置オフセット

		s_owpEulerGraph->deleteKey();
		s_owpEulerGraph->deleteLine();

		s_owpEulerGraph->newLine(0, 0, _T("X"));
		s_owpEulerGraph->newLine(0, 0, _T("Y"));
		s_owpEulerGraph->newLine(0, 0, _T("Z"));
		s_owpEulerGraph->newLine(0, 0, _T("S"));

		//s_owpLTimeline->setMaxTime( s_model->m_curmotinfo->frameleng - 1.0 );
		s_owpEulerGraph->setMaxTime(s_model->GetCurMotInfo()->frameleng);//左端の１マスを選んだ状態がフレーム０を選んだ状態だから　-1 しない。



		if (s_model && (s_curboneno >= 0)){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				//if (s_parentcheck) {
					//int check = s_parentcheck->getValue();
					//if (check == 1) {
						CBone* parentbone = curbone->GetParent();
						//if ((s_ikkind == 0) && parentbone) {//!!!!!!!!!!!!
						if (parentbone) {
							curbone = parentbone;
						}
					//}
				//}

				MOTINFO* curmi = s_model->GetCurMotInfo();
				if (curmi){
					int curtime;
					float minval = 0.0;
					float maxval = 0.0;
					int minfirstflag = 1;
					int maxfirstflag = 1;

					double firstframe = 0.0;
					ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
					int ret;
					ret = s_owpEulerGraph->getEuler(firstframe, &befeul);
					if (ret) {
						befeul = ChaVector3(0.0f, 0.0f, 0.0f);
					}
					for (curtime = 0; curtime < frameleng; curtime++) {
						const WCHAR* wbonename = curbone->GetWBoneName();
						ChaVector3 orgeul = ChaVector3(0.0f, 0.0f, 0.0f);
						ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
						//cureul = curbone->CalcFBXEul(curmi->motid, (double)curtime, &befeul);
						//befeul = cureul;//!!!!!!!

						CMotionPoint* curmp = curbone->GetMotionPoint(curmi->motid, (double)curtime);
						if (curmp) {
							if (s_ikkind == 0) {//回転
								curbone->GetLimitedWorldMat(curmi->motid, (double)curtime, &cureul);
							}
							else if (s_ikkind == 1) {//移動
								cureul = curbone->CalcLocalTraAnim(curmi->motid, (double)curtime);
							}
							else if (s_ikkind == 2) {//スケール
								cureul = curbone->CalcLocalScaleAnim(curmi->motid, (double)curtime);
							}
						}
						else {
							cureul.x = 0.0;
							cureul.y = 0.0;
							cureul.z = 0.0;
							//befeul = cureul;
						}
						if ((curtime == 0.0) || (curtime == 1.0) || IsValidNewEul(cureul, befeul)) {
							befeul = cureul;
						}


						s_owpEulerGraph->newKey(_T("X"), (double)curtime, cureul.x);
						s_owpEulerGraph->newKey(_T("Y"), (double)curtime, cureul.y);
						s_owpEulerGraph->newKey(_T("Z"), (double)curtime, cureul.z);
						//s_owpEulerGraph->newKey(_T("S"), (double)curtime, 0.0);
						

						if (minfirstflag == 1) {
							minval = min(cureul.z, min(cureul.x, cureul.y));
							minfirstflag = 0;
						}
						if (minval > cureul.x) {
							minval = cureul.x;
						}
						if (minval > cureul.y) {
							minval = cureul.y;
						}
						if (minval > cureul.z) {
							minval = cureul.z;
						}

						if (maxfirstflag == 1) {
							maxval = max(cureul.z, max(cureul.x, cureul.y));
							maxfirstflag = 0;
						}
						if (maxval < cureul.x) {
							maxval = cureul.x;
						}
						if (maxval < cureul.y) {
							maxval = cureul.y;
						}
						if (maxval < cureul.z) {
							maxval = cureul.z;
						}

					}

					//_ASSERT(0);
					s_owpEulerGraph->setEulMinMax(s_ikkind, minval, maxval);

					if (g_motionbrush_value) {

						double scalemin, scalemax;
						if (minval != maxval) {
							scalemin = minval;
							scalemax = maxval;
						}
						else {
							//Eulerが全て０　例えば全フレームを選択してツールの姿勢初期化を実行した後など
							//仮のminとmaxを指定
							scalemin = minval;
							scalemax = maxval + 10.0;
						}

						
						unsigned int scaleindex;
						for (scaleindex = 0; scaleindex < frameleng; scaleindex++) {
							double curscalevalue;
							if ((scaleindex >= (int)g_motionbrush_startframe) && (scaleindex <= (int)g_motionbrush_endframe) && (scaleindex < (int)g_motionbrush_frameleng)) {
								curscalevalue = (double)(*(g_motionbrush_value + scaleindex));// *(scalemax - scalemin) + scalemin;
								curscalevalue = (curscalevalue * 0.5 + 0.5) * (scalemax - scalemin) + scalemin;
							}
							else {
								curscalevalue = 0.0;// *(scalemax - scalemin) + scalemin;
								curscalevalue = (curscalevalue * 0.5 + 0.5) * (scalemax - scalemin) + scalemin;
							}
							s_owpEulerGraph->newKey(_T("S"), (double)scaleindex, curscalevalue);//newkey
						}
					}
				}
			}
		}

		//s_owpEulerGraph->setCurrentTime(0.0, false);

	}

	return 0;
}


//タイムラインにモーションデータのキーを設定する
void refreshTimeline(OWP_Timeline& timeline){

	if (!s_model || !s_owpLTimeline || !s_owpEulerGraph) {
		return;
	}

	//if (s_model && (s_model->GetLoadedFlag() == false)) {
	//	return;
	//}

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
	timeline.deleteKey();
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
	s_owpLTimeline->setCurrentTime(1.0, true);
	//timeline.setCurrentTime(0.0);


 //   CDXUTComboBox* pComboBox = g_SampleUI.GetComboBox( IDC_COMBO_BONE );
	//pComboBox->RemoveAllItems();
	//if( s_model ){
	//	map<int, CBone*>::iterator itrbone;
	//	for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
	//		ULONG boneno = (ULONG)itrbone->first;
	//		CBone* curbone = itrbone->second;
	//		if( curbone && (boneno >= 0) ){
	//			char* nameptr = (char*)curbone->GetBoneName();
	//			WCHAR wname[256];
	//			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nameptr, 256, wname, 256 );
	//			pComboBox->AddItem( wname, ULongToPtr( boneno ) );
	//		}
	//	}
	//}

	refreshEulerGraph();
	s_owpEulerGraph->setCurrentTime(1.0, false);
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

int AddBoneScale2(ChaVector3 diffvec)
{
	if (!s_model || (s_curboneno < 0) && !s_model->GetTopBone()) {
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (!curbone) {
		_ASSERT(0);
		return 0;
	}


	float scaleval;

	float upval = 1.1f;
	float downval = 0.9f;


	if (fabs(diffvec.x) >= fabs(diffvec.y)) {
		if (fabs(diffvec.x) >= fabs(diffvec.z)) {
			//x最大
			if (diffvec.x >= 0.0) {
				scaleval = upval;
			}
			else {
				scaleval = downval;
			}
		}
		else {
			//z最大
			if (diffvec.z >= 0.0) {
				scaleval = upval;
			}
			else {
				scaleval = downval;
			}
		}
	}
	else {
		if (fabs(diffvec.y) >= fabs(diffvec.z)) {
			//y最大
			if (diffvec.y >= 0.0) {
				scaleval = upval;
			}
			else {
				scaleval = downval;
			}
		}
		else {
			//z最大
			if (diffvec.z >= 0.0) {
				scaleval = upval;
			}
			else {
				scaleval = downval;
			}
		}
	}

	ChaVector3 scalevec;
	scalevec.x = scaleval;
	scalevec.y = scaleval;
	scalevec.z = scaleval;

	s_model->FKBoneScale(0, &s_editrange, s_curboneno, scalevec);

	s_editmotionflag = s_curboneno;

	return 0;
}

int AddBoneScale(int kind, float srcscale)
{
	if (!s_model || (s_curboneno < 0) && !s_model->GetTopBone()) {
		return 0;
	}

	CBone* curbone = s_model->GetBoneByID(s_curboneno);
	if (!curbone) {
		_ASSERT(0);
		return 0;
	}

	float upval = 1.1f;
	float downval = 0.9f;


	float scaleval;
	if (srcscale >= 0.0) {
		scaleval = upval;
	}
	else {
		scaleval = downval;
	}

	s_model->FKBoneScaleAxis(0, &s_editrange, s_curboneno, kind, scaleval);


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
	//#########################
	// not toggle : 2021/10/19
	//#########################


	bool savedispobj = s_dispobj;
	CreateLayerWnd();
	if (!s_layerWnd || !(s_layerWnd->getHWnd())) {
		return 0;
	}

	if(!savedispobj){
		s_layerWnd->setListenMouse(false);
		s_layerWnd->setVisible( false );
		s_dispobj = false;
	}else{
		s_layerWnd->setListenMouse(true);
		s_layerWnd->setVisible( true );
		s_dispobj = true;

		//RECT dlgrect;
		//GetWindowRect(s_layerWnd->getHWnd(), &dlgrect);
		//SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
	}

	return 0;
}
int DispModelPanel()
{
	//#########################
	// not toggle : 2021/10/19
	//#########################


	bool savedispmodel = s_dispmodel;
	CreateModelPanel();
	if (!s_modelpanel.panel || !(s_modelpanel.panel->getHWnd())) {
		return 0;
	}

	if(!savedispmodel){
		s_modelpanel.panel->setListenMouse(false);
		s_modelpanel.panel->setVisible( false );
		s_dispmodel = false;
	}else{
		s_modelpanel.panel->setListenMouse(true);
		s_modelpanel.panel->setVisible( true );
		s_dispmodel = true;

		//RECT dlgrect;
		//GetWindowRect(s_modelpanel.panel->getHWnd(), &dlgrect);
		//SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
	}


	DispMotionPanel();//!!!!!!!!!!!!!
	DispObjPanel();//!!!!!!!!!!!!!!!!


	return 0;
}

int DispMotionPanel()
{
	//#########################
	// not toggle : 2021/10/19
	//#########################


	bool savedispmotion = s_dispmotion;
	CreateMotionPanel();
	if (!s_motionpanel.panel || !(s_motionpanel.panel->getHWnd())) {
		return 0;
	}

	if (!savedispmotion) {
		s_motionpanel.panel->setListenMouse(false);
		s_motionpanel.panel->setVisible(false);
		s_dispmotion = false;
	}
	else {
		s_motionpanel.panel->setListenMouse(true);
		s_motionpanel.panel->setVisible(true);
		s_dispmotion = true;

		//RECT dlgrect;
		//GetWindowRect(s_motionpanel.panel->getHWnd(), &dlgrect);
		//SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
	}

	return 0;
}


//int DispConvBoneWindow()
//{
//	if (!s_model){
//		return 0;
//	}
//
//	CreateConvBoneWnd();
//
//	if (!s_convboneWnd){
//		return 0;
//	}
//
//	if (s_dispconvbone){
//		s_convboneWnd->setVisible(false);
//		s_dispconvbone = false;
//	}
//	else{
//		s_convboneWnd->setVisible(true);
//		s_dispconvbone = true;
//	}
//	return 0;
//}


int EraseKeyList()
{
	s_copyKeyInfoList.clear();
	s_copymotvec.clear();
	s_selectKeyInfoList.clear();
	s_deletedKeyInfoList.clear();

	return 0;
}

int AddMotion( WCHAR* wfilename, double srcmotleng )
{
	int motnum = (int)s_tlarray.size();
	if( motnum >= MAXMOTIONNUM ){
		::DSMessageBox(s_3dwnd, L"Can't Load More.", L"error!!!", MB_OK);
		//DSMessageBox( s_3dwnd, L"これ以上モーションを読み込めません。", L"モーション数が多すぎます。", MB_OK );
		return 0;
	}

	char motionname[256];
	ZeroMemory( motionname, 256 );
	SYSTEMTIME systime;
	GetLocalTime( &systime );
	sprintf_s( motionname, 256, "motion%u%u%u%u%u%u%u",
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

	CallF( AddTimeLine( newmotid, true ), return 1 );

	int selindex = (int)s_tlarray.size() - 1;
	CallF( OnAnimMenu( true, selindex ), return 1 );


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


int OnAnimMenu( bool dorefreshflag, int selindex, int saveundoflag )
{

	s_underselectmotion = true;

	//大きいフレーム一のまま小さいフレーム長のデータを読み込んだ時にエラーにならないように。
	InitTimelineSelection();

	if (!s_model) {
		_ASSERT(0);
		SetMainWindowTitle();
		s_underselectmotion = false;
		return 0;
	}

	s_motmenuindexmap[s_model] = selindex;

	if( selindex < 0 ){
		SetMainWindowTitle();
		s_underselectmotion = false;
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
		SetMainWindowTitle();
		s_underselectmotion = false;
		return 0;//!!!!!!!!!!!!!!!!!!
	}

	cAnimSets = (int)s_tlarray.size();

	if( cAnimSets <= 0 ){
		if( s_owpTimeline && dorefreshflag){
			refreshTimeline(*s_owpTimeline);
		}
		s_curmotid = -1;
		SetMainWindowTitle();
		s_underselectmotion = false;
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
			//s_owpTimeline->setCurrentTime( 0.0 );
			s_owpTimeline->setCurrentTime(1.0);
			s_curmotid = selmotid;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			if (dorefreshflag) {
				s_model->CreateBtObject(1);
				//s_model->CalcBoneEul(-1);
				s_model->CalcBoneEul(selmotid);//2021/08/25
			}
		}
	}

	if( s_owpTimeline && dorefreshflag){
		//タイムラインのキーを設定
		refreshTimeline(*s_owpTimeline);
		//s_owpTimeline->setCurrentTime( 0.0 );
		s_owpTimeline->setCurrentTime(1.0);
	}

	//MOTINFO* curmi = s_model->GetCurMotInfo();
	//if (curmi) {
	//	OnAddMotion(curmi->motid);
	//}

	if( saveundoflag == 1 ){
		if( s_model ){
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
		}
	}else{
		if( s_model ){
			//double curframe = s_model->GetCurMotInfo()->curframe;
			double curframe = 1.0;
			s_owpLTimeline->setCurrentTime( curframe, true );
			s_owpEulerGraph->setCurrentTime(curframe, false);
		}
	}

	s_owpLTimeline->selectClear();


	//refreshMotionPanel();
	DispMotionPanel();

	SetMainWindowTitle();


	InitTimelineSelection();


	s_underselectmotion = false;




	if (s_model->GetInitAxisMatX() == 0){//OnAnimMenuに移動
		s_owpLTimeline->setCurrentTime(0.0, true);
		s_owpEulerGraph->setCurrentTime(0.0, false);
		s_model->SetMotionFrame(0.0);
		s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
		//ここでAxisMatXの初期化
		s_model->CreateBtObject(1);
		s_model->CalcBtAxismat(2);//2
		s_model->SetInitAxisMatX(1);
	}


	return 0;
}

int OnModelMenu( bool dorefreshtl, int selindex, int callbymenu )
{
	s_underselectmodel = true;

	s_customrigbone = 0;
	
	if (s_anglelimitdlg){
		s_underanglelimithscroll = 0;
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
		OrgWindowListenMouse(false);

		s_model = 0;
		s_curboneno = -1;
		if( s_owpTimeline && dorefreshtl){
			refreshTimeline(*s_owpTimeline);
		}
		refreshModelPanel();
		DispMotionPanel();

		SetMainWindowTitle();

		s_underselectmodel = false;
		return 0;//!!!!!!!!!
	}

	cMdlSets = (int)s_modelindex.size();
	if( cMdlSets <= 0 ){
		OrgWindowListenMouse(false);

		s_model = 0;
		if( s_owpTimeline && dorefreshtl){
			refreshTimeline(*s_owpTimeline);
		}
		refreshModelPanel();
		DispMotionPanel();

		SetMainWindowTitle();

		s_underselectmodel = false;
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
		if (s_model) {
			s_tlarray = s_modelindex[selindex].tlarray;
			s_motmenuindexmap[s_model] = s_modelindex[selindex].motmenuindex;
			s_lineno2boneno = s_modelindex[selindex].lineno2boneno;
			s_boneno2lineno = s_modelindex[selindex].boneno2lineno;

			refreshModelPanel();

			OnAnimMenu(dorefreshtl, s_motmenuindexmap[s_model]);
		}
	}
	//else {
		//大きいフレーム位置のまま小さいフレーム長のデータを読み込んだ時にエラーにならないように。
		InitTimelineSelection();
	//}


	if( s_model ){
		if( s_model->GetRigidElemInfoSize() > 0 ){
			int result1 = OnREMenu( 0, 0 );
			if (result1) {
				s_underselectmodel = false;
				return 0;
			}
			int result2 = OnRgdMenu( 0, 0 );
			if (result2) {
				s_underselectmodel = false;
				return 0;
			}
			int result3 = OnImpMenu( 0 );
			if (result3) {
				s_underselectmodel = false;
				return 0;
			}
		}else{
			int result1 = OnREMenu(-1, 0);
			if (result1) {
				s_underselectmodel = false;
				return 0;
			}
			int result2 = OnRgdMenu(-1, 0);
			if (result2) {
				s_underselectmodel = false;
				return 0;
			}
			int result3 = OnImpMenu(-1);
			if (result3) {
				s_underselectmodel = false;
				return 0;
			}
		}
	}else{
		int result1 = OnREMenu(-1, 0);
		if (result1) {
			s_underselectmodel = false;
			return 0;
		}
		int result2 = OnRgdMenu(-1, 0);
		if (result2) {
			s_underselectmodel = false;
			return 0;
		}
		int result3 = OnImpMenu(-1);
		if (result3) {
			s_underselectmodel = false;
			return 0;
		}
	}


	g_SampleUI.GetSlider( IDC_SPEED )->SetValue( (int)( g_dspeed * 100.0f ) );
	WCHAR sz[100];
	swprintf_s( sz, 100, L"Speed: %0.4f", g_dspeed );
    g_SampleUI.GetStatic( IDC_SPEED_STATIC )->SetText( sz );

	//if (!g_bvh2fbxbatchflag && !g_motioncachebatchflag && !g_retargetbatchflag) {
	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 0) && (InterlockedAdd(&g_motioncachebatchflag, 0) == 0) && (InterlockedAdd(&g_retargetbatchflag, 0) == 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 0) && (InterlockedAdd(&g_retargetbatchflag, 0) == 0)) {
		CreateConvBoneWnd();//!!!!!!!!!!!!! モデル選択変更によりリターゲットウインドウ作り直し
	}

	SetMainWindowTitle();

	SetTimelineHasRigFlag();

	s_underselectmodel = false;

	return 0;
}

int OnREMenu( int selindex, int callbymenu )
{
	if (!s_model) {
		_ASSERT(0);
		SetMainWindowTitle();
		return 0;
	}
	s_reindexmap[s_model] = selindex;

	_ASSERT( s_remenu );
	int iReSet, cReSets;
	cReSets = GetMenuItemCount( s_remenu );
	for (iReSet = 0; iReSet < cReSets; iReSet++)
	{
		RemoveMenu(s_remenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		AppendMenu(s_remenu, MF_STRING, 62000, L"NotLoaded" );
		SetMainWindowTitle();
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetRigidElemInfoSize();
	if( cReSets <= 0 ){
		if (s_model) {
			s_reindexmap[s_model] = -1;
		}
		AppendMenu(s_remenu, MF_STRING, 62000, L"NotLoaded" );
		SetMainWindowTitle();
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

	CallF( s_model->SetCurrentRigidElem( s_reindexmap[s_model] ), return 1 );

	RigidElem2WndParam();
	
	SetMainWindowTitle();

	return 0;
}

int OnRgdMenu( int selindex, int callbymenu )
{
	if( s_model ){
		s_model->SetRgdIndex( selindex );
	}
	else {
		_ASSERT(0);
		return 0;
	}
	s_rgdindexmap[s_model] = selindex;

	_ASSERT( s_rgdmenu );
	int iReSet, cReSets;
	cReSets = GetMenuItemCount( s_rgdmenu );
	for (iReSet = 0; iReSet < cReSets; iReSet++)
	{
		RemoveMenu(s_rgdmenu, 0, MF_BYPOSITION);
	}

	if( (selindex < 0) || !s_model ){
		AppendMenu(s_rgdmenu, MF_STRING, 63000, L"NotLoaded" );
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetRigidElemInfoSize();
	if( cReSets <= 0 ){
		s_rgdindexmap[s_model] = -1;
		AppendMenu(s_rgdmenu, MF_STRING, 63000, L"NotLoaded" );
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
		AppendMenu(s_impmenu, MF_STRING, 64500, L"NotLoaded" );
		return 0;//!!!!!!!!!
	}

	cReSets = s_model->GetImpInfoSize();
	if( cReSets <= 0 ){
		s_model->SetCurImpIndex( 0 );
		AppendMenu(s_impmenu, MF_STRING, 64500, L"NotLoaded" );
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

int OnDelMotion( int delmenuindex, bool ondelbutton )//default : ondelbutton = false
{
	s_underdelmotion = true;

	int tlnum = (int)s_tlarray.size();
	if( (tlnum <= 0) || (delmenuindex < 0) || (delmenuindex >= tlnum) ){
		s_underdelmotion = false;
		return 0;
	}

	int delmotid = s_tlarray[ delmenuindex ].motionid;
	int result;
	result = s_model->DeleteMotion(delmotid);
	if (result) {
		s_underdelmotion = false;
		return 1;
	}


	int tlno;
	for (tlno = delmenuindex; tlno < (tlnum - 1); tlno++) {
		s_tlarray[tlno] = s_tlarray[tlno + 1];
	}
	s_tlarray.pop_back();

	int newtlnum = (int)s_tlarray.size();
	if( newtlnum == 0 ){
		AddMotion( 0 );
		InitCurMotion(0, 0);
	}else{
		OnAnimMenu( true, 0 );
	}

	DispMotionPanel();

	s_underdelmotion = false;

	return 0;
}

int OnDelModel( int delmenuindex, bool ondelbutton )//default : ondelbutton == false
{
	s_underdelmodel = true;

	int mdlnum = (int)s_modelindex.size();
	if( (mdlnum <= 0) || (delmenuindex < 0) || (delmenuindex >= mdlnum) ){
		s_underdelmodel = false;
		return 0;
	}

	if (mdlnum == 1){
		OnDelAllModel();//psdk rootnode初期化
		s_underdelmodel = false;
		return 0;
	}


	CModel* delmodel = s_modelindex[ delmenuindex ].modelptr;
	if( delmodel ){
		//map<CModel*, int>::iterator itrbonecnt;
		//itrbonecnt = g_bonecntmap.find(delmodel);
		//if (itrbonecnt != g_bonecntmap.end()){
		//	g_bonecntmap.erase(itrbonecnt);
		//}

		CBone::OnDelModel(delmodel);


		//FbxScene* pscene = delmodel->GetScene();
		//if (pscene){
		//	pscene->Destroy();
		//}
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
		s_motmenuindexmap.clear();
		s_lineno2boneno.clear();
		s_boneno2lineno.clear();
	}else{
		s_curboneno = -1;
		s_model = s_modelindex[ 0 ].modelptr;
		if (s_model) {
			s_motmenuindexmap[s_model] = s_modelindex[0].motmenuindex;
		}
		s_tlarray = s_modelindex[0].tlarray;
		s_lineno2boneno = s_modelindex[0].lineno2boneno;
		s_boneno2lineno = s_modelindex[0].boneno2lineno;
	}

	OnModelMenu( true, 0, 0 );

	DispModelPanel();

	if (!s_model) {
		OrgWindowListenMouse(false);
	}

	s_underdelmodel = false;

	return 0;
}

int OnDelAllModel()
{
	int mdlnum = (int)s_modelindex.size();
	if( mdlnum <= 0 ){
		return 0;
	}

	//g_bonecntmap.clear();

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* delmodel = itrmodel->modelptr;
		if( delmodel ){
			CBone::OnDelModel(delmodel);

			FbxScene* pscene = delmodel->GetScene();
			//if (pscene){
			//	pscene->Destroy();
			//}
			delete delmodel;
		}
		itrmodel->tlarray.clear();
		itrmodel->boneno2lineno.clear();
		itrmodel->lineno2boneno.clear();
	}

	OrgWindowListenMouse(false);

	s_modelindex.clear();

	s_curboneno = -1;
	s_model = 0;
	s_curmodelmenuindex = -1;
	s_tlarray.clear();
	s_motmenuindexmap.clear();
	s_lineno2boneno.clear();
	s_boneno2lineno.clear();

	OnModelMenu( true, -1, 0 );

	DispModelPanel();

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
	mb->r = (float)ChaVector3LengthDbl( &diff );

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
		//if( s_curmodelmenuindex >= 0 ){
			s_modelpanel.modelindex = s_curmodelmenuindex;
			s_modelpanel.radiobutton->setSelectIndex( s_modelpanel.modelindex );
		//}
	}

	return 0;
}

//int refreshMotionPanel()
//{
//	if (s_motionpanel.radiobutton && ((int)s_modelindex.size() > 0) && (s_curmodelmenuindex >= 0)) {
//		if (s_model && (s_model->GetMotInfoSize() > 0) && s_motmenuindexmap[s_model] >= 0) {
//			s_motionpanel.radiobutton->setSelectIndex(s_motmenuindexmap[s_model]);
//		}
//	}
//
//	return 0;
//}


float CalcSelectScale(CBone* curboneptr)
{
	MODELBOUND mb;
	s_model->GetModelBound(&mb);
	double modelr = (double)mb.r;

	s_selectscale = modelr * 0.0020;
	if (s_oprigflag == 1) {
		s_selectscale *= 0.125f;
		//s_selectscale *= 0.50f;
	}
	//if (g_4kresolution) {
	//	s_selectscale *= 0.5f;
	//}


	//ChaMatrix selm;
	//selm = s_selm;
	//ChaMatrixNormalizeRot(&selm);
	//ChaVector3 orgcenterpos = curboneptr->GetJointFPos();
	//ChaVector3 orgedgepos = ChaVector3(orgcenterpos.x, orgcenterpos.y + 100.0f, orgcenterpos.z);
	//ChaVector3 dispcenterpos, dispedgepos;
	//ChaVector3TransformCoord(&dispcenterpos, &orgcenterpos, &selm);
	//ChaVector3TransformCoord(&dispedgepos, &orgedgepos, &selm);
	//double lineleng = (dispedgepos.x - dispcenterpos.x) * (dispedgepos.x - dispcenterpos.x) + (dispedgepos.y - dispcenterpos.y) * (dispedgepos.y - dispcenterpos.y);
	//if (lineleng > 0.00001) {
	//	lineleng = sqrt(lineleng);
	//	//s_selectscale = 0.0020f / lineleng;
	//	//s_selectscale = (modelr * 0.015f * 0.75f) / (lineleng * 100.0f);
	//	s_selectscale = (float)((modelr * 0.40) / lineleng);// *(s_camdist / g_initcamdist);
	//	if (s_oprigflag == 1) {
	//		s_selectscale *= 0.25f;
	//		//s_selectscale *= 0.50f;
	//	}
	//	if (g_4kresolution) {
	//		s_selectscale *= 0.5f;
	//	}
	//}
	//else {
	//	//s_selectscaleの計算はしない。s_selectscaleは前回の計算値を使用。
	//}

	return s_selectscale;
}


int RenderSelectMark(ID3D11DeviceContext* pd3dImmediateContext, int renderflag)
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
			int calccapsuleflag = 0;
			s_selm_posture = curboneptr->CalcManipulatorPostureMatrix(calccapsuleflag, 0, 0, multworld, 0);
		}


		CalcSelectScale(curboneptr);//s_selectscaleにセット

		ChaMatrix scalemat;
		ChaMatrixIdentity( &scalemat );
		ChaMatrixScaling(&scalemat, s_selectscale, s_selectscale, s_selectscale);

		ChaVector3 bonepos = curboneptr->GetWorldPos(curmi->motid, curmi->curframe);

		//s_selectmat = scalemat * s_selm;
		s_selectmat = s_selm;
		ChaMatrixNormalizeRot(&s_selectmat);
		s_selectmat = scalemat * s_selectmat;

		s_selectmat._41 = bonepos.x;
		s_selectmat._42 = bonepos.y;
		s_selectmat._43 = bonepos.z;

		//s_selectmat_posture = scalemat * s_selm_posture;
		s_selectmat_posture = s_selm_posture;
		ChaMatrixNormalizeRot(&s_selectmat_posture);
		s_selectmat_posture = scalemat * s_selectmat_posture;

		s_selectmat_posture._41 = bonepos.x;
		s_selectmat_posture._42 = bonepos.y;
		s_selectmat_posture._43 = bonepos.z;

		if (renderflag){
			g_hmVP->SetMatrix((float*)&s_matVP);

			g_hmWorld->SetMatrix((float*)&s_selectmat);
			RenderSelectFunc(pd3dImmediateContext);


			g_hmWorld->SetMatrix((float*)&s_selectmat_posture);
			if (s_oprigflag == 0) {
				RenderSelectPostureFunc(pd3dImmediateContext);
			}
			else {
				if (curboneptr == s_customrigbone) {
				}
				else {
					RenderSelectPostureFunc(pd3dImmediateContext);
				}
			}


			//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);

		}
	}

	return 0;
}

int RenderSelectFunc(ID3D11DeviceContext* pd3dImmediateContext)
{
	s_select->UpdateMatrix(&s_selectmat, &s_matVP);
	//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
	if (s_dispselect){
		int lightflag = 1;
		//ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 0.7f);
		s_select->OnRender(pd3dImmediateContext, lightflag, diffusemult);
	}
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);

	return 0;

}

int RenderSelectPostureFunc(ID3D11DeviceContext* pd3dImmediateContext)
{
	s_select_posture->UpdateMatrix(&s_selectmat_posture, &s_matVP);
	//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
	if (s_dispselect) {
		int lightflag = 1;
		//ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 0.7f);
		s_select_posture->OnRender(pd3dImmediateContext, lightflag, diffusemult);
	}
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);

	return 0;

}


int RenderRigMarkFunc(ID3D11DeviceContext* pd3dImmediateContext)
{
	if (!s_model) {
		return 0;
	}

	g_hmVP->SetMatrix((float*)&s_matVP);


	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (curmi) {
		int curmotid = curmi->motid;
		double curframe = curmi->curframe;

		std::map<int, CBone*>::iterator itrbone;
		for (itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++) {
			CBone* curbone = (CBone*)itrbone->second;
			if (curbone) {
				int rigno;
				for (rigno = 0; rigno < MAXRIGNUM; rigno++) {
					CUSTOMRIG currig = curbone->GetCustomRig(rigno);
					if (currig.useflag == 2) {//0: free, 1: allocated, 2: valid
					//if (currig.rigboneno > 0) {
						ChaVector3 curbonepos = curbone->GetWorldPos(curmotid, curframe);
						ChaMatrix rigmat;
						ChaMatrixIdentity(&rigmat);
						rigmat = CalcRigMat(curbone, curmotid, curframe, currig.dispaxis, currig.disporder, currig.posinverse);

						if (currig.dispaxis == 0) {
							s_matrigmat = ChaVector4(1.0f, 0.5f, 0.5f, 0.79f);
						}
						else if (currig.dispaxis == 1) {
							s_matrigmat = ChaVector4(0.0f, 1.0f, 0.0f, 0.79f);
						}
						else if (currig.dispaxis == 2) {
							s_matrigmat = ChaVector4(15.0f / 255.0f, 200.0f / 255.0f, 1.0f, 0.79f);
						}
						s_matrig->SetDif4F(s_matrigmat);


						g_hmWorld->SetMatrix((float*)&rigmat);

						s_rigmark->UpdateMatrix(&rigmat, &s_matVP);
						//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
						pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
						int lightflag = 0;
						//ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
						ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
						s_rigmark->OnRender(pd3dImmediateContext, lightflag, diffusemult);
						//s_pdev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
						pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);
					}
				}
			}
		}
	}

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
	//以下2行。相対位置で動かすことができるが、マウスが可動でボーンが可動でないような位置への操作があると、その後の操作と結果の関係が不自然にみえる。
	mousesc.x = s_pickinfo.objscreen.x + s_pickinfo.diffmouse.x;
	mousesc.y = s_pickinfo.objscreen.y + s_pickinfo.diffmouse.y;

	//以下２行。常にマウス位置を目標にする。
	//mousesc.x = s_pickinfo.mousepos.x;
	//mousesc.y = s_pickinfo.mousepos.y;
	mousesc.z = s_pickinfo.objscreen.z;

	ChaVector3 startsc, endsc;
	float rayx, rayy;
	rayx = mousesc.x / (s_pickinfo.winx / 2.0f) - 1.0f;
	rayy = 1.0f - mousesc.y / (s_pickinfo.winy / 2.0f);

	startsc = ChaVector3( rayx, rayy, 0.0f );
	endsc = ChaVector3( rayx, rayy, 1.0f );
	
    ChaMatrix mView;
    ChaMatrix mProj;
    mProj = g_Camera->GetProjMatrix();
    mView = s_matView;
	ChaMatrix mVP, invmVP;
	mVP = mView * mProj;
	ChaMatrixInverse( &invmVP, NULL, &mVP );

	ChaVector3TransformCoord( startptr, &startsc, &invmVP );
	ChaVector3TransformCoord( endptr, &endsc, &invmVP );

	return 0;
}

////s_openfilehwnd
////UINT CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
////UINT CALLBACK GetFileNameHook(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
////UINT_PTR CALLBACK GetFileNameHook(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
////{
////
////	static int s_getfilenametimer = 338;
////
////	WORD loword;
////	WORD hiword;
////
////	switch (msg) {
////	case WM_INITDIALOG:
////		SetTimer(hDlgWnd, s_getfilenametimer, 20, NULL);
////		s_openfilehwnd = hDlgWnd;
////		return FALSE;
////	case WM_COMMAND:
////		switch (LOWORD(wp)) {
////		case IDOK:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDOK);
////			return TRUE;
////			break;
////		case IDCANCEL:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDCANCEL);
////			return TRUE;
////			break;
////		default:
////			return FALSE;
////		}
////	case WM_LBUTTONDOWN:
////		loword = LOWORD(wp);
////		hiword = HIWORD(wp);
////		break;
////	case WM_LBUTTONUP:
////		loword = LOWORD(wp);
////		hiword = HIWORD(wp);
////
////		switch (LOWORD(wp)) {
////		case IDOK:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDOK);
////			return TRUE;
////			break;
////		case IDCANCEL:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDCANCEL);
////			return TRUE;
////			break;
////		default:
////			return FALSE;
////		}
////		break;
////	case WM_NOTIFY:
/////*
////WM_NOTIFYメッセージは、標準のコントロールやユーザーの操作によるさまざまな通知メッセージが送られたときに送られてきます。
////lParamにはOFNOTIFY構造体のアドレスが入っていてこれを調べると通知メッセージの種類がわかります。
////この構造体の説明はここでは省略します。通知メッセージとしては、
////CDN_INITDONE          ダイアログ配置が終わったことを知らせます。
////CDN_FILEOK            ＯＫボタンが押されたことを知らせます。
////CDN_FOLDERCHANGE      開いているフォルダが変わったことを知らせます。
////CDN_HELP              ＨＥＬＰボタンが押されたことを知らせます。
////CDN_SELCHANGE         別のファイルが選ばれたことを知らせます。
////CDN_SHAREVIOLATION    共有違反が発生したことを知らせます。
////CDN_TYPECHANGE        ファイルの種類が変更されたことを知らせます。
////また、ダイアログの状態を知るためには次のようなメッセージを送ります。
////それぞれのメッセージについては各自で調べてください（今回こればっか、手抜きだというのがばれてしまう）。
////CDM_GETFILEPATH       選択されているファイルのパス（フォルダ名＋ファイル名）を取得します。
////CDM_GETSPEC           選択されているファイルのファイル名だけを取得します。
////CDM_GETFOLDERPATH     現在のフォルダのパスを取得します。
////CDM_GETFOLDERIDLIST   現在のフォルダのItem-ID-Listを取得します。
////CDM_HIDECONTROL       指定したコントロールを隠します。
////CDM_SETCONTROLTEXT    指定したコントロールにテキストを設定します。
////CDM_SETDEFEXT         表示するファイルの拡張子を設定します。*/
////		loword = LOWORD(wp);
////		hiword = HIWORD(wp);
////
////		switch (LOWORD(wp)) {
////		case IDOK:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDOK);
////			return TRUE;
////			break;
////		case IDCANCEL:
////			s_openfilehwnd = 0;
////			KillTimer(hDlgWnd, s_getfilenametimer);
////			EndDialog(hDlgWnd, IDCANCEL);
////			return TRUE;
////			break;
////		default:
////			return FALSE;
////		}
////		break;
////	case WM_TIMER:
////		OnDSUpdate();
////		return FALSE;
////		break;
////	default:
////		DefWindowProc(hDlgWnd, msg, wp, lp);
////		return FALSE;
////	}
////	return TRUE;
////
////
////}

LRESULT CALLBACK OpenMqoDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	
	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = s_3dwnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"project(*.cha)chara(*.fbx)\0*.cha;*.fbx\0Rigid(*.ref)\0*.ref\0Impulse(*.imp)\0*.imp\0Ground(*.gco)\0*.gco\0BVH(*.bvh)\0*.bvh\0MNL(*.mnl)\0*.mnl\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = s_filterindex;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	//ofn.lpstrTitle = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_ENABLEHOOK;
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt =NULL;
	ofn.lCustData = NULL;
	//ofn.lpfnHook = GetFileNameHook;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	WCHAR strmult[256];
	wcscpy_s( strmult, 256, L"1.000" );
	
	static int s_openmqoproctimer = 336;
	static bool s_refokflag = false;

	switch (msg) {
		case WM_INITDIALOG: 
			{
			s_refokflag = false;
			g_tmpmqomult = 1.0f;
			ZeroMemory(g_tmpmqopath, sizeof(WCHAR) * MULTIPATH);
			SetDlgItemText(hDlgWnd, IDC_MULT, strmult);
			SetDlgItemText(hDlgWnd, IDC_FILEPATH, L"PushRefButtonToSelectFile.");

			if (s_filterindex == 5) {
				//bvh2FBXの単体ファイル履歴
				std::vector<wstring> vecopenfilename;
				GetbvhHistoryDir(vecopenfilename);

				int radiocnt = 0;
				int radionum = min(OPENHISTORYMAXNUM, (int)vecopenfilename.size());
				if (radionum != 0) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO1), vecopenfilename[0].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO1), L"Loading History not Exist.");
				}
				if (radionum >= 2) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO2), vecopenfilename[1].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO2), L"Loading History not Exist.");
				}
				if (radionum >= 3) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO3), vecopenfilename[2].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO3), L"Loading History not Exist.");
				}
				if (radionum >= 4) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO4), vecopenfilename[3].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO4), L"Loading History not Exist.");
				}
				if (radionum >= 5) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO5), vecopenfilename[4].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO5), L"Loading History not Exist.");
				}
				if (radionum >= 6) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO6), vecopenfilename[5].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO6), L"Loading History not Exist.");
				}
				if (radionum >= 7) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO7), vecopenfilename[6].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO7), L"Loading History not Exist.");
				}
				if (radionum >= 8) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO8), vecopenfilename[7].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO8), L"Loading History not Exist.");
				}
				if (radionum >= 9) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO9), vecopenfilename[8].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO9), L"Loading History not Exist.");
				}
				if (radionum >= 10) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO10), vecopenfilename[9].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO10), L"Loading History not Exist.");
				}


			}
			else {
				//MB3DOpenProj_20210410215628.txt
				WCHAR searchfilename[MAX_PATH] = { 0L };
				swprintf_s(searchfilename, MAX_PATH, L"%sMB3DOpenProj_*.txt", s_temppath);
				HANDLE hFind;
				WIN32_FIND_DATA win32fd;
				hFind = FindFirstFileW(searchfilename, &win32fd);

				std::vector<HISTORYELEM> vechistory;//!!!!!!!!! tmpファイル名
				std::vector<wstring> vecopenfilename;//!!!!!!!! tmpファイル内に書いてあるopenfilename

				vechistory.clear();
				bool notfoundfirst = true;
				if (hFind != INVALID_HANDLE_VALUE) {
					notfoundfirst = false;
					do {
						if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

							HISTORYELEM curelem;
							curelem.filetime = win32fd.ftCreationTime;

							//printf("%s\n", win32fd.cFileName);
							curelem.wfilename[MAX_PATH - 1] = { 0L };
							swprintf_s(curelem.wfilename, MAX_PATH, L"%s%s", s_temppath, win32fd.cFileName);

							vechistory.push_back(curelem);
						}
					} while (FindNextFile(hFind, &win32fd));
					FindClose(hFind);
				}

				if (!vechistory.empty()) {

					std::sort(vechistory.begin(), vechistory.end());
					std::reverse(vechistory.begin(), vechistory.end());

					int numhistory = (int)vechistory.size();
					int dispnum = min(OPENHISTORYMAXNUM, numhistory);



					int foundnum = 0;
					int historyno;
					for (historyno = 0; historyno < numhistory; historyno++) {
						WCHAR openfilename[MAX_PATH] = { 0L };
						wcscpy_s(openfilename, MAX_PATH, vechistory[historyno].wfilename);

						HANDLE hfile;
						hfile = CreateFileW(openfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
							FILE_FLAG_SEQUENTIAL_SCAN, NULL);
						if (hfile != INVALID_HANDLE_VALUE) {
							WCHAR readwstr[MAX_PATH] = { 0L };
							DWORD readleng = 0;
							bool bsuccess;
							bsuccess = ReadFile(hfile, readwstr, (MAX_PATH * sizeof(WCHAR)), &readleng, NULL);
							if (bsuccess) {
								bool foundsame = false;
								wstring newwstr = readwstr;
								std::vector<wstring>::iterator itropenfilename;
								for (itropenfilename = vecopenfilename.begin(); itropenfilename != vecopenfilename.end(); itropenfilename++) {
									if (newwstr.compare(*itropenfilename) == 0) {
										foundsame = true;
									}
								}
								if (foundsame == false) {
									vecopenfilename.push_back(readwstr);
									foundnum++;
									if (foundnum >= dispnum) {
										break;
									}
								}
							}
							CloseHandle(hfile);
						}
					}
				}

				int radiocnt = 0;
				int radionum = min(OPENHISTORYMAXNUM, (int)vecopenfilename.size());
				if (radionum != 0) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO1), vecopenfilename[0].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO1), L"Loading History not Exist.");
				}
				if (radionum >= 2) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO2), vecopenfilename[1].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO2), L"Loading History not Exist.");
				}
				if (radionum >= 3) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO3), vecopenfilename[2].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO3), L"Loading History not Exist.");
				}
				if (radionum >= 4) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO4), vecopenfilename[3].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO4), L"Loading History not Exist.");
				}
				if (radionum >= 5) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO5), vecopenfilename[4].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO5), L"Loading History not Exist.");
				}
				if (radionum >= 6) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO6), vecopenfilename[5].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO6), L"Loading History not Exist.");
				}
				if (radionum >= 7) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO7), vecopenfilename[6].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO7), L"Loading History not Exist.");
				}
				if (radionum >= 8) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO8), vecopenfilename[7].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO8), L"Loading History not Exist.");
				}
				if (radionum >= 9) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO9), vecopenfilename[8].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO9), L"Loading History not Exist.");
				}
				if (radionum >= 10) {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO10), vecopenfilename[9].c_str());
				}
				else {
					SetWindowTextW(GetDlgItem(hDlgWnd, IDC_RADIO10), L"Loading History not Exist.");
				}
			}

			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);

			SetTimer(hDlgWnd, s_openmqoproctimer, 20, NULL);
			s_mqodlghwnd = hDlgWnd;
			}
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText( hDlgWnd, IDC_MULT, strmult, 256 );
					g_tmpmqomult = (float)_wtof( strmult );
					if (s_refokflag == false) {
						UINT ischecked1 = 0;
						UINT ischecked2 = 0;
						UINT ischecked3 = 0;
						UINT ischecked4 = 0;
						UINT ischecked5 = 0;
						UINT ischecked6 = 0;
						UINT ischecked7 = 0;
						UINT ischecked8 = 0;
						UINT ischecked9 = 0;
						UINT ischecked10 = 0;
						ischecked1 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO1);
						ischecked2 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO2);
						ischecked3 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO3);
						ischecked4 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO4);
						ischecked5 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO5);
						ischecked6 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO6);
						ischecked7 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO7);
						ischecked8 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO8);
						ischecked9 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO9);
						ischecked10 = IsDlgButtonChecked(hDlgWnd, IDC_RADIO10);
						if (ischecked1 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO1, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
							else {
								g_tmpmqopath[0] = 0L;
							}
						}
						else if (ischecked2 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO2, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
							else {
								g_tmpmqopath[0] = 0L;
							}
						}
						else if (ischecked3 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO3, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked4 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO4, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked5 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO5, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked6 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO6, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked7 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO7, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked8 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO8, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked9 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO9, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else if (ischecked10 == BST_CHECKED) {
							WCHAR checkedpath[MAX_PATH] = { 0L };
							GetDlgItemTextW(hDlgWnd, IDC_RADIO10, checkedpath, MAX_PATH);
							if (wcscmp(L"Loading History not Exist.", checkedpath) != 0) {
								wcscpy_s(g_tmpmqopath, MAX_PATH, checkedpath);
							}
						}
						else {
							g_tmpmqopath[0] = 0L;
						}
					}
					if (g_tmpmqopath[0]) {
						//GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );
						s_mqodlghwnd = 0;
						KillTimer(hDlgWnd, s_openmqoproctimer);
						EndDialog(hDlgWnd, IDOK);
					}
					return TRUE;
                    break;
                case IDCANCEL:
					s_mqodlghwnd = 0;
					KillTimer(hDlgWnd, s_openmqoproctimer);
                    EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
                    break;
				case IDC_REFMQO:
				{
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
					HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
						WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
					InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

					if (GetOpenFileNameW(&ofn) == IDOK) {
						SetDlgItemText(hDlgWnd, IDC_FILEPATH, g_tmpmqopath);
						s_refokflag = true;
					}
					
					InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					UnhookWinEvent(hhook);
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
				}
					break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			return FALSE;
			break;
        default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
            return FALSE;
    }
    return TRUE;
}


LRESULT CALLBACK OpenBvhDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = s_3dwnd;
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
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
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

	static int s_openbvhproctimer = 352;


	switch (msg) {
	case WM_INITDIALOG:
		g_tmpmqomult = 1.0f;
		ZeroMemory(g_tmpmqopath, sizeof(WCHAR)* MULTIPATH);
		SetDlgItemText(hDlgWnd, IDC_MULT, strmult);
		SetDlgItemText(hDlgWnd, IDC_EDITFILTER, strfilter);
		SetDlgItemText(hDlgWnd, IDC_FILEPATH, L"PushRefButtonToSelectFile.");

		RECT dlgrect;
		GetWindowRect(hDlgWnd, &dlgrect);
		SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
		SetTimer(hDlgWnd, s_openbvhproctimer, 20, NULL);
		s_bvhdlghwnd = hDlgWnd;

		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			s_bvhdlghwnd = 0;
			KillTimer(hDlgWnd, s_openbvhproctimer);

			GetDlgItemText(hDlgWnd, IDC_MULT, strmult, 256);
			g_tmpmqomult = (float)_wtof(strmult);
			//GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );
			GetDlgItemText(hDlgWnd, IDC_EDITFILTER, strfilter, 256);
			g_tmpbvhfilter = (float)_wtof(strfilter);

			EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			s_bvhdlghwnd = 0;
			KillTimer(hDlgWnd, s_openbvhproctimer);

			EndDialog(hDlgWnd, IDCANCEL);
			break;
		case IDC_REFMQO:
		{
			s_getfilenamehwnd = 0;
			s_getfilenametreeview = 0;
			HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
				WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
			InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

			if (GetOpenFileNameW(&ofn) == IDOK) {
				SetDlgItemText(hDlgWnd, IDC_FILEPATH, g_tmpmqopath);
			}

			InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
			UnhookWinEvent(hhook);
			s_getfilenamehwnd = 0;
			s_getfilenametreeview = 0;
		}
			break;
		default:
			return FALSE;
		}
	case WM_TIMER:
		OnDSUpdate();
		return FALSE;
		break;
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
		break;
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

	static int s_motproptimerid = 345;


	switch (msg) {
        case WM_INITDIALOG:
		{
			if (s_model && s_model->GetCurMotInfo()) {
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
					s_model->GetCurMotInfo()->motname, 256, s_tmpmotname, 256);
				SetDlgItemText(hDlgWnd, IDC_MOTNAME, s_tmpmotname);

				s_tmpmotframeleng = s_model->GetCurMotInfo()->frameleng;
				swprintf_s(strframeleng, 256, L"%f", s_tmpmotframeleng);
				SetDlgItemText(hDlgWnd, IDC_FRAMELENG, strframeleng);

				s_tmpmotloop = s_model->GetCurMotInfo()->loopflag;
				SendMessage(GetDlgItem(hDlgWnd, IDC_LOOP), BM_SETCHECK, (WPARAM)s_tmpmotloop, 0L);
			}


			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);

			s_motpropdlghwnd = hDlgWnd;
			SetTimer(hDlgWnd, s_motproptimerid, 20, NULL);
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

					KillTimer(hDlgWnd, s_motproptimerid);
					s_motpropdlghwnd = 0;
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					KillTimer(hDlgWnd, s_motproptimerid);
					s_motpropdlghwnd = 0;
					EndDialog(hDlgWnd, IDCANCEL);
                    break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			break;
        default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
            return FALSE;
    }
    return TRUE;

}

LRESULT CALLBACK RetargetBatchDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR strnumcnt[1024] = { 0L };
	HWND hProg;

	switch (msg) {
	case WM_INITDIALOG:
		s_retargetbatchwnd = hDlgWnd;

		swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", (s_retargetcnt + 1), s_retargetnum);
		SetDlgItemTextW(s_retargetbatchwnd, IDC_STRBVH2FBXBATCH, strnumcnt);
		hProg = GetDlgItem(s_retargetbatchwnd, IDC_PROGRESS1);
		if (hProg) {
			//プログレスバーの範囲         
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
			//ステップの範囲を設定 
			SendMessage(hProg, PBM_SETSTEP, 1, 0);
			//現在位置を設定  
			SendMessage(hProg, PBM_SETPOS, 0, 0);
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			if (s_retargetbatchwnd) {
				DestroyWindow(s_retargetbatchwnd);
				s_retargetbatchwnd = 0;
			}
			InterlockedExchange(&g_retargetbatchflag, 2);
			//EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			if (s_retargetbatchwnd) {
				DestroyWindow(s_retargetbatchwnd);
				s_retargetbatchwnd = 0;
			}
			InterlockedExchange(&g_retargetbatchflag, 2);
			//EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	case WM_USER_FOR_BATCH_PROGRESS:
		break;
	case WM_CLOSE:
		if (s_retargetbatchwnd) {
			DestroyWindow(s_retargetbatchwnd);
			s_retargetbatchwnd = 0;
		}
		InterlockedExchange(&g_retargetbatchflag, 2);
		//EndDialog(hDlgWnd, IDOK);
		break;
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
		break;
	}
	return TRUE;
}



LRESULT CALLBACK ProgressDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	//static int s_timerid = 371;

	WCHAR strnumcnt[1024] = { 0L };
	HWND hProg;

	switch (msg) {
	case WM_INITDIALOG:
		s_progresswnd = hDlgWnd;

		swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", (s_progresscnt + 1), s_progressnum);
		SetDlgItemTextW(s_progresswnd, IDC_STRBVH2FBXBATCH, strnumcnt);
		hProg = GetDlgItem(s_progresswnd, IDC_PROGRESS1);
		if (hProg) {
			//プログレスバーの範囲         
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
			//ステップの範囲を設定 
			SendMessage(hProg, PBM_SETSTEP, 1, 0);
			//現在位置を設定  
			SendMessage(hProg, PBM_SETPOS, 0, 0);
		}

		//SetTimer(hDlgWnd, s_timerid, 100, NULL);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			//if (s_progresswnd) {
			//	if (s_timerid != 0) {
			//		KillTimer(s_progresswnd, s_timerid);
			//		s_timerid = 0;
			//	}
			//	DestroyWindow(s_progresswnd);
			//	s_progresswnd = 0;
			//}
			//InterlockedExchange(&g_progressflag, 2);
			////EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			//if (s_progresswnd) {
			//	if (s_timerid != 0) {
			//		KillTimer(s_progresswnd, s_timerid);
			//		s_timerid = 0;
			//	}
			//	DestroyWindow(s_progresswnd);
			//	s_progresswnd = 0;
			//}
			//InterlockedExchange(&g_progressflag, 2);
			////EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	case WM_USER_FOR_BATCH_PROGRESS:
	//case WM_TIMER:
	//	swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", (s_progresscnt + 1), s_progressnum);
	//	SetDlgItemTextW(s_progresswnd, IDC_STRBVH2FBXBATCH, strnumcnt);

	//	if (s_progresswnd) {
	//		//HWND hProg;
	//		hProg = GetDlgItem(s_progresswnd, IDC_PROGRESS1);
	//		if (hProg) {
	//			//プログレスバーの範囲を0-300にする           
	//			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, s_progressnum));
	//			//現在位置を設定  
	//			SendMessage(hProg, PBM_SETPOS, (s_progresscnt + 1), 0);
	//			//ステップの範囲を設定 
	//			//SendMessage(hProg, PBM_SETSTEP, 1, 0);
	//		}
	//		UpdateWindow(s_progresswnd);
	//	}
		break;
	case WM_CLOSE:
		if (s_progresswnd) {
			//if (s_timerid != 0) {
			//	KillTimer(s_progresswnd, s_timerid);
			//	s_timerid = 0;
			//}
			DestroyWindow(s_progresswnd);
			s_progresswnd = 0;
		}
		//InterlockedExchange(&g_progressflag, 2);
		//EndDialog(hDlgWnd, IDOK);
		break;
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
		break;
	}
	return TRUE;
}




LRESULT CALLBACK bvh2FbxBatchDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

	WCHAR strnumcnt[1024] = { 0L };
	HWND hProg;

	switch (msg) {
	case WM_INITDIALOG:
		s_bvh2fbxbatchwnd = hDlgWnd;

		swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", (s_bvh2fbxcnt + 1), s_bvh2fbxnum);
		SetDlgItemTextW(s_bvh2fbxbatchwnd, IDC_STRBVH2FBXBATCH, strnumcnt);
		hProg = GetDlgItem(s_bvh2fbxbatchwnd, IDC_PROGRESS1);
		if (hProg) {
			//プログレスバーの範囲         
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, s_bvh2fbxnum));
			//ステップの範囲を設定 
			SendMessage(hProg, PBM_SETSTEP, 1, 0);
			//現在位置を設定  
			SendMessage(hProg, PBM_SETPOS, 0, 0);
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			if (s_bvh2fbxbatchwnd) {
				DestroyWindow(s_bvh2fbxbatchwnd);
				s_bvh2fbxbatchwnd = 0;
			}
			InterlockedExchange(&g_bvh2fbxbatchflag, 2);
			//EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			if (s_bvh2fbxbatchwnd) {
				DestroyWindow(s_bvh2fbxbatchwnd);
				s_bvh2fbxbatchwnd = 0;
			}
			InterlockedExchange(&g_bvh2fbxbatchflag, 2);
			//EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
		if (s_bvh2fbxbatchwnd) {
			DestroyWindow(s_bvh2fbxbatchwnd);
			s_bvh2fbxbatchwnd = 0;
		}
		InterlockedExchange(&g_bvh2fbxbatchflag, 2);
		//EndDialog(hDlgWnd, IDOK);
		break;
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
		break;
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
	//ofn.hwndOwner = NULL;
	ofn.hwndOwner = s_3dwnd;
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
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"gco";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;


	static int s_savegcoproctimer = 356;

	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_Gconame );
			}

			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
			SetTimer(hDlgWnd, s_savegcoproctimer, 20, NULL);
			s_savegcodlghwnd = hDlgWnd;

            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					s_savegcodlghwnd = 0;
					KillTimer(hDlgWnd, s_savegcoproctimer);

					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_Gconame, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					s_savegcodlghwnd = 0;
					KillTimer(hDlgWnd, s_savegcoproctimer);
					
					EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
				{
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
					HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
						WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
					InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

					if (GetOpenFileNameW(&ofn) == IDOK) {
						SetDlgItemText(hDlgWnd, IDC_FILENAME, buf);
					}

					InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					UnhookWinEvent(hhook);
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
				}
					break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			return FALSE;
			break;
		default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
			return FALSE;
			break;
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
	//ofn.hwndOwner = NULL;
	ofn.hwndOwner = s_3dwnd;
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
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"imp";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	static int s_saveimpproctimer = 355;

	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_Impname );
			}
			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
			SetTimer(hDlgWnd, s_saveimpproctimer, 20, NULL);
			s_saveimpdlghwnd = hDlgWnd;

            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					s_saveimpdlghwnd = 0;
					KillTimer(hDlgWnd, s_saveimpproctimer);

					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_Impname, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					s_saveimpdlghwnd = 0;
					KillTimer(hDlgWnd, s_saveimpproctimer);
					
					EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
				{
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
					HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
						WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
					InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

					if (GetOpenFileNameW(&ofn) == IDOK) {
						SetDlgItemText(hDlgWnd, IDC_FILENAME, buf);
					}

					InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					UnhookWinEvent(hhook);
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
				}
					break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			return FALSE;
			break;
		default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
			return FALSE;
			break;
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
	//ofn.hwndOwner = NULL;
	ofn.hwndOwner = s_3dwnd;
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
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"ref";
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	static int s_savereproctimer = 354;

	switch (msg) {
        case WM_INITDIALOG:
			if( s_model && s_model->GetTopBone() ){
				SetDlgItemText( hDlgWnd, IDC_FILENAME, s_REname );
			}
			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
			SetTimer(hDlgWnd, s_savereproctimer, 20, NULL);
			s_saveredlghwnd = hDlgWnd;

            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					s_saveredlghwnd = 0;
					KillTimer(hDlgWnd, s_savereproctimer);

					GetDlgItemText( hDlgWnd, IDC_FILENAME, s_REname, MAX_PATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					s_saveredlghwnd = 0;
					KillTimer(hDlgWnd, s_savereproctimer);
					
					EndDialog(hDlgWnd, IDCANCEL);
                    break;
					break;
				case IDC_REFFILE:
				{
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
					HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
						WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
					InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

					if (GetOpenFileNameW(&ofn) == IDOK) {
						SetDlgItemText(hDlgWnd, IDC_FILENAME, buf);
					}

					InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					UnhookWinEvent(hhook);
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
				}

					break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			return FALSE;
			break;
		default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
			return FALSE;
			break;
    }
    return TRUE;

}


LRESULT CALLBACK ExportXDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	WCHAR buf[MAX_PATH];
	OPENFILENAME ofn;
	buf[0] = 0L;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = NULL;
	ofn.hwndOwner = s_3dwnd;
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
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt =NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	WCHAR strmult[256];
	wcscpy_s( strmult, 256, L"1.000" );
	
	static int s_exportxproctimer = 360;

	switch (msg) {
        case WM_INITDIALOG:
			g_tmpmqomult = 1.0f;
			ZeroMemory( g_tmpmqopath, sizeof( WCHAR ) * MULTIPATH );
			SetDlgItemText( hDlgWnd, IDC_MULT, strmult );
			SetDlgItemText( hDlgWnd, IDC_FILEPATH, L"PushRefButtonToSelectFile." );

			SetTimer(hDlgWnd, s_exportxproctimer, 20, NULL);
			s_exportxdlghwnd = hDlgWnd;
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					s_exportxdlghwnd = 0;
					KillTimer(hDlgWnd, s_exportxproctimer);
					GetDlgItemText( hDlgWnd, IDC_MULT, strmult, 256 );
					g_tmpmqomult = (float)_wtof( strmult );
					GetDlgItemText( hDlgWnd, IDC_FILEPATH, g_tmpmqopath, MULTIPATH );
                    EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					s_exportxdlghwnd = 0;
					KillTimer(hDlgWnd, s_exportxproctimer);
					EndDialog(hDlgWnd, IDCANCEL);
                    break;
				case IDC_REFX:
				{
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
					HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
						WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
					InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

					if (GetOpenFileNameW(&ofn) == IDOK) {
						SetDlgItemText(hDlgWnd, IDC_FILEPATH, buf);
					}

					InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					UnhookWinEvent(hhook);
					s_getfilenamehwnd = 0;
					s_getfilenametreeview = 0;
				}
					break;
				default:
                    return FALSE;
            }
		case WM_TIMER:
			OnDSUpdate();
			return FALSE;
			break;
		default:
			DefWindowProc(hDlgWnd, msg, wp, lp);
			return FALSE;

    }
    return TRUE;
}



void ActivatePanel( int state )
{
	if (!s_timelineWnd || !s_toolWnd || !s_layerWnd || !s_modelpanel.panel || !s_motionpanel.panel)
		return;


	if( state == 1 ){
		if( s_dispmw && s_timelineWnd){
			s_timelineWnd->setVisible( false );
			s_timelineWnd->setVisible( true );
		}
		if( s_disptool && s_toolWnd){
			s_toolWnd->setVisible( true );
		}
		if( s_dispobj && s_layerWnd){
			s_layerWnd->setVisible( true );
		}
		
	}else if( state == 0 ){
		if (s_timelineWnd) {
			s_timelineWnd->setVisible(false);
		}
		if (s_toolWnd) {
			s_toolWnd->setVisible(false);
		}

		if (s_layerWnd) {
			s_layerWnd->setVisible(false);
		}
		s_dispobj = false;
	}

	DispModelPanel();
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
	if (s_modelpanel.separator2) {
		delete s_modelpanel.separator2;
		s_modelpanel.separator2 = 0;
	}
	if( !(s_modelpanel.checkvec.empty()) ){
		int checknum = (int)s_modelpanel.checkvec.size();
		int checkno;
		for( checkno = 0; checkno < checknum; checkno++ ){
			delete s_modelpanel.checkvec[checkno];
		}
	}
	s_modelpanel.checkvec.clear();
	if (!(s_modelpanel.delbutton.empty())) {
		int buttonnum = (int)s_modelpanel.delbutton.size();
		int buttonno;
		for (buttonno = 0; buttonno < buttonnum; buttonno++) {
			delete s_modelpanel.delbutton[buttonno];
		}
	}
	s_modelpanel.delbutton.clear();

	s_modelpanel.modelindex = -1;

	return 0;
}

int CreateModelPanel()
{

	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		return 0;
	}

	RECT panelrect;
	if (s_modelpanel.panel && IsWindow(s_modelpanel.panel->getHWnd())) {
		GetWindowRect(s_modelpanel.panel->getHWnd(), &panelrect);
		s_modelpanelpos = WindowPos(panelrect.left, panelrect.top);
	}
	else {
		s_modelpanelpos = WindowPos(200, s_2ndposy);
	}


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
		WindowPos(200, s_2ndposy),		//位置
		WindowSize(400,460),	//サイズ
		L"ModelPanel",	//タイトル
		//s_mainhwnd,					//親ウィンドウハンドル
		//false,
		NULL,//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 他所をクリックしても隠れないように
		true,					//表示・非表示状態
		//70,50,70,				//カラー
		0, 0, 0,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否
	if (!s_modelpanel.panel) {
		_ASSERT(0);
		return 1;
	}


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

	//s_modelpanel.separator =  new OWP_Separator(s_modelpanel.panel, false);									// セパレータ1（境界線による横方向2分割）
	s_modelpanel.separator =  new OWP_Separator(s_modelpanel.panel, true);									// セパレータ1（境界線による横方向2分割）
	if (!s_modelpanel.separator) {
		_ASSERT(0);
		return 1;
	}
	s_modelpanel.separator->setSize(WindowSize(400, 30));
	//s_modelpanel.separator2 = new OWP_Separator(s_modelpanel.panel, false);									// セパレータ2（境界線による横方向2分割）
	s_modelpanel.separator2 = new OWP_Separator(s_modelpanel.panel, true);									// セパレータ2（境界線による横方向2分割）
	if (!s_modelpanel.separator2) {
		_ASSERT(0);
		return 1;
	}
	s_modelpanel.separator2->setSize(WindowSize(200, 30));


	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		CModel* curmodel = s_modelindex[modelcnt].modelptr;
		OWP_CheckBoxA *owpCheckBox= new OWP_CheckBoxA( L"Show/Hide", curmodel->GetModelDisp() );
		s_modelpanel.checkvec.push_back( owpCheckBox );
		OWP_Button* owpButton = new OWP_Button(L"delete");
		s_modelpanel.delbutton.push_back(owpButton);
	}

	s_modelpanel.panel->addParts( *(s_modelpanel.separator) );

	s_modelpanel.separator->addParts1( *(s_modelpanel.radiobutton) );
	s_modelpanel.separator->addParts2(*(s_modelpanel.separator2));

	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		OWP_CheckBoxA* curcb = s_modelpanel.checkvec[modelcnt];
		s_modelpanel.separator2->addParts1( *curcb );

		OWP_Button* curbutton = s_modelpanel.delbutton[modelcnt];
		s_modelpanel.separator2->addParts2(*curbutton);
	}

	s_modelpanel.modelindex = s_curmodelmenuindex;
	s_modelpanel.radiobutton->setSelectIndex( s_modelpanel.modelindex );

	s_modelpanel.panel->setVisible( 0 );//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

////////////
	s_modelpanel.panel->setCloseListener( [](){ 
		if (s_model) {
			s_closemodelFlag = true;
		}
	});

	for( modelcnt = 0; modelcnt < modelnum; modelcnt++ ){
		s_modelpanel.checkvec[modelcnt]->setButtonListener( [modelcnt](){
			if (s_model) {
				if (modelcnt < s_modelindex.size()) {
					CModel* curmodel = s_modelindex[modelcnt].modelptr;
					if (curmodel) {
						//s_modelpanel.checkvec[modelcnt]->setValue(!(s_modelpanel.checkvec[modelcnt]->getValue()));
						curmodel->SetModelDisp(s_modelpanel.checkvec[modelcnt]->getValue());
						s_modelpanel.panel->callRewrite();
					}
				}
			}
		} );

		s_modelpanel.delbutton[modelcnt]->setButtonListener([modelcnt]() {
			if (s_model) {
				if ((modelcnt < s_modelindex.size()) && (s_modelindex.size() >= 2)) {//全部消すときはメインメニューから
					CModel* curmodel = s_modelindex[modelcnt].modelptr;
					if ((s_opedelmodelcnt < 0) && curmodel && !s_underdelmodel) {
						s_underdelmodel = true;
						//bool ondelbutton = true;
						//OnDelModel(modelcnt, ondelbutton);//s_modelpanel.modelindexはs_modelのindexなので違う

						//ここでOnDelModelを呼ぶとOrgWindowの関数を実行中にparentWindowがNULLになるなどしてエラーになる.フラグを立ててループで呼ぶ
						s_opedelmodelcnt = modelcnt;
					}
				}
			}
		});
	}

	s_modelpanel.radiobutton->setSelectListener( [](){
		if (s_model) {
			int curindex = s_modelpanel.radiobutton->getSelectIndex();
			if ((s_opeselectmodelcnt < 0) && !s_underselectmodel && (curindex >= 0) && (curindex < s_modelindex.size())) {
				//s_modelpanel.modelindex = curindex;
				//OnModelMenu(true, s_modelpanel.modelindex, 1);
				//s_modelpanel.panel->callRewrite();

				//ここでOnModelMenuを呼ぶとOrgWindowの関数を実行中にparentWindowがNULLになるなどしてエラーになる.フラグを立ててループで呼ぶ
				s_underselectmodel = true;
				s_opeselectmodelcnt = curindex;
			}
		}
	} );


	if (s_firstmodelpanelpos) {
		RECT wnd3drect;
		if (s_mainhwnd) {
			GetWindowRect(s_mainhwnd, &wnd3drect);
			s_modelpanelpos = WindowPos(wnd3drect.left + 100, wnd3drect.top + 500);
		}
		else {
			s_modelpanelpos = WindowPos(200, s_2ndposy);
		}
		s_firstmodelpanelpos = false;
	}
	s_modelpanel.panel->setPos(s_modelpanelpos);


	//s_modelpanel.panel->setSize(WindowSize(200, 100));//880
	s_modelpanel.panel->setSize(WindowSize(400, 460));//880
	s_modelpanel.panel->setVisible(false);
	s_dispmodel = false;//!!!!!!!!!!!!!!!!! modelpanelのdispflag


	return 0;
}



int DestroyMotionPanel()
{
	if (s_motionpanel.panel) {
		s_motionpanel.panel->setVisible(false);
		delete s_motionpanel.panel;
		s_motionpanel.panel = 0;
	}
	if (s_motionpanel.radiobutton) {
		delete s_motionpanel.radiobutton;
		s_motionpanel.radiobutton = 0;
	}
	if (s_motionpanel.separator) {
		delete s_motionpanel.separator;
		s_motionpanel.separator = 0;
	}
	if (!(s_motionpanel.delbutton.empty())) {
		int buttonnum = (int)s_motionpanel.delbutton.size();
		int buttonno;
		for (buttonno = 0; buttonno < buttonnum; buttonno++) {
			delete s_motionpanel.delbutton[buttonno];
		}
	}
	s_motionpanel.delbutton.clear();

	s_motionpanel.modelindex = -1;

	return 0;
}

int CreateMotionPanel()
{
	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		return 0;
	}


	RECT panelrect;
	if (s_motionpanel.panel && IsWindow(s_motionpanel.panel->getHWnd())) {
		GetWindowRect(s_motionpanel.panel->getHWnd(), &panelrect);
		s_motionpanelpos = WindowPos(panelrect.left, panelrect.top);
	}
	else {
		s_motionpanelpos = WindowPos(200, s_2ndposy);
	}



	DestroyMotionPanel();

	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0) {
		return 0;
	}
	if (!s_model) {
		return 0;
	}
	int motionnum = s_model->GetMotInfoSize();
	if (motionnum <= 0) {
		return 0;
	}

	int classcnt = 0;
	WCHAR clsname[256];
	swprintf_s(clsname, 256, L"MotionPanel%d", classcnt);

	s_motionpanel.panel = new OrgWindow(
		1,
		clsname,		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(600, s_2ndposy),		//位置
		WindowSize(400, 700),	//サイズ
		L"MotionPanel",	//タイトル
		//s_mainhwnd,					//親ウィンドウハンドル
		//false,
		NULL,//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 他所をクリックしても隠れないように
		true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否
	if (!s_motionpanel.panel) {
		_ASSERT(0);
		return 1;
	}


	s_motionpanel.panel->setSizeMin(WindowSize(150, 150));		// 最小サイズを設定

	s_motionpanel.separator = new OWP_Separator(s_motionpanel.panel, false);									// セパレータ1（境界線による横方向2分割）
	if (!s_motionpanel.separator) {
		_ASSERT(0);
		return 1;
	}
	s_motionpanel.separator->setSize(WindowSize(400, 30));

	s_motionpanel.panel->addParts(*(s_motionpanel.separator));


	int motioncnt = 0;
	std::map<int, MOTINFO*>::iterator itrmi;
	for (itrmi = s_model->GetMotInfoBegin(); itrmi != s_model->GetMotInfoEnd(); itrmi++) {
		MOTINFO* curmi = (MOTINFO*)(itrmi->second);
		if (curmi) {
			WCHAR wmotname[MAX_PATH] = { 0L };
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, curmi->motname, 256, wmotname, MAX_PATH);
			if (motioncnt == 0) {
				s_motionpanel.radiobutton = new OWP_RadioButton(wmotname);
			}
			else {
				s_motionpanel.radiobutton->addLine(wmotname);
			}

			motioncnt++;
		}
	}

	s_motionpanel.separator->addParts1(*(s_motionpanel.radiobutton));//add once

	std::map<int, MOTINFO*>::iterator itrmi2;
	for (itrmi2 = s_model->GetMotInfoBegin(); itrmi2 != s_model->GetMotInfoEnd(); itrmi2++) {
		MOTINFO* curmi = (MOTINFO*)(itrmi2->second);
		if (curmi) {
			OWP_Button* owpButton = new OWP_Button(L"delete");
			s_motionpanel.delbutton.push_back(owpButton);
			s_motionpanel.separator->addParts2(*owpButton);
		}
	}


	s_motionpanel.modelindex = s_curmodelmenuindex;
	//s_motionpanel.radiobutton->setSelectIndex(0);
	if (s_model) {
		s_motionpanel.radiobutton->setSelectIndex(s_motmenuindexmap[s_model]);//!!!!
	}
	else {
		_ASSERT(0);
		return 1;
	}

	s_motionpanel.panel->setVisible(0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

////////////
	s_motionpanel.panel->setCloseListener([]() {
		if (s_model) {
			s_closemotionFlag = true;
		}
		});

	int delmenuindex = 0;
	for (itrmi = s_model->GetMotInfoBegin(); itrmi != s_model->GetMotInfoEnd(); itrmi++) {
		MOTINFO* curmi = (MOTINFO*)(itrmi->second);
		if (curmi) {
			s_motionpanel.delbutton[delmenuindex]->setButtonListener([delmenuindex]() {
				if ((s_opedelmotioncnt < 0) && !s_underdelmotion && s_model && (s_model->GetMotInfoSize() >= 2)) {//全部消すときはメインメニューから
					s_underdelmotion = true;
					//bool ondelbutton = true;
					//OnDelMotion(delmenuindex, ondelbutton);

					//ここでOnDelMotionを呼ぶとOrgWindowの関数を実行中にparentWindowがNULLになるなどしてエラーになる.フラグを立ててループで呼ぶ
					s_opedelmotioncnt = delmenuindex;
				}
			});
			delmenuindex++;
		}
		else {
			_ASSERT(0);
		}
	}

	s_motionpanel.radiobutton->setSelectListener([]() {
		if (s_model) {
			int curindex = s_motionpanel.radiobutton->getSelectIndex();
			if ((s_opeselectmotioncnt < 0) && !s_underselectmotion && (curindex >= 0) && (curindex < s_model->GetMotInfoSize())) {
				s_underselectmotion = true;
				//int motionindex = curindex;
				//OnAnimMenu(true, motionindex, 1);
				//s_motionpanel.panel->callRewrite();

				//ここでOnAnimMenuを呼ぶとOrgWindowの関数を実行中にparentWindowがNULLになるなどしてエラーになる.フラグを立ててループで呼ぶ
				s_opeselectmotioncnt = curindex;
			}
		}
	});


	if (s_firstmotionpanelpos) {
		RECT wnd3drect;
		if (s_mainhwnd) {
			GetWindowRect(s_mainhwnd, &wnd3drect);
			s_motionpanelpos = WindowPos(wnd3drect.left + 500, wnd3drect.top + 500);
		}
		else {
			s_motionpanelpos = WindowPos(600, s_2ndposy);
		}
		s_firstmotionpanelpos = false;
	}
	s_motionpanel.panel->setPos(s_motionpanelpos);

	//s_motionpanel.panel->setSize(WindowSize(200, 100));//880
	s_motionpanel.panel->setSize(WindowSize(400, 700));//880
	s_motionpanel.panel->setVisible(false);
	s_dispmotion = false;//!!!!!!!!!!!!!!!!! motionpanelのdispflag


	return 0;
}


int DestroyConvBoneWnd()
{
	s_convbone_model = 0;
	s_convbone_bvh = 0;
	s_convbonemap.clear();

	if (s_convboneWnd){
		s_convboneWnd->setListenMouse(false);
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
	if (s_rtgfilesave) {
		delete s_rtgfilesave;
		s_rtgfilesave = 0;
	}
	if (s_rtgfileload) {
		delete s_rtgfileload;
		s_rtgfileload = 0;
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

	s_dsretargetctrls.clear();

	s_convbonenum = s_model->GetBoneListSize();
	if (s_convbonenum >= CONVBONEMAX){
		_ASSERT(0);
		return 1;
	}

	s_convboneWnd = new OrgWindow(
		0,
		L"convbone0",		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight),		//位置
		WindowSize(s_sidewidth, s_sideheight),	//サイズ
		L"ConvBoneWnd",	//タイトル
		s_mainhwnd,					//親ウィンドウハンドル
		false,					//表示・非表示状態
		//true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
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

			swprintf_s(bvhbonename, MAX_PATH, L"NotSet_%03d", cbno);
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


	s_convbonesp = new OWP_Separator(s_convboneWnd, false, 0.5, true);									// セパレータ1（境界線による横方向2分割）

	s_cbselmodel = new OWP_Button(L"SelectShapeModel");
	s_cbselbvh = new OWP_Button(L"SelectMotionModel");
	s_convboneconvert = new OWP_Button(L"ConvertButton");
	s_rtgfilesave = new OWP_Button(L"Save RtgFile");
	s_rtgfileload = new OWP_Button(L"Load RtgFile");
	s_convbonemidashi[0] = new OWP_Label(L"ShapeSide");
	s_convbonemidashi[1] = new OWP_Label(L"MotionSide");


	COLORREF importantcol = RGB(168, 129, 129);
	s_convboneconvert->setTextColor(importantcol);
	s_rtgfilesave->setTextColor(importantcol);
	s_rtgfileload->setTextColor(importantcol);
	


	s_convboneSCWnd->addParts(*s_convbonesp);


	s_convbonesp->addParts1(*s_convbonemidashi[0]);
	s_convbonesp->addParts1(*s_cbselmodel);
	s_convbonesp->addParts2(*s_convbonemidashi[1]);
	s_convbonesp->addParts2(*s_cbselbvh);


	s_dsretargetctrls.push_back(s_cbselmodel);
	s_dsretargetctrls.push_back(s_cbselbvh);


	for (cbno = 0; cbno < s_convbonenum; cbno++) {
		s_convbonesp->addParts1(*s_modelbone[cbno]);
		s_convbonesp->addParts2(*s_bvhbone[cbno]);

		//s_dsretargetctrls.push_back(s_modelbone[cbno]);
		s_dsretargetctrls.push_back(s_bvhbone[cbno]);
	}

	s_convbonesp->addParts1(*s_convboneconvert);
	s_dsretargetctrls.push_back(s_convboneconvert);

	s_convbonesp->addParts1(*s_rtgfilesave);
	s_dsretargetctrls.push_back(s_rtgfilesave);

	s_convbonesp->addParts2(*s_rtgfileload);
	s_dsretargetctrls.push_back(s_rtgfileload);

	s_convboneWnd->setListenMouse(false);
	s_convboneWnd->setVisible(0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	////////////
	s_convboneWnd->setCloseListener([](){ 
		if (s_model) {
			s_closeconvboneFlag = true;
		}
	});


	s_cbselmodel->setButtonListener([](){
		if (s_model) {
			SetConvBoneModel();
			s_convboneWnd->callRewrite();
		}
	});
	s_cbselbvh->setButtonListener([](){
		if (s_model) {
			if (!s_convbone_model || (s_convbone_model != s_model)) {
				::DSMessageBox(s_mainhwnd, L"Retry after selecting ShapeModel using ModelMenu Of MainWindow.", L"error!!!", MB_OK);
			}
			else {
				SetConvBoneBvh();
			}
			s_convboneWnd->callRewrite();
		}
	});
	for (cbno = 0; cbno < s_convbonenum; cbno++){
		s_bvhbone[cbno]->setButtonListener([cbno](){
			if (s_model) {
				SetConvBone(cbno);
				//CModel* curmodel = s_modelindex[modelcnt].modelptr;
				//curmodel->SetModelDisp(s_modelpanel.checkvec[modelcnt]->getValue());
				s_convboneWnd->callRewrite();
			}
		});
	}
	s_convboneconvert->setButtonListener([](){
		if (s_model) {
			RetargetMotion();
		}
	});

	s_rtgfilesave->setButtonListener([]() {
		if (s_model) {
			SaveRetargetFile();
		}
	});

	s_rtgfileload->setButtonListener([]() {
		if (s_model) {
			LoadRetargetFile(0);
		}
	});

	s_convboneWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_convboneWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));

	s_convboneWnd->setVisible(false);

	return 0;
}

int SetConvBoneModel()
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	HWND parwnd;
	//parwnd = s_convboneWnd->getHWnd();
	parwnd = s_3dwnd;

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd, MENUOFFSET_SETCONVBONEMODEL);
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
				int setmenuid = ID_RMENU_0 + modelno + MENUOFFSET_SETCONVBONEMODEL;
				AppendMenu(submenu, MF_STRING, setmenuid, modelname);
			}
		}
	}


	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	s_cursubmenu = rmenu->GetSubMenu();

	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	//if ((menuid >= ID_RMENU_0) && (menuid < (ID_RMENU_0 + modelnum))) {
	//	int modelindex = menuid - ID_RMENU_0;
	//	s_convbone_model = s_modelindex[modelindex].modelptr;

	//	WCHAR strmes[1024];
	//	if (!s_convbone_model) {
	//		swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
	//		::DSMessageBox(NULL, strmes, L"check", MB_OK);
	//	}
	//	else {
	//		swprintf_s(strmes, 1024, L"%s", s_convbone_model->GetFileName());
	//		s_cbselmodel->setName(strmes);
	//	}
	//}


	rmenu->Destroy();
	delete rmenu;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	return 0;
}
int SetConvBoneBvh()
{
	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	HWND parwnd;
	//parwnd = s_convboneWnd->getHWnd();
	parwnd = s_3dwnd;

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd, MENUOFFSET_SETCONVBONEBVH);
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
				int setmenuid = ID_RMENU_0 + modelno + MENUOFFSET_SETCONVBONEBVH;
				AppendMenu(submenu, MF_STRING, setmenuid, modelname);
			}
		}
	}


	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	s_cursubmenu = rmenu->GetSubMenu();

	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	//if ((menuid >= ID_RMENU_0) && (menuid < (ID_RMENU_0 + modelnum))){
	//	int modelindex = menuid - ID_RMENU_0;
	//	s_convbone_bvh = s_modelindex[modelindex].modelptr;

	//	WCHAR strmes[1024];
	//	if (!s_convbone_bvh){
	//		swprintf_s(strmes, 1024, L"convbone : sel model : modelptr NULL !!!");
	//		::DSMessageBox(NULL, strmes, L"check", MB_OK);
	//	}
	//	else{
	//		swprintf_s(strmes, 1024, L"%s", s_convbone_bvh->GetFileName());
	//		s_cbselbvh->setName(strmes);
	//	}
	//}


	rmenu->Destroy();
	delete rmenu;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	return 0;
}
int SetConvBone( int cbno )
{
	s_bvhbone_cbno = cbno;

	int modelnum = (int)s_modelindex.size();
	if (modelnum <= 0){
		return 0;
	}

	if (!s_convbone_model || !s_convbone_bvh){
		return 0;
	}

	HWND parwnd;
	//parwnd = s_convboneWnd->getHWnd();
	parwnd = s_3dwnd;
	//parwnd = NULL;


	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd, MENUOFFSET_SETCONVBONE);
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
	AppendMenu(submenu, MF_STRING, setmenuid0, L"NotSet");

	int maxboneno = 0;
	map<int, CBone*>::iterator itrbone;
	for (itrbone = s_convbone_bvh->GetBoneListBegin(); itrbone != s_convbone_bvh->GetBoneListEnd(); itrbone++){
		CBone* curbone = itrbone->second;
		if (curbone){
			int boneno = curbone->GetBoneNo();
			int setmenuid = ID_RMENU_0 + boneno + 1 + MENUOFFSET_SETCONVBONE;
			AppendMenu(submenu, MF_STRING, setmenuid, curbone->GetWBoneName());
			if (boneno > maxboneno){
				maxboneno = boneno;
			}
		}
	}
	s_maxboneno = s_convbone_bvh->GetBoneListSize();

	POINT pt;
	GetCursorPos(&pt);
	//::ScreenToClient(parwnd, &pt);

	s_cursubmenu = rmenu->GetSubMenu();

	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	//if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + maxboneno + 1))){
	//	if (menuid == (ID_RMENU_0 + 0)){
	//		//未設定
	//		s_bvhbone_bone[cbno] = 0;
	//		CBone* modelbone = s_modelbone_bone[cbno];
	//		_ASSERT(modelbone);
	//		if (modelbone){
	//			s_convbonemap[modelbone] = 0;
	//		}
	//		s_bvhbone[cbno]->setName(L"NotSet");
	//	}
	//	else{
	//		int boneno = menuid - ID_RMENU_0 - 1;
	//		CBone* curbone = s_convbone_bvh->GetBoneByID(boneno);
	//		WCHAR strmes[1024];
	//		if (!curbone){
	//			s_bvhbone_bone[cbno] = 0;
	//			CBone* modelbone = s_modelbone_bone[cbno];
	//			_ASSERT(modelbone);
	//			if (modelbone){
	//				s_convbonemap[modelbone] = 0;
	//			}
	//			s_bvhbone[cbno]->setName(L"NotSet");

	//			swprintf_s(strmes, 1024, L"convbone : sel bvh bone : curbone NULL !!!");
	//			::DSMessageBox(NULL, strmes, L"check", MB_OK);
	//		}
	//		else{
	//			swprintf_s(strmes, 1024, L"%s", curbone->GetWBoneName());
	//			s_bvhbone[cbno]->setName(strmes);
	//			s_bvhbone_bone[cbno] = curbone;

	//			CBone* modelbone = s_modelbone_bone[cbno];
	//			if (modelbone){
	//				s_convbonemap[modelbone] = curbone;
	//			}
	//		}
	//	}
	//}

	rmenu->Destroy();
	delete rmenu;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	return 0;
}

int SetJointPair2ConvBoneWnd()
{
	if (s_convbonemap.empty()) {
		return 0;
	}
	if (!s_model) {
		return 0;
	}
	if (!s_convbone_bvh) {
		return 0;
	}

	WCHAR bvhbonename[MAX_PATH];
	int cbno = 0;
	map<int, CBone*>::iterator itrbone;
	for (itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++) {
		CBone* curbone = itrbone->second;
		if (curbone) {
			CBone* bvhbone = s_convbonemap[curbone];
			if (bvhbone) {
				swprintf_s(bvhbonename, MAX_PATH, bvhbone->GetWBoneName());
				(s_bvhbone[cbno])->setName(bvhbonename);
				s_bvhbone_bone[cbno] = bvhbone;
			}
			else {
				swprintf_s(bvhbonename, MAX_PATH, L"NotSet_%03d", cbno);
				(s_bvhbone[cbno])->setName(bvhbonename);
				s_bvhbone_bone[cbno] = 0;
			}

			cbno++;
		}
	}

	_ASSERT(cbno == s_convbonenum);

	return 0;
}


int SaveMotionNameListFile()
{

	ChangeCurDirFromMameMediaToTest();

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = s_3dwnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"Retarget(*.mnl)\0*.mnl\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;
	HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
		WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);


	WCHAR* pext;
	pext = wcsstr(g_tmpmqopath, L".mnl");
	if (!pext) {
		ZeroMemory(g_tmpmqopath, sizeof(WCHAR) * MAX_PATH);//異なる拡張子のファイル名が残っている場合があるから
	}

	if (GetOpenFileNameW(&ofn) == IDOK) {
		CMNLFile mnlfile;
		int result;
		result = mnlfile.WriteMNLFile(g_tmpmqopath, s_model);
	}

	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	UnhookWinEvent(hhook);
	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;

	return 0;
}



int SaveRetargetFile()
{

	ChangeCurDirFromMameMediaToTest();

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = s_3dwnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"Retarget(*.rtg)\0*.rtg\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;
	HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
		WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);


	WCHAR* pext;
	pext = wcsstr(g_tmpmqopath, L".rtg");
	if (!pext) {
		ZeroMemory(g_tmpmqopath, sizeof(WCHAR) * MAX_PATH);//異なる拡張子のファイル名が残っている場合があるから
	}

	if (GetOpenFileNameW(&ofn) == IDOK) {
		CRetargetFile rtgfile;
		int result;
		result = rtgfile.WriteRetargetFile(g_tmpmqopath, s_convbone_model, s_convbone_bvh, s_convbonemap);
	}

	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	UnhookWinEvent(hhook);
	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;

	return 0;
}
int LoadRetargetFile(WCHAR* srcfilename)
{
	if (!s_convbone_model || !s_convbone_bvh) {
		_ASSERT(0);
		return 1;
	}

	ChangeCurDirFromMameMediaToTest();

	s_convbonemap.clear();


	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = hDlgWnd;
	ofn.hwndOwner = s_3dwnd;
	ofn.hInstance = 0;
	ofn.lpstrFilter = L"Retarget(*.rtg)\0*.rtg\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = g_tmpmqopath;
	ofn.nMaxFile = MULTIPATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"GetFileNameDlg";
	//ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	int result = 0;

	if (srcfilename == 0) {
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
		HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
			WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
		InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

		if (GetOpenFileNameW(&ofn) == IDOK) {
			CRetargetFile rtgfile;
			result = rtgfile.LoadRetargetFile(g_tmpmqopath, s_convbone_model, s_convbone_bvh, s_convbonemap);
			if (result == 0) {
				//if (g_retargetbatchflag == 0) {
				if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {
					SetJointPair2ConvBoneWnd();
				}
			}
		}

	}
	else {
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
		HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
			WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
		InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

		CRetargetFile rtgfile;
		result = rtgfile.LoadRetargetFile(srcfilename, s_convbone_model, s_convbone_bvh, s_convbonemap);
		if (result == 0) {
			//if (g_retargetbatchflag == 0) {
			if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {
				SetJointPair2ConvBoneWnd();
			}
		}

		InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
		UnhookWinEvent(hhook);
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
	}
	return result;
}


int RetargetMotion()
{
	//static CModel* s_convbone_model = 0;
	//static CModel* s_convbone_model_batch = 0;
	//static CModel* s_convbone_bvh = 0;

	if (!s_convbone_model || !s_convbone_bvh) {
		return 0;
	}

	if (s_model != s_convbone_model) {
		::DSMessageBox(NULL, L"Retry After Selectiong ShapeModel using ModelMenu Of MainWindow.", L"error!!!", MB_OK);
		return 1;
	}

	int result = MameBake3DLibRetarget::Retarget(s_convbone_model, s_convbone_bvh, s_matVP, s_convbonemap, AddMotion, InitCurMotion);//Retarget.h, Retarget.cpp
	if (result) {
		_ASSERT(0);
		return 1;
	}

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


int SetCamera6Angle()
{

	ChaVector3 weye, wdiff;
	weye = g_camEye;
	wdiff = g_camtargetpos - weye;
	float camdist = (float)ChaVector3LengthDbl( &wdiff );

	ChaVector3 neweye;
	float delta = 0.10f;

	if( g_keybuf[ VK_F1 ] & 0x80 ){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z - camdist;

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

	}else if( g_keybuf[ VK_F2 ] & 0x80 ){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z + camdist;

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F3] & 0x80){
		neweye.x = g_camtargetpos.x - camdist;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z;

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F4] & 0x80){
		neweye.x = g_camtargetpos.x + camdist;
		neweye.y = g_camtargetpos.y;
		neweye.z = g_camtargetpos.z;
		
		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F5] & 0x80){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y + camdist;
		neweye.z = g_camtargetpos.z + delta;

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}
	else if (g_keybuf[VK_F6] & 0x80){
		neweye.x = g_camtargetpos.x;
		neweye.y = g_camtargetpos.y - camdist;
		neweye.z = g_camtargetpos.z - delta;

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_camEye = neweye;
		//!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
	}

	ChaVector3 diffv;
	diffv = g_camEye - g_camtargetpos;
	s_camdist = (float)ChaVector3LengthDbl( &diffv );

	return 0;
}

int OnAddMotion( int srcmotid, bool dorefreshtl)
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;

	//MOTINFO* newmotinfo = s_model->GetMotInfo( srcmotid );

//WCHAR strchk[256] = { 0L };
//swprintf_s(strchk, 256, L"check OnAddMotion : 1 : %d, %d", srcmotid, (int)dorefreshtl);
//::MessageBox(s_mainhwnd, strchk, L"check!!!", MB_OK);

	CallF( AddTimeLine( srcmotid, dorefreshtl ), return 1 );

//swprintf_s(strchk, 256, L"check OnAddMotion : 2 : %d, %d", srcmotid, (int)dorefreshtl);
//::MessageBox(s_mainhwnd, strchk, L"check!!!", MB_OK);

	int selindex = (int)s_tlarray.size() - 1;
	CallF( OnAnimMenu( dorefreshtl, selindex ), return 1 );

//swprintf_s(strchk, 256, L"check OnAddMotion : 3 : %d, %d", srcmotid, (int)dorefreshtl);
//::MessageBox(s_mainhwnd, strchk, L"check!!!", MB_OK);


	return 0;

}

int StopBt()
{
	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel) {
			curmodel->BulletSimulationStop();
			SetKinematicToHand(curmodel, false);
		}
	}


	g_previewFlag = 0;
	g_limitdegflag = s_savelimitdegflag;
	if (s_LimitDegCheckBox) {
		s_LimitDegCheckBox->SetChecked(g_limitdegflag);
	}

	return 0;
}


int StartBt(CModel* curmodel, BOOL isfirstmodel, int flag, int btcntzero)
{
	if (!s_model || !curmodel){
		return 0;
	}




	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//previewmodeがglobalなのでmodelごとにモードを設定するようにはなっていない。
	//ひとまず、全モデルへの適用という形をとってエラーにならないようにする。
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	double curframe = 1.0;//初期値、start時の揺れに影響？

	if (flag == 1) {
		//まず物理IKを停止する。
		//プレビューを止めないとtimelineはスタートフレームになるが姿勢がスタートフレームにならない。
		//flag == 0で呼ぶとシミュが動かない。

		vector<MODELELEM>::iterator itrmodel;
		for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
			CModel* pmodel = itrmodel->modelptr;
			if (pmodel) {
				//全モデルシミュ停止
				pmodel->BulletSimulationStop();
			}
		}

		g_previewFlag = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!

		s_previewrange = s_editrange;
		//double rangestart;
		if (s_previewrange.IsSameStartAndEnd()) {
			//rangestart = 1.0;
			curframe = 1.0;
		}
		else {
			if (g_previewFlag == 5) {
				int tmpleng;
				double tmpstart, tmpend;
				s_previewrange.GetRange(&tmpleng, &tmpstart, &tmpend);
				curframe = s_previewrange.GetApplyFrame();
			}
			else {
				//rangestart = s_previewrange.GetStartFrame();
				curframe = s_previewrange.GetStartFrame();
			}
		}

		s_owpLTimeline->setCurrentTime(curframe, true);
		s_owpEulerGraph->setCurrentTime(curframe, false);

		OnFramePreviewStop();
	}

	static int resetflag = 0;//!!!!!!!!!!!!!!!!!!!! static 
	static int createflag = 0;//!!!!!!!!!!!!!!!!!!!! static 

	if (isfirstmodel == TRUE) {
		if ((flag == 0) && (g_previewFlag != 4)) {
			//F9キー
			g_previewFlag = 4;
			createflag = 1;
			resetflag = 0;//2021/06/18
		}
		else if (flag == 1) {
			//F10キー
			g_previewFlag = 5;
			createflag = 1;
			resetflag = 0;//2021/06/18
		}
		else if (flag == 2) {
			//spaceキー
			if (g_previewFlag == 4) {
				createflag = 1;//2021/06/18
				resetflag = 1;
			}
			else if (g_previewFlag == 5) {
				createflag = 1;//2021/06/18
				resetflag = 0;
			}
		}
		else {
			g_previewFlag = 0;
		}
	}


	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		CModel* pmodel = itrmodel->modelptr;


		if (pmodel) {
			//if ((flag == 0) && (g_previewFlag != 4)){
				//F9キー
			if (btcntzero == 1) {
				pmodel->ZeroBtCnt();
				pmodel->SetCreateBtFlag(false);
			}
			//}
			//else if (flag == 1){
			//	//F10キー
			//	if (btcntzero == 1){
			//		curmodel->ZeroBtCnt();
			//		curmodel->SetCreateBtFlag(false);
			//	}
			//}


			//double curframe;
			if (flag == 0) {// bt simu
				if (resetflag == 1) {
					curframe = s_owpTimeline->getCurrentTime();
					//curmodel->GetMotionFrame(&curframe);
				}
				else {
					s_previewrange = s_editrange;
					//double rangestart;
					if (s_previewrange.IsSameStartAndEnd()) {
						//rangestart = 1.0;
						curframe = 1.0;
					}
					else {
						if (g_previewFlag == 5) {
							int tmpleng;
							double tmpstart, tmpend;
							s_previewrange.GetRange(&tmpleng, &tmpstart, &tmpend);
							curframe = s_previewrange.GetApplyFrame();
						}
						else {
							//rangestart = s_previewrange.GetStartFrame();
							curframe = s_previewrange.GetStartFrame();
						}
					}
					//curframe = 1.0;//!!!!!!!!!!

					//curframe = s_editrange.GetStartFrame();
					//curframe = s_buttonselectstart;

					s_owpLTimeline->setCurrentTime(curframe, false);
					s_owpEulerGraph->setCurrentTime(curframe, false);
				}
			}
			else if (flag == 2) {//2021/06/18
				if (resetflag == 1) {
					//curframe = s_owpTimeline->getCurrentTime();
					pmodel->GetMotionFrame(&curframe);
					s_owpLTimeline->setCurrentTime(curframe, false);
					s_owpEulerGraph->setCurrentTime(curframe, false);
				}
			}

			if ((g_previewFlag == 4) || (g_previewFlag == 5)) {

				if (g_previewFlag == 4) {
					//curmodel->SetCurrentRigidElem(s_curreindex);//s_curreindexをmodelごとに持つ必要あり！！！
					pmodel->SetCurrentRigidElem(s_reindexmap[pmodel]);//s_curreindexをmodelごとに持つ必要あり！！！

					//決め打ち
					s_btWorld->setGravity(btVector3(0.0, -9.8, 0.0)); // 重力加速度の設定
					
																	  
																	  
					//s_btWorld->setGravity(btVector3(0.0, 0.0, 0.0)); // 重力加速度の設定
					s_bpWorld->setGlobalERP(btScalar(g_erp));// ERP



					//s_model->SetAllKData(-1, s_curreindex, 3, 3, 800.0, 20.0);
					//curmodel->SetAllKData(-1, s_curreindex, 3, 3, 1000.0, 30.0);


					//決め打ち
					//curmodel->SetAllMassDataByBoneLeng(-1, s_curreindex, 30.0);//!!!!!!!! Mass自動設定中 !!!!!


					pmodel->SetMotionFrame(curframe);

					vector<MODELELEM>::iterator itrmodel2;
					for (itrmodel2 = s_modelindex.begin(); itrmodel2 != s_modelindex.end(); itrmodel2++) {
						CModel* updatemodel = itrmodel2->modelptr;
						if (updatemodel) {
							updatemodel->UpdateMatrix(&updatemodel->GetWorldMat(), &s_matVP);
						}
					}

					//curmodel->SetCurrentRigidElem(s_curreindex);//s_curreindexをmodelごとに持つ必要あり！！！reの内容を変えてから呼ぶ
					//s_curreindex = 1;
					pmodel->SetMotionSpeed(g_dspeed);
				}
				else if (g_previewFlag == 5) {
					s_rectime = 0.0;
					s_reccnt = 0;

					pmodel->SetCurrentRigidElem(s_rgdindexmap[pmodel]);//s_rgdindexをmodelごとに持つ必要あり！！！

					s_btWorld->setGravity(btVector3(0.0, 0.0, 0.0)); // 重力加速度の設定

					//s_bpWorld->setGlobalERP(btScalar(g_erp));// ERP

				//ラグドールの時のERPは決め打ち
					s_bpWorld->setGlobalERP(0.0);// ERP
					//s_bpWorld->setGlobalERP(1.0);// ERP
					//s_bpWorld->setGlobalERP(0.2);// ERP
					//s_bpWorld->setGlobalERP(0.001);// ERP
					//s_bpWorld->setGlobalERP(1.0e-8);// ERP


					//s_bpWorld->setGlobalERP(0.00020);// ERP
					//s_bpWorld->setGlobalERP(0.00030);// ERP
					//s_bpWorld->setGlobalERP(g_erp);// ERP
					//s_bpWorld->setGlobalERP(0.00040);// ERP
					//s_bpWorld->setGlobalERP(0.0010);// ERP
					//s_bpWorld->setGlobalERP(0.80);// ERP


					//s_bpWorld->setGlobalERP(0.0010);// ERP
					//s_bpWorld->setGlobalERP(0.0020);// ERP
					//s_bpWorld->setGlobalERP(0.0040);// ERP !!!!

					//s_bpWorld->setGlobalERP(0.0100);// ERP
					//s_bpWorld->setGlobalERP(0.0200);// ERP
					//s_bpWorld->setGlobalERP(0.0400);// ERP

					//s_bpWorld->setGlobalERP(g_erp);// ERP



					pmodel->SetMotionFrame(curframe);

					vector<MODELELEM>::iterator itrmodel3;
					for (itrmodel3 = s_modelindex.begin(); itrmodel3 != s_modelindex.end(); itrmodel3++) {
						CModel* updatemodel = itrmodel3->modelptr;
						if (updatemodel) {
							updatemodel->UpdateMatrix(&updatemodel->GetWorldMat(), &s_matVP);
						}
					}


					//curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 0.1);

					//決め打ち
					pmodel->SetAllKData(-1, s_rgdindexmap[pmodel], 3, 3, 1500.0, 30.0);
										
					//curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 30.0);

/*
					curmodel->SetColTypeAll(s_rgdindex, COL_CONE_INDEX);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					//curmodel->SetColTypeAll(s_rgdindex, COL_CAPSULE_INDEX);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!



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


					//if (s_physicskind == 0) {
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 30.0);
						curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 20.0);
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1600.0, 20.0);
					//}
					//else {
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 60.0);
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 2000.0, 60.0);
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 10000.0, 60.0);
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 13000.0, 200.0);
					//	//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 40000.0, 100.0);
					//	curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 1000.0, 30.0);
					//}

					//curmodel->SetAllKData(-1, s_rgdindex, 3, 3, 0.0, 20.0);

					//s_model->SetAllMassData(-1, s_rgdindex, 100.0);
					//s_model->SetAllMassData(-1, s_rgdindex, 30.0);
					//s_model->SetAllKData(-1, s_rgdindex, 3, 3, 800.0, 30.0);
*/
					//決め打ち
					pmodel->SetAllMassDataByBoneLeng(-1, s_rgdindexmap[pmodel], 30.0);
					
					//curmodel->SetAllMassData(-1, s_rgdindex, 1.0);


					pmodel->SetMotionSpeed(g_dspeed);
				}

				s_btstartframe = curframe;

				//CallF(curmodel->CreateBtObject(s_coldisp, 0), return 1);
				CallF(pmodel->CreateBtObject(1), return 1);


				//if( g_previewFlag == 4 ){

				pmodel->BulletSimulationStart();





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



				//UpdateBtSimu(curframe, curmodel);




				//}

				//if( g_previewFlag == 5 ){
				//	s_model->SetBtImpulse();
				//}


				if (pmodel->GetRgdMorphIndex() >= 0) {
					MOTINFO* morphmi = pmodel->GetRgdMorphInfo();
					if (morphmi) {
						//morphmi->curframe = 0.0;
						morphmi->curframe = s_btstartframe;
					}
				}

			}
		}
	}


	//curmodel : 引数で渡されたmodel
	if (s_model && (curmodel == s_model)) {
		PrepairUndo();
	}


	return 0;
}

int RigidElem2WndParam()
{
	if (!s_model) {
		return 0;
	}
	if( s_curboneno < 0 ){
		return 0;
	}


	//ダイアログの数値はメニューで選択中のもの
	s_model->SetCurrentRigidElem(s_reindexmap[s_model]);


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		//int kinflag = curbone->m_btforce;

		CBone* parentbone = curbone->GetParent();
		if( parentbone ){
			int kinflag = parentbone->GetBtForce();
			s_btforce->setValue( (bool)kinflag );
			
			CRigidElem* curre = parentbone->GetRigidElem( curbone );
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

				s_sphrateSlider->setValue( rate );
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
				s_sphrateSlider->setValue(1.0);
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
			s_sphrateSlider->setValue(1.0);
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
		wcscpy_s( noname, 256, L"BoneName：not selected" );
		s_namelabel->setName( noname );
	}
	if( s_model ){
		s_btgscSlider->setValue( s_model->GetBtGScale(s_model->GetCurReIndex()) );
	}

	s_rigidWnd->callRewrite();



	//再生中、シミュレーション中への対応。元の状態に戻す。
	if (g_previewFlag != 5){
		s_model->SetCurrentRigidElem(s_reindexmap[s_model]);
	}
	else{
		s_model->SetCurrentRigidElem(s_rgdindexmap[s_model]);
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
		swprintf_s( curlabel, 512, L"BonaName：%s", curbto->GetEndBone()->GetWBoneName() );
		if( s_namelabel ){
			s_namelabel->setName( curlabel );
		}

		WCHAR curlabel2[256];
		swprintf_s( curlabel2, 256, L"BoneLength:%.3f[m]", curbto->GetBoneLeng() );
		if( s_lenglabel ){
			s_lenglabel->setName( curlabel2 );
		}
	}else{
		WCHAR curlabel[512];
		wcscpy_s( curlabel, 512, L"BoneName：not selected" );
		if( s_namelabel ){
			s_namelabel->setName( curlabel );
		}

		WCHAR curlabel2[256];
		wcscpy_s( curlabel2, 256, L"BoneLength：not selected" );
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
	if( s_rgdindexmap[s_model] < 0 ){
		return 0;
	}


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		CBone* parentbone = curbone->GetParent();
		if( parentbone ){
			ChaVector3 setimp( 0.0f, 0.0f, 0.0f );


			int impnum = parentbone->GetImpMapSize();
			if( (s_model->GetCurImpIndex() >= 0) && (s_model->GetCurImpIndex() < impnum) ){
				string curimpname = s_model->GetImpInfo( s_model->GetCurImpIndex() );
				setimp = parentbone->GetImpMap( curimpname, curbone );
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
	if( s_rgdindexmap[s_model] < 0 ){
		return 0;
	}


	CBone* curbone = s_model->GetBoneByID( s_curboneno );
	if( curbone ){
		CBone* parentbone = curbone->GetParent();
		if( parentbone ){
			char* filename = s_model->GetRigidElemInfo( s_rgdindexmap[s_model] ).filename;
			CRigidElem* curre = parentbone->GetRigidElemOfMap( filename, curbone );
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
	if (!s_bpWorld) {
		return 0;
	}

	if (s_modelindex.empty()) {
		return 0;
	}

	int dlgret;
	dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVECHADLG),
		s_3dwnd, (DLGPROC)SaveChaDlgProc);
	if ((dlgret != IDOK) || !s_projectname[0] || !s_projectdir[0]) {
		return 0;
	}


	s_savelimitdegflag = g_limitdegflag;
	g_limitdegflag = g_bakelimiteulonsave;
	if (s_LimitDegCheckBox) {
		s_LimitDegCheckBox->SetChecked(g_bakelimiteulonsave);
	}


	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel) {
			s_owpLTimeline->setCurrentTime(0.0, true);
			s_owpEulerGraph->setCurrentTime(0.0, false);
			curmodel->SetMotionFrame(0.0);
			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);

			//ここでAxisMatXの初期化
			curmodel->CreateBtObject(1);
			curmodel->CalcBtAxismat(2);//2
			curmodel->SetInitAxisMatX(1);
		}
	}


	WCHAR saveprojpath[MAX_PATH] = { 0L };
	swprintf_s(saveprojpath, MAX_PATH, L"%s\\%s\\%s.cha", s_projectdir, s_projectname, s_projectname);


	CChaFile chafile;
	int result = chafile.WriteChaFile(s_bpWorld, s_projectdir, s_projectname, s_modelindex, (float)g_dspeed);
	if (result) {
		::MessageBox(s_mainhwnd, L"保存に失敗しました。", L"Error", MB_OK);
		g_limitdegflag = s_savelimitdegflag;
		if (s_LimitDegCheckBox) {
			s_LimitDegCheckBox->SetChecked(s_savelimitdegflag);
		}
		return 1;
	}


	//書き込み処理が成功してから履歴を保存する。chaファイルだけ。
	int savepathlen;
	saveprojpath[MAX_PATH - 1] = 0L;
	savepathlen = (int)wcslen(saveprojpath);
	if (savepathlen > 4) {
		WCHAR* pwext;
		pwext = saveprojpath + ((size_t)savepathlen - 1) - 3;
		if (wcscmp(pwext, L".cha") == 0) {
			SYSTEMTIME localtime;
			GetLocalTime(&localtime);
			WCHAR HistoryForOpeningProjectWithGamePad[MAX_PATH] = { 0L };
			swprintf_s(HistoryForOpeningProjectWithGamePad, MAX_PATH, L"%s\\MB3DOpenProj_%04u%02u%02u%02u%02u%02u.txt",
				s_temppath,
				localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
			HANDLE hfile;
			hfile = CreateFile(HistoryForOpeningProjectWithGamePad, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				//int pathlen;
				//pathlen = (int)wcslen(saveprojpath);
				//if ((pathlen > 0) && (pathlen < MAX_PATH)) {
				if ((savepathlen > 0) && (savepathlen < MAX_PATH)) {
					DWORD writelen = 0;
					WriteFile(hfile, saveprojpath, (savepathlen * sizeof(WCHAR)), &writelen, NULL);
					_ASSERT((savepathlen * sizeof(WCHAR)) == writelen);
				}
				CloseHandle(hfile);
			}
		}
	}

	g_limitdegflag = s_savelimitdegflag;
	if (s_LimitDegCheckBox) {
		s_LimitDegCheckBox->SetChecked(s_savelimitdegflag);
	}


	return 0;
}


LRESULT CALLBACK SaveChaDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

	//	static WCHAR s_projectname[64] = {0L};
	//	static WCHAR s_projectdir[MAX_PATH] = {0L};

	BROWSEINFO bi;
	LPITEMIDLIST curlpidl = 0;
	WCHAR dispname[MAX_PATH] = { 0L };
	WCHAR selectname[MAX_PATH] = { 0L };
	int iImage = 0;

	static int s_savechaproctimer = 351;


	switch (msg) {
	case WM_INITDIALOG:
		if (s_model && s_model->GetCurMotInfo()) {
			if (s_chasavename[0]) {
				SetDlgItemText(hDlgWnd, IDC_PROJNAME, s_chasavename);
			}
			if (s_chasavedir[0]) {
				SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_chasavedir);
			}
			else {
				if (s_projectdir[0]) {
					SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir);
				}
				else {
					LPITEMIDLIST pidl;

					HWND hWnd = NULL;

					IMalloc *pMalloc;
					if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
						if (SUCCEEDED(SHGetSpecialFolderLocation(s_3dwnd, CSIDL_DESKTOPDIRECTORY, &pidl)))
						{
							// パスに変換する
							SHGetPathFromIDList(pidl, s_projectdir);
								// 取得したIDLを解放する (CoTaskMemFreeでも可)
								pMalloc->Free(pidl);
								SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir);
						}
						pMalloc->Release();
					}
				}
			}

			if (g_bakelimiteulonsave) {
				CheckDlgButton(hDlgWnd, IDC_CHECK1, BST_CHECKED);
			}
			else {
				CheckDlgButton(hDlgWnd, IDC_CHECK1, BST_UNCHECKED);
			}
			

			RECT dlgrect;
			GetWindowRect(hDlgWnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);

			s_savechadlghwnd = hDlgWnd;
			SetTimer(hDlgWnd, s_savechaproctimer, 20, NULL);

		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
		{


			s_savechadlghwnd = 0;
			KillTimer(hDlgWnd, s_savechaproctimer);

			GetDlgItemText(hDlgWnd, IDC_PROJNAME, s_projectname, 64);
			GetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir, MAX_PATH);
			wcscpy_s(s_chasavename, 64, s_projectname);
			wcscpy_s(s_chasavedir, MAX_PATH, s_projectdir);


			UINT ischecked = IsDlgButtonChecked(hDlgWnd, IDC_CHECK1);
			if (ischecked == BST_CHECKED) {
				g_bakelimiteulonsave = true;
			}
			else {
				g_bakelimiteulonsave = false;
			}

			EndDialog(hDlgWnd, IDOK);
		}
			break;
		case IDCANCEL:
			s_savechadlghwnd = 0;
			KillTimer(hDlgWnd, s_savechaproctimer);

			s_projectname[0] = 0L;
			s_projectdir[0] = 0L;
			EndDialog(hDlgWnd, IDCANCEL);
			break;
		case IDC_REFDIR:
		{
			bi.hwndOwner = hDlgWnd;
			bi.pidlRoot = NULL;//!!!!!!!
			bi.pszDisplayName = dispname;
			//bi.lpszTitle = L"保存フォルダを選択してください。";
			bi.lpszTitle = L"SelectDirectoryForSave";
			//bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;// | BIF_NEWDIALOGSTYLE;//BIF_NEWDIALOGSTYLEを指定すると固まる　謎
			bi.lpfn = NULL;
			bi.lParam = 0;
			bi.iImage = iImage;


			s_getfilenamehwnd = 0;
			s_getfilenametreeview = 0;
			HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
				WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
			InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

			curlpidl = SHBrowseForFolder(&bi);

			InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
			UnhookWinEvent(hhook);
			s_getfilenamehwnd = 0;
			s_getfilenametreeview = 0;

			if (curlpidl) {
				//::DSMessageBox( m_hWnd, dispname, "フォルダー名", MB_OK );

				BOOL bret;
				bret = SHGetPathFromIDList(curlpidl, selectname);
				if (bret == FALSE) {
					_ASSERT(0);
					if (curlpidl)
						CoTaskMemFree(curlpidl);
					return 1;
				}

				if (curlpidl)
					CoTaskMemFree(curlpidl);

				wcscpy_s(s_projectdir, MAX_PATH, selectname);
				SetDlgItemText(hDlgWnd, IDC_DIRNAME, s_projectdir);
			}
		}
			break;
		default:
			return FALSE;
		}
	case WM_TIMER:
		OnDSUpdate();
		return FALSE;
		break;
	default:
		DefWindowProc(hDlgWnd, msg, wp, lp);
		return FALSE;
	}
	return TRUE;

}

int OpenChaFile()
{

	//g_tmpmqopathはプロジェクト読み込み時にプロジェクトファイル内に記述されているファイル名に変わっていくので先に保存しておく。
	WCHAR saveprojpath[MAX_PATH] = { 0L };
	wcscpy_s(saveprojpath, MAX_PATH, g_tmpmqopath);

	WCHAR* lasten = 0;
	g_tmpmqopath[MAX_PATH - 1] = 0L;
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
		s_bpWorld->setGlobalERP(btScalar(g_erp));// ERP
									   //s_bpWorld->start();// ウィンドウを表示して，シミュレーションを開始する
		s_btWorld = s_bpWorld->getDynamicsWorld();
		s_bpWorld->setNumThread(g_numthread);

	}

	HCURSOR oldcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));




	CChaFile chafile;
	int ret = chafile.LoadChaFile( g_tmpmqopath, OpenFBXFile, OpenREFile, OpenImpFile, OpenGcoFile, OnREMenu, OnRgdMenu, OnRgdMorphMenu, OnImpMenu );
	if (ret == 1) {
		_ASSERT(0);
		return 1;
	}
	//OnAddMotion(s_model->GetCurMotInfo()->motid);

	SetCursor(oldcursor);
	
	//読み込み処理が成功してから履歴を保存する。chaファイルだけ。
	int savepathlen;
	saveprojpath[MAX_PATH - 1] = 0L;
	savepathlen = (int)wcslen(saveprojpath);
	if (savepathlen > 4) {
		WCHAR* pwext;
		pwext = saveprojpath + ((size_t)savepathlen - 1) - 3;
		if (wcscmp(pwext, L".cha") == 0) {
			SYSTEMTIME localtime;
			GetLocalTime(&localtime);
			WCHAR HistoryForOpeningProjectWithGamePad[MAX_PATH] = { 0L };
			swprintf_s(HistoryForOpeningProjectWithGamePad, MAX_PATH, L"%s\\MB3DOpenProj_%04u%02u%02u%02u%02u%02u.txt",
				s_temppath,
				localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
			HANDLE hfile;
			hfile = CreateFile(HistoryForOpeningProjectWithGamePad, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				//int pathlen;
				//pathlen = (int)wcslen(saveprojpath);
				if ((savepathlen > 0) && (savepathlen < MAX_PATH)) {
					DWORD writelen = 0;
					WriteFile(hfile, saveprojpath, (savepathlen * sizeof(WCHAR)), &writelen, NULL);
					_ASSERT((savepathlen * sizeof(WCHAR)) == writelen);
				}
				CloseHandle(hfile);
			}
		}
	}

	//ChangeCurrentBone();

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


int SetSpAxisParams()
{
	if( !(s_spaxis[0].sprite) ){
		return 0;
	}

	float spawidth = 50.0f;
	float spaheight = 50.0f;
	int spashift = 6;
	//s_spaxis[0].dispcenter.x = (int)( s_mainwidth * 0.57f );
	//s_spaxis[0].dispcenter.y = (int)( 30.0f * ( (float)s_mainheight / 620.0 ) );
	//s_spaxis[0].dispcenter.x = s_mainwidth - 50 - 10 - (32 + 12) * 4;
	//s_spaxis[0].dispcenter.y = 16 + 10;
	s_spaxis[0].dispcenter.x = s_mainwidth - 35 - 50 - 10;
	s_spaxis[0].dispcenter.y = 35;
	//spashift = (int)( (float)spashift * ( (float)s_mainwidth / 600.0 ) );

	s_spaxis[1].dispcenter.x = s_spaxis[0].dispcenter.x;
	s_spaxis[1].dispcenter.y = s_spaxis[0].dispcenter.y + (int)spaheight + spashift;

	s_spaxis[2].dispcenter.x = s_spaxis[0].dispcenter.x;
	s_spaxis[2].dispcenter.y = s_spaxis[1].dispcenter.y + (int)spaheight + spashift;;

	int spacnt;
	for( spacnt = 0; spacnt < SPAXISNUM; spacnt++ ){
		ChaVector3 disppos;
		disppos.x = (float)( s_spaxis[spacnt].dispcenter.x ) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)( s_spaxis[spacnt].dispcenter.y ) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2( spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f );
		if (s_spaxis[spacnt].sprite) {
			CallF(s_spaxis[spacnt].sprite->SetPos(disppos), return 1);
			CallF(s_spaxis[spacnt].sprite->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
	}

	return 0;

}

int SetSpRigidSWParams()
{
	if (!(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteON) || !(s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}

	//float spgwidth = 140.0f;
	float spgwidth = 124.0f;
	float spgheight = 28.0f;
	int spgshift = 6;
	s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.x = s_guibarX0;
	//s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y = 486;
	
	
	//s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y = 486 - MAINMENUAIMBARH;
	if (g_4kresolution) {
		s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y = 486 * 2 - MAINMENUAIMBARH + 32;
	}else{
		s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y = 486 - MAINMENUAIMBARH;
	}


	s_sprigidsw[SPRIGIDSW_IMPULSE].dispcenter.x = s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.x + (int)spgwidth + spgshift;
	s_sprigidsw[SPRIGIDSW_IMPULSE].dispcenter.y = s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y;

	s_sprigidsw[SPRIGIDSW_GROUNDPLANE].dispcenter.x = s_sprigidsw[SPRIGIDSW_IMPULSE].dispcenter.x + (int)spgwidth + spgshift;
	s_sprigidsw[SPRIGIDSW_GROUNDPLANE].dispcenter.y = s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y;

	s_sprigidsw[SPRIGIDSW_DAMPANIM].dispcenter.x = s_sprigidsw[SPRIGIDSW_GROUNDPLANE].dispcenter.x + (int)spgwidth + spgshift;
	s_sprigidsw[SPRIGIDSW_DAMPANIM].dispcenter.y = s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y;

	int spgcnt;
	for (spgcnt = 0; spgcnt < SPRIGIDSWNUM; spgcnt++) {
		ChaVector3 disppos;
		disppos.x = (float)(s_sprigidsw[spgcnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_sprigidsw[spgcnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

		if (s_sprigidsw[spgcnt].spriteON) {
			CallF(s_sprigidsw[spgcnt].spriteON->SetPos(disppos), return 1);
			CallF(s_sprigidsw[spgcnt].spriteON->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
		if (s_sprigidsw[spgcnt].spriteOFF) {
			CallF(s_sprigidsw[spgcnt].spriteOFF->SetPos(disppos), return 1);
			CallF(s_sprigidsw[spgcnt].spriteOFF->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}

	}

	return 0;

}

int SetSpRetargetSWParams()
{
	if (!(s_spretargetsw[SPRETARGETSW_RETARGET].spriteON) || !(s_spretargetsw[SPRETARGETSW_RETARGET].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}

	//float spgwidth = 140.0f;
	float spgwidth = 124.0f;
	float spgheight = 28.0f;
	int spgshift = 6;
	s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.x = s_guibarX0;
	
	
	//s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.y = 486 - MAINMENUAIMBARH;
	if (g_4kresolution) {
		s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.y = 486 * 2 - MAINMENUAIMBARH + 32;
	}
	else {
		s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.y = 486 - MAINMENUAIMBARH;
	}


	s_spretargetsw[SPRETARGETSW_LIMITEULER].dispcenter.x = s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.x + (int)spgwidth + spgshift;
	s_spretargetsw[SPRETARGETSW_LIMITEULER].dispcenter.y = s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.y;


	int sprcnt;
	for (sprcnt = 0; sprcnt < SPRETARGETSWNUM; sprcnt++) {
		ChaVector3 disppos;
		disppos.x = (float)(s_spretargetsw[sprcnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spretargetsw[sprcnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

		if (s_spretargetsw[sprcnt].spriteON) {
			CallF(s_spretargetsw[sprcnt].spriteON->SetPos(disppos), return 1);
			CallF(s_spretargetsw[sprcnt].spriteON->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
		if (s_spretargetsw[sprcnt].spriteOFF) {
			CallF(s_spretargetsw[sprcnt].spriteOFF->SetPos(disppos), return 1);
			CallF(s_spretargetsw[sprcnt].spriteOFF->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
	}

	return 0;

}

int SetSpMenuAimBarParams()
{
	if (!(s_spmenuaimbar[0].spriteON) || !(s_spmenuaimbar[0].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}

	////float spgwidth = 140.0f;
	////float spgheight = 6.0f;
	////int spgshift = 6;

	//HMENU mainmenu;
	//mainmenu = GetMenu(s_mainhwnd);
	//int menuno;
	//for (menuno = 0; menuno < SPMENU_MAX; menuno++) {
	//	HMENU submenu = GetSubMenu(mainmenu, menuno);
	//	if (submenu) {
	//		int curmenuitemid;
	//		curmenuitemid = ::GetMenuItemID(submenu, 0);
	//		if (curmenuitemid >= 0) {
	//			RECT rc;
	//			GetMenuItemRect(s_mainhwnd, mainmenu, menuno, &rc);
	//			int spritewidth = rc.right - rc.left;//org:140
	//			int spriteheight = 6;//org:6

	//			POINT point1;
	//			POINT point2;
	//			point1.x = rc.left;
	//			point1.y = rc.top;
	//			point2.x = rc.right;
	//			point2.y = rc.bottom;
	//			::ClientToScreen(s_mainhwnd, &point1);
	//			::ClientToScreen(s_mainhwnd, &point2);
	//			//::ScreenToClient(s_mainmenuaimbarWnd->getHWnd(), &point1);
	//			//::ScreenToClient(s_mainmenuaimbarWnd->getHWnd(), &point2);

	//			s_spmenuaimbar[menuno].dispcenter.x = point1.x;//screen pos!!!!!!!!!!!
	//			s_spmenuaimbar[menuno].dispcenter.y = 16;

	//			//ChaVector3 disppos;
	//			//disppos.x = (float)(s_spmenuaimbar[menuno].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	//			//disppos.y = -((float)(s_spmenuaimbar[menuno].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	//			//disppos.z = 0.0f;
	//			//ChaVector2 dispsize = ChaVector2(spritewidth / (float)s_mainwidth * 2.0f, spriteheight / (float)s_mainheight * 2.0f);

	//			//if (s_spmenuaimbar[menuno].spriteON) {
	//			//	CallF(s_spmenuaimbar[menuno].spriteON->SetPos(disppos), return 1);
	//			//	CallF(s_spmenuaimbar[menuno].spriteON->SetSize(dispsize), return 1);
	//			//}
	//			//else {
	//			//	_ASSERT(0);
	//			//}
	//			//if (s_spmenuaimbar[menuno].spriteOFF) {
	//			//	CallF(s_spmenuaimbar[menuno].spriteOFF->SetPos(disppos), return 1);
	//			//	CallF(s_spmenuaimbar[menuno].spriteOFF->SetSize(dispsize), return 1);
	//			//}
	//			//else {
	//			//	_ASSERT(0);
	//			//}

	//		}
	//	}

	//}

	return 0;

}


int SetSpAimBarParams()
{
	if (!(s_spaimbar[SPAIMBAR_1].spriteON) || !(s_spaimbar[SPAIMBAR_1].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}

	//float spgwidth = 140.0f;
	float spgwidth = 124.0f;
	float spgheight = 6.0f;
	int spgshift = 6;
	s_spaimbar[SPAIMBAR_1].dispcenter.x = s_guibarX0;
	//s_spaimbar[SPAIMBAR_1].dispcenter.y = 486 + (28 / 2) + (6 / 2);
	
	
	//s_spaimbar[SPAIMBAR_1].dispcenter.y = 486 + (28 / 2) + (6 / 2) - MAINMENUAIMBARH;
	if (g_4kresolution) {
		s_spaimbar[SPAIMBAR_1].dispcenter.y = 486 * 2 - MAINMENUAIMBARH + 32 + 16 + 4;
	}
	else {
		s_spaimbar[SPAIMBAR_1].dispcenter.y = 486 + (28 / 2) + (6 / 2) - MAINMENUAIMBARH;
	}

	
	s_spaimbar[SPAIMBAR_2].dispcenter.x = s_spaimbar[SPAIMBAR_1].dispcenter.x + (int)spgwidth + spgshift;
	s_spaimbar[SPAIMBAR_2].dispcenter.y = s_spaimbar[SPAIMBAR_1].dispcenter.y;

	s_spaimbar[SPAIMBAR_3].dispcenter.x = s_spaimbar[SPAIMBAR_2].dispcenter.x + (int)spgwidth + spgshift;
	s_spaimbar[SPAIMBAR_3].dispcenter.y = s_spaimbar[SPAIMBAR_1].dispcenter.y;

	s_spaimbar[SPAIMBAR_4].dispcenter.x = s_spaimbar[SPAIMBAR_3].dispcenter.x + (int)spgwidth + spgshift;
	s_spaimbar[SPAIMBAR_4].dispcenter.y = s_spaimbar[SPAIMBAR_1].dispcenter.y;

	s_spaimbar[SPAIMBAR_5].dispcenter.x = s_spaimbar[SPAIMBAR_4].dispcenter.x + (int)spgwidth + spgshift;
	s_spaimbar[SPAIMBAR_5].dispcenter.y = s_spaimbar[SPAIMBAR_1].dispcenter.y;

	int spgcnt;
	for (spgcnt = 0; spgcnt < SPAIMBARNUM; spgcnt++) {
		ChaVector3 disppos;
		disppos.x = (float)(s_spaimbar[spgcnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spaimbar[spgcnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

		if (s_spaimbar[spgcnt].spriteON) {
			CallF(s_spaimbar[spgcnt].spriteON->SetPos(disppos), return 1);
			CallF(s_spaimbar[spgcnt].spriteON->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
		if (s_spaimbar[spgcnt].spriteOFF) {
			CallF(s_spaimbar[spgcnt].spriteOFF->SetPos(disppos), return 1);
			CallF(s_spaimbar[spgcnt].spriteOFF->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}

	}

	return 0;

}

int SetSpMouseCenterParams()
{
	if (!s_mousecenteron.sprite) {
		_ASSERT(0);
		return 0;
	}

	//float spgwidth = 91.0f;
	//float spgheight = 145.0f;
	//float spgwidth = 50;
	//float spgheight = 67;
	float spgwidth = 50.0f * 0.6f;
	float spgheight = 67.0f * 0.6f;
	int spgshift = 6;

	_ASSERT(s_3dwnd);
	RECT clientrect;
	::GetClientRect(s_3dwnd, &clientrect);

	//s_spsel3d.dispcenter.x = (LONG)(clientrect.right - spgwidth / 2 - 20);
	//s_spsel3d.dispcenter.y = (LONG)(clientrect.top + spgheight / 2 + 20);
	s_mousecenteron.dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - (50 + 6) * 6;
	s_mousecenteron.dispcenter.y = (LONG)(clientrect.top + spgheight / 2 + 20);

	ChaVector3 disppos;
	disppos.x = (float)(s_mousecenteron.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_mousecenteron.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

	if (s_mousecenteron.sprite) {
		CallF(s_mousecenteron.sprite->SetPos(disppos), return 1);
		CallF(s_mousecenteron.sprite->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}
	
	return 0;

}

int SetSpSel3DParams()
{
	if (!(s_spsel3d.spriteON) || !(s_spsel3d.spriteOFF)) {
		_ASSERT(0);
		return 0;
	}


/*
	float spawidth = 50.0f;
	int spashift = 6;
	//s_spcam[0].dispcenter.x = (int)(s_mainwidth * 0.57f);
	//s_spcam[0].dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0)) + (int(spawidth * 1.5f));
	//s_spcam[0].dispcenter.x = s_mainwidth - 50 - 10 - (32 + 12) * 3;
	//s_spcam[0].dispcenter.y = 16 + 10 + (int(spawidth * 1.5f));
	s_spcam[0].dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - (50 + 6) * 4;
	s_spcam[0].dispcenter.y = 25 + 10;
*/


	//float spgwidth = 91.0f;
	//float spgheight = 145.0f;
	float spgwidth = 91.0f * 0.25f;
	float spgheight = 145.0f * 0.25f;
	int spgshift = 6;

	_ASSERT(s_3dwnd);
	RECT clientrect;
	::GetClientRect(s_3dwnd, &clientrect);

	//s_spsel3d.dispcenter.x = (LONG)(clientrect.right - spgwidth / 2 - 20);
	//s_spsel3d.dispcenter.y = (LONG)(clientrect.top + spgheight / 2 + 20);
	s_spsel3d.dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - (50 + 6) * 7;
	s_spsel3d.dispcenter.y = (LONG)(clientrect.top + spgheight / 2 + 20);

	ChaVector3 disppos;
	disppos.x = (float)(s_spsel3d.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_spsel3d.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

	if (s_spsel3d.spriteON) {
		CallF(s_spsel3d.spriteON->SetPos(disppos), return 1);
		CallF(s_spsel3d.spriteON->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}
	if (s_spsel3d.spriteOFF) {
		CallF(s_spsel3d.spriteOFF->SetPos(disppos), return 1);
		CallF(s_spsel3d.spriteOFF->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}

	return 0;

}

int SetSpIkModeSWParams()
{
	if (!(s_spikmodesw[0].spriteON) || !(s_spikmodesw[0].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}


	float spgwidth = 50.0f;
	float spgheight = 50.0f;
	int spgshift = 6;
	s_spikmodesw[0].dispcenter.x = s_mainwidth - 35;
	s_spikmodesw[0].dispcenter.y = 35;

	s_spikmodesw[1].dispcenter.x = s_spikmodesw[0].dispcenter.x;
	s_spikmodesw[1].dispcenter.y = s_spikmodesw[0].dispcenter.y + (int)spgheight + spgshift;

	s_spikmodesw[2].dispcenter.x = s_spikmodesw[1].dispcenter.x;
	s_spikmodesw[2].dispcenter.y = s_spikmodesw[1].dispcenter.y + (int)spgheight + spgshift;

	int spgcnt;
	for (spgcnt = 0; spgcnt < 3; spgcnt++) {
		ChaVector3 disppos;
		disppos.x = (float)(s_spikmodesw[spgcnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spikmodesw[spgcnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

		if (s_spikmodesw[spgcnt].spriteON) {
			CallF(s_spikmodesw[spgcnt].spriteON->SetPos(disppos), return 1);
			CallF(s_spikmodesw[spgcnt].spriteON->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
		if (s_spikmodesw[spgcnt].spriteOFF) {
			CallF(s_spikmodesw[spgcnt].spriteOFF->SetPos(disppos), return 1);
			CallF(s_spikmodesw[spgcnt].spriteOFF->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
	}

	return 0;
}


int SetSpRefPosSWParams()
{
	if (!(s_sprefpos.spriteON) || !(s_sprefpos.spriteOFF)) {
		_ASSERT(0);
		return 0;
	}


	float spgwidth = 50.0f;
	float spgheight = 50.0f;
	int spgshift = 6;
	s_sprefpos.dispcenter.x = s_mainwidth - 35;
	s_sprefpos.dispcenter.y = 35 + ((int)spgheight + spgshift) * 3;

	ChaVector3 disppos;
	disppos.x = (float)(s_sprefpos.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_sprefpos.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

	if (s_sprefpos.spriteON) {
		CallF(s_sprefpos.spriteON->SetPos(disppos), return 1);
		CallF(s_sprefpos.spriteON->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}
	if (s_sprefpos.spriteOFF) {
		CallF(s_sprefpos.spriteOFF->SetPos(disppos), return 1);
		CallF(s_sprefpos.spriteOFF->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}

	return 0;
}

int SetSpGUISWParams()
{
	if (!(s_spguisw[SPGUISW_SPRITEFK].spriteON) || !(s_spguisw[SPGUISW_SPRITEFK].spriteOFF)) {
		_ASSERT(0);
		return 0;
	}
	if (!(s_spret2prev.sprite)) {
		_ASSERT(0);
		return 0;
	}

	{
		float spretwidth = 32.0f;
		float spretheight = 32.0f;
		int spretshift = 0;
		//s_spret2prev.dispcenter.x = (LONG)(16.0f + 8.0f);
		s_spret2prev.dispcenter.x = s_guibarX0 - 70 - (LONG)(16.0f + 8.0f);
		//s_spret2prev.dispcenter.y = 486;
		if (g_4kresolution) {
			s_spret2prev.dispcenter.y = 486 * 2 - MAINMENUAIMBARH + 32;
		}
		else {
			s_spret2prev.dispcenter.y = 486 - MAINMENUAIMBARH;
		}

		ChaVector3 disppos;
		disppos.x = (float)(s_spret2prev.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spret2prev.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spretwidth / (float)s_mainwidth * 2.0f, spretheight / (float)s_mainheight * 2.0f);

		if (s_spret2prev.sprite) {
			CallF(s_spret2prev.sprite->SetPos(disppos), return 1);
			CallF(s_spret2prev.sprite->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
	}



	//float spgwidth = 140.0f;
	float spgwidth = 124.0f;
	float spgheight = 28.0f;
	int spgshift = 6;
	s_spguisw[SPGUISW_SPRITEFK].dispcenter.x = s_guibarX0;
	//s_spguisw[SPGUISW_SPRITEFK].dispcenter.y = 486;

	//s_spguisw[SPGUISW_SPRITEFK].dispcenter.y = 486 * s_winsizemult - MAINMENUAIMBARH;
	if (g_4kresolution) {
		s_spguisw[SPGUISW_SPRITEFK].dispcenter.y = 486 * 2 - MAINMENUAIMBARH + 32;
	}
	else {
		s_spguisw[SPGUISW_SPRITEFK].dispcenter.y = 486 - MAINMENUAIMBARH;
	}

	s_spguisw[SPGUISW_LEFT].dispcenter.x = s_spguisw[SPGUISW_SPRITEFK].dispcenter.x + (int)spgwidth + spgshift;
	s_spguisw[SPGUISW_LEFT].dispcenter.y = s_spguisw[SPGUISW_SPRITEFK].dispcenter.y;

	s_spguisw[SPGUISW_LEFT2ND].dispcenter.x = s_spguisw[SPGUISW_LEFT].dispcenter.x + (int)spgwidth + spgshift;
	s_spguisw[SPGUISW_LEFT2ND].dispcenter.y = s_spguisw[SPGUISW_SPRITEFK].dispcenter.y;

	s_spguisw[SPGUISW_BULLETPHYSICS].dispcenter.x = s_spguisw[SPGUISW_LEFT2ND].dispcenter.x + (int)spgwidth + spgshift;
	s_spguisw[SPGUISW_BULLETPHYSICS].dispcenter.y = s_spguisw[SPGUISW_SPRITEFK].dispcenter.y;

	s_spguisw[SPGUISW_PHYSICSIK].dispcenter.x = s_spguisw[SPGUISW_BULLETPHYSICS].dispcenter.x + (int)spgwidth + spgshift;
	s_spguisw[SPGUISW_PHYSICSIK].dispcenter.y = s_spguisw[SPGUISW_SPRITEFK].dispcenter.y;

	int spgcnt;
	for (spgcnt = 0; spgcnt < SPGUISWNUM; spgcnt++) {
		ChaVector3 disppos;
		disppos.x = (float)(s_spguisw[spgcnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spguisw[spgcnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spgwidth / (float)s_mainwidth * 2.0f, spgheight / (float)s_mainheight * 2.0f);

		if (s_spguisw[spgcnt].spriteON) {
			CallF(s_spguisw[spgcnt].spriteON->SetPos(disppos), return 1);
			CallF(s_spguisw[spgcnt].spriteON->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
		if (s_spguisw[spgcnt].spriteOFF) {
			CallF(s_spguisw[spgcnt].spriteOFF->SetPos(disppos), return 1);
			CallF(s_spguisw[spgcnt].spriteOFF->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}

	}

	return 0;

}



int SetSpCamParams()
{
	if (!(s_spcam[SPR_CAM_I].sprite) || !(s_spcam[SPR_CAM_KAI].sprite) || !(s_spcam[SPR_CAM_KAKU].sprite)){
		return 0;
	}

	float spawidth = 50.0f;
	int spashift = 6;
	//s_spcam[0].dispcenter.x = (int)(s_mainwidth * 0.57f);
	//s_spcam[0].dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0)) + (int(spawidth * 1.5f));
	//s_spcam[0].dispcenter.x = s_mainwidth - 50 - 10 - (32 + 12) * 3;
	//s_spcam[0].dispcenter.y = 16 + 10 + (int(spawidth * 1.5f));
	s_spcam[0].dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - (50 + 6) * 4;
	s_spcam[0].dispcenter.y = 25 + 10;

	//spashift = (int)((float)spashift * ((float)s_mainwidth / 600.0));

	s_spcam[1].dispcenter.x = s_spcam[0].dispcenter.x + (int)(spawidth)+spashift;
	s_spcam[1].dispcenter.y = s_spcam[0].dispcenter.y;

	s_spcam[2].dispcenter.x = s_spcam[1].dispcenter.x + (int)(spawidth)+spashift;
	s_spcam[2].dispcenter.y = s_spcam[0].dispcenter.y;

	int spacnt;
	for (spacnt = 0; spacnt < SPR_CAM_MAX; spacnt++){
		ChaVector3 disppos;
		disppos.x = (float)(s_spcam[spacnt].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
		disppos.y = -((float)(s_spcam[spacnt].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
		disppos.z = 0.0f;
		ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
		if (s_spcam[spacnt].sprite) {
			CallF(s_spcam[spacnt].sprite->SetPos(disppos), return 1);
			CallF(s_spcam[spacnt].sprite->SetSize(dispsize), return 1);
		}
		else {
			_ASSERT(0);
		}
	}

	return 0;

}

int SetSpRigParams()
{
	if (!(s_sprig[SPRIG_INACTIVE].sprite) || !(s_sprig[SPRIG_ACTIVE].sprite)){
		return 0;
	}

/*
	//sprefpos
	float spgwidth = 50.0f;
	float spgheight = 50.0f;
	int spgshift = 6;
	s_sprefpos.dispcenter.x = s_mainwidth - 35;
	s_sprefpos.dispcenter.y = 35 + ((int)spgheight + spgshift) * 3;
*/

	float spawidth = 50.0f;
	float spaheight = 50.0f;
	int spashift = 6;
	//spashift = (int)((float)spashift * ((float)s_mainwidth / s_2ndposy));
	//s_sprig[SPRIG_INACTIVE].dispcenter.x = (int)(s_mainwidth * 0.57f) + ((int)(spawidth)+spashift) * 3;
	//s_sprig[SPRIG_INACTIVE].dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0));// +(int(spawidth * 1.5f) * 2);
	//s_sprig[SPRIG_INACTIVE].dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - 50 - 6;
	//s_sprig[SPRIG_INACTIVE].dispcenter.y = 25 + 10;
	s_sprig[SPRIG_INACTIVE].dispcenter.x = s_mainwidth - 35;
	s_sprig[SPRIG_INACTIVE].dispcenter.y = 35 + ((int)spaheight + spashift) * 4;

	//s_spcam[0].dispcenter.x = s_mainwidth - 50 - 10 - 50 - 6 - (50 + 6) * 4;
	//s_spcam[0].dispcenter.y = 25 + 10;

	s_sprig[SPRIG_ACTIVE].dispcenter.x = s_sprig[SPRIG_INACTIVE].dispcenter.x;
	s_sprig[SPRIG_ACTIVE].dispcenter.y = s_sprig[SPRIG_INACTIVE].dispcenter.y;


	ChaVector3 disppos;
	disppos.x = (float)(s_sprig[0].dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_sprig[0].dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
	if (s_sprig[SPRIG_INACTIVE].sprite) {
		CallF(s_sprig[SPRIG_INACTIVE].sprite->SetPos(disppos), return 1);
		CallF(s_sprig[SPRIG_INACTIVE].sprite->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}
	if (s_sprig[SPRIG_ACTIVE].sprite) {
		CallF(s_sprig[SPRIG_ACTIVE].sprite->SetPos(disppos), return 1);
		CallF(s_sprig[SPRIG_ACTIVE].sprite->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}

	return 0;

}

int SetSpMouseHereParams()
{
	if (!s_spmousehere.sprite) {
		return 0;
	}

	float spawidth = 52.0f;
	int spashift = 50;
	s_spmousehere.dispcenter.x = 0;
	s_spmousehere.dispcenter.y = 0;


	ChaVector3 disppos;
	disppos.x = (float)(s_spmousehere.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
	disppos.y = -((float)(s_spmousehere.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
	disppos.z = 0.0f;
	ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
	if (s_spmousehere.sprite) {
		CallF(s_spmousehere.sprite->SetPos(disppos), return 1);
		CallF(s_spmousehere.sprite->SetSize(dispsize), return 1);
	}
	else {
		_ASSERT(0);
	}

	return 0;
}

//int SetSpBtParams()
//{
//	if (!s_spbt.sprite){
//		return 0;
//	}
//
//	float spawidth = 32.0f;
//	int spashift = 12;
//	spashift = (int)((float)spashift * ((float)s_mainwidth / 600.0));
//	s_spbt.dispcenter.x = (int)(s_mainwidth * 0.57f) + ((int)(spawidth)+spashift) * 3;
//	s_spbt.dispcenter.y = (int)(30.0f * ((float)s_mainheight / 620.0)) + (int(spawidth * 1.5f));// *2);
//
//
//	ChaVector3 disppos;
//	disppos.x = (float)(s_spbt.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
//	disppos.y = -((float)(s_spbt.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
//	disppos.z = 0.0f;
//	ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
//	if (s_spbt.sprite) {
//		CallF(s_spbt.sprite->SetPos(disppos), return 1);
//		CallF(s_spbt.sprite->SetSize(dispsize), return 1);
//	}
//	else {
//		_ASSERT(0);
//	}
//
//	return 0;
//
//}



int PickSpAxis( POINT srcpos )
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	int startx = s_spaxis[0].dispcenter.x - 25;
	int endx = startx + 50;

	if( (srcpos.x >= startx) && (srcpos.x <= endx) ){
		int spacnt;
		for( spacnt = 0; spacnt < SPAXISNUM; spacnt++ ){
			int starty = s_spaxis[spacnt].dispcenter.y - 25;
			int endy = starty + 50;

			if( (srcpos.y >= starty) && (srcpos.y <= endy) ){
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

int PickSpRigidSW(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	//ret2prev
	int starty0 = s_spret2prev.dispcenter.y - 16;
	int endy0 = starty0 + 32;
	if ((srcpos.y >= starty0) && (srcpos.y <= endy0)) {
		int startx0 = s_spret2prev.dispcenter.x - 16;
		int endx0 = startx0 + 32;
		if ((srcpos.x >= startx0) && (srcpos.x <= endx0)) {
			kind = -2;
		}
	}

	//spguisw
	if (kind == 0) {//カエルボタンを押していないとき
		int starty = s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].dispcenter.y - 14;
		int endy = starty + 28;


		if ((srcpos.y >= starty) && (srcpos.y <= endy)) {
			int spgcnt;
			for (spgcnt = 0; spgcnt < 4; spgcnt++) {
				int startx = s_sprigidsw[spgcnt].dispcenter.x - 70;
				int endx = startx + 124 + 6;

				if ((srcpos.x >= startx) && (srcpos.x <= endx)) {
					switch (spgcnt) {
					case 0:
						kind = 1;
						break;
					case 1:
						kind = 2;
						break;
					case 2:
						kind = 3;
						break;
					case 3:
						kind = 4;
						break;
					default:
						kind = 0;
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
	}
	return kind;
}

int PickSpRetargetSW(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	//ret2prev
	int starty0 = s_spret2prev.dispcenter.y - 16;
	int endy0 = starty0 + 32;
	if ((srcpos.y >= starty0) && (srcpos.y <= endy0)) {
		int startx0 = s_spret2prev.dispcenter.x - 16;
		int endx0 = startx0 + 32;
		if ((srcpos.x >= startx0) && (srcpos.x <= endx0)) {
			kind = -2;
		}
	}

	//spretargetsw
	if (kind == 0) {
		int starty = s_spretargetsw[SPRETARGETSW_RETARGET].dispcenter.y - 14;
		int endy = starty + 28;


		if ((srcpos.y >= starty) && (srcpos.y <= endy)) {
			int sprcnt;
			for (sprcnt = 0; sprcnt < SPRETARGETSWNUM; sprcnt++) {

				int startx = s_spretargetsw[sprcnt].dispcenter.x - 70;
				int endx = startx + 124 + 6;

				if ((srcpos.x >= startx) && (srcpos.x <= endx)) {
					kind = sprcnt + 1;
					return kind;
				}
			}
		}
		//DbgOut( L"pickspaxis : kind %d, mouse (%d, %d), starty %d, endy %d\r\n",
		//	kind, srcpos.x, srcpos.y, starty, endy );
		//int spacnt;
		//for( spacnt = 0; spacnt < 3; spacnt++ ){
		//	DbgOut( L"\tspa %d : startx %d, endx %d\r\n", spacnt, s_spaxis[spacnt].dispcenter.x, s_spaxis[spacnt].dispcenter.x + 32 );
		//}
	}
	return kind;
}

int PickSpIkModeSW(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}

	//spikmodesw
	if (kind == 0) {
		int startx = s_spikmodesw[0].dispcenter.x - 25;
		int endx = startx + 50;


		if ((srcpos.x >= startx) && (srcpos.x <= endx)) {
			int spgcnt;
			for (spgcnt = 0; spgcnt < 3; spgcnt++) {
				int starty = s_spikmodesw[spgcnt].dispcenter.y - 25;
				int endy = starty + 50 + 6;

				if ((srcpos.y >= starty) && (srcpos.y <= endy)) {
					switch (spgcnt) {
					case 0:
						kind = 1;
						break;
					case 1:
						kind = 2;
						break;
					case 2:
						kind = 3;
						break;
					default:
						kind = 1;
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
	}
	return kind;
}

int PickSpRefPosSW(POINT srcpos)
{
	int ispick = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}

	//sprefpos
	int startx = s_sprefpos.dispcenter.x - 25;
	int endx = startx + 50;

	if ((srcpos.x >= startx) && (srcpos.x <= endx)) {
		int starty = s_sprefpos.dispcenter.y - 25;
		int endy = starty + 50 + 6;

		if ((srcpos.y >= starty) && (srcpos.y <= endy)) {
			ispick = 1;
		}
	}

	return ispick;
}


int PickSpGUISW(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	//ret2prev
	int starty0 = s_spret2prev.dispcenter.y - 16;
	int endy0 = starty0 + 32;
	if ((srcpos.y >= starty0) && (srcpos.y <= endy0)) {
		int startx0 = s_spret2prev.dispcenter.x - 16;
		int endx0 = startx0 + 32;
		if ((srcpos.x >= startx0) && (srcpos.x <= endx0)) {
			kind = -2;
		}
	}

	//spguisw
	if (kind == 0) {
		int starty = s_spguisw[SPGUISW_SPRITEFK].dispcenter.y - 14;
		int endy = starty + 28;


		if ((srcpos.y >= starty) && (srcpos.y <= endy)) {
			int spgcnt;
			for (spgcnt = 0; spgcnt < SPGUISWNUM; spgcnt++) {
				//int startx = s_spguisw[spgcnt].dispcenter.x - 70;
				//int endx = startx + 140 + 6;
				int startx = s_spguisw[spgcnt].dispcenter.x - 62;
				int endx = startx + 124 + 6;

				if ((srcpos.x >= startx) && (srcpos.x <= endx)) {
					switch (spgcnt) {
					case 0:
						kind = (SPGUISW_SPRITEFK + 2);
						break;
					case 1:
						kind = (SPGUISW_LEFT + 2);
						break;
					case 2:
						kind = (SPGUISW_LEFT2ND + 2);
						break;
					case 3:
						kind = (SPGUISW_BULLETPHYSICS + 2);
						break;
					case 4:
						kind = (SPGUISW_PHYSICSIK + 2);
						break;
					default:
						kind = 1;
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
	}
	return kind;
}

int PickSpCam(POINT srcpos)
{
	int kind = 0;

	//if (g_previewFlag == 5){
	//	return 0;
	//}


	int starty = s_spcam[SPR_CAM_I].dispcenter.y - 25;
	int endy = starty + 50;

	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
		int spacnt;
		for (spacnt = 0; spacnt < SPR_CAM_MAX; spacnt++){
			int startx = s_spcam[spacnt].dispcenter.x - 25;
			int endx = startx + 50;

			if ((srcpos.x >= startx) && (srcpos.x <= endx)){
				switch (spacnt){
				case 0:
					kind = PICK_CAMROT;
					break;
				case 1:
					kind = PICK_CAMMOVE;
					break;
				case 2:
					kind = PICK_CAMDIST;
					break;
				default:
					kind = 0;
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


	if (s_sprig[SPRIG_INACTIVE].sprite == 0){
		return 0;
	}

	int starty = s_sprig[SPRIG_INACTIVE].dispcenter.y - 25;
	int endy = starty + 50;

	//SPRIG_INACTIVEとSPRIG_ACTIVEは同じ位置なので当たり判定は１回で良い
	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
		int startx = s_sprig[SPRIG_INACTIVE].dispcenter.x - 25;
		int endx = startx + 50;

		if ((srcpos.x >= startx) && (srcpos.x <= endx)){
			pickflag = 1;
		}
	}

	return pickflag;
}


//int PickSpBt(POINT srcpos)
//{
//	int pickflag = 0;
//
//	//if (g_previewFlag == 5){
//	//	return 0;
//	//}
//
//
//	if (s_spbt.sprite == 0){
//		return 0;
//	}
//
//	int starty = s_spbt.dispcenter.y - 16;
//	int endy = starty + 32;
//
//	if ((srcpos.y >= starty) && (srcpos.y <= endy)){
//		int startx = s_spbt.dispcenter.x - 16;
//		int endx = startx + 32;
//
//		if ((srcpos.x >= startx) && (srcpos.x <= endx)){
//			pickflag = 1;
//		}
//	}
//
//	return pickflag;
//}

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
	}else if( (s_ikkind == 1) || (s_ikkind == 2) ){
		s_select->SetDispFlag( "ringX", 0 );
		s_select->SetDispFlag( "ringY", 0 );
		s_select->SetDispFlag( "ringZ", 0 );
	}
////////
	PICKINFO pickinfo;
	ZeroMemory( &pickinfo, sizeof( PICKINFO ) );
	POINT ptCursor;
	GetCursorPos( &ptCursor );
	::ScreenToClient( s_3dwnd, &ptCursor );
	pickinfo.clickpos = ptCursor;
	pickinfo.mousepos = ptCursor;
	pickinfo.mousebefpos = ptCursor;
	pickinfo.diffmouse = ChaVector2( 0.0f, 0.0f );

	pickinfo.winx = (int)DXUTGetWindowWidth();
	pickinfo.winy = (int)DXUTGetWindowHeight();
	pickinfo.pickrange = PICKRANGE;
	pickinfo.buttonflag = 0;

	//pickinfo.pickobjno = s_curboneno;
	pickinfo.pickobjno = -1;

	int spakind = 0;
	int spckind = 0;
	if (s_spguisw[SPGUISW_SPRITEFK].state) {
		spakind = PickSpAxis(ptCursor);
		spckind = PickSpCam(ptCursor);
	}
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

	//float hia = 0.3f;
	float hia = 0.7f;
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
	if (g_previewFlag != 0) {
		return 0;
	}

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

int CreateMotionBrush(double srcstart, double srcend, bool onrefreshflag)
{
	//int keynum;
	double startframe, endframe;
	//s_editrange.GetRange(&keynum, &startframe, &endframe);
	startframe = srcstart;
	endframe = srcend;


	if (g_motionbrush_value) {
		free(g_motionbrush_value);
		g_motionbrush_value = 0;
	}

	int frameleng = (int)s_model->GetCurMotInfo()->frameleng;

	g_motionbrush_startframe = startframe;
	g_motionbrush_endframe = endframe;
	g_motionbrush_numframe = endframe - startframe + 1;
	g_motionbrush_frameleng = frameleng;

	g_motionbrush_applyframe = startframe + (endframe - startframe) * g_applyrate * 0.01;
	if ((g_motionbrush_applyframe < 0) || (g_motionbrush_applyframe > endframe)) {
		_ASSERT(0);
		return -1;
	}

	g_motionbrush_value = (float*)malloc(sizeof(float) * (g_motionbrush_frameleng + 1));
	if (!g_motionbrush_value) {
		_ASSERT(0);
		return -1;
	}
	::memset(g_motionbrush_value, 0, sizeof(float) * (g_motionbrush_frameleng + 1));

	float* tempvalue;
	tempvalue = (float*)malloc(sizeof(float) * (g_motionbrush_frameleng + 1));
	if (!tempvalue) {
		_ASSERT(0);
		return -1;
	}
	::memset(tempvalue, 0, sizeof(float) * (g_motionbrush_frameleng + 1));


	int ret = 0;

	if (s_plugin && (g_motionbrush_method >= 0) && (g_motionbrush_method <= MAXPLUGIN)) {
		s_onselectplugin = 1;

		int pluginno;
		for (pluginno = 0; pluginno < MAXPLUGIN; pluginno++) {
			if ((s_plugin + pluginno)->menuid == g_motionbrush_method) {
				//DbgOut( "viewer : OnSelectPlugin : pluginno %d, menuid %d\r\n", pluginno, menuid );
				ret = (s_plugin + pluginno)->CreateMotionBrush(g_motionbrush_startframe, g_motionbrush_endframe, g_motionbrush_applyframe, g_motionbrush_frameleng, g_brushrepeats, g_brushmirrorUflag, g_brushmirrorVflag, g_ifmirrorVDiv2flag, tempvalue);
				_ASSERT(!ret);
			}
		}

		//if (g_motionbrush_applyframe == g_motionbrush_startframe) {
		//	_ASSERT(0);
		//}

		int cpframe;
		for (cpframe = 0; cpframe < (int)g_motionbrush_frameleng; cpframe++) {
			float cpvalue;

			if ((cpframe >= (int)g_motionbrush_startframe) && (cpframe <= (int)g_motionbrush_endframe)) {
				cpvalue = *(tempvalue + cpframe);
				cpvalue = min(1.0f, cpvalue);
				cpvalue = max(-1.0f, cpvalue);
			}
			else {
				cpvalue = 0.0f;
			}

			*(g_motionbrush_value + cpframe) = cpvalue;
		}



		s_onselectplugin = 0;
	}


	if (tempvalue) {
		free(tempvalue);
		tempvalue = 0;
	}

	if (onrefreshflag == false) {//Refresh関数以外から呼び出したとき

		if(s_owpTimeline)
			s_owpTimeline->setMaxTime(frameleng);//!!!!!!!!!!!!!!!!!!!!!
		if(s_owpLTimeline)
			s_owpLTimeline->setMaxTime(frameleng);//!!!!!!!!!!!!!!!!!!!!!

		if (s_owpTimeline)
			s_owpTimeline->setCurrentTime(g_motionbrush_applyframe, false);//!!!!!!!!!!!!!!!!!!!!!
		if (s_owpLTimeline)
			s_owpLTimeline->setCurrentTime(g_motionbrush_applyframe, false);//!!!!!!!!!!!!!!!!!!!!!
		if(s_owpEulerGraph)
			s_owpEulerGraph->setCurrentTime(g_motionbrush_applyframe, false);//!!!!!!!!!!!!!!!!!!!!!

		UpdateEditedEuler();
	}

	return 0;
}


int SetTimelineMark()
{
	if( !s_model || !s_owpTimeline || !s_owpLTimeline ){
		_ASSERT( 0 );
		return 0;
	}

	if (g_previewFlag != 0) {
		return 0;
	}


	s_owpTimeline->setRewriteOnChangeFlag(false);		//再描画要求を再開


//	s_owpTimeline->deleteKey();

	//map<int,CBone*>::iterator itrbone;
	//for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
	//	CBone* curbone = itrbone->second;
	//	if( curbone ){
	//		int curlineno = s_boneno2lineno[ curbone->GetBoneNo() ];
	//		if( curlineno >= 0 ){
	//			double curframe;
	//			for( curframe = 0.0; curframe < s_model->GetCurMotInfo()->frameleng; curframe += 1.0 ){
	//				KeyInfo chkki = s_owpTimeline->ExistKey( curlineno, curframe );
	//				if( chkki.lineIndex >= 0 ){
	//					s_owpTimeline->deleteKey( curlineno, curframe );
	//				}
	//			}
	//		}
	//	}
	//}

	//map<int, CBone*>::iterator itrbone;
	//for( itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++ ){
	//	CBone* curbone = itrbone->second;
	//	if( curbone ){
	//		int curlineno = s_boneno2lineno[ curbone->GetBoneNo() ];
	//		if( curlineno >= 0 ){
	//			map<int, map<double, int>>::iterator itrcur;
	//			itrcur = curbone->FindMotMarkOfMap( s_model->GetCurMotInfo()->motid );
	//			if( itrcur != curbone->GetMotMarkOfMapBegin() ){
	//				map<double, int>::iterator itrmark;
	//				for( itrmark = itrcur->second.begin(); itrmark != itrcur->second.end(); itrmark++ ){
	//					double curframe = itrmark->first;
	//					s_owpTimeline->newKey( curbone->GetWBoneName(), curframe, 0 );
	//				}
	//			}
	//		}
	//	}
	//}

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
	if (g_previewFlag != 0){
		return 0;
	}

	if( (curboneno >= 0) && s_model && s_owpTimeline && s_owpLTimeline ){
		CBone* curbone = s_model->GetBoneByID( curboneno );
		if( curbone ){
			CBone* parentbone = curbone->GetParent();
			if (parentbone) {
				curbone = parentbone;
			}

			int curlineno = s_boneno2lineno[ curboneno ];
			if( curlineno >= 0 ){
				s_owpLTimeline->deleteLine( 2 );

				WCHAR markname[256] = {0L};
				//swprintf_s( markname, 256, L"Mark:%s", curbone->GetWBoneName() );
				swprintf_s(markname, 256, L"%s", curbone->GetWBoneName());
				s_owpLTimeline->newLine(0, 0, markname, RGB(168, 129, 129));

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
	s_owpLTimeline->setCurrentTime(0.0, true);

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel && curmodel->GetCurMotInfo() ){
			curmodel->SetMotionFrame(0.0);
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
	//ofn1.hwndOwner = s_3dwnd;
	ofn1.hwndOwner = s_3dwnd;
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
	ofn1.lpstrTitle = L"GetFileNameDlg";
	//ofn1.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn1.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn1.nFileOffset = 0;
	ofn1.nFileExtension = 0;
	ofn1.lpstrDefExt = L"fbx";
	ofn1.lCustData = NULL;
	ofn1.lpfnHook = NULL;
	ofn1.lpTemplateName = NULL;


	{
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
		HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
			WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
		InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

		if (GetOpenFileNameW(&ofn1) != IDOK) {
			return 0;
		}

		InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
		UnhookWinEvent(hhook);
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
	}


	
	char fbxpath[MAX_PATH] = {0};
	WideCharToMultiByte( CP_UTF8, 0, filename, -1, fbxpath, MAX_PATH, NULL, NULL );	


	{
		s_owpLTimeline->setCurrentTime(0.0, true);
		s_model->SetMotionFrame(0.0);
		s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);

		//ここでAxisMatXの初期化
		s_model->CreateBtObject(1);
		s_model->CalcBtAxismat(2);//2
		s_model->SetInitAxisMatX(1);
	}


	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	char fbxdate[MAX_PATH] = { 0L };
	sprintf_s(fbxdate, MAX_PATH, "CommentForEGP_%04u%02u%02u%02u%02u%02u",
		localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
	//CallF( WriteFBXFile( s_model, fbxpath, s_dummytri, mb, g_tmpmqomult, s_fbxbunki ), return 1 );
	CallF( WriteFBXFile( s_psdk, s_model, fbxpath, fbxdate ), return 1 );

	if (s_model->GetOldAxisFlagAtLoading() == 0){
		WCHAR lmtname[MAX_PATH] = { 0L };
		swprintf_s(lmtname, MAX_PATH, L"%s.lmt", filename);
		CLmtFile lmtfile;
		lmtfile.WriteLmtFile(lmtname, s_model, fbxdate);

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
	s_owpLTimeline->setCurrentTime(0.0, true);

	vector<MODELELEM>::iterator itrmodel;
	for( itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++ ){
		CModel* curmodel = itrmodel->modelptr;
		if( curmodel && curmodel->GetCurMotInfo() ){
			curmodel->SetMotionFrame(0.0);
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
	ofn1.hwndOwner = s_3dwnd;
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
	ofn1.lpstrTitle = L"GetFileNameDlg";
	//ofn1.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn1.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn1.nFileOffset = 0;
	ofn1.nFileExtension = 0;
	ofn1.lpstrDefExt = L"bnt";
	ofn1.lCustData = NULL;
	ofn1.lpfnHook = NULL;
	ofn1.lpTemplateName = NULL;


	{
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
		HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
			WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
		InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

		if (GetOpenFileNameW(&ofn1) != IDOK) {
			return 0;
		}

		InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
		UnhookWinEvent(hhook);
		s_getfilenamehwnd = 0;
		s_getfilenametreeview = 0;
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

	if (!s_editrangehistory) {
		return 0;
	}

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



int DispAngleLimitDlg()
{
	s_underanglelimithscroll = 0;

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
		::DSMessageBox(s_3dwnd, L"Work Only After Setting Of Axis.\nRetry after Saving FBX file.", L"error!!!", MB_OK);
		return 0;
	}

	s_dseullimitctrls.clear();

	s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
	int setcheckflag = 1;
	Bone2AngleLimit(setcheckflag);

	/*
	int dlgret;
	dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANGLELIMITDLG),
		s_3dwnd, (DLGPROC)AngleLimitDlgProc);
	if (dlgret != IDOK){
		return 0;
	}
	*/
	//s_anglelimitdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANGLELIMITDLG), s_3dwnd, (DLGPROC)AngleLimitDlgProc);
	s_anglelimitdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANGLELIMITDLG), s_mainhwnd, (DLGPROC)AngleLimitDlgProc);
	if (!s_anglelimitdlg){
		_ASSERT(0);
		return 1;
	}

	SetParent(s_anglelimitdlg, s_mainhwnd);
	SetWindowPos(
		s_anglelimitdlg,
		HWND_TOP,
		s_timelinewidth + s_mainwidth + 16,
		s_sidemenuheight,
		s_sidewidth,
		s_sideheight,
		SWP_SHOWWINDOW
	);

	//s_dseullimitctrls.push_back(IDD_ANGLELIMITDLG);
	s_dseullimitctrls.push_back(IDC_BONEAXIS);
	s_dseullimitctrls.push_back(IDC_SLXL);
	s_dseullimitctrls.push_back(IDC_SLXU);
	s_dseullimitctrls.push_back(IDC_SLYL);
	s_dseullimitctrls.push_back(IDC_SLYU);
	s_dseullimitctrls.push_back(IDC_SLZL);
	s_dseullimitctrls.push_back(IDC_SLZU);
	s_dseullimitctrls.push_back(IDOK);


	//::MoveWindow(s_anglelimitdlg, 1200, 32, 450, 858, TRUE);
	//s_rigidWnd->setSize(WindowSize(450, 858));//880
	//s_rigidWnd->setPos(WindowPos(1200, 32));

	ShowWindow(s_anglelimitdlg, SW_SHOW);
	UpdateWindow(s_anglelimitdlg);

	//AngleLimit2Bone();


	return 0;
}

int Bone2AngleLimit(int setcheckflag)
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

	ANGLELIMIT saveal;
	saveal = s_anglelimit;


	CBone* curbone;
	curbone = s_model->GetBoneByID(s_curboneno);

	//オイラーグラフの表示と合わせるために選択ジョイントの１階層親のジョイントを扱う //2021/11/17
	if (curbone) {
		if (curbone->GetParent()) {
			s_anglelimitbone = curbone->GetParent();
		}
		else {
			s_anglelimitbone = curbone;
		}
	}
	else {
		s_anglelimitbone = 0;
	}

	if (s_anglelimitbone){
		MOTINFO* curmi;
		curmi = s_model->GetCurMotInfo();
		if (curmi) {
			s_anglelimit = s_anglelimitbone->GetAngleLimit(1);


			s_anglelimit.chkeul[AXIS_X] = min(s_anglelimit.upper[AXIS_X], s_anglelimit.chkeul[AXIS_X]);
			s_anglelimit.chkeul[AXIS_Y] = min(s_anglelimit.upper[AXIS_Y], s_anglelimit.chkeul[AXIS_Y]);
			s_anglelimit.chkeul[AXIS_Z] = min(s_anglelimit.upper[AXIS_Z], s_anglelimit.chkeul[AXIS_Z]);

			s_anglelimit.chkeul[AXIS_X] = max(s_anglelimit.lower[AXIS_X], s_anglelimit.chkeul[AXIS_X]);
			s_anglelimit.chkeul[AXIS_Y] = max(s_anglelimit.lower[AXIS_Y], s_anglelimit.chkeul[AXIS_Y]);
			s_anglelimit.chkeul[AXIS_Z] = max(s_anglelimit.lower[AXIS_Z], s_anglelimit.chkeul[AXIS_Z]);
		}
	}
	else{
		_ASSERT(0);
		InitAngleLimit(&s_anglelimit);
	}

	//setcheckflag == 0のときにはチェックボックスの状態を変えずに復元する
	if (setcheckflag == 0) {
		s_anglelimit.applyeul[AXIS_X] = saveal.applyeul[AXIS_X];
		s_anglelimit.applyeul[AXIS_Y] = saveal.applyeul[AXIS_Y];
		s_anglelimit.applyeul[AXIS_Z] = saveal.applyeul[AXIS_Z];

		s_anglelimit.via180flag[AXIS_X] = saveal.via180flag[AXIS_X];
		s_anglelimit.via180flag[AXIS_Y] = saveal.via180flag[AXIS_Y];
		s_anglelimit.via180flag[AXIS_Z] = saveal.via180flag[AXIS_Z];
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

	//if (g_previewFlag == 0) {
		if (s_anglelimitbone) {
			MOTINFO* curmi = s_model->GetCurMotInfo();
			if (curmi) {
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
				////cureul = curbone->CalcFBXEul(curmi->motid, (double)curtime, &befeul);
				////befeul = cureul;//!!!!!!!
				//cureul = s_anglelimitbone->CalcLocalEulXYZ(s_anglelimit.boneaxiskind, curmi->motid, curmi->curframe, BEFEUL_BEFFRAME);
				cureul = s_anglelimitbone->GetLocalEul(curmi->motid, (double)((int)(curmi->curframe + 0.1)));

				////cureul = s_anglelimitbone->CalcLocalEulXYZ(g_boneaxis, curmi->motid, curmi->curframe, BEFEUL_BEFFRAME);

				//cureul.x = s_anglelimit.chkeul[AXIS_X];
				//cureul.y = s_anglelimit.chkeul[AXIS_Y];
				//cureul.z = s_anglelimit.chkeul[AXIS_Z];

				neweul = cureul;
				if (s_anglelimit.applyeul[AXIS_X]) {
					neweul.x = s_anglelimit.chkeul[AXIS_X];
				}
				if (s_anglelimit.applyeul[AXIS_Y]) {
					neweul.y = s_anglelimit.chkeul[AXIS_Y];
				}
				if (s_anglelimit.applyeul[AXIS_Z]) {
					neweul.z = s_anglelimit.chkeul[AXIS_Z];
				}

				//s_anglelimit.chkeul[AXIS_X] = neweul.x;
				//s_anglelimit.chkeul[AXIS_Y] = neweul.y;
				//s_anglelimit.chkeul[AXIS_Z] = neweul.z;

				int ismovable = s_anglelimitbone->ChkMovableEul(neweul);
				if (ismovable == 1) {
					//int inittraflag = 0;
					//int setchildflag = 1;
					//int initscaleflag = 0;
					//s_anglelimitbone->SetWorldMatFromEul(inittraflag, setchildflag, neweul, curmi->motid, curmi->curframe, initscaleflag);
					s_anglelimit.chkeul[AXIS_X] = neweul.x;
					s_anglelimit.chkeul[AXIS_Y] = neweul.y;
					s_anglelimit.chkeul[AXIS_Z] = neweul.z;
				}
				else {
					s_anglelimit.chkeul[AXIS_X] = cureul.x;
					s_anglelimit.chkeul[AXIS_Y] = cureul.y;
					s_anglelimit.chkeul[AXIS_Z] = cureul.z;
				}

				s_anglelimitbone->SetAngleLimit(s_anglelimit);
			}

		}
			
		//}
	//}

	return 0;
}

int InitAngleLimitSlider(HWND hDlgWnd, int slresid, int txtresid, int srclimit)
{
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)-180);
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)180);

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_CLEARTICS, 0, 0);
	int tickcnt;
	for (tickcnt = 0; tickcnt <= 36; tickcnt++) {
		int tickval = (-180 + 10 * tickcnt);
		SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETTIC, 1, (LPARAM)(LONG)tickval);
	}

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)srclimit);

	WCHAR strval[256] = { 0L };
	swprintf_s(strval, 256, L"%d", srclimit);
	SetDlgItemText(hDlgWnd, txtresid, (LPCWSTR)strval);

	return 0;
}


int InitAngleLimitSliderFloat(HWND hDlgWnd, int slresid, int txtresid, float srclimit)
{
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)-180);
	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)180);

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_CLEARTICS, 0, 0);
	int tickcnt;
	for (tickcnt = 0; tickcnt <= 36; tickcnt++){
		int tickval = (-180 + 10 * tickcnt);
		SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETTIC, 1, (LPARAM)(LONG)tickval);
	}

	SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)((int)(srclimit + 0.1f)));

	WCHAR strval[256] = {0L};
	swprintf_s(strval, 256, L"%.1f", srclimit);
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

int GetAngleLimitSliderValFloat(HWND hDlgWnd, int slresid, int txtresid, float* dstptr)
{
	int curval = (int)SendMessage(GetDlgItem(hDlgWnd, slresid), TBM_GETPOS, 0, 0);
	float curvalfloat = (float)curval;

	*dstptr = curval;
	WCHAR strval[256];
	swprintf_s(strval, 256, L"%.1f", curvalfloat);
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
		EnableWindow(GetDlgItem(hDlgWnd, IDC_BONEAXIS), FALSE);//固定値


		InitAngleLimitSlider(hDlgWnd, IDC_SLXL, IDC_XLVAL, s_anglelimit.lower[AXIS_X]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLXU, IDC_XUVAL, s_anglelimit.upper[AXIS_X]);
		InitAngleLimitSliderFloat(hDlgWnd, IDC_SLCHKX, IDC_CHKXVAL, s_anglelimit.chkeul[AXIS_X]);

		InitAngleLimitSlider(hDlgWnd, IDC_SLYL, IDC_YLVAL, s_anglelimit.lower[AXIS_Y]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLYU, IDC_YUVAL, s_anglelimit.upper[AXIS_Y]);
		InitAngleLimitSliderFloat(hDlgWnd, IDC_SLCHKY, IDC_CHKYVAL, s_anglelimit.chkeul[AXIS_Y]);

		InitAngleLimitSlider(hDlgWnd, IDC_SLZL, IDC_ZLVAL, s_anglelimit.lower[AXIS_Z]);
		InitAngleLimitSlider(hDlgWnd, IDC_SLZU, IDC_ZUVAL, s_anglelimit.upper[AXIS_Z]);
		InitAngleLimitSliderFloat(hDlgWnd, IDC_SLCHKZ, IDC_CHKZVAL, s_anglelimit.chkeul[AXIS_Z]);

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


		if (s_anglelimit.applyeul[AXIS_X] == 1) {
			CheckDlgButton(hDlgWnd, IDC_APPLYX, BST_CHECKED);
		}
		else {
			CheckDlgButton(hDlgWnd, IDC_APPLYX, BST_UNCHECKED);
		}
		if (s_anglelimit.applyeul[AXIS_Y] == 1) {
			CheckDlgButton(hDlgWnd, IDC_APPLYY, BST_CHECKED);
		}
		else {
			CheckDlgButton(hDlgWnd, IDC_APPLYY, BST_UNCHECKED);
		}
		if (s_anglelimit.applyeul[AXIS_Z] == 1) {
			CheckDlgButton(hDlgWnd, IDC_APPLYZ, BST_CHECKED);
		}
		else {
			CheckDlgButton(hDlgWnd, IDC_APPLYZ, BST_UNCHECKED);
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
	case WM_LBUTTONDOWN:
		s_underanglelimithscroll = 1;
		break;
	case WM_LBUTTONUP:
		s_underanglelimithscroll = 0;
		break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_BONEAXIS:

			//lmtファイルによる固定値

			//{
			//	int combono = (int)SendMessage(GetDlgItem(hDlgWnd, IDC_BONEAXIS), CB_GETCURSEL, 0, 0);
			//	if ((combono >= BONEAXIS_CURRENT) && (combono <= BONEAXIS_GLOBAL)){
			//		s_anglelimit.boneaxiskind = combono;
			//	}
			//}
			break;
		case IDC_APPLYX:
			if (IsDlgButtonChecked(hDlgWnd, IDC_APPLYX) == BST_CHECKED) {
				s_anglelimit.applyeul[AXIS_X] = true;
			}
			else {
				s_anglelimit.applyeul[AXIS_X] = false;
			}
			AngleLimit2Bone();
			break;
		case IDC_APPLYY:
			if (IsDlgButtonChecked(hDlgWnd, IDC_APPLYY) == BST_CHECKED) {
				s_anglelimit.applyeul[AXIS_Y] = true;
			}
			else {
				s_anglelimit.applyeul[AXIS_Y] = false;
			}
			AngleLimit2Bone();
			break;
		case IDC_APPLYZ:
			if (IsDlgButtonChecked(hDlgWnd, IDC_APPLYZ) == BST_CHECKED) {
				s_anglelimit.applyeul[AXIS_Z] = true;
			}
			else {
				s_anglelimit.applyeul[AXIS_Z] = false;
			}
			AngleLimit2Bone();
			break;

		case IDC_CHECKX:
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKX) == BST_CHECKED) {
				s_anglelimit.via180flag[AXIS_X] = 1;
			}
			else {
				s_anglelimit.via180flag[AXIS_X] = 0;
			}
			AngleLimit2Bone();
			break;
		case IDC_CHECKY:
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKY) == BST_CHECKED) {
				s_anglelimit.via180flag[AXIS_Y] = 1;
			}
			else {
				s_anglelimit.via180flag[AXIS_Y] = 0;
			}
			AngleLimit2Bone();
			break;
		case IDC_CHECKZ:
			if (IsDlgButtonChecked(hDlgWnd, IDC_CHECKZ) == BST_CHECKED) {
				s_anglelimit.via180flag[AXIS_Z] = 1;
			}
			else {
				s_anglelimit.via180flag[AXIS_Z] = 0;
			}
			AngleLimit2Bone();
			break;

		case TB_THUMBTRACK:
			s_underanglelimithscroll = 1;
			break;
		case TB_THUMBPOSITION:
			s_underanglelimithscroll = 0;
			break;
		case IDC_BUTTON1:
		{
			//CopyToSymBone
			if (s_anglelimitbone && s_anglelimitdlg) {
				int symboneno = 0;
				int existflag = 0;
				s_model->GetSymBoneNo(s_anglelimitbone->GetBoneNo(), &symboneno, &existflag);
				if (symboneno >= 0) {
					CBone* symbone = s_model->GetBoneByID(symboneno);
					if (symbone) {
						ANGLELIMIT symanglelimit = s_anglelimit;
						//symanglelimit.lower[1] *= -1;
						symanglelimit.lower[2] *= -1;
						//symanglelimit.upper[1] *= -1;
						symanglelimit.upper[2] *= -1;

						MOTINFO* curmi;
						curmi = s_model->GetCurMotInfo();
						if (curmi) {
							symbone->SetAngleLimit(symanglelimit);

							ClearLimitedWM();//2022/01/11
						}
						
					}
				}
			}
		}
			break;
		case IDC_BUTTON2:
		{
			//CopyFromSymBone
			if (s_anglelimitbone && s_anglelimitdlg) {
				int symboneno = 0;
				int existflag = 0;
				s_model->GetSymBoneNo(s_anglelimitbone->GetBoneNo(), &symboneno, &existflag);
				if (symboneno >= 0) {
					CBone* symbone = s_model->GetBoneByID(symboneno);
					if (symbone) {
						MOTINFO* curmi;
						curmi = s_model->GetCurMotInfo();
						if (curmi) {
							ANGLELIMIT anglelimit = symbone->GetAngleLimit(0);
							ANGLELIMIT symanglelimit = anglelimit;
							//symanglelimit.lower[1] *= -1;
							symanglelimit.lower[2] *= -1;
							//symanglelimit.upper[1] *= -1;
							symanglelimit.upper[2] *= -1;

							//s_anglelimitbone->SetAngleLimit(symanglelimit, curmi->motid, curmi->curframe);
							s_anglelimit = symanglelimit;
							AngleLimit2Bone();
							AngleLimit2Dlg(s_anglelimitdlg);

							ClearLimitedWM();//2022/01/11
						}
						
					}
				}
			}
		}
			break;
		case IDC_BUTTON3:
		{
			//copy button
			if (s_anglelimitbone && s_anglelimitdlg) {
				s_anglelimitcopy = s_anglelimit;
			}
		}
			break;
		case IDC_BUTTON4:
		{
			//paste button
			if (s_anglelimitbone && s_anglelimitdlg) {
				s_anglelimit = s_anglelimitcopy;
				AngleLimit2Bone();

				ClearLimitedWM();//2022/01/11

				AngleLimit2Dlg(s_anglelimitdlg);
			}
		}
			break;
		case IDC_RESETLIM:
		{
			if (s_model && s_anglelimitdlg) {
				s_model->ResetAngleLimit(180);

				ClearLimitedWM();//2022/01/11

				Bone2AngleLimit(0);
				AngleLimit2Dlg(s_anglelimitdlg);
				UpdateWindow(s_anglelimitdlg);
			}
		}
			break;
		case IDC_RESET0:
		{
			if (s_model && s_anglelimitdlg) {
				s_model->ResetAngleLimit(0);

				ClearLimitedWM();//2022/01/11

				Bone2AngleLimit(0);
				AngleLimit2Dlg(s_anglelimitdlg);
				UpdateWindow(s_anglelimitdlg);
			}
		}
			break;
		case IDC_REPLACE180TO170:
		{
			if (s_model && s_anglelimitdlg) {
				s_model->AngleLimitReplace180to170();

				ClearLimitedWM();//2022/01/11

				Bone2AngleLimit(0);
				AngleLimit2Dlg(s_anglelimitdlg);
				UpdateWindow(s_anglelimitdlg);
			}

		}
			break;
		case IDC_LIMITFROMMOTION:
		{
			if (s_model && s_anglelimitdlg) {
				MOTINFO* curmi;
				curmi = s_model->GetCurMotInfo();
				if (curmi) {

					s_savelimitdegflag = g_limitdegflag;
					g_limitdegflag = false;
					if (s_LimitDegCheckBox) {
						s_LimitDegCheckBox->SetChecked(g_limitdegflag);
					}


					//UINT checkadditive;
					//checkadditive = IsDlgButtonChecked(s_anglelimitdlg, IDC_ADDITIVE);
					//if (checkadditive != BST_CHECKED) {
					//	SendMessage(s_anglelimitdlg, WM_COMMAND, IDC_RESETLIM, 0);
					//}

					int curmotid;
					double curmotleng;
					curmotid = curmi->motid;
					curmotleng = curmi->frameleng;

					double curframe;
					for (curframe = 1.0; curframe < curmotleng; curframe += 1.0) {
						s_model->SetMotionFrame(curframe);
						s_model->UpdateMatrix(&(s_model->GetWorldMat()), &s_matVP);
						s_model->AdditiveCurrentToAngleLimit();
					}


					g_limitdegflag = s_savelimitdegflag;
					if (s_LimitDegCheckBox) {
						s_LimitDegCheckBox->SetChecked(g_limitdegflag);
					}

					ClearLimitedWM();//2022/01/11

					s_model->SetMotionFrame(1.0);
					s_model->UpdateMatrix(&(s_model->GetWorldMat()), &s_matVP);
					Bone2AngleLimit(0);
					AngleLimit2Dlg(s_anglelimitdlg);
					UpdateWindow(s_anglelimitdlg);

				}
			}
		}
			break;
		//case IDOK:
			//break;
		case IDCANCEL:
			//EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			return FALSE;
		}
		break;
	case WM_CLOSE:
		if (s_anglelimitdlg){
			s_underanglelimithscroll = 0;
			DestroyWindow(s_anglelimitdlg);
			s_anglelimitdlg = 0;
		}
		break;
	case WM_HSCROLL:
		{
			HWND ctrlwnd;
			ctrlwnd = (HWND)lp;

			if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLXL)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLXL, IDC_XLVAL, &(s_anglelimit.lower[AXIS_X]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLXU)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLXU, IDC_XUVAL, &(s_anglelimit.upper[AXIS_X]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLYL)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLYL, IDC_YLVAL, &(s_anglelimit.lower[AXIS_Y]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLYU)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLYU, IDC_YUVAL, &(s_anglelimit.upper[AXIS_Y]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLZL)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLZL, IDC_ZLVAL, &(s_anglelimit.lower[AXIS_Z]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLZU)){
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderVal(hDlgWnd, IDC_SLZU, IDC_ZUVAL, &(s_anglelimit.upper[AXIS_Z]));
			}

			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLCHKX)) {
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderValFloat(hDlgWnd, IDC_SLCHKX, IDC_CHKXVAL, &(s_anglelimit.chkeul[AXIS_X]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLCHKY)) {
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderValFloat(hDlgWnd, IDC_SLCHKY, IDC_CHKYVAL, &(s_anglelimit.chkeul[AXIS_Y]));
			}
			else if (ctrlwnd == GetDlgItem(hDlgWnd, IDC_SLCHKZ)) {
				s_underanglelimithscroll = 1;
				GetAngleLimitSliderValFloat(hDlgWnd, IDC_SLCHKZ, IDC_CHKZVAL, &(s_anglelimit.chkeul[AXIS_Z]));
			}

			AngleLimit2Bone();

			ClearLimitedWM();//2022/01/11

			if (s_model) {
				if (s_owpLTimeline) {
					double curframe = s_owpLTimeline->getCurrentTime();
					s_model->SetMotionFrame(curframe);
					//s_model->CalcRigidElem();
					s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
				}
			}

			//読み込みなおし：lowerとupperは大小関係で入れ替わることがあるため適用後読み込みなおす。
			int setcheckflag = 1;
			Bone2AngleLimit(setcheckflag);
			//AngleLimit2Dlg(s_anglelimitdlg);
			ChangeCurrentBone();
			//EndDialog(hDlgWnd, IDOK);

			//UpdateEditedEuler();//selectした範囲のみ

			refreshEulerGraph();//モーション全体
			//s_tum.UpdateEditedEuler(refreshEulerGraph);//非ブロッキング


			s_underanglelimithscroll = 0;

			break;
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
		::DSMessageBox(s_3dwnd, L"Work Only After Setting Of Axis.\nRetry After Saving FBX file.", L"error", MB_OK);
		return 0;
	}


	//s_rotaxisdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ROTAXISDLG), s_3dwnd, (DLGPROC)RotAxisDlgProc);
	s_rotaxisdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ROTAXISDLG), s_mainhwnd, (DLGPROC)RotAxisDlgProc);
	if (!s_rotaxisdlg){
		_ASSERT(0);
		return 1;
	}

	SetParent(s_rotaxisdlg, s_mainhwnd);
	SetWindowPos(
		s_rotaxisdlg,
		HWND_TOP,
		s_timelinewidth + s_mainwidth + 16,
		s_sidemenuheight,
		s_sidewidth,
		s_sideheight,
		SWP_SHOWWINDOW
	);


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
	strdeg[256 - 1] = 0L;
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
			//::DSMessageBox(hDlgWnd, strmes, L"Check!!!", MB_OK);
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
		
		s_rotzisdlghwnd = hDlgWnd;

		return FALSE;
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			s_rotzisdlghwnd = 0;

			//EndDialog(hDlgWnd, IDOK);
			if (s_rotaxisdlg){
				DestroyWindow(s_rotaxisdlg);
				s_rotaxisdlg = 0;
			}
			break;
		case IDCANCEL:
			s_rotzisdlghwnd = 0;

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
	static CModel* s_befmodel = 0;
	static CBone* s_befbone = 0;

	if (!s_model) {
		return 0;
	}
	if (s_model->GetLoadedFlag() == false) {//2021/08/23
		return 0;
	}
	if (s_nowloading == true) {
		return 0;
	}


	//if (g_retargetbatchflag == 0) {
	if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {

		if (s_model) {
			//CDXUTComboBox* pComboBox;
			//pComboBox = g_SampleUI.GetComboBox(IDC_COMBO_BONE);
			//CBone* pBone;
			//pBone = s_model->GetBoneByID(s_curboneno);
			//if (pBone) {
			//	pComboBox->SetSelectedByData(ULongToPtr(s_curboneno));
			//}

			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone) {
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
			if (s_anglelimitdlg) {
				int setcheckflag = 1;
				//s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);//commentout
				Bone2AngleLimit(setcheckflag);
				AngleLimit2Dlg(s_anglelimitdlg);
			}

			//if (s_befbone != curbone) {
			//	refreshEulerGraph();
			//}
			if ((s_befbone != curbone) || (s_befmodel != s_model)) {
				//if (s_owpTimeline) {
					//refreshTimeline(*s_owpTimeline);
				
				refreshEulerGraph();
				//s_tum.UpdateEditedEuler(refreshEulerGraph);//非ブロッキング


				//}
			}


			s_befbone = curbone;
			s_befmodel = s_model;
		}
		else {
			s_befbone = 0;
			s_befmodel = s_model;
		}
	}

	return 0;
}

int OnFrameKeyboard()
{
	MoveMemory(g_savekeybuf, g_keybuf, sizeof(BYTE) * 256);

	if (GetKeyboardState((PBYTE)g_keybuf) == FALSE) {
		//失敗した場合にはゴミが入らないように初期化
		MoveMemory(g_savekeybuf, g_keybuf, sizeof(BYTE) * 256);
	}

	if (g_keybuf[VK_SHIFT] & 0x80) {
		s_dispselect = false;
	}
	else {
		s_dispselect = true;
	}
	if ((g_keybuf[VK_F9] & 0x80) && ((g_savekeybuf[VK_F9] & 0x80) == 0)) {
		StartBt(s_model, TRUE, 0, 1);
	}
	if ((g_keybuf[VK_F10] & 0x80) && ((g_savekeybuf[VK_F10] & 0x80) == 0)) {
		StartBt(s_model, TRUE, 1, 1);
	}
	if ((g_keybuf[' '] & 0x80) && ((g_savekeybuf[' '] & 0x80) == 0)) {
		if (s_bpWorld && s_model) {
			StartBt(s_model, TRUE, 2, 1);
		}
	}
	if (g_keybuf[VK_CONTROL] & 0x80) {
		g_controlkey = true;
	}
	else {
		g_controlkey = false;
	}
	if (g_keybuf[VK_SHIFT] & 0x80) {
		g_shiftkey = true;
	}
	else {
		g_shiftkey = false;
	}


	//end of BoneTwist on MouseWheel 
	if ((s_tkeyflag != 0) && (s_editmotionflag >= 0) && ((g_keybuf['T'] & 0x80) == 0)) {
		s_tkeyflag = 0;
		PrepairUndo();
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
			if (g_erp <= 0.9){
				g_erp += 0.1;
				Sleep(200);
			}
		}
		else if (g_keybuf[VK_SUBTRACT] & 0x80){
			if (g_erp >= 0.1){
				g_erp -= 0.1;
				Sleep(200);
			}
		}
	}
	*/



	//####################################################################################################################
	//Undo,RedoのRedoのコマンドキーがCtrl + Shift + Zなので,　MButton用のキーとしてCtrl + Shiftは使えない。コメントアウト　2022/01/11
	//####################################################################################################################
	//if (g_ctrlshiftkeyformb == false) {
	//	if ((g_keybuf[VK_CONTROL] & 0x80) && (g_keybuf[VK_SHIFT] & 0x80)) {
	//		if (((g_savekeybuf[VK_CONTROL] & 0x80) == 0) || ((g_savekeybuf[VK_SHIFT] & 0x80) == 0)) {
	//			g_ctrlshiftkeyformb = true;
	//			//reset g_ctrlshiftkeyformb at the place of calling OnTimelineMButtonDown
	//		}
	//	}
	//}



	if (g_keybuf['A'] & 0x80) {
		s_akeycnt++;
	}
	else {
		s_akeycnt = 0;
	}
	if (g_keybuf['D'] & 0x80) {
		s_dkeycnt++;
	}
	else {
		s_dkeycnt = 0;
	}

	if (g_keybuf['1'] & 0x80) {//num
		s_1keycnt++;
	}
	else {
		s_1keycnt = 0;
	}



	/////// all model bone
	if (s_model && g_controlkey && (g_keybuf['A'] & 0x80) && !(g_savekeybuf['A'] & 0x80)) {
		s_allmodelbone = !s_allmodelbone;
	}

	if (g_controlkey && (s_1keycnt == 1)) {
		s_dispsampleui = !s_dispsampleui;
	}

	return 0;
}

int OnFrameUtCheckBox()
{
	//g_applyendflag = (int)s_ApplyEndCheckBox->GetChecked();
	//g_slerpoffflag = (int)s_SlerpOffCheckBox->GetChecked();
	if (s_AbsIKCheckBox) {
		g_absikflag = (int)s_AbsIKCheckBox->GetChecked();
	}
	if (s_BoneMarkCheckBox) {
		g_bonemarkflag = (int)s_BoneMarkCheckBox->GetChecked();
	}
	if (s_RigidMarkCheckBox) {
		g_rigidmarkflag = (int)s_RigidMarkCheckBox->GetChecked();
	}
	//if (s_PseudoLocalCheckBox) {
	//	g_pseudolocalflag = (int)s_PseudoLocalCheckBox->GetChecked();
	//}
	if (s_WallScrapingIKCheckBox) {
		g_wallscrapingikflag = (int)s_WallScrapingIKCheckBox->GetChecked();
	}
	if (s_LimitDegCheckBox) {
		g_limitdegflag = s_LimitDegCheckBox->GetChecked();
		if (s_model && s_model->GetCurMotInfo() && (s_curboneno >= 0) && (g_limitdegflag != s_beflimitdegflag)) {
			refreshEulerGraph();
			//s_tum.UpdateEditedEuler(refreshEulerGraph);//非ブロッキング
		}
		s_beflimitdegflag = g_limitdegflag;
	}
	if (s_BrushMirrorUCheckBox) {
		g_brushmirrorUflag = (int)s_BrushMirrorUCheckBox->GetChecked();
	}
	if (s_BrushMirrorVCheckBox) {
		g_brushmirrorVflag = (int)s_BrushMirrorVCheckBox->GetChecked();
	}
	if (s_IfMirrorVDiv2CheckBox) {
		g_ifmirrorVDiv2flag = (int)s_IfMirrorVDiv2CheckBox->GetChecked();
	}

	return 0;
}

int OnFrameAngleLimit()
{
	if (s_anglelimitdlg) {
		int setcheckflag = 0;
		if (s_model && (s_underanglelimithscroll == 0)) {//HScroll中に値を取得して設定するとスライダーが動かないから
			//s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);//commentout
			Bone2AngleLimit(setcheckflag);
			AngleLimit2Dlg(s_anglelimitdlg);
			if (g_previewFlag == 0) {//underchecking
				AngleLimit2Bone();
			}
			UpdateWindow(s_anglelimitdlg);

			//Bone2AngleLimit(setcheckflag);
			//AngleLimit2Dlg(s_anglelimitdlg);
		}
	}

	return 0;
}

int OnFramePreviewStop()
{
	if (!s_owpLTimeline) {
		return 0;
	}

	double currenttime = s_owpLTimeline->getCurrentTime();

	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){

			if (curmodel && curmodel->GetCurMotInfo()) {
				curmodel->SetMotionFrame(currenttime);
			}

			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		}
	}

	//s_tum.UpdateMatrix(s_modelindex, &s_matVP);//ブロッキング

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
	s_model->AdvanceTime(s_onefps, s_previewrange, g_previewFlag, *pdifftime, pnextframe, &endflag, &loopstartflag, -1);
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

#ifndef SKIP_EULERGRAPH__
	s_owpLTimeline->setCurrentTime(*pnextframe, false);
	s_owpEulerGraph->setCurrentTime(*pnextframe, false);
#endif


	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){
			curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		}
	}
	//s_tum.UpdateMatrix(s_modelindex, &s_matVP);//ブロッキング


	if (s_anglelimitdlg) {
		UpdateEditedEuler();
		//s_tum.UpdateEditedEuler(UpdateEditedEuler);//非ブロッキング
	}


	//playerButtonのonefpsボタン
	if (s_onefps == 1) {
		Sleep(1000);//1fps
	}
	else if (s_onefps == 2) {
		Sleep(500);//2fps
	}


	return 0;
}



int OnFramePreviewBt(double* pnextframe, double* pdifftime)
{
	if (!s_model) {
		return 0;
	}


	//BOOL isstartframe = FALSE;
	//double rangestart = 1.0;
	//s_previewrange = s_editrange;
	//if (s_previewrange.IsSameStartAndEnd()) {
	//	rangestart = 1.0;
	//}
	//else {
	//	rangestart = s_previewrange.GetStartFrame();
	//}

	//if (g_previewFlag != 0) {
	//	if (s_savepreviewFlag == 0) {
	//		//preview start frame
	//		*pdifftime = 0.0;
	//		*pnextframe = rangestart;
	//		isstartframe = TRUE;
	//	}
	//}

	if (g_previewFlag != 0) {
		if (s_savepreviewFlag == 0) {
			//preview start frame
			s_previewrange = s_editrange;
			double rangestart;
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


	//CModel* curmodel = s_model;

	bool recstopflag = false;
	vector<MODELELEM>::iterator itrmodel;
	BOOL firstmodelflag = TRUE;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;
		if (curmodel){

			if (curmodel->GetBtCnt() <= 10) {
				curmodel->SetKinematicFlag();
				//!!curmodel->SetBtEquilibriumPoint();
			}
			else {
				curmodel->SetBtKinFlagReq(curmodel->GetTopBt(), 0);
				//curmodel->SetBtEquilibriumPoint();
			}

			if ((curmodel == s_model) && (s_model->GetBtCnt() == 0)) {
				s_rectime = 0.0;
				s_reccnt = 0;
				s_model->PhysIKRec(s_rectime);
			}

			int endflag = 0;
			int loopstartflag = 0;
			//if (isstartframe == FALSE) {
				curmodel->AdvanceTime(s_onefps, s_previewrange, g_previewFlag, *pdifftime, pnextframe, &endflag, &loopstartflag, -1);
			//}
			//else {
			//	loopstartflag = 1;
			//}

			if ((g_btsimurecflag == true) && ((loopstartflag == 1) || (endflag == 1))) {
				recstopflag = true;//!!!!!!!!!!!!!!!!!!!!!!!
				break;
			}


			if (firstmodelflag) {
#ifndef SKIP_EULERGRAPH__
				s_owpLTimeline->setCurrentTime(*pnextframe, false);
				s_owpEulerGraph->setCurrentTime(*pnextframe, false);
#endif
				firstmodelflag = 0;
			}
			//if (endflag == 1) {
				//g_previewFlag = 0;
			//}

			int firstflag = 0;
			curmodel->SetMotionFrame(*pnextframe);
			////if ((IsTimeEqual(*pnextframe, rangestart)) || (loopstartflag == 1)){
			//if((curmodel->GetBtCnt() != 0) && (loopstartflag == 1)) {
			//	curmodel->ZeroBtCnt();
			//	//g_previewFlag = 0;
			//	//StartBt(curmodel, firstmodelflag, 2, 0);
			//	//firstflag = 1;

			//	curmodel->SetKinematicFlag();
			//	//!!curmodel->SetBtEquilibriumPoint();

			//}



			if ((curmodel->GetBtCnt() != 0) && (loopstartflag == 1)) {
				curmodel->ZeroBtCnt();
				//StartBt(curmodel, TRUE, 2, 1);//flag = 2 --> resetflag = 1  //短いリターゲットモデルが頻繁に呼び出し重すぎるし他のモデルも揺れなくなる。作り直さなくても良いのでコメントアウト.　2022/01/11
			}
			else {
				//UpdateBtSimu(*pnextframe, curmodel);
				if (curmodel && curmodel->GetCurMotInfo()) {
					curmodel->Motion2Bt(firstflag, *pnextframe, &curmodel->GetWorldMat(), &s_matVP, s_curboneno);
				}
				curmodel->PlusPlusBtCnt();
			}
		}
	}

	if (s_model && (recstopflag == true)) {
		StopBt();
		s_model->ApplyPhysIkRec();
		PrepairUndo();
		g_btsimurecflag = false;
	}
	else {
		s_bpWorld->clientMoveAndDisplay();

		for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
			CModel* curmodel = itrmodel->modelptr;
			//if (curmodel) {
			if(curmodel && (curmodel->GetBtCnt() != 0)){
				if (curmodel && curmodel->GetCurMotInfo()) {
					//curmodel->SetBtMotion(curmodel->GetBoneByID(s_curboneno), 0, *pnextframe, &curmodel->GetWorldMat(), &s_matVP);
					curmodel->SetBtMotion(0, 0, *pnextframe, &curmodel->GetWorldMat(), &s_matVP);//第一引数は物理IK用

					//60 x 30 frames limit : 30 sec limit
					if ((curmodel == s_model) && (s_model->GetBtCnt() > 0) && (s_reccnt < MAXPHYSIKRECCNT)) {
						s_rectime = (double)((int)s_reccnt);
						s_model->PhysIKRec(s_rectime);
						s_reccnt++;
					}
				}
			}
		}

		//s_tum.SetBtMotion(OnFramePreviewBtAftFunc, s_modelindex, *pnextframe);

	}

	//playerButtonのonefpsボタン
	if (s_onefps == 1) {
		Sleep(1000);//1fps
	}
	else if (s_onefps == 2) {
		Sleep(500);//2fps
	}


	return 0;
}

int OnFramePreviewBtAftFunc(double nextframe, CModel* curmodel)
{
	if (curmodel && (curmodel->GetBtCnt() != 0)) {
		if (curmodel && curmodel->GetCurMotInfo()) {
			curmodel->SetBtMotion(curmodel->GetBoneByID(s_curboneno), 0, nextframe, &curmodel->GetWorldMat(), &s_matVP);

			//60 x 30 frames limit : 30 sec limit
			if ((curmodel == s_model) && (s_model->GetBtCnt() > 0) && (s_reccnt < MAXPHYSIKRECCNT)) {
				s_rectime = (double)((int)s_reccnt);
				s_model->PhysIKRec(s_rectime);
				s_reccnt++;
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
		curmodel->SetBtMotion(curmodel->GetBoneByID(s_curboneno), 0, nextframe, &curmodel->GetWorldMat(), &s_matVP);
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

	//BOOL isstartframe = FALSE;
	//double rangestart = 1.0;
	//s_previewrange = s_editrange;
	//if (s_previewrange.IsSameStartAndEnd()) {
	//	rangestart = 1.0;
	//}
	//else {
	//	rangestart = s_previewrange.GetStartFrame();
	//}

	//if (g_previewFlag != 0) {
	//	if (s_savepreviewFlag == 0) {
	//		//preview start frame
	//		*pdifftime = 0.0;
	//		*pnextframe = rangestart;
	//		isstartframe = TRUE;
	//	}
	//}


	//*pnextframe = s_previewrange.GetApplyFrame();
	int tmpleng;
	double tmpstart, tmpend;
	s_previewrange.GetRange(&tmpleng, &tmpstart, &tmpend);
	*pnextframe = s_previewrange.GetApplyFrame();
	//*pnextframe = g_motionbrush_applyframe;

	if (curmodel && curmodel->GetCurMotInfo()){
		//if (s_onragdollik != 0){
		//	if (s_underikflag == 0){
		//		curmodel->BulletSimulationStop();
		//		s_underikflag = 1;
		//	}
		//}


		if (curmodel->GetBtCnt() <= g_prepcntonphysik) {
			curmodel->SetKinematicFlag();
			curmodel->SetMotionFrame(*pnextframe);

			//UpdateBtSimu(*pnextframe, curmodel);
			if (curmodel && curmodel->GetCurMotInfo()) {
				int firstflag = 1;
				curmodel->Motion2Bt(firstflag, *pnextframe, &curmodel->GetWorldMat(), &s_matVP, s_curboneno);
			}
			curmodel->SetBtEquilibriumPoint();//必要

			if (curmodel->GetBtCnt() == 0) {
				s_rectime = 0.0;
				s_reccnt = 0;
				s_model->PhysIKRec(s_rectime);
				SetKinematicToHand(curmodel, true);//指が変にならないように
			}
		}
		else {
			curmodel->SetRagdollKinFlag(s_curboneno, s_physicskind);
			curmodel->SetBtEquilibriumPoint();//
		}




		//curmodel->SetRagdollKinFlag(s_curboneno, s_physicskind);

	}

	//physics mvの場合、クリックしていないときには直下のif文内を呼ばない。
	//physics rotの場合、クリックしていなくても直下のif文内を呼んだほうが剛体がゆらゆらしない。
	if((s_curboneno >= 0) && ((s_onragdollik != 0) || (s_physicskind == 0))){
		s_pickinfo.mousebefpos = s_pickinfo.mousepos;
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_3dwnd, &ptCursor);
		s_pickinfo.mousepos = ptCursor;

		ChaVector3 tmpsc;
		curmodel->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

		if (s_oprigflag == 0) {//Rig操作ではないとき
			ChaVector3 targetpos(0.0f, 0.0f, 0.0f);
			CallF(CalcTargetPos(&targetpos), return 1);

			s_model->SetDofRotAxis(s_pickinfo.buttonflag);//!!!!!!!!!!!!!!!!!!!!!!!


			if (s_physicskind == 0) {
				if (s_onragdollik == 1) {
					int ikmaxlevel = 0;
					curmodel->PhysicsRot(&s_editrange, s_pickinfo.pickobjno, targetpos, ikmaxlevel);
				}
				else if ((s_onragdollik == 2) || (s_onragdollik == 3)) {
					float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.1f;
					if (g_controlkey == true) {
						deltax *= 0.250f;
					}
					//s_editmotionflag = s_model->PhysicsRotAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, g_iklevel, s_ikcnt, s_ikselectmat);
					s_editmotionflag = s_model->PhysicsRotAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_curboneno, deltax, g_iklevel, s_ikcnt, s_ikselectmat);
					s_ikcnt++;
				}

				//if (s_curboneno <= 0){
				//	::MessageBoxA(NULL, "OnFramePreviewRagdoll : curboneno error", "check", MB_OK);
				//}

			}
			else {
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//少しずつ動かさないと壊れやすい
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				int ikmaxlevel = 0;
				ChaVector3 diffvec = (targetpos - s_pickinfo.objworld) * 0.2;//PhysicsMV関数内でさらにg_physicsmvrateが掛けられる。
				curmodel->PhysicsMV(&s_editrange, s_pickinfo.pickobjno, diffvec);
			}
		}


		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		if (curbone) {
			CBone* parentbone = curbone->GetParent();
			if (parentbone) {
				s_editmotionflag = parentbone->GetBoneNo();
			}
			else {
				s_editmotionflag = s_curboneno;
			}
			s_ikcnt++;
		}


	}


	//curmodel->SetMotionFrame(s_editrange.GetStartFrame());
	//*pnextframe = s_editrange.GetStartFrame();//!!!!!!!!!!!!!!!
	{
		int tmpleng2;
		double tmpstart2, tmpend2;
		s_previewrange.GetRange(&tmpleng2, &tmpstart2, &tmpend2);
		*pnextframe = s_previewrange.GetApplyFrame();
		curmodel->SetMotionFrame(*pnextframe);
	}

#ifndef SKIP_EULERGRAPH__
	s_owpLTimeline->setCurrentTime(*pnextframe, false);
	s_owpEulerGraph->setCurrentTime(*pnextframe, false);
#endif


	s_bpWorld->clientMoveAndDisplay();

	if (curmodel && curmodel->GetCurMotInfo()){
		curmodel->SetBtMotion(curmodel->GetBoneByID(s_curboneno), 1, *pnextframe, &curmodel->GetWorldMat(), &s_matVP);
		curmodel->UpdateMatrix(&curmodel->GetWorldMat(), &s_matVP);
		curmodel->PlusPlusBtCnt();

		//ドラッグ中だけ記録
		if ((s_curboneno >= 0) && ((s_onragdollik != 0) || (s_physicskind == 0))) {
			//60 x 30 frames limit : 30 sec limit
			if ((curmodel->GetBtCnt() > g_prepcntonphysik) && (s_reccnt < MAXPHYSIKRECCNT)) {
				s_rectime = (double)((int)s_reccnt);
				s_model->PhysIKRec(s_rectime);
				s_reccnt++;
			}
		}
	}

	s_bpWorld->clientMoveAndDisplay();//tmpkinematic部分のbullet情報変更がSetBtMotion内で生じるので、もう一回シミュをまわす。


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
	if (s_closemotionFlag) {
		s_closemotionFlag = false;
		s_dispmotion = false;
		if (s_motionpanel.panel) {
			s_motionpanel.panel->setVisible(0);
		}
	}
	if (s_closeconvboneFlag){
		s_closeconvboneFlag = false;
		s_dispconvbone = false;
		if (s_convboneWnd){
			s_convboneWnd->setVisible(false);
			s_convboneWnd->setListenMouse(false);
		}
	}
	if (s_DcloseFlag){
		s_DcloseFlag = false;
		s_dmpanimWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(btScalar(g_erp));
		}
		if (s_model){
			CallF(s_model->CreateBtObject(0), return 1);
		}
	}
	if (s_RcloseFlag){
		s_RcloseFlag = false;
		s_rigidWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(btScalar(g_erp));
		}
		if (s_model){
			CallF(s_model->CreateBtObject(0), return 1);
		}
	}
	if (s_ScloseFlag) {
		s_ScloseFlag = false;
		s_sidemenuWnd->setVisible(0);
	}
	if (s_IcloseFlag){
		s_IcloseFlag = false;
		s_impWnd->setVisible(0);
		if (s_bpWorld){
			s_bpWorld->setGlobalERP(btScalar(g_erp));
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


int TimelineCursorToMotion()
{
	if (s_owpTimeline && s_model && s_model->GetCurMotInfo()) {

		GetCurrentBoneFromTimeline(&s_curboneno);

		// カーソル位置を姿勢に反映。
		if (g_previewFlag == 0) {//underchecking
			double curframe = s_owpTimeline->getCurrentTime();// 選択時刻

			vector<MODELELEM>::iterator itrmodel;
			for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
				CModel* curmodel = itrmodel->modelptr;
				if (curmodel && curmodel->GetCurMotInfo()) {
					curmodel->SetMotionFrame(curframe);
				}
			}
		}
	}
	return 0;
}


int OnFrameTimeLineWnd()
{
	// カーソル移動フラグを確認 //////////////////////////////////////////////////


	if (g_previewFlag != 0) {//underchecking

		//カレントフレームの１フレームを選択状態にしてしまうのでfalseにする
		s_cursorFlag = false;
		s_LcursorFlag = false;
		s_keyShiftFlag = false;
		return 0;
	}



	if (s_LstartFlag) {
		s_LstartFlag = false;
		s_buttonselectstart = s_editrange.GetStartFrame();
		s_buttonselectend = s_editrange.GetEndFrame();
		if (s_owpTimeline) {
			s_owpTimeline->setCurrentTime(s_buttonselectstart, false);
		}
		if (s_owpLTimeline) {
			s_owpLTimeline->setCurrentTime(s_buttonselectstart, false);
		}
		vector<MODELELEM>::iterator itrmodel;
		for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
			CModel* curmodel = itrmodel->modelptr;
			if (curmodel && curmodel->GetCurMotInfo()) {
				curmodel->SetMotionFrame(s_buttonselectstart);
				//curmodel->UpdateMatrix(&s_matWorld, &s_matVP);
				curmodel->UpdateMatrix(&(curmodel->GetWorldMat()), &s_matVP);//2021/12/21
			}
		}

		int setcheckflag = 1;
		Bone2AngleLimit(setcheckflag);

	}


	if (s_cursorFlag) {
		s_cursorFlag = false;
		TimelineCursorToMotion();
	}

	if (s_LcursorFlag) {
		s_LcursorFlag = false;

		if (g_underselectingframe == 0) {
			//これがないとモーション停止ボタンを押した後にselect表示されない。
			s_buttonselectstart = s_editrange.GetStartFrame();
			s_buttonselectend = s_editrange.GetEndFrame();
			
			//if (s_copyKeyInfoList.size() == 0) {//フレームを選択していないときだけ呼ぶ。選択済のときにTimeline::OnSelectButtonがループするのを防止
				OnTimeLineButtonSelectFromSelectStartEnd(0);
			//}
		}

		//g_underselectingframe = 1;
		OnTimeLineCursor(0, 0.0);

		if (g_previewFlag != 0) {
			//これがないとモーション再生中にselectが表示されない。
			s_buttonselectstart = s_previewrange.GetStartFrame();
			s_buttonselectend = s_previewrange.GetEndFrame();
			OnTimeLineButtonSelectFromSelectStartEnd(0);
		}

		if (s_owpLTimeline && s_model && s_model->GetCurMotInfo()) {
			if (g_previewFlag == 0) {//underchecking
				double curframe = s_owpLTimeline->getCurrentTime();// 選択時刻
				vector<MODELELEM>::iterator itrmodel;
				for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++) {
					CModel* curmodel = itrmodel->modelptr;
					if (curmodel && curmodel->GetCurMotInfo()) {
						curmodel->SetMotionFrame(curframe);
					}
				}
			}
		}

		OnFrameAngleLimit();

		//if (s_anglelimitdlg) {
		//	int setcheckflag = 0;
		//	if (s_underanglelimithscroll == 0) {//HScroll中に値を取得して設定するとスライダーが動かないから
		//		AngleLimit2Bone();
		//		Bone2AngleLimit(setcheckflag);
		//		AngleLimit2Dlg(s_anglelimitdlg);
		//	}
		//}


	}





	// キー移動フラグを確認 ///////////////////////////////////////////////////////////
	//if (s_keyShiftFlag){
		s_keyShiftFlag = false;
	//}

	return 0;
}

int OnFrameMouseButton()
{
	if (s_timelinembuttonFlag || g_ctrlshiftkeyformb){
		s_timelinembuttonFlag = false;
		OnTimeLineMButtonDown(g_ctrlshiftkeyformb);
		g_ctrlshiftkeyformb = false;
	}
	if (s_timelinewheelFlag || (g_underselectingframe && ((g_keybuf['A'] & 0x80) || (g_keybuf['D'] & 0x80)))){
		s_timelinewheelFlag = false;
		OnTimeLineWheel();
	}

	if (s_timelineshowposFlag) {
		s_timelineshowposFlag = false;

		if (s_owpLTimeline) {
			s_owpLTimeline->WheelShowPosTime();
		}
		if (s_owpEulerGraph) {
			s_owpEulerGraph->WheelShowPosTime();
			refreshEulerGraph();
			//s_tum.UpdateEditedEuler(refreshEulerGraph);//非ブロッキング
		}
		
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
			s_underframecopydlg = true;
			s_selbonedlg.DoModal();
			s_underframecopydlg = false;
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
			//s_model->SaveUndoMotion(s_curboneno, s_curbaseno, (double)g_applyrate);

			//int keynum;
			//double startframe, endframe, applyframe;
			//s_editrange.Clear();
			//s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
			//s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);
			//s_model->InterpolateBetweenSelection(startframe, endframe);

			s_model->InterpolateBetweenSelection(s_buttonselectstart, s_buttonselectend);

			UpdateEditedEuler();

			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
		}

	}



	if (s_opedelmodelcnt >= 0) {
		int modelcnt = s_opedelmodelcnt;
		s_opedelmodelcnt = -1;
		bool ondelbutton = true;
		OnDelModel(modelcnt, ondelbutton);//s_modelpanel.modelindexはs_modelのindexなので違う
	}
	if ((s_opeselectmodelcnt >= 0) && s_modelpanel.panel) {
		s_modelpanel.modelindex = s_opeselectmodelcnt;
		s_opeselectmodelcnt = -1;
		OnModelMenu(true, s_modelpanel.modelindex, 1);
		s_modelpanel.panel->callRewrite();
	}
	if (s_opedelmotioncnt >= 0) {
		int delmenuindex = s_opedelmotioncnt;
		s_opedelmotioncnt = -1;
		bool ondelbutton = true;
		OnDelMotion(delmenuindex, ondelbutton);
	}
	if ((s_opeselectmotioncnt >= 0) && s_motionpanel.panel) {
		int motionindex = s_opeselectmotioncnt;
		s_opeselectmotioncnt = -1;
		OnAnimMenu(true, motionindex, 1);
		s_motionpanel.panel->callRewrite();
	}



	if (s_selCopyHisotryFlag) {
		s_selCopyHisotryFlag = false;

		GUIMenuSetVisible(-1, -1);

		GetCPTFileName(s_cptfilename);
		//s_copyhistorydlg.SetNames(s_cptfilename);
		//s_copyhistorydlg.DoModal();
		if (s_copyhistorydlg.GetCreatedFlag() == false) {
			s_copyhistorydlg.Create(s_mainhwnd);
		}
		SetParent(s_copyhistorydlg.m_hWnd, s_mainhwnd);

		SetWindowPos(
			s_copyhistorydlg.m_hWnd,
			HWND_TOP,
			s_timelinewidth + s_mainwidth + 16,
			s_sidemenuheight,
			s_sidewidth,
			s_sideheight,
			SWP_SHOWWINDOW
		);

		s_copyhistorydlg.ShowWindow(SW_SHOW);
		s_copyhistorydlg.SetNames(s_cptfilename);
	}

	if (s_copyFlag){
		s_copyFlag = false;
		s_undersymcopyFlag = false;

		if (s_model && s_owpTimeline && s_owpLTimeline && s_model->GetCurMotInfo()){
			s_copymotvec.clear();
			s_copyKeyInfoList.clear();
			s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();

			list<KeyInfo>::iterator itrcp;
			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
				double curframe = itrcp->time;
				InsertCopyMPReq(s_model->GetTopBone(), curframe);
			}

			if (!s_copymotvec.empty()) {
				//添付フォルダのファイルに記録
				WCHAR retcptfilename[MAX_PATH] = { 0L };
				int result = WriteCPTFile(retcptfilename);
				_ASSERT(result == 0);
				int result2 = WriteCPIFile(retcptfilename);//cp info
				_ASSERT(result2 == 0);
			}

			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);


			if (s_copyhistorydlg.GetCreatedFlag() == true) {
				GetCPTFileName(s_cptfilename);
				s_copyhistorydlg.SetNames(s_cptfilename);
			}
		}
	}

	if (s_symcopyFlag){
		s_symcopyFlag = false;
		s_undersymcopyFlag = true;

		if (s_model && s_owpTimeline && s_owpLTimeline && s_model->GetCurMotInfo()){
			int symrootmode = GetSymRootMode();

			s_copymotvec.clear();
			s_copyKeyInfoList.clear();
			s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();

			list<KeyInfo>::iterator itrcp;
			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
				double curframe = itrcp->time;
				InsertSymMPReq(s_model->GetTopBone(), curframe, symrootmode);
			}

			if (!s_copymotvec.empty()) {
				//添付フォルダのファイルに記録
				WCHAR retcptfilename[MAX_PATH] = { 0L };
				int result = WriteCPTFile(retcptfilename);
				_ASSERT(result == 0);
				int result2 = WriteCPIFile(retcptfilename);//cp info
				_ASSERT(result2 == 0);
			}

			if (s_copyhistorydlg.GetCreatedFlag() == true) {
				GetCPTFileName(s_cptfilename);
				s_copyhistorydlg.SetNames(s_cptfilename);
			}
		}
	}


	if (s_pasteFlag){
		s_pasteFlag = false;


		//添付ファイルを読み取ってs_pastemotvecに格納する
		s_pastemotvec.clear();
		bool result;
		result = LoadCPTFile();

		if (result && s_model && s_owpTimeline && s_model->GetCurMotInfo() && !s_pastemotvec.empty())
		{
			int cpnum = (int)s_selbonedlg.m_cpvec.size();
			int keynum = 0;
			double startframe, endframe, applyframe;

			double pastestartframe = 0.0;
			s_editrange.Clear();
			if (s_model && s_model->GetCurMotInfo()) {
				if (s_owpTimeline && s_owpLTimeline) {
					s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
					s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);
				}
			}

			//double curmaxframe = s_model->m_curmotinfo->frameleng;

			//コピーされたキーの先頭時刻を求める
			double copyStartTime = DBL_MAX;
			double copyEndTime = 0;
			vector<CPELEM2>::iterator itrcp;
			for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++) {
				if (itrcp->mp.GetFrame() <= copyStartTime) {
					copyStartTime = itrcp->mp.GetFrame();
				}
				if (itrcp->mp.GetFrame() >= copyEndTime) {
					copyEndTime = itrcp->mp.GetFrame();
				}
			}

			if (keynum >= 0) {
				if (keynum == 0) {
					double motleng = s_model->GetCurMotInfo()->frameleng - 1;
					double srcendframe = min(motleng, startframe + (copyEndTime - copyStartTime));
					srcendframe = max(srcendframe, 0.0);
					PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, srcendframe);
				}
				else {
					PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, endframe);

					//if (keynum <= (int)(copyEndTime - copyStartTime + 1.0 + 0.1)){
					//	PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, endframe);
					//}else{
					//	PasteMotionPointJustInTerm(copyStartTime, copyEndTime, startframe, endframe);

					//	//コピー元の最終フレームの姿勢をコピー先の残りのフレームにペースト
					//	PasteMotionPointAfterCopyEnd(copyStartTime, copyEndTime, startframe, endframe);
					//}
				}
			}

			UpdateEditedEuler();
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
		}

	}

	if (s_motpropFlag){
		s_motpropFlag = false;

		if (s_model && s_model->GetCurMotInfo()){
			int dlgret;
			dlgret = (int)DialogBoxW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MOTPROPDLG),
				s_3dwnd, (DLGPROC)MotPropDlgProc);
			if ((dlgret == IDOK) && s_tmpmotname[0]){
				WideCharToMultiByte(CP_ACP, 0, s_tmpmotname, -1, s_model->GetCurMotInfo()->motname, 256, NULL, NULL);
				//s_model->m_curmotinfo->frameleng = s_tmpmotframeleng;
				s_model->GetCurMotInfo()->loopflag = s_tmpmotloop;
				double oldframeleng = s_model->GetCurMotInfo()->frameleng;

				s_owpTimeline->setMaxTime(s_tmpmotframeleng);
				s_model->ChangeMotFrameLeng(s_model->GetCurMotInfo()->motid, s_tmpmotframeleng);//はみ出たmpも削除
				InitCurMotion(0, oldframeleng);

				//メニュー書き換え, timeline update
				OnAnimMenu(true, s_motmenuindexmap[s_model]);
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
	::DSMessageBox(NULL, L"SetWorldMatFromEul", L"Check", MB_OK);
	}
	}
	}
	}
	*/

	if (s_filterFlag == true){
		s_filterFlag = false;

		if (s_model){
			s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
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

					UpdateEditedEuler();

					if (s_model){
						s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
					}
				}
				else{
					::DSMessageBox(s_3dwnd, L"Retry After Setting Of Selection MultiFrames.", L"error!!!", MB_OK);
				}
			}
		}


	}

	if (s_btresetFlag == true) {
		s_btresetFlag = false;
		if (s_model) {
			StartBt(s_model, TRUE, 2, 1);//flag = 2 --> resetflag = 1
		}
	}


	if (s_deleteFlag){
		s_deleteFlag = false;
		/***
		if( s_model && s_owpTimeline && s_model->m_curmotinfo){
		s_owpTimeline->deleteKey();
		//motionpointのdeleteはdelete Listenerでする。
		s_model->SaveUndoMotion(s_curboneno, s_curbaseno, &s_editrange, (double)g_applyrate);
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

				CBone* parentbone = srcbone->GetParent();
				if (parentbone){
					int cpno2;
					for (cpno2 = 0; cpno2 < cpnum; cpno2++){
						CBone* chkparentbone = s_selbonedlg.m_cpvec[cpno2];
						if (chkparentbone == parentbone){
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

				CBone* parentbone = srcbone->GetParent();
				if (parentbone){
					int cpno2;
					for (cpno2 = 0; cpno2 < cpnum; cpno2++){
						CBone* chkparentbone = s_selbonedlg.m_cpvec[cpno2];
						if (chkparentbone == parentbone){
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
				CBone* parentbone = srcbone->GetParent();
				if (parentbone){
					CMotionPoint* parmp = parentbone->GetMotionPoint(curmotid, newframe);
					int setmatflag1 = 1;
					CQuaternion dummyq;
					ChaVector3 dummytra = ChaVector3(0.0f, 0.0f, 0.0f);

					parmp->SetBefWorldMat(parmp->GetWorldMat());
					bool infooutflag = false;
					srcbone->RotBoneQReq(infooutflag, parentbone, curmotid, newframe, dummyq, 0, dummytra);
					//_ASSERT(0);
				}
			}
		}
	}

	return 0;
}

int PasteMotionPointJustInTerm(double copyStartTime, double copyEndTime, double startframe, double endframe)
{
	int cpnum = (int)s_selbonedlg.m_cpvec.size();


	double srcleng;
	double dstleng;
	srcleng = copyEndTime - copyStartTime + 1;
	dstleng = endframe - startframe + 1;
	double dstframe;
	for (dstframe = startframe; dstframe <= endframe; dstframe++) {
		double dstrate = (dstframe - startframe) / dstleng;
		double srcframe;
		srcframe = (double)((int)(copyStartTime + dstrate * srcleng));
		vector<CPELEM2>::iterator itrcp;
		for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++) {
			CBone* srcbone = itrcp->bone;
			if (srcbone) {
				CMotionPoint srcmp = itrcp->mp;
				if (srcmp.GetFrame() == srcframe) {
					PasteMotionPoint(srcbone, srcmp, dstframe);
				}
			}
		}
	}

	//移動しないボーンのための処理
	for (dstframe = startframe; dstframe <= endframe; dstframe++) {
		double dstrate = (dstframe - startframe) / dstleng;
		double srcframe;
		srcframe = (double)((int)(copyStartTime + dstrate * srcleng));
		vector<CPELEM2>::iterator itrcp;
		for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++) {
			CBone* srcbone = itrcp->bone;
			if (srcbone) {
				CMotionPoint srcmp = itrcp->mp;
				if (srcmp.GetFrame() == srcframe) {
					PasteNotMvParMotionPoint(srcbone, srcmp, dstframe);
				}
			}
		}
	}


	//vector<CPELEM>::iterator itrcp;
	//for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++){
	//	CBone* srcbone = itrcp->bone;
	//	if (srcbone){
	//		CMotionPoint srcmp = itrcp->mp;
	//		double newframe = (double)((int)(srcmp.GetFrame() - copyStartTime + startframe + 0.1));//!!!!!!!!!!!!!!!!!!
	//		if ((newframe >= startframe) && (newframe <= endframe)){
	//			PasteMotionPoint(srcbone, srcmp, newframe);
	//		}
	//	}
	//}

	////移動しないボーンのための処理
	//for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++){
	//	CBone* srcbone = itrcp->bone;
	//	if (srcbone){
	//		CMotionPoint srcmp = itrcp->mp;
	//		double newframe = (double)((int)(srcmp.GetFrame() - copyStartTime + startframe + 0.1));//!!!!!!!!!!!!!!!!!!
	//		if ((newframe >= startframe) && (newframe <= endframe)){
	//			PasteNotMvParMotionPoint(srcbone, srcmp, newframe);
	//		}
	//	}
	//}

	return 0;
}

int PasteMotionPointAfterCopyEnd(double copyStartTime, double copyEndTime, double startframe, double endframe)
{
	vector<CPELEM2>::iterator itrcp;

	double newframe;
	for (newframe = (double)((int)(copyEndTime - copyStartTime + startframe + 0.1)); newframe <= endframe; newframe += 1.0){
		for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++){
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
		for (itrcp = s_pastemotvec.begin(); itrcp != s_pastemotvec.end(); itrcp++){
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


void DispProgressCalcLimitedWM()
{
	LONG framenum = InterlockedAdd(&s_progressnum, 0);
	LONG curframe = InterlockedAdd(&s_progresscnt, 0);
	LONG modelnum = InterlockedAdd(&s_progressmodelnum, 0);
	LONG modelcnt = InterlockedAdd(&s_progressmodelcnt, 0);
	LONG befframenum = InterlockedAdd(&s_befprogressnum, 0);
	LONG befcurframe = InterlockedAdd(&s_befprogresscnt, 0);
	LONG befmodelnum = InterlockedAdd(&s_befprogressmodelnum, 0);
	LONG befmodelcnt = InterlockedAdd(&s_befprogressmodelcnt, 0);

	if (( framenum != 0) && (modelnum != 0) && 
		((framenum != befframenum) || (curframe != befcurframe) || (modelnum != befmodelnum) || (modelcnt != befmodelcnt))) {//ちらつかないように変更があったときだけ
		//if ((curframe % 25) == 0) {
			if (s_progresswnd) {
				HWND hProg2;
				hProg2 = GetDlgItem(s_progresswnd, IDC_PROGRESS1);
				if (hProg2) {
					//現在位置を設定 
					int curpercent = (int)((double)curframe / (double)framenum * 100.0);
					curpercent = min(100, curpercent);
					curpercent = max(0, curpercent);
					SendMessage(hProg2, PBM_SETPOS, (WPARAM)curpercent, 0);
				}
			}
			if (s_progresswnd) {
				WCHAR strnumcnt[1024] = { 0L };
				swprintf_s(strnumcnt, 1024, L"%d / %d chara (cnt / num)", modelcnt, modelnum);
				SetDlgItemTextW(s_progresswnd, IDC_STRBVH2FBXBATCH, strnumcnt);

				UpdateWindow(s_progresswnd);
			}
			//SleepEx(1, TRUE);
		//}
	}

	InterlockedExchange(&s_befprogressnum, framenum);
	InterlockedExchange(&s_befprogresscnt, curframe);
	InterlockedExchange(&s_befprogressmodelnum, modelnum);
	InterlockedExchange(&s_befprogressmodelcnt, modelcnt);
}
 
int OnFrameBatchThread()
{
	//bhv2fbx batch
	if(InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 1){//under thread working
		LONG fbxnum = InterlockedAdd(&s_bvh2fbxnum, 0);
		LONG fbxcnt = InterlockedAdd(&s_bvh2fbxcnt, 0);
		LONG beffbxnum = InterlockedAdd(&s_befbvh2fbxnum, 0);
		LONG beffbxcnt = InterlockedAdd(&s_befbvh2fbxcnt, 0);
		if (s_bvh2fbxbatchwnd && (fbxnum > 0) && 
			((fbxnum != beffbxnum) || (fbxcnt != beffbxcnt))) {//ちらつかないように変更があったときだけ
			WCHAR strnumcnt[1024] = { 0L };
			swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", (fbxcnt + 1), fbxnum);
			SetDlgItemTextW(s_bvh2fbxbatchwnd, IDC_STRBVH2FBXBATCH, strnumcnt);

			if (s_bvh2fbxbatchwnd) {
				HWND hProg = GetDlgItem(s_bvh2fbxbatchwnd, IDC_PROGRESS1);
				if (hProg) {
					//現在位置を設定
					int percent = (int)((double)fbxcnt / (double)fbxnum * 100.0);
					SendMessage(hProg, PBM_SETPOS, percent, 0);
					//ステップの範囲を設定 
					//SendMessage(hProg, PBM_SETSTEP, 1, 0);
				}
				UpdateWindow(s_bvh2fbxbatchwnd);
			}
		}
		InterlockedExchange(&s_befbvh2fbxnum, fbxnum);
		InterlockedExchange(&s_befbvh2fbxcnt, fbxcnt);
	}
	else if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 2) || (InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 3)) {//2はダイアログでのキャンセル
		InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)0);
		if (s_bvh2fbxbatchwnd) {
			SendMessage(s_bvh2fbxbatchwnd, WM_CLOSE, 0, 0);
		}
		InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)0);//WM_CLOSEで変わる可能性あり
		InterlockedExchange(&s_bvh2fbxcnt, 0);
		InterlockedExchange(&s_bvh2fbxnum, 0);
		InterlockedExchange(&s_befbvh2fbxnum, 0);
		InterlockedExchange(&s_befbvh2fbxcnt, 0);
	}


	//retarget batch
	if (InterlockedAdd(&g_retargetbatchflag, 0) == 1) {//under thread working
		LONG retargetnum = InterlockedAdd(&s_retargetnum, 0);
		LONG retargetcnt = InterlockedAdd(&s_retargetcnt, 0);
		LONG befretargetnum = InterlockedAdd(&s_befretargetnum, 0);
		LONG befretargetcnt = InterlockedAdd(&s_befretargetcnt, 0);
		if (s_retargetbatchwnd && (retargetnum > 0) && 
			((retargetnum != befretargetnum) || (retargetcnt != befretargetcnt))) {//ちらつかないように変更があったときだけ
			WCHAR strnumcnt[1024] = { 0L };
			swprintf_s(strnumcnt, 1024, L"%d / %d (cnt / num)", retargetcnt, retargetnum);
			SetDlgItemTextW(s_retargetbatchwnd, IDC_STRBVH2FBXBATCH, strnumcnt);

			if (s_retargetbatchwnd) {
				HWND hProg = GetDlgItem(s_retargetbatchwnd, IDC_PROGRESS1);
				if (hProg) {
					//現在位置を設定
					int percent = (int)((double)retargetcnt / (double)retargetnum * 100.0);
					SendMessage(hProg, PBM_SETPOS, percent, 0);
					//ステップの範囲を設定 
					//SendMessage(hProg, PBM_SETSTEP, 1, 0);
				}
				UpdateWindow(s_retargetbatchwnd);
			}
		}
		InterlockedExchange(&s_befretargetnum, retargetnum);
		InterlockedExchange(&s_befretargetcnt, retargetcnt);
	}
	else if ((InterlockedAdd(&g_retargetbatchflag, 0) == 2) || (InterlockedAdd(&g_retargetbatchflag, 0) == 3)) {//2はダイアログでのキャンセル
		InterlockedExchange(&g_retargetbatchflag, (LONG)0);
		if (s_retargetbatchwnd) {
			SendMessage(s_retargetbatchwnd, WM_CLOSE, 0, 0);
		}
		InterlockedExchange(&g_retargetbatchflag, (LONG)0);//WM_CLOSEで変わる可能性あり
		InterlockedExchange(&s_retargetcnt, 0);
		InterlockedExchange(&s_retargetnum, 0);
		InterlockedExchange(&s_befretargetnum, 0);
		InterlockedExchange(&s_befretargetcnt, 0);
	}


	return 0;
}

int OnFrameStartPreview(double curtime, double* psavetime)
{

	//normal preview start
	if (s_calclimitedwmState == 2) {
		if (InterlockedAdd(&g_calclimitedwmflag, 0) == 1) {//under threadfunc working
			DispProgressCalcLimitedWM();
		}
		else if (InterlockedAdd(&g_calclimitedwmflag, 0) == 2) {//confirm threadfunc finished
			s_calclimitedwmState = 0;
			g_previewFlag = 1;//!!!!!!
			InterlockedExchange(&g_calclimitedwmflag, (LONG)0);
		}
		*psavetime = curtime;
	}
	if (s_calclimitedwmState == 1) {
		s_calclimitedwmState = 2;
		CalcLimitedWorldMat();
	}

	//preview to backword
	if (s_calclimitedwmState == 22) {
		if (InterlockedAdd(&g_calclimitedwmflag, 0) == 1) {//under threadfunc working
			DispProgressCalcLimitedWM();
		}
		else if (InterlockedAdd(&g_calclimitedwmflag, 0) == 2) {//confirm threadfunc finished
			s_calclimitedwmState = 0;
			g_previewFlag = -1;//!!!!!
			InterlockedExchange(&g_calclimitedwmflag, (LONG)0);
		}
		*psavetime = curtime;
	}
	if (s_calclimitedwmState == 11) {
		s_calclimitedwmState = 22;
		CalcLimitedWorldMat();
	}


	//bullet simulation start
	if (s_calclimitedwmState == 107) {
		s_calclimitedwmState = 0;
		StartBt(s_model, TRUE, 0, 1);
		*psavetime = curtime;
	}
	if ((s_calclimitedwmState >= 103) && (s_calclimitedwmState <= 106)) {
		s_calclimitedwmState++;
	}
	if(s_calclimitedwmState == 102){
		if (InterlockedAdd(&g_calclimitedwmflag, 0) == 1) {//under threadfunc working
			DispProgressCalcLimitedWM();
		}
		else if (InterlockedAdd(&g_calclimitedwmflag, 0) == 2) {//confirm threadfunc finished
			s_calclimitedwmState = 103;
			InterlockedExchange(&g_calclimitedwmflag, (LONG)0);
		}
	}
	if (s_calclimitedwmState == 101) {
		s_calclimitedwmState = 102;
		CalcLimitedWorldMat();
		*psavetime = curtime;
	}
	if (s_calclimitedwmState == 1001) {
		s_calclimitedwmState = 102;
		CalcLimitedWorldMat();
		g_btsimurecflag = true;//rec flag
		*psavetime = curtime;
	}

	return 0;
}


int OnFramePlayButton()
{


	if (s_firstkeyFlag){
		//先頭フレームへ
		s_firstkeyFlag = false;
		g_previewFlag = 0;
		//if (s_owpTimeline){
		//	s_owpLTimeline->setCurrentTime(1.0, true);
		//	s_owpEulerGraph->setCurrentTime(1.0, false);
		//}
		double newframe = 1.0;
		s_buttonselectstart = newframe;
		s_buttonselectend = newframe;
		OnTimeLineButtonSelectFromSelectStartEnd(0);
	}
	if (s_lastkeyFlag){
		//最終フレームへ
		s_lastkeyFlag = false;
		g_previewFlag = 0;
		//if (s_model){
		//	if (s_owpTimeline){
		//		MOTINFO* curmi = s_model->GetCurMotInfo();
		//		if (curmi){
		//			double lastframe = max(0, s_model->GetCurMotInfo()->frameleng - 1.0);
		//			s_owpLTimeline->setCurrentTime(lastframe, true);
		//			s_owpEulerGraph->setCurrentTime(lastframe, false);
		//		}
		//	}
		//}
		double newframe = 1.0;
		//if (s_owpLTimeline) {
		//	newframe = s_owpLTimeline->getMaxTime();
		//}
		if (s_model){
			if (s_owpTimeline){
				MOTINFO* curmi = s_model->GetCurMotInfo();
				if (curmi){
					newframe = max(0.0, s_model->GetCurMotInfo()->frameleng - 1.0);
				}
			}
		}
		s_buttonselectstart = newframe;
		s_buttonselectend = newframe;
		OnTimeLineButtonSelectFromSelectStartEnd(0);
	}
	return 0;
}


int OnFrameUndo(bool fromds, int fromdskind)
{
	bool undodoneflag = false;
	double tmpselectstart = 1.0;
	double tmpselectend = 1.0;
	double tmpapplyrate = 50.0;

	///////////// undo
	if (fromds || (s_model && g_controlkey && (g_keybuf['Z'] & 0x80) && !(g_savekeybuf['Z'] & 0x80))){

		StopBt();

		if ((fromds && (fromdskind == 1)) || (g_keybuf[VK_SHIFT] & 0x80)){
			//redo
			s_model->RollBackUndoMotion(1, &s_curboneno, &s_curbaseno, &tmpselectstart, &tmpselectend, &tmpapplyrate);//!!!!!!!!!!!
			undodoneflag = true;
		}
		else if((fromds && (fromdskind == 0)) || !fromds){
			//undo
			s_model->RollBackUndoMotion(0, &s_curboneno, &s_curbaseno, &tmpselectstart, &tmpselectend, &tmpapplyrate);//!!!!!!!!!!!
			undodoneflag = true;
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
				if (curmi) {
					if (curmi == s_model->GetCurMotInfo()) {
						findflag = 1;
						break;
					}
					chkcnt++;
				}
			}

			if (findflag == 1){
				int selindex;
				selindex = chkcnt;
				OnAnimMenu(true, selindex, 0);
			}
		}
		else{
			if (s_model) {
				//メニュー書き換え, timeline update
				OnAnimMenu(true, s_motmenuindexmap[s_model], 0);
			}
		}

		if (s_curboneno >= 0) {
			int curlineno = s_boneno2lineno[s_curboneno];
			if (s_owpTimeline) {
				s_owpTimeline->setCurrentLine(curlineno, true);
			}

			SetTimelineMark();
			SetLTimelineMark(s_curboneno);
		}


		//Sleep(500);
		//Sleep(30);
	}


	OnGUIEventSpeed();

	if (s_model && undodoneflag) {
		MOTINFO* curmi;
		curmi = s_model->GetCurMotInfo();
		if (curmi) {
			s_buttonselectstart = max(0.0, tmpselectstart);
			s_buttonselectstart = min((curmi->frameleng - 1.0), s_buttonselectstart);

			s_buttonselectend = max(0.0, tmpselectend);
			s_buttonselectend = min((curmi->frameleng - 1.0), s_buttonselectend);

			
			OnTimeLineButtonSelectFromSelectStartEnd(0);


			g_applyrate = (int)tmpapplyrate;
			if (g_SampleUI.GetSlider(IDC_SL_APPLYRATE)) {
				g_SampleUI.GetSlider(IDC_SL_APPLYRATE)->SetValue(g_applyrate);
			}

			int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
			if (result) {
				_ASSERT(0);
			}

			SetShowPosTime();

		}
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

int InitPluginMenu()
{

	//g_SampleUI.AddComboBox(IDC_COMBO_MOTIONBRUSH_METHOD, 35, iY += addh, ctrlxlen + 25, ctrlh);
	//s_ui_motionbrush = g_SampleUI.GetControl(IDC_COMBO_MOTIONBRUSH_METHOD);
	//_ASSERT(s_ui_motionbrush);
	//s_dsutgui0.push_back(s_ui_motionbrush);
	//s_dsutguiid0.push_back(IDC_COMBO_MOTIONBRUSH_METHOD);
	//CDXUTComboBox* pComboBox5 = g_SampleUI.GetComboBox(IDC_COMBO_MOTIONBRUSH_METHOD);
	//pComboBox5->RemoveAllItems();
	//pComboBox5->AddItem(L"Linear", ULongToPtr(0));
	//pComboBox5->AddItem(L"Cos(x+PI)", ULongToPtr(1));
	//pComboBox5->AddItem(L"Cos(x^2+PI)", ULongToPtr(2));
	//pComboBox5->AddItem(L"Rect", ULongToPtr(3));
	//pComboBox5->SetSelectedByData(ULongToPtr(0));


	s_plugin = new CPluginElem[MAXPLUGIN];
	if (!s_plugin) {
		DbgOut(L"viewer : InitPluginMenu : plugin alloc error !!!\n");
		_ASSERT(0);
		return 1;
	}

	///////////
	WCHAR plugindir[MAX_PATH];
	wcscpy_s(plugindir, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(plugindir, TEXT('\\'));
	if (!lasten) {
		_ASSERT(0);
		return S_FALSE;
	}
	*lasten = 0L;
	last2en = wcsrchr(plugindir, TEXT('\\'));
	if (!last2en) {
		_ASSERT(0);
		return S_FALSE;
	}
	*last2en = 0L;
	wcscat_s(plugindir, MAX_PATH, L"\\BrushesFolder\\");

	WCHAR finddir[_MAX_PATH];
	finddir[0] = 0;
	wcscpy_s(finddir, _MAX_PATH, plugindir);
	wcscat(finddir, L"*.dll");

	///////////
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(finddir, &FindFileData);
	int pluginno = 0;

	if (hFind != INVALID_HANDLE_VALUE) {
		WCHAR pluginpath[_MAX_PATH];
		wcscpy_s(pluginpath, _MAX_PATH, plugindir);
		wcscat(pluginpath, FindFileData.cFileName);
		DbgOut(L"InitPluginMenu : FindFirst : %s\r\n", FindFileData.cFileName);

		(s_plugin + pluginno)->SetFilePath(pluginpath);
		(s_plugin + pluginno)->LoadPlugin();
		pluginno++;

		BOOL bret = 1;
		while (bret != 0) {
			bret = FindNextFile(hFind, &FindFileData);
			if (bret != 0) {

				wcscpy_s(pluginpath, _MAX_PATH, plugindir);
				wcscat(pluginpath, FindFileData.cFileName);
				DbgOut(L"InitPluginMenu : FindNext : %s\r\n", FindFileData.cFileName);

				(s_plugin + pluginno)->SetFilePath(pluginpath);
				(s_plugin + pluginno)->LoadPlugin();

				pluginno++;
			}
		}

		FindClose(hFind);
	}

	//g_SampleUI.AddComboBox(IDC_COMBO_MOTIONBRUSH_METHOD, 35, iY += addh, ctrlxlen + 25, ctrlh);
	//s_ui_motionbrush = g_SampleUI.GetControl(IDC_COMBO_MOTIONBRUSH_METHOD);
	//_ASSERT(s_ui_motionbrush);
	//s_dsutgui0.push_back(s_ui_motionbrush);
	//s_dsutguiid0.push_back(IDC_COMBO_MOTIONBRUSH_METHOD);
	CDXUTComboBox* pComboBox5 = g_SampleUI.GetComboBox(IDC_COMBO_MOTIONBRUSH_METHOD);
	pComboBox5->RemoveAllItems();
	
	
	//pComboBox5->AddItem(L"Linear", ULongToPtr(0));
	//pComboBox5->AddItem(L"Cos(x+PI)", ULongToPtr(1));
	//pComboBox5->AddItem(L"Cos(x^2+PI)", ULongToPtr(2));
	//pComboBox5->AddItem(L"Rect", ULongToPtr(3));
	//pComboBox5->SetSelectedByData(ULongToPtr(0));
	int setno = 0;
	int firstmenuno = -1;
	for (pluginno = 0; pluginno < MAXPLUGIN; pluginno++) {
		if ((s_plugin + pluginno)->validflag == 1) {
			int menuid = setno;
			if (firstmenuno == -1) {
				firstmenuno = menuid;
			}
			pComboBox5->AddItem((s_plugin + pluginno)->pluginname, ULongToPtr(menuid));
			(s_plugin + pluginno)->menuid = menuid;
			setno++;
		}
	}
	pComboBox5->SetSelectedByData(ULongToPtr(firstmenuno));

	return 0;
}


int CreateUtDialog()
{

	// Initialize dialogs
	//g_SettingsDlg.Init(&g_DialogResourceManager);
	g_SampleUI.Init(&g_DialogResourceManager);

	int iY;
	g_SampleUI.SetCallback(OnGUIEvent); 
	//iY = 15;
	iY = 0;

	int ctrlh = 25;
	int addh = ctrlh + 2;

	int ctrlxlen = 120;
	int checkboxxlen = 120;

	WCHAR sz[100];
//################
//utguikind == 0
//################
	//iY += 24;

	int iX0;

	if (g_4kresolution) {
		//iY = s_mainheight - (520 - MAINMENUAIMBARH);
		//iY = s_mainheight - 210 - addh - 10;
		//iY = s_mainheight - 210 - 10;
		iY = s_mainheight - 210 - 10 - 3 * addh;
		iX0 = s_mainwidth / 2 - 180 - 2 * 180 - 30;
	}
	else {
		iY = 0;
		iX0 = 0;
	}


	s_dsutgui0.clear();
	s_dsutguiid0.clear();

	//swprintf_s(sz, 100, L"Light : %0.2f", g_fLightScale);
	//g_SampleUI.AddStatic(IDC_LIGHT_SCALE_STATIC, sz, 35, iY, ctrlxlen, ctrlh);
	//iY += addh;
	//g_SampleUI.AddSlider(IDC_LIGHT_SCALE, 50, iY += addh, 100, ctrlh, 0, 20, (int)(g_fLightScale * 10.0f));
	g_SampleUI.AddSlider(IDC_LIGHT_SCALE, iX0 + 50, iY, 100, ctrlh, 0, 20, (int)(g_fLightScale * 10.0f));
	s_ui_lightscale = g_SampleUI.GetControl(IDC_LIGHT_SCALE);
	_ASSERT(s_ui_lightscale);
	s_dsutgui0.push_back(s_ui_lightscale);
	s_dsutguiid0.push_back(IDC_LIGHT_SCALE);

	g_SampleUI.AddCheckBox(IDC_BMARK, L"DispBone", iX0 + 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_BoneMarkCheckBox);
	s_ui_dispbone = g_SampleUI.GetControl(IDC_BMARK);
	_ASSERT(s_ui_dispbone);
	s_dsutgui0.push_back(s_ui_dispbone);
	s_dsutguiid0.push_back(IDC_BMARK);
	g_SampleUI.AddCheckBox(IDC_RMARK, L"DispRigid", iX0 + 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_RigidMarkCheckBox);
	s_ui_disprigid = g_SampleUI.GetControl(IDC_RMARK);
	_ASSERT(s_ui_disprigid);
	s_dsutgui0.push_back(s_ui_disprigid);
	s_dsutguiid0.push_back(IDC_RMARK);


	s_savebonemarkflag = 1;
	s_saverigidmarkflag = 1;


	g_SampleUI.AddCheckBox(IDC_CAMTARGET, L"LockToSel", iX0 + 25, iY += addh, ctrlxlen, 16, false, 0U, false, &s_CamTargetCheckBox);
	s_ui_locktosel = g_SampleUI.GetControl(IDC_CAMTARGET);
	_ASSERT(s_ui_locktosel);
	s_dsutgui0.push_back(s_ui_locktosel);
	s_dsutguiid0.push_back(IDC_CAMTARGET);


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
	g_SampleUI.AddComboBox(IDC_COMBO_BONEAXIS, iX0 + 35, iY += addh, ctrlxlen, ctrlh);
	s_ui_boneaxis = g_SampleUI.GetControl(IDC_COMBO_BONEAXIS);
	_ASSERT(s_ui_boneaxis);
	s_dsutgui0.push_back(s_ui_boneaxis);
	s_dsutguiid0.push_back(IDC_COMBO_BONEAXIS);
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
	pComboBox3->SetSelectedByData(ULongToPtr((LONG)g_boneaxis));


	//g_SampleUI.AddComboBox(IDC_COMBO_BONE, 35, iY += addh, ctrlxlen, ctrlh);
	//s_ui_bone = g_SampleUI.GetControl(IDC_COMBO_BONE);
	//_ASSERT(s_ui_bone);
	//s_dsutgui0.push_back(s_ui_bone);
	//s_dsutguiid0.push_back(IDC_COMBO_BONE);

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



	//iY += addh;

	g_SampleUI.AddComboBox(IDC_COMBO_IKLEVEL, iX0 + 35, iY += addh, ctrlxlen, ctrlh);
	s_ui_iklevel = g_SampleUI.GetControl(IDC_COMBO_IKLEVEL);
	_ASSERT(s_ui_iklevel);
	s_dsutgui0.push_back(s_ui_iklevel);
	s_dsutguiid0.push_back(IDC_COMBO_IKLEVEL);
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

	if (g_4kresolution) {
		swprintf_s(sz, 100, L"ReferencePos : %d%%", g_refpos);
		//g_SampleUI.AddStatic(IDC_STATIC_IKRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
		g_SampleUI.AddStatic(IDC_STATIC_REF, sz, iX0 + 35, iY += addh, ctrlxlen, 18);
		s_ui_texref = g_SampleUI.GetControl(IDC_STATIC_REF);
		_ASSERT(s_ui_texref);
		//g_SampleUI.AddSlider(IDC_SL_IKRATE, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikrate * 100.0f));
		g_SampleUI.AddSlider(IDC_SL_REFPOS, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 100, g_refpos);
		s_ui_slirefpos = g_SampleUI.GetControl(IDC_SL_REFPOS);
		g_SampleUI.AddSlider(IDC_SL_REFMULT, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 100, g_refmult);
		s_ui_slirefmult = g_SampleUI.GetControl(IDC_SL_REFMULT);
		_ASSERT(s_ui_slirefmult);
		s_dsutgui0.push_back(s_ui_slirefpos);
		s_dsutguiid0.push_back(IDC_SL_REFPOS);
		s_dsutgui0.push_back(s_ui_slirefmult);
		s_dsutguiid0.push_back(IDC_SL_REFMULT);
	}



	if (g_4kresolution) {
		//iY = s_mainheight - (520 - MAINMENUAIMBARH);
		//iY = s_mainheight - 210 - 3 * addh - 10;
		//iY = s_mainheight - 210 - 4 * addh - 10;
		iY = s_mainheight - 210 - 2 * addh - 10;
		iX0 = s_mainwidth / 2 - 180 - 180 - 40;
	}
	else {
		//そのまま続き
		iY += addh;
		iX0 = 0;
	}


	g_SampleUI.AddComboBox(IDC_COMBO_MOTIONBRUSH_METHOD, iX0 + 35, iY, ctrlxlen + 25, ctrlh);
	s_ui_motionbrush = g_SampleUI.GetControl(IDC_COMBO_MOTIONBRUSH_METHOD);
	_ASSERT(s_ui_motionbrush);
	s_dsutgui0.push_back(s_ui_motionbrush);
	s_dsutguiid0.push_back(IDC_COMBO_MOTIONBRUSH_METHOD);
	InitPluginMenu();
	//CDXUTComboBox* pComboBox5 = g_SampleUI.GetComboBox(IDC_COMBO_MOTIONBRUSH_METHOD);
	//pComboBox5->RemoveAllItems();
	//pComboBox5->AddItem(L"Linear", ULongToPtr(0));
	//pComboBox5->AddItem(L"Cos(x+PI)", ULongToPtr(1));
	//pComboBox5->AddItem(L"Cos(x^2+PI)", ULongToPtr(2));
	//pComboBox5->AddItem(L"Rect", ULongToPtr(3));
	//pComboBox5->SetSelectedByData(ULongToPtr(0));



	swprintf_s(sz, 100, L"TopPos : %d%%", g_applyrate);
	//g_SampleUI.AddStatic(IDC_STATIC_APPLYRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_APPLYRATE, sz, iX0 + 35, iY += addh, ctrlxlen, 18);
	s_ui_texapplyrate = g_SampleUI.GetControl(IDC_STATIC_APPLYRATE);
	_ASSERT(s_ui_texapplyrate);
	//g_SampleUI.AddSlider(IDC_SL_APPLYRATE, 50, iY += addh, 100, ctrlh, 0, 100, g_applyrate);
	g_SampleUI.AddSlider(IDC_SL_APPLYRATE, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 100, g_applyrate);
	s_ui_slapplyrate = g_SampleUI.GetControl(IDC_SL_APPLYRATE);
	_ASSERT(s_ui_slapplyrate);
	CEditRange::SetApplyRate(g_applyrate);
	s_dsutgui0.push_back(s_ui_slapplyrate);
	s_dsutguiid0.push_back(IDC_SL_APPLYRATE);

	


	//swprintf_s( sz, 100, L"IK First Rate : %f", g_ikfirst );
	swprintf_s(sz, 100, L"Brush Repeats : %d", g_brushrepeats);
	//g_SampleUI.AddStatic(IDC_STATIC_BRUSHREPEATS, sz, 35, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_BRUSHREPEATS, sz, iX0 + 35, iY += addh, ctrlxlen, 18);
	s_ui_texbrushrepeats = g_SampleUI.GetControl(IDC_STATIC_BRUSHREPEATS);
	_ASSERT(s_ui_texbrushrepeats);
	//g_SampleUI.AddSlider(IDC_SL_BRUSHREPEATS, 50, iY += addh, 100, ctrlh, 0, 10, (int)g_brushrepeats);
	g_SampleUI.AddSlider(IDC_SL_BRUSHREPEATS, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 10, (int)g_brushrepeats);
	s_ui_brushrepeats = g_SampleUI.GetControl(IDC_SL_BRUSHREPEATS);
	_ASSERT(s_ui_brushrepeats);
	s_dsutgui0.push_back(s_ui_brushrepeats);
	s_dsutguiid0.push_back(IDC_SL_BRUSHREPEATS);

	g_SampleUI.AddCheckBox(IDC_BRUSH_MIRROR_U, L"Brsh Mirror U", iX0 + 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_BrushMirrorUCheckBox);
	s_ui_brushmirroru = g_SampleUI.GetControl(IDC_BRUSH_MIRROR_U);
	_ASSERT(s_ui_brushmirroru);
	s_dsutgui0.push_back(s_ui_brushmirroru);
	s_dsutguiid0.push_back(IDC_BRUSH_MIRROR_U);

	g_SampleUI.AddCheckBox(IDC_BRUSH_MIRROR_V, L"Brsh Mirror V", iX0 + 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_BrushMirrorVCheckBox);
	s_ui_brushmirrorv = g_SampleUI.GetControl(IDC_BRUSH_MIRROR_V);
	_ASSERT(s_ui_brushmirrorv);
	s_dsutgui0.push_back(s_ui_brushmirrorv);
	s_dsutguiid0.push_back(IDC_BRUSH_MIRROR_V);

	g_SampleUI.AddCheckBox(IDC_BRUSH_MIRROR_V_DIV2, L"If Mirror V then Div2", iX0 + 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_IfMirrorVDiv2CheckBox);
	s_ui_ifmirrorvdiv2 = g_SampleUI.GetControl(IDC_BRUSH_MIRROR_V_DIV2);
	_ASSERT(s_ui_ifmirrorvdiv2);
	s_dsutgui0.push_back(s_ui_ifmirrorvdiv2);
	s_dsutguiid0.push_back(IDC_BRUSH_MIRROR_V_DIV2);


	////swprintf_s( sz, 100, L"IK First Rate : %f", g_ikfirst );
	//swprintf_s(sz, 100, L"IK Order : %f", g_ikfirst);
	//g_SampleUI.AddStatic(IDC_STATIC_IKFIRST, sz, 35, iY += addh, ctrlxlen, ctrlh);
	//s_ui_texikorder = g_SampleUI.GetControl(IDC_STATIC_IKFIRST);
	//_ASSERT(s_ui_texikorder);
	//g_SampleUI.AddSlider(IDC_SL_IKFIRST, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikfirst * 25.0f));
	//s_ui_slikorder = g_SampleUI.GetControl(IDC_SL_IKFIRST);
	//_ASSERT(s_ui_slikorder);
	//s_dsutgui0.push_back(s_ui_slikorder);
	//s_dsutguiid0.push_back(IDC_SL_IKFIRST);



	//swprintf_s(sz, 100, L"IK Trans : %f", g_ikrate);
	////g_SampleUI.AddStatic(IDC_STATIC_IKRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
	//g_SampleUI.AddStatic(IDC_STATIC_IKRATE, sz, 35, iY += addh, ctrlxlen, 18);
	//s_ui_texikrate = g_SampleUI.GetControl(IDC_STATIC_IKRATE);
	//_ASSERT(s_ui_texikrate);
	////g_SampleUI.AddSlider(IDC_SL_IKRATE, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikrate * 100.0f));
	//g_SampleUI.AddSlider(IDC_SL_IKRATE, 50, iY += (18 + 2), 100, ctrlh, 0, 100, (int)(g_ikrate * 100.0f));
	//s_ui_slikrate = g_SampleUI.GetControl(IDC_SL_IKRATE);
	//_ASSERT(s_ui_slikrate);
	//s_dsutgui0.push_back(s_ui_slikrate);
	//s_dsutguiid0.push_back(IDC_SL_IKRATE);

	if (g_4kresolution == false) {
		swprintf_s(sz, 100, L"ReferencePos : %d%%", g_refpos);
		//g_SampleUI.AddStatic(IDC_STATIC_IKRATE, sz, 35, iY += addh, ctrlxlen, ctrlh);
		g_SampleUI.AddStatic(IDC_STATIC_REF, sz, iX0 + 35, iY += addh, ctrlxlen, 18);
		s_ui_texref = g_SampleUI.GetControl(IDC_STATIC_REF);
		_ASSERT(s_ui_texref);
		//g_SampleUI.AddSlider(IDC_SL_IKRATE, 50, iY += addh, 100, ctrlh, 0, 100, (int)(g_ikrate * 100.0f));
		g_SampleUI.AddSlider(IDC_SL_REFPOS, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 100, g_refpos);
		s_ui_slirefpos = g_SampleUI.GetControl(IDC_SL_REFPOS);
		g_SampleUI.AddSlider(IDC_SL_REFMULT, iX0 + 50, iY += (18 + 2), 100, ctrlh, 0, 100, g_refmult);
		s_ui_slirefmult = g_SampleUI.GetControl(IDC_SL_REFMULT);
		_ASSERT(s_ui_slirefmult);
		s_dsutgui0.push_back(s_ui_slirefpos);
		s_dsutguiid0.push_back(IDC_SL_REFPOS);
		s_dsutgui0.push_back(s_ui_slirefmult);
		s_dsutguiid0.push_back(IDC_SL_REFMULT);
	}

	//g_SampleUI.AddCheckBox(IDC_APPLY_TO_THEEND, L"ApplyToTheEnd", 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_ApplyEndCheckBox);
	//s_ui_applytotheend = g_SampleUI.GetControl(IDC_APPLY_TO_THEEND);
	//_ASSERT(s_ui_applytotheend);
	//s_dsutgui0.push_back(s_ui_applytotheend);
	//s_dsutguiid0.push_back(IDC_APPLY_TO_THEEND);

	//g_SampleUI.AddCheckBox(IDC_SLERP_OFF, L"SlerpIKOff", 25, iY += addh, checkboxxlen, 16, false, 0U, false, &s_SlerpOffCheckBox);
	//s_ui_slerpoff = g_SampleUI.GetControl(IDC_SLERP_OFF);
	//_ASSERT(s_ui_slerpoff);
	//s_dsutgui0.push_back(s_ui_slerpoff);
	//s_dsutguiid0.push_back(IDC_SLERP_OFF);
	//	g_SampleUI.AddCheckBox(IDC_PSEUDOLOCAL, L"PseudoLocal(疑似ローカル)", 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_PseudoLocalCheckBox);
//	g_SampleUI.AddCheckBox(IDC_LIMITDEG, L"回転角度制限をする", 25, iY += addh, checkboxxlen, 16, true, 0U, false, &s_LimitDegCheckBox);


//################
//utguikind == 1
//################
	//Left Bottom
	s_dsutgui1.clear();
	s_dsutguiid1.clear();

	iY = s_mainheight - 210;
	int startx = s_mainwidth / 2 - 180;

	swprintf_s(sz, 100, L"Speed: %0.2f", g_dspeed);
	//g_SampleUI.AddStatic(IDC_SPEED_STATIC, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_SPEED_STATIC, sz, startx, iY += addh, ctrlxlen, 18);
	s_ui_texspeed = g_SampleUI.GetControl(IDC_SPEED_STATIC);
	_ASSERT(s_ui_texspeed);
	//g_SampleUI.AddSlider(IDC_SPEED, startx, iY += addh, 100, ctrlh, 0, 700, (int)(g_dspeed * 100.0f));
	g_SampleUI.AddSlider(IDC_SPEED, startx, iY += (18 + 2), 100, ctrlh, 0, 700, (int)(g_dspeed * 100.0f));
	s_ui_speed = g_SampleUI.GetControl(IDC_SPEED);
	_ASSERT(s_ui_speed);
	s_dsutgui1.push_back(s_ui_speed);//!!!!!!!!!!!!!!!! dsutgui1
	s_dsutguiid1.push_back(IDC_SPEED);

	//g_SampleUI.AddCheckBox(IDC_PSEUDOLOCAL, L"PseudoLocal", startx, iY += addh, checkboxxlen, 16, true, 0U, false, &s_PseudoLocalCheckBox);
	//s_ui_pseudolocal = g_SampleUI.GetControl(IDC_PSEUDOLOCAL);
	//_ASSERT(s_ui_pseudolocal);
	//s_dsutgui1.push_back(s_ui_pseudolocal);
	//s_dsutguiid1.push_back(IDC_PSEUDOLOCAL);

	g_SampleUI.AddCheckBox(IDC_WALLSCRAPINGIK, L"WallScrapingIK", startx, iY += addh, checkboxxlen, 16, (g_wallscrapingikflag == 1), 0U, false, &s_WallScrapingIKCheckBox);
	s_ui_wallscrapingik = g_SampleUI.GetControl(IDC_WALLSCRAPINGIK);
	_ASSERT(s_ui_wallscrapingik);
	s_dsutgui1.push_back(s_ui_wallscrapingik);
	s_dsutguiid1.push_back(IDC_WALLSCRAPINGIK);


	//g_SampleUI.AddCheckBox(IDC_LIMITDEG, L"LimitEul", startx, iY += addh, checkboxxlen, 16, true, 0U, false, &s_LimitDegCheckBox);
	g_SampleUI.AddCheckBox(IDC_LIMITDEG, L"LimitEul", startx, iY += addh, checkboxxlen, 16, g_limitdegflag, 0U, false, &s_LimitDegCheckBox);
	s_ui_limiteul = g_SampleUI.GetControl(IDC_LIMITDEG);
	_ASSERT(s_ui_limiteul);
	s_dsutgui1.push_back(s_ui_limiteul);
	s_dsutguiid1.push_back(IDC_LIMITDEG);
	g_SampleUI.AddCheckBox(IDC_ABS_IK, L"AbsIKOn", startx, iY += addh, checkboxxlen, 16, false, 0U, false, &s_AbsIKCheckBox);
	s_ui_absikon = g_SampleUI.GetControl(IDC_ABS_IK);
	_ASSERT(s_ui_absikon);
	s_dsutgui1.push_back(s_ui_absikon);
	s_dsutguiid1.push_back(IDC_ABS_IK);

//################
//utguikind == 2
//################
	//CenterRight Bottom
	s_dsutgui2.clear();
	s_dsutguiid2.clear();

	//Center Bottom
	iY = s_mainheight - 210;
	startx = s_mainwidth / 2 - 50;

	int addh2 = 27;

	//iY += 10;
	g_SampleUI.AddButton(IDC_BTSTART, L"BT start", startx, iY += addh, 100, ctrlh);
	s_ui_btstart = g_SampleUI.GetControl(IDC_BTSTART);
	_ASSERT(s_ui_btstart);
	s_dsutgui2.push_back(s_ui_btstart);
	s_dsutguiid2.push_back(IDC_BTSTART);

	//iY += 3;
	g_SampleUI.AddButton(IDC_BTRECSTART, L"BT REC", startx, iY += addh2, 100, ctrlh);
	s_ui_btrecstart = g_SampleUI.GetControl(IDC_BTRECSTART);
	_ASSERT(s_ui_btrecstart);
	s_dsutgui2.push_back(s_ui_btrecstart);
	s_dsutguiid2.push_back(IDC_BTRECSTART);

	//iY += 3;
	g_SampleUI.AddButton(IDC_STOP_BT, L"STOP BT", startx, iY += addh2, 100, ctrlh);
	s_ui_stopbt = g_SampleUI.GetControl(IDC_STOP_BT);
	_ASSERT(s_ui_stopbt);
	s_dsutgui2.push_back(s_ui_stopbt);
	s_dsutguiid2.push_back(IDC_STOP_BT);

	swprintf_s(sz, 100, L"ThreadNum : %d(%d)", g_numthread, gNumIslands);
	//g_SampleUI.AddStatic(IDC_STATIC_NUMTHREAD, sz, startx, iY += addh2, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_NUMTHREAD, sz, startx, iY += addh2, ctrlxlen, 18);
	s_ui_texthreadnum = g_SampleUI.GetControl(IDC_STATIC_NUMTHREAD);
	_ASSERT(s_ui_texthreadnum);
	//g_SampleUI.AddSlider(IDC_SL_NUMTHREAD, startx, iY += addh2, 100, ctrlh, 1, 4, g_numthread);
	g_SampleUI.AddSlider(IDC_SL_NUMTHREAD, startx, iY += (18 + 2), 100, ctrlh, 1, 4, g_numthread);
	s_ui_slthreadnum = g_SampleUI.GetControl(IDC_SL_NUMTHREAD);
	_ASSERT(s_ui_slthreadnum);
	s_dsutgui2.push_back(s_ui_slthreadnum);
	s_dsutguiid2.push_back(IDC_SL_NUMTHREAD);


	iY = s_mainheight - 210;
	startx = s_mainwidth / 2 - 50 + 130;

	swprintf_s(sz, 100, L"BT CalcCnt: %0.2f", g_btcalccnt);
	//g_SampleUI.AddStatic(IDC_STATIC_BTCALCCNT, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_BTCALCCNT, sz, startx, iY += addh, ctrlxlen, 18);
	s_ui_texbtcalccnt = g_SampleUI.GetControl(IDC_STATIC_BTCALCCNT);
	_ASSERT(s_ui_texbtcalccnt);
	//g_SampleUI.AddSlider(IDC_BTCALCCNT, startx, iY += addh, 100, ctrlh, 1, 100, (int)(g_btcalccnt));
	g_SampleUI.AddSlider(IDC_BTCALCCNT, startx, iY += (18 + 2), 100, ctrlh, 1, 100, (int)(g_btcalccnt));
	s_ui_btcalccnt = g_SampleUI.GetControl(IDC_BTCALCCNT);
	_ASSERT(s_ui_btcalccnt);
	s_dsutgui2.push_back(s_ui_btcalccnt);
	s_dsutguiid2.push_back(IDC_BTCALCCNT);

	swprintf_s(sz, 100, L"BT ERP: %0.5f", g_erp);
	//g_SampleUI.AddStatic(IDC_STATIC_ERP, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_ERP, sz, startx, iY += addh, ctrlxlen, 18);
	s_ui_texerp = g_SampleUI.GetControl(IDC_STATIC_ERP);
	_ASSERT(s_ui_texerp);
	//g_SampleUI.AddSlider(IDC_ERP, startx, iY += addh, 100, ctrlh, 0, 5000, (int)(g_erp * 5000.0 + 0.4));
	g_SampleUI.AddSlider(IDC_ERP, startx, iY += (18 + 2), 100, ctrlh, 0, 5000, (int)(g_erp * 5000.0 + 0.4));
	s_ui_erp = g_SampleUI.GetControl(IDC_ERP);
	_ASSERT(s_ui_erp);
	s_dsutgui2.push_back(s_ui_erp);
	s_dsutguiid2.push_back(IDC_ERP);



//################
//utguikind == 3
//################
	//Right Bottom
	s_dsutgui3.clear();
	s_dsutguiid3.clear();

	if (g_4kresolution) {
		iY = s_mainheight - 210 - addh;
		startx = s_mainwidth / 2 - 50 + 130 + 140;
	}
	else {
		iY = s_mainheight - 210 - addh;
		startx = s_mainwidth - 150;
	}

	swprintf_s(sz, 100, L"EditRate : %.3f", g_physicsmvrate);
	//g_SampleUI.AddStatic(IDC_STATIC_PHYSICS_MV_SLIDER, sz, startx, iY += addh, ctrlxlen, ctrlh);
	g_SampleUI.AddStatic(IDC_STATIC_PHYSICS_MV_SLIDER, sz, startx, iY += addh, ctrlxlen, 18);
	s_ui_texphysmv = g_SampleUI.GetControl(IDC_STATIC_PHYSICS_MV_SLIDER);
	_ASSERT(s_ui_texphysmv);
	//g_SampleUI.AddSlider(IDC_PHYSICS_MV_SLIDER, startx, iY += addh, 100, ctrlh, 0, 100, (int)(g_physicsmvrate * 100.0f));
	g_SampleUI.AddSlider(IDC_PHYSICS_MV_SLIDER, startx, iY += (18 + 2), 100, ctrlh, 0, 100, (int)(g_physicsmvrate * 100.0f));
	s_ui_slphysmv = g_SampleUI.GetControl(IDC_PHYSICS_MV_SLIDER);
	_ASSERT(s_ui_slphysmv);
	s_dsutgui3.push_back(s_ui_slphysmv);
	s_dsutguiid3.push_back(IDC_PHYSICS_MV_SLIDER);


	if (g_usephysik == 1) {//MotionBrush%d.iniファイルを編集してUsePhysIKに１を指定した時のみのオプション機能（乱れやすい。少しだけ動かして物理的なノイズを加える位の役には立つかもしれないのでオプションとして残す。）
		iY += 10;
		g_SampleUI.AddButton(IDC_PHYSICS_IK, L"PhysRotStart", startx, iY += addh, 100, ctrlh);
		s_ui_physrotstart = g_SampleUI.GetControl(IDC_PHYSICS_IK);
		_ASSERT(s_ui_physrotstart);
		s_dsutgui3.push_back(s_ui_physrotstart);
		s_dsutguiid3.push_back(IDC_PHYSICS_IK);
		iY += 5;
		g_SampleUI.AddButton(IDC_PHYSICS_MV_IK, L"PhysMvStart", startx, iY += addh, 100, ctrlh);
		s_ui_physmvstart = g_SampleUI.GetControl(IDC_PHYSICS_MV_IK);
		_ASSERT(s_ui_physmvstart);
		s_dsutgui3.push_back(s_ui_physmvstart);
		s_dsutguiid3.push_back(IDC_PHYSICS_MV_IK);
		iY += 5;
		g_SampleUI.AddButton(IDC_PHYSICS_IK_STOP, L"PhysIkStop", startx, iY += addh, 100, ctrlh);
		s_ui_physikstop = g_SampleUI.GetControl(IDC_PHYSICS_IK_STOP);
		_ASSERT(s_ui_physikstop);
		s_dsutgui3.push_back(s_ui_physikstop);
		s_dsutguiid3.push_back(IDC_PHYSICS_IK_STOP);
	}


	//iY += 5;
	//g_SampleUI.AddButton(IDC_APPLY_BT, L"Apply BT", startx, iY += addh, 100, ctrlh);


	return 0;

}

int CreateTimelineWnd()
{

	s_rctreewnd.top = MAINMENUAIMBARH;
	s_rctreewnd.left = 0;


	s_timelineWnd = new OrgWindow(
		0,
		L"TimeLine",				//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos(0, 0),		//位置
		//WindowSize(400, 600),	//サイズ
		WindowPos(0, MAINMENUAIMBARH),		//位置
		WindowSize(s_timelinewidth, s_timelineheight),	//サイズ 
		//WindowSize(150,540),	//サイズ
		L"TimeLine",				//タイトル
		s_mainhwnd,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70);				//カラー
		0, 0, 0);				//カラー

	s_rctreewnd.right = s_timelinewidth;
	s_rctreewnd.bottom = s_timelineheight;

	s_timelineWnd->callRewrite();


	// ウィンドウの閉じるボタンのイベントリスナーに
	// 終了フラグcloseFlagをオンにするラムダ関数を登録する
	s_timelineWnd->setCloseListener([]() {
		if (s_model) {
			s_closeFlag = true;
		}
	});


	// ウィンドウのキーボードイベントリスナーに
	// コピー/カット/ペーストフラグcopyFlag/cutFlag/pasteFlagをオンにするラムダ関数を登録する
	// コピー等のキーボードを使用する処理はキーボードイベントリスナーを使用しなくても
	// メインループ内でマイフレームキー状態を監視することで作成可能である。
	s_timelineWnd->setKeyboardEventListener([](const KeyboardEvent &e){
		if (s_model) {
			if (e.ctrlKey && !e.repeat && e.onDown) {
				switch (e.keyCode) {
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
					//g_previewFlag = 1;
					s_calclimitedwmState = 1;
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
		}
	});


	return 0;
}

int CreateLongTimelineWnd()
{

	//////////
	///////// Long Timeline

	s_rcltwnd.top = 0;
	s_rcltwnd.left = 0;

	s_LtimelineWnd = new OrgWindow(
		0,
		L"EditRangeTimeLine",				//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos( 250, 825 ),		//位置
		//WindowPos(200, 645),		//位置
		WindowPos(s_toolwidth, s_2ndposy),		//位置
		WindowSize(s_longtimelinewidth, s_longtimelineheight),	//サイズ
		L"EditRangeTimeLine",				//タイトル
		s_mainhwnd,					//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70);				//カラー
		0, 0, 0);				//カラー

	s_rcltwnd.bottom = s_longtimelineheight;
	s_rcltwnd.right = s_longtimelinewidth;


	s_LtimelineWnd->callRewrite();


	/////////
	s_owpPlayerButton = new OWP_PlayerButton;
	s_owpPlayerButton->setButtonSize(20);
	s_LtimelineWnd->addParts(*s_owpPlayerButton);//owp_timelineより前

	s_owpPlayerButton->setFrontPlayButtonListener([]() { 
		if (s_model) {
			s_calclimitedwmState = 1; s_LstartFlag = true; s_LcursorFlag = true;;
		}
	});
	s_owpPlayerButton->setBackPlayButtonListener([](){  
		if (s_model) {
			s_calclimitedwmState = 11; s_LstartFlag = true; s_LcursorFlag = true;
		}
	});
	

	s_owpPlayerButton->setFrontStepButtonListener([](){ 
		if (s_model) {
			s_LstartFlag = true; s_LcursorFlag = true; s_lastkeyFlag = true;
		}
	});
	s_owpPlayerButton->setBackStepButtonListener([]() {
		if (s_model) {
			s_LstartFlag = true; s_LcursorFlag = true; s_firstkeyFlag = true;
		}
	});


	s_owpPlayerButton->setOneFpsButtonListener([]() {
		if (s_model) {
			int tmponefps;
			if (s_onefps == 0) {
				tmponefps = 1;
			}
			else if (s_onefps == 1) {
				tmponefps = 2;
			}
			else if (s_onefps == 2) {
				tmponefps = 0;
			}
			else {
				tmponefps = 0;
			}
			s_onefps = tmponefps;
		}
	});
	s_owpPlayerButton->setStopButtonListener([]() {  		
		if (s_model) {
			s_LstopFlag = true; s_LcursorFlag = true; g_previewFlag = 0;
		}
	});
	s_owpPlayerButton->setResetButtonListener([](){ 
		if (s_model) {
			if (s_owpLTimeline) {
				s_LstopFlag = true; g_previewFlag = 0;  s_LcursorFlag = true; s_owpLTimeline->setCurrentTime(1.0, true); s_owpEulerGraph->setCurrentTime(1.0, false);
			}
		}
	});
	s_owpPlayerButton->setSelectToLastButtonListener([](){  
		if (s_model) {
			g_underselecttolast = true;  s_LcursorFlag = true; g_selecttolastFlag = true;
		}
	});
	s_owpPlayerButton->setBtResetButtonListener([](){  
		if (s_model) {
			s_btresetFlag = true;
			//StartBt(s_model, TRUE, 0, 1);
		}
	});
	s_owpPlayerButton->setPrevRangeButtonListener([](){  
		if (s_model) {
			g_undereditrange = true; s_prevrangeFlag = true;
		}
	});
	s_owpPlayerButton->setNextRangeButtonListener([](){  
		if (s_model) {
			g_undereditrange = true; s_nextrangeFlag = true;
		}
	});
	s_owpPlayerButton->setPlusDispButtonListener([]() {
		if (s_model && s_owpEulerGraph) {
			s_owpEulerGraph->PlusDisp();
			s_owpEulerGraph->MinusOffset();//上に動かすにはオフセットを減らす
			s_owpEulerGraph->MinusOffset();//上に動かすにはオフセットを減らす
			s_owpEulerGraph->MinusOffset();//上に動かすにはオフセットを減らす
			//s_owpEulerGraph->MinusOffset();//上に動かすにはオフセットを減らす
		}
	});
	s_owpPlayerButton->setMinusDispButtonListener([]() {
		if (s_model && s_owpEulerGraph) {
			s_owpEulerGraph->MinusDisp();
			s_owpEulerGraph->PlusOffset();//下に動かすにはオフセットを増やす
			s_owpEulerGraph->PlusOffset();//下に動かすにはオフセットを増やす
			s_owpEulerGraph->PlusOffset();//下に動かすにはオフセットを増やす
			//s_owpEulerGraph->PlusOffset();//下に動かすにはオフセットを増やす
		}
	});
	s_owpPlayerButton->setPlusOffsetDispButtonListener([]() {
		if (s_model && s_owpEulerGraph) {
			s_owpEulerGraph->MinusOffset();//上に動かすにはオフセットを減らす
		}
	});
	s_owpPlayerButton->setMinusOffsetDispButtonListener([]() {
		if (s_model && s_owpEulerGraph) {
			s_owpEulerGraph->PlusOffset();//下に動かすにはオフセットを増やす
		}
	});
	s_owpPlayerButton->setResetDispButtonListener([]() {
		if (s_model && s_owpEulerGraph) {
			s_owpEulerGraph->ResetScaleAndOffset();
		}
	});


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
	s_LtimelineWnd->setCloseListener([](){ 
		if (s_model) {
			s_LcloseFlag = true;
		}
	});
	s_LtimelineWnd->setLDownListener([]() { 
		if (s_model) {
			g_underselectingframe = 1;
		}
	});
	s_LtimelineWnd->setLUpListener([](){
		if (s_model) {
			if (s_owpLTimeline) {
				if (g_previewFlag == 0) {
					if (s_prevrangeFlag || s_nextrangeFlag) {
						RollBackEditRange(s_prevrangeFlag, s_nextrangeFlag);
						s_buttonselectstart = s_editrange.GetStartFrame();
						s_buttonselectend = s_editrange.GetEndFrame();

						g_underselectingframe = 0;

						if (s_editmotionflag < 0) {
							int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
							if (result) {
								_ASSERT(0);
							}
						}

						OnTimeLineButtonSelectFromSelectStartEnd(0);
					}
					else if (g_selecttolastFlag == false) {

						if (!s_LstopFlag) {
							if (s_selectFlag) {
								s_selectFlag = false;
								s_selectKeyInfoList.clear();
								s_selectKeyInfoList = s_owpLTimeline->getSelectedKey();
								s_editrange.SetRange(s_selectKeyInfoList, s_owpLTimeline->getCurrentTime());
								s_buttonselectstart = s_editrange.GetStartFrame();
								s_buttonselectend = s_editrange.GetEndFrame();
								g_underselectingframe = 0;
								//_ASSERT(0);
							}
							else {
								s_buttonselectstart = s_owpLTimeline->getCurrentTime();
								s_buttonselectend = s_owpLTimeline->getCurrentTime();
								g_underselectingframe = 0;
								//_ASSERT(0);
							}

							if (s_editmotionflag < 0) {
								int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
								if (result) {
									_ASSERT(0);
								}
							}

							OnTimeLineButtonSelectFromSelectStartEnd(0);
						}
						else {
							//停止ボタンが押されたとき
							//_ASSERT(0);
							s_buttonselectstart = s_editrange.GetStartFrame();
							s_buttonselectend = s_editrange.GetEndFrame();
							g_underselectingframe = 0;
							//_ASSERT(0);

							//int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
							//if (result) {
							//	_ASSERT(0);
							//}

							OnTimeLineButtonSelectFromSelectStartEnd(0);
							//_ASSERT(0);
						}

					}
					else {
						//ToTheLastFrame
						OnTimeLineButtonSelectFromSelectStartEnd(1);

						s_buttonselectstart = s_editrange.GetStartFrame();
						s_buttonselectend = s_editrange.GetEndFrame();

						if (s_editmotionflag < 0) {
							int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
							if (result) {
								_ASSERT(0);
							}
						}

						g_underselectingframe = 0;//!!!! 2021/06/18
					}


				}
				else {
					//再生ボタンが押されたとき
					//_ASSERT(0);
					s_buttonselectstart = s_editrange.GetStartFrame();
					s_buttonselectend = s_editrange.GetEndFrame();
					g_underselectingframe = 0;
					//_ASSERT(0);

					int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
					if (result) {
						_ASSERT(0);
					}

					OnTimeLineButtonSelectFromSelectStartEnd(0);

				}
			}

			s_LstartFlag = false;
			s_LstopFlag = false;
			g_selecttolastFlag = false;
			s_prevrangeFlag = false;
			s_nextrangeFlag = false;
		}

	});


	s_LtimelineWnd->setPos(WindowPos(s_toolwidth, s_2ndposy));
	s_LtimelineWnd->setSize(WindowSize(s_longtimelinewidth, s_longtimelineheight));
	s_LtimelineWnd->callRewrite();

	return 0;
}

int CreateDmpAnimWnd()
{

	s_dsdampctrls.clear();

	/////////

	s_dmpanimWnd = new OrgWindow(
		0,
		_T("dampAnimWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth, s_sidemenuheight),
		WindowSize(s_sidewidth, s_sideheight),		//サイズ
		_T("AnimOfDumping"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_dmpgroupcheck = new OWP_CheckBoxA(L"SetToAllRigidsMeansToSetToSameGroup", 0);
	s_dmpanimLlabel = new OWP_Label(L"posSpringDumpingPerFrame");
	s_dmpanimLSlider = new OWP_Slider(0.0, 1.0, 0.0);
	s_dmpanimAlabel = new OWP_Label(L"rotSpringDumpingPerFrame");
	s_dmpanimASlider = new OWP_Slider(0.0, 1.0, 0.0);
	s_dmpanimB = new OWP_Button(L"SetToAllRigids");

	int slw2 = 500;
	s_dmpanimLSlider->setSize(WindowSize(slw2, 40));
	s_dmpanimASlider->setSize(WindowSize(slw2, 40));

	s_dmpanimWnd->addParts(*s_dmpgroupcheck);
	s_dmpanimWnd->addParts(*s_dmpanimLlabel);
	s_dmpanimWnd->addParts(*s_dmpanimLSlider);
	s_dmpanimWnd->addParts(*s_dmpanimAlabel);
	s_dmpanimWnd->addParts(*s_dmpanimASlider);
	s_dmpanimWnd->addParts(*s_dmpanimB);


	s_dsdampctrls.push_back(s_dmpgroupcheck);
	s_dsdampctrls.push_back(s_dmpanimLlabel);
	s_dsdampctrls.push_back(s_dmpanimLSlider);
	s_dsdampctrls.push_back(s_dmpanimAlabel);
	s_dsdampctrls.push_back(s_dmpanimASlider);
	s_dsdampctrls.push_back(s_dmpanimB);


	s_dmpanimWnd->setCloseListener([](){ 
		if (s_model) {
			s_DcloseFlag = true;
		}
	});

	s_dmpanimLSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRgdRigidElem(s_rgdindexmap[s_model], s_curboneno);
			if (curre) {
				float val = (float)s_dmpanimLSlider->getValue();
				curre->SetDampanimL(val);
			}
			s_dmpanimWnd->callRewrite();						//再描画
		}
	});
	s_dmpanimASlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRgdRigidElem(s_rgdindexmap[s_model], s_curboneno);
			if (curre) {
				float val = (float)s_dmpanimASlider->getValue();
				curre->SetDampanimA(val);
			}
			s_dmpanimWnd->callRewrite();						//再描画
		}
	});
	s_dmpanimB->setButtonListener([](){
		if (s_model && (s_rgdindexmap[s_model] >= 0)){
			float valL = (float)s_dmpanimLSlider->getValue();
			float valA = (float)s_dmpanimASlider->getValue();
			int chkg = (int)s_dmpgroupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRgdRigidElem(s_rgdindexmap[s_model], s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllDampAnimData(gid, s_rgdindexmap[s_model], valL, valA);
		}
	});

	s_dmpanimWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_dmpanimWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));
	s_dmpanimWnd->callRewrite();

	return 0;
}

int CreateMainMenuAimBarWnd()
{
	s_mainmenuaimbarWnd = new OrgWindow(
		0,
		_T("MainMenuAimBarWnd"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
								//WindowPos(100, 200),		//位置
		WindowPos(0, 0),
		//WindowSize(450,880),		//サイズ
		//WindowSize(450,680),		//サイズ
		//WindowSize(450, 760),		//サイズ
		WindowSize(s_mainwidth + s_timelinewidth + 16, MAINMENUAIMBARH),		//サイズ
		_T("MainMenuAimBarWnd"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true, true);					//サイズ変更の可否



	//s_sidemenuWnd->setCloseListener([]() { s_ScloseFlag = true; });

	s_mainmenuaimbarWnd->setBlackTheme();


	s_mainmenulabel = new OWP_Label(L".");
	s_mainmenuaimbarWnd->addParts(*s_mainmenulabel);


	//450, 32
	s_rcmainmenuaimbarwnd.top = 0;
	s_rcmainmenuaimbarwnd.left = 0;
	s_rcmainmenuaimbarwnd.bottom = MAINMENUAIMBARH;


	s_mainmenuaimbarWnd->setPos(WindowPos(0, 0));
	//if (g_4kresolution) {
	//	s_mainmenuaimbarWnd->setSize(WindowSize(1200 * 2, MAINMENUAIMBARH));
	//	s_rcmainmenuaimbarwnd.right = 1200 * 2;
	//}
	//else {
	//	s_mainmenuaimbarWnd->setSize(WindowSize(1200, MAINMENUAIMBARH));
	//	s_rcmainmenuaimbarwnd.right = 1200;
	//}
	s_mainmenuaimbarWnd->setSize(WindowSize(s_mainwidth + s_timelinewidth + 16, MAINMENUAIMBARH));
	s_rcmainmenuaimbarwnd.right = s_mainwidth + s_timelinewidth + 16;

	s_mainmenuaimbarWnd->callRewrite();						//再描画


	return 0;

}

int CreateSideMenuWnd()
{
	s_sidemenuWnd = new OrgWindow(
		0,
		_T("SideMenuWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
								//WindowPos(100, 200),		//位置
		WindowPos(0, 0),
		//WindowSize(450,880),		//サイズ
		//WindowSize(450,680),		//サイズ
		//WindowSize(450, 760),		//サイズ
		WindowSize(s_sidemenuwidth, s_sidemenuheight),		//サイズ
		_T("SideMenu"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true, true);					//サイズ変更の可否


	s_sidemenusp = new OWP_Separator(s_sidemenuWnd, true, 0.5, true);
	s_sidemenuWnd->addParts(*s_sidemenusp);

	s_sidemenusp1 = new OWP_Separator(s_sidemenuWnd, true, 0.5, true);
	s_sidemenusp2 = new OWP_Separator(s_sidemenuWnd, true, 0.5, true);
	s_sidemenusp->addParts1(*s_sidemenusp1);
	s_sidemenusp->addParts2(*s_sidemenusp2);

	s_sidemenu_rigid = new OWP_Button(L"Rigid");
	s_sidemenu_limiteul = new OWP_Button(L"LimitEul");
	s_sidemenu_copyhistory = new OWP_Button(L"CopyHistory");
	s_sidemenu_retarget = new OWP_Button(L"Retarget");

	s_sidemenusp1->addParts1(*s_sidemenu_rigid);
	s_sidemenusp1->addParts2(*s_sidemenu_limiteul);

	s_sidemenusp2->addParts1(*s_sidemenu_copyhistory);
	s_sidemenusp2->addParts2(*s_sidemenu_retarget);


	s_sidemenuWnd->setCloseListener([]() { 
		if (s_model) {
			s_ScloseFlag = true;
		}
	});

	s_sidemenu_rigid->setButtonListener([]() {
		if (s_model && (s_curboneno >= 0)) {
			s_platemenukind = 1;
			GUIMenuSetVisible(s_platemenukind, 1);
		}
		s_sidemenuWnd->callRewrite();						//再描画
	});
	s_sidemenu_limiteul->setButtonListener([]() {
		if (s_model && (s_curboneno >= 0)) {
			s_platemenukind = 2;
			GUIMenuSetVisible(s_platemenukind, 2);
		}
		s_sidemenuWnd->callRewrite();						//再描画
	});
	s_sidemenu_copyhistory->setButtonListener([]() {
		if (s_model && (s_curboneno >= 0)) {
			s_selCopyHisotryFlag = true;
		}
		s_sidemenuWnd->callRewrite();						//再描画
	});
	s_sidemenu_retarget->setButtonListener([]() {
		if (s_model && (s_curboneno >= 0)) {
			s_platemenukind = 2;
			GUIMenuSetVisible(s_platemenukind, 1);
		}
		s_sidemenuWnd->callRewrite();						//再描画
	});



	s_sidemenuWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, 0));

	//450, 32
	s_rcsidemenuwnd.top = 0;
	s_rcsidemenuwnd.left = 0;
	s_rcsidemenuwnd.bottom = s_sidemenuheight;
	s_rcsidemenuwnd.right = s_sidemenuwidth;

	s_sidemenuWnd->callRewrite();						//再描画


	return 0;
}

int CreatePlaceFolderWnd()
{


	s_placefolderWnd = new OrgWindow(
		0,
		_T("PlaceFolderWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight),
		WindowSize(s_sidewidth, s_sideheight),		//サイズ
		_T("PlaceFolderWindow"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true, true);					//サイズ変更の可否

	s_placefolderlabel_1 = new OWP_Label(L"After Loading Model Data,");
	s_placefolderlabel_2 = new OWP_Label(L"Click Frog Button, Change Plate Menu,");
	s_placefolderlabel_3 = new OWP_Label(L"MainWindowGUI Menu->RigidBody Menu->Retarget Menu->MainWindowGUI Menu");

	s_placefolderWnd->addParts(*s_placefolderlabel_1);
	s_placefolderWnd->addParts(*s_placefolderlabel_2);
	s_placefolderWnd->addParts(*s_placefolderlabel_3);

	
	
	s_placefolderWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_placefolderWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));

	s_placefolderWnd->callRewrite();						//再描画

	s_placefolderWnd->setVisible(false);

	return 0;

}

int CreateRigidWnd()
{

	/////////
	//s_rigidWnd = new OrgWindow(
	//	0,
	//	_T("RigidWindow"),		//ウィンドウクラス名
	//	GetModuleHandle(NULL),	//インスタンスハンドル
	//							//WindowPos(100, 200),		//位置
	//	WindowPos(1216, 48),
	//	//WindowSize(450,880),		//サイズ
	//	//WindowSize(450,680),		//サイズ
	//	//WindowSize(450, 760),		//サイズ
	//	WindowSize(450, 780),		//サイズ
	//	_T("剛体設定ウィンドウ"),	//タイトル
	//	s_mainhwnd,	//親ウィンドウハンドル
	//	true,					//表示・非表示状態
	//	70, 50, 70,				//カラー
	//	true,					//閉じられるか否か
	//	true);					//サイズ変更の可否

	s_dsrigidctrls.clear();

	s_rigidWnd = new OrgWindow(
		0,
		_T("RigidWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight),
		WindowSize(s_sidewidth, s_sideheight),		//サイズ
		_T("RigidWindow"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		true,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true, true);					//サイズ変更の可否

	s_groupcheck = new OWP_CheckBoxA(L"ToAll_MeansToSetToSameGroupRigids", 0);
	s_sphrateSlider = new OWP_Slider(0.6, 20.0, 0.0);
	s_boxzSlider = new OWP_Slider(0.6, 20.0, 0.0);
	s_massSlider = new OWP_Slider(g_initmass, 30.0, 0.0);
	//s_massSeparator = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	//s_massSeparator1 = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	//s_massSeparator2 = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	s_massB = new OWP_Button(L"MassToAll");
	s_thicknessB = new OWP_Button(L"ThicknessToAll");
	s_depthB = new OWP_Button(L"DepthToAll");
	s_massspacelabel = new OWP_Label(L"(Space4)");
	s_validSeparator = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	s_skipB = new OWP_Button(L"ToAll");
	s_rigidskip = new OWP_CheckBoxA(L"Valid/Invalid (有効/無効)", 1);
	s_forbidSeparator = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	s_forbidB = new OWP_Button(L"ToAll");
	s_forbidrot = new OWP_CheckBoxA(L"ForbidRot", 0);
	//s_allrigidenableB = new OWP_Button(L"ValidateAllRigids");
	//s_allrigiddisableB = new OWP_Button(L"InvalidateAllRigids");
	s_btgSlider = new OWP_Slider(-1.0, 1.0, -1.0);
	s_btgscSlider = new OWP_Slider(10.0, 100.0, 0.0);
	s_btgB = new OWP_Button(L"GToAll");
	s_btforceSeparator = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	s_btforce = new OWP_CheckBoxA(L"RigidBodySimulation", 0);
	s_btforceB = new OWP_Button(L"ToAll");

	s_shplabel = new OWP_Label(L"Thickness");
	s_boxzlabel = new OWP_Label(L"Depth");
	s_massSLlabel = new OWP_Label(L"Mass");
	s_btglabel = new OWP_Label(L"Gravity");
	s_btgsclabel = new OWP_Label(L"ScaleOfGravity");

	s_namelabel = new OWP_Label(L"BonaName:????");
	s_lenglabel = new OWP_Label(L"BoneLength:*****[m]");


	s_kB = new OWP_Button(L"SpringParamsToAll");
	s_restB = new OWP_Button(L"RestitutionAndFrictionToAll");

	s_colSeparator = new OWP_Separator(s_rigidWnd, true, 0.5, true);
	s_colB = new OWP_Button(L"ToAll");
	s_colradio = new OWP_RadioButton(L"Cone");
	s_colradio->addLine(L"Capsule");
	s_colradio->addLine(L"Sphere");
	s_colradio->addLine(L"Rectangular");

	s_lkradio = new OWP_RadioButton(L"[posSpring]very weak");
	s_lkradio->addLine(L"[posSpring]weak");
	s_lkradio->addLine(L"[posSpring]regular");
	s_lkradio->addLine(L"[posSpring]custom");

	//s_lkSlider = new OWP_Slider(g_initcuslk, 1e6, 1e4);//60000
	//s_lkSlider = new OWP_Slider(g_initcuslk, 1e10, 1e8);//60000
	s_lkSlider = new OWP_Slider(g_initcuslk, 1e4, 1e2);//60000
	s_lklabel = new OWP_Label(L"posSpring customValue");

	s_akradio = new OWP_RadioButton(L"[rotSpring]very weak");
	s_akradio->addLine(L"[rotSpring]weak");
	s_akradio->addLine(L"[rotSpring]regular");
	s_akradio->addLine(L"[rotSpring]custom");

	//s_akSlider = new OWP_Slider(g_initcusak, 6000.0f, 0.0f);//300
	//s_akSlider = new OWP_Slider(g_initcusak, 30.0f, 0.0f);//300
	//s_akSlider = new OWP_Slider(g_initcusak, 3000.0f, 30.0f);//300
	//s_akSlider = new OWP_Slider(g_initcusak, 3000.0f, 10.0f);//300 ver10024
	s_akSlider = new OWP_Slider(g_initcusak, 3000.0f, 2.0f);//2022/07/19
	s_aklabel = new OWP_Label(L"rotSpring customValue");

	s_restSlider = new OWP_Slider(0.5f, 1.0f, 0.0f);
	s_restlabel = new OWP_Label(L"RigidRestitution");
	s_fricSlider = new OWP_Slider(0.5f, 1.0f, 0.0f);
	s_friclabel = new OWP_Label(L"RigidFriction");


	s_ldmplabel = new OWP_Label(L"[posSpring]rateOfDumping");
	s_admplabel = new OWP_Label(L"[rotSpring]rateOfDumping");
	s_ldmpSlider = new OWP_Slider(g_l_dmp, 1.0, 0.0);
	s_admpSlider = new OWP_Slider(g_a_dmp, 1.0, 0.0);
	s_dmpB = new OWP_Button(L"DumpingToAll");
	s_groupB = new OWP_Button(L"SetRigidGroupIDForConflict");
	s_gcoliB = new OWP_Button(L"SetGroundGroupIDForConflict");




	COLORREF colorToAll = RGB(64, 128 + 32, 128 + 32);
	COLORREF colorValidInvalid = RGB(168, 129, 129);

	s_groupcheck->setTextColor(colorToAll);
	s_massB->setTextColor(colorToAll);
	s_thicknessB->setTextColor(colorToAll);
	s_depthB->setTextColor(colorToAll);
	s_skipB->setTextColor(colorToAll);
	s_forbidB->setTextColor(colorToAll);
	s_btgB->setTextColor(colorToAll);
	s_btforceB->setTextColor(colorToAll);
	s_colB->setTextColor(colorToAll);
	s_dmpB->setTextColor(colorToAll);
	s_kB->setTextColor(colorToAll);
	s_restB->setTextColor(colorToAll);


	s_rigidskip->setTextColor(colorValidInvalid);
	s_btforce->setTextColor(colorValidInvalid);



	int slw = 350;

	s_sphrateSlider->setSize(WindowSize(slw, 40));
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
	s_rigidWnd->addParts(*s_sphrateSlider);
	s_rigidWnd->addParts(*s_thicknessB);
	s_rigidWnd->addParts(*s_boxzlabel);
	s_rigidWnd->addParts(*s_boxzSlider);
	s_rigidWnd->addParts(*s_depthB);
	s_rigidWnd->addParts(*s_massSLlabel);
	s_rigidWnd->addParts(*s_massSlider);
	s_rigidWnd->addParts(*s_massB);

	////s_rigidWnd->addParts(*s_massB);
	////s_massB->setColor(64, 128, 128);
	////s_thicknessB->setColor(64, 128, 128);
	////s_depthB->setColor(64, 128, 128);
	//s_rigidWnd->addParts(*s_massSeparator);
	//s_massSeparator->addParts1(*s_massSeparator1);
	//s_massSeparator->addParts2(*s_massSeparator2);
	//s_massSeparator1->addParts1(*s_massB);
	//s_massSeparator1->addParts2(*s_thicknessB);
	//s_massSeparator2->addParts1(*s_depthB);
	////s_massSeparator2->addParts2(*s_massspacelabel);

	s_rigidWnd->addParts(*s_lenglabel);

	s_rigidWnd->addParts(*s_validSeparator);
	s_validSeparator->addParts1(*s_rigidskip);
	s_validSeparator->addParts2(*s_skipB);
	//s_rigidWnd->addParts(*s_rigidskip);
	
	s_rigidWnd->addParts(*s_forbidSeparator);
	s_forbidSeparator->addParts1(*s_forbidrot);
	s_forbidSeparator->addParts2(*s_forbidB);
	//s_rigidWnd->addParts(*s_forbidrot);
	//s_rigidWnd->addParts(*s_allrigidenableB);
	//s_rigidWnd->addParts(*s_allrigiddisableB);


	s_rigidWnd->addParts(*s_colSeparator);
	s_colSeparator->addParts1(*s_colradio);
	s_colSeparator->addParts2(*s_colB);
	//s_rigidWnd->addParts(*s_colradio);

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
	s_rigidWnd->addParts(*s_btgsclabel);
	s_rigidWnd->addParts(*s_btgscSlider);
	
	s_rigidWnd->addParts(*s_btgB);
	
	//s_rigidWnd->addParts(*s_btforce);
	s_rigidWnd->addParts(*s_btforceSeparator);
	s_btforceSeparator->addParts1(*s_btforce);
	s_btforceSeparator->addParts2(*s_btforceB);

	s_rigidWnd->addParts(*s_groupB);
	s_rigidWnd->addParts(*s_gcoliB);
/////////
	s_dsrigidctrls.push_back(s_namelabel);
	s_dsrigidctrls.push_back(s_groupcheck);
	s_dsrigidctrls.push_back(s_shplabel);
	s_dsrigidctrls.push_back(s_sphrateSlider);
	s_dsrigidctrls.push_back(s_boxzlabel);
	s_dsrigidctrls.push_back(s_boxzSlider);
	s_dsrigidctrls.push_back(s_massSLlabel);
	s_dsrigidctrls.push_back(s_massSlider);
	s_dsrigidctrls.push_back(s_massB);
	s_dsrigidctrls.push_back(s_thicknessB);
	s_dsrigidctrls.push_back(s_depthB);
	s_dsrigidctrls.push_back(s_lenglabel);
	s_dsrigidctrls.push_back(s_rigidskip);
	s_dsrigidctrls.push_back(s_skipB);
	s_dsrigidctrls.push_back(s_forbidrot);
	s_dsrigidctrls.push_back(s_forbidB);
	//s_dsrigidctrls.push_back(s_allrigidenableB);
	//s_dsrigidctrls.push_back(s_allrigiddisableB);

	s_dsrigidctrls.push_back(s_colradio);
	s_dsrigidctrls.push_back(s_colB);

	s_dsrigidctrls.push_back(s_lkradio);
	s_dsrigidctrls.push_back(s_lklabel);
	s_dsrigidctrls.push_back(s_lkSlider);
	s_dsrigidctrls.push_back(s_akradio);
	s_dsrigidctrls.push_back(s_aklabel);
	s_dsrigidctrls.push_back(s_akSlider);
	s_dsrigidctrls.push_back(s_kB);

	s_dsrigidctrls.push_back(s_restlabel);
	s_dsrigidctrls.push_back(s_restSlider);
	s_dsrigidctrls.push_back(s_friclabel);
	s_dsrigidctrls.push_back(s_fricSlider);
	s_dsrigidctrls.push_back(s_restB);


	s_dsrigidctrls.push_back(s_ldmplabel);
	s_dsrigidctrls.push_back(s_ldmpSlider);
	s_dsrigidctrls.push_back(s_admplabel);
	s_dsrigidctrls.push_back(s_admpSlider);
	s_dsrigidctrls.push_back(s_dmpB);

	s_dsrigidctrls.push_back(s_btglabel);
	s_dsrigidctrls.push_back(s_btgSlider);
	s_dsrigidctrls.push_back(s_btgsclabel);
	s_dsrigidctrls.push_back(s_btgscSlider);
	s_dsrigidctrls.push_back(s_btgB);
	s_dsrigidctrls.push_back(s_btforce);
	s_dsrigidctrls.push_back(s_btforceB);

	s_dsrigidctrls.push_back(s_groupB);
	s_dsrigidctrls.push_back(s_gcoliB);


	s_rigidWnd->setCloseListener([](){ 
		if (s_model) {
			s_RcloseFlag = true;
		}
	});

	s_sphrateSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_sphrateSlider->getValue();
				curre->SetSphrate(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_boxzSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_boxzSlider->getValue();
				curre->SetBoxzrate(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});

	s_massSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_massSlider->getValue();
				curre->SetMass(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});

	s_ldmpSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_ldmpSlider->getValue();
				curre->SetLDamping(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_admpSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_admpSlider->getValue();
				curre->SetADamping(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});

	s_lkSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_lkSlider->getValue();
				curre->SetCusLk(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_akSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_akSlider->getValue();
				curre->SetCusAk(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});


	s_restSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_restSlider->getValue();
				curre->SetRestitution(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_fricSlider->setCursorListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				float val = (float)s_fricSlider->getValue();
				curre->SetFriction(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_rigidskip->setButtonListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				bool validflag = s_rigidskip->getValue();
				if (validflag == false) {
					curre->SetSkipflag(1);
				}
				else {
					curre->SetSkipflag(0);
				}
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_skipB->setButtonListener([]() {
		if (s_model) {
			bool validflag = s_rigidskip->getValue();
			bool skipflag;
			if (validflag == false) {
				skipflag = true;
			}
			else {
				skipflag = false;
			}
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg) {
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre) {
					gid = curre->GetGroupid();
				}
				else {
					gid = -1;
				}
			}
			s_model->SetAllSkipflagData(gid, s_reindexmap[s_model], skipflag);
		}
		});

	s_forbidrot->setButtonListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				bool validflag = s_forbidrot->getValue();
				if (validflag == false) {
					curre->SetForbidRotFlag(0);
				}
				else {
					curre->SetForbidRotFlag(1);
				}
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_forbidB->setButtonListener([]() {
		if (s_model) {
			bool validflag = s_forbidrot->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg) {
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre) {
					gid = curre->GetGroupid();
				}
				else {
					gid = -1;
				}
			}
			s_model->SetAllForbidrotData(gid, s_reindexmap[s_model], validflag);
		}
		});

	//s_allrigidenableB->setButtonListener([](){
	//	if (s_model){
	//		s_model->EnableAllRigidElem(s_reindexmap[s_model]);
	//	}
	//	s_rigidWnd->callRewrite();						//再描画
	//});
	//s_allrigiddisableB->setButtonListener([](){
	//	if (s_model){
	//		s_model->DisableAllRigidElem(s_reindexmap[s_model]);
	//	}
	//	s_rigidWnd->callRewrite();						//再描画
	//});


	s_btforce->setButtonListener([](){
		if (s_model && (s_curboneno >= 0)){
			CBone* curbone = s_model->GetBoneByID(s_curboneno);
			if (curbone){
				CBone* parentbone = curbone->GetParent();
				if (parentbone){
					bool kinflag = s_btforce->getValue();
					if (kinflag == false){
						parentbone->SetBtForce(0);
					}
					else{
						parentbone->SetBtForce(1);
					}
				}
			}
		}
		s_rigidWnd->callRewrite();						//再描画
	});

	s_btgSlider->setCursorListener([](){
		if (s_model) {
			float btg = (float)s_btgSlider->getValue();
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				curre->SetBtg(btg);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_btgscSlider->setCursorListener([](){
		if (s_model) {
			float btgsc = (float)s_btgscSlider->getValue();
			if (s_model && (s_reindexmap[s_model] >= 0)) {
				REINFO tmpinfo = s_model->GetRigidElemInfo(s_reindexmap[s_model]);
				tmpinfo.btgscale = btgsc;
				s_model->SetRigidElemInfo(s_reindexmap[s_model], tmpinfo);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});

	s_colradio->setSelectListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				int val = s_colradio->getSelectIndex();
				curre->SetColtype(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_colB->setButtonListener([]() {
		if (s_model) {
			int val = s_colradio->getSelectIndex();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg) {
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre) {
					gid = curre->GetGroupid();
				}
				else {
					gid = -1;
				}
			}
			s_model->SetAllColtypeData(gid, s_reindexmap[s_model], val);
		}
		});

	s_lkradio->setSelectListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				int val = s_lkradio->getSelectIndex();
				curre->SetLKindex(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
	});
	s_akradio->setSelectListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				int val = s_akradio->getSelectIndex();
				curre->SetAKindex(val);
			}
			s_rigidWnd->callRewrite();						//再描画
		}
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
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllKData(gid, s_reindexmap[s_model], lindex, aindex, cuslk, cusak);
		}
	});
	s_restB->setButtonListener([](){
		if (s_model){
			float rest = (float)s_restSlider->getValue();
			float fric = (float)s_fricSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllRestData(gid, s_reindexmap[s_model], rest, fric);
		}
	});
	s_dmpB->setButtonListener([](){
		if (s_model){
			float ldmp = (float)s_ldmpSlider->getValue();
			float admp = (float)s_admpSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllDmpData(gid, s_reindexmap[s_model], ldmp, admp);
		}
	});

	s_groupB->setButtonListener([](){
		if (s_model) {
			CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
			if (curre) {
				CColiIDDlg dlg(curre);
				s_pcolidlg = &dlg;
				s_undercolidlg = true;

				dlg.DoModal();
				if (dlg.m_setgroup == 1) {
					if (s_model) {
						s_model->SetColiIDtoGroup(curre);
					}
				}

				s_undercolidlg = false;
				s_pcolidlg = 0;

			}
		}
	});
	s_gcoliB->setButtonListener([](){
		if (s_model) {
			if (s_bpWorld) {
				CGColiIDDlg dlg(s_bpWorld->m_coliids, s_bpWorld->m_myselfflag);
				s_pgcolidlg = &dlg;
				s_undergcolidlg = true;

				int dlgret = (int)dlg.DoModal();
				if (dlgret == IDOK) {
					s_bpWorld->m_coliids = dlg.m_coliids;
					s_bpWorld->m_myselfflag = dlg.m_myself;
					s_bpWorld->RemakeG();
				}

				s_undergcolidlg = false;
				s_pgcolidlg = 0;
			}
		}
	});

	s_massB->setButtonListener([](){
		if (s_model){
			float mass = (float)s_massSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllMassData(gid, s_reindexmap[s_model], mass);
		}
		//		_ASSERT( 0 );
	});
	s_thicknessB->setButtonListener([]() {
		if (s_model) {
			float sphrate = (float)s_sphrateSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg) {
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre) {
					gid = curre->GetGroupid();
				}
				else {
					gid = -1;
				}
			}
			s_model->SetAllSphrateData(gid, s_reindexmap[s_model], sphrate);
		}
		//		_ASSERT( 0 );
	});
	s_depthB->setButtonListener([]() {
		if (s_model) {
			float boxzrate = (float)s_boxzSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg) {
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre) {
					gid = curre->GetGroupid();
				}
				else {
					gid = -1;
				}
			}
			s_model->SetAllBoxzrateData(gid, s_reindexmap[s_model], boxzrate);
		}
		//		_ASSERT( 0 );
		});
	s_btgB->setButtonListener([](){
		if (s_model){
			float btg = (float)s_btgSlider->getValue();
			int chkg = (int)s_groupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				if (curre){
					gid = curre->GetGroupid();
				}
				else{
					gid = -1;
				}
			}
			s_model->SetAllBtgData(gid, s_reindexmap[s_model], btg);
		}
	});
	s_btforceB->setButtonListener([]() {
		if (s_model) {
			bool kinflag = s_btforce->getValue();
			s_model->SetAllBtforceData(s_reindexmap[s_model], kinflag);
		}
	});

	s_rigidWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_rigidWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));


	s_rigidWnd->callRewrite();						//再描画
	s_rigidWnd->setVisible(false);

	s_rcrigidwnd.top = s_sidemenuheight;
	s_rcrigidwnd.left = 0;

	s_rcrigidwnd.bottom = s_sideheight;
	s_rcrigidwnd.right = s_sidewidth;


	return 0;
}

int CreateImpulseWnd()
{

	s_dsimpulsectrls.clear();

	//////////
	s_impWnd = new OrgWindow(
		0,
		_T("ImpulseWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight),
		WindowSize(s_sidewidth, s_sideheight),		//サイズ
		_T("ImpulseWindow"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_impgroupcheck = new OWP_CheckBoxA(L"SetToAllRigidSMeansToSetSameGroup", 0);

	s_impxSlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impySlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impzSlider = new OWP_Slider(0.0, 50.0, -50.0);
	s_impscaleSlider = new OWP_Slider(1.0, 10.0, 0.0);
	s_impxlabel = new OWP_Label(L"Impulse X");
	s_impylabel = new OWP_Label(L"Impulse Y");
	s_impzlabel = new OWP_Label(L"Impulse Z");
	s_impscalelabel = new OWP_Label(L"ScaleOfImpulse ");
	s_impallB = new OWP_Button(L"SetImpulseToAllRigies");

	int slw = 350;

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
/////////
	s_dsimpulsectrls.push_back(s_impgroupcheck);
	s_dsimpulsectrls.push_back(s_impxlabel);
	s_dsimpulsectrls.push_back(s_impxSlider);
	s_dsimpulsectrls.push_back(s_impylabel);
	s_dsimpulsectrls.push_back(s_impySlider);
	s_dsimpulsectrls.push_back(s_impzlabel);
	s_dsimpulsectrls.push_back(s_impzSlider);
	s_dsimpulsectrls.push_back(s_impscalelabel);
	s_dsimpulsectrls.push_back(s_impscaleSlider);
	s_dsimpulsectrls.push_back(s_impallB);


	s_impWnd->setCloseListener([](){ 
		if (s_model) {
			s_IcloseFlag = true;
		}
	});

	s_impzSlider->setCursorListener([](){
		if (s_model) {
			float val = (float)s_impzSlider->getValue();
			if (s_model) {
				s_model->SetImp(s_curboneno, 2, val);
			}
			s_impWnd->callRewrite();						//再描画
		}
	});
	s_impySlider->setCursorListener([](){
		if (s_model) {
			float val = (float)s_impySlider->getValue();
			if (s_model) {
				s_model->SetImp(s_curboneno, 1, val);
			}
			s_impWnd->callRewrite();						//再描画
		}
	});
	s_impxSlider->setCursorListener([](){
		if (s_model) {
			float val = (float)s_impxSlider->getValue();
			if (s_model) {
				s_model->SetImp(s_curboneno, 0, val);
			}
			s_impWnd->callRewrite();						//再描画
		}
	});
	s_impscaleSlider->setCursorListener([](){
		if (s_model) {
			float scale = (float)s_impscaleSlider->getValue();
			g_impscale = scale;
			s_impWnd->callRewrite();						//再描画
		}
	});
	s_impallB->setButtonListener([](){
		if (s_model){
			float impx = (float)s_impxSlider->getValue();
			float impy = (float)s_impySlider->getValue();
			float impz = (float)s_impzSlider->getValue();
			int chkg = (int)s_impgroupcheck->getValue();
			int gid = -1;
			if (chkg){
				CRigidElem* curre = s_model->GetRgdRigidElem(s_rgdindexmap[s_model], s_curboneno);
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


	s_impWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_impWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));
	s_impWnd->callRewrite();

	return 0;
}

int CreateGPlaneWnd()
{

	s_dsgpctrls.clear();

	//////////
	s_gpWnd = new OrgWindow(
		0,
		_T("GPlaneWindow"),		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight),		//位置
		WindowSize(s_sidewidth, s_sideheight),		//サイズ
		_T("GroudOfPhysics"),	//タイトル
		s_mainhwnd,	//親ウィンドウハンドル
		false,					//表示・非表示状態
		//70, 50, 70,				//カラー
		0, 0, 0,				//カラー
		true,					//閉じられるか否か
		true);					//サイズ変更の可否

	s_ghSlider = new OWP_Slider(-1.5, 5.0, -15.0);
	s_gsizexSlider = new OWP_Slider(5.0, 50.0, -50.0);
	s_gsizezSlider = new OWP_Slider(5.0, 50.0, -50.0);
	s_ghlabel = new OWP_Label(L"Height");
	s_gsizexlabel = new OWP_Label(L"SizeOfX");
	s_gsizezlabel = new OWP_Label(L"SizeOfZ");
	s_gpdisp = new OWP_CheckBoxA(L"Display", 1);

	s_grestSlider = new OWP_Slider(0.5, 1.0, 0.0);
	s_gfricSlider = new OWP_Slider(0.5, 1.0, 0.0);
	s_grestlabel = new OWP_Label(L"Restitution");
	s_gfriclabel = new OWP_Label(L"Friction");


	int slw = 350;

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
/////////
	s_dsgpctrls.push_back(s_ghlabel);
	s_dsgpctrls.push_back(s_ghSlider);
	s_dsgpctrls.push_back(s_gsizexlabel);
	s_dsgpctrls.push_back(s_gsizexSlider);
	s_dsgpctrls.push_back(s_gsizezlabel);
	s_dsgpctrls.push_back(s_gsizezSlider);
	s_dsgpctrls.push_back(s_gpdisp);

	s_dsgpctrls.push_back(s_grestlabel);
	s_dsgpctrls.push_back(s_grestSlider);
	s_dsgpctrls.push_back(s_gfriclabel);
	s_dsgpctrls.push_back(s_gfricSlider);

	s_gpWnd->setCloseListener([](){ 
		if (s_model) {
			s_GcloseFlag = true;
		}
	});

	s_ghSlider->setCursorListener([](){
		if (s_model) {
			if (s_bpWorld) {
				s_bpWorld->m_gplaneh = (float)s_ghSlider->getValue();
				s_bpWorld->RemakeG();

				ChaVector3 tra(0.0f, 0.0f, 0.0f);
				ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
				CallF(s_gplane->MultDispObj(mult, tra), return);

				s_gpWnd->callRewrite();						//再描画
			}
		}
	});
	s_gsizexSlider->setCursorListener([](){
		if (s_model) {
			if (s_bpWorld && s_gplane) {
				s_bpWorld->m_gplanesize.x = (float)s_gsizexSlider->getValue();

				ChaVector3 tra(0.0f, 0.0f, 0.0f);
				ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
				CallF(s_gplane->MultDispObj(mult, tra), return);
				s_gpWnd->callRewrite();						//再描画
			}
		}
	});
	s_gsizezSlider->setCursorListener([](){
		if (s_model) {
			if (s_bpWorld && s_gplane) {
				s_bpWorld->m_gplanesize.y = (float)s_gsizezSlider->getValue();

				ChaVector3 tra(0.0f, 0.0f, 0.0f);
				ChaVector3 mult(s_bpWorld->m_gplanesize.x, 1.0f, s_bpWorld->m_gplanesize.y);
				CallF(s_gplane->MultDispObj(mult, tra), return);
				s_gpWnd->callRewrite();						//再描画
			}
		}
	});
	s_gpdisp->setButtonListener([](){
		if (s_model) {
			if (s_bpWorld) {
				bool dispflag = s_gpdisp->getValue();
				s_bpWorld->m_gplanedisp = (int)dispflag;
				s_gpWnd->callRewrite();						//再描画
			}
		}
	});
	s_grestSlider->setCursorListener([](){
		if (s_model) {
			if (s_bpWorld && s_gplane) {
				s_bpWorld->m_restitution = (float)s_grestSlider->getValue();
				s_bpWorld->RemakeG();

				s_gpWnd->callRewrite();						//再描画
			}
		}
	});
	s_gfricSlider->setCursorListener([](){
		if (s_model) {
			if (s_bpWorld && s_gplane) {
				s_bpWorld->m_friction = (float)s_gfricSlider->getValue();
				s_bpWorld->RemakeG();

				s_gpWnd->callRewrite();						//再描画
			}
		}
	});


	s_gpWnd->setSize(WindowSize(s_sidewidth, s_sideheight));
	s_gpWnd->setPos(WindowPos(s_timelinewidth + s_mainwidth + 16, s_sidemenuheight));
	s_gpWnd->callRewrite();

	return 0;
}

int CreateToolWnd()
{
	/////////
	// ツールウィンドウを作成してボタン類を追加
	s_toolWnd = new OrgWindow(
		0,
		L"ToolWindow_",		//ウィンドウクラス名
		GetModuleHandle(NULL),	//インスタンスハンドル
		//WindowPos(400, 580),		//位置
		//WindowPos(50, 645),		//位置
		WindowPos(0, s_2ndposy),		//位置
		WindowSize(s_toolwidth, s_toolheight),		//サイズ
		L"ToolWindow",	//タイトル
		//s_timelineWnd->getHWnd(),	//親ウィンドウハンドル
		s_mainhwnd,
		true,					//表示・非表示状態
		//70, 50, 70);// ,				//カラー
		0, 0, 0);				//カラー

	//	true);// ,					//閉じられるか否か
	//	false);					//サイズ変更の可否

	//s_toolWnd->callRewrite();


	s_toolSelBoneB = new OWP_Button(_T("コマンド対象ボーン target bone"));
	s_toolSelectCopyFileName = new OWP_Button(_T("コピー履歴選択 sel cp history"));
	s_toolCopyB = new OWP_Button(_T("コピー copy"));
	s_toolSymCopyB = new OWP_Button(_T("対称コピー sym copy"));
	//s_toolCutB = new OWP_Button(_T("カット"));
	s_toolPasteB = new OWP_Button(_T("ペースト paste"));
	s_toolInitMPB = new OWP_Button(_T("姿勢初期化 init"));
	//s_toolDeleteB = new OWP_Button(_T("削除"));
	//s_toolMarkB = new OWP_Button(_T("マーク作成"));
	s_toolMotPropB = new OWP_Button(_T("プロパティ property"));
	s_toolFilterB = new OWP_Button(_T("平滑化 smoothing"));
	s_toolInterpolateB = new OWP_Button(_T("補間 interpolate"));

	s_toolWnd->addParts(*s_toolSelBoneB);
	s_toolWnd->addParts(*s_toolSelectCopyFileName);
	s_toolWnd->addParts(*s_toolCopyB);
	s_toolWnd->addParts(*s_toolSymCopyB);
	s_toolWnd->addParts(*s_toolPasteB);
	s_toolWnd->addParts(*s_toolInitMPB);
	//s_toolWnd->addParts(*s_toolMarkB);
	s_toolWnd->addParts(*s_toolMotPropB);
	s_toolWnd->addParts(*s_toolFilterB);
	s_toolWnd->addParts(*s_toolInterpolateB);

	s_dstoolctrls.push_back(s_toolSelBoneB);
	s_dstoolctrls.push_back(s_toolCopyB);
	s_dstoolctrls.push_back(s_toolSymCopyB);
	s_dstoolctrls.push_back(s_toolPasteB);
	s_dstoolctrls.push_back(s_toolInitMPB);
	//s_dstoolctrls.push_back(s_toolMarkB);
	s_dstoolctrls.push_back(s_toolMotPropB);
	s_dstoolctrls.push_back(s_toolFilterB);
	s_dstoolctrls.push_back(s_toolInterpolateB);


	s_toolWnd->setCloseListener([](){ 
		if (s_model) {
			s_closetoolFlag = true;
		}
	});
	//s_toolWnd->setHoverListener([]() { SetCapture(s_toolWnd->getHWnd()); });
	//s_toolWnd->setLeaveListener([]() { ReleaseCapture(); });
	s_toolSelectCopyFileName->setButtonListener([]() {
		if (s_model) {
			s_selCopyHisotryFlag = true;
		}
	});
	s_toolCopyB->setButtonListener([](){ 
		if (s_model) {
			s_copyFlag = true;
		}
	});
	s_toolSymCopyB->setButtonListener([](){ 
		if (s_model) {
			s_symcopyFlag = true;
		}
	});
	s_toolPasteB->setButtonListener([](){ 
		if (s_model) {
			s_pasteFlag = true;
		}
	});
	s_toolMotPropB->setButtonListener([](){ 
		if (s_model) {
			s_motpropFlag = true;
		}
	});
	//s_toolMarkB->setButtonListener( [](){ s_markFlag = true; } );
	s_toolSelBoneB->setButtonListener([](){ 
		if (s_model) {
			s_selboneFlag = true;
		}
	});
	s_toolInitMPB->setButtonListener([](){ 
		if (s_model) {
			s_initmpFlag = true;
		}
	});
	s_toolFilterB->setButtonListener([](){ 
		if (s_model) {
			s_filterFlag = true;
		}
	});
	s_toolInterpolateB->setButtonListener([]() { 
		if (s_model) {
			s_interpolateFlag = true;
		}
	});

	s_rctoolwnd.top = 0;
	s_rctoolwnd.left = 0;
	s_toolWnd->setPos(WindowPos(0, s_2ndposy));
	s_rctoolwnd.bottom = s_toolheight;
	s_rctoolwnd.right = s_toolwidth;
	

	s_toolWnd->autoResizeAllParts();
	s_toolWnd->refreshPosAndSize();//これを呼ばないと1回目のクリック位置がずれることがある。


	return 0;

}


int CreateLayerWnd()
{
	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		return 0;
	}

	if (s_layerWnd) {
		//_ASSERT(0);
		return 0;
	}


	////
	// ウィンドウを作成
	if (g_4kresolution) {
		s_layerWnd = new OrgWindow(
			1,
			_T("LayerTool"),		//ウィンドウクラス名
			GetModuleHandle(NULL),	//インスタンスハンドル
			//WindowPos(800, 500),		//位置
			//WindowPos(250, 645),		//位置
			//WindowPos(250, 660),		//位置
			WindowPos(2000, 660),		//位置
			WindowSize(150 * 2, 200 * 2),		//サイズ
			_T("LayerTool"),	//タイトル
			//s_mainhwnd,					//親ウィンドウハンドル
			//s_3dwnd,
			NULL,//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 他所をクリックしても隠れないように
			true,					//表示・非表示状態
			//false,					//表示・非表示状態
			//70, 50, 70,				//カラー
			0, 0, 0,				//カラー
			true,					//閉じられるか否か
			true);					//サイズ変更の可否
	}
	else {
		s_layerWnd = new OrgWindow(
			1,
			_T("LayerTool"),		//ウィンドウクラス名
			GetModuleHandle(NULL),	//インスタンスハンドル
			//WindowPos(800, 500),		//位置
			//WindowPos(250, 645),		//位置
			//WindowPos(250, 660),		//位置
			WindowPos(2000, 660),		//位置
			WindowSize(150, 200),		//サイズ
			_T("LayerTool"),	//タイトル
			//s_mainhwnd,					//親ウィンドウハンドル
			//s_3dwnd,
			NULL,//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 他所をクリックしても隠れないように
			true,					//表示・非表示状態
			//false,					//表示・非表示状態
			//70, 50, 70,				//カラー
			0, 0, 0,				//カラー
			true,					//閉じられるか否か
			true);					//サイズ変更の可否
	}

	// レイヤーウィンドウパーツを作成
	s_owpLayerTable = new OWP_LayerTable(_T("レイヤーテーブル"));
	WCHAR label[256];
	wcscpy_s(label, 256, L"dummy name");
	s_owpLayerTable->newLine(label, 0);

	// ウィンドウにウィンドウパーツを登録
	s_layerWnd->addParts(*s_owpLayerTable);


	s_layerWnd->setCloseListener([](){ 
		if (s_model) {
			s_closeobjFlag = true;
		}
	});

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

	RECT wnd3drect;
	if (s_mainhwnd) {
		GetWindowRect(s_mainhwnd, &wnd3drect);
		s_layerWnd->setPos(WindowPos(wnd3drect.left + 750, wnd3drect.top + 500));
	}
	else {
		s_layerWnd->setPos(WindowPos(600, 200));
	}
	s_layerWnd->setVisible(false);

	return 0;

}

int OnRenderRefPose(ID3D11DeviceContext* pd3dImmediateContext, CModel* curmodel)
{
	if (!pd3dImmediateContext || !curmodel) {
		return 0;
	}

	if (s_sprefpos.state) {
		if (curmodel == s_model) {
			int lightflag = 0;
			int btflag = 0;

			double refframe = CalcRefFrame();
			if (refframe >= 0.0) {
				MOTINFO* curmi = s_model->GetCurMotInfo();
				if (curmi) {
					double saveframe = curmi->curframe;
					if (saveframe != refframe) {
						CBone* curbone = s_model->GetBoneByID(s_curboneno);
						if (curbone) {
							std::vector<ChaVector3> vecbonepos;
							vecbonepos.clear();
							ChaVector3 curbonepos;

							double starttime, endtime;
							if (refframe < saveframe) {
								starttime = refframe;
								endtime = saveframe;
							}
							else {
								starttime = saveframe;
								endtime = refframe;
							}
							double insideframe;
							for (insideframe = starttime; insideframe <= endtime; insideframe += 1.0) {
								s_model->SetMotionFrame(insideframe);
								//s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
								s_model->HierarchyRouteUpdateMatrix(curbone, &s_model->GetWorldMat(), &s_matVP);//高速化：関係ボーンルート限定アップデート
								ChaVector3TransformCoord(&curbonepos, &(curbone->GetJointFPos()), &(curbone->GetCurMp().GetWorldMat()));
								vecbonepos.push_back(curbonepos);
							}

							//refframeのポーズを表示
							s_model->SetMotionFrame(refframe);
							s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
							ChaVector4 refdiffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 0.25f);
							s_model->OnRender(pd3dImmediateContext, lightflag, refdiffusemult, btflag);//render model at reference pos


							//元のフレームに戻す
							s_model->SetMotionFrame(saveframe);
							s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);


							//render arrow : selected bone : befpos --> aftpos arrow
							CBone* childbone = curbone->GetChild();
							if (childbone && curbone->GetColDisp(childbone, COL_CONE_INDEX)) {
								ChaVector4 arrowdiffusemult = ChaVector4(1.0f, 0.5f, 0.5f, 0.85f);
								curbone->GetColDisp(childbone, COL_CONE_INDEX)->RenderRefArrow(pd3dImmediateContext, curbone, arrowdiffusemult, g_refmult, vecbonepos);
							}

						}
					}
				}
			}
		}
	}

	return 0;
}

int OnRenderModel(ID3D11DeviceContext* pd3dImmediateContext)
{
	//if (g_bvh2fbxbatchflag || g_motioncachebatchflag || g_retargetbatchflag) {
	//if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_motioncachebatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) != 0) || (InterlockedAdd(&g_retargetbatchflag, 0) != 0)) {
		return 0;
	}

	if (s_nowloading == true) {
		return 0;
	}

	if (!s_model) {
		return 0;
	}


	vector<MODELELEM>::iterator itrmodel;
	for (itrmodel = s_modelindex.begin(); itrmodel != s_modelindex.end(); itrmodel++){
		CModel* curmodel = itrmodel->modelptr;

		//if (curmodel && curmodel->GetLoadedFlag() && curmodel->GetModelDisp()){
		if (curmodel && curmodel->m_loadedflag && curmodel->m_modeldisp) {//curmodelが作成途中の場合を考えて、先頭から２つのpublicデータメンバーを参照する
			int lightflag = 1;
			ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
			int btflag = 0;
			if ((g_previewFlag != 4) && (g_previewFlag != 5)){
				btflag = 0;
			}
			else{
				if (g_previewFlag == 4) {
					btflag = 1;
				}
				else {
					//previewFlag == 5
					if ((s_curboneno >= 0) && ((s_onragdollik != 0) || (s_physicskind == 0))) {
						btflag = 2;//2022/07/09
					}
				}
			}

			curmodel->OnRender(pd3dImmediateContext, lightflag, diffusemult, btflag);

			OnRenderRefPose(pd3dImmediateContext, curmodel);
		}
	}

	return 0;
}

int OnRenderGround(ID3D11DeviceContext* pd3dImmediateContext)
{
	if (s_ground && s_dispground){
		g_hmWorld->SetMatrix((float*)&s_matWorld);
		//g_pEffect->SetMatrix(g_hmWorld, &(s_matWorld.D3DX()));

		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		s_ground->OnRender(pd3dImmediateContext, 0, diffusemult);
	}
	if (s_gplane && s_bpWorld && s_bpWorld->m_gplanedisp){
		ChaMatrix gpmat = s_inimat;
		gpmat._42 = s_bpWorld->m_gplaneh;
		g_hmWorld->SetMatrix((float*)&gpmat);
		//g_pEffect->SetMatrix(g_hmWorld, &(gpmat.D3DX()));

		ChaVector4 diffusemult = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);
		s_gplane->OnRender(pd3dImmediateContext, 0, diffusemult);
	}

	return 0;
}

int OnRenderBoneMark(ID3D11DeviceContext* pd3dImmediateContext)
{
	if (g_bonemarkflag || g_rigidmarkflag) {

		if (s_allmodelbone == 0) {
			//if ((g_previewFlag != 1) && (g_previewFlag != -1) && (g_previewFlag != 4)){
			if (s_model && s_model->GetModelDisp()) {
				//if (s_ikkind >= 3){
				s_model->RenderBoneMark(pd3dImmediateContext, s_bmark, s_bcircle, s_curboneno);
				//}
				//else{
				//	s_model->RenderBoneMark(s_pdev, s_bmark, s_bcircle, 0, s_curboneno);
				//}
			}
		//}
		}
		else {
			vector<MODELELEM>::iterator itrme;
			for (itrme = s_modelindex.begin(); itrme != s_modelindex.end(); itrme++) {
				MODELELEM curme = *itrme;
				CModel* curmodel = curme.modelptr;
				curmodel->RenderBoneMark(pd3dImmediateContext, s_bmark, s_bcircle, 0, s_curboneno);
			}
		}
	}

	return 0;
}
int OnRenderSelect(ID3D11DeviceContext* pd3dImmediateContext)
{
	if ((g_previewFlag != 4) && (g_previewFlag != 5)){
		if (s_select && (s_curboneno >= 0) && (g_previewFlag == 0) && (s_model && s_model->GetModelDisp()) && (g_bonemarkflag != 0)){//underchecking
			//SetSelectCol();
			SetSelectState();
			RenderSelectMark(pd3dImmediateContext, 1);
		}
		if ((s_model && s_model->GetModelDisp()) && (s_oprigflag != 0)) {
			RenderRigMarkFunc(pd3dImmediateContext);
		}
	}
	//else if ((g_previewFlag == 5) && (s_oprigflag == 1)){
	else if (g_previewFlag == 5){
		if (s_select && (s_curboneno >= 0) && (s_model && s_model->GetModelDisp())){
			//SetSelectCol();
			SetSelectState();
			RenderSelectMark(pd3dImmediateContext, 1);
		}
	}

	return 0;
}

int OnRenderSprite(ID3D11DeviceContext* pd3dImmediateContext)
{
	if (s_spret2prev.sprite) {
		s_spret2prev.sprite->OnRender(pd3dImmediateContext);
	}
	else {
		_ASSERT(0);
	}

	if ((s_mbuttoncnt == 0) && (s_mousecenteron.sprite)) {
		s_mousecenteron.sprite->OnRender(pd3dImmediateContext);
	}


	//IK Mode
	if(g_previewFlag == 0){
		int spgcnt;
		for (spgcnt = 0; spgcnt < 3; spgcnt++) {
			if (s_spikmodesw[spgcnt].state) {
				if (s_spikmodesw[spgcnt].spriteON) {
					s_spikmodesw[spgcnt].spriteON->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
			else {
				if (s_spikmodesw[spgcnt].spriteOFF) {
					s_spikmodesw[spgcnt].spriteOFF->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
		}
	}


	//refpossw
	if (g_previewFlag == 0) {
		if (s_sprefpos.state) {
			if (s_sprefpos.spriteON) {
				s_sprefpos.spriteON->OnRender(pd3dImmediateContext);
			}
			else {
				_ASSERT(0);
			}
		}
		else {
			if (s_sprefpos.spriteOFF) {
				s_sprefpos.spriteOFF->OnRender(pd3dImmediateContext);
			}
			else {
				_ASSERT(0);
			}
		}
	}


	//aimbar
	if (g_enableDS && (s_dsdeviceid >= 0)) {

		int platemenukind = s_platemenukind;
		int platenomax = 0;
		if (platemenukind == SPPLATEMENUKIND_GUI) {
			platenomax = SPGUISWNUM;
		}
		else {
			switch (platemenukind) {
			case SPPLATEMENUKIND_RIGID:
				platenomax = SPRIGIDSWNUM;
				break;
			case SPPLATEMENUKIND_RETARGET:
				platenomax = SPRETARGETSWNUM;
				break;
			default:
				platenomax = 0;
				break;
			}
		}

		{
			if (s_spsel3d.state) {
				if (s_spsel3d.spriteON) {
					s_spsel3d.spriteON->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
			else {
				if (s_spsel3d.spriteOFF) {
					s_spsel3d.spriteOFF->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
		}
		{
			int spgcnt;
			int chkplatenomax;
			chkplatenomax = min(SPAIMBARNUM, platenomax);
			for (spgcnt = 0; spgcnt < chkplatenomax; spgcnt++) {
				if (s_spaimbar[spgcnt].state) {
					if (s_spaimbar[spgcnt].spriteON) {
						s_spaimbar[spgcnt].spriteON->OnRender(pd3dImmediateContext);
					}
					else {
						_ASSERT(0);
					}
				}
				else {
					if (s_spaimbar[spgcnt].spriteOFF) {
						s_spaimbar[spgcnt].spriteOFF->OnRender(pd3dImmediateContext);
					}
					else {
						_ASSERT(0);
					}
				}
			}
		}
		{
			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->callRewrite();
			}


			//int spgcnt;
			//for (spgcnt = 0; spgcnt < SPMENU_MAX; spgcnt++) {
			//	//MainMenuAimBarWndの背景色は、非選択時に茶色、選択時にオレンジ。オレンジはspriteONの色。よってスプライト表示のオンとオフを入れ替える。
			//	if (s_spmenuaimbar[spgcnt].state) {
			//		if (s_spmenuaimbar[spgcnt].spriteOFF) {//ONのときにOFF色
			//			s_spmenuaimbar[spgcnt].spriteOFF->OnRender(pd3dImmediateContext);
			//		}
			//		else {
			//			_ASSERT(0);
			//		}
			//	}
			//	else {
			//		if (s_spmenuaimbar[spgcnt].spriteON) {//OFFのときにON色
			//			s_spmenuaimbar[spgcnt].spriteON->OnRender(pd3dImmediateContext);
			//		}
			//		else {
			//			_ASSERT(0);
			//		}
			//	}
			//}
		}

	}




	if (s_platemenukind == SPPLATEMENUKIND_GUI) {
		//menu 0 : Select 3DWindow GUI
		{
			int spgcnt;
			for (spgcnt = 0; spgcnt < SPGUISWNUM; spgcnt++) {
				if (s_spguisw[spgcnt].state) {
					if (s_spguisw[spgcnt].spriteON) {
						s_spguisw[spgcnt].spriteON->OnRender(pd3dImmediateContext);
					}
					else {
						_ASSERT(0);
					}
				}
				else {
					if (s_spguisw[spgcnt].spriteOFF) {
						s_spguisw[spgcnt].spriteOFF->OnRender(pd3dImmediateContext);
					}
					else {
						_ASSERT(0);
					}
				}
			}
		}

	}
	else if (s_platemenukind == SPPLATEMENUKIND_RIGID) {
		//menu 1 : Select SideMenu 
		int spgcnt;
		for (spgcnt = 0; spgcnt < SPRIGIDSWNUM; spgcnt++) {
			if (s_sprigidsw[spgcnt].state) {
				if (s_sprigidsw[spgcnt].spriteON) {
					s_sprigidsw[spgcnt].spriteON->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
			else {
				if (s_sprigidsw[spgcnt].spriteOFF) {
					s_sprigidsw[spgcnt].spriteOFF->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
		}
	}
	else if (s_platemenukind == SPPLATEMENUKIND_RETARGET) {
		int sprcnt;
		for (sprcnt = 0; sprcnt < SPRETARGETSWNUM; sprcnt++) {
			if (s_spretargetsw[sprcnt].state) {
				if (s_spretargetsw[sprcnt].spriteON) {
					s_spretargetsw[sprcnt].spriteON->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
			else {
				if (s_spretargetsw[sprcnt].spriteOFF) {
					s_spretargetsw[sprcnt].spriteOFF->OnRender(pd3dImmediateContext);
				}
				else {
					_ASSERT(0);
				}
			}
		}
	}

	if (s_spguisw[SPGUISW_SPRITEFK].state) {

		int spacnt;
		for (spacnt = 0; spacnt < SPAXISNUM; spacnt++) {
			if (s_spaxis[spacnt].sprite) {
				s_spaxis[spacnt].sprite->OnRender(pd3dImmediateContext);
			}
			else {
				_ASSERT(0);
			}
		}
		int spccnt;
		for (spccnt = 0; spccnt < SPR_CAM_MAX; spccnt++) {
			if (s_spcam[spccnt].sprite) {
				s_spcam[spccnt].sprite->OnRender(pd3dImmediateContext);
			}
			else {
				_ASSERT(0);
			}
		}

		//if (s_spbt.sprite) {
		//	s_spbt.sprite->OnRender(pd3dImmediateContext);
		//}
		//else {
		//	_ASSERT(0);
		//}


	}

	if ((s_oprigflag >= 0) && (s_oprigflag < SPRIGMAX)) {
		//if (s_customrigbone) {
			if (s_sprig[s_oprigflag].sprite) {
				s_sprig[s_oprigflag].sprite->OnRender(pd3dImmediateContext);
			}
			else {
				_ASSERT(0);
			}
		//}
	}


	if (g_dsmousewait == 1) {
		if (s_spmousehere.sprite) {
			s_spmousehere.sprite->OnRender(pd3dImmediateContext);
		}
		else {
			_ASSERT(0);
		}
	}


	return 0;
}

int OnRenderSetShaderConst()
{
	//HRESULT hr;

	ChaVector3 vLightDir[MAX_LIGHTS];
	ChaVector4 vLightDiffuse[MAX_LIGHTS];

	// Get the projection & view matrix from the camera class
	g_hmVP->SetMatrix((float*)&s_matVP);
	//g_pEffect->SetMatrix(g_hmVP, &(s_matVP.D3DX()));
	////g_pEffect->SetMatrix(g_hmWorld, &s_matW);//CModelへ


	ChaVector3 lightdir0, nlightdir0;
	lightdir0 = g_camEye;
	ChaVector3Normalize(&nlightdir0, &lightdir0);
	g_LightControl[0].SetLightDirection(nlightdir0.D3DX());

	// Render the light arrow so the user can visually see the light dir
	for (int i = 0; i < g_nNumActiveLights; i++)
	{
		//if( s_displightarrow ){
		//	D3DXCOLOR arrowColor = ( i == g_nActiveLight ) ? D3DXCOLOR( 1, 1, 0, 1 ) : D3DXCOLOR( 1, 1, 1, 1 );
		//	V( g_LightControl[i].OnRender10( arrowColor, &mView, &mProj, &g_camEye ) );
		//}
		vLightDir[i] = ChaVector3(g_LightControl[i].GetLightDirection());
		vLightDiffuse[i] = ChaVector4(g_fLightScale, g_fLightScale, g_fLightScale, g_fLightScale);
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

int OnRenderUtDialog(ID3D11DeviceContext* pd3dImmediateContext, float fElapsedTime)
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
	//parwnd = s_3dwnd;
	parwnd = s_3dwnd;

	CRMenuMain* rmenu;
	rmenu = new CRMenuMain(IDR_RMENU);
	if (!rmenu){
		return 1;
	}
	int ret;
	ret = rmenu->Create(parwnd, MENUOFFSET_INITMPFROMTOOL);
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
	
	WCHAR strinitmpsub[3][32] = { L"AllBones", L"OneSelectedBone", L"SelectedAndChildren" };
	WCHAR strinitmpsubsub[4][32] = { L"InitRotAndPosAndScale", L"InitRot", L"InitPos", L"InitScale" };

	int subno;
	for (subno = 0; subno < 3; subno++){
		setmenuid = ID_RMENU_0 + subno * 4 + MENUOFFSET_INITMPFROMTOOL;

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
		for (subsubno = 0; subsubno < 4; subsubno++){
			int subsubid = setmenuid + subsubno;
			AppendMenu(subsubmenu, MF_STRING, subsubid, strinitmpsubsub[subsubno]);

		}
	}

/////////////
	s_cursubmenu = rmenu->GetSubMenu();

	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
	int initmode = -1;
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	//if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + 3 * 3))){
	//	int subid = (menuid - ID_RMENU_0) / 3;
	//	int initmode = (menuid - ID_RMENU_0) - subid * 3;

	//	s_copymotvec.clear();
	//	s_copyKeyInfoList.clear();
	//	s_copyKeyInfoList = s_owpLTimeline->getSelectedKey();
	//	s_editrange.SetRange(s_copyKeyInfoList, s_owpTimeline->getCurrentTime());

	//	if (subid == 0){
	//		list<KeyInfo>::iterator itrcp;
	//		for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
	//			double curframe = itrcp->time;
	//			CBone* topbone = s_model->GetTopBone();
	//			if (topbone){
	//				InitMpByEulReq(initmode, topbone, mi->motid, curframe);//topbone req
	//			}
	//		}
	//	}
	//	else if (subid == 1){
	//		CBone* curbone = s_model->GetBoneByID(s_curboneno);
	//		if (curbone){
	//			list<KeyInfo>::iterator itrcp;
	//			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
	//				double curframe = itrcp->time;
	//				InitMpByEul(initmode, curbone, mi->motid, curframe);//curbone
	//			}
	//		}
	//	}
	//	else if (subid == 2){
	//		CBone* curbone = s_model->GetBoneByID(s_curboneno);
	//		if (curbone){
	//			list<KeyInfo>::iterator itrcp;
	//			for (itrcp = s_copyKeyInfoList.begin(); itrcp != s_copyKeyInfoList.end(); itrcp++){
	//				double curframe = itrcp->time;
	//				InitMpByEulReq(initmode, curbone, mi->motid, curframe);//curbone req
	//			}
	//		}
	//	}

	//	UpdateEditedEuler();
	//}

	for (subno = 0; subno < 3; subno++){
		CRMenuMain* delsubmenu = rsubmenu[subno];
		if (delsubmenu){
			delete delsubmenu;
		}
	}

	rmenu->Destroy();
	delete rmenu;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	return 0;
}

int InitMpByEul(int initmode, CBone* curbone, int srcmotid, double srcframe)
{
	if (curbone){
		if (curbone->GetChild()){
			if (initmode == INITMP_ROTTRA){
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				ChaVector3 traanim = ChaVector3(0.0f, 0.0f, 0.0f);
				//int inittraflag1 = 1;
				int setchildflag1 = 1;
				//int initscaleflag1 = 1;//!!!!!!!
				//curbone->SetWorldMatFromEul(inittraflag1, setchildflag1, cureul, srcmotid, srcframe, initscaleflag1);
				curbone->SetWorldMatFromEulAndTra(setchildflag1, cureul, traanim, srcmotid, srcframe);//scale計算無し
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
				//int isfirstbone = 0;
				cureul = curbone->CalcLocalEulXYZ(paraxsiflag1, srcmotid, srcframe, BEFEUL_ZERO);

				int inittraflag1 = 1;
				int setchildflag1 = 1;
				curbone->SetWorldMatFromEul(inittraflag1, setchildflag1, cureul, srcmotid, srcframe);
			}
			else if (initmode == INITMP_SCALE) {
				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				int paraxsiflag1 = 1;
				//int isfirstbone = 0;
				cureul = curbone->CalcLocalEulXYZ(paraxsiflag1, srcmotid, srcframe, BEFEUL_ZERO);

				ChaVector3 traanim = curbone->CalcLocalTraAnim(srcmotid, srcframe);

				//int inittraflag1 = 0;
				int setchildflag1 = 1;
				//int initscaleflag1 = 1;//!!!!!!!
				curbone->SetWorldMatFromEulAndTra(setchildflag1, cureul, traanim, srcmotid, srcframe);//scale計算無し
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
		::DSMessageBox(s_3dwnd, L"Work Only After Setting Of Axis.\nRetry After Saving Of FBX file.", L"error!!!", MB_OK);
		return 0;
	}

	//古いダイアログを閉じる
	if (s_customrigdlg) {
		DestroyWindow(s_customrigdlg);
		s_customrigdlg = 0;
	}

	Bone2CustomRig(rigno);

	if (!s_customrigdlg){
		//s_customrigdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CUSTOMRIGDLG), s_3dwnd, (DLGPROC)CustomRigDlgProc);
		s_customrigdlg = CreateDialogW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CUSTOMRIGDLG), s_mainhwnd, (DLGPROC)CustomRigDlgProc);
		if (!s_customrigdlg){
			_ASSERT(0);
			return 1;
		}
	}
	else{
		CustomRig2Dlg(s_customrigdlg);
	}
	
	SetParent(s_customrigdlg, s_mainhwnd);
	SetWindowPos(
		s_customrigdlg,
		HWND_TOP,
		s_timelinewidth + s_mainwidth + 16,
		s_sidemenuheight,
		s_sidewidth,
		s_sideheight,
		SWP_SHOWWINDOW
	);


	ShowWindow(s_customrigdlg, SW_SHOW);
	UpdateWindow(s_customrigdlg);
	
	s_oprigflag = 1;

	return 0;
}

int InvalidateCustomRig(int rigno)
{
	if (!s_model) {
		return 0;
	}
	if (s_curboneno < 0) {
		return 0;
	}
	if (!s_model->GetTopBone()) {
		return 0;
	}
	if (s_model->GetOldAxisFlagAtLoading() == 1) {
		::DSMessageBox(s_3dwnd, L"Work Only After Setting Of Axis.\nRetry After Saving Of FBX file.", L"error!!!", MB_OK);
		return 0;
	}

	s_customrigbone = s_model->GetBoneByID(s_curboneno);
	if (s_customrigbone) {
		if ((rigno >= 0) && (rigno < MAXRIGNUM)) {
			s_customrigbone->InvalidateCustomRig(rigno);
		}
	}

	//古いダイアログを閉じる
	if (s_customrigdlg) {
		DestroyWindow(s_customrigdlg);
		s_customrigdlg = 0;
	}

	s_customrigbone = 0;
	s_oprigflag = 0;


	SetTimelineHasRigFlag();


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
			::DSMessageBox(s_3dwnd, L"Invalid Parameter", L"error!!!", MB_OK);
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
		::DSMessageBox(hDlgWnd, L"Limit Range From -100.0 to 100.", L"Out Of Limit Error", MB_OK);
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


		//dispaxis
		WCHAR strcomboda[256] = { 0L };
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_RESETCONTENT, 0, 0);
		wcscpy_s(strcomboda, 256, L"X");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_ADDSTRING, 0, (LPARAM)strcomboda);
		wcscpy_s(strcomboda, 256, L"Y");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_ADDSTRING, 0, (LPARAM)strcomboda);
		wcscpy_s(strcomboda, 256, L"Z");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_ADDSTRING, 0, (LPARAM)strcomboda);
		if ((s_customrig.dispaxis >= 0) && (s_customrig.disporder <= 2)) {
			SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_SETCURSEL, s_customrig.dispaxis, 0);
		}

		//disporder
		WCHAR strcombodo[256] = { 0L };
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_RESETCONTENT, 0, 0);
		wcscpy_s(strcombodo, 256, L"0");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_ADDSTRING, 0, (LPARAM)strcombodo);
		wcscpy_s(strcombodo, 256, L"1");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_ADDSTRING, 0, (LPARAM)strcombodo);
		wcscpy_s(strcombodo, 256, L"2");
		SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_ADDSTRING, 0, (LPARAM)strcombodo);
		if ((s_customrig.disporder >= 0) && (s_customrig.disporder <= 2)) {
			SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_SETCURSEL, s_customrig.disporder, 0);
		}

		//posinverse
		if (s_customrig.posinverse) {
			CheckDlgButton(hDlgWnd, IDC_CHKINV, BST_CHECKED);
		}
		else {
			CheckDlgButton(hDlgWnd, IDC_CHKINV, BST_UNCHECKED);
		}


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
					SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"None");
					//return 1;
				}
			}
			else{
				SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"None");
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
		SetDlgItemText(hDlgWnd, gpboxid[elemno], L"None");
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
//	::DSMessageBox(hDlgWnd, strdbg, L"combolabel", MB_OK);
//}
		combolabel[256 - 1] = 0L;
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
//	::DSMessageBox(hDlgWnd, strdbg, L"rigrigno", MB_OK);
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
//	::DSMessageBox(hDlgWnd, strdbg, L"rigrigbonename", MB_OK);
//}
							CBone* rigrigbone = s_model->GetBoneByWName(rigrigbonename);
							if (rigrigbone){
								s_customrig.rigelem[elemno].rigrigboneno = rigrigbone->GetBoneNo();

//{
//	WCHAR strdbg[256];
//	swprintf_s(strdbg, 256, L"rigrigboneno %d", s_customrig.rigelem[elemno].rigrigboneno);
//	::DSMessageBox(hDlgWnd, strdbg, L"rigrigboneno", MB_OK);
//}
								//int gpboxid[5] = { IDC_CHILD1, IDC_CHILD2, IDC_CHILD3, IDC_CHILD4, IDC_CHILD5 };
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
		s_customrighwnd = hDlgWnd;
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
				s_customrighwnd = 0;

				WCHAR strrigname[256] = { 0L };
				GetDlgItemText(hDlgWnd, IDC_RIGNAME, strrigname, 256);
				wcscpy_s(s_customrig.rigname, 256, strrigname);

				int combonoda = (int)SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPAXIS), CB_GETCURSEL, 0, 0);
				if ((combonoda >= 0) && (combonoda <= 2)) {
					s_customrig.dispaxis = combonoda;
				}

				int combonodo = (int)SendMessage(GetDlgItem(hDlgWnd, IDC_COMBO_DISPORDER), CB_GETCURSEL, 0, 0);
				if ((combonodo >= 0) && (combonodo <= 2)) {
					s_customrig.disporder = combonodo;
				}

				if (IsDlgButtonChecked(hDlgWnd, IDC_CHKINV) == BST_CHECKED) {
					s_customrig.posinverse = true;
				}
				else {
					s_customrig.posinverse = false;
				}

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
						::DSMessageBox(hDlgWnd, L"Invalid VerticalScale. Limit From -100.0 to 100.0.", L"error!!!", MB_OK);
						return 0;
					}
					s_customrig.rigelem[elemno].transuv[0].applyrate = tmprate;

					ret = GetCustomRigRateVal(hDlgWnd, ratevid[elemno], &tmprate);
					if (ret){
						::DSMessageBox(hDlgWnd, L"Invalid HolizontalScale. Limit From -100.0 to 100.0.", L"error!!!", MB_OK);
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
					::DSMessageBox(hDlgWnd, L"Invalid Parameter", L"error!!!", MB_OK);
					return 0;
				}

				CustomRig2Bone();

				SetTimelineHasRigFlag();

				//EndDialog(hDlgWnd, IDOK);
			}
			break;
			case IDCANCEL:
				s_customrighwnd = 0;
				//EndDialog(hDlgWnd, IDCANCEL);
				SetTimelineHasRigFlag();

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

		GUIMenuSetVisible(s_platemenukind, s_platemenuno);

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
		//SetCapture(s_3dwnd);
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_3dwnd, &ptCursor);
		s_pickinfo.clickpos = ptCursor;
		s_pickinfo.mousepos = ptCursor;
		s_pickinfo.mousebefpos = ptCursor;
		s_pickinfo.diffmouse = ChaVector2(0.0f, 0.0f);
		s_pickinfo.firstdiff = ChaVector2(0.0f, 0.0f);

		s_pickinfo.winx = (int)DXUTGetWindowWidth();
		s_pickinfo.winy = (int)DXUTGetWindowHeight();
		s_pickinfo.pickrange = PICKRANGE;

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
				//parwnd = s_3dwnd;
				parwnd = s_3dwnd;

				CRMenuMain* rmenu;
				rmenu = new CRMenuMain(IDR_RMENU);
				if (!rmenu){
					return 1;
				}
				int ret;
				ret = rmenu->Create(parwnd, MENUOFFSET_BONERCLICK);
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

				//位置コンストレイントはMass0で実現する。
				//if (curbone->GetPosConstraint() == 0){
				//	AppendMenu(submenu, MF_STRING, ID_RMENU_PHYSICSCONSTRAINT, L"Physics Pos Constraint設定");
				//}
				//else{
				//	AppendMenu(submenu, MF_STRING, ID_RMENU_PHYSICSCONSTRAINT, L"Physics Pos Constraint解除");
				//}

				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_ON_ALL + MENUOFFSET_BONERCLICK, L"Mass0 ON tO AllJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_OFF_ALL + MENUOFFSET_BONERCLICK, L"Mass0 OFF to AllJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_ON_UPPER + MENUOFFSET_BONERCLICK, L"Mass0 ON to UpperJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_OFF_UPPER + MENUOFFSET_BONERCLICK, L"Mass0 OFF to UpperJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_ON_LOWER + MENUOFFSET_BONERCLICK, L"Mass0 ON to LowerJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0_OFF_LOWER + MENUOFFSET_BONERCLICK, L"Mass0 OFF to LowerJoints");

				if (curbone->GetMass0() == 0){
					AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0 + MENUOFFSET_BONERCLICK, L"Tempolary Mass0 Set");
				}
				else{
					AppendMenu(submenu, MF_STRING, ID_RMENU_MASS0 + MENUOFFSET_BONERCLICK, L"Tempolary Mass0 Unset");
				}

				if (curbone->GetExcludeMv() == 0){
					AppendMenu(submenu, MF_STRING, ID_RMENU_EXCLUDE_MV + MENUOFFSET_BONERCLICK, L"Exclude MV Set");
				}
				else{
					AppendMenu(submenu, MF_STRING, ID_RMENU_EXCLUDE_MV + MENUOFFSET_BONERCLICK, L"Exclude MV Unset");
				}

				AppendMenu(submenu, MF_STRING, ID_RMENU_KINEMATIC_ON_LOWER + MENUOFFSET_BONERCLICK, L"Kinematic ON to LowerJoints");
				AppendMenu(submenu, MF_STRING, ID_RMENU_KINEMATIC_OFF_LOWER + MENUOFFSET_BONERCLICK, L"Kinematic OFF to LowerJoints");

				CRigidElem* curre = s_model->GetRigidElem(s_curboneno);
				int forbidflag = 0;
				if (curre) {
					forbidflag = curre->GetForbidRotFlag();
					if (forbidflag == 0) {
						AppendMenu(submenu, MF_STRING, ID_RMENU_FORBIDROT_ONE + MENUOFFSET_BONERCLICK, L"Forbid Rot Of ParentJoint.");
					}
					else {
						AppendMenu(submenu, MF_STRING, ID_RMENU_ENABLEROT_ONE + MENUOFFSET_BONERCLICK, L"NotForbid Rot Of ParentJoint");
					}


					AppendMenu(submenu, MF_STRING, ID_RMENU_FORBIDROT_CHILDREN + MENUOFFSET_BONERCLICK, L"Forbid Rot Of LowerJoints");
					AppendMenu(submenu, MF_STRING, ID_RMENU_ENABLEROT_CHILDREN + MENUOFFSET_BONERCLICK, L"NotForbid Rot Of LowerJoints");

				}


				AppendMenu(submenu, MF_STRING, ID_RMENU_0 + MENUOFFSET_BONERCLICK, L"CreateNewRig");
				int setmenuno = 1;
				int rigno;
				for (rigno = 0; rigno < MAXRIGNUM; rigno++){
					CUSTOMRIG currig = curbone->GetCustomRig(rigno);
					if (currig.useflag == 2){
						int setmenuid = ID_RMENU_0 + setmenuno + MENUOFFSET_BONERCLICK;

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
						int subsubid3 = setmenuid + MAXRIGNUM * 3;
						AppendMenu(subsubmenu, MF_STRING, subsubid1, L"SettingOfRig");
						AppendMenu(subsubmenu, MF_STRING, subsubid2, L"Execute Rig");
						AppendMenu(subsubmenu, MF_STRING, subsubid3, L"Invalidate Rig");

						setmenuno++;
					}
				}


				POINT pt;
				GetCursorPos(&pt);
				//::ScreenToClient(parwnd, &pt);

				s_cursubmenu = rmenu->GetSubMenu();

				InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
				int currigno = -1;
				int menuid;
				menuid = rmenu->TrackPopupMenu(pt);
				//if (menuid == ID_RMENU_PHYSICSCONSTRAINT){
				//	//位置コンストレイントはMass0で実現する。
				//	////toggle
				//	//if (curbone->GetPosConstraint() == 0){
				//	//	s_model->CreatePhysicsPosConstraint(curbone);
				//	//}
				//	//else{
				//	//	s_model->DestroyPhysicsPosConstraint(curbone);
				//	//}
				//}
				//else if (menuid == ID_RMENU_KINEMATIC_ON_LOWER) {
				//	s_model->SetKinematicTmpLower(curbone, true);
				//}
				//else if (menuid == ID_RMENU_KINEMATIC_OFF_LOWER) {
				//	s_model->SetKinematicTmpLower(curbone, false);
				//}
				//else if (menuid == ID_RMENU_MASS0_ON_ALL) {
				//	s_model->Mass0_All(true);
				//}
				//else if (menuid == ID_RMENU_MASS0_OFF_ALL) {
				//	s_model->Mass0_All(false);
				//}
				//else if (menuid == ID_RMENU_MASS0_ON_UPPER) {
				//	s_model->Mass0_Upper(true, curbone);
				//}
				//else if (menuid == ID_RMENU_MASS0_OFF_UPPER) {
				//	s_model->Mass0_Upper(false, curbone);
				//}
				//else if (menuid == ID_RMENU_MASS0_ON_LOWER) {
				//	s_model->Mass0_Lower(true, curbone);
				//}
				//else if (menuid == ID_RMENU_MASS0_OFF_LOWER) {
				//	s_model->Mass0_Lower(false, curbone);
				//}
				//else if (menuid == ID_RMENU_MASS0){
				//	//toggle
				//	if (curbone->GetMass0() == 0){
				//		s_model->SetMass0(curbone);
				//	}
				//	else{
				//		s_model->RestoreMass(curbone);
				//	}
				//}
				//else if (menuid == ID_RMENU_EXCLUDE_MV){
				//	//toggle
				//	if (curbone->GetExcludeMv() == 0){
				//		curbone->SetExcludeMv(1);
				//	}
				//	else{
				//		curbone->SetExcludeMv(0);
				//	}
				//}
				//else if (menuid == ID_RMENU_FORBIDROT_ONE) {
				//	if (curre) {
				//		curre->SetForbidRotFlag(1);
				//	}
				//}
				//else if (menuid == ID_RMENU_ENABLEROT_ONE) {
				//	if (curre) {
				//		curre->SetForbidRotFlag(0);
				//	}
				//}
				//else if (menuid == ID_RMENU_FORBIDROT_CHILDREN) {
				//	if (curre) {
				//		s_model->EnableRotChildren(curbone, false);
				//	}
				//}
				//else if (menuid == ID_RMENU_ENABLEROT_CHILDREN) {
				//	if (curre) {
				//		s_model->EnableRotChildren(curbone, true);
				//	}
				//}

				//else if (menuid == ID_RMENU_0){
				//	//新規
				//	GUIMenuSetVisible(-1, -1);
				//	currigno = -1;
				//	DispCustomRigDlg(currigno);
				//}
				//else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 2))){
				//	//設定
				//	GUIMenuSetVisible(-1, -1);
				//	currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM)];
				//	DispCustomRigDlg(currigno);

				//}
				//else if ((menuid >= (ID_RMENU_0 + MAXRIGNUM * 2)) && (menuid < (ID_RMENU_0 + MAXRIGNUM * 3))){
				//	//実行
				//	currigno = s_customrigmenuindex[menuid - (ID_RMENU_0 + MAXRIGNUM * 2)];
				//	Bone2CustomRig(currigno);
				//	if (s_customrigbone){
				//		s_oprigflag = 1;
				//	}
				//}
				
				for (rigno = 0; rigno < MAXRIGNUM; rigno++){
					CRMenuMain* curmenu = rsubmenu[rigno];
					if (curmenu){
						curmenu->Destroy();
						delete curmenu;
					}
				}

				rmenu->Destroy();
				delete rmenu;
				InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
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

	SetDlgItemText(hDlgWnd, gpboxid[elemno], (LPCWSTR)L"NotSet");
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_RESETCONTENT, 0, 0);
	wcscpy_s(strcombo, 256, L"RegularBone");
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_ADDSTRING, 0, (LPARAM)strcombo);
	SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_SETCURSEL, 0, 0);


	if (elemno < s_customrig.elemnum){
		RIGELEM currigelem = s_customrig.rigelem[elemno];
		int selrigrigcombono = 0;
		SendMessage(GetDlgItem(hDlgWnd, rigrigcomboid[elemno]), CB_RESETCONTENT, 0, 0);
		wcscpy_s(strcombo, 256, L"RegularBone");
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
		CBone* parentbone = s_customrigbone->GetParent();
		while (parentbone && (parno < MAXRIGELEMNUM) && (parno < levelnum)){
			SetDlgItemText(s_customrigdlg, gpboxid[parno], (LPCWSTR)parentbone->GetWBoneName());
			s_customrig.rigelem[parno].boneno = parentbone->GetBoneNo();
			parentbone = parentbone->GetParent();
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
	//if (s_customrigbone){
		if (s_oprigflag == 0){
			s_oprigflag = 1;

			if (s_BoneMarkCheckBox) {
				s_savebonemarkflag = (int)s_BoneMarkCheckBox->GetChecked();
				s_BoneMarkCheckBox->SetChecked(false);
			}
			if (s_RigidMarkCheckBox) {
				s_saverigidmarkflag = (int)s_RigidMarkCheckBox->GetChecked();
				s_RigidMarkCheckBox->SetChecked(false);
			}

			//s_curboneno = s_customrigbone->GetBoneNo();
			//s_pickinfo.buttonflag = PICK_CENTER;
		}
		else{
			s_oprigflag = 0;
			s_pickinfo.buttonflag = 0;

			if (s_BoneMarkCheckBox) {
				s_BoneMarkCheckBox->SetChecked(s_savebonemarkflag == 1);
			}
			if (s_RigidMarkCheckBox) {
				s_RigidMarkCheckBox->SetChecked(s_saverigidmarkflag == 1);
			}


			//if (s_customrigdlg){
			//	DestroyWindow(s_customrigdlg);
			//	s_customrigdlg = 0;
			//}

			//GUIMenuSetVisible(s_platemenukind, s_platemenuno);

		}
	//}
	//else{
	//	s_oprigflag = 0;
	//	if (s_customrigdlg){
	//		DestroyWindow(s_customrigdlg);
	//		s_customrigdlg = 0;
	//	}
	//	s_pickinfo.buttonflag = 0;

	//	//GUIMenuSetVisible(s_platemenukind, s_platemenuno);
	//}
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
	ret = rmenu->Create(s_3dwnd, MENUOFFSET_GETSYMROOTMODE);
	//ret = rmenu->Create(s_mainhwnd);
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

	//scaleを変えるとtraanimも変わるのでscaleメニューを分けないでおく。
	int setmenuid;
	setmenuid = ID_RMENU_0 + MENUOFFSET_GETSYMROOTMODE;
	AppendMenu(submenu, MF_STRING, setmenuid, L"RootBone:SameToSource");
	setmenuid = ID_RMENU_0 + 1 + MENUOFFSET_GETSYMROOTMODE;
	AppendMenu(submenu, MF_STRING, setmenuid, L"RootBone:SymDirAndSymPosAndSymScale");
	setmenuid = ID_RMENU_0 + 2 + MENUOFFSET_GETSYMROOTMODE;
	AppendMenu(submenu, MF_STRING, setmenuid, L"RootBone:SymDirAndSymSale");
	setmenuid = ID_RMENU_0 + 3 + MENUOFFSET_GETSYMROOTMODE;
	AppendMenu(submenu, MF_STRING, setmenuid, L"RootBone:SymPos");


	POINT pt;
	GetCursorPos(&pt);

	s_cursubmenu = rmenu->GetSubMenu();

	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
	s_getsym_retmode = 0;
	int menuid;
	menuid = rmenu->TrackPopupMenu(pt);
	//if ((menuid >= ID_RMENU_0) && (menuid <= (ID_RMENU_0 + 3))){
	//	switch(menuid){
	//	case (ID_RMENU_0) :
	//		retmode = SYMROOTBONE_SAMEORG;
	//		break;
	//	case (ID_RMENU_0 + 1) :
	//		retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
	//		break;
	//	case (ID_RMENU_0 + 2) :
	//		retmode = SYMROOTBONE_SYMDIR;
	//		break;
	//	case (ID_RMENU_0 + 3) :
	//		retmode = SYMROOTBONE_SYMPOS;
	//		break;
	//	default:
	//		retmode = SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS;
	//		break;
	//	}
	//}

	rmenu->Destroy();
	delete rmenu;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	return s_getsym_retmode;
}

void AutoCameraTarget()
{
	s_camtargetflag = (int)s_CamTargetCheckBox->GetChecked();
	if (s_model && (s_curboneno >= 0) && s_camtargetflag){
		CBone* curbone = s_model->GetBoneByID(s_curboneno);
		_ASSERT(curbone);
		if (curbone){
			g_befcamtargetpos = g_camtargetpos;
			g_camtargetpos = curbone->GetChildWorld();

			g_Camera->SetViewParams(g_camEye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));//!!!!!!!!!!!
			//g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));//!!!!!!!!!!!

			//!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			ChaVector3 diffv;
			diffv = g_camEye - g_camtargetpos;
			s_camdist = (float)ChaVector3LengthDbl(&diffv);

			s_matView = g_Camera->GetViewMatrix();
			s_matProj = g_Camera->GetProjMatrix();
		}
	}
}


/////// for directx9

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
//bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
//	D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
//{
//	//// No fallback defined by this app, so reject any device that doesn't support at least ps2.0
//	//if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
//	//	return false;
//
//	//// Skip backbuffer formats that don't support alpha blending
//	//IDirect3D9* pD3D = DXUTGetD3D9Object();
//	//if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
//	//	AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
//	//	D3DRTYPE_TEXTURE, BackBufferFormat)))
//	//	return false;
//
//	//return true;
//	return false;
//}


//////////////////////////////////////////
int OnTimeLineSelectFromSelectedKey()
{
	if (g_previewFlag != 0) {
		return 0;
	}

	s_editrange.Clear();
	if (s_model && s_model->GetCurMotInfo()) {
		if (s_owpTimeline && s_owpLTimeline && s_owpEulerGraph) {
			s_editrange.SetRange(s_owpLTimeline->getSelectedKey(), s_owpLTimeline->getCurrentTime());
			int keynum;
			double startframe, endframe, applyframe;
			s_editrange.GetRange(&keynum, &startframe, &endframe, &applyframe);

			if (g_underselectingframe != 0) {
				//if (s_buttonselecttothelast == 0) {//tothelastのときも同じ処理
					if (s_buttonselectstart <= s_buttonselectend) {
						s_owpLTimeline->setCurrentTime(endframe, true);
						s_owpEulerGraph->setCurrentTime(endframe, false);
					}
					else {
						s_owpLTimeline->setCurrentTime(startframe, true);
						s_owpEulerGraph->setCurrentTime(startframe, false);
					}
				//}
				//else {
				//	//to the last selectionの際にはカレントをアプライフレームへ
				//	s_owpLTimeline->setCurrentTime(applyframe, true);
				//	s_owpEulerGraph->setCurrentTime(applyframe, false);
				//}
			}
			else {
				s_owpLTimeline->setCurrentTime(applyframe, true);
				s_owpEulerGraph->setCurrentTime(applyframe, false);
				AddEditRangeHistory();
			}
		}
	}

	return 0;
}

int OnTimeLineButtonSelectFromSelectStartEnd(int tothelastflag)
{
	s_buttonselecttothelast = tothelastflag;

	if (s_owpLTimeline && (g_underselectingframe == 0) && s_timelinewheelFlag) {
		s_owpLTimeline->selectClear(false);
		return 0;
	}

	//if ((s_copyKeyInfoList.size() > 0) || tothelastflag) {//2021/11/09 選択済の場合にはそのまま　これがないと１フレーム長選択でOnButtonSelectがループする
	if (s_owpLTimeline) {
		s_owpLTimeline->selectClear(false);//フレームに色付選択していない場合には呼ばれないので再帰ループしない
		//if ((s_buttonselectstart != s_buttonselectend) || tothelastflag) {//tothelastの際には　範囲を指定していなくても実行
		//if ((s_buttonselectstart <= s_buttonselectend) || tothelastflag) {//2021/11/09
			double tmpmaxselectionframe;
			tmpmaxselectionframe = s_owpLTimeline->OnButtonSelect(s_buttonselectstart, s_buttonselectend, s_buttonselecttothelast);
			s_buttonselectend = tmpmaxselectionframe;//tothelast対応
		//}
	}

	OnTimeLineSelectFromSelectedKey();
	//}

	return 0;
}

int OnTimeLineCursorFunc(int mbuttonflag, double newframe)
{
	if (g_previewFlag != 0) {
		return 0;
	}

	if (s_owpLTimeline && s_model && s_model->GetCurMotInfo()) {
		double curframe;
		if (mbuttonflag != 2) {
			curframe = s_owpLTimeline->getCurrentTime();// 選択時刻
			s_owpTimeline->setCurrentTime(curframe, false);
			//s_owpLTimeline->setCurrentTime(curframe, false);
			s_owpEulerGraph->setCurrentTime(curframe, false);
		}
		else {
			curframe = newframe;
			s_owpTimeline->setCurrentTime(curframe, false);
			s_owpLTimeline->setCurrentTime(curframe, false);
			s_owpEulerGraph->setCurrentTime(curframe, false);
		}
	}

	return 0;
}

int OnTimeLineCursor(int mbuttonflag, double newframe)
{
	s_tum.UpdateTimeline(OnTimeLineCursorFunc, mbuttonflag, newframe);//非ブロック

	//if (g_previewFlag != 0){
	//	return 0;
	//}

	//if (s_owpLTimeline && s_model && s_model->GetCurMotInfo()) {
	//	double curframe;
	//	if (mbuttonflag != 2) {
	//		curframe = s_owpLTimeline->getCurrentTime();// 選択時刻
	//		s_owpTimeline->setCurrentTime(curframe, false);
	//		//s_owpLTimeline->setCurrentTime(curframe, false);
	//		s_owpEulerGraph->setCurrentTime(curframe, false);
	//	}
	//	else {
	//		curframe = newframe;
	//		s_owpTimeline->setCurrentTime(curframe, false);
	//		s_owpLTimeline->setCurrentTime(curframe, false);
	//		s_owpEulerGraph->setCurrentTime(curframe, false);
	//	}
	//}

	return 0;
}

int OnTimeLineMButtonDown(bool ctrlshiftflag)
{
	//if (g_underselectingframe == 0){
	if (s_mbuttoncnt == 1) {
		if (ctrlshiftflag == false) {
			g_underselectingframe = 1;
		}
		else {
			g_underselectingframe = 2;
		}
		if (g_previewFlag == 0) {
			if (s_owpLTimeline) {
				s_buttonselectstart = s_owpLTimeline->getCurrentTime();
				s_buttonselectend = s_buttonselectstart;
				s_mbuttonstart = s_buttonselectstart;//2021/11/10
				OnTimeLineCursor(1, 0.0);
			}
		}
	}
	else {
		g_underselectingframe = 0;
		OnTimeLineButtonSelectFromSelectStartEnd(0);

		if (s_editmotionflag < 0) {
			int result = CreateMotionBrush(s_buttonselectstart, s_buttonselectend, false);
			if (result) {
				_ASSERT(0);
			}
		}

	}

	s_mbuttoncnt = (int)(!(s_mbuttoncnt == 1));
	//if (s_mbuttoncnt == 0) {
	//	s_mbuttoncnt = 1;
	//}
	//else {
	//	s_mbuttoncnt = 0;
	//}


	DbgOut(L"OnTimeLineMButtonDown : underselectingframe %d, start %lf, end %lf\r\n", g_underselectingframe, s_buttonselectstart, s_buttonselectend);

	return 0;
}


int OnTimeLineWheel()
{

	DbgOut(L"OnTimeLineWheel Called\r\n");

	if (s_owpLTimeline) {
		if ((g_underselectingframe == 1) || (g_underselectingframe == 2)) {
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
				if (s_buttonselectstart == s_mbuttonstart) {
					s_buttonselectend += delta2;
				}
				else {
					s_buttonselectstart += delta2;
				}
				if (s_buttonselectstart > s_buttonselectend) {
					double tmp;
					tmp = s_buttonselectstart;
					s_buttonselectstart = s_buttonselectend;
					s_buttonselectend = tmp;
				}
				
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
				s_buttonselectstart = newframe;
				s_buttonselectend = newframe;
				//s_editrange.Clear();
				//s_owpLTimeline->selectClear();
				//OnTimeLineCursor(2, newframe);

				OnTimeLineButtonSelectFromSelectStartEnd(0);
			}

		}
	}

	return 0;
}



int InitializeMainWindow(CREATESTRUCT* createWindowArgs)
{
	//TCHAR message[1024];
	//int messageResult;
	//wsprintf(message,
	//	TEXT("ウィンドウクラス:%s\nタイトル:%s\nウィンドウを生成しますか？"),
	//	createWindowArgs->lpszClass, createWindowArgs->lpszName
	//);

	//messageResult = DSMessageBox(NULL, message, TEXT("確認"), MB_YESNO | MB_ICONINFORMATION);

	//if (messageResult == IDNO)
	//	return -1;
	return 0;
}



LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD menuid;
	menuid = LOWORD(wParam);

	LRESULT lret = 0;

	switch (uMsg)
	{
	//case WM_TIMER:
	//	OnTimerFunc(wParam);
	//	break;

	//case WM_LBUTTONDOWN:
	//case WM_RBUTTONDOWN:
	//	SetCapture(s_mainhwnd);
	//	break;
	//case WM_LBUTTONUP:
	//case WM_RBUTTONUP:
	//	ReleaseCapture();
	//	break;
	//case WM_MOUSEMOVE:
	//	OnMouseMoveFunc();
	//	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		//return 0;
		break;
	case WM_CREATE:
		InitializeMainWindow((CREATESTRUCT*)lParam);
		break;
	case WM_COMMAND:
	{
		if ((menuid >= 59900) && (menuid <= (59900 + MAXMOTIONNUM))) {
			ActivatePanel(0);
			int selindex = menuid - 59900;
			OnAnimMenu(true, selindex);
			ActivatePanel(1);
			//return 0;
		}
		else if ((menuid >= 61000) && (menuid <= (61000 + MAXMODELNUM))) {
			ActivatePanel(0);
			int selindex = menuid - 61000;
			OnModelMenu(true, selindex, 1);
			ActivatePanel(1);
			//return 0;
		}
		else if ((menuid >= 62000) && (menuid <= (62000 + MAXRENUM))) {
			ActivatePanel(0);
			int selindex = menuid - 62000;
			OnREMenu(selindex, 1);
			ActivatePanel(1);
			//return 0;
		}
		else if ((menuid >= 63000) && (menuid <= (63000 + MAXRENUM))) {
			ActivatePanel(0);
			int selindex = menuid - 63000;
			OnRgdMenu(selindex, 1);
			ActivatePanel(1);
			//return 0;
		}
		else if ((menuid >= 64000) && (menuid <= (64000 + MAXMOTIONNUM))) {
			ActivatePanel(0);
			int selindex = menuid - 64000;
			OnRgdMorphMenu(selindex);
			ActivatePanel(1);
			//return 0;
		}
		else if ((menuid >= 64500) && (menuid <= (64500 + MAXMOTIONNUM))) {
			ActivatePanel(0);
			int selindex = menuid - 64500;
			OnImpMenu(selindex);
			ActivatePanel(1);
			//return 0;
		}
		else {
			switch (menuid) {
			case ID_40047:
				// "編集・変換"
				// "ボーン軸をXに再計算"
				ActivatePanel(0);
				//RecalcBoneAxisX(0);
				RecalcAxisX_All();
				ActivatePanel(1);
				//return 0;
				break;
			case 29800:
				ActivatePanel(0);
				//RegistKey();
				AboutMotionBrush();
				ActivatePanel(1);
				//return 0;
				break;
			case ID_FILE_EXPORTBNT:
				ActivatePanel(0);
				ExportBntFile();
				ActivatePanel(1);
				//return 0;
				break;
			case ID_FILE_OPEN40001:
				ActivatePanel(0);
				OpenFile();
				ActivatePanel(1);
				//return 0;
				break;
			case ID_FILE_BVH2FBX:
				if (s_registflag == 1) {
					ActivatePanel(0);
					BVH2FBX();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_FILE_BVH2FBXBATCH:
				if (s_registflag == 1) {
					ActivatePanel(0);
					BVH2FBXBatch();
					ActivatePanel(1);
				}
				//return 0;
				break;
			//case ID_FILE_MOTIONCACHE:
			//	if (s_registflag == 1) {
			//		ActivatePanel(0);
			//		MotionCacheBatch();
			//		ActivatePanel(1);
			//	}
			//	//return 0;
			//	break;
			case ID_FILE_RETARGETBATCH:
				if (s_registflag == 1) {
					ActivatePanel(0);
					RetargetBatch();
					ActivatePanel(1);
				}
				//return 0;
				break;

			case ID_SAVE_FBX40039:
				if (s_registflag == 1) {
					ActivatePanel(0);
					ExportFBXFile();
					ActivatePanel(1);
				}
				break;
			case ID_SAVEPROJ_40035:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveProject();
					ActivatePanel(1);
				}
				break;
			case ID_RESAVE_40028:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveREFile();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_IMPSAVE_40030:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveImpFile();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_SAVEGCOLI_40033:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveGcoFile();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_SAVE_MOTIONNAMELIST:
				if (s_registflag == 1) {
					ActivatePanel(0);
					SaveMotionNameListFile();
					ActivatePanel(1);
				}
				//return 0;
				break;
			case ID_DISPMW40002:
				DispMotionWindow();
				//return 0;
				break;
			case 4007:
				DispToolWindow();
				//return 0;
				break;
			case 40012:
			{
				bool savedispflag = s_dispobj;
				s_dispobj = !savedispflag;
				DispObjPanel();
				//return 0;
			}
				break;
			case ID_40048:
				//DispConvBoneWindow();
				s_platemenukind = 2;
				GUIMenuSetVisible(s_platemenukind, 1);
				//return 0;
				break;
			case ID_40049:
				GUIMenuSetVisible(-1, -1);
				DispAngleLimitDlg();
				//return 0;
				break;
			case ID_40050:
				GUIMenuSetVisible(-1, -1);
				DispRotAxisDlg();
				//return 0;
				break;
			case ID_DISPMODELPANEL:
			{
				bool savedispflag = s_dispmodel;
				s_dispmodel = !savedispflag;
				DispModelPanel();
				//return 0;
			}
				break;
			case ID_MOTIONPANEL:
			{
				bool savedispflag = s_dispmotion;
				s_dispmotion = !savedispflag;
				DispMotionPanel();
				//return 0;
			}
				break;
			case ID_SETTINGS:
			{
				CSettingsDlg dlg;
				dlg.DoModal();
			}
			break;

			case ID_DISPGROUND:
				s_dispground = !s_dispground;
				//return 0;
				break;
			case ID_NEWMOT:
				AddMotion(0);
				InitCurMotion(0, 0);
				//return 0;
				break;
			case ID_DELCURMOT:
				if (s_model) {
					OnDelMotion(s_motmenuindexmap[s_model]);
				}
				//return 0;
				break;
			case ID_DELMODEL:
				OnDelModel(s_curmodelmenuindex);
				//return 0;
				break;
			case ID_DELALLMODEL:
				OnDelAllModel();
				//return 0;
				break;
			default:
				break;
			}
		}
	}
	break;

	case WM_MOUSEWHEEL:
	{
		if ((g_keybuf['T'] & 0x80) != 0) {
			if (s_model && (s_curboneno > 0)) {
				s_tkeyflag = 1;

				int delta;
				delta = GET_WHEEL_DELTA_WPARAM(wParam);
				s_editmotionflag = s_model->TwistBoneAxisDelta(&s_editrange, s_curboneno, (float)delta, g_iklevel, s_ikcnt, s_ikselectmat);
			}
		}
	}
	break;

	default:
		//if (uMsg != WM_SETCURSOR) {
			lret = DefWindowProc(hwnd, uMsg, wParam, lParam);
		//}
		break;
	}

	return lret;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lp) {
	WCHAR strWindowText[1024];
	GetWindowTextW(hwnd, strWindowText, 1024);
	if (wcsstr(strWindowText, L"MotionBrushFreeC4") != 0) {
		if (lp) {
			*((HWND*)lp) = hwnd;
		}
		return FALSE;
	}
	else {
		return TRUE;
	}
}


HWND CreateMainWindow()
{
	if (s_mainhwnd && IsWindow(s_mainhwnd)) {
		DestroyWindow(s_mainhwnd);
		s_mainhwnd = 0;
	}
	s_mainhwnd = 0;


	//MotionBrushFC4.exeが起動していればそのウインドウを親にする
	HWND parenthwnd = 0;
	if (s_launchbyc4 != 0) {
		EnumWindows(EnumWindowsProc, (LPARAM)&parenthwnd);
	}


	HWND window;
	WNDCLASSEX wcx;
	int returnCode = 0;

	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wcx.hIcon = NULL;
	wcx.hCursor = NULL;
	wcx.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 1;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = WINDOWS_CLASS_NAME;
	wcx.hIconSm = NULL;

	if (!RegisterClassEx(&wcx))
	{
		OutputDebugString(TEXT("Error: ウィンドウクラスの登録ができません。\n"));
		return NULL;
	}

	HICON appicon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));

	s_mainmenu = LoadMenuW((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
	if (s_mainmenu == NULL) {
		_ASSERT(0);
		return NULL;
	}

	//animmenu
	HMENU motmenu;
	motmenu = GetSubMenu(s_mainmenu, 2);
	s_animmenu = GetSubMenu(motmenu, 3);
	_ASSERT(s_animmenu);

	HMENU mdlmenu = GetSubMenu(s_mainmenu, 3);
	s_modelmenu = GetSubMenu(mdlmenu, 3);
	_ASSERT(s_modelmenu);

	//編集メニュー　4

	s_remenu = GetSubMenu(s_mainmenu, 5);
	_ASSERT(s_remenu);

	s_rgdmenu = GetSubMenu(s_mainmenu, 6);
	_ASSERT(s_rgdmenu);

	s_morphmenu = GetSubMenu(s_mainmenu, 7);
	_ASSERT(s_morphmenu);

	s_impmenu = GetSubMenu(s_mainmenu, 8);
	_ASSERT(s_impmenu);




	WCHAR strwindowname[MAX_PATH] = { 0L };
	swprintf_s(strwindowname, MAX_PATH, L"MotionBrushFree Ver1.0.0.25 : No.%d : ", s_appcnt);

	s_rcmainwnd.top = 0;
	s_rcmainwnd.left = 0;



	window = CreateWindowEx(
		WS_EX_LEFT, WINDOWS_CLASS_NAME, strwindowname,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, s_totalwndwidth, s_totalwndheight,
		parenthwnd, s_mainmenu, (HINSTANCE)GetModuleHandle(NULL), NULL
	);

	s_rcmainwnd.right = s_totalwndwidth;
	s_rcmainwnd.bottom = s_totalwndheight;

	if (!window)
	{
		OutputDebugString(TEXT("Error: ウィンドウが作成できません。\n"));
		return NULL;
	}




	s_mainhwnd = window;

	if ((s_launchbyc4 != 0) && parenthwnd) {
		SetParent(s_mainhwnd, parenthwnd);
		//::MessageBox(s_mainhwnd, L"setparent", L"check!!!", MB_OK);
	}


	HWND desktopwnd;
	desktopwnd = ::GetDesktopWindow();
	if (desktopwnd) {
		RECT desktoprect;
		::GetClientRect(desktopwnd, &desktoprect);
		//if ((s_appcnt == 0) && (desktoprect.right >= 3840) && (desktoprect.bottom >= 2160)) {
		if (s_appcnt == 0) {
			//if (g_4kresolution) {
			//	SetWindowPos(s_mainhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
			//}
			//else {
			//	if (s_launchbyc4 == 0) {
			//		SetWindowPos(s_mainhwnd, HWND_TOP, 1100, 1000, 0, 0, SWP_NOSIZE);
			//	}
			//	else {
			//		SetWindowPos(s_mainhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
			//	}
			//}
			if (s_launchbyc4 == 0) {

				//desktopの中心にメインウインドウの中心が来るように移動

				int desktopcenterx, desktopcentery;
				desktopcenterx = (desktoprect.left + desktoprect.right) / 2;
				desktopcentery = (desktoprect.top + desktoprect.bottom) / 2;

				int currentcenterx, currentcentery;
				currentcenterx = s_totalwndwidth / 2;
				currentcentery = s_totalwndheight / 2;

				int diffx, diffy;
				diffx = desktopcenterx - currentcenterx;
				diffy = desktopcentery - currentcentery;

				SetWindowPos(s_mainhwnd, HWND_TOP, diffx, diffy, 0, 0, SWP_NOSIZE);

			}
			else {
				SetWindowPos(s_mainhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
			}

		}
		else if (s_appcnt == 1) {
			SetWindowPos(s_mainhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
		}
		else if (s_appcnt == 2) {
			SetWindowPos(s_mainhwnd, HWND_TOP, s_totalwndwidth, 0, 0, 0, SWP_NOSIZE);
		}
		else if (s_appcnt == 3) {
			SetWindowPos(s_mainhwnd, HWND_TOP, 0, s_totalwndheight, 0, 0, SWP_NOSIZE);
		}
		else if (s_appcnt == 4) {
			SetWindowPos(s_mainhwnd, HWND_TOP, s_totalwndwidth, s_totalwndheight, 0, 0, SWP_NOSIZE);
		}
	}

	return window;

}

HWND Create3DWnd()
{
	HRESULT hr;

	if (s_3dwnd && IsWindow(s_3dwnd)) {
		DestroyWindow(s_3dwnd);
		s_3dwnd = 0;
	}
	s_3dwnd = 0;

	//if (g_4kresolution) {
	//	hr = DXUTCreateWindow(L"MameBake3D", 0, 0, 0, 450 * 2, 0);
	//}
	//else {
	//	hr = DXUTCreateWindow(L"MameBake3D", 0, 0, 0, 450, 0);
	//}
	

	if (g_4kresolution) {
		hr = DXUTCreateWindow(L"MameBake3D", 0, 0, 0, s_toolwidth, 0);
	}
	else {
		hr = DXUTCreateWindow(L"MameBake3D", 0, 0, 0, s_timelinewidth, 0);
	}

	if (FAILED(hr)) {
		_ASSERT(0);
		return 0;
	}

	s_3dwnd = DXUTGetHWND();
	_ASSERT(s_3dwnd);
	RECT clientrect;
	GetClientRect(s_3dwnd, &clientrect);
	s_bufwidth = clientrect.right;
	s_bufheight = clientrect.bottom;


	LONG winstyle = ::GetWindowLong(s_3dwnd, GWL_STYLE);
	winstyle &= ~WS_CAPTION;
	::SetWindowLong(s_3dwnd, GWL_STYLE, winstyle);


	SetParent(s_3dwnd, s_mainhwnd);


	//int cycaption = GetSystemMetrics(SM_CYCAPTION);
	//int cymenu = GetSystemMetrics(SM_CYMENU);
	//int cyborder = GetSystemMetrics(SM_CYBORDER);
	//int bufwidth = s_mainwidth;
	//int bufheight = s_mainheight - cycaption - cymenu - cyborder;

	//hr = DXUTCreateDevice(true);//mac + VM Fusionの場合はこっち
	//hr = DXUTCreateDevice(true, bufwidth, bufheight);
	//hr = DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, s_mainwidth, s_mainheight);
	hr = DXUTCreateDevice(D3D_FEATURE_LEVEL_11_1, true, s_mainwidth, s_mainheight);
	if (FAILED(hr)) {
		_ASSERT(0);
		return 0;
	}
	s_3dwnd = DXUTGetHWND();
	_ASSERT(s_3dwnd);
	RECT clientrect2;
	GetClientRect(s_3dwnd, &clientrect2);


	//RECT clientrect;
	//GetClientRect(s_3dwnd, &clientrect);
	//s_mainwidth = clientrect.right;
	//s_mainheight = clientrect.bottom;
	//ShowWindow( s_3dwnd, SW_SHOW );
	//SetWindowPos( s_3dwnd, HWND_TOP, 450, 0, s_mainwidth, s_mainheight, SWP_NOSIZE ); 

	RECT winrect;
	::GetWindowRect(s_3dwnd, &winrect);
	//::MoveWindow(s_3dwnd, 400, 0, winrect.right - winrect.left, winrect.bottom - winrect.top, TRUE);
	//::MoveWindow(s_3dwnd, 400, 0, s_mainwidth, s_mainheight, TRUE);
	if (g_4kresolution) {
		::MoveWindow(s_3dwnd, s_toolwidth, MAINMENUAIMBARH, s_mainwidth, s_mainheight, TRUE);
	}
	else {
		::MoveWindow(s_3dwnd, s_timelinewidth, MAINMENUAIMBARH, s_mainwidth, s_mainheight, TRUE);
	}
	

	s_rc3dwnd.top = MAINMENUAIMBARH;
	s_rc3dwnd.left = 0;
	s_rc3dwnd.right = s_mainwidth;
	s_rc3dwnd.bottom = (s_mainheight + MAINMENUAIMBARH);


	//#############################################################################
	//次のコメントアウトブロックはDXUTのウインドウにメニューを付けるときに使用する。
	//#############################################################################
	//最大化してから元に戻すことにより
	//バックバッファの大きさ問題（メニューやキャプションがあるときのずれ）が解消される。
	//::ShowWindow(s_3dwnd, SW_MAXIMIZE);
	//::ShowWindow(s_3dwnd, SW_SHOWNORMAL);


	return s_3dwnd;
}

CInfoWindow* CreateInfoWnd()
{
	if (g_infownd) {
		delete g_infownd;
		g_infownd = 0;
	}

	int cxframe = GetSystemMetrics(SM_CXFRAME);
	int cyframe = GetSystemMetrics(SM_CYFRAME);

	s_rcinfownd.top = s_mainheight + 3 * cyframe + MAINMENUAIMBARH;
	s_rcinfownd.bottom = (s_infowinheight + 2 * cyframe);
	s_rcinfownd.right = s_infowinwidth;
	s_rcinfownd.left = 400;

	CInfoWindow* newinfownd = new CInfoWindow();
	if (newinfownd) {

		int ret;
		//if (g_4kresolution) {
		//	ret = newinfownd->CreateInfoWindow(s_mainhwnd,
		//		400 * 2, s_mainheight + 3 * cyframe + MAINMENUAIMBARH,
		//		s_infowinwidth, s_infowinheight + 2 * cyframe);

		//	s_rcinfownd.left = 400 * 2;
		//}
		//else {
		//	ret = newinfownd->CreateInfoWindow(s_mainhwnd,
		//		400, s_mainheight + 3 * cyframe + MAINMENUAIMBARH,
		//		s_infowinwidth, s_infowinheight + 2 * cyframe);

		//	s_rcinfownd.left = 400;
		//}

		if (g_4kresolution) {
			ret = newinfownd->CreateInfoWindow(s_mainhwnd,
				s_toolwidth, s_mainheight + 3 * cyframe + MAINMENUAIMBARH,
				s_infowinwidth, s_infowinheight + 2 * cyframe);

			s_rcinfownd.left = s_toolwidth;
		}
		else {
			ret = newinfownd->CreateInfoWindow(s_mainhwnd,
				s_timelinewidth, s_mainheight + 3 * cyframe + MAINMENUAIMBARH,
				s_infowinwidth, s_infowinheight + 2 * cyframe);

			s_rcinfownd.left = s_timelinewidth;
		}


		if (ret == 0) {
			g_infownd = newinfownd;

			//OutputToInfoWnd(L"InfoWindow initialized 1");
			OutputToInfoWnd(L"Upper to lower, older to newer. Limit to 6,000 lines.");
			OutputToInfoWnd(L"Scroll is enable by mouse wheel.");
			OutputToInfoWnd(L"If the most newest line is shown at lowest position, AutoScroll works.Save to info_(date).txt on exit.");
			OutputToInfoWnd(L"上：古,下：新。6,000行。ホイールでスクロール。一番新しいものを表示している時AutoScroll。終了時にinfo_日時.txtにセーブ。");
		}

	}



	return newinfownd;
}


int RecalcBoneAxisX(CBone* srcbone)
{
	if (s_model && (s_model->GetOldAxisFlagAtLoading() == 1)) {
		::DSMessageBox(s_3dwnd, L"Retry After Saving And Loading.", L"error!!!", MB_OK);
		return 0;
	}

	s_model->RecalcBoneAxisX(srcbone);

	return 0;
}

void RecalcAxisX_All()
{
	if (s_model) {
		if (s_model && (s_model->GetOldAxisFlagAtLoading() == 1)) {
			::DSMessageBox(s_3dwnd, L"Retry After Saving And Loding.", L"error!!!", MB_OK);
			return;
		}

		s_owpLTimeline->setCurrentTime(0.0, true);
		s_owpEulerGraph->setCurrentTime(0.0, false);
		s_model->SetMotionFrame(0.0);
		s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);

		//ここでAxisMatXの初期化
		s_model->CreateBtObject(1);
		s_model->CalcBtAxismat(2);//2
		s_model->SetInitAxisMatX(1);
	}
}


int OnMouseMoveFunc()
{
	static bool s_doingflag = false;
	if (s_doingflag == true) {
		return 0;
	}
	s_doingflag = true;

	if (s_pickinfo.buttonflag == PICK_CENTER) {
		if (s_model) {
			if (g_previewFlag == 0) {

				s_pickinfo.mousebefpos = s_pickinfo.mousepos;
				POINT ptCursor;
				GetCursorPos(&ptCursor);
				::ScreenToClient(s_3dwnd, &ptCursor);
				s_pickinfo.mousepos = ptCursor;

				ChaVector3 tmpsc;
				s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

				if (s_model) {
					if (s_oprigflag == 0) {
						ChaVector3 targetpos(0.0f, 0.0f, 0.0f);
						//CallF(CalcTargetPos(&targetpos), return 1);
						CalcTargetPos(&targetpos);
						if (s_ikkind == 0) {
							s_editmotionflag = s_model->IKRotate(&s_editrange, s_pickinfo.pickobjno, targetpos, g_iklevel);
						}
						else if (s_ikkind == 1) {
							ChaVector3 diffvec = targetpos - s_pickinfo.objworld;
							AddBoneTra2(diffvec);
							s_editmotionflag = s_curboneno;
						}
						else if (s_ikkind == 2) {
							ChaVector3 diffvec = targetpos - s_pickinfo.objworld;
							AddBoneScale2(diffvec);
							s_editmotionflag = s_curboneno;
						}

					}
					else {
						if (s_customrigbone) {
							float deltau = (float)(s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) * 0.5f;
							float deltav = (float)(s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y) * 0.5f;
							if (g_controlkey == true) {
								deltau *= 0.250f;
								deltav *= 0.250f;
							}

							s_ikcustomrig = s_customrigbone->GetCustomRig(s_customrigno);

							s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 0, deltau, s_ikcustomrig);
							s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
							s_model->RigControl(0, &s_editrange, s_pickinfo.pickobjno, 1, deltav, s_ikcustomrig);
							s_model->UpdateMatrix(&s_model->GetWorldMat(), &s_matVP);
							//s_editmotionflag = s_curboneno;
							s_editmotionflag = 0;
						}
					}
					s_ikcnt++;
				}
			}
			else if (g_previewFlag == 5) {
				if (s_model) {
					//if (s_onragdollik == 0){
					//	StartBt(1, 1);
					//}
					s_onragdollik = 1;
				}
			}
		}
	}
	else if ((s_pickinfo.buttonflag == PICK_X) || (s_pickinfo.buttonflag == PICK_Y) || (s_pickinfo.buttonflag == PICK_Z)) {
		if (s_model) {
			if (g_previewFlag == 0) {
				s_pickinfo.mousebefpos = s_pickinfo.mousepos;
				POINT ptCursor;
				GetCursorPos(&ptCursor);
				::ScreenToClient(s_3dwnd, &ptCursor);
				s_pickinfo.mousepos = ptCursor;

				ChaVector3 tmpsc;
				s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

				if (g_previewFlag == 0) {
					float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.5f;
					if (g_controlkey == true) {
						deltax *= 0.250f;
					}
					if (s_ikkind == 0) {
						s_editmotionflag = s_model->IKRotateAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, g_iklevel, s_ikcnt, s_ikselectmat);
					}
					else if(s_ikkind == 1){
						AddBoneTra(s_pickinfo.buttonflag - PICK_X, deltax * 0.1f);
						s_editmotionflag = s_curboneno;
					}
					else if (s_ikkind == 2) {
						AddBoneScale(s_pickinfo.buttonflag - PICK_X, deltax);
						s_editmotionflag = s_curboneno;
					}
					s_ikcnt++;
				}
			}
			else if (g_previewFlag == 5) {
				if (s_model) {
					s_onragdollik = 2;
				}
			}
		}
	}
	else if ((s_pickinfo.buttonflag == PICK_SPA_X) || (s_pickinfo.buttonflag == PICK_SPA_Y) || (s_pickinfo.buttonflag == PICK_SPA_Z)) {
		if (s_model) {
			if (g_previewFlag == 0) {
				s_pickinfo.buttonflag = s_pickinfo.buttonflag - PICK_SPA_X + PICK_X;

				s_pickinfo.mousebefpos = s_pickinfo.mousepos;
				POINT ptCursor;
				GetCursorPos(&ptCursor);
				::ScreenToClient(s_3dwnd, &ptCursor);
				s_pickinfo.mousepos = ptCursor;

				ChaVector3 tmpsc;
				s_model->TransformBone(s_pickinfo.winx, s_pickinfo.winy, s_curboneno, &s_pickinfo.objworld, &tmpsc, &s_pickinfo.objscreen);

				if (g_previewFlag == 0) {
					float deltax = (float)((s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y)) * 0.5f;
					if (g_controlkey == true) {
						deltax *= 0.250f;
					}

					if (s_ikkind == 0) {
						s_editmotionflag = s_model->IKRotateAxisDelta(&s_editrange, s_pickinfo.buttonflag, s_pickinfo.pickobjno, deltax, g_iklevel, s_ikcnt, s_ikselectmat);
					}
					else if(s_ikkind == 1){
						AddBoneTra(s_pickinfo.buttonflag - PICK_X, deltax * 0.1f);
						s_editmotionflag = s_curboneno;
					}
					else if (s_ikkind == 2) {
						AddBoneScale(s_pickinfo.buttonflag - PICK_X, deltax);
						s_editmotionflag = s_curboneno;
					}
					s_ikcnt++;
				}
			}
			else if (g_previewFlag == 5) {
				if (s_model) {
					s_onragdollik = 3;
				}
			}
		}
	}
	else if (s_pickinfo.buttonflag == PICK_CAMMOVE) {

		s_pickinfo.mousebefpos = s_pickinfo.mousepos;
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_3dwnd, &ptCursor);
		s_pickinfo.mousepos = ptCursor;

		ChaVector3 cammv;
		cammv.x = ((float)s_pickinfo.mousepos.x - (float)s_pickinfo.mousebefpos.x) / (float)s_pickinfo.winx * -s_cammvstep;
		cammv.y = ((float)s_pickinfo.mousepos.y - (float)s_pickinfo.mousebefpos.y) / (float)s_pickinfo.winy * s_cammvstep;
		cammv.z = 0.0f;
		if (g_controlkey == true) {
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

		g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), newat.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

		g_befcamEye = g_camEye;
		g_befcamtargetpos = g_camtargetpos;
		g_camEye = neweye;
		g_camtargetpos = newat;
		//!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		ChaVector3 diffv;
		diffv = neweye - newat;
		s_camdist = (float)ChaVector3LengthDbl(&diffv);


	}
	else if (s_pickinfo.buttonflag == PICK_CAMROT) {

	//not use quaternion yet int this part, so ジンバルロック未回避.

		s_pickinfo.mousebefpos = s_pickinfo.mousepos;
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_3dwnd, &ptCursor);
		s_pickinfo.mousepos = ptCursor;

		float roty, rotxz;
		rotxz = -((float)s_pickinfo.mousepos.x - (float)s_pickinfo.mousebefpos.x) / (float)s_pickinfo.winx * 250.0f;
		roty = ((float)s_pickinfo.mousepos.y - (float)s_pickinfo.mousebefpos.y) / (float)s_pickinfo.winy * 250.0f;
		if (g_controlkey == true) {
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
		if (fabs(chkdot) < 0.99965f) {
			ChaVector3Cross(&rotaxisxz, (const ChaVector3*)&upvec, (const ChaVector3*)&viewvec);
			ChaVector3Normalize(&rotaxisxz, &rotaxisxz);

			ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
			ChaVector3Normalize(&rotaxisy, &rotaxisy);
			//}
			//else if (chkdot >= 0.99965f) {
			//	rotaxisxz = upvec;
			//	ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
			//	ChaVector3Normalize(&rotaxisy, &rotaxisy);
			//	//rotxz = 0.0f;
			//	//if (roty < 0.0f) {
			//	//	roty = 0.0f;
			//	//}
			//	//else {
			//	//}
			//}
			//else {
			//	rotaxisxz = upvec;
			//	ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
			//	ChaVector3Normalize(&rotaxisy, &rotaxisy);
			//	//rotxz = 0.0f;
			//	//if (roty > 0.0f) {
			//	//	roty = 0.0f;
			//	//}
			//	//else {
			//	//	//rotyだけ回す。
			//	//}
			//}


			if (s_model && (s_curboneno >= 0) && s_camtargetflag) {
				CBone* curbone = s_model->GetBoneByID(s_curboneno);
				_ASSERT(curbone);
				if (curbone) {
					g_befcamtargetpos = g_camtargetpos;
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

			//float chkdot2;
			//ChaVector3 newviewvec = weye - neweye;
			//ChaVector3Normalize(&newviewvec, &newviewvec);
			//chkdot2 = ChaVector3Dot(&newviewvec, &upvec);
			//if (fabs(chkdot2) < 0.99965f) {
			//	ChaVector3Cross(&rotaxisxz, (const ChaVector3*)&upvec, (const ChaVector3*)&viewvec);
			//	ChaVector3Normalize(&rotaxisxz, &rotaxisxz);

			//	ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
			//	ChaVector3Normalize(&rotaxisy, &rotaxisy);
			//}
			//else {
			//	roty = 0.0f;
			//	rotaxisxz = upvec;
			//	ChaVector3Cross(&rotaxisy, (const ChaVector3*)&viewvec, (const ChaVector3*)&rotaxisxz);
			//	ChaVector3Normalize(&rotaxisy, &rotaxisy);
			//}
			//ChaMatrixRotationAxis(&rotmaty, &rotaxisy, rotxz * (float)DEG2PAI);
			//ChaMatrixRotationAxis(&rotmatxz, &rotaxisxz, roty * (float)DEG2PAI);
			//mat = befrotmat * rotmatxz * rotmaty * aftrotmat;
			//ChaVector3TransformCoord(&neweye, &weye, &mat);

			g_Camera->SetViewParams(neweye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
			s_matView = g_Camera->GetViewMatrix();
			s_matProj = g_Camera->GetProjMatrix();

			g_befcamEye = g_camEye;
			g_camEye = neweye;
			//!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
			//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

			ChaVector3 diffv;
			diffv = neweye - g_camtargetpos;
			s_camdist = (float)ChaVector3LengthDbl(&diffv);

		}
		else {
			g_camEye = g_befcamEye;
			g_camtargetpos = g_befcamtargetpos;
		}
	}
	else if (s_pickinfo.buttonflag == PICK_CAMDIST) {
		s_pickinfo.mousebefpos = s_pickinfo.mousepos;
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		::ScreenToClient(s_3dwnd, &ptCursor);
		s_pickinfo.mousepos = ptCursor;

		float deltadist = (float)(s_pickinfo.mousepos.x - s_pickinfo.mousebefpos.x) + (s_pickinfo.mousepos.y - s_pickinfo.mousebefpos.y) * 0.5f;
		//float mdelta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
		//float deltadist = mdelta * s_camdist * 0.0010f;
		if (g_controlkey == true) {
			deltadist *= 0.250f;
		}

		s_camdist += deltadist;
		if (s_camdist < 0.0001f) {
			s_camdist = 0.0001f;
		}

		ChaVector3 camvec = g_camEye - g_camtargetpos;
		ChaVector3Normalize(&camvec, &camvec);
		g_befcamEye = g_camEye;
		g_camEye = g_camtargetpos + camvec * s_camdist;
		//!!!!!!!!!ChaMatrixLookAtRH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);
		//ChaMatrixLookAtLH(&s_matView, &g_camEye, &g_camtargetpos, &s_camUpVec);

		g_Camera->SetViewParams(g_camEye.XMVECTOR(1.0f), g_camtargetpos.XMVECTOR(1.0f));
		s_matView = g_Camera->GetViewMatrix();
		s_matProj = g_Camera->GetProjMatrix();

	}

	s_doingflag = false;

	return 0;
}

void GUIRetargetSetVisible(int srcplateno)
{
	if (srcplateno == 1) {
		ShowRetargetWnd(true);
		ShowLimitEulerWnd(false);
	}
	else if (srcplateno == 2) {
		ShowRetargetWnd(false);
		ShowLimitEulerWnd(true);
	}
	else if (srcplateno == -2) {
		ShowRetargetWnd(false);
		ShowLimitEulerWnd(false);
	}
	else {
		_ASSERT(0);
	}
}


void GUIRigidSetVisible(int srcplateno)
{
	if (srcplateno == 1) {
		ShowRigidWnd(true);
		ShowImpulseWnd(false);
		ShowGroundWnd(false);
		ShowDampAnimWnd(false);
	}
	else if (srcplateno == 2) {
		ShowRigidWnd(false);
		ShowImpulseWnd(true);
		ShowGroundWnd(false);
		ShowDampAnimWnd(false);
	}
	else if (srcplateno == 3) {
		ShowRigidWnd(false);
		ShowImpulseWnd(false);
		ShowGroundWnd(true);
		ShowDampAnimWnd(false);
	}
	else if (srcplateno == 4) {
		ShowRigidWnd(false);
		ShowImpulseWnd(false);
		ShowGroundWnd(false);
		ShowDampAnimWnd(true);
	}
	else if(srcplateno == -2){
		ShowRigidWnd(false);
		ShowImpulseWnd(false);
		ShowGroundWnd(false);
		ShowDampAnimWnd(false);
	}
	else {
		_ASSERT(0);
	}
}



void GUISetVisible(int srcplateno)
{
	if (srcplateno == 1) {
		//srcplateno == 1 --> PlaceFolderWnd visible
	}
	else if (srcplateno == 2) {
		GUISetVisible_SpriteFK();
	}
	else if (srcplateno == 3) {
		GUISetVisible_Left();
	}
	else if (srcplateno == 4) {
		GUISetVisible_Left2nd();
	}
	else if (srcplateno == 5) {
		GUISetVisible_Bullet();
	}
	else if (srcplateno == 6) {
		GUISetVisible_PhysicsIK();
	}
	else {
		_ASSERT(0);
	}
}

void GUISetVisible_Sel3D()
{
	if (s_currentwndid == 1) {
		s_spsel3d.state = true;
	}
	else {
		s_spsel3d.state = false;
	}
}


// g_currentsubmenuid
void GUISetVisible_MenuAimBar()
{
	//static int s_curaimbarno
	if ((g_currentsubmenuid >= 0) && (g_currentsubmenuid < SPMENU_MAX)) {
		int aimno;
		for (aimno = 0; aimno < SPMENU_MAX; aimno++) {
			s_spmenuaimbar[aimno].state = false;
		}

		s_spmenuaimbar[g_currentsubmenuid].state = true;

	}
}

void GUISetVisible_AimBar()
{
	//static int s_curaimbarno
	if ((s_curaimbarno >= 0) && (s_curaimbarno < SPAIMBARNUM)) {
		int aimno;
		for (aimno = 0; aimno < SPAIMBARNUM; aimno++) {
			s_spaimbar[aimno].state = false;
		}

		s_spaimbar[s_curaimbarno].state = true;

	}
}

void GUISetVisible_SpriteFK()
{
	bool nextvisible = !(s_spguisw[SPGUISW_SPRITEFK].state);

	//SpritePlateは常に５枚表示

	//FK用のスプライトの表示のオンオフフラグの設定

	s_spguisw[SPGUISW_SPRITEFK].state = nextvisible;
}

void GUISetVisible_Left()
{
	bool nextvisible = !(s_spguisw[SPGUISW_LEFT].state);

	if (s_ui_lightscale) {
		s_ui_lightscale->SetVisible(nextvisible);
	}
	if (s_ui_dispbone) {
		s_ui_dispbone->SetVisible(nextvisible);
	}
	if (s_ui_disprigid) {
		s_ui_disprigid->SetVisible(nextvisible);
	}
	if (s_ui_boneaxis) {
		s_ui_boneaxis->SetVisible(nextvisible);
	}
	if (s_ui_bone) {
		s_ui_bone->SetVisible(nextvisible);
	}
	if (s_ui_locktosel) {
		s_ui_locktosel->SetVisible(nextvisible);
	}
	if (s_ui_iklevel) {
		s_ui_iklevel->SetVisible(nextvisible);
	}
	if (s_ui_editmode) {
		s_ui_editmode->SetVisible(nextvisible);
	}
	if (s_ui_texapplyrate) {
		s_ui_texapplyrate->SetVisible(nextvisible);
	}
	if (s_ui_slapplyrate) {
		s_ui_slapplyrate->SetVisible(nextvisible);
	}
	if (s_ui_motionbrush) {
		s_ui_motionbrush->SetVisible(nextvisible);
	}
	if (s_ui_texikorder) {
		s_ui_texikorder->SetVisible(nextvisible);
	}
	if (s_ui_slikorder) {
		s_ui_slikorder->SetVisible(nextvisible);
	}
	//if (s_ui_texikrate) {
	//	s_ui_texikrate->SetVisible(nextvisible);
	//}
	//if (s_ui_slikrate) {
	//	s_ui_slikrate->SetVisible(nextvisible);
	//}
	if (s_ui_texref) {
		s_ui_texref->SetVisible(nextvisible);
	}
	if (s_ui_slirefpos) {
		s_ui_slirefpos->SetVisible(nextvisible);
	}
	if (s_ui_slirefmult) {
		s_ui_slirefmult->SetVisible(nextvisible);
	}
	if (s_ui_applytotheend) {
		s_ui_applytotheend->SetVisible(nextvisible);
	}
	if (s_ui_slerpoff) {
		s_ui_slerpoff->SetVisible(nextvisible);
	}


	if (s_ui_texbrushrepeats) {
		s_ui_texbrushrepeats->SetVisible(nextvisible);
	}
	if (s_ui_brushrepeats) {
		s_ui_brushrepeats->SetVisible(nextvisible);
	}
	if (s_ui_brushmirroru) {
		s_ui_brushmirroru->SetVisible(nextvisible);
	}
	if (s_ui_brushmirrorv) {
		s_ui_brushmirrorv->SetVisible(nextvisible);
	}
	if (s_ui_ifmirrorvdiv2) {
		s_ui_ifmirrorvdiv2->SetVisible(nextvisible);
	}

	s_spguisw[SPGUISW_LEFT].state = nextvisible;

}
void GUISetVisible_Left2nd()
{
	bool nextvisible = !(s_spguisw[SPGUISW_LEFT2ND].state);

	
	
	//if (s_ui_pseudolocal) {
	//	s_ui_pseudolocal->SetVisible(nextvisible);
	//}
	if (s_ui_wallscrapingik) {
		s_ui_wallscrapingik->SetVisible(nextvisible);
	}
	if (s_ui_limiteul) {
		s_ui_limiteul->SetVisible(nextvisible);
	}
	if (s_ui_texspeed) {
		s_ui_texspeed->SetVisible(nextvisible);
	}
	if (s_ui_speed) {
		s_ui_speed->SetVisible(nextvisible);
	}
	if (s_ui_absikon) {
		s_ui_absikon->SetVisible(nextvisible);
	}

	s_spguisw[SPGUISW_LEFT2ND].state = nextvisible;
}
void GUISetVisible_Bullet()
{
	bool nextvisible = !(s_spguisw[SPGUISW_BULLETPHYSICS].state);

	if (s_ui_texthreadnum) {
		s_ui_texthreadnum->SetVisible(nextvisible);
	}
	if (s_ui_slthreadnum) {
		s_ui_slthreadnum->SetVisible(nextvisible);
	}
	if (s_ui_btstart) {
		s_ui_btstart->SetVisible(nextvisible);
	}
	if (s_ui_btrecstart) {
		s_ui_btrecstart->SetVisible(nextvisible);
	}
	if (s_ui_stopbt) {
		s_ui_stopbt->SetVisible(nextvisible);
	}
	if (s_ui_texbtcalccnt) {
		s_ui_texbtcalccnt->SetVisible(nextvisible);
	}
	if (s_ui_btcalccnt) {
		s_ui_btcalccnt->SetVisible(nextvisible);
	}
	if (s_ui_texerp) {
		s_ui_texerp->SetVisible(nextvisible);
	}
	if (s_ui_erp) {
		s_ui_erp->SetVisible(nextvisible);
	}


	s_spguisw[SPGUISW_BULLETPHYSICS].state = nextvisible;
}
void GUISetVisible_PhysicsIK()
{
	bool nextvisible = !(s_spguisw[SPGUISW_PHYSICSIK].state);

	if (s_ui_texphysmv) {
		s_ui_texphysmv->SetVisible(nextvisible);
	}
	if (s_ui_slphysmv) {
		s_ui_slphysmv->SetVisible(nextvisible);
	}
	if (s_ui_physrotstart) {
		s_ui_physrotstart->SetVisible(nextvisible);
	}
	if (s_ui_physmvstart) {
		s_ui_physmvstart->SetVisible(nextvisible);
	}
	if (s_ui_physikstop) {
		s_ui_physikstop->SetVisible(nextvisible);
	}

	s_spguisw[SPGUISW_PHYSICSIK].state = nextvisible;
}

void ShowRetargetWnd(bool srcflag)
{
	if (s_model && (s_curboneno >= 0)) {
		if (s_bpWorld) {
			if (srcflag == true) {
				//if (!s_convboneWnd) {
					CreateConvBoneWnd();
				//}
				s_convboneWnd->setListenMouse(true);
				s_convboneWnd->setVisible(true);
				s_spretargetsw[SPRETARGETSW_RETARGET].state = true;
				s_dispconvbone = true;
			}
			else {
				if (s_convboneWnd) {
					s_convboneWnd->setListenMouse(false);
					s_convboneWnd->setVisible(false);
				}
				s_spretargetsw[SPRETARGETSW_RETARGET].state = false;
				s_dispconvbone = false;
			}
		}
	}
}

void ShowLimitEulerWnd(bool srcflag)
{
	if (s_model && (s_curboneno >= 0)) {
		if (s_bpWorld) {
			if (srcflag == true) {
				if (s_anglelimitdlg) {
					s_underanglelimithscroll = 0;
					DestroyWindow(s_anglelimitdlg);
					s_anglelimitdlg = 0;
				}

				DispAngleLimitDlg();

				s_spretargetsw[1].state = true;
			}
			else {
				if (s_anglelimitdlg) {
					s_underanglelimithscroll = 0;
					DestroyWindow(s_anglelimitdlg);
					s_anglelimitdlg = 0;
				}
				s_spretargetsw[1].state = false;
			}
		}
	}
}


void ShowRigidWnd(bool srcflag)
{
	if (s_model && (s_curboneno >= 0)) {
	//if (s_model) {
		if (s_bpWorld) {
			s_model->SetCurrentRigidElem(s_reindexmap[s_model]);

			//CallF(s_model->CreateBtObject(0), return);
			CallF(s_model->CreateBtObject(1), return);


			//s_impWnd->setVisible(0);
			//s_gpWnd->setVisible(0);
			//s_dmpanimWnd->setVisible(0);

			//s_ikkind = 3;
			s_rigidWnd->setVisible(srcflag);
			SetRigidLeng();
			RigidElem2WndParam();

			s_sprigidsw[SPRIGIDTSW_RIGIDPARAMS].state = srcflag;
			//s_sprigidsw[1].state = false;
			//s_sprigidsw[2].state = false;
			//s_sprigidsw[3].state = false;
		}
	}
}

void ShowImpulseWnd(bool srcflag)
{
	//if (s_model && (s_curboneno >= 0)) {
	if (s_model) {
		if (s_bpWorld) {
			CallF(s_model->CreateBtObject(0), return);

			//s_rigidWnd->setVisible(0);
			//s_gpWnd->setVisible(0);
			//s_dmpanimWnd->setVisible(0);

			//s_ikkind = 4;
			s_impWnd->setVisible(srcflag);

			SetImpWndParams();
			s_impWnd->callRewrite();

			//s_sprigidsw[0].state = false;
			s_sprigidsw[SPRIGIDSW_IMPULSE].state = srcflag;
			//s_sprigidsw[2].state = false;
			//s_sprigidsw[3].state = false;

		}
	}
}
void ShowGroundWnd(bool srcflag)
{
	//if (s_model && (s_curboneno >= 0)) {
	if (s_model) {
		if (s_bpWorld) {
			CallF(s_model->CreateBtObject(0), return);

			//s_rigidWnd->setVisible(0);
			//s_impWnd->setVisible(0);
			//s_dmpanimWnd->setVisible(0);


			//s_ikkind = 5;
			s_gpWnd->setVisible(srcflag);

			SetGpWndParams();
			s_gpWnd->callRewrite();

			//s_sprigidsw[0].state = false;
			//s_sprigidsw[1].state = false;
			s_sprigidsw[SPRIGIDSW_GROUNDPLANE].state = srcflag;
			//s_sprigidsw[3].state = false;
		}
	}
}
void ShowDampAnimWnd(bool srcflag)
{
	//if (s_model && (s_curboneno >= 0)) {
	if (s_model) {
		if (s_bpWorld) {
			CallF(s_model->CreateBtObject(0), return);

			//s_rigidWnd->setVisible(0);
			//s_impWnd->setVisible(0);
			//s_gpWnd->setVisible(0);

			//s_ikkind = 6;
			s_dmpanimWnd->setVisible(srcflag);

			SetDmpWndParams();
			s_dmpanimWnd->callRewrite();

			//s_sprigidsw[0].state = false;
			//s_sprigidsw[1].state = false;
			//s_sprigidsw[2].state = false;
			s_sprigidsw[SPRIGIDSW_DAMPANIM].state = srcflag;
		}
	}
}

void GUIMenuSetVisible(int srcmenukind, int srcplateno)
{
	if ((srcmenukind >= SPPLATEMENUKIND_GUI) && (srcmenukind <= SPPLATEMENUKIND_RETARGET)) {

		//platemenu用のウインドウ以外を閉じるまたは破棄する
		if (s_copyhistorydlg.GetCreatedFlag() == true) {
			s_copyhistorydlg.ShowWindow(SW_HIDE);
		}
		if (s_anglelimitdlg) {
			s_underanglelimithscroll = 0;
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

		
		//プレート
		s_platemenukind = srcmenukind;

		switch (s_platemenukind) {
		case SPPLATEMENUKIND_GUI:
			if ((srcplateno >= 1) && (srcplateno < (SPGUISWNUM + 2))) {
				GUIRigidSetVisible(-2);
				GUIRetargetSetVisible(-2);
				GUISetVisible(srcplateno);//((spgno == 0) && (spgno < SPGUISWNUM))でGUISetVisible(spgno + 2)でGUISetVisible(1)はPlaceFolderWindow用
				if (s_placefolderWnd) {
					s_placefolderWnd->setVisible(true);
				}
				SelectNextWindow(MB3D_WND_3D);
			}
			else {
				GUIRigidSetVisible(-2);
				GUIRetargetSetVisible(-2);
				//GUISetVisible(-2);
				if (s_placefolderWnd) {
					s_placefolderWnd->setVisible(true);
				}
				SelectNextWindow(MB3D_WND_SIDE);
			}
			break;
		case SPPLATEMENUKIND_RIGID:
			if ((srcplateno >= 1) && (srcplateno <= SPRIGIDSWNUM)) {
				GUIRetargetSetVisible(-2);
				if (s_customrigdlg) {
					DestroyWindow(s_customrigdlg);
					s_customrigdlg = 0;
				}
				if (s_placefolderWnd) {
					s_placefolderWnd->setVisible(false);
				}
				GUIRigidSetVisible(srcplateno);
				SelectNextWindow(MB3D_WND_SIDE);
			}
			else {
				_ASSERT(0);
			}
			break;
		case SPPLATEMENUKIND_RETARGET:
			if ((srcplateno >= 1) && (srcplateno <= SPRETARGETSWNUM)) {
				GUIRigidSetVisible(-2);
				if (s_customrigdlg) {
					DestroyWindow(s_customrigdlg);
					s_customrigdlg = 0;
				}
				if (s_placefolderWnd) {
					s_placefolderWnd->setVisible(false);
				}
				GUIRetargetSetVisible(srcplateno);
				SelectNextWindow(MB3D_WND_SIDE);
			}
			break;
		default:
			break;
		}
	}
	else if (srcmenukind == -1) {
		if (s_placefolderWnd) {
			s_placefolderWnd->setVisible(false);
		}
		GUIRetargetSetVisible(-2);
		GUIRigidSetVisible(-2);
		SelectNextWindow(MB3D_WND_3D);
	}
	else {
		_ASSERT(0);
		return;
	}

}


void GUIGetNextMenu(POINT ptCursor, int srcmenukind, int* dstmenukind, int* dstplateno)
{
	if ((srcmenukind >= 0) && (srcmenukind <= 2)) {

		int nextmenukind = srcmenukind;
		int nextplateno = 1;

		//int spckind = 0;
		int pickguiplateno = 0;
		int pickrigidplateno = 0;
		int pickretargetplateno = 0;
		if (srcmenukind == SPPLATEMENUKIND_GUI) {
			pickguiplateno = PickSpGUISW(ptCursor);//カエルボタンを押したときは -2, (SPGUISW_* + 2)が返る
			if (pickguiplateno == -2) {
				nextmenukind = SPPLATEMENUKIND_RIGID;
				nextplateno = 1;//最初のプレート
			}
			else if (pickguiplateno >= 2) {
				nextmenukind = srcmenukind;
				nextplateno = pickguiplateno;
			}
			else {
				nextmenukind = -1;
				nextplateno = 1;
			}
		}
		else if (srcmenukind == SPPLATEMENUKIND_RIGID) {
			pickrigidplateno = PickSpRigidSW(ptCursor);//カエルボタンを押したときは -2
			if (pickrigidplateno == -2) {
				nextmenukind = SPPLATEMENUKIND_RETARGET;
				nextplateno = 1;//最初のプレート
			}
			else if (pickrigidplateno != 0) {
				nextmenukind = srcmenukind;
				nextplateno = pickrigidplateno;
			}
			else {
				nextmenukind = -1;
				nextplateno = 1;
			}
		}
		else if (srcmenukind == SPPLATEMENUKIND_RETARGET) {
			pickretargetplateno = PickSpRetargetSW(ptCursor);//カエルボタンを押したときは -2
			if (pickretargetplateno == -2) {
				nextmenukind = SPPLATEMENUKIND_GUI;
				nextplateno = 1;//最初のプレート
			}
			else if (pickretargetplateno != 0) {
				nextmenukind = srcmenukind;
				nextplateno = pickretargetplateno;
			}
			else {
				nextmenukind = -1;
				nextplateno = 1;
			}
		}

		*dstmenukind = nextmenukind;
		*dstplateno = nextplateno;
	}
	else {
		_ASSERT(0);
		*dstmenukind = -1;
		*dstplateno = 1;
	}
}

void InitDSValues()
{
	//static int s_currentwndid = 0;
	//static HWND s_currenthwnd = 0;
	//static int s_currentctrlid = -1;
	//static HWND s_currentctrlhwnd = 0;

	//static int s_dsbuttondown[MB3D_DSBUTTONNUM];
	//static int s_bef_dsbuttondown[MB3D_DSBUTTONNUM];
	//static int s_dsbuttonup[MB3D_DSBUTTONNUM];
	//static int s_bef_dsbuttonup[MB3D_DSBUTTONNUM];
	//static float s_dsaxisvalue[MB3D_DSAXISNUM];
	//static float s_bef_dsaxisvalue[MB3D_DSAXISNUM];
	//static int s_dsaxisOverSrh[MB3D_DSAXISNUM];
	//static int s_bef_dsaxisOverSrh[MB3D_DSAXISNUM];
	//static int s_dsaxisMOverSrh[MB3D_DSAXISNUM];
	//static int s_bef_dsaxisMOverSrh[MB3D_DSAXISNUM];

	g_enableDS = false;
	s_dsdeviceid = -1;
	s_curaimbarno = -1;

	s_wmlbuttonup = 0;
	s_dspushedOK = 0;
	s_dspushedL3 = 0;
	s_dspushedR3 = 0;
	g_dsmousewait = 0;
	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);

	s_firstmoveaimbar = true;

	s_mqodlghwnd = 0;
	s_underframecopydlg = false;
	s_pcolidlg = 0;
	s_undercolidlg = false;
	s_pgcolidlg = 0;
	s_undergcolidlg = false;
	s_motpropdlghwnd = 0;
	s_savechadlghwnd = 0;
	s_bvhdlghwnd = 0;
	s_saveredlghwnd = 0;
	s_saveimpdlghwnd = 0;
	s_savegcodlghwnd = 0;
	s_rotzisdlghwnd = 0;
	s_customrighwnd = 0;
	s_exportxdlghwnd = 0;

	s_enumdist.clear();


	s_currentwndid = 0;
	s_currenthwnd = 0;
	s_restorewndid = 0;
	s_restorehwnd = 0;
	s_currentctrlid = -1;
	s_currentctrlhwnd = 0;
	s_wmlbuttonup = 0;
	s_restorecursorpos.x = 0;
	s_restorecursorpos.y = 0;

	s_ofhwnd = 0;
	s_messageboxhwnd = 0;
	s_messageboxpushcnt = 0;
	s_getfilenamehwnd = 0;
	s_getfilenametreeview = 0;

	s_curdsutguikind = 0;
	s_curdsutguino = 0;
	s_dsutgui0.clear();
	s_dsutgui1.clear();
	s_dsutgui2.clear();
	s_dsutgui3.clear();
	s_dsutguiid0.clear();
	s_dsutguiid1.clear();
	s_dsutguiid2.clear();
	s_dsutguiid3.clear();

	s_curdstoolctrlno = 0;
	s_dstoolctrls.clear();

	s_curdsplayerbtnno = 0;

	s_curdsrigidctrlno = 0;
	s_dsrigidctrls.clear();

	s_curdsimpulsectrlno = 0;
	s_dsimpulsectrls.clear();

	s_curdsgpctrlno = 0;
	s_dsgpctrls.clear();

	s_curdsdampctrlno = 0;
	s_dsdampctrls.clear();

	s_curdsretargetctrlno = 0;
	s_dsretargetctrls.clear();;

	s_curdseullimitctrlno = 0;
	s_dseullimitctrls.clear();

	g_currentsubmenuid = 0;
	s_currentsubmenuitemid = 0;

	ZeroMemory(s_dsbuttondown, sizeof(int) * MB3D_DSBUTTONNUM);
	ZeroMemory(s_bef_dsbuttondown, sizeof(int) * MB3D_DSBUTTONNUM);
	ZeroMemory(s_dsbuttonup, sizeof(int) * MB3D_DSBUTTONNUM);
	ZeroMemory(s_bef_dsbuttonup, sizeof(int) * MB3D_DSBUTTONNUM);

	ZeroMemory(s_dsaxisvalue, sizeof(float) * MB3D_DSAXISNUM);
	ZeroMemory(s_bef_dsaxisvalue, sizeof(float) * MB3D_DSAXISNUM);

	ZeroMemory(s_dsaxisOverSrh, sizeof(int) * MB3D_DSAXISNUM);
	ZeroMemory(s_bef_dsaxisOverSrh, sizeof(int) * MB3D_DSAXISNUM);
	ZeroMemory(s_dsaxisMOverSrh, sizeof(int) * MB3D_DSAXISNUM);
	ZeroMemory(s_bef_dsaxisMOverSrh, sizeof(int) * MB3D_DSAXISNUM);


}
void OnDSUpdate()
{

	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合にはDS関数でキーボードの矢印キーに対応

		OnArrowKey();

		return;
	}


	GetDSValues();

	ChangeMouseSetCapture();//処理前にキャプチャーをセット



	DSColorAndVibration();

	DSSelectWindowAndCtrl();//L1, square, triangle
	DSSelectCharactor();//(L2 or R2) and L1

	bool firstctrlselect = false;
	DSCrossButton(firstctrlselect);


	//R1ボタン：３Dウインドウ選択、カレントボーン位置へマウスジャンプ
	DSR1ButtonSelectCurrentBone();
	DSR1ButtonSelectMotion();

	//L3, R3ボタンでマウス位置アピール
	DSL3R3ButtonMouseHere();


	if (g_undertrackingRMenu == 0) {
		//OK button popupmenuを出していないとき
		DSAimBarOK();

		//optionボタンは右クリック相当
		DSOptionButtonRightClick();
	}
	else if (g_undertrackingRMenu == 1) {
		//OK button popupmenuを出しているとき
		DSOButtonSelectedPopupMenu();
	}

	//Cancel button : メニューのドロップダウンをキャンセルする　Cancel dropdown menu. L2 + X --> Undo, R2 + X --> Redo.
	DSXButtonCancel();

	//Axis L Mouse Move
	DSAxisLMouseMove();

	//Axis R MenuBar of MainWindow
	DSAxisRMainMenuBar();


	ChangeMouseReleaseCapture();//処理が終わってからキャプチャーを外す

	//OutputToInfoWnd(L"\n\n");
	//OutputToInfoWnd(L"Axis 0 : %1.4f\n", axisval0);
	//OutputToInfoWnd(L"Axis 1 : %1.4f\n", axisval1);
	//OutputToInfoWnd(L"Axis 2 : %1.4f\n", axisval2);
	//OutputToInfoWnd(L"Axis 3 : %1.4f\n", axisval3);
	//OutputToInfoWnd(L"Axis 4 : %1.4f\n", axisval4);
	//OutputToInfoWnd(L"Axis 5 : %1.4f\n", axisval5);

	//ChangeColor(int id, const UCHAR r, const UCHAR g, const UCHAR b);

}

void GetDSValues()
{
	//static int s_dsbuttondown[MB3D_DSBUTTONNUM];
	//static int s_bef_dsbuttondown[MB3D_DSBUTTONNUM];
	//static int s_dsbuttonup[MB3D_DSBUTTONNUM];
	//static int s_bef_dsbuttonup[MB3D_DSBUTTONNUM];
	//static float s_dsaxisvalue[MB3D_DSAXISNUM];
	//static float s_bef_dsaxisvalue[MB3D_DSAXISNUM];
	//static int s_dsaxisOverSrh[MB3D_DSAXISNUM];
	//static int s_bef_dsaxisOverSrh[MB3D_DSAXISNUM];
	//static int s_dsaxisMOverSrh[MB3D_DSAXISNUM];
	//static int s_bef_dsaxisMOverSrh[MB3D_DSAXISNUM];


	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	UpdateInputReport();
	SendOutput(0);

	//bool GetButtonDown(int id, UCHAR key);
	//bool GetButtonUp(int id, UCHAR key);

	int buttonno;
	for (buttonno = 0; buttonno < MB3D_DSBUTTONNUM; buttonno++) {
		s_bef_dsbuttondown[buttonno] = s_dsbuttondown[buttonno];
		s_bef_dsbuttonup[buttonno] = s_dsbuttonup[buttonno];

		if (GetButtonDown(s_dsdeviceid, buttonno)) {
			s_dsbuttondown[buttonno] = GetButtonDown(s_dsdeviceid, buttonno);
		}
		else {
			s_dsbuttondown[buttonno] = 0;
		}

		if (GetButtonUp(s_dsdeviceid, buttonno)) {
			s_dsbuttonup[buttonno] = GetButtonUp(s_dsdeviceid, buttonno);
		}
		else {
			s_dsbuttonup[buttonno] = 0;
		}

	}

	int axisno;
	for (axisno = 0; axisno < MB3D_DSAXISNUM; axisno++) {

		////for debug
		//{
		//	if (s_dsbuttonup[4] >= 1) {
		//		_ASSERT(0);
		//	}
		//}


		s_bef_dsaxisvalue[axisno] = s_dsaxisvalue[axisno];
		s_bef_dsaxisOverSrh[axisno] = s_dsaxisOverSrh[axisno];
		s_bef_dsaxisMOverSrh[axisno] = s_dsaxisMOverSrh[axisno];

		s_dsaxisvalue[axisno] = GetAxis(s_dsdeviceid, axisno);

		s_dsaxisOverSrh[axisno] = (int)(s_dsaxisvalue[axisno] >= MB3D_DSAXISSRH);
		s_dsaxisMOverSrh[axisno] = (int)(s_dsaxisvalue[axisno] <= -(MB3D_DSAXISSRH));
	}


	if (s_dsbuttondown[2] >= 1) {
		s_dspushedOK = 1;
	}
	if (s_dsbuttonup[2] >= 1) {
		s_dspushedOK = 0;
	}


	if (s_dsbuttondown[12] >= 1) {
		s_dspushedL3 = 1;
	}
	if (s_dsbuttonup[12] >= 1) {
		s_dspushedL3 = 0;
	}
	if (s_dsbuttondown[13] >= 1) {
		s_dspushedR3 = 1;
	}
	if (s_dsbuttonup[13] >= 1) {
		s_dspushedR3 = 0;
	}



}

void DSColorAndVibration()
{
	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}


	int buttonno;
	//for (buttonno = 0; buttonno < MB3D_DSBUTTONNUM - 2; buttonno++) {//L3, R3は別処理！！！！！
	for (buttonno = 0; buttonno < MB3D_DSBUTTONNUM; buttonno++) {//L3, R3は別処理！！！！！
		int curbuttondown = s_dsbuttondown[buttonno];
		int curbuttonup = s_dsbuttonup[buttonno];

		if (curbuttondown >= 1) {
			ChangeColor(s_dsdeviceid, 255, 0, 0);
			//ChangeVibration(s_dsdeviceid, 200, 200);
			//OutputToInfoWnd(L"Button Down %d", buttonno);
		}
		else if (curbuttonup >= 1) {
			ChangeColor(s_dsdeviceid, 0, 0, 255);
			//ChangeVibration(s_dsdeviceid, 0, 0);
			//OutputToInfoWnd(L"Button Up %d", buttonno);
		}
	}

	bool existon = false;
	static bool s_bef_existon = false;


	int axisno;
	for (axisno = 0; axisno < MB3D_DSAXISNUM; axisno++) {
		bool curbuttondown = ((bool)s_dsaxisOverSrh[axisno] || (bool)s_dsaxisMOverSrh[axisno]);
		if (curbuttondown) {
			existon = true;
			break;
		}
	}
	if (existon) {
		ChangeColor(s_dsdeviceid, 255, 0, 0);
		//ChangeVibration(s_dsdeviceid, 200, 200);
		//OutputToInfoWnd(L"Axis On %d", axisno);
	}else if((existon == false) && (s_bef_existon == true)){
		ChangeColor(s_dsdeviceid, 0, 0, 255);
		//ChangeVibration(s_dsdeviceid, 0, 0);
		//OutputToInfoWnd(L"Axis Off %d", axisno);
	}

	s_bef_existon = existon;

}

void DSR1ButtonSelectMotion()
{
	//R1ボタン：３Dウインドウ選択、カレントボーン位置へマウスジャンプ
	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	if (!s_model) {
		//モデル読み込み前は処理しないでリターン
		return;
	}

	if (!s_3dwnd) {
		return;
	}


	int buttonR1 = 9;
	int curbuttondown = s_dsbuttondown[buttonR1];
	int curbuttonup = s_dsbuttonup[buttonR1];

	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;
	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;
	accelbothflag = accelaxis1 && accelaxis2;

	if ((accelflag != 0) && (curbuttonup >= 1)) {//アクセル有り
		int cAnimSets = (int)s_tlarray.size();
		int nextmotionindex;
		nextmotionindex = s_motmenuindexmap[s_model] + 1;
	
		if (nextmotionindex < 0) {
			nextmotionindex = cAnimSets - 1;
		}
		else if (nextmotionindex >= cAnimSets) {
			nextmotionindex = 0;
		}

		OnAnimMenu(true, nextmotionindex);
	}

}


void DSR1ButtonSelectCurrentBone()
{
	//R1ボタン：３Dウインドウ選択、カレントボーン位置へマウスジャンプ
	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	if (!s_model) {
		//モデル読み込み前は処理しないでリターン
		return;
	}

	if (!s_3dwnd) {
		return;
	}

	//###################################################
	//R1ボタンを押して、jointとprev_selected_windowを往復する
	//###################################################

	int buttonR1 = 9;
	int curbuttondown = s_dsbuttondown[buttonR1];
	int curbuttonup = s_dsbuttonup[buttonR1];

	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;
	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;
	accelbothflag = accelaxis1 && accelaxis2;

	if ((accelflag == 0) && (curbuttonup >= 1)) {//アクセル無し
		
		::SetWindowPos(s_3dwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		if (s_mainmenuaimbarWnd) {
			s_mainmenuaimbarWnd->setBackGroundColor(false);
		}

		if (s_timelineWnd) {
			s_timelineWnd->setBackGroundColor(false);
		}
		if (s_toolWnd) {
			s_toolWnd->setBackGroundColor(false);
		}
		if (s_LtimelineWnd) {
			s_LtimelineWnd->setBackGroundColor(false);
		}
		if (s_sidemenuWnd) {
			s_sidemenuWnd->setBackGroundColor(false);
		}


		if ((s_restorewndid <= 0) || !s_restorehwnd) {
			if (s_model && (s_curboneno >= 0)) {
				CBone* boneptr = s_model->GetBoneByID(s_curboneno);
				if (boneptr) {
					ChaVector3 jointpos;
					jointpos.x = s_selectmat._41;
					jointpos.y = s_selectmat._42;
					jointpos.z = s_selectmat._43;

					ChaMatrix bcmat;
					bcmat = boneptr->GetCurMp().GetWorldMat();
					ChaMatrix transmat = bcmat * s_matVP;
					ChaVector3 scpos;
					ChaVector3 firstpos = boneptr->GetJointFPos();
					ChaVector3TransformCoord(&scpos, &firstpos, &transmat);
					scpos.z = 0.0f;
					POINT mousepos;
					mousepos.x = (LONG)((scpos.x + 1.0f) * 0.5f * s_mainwidth);
					mousepos.y = (LONG)((-scpos.y + 1.0f) * 0.5f * s_mainheight);

					s_restorewndid = s_currentwndid;
					s_restorehwnd = s_currenthwnd;
					::GetCursorPos(&s_restorecursorpos);
					s_currentwndid = MB3D_WND_3D;
					s_currenthwnd = s_3dwnd;

					GUISetVisible_Sel3D();//3DWindowを選択しているかどうかのマークを右上隅に表示

					::ClientToScreen(s_3dwnd, &mousepos);
					::SetCursorPos(mousepos.x, mousepos.y);

				}
			}
		}
		else {
			int nextwndid;
			nextwndid = s_restorewndid;

			SelectNextWindow(nextwndid);

			::SetCursorPos(s_restorecursorpos.x, s_restorecursorpos.y);

			//!!!!!!!!!!!!!!!!!!!!!!!!!!
			s_restorewndid = 0;
			s_restorehwnd = 0;
		}
	}
}


void SelectNextWindow(int nextwndid)
{
	HWND tmptlwnd;
	if (s_timelineWnd) {
		tmptlwnd = s_timelineWnd->getHWnd();
	}
	else {
		tmptlwnd = 0;
	}
	HWND tmptoolwnd;
	if (s_toolWnd) {
		tmptoolwnd = s_toolWnd->getHWnd();
	}
	else {
		tmptoolwnd = 0;
	}
	HWND tmplongtlwnd;
	if (s_LtimelineWnd) {
		tmplongtlwnd = s_LtimelineWnd->getHWnd();
	}
	else {
		tmplongtlwnd = 0;
	}
	HWND tmpsidewnd;
	if (s_sidemenuWnd) {
		tmpsidewnd = s_sidemenuWnd->getHWnd();
	}
	else {
		tmpsidewnd = 0;
	}
	HWND tmpmainmenuaimbarwnd;
	if (s_mainmenuaimbarWnd) {
		tmpmainmenuaimbarwnd = s_mainmenuaimbarWnd->getHWnd();
	}
	else {
		tmpmainmenuaimbarwnd = 0;
	}


	HWND hwnds[MB3D_WND_MAX];
	//hwnds[MB3D_WND_MAIN] = s_mainhwnd;
	hwnds[MB3D_WND_MAIN] = tmpmainmenuaimbarwnd;
	hwnds[MB3D_WND_3D] = s_3dwnd;
	hwnds[MB3D_WND_TREE] = tmptlwnd;
	hwnds[MB3D_WND_TOOL] = tmptoolwnd;
	hwnds[MB3D_WND_TIMELINE] = tmplongtlwnd;
	hwnds[MB3D_WND_SIDE] = tmpsidewnd;

	int dbgcnt = 0;
	HWND nexthwnd = 0;
	nexthwnd = hwnds[nextwndid];
	//while (nexthwnd == 0) {
	//	nextwndid++;
	//	if (nextwndid >= MB3D_WND_MAX) {
	//		nextwndid = 0;
	//	}
	//	dbgcnt++;
	//	if (dbgcnt >= MB3D_WND_MAX) {
	//		nextwndid = 0;
	//		break;
	//	}
	//	nexthwnd = hwnds[nextwndid];
	//}

	if ((nextwndid >= 0) && (nextwndid < MB3D_WND_MAX) && nexthwnd) {

		BYTE selectR = 255;
		BYTE selectG = 128;
		BYTE selectB = 64;

		BYTE unselectR = 70;
		BYTE unselectG = 50;
		BYTE unselectB = 70;

		HBRUSH selectbrush = CreateSolidBrush(RGB(selectR, selectG, selectB));
		HBRUSH unselectbrush = CreateSolidBrush(RGB(unselectR, unselectG, unselectB));

		::SetFocus(nexthwnd);

		if (nextwndid == MB3D_WND_MAIN) {
			//:: SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)selectbrush);
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			//SetThemeAppProperties(STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);//STAP_ALLOW_WEBCONTENT
			//SendMessage(hwnds[0], WM_THEMECHANGED, 0, 0);
			//RedrawWindow(hwnds[0], 0, 0, RDW_UPDATENOW);


			//::SetWindowPos(hwnds[0], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			::SetWindowPos(s_mainhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(true);
			}

			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(false);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(false);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(false);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(false);
			}
		}
		else if (nextwndid == MB3D_WND_3D) {
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)selectbrush);
			//::SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			//SetThemeAppProperties(STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);//STAP_ALLOW_WEBCONTENT
			//SendMessage(hwnds[1], WM_THEMECHANGED, 0, 0);
			//RedrawWindow(hwnds[1], 0, 0, RDW_UPDATENOW);

			::SetWindowPos(hwnds[1], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(false);
			}


			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(false);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(false);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(false);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(false);
			}
		}
		else if (nextwndid == MB3D_WND_TREE) {
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);
			//::SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(false);
			}

			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(true);
				::SetWindowPos(nexthwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(false);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(false);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(false);
			}
		}
		else if (nextwndid == MB3D_WND_TOOL) {
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);
			//::SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(false);
			}

			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(false);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(true);
				::SetWindowPos(nexthwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(false);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(false);
			}
		}
		else if (nextwndid == MB3D_WND_TIMELINE) {
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);
			//::SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(false);
			}

			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(false);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(false);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(true);
				::SetWindowPos(nexthwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(false);
			}
		}
		else if (nextwndid == MB3D_WND_SIDE) {
			//::SetClassLongPtr(hwnds[1], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);
			//::SetClassLongPtr(hwnds[0], GCLP_HBRBACKGROUND, (LONG_PTR)unselectbrush);

			if (s_mainmenuaimbarWnd) {
				s_mainmenuaimbarWnd->setBackGroundColor(false);
			}

			if (s_timelineWnd) {
				s_timelineWnd->setBackGroundColor(false);
			}
			if (s_toolWnd) {
				s_toolWnd->setBackGroundColor(false);
			}
			if (s_LtimelineWnd) {
				s_LtimelineWnd->setBackGroundColor(false);
			}
			if (s_sidemenuWnd) {
				s_sidemenuWnd->setBackGroundColor(true);
				::SetWindowPos(nexthwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
		}
		else {
			_ASSERT(0);
		}




		DeleteObject(selectbrush);
		DeleteObject(unselectbrush);


		//if (s_befactivehwnd) {
		//	ReleaseCapture();
		//}
		//SetCapture(nexthwnd);

		//SetForegroundWindow(nexthwnd);

		//if (s_befactivehwnd) {
		//	ReleaseCapture();
		//}
		//::SetWindowPos(nexthwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		//HWND befactive;
		//befactive = ::SetActiveWindow(nexthwnd);
		//SetCapture(nexthwnd);


		//SetCaptureWindow !!!!!!!!!!!!!!!!!

		s_currentwndid = nextwndid;
		if ((s_currentwndid >= 0) && (s_currentwndid < MB3D_WND_MAX)) {
			s_currenthwnd = hwnds[s_currentwndid];
		}

		bool firstctrlselect = true;
		DSCrossButton(firstctrlselect);

		GUISetVisible_Sel3D();//3DWindowを選択しているかどうかのマークを右上隅に表示

	}
	//OutputToInfoWnd(L"Button Down %d", buttonL1);
}

void DSSelectCharactor()
{
	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	if (!s_model) {
		//モデル読み込み前は処理しないでリターン
		return;
	}

	if (!s_3dwnd) {
		return;
	}

	int buttonL1 = 8;
	int curbuttondown = s_dsbuttondown[buttonL1];
	int curbuttonup = s_dsbuttonup[buttonL1];


	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;
	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;
	accelbothflag = accelaxis1 && accelaxis2;

	//L1 Button Up（L2, R2 not pushed）
	if ((accelflag != 0) && (curbuttonup >= 1)) {
		int modelnum;
		modelnum = (int)s_modelindex.size();
		if (modelnum > 0) {
			int nextmodelindex;
			nextmodelindex = s_curmodelmenuindex + 1;
			if (nextmodelindex < 0) {
				nextmodelindex = modelnum - 1;
			}
			else if (nextmodelindex >= modelnum) {
				nextmodelindex = 0;
			}

			s_modelpanel.modelindex = nextmodelindex;
			OnModelMenu(true, s_modelpanel.modelindex, 1);
			s_modelpanel.panel->callRewrite();

		}
	}
}

void DSSelectWindowAndCtrl()
{

	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	if (!s_model) {
		//モデル読み込み前は処理しないでリターン
		return;
	}

	if (!s_3dwnd) {
		return;
	}

	//static HWND s_befactivehwnd = 0;

	
	bool doneflag = false;

//ウインドウ選択ブロック
	{
		//static int s_currentwndid = 0;
		//s_currentwndid = 0;
		//s_currenthwnd = 0;
		//s_currentctrlid = 0;
		//s_currentctrlhwnd = 0;

		int buttonL1 = 8;
		int curbuttondown = s_dsbuttondown[buttonL1];
		int curbuttonup = s_dsbuttonup[buttonL1];


		int accelaxisid1 = 4;//axisid
		int accelaxisid2 = 5;//axisid
		bool accelaxis1 = 0;
		bool accelaxis2 = 0;
		bool accelflag = false;
		bool accelbothflag = false;
		accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
		accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
		accelflag = accelaxis1 || accelaxis2;
		accelbothflag = accelaxis1 && accelaxis2;

		//L1 Button Up（L2, R2 not pushed）
		if ((accelflag == 0) && (curbuttonup >= 1)) {
			int nextwndid = 0;
			nextwndid = s_currentwndid + 1;

			if (nextwndid >= MB3D_WND_MAX) {
				nextwndid = 0;
			}
			if (nextwndid < 0) {
				nextwndid = MB3D_WND_MAX - 1;
			}

			SelectNextWindow(nextwndid);//マウスカーソルをプレート位置に移動する前に呼ぶ（この関数ではコントロール位置にマウスは移動する）

			doneflag = true;
		}
	}

	//R1モードに関係なく
	//カエルボタン　プレートメニュー選択ブロック
	//R1モードによって機能が変わる場合には別関数にする
	if (!doneflag) {
		int frogbuttonid = 3;
		int platebuttonid = 0;
		int frogbutton;
		int platebutton;
		frogbutton = s_dsbuttonup[frogbuttonid];
		platebutton = s_dsbuttonup[platebuttonid];


		int platemenukind = s_platemenukind;
		int platemenuno = s_platemenuno;
		int nextplatemenukind = 0;
		int nextplateno = -1;

		//frogButton ret2prev
		if ((frogbutton >= 1) && (s_spret2prev.sprite != 0)) {
			POINT frogbuttonpos;
			frogbuttonpos = s_spret2prev.dispcenter;

			GUIGetNextMenu(frogbuttonpos, platemenukind, &nextplatemenukind, &nextplateno);
			if ((nextplatemenukind >= 0) && (nextplateno >= 0)) {
				s_platemenukind = nextplatemenukind;
				s_platemenuno = nextplateno;
				GUIMenuSetVisible(s_platemenukind, nextplateno);

				s_curaimbarno = 0;
				GUISetVisible_AimBar();

				if (s_platemenukind == SPPLATEMENUKIND_GUI) {
					SelectNextWindow(MB3D_WND_3D);
				}
				else if (s_platemenukind == SPPLATEMENUKIND_RIGID) {
					SelectNextWindow(MB3D_WND_SIDE);
				}
				else if (s_platemenukind == SPPLATEMENUKIND_RETARGET) {
					SelectNextWindow(MB3D_WND_SIDE);
				}
				else {
					_ASSERT(0);
				}

				doneflag = true;
			}
		}

		/*
		#define SPAXISNUM	3
		//#define SPCAMNUM	3	//Coef.h : SPR_CAM_MAX
		#define SPRIGMAX	2
		#define SPGUISWNUM	5
		#define SPRIGIDSWNUM	4
		#define SPRETARGETSWNUM	2
		#define SPAIMBARNUM	5
		*/

		//move aimbar
		if ((platebutton >= 1) && (s_curaimbarno >= 0)) {
			if ((platemenukind >= 0) && (platemenuno >= 0)) {
				int nextaimbarno;
				if (s_firstmoveaimbar) {
					nextaimbarno = s_curaimbarno;
					s_firstmoveaimbar = false;
				}
				else {
					nextaimbarno = s_curaimbarno + 1;
				}

				if (platemenukind == SPPLATEMENUKIND_GUI) {
					if (nextaimbarno >= SPGUISWNUM) {
						nextaimbarno = 0;
					}
				}
				else if (platemenukind == SPPLATEMENUKIND_RIGID) {
					if (nextaimbarno >= SPRIGIDSWNUM) {
						nextaimbarno = 0;
					}
				}
				else if (platemenukind == SPPLATEMENUKIND_RETARGET) {
					if (nextaimbarno >= SPRETARGETSWNUM) {
						nextaimbarno = 0;
					}
				}
				else {
					nextaimbarno = -1;
				}


				if ((nextaimbarno >= 0) && (nextaimbarno < SPAIMBARNUM)) {
					s_curaimbarno = nextaimbarno;


					SelectNextWindow(MB3D_WND_3D);//マウスカーソルをプレート位置に移動する前に呼ぶ（この関数ではコントロール位置にマウスは移動する）

					POINT buttonpos;
					buttonpos = s_spaimbar[s_curaimbarno].dispcenter;
					buttonpos.y -= (28 / 2 + 6 / 2);
					ClientToScreen(s_3dwnd, &buttonpos);
					::SetCursorPos(buttonpos.x, buttonpos.y);

					GUISetVisible_AimBar();
				}
			}
		}
	}


}

void DSCrossButton(bool firstctrlselect)
{

	if (!g_enableDS || (s_dsdeviceid < 0) || (s_dsdeviceid >= 3)) {
		//DS deviceが無い場合には何もせずにリターン
		return;
	}

	//選択ウインドウ依存
	//十字キー処理
	{
		if (s_currentwndid == MB3D_WND_3D) {
			DSCrossButtonSelectUTGUI(firstctrlselect);
		}
		else if (s_currentwndid == MB3D_WND_TREE) {
			DSCrossButtonSelectTree(firstctrlselect);
		}
		else if (s_currentwndid == MB3D_WND_TOOL) {
			DSCrossButtonSelectToolCtrls(firstctrlselect);
		}
		else if (s_currentwndid == MB3D_WND_TIMELINE) {
			DSCrossButtonSelectPlayerBtns(firstctrlselect);
		}
		else if (s_currentwndid == MB3D_WND_SIDE) {
			if (s_platemenukind == SPPLATEMENUKIND_RIGID) {
				switch (s_platemenuno) {
				case (SPRIGIDTSW_RIGIDPARAMS + 1):
					DSCrossButtonSelectRigidCtrls(firstctrlselect);
					break;
				case (SPRIGIDSW_IMPULSE + 1):
					DSCrossButtonSelectImpulseCtrls(firstctrlselect);
					break;
				case (SPRIGIDSW_GROUNDPLANE + 1):
					DSCrossButtonSelectGPCtrls(firstctrlselect);
					break;
				case (SPRIGIDSW_DAMPANIM + 1):
					DSCrossButtonSelectDampCtrls(firstctrlselect);
					break;
				default:
					break;
				}
			}
			else if (s_platemenukind == SPPLATEMENUKIND_RETARGET) {
				switch (s_platemenuno) {
				case (SPRETARGETSW_RETARGET + 1):
					DSCrossButtonSelectRetargetCtrls(firstctrlselect);
					break;
				case (SPRETARGETSW_LIMITEULER + 1):
					DSCrossButtonSelectEulLimitCtrls(firstctrlselect);
					break;
				default:
					break;
				}
			}
		}
	}
}



void DSCrossButtonSelectTree(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}


	if (!s_owpTimeline) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid >= 0) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);


			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;
				changeflag = true;
			}


			if (s_currentwndid == MB3D_WND_TREE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					if (curbone) {
						if (parentbutton >= 1) {
							if ((accelaxis1 >= 1) || (accelaxis2 >= 1)) {
								//アクセル　L2またはL3がオンのとき　
								CBone* upperbranchbone = curbone->GetUpperBranchBone();
								if (upperbranchbone) {
									int upperbranchboneno = upperbranchbone->GetBoneNo();
									if (upperbranchboneno >= 0) {
										s_curboneno = upperbranchboneno;
										changeflag = true;
									}
								}
							}
							else {
								CBone* parbone = curbone->GetParent();
								if (parbone) {
									int parboneno = parbone->GetBoneNo();
									if (parboneno >= 0) {
										s_curboneno = parboneno;
										changeflag = true;
									}
								}
							}
						}
						else if (sisterbutton >= 1) {
							CBone* sisterbone = curbone->GetSister();
							if (sisterbone) {
								int sisterboneno = sisterbone->GetBoneNo();
								if (sisterboneno >= 0) {
									s_curboneno = sisterboneno;
									changeflag = true;
								}
							}
							else {
								//sister loop
								//brotherがある場合　末端のbrotherに移動
								CBone* brotherbone = curbone->GetBrother();
								while (brotherbone && brotherbone->GetBrother()) {
									brotherbone = brotherbone->GetBrother();
								}
								if (brotherbone) {
									int nextboneno = brotherbone->GetBoneNo();
									if (nextboneno >= 0) {
										s_curboneno = nextboneno;
										changeflag = true;
									}
								}

								//階層的としてのsister, brotherが無い場合には名前としてのsister, brotherをチェックする
								if (changeflag != true) {
									string strcurbonename = curbone->GetBoneName();
									string strLeft = "Left";
									string strRight = "Right";

									string chkLeft = strcurbonename;
									string chkRight = strcurbonename;


									std::string::size_type leftpos = chkLeft.find(strLeft);
									if (leftpos != std::string::npos) {
										//Leftの部分をRightに変えてボーンが存在すればそのボーンに移動
										chkLeft.replace(leftpos, strLeft.length(), strRight);
										CBone* rightbone = s_model->GetBoneByName(chkLeft.c_str());
										if (rightbone) {
											int nextboneno = rightbone->GetBoneNo();
											if (nextboneno >= 0) {
												s_curboneno = nextboneno;
												changeflag = true;
											}
										}
									}
									else {
										std::string::size_type rightpos = chkRight.find(strRight);
										if (rightpos != std::string::npos) {
											//Rightの部分をLeftに変えてボーンが存在すればそのボーンに移動
											chkRight.replace(rightpos, strRight.length(), strLeft);
											CBone* leftbone = s_model->GetBoneByName(chkRight.c_str());
											if (leftbone) {
												int nextboneno = leftbone->GetBoneNo();
												if (nextboneno >= 0) {
													s_curboneno = nextboneno;
													changeflag = true;
												}
											}
										}
									}

								}
							}
						}
						else if (childbutton >= 1) {
							if ((accelaxis1 >= 1) || (accelaxis2 >= 1)) {
								//アクセル　L2またはL3がオンのとき　
								CBone* lowerbranchbone = curbone->GetLowerBranchBone();
								if (lowerbranchbone) {
									int lowerbranchboneno = lowerbranchbone->GetBoneNo();
									if (lowerbranchboneno >= 0) {
										s_curboneno = lowerbranchboneno;
										changeflag = true;
									}
								}
							}
							else {
								CBone* childbone = curbone->GetChild();
								if (childbone) {
									int childboneno = childbone->GetBoneNo();
									if (childboneno >= 0) {
										s_curboneno = childboneno;
										changeflag = true;
									}
								}
							}
						}
						else if (brotherbutton >= 1) {
							CBone* brotherbone = curbone->GetBrother();
							if (brotherbone) {
								int brotherboneno = brotherbone->GetBoneNo();
								if (brotherboneno >= 0) {
									s_curboneno = brotherboneno;
									changeflag = true;
								}
							}
							else {
								//brother loop
								//sisterとparentのchildがある場合　parentのchildに移動
								CBone* sisterbone = curbone->GetSister();
								if (sisterbone) {
									CBone* parentbone = curbone->GetParent();
									if (parentbone) {
										CBone* childbone = parentbone->GetChild();
										if (childbone) {
											int nextboneno = childbone->GetBoneNo();
											if (nextboneno >= 0) {
												s_curboneno = nextboneno;
												changeflag = true;
											}
										}
									}
								}
							}

							//階層的としてのsister, brotherが無い場合には名前としてのsister, brotherをチェックする
							if (changeflag != true) {
								string strcurbonename = curbone->GetBoneName();
								string strLeft = "Left";
								string strRight = "Right";

								string chkLeft = strcurbonename;
								string chkRight = strcurbonename;


								std::string::size_type leftpos = chkLeft.find(strLeft);
								if (leftpos != std::string::npos) {
									//Leftの部分をRightに変えてボーンが存在すればそのボーンに移動
									chkLeft.replace(leftpos, strLeft.length(), strRight);
									CBone* rightbone = s_model->GetBoneByName(chkLeft.c_str());
									if (rightbone) {
										int nextboneno = rightbone->GetBoneNo();
										if (nextboneno >= 0) {
											s_curboneno = nextboneno;
											changeflag = true;
										}
									}
								}
								else {
									std::string::size_type rightpos = chkRight.find(strRight);
									if (rightpos != std::string::npos) {
										//Rightの部分をLeftに変えてボーンが存在すればそのボーンに移動
										chkRight.replace(rightpos, strRight.length(), strLeft);
										CBone* leftbone = s_model->GetBoneByName(chkRight.c_str());
										if (leftbone) {
											int nextboneno = leftbone->GetBoneNo();
											if (nextboneno >= 0) {
												s_curboneno = nextboneno;
												changeflag = true;
											}
										}
									}
								}
							}
						}

						if (changeflag && (s_curboneno >= 0)) {
							if (s_owpTimeline) {
								s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);


								//マウスがタイムライン上にあった場合にだけマウスの位置を選択位置へ移動
								//他のダイアログなどを設定中にカーソルがタイムラインに飛ばないように。

								POINT cursorpos;
								GetCursorPos(&cursorpos);
								ScreenToClient(s_timelineWnd->getHWnd(), &cursorpos);
								RECT timelinerect;
								GetClientRect(s_timelineWnd->getHWnd(), &timelinerect);
								if ((cursorpos.x >= timelinerect.left) && (cursorpos.x <= timelinerect.right) &&
									(cursorpos.y >= timelinerect.top) && (cursorpos.y <= timelinerect.bottom)) {

									WindowPos currentpos = s_owpTimeline->getCurrentLinePos();
									POINT mousepos;
									mousepos.x = currentpos.x;
									mousepos.y = currentpos.y;

									::ClientToScreen(s_timelineWnd->getHWnd(), &mousepos);
									::SetCursorPos(mousepos.x, mousepos.y);
								}



							}
							ChangeCurrentBone();
						}

					}
				}
			}
		}
	}


}




void DSCrossButtonSelectUTGUI(bool firstctrlselect)
{


	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_3dwnd) {
		return;
	}
	if (!IsWindow(s_3dwnd)) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == 1) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;




			if (s_currentwndid == MB3D_WND_3D) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsutguikind = s_curdsutguikind;
					int curdsutguino = s_curdsutguino;


					if (firstctrlselect) {
						parentbutton = 0;
						sisterbutton = 0;
						childbutton = 0;
						brotherbutton = 0;

						accelaxis1 = 0;
						accelaxis2 = 0;

						changeflag = true;
						chkflag = true;
					}



					//############################################################################################################################
					//STL.size()の返り値はunsigned。-1との比較の際には0xFFFFFFFFとsize()との比較になり、意図しない結果を招きやすい。signedに代入してから-1と比較する。
					//############################################################################################################################
					int guisize0 = (int)s_dsutgui0.size();
					int guisize1 = (int)s_dsutgui1.size();
					int guisize2 = (int)s_dsutgui2.size();
					int guisize3 = (int)s_dsutgui3.size();

					if (curbone) {


						if (parentbutton >= 1) {
							curdsutguino--;
							int guigroupid = curdsutguikind;//guikindからSPGUISW?_*への変換。guikind(1)がSPGUISW_LEFT(1)で、guikind(3)がSPGUISW_PHYSICSIK(3)
							switch (guigroupid) {
							case SPGUISW_SPRITEFK:
							{
								if (s_spguisw[SPGUISW_SPRITEFK].state) {
									if (curdsutguino >= SPR_CAM_MAX) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = SPR_CAM_MAX - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;
							case SPGUISW_LEFT:
							{
								if (s_spguisw[SPGUISW_LEFT].state) {
									if (curdsutguino >= guisize0) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize0 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_LEFT2ND:
							{
								if (s_spguisw[SPGUISW_LEFT2ND].state) {
									if (curdsutguino >= guisize1) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize1 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_BULLETPHYSICS:
							{
								if (s_spguisw[SPGUISW_BULLETPHYSICS].state) {
									if (curdsutguino >= guisize2) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize2 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_PHYSICSIK:
							{
								if (s_spguisw[SPGUISW_PHYSICSIK].state) {
									if (curdsutguino >= guisize3) {
										curdsutguino = 0;//size >= 1は関数先頭で確認 ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize3 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							default:
								_ASSERT(0);
								changeflag = false;
								chkflag = true;
								break;
							}
						}
						else if (sisterbutton >= 1) {
							curdsutguikind++;
							curdsutguino = 0;
							int dbgcnt = 0;
							while (chkflag == false) {
								int curgroupid = curdsutguikind;
								if ((curgroupid >= SPGUISW_SPRITEFK) && (curgroupid <= SPGUISW_PHYSICSIK)) {
									if (s_spguisw[curgroupid].state) {
										changeflag = true;
										chkflag = true;
										break;
									}
									else {
										curdsutguikind++;//!!!!!!!!!!!
									}
								}
								else {
									curdsutguikind = 0;//ring
									//changeflag = false;
									//chkflag = true;
									//break;
								}
								dbgcnt++;
								if (dbgcnt >= (SPGUISW_PHYSICSIK - SPGUISW_SPRITEFK + 1)) {//1周分チェックしたら抜ける
									changeflag = false;
									chkflag = true;
									break;
								}
							}
						}
						else if (childbutton >= 1) {
							//if ((accelaxis1 >= 1) || (accelaxis2 >= 1)) {
							//	//アクセル　L2またはL3がオンのとき　

							//}
							//else {

							//}
							curdsutguino++;
							int guigroupid = curdsutguikind;//guikindからSPGUISW?_*への変換。
							switch (guigroupid) {
							case SPGUISW_SPRITEFK:
							{
								if (s_spguisw[SPGUISW_SPRITEFK].state) {
									if (curdsutguino >= SPR_CAM_MAX) {
										curdsutguino = 0;//size >= 1は関数先頭で確認 ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = SPR_CAM_MAX - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_LEFT:
							{
								if (s_spguisw[SPGUISW_LEFT].state) {
									if (curdsutguino >= guisize0) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize0 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_LEFT2ND:
							{
								if (s_spguisw[SPGUISW_LEFT2ND].state) {
									if (curdsutguino >= guisize1) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize1 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_BULLETPHYSICS:
							{
								if (s_spguisw[SPGUISW_BULLETPHYSICS].state) {
									if (curdsutguino >= guisize2) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize2 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							case SPGUISW_PHYSICSIK:
							{
								if (s_spguisw[SPGUISW_PHYSICSIK].state) {
									if (curdsutguino >= guisize3) {
										curdsutguino = 0;//size >= 1は関数先頭で確認　ring
									}
									else if (curdsutguino < 0) {
										curdsutguino = guisize3 - 1;//ring
									}
									changeflag = true;
									chkflag = true;
								}
								else {
									changeflag = false;
									chkflag = true;
								}
							}
							break;

							default:
								_ASSERT(0);
								changeflag = false;
								chkflag = true;
								break;
							}

						}
						else if (brotherbutton >= 1) {
							curdsutguikind--;
							curdsutguino = 0;
							int dbgcnt = 0;
							while (chkflag == false) {
								int curgroupid = curdsutguikind;
								if ((curgroupid >= SPGUISW_SPRITEFK) && (curgroupid <= SPGUISW_PHYSICSIK)) {
									if (s_spguisw[curgroupid].state) {
										changeflag = true;
										chkflag = true;
										break;
									}
									else {
										curdsutguikind--;//!!!!!!!!!!!
									}
								}
								else {
									curdsutguikind = SPGUISW_PHYSICSIK;//ring
									//changeflag = false;
									//chkflag = true;
									//break;
								}
								dbgcnt++;
								if (dbgcnt >= (SPGUISW_PHYSICSIK - SPGUISW_SPRITEFK + 1)) {//1周分チェックしたら抜ける
									changeflag = false;
									chkflag = true;
									break;
								}
							}
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/

/* 
//#####################################################################################################
//プレート上で〇ボタンを押したときにマウスがコントロールへ飛ぶ。プレート上にマウスが残った方が便利だったのでコメントアウト
//#####################################################################################################
*
* 
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if ((curdsutguikind >= SPGUISW_SPRITEFK) && (curdsutguikind <= SPGUISW_PHYSICSIK)) {

								s_curdsutguikind = curdsutguikind;
								s_curdsutguino = curdsutguino;
								int guigroupid = s_curdsutguikind;
								switch (guigroupid) {
								case SPGUISW_SPRITEFK:
								{
									if ((s_curdsutguino >= 0) && (s_curdsutguino < SPR_CAM_MAX)) {
										POINT ctrlpos;
										ctrlpos.x = s_spcam[s_curdsutguino].dispcenter.x;
										ctrlpos.y = s_spcam[s_curdsutguino].dispcenter.y;

										ClientToScreen(s_3dwnd, &ctrlpos);
										::SetCursorPos(ctrlpos.x, ctrlpos.y);
									}
								}
								break;

								case SPGUISW_LEFT:
									if ((s_curdsutguino >= 0) && (s_curdsutguino < guisize0) && s_dsutgui0[s_curdsutguino]) {
										//s_dsutgui0[s_curdsutguino]->SetTextColor(0xFF0000FF);
										//s_dsutgui0[s_curdsutguino]->OnFocusIn();
										POINT ctrlpos;
										s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
										//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
										UINT type = s_dsutgui0[s_curdsutguino]->GetType();
										switch (type) {
										case DXUT_CONTROL_CHECKBOX:
										case DXUT_CONTROL_RADIOBUTTON:
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											break;
										case DXUT_CONTROL_BUTTON:
										case DXUT_CONTROL_COMBOBOX:
											ctrlpos.x += (100 / 2);
											ctrlpos.y += (25 / 2);
											break;
										case DXUT_CONTROL_STATIC:
											break;

										case DXUT_CONTROL_SLIDER:
										{
											int slmin = 0;
											int slmax = 0;
											int slvalue = 0;
											((CDXUTSlider*)s_dsutgui0[s_curdsutguino])->GetRange(slmin, slmax);
											slvalue = ((CDXUTSlider*)s_dsutgui0[s_curdsutguino])->GetValue();
											float rate = 0.0f;
											int length = slmax - slmin;
											if (length != 0) {
												rate = (float)(slvalue - slmin) / (float)length;
											}
											else {
												rate = 0.0f;
											}

											ctrlpos.x += (int)(100.0f * rate);//100.0f : ctrl width
											ctrlpos.y += (25 / 2);
										}
										break;
										case DXUT_CONTROL_EDITBOX:
										case DXUT_CONTROL_IMEEDITBOX:
										case DXUT_CONTROL_LISTBOX:
										case DXUT_CONTROL_SCROLLBAR:
											break;
										}
										ClientToScreen(s_3dwnd, &ctrlpos);
										::SetCursorPos(ctrlpos.x, ctrlpos.y);
									}
									break;

								case SPGUISW_LEFT2ND:
									if ((s_curdsutguino >= 0) && (s_curdsutguino < guisize1) && s_dsutgui1[s_curdsutguino]) {
										//s_dsutgui1[s_curdsutguino]->SetTextColor(0xFF0000FF);
										//s_dsutgui1[s_curdsutguino]->OnFocusIn();
										POINT ctrlpos;
										s_dsutgui1[s_curdsutguino]->GetLocation(&ctrlpos);
										UINT type = s_dsutgui1[s_curdsutguino]->GetType();
										switch (type) {
										case DXUT_CONTROL_CHECKBOX:
										case DXUT_CONTROL_RADIOBUTTON:
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											break;
										case DXUT_CONTROL_BUTTON:
										case DXUT_CONTROL_COMBOBOX:
											ctrlpos.x += (100 / 2);
											ctrlpos.y += (25 / 2);
											break;
										case DXUT_CONTROL_STATIC:
											break;

										case DXUT_CONTROL_SLIDER:
										{
											int slmin = 0;
											int slmax = 0;
											int slvalue = 0;
											((CDXUTSlider*)s_dsutgui1[s_curdsutguino])->GetRange(slmin, slmax);
											slvalue = ((CDXUTSlider*)s_dsutgui1[s_curdsutguino])->GetValue();
											float rate = 0.0f;
											int length = slmax - slmin;
											if (length != 0) {
												rate = (float)(slvalue - slmin) / (float)length;
											}
											else {
												rate = 0.0f;
											}

											ctrlpos.x += (int)(100.0f * rate);
											ctrlpos.y += (25 / 2);
										}
										break;
										case DXUT_CONTROL_EDITBOX:
										case DXUT_CONTROL_IMEEDITBOX:
										case DXUT_CONTROL_LISTBOX:
										case DXUT_CONTROL_SCROLLBAR:
											break;
										}
										ClientToScreen(s_3dwnd, &ctrlpos);
										::SetCursorPos(ctrlpos.x, ctrlpos.y);
									}
									break;

								case SPGUISW_BULLETPHYSICS:
									if ((s_curdsutguino >= 0) && (s_curdsutguino < guisize2) && s_dsutgui2[s_curdsutguino]) {
										//s_dsutgui2[s_curdsutguino]->SetTextColor(0xFF0000FF);
										//s_dsutgui2[s_curdsutguino]->OnFocusIn();
										POINT ctrlpos;
										s_dsutgui2[s_curdsutguino]->GetLocation(&ctrlpos);
										UINT type = s_dsutgui2[s_curdsutguino]->GetType();
										switch (type) {
										case DXUT_CONTROL_CHECKBOX:
										case DXUT_CONTROL_RADIOBUTTON:
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											break;
										case DXUT_CONTROL_BUTTON:
										case DXUT_CONTROL_COMBOBOX:
											ctrlpos.x += (100 / 2);
											ctrlpos.y += (25 / 2);
											break;
										case DXUT_CONTROL_STATIC:
											break;

										case DXUT_CONTROL_SLIDER:
										{
											int slmin = 0;
											int slmax = 0;
											int slvalue = 0;
											((CDXUTSlider*)s_dsutgui2[s_curdsutguino])->GetRange(slmin, slmax);
											slvalue = ((CDXUTSlider*)s_dsutgui2[s_curdsutguino])->GetValue();
											float rate = 0.0f;
											int length = slmax - slmin;
											if (length != 0) {
												rate = (float)(slvalue - slmin) / (float)length;
											}
											else {
												rate = 0.0f;
											}

											ctrlpos.x += (int)(100.0f * rate);
											ctrlpos.y += (25 / 2);
										}
										break;
										case DXUT_CONTROL_EDITBOX:
										case DXUT_CONTROL_IMEEDITBOX:
										case DXUT_CONTROL_LISTBOX:
										case DXUT_CONTROL_SCROLLBAR:
											break;
										}
										ClientToScreen(s_3dwnd, &ctrlpos);
										::SetCursorPos(ctrlpos.x, ctrlpos.y);

									}
									break;

								case SPGUISW_PHYSICSIK:
									if ((s_curdsutguino >= 0) && (s_curdsutguino < guisize3) && s_dsutgui3[s_curdsutguino]) {
										//s_dsutgui3[s_curdsutguino]->SetTextColor(0xFF0000FF);
										//s_dsutgui3[s_curdsutguino]->OnFocusIn();
										POINT ctrlpos;
										s_dsutgui3[s_curdsutguino]->GetLocation(&ctrlpos);
										UINT type = s_dsutgui3[s_curdsutguino]->GetType();
										switch (type) {
										case DXUT_CONTROL_CHECKBOX:
										case DXUT_CONTROL_RADIOBUTTON:
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											break;
										case DXUT_CONTROL_BUTTON:
										case DXUT_CONTROL_COMBOBOX:
											ctrlpos.x += (100 / 2);
											ctrlpos.y += (25 / 2);
											break;
										case DXUT_CONTROL_STATIC:
											break;

										case DXUT_CONTROL_SLIDER:
										{
											int slmin = 0;
											int slmax = 0;
											int slvalue = 0;
											((CDXUTSlider*)s_dsutgui3[s_curdsutguino])->GetRange(slmin, slmax);
											slvalue = ((CDXUTSlider*)s_dsutgui3[s_curdsutguino])->GetValue();
											float rate = 0.0f;
											int length = slmax - slmin;
											if (length != 0) {
												rate = (float)(slvalue - slmin) / (float)length;
											}
											else {
												rate = 0.0f;
											}

											ctrlpos.x += (int)(100.0f * rate);
											ctrlpos.y += (25 / 2);
										}
										break;
										case DXUT_CONTROL_EDITBOX:
										case DXUT_CONTROL_IMEEDITBOX:
										case DXUT_CONTROL_LISTBOX:
										case DXUT_CONTROL_SCROLLBAR:
											break;
										}
										ClientToScreen(s_3dwnd, &ctrlpos);
										::SetCursorPos(ctrlpos.x, ctrlpos.y);

									}
									break;


								default:
									break;
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}
*/
					}
				}
			}
		}
	}

}


void DSCrossButtonSelectEulLimitCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_anglelimitdlg || !IsWindow(s_anglelimitdlg)) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;


			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdseullimitctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dseullimitctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dseullimitctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dseullimitctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dseullimitctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dseullimitctrls.size())) {

								s_curdseullimitctrlno = curdsctrlno;


								if (s_anglelimitdlg && IsWindow(s_anglelimitdlg) &&
									(s_curdseullimitctrlno >= 0) && (s_curdseullimitctrlno < s_dseullimitctrls.size()) && (s_dseullimitctrls[s_curdseullimitctrlno])) {

								
									POINT ctrlpos;

									HWND ctrlwnd = ::GetDlgItem(s_anglelimitdlg, s_dseullimitctrls[s_curdseullimitctrlno]);
									if (ctrlwnd) {
										RECT ctrlrect;
										::GetWindowRect(ctrlwnd, &ctrlrect);
										if (s_curdseullimitctrlno == 0) {
											ctrlpos.x = ctrlrect.left + 175;
											ctrlpos.y = ctrlrect.top + 20 / 2;
										}
										else if(s_curdseullimitctrlno == (s_dseullimitctrls.size() - 1)){
											ctrlpos.x = ctrlrect.left + 100 / 2;
											ctrlpos.y = ctrlrect.top + 20 / 2;
										}
										else {
											int slidervalue;
											slidervalue = (int)::SendMessage(ctrlwnd, TBM_GETPOS, 0, 0);
											int sliderposx = (int)((float)(slidervalue + 180) / 360.0f * 274.0f);
											ctrlpos.x = ctrlrect.left + sliderposx + 12;
											ctrlpos.y = ctrlrect.top + 20 / 2;
										}

										//ClientToScreen(s_anglelimitdlg, &ctrlpos);

										::SetCursorPos(ctrlpos.x, ctrlpos.y);
									}
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}

void DSCrossButtonSelectRetargetCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_convboneWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}

			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsretargetctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsretargetctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsretargetctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsretargetctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsretargetctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dsretargetctrls.size())) {
								s_curdsretargetctrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dsretargetctrls[s_curdsretargetctrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_convboneWnd && IsWindow(s_convboneWnd->getHWnd()) &&
									(s_curdsretargetctrlno >= 0) && (s_curdsretargetctrlno < s_dsretargetctrls.size()) && (s_dsretargetctrls[s_curdsretargetctrlno])) {

									bool isslider;
									isslider = ((OrgWinGUI::OrgWindowParts*)s_dsretargetctrls[s_curdsretargetctrlno])->getIsSlider();
									if (isslider) {
										//sizex : 350, sizey : 20, window with 450

										double minval;
										double maxval;
										double curval;
										double length;
										minval = ((OrgWinGUI::OWP_Slider*)s_dsretargetctrls[s_curdsretargetctrlno])->getMinValue();
										maxval = ((OrgWinGUI::OWP_Slider*)s_dsretargetctrls[s_curdsretargetctrlno])->getMaxValue();
										curval = ((OrgWinGUI::OWP_Slider*)s_dsretargetctrls[s_curdsretargetctrlno])->getValue();
										length = maxval - minval;
										if (length != 0.0) {
											double rate;
											rate = (curval - minval) / length;
											//ctrlpos.x += (LONG)(350.0 * rate);
											//pos1x+ (int)((value - minValue) * (float)(pos2x - pos1x) / (maxValue - minValue) + 0.5f);
											int pos2xpos1x = 450 - 6 - 65 - 5;
											ctrlpos.x += (5 + (int)((curval - minval) * (float)(pos2xpos1x) / (maxval - minval) + 0.5f));//LABEL_SIZE_X : 65.0, AXIS_POS_X : 5.0
											ctrlpos.y += (int)(20.0 / 2.0);
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
										else {
											//if button
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
									}
									else {
										//if button
										ctrlpos.x += 6;
										ctrlpos.y += 6;
										//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
									}

									WindowPos retargetpos;
									retargetpos = s_convboneWnd->getPos();
									ctrlpos.x += retargetpos.x;
									ctrlpos.y += retargetpos.y;
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}

void DSCrossButtonSelectDampCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_dmpanimWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsdampctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsdampctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsdampctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsdampctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsdampctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dsdampctrls.size())) {
								s_curdsdampctrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dsdampctrls[s_curdsdampctrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_dmpanimWnd && IsWindow(s_dmpanimWnd->getHWnd()) &&
									(s_curdsdampctrlno >= 0) && (s_curdsdampctrlno < s_dsdampctrls.size()) && (s_dsdampctrls[s_curdsdampctrlno])) {

									bool isslider;
									isslider = ((OrgWinGUI::OrgWindowParts*)s_dsdampctrls[s_curdsdampctrlno])->getIsSlider();
									if (isslider) {
										//sizex : 350, sizey : 20, window with 450

										double minval;
										double maxval;
										double curval;
										double length;
										minval = ((OrgWinGUI::OWP_Slider*)s_dsdampctrls[s_curdsdampctrlno])->getMinValue();
										maxval = ((OrgWinGUI::OWP_Slider*)s_dsdampctrls[s_curdsdampctrlno])->getMaxValue();
										curval = ((OrgWinGUI::OWP_Slider*)s_dsdampctrls[s_curdsdampctrlno])->getValue();
										length = maxval - minval;
										if (length != 0.0) {
											double rate;
											rate = (curval - minval) / length;
											//ctrlpos.x += (LONG)(350.0 * rate);
											//pos1x+ (int)((value - minValue) * (float)(pos2x - pos1x) / (maxValue - minValue) + 0.5f);
											int pos2xpos1x = 450 - 6 - 65 - 5;
											ctrlpos.x += (5 + (int)((curval - minval) * (float)(pos2xpos1x) / (maxval - minval) + 0.5f));//LABEL_SIZE_X : 65.0, AXIS_POS_X : 5.0
											ctrlpos.y += (int)(20.0 / 2.0);
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
										else {
											//if button
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
									}
									else {
										//if button
										ctrlpos.x += 6;
										ctrlpos.y += 6;
										//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
									}

									WindowPos damppos;
									damppos = s_dmpanimWnd->getPos();
									ctrlpos.x += damppos.x;
									ctrlpos.y += damppos.y;
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}

void DSCrossButtonSelectGPCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_gpWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsgpctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsgpctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsgpctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsgpctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsgpctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dsgpctrls.size())) {
								s_curdsgpctrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dsgpctrls[s_curdsgpctrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_gpWnd && IsWindow(s_gpWnd->getHWnd()) &&
									(s_curdsgpctrlno >= 0) && (s_curdsgpctrlno < s_dsgpctrls.size()) && (s_dsgpctrls[s_curdsgpctrlno])) {

									bool isslider;
									isslider = ((OrgWinGUI::OrgWindowParts*)s_dsgpctrls[s_curdsgpctrlno])->getIsSlider();
									if (isslider) {
										//sizex : 350, sizey : 20, window with 450

										double minval;
										double maxval;
										double curval;
										double length;
										minval = ((OrgWinGUI::OWP_Slider*)s_dsgpctrls[s_curdsgpctrlno])->getMinValue();
										maxval = ((OrgWinGUI::OWP_Slider*)s_dsgpctrls[s_curdsgpctrlno])->getMaxValue();
										curval = ((OrgWinGUI::OWP_Slider*)s_dsgpctrls[s_curdsgpctrlno])->getValue();
										length = maxval - minval;
										if (length != 0.0) {
											double rate;
											rate = (curval - minval) / length;
											//ctrlpos.x += (LONG)(350.0 * rate);
											//pos1x+ (int)((value - minValue) * (float)(pos2x - pos1x) / (maxValue - minValue) + 0.5f);
											int pos2xpos1x = 450 - 6 - 65 - 5;
											ctrlpos.x += (5 + (int)((curval - minval) * (float)(pos2xpos1x) / (maxval - minval) + 0.5f));//LABEL_SIZE_X : 65.0, AXIS_POS_X : 5.0
											ctrlpos.y += (int)(20.0 / 2.0);
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
										else {
											//if button
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
									}
									else {
										//if button
										ctrlpos.x += 6;
										ctrlpos.y += 6;
										//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
									}

									WindowPos gppos;
									gppos = s_gpWnd->getPos();
									ctrlpos.x += gppos.x;
									ctrlpos.y += gppos.y;
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}
void DSCrossButtonSelectImpulseCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_impWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsimpulsectrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsimpulsectrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsimpulsectrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsimpulsectrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsimpulsectrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dsimpulsectrls.size())) {
								s_curdsimpulsectrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dsimpulsectrls[s_curdsimpulsectrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_impWnd && IsWindow(s_impWnd->getHWnd()) &&
									(s_curdsimpulsectrlno >= 0) && (s_curdsimpulsectrlno < s_dsimpulsectrls.size()) && (s_dsimpulsectrls[s_curdsimpulsectrlno])) {

									bool isslider;
									isslider = ((OrgWinGUI::OrgWindowParts*)s_dsimpulsectrls[s_curdsimpulsectrlno])->getIsSlider();
									if (isslider) {
										//sizex : 350, sizey : 20, window with 450

										double minval;
										double maxval;
										double curval;
										double length;
										minval = ((OrgWinGUI::OWP_Slider*)s_dsimpulsectrls[s_curdsimpulsectrlno])->getMinValue();
										maxval = ((OrgWinGUI::OWP_Slider*)s_dsimpulsectrls[s_curdsimpulsectrlno])->getMaxValue();
										curval = ((OrgWinGUI::OWP_Slider*)s_dsimpulsectrls[s_curdsimpulsectrlno])->getValue();
										length = maxval - minval;
										if (length != 0.0) {
											double rate;
											rate = (curval - minval) / length;
											//ctrlpos.x += (LONG)(350.0 * rate);
											//pos1x+ (int)((value - minValue) * (float)(pos2x - pos1x) / (maxValue - minValue) + 0.5f);
											int pos2xpos1x = 450 - 6 - 65 - 5;
											ctrlpos.x += (5 + (int)((curval - minval) * (float)(pos2xpos1x) / (maxval - minval) + 0.5f));//LABEL_SIZE_X : 65.0, AXIS_POS_X : 5.0
											ctrlpos.y += (int)(20.0 / 2.0);
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
										else {
											//if button
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
									}
									else {
										//if button
										ctrlpos.x += 6;
										ctrlpos.y += 6;
										//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
									}

									WindowPos impulsepos;
									impulsepos = s_impWnd->getPos();
									ctrlpos.x += impulsepos.x;
									ctrlpos.y += impulsepos.y;
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}

void DSCrossButtonSelectRigidCtrls(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_rigidWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_SIDE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}



			if (s_currentwndid == MB3D_WND_SIDE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsrigidctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsrigidctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsrigidctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dsrigidctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dsrigidctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dsrigidctrls.size())) {
								s_curdsrigidctrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dsrigidctrls[s_curdsrigidctrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_rigidWnd && IsWindow(s_rigidWnd->getHWnd()) && 
									(s_curdsrigidctrlno >= 0) && (s_curdsrigidctrlno < s_dsrigidctrls.size()) && (s_dsrigidctrls[s_curdsrigidctrlno])) {

									bool isslider;
									isslider = ((OrgWinGUI::OrgWindowParts*)s_dsrigidctrls[s_curdsrigidctrlno])->getIsSlider();
									if (isslider) {
										//sizex : 350, sizey : 20, window with 450

										double minval;
										double maxval;
										double curval;
										double length;
										minval = ((OrgWinGUI::OWP_Slider*)s_dsrigidctrls[s_curdsrigidctrlno])->getMinValue();
										maxval = ((OrgWinGUI::OWP_Slider*)s_dsrigidctrls[s_curdsrigidctrlno])->getMaxValue();
										curval = ((OrgWinGUI::OWP_Slider*)s_dsrigidctrls[s_curdsrigidctrlno])->getValue();
										length = maxval - minval;
										if (length != 0.0) {
											double rate;
											rate = (curval - minval) / length;
											//ctrlpos.x += (LONG)(350.0 * rate);
											//pos1x+ (int)((value - minValue) * (float)(pos2x - pos1x) / (maxValue - minValue) + 0.5f);
											int pos2xpos1x = 450 - 6 - 65 - 5;
											ctrlpos.x += (5 + (int)((curval - minval) * (float)(pos2xpos1x) / (maxval - minval) + 0.5f));//LABEL_SIZE_X : 65.0, AXIS_POS_X : 5.0
											ctrlpos.y += (int)(20.0 / 2.0);
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
										else {
											//if button
											ctrlpos.x += 6;
											ctrlpos.y += 6;
											//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
										}
									}
									else {
										//if button
										ctrlpos.x += 6;
										ctrlpos.y += 6;
										//ClientToScreen(s_rigidWnd->getHWnd(), &ctrlpos);
									}

									WindowPos rigidpos;
									rigidpos = s_rigidWnd->getPos();
									ctrlpos.x += rigidpos.x;
									ctrlpos.y += rigidpos.y;
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}

void DSCrossButtonSelectToolCtrls(bool firstctrlselect)
{


	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_toolWnd) {
		return;
	}

	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_TOOL) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_TOOL) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdstoolctrlno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dstoolctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dstoolctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= (int)s_dstoolctrls.size()) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = (int)s_dstoolctrls.size() - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < s_dstoolctrls.size())) {
								s_curdstoolctrlno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_dstoolctrls[s_curdstoolctrlno]->getPos();
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;


								//if button
								ctrlpos.x += 6;
								ctrlpos.y += 6;


								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_toolWnd && IsWindow(s_toolWnd->getHWnd())) {
									ClientToScreen(s_toolWnd->getHWnd(), &ctrlpos);
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}


void DSCrossButtonSelectPlayerBtns(bool firstctrlselect)
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_LtimelineWnd) {
		return;
	}

	if (!s_owpPlayerButton) {
		return;
	}


	//select control
	//十字キー移動ブロック
	{
		if ((s_currentwndid == MB3D_WND_TIMELINE) && (s_currenthwnd != 0)) {


			int parentbuttonid = 4;
			int sisterbuttonid = 5;
			int childbuttonid = 6;
			int brotherbuttonid = 7;
			int accelaxisid1 = 4;//axisid
			int accelaxisid2 = 5;//axisid

			int parentbutton = 0;
			int sisterbutton = 0;
			int childbutton = 0;
			int brotherbutton = 0;
			int accelaxis1 = 0;
			int accelaxis2 = 0;

			parentbutton = s_dsbuttonup[parentbuttonid];
			sisterbutton = s_dsbuttonup[sisterbuttonid];
			childbutton = s_dsbuttonup[childbuttonid];
			brotherbutton = s_dsbuttonup[brotherbuttonid];

			accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
			accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

			bool changeflag = false;
			bool chkflag = false;

			//WS_TABSTOP
			//VK_TAB
			//WS_GROUP
			//または
			//HWND SetFocus(HWND hWnd);

			//static int s_curdsutguikind = 0;
			//static int s_curdsutguino = 0;

			if (firstctrlselect) {
				parentbutton = 0;
				sisterbutton = 0;
				childbutton = 0;
				brotherbutton = 0;

				accelaxis1 = 0;
				accelaxis2 = 0;

				changeflag = true;
				chkflag = true;
			}


			if (s_currentwndid == MB3D_WND_TIMELINE) {
				if (s_model && (s_curboneno >= 0)) {
					CBone* curbone = s_model->GetBoneByID(s_curboneno);
					int curdsctrlno = s_curdsplayerbtnno;
					if (curbone) {

						if (parentbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = SPPLAYERBUTTONNUM - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (sisterbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= SPPLAYERBUTTONNUM) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (childbutton >= 1) {
							curdsctrlno++;
							if (curdsctrlno >= SPPLAYERBUTTONNUM) {
								curdsctrlno = 0;//Ring
							}
							changeflag = true;
							chkflag = true;
						}
						else if (brotherbutton >= 1) {
							curdsctrlno--;
							if (curdsctrlno < 0) {
								curdsctrlno = SPPLAYERBUTTONNUM - 1;//Ring
							}
							changeflag = true;
							chkflag = true;
						}


						/*
						enum DXUT_CONTROL_TYPE
						{
							DXUT_CONTROL_BUTTON,
							DXUT_CONTROL_STATIC,
							DXUT_CONTROL_CHECKBOX,
							DXUT_CONTROL_RADIOBUTTON,
							DXUT_CONTROL_COMBOBOX,
							DXUT_CONTROL_SLIDER,
							DXUT_CONTROL_EDITBOX,
							DXUT_CONTROL_IMEEDITBOX,
							DXUT_CONTROL_LISTBOX,
							DXUT_CONTROL_SCROLLBAR,
						};
						*/
						if (chkflag && changeflag && (s_curboneno >= 0)) {
							if (((curdsctrlno) >= 0) && ((curdsctrlno) < SPPLAYERBUTTONNUM)) {
								s_curdsplayerbtnno = curdsctrlno;

								OrgWinGUI::WindowPos ctrlwinpos;
								POINT ctrlpos;
								ctrlwinpos = s_owpPlayerButton->getButtonPos(s_curdsplayerbtnno);
								ctrlpos.x = ctrlwinpos.x;
								ctrlpos.y = ctrlwinpos.y;

								//if button
								//ctrlpos.x += 10;
								//ctrlpos.y += 10;


								//s_dsutgui0[s_curdsutguino]->GetLocation(&ctrlpos);
								//UINT type = s_dsutgui0[s_curdsutguino]->GetType();
								if (s_owpPlayerButton && s_LtimelineWnd && IsWindow(s_LtimelineWnd->getHWnd())) {
									ClientToScreen(s_LtimelineWnd->getHWnd(), &ctrlpos);
									::SetCursorPos(ctrlpos.x, ctrlpos.y);
								}
							}

							//if (s_owpTimeline) {
							//	s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
							//}
							//ChangeCurrentBone();
						}

					}
				}
			}
		}
	}

}


void DSAxisRMainMenuBar()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	POINT cursorpos;
	::GetCursorPos(&cursorpos);

	int upbutton;
	int downbutton;
	int leftbutton;
	int rightbutton;
	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;

	upbutton = s_dsaxisMOverSrh[1];
	downbutton = s_dsaxisOverSrh[1];
	leftbutton = s_dsaxisMOverSrh[0];
	rightbutton = s_dsaxisOverSrh[0];

	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;
	accelbothflag = accelaxis1 && accelaxis2;


	//if (s_undertrackingRMenu == 0) {
	bool changeflag = false;

	int delta = 1;
	//if (accelflag) {
	//	delta = 4;
	//}
	//else {
	//	delta = 2;
	//}



	int nextsubmenuid;

	nextsubmenuid = g_currentsubmenuid;

	if (upbutton >= 1) {
		changeflag = true;
	}
	if (downbutton >= 1) {
		changeflag = true;
	}
	if (leftbutton >= 1) {
		nextsubmenuid -= delta;
		changeflag = true;
	}
	if (rightbutton >= 1) {
		nextsubmenuid += delta;
		changeflag = true;
	}

	if (nextsubmenuid >= SPMENU_MAX) {
		nextsubmenuid = 0;//ring
	}
	if (nextsubmenuid < 0) {
		nextsubmenuid = (SPMENU_MAX - 1);//ring
	}

	if (changeflag == true) {

		if (g_undertrackingRMenu == 1) {
			//多重トラックポップアップ禁止（多重ポップアップするとプログラムからマウス移動できなくなる）
			return;
		}


		SelectNextWindow(MB3D_WND_MAIN);//MainMenuAimBarWndをハイライト


		g_currentsubmenuid = nextsubmenuid;
		s_currentsubmenuitemid = 0;


		if ((g_currentsubmenuid >= 0) && (g_currentsubmenuid < SPMENU_MAX)) {

			HMENU mainmenu;
			mainmenu = GetMenu(s_mainhwnd);
			int menuno = g_currentsubmenuid;
			s_cursubmenu = GetSubMenu(mainmenu, menuno);
			if (s_cursubmenu) {
				int curmenuitemid;
				curmenuitemid = ::GetMenuItemID(s_cursubmenu, 0);
				if (curmenuitemid >= 0) {
					RECT rc;
					GetMenuItemRect(s_mainhwnd, mainmenu, menuno, &rc);//rcはスクリーン座標
					g_submenuwidth = rc.right - rc.left;//org:140

					::SetCursorPos(rc.left, rc.bottom + 22);
					g_currentsubmenupos.x = rc.left;
					g_currentsubmenupos.y = rc.bottom + 16;

					Sleep(200);
				}
			}
		}



		//HMENU mainmenu;
		////HMENU cursubmenu;
		//mainmenu = GetMenu(s_mainhwnd);
		//s_cursubmenu = GetSubMenu(mainmenu, g_currentsubmenuid);
		//if (s_cursubmenu) {
		//	int curmenuitemid;
		//	curmenuitemid = ::GetMenuItemID(s_cursubmenu, 0);
		//	if (curmenuitemid >= 0) {
		//		
		//		//::SendMessage(s_mainhwnd, WM_NOTIFY, 0, (LPARAM)&nmtoolbara);
		//		////::SendMessage(s_mainhwnd, WM_COMMAND, curmenuitemid, 0);//選択決定時のコマンド

		//		RECT rc;
		//		TPMPARAMS tpm;
		//		GetMenuItemRect(s_mainhwnd, mainmenu, g_currentsubmenuid, &rc);
		//		tpm.cbSize = sizeof(TPMPARAMS);//
		//		tpm.rcExclude = rc;//

		//		POINT mousepoint;
		//		mousepoint.x = rc.left + 30;
		//		mousepoint.y = rc.bottom + 10;
		//		::SetCursorPos(mousepoint.x, mousepoint.y);

		//		//HMENU hMenuLoaded = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));//
		//		//HMENU hPopupMenu = GetSubMenu(s_mainmenu, g_currentsubmenuid);//
		//		InterlockedExchange(&g_undertrackingRMenu, 1);

		//		//#################
		//		//選択決定成功例その２
		//		//#################
		//		//第2項目（インデックス値 = 1, ID = 0x2711）を選択したところ、
		//		//wParam = 0x00012711
		//		//のように、上位2バイトにインデックス値が、下位2バイトにIDが入ります。
		//		//WPARAM wparam;
		//		//wparam = (g_currentsubmenuid << 16) | curmenuitemid;
		//		//LPARAM lparam;
		//		//lparam = (LPARAM)mainmenu;
		//		//::SendMessage(s_mainhwnd, WM_COMMAND, wparam, lparam);



		//		//wparam = ((MF_POPUP | MF_MOUSESELECT) << 16) | (WORD)g_currentsubmenuid;//g_currentsubmenuid, curmenuitemid
		//		//::SendMessage(s_mainhwnd, WM_MENUSELECT, wparam, (LPARAM)mainmenu);//GetMenu(s_mainhwnd), cursubmenu




		//		//TrackPopupMenuEx(cursubmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
		//		//	rc.left, rc.bottom, s_mainhwnd, &tpm);//
		//		//SetCapture(s_3dwnd);

		//		SetForegroundWindow(s_mainhwnd);//この処理をしないと範囲外クリックでPopupが閉じない

		//		int retmenuid = ::TrackPopupMenu(s_cursubmenu, TPM_RETURNCMD | TPM_LEFTALIGN, rc.left, rc.bottom, 0, s_mainhwnd, NULL);

		//		//ReleaseCapture();

		//		InterlockedExchange(&g_undertrackingRMenu, 0);

		//		//int retmenuid = ::TrackPopupMenu(hPopupMenu, TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, 0, m_menuwnd, NULL);
		//		//DestroyMenu(hMenuLoaded);//
		//	}
		//}

		//LPARAM lparam;
		//lparam = (cursorpos.y << 16) | cursorpos.x;

		//HWND desktopwnd;
		//desktopwnd = ::GetDesktopWindow();
		//if (desktopwnd) {
		//	RECT desktoprect;
		//	::GetClientRect(desktopwnd, &desktoprect);
		//	::ClipCursor(&desktoprect);


		//	::SetCursorPos(cursorpos.x, cursorpos.y);
		//	//::SetCursorPos(cursorpos.x, cursorpos.y);

		//	if (s_3dwnd) {
		//		POINT client3dpoint;
		//		client3dpoint = cursorpos;
		//		::ScreenToClient(s_3dwnd, &client3dpoint);
		//		LPARAM threelparam;
		//		threelparam = (client3dpoint.y << 16) | client3dpoint.x;
		//		::SendMessage(s_3dwnd, WM_MOUSEMOVE, 0, threelparam);
		//	}
		//	if (s_mainhwnd) {
		//		POINT clientpoint;
		//		clientpoint = cursorpos;
		//		::ScreenToClient(s_mainhwnd, &clientpoint);
		//		LPARAM mainlparam;
		//		mainlparam = (clientpoint.y << 16) | clientpoint.x;
		//		::SendMessage(s_mainhwnd, WM_MOUSEMOVE, 0, mainlparam);
		//	}
		//	HWND dlghwnd;
		//	dlghwnd = g_SampleUI.GetHWnd();
		//	if (dlghwnd) {
		//		POINT clientpoint;
		//		clientpoint = cursorpos;
		//		::ScreenToClient(dlghwnd, &clientpoint);
		//		LPARAM dlglparam;
		//		dlglparam = (clientpoint.y << 16) | clientpoint.x;
		//		::SendMessage(dlghwnd, WM_MOUSEMOVE, 0, dlglparam);
		//	}
		//
		//	//::SendMessage(desktopwnd, WM_MOUSEMOVE, 0, (LPARAM)lparam);
		//}
	}

}

void DSAxisLSelectingPopupMenu()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_cursubmenu) {
		return;
	}

	int upbutton;
	int downbutton;
	int leftbutton;
	int rightbutton;
	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;

	upbutton = s_dsaxisMOverSrh[3];
	downbutton = s_dsaxisOverSrh[3];
	leftbutton = s_dsaxisMOverSrh[2];
	rightbutton = s_dsaxisOverSrh[2];

	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;

	//if (s_undertrackingRMenu == 0) {
	bool changeflag = false;
	int currentsubmenuitemid;
	currentsubmenuitemid = s_currentsubmenuitemid;

	int delta = 0;

	if (upbutton >= 1) {
		currentsubmenuitemid -= 1;
		changeflag = true;
	}
	if (downbutton >= 1) {
		currentsubmenuitemid += 1;
		changeflag = true;
	}


	int submenuitemnum;
	submenuitemnum = GetMenuItemCount(s_cursubmenu);


	if (currentsubmenuitemid >= submenuitemnum) {
		currentsubmenuitemid = 0;//ring
	}
	if (currentsubmenuitemid < 0) {
		currentsubmenuitemid = submenuitemnum - 1;//ring
	}

/*
typedef struct tagMENUITEMINFO {
	UINT    cbSize;
	UINT    fMask;
	UINT    fType;
	UINT    fState;
	UINT    wID;
	HMENU   hSubMenu;
	HBITMAP hbmpChecked;
	HBITMAP hbmpUnchecked;
	DWORD   dwItemData;
	LPTSTR  dwTypeData;
	UINT    cch;
} MENUITEMINFO, FAR* LPMENUITEMINFO;
cbSize は、この構造体のサイズをバイト単位


fState は、メニュー項目の状態を表す定数を組み合わせて指定します
定数は、次のものを指定できます
定数	解説
MFS_CHECKED	項目をチェックする
MFS_DEFAULT	項目はデフォルトである
MFS_DISABLED	項目を無効状態にする
MFS_ENABLED	項目を有効状態にする（デフォルト）
MFS_GRAYED	項目をグレー状態にする
MFS_HILITE	項目をハイライト状態にする//#################
MFS_UNCHECKED	項目のチェックを外す
MFS_UNHILITE	項目のハイライトを削除する

fByPosition が TRUE ならば uItem はインデックスだと判断し
FALSE ならば、uItem がメニューアイテムの ID であると判断されます

BOOL SetMenuItemInfo(
	HMENU hMenu , UINT uItem ,
	BOOL fByPosition , LPMENUITEMINFO lpmii
);

BOOL GetMenuItemInfo(
	HMENU hMenu , UINT uItem ,
	BOOL fByPosition , LPMENUITEMINFO lpmii
);

UINT GetMenuState(
  HMENU hMenu,
  UINT  uId,
  UINT  uFlags
);
*/
	
	//if (changeflag == true) {
	//	if (g_undertrackingRMenu == 1) {
	//		MENUITEMINFO menuiteminfo;
	//		ZeroMemory(&menuiteminfo, sizeof(MENUITEMINFO));
	//		menuiteminfo.cbSize = sizeof(MENUITEMINFO);


	//		//typedef struct tagMENUITEMINFO {
	//		//	UINT    cbSize;
	//		//	UINT    fMask;
	//		//	UINT    fType;
	//		//	UINT    fState;
	//		//	UINT    wID;
	//		//	HMENU   hSubMenu;
	//		//	HBITMAP hbmpChecked;
	//		//	HBITMAP hbmpUnchecked;
	//		//	DWORD   dwItemData;
	//		//	LPTSTR  dwTypeData;
	//		//	UINT    cch;
	//		//} MENUITEMINFO, FAR* LPMENUITEMINFO;


	//			/*
	//			IDR_MENU1 MENU
	//			BEGIN
	//				POPUP "File"
	//				BEGIN
	//					MENUITEM "Open",                        ID_FILE_OPEN40001
	//					POPUP "Save"
	//					BEGIN
	//						MENUITEM "Project(*.cha)",              ID_SAVEPROJ_40035
	//						MENUITEM "RigidParams(*ref)",           ID_RESAVE_40028
	//						MENUITEM "ImpulseParams(*.imp)",        ID_IMPSAVE_40030
	//						MENUITEM "GroundParams(*.gco)",         ID_SAVEGCOLI_40033
	//					END
	//					MENUITEM "bvh2FBX",                     ID_FILE_BVH2FBX
	//					MENUITEM "Export bnt",                  ID_FILE_EXPORTBNT
	//				END
	//				POPUP "表示(disp)"
	//				BEGIN
	//					MENUITEM "モーションウインドウ(motion)",          ID_DISPMW40002
	//					MENUITEM "ツールウインドウ(tool)",              4007
	//					MENUITEM "モデルパネル(model)",               40026
	//					MENUITEM "オブジェクトパネル(object)",           40012
	//					MENUITEM "地面オブジェクト(ground)",            ID_DISPGROUND
	//				END
	//				POPUP "モーション(motion)"
	//				BEGIN
	//					MENUITEM "新規空モーション(new empty)",         40004
	//					MENUITEM "編集中モーションの削除(del under editting)", 40006
	//					MENUITEM SEPARATOR
	//					POPUP "モーションの選択(select)"
	//					BEGIN
	//						MENUITEM "PlacingFolder",               ID_40062
	//					END
	//				END
	//				POPUP "モデル(model)"
	//				BEGIN
	//					MENUITEM "編集中のモデルを削除(del under editting)", ID_DELMODEL
	//					MENUITEM "全モデル削除(del all)",             ID_DELALLMODEL
	//					MENUITEM SEPARATOR
	//					POPUP "モデルの選択(select)"
	//					BEGIN
	//						MENUITEM "PlacingFolder",               0
	//					END
	//				END
	//				POPUP "編集・変換(edit, conv)"
	//				BEGIN
	//					MENUITEM "ボーン軸をXに再計算(RecalcAxisX)",     ID_40047
	//					MENUITEM "モーションのリターゲット(retarget)",      ID_40048
	//					MENUITEM "オイラー角　角度制限(limit euler)",     ID_40049
	//					MENUITEM "ボーン座標軸回転(rot axis)",          ID_40050
	//				END
	//				POPUP "剛体設定切り替え(select rigid)"
	//				BEGIN
	//					MENUITEM "PlacingFolder",               0
	//				END
	//				POPUP "ragdoll剛体選択(select ragdoll)"
	//				BEGIN
	//					MENUITEM "PlacingFolder",               0
	//				END
	//				POPUP "ragdollモーフ選択(ragdoll morph)"
	//				BEGIN
	//					MENUITEM "PlacingFolder",               0
	//				END
	//				POPUP "Imp選択(impulse)"
	//				BEGIN
	//					MENUITEM "PlacingFolder",               64500
	//				END
	//				POPUP "HELP"
	//				BEGIN
	//					MENUITEM "Regist",                      29800
	//				END
	//			END

	//			IDR_RMENU MENU
	//			BEGIN
	//				POPUP "RMenu"
	//				BEGIN
	//					MENUITEM "menutitle",                   ID_RMENU_0
	//				END
	//			END

	//			IDR_MENU3 MENU
	//			BEGIN
	//				POPUP "RMenu2"
	//				BEGIN
	//					MENUITEM "全ボーン(all bones)",             ID_RMENU2_40051
	//					MENUITEM "選択ボーン１つ(one bone)",           ID_RMENU2_40052
	//					MENUITEM "選択ボーンと子供ボーン(selected and children)", ID_RMENU2_40053
	//				END
	//			END

	//			IDR_MENU4 MENU
	//			BEGIN
	//				POPUP "RMenu3"
	//				BEGIN
	//					MENUITEM "Rigの設定(set)",                 ID_RMENU3_RIG40055
	//					MENUITEM "Rigの実行(run)",                 ID_RMENU3_RIG40056
	//				END
	//			END

	//			*/

	//		}
	//	}
	//}

}

void DSAxisLMouseMove()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}


	//each of plate button
	//int okbuttonid = 2;
	//int okbuttondown;
	//int okbuttonup;

	//okbuttondown = s_dsbuttondown[okbuttonid];
	//okbuttonup = s_dsbuttonup[okbuttonid];


	POINT cursorpos;
	::GetCursorPos(&cursorpos);

	float axisX;
	float axisY;

	//int upbutton;
	//int downbutton;
	//int leftbutton;
	//int rightbutton;
	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid
	bool accelaxis1 = 0;
	bool accelaxis2 = 0;
	bool accelflag = false;
	bool accelbothflag = false;

	//upbutton = s_dsaxisMOverSrh[3];
	//downbutton = s_dsaxisOverSrh[3];
	//leftbutton = s_dsaxisMOverSrh[2];
	//rightbutton = s_dsaxisOverSrh[2];

	axisX = s_dsaxisvalue[2];//float
	axisY = s_dsaxisvalue[3];//float

	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));
	accelflag = accelaxis1 || accelaxis2;
	accelbothflag = accelaxis1 && accelaxis2;

	//if (g_undertrackingRMenu == 0) {
		bool changeflag = false;

		float deltascale = 1.0f;
		int deltax = 0;
		int deltay = 0;
		if (accelbothflag) {
			deltascale = 10.0f;
		}
		else if (accelflag) {
			deltascale = 4.0f;
		}
		else {
			deltascale = 1.5f;
		}



		if (axisX != 0.0f) {
			deltax = (int)(axisX * deltascale);
			changeflag = true;
		}
		if (axisY != 0.0f) {
			deltay = (int)(axisY * deltascale);
			changeflag = true;
		}


		//if (upbutton >= 1) {
		//	deltay = -deltascale;
		//	changeflag = true;
		//}
		//if (downbutton >= 1) {
		//	deltay = deltascale;
		//	changeflag = true;
		//}
		//if (leftbutton >= 1) {
		//	deltax = -deltascale;
		//	changeflag = true;
		//}
		//if (rightbutton >= 1) {
		//	deltax = deltascale;
		//	changeflag = true;
		//}


		if (changeflag == true) {
			LPARAM lparam;
			lparam = (cursorpos.y << 16) | cursorpos.x;


			HWND desktopwnd;
			desktopwnd = ::GetDesktopWindow();
			if (desktopwnd) {
				RECT desktoprect;
				//::GetClientRect(desktopwnd, &desktoprect);
				::GetWindowRect(desktopwnd, &desktoprect);
				::ClipCursor(&desktoprect);

				//WM_MENUSELECT
				//MF_MOUSESELECT(上位)
				//下位はメニューID
				//WPARAM wparam = ((MF_MOUSESELECT | MF_SYSMENU) << 16) | 40026;
				//WPARAM wparam = (MF_SYSMENU << 16) | 40026;
				//WPARAM wparam = (MF_SYSMENU << 16) | 2;
				//WPARAM wparam = ((MF_MOUSESELECT | MF_SYSMENU) << 16) | 3;

				//HMENU motmenu = GetSubMenu(s_mainmenu, 2);
				//::SendMessage(s_mainhwnd, WM_MENUSELECT, wparam, (LPARAM)motmenu);
				//::SendMessage(s_mainhwnd, WM_MENUSELECT, wparam, (LPARAM)s_mainmenu);
				//::SendMessage(s_mainhwnd, WM_MENUSELECT, wparam, (LPARAM)40026);

				
				
				//::SetCursorPos(cursorpos.x, cursorpos.y);
				
				::SetCursorPos(cursorpos.x + deltax, cursorpos.y + deltay);				



				//::SendMessage(s_mainhwnd, WM_KEYDOWN, VK_DOWN, 0);


				//const double ScaleX = 0xffff / GetSystemMetrics(SM_CXSCREEN);
				//const double ScaleY = 0xffff / GetSystemMetrics(SM_CYSCREEN);
				//const double ScaleX = (double)0xffff / (double)GetSystemMetrics(SM_CXSCREEN);
				//const double ScaleY = (double)0xffff / (double)GetSystemMetrics(SM_CYSCREEN);
				//const double ScaleX = (double)0xffff / (double)(desktoprect.right - desktoprect.left);
				//const double ScaleY = (double)0xffff / (double)(desktoprect.bottom - desktoprect.top);



				//WM_MOUSEMOVEはカメラ操作時などのときに画面が	MB3D_WND_3Dでドラッグする場合に必要
				if (s_3dwnd) {
					POINT client3dpoint;
					client3dpoint = cursorpos;
					::ScreenToClient(s_3dwnd, &client3dpoint);
					LPARAM threelparam;
					threelparam = (client3dpoint.y << 16) | client3dpoint.x;
					::SendMessage(s_3dwnd, WM_MOUSEMOVE, 0, threelparam);
				}
				if (s_mainhwnd) {
					POINT clientpoint;
					clientpoint = cursorpos;
					::ScreenToClient(s_mainhwnd, &clientpoint);
					LPARAM mainlparam;
					mainlparam = (clientpoint.y << 16) | clientpoint.x;
					::SendMessage(s_mainhwnd, WM_MOUSEMOVE, 0, mainlparam);
				}
				HWND dlghwnd;
				dlghwnd = g_SampleUI.GetHWnd();
				if (dlghwnd) {
					POINT clientpoint;
					clientpoint = cursorpos;
					::ScreenToClient(dlghwnd, &clientpoint);
					LPARAM dlglparam;
					dlglparam = (clientpoint.y << 16) | clientpoint.x;
					::SendMessage(dlghwnd, WM_MOUSEMOVE, 0, dlglparam);
				}


				//dialog ctrlのドラッグは　enter buttonを押している間だけ
				if (s_dspushedOK >= 1) {
					HWND ctrlwnd = GetOFWnd(cursorpos);
					if (ctrlwnd) {
						WCHAR wclassname[MAX_PATH] = { 0L };
						::GetClassNameW(ctrlwnd, wclassname, MAX_PATH);
						//::DSMessageBox(s_anglelimitdlg, wclassname, L"check!!!", MB_OK);
						if (wcscmp(L"msctls_trackbar32", wclassname) == 0) {
							//Slider
							RECT sliderloc;
							::GetWindowRect(ctrlwnd, &sliderloc);
							POINT cappoint = cursorpos;
							::ScreenToClient(ctrlwnd, &cappoint);

							float rangemin;
							float rangemax;
							rangemin = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMIN, 0, 0);
							rangemax = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMAX, 0, 0);
							
							float thumblength;
							thumblength = (float)::SendMessage(ctrlwnd, TBM_GETTHUMBLENGTH, 0, 0);

							//int sliderpos = (int)((float)(cappoint.x - 12 - 274 / 2) / 274.0f * 360.0f);
							int sliderpos = (int)((float)(cappoint.x - thumblength / 2.0f) / (float)(sliderloc.right - sliderloc.left - thumblength) * (float)(rangemax - rangemin) + rangemin);

							::SendMessage(ctrlwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)sliderpos);
							::SendMessage(s_ofhwnd, WM_HSCROLL, 0, (LPARAM)ctrlwnd);
						}
						//else if (wcsstr(wclassname, L"ListBox") != 0) {
						else{
							POINT clientpoint;
							clientpoint = cursorpos;
							::ScreenToClient(ctrlwnd, &clientpoint);
							LPARAM dlglparam;
							dlglparam = (clientpoint.y << 16) | clientpoint.x;
							::SendMessage(ctrlwnd, WM_MOUSEMOVE, 0, dlglparam);
							
						}
					}
					else if (s_getfilenamehwnd) {

						//###############################################################################################################################################
						// GetOpenFileNameのダイアログの２つあるリストボックスの垂直スクロールバー(スクロールバーはFindWindowExで取得できず。EnumChildProcでもクライアントエリア内に入らなかった)
						//###############################################################################################################################################

						//ctrlwnd == 0
						//scrollwnd = FindWindowEx(s_getfilenamehwnd, 0, L"ScrollBar", 0);//   scrollwnd == 0
						//s_ofhwnd = 0;
						s_enumdist.clear();
						::EnumChildWindows(s_getfilenamehwnd, EnumTreeViewProc, (LPARAM)&ctrlwnd);
						//ctrlwnd = GetNearestEnumDist();
						std::vector<ENUMDIST>::iterator itrenumdist;
						for (itrenumdist = s_enumdist.begin(); itrenumdist != s_enumdist.end(); itrenumdist++) {
							HWND listboxwnd = itrenumdist->hwnd;

							//POINT clientpoint;
							//clientpoint = cursorpos;
							//::ScreenToClient(listboxwnd, &clientpoint);
							//LPARAM dlglparam;
							//dlglparam = (clientpoint.y << 16) | clientpoint.x;
							//::SendMessage(listboxwnd, WM_VSCROLL, SB_THUMBTRACK, (LPARAM)dlglparam);

							if (deltay > 0) {
								::SendMessage(listboxwnd, WM_VSCROLL, SB_LINEDOWN, (LPARAM)0);
							}
							else if (deltay < 0) {
								::SendMessage(listboxwnd, WM_VSCROLL, SB_LINEUP, (LPARAM)0);
							}
						}

					}
				}

				

				//::SendMessage(desktopwnd, WM_MOUSEMOVE, 0, (LPARAM)lparam);
			}
		}
	
}

void DSL3R3ButtonMouseHere()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	static HCURSOR s_prevcursor = 0;

	if ((s_dspushedL3 == 1) || (s_dspushedR3 == 1)) {
		if (g_dsmousewait == 0) {
			s_prevcursor = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
			g_dsmousewait = 1;
		}
	}
	else if ((s_dspushedL3 == 0) && (s_dspushedR3 == 0)) {
		if (g_dsmousewait == 1) {
			if (s_prevcursor) {
				::SetCursor(s_prevcursor);
			}
			g_dsmousewait = 0;
		}
	}


	if (g_dsmousewait == 1) {
		if (s_timelineWnd) {
			s_timelineWnd->callRewrite();
		}
		if (s_toolWnd) {
			s_toolWnd->callRewrite();
		}
		if (s_LtimelineWnd) {
			s_LtimelineWnd->callRewrite();
		}
		if (s_placefolderWnd && s_placefolderWnd->getVisible()) {
			s_placefolderWnd->callRewrite();
		}
		if (s_rigidWnd && s_rigidWnd->getVisible()) {
			s_rigidWnd->callRewrite();
		}
		if (s_impWnd && s_impWnd->getVisible()) {
			s_impWnd->callRewrite();
		}
		if (s_gpWnd && s_gpWnd->getVisible()) {
			s_gpWnd->callRewrite();
		}
		if (s_dmpanimWnd && s_dmpanimWnd->getVisible()){
			s_dmpanimWnd->callRewrite();
		}
		if (s_convboneWnd && s_convboneWnd->getVisible()) {
			s_convboneWnd->callRewrite();
		}
		if (s_anglelimitdlg && s_spretargetsw[SPRETARGETSW_LIMITEULER].state) {
			//POINT mousepoint;
			//::GetCursorPos(&mousepoint);			
			//::ScreenToClient(s_anglelimitdlg, &mousepoint);
			//PAINTSTRUCT ps;
			//HDC hdc = BeginPaint(s_anglelimitdlg, &ps);
			//// メモリデバイスコンテキストを作成する
			//HDC hCompatDC = CreateCompatibleDC(hdc);
			//// ロードしたビットマップを選択する
			//HBITMAP hPrevBitmap = (HBITMAP)SelectObject(hCompatDC, g_mouseherebmp);
			//BITMAP bmp;
			//GetObject(g_mouseherebmp, sizeof(BITMAP), &bmp);
			//int BMP_W = (int)bmp.bmWidth;
			//int BMP_H = (int)bmp.bmHeight;
			//BitBlt(hdc, mousepoint.x, mousepoint.y, BMP_W, BMP_H, hCompatDC, 0, 0, SRCCOPY);
			//DeleteDC(hCompatDC);
			////EndPaint(hWnd, &ps);
		}
	}
}

void OnDSMouseHereApeal()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_3dwnd) {
		return;
	}

	static float s_mousehereval = 0.0f;
	static int s_mouseheredir = 0;

	//if (g_dsmousewait >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		::ScreenToClient(s_3dwnd, &cursorpos);
		if (s_spmousehere.sprite) {

			float spawidth = 52.0f;
			int spashift = 50;

			s_spmousehere.dispcenter.x = cursorpos.x + 52 / 2;
			s_spmousehere.dispcenter.y = cursorpos.y + 50 / 2;

			
			ChaVector3 disppos;
			disppos.x = (float)(s_spmousehere.dispcenter.x) / ((float)s_mainwidth / 2.0f) - 1.0f;
			disppos.y = -((float)(s_spmousehere.dispcenter.y) / ((float)s_mainheight / 2.0f) - 1.0f);
			disppos.z = 0.0f;
			ChaVector2 dispsize = ChaVector2(spawidth / (float)s_mainwidth * 2.0f, spawidth / (float)s_mainheight * 2.0f);
			if (s_spmousehere.sprite) {
				CallF(s_spmousehere.sprite->SetPos(disppos), return);
				CallF(s_spmousehere.sprite->SetSize(dispsize), return);

				if (s_mouseheredir == 0) {
					s_mousehereval += 0.08f;
				}
				else if (s_mouseheredir == 1) {
					s_mousehereval -= 0.08f;
				}

				if (s_mousehereval >= 1.0f) {
					s_mousehereval = 1.0;
					s_mouseheredir = 1;
				}
				else if (s_mousehereval <= 0.0f) {
					s_mousehereval = 0.0f;
					s_mouseheredir = 0;
				}
				g_mouseherealpha = s_mousehereval * s_mousehereval;

				ChaVector4 spritecol = ChaVector4(1.0f, 1.0f, 1.0f, g_mouseherealpha);
				CallF(s_spmousehere.sprite->SetColor(spritecol), return);
			}
			else {
				_ASSERT(0);
			}

		}
	//}
}

void DSXButtonCancel()
{
	//Cancel button : メニューのドロップダウンをキャンセルする　Cancel dropdown menu. L2 + X --> Undo, R2 + X --> Redo.

	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_mainhwnd) {
		return;
	}
	if (!s_3dwnd) {
		return;
	}

	int cancelbuttonid = 1;
	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid

	int cancelbuttondown;
	int cancelbuttonup;
	int accelaxis1 = 0;
	int accelaxis2 = 0;

	cancelbuttondown = s_dsbuttondown[cancelbuttonid];
	cancelbuttonup = s_dsbuttonup[cancelbuttonid];
	accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));


	if (cancelbuttondown >= 1) {
		if (accelaxis1 >= 1) {
			//Undo
			OnFrameUndo(true, 0);//fromds, fromdskind
		}
		else if (accelaxis2 >= 1) {
			//Redo
			OnFrameUndo(true, 1);//fromds, fromdskind
		}
		else {
			//TrackPopupMenuの前でSetForegrandWindow(s_mainhwnd)をしている場合に次の関数でpopupを閉じることが出来る。
			PostMessage(s_mainhwnd, WM_KEYDOWN, VK_ESCAPE, 0);
			PostMessage(s_3dwnd, WM_KEYDOWN, VK_ESCAPE, 0);

		}
	}
}


void GetHiLiteSubmenu(HMENU* pcommandsubmenu, int* pcommandsubmenunum, int* pcommandsubmenuno)
{
	if (!pcommandsubmenu || !pcommandsubmenunum || !pcommandsubmenuno) {
		return;
	}
	*pcommandsubmenu = 0;
	*pcommandsubmenunum = 0;
	*pcommandsubmenuno = 0;



	int selectedsubmenuitemno = -1;
	int submenuitemnum;
	submenuitemnum = GetMenuItemCount(s_cursubmenu);
	int submenuitemcnt;
	for (submenuitemcnt = 0; submenuitemcnt < submenuitemnum; submenuitemcnt++) {
		UINT submenuitemstate;
		submenuitemstate = GetMenuState(s_cursubmenu, submenuitemcnt, MF_BYPOSITION);
		if (submenuitemstate == MF_HILITE) {
			selectedsubmenuitemno = submenuitemcnt;
			break;
		}
		else {
			//submenu ２段階までは自動でチェック
			HMENU subsubmenu = 0;
			subsubmenu = GetSubMenu(s_cursubmenu, submenuitemcnt);
			if (subsubmenu) {
				int subsubmenuitemnum;
				subsubmenuitemnum = GetMenuItemCount(subsubmenu);
				int subsubmenuitemcnt;
				for (subsubmenuitemcnt = 0; subsubmenuitemcnt < subsubmenuitemnum; subsubmenuitemcnt++) {
					UINT subsubmenuitemstate;
					subsubmenuitemstate = GetMenuState(subsubmenu, subsubmenuitemcnt, MF_BYPOSITION);
					if (subsubmenuitemstate == MF_HILITE) {
						s_cursubmenu = subsubmenu;//!!!!!!!!!
						selectedsubmenuitemno = subsubmenuitemcnt;
						submenuitemnum = subsubmenuitemnum;
						break;//!!!!!!!!!!!!!!!!
					}
				}
			}
		}
	}

	if (selectedsubmenuitemno == -1) {
		HMENU submenu2 = 0;

		if (g_currentsubmenuid == 0) {
			submenu2 = GetSubMenu(s_cursubmenu, 1);
		}
		else if (g_currentsubmenuid == 2) {
			submenu2 = GetSubMenu(s_cursubmenu, 3);
		}
		else if (g_currentsubmenuid == 3) {
			submenu2 = GetSubMenu(s_cursubmenu, 3);
		}
		else if (g_currentsubmenuid == 4) {
			submenu2 = GetSubMenu(s_cursubmenu, 3);
		}
		else if ((g_currentsubmenuid >= 5) && (g_currentsubmenuid <= 8)) {
			submenu2 = GetSubMenu(s_cursubmenu, 0);
		}

		if (submenu2) {
			{
				int tmpselectedsubmenuitemno = -1;
				int tmpsubmenuitemnum;
				tmpsubmenuitemnum = GetMenuItemCount(submenu2);
				int tmpsubmenuitemcnt;
				for (tmpsubmenuitemcnt = 0; tmpsubmenuitemcnt < tmpsubmenuitemnum; tmpsubmenuitemcnt++) {
					UINT submenuitemstate;
					submenuitemstate = GetMenuState(submenu2, tmpsubmenuitemcnt, MF_BYPOSITION);
					if (submenuitemstate == MF_HILITE) {
						tmpselectedsubmenuitemno = tmpsubmenuitemcnt;
						break;
					}
				}
				if (tmpselectedsubmenuitemno >= 0) {
					*pcommandsubmenu = submenu2;
					*pcommandsubmenunum = tmpsubmenuitemnum;
					*pcommandsubmenuno = tmpselectedsubmenuitemno;
				}
			}
		}
	}
	else {
		*pcommandsubmenu = s_cursubmenu;
		*pcommandsubmenunum = submenuitemnum;
		*pcommandsubmenuno = selectedsubmenuitemno;

	}
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime)
{

	//if ((event >= EVENT_SYSTEM_DIALOGSTART) && (event <= EVENT_SYSTEM_DIALOGEND)) {
	//if((event == EVENT_OBJECT_INVOKED) || (event == EVENT_OBJECT_SELECTION) || (event == EVENT_OBJECT_SELECTIONWITHIN) || (event == EVENT_OBJECT_STATECHANGE)){
	//	WCHAR classname[MAX_PATH] = { 0L };
	//	WCHAR wintext[MAX_PATH] = { 0L };
	//	::GetClassName(hwnd, classname, MAX_PATH);
	//	::GetWindowText(hwnd, wintext, MAX_PATH);
	//	//if (wcscmp(L"Button", classname) == 0) {
	//		if (wcscmp(L"OK", wintext) == 0) {
	//			SendMessage(hwnd, BM_CLICK, 0, 0);
	//		}
	//	//}
	//}
	////::SendMessage(hwnd, WM_COMMAND, IDOK, 0);
	//HWND hwndChild = FindWindowEx(hwnd, 0, L"Button", L"OK");
	//if (hwndChild) {
	//	SendMessage(hwndChild, BM_CLICK, 0, 0);
	//	//::SendMessage(hwndChild, WM_COMMAND, IDOK, 0);
	//}

	//if ((event == EVENT_OBJECT_INVOKED) || (event == EVENT_OBJECT_SELECTION) || (event == EVENT_OBJECT_SELECTIONWITHIN) || (event == EVENT_OBJECT_STATECHANGE)) {
	//if (event == EVENT_SYSTEM_DIALOGSTART) {
	//if(event == EVENT_OBJECT_CREATE){
		WCHAR classname[MAX_PATH] = { 0L };
		WCHAR wintext[MAX_PATH] = { 0L };
		::GetClassName(hwnd, classname, MAX_PATH);
		::GetWindowText(hwnd, wintext, MAX_PATH);


		HWND fgwnd = 0;

		//if (wcscmp(L"error!!!", wintext) == 0) {
		if(wcsstr(wintext, L"error!!!") != 0){
			//::SendMessage(hwnd, WM_COMMAND, IDOK, 0);
			s_messageboxhwnd = hwnd;
			fgwnd = hwnd;
		}
		if (wcsstr(wintext, L"warning!!!") != 0) {
			s_messageboxhwnd = hwnd;
			fgwnd = hwnd;
		}
		if (wcsstr(wintext, L"check!!!") != 0) {
			s_messageboxhwnd = hwnd;
			fgwnd = hwnd;
		}
		if (wcsstr(wintext, L"GetFileNameDlg") != 0) {
			s_getfilenamehwnd = hwnd;
			fgwnd = hwnd;
		}
		if ((wcscmp(classname, L"#32770") == 0) || (wcsstr(wintext, L"フォルダーの参照") != 0)) {//SHGetSpecialFolderLocation
			s_getfilenamehwnd = hwnd;
			fgwnd = hwnd;
		}

		if (fgwnd) {
			RECT dlgrect;
			GetWindowRect(fgwnd, &dlgrect);
			SetCursorPos(dlgrect.left + 25, dlgrect.top + 10);
		}

		//HWND hwndChild = FindWindowExA(hwnd, 0, "Button", "OK");
		//if (hwndChild) {
		//	SendMessage(hwndChild, BM_CLICK, 0, 0);
		//	::SendMessage(hwndChild, WM_COMMAND, IDOK, 0);
		//}
		//else {
		//	HWND hwndChildW = FindWindowExW(hwnd, 0, L"Button", L"OK");
		//	if (hwndChildW) {
		//		SendMessage(hwndChildW, BM_CLICK, 0, 0);
		//		::SendMessage(hwndChild, WM_COMMAND, IDOK, 0);
		//	}
		//}
	//}

	return;
}

void DSOButtonSelectedPopupMenu()
{
	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_cursubmenu) {
		return;
	}

	static HWINEVENTHOOK s_eventhook = 0;

	int okbuttonid = 2;
	int okbuttondown;
	int okbuttonup;

	int cancelbuttonid = 1;
	int cancelbuttondown;
	int cancelbuttonup;

	okbuttondown = s_dsbuttondown[okbuttonid];
	okbuttonup = s_dsbuttonup[okbuttonid];

	cancelbuttondown = s_dsbuttondown[cancelbuttonid];
	cancelbuttonup = s_dsbuttonup[cancelbuttonid];


	if (okbuttonup >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		//if (g_undertrackingRMenu == 1) {
		if (InterlockedAdd(&g_undertrackingRMenu, 0) == 1) {


			HMENU commandsubmenu = 0;
			int commandsubmenunum = 0;
			int	commandsubmenuno = -1;
			GetHiLiteSubmenu(&commandsubmenu, &commandsubmenunum, &commandsubmenuno);
			if (commandsubmenu && (commandsubmenunum >= 1) && (commandsubmenuno >= 0)) {


				InterlockedExchange(&g_undertrackingRMenu, (LONG)0);//コマンド発行が決まったらトラッキングフラグ解除!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


				//::PostMessage(s_mainhwnd, WM_KEYDOWN, VK_RETURN, 0);


				//if (commandsubmenunum == 1) {
					//メニュー項目が１つだけの場合にはポップアップを解除してからWM_COMMANDを呼んでみる
					//TrackPopupMenuの前でSetForegrandWindow(s_mainhwnd)をしている場合に次の関数でpopupを閉じることが出来る。
					PostMessage(s_mainhwnd, WM_KEYDOWN, VK_ESCAPE, 0);
					PostMessage(s_3dwnd, WM_KEYDOWN, VK_ESCAPE, 0);
				//}

				int commandmenuid;
				commandmenuid = GetMenuItemID(commandsubmenu, commandsubmenuno);
				WPARAM wparam;
				//wparam = (selectedsubmenuitemno << 16) | selectedmenuid;
				//wparam = (commandsubmenuno << 16) | commandmenuid;
				wparam = commandmenuid;
				
				
				//LPARAM lparam;
				////lparam = (LPARAM)s_mainmenu;
				//lparam = (LPARAM)commandsubmenu;


				//::SendMessage(s_mainhwnd, WM_COMMAND, wparam, lparam);
				//::SendMessage(s_3dwnd, WM_COMMAND, wparam, lparam);//menuのMsgProcはs_3dwndのメッセージプロック
				::SendMessage(s_3dwnd, WM_COMMAND, wparam, 0);//menuのMsgProcはs_3dwndのメッセージプロック
			}
			else {

				if (s_messageboxhwnd) {
					HWND ctrlwnd = 0;
					::EnumChildWindows(s_messageboxhwnd, EnumIDOKProc, (LPARAM)&ctrlwnd);
					if (ctrlwnd) {
						s_messageboxpushcnt++;

						//一回目はダイアログを出すときのクリック。２回目でIDOKを押す。
						if (s_messageboxpushcnt >= 2) {
							//WPARAM wparam;
							//wparam = (BN_CLICKED << 16) | IDOK;
							//::SendMessage(s_messageboxhwnd, WM_COMMAND, wparam, (LPARAM)ctrlwnd);
							SendMessage(ctrlwnd, BM_CLICK, 0, 0);
						}
					}
					//::SendMessage(s_messageboxhwnd, WM_COMMAND, IDOK, 0);
					//	SendMessage(hwndChild, BM_CLICK, 0, 0);
				}
				else {
					HWND ctrlwnd = 0;
					ctrlwnd = GetOFWnd(cursorpos);
					//s_ofhwnd = 0;
					//s_enumdist.clear();
					//::EnumChildWindows(s_getfilenamehwnd, EnumTreeViewProc, (LPARAM)&ctrlwnd);
					//ctrlwnd = GetNearestEnumDist();
					if (ctrlwnd) {

						int ctrlid;
						ctrlid = GetDlgCtrlID(ctrlwnd);
						
						//::SendMessage(s_getfilenametreeview, WM_COMMAND, wparam, (LPARAM)ctrlwnd);

						POINT dlgpoint;
						dlgpoint = cursorpos;
						::ScreenToClient(s_ofhwnd, &dlgpoint);
						LPARAM dlglparam;
						dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
						::SendMessage(s_ofhwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam);
						//::SendMessage(s_ofhwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, dlglparam);//!!!!!!!!!!!!!!

						WPARAM wparam;
						wparam = (BN_CLICKED << 16) | ctrlid;
						::SendMessage(s_ofhwnd, WM_COMMAND, wparam, (LPARAM)ctrlwnd);

						POINT dlgpoint2;
						dlgpoint2 = cursorpos;
						::ScreenToClient(ctrlwnd, &dlgpoint2);
						LPARAM dlglparam2;
						dlglparam2 = (dlgpoint2.y << 16) | dlgpoint2.x;
						//::SendMessage(ctrlwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam2);
						::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam2);
						//::SendMessage(ctrlwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam2);
						//::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam2);
						//::SendMessage(ctrlwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, dlglparam2);//!!!!!!!!!


						SendMessage(ctrlwnd, BM_CLICK, 0, 0);
						//SendMessage(ctrlwnd, BM_CLICK, 0, 0);

						
						WCHAR ctrlclassname[MAX_PATH] = { 0L };
						GetClassName(ctrlwnd, ctrlclassname, MAX_PATH);
						if (wcsstr(L"ListBox", ctrlclassname) != 0) {

							//WCHAR ctrlwintext[MAX_PATH] = { 0L };
							//GetWindowText(ctrlwnd, ctrlwintext, MAX_PATH);//L""

							POINT lbpoint;
							lbpoint = cursorpos;
							::ScreenToClient(ctrlwnd, &lbpoint);
							LPARAM lblparam;
							lblparam = (lbpoint.y << 16) | lbpoint.x;
							DWORD hitinfo = (DWORD)SendMessage(ctrlwnd, LB_ITEMFROMPOINT, 0, lblparam);
							WORD hitflag;
							WORD hitindex;
							hitflag = HIWORD(hitinfo);
							hitindex = LOWORD(hitinfo);
							if (hitflag == 0) {


								//ディレクトリ指定用のListBoxのときにはエンターキーを押して展開表示する。
								//int itrcnt = 0;
								//int findindex = 0;
								//if (s_enumdist.size() >= 2) {
								//	std::vector<ENUMDIST>::iterator itrenumdist;
								//	for (itrenumdist = s_enumdist.begin(); itrenumdist != s_enumdist.end(); itrenumdist++) {
								//		if (itrenumdist->hwnd == ctrlwnd) {
								//			findindex = itrcnt;
								//			break;
								//		}
								//		itrcnt++;
								//	}
								//}
								//if (findindex == 0) {
								//	::PostMessage(ctrlwnd, WM_KEYDOWN, VK_RETURN, 0);
								//}
								 
								

								//ディレクトリ指定用のListBoxのときにはエンターキーを押して展開表示する。
								RECT ctrlrect;
								GetWindowRect(ctrlwnd, &ctrlrect);
								POINT ctrllefttop;
								ctrllefttop.x = ctrlrect.left;
								ctrllefttop.y = ctrlrect.top;
								ScreenToClient(s_getfilenamehwnd, &ctrllefttop);
								if (ctrllefttop.x > 150) {
									::PostMessage(ctrlwnd, WM_KEYDOWN, VK_RETURN, 0);
								}





								//SendMessage(ctrlwnd, WM_CHAR, VK_RETURN, 0);
								//SendMessage(s_ofhwnd, WM_CHAR, VK_RETURN, 0);
								//SendMessage(s_getfilenamehwnd, WM_CHAR, VK_RETURN, 0);

								//SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
								//SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)TRUE, (LPARAM)hitindex);
								
								
								
								
								//SendMessage(ctrlwnd, LB_SETCURSEL, (WPARAM)hitindex, (LPARAM)0);

								//::SendMessage(ctrlwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, dlglparam2);//!!!!!!!!!

								//WCHAR itembuf[MAX_PATH] = { 0L };
								//LVITEM item;
								//ZeroMemory(&item, sizeof(LVITEM));
								//item.mask = LVIF_STATE | LVIF_TEXT;
								//item.state = 0;
								//item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
								//item.iItem = hitindex;
								//item.iSubItem = 0;          //取得するサブアイテムの番号
								//item.pszText = itembuf;         //格納するテキストバッファ
								//item.cchTextMax = MAX_PATH; //バッファ容量
								//ListView_GetItem(ctrlwnd, &item);
								//if (item.state & LVIS_SELECTED) {
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
								//}
								//else if (item.state & LVIS_FOCUSED) {
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
								//}
								//else {
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)TRUE, (LPARAM)hitindex);
								//}

								
								//DWORD itemstate = ListView_GetItemState(ctrlwnd, hitindex, LVIS_SELECTED | LVIS_FOCUSED);
								//if ((itemstate & LVIS_SELECTED) != 0) {
								//	//ListView_SetItemState(ctrlwnd, hitindex, 0, LVIS_SELECTED);
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
								//}
								//else if ((itemstate & LVIS_FOCUSED) != 0) {
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
								//}
								//else {
								//	SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)TRUE, (LPARAM)hitindex);
								//}
							}
						}
						else if ((wcsstr(L"TreeView", ctrlclassname) != 0) || (wcsstr(L"SysTreeView32", ctrlclassname) != 0)) {
						//for SHGetSpecialFolderLocation
						//select : click text
						//expand tree : click triangleMark

							POINT tvpoint;
							tvpoint = cursorpos;
							::ScreenToClient(ctrlwnd, &tvpoint);
							LPARAM tvlparam;
							tvlparam = (tvpoint.y << 16) | tvpoint.x;

							TVHITTESTINFO ht;
							ZeroMemory(&ht, sizeof(TVHITTESTINFO));
							ht.flags = TVHT_ONITEM;
							ht.hItem = NULL;
							ht.pt = tvpoint;
							HTREEITEM hItem = TreeView_HitTest(ctrlwnd, &ht);
							if (ht.flags & TVHT_ONITEM) {
								TreeView_SelectItem(ctrlwnd, ht.hItem);
							}
						}


						//WPARAM wparam;
						//wparam = (BN_CLICKED << 16) | ctrlid;
						////wparam = (BN_PUSHED << 16) | ctrlid;
						//::SendMessage(s_ofhwnd, WM_COMMAND, wparam, (LPARAM)ctrlwnd);
						//::SendMessage(ctrlwnd, BM_CLICK, 0, 0);

						//HWND hList = ctrlwnd;
						//LV_HITTESTINFO lvinfo;
						//WCHAR buf[MAX_PATH];
						//ZeroMemory(&lvinfo, sizeof(LV_HITTESTINFO));
						//GetCursorPos((LPPOINT)&lvinfo.pt);
						//ScreenToClient(hList, &lvinfo.pt);
						//ListView_HitTest(hList, &lvinfo);
						//if ((lvinfo.flags & LVHT_ONITEM) != 0)
						//{
						//	LVITEM item;
						//	ZeroMemory(&item, sizeof(LVITEM));
						//	item.mask = TVIF_HANDLE | TVIF_TEXT;
						//	item.iItem = lvinfo.iItem;
						//	item.iSubItem = 0;          //取得するサブアイテムの番号
						//	item.pszText = buf;         //格納するテキストバッファ
						//	item.cchTextMax = MAX_PATH; //バッファ容量
						//	ListView_GetItem(hList, &item);
						//	DSMessageBox(s_3dwnd, buf, L"選択したデータ", MB_OK);
						//}
						


						//dHWND treeviewhwnd = 0;
						//::EnumChildWindows(s_ofhwnd, EnumTreeViewProc, (LPARAM)&treeviewhwnd);
						//if (treeviewhwnd) {
						//	s_getfilenametreeview = treeviewhwnd;

						//	::SendMessage(s_getfilenametreeview, WM_COMMAND, wparam, (LPARAM)ctrlwnd);

						//	POINT dlgpoint;
						//	dlgpoint = cursorpos;
						//	::ScreenToClient(s_ofhwnd, &dlgpoint);
						//	LPARAM dlglparam;
						//	dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
						//	::SendMessage(s_ofhwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam);

						//	POINT dlgpoint2;
						//	dlgpoint2 = cursorpos;
						//	::ScreenToClient(s_getfilenametreeview, &dlgpoint2);
						//	LPARAM dlglparam2;
						//	dlglparam2 = (dlgpoint2.y << 16) | dlgpoint2.x;
						//	::SendMessage(s_getfilenametreeview, WM_LBUTTONUP, MK_LBUTTON, dlglparam2);
						//}
					}
				}



				//if(s_eventhook){
				//	UnhookWinEvent(s_eventhook);
				//	s_eventhook = 0;
				//}
				//m_event = nullptr;
			}
		}


		//int selectedsubmenuitemno = -1;
		//int submenuitemnum;
		//submenuitemnum = GetMenuItemCount(s_cursubmenu);
		//int submenuitemcnt;
		//for (submenuitemcnt = 0; submenuitemcnt < submenuitemnum; submenuitemcnt++) {
		//	UINT submenuitemstate;
		//	submenuitemstate = GetMenuState(s_cursubmenu, submenuitemcnt, MF_BYPOSITION);
		//	if (submenuitemstate == MF_HILITE) {
		//		selectedsubmenuitemno = submenuitemcnt;
		//		break;
		//	}
		//}
				//if (selectedmenuid >= 0) {
				//	//#################
				//	//選択決定成功例その２
				//	//#################
				//	//第2項目（インデックス値 = 1, ID = 0x2711）を選択したところ、
				//	//wParam = 0x00012711
				//	//のように、上位2バイトにインデックス値が、下位2バイトにIDが入ります。
				//	//WPARAM wparam;
				//	//wparam = (g_currentsubmenuid << 16) | curmenuitemid;
				//	//LPARAM lparam;
				//	//lparam = (LPARAM)mainmenu;
				//	//::SendMessage(s_mainhwnd, WM_COMMAND, wparam, lparam);

				//	if ((submenuitemnum == 1) && (selectedsubmenuitemno >= 0) && (g_currentsubmenuid >= 0)) {
				//		//メニュー項目が１つだけの場合にはポップアップを解除してからWM_COMMANDを呼んでみる
				//		//TrackPopupMenuの前でSetForegrandWindow(s_mainhwnd)をしている場合に次の関数でpopupを閉じることが出来る。
				//		PostMessage(s_mainhwnd, WM_KEYDOWN, VK_ESCAPE, 0);
				//		PostMessage(s_3dwnd, WM_KEYDOWN, VK_ESCAPE, 0);
				//	}
				//	
				//	int commandmenuid;
				//	commandmenuid = GetMenuItemID(s_cursubmenu, selectedsubmenuitemno);
				//	WPARAM wparam;
				//	//wparam = (selectedsubmenuitemno << 16) | selectedmenuid;
				//	wparam = (selectedsubmenuitemno << 16) | commandmenuid;
				//	LPARAM lparam;
				//	//lparam = (LPARAM)s_mainmenu;
				//	lparam = (LPARAM)s_cursubmenu;
				//	::SendMessage(s_mainhwnd, WM_COMMAND, wparam, lparam);
				//}
	}


	if (okbuttondown >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		//if (g_undertrackingRMenu == 1) {
		if (InterlockedAdd(&g_undertrackingRMenu, 0) == 1) {
			HMENU commandsubmenu = 0;
			int commandsubmenunum = 0;
			int	commandsubmenuno = -1;
			GetHiLiteSubmenu(&commandsubmenu, &commandsubmenunum, &commandsubmenuno);
			if (commandsubmenu && (commandsubmenunum >= 1) && (commandsubmenuno >= 0)) {
				//
			}
			else {
				HWND ctrlwnd = 0;
				ctrlwnd = GetOFWnd(cursorpos);
				//s_ofhwnd = 0;
				//s_enumdist.clear();
				//::EnumChildWindows(s_getfilenamehwnd, EnumTreeViewProc, (LPARAM)&ctrlwnd);
				//ctrlwnd = GetNearestEnumDist();
				if (ctrlwnd) {

					int ctrlid;
					ctrlid = GetDlgCtrlID(ctrlwnd);

					//::SendMessage(s_getfilenametreeview, WM_COMMAND, wparam, (LPARAM)ctrlwnd);

					POINT dlgpoint;
					dlgpoint = cursorpos;
					::ScreenToClient(s_ofhwnd, &dlgpoint);
					LPARAM dlglparam;
					dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
					::SendMessage(s_ofhwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam);
					//::SendMessage(s_ofhwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, dlglparam);//!!!!!!!!!!!!!!

					WPARAM wparam;
					wparam = (BN_CLICKED << 16) | ctrlid;
					//::SendMessage(s_ofhwnd, WM_COMMAND, wparam, (LPARAM)ctrlwnd);

					POINT dlgpoint2;
					dlgpoint2 = cursorpos;
					::ScreenToClient(ctrlwnd, &dlgpoint2);
					LPARAM dlglparam2;
					dlglparam2 = (dlgpoint2.y << 16) | dlgpoint2.x;
					//::SendMessage(ctrlwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam2);
					::SendMessage(ctrlwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam2);
				}
			
		






				//HWND treeviewhwnd = 0;
				//treeviewhwnd = FindWindowExW(s_getfilenamehwnd, 0, WC_LISTVIEW, 0);

				//WCHAR dlgclassname[MAX_PATH] = { 0L };
				//GetClassName(s_getfilenamehwnd, dlgclassname, MAX_PATH);
				//WCHAR dlgwintext[MAX_PATH] = { 0L };
				//GetWindowText(s_getfilenamehwnd, dlgwintext, MAX_PATH);



				//HWND ctrlwnd = 0;
				//ctrlwnd = GetOFWnd(cursorpos);
				//if (ctrlwnd) {
				//	WCHAR ctrlclassname[MAX_PATH] = { 0L };
				//	GetClassName(ctrlwnd, ctrlclassname, MAX_PATH);
				//	WCHAR ctrlwintext[MAX_PATH] = { 0L };
				//	GetWindowText(ctrlwnd, ctrlwintext, MAX_PATH);

				//	int ctrlid;
				//	ctrlid = GetDlgCtrlID(ctrlwnd);
				//	WPARAM wparam;
				//	wparam = (BN_CLICKED << 16) | ctrlid;
				//	//wparam = (BN_PUSHED << 16) | ctrlid;
				//	//::SendMessage(s_ofhwnd, WM_COMMAND, wparam, (LPARAM)ctrlwnd);
				//	//::SendMessage(ctrlwnd, BM_CLICK, 0, 0);

				//	//HWND parenthwnd = 0;
				//	//parenthwnd = GetParent(ctrlwnd);
				//	//if (parenthwnd) {
				//	//	WCHAR parentclassname[MAX_PATH] = { 0L };
				//	//	GetClassName(parenthwnd, parentclassname, MAX_PATH);
				//	//	WCHAR parentwintext[MAX_PATH] = { 0L };
				//	//	GetWindowText(parenthwnd, parentwintext, MAX_PATH);

				//	//	int dbgcnt;
				//	//	dbgcnt = 0;
				//	//	//{
				//	//	//	LVCOLUMN lvcol;
				//	//	//	LVITEM item;
				//	//	//	UINT i;
				//	//	//	//HWND hList = parenthwnd;
				//	//	//	//HWND hList = ctrlwnd;
				//	//	//	HWND hList = s_getfilenamehwnd;
				//	//	//	LV_HITTESTINFO lvinfo;
				//	//	//	WCHAR buf[MAX_PATH];

				//	//	//	ZeroMemory(&lvinfo, sizeof(LV_HITTESTINFO));

				//	//	//	GetCursorPos((LPPOINT)&lvinfo.pt);
				//	//	//	ScreenToClient(hList, &lvinfo.pt);
				//	//	//	ListView_HitTest(hList, &lvinfo);
				//	//	//	if ((lvinfo.flags & LVHT_ONITEM) != 0)
				//	//	//	{
				//	//	//		item.mask = TVIF_HANDLE | TVIF_TEXT;
				//	//	//		item.iItem = lvinfo.iItem;
				//	//	//		item.iSubItem = 0;          //取得するサブアイテムの番号
				//	//	//		item.pszText = buf;         //格納するテキストバッファ
				//	//	//		item.cchTextMax = MAX_PATH; //バッファ容量
				//	//	//		ListView_GetItem(hList, &item);
				//	//	//		DSMessageBox(s_3dwnd, buf, L"選択したデータ", MB_OK);
				//	//	//	}
				//	//	//}
				//	//	//int hotitem;
				//	//	//int parenthotitem;
				//	//	//hotitem = ListView_GetHotItem(ctrlwnd);
				//	//	//parenthotitem = ListView_GetHotItem(parenthwnd);
				//	//	//if (hotitem > 0) {
				//	//	//	_ASSERT(0);
				//	//	//}
				//	//	//if (parenthotitem > 0) {
				//	//	//	_ASSERT(0);
				//	//	//}

				//	//	//s_getfilenametreeview = parenthwnd;


				//	//	//POINT treepos;
				//	//	//treepos = cursorpos;
				//	//	////ScreenToClient(ctrlwnd, &treepos);
				//	//	////ScreenToClient(s_getfilenametreeview, &treepos);
				//	//	////ScreenToClient(s_ofhwnd, &treepos);
				//	//	//TVHITTESTINFO ht;
				//	//	//ZeroMemory(&ht, sizeof(TVHITTESTINFO));
				//	//	//ht.flags = TVHT_ONITEM;
				//	//	//ht.hItem = NULL;
				//	//	//ht.pt = treepos;
				//	//	////ht.pt.x = cursorpos.x;
				//	//	////ht.pt.y = cursorpos.y;
				//	//	////TreeView_HitTest(s_getfilenametreeview, &ht);
				//	//	//HTREEITEM hItem = TreeView_HitTest(ctrlwnd, &ht);
				//	//	//if (ht.flags & TVHT_ONITEM) {
				//	//	//	//TreeView_SelectItem(s_getfilenametreeview, hititem);

				//	//	//	//TVITEM ti;
				//	//	//	//ZeroMemory(&ti, sizeof(TVITEM));
				//	//	//	//ti.mask = TVIF_HANDLE | TVIF_PARAM;
				//	//	//	//ti.hItem = hititem;
				//	//	//	//TreeView_GetItem(s_getfilenametreeview, &ti);
				//	//	//	//int clickno = (int)tvi.lParam;

				//	//	//	//TreeView_Select(s_getfilenametreeview, ht.hItem, TVGN_CARET);
				//	//	//	TreeView_Select(ctrlwnd, ht.hItem, TVGN_CARET);


				//	//	//	int dbgcnt = 0;
				//	//	//}
				//	//}


				//	//s_ofhwnd = 0;
				//	//s_enumdist.clear();
				//	//::EnumChildWindows(s_getfilenamehwnd, EnumTreeViewProc, (LPARAM)&treeviewhwnd);
				//	//treeviewhwnd = GetNearestEnumDist();
				////if (treeviewhwnd) {
				//	//s_getfilenametreeview = treeviewhwnd;


				//	//POINT dlgpoint;
				//	//dlgpoint = cursorpos;
				//	//::ScreenToClient(s_ofhwnd, &dlgpoint);
				//	//LPARAM dlglparam;
				//	//dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
				//	//::SendMessage(s_ofhwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam);

				//	//POINT dlgpoint2;
				//	//dlgpoint2 = cursorpos;
				//	//::ScreenToClient(s_getfilenametreeview, &dlgpoint2);
				//	//LPARAM dlglparam2;
				//	//dlglparam2 = (dlgpoint2.y << 16) | dlgpoint2.x;
				//	//::SendMessage(s_getfilenametreeview, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam2);




				//}
				//}
					//if (!s_eventhook) {
					//	s_eventhook = SetWinEventHook(
					//		EVENT_SYSTEM_DIALOGSTART,
					//		//EVENT_SYSTEM_DIALOGSTART,
					//		EVENT_SYSTEM_DIALOGEND,
					//		NULL,
					//		WinEventProc,
					//		//GetDlgItemInt(IDC_PROCESSID),
					//		//GetProcessId(GetModuleHandle(NULL)),
					//		//GetThreadId(g_hUnderTrackingThread),
					//		//GetThreadId(s_messageboxthread),
					//		0,
					//		0,
					//		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
					//	);
					//}
					/*if (m_event != nullptr)
					{
						SetDlgItemText(IDOK, _T("停止"));
					}*/
			}
		}
	}


	if (cancelbuttonup >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		//if (g_undertrackingRMenu == 1) {
		if (InterlockedAdd(&g_undertrackingRMenu, 0) == 1) {
			if (s_messageboxhwnd) {
			}
			else {
				HWND ctrlwnd = 0;
				ctrlwnd = GetOFWnd(cursorpos);
				if (ctrlwnd) {
					int ctrlid;
					ctrlid = GetDlgCtrlID(ctrlwnd);
					WCHAR ctrlclassname[MAX_PATH] = { 0L };
					GetClassName(ctrlwnd, ctrlclassname, MAX_PATH);
					if (wcscmp(L"ListBox", ctrlclassname) == 0) {

						POINT lbpoint;
						lbpoint = cursorpos;
						::ScreenToClient(ctrlwnd, &lbpoint);
						LPARAM lblparam;
						lblparam = (lbpoint.y << 16) | lbpoint.x;
						DWORD hitinfo = (DWORD)SendMessage(ctrlwnd, LB_ITEMFROMPOINT, 0, lblparam);
						WORD hitflag;
						WORD hitindex;
						hitflag = HIWORD(hitinfo);
						hitindex = LOWORD(hitinfo);
						if (hitflag == 0) {
							//X buttonで  selectをCancel
							SendMessage(ctrlwnd, LB_SETSEL, (WPARAM)FALSE, (LPARAM)hitindex);
						}
					}
					/*else if (wcscmp(L"TreeView", ctrlclassname) == 0) {
						POINT tvpoint;
						tvpoint = cursorpos;
						::ScreenToClient(ctrlwnd, &tvpoint);
						LPARAM tvlparam;
						tvlparam = (tvpoint.y << 16) | tvpoint.x;

						TVHITTESTINFO ht;
						ZeroMemory(&ht, sizeof(TVHITTESTINFO));
						ht.flags = TVHT_ONITEM;
						ht.hItem = NULL;
						ht.pt = tvpoint;
						HTREEITEM hItem = TreeView_HitTest(ctrlwnd, &ht);
						if (ht.flags & TVHT_ONITEM) {
							TreeView_SelectItem(ctrlwnd, ht.hItem);
						}
					}*/
				}
			}
		}
	}
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	if (!lParam) {
		return FALSE;
	}
	HWND* rethwnd = (HWND*)lParam;
	*rethwnd = 0;

	POINT mousepoint;
	::GetCursorPos(&mousepoint);
	::ScreenToClient(hwnd, &mousepoint);

	RECT ctrlrect;
	GetClientRect(hwnd, &ctrlrect);



	if ((mousepoint.x >= ctrlrect.left) && (mousepoint.x <= ctrlrect.right)
		&& (mousepoint.y >= ctrlrect.top) && (mousepoint.y <= ctrlrect.bottom)) {
		//if ((mousepoint.x >= 0) && (mousepoint.x <= (ctrlrect.right - ctrlrect.left))
		//	&& (mousepoint.y >= 0) && (mousepoint.y <= (ctrlrect.bottom - ctrlrect.top))) {
		
		//#################################################################################################################################################
		//groupboxのclassnameもButton。groupboxの中のコントロールを押すためには距離で判定する必要があった。マウス位置に一番近いコントロールを探すための情報をs_enumdistにpush_back
		//#################################################################################################################################################
		POINT ctrllefttop;
		//ctrlcenter.x = (ctrlrect.right - ctrlrect.left) / 2;//ラジオボタンなどはコントロール真ん中ではなく左側を押すことが多いので中央はやめる
		//ctrlcenter.y = (ctrlrect.bottom - ctrlrect.top) / 2;
		ctrllefttop.x = ctrlrect.left;
		ctrllefttop.y = ctrlrect.top;

		ENUMDIST enumdist;
		enumdist.hwnd = hwnd;
		enumdist.dist = (float)(mousepoint.x - ctrllefttop.x) * (float)(mousepoint.x - ctrllefttop.x) + (float)(mousepoint.y - ctrllefttop.y) * (float)(mousepoint.y - ctrllefttop.y);
		s_enumdist.push_back(enumdist);


		//*rethwnd = hwnd;
		//return FALSE;//探索終了

		return TRUE;//探索続行
	}
	else {
		*rethwnd = 0;
		return TRUE;//探索続行
	}

}

BOOL CALLBACK EnumTreeViewProc(HWND hwnd, LPARAM lParam)
{
	if (!lParam) {
		return FALSE;
	}
	HWND* rethwnd = (HWND*)lParam;
	*rethwnd = 0;

	WCHAR classname[MAX_PATH] = { 0L };
	::GetClassName(hwnd, classname, MAX_PATH);

	//if ((wcscmp(L"SysListView32", classname) == 0) || (wcscmp(L"SysTreeView32", classname) == 0)) {
	if ((wcsstr(classname, L"ListBox") != 0) || (wcsstr(classname, L"TreeView") != 0)) {
		//*rethwnd = hwnd;
		//return FALSE;//探索終了

		POINT mousepoint;
		::GetCursorPos(&mousepoint);
		::ScreenToClient(hwnd, &mousepoint);
		//::ScreenToClient(s_getfilenamehwnd, &mousepoint);

		RECT ctrlrect;
		GetClientRect(hwnd, &ctrlrect);
		//GetWindowRect(hwnd, &ctrlrect);

		POINT ctrllefttop;
		//ctrlcenter.x = (ctrlrect.right - ctrlrect.left) / 2;//ラジオボタンなどはコントロール真ん中ではなく左側を押すことが多いので中央はやめる
		//ctrlcenter.y = (ctrlrect.bottom - ctrlrect.top) / 2;
		ctrllefttop.x = ctrlrect.left;
		ctrllefttop.y = ctrlrect.top;

		ENUMDIST enumdist;
		enumdist.hwnd = hwnd;
		enumdist.dist = (float)(mousepoint.x - ctrllefttop.x) * (float)(mousepoint.x - ctrllefttop.x) + (float)(mousepoint.y - ctrllefttop.y) * (float)(mousepoint.y - ctrllefttop.y);
		s_enumdist.push_back(enumdist);

		*rethwnd = 0;
		return TRUE;//探索続行
	}
	else {
		*rethwnd = 0;
		return TRUE;//探索続行
	}
}



BOOL CALLBACK EnumIDOKProc(HWND hwnd, LPARAM lParam)
{
	if (!lParam) {
		return FALSE;
	}
	HWND* rethwnd = (HWND*)lParam;
	*rethwnd = 0;

	WCHAR wintext[MAX_PATH] = { 0L };
	::GetWindowText(hwnd, wintext, MAX_PATH);

	if(wcscmp(L"OK", wintext) == 0){
		*rethwnd = hwnd;
		return FALSE;//探索終了
	}
	else {
		*rethwnd = 0;
		return TRUE;//探索続行
	}

}


//BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
//{
//	if (!lParam) {
//		return FALSE;
//	}
//	HWND* rethwnd = (HWND*)lParam;
//	//*rethwnd = 0;
//
//	WCHAR szBuff[512];
//	GetWindowTextW(hwnd, szBuff, 512);
//	int cmp;
//	cmp = wcscmp(L"OpenMqoDlg", szBuff);
//	if (cmp == 0) {
//		//HWND hbtn = NULL;
//		//::EnumChildWindows(hwnd, EnumChildProcOF, reinterpret_cast<LPARAM>(&hbtn));
//		//if (hbtn) {
//		*rethwnd = hwnd;
//		return FALSE;
//		//}
//		//else {
//		//	return TRUE;
//		//}
//	}
//	else {
//		//*rethwnd = 0;
//		return TRUE;
//	}
//}

HWND GetNearestEnumDist()
{
	if (s_enumdist.empty()) {
		return 0;
	}
	else {
		bool isfirst = true;
		float nearestdist = 1e20;
		HWND nearesthwnd = 0;
		std::vector<ENUMDIST>::iterator itrenumdist;
		for (itrenumdist = s_enumdist.begin(); itrenumdist != s_enumdist.end(); itrenumdist++) {
			if (isfirst) {
				nearesthwnd = itrenumdist->hwnd;
				nearestdist = itrenumdist->dist;
				isfirst = false;
			}
			else {
				if (nearestdist > itrenumdist->dist) {
					nearesthwnd = itrenumdist->hwnd;
					nearestdist = itrenumdist->dist;
				}
			}
		}

		return nearesthwnd;
	}
}


HWND GetOFWnd(POINT srcpoint)
{
	HWND retctrlwnd = 0;

	s_ofhwnd = 0;
	s_enumdist.clear();

	if (!retctrlwnd && s_getfilenamehwnd) {
		::EnumChildWindows(s_getfilenamehwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_getfilenamehwnd;
			return retctrlwnd;
		}
	}else if (!retctrlwnd && s_mqodlghwnd) {
		::EnumChildWindows(s_mqodlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_mqodlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_underframecopydlg && s_selbonedlg.m_hWnd) {
		::EnumChildWindows(s_selbonedlg.m_hWnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_selbonedlg.m_hWnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_undercolidlg && s_pcolidlg && s_pcolidlg->m_hWnd && IsWindow(s_pcolidlg->m_hWnd)) {
		::EnumChildWindows(s_pcolidlg->m_hWnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_pcolidlg->m_hWnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_undergcolidlg && s_pgcolidlg && s_pgcolidlg->m_hWnd && IsWindow(s_pgcolidlg->m_hWnd)) {
		::EnumChildWindows(s_pgcolidlg->m_hWnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_pgcolidlg->m_hWnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_motpropdlghwnd) {
		::EnumChildWindows(s_motpropdlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_motpropdlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && g_filterdlghwnd) {
		::EnumChildWindows(g_filterdlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = g_filterdlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && (s_currentwndid == MB3D_WND_SIDE) && s_anglelimitdlg && (s_platemenukind == SPPLATEMENUKIND_RETARGET) && (s_platemenuno == (SPRETARGETSW_LIMITEULER + 1))) {
		//if (s_anglelimitdlg && (s_platemenukind == SPPLATEMENUKIND_RETARGET) && (s_platemenuno == (SPRETARGETSW_LIMITEULER + 1))) {
		if ((s_curdseullimitctrlno >= 0) && (s_curdseullimitctrlno < s_dseullimitctrls.size()) && s_dseullimitctrls[s_curdseullimitctrlno]) {
			::EnumChildWindows(s_anglelimitdlg, EnumChildProc, (LPARAM)&retctrlwnd);
			retctrlwnd = GetNearestEnumDist();
			if (retctrlwnd) {
				s_ofhwnd = s_anglelimitdlg;
				return retctrlwnd;
			}
		}
	}
	else if (!retctrlwnd && s_savechadlghwnd) {
		::EnumChildWindows(s_savechadlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_savechadlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_bvhdlghwnd) {
		::EnumChildWindows(s_bvhdlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_bvhdlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_saveredlghwnd) {
		::EnumChildWindows(s_saveredlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_saveredlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_savegcodlghwnd) {
		::EnumChildWindows(s_savegcodlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_savegcodlghwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_rotzisdlghwnd) {
		::EnumChildWindows(s_rotzisdlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_rotzisdlghwnd;

			//WCHAR chkclassname[MAX_PATH] = { 0L };
			//::GetClassName(retctrlwnd, chkclassname, MAX_PATH);
			//::DSMessageBox(NULL, chkclassname, L"check!!!", MB_OK);

			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_customrighwnd) {
		::EnumChildWindows(s_customrighwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_customrighwnd;
			return retctrlwnd;
		}
	}
	else if (!retctrlwnd && s_mqodlghwnd) {
		::EnumChildWindows(s_exportxdlghwnd, EnumChildProc, (LPARAM)&retctrlwnd);
		retctrlwnd = GetNearestEnumDist();
		if (retctrlwnd) {
			s_ofhwnd = s_exportxdlghwnd;
			return retctrlwnd;
		}
	}

	return 0;

}

void DSOptionButtonRightClick()
{
	//optionボタンは右クリック相当
	//カーソルがジョイント位置にあるときに右クリックするとコンテクストメニューが出る。

	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	int optbuttonid = 11;
	int optbuttondown;
	int optbuttonup;

	optbuttondown = s_dsbuttondown[optbuttonid];
	optbuttonup = s_dsbuttonup[optbuttonid];

	if (optbuttondown >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		if (g_undertrackingRMenu == 0) {
			if ((s_currentwndid == MB3D_WND_3D) && (s_curboneno >= 0)) {
				POINT cappoint;
				cappoint = cursorpos;
				::ScreenToClient(s_3dwnd, &cappoint);
				LPARAM caplparam;
				caplparam = (cappoint.y << 16) | cappoint.x;

				::SendMessage(s_3dwnd, WM_RBUTTONDOWN, MK_RBUTTON, caplparam);
			}
		}
	}

	if (optbuttonup >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		if (g_undertrackingRMenu == 0) {
			if ((s_currentwndid == MB3D_WND_3D) && (s_curboneno >= 0)) {
				POINT cappoint;
				cappoint = cursorpos;
				::ScreenToClient(s_3dwnd, &cappoint);
				LPARAM caplparam;
				caplparam = (cappoint.y << 16) | cappoint.x;

				::SendMessage(s_3dwnd, WM_RBUTTONUP, MK_RBUTTON, caplparam);
			}
		}
	}

}


void DSAimBarOK()
{

	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	int okbuttonid = 2;
	int okbuttondown;
	int okbuttonup;

	okbuttondown = s_dsbuttondown[okbuttonid];
	okbuttonup = s_dsbuttonup[okbuttonid];

	if (okbuttondown >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		//if (g_undertrackingRMenu == 0) {
		if (InterlockedAdd(&g_undertrackingRMenu, 0) == 0) {
			{
				//dialog ctrl
				HWND ctrlwnd;
				ctrlwnd = GetOFWnd(cursorpos);
				if (ctrlwnd) {
					
					WCHAR wclassname[MAX_PATH] = { 0L };
					::GetClassNameW(ctrlwnd, wclassname, MAX_PATH);
					//::DSMessageBox(s_anglelimitdlg, wclassname, L"check!!!", MB_OK);
					if (wcscmp(L"ComboBox", wclassname) == 0) {
						//COMBOBOX
						::SendMessage(ctrlwnd, CB_SHOWDROPDOWN, TRUE, 0);

						//HWND caphwnd;
						//caphwnd = ::GetCapture();
						//if (caphwnd && IsWindow(caphwnd)) {
						//	::SendMessage(caphwnd, CB_SHOWDROPDOWN, TRUE, 0);
						//}
					}
					else if (wcscmp(L"msctls_trackbar32", wclassname) == 0) {
						//Slider
						RECT sliderloc;
						::GetWindowRect(ctrlwnd, &sliderloc);
						POINT cappoint = cursorpos;
						::ScreenToClient(ctrlwnd, &cappoint);

						float rangemin;
						float rangemax;
						rangemin = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMIN, 0, 0);
						rangemax = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMAX, 0, 0);

						float thumblength;
						thumblength = (float)::SendMessage(ctrlwnd, TBM_GETTHUMBLENGTH, 0, 0);

						//int sliderpos = (int)((float)(cappoint.x - 12 - 274 / 2) / 274.0f * 360.0f);
						int sliderpos = (int)((float)(cappoint.x - thumblength / 2.0f) / (float)(sliderloc.right - sliderloc.left - thumblength) * (float)(rangemax - rangemin) + rangemin);

						::SendMessage(ctrlwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)sliderpos);
						::SendMessage(s_ofhwnd, WM_HSCROLL, 0, (LPARAM)ctrlwnd);
					}
					else {
						POINT dlgpoint;
						dlgpoint = cursorpos;
						::ScreenToClient(s_ofhwnd, &dlgpoint);
						LPARAM dlglparam;
						dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
						::SendMessage(s_ofhwnd, WM_LBUTTONDOWN, MK_LBUTTON, dlglparam);

						POINT cappoint;
						cappoint = cursorpos;
						::ScreenToClient(ctrlwnd, &cappoint);
						LPARAM caplparam;
						caplparam = (cappoint.y << 16) | cappoint.x;
						::SendMessage(ctrlwnd, WM_LBUTTONDOWN, MK_LBUTTON, caplparam);
					}
				}
				else {
					bool doneflag1 = false;
					if (s_dispmodel && s_modelpanel.panel && s_modelpanel.panel->getHWnd()) {
						RECT panelrect;
						GetWindowRect(s_modelpanel.panel->getHWnd(), &panelrect);
						if ((cursorpos.x >= panelrect.left) && (cursorpos.x <= panelrect.right) && 
							(cursorpos.y >= panelrect.top) && (cursorpos.y <= panelrect.bottom)) {
							POINT cappoint;
							cappoint = cursorpos;
							::ScreenToClient(s_modelpanel.panel->getHWnd(), &cappoint);
							LPARAM caplparam;
							caplparam = (cappoint.y << 16) | cappoint.x;
							::SendMessage(s_modelpanel.panel->getHWnd(), WM_LBUTTONDOWN, MK_LBUTTON, caplparam);
							SetCapture(s_modelpanel.panel->getHWnd());
							doneflag1 = true;
						}
					}
					if(!doneflag1){
						if (s_dispobj && s_layerWnd && s_layerWnd->getHWnd()) {
							RECT panelrect;
							GetWindowRect(s_layerWnd->getHWnd(), &panelrect);
							if ((cursorpos.x >= panelrect.left) && (cursorpos.x <= panelrect.right) &&
								(cursorpos.y >= panelrect.top) && (cursorpos.y <= panelrect.bottom)) {
								POINT cappoint;
								cappoint = cursorpos;
								::ScreenToClient(s_layerWnd->getHWnd(), &cappoint);
								LPARAM caplparam;
								caplparam = (cappoint.y << 16) | cappoint.x;
								::SendMessage(s_layerWnd->getHWnd(), WM_LBUTTONDOWN, MK_LBUTTON, caplparam);
								SetCapture(s_layerWnd->getHWnd());
								doneflag1 = true;
							}
						}
					}
					if (!doneflag1) {
						//aimbar
						HWND caphwnd;
						caphwnd = ::GetCapture();
						if (caphwnd && IsWindow(caphwnd)) {
							POINT cappoint;
							cappoint = cursorpos;
							::ScreenToClient(caphwnd, &cappoint);
							LPARAM caplparam;
							caplparam = (cappoint.y << 16) | cappoint.x;
							::SendMessage(caphwnd, WM_LBUTTONDOWN, MK_LBUTTON, caplparam);
						}
					}
				}
			}

		}
	}

	//g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);


	if (okbuttonup >= 1) {
		POINT cursorpos;
		::GetCursorPos(&cursorpos);
		LPARAM lparam;
		lparam = (cursorpos.y << 16) | cursorpos.x;

		//if (g_undertrackingRMenu == 0) {
		if (InterlockedAdd(&g_undertrackingRMenu, 0) == 0) {
			//dialog ctrl
			HWND ctrlwnd;
			ctrlwnd = GetOFWnd(cursorpos);
			if (ctrlwnd) {
				::SendMessage(ctrlwnd, WM_LBUTTONUP, 0, 0);
				if (s_ofhwnd) {
					POINT cappoint;
					cappoint = cursorpos;
					::ScreenToClient(ctrlwnd, &cappoint);
					LPARAM caplparam;
					caplparam = (cappoint.y << 16) | cappoint.x;

					//int ctrlid;
					//ctrlid = GetDlgCtrlID(ctrlwnd);
					//::SendMessage(s_ofhwnd, WM_COMMAND, ctrlid, 0);//WM_COMMANDはマウスのUPよりも後で呼ばないとUPでコマンドが終了してしまうことがある。


					WCHAR wclassname[MAX_PATH] = { 0L };
					::GetClassNameW(ctrlwnd, wclassname, MAX_PATH);
					//::DSMessageBox(s_anglelimitdlg, wclassname, L"check!!!", MB_OK);
					if (wcscmp(L"ComboBox", wclassname) == 0) {
					//	//COMBOBOX
					//	::SendMessage(ctrlwnd, CB_SHOWDROPDOWN, TRUE, 0);//DOWNのときに処理する
					}
					else if (wcscmp(L"msctls_trackbar32", wclassname) == 0) {
						//Slider
						RECT sliderloc;
						::GetWindowRect(ctrlwnd, &sliderloc);
						POINT cappoint2 = cursorpos;
						::ScreenToClient(ctrlwnd, &cappoint2);

						float rangemin;
						float rangemax;
						rangemin = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMIN, 0, 0);
						rangemax = (float)::SendMessage(ctrlwnd, TBM_GETRANGEMAX, 0, 0);

						float thumblength;
						thumblength = (float)::SendMessage(ctrlwnd, TBM_GETTHUMBLENGTH, 0, 0);

						//int sliderpos = (int)((float)(cappoint2.x - 12 - 274 / 2) / 274.0f * 360.0f);
						int sliderpos = (int)((float)(cappoint2.x - thumblength / 2.0f) / (float)(sliderloc.right - sliderloc.left - thumblength) * (float)(rangemax - rangemin) + rangemin);

						::SendMessage(ctrlwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)sliderpos);
						::SendMessage(s_ofhwnd, WM_HSCROLL, 0, (LPARAM)ctrlwnd);
					}
					else if (wcscmp(L"Button", wclassname) == 0) {
						POINT dlgpoint;
						dlgpoint = cursorpos;
						::ScreenToClient(s_ofhwnd, &dlgpoint);
						LPARAM dlglparam;
						dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
						::SendMessage(s_ofhwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam);

						POINT cappoint2;
						cappoint2 = cursorpos;
						::ScreenToClient(ctrlwnd, &cappoint2);
						LPARAM caplparam2;
						caplparam2 = (cappoint2.y << 16) | cappoint2.x;
						::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, caplparam2);

						//::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, caplparam);

						//int check1 = SendMessage(ctrlwnd, BM_GETCHECK, 0, 0);
						//if (check1 == 0) {
						//	SendMessage(ctrlwnd,
						//		BM_SETCHECK,
						//		BST_CHECKED,    // チェックをつける
						//		0);
						//}
						//else if (check1 == 1) {
						//	SendMessage(ctrlwnd,
						//		BM_SETCHECK,
						//		BST_UNCHECKED,    // チェックをはずす
						//		0);
						//}
					}
					else {
						POINT dlgpoint;
						dlgpoint = cursorpos;
						::ScreenToClient(s_ofhwnd, &dlgpoint);
						LPARAM dlglparam;
						dlglparam = (dlgpoint.y << 16) | dlgpoint.x;
						::SendMessage(s_ofhwnd, WM_LBUTTONUP, MK_LBUTTON, dlglparam);

						POINT cappoint2;
						cappoint2 = cursorpos;
						::ScreenToClient(ctrlwnd, &cappoint2);
						LPARAM caplparam2;
						caplparam2 = (cappoint2.y << 16) | cappoint2.x;
						::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, caplparam2);

						//::SendMessage(ctrlwnd, WM_LBUTTONUP, MK_LBUTTON, caplparam);
					}

					int ctrlid;
					ctrlid = GetDlgCtrlID(ctrlwnd);
					::SendMessage(s_ofhwnd, WM_COMMAND, ctrlid, 0);//WM_COMMANDはマウスのUPよりも後で呼ばないとUPでコマンドが終了してしまうことがある。

				}
			}
			else{

				bool doneflag1 = false;
				if (s_dispmodel && s_modelpanel.panel && s_modelpanel.panel->getHWnd()) {
					RECT panelrect;
					GetWindowRect(s_modelpanel.panel->getHWnd(), &panelrect);
					if ((cursorpos.x >= panelrect.left) && (cursorpos.x <= panelrect.right) &&
						(cursorpos.y >= panelrect.top) && (cursorpos.y <= panelrect.bottom)) {
						POINT cappoint;
						cappoint = cursorpos;
						::ScreenToClient(s_modelpanel.panel->getHWnd(), &cappoint);
						LPARAM caplparam;
						caplparam = (cappoint.y << 16) | cappoint.x;
						::SendMessage(s_modelpanel.panel->getHWnd(), WM_LBUTTONUP, MK_LBUTTON, caplparam);
						ReleaseCapture();
						doneflag1 = true;
					}
				}
				if (!doneflag1) {
					if (s_dispobj && s_layerWnd && s_layerWnd->getHWnd()) {
						RECT panelrect;
						GetWindowRect(s_layerWnd->getHWnd(), &panelrect);
						if ((cursorpos.x >= panelrect.left) && (cursorpos.x <= panelrect.right) &&
							(cursorpos.y >= panelrect.top) && (cursorpos.y <= panelrect.bottom)) {
							POINT cappoint;
							cappoint = cursorpos;
							::ScreenToClient(s_layerWnd->getHWnd(), &cappoint);
							LPARAM caplparam;
							caplparam = (cappoint.y << 16) | cappoint.x;
							::SendMessage(s_layerWnd->getHWnd(), WM_LBUTTONUP, MK_LBUTTON, caplparam);
							ReleaseCapture();
							doneflag1 = true;
						}
					}
				}
				if (!doneflag1) {
					//aim bar
					HWND caphwnd;
					caphwnd = ::GetCapture();
					if (caphwnd && IsWindow(caphwnd)) {
						//WPARAM wparam = (0xFFFF << 16) | (WORD)g_currentsubmenuid;//g_currentsubmenuid, curmenuitemid
						//::SendMessage(s_mainhwnd, WM_MENUSELECT, wparam, (LPARAM)GetMenu(s_mainhwnd));//GetMenu(s_mainhwnd), cursubmenu
						POINT cappoint;
						cappoint = cursorpos;
						::ScreenToClient(caphwnd, &cappoint);
						LPARAM caplparam;
						caplparam = (cappoint.y << 16) | cappoint.x;
						::SendMessage(caphwnd, WM_LBUTTONUP, MK_LBUTTON, caplparam);
					}

					//MainMenuAimBar
					if ((s_currentwndid == MB3D_WND_MAIN) && s_cursubmenu && (g_currentsubmenuid >= 0) && (g_currentsubmenuid < SPMENU_MAX)) {
						//SelectNextWindow(MB3D_WND_3D);//続いて　O button を押したときにメニューが開かないように。//プレート選択時に該当ウインドウをハイライトするようにしたので必要ない。
						InterlockedExchange(&g_undertrackingRMenu, (LONG)1);
						//SetForegroundWindow(s_mainhwnd);//この処理をしないと範囲外クリックでPopupが閉じない
						SetForegroundWindow(s_3dwnd);//この処理をしないと範囲外クリックでPopupが閉じない
						//int retmenuid = ::TrackPopupMenu(s_cursubmenu, TPM_RETURNCMD | TPM_LEFTALIGN, g_currentsubmenupos.x, g_currentsubmenupos.y, 0, s_mainhwnd, NULL);
						int retmenuid = ::TrackPopupMenu(s_cursubmenu, TPM_RETURNCMD | TPM_LEFTALIGN, g_currentsubmenupos.x, g_currentsubmenupos.y, 0, s_3dwnd, NULL);
						InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
					}
				}
			}

			//MainWindow MsgProc ; Prepair For Undo
			PrepairUndo();

			s_wmlbuttonup = 1;

		}

	}

}


/*
コード	ファイル	行	列
s_admpSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14612	14
s_akSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14614	12
s_boxzSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14607	14
s_btgscSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14610	15
s_btgSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14609	13
s_convboneWnd->setSize(WindowSize(450, 858));//880	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	9069	15
s_dmpanimASlider->setSize(WindowSize(slw2, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14329	18
s_dmpanimLSlider->setSize(WindowSize(slw2, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14328	18
s_dmpanimWnd->setSize(WindowSize(450, 858));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14379	14
s_fricSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14616	14
s_gfricSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15157	15
s_ghSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15153	12
s_gpWnd->setSize(WindowSize(450, 858));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15230	9
s_grestSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15156	15
s_gsizexSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15154	16
s_gsizezSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15155	16
s_impWnd->setSize(WindowSize(450, 858));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15109	10
s_impySlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15048	14
s_impzSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15047	14
s_ldmpSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14611	14
s_lkSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14613	12
s_massSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14608	14
s_modelpanel.panel->setSize(WindowSize(200, 100));//880	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	8883	20
s_placefolderWnd->setSize(WindowSize(450, 858));//880	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14489	18
s_restSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14615	14
s_rigidWnd->setSize(WindowSize(450, 858));//880	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	15004	12
s_sphrateSlider->setSize(WindowSize(slw, 40));	E:\PG\MameBake3D_git\MameBake3D\MameBake3D\Main.cpp	14606	17

WindowSize(400, 600)
*/

void ChangeMouseSetCapture()
{

	static bool s_firstflag = true;
	static int s_capwndid = -1;

	if ((s_dsutgui0.size() <= 0) || (s_dsutgui1.size() <= 0) || (s_dsutgui2.size() <= 0) || (s_dsutgui3.size() <= 0)) {
		return;
	}

	if (!s_mainhwnd) {
		return;
	}

	int dragbuttondown;
	int dragbuttonup;

	dragbuttondown = s_dsbuttondown[2];
	dragbuttonup = s_dsbuttonup[2];


	//〇ボタン押していないときには何もしないでリターン
	if (!dragbuttondown) {
		return;
	}


	POINT mousepoint;
	::GetCursorPos(&mousepoint);
	POINT clientpoint = mousepoint;
	::ScreenToClient(s_mainhwnd, &clientpoint);

	int chkx = clientpoint.x;
	int chky = clientpoint.y;


	//static RECT s_rcmainwnd;
	//static RECT s_rc3dwnd;
	//static RECT s_rctreewnd;
	//static RECT s_rctoolwnd;
	//static RECT s_rcltwnd;
	//static RECT s_rcsidemenuwnd;
	//static RECT s_rcrigidwnd;
	//static RECT s_rcinfownd;


	int nextcapwndid = -1;

	////check mainwnd
	{
		int wndtop = 0;
		int wndleft = 0;
		int wndbottom = s_rcmainwnd.bottom;
		int wndright = s_rcmainwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 0;
		}
	}

	//check 3dwnd
	{
		int wndtop = s_rc3dwnd.top;
		int wndleft = s_rctreewnd.right;
		int wndbottom = s_rc3dwnd.bottom;
		int wndright = wndleft + s_rc3dwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 1;
		}
	}

	//check treewnd
	{
		int wndtop = s_rctreewnd.top;
		int wndleft = 0;
		int wndbottom = s_rctreewnd.bottom;
		int wndright = s_rctreewnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 2;
		}
	}

	//check toolwnd
	{
		int wndtop = s_rctreewnd.bottom;
		int wndleft = 0;
		int wndbottom = s_rctreewnd.bottom + s_rctoolwnd.bottom;
		int wndright = s_rctoolwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 3;
		}
	}

	//check ltwnd
	{
		int wndtop = s_rctreewnd.bottom;
		int wndleft = s_rctoolwnd.right;
		int wndbottom = s_rctreewnd.bottom + s_rcltwnd.bottom;
		int wndright = s_rctoolwnd.right + s_rcltwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 4;
		}
	}

	//check sidemenuwnd
	{
		int wndtop = 0;
		int wndleft = s_rctreewnd.right + s_rc3dwnd.right;
		int wndbottom = s_rcsidemenuwnd.bottom;
		int wndright = s_rctreewnd.right + s_rc3dwnd.right + s_rcsidemenuwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 5;
		}
	}

	//check rigidwnd
	{
		int wndtop = s_rcsidemenuwnd.bottom;
		int wndleft = s_rctreewnd.right + s_rc3dwnd.right;
		int wndbottom = s_rcsidemenuwnd.bottom + s_rcrigidwnd.bottom;
		int wndright = s_rctreewnd.right + s_rc3dwnd.right + s_rcrigidwnd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 6;
		}
	}

	//check infownd
	{
		int wndtop = s_rc3dwnd.bottom;
		int wndleft = s_rctreewnd.right;
		int wndbottom = s_rc3dwnd.bottom + s_rcinfownd.bottom;
		int wndright = s_rctreewnd.right + s_rcinfownd.right;

		if ((chkx >= wndleft) && (chkx <= wndright) && (chky >= wndtop) && (chky <= wndbottom)) {
			nextcapwndid = 7;
		}
	}

	///////////////
	///////////////

	//if ((s_pickinfo.buttonflag == PICK_CAMMOVE) || (s_pickinfo.buttonflag == PICK_CAMROT) || (s_pickinfo.buttonflag == PICK_CAMDIST)) {
	//	//ここでは、カメラスプライトをドラッグ中はマウスキャプチャーをいじらない
	//}
	//if ((nextcapwndid != s_capwndid) || (s_wmlbuttonup == 1) || (s_dspushedOK == 0) || (g_undertrackingRMenu == 0)) {
	//else if ((nextcapwndid != s_capwndid) || (s_wmlbuttonup == 1) || (s_dspushedOK == 0)) {
	

	if (dragbuttondown >= 1) {//〇ボタンを押したときにキャプチャーオン

		//if ((nextcapwndid != s_capwndid) || (s_wmlbuttonup == 1)) {//常にではなく、〇ボタンを押したときだけ処理。同じウインドウでも必要なことがある。
			switch (nextcapwndid) {
			case 0:
				if (s_mainhwnd) {
					if (s_mainhwnd) {
						SetCapture(s_mainhwnd);
					}
				}
				break;

			case 1:
				if (s_3dwnd) {
					//SetCapture(s_3dwnd);
					HWND dlghwnd;
					dlghwnd = g_SampleUI.GetHWnd();
					if (dlghwnd) {
						SetCapture(dlghwnd);
					}
				}
				break;
			case 2:
				if (s_timelineWnd) {
					SetCapture(s_timelineWnd->getHWnd());
				}
				break;
			case 3:
				if (s_toolWnd) {
					SetCapture(s_toolWnd->getHWnd());
				}
				break;
			case 4:
				if (s_LtimelineWnd) {
					SetCapture(s_LtimelineWnd->getHWnd());
				}
				break;
			case 5:
				if (s_sidemenuWnd) {
					SetCapture(s_sidemenuWnd->getHWnd());
				}
				break;
			case 6:
				//plate menuで場合分け必要
				if (s_platemenukind == SPPLATEMENUKIND_GUI) {
					if (s_mainhwnd) {
						SetCapture(s_mainhwnd);
					}
				}
				else if (s_platemenukind == SPPLATEMENUKIND_RIGID) {
					if (s_platemenuno == (SPRIGIDTSW_RIGIDPARAMS + 1)) {
						if (s_rigidWnd) {
							SetCapture(s_rigidWnd->getHWnd());
						}
					}
					else if (s_platemenuno == (SPRIGIDSW_IMPULSE + 1)) {
						if (s_impWnd) {
							SetCapture(s_impWnd->getHWnd());
						}
					}
					else if (s_platemenuno == (SPRIGIDSW_GROUNDPLANE + 1)) {
						if (s_gpWnd) {
							SetCapture(s_gpWnd->getHWnd());
						}
					}
					else if (s_platemenuno == (SPRIGIDSW_DAMPANIM + 1)) {
						if (s_dmpanimWnd) {
							SetCapture(s_dmpanimWnd->getHWnd());
						}
					}
				}
				else if (s_platemenukind == SPPLATEMENUKIND_RETARGET) {
					if (s_platemenuno == (SPRETARGETSW_RETARGET + 1)) {
						if (s_convboneWnd) {
							SetCapture(s_convboneWnd->getHWnd());
						}
					}
					else if (s_platemenuno == (SPRETARGETSW_LIMITEULER + 1)) {
						if (s_anglelimitdlg) {
							SetCapture(s_anglelimitdlg);
						}
					}
				}
				break;
			case 7:
				if (g_infownd) {
					SetCapture(g_infownd->GetHWnd());
				}
				break;

			default:
				if (s_mainhwnd) {
					SetCapture(s_mainhwnd);
				}
				//if (s_mainhwnd) {
				//	if (!s_firstflag) {
				//		ReleaseCapture();
				//		s_firstflag = false;
				//	}
				//	SetCapture(s_mainhwnd);
				//}
				break;
			}

			s_capwndid = nextcapwndid;
			s_wmlbuttonup = 0;
		//}
	}
}


void ChangeMouseReleaseCapture()
{
	if (!s_mainhwnd) {
		return;
	}

	int dragbuttondown;
	int dragbuttonup;

	dragbuttondown = s_dsbuttondown[2];
	dragbuttonup = s_dsbuttonup[2];


	//〇ボタンを話した時にリリースキャプチャしてリターン
	if (dragbuttonup >= 1) {
		ReleaseCapture();
		return;
	}
}

void OrgWindowListenMouse(bool srcflag)
{
	if (s_timelineWnd) {
		s_timelineWnd->setListenMouse(srcflag);
	}
	if (s_LtimelineWnd) {
		s_LtimelineWnd->setListenMouse(srcflag);
	}
	if (s_dmpanimWnd) {
		s_dmpanimWnd->setListenMouse(srcflag);
	}
	if (s_sidemenuWnd) {
		s_sidemenuWnd->setListenMouse(srcflag);
	}
	if (s_mainmenuaimbarWnd) {
		s_mainmenuaimbarWnd->setListenMouse(srcflag);
	}
	if (s_placefolderWnd) {
		s_placefolderWnd->setListenMouse(srcflag);
	}
	if (s_rigidWnd) {
		s_rigidWnd->setListenMouse(srcflag);
	}
	if (s_impWnd) {
		s_impWnd->setListenMouse(srcflag);
	}
	if (s_gpWnd) {
		s_gpWnd->setListenMouse(srcflag);
	}
	if (s_toolWnd) {
		s_toolWnd->setListenMouse(srcflag);
	}
	if (s_convboneWnd) {
		s_convboneWnd->setListenMouse(srcflag);
	}
	if (s_layerWnd) {
		s_layerWnd->setListenMouse(srcflag);
	}
	if (s_convboneWnd) {
		s_convboneWnd->setListenMouse(srcflag);
	}

	//anglelimitdlgはWindowsDialog

}

void DSMessageBox(HWND srcparenthwnd, WCHAR* srcmessage, WCHAR* srctitle, LONG srcok)
{
	if (!srcmessage || !srctitle) {
		return;
	}

	_ASSERT(srcok == MB_OK);

	int cmperror;
	int cmpwarning;
	int cmpcheck;
	cmperror = wcscmp(srctitle, L"error!!!");
	cmpwarning = wcscmp(srctitle, L"warning!!!");
	cmpcheck = wcscmp(srctitle, L"check!!!");
	_ASSERT((cmperror == 0) || (cmpwarning == 0) || (cmpcheck == 0));


	s_messageboxhwnd = 0;
	s_messageboxpushcnt = 0;
	HWINEVENTHOOK hhook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0,
		WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	InterlockedExchange(&g_undertrackingRMenu, (LONG)1);

	::MessageBoxW(srcparenthwnd, srcmessage, srctitle, srcok);

	InterlockedExchange(&g_undertrackingRMenu, (LONG)0);
	UnhookWinEvent(hhook);
	s_messageboxhwnd = 0;
	s_messageboxpushcnt = 0;
}

void SetMainWindowTitle()
{
	if (!s_mainhwnd) {
		return;
	}


	//"まめばけ３D (MameBake3D)"
	WCHAR strmaintitle[MAX_PATH * 3] = { 0L };
	swprintf_s(strmaintitle, MAX_PATH * 3, L"MotionBrushFree Ver1.0.0.25 : No.%d : ", s_appcnt);


	if (s_model) {
		//WCHAR strcharactor[MAX_PATH * 3] = { 0L };
		WCHAR strindexedcharactor[MAX_PATH * 3] = { 0L };
		char strmotionA[MAX_PATH * 3] = { 0 };
		WCHAR strmotionW[MAX_PATH * 3] = { 0 };
		WCHAR strrefW[MAX_PATH * 3] = { 0 };

		int modelnum = (int)s_modelindex.size();

		if ((modelnum >= 1) && (s_curmodelmenuindex >= 0) && (s_curmodelmenuindex < modelnum)) {
			CModel* curmodel;
			curmodel = s_modelindex[s_curmodelmenuindex].modelptr;
			if (curmodel) {
				swprintf_s(strindexedcharactor, MAX_PATH * 3, L"%d : %s", s_curmodelmenuindex, curmodel->GetFileName());
				wcscat_s(strmaintitle, (MAX_PATH * 3), strindexedcharactor);
				wcscat_s(strmaintitle, (MAX_PATH * 3), L" : ");

				MOTINFO* curmi;
				curmi = s_model->GetCurMotInfo();
				if (curmi) {
					strcpy_s(strmotionA, (MAX_PATH * 3), curmi->motname);
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strmotionA, (MAX_PATH * 3), strmotionW, (MAX_PATH * 3));
					wcscat_s(strmaintitle, (MAX_PATH * 3), strmotionW);
					wcscat_s(strmaintitle, (MAX_PATH * 3), L" : ");
				}

				int retrefindex = -1;
				REINFO reinfo;
				reinfo = s_model->GetCurrentRigidElemInfo(&retrefindex);
				if (retrefindex >= 0) {
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinfo.filename, (MAX_PATH * 3), strrefW, (MAX_PATH * 3));
					wcscat_s(strmaintitle, (MAX_PATH * 3), strrefW);
				}
				else {
					wcscat_s(strmaintitle, (MAX_PATH * 3), L"reffile unknown");
				}
			}
		}
	}

	SetWindowText(s_mainhwnd, strmaintitle);

}

void SetKinematicToHand(CModel* srcmodel, bool srcflag)
{
	if (!srcmodel) {
		return;
	}
	if (!srcmodel->GetTopBone()) {
		return;
	}

	SetKinematicToHandReq(srcmodel, srcmodel->GetTopBone(), srcflag);
}


void SetKinematicToHandReq(CModel* srcmodel, CBone* srcbone, bool srcflag)
{
	if (!srcmodel) {
		return;
	}
	if (!srcbone) {
		return;
	}

	const char* pbonename = (const char*)srcbone->GetBoneName();
	const char* phandpat1 = strstr(pbonename, "Elbow_branch");
	const char* phandpat2 = strstr(pbonename, "Hand");

	if (phandpat1 || phandpat2) {
		srcmodel->SetKinematicTmpLower(srcbone, srcflag);
	}


	if (srcbone->GetChild()) {
		SetKinematicToHandReq(srcmodel, srcbone->GetChild(), srcflag);
	}
	if(srcbone->GetBrother()){
		SetKinematicToHandReq(srcmodel, srcbone->GetBrother(), srcflag);
	}

}

void OnGUIEventSpeed()
{
	int modelnum;
	modelnum = (int)s_modelindex.size();
	int modelno;

	RollbackCurBoneNo();
	g_dspeed = (float)((double)g_SampleUI.GetSlider(IDC_SPEED)->GetValue() * 0.010);
	for (modelno = 0; modelno < modelnum; modelno++) {
		s_modelindex[modelno].modelptr->SetMotionSpeed(g_dspeed);
	}

	WCHAR sz[100] = { 0L };
	swprintf_s(sz, 100, L"Speed: %0.4f", g_dspeed);
	g_SampleUI.GetStatic(IDC_SPEED_STATIC)->SetText(sz);
}

void WaitRetargetThreads()
{
	//if ((g_retargetbatchflag == 2) || (g_retargetbatchflag == 3)) {//2はダイアログでのキャンセル
	if ((InterlockedAdd(&g_retargetbatchflag, 0) == 2) || (InterlockedAdd(&g_retargetbatchflag, 0) == 3)) {//2はダイアログでのキャンセル
		InterlockedExchange(&g_retargetbatchflag, (LONG)0);
		if (s_retargetbatchwnd) {
			SendMessage(s_retargetbatchwnd, WM_CLOSE, 0, 0);
		}
		s_retargetcnt = 0;
		InterlockedExchange(&g_retargetbatchflag, (LONG)0);//WM_CLOSEで変わる可能性あり

		g_limitdegflag = s_savelimitdegflag;
		if (s_LimitDegCheckBox) {
			s_LimitDegCheckBox->SetChecked(g_limitdegflag);
		}

		OnModelMenu(true, s_saveretargetmodel, 1);
	}

}

//void WaitMotionCacheThreads()
//{
//	//if ((g_motioncachebatchflag == 2) || (g_motioncachebatchflag == 3)) {//2はダイアログでのキャンセル
//	if ((InterlockedAdd(&g_motioncachebatchflag, 0) == 2) || (InterlockedAdd(&g_motioncachebatchflag, 0) == 3)) {//2はダイアログでのキャンセル
//		InterlockedExchange(&g_motioncachebatchflag, (LONG)0);
//		if (s_motioncachebatchwnd) {
//			SendMessage(s_motioncachebatchwnd, WM_CLOSE, 0, 0);
//		}
//		s_motioncachecnt = 0;
//		InterlockedExchange(&g_motioncachebatchflag, (LONG)0);//WM_CLOSEで変わる可能性あり
//	}
//
//}

//void WaitBvh2FbxThreads()
//{
//	//if ((g_bvh2fbxbatchflag == 2) || (g_bvh2fbxbatchflag == 3)) {//2はダイアログでのキャンセル
//	if ((InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 2) || (InterlockedAdd(&g_bvh2fbxbatchflag, 0) == 3)) {//2はダイアログでのキャンセル
//		InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)0);
//		if (s_bvh2fbxbatchwnd) {
//			SendMessage(s_bvh2fbxbatchwnd, WM_CLOSE, 0, 0);
//		}
//		s_bvh2fbxcnt = 0;
//		InterlockedExchange(&g_bvh2fbxbatchflag, (LONG)0);//WM_CLOSEで変わる可能性あり
//	}
//}

int Savebvh2FBXHistory(WCHAR* selectname)
{
	WCHAR saveprojpath[MAX_PATH] = { 0L };
	wcscpy_s(saveprojpath, MAX_PATH, selectname);


	//書き込み処理が成功してから履歴を保存する。chaファイルだけ。
	int savepathlen;
	saveprojpath[MAX_PATH - 1] = 0L;
	savepathlen = (int)wcslen(saveprojpath);
	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	WCHAR HistoryForOpeningProjectWithGamePad[MAX_PATH] = { 0L };
	swprintf_s(HistoryForOpeningProjectWithGamePad, MAX_PATH, L"%s\\MB3DOpenProjBvhDir_%04u%02u%02u%02u%02u%02u.txt",
		s_temppath,
		localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
	HANDLE hfile;
	hfile = CreateFile(HistoryForOpeningProjectWithGamePad, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		//int pathlen;
		//pathlen = (int)wcslen(saveprojpath);
		if ((savepathlen > 0) && (savepathlen < MAX_PATH)) {
			DWORD writelen = 0;
			WriteFile(hfile, saveprojpath, (savepathlen * sizeof(WCHAR)), &writelen, NULL);
			_ASSERT((savepathlen * sizeof(WCHAR)) == writelen);
		}
		CloseHandle(hfile);
	}

	return 0;
}


int SaveBatchHistory(WCHAR* selectname)
{
	WCHAR saveprojpath[MAX_PATH] = { 0L };
	wcscpy_s(saveprojpath, MAX_PATH, selectname);


	//書き込み処理が成功してから履歴を保存する。chaファイルだけ。
	int savepathlen;
	saveprojpath[MAX_PATH - 1] = 0L;
	savepathlen = (int)wcslen(saveprojpath);
	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	WCHAR HistoryForOpeningProjectWithGamePad[MAX_PATH] = { 0L };
	swprintf_s(HistoryForOpeningProjectWithGamePad, MAX_PATH, L"%s\\MB3DOpenProjBatchDir_%04u%02u%02u%02u%02u%02u.txt",
		s_temppath,
		localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
	HANDLE hfile;
	hfile = CreateFile(HistoryForOpeningProjectWithGamePad, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		//int pathlen;
		//pathlen = (int)wcslen(saveprojpath);
		if ((savepathlen > 0) && (savepathlen < MAX_PATH)) {
			DWORD writelen = 0;
			WriteFile(hfile, saveprojpath, (savepathlen * sizeof(WCHAR)), &writelen, NULL);
			_ASSERT((savepathlen * sizeof(WCHAR)) == writelen);
		}
		CloseHandle(hfile);
	}

	return 0;
}


int GetbvhHistoryDir(std::vector<wstring>& dstvecopenfilename)
{

	dstvecopenfilename.clear();
	//ZeroMemory(dstname, sizeof(WCHAR) * dstlen);


	//MB3DOpenProj_20210410215628.txt
	WCHAR searchfilename[MAX_PATH] = { 0L };
	swprintf_s(searchfilename, MAX_PATH, L"%sMB3DOpenProjBvhDir_*.txt", s_temppath);
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	hFind = FindFirstFileW(searchfilename, &win32fd);

	std::vector<HISTORYELEM> vechistory;//!!!!!!!!! tmpファイル名
	std::vector<wstring> vecopenfilename;//!!!!!!!! tmpファイル内に書いてあるopenfilename

	vechistory.clear();
	bool notfoundfirst = true;
	if (hFind != INVALID_HANDLE_VALUE) {
		notfoundfirst = false;
		do {
			if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				HISTORYELEM curelem;
				curelem.filetime = win32fd.ftCreationTime;

				//printf("%s\n", win32fd.cFileName);
				curelem.wfilename[MAX_PATH - 1] = { 0L };
				swprintf_s(curelem.wfilename, MAX_PATH, L"%s%s", s_temppath, win32fd.cFileName);

				vechistory.push_back(curelem);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}

	if (!vechistory.empty()) {

		std::sort(vechistory.begin(), vechistory.end());
		std::reverse(vechistory.begin(), vechistory.end());

		int numhistory = (int)vechistory.size();
		int dispnum = min(OPENHISTORYMAXNUM, numhistory);

		int foundnum = 0;
		int historyno;
		for (historyno = 0; historyno < numhistory; historyno++) {
			WCHAR openfilename[MAX_PATH] = { 0L };
			wcscpy_s(openfilename, MAX_PATH, vechistory[historyno].wfilename);

			HANDLE hfile;
			hfile = CreateFileW(openfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				WCHAR readwstr[MAX_PATH] = { 0L };
				DWORD readleng = 0;
				bool bsuccess;
				bsuccess = ReadFile(hfile, readwstr, (MAX_PATH * sizeof(WCHAR)), &readleng, NULL);
				if (bsuccess) {
					bool foundsame = false;
					wstring newwstr = readwstr;
					std::vector<wstring>::iterator itropenfilename;
					for (itropenfilename = vecopenfilename.begin(); itropenfilename != vecopenfilename.end(); itropenfilename++) {
						if (newwstr.compare(*itropenfilename) == 0) {
							foundsame = true;
						}
					}
					if (foundsame == false) {
						vecopenfilename.push_back(readwstr);
						foundnum++;
						if (foundnum >= dispnum) {
							break;
						}
					}
				}
				CloseHandle(hfile);
			}
		}
	}

	if (!vecopenfilename.empty()) {
		dstvecopenfilename = vecopenfilename;
	}
	else {
		dstvecopenfilename.clear();
	}
	return 0;
}

int GetCPTFileName(std::vector<HISTORYELEM>& dstvecopenfilename)
{

	dstvecopenfilename.clear();
	//ZeroMemory(dstname, sizeof(WCHAR) * dstlen);


	//MB3DOpenProj_20210410215628.txt
	WCHAR searchfilename[MAX_PATH] = { 0L };
	swprintf_s(searchfilename, MAX_PATH, L"%sMB3DTempCopyFrames_v1.0.0.18_*.cpt", s_temppath);
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	hFind = FindFirstFileW(searchfilename, &win32fd);

	std::vector<HISTORYELEM> vechistory;//!!!!!!!!! tmpファイル名
	//std::vector<wstring> vecopenfilename;//!!!!!!!! tmpファイル内に書いてあるopenfilename

	vechistory.clear();
	bool notfoundfirst = true;
	if (hFind != INVALID_HANDLE_VALUE) {
		notfoundfirst = false;
		do {
			if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				HISTORYELEM curelem;
				curelem.filetime = win32fd.ftCreationTime;

				//printf("%s\n", win32fd.cFileName);
				curelem.wfilename[MAX_PATH - 1] = { 0L };
				swprintf_s(curelem.wfilename, MAX_PATH, L"%s%s", s_temppath, win32fd.cFileName);

				vechistory.push_back(curelem);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}

	if (!vechistory.empty()) {

		std::sort(vechistory.begin(), vechistory.end());
		std::reverse(vechistory.begin(), vechistory.end());

		std::vector<HISTORYELEM>::iterator itrhistoryelem;
		for (itrhistoryelem = vechistory.begin(); itrhistoryelem != vechistory.end(); itrhistoryelem++) {
			HISTORYELEM curelem = *itrhistoryelem;
			int result = LoadCPIFile(&curelem);
			_ASSERT(result == 0);
			*itrhistoryelem = curelem;//失敗した時にはnewelem.hascpinfo = 0がセットされている
		}
	}

	if (!vechistory.empty()) {
		dstvecopenfilename = vechistory;
	}

	//if (!vecopenfilename.empty()) {
	//	dstvecopenfilename = vecopenfilename;
	//}
	//else {
	//	dstvecopenfilename.clear();
	//}
	return 0;
}

int GetBatchHistoryDir(WCHAR* dstname, int dstlen)
{
	
	ZeroMemory(dstname, sizeof(WCHAR) * dstlen);


	//MB3DOpenProj_20210410215628.txt
	WCHAR searchfilename[MAX_PATH] = { 0L };
	swprintf_s(searchfilename, MAX_PATH, L"%sMB3DOpenProjBatchDir_*.txt", s_temppath);
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	hFind = FindFirstFileW(searchfilename, &win32fd);

	std::vector<HISTORYELEM> vechistory;//!!!!!!!!! tmpファイル名
	std::vector<wstring> vecopenfilename;//!!!!!!!! tmpファイル内に書いてあるopenfilename

	vechistory.clear();
	bool notfoundfirst = true;
	if (hFind != INVALID_HANDLE_VALUE) {
		notfoundfirst = false;
		do {
			if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				HISTORYELEM curelem;
				curelem.filetime = win32fd.ftCreationTime;

				//printf("%s\n", win32fd.cFileName);
				curelem.wfilename[MAX_PATH - 1] = { 0L };
				swprintf_s(curelem.wfilename, MAX_PATH, L"%s%s", s_temppath, win32fd.cFileName);

				vechistory.push_back(curelem);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}

	if (!vechistory.empty()) {

		std::sort(vechistory.begin(), vechistory.end());
		std::reverse(vechistory.begin(), vechistory.end());

		int numhistory = (int)vechistory.size();
		int dispnum = min(OPENHISTORYMAXNUM, numhistory);

		int foundnum = 0;
		int historyno;
		for (historyno = 0; historyno < numhistory; historyno++) {
			WCHAR openfilename[MAX_PATH] = { 0L };
			wcscpy_s(openfilename, MAX_PATH, vechistory[historyno].wfilename);

			HANDLE hfile;
			hfile = CreateFileW(openfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				WCHAR readwstr[MAX_PATH] = { 0L };
				DWORD readleng = 0;
				bool bsuccess;
				bsuccess = ReadFile(hfile, readwstr, (MAX_PATH * sizeof(WCHAR)), &readleng, NULL);
				if (bsuccess) {
					bool foundsame = false;
					wstring newwstr = readwstr;
					std::vector<wstring>::iterator itropenfilename;
					for (itropenfilename = vecopenfilename.begin(); itropenfilename != vecopenfilename.end(); itropenfilename++) {
						if (newwstr.compare(*itropenfilename) == 0) {
							foundsame = true;
						}
					}
					if (foundsame == false) {
						vecopenfilename.push_back(readwstr);
						foundnum++;
						if (foundnum >= dispnum) {
							break;
						}
					}
				}
				CloseHandle(hfile);
			}
		}
	}

	if (!vecopenfilename.empty()) {
		wcscpy_s(dstname, dstlen, vecopenfilename[0].c_str());
	}
	else {
		*dstname = 0L;
	}

	return 0;
}


//int CopyToClipBoardData(int shdnum, int cpframenum, COPYELEM* cpELem, HGLOBAL* cbhgptr)
//{
//	int ret = 0;
//	HGLOBAL hGlobal = 0;
//	//int datasize;
//	int headerlen, shdnumlen, framenumlen, celen;
//	int dstcharsize;
//
//	headerlen = (int)strlen(clipboardheader2);
//	shdnumlen = sizeof(int);
//	framenumlen = sizeof(int);
//	celen = sizeof(COPYELEM) * shdnum * cpframenum;
//	//datasize = headerlen + numlen + celen;
//	dstcharsize = headerlen + (shdnumlen * 2) + (framenumlen * 2) + (celen * 2) + 1;// char以外の1byte --> 2文字、と終端ＮＵＬＬ
//
//
//	hGlobal = GlobalAlloc(GHND, dstcharsize);
//	if (hGlobal == NULL) {
//		DbgOut("motparamdlg : CopyToClipBoardData : GlobalAlloc error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//
//	char* dstptr;
//	dstptr = (char*)GlobalLock(hGlobal);
//	if (!dstptr) {
//		DbgOut("motparamdlg : CopyToClipBoardData : GlobalLock error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//
//	int dstpos = 0;
//	strcpy_s(dstptr, dstcharsize - dstpos, clipboardheader2);
//	dstpos += headerlen;
//
//	ret = Bin2Char((unsigned char*)&shdnum, dstptr + dstpos, shdnumlen);
//	if (ret) {
//		DbgOut("motparamdlg : CopyToClipBoardData : Bin2Char shdnum error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//	dstpos += shdnumlen * 2;
//
//
//	ret = Bin2Char((unsigned char*)&cpframenum, dstptr + dstpos, framenumlen);
//	if (ret) {
//		DbgOut("motparamdlg : CopyToClipBoardData : Bin2Char cpframenum error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//	dstpos += framenumlen * 2;
//
//
//	ret = Bin2Char((unsigned char*)srcce, dstptr + dstpos, celen);
//	if (ret) {
//		DbgOut("motparamdlg : CopyToClipBoardData : Bin2Char ce error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//	dstpos += celen * 2;
//
//	*(dstptr + dstpos) = 0;//!!!!!
//
//	if (dstpos > dstcharsize) {
//		DbgOut("motparamdlg : CopyToClipBoardData : dstpos error !!!\n");
//		_ASSERT(0);
//		ret = 1;
//		goto cptocbexit;
//	}
//
//
//	goto cptocbexit;
//
//cptocbexit:
//	if (hGlobal)
//		GlobalUnlock(hGlobal);
//	if (ret && hGlobal)
//		GlobalFree(hGlobal);
//
//	*cbhgptr = hGlobal;
//	return ret;
//}
//
//int Bin2Char(unsigned char* srcptr, char* dstptr, int srclen)
//{
//	int srcpos;
//	int dstpos = 0;
//	//int ret;
//	unsigned char curuc;
//	unsigned char lowuc, hiuc;
//	char lowc, hic;
//	for (srcpos = 0; srcpos < srclen; srcpos++) {
//		curuc = *(srcptr + srcpos);
//
//		hiuc = curuc >> 4;
//		if (hiuc <= 9) {
//			hic = '0' + hiuc;
//		}
//		else {
//			hic = 'A' + hiuc - 10;
//		}
//
//		lowuc = curuc & 0x0F;
//		if (lowuc <= 9) {
//			lowc = '0' + lowuc;
//		}
//		else {
//			lowc = 'A' + lowuc - 10;
//		}
//
//		*(dstptr + dstpos) = hic;
//		*(dstptr + dstpos + 1) = lowc;
//		dstpos += 2;
//	}
//
//	return 0;
//}
//
//
//
//int PasteFromClipBoardData(COPYELEM* dstce, int* bufshdnum, int* bufframenum, HGLOBAL cbhg)
//{
//	*bufshdnum = 0;
//	*bufframenum = 0;
//
//	char* srcptr;
//
//	srcptr = (char*)GlobalLock(cbhg);
//	if (!srcptr) {
//		DbgOut("motparamdlg : PasteFromClipBoardData : GlobalLock error !!!\n");
//		_ASSERT(0);
//		return 1;
//	}
//
//	int headerleng;
//	headerleng = (int)strlen(clipboardheader2);
//
//	int srcleng;
//	srcleng = (int)strlen(srcptr);
//	int minimumleng = headerleng + sizeof(int) * 2 + sizeof(int) * 2;
//	if (srcleng <= minimumleng) {
//		DbgOut("motparamdlg : PasteFromClipBoardData : srcleng 0 error !!!\n");
//		::MessageBox(m_hWnd, "クリップボードにデータがありませんでした。", "ペースト失敗", MB_OK);
//		_ASSERT(0);
//		GlobalUnlock(cbhg);
//		return 1;
//	}
//
//	int srcpos = 0;
//	int cmp0;
//	cmp0 = strncmp(srcptr, clipboardheader2, headerleng);
//	if (cmp0) {
//		DbgOut("motparamdlg : PasteFromClipBoardData : invalid data type error !!!\n");
//		::MessageBox(m_hWnd, "クリップボードのデータの種類が違います。", "ペースト失敗", MB_OK);
//		_ASSERT(0);
//		GlobalUnlock(cbhg);
//		return 1;
//	}
//	srcpos += headerleng;
//
//
//	int ret;
//	int shdnum;
//	ret = Char2Hex(srcptr + srcpos, (unsigned char*)&shdnum, sizeof(int) * 2);
//	if (ret) {
//		DbgOut("motparamdlg : PasteFromClipBoard : Char2Hex shdnum error !!!\n");
//		::MessageBox(m_hWnd, "クリップボードのデータが不正です。", "ペースト失敗", MB_OK);
//		_ASSERT(0);
//		GlobalUnlock(cbhg);
//		return 1;
//	}
//	srcpos += (sizeof(int) * 2);
//
//	int framenum;
//	ret = Char2Hex(srcptr + srcpos, (unsigned char*)&framenum, sizeof(int) * 2);
//	if (ret || (framenum <= 0) || (framenum > CPFRAMEMAX)) {
//		DbgOut("motparamdlg : PasteFromClipBoard : Char2Hex framenum error !!!\n");
//		::MessageBox(m_hWnd, "クリップボードのデータが不正です。", "ペースト失敗", MB_OK);
//		_ASSERT(0);
//		GlobalUnlock(cbhg);
//		return 1;
//	}
//	srcpos += (sizeof(int) * 2);
//
//	int cesize;
//	cesize = shdnum * sizeof(COPYELEM) * framenum;
//	int sizeondata;
//	sizeondata = srcleng - headerleng - (sizeof(int) * 2) - (sizeof(int) * 2);
//	if (sizeondata != (cesize * 2)) {
//		DbgOut("motparamdlg : PasteFromClipBoard : data size error !!!\n");
//		::MessageBox(m_hWnd, "クリップボードのデータ長が不正です。", "ペースト失敗", MB_OK);
//		_ASSERT(0);
//		GlobalUnlock(cbhg);
//		return 1;
//	}
//
//
//	InitCEBuff();//!!!!!!!!!!!!
//
//	int framecnt;
//	int ceno;
//	COPYELEM tempce;
//	for (framecnt = 0; framecnt < framenum; framecnt++) {
//		for (ceno = 0; ceno < shdnum; ceno++) {
//			ret = Char2Hex(srcptr + srcpos, (unsigned char*)&tempce, sizeof(COPYELEM) * 2);
//			if (ret) {
//				DbgOut("motparamdlg : PasteFromClipBoard : Char2Hex ce %d %d error !!!\n", ceno, shdnum);
//				::MessageBox(m_hWnd, "データ変換中にエラーが生じました。", "ペースト失敗", MB_OK);
//				_ASSERT(0);
//				GlobalUnlock(cbhg);
//				return 1;
//			}
//			srcpos += (sizeof(COPYELEM) * 2);
//
//			tempce.mp.m_spp = 0;//!!!!!!!!!!
//
//			if (tempce.mp.m_frameno >= 0) {
//
//				//tempceの名前に対応するserialnoを探し、そのmpにデータをコピーする。
//				int chkleng;
//				chkleng = (int)strlen(tempce.name);
//				if (chkleng >= 256) {
//					DbgOut("motparamdlg : PasteFromClipBoard : check tempce name error %d !!!\n", ceno);
//					::MessageBox(m_hWnd, "不正な名前が見つかりました。１", "ペースト失敗", MB_OK);
//					_ASSERT(0);
//					GlobalUnlock(cbhg);
//					return 1;
//				}
//
//				int findno;
//				//ret = m_thandler->GetPartNoByName( tempce.name, &findno );
//				ret = m_thandler->GetBoneNoByName(tempce.name, &findno, m_shandler, 0);
//				if (ret) {
//					DbgOut("motparamdlg : PasteFromClipBoard : GetPartNoByName error !!!\n");
//					::MessageBox(m_hWnd, "不正な名前が見つかりました。２", "ペースト失敗", MB_OK);
//					_ASSERT(0);
//					GlobalUnlock(cbhg);
//					return 1;
//				}
//
//				if (findno > 0) {
//					*(m_CEBuff + shdnum * framecnt + findno) = tempce;//!!!!!!!!
//				}
//			}
//		}
//	}
//
//	GlobalUnlock(cbhg);
//
//	*bufshdnum = shdnum;
//	*bufframenum = framenum;
//
//	return 0;
//}
//
//
//int Char2Hex(char* orgdata, unsigned char* donedata, DWORD charleng)
//{
//	int ret;
//	DWORD charno;
//	DWORD rest;
//
//	unsigned char ucval;
//	unsigned char* doneptr = 0;
//
//
//	rest = charleng % 2;
//	if (rest) {
//		_ASSERT(0);
//		return 1;
//	}
//
//	doneptr = donedata;
//	for (charno = 0; charno < charleng; charno += 2) {
//		ret = ConvData(orgdata + charno, &ucval, 0);
//		_ASSERT(!ret);
//
//		ret = ConvData(orgdata + charno + 1, &ucval, 1);
//		_ASSERT(!ret);
//
//		*doneptr = ucval;
//		doneptr++;
//	}
//
//	return 0;
//}
//
//int ConvData(char* charptr, unsigned char* ucptr, int pos)
//{
//	char charval;
//	unsigned char convval;
//
//	charval = *charptr;
//	if ((charval >= '0') && (charval <= '9')) {
//		convval = charval - '0';
//	}
//	else if ((charval >= 'A') && (charval <= 'F')) {
//		convval = charval - 'A' + 10;
//	}
//	else {
//		_ASSERT(0);
//		return 1;
//	}
//
//	if (pos == 0) {
//		*ucptr = convval << 4;
//	}
//	else {
//		*ucptr |= convval;
//	}
//
//	return 0;
//}


int WriteCPTFile(WCHAR* dstfilename)
{
	int cpelemnum;
	cpelemnum = (int)s_copymotvec.size();
	if (cpelemnum <= 0) {
		return 0;
	}
	if (!s_model) {
		return 0;
	}
	if (!s_model->GetCurMotInfo()) {
		return 0;
	}

	if (!dstfilename) {
		return 0;
	}

	*dstfilename = 0L;

	SYSTEMTIME localtime;
	GetLocalTime(&localtime);
	WCHAR cptfilename[MAX_PATH] = { 0L };
	swprintf_s(cptfilename, MAX_PATH, L"%s\\MB3DTempCopyFrames_v1.0.0.18_%04u%02u%02u%02u%02u%02u.cpt",
		s_temppath,
		localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);

	HANDLE hfile = CreateFile(cptfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		DbgOut(L"CPTFile : WriteCPTFile : file open error !!!\n");
		_ASSERT(0);
		return 1;
	}

	wcscpy_s(dstfilename, MAX_PATH, cptfilename);
	*(dstfilename + MAX_PATH - 1) = 0L;


	char CPTheader[256];
	::ZeroMemory(CPTheader, sizeof(char) * 256);
	strcpy_s(CPTheader, 256, "MB3DTempCopyFramesFile ver1.0.0.18");//本体ではない

	DWORD wleng = 0;
	WriteFile(hfile, CPTheader, sizeof(char) * 256, &wleng, NULL);
	if (wleng != (sizeof(char) * 256)) {
		_ASSERT(0);
		return 1;
	}


	wleng = 0;
	WriteFile(hfile, &cpelemnum, sizeof(int), &wleng, NULL);
	if (wleng != sizeof(int)) {
		_ASSERT(0);
		return 1;
	}



	int cpelemno;
	for (cpelemno = 0; cpelemno < cpelemnum; cpelemno++) {
		CPELEM2 curcpelem = s_copymotvec[cpelemno];
		if (curcpelem.bone) {
			char curbonename[MAX_PATH] = { 0 };
			strcpy_s(curbonename, MAX_PATH, curcpelem.bone->GetBoneName());

			wleng = 0;
			WriteFile(hfile, curbonename, sizeof(char) * MAX_PATH, &wleng, NULL);
			if (wleng != (sizeof(char) * MAX_PATH)) {
				_ASSERT(0);
				return 1;
			}
			
			double curframe;
			curframe = curcpelem.mp.GetFrame();
			wleng = 0;
			WriteFile(hfile, &curframe, sizeof(double), &wleng, NULL);
			if (wleng != (sizeof(double))) {
				_ASSERT(0);
				return 1;
			}

			ChaMatrix curlocalmat;
			curlocalmat = curcpelem.mp.GetWorldMat();
			wleng = 0;
			WriteFile(hfile, &curlocalmat, sizeof(ChaMatrix), &wleng, NULL);
			if (wleng != (sizeof(ChaMatrix))) {
				_ASSERT(0);
				return 1;
			}

			int localmatflag;
			localmatflag = curcpelem.mp.GetLocalMatFlag();
			wleng = 0;
			WriteFile(hfile, &localmatflag, sizeof(int), &wleng, NULL);
			if (wleng != (sizeof(int))) {
				_ASSERT(0);
				return 1;
			}
		}
/*
	cpelem.bone = curbone;
	cpelem.mp.SetFrame(curframe);
	cpelem.mp.SetWorldMat(localmat);
	cpelem.mp.SetLocalMatFlag(1);//!!!!!!!!!!
*/

	}

	FlushFileBuffers(hfile);
	SetEndOfFile(hfile);
	CloseHandle(hfile);


	return 0;
}

int WriteCPIFile(WCHAR* srccptfilename)
{


	int cpelemnum;
	cpelemnum = (int)s_copymotvec.size();
	if (cpelemnum <= 0) {
		return 0;
	}
	if (!s_model) {
		return 0;
	}
	if (!s_model->GetCurMotInfo()) {
		return 0;
	}
	if (!srccptfilename) {
		return 0;
	}

	CPMOTINFO cpinfo;
	ZeroMemory(&cpinfo, sizeof(CPMOTINFO));
	/*
		typedef struct tag_cpinfo
		{
			WCHAR fbxname[MAX_PATH];
			WCHAR motionname[MAX_PATH];
			double startframe;
			double framenum;
			int bvhtype;//0:undef, 1-144:bvh1 - bvh144, -1:bvh_other
			int importance;//0:undef, 1:tiny, 2:alittle, 3:normal, 4:noticed, 5:imortant, 6:very important
			WCHAR comment[32];//WCHAR * 31文字まで。３２文字目は終端記号

		}CPMOTINFO;
	*/
	cpinfo.startframe = s_copymotvec[0].mp.GetFrame();
	cpinfo.framenum = s_copymotvec[cpelemnum - 1].mp.GetFrame() - cpinfo.startframe + 1;
	wcscpy_s(cpinfo.fbxname, MAX_PATH, s_model->GetFileName());
	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (!curmi) {
		return 1;
	}
	WCHAR wmotname[MAX_PATH] = { 0L };
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, curmi->motname, 256, wmotname, MAX_PATH);
	wcscpy_s(cpinfo.motionname, MAX_PATH, wmotname);

	CCpInfoDlg dlg;
	dlg.SetCpInfo(&cpinfo);
	int dlgret = (int)dlg.DoModal();
	if (dlgret != IDOK) {
		return 1;
	}


	*(srccptfilename + MAX_PATH - 1) = 0L;
	int cptfilenameleng = wcslen(srccptfilename);
	if ((cptfilenameleng <= 0) || (cptfilenameleng >= MAX_PATH)) {
		return 1;
	}


	//cpiファイル名はcptファイルの拡張子をcpiに変えたもの
	WCHAR cpifilename[MAX_PATH] = { 0L };
	wcscpy_s(cpifilename, MAX_PATH, srccptfilename);
	WCHAR* pext;
	pext = wcsrchr(cpifilename, TEXT('.'));
	if (!pext) {
		return 1;
	}
	*pext = 0L;
	wcscat_s(cpifilename, MAX_PATH, L".cpi");


	HANDLE hfile = CreateFile(cpifilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		DbgOut(L"CPTFile : WriteCPTFile : file open error !!!\n");
		_ASSERT(0);
		return 1;
	}


	char CPTheader[256];
	::ZeroMemory(CPTheader, sizeof(char) * 256);
	strcpy_s(CPTheader, 256, "MB3DTempCopyInfoFile ver1.0.0.18");//本体ではない

	DWORD wleng = 0;
	WriteFile(hfile, CPTheader, sizeof(char) * 256, &wleng, NULL);
	if (wleng != (sizeof(char) * 256)) {
		_ASSERT(0);
		return 1;
	}

	int datasize = sizeof(CPMOTINFO);
	wleng = 0;
	WriteFile(hfile, &datasize, sizeof(int), &wleng, NULL);
	if (wleng != sizeof(int)) {
		_ASSERT(0);
		return 1;
	}

	wleng = 0;
	WriteFile(hfile, &cpinfo, sizeof(CPMOTINFO), &wleng, NULL);
	if (wleng != (sizeof(CPMOTINFO))) {
		_ASSERT(0);
		return 1;
	}

	FlushFileBuffers(hfile);
	SetEndOfFile(hfile);
	CloseHandle(hfile);

	return 0;
}


bool ValidateCPIFile(char* dstCPIh, int* dstinfosize, char* srcbuf, DWORD bufleng)
{
	if (!dstCPIh || !dstinfosize || !srcbuf || (bufleng <= 0)) {
		_ASSERT(0);
		return false;
	}

	if (bufleng <= (sizeof(char) * 256 + sizeof(int))) {
		_ASSERT(0);
		return false;
	}

	MoveMemory(dstCPIh, srcbuf, sizeof(char) * 256);

	int magicstrlen;
	magicstrlen = (int)strlen(dstCPIh);
	if ((magicstrlen <= 0) || (magicstrlen >= 256)) {
		_ASSERT(0);
		return false;
	}
	int cmp18;
	cmp18 = strcmp(dstCPIh, "MB3DTempCopyInfoFile ver1.0.0.18");//本体ではない
	if (cmp18 != 0) {
		_ASSERT(0);
		return false;
	}


	int infosize;
	MoveMemory(&infosize, srcbuf + sizeof(char) * 256, sizeof(int));

	DWORD datasize;
	datasize = (bufleng - sizeof(char) * 256 - sizeof(int));
	DWORD elemsize;
	elemsize = sizeof(CPMOTINFO);

	if ((infosize != elemsize) || (datasize != elemsize)) {
		_ASSERT(0);
		return false;
	}

	*dstinfosize = infosize;

	return true;

}


bool ValidateCPTFile(char* dstCPTh, int* dstcpelemnum, char* srcbuf, DWORD bufleng)
{
	if (!dstCPTh || !dstcpelemnum || !srcbuf || (bufleng <= 0)) {
		_ASSERT(0);
		return false;
	}

	if (bufleng <= (sizeof(char) * 256 + sizeof(int))) {
		_ASSERT(0);
		return false;
	}

	MoveMemory(dstCPTh, srcbuf, sizeof(char) * 256);

	//typedef struct tag_CPTheader
	//{
	//	char magicstr[32];//EvaluateGlobalPosition
	//	char version[16];
	//	char fbxdate[256];
	//	int animno;
	//	int jointnum;
	//	int framenum;
	//	int reserved;
	//}CPTHEADER;

	int magicstrlen;
	magicstrlen = (int)strlen(dstCPTh);
	if ((magicstrlen <= 0) || (magicstrlen >= 256)) {
		_ASSERT(0);
		return false;
	}
	//int cmp7;
	//int cmp8;
	//int cmp9;
	//cmp7 = strcmp(dstCPTh, "MB3DTempCopyFramesFile ver1.0.0.7");//本体ではない
	//cmp8 = strcmp(dstCPTh, "MB3DTempCopyFramesFile ver1.0.0.8");//本体ではない
	//cmp9 = strcmp(dstCPTh, "MB3DTempCopyFramesFile ver1.0.0.9");//本体ではない
	//if ((cmp7 != 0) && (cmp8 != 0) && (cmp9 != 0)) {
	//	_ASSERT(0);
	//	return false;
	//}
	int cmp18;
	cmp18 = strcmp(dstCPTh, "MB3DTempCopyFramesFile ver1.0.0.18");//本体ではない
	if (cmp18 != 0) {
		_ASSERT(0);
		return false;
	}


	int cpelemnum;
	MoveMemory(&cpelemnum, srcbuf + sizeof(char) * 256, sizeof(int));

	DWORD datasize;
	datasize = (bufleng - sizeof(char) * 256 - sizeof(int));
	DWORD elemsize;
	elemsize = sizeof(char) * MAX_PATH + sizeof(double) + sizeof(ChaMatrix) + sizeof(int);

	if (datasize != (cpelemnum * elemsize)) {
		_ASSERT(0);
		return false;
	}

	*dstcpelemnum = cpelemnum;


	return true;
}

int LoadCPIFile(HISTORYELEM* srcdstelem)
{

	WCHAR cpifilename[MAX_PATH] = { 0L };
	wcscpy_s(cpifilename, MAX_PATH, srcdstelem->wfilename);

	WCHAR* pdot = wcsrchr(cpifilename, TEXT('.'));
	if (!pdot) {
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	*pdot = 0L;
	wcscat_s(cpifilename, MAX_PATH, L".cpi");

	HANDLE hfile;
	hfile = CreateFile(cpifilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		_ASSERT(0);
		srcdstelem->hascpinfo = 0;
		return 1;
	}


	DWORD sizehigh;
	DWORD bufleng;
	bufleng = GetFileSize(hfile, &sizehigh);
	if (bufleng <= 0) {
		_ASSERT(0);
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	if (sizehigh != 0) {
		_ASSERT(0);
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	char* newbuf;
	newbuf = (char*)malloc(sizeof(char) * bufleng);//bufleng + 1
	if (!newbuf) {
		_ASSERT(0);
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	ZeroMemory(newbuf, sizeof(char) * bufleng);
	DWORD rleng, readleng;
	rleng = bufleng;
	BOOL bsuccess;
	bsuccess = ReadFile(hfile, (void*)newbuf, rleng, &readleng, NULL);
	if (!bsuccess || (rleng != readleng)) {
		_ASSERT(0);
		if (newbuf) {
			free(newbuf);
			newbuf = 0;
		}
		CloseHandle(hfile);
		srcdstelem->hascpinfo = 0;
		return 1;
	}

	int infosize = 0;
	char CPIheader[256];
	ZeroMemory(CPIheader, sizeof(char) * 256);
	bool isvalid;
	isvalid = ValidateCPIFile(CPIheader, &infosize, newbuf, bufleng);
	if (!isvalid) {
		_ASSERT(0);
		if (newbuf) {
			free(newbuf);
			newbuf = 0;
		}
		CloseHandle(hfile);
		srcdstelem->hascpinfo = 0;
		return 1;
	}

	DWORD curpos;
	curpos = sizeof(char) * 256 + sizeof(int);

	CPMOTINFO cpmotinfo;
	ZeroMemory(&cpmotinfo, sizeof(CPMOTINFO));
	::MoveMemory(&cpmotinfo, newbuf + curpos, sizeof(CPMOTINFO));

	cpmotinfo.fbxname[MAX_PATH - 1] = 0L;
	cpmotinfo.motionname[MAX_PATH - 1] = 0L;
	if (cpmotinfo.fbxname[0] == 0L) {
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	if (cpmotinfo.motionname[0] == 0L) {
		srcdstelem->hascpinfo = 0;
		return 1;
	}

	if ((cpmotinfo.bvhtype < 0) || (cpmotinfo.bvhtype > 144)) {
		srcdstelem->hascpinfo = 0;
		return 1;
	}
	if ((cpmotinfo.importance < 0) || (cpmotinfo.importance > 6)) {
		srcdstelem->hascpinfo = 0;
		return 1;
	}

	srcdstelem->cpinfo = cpmotinfo;
	srcdstelem->hascpinfo = 1;

	if (newbuf) {
		free(newbuf);
		newbuf = 0;
	}
	CloseHandle(hfile);

	return 0;

}



bool LoadCPTFile()
{
	if (!s_model) {
		_ASSERT(0);
		return false;
	}
	if (!s_model->GetCurMotInfo()) {
		_ASSERT(0);
		return false;
	}


	s_pastemotvec.clear();

	//std::vector<HISTORYELEM> cptfilename;
	s_cptfilename.clear();
	GetCPTFileName(s_cptfilename);

	if (s_cptfilename.empty()) {
		_ASSERT(0);
		return false;
	}

	WCHAR infilename[MAX_PATH] = { 0L };

	if (s_copyhistorydlg.IsCheckedMostRecent()) {
		wcscpy_s(infilename, MAX_PATH, s_cptfilename[0].wfilename);
	}
	else {
		int result = s_copyhistorydlg.GetSelectedFileName(infilename);
		infilename[MAX_PATH - 1] = 0L;

		if (result || (infilename[0] == 0L)) {
			_ASSERT(0);
			return false;
		}
	}


	HANDLE hfile;
	hfile = CreateFile(infilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		_ASSERT(0);
		return false;
	}


	DWORD sizehigh;
	DWORD bufleng;
	bufleng = GetFileSize(hfile, &sizehigh);
	if (bufleng <= 0) {
		_ASSERT(0);
		return false;
	}
	if (sizehigh != 0) {
		_ASSERT(0);
		return false;
	}
	char* newbuf;
	newbuf = (char*)malloc(sizeof(char) * bufleng);//bufleng + 1
	if (!newbuf) {
		_ASSERT(0);
		return false;
	}
	ZeroMemory(newbuf, sizeof(char) * bufleng);
	DWORD rleng, readleng;
	rleng = bufleng;
	BOOL bsuccess;
	bsuccess = ReadFile(hfile, (void*)newbuf, rleng, &readleng, NULL);
	if (!bsuccess || (rleng != readleng)) {
		_ASSERT(0);
		if (newbuf) {
			free(newbuf);
			newbuf = 0;
		}
		CloseHandle(hfile);
		return false;
	}

	int cpelemnum = 0;
	char CPTheader[256];
	ZeroMemory(CPTheader, sizeof(char) * 256);
	bool isvalid;
	isvalid = ValidateCPTFile(CPTheader, &cpelemnum, newbuf, bufleng);
	if (!isvalid) {
		_ASSERT(0);
		if (newbuf) {
			free(newbuf);
			newbuf = 0;
		}
		CloseHandle(hfile);
		return false;
	}

	DWORD curpos;
	curpos = sizeof(char) * 256 + sizeof(int);
	
	int cpelemno;
	for (cpelemno = 0; cpelemno < cpelemnum; cpelemno++) {
		char curbonename[MAX_PATH] = { 0 };
		ZeroMemory(curbonename, sizeof(char) * MAX_PATH);
		if ((curpos + (sizeof(char) * MAX_PATH)) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		::MoveMemory(curbonename, newbuf + curpos, sizeof(char) * MAX_PATH);
		curpos += (sizeof(char) * MAX_PATH);


		double curframe;
		if ((curpos + sizeof(double)) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		::MoveMemory(&curframe, newbuf + curpos, sizeof(double));
		curpos += sizeof(double);



		ChaMatrix curlocalmat;
		if ((curpos + sizeof(ChaMatrix)) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		::MoveMemory(&curlocalmat, newbuf + curpos, sizeof(ChaMatrix));
		curpos += sizeof(ChaMatrix);



		int localmatflag = 0;
		if ((curpos + sizeof(int)) > bufleng) {
			_ASSERT(0);
			if (newbuf) {
				free(newbuf);
				newbuf = 0;
			}
			CloseHandle(hfile);
			return false;
		}
		::MoveMemory(&localmatflag, newbuf + curpos, sizeof(int));
		curpos += sizeof(int);



		CBone* curbone;
		curbone = s_model->GetBoneByName(curbonename);
		if (curbone) {
			CPELEM2 curcpelem;
			ZeroMemory(&curcpelem, sizeof(CPELEM2));

			curcpelem.bone = curbone;
			curcpelem.mp.SetFrame(curframe);
			curcpelem.mp.SetWorldMat(curlocalmat);
			curcpelem.mp.SetLocalMatFlag(localmatflag);

			s_pastemotvec.push_back(curcpelem);
		}
		else {
			_ASSERT(0);
		}
	}

	if (newbuf) {
		free(newbuf);
		newbuf = 0;
	}
	CloseHandle(hfile);

	return true;
}

void ChangeCurDirFromMameMediaToTest()
{
	//CurrentDirectoryがMameMediaになっていたときにはTestディレクトリに変える
	WCHAR curdir[MAX_PATH] = { 0L };
	ZeroMemory(curdir, sizeof(WCHAR) * MAX_PATH);
	GetCurrentDirectory(MAX_PATH, curdir);
	WCHAR* findpat = wcsstr(curdir, L"\\MameMedia");
	if (findpat) {
		WCHAR initialdir[MAX_PATH] = { 0L };
		wcscpy_s(initialdir, MAX_PATH, g_basedir);
		wcscat_s(initialdir, MAX_PATH, L"..\\Test\\");
		SetCurrentDirectoryW(initialdir);
	}
}

void InitTimelineSelection()
{
	s_editrange.Clear();
	s_buttonselectstart = 1.0;
	s_buttonselectend = 1.0;

	if (g_motionbrush_value) {
		free(g_motionbrush_value);
		g_motionbrush_value = 0;
	}
	g_motionbrush_value = (float*)malloc(sizeof(float) * 1);
	if (g_motionbrush_value) {
		*g_motionbrush_value = 1.0f;
	}

	//g_motionbrush_method = 0;
	g_motionbrush_startframe = 1.0;
	g_motionbrush_endframe = 1.0;
	g_motionbrush_applyframe = 1.0;
	g_motionbrush_numframe = 1.0;
	g_motionbrush_frameleng = 1;

	if (s_owpTimeline) {
		s_owpTimeline->setCurrentTime(1.0, false);
	}
	if (s_owpLTimeline) {
		s_owpLTimeline->setCurrentTime(1.0, false);
	}
	
	//if (s_model) {
	//	MOTINFO* curmi;
	//	curmi = s_model->GetCurMotInfo();
	//	if (curmi) {
	//		s_model->SetMotionFrame(1.0);
	//	}
	//}

	int tothelastflag = 0;
	OnTimeLineButtonSelectFromSelectStartEnd(tothelastflag);

}


void OnArrowKey()
{

	//マウスカーソルが起動中ソフトのウインドウ内にある場合だけ処理
	//複数個のMotionBrushを立ち上げたときに操作中のMotionBrushだけに影響するように
	POINT cursorpoint;
	GetCursorPos(&cursorpoint);
	::ScreenToClient(s_mainhwnd, &cursorpoint);
	RECT appclientrect;
	GetClientRect(s_mainhwnd, &appclientrect);
	if ((cursorpoint.x < appclientrect.left) || (cursorpoint.x > appclientrect.right) || 
		(cursorpoint.y < appclientrect.top) || (cursorpoint.y > appclientrect.bottom)) {
		//MainWindow外につき処理しない
		return;
	}


	int parentbuttonid = 4;
	int sisterbuttonid = 5;
	int childbuttonid = 6;
	int brotherbuttonid = 7;
	int accelaxisid1 = 4;//axisid
	int accelaxisid2 = 5;//axisid

	//int parentbutton = 0;
	//int sisterbutton = 0;
	//int childbutton = 0;
	//int brotherbutton = 0;
	//int accelaxis1 = 0;
	//int accelaxis2 = 0;

	//parentbutton = s_dsbuttonup[parentbuttonid];
	//sisterbutton = s_dsbuttonup[sisterbuttonid];
	//childbutton = s_dsbuttonup[childbuttonid];
	//brotherbutton = s_dsbuttonup[brotherbuttonid];

	//accelaxis1 = ((bool)(s_dsaxisOverSrh[accelaxisid1] + s_dsaxisMOverSrh[accelaxisid1]));
	//accelaxis2 = ((bool)(s_dsaxisOverSrh[accelaxisid2] + s_dsaxisMOverSrh[accelaxisid2]));

	s_dsbuttonup[parentbuttonid] = 0;
	s_dsbuttonup[sisterbuttonid] = 0;
	s_dsbuttonup[childbuttonid] = 0;
	s_dsbuttonup[brotherbuttonid] = 0;
	
	s_currentwndid = MB3D_WND_TREE;


	bool arrowkeypushed = false;
	if (((g_savekeybuf[VK_UP] & 0x80) == 0) && ((g_keybuf[VK_UP] & 0x80) != 0)) {
		s_dsbuttonup[parentbuttonid] = 1;
		s_dsbuttonup[sisterbuttonid] = 0;
		s_dsbuttonup[childbuttonid] = 0;
		s_dsbuttonup[brotherbuttonid] = 0;
		arrowkeypushed = true;
	}
	else if (((g_savekeybuf[VK_DOWN] & 0x80) == 0) && ((g_keybuf[VK_DOWN] & 0x80) != 0)) {
		s_dsbuttonup[parentbuttonid] = 0;
		s_dsbuttonup[sisterbuttonid] = 0;
		s_dsbuttonup[childbuttonid] = 1;
		s_dsbuttonup[brotherbuttonid] = 0;
		arrowkeypushed = true;
	}
	else if (((g_savekeybuf[VK_LEFT] & 0x80) == 0) && ((g_keybuf[VK_LEFT] & 0x80) != 0)) {
		s_dsbuttonup[parentbuttonid] = 0;
		s_dsbuttonup[sisterbuttonid] = 1;
		s_dsbuttonup[childbuttonid] = 0;
		s_dsbuttonup[brotherbuttonid] = 0;
		arrowkeypushed = true;
	}
	else if (((g_savekeybuf[VK_RIGHT] & 0x80) == 0) && ((g_keybuf[VK_RIGHT] & 0x80) != 0)) {
		s_dsbuttonup[parentbuttonid] = 0;
		s_dsbuttonup[sisterbuttonid] = 0;
		s_dsbuttonup[childbuttonid] = 0;
		s_dsbuttonup[brotherbuttonid] = 1;
		arrowkeypushed = true;
	}

	if (arrowkeypushed == true) {
		bool firstctrlselect = false;
		DSCrossButtonSelectTree(firstctrlselect);

	}

}

double CalcRefFrame()
{
	double startframe = s_editrange.GetStartFrame();
	double endframe = s_editrange.GetEndFrame();
	if (startframe != endframe) {
		double offset = 0;
		double refframe = (double)((int)(startframe + (endframe - startframe) * g_refpos / 100.0 + offset));

		return refframe;
	}
	else {
		return -1.0;
	}

}

ChaMatrix CalcRigMat(CBone* curbone, int curmotid, double curframe, int dispaxis, int disporder, bool posinverse)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (!curbone) {
		return retmat;
	}

	int multworld = 1;
	ChaMatrix selm = curbone->CalcManipulatorMatrix(0, 0, multworld, curmotid, curframe);
	selm._41 = 0.0f;
	selm._42 = 0.0f;
	selm._43 = 0.0f;


	CalcSelectScale(curbone);//s_selectscaleにセット
	
	ChaMatrix scalemat;
	ChaMatrixIdentity(&scalemat);
	ChaMatrixScaling(&scalemat, s_selectscale, s_selectscale, s_selectscale);

	ChaVector3 curbonepos = curbone->GetWorldPos(curmotid, curframe);

	float rigorderoffset = 0.0f;
	if (disporder == 0) {
		rigorderoffset = 0.05f;
	}
	else if (disporder == 1) {
		rigorderoffset = 0.35f;
	}
	else if (disporder == 2) {
		rigorderoffset = 0.65f;
	}

	if (posinverse) {
		rigorderoffset *= -1.0f;
	}


	ChaVector3 offsetorgpos = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 offsetdisppos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (dispaxis == 0) {
		offsetorgpos = ChaVector3(rigorderoffset, 0.0f, 0.0f);
	}
	else if (dispaxis == 1) {
		offsetorgpos = ChaVector3(0.0f, rigorderoffset, 0.0f);
	}
	else if (dispaxis == 2) {
		offsetorgpos = ChaVector3(0.0f, 0.0f, rigorderoffset);
	}
	ChaVector3TransformCoord(&offsetdisppos, &offsetorgpos, &selm);
	ChaMatrix tramat;
	ChaMatrixIdentity(&tramat);
	ChaMatrixTranslation(&tramat, curbonepos.x + offsetdisppos.x, curbonepos.y + offsetdisppos.y, curbonepos.z + offsetdisppos.z);

	retmat = scalemat * tramat;

	return retmat;
}

int PickRigBone(PICKINFO* ppickinfo)
{
	if (!s_model || !ppickinfo) {
		return -1;
	}



	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (curmi) {
		int curmotid = curmi->motid;
		double curframe = curmi->curframe;

		std::map<int, CBone*>::iterator itrbone;
		for (itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++) {
			CBone* curbone = (CBone*)itrbone->second;
			if (curbone) {
				int rigno;
				for (rigno = 0; rigno < MAXRIGNUM; rigno++) {
					CUSTOMRIG currig = curbone->GetCustomRig(rigno);
					if (currig.useflag == 2) {//0: free, 1: allocated, 2: valid
						//s_matrig->SetDif4F(s_matrigmat);

						ChaMatrix rigmat;
						ChaMatrixIdentity(&rigmat);
						rigmat = CalcRigMat(curbone, curmotid, curframe, currig.dispaxis, currig.disporder, currig.posinverse);


						g_hmWorld->SetMatrix((float*)&rigmat);
						s_rigmark->UpdateMatrix(&rigmat, &s_matVP);


						int chkboneno = curbone->GetBoneNo();

						PICKINFO chkpickinfo;
						chkpickinfo = *ppickinfo;

						chkpickinfo.buttonflag = PICK_CENTER;
						chkpickinfo.pickobjno = chkboneno;

						int colliobj;
						colliobj = s_rigmark->CollisionNoBoneObj_Mouse(&chkpickinfo, "obj1");
						if (colliobj) {
							RollbackCurBoneNo();

							*ppickinfo = chkpickinfo;

							s_curboneno = chkboneno;


							CBone* chkbone = s_model->GetBoneByID(s_curboneno);
							if (chkbone != s_customrigbone) {
								//開いている設定ダイアログを閉じないと、設定ダイアログのrigboneと新たなrigboneが異なってしまい、Applyボタンで異なるリグを保存することがある
								if (s_customrigdlg) {
									DestroyWindow(s_customrigdlg);
									s_customrigdlg = 0;
								}
							}

							if (s_owpTimeline) {
								s_owpTimeline->setCurrentLine(s_boneno2lineno[s_curboneno], true);
								//WindowPos currentpos = s_owpTimeline->getCurrentLinePos();
								//POINT mousepos;
								//mousepos.x = currentpos.x;
								//mousepos.y = currentpos.y;
								//::ClientToScreen(s_timelineWnd->getHWnd(), &mousepos);
								//::SetCursorPos(mousepos.x, mousepos.y);
							}
							ChangeCurrentBone();//2021/11/19

							Bone2CustomRig(rigno);

							return chkboneno;//!!!!!!!!!!!!!!!!!!!!!!!!!!!! found !!!!!!!!!!!!!!!!!!
						}

					}
				}
			}
		}
	}
	else {
		return -1;
	}

	return -1;

}

int LoadIniFile()
{
	WCHAR path[MAX_PATH] = { 0L };
	wcscpy_s(path, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(path, TEXT('\\'));
	if (!lasten) {
		_ASSERT(0);
		return 1;
	}
	*lasten = 0L;
	last2en = wcsrchr(path, TEXT('\\'));
	if (!last2en) {
		_ASSERT(0);
		return 1;
	}
	*last2en = 0L;

	WCHAR inifilepath[MAX_PATH] = { 0L };
	swprintf_s(inifilepath, MAX_PATH, L"%s\\MotionBrush%d.ini", path, s_appcnt);

	CIniFile inifile;
	inifile.LoadIniFile(inifilepath);

	return 0;
}

int SaveIniFile()
{
	WCHAR path[MAX_PATH] = { 0L };
	wcscpy_s(path, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(path, TEXT('\\'));
	if (!lasten) {
		_ASSERT(0);
		return 1;
	}
	*lasten = 0L;
	last2en = wcsrchr(path, TEXT('\\'));
	if (!last2en) {
		_ASSERT(0);
		return 1;
	}
	*last2en = 0L;

	WCHAR inifilepath[MAX_PATH] = { 0L };
	swprintf_s(inifilepath, MAX_PATH, L"%s\\MotionBrush%d.ini", path, s_appcnt);

	CIniFile inifile;
	inifile.WriteIniFile(inifilepath);

	return 0;
}

int SetTimelineHasRigFlag()
{
	if (!s_owpTimeline) {
		return 0;
	}
	if (!s_model) {
		return 0;
	}


	std::map<int, CBone*>::iterator itrbone;
	for (itrbone = s_model->GetBoneListBegin(); itrbone != s_model->GetBoneListEnd(); itrbone++) {
		CBone* curbone = (CBone*)itrbone->second;
		if (curbone) {
			bool hasrigflag = s_model->ChkBoneHasRig(curbone);
			s_owpTimeline->setHasRigFlag(curbone->GetWBoneName(), hasrigflag);
		}
	}

	return 0;
}


int ClearLimitedWM()
{
	if (!s_model) {
		return 0;
	}
	MOTINFO* curmi = s_model->GetCurMotInfo();
	if (!curmi) {
		return 0;
	}

	double frameleng = curmi->frameleng;

	double curframe;
	for (curframe = 0.0; curframe < frameleng; curframe += 1.0) {
		s_model->ClearLimitedWM(curmi->motid, curframe);
	}

	return 0;
}

int SetShowPosTime()
{

	double startframe, endframe, applyframe;
	int selnum;
	s_editrange.GetRange(&selnum, &startframe, &endframe, &applyframe);

	double curframe = applyframe;

	if (s_owpTimeline) {
		s_owpTimeline->setShowPosTime(curframe);
	}
	if (s_owpLTimeline) {
		s_owpLTimeline->setShowPosTime(curframe);
	}

	return 0;
}

