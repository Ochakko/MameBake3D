#ifndef GLOBALVARH
#define GLOBALVARH

//#include <d3dx10.h>
#include <ChaVecCalc.h>
#include <TexBank.h>

#ifdef MAMEBAKE3DLIBGLOBALVAR
ID3D10DepthStencilState *g_pDSStateZCmp = 0;
ID3D10DepthStencilState *g_pDSStateZCmpAlways = 0;
ID3D10ShaderResourceView* g_presview = 0;

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
float g_initmass = 1.0f;

ChaVector3 g_camEye = ChaVector3(0.0f, 0.0f, 0.0f);
ChaVector3 g_camtargetpos = ChaVector3(0.0f, 0.0f, 0.0f);

float g_l_kval[3] = { 1.0f, powf(10.0f, 2.61f), 2000.0f };//
float g_a_kval[3] = { 0.1f, powf(10.0f, 0.3f), 70.0f };//
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
WCHAR g_tmpmqopath[MULTIPATH] = { 0L };
float g_tmpbvhfilter = 100.0f;

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

WCHAR g_basedir[MAX_PATH] = { 0 };

double						g_dspeed = 3.0;
//double						g_dspeed = 0.0;


float g_ikfirst = 1.0f;
float g_ikrate = 1.0f;
int g_applyrate = 50;
float g_physicsmvrate = 1.0f;

float                       g_fLightScale;
int                         g_nNumActiveLights;
int                         g_nActiveLight;

#else
extern ID3D10DepthStencilState *g_pDSStateZCmp;
extern ID3D10DepthStencilState *g_pDSStateZCmpAlways;
extern ID3D10ShaderResourceView* g_presview;

extern int	g_numthread;
extern double g_btcalccnt;
extern int g_dbgloadcnt;
extern double g_calcfps;

extern bool g_selecttolastFlag;
extern bool g_underselecttolast;
extern bool g_undereditrange;

extern bool g_limitdegflag;
extern bool g_wmatDirectSetFlag;
extern bool g_underRetargetFlag;

extern float g_impscale;
extern float g_initmass;

extern ChaVector3 g_camEye;
extern ChaVector3 g_camtargetpos;

extern float g_l_kval[3];//
extern float g_a_kval[3];//
extern float g_initcuslk;
//float g_initcuslk = 2000.0f;
//float g_initcuslk = 100.0f;
extern float g_initcusak;


//float g_l_dmp = 0.75f;
//float g_a_dmp = 0.50f;

extern float g_l_dmp;
extern float g_a_dmp;

extern int g_previewFlag;			// プレビューフラグ

extern int g_applyendflag;
extern int g_slerpoffflag;
extern int g_absikflag;
extern int g_bonemarkflag;
extern int g_pseudolocalflag;
extern int g_boneaxis;

extern CTexBank*	g_texbank;

extern float g_tmpmqomult;
extern WCHAR g_tmpmqopath[MULTIPATH];
extern float g_tmpbvhfilter;

extern ID3D10EffectTechnique* g_hRenderBoneL0;
extern ID3D10EffectTechnique* g_hRenderBoneL1;
extern ID3D10EffectTechnique* g_hRenderBoneL2;
extern ID3D10EffectTechnique* g_hRenderBoneL3;
extern ID3D10EffectTechnique* g_hRenderNoBoneL0;
extern ID3D10EffectTechnique* g_hRenderNoBoneL1;
extern ID3D10EffectTechnique* g_hRenderNoBoneL2;
extern ID3D10EffectTechnique* g_hRenderNoBoneL3;
extern ID3D10EffectTechnique* g_hRenderLine;
extern ID3D10EffectTechnique* g_hRenderSprite;

extern ID3D10EffectMatrixVariable* g_hm4x4Mat;
extern ID3D10EffectMatrixVariable* g_hmWorld;
extern ID3D10EffectMatrixVariable* g_hmVP;

extern ID3D10EffectVectorVariable* g_hEyePos;
extern ID3D10EffectScalarVariable* g_hnNumLight;
extern ID3D10EffectVectorVariable* g_hLightDir;
extern ID3D10EffectVectorVariable* g_hLightDiffuse;
extern ID3D10EffectVectorVariable* g_hLightAmbient;
extern ID3D10EffectVectorVariable* g_hSpriteOffset;
extern ID3D10EffectVectorVariable* g_hSpriteScale;
extern ID3D10EffectVectorVariable* g_hPm3Scale;
extern ID3D10EffectVectorVariable* g_hPm3Offset;


extern ID3D10EffectVectorVariable* g_hdiffuse;
extern ID3D10EffectVectorVariable* g_hambient;
extern ID3D10EffectVectorVariable* g_hspecular;
extern ID3D10EffectScalarVariable* g_hpower;
extern ID3D10EffectVectorVariable* g_hemissive;
extern ID3D10EffectShaderResourceVariable* g_hMeshTexture;

extern BYTE g_keybuf[256];
extern BYTE g_savekeybuf[256];

extern WCHAR g_basedir[MAX_PATH];

extern double						g_dspeed;
//double						g_dspeed = 0.0;


extern float g_ikfirst;
extern float g_ikrate;
extern int g_applyrate;
extern float g_physicsmvrate;

extern float                       g_fLightScale;
extern int                         g_nNumActiveLights;
extern int                         g_nActiveLight;

#endif

#endif

