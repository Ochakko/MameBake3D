#ifndef MODELH
#define MODELH

#include <d3dx9.h>
#include <wchar.h>
#include <Coef.h>
#include <string>
#include <map>

#include <OrgWindow.h>

#include <fbxsdk.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

#include <UndoMotion.h>

using namespace std;

class CMQOMaterial;
class CMQOObject;
class CMQOFace;
class CBone;
class CInfScope;
class CMySprite;
class CMotionPoint;
class CQuaternion;
class CBtObject;
class CRigidElem;
class CEditRange;

typedef struct tag_newmpelem
{
	CBone* boneptr;
	CMotionPoint* mpptr;
}NEWMPELEM;

typedef struct tag_fbxobj
{
	FbxNode* node;
	FbxMesh* mesh;
}FBXOBJ;


class CModel
{
public:
	CModel();
	~CModel();

	int LoadMQO( LPDIRECT3DDEVICE9 pdev, WCHAR* wfile, WCHAR* modelfolder, float srcmult, int ismedia, int texpool = D3DPOOL_DEFAULT );
	int LoadFBX( int skipdefref, LPDIRECT3DDEVICE9 pdev, WCHAR* wfile, WCHAR* modelfolder, float srcmult, FbxManager* psdk, FbxImporter** ppimporter, FbxScene** ppscene );
	int LoadFBXAnim( FbxManager* psdk, FbxImporter* pimporter, FbxScene* pscene, int (*tlfunc)( int srcmotid ) );

	int OnRender( LPDIRECT3DDEVICE9 pdev, int lightflag, D3DXVECTOR4 diffusemult, int btflag = 0 );
	int RenderBoneMark( LPDIRECT3DDEVICE9 pdev, CModel* bmarkptr, CMySprite* bcircleptr, CModel* cpslptr[COL_MAX], int selboneno );
	int GetModelBound( MODELBOUND* dstb );

	int MakeObjectName();
	int MakeBoneTri();
	int MakeInfScope();
	int MakePolyMesh3();
	int MakePolyMesh4();
	int MakeExtLine();
	int MakeDispObj();

	int CalcInf();

	int DbgDump();

	int UpdateMatrix( int startbone, D3DXMATRIX* wmat, D3DXMATRIX* vpmat );
	int SetShaderConst( CMQOObject* srcobj, int btflag = 0 );

	int SetBoneEul( int boneno, D3DXVECTOR3 srceul );
	int GetBoneEul( int boneno, D3DXVECTOR3* dsteul );

	int FillTimeLine( OrgWinGUI::OWP_Timeline& timeline, map<int, int>& lineno2boneno, map<int, int>& boneno2lineno );

	int AddMotion( char* srcname, WCHAR* wfilename, double srcleng, int* dstid );
	CMotionPoint* AddMotionPoint( int srcmotid, double srcframe, int srcboneno, int calcflag, int* existptr );

	int SetCurrentMotion( int srcmotid );
	int SetMotionFrame( double srcframe );
	int GetMotionFrame( double* dstframe );
	int SetMotionSpeed( double srcspeed );
	int GetMotionSpeed( double* dstspeed );

	int DeleteMotion( int motid );
	int GetSymBoneNo( int srcboneno, int* dstboneno, int* existptr );

	int PickBone( PICKINFO* pickinfo );
	int IKRotate( CEditRange* erptr, int srcboneno, D3DXVECTOR3 targetpos, int maxlevel );
	int IKRotateAxisDelta( CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt );
	int RotateXDelta( CEditRange* erptr, int srcboneno, float delta );
	int IKMove( int srcboneno, D3DXVECTOR3 targetpos );
	int IKMoveAxisDelta( int axiskind, int srcboneno, float deltax );

	int FKRotate( CEditRange* erptr, int srcboneno, int srcmotid, double srcframe, CQuaternion rotq );
	int FKBoneTra( CEditRange* erptr, int srcboneno, int srcmotid, double srcframe, D3DXVECTOR3 addtra );


	int CollisionNoBoneObj_Mouse( PICKINFO* pickinfo, char* objnameptr );
	int TransformBone( int winx, int winy, int srcboneno, D3DXVECTOR3* worldptr, D3DXVECTOR3* screenptr, D3DXVECTOR3* dispptr );
	int ChangeMotFrameLeng( int motid, double srcleng );
	int AdvanceTime( int previewflag, double difftime, double* nextframeptr, int* endflagptr, int srcmotid );

	int MakeEnglishName();
	int CalcXTransformMatrix( float mult );
	int AddDefMaterial();

	int SetMiko3rdVec( int boneno, D3DXVECTOR3 srcvec );
	int UpdateMatrixNext();
	int SetKinectFirstPose();

	CBone* GetValidBroBone( CBone* srcbone );
	CBone* GetValidChilBone( CBone* srcbone );

	int CreateBtObject( CModel* coldisp[COL_MAX], int onfirstcreate );
	//int CalcBtAxismat( float delta );

	int CreateBtConstraint();
	void CreateBtConstraintReq( CBtObject* curbto );
	int SetBtMotion( int rgdollflag, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat, double difftime );
	int SetBtImpulse();

	int SetImp( int srcboneno, int kind, float srcval );
	int SetDispFlag( char* objname, int flag );
	CBtObject* FindBtObject( int srcboneno );
	int SetAllDampAnimData( int gid, int rgdindex, float valL, float valA );
	int SetAllBtgData( int gid, int reindex, float btg );
	int SetAllImpulseData( int gid, float impx, float impy, float impz );
	int SetAllKData( int gid, int reindex, int srclk, int srcak, float srccuslk, float srccusak );
	int SetAllRestData( int gid, int reindex, float rest, float fric );
	int SetAllMassData( int gid, int reindex, float srcmass );
	int SetAllDmpData( int gid, int reindex, float ldmp, float admp );

	int Motion2Bt( int firstflag, CModel* coldisp[COL_MAX], double nextframe, D3DXMATRIX* mW, D3DXMATRIX* mVP );
	int SetRagdollKinFlag();
	int SetCurrentRigidElem( int curindex );

	void CreateRigidElemReq( CBone* curbone, int reflag, string rename, int impflag, string impname );

	int MultDispObj( D3DXVECTOR3 srcmult, D3DXVECTOR3 srctra );
	MOTINFO* GetRgdMorphInfo();

	int SetColiIDtoGroup( CRigidElem* curre );
	int ResetBt();

	int InitUndoMotion( int saveflag );
	int SaveUndoMotion( int curboneno, int curbaseno );
	int RollBackUndoMotion( int redoflag, int* curboneno, int* curbaseno );

	int AddBoneMotMark( OrgWinGUI::OWP_Timeline* owpTimeline, int curboneno, int curlineno, double startframe, double endframe, int flag );

	int MotionID2Index( int motid );
	FbxAnimLayer* GetAnimLayer( int motid );
	float GetTargetWeight( int motid, double dframe, double timescale, CMQOObject* baseobj, std::string targetname );

private:
	int InitParams();
	int DestroyObjs();
	int CreateMaterialTexture();

	int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb );

	int DestroyMaterial();
	int DestroyObject();
	int DestroyAncObj();
	int DestroyAllMotionInfo();

	void MakeBoneReq( CBone* parbone, CMQOFace* curface, D3DXVECTOR3* pointptr, int broflag, int* errcntptr );

	int SetShapeNoReq( CMQOFace** ppface, int facenum, int searchp, int shapeno, int* setfacenum);
	int SetFaceOfShape( CMQOFace** ppface, int facenum, int shapeno, CMQOFace** ppface2, int setfacenum );

	int DbgDumpBoneReq( CBone* boneptr, int broflag );

	void UpdateMatrixReq( int srcmotid, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat, 
		D3DXMATRIX* parmat, CQuaternion* parq, CBone* srcbone, int broflag );

	void FillTimelineReq( OrgWinGUI::OWP_Timeline& timeline, CBone* curbone, int* linenoptr, 
		map<int, int>& lineno2boneno, map<int, int>& boneno2lineno, int broflag );

	void SetSelectFlagReq( CBone* boneptr, int broflag );
	int CalcMouseLocalRay( PICKINFO* pickinfo, D3DXVECTOR3* startptr, D3DXVECTOR3* dirptr );
	CBone* GetCalcRootBone( CBone* firstbone, int maxlevel );
	void CalcXTransformMatrixReq( CBone* srcbone, D3DXMATRIX parenttra, float mult );


	int InitFBXManager( FbxManager** ppSdkManager, FbxImporter** ppImporter, FbxScene** ppScene, char* utfname );
	int CreateFBXMeshReq( FbxNode* pNode );
	int CreateFBXShape( FbxAnimLayer* panimlayer, int animleng, FbxTime starttime, FbxTime timestep );

	CMQOObject* GetFBXMesh( FbxNode* pNode, FbxNodeAttribute *pAttrib, const char* nodename );
	int GetFBXShape(FbxMesh* pMesh, CMQOObject* curobj, FbxAnimLayer* panimlayer, int animleng, FbxTime starttime, FbxTime timestep );
	//int ComputeShapeDeformation(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );
	//int ComputeShapeDeformation2(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );

	int SetMQOMaterial( CMQOMaterial* newmqomat, FbxSurfaceMaterial* material );
	int CreateFBXBoneReq( FbxScene* pScene, FbxNode* pNode, FbxNode* parnode );
	int GetFBXBone( FbxScene* pScene, FbxNodeAttribute::EType type, FbxNodeAttribute *pAttrib, const char* nodename, FbxNode* curnode, FbxNode* parnode );
	int CreateFBXAnim( FbxScene* pScene, FbxNode* prootnode );
	int CreateFBXAnimReq( int animno, FbxPose* pPose, FbxNode* pNode, int motid, int animleng, FbxTime mStart, FbxTime mFrameTime );
	int GetFBXAnim( int animno, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute *pAttrib, int motid, int animleng, FbxTime mStart, FbxTime mFrameTime );
	int CreateFBXSkinReq( FbxNode* pNode );
	int GetFBXSkin( FbxNodeAttribute *pAttrib, FbxNode* pNode );

	int DestroyFBXSDK();
	int GetShapeWeight(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj );
	float GetFbxTargetWeight(FbxNode* pbaseNode, FbxMesh* pbaseMesh, std::string targetname, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* baseobj );
	
	int SetDefaultBonePos();
	void SetDefaultBonePosReq( CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition );

	void FillUpEmptyKeyReq( int motid, int animleng, CBone* curbone, CBone* parbone );

	int SetMaterialName();

	int DestroyBtObject();
	void DestroyBtObjectReq( CBtObject* curbt );
	void CreateBtObjectReq( CModel* cpslptr[COL_MAX], CBtObject* parbt, CBone* parbone, CBone* curbone );

	void CalcBtAxismatReq( CModel* coldisp[COL_MAX], CBone* curbone, float delta );
	void SetBtMotionReq( CBtObject* curbto, D3DXMATRIX* wmat, D3DXMATRIX* vpmat );

	void FindBtObjectReq( CBtObject* srcbto, int srcboneno, CBtObject** ppret );
	void SetImpulseDataReq( int gid, CBone* srcbone, D3DXVECTOR3 srcimp );
	void SetBtImpulseReq( CBone* srcbone );
	
	void SetDampAnimDataReq( int gid, int rgdindex, CBone* srcbone, float valL, float valA );
	void SetBtgDataReq( int gid, int reindex, CBone* srcbone, float btg );
	void SetKDataReq( int gid, int reindex, CBone* srcbone, int srclk, int srcak, float srccuslk, float srccusak );
	void SetRestDataReq( int gid, int reindex, CBone* srcbone, float rest, float fric );
	void SetDmpDataReq( int gid, int reindex, CBone* srcbone, float ldmp, float admp );
	void SetMassDataReq( int gid, int reindex, CBone* srcbone, float srcmass );
	void FindTopBoneReq( int motid, CBone* curbone, CBone** ppret );
	void SetBtKinFlagReq( CBtObject* srcbto, int oncreateflag );
	void Motion2BtReq( CBtObject* srcbto );
	void SetBtGravityReq( CBtObject* srcbto );
	void SetRagdollKinFlagReq( CBtObject* srcbto );
	void CreateBtConnectReq( CBone* curbone );
	void SetColiIDReq( CBone* srcbone, CRigidElem* srcre );

	int DampAnim( MOTINFO* rgdmorphinfo );

	void ResetBtReq( CBtObject* curbto );

	int GetValidUndoID( int* rbundoid );
	int GetValidRedoID( int* rbundoid );

	int SetBefEditMat( CEditRange* erptr, CBone* curbone, int maxlevel );
	int SetBefEditMatFK( CEditRange* erptr, CBone* curbone );

	int AdjustBoneTra( CEditRange* erptr, CBone* lastpar );


public:
	char m_defaultrename[MAX_PATH];
	char m_defaultimpname[MAX_PATH];

	map<CMQOObject*, FBXOBJ> m_fbxobj;
	int m_modelno;

	LPDIRECT3DDEVICE9 m_pdev;
	btDynamicsWorld* m_btWorld;

	bool m_modeldisp;
	map<int, CMQOMaterial*> m_material;
	map<string, CMQOMaterial*> m_materialname;
	map<int, CMQOObject*> m_object;
	map<string, CMQOObject*> m_objectname;

	map<int, CMQOMaterial*> m_ancmaterial;
	map<int, CMQOObject*> m_ancobject;
	map<int, CMQOObject*> m_boneobject;
	map<string, CBone*> m_bonename;

	map<int, CBone*> m_bonelist;
	map<CBone*, FbxNode*> m_bone2node;

	//int m_topbonenum;
	CBone* m_topbone;
	FbxNode* m_firstbone;
	CBtObject* m_topbt;

	map<int, CInfScope*> m_infscope;


	WCHAR m_filename[MAX_PATH];
	WCHAR m_dirname[MAX_PATH];
	WCHAR m_modelfolder[MAX_PATH];

	float m_loadmult;

	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matVP;

	map<int, MOTINFO*> m_motinfo;
	MOTINFO* m_curmotinfo;

	vector<NEWMPELEM> m_newmplist;

    FbxArray<FbxString*> mAnimStackNameArray;
	int (*m_tlFunc)( int srcmotid );
	FbxTime m_ktime0;
	FbxAnimEvaluator* m_animevaluator;
	mutable FbxTime mStart, mStop, mFrameTime, mFrameTime2;

	FbxManager* m_psdk;
	FbxImporter* m_pimporter;
	FbxScene* m_pscene;

	int m_btcnt;
	//float m_btg;
	float m_btgscale;

	map<CBone*,CBone*> m_rigidbone;
	vector<REINFO> m_rigideleminfo;
	vector<string> m_impinfo;

	int m_curreindex;
	int m_curimpindex;
	int m_rgdindex;
	int m_rgdmorphid;

	float m_tmpmotspeed;
	int m_texpool;

	D3DXVECTOR3 m_ikrotaxis;

	CUndoMotion m_undomotion[ UNDOMAX ];
	int m_undoid;

};

#endif