#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>

#define FBXFILECPP
#include <FBXFile.h>
//#include <fbxfilesdk/kfbxtransformation.h>

#define	DBGH
#include <dbg.h>

#include <crtdbg.h>
#include <coef.h>

#include <FBXBone.h>
#include <Model.h>
#include <MQOObject.h>
#include <Bone.h>
#include <PolyMesh4.h>
#include <MQOMaterial.h>
#include <InfBone.h>
#include <MotionPoint.h>
//#include <MorphKey.h>

#include <bvhelem.h>
#include <bvhfile.h>


#define KARCH_ENV_WIN

static FbxNode::EPivotSet s_convPivot;

static float FlClamp( float srcval, float minval, float maxval );

static CBVHElem* s_behead = 0;
static int s_invindex[3] = {0, 2, 1};
static int s_firstanimout = 0;

#include <map>
using namespace std;

typedef struct tag_blsindex
{
	int serialno;
	int blendshapeno;
	int channelno;
}BLSINDEX;
typedef struct tag_blsinfo
{
	BLSINDEX blsindex;
	FbxNode* basenode;
	CMQOObject* base;
	string targetname;
}BLSINFO;
static map<int, BLSINFO> s_blsinfo;

typedef struct tag_animinfo
{
	int motid;
	int orgindex;
	int maxframe;
	char* engmotname;
	FbxAnimLayer* animlayer;
}ANIMINFO;

static ANIMINFO* s_ai = 0;
static int s_ainum = 0;

static CFBXBone* s_fbxbone = 0;

static int sortfunc_leng( void *context, const void *elem1, const void *elem2);

int sortfunc_leng( void *context, const void *elem1, const void *elem2)
{
	ANIMINFO* info1 = (ANIMINFO*)elem1;
	ANIMINFO* info2 = (ANIMINFO*)elem2;

	int diffleng = info1->maxframe - info2->maxframe;
	return diffleng;
}

static FbxManager* s_pSdkManager = 0;
static int s_bvhflag = 0;

//static map<CBone*, FbxNode*> s_bone2skel;
static int s_firstoutmot;

static CFBXBone* CreateFBXBone( FbxScene* pScene, CModel* pmodel );
static void CreateFBXBoneReq( FbxScene* pScene, CBone* pbone, CFBXBone* parfbxbone );

static CFBXBone* CreateFBXBoneOfBVH( FbxScene* pScene );
static void CreateFBXBoneOfBVHReq( FbxScene* pScene, CBVHElem* pbe, CFBXBone* parfbxbone );

static int DestroyFBXBoneReq( CFBXBone* fbxbone );



static void CreateAndFillIOSettings(FbxManager* pSdkManager);
static bool SaveScene(FbxManager* pSdkManager, FbxDocument* pScene, const char* pFilename, int pFileFormat=-1, bool pEmbedMedia=false);


static bool CreateScene(FbxManager* pSdkManager, FbxScene* pScene, CModel* pmodel );
static bool CreateBVHScene(FbxManager* pSdkManager, FbxScene* pScene );
static FbxNode* CreateFbxMesh( FbxManager* pSdkManager, FbxScene* pScene, CModel* pmodel, CMQOObject* curobj );
static FbxNode* CreateSkeleton(FbxScene* pScene, CModel* pmodel);
static void CreateSkeletonReq( FbxScene* pScene, CBone* pbone, CBone* pparbone, FbxNode* pparnode );
static void LinkMeshToSkeletonReq( CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* lMesh, CMQOObject* curobj, CModel* pmodel );

static int WriteBindPose(FbxScene* pScene);
static void WriteBindPoseReq( CFBXBone* fbxbone, FbxPose* lPose );



static void AnimateSkeleton(FbxScene* pScene, CModel* pmodel);
static void AnimateBoneReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int motmax );
static int AnimateMorph(FbxScene* pScene, CModel* pmodel);

static void AnimateSkeletonOfBVH( FbxScene* pScene );
static void AnimateBoneOfBVHReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer );


static FbxTexture*  CreateTexture( FbxManager* pSdkManager, CMQOMaterial* mqomat );
static int ExistBoneInInf( int boneno, CMQOObject* srcobj, int* dstclusterno );

static int MapShapesOnMesh( FbxScene* pScene, FbxNode* pNode, CModel* pmodel, CMQOObject* curobj, BLSINDEX* blsindex );
static int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, D3DXVECTOR3* targetv, int targetcnt );


#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif


float FlClamp( float srcval, float minval, float maxval )
{
	float retval = srcval;

	if( srcval < minval ){
		return minval;
	}else if( srcval > maxval ){
		return maxval;
	}else{
		return retval;
	}
}

int InitializeSdkObjects()
{
    // The first thing to do is to create the FBX SDK manager which is the 
    // object allocator for almost all the classes in the SDK.
    s_pSdkManager = FbxManager::Create();

    if (!s_pSdkManager)
    {
		_ASSERT( 0 );
		return 1;
	}

	// create an IOSettings object
	FbxIOSettings * ios = FbxIOSettings::Create(s_pSdkManager, IOSROOT );
	s_pSdkManager->SetIOSettings(ios);

	// Load plugins from the executable directory
	FbxString lPath = FbxGetApplicationDirectory();
#if defined(KARCH_ENV_WIN)
	FbxString lExtension = "dll";
#elif defined(KARCH_ENV_MACOSX)
	FbxString lExtension = "dylib";
#elif defined(KARCH_ENV_LINUX)
	FbxString lExtension = "so";
#endif
	s_pSdkManager->LoadPluginsDirectory(lPath.Buffer(), lExtension.Buffer());

	return 0;
}

int DestroySdkObjects()
{
    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
    if (s_pSdkManager) s_pSdkManager->Destroy();
    s_pSdkManager = 0;


	return 0;
}

bool SaveScene(FbxManager* pSdkManager, FbxDocument* pScene, const char* pFilename, int pFileFormat, bool pEmbedMedia)
{
    int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

    if( pFileFormat < 0 || pFileFormat >= pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount() )
    {
        // Write in fall back format in less no ASCII format found
        pFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

        //Try to export in ASCII if possible
        int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

        for (lFormatIndex=0; lFormatIndex<lFormatCount; lFormatIndex++)
        {
            if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
            {
                FbxString lDesc =pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
                char *lASCII = "ascii";
                if (lDesc.Find(lASCII)>=0)
                {
                    pFileFormat = lFormatIndex;
                    break;
                }
            }
        }
    }

    // Set the export states. By default, the export states are always set to 
    // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
    // shows how to change these states.

    IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
    IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
    IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
    IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
    IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
    IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
    IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    // Initialize the exporter by providing a filename.
    if(lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
    {
        printf("Call to FbxExporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lExporter->GetLastErrorString());
        return false;
    }

    FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
    printf("FBX version number for this version of the FBX SDK is %d.%d.%d\n\n", lMajor, lMinor, lRevision);

    // Export the scene.
    lStatus = lExporter->Export(pScene); 

    // Destroy the exporter.
    lExporter->Destroy();
    return lStatus;
}

int BVH2FBXFile( FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename )
{
	s_bvhflag = 1;
	s_behead = pbvhfile->m_behead;

	s_pSdkManager = psdk;

	if( s_fbxbone ){
		DestroyFBXBoneReq( s_fbxbone );
		s_fbxbone = 0;
	}


	s_firstoutmot = -1;

    bool lResult;
    // Create the entity that will hold the scene.
	FbxScene* lScene;
    lScene = FbxScene::Create(s_pSdkManager,"");
    // Create the scene.
    lResult = CreateBVHScene( s_pSdkManager, lScene );// create, set
    if(lResult == false)
    {
		_ASSERT( 0 );
		return 1;
    }

    // Save the scene.
    // The example can take an output file name as an argument.
    lResult = SaveScene( s_pSdkManager, lScene, pfilename );// write
    if(lResult == false)
    {
		_ASSERT( 0 );
		return 1;
	}

	if( s_fbxbone ){
		DestroyFBXBoneReq( s_fbxbone );
		s_fbxbone = 0;
	}


	return 0;
}


int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename )
{
	s_bvhflag = 0;
	s_pSdkManager = psdk;

	if( s_fbxbone ){
		DestroyFBXBoneReq( s_fbxbone );
		s_fbxbone = 0;
	}


	s_firstoutmot = -1;

	CallF( pmodel->MakeEnglishName(), return 1 );

    bool lResult;

    // Create the entity that will hold the scene.
	FbxScene* lScene;
    lScene = FbxScene::Create(s_pSdkManager,"");

    // Create the scene.
    lResult = CreateScene( s_pSdkManager, lScene, pmodel );

    if(lResult == false)
    {
		_ASSERT( 0 );
		return 1;
    }

    // Save the scene.

    // The example can take an output file name as an argument.
    lResult = SaveScene( s_pSdkManager, lScene, pfilename );

    if(lResult == false)
    {
		_ASSERT( 0 );
		return 1;
	}

	if( s_fbxbone ){
		DestroyFBXBoneReq( s_fbxbone );
		s_fbxbone = 0;
	}

	return 0;
}

bool CreateBVHScene( FbxManager *pSdkManager, FbxScene* pScene )
{
    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
    sceneInfo->mTitle = "scene made by OpenRDB";
    sceneInfo->mSubject = "skinmesh and animation";
    sceneInfo->mAuthor = "OpenRDB user";
    sceneInfo->mRevision = "rev. 1.0";
    sceneInfo->mKeywords = "skinmesh animation";
    sceneInfo->mComment = "no particular comments required.";

    // we need to add the sceneInfo before calling AddThumbNailToScene because
    // that function is asking the scene for the sceneInfo.
    pScene->SetSceneInfo(sceneInfo);

//    AddThumbnailToScene(pScene);

    FbxNode* lRootNode = pScene->GetRootNode();

	s_fbxbone = CreateFBXBoneOfBVH( pScene );
	lRootNode->AddChild( s_fbxbone->skelnode );
	if( !s_fbxbone ){
		_ASSERT( 0 );
		return 0;
	}

/***
	BLSINDEX blsindex;
	ZeroMemory( &blsindex, sizeof( BLSINDEX ) );
	s_blsinfo.clear();
	map<int,CMQOObject*>::iterator itrobj;
	for( itrobj = pmodel->m_object.begin(); itrobj != pmodel->m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		FbxNode* lMesh = CreateFbxMesh( pSdkManager, pScene, pmodel, curobj );
		if( !(curobj->m_findshape.empty()) ){
			MapShapesOnMesh( pScene, lMesh, pmodel, curobj, &blsindex );
		}
		lRootNode->AddChild(lMesh);

		FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
		FbxSkin* lSkin = FbxSkin::Create(pScene, "");
		LinkMeshToSkeletonReq( s_fbxbone, lSkin, pScene, lMesh, curobj, pmodel);
		lMeshAttribute->AddDeformer(lSkin);
	}
***/

//    StoreRestPose(pScene, lSkeletonRoot);

	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    AnimateSkeletonOfBVH( pScene );

	WriteBindPose(pScene);

	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    return true;
}



bool CreateScene( FbxManager *pSdkManager, FbxScene* pScene, CModel* pmodel )
{
    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
    sceneInfo->mTitle = "scene made by OpenRDB";
    sceneInfo->mSubject = "skinmesh and animation";
    sceneInfo->mAuthor = "OpenRDB user";
    sceneInfo->mRevision = "rev. 1.0";
    sceneInfo->mKeywords = "skinmesh animation";
    sceneInfo->mComment = "no particular comments required.";

    // we need to add the sceneInfo before calling AddThumbNailToScene because
    // that function is asking the scene for the sceneInfo.
    pScene->SetSceneInfo(sceneInfo);

//    AddThumbnailToScene(pScene);

    FbxNode* lRootNode = pScene->GetRootNode();

	s_fbxbone = CreateFBXBone(pScene, pmodel);
	lRootNode->AddChild( s_fbxbone->skelnode );
	if( !s_fbxbone ){
		_ASSERT( 0 );
		return 0;
	}

	BLSINDEX blsindex;
	ZeroMemory( &blsindex, sizeof( BLSINDEX ) );
	s_blsinfo.clear();
	map<int,CMQOObject*>::iterator itrobj;
	for( itrobj = pmodel->m_object.begin(); itrobj != pmodel->m_object.end(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		FbxNode* lMesh = CreateFbxMesh( pSdkManager, pScene, pmodel, curobj );
		if( !(curobj->m_findshape.empty()) ){
			MapShapesOnMesh( pScene, lMesh, pmodel, curobj, &blsindex );
		}
		lRootNode->AddChild(lMesh);

		FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
		FbxSkin* lSkin = FbxSkin::Create(pScene, "");
		LinkMeshToSkeletonReq( s_fbxbone, lSkin, pScene, lMesh, curobj, pmodel);
		lMeshAttribute->AddDeformer(lSkin);
	}

//    StoreRestPose(pScene, lSkeletonRoot);

	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    AnimateSkeleton(pScene, pmodel);
	AnimateMorph(pScene, pmodel);

	WriteBindPose(pScene);

	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    return true;
}




//void MapBoxShape(FbxScene* pScene, FbxBlendShapeChannel* lBlendShapeChannel)
//int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, CMQOObject* curtarget, MATERIALBLOCK* pmb, int mbno )
int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, D3DXVECTOR3* targetv, int targetcnt )
{
	char shapename[256]={0};
	sprintf_s( shapename, 256, "SHAPE_%s_%d", curobj->m_engname, targetcnt );
    FbxShape* lShape = FbxShape::Create(pScene,shapename);

	CPolyMesh4* basepm4 = curobj->m_pm4;
	_ASSERT( basepm4 );

	int shapevertnum = basepm4->m_facenum * 3;
	lShape->InitControlPoints( shapevertnum );
    FbxVector4* lVector4 = lShape->GetControlPoints();

	int shapevno;
	for( shapevno = 0; shapevno < shapevertnum; shapevno++ ){
		int orgvno = *( basepm4->m_dispindex + shapevno );

		D3DXVECTOR3 shapev = *( targetv + orgvno );
		lVector4[ shapevno ].Set( shapev.x, shapev.y, shapev.z, 1.0 );
	}
	lBlendShapeChannel->AddTargetShape(lShape);
	return 0;
}


int MapShapesOnMesh( FbxScene* pScene, FbxNode* pNode, CModel* pmodel, CMQOObject* curobj, BLSINDEX* blsindex )
{
	char blsname[256] = {0};
	int mbno = 0;//mqo‚Å‚Â‚©‚¤
	sprintf_s( blsname, 256, "BLS_%s_%d", curobj->m_engname, mbno );
	FbxBlendShape* lBlendShape = FbxBlendShape::Create(pScene, blsname);

	(blsindex->channelno) = 0;

	int targetcnt = 0;
	map<string,D3DXVECTOR3*>::iterator itrshapev;
	for( itrshapev = curobj->m_shapevert.begin(); itrshapev != curobj->m_shapevert.end(); itrshapev++ ){
		string targetname = itrshapev->first;
		D3DXVECTOR3* curv = itrshapev->second;
		if( curv ){
			string curshapename = itrshapev->first;

			char blscname[256] = {0};
			strcpy_s( blscname, 256, curshapename.c_str() );
			FbxBlendShapeChannel* lBlendShapeChannel = FbxBlendShapeChannel::Create(pScene,blscname);
			MapTargetShape( lBlendShapeChannel, pScene, curobj, curv, targetcnt );
			lBlendShape->AddBlendShapeChannel(lBlendShapeChannel);


			BLSINFO blsinfo;
			ZeroMemory( &blsinfo, sizeof( BLSINFO ) );

			blsinfo.blsindex = *blsindex;
			blsinfo.base = curobj;
			blsinfo.targetname = targetname;
			blsinfo.basenode = pNode;

			s_blsinfo[ blsindex->serialno ] = blsinfo;

			(blsindex->channelno)++;
			(blsindex->serialno)++;

			targetcnt++;
		}
	}


	FbxGeometry* lGeometry = pNode->GetGeometry();
	lGeometry->AddDeformer(lBlendShape);
	return 0;
};


FbxNode* CreateFbxMesh(FbxManager* pSdkManager, FbxScene* pScene, CModel* pmodel, CMQOObject* curobj)
{
	CPolyMesh4* pm4 = curobj->m_pm4;
	_ASSERT( pm4 );
	CMQOMaterial* mqomat = curobj->m_material.begin()->second;
	_ASSERT( mqomat );
	char meshname[256] = {0};

	sprintf_s( meshname, 256, "%s", curobj->m_engname );
	int facenum = pm4->m_facenum;

	FbxMesh* lMesh = FbxMesh::Create( pScene, meshname );
	lMesh->InitControlPoints( facenum * 3 );
	FbxVector4* lcp = lMesh->GetControlPoints();

	FbxGeometryElementNormal* lElementNormal= lMesh->CreateElementNormal();
	lElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

	FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV( "DiffuseUV");
	_ASSERT( lUVDiffuseElement != NULL);
	lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

	_ASSERT( pm4->m_dispv );

	int vsetno = 0;
	int faceno;
	for( faceno = 0; faceno < facenum; faceno++ ){
		PM3DISPV* curdispv = pm4->m_dispv + faceno * 3;
		int vcnt;
		for( vcnt = 0; vcnt < 3; vcnt++ ){
			//PM3DISPV* curv = curdispv + vcnt;
			PM3DISPV* curv = curdispv + s_invindex[vcnt];
			*( lcp + vsetno ) = FbxVector4( curv->pos.x, curv->pos.y, curv->pos.z, curv->pos.w );

			FbxVector4 fbxn = FbxVector4( curv->normal.x, curv->normal.y, curv->normal.z, 0.0f );
			lElementNormal->GetDirectArray().Add( fbxn );

			FbxVector2 fbxuv = FbxVector2( curv->uv.x, 1.0f - curv->uv.y );
			lUVDiffuseElement->GetDirectArray().Add( fbxuv );

			vsetno++;
		}
	}
	lUVDiffuseElement->GetIndexArray().SetCount(facenum * 3);

	vsetno = 0;
	for( faceno = 0; faceno < facenum; faceno++ ){
		lMesh->BeginPolygon(-1, -1, -1, false);
		int vcnt;
		for( vcnt = 0; vcnt < 3; vcnt++ ){
			lMesh->AddPolygon( vsetno );  
			lUVDiffuseElement->GetIndexArray().SetAt( vsetno, vsetno );
			vsetno++;
		}
		lMesh->EndPolygon ();
	}


	// create a FbxNode
//		FbxNode* lNode = FbxNode::Create(pSdkManager,pName);
	FbxNode* lNode = FbxNode::Create( pScene, meshname );
	// set the node attribute
	lNode->SetNodeAttribute(lMesh);
	// set the shading mode to view texture
	lNode->SetShadingMode(FbxNode::eTextureShading);
	// rotate the plane
	lNode->LclRotation.Set(FbxVector4(0, 0, 0));

	// Set material mapping.
	FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
	lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
	lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
	if( !lMesh->GetElementMaterial(0) ){
		_ASSERT( 0 );
		return NULL;
	}

	// add material to the node. 
	// the material can't in different document with the geometry node or in sub-document
	// we create a simple material here which belong to main document
	static int s_matcnt = 0;
	s_matcnt++;

	char matname[256];
	sprintf_s( matname, 256, "%s_%d", mqomat->name, s_matcnt );
	//FbxString lMaterialName = mqomat->name;
	FbxString lMaterialName = matname;
	FbxString lShadingName  = "Phong";
	FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create( pScene, lMaterialName.Buffer() );

	lMaterial->Diffuse.Set(FbxDouble3(mqomat->dif4f.x, mqomat->dif4f.y, mqomat->dif4f.z));
    lMaterial->Emissive.Set(FbxDouble3(mqomat->emi3f.x, mqomat->emi3f.y, mqomat->emi3f.z));
    lMaterial->Ambient.Set(FbxDouble3(mqomat->amb3f.x, mqomat->amb3f.y, mqomat->amb3f.z));
    lMaterial->AmbientFactor.Set(1.0);
	FbxTexture* curtex = CreateTexture(pSdkManager, mqomat);
	if( curtex ){
		lMaterial->Diffuse.ConnectSrcObject( curtex );
	}
    lMaterial->DiffuseFactor.Set(1.0);
    lMaterial->TransparencyFactor.Set(mqomat->dif4f.w);
    lMaterial->ShadingModel.Set(lShadingName);
    lMaterial->Shininess.Set(0.5);
    lMaterial->Specular.Set(FbxDouble3(mqomat->spc3f.x, mqomat->spc3f.y, mqomat->spc3f.z));
    lMaterial->SpecularFactor.Set(0.3);

	lNode->AddMaterial(lMaterial);
	// We are in eByPolygon, so there's only need for index (a plane has 1 polygon).
	lMaterialElement->GetIndexArray().SetCount( lMesh->GetPolygonCount() );
	// Set the Index to the material
	for(int i=0; i<lMesh->GetPolygonCount(); ++i){
		lMaterialElement->GetIndexArray().SetAt(i,0);
	}

	return lNode;
}

FbxTexture*  CreateTexture(FbxManager* pSdkManager, CMQOMaterial* mqomat)
{
	if( !mqomat->tex[0] ){
		return NULL;
	}

    FbxFileTexture* lTexture = FbxFileTexture::Create(pSdkManager,"");
    FbxString lTexPath = mqomat->tex;

    // Set texture properties.
    lTexture->SetFileName(lTexPath.Buffer());
    //lTexture->SetName("Diffuse Texture");
	lTexture->SetName(mqomat->tex);
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetAlphaSource (FbxTexture::eNone);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);

    return lTexture;
}




void LinkMeshToSkeletonReq( CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CModel* pmodel )
{
	CPolyMesh4* pm4 = curobj->m_pm4;
	_ASSERT( pm4 );
    FbxGeometry* lMeshAttribute = (FbxGeometry*)pMesh->GetNodeAttribute();
	FbxAMatrix lXMatrix;
    FbxNode* lSkel;

	if( fbxbone->type == FB_NORMAL ){
		CBone* curbone = fbxbone->bone;

		if( curbone ){
			lSkel = fbxbone->skelnode;
			if( !lSkel ){
				_ASSERT( 0 );
				return;
			}

			int curclusterno = -1;
			int infdirty = ExistBoneInInf( curbone->m_boneno, curobj, &curclusterno );

			if( infdirty ){
				FbxCluster *lCluster = FbxCluster::Create(pScene,"");
				lCluster->SetLink(lSkel);
				lCluster->SetLinkMode(FbxCluster::eTotalOne);

				int vsetno = 0;
				int fno;
				for( fno = 0; fno < pm4->m_facenum; fno++ ){
					int vcnt;
					for( vcnt = 0; vcnt < 3; vcnt++ ){
						int orgvno = *( pm4->m_orgindex + fno * 3 + s_invindex[vcnt] );
						if( (orgvno >= 0) && (orgvno < pm4->m_orgpointnum) ){
							CInfBone* curib = pm4->m_infbone + orgvno;
							int ieno = curib->ExistBone( curobj, curclusterno );
							if( ieno >= 0 ){
								INFDATA* infd = curib->m_infdata[ curobj ];
								if( infd ){
									lCluster->AddControlPointIndex( vsetno, (double)( infd->m_infelem[ieno].dispinf ) );
								}
							}
							vsetno++;
						}
					}
				}

				//FbxScene* lScene = pMesh->GetScene();
				lXMatrix = pMesh->EvaluateGlobalTransform();
				lCluster->SetTransformMatrix(lXMatrix);

				D3DXMATRIX xmat;
				xmat._11 = (float)lXMatrix.Get( 0, 0 );
				xmat._12 = (float)lXMatrix.Get( 0, 1 );
				xmat._13 = (float)lXMatrix.Get( 0, 2 );
				xmat._14 = (float)lXMatrix.Get( 0, 3 );

				xmat._21 = (float)lXMatrix.Get( 1, 0 );
				xmat._22 = (float)lXMatrix.Get( 1, 1 );
				xmat._23 = (float)lXMatrix.Get( 1, 2 );
				xmat._24 = (float)lXMatrix.Get( 1, 3 );

				xmat._31 = (float)lXMatrix.Get( 2, 0 );
				xmat._32 = (float)lXMatrix.Get( 2, 1 );
				xmat._33 = (float)lXMatrix.Get( 2, 2 );
				xmat._34 = (float)lXMatrix.Get( 2, 3 );

				xmat._41 = 0.0f;
				xmat._42 = 0.0f;
				xmat._43 = 0.0f;
				xmat._44 = (float)lXMatrix.Get( 3, 3 );

				D3DXQUATERNION xq;
				D3DXQuaternionRotationMatrix( &xq, &xmat );
				fbxbone->axisq.SetParams( xq );

				//lXMatrix.SetIdentity();
				//lXMatrix[3][0] = -curbone->m_vertpos[BT_PARENT].x;
				//lXMatrix[3][1] = -curbone->m_vertpos[BT_PARENT].y;
				//lXMatrix[3][2] = curbone->m_vertpos[BT_PARENT].z;
				//lCluster->SetTransformMatrix(lXMatrix);

				lXMatrix = lSkel->EvaluateGlobalTransform();
				lCluster->SetTransformLinkMatrix(lXMatrix);
				//lXMatrix.SetIdentity();
				//lCluster->SetTransformLinkMatrix(lXMatrix);

				lSkin->AddCluster(lCluster);
			}
		}
	}

	if( fbxbone->m_child ){
		LinkMeshToSkeletonReq(fbxbone->m_child, lSkin, pScene, pMesh, curobj, pmodel);
	}
	if( fbxbone->m_brother ){
		LinkMeshToSkeletonReq(fbxbone->m_brother, lSkin, pScene, pMesh, curobj, pmodel);
	}

}

void AnimateSkeletonOfBVH( FbxScene* pScene )
{
    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;


	lAnimStackName = "bvh animation nyan";
	FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);
	FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

	s_firstanimout = 1;
	AnimateBoneOfBVHReq( s_fbxbone, lAnimLayer );

	//pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, s_convPivot, 30.0, true );
	//pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, FbxNode::eDestinationPivot, 30.0, true );
	//pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, FbxNode::eSourcePivot, 30.0, true );


}
void AnimateBoneOfBVHReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer )
{

	FbxTime lTime;
    int lKeyIndex = 0;
    FbxNode* lSkel = 0;

	//double timescale = 30.0;
	double timescale = 30.0;

//	if( (fbxbone->type == FB_NORMAL) || (fbxbone->type == FB_ROOT) ){
	if( 1 ){
		CBVHElem* curbe = fbxbone->pbe;
		_ASSERT( curbe );
		if( curbe ){

			lSkel = fbxbone->skelnode;
			if( !lSkel ){
				_ASSERT( 0 );
				return;
			}



			EFbxRotationOrder lRotationOrderSrc = eEulerZXY;
			EFbxRotationOrder lRotationOrderDst = eEulerXYZ;
			lSkel->GetRotationOrder( FbxNode::eSourcePivot, lRotationOrderSrc );
			lSkel->GetRotationOrder( FbxNode::eDestinationPivot, lRotationOrderDst );

			lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ );
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY );
			s_convPivot =  FbxNode::eSourcePivot;

/***
			EFbxRotationOrder lRotationOrder0 = eEulerXYZ;
			EFbxRotationOrder lRotationOrder1 = eEulerYXZ;
			//EFbxRotationOrder lRotationOrder1 = eEulerYXZ;
			//EFbxRotationOrder lRotationOrder1 = eEulerZXY;
			//EFbxRotationOrder lRotationOrder1 = eEulerXYZ;
			lSkel->SetRotationOrder(FbxNode::eSourcePivot , lRotationOrder0 );
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot , lRotationOrder1 );
			s_convPivot =  FbxNode::eSourcePivot;
***/

			FbxAnimCurve* lCurveTX;
			FbxAnimCurve* lCurveTY;
			FbxAnimCurve* lCurveTZ;

			int topflag = 0;


			int frameno;
			CMotionPoint* calcmp;
			D3DXVECTOR3 difftra, orgtra, settra;
			CBVHElem* parbe = curbe->parent;
			if( parbe ){
				if( parbe == s_fbxbone->pbe ){
					orgtra = curbe->position;
				}else{
					orgtra = curbe->position - parbe->position;
				}
			}else{
				orgtra = curbe->position - s_fbxbone->pbe->position;
				topflag = 1;
			}


			lCurveTX = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
			lCurveTX->KeyModifyBegin();
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				curbe->CalcDiffTra( frameno, &difftra );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTX->KeyAdd( lTime );
				lCurveTX->KeySetValue( lKeyIndex, difftra.x + orgtra.x );
				lCurveTX->KeySetInterpolation( lKeyIndex, FbxAnimCurveDef::eInterpolationLinear );
			}
			lCurveTX->KeyModifyEnd();

			lCurveTY = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
			lCurveTY->KeyModifyBegin();
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				curbe->CalcDiffTra( frameno, &difftra );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTY->KeyAdd( lTime );
				lCurveTY->KeySetValue(lKeyIndex, difftra.y + orgtra.y);
				lCurveTY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			}
			lCurveTY->KeyModifyEnd();

			lCurveTZ = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
			lCurveTZ->KeyModifyBegin();
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				curbe->CalcDiffTra( frameno, &difftra );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTZ->KeyAdd( lTime );
				lCurveTZ->KeySetValue(lKeyIndex, difftra.z + orgtra.z);
				lCurveTZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			}
			lCurveTZ->KeyModifyEnd();

/////////////////////
			D3DXVECTOR3 befeul( 0.0f, 0.0f, 0.0f );
			D3DXVECTOR3 cureul( 0.0f, 0.0f, 0.0f );

			float befval;
			float curval;
			float diffval;


			befval = (curbe->rotate + 1)->x;
			curval = befval;

			FbxAnimCurve* lCurveRX;
//			lCurveRX = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_X", true);
			lCurveRX = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
			lCurveRX->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRX->KeyAdd( lTime );

				diffval = (curbe->rotate + frameno)->x - befval;
				if( fabs( diffval ) <= 30.0 ){
					curval = (curbe->rotate + frameno)->x;
				}else{
					curval = befval;
				}
				lCurveRX->KeySetValue(lKeyIndex, curval );
				//lCurveRX->KeySetValue(lKeyIndex, FlClamp( (curbe->rotate + frameno)->x, 0.0f, 89.0f ) );
				//lCurveRX->KeySetValue(lKeyIndex, 0.0);
				lCurveRX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
				befval = curval;
			}
			lCurveRX->KeyModifyEnd();


			befval = (curbe->rotate + 1)->y;
			curval = befval;
			FbxAnimCurve* lCurveRY;
//			lCurveRY = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_Y", true);
			lCurveRY = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
			lCurveRY->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRY->KeyAdd( lTime );
				diffval = (curbe->rotate + frameno)->y - befval;
				if( fabs( diffval ) <= 30.0 ){
					curval = (curbe->rotate + frameno)->y;
				}else{
					curval = befval;
				}
				lCurveRY->KeySetValue(lKeyIndex, curval );
				//lCurveRX->KeySetValue(lKeyIndex, FlClamp( (curbe->rotate + frameno)->x, 0.0f, 89.0f ) );
				//lCurveRX->KeySetValue(lKeyIndex, 0.0);
				lCurveRY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
				befval = curval;
			}
			lCurveRY->KeyModifyEnd();
///////////////

			befval = (curbe->rotate + 1)->z;
			curval = befval;
			FbxAnimCurve* lCurveRZ;
//			lCurveRZ = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_Z", true);
			lCurveRZ = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
			lCurveRZ->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= curbe->framenum; frameno++ ){
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRZ->KeyAdd( lTime );
				diffval = (curbe->rotate + frameno)->z - befval;
				if( fabs( diffval ) <= 30.0 ){
					curval = (curbe->rotate + frameno)->z;
				}else{
					curval = befval;
				}
				lCurveRZ->KeySetValue(lKeyIndex, curval );
				//lCurveRX->KeySetValue(lKeyIndex, FlClamp( (curbe->rotate + frameno)->x, 0.0f, 89.0f ) );
				//lCurveRX->KeySetValue(lKeyIndex, 0.0);
				lCurveRZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
				befval = curval;
			}
			lCurveRZ->KeyModifyEnd();


		}
	}

	if( fbxbone->m_child ){
		AnimateBoneOfBVHReq( fbxbone->m_child, lAnimLayer );
	}
	if( fbxbone->m_brother ){
		AnimateBoneOfBVHReq( fbxbone->m_brother, lAnimLayer );
	}

}


void AnimateSkeleton(FbxScene* pScene, CModel* pmodel)
{

    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;

	int motionnum = pmodel->m_motinfo.size();
	if( motionnum <= 0 ){
		return;
	}

	s_ai = (ANIMINFO*)malloc( sizeof( ANIMINFO ) * motionnum );
	if( !s_ai ){
		_ASSERT( 0 );
		return;
	}
	ZeroMemory( s_ai, sizeof( ANIMINFO ) * motionnum );
	s_ainum = motionnum;

	int aino = 0;
	map<int, MOTINFO*>::iterator itrmi;
	for( itrmi = pmodel->m_motinfo.begin(); itrmi != pmodel->m_motinfo.end(); itrmi++ ){
		MOTINFO* curmi = itrmi->second;
		(s_ai + aino)->motid = curmi->motid;
		(s_ai + aino)->orgindex = aino;
		(s_ai + aino)->maxframe = (int)( curmi->frameleng - 1.0 );
		(s_ai + aino)->engmotname = curmi->engmotname;
		aino++;
	}

_ASSERT( motionnum == aino );

	//qsort_s( s_ai, motionnum, sizeof( ANIMINFO ), sortfunc_leng, NULL );

	s_firstoutmot = s_ai->motid;

	for( aino = 0; aino < motionnum; aino++ ){
		ANIMINFO* curai = s_ai + aino;
		int curmotid = curai->motid;
		int maxframe = curai->maxframe;

		lAnimStackName = curai->engmotname;
	    FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);
		FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
		curai->animlayer = lAnimLayer;
        lAnimStack->AddMember(lAnimLayer);

		pmodel->SetCurrentMotion( curmotid );

		s_firstanimout = 1;
		AnimateBoneReq( s_fbxbone, lAnimLayer, curmotid, maxframe );

		pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, s_convPivot, 30.0, true );
		//pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, FbxNode::eDestinationPivot, 30.0, true );
	}

}

void AnimateBoneReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe )
{
	static int s_dbgcnt = 0;

	FbxTime lTime;
    int lKeyIndex = 0;
    FbxNode* lSkel = 0;

	double timescale = 30.0;
	//double timescale = 300.0;

//	if( (fbxbone->type == FB_NORMAL) || (fbxbone->type == FB_ROOT) ){
	if( 1 ){
		CBone* curbone = fbxbone->bone;
		_ASSERT( curbone );
		if( curbone ){

			lSkel = fbxbone->skelnode;
			if( !lSkel ){
				_ASSERT( 0 );
				return;
			}

/***		
			EFbxRotationOrder lRotationOrder0 = eEulerXYZ;
			EFbxRotationOrder lRotationOrder1 = eEulerZXY;
			//EFbxRotationOrder lRotationOrder1 = eEulerXYZ;
			//lSkel->GetRotationOrder( FbxNode::eDestinationPivot, lRotationOrder0 );
			lSkel->SetRotationOrder(FbxNode::eSourcePivot , lRotationOrder0 );
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot , lRotationOrder1 );
***/

			
			EFbxRotationOrder lRotationOrderSrc = eEulerZXY;
			EFbxRotationOrder lRotationOrderDst = eEulerXYZ;
			lSkel->GetRotationOrder( FbxNode::eSourcePivot, lRotationOrderSrc );
			lSkel->GetRotationOrder( FbxNode::eDestinationPivot, lRotationOrderDst );

			
/***
			if( lRotationOrder0 == eEulerZXY ){
				s_convPivot =  FbxNode::eSourcePivot;
			}else{
				s_convPivot =  FbxNode::eDestinationPivot;
			}
***/


			//s_convPivot =  FbxNode::eDestinationPivot;//maya X
			//s_convPivot =  FbxNode::eSourcePivot;
			//lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerZXY );
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ );

			lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ );
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY );
			s_convPivot =  FbxNode::eSourcePivot;
			//maya O, ChaFuBX O


			//s_convPivot =  FbxNode::eDestinationPivot;
			//lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ );
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY );


/***
			if( lRotationOrderDst == eEulerZXY ){
				s_convPivot =  FbxNode::eDestinationPivot;
				//s_convPivot =  FbxNode::eSourcePivot;

				lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerZXY );
				lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ );

//if( s_dbgcnt < 3 ){
//	char dbgmes[256];
//	sprintf_s( dbgmes, 256, "dbgcnt %d : orderdst == ZXY", s_dbgcnt );
//	::MessageBoxA( NULL, dbgmes, "check type0", MB_OK );
//}
			}else{
				s_convPivot =  FbxNode::eSourcePivot;
				//s_convPivot =  FbxNode::eDestinationPivot;

				lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ );
				lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY );

				//maya X, ChaFuBX O

//if( s_dbgcnt < 3 ){
//	char dbgmes[256];
//	sprintf_s( dbgmes, 256, "dbgcnt %d : orderdst != ZXY", s_dbgcnt );
//	::MessageBoxA( NULL, dbgmes, "check type1", MB_OK );
//}
			}
***/

			s_dbgcnt++;


			FbxAnimCurve* lCurveTX;
			FbxAnimCurve* lCurveTY;
			FbxAnimCurve* lCurveTZ;

			int topflag = 0;

/***
			int frameno;
			CMotionPoint* calcmp;
			D3DXVECTOR3 orgtra, settra;
			CBone* parbone = curbone->m_parent;
			if( parbone ){
				if( parbone == s_fbxbone->bone ){
					orgtra = curbone->m_jointfpos;
				}else{
					CBone* gparbone = parbone->m_parent;
					if( gparbone ){
						orgtra = parbone->m_jointfpos - gparbone->m_jointfpos;
					}else{
						orgtra = parbone->m_jointfpos;
					}
				}
			}else{
				orgtra = curbone->m_jointfpos - s_fbxbone->bone->m_jointfpos;
				topflag = 1;
			}
***/
			int frameno;
			CMotionPoint* calcmp;
			D3DXVECTOR3 orgtra, settra;
			CBone* parbone = curbone->m_parent;
			if( parbone ){
				if( parbone == s_fbxbone->bone ){
					orgtra = curbone->m_jointfpos;
				}else{
					orgtra = curbone->m_jointfpos - parbone->m_jointfpos;
				}
			}else{
				orgtra = curbone->m_jointfpos - s_fbxbone->bone->m_jointfpos;
				topflag = 1;
			}


			D3DXVECTOR3 difftra;

			lCurveTX = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
			lCurveTX->KeyModifyBegin();
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTX->KeyAdd( lTime );
				lCurveTX->KeySetValue( lKeyIndex, calcmp->m_tra.x + orgtra.x );
				lCurveTX->KeySetInterpolation( lKeyIndex, FbxAnimCurveDef::eInterpolationLinear );
			}
			lCurveTX->KeyModifyEnd();

			lCurveTY = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
			lCurveTY->KeyModifyBegin();
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTY->KeyAdd( lTime );
				lCurveTY->KeySetValue(lKeyIndex, calcmp->m_tra.y + orgtra.y);
				lCurveTY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			}
			lCurveTY->KeyModifyEnd();

			lCurveTZ = lSkel->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
			lCurveTZ->KeyModifyBegin();
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveTZ->KeyAdd( lTime );
				lCurveTZ->KeySetValue(lKeyIndex, calcmp->m_tra.z + orgtra.z);
				lCurveTZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			}
			lCurveTZ->KeyModifyEnd();

/////////////////////
			D3DXVECTOR3 befeul( 0.0f, 0.0f, 0.0f );
			D3DXVECTOR3 cureul( 0.0f, 0.0f, 0.0f );

			FbxAnimCurve* lCurveRZ;
//			lCurveRZ = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_Z", true);
			lCurveRZ = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
			lCurveRZ->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}

				calcmp->m_q.CalcFBXEul( 0, befeul, &cureul );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRZ->KeyAdd( lTime );
				lCurveRZ->KeySetValue(lKeyIndex, cureul.z);
				//lCurveRZ->KeySetValue(lKeyIndex, 0.0);
				lCurveRZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
			}
			lCurveRZ->KeyModifyEnd();

			FbxAnimCurve* lCurveRX;
//			lCurveRX = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_X", true);
			lCurveRX = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
			lCurveRX->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}
				calcmp->m_q.CalcFBXEul( 0, befeul, &cureul );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRX->KeyAdd( lTime );
				lCurveRX->KeySetValue(lKeyIndex, cureul.x);
				//lCurveRX->KeySetValue(lKeyIndex, 0.0);
				lCurveRX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
			}
			lCurveRX->KeyModifyEnd();

			FbxAnimCurve* lCurveRY;
//			lCurveRY = lSkel->LclTranslation.GetCurve(lAnimLayer, "R_Y", true);
			lCurveRY = lSkel->LclRotation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
			lCurveRY->KeyModifyBegin();
			befeul = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
			for( frameno = 0; frameno <= maxframe; frameno++ ){
				curbone->CalcLocalInfo( curmotid, (double)frameno, &calcmp );
				if( !calcmp ){
					_ASSERT( 0 );
					break;
				}
				calcmp->m_q.CalcFBXEul( 0, befeul, &cureul );
				lTime.SetSecondDouble( (double)frameno / timescale );
				lKeyIndex = lCurveRY->KeyAdd( lTime );
				lCurveRY->KeySetValue(lKeyIndex, cureul.y);
				//lCurveRY->KeySetValue(lKeyIndex, 0.0);
				lCurveRY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
				befeul = cureul;
			}
			lCurveRY->KeyModifyEnd();
///////////////


		}
	}

	if( fbxbone->m_child ){
		AnimateBoneReq( fbxbone->m_child, lAnimLayer, curmotid, maxframe );
	}
	if( fbxbone->m_brother ){
		AnimateBoneReq( fbxbone->m_brother, lAnimLayer, curmotid, maxframe );
	}
}



/***
int WriteBindPose(FbxScene* pScene, CModel* pmodel)
{
	FbxTime lTime0;
	lTime0.SetSecondDouble( 0.0 );

	FbxPose* lPose = FbxPose::Create(pScene,"BindPose0");
	lPose->SetIsBindPose(true);

	if( s_firstoutmot >= 0 ){
		KFbxAnimStack * lCurrentAnimationStack = pScene->FindMember(FBX_TYPE(KFbxAnimStack), s_ai->engmotname);
		if (lCurrentAnimationStack == NULL)
		{
			_ASSERT( 0 );
			return 1;
		}
		FbxAnimLayer * mCurrentAnimLayer;
		mCurrentAnimLayer = lCurrentAnimationStack->GetMember(FBX_TYPE(FbxAnimLayer), 0);
		pScene->GetEvaluator()->SetContext(lCurrentAnimationStack);
	}else{
		_ASSERT( 0 );
	}

	map<int, CBone*>::iterator itrbone;
	for( itrbone = pmodel->m_bonelist.begin(); itrbone != pmodel->m_bonelist.end(); itrbone++ ){
		CBone* curbone = itrbone->second;
		FbxNode* curskel = s_bone2skel[ curbone ];
		if( curskel ){
			if( curskel ){
				FbxAMatrix lBindMatrix = curskel->EvaluateGlobalTransform( lTime0 );
				lPose->Add(curskel, lBindMatrix);
			}
		}
	}

	pScene->AddPose(lPose);

	return 0;
}
***/



int WriteBindPose(FbxScene* pScene)
{

	FbxPose* lPose = FbxPose::Create(pScene,"BindPose0");
	lPose->SetIsBindPose(true);

	if( s_firstoutmot >= 0 ){
		FbxAnimStack * lCurrentAnimationStack = pScene->FindMember(FBX_TYPE(FbxAnimStack), s_ai->engmotname);
		if (lCurrentAnimationStack == NULL)
		{
			_ASSERT( 0 );
			return 1;
		}
		FbxAnimLayer * mCurrentAnimLayer;
		mCurrentAnimLayer = lCurrentAnimationStack->GetMember(FBX_TYPE(FbxAnimLayer), 0);
		pScene->GetEvaluator()->SetContext(lCurrentAnimationStack);
	}else{
		_ASSERT( 0 );
	}

	WriteBindPoseReq( s_fbxbone, lPose );

	pScene->AddPose(lPose);

	return 0;
}

void WriteBindPoseReq( CFBXBone* fbxbone, FbxPose* lPose )
{
	FbxTime lTime0;
	lTime0.SetSecondDouble( 0.0 );
	
	if( fbxbone->type != FB_ROOT ){
		FbxNode* curskel = fbxbone->skelnode;
		if( curskel ){
			FbxAMatrix lBindMatrix = curskel->EvaluateGlobalTransform( lTime0 );
			lPose->Add(curskel, lBindMatrix);
		}
	}

	if( fbxbone->m_child ){
		WriteBindPoseReq( fbxbone->m_child, lPose );
	}
	if( fbxbone->m_brother ){
		WriteBindPoseReq( fbxbone->m_brother, lPose );
	}
}

/***
void StoreBindPose(FbxScene* pScene, FbxNode* pMesh, FbxNode* pSkeletonRoot)
{
    // In the bind pose, we must store all the link's global matrix at the time of the bind.
    // Plus, we must store all the parent(s) global matrix of a link, even if they are not
    // themselves deforming any model.

    // In this example, since there is only one model deformed, we don't need walk through 
    // the scene
    //

    // Now list the all the link involve in the Mesh deformation
    KArrayTemplate<FbxNode*> lClusteredFbxNodes;
    int                       i, j;

    if (pMesh && pMesh->GetNodeAttribute())
    {
        int lSkinCount=0;
        int lClusterCount=0;
        switch (pMesh->GetNodeAttribute()->GetAttributeType())
        {
        case FbxNodeAttribute::eMESH:
        case FbxNodeAttribute::eNURB:

            lSkinCount = ((FbxGeometry*)pMesh->GetNodeAttribute())->GetDeformerCount(KFbxDeformer::eSKIN);
            //Go through all the skins and count them
            //then go through each skin and get their cluster count
            for(i=0; i<lSkinCount; ++i)
            {
                FbxSkin *lSkin=(FbxSkin*)((FbxGeometry*)pMesh->GetNodeAttribute())->GetDeformer(i, KFbxDeformer::eSKIN);
                lClusterCount+=lSkin->GetClusterCount();
            }
            break;
        }
        //if we found some clusters we must add the node
        if (lClusterCount)
        {
            //Again, go through all the skins get each cluster link and add them
            for (i=0; i<lSkinCount; ++i)
            {
                FbxSkin *lSkin=(FbxSkin*)((FbxGeometry*)pMesh->GetNodeAttribute())->GetDeformer(i, KFbxDeformer::eSKIN);
                lClusterCount=lSkin->GetClusterCount();
                for (j=0; j<lClusterCount; ++j)
                {
                    FbxNode* lClusterNode = lSkin->GetCluster(j)->GetLink();
                    AddNodeRecursively(lClusteredFbxNodes, lClusterNode);
                }

            }

            // Add the Mesh to the pose
            lClusteredFbxNodes.Add(pMesh);
        }
    }

    // Now create a bind pose with the link list
    if (lClusteredFbxNodes.GetCount())
    {
        // A pose must be named. Arbitrarily use the name of the Mesh node.
        FbxPose* lPose = FbxPose::Create(pScene,pMesh->GetName());

        // default pose type is rest pose, so we need to set the type as bind pose
        lPose->SetIsBindPose(true);

        for (i=0; i<lClusteredFbxNodes.GetCount(); i++)
        {
            FbxNode*  lFbxNode   = lClusteredFbxNodes.GetAt(i);
            FbxAMatrix lBindMatrix = lFbxNode->EvaluateGlobalTransform();

            lPose->Add(lFbxNode, lBindMatrix);
        }

        // Add the pose to the scene
        pScene->AddPose(lPose);
    }
}
***/

int ExistBoneInInf( int boneno, CMQOObject* srcobj, int* dstclusterno )
{
	*dstclusterno = -1;

	CPolyMesh4* pm4 = srcobj->m_pm4;
	_ASSERT( pm4 );

	int dirtyflag = 0;

	int clusterno = -1;
	CBone* chkbone;
	int clno;
	for( clno = 0; clno < srcobj->m_cluster.size(); clno++ ){
		chkbone = srcobj->m_cluster[ clno ];
		if( chkbone ){
			if( chkbone->m_boneno == boneno ){
				clusterno = clno;
				break;
			}
		}
	}

	if( clusterno < 0 ){
		return 0;
	}

	*dstclusterno = clusterno;

	int vno;
	for( vno = 0; vno < pm4->m_orgpointnum; vno++ ){
		CInfBone* curib = pm4->m_infbone + vno;
		int ieno = curib->ExistBone( srcobj, clusterno );
		if( ieno >= 0 ){
			dirtyflag = 1;
			break;
		}
	}
	return dirtyflag;
}


int AnimateMorph(FbxScene* pScene, CModel* pmodel)
{
    FbxTime lTime;
    int lKeyIndex = 0;

	if( s_blsinfo.empty() ){
		return 0;
	}

	if( s_ainum <= 0 ){
		return 0;
	}

	double timescale = 30.0;
//	double timescale = 300.0;

	int aino;
	for( aino = 0; aino < s_ainum; aino++ ){

		ANIMINFO* curai = s_ai + aino;
		int curmotid = curai->motid;
		int maxframe = curai->maxframe;

		FbxAnimStack * lCurrentAnimationStack = pmodel->m_pscene->FindMember<FbxAnimStack>(pmodel->mAnimStackNameArray[curai->orgindex]->Buffer());
		if (lCurrentAnimationStack == NULL){
			_ASSERT( 0 );
			return 1;
		}
		pmodel->m_pscene->GetEvaluator()->SetContext(lCurrentAnimationStack);


		FbxAnimLayer* ldstAnimLayer = curai->animlayer;
		map<int, BLSINFO>::iterator itrblsinfo;
		for( itrblsinfo = s_blsinfo.begin(); itrblsinfo != s_blsinfo.end(); itrblsinfo++ ){
			BLSINFO curinfo = itrblsinfo->second;
			FbxGeometry* lAttribute = (FbxGeometry*)(curinfo.basenode)->GetNodeAttribute();
			FbxAnimCurve* lCurve = lAttribute->GetShapeChannel(0, curinfo.blsindex.channelno, ldstAnimLayer, true);
			if (lCurve)
			{
				lCurve->KeyModifyBegin();

				int frameno;
				for( frameno = 0; frameno <= maxframe; frameno++ ){
					double dframe = (double)frameno;
					float curweight = pmodel->GetTargetWeight( curmotid, dframe, timescale, curinfo.base, curinfo.targetname );

					lTime.SetSecondDouble( dframe / timescale);
					lKeyIndex = lCurve->KeyAdd(lTime);
					lCurve->KeySetValue(lKeyIndex, curweight);
					lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);

//if( aino == 0 ){
//	WCHAR wtargetname[256]={0L};
//	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (char*)curinfo.targetname.c_str(), 256, wtargetname, 256 );
//	DbgOut( L"weight check !!! : target %s, frame %f, weight %f\r\n",
//		wtargetname, dframe, curweight );
//}
				}
				lCurve->KeyModifyEnd();
			}
		}
	}
	return 0;
}



CFBXBone* CreateFBXBoneOfBVH( FbxScene* pScene )
{
	if( !s_behead ){
		_ASSERT( 0 );
		return 0;
	}

	FbxNode* lSkeletonNode;
	FbxString lNodeName( s_behead->name );
	FbxSkeleton* lSkeletonNodeAttribute = FbxSkeleton::Create(pScene, lNodeName);
	lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eRoot);
	lSkeletonNode = FbxNode::Create(pScene,lNodeName.Buffer());
	lSkeletonNode->SetNodeAttribute(lSkeletonNodeAttribute);
	lSkeletonNode->LclTranslation.Set(FbxVector4(s_behead->position.x, s_behead->position.y, s_behead->position.z));

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return 0;
	}
	fbxbone->type = FB_ROOT;
	fbxbone->bone = 0;
	fbxbone->pbe = s_behead;
	fbxbone->skelnode = lSkeletonNode;

	s_fbxbone = fbxbone;//!!!!!!!!!!!!!!!!!!!!!!!!!

	if( s_behead->child ){
		CreateFBXBoneOfBVHReq( pScene, s_behead->child, fbxbone );
	}

	return fbxbone;

}
void CreateFBXBoneOfBVHReq( FbxScene* pScene, CBVHElem* pbe, CFBXBone* parfbxbone )
{
	char* bunkiptr = 0;
	D3DXVECTOR3 curpos, parpos, gparpos;
	curpos = pbe->position;
	CBVHElem* parbe = pbe->parent;
	if( parbe ){
		parpos = parbe->position;
		CBVHElem* gparbe = parbe->parent;
		if( gparbe ){
			gparpos = gparbe->position;
		}else{
			gparpos = parpos;
		}
	}else{
		parpos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		gparpos = parpos;
	}

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return;
	}

	char newname[256] = {0};
	sprintf_s( newname, 256, "%s", pbe->name );

	FbxString lLimbNodeName1( newname );
	FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene,lLimbNodeName1);
	lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
	lSkeletonLimbNodeAttribute1->Size.Set(1.0);
	FbxNode* lSkeletonLimbNode1 = FbxNode::Create(pScene,lLimbNodeName1.Buffer());
	lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
	if( pbe->child ){
		lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x - parpos.x, curpos.y - parpos.y, curpos.z - parpos.z));
	}else{
		lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(parpos.x - gparpos.x, parpos.y - gparpos.y, parpos.z - gparpos.z));
	}
	parfbxbone->skelnode->AddChild(lSkeletonLimbNode1);

	fbxbone->pbe = pbe;
	fbxbone->skelnode = lSkeletonLimbNode1;
	parfbxbone->AddChild( fbxbone );

	if( pbe->child ){
		//bunki ˆ—íœ
		fbxbone->type = FB_NORMAL;
	}else{
		//endjointo—Í
		fbxbone->type = FB_ENDJOINT;
	}

	if( pbe->child ){
		CreateFBXBoneOfBVHReq( pScene, pbe->child, fbxbone );
	}
	if( pbe->brother ){
		CreateFBXBoneOfBVHReq( pScene, pbe->brother, parfbxbone );
	}

}


CFBXBone* CreateFBXBone( FbxScene* pScene, CModel* pmodel )
{
	CBone* topj = pmodel->m_topbone;
	if( !topj ){
		_ASSERT( 0 );
		return 0;
	}

	FbxNode* lSkeletonNode;
	FbxString lNodeName( topj->m_engbonename );
	FbxSkeleton* lSkeletonNodeAttribute = FbxSkeleton::Create(pScene, lNodeName);
	lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eRoot);
	lSkeletonNode = FbxNode::Create(pScene,lNodeName.Buffer());
	lSkeletonNode->SetNodeAttribute(lSkeletonNodeAttribute);
	lSkeletonNode->LclTranslation.Set(FbxVector4(topj->m_jointfpos.x, topj->m_jointfpos.y, topj->m_jointfpos.z));

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return 0;
	}
	fbxbone->type = FB_ROOT;
	fbxbone->bone = topj;
	fbxbone->skelnode = lSkeletonNode;

	s_fbxbone = fbxbone;//!!!!!!!!!!!!!!!!!!!!!!!!!

	if( topj->m_child ){
		CreateFBXBoneReq( pScene, topj->m_child, fbxbone );
	}

	return fbxbone;

}

void CreateFBXBoneReq( FbxScene* pScene, CBone* pbone, CFBXBone* parfbxbone )
{
	char* bunkiptr = 0;
	D3DXVECTOR3 curpos, parpos, gparpos;
	curpos = pbone->m_jointfpos;
	CBone* parbone = pbone->m_parent;
	if( parbone ){
		parpos = parbone->m_jointfpos;
		CBone* gparbone = parbone->m_parent;
		if( gparbone ){
			gparpos = gparbone->m_jointfpos;
		}else{
			gparpos = parpos;
		}
	}else{
		parpos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		gparpos = parpos;
	}

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return;
	}

	bunkiptr = strstr( pbone->m_bonename, "_bunki" );

	char newname[256] = {0};
	sprintf_s( newname, 256, "%s", pbone->m_engbonename );

	FbxString lLimbNodeName1( newname );
	FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene,lLimbNodeName1);
	lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
	lSkeletonLimbNodeAttribute1->Size.Set(1.0);
	FbxNode* lSkeletonLimbNode1 = FbxNode::Create(pScene,lLimbNodeName1.Buffer());
	lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
	if( pbone->m_child ){
		lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x - parpos.x, curpos.y - parpos.y, curpos.z - parpos.z));
	}else{
		lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(parpos.x - gparpos.x, parpos.y - gparpos.y, parpos.z - gparpos.z));
	}
	parfbxbone->skelnode->AddChild(lSkeletonLimbNode1);

	fbxbone->bone = pbone;
	fbxbone->skelnode = lSkeletonLimbNode1;
	parfbxbone->AddChild( fbxbone );

	if( pbone->m_child ){
		if( !bunkiptr ){
			fbxbone->type = FB_NORMAL;
		}else{
			//fbxbone->type = FB_BUNKI;
		}
	}else{
		//endjointo—Í
		fbxbone->type = FB_ENDJOINT;
	}

	if( pbone->m_child ){
		CreateFBXBoneReq( pScene, pbone->m_child, fbxbone );
	}
	if( pbone->m_brother ){
		CreateFBXBoneReq( pScene, pbone->m_brother, parfbxbone );
	}
}
int DestroyFBXBoneReq( CFBXBone* fbxbone )
{
	CFBXBone* chilbone = fbxbone->m_child;
	CFBXBone* brobone = fbxbone->m_brother;

	delete fbxbone;

	if( chilbone ){
		DestroyFBXBoneReq( chilbone );
	}
	if( brobone ){
		DestroyFBXBoneReq( brobone );
	}

	return 0;
}

