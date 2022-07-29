#ifndef MODELH
#define MODELH


//class ID3D11Device;

//#include <d3dx9.h>
#include <wchar.h>
#include <string>
#include <map>

#include <Coef.h>
#include <OrgWindow.h>

#include <ChaVecCalc.h>
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

#include <fbxsdk.h>


#include <UndoMotion.h>

using namespace std;

class CMQOMaterial;
class CMQOObject;
class CMQOFace;
class CBone;
class CMySprite;
class CMotionPoint;
class CQuaternion;
class CBtObject;
class CRigidElem;
class CEditRange;

typedef struct funcmpparams
{
	int slotno;
	FbxMesh* fbxmesh;
	CModel* pmodel;
	FbxNode* jointnode;
	FbxNode* linknode;
	int framestart;
	int frameend;
	CBone* curbone;
	int animno;
	int motid;
	double animleng;
	FbxCluster* cluster;
	FbxPose* pPose;
	FbxAMatrix globalcurrentpos;
}FUNCMPPARAMS;


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


typedef struct tag_physikrec
{
	double time;
	CBone* pbone;
	ChaMatrix btmat;
}PHYSIKREC;

#define MAXPHYSIKRECCNT		(60 * 60)

class CModel
{
public:

/**
 * @fn
 * CModel
 * @breaf コンストラクタ
 * @return なし。
 */
	CModel();

/**
 * @fn
 * ~CModel
 * @breaf デストラクタ。
 * @return デストラクタ。
 */
	~CModel();

/**
 * @fn
 * LoadMQO
 * @breaf メタセコイアで作成した３Dデータファイル*.mqoを読み込む。
 * @param (ID3D11Device* pdev) IN Direct3Dのデバイス。
 * @param (WCHAR* wfile) IN mqoファイルのフルパス。
 * @param (WCHAR* modelfolder) IN FBX書き出しの際に使用するモデルフォルダー名。ファイル名から拡張子を取ったものに通し番号を付けたものがデフォルト。
 * @param (float srcmult) IN 読み込み倍率。
 * @param (int ismedia) IN SDKmisc.cppのDXUTFindDXSDKMediaFileCchで探すディレクトリ内のファイルかどうかというフラグ。
 * @param (int texpool = 0) IN テクスチャを作成する場所。
 * @return 成功したら０。
 * @detail texpool引数にはデフォルト値があるので省略可能。
 */
	int LoadMQO( ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, WCHAR* wfile, WCHAR* modelfolder, float srcmult, int ismedia, int texpool = 0 );
	
/**
 * @fn
 * LoadFBX
 * @breaf FBXファイルを読み込む。
 * @param (int skipdefref) IN デフォルト剛体設定を利用をスキップするかどうかのフラグ。chaファイルから呼び出すときはrefファイルがあるので１。FBX単体で読み込むときは０。
 * @param (ID3D11Device* pdev) IN Direct3DのDevice。
 * @param (WCHAR* wfile) IN FBXファイルのフルパス。
 * @param (WCHAR* modelfolder) IN FBX書き出し時に使用するFBXファイルがあるフォルダの名前。chaファイルがあるフォルダの中のFBXがあるフォルダの名前となる。
 * @param (float srcmult) IN 読み込み倍率。
 * @param (FbxManager* psdk) IN FBXSDKのマネージャ。
 * @param (FbxImporter** ppimporter) IN FBXSDKのインポーター。
 * @param (FbxScene** ppscene) IN FBXSDKのシーン。
 * @param (int forcenewaxisflag) 過渡期ファイルのフラグ。
 * @return 成功したら０。
 */
	int LoadFBX( int skipdefref, ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, WCHAR* wfile, WCHAR* modelfolder, float srcmult, FbxManager* psdk, FbxImporter** ppimporter, FbxScene** ppscene, int forcenewaxisflag, BOOL motioncachebatchflag);

/**
 * @fn
 * LoadFBXAnim
 * @breaf FBXのアニメーションを読み込む。
 * @param (FbxManager* psdk) IN FBXSDKのマネージャ。
 * @param (FbxImporter* pimporter) IN FBXSDKのインポーター。
 * @param (FbxScene* pscene) IN FBXSDKのシーン。
 * @param (int (*tlfunc)( int srcmotid )) IN タイムライン初期化用の関数へのポインタ。
 * @return 成功したら０。
 * @detail LoadFBX呼び出しが成功した後でこの関数を呼び出す。
 */
	int LoadFBXAnim( FbxManager* psdk, FbxImporter* pimporter, FbxScene* pscene, int (*tlfunc)( int srcmotid ), BOOL motioncachebatchflag);


/**
 * @fn
 * OnRender
 * @breaf モデルデータを描画する。
 * @param (ID3D11Device* pdev) IN Direct3DのDevice。
 * @param (int lightflag) IN 照光処理をするかどうかのフラグ。
 * @param (ChaVector4 diffusemult) IN ディフューズ(拡散光)に乗算するRGBAの値。ライトを変えなくても明るさを変えることが出来る。
 * @param (int btflag = 0) IN bulletのシミュレーション中であるかどうかのフラグ。
 * @return 成功したら０。
 */
	int OnRender(ID3D11DeviceContext* pd3dImmediateContext, int lightflag, ChaVector4 diffusemult, int btflag = 0 );

	int RenderRefArrow(ID3D11DeviceContext* pd3dImmediateContext, CBone* boneptr, ChaVector4 diffusemult, int refmult, std::vector<ChaVector3> vecbonepos);
/**
 * @fn
 * RenderBoneMark
 * @breaf ボーンマークとジョイントマークと剛体形状を表示する。
 * @param (ID3D11Device* pdev) IN Direct3DのDevice。
 * @param (CModel* bmarkptr) IN ボーンマークのモデルデータ。
 * @param (CMySprite* bcircleptr) IN ジョイント部分の表示のSprite。
 * @param (int selboneno) IN 選択中のボーンのID。
 * @param (int skiptopbonemark) IN 一番親からのボーンを表示しないフラグ。
 * @return 成功したら０。
 */
	int RenderBoneMark(ID3D11DeviceContext* pd3dImmediateContext, CModel* bmarkptr, CMySprite* bcircleptr, int selboneno, int skiptopbonemark = 0 );


	void RenderCapsuleReq(ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto);

	void RenderBoneCircleReq(ID3D11DeviceContext* pd3dImmediateContext, CBtObject* srcbto, CMySprite* bcircleptr);

/**
 * @fn
 * GetModelBound
 * @breaf バウンダリーデータを取得する。
 * @param (MODELBOUND* dstb) OUT バウンダリーデータ（モデルを囲い込む最小範囲データ）がセットされる。
 * @return 成功したら０。
 */
	int GetModelBound( MODELBOUND* dstb );

/**
 * @fn
 * MakeObjectName
 * @breaf オブジェクトの名前からCMQOObjectを検索するためのデータをセットする。
 * @return 成功したら０。
 */
	int MakeObjectName();

/**
 * @fn
 * MakePolyMesh3
 * @breaf メタセコイアから読み込んだデータの表示用データを作成する。
 * @return 成功したら０。
 */
	int MakePolyMesh3();

/**
 * @fn
 * MakePolyMesh4
 * @breaf FBXから読み込んだデータの表示用データを作成する。
 * @return 成功したら０。
 */
	int MakePolyMesh4();

/**
 * @fn
 * MakeExtLine
 * @breaf メタセコイアやFBXから読み込んだデータの線分表示用のデータを作成する。
 * @return 成功したら０。
 */
	int MakeExtLine();
	
/**
 * @fn
 * MakeDispObj
 * @breaf DirectXの描画用バッファのラッパデータを作成する。
 * @return 成功したら０。
 */	
	int MakeDispObj();

/**
 * @fn
 * UpdateMatrix
 * @breaf アニメーションデータを適用する。現在の時間の姿勢をセットする。モーフアニメがあればモーフアニメも適用する。
 * @param (ChaMatrix* wmat) IN ワールド変換行列。
 * @param (ChaMatrix* vpmat) IN View * Projection変換行列。
 * @return 成功したら０。
 */
	int UpdateMatrix( ChaMatrix* wmat, ChaMatrix* vpmat );
	int HierarchyRouteUpdateMatrix(CBone* srcbone, ChaMatrix* wmat, ChaMatrix* vpmat);
	int UpdateLimitedWM(int srcmotid, double srcframe);
	int ClearLimitedWM(int srcmotid, double srcframe);

/**
 * @fn
 * SetShaderConst
 * @breaf シェーダーの定数をセットする。アニメーション、ライト、テクスチャなどのシェーダー定数をDirect3Dの描画命令を呼び出す前にセットしておく。
 * @param (CMQOObject* srcobj) IN 描画対象のCMQOObject。
 * @param (int btflag = 0) IN bulletのシミュレーション中かどうかのフラグ。
 * @return 成功したら０。
 */
	int SetShaderConst( CMQOObject* srcobj, int btflag = 0 );

/**
 * @fn
 * FillTimeLine
 * @breaf タイムラインにボーンの分、行を追加する。
 * @param (OrgWinGUI::OWP_Timeline& timeline) OUT タイムライン。
 * @param (map<int, int>& lineno2boneno) OUT タイムラインの行番号からボーンIDを検索するためのmap。
 * @param (map<int, int>& boneno2lineno) OUT ボーンIDからタイムラインの行番号を検索するためのmap。
 * @return 成功したら０。
 */
	int FillTimeLine( OrgWinGUI::OWP_Timeline& timeline, map<int, int>& lineno2boneno, map<int, int>& boneno2lineno );

/**
 * @fn
 * AddMotion
 * @breaf アニメーション保持用のエントリーを作成する。
 * @param (char* srcname) IN アニメーションの名前。
 * @param (WCHAR* wfilename) IN アニメーションファイル書き出し時のための名前。
 * @param (double srcleng) IN アニメーションの長さ。
 * @param (int* dstid) OUT アニメーションの選択に使用するIDがセットされる。
 * @return 成功したら０。
 */
	int AddMotion( char* srcname, WCHAR* wfilename, double srcleng, int* dstid );

/**
 * @fn
 * SetCurrentMotion
 * @breaf 現在のモーションIDを指定する。
 * @param (int srcmotid) IN モーションID。
 * @return 成功したら０。
 */
	int SetCurrentMotion( int srcmotid );

/**
 * @fn
 * SetMotionFrame
 * @breaf 再生するモーションのフレームを指定する。
 * @param (double srcframe) IN フレーム（時間）を指定する。
 * @return 成功したら０。
 */
	int SetMotionFrame( double srcframe );


/**
 * @fn
 * GetMotionFrame
 * @breaf 現在のモーションの再生フレームを取得する。
 * @param (double* dstframe) OUT 再生フレームがセットされる。
 * @return 成功したら０。
 */
	int GetMotionFrame( double* dstframe );

/**
 * @fn
 * SetMotionSpeed
 * @breaf モーションの再生スピード(60fpsに対しての倍率)を指定する。
 * @param (double srcspeed) IN 再生スピード。
 * @return 成功したら０。
 */
	int SetMotionSpeed( double srcspeed );

/**
 * @fn
 * GetMotionSpeed
 * @breaf モーションの再生スピードを取得する。
 * @param (double* dstspeed) OUT 再生スピードがセットされる。
 * @return 成功したら０。
 */
	int GetMotionSpeed( double* dstspeed );

/**
 * @fn
 * DeleteMotion
 * @breaf モーションを削除する。
 * @param (int motid) IN 削除するモーションのID。
 * @return 成功したら０。
 * @detail 姿勢のキーだけでなく、モーションのエントリーも削除する。
 */
	int DeleteMotion( int motid );


/**
 * @fn
 * GetSymBoneNo
 * @breaf 左右対称設定の名前のボーンのIDを取得する。
 * @param (int srcboneno) IN 元のボーンのID。
 * @param (int* dstboneno) OUT 左右対称設定のボーンのIDがセットされる。 
 * @param (int* existptr) OUT 左右対称設定のボーンが見つかれば１、無ければ０がセットされる。
 * @return 成功したら０。
 */
	int GetSymBoneNo( int srcboneno, int* dstboneno, int* existptr );


/**
 * @fn
 * PickBone
 * @breaf 指定した画面の２D座標に近いボーンを取得する。
 * @param (PICKINFO* pickinfo) INOUT 取得情報へのポインタ。
 * @return 成功したら０。
 * @detail pickinfoにウインドウのサイズとクリックした２D座標とボーンとの最大距離を指定して呼び出す。
 */
	int PickBone( PICKINFO* pickinfo );

/**
 * @fn
 * IKRotate
 * @breaf 選択ボーンの選択フレームに対してIKで姿勢を回転する。
 * @param (CEditRange* erptr) IN 選択フレーム情報を指定する。
 * @param (int srcboneno) IN 選択ボーンのIDを指定する。
 * @param (ChaVector3 targetpos) IN 選択ボーンの目標座標を指定する。
 * @param (int maxlevel) IN IK計算で何階層親までさかのぼるかを指定する。
 * @return 成功したら０。
 * @detail MameBake3Dにおいては、マニピュレータの中央の黄色をドラッグした時に呼ばれる。
 */
	int IKRotate( CEditRange* erptr, int srcboneno, ChaVector3 targetpos, int maxlevel );


	int PhysicsRot(CEditRange* erptr, int srcboneno, ChaVector3 targetpos, int maxlevel);
	int PhysicsMV(CEditRange* erptr, int srcboneno, ChaVector3 diffvec);

	int CreatePhysicsPosConstraint(CBone* srcbone);
	int DestroyPhysicsPosConstraint(CBone* srcbone);
	int CreatePhysicsPosConstraintAll();
	int DestroyPhysicsPosConstraintAll();
	int CreatePhysicsPosConstraintUpper(CBone* srcbone);
	int DestroyPhysicsPosConstraintUpper(CBone* srcbone);
	int CreatePhysicsPosConstraintLower(CBone* srcbone);
	int DestroyPhysicsPosConstraintLower(CBone* srcbone);
	
	int Mass0_All(bool setflag);
	int Mass0_Upper(bool setflag, CBone* srcbone);
	int Mass0_Lower(bool setflag, CBone* srcbone);

	int SetMass0(CBone* srcbone);
	int RestoreMass(CBone* srcbone);

	int SetKinematicTmpLower(CBone* srcbone, bool srcflag);
	
/**
 * @fn
 * IKRotateAxisDelta
 * @breaf 選択ボーンの選択フレームに対して軸指定でIK回転をする。
 * @param (CEditRange* erptr) IN 選択フレーム情報を指定する。
 * @param (int axiskind) IN 回転軸を指定する。PICK_X, PICK_Y, PICK_Zのいずれか。
 * @param (int srcboneno) IN 選択ボーンのIDを指定する。
 * @param (float delta) IN 回転角度を指定する。
 * @param (int maxlevel) IN IK計算で何階層親までさかのぼるかを指定する。
 * @param (int ikcnt) IN マウスでドラッグを開始してから何回この関数を実行したか。
 * @return 成功したら０。
 * @detail MameBake3Dにおいては、マニピュレータのリングまたは球をドラッグした時に呼ばれる。
 */
	int IKRotateAxisDelta( CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat );
	int TwistBoneAxisDelta(CEditRange* erptr, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat);


	int PhysicsRotAxisDelta(CEditRange* erptr, int axiskind, int srcboneno, float delta, int maxlevel, int ikcnt, ChaMatrix selectmat);
	int SetDofRotAxis(int srcaxiskind);


/**
 * @fn
 * RotateXDelta
 * @breaf ボーン軸に関して姿勢をねじる。
 * @param (CEditRange* erptr) IN 選択フレーム情報を指定する。
 * @param (int srcboneno) IN 選択ボーンのIDを指定する。
 * @param (float delta) IN 回転角度を指定する。
 * @return 成功したら０。
 * @detail MameBake3Dにおいては、マニピュレータのリングまたは球でボーン軸に関して回転するときに呼ばれる。
 */
	int RotateXDelta( CEditRange* erptr, int srcboneno, float delta );

/**
 * @fn
 * FKRotate
 * @breaf 選択ボーンの選択フレームに対してFKで指定した分だけ回転する。
 * @param (double srcframe) IN 選択フレーム情報を指定する。
 * @param (int srcboneno) IN 選択ボーンのIDを指定する。
 * @param (CQuaternion rotq) IN 回転を表すクォータニオン。
 * @return 成功したら０。
 */
	int FKRotate(int reqflag, CBone* bvhbone, int traflag, ChaVector3 traanim, double srcframe, int srcboneno, CQuaternion srcq, int setmatflag = 0, ChaMatrix* psetmat = 0);
	//int FKRotate(double srcframe, int srcboneno, ChaMatrix srcmat);

/**
 * @fn
 * FKBoneTra
 * @breaf 選択ボーンの選択フレームに対してFKで指定した分だけ移動する。
 * @param (CEditRange* erptr) IN 選択フレーム情報を指定する。
 * @param (int srcboneno) IN 選択ボーンのIDを指定する。
 * @param (ChaVector3 addtra) IN 移動分のベクトル。
 * @return 成功したら０。
 */
	int FKBoneTra( int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 addtra );

	int FKBoneTraAxis(int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float delta);


	int FKBoneScale(int onlyoneflag, CEditRange* erptr, int srcboneno, ChaVector3 scalevec);

	int FKBoneScaleAxis(int onlyoneflag, CEditRange* erptr, int srcboneno, int axiskind, float scaleval);


/**
 * @fn
 * CollisionNoBoneObj_Mouse
 * @breaf 画面の２Dの位置にオブジェクトがあるかどうか調べる。
 * @param (PICKINFO* pickinfo) INOUT 画面の２D座標と最大距離を指定する。
 * @param (char* objnameptr) IN 調べたいオブジェクトの名前を指定する。
 * @return ２D位置にオブジェクトがあれば１、無ければ０を返す。
 */
	int CollisionNoBoneObj_Mouse( PICKINFO* pickinfo, char* objnameptr );

/**
 * @fn
 * TransformBone
 * @breaf ボーンの座標を変換計算する。
 * @param (int winx) IN ３D表示ウインドウの幅
 * @param (int winy) IN ３D表示ウインドウの高さ
 * @param (int srcboneno) IN ボーンのID。
 * @param (ChaVector3* worldptr) OUT ワールド座標系の変換結果。
 * @param (ChaVector3* screenptr) OUT *worldptrに更にView Projectionを適用した座標。
 * @param (ChaVector3* dispptr) OUT *screenptrを-WindowSize/2から+WindowSize/2までの２D座標にしたもの。
 * @return 成功したら０。
 * @detail CBoneのUpdateMatrixが呼ばれた後で呼び出されることを想定している。(CBone::m_childworldを使用している。)
 */
	int TransformBone( int winx, int winy, int srcboneno, ChaVector3* worldptr, ChaVector3* screenptr, ChaVector3* dispptr );

/**
 * @fn
 * ChangeMotFrameLeng
 * @breaf モーションのフレーム長を変更する。
 * @param (int motid) IN モーションのID。
 * @param (double srcleng) IN 変更後のフレーム長。
 * @return 成功したら０。
 * @detail 範囲外のキーは削除される。
 */
	int ChangeMotFrameLeng( int motid, double srcleng );

/**
 * @fn
 * AdvanceTime
 * @breaf 経過時間を元に、モーションの次に再生するフレームを計算する。
 * @param (int previewflag) IN プレビュー状態を指定。
 * @param (double difftime) IN 前回描画してからの時間を指定。
 * @param (double* nextframeptr) OUT 次に描画するフレーム。
 * @param (int* endflagptr) OUT 繰り返し再生ではないモーションの最終フレームに達したかどうか。
 * @param (int* loopstartflag) 繰り返しにより最初のフレームに戻ったかどうか。
 * @param (int srcmotid) IN モーションのID。
 * @return 成功したら０。
 * @detail previewflagは停止中０、通常再生１、bullet物理シミュレーション４、bulletラグドールシミュレーション５。
 */
	int AdvanceTime(int onefps, CEditRange srcrange, int previewflag, double difftime, double* nextframeptr, int* endflagptr, int* loopstartflag, int srcmotid);

/**
 * @fn
 * MakeEnglishName
 * @breaf ファイル書き出し(主にFBXファイル)のために各種名前を英語表記にする。
 * @return 成功したら０。
 */
	int MakeEnglishName();

/**
 * @fn
 * AddDefMaterial
 * @breaf デフォルトの材質を作成する。
 * @return 成功したら０。
 * @detail メタセコイアファイルには材質が含まれていない場合がある。材質が１つもないと描画時にエラーになるのでデフォルトの材質を作成する。
 */
	int AddDefMaterial();

/**
 * @fn
 * CreateBtObject
 * @breaf bulletシミュレーション用のオブジェクトを作成する。
 * @param (int onfirstcreate) IN 一番最初の呼び出し時にだけ１を指定する。
 * @return 成功したら０。
 * @detail シミュレーション開始のたびに呼ぶ。一番最初の呼び出しだけonfirstcreateを１にする。
 */
	int CreateBtObject( int onfirstcreate );

/**
 * @fn
 * SetBtMotion
 * @breaf bulletシミュレーション結果を適用する。
 * @param (int rgdollflag) IN ラグドール時に１。
 * @param (double srcframe) IN モーションのフレーム。
 * @param (ChaMatrix* wmat) IN ワールド変換行列。
 * @param (ChaMatrix* vpmat) IN ViewProj変換行列。
 * @param (double difftime) IN 前回の描画からの時間。
 * @return 成功したら０。
 * @detail bulletシミュレーション時には、CModel::Motion2Bt-->BPWorld::clientMoveAndDisplay-->CModel::SetBtMotionという流れで呼び出す。
 */
	int SetBtMotion(CBone* srcbone, int rgdollflag, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat );

/**
 * @fn
 * SetBtImpulse
 * @breaf 設定されていたインパルスを適用する。
 * @return 成功したら０。
 */
	int SetBtImpulse();

/**
 * @fn
 * SetImp
 * @breaf ボーンに与えるインパルスを設定する。
 * @param (int srcboneno) IN ボーンのID。
 * @param (int kind) IN X成分指定時０、Y指定時１、Z指定時２。
 * @param (float srcval) IN インパルスの大きさ。
 * @return 成功したら０。
 */
	int SetImp( int srcboneno, int kind, float srcval );

/**
 * @fn
 * SetDispFlag
 * @breaf 表示するかどうかのフラグを設定する。
 * @param (char* objname) IN オブジェクトの名前を指定する。
 * @param (int flag) IN 表示するとき１、しないとき０。
 * @return 成功したら０。
 * @detail 表示するとき１、しないとき０。
 */
	int SetDispFlag( char* objname, int flag );

/**
 * @fn
 * FindBtObject
 * @breaf 子供ジョイントの名前に対応するCBtObjectを検索する。
 * @param (int srcboneno) IN 子供ジョイントの名前。
 * @return CBtObject* 検索結果。
 */
	CBtObject* FindBtObject( int srcboneno );


	int FillUpEmptyMotion(int motid);


	int SetAllSphrateData(int gid, int rgdindex, float srcval);
	int SetAllBoxzrateData(int gid, int rgdindex, float srcval);
	int SetAllSkipflagData(int gid, int rgdindex, bool srcval);
	int SetAllForbidrotData(int gid, int rgdindex, bool srcval);
	int SetAllColtypeData(int gid, int rgdindex, int srcval);
	int SetAllBtforceData(int rgdindex, bool srcval);
	int SetAllDampAnimData( int gid, int rgdindex, float valL, float valA );
	int SetAllBtgData( int gid, int reindex, float btg );
	int SetAllImpulseData( int gid, float impx, float impy, float impz );
	int SetAllKData( int gid, int reindex, int srclk, int srcak, float srccuslk, float srccusak );
	int SetAllRestData( int gid, int reindex, float rest, float fric );
	int SetAllMassData( int gid, int reindex, float srcmass );
	int SetAllMassDataByBoneLeng(int gid, int reindex, float srcmass);

	int SetAllDmpData( int gid, int reindex, float ldmp, float admp );
	int EnableAllRigidElem(int srcrgdindex);
	int DisableAllRigidElem(int srcrgdindex);

	int SetColTypeAll(int reindex, int srctype);

	int Motion2Bt( int firstflag, double nextframe, ChaMatrix* mW, ChaMatrix* mVP, int selectboneno );
	int SetRagdollKinFlag(int selectbone, int physicsmvkind = 0);
	int SetKinematicFlag();
	int CreateRigidElem();
	int SetCurrentRigidElem( int curindex );
	REINFO GetCurrentRigidElemInfo(int* retindex);
	void CreateRigidElemReq( CBone* curbone, int reflag, string rename, int impflag, string impname );
	int SetBtEquilibriumPoint();
	int SetBtEquilibriumPointReq( CBtObject* srcbto );
	void SetDofRotAxisReq(CBtObject* srcbto, int srcaxiskind);


	int MultDispObj( ChaVector3 srcmult, ChaVector3 srctra );
	MOTINFO* GetRgdMorphInfo();

	int SetColiIDtoGroup( CRigidElem* curre );
	int ResetBt();

	int InitUndoMotion( int saveflag );
	int SaveUndoMotion( int curboneno, int curbaseno, CEditRange* srcer, double srcapplyrate );
	int RollBackUndoMotion( int redoflag, int* curboneno, int* curbaseno, double* dststartframe, double* dstendframe, double* dstapplyframe );

	int AddBoneMotMark( OrgWinGUI::OWP_Timeline* owpTimeline, int curboneno, int curlineno, double startframe, double endframe, int flag );

	int MotionID2Index( int motid );
	FbxAnimLayer* GetAnimLayer( int motid );
	float GetTargetWeight( int motid, double dframe, double timescale, CMQOObject* baseobj, std::string targetname );


	int SetFirstFrameBonePos(HINFO* phinfo);

	int RecalcBoneAxisX(CBone* srcbone);

	int CalcBoneEul(int srcmotid);
	void CalcBoneEulReq(CBone* curbone, int srcmotid, double srcframe);

	int RigControl(int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig);
	int PhysicsRigControl(int depthcnt, CEditRange* erptr, int srcboneno, int uvno, float srcdelta, CUSTOMRIG ikcustomrig);

	int DbgDump();

	CBone* GetSymPosBone(CBone* srcbone);

	int InterpolateBetweenSelection(double srcstartframe, double srcendframe);

	int BulletSimulationStop();
	int BulletSimulationStart();
	int ApplyBtToMotion();
	void CalcBtAxismat(int onfirstcreate);
	void CalcRigidElem();
	void SetBtKinFlagReq(CBtObject* srcbto, int oncreateflag);
	void SetKinematicFlagReq(CBtObject* srcbto);

	void ResetMotionCache();

	void EnableRotChildren(CBone* srcbone, bool srcflag);
	void EnableRotChildrenReq(CBone* srcbone, bool srcflag);

	void DestroyScene();


	void PhysIKRec(double srcrectime);
	void PhysIKRecReq(CBone* srcbone, double srcrectime);
	void ApplyPhysIkRec();
	void ApplyPhysIkRecReq(CBone* srcbone, double srcframe, double srcrectime);

	int ResetAngleLimit(int srcval);
	int AngleLimitReplace180to170();
	int AdditiveCurrentToAngleLimit();

	bool ChkBoneHasRig(CBone* srcbone);

private:
	int InitParams();
	int DestroyObjs();
	int CreateMaterialTexture(ID3D11DeviceContext* pd3dImmediateContext);


	int AddModelBound( MODELBOUND* mb, MODELBOUND* addmb );

	int DestroyMaterial();
	int DestroyObject();
	int DestroyAncObj();
	int DestroyAllMotionInfo();

	//void MakeBoneReq( CBone* parentbone, CMQOFace* curface, ChaVector3* pointptr, int broflag, int* errcntptr );

	int SetShapeNoReq( CMQOFace** ppface, int facenum, int searchp, int shapeno, int* setfacenum);
	int SetFaceOfShape( CMQOFace** ppface, int facenum, int shapeno, CMQOFace** ppface2, int setfacenum );

	int CreateBtConstraint();
	void CreateBtConstraintReq( CBtObject* curbto );
	//void CreateBtConstraintReq(CBone* curbone);

	void CreateIndexedMotionPointReq(CBone* srcbone, int srcmotid, double srcanimleng);

	//void UpdateMatrixReq( int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat, 
	//	ChaMatrix* parmat, CQuaternion* parq, CBone* srcbone, int broflag );

	void FillTimelineReq( OrgWinGUI::OWP_Timeline& timeline, CBone* curbone, int* linenoptr, 
		map<int, int>& lineno2boneno, map<int, int>& boneno2lineno, int broflag );

	void SetSelectFlagReq( CBone* boneptr, int broflag );
	int CalcMouseLocalRay( PICKINFO* pickinfo, ChaVector3* startptr, ChaVector3* dirptr );
	CBone* GetCalcRootBone( CBone* firstbone, int maxlevel );
	//void CalcXTransformMatrixReq( CBone* srcbone, ChaMatrix parenttra, float mult );


	//int InitFBXManager( FbxManager** ppSdkManager, FbxImporter** ppImporter, FbxScene** ppScene, char* utfname );
	int CreateFBXMeshReq( FbxNode* pNode );
	int CreateFBXShape( FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep );

	CMQOObject* GetFBXMesh(FbxNode* pNode, FbxNodeAttribute *pAttrib );
	int GetFBXShape(FbxMesh* pMesh, CMQOObject* curobj, FbxAnimLayer* panimlayer, double animleng, FbxTime starttime, FbxTime timestep );
	//int ComputeShapeDeformation(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );
	//int ComputeShapeDeformation2(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj, char* takename );

	int SetMQOMaterial( CMQOMaterial* newmqomat, FbxSurfaceMaterial* material );
	void CreateFBXBoneReq(FbxScene* pScene, FbxNode* pNode, FbxNode* parnode );
	void CreateExtendBoneReq(CBone* srcbone);
	int GetFBXBone(FbxScene* pScene, FbxNodeAttribute::EType type, FbxNodeAttribute *pAttrib, FbxNode* curnode, FbxNode* parnode );


	int CreateFBXAnim( FbxScene* pScene, FbxNode* prootnode, BOOL motioncachebatchflag );
	void CreateFBXAnimReq( int animno, FbxScene* pScene, FbxPose* pPose, FbxNode* pNode, int motid, double animleng );
	int GetFBXAnim( int animno, FbxScene* pScene, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute *pAttrib, int motid, double animleng );
	void CreateFBXSkinReq( FbxNode* pNode );
	int GetFBXSkin( FbxNodeAttribute *pAttrib, FbxNode* pNode );

	void InitMpScaleReq(CBone* curbone, int srcmotid, double srcframe);
	int CorrectFbxScaleAnim(int animno, FbxScene* pScene, FbxNode* pNode, FbxPose* pPose, FbxNodeAttribute* pAttrib, int motid, double animleng);


	int DestroyFBXSDK();
	int GetShapeWeight(FbxNode* pNode, FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* curobj );
	float GetFbxTargetWeight(FbxNode* pbaseNode, FbxMesh* pbaseMesh, std::string targetname, FbxTime& pTime, FbxAnimLayer * pAnimLayer, CMQOObject* baseobj );
	
	int SetDefaultBonePos();


	//SetDefaultBonePosReqは関数名にFbxを付けて FbxFile.h, FbxFile.cppに移動になりました。リンクエラーの関係で。
	//void SetDefaultBonePosReq( CBone* curbone, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix pParentGlobalPosition );



	void FillUpEmptyKeyReq( int motid, double animleng, CBone* curbone, CBone* parentbone );

	int SetMaterialName();

	int DestroyBtObject();
	void DestroyBtObjectReq( CBtObject* curbt );
	void CreateBtObjectReq( CBtObject* parbt, CBone* parentbone, CBone* curbone );

	void CalcBtAxismatReq( CBone* curbone, int onfirstcreate );
	void CalcRigidElemReq(CBone* curbone);
	void SetBtMotionReq( CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat );
	void SetBtMotionPostLowerReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat, int kinematicadjustflag);
	void SetBtMotionPostUpperReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void SetBtMotionMass0BottomUpReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void FindAndSetKinematicReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);//Kinematicとそうでないところの境目を探してみつかったらLowerReqで親行列をセットする。
	void SetBtMotionKinematicLowerReq(CBtObject* curbto, ChaMatrix oldparentmat, ChaMatrix newparentmat);
	void AdjustBtMatToParent(CBone* curbone, CBone* childbone, int adjustrot = 0);
	void AdjustBtMatToCurrent(CBone* curbone);
	void AdjustBtMatToChild(CBone* curbone, CBone* childbone, int adjustrot = 0);
	void InitBtMatTraAnimReq(CBtObject* curbto);

	void BtMat2BtObjReq(CBtObject* curbto, ChaMatrix* wmat, ChaMatrix* vpmat);
	void RecalcConstraintFrameABReq(CBtObject* curbto);

	void FindBtObjectReq( CBtObject* srcbto, int srcboneno, CBtObject** ppret );
	void SetImpulseDataReq( int gid, CBone* srcbone, ChaVector3 srcimp );
	void SetBtImpulseReq( CBone* srcbone );
	
	void SetSphrateDataReq(int gid, int rgdindex, CBone* srcbone, float srcval);
	void SetBoxzrateDataReq(int gid, int rgdindex, CBone* srcbone, float srcval);
	void SetSkipflagDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval);
	void SetForbidrotDataReq(int gid, int rgdindex, CBone* srcbone, bool srcval);
	void SetColtypeDataReq(int gid, int rgdindex, CBone* srcbone, int srcval);
	void SetBtforceDataReq(int rgdindex, CBone* srcbone, bool srcval);
	void SetDampAnimDataReq( int gid, int rgdindex, CBone* srcbone, float valL, float valA );
	void SetBtgDataReq( int gid, int reindex, CBone* srcbone, float btg );
	void SetKDataReq( int gid, int reindex, CBone* srcbone, int srclk, int srcak, float srccuslk, float srccusak );
	void SetRestDataReq( int gid, int reindex, CBone* srcbone, float rest, float fric );
	void SetDmpDataReq( int gid, int reindex, CBone* srcbone, float ldmp, float admp );
	void SetMassDataReq( int gid, int reindex, CBone* srcbone, float srcmass );
	void SetMassDataByBoneLengReq(int gid, int reindex, CBone* srcbone, float srcmass);
	void SetColTypeReq(int reindex, CBone* srcbone, int srctype);

	//void SetBtKinFlagReq(CBtObject* srcbto, int oncreateflag);
	void Motion2BtReq( CBtObject* srcbto );
	//void SetBtGravityReq( CBtObject* srcbto );
	void SetRagdollKinFlagReq( CBtObject* srcbto, int selectbone, int physicsmvkind = 0 );
	//void CreateBtConnectReq( CBone* curbone );
	void SetColiIDReq( CBone* srcbone, CRigidElem* srcre );
	void EnableAllRigidElemReq(CBone* srcbone, int srcrgdindex);
	void DisableAllRigidElemReq(CBone* srcbone, int srcrgdindex);

	int DampAnim( MOTINFO* rgdmorphinfo );

	void ResetBtReq( CBtObject* curbto );

	int GetValidUndoID( int* rbundoid );
	int GetValidRedoID( int* rbundoid );

	int SetBefEditMat( CEditRange* erptr, CBone* curbone, int maxlevel );
	int SetBefEditMatFK( CEditRange* erptr, CBone* curbone );

	int AdjustBoneTra( CEditRange* erptr, CBone* lastpar );

	void SetFirstFrameBonePosReq(CBone* srcbone, int srcmotid, HINFO* phinfo);
	void InterpolateBetweenSelectionReq(CBone* srcbone, double srcstartframe, double srcendframe);


	int DbgDumpBoneReq(CBone* boneptr, int broflag);
	void DumpBtObjectReq(CBtObject* srcbto, int srcdepth);
	void DumpBtConstraintReq(CBtObject* srcbto, int srcdepth);
	FbxPose* GetBindPose();

	void CreatePhysicsPosConstraintReq(CBone* srcbone, int forceflag = 0);
	void DestroyPhysicsPosConstraintReq(CBone* srcbone, int forceflag = 0);
	void SetMass0Req(CBone* srcbone, bool forceflag);
	void RestoreMassReq(CBone* srcbone);
	void SetKinematicTmpLowerReq(CBone* srcbone, bool srcflag);

	void PhysicsMVReq(CBone* srcbone, ChaVector3 diffvec);
	int WithConstraint(CBone* srcbone);
	void BulletSimulationStopReq(CBtObject* srcbto);
	void BulletSimulationStartReq(CBtObject* srcbto);
	void ApplyBtToMotionReq(CBone* srcbone);

	void SetCurrentRigidElemReq(CBone* srcbone, std::string curname);

	//int GetFreeThreadIndex();
	//void WaitAllTheadOfGetFbxAnim();

	//static unsigned __stdcall ThreadFunc_MP1(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP2(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP3(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP4(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP5(void* pArguments);
	//static unsigned __stdcall ThreadFunc_MP6(void* pArguments);

public: //accesser
	FbxManager* GetFBXSDK(){
		return m_psdk;
	};
	int GetModelNo(){ return m_modelno; };
	void SetModelNo( int srcno ){ m_modelno = srcno; };

	bool GetModelDisp(){ return m_modeldisp; };
	void SetModelDisp( bool srcflag ){ m_modeldisp = srcflag; };

	float GetLoadMult(){ return m_loadmult; };
	void SetLoadMult( float srcmult ){ m_loadmult = srcmult; };

	const WCHAR* GetFileName(){ return m_filename; };
	const WCHAR* GetDirName(){ return m_dirname; };
	const WCHAR* GetModelFolder(){ return m_modelfolder; };
	const char* GetDefaultReName(){ return m_defaultrename; };
	const char* GetDefaultImpName(){ return m_defaultimpname; };

	int GetMqoObjectSize(){
		return (int)m_object.size();
	};
	CMQOObject* GetMqoObject( int srcobjno ){
		map<int, CMQOObject*>::iterator itrobj;
		itrobj = m_object.find(srcobjno);
		if (itrobj != m_object.end()){
			return itrobj->second;
		}
		else{
			return 0;
		}
		//return m_object[ srcobjno ];
	};
	std::map<int,CMQOObject*>::iterator GetMqoObjectBegin(){
		return m_object.begin();
	};
	std::map<int,CMQOObject*>::iterator GetMqoObjectEnd(){
		return m_object.end();
	};
	void SetMqoObject( int srcindex, CMQOObject* srcobj ){
		m_object[ srcindex ] = srcobj;
	};

	CBone* GetBoneByName( string srcname ){
		map<string, CBone*>::iterator itrbone;
		itrbone = m_bonename.find(srcname);
		if (itrbone != m_bonename.end()){
			return itrbone->second;
		}
		else{
			return 0;
		}
		//return m_bonename[ srcname ];
	};

	CBone* GetBoneByWName(WCHAR* srcname){
		if (!srcname){
			return 0;
		}
		size_t namelen = wcslen(srcname);
		if ((namelen > 0) && (namelen < 256)){
			char mbname[1024] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, srcname, -1, mbname, 1024, NULL, NULL);
			return GetBoneByName(mbname);
		}
		else{
			return 0;
		}
	};

	int GetBoneListSize(){
		return (int)m_bonelist.size();
	};
	std::map<int,CBone*>::iterator GetBoneListBegin(){
		return m_bonelist.begin();
	};
	std::map<int,CBone*>::iterator GetBoneListEnd(){
		return m_bonelist.end();
	};
	CBone* GetBoneByID( int srcid ){
		map<int, CBone*>::iterator itrbone;
		itrbone = m_bonelist.find(srcid);
		if (itrbone != m_bonelist.end()){
			return itrbone->second;
		}
		else{
			return 0;
		}
		//return m_bonelist[ srcid ];
	};
	CBone* GetBoneByZeroBaseIndex(int srcindex) {
		int bonenum = (int)m_bonelist.size();
		if ((srcindex < 0) || (srcindex >= bonenum)) {
			return 0;
		}
		std::map<int, CBone*>::iterator itrbone;
		int curindex = 0;
		itrbone = m_bonelist.begin();
		while (curindex < srcindex) {
			itrbone++;
			if (itrbone == m_bonelist.end()) {
				return 0;
			}
			curindex++;
		}
		if (itrbone != m_bonelist.end()) {
			return itrbone->second;
		}
		else {
			return 0;
		}
	};



	CBone* GetTopBone(){
		return m_topbone;
	};

	CBtObject* GetTopBt(){
		return m_topbt;
	};

	float GetBtGScale(int srcindex){
		if ((srcindex >= 0) && (srcindex < (int)m_rigideleminfo.size())){
			return m_rigideleminfo[srcindex].btgscale;
		}
		else{
			_ASSERT(0);
			return 0.0f;
		}
	};
	void SetBtGScale( float srcval, int srcindex ){
		if ((srcindex >= 0) && (srcindex < (int)m_rigideleminfo.size())){
			m_rigideleminfo[srcindex].btgscale = srcval;
		}
		else{
			_ASSERT(0);
		}
	};

	int GetMotInfoSize(){
		return (int)m_motinfo.size();
	};
	MOTINFO* GetMotInfo( int srcid ){//motidは1から
		//return m_motinfo[srcid - 1];

		//DeleteMotion時に要素をeraseするのでid - 1が配列のインデックスになるとは限らない//2021/08/26
		int miindex;
		miindex = MotionID2Index(srcid);
		if (miindex >= 0) {
			return m_motinfo[miindex];
		}
		else {
			return 0;
		}
	};
	std::map<int,MOTINFO*>::iterator GetMotInfoBegin(){
		return m_motinfo.begin();
	};
	std::map<int,MOTINFO*>::iterator GetMotInfoEnd(){
		return m_motinfo.end();
	};

	MOTINFO* GetCurMotInfo(){
		return m_curmotinfo;
	};
	void SetCurMotInfo( MOTINFO* srcinfo ){
		m_curmotinfo = srcinfo;
	};

	CRigidElem* GetRigidElem(int srcboneno);
	CRigidElem* GetRgdRigidElem(int srcrgdindex, int srcboneno);

	int GetRigidElemInfoSize(){
		return (int)m_rigideleminfo.size();
	};
	REINFO GetRigidElemInfo( int srcindex ){
		return m_rigideleminfo[ srcindex ];
	};
	void PushBackRigidElemInfo( REINFO srcinfo ){
		m_rigideleminfo.push_back( srcinfo );
	};
	void SetRigidElemInfo( int srcindex, REINFO srcinfo ){
		m_rigideleminfo[ srcindex ] = srcinfo;
	};

	int GetImpInfoSize(){
		return (int)m_impinfo.size();
	};
	string GetImpInfo( int srcindex ){
		return m_impinfo[ srcindex ];
	};
	string GetCurImpName(){
		string curimpname = m_impinfo[m_curimpindex];
		return curimpname;
	};
	void PushBackImpInfo( std::string srcname )
	{
		m_impinfo.push_back( srcname );
	};


	int GetCurReIndex(){
		return m_curreindex;
	};

	int GetCurImpIndex(){
		return m_curimpindex;
	};
	void SetCurImpIndex( int srcindex ){
		m_curimpindex = srcindex;
	};


	int GetRgdIndex(){
		return m_rgdindex;
	};
	void SetRgdIndex( int srcindex ){
		m_rgdindex = srcindex;
	};

	int GetRgdMorphIndex(){
		return m_rgdmorphid;
	};
	void SetRgdMorphIndex( int srcindex ){
		m_rgdmorphid = srcindex;
	};

	float GetTmpMotSpeed(){
		return m_tmpmotspeed;
	};

	int GetMQOMaterialSize(){
		return (int)m_material.size();
	};
	CMQOMaterial* GetMQOMaterial( int srcindex ){
		return m_material[ srcindex ];
	};
	std::map<int,CMQOMaterial*>::iterator GetMQOMaterialBegin(){
		return m_material.begin();
	};
	std::map<int,CMQOMaterial*>::iterator GetMQOMaterialEnd(){
		return m_material.end();
	};

	void SetMQOMaterial( int srcindex, CMQOMaterial* srcmat ){
		m_material[ srcindex ] = srcmat;
	};

	CMQOMaterial* GetMQOMaterialByName( string srcname ){
		return m_materialname[ srcname ];
	};
	std::map<std::string,CMQOMaterial*>::iterator GetMQOMaterialNameBegin(){
		return m_materialname.begin();
	};
	std::map<std::string,CMQOMaterial*>::iterator GetMQOMaterialNameEnd(){
		return m_materialname.end();
	};


	FbxScene* GetScene(){
		return m_pscene;
	};

	FbxString* GetAnimStackName( int srcindex ){
		return mAnimStackNameArray[ srcindex ];
	};

	int GetBtCnt(){
		return m_btcnt;
	};
	void PlusPlusBtCnt(){
		m_btcnt++;
	};
	void ZeroBtCnt(){
		m_btcnt = 0;
	}

	void SetBtWorld( btDynamicsWorld* srcworld ){
		m_btWorld = srcworld;
	};

	int GetOldAxisFlagAtLoading()
	{
		return m_oldaxis_atloading;
	};
	void SetOldAxisFlagAtLoading(int srcflag)
	{
		m_oldaxis_atloading = srcflag;
	};

	CMQOObject* GetObjectByName(string strname)
	{
		map<string, CMQOObject*>::iterator itrobjname;
		itrobjname = m_objectname.find(strname);
		if (itrobjname == m_objectname.end()){
			return 0;
		}
		else{
			return itrobjname->second;
		}
	};

	bool GetCreateBtFlag()
	{
		return m_createbtflag;
	};
	void SetCreateBtFlag(bool srcflag)
	{
		m_createbtflag = srcflag;
	};
	bool GetLoadedFlag()
	{
		return m_loadedflag;
	};
	void SetLoadedFlag(bool srcflag)
	{
		m_loadedflag = srcflag;
	};
	int GetInitAxisMatX(){
		return m_initaxismatx;
	};
	void SetInitAxisMatX(int srcval){
		m_initaxismatx = srcval;
	};

	void SetModelPosition(ChaVector3 srcpos){
		m_modelposition = srcpos;
	};
	ChaVector3 GetModelPosition(){
		return m_modelposition;
	};
	void SetWorldMatFromCamera(ChaMatrix srcmat){
		m_worldmat = srcmat;
		m_worldmat._41 = m_modelposition.x;
		m_worldmat._42 = m_modelposition.y;
		m_worldmat._43 = m_modelposition.z;
	};
	ChaMatrix GetWorldMat()
	{
		return m_worldmat;
	};

	int GetFbxComment(char* dstcomment, int dstlen)
	{
		if (!dstcomment) {
			_ASSERT(0);
			return 1;
		}
		if ((dstlen < 0) || (dstlen > MAX_PATH)) {
			_ASSERT(0);
			return 1;
		}
		strcpy_s(dstcomment, dstlen, m_fbxcomment.Buffer());
		return 0;
	}

	FbxNode* GetBoneNode(CBone* curbone) {
		if (!curbone) {
			return 0;
		}
		else {
			FbxNode* pNode = m_bone2node[curbone];
			return pNode;
		}
	}

	int GetHasBindPose() {
		return m_hasbindpose;
	}
	void SetHasBindPose(int srcval) {
		m_hasbindpose = srcval;
	}

public:
	//CRITICAL_SECTION m_CritSection_GetGP;
	//FUNCMPPARAMS* m_armpparams[6];
	//HANDLE m_arhthread[6];

	bool m_loadedflag;//初期の読み込み処理が終了したらtrue;
	bool m_modeldisp;//表示するかどうか
	bool m_createbtflag;//CreateBtObjectを読んだことがあればtrue。

private:
	
	int m_physicsikcnt;
	int m_initaxismatx;
	int m_modelno;//モデルのID
	float m_loadmult;//表示倍率
	int m_oldaxis_atloading;//FBX読み込み時に旧データ互換チェックボックスにチェックをしていたかどうか。
	int m_hasbindpose;


	WCHAR m_filename[MAX_PATH];//モデルファイル名、フルパス。
	WCHAR m_dirname[MAX_PATH];//モデルファイルのディレクトリのパス。ファイル名の手前まで。末尾に\\無し。
	WCHAR m_modelfolder[MAX_PATH];//モデルファイルが入っているディレクトリの名前(パスではない)。*.chaファイルがあるディレクトリの中のFBXが入っているディレクトリの名前。
	char m_defaultrename[MAX_PATH];//RigidEelemファイル*.refのデフォルトのファイル名。
	char m_defaultimpname[MAX_PATH];//インパルスファイル*.impのデフォルトのファイル名。

	map<int, CMQOObject*> m_object;//オブジェクト。別の言葉でいうと３Dモデルにおける名前が付けられているパーツや部品。
	map<int, CBone*> m_bonelist;//ボーンをボーンIDから検索できるようにしたmap。
	map<string, CBone*> m_bonename;//ボーンを名前から検索できるようにしたmap。

	CBone* m_topbone;//一番親のボーン。
	CBtObject* m_topbt;//一番親のbullet剛体オブジェクト。
	//float m_btgscale;//bulletの重力に掛け算するスケール。--> m_rigideleminfoのbtgscaleに移動。

	map<int, MOTINFO*> m_motinfo;//モーションのプロパティをモーションIDから検索できるようにしたmap。
	MOTINFO* m_curmotinfo;//m_motinfoの中の現在再生中のMOTINFOへのポインタ。

	vector<REINFO> m_rigideleminfo;//剛体設定ファイル*.refのファイル情報のvector。
	vector<string> m_impinfo;//インパルスファイル*.impのファイル名のvector。

	int m_curreindex;//現在有効になっている剛体ファイルのインデックス。剛体ファイルは複数読み込むことが出来て、カレントを設定できる。
	int m_curimpindex;//現在有効になっているインパルスファイルのインデックス。インパルスファイルは複数読み込むことが出来、カレントを設定できる。
	int m_rgdindex;//ラグドールシミュレーション用の剛体設定ファイルのインデックス。
	int m_rgdmorphid;//ラグドール時に使用するアニメーションへのインデックス。ラグドール時にはモーフアニメを通常アニメ時と変えることが多いためこの設定が存在する。例えばやられ顔のモーフアニメを再生したりする。

	float m_tmpmotspeed;//モーション再生倍率の一時保存用。

	//polymesh3用のマテリアル。polymesh4はmqoobjectのmqomaterialを使用する。
	map<int, CMQOMaterial*> m_material;
	map<string, CMQOMaterial*> m_materialname;

	FbxScene* m_pscene;//FBX SDKのシーンへのポインタ。CModel内でアロケート。
	FbxArray<FbxString*> mAnimStackNameArray;//アニメーション名を保存するFBX形式のデータ。

	btDynamicsWorld* m_btWorld;//bulletのシミュレーション用オブジェクトへのポインタ。
	int m_btcnt;//bulletのシミュレーションをしたフレーム数を記録する。初回のシミュレーション時に異なる処理をするために必要である。


//以下、privateかつローカルからしか参照しないデータだからアクセッサーも無し。
	ID3D11Device* m_pdev;//外部メモリ。Direct3DのDevice。
	FbxManager* m_psdk;//外部メモリ。FBX SDKのマネージャ。
	FbxImporter* m_pimporter;//FBX SDKのインポーター。CModel内でアロケート。
	FbxString m_fbxcomment;
	WCHAR m_fbxfullname[MAX_PATH];
	bool m_useegpfile;

	ChaMatrix m_matWorld;//ワールド変換行列。
	ChaMatrix m_matVP;//View * Projection 変換行列。

	map<CMQOObject*, FBXOBJ*> m_fbxobj;//FbxNodeのラッパークラスとCMQOObjectとのmap。
	map<string, CMQOObject*> m_objectname;//CMQOObjectを名前で検索するためのmap。


	int (*m_tlFunc)( int srcmotid );//Main.cppに実態があるタイムライン初期化用の関数へのポインタ。データ読み込み時にCModelから初期化関数を呼ぶ。

	mutable FbxTime mStart, mStop, mFrameTime, mFrameTime2;//Fbxでの時間表現。アニメーションの時間(フレーム)指定などに使用。

	map<CBone*, FbxNode*> m_bone2node;//ボーンとFbxNodeの対応表。FbxNodeはFBXファイル内のオブジェクトの階層をたどる際などに利用する。
	map<CBone*,CBone*> m_rigidbone;//剛体１つはボーン１つに対応している。ボーンは親のジョイントと子供のジョイントからなる。ジョイントとボーンは同じように呼ぶことがある。剛体の親ボーンを子供ボーンからけんさくすることに使用する。

	int m_texpool;//Direct3Dのテクスチャ作成プール（場所）。システムメモリかビデオメモリかマネージドか選ぶ。通常は0でビデオメモリを指定する。
	ChaVector3 m_ikrotaxis;//IK, FKでボーン回転するための回転軸を一時的に保存する。
	CUndoMotion m_undomotion[ UNDOMAX ];//アンドゥー機能のためのCUndoMotionの配列。CUndoMotionの１つのインスタンスは１フレーム分のモーションを保存する。
	int m_undoid;//アンドゥー用データをリングバッファで使用するための現在位置へのインデックス。

	ChaMatrix m_worldmat;
	ChaVector3 m_modelposition;

	std::vector<PHYSIKREC> m_physikrec0;
	std::vector<PHYSIKREC> m_physikrec;
	double m_phyikrectime;

};

#endif