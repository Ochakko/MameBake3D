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

#define DBGH
#include <dbg.h>

#include <DXUT.h>
#include <DXUTcamera.h>
#include <DXUTgui.h>
#include <DXUTsettingsdlg.h>
#include <SDKmisc.h>

#include <DispObj.h>
#include <InfScope.h>
#include <MySprite.h>

#include <MotionPoint.h>
#include <quaternion.h>
#include <VecMath.h>

#include <Collision.h>
#include <EngName.h>

#include <RigidElem.h>

#include <string>

#include <fbxsdk.h>

#include <fbxsdk/scene/shading/fbxlayeredtexture.h>
//#include <fbxsdk/scene/shading/fbxtexture.h>
//#include <fbxsdk/scene/shading/fbxsurfacematerial.h>
//#include <fbxsdk/scene/animation/fbxanimstack.h>
//#include <fbxsdk/scene/animation/fbxanimlayer.h>


#include <BopFile.h>
#include <BtObject.h>

#include <collision.h>
#include <EditRange.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

using namespace OrgWinGUI;

static int s_alloccnt = 0;

extern int g_previewFlag;			// プレビューフラグ
extern WCHAR g_basedir[ MAX_PATH ];
extern ID3DXEffect*	g_pEffect;
extern D3DXHANDLE g_hm3x4Mat;
extern D3DXHANDLE g_hmWorld;
extern float g_impscale;
extern btScalar G_ACC; // 重力加速度 : BPWorld.cpp

extern float g_l_kval[3];
extern float g_a_kval[3];

extern float g_ikfirst;
extern float g_ikrate;
extern int g_slerpoffflag;
extern int g_absikflag;
extern int g_applyendflag;

extern D3DXVECTOR3 g_camEye;
extern D3DXVECTOR3 g_camtargetpos;

int g_dbgflag = 0;


//////////
static FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial, const char * pPropertyName, const char * pFactorPropertyName, char** ppTextureName);
static int IsValidCluster( FbxCluster* pcluster );
static FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition = NULL );
static FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
static FbxAMatrix GetGeometry(FbxNode* pNode);

static int s_setrigidflag = 0;
static DWORD s_rigidflag = 0;

CModel::CModel() : m_newmplist()
{
	InitParams();
	s_alloccnt++;
	m_modelno = s_alloccnt;
}
CModel::~CModel()
{
	DestroyObjs();
}
int CModel::InitParams()
{

	m_ikrotaxis = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	m_texpool = D3DPOOL_DEFAULT;
	m_tmpmotspeed = 1.0f;

	m_curreindex = 0;
	m_rgdindex = 0;
	m_rgdmorphid = -1;

	strcpy_s( m_defaultrename, MAX_PATH, "default_ref.ref" );
	strcpy_s( m_defaultimpname, MAX_PATH, "default_imp.imp" );

	m_rigidbone.clear();
	//m_btg = -1.0f;
	m_btgscale = 9.07f;
	m_fbxobj.clear();
	m_btWorld = 0;

	m_modelno = 0;
	m_pdev = 0;

	ZeroMemory( m_filename, sizeof( WCHAR ) * MAX_PATH );
	ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
	ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	m_loadmult = 1.0f;

	D3DXMatrixIdentity( &m_matWorld );
	D3DXMatrixIdentity( &m_matVP );

	m_curmotinfo = 0;

	m_newmplist.erase( m_newmplist.begin(), m_newmplist.end() );

	m_modeldisp = true;
	m_topbone = 0;
	m_firstbone = 0;

	this->m_tlFunc = 0;

	m_psdk = 0;
	m_pimporter = 0;
	m_pscene = 0;

	m_btcnt = 0;
	m_topbt = 0;

	m_rigideleminfo.clear();
	m_impinfo.clear();

	InitUndoMotion( 0 );

	return 0;
}
int CModel::DestroyObjs()
{
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
	
	if( m_pimporter ){
		FbxArrayDelete(mAnimStackNameArray);
	}

//	if( m_pscene ){
//		m_pscene->Destroy();
//		m_pscene = 0;
//	}

	if( m_pimporter ){
		m_pimporter->Destroy();// インポータの削除
		m_pimporter = 0;
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
	m_motinfo.erase( m_motinfo.begin(), m_motinfo.end() );

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
	m_material.erase( m_material.begin(), m_material.end() );

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
	m_object.erase( m_object.begin(), m_object.end() );

	return 0;
}

int CModel::DestroyAncObj()
{
	map<int, CMQOMaterial*>::iterator itrmat;
	for( itrmat = m_ancmaterial.begin(); itrmat != m_ancmaterial.end(); itrmat++ ){
		CMQOMaterial* delmat = itrmat->second;
		if( delmat ){
			delete delmat;
		}
	}
	m_ancmaterial.erase( m_ancmaterial.begin(), m_ancmaterial.end() );

	map<int, CMQOObject*>::iterator itrobj;
	for( itrobj = m_ancobject.begin(); itrobj != m_ancobject.end(); itrobj++ ){
		CMQOObject* delobj = itrobj->second;
		if( delobj ){
			delete delobj;
		}
	}
	m_ancobject.erase( m_ancobject.begin(), m_ancobject.end() );


	map<int, CMQOObject*>::iterator itrobj2;
	for( itrobj2 = m_boneobject.begin(); itrobj2 != m_boneobject.end(); itrobj2++ ){
		CMQOObject* delobj = itrobj2->second;
		if( delobj ){
			delete delobj;
		}
	}
	m_boneobject.erase( m_boneobject.begin(), m_boneobject.end() );


	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* delbone = itrbone->second;
		if( delbone ){
			delete delbone;
		}
	}
	m_bonelist.erase( m_bonelist.begin(), m_bonelist.end() );

	m_topbone = 0;

	map<int, CInfScope*>::iterator itris;
	for( itris = m_infscope.begin(); itris != m_infscope.end(); itris++ ){
		CInfScope* delis = itris->second;
		if( delis ){
			delete delis;
		}
	}
	m_infscope.erase( m_infscope.begin(), m_infscope.end() );

	m_objectname.erase( m_objectname.begin(), m_objectname.end() );
	m_bonename.erase( m_bonename.begin(), m_bonename.end() );

	return 0;
}

int CModel::LoadMQO( LPDIRECT3DDEVICE9 pdev, WCHAR* wfile, WCHAR* modelfolder, float srcmult, int ismedia, int texpool )
{
	if( modelfolder ){
		wcscpy_s( m_modelfolder, MAX_PATH, modelfolder );
	}else{
		ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	}
	m_loadmult = srcmult;
	m_pdev = pdev;
	m_texpool = texpool;

	WCHAR fullname[MAX_PATH];

	if( ismedia == 1 ){
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
		ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
		wcscpy_s( m_filename, MAX_PATH, fullname );
	}

//WCHAR* dbgfind = wcsstr( wfile, L"gplane" );
//if( dbgfile ){
//	_ASSERT( 0 );
//}

	SetCurrentDirectory( m_dirname );


	DestroyMaterial();
	DestroyObject();

	CMQOFile mqofile;
	D3DXVECTOR3 vop( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vor( 0.0f, 0.0f, 0.0f );
	CallF( mqofile.LoadMQOFile( m_pdev, srcmult, m_filename, vop, vor, this ), return 1 );

	CallF( MakeObjectName(), return 1 );


	CallF( CreateMaterialTexture(), return 1 );

	SetMaterialName();

	return 0;
}


int CModel::LoadFBX( int skipdefref, LPDIRECT3DDEVICE9 pdev, WCHAR* wfile, WCHAR* modelfolder, float srcmult, FbxManager* psdk, FbxImporter** ppimporter, FbxScene** ppscene )
{

	//DestroyFBXSDK();

	m_psdk = psdk;
	*ppimporter = 0;
	*ppscene = 0;

	if( modelfolder ){
		wcscpy_s( m_modelfolder, MAX_PATH, modelfolder );
	}else{
		ZeroMemory( m_modelfolder, sizeof( WCHAR ) * MAX_PATH );
	}
	m_loadmult = srcmult;
	m_pdev = pdev;

	WCHAR fullname[MAX_PATH];

	wcscpy_s( fullname, MAX_PATH, wfile );

    WCHAR* strLastSlash = NULL;
    strLastSlash = wcsrchr( fullname, TEXT( '\\' ) );
    if( strLastSlash )
    {
		*strLastSlash = 0;
		wcscpy_s( m_dirname, MAX_PATH, fullname );
		wcscpy_s( m_filename, MAX_PATH, strLastSlash + 1 );
	}else{
		ZeroMemory( m_dirname, sizeof( WCHAR ) * MAX_PATH );
		wcscpy_s( m_filename, MAX_PATH, fullname );
	}


	DestroyMaterial();
	DestroyObject();

	char utf8path[MAX_PATH] = {0};
    // Unicode 文字コードを第一引数で指定した文字コードに変換する
    ::WideCharToMultiByte( CP_UTF8, 0, wfile, -1, utf8path, MAX_PATH, NULL, NULL );	

	FbxScene* pScene = 0;
	FbxImporter* pImporter = 0;

    pScene = FbxScene::Create(m_psdk,"");

    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    bool lStatus;
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);
    pImporter = FbxImporter::Create(m_psdk,"");
    const bool lImportStatus = pImporter->Initialize(utf8path, -1, m_psdk->GetIOSettings());
    pImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
    if( !lImportStatus )
    {
		_ASSERT( 0 );
		return 1;
	}
	
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
    }

    // Import the scene.
    lStatus = pImporter->Import(pScene);
    if(lStatus == false )
    {
		_ASSERT( 0 );
		return 1;
	}


//	CallF( InitFBXManager( &pSdkManager, &pImporter, &pScene, utf8path ), return 1 );

	m_bone2node.clear();
	FbxNode *pRootNode = pScene->GetRootNode();

	//CBone* rootbone = new CBone( this );
	//_ASSERT( rootbone );
	//rootbone->SetName( "ModelRootBone" );
	//rootbone->m_topboneflag = 1;
	//m_topbone = rootbone;
	//m_bonelist[rootbone->m_boneno] = rootbone;
	//m_bonename[ rootbone->m_bonename ] = rootbone;

	m_topbone = 0;
//	CBone* dummybone = new CBone( this );
//	_ASSERT( dummybone );
//	dummybone->SetName( "dummyBone" );
//	m_bonelist[dummybone->m_boneno] = dummybone;//これをm_topboneにしてはいけない。これは０でエラーが起こらないための応急措置。
//	m_bonename[dummybone->m_bonename] = dummybone;

	CreateFBXBoneReq( pScene, pRootNode, 0 );
	if( m_bonelist.size() <= 1 ){
		_ASSERT( 0 );
		delete (CBone*)(m_bonelist.begin()->second);
		m_bonelist.clear();
		m_topbone = 0;
	}
	CBone* chkbone = m_bonelist[0];
	if( !chkbone ){
		CBone* dummybone = new CBone( this );
		_ASSERT( dummybone );
		dummybone->SetName( "DummyBone" );
	}

	CreateFBXMeshReq( pRootNode );

DbgOut( L"fbx bonenum %d\r\n", m_bonelist.size() );


	D3DXMATRIX offsetmat;
	D3DXMatrixIdentity( &offsetmat );
	offsetmat._11 = srcmult;
	offsetmat._22 = srcmult;
	offsetmat._33 = srcmult;
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MultMat( offsetmat ), return 1 );
			CallF( curobj->MultVertex(), return 1; );
		}
	}

	//m_ktime0.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
	m_ktime0.SetSecondDouble( 1.0 / 30.0 );
//	m_ktime0.SetSecondDouble( 1.0 / 300.0 );


	CallF( MakePolyMesh4(), return 1 );
	CallF( MakeObjectName(), return 1 );
	CallF( CreateMaterialTexture(), return 1 );
	if( m_topbone ){
		CallF( CreateFBXSkinReq( pRootNode ), return 1 );
	}

	SetMaterialName();


	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		curbone->m_btkinflag = 1;
	}


	map<int,CMQOObject*>::iterator itr2;
	for( itr2 = m_object.begin(); itr2 != m_object.end(); itr2++ ){
		CMQOObject* curobj = itr2->second;
		if( curobj ){
			char* findnd = strstr( curobj->m_name, "_ND" );
			if( findnd ){
				curobj->m_dispflag = 0;
			}
		}
	}

	m_rigideleminfo.clear();
	m_impinfo.clear();

	if( skipdefref == 0 ){
		REINFO reinfo;
		ZeroMemory( &reinfo, sizeof( REINFO ) );
		strcpy_s( reinfo.filename, MAX_PATH, m_defaultrename );
		reinfo.btgscale = 9.07;
		m_rigideleminfo.push_back( reinfo );
		m_impinfo.push_back( m_defaultimpname );

		CreateRigidElemReq( m_topbone, 1, m_defaultrename, 1, m_defaultimpname );

		SetCurrentRigidElem( 0 );
		m_curreindex = 0;
		m_curimpindex = 0;


	}

	*ppimporter = pImporter;
	*ppscene = pScene;

	m_pimporter = pImporter;
	m_pscene = pScene;

	return 0;
}

int CModel::LoadFBXAnim( FbxManager* psdk, FbxImporter* pimporter, FbxScene* pscene, int (*tlfunc)( int srcmotid ) )
{
	if( !psdk || !pimporter || !pscene ){
		_ASSERT( 0 );
		return 0;
	}

	if( !m_topbone ){
		return 0;
	}

	this->m_tlFunc = tlfunc;

	FbxNode *pRootNode = pscene->GetRootNode();
	CallF( CreateFBXAnim( pscene, pRootNode ), return 1 );


	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		curbone->CalcAxisMat( 1, 0.0f );
	}

	return 0;
}

int CModel::CreateMaterialTexture()
{
	map<int,CMQOMaterial*>::iterator itr;
	for( itr = m_material.begin(); itr != m_material.end(); itr++ ){
		CMQOMaterial* curmat = itr->second;
		CallF( curmat->CreateTexture( m_dirname, m_texpool ), return 1 );
	}

	map<int,CMQOObject*>::iterator itrobj;
	for( itrobj = m_object.begin(); itrobj != m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( curobj ){
			map<int,CMQOMaterial*>::iterator itr;
			for( itr = curobj->m_material.begin(); itr != curobj->m_material.end(); itr++ ){
				CMQOMaterial* curmat = itr->second;
				CallF( curmat->CreateTexture( m_dirname, m_texpool ), return 1 );
			}
		}
	}

	return 0;
}

int CModel::OnRender( LPDIRECT3DDEVICE9 pdev, int lightflag, D3DXVECTOR4 diffusemult, int btflag )
{
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj && curobj->m_dispflag ){
			if( curobj->m_dispobj ){
				CallF( SetShaderConst( curobj, btflag ), return 1 );

				CMQOMaterial* rmaterial = 0;
				if( curobj->m_pm3 ){
					g_pEffect->SetMatrix( g_hmWorld, &m_matWorld );
					CallF( curobj->m_dispobj->RenderNormalPM3( lightflag, diffusemult ), return 1 );
				}else if( curobj->m_pm4 ){
					rmaterial = curobj->m_material.begin()->second;
					CallF( curobj->m_dispobj->RenderNormal( rmaterial, lightflag, diffusemult ), return 1 );
				}else{
					_ASSERT( 0 );
				}
			}
			if( curobj->m_displine ){
				CallF( curobj->m_displine->RenderLine( diffusemult ), return 1 );
			}
		}
	}

	return 0;
}

int CModel::GetModelBound( MODELBOUND* dstb )
{
	MODELBOUND mb;
	MODELBOUND addmb;
	ZeroMemory( &mb, sizeof( MODELBOUND ) );

	int calcflag = 0;
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_object.begin(); itr != m_object.end(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj->m_pm3 ){
			curobj->m_pm3->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->m_pm3->m_bound;
			}else{
				addmb = curobj->m_pm3->m_bound;
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
		if( curobj->m_pm4 ){
			curobj->m_pm4->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->m_pm4->m_bound;
			}else{
				addmb = curobj->m_pm4->m_bound;
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
		if( curobj->m_extline ){
			curobj->m_extline->CalcBound();
			if( calcflag == 0 ){
				mb = curobj->m_extline->m_bound;
			}else{
				addmb = curobj->m_extline->m_bound;
				AddModelBound( &mb, &addmb );
			}
			calcflag++;
		}
	}

	*dstb = mb;

	return 0;
}

int CModel::AddModelBound( MODELBOUND* mb, MODELBOUND* addmb )
{
	D3DXVECTOR3 newmin = mb->min;
	D3DXVECTOR3 newmax = mb->max;

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

	D3DXVECTOR3 diff;
	diff = mb->center - newmin;
	mb->r = D3DXVec3Length( &diff );

	return 0;
}



int CModel::SetShapeNoReq( CMQOFace** ppface, int facenum, int searchp, int shapeno, int* setfacenum )
{

	int fno;
	CMQOFace* findface[200];
	ZeroMemory( findface, sizeof( CMQOFace* ) * 200 );
	int findnum = 0;

	for( fno = 0; fno < facenum; fno++ ){
		CMQOFace* curface = *( ppface + fno );
		if( curface->m_shapeno != -1 ){
			continue;
		}

		int chki;
		for( chki = 0; chki < curface->m_pointnum; chki++ ){
			if( searchp == curface->m_index[ chki ] ){
				if( findnum >= 200 ){
					_ASSERT( 0 );
					return 1;
				}
				curface->m_shapeno = shapeno;
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
			for( i = 0; i < fface->m_pointnum; i++ ){
				int newsearch = fface->m_index[ i ];
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
		if( curface->m_shapeno == shapeno ){
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
			char* nameptr = curobj->m_name;
			int sdefcmp, bdefcmp;
			sdefcmp = strncmp( nameptr, "sdef:", 5 );
			bdefcmp = strncmp( nameptr, "bdef:", 5 );
			if( (sdefcmp != 0) && (bdefcmp != 0) ){
				int leng = (int)strlen( nameptr );
				string firstname( nameptr, nameptr + leng );
				m_objectname[ firstname ] = curobj;
				curobj->m_dispname = firstname;
			}else{
				char* startptr = nameptr + 5;
				int leng = (int)strlen( startptr );
				string firstname( startptr, startptr + leng );
				m_objectname[ firstname ] = curobj;
				curobj->m_dispname = firstname;
			}
		}
	}

	return 0;
}

int CModel::DbgDump()
{
	DbgOut( L"######start DbgDump\r\n" );

	DbgOut( L"Dump Bone And InfScope\r\n" );

	if( m_topbone ){
		DbgDumpBoneReq( m_topbone, 0 );
	}

//	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, name, 256, wname, 256 );

	DbgOut( L"######end DbgDump\r\n" );


	return 0;
}

int CModel::DbgDumpBoneReq( CBone* boneptr, int broflag )
{
	char mes[1024];
	WCHAR wmes[1024];

	if( boneptr->m_parent ){
		sprintf_s( mes, 1024, "\tboneno %d, bonename %s - parent %s\r\n", boneptr->m_boneno, boneptr->m_bonename, boneptr->m_parent->m_bonename );
	}else{
		sprintf_s( mes, 1024, "\tboneno %d, bonename %s - parent NONE\r\n", boneptr->m_boneno, boneptr->m_bonename );
	}
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
	DbgOut( wmes );

	DbgOut( L"\t\tbonepos (%f, %f, %f), (%f, %f, %f)\r\n", 
		boneptr->m_jointwpos.x, boneptr->m_jointwpos.y, boneptr->m_jointwpos.z,
		boneptr->m_jointfpos.x, boneptr->m_jointfpos.y, boneptr->m_jointfpos.z );

	
	DbgOut( L"\t\tinfscopenum %d\r\n", boneptr->m_isnum );
	int isno;
	for( isno = 0; isno < boneptr->m_isnum; isno++ ){
		CInfScope* curis = boneptr->m_isarray[ isno ];
		CBone* infbone = 0;
		if( curis->m_applyboneno >= 0 ){
			infbone = m_bonelist[ curis->m_applyboneno ];
			sprintf_s( mes, 1024, "\t\tInfScope %d, validflag %d, facenum %d, applybone %s\r\n", 
				isno, curis->m_validflag, curis->m_facenum, infbone->m_bonename );
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
			DbgOut( wmes );

		}else{
			DbgOut( L"\t\tInfScope %d, validflag %d, facenum %d, applybone is none\r\n", 
				isno, curis->m_validflag, curis->m_facenum );
		}

		if( curis->m_targetobj ){
			sprintf_s( mes, 1024, "\t\t\ttargetobj %s\r\n", curis->m_targetobj->m_name );
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, mes, 1024, wmes, 1024 );
			DbgOut( wmes );
		}else{
			DbgOut( L"\t\t\ttargetobj is none\r\n" );
		}
	}

//////////
	if( boneptr->m_child ){
		DbgDumpBoneReq( boneptr->m_child, 1 );
	}
	if( (broflag == 1) && boneptr->m_brother ){
		DbgDumpBoneReq( boneptr->m_brother, 1 );
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
			CallF( curobj->MakePolymesh4( m_pdev, m_material ), return 1 );
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

int CModel::Motion2Bt( int firstflag, CModel* coldisp[COL_MAX], double nextframe, D3DXMATRIX* mW, D3DXMATRIX* mVP )
{
	UpdateMatrix( -1, mW, mVP );

	if( m_topbt ){
		SetBtKinFlagReq( m_topbt, 0 );
	}

	if( firstflag == 1 ){
		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* boneptr = itrbone->second;
			if( boneptr ){
				boneptr->m_startmat2 = boneptr->m_curmp.m_worldmat;
			}
		}
	}

	if( !m_topbt ){
		return 0;
	}

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* boneptr = itrbone->second;
		if( boneptr ){
			map<CBone*,CRigidElem*>::iterator itrre;
			for( itrre = boneptr->m_rigidelem.begin(); itrre != boneptr->m_rigidelem.end(); itrre++ ){
				CRigidElem* curre = itrre->second;
				if( curre && (curre->m_skipflag != 1) ){
					CBone* chilbone = itrre->first;
					_ASSERT( chilbone );

					boneptr->CalcRigidElemParams( coldisp, chilbone, firstflag );
				}
			}				
		}
	}

/***
CBone* chkbone1 = m_bonename[ "FLOAT_BT_twinte1_L__Joint" ];
_ASSERT( chkbone1 );
CBone* chkbone2 = m_bonename[ "atama_Joint_bunki" ];
_ASSERT( chkbone2 );

DbgOut( L"check kinflag !!!! : previewflag %d, float kinflag %d, bunki kinflag %d\r\n",
	g_previewFlag, chkbone1->m_btkinflag, chkbone2->m_btkinflag );
***/

	Motion2BtReq( m_topbt );


	return 0;
}

void CModel::Motion2BtReq( CBtObject* srcbto )
{
	if( srcbto->m_bone && (srcbto->m_bone->m_btkinflag == 1) ){
		srcbto->Motion2Bt();
	}

	int chilnum = srcbto->m_chilbt.size();
	int chilno;
	for( chilno = 0; chilno < chilnum; chilno++ ){
		Motion2BtReq( srcbto->m_chilbt[ chilno ] );
	}
}

int CModel::UpdateMatrix( int startbone, D3DXMATRIX* wmat, D3DXMATRIX* vpmat )
{
	m_matWorld = *wmat;
	m_matVP = *vpmat;

	if( !m_curmotinfo ){
		return 0;//!!!!!!!!!!!!
	}

	int curmotid = m_curmotinfo->motid;
	double curframe = m_curmotinfo->curframe;

//	D3DXMATRIX inimat;
//	D3DXMatrixIdentity( &inimat );
//	CQuaternion iniq;
//	iniq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			curbone->UpdateMatrix( curmotid, curframe, wmat, vpmat );
		}
	}

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
		if( !(curobj->m_findshape.empty()) ){
			GetShapeWeight( m_fbxobj[curobj].node, m_fbxobj[curobj].mesh, lTime, mCurrentAnimLayer, curobj );

			CallF( curobj->UpdateMorphBuffer(), return 1 );
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

	MoveMemory( curobj->m_mpoint, curobj->m_pointbuf, sizeof( D3DXVECTOR3 ) * lVertexCount );

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
							D3DXVECTOR3 xv;
							D3DXVECTOR3 diffpoint;

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

int CModel::GetFBXShape( FbxMesh* pMesh, CMQOObject* curobj, FbxAnimLayer* panimlayer, int animleng, FbxTime starttime, FbxTime timestep )
{
	int lVertexCount = pMesh->GetControlPointsCount();
	if( lVertexCount != curobj->m_vertex ){
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
				int framecnt;
				for( framecnt = 0; framecnt < animleng; framecnt++ ){
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
						const char* nameptr = lChannel->GetName();
						int existshape = 0;
						existshape = curobj->ExistShape( (char*)nameptr );
						if( existshape == 0 ){

							curobj->AddShapeName( (char*)nameptr );

//WCHAR wcurname[256]={0L};
//WCHAR wobjname[256]={0L};
//ZeroMemory( wcurname, sizeof( WCHAR ) * 256 );
//MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)nameptr, 256, wcurname, 256 );
//ZeroMemory( wobjname, sizeof( WCHAR ) * 256 );
//MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)curobj->m_name, 256, wobjname, 256 );
//DbgOut( L"addmorph : objname %s, targetname %s\r\n",
//	   wobjname, wcurname );

							FbxVector4* shapev = lShape->GetControlPoints();
							_ASSERT( shapev );
							for (int j = 0; j < lVertexCount; j++)
							{
								D3DXVECTOR3 xv;
								xv.x = (float)shapev[j][0];
								xv.y = (float)shapev[j][1];
								xv.z = (float)shapev[j][2];
								curobj->SetShapeVert( (char*)nameptr, j, xv );
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
	if( lVertexCount != curobj->m_vertex ){
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

				const char* nameptr = lChannel->GetName();
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
						curobj->SetShapeWeight( (char*)nameptr, (float)lWeight );

//double curframe = m_curmotinfo->curframe;
//WCHAR wtargetname[256]={0L};
//MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)nameptr, 256, wtargetname, 256 );
//DbgOut( L"weight check !!! : target %s, frame %f, weight %f\r\n",
//	wtargetname, curframe, (float)lWeight );

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
		return 0;//!!!!!!!!!!!
	}

	float set3x4[MAXCLUSTERNUM][12];
	ZeroMemory( set3x4, sizeof( float ) * 12 * MAXCLUSTERNUM );

	int setclcnt = 0;
	int clcnt;
	for( clcnt = 0; clcnt < (int)srcobj->m_cluster.size(); clcnt++ ){
		CBone* curbone = srcobj->m_cluster[ clcnt ];
		if( !curbone ){
			_ASSERT( 0 );
			return 1;
		}

		if( btflag == 0 ){
			set3x4[clcnt][0] = curbone->m_curmp.m_worldmat._11;
			set3x4[clcnt][1] = curbone->m_curmp.m_worldmat._12;
			set3x4[clcnt][2] = curbone->m_curmp.m_worldmat._13;

			set3x4[clcnt][3] = curbone->m_curmp.m_worldmat._21;
			set3x4[clcnt][4] = curbone->m_curmp.m_worldmat._22;
			set3x4[clcnt][5] = curbone->m_curmp.m_worldmat._23;

			set3x4[clcnt][6] = curbone->m_curmp.m_worldmat._31;
			set3x4[clcnt][7] = curbone->m_curmp.m_worldmat._32;
			set3x4[clcnt][8] = curbone->m_curmp.m_worldmat._33;

			set3x4[clcnt][9] = curbone->m_curmp.m_worldmat._41;
			set3x4[clcnt][10] = curbone->m_curmp.m_worldmat._42;
			set3x4[clcnt][11] = curbone->m_curmp.m_worldmat._43;
		}else{
			set3x4[clcnt][0] = curbone->m_curmp.m_btmat._11;
			set3x4[clcnt][1] = curbone->m_curmp.m_btmat._12;
			set3x4[clcnt][2] = curbone->m_curmp.m_btmat._13;

			set3x4[clcnt][3] = curbone->m_curmp.m_btmat._21;
			set3x4[clcnt][4] = curbone->m_curmp.m_btmat._22;
			set3x4[clcnt][5] = curbone->m_curmp.m_btmat._23;

			set3x4[clcnt][6] = curbone->m_curmp.m_btmat._31;
			set3x4[clcnt][7] = curbone->m_curmp.m_btmat._32;
			set3x4[clcnt][8] = curbone->m_curmp.m_btmat._33;

			set3x4[clcnt][9] = curbone->m_curmp.m_btmat._41;
			set3x4[clcnt][10] = curbone->m_curmp.m_btmat._42;
			set3x4[clcnt][11] = curbone->m_curmp.m_btmat._43;
		}
		setclcnt++;

	}

	if( setclcnt > 0 ){
		HRESULT hr;
		hr = g_pEffect->SetValue( g_hm3x4Mat, (void*)set3x4, sizeof( float ) * 12 * MAXCLUSTERNUM );
		if( hr != D3D_OK ){
			_ASSERT( 0 );
			return 1;
		}
	}

	return 0;
}

int CModel::SetBoneEul( int boneno, D3DXVECTOR3 srceul )
{
	if( boneno < 0 ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone;
	curbone = m_bonelist[ boneno ];
	if( curbone ){
		curbone->m_curmp.SetEul( &(curbone->m_axisq), srceul );
	}

	return 0;
}

int CModel::GetBoneEul( int boneno, D3DXVECTOR3* dsteul )
{
	if( boneno < 0 ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone;
	curbone = m_bonelist[ boneno ];
	if( curbone ){
		*dsteul = curbone->m_curmp.m_eul;
	}else{
		*dsteul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	}

	return 0;
}

int CModel::FillTimeLine( OrgWinGUI::OWP_Timeline& timeline, map<int, int>& lineno2boneno, map<int, int>& boneno2lineno )
{
	lineno2boneno.erase( lineno2boneno.begin(), lineno2boneno.end() );
	boneno2lineno.erase( boneno2lineno.begin(), boneno2lineno.end() );

	if( m_bonelist.empty() ){
		return 0;
	}

	int lineno = 0;
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		_ASSERT( curbone );

		//行を追加
		if( curbone->m_type != FBXBONE_NULL ){
			timeline.newLine( 0, curbone->m_wbonename );
		}else{
			timeline.newLine( 1, curbone->m_wbonename );
		}

		lineno2boneno[ lineno ] = curbone->m_boneno;
		boneno2lineno[ curbone->m_boneno ] = lineno;
		lineno++;
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

	//選択行を設定
	timeline.setCurrentLineName( m_topbone->m_wbonename );

	return 0;
}

void CModel::FillTimelineReq( OrgWinGUI::OWP_Timeline& timeline, CBone* curbone, int* linenoptr, 
	map<int, int>& lineno2boneno, map<int, int>& boneno2lineno, int broflag )
{
	//行を追加
	if( curbone->m_type != FBXBONE_NULL ){
		timeline.newLine( 0, curbone->m_wbonename );
	}else{
		timeline.newLine( 1, curbone->m_wbonename );
	}

	lineno2boneno[ *linenoptr ] = curbone->m_boneno;
	boneno2lineno[ curbone->m_boneno ] = *linenoptr;
	(*linenoptr)++;

/***
	_ASSERT( m_curmotinfo );
	CMotionPoint* curmp = curbone->m_motionkey[ m_curmotinfo->motid ];
	while( curmp ){
		timeline.newKey( curbone->m_wbonename, curmp->m_frame, (void*)curmp );
		curmp = curmp->m_next;
	}
***/

	if( curbone->m_child ){
		FillTimelineReq( timeline, curbone->m_child, linenoptr, lineno2boneno, boneno2lineno, 1 );
	}
	if( broflag && curbone->m_brother ){
		FillTimelineReq( timeline, curbone->m_brother, linenoptr, lineno2boneno, boneno2lineno, 1 );
	}
}

int CModel::AddMotion( char* srcname, WCHAR* wfilename, double srcleng, int* dstid )
{
	*dstid = -1;
	int leng = (int)strlen( srcname );

	int maxid = 0;
	map<int, MOTINFO*>::iterator itrmi;
	for( itrmi = m_motinfo.begin(); itrmi != m_motinfo.end(); itrmi++ ){
		MOTINFO* chkmi = itrmi->second;
		if( chkmi ){
			if( maxid < chkmi->motid ){
				maxid = chkmi->motid;
			}
		}
	}
	int newid = maxid + 1;


	MOTINFO* newmi = (MOTINFO*)malloc( sizeof( MOTINFO ) );
	if( !newmi ){
		_ASSERT( 0 );
		return 1;
	}
	ZeroMemory( newmi, sizeof( MOTINFO ) );

	strcpy_s( newmi->motname, 256, srcname );
	if( wfilename ){
		wcscpy_s( newmi->wfilename, MAX_PATH, wfilename );
	}else{
		ZeroMemory( newmi->wfilename, sizeof( WCHAR ) * MAX_PATH );
	}
	ZeroMemory( newmi->engmotname, sizeof( char ) * 256 );

	newmi->motid = newid;
	newmi->frameleng = srcleng;
	newmi->curframe = 0.0;
	newmi->speed = 1.0;
	newmi->loopflag = 1;

	m_motinfo[ newid ] = newmi;

///////////////
/***
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CallF( curbone->MakeFirstMP( newid ), return 1 );
		}
	}
***/

	*dstid = newid;

	return 0;
}

CMotionPoint* CModel::AddMotionPoint( int srcmotid, double srcframe, int srcboneno, int calcflag, int* existptr )
{
	if( srcboneno < 0 ){
		return 0;
	}

	CBone* curbone = m_bonelist[ srcboneno ];
	if( curbone ){
		return curbone->AddMotionPoint( srcmotid, srcframe, calcflag, existptr );
	}else{
		return 0;
	}

}

int CModel::SetCurrentMotion( int srcmotid )
{
	m_curmotinfo = m_motinfo[ srcmotid ];
	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 1;
	}else{
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
		_ASSERT( 0 );
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
	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CallF( curbone->DeleteMotion( motid ), return 1 );
		}
	}

	map<int, MOTINFO*>::iterator itrmi;
	itrmi = m_motinfo.find( motid );
	if( itrmi != m_motinfo.end() ){
		MOTINFO* delmi = itrmi->second;
		if( delmi ){
			delete delmi;
		}
		m_motinfo.erase( itrmi );
	}

	int undono;
	for( undono = 0; undono < UNDOMAX; undono++ ){
		if( m_undomotion[undono].m_savemotinfo.motid == motid ){
			m_undomotion[undono].m_validflag = 0;
		}
	}


	return 0;
}

int CModel::GetSymBoneNo( int srcboneno, int* dstboneno, int* existptr )
{
	*existptr = 0;

	CBone* srcbone = m_bonelist[ srcboneno ];
	if( !srcbone ){
		*dstboneno = -1;
		return 0;
	}

	int findflag = 0;
	WCHAR findname[256];
	ZeroMemory( findname, sizeof( WCHAR ) * 256 );
	wcscpy_s( findname, 256, srcbone->m_wbonename );

	WCHAR* lpat = wcsstr( findname, L"_L_" );
	if( lpat ){
		*lpat = TEXT( '_' );
		*(lpat + 1) = TEXT( 'R' );
		*(lpat + 2) = TEXT( '_' );
		//wcsncat_s( findname, 256, L"_R_", 3 );
		findflag = 1;
	}else{
		WCHAR* rpat = wcsstr( findname, L"_R_" );
		if( rpat ){
			*rpat = TEXT( '_' );
			*(rpat + 1) = TEXT( 'L' );
			*(rpat + 2) = TEXT( '_' );
			//wcsncat_s( findname, 256, L"_L_", 3 );
			findflag = 1;
		}
	}

	if( findflag == 0 ){
		*dstboneno = srcboneno;
		*existptr = 0;
	}else{
		CBone* dstbone = 0;
		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* chkbone = itrbone->second;
			if( chkbone && (wcscmp( findname, chkbone->m_wbonename ) == 0) ){
				dstbone = chkbone;
				break;
			}
		}
		if( dstbone ){
			*dstboneno = dstbone->m_boneno;
			*existptr = 1;
		}else{
			*dstboneno = srcboneno;
			*existptr = 0;
		}
	}

	return 0;
}

int CModel::PickBone( PICKINFO* pickinfo )
{
	pickinfo->pickobjno = -1;

	float fw, fh;
	fw = (float)pickinfo->winx / 2.0f;
	fh = (float)pickinfo->winy / 2.0f;

	int minno = -1;
	D3DXVECTOR3 cmpsc;
	D3DXVECTOR3 picksc = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 pickworld = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	float cmpdist;
	float mindist = 0.0f;
	int firstflag = 1;


	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			cmpsc.x = ( 1.0f + curbone->m_childscreen.x ) * fw;
			cmpsc.y = ( 1.0f - curbone->m_childscreen.y ) * fh;
			cmpsc.z = curbone->m_childscreen.z;

			if( (cmpsc.z >= 0.0f) && (cmpsc.z <= 1.0f) ){
				float mag;
				mag = ( (float)pickinfo->clickpos.x - cmpsc.x ) * ( (float)pickinfo->clickpos.x - cmpsc.x ) + 
					( (float)pickinfo->clickpos.y - cmpsc.y ) * ( (float)pickinfo->clickpos.y - cmpsc.y );
				if( mag != 0.0f ){
					cmpdist = sqrtf( mag );
				}else{
					cmpdist = 0.0f;
				}

				if( (firstflag || (cmpdist <= mindist)) && (cmpdist <= (float)pickinfo->pickrange ) ){
					minno = curbone->m_boneno;
					mindist = cmpdist;
					picksc = cmpsc;
					pickworld = curbone->m_childworld;
					firstflag = 0;
				}
			}
		}
	}

	pickinfo->pickobjno = minno;
	if( minno >= 0 ){
		pickinfo->objscreen = picksc;
		pickinfo->objworld = pickworld;
	}

	return 0;
}

void CModel::SetSelectFlagReq( CBone* boneptr, int broflag )
{
	boneptr->m_selectflag = 1;

	if( boneptr->m_child ){
		SetSelectFlagReq( boneptr->m_child, 1 );
	}
	if( boneptr->m_brother && broflag ){
		SetSelectFlagReq( boneptr->m_brother, 1 );
	}
}


int CModel::CollisionNoBoneObj_Mouse( PICKINFO* pickinfo, char* objnameptr )
{
	//当たったら１、当たらなかったら０を返す。エラーも０を返す。

	CMQOObject* curobj = m_objectname[ objnameptr ];
	if( !curobj ){
		_ASSERT( 0 );
		return 0;
	}

	D3DXVECTOR3 startlocal, dirlocal;
	CalcMouseLocalRay( pickinfo, &startlocal, &dirlocal );

	int colli = curobj->CollisionLocal_Ray( startlocal, dirlocal );

	return colli;
}

int CModel::CalcMouseLocalRay( PICKINFO* pickinfo, D3DXVECTOR3* startptr, D3DXVECTOR3* dirptr )
{
	D3DXVECTOR3 startsc, endsc;
	float rayx, rayy;
	rayx = (float)pickinfo->clickpos.x / ((float)pickinfo->winx / 2.0f) - 1.0f;
	rayy = 1.0f - (float)pickinfo->clickpos.y / ((float)pickinfo->winy / 2.0f);

	startsc = D3DXVECTOR3( rayx, rayy, 0.0f );
	endsc = D3DXVECTOR3( rayx, rayy, 1.0f );
	
    D3DXMATRIX mWVP, invmWVP;
	mWVP = m_matWorld * m_matVP;
	D3DXMatrixInverse( &invmWVP, NULL, &mWVP );

	D3DXVECTOR3 startlocal, endlocal;

	D3DXVec3TransformCoord( &startlocal, &startsc, &invmWVP );
	D3DXVec3TransformCoord( &endlocal, &endsc, &invmWVP );

	D3DXVECTOR3 dirlocal = endlocal - startlocal;
	DXVec3Normalize( &dirlocal, &dirlocal );

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
		if( curbone->m_faceptr && (curbone->m_faceptr && curbone->m_faceptr->m_bonetype != MIKOBONE_NORMAL) ){
			break;
		}
		curbone = curbone->m_parent;
		levelcnt++;
	}

	return retbone;
}


int CModel::TransformBone( int winx, int winy, int srcboneno, D3DXVECTOR3* worldptr, D3DXVECTOR3* screenptr, D3DXVECTOR3* dispptr )
{					
	CBone* curbone;
	curbone = m_bonelist[ srcboneno ];
	*worldptr = curbone->m_childworld;
	D3DXMATRIX mWVP = curbone->m_curmp.m_worldmat * m_matVP;
	D3DXVec3TransformCoord( screenptr, &curbone->m_jointfpos, &mWVP );

	float fw, fh;
	fw = (float)winx / 2.0f;
	fh = (float)winy / 2.0f;
	dispptr->x = ( 1.0f + screenptr->x ) * fw;
	dispptr->y = ( 1.0f - screenptr->y ) * fh;
	dispptr->z = screenptr->z;

	return 0;
}


int CModel::ChangeMotFrameLeng( int motid, double srcleng )
{
	MOTINFO* dstmi = m_motinfo[ motid ];
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

int CModel::AdvanceTime( int previewflag, double difftime, double* nextframeptr, int* endflagptr, int srcmotid )
{
	*endflagptr = 0;

	int loopflag = 0;
	MOTINFO* curmotinfo;
	if( srcmotid >= 0 ){
		curmotinfo = m_motinfo[ srcmotid ];
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

	if( previewflag > 0 ){
		nextframe = curframe + difftime / oneframe * curspeed;
		if( nextframe > ( curmotinfo->frameleng - 1.0 ) ){
			if( loopflag == 0 ){
				nextframe = curmotinfo->frameleng - 1.0;
				*endflagptr = 1;
			}else{
				nextframe = 0.0;
			}
		}
	}else{
		nextframe = curframe - difftime / oneframe * curspeed;
		if( nextframe < 0.0 ){
			if( loopflag == 0 ){
				nextframe = 0.0;
				*endflagptr = 1;
			}else{
				nextframe = curmotinfo->frameleng - 1.0;
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
			CallF( ConvEngName( ENGNAME_DISP, curobj->m_name, 256, curobj->m_engname, 256 ), return 1 );
		}
	}


	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CallF( ConvEngName( ENGNAME_BONE, curbone->m_bonename, 256, curbone->m_engbonename, 256 ), return 1 );
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

	int defmaterialno = m_material.size();
	dummymat->materialno = defmaterialno;
	strcpy_s( dummymat->name, 256, "dummyMaterial" );

	m_material[defmaterialno] = dummymat;

	return 0;
}


int CModel::CreateFBXMeshReq( FbxNode* pNode )
{
	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();
        FbxGeometryConverter lConverter(pNode->GetFbxManager());

		char mes[256];

		int shapecnt;
		CMQOObject* newobj = 0;

		switch ( type )
		{
			case FbxNodeAttribute::eMesh:

				newobj = GetFBXMesh( pNode, pAttrib, pNode->GetName() );     // メッシュを作成
				shapecnt = pNode->GetMesh()->GetShapeCount();
				if( shapecnt > 0 ){
					sprintf_s( mes, 256, "%s, shapecnt %d", pNode->GetName(), shapecnt );
					MessageBoxA( NULL, mes, "check", MB_OK );
				}
				break;
//			case FbxNodeAttribute::eNURB:
//			case FbxNodeAttribute::eNURBS_SURFACE:
//                lConverter.TriangulateInPlace(pNode);
//				GetFBXMesh( pAttrib, pNode->GetName() );     // メッシュを作成
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
		CreateFBXMeshReq( pChild );
	}

	return 0;
}

int CModel::CreateFBXShape( FbxAnimLayer* panimlayer, int animleng, FbxTime starttime, FbxTime timestep )
{
	map<CMQOObject*,FBXOBJ>::iterator itrobjindex;
	for( itrobjindex = m_fbxobj.begin(); itrobjindex != m_fbxobj.end(); itrobjindex++ ){
		FBXOBJ curfbxobj = itrobjindex->second;
		FbxMesh* curmesh = curfbxobj.mesh;
		CMQOObject* curobj = itrobjindex->first;
		if( curmesh && curobj ){
			int shapecnt = curmesh->GetShapeCount();
			if( shapecnt > 0 ){
				CallF( GetFBXShape( curmesh, curobj, panimlayer, animleng, starttime, timestep ), return 1 );
			}
		}
	}
	return 0;
}


CMQOObject* CModel::GetFBXMesh( FbxNode* pNode, FbxNodeAttribute *pAttrib, const char* nodename )
{
	FbxMesh *pMesh = (FbxMesh*)pAttrib;
	CMQOObject* newobj = new CMQOObject();
	_ASSERT( newobj );
	newobj->m_objfrom = OBJFROM_FBX;
	strcpy_s( newobj->m_name, 256, nodename );
	m_object[ newobj->m_objectno ] = newobj;

	FBXOBJ fbxobj;
	fbxobj.node = pNode;
	fbxobj.mesh = pMesh;
	m_fbxobj[newobj] = fbxobj;

//shape
//	int morphnum = pMesh->GetShapeCount();
//	newobj->m_morphnum = morphnum;

//マテリアル
	FbxNode* node = pMesh->GetNode();
	if ( node != 0 ) {
		// マテリアルの数
		int materialNum_ = node->GetMaterialCount();
		// マテリアル情報を取得
		for( int i = 0; i < materialNum_; ++i ) {
			FbxSurfaceMaterial* material = node->GetMaterial( i );
			if ( material != 0 ) {
				CMQOMaterial* newmqomat = new CMQOMaterial();
				int mqomatno = newobj->m_material.size();
				newmqomat->materialno = mqomatno;
				newobj->m_material[mqomatno] = newmqomat;

				SetMQOMaterial( newmqomat, material );

			}
		}
	}

//頂点
	int PolygonNum       = pMesh->GetPolygonCount();
	int PolygonVertexNum = pMesh->GetPolygonVertexCount();
	int *IndexAry        = pMesh->GetPolygonVertices();

	int controlNum = pMesh->GetControlPointsCount();   // 頂点数
	FbxVector4* src = pMesh->GetControlPoints();    // 頂点座標配列

	// コピー
	newobj->m_vertex = controlNum;
	newobj->m_pointbuf = (D3DXVECTOR3*)malloc( sizeof( D3DXVECTOR3 ) * controlNum );
	for ( int i = 0; i < controlNum; ++i ) {
		D3DXVECTOR3* curctrl = newobj->m_pointbuf + i;
		curctrl->x = (float)src[ i ][ 0 ];
		curctrl->y = (float)src[ i ][ 1 ];
		curctrl->z = (float)src[ i ][ 2 ];
		//curctrl->w = (float)src[ i ][ 3 ];

//DbgOut( L"GetFBXMesh : ctrl %d, (%f, %f, %f)\r\n",
//	i, curctrl->x, curctrl->y, curctrl->z );

	}

	newobj->m_face = PolygonNum;
	newobj->m_facebuf = new CMQOFace[ PolygonNum ];
	for ( int p = 0; p < PolygonNum; p++ ) {
		int IndexNumInPolygon = pMesh->GetPolygonSize( p );  // p番目のポリゴンの頂点数
		if( (IndexNumInPolygon != 3) && (IndexNumInPolygon != 4) ){
			_ASSERT( 0 );
			return 0;
		}
		
		CMQOFace* curface = newobj->m_facebuf + p;
		curface->m_pointnum = IndexNumInPolygon;

		for ( int n = 0; n < IndexNumInPolygon; n++ ) {
			// ポリゴンpを構成するn番目の頂点のインデックス番号
			int IndexNumber = pMesh->GetPolygonVertex( p, n );
			curface->m_faceno = p;
			curface->m_index[ n ] = IndexNumber;
			curface->m_materialno = 0;
			curface->m_bonetype = MIKOBONE_NONE;
		}
	}

//法線
	int layerNum = pMesh->GetLayerCount();
	for ( int i = 0; i < layerNum; ++i ) {
	   FbxLayer* layer = pMesh->GetLayer( i );
	   FbxLayerElementNormal* normalElem = layer->GetNormals();
	   if ( normalElem == 0 ) {
		  continue;   // 法線無し
	   }
	   // 法線あった！
		// 法線の数・インデックス
		int    normalNum          = normalElem->GetDirectArray().GetCount();
		int    indexNum           = normalElem->GetIndexArray().GetCount();

		newobj->m_normalleng = normalNum;
		newobj->m_normal = (D3DXVECTOR3*)malloc( sizeof( D3DXVECTOR3 ) * normalNum );

		// マッピングモード・リファレンスモード取得
		FbxLayerElement::EMappingMode mappingMode = normalElem->GetMappingMode();
		FbxLayerElement::EReferenceMode refMode = normalElem->GetReferenceMode();

		if ( mappingMode == FbxLayerElement::eByPolygonVertex ) {
		   if ( refMode == FbxLayerElement::eDirect ) {
			  // 直接取得
			  for ( int i = 0; i < normalNum; ++i ) {
				D3DXVECTOR3* curn = newobj->m_normal + i;
				curn->x = (float)normalElem->GetDirectArray().GetAt( i )[ 0 ];
				curn->y = (float)normalElem->GetDirectArray().GetAt( i )[ 1 ];
				curn->z = (float)normalElem->GetDirectArray().GetAt( i )[ 2 ];
			  }
		   }
		} else if ( mappingMode == FbxLayerElement::eByControlPoint ) {
		   if ( refMode == FbxLayerElement::eDirect ) {
			  // 直接取得
			  for ( int i = 0; i < normalNum; ++i ) {
				D3DXVECTOR3* curn = newobj->m_normal + i;
				curn->x = (float)normalElem->GetDirectArray().GetAt( i )[ 0 ];
				curn->y = (float)normalElem->GetDirectArray().GetAt( i )[ 1 ];
				curn->z = (float)normalElem->GetDirectArray().GetAt( i )[ 2 ];
			  }
		   }
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

		if ( mappingMode == FbxLayerElement::eByPolygonVertex ) {
			if ( refMode == FbxLayerElement::eDirect ) {
				int size = UVNum;
				newobj->m_uvleng = size;
				newobj->m_uvbuf = (D3DXVECTOR2*)malloc( sizeof( D3DXVECTOR2 ) * size );

				// 直接取得
				for ( int i = 0; i < size; ++i ) {
					(newobj->m_uvbuf + i)->x = (float)elem->GetDirectArray().GetAt( i )[ 0 ];
					(newobj->m_uvbuf + i)->y = (float)elem->GetDirectArray().GetAt( i )[ 1 ];
				}
			} else if ( refMode == FbxLayerElement::eIndexToDirect ) {
				int size = indexNum;
				newobj->m_uvleng = size;
				newobj->m_uvbuf = (D3DXVECTOR2*)malloc( sizeof( D3DXVECTOR2 ) * size );

				// インデックスから取得
				for ( int i = 0; i < size; ++i ) {
					int index = elem->GetIndexArray().GetAt( i );
					(newobj->m_uvbuf + i)->x = (float)elem->GetDirectArray().GetAt( index )[ 0 ];
					(newobj->m_uvbuf + i)->y = (float)elem->GetDirectArray().GetAt( index )[ 1 ];
				}
			}
		}
		break;
	}

	return newobj;
}

int CModel::SetMQOMaterial( CMQOMaterial* newmqomat, FbxSurfaceMaterial* pMaterial )
{
	strcpy_s( newmqomat->name, 256, pMaterial->GetName() );

	char* emitex = 0;
    const FbxDouble3 lEmissive = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, &emitex);
	if( emitex ){
		DbgOut( L"SetMQOMaterial : emitexture find\r\n" );
	}
	newmqomat->emi3f.x = (float)lEmissive[0];
	newmqomat->emi3f.y = (float)lEmissive[1];
	newmqomat->emi3f.z = (float)lEmissive[2];

	char* ambtex = 0;
	const FbxDouble3 lAmbient = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, &ambtex);
	if( ambtex ){
		DbgOut( L"SetMQOMaterial : ambtexture find\r\n" );
	}
	newmqomat->amb3f.x = (float)lAmbient[0];
	newmqomat->amb3f.y = (float)lAmbient[1];
	newmqomat->amb3f.z = (float)lAmbient[2];

	char* diffusetex = 0;
    const FbxDouble3 lDiffuse = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, &diffusetex);
	if( diffusetex ){
		//strcpy_s( newmqomat->tex, 256, diffusetex );
		DbgOut( L"SetMQOMaterial : diffusetexture find\r\n" );
	}
	newmqomat->dif4f.x = (float)lDiffuse[0];
	newmqomat->dif4f.y = (float)lDiffuse[1];
	newmqomat->dif4f.z = (float)lDiffuse[2];
	newmqomat->dif4f.w = 1.0f;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	char* spctex = 0;
    const FbxDouble3 lSpecular = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, &spctex);
	if( spctex ){
		DbgOut( L"SetMQOMaterial : spctexture find\r\n" );
	}
	newmqomat->spc3f.x = (float)lSpecular[0];
	newmqomat->spc3f.y = (float)lSpecular[1];
	newmqomat->spc3f.z = (float)lSpecular[2];

    FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        newmqomat->power = static_cast<float>(lShininess);
    }


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
                char* nameptr = (char*)lLayeredTexture->GetName();
				if( nameptr ){
					char tempname[256];
					strcpy_s( tempname, 256, nameptr );
					char* lastslash = strrchr( tempname, '/' );
					if( !lastslash ){
						lastslash = strrchr( tempname, '\\' );
					}
					if( lastslash ){
						strcpy_s( newmqomat->tex, 256, lastslash + 1 );
					}else{
						strcpy_s( newmqomat->tex, 256, tempname );
					}
					char* lastp = strrchr( newmqomat->tex, '.' );
					if( !lastp ){
						strcat_s( newmqomat->tex, 256, ".tga" );
					}
					WCHAR wname[256];
					ZeroMemory( wname, sizeof( WCHAR ) * 256 );
					MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, newmqomat->tex, 256, wname, 256 );

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
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if(lTexture)
                {
                    char* nameptr = (char*)lTexture->GetName();
					if( nameptr ){
						char tempname[256];
						strcpy_s( tempname, 256, nameptr );
						char* lastslash = strrchr( tempname, '/' );
						if( !lastslash ){
							lastslash = strrchr( tempname, '\\' );
						}
						if( lastslash ){
							strcpy_s( newmqomat->tex, 256, lastslash + 1 );
						}else{
							strcpy_s( newmqomat->tex, 256, tempname );
						}
						char* lastp = strrchr( newmqomat->tex, '.' );
						if( !lastp ){
							strcat_s( newmqomat->tex, 256, ".tga" );
						}

						WCHAR wname[256];
						ZeroMemory( wname, sizeof( WCHAR ) * 256 );
						MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, newmqomat->tex, 256, wname, 256 );

DbgOut( L"SetMQOMaterial : texture %s\r\n", wname );

						break;
					}
                }
            }
        }
    }


   return 0;
}

// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
    const char * pPropertyName,
    const char * pFactorPropertyName,
    char** ppTextureName)
{
	*ppTextureName = 0;

    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
    if (lProperty.IsValid() && lFactorProperty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }


    return lResult;
}



int CModel::CreateFBXBoneReq( FbxScene* pScene, FbxNode* pNode, FbxNode* parnode )
{
//	EFbxRotationOrder lRotationOrder0 = eEulerZXY;
//	EFbxRotationOrder lRotationOrder1 = eEulerXYZ;

	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();
		const char* nodename = pNode->GetName();
		WCHAR wname[256];
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, nodename, -1, wname, 256 );

		FbxNode* parbonenode = 0;
		if( parnode ){
			FbxNodeAttribute *parattr = parnode->GetNodeAttribute();
			if ( parattr ) {
				FbxNodeAttribute::EType partype = parattr->GetAttributeType();
				switch ( partype )
				{
					case FbxNodeAttribute::eSkeleton:
					case FbxNodeAttribute::eNull:
						parbonenode = parnode;
						break;
					default:
						parbonenode = 0;
						break;
				}
			}
		}

		switch ( type )
		{
			case FbxNodeAttribute::eSkeleton:
			case FbxNodeAttribute::eNull:
	
				//EFbxRotationOrder lRotationOrder = eEULER_ZXY;
				//pNode->SetRotationOrder(FbxNode::eSourcePivot , lRotationOrder0 );
				//pNode->SetRotationOrder(FbxNode::eDestinationPivot , lRotationOrder1 );
				
				DbgOut( L"CreateFBXBoneReq : skeleton : %s\r\n", wname );
				GetFBXBone( pScene, type, pAttrib, nodename, pNode, parbonenode );
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
		CreateFBXBoneReq( pScene, pChild, pNode );
	}

	return 0;
}

int CModel::GetFBXBone( FbxScene* pScene, FbxNodeAttribute::EType type, FbxNodeAttribute *pAttrib, const char* nodename, FbxNode* curnode, FbxNode* parnode )
{
	int settopflag = 0;
	CBone* newbone = new CBone( this );
	_ASSERT( newbone );
	newbone->SetName( (char*)nodename );
	newbone->m_topboneflag = 0;
	m_bonelist[newbone->m_boneno] = newbone;
	m_bonename[ newbone->m_bonename ] = newbone;

	if( type == FbxNodeAttribute::eSkeleton ){
		newbone->m_type = FBXBONE_NORMAL;
	}else if( type == FbxNodeAttribute::eNull ){
		newbone->m_type = FBXBONE_NULL;
	}else{
		_ASSERT( 0 );
	}

	if( !parnode ){
		m_firstbone = curnode;
	}
	if( !m_topbone ){
		m_topbone = newbone;
		settopflag = 1;
	}else{
		m_bone2node[newbone] = curnode;
	}

	EFbxRotationOrder lRotationOrder0;
	curnode->GetRotationOrder (FbxNode::eSourcePivot, lRotationOrder0);
	EFbxRotationOrder lRotationOrder1 = eEulerXYZ;
	curnode->SetRotationOrder(FbxNode::eDestinationPivot , lRotationOrder1 );

	if( parnode ){
		const char* parbonename = parnode->GetName();
		CBone* parbone = m_bonename[ parbonename ];
		if( parbone ){
			parbone->AddChild( newbone );
		}else{
			/***
			FbxNodeAttribute *parattr = parnode->GetNodeAttribute();
			if( parattr ){
				FbxNodeAttribute::EType type = parattr->GetAttributeType();
				if( type == FbxNodeAttribute::eSkeleton ){
					_ASSERT( 0 );
				}else if( type == FbxNodeAttribute::eNull ){
					_ASSERT( 0 );
				}else{
					_ASSERT( 0 );
				}
			}else{
				_ASSERT( 0 );
			}
			***/
			::MessageBoxA( NULL, "GetFBXBone : parbone NULL error ", parbonename, MB_OK );
		}
	}else{
		if( settopflag == 0 ){
			_ASSERT( 0 );
			m_topbone->AddChild( newbone );
		}else{
			//::MessageBoxA( NULL, "GetFBXBone : parbone NULL error ", nodename, MB_OK );
		}
	}

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


int CModel::CreateFBXAnim( FbxScene* pScene, FbxNode* prootnode )
{
	static int s_dbgcnt = 0;
	s_dbgcnt++;

	pScene->FillAnimStackNameArray(mAnimStackNameArray);
    const int lAnimStackCount = mAnimStackNameArray.GetCount();

	DbgOut( L"FBX anim num %d\r\n", lAnimStackCount );

	if( lAnimStackCount <= 0 ){
		_ASSERT( 0 );
		return 0;
	}

	int animno;
	for( animno = 0; animno < lAnimStackCount; animno++ ){
		// select the base layer from the animation stack
		//char* animname = mAnimStackNameArray[animno]->Buffer();
		//MessageBoxA( NULL, animname, "check", MB_OK );
		FbxAnimStack * lCurrentAnimationStack = m_pscene->FindMember<FbxAnimStack>(mAnimStackNameArray[animno]->Buffer());
		if (lCurrentAnimationStack == NULL){
			_ASSERT( 0 );
			return 1;
		}
		FbxAnimLayer * mCurrentAnimLayer;
		mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();

		m_animevaluator = pScene->GetEvaluator();
		//m_animevaluator->SetContext(lCurrentAnimationStack);
		pScene->GetEvaluator()->SetContext(lCurrentAnimationStack);


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
			_ASSERT( 0 );
			// Take the time line value
			FbxTimeSpan lTimeLineTimeSpan;
			pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
			mStart = lTimeLineTimeSpan.GetStart();
			mStop  = lTimeLineTimeSpan.GetStop();
		}


//		int animleng = (int)mStop.GetFrame();// - mStart.GetFrame() + 1;		
		//mFrameTime.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
		//mFrameTime2.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
		mFrameTime.SetSecondDouble( 1.0 / 30.0 );
		mFrameTime2.SetSecondDouble( 1.0 / 30.0 );
		//mFrameTime.SetSecondDouble( 1.0 / 300.0 );
		//mFrameTime2.SetSecondDouble( 1.0 / 300.0 );
	


//		int fcnt = 0;
//		FbxTime chktime;
//		for( chktime = mStart; chktime < mStop; chktime += mFrameTime ){
//			fcnt++;
//		}
//		int animleng = fcnt;

		int animleng = (int)( (mStop.GetSecondDouble() - mStart.GetSecondDouble()) * 30.0 );
//		int animleng = (int)( (mStop.GetSecondDouble() - mStart.GetSecondDouble()) * 300.0 );
//		_ASSERT( 0 );

//char mes[256];
//sprintf_s( mes, 256, "%d", animleng );
//MessageBoxA( NULL, mes, "check", MB_OK );


		//_ASSERT( 0 );


		DbgOut( L"FBX anim %d, animleng %d\r\n", animno, animleng );


		if( animno == 0 ){
			SetDefaultBonePos();
		}


		int curmotid = -1;
		AddMotion( mAnimStackNameArray[animno]->Buffer(), 0, (double)animleng, &curmotid );


		map<int,CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* curbone = itrbone->second;
			if( curbone ){
				curbone->m_getanimflag = 0;
			}
		}


        //FbxPose* pPose = pScene->GetPose( animno );
		//FbxPose* pPose = pScene->GetPose( 10 );
		FbxPose* pPose = NULL;

		CreateFBXAnimReq( animno, pPose, prootnode, curmotid, animleng, mStart, mFrameTime2 );	

		FillUpEmptyKeyReq( curmotid, animleng, m_topbone, 0 );

		if( animno == 0 ){
			CallF( CreateFBXShape( mCurrentAnimLayer, animleng, mStart, mFrameTime2 ), return 1 );
		}

		(this->m_tlFunc)( curmotid );

	}

	return 0;
}

int CModel::CreateFBXAnimReq( int animno, FbxPose* pPose, FbxNode* pNode, int motid, int animleng, FbxTime mStart, FbxTime mFrameTime )
{
	//static int dbgcnt = 0;

	//int lSkinCount;

	FbxNodeAttribute *pAttrib = pNode->GetNodeAttribute();
	if ( pAttrib ) {
		FbxNodeAttribute::EType type = pAttrib->GetAttributeType();

		switch ( type )
		{
			case FbxNodeAttribute::eMesh:
//			case FbxNodeAttribute::eNURB:
//			case FbxNodeAttribute::eNURBS_SURFACE:
				GetFBXAnim( animno, pNode, pPose, pAttrib, motid, animleng, mStart, mFrameTime );     // メッシュを作成

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
		CreateFBXAnimReq( animno, pPose, pChild, motid, animleng, mStart, mFrameTime );
	}

	return 0;
}
int CModel::GetFBXAnim( int animno, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute *pAttrib, int motid, int animleng, FbxTime mStart, FbxTime mFrameTime )
{
	FbxAMatrix pGlobalPosition;
	pGlobalPosition.SetIdentity();


	FbxMesh *pMesh = (FbxMesh*)pAttrib;


	// スキンの数を取得
	int skinCount  = pMesh->GetDeformerCount( FbxDeformer::eSkin );

	for ( int i = 0; i < skinCount; ++i ) {
		// i番目のスキンを取得
		FbxSkin* skin = (FbxSkin*)( pMesh->GetDeformer( i, FbxDeformer::eSkin ) );

		// クラスターの数を取得
		int clusterNum = skin->GetClusterCount();

		for ( int j = 0; j < clusterNum; ++j ) {
			// j番目のクラスタを取得
			FbxCluster* cluster = skin->GetCluster( j );

			const char* bonename = ((FbxNode*)cluster->GetLink())->GetName();
			CBone* curbone = m_bonename[ (char*)bonename ];
			if( curbone && !curbone->m_getanimflag){
				curbone->m_getanimflag = 1;


				FbxAMatrix mat;
				FbxTime ktime = mStart;
				int framecnt;
				for( framecnt = 0; framecnt < animleng; framecnt++ ){
					FbxCluster::ELinkMode lClusterMode = cluster->GetLinkMode();

					/***
					if( framecnt <= 2 ){
						int pointNum = cluster->GetControlPointIndicesCount();
						int* pointAry = cluster->GetControlPointIndices();
						double* weightAry = cluster->GetControlPointWeights();

						FbxCluster::ELinkMode lClusterMode = cluster->GetLinkMode();
						int index;
						float weight;
						for ( int i2 = 0; i2 < pointNum; i2++ ) {
							// 頂点インデックスとウェイトを取得
							index  = pointAry[ i2 ];
							weight = (float)weightAry[ i2 ];

							if( lClusterMode == FbxCluster::eAdditive ){
								DbgOut( L"clustermodecheck : bonename %s, framecnt %d, clastermode Additive, index %d, weight %f\r\n", curbone->m_wbonename, framecnt, index, weight );
							}else if( lClusterMode == FbxCluster::eNormalize ){
								DbgOut( L"clustermodecheck : bonename %s, framecnt %d, clastermode Normalize, index %d, weight %f\r\n", curbone->m_wbonename, framecnt, index, weight );
							}else if( lClusterMode == FbxCluster::eTotalOne ){
								DbgOut( L"clustermodecheck : bonename %s, framecnt %d, clastermode eTotalOne, index %d, weight %f\r\n", curbone->m_wbonename, framecnt, index, weight );
							}else{
								DbgOut( L"clustermodecheck : bonename %s, framecnt %d, clastermode %d, index %d, weight %f\r\n", curbone->m_wbonename, framecnt, lClusterMode, index, weight );
							}
						}
					}
					***/

					FbxAMatrix lReferenceGlobalInitPosition;
					FbxAMatrix lReferenceGlobalCurrentPosition;
					FbxAMatrix lAssociateGlobalInitPosition;
					FbxAMatrix lAssociateGlobalCurrentPosition;
					FbxAMatrix lClusterGlobalInitPosition;
					FbxAMatrix lClusterGlobalCurrentPosition;

					FbxAMatrix lReferenceGeometry;
					FbxAMatrix lAssociateGeometry;
					FbxAMatrix lClusterGeometry;

					FbxAMatrix lClusterRelativeInitPosition;
					FbxAMatrix lClusterRelativeCurrentPositionInverse;
	
/***
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//					if (lClusterMode == FbxCluster::eAdditive && cluster->GetAssociateModel())
//					if( ( (lClusterMode == FbxCluster::eAdditive) || (lClusterMode == FbxCluster::eTotalOne) ) && cluster->GetAssociateModel() )
***/
/***
					if (lClusterMode == FbxCluster::eAdditive && cluster->GetAssociateModel())
					{
						cluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
						// Geometric transform of the model
						lAssociateGeometry = GetGeometry(cluster->GetAssociateModel());
						lAssociateGlobalInitPosition *= lAssociateGeometry;
						lAssociateGlobalCurrentPosition = GetGlobalPosition(cluster->GetAssociateModel(), ktime, pPose);

						cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
						// Multiply lReferenceGlobalInitPosition by Geometric Transformation
						lReferenceGeometry = GetGeometry(pMesh->GetNode());
						lReferenceGlobalInitPosition *= lReferenceGeometry;
						lReferenceGlobalCurrentPosition = pGlobalPosition;

						// Get the link initial global position and the link current global position.
						cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
						// Multiply lClusterGlobalInitPosition by Geometric Transformation
						lClusterGeometry = GetGeometry(cluster->GetLink());
						lClusterGlobalInitPosition *= lClusterGeometry;
						lClusterGlobalCurrentPosition = GetGlobalPosition(cluster->GetLink(), ktime, pPose);

						// Compute the shift of the link relative to the reference.
						//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
						mat = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
							lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
					}
					else
					{
						cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
						lReferenceGlobalCurrentPosition = pGlobalPosition;
						// Multiply lReferenceGlobalInitPosition by Geometric Transformation
						lReferenceGeometry = GetGeometry(pMesh->GetNode());
						lReferenceGlobalInitPosition *= lReferenceGeometry;

						// Get the link initial global position and the link current global position.
						cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
						lClusterGlobalCurrentPosition = GetGlobalPosition(cluster->GetLink(), ktime, pPose);

						// Compute the initial position of the link relative to the reference.
						lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

						// Compute the current position of the link relative to the reference.
						lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

						// Compute the shift of the link relative to the reference.
						mat = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
					}
***/

					cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
					lReferenceGlobalCurrentPosition = pGlobalPosition;
					// Multiply lReferenceGlobalInitPosition by Geometric Transformation
					lReferenceGeometry = GetGeometry(pMesh->GetNode());
					lReferenceGlobalInitPosition *= lReferenceGeometry;

					// Get the link initial global position and the link current global position.
					cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
					lClusterGlobalCurrentPosition = GetGlobalPosition(cluster->GetLink(), ktime, pPose);

					// Compute the initial position of the link relative to the reference.
					lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

					// Compute the current position of the link relative to the reference.
					lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

					// Compute the shift of the link relative to the reference.
					mat = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;


					D3DXMATRIX xmat;
					xmat._11 = (float)mat.Get( 0, 0 );
					xmat._12 = (float)mat.Get( 0, 1 );
					xmat._13 = (float)mat.Get( 0, 2 );
					xmat._14 = (float)mat.Get( 0, 3 );

					xmat._21 = (float)mat.Get( 1, 0 );
					xmat._22 = (float)mat.Get( 1, 1 );
					xmat._23 = (float)mat.Get( 1, 2 );
					xmat._24 = (float)mat.Get( 1, 3 );

					xmat._31 = (float)mat.Get( 2, 0 );
					xmat._32 = (float)mat.Get( 2, 1 );
					xmat._33 = (float)mat.Get( 2, 2 );
					xmat._34 = (float)mat.Get( 2, 3 );

					xmat._41 = (float)mat.Get( 3, 0 );
					xmat._42 = (float)mat.Get( 3, 1 );
					xmat._43 = (float)mat.Get( 3, 2 );
					xmat._44 = (float)mat.Get( 3, 3 );

					if( (animno == 0) && (framecnt == 0) ){
						curbone->m_firstmat = xmat;
						D3DXMatrixInverse( &(curbone->m_invfirstmat), NULL, &xmat );
						D3DXMATRIX calcmat = curbone->m_nodemat * curbone->m_invfirstmat;
						D3DXVECTOR3 zeropos( 0.0f, 0.0f, 0.0f );
						D3DXVec3TransformCoord( &(curbone->m_jointfpos), &zeropos, &calcmat );


//						D3DXMATRIX calcmat = curbone->m_invfirstmat;
//						D3DXVECTOR3 zeropos = curbone->m_jointwpos;
//						D3DXVec3TransformCoord( &(curbone->m_jointfpos), &zeropos, &calcmat );


//char* chk;
//chk = strstr( (char*)bonename, "End_Right_Toe" );
//if( chk ){
//	_ASSERT( 0 );
//}

					}

					CMotionPoint* curmp = 0;
					int existflag = 0;
					curmp = curbone->AddMotionPoint( motid, (double)(framecnt), 0, &existflag );
					if( !curmp ){
						_ASSERT( 0 );
						return 1;
					}
					curmp->m_worldmat = xmat;

					ktime += mFrameTime;
				}
			}
		}
	}


	return 0;
}

int CModel::CreateFBXSkinReq( FbxNode* pNode )
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

	return 0;
}
int CModel::GetFBXSkin( FbxNodeAttribute *pAttrib, FbxNode* pNode )
{
	const char* nodename = pNode->GetName();

	FbxMesh *pMesh = (FbxMesh*)pAttrib;
	CMQOObject* newobj = 0;
	newobj = m_objectname[ nodename ];
	if( !newobj ){
		_ASSERT( 0 );
		return 1;
	}

//スキン
	// スキンの数を取得
	int skinCount  = pMesh->GetDeformerCount( FbxDeformer::eSkin );

	int makecnt = 0;
	for ( int i = 0; i < skinCount; ++i ) {
		// i番目のスキンを取得
		FbxSkin* skin = (FbxSkin*)( pMesh->GetDeformer( i, FbxDeformer::eSkin ) );

		// クラスターの数を取得
		int clusterNum = skin->GetClusterCount();
DbgOut( L"fbx : skin : org clusternum %d\r\n", clusterNum );

		for ( int j = 0; j < clusterNum; ++j ) {
			// j番目のクラスタを取得
			FbxCluster* cluster = skin->GetCluster( j );

			int validflag = IsValidCluster( cluster );
			if( validflag == 0 ){
				continue;
			}

			const char* bonename = ((FbxNode*)cluster->GetLink())->GetName();
//			int namelen = (int)strlen( clustername );
			WCHAR wname[256];
			ZeroMemory( wname, sizeof( WCHAR ) * 256 );
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, bonename, -1, wname, 256 );
//			DbgOut( L"cluster (%d, %d), name : %s\r\n", i, j, wname );

			CBone* curbone = m_bonename[ (char*)bonename ];

//char* chkptr = strstr( (char*)bonename, "End_Right_Toe" );
//if( chkptr ){
//	_ASSERT( 0 );
//}

			if( curbone ){
				int curclusterno = newobj->m_cluster.size();

				if( curclusterno >= MAXCLUSTERNUM ){
					WCHAR wmes[256];
					swprintf_s( wmes, 256, L"１つのパーツに影響できるボーンの制限数(%d個)を超えました。読み込めません。", MAXCLUSTERNUM );
					MessageBoxW( NULL, wmes, L"ボーン数エラー", MB_OK );
					_ASSERT( 0 );
					return 1;
				}


				newobj->m_cluster.push_back( curbone );

				int pointNum = cluster->GetControlPointIndicesCount();
				int* pointAry = cluster->GetControlPointIndices();
				double* weightAry = cluster->GetControlPointWeights();

				FbxCluster::ELinkMode lClusterMode = cluster->GetLinkMode();
				int index;
				float weight;
				for ( int i2 = 0; i2 < pointNum; i2++ ) {
					// 頂点インデックスとウェイトを取得
					index  = pointAry[ i2 ];
					weight = (float)weightAry[ i2 ];

					int isadditive;
					if( lClusterMode == FbxCluster::eAdditive ){
						isadditive = 1;
					}else{
						isadditive = 0;
					}

					if( (lClusterMode == FbxCluster::eAdditive) || (weight >= 0.05f) ){
						newobj->AddInfBone( curclusterno, index, weight, isadditive );
					}
				}

//if( chkptr ){
//	_ASSERT( 0 );
//}

				FbxAMatrix initMat;
				cluster->GetTransformLinkMatrix( initMat );

				curbone->m_initmat._11 = (float)initMat.Get( 0, 0 );
				curbone->m_initmat._12 = (float)initMat.Get( 0, 1 );
				curbone->m_initmat._13 = (float)initMat.Get( 0, 2 );
				curbone->m_initmat._14 = (float)initMat.Get( 0, 3 );

				curbone->m_initmat._21 = (float)initMat.Get( 1, 0 );
				curbone->m_initmat._22 = (float)initMat.Get( 1, 1 );
				curbone->m_initmat._23 = (float)initMat.Get( 1, 2 );
				curbone->m_initmat._24 = (float)initMat.Get( 1, 3 );

				curbone->m_initmat._31 = (float)initMat.Get( 2, 0 );
				curbone->m_initmat._32 = (float)initMat.Get( 2, 1 );
				curbone->m_initmat._33 = (float)initMat.Get( 2, 2 );
				curbone->m_initmat._34 = (float)initMat.Get( 2, 3 );

				curbone->m_initmat._41 = (float)initMat.Get( 3, 0 );
				curbone->m_initmat._42 = (float)initMat.Get( 3, 1 );
				curbone->m_initmat._43 = (float)initMat.Get( 3, 2 );
				curbone->m_initmat._44 = (float)initMat.Get( 3, 3 );

				D3DXMatrixInverse( &curbone->m_invinitmat, NULL, &curbone->m_initmat );

DbgOut( L"bone %s : initmat : (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f)\r\n",
	wname, 
	curbone->m_initmat._11, curbone->m_initmat._12, curbone->m_initmat._13, curbone->m_initmat._14,
	curbone->m_initmat._21, curbone->m_initmat._22, curbone->m_initmat._23, curbone->m_initmat._24,
	curbone->m_initmat._31, curbone->m_initmat._32, curbone->m_initmat._33, curbone->m_initmat._34,
	curbone->m_initmat._41, curbone->m_initmat._42, curbone->m_initmat._43, curbone->m_initmat._44
	);

/***				
				FbxAMatrix gpos;
				FbxNodeEvalState es( pNode );
				gpos = es.mGX;

				//gpos = pNode->EvaluateGlobalTransform( m_ktime0 );

				curbone->m_nodemat._11 = (float)gpos.Get( 0, 0 );
				curbone->m_nodemat._12 = (float)gpos.Get( 0, 1 );
				curbone->m_nodemat._13 = (float)gpos.Get( 0, 2 );
				curbone->m_nodemat._14 = (float)gpos.Get( 0, 3 );

				curbone->m_nodemat._21 = (float)gpos.Get( 1, 0 );
				curbone->m_nodemat._22 = (float)gpos.Get( 1, 1 );
				curbone->m_nodemat._23 = (float)gpos.Get( 1, 2 );
				curbone->m_nodemat._24 = (float)gpos.Get( 1, 3 );

				curbone->m_nodemat._31 = (float)gpos.Get( 2, 0 );
				curbone->m_nodemat._32 = (float)gpos.Get( 2, 1 );
				curbone->m_nodemat._33 = (float)gpos.Get( 2, 2 );
				curbone->m_nodemat._34 = (float)gpos.Get( 2, 3 );

				curbone->m_nodemat._41 = (float)gpos.Get( 3, 0 );
				curbone->m_nodemat._42 = (float)gpos.Get( 3, 1 );
				curbone->m_nodemat._43 = (float)gpos.Get( 3, 2 );
				curbone->m_nodemat._44 = (float)gpos.Get( 3, 3 );
***/
				makecnt++;

			}else{
				_ASSERT( 0 );
			}

		}

		newobj->NormalizeInfBone();


	}

	DbgOut( L"fbx skin : make cluster %d\r\n", makecnt );


	return 0;
}


int IsValidCluster( FbxCluster* cluster )
{
	int findflag = 0;

	int pointNum = cluster->GetControlPointIndicesCount();
	int* pointAry = cluster->GetControlPointIndices();
	double* weightAry = cluster->GetControlPointWeights();

	FbxCluster::ELinkMode lClusterMode = cluster->GetLinkMode();

	int index;
	double weight;
	for ( int i2 = 0; i2 < pointNum; i2++ ) {
		// 頂点インデックスとウェイトを取得
		index  = pointAry[ i2 ];
		weight = weightAry[ i2 ];

		if( (lClusterMode == FbxCluster::eAdditive) || (weight >= 0.05) ){
			findflag = 1;
			break;
		}
	}

	return findflag;
}

int CModel::RenderBoneMark( LPDIRECT3DDEVICE9 pdev, CModel* bmarkptr, CMySprite* bcircleptr, CModel* cpslptr[COL_MAX], int selboneno )
{
	if( m_bonelist.empty() ){
		return 0;
	}

	map<int, CBone*>::iterator itrb;
	for( itrb = m_bonelist.begin(); itrb != m_bonelist.end(); itrb++ ){
		CBone* curbone = itrb->second;
		if( curbone ){
			curbone->m_selectflag = 0;
		}
	}

	if( selboneno > 0 ){
		CBone* selbone = m_bonelist[ selboneno ];
		if( selbone ){
			SetSelectFlagReq( selbone, 0 );
			selbone->m_selectflag = 2;
	
			CBone* parbone = selbone->m_parent;
			if( parbone ){
//				parbone->m_selectflag = 2;

				CBtObject* curbto = FindBtObject( selbone->m_boneno );
				if( curbto ){
					parbone->m_selectflag += 4;
				}
			}
		}
	}

	pdev->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
/***
	if( bmarkptr ){
		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* boneptr = itrbone->second;
			if( boneptr ){
				CBone* parptr = boneptr->m_parent;
				if( boneptr && parptr ){
					D3DXMATRIX wmat = srcstartmap->m_curmp[boneptr]->m_worldmat;
					D3DXMATRIX parmat = srcstartmap->m_curmp[parptr]->m_worldmat;

					D3DXVECTOR3 aftbonepos;
					D3DXVec3TransformCoord( &aftbonepos, &parptr->m_jointfpos, &parmat );

					D3DXVECTOR3 aftchilpos;
					D3DXVec3TransformCoord( &aftchilpos, &boneptr->m_jointfpos, &wmat );

					boneptr->CalcAxisMatZ( &aftbonepos, &aftchilpos );
					D3DXMATRIX bmmat;
					bmmat = boneptr->m_laxismat;// * boneptr->m_curmp.m_worldmat;

					D3DXVECTOR3 diffvec = aftchilpos - aftbonepos;
					float diffleng = D3DXVec3Length( &diffvec );
					
					float fscale;
					D3DXMATRIX scalemat;
					D3DXMatrixIdentity( &scalemat );
					fscale = diffleng / 50.0f;
					scalemat._11 = fscale;
					scalemat._22 = fscale;
					scalemat._33 = fscale;

					bmmat = scalemat * bmmat;

					bmmat._41 = aftbonepos.x;
					bmmat._42 = aftbonepos.y;
					bmmat._43 = aftbonepos.z;


					g_pEffect->SetMatrix( g_hmWorld, &bmmat );
					bmarkptr->UpdateMatrix( 0, -1, &bmmat, &m_matVP );

					D3DXVECTOR4 difmult;
					if( boneptr->m_selectflag == 2 ){
						difmult = D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.5f );
					}else{
						difmult = D3DXVECTOR4( 0.25f, 0.5f, 0.5f, 0.5f );
					}
					CallFnc( bmarkptr->OnRender( pdev, 0, difmult, 0, 0 ), return 1 );
				}				
			}
		}
	}
***/

	if( bmarkptr ){
		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* boneptr = itrbone->second;
			if( boneptr ){
				if( boneptr->m_child ){

					D3DXVECTOR3 aftbonepos;
					D3DXVec3TransformCoord( &aftbonepos, &boneptr->m_jointfpos, &(boneptr->m_curmp.m_worldmat) );

					D3DXVECTOR3 aftchilpos;
					D3DXVec3TransformCoord( &aftchilpos, &boneptr->m_child->m_jointfpos, &(boneptr->m_child->m_curmp.m_worldmat) );


					boneptr->CalcAxisMatZ( &aftbonepos, &aftchilpos );

					D3DXMATRIX bmmat;
					bmmat = boneptr->m_laxismat;// * boneptr->m_curmp.m_worldmat;


					D3DXVECTOR3 diffvec = aftchilpos - aftbonepos;
					float diffleng = D3DXVec3Length( &diffvec );
					
					float fscale;
					D3DXMATRIX scalemat;
					D3DXMatrixIdentity( &scalemat );
					fscale = diffleng / 50.0f;
					scalemat._11 = fscale;
					scalemat._22 = fscale;
					scalemat._33 = fscale;

					bmmat = scalemat * bmmat;

					bmmat._41 = aftbonepos.x;
					bmmat._42 = aftbonepos.y;
					bmmat._43 = aftbonepos.z;


					g_pEffect->SetMatrix( g_hmWorld, &bmmat );
					bmarkptr->UpdateMatrix( -1, &bmmat, &m_matVP );
					D3DXVECTOR4 difmult;
					if( boneptr->m_selectflag == 2 ){
						difmult = D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.5f );
					}else{
						difmult = D3DXVECTOR4( 0.25f, 0.5f, 0.5f, 0.5f );
					}
					CallF( bmarkptr->OnRender( pdev, 0, difmult ), return 1 );
				}				
			}
		}
	}


	if( cpslptr ){

		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* boneptr = itrbone->second;
			if( boneptr ){
				map<CBone*,CRigidElem*>::iterator itrre;
				for( itrre = boneptr->m_rigidelem.begin(); itrre != boneptr->m_rigidelem.end(); itrre++ ){
					CRigidElem* curre = itrre->second;
					if( curre && (curre->m_skipflag != 1) ){
						CBone* chilbone = itrre->first;
						_ASSERT( chilbone );

						CModel* curcoldisp = cpslptr[curre->m_coltype];
						_ASSERT( curcoldisp );

//DbgOut( L"check!!!: curbone %s, chilbone %s\r\n", boneptr->m_wbonename, chilbone->m_wbonename );

						boneptr->CalcRigidElemParams( cpslptr, chilbone, 0 );

						g_pEffect->SetMatrix( g_hmWorld, &(curre->m_capsulemat) );
						curcoldisp->UpdateMatrix( -1, &(curre->m_capsulemat), &m_matVP );
						D3DXVECTOR4 difmult;
						if( boneptr->m_selectflag & 4 ){
							difmult = D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.5f );
						}else{
							difmult = D3DXVECTOR4( 0.25f, 0.5f, 0.5f, 0.5f );
						}
						CallF( curcoldisp->OnRender( pdev, 0, difmult ), return 1 );
					}
				}
			}
		}
	}


	if( bcircleptr ){
		map<int, CBone*>::iterator itrbone;
		for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
			CBone* boneptr = itrbone->second;
			if( boneptr && (boneptr->m_type == FBXBONE_NORMAL) ){

				D3DXMATRIX bcmat;
				bcmat = boneptr->m_curmp.m_worldmat;
				CBone* parbone = boneptr->m_parent;
				CBone* chilbone = boneptr->m_child;
				//if( parbone && !chilbone ){
				//	bcmat = parbone->m_invfirstmat * parbone->m_curmp.m_worldmat;
				//}

				//D3DXMATRIX transmat = boneptr->m_invinitmat * boneptr->m_nodemat * m_matVP;
				//D3DXMATRIX transmat = boneptr->m_nodemat * m_matVP;
				//D3DXMATRIX transmat = b0mat * bcmat * m_matVP;
				D3DXMATRIX transmat = bcmat * m_matVP;
				//D3DXMATRIX transmat = m_matVP;
				D3DXVECTOR3 scpos;
				//D3DXVECTOR3 firstpos = boneptr->m_jointwpos;
				D3DXVECTOR3 firstpos = boneptr->m_jointfpos;
				//float w = boneptr->m_nodemat._44;
				//D3DXVECTOR3 firstpos = D3DXVECTOR3( boneptr->m_nodemat._41 / w, boneptr->m_nodemat._42 / w, boneptr->m_nodemat._43 / w );
				//D3DXVECTOR3 firstpos = boneptr->m_vertpos[BT_CHILD];
				//D3DXVECTOR3 firstpos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
				//D3DXVECTOR3 firstpos = D3DXVECTOR3( boneptr->m_initmat._41, boneptr->m_initmat._42, boneptr->m_initmat._43 );
				D3DXVec3TransformCoord( &scpos, &firstpos, &transmat );
				scpos.z = 0.0f;
				bcircleptr->SetPos( scpos );
				D3DXVECTOR2 bsize;
				if( boneptr->m_selectflag & 2 ){
					bcircleptr->SetColor( D3DXVECTOR4( 0.0f, 0.0f, 1.0f, 0.7f ) );
					bsize = D3DXVECTOR2( 0.050f, 0.050f );
					bcircleptr->SetSize( bsize );
				}else if( boneptr->m_selectflag & 1 ){
					bcircleptr->SetColor( D3DXVECTOR4( 1.0f, 0.0f, 0.0f, 0.7f ) );
					bsize = D3DXVECTOR2( 0.025f, 0.025f );
					bcircleptr->SetSize( bsize );
				}else{
					bcircleptr->SetColor( D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 0.7f ) );
					bsize = D3DXVECTOR2( 0.025f, 0.025f );
					bcircleptr->SetSize( bsize );
				}
				CallF( bcircleptr->OnRender(), return 1 );

			}
		}
	}

	pdev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	return 0;
}
void CModel::SetDefaultBonePosReq( CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition )
{
	FbxNode* pNode = m_bone2node[ curbone ];


	FbxAMatrix lGlobalPosition;
	bool        lPositionFound = false;

	if( pPose ){
		int lNodeIndex = pPose->Find(pNode);
		if (lNodeIndex > -1)
		{
			// The bind pose is always a global matrix.
			// If we have a rest pose, we need to check if it is
			// stored in global or local space.
			if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
			{
				lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
			}
			else
			{
				// We have a local matrix, we need to convert it to
				// a global space matrix.
				FbxAMatrix lParentGlobalPosition;

				if (pParentGlobalPosition)
				{
					lParentGlobalPosition = *pParentGlobalPosition;
				}
				else
				{
					if (pNode->GetParent())
					{
						lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
					}
				}

				FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
				lGlobalPosition = lParentGlobalPosition * lLocalPosition;
 			}

			lPositionFound = true;
		}
	}

	if (!lPositionFound)
	{
		// There is no pose entry for that node, get the current global position instead.

		// Ideally this would use parent global position and local position to compute the global position.
		// Unfortunately the equation 
		//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
		// does not hold when inheritance type is other than "Parent" (RSrs).
		// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
		lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
	}

	curbone->m_nodemat._11 = (float)lGlobalPosition.Get( 0, 0 );
	curbone->m_nodemat._12 = (float)lGlobalPosition.Get( 0, 1 );
	curbone->m_nodemat._13 = (float)lGlobalPosition.Get( 0, 2 );
	curbone->m_nodemat._14 = (float)lGlobalPosition.Get( 0, 3 );

	curbone->m_nodemat._21 = (float)lGlobalPosition.Get( 1, 0 );
	curbone->m_nodemat._22 = (float)lGlobalPosition.Get( 1, 1 );
	curbone->m_nodemat._23 = (float)lGlobalPosition.Get( 1, 2 );
	curbone->m_nodemat._24 = (float)lGlobalPosition.Get( 1, 3 );

	curbone->m_nodemat._31 = (float)lGlobalPosition.Get( 2, 0 );
	curbone->m_nodemat._32 = (float)lGlobalPosition.Get( 2, 1 );
	curbone->m_nodemat._33 = (float)lGlobalPosition.Get( 2, 2 );
	curbone->m_nodemat._34 = (float)lGlobalPosition.Get( 2, 3 );

	curbone->m_nodemat._41 = (float)lGlobalPosition.Get( 3, 0 );
	curbone->m_nodemat._42 = (float)lGlobalPosition.Get( 3, 1 );
	curbone->m_nodemat._43 = (float)lGlobalPosition.Get( 3, 2 );
	curbone->m_nodemat._44 = (float)lGlobalPosition.Get( 3, 3 );

	curbone->m_globalpos = lGlobalPosition;

	D3DXVECTOR3 zeropos( 0.0f, 0.0f, 0.0f );
	D3DXVec3TransformCoord( &(curbone->m_jointwpos), &zeropos, &(curbone->m_nodemat) );

	curbone->m_jointfpos = curbone->m_jointwpos;


	if( curbone->m_child ){
		SetDefaultBonePosReq( curbone->m_child, pTime, pPose, &curbone->m_globalpos );
	}
	if( curbone->m_brother ){
		SetDefaultBonePosReq( curbone->m_brother, pTime, pPose, pParentGlobalPosition );
	}

}


int CModel::SetDefaultBonePos()
{
	if( !m_topbone ){
		return 0;
	}

	FbxPose* bindpose = 0;
	FbxPose* curpose = m_pscene->GetPose( 0 );
	int curpindex = 1;
	while( curpose ){
		if( curpose->IsBindPose() ){
			bindpose = curpose;
			break;
		}
		curpose = m_pscene->GetPose( curpindex );
		curpindex++;
	}

	if( !bindpose ){
		::MessageBoxA( NULL, "バインドポーズがありません。", "警告", MB_OK );
		bindpose = m_pscene->GetPose( 0 );
	}

	FbxTime pTime;
	pTime.SetSecondDouble( 0.0 );

	CBone* secbone = m_topbone->m_child;

	if( secbone ){
		SetDefaultBonePosReq( secbone, pTime, bindpose, 0 );
	}

	return 0;
}

FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition;
    bool        lPositionFound = false;

    if (pPose)
    {
        int lNodeIndex = pPose->Find(pNode);

        if (lNodeIndex > -1)
        {
            // The bind pose is always a global matrix.
            // If we have a rest pose, we need to check if it is
            // stored in global or local space.
            if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
            {
                lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
            }
            else
            {
                // We have a local matrix, we need to convert it to
                // a global space matrix.
                FbxAMatrix lParentGlobalPosition;

                if (pParentGlobalPosition)
                {
                    lParentGlobalPosition = *pParentGlobalPosition;
                }
                else
                {
                    if (pNode->GetParent())
                    {
                        lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
                    }
                }

                FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
                lGlobalPosition = lParentGlobalPosition * lLocalPosition;
            }

            lPositionFound = true;
        }
    }

    if (!lPositionFound)
    {
        // There is no pose entry for that node, get the current global position instead.

        // Ideally this would use parent global position and local position to compute the global position.
        // Unfortunately the equation 
        //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
        // does not hold when inheritance type is other than "Parent" (RSrs).
        // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
        lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
    }

    return lGlobalPosition;
}

// Get the matrix of the given pose
FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

void CModel::FindTopBoneReq( int motid, CBone* curbone, CBone** ppret )
{
	if( *ppret ){
		return;
	}
//	int cmp0 = strncmp( curbone->m_bonename, "TOP_", 4 );
//	if( cmp0 == 0 ){
//		*ppret = curbone;
//		return;
//	}
	CMotionPoint* firstmp = 0;
	firstmp = curbone->m_motionkey[ motid ];
	if( firstmp ){
		if( (firstmp->m_worldmat._41 != 0.0f) || (firstmp->m_worldmat._42 != 0.0f) || (firstmp->m_worldmat._43 != 0.0f) ){
			*ppret = curbone;
			return;
		}
	}

	if( curbone->m_child ){
		FindTopBoneReq( motid, curbone->m_child, ppret );
	}
	if( curbone->m_brother ){
		FindTopBoneReq( motid, curbone->m_brother, ppret );
	}
}

void CModel::FillUpEmptyKeyReq( int motid, int animleng, CBone* curbone, CBone* parbone )
{
	D3DXMATRIX parfirstmat, invparfirstmat;
	D3DXMatrixIdentity( &parfirstmat );
	D3DXMatrixIdentity( &invparfirstmat );
	if( parbone ){
		double zeroframe = 0.0;
		int existz = 0;
		CMotionPoint* parmp = parbone->AddMotionPoint( motid, zeroframe, 0, &existz );
		if( existz && parmp ){
			parfirstmat = parmp->m_worldmat;//!!!!!!!!!!!!!! この時点ではm_matWorldが掛かっていないから後で修正必要かも？？
			D3DXMatrixInverse( &invparfirstmat, NULL, &parfirstmat );
		}else{
			D3DXMatrixIdentity( &parfirstmat );
			D3DXMatrixIdentity( &invparfirstmat );			
		}
	}

	int framecnt;
	for( framecnt = 0; framecnt < animleng; framecnt++ ){
		double frame = (double)framecnt;

		D3DXMATRIX mvmat;
		D3DXMatrixIdentity( &mvmat );

		CMotionPoint* pbef = 0;
		CMotionPoint* pnext = 0;
		int existflag = 0;
		curbone->GetBefNextMP( motid, frame, &pbef, &pnext, &existflag );
		if( existflag == 0 ){
			int exist2 = 0;
			CMotionPoint* newmp = curbone->AddMotionPoint( motid, frame, 0, &exist2 );
			if( !newmp ){
				_ASSERT( 0 );
				return;
			}

			if( framecnt == 0 ){
				curbone->m_curfirstmat = newmp->m_worldmat;
			}

			if( parbone ){
				int exist3 = 0;
				CMotionPoint* parmp = parbone->AddMotionPoint( motid, frame, 0, &exist3 );
				newmp->m_worldmat = parbone->m_invfirstmat * parmp->m_worldmat;//!!!!!!!!!!!!!!!!!! endjointはこれでうまく行くが、floatと分岐が不動になる。
			}
		}
	}

	if( curbone->m_child ){
		FillUpEmptyKeyReq( motid, animleng, curbone->m_child, curbone );
	}
	if( curbone->m_brother ){
		FillUpEmptyKeyReq( motid, animleng, curbone->m_brother, parbone );
	}

}

int CModel::SetMaterialName()
{
	m_materialname.clear();

	map<int, CMQOMaterial*>::iterator itrmat;
	for( itrmat = m_material.begin(); itrmat != m_material.end(); itrmat++ ){
		CMQOMaterial* curmat = itrmat->second;
		m_materialname[ curmat->name ] = curmat;
	}

	return 0;
}

CBone* CModel::GetValidBroBone( CBone* srcbone )
{
	CBone* retbone = srcbone->m_brother;
	while( retbone ){
		if( retbone->m_validflag == 1 ){
			break;
		}
		retbone = retbone->m_brother;
	}
	return retbone;
}
CBone* CModel::GetValidChilBone( CBone* srcbone )
{
	CBone* retbone = srcbone->m_child;
	while( retbone ){
		if( retbone->m_validflag == 1 ){
			break;
		}
		retbone = retbone->m_child;
	}
	return retbone;
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
		curbone->m_btobject.clear();
	}


	return 0;
}
void CModel::DestroyBtObjectReq( CBtObject* curbt )
{
	vector<CBtObject*> tmpchil;
	tmpchil = curbt->m_chilbt;

	delete curbt;

	int chilno;
	for( chilno = 0; chilno < (int)tmpchil.size(); chilno++ ){
		CBtObject* chilbt = tmpchil[ chilno ];
		if( chilbt ){
			DestroyBtObjectReq( chilbt );
		}
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

	CBone* srcbone = srcbto->m_bone;
	if( srcbone ){
//		srcbone->m_btkinflag = 0;
		
		int cmp0 = strncmp( srcbone->m_bonename, "BT_", 3 );
		if( (cmp0 == 0) && (srcbone->m_btforce == 1) ){
			srcbone->m_btkinflag = 0;
		}else{
			srcbone->m_btkinflag = 1;
		}

		if( (srcbone->m_btkinflag == 1) && (srcbto->m_rigidbody) ){
			DWORD curflag = srcbto->m_rigidbody->getCollisionFlags();
			if( s_setrigidflag == 0 ){
				s_rigidflag = curflag;
				s_setrigidflag = 1;
			}
			srcbto->m_rigidbody->setCollisionFlags( curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
			//srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
			//srcbto->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
			//srcbto->m_rigidbody->setActivationState(DISABLE_SIMULATION);
			//CF_STATIC_OBJECT
		}else if( srcbto->m_rigidbody ){
			if( srcbone->m_parent ){
				CRigidElem* curre = srcbone->m_parent->m_rigidelem[ srcbone ];
				if( curre ){
					srcbto->m_rigidbody->setGravity( btVector3( 0.0f, curre->m_btg * m_btgscale, 0.0f ) );
					srcbto->m_rigidbody->applyGravity();
				}
			}
		}

	}

	int chilno;
	for( chilno = 0; chilno < srcbto->m_chilbt.size(); chilno++ ){
		CBtObject* chilbto = srcbto->m_chilbt[ chilno ];
		SetBtKinFlagReq( chilbto, oncreateflag );
	}
}

int CModel::CreateBtConstraint()
{
	if( !m_topbone ){
		return 0;
	}
	if( !m_topbt ){
		return 0;
	}

	CreateBtConstraintReq( m_topbt );

	CreateBtConnectReq( m_topbone );

	return 0;
}
void CModel::CreateBtConstraintReq( CBtObject* curbto )
{
	if( curbto->m_topflag == 0 ){
		CallF( curbto->CreateBtConstraint(), return );
	}

	int btono;
	for( btono = 0; btono < (int)curbto->m_chilbt.size(); btono++ ){
		CBtObject* chilbto = curbto->m_chilbt[ btono ];
		if( chilbto ){
			CreateBtConstraintReq( chilbto );
		}
	}
}
void CModel::CreateBtConnectReq( CBone* curbone )
{
	char* findpat = strstr( curbone->m_bonename, "bunki" );
	if( findpat ){
		map<CBone*,CBtObject*>::iterator itrbto1;
		for( itrbto1 = curbone->m_btobject.begin(); itrbto1 != curbone->m_btobject.end(); itrbto1++ ){
			CBtObject* bto1 = itrbto1->second;

			map<CBone*,CBtObject*>::iterator itrbto2;
			for( itrbto2 = curbone->m_btobject.begin(); itrbto2 != curbone->m_btobject.end(); itrbto2++ ){
				CBtObject* bto2 = itrbto2->second;
				if( (bto1 != bto2) && bto1->m_rigidbody && bto2->m_rigidbody && (bto1->m_connectflag == 0) && (bto2->m_connectflag == 0) ){
					D3DXVECTOR3 diffchil;
					diffchil = bto1->m_endbone->m_jointfpos - bto2->m_endbone->m_jointfpos;
					float diffleng = D3DXVec3Length( &diffchil );
					if( diffleng < 0.0001f ){

DbgOut( L"CreateBtConnect : bto1 %s--%s, bto2 %s--%s\r\n",
	bto1->m_bone->m_wbonename, bto1->m_endbone->m_wbonename,
	bto2->m_bone->m_wbonename, bto2->m_endbone->m_wbonename
);

/***
						btPoint2PointConstraint* p2pC;
						p2pC = new btPoint2PointConstraint( *bto1->m_rigidbody, *bto2->m_rigidbody, bto1->m_curpivot, bto2->m_curpivot );
						p2pC->setBreakingImpulseThreshold( FLT_MAX );
						p2pC->setParam( BT_CONSTRAINT_CFM, 0.0f );
						bto1->m_constraint.push_back( (btTypedConstraint*)p2pC );
						m_btWorld->addConstraint(p2pC, true);
						//m_btWorld->addConstraint(p2pC, false);
***/
						float angPAI2, angPAI;
						angPAI2 = 90.0f * (float)DEG2PAI;
						angPAI = 180.0f * (float)DEG2PAI;

						float lmax, lmin;
						lmin = 1.0f;
						lmax = -1.0f;

			/***
						float lmax, lmin;
						lmin = 0.0f;
						lmax = 0.0f;
			***/
						btGeneric6DofSpringConstraint* dofC;
						dofC = new btGeneric6DofSpringConstraint( *bto1->m_rigidbody, *bto2->m_rigidbody, bto1->m_FrameA, bto2->m_FrameA, true );
						_ASSERT( dofC );

						dofC->setLinearLowerLimit( btVector3( lmin, lmin, lmin ) );
						dofC->setLinearUpperLimit( btVector3( lmax, lmax, lmax ) );
						//dofC->setAngularLowerLimit( btVector3( -angPAI, -angPAI2, -angPAI ) );
						//dofC->setAngularUpperLimit( btVector3( angPAI, angPAI2, angPAI ) );
						dofC->setAngularLowerLimit( btVector3( angPAI, angPAI2, angPAI ) );
						dofC->setAngularUpperLimit( btVector3( -angPAI, -angPAI2, -angPAI ) );
						dofC->setBreakingImpulseThreshold( FLT_MAX );
						//dofC->setBreakingImpulseThreshold( 1e9 );
						
						int l_kindex = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_l_kindex;
						int a_kindex = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_a_kindex;
						float l_damping = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_l_damping;
						float a_damping = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_a_damping;
						float l_cusk = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_cus_lk;
						float a_cusk = bto1->m_bone->m_rigidelem[ bto1->m_endbone ]->m_cus_ak;

						/***
						int dofid;
						for( dofid = 0; dofid < 3; dofid++ ){
							dofC->enableSpring( dofid, true );
							if( l_kindex <= 2 ){
								dofC->setStiffness( dofid, g_l_kval[ l_kindex ] );
							}else{
								dofC->setStiffness( dofid, l_cusk );
							}
							dofC->setDamping( dofid, l_damping );
						}
						for( dofid = 3; dofid < 6; dofid++ ){
							dofC->enableSpring( dofid, true );
							if( a_kindex <= 2 ){
								dofC->setStiffness( dofid, g_a_kval[ a_kindex ] );
							}else{
								dofC->setStiffness( dofid, a_cusk );
							}
							dofC->setDamping( dofid, a_damping );
						}
						***/
						int dofid;
						for( dofid = 0; dofid < 3; dofid++ ){
							dofC->enableSpring( dofid, true );
							//dofC->setStiffness( dofid, 1000.0f );
							dofC->setStiffness( dofid, 2000.0f );
							dofC->setDamping( dofid, 0.5f );
						}
						for( dofid = 3; dofid < 6; dofid++ ){
							dofC->enableSpring( dofid, true );
							//dofC->setStiffness( dofid, 0.5f );
							dofC->setStiffness( dofid, 80.0f );
							dofC->setDamping( dofid, 0.01f );
						}


						dofC->setEquilibriumPoint();

						bto1->m_constraint.push_back( (btTypedConstraint*)dofC );
						m_btWorld->addConstraint(dofC, true);
						//m_btWorld->addConstraint(dofC, false);


						bto2->m_connectflag = 1;
					}
				}
			}
			bto1->m_connectflag = 1;
		}
	}


	if( curbone->m_child ){
		CreateBtConnectReq( curbone->m_child );
	}
	if( curbone->m_brother ){
		CreateBtConnectReq( curbone->m_brother );
	}
}


int CModel::CreateBtObject( CModel* coldisp[COL_MAX], int onfirstcreate )
{

	DestroyBtObject();

	if( !m_topbone ){
		return 0;
	}

	CalcBtAxismatReq( coldisp, m_topbone, 0.0f );//!!!!!!!!!!!!!
	


	m_topbt = new CBtObject( 0, m_btWorld );
	if( !m_topbt ){
		_ASSERT( 0 );
		return 1;
	}
	m_topbt->m_topflag = 1;

	//CBone* startbone = m_bonename[ "jiku_Joint_bunki" ];
	//CBone* startbone = m_bonename[ "jiku_Joint" ];
	CBone* startbone = m_topbone;
	//CBone* startbone = m_bonename[ "Bip01" ];
	_ASSERT( startbone );
	CreateBtObjectReq( coldisp, m_topbt, startbone, startbone->m_child );
	//CreateBtObjectReq( coldisp, m_topbt, startbone->m_parent, startbone );

/***
	CBone* brobone = GetValidBroBone( startbone );
	if( brobone ){
		CreateBtObjectReq( coldisp, m_topbt, brobone, brobone->m_child );
		brobone = brobone->m_brother;
	}
***/
	CreateBtConstraint();


	if( m_topbt ){
		SetBtKinFlagReq( m_topbt, onfirstcreate );
	}

	return 0;
}

void CModel::CreateBtObjectReq( CModel* cpslptr[COL_MAX], CBtObject* parbt, CBone* parbone, CBone* curbone )
{
	map<CBone*,CRigidElem*>::iterator itrre;
	for( itrre = curbone->m_rigidelem.begin(); itrre != curbone->m_rigidelem.end(); itrre++ ){
		CRigidElem* curre = itrre->second;
		CBone* chilbone = itrre->first;
		D3DXVECTOR3 diffbone = curbone->m_jointfpos - chilbone->m_jointfpos;
		float leng = D3DXVec3Length( &diffbone );
		map<CBone*,CBone*>::iterator itrfind = m_rigidbone.find( chilbone );
		if( itrfind == m_rigidbone.end() ){
			//if( leng > 0.00001f ){
			if( curre->m_skipflag == 0 ){
DbgOut( L"CreateBtObject : curbone %s, chilbone %s\r\n",
curbone->m_wbonename, chilbone->m_wbonename );

				m_rigidbone[ chilbone ] = curbone;
				CBtObject* newbto = new CBtObject( parbt, m_btWorld );
				if( !newbto ){
					_ASSERT( 0 );
					return;
				}
				CallF( newbto->CreateObject( parbt, parbone, curbone, chilbone ), return );
				curbone->m_btobject[ chilbone ] = newbto;
				CreateBtObjectReq( cpslptr, newbto, curbone, chilbone );
			}else{
DbgOut( L"CreateBtObject : skip !!!! : curbone %s, chilbone %s\r\n",
curbone->m_wbonename, chilbone->m_wbonename );

				CreateBtObjectReq( cpslptr, parbt, curbone, chilbone );
			}
		}
	}
}

void CModel::CalcBtAxismatReq( CModel* coldisp[COL_MAX], CBone* curbone, float delta )
{
	int setstartflag;
	if( delta == 0.0f ){
		setstartflag = 1;
		curbone->m_startmat2 = curbone->m_curmp.m_worldmat;
	}else{
		setstartflag = 0;
	}

	if( curbone->m_child ){
		//curbone->CalcAxisMat( firstflag, delta );
		curbone->CalcRigidElemParams( coldisp, curbone->m_child, setstartflag );
		CBone* bro = curbone->m_child->m_brother;
		while( bro ){
			curbone->CalcRigidElemParams( coldisp, bro, setstartflag );
			bro = bro->m_brother;
		}
DbgOut( L"check!!!!:CalcBtAxismatReq : %s, %s\r\n", curbone->m_wbonename, curbone->m_child->m_wbonename );

	}

	if( curbone->m_child ){
		CalcBtAxismatReq( coldisp, curbone->m_child, delta );
	}
	if( curbone->m_brother ){
		CalcBtAxismatReq( coldisp, curbone->m_brother, delta );
	}
}

int CModel::SetBtMotion( int ragdollflag, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat, double difftime )
{
	m_matWorld = *wmat;
	m_matVP = *vpmat;

	if( !m_topbt ){
		_ASSERT( 0 );
		return 0;
	}

	if( !m_curmotinfo ){
		_ASSERT( 0 );
		return 0;//!!!!!!!!!!!!
	}

	int curmotid = m_curmotinfo->motid;
	double curframe = m_curmotinfo->curframe;

	D3DXMATRIX inimat;
	D3DXMatrixIdentity( &inimat );
	CQuaternion iniq;
	iniq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );

	map<int, CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			curbone->m_curmp.m_setbtflag = 0;
		}
	}

	SetBtMotionReq( m_topbt, wmat, vpmat );


	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone && (curbone->m_curmp.m_setbtflag == 0) ){
			if( g_previewFlag == 4 ){
				if( curbone->m_btkinflag == 0 ){
					if( curbone->m_parent ){
						//curbone->m_curmp.m_btmat = curbone->m_parent->m_curmp.m_btmat;
						D3DXMATRIX invstart;
						D3DXMatrixInverse( &invstart, NULL, &(curbone->m_parent->m_startmat2) );
						D3DXMATRIX diffmat;
						diffmat = invstart * curbone->m_parent->m_curmp.m_btmat;
						curbone->m_curmp.m_btmat = curbone->m_startmat2 * diffmat;
					}else{
						curbone->m_curmp.m_btmat = curbone->m_startmat2;
					}
				}else{
					curbone->m_curmp.m_btmat = curbone->m_curmp.m_worldmat;
				}
			}else if( g_previewFlag == 5 ){
				if( curbone->m_parent ){
					//curbone->m_curmp.m_btmat = curbone->m_parent->m_curmp.m_btmat;
					D3DXMATRIX invstart;
					D3DXMatrixInverse( &invstart, NULL, &(curbone->m_parent->m_startmat2) );
					D3DXMATRIX diffmat;
					diffmat = invstart * curbone->m_parent->m_curmp.m_btmat;
					curbone->m_curmp.m_btmat = curbone->m_startmat2 * diffmat;
				}else{
					curbone->m_curmp.m_btmat = curbone->m_startmat2;
				}
			}
			curbone->m_curmp.m_setbtflag = 1;
		}
	}

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
				if( !(curobj->m_findshape.empty()) ){
					GetShapeWeight( m_fbxobj[curobj].node, m_fbxobj[curobj].mesh, lTime, mCurrentAnimLayer, curobj );
					CallF( curobj->UpdateMorphBuffer(), return 1 );
				}
			}
		}
	}

	return 0;
}

int CModel::DampAnim( MOTINFO* rgdmorphinfo )
{
	if( !rgdmorphinfo || (m_rgdindex < 0) ){
		return 0;
	}

	double diffframe = rgdmorphinfo->curframe;

	if( m_rgdindex >= m_rigideleminfo.size() ){
		_ASSERT( 0 );
		return 0;
	}

	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		if( curbone ){
			CBone* parbone = curbone->m_parent;
			if( parbone ){
				CRigidElem* curre = parbone->m_rigidelem[ curbone ];
				if( curre ){

					float newdampl = curre->m_l_damping - diffframe * curre->m_dampanim_l * curre->m_l_damping;
					if( newdampl < 0.0f ){
						newdampl = 0.0f;
					}
					float newdampa = curre->m_a_damping - diffframe * curre->m_dampanim_a * curre->m_a_damping;
					if( newdampa < 0.0f ){
						newdampa = 0.0f;
					}


					CBtObject* curbto = FindBtObject( curbone->m_boneno );
					if( curbto ){
						btGeneric6DofSpringConstraint* dofC = curbto->m_dofC;
						if( dofC ){
							int dofid;
							for( dofid = 0; dofid < 3; dofid++ ){
								dofC->setDamping( dofid, newdampl );
							}
							for( dofid = 3; dofid < 6; dofid++ ){
								dofC->setDamping( dofid, newdampa );
							}
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

	int motionnum = m_motinfo.size();
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


void CModel::SetBtMotionReq( CBtObject* curbto, D3DXMATRIX* wmat, D3DXMATRIX* vpmat )
{
	if( g_previewFlag == 4 ){
		if( (curbto->m_topflag == 0) && curbto->m_bone && (curbto->m_bone->m_btkinflag == 0) ){
			curbto->SetBtMotion( wmat, vpmat );
		}
	}else if( g_previewFlag == 5 ){
		if( (curbto->m_topflag == 0) && curbto->m_bone ){
			curbto->SetBtMotion( wmat, vpmat );
		}
	}

	int chilno;
	for( chilno = 0; chilno < curbto->m_chilbt.size(); chilno++ ){
		CBtObject* chilbto = curbto->m_chilbt[ chilno ];
		if( chilbto ){
			SetBtMotionReq( chilbto, wmat, vpmat );
		}
	}

}

void CModel::CreateRigidElemReq( CBone* curbone, int reflag, string rename, int impflag, string impname )
{
	CBone* chil = curbone->m_child;
	while( chil ){
		curbone->CreateRigidElem( chil, reflag, rename, impflag, impname );
		chil = chil->m_brother;
		while( chil ){
			curbone->CreateRigidElem( chil, reflag, rename, impflag, impname );
			chil = chil->m_brother;
		}
	}

	if( curbone->m_child ){
		CreateRigidElemReq( curbone->m_child, reflag, rename, impflag, impname );
	}
	if( curbone->m_brother ){
		CreateRigidElemReq( curbone->m_brother, reflag, rename, impflag, impname );
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
	CBone* parbone = srcbone->m_parent;

	if( parbone ){
		D3DXVECTOR3 setimp( 0.0f, 0.0f, 0.0f );

		int impnum = parbone->m_impmap.size();
		if( (m_curimpindex >= 0) && (m_curimpindex < impnum) ){

			string curimpname = m_impinfo[ m_curimpindex ];
			map<string, map<CBone*, D3DXVECTOR3>>::iterator findimpmap;
			findimpmap = parbone->m_impmap.find( curimpname );
			if( findimpmap != parbone->m_impmap.end() ){
				map<CBone*, D3DXVECTOR3>& curmap = findimpmap->second;
	
				map<CBone*,D3DXVECTOR3>::iterator itrimp;
				itrimp = curmap.find( srcbone );
				if( itrimp != curmap.end() ){
					setimp = itrimp->second;
				}
			}
		}

		CRigidElem* curre = parbone->m_rigidelem[ srcbone ];
		if( curre ){
			D3DXVECTOR3 imp = setimp * g_impscale;

			CBtObject* findbto = FindBtObject( srcbone->m_boneno );
			if( findbto && findbto->m_rigidbody ){
				findbto->m_rigidbody->applyImpulse( btVector3( imp.x, imp.y, imp.z ), btVector3( 0.0f, 0.0f, 0.0f ) );
			}
		}
	}

	if( srcbone->m_child ){
		SetBtImpulseReq( srcbone->m_child );
	}
	if( srcbone->m_brother ){
		SetBtImpulseReq( srcbone->m_brother );
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

	if( srcbto->m_bone ){
		CBone* curbone;
		curbone = m_bonelist[ srcboneno ];
		if( curbone ){
			CBone* parbone;
			parbone = curbone->m_parent;
			if( parbone ){
				if( (srcbto->m_bone == parbone) && (srcbto->m_endbone == curbone) ){
					*ppret = srcbto;
					return;
				}
			}
		}
	}

	int chilno;
	for( chilno = 0; chilno < srcbto->m_chilbt.size(); chilno++ ){
		CBtObject* chilbto = srcbto->m_chilbt[ chilno ];
		FindBtObjectReq( chilbto, srcboneno, ppret );
	}
}
int CModel::SetDispFlag( char* srcobjname, int srcflag )
{
	CMQOObject* curobj = m_objectname[ srcobjname ];
	if( curobj ){
		curobj->m_dispflag = srcflag;
	}

	return 0;
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[reindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_btg = btg;
			}
		}
	}

	if( srcbone->m_child ){
		SetBtgDataReq( gid, reindex, srcbone->m_child, btg );
	}
	if( srcbone->m_brother ){
		SetBtgDataReq( gid, reindex, srcbone->m_brother, btg );
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
	if( rgdindex < 0 ){
		return;
	}

	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[rgdindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_dampanim_l = valL;
				curre->m_dampanim_a = valA;
			}
		}
	}

	if( srcbone->m_child ){
		SetDampAnimDataReq( gid, rgdindex, srcbone->m_child, valL, valA );
	}
	if( srcbone->m_brother ){
		SetDampAnimDataReq( gid, rgdindex, srcbone->m_brother, valL, valA );
	}
}


int CModel::SetAllImpulseData( int gid, float impx, float impy, float impz )
{
	if( !m_topbone ){
		return 0;
	}
	D3DXVECTOR3 srcimp = D3DXVECTOR3( impx, impy, impz );

	SetImpulseDataReq( gid, m_topbone, srcimp );

	return 0;
}

void CModel::SetImpulseDataReq( int gid, CBone* srcbone, D3DXVECTOR3 srcimp )
{
	CBone* parbone = srcbone->m_parent;

	if( parbone ){
		int renum = m_rigideleminfo.size();
		int impnum = parbone->m_impmap.size();
		if( (m_curimpindex >= 0) && (m_curimpindex < impnum) && (m_curreindex >= 0) && (m_curreindex < renum) ){

			char* filename = m_rigideleminfo[m_curreindex].filename;
			CRigidElem* curre = (parbone->m_remap[ filename ])[ srcbone ];
			if( curre ){
				if( (gid == -1) || (gid == curre->m_groupid) ){
					string curimpname = m_impinfo[ m_curimpindex ];
					map<string, map<CBone*, D3DXVECTOR3>>::iterator findimpmap;
					findimpmap = parbone->m_impmap.find( curimpname );
					if( findimpmap != parbone->m_impmap.end() ){
						map<CBone*, D3DXVECTOR3>& curmap = findimpmap->second;
	
						map<CBone*,D3DXVECTOR3>::iterator itrimp;
						itrimp = curmap.find( srcbone );
						if( itrimp != curmap.end() ){
							itrimp->second = srcimp;
						}
					}
				}
			}
		}
	}

	if( srcbone->m_child ){
		SetImpulseDataReq( gid, srcbone->m_child, srcimp );
	}
	if( srcbone->m_brother ){
		SetImpulseDataReq( gid, srcbone->m_brother, srcimp );
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

	CBone* parbone = curbone->m_parent;
	if( !parbone ){
		return 0;
	}

	int impnum = parbone->m_impmap.size();
	if( m_curimpindex >= impnum ){
		_ASSERT( 0 );
		return 0;
	}

	string curimpname = m_impinfo[ m_curimpindex ];
	map<string, map<CBone*, D3DXVECTOR3>>::iterator findimpmap;
	findimpmap = parbone->m_impmap.find( curimpname );
	if( findimpmap != parbone->m_impmap.end() ){
		map<CBone*, D3DXVECTOR3>& curmap = findimpmap->second;
	
		map<CBone*,D3DXVECTOR3>::iterator itrimp;
		itrimp = curmap.find( curbone );
		if( itrimp == curmap.end() ){
			_ASSERT( 0 );
			return 0;
		}
	
		if( kind == 0 ){
			itrimp->second.x = srcval;
		}else if( kind == 1 ){
			itrimp->second.y = srcval;
		}else if( kind == 2 ){
			itrimp->second.z = srcval;
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[reindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_l_damping = ldmp;
				curre->m_a_damping = admp;
			}
		}
	}

	if( srcbone->m_child ){
		SetDmpDataReq( gid, reindex, srcbone->m_child, ldmp, admp );
	}
	if( srcbone->m_brother ){
		SetDmpDataReq( gid, reindex, srcbone->m_brother, ldmp, admp );
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[reindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_restitution = rest;
				curre->m_friction = fric;
			}
		}
	}

	if( srcbone->m_child ){
		SetRestDataReq( gid, reindex, srcbone->m_child, rest, fric );
	}
	if( srcbone->m_brother ){
		SetRestDataReq( gid, reindex, srcbone->m_brother, rest, fric );
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[reindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_l_kindex = srclk;
				curre->m_a_kindex = srcak;
				curre->m_cus_lk = srccuslk;
				curre->m_cus_ak = srccusak;
			}
		}
	}

	if( srcbone->m_child ){
		SetKDataReq( gid, reindex, srcbone->m_child, srclk, srcak, srccuslk, srccusak );
	}
	if( srcbone->m_brother ){
		SetKDataReq( gid, reindex, srcbone->m_brother, srclk, srcak, srccuslk, srccusak );
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[reindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( (gid == -1) || (gid == curre->m_groupid) ){
				curre->m_mass = srcmass;
			}
		}
	}

	if( srcbone->m_child ){
		SetMassDataReq( gid, reindex, srcbone->m_child, srcmass );
	}
	if( srcbone->m_brother ){
		SetMassDataReq( gid, reindex, srcbone->m_brother, srcmass );
	}
}

int CModel::SetRagdollKinFlag()
{

	if( !m_topbt ){
		return 0;
	}

	SetRagdollKinFlagReq( m_topbt );

	return 0;
}
void CModel::SetRagdollKinFlagReq( CBtObject* srcbto )
{

	if( srcbto->m_bone && srcbto->m_rigidbody ){
		DWORD curflag = srcbto->m_rigidbody->getCollisionFlags();
		srcbto->m_rigidbody->setCollisionFlags( curflag & ~btCollisionObject::CF_KINEMATIC_OBJECT);

//		_ASSERT( s_setrigidflag );
//		srcbto->m_rigidbody->setCollisionFlags( s_rigidflag );
//		srcbto->m_rigidbody->setCollisionFlags( s_rigidflag  | btCollisionObject::CF_STATIC_OBJECT);
//		srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);

		if( srcbto->m_bone->m_parent ){
			CRigidElem* curre = srcbto->m_bone->m_parent->m_rigidelem[ srcbto->m_bone ];
			if( curre ){
				srcbto->m_rigidbody->setGravity( btVector3( 0.0f, curre->m_btg * m_btgscale, 0.0f ) );
				srcbto->m_rigidbody->applyGravity();
			}
		}
	}

	int chilno;
	for( chilno = 0; chilno < srcbto->m_chilbt.size(); chilno++ ){
		CBtObject* chilbto = srcbto->m_chilbt[ chilno ];
		if( chilbto ){
			SetRagdollKinFlagReq( chilbto );
		}
	}
}

int CModel::SetCurrentRigidElem( int curindex )
{
	if( curindex < 0 ){
		return 0;
	}
	if( curindex >= m_rigideleminfo.size() ){
		_ASSERT( 0 );
		return 0;
	}

	m_curreindex = curindex;

	string curname = m_rigideleminfo[ curindex ].filename;
	m_btgscale = m_rigideleminfo[ curindex ].btgscale;

	map<int,CBone*>::iterator itrbone;
	for( itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		CallF( curbone->SetCurrentRigidElem( curname ), return 1 );
	}

	return 0;
}

int CModel::MultDispObj( D3DXVECTOR3 srcmult, D3DXVECTOR3 srctra )
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
	if( srcbone->m_parent ){
		char* filename = m_rigideleminfo[m_curreindex].filename;
		CRigidElem* curre = (srcbone->m_parent->m_remap[ filename ])[ srcbone ];
		if( curre ){
			if( curre->m_groupid == srcre->m_groupid ){
				curre->m_coliids = srcre->m_coliids;
			}
		}
	}

	if( srcbone->m_child ){
		SetColiIDReq( srcbone->m_child, srcre );
	}
	if( srcbone->m_brother ){
		SetColiIDReq( srcbone->m_brother, srcre );
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
	if( curbto->m_rigidbody ){
		if (curbto->m_rigidbody->getMotionState())
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)curbto->m_rigidbody->getMotionState();
			myMotionState->m_graphicsWorldTrans = myMotionState->m_startWorldTrans;
			curbto->m_rigidbody->setCenterOfMassTransform( myMotionState->m_graphicsWorldTrans );
			curbto->m_rigidbody->setInterpolationWorldTransform( myMotionState->m_startWorldTrans );
			curbto->m_rigidbody->forceActivationState(ACTIVE_TAG);
			curbto->m_rigidbody->activate();
			curbto->m_rigidbody->setDeactivationTime(0);
			//colObj->setActivationState(WANTS_DEACTIVATION);
		}
		if (curbto->m_rigidbody && !curbto->m_rigidbody->isStaticObject())
		{
			curbto->m_rigidbody->setLinearVelocity(btVector3(0,0,0));
			curbto->m_rigidbody->setAngularVelocity(btVector3(0,0,0));
		}
	}

	int chilno;
	for( chilno = 0; chilno < curbto->m_chilbt.size(); chilno++ ){
		CBtObject* chilbto = curbto->m_chilbt[chilno];
		if( chilbto ){
			ResetBtReq( chilbto );
		}
	}
}

int CModel::SetBefEditMat( CEditRange* erptr, CBone* curbone, int maxlevel )
{
	int levelcnt0 = 0;
	while( curbone && ((maxlevel == 0) || (levelcnt0 < (maxlevel+1))) )
	{
		list<KeyInfo>::iterator itrki;
		double firstframe = 0.0;
		for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
			double curframe = itrki->time;
			int existflag3 = 0;
			CMotionPoint* editmp = 0;
			CMotionPoint* nextmp = 0;
			curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &editmp, &nextmp, &existflag3 );
			if( existflag3 ){
				editmp->m_befeditmat = editmp->m_worldmat; 
			}else{
				_ASSERT( 0 );
			}
		}
		curbone = curbone->m_parent;
		levelcnt0++;
	}
	return 0;
}

int CModel::SetBefEditMatFK( CEditRange* erptr, CBone* curbone )
{
	list<KeyInfo>::iterator itrki;
	double firstframe = 0.0;
	for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
		double curframe = itrki->time;
		int existflag3 = 0;
		CMotionPoint* editmp = 0;
		CMotionPoint* nextmp = 0;
		curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &editmp, &nextmp, &existflag3 );
		if( existflag3 ){
			editmp->m_befeditmat = editmp->m_worldmat; 
		}else{
			_ASSERT( 0 );
		}
	}
	return 0;
}

int CModel::IKRotate( CEditRange* erptr, int srcboneno, D3DXVECTOR3 targetpos, int maxlevel )
{
	//m_newmplist.erase( m_newmplist.begin(), m_newmplist.end() );

	CBone* firstbone = m_bonelist[ srcboneno ];
	if( !firstbone ){
		_ASSERT( 0 );
		return -1;
	}


	D3DXVECTOR3 ikaxis = g_camtargetpos - g_camEye;
	D3DXVec3Normalize( &ikaxis, &ikaxis );

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange( &keynum, &startframe, &endframe, &applyframe );

	CBone* curbone = firstbone;
	SetBefEditMat( erptr, curbone, maxlevel );

	CBone* lastpar = firstbone->m_parent;
	curbone = firstbone;
	int calcnum = 3;
	int calccnt;
	for( calccnt = 0; calccnt < calcnum; calccnt++ ){
		int levelcnt = 0;

		float currate = 1.0f;

		CBone* curbone = firstbone;
		while( curbone && ((maxlevel == 0) || (levelcnt < maxlevel)) )
		{
			CBone* parbone = curbone->m_parent;
			if( parbone && (curbone->m_jointfpos != parbone->m_jointfpos) ){
				D3DXVECTOR3 parworld, chilworld;
				D3DXVec3TransformCoord( &chilworld, &(curbone->m_jointfpos), &(curbone->m_curmp.m_worldmat) );
				D3DXVec3TransformCoord( &parworld, &(parbone->m_jointfpos), &(parbone->m_curmp.m_worldmat) );

				D3DXVECTOR3 parbef, chilbef, tarbef;
				parbef = parworld;
				CalcShadowToPlane( chilworld, ikaxis, parworld, &chilbef );
				CalcShadowToPlane( targetpos, ikaxis, parworld, &tarbef );

				D3DXVECTOR3 vec0, vec1;
				vec0 = chilbef - parbef;
				D3DXVec3Normalize( &vec0, &vec0 );
				vec1 = tarbef - parbef;
				D3DXVec3Normalize( &vec1, &vec1 );

				D3DXVECTOR3 rotaxis2;
				D3DXVec3Cross( &rotaxis2, &vec0, &vec1 );
				D3DXVec3Normalize( &rotaxis2, &rotaxis2 );

				float rotdot2, rotrad2;
				rotdot2 = DXVec3Dot( &vec0, &vec1 );
				rotdot2 = min( 1.0f, rotdot2 );
				rotdot2 = max( -1.0f, rotdot2 );
				rotrad2 = (float)acos( rotdot2 );
				rotrad2 *= currate;
				double firstframe = 0.0;
				if( fabs( rotrad2 ) > 1.0e-4 ){
					CQuaternion rotq;
					rotq.SetAxisAndRot( rotaxis2, rotrad2 );
					if( keynum >= 2 ){
						int keyno = 0;
						list<KeyInfo>::iterator itrki;

						for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
							double curframe = itrki->time;
							double changerate;
							if( curframe <= applyframe ){
								if( applyframe != startframe ){
									changerate = 1.0 / (applyframe - startframe);
								}else{
									changerate = 1.0;
								}
							}else{
								if( applyframe != endframe ){
									changerate = 1.0 / (endframe - applyframe );
								}else{
									changerate = 0.0;
								}
							}


							if( keyno == 0 ){
								firstframe = curframe;
							}
							if( g_absikflag == 0 ){
								if( g_slerpoffflag == 0 ){
									CQuaternion endq;
									endq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
									CQuaternion curq;
									double currate2;
									if( curframe <= applyframe ){
										if( applyframe != startframe ){
											currate2 = changerate * (curframe - startframe);
										}else{
											currate2 = 1.0;
										}
									}else{
										currate2 = changerate * (endframe - curframe);
									}
									rotq.Slerp2( endq, 1.0 - currate2, &curq );

									parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, curq );
								}else{
									parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
								}
							}else{
								if( keyno == 0 ){
									parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
								}else{
									parbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );
								}
							}
							keyno++;
						}
					}else{
						parbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
					}


					if( g_applyendflag == 1 ){
						//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
						int tolast;
						for( tolast = m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++ ){
							(m_bonelist[ 0 ])->PasteRotReq( m_curmotinfo->motid, m_curmotinfo->curframe, tolast );
						}
					}

					//parbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
					UpdateMatrix( parbone->m_boneno, &m_matWorld, &m_matVP );
				}else{
					UpdateMatrix( parbone->m_boneno, &m_matWorld, &m_matVP );
				}


			}


			if( curbone->m_parent ){
				lastpar = curbone->m_parent;
			}
			curbone = curbone->m_parent;

			levelcnt++;

			currate = pow( g_ikrate, g_ikfirst * levelcnt );
		}

		if( (calccnt == (calcnum - 1)) && g_absikflag && lastpar ){
			AdjustBoneTra( erptr, lastpar );
		}
	}

	if( lastpar ){
		return lastpar->m_boneno;
	}else{
		return srcboneno;
	}

}

int CModel::AdjustBoneTra( CEditRange* erptr, CBone* lastpar )
{
	if( g_applyendflag == 1 ){
		if( lastpar && (erptr->m_ki.size() >= 2) ){
			list<KeyInfo>::iterator itrki;
			int keyno = 0;
			int startframe = erptr->m_ki.begin()->time;
			int endframe = m_curmotinfo->frameleng - 1;
			double curframe;
			for( curframe = startframe; curframe <= endframe; curframe += 1.0 ){
				if( keyno >= 1 ){
					int existflag2 = 0;
					CMotionPoint* pbef = 0;
					CMotionPoint* pnext = 0;
					int curmotid = m_curmotinfo->motid;
					lastpar->GetBefNextMP( curmotid, curframe, &pbef, &pnext, &existflag2 );
					if( existflag2 ){
						D3DXVECTOR3 orgpos;
						D3DXVec3TransformCoord( &orgpos, &(lastpar->m_jointfpos), &(pbef->m_befeditmat) );

						D3DXVECTOR3 newpos;
						D3DXVec3TransformCoord( &newpos, &(lastpar->m_jointfpos), &(pbef->m_worldmat) );

						D3DXVECTOR3 diffpos;
						diffpos = orgpos - newpos;

						CEditRange tmper;
						KeyInfo tmpki;
						tmpki.time = curframe;
						list<KeyInfo> tmplist;
						tmplist.push_back( tmpki );
						tmper.SetRange( tmplist, curframe );
						FKBoneTra( &tmper, lastpar->m_boneno, curmotid, curframe, diffpos );
					}
				}
				keyno++;
			}
		}
	}else{
		if( lastpar && (erptr->m_ki.size() >= 2) ){
			list<KeyInfo>::iterator itrki;
			int keyno = 0;
			for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
				if( keyno >= 1 ){
					double curframe = itrki->time;
					int existflag2 = 0;
					CMotionPoint* pbef = 0;
					CMotionPoint* pnext = 0;
					int curmotid = m_curmotinfo->motid;
					lastpar->GetBefNextMP( curmotid, curframe, &pbef, &pnext, &existflag2 );
					if( existflag2 ){
						D3DXVECTOR3 orgpos;
						D3DXVec3TransformCoord( &orgpos, &(lastpar->m_jointfpos), &(pbef->m_befeditmat) );

						D3DXVECTOR3 newpos;
						D3DXVec3TransformCoord( &newpos, &(lastpar->m_jointfpos), &(pbef->m_worldmat) );

						D3DXVECTOR3 diffpos;
						diffpos = orgpos - newpos;

						CEditRange tmper;
						KeyInfo tmpki;
						tmpki.time = curframe;
						list<KeyInfo> tmplist;
						tmplist.push_back( tmpki );
						tmper.SetRange( tmplist, curframe );
						FKBoneTra( &tmper, lastpar->m_boneno, curmotid, curframe, diffpos );
					}
				}
				keyno++;
			}
		}
	}

	return 0;
}


int CModel::IKRotateAxisDelta( CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt )
{
	D3DXMATRIX selectmat = D3DXMATRIX( 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f );
	int levelcnt = 0;
	CBone* firstbone = m_bonelist[ srcboneno ];
	if( !firstbone ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone = firstbone;
	SetBefEditMat( erptr, curbone, maxlevel );


	CBone* calcrootbone = GetCalcRootBone( firstbone, maxlevel );
	_ASSERT( calcrootbone );
	D3DXVECTOR3 rootpos = calcrootbone->m_childworld;

	D3DXVECTOR3 axis0;
	D3DXVECTOR3 targetpos;
	D3DXMATRIX mat, befrotmat, rotmat, aftrotmat;

	float rotrad;
	selectmat = firstbone->m_axismat_par * firstbone->m_curmp.m_worldmat; 
	selectmat._41 = 0.0f;
	selectmat._42 = 0.0f;
	selectmat._43 = 0.0f;

	if( axiskind == PICK_X ){
		axis0 = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
		D3DXVec3TransformCoord( &m_ikrotaxis, &axis0, &selectmat );
		D3DXVec3Normalize( &m_ikrotaxis, &m_ikrotaxis );
	}else if( axiskind == PICK_Y ){
		axis0 = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		D3DXVec3TransformCoord( &m_ikrotaxis, &axis0, &selectmat );
		D3DXVec3Normalize( &m_ikrotaxis, &m_ikrotaxis );
	}else if( axiskind == PICK_Z ){
		axis0 = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		D3DXVec3TransformCoord( &m_ikrotaxis, &axis0, &selectmat );
		D3DXVec3Normalize( &m_ikrotaxis, &m_ikrotaxis );
	}else{
		_ASSERT( 0 );
		return 1;
	}
	rotrad = delta / 10.0f * (float)PAI / 12.0f;

	if( fabs(rotrad) < (0.02f * (float)DEG2PAI) ){
		return 0;
	}

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange( &keynum, &startframe, &endframe, &applyframe );
	int calcnum = 3;

	CBone* lastpar = firstbone;
	double firstframe = 0.0;

	if( (firstbone == calcrootbone) && !firstbone->m_parent ){
		int calccnt;
		for( calccnt = 0; calccnt < calcnum; calccnt++ ){
			int levelcnt = 0;

			float currate = 1.0f;


			float rotrad2 = rotrad * currate;
			if( fabs( rotrad2 ) > 1.0e-4 ){
				CQuaternion rotq;
				rotq.SetAxisAndRot( m_ikrotaxis, rotrad2 );

				if( keynum >= 2 ){
					int keyno = 0;
					list<KeyInfo>::iterator itrki;
					for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
						double curframe = itrki->time;
						double changerate;
						if( curframe <= applyframe ){
							if( applyframe != startframe ){
								changerate = 1.0 / (applyframe - startframe);
							}else{
								changerate = 1.0;
							}
						}else{
							if( applyframe != endframe ){
								changerate = 1.0 / (endframe - applyframe );
							}else{
								changerate = 0.0;
							}
						}
						if( keyno == 0 ){
							firstframe = curframe;
						}
						if( g_absikflag == 0 ){
							if( g_slerpoffflag == 0 ){
								CQuaternion endq;
								endq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
								CQuaternion curq;
								double currate2;
								if( curframe <= applyframe ){
									if( applyframe != startframe ){
										currate2 = changerate * (curframe - startframe);
									}else{
										currate2 = 1.0;
									}
								}else{
									currate2 = changerate * (endframe - curframe);
								}
								rotq.Slerp2( endq, 1.0 - currate2, &curq );

								firstbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, curq );
							}else{
								firstbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
							}
						}else{
							if( keyno == 0 ){
								firstbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
							}else{
								firstbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );
							}
						}
						keyno++;
					}


					if( g_applyendflag == 1 ){
						//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
						int tolast;
						for( tolast = m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++ ){
							(m_bonelist[ 0 ])->PasteRotReq( m_curmotinfo->motid, m_curmotinfo->curframe, tolast );
						}
					}


				}else{
					firstbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
				}

				//parbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
				UpdateMatrix( firstbone->m_boneno, &m_matWorld, &m_matVP );

				if( (calccnt == (calcnum - 1)) && g_absikflag ){
					AdjustBoneTra( erptr, firstbone );
				}
			}
		}		
	}else{
		D3DXVECTOR3 rcen;
		D3DXVec3TransformCoord( &rcen, &(firstbone->m_jointfpos), &(firstbone->m_curmp.m_worldmat) );

		D3DXMatrixTranslation( &befrotmat, -rootpos.x, -rootpos.y, -rootpos.z );
		D3DXMatrixTranslation( &aftrotmat, rootpos.x, rootpos.y, rootpos.z );
		D3DXMatrixRotationAxis( &rotmat, &m_ikrotaxis, rotrad );
		mat = firstbone->m_curmp.m_worldmat * befrotmat * rotmat * aftrotmat;
		D3DXVec3TransformCoord( &targetpos, &firstbone->m_jointfpos, &mat );

		int calccnt;
		for( calccnt = 0; calccnt < calcnum; calccnt++ ){
			int levelcnt = 0;

			float currate = 1.0f;

			CBone* curbone = firstbone;
			while( curbone && ((maxlevel == 0) || (levelcnt < maxlevel)) )
			{
				CBone* parbone = curbone->m_parent;
				if( parbone && (curbone->m_jointfpos != parbone->m_jointfpos) ){
					D3DXVECTOR3 parworld, chilworld;
					//chilworld = firstbone->m_childworld;
					D3DXVec3TransformCoord( &chilworld, &(curbone->m_jointfpos), &(curbone->m_curmp.m_worldmat) );
					D3DXVec3TransformCoord( &parworld, &(parbone->m_jointfpos), &(parbone->m_curmp.m_worldmat) );

					D3DXVECTOR3 parbef, chilbef, tarbef;
					parbef = parworld;
					CalcShadowToPlane( chilworld, m_ikrotaxis, parworld, &chilbef );
					CalcShadowToPlane( targetpos, m_ikrotaxis, parworld, &tarbef );

					D3DXVECTOR3 vec0, vec1;
					vec0 = chilbef - parbef;
					D3DXVec3Normalize( &vec0, &vec0 );
					vec1 = tarbef - parbef;
					D3DXVec3Normalize( &vec1, &vec1 );

					D3DXVECTOR3 rotaxis2;
					D3DXVec3Cross( &rotaxis2, &vec0, &vec1 );
					D3DXVec3Normalize( &rotaxis2, &rotaxis2 );

					float rotdot2, rotrad2;
					rotdot2 = DXVec3Dot( &vec0, &vec1 );
					rotdot2 = min( 1.0f, rotdot2 );
					rotdot2 = max( -1.0f, rotdot2 );
					rotrad2 = (float)acos( rotdot2 );
					rotrad2 *= currate;
					if( fabs( rotrad2 ) > 1.0e-4 ){
						CQuaternion rotq;
						rotq.SetAxisAndRot( rotaxis2, rotrad2 );

						if( keynum >= 2 ){
							int keyno = 0;
							list<KeyInfo>::iterator itrki;
							for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
								double curframe = itrki->time;

								double changerate;
								if( curframe <= applyframe ){
									if( applyframe != startframe ){
										changerate = 1.0 / (applyframe - startframe);
									}else{
										changerate = 1.0;
									}
								}else{
									if( applyframe != endframe ){
										changerate = 1.0 / (endframe - applyframe );
									}else{
										changerate = 0.0;
									}
								}

								if( keyno == 0 ){
									firstframe = curframe;
								}
								if( g_absikflag == 0 ){
									if( g_slerpoffflag == 0 ){
										double currate2;
										CQuaternion endq;
										CQuaternion curq;
										endq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
										if( curframe <= applyframe ){
											if( applyframe != startframe ){
												currate2 = changerate * (curframe - startframe);
											}else{
												currate2 = 1.0;
											}
										}else{
											currate2 = changerate * (endframe - curframe);
										}
										rotq.Slerp2( endq, 1.0 - currate2, &curq );

										parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, curq );
									}else{
										parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
									}
								}else{
									if( keyno == 0 ){
										parbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
									}else{
										parbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );
									}
								}
								keyno++;
							}

						}else{
							parbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
						}


						if( g_applyendflag == 1 ){
							//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
							int tolast;
							for( tolast = m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++ ){
								(m_bonelist[ 0 ])->PasteRotReq( m_curmotinfo->motid, m_curmotinfo->curframe, tolast );
							}
						}

						//parbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
						UpdateMatrix( parbone->m_boneno, &m_matWorld, &m_matVP );
					}else{
						UpdateMatrix( parbone->m_boneno, &m_matWorld, &m_matVP );
					}
				}

				if( curbone->m_parent ){
					lastpar = curbone->m_parent;
				}
				curbone = curbone->m_parent;
				levelcnt++;

				currate = pow( g_ikrate, g_ikfirst * levelcnt );

			}

			if( (calccnt == (calcnum - 1)) && g_absikflag && lastpar ){
				AdjustBoneTra( erptr, lastpar );
			}
		}

	}

	if( lastpar ){
		return lastpar->m_boneno;
	}else{
		return srcboneno;
	}

}

int CModel::RotateXDelta( CEditRange* erptr, int srcboneno, float delta )
{
	CBone* firstbone = m_bonelist[ srcboneno ];
	if( !firstbone ){
		_ASSERT( 0 );
		return 1;
	}

	CBone* curbone = firstbone;
	SetBefEditMatFK( erptr, curbone );

	CBone* lastpar = firstbone->m_parent;


	float rotrad;
	D3DXVECTOR3 axis0, rotaxis;
	D3DXMATRIX selectmat;

	selectmat = curbone->m_axismat_par * curbone->m_curmp.m_worldmat; 
	selectmat._41 = 0.0f;
	selectmat._42 = 0.0f;
	selectmat._43 = 0.0f;

	axis0 = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	D3DXVec3TransformCoord( &rotaxis, &axis0, &selectmat );
	D3DXVec3Normalize( &rotaxis, &rotaxis );
	rotrad = delta / 10.0f * (float)PAI / 12.0f;

	if( fabs(rotrad) < (0.02f * (float)DEG2PAI) ){
		return 0;
	}

	CQuaternion rotq;
	rotq.SetAxisAndRot( rotaxis, rotrad );

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange( &keynum, &startframe, &endframe, &applyframe );
	
	curbone = firstbone;
	double firstframe = 0.0;

	if( keynum >= 2 ){
		int keyno = 0;
		list<KeyInfo>::iterator itrki;
		for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
			double curframe = itrki->time;
			double changerate;
			if( curframe <= applyframe ){
				if( applyframe != startframe ){
					changerate = 1.0 / (applyframe - startframe);
				}else{
					changerate = 1.0;
				}
			}else{
				if( applyframe != endframe ){
					changerate = 1.0 / (endframe - applyframe );
				}else{
					changerate = 0.0;
				}
			}

			if( keyno == 0 ){
				firstframe = curframe;
			}
			if( g_absikflag == 0 ){
				if( g_slerpoffflag == 0 ){
					double currate2;
					CQuaternion endq;
					CQuaternion curq;
					endq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
					if( curframe <= applyframe ){
						if( applyframe != startframe ){
							currate2 = changerate * (curframe - startframe);
						}else{
							currate2 = 1.0;
						}
					}else{
						currate2 = changerate * (endframe - curframe);
					}
					rotq.Slerp2( endq, 1.0 - currate2, &curq );

					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, curq );
				}else{
					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
				}
			}else{
				if( keyno == 0 ){
					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
				}else{
					curbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );
				}
			}
			keyno++;
		}

		if( g_applyendflag == 1 ){
			//curmotinfo->curframeから最後までcurmotinfo->curframeの姿勢を適用
			int tolast;
			for( tolast = m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++ ){
				(m_bonelist[ 0 ])->PasteRotReq( m_curmotinfo->motid, m_curmotinfo->curframe, tolast );
			}
		}


	}else{
		curbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
	}

	UpdateMatrix( curbone->m_boneno, &m_matWorld, &m_matVP );

	if( g_absikflag && curbone ){
		AdjustBoneTra( erptr, curbone );
	}

	return curbone->m_boneno;
}


int CModel::FKRotate( CEditRange* erptr, int srcboneno, int srcmotid, double srcframe, CQuaternion rotq )
{
	static CMotionPoint* s_dbgmp0 = 0;
	static CMotionPoint* s_dbgmp1 = 0;

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
	SetBefEditMatFK( erptr, curbone );

	CBone* lastpar = firstbone->m_parent;


	int keynum;
	double startframe, endframe;
	erptr->GetRange( &keynum, &startframe, &endframe );

	curbone = firstbone;
	double firstframe = 0.0;

	if( keynum >= 2 ){
		float changerate = 1.0 / (float)(endframe - startframe);

		D3DXMATRIX inimat;
		D3DXMatrixIdentity( &inimat );

		int keyno = 0;
		list<KeyInfo>::iterator itrki;
		for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
			double curframe = itrki->time;
			if( keyno == 0 ){
				firstframe = curframe;
			}
			if( g_absikflag == 0 ){
				if( g_slerpoffflag == 0 ){
					double currate2;
					currate2 = changerate * keyno;
					CQuaternion endq;
					endq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
					CQuaternion curq;
					rotq.Slerp2( endq, 1.0 - currate2, &curq );

					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, curq );
				}else{
					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );
				}
			}else{
				if( keyno == 0 ){
					curbone->RotBoneQReq( 0, m_curmotinfo->motid, curframe, rotq );

					int existflag0 = 0;
					CMotionPoint* pbef0 = 0;
					CMotionPoint* pnext0 = 0;
					curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &pbef0, &pnext0, &existflag0 );
					_ASSERT( existflag0 );
					s_dbgmp0 = pbef0;

				}else{
					curbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );

					int existflag1 = 0;
					CMotionPoint* pbef1 = 0;
					CMotionPoint* pnext1 = 0;
					curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &pbef1, &pnext1, &existflag1 );
					_ASSERT( existflag1 );
					s_dbgmp1 = pbef1;
					//_ASSERT( 0 );
				}
			}
			keyno++;
		}
	}else{
		curbone->RotBoneQReq( 0, m_curmotinfo->motid, m_curmotinfo->curframe, rotq );
	}
	UpdateMatrix( curbone->m_boneno, &m_matWorld, &m_matVP );

	if( g_absikflag && curbone ){
		AdjustBoneTra( erptr, curbone );
	}

	return curbone->m_boneno;
}

int CModel::FKBoneTra( CEditRange* erptr, int srcboneno, int srcmotid, double srcframe, D3DXVECTOR3 addtra )
{
	static CMotionPoint* s_dbgmp0 = 0;
	static CMotionPoint* s_dbgmp1 = 0;

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
	SetBefEditMatFK( erptr, curbone );

	CBone* lastpar = firstbone->m_parent;

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange( &keynum, &startframe, &endframe, &applyframe );

	curbone = firstbone;
	double firstframe = 0.0;

	if( keynum >= 2 ){
		float changerate = 1.0 / (float)(endframe - startframe);

		int keyno = 0;
		list<KeyInfo>::iterator itrki;
		for( itrki = erptr->m_ki.begin(); itrki != erptr->m_ki.end(); itrki++ ){
			double curframe = itrki->time;
			double changerate;
			if( curframe <= applyframe ){
				if( applyframe != startframe ){
					changerate = 1.0 / (applyframe - startframe);
				}else{
					changerate = 1.0;
				}
			}else{
				if( applyframe != endframe ){
					changerate = 1.0 / (endframe - applyframe );
				}else{
					changerate = 0.0;
				}
			}

			if( keyno == 0 ){
				firstframe = curframe;
			}
			if( g_absikflag == 0 ){
				if( g_slerpoffflag == 0 ){
					double currate2;
					if( curframe <= applyframe ){
						if( applyframe != startframe ){
							currate2 = changerate * (curframe - startframe);
						}else{
							currate2 = 1.0;
						}
					}else{
						currate2 = changerate * (endframe - curframe);
					}
					D3DXVECTOR3 curtra;
					curtra = currate2 * addtra;

					//currate2 = changerate * keyno;
					//D3DXVECTOR3 curtra;
					//curtra = (1.0 - currate2) * addtra;

					curbone->AddBoneTraReq( 0, m_curmotinfo->motid, curframe, curtra );
				}else{
					curbone->AddBoneTraReq( 0, m_curmotinfo->motid, curframe, addtra );
				}
			}else{
				if( keyno == 0 ){
					curbone->AddBoneTraReq( 0, m_curmotinfo->motid, curframe, addtra );

					int existflag0 = 0;
					CMotionPoint* pbef0 = 0;
					CMotionPoint* pnext0 = 0;
					curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &pbef0, &pnext0, &existflag0 );
					_ASSERT( existflag0 );
					s_dbgmp0 = pbef0;
				}else{
					curbone->SetAbsMatReq( 0, m_curmotinfo->motid, curframe, firstframe );

					int existflag1 = 0;
					CMotionPoint* pbef1 = 0;
					CMotionPoint* pnext1 = 0;
					curbone->GetBefNextMP( m_curmotinfo->motid, curframe, &pbef1, &pnext1, &existflag1 );
					_ASSERT( existflag1 );
					s_dbgmp1 = pbef1;
				}
			}
			keyno++;

		}
	}else{
		curbone->AddBoneTraReq( 0, m_curmotinfo->motid,  srcframe, addtra );
	}
	UpdateMatrix( curbone->m_boneno, &m_matWorld, &m_matVP );

	return curbone->m_boneno;
}

int CModel::InitUndoMotion( int saveflag )
{
	int undono;
	for( undono = 0; undono < UNDOMAX; undono++ ){
		m_undomotion[ undono ].ClearData();
	}

	m_undoid = 0;

	if( saveflag ){
		m_undomotion[0].SaveUndoMotion( this, -1, -1 );
	}


	return 0;
}

int CModel::SaveUndoMotion( int curboneno, int curbaseno )
{
	if( m_bonelist.empty() || !m_curmotinfo ){
		return 0;
	}

	int nextundoid;
	nextundoid = m_undoid + 1;
	if( nextundoid >= UNDOMAX ){
		nextundoid = 0;
	}
	m_undoid = nextundoid;

	CallF( m_undomotion[m_undoid].SaveUndoMotion( this, curboneno, curbaseno ), return 1 );

	return 0;
}
int CModel::RollBackUndoMotion( int redoflag, int* curboneno, int* curbaseno )
{
	if( m_bonelist.empty() || !m_curmotinfo ){
		return 0;
	}

	int rbundoid = -1;
	if( redoflag == 0 ){
		GetValidUndoID( &rbundoid );
	}else{
		GetValidRedoID( &rbundoid );
	}

	if( rbundoid >= 0 ){
		m_undomotion[ rbundoid ].RollBackMotion( this, curboneno, curbaseno );
		m_undoid = rbundoid;
	}

	return 0;
}
int CModel::GetValidUndoID( int* rbundoid )
{
	int retid = -1;

	int chkcnt;
	int curid = m_undoid;
	for( chkcnt = 0; chkcnt < UNDOMAX; chkcnt++ ){
		curid = curid - 1;
		if( curid < 0 ){
			curid = UNDOMAX - 1;
		}

		if( m_undomotion[curid].m_validflag == 1 ){
			retid = curid;
			break;
		}
	}
	*rbundoid = retid;
	return 0;
}
int CModel::GetValidRedoID( int* rbundoid )
{
	int retid = -1;

	int chkcnt;
	int curid = m_undoid;
	for( chkcnt = 0; chkcnt < UNDOMAX; chkcnt++ ){
		curid = curid + 1;
		if( curid >= UNDOMAX ){
			curid = 0;
		}

		if( m_undomotion[curid].m_validflag == 1 ){
			retid = curid;
			break;
		}
	}
	*rbundoid = retid;

	return 0;
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

	return GetFbxTargetWeight( m_fbxobj[srcbaseobj].node, m_fbxobj[srcbaseobj].mesh, srctargetname, lTime, curanimlayer, srcbaseobj );
}

float CModel::GetFbxTargetWeight(FbxNode* pbaseNode, FbxMesh* pbaseMesh, std::string targetname, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* baseobj )
{
    int lVertexCount = pbaseMesh->GetControlPointsCount();
	if( lVertexCount != baseobj->m_vertex ){
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
				const char* nameptr = lChannel->GetName();
				int cmp0;
				cmp0 = strcmp( nameptr, targetname.c_str() );
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
