#include "stdafx.h"
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
#include <MQOFace.h>
#include <InfBone.h>
#include <MotionPoint.h>
//#include <MorphKey.h>

#include <bvhelem.h>
#include <bvhfile.h>

#include <GlobalVar.h>

#define KARCH_ENV_WIN


static FbxNode::EPivotSet s_convPivot;

static float FlClamp( float srcval, float minval, float maxval );

static int s_doublevertices = 0;
static CBVHElem* s_behead = 0;
static int s_invindex[3] = {0, 2, 1};
static int s_firstanimout = 0;
static int s_zeroframemotid = 0;


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


static map<CBone*, map<int, int>> s_linkdirty;
static CModel* s_model = 0;

static ANIMINFO* s_ai = 0;
static int s_ainum = 0;

static CFBXBone* s_fbxbone = 0;
static CFBXBone* s_firsttopbone = 0;
static int s_fbxbonenum = 0;


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
static int s_bvhjointnum = 0;

//static map<CBone*, FbxNode*> s_bone2skel;
static int s_firstoutmot;




static CFBXBone* CreateFBXBone( FbxScene* pScene, CModel* pmodel );
static void CreateFBXBoneReq( FbxScene* pScene, CBone* pbone, CFBXBone* parfbxbone );

static CFBXBone* CreateFBXBoneOfBVH( FbxScene* pScene );
static void CreateFBXBoneOfBVHReq( FbxScene* pScene, CBVHElem* pbe, CFBXBone* parfbxbone );

static int DestroyFBXBoneReq( CFBXBone* fbxbone );



//static void CreateAndFillIOSettings(FbxManager* pSdkManager);
static bool SaveScene(FbxManager* pSdkManager, FbxDocument* pScene, const char* pFilename, int pFileFormat=-1, bool pEmbedMedia=false);


static bool CreateScene(bool limitdegflag, FbxManager* pSdkManager, FbxScene* pScene, CModel* pmodel, char* fbxdate );
static bool CreateBVHScene(FbxManager* pSdkManager, FbxScene* pScene, char* fbxdate );
static FbxNode* CreateFbxMesh( FbxManager* pSdkManager, FbxScene* pScene, CModel* pmodel, CMQOObject* curobj );
static int CreateFbxMaterial(FbxManager* pSdkManager, FbxScene* pScene, FbxNode* lNode, FbxMesh* lMesh, FbxGeometryElementMaterial* lMaterialElement, CModel* pmodel, CMQOObject* curobj);
static int CreateFbxMaterialFromMQOMaterial(FbxManager* pSdkManager, FbxScene* pScene, FbxNode* lNode, FbxGeometryElementMaterial* lMaterialElement, CModel* pmodel, CMQOObject* curobj, CMQOMaterial* mqomat, int curtrinum);

//static FbxNode* CreateSkeleton(FbxScene* pScene, CModel* pmodel);
//static void CreateSkeletonReq( FbxScene* pScene, CBone* pbone, CBone* pparentbone, FbxNode* pparnode );
//static void LinkMeshToSkeletonReq( CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* lMesh, CMQOObject* curobj, CModel* pmodel );
static void LinkMeshToSkeletonReq(CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone);
static BOOL LinkMeshToSkeletonFunc(FbxCluster* lCluster, CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone);

static void LinkToTopBone(FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone);
static BOOL LinkToTopBoneFunc(FbxCluster* lCluster, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone);



static int WriteBindPose(FbxScene* pScene, int bvhflag = 0);
static void WriteBindPoseReq( CFBXBone* fbxbone, FbxPose* lPose );


static void FindHipsFbxBoneReq(CFBXBone* srcfbxbone, CFBXBone** ppfindfbxbone);
static void AnimateSkeleton(bool limitdegflag, FbxScene* pScene, CModel* pmodel);
static void AnimateBoneReq(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int motmax);
static int AnimateMorph(FbxScene* pScene, CModel* pmodel);

static void AnimateSkeletonOfBVH( FbxScene* pScene );
static void AnimateBoneOfBVHReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer );


static FbxTexture*  CreateTexture( FbxManager* pSdkManager, CMQOMaterial* mqomat );
static int ExistBoneInInf( int boneno, CMQOObject* srcobj, int* dstclusterno );

static int MapShapesOnMesh( FbxScene* pScene, FbxNode* pNode, CModel* pmodel, CMQOObject* curobj, BLSINDEX* blsindex );
static int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, ChaVector3* targetv, int targetcnt );

static void CreateDummyInfDataReq(CFBXBone* fbxbone, FbxManager*& pSdkManager, FbxScene*& pScene, FbxNode* lMesh, FbxSkin* lSkin, CBone** ppsetbone, int* bonecnt);
static FbxNode* CreateDummyFbxMesh(FbxManager* pSdkManager, FbxScene* pScene, CBone** ppsetbone);
static void LinkDummyMeshToSkeleton(CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, int* bonecnt);


static int CalcLocalNodeMat(CModel* pmodel, CBone* curbone, ChaMatrix* dstnodemat, ChaMatrix* dstnodeanimmat);//pNode = pmodel->GetBoneNode(curbone)������Ŏg�p
static void CalcBindMatrix(CFBXBone* fbxbone, FbxAMatrix& lMatrix);

static int WriteFBXAnimTra(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind);
static int WriteFBXAnimRot(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind);
static int WriteFBXAnimScale(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind);
static int WriteFBXAnimTraOfBVH(CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int axiskind, int zeroflag);
static int WriteFBXAnimRotOfBVH(CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int axiskind, int zeroflag);


static int SaveCurrentMotionID(CModel* curmodel);//2022/08/18


#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif


extern float g_tmpbvhfilter;



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

		/*
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
		*/
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
        //printf("Error returned: %s\n\n", lExporter->GetLastErrorString());
        return false;
    }


	//FBX File�����������ǂ�����Maya�œǂݍ��܂Ȃ��ƕ�����Ȃ�
	//�����Ă���̂�Maya2013�Ȃ̂�Maya2013�Ŋm���߂�
	//FBX201300�ŏo��
	//Maya2013��FBX plugin 2013.3��API version��201300
	//lExporter->SetFileExportVersion(FbxString("FBX201300"),
	//	FbxSceneRenamer::eNone
	//);

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	printf("FBX version number for this version of the FBX SDK is %d.%d.%d\n\n", lMajor, lMinor, lRevision);


    // Export the scene.
    lStatus = lExporter->Export(pScene); 

    // Destroy the exporter.
    lExporter->Destroy();
    return lStatus;
}

int BVH2FBXFile(FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename, char* fbxdate)
{
	if (!pbvhfile) {
		return 1;
	}

	s_bvhflag = 1;
	s_bvhjointnum = 0;
	s_behead = pbvhfile->GetBVHElem();
	s_bvhjointnum = pbvhfile->GetJointNum();
	s_model = 0;

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
    lResult = CreateBVHScene( s_pSdkManager, lScene, fbxdate );// create, set
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

	if (lScene) {
		lScene->Destroy(true);
		lScene = 0;
	}

	s_bvhflag = 0;

	return 0;
}


int WriteFBXFile(bool limitdegflag, FbxManager* psdk, CModel* pmodel, char* pfilename, char* fbxdate)
{

	s_bvhflag = 0;//�����͏������̈Ӗ��BCreateScene()�ŃZ�b�g�B
	s_pSdkManager = psdk;
	s_model = pmodel;
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
    lResult = CreateScene(limitdegflag, s_pSdkManager, lScene, pmodel, fbxdate);

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

	if (lScene) {
		lScene->Destroy(true);
		lScene = 0;
	}

	return 0;
}

bool CreateBVHScene( FbxManager *pSdkManager, FbxScene* pScene, char* fbxdate )
{
	if (!pSdkManager || !pScene || !fbxdate) {
		return 1;
	}

    // create scene info
    FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager,"SceneInfo");
	if (sceneInfo) {
		sceneInfo->mTitle = "scene made by MameBake3D";
		sceneInfo->mSubject = "skinmesh and animation";
		sceneInfo->mAuthor = "OchakkoLab";
		//sceneInfo->mRevision = "rev. 2.2";
		//sceneInfo->mRevision = "rev. 2.3";//since 2021/05/11 about AM12:00
		//sceneInfo->mRevision = "rev. 2.5";//since 2022/09/05 about PM11:40
		//sceneInfo->mRevision = "rev. 2.6";//since 2022/10/31 about PM09:00
		//sceneInfo->mRevision = "rev. 2.7";//since 2022/11/23 about PM07:00
		sceneInfo->mRevision = "rev. 2.8";//since 2022/12/30 about PM05:00 for version1.1.0.10
		//######################################################################
		//rev�ύX���́@FbxSetDefaultBonePosReq ��oldbvh�����������X�V����K�v�L
		//######################################################################

		sceneInfo->mKeywords = "BVH animation";
		//sceneInfo->mComment = "no particular comments required.";
		sceneInfo->mComment = fbxdate;//!!!!!!!!!!!!!!!//since 2021/05/11 about AM12:00


		// we need to add the sceneInfo before calling AddThumbNailToScene because
		// that function is asking the scene for the sceneInfo.
		pScene->SetSceneInfo(sceneInfo);
	}
	

//    AddThumbnailToScene(pScene);

    FbxNode* lRootNode = pScene->GetRootNode();

	s_fbxbone = CreateFBXBoneOfBVH( pScene );
	lRootNode->AddChild( s_fbxbone->GetSkelNode() );
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

	CBone** ppsetbone = 0;
	//CBone** ppsetbone = (CBone**)malloc(s_model->GetBoneListSize() * sizeof(CBone*));
	//if (!ppsetbone) {
	//	_ASSERT(0);
	//	return 1;
	//}
	//_ASSERT(ppsetbone);
	//ZeroMemory(ppsetbone, s_model->GetBoneListSize() * sizeof(CBone*));


	FbxNode* lMesh = CreateDummyFbxMesh(pSdkManager, pScene, ppsetbone);
	lRootNode->AddChild(lMesh);
	FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
	FbxSkin* lSkin = FbxSkin::Create(pScene, "");
	int sbonecnt = 0;
	CreateDummyInfDataReq(s_fbxbone, pSdkManager, pScene, lMesh, lSkin, ppsetbone, &sbonecnt);
	lMeshAttribute->AddDeformer(lSkin);

	if (ppsetbone) {
		free(ppsetbone);
		ppsetbone = 0;
	}

//    StoreRestPose(pScene, lSkeletonRoot);


    AnimateSkeletonOfBVH( pScene );


	s_firstoutmot = 1;

	//2022/11/23 bvh2fbx�̏ꍇ�ɂ�bindmat���������܂Ȃ� ��������fbx�ǂݍ��ݎ���NodeMat���쐬�����
	//WriteBindPose(pScene, 1);

	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    return true;
}



bool CreateScene(bool limitdegflag, FbxManager *pSdkManager, FbxScene* pScene, CModel* pmodel, char* fbxdate)
{
	if (!pSdkManager || !pScene || !pmodel || !fbxdate) {
		return 1;
	}

	s_model = pmodel;

	//source scene��bvh������ꂽFBX���ǂ����𔻒�
	//FbxDocumentInfo* sceneinfo = pScene->GetSceneInfo();
	//if (sceneinfo) {
	//	FbxString mKeywords = "BVH animation";
	//	if (sceneinfo->mKeywords == mKeywords) {
	//		s_bvhflag = 1;//!!!!!! bvh��FBX�ɕϊ����ĕۑ����A�����ǂݍ���ł���ۑ�����ꍇ
	//	}
	//}
	if (pmodel->GetFromBvhFlag()) {
		s_bvhflag = 1;
	}
	else {
		s_bvhflag = 0;
	}



	// create scene info
	FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager, "SceneInfo");
	if (sceneInfo) {
		sceneInfo->mTitle = "scene made by MameBake3D";
		sceneInfo->mSubject = "skinmesh and animation";
		sceneInfo->mAuthor = "OchakkoLab";
		//sceneInfo->mRevision = "rev. 2.2";
		//sceneInfo->mRevision = "rev. 2.3";//since 2021/05/11 about AM12:00
		//sceneInfo->mRevision = "rev. 2.4";//since 2022/07/05 about PM3:00
		//sceneInfo->mRevision = "rev. 2.5";//since 2022/09/05 about PM11:40
		//sceneInfo->mRevision = "rev. 2.6";//since 2022/10/31 about PM09:00
		//sceneInfo->mRevision = "rev. 2.7";//since 2022/11/23 about PM07:00
		sceneInfo->mRevision = "rev. 2.8";//since 2022/12/30 about PM05:00 for version1.1.0.10
		//######################################################################
		//rev�ύX���́@FbxSetDefaultBonePosReq ��oldbvh�����������X�V����K�v�L
		//######################################################################


		if (pmodel->GetHasBindPose() && (pmodel->GetFromNoBindPoseFlag() == false)) {
			sceneInfo->mKeywords = "skinmesh animation";
		}
		else {
			sceneInfo->mKeywords = "skinmesh animation, start from no bindpose fbx";
		}

		//sceneInfo->mComment = "no particular comments required.";
		sceneInfo->mComment = fbxdate;//!!!!!!!!!!!!!!!//since 2021/05/11 about AM12:00


		// we need to add the sceneInfo before calling AddThumbNailToScene because
		// that function is asking the scene for the sceneInfo.
		pScene->SetSceneInfo(sceneInfo);
	}
	
	//    AddThumbnailToScene(pScene);

	FbxNode* lRootNode = pScene->GetRootNode();

	s_fbxbone = CreateFBXBone(pScene, pmodel);
	lRootNode->AddChild(s_fbxbone->GetSkelNode());
	if (!s_fbxbone) {
		_ASSERT(0);
		return 0;
	}

	
	if (!s_model) {
		_ASSERT(0);
		return 1;//!!!!!!
	}

	SaveCurrentMotionID(s_model);//2022/08/18

	CBone** ppsetbone = (CBone**)malloc(s_model->GetBoneListSize() * sizeof(CBone*));
	if (!ppsetbone) {
		_ASSERT(0);
		return 1;
	}
	_ASSERT(ppsetbone);
	ZeroMemory(ppsetbone, s_model->GetBoneListSize() * sizeof(CBone*));

	BLSINDEX blsindex;
	ZeroMemory(&blsindex, sizeof(BLSINDEX));
	s_blsinfo.clear();
	map<int, CMQOObject*>::iterator itrobj;
	for (itrobj = pmodel->GetMqoObjectBegin(); itrobj != pmodel->GetMqoObjectEnd(); itrobj++) {
		CMQOObject* curobj = itrobj->second;

		const char* dummynameptr = strstr(curobj->GetName(), "_ND_dtri");
		if (dummynameptr) {
			continue;
		}

		FbxNode* lMesh = CreateFbxMesh(pSdkManager, pScene, pmodel, curobj);
		if (!lMesh) {
			continue;//RootNode
		}
		//if( !(curobj->EmptyFindShape()) ){
		//	MapShapesOnMesh( pScene, lMesh, pmodel, curobj, &blsindex );
		//}
		lRootNode->AddChild(lMesh);

		FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
		FbxSkin* lSkin = FbxSkin::Create(pScene, "");


		int* psetflag = (int*)malloc(curobj->GetPm4()->GetOptLeng() * sizeof(int));
		if (!psetflag) {
			_ASSERT(psetflag);
			return 1;
		}
		ZeroMemory(psetflag, curobj->GetPm4()->GetOptLeng() * sizeof(int));

		s_linkdirty.clear();
		LinkMeshToSkeletonReq(s_fbxbone, lSkin, pScene, lMesh, curobj, curobj->GetPm4(), psetflag, ppsetbone);
		LinkToTopBone(lSkin, pScene, lMesh, curobj, curobj->GetPm4(), psetflag, ppsetbone);

		free(psetflag);
		psetflag = 0;

		lMeshAttribute->AddDeformer(lSkin);
	}




	FbxNode* lMesh = CreateDummyFbxMesh(pSdkManager, pScene, ppsetbone);
	if (lMesh) {
		lRootNode->AddChild(lMesh);
		FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
		FbxSkin* lSkin = FbxSkin::Create(pScene, "");
		int sbonecnt = 0;
		CreateDummyInfDataReq(s_fbxbone, pSdkManager, pScene, lMesh, lSkin, ppsetbone, &sbonecnt);
		lMeshAttribute->AddDeformer(lSkin);
	}

	free(ppsetbone);
	ppsetbone = 0;

//    StoreRestPose(pScene, lSkeletonRoot);


    AnimateSkeleton(limitdegflag, pScene, pmodel);
	AnimateMorph(pScene, pmodel);

	//if (pmodel && (pmodel->GetFromNoBindPoseFlag() == false)) {
		WriteBindPose(pScene, s_bvhflag);
	//}
	
	
	if( s_ai ){
		free( s_ai );
		s_ai = 0;
	}
	s_ainum = 0;

    return true;
}




//void MapBoxShape(FbxScene* pScene, FbxBlendShapeChannel* lBlendShapeChannel)
//int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, CMQOObject* curtarget, MATERIALBLOCK* pmb, int mbno )
int MapTargetShape( FbxBlendShapeChannel* lBlendShapeChannel, FbxScene* pScene, CMQOObject* curobj, ChaVector3* targetv, int targetcnt )
{
	char shapename[256]={0};
	char tmpname[256] = { 0 };
	strcpy_s(tmpname, 256, curobj->GetEngName());
	char* underbar = strchr(tmpname, '_');
	if (underbar) {
		*underbar = 0;
		sprintf_s(shapename, 256, "SHAPE_%s_%d", tmpname, targetcnt);
	}
	else {
		sprintf_s(shapename, 256, "SHAPE_%s_%d", curobj->GetEngName(), targetcnt);
	}
    FbxShape* lShape = FbxShape::Create(pScene,shapename);

	CPolyMesh4* basepm4 = curobj->GetPm4();
	_ASSERT( basepm4 );

	int shapevertnum = basepm4->GetFaceNum() * 3;
	lShape->InitControlPoints( shapevertnum );
    FbxVector4* lVector4 = lShape->GetControlPoints();

	int shapevno;
	for( shapevno = 0; shapevno < shapevertnum; shapevno++ ){
		int orgvno = *( basepm4->GetDispIndex() + shapevno );

		ChaVector3 shapev = *( targetv + orgvno );
		lVector4[ shapevno ].Set( shapev.x, shapev.y, shapev.z, 1.0 );
	}
	lBlendShapeChannel->AddTargetShape(lShape);
	return 0;
}


int MapShapesOnMesh( FbxScene* pScene, FbxNode* pNode, CModel* pmodel, CMQOObject* curobj, BLSINDEX* blsindex )
{
	char blsname[256] = {0};
	int mbno = 0;//mqo�ł���

	char tmpname[256] = { 0 };
	strcpy_s(tmpname, 256, curobj->GetEngName());
	char* underbar = strchr(tmpname, '_');
	if (underbar) {
		*underbar = 0;
		sprintf_s(blsname, 256, "BLS_%s_%d", tmpname, mbno);
	}
	else {
		sprintf_s(blsname, 256, "BLS_%s_%d", curobj->GetEngName(), mbno);
	}
	//sprintf_s( blsname, 256, "BLS_%s_%d", curobj->GetEngName(), mbno );
	FbxBlendShape* lBlendShape = FbxBlendShape::Create(pScene, blsname);

	(blsindex->channelno) = 0;

	int targetcnt = 0;
	map<string,ChaVector3*> tmpmap;
	curobj->GetShapeVert2( tmpmap );
	map<string,ChaVector3*>::iterator itrshapev;
	for( itrshapev = tmpmap.begin(); itrshapev != tmpmap.end(); itrshapev++ ){
		string targetname = itrshapev->first;
		ChaVector3* curv = itrshapev->second;
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
	CPolyMesh4* pm4 = curobj->GetPm4();
	if (pm4 == NULL) {
		_ASSERT(0);
		return NULL;
	}

	//CMQOMaterial* mqomat = curobj->GetMaterialBegin()->second;
	//_ASSERT( mqomat );
	if (!pm4->GetOrgPointBuf()){
		return 0;// RootNode
	}

	char meshname[256] = {0};

	sprintf_s( meshname, 256, "%s", curobj->GetEngName() );
	int facenum = pm4->GetFaceNum();
	s_doublevertices = 0;
	FbxMesh* lMesh = FbxMesh::Create(pScene, meshname);
	//lMesh->InitControlPoints(pm4->GetOrgPointNum());
	lMesh->InitControlPoints(pm4->GetOptLeng());//###


	FbxVector4* lcp = lMesh->GetControlPoints();

	FbxGeometryElementNormal* lElementNormal = lMesh->CreateElementNormal();
	//lElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lElementNormal->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	lElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

	FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV( "DiffuseUV");
	_ASSERT( lUVDiffuseElement != NULL);
	//lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eDirect);
	//lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

	int vsetno = 0;
	//vsetno = 0;
	//for (vsetno = 0; vsetno < pm4->GetOptLeng(); vsetno++) {
	//	PM3DISPV* pm3dispv = pm4->GetPm3Disp() + vsetno;
	//	*(lcp + vsetno) = FbxVector4(pm3dispv->pos.x, pm3dispv->pos.y, -pm3dispv->pos.z, 1.0);

	//	//int orgvno = *(pm4->GetOrgIndex() + vsetno);
	//	//ChaVector2 curuv = pm4->GetUVByControlPointNo(orgvno);
	//	
	//	//FbxVector2 fbxuv = FbxVector2(pm3dispv->uv.x, 1.0f - pm3dispv->uv.y);
	//	//lUVDiffuseElement->GetDirectArray().Add(fbxuv);

	//	//FbxVector4 fbxn = FbxVector4(pm3dispv->normal.x, pm3dispv->normal.y, pm3dispv->normal.z, 0.0);
	//	//lElementNormal->GetDirectArray().Add(fbxn);
	//}
	
	int faceno;
	for (faceno = 0; faceno < facenum; faceno++) {
		int srcno[3];
		srcno[0] = faceno * 3;
		srcno[1] = faceno * 3 + 2;
		srcno[2] = faceno * 3 + 1;

		int nno[3];
		nno[0] = faceno * 3;
		nno[1] = faceno * 3 + 1;
		nno[2] = faceno * 3 + 2;

		//int vcnt;
		//for (vcnt = 0; vcnt < 3; vcnt++) {
		//	int vno = *(pm4->GetDispIndex() + srcno[vcnt]);
		//	PM3DISPV* pm3dispv = pm4->GetPm3Disp() + vno;

		//	//0 2 1�̃C���f�b�N�X���ŏ����o���ā@�ǂݍ��ݎ��́@�O�@�P�@�Q�̏��ɒ���
		//	*(lcp + vno) = FbxVector4(pm3dispv->pos.x, pm3dispv->pos.y, pm3dispv->pos.z, 1.0);
		//	FbxVector2 fbxuv = FbxVector2(pm3dispv->uv.x, 1.0f - pm3dispv->uv.y);
		//	lUVDiffuseElement->GetDirectArray().Add(fbxuv);


		//	//0 1 2 �̏��Ԃ̂܂�
		//	int nvno = *(pm4->GetDispIndex() + nno[vcnt]);
		//	PM3DISPV* npm3dispv = pm4->GetPm3Disp() + nvno;
		//	FbxVector4 fbxn = FbxVector4(npm3dispv->normal.x, npm3dispv->normal.y, npm3dispv->normal.z, 0.0);
		//	lElementNormal->GetDirectArray().Add(fbxn);
		//}
		int vcnt;
		for (vcnt = 0; vcnt < 3; vcnt++) {
			int vno = *(pm4->GetDispIndex() + srcno[vcnt]);
			PM3DISPV* pm3dispv = pm4->GetPm3Disp() + vno;
			PM3INF* pm3inf = pm4->GetPm3Inf() + vno;

			////�O�t���[���ҏW���l�����ĕϊ� 2022/08/18
			//ChaMatrix zeroframemat;
			//ZeroMemory(&zeroframemat, sizeof(ChaMatrix));
			//int infno;
			//for (infno = 0; infno < 4; infno++) {
			//	int infindex = pm3inf->boneindex[infno];
			//	float infrate = pm3inf->weight[infno];
			//	if (infindex >= 0) {
			//		CBone* curbone = curobj->GetCluster(infindex);
			//		if (curbone) {
			//			CMotionPoint* curmp = curbone->GetMotionPoint(s_zeroframemotid, 0.0);
			//			if (curmp) {
			//				ChaMatrix curmat = curmp->GetWorldMat();
			//				zeroframemat += curmat * infrate;
			//			}
			//		}
			//	}
			//}
			//ChaVector3 orgpos;
			//ChaVector3 zeroframepos;
			//orgpos = ChaVector3(pm3dispv->pos.x, pm3dispv->pos.y, pm3dispv->pos.z);
			//ChaVector3TransformCoord(&zeroframepos, &orgpos, &zeroframemat);
			//*(lcp + vno) = FbxVector4(zeroframepos.x, zeroframepos.y, zeroframepos.z, 1.0);//2022/08/18

			//0 2 1�̃C���f�b�N�X���ŏ����o���ā@�ǂݍ��ݎ��́@�O�@�P�@�Q�̏��ɒ���
			*(lcp + vno) = FbxVector4(pm3dispv->pos.x, pm3dispv->pos.y, pm3dispv->pos.z, 1.0);


			FbxVector2 fbxuv = FbxVector2(pm3dispv->uv.x, 1.0f - pm3dispv->uv.y);
			lUVDiffuseElement->GetDirectArray().Add(fbxuv);

			FbxVector4 fbxn = FbxVector4(pm3dispv->normal.x, pm3dispv->normal.y, pm3dispv->normal.z, 0.0);
			lElementNormal->GetDirectArray().Add(fbxn);
		}

	}
	lUVDiffuseElement->GetIndexArray().SetCount(facenum * 3);


	vsetno = 0;
	//int faceno;
	for( faceno = 0; faceno < facenum; faceno++ ){

		lMesh->BeginPolygon(-1, -1, -1, false);

		int srcno[3];
		srcno[0] = faceno * 3;
		srcno[1] = faceno * 3 + 2;
		srcno[2] = faceno * 3 + 1;

		int vcnt;
		for( vcnt = 0; vcnt < 3; vcnt++ ){
			int vno = *(pm4->GetDispIndex() + srcno[vcnt]);
			_ASSERT((vno >= 0) && (vno < pm4->GetOptLeng()));
		
			lMesh->AddPolygon(vno);
		}
		lMesh->EndPolygon ();
	}


	// create a FbxNode
//		FbxNode* lNode = FbxNode::Create(pSdkManager,pName);
	FbxNode* lNode = FbxNode::Create( pScene, meshname );
	// set the node attribute
	lNode->SetNodeAttribute(lMesh);
	// set the shading mode to view texture
	//lNode->SetShadingMode(FbxNode::eTextureShading);
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


	int result2 = CreateFbxMaterial(pSdkManager, pScene, lNode, lMesh, lMaterialElement, pmodel, curobj);
	if (result2 != 0) {
		_ASSERT(0);
	}

	// Set the Index to the material
	int materialindex = 0;
	int befmaterialindex = -1;
	//for(int i=0; i<lMesh->GetPolygonCount(); ++i){
	for (int i = 0; i<lMesh->GetPolygonCount(); i++){
		
		materialindex = pm4->GetMaterialNoFromFaceNo(i);

		////for debug
		//if (materialindex != befmaterialindex) {
		//	_ASSERT(0);
		//}

		lMaterialElement->GetIndexArray().SetAt(i, materialindex);

		
		//lMaterialElement->GetIndexArray().SetAt(i,0);



		befmaterialindex = materialindex;
	}

	return lNode;
}

int CreateFbxMaterial(FbxManager* pSdkManager, FbxScene* pScene, FbxNode* lNode, FbxMesh* lMesh, FbxGeometryElementMaterial* lMaterialElement, CModel* pmodel, CMQOObject* curobj)
{
	if (!curobj) {
		_ASSERT(0);
		return 1;
	}

	//CPolyMesh3* pm3 = curobj->GetPm3();
	CPolyMesh4* pm4 = curobj->GetPm4();

	int materialnum;
	//if (pm3) {
	//	materialnum = 1;
	//	CMQOMaterial* curmqomat = curobj->GetMaterialBegin()->second;
	//	if (curmqomat != NULL) {
	//		CreateFbxMaterialFromMQOMaterial(pSdkManager, pScene, lNode, lMaterialElement, pmodel, curobj, curmqomat, lMesh->GetPolygonCount());
	//	}
	//}
	//else 
	if (pm4) {
		materialnum = pm4->GetDispMaterialNum();
		int materialcnt;
		for (materialcnt = 0; materialcnt < materialnum; materialcnt++) {
			CMQOMaterial* curmqomat = 0;
			int curoffset = 0;
			int curtrinum = 0;
			int result1 = pm4->GetDispMaterial(materialcnt, &curmqomat, &curoffset, &curtrinum);
			if ((result1 == 0) && (curmqomat != NULL) && (curtrinum > 0)) {
				CreateFbxMaterialFromMQOMaterial(pSdkManager, pScene, lNode, lMaterialElement, pmodel, curobj, curmqomat, curtrinum);
			}
		}
		lMaterialElement->GetIndexArray().SetCount(lMesh->GetPolygonCount());
	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int	CreateFbxMaterialFromMQOMaterial(FbxManager* pSdkManager, FbxScene* pScene, FbxNode* lNode, FbxGeometryElementMaterial* lMaterialElement, CModel* pmodel, CMQOObject* curobj, CMQOMaterial* mqomat, int curtrinum)
{
	static int s_matcnt = 0;
	s_matcnt++;


	char matname[256];
	char tmpname[256] = { 0 };
	strcpy_s(tmpname, 256, mqomat->GetName());
	char* underbar = strchr(tmpname, '_');
	if (underbar) {
		*underbar = 0;
		sprintf_s(matname, 256, "%s_%d", tmpname, s_matcnt);
	}
	else {
		sprintf_s(matname, 256, "%s_%d", curobj->GetEngName(), s_matcnt);
	}
	//sprintf_s( matname, 256, "%s_%d", mqomat->GetName(), s_matcnt );
	//FbxString lMaterialName = mqomat->name;
	FbxString lMaterialName = matname;
	FbxString lShadingName = "Phong";
	FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

	lMaterial->Diffuse.Set(FbxDouble3(mqomat->GetDif4F().x, mqomat->GetDif4F().y, mqomat->GetDif4F().z));
	lMaterial->Emissive.Set(FbxDouble3(mqomat->GetEmi3F().x, mqomat->GetEmi3F().y, mqomat->GetEmi3F().z));
	lMaterial->Ambient.Set(FbxDouble3(mqomat->GetAmb3F().x, mqomat->GetAmb3F().y, mqomat->GetAmb3F().z));
	//lMaterial->AmbientFactor.Set(1.0);
	//lMaterial->AmbientFactor.Set(0.1);
	lMaterial->AmbientFactor.Set(g_AmbientFactorAtSaving);
	FbxTexture* curtex = CreateTexture(pSdkManager, mqomat);
	if (curtex) {
		lMaterial->Diffuse.ConnectSrcObject(curtex);
		lNode->SetShadingMode(FbxNode::eTextureShading);
	}
	else {
		lNode->SetShadingMode(FbxNode::eHardShading);
	}

	//lMaterial->DiffuseFactor.Set(1.0);
	lMaterial->DiffuseFactor.Set(g_DiffuseFactorAtSaving);
	lMaterial->TransparencyFactor.Set(mqomat->GetDif4F().w);
	lMaterial->ShadingModel.Set(lShadingName);
	//lMaterial->Shininess.Set(0.5);
	lMaterial->Shininess.Set(mqomat->GetPower());
	lMaterial->Specular.Set(FbxDouble3(mqomat->GetSpc3F().x, mqomat->GetSpc3F().y, mqomat->GetSpc3F().z));
	//lMaterial->SpecularFactor.Set(0.3);
	lMaterial->SpecularFactor.Set(g_SpecularFactorAtSaving);
	//lMaterial->EmissiveFactor.Set(0.1);
	lMaterial->EmissiveFactor.Set(g_EmissiveFactorAtSaving);


	lNode->AddMaterial(lMaterial);
	// We are in eByPolygon, so there's only need for index (a plane has 1 polygon).
	
	
	
	//lMaterialElement->GetIndexArray().SetCount(lMesh->GetPolygonCount());



	return 0;
}


FbxTexture*  CreateTexture(FbxManager* pSdkManager, CMQOMaterial* mqomat)
{
	if( !*(mqomat->GetTex()) ){
		return NULL;
	}

    FbxFileTexture* lTexture = FbxFileTexture::Create(pSdkManager,"");
    FbxString lTexPath = mqomat->GetTex();

    // Set texture properties.
    lTexture->SetFileName(lTexPath.Buffer());
    //lTexture->SetName("Diffuse Texture");
	lTexture->SetName(mqomat->GetTex());
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

BOOL LinkToTopBoneFunc(FbxCluster* lCluster, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone)
{
	FbxAMatrix lXMatrix;
	FbxNode* lSkel;

	if (s_firsttopbone) {
		//lSkel = s_fbxbone->skelnode;
		lSkel = s_firsttopbone->GetSkelNode();
		if (!lSkel) {
			_ASSERT(0);
			return FALSE;
		}
		CBone* curbone = s_firsttopbone->GetBone();
		_ASSERT(curbone);
		if (curbone) {
			int foundinf = 0;

			map<CBone*, map<int, int>>::iterator itrlinkdirty;
			map<int, int> mapdirty;
			s_linkdirty[curbone] = mapdirty;
			itrlinkdirty = s_linkdirty.find(curbone);

			//FbxCluster *lCluster = 0;
			int curclusterno = -1;
			int infdirty = ExistBoneInInf(curbone->GetBoneNo(), curobj, &curclusterno);

			if (infdirty) {
				int fno;
				for (fno = 0; fno < pm4->GetFaceNum(); fno++) {
					CMQOFace* curface = pm4->GetTriFace() + fno;
					int orgvno[3];
					orgvno[0] = curface->GetIndex(0);
					orgvno[2] = curface->GetIndex(2);
					orgvno[1] = curface->GetIndex(1);

					_ASSERT((orgvno[0] >= 0) && (orgvno[0] < pm4->GetOrgPointNum()));
					_ASSERT((orgvno[1] >= 0) && (orgvno[1] < pm4->GetOrgPointNum()));
					_ASSERT((orgvno[2] >= 0) && (orgvno[2] < pm4->GetOrgPointNum()));

					int vcnt;
					for (vcnt = 0; vcnt < 3; vcnt++) {
						int dispvno;
						dispvno = fno * 3 + s_invindex[vcnt];

						CInfBone* curib = pm4->GetInfBone() + orgvno[vcnt];
						int ieno = curib->ExistBone(curobj, curclusterno);
						if (ieno >= 0) {
							INFDATA* infd = curib->GetInfData(curobj);
							if (infd) {
								if (foundinf == 0)
								{
									foundinf = 1;
									if (lCluster) {
										lCluster->SetLinkMode(FbxCluster::eTotalOne);

										_ASSERT(curbone->GetBoneNo() >= 0);
										_ASSERT(curbone->GetBoneNo() < s_model->GetBoneListSize());
										if (!(ppsetbone && *(ppsetbone + curbone->GetBoneNo()))) {
											*(ppsetbone + curbone->GetBoneNo()) = curbone;//!!!!!!!!!
										}
									}
									else {
										//lCluster���O�̂Ƃ��ɂ͕Ԃ�l��dirty�������邾��
										return TRUE;
									}
								}
								if (lCluster) {
									lCluster->AddControlPointIndex(dispvno, (double)(infd->m_infelem[ieno].dispinf));
								}
							}
							*(psetflag + dispvno) = 1;
						}
					}
				}
			}
			if (foundinf == 0) {
				return FALSE;
			}

			if (lCluster) {
				FbxScene* lScene = pMesh->GetScene();

				FbxTime fbxtime0;
				fbxtime0.SetSecondDouble(0.0);

				FbxAMatrix MeshMatrix = pMesh->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
				lCluster->SetTransformMatrix(MeshMatrix);
				FbxAMatrix SkelMatrix = lSkel->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
				lCluster->SetTransformLinkMatrix(SkelMatrix);

				lSkin->AddCluster(lCluster);
			}
			//if (lCluster) {
			//	if (foundinf == 0) {
			//		lCluster->SetLinkMode(FbxCluster::eAdditive);
			//	}

			//	ChaVector3 pos = ChaVector3(0.0f, 0.0f, 0.0f);
			//	pos.x = s_firsttopbone->GetBone()->GetJointFPos().x;
			//	pos.y = s_firsttopbone->GetBone()->GetJointFPos().y;
			//	pos.z = s_firsttopbone->GetBone()->GetJointFPos().z;

			//	//KFbxScene* lScene = pMesh->GetScene();
			//	//lXMatrix = pMesh->EvaluateGlobalTransform();
			//	//lCluster->SetTransformMatrix(lXMatrix);
			//	lXMatrix.SetIdentity();
			//	lXMatrix[3][0] = -pos.x;
			//	lXMatrix[3][1] = -pos.y;
			//	lXMatrix[3][2] = pos.z;
			//	lCluster->SetTransformMatrix(lXMatrix);

			//	//lXMatrix = lSkel->EvaluateGlobalTransform();
			//	//lCluster->SetTransformLinkMatrix(lXMatrix);
			//	lXMatrix.SetIdentity();
			//	lCluster->SetTransformLinkMatrix(lXMatrix);

			//	lSkin->AddCluster(lCluster);

			//	return TRUE;
			//}
		}
	}
	return FALSE;

}


void LinkToTopBone(FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone)
{

	FbxAMatrix lXMatrix;
	FbxNode* lSkel;
	FbxCluster *lCluster = 0;

	if (s_firsttopbone){
		//lSkel = s_fbxbone->skelnode;
		lSkel = s_firsttopbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return;
		}
		CBone* curbone = s_firsttopbone->GetBone();
		_ASSERT(curbone);
		if (curbone){
			BOOL isdirty;
			isdirty = LinkToTopBoneFunc(0, lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
			if (isdirty == TRUE) {
				lCluster = FbxCluster::Create(pScene, "");
				lCluster->SetLink(lSkel);

				BOOL isdirty2;
				isdirty2 = LinkToTopBoneFunc(lCluster, lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
			}
		}
	}
}

BOOL LinkMeshToSkeletonFunc(FbxCluster* lCluster, CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone)
{
	_ASSERT(pm4);
	FbxGeometry* lMeshAttribute;
	FbxNode* lSkel;

	lSkel = fbxbone->GetSkelNode();
	if (!lSkel) {
		_ASSERT(0);
		return FALSE;
	}
	lMeshAttribute = (FbxGeometry*)pMesh->GetNodeAttribute();

	if (!fbxbone) {
		_ASSERT(0);
		return FALSE;
	}
	CBone* curbone = fbxbone->GetBone();
	if (curbone) {
		int foundinf = 0;

		//if ((fbxbone != s_fbxbone) && (fbxbone->GetBone() != s_firsttopbone->GetBone()) && ((fbxbone->GetType() == FB_NORMAL) || (fbxbone->GetType() == FB_BUNKI_CHIL))){
		if (fbxbone && (fbxbone != s_fbxbone) && (fbxbone->GetBone() != s_firsttopbone->GetBone())) {
			//CBone* curbone = fbxbone->GetBone();
			//_ASSERT(curbone);
			//if (curbone) {
				map<int, int> mapdirty;
				s_linkdirty[curbone] = mapdirty;
				map<CBone*, map<int, int>>::iterator itrlinkdirty;
				itrlinkdirty = s_linkdirty.find(curbone);

				int curclusterno = -1;
				int infdirty = ExistBoneInInf(curbone->GetBoneNo(), curobj, &curclusterno);

				if (infdirty) {
					int fno;
					for (fno = 0; fno < pm4->GetFaceNum(); fno++) {
						CMQOFace* curface = pm4->GetTriFace() + fno;
						int orgvno[3];
						orgvno[0] = curface->GetIndex(0);
						orgvno[2] = curface->GetIndex(2);
						orgvno[1] = curface->GetIndex(1);

						_ASSERT((orgvno[0] >= 0) && (orgvno[0] < pm4->GetOrgPointNum()));
						_ASSERT((orgvno[1] >= 0) && (orgvno[1] < pm4->GetOrgPointNum()));
						_ASSERT((orgvno[2] >= 0) && (orgvno[2] < pm4->GetOrgPointNum()));

						int vcnt;
						for (vcnt = 0; vcnt < 3; vcnt++) {
							int dispvno;
							dispvno = fno * 3 + s_invindex[vcnt];

							CInfBone* curib = pm4->GetInfBone() + orgvno[vcnt];
							int ieno = curib->ExistBone(curobj, curclusterno);
							if (ieno >= 0) {
								INFDATA* infd = curib->GetInfData(curobj);
								if (infd) {
									if (foundinf == 0) {
										foundinf = 1;
										if (lCluster) {
											lCluster->SetLinkMode(FbxCluster::eTotalOne);

											_ASSERT(curbone->GetBoneNo() >= 0);
											_ASSERT(curbone->GetBoneNo() < s_model->GetBoneListSize());
											if (!(ppsetbone && *(ppsetbone + curbone->GetBoneNo()))) {
												*(ppsetbone + curbone->GetBoneNo()) = curbone;//!!!!!!!!!
											}
										}
										else {
											//lCluster���O�̂Ƃ��ɂ͕Ԃ�l��dirty�������邾��
											return TRUE;
										}
									}
									if (lCluster) {
										lCluster->AddControlPointIndex(dispvno, (double)(infd->m_infelem[ieno].dispinf));
									}
								}
								*(psetflag + dispvno) = 1;
							}
						}
					}
				}
			//}
		}
		if (foundinf == 0) {
			//lCluster->SetLinkMode(FbxCluster::eAdditive);
			return FALSE;
		}

		if (lCluster) {
			FbxScene* lScene = pMesh->GetScene();

			FbxTime fbxtime0;
			fbxtime0.SetSecondDouble(0.0);

			FbxAMatrix MeshMatrix = pMesh->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
			lCluster->SetTransformMatrix(MeshMatrix);
			FbxAMatrix SkelMatrix = lSkel->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
			lCluster->SetTransformLinkMatrix(SkelMatrix);

			lSkin->AddCluster(lCluster);
		}

		//if (lCluster) {
		//	if (fbxbone && fbxbone->GetBone()) {
		//		CBone* curbone = fbxbone->GetBone();
		//		if (curbone) {
		//			ChaVector3 pos;
		//			pos = curbone->GetJointFPos();

		//			FbxAMatrix lXMatrix;
		//			lXMatrix.SetIdentity();
		//			lXMatrix[3][0] = -pos.x;
		//			lXMatrix[3][1] = -pos.y;
		//			lXMatrix[3][2] = -pos.z;
		//			lCluster->SetTransformMatrix(lXMatrix);

		//			lXMatrix.SetIdentity();
		//			lCluster->SetTransformLinkMatrix(lXMatrix);
		//		}
		//		else {
		//			FbxAMatrix lXMatrix;
		//			lXMatrix.SetIdentity();
		//			lCluster->SetTransformMatrix(lXMatrix);
		//			lCluster->SetTransformLinkMatrix(lXMatrix);
		//		}
		//		lSkin->AddCluster(lCluster);
		//	}
		//	else if (fbxbone && fbxbone->GetBvhElem()) {
		//		CBVHElem* curbone = fbxbone->GetBvhElem();
		//		if (curbone) {
		//			ChaVector3 pos;
		//			pos = curbone->GetPosition();

		//			FbxAMatrix lXMatrix;
		//			lXMatrix.SetIdentity();
		//			lXMatrix[3][0] = -pos.x;
		//			lXMatrix[3][1] = -pos.y;
		//			lXMatrix[3][2] = -pos.z;
		//			lCluster->SetTransformMatrix(lXMatrix);

		//			lXMatrix.SetIdentity();
		//			lCluster->SetTransformLinkMatrix(lXMatrix);
		//		}
		//		else {
		//			FbxAMatrix lXMatrix;
		//			lXMatrix.SetIdentity();
		//			lCluster->SetTransformMatrix(lXMatrix);
		//			lCluster->SetTransformLinkMatrix(lXMatrix);
		//		}
		//		lSkin->AddCluster(lCluster);
		//	}
		//}

		return TRUE;

	}
	else {
		return FALSE;
	}

}


void LinkMeshToSkeletonReq(CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, CMQOObject* curobj, CPolyMesh4* pm4, int* psetflag, CBone** ppsetbone)
{
	_ASSERT( pm4 );
	FbxGeometry* lMeshAttribute;
    FbxNode* lSkel;

	lSkel = fbxbone->GetSkelNode();
	if (!lSkel) {
		_ASSERT(0);
		return;
	}
	lMeshAttribute = (FbxGeometry*)pMesh->GetNodeAttribute();

	if (!fbxbone) {
		_ASSERT(0);
		return;
	}
	CBone* curbone = fbxbone->GetBone();
	if (curbone) {
		BOOL isdirty;
		isdirty = LinkMeshToSkeletonFunc(0, fbxbone, lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
		if (isdirty == TRUE) {

			FbxCluster *lCluster = FbxCluster::Create(pScene, "");
			lCluster->SetLink(lSkel);

			BOOL isdirty2;
			isdirty2 = LinkMeshToSkeletonFunc(lCluster, fbxbone, lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
			_ASSERT(isdirty2 == TRUE);
		}
	}

	if( fbxbone->GetChild() ){
		LinkMeshToSkeletonReq(fbxbone->GetChild(), lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
	}
	if( fbxbone->GetBrother() ){
		LinkMeshToSkeletonReq(fbxbone->GetBrother(), lSkin, pScene, pMesh, curobj, pm4, psetflag, ppsetbone);
	}

}

/*
void SkinToSkeletonReq(CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene)
{
	//CPolyMesh4* pm4 = curobj->GetPm4();
	//_ASSERT(pm4);
	FbxGeometry* lMeshAttribute = 0;
	FbxAMatrix lXMatrix;
	FbxNode* lSkel;

	//lMeshAttribute = (FbxGeometry*)pMesh->GetNodeAttribute();

	if (fbxbone->GetType() == FB_NORMAL){
		CBone* curbone = fbxbone->GetBone();

		if (curbone){
			lSkel = fbxbone->GetSkelNode();
			if (!lSkel){
				_ASSERT(0);
				return;
			}

			int curclusterno = -1;

			FbxCluster *lCluster = FbxCluster::Create(pScene, "");
			lCluster->SetLink(lSkel);
			lCluster->SetLinkMode(FbxCluster::eTotalOne);

			lCluster->AddControlPointIndex(0, 1.0);
			
			lXMatrix = lSkel->EvaluateGlobalTransform();//!!!!!!!!!!!!
			lCluster->SetTransformMatrix(lXMatrix);


			lSkel    ->AddCluster(lCluster);
		}
	}

	if (fbxbone->GetChild()){
		LinkMeshToSkeletonReq(fbxbone->GetChild(), lSkin, pScene, pMesh, curobj, pmodel);
	}
	if (fbxbone->GetBrother()){
		LinkMeshToSkeletonReq(fbxbone->GetBrother(), lSkin, pScene, pMesh, curobj, pmodel);
	}

}
*/

void AnimateSkeletonOfBVH( FbxScene* pScene )
{
    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;

	
	lAnimStackName = "bvh_animation_nyan";
	FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);
	FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, "Base Layer");
    lAnimStack->AddMember(lAnimLayer);

	s_firstanimout = 1;
	AnimateBoneOfBVHReq( s_fbxbone, lAnimLayer );

	////pScene->GetRootNode()->ConvertPivotAnimationRecursive(lAnimStackName, s_convPivot, 30.0, true);
	pScene->GetRootNode()->ConvertPivotAnimationRecursive(lAnimStack, s_convPivot, 30.0, true);

}
void AnimateBoneOfBVHReq( CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer )
{
    FbxNode* lSkel = 0;

	if (fbxbone){
		CBVHElem* curbe = fbxbone->GetBvhElem();
		//_ASSERT(curbe);
		if (curbe){

			lSkel = fbxbone->GetSkelNode();
			if (!lSkel){
				_ASSERT(0);
				return;
			}

			//EFbxRotationOrder lRotationOrderSrc = eEulerZXY;
			//EFbxRotationOrder lRotationOrderDst = eEulerXYZ;
			//lSkel->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrderSrc);
			//lSkel->GetRotationOrder(FbxNode::eDestinationPivot, lRotationOrderDst);
			
			
			////lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerZXY);
			////lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ);
			//lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ);
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY);

			lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ);
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ);
			//s_convPivot = FbxNode::eSourcePivot;
			s_convPivot = FbxNode::eDestinationPivot;


			int zeroflag;

			
			if ((fbxbone->GetType() == FB_BUNKI_CHIL) || (fbxbone->GetType() == FB_ROOT)){
				zeroflag = 1;
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_X, zeroflag);
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_Y, zeroflag);
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_Z, zeroflag);

				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_X, zeroflag);
				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_Y, zeroflag);
				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_Z, zeroflag);
			}
			else{
				zeroflag = 0;
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_X, zeroflag);
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_Y, zeroflag);
				WriteFBXAnimTraOfBVH(fbxbone, lAnimLayer, AXIS_Z, zeroflag);

				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_X, zeroflag);
				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_Y, zeroflag);
				WriteFBXAnimRotOfBVH(fbxbone, lAnimLayer, AXIS_Z, zeroflag);
			}
			
		}

		if (fbxbone->GetChild()){
			AnimateBoneOfBVHReq(fbxbone->GetChild(), lAnimLayer);
		}
		if (fbxbone->GetBrother()){
			AnimateBoneOfBVHReq(fbxbone->GetBrother(), lAnimLayer);
		}
	}
}


void FindHipsFbxBoneReq(CFBXBone* srcfbxbone, CFBXBone** ppfindfbxbone)
{
	if (srcfbxbone && ppfindfbxbone && !(*ppfindfbxbone)) {
		CBone* curbone = srcfbxbone->GetBone();
		if (curbone && curbone->IsHipsBone()) {
			*ppfindfbxbone = srcfbxbone;
			return;
		}

		if (srcfbxbone->GetChild() && !(*ppfindfbxbone)) {
			FindHipsFbxBoneReq(srcfbxbone->GetChild(), ppfindfbxbone);
		}
		if (srcfbxbone->GetBrother() && !(*ppfindfbxbone)) {
			FindHipsFbxBoneReq(srcfbxbone->GetBrother(), ppfindfbxbone);
		}
	}
}


void AnimateSkeleton(bool limitdegflag, FbxScene* pScene, CModel* pmodel)
{
	static int s_dbgcnt = 0;

    FbxString lAnimStackName;
    FbxTime lTime;
    int lKeyIndex = 0;

	int motionnum = pmodel->GetMotInfoSize();
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
	for( itrmi = pmodel->GetMotInfoBegin(); itrmi != pmodel->GetMotInfoEnd(); itrmi++ ){
		MOTINFO* curmi = itrmi->second;
		if (curmi) {
			(s_ai + aino)->motid = curmi->motid;
			(s_ai + aino)->orgindex = aino;
			(s_ai + aino)->maxframe = (int)(curmi->frameleng - 1.0);
			(s_ai + aino)->engmotname = curmi->engmotname;
			aino++;
		}
	}

	if (motionnum != aino) {
		//NULL��MOTPARAM*���������ꍇ�A���������킹��
		_ASSERT(0);
		motionnum = aino;
	}


	qsort_s( s_ai, motionnum, sizeof( ANIMINFO ), sortfunc_leng, NULL );//���[�V���������Z�����ɏo�͂��Ȃ��Ɛ������ǂݍ��߂Ȃ��BFBX�̎d�l�H

	s_firstoutmot = s_ai->motid;

	for( aino = 0; aino < motionnum; aino++ ){
		//char takename[256];
		//sprintf_s(takename, 256, "take%d", aino);
		//pScene->SetCurrentTake

		ANIMINFO* curai = s_ai + aino;
		int curmotid = curai->motid;
		int maxframe = curai->maxframe;


		lAnimStackName = curai->engmotname;
		char layername[256];
		sprintf_s(layername, 256, "%sLayer%d", lAnimStackName.Buffer(), aino);//2021/08/26 GetBuffer()
		
		FbxAnimStack* lAnimStack = FbxAnimStack::Create(pScene, lAnimStackName);
		FbxAnimLayer* lAnimLayer = FbxAnimLayer::Create(pScene, layername);
		//FbxTakeInfo takeinfo;
	

		curai->animlayer = lAnimLayer;
        //lAnimStack->AddMember(lAnimLayer);
		lAnimStack->AddMember(lAnimLayer);


		pmodel->SetCurrentMotion( curmotid );

		s_firstanimout = 1;
		//AnimateBoneReq( pmodel->GetFromNoBindPoseFlag(), s_fbxbone, lAnimLayer, curmotid, maxframe );

		CFBXBone* hipsfbxbone = 0;
		FindHipsFbxBoneReq(s_fbxbone, &hipsfbxbone);
		if (hipsfbxbone) {
			AnimateBoneReq(limitdegflag, hipsfbxbone, lAnimLayer, curmotid, maxframe);
		}
		else {
			AnimateBoneReq(limitdegflag, s_fbxbone, lAnimLayer, curmotid, maxframe);
		}

		//AnimateBoneReq(limitdegflag, s_fbxbone, lAnimLayer, curmotid, maxframe);//<--- transform node�̕��@�ۑ��̓x�ɉ��


		pScene->AddMember(lAnimStack);//!!!!!!!!

		////pScene->GetRootNode()->ConvertPivotAnimationRecursive( lAnimStackName, s_convPivot, 30.0, true );
		//pScene->GetRootNode()->ConvertPivotAnimationRecursive(lAnimStack, s_convPivot, 30.0, true);//2022/07/28�R�����g�A�E�g

	}


}

void AnimateBoneReq(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe)
{

	static int s_dbgcnt = 0;

    int lKeyIndex = 0;
    FbxNode* lSkel = 0;

	if (fbxbone) {
		CBone* curbone = fbxbone->GetBone();
		//if( curbone && (curbone->GetType() != FBXBONE_NULL)){
		if (curbone) {
			lSkel = fbxbone->GetSkelNode();
			if (!lSkel){
				_ASSERT( 0 );
				return;
			}
			
			//EFbxRotationOrder lRotationOrderSrc = eEulerZXY;
			//EFbxRotationOrder lRotationOrderSrc = eEulerXYZ;
			//EFbxRotationOrder lRotationOrderSrc = eEulerYXZ;
			//EFbxRotationOrder lRotationOrderDst = eEulerXYZ;
			//EFbxRotationOrder lRotationOrderDst = eEulerXYZ;

			//lSkel->GetRotationOrder( FbxNode::eSourcePivot, lRotationOrderSrc );
			//lSkel->GetRotationOrder( FbxNode::eDestinationPivot, lRotationOrderDst );

			
			lSkel->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ );
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerYXZ );
			lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ);
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerZXY);
			//lSkel->SetRotationOrder(FbxNode::eDestinationPivot, eEulerXYZ);
			//s_convPivot =  FbxNode::eSourcePivot;
			s_convPivot = FbxNode::eDestinationPivot;
			s_dbgcnt++;


			WriteFBXAnimTra(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_X);
			WriteFBXAnimTra(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Y);
			WriteFBXAnimTra(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Z);

			WriteFBXAnimRot(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_X);
			WriteFBXAnimRot(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Y);
			WriteFBXAnimRot(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Z);

			WriteFBXAnimScale(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_X);
			WriteFBXAnimScale(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Y);
			WriteFBXAnimScale(limitdegflag, fbxbone, lAnimLayer, curmotid, maxframe, AXIS_Z);


		}
	}

	if( fbxbone->GetChild() ){
		AnimateBoneReq(limitdegflag, fbxbone->GetChild(), lAnimLayer, curmotid, maxframe );
	}
	if( fbxbone->GetBrother() ){
		AnimateBoneReq(limitdegflag, fbxbone->GetBrother(), lAnimLayer, curmotid, maxframe );
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



int WriteBindPose(FbxScene* pScene, int bvhflag)
{
	/*
	FbxObject* FindMember	(	const FbxCriteria & 	pCriteria,
	const char * 	pName
	)		const
	*/

	static int s_bvhanimcnt = 0;

	FbxPose* lPose = FbxPose::Create(pScene,"BindPose1");
	lPose->SetIsBindPose(true);


	if( s_firstoutmot >= 0 ){
		FbxAnimStack * lCurrentAnimationStack;
		//lCurrentAnimationStack = pScene->GetMember(FBX_TYPE(FbxAnimStack), s_ai->motid);

		
		
		if (bvhflag == 0) {
			lCurrentAnimationStack = pScene->GetSrcObject<FbxAnimStack>(s_ai->motid);
		}
		else {
			lCurrentAnimationStack = pScene->GetSrcObject<FbxAnimStack>(0);
		}


		//if (bvhflag == 0){
		//	lCurrentAnimationStack = pScene->FindMember(FBX_TYPE(FbxAnimStack), s_ai->engmotname);
		//}
		//else{
		//	lCurrentAnimationStack = pScene->FindMember(FBX_TYPE(FbxAnimStack), "bvh_animation_nyan");
		//}
		if (lCurrentAnimationStack != NULL)
		{
			//_ASSERT( 0 );
			//return 1;

			//FbxAnimLayer * mCurrentAnimLayer;
			////mCurrentAnimLayer = lCurrentAnimationStack->GetMember(FBX_TYPE(FbxAnimLayer), 0);
			//mCurrentAnimLayer = lCurrentAnimationStack->GetSrcObject<FbxAnimLayer>(0);
			//pScene->GetEvaluator()->SetContext(lCurrentAnimationStack);
			//

			//gSceneContext->SetCurrentAnimStack(s_ai->motid);
			//pScene->SetCurrentAnimStack(s_ai->motid);
			pScene->SetCurrentAnimationStack(lCurrentAnimationStack);
		}
	}else{
		_ASSERT( 0 );
	}


	CFBXBone* hipsfbxbone = 0;
	FindHipsFbxBoneReq(s_fbxbone, &hipsfbxbone);
	if (hipsfbxbone) {
		WriteBindPoseReq(hipsfbxbone, lPose);
	}
	else {
		WriteBindPoseReq(s_fbxbone, lPose);
	}
	//WriteBindPoseReq(s_fbxbone, lPose);

	pScene->AddPose(lPose);

	return 0;
}

void WriteBindPoseReq( CFBXBone* fbxbone, FbxPose* lPose )
{
	FbxTime lTime0;
	lTime0.SetSecondDouble( 0.0 );

	FbxAMatrix lBindMatrix;
	lBindMatrix.SetIdentity();
	ChaMatrix tramat;
	ChaMatrixIdentity(&tramat);
	FbxNode* curskel = fbxbone->GetSkelNode();
	if (fbxbone->GetType() != FB_ROOT){
		if (curskel){
			//FbxAMatrix lBindMatrix;
			CalcBindMatrix(fbxbone, lBindMatrix);
			lPose->Add(curskel, lBindMatrix);
		}
	}


	if( fbxbone->GetChild() ){
		WriteBindPoseReq( fbxbone->GetChild(), lPose );
	}
	if( fbxbone->GetBrother() ){
		WriteBindPoseReq( fbxbone->GetBrother(), lPose );
	}
}


int CalcLocalNodeMat(CModel* pmodel, CBone* curbone, ChaMatrix* dstnodemat, ChaMatrix* dstnodeanimmat)//pNode = pmodel->GetBoneNode(curbone)������Ŏg�p
{
	//parent��SetNodeMat()����Ă��邱�Ƃ��O��
	//Req�ŌĂяo��

	if (!dstnodemat || !dstnodeanimmat || !curbone) {
		_ASSERT(0);
		return 1;
	}


	ChaMatrix retmat;
	retmat.SetIdentity();

	FbxNode* pNode = pmodel->GetBoneNode(curbone);
	if (pNode) {

		curbone->SaveFbxNodePosture(pNode);//2023/02/16

		FbxDouble3 fbxLclPos = curbone->GetFbxLclPos();
		FbxDouble3 fbxLclRot = curbone->GetFbxLclRot();
		FbxDouble3 fbxLclScl = curbone->GetFbxLclScl();

		FbxDouble3 fbxRotOff = curbone->GetFbxRotOff();
		FbxDouble3 fbxRotPiv = curbone->GetFbxRotPiv();
		FbxDouble3 fbxPreRot = curbone->GetFbxPreRot();
		FbxDouble3 fbxPostRot = curbone->GetFbxPostRot();
		FbxDouble3 fbxSclOff = curbone->GetFbxSclOff();
		FbxDouble3 fbxSclPiv = curbone->GetFbxSclPiv();

		bool rotationActive = curbone->GetFbxRotationActive();
		EFbxRotationOrder rotationorder = curbone->GetFbxRotationOrder();

		ChaMatrix fbxT, fbxRoff, fbxRp, fbxRpre, fbxR, fbxRpost, fbxRpinv, fbxSoff, fbxSp, fbxS, fbxSpinv;
		fbxT.SetIdentity();
		fbxRoff.SetIdentity();
		fbxRp.SetIdentity();
		fbxRpre.SetIdentity();
		fbxR.SetIdentity();
		fbxRpost.SetIdentity();
		fbxRpinv.SetIdentity();
		fbxSoff.SetIdentity();
		fbxSp.SetIdentity();
		fbxS.SetIdentity();
		fbxSpinv.SetIdentity();

		fbxT.SetTranslation(ChaVector3((float)fbxLclPos[0], (float)fbxLclPos[1], (float)fbxLclPos[2]));
		fbxRoff.SetTranslation(ChaVector3((float)fbxRotOff[0], (float)fbxRotOff[1], (float)fbxRotOff[2]));
		fbxRp.SetTranslation(ChaVector3((float)fbxRotPiv[0], (float)fbxRotPiv[1], (float)fbxRotPiv[2]));

		////2023/03/27 : rotationorder�Ή�
		//fbxRpre.SetRotation(rotationorder, 0, ChaVector3((float)fbxPreRot[0], (float)fbxPreRot[1], (float)fbxPreRot[2]));
		//fbxR.SetRotation(rotationorder, 0, ChaVector3((float)fbxLclRot[0], (float)fbxLclRot[1], (float)fbxLclRot[2]));
		//fbxRpost.SetRotation(rotationorder, 0, ChaVector3((float)fbxPostRot[0], (float)fbxPostRot[1], (float)fbxPostRot[2]));
		//fbxRpre.SetRotation(rotationorder, 0, ChaVector3((float)fbxPreRot[1], (float)fbxPreRot[2], (float)fbxPreRot[0]));
		//fbxR.SetRotation(rotationorder, 0, ChaVector3((float)fbxLclRot[1], (float)fbxLclRot[2], (float)fbxLclRot[0]));
		//fbxRpost.SetRotation(rotationorder, 0, ChaVector3((float)fbxPostRot[1], (float)fbxPostRot[2], (float)fbxPostRot[0]));


		//#################################################################################################
		//2023/03/28 : ���s����̌��� --> �W���C���g�̏��������Ƃ��Ă̊p�x�́@��Ɂ@XYZ�̉�]�����̂悤��
		//fbx�̓��e�����̂܂�XYZ�����ň����Ηǂ��݂���
		//#################################################################################################
		fbxRpre.SetXYZRotation(0, ChaVector3((float)fbxPreRot[0], (float)fbxPreRot[1], (float)fbxPreRot[2]));
		fbxR.SetXYZRotation(0, ChaVector3((float)fbxLclRot[0], (float)fbxLclRot[1], (float)fbxLclRot[2]));
		fbxRpost.SetXYZRotation(0, ChaVector3((float)fbxPostRot[0], (float)fbxPostRot[1], (float)fbxPostRot[2]));


		fbxRpinv = ChaMatrixInv(fbxRp);
		fbxSoff.SetTranslation(ChaVector3((float)fbxSclOff[0], (float)fbxSclOff[1], (float)fbxSclOff[2]));
		fbxSp.SetTranslation(ChaVector3((float)fbxSclPiv[0], (float)fbxSclPiv[1], (float)fbxSclPiv[2]));
		fbxS.SetScale(ChaVector3((float)fbxLclScl[0], (float)fbxLclScl[1], (float)fbxLclScl[2]));
		fbxSpinv = ChaMatrixInv(fbxSp);

		//#############################################################################
		//Transform = T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
		//#############################################################################

		ChaMatrix localnodemat, localnodeanimmat;
		localnodeanimmat = fbxT * fbxRoff * fbxRp * fbxRpre * fbxR * fbxRpost * fbxRpinv * fbxSoff * fbxSp * fbxS * fbxSpinv;
		
		//0�t���[���A�j������ : fbxR��fbxS����
		localnodemat = fbxT * fbxRoff * fbxRp * fbxRpre * fbxRpost * fbxRpinv * fbxSoff * fbxSp * fbxSpinv;

		*dstnodemat = localnodemat;
		*dstnodeanimmat = localnodeanimmat;
	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

//int CalcLocalNodeMatForMesh(FbxNode* pNode, ChaMatrix* dstmeshmat)
//{
//	if (!pNode || !dstmeshmat) {
//		_ASSERT(0);
//		return 1;
//	}
//
//	ChaMatrix retmat;
//	retmat.SetIdentity();
//
//	if (pNode) {
//		FbxTime fbxtime;
//		fbxtime.SetSecondDouble(0.0);
//		FbxDouble3 fbxLclPos = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot);
//		FbxDouble3 fbxLclRot = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot);
//		FbxDouble3 fbxLclScl = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot);
//		
//		EFbxRotationOrder rotationorder;
//		pNode->GetRotationOrder(FbxNode::eSourcePivot, rotationorder);
//		FbxDouble3 fbxRotOff = pNode->GetRotationOffset(FbxNode::eSourcePivot);
//		FbxDouble3 fbxRotPiv = pNode->GetRotationPivot(FbxNode::eSourcePivot);
//		FbxDouble3 fbxPreRot = pNode->GetPreRotation(FbxNode::eSourcePivot);
//		FbxDouble3 fbxPostRot = pNode->GetPostRotation(FbxNode::eSourcePivot);
//		FbxDouble3 fbxSclOff = pNode->GetScalingOffset(FbxNode::eSourcePivot);
//		FbxDouble3 fbxSclPiv = pNode->GetScalingPivot(FbxNode::eSourcePivot);
//		bool rotationActive = pNode->GetRotationActive();
//
//
//		ChaMatrix fbxT, fbxRoff, fbxRp, fbxRpre, fbxR, fbxRpost, fbxRpinv, fbxSoff, fbxSp, fbxS, fbxSpinv;
//		fbxT.SetIdentity();
//		fbxRoff.SetIdentity();
//		fbxRp.SetIdentity();
//		fbxRpre.SetIdentity();
//		fbxR.SetIdentity();
//		fbxRpost.SetIdentity();
//		fbxRpinv.SetIdentity();
//		fbxSoff.SetIdentity();
//		fbxSp.SetIdentity();
//		fbxS.SetIdentity();
//		fbxSpinv.SetIdentity();
//
//		fbxT.SetTranslation(ChaVector3((float)fbxLclPos[0], (float)fbxLclPos[1], (float)fbxLclPos[2]));
//		fbxRoff.SetTranslation(ChaVector3((float)fbxRotOff[0], (float)fbxRotOff[1], (float)fbxRotOff[2]));
//		fbxRp.SetTranslation(ChaVector3((float)fbxRotPiv[0], (float)fbxRotPiv[1], (float)fbxRotPiv[2]));
//		fbxRpre.SetXYZRotation(0, ChaVector3((float)fbxPreRot[0], (float)fbxPreRot[1], (float)fbxPreRot[2]));
//		fbxR.SetXYZRotation(0, ChaVector3((float)fbxLclRot[0], (float)fbxLclRot[1], (float)fbxLclRot[2]));
//		fbxRpost.SetXYZRotation(0, ChaVector3((float)fbxPostRot[0], (float)fbxPostRot[1], (float)fbxPostRot[2]));
//		fbxRpinv = ChaMatrixInv(fbxRp);
//		fbxSoff.SetTranslation(ChaVector3((float)fbxSclOff[0], (float)fbxSclOff[1], (float)fbxSclOff[2]));
//		fbxSp.SetTranslation(ChaVector3((float)fbxSclPiv[0], (float)fbxSclPiv[1], (float)fbxSclPiv[2]));
//		fbxS.SetScale(ChaVector3((float)fbxLclScl[0], (float)fbxLclScl[1], (float)fbxLclScl[2]));
//		fbxSpinv = ChaMatrixInv(fbxSp);
//
//		//#############################################################################
//		//Transform = T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
//		//#############################################################################
//
//		ChaMatrix localnodemat, localnodeanimmat;
//		localnodeanimmat = fbxT * fbxRoff * fbxRp * fbxRpre * fbxR * fbxRpost * fbxRpinv * fbxSoff * fbxSp * fbxS * fbxSpinv;
//
//		//0�t���[���A�j������ : fbxR��fbxS����
//		localnodemat = fbxT * fbxRoff * fbxRp * fbxRpre * fbxRpost * fbxRpinv * fbxSoff * fbxSp * fbxSpinv;
//	
//		//*dstmeshmat = localnodeanimmat;
//		*dstmeshmat = localnodemat;
//	}
//	else {
//		_ASSERT(0);
//		return 1;
//	}
//
//	return 0;
//
//}


void CalcBindMatrix(CFBXBone* fbxbone, FbxAMatrix& lBindMatrix)
{
	int notexistflag = 0;

	ChaVector3 curpos, parentpos;
	ChaMatrix bvhmat;
	bvhmat.SetIdentity();

	if (s_bvhflag == 1){
		CBVHElem* curbone = fbxbone->GetBvhElem();
		if (curbone){
			bvhmat = *(curbone->GetTransMat());
			curpos = curbone->GetPosition();

			CFBXBone* parfbxbone;
			parfbxbone = fbxbone->GetParent();
			if (parfbxbone){
				CBVHElem* parentbone = parfbxbone->GetBvhElem();
				if (parentbone){
					parentpos = parentbone->GetPosition();
				}
				else{
					parentpos = ChaVector3(0.0f, 0.0f, 0.0f);
				}
			}
			else{
				parentpos = ChaVector3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			notexistflag = 1;
		}
	}
	else{
		CBone* curbone = fbxbone->GetBone();
		if (curbone){
			curpos = curbone->GetJointFPos();

			CFBXBone* parfbxbone;
			parfbxbone = fbxbone->GetParent();
			if (parfbxbone){
				CBone* parentbone = parfbxbone->GetBone();
				if (parentbone){
					parentpos = parentbone->GetJointFPos();
				}
				else{
					parentpos = ChaVector3(0.0f, 0.0f, 0.0f);
				}
			}
			else{
				parentpos = ChaVector3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			notexistflag = 1;
		}
	}

	//CBone calcbone(s_model);
	CBone calcbone = CBone(s_model);

	ChaMatrix tramat;
	ChaMatrixIdentity(&tramat);

	if (notexistflag == 1){
		lBindMatrix.SetIdentity();
		return;//!!!!!!!!!!!!!!! curbone == NULL
	}


	if ((s_bvhflag == 0) && fbxbone->GetBone() && (s_model->GetOldAxisFlagAtLoading() == 0)){

		tramat = fbxbone->GetBone()->GetNodeMat();

	}
	else{
		//bvh��fbx�ɕϊ����Ă���g���Bbvh�̂O�t���[�������̃\�t�g�ŕҏW����\��͍��͖����B
		ChaVector3 diffvec = curpos - parentpos;

		////2022/11/23 rev. 2.7�ȍ~ bvh2fbx���ɂ͂�����ʂ�Ȃ�(bindmat�͏������܂Ȃ�)
		tramat.SetIdentity();
		tramat.data[MATI_41] = curpos.x;
		tramat.data[MATI_42] = curpos.y;
		tramat.data[MATI_43] = curpos.z;
	}

	lBindMatrix[0][0] = tramat.data[MATI_11];
	lBindMatrix[0][1] = tramat.data[MATI_12];
	lBindMatrix[0][2] = tramat.data[MATI_13];
	lBindMatrix[0][3] = tramat.data[MATI_14];
	lBindMatrix[1][0] = tramat.data[MATI_21];
	lBindMatrix[1][1] = tramat.data[MATI_22];
	lBindMatrix[1][2] = tramat.data[MATI_23];
	lBindMatrix[1][3] = tramat.data[MATI_24];
	lBindMatrix[2][0] = tramat.data[MATI_31];
	lBindMatrix[2][1] = tramat.data[MATI_32];
	lBindMatrix[2][2] = tramat.data[MATI_33];
	lBindMatrix[2][3] = tramat.data[MATI_34];
	lBindMatrix[3][0] = tramat.data[MATI_41];
	lBindMatrix[3][1] = tramat.data[MATI_42];
	lBindMatrix[3][2] = tramat.data[MATI_43];
	lBindMatrix[3][3] = tramat.data[MATI_44];

	

	return;
}


/*
FbxAMatrix CalcBindMatrix(CFBXBone* fbxbone)
{
	FbxAMatrix lBindMatrix;
	FbxTime lTime0;
	lTime0.SetSecondDouble(0.0);

	FbxNode* curskel = fbxbone->GetSkelNode();
	if (curskel){
		lBindMatrix = curskel->EvaluateGlobalTransform(lTime0);
	}
	else{
		lBindMatrix.SetIdentity();
	}
	return lBindMatrix;
}


*/

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

	CPolyMesh4* pm4 = srcobj->GetPm4();
	_ASSERT( pm4 );

	int dirtyflag = 0;

	int clusterno = -1;
	CBone* chkbone;
	int clno;
	for( clno = 0; clno < srcobj->GetClusterSize(); clno++ ){
		chkbone = srcobj->GetCluster( clno );
		if( chkbone ){
			if( chkbone->GetBoneNo() == boneno ){
				clusterno = clno;
				break;
			}
		}
	}

	if( clusterno < 0 ){
		return 0;
	}

	*dstclusterno = clusterno;

	int faceno;
	for (faceno = 0; faceno < srcobj->GetPm4()->GetFaceNum(); faceno++){
		CMQOFace* curface = pm4->GetTriFace() + faceno;
		int vno[3];
		vno[0] = curface->GetIndex(0);
		vno[2] = curface->GetIndex(2);
		vno[1] = curface->GetIndex(1);

		_ASSERT((vno[0] >= 0) && (vno[0] < pm4->GetOrgPointNum()));
		_ASSERT((vno[1] >= 0) && (vno[1] < pm4->GetOrgPointNum()));
		_ASSERT((vno[2] >= 0) && (vno[2] < pm4->GetOrgPointNum()));

		int vcnt;
		for (vcnt = 0; vcnt < 3; vcnt++){
			CInfBone* curib = pm4->GetInfBone() + vno[vcnt];
			int ieno = curib->ExistBone(srcobj, clusterno);
			if (ieno >= 0){
				dirtyflag = 1;
				break;
			}
		}
	}
	/*
	int vno;
	for (vno = 0; vno < pm4->GetOrgPointNum(); vno++){
		CInfBone* curib = pm4->GetInfBone() + vno;
		int ieno = curib->ExistBone(srcobj, clusterno);
		if (ieno >= 0){
			dirtyflag = 1;
			break;
		}
	}
	*/
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

		FbxAnimStack * lCurrentAnimationStack = pmodel->GetScene()->FindMember<FbxAnimStack>(pmodel->GetAnimStackName( curai->orgindex )->Buffer());
		if (lCurrentAnimationStack == NULL){
			_ASSERT( 0 );
			return 1;
		}
		//pmodel->GetScene()->GetEvaluator()->SetContext(lCurrentAnimationStack);
		pmodel->GetScene()->SetCurrentAnimationStack(lCurrentAnimationStack);



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
	s_fbxbonenum = 0;
	if( !s_behead ){
		_ASSERT( 0 );
		return 0;
	}


	FbxNode* lSkeletonNode;
	FbxString lNodeName( "RootNode" );
	FbxSkeleton* lSkeletonNodeAttribute = FbxSkeleton::Create(pScene, lNodeName);
	lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eRoot);
	lSkeletonNode = FbxNode::Create(pScene,lNodeName.Buffer());
	lSkeletonNodeAttribute->Size.Set(1.0);
	lSkeletonNode->SetNodeAttribute(lSkeletonNodeAttribute);
	lSkeletonNode->LclTranslation.Set(FbxVector4(0.0, 0.0, 0.0));
	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return 0;
	}
	fbxbone->SetType( FB_ROOT );
	fbxbone->SetBvhElem( 0 );
	fbxbone->SetSkelNode( lSkeletonNode );
	s_fbxbonenum++;

	FbxNode* lSkeletonNode2;
	FbxString lNodeName2(s_behead->GetName());
	FbxSkeleton* lSkeletonNodeAttribute2 = FbxSkeleton::Create(pScene, lNodeName2);
	lSkeletonNodeAttribute2->SetSkeletonType(FbxSkeleton::eLimbNode);
	lSkeletonNode2 = FbxNode::Create(pScene, lNodeName2.Buffer());
	lSkeletonNodeAttribute2->Size.Set(1.0);
	lSkeletonNode2->SetNodeAttribute(lSkeletonNodeAttribute2);
	lSkeletonNode2->LclTranslation.Set(FbxVector4(s_behead->GetPosition().x, s_behead->GetPosition().y, s_behead->GetPosition().z));
	CFBXBone* fbxbone2 = new CFBXBone();
	if (!fbxbone2){
		_ASSERT(0);
		return 0;
	}
	fbxbone2->SetType(FB_NORMAL);
	fbxbone2->SetBvhElem(s_behead);
	fbxbone2->SetSkelNode(lSkeletonNode2);
	fbxbone->GetSkelNode()->AddChild(lSkeletonNode2);
	fbxbone->AddChild(fbxbone2);
	s_fbxbonenum++;


	s_fbxbone = fbxbone;//!!!!!!!!!!!!!!!!!!!!!!!!!

	if( s_behead->GetChild() ){
		CreateFBXBoneOfBVHReq( pScene, s_behead->GetChild(), fbxbone2 );
	}

	return fbxbone;

}
void CreateFBXBoneOfBVHReq( FbxScene* pScene, CBVHElem* pbe, CFBXBone* parfbxbone )
{
	static int s_siteno = 0;


	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return;
	}


	CBVHElem* parpbe;// = pbe->GetParent();
	if (parfbxbone){
		parpbe = parfbxbone->GetBvhElem();
	}
	else{
		parpbe = 0;
	}

	ChaVector3 curpos, parentpos, gparentpos;
	curpos = pbe->GetPosition();
	if (parpbe){
		parentpos = parpbe->GetPosition();
	}
	else{
		parentpos = ChaVector3(0.0f, 0.0f, 0.0f);
	}


	if (parpbe && (parpbe->GetBoneNum() >= 2)){//bvhelem��bonenum�ǉ�
		CFBXBone* fbxbone2 = new CFBXBone();
		if (!fbxbone2){
			_ASSERT(0);
			return;
		}

				
		char newname2[256] = { 0 };
		sprintf_s(newname2, 256, "%s_bunki%d", parpbe->GetName(), pbe->GetBroNo());

		FbxString lLimbNodeName2(newname2);
		FbxSkeleton* lSkeletonLimbNodeAttribute2 = FbxSkeleton::Create(pScene, lLimbNodeName2);
		lSkeletonLimbNodeAttribute2->SetSkeletonType(FbxSkeleton::eLimbNode);
		lSkeletonLimbNodeAttribute2->Size.Set(1.0);
		FbxNode* lSkeletonLimbNode2 = FbxNode::Create(pScene, lLimbNodeName2.Buffer());
		lSkeletonLimbNode2->SetNodeAttribute(lSkeletonLimbNodeAttribute2);
		//lSkeletonLimbNode2->LclTranslation.Set(FbxVector4(curpos2.x - parentpos2.x, curpos2.y - parentpos2.y, curpos2.z - parentpos2.z));
		lSkeletonLimbNode2->LclTranslation.Set(FbxVector4(0.0, 0.0, 0.0));
		parfbxbone->GetSkelNode()->AddChild(lSkeletonLimbNode2);

		fbxbone2->SetBvhElem(parpbe);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		fbxbone2->SetSkelNode(lSkeletonLimbNode2);
		parfbxbone->AddChild(fbxbone2);

		fbxbone2->SetType(FB_BUNKI_CHIL);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (parpbe){
			parfbxbone->SetType(FB_BUNKI_PAR);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		s_fbxbonenum++;

		char newname[256] = { 0 };
		sprintf_s(newname, 256, "%s", pbe->GetName());


		FbxString lLimbNodeName1(newname);
		FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene, lLimbNodeName1);
		lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
		lSkeletonLimbNodeAttribute1->Size.Set(1.0);
		FbxNode* lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
		lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
		//if (pbe->GetChild()){
		if (pbe->GetParent()){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x - parentpos.x, curpos.y - parentpos.y, curpos.z - parentpos.z));
		}
		else{
			//lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(parentpos.x - gparentpos.x, parentpos.y - gparentpos.y, parentpos.z - gparentpos.z));
			lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x, curpos.y, curpos.z));//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		fbxbone2->GetSkelNode()->AddChild(lSkeletonLimbNode1);

		fbxbone->SetBvhElem(pbe);
		fbxbone->SetSkelNode(lSkeletonLimbNode1);
		fbxbone2->AddChild(fbxbone);

		if (pbe->GetChild()){
			fbxbone->SetType(FB_NORMAL);
		}
		else{
			//endjoint
			fbxbone->SetType(FB_ENDJOINT);
		}
		s_fbxbonenum++;

		if (pbe->GetBrother()){
			CreateFBXBoneOfBVHReq(pScene, pbe->GetBrother(), parfbxbone);
		}

		if (pbe->GetChild()){
			CreateFBXBoneOfBVHReq(pScene, pbe->GetChild(), fbxbone);
		}

		/*
		if (pbone->child){
			CreateFBXBoneReq(pScene, pbone->child, fbxbone2);
		}

		CShdElem* validbro = s_lpsh->GetValidBrother(pbone->brother);
		if (validbro){
			CreateFBXBoneReq(pScene, validbro, parfbxbone);
		}
		*/
	}
	else{
		char newname[256] = { 0 };
		sprintf_s(newname, 256, "%s", pbe->GetName());
		int cmp0 = strcmp(newname, "Site");
		int cmp1 = strcmp(newname, "site");
		if ((cmp0 == 0) || (cmp1 == 0)){
			char addstr[256];
			sprintf_s(addstr, 256, "_%d", s_siteno);
			strcat_s(newname, 256, addstr);
			s_siteno++;
		}

		FbxString lLimbNodeName1(newname);
		FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene, lLimbNodeName1);
		lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
		lSkeletonLimbNodeAttribute1->Size.Set(1.0);
		FbxNode* lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
		lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
		//if (pbe->GetChild()){
		if (pbe->GetParent()){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x - parentpos.x, curpos.y - parentpos.y, curpos.z - parentpos.z));
		}
		else{
			//lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(parentpos.x - gparentpos.x, parentpos.y - gparentpos.y, parentpos.z - gparentpos.z));
			lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x, curpos.y, curpos.z));//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			_ASSERT(0);
		}
		parfbxbone->GetSkelNode()->AddChild(lSkeletonLimbNode1);

		fbxbone->SetBvhElem(pbe);
		fbxbone->SetSkelNode(lSkeletonLimbNode1);
		parfbxbone->AddChild(fbxbone);

		if (pbe->GetChild()){
			fbxbone->SetType(FB_NORMAL);
		}
		else{
			//endjoint�o��
			fbxbone->SetType(FB_ENDJOINT);
		}
		s_fbxbonenum++;

		if (pbe->GetBrother()){
			CreateFBXBoneOfBVHReq(pScene, pbe->GetBrother(), parfbxbone);
		}

		if (pbe->GetChild()){
			CreateFBXBoneOfBVHReq(pScene, pbe->GetChild(), fbxbone);
		}
	}
}


CFBXBone* CreateFBXBone(FbxScene* pScene, CModel* pmodel )
{
	s_fbxbonenum = 0;

	CBone* topj = pmodel->GetTopBone();
	//CBone* topj = 0;
	//pmodel->GetHipsBoneReq(pmodel->GetTopBone(), &topj);
	//if (!topj) {
	//	topj = pmodel->GetTopBone();
	//}
	if( !topj ){
		_ASSERT( 0 );
		return 0;
	}

	//_ASSERT(0);

	FbxNode* lSkeletonNode = 0;
	FbxSkeleton* lSkeletonNodeAttribute = 0;
	const FbxNode* nodeonload = pmodel->GetBoneNode(topj);
	FbxString lNodeName(topj->GetEngBoneName());


	CFBXBone* fbxbone2 = 0;//RootNode���܂����������ꍇ�́@�V�KRootNode

	if (strstr(topj->GetBoneName(), "RootNode") == 0) {

		//�ŏ��̃m�[�h��RootNode�ł͂Ȃ��ꍇ

		//�P�ڂɁ@eRoot��V�K
		FbxNode* lSkeletonNode2;
		FbxString lNodeName2("RootNode");
		FbxSkeleton* lSkeletonNodeAttribute2 = FbxSkeleton::Create(pScene, lNodeName2);
		lSkeletonNodeAttribute2->SetSkeletonType(FbxSkeleton::eRoot);
		lSkeletonNodeAttribute2->Size.Set(1.0);
		lSkeletonNode2 = FbxNode::Create(pScene, lNodeName2.Buffer());
		lSkeletonNode2->SetNodeAttribute(lSkeletonNodeAttribute2);
		lSkeletonNode2->LclTranslation.Set(FbxVector4(0.0f, 0.0f, 0.0f));

		fbxbone2 = new CFBXBone();
		if (!fbxbone2) {
			_ASSERT(0);
			return 0;
		}
		fbxbone2->SetType(FB_ROOT);
		fbxbone2->SetBone(0);
		fbxbone2->SetSkelNode(lSkeletonNode2);
		s_fbxbone = fbxbone2;//!!!!!!!!!!!!!!!!!!!!!!!!!


		//�Q�ڂ́@eNull�@�������́@eSkeleton
		//if (topj->GetType() == FBXBONE_NULL) {
		//	FbxNull* lNullAttribute = FbxNull::Create(pScene, lNodeName);
		//	lSkeletonNode = FbxNode::Create(pScene, lNodeName.Buffer());
		//	lSkeletonNode->SetNodeAttribute(lNullAttribute);
		//	if (nodeonload) {
		//		lSkeletonNode->Copy(*nodeonload);
		//	}
		//	else {
		//		topj->RestoreFbxNodePosture(lSkeletonNode);
		//	}
		//}
		//else {
			lSkeletonNodeAttribute = FbxSkeleton::Create(pScene, lNodeName);
			lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eLimbNode);
			lSkeletonNodeAttribute->Size.Set(1.0);
			lSkeletonNode = FbxNode::Create(pScene, lNodeName.Buffer());
			lSkeletonNode->SetNodeAttribute(lSkeletonNodeAttribute);
			if (nodeonload) {
				lSkeletonNode->Copy(*nodeonload);
			}
			//else {
			//	topj->RestoreFbxNodePosture(lSkeletonNode);
			//}
			topj->RestoreFbxNodePosture(lSkeletonNode);

		//}
	}
	else {

		//topj��RootNode�̏ꍇ
		
		//�P�ڂ�eRoot
		lSkeletonNodeAttribute = FbxSkeleton::Create(pScene, lNodeName);
		lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eRoot);
		lSkeletonNodeAttribute->Size.Set(1.0);
		lSkeletonNode = FbxNode::Create(pScene, lNodeName.Buffer());
		lSkeletonNode->SetNodeAttribute(lSkeletonNodeAttribute);
		if (nodeonload) {
			lSkeletonNode->Copy(*nodeonload);
		}
		//else {
		//	topj->RestoreFbxNodePosture(lSkeletonNode);
		//}
		topj->RestoreFbxNodePosture(lSkeletonNode);
	}


	//lSkeletonNodeAttribute->SetSkeletonType(FbxSkeleton::eRoot);

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return 0;
	}
	fbxbone->SetType( FB_NORMAL );
	//fbxbone->SetType(FB_ROOT);
	fbxbone->SetBone( topj );
	fbxbone->SetSkelNode( lSkeletonNode );

	if (fbxbone2) {
		fbxbone2->GetSkelNode()->AddChild(lSkeletonNode);
		fbxbone2->AddChild(fbxbone);
	}

	s_fbxbonenum++;

	s_firsttopbone = fbxbone;//root�̍ŏ��̎q��

	if( topj->GetChild() ){
		CreateFBXBoneReq( pScene, topj->GetChild(), fbxbone );
	}

	//if (topj) {
	//	CreateFBXBoneReq(pScene, topj, fbxbone);
	//}

	//return fbxbone;

	if (fbxbone2) {
		return fbxbone2;//!!!!!!!!!!
	}
	else {
		return fbxbone;
	}
	
}

void CreateFBXBoneReq( FbxScene* pScene, CBone* pbone, CFBXBone* parfbxbone )
{
	ChaVector3 curpos, parentpos, gparentpos;
	curpos = pbone->GetJointFPos();
	CBone* parentbone = pbone->GetParent();
	if( parentbone ){
		parentpos = parentbone->GetJointFPos();
	}else{
		parentpos = ChaVector3( 0.0f, 0.0f, 0.0f );
	}

	CFBXBone* fbxbone = new CFBXBone();
	if( !fbxbone ){
		_ASSERT( 0 );
		return;
	}

	char newname[256] = {0};
	sprintf_s( newname, 256, "%s", pbone->GetEngBoneName() );

	FbxString lLimbNodeName1( newname );
	FbxNode* lSkeletonLimbNode1;
	FbxSkeleton* lSkeletonLimbNodeAttribute1;
	const FbxNode* nodeonload = pbone->GetParModel()->GetBoneNode(pbone);
	//if (pbone->GetType() == FBXBONE_NULL) {
	//	////lSkeletonLimbNodeAttribute1 = 0;
	//	////lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
	//	////lSkeletonLimbNode1->SetNodeAttribute(NULL);
	//	//////lSkeletonNodeAttribute = 0;
	//	//FbxNull* lNullAttribute = FbxNull::Create(pScene, lLimbNodeName1);
	//	//lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
	//	//lSkeletonLimbNode1->SetNodeAttribute(lNullAttribute);

	//	FbxNull* lNullAttribute = FbxNull::Create(pScene, lLimbNodeName1);
	//	lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
	//	lSkeletonLimbNode1->SetNodeAttribute(lNullAttribute);
	//	if (nodeonload) {
	//		lSkeletonLimbNode1->Copy(*nodeonload);
	//	}
	//	else {
	//		pbone->RestoreFbxNodePosture(lSkeletonLimbNode1);
	//	}

	//}
	//else {
		lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(pScene, lLimbNodeName1);
		lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
		lSkeletonLimbNodeAttribute1->Size.Set(1.0);
		lSkeletonLimbNode1 = FbxNode::Create(pScene, lLimbNodeName1.Buffer());
		lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
		if (nodeonload) {
			lSkeletonLimbNode1->Copy(*nodeonload);
		}
		//else {
		//	pbone->RestoreFbxNodePosture(lSkeletonLimbNode1);
		//}
		pbone->RestoreFbxNodePosture(lSkeletonLimbNode1);
	//}

	////lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(curpos.x - parentpos.x, curpos.y - parentpos.y, curpos.z - parentpos.z));
	//pbone->RestoreFbxNodePosture(lSkeletonLimbNode1);


	parfbxbone->GetSkelNode()->AddChild(lSkeletonLimbNode1);

	fbxbone->SetBone( pbone );
	fbxbone->SetSkelNode( lSkeletonLimbNode1 );
	parfbxbone->AddChild( fbxbone );

	if( pbone->GetChild() ){
		fbxbone->SetType( FB_NORMAL );
	}else{
		//endjoint�o��
		fbxbone->SetType( FB_ENDJOINT );
	}
	s_fbxbonenum++;

	if( pbone->GetChild() ){
		CreateFBXBoneReq( pScene, pbone->GetChild(), fbxbone );
	}
	if( pbone->GetBrother() ){
		CreateFBXBoneReq( pScene, pbone->GetBrother(), parfbxbone );
	}
}
int DestroyFBXBoneReq( CFBXBone* fbxbone )
{
	CFBXBone* childbone = fbxbone->GetChild();
	CFBXBone* brobone = fbxbone->GetBrother();

	delete fbxbone;

	if( childbone ){
		DestroyFBXBoneReq( childbone );
	}
	if( brobone ){
		DestroyFBXBoneReq( brobone );
	}

	return 0;
}


void CreateDummyInfDataReq(CFBXBone* fbxbone, FbxManager*& pSdkManager, FbxScene*& pScene, FbxNode* lMesh, FbxSkin* lSkin, CBone** ppsetbone, int* bonecnt)
{
	//FbxNode* lMesh = CreateDummyFbxMesh(pSdkManager, pScene);
	//srcRootNode->AddChild(lMesh);
	//FbxGeometry* lMeshAttribute = (FbxGeometry*)lMesh->GetNodeAttribute();
	//FbxSkin* lSkin = FbxSkin::Create(pScene, "");

	CBone* curbone = fbxbone->GetBone();
	if (curbone || !ppsetbone) {//bvh�̏ꍇ�ɂ�fbxbone->GetBone() == NULL ����!ppsetbone
		if (!ppsetbone ||(ppsetbone && *(ppsetbone + curbone->GetBoneNo()) == 0)) {
			LinkDummyMeshToSkeleton(fbxbone, lSkin, pScene, lMesh, bonecnt);//������fbxbone->GetBone()�͎g��Ȃ�
			//lMeshAttribute->AddDeformer(lSkin);
			(*bonecnt)++;
		}
	}

	if (fbxbone->GetChild()){
		CreateDummyInfDataReq(fbxbone->GetChild(), pSdkManager, pScene, lMesh, lSkin, ppsetbone, bonecnt);
	}

	if (fbxbone->GetBrother()){
		CreateDummyInfDataReq(fbxbone->GetBrother() , pSdkManager, pScene, lMesh, lSkin, ppsetbone, bonecnt);
	}

}

FbxNode* CreateDummyFbxMesh(FbxManager* pSdkManager, FbxScene* pScene, CBone** ppsetbone)
{
	static int s_namecnt = 0;

	int notdirtycnt = 0;
	int bonecnt;
	if (ppsetbone) {
		for (bonecnt = 0; bonecnt < s_model->GetBoneListSize(); bonecnt++) {
			if (*(ppsetbone + bonecnt) == 0) {
				notdirtycnt++;
			}
		}
		if (notdirtycnt == 0) {
			return NULL;
		}
	}
	else if(s_bvhflag == 1){
		notdirtycnt = s_bvhjointnum;
	}

	char meshname[256] = { 0 };
	sprintf_s(meshname, 256, "_ND_dtri%d", s_namecnt);
	s_namecnt++;
	//int facenum = s_fbxbonenum;
	int facenum = notdirtycnt;

	FbxMesh* lMesh = FbxMesh::Create(pScene, meshname);
	lMesh->InitControlPoints(facenum * 3);
	FbxVector4* lcp = lMesh->GetControlPoints();

	FbxGeometryElementNormal* lElementNormal = lMesh->CreateElementNormal();
	lElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

	FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV("DiffuseUV");
	_ASSERT(lUVDiffuseElement != NULL);
	lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

	int vsetno = 0;
	int vcnt;
	for (vcnt = 0; vcnt < facenum * 3; vcnt++){
		*(lcp + vsetno) = FbxVector4(0.0f, 0.0f, 0.0f, 1.0f);
		FbxVector4 fbxn = FbxVector4(0.0f, 0.0f, -1.0f, 0.0f);
		lElementNormal->GetDirectArray().Add(fbxn);
		FbxVector2 fbxuv = FbxVector2(0.0f, 0.0f);
		lUVDiffuseElement->GetDirectArray().Add(fbxuv);
		vsetno++;
	}

	lUVDiffuseElement->GetIndexArray().SetCount(facenum * 3);

	vsetno = 0;
	int faceno;
	for (faceno = 0; faceno < facenum; faceno++){
		lMesh->BeginPolygon(-1, -1, -1, false);
		int i;
		for (i = 0; i < 3; i++)
		{
			// Control point index
			lMesh->AddPolygon(vsetno);
			//lUVDiffuseElement->GetIndexArray().SetAt(vsetno, vcnt);//vertex, vcnt
			lUVDiffuseElement->GetIndexArray().SetAt(vsetno, vsetno);//vertex, vcnt
			vsetno++;
		}
		lMesh->EndPolygon();
	}


	// create a KFbxNode
	//		KFbxNode* lNode = KFbxNode::Create(pSdkManager,pName);
	FbxNode* lNode = FbxNode::Create(pScene, meshname);
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
	if (!lMesh->GetElementMaterial(0)){
		_ASSERT(0);
		return NULL;
	}

	// add material to the node. 
	// the material can't in different document with the geometry node or in sub-document
	// we create a simple material here which belong to main document
	static int s_matcnt = 0;
	s_matcnt++;


	char matname[256];
	sprintf_s(matname, 256, "material_%d", s_matcnt);
	//KString lMaterialName = mqomat->name;
	FbxString lMaterialName = matname;
	FbxString lShadingName = "Phong";
	FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

	//	lMaterial->Diffuse.Set(fbxDouble3(mqomat->dif4f.r, mqomat->dif4f.g, mqomat->dif4f.b));
	lMaterial->Diffuse.Set(FbxDouble3(1.0, 1.0, 1.0));
	lMaterial->Emissive.Set(FbxDouble3(0.0, 0.0, 0.0));
	lMaterial->Ambient.Set(FbxDouble3(0.0, 0.0, 0.0));
	lMaterial->AmbientFactor.Set(1.0);
	//KFbxTexture* curtex = CreateTexture(pSdkManager, xmat->pTextureFilename);
	//if (curtex){
	//	lMaterial->Diffuse.ConnectSrcObject(curtex);
	//}
	lMaterial->DiffuseFactor.Set(1.0);
	lMaterial->TransparencyFactor.Set(1.0);
	lMaterial->ShadingModel.Set(lShadingName);
	lMaterial->Shininess.Set(0.5);
	lMaterial->Specular.Set(FbxDouble3(0.0, 0.0, 0.0));
	lMaterial->SpecularFactor.Set(1.0);

	lNode->AddMaterial(lMaterial);
	// We are in eBY_POLYGON, so there's only need for index (a plane has 1 polygon).
	lMaterialElement->GetIndexArray().SetCount(lMesh->GetPolygonCount());
	// Set the Index to the material
	//for (int i = 0; i<lMesh->GetPolygonCount(); ++i){
	for (int i = 0; i<lMesh->GetPolygonCount(); i++){
		lMaterialElement->GetIndexArray().SetAt(i, 0);
	}

	return lNode;
}

void LinkDummyMeshToSkeleton(CFBXBone* fbxbone, FbxSkin* lSkin, FbxScene* pScene, FbxNode* pMesh, int* bonecnt)
{
	FbxNode* lSkel;

	//if ((fbxbone->GetType() == FB_NORMAL) || (fbxbone->GetType() == FB_BUNKI_PAR) || (fbxbone->GetType() == FB_BUNKI_CHIL)){
		lSkel = fbxbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return;
		}

		FbxCluster *lCluster = FbxCluster::Create(pScene, "");
		lCluster->SetLink(lSkel);
		//enum  	ELinkMode { eNormalize, eAdditive, eTotalOne }
		lCluster->SetLinkMode(FbxCluster::eTotalOne);
		//lCluster->SetLinkMode(FbxCluster::eNormalize);
		//lCluster->SetLinkMode(FbxCluster::eAdditive);

		int vsetno = *bonecnt * 3;
		int vcnt;
		for (vcnt = 0; vcnt < 3; vcnt++){
			lCluster->AddControlPointIndex(vsetno, 1.0);
			vsetno++;
		}

		/*
		FbxAMatrix lXMatrix;
		lXMatrix.SetIdentity();
		lCluster->SetTransformMatrix(lXMatrix);

		FbxAMatrix lXMatrix2;
		lXMatrix2 = lSkel->EvaluateGlobalTransform();
		lCluster->SetTransformLinkMatrix(lXMatrix2);


		lSkin->AddCluster(lCluster);
		*/

		if (lCluster) {
			FbxScene* lScene = pMesh->GetScene();

			FbxTime fbxtime0;
			fbxtime0.SetSecondDouble(0.0);

			FbxAMatrix MeshMatrix = pMesh->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
			lCluster->SetTransformMatrix(MeshMatrix);
			FbxAMatrix SkelMatrix = lSkel->EvaluateGlobalTransform(fbxtime0, FbxNode::eSourcePivot, true, true);
			lCluster->SetTransformLinkMatrix(SkelMatrix);

			lSkin->AddCluster(lCluster);
		}

		//if (fbxbone->GetBone()){
		//	CBone* curbone = fbxbone->GetBone();
		//	if (curbone){
		//		ChaVector3 pos;
		//		pos = curbone->GetJointFPos();

		//		FbxAMatrix lXMatrix;
		//		lXMatrix.SetIdentity();
		//		lXMatrix[3][0] = -pos.x;
		//		lXMatrix[3][1] = -pos.y;
		//		lXMatrix[3][2] = -pos.z;
		//		lCluster->SetTransformMatrix(lXMatrix);

		//		lXMatrix.SetIdentity();
		//		lCluster->SetTransformLinkMatrix(lXMatrix);
		//	}
		//	else{
		//		FbxAMatrix lXMatrix;
		//		lXMatrix.SetIdentity();
		//		lCluster->SetTransformMatrix(lXMatrix);
		//		lCluster->SetTransformLinkMatrix(lXMatrix);
		//	}
		//	lSkin->AddCluster(lCluster);
		//}
		//else if (fbxbone->GetBvhElem()){
		//	CBVHElem* curbone = fbxbone->GetBvhElem();
		//	if (curbone){
		//		ChaVector3 pos;
		//		pos = curbone->GetPosition();

		//		FbxAMatrix lXMatrix;
		//		lXMatrix.SetIdentity();
		//		lXMatrix[3][0] = -pos.x;
		//		lXMatrix[3][1] = -pos.y;
		//		lXMatrix[3][2] = -pos.z;
		//		lCluster->SetTransformMatrix(lXMatrix);

		//		lXMatrix.SetIdentity();
		//		lCluster->SetTransformLinkMatrix(lXMatrix);
		//	}
		//	else{
		//		FbxAMatrix lXMatrix;
		//		lXMatrix.SetIdentity();
		//		lCluster->SetTransformMatrix(lXMatrix);
		//		lCluster->SetTransformLinkMatrix(lXMatrix);
		//	}
		//	lSkin->AddCluster(lCluster);
		//}

		//}

	/*
	if (fbxbone->m_child){
		LinkDummyMeshToSkeleton(fbxbone->m_child, lSkin, pScene);
	}
	if (fbxbone->m_brother){
		LinkDummyMeshToSkeleton(fbxbone->m_brother, lSkin, pScene);
	}
	*/

}


int WriteFBXAnimTra(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind)
{
	FbxTime lTime;
	int lKeyIndex = 0;
	double timescale = 30.0;


	CBone* curbone = fbxbone->GetBone();
	if (curbone){
		FbxNode* lSkel;
		lSkel = fbxbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return 1;
		}

		FbxAnimCurve* lCurve;
		int frameno;

		const char* strChannel;
		switch (axiskind){
		case AXIS_X:
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		case AXIS_Y:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Y;
			break;
		case AXIS_Z:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Z;
			break;
		default:
			_ASSERT(0);
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		}


		ChaVector3 fbxtra;
		lCurve = lSkel->LclTranslation.GetCurve(lAnimLayer, strChannel, true);
		lCurve->KeyModifyBegin();
		for (frameno = 0; frameno <= maxframe; frameno++){
			fbxtra = curbone->CalcFBXTra(limitdegflag, curmotid, frameno);
			lTime.SetSecondDouble((double)frameno / timescale);
			lKeyIndex = lCurve->KeyAdd(lTime);
			switch (axiskind){
			case AXIS_X:
				lCurve->KeySetValue(lKeyIndex, fbxtra.x);
				break;
			case AXIS_Y:
				lCurve->KeySetValue(lKeyIndex, fbxtra.y);
				break;
			case AXIS_Z:
				lCurve->KeySetValue(lKeyIndex, fbxtra.z);
				break;
			default:
				_ASSERT(0);
				lCurve->KeySetValue(lKeyIndex, fbxtra.x);
				break;
			}

			//lCurve->KeySetValue(lKeyIndex, 0.0f);


			lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
		}
		lCurve->KeyModifyEnd();

	}

	return 0;
}
int WriteFBXAnimRot(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind)
{
	FbxTime lTime;
	int lKeyIndex = 0;
	double timescale = 30.0;


	CBone* curbone = fbxbone->GetBone();
	if (curbone){
		FbxNode* lSkel;
		lSkel = fbxbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return 1;
		}

		FbxAnimCurve* lCurve;
		int frameno;

		const char* strChannel;
		switch (axiskind){
		case AXIS_X:
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		case AXIS_Y:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Y;
			break;
		case AXIS_Z:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Z;
			break;
		default:
			_ASSERT(0);
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		}

		ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);

		lCurve = lSkel->LclRotation.GetCurve(lAnimLayer, strChannel, true);
		lCurve->KeyModifyBegin();
		
		for (frameno = 0; frameno <= maxframe; frameno++){
			cureul = curbone->CalcFBXEulXYZ(limitdegflag, curmotid, frameno, &befeul);
					
			lTime.SetSecondDouble((double)frameno / timescale);
			lKeyIndex = lCurve->KeyAdd(lTime);

			switch (axiskind){
			case AXIS_X:
				lCurve->KeySetValue(lKeyIndex, cureul.x);
				break;
			case AXIS_Y:
				lCurve->KeySetValue(lKeyIndex, cureul.y);
				break;
			case AXIS_Z:
				lCurve->KeySetValue(lKeyIndex, cureul.z);
				break;
			default:
				_ASSERT(0);
				lCurve->KeySetValue(lKeyIndex, cureul.x);
				break;
			}
			lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			if ((frameno == 0) || (frameno == 1) || IsValidNewEul(cureul, befeul)) {
				befeul = cureul;
			}

		}
		lCurve->KeyModifyEnd();
	}
	return 0;
}


int WriteFBXAnimScale(bool limitdegflag, CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int curmotid, int maxframe, int axiskind)
{
	FbxTime lTime;
	int lKeyIndex = 0;
	double timescale = 30.0;


	CBone* curbone = fbxbone->GetBone();
	if (curbone) {
		FbxNode* lSkel;
		lSkel = fbxbone->GetSkelNode();
		if (!lSkel) {
			_ASSERT(0);
			return 1;
		}

		FbxAnimCurve* lCurve;
		int frameno;

		const char* strChannel;
		switch (axiskind) {
		case AXIS_X:
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		case AXIS_Y:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Y;
			break;
		case AXIS_Z:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Z;
			break;
		default:
			_ASSERT(0);
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		}


		ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);

		lCurve = lSkel->LclScaling.GetCurve(lAnimLayer, strChannel, true);
		lCurve->KeyModifyBegin();
		for (frameno = 0; frameno <= maxframe; frameno++) {
			//cureul = curbone->CalcFBXEul(curmotid, frameno, &befeul);
			cureul = curbone->CalcFbxScaleAnim(limitdegflag, curmotid, frameno);
			lTime.SetSecondDouble((double)frameno / timescale);
			lKeyIndex = lCurve->KeyAdd(lTime);

			switch (axiskind) {
			case AXIS_X:
				lCurve->KeySetValue(lKeyIndex, cureul.x);
				break;
			case AXIS_Y:
				lCurve->KeySetValue(lKeyIndex, cureul.y);
				break;
			case AXIS_Z:
				lCurve->KeySetValue(lKeyIndex, cureul.z);
				break;
			default:
				_ASSERT(0);
				lCurve->KeySetValue(lKeyIndex, cureul.x);
				break;
			}
			lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
			if ((frameno == 0) || (frameno == 1) || IsValidNewEul(cureul, befeul)) {
				befeul = cureul;
			}
		}
		lCurve->KeyModifyEnd();
	}
	return 0;
}


int WriteFBXAnimTraOfBVH(CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int axiskind, int zeroflag)
{
	FbxTime lTime;
	int lKeyIndex = 0;
	FbxNode* lSkel = 0;
	double timescale = 30.0;

	CBVHElem* curbe = fbxbone->GetBvhElem();
	_ASSERT(curbe);
	if (curbe){

		lSkel = fbxbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return 1;
		}
		ChaVector3 difftra, orgtra, settra;
		CBVHElem* parbe;// = curbe->GetParent();
		if (fbxbone->GetParent()){
			parbe = fbxbone->GetParent()->GetBvhElem();
		}
		else{
			parbe = 0;
		}
		if (parbe){
			if (parbe == s_fbxbone->GetBvhElem()){
				orgtra = curbe->GetPosition();
			}
			else{
				orgtra = curbe->GetPosition() - parbe->GetPosition();
			}
		}
		else{
			orgtra = curbe->GetPosition();
		}

		const char* strChannel;
		switch (axiskind){
		case AXIS_X:
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		case AXIS_Y:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Y;
			break;
		case AXIS_Z:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Z;
			break;
		default:
			_ASSERT(0);
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		}

		FbxAnimCurve* lCurve;
		int frameno;

		lCurve = lSkel->LclTranslation.GetCurve(lAnimLayer, strChannel, true);
		lCurve->KeyModifyBegin();
		//for (frameno = 0; frameno <= curbe->GetFrameNum(); frameno++){
		for (frameno = 0; frameno < curbe->GetFrameNum(); frameno++) {
			curbe->GetTrans(frameno, &difftra);
			lTime.SetSecondDouble((double)frameno / timescale);
			lKeyIndex = lCurve->KeyAdd(lTime);
			if (zeroflag == 1){
				lCurve->KeySetValue(lKeyIndex, 0.0f);
			}
			else{
				switch (axiskind){
				case AXIS_X:
					lCurve->KeySetValue(lKeyIndex, difftra.x + orgtra.x);
					break;
				case AXIS_Y:
					lCurve->KeySetValue(lKeyIndex, difftra.y + orgtra.y);
					break;
				case AXIS_Z:
					lCurve->KeySetValue(lKeyIndex, difftra.z + orgtra.z);
					break;
				default:
					_ASSERT(0);
					lCurve->KeySetValue(lKeyIndex, difftra.x + orgtra.x);
					break;
				}
			}
			lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
		}
		lCurve->KeyModifyEnd();
	}

	return 0;
}

int WriteFBXAnimRotOfBVH(CFBXBone* fbxbone, FbxAnimLayer* lAnimLayer, int axiskind, int zeroflag)
{
	FbxTime lTime;
	int lKeyIndex = 0;
	FbxNode* lSkel = 0;
	double timescale = 30.0;

	CBVHElem* curbe = fbxbone->GetBvhElem();
	_ASSERT(curbe);
	if (curbe){

		lSkel = fbxbone->GetSkelNode();
		if (!lSkel){
			_ASSERT(0);
			return 1;
		}

		FbxAnimCurve* lCurve;
		int frameno;
		float curval;

		const char* strChannel;
		switch (axiskind){
		case AXIS_X:
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		case AXIS_Y:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Y;
			break;
		case AXIS_Z:
			strChannel = FBXSDK_CURVENODE_COMPONENT_Z;
			break;
		default:
			_ASSERT(0);
			strChannel = FBXSDK_CURVENODE_COMPONENT_X;
			break;
		}

		lCurve = lSkel->LclRotation.GetCurve(lAnimLayer, strChannel, true);
		lCurve->KeyModifyBegin();
		//for (frameno = 0; frameno <= curbe->GetFrameNum(); frameno++){
		for (frameno = 0; frameno < curbe->GetFrameNum(); frameno++) {
			lTime.SetSecondDouble((double)frameno / timescale);
			lKeyIndex = lCurve->KeyAdd(lTime);
			if (zeroflag == 1){
				curval = 0.0f;
			}
			else{
				switch (axiskind){
				case AXIS_X:
					curval = curbe->GetXYZRot(frameno, ROTAXIS_X);
					break;
				case AXIS_Y:
					curval = curbe->GetXYZRot(frameno, ROTAXIS_Y);
					break;
				case AXIS_Z:
					curval = curbe->GetXYZRot(frameno, ROTAXIS_Z);
					break;
				default:
					_ASSERT(0);
					curval = curbe->GetXYZRot(frameno, ROTAXIS_X);
					break;
				}
			}
			lCurve->KeySetValue(lKeyIndex, curval);
			lCurve->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationLinear);
		}
		lCurve->KeyModifyEnd();
	}

	return 0;
}


// Get the matrix of the given pose
FbxAMatrix FbxGetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	lPoseMatrix.SetIdentity();

	if (pPose) {
		FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);
		memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));
	}

	return lPoseMatrix;
}

// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix FbxGetGeometry(FbxNode* pNode)
{
	if (pNode != NULL) {
		const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(lT, lR, lS);
	}
	else {
		FbxAMatrix initmat;
		initmat.SetIdentity();
		return initmat;
	}
}

FbxAMatrix FbxGetGlobalPosition(bool usecache, CModel* srcmodel, FbxScene* pScene, FbxNode* pNode, const FbxTime& pTime, int srcframe, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
{
	FbxAMatrix lGlobalPosition;
	bool        lPositionFound = false;

	char bonename2[256];
	//strcpy_s(bonename2, 256, clusterlink->GetName());
	strcpy_s(bonename2, 256, pNode->GetName());
	//srcmodel->TermJointRepeats(bonename2);
	CBone* curbone = srcmodel->GetBoneByName(bonename2);

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
				lGlobalPosition = FbxGetPoseMatrix(pPose, lNodeIndex);
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
						lParentGlobalPosition = FbxGetGlobalPosition(usecache, srcmodel, pScene, pNode->GetParent(), pTime, srcframe, pPose);
					}
				}

				FbxAMatrix lLocalPosition = FbxGetPoseMatrix(pPose, lNodeIndex);
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
		//lGlobalPosition = pNode->EvaluateGlobalTransform(pTime, fbxsdk::FbxNode::eDestinationPivot, true, false);
		//lGlobalPosition = pNode->EvaluateGlobalTransform(pTime, fbxsdk::FbxNode::eDestinationPivot, false, false);


		//lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);

		FbxAnimEvaluator* animevaluator = pScene->GetAnimationEvaluator();
		if (animevaluator) {
			//animevaluator->Flush(pNode);
			//lGlobalPosition = animevaluator->GetNodeGlobalTransform(pNode, pTime, fbxsdk::FbxNode::eDestinationPivot);
			lGlobalPosition = animevaluator->GetNodeGlobalTransform(pNode, pTime, fbxsdk::FbxNode::eDestinationPivot, true, true);
			//lGlobalPosition = animevaluator->GetNodeGlobalTransform(pNode, pTime, fbxsdk::FbxNode::eDestinationPivot, false, false);
		}
		else {
			lGlobalPosition.SetIdentity();
		}

	}

	return lGlobalPosition;
}


void FbxSetDefaultBonePosReq(FbxScene* pScene, CModel* pmodel, CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* ParentGlobalPosition)
{
	if (!pScene || !pmodel || !curbone) {
		return;
	}

	//FbxNode* pNode = pmodel->m_bone2node[curbone];
	FbxNode* pNode = pmodel->GetBoneNode(curbone);

	FbxAMatrix lGlobalPosition;
	bool        lPositionFound = false;//�o�C���h�|�[�Y�������o���Ȃ��ꍇ��Hips�Ȃǂ̏ꍇ�͂O�ɂȂ�H

	lGlobalPosition.SetIdentity();


	//FbxSkeleton* pskeleton = pNode->GetSkeleton();


	//2022/11/23
	bool oldbvh = false;
	FbxDocumentInfo* sceneinfo = pScene->GetSceneInfo();
	if (sceneinfo) {
		FbxString oldauthor = "OpenRDB user";
		if (sceneinfo->mAuthor == oldauthor) {
			_ASSERT(0);
			FbxString currentrev1 = "rev. 2.7";
			FbxString currentrev2 = "rev. 2.8";
			//2.7, 2.8�����e�ύX��̐V�o�[�W����
			if ((sceneinfo->mRevision != currentrev1) && (sceneinfo->mRevision != currentrev2)) {
				oldbvh = true;//!!!!!!!!!!!!!!!!!!!!
			}
		}
	}


	if (((pmodel->GetFromBvhFlag() == false) || ((pmodel->GetFromBvhFlag() == true) && (oldbvh == false))) &&
		pmodel->GetHasBindPose()) {//Pose������ꍇ�ł�BindPose�łȂ��ꍇ�͏��O����
	//if (bvhflag == 0) {
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
					else
					{
						// We have a local matrix, we need to convert it to
						// a global space matrix.
						FbxAMatrix lParentGlobalPosition;

						//if (pParentGlobalPosition)
						if (curbone->GetParent())
						{
							lParentGlobalPosition = *ParentGlobalPosition;
						}
						else
						{
							if (pNode->GetParent())
							{
								//time == 0.0�����̃L���b�V��������s�v�Z
								bool usecache = false;
								int dummyframe = 0;
								lParentGlobalPosition = FbxGetGlobalPosition(usecache, pmodel, pNode->GetScene(), pNode->GetParent(), pTime, dummyframe, pPose);
							}
						}

						FbxAMatrix lLocalPosition = FbxGetPoseMatrix(pPose, lNodeIndex);
						lGlobalPosition = lParentGlobalPosition * lLocalPosition;
					}

					lPositionFound = true;
				}
			}
		}
	}

	if (lPositionFound) {//BindPose������ꍇ�Bbvh�͂�����ł͂Ȃ�else�̕�
		curbone->SetBindMat(lGlobalPosition);

		ChaMatrix nodemat0, nodemat, nodeanimmat;
		nodemat0.SetIdentity();
		nodemat.SetIdentity();
		nodeanimmat.SetIdentity();
		ChaMatrix localnodemat, localnodeanimmat;
		CalcLocalNodeMat(pmodel, curbone, &localnodemat, &localnodeanimmat);//2022/12/21 support prerot postrot ...etc.
		ChaMatrix parentnodemat, parentnodeanimmat;
		parentnodemat.SetIdentity();
		parentnodeanimmat.SetIdentity();
		if (curbone->GetParent()) {
			parentnodemat = curbone->GetParent()->GetNodeMat();
			parentnodeanimmat = curbone->GetParent()->GetNodeAnimMat();
		}
		nodemat0 = localnodemat * parentnodemat;
		nodeanimmat = localnodeanimmat * parentnodeanimmat;//!!!!!!!!! bindmat�Ɠ���
		
		
		//nodemat = nodeanimmat;
		nodemat = ChaMatrixFromFbxAMatrix(lGlobalPosition);//!!!!!! nodeanimmat�Ɠ���
		//nodemat = nodemat0;

		//curbone->SetPositionFound(lPositionFound);//!!!
		curbone->SetPositionFound(true);//!!! 2022/07/30 bone mark��\�����邽��true�ɁB

		curbone->SetNodeMat(nodemat);//2022/12/19
		//curbone->SetNodeMat(nodeanimmat);
		curbone->SetNodeAnimMat(nodeanimmat);//2022/12/21

		curbone->SetGlobalPosMat(lGlobalPosition);

		ChaVector3 zeropos(0.0f, 0.0f, 0.0f);
		ChaVector3 tmppos;
		ChaMatrix tmpnm = curbone->GetNodeMat();
		ChaVector3TransformCoord(&tmppos, &zeropos, &tmpnm);
		curbone->SetJointWPos(tmppos);
		curbone->SetJointFPos(tmppos);
	}
	else {
		// There is no pose entry for that node, get the current global position instead.

		// Ideally this would use parent global position and local position to compute the global position.
		// Unfortunately the equation 
		//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
		// does not hold when inheritance type is other than "Parent" (RSrs).
		// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.

		ChaMatrix nodemat, nodeanimmat;
		nodemat.SetIdentity();
		nodeanimmat.SetIdentity();

		if (pNode) {
			//time == 0.0 �̂P�t���[���������̃L���b�V��������s�v�Z
			//joint�̌������ݒ肳��Ă���ꍇ�ɃA�j���[�V�����̊�Ƃ��Ďg���Ƃ��������Ȃ���
			//BindMat���ݒ肳��Ă��Ȃ��ꍇ(���������s����ꍇ)�ɂ͂��܂�����
			//�I�I�I�I�@BindMat���ݒ肵�Ă���ꍇ��BindMat���g���@�I�I�I�I
			//lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
			//nodemat = ChaMatrixFromFbxAMatrix(lGlobalPosition);


			ChaMatrix localnodemat, localnodeanimmat;
			CalcLocalNodeMat(pmodel, curbone, &localnodemat, &localnodeanimmat);//2022/12/21 support prerot postrot ...etc.
			ChaMatrix parentnodemat, parentnodeanimmat;
			parentnodemat.SetIdentity();
			parentnodeanimmat.SetIdentity();
			if (curbone->GetParent()) {
				parentnodemat = curbone->GetParent()->GetNodeMat();
				parentnodeanimmat = curbone->GetParent()->GetNodeAnimMat();
			}
			nodemat = localnodemat * parentnodemat;
			nodeanimmat = localnodeanimmat * parentnodeanimmat;
		}


		//curbone->SetPositionFound(lPositionFound);//!!!
		curbone->SetPositionFound(true);//!!! 2022/07/30 bone mark��\�����邽��true�ɁB

		curbone->SetNodeMat(nodemat);
		curbone->SetNodeAnimMat(nodeanimmat);
		curbone->SetGlobalPosMat(lGlobalPosition);

		ChaVector3 zeropos(0.0f, 0.0f, 0.0f);
		ChaVector3 tmppos;
		ChaMatrix tmpnm = curbone->GetNodeMat();
		ChaVector3TransformCoord(&tmppos, &zeropos, &tmpnm);
		curbone->SetJointWPos(tmppos);
		curbone->SetJointFPos(tmppos);


		//}
	}

	//WCHAR wname[256];
	//ZeroMemory( wname, sizeof( WCHAR ) * 256 );
	//MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, curbone->m_bonename, 256, wname, 256 );
	//DbgOut( L"SetDefaultBonePos : %s : wpos (%f, %f, %f)\r\n", wname, curbone->m_jointfpos.x, curbone->m_jointfpos.y, curbone->m_jointfpos.z );


	if (curbone->GetChild()) {
		//if (curbone->GetChild()->GetChild()) {
		FbxAMatrix parentposition = curbone->GetGlobalPosMat();
		FbxSetDefaultBonePosReq(pScene, pmodel, curbone->GetChild(), pTime, pPose, &parentposition);
		//}
		//else {
		//	SetDefaultBonePosReq(curbone->GetBrother(), pTime, pPose, ParentGlobalPosition);
		//}
	}
	if (curbone->GetBrother()) {
		FbxSetDefaultBonePosReq(pScene, pmodel, curbone->GetBrother(), pTime, pPose, ParentGlobalPosition);
	}

}


// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
FbxDouble3 FbxGetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
	const char* pPropertyName,
	const char* pFactorPropertyName,
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

FbxDouble FbxGetMaterialShininessProperty(const FbxSurfaceMaterial* pMaterial)
{
	FbxDouble lResult(0.0);
	const FbxProperty lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
	if (lProperty.IsValid())
	{
		lResult = lProperty.Get<FbxDouble>();
	}

	return lResult;
}

int IsValidFbxCluster(FbxCluster* cluster)
{
	int findflag = 0;

	int pointNum = cluster->GetControlPointIndicesCount();
	int* pointAry = cluster->GetControlPointIndices();
	double* weightAry = cluster->GetControlPointWeights();

	FbxCluster::ELinkMode lClusterMode = cluster->GetLinkMode();

	int index;
	double weight;
	for (int i2 = 0; i2 < pointNum; i2++) {
		// ���_�C���f�b�N�X�ƃE�F�C�g���擾
		index = pointAry[i2];
		weight = weightAry[i2];

		if ((lClusterMode == FbxCluster::eAdditive) || (weight >= 0.05)) {
			//if ((lClusterMode == FbxCluster::eAdditive)){
			findflag = 1;
			break;
		}
	}

	return findflag;
}


int SaveCurrentMotionID(CModel* curmodel)
{
	s_zeroframemotid = 0;

	if (!curmodel) {
		_ASSERT(0);
		return 1;
	}

	MOTINFO* curmi = curmodel->GetCurMotInfo();
	if (curmi) {
		s_zeroframemotid = curmi->motid;
	}
	else {
		_ASSERT(0);
		return 0;
	}

	return 0;
}
