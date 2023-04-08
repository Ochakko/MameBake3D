#include "stdafx.h"
//#include <stdafx.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <Model.h>
#include <polymesh3.h>
#include <polymesh4.h>
#include <ExtLine.h>

#include <GetMaterial.h>

#include <mqofile.h>
#include <mqomaterial.h>
#include <mqoobject.h>

#include <Bone.h>
#include <mqoface.h>

#include <InfoWindow.h>

#include <FbxFile.h>

#include <EGPFile.h>

#define DBGH
#include <dbg.h>

//#include <DXUT.h>
//#include <DXUTcamera.h>
//#include <DXUTgui.h>
//#include <DXUTsettingsdlg.h>
//#include <SDKmisc.h>

#include <DispObj.h>
#include <MySprite.h>

#include <MotionPoint.h>
//#include <quaternion.h>
#include <ChaVecCalc.h>

#include <Collision.h>
#include <EngName.h>

#include <RigidElem.h>

#include <string>

#include <fbxsdk.h>
#include <fbxsdk/scene/shading/fbxlayeredtexture.h>
#include <fbxsdk/scene/animation/fbxanimevaluator.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"


#include <BopFile.h>
#include <BtObject.h>

#include <collision.h>
#include <EditRange.h>
//#include <BoneProp.h>

#include <ThreadingLoadFbx.h>
#include <ThreadingUpdateMatrix.h>

#include <DXUT.h>
#include <io.h>


//for __nop()
#include <intrin.h>


using namespace std;


////######################################
//// Custom stream 
//
//static long long int s_mpos = 0;
//
//class CustomStreamClass : public FbxStream
//{
//public:
//	CustomStreamClass(FbxManager* pSdkManager, const char* mode, const char* ut8name)
//	{
//		mFile = NULL;
//		mbuffer = 0;
//		msize = 0;
//		s_mpos = 0;
//
//		// expect the mode to contain something
//		if (mode == NULL) return;
//
//
//		FBXSDK_strcpy(mFileName, MAX_PATH, ut8name);
//		FBXSDK_strcpy(mMode, 3, (mode) ? mode : "r");
//
//		if (mode[0] == 'r')
//		{
//			const char* format = "FBX (*.fbx)";
//			mReaderID = pSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription(format);
//			mWriterID = -1;
//		}
//		else
//		{
//			//const char* format = "FBX ascii (*.fbx)";
//			//mWriterID = pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription(format);
//			//mReaderID = -1;
//			mWriterID = -1;
//			mReaderID = -1;
//		}
//	}
//
//	~CustomStreamClass()
//	{
//		Close();
//	}
//
//	virtual EState GetState()
//	{
//		return mFile ? FbxStream::eOpen : eClosed;
//	}
//
//	virtual bool Open(void* /*pStreamData*/)
//	{
//		if (mMode[0] != 'r') {
//			return false;
//		}
//
//		if (mFile == NULL) {
//			fopen_s(&mFile, mFileName, mMode);
//			if (!mFile) {
//				return false;
//			}
//
//			msize = _filelengthi64(_fileno(mFile));
//			if (msize == -1L) {
//				return false;
//			}
//
//			mbuffer = (char*)malloc(sizeof(char) * msize);
//			if (!mbuffer) {
//				return false;
//			}
//			ZeroMemory(mbuffer, sizeof(char) * msize);
//
//			long long int doread = 0;
//			while (doread < msize) {
//				doread += fread(mbuffer + doread, 1, (msize - doread), mFile);
//			}
//			s_mpos = 0;
//			return true;
//		}
//		else {
//			return false;
//		}
//
//
//	}
//
//	virtual bool Close()
//	{
//		// This method can be called several times during the
//		// Initialize phase so it is important that it can handle multiple closes
//		if (mFile) {
//			fclose(mFile);
//			mFile = NULL;
//		}
//		if (mbuffer) {
//			free(mbuffer);
//			mbuffer = 0;
//		}
//		s_mpos = 0;
//
//		mWriterID = -1;
//		mReaderID = -1;
//
//		return true;
//	}
//
//	virtual bool Flush()
//	{
//		return true;
//	}
//
//	virtual int Write(const void* pData, int pSize)
//	{
//		//if (mFile == NULL)
//		//	return 0;
//		//return (int)fwrite(pData, 1, pSize, mFile);
//		return 0;
//	}
//
//	virtual int Read(void* pData, int pSize) const
//	{
//		if (mFile == NULL) {
//			return 0;
//		}
//		if (!mbuffer) {
//			return 0;
//		}
//
//
//		long long int readsize;
//		if ((s_mpos >= 0) && ((s_mpos + pSize) <= msize)) {
//			readsize = pSize;
//		}
//		else if ((s_mpos >= 0) && ((s_mpos + pSize) > msize)) {
//			readsize = msize - s_mpos;
//		}
//		else {
//			readsize = 0;
//		}
//
//		if (readsize > 0) {
//			MoveMemory(pData, mbuffer + s_mpos, readsize);
//		}
//
//		s_mpos = s_mpos + readsize;//constなのでメンバー変数を変えることが出来ない。mposを外で定義する。
//
//		return readsize;
//	}
//
//	virtual int GetReaderID() const
//	{
//		return mReaderID;
//	}
//
//	virtual int GetWriterID() const
//	{
//		return mWriterID;
//	}
//
//	void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
//	{
//		switch (pSeekPos)
//		{
//		case FbxFile::eBegin:
//			//fseek(mFile, (long)pOffset, SEEK_SET);
//			s_mpos = (long)pOffset;
//			break;
//		case FbxFile::eCurrent:
//			//fseek(mFile, (long)pOffset, SEEK_CUR);
//			s_mpos = s_mpos + (long)pOffset;
//			break;
//		case FbxFile::eEnd:
//			//fseek(mFile, (long)pOffset, SEEK_END);
//			s_mpos = msize - (long)pOffset;
//			break;
//		}
//	}
//
//	virtual long GetPosition() const
//	{
//		if (mFile == NULL) {
//			return 0;
//		}
//		if (!mbuffer) {
//			return 0;
//		}
//		//return ftell(mFile);
//		return s_mpos;
//	}
//	virtual void SetPosition(long pPosition)
//	{
//		if (mFile && mbuffer) {
//			//fseek(mFile, pPosition, SEEK_SET);
//			s_mpos = pPosition;
//		}
//	}
//
//	virtual int GetError() const
//	{
//		if (mFile == NULL) {
//			return 0;
//		}
//		if (!mbuffer) {
//			return 0;
//		}
//		//return ferror(mFile);
//		if ((s_mpos < 0) || (s_mpos > msize)) {
//			return -1;
//		}
//		return 0;
//	}
//	virtual void ClearError()
//	{
//		if (mFile != NULL) {
//			//clearerr(mFile);
//			s_mpos = 0;
//		}
//	}
//
//private:
//	FILE* mFile;
//	int             mReaderID;
//	int             mWriterID;
//	char    mFileName[MAX_PATH];
//	char    mMode[3];
//	char* mbuffer;
//	long long int msize;
//	//long long int mpos;
//};
//
//
////#######################################
////#######################################


using namespace OrgWinGUI;

static int s_alloccnt = 0;
static FbxManager* s_psdk = 0;
static FbxAnimEvaluator* s_animevaluator = 0;

#define MAMEBAKE3DLIBGLOBALVAR
#include <GlobalVar.h>

extern bool g_btsimurecflag;

extern CRITICAL_SECTION g_CritSection_GetGP;
extern int CalcLocalMeshMat(FbxMesh* pMesh, ChaMatrix* dstmeshmat);//FbxFile.cpp


/*
extern ID3D11DepthStencilState *g_pDSStateZCmp;
extern ID3D11DepthStencilState *g_pDSStateZCmpAlways;

extern int g_boneaxis;
extern int g_dbgloadcnt;
extern int g_pseudolocalflag;
extern int g_previewFlag;			// プレビューフラグ
extern WCHAR g_basedir[ MAX_PATH ];

extern ID3D11Effect*		g_pEffect;
extern ID3D11EffectTechnique* g_hRenderBoneL0;
extern ID3D11EffectTechnique* g_hRenderBoneL1;
extern ID3D11EffectTechnique* g_hRenderBoneL2;
extern ID3D11EffectTechnique* g_hRenderBoneL3;
extern ID3D11EffectTechnique* g_hRenderNoBoneL0;
extern ID3D11EffectTechnique* g_hRenderNoBoneL1;
extern ID3D11EffectTechnique* g_hRenderNoBoneL2;
extern ID3D11EffectTechnique* g_hRenderNoBoneL3;
extern ID3D11EffectTechnique* g_hRenderLine;
extern ID3D11EffectTechnique* g_hRenderSprite;

extern ID3D11EffectMatrixVariable* g_hm4x4Mat;
extern ID3D11EffectMatrixVariable* g_hmWorld;
extern ID3D11EffectMatrixVariable* g_hmVP;

extern ID3D11EffectVectorVariable* g_hEyePos;
extern ID3D11EffectScalarVariable* g_hnNumLight;
extern ID3D11EffectVectorVariable* g_hLightDir;
extern ID3D11EffectVectorVariable* g_hLightDiffuse;
extern ID3D11EffectVectorVariable* g_hLightAmbient;

extern ID3D11EffectVectorVariable*g_hdiffuse;
extern ID3D11EffectVectorVariable* g_hambient;
extern ID3D11EffectVectorVariable* g_hspecular;
extern ID3D11EffectScalarVariable* g_hpower;
extern ID3D11EffectVectorVariable* g_hemissive;
extern ID3D11EffectShaderResourceVariable* g_hMeshTexture;


extern float g_impscale;
extern btScalar G_ACC; // 重力加速度 : BPWorld.cpp

extern float g_l_kval[3];
extern float g_a_kval[3];

extern float g_ikfirst;
extern float g_ikrate;
extern int g_slerpoffflag;
extern int g_absikflag;
extern int g_applyendflag;
extern int g_bonemarkflag;
extern float g_physicsmvrate;

extern ChaVector3 g_camEye;
extern ChaVector3 g_camtargetpos;

//extern bool g_wmatDirectSetFlag;
*/

int g_dbgflag = 0;


//////////



//以下の５つの関数は関数名にFbxを付けて FbxFile.h, FbxFile.cppに移動になりました。リンクエラーの関係で。
// 
// 
// 調査の結果、リンクエラーの原因はFbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);だったのですが以下の５つの関数はFbxFileに移動のままで。
// 
// 
//static FbxAMatrix GetGlobalPosition((CModel* srcmodel, FbxScene* pScene, FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition = NULL );
//static FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
//static FbxAMatrix GetGeometry(FbxNode* pNode);
//static FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial* pMaterial, const char* pPropertyName, const char* pFactorPropertyName, char** ppTextureName);
//static int IsValidCluster(FbxCluster* pcluster);


static void ChaMatrix2FbxAMatrix(FbxAMatrix& retmat, ChaMatrix& srcmat);
static void FbxAMatrix2ChaMatrix(ChaMatrix& retmat, FbxAMatrix srcmat);


void ChaMatrix2FbxAMatrix(FbxAMatrix& retmat, ChaMatrix& srcmat)
{
	retmat.SetIdentity();
	retmat.SetRow(0, FbxVector4(srcmat.data[MATI_11], srcmat.data[MATI_12], srcmat.data[MATI_13], srcmat.data[MATI_14]));
	retmat.SetRow(1, FbxVector4(srcmat.data[MATI_21], srcmat.data[MATI_22], srcmat.data[MATI_23], srcmat.data[MATI_24]));
	retmat.SetRow(2, FbxVector4(srcmat.data[MATI_31], srcmat.data[MATI_32], srcmat.data[MATI_33], srcmat.data[MATI_34]));
	retmat.SetRow(3, FbxVector4(srcmat.data[MATI_41], srcmat.data[MATI_42], srcmat.data[MATI_43], srcmat.data[MATI_44]));
	//retmat.SetRow(0, FbxVector4(srcmat._11, srcmat._21, srcmat._31, srcmat._41));
	//retmat.SetRow(1, FbxVector4(srcmat._12, srcmat._22, srcmat._32, srcmat._42));
	//retmat.SetRow(2, FbxVector4(srcmat._13, srcmat._23, srcmat._33, srcmat._43));
	//retmat.SetRow(3, FbxVector4(srcmat._14, srcmat._24, srcmat._34, srcmat._44));

}

void FbxAMatrix2ChaMatrix(ChaMatrix& retmat, FbxAMatrix srcmat)
{
	ChaMatrixIdentity(&retmat);
	retmat = ChaMatrix(
		(float)srcmat.Get(0, 0), (float)srcmat.Get(0, 1), (float)srcmat.Get(0, 2), (float)srcmat.Get(0, 3),
		(float)srcmat.Get(1, 0), (float)srcmat.Get(1, 1), (float)srcmat.Get(1, 2), (float)srcmat.Get(1, 3),
		(float)srcmat.Get(2, 0), (float)srcmat.Get(2, 1), (float)srcmat.Get(2, 2), (float)srcmat.Get(2, 3),
		(float)srcmat.Get(3, 0), (float)srcmat.Get(3, 1), (float)srcmat.Get(3, 2), (float)srcmat.Get(3, 3)
	);
}


static int s_setrigidflag = 0;
static DWORD s_rigidflag = 0;

CModel::CModel()
{
	InitializeCriticalSection(&m_CritSection_Node);

	InitParams();
	s_alloccnt++;
	m_modelno = s_alloccnt;
}
CModel::~CModel()
{
	//DeleteCriticalSection(&m_CritSection_Node);//DestroyObjsのthread終了よりも後に移動


	DestroyObjs();
}
int CModel::InitParams()
{
	m_iktargetbonevec.clear();

	m_materialdisprate = ChaVector4(1.0f, 1.0f, 1.0f, 1.0f);//diffuse, specular, emissive, ambient
	m_currentanimlayer = 0;

	m_fromBvh = false;
	m_fromNoBindPose = false;

	m_loadingmotionnum = 0;

	m_physikrec0.clear();
	m_physikrec.clear();
	m_phyikrectime = 0.0;

	m_bonelist.clear();
	m_boneupdatematrix = 0;
	m_LoadFbxAnim = 0;
	m_creatednum_boneupdatematrix = 0;//スレッド数の変化に対応。作成済の数。処理用。
	m_creatednum_loadfbxanim = 0;//スレッド数の変化に対応。作成済の数。処理用。



	m_loadbonecount = 0;

	::ZeroMemory(m_fbxfullname, sizeof(WCHAR) * MAX_PATH);
	m_useegpfile = false;

	m_hasbindpose = 1;

	//ZeroMemory(m_armpparams, sizeof(FUNCMPPARAMS*) * 6);
	//ZeroMemory(m_arhthread, sizeof(HANDLE) * 6);

	m_physicsikcnt = 0;
	ChaMatrixIdentity(&m_worldmat);
	m_modelposition = ChaVector3(0.0f, 0.0f, 0.0f);//読み込み時位置
	m_modelrotation = ChaVector3(0.0f, 0.0f, 0.0f);//読み込み時向き
	m_initaxismatx = 0;
	m_loadedflag = false;
	m_createbtflag = false;
	m_oldaxis_atloading = 0;
	m_ikrotaxis = ChaVector3( 1.0f, 0.0f, 0.0f );
	m_texpool = 0;
	m_tmpmotspeed = 1.0f;

	m_curreindex = 0;
	m_rgdindex = 0;
	m_rgdmorphid = -1;

	strcpy_s( m_defaultrename, MAX_PATH, "default_ref.ref" );
	strcpy_s( m_defaultimpname, MAX_PATH, "default_imp.imp" );

	m_rigidbone.clear();
	//m_btg = -1.0f;
	m_fbxobj.clear();
	m_btWorld = 0;

	m_modelno = 0;
	m_pdev = 0;

	::ZeroMemory( m_filename, sizeof( WCHAR ) * MAX_PATH );
	::ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
	::ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	m_loadmult = 1.0f;

	ChaMatrixIdentity( &m_matWorld );
	ChaMatrixIdentity( &m_matVP );

	m_curmotinfo = 0;

	m_modeldisp = true;
	m_topbone = 0;
	//m_firstbone = 0;

	this->m_tlFunc = 0;

	m_psdk = 0;
	m_pimporter = 0;
	m_pscene = 0;

	m_btcnt = 0;
	m_topbt = 0;

	m_rigideleminfo.clear();
	m_impinfo.clear();


	//g_motionbrush_startframe = 0.0;
	//g_motionbrush_endframe = 0.0;
	//g_motionbrush_numframe = 1;
	//g_motionbrush_value = 0;

	::ZeroMemory(m_setfl4x4, sizeof(float) * 16 * MAXCLUSTERNUM);//SetShaderConst用


	InitUndoMotion( 0 );

	return 0;
}
int CModel::DestroyObjs()
{
	InitUndoMotion(0);

	//スレッドを先に止める
	DestroyBoneUpdateMatrix();
	WaitUpdateMatrixFinished();
	DestroyLoadFbxAnim();
	WaitLoadFbxAnimFinished();
	DeleteCriticalSection(&m_CritSection_Node);//スレッド終了よりも後



	DestroyMaterial();
	DestroyObject();
	DestroyAncObj();
	DestroyAllMotionInfo();

	DestroyFBXSDK();

	DestroyBtObject();



	InitParams();

	return 0;
}



int CModel::DestroyFBXSDK()
{
	
	//if( m_pimporter ){
	
	//if(mAnimStackNameArray.Size() >= 1){
	//	FbxArrayDelete(mAnimStackNameArray);
	//}

	if (m_pimporter) {
		m_pimporter->Destroy();// インポータの削除
		m_pimporter = 0;
	}

	if( m_pscene ){
		m_pscene->Destroy();
		m_pscene = 0;
	}


	return 0;
}


int CModel::DestroyAllMotionInfo()
{
	map<int, MOTINFO*>::iterator itrmi;
	for( itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++ ){
		MOTINFO* miptr = itrmi->second;
		if( miptr ){
			free( miptr );
		}
	}
	m_motinfo.clear();

	m_curmotinfo = 0;

	return 0;
}

int CModel::DestroyMaterial()
{

	map<int, CMQOMaterial*>::iterator itr;
	for( itr = m_material.begin(); itr != m_material.end(); itr++ ){
		CMQOMaterial* delmat = itr->second;
		if( delmat ){
			delete delmat;
		}
	}
	m_material.clear();

	return 0;
}
int CModel::DestroyObject()
{

	map<int, CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* delobj = itr->second;
		if( delobj ){
			delete delobj;
		}
	}
	m_object.clear();

	return 0;
}

int CModel::DestroyAncObj()
{
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* delbone = itrbone->second;
		if( delbone ){
			//delete delbone;
			CBone::InvalidateBone(delbone);
		}
	}
	m_bonelist.clear();


	map<CMQOObject*, FBXOBJ*>::iterator itrobjindex;
	for (itrobjindex = m_fbxobj.begin(); itrobjindex != m_fbxobj.end(); itrobjindex++) {
		FBXOBJ* curfbxobj = itrobjindex->second;
		if (curfbxobj) {
			free(curfbxobj);
		}
	}
	m_fbxobj.clear();


	m_topbone = 0;

	m_objectname.clear();
	m_bonename.clear();

	return 0;
}

int CModel::LoadMQO( ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, const WCHAR* wfile, const WCHAR* modelfolder, float srcmult, int ismedia, int texpool )
{
	if( modelfolder ){
		wcscpy_s( m_modelfolder, MAX_PATH, modelfolder );
	}else{
		::ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	}
	m_loadmult = srcmult;
	m_pdev = pdev;
	m_texpool = texpool;

	WCHAR fullname[MAX_PATH];

	if( ismedia == 1 ){
		/*
		WCHAR str[MAX_PATH];
		HRESULT hr;
		hr = DXUTFindDXSDKMediaFileCch( str, MAX_PATH, wfile );
		if( hr != S_OK ){
			::MessageBoxA( NULL, "media not found error !!!", "load error", MB_OK );
			_ASSERT( 0 );
			return 1;
		}
		wcscpy_s( fullname, MAX_PATH, g_basedir );
		wcscat_s( fullname, MAX_PATH, str );
		*/
		_ASSERT(0);
		wcscpy_s(fullname, MAX_PATH, g_basedir);
		wcscat_s(fullname, MAX_PATH, wfile);

	}else{
		wcscpy_s( fullname, MAX_PATH, wfile );
	}

    WCHAR* strLastSlash = NULL;
    strLastSlash = wcsrchr( fullname, TEXT( '\\' ) );
    if( strLastSlash )
    {
		*strLastSlash = 0;
		wcscpy_s( m_dirname, MAX_PATH, fullname );
		wcscpy_s( m_filename, MAX_PATH, strLastSlash + 1 );
	}else{
		::ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
		wcscpy_s( m_filename, MAX_PATH, fullname );
	}

	SetCurrentDirectory( m_dirname );


	DestroyMaterial();
	DestroyObject();

	CMQOFile mqofile;
	ChaVector3 vop( 0.0f, 0.0f, 0.0f );
	ChaVector3 vor( 0.0f, 0.0f, 0.0f );
	CallF( mqofile.LoadMQOFile( m_pdev, srcmult, m_filename, vop, vor, this ), return 1 );

	CallF( MakeObjectName(), return 1 );


	CallF( CreateMaterialTexture(pd3dImmediateContext), return 1 );

	SetMaterialName();

	return 0;
}

int CModel::LoadFBX(int skipdefref, ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, const WCHAR* wfile, const WCHAR* modelfolder, float srcmult, FbxManager* psdk, FbxImporter** ppimporter, FbxScene** ppscene, int forcenewaxisflag, BOOL motioncachebatchflag)
{

	//DestroyFBXSDK();

	m_psdk = psdk;
	s_psdk = psdk;
	*ppimporter = 0;
	*ppscene = 0;

	if( modelfolder ){
		wcscpy_s( m_modelfolder, MAX_PATH, modelfolder );
	}else{
		::ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	}
	m_loadmult = srcmult;
	m_pdev = pdev;

	WCHAR fullname[MAX_PATH];
	wcscpy_s( m_fbxfullname, MAX_PATH, wfile );//save org fullpath
	wcscpy_s(fullname, MAX_PATH, wfile);

    WCHAR* strLastSlash = NULL;
    strLastSlash = wcsrchr( fullname, TEXT( '\\' ) );
    if( strLastSlash )
    {
		*strLastSlash = 0;
		wcscpy_s( m_dirname, MAX_PATH, fullname );
		wcscpy_s( m_filename, MAX_PATH, strLastSlash + 1 );
	}else{
		::ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
		wcscpy_s( m_filename, MAX_PATH, fullname );
	}

	DestroyBoneUpdateMatrix();
	DestroyMaterial();
	DestroyObject();

	char utf8path[MAX_PATH] = {0};
    // Unicode 文字コードを第一引数で指定した文字コードに変換する
    ::WideCharToMultiByte( CP_UTF8, 0, wfile, -1, utf8path, MAX_PATH, NULL, NULL );	

	FbxScene* pScene = 0;
	FbxImporter* pImporter = 0;

	char scenename[256];
	sprintf_s(scenename, "scene_%05d", s_alloccnt);
    pScene = FbxScene::Create(m_psdk, scenename);

    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    bool lStatus;
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	char importername[256];
	sprintf_s(importername, "importer_%d", s_alloccnt);
	pImporter = FbxImporter::Create(m_psdk, importername);

    const bool lImportStatus = pImporter->Initialize(utf8path, -1, m_psdk->GetIOSettings());
	if (!lImportStatus) {
		_ASSERT(0);
		return -1;
	}

	//if (s_animevaluator) {
	//	s_animevaluator->Destroy();
	//	s_animevaluator = 0;
	//}
	//s_animevaluator = FbxAnimEvaluator::Create(s_psdk, "animevaluator1");


	//Initialize(FbxStream * pStream, void* pStreamData = NULL, const int pFileFormat = -1, FbxIOSettings * pIOSettings = NULL)


	////###############################################################
	//// load from buffer using FbxStream CustomStreamClass 2021/05/06
	////###############################################################
	//FbxIOSettings* ios = FbxIOSettings::Create(m_psdk, IOSROOT);
	//CustomStreamClass stream(m_psdk, "rb", utf8path);
	//// can pass in a void* data pointer to be passed to the stream on FileOpen
	//void* streamData = NULL;
	//// initialize the importer with a stream
	//if (!pImporter->Initialize(&stream, streamData, -1, ios)) {
	//	_ASSERT(0);
	//	return -1;
	//}

    pImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (pImporter->IsFBX())
    {
        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL,        true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE,         true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_LINK,            true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE,           true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO,            true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION,       true);
        (*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
		//(*(m_psdk->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }



    // Import the scene.
    lStatus = pImporter->Import(pScene);
    if(lStatus == false )
    {
		_ASSERT( 0 );
		return 1;
	}

	FbxDocumentInfo* sceneinfo = pScene->GetSceneInfo();
	m_fbxcomment = sceneinfo->mComment;
	m_oldaxis_atloading = 0;
	if (forcenewaxisflag == 0){
		if (sceneinfo){
			FbxString oldauthor = "OpenRDB user";
			if (sceneinfo->mAuthor == oldauthor){
				_ASSERT(0);
				FbxString oldrevision = "rev. 1.0";
				if (sceneinfo->mRevision == oldrevision){
					m_oldaxis_atloading = 1;//!!!!!!!!!!!!!!!!!!!!
				}
			}

			FbxString bvhmark = "BVH animation";
			if (sceneinfo->mKeywords == bvhmark) {
				SetFromBvhFlag(true);
			}
			else {
				SetFromBvhFlag(false);
			}

			FbxString nobindposemark = "skinmesh animation, start from no bindpose fbx";
			if (sceneinfo->mKeywords == nobindposemark) {
				SetFromNoBindPoseFlag(true);
			}
			else {
				SetFromNoBindPoseFlag(false);
			}
		}
	}


//	CallF( InitFBXManager( &pSdkManager, &pImporter, &pScene, utf8path ), return 1 );

	m_bone2node.clear();
	FbxNode *pRootNode = pScene->GetRootNode();

	m_topbone = 0;

	
	CreateFBXBoneReq(pScene, pRootNode, 0, 0);
	if ((int)m_bonelist.size() <= 0){
		//_ASSERT( 0 );
		if (!m_bonelist.empty()) {
			CBone* delbone = (CBone*)(m_bonelist.begin()->second);
			if (delbone) {
				CBone::InvalidateBone(delbone);
			}
		}
		m_bonelist.clear();
		m_topbone = 0;
		//_ASSERT(0);
	}


	//WCHAR* findmqo = wcsstr((WCHAR*)this->GetFileName(), L".mqo");
	//if (findmqo == 0) {//mqo以外の場合には　m_topbone必要
		if (m_bonelist.empty()) {
			CBone* dummybone = CBone::GetNewBone(this);
			//_ASSERT( dummybone );
			if (dummybone) {
				dummybone->SetName("RootNode_");
				m_bonelist[0] = dummybone;

				dummybone->LoadCapsuleShape(m_pdev, pd3dImmediateContext);//!!!!!!!!!!
				m_topbone = dummybone;
			}
			//_ASSERT(0);
		}
	//}


	//CreateExtendBoneReq(m_topbone);

_ASSERT(m_bonelist[0]);


	CreateBoneUpdateMatrix();


	ChaMatrix firstmeshmat;
	firstmeshmat.SetIdentity();
	CreateFBXMeshReq(pRootNode);

	DbgOut(L"fbx bonenum %d\r\n", (int)m_bonelist.size());
_ASSERT(m_bonelist[0]);

	ChaMatrix offsetmat;
	ChaMatrixIdentity( &offsetmat );
	offsetmat.data[MATI_11] = srcmult;
	offsetmat.data[MATI_22] = srcmult;
	offsetmat.data[MATI_33] = srcmult;
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MultMat( offsetmat ), return 1 );
			CallF( curobj->MultVertex(), return 1; );
		}
	}

_ASSERT(m_bonelist[0]);

	//m_ktime0.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
//	m_ktime0.SetSecondDouble( 1.0 / 300.0 );
//	m_ktime0.SetSecondDouble( 1.0 / 30.0 );


	if (motioncachebatchflag == FALSE) {
		CallF(MakePolyMesh4(), return 1);
		CallF(MakeObjectName(), return 1);
		CallF(CreateMaterialTexture(pd3dImmediateContext), return 1);
		if (m_topbone) {
			CreateFBXSkinReq(pRootNode);
		}

		SetMaterialName();

		_ASSERT(m_bonelist[0]);

		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
			CBone* curbone = itrbone->second;
			if (curbone) {
				curbone->SetBtKinFlag(1);
			}
		}


		map<int, CMQOObject*>::iterator itr2;
		for (itr2 = m_object.begin(); itr2 != m_object.end(); itr2++) {
			CMQOObject* curobj = itr2->second;
			if (curobj) {
				char* findnd = strstr((char*)curobj->GetName(), "_ND");
				if (findnd) {
					curobj->SetDispFlag(0);
				}
			}
		}

		m_rigideleminfo.clear();
		m_impinfo.clear();

		if (skipdefref == 0) {
			REINFO reinfo;
			::ZeroMemory(&reinfo, sizeof(REINFO));
			strcpy_s(reinfo.filename, MAX_PATH, m_defaultrename);
			reinfo.btgscale = 9.07f;
			m_rigideleminfo.push_back(reinfo);
			m_impinfo.push_back(m_defaultimpname);

			if (m_topbone) {
				CreateRigidElemReq(m_topbone, 1, m_defaultrename, 1, m_defaultimpname);
			}

			SetCurrentRigidElem(0);
			m_curreindex = 0;
			m_curimpindex = 0;
		}
	}

	*ppimporter = pImporter;
	*ppscene = pScene;

	//m_pimporter = pImporter;
	m_pscene = pScene;

	if (s_animevaluator) {
		s_animevaluator->Destroy();
		s_animevaluator = 0;
	}
	pImporter->Destroy();


	return 0;
}

int CModel::LoadFBXAnim( FbxManager* psdk, FbxImporter* pimporter, FbxScene* pscene, int (*tlfunc)( int srcmotid ), BOOL motioncachebatchflag)
{
	if( !psdk || !pimporter || !pscene ){
		_ASSERT( 0 );
		return 0;
	}

	if( !m_topbone ){
		return 0;
	}

	this->m_tlFunc = tlfunc;//未使用


	FbxNode *pRootNode = pscene->GetRootNode();
	CallF( CreateFBXAnim( pscene, pRootNode, motioncachebatchflag ), return 1 );



	/*
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if (curbone){
			curbone->CalcAxisMat(1, 0.0f);
		}
	}
	*/
	//CalcBtAxismatReq(m_topbone, 1);


	return 0;
}

int CModel::CreateMaterialTexture(ID3D11DeviceContext* pd3dImmediateContext)
{

	map<int,CMQOMaterial*>::iterator itr;
	for( itr = m_material.begin(); itr != m_material.end(); itr++ ){
		CMQOMaterial* curmat = itr->second;
		CallF( curmat->CreateTexture( pd3dImmediateContext, m_dirname, m_texpool ), return 1 );
	}


	map<int,CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( curobj ){
			map<int,CMQOMaterial*>::iterator itrmat;
			for( itrmat = curobj->GetMaterialBegin(); itrmat != curobj->GetMaterialEnd(); itrmat++ ){
				CMQOMaterial* curmat = itrmat->second;
				CallF( curmat->CreateTexture( pd3dImmediateContext, m_dirname, m_texpool ), return 1 );
			}
		}
	}

	return 0;
}

int CModel::OnRender(bool withalpha, 
	ID3D11DeviceContext* pd3dImmediateContext, int lightflag, ChaVector4 diffusemult, int btflag )
{
	ChaVector4 materialdisprate = GetMaterialDispRate();

	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj && curobj->GetDispFlag() ){
			if( curobj->GetDispObj() ){

				CMQOMaterial* rmaterial = 0;
				if( curobj->GetPm3() ){
					bool found_alpha = false;
					bool found_noalpha = false;
					int blno;
					for (blno = 0; blno < curobj->GetPm3()->GetOptMatNum(); blno++) {
						MATERIALBLOCK* currb = curobj->GetPm3()->GetMatBlock() + blno;
						CMQOMaterial* curmat;
						curmat = currb->mqomat;
						if (!curmat) {
							continue;
						}
						if ((curmat->GetDif4F().w * diffusemult.w) <= 0.99999f) {
							found_alpha = true;
						}
						else {
							found_noalpha = true;
						}
					}
					if ((withalpha == false) && (found_noalpha == false)) {
						//不透明描画時　１つも不透明がなければ　レンダースキップ
						continue;
					}
					if ((withalpha == true) && (found_alpha == false)) {
						//半透明描画時　１つも半透明がなければ　レンダースキップ
						continue;
					}

					CallF(SetShaderConst(curobj, btflag), return 1);
					CallF( curobj->GetDispObj()->RenderNormalPM3(withalpha, pd3dImmediateContext, lightflag, diffusemult, materialdisprate ), return 1 );
				}
				else if (curobj->GetPm4()){
					bool found_alpha = false;
					bool found_noalpha = false;
					std::map<int, CMQOMaterial*>::iterator itrmaterial;
					for (itrmaterial = curobj->GetMaterialBegin(); itrmaterial != curobj->GetMaterialEnd(); itrmaterial++) {
						CMQOMaterial* curmaterial = itrmaterial->second;
						if (curmaterial) {
							if ((curmaterial->GetDif4F().w * diffusemult.w) <= 0.99999f) {
								found_alpha = true;
							}
							else {
								found_noalpha = true;
							}
						}
					}
					if ((withalpha == false) && (found_noalpha == false)) {
						//不透明描画時　１つも不透明がなければ　レンダースキップ
						continue;
					}
					if ((withalpha == true) && (found_alpha == false)) {
						//半透明描画時　１つも半透明がなければ　レンダースキップ
						continue;
					}
					CallF(SetShaderConst(curobj, btflag), return 1);
					rmaterial = curobj->GetMaterialBegin()->second;
					CallF( curobj->GetDispObj()->RenderNormal(withalpha, pd3dImmediateContext, rmaterial, lightflag, diffusemult, materialdisprate ), return 1 );
				}else{
					_ASSERT( 0 );
				}
			}
			if (curobj->GetDispLine()) {
				if ((withalpha == false) && ((curobj->GetExtLine()->m_color.w * diffusemult.w) > 0.99999f)) {
					CallF(curobj->GetDispLine()->RenderLine(withalpha, pd3dImmediateContext, diffusemult, materialdisprate), return 1);
				}
				else if ((withalpha == true) && (((curobj->GetExtLine()->m_color.w * diffusemult.w) <= 0.99999f))) {
					CallF(curobj->GetDispLine()->RenderLine(withalpha, pd3dImmediateContext, diffusemult, materialdisprate), return 1);
				}
			}
		}
	}

	return 0;
}

int CModel::GetModelBound( MODELBOUND* dstb )
{
	MODELBOUND mb;
	MODELBOUND addmb;
	::ZeroMemory( &mb, sizeof( MODELBOUND ) );

	int calcflag = 0;
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj->GetPm3() ){
			curobj->GetPm3()->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->GetPm3()->GetBound();
			}else{
				addmb = curobj->GetPm3()->GetBound();
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
		if( curobj->GetPm4() ){
			curobj->GetPm4()->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->GetPm4()->GetBound();
			}else{
				addmb = curobj->GetPm4()->GetBound();
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
		if( curobj->GetExtLine() ){
			curobj->GetExtLine()->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->GetExtLine()->m_bound;
			}else{
				addmb = curobj->GetExtLine()->m_bound;
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
	}

	if (GetTopBone()) {
		if (calcflag == 0) {
			mb = CalcBoneBound();
		}
		else {
			addmb = CalcBoneBound();
			AddModelBound(&mb, &addmb);
		}
		calcflag++;
	}


	*dstb = mb;

	return 0;
}

MODELBOUND CModel::CalcBoneBound()
{
	MODELBOUND mb;
	::ZeroMemory(&mb, sizeof(MODELBOUND));

	ChaVector3 min = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 max = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 center = ChaVector3(0.0f, 0.0f, 0.0f);
	float r = 0.0f;

	bool firstflag = true;

	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		CBone* curbone = itrbone->second;
		if (curbone) {

			//ボーンの初期位置は全て０の場合があるので、０フレームのアニメ付きのボーン位置を計算する
			ChaVector3 fpos0 = curbone->GetJointFPos();
			MOTINFO* mi = GetCurMotInfo();
			ChaVector3 fpos = fpos0;
			if (mi) {
				ChaMatrix curwmat;
				CMotionPoint* curmp = curbone->GetMotionPoint(mi->motid, 0.0);
				if (curmp) {
					curwmat = curmp->GetWorldMat();
				}
				else {
					curwmat.SetIdentity();
				}
				ChaVector3TransformCoord(&fpos, &fpos0, &curwmat);
			}


			if (firstflag == true) {
				min = fpos;
				max = fpos;

				firstflag = false;
			}
			else {
				if (fpos.x < min.x) {
					min.x = fpos.x;
				}
				if (fpos.y < min.y) {
					min.y = fpos.y;
				}
				if (fpos.z < min.z) {
					min.z = fpos.z;
				}

				if (fpos.x > max.x) {
					max.x = fpos.x;
				}
				if (fpos.y > max.y) {
					max.y = fpos.y;
				}
				if (fpos.z > max.z) {
					max.z = fpos.z;
				}
			}
		}
	}

	center = (min + max) * 0.5f;
	ChaVector3 diff = center - min;
	r = (float)ChaVector3LengthDbl(&diff);

	mb.min = min;
	mb.max = max;
	mb.center = center;
	mb.r = r;

	return mb;
}

int CModel::AddModelBound( MODELBOUND* mb, MODELBOUND* addmb )
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



int CModel::SetShapeNoReq( CMQOFace** ppface, int facenum, int searchp, int shapeno, int* setfacenum )
{

	int fno;
	CMQOFace* findface[200];
	::ZeroMemory( findface, sizeof( CMQOFace* ) * 200 );
	int findnum = 0;

	for( fno = 0; fno < facenum; fno++ ){
		CMQOFace* curface = *( ppface + fno );
		if( curface->GetShapeNo() != -1 ){
			continue;
		}

		int chki;
		for( chki = 0; chki < curface->GetPointNum(); chki++ ){
			if( searchp == curface->GetIndex( chki ) ){
				if( findnum >= 200 ){
					_ASSERT( 0 );
					return 1;
				}
				curface->SetShapeNo( shapeno );
				findface[findnum] = curface;
				findnum++;
				break;
			}
		}
	}

	if( findnum > 0 ){
		(*setfacenum) += findnum;

		int findno;
		for( findno = 0; findno < findnum; findno++ ){
			CMQOFace* fface = findface[ findno ];
			int i;
			for( i = 0; i < fface->GetPointNum(); i++ ){
				int newsearch = fface->GetIndex( i );
				if( newsearch != searchp ){
					SetShapeNoReq( ppface, facenum, newsearch, shapeno, setfacenum );
				}
			}
		}
	}
	return 0;
}

int CModel::SetFaceOfShape( CMQOFace** ppface, int facenum, int shapeno, CMQOFace** ppface2, int setfacenum )
{
	int setno = 0;
	int fno;
	for( fno = 0; fno < facenum; fno++ ){
		CMQOFace* curface = *( ppface + fno );
		if( curface->GetShapeNo() == shapeno ){
			if( setno >= setfacenum ){
				_ASSERT( 0 );
				return 1;
			}
			*( ppface2 + setno ) = curface;
			setno++;
		}
	}

	_ASSERT( setno == setfacenum );

	return 0;
}

int CModel::MakeObjectName()
{
	map<int, CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( curobj ){
			char* nameptr = (char*)curobj->GetName();
			if (nameptr) {
				*(nameptr + 256 - 1) = 0;
				int sdefcmp, bdefcmp;
				sdefcmp = strncmp(nameptr, "sdef:", 5);
				bdefcmp = strncmp(nameptr, "bdef:", 5);
				if ((sdefcmp != 0) && (bdefcmp != 0)) {
					size_t leng = strlen(nameptr);
					if ((leng > 0) && (leng < 4098)) {
						string firstname(nameptr, nameptr + leng);
						m_objectname[firstname] = curobj;
						curobj->SetDispName(firstname);
					}
					else {
						_ASSERT(0);
						curobj->SetDispName("MakeObjecctName_Error");
					}
				}
				else {
					char* startptr = nameptr + 5;
					size_t leng = strlen(startptr);
					if ((leng > 0) && (leng < 4098)) {
						string firstname(startptr, startptr + leng);
						m_objectname[firstname] = curobj;
						curobj->SetDispName(firstname);
					}
					else {
						_ASSERT(0);
						curobj->SetDispName("MakeObjecctName_Error");
					}
				}
			}
		}
	}

	return 0;
}

int CModel::DbgDump()
{
	DbgOut( L"######start DbgDump %s\r\n", GetFileName() );

	DbgOut( L"Dump Bone And InfScope\r\n" );

	if( m_topbone ){
		DbgDumpBoneReq( m_topbone, 0 );
	}

	DbgOut( L"######end DbgDump\r\n" );


	return 0;
}

int CModel::DbgDumpBoneReq( CBone* boneptr, int broflag )
{
	char mes[1024];
	WCHAR wmes[1024];

	if( boneptr->GetParent() ){
		sprintf_s( mes, 1024, "\tboneno %d, bonename %s - parent %s\r\n", boneptr->GetBoneNo(), boneptr->GetBoneName(), boneptr->GetParent()->GetBoneName() );
	}else{
		sprintf_s( mes, 1024, "\tboneno %d, bonename %s - parent NONE\r\n", boneptr->GetBoneNo(), boneptr->GetBoneName() );
	}
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
	DbgOut( wmes );

	DbgOut( L"\t\tbonepos (%f, %f, %f), (%f, %f, %f)\r\n", 
		boneptr->GetJointWPos().x, boneptr->GetJointWPos().y, boneptr->GetJointWPos().z,
		boneptr->GetJointFPos().x, boneptr->GetJointFPos().y, boneptr->GetJointFPos().z );

	
	//DbgOut( L"\t\tinfscopenum %d\r\n", boneptr->m_isnum );
	//int isno;
	//for( isno = 0; isno < boneptr->m_isnum; isno++ ){
	//	CInfScope* curis = boneptr->m_isarray[ isno ];
	//	CBone* infbone = 0;
	//	if( curis->m_applyboneno >= 0 ){
	//		infbone = m_bonelist[ curis->m_applyboneno ];
	//		sprintf_s( mes, 1024, "\t\tInfScope %d, validflag %d, facenum %d, applybone %s\r\n", 
	//			isno, curis->m_validflag, curis->m_facenum, infbone->m_bonename );
	//		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
	//		DbgOut( wmes );
	//	}else{
	//		DbgOut( L"\t\tInfScope %d, validflag %d, facenum %d, applybone is none\r\n", 
	//			isno, curis->m_validflag, curis->m_facenum );
	//	}
	//
	//	if( curis->m_targetobj ){
	//		sprintf_s( mes, 1024, "\t\t\ttargetobj %s\r\n", curis->m_targetobj->m_name );
	//		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
	//		DbgOut( wmes );
	//	}else{
	//		DbgOut( L"\t\t\ttargetobj is none\r\n" );
	//	}
	//}

//////////
	if( boneptr->GetChild() ){
		DbgDumpBoneReq( boneptr->GetChild(), 1 );
	}
	if( (broflag == 1) && boneptr->GetBrother() ){
		DbgDumpBoneReq( boneptr->GetBrother(), 1 );
	}



	return 0;
}

int CModel::MakePolyMesh3()
{
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MakePolymesh3( m_pdev, m_material ), return 1 );
		}
	}

	return 0;
}
int CModel::MakePolyMesh4()
{
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MakePolymesh4( m_pdev ), return 1 );
		}
	}

	return 0;
}

int CModel::MakeExtLine()
{
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MakeExtLine(), return 1 );
		}
	}

	return 0;
}

int CModel::MakeDispObj()
{
	int hasbone;
	if( m_bonelist.empty() ){
		hasbone = 0;
	}else{
		hasbone = 1;
	}

	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MakeDispObj( m_pdev, m_material, hasbone ), return 1 );
		}
	}

	return 0;
}

int CModel::Motion2Bt(bool limitdegflag, int firstflag, double nextframe, ChaMatrix* mW, ChaMatrix* mVP, int selectboneno )
{
	UpdateMatrix(limitdegflag, mW, mVP);


	if (!m_topbt){
		return 0;
	}
	if (!m_btWorld){
		return 0;
	}


	//if( m_topbt ){
	//	if (g_previewFlag != 5){
	//		//SetBtKinFlagReq(m_topbt, 0);
	//	}
	//	else{
	//		//if (GetBtCnt() <= 10) {
	//		//	SetKinematicFlag();
	//		//	//SetBtEquilibriumPoint();
	//		//}
	//		//else {
	//		//	SetRagdollKinFlagReq(m_topbt, selectboneno);
	//		//}
	//	}
	//}

	//if (firstflag == 1){
	//	map<int, CBone*>::iterator itrbone;
	//	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
	//		CBone* boneptr = itrbone->second;
	//		if (boneptr){
	//			//boneptr->SetStartMat2(boneptr->GetCurMp().GetWorldMat());
	//			boneptr->SetStartMat2(boneptr->GetCurrentZeroFrameMat(0));
	//		}
	//	}
	//}

	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
		CBone* boneptr = itrbone->second;
		if (boneptr && boneptr->GetParent()){
			CRigidElem* curre = boneptr->GetParent()->GetRigidElem(boneptr);
			if (curre){
				boneptr->GetParent()->CalcRigidElemParams(boneptr, firstflag);
			}
		}

		/*
		if (boneptr){
			std::map<CBone*, CRigidElem*>::iterator itrtmpmap;
			for (itrtmpmap = boneptr->GetRigidElemMapBegin(); itrtmpmap != boneptr->GetRigidElemMapEnd(); itrtmpmap++){
				CRigidElem* curre = itrtmpmap->second;
				if (curre){
					CBone* childbone = itrtmpmap->first;
					_ASSERT(childbone);
					if (childbone){
						boneptr->CalcRigidElemParams(childbone, firstflag);
					}
				}
			}
		}
		*/
	}


	Motion2BtReq(m_topbt);

	/*
	//map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
		CBone* curbone = itrbone->second;
		if (curbone){
			if ((curbone->GetBtKinFlag() == 0) && curbone->GetParent() && (curbone->GetParent()->GetBtKinFlag() == 1)){
				map<CBone*, CBtObject*>::iterator itrbto;
				for (itrbto = curbone->GetBtObjectMapBegin(); itrbto != curbone->GetBtObjectMapEnd(); itrbto++){
					CBtObject* curbto = itrbto->second;
					if (curbto){
						CBtObject* parbto = itrbto->second->GetParBt();
						if (parbto){
							int constraintnum = parbto->GetConstraintSize();
							int cno;
							for (cno = 0; cno < constraintnum; cno++){
								CONSTRAINTELEM ce = parbto->GetConstraintElem(cno);
								if (ce.centerbone == curbone){
									btGeneric6DofSpringConstraint* dofC = ce.constraint;
									if (dofC){
										int dofid;
										for (dofid = 0; dofid < 3; dofid++){
											dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
											//dofC->enableSpring(dofid, true);
										}
										for (dofid = 3; dofid < 6; dofid++){
											//dofC->enableSpring(dofid, true);
											dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
										}
										//dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
										//dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));

										//ChaVector3 bonepos = parbto->Getparentbone()->GetBtchildpos();
										//ChaVector3 bonepos = curbone->GetParent()->GetBtchildpos();
										ChaVector3 bonepos = curbone->GetBtparentpos();

										dofC->setLinearLowerLimit(btVector3(bonepos.x, bonepos.y, bonepos.z));
										dofC->setLinearUpperLimit(btVector3(bonepos.x, bonepos.y, bonepos.z));
									}
								}
							}
						}
					}
				}
			}
		}
	}
	*/

	//if (g_previewFlag == 5){
	//	if (m_topbt){
	//		SetBtEquilibriumPointReq(m_topbt);
	//	}
	//}


	return 0;
}

void CModel::Motion2BtReq( CBtObject* srcbto )
{
	//if( srcbto->GetBone() && (srcbto->GetBone()->GetBtKinFlag() == 1) ){
	if (srcbto->GetBone()){
		MOTINFO* curmi = GetCurMotInfo();
		if (curmi) {
			srcbto->Motion2Bt(this, curmi->motid, curmi->curframe);
		}
		
	}

	int chilnum = srcbto->GetChildBtSize();
	int chilno;
	for( chilno = 0; chilno < chilnum; chilno++ ){
		Motion2BtReq( srcbto->GetChildBt( chilno ) );
	}
}

//void CModel::CalcLimitedEulAfterThreadReq(CBone* srcbone, int srcmotid, double srcframe)
//{
//	//スレッド後処理用
//	//CalcWorldMatAfterThreadReqで計算した　limitedwmを元に
//	//limitedeulを計算する
//
//	if (srcbone) {
//		ChaVector3 limitedeul = ChaVector3(0.0f, 0.0f, 0.0f);
//		limitedeul = srcbone->CalcLocalLimitedEulXYZ(srcmotid, srcframe);
//		srcbone->SetLimitedLocalEul(srcmotid, srcframe, limitedeul);
//
//		if (srcbone->GetBrother()) {
//			CalcLimitedEulAfterThreadReq(srcbone->GetBrother(), srcmotid, srcframe);
//		}
//		if (srcbone->GetChild()) {
//			CalcLimitedEulAfterThreadReq(srcbone->GetChild(), srcmotid, srcframe);
//		}
//	}
//}



//void CModel::CalcWorldMatAfterThreadReq(CBone* srcbone, int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	if (srcbone) {
//
//
//		srcbone->CalcWorldMatAfterThread(srcmotid, srcframe, wmat, vpmat);
//
//
//		if (srcbone->GetBrother()) {
//			CalcWorldMatAfterThreadReq(srcbone->GetBrother(), srcmotid, srcframe, wmat, vpmat);
//		}
//		if (srcbone->GetChild()) {
//			CalcWorldMatAfterThreadReq(srcbone->GetChild(), srcmotid, srcframe, wmat, vpmat);
//		}
//	}
//}

int CModel::SwapCurrentMotionPoint()
{
	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		CBone* curbone = itrbone->second;
		if (curbone) {
			curbone->SwapCurrentMotionPoint();
		}
	}
	return 0;
}

int CModel::UpdateMatrix(bool limitdegflag, ChaMatrix* wmat, ChaMatrix* vpmat, bool needwaitfinished) // default : needwaitfinished = false
{
	m_matWorld = *wmat;
	m_matVP = *vpmat;

	if( !m_curmotinfo ){
		return 0;//!!!!!!!!!!!!
	}


	//if (m_topbone) {
	//	m_topbone->InitAddLimitQAll();
	//}

	int curmotid = m_curmotinfo->motid;
	double curframe = m_curmotinfo->curframe;


	//if ((m_boneupdatematrix != NULL) && (m_bonelist.size() >= (m_creatednum_boneupdatematrix * 4))) {
	if ((m_boneupdatematrix != NULL) && (m_bonelist.size() >= (m_creatednum_boneupdatematrix * 2))) {

		//WaitUpdateMatrixFinished();//needwaitfinishedがfalseのときにも必要
		//SwapCurrentMotionPoint();//<--- この方式は角度制限を有効にしたときに顕著にぎくしゃくするのでやめた


		//2023/02/02
		//limitdegflag == true時にも　UpdateMatrixは計算済を補間するだけなので　再帰呼び出ししなくてOKになった
		//limitdegflag == true時にも　UpdateMatrixでオイラー角もセットされるようになった(スレッド後処理は不要)
		int updatecount;
		for (updatecount = 0; updatecount < m_creatednum_boneupdatematrix; updatecount++) {
			CThreadingUpdateMatrix* curupdate = m_boneupdatematrix + updatecount;
			curupdate->UpdateMatrix(limitdegflag, curmotid, curframe, wmat, vpmat);
		}

		////if (needwaitfinished) {//<--- この方式は角度制限を有効にしたときに顕著にぎくしゃくするのでやめた
			WaitUpdateMatrixFinished();
		////}

		//if (g_limitdegflag == true) {
		//	CalcWorldMatAfterThreadReq(m_topbone, curmotid, curframe, wmat, vpmat);//curframe : 時間補間有り
		//	//CalcLimitedEulAfterThreadReq(m_topbone, curmotid, curframe);
		//}
	}
	else {
		//map<int, CBone*>::iterator itrbone;
		//for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		//	CBone* curbone = itrbone->second;
		//	if( curbone ){
		//		curbone->UpdateMatrix( curmotid, curframe, wmat, vpmat );
		//	}
		//}
		UpdateMatrixReq(limitdegflag, GetTopBone(), curmotid, curframe, wmat, vpmat);
	}







	//if (m_topbone) {
	//	m_topbone->RotQAddLimitQAll(curmotid, curframe);
	//}


	/*
	// for morph anim

	//groundのUpdateMatrixでエラー


	int chkcnt = 0;
	int motionorder = -1;
	map<int, MOTINFO*>::iterator itrmi;
	for( itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++ ){
		MOTINFO* chkmi = itrmi->second;
		if( chkmi ){
			if( chkmi->motid == m_curmotinfo->motid ){
				motionorder = chkcnt;
				break;
			}
		}
		chkcnt++;
	}
	if( motionorder < 0 ){
		_ASSERT( 0 );
		return 1;
	}


	//読み込み時にアニメがなければ以下はスキップ
	const int lAnimStackCount = mAnimStackNameArray.GetCount();
	if (lAnimStackCount <= 0){
		//_ASSERT(0);
		return 0;
	}

	FbxAnimStack * lCurrentAnimationStack = m_pscene->FindMember<FbxAnimStack>(mAnimStackNameArray[motionorder]->Buffer());
	if (lCurrentAnimationStack == NULL){
		_ASSERT( 0 );
		return 1;
	}
	FbxAnimLayer * mCurrentAnimLayer;
	mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
   

	FbxTime lTime;
	lTime.SetSecondDouble( m_curmotinfo->curframe / 30.0 );
	//lTime.SetSecondDouble( m_curmotinfo->curframe / 300.0 );

	map<int, CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		_ASSERT( curobj );
		if( !(curobj->EmptyFindShape()) ){
			GetShapeWeight( m_fbxobj[curobj].node, m_fbxobj[curobj].mesh, lTime, mCurrentAnimLayer, curobj );

			CallF( curobj->UpdateMorphBuffer(), return 1 );
		}
	}
	*/

	return 0;
}

void CModel::UpdateMatrixReq(bool limitdegflag, CBone* srcbone, int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat)
{
	if (srcbone) {

		srcbone->UpdateMatrix(limitdegflag, srcmotid, srcframe, wmat, vpmat);

		if (srcbone->GetChild()) {
			UpdateMatrixReq(limitdegflag, srcbone->GetChild(), srcmotid, srcframe, wmat, vpmat);
		}
		if (srcbone->GetBrother()) {
			UpdateMatrixReq(limitdegflag, srcbone->GetBrother(), srcmotid, srcframe, wmat, vpmat);
		}
	}
}

int CModel::CopyLimitedWorldToWorldOne(CBone* srcbone, int srcmotid, double srcframe)
{
	if (srcbone) {
		srcbone->CopyLimitedWorldToWorld(srcmotid, srcframe);
	}
	return 0;
}

void CModel::CopyLimitedWorldToWorldReq(CBone* srcbone, int srcmotid, double srcframe)
{
	if (srcbone) {

		srcbone->CopyLimitedWorldToWorld(srcmotid, srcframe);

		if (srcbone->GetChild()) {
			CopyLimitedWorldToWorldReq(srcbone->GetChild(), srcmotid, srcframe);
		}
		if (srcbone->GetBrother()) {
			CopyLimitedWorldToWorldReq(srcbone->GetBrother(), srcmotid, srcframe);
		}
	}
}


void CModel::CopyWorldToLimitedWorldReq(CBone* srcbone, int srcmotid, double srcframe)
{
	if (srcbone) {

		srcbone->CopyWorldToLimitedWorld(srcmotid, srcframe);

		if (srcbone->GetChild()) {
			CopyWorldToLimitedWorldReq(srcbone->GetChild(), srcmotid, srcframe);
		}
		if (srcbone->GetBrother()) {
			CopyWorldToLimitedWorldReq(srcbone->GetBrother(), srcmotid, srcframe);
		}
	}
}


void CModel::ApplyNewLimitsToWMReq(CBone* srcbone, int srcmotid, double srcframe)
{

	//2023/02/03
	//この関数を実行する前に　CopyWorldToLimitedWorldを実行しておく

	if (srcbone) {

		srcbone->ApplyNewLimitsToWM(srcmotid, srcframe);

		if (srcbone->GetChild()) {
			ApplyNewLimitsToWMReq(srcbone->GetChild(), srcmotid, srcframe);
		}
		if (srcbone->GetBrother()) {
			ApplyNewLimitsToWMReq(srcbone->GetBrother(), srcmotid, srcframe);
		}
	}
}



int CModel::ClearLimitedWM(int srcmotid, double srcframe)
{
	if (!m_curmotinfo) {
		return 0;//!!!!!!!!!!!!
	}

	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		CBone* curbone = itrbone->second;
		if (curbone) {
			curbone->ClearLimitedWorldMat(srcmotid, srcframe);
		}
	}

	return 0;
}


int CModel::HierarchyRouteUpdateMatrix(bool limitdegflag, CBone* srcbone, ChaMatrix* wmat, ChaMatrix* vpmat)
{
	m_matWorld = *wmat;
	m_matVP = *vpmat;

	if (!m_curmotinfo) {
		return 0;//!!!!!!!!!!!!
	}

	if (!srcbone) {
		return 0;
	}

	std::vector<CBone*> vecroute;
	vecroute.clear();
	
	CBone* routebone = srcbone;
	while (routebone) {
		vecroute.push_back(routebone);
		routebone = routebone->GetParent();
	}
	std::reverse(vecroute.begin(), vecroute.end());

	int curmotid = m_curmotinfo->motid;
	double curframe = m_curmotinfo->curframe;

	std::vector<CBone*>::iterator itrbone;
	for (itrbone = vecroute.begin(); itrbone != vecroute.end(); itrbone++) {
		CBone* curbone = *itrbone;
		if (curbone) {
			curbone->UpdateMatrix(limitdegflag, curmotid, curframe, wmat, vpmat);
		}
	}

	return 0;

}



/***
int CModel::ComputeShapeDeformation(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename )
{
    int lVertexCount = pMesh->GetControlPointsCount();
	if( lVertexCount != curobj->m_vertex ){
		_ASSERT( 0 );
		return 1;
	}

	MoveMemory( curobj->m_mpoint, curobj->m_pointbuf, sizeof( ChaVector3 ) * lVertexCount );

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; lChannelIndex++)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if(lChannel)
			{
				// Get the percentage of influence of the shape.
				FbxAnimCurve* lFCurve;
				double lWeight = 0.0;
				lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
				if (lFCurve){
					lWeight = lFCurve->Evaluate(pTime);
				}else{
					continue;
				}

				if( lWeight == 0.0 ){
					continue;
				}

				//Find which shape should we use according to the weight.
				int lShapeCount = lChannel->GetTargetShapeCount();
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();
				for(int lShapeIndex = 0; lShapeIndex<lShapeCount; lShapeIndex++)
				{
					FbxShape* lShape = NULL;
					lShape = lChannel->GetTargetShape(lShapeIndex);//lShapeIndex+1ではない！！！！！！！！！！！！！！！！
					if(lShape)
					{		
						FbxVector4* shapev = lShape->GetControlPoints();

						for (int j = 0; j < lVertexCount; j++)
						{
							// Add the influence of the shape vertex to the mesh vertex.
							ChaVector3 xv;
							ChaVector3 diffpoint;

							xv.x = (float)shapev[j][0];
							xv.y = (float)shapev[j][1];
							xv.z = (float)shapev[j][2];

							diffpoint = (xv - *(curobj->m_pointbuf + j)) * (float)lWeight * 0.01f;

							*(curobj->m_mpoint + j) += diffpoint;
						}						
					}
				}//For each target shape
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	return 0;
}
***/

int CModel::GetFBXShape( FbxMesh* pMesh, CMQOObject* curobj, FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep )
{
	int lVertexCount = pMesh->GetControlPointsCount();
	if( lVertexCount != curobj->GetVertex() ){
		_ASSERT( 0 );
		return 1;
	}

	curobj->DestroyShapeObj();

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; lChannelIndex++)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if(lChannel)
			{
				FbxTime curtime = starttime;
				double framecnt;
				for( framecnt = 0.0; framecnt < animleng; framecnt+=1.0 ){
					FbxAnimCurve* lFCurve;
					double lWeight = 0.0;
					lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, panimlayer);
					if (lFCurve){
						lWeight = lFCurve->Evaluate( curtime );
					}else{
						curtime += timestep;
						continue;
					}
					if( lWeight == 0.0 ){
						curtime += timestep;
						continue;
					}
						
					int lShapeIndex = 0;
					FbxShape* lShape = NULL;
					lShape = lChannel->GetTargetShape(lShapeIndex);//lShapeIndex+1ではない！！！！！！！！！！！！！！！！
					if(lShape)
					{		
						//const char* nameptr = lChannel->GetName();
						int existshape = 0;
						existshape = curobj->ExistShape( (char*)lChannel->GetName());
						if( existshape == 0 ){

							curobj->AddShapeName( (char*)lChannel->GetName());

							FbxVector4* shapev = lShape->GetControlPoints();
							_ASSERT( shapev );
							for (int j = 0; j < lVertexCount; j++)
							{
								ChaVector3 xv;
								xv.x = (float)shapev[j][0];
								xv.y = (float)shapev[j][1];
								xv.z = (float)shapev[j][2];
								curobj->SetShapeVert( (char*)lChannel->GetName(), j, xv );
							}						
						}
					}
					curtime += timestep;
				}
			}//If lChannel is validf
		}//For each blend shape channel
	}//For each blend shape deformer

	return 0;
}

// Deform the vertex array with the shapes contained in the mesh.
int CModel::GetShapeWeight(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj )
{
    int lVertexCount = pMesh->GetControlPointsCount();
	if( lVertexCount != curobj->GetVertex() ){
		_ASSERT( 0 );
		return 1;
	}

	curobj->InitShapeWeight();

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; lChannelIndex++)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if(lChannel)
			{

				//const char* nameptr = lChannel->GetName();
				// Get the percentage of influence of the shape.
				FbxAnimCurve* lFCurve;
				double lWeight = 0.0;
				lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
				if (lFCurve){
					lWeight = lFCurve->Evaluate(pTime);
				}else{
					continue;
				}

				if( lWeight == 0.0 ){
					continue;
				}

				//Find which shape should we use according to the weight.
				int lShapeCount = lChannel->GetTargetShapeCount();
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();
				for(int lShapeIndex = 0; lShapeIndex < lShapeCount; lShapeIndex++)
				{
					FbxShape* lShape = NULL;
					lShape = lChannel->GetTargetShape(lShapeIndex);//lShapeIndex+1ではない！！！！！！！！！！！！！！！！
					if(lShape)
					{	
						char tmpname[512];
						strcpy_s(tmpname, 512, lChannel->GetName());
						curobj->SetShapeWeight( tmpname, (float)lWeight );
					}
				}//For each target shape
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	return 0;
}


int CModel::SetShaderConst( CMQOObject* srcobj, int btflag )
{

	if( !m_topbone ){
		//_ASSERT(0);
		return 0;//!!!!!!!!!!!
	}

	MOTINFO* curmi = 0;
	int curmotid;
	double curframe;
	curmi = GetCurMotInfo();
	if (!curmi) {
		_ASSERT(0);
		return 0;
	}
	curmotid = curmi->motid;
	curframe = (double)((int)(curmi->curframe + 0.0001));


	g_hmWorld->SetMatrix((float*)&(m_worldmat.data[MATI_11]));
	//g_pEffect->SetMatrix(g_hmWorld, &(m_worldmat.D3DX()));


	//ChaMatrix set4x4[MAXCLUSTERNUM];

	//float setfl4x4[16 * MAXCLUSTERNUM];//メンバーに
	
	//ZeroMemory(&(setfl4x4[0]), sizeof(float) * 16 * MAXCLUSTERNUM);

	//int inicnt;
	//for (inicnt = 0; inicnt < MAXCLUSTERNUM; inicnt++) {
	//	ChaMatrixIdentity(&(set4x4[inicnt]));
	//}
	////ZeroMemory( &set4x4[0], sizeof( ChaMatrix ) * MAXCLUSTERNUM );

	int setclcnt = 0;
	int clcnt;
	for( clcnt = 0; clcnt < (int)srcobj->GetClusterSize(); clcnt++ ){
		CBone* curbone = srcobj->GetCluster( clcnt );
		if( !curbone ){
			_ASSERT( 0 );
			return 1;
		}

		bool currentlimitdegflag = g_limitdegflag;
		CMotionPoint curmp = curbone->GetCurMp();




		//CMotionPoint tmpmp = curbone->GetCurMp();
		if( btflag == 0 ){
			//set4x4[clcnt] = tmpmp.GetWorldMat();
			MoveMemory(&(m_setfl4x4[16 * clcnt]), 
				&(curbone->GetWorldMat(currentlimitdegflag, curmotid, curframe, &curmp).data[MATI_11]), sizeof(float) * 16);
		}else if(btflag == 1){
			//物理シミュ
			//set4x4[clcnt] = curbone->GetBtMat();
			MoveMemory(&(m_setfl4x4[16 * clcnt]), 
				&(curbone->GetBtMat().data[MATI_11]), sizeof(float) * 16);
		}
		else if (btflag == 2) {
			//物理IK
			//set4x4[clcnt] = curbone->GetBtMat();
			MoveMemory(&(m_setfl4x4[16 * clcnt]), 
				&(curbone->GetBtMat().data[MATI_11]), sizeof(float) * 16);
		}
		else {
			//set4x4[clcnt] = tmpmp.GetWorldMat();
			MoveMemory(&(m_setfl4x4[16 * clcnt]), 
				&(curbone->GetWorldMat(currentlimitdegflag, curmotid, curframe, &curmp).data[MATI_11]), sizeof(float) * 16);
		}
		setclcnt++;
	}


	if(setclcnt > 0 ){
		_ASSERT(setclcnt <= MAXCLUSTERNUM);

		HRESULT hr = S_OK;
		hr = g_hm4x4Mat->SetMatrixArray((float*)(&m_setfl4x4[0]), 0, setclcnt);
		//hr = g_hm4x4Mat->SetRawValue((float*)(&setfl4x4[0]), 0, sizeof(float) * 16 * setclcnt);


		//hr = g_hm4x4Mat->SetMatrixArray((float*)(&set4x4[0]), 0, MAXCLUSTERNUM);

		//hr = g_hm4x4Mat->SetRawValue((float*)(&set4x4[0]), 0, sizeof(float) * 16 * MAXCLUSTERNUM);
		//hr = g_pEffect->SetValue(g_hm4x4Mat, (void*)set4x4, sizeof(float) * 16 * MAXCLUSTERNUM);
		
		//hr = g_hm4x4Mat->SetMatrixArray((float*)(&set4x4[0]), 0, MAXCLUSTERNUM);
		//hr = g_hm4x4Mat->SetRawValue((float*)(&set4x4[0]), 0, sizeof(float) * 16 * MAXCLUSTERNUM);
		//hr = g_hm4x4Mat->SetMatrixArray((float*)(&set4x4[0]), 0, MAXCLUSTERNUM);
		//hr = g_pEffect->SetValue( g_hm4x4Mat, (void*)set3x4, sizeof( float ) * 12 * MAXCLUSTERNUM );
		if(FAILED(hr)){
			_ASSERT( 0 );
			return 1;
		}
	}

	return 0;
}

int CModel::FillTimeLineOne(CBone* curbone, int lineno,
	OrgWinGUI::OWP_Timeline& timeline, 
	std::map<int, int>& lineno2boneno, std::map<int, int>& boneno2lineno)
{

	if (curbone) {
		int depth = curbone->CalcBoneDepth();
		bool ikstopflag = curbone->GetIKStopFlag();
		bool iktargetflag = curbone->GetIKTargetFlag();

		WCHAR dispname[JOINTNAMELENG];
		ZeroMemory(dispname, sizeof(WCHAR) * JOINTNAMELENG);
		if (curbone->GetWBoneName()) {
			size_t namelen = wcslen(curbone->GetWBoneName());
			size_t cplen;
			if (namelen < JOINTNAMELENG) {
				cplen = namelen;
			}
			else {
				
				_ASSERT(0);

				//タイムラインのnameとして　途中までしか登録されないので
				//タイムラインにおける名前での検索は　みつからなくなる
				cplen = JOINTNAMELENG - 1;
			}
			wcsncpy_s(dispname, JOINTNAMELENG, curbone->GetWBoneName(), cplen);
		}
		else {
			_ASSERT(0);
			wcscpy_s(dispname, JOINTNAMELENG, L"No Name");
		}

		//行を追加
		if (curbone->IsBranchBone()) {
			timeline.newLine(depth, 2, ikstopflag, iktargetflag, dispname);
		}
		else if (curbone->GetType() != FBXBONE_NULL) {
			timeline.newLine(depth, 0, ikstopflag, iktargetflag, dispname);
		}
		else {
			timeline.newLine(depth, 1, ikstopflag, iktargetflag, dispname);
		}

		timeline.setHasRigFlag(curbone->GetWBoneName(), ChkBoneHasRig(curbone));


		lineno2boneno[lineno] = curbone->GetBoneNo();
		boneno2lineno[curbone->GetBoneNo()] = lineno;
	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int CModel::FillTimeLine(OrgWinGUI::OWP_Timeline& timeline, map<int, int>& lineno2boneno, map<int, int>& boneno2lineno)
{
	lineno2boneno.clear();
	boneno2lineno.clear();

	if( m_bonelist.empty() ){
		return 0;
	}

	int lineno = 0;
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );

		if (curbone) {

			FillTimeLineOne(curbone, lineno,
				timeline,
				lineno2boneno, boneno2lineno);

			lineno++;
		}


/***
		_ASSERT( m_curmotinfo );
		CMotionPoint* curmp = curbone->m_motionkey[ m_curmotinfo->motid ];
		while( curmp ){
			timeline.newKey( curbone->m_wbonename, curmp->m_frame, (void*)curmp );
			curmp = curmp->m_next;
		}
***/
	}

/***
	int lineno = 0;
	FillTimelineReq( timeline, m_topbone, &lineno, lineno2boneno, boneno2lineno, 0 );
***/

	if (m_topbone){
		//選択行を設定
		timeline.setCurrentLineName(m_topbone->GetWBoneName());
	}
	return 0;
}

void CModel::FillTimelineReq( OrgWinGUI::OWP_Timeline& timeline, CBone* curbone, int* linenoptr, 
	map<int, int>& lineno2boneno, map<int, int>& boneno2lineno, int broflag )
{
	if (!curbone || !linenoptr){
		return;
	}

	FillTimeLineOne(curbone, *linenoptr,
		timeline,
		lineno2boneno, boneno2lineno);

	(*linenoptr)++;

/***
	_ASSERT( m_curmotinfo );
	CMotionPoint* curmp = curbone->m_motionkey[ m_curmotinfo->motid ];
	while( curmp ){
		timeline.newKey( curbone->m_wbonename, curmp->m_frame, (void*)curmp );
		curmp = curmp->m_next;
	}
***/

	if( curbone->GetChild() ){
		FillTimelineReq( timeline, curbone->GetChild(), linenoptr, lineno2boneno, boneno2lineno, 1 );
	}
	if( broflag && curbone->GetBrother() ){
		FillTimelineReq( timeline, curbone->GetBrother(), linenoptr, lineno2boneno, boneno2lineno, 1 );
	}
}

int CModel::AddMotion(const char* srcname, const WCHAR* wfilename, double srcleng, int* dstid)
{

	bool limitdegflagOnAddMotion = false;//読み込みに関しては　unlimitedWorld


	*dstid = -1;
	//int leng = (int)strlen(srcname);




	int maxid = 0;
	map<int, MOTINFO*>::iterator itrmi;
	for (itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++){
		MOTINFO* chkmi = itrmi->second;
		if (chkmi){
			if (maxid < chkmi->motid){
				maxid = chkmi->motid;
			}
		}
	}
	int newid = maxid + 1;


	MOTINFO* newmi = (MOTINFO*)malloc(sizeof(MOTINFO));
	if (!newmi){
		_ASSERT(0);
		return 1;
	}
	::ZeroMemory(newmi, sizeof(MOTINFO));

	strcpy_s(newmi->motname, 256, srcname);
	if (wfilename){
		wcscpy_s(newmi->wfilename, MAX_PATH, wfilename);
	}
	else{
		::ZeroMemory(newmi->wfilename, sizeof(WCHAR)* MAX_PATH);
	}
	::ZeroMemory(newmi->engmotname, sizeof(char)* 256);

	newmi->motid = newid;
	newmi->frameleng = srcleng;
	//newmi->curframe = 0.0;
	newmi->curframe = 1.0;
	newmi->speed = 1.0;
	newmi->loopflag = 1;

	m_motinfo[(int)m_motinfo.size()] = newmi;//2021/08/26 eraseすることがあるのでindex = motid - 1とは限らない




	//初期化　2022/08/28
	SetCurrentMotion(newid);
	
	
	//2023/02/11
	//この関数AddMotionにおいて
	//InitMPReqとCreateIndexedMotionPointReqは
	//fbxのモーションを読み込み済の場合に　新規モーション追加の際だけ意味がある
	// 
	//fbxAnim読み込み時は　姿勢データ読み込み後に　CreateIndexedMotionPointを呼ぶ(2023/02/11)
	// 
	//モーションを持たないfbxに対して　MotionPointを作成するのは　
	//Main.cppのInitCurMotion()から呼ばれるInitMPReqとCreateIndexedMotionPointReq

	if (GetLoadedFlag() == true) {
		double framecnt;
		for (framecnt = 0.0; framecnt < srcleng; framecnt += 1.0) {
			InitMPReq(limitdegflagOnAddMotion, m_topbone, newid, framecnt);//motionpointが無い場合は作成も
		}

		int errorcount = 0;
		CreateIndexedMotionPointReq(m_topbone, newid, srcleng, &errorcount);//2022/10/30
		if (errorcount != 0) {
			_ASSERT(0);
		}
	}

	*dstid = newid;

	return 0;
}


int CModel::SetCurrentMotion( int srcmotid )
{
	int motnum;
	motnum = (int)m_motinfo.size();
	//if ((srcmotid <= 0) || (srcmotid > motnum)) {
	//	_ASSERT(0);
	//	return 1;
	//}

	//m_curmotinfo = m_motinfo[ srcmotid - 1 ];//idは１から
	m_curmotinfo = GetMotInfo(srcmotid);//2021/08/26 eraseすることがあるのでindex = motid - 1とは限らない
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 1;
	}else{
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
			CBone* curbone = itrbone->second;
			if (curbone) {
				curbone->SetCurrentMotion(srcmotid, m_curmotinfo->frameleng);
			}
		}
		//ResetMotionCache();

		m_curmotinfo->curframe = 1.0;

		return 0;
	}
}
int CModel::SetMotionFrame( double srcframe )
{
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 1;
	}

	m_curmotinfo->curframe = max( 0.0, min( (m_curmotinfo->frameleng - 1), srcframe ) );

	return 0;
}
int CModel::GetMotionFrame( double* dstframe )
{
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 1;
	}

	*dstframe = m_curmotinfo->curframe;

	return 0;
}
int CModel::SetMotionSpeed( double srcspeed )
{
	if( !m_curmotinfo ){
		//_ASSERT( 0 );
		return 1;
	}

	m_curmotinfo->speed = srcspeed;

	return 0;
}
int CModel::GetMotionSpeed( double* dstspeed )
{
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 1;
	}

	*dstspeed = m_curmotinfo->speed;
	return 0;
}

int CModel::DeleteMotion( int motid )
{
	//##############################################################
	//削除によりmapのm_motinfo[]の添え字は変わるがmotidは変わらない
	//##############################################################

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CallF( curbone->DeleteMotion( motid ), return 1 );
		}
	}

	int miindex;
	miindex = MotionID2Index(motid);
	if (miindex >= 0) {
		MOTINFO* delmi = m_motinfo[miindex];
		if (delmi) {
			//delete delmi;
			free(delmi);
		}

		std::map<int, MOTINFO*> tmpmimap;
		tmpmimap.clear();

		int destindex = 0;
		int srcindex;
		for (srcindex = 0; srcindex < m_motinfo.size(); srcindex++) {
			if (srcindex != miindex) {
				tmpmimap[destindex] = m_motinfo[srcindex];
				destindex++;
			}
		}

		m_motinfo.clear();

		for (srcindex = 0; srcindex < tmpmimap.size(); srcindex++) {
			m_motinfo[srcindex] = tmpmimap[srcindex];
		}

	}

	int undono;
	for( undono = 0; undono < UNDOMAX; undono++ ){
		if( m_undomotion[undono].GetSaveMotInfo().motid == motid ){
			m_undomotion[undono].SetValidFlag( 0 );
		}
	}

	ResetMotionCache();

	return 0;
}

void CModel::ResetMotionCache()
{
	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		CBone* curbone = itrbone->second;
		if (curbone) {
			curbone->ResetMotionCache();
		}
	}
}

int CModel::GetSymBoneNo( int srcboneno, int* dstboneno, int* existptr )
{
	*existptr = 0;

	CBone* srcbone = m_bonelist[ srcboneno ];
	if( !srcbone ){
		*dstboneno = -1;
		return 0;
	}

	//int findflag = 0;
	//WCHAR findname[256];
	//ZeroMemory( findname, sizeof( WCHAR ) * 256 );
	//wcscpy_s( findname, 256, srcbone->GetWBoneName() );
	//WCHAR* lpat = wcsstr( findname, L"_L_" );
	//if( lpat ){
	//	*lpat = TEXT( '_' );
	//	*(lpat + 1) = TEXT( 'R' );
	//	*(lpat + 2) = TEXT( '_' );
	//	//wcsncat_s( findname, 256, L"_R_", 3 );
	//	findflag = 1;
	//}else{
	//	WCHAR* rpat = wcsstr( findname, L"_R_" );
	//	if( rpat ){
	//		*rpat = TEXT( '_' );
	//		*(rpat + 1) = TEXT( 'L' );
	//		*(rpat + 2) = TEXT( '_' );
	//		//wcsncat_s( findname, 256, L"_L_", 3 );
	//		findflag = 1;
	//	}
	//}
	//int setflag = 0;
	////if( findflag == 0 ){
	////	*dstboneno = srcboneno;
	////	*existptr = 0;
	////}else{
	//if (findflag != 0){
	//	CBone* dstbone = 0;
	//	map<int, CBone*>::iterator itrbone;
	//	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
	//		CBone* chkbone = itrbone->second;
	//		if( chkbone && (wcscmp( findname, chkbone->GetWBoneName() ) == 0) ){
	//			dstbone = chkbone;
	//			break;
	//		}
	//	}
	//	if( dstbone ){
	//		*dstboneno = dstbone->GetBoneNo();
	//		*existptr = 1;
	//		setflag = 1;
	//	}
	//	//else{
	//	//	*dstboneno = srcboneno;
	//	//	*existptr = 0;
	//	//}
	//}

	int setflag = 0;
	{
		string strcurbonename = srcbone->GetBoneName();
		string strLeft = "Left";
		string strRight = "Right";

		string chkLeft = strcurbonename;
		string chkRight = strcurbonename;

		std::string::size_type leftpos = chkLeft.find(strLeft);
		if (leftpos != std::string::npos) {
			//Leftの部分をRightに変えてボーンが存在すればそのボーンに移動
			chkLeft.replace(leftpos, strLeft.length(), strRight);
			CBone* rightbone = GetBoneByName(chkLeft.c_str());
			if (rightbone) {
				int nextboneno = rightbone->GetBoneNo();
				if (nextboneno >= 0) {
					*dstboneno = nextboneno;
					*existptr = 1;
					setflag = true;
				}
			}
		}
		else {
			std::string::size_type rightpos = chkRight.find(strRight);
			if (rightpos != std::string::npos) {
				//Rightの部分をLeftに変えてボーンが存在すればそのボーンに移動
				chkRight.replace(rightpos, strRight.length(), strLeft);
				CBone* leftbone = GetBoneByName(chkRight.c_str());
				if (leftbone) {
					int nextboneno = leftbone->GetBoneNo();
					if (nextboneno >= 0) {
						*dstboneno = nextboneno;
						*existptr = 1;
						setflag = true;
					}
				}
			}
		}
	}

	if(setflag == 0)
	{
		string strcurbonename = srcbone->GetBoneName();
		string strLeft = "_L_";
		string strRight = "_R_";

		string chkLeft = strcurbonename;
		string chkRight = strcurbonename;

		std::string::size_type leftpos = chkLeft.find(strLeft);
		if (leftpos != std::string::npos) {
			//_L_の部分を_R_に変えてボーンが存在すればそのボーンに移動
			chkLeft.replace(leftpos, strLeft.length(), strRight);
			CBone* rightbone = GetBoneByName(chkLeft.c_str());
			if (rightbone) {
				int nextboneno = rightbone->GetBoneNo();
				if (nextboneno >= 0) {
					*dstboneno = nextboneno;
					*existptr = 1;
					setflag = true;
				}
			}
		}
		else {
			std::string::size_type rightpos = chkRight.find(strRight);
			if (rightpos != std::string::npos) {
				//_R_の部分を_L_に変えてボーンが存在すればそのボーンに移動
				chkRight.replace(rightpos, strRight.length(), strLeft);
				CBone* leftbone = GetBoneByName(chkRight.c_str());
				if (leftbone) {
					int nextboneno = leftbone->GetBoneNo();
					if (nextboneno >= 0) {
						*dstboneno = nextboneno;
						*existptr = 1;
						setflag = true;
					}
				}
			}
		}
	}


	if (setflag == 0){
		CBone* symposbone = GetSymPosBone(srcbone);
		if (symposbone){
			*dstboneno = symposbone->GetBoneNo();
			*existptr = 1;
		}
		else{
			*dstboneno = srcboneno;
			*existptr = 0;
		}
	}


	return 0;
}

CBone* CModel::GetSymPosBone(CBone* srcbone)
{
	if (!srcbone){
		return 0;
	}

	CBone* findbone = 0;
	ChaVector3 srcpos = srcbone->GetJointFPos();
	srcpos.x *= -1.0f;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	float mindist = FLT_MAX;
	float matchdist;

	//MODELBOUND mb;
	//GetModelBound(&mb);
	//matchdist = mb.r * 0.001f;
	matchdist = 0.30f;

	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
		CBone* curbone = itrbone->second;
		if (curbone){
			ChaVector3 curpos = curbone->GetJointFPos();
			ChaVector3 diffpos = curpos - srcpos;
			float curdist = (float)ChaVector3LengthDbl(&diffpos);
			if ((curdist <= mindist) && (curdist <= matchdist) && (curbone != srcbone)){
				//同一位置のボーンが存在する場合があるので、親もチェックする。
				CBone* srcparentbone = srcbone->GetParent();
				CBone* curparentbone = curbone->GetParent();
				if (srcparentbone && curparentbone){
					ChaVector3 srcparentpos = srcparentbone->GetJointFPos();
					srcparentpos.x *= -1.0f;
					ChaVector3 curparentpos = curparentbone->GetJointFPos();
					ChaVector3 pardiffpos = srcparentpos - curparentpos;
					float pardist = (float)ChaVector3LengthDbl(&pardiffpos);
					if (pardist <= matchdist)
					{
						findbone = curbone;
						mindist = curdist;
					}
				}
			}
		}
	}

	return findbone;
}


int CModel::PickBone( UIPICKINFO* pickinfo )
{
	pickinfo->pickobjno = -1;

	float fw, fh;
	fw = (float)pickinfo->winx / 2.0f;
	fh = (float)pickinfo->winy / 2.0f;

	int minno = -1;
	ChaVector3 cmpsc;
	ChaVector3 picksc = ChaVector3( 0.0f, 0.0f, 0.0f );
	ChaVector3 pickworld = ChaVector3( 0.0f, 0.0f, 0.0f );
	float cmpdist;
	float mindist = 0.0f;
	int firstflag = 1;
	ChaVector2 firstdiff = ChaVector2(0.0f, 0.0f);

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone && !curbone->GetSkipRenderBoneMark()){//ボーンマーク表示をスキップしているボーンはPick対象から外す 2022/09/16
			cmpsc.x = ( 1.0f + curbone->GetChildScreen().x ) * fw;
			cmpsc.y = ( 1.0f - curbone->GetChildScreen().y ) * fh;
			cmpsc.z = curbone->GetChildScreen().z;

			if( (cmpsc.z >= 0.0f) && (cmpsc.z <= 1.0f) ){
				float mag;
				firstdiff.x = (float)pickinfo->clickpos.x - cmpsc.x;
				firstdiff.y = (float)pickinfo->clickpos.y - cmpsc.y;
				mag = firstdiff.x * firstdiff.x + firstdiff.y * firstdiff.y;
				if (mag != 0.0f) {
					cmpdist = sqrtf( mag );
				}else{
					cmpdist = 0.0f;
				}

				if( (firstflag || (cmpdist <= mindist)) && (cmpdist <= (float)pickinfo->pickrange ) ){
					minno = curbone->GetBoneNo();
					mindist = cmpdist;
					picksc = cmpsc;
					pickworld = curbone->GetChildWorld();
					firstflag = 0;
				}
			}
		}
	}

	pickinfo->pickobjno = minno;
	if( minno >= 0 ){
		pickinfo->objscreen = picksc;
		pickinfo->objworld = pickworld;
		pickinfo->firstdiff = firstdiff;
	}

	return 0;
}

void CModel::SetSelectFlagReq( CBone* boneptr, int broflag )
{
	boneptr->SetSelectFlag( 1 );

	if( boneptr->GetChild() ){
		SetSelectFlagReq( boneptr->GetChild(), 1 );
	}
	if( boneptr->GetBrother() && broflag ){
		SetSelectFlagReq( boneptr->GetBrother(), 1 );
	}
}


int CModel::CollisionNoBoneObj_Mouse( UIPICKINFO* pickinfo, const char* objnameptr, 
	bool excludeinvface)
{
	//当たったら１、当たらなかったら０を返す。エラーも０を返す。

	CMQOObject* curobj = m_objectname[ objnameptr ];
	if( !curobj ){
		_ASSERT( 0 );
		return 0;
	}

	ChaVector3 startlocal, dirlocal;
	CalcMouseLocalRay( pickinfo, &startlocal, &dirlocal );

	int colli = curobj->CollisionLocal_Ray(startlocal, dirlocal, excludeinvface);

	return colli;
}

int CModel::CalcMouseLocalRay( UIPICKINFO* pickinfo, ChaVector3* startptr, ChaVector3* dirptr )
{
	ChaVector3 startsc, endsc;
	float rayx, rayy;
	rayx = (float)pickinfo->clickpos.x / ((float)pickinfo->winx / 2.0f) - 1.0f;
	rayy = 1.0f - (float)pickinfo->clickpos.y / ((float)pickinfo->winy / 2.0f);

	startsc = ChaVector3( rayx, rayy, 0.0f );
	endsc = ChaVector3( rayx, rayy, 1.0f );
	
    ChaMatrix mWVP, invmWVP;
	mWVP = m_matWorld * m_matVP;
	//ChaMatrixInverse( &invmWVP, NULL, &mWVP );
	invmWVP = ChaMatrixInv(mWVP);

	ChaVector3 startlocal, endlocal;

	ChaVector3TransformCoord( &startlocal, &startsc, &invmWVP );
	ChaVector3TransformCoord( &endlocal, &endsc, &invmWVP );

	ChaVector3 dirlocal = endlocal - startlocal;
	ChaVector3Normalize( &dirlocal, &dirlocal );

	*startptr = startlocal;
	*dirptr = dirlocal;

	return 0;
}

CBone* CModel::GetCalcRootBone( CBone* firstbone, int maxlevel )
{
	int levelcnt = 0;
	CBone* retbone = firstbone;
	CBone* curbone = firstbone;
	while( curbone && ((maxlevel == 0) || (levelcnt <= maxlevel)) )
	{
		retbone = curbone;
		curbone = curbone->GetParent();
		levelcnt++;
	}

	return retbone;
}


int CModel::TransformBone( int winx, int winy, int srcboneno, ChaVector3* worldptr, ChaVector3* screenptr, ChaVector3* dispptr )
{					
	CBone* curbone;
	curbone = m_bonelist[ srcboneno ];
	if (curbone){
		*worldptr = curbone->GetChildWorld();
		ChaMatrix mWVP;
		ChaMatrix mW;
		if (g_previewFlag != 5){
			if (curbone->GetParent()) {
				//mW = curbone->GetParent()->GetCurMp().GetWorldMat();
				mW = curbone->GetParent()->GetCurrentWorldMat(true);
			}
			else {
				//mW = curbone->GetCurMp().GetWorldMat();
				mW = curbone->GetCurrentWorldMat(true);
			}
		}
		else{
			CBone* parentbone = curbone->GetParent();
			if (parentbone){
				mW = parentbone->GetBtMat();//endjointのbtmatがおかしい可能性があるため。
			}
			else{
				mW = curbone->GetBtMat();
			}
		}

		ChaVector3 tmpfpos = curbone->GetJointFPos();
		ChaVector3TransformCoord(worldptr, &tmpfpos, &mW);
		mWVP = mW * m_matVP;
		ChaVector3TransformCoord(screenptr, &tmpfpos, &mWVP);

		float fw, fh;
		fw = (float)winx / 2.0f;
		fh = (float)winy / 2.0f;
		dispptr->x = (1.0f + screenptr->x) * fw;
		dispptr->y = (1.0f - screenptr->y) * fh;
		dispptr->z = screenptr->z;
	}
	return 0;
}


int CModel::ChangeMotFrameLeng( int motid, double srcleng )
{
	//MOTINFO* dstmi = m_motinfo[ motid - 1 ];//idは１から
	MOTINFO* dstmi = GetMotInfo(motid);//2021/08/26 eraseすることがあるのでindex = motid - 1とは限らない
	if( dstmi ){
		double befleng = dstmi->frameleng;

		dstmi->frameleng = srcleng;

		if( befleng > srcleng ){
			map<int, CBone*>::iterator itrbone;
			for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
				CBone* curbone = itrbone->second;
				if( curbone ){
					curbone->DeleteMPOutOfRange( motid, srcleng - 1.0 );
				}
			}
		}
	}
	return 0;
}

int CModel::AdvanceTime( int onefps, CEditRange srcrange, int previewflag, double difftime, double* nextframeptr, int* endflagptr, int* loopstartflag, int srcmotid)
{
	if (!nextframeptr || !endflagptr || !loopstartflag) {
		return 1;
	}

	*endflagptr = 0;
	*loopstartflag = 0;

	int loopflag = 0;
	MOTINFO* curmotinfo;
	if( srcmotid >= 0 ){
		//curmotinfo = m_motinfo[ srcmotid - 1];//idは１から
		curmotinfo = GetMotInfo(srcmotid);//2021/08/26 eraseすることがあるのでindex = motid - 1とは限らない
		loopflag = 0;
	}else{
		curmotinfo = m_curmotinfo;
		loopflag = curmotinfo->loopflag;
	}

	if( !curmotinfo ){
		return 0;
	}


	double curspeed, curframe;
	curspeed = curmotinfo->speed;
	curframe = curmotinfo->curframe;

	double nextframe;
	double oneframe = 1.0 / 30.0;
	//double oneframe = 1.0 / 300.0;

	double rangestart, rangeend;
	rangestart = srcrange.GetStartFrame();
	rangeend = srcrange.GetEndFrame();
	if (rangestart == rangeend){
		rangestart = 1.0;
		rangeend = curmotinfo->frameleng - 1.0;
	}

	rangestart = max(1.0, rangestart);


	if( previewflag >= 0 ){
		if (onefps == 0) {
			nextframe = curframe + difftime / oneframe * curspeed;
		}
		else {
			nextframe = curframe + 1.0;
		}
		if( nextframe > rangeend ){
			if( loopflag == 0 ){
				nextframe = rangeend;
				*endflagptr = 1;
			}else{
				nextframe = rangestart;
				*loopstartflag = 1;
			}
		}
	}else{
		if (onefps == 0) {
			nextframe = curframe - difftime / oneframe * curspeed;
		}
		else {
			nextframe = curframe + 1.0;
		}
		if( nextframe < rangestart ){
			if( loopflag == 0 ){
				nextframe = rangestart;
				*endflagptr = 1;
			}else{
				nextframe = rangeend;
			}
		}
	}

	*nextframeptr = nextframe;

	return 0;
}


int CModel::MakeEnglishName()
{
	map<int, CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( curobj ){
			CallF( ConvEngName( ENGNAME_DISP, (char*)curobj->GetName(), 256, (char*)curobj->GetEngName(), 256 ), return 1 );
		}
	}


	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CallF( ConvEngName( ENGNAME_BONE, (char*)curbone->GetBoneName(), 256, (char*)curbone->GetEngBoneName(), 256 ), return 1 );
		}
	}

	map<int, MOTINFO*>::iterator itrmi;
	for( itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++ ){
		MOTINFO* curmi = itrmi->second;
		if( curmi ){
			CallF( ConvEngName( ENGNAME_MOTION, curmi->motname, 256, curmi->engmotname, 256 ), return 1 );
		}
	}

	return 0;
}

int CModel::AddDefMaterial()
{

	CMQOMaterial* dummymat = new CMQOMaterial();
	if( !dummymat ){
		_ASSERT( 0 );
		return 1;
	}

	int defmaterialno = (int)m_material.size();
	dummymat->SetMaterialNo( defmaterialno );
	dummymat->SetName( "dummyMaterial" );

	m_material[defmaterialno] = dummymat;

	return 0;
}


int CModel::CreateFBXMeshReq( FbxNode* pNode)
{
	if (!pNode) {
		return 0;
	}

	ChaMatrix curmeshmat;
	curmeshmat.SetIdentity();
	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = (FbxNodeAttribute::EType)(pAttrib->GetAttributeType());
        //FbxGeometryConverter lConverter(pNode->GetFbxManager());

		char mes[256];

		int shapecnt;
		CMQOObject* newobj = 0;

		switch ( type )
		{
			case FbxNodeAttribute::eMesh:
				newobj = GetFBXMesh(pNode, pAttrib);     // メッシュを作成
				if (newobj){
					shapecnt = pNode->GetMesh()->GetShapeCount();
					if (shapecnt > 0){
						sprintf_s(mes, 256, "%s, shapecnt %d", pNode->GetName(), shapecnt);
						//MessageBoxA(NULL, mes, "check", MB_OK);
					}
				}
				break;
//			case FbxNodeAttribute::eNURB:
//			case FbxNodeAttribute::eNURBS_SURFACE:
//                lConverter.TriangulateInPlace(pNode);
//				GetFBXMesh( pAttrib, pNode->GetName() );     // メッシュを作成
//				break;
			case FbxNodeAttribute::eNull:
			case FbxNodeAttribute::eSkeleton:
					break;
			default:
				break;
		}
	}

	int childNodeNum;
	childNodeNum = pNode->GetChildCount();
	for ( int i = 0; i < childNodeNum; i++ )
	{
		FbxNode *pChild = pNode->GetChild(i);  // 子ノードを取得
		if (pChild) {
			CreateFBXMeshReq(pChild);
		}
	}

	return 0;
}

int CModel::CreateFBXShape( FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep )
{
	map<CMQOObject*,FBXOBJ*>::iterator itrobjindex;
	for( itrobjindex = m_fbxobj.begin(); itrobjindex != m_fbxobj.end(); itrobjindex++ ){
		FBXOBJ* curfbxobj = itrobjindex->second;
		if (curfbxobj) {
			const FbxMesh* curmesh = curfbxobj->mesh;
			CMQOObject* curobj = itrobjindex->first;
			if (curmesh && curobj) {
				int shapecnt = curmesh->GetShapeCount();
				if (shapecnt > 0) {
					CallF(GetFBXShape((FbxMesh*)curmesh, curobj, panimlayer, animleng, starttime, timestep), return 1);
				}
			}
		}
	}
	return 0;
}


CMQOObject* CModel::GetFBXMesh(FbxNode* pNode, FbxNodeAttribute *pAttrib)
{
	if (!pNode || !pAttrib) {
		_ASSERT(0);
		return 0;
	}
	if (!pNode->GetName()) {
		_ASSERT(0);
	}

	FbxMesh *pMesh = (FbxMesh*)pAttrib;
	if (strcmp("RootNode", pAttrib->GetName()) == 0){
		_ASSERT(0);
		return 0;
	}



	ChaMatrix globalmeshmat;//頂点用
	ChaMatrix globalnormalmat;//法線用
	globalmeshmat.SetIdentity();
	globalnormalmat.SetIdentity();
	{
		globalmeshmat.SetIdentity();
		FbxTime fbxtime;
		fbxtime.SetSecondDouble(0.0);
		FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(fbxtime);
		globalmeshmat = ChaMatrixFromFbxAMatrix(lGlobalPosition);

		globalnormalmat = globalmeshmat;
		globalnormalmat.SetTranslation(ChaVector3(0.0f, 0.0f, 0.0f));
		globalnormalmat.data[MATI_44] = 0.0f;
	}
	


	CMQOObject* newobj = new CMQOObject();
	_ASSERT( newobj );
	if (!newobj) {
		_ASSERT(0);
		return 0;
	}
	newobj->SetObjFrom( OBJFROM_FBX );
	newobj->SetName( (char*)pNode->GetName() );
	m_object[ newobj->GetObjectNo() ] = newobj;

	WCHAR wname[256] = L"none for debug";
	//ZeroMemory( wname, sizeof( WCHAR ) * 256 );
	//MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pNode->GetName(), 256, wname, 256 );//複数キャラ読み込み時に落ちることがある？？


	FBXOBJ* fbxobj = (FBXOBJ*)malloc(sizeof(FBXOBJ));
	if (!fbxobj) {
		_ASSERT(0);
		return 0;
	}
	fbxobj->node = pNode;
	fbxobj->mesh = pMesh;
	m_fbxobj[newobj] = fbxobj;

//shape
//	int morphnum = pMesh->GetShapeCount();
//	newobj->m_morphnum = morphnum;

//マテリアル
	int materialNum_ = 1;
	FbxNode* node = pMesh->GetNode();
	if ( node != 0 ) {
		// マテリアルの数
		materialNum_ = node->GetMaterialCount();
		

		//for dbginfo
		//if (materialNum_ != 1) {
			//char strinfo[MAX_PATH] = { 0 };
			//sprintf_s(strinfo, MAX_PATH, "mesh %s, materialNum_ %d", pNode->GetName(), materialNum_);
			//::MessageBoxA(NULL, strinfo, "check!!!", MB_OK);
		//}


		// マテリアル情報を取得
		//for( int i = 0; i < materialNum_; ++i ) {
		for (int i = 0; i < materialNum_; i++) {
			FbxSurfaceMaterial* material = node->GetMaterial( i );
			if ( material != 0 ) {
				CMQOMaterial* newmqomat = new CMQOMaterial();
				int mqomatno = newobj->GetMaterialSize();
				newmqomat->SetMaterialNo( mqomatno );
				newobj->SetMaterial( mqomatno, newmqomat );

				SetMQOMaterial( newmqomat, material );

				//const char* texname = newmqomat->GetTex();
				//const char* nodename = node->GetName();
				//_ASSERT(0);
			}
		}
	}


//頂点
	int PolygonNum       = pMesh->GetPolygonCount();
	int PolygonVertexNum = pMesh->GetPolygonVertexCount();
	int *IndexAry        = pMesh->GetPolygonVertices();

	int controlNum = pMesh->GetControlPointsCount();   // 頂点数
	FbxVector4* src = pMesh->GetControlPoints();    // 頂点座標配列

	//DbgOut(L"LDCheck : GetFBXMesh : nodename %s, controlnum %d, polygonnum %d, polygonvertexnum %d\r\n", wname, controlNum, PolygonNum, PolygonVertexNum);

	// コピー
	newobj->SetVertex( controlNum );
	newobj->SetPointBuf( (ChaVector3*)malloc( sizeof( ChaVector3 ) * controlNum ) );
	newobj->SetMeshMat(globalmeshmat);
	//for ( int i = 0; i < controlNum; ++i ) {
	for (int i = 0; i < controlNum; i++) {
		//ChaVector3* curctrl = newobj->GetPointBuf() + i;
		//curctrl->x = (float)src[i][0];
		//curctrl->y = (float)src[i][1];
		//curctrl->z = (float)src[i][2];
		////curctrl->w = (float)src[i][3];

		ChaVector3 tmpp;
		tmpp.x = (float)src[ i ][ 0 ];
		tmpp.y = (float)src[ i ][ 1 ];
		tmpp.z = (float)src[ i ][ 2 ];
		//curctrl->w = (float)src[ i ][ 3 ];
		//eNullのtransformによるメッシュ頂点の変換
		//アニメーションカーブが無い場合には　CBone::GetFbxAnimでworldmatをidentityにしないと副作用が出る
		ChaVector3* curctrl = newobj->GetPointBuf() + i;
		ChaVector3TransformCoord(curctrl, &tmpp, &globalmeshmat);


		////*curctrl = tmpp;


//DbgOut( L"GetFBXMesh : ctrl %d, (%f, %f, %f)\r\n",
//	i, curctrl->x, curctrl->y, curctrl->z );

	}

	const FbxLayerElementMaterial* pPolygonMaterials = NULL;
	FbxGeometryElement::EMappingMode materialmappingMode = FbxGeometryElement::eAllSame;
	pPolygonMaterials = pMesh->GetElementMaterial();
	if (pPolygonMaterials != NULL) {
		materialmappingMode = pPolygonMaterials->GetMappingMode();
	}

	newobj->SetFace(PolygonNum);
	newobj->SetFaceBuf(new CMQOFace[PolygonNum]);
	for (int p = 0; p < PolygonNum; p++) {
		int IndexNumInPolygon = pMesh->GetPolygonSize(p);  // p番目のポリゴンの頂点数
		if ((IndexNumInPolygon != 3) && (IndexNumInPolygon != 4)) {
			_ASSERT(0);
			return 0;
		}

		CMQOFace* curface = newobj->GetFaceBuf() + p;
		curface->SetPointNum(IndexNumInPolygon);

		curface->SetFaceNo(p);
		curface->SetBoneType(MIKOBONE_NONE);


		int lookupIndex = 0;
		if (pPolygonMaterials) {
			switch (materialmappingMode) {
			case FbxGeometryElement::eByPolygon:
				lookupIndex = p;//triangleNo.
				break;
			case FbxGeometryElement::eAllSame:
				lookupIndex = 0;
				break;
			default:
				lookupIndex = 0;
				break;
			}
			int materialIndex = pPolygonMaterials->mIndexArray->GetAt(lookupIndex);
			if ((materialIndex >= 0) && (materialIndex < materialNum_)) {
				curface->SetMaterialNo(materialIndex);
			}
		}
		else {
			lookupIndex = 0;
			curface->SetMaterialNo(0);
		}

		for (int n = 0; n < IndexNumInPolygon; n++) {
			// ポリゴンpを構成するn番目の頂点のインデックス番号
			int IndexNumber = pMesh->GetPolygonVertex(p, n);
			curface->SetIndex(n, IndexNumber);
		}
	}
	



	/*
    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal * lNormalElement = NULL;
        const FbxGeometryElementUV * lUVElement = NULL;
        if (mHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (mHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
            lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
            lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
            lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
            lVertices[lIndex * VERTEX_STRIDE + 3] = 1;

            // Save the normal.
            if (mHasNormal)
            {
                int lNormalIndex = lIndex;
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
                lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
                lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
            }

            // Save the UV.
            if (mHasUV)
            {
                int lUVIndex = lIndex;
                if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
                lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
            }
        }

    }
	*/



//法線
	int layerNum = pMesh->GetLayerCount();
	//for ( int i = 0; i < layerNum; ++i ) {
	for (int i = 0; i < layerNum; i++) {
	   FbxLayer* layer = pMesh->GetLayer( i );
	   FbxLayerElementNormal* normalElem = layer->GetNormals();
	   if ( normalElem == 0 ) {
		  continue;   // 法線無し
	   }
	   // 法線あった！
		// 法線の数・インデックス
		int    normalNum          = normalElem->GetDirectArray().GetCount();
		int    indexNum           = normalElem->GetIndexArray().GetCount();


//DbgOut( L"GetFBXMesh : %s : normalNum %d : indexNum %d\r\n", wname, normalNum, indexNum );

		// マッピングモード・リファレンスモード取得
		FbxLayerElement::EMappingMode mappingMode = normalElem->GetMappingMode();
		FbxLayerElement::EReferenceMode refMode = normalElem->GetReferenceMode();

		if ( mappingMode == FbxLayerElement::eByPolygonVertex ) {
//DbgOut( L"GetFBXMesh : %s : mapping eByPolygonVertex\r\n", wname );

			newobj->SetNormalMappingMode(0);

			if ( refMode == FbxLayerElement::eDirect ) {
//DbgOut( L"GetFBXMesh : %s : ref eDirect\r\n", wname );

				newobj->SetNormalLeng( normalNum );
				newobj->SetNormal( (ChaVector3*)malloc( sizeof( ChaVector3 ) * normalNum ) );

				// 直接取得
			  //for ( int i = 0; i < normalNum; ++i ) {
				for (int i2 = 0; i2 < normalNum; i2++) {
					ChaVector3* curn = newobj->GetNormal() + i2;
					ChaVector3 tmpn;
					tmpn.x = (float)normalElem->GetDirectArray().GetAt( i2 )[ 0 ];
					tmpn.y = (float)normalElem->GetDirectArray().GetAt( i2 )[ 1 ];
					tmpn.z = (float)normalElem->GetDirectArray().GetAt( i2 )[ 2 ];

					ChaVector3TransformCoord(curn, &tmpn, &globalnormalmat);

				}
		   }else if ( refMode == FbxLayerElement::eIndexToDirect ){
//DbgOut( L"GetFBXMesh : %s : ref eIndexToDirect\r\n", wname );

				newobj->SetNormalLeng( indexNum );
				newobj->SetNormal( (ChaVector3*)malloc( sizeof( ChaVector3 ) * indexNum ) );

				int lIndex;
				for( lIndex = 0; lIndex < indexNum; lIndex++ ){

					int lNormalIndex = normalElem->GetIndexArray().GetAt(lIndex);

				    FbxVector4 lCurrentNormal;
					lCurrentNormal = normalElem->GetDirectArray().GetAt(lNormalIndex);
					ChaVector3* curn = newobj->GetNormal() + lIndex;
					ChaVector3 tmpn;
					tmpn.x = static_cast<float>(lCurrentNormal[0]);
					tmpn.y = static_cast<float>(lCurrentNormal[1]);
					tmpn.z = static_cast<float>(lCurrentNormal[2]);

					ChaVector3TransformCoord(curn, &tmpn, &globalnormalmat);
				}
		   }
		} else if ( mappingMode == FbxLayerElement::eByControlPoint ) {
//DbgOut( L"GetFBXMesh : %s : mapping eByControlPoint\r\n", wname );

			newobj->SetNormalMappingMode(1);


		   if ( refMode == FbxLayerElement::eDirect ) {
//DbgOut( L"GetFBXMesh : %s : ref eDirect\r\n", wname );

				newobj->SetNormalLeng( normalNum );
				newobj->SetNormal( (ChaVector3*)malloc( sizeof( ChaVector3 ) * normalNum ) );

				// 直接取得
				//for ( int i = 0; i < normalNum; ++i ) {
				for (int i2 = 0; i2 < normalNum; i2++) {
					ChaVector3* curn = newobj->GetNormal() + i2;
					ChaVector3 tmpn;
					tmpn.x = (float)normalElem->GetDirectArray().GetAt( i2 )[ 0 ];
					tmpn.y = (float)normalElem->GetDirectArray().GetAt( i2 )[ 1 ];
					tmpn.z = (float)normalElem->GetDirectArray().GetAt( i2 )[ 2 ];

					ChaVector3TransformCoord(curn, &tmpn, &globalnormalmat);
				}
		   }else{
//DbgOut( L"GetFBXMesh : %s : ref %d\r\n", wname, refMode );
			   _ASSERT(0);
		   }
		} else {
			_ASSERT( 0 );
		}

		break;
	}

//UV
	int layerCount = pMesh->GetLayerCount();   // meshはFbxMesh
	for ( int uvi = 0; uvi < layerCount; ++uvi ) {
		FbxLayer* layer = pMesh->GetLayer( uvi );
		FbxLayerElementUV* elem = layer->GetUVs();
		if ( elem == 0 ) {
			continue;
		}

		// UV情報を取得
		// UVの数・インデックス
		int UVNum = elem->GetDirectArray().GetCount();
		int indexNum = elem->GetIndexArray().GetCount();
//		int size = UVNum > indexNum ? UVNum : indexNum;

		// マッピングモード・リファレンスモード別にUV取得
		FbxLayerElement::EMappingMode mappingMode = elem->GetMappingMode();
		FbxLayerElement::EReferenceMode refMode = elem->GetReferenceMode();

		if (mappingMode == FbxLayerElement::eByPolygonVertex) {
			DbgOut(L"\r\n\r\n");
			DbgOut(L"check !!! GetFBXMesh : %s : UV : mapping eByPolygonVertex\r\n", wname);

			if (refMode == FbxLayerElement::eDirect) {
				DbgOut(L"GetFBXMesh : %s : UV : refMode eDirect\r\n", wname);
				int size = UVNum;
				newobj->SetUVLeng(size);
				newobj->SetUVBuf((ChaVector2*)malloc(sizeof(ChaVector2) * size));

				// 直接取得
				//for (int i = 0; i < size; ++i) {
				for (int i = 0; i < size; i++) {
					(newobj->GetUVBuf() + i)->x = (float)elem->GetDirectArray().GetAt(i)[0];
					(newobj->GetUVBuf() + i)->y = 1.0f - (float)elem->GetDirectArray().GetAt(i)[1];
					DbgOut(L"direct %d, u : %f, v : %f\r\n", i, (newobj->GetUVBuf() + i)->x, (newobj->GetUVBuf() + i)->y);
				}
				DbgOut(L"\r\n\r\n");
			}
			else if (refMode == FbxLayerElement::eIndexToDirect) {
				DbgOut(L"\r\n\r\n");
				DbgOut(L"Check !!! GetFBXMesh : %s : UV : refMode eIndexToDirect\r\n", wname);
				int size = indexNum;
				newobj->SetUVLeng(size);
				newobj->SetUVBuf((ChaVector2*)malloc(sizeof(ChaVector2) * size));

				// インデックスから取得
				//for (int i = 0; i < size; ++i) {
				for (int i = 0; i < size; i++) {
					int index = elem->GetIndexArray().GetAt(i);
					(newobj->GetUVBuf() + i)->x = (float)elem->GetDirectArray().GetAt(index)[0];
					(newobj->GetUVBuf() + i)->y = 1.0f - (float)elem->GetDirectArray().GetAt(index)[1];
					DbgOut(L"direct %d, u : %f, v : %f\r\n", i, (newobj->GetUVBuf() + i)->x, (newobj->GetUVBuf() + i)->y);
				}
				DbgOut(L"\r\n\r\n");
			}
			else {
				DbgOut(L"GetFBXMesh : %s : UV : refMode %d\r\n", wname, refMode);
				_ASSERT(0);
			}
		}
		else if (mappingMode == FbxLayerElement::eByControlPoint) {
			if (refMode == FbxLayerElement::eDirect) {
				DbgOut(L"\r\n\r\n");
				DbgOut(L"check !!! GetFBXMesh : %s : UV : refMode eDirect\r\n", wname);
				int size = UVNum;
				newobj->SetUVLeng(size);
				ChaVector2* newuv = (ChaVector2*)malloc(sizeof(ChaVector2) * size);
				_ASSERT(newuv);
				newobj->SetUVBuf(newuv);
				//newobj->SetUVBuf((ChaVector2*)malloc(sizeof(ChaVector2) * size));

				// 直接取得
				//for (int i = 0; i < size; ++i) {
				for (int i = 0; i < size; i++) {
					(newobj->GetUVBuf() + i)->x = (float)elem->GetDirectArray().GetAt(i)[0];
					(newobj->GetUVBuf() + i)->y = 1.0f - (float)elem->GetDirectArray().GetAt(i)[1];
					DbgOut(L"direct %d, u : %f, v : %f\r\n", i, (newobj->GetUVBuf() + i)->x, (newobj->GetUVBuf() + i)->y);
				}
			}
			DbgOut(L"\r\n\r\n");
		} else {
DbgOut( L"GetFBXMesh : %s : UV : mappingMode %d\r\n", wname, mappingMode );
DbgOut( L"GetFBXMesh : %s : UV : refMode %d\r\n", wname, refMode );
_ASSERT(0);
		}
		break;
	}

	return newobj;
}

int CModel::SetMQOMaterial( CMQOMaterial* newmqomat, FbxSurfaceMaterial* pMaterial )
{
	char tmpname[512];
	strcpy_s(tmpname, 512, pMaterial->GetName());
	newmqomat->SetName( tmpname );

	char* emitex = 0;
    const FbxDouble3 lEmissive = FbxGetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, &emitex);
	if( emitex ){
		DbgOut( L"SetMQOMaterial : emitexture find\r\n" );
	}
	ChaVector3 tmpemi;

	tmpemi.x = (float)lEmissive[0] * g_EmissiveFactorAtLoading;
	tmpemi.y = (float)lEmissive[1] * g_EmissiveFactorAtLoading;
	tmpemi.z = (float)lEmissive[2] * g_EmissiveFactorAtLoading;
	newmqomat->SetEmi3F( tmpemi );


	char* ambtex = 0;
	const FbxDouble3 lAmbient = FbxGetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, &ambtex);
	if( ambtex ){
		DbgOut( L"SetMQOMaterial : ambtexture find\r\n" );
	}
	ChaVector3 tmpamb;
	tmpamb.x = (float)lAmbient[0] * g_AmbientFactorAtLoading;
	tmpamb.y = (float)lAmbient[1] * g_AmbientFactorAtLoading;
	tmpamb.z = (float)lAmbient[2] * g_AmbientFactorAtLoading;
	newmqomat->SetAmb3F( tmpamb );

	char* diffusetex = 0;
    const FbxDouble3 lDiffuse = FbxGetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, &diffusetex);
	if( diffusetex ){
		//strcpy_s( newmqomat->tex, 256, diffusetex );
		DbgOut( L"SetMQOMaterial : diffusetexture find\r\n" );
	}
	ChaVector4 tmpdif;
	tmpdif.x = (float)lDiffuse[0] * g_DiffuseFactorAtLoading;
	tmpdif.y = (float)lDiffuse[1] * g_DiffuseFactorAtLoading;
	tmpdif.z = (float)lDiffuse[2] * g_DiffuseFactorAtLoading;
	tmpdif.w = 1.0f;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	newmqomat->SetDif4F( tmpdif );


	char* spctex = 0;
    const FbxDouble3 lSpecular = FbxGetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, &spctex);
	if( spctex ){
		DbgOut( L"SetMQOMaterial : spctexture find\r\n" );
	}
	ChaVector3 tmpspc;
	tmpspc.x = (float)lSpecular[0] * g_SpecularFactorAtLoading;
	tmpspc.y = (float)lSpecular[1] * g_SpecularFactorAtLoading;
	tmpspc.z = (float)lSpecular[2] * g_SpecularFactorAtLoading;
	newmqomat->SetSpc3F( tmpspc );

    //FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);//<-- link error : MameBake3DLib.lib外だから？　
	//char* shininesstex = 0;
	//FbxProperty lShininessProperty = FbxGetMaterialProperty(pMaterial,
	//	FbxSurfaceMaterial::sShininess, FbxSurfaceMaterial::sShininessFactor, &shininesstex);//<-- sSnininessFactorは無い. sShininessは有る.
 //   if (lShininessProperty.IsValid())
 //   {
 //       double lShininess = lShininessProperty.Get<FbxDouble>();
 //       newmqomat->SetPower( static_cast<float>(lShininess) );
 //   }
	FbxDouble Shininess = FbxGetMaterialShininessProperty(pMaterial);
	newmqomat->SetPower(static_cast<float>(Shininess));

//texture

	FbxProperty pProperty;
	pProperty = pMaterial->FindProperty( FbxSurfaceMaterial::sDiffuse );
    int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
    if(lLayeredTextureCount > 0)
    {
        for(int j=0; j<lLayeredTextureCount; ++j)
        {
            FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
            int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
            for(int k =0; k<lNbTextures; ++k)
            {
                //char* nameptr = (char*)lLayeredTexture->GetName();
				if(lLayeredTexture->GetName()){
					char tempname[256];
					strcpy_s( tempname, 256, lLayeredTexture->GetName());
					char* lastslash = strrchr( tempname, '/' );
					if( !lastslash ){
						lastslash = strrchr( tempname, '\\' );
					}
					if( lastslash ){
						newmqomat->SetTex( lastslash + 1 );
					}else{
						newmqomat->SetTex( tempname );
					}
					char* lastp = strrchr( (char*)newmqomat->GetTex(), '.' );
					if( !lastp ){
						newmqomat->Add2Tex( ".tga" );
					}
					WCHAR wname[256];
					::ZeroMemory( wname, sizeof( WCHAR ) * 256 );
					MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, newmqomat->GetTex(), 256, wname, 256 );

DbgOut( L"SetMQOMaterial : layered texture %s\r\n", wname );

					break;
				}
            }
        }
    }
    else
    {
        //no layered texture simply get on the property
        int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();
        if(lNbTextures > 0)
        {
            for(int j =0; j<lNbTextures; ++j)
            {
                FbxFileTexture* lTexture = pProperty.GetSrcObject<FbxFileTexture>(j);
                if(lTexture)
                {
                    char* nameptr = (char*)lTexture->GetFileName();
					if( nameptr ){
						char tempname[256];
						strcpy_s( tempname, 256, nameptr );
						char* lastslash = strrchr( tempname, '/' );
						if( !lastslash ){
							lastslash = strrchr( tempname, '\\' );
						}
						if( lastslash ){
							newmqomat->SetTex( lastslash + 1 );
						}else{
							newmqomat->SetTex( tempname );
						}
						char* lastp = strrchr( (char*)newmqomat->GetTex(), '.' );
						if( !lastp ){
							newmqomat->Add2Tex( ".tga" );
						}

						WCHAR wname[256];
						::ZeroMemory( wname, sizeof( WCHAR ) * 256 );
						MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, newmqomat->GetTex(), 256, wname, 256 );

DbgOut( L"SetMQOMaterial : texture %s\r\n", wname );

						break;
					}
                }
            }
        }
    }


   return 0;
}

//// Get specific property value and connected texture if any.
//// Value = Property value * Factor property value (if no factor property, multiply by 1).
//FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
//    const char * pPropertyName,
//    const char * pFactorPropertyName,
//    char** ppTextureName)
//{
//	*ppTextureName = 0;
//
//    FbxDouble3 lResult(0, 0, 0);
//    const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
//    const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
//    if (lProperty.IsValid() && lFactorProperty.IsValid())
//    {
//        lResult = lProperty.Get<FbxDouble3>();
//        double lFactor = lFactorProperty.Get<FbxDouble>();
//        if (lFactor != 1)
//        {
//            lResult[0] *= lFactor;
//            lResult[1] *= lFactor;
//            lResult[2] *= lFactor;
//        }
//    }
//
//
//    return lResult;
//}

//void CModel::CreateExtendBoneReq(CBone* srcbone)
//{
//	if (srcbone) {
//		if (!srcbone->GetExtendFlag() && !srcbone->GetChild()) {
//			CBone* newbone = CBone::GetNewBone(this);
//			_ASSERT(newbone);
//			if (!newbone) {
//				_ASSERT(0);
//				return;
//			}
//			newbone->SetExtendFlag(true);
//			srcbone->AddChild(newbone);
//
//			char newbonename[256];
//			strcpy_s(newbonename, 256, "ext_");
//			strcat_s(newbonename, 256, srcbone->GetBoneName());
//			newbone->SetName(newbonename);
//
//			newbone->SetTopBoneFlag(0);
//			m_bonelist[newbone->GetBoneNo()] = newbone;
//			m_bonename[newbone->GetBoneName()] = newbone;
//
//			//if (type == FbxNodeAttribute::eSkeleton) {
//			newbone->SetType(FBXBONE_NORMAL);
//			//}
//			//else if (type == FbxNodeAttribute::eNull) {
//			//	newbone->SetType(FBXBONE_NULL);
//			//}
//			//else {
//			//	_ASSERT(0);
//			//}
//
//		}
//		else {
//			if (srcbone->GetChild()) {
//				CreateExtendBoneReq(srcbone->GetChild());
//			}
//		}
//
//		if (srcbone->GetBrother()) {
//			CreateExtendBoneReq(srcbone->GetBrother());
//		}
//	}
//}



void CModel::CreateFBXBoneReq(FbxScene* pScene, FbxNode* pNode, FbxNode* parnode, FbxNode* parentbonenode)
{
//	EFbxRotationOrder lRotationOrder0 = eEulerZXY;
//	EFbxRotationOrder lRotationOrder1 = eEulerXYZ;

	WCHAR wname[256];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pNode->GetName(), -1, wname, 256);
	//if (wcsstr(wname, L"Character") != 0) {
	//	_ASSERT(0);
	//}
	const char* nodetypename = pNode->GetTypeName();

	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();
		//const char* nodename = pNode->GetName();
		//		//const char* parnodename = parnode->GetName();
		//		WCHAR parwname[256];
		//		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, parnode->GetName(), -1, parwname, 256 );

/*
typedef enum
{
 eUnknown,
		eNull,
		eMarker,
		eSkeleton,
		eMesh,
		eNurbs,
		ePatch,
		eCamera,
		eCameraStereo,
		eCameraSwitcher,
		eLight,
		eOpticalReference,
		eOpticalMarker,
		eNurbsCurve,
		eTrimNurbsSurface,
		eBoundary,
		eNurbsSurface,
		eShape,
		eLODGroup,
		eSubDiv,
		eCachedEffect,
		eLine
} FbxNodeAttribute;*/


		switch ( type )
		{
		case FbxNodeAttribute::eNull:
		case FbxNodeAttribute::eSkeleton:
			{
				GetFBXBone(pScene, type, pNode, parentbonenode);
				//if (strcmp(pNode->GetName(), "RootNode") != 0) {
				//	if (parnode && (strcmp(parnode->GetName(), "RootNode") != 0)) {
				//		GetFBXBone(pScene, type, pAttrib, pNode, parentbonenode);
				//	}
				//	else {
				//		GetFBXBone(pScene, type, pAttrib, pNode, 0);
				//	}
				//}
				//else {
				//	//_ASSERT(0);
				//}

				int childNodeNum;
				childNodeNum = pNode->GetChildCount();
				for (int i = 0; i < childNodeNum; i++)
				{
					FbxNode* pChild = pNode->GetChild(i);  // 子ノードを取得
					CreateFBXBoneReq(pScene, pChild, pNode, pNode);//pNode
				}

			}
				break;
		case FbxSkeleton::eRoot:
			default:
			{
				int childNodeNum;
				childNodeNum = pNode->GetChildCount();
				for (int i = 0; i < childNodeNum; i++)
				{
					FbxNode* pChild = pNode->GetChild(i);  // 子ノードを取得
					CreateFBXBoneReq(pScene, pChild, pNode, parentbonenode);//parentbonenode
				}
			}
				break;
		}

	}
	else {
		//eNullはattributeを持たないことがある　その場合はGetTypeNameが"Null"という文字列になっている

		if ((strcmp(nodetypename, "Null") == 0) && (strcmp(pNode->GetName(), "RootNode") != 0)) {
			GetFBXBone(pScene, FbxNodeAttribute::eNull, pNode, parentbonenode);

			int childNodeNum;
			childNodeNum = pNode->GetChildCount();
			for (int i = 0; i < childNodeNum; i++)
			{
				FbxNode* pChild = pNode->GetChild(i);  // 子ノードを取得
				CreateFBXBoneReq(pScene, pChild, pNode, pNode);//pNode
				//CreateFBXBoneReq(pScene, pChild, pNode, parentbonenode);//pNode
			}

		}
		else {
			int childNodeNum;
			childNodeNum = pNode->GetChildCount();
			for (int i = 0; i < childNodeNum; i++)
			{
				FbxNode* pChild = pNode->GetChild(i);  // 子ノードを取得
				CreateFBXBoneReq(pScene, pChild, pNode, parentbonenode);//parentbonenode
			}
		}
	}


	return;
}

CBone* CModel::CreateNewFbxBone(FbxNodeAttribute::EType type, FbxNode* curnode, FbxNode* parnode)
{
	int settopflag = 0;

	if (!curnode) {
		_ASSERT(0);
		return 0;
	}

	CBone* newbone = CBone::GetNewBone(this);
	_ASSERT(newbone);
	if (!newbone) {
		_ASSERT(0);
		return 0;
	}

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	newbone->LoadCapsuleShape(m_pdev, pd3dImmediateContext);//!!!!!!!!!!!

	char newbonename[JOINTNAMELENG];
	if (!curnode) {
		//Rootボーン作成
		strcpy_s(newbonename, JOINTNAMELENG, "Root");
	}
	else {
		//通常のボーン作成
		strcpy_s(newbonename, JOINTNAMELENG, curnode->GetName());
	}
	TermJointRepeats(newbonename);
	newbone->SetName(newbonename);
	newbone->SetFbxNodeOnLoad(curnode);
	

	m_bonelist[newbone->GetBoneNo()] = newbone;
	m_bonename[newbone->GetBoneName()] = newbone;


	//2023/02/19
	//とりあえず　ジョイント名をみて　自動でIKStopFlagをセット
	if ((strstr(newbone->GetBoneName(), "UpperLeg") != 0) ||
		//(strstr(newbone->GetBoneName(), "UpperArm") != 0)) {
		(strstr(newbone->GetBoneName(), "Shoulder") != 0)) {
		newbone->SetIKStopFlag(true);
	}
	else {
		newbone->SetIKStopFlag(false);
	}

	//const char* strextend;
	//strextend = strstr(newbone->GetBoneName(), "_extend_");
	//if (!strextend) {
	if (type == FbxNodeAttribute::eSkeleton) {
		newbone->SetType(FBXBONE_NORMAL);
	}
	else if (type == FbxNodeAttribute::eNull) {
		newbone->SetType(FBXBONE_NULL);
	}
	else {
		_ASSERT(0);
	}
	//}
	//else {
	//	newbone->SetType(FBXBONE_NULL);
	//}
	if (!m_topbone) {
		m_topbone = newbone;
		m_bone2node[newbone] = curnode;
		newbone->SetTopBoneFlag(1);
		settopflag = 1;
	}
	else {
		m_bone2node[newbone] = curnode;
		newbone->SetTopBoneFlag(0);
	}


	if (parnode) {
		//const char* parentbonename = parnode->GetName();
		char parentbonename[JOINTNAMELENG];
		strcpy_s(parentbonename, JOINTNAMELENG, parnode->GetName());
		TermJointRepeats(parentbonename);

		CBone* parentbone = m_bonename[parentbonename];
		if (parentbone) {
			parentbone->AddChild(newbone);
			//_ASSERT(0);
		}
		else {
			::MessageBoxA(NULL, "GetFBXBone : parentbone NULL error ", parentbonename, MB_OK);
		}
	}
	else {
		if (settopflag == 0) {
			//この関数呼び出し時にparnode == 0で　この関数呼び出し以前にm_topboneがすでにセットされている場合
			m_topbone->AddChild(newbone);
		}
		else {
			//_ASSERT(0);
			//::MessageBoxA( NULL, "GetFBXBone : parentbone NULL error ", nodename, MB_OK );
		}
	}


	return newbone;

}


int CModel::GetFBXBone(FbxScene* pScene, FbxNodeAttribute::EType type, FbxNode* curnode, FbxNode* parnode )
{
	if (!pScene || !curnode) {
		_ASSERT(0);
		return 1;
	}



	//CBone* tmptopbone = m_topbone;
	CBone* newbone = 0;
	//if (!m_topbone && !IncludeRootOrReference(pScene->GetRootNode())) {
	//	//最初のボーンで　ノードツリーにRootもReferenceも無い場合　Rootボーンを作成
	//	tmptopbone = CreateNewFbxBone(FbxNodeAttribute::eSkeleton, 0, 0);
	//}


	//通常のボーン　または　名前にRoot, Referenceが入っている場合のボーン　作成
	newbone = CreateNewFbxBone(type, curnode, parnode);

	return 0;
}

int CModel::MotionID2Index( int motid )
{
	int retindex = -1;

	int chkcnt = 0;
	map<int,MOTINFO*>::iterator itrmotinfo;
	for( itrmotinfo = m_motinfo.begin(); itrmotinfo != m_motinfo.end(); itrmotinfo++ ){
		MOTINFO* curmi = itrmotinfo->second;
		if( curmi ){
			if( curmi->motid == motid ){
				retindex = chkcnt;
				break;
			}
		}
		chkcnt++;
	}
	return retindex;
}


FbxAnimLayer* CModel::GetAnimLayer( int motid )
{
	FbxAnimLayer *retAnimLayer = 0;

	int motindex = MotionID2Index( motid );
	if( motindex < 0 ){
		return 0;
	}

	FbxAnimStack *lCurrentAnimationStack = m_pscene->FindMember<FbxAnimStack>(mAnimStackNameArray[motindex]->Buffer());
	if (lCurrentAnimationStack == NULL){
		_ASSERT( 0 );
		return 0;
	}
	retAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();

	return retAnimLayer;
}

//void CModel::InitMpScaleReq(CBone* curbone, int srcmotid, double srcframe)
//{
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	if (curbone) {
//		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//		int paraxiskind = -1;//2021/11/18
//		//int isfirstbone = 0;
//		cureul = curbone->CalcLocalEulXYZ(paraxiskind, srcmotid, roundingframe, BEFEUL_BEFFRAME);
//
//		int inittraflag1 = 0;
//		int setchildflag1 = 1;
//		int initscaleflag1 = 1;//!!!!!!!
//		ChaMatrix befwm = curbone->GetWorldMat(srcmotid, roundingframe, 0);
//		curbone->SetWorldMatFromEul(inittraflag1, setchildflag1, befwm, cureul, srcmotid, roundingframe, initscaleflag1);
//
//		if (curbone->GetChild()) {
//			InitMpScaleReq(curbone->GetChild(), srcmotid, roundingframe);
//		}
//		if (curbone->GetBrother()) {
//			InitMpScaleReq(curbone->GetBrother(), srcmotid, roundingframe);
//		}
//
//	}
//}

int CModel::CreateFBXAnim( FbxScene* pScene, FbxNode* prootnode, BOOL motioncachebatchflag)
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;


	SetDefaultBonePos(pScene);


	pScene->FillAnimStackNameArray(mAnimStackNameArray);
    const int lAnimStackCount = mAnimStackNameArray.GetCount();

	SetLoadingMotionCount(lAnimStackCount);

	DbgOut( L"FBX anim num %d\r\n", lAnimStackCount );

	if( lAnimStackCount <= 0 ){
		//_ASSERT( 0 );
		return 0;
	}
	else {
		int animno;
		for (animno = 0; animno < lAnimStackCount; animno++) {





			// select the base layer from the animation stack
			//char* animname = mAnimStackNameArray[animno]->Buffer();
			//MessageBoxA( NULL, animname, "check", MB_OK );
			FbxAnimStack* lCurrentAnimationStack = m_pscene->FindMember<FbxAnimStack>(mAnimStackNameArray[animno]->Buffer());
			if (lCurrentAnimationStack == NULL) {
				_ASSERT(0);
				return 1;
			}


			////RokDeBone2のデータを読み込んだ場合にはZXYをXYZにコンバートする
			//pScene->GetRootNode()->ConvertPivotAnimationRecursive(lCurrentAnimationStack, FbxNode::eDestinationPivot, 30.0, true);//2022/07/28コメントアウト


			FbxAnimLayer* mCurrentAnimLayer;
			mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
			SetCurrentAnimLayer(mCurrentAnimLayer);

			//pScene->GetEvaluator()->SetContext(lCurrentAnimationStack);
			pScene->SetCurrentAnimationStack(lCurrentAnimationStack);
			//pScene->GetAnimationEvaluator()->SetContext(lCurrentAnimationStack);



			//pScene->GetRootNode()->ConvertPivotAnimationRecursive( mAnimStackNameArray[animno]->Buffer(), FbxNode::eDestinationPivot, 30.0, true );
			//pScene->GetRootNode()->ConvertPivotAnimationRecursive( mAnimStackNameArray[animno]->Buffer(), FbxNode::eSourcePivot, 30.0, true );

			FbxTakeInfo* lCurrentTakeInfo = pScene->GetTakeInfo(*(mAnimStackNameArray[animno]));
			if (lCurrentTakeInfo)
			{
				mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
				mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();

				double dstart = mStart.GetSecondDouble();
				double dstop = mStop.GetSecondDouble();
				//_ASSERT( 0 );
			}
			else
			{
				_ASSERT(0);
				// Take the time line value
				FbxTimeSpan lTimeLineTimeSpan;
				pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
				mStart = lTimeLineTimeSpan.GetStart();
				mStop = lTimeLineTimeSpan.GetStop();
			}


			//		int animleng = (int)mStop.GetFrame();// - mStart.GetFrame() + 1;		
					//mFrameTime.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
					//mFrameTime2.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
			mFrameTime.SetSecondDouble(1.0 / 30.0);
			mFrameTime2.SetSecondDouble(1.0 / 30.0);
			//mFrameTime.SetSecondDouble( 1.0 / 300.0 );
			//mFrameTime2.SetSecondDouble( 1.0 / 300.0 );


			double animleng = (int)((mStop.GetSecondDouble() - mStart.GetSecondDouble()) * 30.0);

			DbgOut(L"FBX anim %d, animleng %lf\r\n", animno, animleng);




			//######################################################################################
			//### AddMotion() : MotInfo, AddMotionPoint, CreateIndexedMotionPoint #################
			//######################################################################################
			int curmotid = -1;
			//AddMotion(mAnimStackNameArray[animno]->Buffer(), 0, (animleng - 1), &curmotid);// animleng - 1 !!!!!
			AddMotion(mAnimStackNameArray[animno]->Buffer(), 0, animleng, &curmotid);//2022/10/21 : animleng - 1だと最終フレームにモーションポイントが出来ない




			map<int, CBone*>::iterator itrbone;
			for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
				CBone* curbone = itrbone->second;
				if (curbone) {
					curbone->SetGetAnimFlag(0);
				}
			}

			//FbxPose* pPose = pScene->GetPose( animno );
			//FbxPose* pPose = pScene->GetPose( 10 );

	//!!!!!!!!!!!!!!!!!!!!!		
			//FbxPose* pPose = GetBindPose();
			FbxPose* pPose = NULL;



			//Local情報からモーションを計算する
			m_loadbonecount = 0;

			if (animno == 0) {
				CreateLoadFbxAnim(pScene);

				//#### 2022/11/01 ################################################
				//この時点において　m_bonelistにはCBone*がセットされている 
				//LoadFbx()内で　CBone::SetFbxNodeOnLoadするようにした
				//よって　スレッド準備は　CreateLoadFbxAnimでしてしまうことが可能
				//CreateFbxAnimReq不要
				//#### 2022/11/01 ################################################
				//CreateFBXAnimReq(animno, pScene, pPose, prootnode, curmotid, animleng);//2022/10/21 : 最終フレームのモーションフレーム無し問題対応
			
			
			}

			//スレッド読み込みしない場合
			//map<int, CBone*>::iterator itranimbone;
			//for (itranimbone = m_bonelist.begin(); itranimbone != m_bonelist.end(); itranimbone++) {
			//	CBone* curbone = itranimbone->second;
			//	if (curbone) {
			//		FbxNode* curnode = curbone->GetFbxNodeOnLoad();
			//		if (curnode) {
			//			curbone->GetFBXAnim((int)GetFromBvhFlag(), curnode, animno, curmotid, animleng, false);
			//		}
			//	}
			//}

			//マルチスレッド読み込み
			//if ((m_LoadFbxAnim != NULL) && (m_bonelist.size() >= (LOADFBXANIMTHREAD * 4))) {
			if ((m_LoadFbxAnim != NULL) && (m_bonelist.size() >= 1)) {
				int loadcount;
				for (loadcount = 0; loadcount < m_creatednum_loadfbxanim; loadcount++) {
					CThreadingLoadFbx* curload = m_LoadFbxAnim + loadcount;
					//curload->LoadFbxAnim(animno, curmotid, (animleng - 1));
					curload->LoadFbxAnim(animno, curmotid, animleng);//2022/10/21 : 最終フレームのモーションフレーム無し問題対応
				}

				WaitLoadFbxAnimFinished();//読み込み終了待ち
				

				//#### 2022/11/01 ###########################################################################
				//CBone::GetFbxAnimがlocalではなくglobal計算になったので　Post処理は不要になった
				//CBone::SetFirstMot()は　CBone::GetFbxAnimに移動　SetLocalMatは都度計算するのでここでは不要
				//ローカルオイラー角情報は　必要時に　Main.cppのrefreshEulerGraph()で計算される　
				//###########################################################################################

				//#### 2023/01/31 ####################################################################
				//読み込み時にLocalEulとLimitedLocalEulの初期化をするべきなので　復活
				//####################################################################################
				PostLoadFbxAnim(curmotid);//並列化出来なかった計算をする


				//2023/02/11
				int errorcount = 0;
				CreateIndexedMotionPointReq(GetTopBone(), curmotid, animleng, &errorcount);
				if (errorcount != 0) {
					_ASSERT(0);
				}

			}
			else {
			}
			if (animno == (lAnimStackCount - 1)) {
				DestroyLoadFbxAnim();
			}


			//motioncreatebatchflagが立っていた場合ここまで
			//if (motioncachebatchflag == FALSE) {


				//FillUpEmptyKeyReq(curmotid, (animleng - 1), m_topbone, 0);//nullジョイント対策？ 初期姿勢で初期化する処理はmotid==1以外のときにAddMotion内で行う


			if (animno == 0) {
				//2022/11/01 Comment out
				//CallF(CreateFBXShape(mCurrentAnimLayer, animleng, mStart, mFrameTime2), return 1);//2022/10/21 : 最終フレームのモーションフレーム無し問題対応
			}

			SetCurrentMotion(curmotid);
			//}
		}
	}

	

	return 0;
}


//int CModel::AddMotionPointAll(int srcmotid, double animleng)
//{
//	map<int, CBone*>::iterator itrbone;
//	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
//		CBone* curbone = itrbone->second;
//		if (curbone) {
//			int result = curbone->AddMotionPointAll(srcmotid, animleng);
//			if (result != 0) {
//				_ASSERT(0);
//				return result;
//			}
//		}
//	}
//	return 0;
//}


void CModel::CreateIndexedMotionPointReq(CBone* srcbone, int srcmotid, double srcanimleng, int* perrorcount)
{
	if (!srcbone || !perrorcount) {
		return;
	}


	int result;
	result = srcbone->CreateIndexedMotionPoint(srcmotid, srcanimleng);
	//_ASSERT(result == 0);
	if (result != 0) {
		(*perrorcount)++;
	}
	if (srcbone->GetChild()) {
		CreateIndexedMotionPointReq(srcbone->GetChild(), srcmotid, srcanimleng, perrorcount);
	}
	if (srcbone->GetBrother()) {
		CreateIndexedMotionPointReq(srcbone->GetBrother(), srcmotid, srcanimleng, perrorcount);
	}

}



//void CModel::CreateFBXAnimReq( int animno, FbxScene* pScene, FbxPose* pPose, FbxNode* pNode, int motid, double animleng )
//{
//	//static int dbgcnt = 0;
//
//	//int lSkinCount;
//
//	if (!pNode) {
//		return;
//	}
//
//
//	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
//	if ( pAttrib ) {
//		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();
//
//		switch ( type )
//		{
////			case FbxNodeAttribute::eMesh:
////			case FbxNodeAttribute::eNURB:
////			case FbxNodeAttribute::eNURBS_SURFACE:
//			case FbxNodeAttribute::eSkeleton:
//				{
//					//GetFBXAnim( animno, pNode, motid, animleng );
//
//					if (m_LoadFbxAnim) {
//						//マルチスレッド読み込み準備
//						char bonename2[256];
//						strcpy_s(bonename2, 256, pNode->GetName());
//						TermJointRepeats(bonename2);
//						CBone* curbone = m_bonename[(char*)bonename2];
//						if (curbone) {
//							int threadcount;
//							int bonenointhread;
//							int bonenum;
//							int maxbonenuminthread;
//
//							bonenum = m_bonelist.size();
//							maxbonenuminthread = bonenum / m_creatednum_loadfbxanim + 1;
//							threadcount = m_loadbonecount / maxbonenuminthread;
//							CThreadingLoadFbx* curload = m_LoadFbxAnim + threadcount;
//							if (curload) {
//								bonenointhread = curload->GetBoneNum();
//								curload->SetBoneList(bonenointhread, pNode, curbone);
//								m_loadbonecount++;
//							}
//						}
//					}
//				}
//				break;
//			default:
//				break;
//		}
//	}
//
//	int childNodeNum;
//	childNodeNum = pNode->GetChildCount();
//	for ( int i = 0; i < childNodeNum; i++ )
//	{
//		FbxNode *pChild = pNode->GetChild(i);  // 子ノードを取得
//		CreateFBXAnimReq( animno, pScene, pPose, pChild, motid, animleng );
//	}
//
//	return;
//}


//int CModel::GetFreeThreadIndex()
//{
//	//４つのスレッドの中から空きスレッドを探す、なければ１つ以上スレッドが終了するのを待って終了したスレッドインデックスを返す
//
//	int chkindex;
//	for (chkindex = 0; chkindex < 6; chkindex++) {
//		HANDLE chkhandle = m_arhthread[chkindex];
//		if (chkhandle == 0) {
//			return chkindex;
//		}
//	}
//
//	//空きがみつからなかったとき
//	DWORD dwstatus;
//	dwstatus = WaitForMultipleObjects(6, m_arhthread, FALSE, INFINITE);
//
//	unsigned int signaledstatus;
//	signaledstatus = dwstatus & 0x0000007F;
//
//	int signaledindex = signaledstatus - WAIT_OBJECT_0;
//	if((signaledindex >= 0) && (signaledindex < 6)) {
//		
//		m_arhthread[signaledindex] = 0;
//		
//		FUNCMPPARAMS* curparams = m_armpparams[signaledindex];
//		if (curparams) {
//			free(curparams);
//		}
//		m_armpparams[signaledindex] = 0;
//
//		return signaledindex;
//	}
//	else {
//		return -1;
//	}
//
//	return -1;
//}
//
//void CModel::WaitAllTheadOfGetFbxAnim()
//{
//	int threadnum = 0;
//	HANDLE remainthread[6] = { 0, 0, 0, 0, 0, 0 };
//	int chkno;
//	for (chkno = 0; chkno < 6; chkno++) {
//		if (m_arhthread[chkno] != 0) {
//			remainthread[threadnum] = m_arhthread[chkno];
//			threadnum++;
//		}
//	}
//
//	if (threadnum >= 1) {
//		WaitForMultipleObjects(threadnum, remainthread, TRUE, INFINITE);
//
//		int paramno;
//		for (paramno = 0; paramno < 6; paramno++) {
//
//			FUNCMPPARAMS* curparams = m_armpparams[paramno];
//			if (curparams) {
//				free(curparams);
//			}
//		}
//	}
//
//	int resetindex;
//	for (resetindex = 0; resetindex < 6; resetindex++) {
//		m_armpparams[resetindex] = 0;
//		m_arhthread[resetindex] = 0;
//	}
//}


//#################################################
// マルチスレッド化のためにCBoneに移動 2022/08/14
//#################################################

//int CModel::GetFBXAnim(int animno, FbxNode* pNode, int motid, double animleng, bool callingbythread) // default : callingbythread = false
//{
//	//const char* bonename = ((FbxNode*)cluster->GetLink())->GetName();
//	char bonename2[256];
//	//strcpy_s(bonename2, 256, clusterlink->GetName());
//	strcpy_s(bonename2, 256, pNode->GetName());
//	TermJointRepeats(bonename2);
//	CBone* curbone = m_bonename[ (char*)bonename2 ];
//
//	WCHAR wname[256]={0L};
//	ZeroMemory( wname, sizeof( WCHAR ) * 256 );
//	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)bonename2, 256, wname, 256 );
//
//	if( curbone && !curbone->GetGetAnimFlag() && pNode){
//		curbone->SetGetAnimFlag( 1 );
//
//		int iships = 0;
//		char* phips = strstr(bonename2, "Hips");
//		if (phips) {
//			iships = 1;
//		}
//		else {
//			iships = 0;
//		}
//
//
//
//
//		FbxTime fbxtime;
//		fbxtime.SetSecondDouble(0.0);
//		FbxTime difftime;
//		difftime.SetSecondDouble(1.0 / 30);
//		double framecnt;
//		//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
//		for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//関数呼び出し時にanimleng - 1している
//
//		//##################################
//		//calclate motion by local info
//		//##################################
//
//			FbxAMatrix correctscalemat;
//			correctscalemat.SetIdentity();
//			FbxAMatrix currentmat;
//			currentmat.SetIdentity();
//			FbxAMatrix parentmat;
//			parentmat.SetIdentity();
//			//const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eDestinationPivot);
//			//const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eDestinationPivot);
//			//const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eDestinationPivot);
//			const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot);
//			const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot);
//			const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot);
//			FbxAMatrix lSRT = pNode->EvaluateLocalTransform(fbxtime, FbxNode::eSourcePivot);
//			FbxAMatrix lGlobalSRT = pNode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot);
//
//
//			ChaVector3 chatra = ChaVector3((float)lT2[0], (float)lT2[1], (float)lT2[2]);
//			ChaVector3 chaeul = ChaVector3((float)lR2[0], (float)lR2[1], (float)lR2[2]);
//			ChaVector3 chascale = ChaVector3((float)lS2[0], (float)lS2[1], (float)lS2[2]);
//			ChaMatrix chaSRT;
//			chaSRT = ChaMatrixFromFbxAMatrix(lSRT);
//			ChaMatrix chaGlobalSRT;
//			chaGlobalSRT = ChaMatrixFromFbxAMatrix(lGlobalSRT);
//
//			
//			//if ((GetHasBindPose() == 0) && (framecnt == 0.0) && (animno == 0)) {
//			//	//#########################################################################################
//			//	//2022/07/28-2022/07/29  bindpose無し、0フレームアニメ在りの場合のジョイント位置計算し直し
//			//	//#########################################################################################
//
//			//	//ChaMatrix firstSRT = chascalemat * charotmat * chatramat;
//			//	ChaMatrix firstSRT = chaSRT;
//			//	curbone->SetFirstSRT(firstSRT);
//
//			//	
//			//	ChaMatrix globalSRT;
//			//	ChaMatrix parentglobalSRT;
//			//	parentglobalSRT = curbone->CalcParentGlobalSRT();
//			//	globalSRT = firstSRT * parentglobalSRT;
//			//	curbone->SetFirstGlobalSRT(globalSRT);
//
//
//			//	ChaMatrix newnodemat = curbone->GetNodeMat() * ChaMatrixInv(globalSRT);
//			//	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);
//			//	ChaVector3 truefpos;
//			//	ChaVector3TransformCoord(&truefpos, &zeropos, &newnodemat);
//
//
//			//	FbxAMatrix fbxmat;
//			//	fbxmat.SetIdentity();
//			//	fbxmat.SetRow(0, FbxVector4(newnodemat._11, newnodemat._12, newnodemat._13, newnodemat._14));
//			//	fbxmat.SetRow(1, FbxVector4(newnodemat._21, newnodemat._22, newnodemat._23, newnodemat._24));
//			//	fbxmat.SetRow(2, FbxVector4(newnodemat._31, newnodemat._32, newnodemat._33, newnodemat._34));
//			//	fbxmat.SetRow(3, FbxVector4(newnodemat._41, newnodemat._42, newnodemat._43, newnodemat._44));
//			//	
//			//	curbone->SetPositionFound(true);//!!!
//			//	//curbone->SetPositionFound(false);//!!!
//			//	curbone->SetNodeMat(newnodemat);
//			//	curbone->SetGlobalPosMat(fbxmat);
//			//	curbone->SetJointFPos(truefpos);
//			//	//curbone->SetJointWPos(truefpos);//0frameのアニメ付きジョイント位置を保持するため、ここでは更新しない。
//			//}
//
//
//
//
//
//			//####################
//			//calc joint position
//			//####################
//			ChaVector3 jointpos;
//			jointpos = curbone->GetJointFPos();
//			ChaVector3 parentjointpos;
//			if (curbone->GetParent()) {
//				parentjointpos = curbone->GetParent()->GetJointFPos();
//			}
//			else {
//				parentjointpos = ChaVector3(0.0f, 0.0f, 0.0f);
//			}
//
//			//##############
//			//calc rotation
//			//##############
//			CQuaternion chaq;
//			chaq.SetRotationXYZ(0, chaeul);
//			ChaMatrix charotmat;
//			charotmat = chaq.MakeRotMatX();
//
//			ChaMatrix befrotmat, aftrotmat;
//			ChaMatrixTranslation(&befrotmat, -jointpos.x, -jointpos.y, -jointpos.z);
//			ChaMatrixTranslation(&aftrotmat, jointpos.x, jointpos.y, jointpos.z);
//
//
//
//			//#################
//			//calc translation
//			//#################
//			ChaMatrix chatramat;
//			ChaMatrixIdentity(&chatramat);
//			ChaMatrixTranslation(&chatramat, chatra.x - jointpos.x + parentjointpos.x, chatra.y - jointpos.y + parentjointpos.y, chatra.z - jointpos.z + parentjointpos.z);
//			
//
//			//##############
//			//calc scalling
//			//##############
//			ChaMatrix chascalemat;
//			ChaMatrixScaling(&chascalemat, chascale.x, chascale.y, chascale.z);
//
//
//
//			//Set Local frame0
//			if (framecnt == 0.0) {
//				curbone->SetLocalR0(chaq);
//				curbone->SetLocalT0(chatramat);
//				curbone->SetLocalS0(chascalemat);
//				curbone->SetFirstSRT(chaSRT);
//			}
//
//
//			//##############
//			//calc localmat
//			//##############
//			ChaMatrix localmat;
//			ChaMatrixIdentity(&localmat);
//			ChaMatrix globalmat;
//			ChaMatrixIdentity(&globalmat);
//
//			CMotionPoint* curmp = 0;
//			int existflag = 0;
//			curmp = curbone->AddMotionPoint(motid, framecnt, &existflag);
//			if (!curmp) {
//				_ASSERT(0);
//				return 1;
//			}
//
//			curmp->SetSRT(chaSRT);
//			//curmp->SetSRT(befrotmat * chaSRT);//!!!!!!!!!!!!!
//
//
//			if (GetHasBindPose()) {
//				//######################################
//				// バインドポーズがある場合
//				//######################################
//
//				localmat = befrotmat * chascalemat * charotmat * aftrotmat * chatramat;
//
//
//				//#############
//				//set localmat
//				//#############
//				curmp->SetLocalMat(localmat);//anglelimit無し
//
//
//
//				if (callingbythread == false) {//並列化していない場合に実行。並列化時には他のboneの姿勢に依存しないように。
//					//###############
//					//calc globalmat
//					//###############
//					ChaMatrix parentglobalmat;
//					//parentglobalmat = curbone->CalcParentGlobalMat(motid, framecnt);//間にモーションを持たないジョイントが入っても正しくするためにこの関数で再帰計算する必要あり
//					if (curbone->GetParent()) {
//						parentglobalmat = curbone->GetParent()->GetWorldMat(motid, framecnt);
//					}
//					else {
//						ChaMatrixIdentity(&parentglobalmat);
//					}
//
//					globalmat = localmat * parentglobalmat;
//					curmp->SetWorldMat(globalmat);//anglelimit無し
//				}
//
//			}
//			else {
//				//############################################################
//				// バインドポーズが無い場合
//				// 0フレームにアニメーションが設定してあっても正常に再生可能
//				//############################################################
//
//
//				//###############
//				//calc globalmat
//				//###############
//				globalmat = (ChaMatrixInv(curbone->GetNodeMat())* chaGlobalSRT);
//				curmp->SetWorldMat(globalmat);//anglelimit無し
//
//
//
//				if (callingbythread == false) {//並列化していない場合に実行。並列化時には他のboneの姿勢に依存しないように。
//
//					//#############
//					//set localmat
//					//#############
//					ChaMatrix parentglobalmat;
//					//parentglobalmat = curbone->CalcParentGlobalMat(motid, framecnt);//間にモーションを持たないジョイントが入っても正しくするためにこの関数で再帰計算する必要あり
//					if (curbone->GetParent()) {
//						parentglobalmat = curbone->GetParent()->GetWorldMat(motid, framecnt);
//					}
//					else {
//						ChaMatrixIdentity(&parentglobalmat);
//					}
//					localmat = globalmat * ChaMatrixInv(parentglobalmat);
//					curmp->SetLocalMat(localmat);//anglelimit無し
//				}
//			}
//
//
//
//
//				//#########################################################
//				//説明しよう
//				//Fbxのローカル姿勢表現とMameBake3Dのローカル姿勢表現の互換について
//				//#########################################################
//
//				//fbxはひとつのボーンの姿勢を T * S * Rであらわす
//				//２つのボーンを表現すると　T1 S1 R1  T2 S2 R2 これは実は省略形というか効率化した形式
//
//				//本ソフトでは省略しない表現で計算していて　-T0 S0 R0 T0 Tanim_0  -Ta Sa Ra Ta Tanim_a  -Tb Sb Rb Tb Tanim_b　と解釈する
//
//				//この２つの表現形式は互換である。
//
//				//省略形を表現しなおすと　　...(T0 Tanim_a -Ta) Sa Ra   (Ta Tanim_b -Tb) Sb Rb    (Tb Tanim_c -Tc) ....
//				// LclTranslation = T0 + Tanim_a - Ta = parentjointpos + Tanim_a - jointpos (式１)
//				// 
//				//求めたいのは省略しない形式における-T S R T の後ろに掛けるTanim_aであり　式１より　Tanim_a = LclTranslation - jointpos + parentjointpos (式２)となる（トリッキー）
//				// 
//				// 
//				//-Ta Sa Ra Ta Tanim_a のTanim_aが式２により求まった。
//				//この関数内では　-Ta = befrotmat, Sa = chascalemat, Ra = charotmat, Ta = aftrotmat, Tanim_a = chatramat.
//				
//
//				//ここで実はものすごいトリッキーなことが起こった？？？？？
//				//SRの前にTanimとSRの後に掛けるTanimとは同じ記号で書いたが同じとは限らない。そのようになるように(TanimにSRを施してTanim_aになるように)計算しているのであろう.たぶん.
//				//##### ここまで説明してよく分からないということがわかった。 (試行錯誤の苦行によりなんとなく出来たのである) #####	
//				
//
//
//
//			//##################
//			//For old version
//			//##################
//			if ((animno == 0) && (framecnt == 0.0)) {
//				curbone->SetFirstMat(globalmat);
//				curbone->SetInitMat(globalmat);
//				//ChaMatrix calcmat = curbone->GetNodeMat() * curbone->GetInvFirstMat();
//
//				//curbone->SetFirstMat(curbone->GetNodeMat());//!!!!!!!!!! 2022/07/29
//				//curbone->SetInitMat(curbone->GetNodeMat());//!!!!!!!!!! 2022/07/29
//				ChaMatrix calcmat = curbone->GetNodeMat();//!!!!!!!!!! 2022/07/29
//
//				ChaVector3 zeropos(0.0f, 0.0f, 0.0f);
//				ChaVector3 tmppos;
//				ChaVector3TransformCoord(&tmppos, &zeropos, &calcmat);
//				curbone->SetOldJointFPos(tmppos);
//			}
//				  
//
//			////############################
//			////Check For Debug
//			////############################
//			// 
//			// Tanim_a = LclTranslation - jointpos + parentjointpos (式２)の確認
//			// 		
//			//ChaVector3 traanim;
//			//traanim = curbone->CalcLocalTraAnim(motid, framecnt);
//
//			//ChaVector3 calcTraAnim;
//			//calcTraAnim = chatra - jointpos + parentjointpos;
//
//			//if (wcsstr(m_filename, L"yuri") != 0) {
//			//	WCHAR strdbg[4098];
//			//	swprintf_s(strdbg, 4098, L"#### framecnt %lf, bonename %s, LclTranslation : (%f, %f, %f), traanim : (%f, %f, %f), CalcTraAnim : (%f, %f, %f)\r\n",
//			//		framecnt, curbone->GetWBoneName(), chatra.x, chatra.y, chatra.z, traanim.x, traanim.y, traanim.z,
//			//		calcTraAnim.x, calcTraAnim.y, calcTraAnim.z);
//			//	DbgOut(strdbg);
//			//}
//
//			// Check OK.
//
//
//			//###########
//			//step time
//			//###########
//			fbxtime = fbxtime + difftime;
//
//			Sleep(0);
//		}
//
//	}
//
//	return 0;
//}

//void CModel::PostLoadNullNodeReq(CBone* srcbone, int srcmotid, double animlen)
//{
//	if (srcbone) {
//
//		if ((srcbone->GetType() == FBXBONE_NULL) && (srcbone->GetHasMotionCurve() == false)) {
//			double curframe;
//			for (curframe = 0.0; curframe < animlen; curframe += 1.0) {//関数呼び出し時にanimleng - 1している
//				//ChaMatrix curwm = srcbone->GetWorldMat(false, srcmotid, curframe, 0);
//				//ChaMatrix curnodemat = srcbone->GetNodeMat();
//				//ChaMatrix newnodemat = srcbone->GetNodeAnimMat();
//				//ChaMatrix newwm = ChaMatrixInv(newnodemat) * curnodemat * curwm;
//				ChaMatrix newwm;
//				newwm.SetIdentity();
//				srcbone->UpdateCurrentWM(false, srcmotid, curframe, newwm);
//			}
//		}
//
//		if (srcbone->GetChild()) {
//			PostLoadNullNodeReq(srcbone->GetChild(), srcmotid, animlen);
//		}
//		if (srcbone->GetBrother()) {
//			PostLoadNullNodeReq(srcbone->GetBrother(), srcmotid, animlen);
//		}
//	}
//
//}

int CModel::PostLoadFbxAnim(int srcmotid)
{
	MOTINFO* curmi = GetMotInfo(srcmotid);
	if (curmi) {
		double animlen = curmi->frameleng;

		//CBone* hipsbone = 0;
		//GetHipsBoneReq(GetTopBone(), &hipsbone);
		//if (hipsbone) {
		//	double curframe;
		//	for (curframe = 0.0; curframe < animlen; curframe += 1.0) {//関数呼び出し時にanimleng - 1している
		//		ChaMatrix newwm = hipsbone->GetOffsetMat() * hipsbone->GetWorldMat(false, srcmotid, curframe, 0);
		//		hipsbone->UpdateCurrentWM(false, srcmotid, curframe, newwm);
		//	}
		//}
		PostLoadFbxAnimReq(srcmotid, animlen, m_topbone);

		//PostLoadNullNodeReq(GetTopBone(), srcmotid, animlen);

	}
	return 0;
}

void CModel::PostLoadFbxAnimReq(int srcmotid, double animlen, CBone* srcbone)
{
	if (srcbone) {
		double curframe;
		for (curframe = 0.0; curframe < animlen; curframe += 1.0) {//関数呼び出し時にanimleng - 1している

			CMotionPoint* curmp;
			curmp = srcbone->GetMotionPoint(srcmotid, curframe);
			if (curmp) {				
				ChaMatrix globalmat;
				globalmat = curmp->GetWorldMat();

				//if (isfirstmot && (curframe == 0.0)) {
				//	srcbone->SetFirstMat(globalmat);
				//}

				//#############
				//set localmat
				//#############
				ChaMatrix parentglobalmat;
				//parentglobalmat = curbone->CalcParentGlobalMat(motid, framecnt);//間にモーションを持たないジョイントが入っても正しくするためにこの関数で再帰計算する必要あり
				if (srcbone->GetParent()) {
					CMotionPoint* parentmp = srcbone->GetParent()->GetMotionPoint(srcmotid, curframe);
					if (parentmp) {
						parentglobalmat = parentmp->GetWorldMat();
					}
					else {
						parentglobalmat.SetIdentity();
					}
				}
				else {
					ChaMatrixIdentity(&parentglobalmat);
				}
				ChaMatrix localmat = globalmat * ChaMatrixInv(parentglobalmat);
				curmp->SetLocalMat(localmat);//anglelimit無し

				////for debug
				//if ((curframe <= 2.1) && (strstr(srcbone->GetBoneName(), "Root") != 0)) {
				//	_ASSERT(0);
				//}

				bool limitdegflag = false;//unlimitedで計算してunlimitedにセット
				//2023/01/31
				ChaVector3 cureul = srcbone->CalcLocalEulXYZ(limitdegflag, -1, srcmotid, curframe, BEFEUL_BEFFRAME);
				curmp->SetLocalEul(cureul);
				curmp->SetLimitedLocalEul(cureul);
				curmp->SetCalcLimitedWM(2);
			}
		}

		//#####################################################################################################
		//念のために　ジョイントの向きを強制リセットしていたころの　ソースをコメントアウトして残す　2022/10/31
		//#####################################################################################################
		//if ((bvhflag == 0) &&
		//	GetHasBindPose()) {
		//
		//
		//	//######################################
		//	// バインドポーズがある場合
		//	//######################################
		//
		//	ChaMatrix localmat;
		//	localmat = curmp->GetLocalMat();
		//
		//	//###############
		//	//calc globalmat
		//	//###############
		//	ChaMatrix parentglobalmat;
		//	//parentglobalmat = curbone->CalcParentGlobalMat(motid, framecnt);//間にモーションを持たないジョイントが入っても正しくするためにこの関数で再帰計算する必要あり
		//	if (srcbone->GetParent()) {
		//		parentglobalmat = srcbone->GetParent()->GetWorldMat(srcmotid, curframe);
		//	}
		//	else {
		//		ChaMatrixIdentity(&parentglobalmat);
		//	}
		//
		//	ChaMatrix globalmat = localmat * parentglobalmat;
		//	curmp->SetWorldMat(globalmat);//anglelimit無し
		//
		//
		//	if (isfirstmot && (curframe == 0.0)) {
		//		srcbone->SetFirstMat(globalmat);
		//	}
		//
		//}


		if (srcbone->GetChild()) {
			PostLoadFbxAnimReq(srcmotid, animlen, srcbone->GetChild());
		}
		if (srcbone->GetBrother()) {
			PostLoadFbxAnimReq(srcmotid, animlen, srcbone->GetBrother());
		}
	}
}


//int CModel::CorrectFbxScaleAnim(int animno, FbxScene* pScene, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute* pAttrib, int motid, double animleng)
//{
//	//const char* bonename = ((FbxNode*)cluster->GetLink())->GetName();
//	char bonename2[256];
//	//strcpy_s(bonename2, 256, clusterlink->GetName());
//	strcpy_s(bonename2, 256, pNode->GetName());
//	TermJointRepeats(bonename2);
//	CBone* curbone = m_bonename[(char*)bonename2];
//
//	WCHAR wname[256] = { 0L };
//	::ZeroMemory(wname, sizeof(WCHAR) * 256);
//	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)bonename2, 256, wname, 256);
//
//	if (curbone && !curbone->GetGetAnimFlag() && pNode) {
//		curbone->SetGetAnimFlag(1);
//
//		FbxTime fbxtime;
//		fbxtime.SetSecondDouble(0.0);
//		FbxTime difftime;
//		difftime.SetSecondDouble(1.0 / 30);
//		double framecnt;
//		for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
//
//			//#######################
//			//calclate correct scale
//			//#######################
//				FbxAMatrix correctscalemat;
//				correctscalemat.SetIdentity();
//				FbxAMatrix currentmat;
//				currentmat.SetIdentity();
//				FbxAMatrix parentmat;
//				parentmat.SetIdentity();
//				const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eDestinationPivot);
//				const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eDestinationPivot);
//				const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eDestinationPivot);
//				//FbxVector4 lT3 = FbxVector4(0.0, 0.0, 0.0, 1.0);
//				//currentmat.SetTRS(lT3, lR2, lS2);
//				//if (curbone->GetParent() && pNode->GetParent()) {
//				//	CMotionPoint* parentmp = curbone->GetParent()->GetMotionPoint(motid, framecnt);
//				//	if (parentmp) {
//				//		ChaMatrix parentworldmat = parentmp->GetWorldMat();
//				//		ChaMatrix2FbxAMatrix(parentmat, parentworldmat);
//				//		correctscalemat = parentmat * currentmat;//scaleは合っている
//				//	}
//				//	else {
//				//		correctscalemat = currentmat;
//				//	}
//				//}
//				//else {
//				//	correctscalemat = currentmat;
//				//}
//				//const FbxVector4 correctscale = correctscalemat.GetS();
//
//			////#############################
//			////adjust scale and set motion
//			////#############################
//
//				//正しいスケールで姿勢をセットし直し
//				ChaVector3 chascale = ChaVector3((float)lS2[0], (float)lS2[1] , (float)lS2[2]);
//				ChaVector3 chatra = ChaVector3((float)lT2[0], (float)lT2[1], (float)lT2[2]);
//				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//				int paraxiskind = -1;//2021/11/18
//				cureul = curbone->CalcLocalEulXYZ(paraxiskind, motid, framecnt, BEFEUL_BEFFRAME);
//				int inittraflag1 = 0;
//				int setchildflag1 = 0;
//				int initscaleflag = 1;//!!!!!!!!!!!!
//				ChaMatrix befwm = curbone->GetWorldMat(motid, framecnt);
//				curbone->SetWorldMatFromEulAndScaleAndTra(inittraflag1, setchildflag1, befwm, cureul, chascale, chatra, motid, framecnt);
//
//
//			fbxtime = fbxtime + difftime;
//
//		}
//
//	}
//
//	return 0;
//}




void CModel::CreateFBXSkinReq( FbxNode* pNode )
{
	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();

		switch ( type )
		{
			case FbxNodeAttribute::eMesh:
//			case FbxNodeAttribute::eNURB:
//			case FbxNodeAttribute::eNURBS_SURFACE:

				GetFBXSkin( pAttrib, pNode );     // メッシュを作成
				break;
			default:
				break;
		}
	}

	int childNodeNum;
	childNodeNum = pNode->GetChildCount();
	for ( int i = 0; i < childNodeNum; i++ )
	{
		FbxNode *pChild = pNode->GetChild(i);  // 子ノードを取得
		CreateFBXSkinReq( pChild );
	}

	return;
}
int CModel::GetFBXSkin( FbxNodeAttribute *pAttrib, FbxNode* pNode )
{
	//const char* nodename = pNode->GetName();

	FbxMesh *pMesh = (FbxMesh*)pAttrib;
	CMQOObject* newobj = 0;
	newobj = m_objectname[ pNode->GetName() ];
	if( !newobj ){
		_ASSERT( 0 );
		return 1;
	}

//スキン
	// スキンの数を取得
	int skinCount  = pMesh->GetDeformerCount( FbxDeformer::eSkin );

	int makecnt = 0;
	if (skinCount >= 1) {
		//for ( int i = 0; i < skinCount; ++i ) {
		for (int i = 0; i < skinCount; i++) {
			// i番目のスキンを取得
			FbxSkin* skin = (FbxSkin*)(pMesh->GetDeformer(i, FbxDeformer::eSkin));

			// クラスターの数を取得
			int clusterNum = skin->GetClusterCount();
			DbgOut(L"fbx : skin : org clusternum %d\r\n", clusterNum);

			for (int j = 0; j < clusterNum; ++j) {
				// j番目のクラスタを取得
				FbxCluster* cluster = skin->GetCluster(j);

				int validflag = IsValidFbxCluster(cluster);
				if (validflag == 0) {
					continue;
				}

				//const char* bonename = ((FbxNode*)cluster->GetLink())->GetName();
				char bonename2[256];
				strcpy_s(bonename2, 256, ((FbxNode*)cluster->GetLink())->GetName());
				TermJointRepeats(bonename2);
				//			int namelen = (int)strlen( clustername );
				WCHAR wname[256];
				::ZeroMemory(wname, sizeof(WCHAR) * 256);
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, bonename2, -1, wname, 256);
				//			DbgOut( L"cluster (%d, %d), name : %s\r\n", i, j, wname );

				CBone* curbone = m_bonename[(char*)bonename2];

				if (curbone) {
					int curclusterno = newobj->GetClusterSize();

					if (curclusterno >= MAXCLUSTERNUM) {
						WCHAR wmes[256];
						swprintf_s(wmes, 256, L"１つのパーツに影響できるボーンの制限数(%d個)を超えました。読み込めません。", MAXCLUSTERNUM);
						MessageBoxW(NULL, wmes, L"ボーン数エラー", MB_OK);
						_ASSERT(0);
						return 1;
					}


					newobj->PushBackCluster(curbone);

					int pointNum = cluster->GetControlPointIndicesCount();
					int* pointAry = cluster->GetControlPointIndices();
					double* weightAry = cluster->GetControlPointWeights();

					FbxCluster::ELinkMode lClusterMode = (FbxCluster::ELinkMode)cluster->GetLinkMode();
					int index;
					float weight;
					for (int i2 = 0; i2 < pointNum; i2++) {
						// 頂点インデックスとウェイトを取得
						index = pointAry[i2];
						weight = (float)weightAry[i2];

						int isadditive;
						if (lClusterMode == FbxCluster::eAdditive) {
							isadditive = 1;
						}
						else {
							isadditive = 0;
						}

						if ((lClusterMode == FbxCluster::eAdditive) || (weight >= 0.05f)) {
							//if ((lClusterMode == FbxCluster::eAdditive)){
							newobj->AddInfBone(curclusterno, index, weight, isadditive);
						}
					}

					makecnt++;

				}
				else {
					//RootNodeという名前のジョイントが無いときにもここを通る
					//_ASSERT( 0 );
				}

			}

			newobj->NormalizeInfBone();
		}
	}
	else {
		//#################################
		//boneが　TopBoneただ１つだけの場合
		//#################################

		CBone* curbone = GetTopBone();

		if (curbone) {
			newobj->PushBackCluster(curbone);

			CPolyMesh4* pm4 = newobj->GetPm4();
			if (pm4) {
				int pointNum = pm4->GetOrgPointNum();

				int pointindex;
				float weight;
				for (pointindex = 0; pointindex < pointNum; pointindex++) {
					// 頂点インデックスとウェイトを取得

					weight = 1.0f;//topbone１つに1.0の重み

					int isadditive;
					isadditive = 1;
					int curclusterno = 0;
					newobj->AddInfBone(curclusterno, pointindex, weight, isadditive);
				}

				makecnt++;
			}
		}
		else {
			//RootNodeという名前のジョイントが無いときにもここを通る
			//_ASSERT( 0 );
		}
		newobj->NormalizeInfBone();
	}

	

	DbgOut( L"fbx skin : make cluster %d\r\n", makecnt );


	return 0;
}

int CModel::RenderRefArrow(bool limitdegflag, ID3D11DeviceContext* pd3dImmediateContext, CBone* boneptr, ChaVector4 diffusemult, int refmult, std::vector<ChaVector3> vecbonepos)
{
	if (!pd3dImmediateContext || !boneptr) {
		return 1;
	}

	if(vecbonepos.empty()){
		return 0;
	}
	int vecsize = (int)vecbonepos.size();
	if (vecsize <= 1) {
		return 0;
	}


	int vecno;
	for (vecno = 0; vecno < (vecsize - 1); vecno++) {
		ChaVector3 diffvec = vecbonepos[vecno + 1] - vecbonepos[vecno];
		double diffleng = ChaVector3LengthDbl(&diffvec);
		ChaVector3Normalize(&diffvec, &diffvec);

		ChaVector3 orgdir = ChaVector3(1.0f, 0.0f, 0.0f);

		CQuaternion rotq;
		rotq.RotationArc(orgdir, diffvec);

		ChaMatrix bmmat;
		bmmat = rotq.MakeRotMatX();

		double fscale = diffleng / 200.0f;
		ChaMatrix scalemat;
		ChaMatrixIdentity(&scalemat);
		scalemat.data[MATI_11] = (float)(fscale);
		scalemat.data[MATI_22] = (float)(fscale * (double)refmult);
		scalemat.data[MATI_33] = (float)(fscale * (double)refmult);
		bmmat = scalemat * bmmat;
		bmmat.data[MATI_41] = vecbonepos[vecno].x;
		bmmat.data[MATI_42] = vecbonepos[vecno].y;
		bmmat.data[MATI_43] = vecbonepos[vecno].z;

		g_hmWorld->SetMatrix((float*)&(bmmat.data[MATI_11]));
		//g_pEffect->SetMatrix(g_hmWorld, &(bmmat.D3DX()));
		this->UpdateMatrix(limitdegflag, &bmmat, &m_matVP);

		bool withalpha;
		if (diffusemult.w <= 0.99999f) {
			withalpha = true;
		}
		else {
			withalpha = false;
		}
		CallF(this->OnRender(withalpha, pd3dImmediateContext, 0, diffusemult), return 1);
	}

	return 0;
}



int CModel::RenderBoneMark(bool limitdegflag, ID3D11DeviceContext* pd3dImmediateContext, CModel* bmarkptr, CMySprite* bcircleptr, int selboneno, int skiptopbonemark )
{
	if( m_bonelist.empty() ){
		return 0;
	}

	MOTINFO* curmi = 0;
	int curmotid;
	double roundingframe;
	curmi = GetCurMotInfo();
	if (curmi) {
		curmotid = curmi->motid;
		roundingframe = (double)((int)(curmi->curframe + 0.0001));
	}
	else {
		return 0;
	}



	map<int, CBone*>::iterator itrb;
	for( itrb = m_bonelist.begin(); itrb != m_bonelist.end(); itrb++ ){
		CBone* curbone = itrb->second;
		if( curbone ){
			curbone->SetSelectFlag( 0 );
		}
	}

	if (selboneno > 0){
		CBone* selbone = m_bonelist[selboneno];
		if (selbone){
			SetSelectFlagReq(selbone, 0);
			selbone->SetSelectFlag(2);

			CBone* parentbone = selbone->GetParent();
			if (parentbone){
				CBtObject* curbto = FindBtObject(selbone->GetBoneNo());
				if (curbto){
					int tmpflag = selbone->GetSelectFlag() + 4;
					selbone->SetSelectFlag(tmpflag);
				}
			}
		}
	}

	//pdev->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmpAlways, 1);
	g_zcmpalways = true;

	//ボーンの三角錐表示
	if ((g_previewFlag != 5) && (g_previewFlag != 4)){
		if (g_bonemarkflag && bmarkptr){
			map<int, CBone*>::iterator itrbone;
			for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
				CBone* boneptr = itrbone->second;
				if (boneptr && !boneptr->GetSkipRenderBoneMark()){

					CMotionPoint curmp = boneptr->GetCurMp();

					CBone* childbone = boneptr->GetChild();
					while (childbone){

						CMotionPoint childmp = childbone->GetCurMp();

						int renderflag = 0;
						if (skiptopbonemark == 0){
							renderflag = 1;
						}
						else{
							CBone* parentbone = boneptr->GetParent();
							if (parentbone){
								renderflag = 1;
							}
							else{
								renderflag = 0;
							}
						}
						if (renderflag == 1){

							ChaVector3 aftbonepos;
							ChaVector3 tmpfpos = boneptr->GetJointFPos();
							ChaMatrix tmpwm = boneptr->GetWorldMat(limitdegflag, curmotid, roundingframe, &curmp);
							ChaVector3TransformCoord(&aftbonepos, &tmpfpos, &tmpwm);

							ChaVector3 aftchildpos;
							ChaVector3 tmpchildfpos = childbone->GetJointFPos();
							ChaMatrix tmpchildwm = childbone->GetWorldMat(limitdegflag, curmotid, roundingframe, &childmp);
							ChaVector3TransformCoord(&aftchildpos, &tmpchildfpos, &tmpchildwm);//2022/07/29
							//ChaVector3TransformCoord(&aftchildpos, &childbone->GetJointFPos(), &(boneptr->GetCurMp().GetWorldMat()));


							boneptr->CalcAxisMatZ(&aftbonepos, &aftchildpos);

							ChaMatrix bmmat;
							bmmat = boneptr->GetLAxisMat();// * boneptr->m_curmp.m_worldmat;


							ChaVector3 diffvec = aftchildpos - aftbonepos;
							float diffleng = (float)ChaVector3LengthDbl(&diffvec);

							float fscale;
							ChaMatrix scalemat;
							ChaMatrixIdentity(&scalemat);
							fscale = diffleng / 50.0f;
							scalemat.data[MATI_11] = fscale;
							scalemat.data[MATI_22] = fscale;
							scalemat.data[MATI_33] = fscale;

							bmmat = scalemat * bmmat;

							bmmat.data[MATI_41] = aftbonepos.x;
							bmmat.data[MATI_42] = aftbonepos.y;
							bmmat.data[MATI_43] = aftbonepos.z;

							g_hmWorld->SetMatrix((float*)&(bmmat.data[MATI_11]));
							//g_pEffect->SetMatrix(g_hmWorld, &(bmmat.D3DX()));
							bmarkptr->UpdateMatrix(limitdegflag, &bmmat, &m_matVP);
							ChaVector4 difmult;
							if (childbone->GetSelectFlag() & 2){
								difmult = ChaVector4(1.0f, 0.0f, 0.0f, 0.5f);
							}
							else{
								difmult = ChaVector4(0.25f, 0.5f, 0.5f, 0.5f);
							}
							bool withalpha = true;
							if (g_bonemarkflag) {
								CallF(bmarkptr->OnRender(withalpha, pd3dImmediateContext, 0, difmult), return 1);
							}
						}


						childbone = childbone->GetBrother();
					}
				}
			}
		}
	}


	//ボーンの剛体表示
	if ((g_previewFlag != 5) && (g_previewFlag != 4)){
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
			CBone* boneptr = itrbone->second;
			if (boneptr && !boneptr->GetSkipRenderBoneMark()){
				CBone* childbone = boneptr->GetChild();
				while (childbone){
					CRigidElem* curre = boneptr->GetRigidElem(childbone);
					if (curre){
						boneptr->CalcRigidElemParams(childbone, 0);

						//ChaMatrix worldcapsulemat = curre->GetCapsulemat(0) * GetWorldMat();
						//ChaMatrix worldcapsulemat = curre->GetCapsulematForColiShape(limitdegflag, 0) * GetWorldMat();//2023/01/18
						ChaMatrix worldcapsulemat = curre->GetCapsulematForColiShape(limitdegflag, 0);//2023/03/24 modelのwmはすでに掛かっている

						g_hmWorld->SetMatrix((float*)&(worldcapsulemat.data[MATI_11]));
						boneptr->GetCurColDisp(childbone)->UpdateMatrix(limitdegflag, &worldcapsulemat, &m_matVP);
						//g_hmWorld->SetMatrix((float*)&(curre->GetCapsulemat(0)));
						//boneptr->GetCurColDisp(childbone)->UpdateMatrix(&(curre->GetCapsulemat(0)), &m_matVP);
						ChaVector4 difmult;
						//if( boneptr->GetSelectFlag() & 4 ){
						if (childbone->GetSelectFlag() & 4){
							difmult = ChaVector4(1.0f, 0.0f, 0.0f, 0.5f);
						}
						else{
							difmult = ChaVector4(0.25f, 0.5f, 0.5f, 0.5f);
						}

						bool withalpha = true;
						if (g_rigidmarkflag) {
							//if ((curre->GetSkipflag() == 0) && srcbone->GetParent() && srcbone->GetParent()->GetParent()) {//有効にされている場合のみ表示　RootNodeなども表示しない
							if (curre->GetSkipflag() == 0) {//有効にされている場合のみ表示
								CallF(boneptr->GetCurColDisp(childbone)->OnRender(withalpha, pd3dImmediateContext, 0, difmult), return 1);
							}
						}
					}

					childbone = childbone->GetBrother();
				}



				/*
				std::map<CBone*, CRigidElem*>::iterator itrtmpmap;
				for (itrtmpmap = boneptr->GetRigidElemMapBegin(); itrtmpmap != boneptr->GetRigidElemMapEnd(); itrtmpmap++){
					CRigidElem* curre = itrtmpmap->second;
					if (curre){
						CBone* childbone = itrtmpmap->first;
						_ASSERT(childbone);
						if (childbone){
							//DbgOut( L"check!!!: curbone %s, childbone %s\r\n", boneptr->m_wbonename, childbone->m_wbonename );
							boneptr->CalcRigidElemParams(childbone, 0);
							g_pEffect->SetMatrix(g_hmWorld, &(curre->GetCapsulemat()));
							boneptr->GetCurColDisp(childbone)->UpdateMatrix(&(curre->GetCapsulemat()), &m_matVP);
							ChaVector4 difmult;
							//if( boneptr->GetSelectFlag() & 4 ){
							if (childbone->GetSelectFlag() & 4){
								difmult = ChaVector4(1.0f, 0.0f, 0.0f, 0.5f);
							}
							else{
								difmult = ChaVector4(0.25f, 0.5f, 0.5f, 0.5f);
							}
							CallF(boneptr->GetCurColDisp(childbone)->OnRender(pdev, 0, difmult), return 1);
						}
					}
				}
				*/
			}
		}
	}
	else{
		RenderCapsuleReq(limitdegflag, pd3dImmediateContext, m_topbt);
	}





	//ボーンのサークル表示
	if ((g_previewFlag != 5) && (g_previewFlag != 4)){

		if (g_bonemarkflag && bcircleptr){
			map<int, CBone*>::iterator itrbone;
			for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
				CBone* boneptr = itrbone->second;
				if (boneptr && (boneptr->GetType() == FBXBONE_NORMAL) && !boneptr->GetSkipRenderBoneMark()){

					CMotionPoint curmp = boneptr->GetCurMp();

					ChaMatrix bcmat;
					bcmat = boneptr->GetWorldMat(limitdegflag, curmotid, roundingframe, &curmp);
					//CBone* parentbone = boneptr->GetParent();
					//CBone* childbone = boneptr->GetChild();
					ChaMatrix transmat = bcmat * m_matVP;
					ChaVector3 wpos, scpos;
					ChaVector3 firstpos = boneptr->GetJointFPos();

					ChaVector3TransformCoord(&wpos, &firstpos, &bcmat);
					ChaVector3 cam2mark = wpos - g_camEye;
					ChaVector3Normalize(&cam2mark, &cam2mark);
					ChaVector3 camdir = g_camtargetpos - g_camEye;
					ChaVector3Normalize(&camdir, &camdir);
					double dot1 = ChaVector3Dot(&cam2mark, &camdir);
					if (dot1 < 0.0f) {
						//2023/03/23 カメラの後ろにあるマークが　逆さに表示されないように
						continue;
					}

					ChaVector3TransformCoord(&scpos, &firstpos, &transmat);
					scpos.z = 0.0f;
					bcircleptr->SetPos(scpos);
					ChaVector2 bsize;
					if (boneptr->GetSelectFlag() & 2){
						bcircleptr->SetColor(ChaVector4(0.0f, 0.0f, 1.0f, 0.7f));
						bsize = ChaVector2(0.050f, 0.050f);
						if (g_4kresolution) {
							bsize = bsize * 0.5f;
						}
						bcircleptr->SetSize(bsize);
					}
					else if (boneptr->GetSelectFlag() & 1){
						bcircleptr->SetColor(ChaVector4(1.0f, 0.0f, 0.0f, 0.7f));
						bsize = ChaVector2(0.025f, 0.025f);
						if (g_4kresolution) {
							bsize = bsize * 0.5f;
						}
						bcircleptr->SetSize(bsize);
					}
					else{
						bcircleptr->SetColor(ChaVector4(1.0f, 1.0f, 1.0f, 0.7f));
						bsize = ChaVector2(0.025f, 0.025f);
						if (g_4kresolution) {
							bsize = bsize * 0.5f;
						}
						bcircleptr->SetSize(bsize);
					}
					CallF(bcircleptr->OnRender(pd3dImmediateContext), return 1);

				}
			}
		}
	}
	else{
		if (g_bonemarkflag && bcircleptr){
			RenderBoneCircleReq(pd3dImmediateContext, m_topbt, bcircleptr);
		}
	}




	//pdev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	pd3dImmediateContext->OMSetDepthStencilState(g_pDSStateZCmp, 1);
	g_zcmpalways = false;

	return 0;
}

int CModel::RenderBoneCircleOne(bool limitdegflag, ID3D11DeviceContext* pd3dImmediateContext, CMySprite* bcircleptr, int selboneno)
{
	if (!pd3dImmediateContext || !bcircleptr) {
		_ASSERT(0);
		return 1;
	}
	if (selboneno < 0) {
		return 0;
	}

	MOTINFO* curmi = 0;
	int curmotid;
	double roundingframe;
	curmi = GetCurMotInfo();
	if (curmi) {
		curmotid = curmi->motid;
		roundingframe = (double)((int)(curmi->curframe + 0.0001));
	}
	else {
		return 0;
	}

	if (bcircleptr) {
		CBone* boneptr = GetBoneByID(selboneno);
		if (boneptr) {
			CMotionPoint curmp = boneptr->GetCurMp();

			ChaMatrix bcmat;
			bcmat = boneptr->GetWorldMat(limitdegflag, curmotid, roundingframe, &curmp);
			ChaMatrix transmat = bcmat * m_matVP;
			ChaVector3 scpos;
			ChaVector3 firstpos = boneptr->GetJointFPos();

			ChaVector3TransformCoord(&scpos, &firstpos, &transmat);
			scpos.z = 0.0f;
			bcircleptr->SetPos(scpos);
			ChaVector2 bsize;
			bcircleptr->SetColor(ChaVector4(1.0f, 0.0f, 0.0f, 0.7f));
			bsize = ChaVector2(0.025f, 0.025f);
			//if (g_4kresolution) {
			//	bsize = bsize * 0.5f;
			//}
			bcircleptr->SetSize(bsize);

			CallF(bcircleptr->OnRender(pd3dImmediateContext), return 1);
		}
	}
	return 0;
}

void CModel::RenderCapsuleReq(bool limitdegflag, ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto)
{
	if (!pd3dImmediateContext || !srcbto) {
		return;
	}

	CBone* srcbone = srcbto->GetBone();
	CBone* childbone = srcbto->GetEndBone();
	if (srcbone && childbone && !srcbone->GetSkipRenderBoneMark()){
		//if (srcbone->GetParent()){
			//CRigidElem* curre = srcbone->GetParent()->GetRigidElem(srcbone);
		CRigidElem* curre = srcbone->GetRigidElem(childbone);
		if (curre){
			srcbone->CalcRigidElemParams(childbone, 0);//形状データのスケールのために呼ぶ。ここでのカプセルマットは次のSetCapsuleBtMotionで上書きされる。
			srcbto->SetCapsuleBtMotion(curre);


			//btmatにはmodelのworldが考慮されたものが入っている！？
			//ChaMatrix worldcapsulemat = curre->GetCapsulemat(0) * GetWorldMat();
			//g_hmWorld->SetMatrix((float*)&(worldcapsulemat));
			//srcbone->GetCurColDisp(childbone)->UpdateMatrix(&worldcapsulemat, &m_matVP);

			
			//ChaMatrix tmpcapmat = curre->GetCapsulemat(0);
			ChaMatrix tmpcapmat = curre->GetCapsulematForColiShape(limitdegflag, 0);//2023/01/18

			g_hmWorld->SetMatrix((float*)&(tmpcapmat.data[MATI_11]));
			srcbone->GetCurColDisp(childbone)->UpdateMatrix(limitdegflag, &tmpcapmat, &m_matVP);
			ChaVector4 difmult;
			//if( boneptr->GetSelectFlag() & 4 ){
			if (childbone->GetSelectFlag() & 4){
				difmult = ChaVector4(1.0f, 0.0f, 0.0f, 0.5f);
			}
			else{
				difmult = ChaVector4(0.25f, 0.5f, 0.5f, 0.5f);
			}
			bool withalpha = true;
			//if ((curre->GetSkipflag() == 0) && srcbone->GetParent() && srcbone->GetParent()->GetParent()) {//有効にされている場合のみ表示　RootNodeなども表示しない
			if (curre->GetSkipflag() == 0) {//有効にされている場合のみ表示
				CallF(srcbone->GetCurColDisp(childbone)->OnRender(withalpha, pd3dImmediateContext, 0, difmult), return);
			}
		}
		//}
	}

	int chilno;
	for (chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++){
		CBtObject* chilbto = srcbto->GetChildBt(chilno);
		RenderCapsuleReq(limitdegflag, pd3dImmediateContext, chilbto);
	}

}


void CModel::RenderBoneCircleReq(ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto, CMySprite* bcircleptr)
{
	if (!srcbto || !bcircleptr) {
		return;
	}

	CBone* srcbone = srcbto->GetBone();
	CBone* childbone = srcbto->GetEndBone();
	if (srcbone && childbone && !srcbone->GetSkipRenderBoneMark()){
		CRigidElem* curre = srcbone->GetRigidElem(childbone);
		if (curre){
			if (childbone && (childbone->GetType() == FBXBONE_NORMAL)){

				//ChaMatrix capsulemat;
				//capsulemat = curre->GetCapsulemat();
				////CBone* parentbone = boneptr->GetParent();
				////CBone* childbone = boneptr->GetChild();
				//ChaMatrix transmat = capsulemat * m_matVP;
				//ChaVector3 scpos;
				//ChaVector3 firstpos = ChaVector3(0.0f, 0.0f, 0.0f);
				//
				//ChaVector3TransformCoord(&scpos, &firstpos, &transmat);
				ChaMatrix btmat = srcbone->GetBtMat();
				ChaMatrix transmat = btmat * m_matVP;
				ChaVector3 firstpos = childbone->GetJointFPos();
				ChaVector3 wpos, scpos;
				ChaVector3TransformCoord(&wpos, &firstpos, &btmat);

				ChaVector3 cam2mark = wpos - g_camEye;
				ChaVector3Normalize(&cam2mark, &cam2mark);
				ChaVector3 camdir = g_camtargetpos - g_camEye;
				ChaVector3Normalize(&camdir, &camdir);
				double dot1 = ChaVector3Dot(&cam2mark, &camdir);
				if (dot1 < 0.0f) {
					//2023/03/23 カメラの後ろにあるマークが　逆さに表示されないように
					return;
				}

				ChaVector3TransformCoord(&scpos, &firstpos, &transmat);
				scpos.z = 0.0f;

				bcircleptr->SetPos(scpos);
				ChaVector2 bsize;
				if (childbone->GetSelectFlag() & 2){
					bcircleptr->SetColor(ChaVector4(0.0f, 0.0f, 1.0f, 0.7f));
					bsize = ChaVector2(0.050f, 0.050f);
					bcircleptr->SetSize(bsize);
				}
				else if (childbone->GetSelectFlag() & 1){
					bcircleptr->SetColor(ChaVector4(1.0f, 0.0f, 0.0f, 0.7f));
					bsize = ChaVector2(0.025f, 0.025f);
					bcircleptr->SetSize(bsize);
				}
				else{
					bcircleptr->SetColor(ChaVector4(0.8f, 0.8f, 0.8f, 0.7f));
					bsize = ChaVector2(0.025f, 0.025f);
					bcircleptr->SetSize(bsize);
				}
				CallF(bcircleptr->OnRender(pd3dImmediateContext), return);

			}
		}
	}

	int chilno;
	for (chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++){
		CBtObject* chilbto = srcbto->GetChildBt(chilno);
		RenderBoneCircleReq(pd3dImmediateContext, chilbto, bcircleptr);
	}
}


//void CModel::SetDefaultBonePosReq( CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix ParentGlobalPosition )
//{
//	if (!curbone){
//		return;
//	}
//
//	FbxNode* pNode = m_bone2node[ curbone ];
//
//
//
//
//	FbxAMatrix lGlobalPosition;
//	bool        lPositionFound = false;//バインドポーズを書き出さない場合やHipsなどの場合は０になる？
//
//	lGlobalPosition.SetIdentity();
//
//	if (pNode) {
//		if (pPose) {
//			int lNodeIndex = pPose->Find(pNode);
//			if (lNodeIndex > -1)
//			{
//				// The bind pose is always a global matrix.
//				// If we have a rest pose, we need to check if it is
//				// stored in global or local space.
//				if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
//				{
//					lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
//				}
//				else
//				{
//					// We have a local matrix, we need to convert it to
//					// a global space matrix.
//					FbxAMatrix lParentGlobalPosition;
//
//					//if (pParentGlobalPosition)
//					if (curbone->GetParent())
//					{
//						lParentGlobalPosition = ParentGlobalPosition;
//					}
//					else
//					{
//						if (pNode->GetParent())
//						{
//							lParentGlobalPosition = GetGlobalPosition(this, pNode->GetScene(), pNode->GetParent(), pTime, pPose);
//						}
//					}
//
//					FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
//					lGlobalPosition = lParentGlobalPosition * lLocalPosition;
//				}
//
//				lPositionFound = true;
//			}
//		}
//	}
//
//	if (!lPositionFound)
//	{
//		// There is no pose entry for that node, get the current global position instead.
//
//		// Ideally this would use parent global position and local position to compute the global position.
//		// Unfortunately the equation 
//		//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
//		// does not hold when inheritance type is other than "Parent" (RSrs).
//		// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
//		if (pNode) {
//			lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
//		}
//	}
//
//	ChaMatrix nodemat;
//
//	nodemat._11 = (float)lGlobalPosition.Get(0, 0);
//	nodemat._12 = (float)lGlobalPosition.Get(0, 1);
//	nodemat._13 = (float)lGlobalPosition.Get(0, 2);
//	nodemat._14 = (float)lGlobalPosition.Get(0, 3);
//
//	nodemat._21 = (float)lGlobalPosition.Get(1, 0);
//	nodemat._22 = (float)lGlobalPosition.Get(1, 1);
//	nodemat._23 = (float)lGlobalPosition.Get(1, 2);
//	nodemat._24 = (float)lGlobalPosition.Get(1, 3);
//
//	nodemat._31 = (float)lGlobalPosition.Get(2, 0);
//	nodemat._32 = (float)lGlobalPosition.Get(2, 1);
//	nodemat._33 = (float)lGlobalPosition.Get(2, 2);
//	nodemat._34 = (float)lGlobalPosition.Get(2, 3);
//
//	nodemat._41 = (float)lGlobalPosition.Get(3, 0);
//	nodemat._42 = (float)lGlobalPosition.Get(3, 1);
//	nodemat._43 = (float)lGlobalPosition.Get(3, 2);
//	nodemat._44 = (float)lGlobalPosition.Get(3, 3);
//
//	curbone->SetPositionFound(lPositionFound);//!!!
//	curbone->SetNodeMat(nodemat);
//	curbone->SetGlobalPosMat(lGlobalPosition);
//
//	ChaVector3 zeropos(0.0f, 0.0f, 0.0f);
//	ChaVector3 tmppos;
//	ChaVector3TransformCoord(&tmppos, &zeropos, &(curbone->GetNodeMat()));
//	curbone->SetJointWPos(tmppos);
//	curbone->SetJointFPos(tmppos);
//
//
////WCHAR wname[256];
////ZeroMemory( wname, sizeof( WCHAR ) * 256 );
////MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, curbone->m_bonename, 256, wname, 256 );
////DbgOut( L"SetDefaultBonePos : %s : wpos (%f, %f, %f)\r\n", wname, curbone->m_jointfpos.x, curbone->m_jointfpos.y, curbone->m_jointfpos.z );
//
//
//	if( curbone->GetChild() ){
//		//if (curbone->GetChild()->GetChild()) {
//			SetDefaultBonePosReq(curbone->GetChild(), pTime, pPose, curbone->GetGlobalPosMat());
//		//}
//		//else {
//		//	SetDefaultBonePosReq(curbone->GetBrother(), pTime, pPose, ParentGlobalPosition);
//		//}
//	}
//	if( curbone->GetBrother() ){
//		SetDefaultBonePosReq( curbone->GetBrother(), pTime, pPose, ParentGlobalPosition );
//	}
//
//}

FbxPose* CModel::GetBindPose()
{
	//###################################################################################
	//商用のfbxにはbindposeを取得できないものがあるが、そういうfbxには手を出さない方針で
	//###################################################################################


/*
		if (pNode) {
			if (pPose) {
				int lNodeIndex = pPose->Find(pNode);
				if (lNodeIndex > -1)
				{
					// The bind pose is always a global matrix.
					// If we have a rest pose, we need to check if it is
					// stored in global or local space.
					if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
					{
						lGlobalPosition = FbxGetPoseMatrix(pPose, lNodeIndex);
					}
*/


	if (!m_pscene) {
		return 0;
	}
	FbxPose* bindpose = 0;
	FbxPose* lastpose = 0;
	int lastpindex = 0;
	FbxPose* curpose = m_pscene->GetPose(0);
	lastpose = curpose;
	int curpindex = 1;
	while (curpose){
		if (curpose->IsBindPose()){
			bindpose = curpose;//最後のバインドポーズ
		}
		lastpose = curpose;
		lastpindex = curpindex;
		curpose = m_pscene->GetPose(curpindex);
		curpindex++;
	}
	if (!bindpose){
		//::MessageBoxA(NULL, "バインドポーズがありません。", "警告", MB_OK);
		////if (lastpose) {
		////	bindpose = lastpose;
		////	char strmes[256] = { 0 };
		////	sprintf_s(strmes, 256, "%d番目のポーズをバインドポーズとして使用します。", lastpindex);
		////	::MessageBoxA(NULL, strmes, "注意", MB_OK);
		////}
		////else {
		////	//bindpose = m_pscene->GetPose(0);
		////	bindpose = 0;
		////	::MessageBoxA(NULL, "ポーズが１つもありません。", "注意", MB_OK);
		////}

		bindpose = m_pscene->GetPose(0);
		if (!bindpose) {
			bindpose = m_pscene->GetPose(-1);
			//if (!bindpose) {
			//	::MessageBoxA(NULL, "ポーズが１つもありません。", "警告", MB_OK);
			//}
		}

		SetHasBindPose(0);
		SetFromNoBindPoseFlag(true);
	}
	else {
		SetHasBindPose(1);
	}
	return bindpose;
}


int CModel::SetDefaultBonePos(FbxScene* pScene)
{
	if( !pScene || !m_topbone ){
		return 0;
	}

	FbxPose* bindpose = GetBindPose();


	FbxTime pTime;
	pTime.SetSecondDouble( 0.0 );

	//CBone* secbone = m_topbone->GetChild();
	CBone* secbone = m_topbone;

	FbxAMatrix inimat;
	inimat.SetIdentity();
	if( secbone ){
		FbxSetDefaultBonePosReq(pScene, this, secbone, pTime, bindpose, &inimat);
	}

	return 0;
}


//void CModel::FillUpEmptyKeyReq( int motid, double animleng, CBone* curbone, CBone* parentbone )
//{
//	if (!curbone){
//		return;
//	}
//
//	curbone->InitMP(motid, animleng);
//
//
//	if( curbone->GetChild() ){
//		FillUpEmptyKeyReq( motid, animleng, curbone->GetChild(), curbone );
//	}
//	if( curbone->GetBrother() ){
//		FillUpEmptyKeyReq( motid, animleng, curbone->GetBrother(), parentbone );
//	}
//
//}
//
//int CModel::FillUpEmptyMotion(int srcmotid)
//{
//
//	MOTINFO* curmi = GetMotInfo( srcmotid );
//	_ASSERT(curmi);
//	if (curmi){
//		FillUpEmptyKeyReq(curmi->motid, curmi->frameleng, m_topbone, 0);
//		return 0;
//	}
//	else{
//		_ASSERT(0);
//		return 1;
//	}
//
//}


int CModel::SetMaterialName()
{
	m_materialname.clear();

	map<int, CMQOMaterial*>::iterator itrmat;
	for( itrmat = m_material.begin(); itrmat != m_material.end(); itrmat++ ){
		CMQOMaterial* curmat = itrmat->second;
		m_materialname[ curmat->GetName() ] = curmat;
	}

	return 0;
}


int CModel::DestroyBtObject()
{
	if( m_topbt ){
		DestroyBtObjectReq( m_topbt );
	}
	m_topbt = 0;
	m_rigidbone.clear();

	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if(curbone){
			curbone->ClearBtObject();
		}
	}


	return 0;
}
void CModel::DestroyBtObjectReq( CBtObject* curbt )
{
	vector<CBtObject*> tmpvec;

	int chilno;
	for (chilno = 0; chilno < (int)curbt->GetChildBtSize(); chilno++){
		CBtObject* chilbt = curbt->GetChildBt(chilno);
		if (chilbt){
			tmpvec.push_back(chilbt);
		}
	}

	delete curbt;

	for (chilno = 0; chilno < tmpvec.size(); chilno++){
		CBtObject* chilbt = tmpvec[chilno];
		DestroyBtObjectReq(chilbt);
	}

}

/***
int CModel::CalcBtAxismat( float delta )
{
	if( !m_topbone ){
		return 0;
	}
	CalcBtAxismatReq( m_topbone, delta );//!!!!!!!!!!!!!

	return 0;
}
***/


void CModel::SetBtKinFlagReq( CBtObject* srcbto, int oncreateflag )
{
	if (!srcbto) {
		return;
	}

	CBone* srcbone = srcbto->GetBone();
	if( srcbone ){
		if (!srcbone->GetParent() || (srcbone->GetParent() && (srcbone->GetParent()->GetTmpKinematic() == false))) {
			int cmp0 = strncmp(srcbone->GetBoneName(), "BT_", 3);
			const char* pcomp1 = strstr(srcbone->GetBoneName(), "Hair1_");
			const char* pcomp2 = strstr(srcbone->GetBoneName(), "Hair2_");
			const char* pcomp3 = strstr(srcbone->GetBoneName(), "Hair3_");
			const char* pcomp4 = strstr(srcbone->GetBoneName(), "Hair4_");
			const char* pcomp5 = strstr(srcbone->GetBoneName(), "Hair5_");
			const char* pcomp6 = strstr(srcbone->GetBoneName(), "Hair6_");

			if ((cmp0 == 0) || (pcomp1 != NULL) || (pcomp2 != NULL) || (pcomp3 != NULL) || (pcomp4 != NULL) || (pcomp5 != NULL) || (pcomp6 != NULL)) {
				//2022/07/11 auto simu settings force
				srcbone->SetBtForce(1);
			}

			if (srcbone->GetBtForce() == 1) {
				if (srcbone->GetParent()) {
					CRigidElem* curre = srcbone->GetParent()->GetRigidElem(srcbone);
					if (curre) {
						//if (curre->GetSkipflag() == 0){
						srcbone->SetBtKinFlag(0);
						//}
						//else{
						//	srcbone->SetBtKinFlag(1);
						//}


					}
					else {
						srcbone->SetBtKinFlag(1);
					}
				}
				else {
					srcbone->SetBtKinFlag(1);
				}
			}
			else {
				srcbone->SetBtKinFlag(1);
			}
		}
		else {
			srcbone->SetBtKinFlag(1);
		}

		/*
		if ((srcbone->GetBtKinFlag() == 0) && srcbone->GetParent() && (srcbone->GetParent()->GetBtKinFlag() == 1)){
			CBtObject* parbto = srcbto->GetParBt();
			if (parbto){
				int constraintnum = parbto->GetConstraintSize();
				int cno;
				for (cno = 0; cno < constraintnum; cno++){
					CONSTRAINTELEM ce = parbto->GetConstraintElem(cno);
					if (ce.centerbone == srcbone){
						btGeneric6DofSpringConstraint* dofC = ce.constraint;
						if (dofC){
							//int dofid;
							//for (dofid = 0; dofid < 3; dofid++){
							//	dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
							//dofC->enableSpring(dofid, true);
							//}
							//for (dofid = 3; dofid < 6; dofid++){
							//dofC->enableSpring(dofid, true);
							//	dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
							//}
							//dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
							//dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));

							ChaVector3 aftbonepos;
							ChaVector3TransformCoord(&aftbonepos, &srcbone->GetJointFPos(), &(srcbone->GetCurMp().GetWorldMat()));

							dofC->setLinearLowerLimit(btVector3(aftbonepos.x, aftbonepos.y, aftbonepos.z));
							dofC->setLinearUpperLimit(btVector3(aftbonepos.x, aftbonepos.y, aftbonepos.z));
						}
					}
				}
			}
		}
		*/
		/*
		else{
			int constraintnum = srcbto->GetConstraintSize();
			int cno;
			for (cno = 0; cno < constraintnum; cno++){
				CONSTRAINTELEM ce = srcbto->GetConstraintElem(cno);
				if (ce.centerbone != srcbone){
					btGeneric6DofSpringConstraint* dofC = ce.constraint;
					if (dofC){
						int dofid;
						for (dofid = 0; dofid < 3; dofid++){
							//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
							dofC->enableSpring(dofid, true);
						}
						for (dofid = 3; dofid < 6; dofid++){
							dofC->enableSpring(dofid, true);
							//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
						}
						//dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
						//dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
						//dofC->setLinearLowerLimit(btVector3(0.0, 0.0, 0.0));
						//dofC->setLinearUpperLimit(btVector3(0.0, 0.0, 0.0));
					}
				}
			}
		}
		*/
		
		/*
		if ((srcbone->GetBtKinFlag() == 1) && srcbone->GetParent() && (srcbone->GetParent()->GetBtKinFlag() == 1)){
			int constraintnum = srcbto->GetConstraintSize();
			int cno;
			for (cno = 0; cno < constraintnum; cno++){
				btGeneric6DofSpringConstraint* dofC = srcbto->GetConstraint(cno);
				if (dofC){
					int dofid;
					for (dofid = 0; dofid < 3; dofid++){
						dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
						//dofC->enableSpring(dofid, true);
					}
					for (dofid = 3; dofid < 6; dofid++){
						//dofC->enableSpring(dofid, true);
						dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
					}
					dofC->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));
					dofC->setAngularUpperLimit(btVector3(0.0, 0.0, 0.0));
					dofC->setLinearLowerLimit(btVector3(0.0, 0.0, 0.0));
					dofC->setLinearUpperLimit(btVector3(0.0, 0.0, 0.0));
				}
			}
		}
		else{
			int constraintnum = srcbto->GetConstraintSize();
			int cno;
			for (cno = 0; cno < constraintnum; cno++){
				btGeneric6DofSpringConstraint* dofC = srcbto->GetConstraint(cno);
				if (dofC){
					int dofid;
					for (dofid = 0; dofid < 3; dofid++){
						//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
						dofC->enableSpring(dofid, true);
					}
					for (dofid = 3; dofid < 6; dofid++){
						dofC->enableSpring(dofid, true);
						//dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
					}

				}
			}
		}
		*/




		if( (srcbone->GetBtKinFlag() == 1) && (srcbto->GetRigidBody()) ){
			DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
			if( s_setrigidflag == 0 ){
				s_rigidflag = curflag;
				s_setrigidflag = 1;
			}
			//srcbto->GetRigidBody()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
			srcbto->GetRigidBody()->setCollisionFlags( curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
			//srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
			//srcbto->GetRigidBody()->setActivationState(WANTS_DEACTIVATION);
			//srcbto->m_rigidbody->setActivationState(DISABLE_SIMULATION);
			//CF_STATIC_OBJECT
		}else if( srcbto->GetRigidBody() ){
			DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
			srcbto->GetRigidBody()->setCollisionFlags(curflag & ~btCollisionObject::CF_KINEMATIC_OBJECT);

			if( srcbone->GetParent() ){
				CRigidElem* curre = srcbone->GetParent()->GetRigidElem( srcbone );
				if( curre ){
					if ((m_curreindex >= 0) && (m_curreindex < (int)m_rigideleminfo.size())){
						srcbto->GetRigidBody()->setGravity(btVector3(0.0f, curre->GetBtg() * m_rigideleminfo[m_curreindex].btgscale, 0.0f));
						srcbto->GetRigidBody()->applyGravity();
					}
					else{
						_ASSERT(0);
					}
				}
			}
		}

		if (srcbto->GetRigidBody()){
			//srcbto->GetRigidBody()->setDeactivationTime(0.0);
			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 150.0);
			//srcbto->GetRigidBody()->activate();
			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
		}
	}

	int chilno;
	for( chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++ ){
		CBtObject* chilbto = srcbto->GetChildBt( chilno );
		SetBtKinFlagReq( chilbto, oncreateflag );
	}
}



int CModel::BulletSimulationStop()
{
	BulletSimulationStopReq(m_topbt);
	return 0;
}

void CModel::BulletSimulationStopReq(CBtObject* srcbto)
{
	if (!srcbto)
		return;

	if (srcbto->GetRigidBody()){
		srcbto->GetRigidBody()->setActivationState(DISABLE_SIMULATION);
	}

	int chilno;
	for (chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++){
		CBtObject* chilbto = srcbto->GetChildBt(chilno);
		BulletSimulationStopReq(chilbto);
	}

}

int CModel::BulletSimulationStart()
{
	BulletSimulationStartReq(m_topbt);
	return 0;
}

void CModel::BulletSimulationStartReq(CBtObject* srcbto)
{
	if (!srcbto)
		return;

	if (srcbto->GetRigidBody()){
		srcbto->GetRigidBody()->forceActivationState(ACTIVE_TAG);

		//srcbto->GetRigidBody()->setDeactivationTime(0.0);
		////srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);

		srcbto->GetRigidBody()->setDeactivationTime(0.0f);
		srcbto->GetRigidBody()->setSleepingThresholds(0.0f, 0.0f);
		srcbto->GetRigidBody()->activate();
	}

	int chilno;
	for (chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++){
		CBtObject* chilbto = srcbto->GetChildBt(chilno);
		BulletSimulationStartReq(chilbto);
	}

}


int CModel::CreateBtConstraint(bool limitdegflag)
{
	if( !m_topbone ){
		return 0;
	}
	if( !m_topbt ){
		return 0;
	}

	CreateBtConstraintReq(limitdegflag, m_topbt);
	//CreateBtConstraintReq(m_topbone);

	//CreateBtConnectReq( m_topbone );

	return 0;
}

/*
void CModel::CreateBtConstraintReq(CBone* curbone)
{
	if (!curbone){
		return;
	}

	map<CBone*, CBtObject*>::iterator itrbto;
	for (itrbto = curbone->GetBtObjectMapBegin(); itrbto != curbone->GetBtObjectMapEnd(); itrbto++){
		CBtObject* curbto = itrbto->second;
		if (curbto){
			curbto->CreateBtConstraint();
		}
	}

	if (curbone->GetChild()){
		CreateBtConstraintReq(curbone->GetChild());
	}
	if (curbone->GetBrother()){
		CreateBtConstraintReq(curbone->GetBrother());
	}
}
*/


void CModel::CreateBtConstraintReq(bool limitdegflag, CBtObject* curbto)
{
	if(curbto->GetTopFlag() == 0){
		CallF(curbto->CreateBtConstraint(limitdegflag), return);
	}

	int btono;
	for(btono = 0; btono < (int)curbto->GetChildBtSize(); btono++){
		CBtObject* chilbto = curbto->GetChildBt(btono);
		if( chilbto ){
			CreateBtConstraintReq(limitdegflag, chilbto);
		}
	}
}

/*
void CModel::CreateBtConnectReq(CBone* curbone)
{
	if (!curbone){
		return;
	}
	if (curbone->GetChild()){
		CBone* brobone1 = curbone->GetChild()->GetBrother();
		if (brobone1){
			CBone* brobone2 = brobone1->GetBrother();
			while (brobone2){
				map<CBone*, CBtObject*>::iterator itrbto1;
				for (itrbto1 = brobone1->GetBtObjectMapBegin(); itrbto1 != brobone1->GetBtObjectMapEnd(); itrbto1++){
					CBtObject* bto1 = itrbto1->second;
					if (bto1 && bto1->GetRigidBody()){
						map<CBone*, CBtObject*>::iterator itrbto2;
						for (itrbto2 = brobone2->GetBtObjectMapBegin(); itrbto2 != brobone2->GetBtObjectMapEnd(); itrbto2++){
							CBtObject* bto2 = itrbto2->second;
							if (bto2 && bto2->GetRigidBody()){


								float angPAI2, angPAI;
								angPAI2 = 90.0f * (float)DEG2PAI;
								angPAI = 180.0f * (float)DEG2PAI;

								float lmax, lmin;
								lmax = 10000.0f;
								lmin = -10000.0f;
								//lmin = 0.0f;
								//lmax = 0.0f;
								//lmin = -1.0e-3;
								//lmax = 1.0e-3;

								btGeneric6DofSpringConstraint* dofC;
								btTransform tmpA, tmpA2;
								bto1->GetFrameA(tmpA);
								bto2->GetFrameA(tmpA2);
								dofC = new btGeneric6DofSpringConstraint(*bto1->GetRigidBody(), *bto2->GetRigidBody(), tmpA, tmpA2, true);
								_ASSERT(dofC);

								dofC->setLinearLowerLimit(btVector3(lmin, lmin, lmin));
								dofC->setLinearUpperLimit(btVector3(lmax, lmax, lmax));
								dofC->setAngularLowerLimit(btVector3(angPAI, angPAI2, angPAI));
								dofC->setAngularUpperLimit(btVector3(-angPAI, -angPAI2, -angPAI));


								//dofC->setBreakingImpulseThreshold(FLT_MAX);
								//dofC->setBreakingImpulseThreshold(0.0);//!!!!!!!!!!!!!!!!!!!!
								dofC->setBreakingImpulseThreshold(1e7);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



								int l_kindex = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetLKindex();
								int a_kindex = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetAKindex();
								float l_damping = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetLDamping();
								float a_damping = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetADamping();
								float l_cusk = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetCusLk();
								float a_cusk = bto1->GetBone()->GetRigidElem(bto1->GetEndBone())->GetCusAk();

								int dofid;
								//for (dofid = 0; dofid < 3; dofid++){
								//	dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
								//}
								//for (dofid = 3; dofid < 6; dofid++){
								//	dofC->enableSpring(dofid, false);//!!!!!!!!!!!!!
								//}
								
								for (dofid = 0; dofid < 3; dofid++){
									dofC->enableSpring(dofid, true);//!!!!!!!!!!!!!!!!!!!
									dofC->setStiffness(dofid, 1.0e12);
									//dofC->setStiffness(dofid, 1.0e6);

									dofC->setDamping(dofid, 0.5f);
								}
								for (dofid = 3; dofid < 6; dofid++){
									dofC->enableSpring(dofid, true);//!!!!!!!!!!!!!!!!!
									dofC->setStiffness(dofid, 80.0f);
									//dofC->setStiffness(dofid, 1000.0f);
									dofC->setDamping(dofid, 0.01f);
								}
								

								dofC->setEquilibriumPoint();//!!!!!!!!!!!!tmp disable

								CONSTRAINTELEM addelem;
								addelem.constraint = dofC;
								addelem.centerbone = 
								bto1->PushBackConstraint(dofC);
								//m_btWorld->addConstraint(dofC, false);//!!!!!!!!!!!! disable collision between linked bodies
								m_btWorld->addConstraint(dofC, true);
							}
						}
					}
				}

				brobone2 = brobone2->GetBrother();
			}
		}
	}

	if (curbone->GetChild()){
		CreateBtConnectReq(curbone->GetChild());
	}
	if (curbone->GetBrother()){
		CreateBtConnectReq(curbone->GetBrother());
	}
}
*/


int CModel::CreateBtObject(bool limitdegflag, int onfirstcreate )
{

	DestroyBtObject();

	if( !m_topbone ){
		return 0;
	}
	if (!m_btWorld){
		return 0;
	}

	CalcBtAxismatReq(m_topbone, 1);//!!!!!!!!!!!!!


	m_topbt = new CBtObject( 0, m_btWorld );
	if( !m_topbt ){
		_ASSERT( 0 );
		return 1;
	}
	m_topbt->SetTopFlag( 1 );
	//m_topbt->CreateObject(NULL, NULL, m_topbone, m_topbone->GetChild());


	m_rigidbone.clear();

	CBone* startbone = m_topbone;
	_ASSERT( startbone );
	if (startbone){
		CreateBtObjectReq(limitdegflag, m_topbt, startbone, startbone->GetChild());
		//CreateBtObjectReq(NULL, startbone, startbone->GetChild());
	}

	CreateBtConstraint(limitdegflag);


	//if( m_topbt ){
	//	SetBtKinFlagReq( m_topbt, onfirstcreate );
	//}


	if (g_previewFlag != 5){
		SetBtKinFlagReq(m_topbt, 1);
		 //RestoreMassReq(m_topbone);
	}
	else{
		//SetRagdollKinFlagReq(m_topbt, -1);
		//CreatePhysicsPosConstraintReq(m_topbone);
		//bool forceflag = false;
		//SetMass0Req(m_topbone, forceflag);
	}


	if (m_topbt){
		SetBtEquilibriumPointReq(limitdegflag, m_topbt);
	}


	////for debug
	//DumpBtObjectReq(m_topbt, 0);
	//DbgOut(L"\r\n\r\n");
	//DumpBtConstraintReq(m_topbt, 0);
	//DbgOut(L"\r\n\r\n");


	SetCreateBtFlag(true);

	return 0;          
}


int CModel::SetBtEquilibriumPoint(bool limitdegflag)
{
	if (m_topbt) {
		SetBtEquilibriumPointReq(limitdegflag, m_topbt);
	}
	return 0;
}

int CModel::SetBtEquilibriumPointReq(bool limitdegflag, CBtObject* srcbto)
{
	if (!srcbto){
		return 0;
	}

	//`角度、位置
	if (g_previewFlag != 5) {
		srcbto->EnableSpring(true, true);
	}
	else {
		srcbto->EnableSpring(false, false);
		//srcbto->EnableSpring(true, true);
		//srcbto->EnableSpring(false, true);
	}
	//srcbto->EnableSpring(false, true);
	//srcbto->EnableSpring(true, false);

	int lflag, aflag;
	aflag = 1;
	lflag = 0;
	srcbto->SetEquilibriumPoint(limitdegflag, lflag, aflag);

	int childno;
	for (childno = 0; childno < srcbto->GetChildBtSize(); childno++){
		CBtObject* childbto = srcbto->GetChildBt(childno);
		if (childbto){
			SetBtEquilibriumPointReq(limitdegflag, childbto);
		}
	}

	return 0;
}


int CModel::SetDofRotAxis(int srcaxiskind)
{
	if (m_topbt){
		SetDofRotAxisReq(m_topbt, srcaxiskind);
	}
	return 0;

}

void CModel::SetDofRotAxisReq(CBtObject* srcbto, int srcaxiskind)
{
	if (!srcbto){
		return;
	}

	//`角度、位置
	srcbto->SetDofRotAxis(srcaxiskind);


	int childno;
	for (childno = 0; childno < srcbto->GetChildBtSize(); childno++){
		CBtObject* childbto = srcbto->GetChildBt(childno);
		if (childbto){
			SetDofRotAxisReq(childbto, srcaxiskind);
		}
	}

	return;
}



void CModel::CreateBtObjectReq(bool limitdegflag, CBtObject* parbt, CBone* parentbone, CBone* curbone)
{
	if (!curbone){
		return;
	}
	if (!parentbone){
		return;
	}
	CRigidElem* curre;
	curre = parentbone->GetRigidElem(curbone);

	CBtObject* newbto = 0;
	CBone* childbone = 0;

	if (curre){
		newbto = new CBtObject(parbt, m_btWorld);
		if (!newbto){
			_ASSERT(0);
			return;
		}

		//if (!m_topbt){
		//	m_topbt = newbto;
		//	m_topbt->SetTopFlag( 1 );
		//}

		if (parbt){
			parbt->AddChild(newbto);

			int curmotid;
			double curframe;
			MOTINFO* curmi;
			curmi = GetCurMotInfo();
			if (curmi) {
				curmotid = curmi->motid;
				curframe = (double)((int)(curmi->curframe + 0.0001));
				CallF(newbto->CreateObject(limitdegflag, curmotid, curframe, parbt, parentbone->GetParent(), parentbone, curbone), return);
			}
			parentbone->SetBtObject(curbone, newbto);

			//if (parbt->GetBone() && parbt->GetEndBone()){
			//	DbgOut(L"checkbt2 : CreateBtObject : parbto %s---%s, curbto %s---%s\r\n",
			//		parbt->GetBone()->GetWBoneName(), parbt->GetEndBone()->GetWBoneName(),
			//		newbto->GetBone()->GetWBoneName(), newbto->GetEndBone()->GetWBoneName());
			//}

			if (curbone->GetChild()){
				CreateBtObjectReq(limitdegflag, newbto, curbone, curbone->GetChild());
			}
			if (curbone->GetBrother()){
				CreateBtObjectReq(limitdegflag, parbt, parentbone, curbone->GetBrother());
			}
		}
	}
}


void CModel::CalcRigidElem()
{
	CalcRigidElemReq(m_topbone);
}


void CModel::CalcRigidElemReq(CBone* curbone)
{
	if (!curbone) {
		return;
	}

	int onfirstcreate = 1;

	//int setstartflag;
	//if (onfirstcreate != 0) {
		if (curbone->GetParent()) {
			curbone->GetParent()->CalcRigidElemParams(curbone, onfirstcreate);//firstflag 1
		}
		////curbone->SetStartMat2( curbone->GetCurMp().GetWorldMat() );
		//curbone->SetStartMat2(curbone->GetCurrentZeroFrameMat(0));
	//}

	if (curbone->GetChild()) {
		CalcRigidElemReq(curbone->GetChild());
	}
	if (curbone->GetBrother()) {
		CalcRigidElemReq(curbone->GetBrother());
	}


}



void CModel::CalcBtAxismat(int onfirstcreate)
{
	CalcBtAxismatReq(m_topbone, onfirstcreate);
	return;
}


void CModel::CalcBtAxismatReq( CBone* curbone, int onfirstcreate )
{
	if (!curbone){
		return;
	}
	//int setstartflag;
	if( onfirstcreate != 0 ){
		if (curbone->GetParent()){
			curbone->GetParent()->CalcRigidElemParams(curbone, onfirstcreate);//firstflag 1
		}
		////curbone->SetStartMat2( curbone->GetCurMp().GetWorldMat() );
		//curbone->SetStartMat2(curbone->GetCurrentZeroFrameMat(0));
	}

	if( curbone->GetChild() ){
		CalcBtAxismatReq(curbone->GetChild(), onfirstcreate);
	}
	if( curbone->GetBrother() ){
		CalcBtAxismatReq(curbone->GetBrother(), onfirstcreate);
	}
}

int CModel::SetBtMotion(bool limitdegflag, CBone* srcbone, int ragdollflag, 
	double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat )
{
	m_matWorld = *wmat;
	m_matVP = *vpmat;

	if( !m_topbt ){
		//_ASSERT( 0 );
		return 0;
	}
	if (!m_btWorld){
		return 0;
	}
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 0;//!!!!!!!!!!!!
	}

	int curmotid = m_curmotinfo->motid;
	double curframe = m_curmotinfo->curframe;

	ChaMatrix inimat;
	ChaMatrixIdentity( &inimat );
	CQuaternion iniq;
	iniq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			//CMotionPoint curmp = curbone->GetCurMp();
			curbone->SetBtFlag( 0 );
			//curbone->SetCurMp( curmp );
		}
	}

	SetBtMotionReq(limitdegflag, m_topbt, wmat, vpmat);

	//if (g_previewFlag == 5) {
	//	//物理IK用
	//	
	//	//SetBtMotionPostReq(m_topbt, wmat, vpmat);
	//	if (srcbone && srcbone->GetParent()) {
	//		CBtObject* startbto = srcbone->GetParent()->GetBtObject(srcbone);
	//		SetBtMotionPostLowerReq(startbto, wmat, vpmat, 1);//kinematicadjust = 0
	//		SetBtMotionPostUpperReq(startbto, wmat, vpmat);
	//		//SetBtMotionPostLowerReq(m_topbt, wmat, vpmat, 0);//mass0adjust = 0
	//		SetBtMotionMass0BottomUpReq(m_topbt, wmat, vpmat);//for mass0
	//		SetBtMotionPostLowerReq(m_topbt, wmat, vpmat, 0);//kinematicadjust = 0

	//		FindAndSetKinematicReq(m_topbt, wmat, vpmat);//Kinematicとそうでないところの境目を探してみつかったらLowerReqで親行列をセットする。

	//		//InitBtMatTraAnimReq(m_topbt);//2022/12/15 comment out

	//		
	//		BtMat2BtObjReq(m_topbt, wmat, vpmat);
	//		//RecalcConstraintFrameABReq(m_topbt);

	//		m_physicsikcnt++;
	//	}
	//}
	//else {
	//	//if (srcbone && srcbone->GetParent()) {
	//	//	RecalcConstraintFrameABReq(m_topbt);
	//	//}
	//}


	//#########################################################################################
	//2022/07/09
	// endjointが頂点に影響度を持つ場合、物理時に動かないendjointに頂点が引っ張られる不具合解消
	//2022/07/11
	// SetShaderConstから本関数SetBtMotionに処理を移動。
	//#########################################################################################

	double roundingframe = (double)((int)(curframe + 0.0001));

	map<int, CBone*>::iterator itrbone2;
	for (itrbone2 = m_bonelist.begin(); itrbone2 != m_bonelist.end(); itrbone2++) {
		CBone* curbone2 = itrbone2->second;
		if (curbone2) {
			if (curbone2->GetChild() == NULL) {
				if (curbone2->GetParent()) {
					//CBtObject* startbto2 = curbone2->GetParent()->GetBtObject(curbone2);
					//if (startbto2 != NULL) {
						//if (ragdollflag == 0) {
							//CMotionPoint tmpmp1 = curbone2->GetCurMp();//motid, curframeを参照してもうなくいかない。GetCurMpを使う。
							//curbone2->SetBtMat(tmpmp1.GetWorldMat());
						//}
						//else {
						if (curbone2->GetParent() != NULL) {
							//CMotionPoint tmpmp2 = curbone2->GetParent()->GetCurMp();//motid, curframeを参照してもうなくいかない。GetCurMpを使う。
							//curbone2->SetBtMat(tmpmp2.GetWorldMat());
							curbone2->SetBtMat(curbone2->GetParent()->GetBtMat());
						}
						else {
							CMotionPoint tmpmp3 = curbone2->GetCurMp();//motid, curframeを参照してもうなくいかない。GetCurMpを使う。
							curbone2->SetBtMat(tmpmp3.GetWorldMat());
						}
						//}
					//}
				}
				else {
					CMotionPoint tmpmp4 = curbone2->GetCurMp();//motid, curframeを参照してもうなくいかない。GetCurMpを使う。
					curbone2->SetBtMat(tmpmp4.GetWorldMat());
				}
			}
		}
	}



	//if (g_previewFlag == 5){
	//	if (m_topbt){
	//		SetBtEquilibriumPointReq(m_topbt);
	//	}
	//}

	/*
	//resetbt処理との問題で一時的にコメントアウト

///// morph

	if( (ragdollflag == 1) && (m_rgdmorphid >= 0) ){
		MOTINFO* rgdmorphinfo = GetRgdMorphInfo();
		if( rgdmorphinfo ){

			double nextframe = 0.0;
			int endflag = 0;
			AdvanceTime( g_previewFlag, difftime, &nextframe, &endflag, rgdmorphinfo->motid );
			rgdmorphinfo->curframe = nextframe;

			DampAnim( rgdmorphinfo );

//DbgOut( L"!!!!! setbtmotion : rgd : morph : rgdmotid %d, curframe %f, difftime %f, frameleng %f\r\n", 
//	rgdmorphinfo->motid, rgdmorphinfo->curframe, difftime, rgdmorphinfo->frameleng );


			FbxAnimStack * lCurrentAnimationStack = m_pscene->FindMember<FbxAnimStack>(mAnimStackNameArray[m_rgdmorphid]->Buffer());
			if (lCurrentAnimationStack == NULL){
				_ASSERT( 0 );
				return 1;
			}
			FbxAnimLayer * mCurrentAnimLayer;
			mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
   

			FbxTime lTime;
			lTime.SetSecondDouble( rgdmorphinfo->curframe / 30.0 );
//			lTime.SetSecondDouble( rgdmorphinfo->curframe / 300.0 );
			//lTime.SetTime(0, 0, 0, (int)rgdmorphinfo->curframe, 0, m_pscene->GetGlobalSettings().GetTimeMode());

			map<int, CMQOObject*>::iterator itrobj;
			for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
				CMQOObject* curobj = itrobj->second;
				_ASSERT( curobj );
				if( !(curobj->EmptyFindShape()) ){
					GetShapeWeight( m_fbxobj[curobj].node, m_fbxobj[curobj].mesh, lTime, mCurrentAnimLayer, curobj );
					CallF( curobj->UpdateMorphBuffer(), return 1 );
				}
			}
		}
	}
	*/
	return 0;
}


int CModel::DampAnim( MOTINFO* rgdmorphinfo )
{
	if( !rgdmorphinfo || (m_rgdindex < 0) ){
		return 0;
	}

	double diffframe = rgdmorphinfo->curframe;

	if( m_rgdindex >= (int)m_rigideleminfo.size() ){
		_ASSERT( 0 );
		return 0;
	}

	//float minval = 0.000050f;
	float minval = 0.000005f;

	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CBone* parentbone = curbone->GetParent();
			if( parentbone ){
				CRigidElem* curre = parentbone->GetRigidElem( curbone );
				if( curre ){

					float newdampl = curre->GetLDamping() - (float)diffframe * curre->GetDampanimL() * curre->GetLDamping();
					if( newdampl < minval ){
						newdampl = minval;
					}
					float newdampa = curre->GetADamping() - (float)diffframe * curre->GetDampanimA() * curre->GetADamping();
					if( newdampa < minval ){
						newdampa = minval;
					}


					CBtObject* curbto = FindBtObject( curbone->GetBoneNo() );
					if( curbto ){
						int constraintnum = curbto->GetConstraintSize();
						int constraintno;
						for( constraintno = 0; constraintno < constraintnum; constraintno++ ){
							btGeneric6DofSpringConstraint* curct = curbto->GetConstraint(constraintno);
							if( curct ){
								int dofid;
							for( dofid = 0; dofid < 3; dofid++ ){
								curct->setDamping( dofid, newdampl );
							}
							for( dofid = 3; dofid < 6; dofid++ ){
								curct->setDamping( dofid, newdampa );
							}
							}

							//curct->setDamping( newdampa );

						}
					}

				}
			}
		}
	}

	return 0;
}



MOTINFO* CModel::GetRgdMorphInfo()
{
	MOTINFO* retmi = 0;

	int motionnum = (int)m_motinfo.size();
	if( m_rgdmorphid < motionnum ){
		map<int,MOTINFO*>::iterator itrmi;
		itrmi = m_motinfo.begin();
		int infcnt;
		for( infcnt = 0; infcnt < m_rgdmorphid; infcnt++ ){
			itrmi++;
			if( itrmi == m_motinfo.end() ){
				break;
			}
		}
		if( itrmi != m_motinfo.end() ){
			retmi = itrmi->second;
		}
	}

	return retmi;
}


//void CModel::InitBtMatTraAnimReq(CBtObject* curbto)
//{
//	if (!curbto) {
//		return;
//	}
//
//	if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
//		CBone* curbone = curbto->GetBone();
//
//		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//		int paraxsiflag1 = 1;
//		//int isfirstbone = 0;
//		cureul = curbone->CalcBtLocalEulXYZ(paraxsiflag1, BEFEUL_BEFFRAME);
//
//		int inittraflag1 = 1;
//		int setchildflag1 = 1;
//		curbone->SetBtWorldMatFromEul(setchildflag1, cureul);
//
//	}
//	int chilno;
//	for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//		CBtObject* chilbto = curbto->GetChildBt(chilno);
//		if (chilbto) {
//			InitBtMatTraAnimReq(chilbto);
//		}
//	}
//}


void CModel::SetBtMotionReq(bool limitdegflag, CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat )
{
	if (!curbto) {
		return;
	}

	if (m_curmotinfo) {
		int curmotid = m_curmotinfo->motid;
		double curframe = m_curmotinfo->curframe;//curframe : 時間補間有り

		if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
			CBone* curbone = curbto->GetBone();
			CBone* childbone = curbto->GetEndBone();

			ChaMatrix curtraanim;
			curtraanim.SetIdentity();

			//if (curbone->IsHipsBone() && (curframe >= 50.0)) {
			//	_ASSERT(0);//for debug
			//}

			//ChaMatrix curwm = curbone->GetWorldMat(curmotid, curframe);
			//ChaMatrix curwm = curbone->GetCurMp().GetWorldMat();
			ChaMatrix curwm;
			curwm.SetIdentity();
			curwm = curbone->GetCurrentWorldMat(true);

			ChaMatrix parentwm;
			parentwm.SetIdentity();
			if (curbone->GetParent()) {
				//parentwm = curbone->GetParent()->GetWorldMat(curmotid, curframe);
				//parentwm = curbone->GetParent()->GetCurMp().GetWorldMat();
				parentwm = curbone->GetParent()->GetCurrentWorldMat(true);
			}
			else {
				parentwm.SetIdentity();
			}
			ChaMatrix curlocalmat;
			curlocalmat = curwm * ChaMatrixInv(parentwm);

			ChaMatrix smat, rmat, tmat;
			GetSRTandTraAnim(curlocalmat, curbone->GetNodeMat(), &smat, &rmat, &tmat, &curtraanim);

			if (curbone) {
				if (curbone->GetBtKinFlag() == 0) {//2023/01/28
					if (g_previewFlag == 4) {
						curbto->SetBtMotion(limitdegflag, curtraanim);
					}
					else if (g_previewFlag == 5) {
						curbto->SetBtMotion(limitdegflag, curtraanim);
					}
				}
				else {
					curbone->SetBtMat(curwm);
					curbone->SetBtFlag(1);
				}
			}

			/*
			ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
			int paraxsiflag1 = 1;
			//int isfirstbone = 0;
			cureul = curbone->CalcBtLocalEulXYZ(paraxsiflag1, BEFEUL_ZERO);

			int inittraflag1 = 1;
			int setchildflag1 = 1;
			curbone->SetBtWorldMatFromEul(setchildflag1, cureul);//SetBtWorldMatFromEul関数内で設定部分がコメントアウトされていた
			*/
		}
	}


	int chilno;
	for( chilno = 0; chilno < curbto->GetChildBtSize(); chilno++ ){
		CBtObject* chilbto = curbto->GetChildBt( chilno );
		if( chilbto ){
			SetBtMotionReq(limitdegflag, chilbto, wmat, vpmat);
		}
	}

}

////adjustrot = 0
//void CModel::AdjustBtMatToParent(CBone* curbone, CBone* childbone, int adjustrot)
//{
//	if (!curbone) {
//		return;
//	}
//	if (!curbone->GetParent()) {
//		return;
//	}
//	if (!childbone) {
//		return;
//	}
//
//	ChaMatrix parentmat = curbone->GetParent()->GetBtMat();
//	ChaMatrix currentmat = curbone->GetBtMat();
//	ChaMatrix smat, rmat, tmat;
//	GetSRTMatrix2(currentmat, &smat, &rmat, &tmat);
//
//	ChaVector3 curpos1, curpos2;
//	ChaVector3 tmpfpos = curbone->GetJointFPos();
//	ChaMatrix tmpbtmat = curbone->GetBtMat();
//	ChaMatrix tmpparentbtmat = curbone->GetParent()->GetBtMat();
//	ChaVector3TransformCoord(&curpos1, &tmpfpos, &tmpbtmat);
//	ChaVector3TransformCoord(&curpos2, &tmpfpos, &tmpparentbtmat);
//	ChaVector3 adjustvec = curpos2 - curpos1;
//	//ChaVector3 adjustvec = ChaVector3(0.0, 0.0, 0.0);//for debug
//
//	if (adjustrot == 1) {
//		ChaVector3 childpos;
//		ChaVector3 tmpchildfpos = childbone->GetJointFPos();
//		ChaMatrix tmpbtmat2 = curbone->GetBtMat();
//		ChaVector3TransformCoord(&childpos, &tmpchildfpos, &tmpbtmat2);
//
//		ChaVector3 oldbonevec = childpos - curpos1;
//		ChaVector3Normalize(&oldbonevec, &oldbonevec);
//
//		ChaVector3 newcurpos;
//		newcurpos = curpos1 + adjustvec;
//		ChaVector3 newbonevec = childpos - newcurpos;
//		ChaVector3Normalize(&newbonevec, &newbonevec);
//
//		CQuaternion addrotq;
//		addrotq.RotationArc(oldbonevec, newbonevec);
//
//		rmat = rmat * addrotq.MakeRotMatX();
//
//		currentmat = smat * rmat * tmat;
//		//ChaVector3 tmpfpos = curbone->GetJointFPos();
//		ChaVector3TransformCoord(&curpos1, &tmpfpos, &currentmat);
//		adjustvec = curpos2 - curpos1;
//		GetSRTMatrix2(currentmat, &smat, &rmat, &tmat);
//	}
//
//	if (ChaVector3LengthDbl(&adjustvec) >= 0.050f) {
//		tmat.data[MATI_41] += adjustvec.x;
//		tmat.data[MATI_42] += adjustvec.y;
//		tmat.data[MATI_43] += adjustvec.z;
//
//		ChaMatrix newmat;
//		newmat = smat * rmat * tmat;
//
//		curbone->SetBtMat(newmat);
//		curbone->SetBtFlag(1);
//	}
//}
//
////int adjustrot = 0
//void CModel::AdjustBtMatToCurrent(CBone* curbone)
//{
//	if (!curbone) {
//		return;
//	}
//	if (!curbone->GetParent()) {
//		return;
//	}
//
//	ChaMatrix parentmat = curbone->GetParent()->GetBtMat();
//	ChaMatrix currentmat = curbone->GetBtMat();
//	ChaMatrix smat, rmat, tmat;
//	GetSRTMatrix2(parentmat, &smat, &rmat, &tmat);
//
//	ChaVector3 basepos, curpos;
//	ChaVector3 tmpfpos = curbone->GetJointFPos();
//	ChaMatrix tmpbtmat = curbone->GetBtMat();
//	ChaMatrix tmpparentbtmat = curbone->GetParent()->GetBtMat();
//	ChaVector3TransformCoord(&basepos, &tmpfpos, &tmpparentbtmat);
//	ChaVector3TransformCoord(&curpos, &tmpfpos, &tmpbtmat);
//	ChaVector3 adjustvec = curpos - basepos;
//	//ChaVector3 adjustvec = ChaVector3(0.0, 0.0, 0.0);//for debug
//
//	if (ChaVector3LengthDbl(&adjustvec) >= 0.050f) {
//		tmat.data[MATI_41] += adjustvec.x;
//		tmat.data[MATI_42] += adjustvec.y;
//		tmat.data[MATI_43] += adjustvec.z;
//
//		ChaMatrix newmat;
//		newmat = smat * rmat * tmat;
//
//		curbone->GetParent()->SetBtMat(newmat);
//		curbone->GetParent()->SetBtFlag(1);
//	}
//}
//
////int adjustrot = 0
//void CModel::AdjustBtMatToChild(CBone* curbone, CBone* childbone, int adjustrot)
//{
//	if (!curbone) {
//		return;
//	}
//	if (!curbone->GetParent()) {
//		return;
//	}
//	if (!childbone) {
//		return;
//	}
//	ChaMatrix parentmat = curbone->GetParent()->GetBtMat();
//	ChaMatrix currentmat = curbone->GetBtMat();
//	ChaMatrix smat, rmat, tmat;
//	GetSRTMatrix2(currentmat, &smat, &rmat, &tmat);
//
//	ChaVector3 childpos1, childpos2;
//	ChaVector3 tmpchildfpos = childbone->GetJointFPos();
//	ChaMatrix tmpbtmat = curbone->GetBtMat();
//	ChaMatrix tmpchildbtmat = childbone->GetBtMat();
//	ChaVector3TransformCoord(&childpos1, &tmpchildfpos, &tmpbtmat);
//	ChaVector3TransformCoord(&childpos2, &tmpchildfpos, &tmpchildbtmat);
//	ChaVector3 adjustvec = childpos2 - childpos1;
//	//ChaVector3 adjustvec = ChaVector3(0.0, 0.0, 0.0);//for debug
//
//	if (adjustrot == 1) {
//		ChaVector3 curpos;
//		ChaVector3 tmpfpos = curbone->GetJointFPos();
//		ChaVector3TransformCoord(&curpos, &tmpfpos, &tmpbtmat);
//
//		ChaVector3 oldbonevec = childpos1 - curpos;
//		ChaVector3Normalize(&oldbonevec, &oldbonevec);
//
//		ChaVector3 newchildpos;
//		newchildpos = childpos1 + adjustvec;
//		ChaVector3 newbonevec = newchildpos - curpos;
//		ChaVector3Normalize(&newbonevec, &newbonevec);
//
//		CQuaternion addrotq;
//		addrotq.RotationArc(oldbonevec, newbonevec);
//
//		rmat = rmat * addrotq.MakeRotMatX();
//
//		currentmat = smat * rmat * tmat;
//		ChaVector3TransformCoord(&childpos1, &tmpchildfpos, &currentmat);
//		adjustvec = childpos2 - childpos1;
//		GetSRTMatrix2(currentmat, &smat, &rmat, &tmat);
//	}
//
//
//	if (ChaVector3LengthDbl(&adjustvec) >= 0.050f) {
//		tmat.data[MATI_41] += adjustvec.x;
//		tmat.data[MATI_42] += adjustvec.y;
//		tmat.data[MATI_43] += adjustvec.z;
//
//		ChaMatrix newmat;
//		newmat = smat * rmat * tmat;
//
//		curbone->SetBtMat(newmat);
//		curbone->SetBtFlag(1);
//	}
//}
//
//
////Post処理。SetBtMotionReqを呼び出した後で呼び出す。
//void CModel::SetBtMotionPostLowerReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat, int kinematicadjustflag)
//{
//	//後処理
//	MOTINFO* curmi = 0;
//	int curmotid;
//	double curframe;
//	curmi = GetCurMotInfo();
//	if (!curmi) {
//		_ASSERT(0);
//		return;
//	}
//
//	curmotid = curmi->motid;
//	curframe = (double)((int)(curmi->curframe + 0.0001));
//
//
//	if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone && curbone->GetParent()) {
//			if (g_previewFlag == 4) {
//				if (curbone->GetBtKinFlag() == 1) {
//					CMotionPoint curmp = curbone->GetCurMp();
//					curbone->SetBtMat(curbone->GetWorldMat(curmotid, curframe, &curmp));
//					curbone->SetBtFlag(1);
//				}
//			}
//			else if (g_previewFlag == 5) {
//				//GetCurMp().GetWorldMat()には物理IK開始時の姿勢が入っている。
//				if ((curbone->GetBtFlag() == 0) || ((curbone->GetMass0() == FALSE) && (curbone->GetParent()->GetTmpKinematic() == true))) {//mass0とkinematic併用の場合にはmass0を適用
//				//if ((curbone->GetBtFlag() == 0) || ((curbone->GetMass0() == FALSE) && (curbone->GetTmpKinematic() == true))) {//mass0とkinematic併用の場合にはmass0を適用
//					if (kinematicadjustflag == 1) {
//						if (curbone->GetParent()) {
//
//							//2023/02/02 後で修正必要　btmatとGetCurMpには　モデルのworldmatが掛かっている
//
//							ChaMatrix newparmat = curbone->GetParent()->GetBtMat();
//							ChaMatrix firstparmat = curbone->GetParent()->GetCurMp().GetWorldMat();
//							ChaMatrix newcurmat = curbone->GetCurMp().GetWorldMat() * ChaMatrixInv(firstparmat) * newparmat;
//							curbone->SetBtMat(newcurmat);
//							curbone->SetBtFlag(1);
//						}
//						else {
//							curbone->SetBtMat(curbone->GetCurMp().GetWorldMat());
//							curbone->SetBtFlag(1);
//						}
//					}
//				}
//				else if ((curbone->GetBtFlag() == 1) && (curbone->GetParent()->GetBtFlag() == 1) && 
//					((curbone->GetParent()->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE))) {//mass0とkinematic併用の場合にはmass0を適用
//					//((curbone->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE))) {//mass0とkinematic併用の場合にはmass0を適用
//					//親の剛体とカレントの剛体のすきま、つまり関節の隙間を無くすために補正をする。
//					if (curbto->GetParBt()) {
//						if (curbone->GetParent()->GetParent()) {
//							//CBone* childbone = curbone->GetChild();
//							CBone* childbone = curbto->GetEndBone();
//							if (childbone) {
//								if (curbone->GetMass0() == 0) {
//									int adjustrot = 0;
//									AdjustBtMatToParent(curbone, childbone, adjustrot);
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//	int chilno;
//	for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//		CBtObject* chilbto = curbto->GetChildBt(chilno);
//		if (chilbto) {
//			SetBtMotionPostLowerReq(chilbto, wmat, vpmat, kinematicadjustflag);
//		}
//	}
//
//
//}
//
////Post処理。SetBtMotionReqを呼び出した後で呼び出す。
//void CModel::SetBtMotionPostUpperReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	//後処理
//
//	MOTINFO* curmi = 0;
//	int curmotid;
//	double curframe;
//	curmi = GetCurMotInfo();
//	if (!curmi) {
//		_ASSERT(0);
//		return;
//	}
//
//	curmotid = curmi->motid;
//	curframe = (double)((int)(curmi->curframe + 0.0001));
//
//	if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone && curbone->GetParent()) {
//			if (g_previewFlag == 4) {
//				if (curbone->GetBtKinFlag() == 1) {
//					CMotionPoint curmp = curbone->GetCurMp();
//					curbone->SetBtMat(curbone->GetWorldMat(curmotid, curframe, &curmp));
//					curbone->SetBtFlag(1);
//				}
//			}
//			else if (g_previewFlag == 5) {
//				//GetCurMp().GetWorldMat()には物理IK開始時の姿勢が入っている。
//				if ((curbone->GetBtFlag() == 1) && (curbone->GetParent()->GetBtFlag() == 1) && 
//					((curbone->GetParent()->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE))) {//mass0とkinematic併用の場合にはmass0を適用
//					//((curbone->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE))) {//mass0とkinematic併用の場合にはmass0を適用
//					//親の剛体とカレントの剛体のすきま、つまり関節の隙間を無くすために補正をする。
//					if (curbto->GetParBt()) {
//						if (curbone->GetParent()->GetParent()) {
//							if (curbone->GetMass0() == 1) {
//							//	int adjustrot = 1;
//							//	AdjustBtMatToCurrent(curbone, adjustrot);
//							}
//							else {
//								AdjustBtMatToCurrent(curbone);
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//	CBtObject* parentbto = curbto->GetParBt();
//	if (parentbto) {
//		SetBtMotionPostUpperReq(parentbto, wmat, vpmat);
//	}
//
//}
//
//
////Post処理。SetBtMotionReqを呼び出した後で呼び出す。
//void CModel::SetBtMotionMass0BottomUpReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	//後処理
//
//	//先に階層をたどってから処理をする。つまり末端から処理を始める。
//	int chilno;
//	for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//		CBtObject* chilbto = curbto->GetChildBt(chilno);
//		if (chilbto) {
//			SetBtMotionMass0BottomUpReq(chilbto, wmat, vpmat);
//		}
//	}
//
//	if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone && curbone->GetParent()) {
//			if (g_previewFlag == 5) {
//				if (curbone->GetBtFlag() == 1) {
//					//親の剛体とカレントの剛体のすきま、つまり関節の隙間を無くすために補正をする。
//					if (curbto->GetParBt()) {
//						if (curbone->GetParent()->GetParent()) {
//							if ((curbone->GetParent()->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE)) {//mass0とkinematic併用の場合にはmass0を適用
//							//if ((curbone->GetTmpKinematic() == false) || (curbone->GetMass0() == TRUE)) {//mass0とkinematic併用の場合にはmass0を適用
//								CBone* childbone = curbto->GetEndBone();
//								if (childbone) {
//									if (childbone->GetChild()) {
//										int adjustrot = 1;//!!!!!!!!!!!!
//										AdjustBtMatToChild(curbone, childbone, adjustrot);
//									}
//									else {
//										AdjustBtMatToCurrent(curbone);
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}
//
//
//void CModel::FindAndSetKinematicReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	//Kinematicとそうでないところの境目を探してみつかったらLowerReqで親行列をセットする。
//
//	//先に階層をたどってから処理をする。つまり末端から処理を始める。
//	int childno;
//	for (childno = 0; childno < curbto->GetChildBtSize(); childno++) {
//		CBtObject* childbto = curbto->GetChildBt(childno);
//		if (childbto) {
//			FindAndSetKinematicReq(childbto, wmat, vpmat);
//		}
//	}
//
//	if (curbto) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone) {
//			CBone* childbone = curbto->GetEndBone();
//			CBone* parentbone = curbone->GetParent();
//			if (parentbone) {
//				if ((parentbone->GetMass0() == FALSE) && (curbone->GetMass0() == FALSE)) {
//					if ((parentbone->GetTmpKinematic() == FALSE) && (curbone->GetTmpKinematic() == TRUE)) {
//						//境目を発見
//
//						//親剛体の姿勢（角度含む）の補正
//						//ChaMatrix oldparentmat = parentbone->GetBtMat();
//						//AdjustBtMatToChild(parentbone, curbone, 1);
//
//						ChaMatrix curmatrix = curbone->GetBtMat();
//						//ChaMatrix newmatrix = curmatrix * ChaMatrixInv(oldparentmat) * parentbone->GetBtMat();
//
//						//SetBtMotionKinematicLowerReq(curbto, curmatrix, curmatrix);
//
//						int childno2;
//						for (childno2 = 0; childno2 < curbto->GetChildBtSize(); childno2++) {
//							CBtObject* childbto2 = curbto->GetChildBt(childno2);
//							if (childbto2) {
//								//SetBtMotionKinematicLowerReq(childbto2, curmatrix, newmatrix);
//								SetBtMotionKinematicLowerReq(childbto2, curmatrix, curmatrix);
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//
//}
//void CModel::SetBtMotionKinematicLowerReq(CBtObject* curbto, ChaMatrix oldparentmat, ChaMatrix newparentmat)
//{
//	//LowerReqで親行列をセットする。
//	
//	if (curbto) {
//		CBone* curbone = curbto->GetBone();
//		if ((curbone->GetMass0() == FALSE) && (curbone->GetTmpKinematic() == TRUE)) {
//			ChaMatrix oldcurmat, newcurmat;
//			if (curbone && curbone->GetParent()) {
//				oldcurmat = curbone->GetBtMat();
//
//				//newcurmat = oldcurmat * ChaMatrixInv(oldparentmat) * newparentmat;//形状修正なしの場合
//
//
//				//2023/02/02 注意　後でチェック！！！　GetCurMpとBtMatにはモデルのwmが掛かっている
//				//そして　CurMpとBtMatには　モデルのwmをかけなければならない
//				//newparentmatには？？？
//
//
//				ChaMatrix curkinematicmat, parentkinematicmat;
//				curkinematicmat = curbone->GetCurMp().GetWorldMat();
//				parentkinematicmat = curbone->GetParent()->GetCurMp().GetWorldMat();
//				newcurmat = curkinematicmat * ChaMatrixInv(parentkinematicmat) * newparentmat;//形状を保持する場合
//
//				curbone->SetBtMat(newcurmat);
//				curbone->SetBtFlag(1);
//
//
//				int chilno;
//				for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//					CBtObject* chilbto = curbto->GetChildBt(chilno);
//					if (chilbto) {
//						SetBtMotionKinematicLowerReq(chilbto, oldcurmat, newcurmat);
//					}
//				}
//			}
//		}
//	}
//}
//
//void CModel::BtMat2BtObjReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//
//	if ((curbto->GetTopFlag() == 0) && curbto->GetBone()) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone && curbone->GetParent()) {
//			if (curbone->GetBtFlag() == 1){
//				if (g_previewFlag == 5) {
//
//					//GetCurMp().GetWorldMat()には物理IK開始時の姿勢が入っている。
//					if ((curbone->GetMass0() == FALSE) && (curbone->GetParent()->GetTmpKinematic() == true)) {
//						ChaMatrix newparmat = curbone->GetParent()->GetBtMat();
//						ChaMatrix firstparmat = curbone->GetParent()->GetCurMp().GetWorldMat();
//						ChaMatrix newcurmat = curbone->GetCurMp().GetWorldMat() * ChaMatrixInv(firstparmat) * newparmat;
//
//						//2023/02/02 注意　後でチェック！！！　GetCurMpとBtMatにはモデルのwmが掛かっている
//						//そして　CurMpとBtMatには　モデルのwmをかけなければならない
//						//curbto->GetFirstTransformMatX()には？？？
//
//
//						ChaMatrix newbtmat;
//						newbtmat = curbto->GetFirstTransformMatX() * ChaMatrixInv(firstparmat) * newparmat;
//
//						ChaVector3 aftcurpos;
//						ChaVector3 befcurpos = curbone->GetJointFPos();
//						ChaVector3TransformCoord(&aftcurpos, &befcurpos, &newcurmat);
//						ChaVector3 aftchildpos;
//						ChaVector3 befchildpos = curbto->GetEndBone()->GetJointFPos();
//						ChaVector3TransformCoord(&aftchildpos, &befchildpos, &newcurmat);
//						ChaVector3 rigidcenter = (aftcurpos + aftchildpos) * 0.5f;
//						//ChaVector3 aftparentpos;
//						//ChaVector3 befparentpos = curbone->GetParent()->GetJointFPos();
//						//ChaVector3TransformCoord(&aftparentpos, &befparentpos, &newparmat);
//						//ChaVector3 aftchildpos;
//						//ChaVector3 befchildpos = curbone->GetJointFPos();
//						//ChaVector3TransformCoord(&aftchildpos, &befchildpos, &newcurmat);
//						//ChaVector3 rigidcenter = (aftparentpos + aftchildpos) * 0.5f;
//						curbto->SetPosture2Bt(newbtmat, rigidcenter, 0);
//
//					}
//					else {
//						if (curbto->GetParBt() && (curbone->GetBtFlag() == 1) && (curbone->GetParent()->GetBtFlag() == 1)) {
//							if (curbone->GetParent()->GetParent()) {
//								ChaMatrix newparmat = curbone->GetParent()->GetBtMat();
//								ChaMatrix newcurmat = curbone->GetBtMat();
//								//ChaMatrix invfirstcurmat = ChaMatrixInv(curbone->GetCurrentZeroFrameMat(0));
//								ChaMatrix invfirstcurmat = ChaMatrixInv(curbone->GetStartMat2());
//								ChaMatrix diffmat = invfirstcurmat * newcurmat;
//
//								ChaMatrix newbtmat;
//								newbtmat = curbto->GetFirstTransformMatX() * diffmat;
//								ChaVector3 aftcurpos;
//								ChaVector3 befcurpos = curbone->GetJointFPos();
//								ChaVector3TransformCoord(&aftcurpos, &befcurpos, &newcurmat);
//								ChaVector3 aftchildpos;
//								ChaVector3 befchildpos = curbto->GetEndBone()->GetJointFPos();
//								ChaVector3TransformCoord(&aftchildpos, &befchildpos, &newcurmat);
//								ChaVector3 rigidcenter = (aftcurpos + aftchildpos) * 0.5f;
//
//								curbto->SetPosture2Bt(newbtmat, rigidcenter, 0);
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//	int chilno;
//	for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//		CBtObject* chilbto = curbto->GetChildBt(chilno);
//		if (chilbto) {
//			BtMat2BtObjReq(chilbto, wmat, vpmat);
//		}
//	}
//
//}

//void CModel::RecalcConstraintFrameABReq(CBtObject* curbto)
//{
//	if (curbto) {
//		CBone* curbone = curbto->GetBone();
//		if (curbone && curbone->GetParent()) {
//			if (g_previewFlag == 5) {
//				//if (curbto->GetParBt() && (curbone->GetBtFlag() == 1) && (curbone->GetParent()->GetBtFlag() == 1)) {
//					if (curbone->GetParent()->GetParent()) {
//						curbto->RecalcConstraintFrameAB();
//					}
//				//}
//			}
//		}
//		int chilno;
//		for (chilno = 0; chilno < curbto->GetChildBtSize(); chilno++) {
//			CBtObject* chilbto = curbto->GetChildBt(chilno);
//			if (chilbto) {
//				RecalcConstraintFrameABReq(chilbto);
//			}
//		}
//	}
//}

int CModel::CreateRigidElem()
{
	if (m_topbone) {
		//CreateRigidElemReq(m_topbone, 1, m_defaultrename, 1, m_defaultimpname);
		CreateRigidElemReq(m_topbone, 1, m_defaultrename, 0, m_defaultimpname);
	}

	//SetCurrentRigidElem(0);
	m_curreindex = 0;
	m_curimpindex = 0;

	return 0;
}

void CModel::CreateRigidElemReq( CBone* curbone, int reflag, string rename, int impflag, string impname )
{
	if (!curbone){
		return;
	}
	//CBone* parentbone = curbone->GetParent();
	//if (parentbone){
	//	parentbone->CreateRigidElem(curbone, reflag, rename, impflag, impname);
	//}

	curbone->CreateRigidElem(curbone->GetParent(), reflag, rename, impflag, impname);


	if( curbone->GetChild() ){
		CreateRigidElemReq( curbone->GetChild(), reflag, rename, impflag, impname );
	}
	if( curbone->GetBrother() ){
		CreateRigidElemReq( curbone->GetBrother(), reflag, rename, impflag, impname );
	}
}

int CModel::SetBtImpulse()
{
	if( !m_topbt ){
		return 0;
	}
	if( !m_topbone ){
		return 0;
	}

	SetBtImpulseReq( m_topbone );

	return 0;
}

void CModel::SetBtImpulseReq( CBone* srcbone )
{
	CBone* parentbone = srcbone->GetParent();

	if( parentbone ){
		ChaVector3 setimp( 0.0f, 0.0f, 0.0f );

		int impnum = parentbone->GetImpMapSize();
		if( (m_curimpindex >= 0) && (m_curimpindex < impnum) ){

			string curimpname = m_impinfo[ m_curimpindex ];
			map<string, map<CBone*, ChaVector3>>::iterator findimpmap;
			findimpmap = parentbone->FindImpMap( curimpname );
			if( findimpmap != parentbone->GetImpMapEnd() ){
				map<CBone*,ChaVector3>::iterator itrimp;
				itrimp = findimpmap->second.find( srcbone );
				if( itrimp != findimpmap->second.end() ){
					setimp = itrimp->second;
				}
			}
		}
		else{
			//_ASSERT(0);
		}

		CRigidElem* curre = parentbone->GetRigidElem( srcbone );
		if( curre ){
			ChaVector3 imp = setimp * g_impscale;

			CBtObject* findbto = FindBtObject( srcbone->GetBoneNo() );
			if( findbto && findbto->GetRigidBody() ){
				findbto->GetRigidBody()->applyImpulse( btVector3( imp.x, imp.y, imp.z ), btVector3( 0.0f, 0.0f, 0.0f ) );
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetBtImpulseReq( srcbone->GetChild() );
	}
	if( srcbone->GetBrother() ){
		SetBtImpulseReq( srcbone->GetBrother() );
	}
}

CBtObject* CModel::FindBtObject( int srcboneno )
{
	CBtObject* retbto = 0;
	if( !m_topbt ){
		return 0;
	}
	FindBtObjectReq( m_topbt, srcboneno, &retbto );

	return retbto;
}

void CModel::FindBtObjectReq( CBtObject* srcbto, int srcboneno, CBtObject** ppret )
{
	if( *ppret ){
		return;
	}

	if( srcbto->GetBone() ){
		CBone* curbone;
		curbone = m_bonelist[ srcboneno ];
		if( curbone ){
			CBone* parentbone;
			parentbone = curbone->GetParent();
			if( parentbone ){
				if( (srcbto->GetBone() == parentbone) && (srcbto->GetEndBone() == curbone) ){
					*ppret = srcbto;
					return;
				}
			}
		}
	}

	int chilno;
	for( chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++ ){
		CBtObject* chilbto = srcbto->GetChildBt( chilno );
		FindBtObjectReq( chilbto, srcboneno, ppret );
	}
}
int CModel::SetDispFlag( const char* srcobjname, int srcflag )
{
	CMQOObject* curobj = m_objectname[ srcobjname ];
	if( curobj ){
		curobj->SetDispFlag( srcflag );
	}

	return 0;
}

int CModel::EnableAllRigidElem(int srcrgdindex)
{
	if (!m_topbone){
		return 0;
	}

	EnableAllRigidElemReq(m_topbone, srcrgdindex);

	return 0;
}
int CModel::DisableAllRigidElem(int srcrgdindex)
{
	if (!m_topbone){
		return 0;
	}

	DisableAllRigidElemReq(m_topbone, srcrgdindex);

	return 0;
}

void CModel::EnableAllRigidElemReq(CBone* srcbone, int srcrgdindex)
{
	if ((srcrgdindex >= 0) && (srcrgdindex < (int)m_rigideleminfo.size())){
		if (srcbone->GetParent()){
			char* filename = m_rigideleminfo[srcrgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				//if (curre->GetBoneLeng() >= 0.00001f){
				curre->SetSkipflag(0);
				//}
			}
		}
	}
	else{
		_ASSERT(0);
	}

	if (srcbone->GetChild()){
		EnableAllRigidElemReq(srcbone->GetChild(), srcrgdindex);
	}
	if (srcbone->GetBrother()){
		EnableAllRigidElemReq(srcbone->GetBrother(), srcrgdindex);
	}

}
void CModel::DisableAllRigidElemReq(CBone* srcbone, int srcrgdindex)
{
	if ((srcrgdindex >= 0) && (srcrgdindex < (int)m_rigideleminfo.size())){
		if (srcbone->GetParent()){
			char* filename = m_rigideleminfo[srcrgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				curre->SetSkipflag(1);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	if (srcbone->GetChild()){
		DisableAllRigidElemReq(srcbone->GetChild(), srcrgdindex);
	}
	if (srcbone->GetBrother()){
		DisableAllRigidElemReq(srcbone->GetBrother(), srcrgdindex);
	}
}

int CModel::SetAllBtgData( int gid, int reindex, float btg )
{
	if( !m_topbone ){
		return 0;
	}
	if( reindex < 0 ){
		return 0;
	}

	SetBtgDataReq( gid, reindex, m_topbone, btg );

	return 0;
}
void CModel::SetBtgDataReq( int gid, int reindex, CBone* srcbone, float btg )
{
	if (!srcbone) {
		return;
	}

	if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
		if (srcbone->GetParent()){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetBtg(btg);
				}
			}
		}
	}
	else{
		_ASSERT(0);
	}

	if( srcbone->GetChild() ){
		SetBtgDataReq( gid, reindex, srcbone->GetChild(), btg );
	}
	if( srcbone->GetBrother() ){
		SetBtgDataReq( gid, reindex, srcbone->GetBrother(), btg );
	}
}


int CModel::SetAllSphrateData(int gid, int rgdindex, float srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetSphrateDataReq(gid, rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetSphrateDataReq(int gid, int rgdindex, CBone* srcbone, float srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre) {
				if ((gid == -1) || (gid == curre->GetGroupid())) {
					curre->SetSphrate(srcval);
				}
			}
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetSphrateDataReq(gid, rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetSphrateDataReq(gid, rgdindex, srcbone->GetBrother(), srcval);
	}
}

int CModel::SetAllBoxzrateData(int gid, int rgdindex, float srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetBoxzrateDataReq(gid, rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetBoxzrateDataReq(int gid, int rgdindex, CBone * srcbone, float srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre) {
				if ((gid == -1) || (gid == curre->GetGroupid())) {
					curre->SetBoxzrate(srcval);
				}
			}
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetBoxzrateDataReq(gid, rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetBoxzrateDataReq(gid, rgdindex, srcbone->GetBrother(), srcval);
	}
}

int CModel::SetAllSkipflagData(int gid, int rgdindex, bool srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetSkipflagDataReq(gid, rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetSkipflagDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre) {
				if ((gid == -1) || (gid == curre->GetGroupid())) {
					curre->SetSkipflag(srcval);
				}
			}
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetSkipflagDataReq(gid, rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetSkipflagDataReq(gid, rgdindex, srcbone->GetBrother(), srcval);
	}
}

int CModel::SetAllForbidrotData(int gid, int rgdindex, bool srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetForbidrotDataReq(gid, rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetForbidrotDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre) {
				if ((gid == -1) || (gid == curre->GetGroupid())) {
					curre->SetForbidRotFlag(srcval);
				}
			}
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetForbidrotDataReq(gid, rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetForbidrotDataReq(gid, rgdindex, srcbone->GetBrother(), srcval);
	}
}

int CModel::SetAllColtypeData(int gid, int rgdindex, int srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetColtypeDataReq(gid, rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetColtypeDataReq(int gid, int rgdindex, CBone* srcbone, int srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre) {
				if ((gid == -1) || (gid == curre->GetGroupid())) {
					curre->SetColtype(srcval);
				}
			}
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetColtypeDataReq(gid, rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetColtypeDataReq(gid, rgdindex, srcbone->GetBrother(), srcval);
	}
}

int CModel::SetAllBtforceData(int rgdindex, bool srcval)
{
	if (!m_topbone) {
		return 0;
	}
	if (rgdindex < 0) {
		return 0;
	}

	SetBtforceDataReq(rgdindex, m_topbone, srcval);
	return 0;
}
void CModel::SetBtforceDataReq(int rgdindex, CBone* srcbone, bool srcval)
{
	if (!srcbone) {
		return;
	}

	if (rgdindex < 0) {
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())) {
		if (srcbone->GetParent()) {
			srcbone->GetParent()->SetBtForce((int)srcval);
		}
	}
	else {
		_ASSERT(0);
	}

	if (srcbone->GetChild()) {
		SetBtforceDataReq(rgdindex, srcbone->GetChild(), srcval);
	}
	if (srcbone->GetBrother()) {
		SetBtforceDataReq(rgdindex, srcbone->GetBrother(), srcval);
	}
}



int CModel::SetAllDampAnimData( int gid, int rgdindex, float valL, float valA )
{
	if( !m_topbone ){
		return 0;
	}
	if( rgdindex < 0 ){
		return 0;
	}

	SetDampAnimDataReq( gid, rgdindex, m_topbone, valL, valA );
	return 0;
}
void CModel::SetDampAnimDataReq( int gid, int rgdindex, CBone* srcbone, float valL, float valA )
{
	if (!srcbone) {
		return;
	}

	if( rgdindex < 0 ){
		return;
	}

	if ((rgdindex >= 0) && (rgdindex < (int)m_rigideleminfo.size())){
		if (srcbone->GetParent()){
			char* filename = m_rigideleminfo[rgdindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetDampanimL(valL);
					curre->SetDampanimA(valA);
				}
			}
		}
	}
	else{
		_ASSERT(0);
	}

	if( srcbone->GetChild() ){
		SetDampAnimDataReq( gid, rgdindex, srcbone->GetChild(), valL, valA );
	}
	if( srcbone->GetBrother() ){
		SetDampAnimDataReq( gid, rgdindex, srcbone->GetBrother(), valL, valA );
	}
}


int CModel::SetAllImpulseData( int gid, float impx, float impy, float impz )
{
	if( !m_topbone ){
		return 0;
	}
	ChaVector3 srcimp = ChaVector3( impx, impy, impz );

	SetImpulseDataReq( gid, m_topbone, srcimp );

	return 0;
}

void CModel::SetImpulseDataReq( int gid, CBone* srcbone, ChaVector3 srcimp )
{
	if (!srcbone) {
		return;
	}

	if ((m_rgdindex < 0) || (m_rgdindex >= (int)m_rigideleminfo.size())){
		return;
	}

	CBone* parentbone = srcbone->GetParent();

	if( parentbone ){
		int renum = (int)m_rigideleminfo.size();
		int impnum = parentbone->GetImpMapSize();
		if( (m_curimpindex >= 0) && (m_curimpindex < impnum) && (m_curreindex >= 0) && (m_curreindex < renum) ){

			char* filename = m_rigideleminfo[m_rgdindex].filename;//!!! rgdindex !!!
			CRigidElem* curre = parentbone->GetRigidElemOfMap( filename, srcbone );
			if( curre ){
				if( (gid == -1) || (gid == curre->GetGroupid()) ){
					string curimpname = m_impinfo[ m_curimpindex ];
					map<string, map<CBone*, ChaVector3>>::iterator findimpmap;
					findimpmap = parentbone->FindImpMap( curimpname );
					if( findimpmap != parentbone->GetImpMapEnd() ){
						map<CBone*,ChaVector3>::iterator itrimp;
						itrimp = findimpmap->second.find( srcbone );
						if( itrimp != findimpmap->second.end() ){
							itrimp->second = srcimp;
						}
					}
				}
			}
		}
	}

	if( srcbone->GetChild() ){
		SetImpulseDataReq( gid, srcbone->GetChild(), srcimp );
	}
	if( srcbone->GetBrother() ){
		SetImpulseDataReq( gid, srcbone->GetBrother(), srcimp );
	}
}

int CModel::SetImp( int srcboneno, int kind, float srcval )
{
	if( !m_topbone ){
		return 0;
	}
	if( srcboneno < 0 ){
		return 0;
	}
	if( m_curimpindex < 0 ){
		return 0;
	}

	CBone* curbone = m_bonelist[srcboneno];
	if( !curbone ){
		return 0;
	}

	CBone* parentbone = curbone->GetParent();
	if( !parentbone ){
		return 0;
	}

	int impnum = parentbone->GetImpMapSize();
	if( m_curimpindex >= impnum ){
		//_ASSERT( 0 );
		return 0;
	}

	string curimpname = m_impinfo[ m_curimpindex ];
	if( parentbone->GetImpMapSize2( curimpname ) > 0 ){
		map<string, map<CBone*, ChaVector3>>::iterator itrfindmap;
		itrfindmap = parentbone->FindImpMap( curimpname );
		map<CBone*,ChaVector3>::iterator itrimp;
		itrimp = itrfindmap->second.find( curbone );
		if( itrimp == itrfindmap->second.end() ){
			if (kind == 0){
				itrfindmap->second[curbone].x = srcval;
			}
			else if (kind == 1){
				itrfindmap->second[curbone].y = srcval;
			}
			else if (kind == 2){
				itrfindmap->second[curbone].z = srcval;
			}
		}
		else{
			if (kind == 0){
				itrimp->second.x = srcval;
			}
			else if (kind == 1){
				itrimp->second.y = srcval;
			}
			else if (kind == 2){
				itrimp->second.z = srcval;
			}
		}
	
	}else{
		_ASSERT( 0 );
	}

	return 0;
}


int CModel::SetAllDmpData( int gid, int reindex, float ldmp, float admp )
{
	if( !m_topbone ){
		return 0;
	}
	if( reindex < 0 ){
		return 0;
	}

	SetDmpDataReq( gid, reindex, m_topbone, ldmp, admp );

	return 0;
}
void CModel::SetDmpDataReq( int gid, int reindex, CBone* srcbone, float ldmp, float admp )
{
	if (!srcbone) {
		return;
	}

	if( srcbone->GetParent() ){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetLDamping(ldmp);
					curre->SetADamping(admp);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetDmpDataReq( gid, reindex, srcbone->GetChild(), ldmp, admp );
	}
	if( srcbone->GetBrother() ){
		SetDmpDataReq( gid, reindex, srcbone->GetBrother(), ldmp, admp );
	}
}

int CModel::SetColTypeAll(int reindex, int srctype)
{
	if (!m_topbone){
		return 0;
	}
	if (reindex < 0){
		return 0;
	}

	if ((srctype >= COL_CONE_INDEX) && (srctype < COL_MAX)){
		SetColTypeReq(reindex, m_topbone, srctype);
	}
	else{
		_ASSERT(0);
	}

	return 0;

}

void CModel::SetColTypeReq(int reindex, CBone* srcbone, int srctype)
{
	if (srcbone->GetParent()){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				curre->SetColtype(srctype);
				curre->SetColtype(srctype);
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if (srcbone->GetChild()){
		SetColTypeReq(reindex, srcbone->GetChild(), srctype);
	}
	if (srcbone->GetBrother()){
		SetColTypeReq(reindex, srcbone->GetBrother(), srctype);
	}
}



int CModel::SetAllRestData( int gid, int reindex, float rest, float fric )
{
	if( !m_topbone ){
		return 0;
	}
	if( reindex < 0 ){
		return 0;
	}

	SetRestDataReq( gid, reindex, m_topbone, rest, fric );

	return 0;
}

void CModel::SetRestDataReq( int gid, int reindex, CBone* srcbone, float rest, float fric )
{
	if (!srcbone) {
		return;
	}

	if( srcbone->GetParent() ){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetRestitution(rest);
					curre->SetFriction(fric);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetRestDataReq( gid, reindex, srcbone->GetChild(), rest, fric );
	}
	if( srcbone->GetBrother() ){
		SetRestDataReq( gid, reindex, srcbone->GetBrother(), rest, fric );
	}
}

int CModel::SetAllKData( int gid, int reindex, int srclk, int srcak, float srccuslk, float srccusak )
{
	if( !m_topbone ){
		return 0;
	}
	if( reindex < 0 ){
		return 0;
	}

	SetKDataReq( gid, reindex, m_topbone, srclk, srcak, srccuslk, srccusak );

	return 0;
}
void CModel::SetKDataReq( int gid, int reindex, CBone* srcbone, int srclk, int srcak, float srccuslk, float srccusak )
{
	if (!srcbone) {
		return;
	}

	if( srcbone->GetParent() ){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetLKindex(srclk);
					curre->SetAKindex(srcak);
					curre->SetCusLk(srccuslk);
					curre->SetCusAk(srccusak);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetKDataReq( gid, reindex, srcbone->GetChild(), srclk, srcak, srccuslk, srccusak );
	}
	if( srcbone->GetBrother() ){
		SetKDataReq( gid, reindex, srcbone->GetBrother(), srclk, srcak, srccuslk, srccusak );
	}
}

int CModel::SetAllMassData( int gid, int reindex, float srcmass )
{
	if( !m_topbone ){
		return 0;
	}
	if( reindex < 0 ){
		return 0;
	}

	SetMassDataReq( gid, reindex, m_topbone, srcmass );


	return 0;
}
void CModel::SetMassDataReq( int gid, int reindex, CBone* srcbone, float srcmass )
{
	if (!srcbone) {
		return;
	}

	if( srcbone->GetParent() ){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					curre->SetMass(srcmass);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetMassDataReq( gid, reindex, srcbone->GetChild(), srcmass );
	}
	if( srcbone->GetBrother() ){
		SetMassDataReq( gid, reindex, srcbone->GetBrother(), srcmass );
	}
}

int CModel::SetAllMassDataByBoneLeng(int gid, int reindex, float srcmass)
{
	if (!m_topbone){
		return 0;
	}
	if (reindex < 0){
		return 0;
	}

	SetMassDataByBoneLengReq(gid, reindex, m_topbone, srcmass);


	return 0;
}
void CModel::SetMassDataByBoneLengReq(int gid, int reindex, CBone* srcbone, float srcmass)
{
	if (srcbone->GetParent()){
		if ((reindex >= 0) && (reindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[reindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if ((gid == -1) || (gid == curre->GetGroupid())){
					ChaVector3 parentpos, curpos, diffpos;
					parentpos = srcbone->GetParent()->GetJointFPos();
					curpos = srcbone->GetJointFPos();
					diffpos = curpos - parentpos;
					float leng = (float)ChaVector3LengthDbl(&diffpos);
					float setmass = srcmass * leng;

					curre->SetMass(setmass);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if (srcbone->GetChild()){
		SetMassDataByBoneLengReq(gid, reindex, srcbone->GetChild(), srcmass);
	}
	if (srcbone->GetBrother()){
		SetMassDataByBoneLengReq(gid, reindex, srcbone->GetBrother(), srcmass);
	}
}

int CModel::SetKinematicFlag()
{
	if (!m_topbt) {
		return 0;
	}

	SetKinematicFlagReq(m_topbt);

	return 0;
}

void CModel::SetKinematicFlagReq(CBtObject* srcbto)
{
	if (!srcbto) {
		return;
	}

	CBone* srcbone = srcbto->GetBone();
	if (srcbone) {

		srcbone->SetBtKinFlag(1);

		DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
		if (s_setrigidflag == 0) {
			s_rigidflag = curflag;
			s_setrigidflag = 1;
		}
		//srcbto->GetRigidBody()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
		srcbto->GetRigidBody()->setCollisionFlags(curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
		//srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
		//srcbto->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
		//srcbto->m_rigidbody->setActivationState(DISABLE_SIMULATION);
		//CF_STATIC_OBJECT

		if (srcbto->GetRigidBody()) {
			//srcbto->GetRigidBody()->activate();
			//###srcbto->GetRigidBody()->setDeactivationTime(0.0);
			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);

			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 150.0);
			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
		}
	}

	int childno;
	for (childno = 0; childno < srcbto->GetChildBtSize(); childno++) {
		CBtObject* childbto = srcbto->GetChildBt(childno);
		if (childbto) {
			SetKinematicFlagReq(childbto);
		}
	}
}


//int CModel::SetRagdollKinFlag(int selectbone, int physicsmvkind)
//{
//
//	if( !m_topbt ){
//		return 0;
//	}
//
//	SetRagdollKinFlagReq( m_topbt, selectbone, physicsmvkind );
//
//	
//
//
//	return 0;
//}
//void CModel::SetRagdollKinFlagReq(CBtObject* srcbto, int selectbone, int physicsmvkind)
//{
//	//Mass0が設定されていた場合、TmpKinematicは強制的に無効
//
//
//	CBone* srcbone = srcbto->GetBone();
//	if (srcbone && srcbone->GetParent()){
//		if ((srcbone->GetMass0() == TRUE) || (srcbone->GetParent()->GetMass0() == TRUE) || (srcbone->GetParent()->GetTmpKinematic() == false)) {
//			CBone* kinchildbone = GetBoneByID(selectbone);
//			if (kinchildbone) {
//				CBone* kinbone = kinchildbone->GetParent();
//				if (kinbone) {
//					int curboneno = srcbone->GetBoneNo();
//					if (curboneno != kinbone->GetBoneNo()) {
//						srcbone->SetBtKinFlag(0);
//					}
//					else {
//						srcbone->SetBtKinFlag(1);
//					}
//
//				}
//			}
//			else {
//				srcbone->SetBtKinFlag(0);
//			}
//		}
//		else {
//			srcbone->SetBtKinFlag(1);
//		}
//
//		//if ((physicsmvkind == 0) && (srcbone->GetBtKinFlag() == 1) && (srcbto->GetRigidBody())){
//		if ((srcbone->GetBtKinFlag() == 1) && (srcbto->GetRigidBody())){
//			DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
//			if (s_setrigidflag == 0){
//				s_rigidflag = curflag;
//				s_setrigidflag = 1;
//			}
//			//srcbto->GetRigidBody()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
//			srcbto->GetRigidBody()->setCollisionFlags(curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
//			//srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
//			//srcbto->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
//			//srcbto->m_rigidbody->setActivationState(DISABLE_SIMULATION);
//			//CF_STATIC_OBJECT
//		}
//		else if (srcbto->GetRigidBody()){
//			DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
//			srcbto->GetRigidBody()->setCollisionFlags(curflag & ~btCollisionObject::CF_KINEMATIC_OBJECT);
//
//			if (srcbone->GetParent()){
//				CRigidElem* curre = srcbone->GetParent()->GetRigidElem(srcbone);
//				if (curre){
//					if ((m_curreindex >= 0) && (m_curreindex < (int)m_rigideleminfo.size())){
//						//srcbto->GetRigidBody()->setGravity(btVector3(0.0f, curre->GetBtg() * m_rigideleminfo[m_curreindex].btgscale, 0.0f));
//						srcbto->GetRigidBody()->setGravity(btVector3(0.0f, 0.0f, 0.0f));
//						srcbto->GetRigidBody()->applyGravity();
//					}
//					else{
//						_ASSERT(0);
//					}
//				}
//			}
//		}
//
//		if (srcbto->GetRigidBody()){
//			//srcbto->GetRigidBody()->activate();
//			//###srcbto->GetRigidBody()->setDeactivationTime(0.0);
//			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
//
//			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 150.0);
//			//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
//		}
//	}
//
//
//
//	int chilno;
//	for( chilno = 0; chilno < srcbto->GetChildBtSize(); chilno++ ){
//		CBtObject* chilbto = srcbto->GetChildBt( chilno );
//		if( chilbto ){
//			SetRagdollKinFlagReq( chilbto, selectbone );
//		}
//	}
//}

int CModel::SetCurrentRigidElem( int curindex )
{
	if( curindex < 0 ){
		return 0;
	}
	if( curindex >= (int)m_rigideleminfo.size() ){
		_ASSERT( 0 );
		return 0;
	}

	m_curreindex = curindex;

	string curname = m_rigideleminfo[ curindex ].filename;


	SetCurrentRigidElemReq(m_topbone, curname);


	/*
	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if (curbone){
			CallF(curbone->SetCurrentRigidElem(curname), return 1);
		}
	}
	*/
	return 0;
}

REINFO CModel::GetCurrentRigidElemInfo(int* retindex)
{
	REINFO retinfo;

	if ((m_curreindex >= 0) && (m_curreindex < (int)m_rigideleminfo.size())) {
		*retindex = m_curreindex;
		retinfo = m_rigideleminfo[m_curreindex];
	}
	else {
		*retindex = -1;
		retinfo.btgscale = 1.0f;
		strcpy_s(retinfo.filename, 256, "unknown");
	}

	return retinfo;
}

void CModel::SetCurrentRigidElemReq(CBone* srcbone, string curname)
{
	if (srcbone){

		srcbone->SetCurrentRigidElem(curname);


		if (srcbone->GetChild()){
			SetCurrentRigidElemReq(srcbone->GetChild(), curname);
		}
		if (srcbone->GetBrother()){
			SetCurrentRigidElemReq(srcbone->GetBrother(), curname);
		}

	}
}


int CModel::MultDispObj( ChaVector3 srcmult, ChaVector3 srctra )
{

	map<int,CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( curobj ){
			CallF( curobj->MultScale( srcmult, srctra ), return 1 );			
		}
	}

	return 0;
}

int CModel::SetColiIDtoGroup( CRigidElem* srcre )
{
	if( !m_topbone ){
		return 0;
	}
	if( m_curreindex < 0 ){
		return 0;
	}

	SetColiIDReq( m_topbone, srcre );

	return 0;
}
void CModel::SetColiIDReq( CBone* srcbone, CRigidElem* srcre )
{
	if( srcbone->GetParent() ){
		if ((m_curreindex >= 0) && (m_curreindex < (int)m_rigideleminfo.size())){
			char* filename = m_rigideleminfo[m_curreindex].filename;
			CRigidElem* curre = srcbone->GetParent()->GetRigidElemOfMap(filename, srcbone);
			if (curre){
				if (curre->GetGroupid() == srcre->GetGroupid()){
					curre->CopyColiids(srcre);
				}
			}
		}
		else{
			_ASSERT(0);
		}
	}

	if( srcbone->GetChild() ){
		SetColiIDReq( srcbone->GetChild(), srcre );
	}
	if( srcbone->GetBrother() ){
		SetColiIDReq( srcbone->GetBrother(), srcre );
	}
}

int CModel::ResetBt()
{
	if( !m_topbt ){
		return 0;
	}

	ResetBtReq( m_topbt );

	return 0;
}

void CModel::ResetBtReq( CBtObject* curbto )
{
	if( curbto->GetRigidBody() ){
		if (curbto->GetRigidBody()->getMotionState())
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)curbto->GetRigidBody()->getMotionState();
			myMotionState->m_graphicsWorldTrans = myMotionState->m_startWorldTrans;
			curbto->GetRigidBody()->setCenterOfMassTransform( myMotionState->m_graphicsWorldTrans );
			curbto->GetRigidBody()->setInterpolationWorldTransform( myMotionState->m_startWorldTrans );
			curbto->GetRigidBody()->forceActivationState(ACTIVE_TAG);
			
			//curbto->GetRigidBody()->activate();
			//curbto->GetRigidBody()->setDeactivationTime(0.0);

			//curbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
			//colObj->setActivationState(WANTS_DEACTIVATION);


			curbto->GetRigidBody()->setDeactivationTime(0.0f);
			curbto->GetRigidBody()->setSleepingThresholds(0.0f, 0.0f);
			curbto->GetRigidBody()->activate();
		}
		if (curbto->GetRigidBody() && !curbto->GetRigidBody()->isStaticObject())
		{
			curbto->GetRigidBody()->setLinearVelocity(btVector3(0,0,0));
			curbto->GetRigidBody()->setAngularVelocity(btVector3(0,0,0));
		}
	}

	int chilno;
	for( chilno = 0; chilno < curbto->GetChildBtSize(); chilno++ ){
		CBtObject* chilbto = curbto->GetChildBt( chilno );
		if( chilbto ){
			ResetBtReq( chilbto );
		}
	}
}

int CModel::SetBefEditMat(bool limitdegflag, CEditRange* erptr, CBone* curbone, int maxlevel )
{
	int levelcnt0 = 0;
	while( curbone && ((maxlevel == 0) || (levelcnt0 < (maxlevel+1))) )
	{
		int keynum;
		double startframe, endframe, applyframe;
		erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
		double curframe;
		for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0){
			CMotionPoint* editmp = 0;
			editmp = curbone->GetMotionPoint(m_curmotinfo->motid, curframe);
			if(editmp){
				if (g_previewFlag != 5){
					editmp->SetBefEditMat(curbone->GetWorldMat(limitdegflag, m_curmotinfo->motid, curframe, editmp));
				}
				else{
					editmp->SetBefEditMat(curbone->GetBtMat());
				}
			}else{
				_ASSERT( 0 );
			}
		}
		curbone = curbone->GetParent();
		levelcnt0++;
	}
	return 0;
}

int CModel::SetBefEditMatFK(bool limitdegflag, CEditRange* erptr, CBone* curbone)
{
	if (curbone){

		int keynum;
		double startframe, endframe, applyframe;
		erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
		double curframe;
		for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0){

			CMotionPoint* editmp = 0;
			editmp = curbone->GetMotionPoint(m_curmotinfo->motid, curframe);
			if (editmp){
				editmp->SetBefEditMat(curbone->GetWorldMat(limitdegflag, m_curmotinfo->motid, curframe, editmp));
			}
			else{
				_ASSERT(0);
			}
		}
	}
	return 0;
}

bool CModel::CalcAxisAndRotForIKRotateAxis(int limitdegflag,
	CBone* parentbone, CBone* firstbone,
	double curframe, ChaVector3 targetpos, 
	ChaVector3 srcikaxis,
	ChaVector3* dstaxis, float* dstrotrad)
{
	//return nearflag : too near to move


	//########################################################
	//2023/03/24
	//model座標系で計算：modelのWorldMatの影響を無くして計算
	//########################################################


	if (!parentbone || !firstbone || !dstaxis || !dstrotrad) {
		_ASSERT(0);
		return true;
	}

	if (!firstbone->GetParent()) {
		_ASSERT(0);
		return true;
	}

	ChaMatrix invmodelwm = ChaMatrixInv(GetWorldMat());

	ChaVector3 ikaxis = srcikaxis;//!!!!!!!!!!!!
	ChaVector3Normalize(&ikaxis, &ikaxis);

	ChaVector3 modelparentpos, modelchildpos;
	{
		ChaVector3 parentworld;
		parentworld = parentbone->GetWorldPos(limitdegflag, m_curmotinfo->motid, curframe);
		ChaVector3TransformCoord(&modelparentpos, &parentworld, &invmodelwm);

		ChaMatrix parentmat = firstbone->GetParent()->GetWorldMat(limitdegflag, m_curmotinfo->motid, curframe, 0);// *GetWorldMat();
		ChaVector3 tmpfirstfpos = firstbone->GetJointFPos();
		ChaVector3TransformCoord(&modelchildpos, &tmpfirstfpos, &parentmat);
	}

	ChaVector3 childtotarget = targetpos - modelchildpos;
	double distance = ChaVector3LengthDbl(&childtotarget);
	//if (distance <= 0.10f) {
	//	return true;
	//}


	ChaVector3 parbef, chilbef, tarbef;
	parbef = modelparentpos;
	CalcShadowToPlane(modelchildpos, ikaxis, modelparentpos, &chilbef);
	CalcShadowToPlane(targetpos, ikaxis, modelparentpos, &tarbef);

	ChaVector3 vec0, vec1;
	vec0 = chilbef - parbef;
	ChaVector3Normalize(&vec0, &vec0);
	vec1 = tarbef - parbef;
	ChaVector3Normalize(&vec1, &vec1);

	ChaVector3 rotaxis2;
	ChaVector3Cross(&rotaxis2, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
	ChaVector3Normalize(&rotaxis2, &rotaxis2);

	float rotdot2, rotrad2;
	rotdot2 = ChaVector3Dot(&vec0, &vec1);
	rotdot2 = min(1.0f, rotdot2);
	rotdot2 = max(-1.0f, rotdot2);
	rotrad2 = (float)acos(rotdot2);

	*dstaxis = rotaxis2;
	*dstrotrad = rotrad2;

	return false;
}


int CModel::CalcAxisAndRotForIKRotate(int limitdegflag, 
	CBone* parentbone, CBone* firstbone, 
	double curframe, ChaVector3 targetpos, 
	ChaVector3* dstaxis, float* dstrotrad)
{

	//########################################################
	//2023/03/24
	//model座標系で計算：modelのWorldMatの影響を無くして計算
	//########################################################


	if (!parentbone || !firstbone || !dstaxis || !dstrotrad) {
		_ASSERT(0);
		return 1;
	}

	if (!firstbone->GetParent()) {
		_ASSERT(0);
		return 1;
	}

	ChaMatrix invmodelwm;
	invmodelwm = ChaMatrixInv(GetWorldMat());
	ChaVector3 modelcamtarget, modelcameye;
	ChaVector3TransformCoord(&modelcamtarget, &g_camtargetpos, &invmodelwm);
	ChaVector3TransformCoord(&modelcameye, &g_camEye, &invmodelwm);

	//ChaVector3 ikaxis = g_camtargetpos - g_camEye;
	ChaVector3 ikaxis = modelcamtarget - modelcameye;
	ChaVector3Normalize(&ikaxis, &ikaxis);

	ChaVector3 modelparentpos, modelchildpos;
	{
		ChaVector3 parentworld, childworld;
		parentworld = parentbone->GetWorldPos(limitdegflag, m_curmotinfo->motid, curframe);
		ChaVector3TransformCoord(&modelparentpos, &parentworld, &invmodelwm);

		ChaMatrix parentmat = firstbone->GetParent()->GetWorldMat(limitdegflag, m_curmotinfo->motid, curframe, 0);// *GetWorldMat();
		ChaVector3 tmpfirstfpos = firstbone->GetJointFPos();
		ChaVector3TransformCoord(&modelchildpos, &tmpfirstfpos, &parentmat);
	}


	ChaVector3 parbef, chilbef, tarbef;
	parbef = modelparentpos;
	CalcShadowToPlane(modelchildpos, ikaxis, modelparentpos, &chilbef);
	CalcShadowToPlane(targetpos, ikaxis, modelparentpos, &tarbef);

	ChaVector3 vec0, vec1;
	vec0 = chilbef - parbef;
	ChaVector3Normalize(&vec0, &vec0);
	vec1 = tarbef - parbef;
	ChaVector3Normalize(&vec1, &vec1);

	ChaVector3 rotaxis2;
	ChaVector3Cross(&rotaxis2, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
	ChaVector3Normalize(&rotaxis2, &rotaxis2);

	float rotdot2, rotrad2;
	rotdot2 = ChaVector3Dot(&vec0, &vec1);
	rotdot2 = min(1.0f, rotdot2);
	rotdot2 = max(-1.0f, rotdot2);
	rotrad2 = (float)acos(rotdot2);

	*dstaxis = rotaxis2;
	*dstrotrad = rotrad2;

	return 0;
}


//camera vertical
int CModel::CalcAxisAndRotForIKRotateVert(int limitdegflag,
	CBone* parentbone, CBone* firstbone,
	double curframe, ChaVector3 targetpos,
	ChaVector3* dstaxis, float* dstrotrad)
{
	if (!parentbone || !firstbone || !dstaxis || !dstrotrad) {
		_ASSERT(0);
		return 1;
	}

	if (!firstbone->GetParent()) {
		_ASSERT(0);
		return 1;
	}

	ChaVector3 ikaxis = g_camtargetpos - g_camEye;
	ChaVector3Normalize(&ikaxis, &ikaxis);

	ChaVector3 parworld, chilworld;
	parworld = ChaVector3(0.0f, 0.0f, 0.0f);
	chilworld = ChaVector3(0.0f, 0.0f, 0.0f);
	{
		parworld = parentbone->GetWorldPos(limitdegflag, m_curmotinfo->motid, curframe);
		ChaMatrix parworldmat = firstbone->GetParent()->GetWorldMat(limitdegflag, m_curmotinfo->motid, curframe, 0);// *GetWorldMat();
		ChaVector3 tmpfirstfpos = firstbone->GetJointFPos();
		ChaVector3TransformCoord(&chilworld, &tmpfirstfpos, &parworldmat);
	}

	ChaVector3 rotaxis2;
	rotaxis2 = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 rotaxis3;
	rotaxis3 = ChaVector3(0.0f, 0.0f, 0.0f);;
	{
		ChaVector3 parbef, chilbef, tarbef;
		parbef = parworld;
		CalcShadowToPlane(chilworld, ikaxis, parworld, &chilbef);
		CalcShadowToPlane(targetpos, ikaxis, parworld, &tarbef);
		ChaVector3 vec0, vec1;
		vec0 = chilbef - parbef;
		ChaVector3Normalize(&vec0, &vec0);
		vec1 = tarbef - parbef;
		ChaVector3Normalize(&vec1, &vec1);

		ChaVector3Cross(&rotaxis2, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
		ChaVector3Normalize(&rotaxis2, &rotaxis2);


		//////// vertical
		rotaxis3 = ChaVector3(0.0f, 0.0f, 0.0f);
		ChaVector3Cross(&rotaxis3, (const ChaVector3*)&ikaxis, (const ChaVector3*)&vec0);
		ChaVector3Normalize(&rotaxis3, &rotaxis3);
	}


	ChaVector3 rotaxis4;
	rotaxis4 = ChaVector3(0.0f, 0.0f, 0.0f);
	float rotrad4 = 0.0f;
	{
		ChaVector3 parbef, chilbef, tarbef;
		parbef = parworld;
		CalcShadowToPlane(chilworld, rotaxis3, parworld, &chilbef);
		CalcShadowToPlane(targetpos, rotaxis3, parworld, &tarbef);
		ChaVector3 vec0, vec1;
		vec0 = chilbef - parbef;
		ChaVector3Normalize(&vec0, &vec0);
		vec1 = tarbef - parbef;
		ChaVector3Normalize(&vec1, &vec1);

		ChaVector3Cross(&rotaxis4, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
		ChaVector3Normalize(&rotaxis4, &rotaxis4);

		float rotdot4;
		rotdot4 = ChaVector3Dot(&vec0, &vec1);
		rotdot4 = min(1.0f, rotdot4);
		rotdot4 = max(-1.0f, rotdot4);
		rotrad4 = (float)acos(rotdot4);
	}

	*dstaxis = rotaxis4;
	*dstrotrad = rotrad4;

	return 0;
}


int CModel::IKRotateOneFrame(int limitdegflag, CEditRange* erptr,
	int keyno, CBone* rotbone, CBone* parentbone,
	double curframe, double startframe, double applyframe,
	CQuaternion rotq0, bool keynum1flag, bool postflag, bool fromiktarget)
{

	//for return value
	int ismovable = 1;


	if (!erptr || !parentbone) {
		_ASSERT(0);
		return 1;
	}

	CQuaternion qForRot;
	CQuaternion qForHipsRot;

	if (fromiktarget == true) {
		qForRot = rotq0;
		qForHipsRot = rotq0;

		//bool calcaplyflag = false;
		//CalcQForRot(limitdegflag, calcaplyflag, 
		//	m_curmotinfo->motid, curframe, applyframe, rotq0,
		//	parentbone, parentbone,
		//	&qForRot, &qForHipsRot);

		//IKTragetの場合には
		//0.080で刻んで　徐々に近づける
		//近づきが足りない場合は　処理後に　ConstExecuteボタンを押す
		CQuaternion endq;
		CQuaternion curqForRot;
		CQuaternion curqForHipsRot;
		endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		qForRot.Slerp2(endq, 0.080f, &curqForRot);
		curqForHipsRot = curqForRot;
		bool infooutflag = true;
		ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
			infooutflag, 0, m_curmotinfo->motid, curframe, curqForRot, curqForHipsRot, fromiktarget);

		//bool infooutflag = true;
		//parentbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
		//	infooutflag, 0, m_curmotinfo->motid, curframe, qForRot, qForHipsRot, fromiktarget);

	}
	else if (keynum1flag) {
		qForRot = rotq0;
		qForHipsRot = rotq0;

		//bool calcaplyflag = false;
		//CalcQForRot(limitdegflag, calcaplyflag, 
		//	m_curmotinfo->motid, curframe, applyframe, rotq0,
		//	parentbone, parentbone,
		//	&qForRot, &qForHipsRot);

		bool infooutflag = true;
		ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
			infooutflag, 0, m_curmotinfo->motid, curframe, qForRot, qForHipsRot, fromiktarget);

		if ((fromiktarget != true) && (postflag != true)) {
			IKTargetVec(limitdegflag, erptr, curframe, postflag);
		}
	}
	else {
		if (g_pseudolocalflag == 1) {
			bool calcaplyflag = true;
			CalcQForRot(limitdegflag, calcaplyflag, 
				m_curmotinfo->motid, curframe, applyframe, rotq0,
				rotbone, parentbone,
				&qForRot, &qForHipsRot);
		}
		else {
			qForRot = rotq0;
			qForHipsRot = rotq0;
		}

		double changerate;
		if (fromiktarget == false) {
			changerate = (double)(*(g_motionbrush_value + (int)curframe));
		}
		else {
			changerate = 1.0;
		}


		bool infooutflag;
		if (curframe == applyframe) {
			infooutflag = true;
		}
		else {
			infooutflag = false;
		}

		double firstframe = 0.0;

		if (keyno == 0) {
			firstframe = curframe;
		}
		if (g_absikflag == 0) {
			if (g_slerpoffflag == 0) {
				CQuaternion endq;
				CQuaternion curqForRot;
				CQuaternion curqForHipsRot;
				endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
				qForRot.Slerp2(endq, 1.0 - changerate, &curqForRot);
				qForHipsRot.Slerp2(endq, 1.0 - changerate, &curqForHipsRot);

				ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
					infooutflag, 0, m_curmotinfo->motid, curframe, curqForRot, curqForHipsRot, fromiktarget);
			}
			else {
				ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
					infooutflag, 0, m_curmotinfo->motid, curframe, qForRot, qForHipsRot, fromiktarget);
			}
		}
		else {
			if (keyno == 0) {
				ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
					infooutflag, 0, m_curmotinfo->motid, curframe, qForRot, qForHipsRot, fromiktarget);
			}
			else {
				rotbone->SetAbsMatReq(limitdegflag, 0, m_curmotinfo->motid, curframe, firstframe);
			}
		}


		if ((fromiktarget != true) && (postflag != true)) {
			IKTargetVec(limitdegflag, erptr, curframe, postflag);
		}
	}
	
	return ismovable;
}

void CModel::ClearIKRotRec()
{
	ClearIKRotRecReq(GetTopBone());
}
void CModel::ClearIKRotRecUV()
{
	ClearIKRotRecUVReq(GetTopBone());
}

void CModel::ClearIKRotRecReq(CBone* srcbone)
{
	if (srcbone) {

		srcbone->ClearIKRotRec();

		if (srcbone->GetChild()) {
			ClearIKRotRecReq(srcbone->GetChild());
		}
		if (srcbone->GetBrother()) {
			ClearIKRotRecReq(srcbone->GetBrother());
		}
	}
}
void CModel::ClearIKRotRecUVReq(CBone* srcbone)
{
	if (srcbone) {

		srcbone->ClearIKRotRecUV();

		if (srcbone->GetChild()) {
			ClearIKRotRecUVReq(srcbone->GetChild());
		}
		if (srcbone->GetBrother()) {
			ClearIKRotRecUVReq(srcbone->GetBrother());
		}
	}
}

int CModel::IKRotateUnderIK(bool limitdegflag, CEditRange* erptr,
	int srcboneno, ChaVector3 targetpos, int maxlevel)
{

	SetIKTargetVec();

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		////g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	bool ishipsjoint = false;
	if (firstbone->GetParent()) {
		ishipsjoint = firstbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	int keynum;
	double startframe0, endframe0;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe0, &endframe0, &applyframe);
	startframe = startframe0;
	endframe = endframe0;


	CBone* curbone = firstbone;
	CBone* lastpar = curbone;
	SetBefEditMat(limitdegflag, erptr, curbone, maxlevel);
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}

	//CBone* lastpar = firstbone->GetParent();
	//int calcnum = 20;
	int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い

	int calccnt;
	for (calccnt = 0; calccnt < calcnum; calccnt++) {
		curbone = firstbone;
		lastpar = curbone;

		int levelcnt = 0;
		float currate = g_ikrate;

		while (curbone && lastpar && lastpar->GetParent() && ((maxlevel == 0) || (levelcnt < maxlevel)))
		{

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			////g_underIKRot = true;

			//CBone* parentbone = curbone->GetParent();
			CBone* parentbone = lastpar->GetParent();
			if (parentbone && (curbone->GetJointFPos() != parentbone->GetJointFPos())) {
				//UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);//curmp更新

				CRigidElem* curre = GetRigidElem(lastpar->GetBoneNo());
				if (curre && (curre->GetForbidRotFlag() != 0)) {

					//_ASSERT(0);

					//回転禁止の場合処理をスキップ
					if (parentbone) {
						lastpar = parentbone;
					}
					levelcnt++;
					currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
					continue;
				}

				ChaVector3 rotaxis2;
				float rotrad2;
				CalcAxisAndRotForIKRotate(limitdegflag,
					parentbone, firstbone, 
					m_curmotinfo->curframe, targetpos,
					&rotaxis2, &rotrad2);

				rotrad2 *= currate;

				double firstframe = 0.0;
				if (fabs(rotrad2) > 1.0e-4) {
					CQuaternion rotq0;
					rotq0.SetAxisAndRot(rotaxis2, rotrad2);

					//parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
					// 
					//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
					parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);

					//2023/01/22 : topposスライダーの位置のフレーム(３D表示中のフレーム)において　
					//制限角度により　回転出来ない場合は　リターンする
					//if (g_limitdegflag != 0) {
					if ((limitdegflag != false) && (g_wallscrapingikflag == 0)) {//2023/01/23
						//2023/01/28 IK時は　GetBtForce()チェックはしない　BtForce == 1でも角度制限する
						int ismovable = IsMovableRot(limitdegflag, m_curmotinfo->motid, applyframe, applyframe,
							rotq0, parentbone, parentbone);
						if (ismovable == 0) {
							////g_underIKRot = false;//2023/01/14 parent limited or not
							if (editboneforret) {
								return editboneforret->GetBoneNo();
							}
							else {
								return srcboneno;
							}
						}
					}


					double curframe = applyframe;
					int ismovable2 = 1;
					if (keynum >= 2) {
						int keyno = 0;

						bool keynum1flag = false;
						bool postflag = false;
						bool fromiktarget = false;
						ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
							keyno, 
							parentbone, parentbone,
							curframe, startframe, applyframe,
							rotq0, keynum1flag, postflag, fromiktarget);

						keyno++;
					}
					else {
						bool keynum1flag = true;
						bool postflag = false;
						bool fromiktarget = false;
						ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
							0, 
							parentbone, parentbone,
							m_curmotinfo->curframe, startframe, applyframe,
							rotq0, keynum1flag, postflag, fromiktarget);
					}


					//2023/03/04 制限角度に引っ掛かった場合には　やめて　次のジョイントの回転へ
					if ((ismovable2 == 0) && (g_wallscrapingikflag == 0)) {
						continue;
					}


					//parentboneのrotqを保存
					IKROTREC currotrec;
					currotrec.rotq = rotq0;
					currotrec.targetpos = targetpos;
					currotrec.lessthanthflag = false;
					parentbone->AddIKRotRec(currotrec);
					
					if (g_applyendflag == 1) {
						//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
						int tolast;
						for (tolast = (int)m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++) {
							(m_bonelist[0])->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
						}
					}
				}
				else {
					//rotqの回転角度が1e-4より小さい場合
					//ウェイトが小さいフレームにおいても　IKTargetが走るように記録する必要がある

					CQuaternion rotq0;
					rotq0.SetAxisAndRot(rotaxis2, rotrad2);

					IKROTREC currotrec;
					currotrec.rotq = rotq0;
					currotrec.targetpos = targetpos;
					currotrec.lessthanthflag = true;//!!!!!!!!!!!
					parentbone->AddIKRotRec(currotrec);
				}

			}

			if (parentbone) {
				lastpar = parentbone;

				//check ikstopflag
				if (parentbone->GetIKStopFlag()) {
					break;
				}
			}

			levelcnt++;

			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
		}

		//絶対モードの場合
		if ((calccnt == (calcnum - 1)) && g_absikflag && lastpar) {
			AdjustBoneTra(limitdegflag, erptr, lastpar);
		}
	}

	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret)
	{
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}

int CModel::IKRotatePostIK(bool limitdegflag, CEditRange* erptr,
	int srcboneno, int maxlevel)
{

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	bool ishipsjoint = false;
	if (firstbone->GetParent()) {
		ishipsjoint = firstbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	int keynum;
	double startframe0, endframe0;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe0, &endframe0, &applyframe);
	startframe = startframe0;
	endframe = endframe0;


	CBone* curbone = firstbone;
	CBone* lastpar = curbone;
	SetBefEditMat(limitdegflag, erptr, curbone, maxlevel);
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}


	//CBone* lastpar = firstbone->GetParent();
	//int calcnum = 20;
	int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い
	int calccnt;
	for (calccnt = 0; calccnt < calcnum; calccnt++) {
		curbone = firstbone;
		lastpar = curbone;

		int levelcnt = 0;
		float currate = g_ikrate;

		while (curbone && lastpar && lastpar->GetParent() && ((maxlevel == 0) || (levelcnt < maxlevel)))
		{

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;

			double firstframe = 0.0;

			//CBone* parentbone = curbone->GetParent();
			CBone* parentbone = lastpar->GetParent();
			if (parentbone && (curbone->GetJointFPos() != parentbone->GetJointFPos())) {
				int rotrecsize = parentbone->GetIKRotRecSize();
				if (rotrecsize > 0) {
					int rotrecno;
					for (rotrecno = 0; rotrecno < rotrecsize; rotrecno++) {
						IKROTREC currotrec = parentbone->GetIKRotRec(rotrecno);
						CQuaternion rotq0 = currotrec.rotq;
						bool lessthanthflag = currotrec.lessthanthflag;

						//UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);//curmp更新

						CRigidElem* curre = GetRigidElem(lastpar->GetBoneNo());
						if (curre && (curre->GetForbidRotFlag() != 0)) {

							//_ASSERT(0);

							//回転禁止の場合処理をスキップ
							if (parentbone) {
								lastpar = parentbone;
							}
							levelcnt++;
							currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
							break;//ikrotrec loopを抜けて　parentbone loopへ
						}

						//parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
						// 
						//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
						parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


						if (lessthanthflag == false) {
							if (keynum >= 2) {
								int keyno = 0;
								double curframe;
								for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {
									if (curframe != applyframe) {
										bool keynum1flag = false;
										bool postflag = true;
										bool fromiktarget = false;
										IKRotateOneFrame(limitdegflag, erptr,
											keyno, 
											parentbone, parentbone,
											curframe, startframe, applyframe,
											rotq0, keynum1flag, postflag, fromiktarget);
									}
								}
								keyno++;
							}
						}
						//else {
						//	bool keynum1flag = true;
						//	IKRotateOneFrame(limitdegflag, erptr,
						//		0, parentbone,
						//		m_curmotinfo->curframe, startframe, applyframe,
						//		rotq0, keynum1flag);
						//}
					}
				}

				if (g_applyendflag == 1) {
					//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
					int tolast;
					for (tolast = (int)m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++) {
						(m_bonelist[0])->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
					}
				}
			}

			if (parentbone) {
				lastpar = parentbone;

				//check ikstopflag
				if (parentbone->GetIKStopFlag()) {
					break;
				}
			}
			levelcnt++;
			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);

		}


		double curframe;
		for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {
			if (curframe != applyframe) {
				bool postflag = true;
				IKTargetVec(limitdegflag, erptr, curframe, postflag);
			}
		}
	}

	//絶対モードの場合
	if ((calccnt == (calcnum - 1)) && g_absikflag && lastpar) {
		AdjustBoneTra(limitdegflag, erptr, lastpar);
	}

	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret)
	{
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}

int CModel::IKRotate(bool limitdegflag, CEditRange* erptr, 
	int srcboneno, ChaVector3 targetpos, int maxlevel, double directframe)//default : directframe = -1.0
{

	CBone* firstbone = m_bonelist[ srcboneno ];
	if( !firstbone ){
		_ASSERT( 0 );
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	bool ishipsjoint = false;
	if (firstbone->GetParent()) {
		ishipsjoint = firstbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	int keynum;
	double startframe0, endframe0;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe0, &endframe0, &applyframe);
	if (directframe < 0.0) {
		startframe = startframe0;
		endframe = endframe0;
	}
	else {
		startframe = directframe;
		endframe = directframe;	
		keynum = 1;
	}
	

	CBone* curbone = firstbone;
	CBone* lastpar = curbone; 
	SetBefEditMat(limitdegflag, erptr, curbone, maxlevel);
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}


	//CBone* lastpar = firstbone->GetParent();
	//int calcnum = 20;
	int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い
	
	int calccnt;
	for( calccnt = 0; calccnt < calcnum; calccnt++ ){
		curbone = firstbone;
		lastpar = curbone;

		int levelcnt = 0;
		float currate = g_ikrate;

		while( curbone && lastpar && lastpar->GetParent() && ((maxlevel == 0) || (levelcnt < maxlevel)) )
		{

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;

			//CBone* parentbone = curbone->GetParent();
			CBone* parentbone = lastpar->GetParent();
			if( parentbone && (curbone->GetJointFPos() != parentbone->GetJointFPos()) ){
				//UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);//curmp更新

				CRigidElem* curre = GetRigidElem(lastpar->GetBoneNo());
				if (curre && (curre->GetForbidRotFlag() != 0)) {
					
					//_ASSERT(0);

					//回転禁止の場合処理をスキップ
					if (parentbone) {
						lastpar = parentbone;
					}
					levelcnt++;
					currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
					continue;
				}

				ChaVector3 rotaxis2;
				float rotrad2;
				CalcAxisAndRotForIKRotate(limitdegflag,
					parentbone, firstbone, 
					m_curmotinfo->curframe, targetpos,
					&rotaxis2, &rotrad2);

				rotrad2 *= currate;
				
				double firstframe = 0.0;
				if( fabs( rotrad2 ) > 1.0e-4 ){
					CQuaternion rotq0;
					rotq0.SetAxisAndRot( rotaxis2, rotrad2 );

					//parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
					// 
					//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
					parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


					//2023/01/22 : topposスライダーの位置のフレーム(３D表示中のフレーム)において　
					//制限角度により　回転出来ない場合は　リターンする
					//if (g_limitdegflag != 0) {
					if ((limitdegflag != false) && (g_wallscrapingikflag == 0)) {//2023/01/23
						//2023/01/28 IK時は　GetBtForce()チェックはしない　BtForce == 1でも角度制限する
						int ismovable = IsMovableRot(limitdegflag, m_curmotinfo->motid, applyframe, applyframe,
							rotq0, parentbone, parentbone);
						if (ismovable == 0) {
							//g_underIKRot = false;//2023/01/14 parent limited or not
							if (editboneforret) {
								return editboneforret->GetBoneNo();
							}
							else {
								return srcboneno;
							}
						}
					}


					if (keynum >= 2) {
						int keyno = 0;
						double curframe;
						for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {
							
							bool keynum1flag = false;
							bool postflag = false;
							bool fromiktarget = false;
							IKRotateOneFrame(limitdegflag, erptr,
								keyno, 
								parentbone, parentbone, 
								curframe, startframe, applyframe,
								rotq0, keynum1flag, postflag, fromiktarget);

							keyno++;
						}
					}
					else {
						bool keynum1flag = true;
						bool postflag = false;
						bool fromiktarget = false;
						IKRotateOneFrame(limitdegflag, erptr,
							0, 
							parentbone, parentbone,
							m_curmotinfo->curframe, startframe, applyframe,
							rotq0, keynum1flag, postflag, fromiktarget);
					}


					if (g_applyendflag == 1) {
						//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
						int tolast;
						for (tolast = (int)m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++) {
							(m_bonelist[0])->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
						}
					}
				}

			}


			//if( curbone->GetParent() ){
			//	lastpar = curbone->GetParent();
			//}
			//curbone = curbone->GetParent();
			if (parentbone) {
				lastpar = parentbone;

				//check ikstopflag
				if (parentbone->GetIKStopFlag()) {
					break;
				}
			}

			levelcnt++;

			currate = (float)pow( (double)g_ikrate, (double)g_ikfirst * (double)levelcnt );
		}

		//絶対モードの場合
		if( (calccnt == (calcnum - 1)) && g_absikflag && lastpar ){
			AdjustBoneTra(limitdegflag, erptr, lastpar);
		}
	}

	//if( lastpar ){
	//	return lastpar->GetBoneNo();
	//}else{
	//	return srcboneno;
	//}


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret)
	{
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}


//IKRotate()と違うところ　：　カメラ軸回転とカメラ軸に垂直な軸回転と　２回回転するところ
int CModel::IKRotateForIKTarget(bool limitdegflag, CEditRange* erptr,
	int srcboneno, ChaVector3 targetpos, int maxlevel, double directframe, bool postflag)
{

	double curframe = directframe;

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	bool ishipsjoint = false;
	if (firstbone->GetParent()) {
		ishipsjoint = firstbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);


	//if (postflag && (directframe == applyframe)) {
	//	return srcboneno;
	//}


	CBone* curbone = firstbone;
	CBone* lastpar = curbone;
	SetBefEditMat(limitdegflag, erptr, curbone, maxlevel);
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}


	//For IKTraget
	//カメラ軸回転とカメラ軸に垂直な軸回転と　２回実行する
	int calcnum = 3;

	int calccnt;
	for (calccnt = 1; calccnt <= calcnum; calccnt++) {
		curbone = firstbone;
		lastpar = curbone;

		int levelcnt = 0;
		float currate;
		//currate = g_ikrate;
		currate = 0.750f;

		while (curbone && lastpar && lastpar->GetParent() && ((maxlevel == 0) || (levelcnt < maxlevel)))
		{

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;

			//CBone* parentbone = curbone->GetParent();
			CBone* parentbone = lastpar->GetParent();
			if (parentbone && (curbone->GetJointFPos() != parentbone->GetJointFPos())) {
				//UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);//curmp更新

				CRigidElem* curre = GetRigidElem(lastpar->GetBoneNo());
				if (curre && (curre->GetForbidRotFlag() != 0)) {

					//_ASSERT(0);

					//回転禁止の場合処理をスキップ
					if (parentbone) {
						lastpar = parentbone;
					}
					levelcnt++;
					currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
					continue;
				}

				ChaVector3 rotaxis2;
				float rotrad2;
				//if ((calccnt % 2) != 0) {
				//	CalcAxisAndRotForIKRotate(limitdegflag,
				//		parentbone, firstbone,
				//		curframe, targetpos,
				//		&rotaxis2, &rotrad2);
				//}
				//else {
				//	CalcAxisAndRotForIKRotateVert(limitdegflag,
				//		parentbone, firstbone,
				//		curframe, targetpos,
				//		&rotaxis2, &rotrad2);
				//}


				ChaMatrix parentnodemat;
				parentnodemat = parentbone->GetNodeMat();
				ChaMatrix parentnoderot;
				parentnoderot = ChaMatrixRot(parentnodemat);
				ChaVector3 ikaxis;
				if ((calccnt % 3) == 0) {
					ikaxis.x = parentnoderot.data[MATI_11];
					ikaxis.y = parentnoderot.data[MATI_12];
					ikaxis.z = parentnoderot.data[MATI_13];
				}
				else if ((calccnt % 2) == 0) {
					ikaxis.x = parentnoderot.data[MATI_21];
					ikaxis.y = parentnoderot.data[MATI_22];
					ikaxis.z = parentnoderot.data[MATI_23];
				}
				else {
					ikaxis.x = parentnoderot.data[MATI_31];
					ikaxis.y = parentnoderot.data[MATI_32];
					ikaxis.z = parentnoderot.data[MATI_33];
				}
				bool nearflag = CalcAxisAndRotForIKRotateAxis(limitdegflag,
					parentbone, firstbone,
					curframe, targetpos,
					ikaxis,
					&rotaxis2, &rotrad2);

				//rotrad2 *= currate;

				double firstframe = 0.0;
				if ((nearflag == false) && (fabs(rotrad2) > 1.0e-4)) {
					CQuaternion rotq0;
					rotq0.SetAxisAndRot(rotaxis2, rotrad2);

					//parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
					// 
					//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
					parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


					//IKRotateは壁すりIKで行うので　回転可能かどうかのチェックはここではしない


					int keyno = 0;
					bool keynum1flag = false;
					bool fromiktarget = true;
					IKRotateOneFrame(limitdegflag, erptr,
						keyno, 
						parentbone, parentbone,
						curframe, startframe, applyframe,
						rotq0, keynum1flag, postflag, fromiktarget);
					keyno++;

					if (g_applyendflag == 1) {
						//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
						int tolast;
						for (tolast = (int)m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++) {
							(m_bonelist[0])->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
						}
					}
				}

			}

			if (parentbone) {
				lastpar = parentbone;

				//コンストレイント用回転も　IKStopで止める必要有
				//体の中心まで回転を伝えた方が　コンストレイントしやすいが
				//shoulderのIKStopで回転を止めない場合
				//右手と左手のコンストレイント順番により　どちらかにしか拘束できなくなる
				//check ikstopflag
				if (parentbone->GetIKStopFlag()) {
					break;
				}
			}


			levelcnt++;

			//currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
		}

		//絶対モードの場合
		if ((calccnt == calcnum) && g_absikflag && lastpar) {
			AdjustBoneTra(limitdegflag, erptr, lastpar);
		}
	}

	//if( lastpar ){
	//	return lastpar->GetBoneNo();
	//}else{
	//	return srcboneno;
	//}


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret)
	{
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}

//int CModel::PhysicsRot(CEditRange* erptr, int srcboneno, ChaVector3 targetpos, int maxlevel)
//{
//
//	//Memo 20180602
//	//PhysicsRotの結果が不安定になるケースが絞り込めてきた。
//	//parrent-->grand parentのボーンが視線に対して平行のときにこの関数を使うとcurrent-->parentボーンがぐるんぐるん回ってしまうことが多い。
//	//腕-->肩と鎖骨が直角なときに腕のIKで起こりやすい。
//	//PhysicsRotAxisDelta関数で回転軸を指定してIKすると安定する。
//
//	//Memo 20180604
//	//上記のぐるんぐるん回る問題
//	//Mediaフォルダのサンプルのfbxファイルの「腕の座標軸がX軸ベースになっていない」ことが分かった。
//	//PhysicsRotボタンを押すとサンプルの該当ボーンの座標軸がX軸ベースでなくなるようだった。
//	//CalcShadowToPlaneの問題の可能性もある。見直したが忘れていてわからない部分があった。
//
//	//Memo 20180606
//	//座標軸は直した。
//	//カレント座標系をデフォルトにした。
//	//ぐるんぐるんはまだ直っていない。
//
//	//Memo 20180619
//	//ぐるんぐるん症状が直った（ただしBtApplyボタンを押さない場合）
//	//newbtmatの計算を修正した。
//
//	//Memo 20180623
//	//BtApplyボタンを押してもぐるんぐるん症状が出なくなった。
//	//ぐるんぐるんする場合は、ボーンが反対回りしているようだった。
//	//回転してターゲットに近づかない場合にはキャンセルすることにより直った。
//	//Bindcapsulematを利用するようにした。
//	//制限角度部分をBindcapsulematを利用するように修正する必要あり（バインドポーズ基点の角度にする必要あり）。
//
//	//Memo 20180922
//	//マニピュレータの座標系（Bindcapsulemat基準）のオイラー角による角度制限。検証中。
//
//	//Memo 20180925
//	//Eulerは通常IK時のものを使用。物理マトリックスは通常IKのdiffで計算。
//
//
//	CBone* firstbone = m_bonelist[srcboneno];
//	if (!firstbone){
//		_ASSERT(0);
//		return -1;
//	}
//
//	CBone* curbone = firstbone;
//	CBone* lastpar = curbone;
//	CBone* parentbone = curbone->GetParent();
//	CBone* gparentbone = 0;
//	if (!parentbone){
//		return 0;
//	}
//	gparentbone = parentbone->GetParent();
//	if (!gparentbone){
//		//grand parentがルートボーンの場合に、まだうまくいかないのでスキップ
//		return 0;
//	}
//	CBone* editboneforret = 0;
//	if (firstbone->GetParent()) {
//		editboneforret = firstbone->GetParent();
//	}
//	else {
//		editboneforret = firstbone;
//	}
//
//
//	ChaVector3 ikaxis = g_camtargetpos - g_camEye;
//	ChaVector3Normalize(&ikaxis, &ikaxis);
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	SetBefEditMat(erptr, curbone, maxlevel);
//
//	float currate = g_physicsmvrate;
//
//	MOTINFO* curmi = GetCurMotInfo();
//	if (!curmi) {
//		return 0;
//	}
//
//	if (parentbone){
//		//CBone* gparentbone = parentbone->GetParent();
//		//CBone* childbone = parentbone->GetChild();
//		CBone* childbone = curbone;
//		int isfirst = 1;
//		//while (childbone){
//		if (childbone){
//			if (childbone->GetJointFPos() != parentbone->GetJointFPos()){
//				double dist0, dist1;// , dist2;
//				ChaVector3 parworld, chilworld;
//				ChaVector3 tmpparentfpos = parentbone->GetJointFPos();
//				ChaVector3 tmpchildfpos = childbone->GetJointFPos();
//				ChaMatrix tmpparentbtmat = parentbone->GetBtMat();
//				ChaVector3TransformCoord(&parworld, &tmpparentfpos, &tmpparentbtmat);
//				ChaVector3TransformCoord(&chilworld, &tmpchildfpos, &tmpparentbtmat);
//				//ChaVector3TransformCoord(&parworld, &(parentbone->GetJointFPos()), &(parentbone->GetBtMat()));
//				//ChaVector3TransformCoord(&chilworld, &(childbone->GetJointFPos()), &(childbone->GetBtMat()));//<--乱れる
//
//
//				ChaVector3 parbef, chilbef, tarbef;
//
//				//parent jointが乗っている平面上
//				parbef = parworld;
//				CalcShadowToPlane(chilworld, ikaxis, parworld, &chilbef);
//				CalcShadowToPlane(targetpos, ikaxis, parworld, &tarbef);
//
//				////ドラッグするジョイントが乗っている平面上
//				//chilbef = chilworld;
//				//CalcShadowToPlane(parworld, ikaxis, chilworld, &parbef);
//				//CalcShadowToPlane(targetpos, ikaxis, chilworld, &tarbef);
//				
//
//				ChaVector3 child2target;
//				child2target = targetpos - chilworld;
//				dist0 = ChaVector3LengthDbl(&child2target);
//
//
//				ChaVector3 vec0, vec1;
//				vec0 = chilbef - parbef;
//				ChaVector3Normalize(&vec0, &vec0);
//				vec1 = tarbef - parbef;
//				ChaVector3Normalize(&vec1, &vec1);
//
//				//double chkdot = ChaVector3DotDbl(&vec0, &vec1);
//				//if (fabs(chkdot) >= 0.999) {
//				//	return srcboneno;//!!!!!!!!!!!!!!!!!!!
//				//}
//
//				ChaVector3 rotaxis2;
//				ChaVector3Cross(&rotaxis2, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
//				ChaVector3Normalize(&rotaxis2, &rotaxis2);
//
//				double rotdot2, rotrad2;
//				rotdot2 = ChaVector3DotDbl(&vec0, &vec1);
//				rotdot2 = min(1.0f, rotdot2);
//				rotdot2 = max(-1.0f, rotdot2);
//				rotrad2 = acos(rotdot2);
//				rotrad2 *= currate;
//				double firstframe = 0.0;
//				if (fabs(rotrad2) > 1.0e-4){
//					//double curframe = startframe;//!!!!!!!!!
//					double curframe = applyframe;//!!!!!!!!!
//
//					CQuaternion rotq1;
//					rotq1.SetAxisAndRot(rotaxis2, rotrad2);
//
//					ChaVector3 rotcenter;
//					ChaVector3 tmpparentfpos2 = parentbone->GetJointFPos();
//					ChaMatrix tmpparentbtmat2 = parentbone->GetBtMat();
//					ChaVector3TransformCoord(&rotcenter, &tmpparentfpos2, &tmpparentbtmat2);
//					ChaMatrix befrot, aftrot;
//					ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//					ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//					ChaMatrix rotmat1 = befrot * rotq1.MakeRotMatX() * aftrot;
//					//ChaMatrix newbtmat1 = parentbone->GetBtMat() * rotmat1;// *tramat;
//
//					ChaMatrix tmpmat0 = parentbone->GetBtMat() * rotmat1;// *tramat;
//					ChaVector3 tmppos;
//					ChaVector3TransformCoord(&tmppos, &tmpparentfpos2, &tmpmat0);
//					ChaVector3 diffvec;
//					diffvec = rotcenter - tmppos;
//					ChaMatrix tmptramat;
//					ChaMatrixIdentity(&tmptramat);
//					ChaMatrixTranslation(&tmptramat, diffvec.x, diffvec.y, diffvec.z);
//					ChaMatrix newbtmat1 = tmpmat0 * tmptramat;// *tramat;
//
//
//					ChaVector3 childworld1;
//					ChaVector3 tmpchildfpos2 = childbone->GetJointFPos();
//					ChaVector3TransformCoord(&childworld1, &tmpchildfpos2, &newbtmat1);
//					child2target = targetpos - childworld1;
//					dist1 = ChaVector3LengthDbl(&child2target);
//
//
//					//比較と選択
//					CQuaternion rotq;
//					ChaMatrix newbtmat;
//					//if ((dist0 * 0.9) >= dist1) {//10%近づかなかったらキャンセル。乱れ防止策。
//						rotq = rotq1;
//						newbtmat = newbtmat1;
//					//}
//					//else {
//					//	return srcboneno;
//					//}
//						bool infooutflag = true;
//					int onlycheck = 1;
//					int isbonemovable;
//					if (g_limitdegflag == true) {
//						isbonemovable = parentbone->SetWorldMat(infooutflag, 1, curmi->motid, curframe, newbtmat, onlycheck);
//						if (isbonemovable == 0) {
//							return 0;//!!!!!!!!!!!!!!!!!!!!!!!!!
//						}
//					}
//					else {
//						isbonemovable = 1;
//					}
//
//					ChaMatrix invfirstmat = parentbone->GetCurrentZeroFrameInvMat(0);
//					ChaMatrix diffworld = invfirstmat * newbtmat;
//					CBtObject* setbto = parentbone->GetBtObject(childbone);
//					ChaMatrix newrigidmat;
//					if (setbto) {
//						newrigidmat = setbto->GetFirstTransformMatX() * diffworld;
//					}
//					else {
//						::MessageBoxA(NULL, "IKRotateRagdoll : setbto NULL error", "error", MB_OK);
//						return -1;
//					}
//					
//					ChaVector3 newparentpos, newchildpos;
//					ChaVector3 jointfpos;
//					jointfpos = parentbone->GetJointFPos();
//					ChaVector3TransformCoord(&newparentpos, &jointfpos, &newbtmat);
//					jointfpos = childbone->GetJointFPos();
//					ChaVector3TransformCoord(&newchildpos, &jointfpos, &newbtmat);
//
//					ChaVector3 rigidcenter = (newparentpos + newchildpos) * 0.5f;
//
//
//					CQuaternion tmpq;
//					tmpq.RotationMatrix(newrigidmat);
//					btQuaternion btrotq(tmpq.x, tmpq.y, tmpq.z, tmpq.w);
//
//					btTransform worldtra;
//					worldtra.setIdentity();
//					worldtra.setRotation(btrotq);
//					worldtra.setOrigin(btVector3(rigidcenter.x, rigidcenter.y, rigidcenter.z));
//
//					if (isbonemovable == 1) {
//						//CQuaternion eulnewrot;
//						//eulnewrot.SetRotationXYZ(&eulaxisq, eul);
//
//						setbto->GetRigidBody()->getMotionState()->setWorldTransform(worldtra);
//						//setbto->GetRigidBody()->forceActivationState(ACTIVE_TAG);
//						//setbto->GetRigidBody()->setDeactivationTime(30000.0);
//						setbto->GetRigidBody()->setDeactivationTime(0.0);
//
//						//if (isfirst == 1) {
//						//parentbone->SetBtMat(newbtmat);
//						//IKボーンはKINEMATICだから。
//						//parentbone->GetCurMp().SetWorldMat(newbtmat);
//						isfirst = 0;
//						//}
//
//						//MOTINFO* curmi = GetCurMotInfo();
//						//parentbone->SetWorldMat(0, curmi->motid, curmi->curframe, newbtmat);
//						//parentbone->SetWorldMat(1, curmi->motid, applyframe, newbtmat);
//					}
//
//				}
//
//			}
//
//			//childbone = childbone->GetBrother();
//		}
//	}
//	//return srcboneno;
//
//	if (editboneforret) {
//		return editboneforret->GetBoneNo();
//	}
//	else {
//		return srcboneno;
//	}
//
//
//}

//int CModel::PhysicsRotAxisDelta(CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat)
//{
//	if (!m_curmotinfo) {
//		return 0;
//	}
//
//
//
//	//float rotrad = delta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
//	int calcnum = 1;
//	//int calcnum = 4;//ctrlを押しながらドラッグでdelta * 0.25になっている。
//	float rotrad = delta / 10.0f * (float)PAI / 12.0f / (float)calcnum * g_physicsmvrate;//PhysicsIKプレートのEditRateスライダーで倍率設定.
//	if (fabs(rotrad) < (0.020 * DEG2PAI)) {
//		return 0;
//	}
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	CBone* curbone = m_bonelist[srcboneno];
//	if (!curbone) {
//		return 0;
//	}
//	CBone* firstbone = curbone;
//	CBone* parentbone = 0;
//	CBone* lastbone = 0;
//	CBone* topbone = GetTopBone();
//	CBone* editboneforret = 0;
//	if (firstbone->GetParent()) {
//		editboneforret = firstbone->GetParent();
//	}
//	else {
//		editboneforret = firstbone;
//	}
//
//
//	int calccnt;
//	for (calccnt = 0; calccnt < calcnum; calccnt++) {
//		curbone = firstbone;
//		if (!curbone) {
//			return 0;
//		}
//		lastbone = curbone;
//
//		float currate = 1.0f;
//
//		double firstframe = 0.0;
//		int levelcnt = 0;
//
//		//while (curbone && ((maxlevel == 0) || (levelcnt < maxlevel))) {
//		parentbone = curbone->GetParent();
//		if (!parentbone) {
//			break;
//		}
//
//		float rotrad2 = currate * rotrad;
//		//float rotrad2 = rotrad;
//		if (fabs(rotrad2) < (0.02 * DEG2PAI)) {
//			break;
//		}
//
//		CRigidElem* curre = GetRigidElem(curbone->GetBoneNo());
//		if (curre && curre->GetForbidRotFlag() != 0) {
//			//回転禁止の場合処理をスキップ
//			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
//			lastbone = curbone;
//			curbone = curbone->GetParent();
//			levelcnt++;
//			continue;
//		}
//
//
//		ChaVector3 axis0;
//		CQuaternion localq;
//		//if (axiskind == PICK_X) {
//		//	axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
//		//	localq.SetAxisAndRot(axis0, rotrad2);
//		//}
//		//else if (axiskind == PICK_Y) {
//		//	axis0 = ChaVector3(0.0f, 1.0f, 0.0f);
//		//	localq.SetAxisAndRot(axis0, rotrad2);
//		//}
//		//else if (axiskind == PICK_Z) {
//		//	axis0 = ChaVector3(0.0f, 0.0f, 1.0f);
//		//	localq.SetAxisAndRot(axis0, rotrad2);
//		//}
//		//else {
//		//	_ASSERT(0);
//		//	return 1;
//		//}
//		if ((axiskind == PICK_X) || (axiskind == PICK_SPA_X)) {
//			axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
//		}
//		else if ((axiskind == PICK_Y) || (axiskind == PICK_SPA_Y)) {
//			axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
//		}
//		else if ((axiskind == PICK_Z) || (axiskind == PICK_SPA_Z)) {
//			axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
//		}
//		else {
//			_ASSERT(0);
//			return 1;
//		}
//		ChaVector3Normalize(&axis0, &axis0);
//		localq.SetAxisAndRot(axis0, rotrad2);
//		//ChaMatrix transmat;
//		//transmat = localq.MakeRotMatX();
//
//
//
//		//ChaMatrix selectmat;
//		ChaMatrix invselectmat;
//		//int multworld = 1;
//		//selectmat = curbone->CalcManipulatorMatrix(0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);//curmotinfo!!!
//		ChaMatrixInverse(&invselectmat, NULL, &selectmat);
//
//
//		//ChaMatrix rotinvworld = firstbone->GetCurMp().GetInvWorldMat();
//		ChaMatrix rotinvworld = ChaMatrixInv(firstbone->GetBtMat());
//		rotinvworld.data[MATI_41] = 0.0f;
//		rotinvworld.data[MATI_42] = 0.0f;
//		rotinvworld.data[MATI_43] = 0.0f;
//		ChaMatrix rotselect = selectmat;
//		rotselect.data[MATI_41] = 0.0f;
//		rotselect.data[MATI_42] = 0.0f;
//		rotselect.data[MATI_43] = 0.0f;
//		ChaMatrix rotinvselect = invselectmat;
//		rotinvselect.data[MATI_41] = 0.0f;
//		rotinvselect.data[MATI_42] = 0.0f;
//		rotinvselect.data[MATI_43] = 0.0f;
//
//		CQuaternion rotinvselectq, rotselectq;
//		rotinvselectq.RotationMatrix(rotinvselect);
//		rotselectq.RotationMatrix(rotselect);
//
//		CQuaternion rotq;
//		if (parentbone) {
//			//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//			////CMotionPoint transmp;
//			////transmp.CalcQandTra(transmat, firstbone);
//			////rotq = transmp.GetQ();
//			//rotq.RotationMatrix(transmat);
//			//rotq = rotinvselectq * localq * rotselectq;
//			rotq = localq;
//
//			ChaVector3 rotcenter;
//			ChaVector3 tmpparentfpos = parentbone->GetJointFPos();
//			ChaMatrix tmpparentbtmat = parentbone->GetBtMat();
//			ChaVector3TransformCoord(&rotcenter, &tmpparentfpos, &tmpparentbtmat);
//			ChaMatrix befrot, aftrot;
//			ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//			ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//			ChaMatrix rotmat1 = befrot * rotq.MakeRotMatX() * aftrot;
//			
//			//ChaMatrix newbtmat1 = parentbone->GetBtMat() * rotmat1;// *tramat;
//
//			ChaMatrix tmpmat0 = parentbone->GetBtMat() * rotmat1;// *tramat;
//			ChaVector3 tmppos;
//			ChaVector3TransformCoord(&tmppos, &tmpparentfpos, &tmpmat0);
//			ChaVector3 diffvec;
//			diffvec = rotcenter - tmppos;
//			ChaMatrix tmptramat;
//			ChaMatrixIdentity(&tmptramat);
//			ChaMatrixTranslation(&tmptramat, diffvec.x, diffvec.y, diffvec.z);
//			ChaMatrix newbtmat1 = tmpmat0 * tmptramat;// *tramat;
//
//			bool infooutflag = true;
//			int onlycheck = 1;
//			int isbonemovable;
//			if (g_limitdegflag == true) {
//				isbonemovable = parentbone->SetWorldMat(infooutflag, 1, m_curmotinfo->motid, startframe, newbtmat1, onlycheck);
//			}
//			else {
//				isbonemovable = 1;
//			}
//			//parentbone->RotBoneQReq(0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq);
//			if (isbonemovable == 1) {
//				ChaMatrix invfirstmat = parentbone->GetCurrentZeroFrameInvMat(0);
//				ChaMatrix diffworld = invfirstmat * newbtmat1;
//				CBtObject* setbto = parentbone->GetBtObject(curbone);
//				ChaMatrix newrigidmat;
//				if (setbto) {
//					newrigidmat = setbto->GetFirstTransformMatX() * diffworld;
//				}
//				else {
//					::MessageBoxA(NULL, "IKRotateRagdoll : setbto NULL error", "error", MB_OK);
//					return -1;
//				}
//
//				ChaVector3 newparentpos, newchildpos;
//				ChaVector3 jointfpos;
//				jointfpos = parentbone->GetJointFPos();
//				ChaVector3TransformCoord(&newparentpos, &jointfpos, &newbtmat1);
//				jointfpos = curbone->GetJointFPos();
//				ChaVector3TransformCoord(&newchildpos, &jointfpos, &newbtmat1);
//
//				ChaVector3 rigidcenter = (newparentpos + newchildpos) * 0.5f;
//
//				CQuaternion tmpq;
//				tmpq.RotationMatrix(newrigidmat);
//				btQuaternion btrotq(tmpq.x, tmpq.y, tmpq.z, tmpq.w);
//
//				btTransform worldtra;
//				worldtra.setIdentity();
//				worldtra.setRotation(btrotq);
//				worldtra.setOrigin(btVector3(rigidcenter.x, rigidcenter.y, rigidcenter.z));
//
//				setbto->GetRigidBody()->getMotionState()->setWorldTransform(worldtra);
//				setbto->GetRigidBody()->setDeactivationTime(0.0);
//
//			}
//
//
//		}
//
//		//}
//	}
//
//	if (editboneforret){
//		return editboneforret->GetBoneNo();
//	}
//	else{
//		return srcboneno;
//	}
//
//}

//int CModel::PhysicsRotAxisDelta(CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat)
//{
//	if (!m_curmotinfo){
//		return 0;
//	}
//
//
//	CBone* curbone = m_bonelist[srcboneno];
//	if (!curbone){
//		return 0;
//	}
//	CBone* parentbone = curbone->GetParent();
//	if (!parentbone){
//		return 0;
//	}
//	if (!parentbone->GetParent()){
//		//grand parentがルートボーンの場合に、まだうまくいかないのでスキップ
//		return 0;
//	}
//
//
//	int calcnum = 3;
//
//	float rotrad = delta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
//	if (fabs(rotrad) < (0.02f * (float)DEG2PAI)){
//		return 0;
//	}
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	SetBefEditMat(erptr, curbone, maxlevel);//!!!!!!!!!!!!
//
//	if (parentbone){
//		CBone* gparentbone = parentbone->GetParent();
//		CBone* childbone = parentbone->GetChild();
//		int isfirst = 1;
//		float currate = 1.0f;
//		double firstframe = 0.0;
//		int levelcnt = 0;
//
//		while (childbone){
//			float rotrad2 = currate * rotrad;
//			//float rotrad2 = rotrad;
//			if (fabs(rotrad2) < (0.02f * (float)DEG2PAI)){
//				continue;
//			}
//
//			ChaVector3 axis0;
//			CQuaternion localq;
//			if (axiskind == PICK_X){
//				axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
//				localq.SetAxisAndRot(axis0, -rotrad2);
//			}
//			else if (axiskind == PICK_Y){
//				axis0 = ChaVector3(0.0f, 1.0f, 0.0f);
//				localq.SetAxisAndRot(axis0, -rotrad2);
//			}
//			else if (axiskind == PICK_Z){
//				axis0 = ChaVector3(0.0f, 0.0f, 1.0f);
//				localq.SetAxisAndRot(axis0, -rotrad2);
//			}
//			else{
//				_ASSERT(0);
//				return 1;
//			}
//
//			ChaMatrix invselectmat;
//			ChaMatrixInverse(&invselectmat, NULL, &selectmat);
//			ChaMatrix rotselect = selectmat;
//			rotselect._41 = 0.0f;
//			rotselect._42 = 0.0f;
//			rotselect._43 = 0.0f;
//			ChaMatrix rotinvselect = invselectmat;
//			rotinvselect._41 = 0.0f;
//			rotinvselect._42 = 0.0f;
//			rotinvselect._43 = 0.0f;
//
//			ChaMatrix gparrotmat, invgparrotmat;
//			if (parentbone->GetParent()){
//				gparrotmat = parentbone->GetParent()->GetBtMat();
//				ChaMatrixInverse(&invgparrotmat, NULL, &gparrotmat);
//
//				gparrotmat._41 = 0.0f;
//				gparrotmat._42 = 0.0f;
//				gparrotmat._43 = 0.0f;
//
//				invgparrotmat._41 = 0.0f;
//				invgparrotmat._42 = 0.0f;
//				invgparrotmat._43 = 0.0f;
//			}
//			else{
//				ChaMatrixIdentity(&gparrotmat);
//				ChaMatrixIdentity(&invgparrotmat);
//			}
//			ChaMatrix parrotmat, invparrotmat;
//			parrotmat = parentbone->GetBtMat();
//			ChaMatrixInverse(&invparrotmat, NULL, &parrotmat);
//			parrotmat._41 = 0.0f;
//			parrotmat._42 = 0.0f;
//			parrotmat._43 = 0.0f;
//
//
//			//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//			ChaMatrix transmat = rotselect * localq.MakeRotMatX() * rotinvselect;
//
//			CMotionPoint transmp;
//			transmp.CalcQandTra(transmat, parentbone);
//			CQuaternion rotq;
//			rotq = transmp.GetQ();
//
//
//			ChaVector3 parworld, chilworld;
//			ChaVector3TransformCoord(&parworld, &(parentbone->GetJointFPos()), &(parentbone->GetBtMat()));
//			ChaVector3TransformCoord(&chilworld, &(childbone->GetJointFPos()), &(parentbone->GetBtMat()));
//
//
//			ChaMatrix newbtmat;
//			ChaVector3 rotcenter;
//			//rotcenter = parworld;
//			rotcenter = parentbone->GetJointFPos();
//
//			ChaMatrix befrot, aftrot;
//			ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//			ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//			ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
//			newbtmat = rotmat * parentbone->GetBtMat();
//
//
//			ChaMatrix firstworld = parentbone->GetStartMat2();
//			ChaMatrix invfirstworld;
//			ChaMatrixInverse(&invfirstworld, NULL, &firstworld);
//
//			ChaMatrix diffworld = invfirstworld * newbtmat;
//			CRigidElem* curre = parentbone->GetRigidElem(childbone);
//			ChaMatrix newrigidmat;
//			if (curre){
//				newrigidmat = curre->GetFirstcapsulemat() * diffworld;
//			}
//			else{
//				::MessageBoxA(NULL, "PhysicsRotAxisDelta : curre NULL error", "error", MB_OK);
//				return -1;
//			}
//
//
//			ChaVector3 newparentpos, newchildpos;
//			ChaVector3 jointfpos;
//			jointfpos = parentbone->GetJointFPos();
//			ChaVector3TransformCoord(&newparentpos, &jointfpos, &newbtmat);
//			jointfpos = childbone->GetJointFPos();
//			ChaVector3TransformCoord(&newchildpos, &jointfpos, &newbtmat);
//
//			ChaVector3 rigidcenter = (newparentpos + newchildpos) * 0.5f;
//
//
//			CQuaternion tmpq;
//			tmpq.RotationMatrix(newrigidmat);
//			btQuaternion btrotq(tmpq.x, tmpq.y, tmpq.z, tmpq.w);
//
//
//			btTransform worldtra;
//			worldtra.setIdentity();
//			worldtra.setRotation(btrotq);
//			worldtra.setOrigin(btVector3(rigidcenter.x, rigidcenter.y, rigidcenter.z));
//
////角度制限　ここから
//			if (gparentbone){
//				CBtObject* parbto = gparentbone->GetBtObject(parentbone);
//				if (parbto){
//					CBtObject* setbto = parentbone->GetBtObject(childbone);
//					if (setbto){
//						btMatrix3x3 firstworldmat = setbto->GetFirstTransformMat();
//
//						btGeneric6DofSpringConstraint* dofC = parbto->FindConstraint(parentbone, childbone);
//						if (dofC){
//
//							//constraint変化分　以下3行　　CreateBtObjectをしたときの状態を基準にした角度になっている。つまりシミュ開始時が０度。
//							//btTransform contraA = dofC->getCalculatedTransformA();
//							//btTransform contraB = dofC->getCalculatedTransformB();
//							//btMatrix3x3 eulmat = contraA.getBasis().inverse() * contraB.getBasis() * contraA.getBasis();
//
//
//							////親ボーンとの角度がオイラー角に入る
//							//btTransform contraA = dofC->getCalculatedTransformA();
//							//btTransform contraB = dofC->getCalculatedTransformB();
//							//btTransform parworldtra;
//							//parbto->GetRigidBody()->getMotionState()->getWorldTransform(parworldtra);
//							//btMatrix3x3 diffmat = worldtra.getBasis() * parworldtra.getBasis().inverse();
//							//btMatrix3x3 eulmat = contraA.getBasis().inverse() * diffmat * contraA.getBasis();
//
//							btTransform contraA = dofC->getCalculatedTransformA();
//							btTransform contraB = dofC->getCalculatedTransformB();
//							btMatrix3x3 diffmat = firstworldmat.inverse() * worldtra.getBasis();
//							btMatrix3x3 eulmat = contraA.getBasis().inverse() * diffmat * contraA.getBasis();
//							//btMatrix3x3 eulmat = contraA.getBasis() * diffmat * contraA.getBasis().inverse();
//
//
//
//							btScalar eulz = 0.0;
//							btScalar euly = 0.0;
//							btScalar eulx = 0.0;
//							ChaVector3 eul = ChaVector3(0.0f, 0.0f, 0.0f);
//							//worldtra.getBasis().getEulerZYX(eulz, euly, eulx, 1);
//							eulmat.getEulerZYX(eulz, euly, eulx, 1);
//							eul.x = eulx * 180.0 / PAI;
//							eul.y = euly * 180.0 / PAI;
//							eul.z = eulz * 180.0 / PAI;
//
//							int ismovable = parentbone->ChkMovableEul(eul);
//							char strmsg[256];
//							//sprintf_s(strmsg, 256, "needmodify 0 : neweul [%f, %f, %f] : dof [%f, %f, %f] : ismovable %d\n", eul.x, eul.y, eul.z, dofx, dofy, dofz, ismovable);
//							sprintf_s(strmsg, 256, "needmodify 0 : neweul [%f, %f, %f] : ismovable %d\n", eul.x, eul.y, eul.z, ismovable);
//							OutputDebugStringA(strmsg);
//
//
//							//Q2EulZYXbtのテスト　以下8行
//							CQuaternion eulq;
//							eulq = QMakeFromBtMat3x3(eulmat);
//							int needmodifyflag = 0;
//							ChaVector3 testbefeul = ChaVector3(0.0f, 0.0f, 0.0f);
//							ChaVector3 testeul = ChaVector3(0.0f, 0.0f, 0.0f);
//							//eulq.Q2EulZYXbt(needmodifyflag, 0, testbefeul, &testeul);
//							eulq.Q2EulXYZ(0, testbefeul, &testeul);//bulletの回転順序は数値検証の結果XYZ。(ZYXではない)。
//							sprintf_s(strmsg, 256, "testeul [%f, %f, %f]\n", testeul.x, testeul.y, testeul.z);
//							OutputDebugStringA(strmsg);
//
//
//							if (ismovable != 1){
//								childbone = childbone->GetBrother();
//								continue;
//							}
//
//							setbto->GetRigidBody()->getMotionState()->setWorldTransform(worldtra);
//							//setbto->GetRigidBody()->forceActivationState(ACTIVE_TAG);
//							//setbto->GetRigidBody()->setDeactivationTime(30000.0);
//							setbto->GetRigidBody()->setDeactivationTime(0.0);
//
//							if (isfirst == 1){
//								parentbone->SetBtMat(newbtmat);
//								//IKボーンはKINEMATICだから。
//								parentbone->GetCurMp().SetWorldMat(newbtmat);
//								isfirst = 0;
//							}
//
//						}
//					}
//				}
////角度制限　ここまで
//			}
//			childbone = childbone->GetBrother();
//		}
//	}
//
//	if (curbone){
//		return curbone->GetBoneNo();
//	}
//	else{
//		return srcboneno;
//	}
//
//}
//


//int CModel::PhysicsRigControl(int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig)
//{
//	if (depthcnt >= 10){
//		_ASSERT(0);
//		return 0;//!!!!!!!!!!!!!!!!!
//	}
//	depthcnt++;
//
//	if (!m_curmotinfo){
//		return 0;
//	}
//
//	int calcnum = 3;
//
//	float rotrad = srcdelta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
//	//if (fabs(rotrad) < (0.02f * (float)DEG2PAI)){
//	//	return 0;
//	//}
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	CBone* curbone = m_bonelist[srcboneno];
//	if (!curbone){
//		return 0;
//	}
//	CBone* parentbone = 0;
//	CBone* lastbone = 0;
//
//	//int calccnt;
//	//for (calccnt = 0; calccnt < calcnum; calccnt++){
//	double firstframe = 0.0;
//
//	int elemno;
//	for (elemno = 0; elemno < ikcustomrig.elemnum; elemno++){
//		RIGELEM currigelem = ikcustomrig.rigelem[elemno];
//		if (currigelem.rigrigboneno >= 0){
//			//rigのrig
//			CBone* rigrigbone = GetBoneByID(currigelem.rigrigboneno);
//			if (rigrigbone){
//				int rigrigno = currigelem.rigrigno;
//				if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)){
//					if (currigelem.transuv[uvno].enable == 1){
//						CUSTOMRIG rigrig = rigrigbone->GetCustomRig(rigrigno);
//						PhysicsRigControl(depthcnt, erptr, rigrigbone->GetBoneNo(), uvno, srcdelta * currigelem.transuv[uvno].applyrate, rigrig);
//					}
//				}
//			}
//		}
//		else{
//			//rigelem
//			curbone = GetBoneByID(currigelem.boneno);
//			if (curbone){
//				lastbone = curbone;
//				parentbone = curbone->GetParent();
//				if (parentbone){
//					int isfirst = 1;
//
//					//CBone* childbone = curbone;
//					//CBone* childbone = parentbone->GetChild();
//					//while (childbone){
//
//						int axiskind = currigelem.transuv[uvno].axiskind;
//						float rotrad2 = rotrad * currigelem.transuv[uvno].applyrate;
//
//						//float rotrad2;
//						//rotrad2 = rotrad;
//
//
//						//char str1[256];
//						//sprintf_s(str1, 256, "AXIS_KIND %d, rotrad %f, rotrad2 %f", axiskind, rotrad, rotrad2);
//						//::MessageBoxA(NULL, str1, "check", MB_OK);
//
//						if (currigelem.transuv[uvno].enable == 1){
//							if (fabs(rotrad2) >= (0.02f * (float)DEG2PAI)){
//								ChaVector3 axis0;
//								CQuaternion localq;
//								
//								//if (axiskind == AXIS_X){
//								//	axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
//								//	localq.SetAxisAndRot(axis0, rotrad2);
//								//}
//								//else if (axiskind == AXIS_Y){
//								//	axis0 = ChaVector3(0.0f, 1.0f, 0.0f);
//								//	localq.SetAxisAndRot(axis0, rotrad2);
//								//}
//								//else if (axiskind == AXIS_Z){
//								//	axis0 = ChaVector3(0.0f, 0.0f, 1.0f);
//								//	localq.SetAxisAndRot(axis0, rotrad2);
//								//}
//								//else{
//								//	_ASSERT(0);
//								//	return 1;
//								//}
//								
//								//axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
//								///localq.SetAxisAndRot(axis0, 0.0697 * 0.50);
//
//								if ((axiskind == PICK_X) || (axiskind == PICK_SPA_X)) {
//									axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
//								}
//								else if ((axiskind == PICK_Y) || (axiskind == PICK_SPA_Y)) {
//									axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
//								}
//								else if ((axiskind == PICK_Z) || (axiskind == PICK_SPA_Z)) {
//									axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
//								}
//								else {
//									_ASSERT(0);
//									return 1;
//								}
//								ChaVector3Normalize(&axis0, &axis0);
//								localq.SetAxisAndRot(axis0, rotrad2);
//								ChaMatrix transmat;
//								transmat = localq.MakeRotMatX();
//
//								/*
//								ChaMatrix selectmat;
//								ChaMatrix invselectmat;
//								//selectmat = elemaxismat[elemno];
//								int multworld = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!
//								selectmat = childbone->CalcManipulatorMatrix(1, 0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);//curmotinfo!!!
//								ChaMatrixInverse(&invselectmat, NULL, &selectmat);
//
//								ChaMatrix rotinvworld = childbone->GetCurMp().GetInvWorldMat();
//								rotinvworld._41 = 0.0f;
//								rotinvworld._42 = 0.0f;
//								rotinvworld._43 = 0.0f;
//								ChaMatrix rotselect = selectmat;
//								rotselect._41 = 0.0f;
//								rotselect._42 = 0.0f;
//								rotselect._43 = 0.0f;
//								ChaMatrix rotinvselect = invselectmat;
//								rotinvselect._41 = 0.0f;
//								rotinvselect._42 = 0.0f;
//								rotinvselect._43 = 0.0f;
//
//
//								CQuaternion rotq;
//
//
//								ChaMatrix gparrotmat, invgparrotmat;
//								if (parentbone->GetParent()){
//									gparrotmat = parentbone->GetParent()->GetBtMat();
//									ChaMatrixInverse(&invgparrotmat, NULL, &gparrotmat);
//
//									gparrotmat._41 = 0.0f;
//									gparrotmat._42 = 0.0f;
//									gparrotmat._43 = 0.0f;
//
//									invgparrotmat._41 = 0.0f;
//									invgparrotmat._42 = 0.0f;
//									invgparrotmat._43 = 0.0f;
//								}
//								else{
//									ChaMatrixIdentity(&gparrotmat);
//									ChaMatrixIdentity(&invgparrotmat);
//								}
//								ChaMatrix parrotmat, invparrotmat;
//								parrotmat = parentbone->GetBtMat();
//								ChaMatrixInverse(&invparrotmat, NULL, &parrotmat);
//
//								*/
//								ChaMatrix transmat2;
//								//これでは体全体が反対を向いたときに回転方向が反対向きになる。
//								//transmat2 = invgparrotmat * rotq0.MakeRotMatX() * gparrotmat;
//
//								//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//								ChaMatrix transmat = localq.MakeRotMatX();
//								//以下のようにすれば体全体が回転した時にも正常に動く
//								//transmat2 = invgparrotmat * parrotmat * transmat * invparrotmat * gparrotmat;
//								//CMotionPoint transmp;
//								//transmp.CalcQandTra(transmat2, parentbone);
//								//rotq = transmp.GetQ();
//
//								ChaMatrix rotmat2;
//								//rotmat2 = rotq.MakeRotMatX();
//								rotmat2 = transmat;
//
//								CBone* childbone = curbone->GetChild();
//								while (childbone){
//									btTransform curworldtra;
//									CBtObject* setbto = curbone->GetBtObject(childbone);
//									if (setbto){
//										curworldtra = setbto->GetRigidBody()->getWorldTransform();
//										btMatrix3x3 curworldmat = curworldtra.getBasis();
//
//										btMatrix3x3 rotworldmat;
//										rotworldmat.setIdentity();
//										rotworldmat.setValue(
//											rotmat2.data[MATI_11], rotmat2.data[MATI_12], rotmat2.data[MATI_13],
//											rotmat2.data[MATI_21], rotmat2.data[MATI_22], rotmat2.data[MATI_23],
//											rotmat2.data[MATI_31], rotmat2.data[MATI_32], rotmat2.data[MATI_33]);
//
//										btMatrix3x3 setrotmat;
//										setrotmat = curworldmat * rotworldmat;
//
//										curworldtra.setBasis(setrotmat);
//										setbto->GetRigidBody()->getMotionState()->setWorldTransform(curworldtra);
//									}
//									childbone = childbone->GetBrother();
//								}
//
//
//								/*
//								ChaVector3 parworld, chilworld;
//								ChaVector3TransformCoord(&parworld, &(parentbone->GetJointFPos()), &(parentbone->GetBtMat()));
//								ChaVector3TransformCoord(&chilworld, &(childbone->GetJointFPos()), &(parentbone->GetBtMat()));
//
//								ChaMatrix newbtmat;
//								ChaVector3 rotcenter;// = m_childworld;
//								rotcenter = parworld;
//
//								ChaMatrix befrot, aftrot;
//								ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//								ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//								ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
//								newbtmat = parentbone->GetBtMat() * rotmat;// *tramat;
//								//newbtmat = parentbone->GetCurMp().GetBtMat() * rotq.MakeRotMatX();// *tramat;
//
//
//								ChaMatrix firstworld = parentbone->GetStartMat2();
//								ChaMatrix invfirstworld;
//								ChaMatrixInverse(&invfirstworld, NULL, &firstworld);
//
//								ChaMatrix diffworld = invfirstworld * newbtmat;
//								CRigidElem* curre = parentbone->GetRigidElem(childbone);
//								ChaMatrix newrigidmat;
//								if (curre){
//									newrigidmat = curre->GetFirstcapsulemat() * diffworld;
//								}
//								else{
//									::MessageBoxA(NULL, "IKRotateRagdoll : curre NULL error", "error", MB_OK);
//									return -1;
//								}
//
//
//								ChaVector3 newparentpos, newchildpos;
//								ChaVector3 jointfpos;
//								jointfpos = parentbone->GetJointFPos();
//								ChaVector3TransformCoord(&newparentpos, &jointfpos, &newbtmat);
//								jointfpos = childbone->GetJointFPos();
//								ChaVector3TransformCoord(&newchildpos, &jointfpos, &newbtmat);
//
//								ChaVector3 rigidcenter = (newparentpos + newchildpos) * 0.5f;
//
//
//								CQuaternion tmpq;
//								tmpq.RotationMatrix(newrigidmat);
//								btQuaternion btrotq(tmpq.x, tmpq.y, tmpq.z, tmpq.w);
//
//
//								btTransform worldtra;
//								worldtra.setIdentity();
//								worldtra.setRotation(btrotq);
//								worldtra.setOrigin(btVector3(rigidcenter.x, rigidcenter.y, rigidcenter.z));
//
//								CBtObject* setbto = parentbone->GetBtObject(childbone);
//								if (setbto){
//									setbto->GetRigidBody()->getMotionState()->setWorldTransform(worldtra);
//									//setbto->GetRigidBody()->forceActivationState(ACTIVE_TAG);
//									//setbto->GetRigidBody()->setDeactivationTime(30000.0);
//								}
//
//								if (isfirst == 1){
//									parentbone->SetBtMat(newbtmat);
//									//IKボーンはKINEMATICだから。
//									parentbone->GetCurMp().SetWorldMat(newbtmat);
//									isfirst = 0;
//								}
//								*/
//
//							}
//						}
//
//						//childbone = childbone->GetBrother();
//					//}
//				}
//			}
//			else{
//				_ASSERT(0);
//			}
//		}
//	}
//
//	//if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone){
//	//if (g_absikflag && lastbone){
//	//		AdjustBoneTra(erptr, lastbone);
//	//}
//	//}
//
//	if (lastbone){
//		return lastbone->GetBoneNo();
//	}
//	else{
//		return srcboneno;
//	}
//}



//int CModel::PhysicsMV(CEditRange* erptr, int srcboneno, ChaVector3 diffvec)
//{
//
//	CBone* firstbone = m_bonelist[srcboneno];
//	if (!firstbone){
//		_ASSERT(0);
//		return -1;
//	}
//
//
//	ChaVector3 ikaxis = g_camtargetpos - g_camEye;
//	ChaVector3Normalize(&ikaxis, &ikaxis);
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	CBone* curbone = firstbone;
//	SetBefEditMat(erptr, curbone, 0);
//
//
//	ChaVector3 mvvec = diffvec * g_physicsmvrate;
//
//
//	int isfirst = 1;
//
//	PhysicsMVReq(m_topbone, mvvec);
//
//	if (m_topbone){
//		return m_topbone->GetBoneNo();
//	}
//	else{
//		return srcboneno;
//	}
//
//}

//int CModel::WithConstraint(CBone* srcbone)
//{
//	if (srcbone){
//
//		CBone* curbone = srcbone;
//		while (curbone){
//			if (curbone->GetPosConstraint() == 1){
//				return 1;
//			}
//			curbone = curbone->GetParent();
//		}
//	}
//	else{
//		return 0;
//	}
//
//	return 0;
//}


//void CModel::PhysicsMVReq(CBone* srcbone, ChaVector3 mvvec)
//{
//	if (srcbone){
//		CBone* curbone = srcbone;
//		CBone* parentbone = curbone->GetParent();
//
//		ChaMatrix mvmat;
//		ChaMatrixIdentity(&mvmat);
//		ChaMatrixTranslation(&mvmat, mvvec.x, mvvec.y, mvvec.z);
//
//
//		if (parentbone && (WithConstraint(curbone) == 0) && (parentbone->GetExcludeMv() == 0)){
//				
//			CBone* childbone = curbone;
//
//			//childbone->SetBtKinFlag(1);
//			CBtObject* srcbto = parentbone->GetBtObject(childbone);
//			if (srcbto){
//				//DWORD curflag = srcbto->GetRigidBody()->getCollisionFlags();
//				//srcbto->GetRigidBody()->setCollisionFlags(curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
//				//if (srcbto->GetRigidBody()){
//				//	srcbto->GetRigidBody()->setDeactivationTime(0.0);
//				//	//srcbto->GetRigidBody()->setDeactivationTime(0.016 / 4.0);
//				//}
//
//				if (parentbone->GetBtKinFlag() == 1) {
//					ChaMatrix newbtmat;
//					newbtmat = parentbone->GetBtMat() * mvmat;// *tramat;
//
//					btTransform worldtra;
//					srcbto->GetRigidBody()->getMotionState()->getWorldTransform(worldtra);
//					btMatrix3x3 worldmat = worldtra.getBasis();
//					btVector3 worldpos = worldtra.getOrigin();
//
//
//					btTransform setworldtra;
//					setworldtra.setIdentity();
//					setworldtra.setBasis(worldmat);
//					setworldtra.setOrigin(btVector3(worldpos.x() + mvvec.x, worldpos.y() + mvvec.y, worldpos.z() + mvvec.z));
//					if (srcbto) {
//						srcbto->GetRigidBody()->getMotionState()->setWorldTransform(setworldtra);
//					}
//					else {
//						::MessageBoxA(NULL, "IKTraRagdoll : setbto NULL !!!!", "check", MB_OK);
//					}
//				}
//			}
//		}
//
//		if (curbone->GetChild()){
//			PhysicsMVReq(curbone->GetChild(), mvvec);
//		}
//		if (curbone->GetBrother()){
//			PhysicsMVReq(curbone->GetBrother(), mvvec);
//		}
//	}
//}


int CModel::AdjustBoneTra(bool limitdegflag, CEditRange* erptr, CBone* lastpar)
{
	int keynum = erptr->GetKeyNum();
	double startframe = erptr->GetStartFrame();
	double endframe;
	if (g_applyendflag == 1){
		endframe = m_curmotinfo->frameleng - 1.0;
	}
	else{
		endframe = erptr->GetEndFrame();
	}

	if( lastpar && (keynum >= 2) ){
		int keyno = 0;
		double curframe;
		for( curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0 ){
			if( keyno >= 1 ){
				CMotionPoint* pcurmp = 0;
				int curmotid = m_curmotinfo->motid;
				pcurmp = lastpar->GetMotionPoint(curmotid, curframe);
				if(pcurmp){
					ChaVector3 orgpos;
					ChaVector3 tmplastfpos = lastpar->GetJointFPos();
					ChaMatrix tmpbefeditmat = pcurmp->GetBefEditMat();
					ChaVector3TransformCoord( &orgpos, &tmplastfpos, &tmpbefeditmat );

					ChaVector3 newpos;
					ChaMatrix tmpwm = lastpar->GetWorldMat(limitdegflag, curmotid, curframe, pcurmp);
					ChaVector3TransformCoord( &newpos, &tmplastfpos, &tmpwm );

					ChaVector3 diffpos;
					diffpos = orgpos - newpos;

					CEditRange tmper;
					KeyInfo tmpki;
					tmpki.time = curframe;
					list<KeyInfo> tmplist;
					tmplist.push_back( tmpki );
					tmper.SetRange( tmplist, curframe );
					//FKBoneTra( 0, &tmper, lastpar->GetBoneNo(), diffpos );
					FKBoneTra(limitdegflag, 1, &tmper, lastpar->GetBoneNo(), diffpos);//2022/11/07 FKBoneTra内でframeno loopしないように　onlyoneflag = 1
				}
			}
			keyno++;
		}
	}

	return 0;
}

int CModel::RigControl(bool limitdegflag, int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig, int buttonflag)
{

	bool fromiktarget = false;


	if (depthcnt >= 10){
		_ASSERT(0);
		return 0;//!!!!!!!!!!!!!!!!!
	}
	depthcnt++;

	if (!m_curmotinfo){
		return 0;
	}


	//rigからrigを呼ぶので　再入禁止には別手段が必要
	//if (g_underIKRot == true) {
	//	return 0;//2023/01/27　再入禁止でギザギザは無くなるかどうかテスト
	//}

	//g_underIKRot = true;//2023/01/14 parent limited or not


	int calcnum = 3;

	float rotrad = srcdelta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
	//if (fabs(rotrad) < (0.020 * DEG2PAI)){
	if (fabs(rotrad) < (0.020 * DEG2PAI)) {//2023/02/11
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}	


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone){
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	CBone* parentbone = 0;
	CBone* lastbone = 0;



	//int calccnt;
	//for (calccnt = 0; calccnt < calcnum; calccnt++){
		double firstframe = 0.0;

		int elemno;
		for (elemno = 0; elemno < ikcustomrig.elemnum; elemno++){
			RIGELEM currigelem = ikcustomrig.rigelem[elemno];
			if (currigelem.rigrigboneno >= 0){
				//rigのrig
				CBone* rigrigbone = GetBoneByID(currigelem.rigrigboneno);
				if (rigrigbone){
					int rigrigno = currigelem.rigrigno;
					if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)){
						if (currigelem.transuv[uvno].enable == 1){
							CUSTOMRIG rigrig = rigrigbone->GetCustomRig(rigrigno);
							RigControl(limitdegflag, depthcnt, erptr, rigrigbone->GetBoneNo(), 
								uvno, srcdelta * currigelem.transuv[uvno].applyrate, rigrig, buttonflag);
						}
					}
				}
			}
			else{
				//rigelem
				curbone = GetBoneByID(currigelem.boneno);
				if (curbone){
					lastbone = curbone;
					parentbone = curbone->GetParent();

					CBone* aplybone;
					if (parentbone) {
						aplybone = parentbone;
					}
					else {
						aplybone = curbone;
					}


					int axiskind = currigelem.transuv[uvno].axiskind;
					float rotrad2 = rotrad * currigelem.transuv[uvno].applyrate;
					if (currigelem.transuv[uvno].enable == 1){
						//if (fabs(rotrad2) >= (0.020 * DEG2PAI)){
						if (fabs(rotrad2) >= (0.020 * DEG2PAI)) {//2023/02/11

							if (fabs(rotrad2) > (0.0550 * DEG2PAI)) {//2023/02/11
								rotrad2 = 0.0550f * fabs(rotrad2) / rotrad2;
							}

							ChaVector3 axis0;
							CQuaternion localq;
							//if (axiskind == AXIS_X){
							//	axis0 = ChaVector3(1.0f, 0.0f, 0.0f);
							//	localq.SetAxisAndRot(axis0, rotrad2);
							//}
							//else if (axiskind == AXIS_Y){
							//	axis0 = ChaVector3(0.0f, 1.0f, 0.0f);
							//	localq.SetAxisAndRot(axis0, rotrad2);
							//}
							//else if (axiskind == AXIS_Z){
							//	axis0 = ChaVector3(0.0f, 0.0f, 1.0f);
							//	localq.SetAxisAndRot(axis0, rotrad2);
							//}
							//else{
							//	_ASSERT(0);
							//	return 1;
							//}

							ChaMatrix selectmat;
							ChaMatrix invselectmat;
							selectmat.SetIdentity();
							invselectmat.SetIdentity();
							//selectmat = elemaxismat[elemno];
							//int multworld = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!
							//selectmat = curbone->CalcManipulatorMatrix(0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);//curmotinfo!!!
							if (curbone && curbone->GetParent()) {
								curbone->GetParent()->CalcAxisMatX_Manipulator(limitdegflag, g_boneaxis, 0, curbone, &selectmat, 0);
							}
							else {
								selectmat.SetIdentity();
							}
							ChaMatrixInverse(&invselectmat, NULL, &selectmat);
							if (axiskind == AXIS_X) {
								axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
							}
							else if (axiskind == AXIS_Y) {
								axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
							}
							else if (axiskind == AXIS_Z) {
								axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
							}
							else {
								_ASSERT(0);
								//g_underIKRot = false;//2023/01/14 parent limited or not
								return 1;
							}
							ChaVector3Normalize(&axis0, &axis0);
							localq.SetAxisAndRot(axis0, rotrad2);
							//ChaMatrix transmat;
							//transmat = localq.MakeRotMatX();


							////ChaMatrix rotinvworld = curbone->GetCurMp().GetInvWorldMat();
							//ChaMatrix rotinvworld = curbone->GetCurrentLimitedWorldMat();
							//rotinvworld.data[MATI_41] = 0.0f;
							//rotinvworld.data[MATI_42] = 0.0f;
							//rotinvworld.data[MATI_43] = 0.0f;
							//ChaMatrix rotselect = selectmat;
							//rotselect.data[MATI_41] = 0.0f;
							//rotselect.data[MATI_42] = 0.0f;
							//rotselect.data[MATI_43] = 0.0f;
							//ChaMatrix rotinvselect = invselectmat;
							//rotinvselect.data[MATI_41] = 0.0f;
							//rotinvselect.data[MATI_42] = 0.0f;
							//rotinvselect.data[MATI_43] = 0.0f;

							//CQuaternion rotq;
							CQuaternion qForRot;
							CQuaternion qForHipsRot;

							//curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
							// 
							//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
							curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


							//2023/01/23 : Rigの場合は　回転できなくても処理を継続
							////2023/01/22 : topposスライダーの位置のフレーム(３D表示中のフレーム)において　
							////制限角度により　回転出来ない場合は　リターンする
							////if (g_limitdegflag != false) {
							//if ((g_limitdegflag != false) && (g_wallscrapingikflag == 0)) {//2023/01/23
							//	int ismovable = IsMovableRot(m_curmotinfo->motid, applyframe, applyframe, 
							//		localq, curbone, aplybone);
							//	if (ismovable == 0) {
							//		//g_underIKRot = false;//2023/01/14 parent limited or not
							//		if (lastbone) {
							//			return lastbone->GetBoneNo();
							//		}
							//		else {
							//			return srcboneno;
							//		}
							//	}
							//}


							if (keynum >= 2){
								int keyno = 0;
								double curframe;
								for (curframe = startframe; curframe <= endframe; curframe += 1.0){

									if (aplybone && (g_pseudolocalflag == 1)) {
										bool calcaplyflag = true;
										CalcQForRot(limitdegflag, calcaplyflag,
											m_curmotinfo->motid, curframe, applyframe, localq,
											curbone, aplybone,
											&qForRot, &qForHipsRot);
									}
									else{
										qForRot = localq;
										qForHipsRot = localq;
									}

									double changerate;
									//if (curframe <= applyframe){
									//	changerate = 1.0 / (applyframe - startframe + 1);
									//}
									//else{
									//	changerate = 1.0 / (endframe - applyframe + 1);
									//}
									changerate = (double)(*(g_motionbrush_value + (int)curframe));

									bool infooutflag;
									if (curframe == applyframe) {
										infooutflag = true;
									}
									else {
										infooutflag = false;
									}


									if (keyno == 0){
										firstframe = curframe;
									}
									if (g_absikflag == 0){
										if (g_slerpoffflag == 0){
											CQuaternion endq;
											CQuaternion curqForRot;
											CQuaternion curqForHipsRot;
											endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
											qForRot.Slerp2(endq, 1.0 - changerate, &curqForRot);
											qForHipsRot.Slerp2(endq, 1.0 - changerate, &curqForHipsRot);

											curbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
												infooutflag, 0, m_curmotinfo->motid, curframe, 
												curqForRot, curqForHipsRot, fromiktarget);
										}
										else{
											curbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
												infooutflag, 0, m_curmotinfo->motid, curframe, 
												qForRot, qForHipsRot, fromiktarget);
										}
									}
									else{
										if (keyno == 0){
											curbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
												infooutflag, 0, m_curmotinfo->motid, curframe, 
												qForRot, qForHipsRot, fromiktarget);
										}
										else{
											curbone->SetAbsMatReq(limitdegflag, 0, m_curmotinfo->motid, curframe, firstframe);
										}
									}
									keyno++;
								}

							}
							else{
								bool infooutflag = true;
								qForRot = localq;
								qForHipsRot = localq;
								curbone->RotAndTraBoneQReq(limitdegflag, 0, (double)((int)(startframe + 0.0001)),
									infooutflag, 0, m_curmotinfo->motid, m_curmotinfo->curframe, 
									qForRot, qForHipsRot, fromiktarget);
							}
							if (g_applyendflag == 1){
								//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
								if (m_topbone){
									int tolast;
									for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++){
										//(m_bonelist[0])->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
										m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
									}
								}
							}
						}
					}
				}
				else{
					_ASSERT(0);
				}
			}
		}

		//if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone){
		//if (g_absikflag && lastbone){
		//		AdjustBoneTra(erptr, lastbone);
		//}
	//}

	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (lastbone){
		return lastbone->GetBoneNo();
	}
	else{
		return srcboneno;
	}
}

int CModel::RigControlUnderRig(bool limitdegflag, int depthcnt, 
	CEditRange* erptr, int srcboneno, 
	int uvno, float srcdelta, 
	CUSTOMRIG ikcustomrig, int buttonflag)
{

	SetIKTargetVec();


	if (depthcnt >= 10) {
		_ASSERT(0);
		return -1;//!!!!!!!!!!!!!!!!!
	}
	depthcnt++;

	if (!m_curmotinfo) {
		return -1;
	}


	//rigからrigを呼ぶので　再入禁止には別手段が必要
	//if (g_underIKRot == true) {
	//	return 0;//2023/01/27　再入禁止でギザギザは無くなるかどうかテスト
	//}
	//g_underIKRot = true;//2023/01/14 parent limited or not


	//float rotrad = srcdelta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
	float rotrad = srcdelta / 10.0f * (float)PAI / 20.0f * g_physicsmvrate;//2023/03/04
	//if (fabs(rotrad) < (0.020 * DEG2PAI)) {//2023/02/11
	if (fabs(rotrad) < (0.010 * DEG2PAI)) {//2023/03/04
		return 0;
	}
	if (fabs(rotrad) > (0.0550 * DEG2PAI)) {//2023/03/04
		rotrad = 0.0550f * fabs(rotrad) / rotrad;
	}


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone) {
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	CBone* parentbone = 0;
	CBone* lastbone = 0;



	//int calccnt;
	//for (calccnt = 0; calccnt < calcnum; calccnt++){
	double firstframe = 0.0;

	int elemno;
	for (elemno = 0; elemno < ikcustomrig.elemnum; elemno++) {
		RIGELEM currigelem = ikcustomrig.rigelem[elemno];

		//2023/03/04
		//同じ軸で２倍回転しないように　重複処理はスキップ
		if ((uvno == 1) &&
			(currigelem.transuv[0].enable == 1) && (currigelem.transuv[1].enable == 1) &&
			(currigelem.transuv[0].axiskind == currigelem.transuv[1].axiskind)) {
			continue;
		}

		if (currigelem.rigrigboneno >= 0) {
			//rigのrig
			CBone* rigrigbone = GetBoneByID(currigelem.rigrigboneno);
			if (rigrigbone) {
				int rigrigno = currigelem.rigrigno;
				if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)) {
					if (currigelem.transuv[uvno].enable == 1) {
						CUSTOMRIG rigrig = rigrigbone->GetCustomRig(rigrigno);
						RigControlUnderRig(limitdegflag, depthcnt, erptr, rigrigbone->GetBoneNo(),
							uvno, srcdelta * currigelem.transuv[uvno].applyrate, rigrig, buttonflag);
					}
				}
			}
		}
		else {
			//rigelem
			curbone = GetBoneByID(currigelem.boneno);
			if (curbone) {
				lastbone = curbone;
				parentbone = curbone->GetParent();

				CBone* aplybone;
				if (parentbone) {
					aplybone = parentbone;
				}
				else {
					aplybone = curbone;
				}


				int axiskind = currigelem.transuv[uvno].axiskind;
				float rotrad2 = rotrad * currigelem.transuv[uvno].applyrate;
				if (currigelem.transuv[uvno].enable == 1) {

					ChaVector3 axis0;
					CQuaternion localq;

					ChaMatrix selectmat;
					ChaMatrix invselectmat;
					selectmat.SetIdentity();
					invselectmat.SetIdentity();
					if (curbone && curbone->GetParent()) {
						curbone->GetParent()->CalcAxisMatX_Manipulator(limitdegflag, g_boneaxis, 0, curbone, &selectmat, 0);
					}
					else {
						selectmat.SetIdentity();
					}
					ChaMatrixInverse(&invselectmat, NULL, &selectmat);
					if (axiskind == AXIS_X) {
						axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
					}
					else if (axiskind == AXIS_Y) {
						axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
					}
					else if (axiskind == AXIS_Z) {
						axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
					}
					else {
						_ASSERT(0);
						//g_underIKRot = false;//2023/01/14 parent limited or not
						return -1;
					}
					ChaVector3Normalize(&axis0, &axis0);


					//if (fabs(rotrad2) >= (0.020 * DEG2PAI)) {//2023/02/11
					if (fabs(rotrad2) >= (0.010 * DEG2PAI)) {//2023/03/04

						if (fabs(rotrad2) > (0.0550 * DEG2PAI)) {//2023/02/11
							rotrad2 = 0.0550f * fabs(rotrad2) / rotrad2;
						}
						localq.SetAxisAndRot(axis0, rotrad2);

						CQuaternion qForRot;
						CQuaternion qForHipsRot;

						//curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
						// 
						//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
						curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);

						int ismovable2 = 1;
						if (keynum >= 2) {
							int keyno = 0;
							double curframe = applyframe;
							bool keynum1flag = false;
							bool postflag = false;
							bool fromiktarget = false;
							ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
								keyno, 
								curbone, aplybone,
								curframe, startframe, applyframe,
								localq, keynum1flag, postflag, fromiktarget);

						}
						else {
							bool keynum1flag = true;
							bool postflag = false;
							bool fromiktarget = false;
							ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
								0, 
								curbone, aplybone,
								m_curmotinfo->curframe, startframe, applyframe,
								localq, keynum1flag, postflag, fromiktarget);
						}


						//2023/03/04 制限角度に引っ掛かった場合には　やめて　次のジョイントの回転へ
						if ((ismovable2 == 0) && (g_wallscrapingikflag == 0)) {
							continue;
						}

						//curboneのrotqを保存
						IKROTREC currotrec;
						currotrec.rotq = localq;
						currotrec.targetpos = ChaVector3(0.0f, 0.0f, 0.0f);
						currotrec.lessthanthflag = false;
						if (uvno == 0) {
							curbone->AddIKRotRec_U(currotrec);
						}
						else if (uvno == 1) {
							curbone->AddIKRotRec_V(currotrec);
						}
						else {
							_ASSERT(0);
							return -1;
						}
						
						if (g_applyendflag == 1) {
							//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
							if (m_topbone) {
								int tolast;
								for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++) {
									m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
								}
							}
						}
					}
					else {
						//rotqの回転角度が1e-4より小さい場合
						//ウェイトが小さいフレームにおいても　IKTargetが走るように記録する必要がある
						if (fabs(rotrad2) > (0.0550 * DEG2PAI)) {//2023/02/11
							rotrad2 = 0.0550f * fabs(rotrad2) / rotrad2;
						}
						localq.SetAxisAndRot(axis0, rotrad2);

						IKROTREC currotrec;
						currotrec.rotq = localq;
						currotrec.targetpos = ChaVector3(0.0f, 0.0f, 0.0f);
						currotrec.lessthanthflag = true;//!!!!!!!!!!!
						if (uvno == 0) {
							curbone->AddIKRotRec_U(currotrec);
						}
						else if (uvno == 1) {
							curbone->AddIKRotRec_V(currotrec);
						}
						else {
							_ASSERT(0);
							return -1;
						}
					}
				}
			}
			else {
				_ASSERT(0);
			}
		}
	}

	if (lastbone) {
		return lastbone->GetBoneNo();
	}
	else {
		return srcboneno;
	}
}

int CModel::RigControlPostRig(bool limitdegflag, int depthcnt, 
	CEditRange* erptr, int srcboneno, 
	int uvno,
	CUSTOMRIG ikcustomrig, int buttonflag)
{

	if (depthcnt >= 10) {
		_ASSERT(0);
		return 0;//!!!!!!!!!!!!!!!!!
	}
	depthcnt++;

	if (!m_curmotinfo) {
		return 0;
	}


	//rigからrigを呼ぶので　再入禁止には別手段が必要
	//if (g_underIKRot == true) {
	//	return 0;//2023/01/27　再入禁止でギザギザは無くなるかどうかテスト
	//}
	//g_underIKRot = true;//2023/01/14 parent limited or not


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	double roundingstartframe, roundingendframe, roundingapplyframe;
	roundingstartframe = (double)((int)(startframe + 0.0001));
	roundingendframe = (double)((int)(endframe + 0.0001));
	roundingapplyframe = (double)((int)(applyframe + 0.0001));

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone) {
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	CBone* parentbone = 0;
	CBone* lastbone = 0;



	//int calccnt;
	//for (calccnt = 0; calccnt < calcnum; calccnt++){
	double firstframe = 0.0;

	int elemno;
	for (elemno = 0; elemno < ikcustomrig.elemnum; elemno++) {
		RIGELEM currigelem = ikcustomrig.rigelem[elemno];

		//2023/03/04
		//同じ軸で２倍回転しないように　重複処理はスキップ
		if ((uvno == 1) &&
			(currigelem.transuv[0].enable == 1) && (currigelem.transuv[1].enable == 1) &&
			(currigelem.transuv[0].axiskind == currigelem.transuv[1].axiskind)) {
			continue;
		}

		if (currigelem.rigrigboneno >= 0) {
			//rigのrig
			CBone* rigrigbone = GetBoneByID(currigelem.rigrigboneno);
			if (rigrigbone) {
				int rigrigno = currigelem.rigrigno;
				if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)) {
					if (currigelem.transuv[uvno].enable == 1) {
						CUSTOMRIG rigrig = rigrigbone->GetCustomRig(rigrigno);
						RigControlPostRig(limitdegflag, depthcnt, erptr, rigrigbone->GetBoneNo(),
							uvno, rigrig, buttonflag);
					}
				}
			}
		}
		else {
			//rigelem
			curbone = GetBoneByID(currigelem.boneno);
			if (curbone) {
				lastbone = curbone;
				parentbone = curbone->GetParent();

				CBone* aplybone;
				if (parentbone) {
					aplybone = parentbone;
				}
				else {
					aplybone = curbone;
				}

				int rotrecsize;
				if (uvno == 0) {
					rotrecsize = curbone->GetIKRotRecSize_U();
				}
				else if (uvno == 1) {
					rotrecsize = curbone->GetIKRotRecSize_V();
				}
				else {
					_ASSERT(0);
					return 0;
				}
				if (rotrecsize > 0) {
					int rotrecno;
					for (rotrecno = 0; rotrecno < rotrecsize; rotrecno++) {
						IKROTREC currotrec;
						if (uvno == 0) {
							currotrec = curbone->GetIKRotRec_U(rotrecno);
						}
						else if (uvno == 1) {
							currotrec = curbone->GetIKRotRec_V(rotrecno);
						}
						else {
							_ASSERT(0);
							return 0;
						}
						CQuaternion localq = currotrec.rotq;
						bool lessthanthflag = currotrec.lessthanthflag;

						//curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
						// 
						//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
						curbone->SaveSRT(limitdegflag, m_curmotinfo->motid, roundingstartframe);

						//2023/01/23 : Rigの場合は　回転できなくても処理を継続
						
						if (lessthanthflag == false) {
							if (keynum >= 2) {
								int keyno = 0;
								double curframe;
								for (curframe = roundingstartframe; curframe <= roundingendframe; curframe += 1.0) {
									if (curframe != roundingapplyframe) {
										bool keynum1flag = false;
										bool postflag = true;
										bool fromiktarget = false;
										IKRotateOneFrame(limitdegflag, erptr,
											keyno, 
											curbone, aplybone,
											curframe, roundingstartframe, roundingapplyframe,
											localq, keynum1flag, postflag, fromiktarget);
									}
									keyno++;
								}
							}
						}

						if (g_applyendflag == 1) {
							//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
							if (m_topbone) {
								int tolast;
								for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++) {
									m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
								}
							}
						}
					}
				}
			}
			else {
				_ASSERT(0);
			}
		}
	}


	if (uvno == 1) {
		double curframe;
		for (curframe = roundingstartframe; curframe <= roundingendframe; curframe += 1.0) {
			if (curframe != roundingapplyframe) {
				bool postflag = true;
				IKTargetVec(limitdegflag, erptr, curframe, postflag);
			}
		}
	}

	if (lastbone) {
		return lastbone->GetBoneNo();
	}
	else {
		return srcboneno;
	}
}



int CModel::InterpolateBetweenSelection(bool limitdegflag, double srcstartframe, double srcendframe, 
	CBone* interpolatebone, int srckind)
{
	//2023/02/06
	//return operationgJointNo

	int operatingjointno = -1;

	if (!GetCurMotInfo()){
		return operatingjointno;
	}

	if (!interpolatebone) {
		return operatingjointno;
	}

	//CBone* parentone;
	//if (srcbone->GetParent()) {
	//	parentone = srcbone->GetParent();
	//}
	//else {
	//	parentone = srcbone;
	//}

	bool firstbroflag = false;

	if (srckind == 1) {
		//all
		bool oneflag = false;
		InterpolateBetweenSelectionReq(limitdegflag, GetTopBone(), srcstartframe, srcendframe, oneflag, firstbroflag);
		operatingjointno = GetTopBone()->GetBoneNo();
	}
	else if (srckind == 2) {
		//parent one
		bool oneflag = true;
		InterpolateBetweenSelectionReq(limitdegflag, interpolatebone, srcstartframe, srcendframe, oneflag, firstbroflag);
		operatingjointno = interpolatebone->GetBoneNo();
	}
	else if (srckind == 3) {
		//parent deeper
		bool oneflag = false;
		InterpolateBetweenSelectionReq(limitdegflag, interpolatebone, srcstartframe, srcendframe, oneflag, firstbroflag);
		operatingjointno = interpolatebone->GetBoneNo();
	}
	else {
		//unknown
	}


	UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);

	return operatingjointno;
}

void CModel::InterpolateBetweenSelectionReq(bool limitdegflag, CBone* srcbone, 
	double srcstartframe, double srcendframe, bool oneflag, bool broflag)
{
	if (!srcbone){
		return;
	}
	if ((srcstartframe < 0.0) || (srcendframe < 0.0) || (srcendframe <= srcstartframe)){
		return;
	}
	if (!GetCurMotInfo()){
		return;
	}

	double roundingstartframe = (double)((int)(srcstartframe + 0.0001));
	double roundingendframe = (double)((int)(srcendframe + 0.0001));

	if (srcbone){
		int curmotid = GetCurMotInfo()->motid;
		CMotionPoint startmp, endmp;
		srcbone->CalcLocalInfo(limitdegflag, curmotid, roundingstartframe, &startmp);
		srcbone->CalcLocalInfo(limitdegflag, curmotid, roundingendframe, &endmp);
		CQuaternion startq, endq;
		ChaVector3 starttra, endtra;
		startq = startmp.GetQ();
		endq = endmp.GetQ();
		starttra = srcbone->CalcLocalTraAnim(limitdegflag, curmotid, roundingstartframe);
		endtra = srcbone->CalcLocalTraAnim(limitdegflag, curmotid, roundingendframe);

		double frame;
		for (frame = roundingstartframe; frame <= roundingendframe; frame += 1.0){
			CQuaternion setq;
			ChaVector3 settra;
			if (IsTimeEqual(frame, roundingstartframe)){
				setq = startq;
				settra = starttra;
			}
			else if (IsTimeEqual(frame, roundingendframe)){
				setq = endq;
				settra = endtra;
			}
			else{
				double changerate;
				if ((frame >= roundingstartframe) && (frame <= roundingendframe)) {
					changerate = (double)(*(g_motionbrush_value + (int)(frame + 0.1)));
				}
				else {
					changerate = (frame - roundingstartframe) / (roundingendframe - roundingstartframe + 1.0);
				}
				
				startq.Slerp2(endq, changerate, &setq);
				settra = starttra + (endtra - starttra) * changerate;

			}
			int setchildflag1 = 1;
			CQuaternion iniq;
			iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			ChaMatrix befwm = srcbone->GetWorldMat(limitdegflag, curmotid, frame, 0);
			srcbone->SetWorldMatFromQAndTra(limitdegflag, setchildflag1, befwm, iniq, setq, settra, curmotid, frame);
		}


		if (oneflag == false) {
			if (srcbone->GetChild()) {
				bool broflag2 = true;
				InterpolateBetweenSelectionReq(limitdegflag, srcbone->GetChild(), roundingstartframe, roundingendframe, oneflag, broflag2);
			}
			if (srcbone->GetBrother() && broflag) {
				InterpolateBetweenSelectionReq(limitdegflag, srcbone->GetBrother(), roundingstartframe, roundingendframe, oneflag, broflag);
			}
		}
	}
}


////IKRotateAxisDeltaのPICK_Xで行うのでコメントアウト
//int CModel::TwistBoneAxisDelta(CEditRange* erptr, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat)
//{
//	if (!m_curmotinfo) {
//		return 0;
//	}
//
//
//	//int calcnum = 3;
//	int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い
//
//	//float rotrad = delta / 10.0f * (float)PAI / 12.0f;// / (float)calcnum;
//	float rotrad;
//	if (delta > 0.0f) {
//		rotrad = 1.0f * (float)DEG2PAI;
//	}
//	else if (delta < 0.0f) {
//		rotrad = -1.0f * (float)DEG2PAI;
//	}
//	else {
//		return 0;
//	}
//
//	if (fabs(rotrad) < (0.020 * DEG2PAI)) {
//		return 0;
//	}
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
//
//	CBone* curbone = m_bonelist[srcboneno];
//	if (!curbone) {
//		return 0;
//	}
//	CBone* firstbone = curbone;
//	CBone* parentbone = 0;
//	CBone* lastbone = 0;
//	CBone* topbone = GetTopBone();
//	CBone* editboneforret = 0;
//	if (firstbone->GetParent()) {
//		editboneforret = firstbone->GetParent();
//	}
//	else {
//		editboneforret = firstbone;
//	}
//
//	int calccnt;
//	for (calccnt = 0; calccnt < calcnum; calccnt++) {
//		curbone = firstbone;
//		if (!curbone) {
//			return 0;
//		}
//		lastbone = curbone;
//
//		float currate = 1.0f;
//
//		double firstframe = 0.0;
//		int levelcnt = 0;
//
//		while (curbone && ((maxlevel == 0) || (levelcnt < maxlevel))) {
//			parentbone = curbone->GetParent();
//
//			//parentboneの無いcurboneをドラッグした時はbreakしない
//			if (!parentbone && (firstbone != curbone)) {
//				break;
//			}
//
//			float rotrad2 = currate * rotrad;
//			//float rotrad2 = rotrad;
//			if (fabs(rotrad2) < (0.020 * DEG2PAI)) {
//				break;
//			}
//
//			CRigidElem* curre = GetRigidElem(curbone->GetBoneNo());
//			if (curre && curre->GetForbidRotFlag() != 0) {
//				//回転禁止の場合処理をスキップ
//				currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
//				lastbone = curbone;
//				curbone = curbone->GetParent();
//				levelcnt++;
//				continue;
//			}
//
//
//			ChaVector3 axis0;
//			CQuaternion localq;
//			ChaVector3 curfpos, curfpos2;
//			ChaVector3 parfpos, parfpos2;
//			curfpos = curbone->GetJointFPos();
//			parfpos = parentbone->GetJointFPos();
//			ChaMatrix tmpapplywm = curbone->GetWorldMat(m_curmotinfo->motid, applyframe);
//			ChaMatrix tmpparentapplywm = parentbone->GetWorldMat(m_curmotinfo->motid, applyframe);
//			ChaVector3TransformCoord(&curfpos2, &curfpos, &tmpapplywm);
//			ChaVector3TransformCoord(&parfpos2, &parfpos, &tmpparentapplywm);
//			axis0 = curfpos2 - parfpos2;
//			ChaVector3Normalize(&axis0, &axis0);
//			localq.SetAxisAndRot(axis0, rotrad2);
//
//			//ChaMatrix selectmat;
//			ChaMatrix invselectmat;
//			//int multworld = 1;
//			//selectmat = curbone->CalcManipulatorMatrix(0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);//curmotinfo!!!
//			ChaMatrixInverse(&invselectmat, NULL, &selectmat);
//
//
//			ChaMatrix rotinvworld = firstbone->GetCurMp().GetInvWorldMat();
//			rotinvworld.data[MATI_41] = 0.0f;
//			rotinvworld.data[MATI_42] = 0.0f;
//			rotinvworld.data[MATI_43] = 0.0f;
//			ChaMatrix rotselect = selectmat;
//			rotselect.data[MATI_41] = 0.0f;
//			rotselect.data[MATI_42] = 0.0f;
//			rotselect.data[MATI_43] = 0.0f;
//			ChaMatrix rotinvselect = invselectmat;
//			rotinvselect.data[MATI_41] = 0.0f;
//			rotinvselect.data[MATI_42] = 0.0f;
//			rotinvselect.data[MATI_43] = 0.0f;
//
//			CQuaternion rotq;
//
//			if (keynum >= 2) {
//				int keyno = 0;
//				double curframe;
//				for (curframe = startframe; curframe <= endframe; curframe += 1.0) {
//
//					CMotionPoint* curparmp = 0;
//					CMotionPoint* aplyparmp = 0;
//					if (parentbone) {
//						curparmp = parentbone->GetMotionPoint(m_curmotinfo->motid, curframe);
//						aplyparmp = parentbone->GetMotionPoint(m_curmotinfo->motid, applyframe);
//					}
//					else {
//						//parentboneが無いときには、curparmpとapplyparmpはcurrentboneのもの
//						curparmp = curbone->GetMotionPoint(m_curmotinfo->motid, curframe);
//						aplyparmp = curbone->GetMotionPoint(m_curmotinfo->motid, applyframe);
//					}
//					if (curparmp && aplyparmp && (g_pseudolocalflag == 1)) {
//						ChaMatrix curparrotmat = curparmp->GetWorldMat();
//						curparrotmat.data[MATI_41] = 0.0f;
//						curparrotmat.data[MATI_42] = 0.0f;
//						curparrotmat.data[MATI_43] = 0.0f;
//						ChaMatrix invcurparrotmat = curparmp->GetInvWorldMat();
//						invcurparrotmat.data[MATI_41] = 0.0f;
//						invcurparrotmat.data[MATI_42] = 0.0f;
//						invcurparrotmat.data[MATI_43] = 0.0f;
//						ChaMatrix aplyparrotmat = aplyparmp->GetWorldMat();
//						aplyparrotmat.data[MATI_41] = 0.0f;
//						aplyparrotmat.data[MATI_42] = 0.0f;
//						aplyparrotmat.data[MATI_43] = 0.0f;
//						ChaMatrix invaplyparrotmat = aplyparmp->GetInvWorldMat();
//						invaplyparrotmat.data[MATI_41] = 0.0f;
//						invaplyparrotmat.data[MATI_42] = 0.0f;
//						invaplyparrotmat.data[MATI_43] = 0.0f;
//
//						//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//						ChaMatrix transmat = localq.MakeRotMatX();//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//						ChaMatrix transmat2;
//						transmat2 = invcurparrotmat * aplyparrotmat * transmat * invaplyparrotmat * curparrotmat;
//						CMotionPoint transmp;
//						transmp.CalcQandTra(transmat2, firstbone);
//						rotq = transmp.GetQ();
//					}
//					else {
//						//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//						ChaMatrix transmat = localq.MakeRotMatX();//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//						//CMotionPoint transmp;
//						//transmp.CalcQandTra(transmat, firstbone);
//						//rotq = transmp.GetQ();
//						rotq.RotationMatrix(transmat);
//					}
//
//					double changerate;
//					//if (curframe <= applyframe){
//					//	changerate = 1.0 / (applyframe - startframe + 1);
//					//}
//					//else{
//					//	changerate = 1.0 / (endframe - applyframe + 1);
//					//}
//					changerate = (double)(*(g_motionbrush_value + (int)curframe));
//
//					if (keyno == 0) {
//						firstframe = curframe;
//					}
//
//					bool infooutflag;
//					if (curframe == applyframe) {
//						infooutflag = true;
//					}
//					else {
//						infooutflag = false;
//					}
//
//					if (g_absikflag == 0) {
//						if (g_slerpoffflag == 0) {
//							//double currate2;
//							CQuaternion endq;
//							CQuaternion curq;
//							endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//							//if (curframe <= applyframe){
//							//	currate2 = changerate * (curframe - startframe + 1);
//							//}
//							//else{
//							//	currate2 = changerate * (endframe - curframe + 1);
//							//}
//							//rotq.Slerp2(endq, 1.0 - currate2, &curq);
//							rotq.Slerp2(endq, 1.0 - changerate, &curq);
//
//							//_ASSERT(0);
//
//							if (parentbone) {
//								parentbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, curq);
//							}
//							else {
//								curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, curq);
//							}
//						}
//						else {
//							if (parentbone) {
//								parentbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//							}
//							else {
//								curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//							}
//						}
//					}
//					else {
//						if (keyno == 0) {
//							if (parentbone) {
//								parentbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//							}
//							else {
//								curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//							}
//						}
//						else {
//							if (parentbone) {
//								parentbone->SetAbsMatReq(0, m_curmotinfo->motid, curframe, firstframe);
//							}
//							else {
//								curbone->SetAbsMatReq(0, m_curmotinfo->motid, curframe, firstframe);
//							}
//						}
//					}
//					keyno++;
//				}
//
//			}
//			else {
//				//ChaMatrix transmat = rotinvselect * localq.MakeRotMatX() * rotselect;
//				ChaMatrix transmat = localq.MakeRotMatX();//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//				//CMotionPoint transmp;
//				//transmp.CalcQandTra(transmat, firstbone);
//				//rotq = transmp.GetQ();
//				rotq.RotationMatrix(transmat);
//
//				bool infooutflag = true;
//
//				if (parentbone) {
//					parentbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq);
//				}
//				else {
//					curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq);
//				}
//			}
//
//
//			if (g_applyendflag == 1) {
//				//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
//				if (m_topbone) {
//					int tolast;
//					for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++) {
//						//(m_bonelist[0])->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
//						m_topbone->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
//					}
//				}
//			}
//
//			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
//			lastbone = curbone;
//			curbone = curbone->GetParent();
//			levelcnt++;
//		}
//
//		if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone) {
//			//if (g_absikflag && lastbone){
//			AdjustBoneTra(erptr, lastbone);
//		}
//	}
//
//	if (editboneforret) {
//		return editboneforret->GetBoneNo();
//	}
//	else {
//		return srcboneno;
//	}
//
//}

int CModel::CalcQForRot(bool limitdegflag, bool calcaplyflag, 
	int srcmotid, double srcframe, double srcapplyframe, CQuaternion srcaddrot,
	CBone* srcrotbone, CBone* srcaplybone, 
	CQuaternion* dstqForRot, CQuaternion* dstqForHipsRot)
{
	if ((!srcrotbone) || (!srcaplybone) || (!dstqForRot) || (!dstqForHipsRot)) {
		_ASSERT(0);
		return 1;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));
	double roundingapplyframe = (double)((int)(srcapplyframe + 0.0001));

	//ChaMatrix curparrotmat = curparmp->GetWorldMat();
	ChaMatrix curparrotmat = srcrotbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	curparrotmat.data[MATI_41] = 0.0f;
	curparrotmat.data[MATI_42] = 0.0f;
	curparrotmat.data[MATI_43] = 0.0f;
	//ChaMatrix invcurparrotmat = curparmp->GetInvWorldMat();
	ChaMatrix invcurparrotmat = ChaMatrixInv(curparrotmat);
	invcurparrotmat.data[MATI_41] = 0.0f;
	invcurparrotmat.data[MATI_42] = 0.0f;
	invcurparrotmat.data[MATI_43] = 0.0f;

	//ChaMatrix aplyparrotmat = aplyparmp->GetWorldMat();
	ChaMatrix aplyparrotmat = srcaplybone->GetWorldMat(limitdegflag, srcmotid, roundingapplyframe, 0);
	aplyparrotmat.data[MATI_41] = 0.0f;
	aplyparrotmat.data[MATI_42] = 0.0f;
	aplyparrotmat.data[MATI_43] = 0.0f;
	//ChaMatrix invaplyparrotmat = aplyparmp->GetInvWorldMat();
	ChaMatrix invaplyparrotmat = ChaMatrixInv(aplyparrotmat);
	invaplyparrotmat.data[MATI_41] = 0.0f;
	invaplyparrotmat.data[MATI_42] = 0.0f;
	invaplyparrotmat.data[MATI_43] = 0.0f;

	CQuaternion invcurparrotq, aplyparrotq, invaplyparrotq, curparrotq;
	invcurparrotq.RotationMatrix(invcurparrotmat);
	aplyparrotq.RotationMatrix(aplyparrotmat);
	invaplyparrotq.RotationMatrix(invaplyparrotmat);
	curparrotq.RotationMatrix(curparrotmat);


	//意味：RotBoneQReq()にrotqを渡し　currentworldmatの後ろに　invpivot * rotq * pivotを掛ける
	//つまり　A = currentworldmat, B = localq.MakeRotMatX()とすると A * (invA * B * A)
	ChaMatrix transmat2ForRot;
	ChaMatrix transmat2ForHipsRot;

	//hisp移動はうまくいくが　回転がおかしい 　hips以外は良い
	//transmat2 = invcurparrotmat * aplyparrotmat * localq.MakeRotMatX() * invaplyparrotmat * curparrotmat;//bef

	//hips回転はうまくいくが　移動がおかしい
	//transmat2 = localq.MakeRotMatX();//for hips edit

	//####################################################################
	//ToDo : RotQBoneReq2()を作って　引数として上記２つの回転情報を渡す
	//####################################################################

	if (calcaplyflag == true) {
		transmat2ForRot = invcurparrotmat * aplyparrotmat * srcaddrot.MakeRotMatX() * invaplyparrotmat * curparrotmat;//bef
	}
	else {
		transmat2ForRot = invcurparrotmat * srcaddrot.MakeRotMatX() * curparrotmat;//bef
	}
	
	transmat2ForHipsRot = srcaddrot.MakeRotMatX();//for hips edit


	dstqForRot->RotationMatrix(transmat2ForRot);
	dstqForHipsRot->RotationMatrix(transmat2ForHipsRot);


	return 0;
}

int CModel::IsMovableRot(bool limitdegflag, int srcmotid, double srcframe, double srcapplyframe, CQuaternion srcaddrot,
	CBone* srcrotbone, CBone* srcaplybone)
{

	bool fromiktarget = false;

	//#############################################
	//movable : return 1,  not movable : return 0
	//#############################################
	if (!srcrotbone || !srcaplybone) {
		_ASSERT(0);
		return 0;//not movable
	}
	if (limitdegflag == false) {
		return 1;//movable
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));
	double roundingapplyframe = (double)((int)(srcapplyframe + 0.0001));

	CQuaternion qForRot;
	CQuaternion qForHipsRot;
	bool calcaplyflag = true;
	CalcQForRot(limitdegflag, calcaplyflag,
		m_curmotinfo->motid, roundingframe, roundingapplyframe, srcaddrot,
		srcrotbone, srcaplybone,
		&qForRot, &qForHipsRot);

	double changerate;
	changerate = (double)(*(g_motionbrush_value + (int)roundingframe));

	bool infooutflag = true;

	CQuaternion endq;
	CQuaternion curqForRot;
	CQuaternion curqForHipsRot;
	endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	qForRot.Slerp2(endq, 1.0 - changerate, &curqForRot);
	qForHipsRot.Slerp2(endq, 1.0 - changerate, &curqForHipsRot);

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();
	double dummystartframe = 0.0;
	int onlycheckIsMovable = 0;
	srcrotbone->RotAndTraBoneQReq(limitdegflag, &onlycheckIsMovable, dummystartframe,
		infooutflag, 0, srcmotid, roundingframe, 
		curqForRot, curqForHipsRot, fromiktarget);

	return onlycheckIsMovable;
}

int CModel::IKRotateAxisDeltaUnderIK( 
	bool limitdegflag, CEditRange* erptr, 
	int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat)
{
	//####################################################################################################
	//2022/11/04
	//1.0.0.32までのバージョンにおいては　回転軸の計算に苦労した
	//1.0.0.33からは　selectmatの使い方を見直し
	//XYZのどれかの軸で回転する場合においては　selectmatから回転軸がすぐに求まる
	//回転平面がブレない
	//スードローカル動作確認 : 体全体を回転しながら　足をRectブラシで開くと　体が反対を向いても足が開く
	//####################################################################################################

	if (!m_curmotinfo) {
		return 0;
	}

	SetIKTargetVec();

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();


	//int calcnum = 3;
	//int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い

	int calcnum = 1;
	//int calcnum = 4;//ctrlを押しながらドラッグでdelta * 0.25になっている.多フレーム選択時の重さを考えると処理を重くすることは出来ないのでゆっくりドラッグする他ない.
	//float rotrad = delta / 10.0f * (float)PAI / 12.0f * g_physicsmvrate;//PhysicsIKプレートのEditRateスライダーで倍率設定.
	float rotrad = delta / 10.0f * (float)PAI / 20.0f * g_physicsmvrate;//2023/03/04
	//if (fabs(rotrad) < (0.020 * DEG2PAI)) {//2023/02/11
	if (fabs(rotrad) < (0.010 * DEG2PAI)) {//2023/03/04
		return 0;
	}
	if (fabs(rotrad) > (0.0550 * DEG2PAI)) {//2023/02/11
		rotrad = 0.0550f * fabs(rotrad) / rotrad;
	}


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone) {
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	bool ishipsjoint = false;
	if (curbone && curbone->GetParent()) {
		ishipsjoint = curbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	CBone* firstbone = curbone;
	CBone* parentbone = 0;
	CBone* lastbone = 0;
	CBone* topbone = GetTopBone();
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}

	int calccnt;
	for (calccnt = 0; calccnt < calcnum; calccnt++) {
		curbone = firstbone;
		if (!curbone) {
			//g_underIKRot = false;//2023/01/14 parent limited or not
			return 0;
		}
		lastbone = curbone;

		float currate = 1.0f;

		double firstframe = 0.0;
		int levelcnt = 0;

		while (curbone && ((maxlevel == 0) || (levelcnt < maxlevel))) {

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;


			parentbone = curbone->GetParent();

			//parentboneの無いcurboneをドラッグした時はbreakしない
			if (!parentbone && (firstbone != curbone)) {
				break;
			}

			float rotrad2 = currate * rotrad;
			////float rotrad2 = rotrad;
			////if (fabs(rotrad2) < (0.020 * DEG2PAI)){
			if (fabs(rotrad2) < (0.020 * DEG2PAI)) {
				break;
			}
			if (fabs(rotrad2) > (0.0550 * DEG2PAI)) {//2023/02/11
				rotrad2 = 0.0550f * fabs(rotrad2) / rotrad2;
			}

			CRigidElem* curre = GetRigidElem(curbone->GetBoneNo());
			if (curre && curre->GetForbidRotFlag() != 0) {
				//回転禁止の場合処理をスキップ
				currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
				lastbone = curbone;
				curbone = curbone->GetParent();
				levelcnt++;
				continue;
			}

			CBone* aplybone;
			if (parentbone) {
				aplybone = parentbone;
			}
			else {
				aplybone = curbone;
			}

			ChaVector3 axis0;
			CQuaternion localq;
			//2022/11/03 回転軸はselectmatがあれば自明！！
			if ((axiskind == PICK_X) || (axiskind == PICK_SPA_X)) {
				axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
			}
			else if ((axiskind == PICK_Y) || (axiskind == PICK_SPA_Y)) {
				axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
			}
			else if ((axiskind == PICK_Z) || (axiskind == PICK_SPA_Z)) {
				axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
			}
			else {
				_ASSERT(0);
				//g_underIKRot = false;//2023/01/14 parent limited or not
				return 1;
			}
			ChaVector3Normalize(&axis0, &axis0);
			localq.SetAxisAndRot(axis0, rotrad2);
			//ChaMatrix transmat;
			//transmat = localq.MakeRotMatX();


			if (fabs(rotrad2) >= 1e-4) {
				CQuaternion qForRot;
				CQuaternion qForHipsRot;


				//aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
				// 
				//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
				aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


				//2023/01/22 : topposスライダーの位置のフレーム(３D表示中のフレーム)において　
				//制限角度により　回転出来ない場合は　リターンする
				//if (g_limitdegflag != false) {
				if ((limitdegflag != false) && (g_wallscrapingikflag == 0)) {//2023/01/23
					//2023/01/28 IK時は　GetBtForce()チェックはしない　BtForce == 1でも角度制限する
					int ismovable = IsMovableRot(limitdegflag, m_curmotinfo->motid, applyframe, applyframe, localq, aplybone, aplybone);
					if (ismovable == 0) {
						////g_underIKRot = false;//2023/01/14 parent limited or not
						//if (editboneforret) {
						//	return editboneforret->GetBoneNo();
						//}
						//else {
						//	return srcboneno;
						//}


						//2023/02/12 returnやめ　動くボーンは動かすことに
						lastbone = curbone;
						curbone = curbone->GetParent();
						levelcnt++;
						continue;
					}
				}

				int ismovable2 = 1;
				if (keynum >= 2) {
					int keyno = 0;
					double curframe = applyframe;
					//for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {

					bool keynum1flag = false;
					bool postflag = false;
					bool fromiktarget = false;
					ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
						keyno, 
						aplybone, aplybone,
						curframe, startframe, applyframe,
						localq, keynum1flag, postflag, fromiktarget);

					keyno++;
					//}

				}
				else {
					//CMotionPoint transmp;
					//rotq.RotationMatrix(transmat);

					bool keynum1flag = true;
					bool postflag = false;
					bool fromiktarget = false;
					ismovable2 = IKRotateOneFrame(limitdegflag, erptr,
						0, 
						aplybone, aplybone,
						m_curmotinfo->curframe, startframe, applyframe,
						localq, keynum1flag, false, fromiktarget);
				}


				//2023/03/04 制限角度に引っ掛かった場合には　やめて　次のジョイントの回転へ
				if ((ismovable2 == 0) && (g_wallscrapingikflag == 0)) {

					//2023/03/14 continueの前にすべきこと追加
					//RotAndTraBoneQReq内でGetMotionPointがNULLの場合にも　ismovable2 == 0
					lastbone = curbone;
					curbone = curbone->GetParent();
					levelcnt++;
					
					continue;
				}


				if (g_applyendflag == 1) {
					//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
					if (m_topbone) {
						int tolast;
						for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++) {
							//(m_bonelist[0])->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
							m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
						}
					}
				}

				////parentboneのrotqを保存
				IKROTREC currotrec;
				currotrec.rotq = localq;
				currotrec.targetpos = ChaVector3(0.0f, 0.0f, 0.0f);
				currotrec.lessthanthflag = false;
				aplybone->AddIKRotRec(currotrec);
			}
			else {
				//rotqの回転角度が1e-4より小さい場合
				//ウェイトが小さいフレームにおいても　IKTargetが走るように記録する必要がある
				IKROTREC currotrec;
				currotrec.rotq = localq;
				currotrec.targetpos = ChaVector3(0.0f, 0.0f, 0.0f);
				currotrec.lessthanthflag = true;//!!!!!!!!!!!
				aplybone->AddIKRotRec(currotrec);
			}

			if (aplybone) {
				if (aplybone->GetIKStopFlag() == true) {
					break;
				}
			}


			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
			lastbone = curbone;
			curbone = curbone->GetParent();
			levelcnt++;
		}

		//絶対モードの場合
		if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone) {
			AdjustBoneTra(limitdegflag, erptr, lastbone);
		}
	}


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret) {
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}

int CModel::IKRotateAxisDeltaPostIK(
	bool limitdegflag, CEditRange* erptr,
	int axiskind, int srcboneno, int maxlevel, int ikcnt)
{
	if (!m_curmotinfo) {
		return 0;
	}

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();

	int calcnum = 1;
	//int calcnum = 4;//ctrlを押しながらドラッグでdelta * 0.25になっている.多フレーム選択時の重さを考えると処理を重くすることは出来ないのでゆっくりドラッグする他ない.


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone) {
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	bool ishipsjoint = false;
	if (curbone && curbone->GetParent()) {
		ishipsjoint = curbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	CBone* firstbone = curbone;
	CBone* parentbone = 0;
	CBone* lastbone = 0;
	CBone* topbone = GetTopBone();
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}

	int calccnt;
	for (calccnt = 0; calccnt < calcnum; calccnt++) {
		curbone = firstbone;
		if (!curbone) {
			//g_underIKRot = false;//2023/01/14 parent limited or not
			return 0;
		}
		lastbone = curbone;

		float currate = 1.0f;

		double firstframe = 0.0;
		int levelcnt = 0;

		while (curbone && ((maxlevel == 0) || (levelcnt < maxlevel))) {

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;


			parentbone = curbone->GetParent();

			//parentboneの無いcurboneをドラッグした時はbreakしない
			if (!parentbone && (firstbone != curbone)) {
				break;
			}

			CRigidElem* curre = GetRigidElem(curbone->GetBoneNo());
			if (curre && curre->GetForbidRotFlag() != 0) {
				//回転禁止の場合処理をスキップ
				currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
				lastbone = curbone;
				curbone = curbone->GetParent();
				levelcnt++;
				continue;
			}

			CBone* aplybone;
			if (parentbone) {
				aplybone = parentbone;
			}
			else {
				aplybone = curbone;
			}

			int rotrecsize = aplybone->GetIKRotRecSize();
			if (rotrecsize > 0) {
				int rotrecno;
				for (rotrecno = 0; rotrecno < rotrecsize; rotrecno++) {
					IKROTREC currotrec = aplybone->GetIKRotRec(rotrecno);
					CQuaternion localq = currotrec.rotq;
					bool lessthanthflag = currotrec.lessthanthflag;

					//aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
					// 
					//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
					aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);

					if (lessthanthflag == false) {
						if (keynum >= 2) {
							int keyno = 0;
							double curframe;
							for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {
								if (curframe != applyframe) {
									bool keynum1flag = false;
									bool postflag = true;
									bool fromiktarget = false;
									IKRotateOneFrame(limitdegflag, erptr,
										keyno, 
										aplybone, aplybone,
										curframe, startframe, applyframe,
										localq, keynum1flag, postflag, fromiktarget);
								}
								keyno++;
							}
						}
					}
					//else {
					//	//CMotionPoint transmp;
					//	//rotq.RotationMatrix(transmat);

					//	bool keynum1flag = true;
					//	IKRotateOneFrame(limitdegflag, erptr,
					//		0, aplybone,
					//		m_curmotinfo->curframe, startframe, applyframe,
					//		localq, keynum1flag);
					//}

				}
			}
			
			if (aplybone) {
				//check ikstopflag
				if (aplybone->GetIKStopFlag()) {
					break;
				}
			}

			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
			lastbone = curbone;
			curbone = curbone->GetParent();
			levelcnt++;
		}

		if (g_applyendflag == 1) {
			//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
			if (m_topbone) {
				int tolast;
				for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++) {
					//(m_bonelist[0])->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
					m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
				}
			}
		}


		double curframe;
		for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0) {
			if (curframe != applyframe) {
				bool postflag = true;
				IKTargetVec(limitdegflag, erptr, curframe, postflag);
			}
		}


	}

	//絶対モードの場合
	if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone) {
		AdjustBoneTra(limitdegflag, erptr, lastbone);
	}
	


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret) {
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}


int CModel::IKRotateAxisDelta(bool limitdegflag, CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat)
{
	//####################################################################################################
	//2022/11/04
	//1.0.0.32までのバージョンにおいては　回転軸の計算に苦労した
	//1.0.0.33からは　selectmatの使い方を見直し
	//XYZのどれかの軸で回転する場合においては　selectmatから回転軸がすぐに求まる
	//回転平面がブレない
	//スードローカル動作確認 : 体全体を回転しながら　足をRectブラシで開くと　体が反対を向いても足が開く
	//####################################################################################################

	if (!m_curmotinfo){
		return 0;
	}

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();


	//int calcnum = 3;
	//int calcnum = 1;//今はtargetposを細かく刻んでいないので１で良い

	int calcnum = 1;
	//int calcnum = 4;//ctrlを押しながらドラッグでdelta * 0.25になっている.多フレーム選択時の重さを考えると処理を重くすることは出来ないのでゆっくりドラッグする他ない.
	float rotrad = delta / 10.0f * (float)PAI / 12.0f * g_physicsmvrate;//PhysicsIKプレートのEditRateスライダーで倍率設定.
	//if (fabs(rotrad) < (0.020 * DEG2PAI)) {
	if (fabs(rotrad) < (0.020 * DEG2PAI)) {//2023/02/11
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	if (fabs(rotrad) > (0.0550 * DEG2PAI)) {//2023/02/11
		rotrad = 0.0550f * fabs(rotrad) / rotrad;
	}


	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	CBone* curbone = m_bonelist[srcboneno];
	if (!curbone){
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return 0;
	}
	bool ishipsjoint = false;
	if (curbone && curbone->GetParent()) {
		ishipsjoint = curbone->GetParent()->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	CBone* firstbone = curbone;
	CBone* parentbone = 0;
	CBone* lastbone = 0;
	CBone* topbone = GetTopBone();
	CBone* editboneforret = 0;
	if (firstbone->GetParent()) {
		editboneforret = firstbone->GetParent();
	}
	else {
		editboneforret = firstbone;
	}

	int calccnt;
	for (calccnt = 0; calccnt < calcnum; calccnt++){
		curbone = firstbone;
		if (!curbone){
			//g_underIKRot = false;//2023/01/14 parent limited or not
			return 0;
		}
		lastbone = curbone;

		float currate = 1.0f;

		double firstframe = 0.0;
		int levelcnt = 0;

		while (curbone && ((maxlevel == 0) || (levelcnt < maxlevel))){

			//IKTarget()でフラグがリセットされるので　ループ先頭で　セットし直し
			//g_underIKRot = true;


			parentbone = curbone->GetParent();

			//parentboneの無いcurboneをドラッグした時はbreakしない
			if (!parentbone && (firstbone != curbone)) {
				break;
			}

			float rotrad2 = currate * rotrad;
			////float rotrad2 = rotrad;
			////if (fabs(rotrad2) < (0.020 * DEG2PAI)){
			if (fabs(rotrad2) < (0.020 * DEG2PAI)) {
				break;
			}
			if (fabs(rotrad2) > (0.0550 * DEG2PAI)) {//2023/02/11
				rotrad2 = 0.0550f * fabs(rotrad2) / rotrad2;
			}

			CRigidElem* curre = GetRigidElem(curbone->GetBoneNo());
			if (curre && curre->GetForbidRotFlag() != 0) {
				//回転禁止の場合処理をスキップ
				currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
				lastbone = curbone;
				curbone = curbone->GetParent();
				levelcnt++;
				continue;
			}

			CBone* aplybone;
			if (parentbone) {
				aplybone = parentbone;
			}
			else {
				aplybone = curbone;
			}

			ChaVector3 axis0;
			CQuaternion localq;
			//2022/11/03 回転軸はselectmatがあれば自明！！
			if ((axiskind == PICK_X) || (axiskind == PICK_SPA_X)) {
				axis0 = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
			}
			else if ((axiskind == PICK_Y) || (axiskind == PICK_SPA_Y)) {
				axis0 = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
			}
			else if ((axiskind == PICK_Z) || (axiskind == PICK_SPA_Z)) {
				axis0 = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
			}
			else {
				_ASSERT(0);
				//g_underIKRot = false;//2023/01/14 parent limited or not
				return 1;
			}
			ChaVector3Normalize(&axis0, &axis0);
			localq.SetAxisAndRot(axis0, rotrad2);
			//ChaMatrix transmat;
			//transmat = localq.MakeRotMatX();

			CQuaternion qForRot;
			CQuaternion qForHipsRot;


			//aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
			// 
			//保存結果は　CBone::RotAndTraBoneQReqにおいてしか使っておらず　startframeしか使っていない
			aplybone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe);


			//2023/01/22 : topposスライダーの位置のフレーム(３D表示中のフレーム)において　
			//制限角度により　回転出来ない場合は　リターンする
			//if (g_limitdegflag != false) {
			if ((limitdegflag != false) && (g_wallscrapingikflag == 0)) {//2023/01/23
				//2023/01/28 IK時は　GetBtForce()チェックはしない　BtForce == 1でも角度制限する
				int ismovable = IsMovableRot(limitdegflag, m_curmotinfo->motid, applyframe, applyframe, localq, aplybone, aplybone);
				if (ismovable == 0) {
					////g_underIKRot = false;//2023/01/14 parent limited or not
					//if (editboneforret) {
					//	return editboneforret->GetBoneNo();
					//}
					//else {
					//	return srcboneno;
					//}


					//2023/02/12 returnやめ　動くボーンは動かすことに
					lastbone = curbone;
					curbone = curbone->GetParent();
					levelcnt++;
					continue;
				}
			}

			if (keynum >= 2){
				int keyno = 0;
				double curframe;
				for (curframe = (double)((int)(startframe + 0.0001)); curframe <= endframe; curframe += 1.0){

					bool keynum1flag = false;
					bool postflag = false;
					bool fromiktarget = false;
					IKRotateOneFrame(limitdegflag, erptr,
						keyno, 
						aplybone, aplybone,
						curframe, startframe, applyframe,
						localq, keynum1flag, postflag, fromiktarget);

					keyno++;
				}

			}
			else{
				//CMotionPoint transmp;
				//rotq.RotationMatrix(transmat);

				bool keynum1flag = true;
				bool postflag = false;
				bool fromiktarget = false;
				IKRotateOneFrame(limitdegflag, erptr,
					0, 
					aplybone, aplybone,
					m_curmotinfo->curframe, startframe, applyframe,
					localq, keynum1flag, postflag, fromiktarget);
			}


			if (g_applyendflag == 1){
				//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
				if (m_topbone){
					int tolast;
					for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++){
						//(m_bonelist[0])->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
						m_topbone->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
					}
				}
			}

			currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
			lastbone = curbone;
			curbone = curbone->GetParent();
			levelcnt++;
		}

		//絶対モードの場合
		if ((calccnt == (calcnum - 1)) && g_absikflag && lastbone){
			AdjustBoneTra(limitdegflag, erptr, lastbone);
		}
	}


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret){
		return editboneforret->GetBoneNo();
	}
	else{
		return srcboneno;
	}

}

//int CModel::RotateXDelta( CEditRange* erptr, int srcboneno, float delta )
//{
//	if (!m_curmotinfo){
//		return 0;
//	}
//
//	CBone* firstbone = m_bonelist[ srcboneno ];
//	if( !firstbone ){
//		_ASSERT( 0 );
//		return 1;
//	}
//
//	CBone* curbone = firstbone;
//	SetBefEditMatFK( erptr, curbone );
//
//	CBone* lastpar = firstbone->GetParent();
//
//
//	float rotrad;
//	ChaVector3 axis0, rotaxis;
//	ChaMatrix selectmat;
//	ChaMatrix invselectmat;
//	selectmat.SetIdentity();
//	invselectmat.SetIdentity();
//	CBone* parentbone = curbone->GetParent();
//	//int multworld = 1;
//	//selectmat = curbone->CalcManipulatorMatrix(0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);
//	if (curbone && curbone->GetParent()) {
//		curbone->GetParent()->CalcAxisMatX_Manipulator(0, curbone, &selectmat, 0);
//	}
//	else {
//		selectmat.SetIdentity();
//	}
//	ChaMatrixInverse(&invselectmat, NULL, &selectmat);
//	selectmat.data[MATI_41] = 0.0f;
//	selectmat.data[MATI_42] = 0.0f;
//	selectmat.data[MATI_43] = 0.0f;
//
//	axis0 = ChaVector3( 1.0f, 0.0f, 0.0f );
//	ChaVector3TransformCoord( &rotaxis, &axis0, &selectmat );
//	ChaVector3Normalize( &rotaxis, &rotaxis );
//	rotrad = delta / 10.0f * (float)PAI / 12.0f;
//
//	if( fabs(rotrad) < (0.020 * DEG2PAI) ){
//		return 0;
//	}
//
//	CQuaternion rotq;
//	rotq.SetAxisAndRot( rotaxis, rotrad );
//
//	int keynum;
//	double startframe, endframe, applyframe;
//	erptr->GetRange( &keynum, &startframe, &endframe, &applyframe );
//	
//	curbone = firstbone;
//	if (!curbone){
//		return 0;
//	}
//
//	double firstframe = 0.0;
//	if (keynum >= 2){
//		int keyno = 0;
//		double curframe;
//		for (curframe = startframe; curframe <= endframe; curframe += 1.0){
//			double changerate;
//			//if (curframe <= applyframe){
//			//	changerate = 1.0 / (applyframe - startframe + 1);
//			//}
//			//else{
//			//	changerate = 1.0 / (endframe - applyframe + 1);
//			//}
//			changerate = (double)(*(g_motionbrush_value + (int)curframe));
//
//			if (keyno == 0){
//				firstframe = curframe;
//			}
//
//			bool infooutflag;
//			if (curframe == applyframe) {
//				infooutflag = true;
//			}
//			else {
//				infooutflag = false;
//			}
//
//			if (g_absikflag == 0){
//				if (g_slerpoffflag == 0){
//					//double currate2;
//					CQuaternion endq;
//					CQuaternion curq;
//					endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//					//if (curframe <= applyframe){
//					//	currate2 = changerate * (curframe - startframe + 1);
//					//}
//					//else{
//					//	currate2 = changerate * (endframe - curframe + 1);
//					//}
//					//rotq.Slerp2(endq, 1.0 - currate2, &curq);
//					rotq.Slerp2(endq, 1.0 - changerate, &curq);
//
//					curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, curq);
//				}
//				else{
//					curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//				}
//			}
//			else{
//				if (keyno == 0){
//					curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, curframe, rotq);
//				}
//				else{
//					curbone->SetAbsMatReq(0, m_curmotinfo->motid, curframe, firstframe);
//				}
//			}
//			keyno++;
//		}
//
//		if (g_applyendflag == 1){
//			//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
//			if (m_topbone){
//				int tolast;
//				for (tolast = (int)m_curmotinfo->curframe + 1; tolast < (int)m_curmotinfo->frameleng; tolast++){
//					m_topbone->PasteRotReq(m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
//				}
//			}
//		}
//
//
//	}
//	else{
//		bool infooutflag = true;
//		curbone->RotBoneQReq(infooutflag, 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq);
//	}
//
//	if( g_absikflag && curbone ){
//		AdjustBoneTra( erptr, curbone );
//	}
//
//	return curbone->GetBoneNo();
//}


//int CModel::FKRotate( double srcframe, int srcboneno, ChaMatrix srcmat )
int CModel::FKRotate(bool limitdegflag, bool onretarget, int reqflag, 
	CBone* bvhbone, int traflag, ChaVector3 traanim, double srcframe, int srcboneno, 
	CQuaternion rotq)//, int setmatflag, ChaMatrix* psetmat)
{

	if( srcboneno < 0 ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone = m_bonelist[ srcboneno ];
	if( !curbone ){
		_ASSERT( 0 );
		return 1;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	bool onaddmotion = true;//for getbychain
	CBone* parentbone = curbone->GetParent();
	CMotionPoint* parmp = 0;
	if (parentbone){
		parmp = parentbone->GetMotionPoint(m_curmotinfo->motid, roundingframe, onaddmotion);
	}

	if (reqflag == 1){
		ChaMatrix dummyparentwm;
		dummyparentwm.SetIdentity();
		bool infooutflag = true;
		curbone->RotBoneQReq(limitdegflag, infooutflag, 0, m_curmotinfo->motid, roundingframe, rotq, dummyparentwm, dummyparentwm,
			bvhbone, traanim);// , setmatflag, psetmat, onretarget);
	}
	else if(bvhbone){
		ChaMatrix setmat = bvhbone->GetTmpMat();
		curbone->RotBoneQOne(limitdegflag, parentbone, parmp, m_curmotinfo->motid, roundingframe, setmat);
	}

	return curbone->GetBoneNo();
}

int CModel::FKBoneTraAxisUnderFK(
	bool limitdegflag, CEditRange* erptr, int srcboneno, int axiskind, float delta, ChaMatrix selectmat)
{
	if ((srcboneno < 0) && !GetTopBone()) {
		return 0;
	}

	CBone* curbone = GetBoneByID(srcboneno);
	if (!curbone) {
		_ASSERT(0);
		return 0;
	}

	SetIKTargetVec();

	ChaVector3 basevec;
	ChaVector3 vecx(1.0f, 0.0f, 0.0f);
	ChaVector3 vecy(0.0f, 1.0f, 0.0f);
	ChaVector3 vecz(0.0f, 0.0f, 1.0f);

	if (axiskind == 0) {
		basevec = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
	}
	else if (axiskind == 1) {
		basevec = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
	}
	else if (axiskind == 2) {
		basevec = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
	}
	else {
		_ASSERT(0);
		basevec = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
	}
	ChaVector3Normalize(&basevec, &basevec);

	ChaVector3 addtra;
	addtra = basevec * delta;

	FKBoneTraUnderFK(limitdegflag, erptr, srcboneno, addtra);

	return 0;
}

int CModel::FKBoneTraAxisPostFK(
	bool limitdegflag, CEditRange* erptr, int srcboneno)
{
	if ((srcboneno < 0) && !GetTopBone()) {
		return 0;
	}

	CBone* curbone = GetBoneByID(srcboneno);
	if (!curbone) {
		_ASSERT(0);
		return 0;
	}

	FKBoneTraPostFK(limitdegflag, erptr, srcboneno);

	return 0;
}


int CModel::FKBoneTraAxis(bool limitdegflag, int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float delta, ChaMatrix selectmat)
{
	if ((srcboneno < 0) && !GetTopBone()){
		return 0;
	}

	CBone* curbone = GetBoneByID(srcboneno);
	if (!curbone){
		_ASSERT(0);
		return 0;
	}

	ChaVector3 basevec;
	ChaVector3 vecx(1.0f, 0.0f, 0.0f);
	ChaVector3 vecy(0.0f, 1.0f, 0.0f);
	ChaVector3 vecz(0.0f, 0.0f, 1.0f);

	if (axiskind == 0){
		basevec = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
	}
	else if (axiskind == 1){
		basevec = ChaVector3(selectmat.data[MATI_21], selectmat.data[MATI_22], selectmat.data[MATI_23]);
	}
	else if (axiskind == 2){
		basevec = ChaVector3(selectmat.data[MATI_31], selectmat.data[MATI_32], selectmat.data[MATI_33]);
	}
	else{
		_ASSERT(0);
		basevec = ChaVector3(selectmat.data[MATI_11], selectmat.data[MATI_12], selectmat.data[MATI_13]);
	}
	ChaVector3Normalize(&basevec, &basevec);

	ChaVector3 addtra;
	addtra = basevec * delta;

	FKBoneTra(limitdegflag, 0, erptr, srcboneno, addtra);

	return 0;
}

int CModel::FKBoneTraUnderFK(
	bool limitdegflag, CEditRange* erptr,
	int srcboneno, ChaVector3 addtra)
{

	if (srcboneno < 0) {
		_ASSERT(0);
		return 1;
	}

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		return 1;
	}

	SetIKTargetVec();

	CBone* curbone = firstbone;
	SetBefEditMatFK(limitdegflag, erptr, curbone);

	CBone* lastpar = firstbone->GetParent();

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	curbone = firstbone;
	double firstframe = 0.0;

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();//Req関数の最初の呼び出し時は　Identityを渡せばよい

	ChaVector3 translation = ChaVector3(0.0f, 0.0f, 0.0f);

	if (keynum >= 2) {
		//float changerate = 1.0f / (float)(endframe - startframe + 1);

		int keyno = 0;
		double curframe = applyframe;
		//for (curframe = startframe; curframe <= endframe; curframe += 1.0) {
			double changerate;
			changerate = (double)(*(g_motionbrush_value + (int)curframe));


			if (keyno == 0) {
				firstframe = curframe;
			}
			if (g_absikflag == 0) {
				if (g_slerpoffflag == 0) {
					translation = addtra * (float)changerate;
					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, translation, dummyparentwm, dummyparentwm);
				}
				else {
					translation = addtra;
					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, translation, dummyparentwm, dummyparentwm);
				}
			}
			else {
				if (keyno == 0) {
					translation = addtra;
					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, translation, dummyparentwm, dummyparentwm);
				}
				//else {
				//	curbone->SetAbsMatReq(limitdegflag, 0, m_curmotinfo->motid, curframe, firstframe);
				//}
			}

			bool postflag = false;
			IKTargetVec(limitdegflag, erptr, curframe, postflag);


			keyno++;

		//}
	}
	else {
		translation = addtra;
		curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, startframe, translation, dummyparentwm, dummyparentwm);

		bool postflag = false;
		IKTargetVec(limitdegflag, erptr, startframe, postflag);

	}

	IKROTREC currotrec;
	currotrec.rotq = CQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
	currotrec.targetpos = translation;
	currotrec.lessthanthflag = false;
	curbone->AddIKRotRec(currotrec);

	return curbone->GetBoneNo();
}

int CModel::FKBoneTraPostFK(
	bool limitdegflag, CEditRange* erptr,
	int srcboneno)
{

	if (srcboneno < 0) {
		_ASSERT(0);
		return 1;
	}

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		return 1;
	}

	CBone* curbone = firstbone;
	SetBefEditMatFK(limitdegflag, erptr, curbone);

	CBone* lastpar = firstbone->GetParent();

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	curbone = firstbone;
	double firstframe = 0.0;

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();//Req関数の最初の呼び出し時は　Identityを渡せばよい

	int rotrecsize = curbone->GetIKRotRecSize();
	if (rotrecsize > 0) {
		int rotrecno;
		for (rotrecno = 0; rotrecno < rotrecsize; rotrecno++) {
			IKROTREC currotrec = curbone->GetIKRotRec(rotrecno);
			ChaVector3 translation = currotrec.targetpos;
			bool lessthanthflag = currotrec.lessthanthflag;

			if (keynum >= 2) {
				int keyno = 0;
				double curframe;
				for (curframe = (double)((int)(startframe + 0.0001)); curframe <= (double)((int)(endframe + 0.0001)); curframe += 1.0) {
					if (curframe != (double)((int)(applyframe + 0.0001))) {
						if (keyno == 0) {
							firstframe = curframe;
						}

						double changerate = (double)(*(g_motionbrush_value + (int)curframe));
						ChaVector3 currenttranslation = translation * changerate;

						curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe,
							currenttranslation, dummyparentwm, dummyparentwm);
					}
					keyno++;
				}
			}
			//else {
			//	translation = addtra;
			//	curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, startframe, translation, dummyparentwm, dummyparentwm);
			//}
		}
	}


	double curframe;
	for (curframe = (double)((int)(startframe + 0.0001)); curframe <= (double)((int)(endframe + 0.0001)); curframe += 1.0) {
		if (curframe != (double)((int)(applyframe + 0.0001))) {
			bool postflag = true;
			IKTargetVec(limitdegflag, erptr, curframe, postflag);
		}
	}


	return curbone->GetBoneNo();
}


//default:onlyoneframe = 0.0. onlyoneflag == 1のとき　onlyoneframeだけを処理
int CModel::FKBoneTra(bool limitdegflag, int onlyoneflag, CEditRange* erptr, 
	int srcboneno, ChaVector3 addtra, double onlyoneframe)
{

	if( srcboneno < 0 ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* firstbone = m_bonelist[ srcboneno ];
	if( !firstbone ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone = firstbone;
	SetBefEditMatFK(limitdegflag, erptr, curbone);

	CBone* lastpar = firstbone->GetParent();

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
	if (onlyoneflag == 0) {
	}
	else {
		startframe = onlyoneframe;
		endframe = onlyoneframe;
	}
	

	curbone = firstbone;
	double firstframe = 0.0;

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();//Req関数の最初の呼び出し時は　Identityを渡せばよい

	if( keynum >= 2 ){
		//float changerate = 1.0f / (float)(endframe - startframe + 1);

		int keyno = 0;
		double curframe;
		for (curframe = startframe; curframe <= endframe; curframe += 1.0){
			double changerate;
			//if( curframe <= applyframe ){
			//	changerate = 1.0 / (applyframe - startframe + 1);
			//}else{
			//	changerate = 1.0 / (endframe - applyframe + 1);
			//}
			changerate = (double)(*(g_motionbrush_value + (int)curframe));


			if( keyno == 0 ){
				firstframe = curframe;
			}
			if( g_absikflag == 0 ){
				if( g_slerpoffflag == 0 ){
					//double currate2;
					//if( curframe <= applyframe ){
					//	currate2 = changerate * (curframe - startframe + 1);
					//}else{
					//	currate2 = changerate * (endframe - curframe + 1);
					//}
					//ChaVector3 curtra;
					//curtra = addtra * (float)currate2;
					ChaVector3 curtra;
					curtra = addtra * (float)changerate;
	
					//currate2 = changerate * keyno;
					//ChaVector3 curtra;
					//curtra = (1.0 - currate2) * addtra;

					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, curtra, dummyparentwm, dummyparentwm);
				}else{
					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, addtra, dummyparentwm, dummyparentwm);
				}
			}else{
				if( keyno == 0 ){
					curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid, curframe, addtra, dummyparentwm, dummyparentwm);
				}else{
					curbone->SetAbsMatReq(limitdegflag, 0, m_curmotinfo->motid, curframe, firstframe);
				}
			}

			bool postflag = false;
			IKTargetVec(limitdegflag, erptr, curframe, postflag);


			keyno++;

		}
	}else{
		curbone->AddBoneTraReq(limitdegflag, 0, m_curmotinfo->motid,  startframe, addtra, dummyparentwm, dummyparentwm);

		bool postflag = false;
		IKTargetVec(limitdegflag, erptr, startframe, postflag);
	}


	return curbone->GetBoneNo();
}


bool CModel::CheckIKTarget()
{
	bool hastarget = false;
	CheckIKTargetReq(GetTopBone(), &hastarget);
	return hastarget;
}
void CModel::CheckIKTargetReq(CBone* srcbone, bool* pfound)
{
	if (srcbone) {

		if (srcbone->GetIKTargetFlag()) {
			if (pfound) {
				*pfound = true;
			}
			return;
		}

		if (srcbone->GetChild()) {
			CheckIKTargetReq(srcbone->GetChild(), pfound);
		}
		if (srcbone->GetBrother()) {
			CheckIKTargetReq(srcbone->GetBrother(), pfound);
		}
	}
}


int CModel::SetIKTargetVec()
{
	m_iktargetbonevec.clear();

	SetIKTargetVecReq(GetTopBone());

	return 0;
}
void CModel::SetIKTargetVecReq(CBone* srcbone)
{
	if (srcbone) {

		if (srcbone->GetIKTargetFlag()) {
			m_iktargetbonevec.push_back(srcbone);
		}


		if (srcbone->GetChild()) {
			SetIKTargetVecReq(srcbone->GetChild());
		}
		if (srcbone->GetBrother()) {
			SetIKTargetVecReq(srcbone->GetBrother());
		}
	}

}

int CModel::RefreshPosConstraint()
{
	SetIKTargetVec();

	std::vector<CBone*>::iterator itrtargetbone;
	for (itrtargetbone = m_iktargetbonevec.begin(); itrtargetbone != m_iktargetbonevec.end(); itrtargetbone++) {
		CBone* srcbone = *itrtargetbone;
		if (srcbone && srcbone->GetParent() && srcbone->GetIKTargetFlag()) {
			srcbone->SetIKTargetFlag(true);
		}
		else {
			_ASSERT(0);
			continue;
		}
	}

	return 0;
}

int CModel::PosConstraintExecuteFromButton(bool limitdegflag, CEditRange* erptr)
{
	if (!erptr) {
		_ASSERT(0);
		return 1;
	}

	if (m_curmotinfo) {
		SetIKTargetVec();
		int keynum;
		double startframe, endframe, applyframe;
		erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
		double curframe;
		for (curframe = (double)((int)startframe + 0.0001); curframe <= (double)((int)endframe + 0.0001); curframe += 1.0) {
			bool postflag = true;
			int calccount;
			for (calccount = 0; calccount < 1; calccount++) {//IKTargetVecでも複数回計算している
				IKTargetVec(limitdegflag, erptr, curframe, postflag);
			}
		}
	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int CModel::IKTargetVec(bool limitdegflag, CEditRange* erptr, double srcframe, bool postflag)
{
	if (!erptr) {
		_ASSERT(0);
		return 1;
	}

	std::vector<CBone*>::iterator itrtargetbone;
	for (itrtargetbone = m_iktargetbonevec.begin(); itrtargetbone != m_iktargetbonevec.end(); itrtargetbone++) {
		CBone* srcbone = *itrtargetbone;
		if (srcbone && srcbone->GetParent() && srcbone->GetIKTargetFlag()) {
			ChaVector3 iktargetpos = srcbone->GetIKTargetPos();//model座標系
			int calccount;
			const int calccountmax = 30;
			for (calccount = 0; calccount < calccountmax; calccount++) {
				int maxlevel = 0;
				IKRotateForIKTarget(limitdegflag, erptr, srcbone->GetBoneNo(), 
					iktargetpos, maxlevel, srcframe, postflag);
			}
		}
	}

	return 0;
}


int CModel::FKBoneScaleAxis(bool limitdegflag, int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float scaleval)
{
	if ((srcboneno < 0) && !GetTopBone()) {
		return 0;
	}

	CBone* curbone = GetBoneByID(srcboneno);
	if (!curbone) {
		_ASSERT(0);
		return 0;
	}

	//ChaVector3 basevec;
	//ChaVector3 vecx(1.0f, 0.0f, 0.0f);
	//ChaVector3 vecy(0.0f, 1.0f, 0.0f);
	//ChaVector3 vecz(0.0f, 0.0f, 1.0f);

	//int multworld = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!
	//ChaMatrix selectmat = curbone->CalcManipulatorMatrix(0, 0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);

	//if (axiskind == 0) {
	//	ChaVector3TransformCoord(&basevec, &vecx, &selectmat);
	//}
	//else if (axiskind == 1) {
	//	ChaVector3TransformCoord(&basevec, &vecy, &selectmat);
	//}
	//else if (axiskind == 2) {
	//	ChaVector3TransformCoord(&basevec, &vecz, &selectmat);
	//}
	//else {
	//	_ASSERT(0);
	//	ChaVector3TransformCoord(&basevec, &vecx, &selectmat);
	//}

	//ChaVector3Normalize(&basevec, &basevec);

	//ChaVector3 addtra;
	//addtra = basevec * delta;


	//scaleはworldmatの先頭に掛ける。マニピュレータ行列変換ではない
	ChaVector3 scalevec;
	if (axiskind == 0) {
		scalevec.x = scaleval;
		scalevec.y = 1.0f;
		scalevec.z = 1.0f;
	}
	else if (axiskind == 1) {
		scalevec.x = 1.0f;
		scalevec.y = scaleval;
		scalevec.z = 1.0f;
	}
	else if (axiskind == 2) {
		scalevec.x = 1.0f;
		scalevec.y = 1.0f;
		scalevec.z = scaleval;
	}
	else {
		_ASSERT(0);
		scalevec.x = scaleval;
		scalevec.y = 1.0f;
		scalevec.z = 1.0f;
	}

	//ChaVector3 basevec = ChaVector3(0.0f, 0.0f, 0.0f);
	//ChaVector3 vecx = ChaVector3(1.0f, 0.0f, 0.0f);
	//ChaVector3 vecy = ChaVector3(0.0f, 1.0f, 0.0f);
	//ChaVector3 vecz = ChaVector3(0.0f, 0.0f, 1.0f);
	//ChaVector3 vec1 = ChaVector3(1.0f, 1.0f, 1.0f);
	//int multworld = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!
	//ChaMatrix selectmat = curbone->CalcManipulatorMatrix(0, 0, multworld, m_curmotinfo->motid, m_curmotinfo->curframe);
	//if (axiskind == 0) {
	//	ChaVector3TransformCoord(&basevec, &vecx, &selectmat);
	//}
	//else if (axiskind == 1) {
	//	ChaVector3TransformCoord(&basevec, &vecy, &selectmat);
	//}
	//else if (axiskind == 2) {
	//	ChaVector3TransformCoord(&basevec, &vecz, &selectmat);
	//}
	//else {
	//	_ASSERT(0);
	//	ChaVector3TransformCoord(&basevec, &vecx, &selectmat);
	//}
	//ChaVector3Normalize(&basevec, &basevec);
	//
	//ChaVector3 scalevec;
	//if (scaleval >= 1.0) {
	//	scalevec = vec1 + basevec * 0.1f;
	//}
	//else {
	//	scalevec = vec1 - basevec * 0.1f;
	//}
	
	//if (fabs(scalevec.x) < 0.0001f) {
	//	scalevec.x = 0.0001f;
	//}
	//if (fabs(scalevec.y) < 0.0001f) {
	//	scalevec.y = 0.0001f;
	//}
	//if (fabs(scalevec.z) < 0.0001f) {
	//	scalevec.z = 0.0001f;
	//}

	FKBoneScale(limitdegflag, 0, erptr, srcboneno, scalevec);

	return 0;
}

int CModel::FKBoneScale(bool limitdegflag, int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 scalevec)
{

	if (srcboneno < 0) {
		_ASSERT(0);
		return 1;
	}

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone) {
		_ASSERT(0);
		return 1;
	}


	//モーションブラシの0から1のウェイトを掛ける準備
	ChaVector3 scalediffvec;
	scalediffvec.x = scalevec.x - 1.0f;
	scalediffvec.y = scalevec.y - 1.0f;
	scalediffvec.z = scalevec.z - 1.0f;



	CBone* curbone = firstbone;
	SetBefEditMatFK(limitdegflag, erptr, curbone);

	CBone* lastpar = firstbone->GetParent();

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	curbone = firstbone;
	double firstframe = 0.0;

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();//Req関数の最初の呼び出し時は　Identityを渡せばよい


	if (keynum >= 2) {
		//float changerate = 1.0f / (float)(endframe - startframe + 1);

		int keyno = 0;
		double curframe;
		for (curframe = startframe; curframe <= endframe; curframe += 1.0) {
			double changerate;
			//if( curframe <= applyframe ){
			//	changerate = 1.0 / (applyframe - startframe + 1);
			//}else{
			//	changerate = 1.0 / (endframe - applyframe + 1);
			//}
			changerate = (double)(*(g_motionbrush_value + (int)(curframe + 0.0001)));


			if (keyno == 0) {
				firstframe = curframe;
			}
			if (g_absikflag == 0) {
				if (g_slerpoffflag == 0) {
					//double currate2;
					//if( curframe <= applyframe ){
					//	currate2 = changerate * (curframe - startframe + 1);
					//}else{
					//	currate2 = changerate * (endframe - curframe + 1);
					//}
					//ChaVector3 curtra;
					//curtra = addtra * (float)currate2;
					ChaVector3 iniscale = ChaVector3(1.0f, 1.0f, 1.0f);
					ChaVector3 curscale;
					curscale = iniscale + (scalediffvec * (float)changerate);

					//currate2 = changerate * keyno;
					//ChaVector3 curtra;
					//curtra = (1.0 - currate2) * addtra;

					curbone->AddBoneScaleReq(limitdegflag, 0, 
						m_curmotinfo->motid, (double)((int)(curframe + 0.0001)), 
						curscale, dummyparentwm, dummyparentwm);
				}
				else {
					curbone->AddBoneScaleReq(limitdegflag, 0, 
						m_curmotinfo->motid, (double)((int)(curframe + 0.0001)), 
						scalevec, dummyparentwm, dummyparentwm);
				}
			}
			else {
				if (keyno == 0) {
					curbone->AddBoneScaleReq(limitdegflag, 0, 
						m_curmotinfo->motid, (double)((int)(curframe + 0.0001)), 
						scalevec, dummyparentwm, dummyparentwm);
				}
				else {
					curbone->SetAbsMatReq(limitdegflag, 0, 
						m_curmotinfo->motid, (double)((int)(curframe + 0.0001)), firstframe);
				}
			}
			keyno++;

		}
	}
	else {
		curbone->AddBoneScaleReq(limitdegflag, 0, 
			m_curmotinfo->motid, (double)((int)(startframe + 0.0001)), 
			scalevec, dummyparentwm, dummyparentwm);
	}


	return curbone->GetBoneNo();
}




/*
int CModel::ImpulseBoneRagdoll(int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 addtra)
{

	if (srcboneno < 0){
		_ASSERT(0);
		return 1;
	}

	CBone* firstbone = m_bonelist[srcboneno];
	if (!firstbone){
		_ASSERT(0);
		return 1;
	}

	CBone* curbone = firstbone;
	SetBefEditMatFK(erptr, curbone);

	CBone* lastpar = firstbone->GetParent();

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);

	curbone = firstbone;
	double firstframe = 0.0;

	ChaVector3 impulse = addtra * g_physicsmvrate;
	//ChaVector3 impulse = ChaVector3(100.0f, 0.0f ,0.0f);

	CBone* parentbone = curbone->GetParent();
	if (parentbone){
		CBtObject* findbto = parentbone->GetBtObject(curbone);
		if (findbto && findbto->GetRigidBody()){
			findbto->GetRigidBody()->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3(0.0f, 0.0f, 0.0f));
		}
	}

	return curbone->GetBoneNo();
}
*/

int CModel::InitUndoMotion( int saveflag )
{
	m_undo_firstflag = true;

	int undono;
	for( undono = 0; undono < UNDOMAX; undono++ ){
		m_undomotion[ undono ].ClearData();
	}

	m_undo_readpoint = 0;
	m_undo_writepoint = 0;

	int savewritepoint = m_undo_writepoint;
	m_undo_writepoint = GetNewUndoID();

	//if( saveflag ){
	//	BRUSHSTATE brushstate;
	//	brushstate.Init();
	//	brushstate.brushmirrorUflag = g_brushmirrorUflag;
	//	brushstate.brushmirrorVflag = g_brushmirrorVflag;
	//	brushstate.ifmirrorVDiv2flag = g_ifmirrorVDiv2flag;
	//	brushstate.limitdegflag = g_limitdegflag;
	//	brushstate.motionbrush_method = g_motionbrush_method;
	//	brushstate.wallscrapingikflag = g_wallscrapingikflag;
	//	brushstate.brushrepeats = g_brushrepeats;
	//	int result = m_undomotion[m_undo_writepoint].SaveUndoMotion( this, -1, -1, 0, 50.0, brushstate );
	//	if (result == 1) {
	//		m_undo_writepoint = savewritepoint;
	//	}
	//	else {
	//		m_undo_firstflag = false;
	//	}
	//}

	return 0;
}

int CModel::SaveUndoMotion(bool limitdegflag, int curboneno, int curbaseno, CEditRange* srcer,
	double srcapplyrate, BRUSHSTATE srcbrushstate, bool allframeflag)
{
	//saveによって次回のundo位置は変わる
	//undoによって次回のsave位置は変わらない


	if( m_bonelist.empty() || !m_curmotinfo ){
		return 0;
	}

	int saveundoid = m_undo_writepoint;
	m_undo_writepoint = GetNewUndoID();

	int result = m_undomotion[m_undo_writepoint].SaveUndoMotion(limitdegflag, this, 
		curboneno, curbaseno, srcer, srcapplyrate, srcbrushstate, allframeflag);
	if (result == 1) {//result == 2はエラーにしない
		_ASSERT(0);

		m_undo_writepoint = saveundoid;
		return 1;
	}
	else {
		//成功した場合
		m_undo_readpoint = m_undo_writepoint;
		m_undo_firstflag = false;
	}



	return 0;
}
int CModel::RollBackUndoMotion(bool limitdegflag, HWND hmainwnd, int redoflag, int* curboneno, int* curbaseno, double* dststartframe, double* dstendframe, double* dstapplyrate, BRUSHSTATE* dstbrushstate)
{
	//saveによって次回のundo位置は変わる
	//undoによって次回のsave位置は変わらない

	if( m_bonelist.empty() || !m_curmotinfo ){
		return 0;
	}

	if (m_undo_firstflag) {
		return 0;
	}


	//2022/11/08
	//アンドゥリドゥの保存バッファは　リングバッファとして使う
	//一番古い保存と一番新しい保存の間を　古い保存へアンドゥする場合には　バッファの最初と最後の境界を越える
	//一番古い保存と一番新しい保存の間を　新しい保存へリドゥする場合にも　バッファの最初と最後の境界を越える
	//しかし　
	//古い保存から新しい保存へはアンドゥ出来ない && 新しい保存よりも古い保存へはリドゥ出来ない
	//
	//アンドゥリドゥは変更部分についてだけ行っている 上記の禁止をしないとすると
	//例えば　一番新しい保存から一番古い保存へとリドゥするとしたら　一番新しい保存から一番古い保存までの間の全てのアンドゥを実行しなければならない　(それをしないことにした)


	int savereadpoint = m_undo_readpoint;
	if( redoflag == 0 ){
		int tmpreadpoint = GetValidUndoID();
		if (tmpreadpoint == m_undo_writepoint) {//2022/11/08
			::MessageBox(hmainwnd, L"最初の保存ポイントよりも昔にはアンドゥ出来ません。", L"can't go to older than the oldest.", MB_OK);
			//returnせずにそのままのm_undo_readpointでRollBackMotionすることにより　ブラシ状態を保つ
		}
		else {
			m_undo_readpoint = tmpreadpoint;
		}
	}else{
		if (m_undo_readpoint == m_undo_writepoint) {//2022/11/08
			::MessageBox(hmainwnd, L"書き出しポイントよりも未来にはリドゥ出来ません。", L"can't go to newer than the newest.", MB_OK);
			//returnせずにそのままのm_undo_readpointでRollBackMotionすることにより　ブラシ状態を保つ
		}
		else {
			m_undo_readpoint = GetValidRedoID();
		}
	}

	if(m_undo_readpoint >= 0 ){
		int result = m_undomotion[m_undo_readpoint].RollBackMotion(limitdegflag, this, 
			curboneno, curbaseno, dststartframe, dstendframe, dstapplyrate, dstbrushstate);
	}

	return 0;
}

int CModel::GetNewUndoID()
{
	//Save用のundoid
	int writepoint = m_undo_writepoint;

	if (m_undo_firstflag == false) {
		writepoint++;
	}
	else {
		writepoint = 0;
	}
	
	if (writepoint >= UNDOMAX) {
		writepoint = 0;
	}

	return writepoint;
}
int CModel::GetValidUndoID()
{
	//undo用のid

	int retid = -1;

	int chkcnt;
	int curid = m_undo_readpoint;
	for( chkcnt = 0; chkcnt < UNDOMAX; chkcnt++ ){
		curid = curid - 1;
		if( curid < 0 ){
			//curid = UNDOMAX - 1;
			curid = m_undo_writepoint;//配列の最後では無く記録の最後
		}

		if( m_undomotion[curid].GetValidFlag() == 1 ){
			retid = curid;
			break;
		}
	}

	return retid;
}
int CModel::GetValidRedoID()
{
	//redo用のid

	int retid = -1;

	int chkcnt;
	int curid = m_undo_readpoint;
	for( chkcnt = 0; chkcnt < UNDOMAX; chkcnt++ ){
		curid = curid + 1;
		if( curid >= UNDOMAX){
			curid = 0;
		}

		if( m_undomotion[curid].GetValidFlag() == 1 ){
			retid = curid;
			break;
		}
	}

	return retid;
}

int CModel::AddBoneMotMark( OWP_Timeline* owpTimeline, int curboneno, int curlineno, double startframe, double endframe, int flag )
{
	if( (curboneno < 0) || (curlineno < 0) ){
		_ASSERT( 0 );
		return 1;
	}
	CBone* curbone = m_bonelist[ curboneno ];
	if( curbone ){
		curbone->AddBoneMotMark( m_curmotinfo->motid, owpTimeline, curlineno, startframe, endframe, flag );
	}

	return 0;
}

float CModel::GetTargetWeight( int motid, double srcframe, double srctimescale, CMQOObject* srcbaseobj, std::string srctargetname )
{
	FbxAnimLayer* curanimlayer = GetAnimLayer( motid );
	if( !curanimlayer ){
		return 0.0f;
	}

	FbxTime lTime;
	lTime.SetSecondDouble( srcframe / srctimescale );

	//m_fbxobj end iterator check必要？
	//return GetFbxTargetWeight( (FbxNode*)m_fbxobj[srcbaseobj].node, (FbxMesh*)m_fbxobj[srcbaseobj].mesh, srctargetname, lTime, curanimlayer, srcbaseobj );
	return 1.0f;
}

float CModel::GetFbxTargetWeight(FbxNode* pbaseNode, FbxMesh* pbaseMesh, std::string targetname, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* baseobj )
{
    int lVertexCount = pbaseMesh->GetControlPointsCount();
	if( lVertexCount != baseobj->GetVertex() ){
		_ASSERT( 0 );
		return 0.0f;
	}

	int lBlendShapeDeformerCount = pbaseMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pbaseMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; lChannelIndex++)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if(lChannel)
			{
				//const char* nameptr = lChannel->GetName();
				int cmp0;
				cmp0 = strcmp(lChannel->GetName(), targetname.c_str() );
				if( cmp0 == 0 ){
					FbxAnimCurve* lFCurve;
					double lWeight = 0.0;
					lFCurve = pbaseMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
					if (lFCurve){
						lWeight = lFCurve->Evaluate(pTime);
					}else{
						lWeight = 0.0;
					}

					return (float)lWeight;
				}
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	return 0.0f;

}

int CModel::SetFirstFrameBonePos(HINFO* phinfo, CBone* srchipsbone)
{
	int motid = m_curmotinfo->motid;
	if (motid < 0){
		_ASSERT(0);
		return 0;
	}
	if (!GetTopBone()){
		_ASSERT(0);
		return 0;
	}

	if (srchipsbone) {
		SetFirstFrameBonePosReq(srchipsbone, motid, phinfo);
	}
	else {
		SetFirstFrameBonePosReq(GetTopBone(), motid, phinfo);
	}
	

	if (phinfo->maxh >= phinfo->minh){
		phinfo->height = phinfo->maxh - phinfo->minh;
	}
	else{
		phinfo->height = 0.0f;
		_ASSERT(0);
	}


	return 0;
}

void CModel::SetFirstFrameBonePosReq(CBone* srcbone, int srcmotid, HINFO* phinfo)
{
	if (srcbone){
		CMotionPoint* curmp = 0;
		double curframe = 0.0;

		bool onaddmotion = true;//for getbychain
		curmp = srcbone->GetMotionPoint(srcmotid, curframe, onaddmotion);
		if (!curmp){
			_ASSERT(0);
		}

		if (curmp){
			//ChaMatrix firstmat = curmp->GetWorldMat();
			//ChaMatrix firstmat = srcbone->GetFirstMat();//2022/07/29

			ChaMatrix firstmat = srcbone->GetNodeMat() * curmp->GetWorldMat();//2023/02/08


			srcbone->CalcFirstFrameBonePos(firstmat);
			ChaVector3 firstpos = srcbone->GetFirstFrameBonePos();
			if (firstpos.y < phinfo->minh){
				phinfo->minh = firstpos.y;
			}
			if (firstpos.y > phinfo->maxh){
				phinfo->maxh = firstpos.y;
			}
		}
	}

	if (srcbone->GetChild()){
		SetFirstFrameBonePosReq(srcbone->GetChild(), srcmotid, phinfo);
	}
	if (srcbone->GetBrother()){
		SetFirstFrameBonePosReq(srcbone->GetBrother(), srcmotid, phinfo);
	}
}

int CModel::RecalcBoneAxisX(CBone* srcbone)
{
	bool limitdegflag = false;//unlimited初期姿勢用

	//2023/02/07
	//この関数に関しては　現在は重要に考えていない
	//昔のものを　とりあえず動くようにしているだけ
	//nodematの編集については　後で考える


	if (GetOldAxisFlagAtLoading() == 1){
		_ASSERT(0);
		return 1;
	}

	if (!srcbone){
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++){
			CBone* curbone = itrbone->second;
			if (curbone){
				ChaMatrix axismat;
				//axismat = curbone->GetFirstAxisMatZ();
				curbone->GetParent()->CalcAxisMatX_Manipulator(limitdegflag, BONEAXIS_CURRENT, 1, curbone, &axismat, 1);//nodemat用？　_Manipulatorを使う
				axismat.data[MATI_41] = curbone->GetJointFPos().x;
				axismat.data[MATI_42] = curbone->GetJointFPos().y;
				axismat.data[MATI_43] = curbone->GetJointFPos().z;
				curbone->SetNodeMat(axismat);
			}
		}
	}
	else{
		ChaMatrix axismat;
		//axismat = srcbone->GetFirstAxisMatZ();
		if (srcbone->GetParent()){
			srcbone->GetParent()->CalcAxisMatX_Manipulator(limitdegflag, BONEAXIS_CURRENT, 1, srcbone, &axismat, 1);//nodemat用？
		}
		else{
			ChaMatrixIdentity(&axismat);
		}
		axismat.data[MATI_41] = srcbone->GetJointFPos().x;
		axismat.data[MATI_42] = srcbone->GetJointFPos().y;
		axismat.data[MATI_43] = srcbone->GetJointFPos().z;
		srcbone->SetNodeMat(axismat);
	}

	return 0;
}

void CModel::CalcBoneEulReq(bool limitdegflag, CBone* curbone, int srcmotid, double srcframe)
{
	if (!curbone){
		return;
	}

	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	int paraxiskind = -1;//2021/11/18
	//int isfirstbone = 0;
	//cureul = curbone->CalcLocalEulXYZ(paraxsiflag, srcmotid, srcframe, BEFEUL_ZERO);
	cureul = curbone->CalcLocalEulXYZ(limitdegflag, paraxiskind, srcmotid, srcframe, BEFEUL_BEFFRAME);
	curbone->SetLocalEul(limitdegflag, srcmotid, srcframe, cureul, 0);

	if (curbone->GetChild()){
		CalcBoneEulReq(limitdegflag, curbone->GetChild(), srcmotid, srcframe);
	}
	if (curbone->GetBrother()){
		CalcBoneEulReq(limitdegflag, curbone->GetBrother(), srcmotid, srcframe);
	}
}


int CModel::CalcBoneEul(bool limitdegflag, int srcmotid)
{
	if (srcmotid >= 0){
		MOTINFO* mi = GetMotInfo(srcmotid);
		if (mi){
			double frame;
			for (frame = 0.0; frame < mi->frameleng; frame += 1.0){
			//for (frame = 1.0; frame < mi->frameleng; frame += 1.0) {//0frameは計算スキップ
				CalcBoneEulReq(limitdegflag, GetTopBone(), mi->motid, frame);
			}
		}
	}
	else{
		map<int, MOTINFO*>::iterator itrmi;
		for (itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++){
			MOTINFO* chkmi = itrmi->second;
			if (chkmi) {
				int motid = chkmi->motid;
				if (motid > 0) {
					CalcBoneEul(limitdegflag, motid);
				}
			}
		}
	}

	return 0;
}

void CModel:: DumpBtObjectReq(CBtObject* srcbto, int srcdepth)
{
	if (!srcbto){
		return;
	}

	if (srcdepth == 0){
		DbgOut(L"\r\n\r\nStart DumpBtObjectReq\r\n");
	}

	if (srcbto){
		int tabno;
		for (tabno = 0; tabno < srcdepth; tabno++){
			DbgOut(L"\t");
		}
		if (srcbto->GetBone() && srcbto->GetEndBone()){
			DbgOut(L"BtObject : %s---%s\r\n",
				srcbto->GetBone()->GetWBoneName(), srcbto->GetEndBone()->GetWBoneName());
		}
		else{
			DbgOut(L"BtObject : (NULL)---(NULL)\r\n");
		}
	}

	int childnum = srcbto->GetChildBtSize();
	int childno;
	for (childno = 0; childno < childnum; childno++){
		CBtObject* childbto = srcbto->GetChildBt(childno);
		if (childbto){
			DumpBtObjectReq(childbto, srcdepth + 1);
		}
	}
}

void CModel::DumpBtConstraintReq(CBtObject* srcbto, int srcdepth)
{
	if (!srcbto){
		return;
	}
	if (srcdepth == 0){
		DbgOut(L"\r\n\r\nStart DumpBtConstraintReq\r\n");
	}

	if (srcbto){
		CONSTRAINTELEM curce;
		int ceno;
		for (ceno = 0; ceno < srcbto->GetConstraintSize(); ceno++){
			curce = srcbto->GetConstraintElem(ceno);
			if (curce.centerbone && curce.childbto && curce.constraint){
				int nullflag = 0;
				if (!srcbto->GetBone() || !srcbto->GetEndBone()){
					int tabno;
					for (tabno = 0; tabno < srcdepth; tabno++){
						DbgOut(L"\t");
					}
					DbgOut(L"Constraint : src NULL Bone\r\n");
					nullflag++;
				}
				if (!curce.childbto->GetBone() || !curce.childbto->GetEndBone()){
					int tabno;
					for (tabno = 0; tabno < srcdepth; tabno++){
						DbgOut(L"\t");
					}
					DbgOut(L"Constraint : child NULL Bone\r\n");
					nullflag++;
				}
				if(nullflag == 0){
					int tabno;
					for (tabno = 0; tabno < srcdepth; tabno++){
						DbgOut(L"\t");
					}
					DbgOut(L"Constraint : (%s---%s) +%s+ (%s---%s)\r\n",
						srcbto->GetBone()->GetWBoneName(), srcbto->GetEndBone()->GetWBoneName(),
						curce.centerbone->GetWBoneName(),
						curce.childbto->GetBone()->GetWBoneName(), curce.childbto->GetEndBone()->GetWBoneName());
				}
			}
		}

		int childnum = srcbto->GetChildBtSize();
		int childno;
		for (childno = 0; childno < childnum; childno++){
			CBtObject* childbto = srcbto->GetChildBt(childno);
			if (childbto){
				DumpBtConstraintReq(childbto, srcdepth + 1);
			}
		}
	}
}

//int CModel::CreatePhysicsPosConstraint(CBone* srcbone)
//{
//	if (!srcbone){
//		return 0;
//	}
//
//	CBone* parentbone = srcbone->GetParent();
//	if (parentbone){
//		CBtObject* curbto = parentbone->GetBtObject(srcbone);
//		if (curbto){
//			curbto->CreatePhysicsPosConstraint();
//			srcbone->SetPosConstraint(1);
//		}
//	}
//
//	return 0;
//}
//
//int CModel::DestroyPhysicsPosConstraint(CBone* srcbone)
//{
//	if (!srcbone){
//		return 0;
//	}
//
//	CBone* parentbone = srcbone->GetParent();
//	if (parentbone){
//		CBtObject* curbto = parentbone->GetBtObject(srcbone);
//		if (curbto){
//			curbto->DestroyPhysicsPosConstraint();
//			srcbone->SetPosConstraint(0);
//		}
//	}
//
//	return 0;
//}
//int CModel::CreatePhysicsPosConstraintAll()
//{
//	if (!m_topbone) {
//		return 0;
//	}
//
//	int forceflag = 1;
//	CreatePhysicsPosConstraintReq(m_topbone, forceflag);
//
//	return 0;
//}
//
//int CModel::DestroyPhysicsPosConstraintAll()
//{
//	if (!m_topbone) {
//		return 0;
//	}
//
//	int forceflag = 1;
//	DestroyPhysicsPosConstraintReq(m_topbone, forceflag);
//
//	return 0;
//}
//int CModel::CreatePhysicsPosConstraintUpper(CBone* srcbone)
//{
//	if (!srcbone) {
//		return 0;
//	}
//
//	CBone* upperbone = srcbone;
//	while (upperbone) {
//		CBone* parentbone = upperbone->GetParent();
//		if (parentbone) {
//			CBtObject* curbto = parentbone->GetBtObject(srcbone);
//			if (curbto) {
//				curbto->CreatePhysicsPosConstraint();
//				srcbone->SetPosConstraint(1);
//			}
//		}
//		upperbone = parentbone;
//	}
//
//	return 0;
//}
//
//int CModel::DestroyPhysicsPosConstraintUpper(CBone* srcbone)
//{
//	if (!srcbone) {
//		return 0;
//	}
//
//	CBone* upperbone = srcbone;
//	while (upperbone) {
//		CBone* parentbone = upperbone->GetParent();
//		if (parentbone) {
//			CBtObject* curbto = parentbone->GetBtObject(srcbone);
//			if (curbto) {
//				curbto->DestroyPhysicsPosConstraint();
//				srcbone->SetPosConstraint(0);
//			}
//		}
//		upperbone = parentbone;
//	}
//
//
//	return 0;
//}
//
//int CModel::CreatePhysicsPosConstraintLower(CBone* srcbone)
//{
//	if (!srcbone) {
//		return 0;
//	}
//
//	int forceflag = 1;
//	CreatePhysicsPosConstraintReq(srcbone, forceflag);
//
//	return 0;
//}
//
//int CModel::DestroyPhysicsPosConstraintLower(CBone* srcbone)
//{
//	if (!srcbone) {
//		return 0;
//	}
//
//	int forceflag = 1;
//	DestroyPhysicsPosConstraintReq(srcbone, forceflag);
//
//
//	return 0;
//}
//
//
////forceflag = 0
//void CModel::CreatePhysicsPosConstraintReq(CBone* srcbone, int forceflag)
//{
//	if (srcbone){
//		if ((forceflag == 1) || (srcbone->GetPosConstraint() == 1)){
//			CreatePhysicsPosConstraint(srcbone);
//		}
//
//		if (srcbone->GetChild()){
//			CreatePhysicsPosConstraintReq(srcbone->GetChild(), forceflag);
//		}
//		if (srcbone->GetBrother()){
//			CreatePhysicsPosConstraintReq(srcbone->GetBrother(), forceflag);
//		}
//	}
//}
//
////forceflag = 0
//void CModel::DestroyPhysicsPosConstraintReq(CBone* srcbone, int forceflag)
//{
//	if (srcbone) {
//		if ((forceflag == 1) || (srcbone->GetPosConstraint() == 0)) {
//			DestroyPhysicsPosConstraint(srcbone);
//		}
//
//		if (srcbone->GetChild()) {
//			DestroyPhysicsPosConstraintReq(srcbone->GetChild(), forceflag);
//		}
//		if (srcbone->GetBrother()) {
//			DestroyPhysicsPosConstraintReq(srcbone->GetBrother(), forceflag);
//		}
//	}
//}

int CModel::SetKinematicTmpLower(CBone* srcbone, bool srcflag)
{
	if (!srcbone) {
		return 1;
	}
	SetKinematicTmpLowerReq(srcbone, srcflag);
	return 0;
}

void CModel::SetKinematicTmpLowerReq(CBone* srcbone, bool srcflag)
{
	if (!srcbone) {
		return;
	}

	srcbone->SetTmpKinematic(srcflag);

	if (srcbone->GetChild()) {
		SetKinematicTmpLowerReq(srcbone->GetChild(), srcflag);
	}
	if (srcbone->GetBrother()) {
		SetKinematicTmpLowerReq(srcbone->GetBrother(), srcflag);
	}
}


//int CModel::Mass0_All(bool setflag)
//{
//	if (!m_topbone) {
//		return 0;
//	}
//	if (setflag == true) {
//		bool forceflag = true;
//		SetMass0Req(m_topbone, forceflag);
//	}
//	else {
//		RestoreMassReq(m_topbone);
//	}
//	return 0;
//}
//int CModel::Mass0_Upper(bool setflag, CBone* srcbone)
//{
//	if (!srcbone) {
//		return 1;
//	}
//	bool forceflag = true;
//	CBone* setbone = srcbone;
//	while (setbone) {
//		if (setflag == true) {
//			SetMass0(setbone);
//		}
//		else {
//			RestoreMass(setbone);
//		}
//		setbone = setbone->GetParent();
//	}
//	return 0;
//}
//int CModel::Mass0_Lower(bool setflag, CBone* srcbone)
//{
//	if (!srcbone) {
//		return 1;
//	}
//	bool forceflag = true;
//	if (setflag == true) {
//		SetMass0Req(srcbone, forceflag);
//	}
//	else {
//		RestoreMassReq(srcbone);
//	}
//	return 0;
//}
//
//int CModel::SetMass0(CBone* srcbone)
//{
//	if (!srcbone){
//		return 0;
//	}
//
//	CBone* parentbone = srcbone->GetParent();
//	if (parentbone){
//		CBtObject* curbto = parentbone->GetBtObject(srcbone);
//		if (curbto){
//			btVector3 localInertia(0, 0, 0);
//			curbto->GetRigidBody()->setMassProps(0.0, localInertia);
//			srcbone->SetMass0(1);
//		}
//	}
//	return 0;
//}
//int CModel::RestoreMass(CBone* srcbone)
//{
//	if (!srcbone){
//		return 0;
//	}
//
//	CBone* parentbone = srcbone->GetParent();
//	if (parentbone){
//		btScalar setmass = 0.0;
//		CRigidElem* curre = parentbone->GetRigidElem(srcbone);
//		if (curre){
//			setmass = curre->GetMass();
//		}
//
//		CBtObject* curbto = parentbone->GetBtObject(srcbone);
//		if (curbto){
//			btVector3 localInertia(0, 0, 0);
//			curbto->GetRigidBody()->setMassProps(setmass, localInertia);
//			srcbone->SetMass0(0);
//		}
//	}
//	return 0;
//
//}
//
//void CModel::SetMass0Req(CBone* srcbone, bool forceflag)
//{
//	if (srcbone){
//		//強制設定or設定の復元
//		if ((forceflag == true) || (srcbone->GetMass0() == 1)){
//			SetMass0(srcbone);
//		}
//
//		if (srcbone->GetChild()){
//			SetMass0Req(srcbone->GetChild(), forceflag);
//		}
//		if (srcbone->GetBrother()){
//			SetMass0Req(srcbone->GetBrother(), forceflag);
//		}
//	}
//}
//
//void CModel::RestoreMassReq(CBone* srcbone)
//{
//	if (srcbone){
//		RestoreMass(srcbone);
//
//		if (srcbone->GetChild()){
//			RestoreMassReq(srcbone->GetChild());
//		}
//		if (srcbone->GetBrother()){
//			RestoreMassReq(srcbone->GetBrother());
//		}
//	}
//}


int CModel::ApplyBtToMotion(bool limitdegflag)
{
	ApplyBtToMotionReq(limitdegflag, m_topbone);

	return 0;
}

void CModel::ApplyBtToMotionReq(bool limitdegflag, CBone* srcbone)
{
	if (!srcbone)
		return;

	if (!m_curmotinfo)
		return;

	if (srcbone->GetParent()){
		ChaMatrix btmat;
		if (srcbone->GetChild()){
			btmat = srcbone->GetBtMat();
		}
		else{
			btmat = srcbone->GetParent()->GetBtMat();
		}

		//ChaMatrix btrotmat;
		//btrotmat = MakeRotMatFromChaMatrix(btmat);
		//ChaMatrix setmat = btrotmat;
		//btrotmat._41 = srcbone->GetParent()->GetJointFPos().x;
		//btrotmat._42 = srcbone->GetParent()->GetJointFPos().y;
		//btrotmat._43 = srcbone->GetParent()->GetJointFPos().z;


		ChaMatrix setmat = btmat;

		bool infooutflag = false;
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		srcbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
			m_curmotinfo->motid, m_curmotinfo->curframe, setmat,
			onlycheck, fromiktarget);

	}

	if (srcbone->GetChild()){
		ApplyBtToMotionReq(limitdegflag, srcbone->GetChild());
	}
	if (srcbone->GetBrother()){
		ApplyBtToMotionReq(limitdegflag, srcbone->GetBrother());
	}

}

CRigidElem* CModel::GetRigidElem(int srcboneno)
{
	CRigidElem* retre = 0;

	//if ((srcboneno >= 0) && (srcreindex >= 0)) {
	if (srcboneno >= 0) {
		CBone* curbone = GetBoneByID(srcboneno);
		if (curbone) {
			CBone* parentbone = curbone->GetParent();
			if (parentbone) {
				//char* filename = GetRigidElemInfo(srcreindex).filename;
				//CRigidElem* curre = parentbone->GetRigidElemOfMap(filename, curbone);
				CRigidElem* curre = parentbone->GetRigidElem(curbone);
				if (curre) {
					retre = curre;
				}
				else {
					retre = 0;
				}
			}
			else {
				retre = 0;
			}
		}
		else {
			retre = 0;
		}
	}
	else {
		retre = 0;
	}

	return retre;
}

CRigidElem* CModel::GetRgdRigidElem(int srcrgdindex, int srcboneno)
{
	CRigidElem* retre = 0;

	if ((srcboneno >= 0) && (srcrgdindex >= 0)) {
		CBone* curbone = GetBoneByID(srcboneno);
		if (curbone) {
			CBone* parentbone = curbone->GetParent();
			if (parentbone) {
				char* filename = GetRigidElemInfo(srcrgdindex).filename;
				CRigidElem* curre = parentbone->GetRigidElemOfMap(filename, curbone);
				if (curre) {
					retre = curre;
				}
				else {
					retre = 0;
				}
			}
			else {
				retre = 0;
			}
		}
		else {
			retre = 0;
		}
	}
	else {
		retre = 0;
	}

	return retre;
}

void CModel::EnableRotChildren(CBone* srcbone, bool srcflag)
{
	if (!srcbone) {
		return;
	}

	//カレントボーンも含めて再帰的に設定する。
	EnableRotChildrenReq(srcbone, srcflag);

}


void CModel::EnableRotChildrenReq(CBone* srcbone, bool srcflag)
{
	if (!srcbone) {
		return;
	}

	CRigidElem* curre = GetRigidElem(srcbone->GetBoneNo());
	if (curre) {
		if (srcflag == true) {
			curre->SetForbidRotFlag(0);
		}
		else {
			curre->SetForbidRotFlag(1);
		}
	}

	if (srcbone->GetChild()) {
		EnableRotChildrenReq(srcbone->GetChild(), srcflag);
	}
	if (srcbone->GetBrother()) {
		EnableRotChildrenReq(srcbone->GetBrother(), srcflag);
	}
}

void CModel::DestroyScene()
{
	if (m_pscene) {
		m_pscene->Destroy(true);
		m_pscene = 0;
	}
}


void CModel::PhysIKRec(bool limitdegflag, double srcrectime)
{
	if (srcrectime == 0.0) {
		m_physikrec0.clear();
		m_physikrec.clear();
	}

	if (!GetTopBone()) {
		return;
	}

	PhysIKRecReq(limitdegflag, GetTopBone(), srcrectime);

}

void CModel::PhysIKRecReq(bool limitdegflag, CBone* srcbone, double srcrectime)
{
	if (!srcbone) {
		return;
	}

	MOTINFO* curmi = GetCurMotInfo();
	if (!curmi) {
		return;
	}

	PHYSIKREC currec;
	currec.time = srcrectime;
	currec.pbone = srcbone;
	currec.btmat = srcbone->GetBtMat();

	//currec.btmat = srcbone->GetWorldMat(curmi->motid, curmi->curframe);

	if (srcrectime == 0.0) {
		m_physikrec0.push_back(currec);
	}
	m_physikrec.push_back(currec);
	m_phyikrectime = srcrectime;

	if (srcbone->GetChild()) {
		PhysIKRecReq(limitdegflag, srcbone->GetChild(), srcrectime);
	}
	if (srcbone->GetBrother()) {
		PhysIKRecReq(limitdegflag, srcbone->GetBrother(), srcrectime);
	}
}


void CModel::ApplyPhysIkRec(bool limitdegflag)
{
	if (!GetTopBone()) {
		return;
	}

	if (g_motionbrush_frameleng == 0.0) {
		return;
	}

	double srcmaxtime = m_phyikrectime;

	/*
		g_motionbrush_startframe = startframe;
		g_motionbrush_endframe = endframe;
		g_motionbrush_numframe = endframe - startframe + 1;
		g_motionbrush_frameleng = frameleng;
	*/
	if (g_btsimurecflag == false) {
		//Physics IK

		double curframe;
		for (curframe = g_motionbrush_startframe; curframe <= g_motionbrush_endframe; curframe += 1.0) {
			double currectime;
			if (curframe == g_motionbrush_applyframe) {
				currectime = srcmaxtime - 1.0;
			}
			else if (curframe < g_motionbrush_applyframe) {
				double rectimestep;
				rectimestep = srcmaxtime / (g_motionbrush_applyframe - g_motionbrush_startframe + 1);
				currectime = (double)((int)(rectimestep * (curframe - g_motionbrush_startframe)));
				currectime = (double)((int)min(currectime, (m_phyikrectime - 1.0)));
				currectime = (double)((int)max(0.0, currectime));
			}
			else {
				double rectimestep;
				rectimestep = srcmaxtime / (g_motionbrush_endframe - g_motionbrush_applyframe + 1);
				currectime = (double)((int)(m_phyikrectime - rectimestep * (curframe - g_motionbrush_applyframe)));
				currectime = (double)((int)min(currectime, (m_phyikrectime - 1.0)));
				currectime = (double)((int)max(0.0, currectime));
			}

			ApplyPhysIkRecReq(limitdegflag, GetTopBone(), curframe, currectime);
		}
	}
	else {
		//bt simulation

		double curframe;
		for (curframe = g_motionbrush_startframe; curframe <= g_motionbrush_endframe; curframe += 1.0) {
			double currectime;
			double rectimestep;
			rectimestep = srcmaxtime / (g_motionbrush_endframe - g_motionbrush_startframe + 1.0);
			currectime = (double)((int)(rectimestep * (curframe - g_motionbrush_startframe)));
			currectime = (double)((int)min(currectime, (m_phyikrectime - 1.0)));
			currectime = (double)((int)max(0.0, currectime));

			ApplyPhysIkRecReq(limitdegflag, GetTopBone(), curframe, currectime);

		}
	}

	return;
}

void CModel::ApplyPhysIkRecReq(bool limitdegflag, CBone* srcbone, double srcframe, double srcrectime)
{
	if (!srcbone)
		return;

	if (g_motionbrush_numframe == 0) {
		return;
	}

	MOTINFO* curmi = GetCurMotInfo();
	if (!curmi)
		return;

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CBone* btbone = 0;
	//if (srcbone->GetParent()) {
	//	if (srcbone->GetChild()) {
	//		btbone = srcbone;
	//	}
	//	else {
	//		btbone = srcbone->GetParent();
	//	}
	//}
	btbone = srcbone;

	if (btbone) {
		PHYSIKREC recdata0;
		PHYSIKREC recdata;
		bool foundrecdata0 = false;
		bool foundrecdata = false;

		std::vector<PHYSIKREC>::iterator itrfind0;
		for (itrfind0 = m_physikrec0.begin(); itrfind0 != m_physikrec0.end(); itrfind0++) {
			PHYSIKREC curdata = *itrfind0;
			if (curdata.pbone == btbone) {
				recdata0 = curdata;
				foundrecdata0 = true;
				break;
			}
		}

		std::vector<PHYSIKREC>::iterator itrfind;
		for (itrfind = m_physikrec.begin(); itrfind != m_physikrec.end(); itrfind++) {
			PHYSIKREC curdata = *itrfind;
			if (curdata.time == srcrectime) {
				if (curdata.pbone == btbone) {
					recdata = curdata;
					foundrecdata = true;
					break;
				}
			}
		}


		if (foundrecdata0 && foundrecdata) {
			ChaMatrix worldmat0 = btbone->GetWorldMat(limitdegflag, curmi->motid, (double)((int)(g_motionbrush_applyframe + 0.0001)), 0);//or srcframe
			ChaMatrix btmat0 = recdata0.btmat;//カレントボーンのApplyFrameにおけるドラッグ時間ゼロの姿勢
			ChaMatrix btmat = recdata.btmat;//カレントボーンのApplyFrameにおけるドラッグ時間カレントの姿勢

			ChaMatrix curworldmat = btbone->GetWorldMat(limitdegflag, curmi->motid, roundingframe, 0);


			//#######################################################################################
			// quaternionのローカル座標系の式　invAxis * B * Axis は Matrixでは　Axis * B * InvAxis
			// グローバルに戻すときには InvAxis * (Axis * B * InvAxis ) * Axis
			// 
			// DiffにBtを使う理由
			// worldmatにはマウスドラッグ直後の姿勢が入っている
			// その後、物理のループを回して他の剛体に力を伝え、またフィードバックをもらい剛体の接続を保つ
			// 剛体の接続を保っているのはbtmat
			// 
			// worldmatでdiffをとる場合には
			// TopDownとBottomUpを組み合わせてジョイントの接続を保つ
			// しかしそれをリアルタイムでしない場合、結果は記録と異なるかもしれない
			// 
			// 
			// Btmatを使った姿勢式は以下（シミュレーションしてみて多数の候補の中から選定した数式）
			// ChaMatrix setmat = curworldmat * ChaMatrixInv(btmat0) * btmat;
			// 
			// 
			// 解釈その１
			// world * worldDiff
			// world * InvAxis * LocalDIff * Axis
			// world * InvAxis * (Axis * Diff * InvAxis) * Axis
			// world * InvAxis * (Axis * (InvBt0 * Bt) * InvAxis) * Axis
			// world * InvBt0 * Bt
			// 
			// ######################################################################################
			
			//########################################################
			//物理IKの数式（シミュレーションしてみて多数の候補の中から選定した数式）
			//########################################################

			ChaMatrix setmat;

			if (g_btsimurecflag == false) {
				//physic IK

				setmat = curworldmat * ChaMatrixInv(btmat0) * btmat;

				int isinitrot = IsInitRot(setmat);
				if (isinitrot == 1) {
					//解釈その３における特異点　編集効果無し
					setmat = curworldmat;
				}
			}
			else {
				//bt simulation
				setmat = btmat;				
			}


			//ChaMatrix setmat = curworldmat * ChaMatrixInv(worldmat0) * ChaMatrixInv(btmat0) * btmat;//姿勢が変化している場所で試すと一見合っていたが、やはり全体として次元も違うしInvworld0は必要ない


			//２つのうまくいかない数式
			//worldmatを多用するとworldmatは接続を保っていないので形状が分断することがある。
			//それを直すにはボーン構造をTopDownとBottomUp両方で処理する必要があると思われる。しかしその場合記録した結果と異なるかもしれない。
			//ChaMatrix setmat = curworldmat * curworldmat * ChaMatrixInv(btmat0) * btmat * ChaMatrixInv(curworldmat);
			//ChaMatrix setmat = curworldmat * ChaMatrixInv(curworldmat) * ChaMatrixInv(btmat0) * btmat * curworldmat;


			//if (btbone->GetMass0() == 0) {

			bool infooutflag = false;
			bool directsetflag = true;

			//srcbone->SetWorldMat(0, curmi->motid, roundingframe, setmat);
			if (btbone->GetChild()) {
				int onlycheck = 0;
				bool fromiktarget = false;
				btbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
					curmi->motid, roundingframe, setmat,
					onlycheck, fromiktarget);
			}
			else if (btbone->GetParent()) {
				//endjointでparentがある場合
				int onlycheck = 0;
				bool fromiktarget = false;
				ChaMatrix parsetmat = btbone->GetParent()->GetWorldMat(limitdegflag, curmi->motid, roundingframe, 0);//limited????
				btbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
					curmi->motid, roundingframe, parsetmat,
					onlycheck, fromiktarget);
				//btbone->SetWorldMat(1, curmi->motid, roundingframe, parsetmat);
			}

			//CalcBoneEulReq(btbone, curmi->motid, roundingframe);


			//}
		}
	}

	if (srcbone->GetChild()) {
		ApplyPhysIkRecReq(limitdegflag, srcbone->GetChild(), roundingframe, srcrectime);
	}
	if (srcbone->GetBrother()) {
		ApplyPhysIkRecReq(limitdegflag, srcbone->GetBrother(), roundingframe, srcrectime);
	}

}

int CModel::ResetAngleLimit(bool excludebt, int srcval, CBone* srcbone)
{
	if (!srcbone) {
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
			CBone* curbone = itrbone->second;
			if (curbone && 
				((excludebt == false) || ((excludebt == true) && (curbone->GetBtForce() == 0)))) {
				curbone->ResetAngleLimit(srcval);
			}
		}
	}
	else {
		if ((excludebt == false) || ((excludebt == true) && (srcbone->GetBtForce() == 0))) {
			srcbone->ResetAngleLimit(srcval);
		}
	}
	return 0;
}

int CModel::AngleLimitReplace180to170(CBone* srcbone)
{
	if (!srcbone) {
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
			CBone* curbone = itrbone->second;
			if (curbone) {
				curbone->AngleLimitReplace180to170();
			}
		}
	}
	else {
		srcbone->AngleLimitReplace180to170();
	}
	return 0;
}

int CModel::CopyWorldToLimitedWorld()
{
	ChaMatrix tmpwm = GetWorldMat();
	MOTINFO* curmi = GetCurMotInfo();
	if (curmi) {
		double curframe;
		//for (curframe = 0.0; curframe < curmi->frameleng; curframe += 1.0) {
		for (curframe = 1.0; curframe < curmi->frameleng; curframe += 1.0) {
			CopyWorldToLimitedWorldReq(GetTopBone(), curmi->motid, curframe);
		}
	}

	return 0;
}


int CModel::AdditiveCurrentToAngleLimit(CBone* srcbone)
{

	if (!srcbone) {
		map<int, CBone*>::iterator itrbone;
		for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
			CBone* curbone = itrbone->second;
			//if (curbone) {
	//物理シミュ用のボーンの制限角度は上書きしないことにした
	//物理シミュ以外のボーンの制限角度を変えるたびに　何回も物理シミュボーンの設定をやり直すのが非常に手間だから
			if (curbone && (curbone->GetBtForce() == 0)) {
				curbone->AdditiveCurrentToAngleLimit();
			}
		}
	}
	else {
		//物理シミュ用のボーンの制限角度は上書きしないことにした
		//物理シミュ以外のボーンの制限角度を変えるたびに　何回も物理シミュボーンの設定をやり直すのが非常に手間だから
		if (srcbone && (srcbone->GetBtForce() == 0)) {
			srcbone->AdditiveCurrentToAngleLimit();
		}
	}
	return 0;
}

int CModel::AdditiveAllMotionsToAngleLimit()
{
	map<int, CBone*>::iterator itrbone;
	for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		CBone* curbone = itrbone->second;
		//if (curbone) {
	//物理シミュ用のボーンの制限角度は上書きしないことにした
	//物理シミュ以外のボーンの制限角度を変えるたびに　何回も物理シミュボーンの設定をやり直すのが非常に手間だから
		if (curbone && (curbone->GetBtForce() == 0)) {
			curbone->AdditiveAllMotionsToAngleLimit();
		}
	}

	return 0;

}


bool CModel::ChkBoneHasRig(CBone* srcbone)
{
	if (srcbone) {
		int rigno;
		for (rigno = 0; rigno < MAXRIGNUM; rigno++) {
			CUSTOMRIG currig = srcbone->GetCustomRig(rigno);
			if (currig.useflag == 2) {//0: free, 1: allocated, 2: valid
				return true;
			}
		}
	}
	else {
		return false;
	}

	return false;
}

//####################################
// Manager func of CThreadingLoadFbx
//####################################

int CModel::CreateLoadFbxAnim(FbxScene* pscene)
{
	DestroyLoadFbxAnim();
	Sleep(100);


	if (m_bonelist[0] == NULL) {
		_ASSERT(0);
		return 1;
	}


	m_LoadFbxAnim = new CThreadingLoadFbx[LOADFBXANIMTHREAD];
	if (!m_LoadFbxAnim) {
		_ASSERT(0);
		return 1;
	}
	int createno;
	for (createno = 0; createno < LOADFBXANIMTHREAD; createno++) {
		CThreadingLoadFbx* curload = m_LoadFbxAnim + createno;
		curload->SetScene(pscene);
		curload->SetModel(this);
		curload->ClearBoneList();
		curload->CreateThread();
	}

	m_creatednum_loadfbxanim = LOADFBXANIMTHREAD;

	int threadcount = 0;
	int befthreadcount = 0;
	int bonenointhread = 0;
	int bonecount;
	int bonenum = (int)m_bonelist.size();
	int maxbonenuminthread = bonenum / LOADFBXANIMTHREAD + 1;

	for (bonecount = 0; bonecount < bonenum; bonecount++) {
		CBone* curbone = m_bonelist[bonecount];
		if (curbone) {
			FbxNode* curnode = curbone->GetFbxNodeOnLoad();
			if (curnode) {
				CThreadingLoadFbx* curupdate = m_LoadFbxAnim + threadcount;

				curupdate->SetBoneList(bonenointhread, curnode, curbone);

				threadcount = bonecount / maxbonenuminthread;

				if (threadcount == befthreadcount) {
					bonenointhread++;
				}
				else {
					bonenointhread = 0;
				}

				befthreadcount = threadcount;
			}
		}
	}

	return 0;

}

int CModel::DestroyLoadFbxAnim()
{
	if (m_LoadFbxAnim) {
		delete[] m_LoadFbxAnim;
	}
	m_LoadFbxAnim = 0;

	return 0;
}

void CModel::WaitLoadFbxAnimFinished()
{
	if (m_LoadFbxAnim != NULL) {

		bool yetflag = true;
		while (yetflag == true) {
			int finishedcount = 0;
			int loadcount;
			for (loadcount = 0; loadcount < m_creatednum_loadfbxanim; loadcount++) {
				CThreadingLoadFbx* curload = m_LoadFbxAnim + loadcount;
				if (curload->IsFinished()) {
					finishedcount++;
				}
			}

			if (finishedcount == m_creatednum_loadfbxanim) {
				yetflag = false;
				return;
			}
			else {
				//__nop();
				//__nop();
				//__nop();
				//__nop();
			}
			Sleep(0);
		}

	}

}

//########################################
// Manager func of CThreadingUpdateMatrix
//########################################

int CModel::CreateBoneUpdateMatrix()
{

	DestroyBoneUpdateMatrix();
	Sleep(100);


	if (m_bonelist[0] == NULL) {
		_ASSERT(0);
		return 1;
	}


	m_boneupdatematrix = new CThreadingUpdateMatrix[g_UpdateMatrixThreads];
	if (!m_boneupdatematrix) {
		_ASSERT(0);
		return 1;
	}
	int createno;
	for (createno = 0; createno < g_UpdateMatrixThreads; createno++) {
		CThreadingUpdateMatrix* curupdate = m_boneupdatematrix + createno;
		curupdate->ClearBoneList();
		curupdate->CreateThread();
	}



	int threadcount = 0;
	int befthreadcount = 0;
	int bonenointhread = 0;
	int bonecount;
	int bonenum = (int)m_bonelist.size();
	int maxbonenuminthread = bonenum / g_UpdateMatrixThreads + 1;



	for (bonecount = 0; bonecount < bonenum; bonecount++) {
		CBone* curbone = m_bonelist[bonecount];
		if (curbone) {
			CThreadingUpdateMatrix* curupdate = m_boneupdatematrix + threadcount;

			curupdate->SetBoneList(bonenointhread, curbone);

			//threadcount++;
			//threadcount = (threadcount % g_UpdateMatrixThreads);
			//if (threadcount == 0) {
			//	bonenointhread++;
			//}


			threadcount = bonecount / maxbonenuminthread;

			if (threadcount == befthreadcount) {
				bonenointhread++;
			}
			else {
				bonenointhread = 0;
			}

			befthreadcount = threadcount;
		}
	}

	m_creatednum_boneupdatematrix = g_UpdateMatrixThreads;


	return 0;
}

int CModel::DestroyBoneUpdateMatrix()
{

	if (m_boneupdatematrix) {
		delete[] m_boneupdatematrix;
	}
	m_boneupdatematrix = 0;

	return 0;
}

void CModel::WaitUpdateMatrixFinished()
{
	if (m_boneupdatematrix != NULL) {

		bool yetflag = true;
		while (yetflag == true) {
			int finishedcount = 0;
			int updatecount;
			for (updatecount = 0; updatecount < m_creatednum_boneupdatematrix; updatecount++) {
				CThreadingUpdateMatrix* curupdate = m_boneupdatematrix + updatecount;
				if (curupdate->IsFinished()) {
					finishedcount++;
				}
			}

			if (finishedcount == m_creatednum_boneupdatematrix) {
				yetflag = false;
				return;
			}
			else {
				//__nop();
				//__nop();
				//__nop();
				//__nop();
			}
		}

	}

}



void CModel::InitMPReq(bool limitdegflag, CBone* curbone, int srcmotid, double curframe)
{
	if (!curbone) {
		return;
	}

	InitMP(limitdegflag, curbone, srcmotid, curframe);

	if (curbone->GetChild()) {
		InitMPReq(limitdegflag, curbone->GetChild(), srcmotid, curframe);
	}
	if (curbone->GetBrother()) {
		InitMPReq(limitdegflag, curbone->GetBrother(), srcmotid, curframe);
	}
}



int CModel::InitMP(bool limitdegflag, CBone* curbone, int srcmotid, double curframe)
{
	//CMotionPoint* pcurmp = 0;
	//pcurmp = curbone->GetMotionPoint(GetCurMotInfo()->motid, curframe);

	//if (pcurmp) {

	//	//pcurmp->SetBefWorldMat(pcurmp->GetWorldMat());

	//	ChaMatrix xmat = curbone->GetFirstMat();
	//	pcurmp->SetWorldMat(xmat);
	//	curbone->SetInitMat(xmat);

	//}
	//else {
	//	CMotionPoint* curmp3 = 0;
	//	int existflag3 = 0;
	//	curmp3 = curbone->AddMotionPoint(GetCurMotInfo()->motid, curframe, &existflag3);
	//	if (!curmp3) {
	//		_ASSERT(0);
	//		return 1;
	//	}
	//	ChaMatrix xmat = curbone->GetFirstMat();
	//	curmp3->SetWorldMat(xmat);
	//	curbone->SetInitMat(xmat);
	//	//_ASSERT( 0 );
	//}

	////オイラー角初期化
	//ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//int paraxsiflag = 1;
	////int isfirstbone = 0;
	//cureul = curbone->CalcLocalEulXYZ(paraxsiflag, GetCurMotInfo()->motid, curframe, BEFEUL_ZERO);
	//curbone->SetLocalEul(GetCurMotInfo()->motid, curframe, cureul);

	
	if (curbone) {
		curbone->InitMP(limitdegflag, srcmotid, curframe);
	}
	

	return 0;
}


bool CModel::IncludeRootOrReference(FbxNode* ptopnode)
{
	FbxNode* foundnode = 0;
	GetRootOrReferenceReq(ptopnode, &foundnode);
	if (foundnode != 0) {
		return true;
	}
	else {
		return false;
	}
}
void CModel::GetRootOrReferenceReq(FbxNode* srcnode, FbxNode** dstppnode)
{
	if (srcnode && dstppnode && !(*dstppnode)) {

		if ((strstr(srcnode->GetName(), "Root") != 0) || (strstr(srcnode->GetName(), "Reference") != 0)) {
			*dstppnode = srcnode;
			return;
		}

		if (!(*dstppnode)) {
			int childNodeNum;
			childNodeNum = srcnode->GetChildCount();
			for (int i = 0; i < childNodeNum; i++)
			{
				FbxNode* pChild = srcnode->GetChild(i);  // 子ノードを取得
				if (pChild) {
					GetRootOrReferenceReq(pChild, dstppnode);
				}
			}
		}
	}

}

void CModel::GetHipsBoneReq(CBone* srcbone, CBone** dstppbone)
{
	if (srcbone && dstppbone && !(*dstppbone)) {

		if (srcbone->IsHipsBone()) {
			*dstppbone = srcbone;
			return;
		}

		if (!(*dstppbone)) {
			if (srcbone->GetBrother()) {
				GetHipsBoneReq(srcbone->GetBrother(), dstppbone);
			}
			if (srcbone->GetChild()) {
				GetHipsBoneReq(srcbone->GetChild(), dstppbone);
			}
		}
	}
}

void CModel::CalcModelWorldMatOnLoad()
{
	ChaMatrix scalemat;
	scalemat.SetIdentity();
	ChaMatrix rotmat;
	rotmat.SetIdentity();
	rotmat.SetXYZRotation(0, GetModelRotation());
	ChaMatrix tramat;
	tramat.SetIdentity();
	tramat.SetTranslation(GetModelPosition());
	ChaMatrix modelnodemat;
	modelnodemat.SetIdentity();

	ChaMatrix worldmatonload;
	worldmatonload.SetIdentity();
	worldmatonload = ChaMatrixFromSRT(true, true, modelnodemat, &scalemat, &rotmat, &tramat);

	m_worldmat = worldmatonload;
}


