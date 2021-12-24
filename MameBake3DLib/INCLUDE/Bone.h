#ifndef BONEH
#define BONEH

//class ID3D11Device;


//#include <d3dx9.h>
//#include <quaternion.h>

#include <fbxsdk.h>

#include <map>
#include <string>

#include <Coef.h>

#include <orgwindow.h>

#include <ChaVecCalc.h>

#include <MotionPoint.h>


#define BONEPOOLBLKLEN	256


class CMQOFace;
class CMotionPoint;
class CRigidElem;
class CBtObject;
class CModel;

class CBone
{
public:

	ChaVector3 m_btparentpos;//Motion2Bt時のボーンの位置(剛体行列計算用)
	ChaVector3 m_btchildpos;//Motion2Bt時のボーンの位置(剛体行列計算用)
	ChaMatrix m_btdiffmat;//Motion2Bt時のbtmatの変化分(剛体行列計算用)


	CBone() {
		InitializeCriticalSection(&m_CritSection_AddMP);
		InitializeCriticalSection(&m_CritSection_GetBefNext);
		InitParams();
	};

/**
 * @fn
 * CBone
 * @breaf CBoneのコンストラクタ。
 * @param (CModel* parmodel) IN 親ボーンへのポインタを指定する。
 * @return なし。
 */
	CBone( CModel* parmodel );

/**
 * @fn
 * ~CBone
 * @breaf CBoneのデストラクタ。
 * @return なし。
 */
	~CBone();

/**
 * @fn 
 * SetName
 * @breaf ボーンの名前を設定する。
 * @param (char* srcname) IN 設定したいボーンの名前を指定する。
 * @return 成功したら０。
 * @detail charのボーンの名前はWCHARに変換されてボーンのUNICODE名もセットされる。
 */
	int SetName( char* srcname );

/**
 * @fn 
 * AddChild
 * @breaf ボーンの階層構造を設定する。
 * @param (CBone* childptr) IN 子供にするCBoneのポインタを指定する。
 * @return 成功したら０。
 * @detail 子供を指定することで階層を設定する。子供だけでなく弟や子供の親の設定もする。
 */
	int AddChild( CBone* childptr );

/**
 * @fn 
 * UpdateMatrix
 * @breaf ボーンの現在の姿勢を計算する。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN モーションのフレーム（時間）を指定する。
 * @param (ChaMatrix* wmat) IN ワールド座標系の変換行列を指定する。
 * @param (ChaMatrix* vpmat) IN カメラプロジェクション座標系の変換行列を指定する。
 * @return 成功したら０。
 * @detail 指定モーションの指定時間の姿勢を計算する。グローバルな姿勢の計算である。
 */
	int UpdateMatrix( int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat );

	int UpdateMatrixFromEul(int srcmotid, double srcframe, ChaVector3 neweul, ChaMatrix* wmat, ChaMatrix* vpmat);

	int UpdateLimitedWorldMat(int srcmotid, double srcframe);

	
/**
 * @fn
 * AddMotionPoint
 * @breaf 指定モーションの指定時間にMotionPointを追加する。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN モーションのフレーム（時間）を指定する。
 * @param (int* existptr) OUT 追加する前からMotionPointが存在していたら１がセットされる、それ以外の場合は０がセットされる。
 * @return 成功したら追加したCMotionPointのポインタ、失敗時は０。
 * @detail 追加したMotionPointの姿勢はIdentity状態である。
 */
	CMotionPoint* AddMotionPoint( int srcmotid, double srcframe, int* existptr );


/**
 * @fn
 * CalcFBXMotion
 * @breaf 指定モーションの指定時間のポーズ(CMotionPoint)を計算する。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN モーションのフレーム(時間)を指定する。
 * @param (CMotionPoint* dstmpptr) OUT 計算した姿勢がセットされる。
 * @param (int* existptr) OUT 指定時間ちょうどのデータがあれば１、なければ０。無い場合は前後から計算する。
 * @return 成功したら０。
 */
	int CalcFBXMotion( int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr );
	int GetCalclatedLimitedWM(int srcmotid, double srcframe, ChaMatrix* plimitedworldmat, CMotionPoint** pporgbefmp = 0);

/**
 * @fn
 * CalcFBXFrame
 * @breaf 指定時間の前後の姿勢から指定時間の姿勢を計算する。
 * @param (double srcframe) IN 指定時間。
 * @param (CMotionPoint* befptr) IN 指定時間の直前の姿勢。
 * @param (CMotionPoint* nextptr) IN 指定時間の直後の姿勢。
 * @param (int existflag) IN 指定時間ちょうどに姿勢データがあるとき１。この時ちょうどのときの姿勢はbefptrに入っている。
 * @param (CMotionPoint* dstmpptr) OUT 指定時間の姿勢が計算されてセットされる。
 * @return 成功したら０。
 * @detail 指定時間ちょうどにデータがあった場合はそれが結果になる。befptrがない場合はnextptrの姿勢が結果となる。nextptrが無い場合はbefptrの姿勢が結果となる。befとnextがある場合は線形補間する。
 */
	int CalcFBXFrame( double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr );

/**
 * @fn
 * GetBefNextMP
 * @breaf 指定モーションの指定時間の直前と直後の姿勢データを取得する。
 * @param (int srcmotid) IN モーションIDを指定する。
 * @param (double srcframe) IN モーションのフレーム（時間）を指定する。
 * @param (CMotionPoint** ppbef) OUT 直前の姿勢データ。
 * @param (CMotionPoint** ppnext) OUT 直後の姿勢データ。
 * @param (int* existptr) OUT 時間ちょうどのデータがあるときに１。
 * @return 成功したら０。
 * @detail existptrの内容が１のとき、ちょうどの時間の姿勢はppbefにセットされる。
 */
	int GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr, bool onaddmotion = false );

/**
 * @fn
 * DeleteMotion
 * @breaf 指定したモーションIDのモーションを削除する。
 * @param (int srcmotid) IN モーションのID。
 * @return 成功したら０。
 * @detail 指定したモーションの姿勢データを削除する。
 */
	int DeleteMotion( int srcmotid );

/**
 * @fn
 * DeleteMPOutOfRange
 * @breaf 指定したモーションの長さをはみ出している(時間がはみ出している)姿勢データを削除する。
 * @param (int motid) IN モーションのIDを指定する。
 * @param (double srcleng) IN モーションの長さ(時間)を指定する。
 * @return 成功したら０。
 * @detail モーションの長さを変更するときに範囲外の姿勢を削除するために呼ばれる。
 */
	int DeleteMPOutOfRange( int motid, double srcleng );

/**
 * @fn
 * CalcRigidElemParams
 * @breaf 剛体表示用のデータを剛体のパラメータに従ってスケールするための変換行列を求めてスケールする。
 * @param (CBone* childbone) IN　剛体を指定するための子供ボーン。
 * @param (int setstartflag) IN　剛体シミュレーション開始時の呼び出し時に１をセットする。
 * @return 成功したら０。
 * @detail 剛体をボーンの位置に表示するために、剛体表示用の形状をスケールするために呼ぶ。剛体はボーンの子供ジョイントと１対１で対応するため、指定にchildboneを使う。
 */
	int CalcRigidElemParams( CBone* childbone, int setstartflag );

/**
 * @fn
 * CalcAxisMat
 * @breaf ボーンの軸のための変換行列を計算する。初期状態がX軸を向いていると仮定して計算する。
 * @param (int firstflag) IN 変換開始の行列を保存したいときに１を指定する。
 * @param (float delta) IN 回転角度をしていする。現在は０でしか使用していない。
 * @return 成功したら０。
 * @detail 計算したデータは、ボーンの位置にマニピュレータを表示するための変換行列に使用する。現在はCalcAxisMatZ関数でボーンの変換行列を計算している。
 */
	//int CalcAxisMat( int firstflag, float delta );
	float CalcAxisMatX(int bindflag, CBone* childbone, ChaMatrix* dstmat, int setstartflag);


/**
 * @fn
 * CalcAxisMatZ
 * @breaf ボーンの親と子供の位置を指定して、その軸の変換行列を計算する。
 * @param (ChaVector3* srccur) IN ボーンの親の座標を指定する。
 * @param (ChaVector3* srcchil) IN ボーンの子供の座標を指定する。
 * @return 成功したら０。
 * @detail 初期状態がZ軸を向いていると仮定して計算する。ボーンの位置にマニピュレータを表示するために使用する。
 */
	int CalcAxisMatZ( ChaVector3* srccur, ChaVector3* srcchil );


/**
 * @fn
 * CreateRigidElem
 * @breaf 剛体シミュレーション用のパラメータを保持するCRigidElemを作成する。指定ボーンに関するCRigidElemを作成する。
 * @param (CBone* parentbone) IN 自分自身とここで指定する親ジョイントで定義されるボーンに関するCRigidElemを作成する。
 * @param (int reflag) IN CRigidElemを作成する場合に１、しない場合に０を指定。
 * @param (std::string rename) IN reflagが１のとき、refファイルのファイル名を指定する。
 * @param (int impflag) IN インパルスを与えるための設定を作成する場合に１、しない場合に０を指定。
 * @param (std::string impname) IN impflagが１のとき、impファイルのファイル名を指定する。
 * @return 成功したら０。
 * @detail インパルス設定データも作成する。
 */
	//int CreateRigidElem( CBone* chil, int reflag, std::string rename, int impflag, std::string impname );
	int CreateRigidElem(CBone* parentbone, int reflag, std::string rename, int impflag, std::string impname);


/**
 * @fn
 * SetCurrentRigidElem
 * @breaf カレント設定として使用するRigidElemをセットする。
 * @param (std::string curname) カレントにしたいrefファイルの名前を指定する。
 * @return 成功したら０。
 */
	int SetCurrentRigidElem( std::string curname );


/**
 * @fn
 * AddBoneTraReq
 * @breaf ボーンの姿勢を指定ベクトルだけ移動する。子供方向に再帰的に計算する。
 * @param (CMotionPoint* parmp) IN ０を指定した時は自分の姿勢を計算する。０以外の親を指定した場合は子供方向に姿勢の変更を伝えていく。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN モーションのフレーム（時間）を指定する。
 * @param (ChaVector3 srctra) IN 移動ベクトルを指定する。
 * @return 計算した姿勢を格納したCMotionPointのポインタを返すが再帰関数であることに注意。ポインタはチェインにセットされたものである。
 * @detail 想定している使い方としては、外部からの呼び出し時にはparmpを０にする。この関数内での再帰呼び出し時にparmpに親をセットする。
 */
	CMotionPoint* AddBoneTraReq(CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srctra);


	CMotionPoint* AddBoneScaleReq(CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srcscale);

/**
 * @fn
 * RotBoneQReq
 * @breaf ボーンの姿勢を指定クォータニオンの分回転する。子供方向に再帰的に計算する。
 * @param (CMotionPoint* parmp) IN ０を指定した時は自分の姿勢を計算する。０以外の親を指定した場合は子供方向に姿勢の変更を伝えていく。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN モーションのフレーム（時間）を指定する。
 * @param (CQuaternion rotq) IN 回転のためのクォータニオンを指定する。
 * @return 計算した姿勢を格納したCMotionPointのポインタを返すが再帰関数であることに注意。ポインタはチェインにセットされたものである。
 * @detail 想定している使い方としては、外部からの呼び出し時にはparmpを０にする。この関数内での再帰呼び出し時にparmpに親をセットする。
 */
	CMotionPoint* RotBoneQReq(bool infooutflag, CBone* parentbone, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, ChaVector3 traanim = ChaVector3(0.0f, 0.0f, 0.0f), int setmatflag = 0, ChaMatrix* psetmat = 0);

	//CMotionPoint* RotBoneQCurrentReq(bool infooutflag, CBone* parbone, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, ChaVector3 traanim = ChaVector3(0.0f, 0.0f, 0.0f), int setmatflag = 0, ChaMatrix* psetmat = 0);

/**

*/
	CMotionPoint* RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, ChaMatrix srcmat);

/**
 * @fn
 * PasteRotReq
 * @breaf srcframeの姿勢をdstframeの姿勢にペーストする。子供方向に再帰的に設定する。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN コピー元のフレーム（時間）を指定する。
 * @param (double dstframe) IN コピー先のフレームを指定する。
 * @return dstのCMotionPointのポインタを返す。ただし再帰的にである。
 */
	CMotionPoint* PasteRotReq( int srcmotid, double srcframe, double dstframe );


/**
 * @fn
 * SetAbsMatReq
 * @breaf firstframeの姿勢をそのままsrcframeに適用する。子供方向に再帰的に実行する。
 * @param (int broflag) IN このボーンの兄弟ボーンに対しても処理をしたい場合は１を、その他の場合は０を指定する。
 * @param (int srcmotid) IN モーションのIDを指定する。
 * @param (double srcframe) IN 姿勢を適用したいフレームを指定する。
 * @param (double firstframe) IN 編集の開始フレームを指定する。
 * @return 編集が適用されたボーンのCMotionPointのポインタが返される。ただし再帰的にである。
 * @detail この関数は絶対IK機能として呼ばれる。絶対IKと相対IKの説明はMain.cppの冒頭の説明を読むこと。
 */
	CMotionPoint* SetAbsMatReq( int broflag, int srcmotid, double srcframe, double firstframe );


/**
 * @fn
 * DeleteMotionKey
 * @breaf 指定したモーションIDのモーションキーを削除する。
 * @param (int srcmotid) IN モーションのID。
 * @return 成功したら０。
 * @detail 指定したモーションの姿勢データを削除する。
 * @detail DeleteMotionがキーを消した後にモーションのエントリーを削除するのに対し、
 * @detail DestroyMotionKeyはキーを削除した後にモーションのエントリーにNULLをセットする。
 */
	int DestroyMotionKey( int srcmotid );


/**
 * @fn
 * AddBoneMotMark
 * @breaf 姿勢編集マークを付ける。
 * @param (int motid) IN モーションのIDを指定する。
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN タイムラインを指定する。
 * @param (int curlineno) IN タイムラインのライン番号を指定する。
 * @param (double startframe) IN 編集開始フレームを指定する。
 * @param (double endframe) IN 編集終了フレームを指定する。
 * @param (int flag) IN マークにセットするフラグ。通常１を指定。
 * @return 成功したら０。
 * @detail 姿勢編集の開始時と終了時にマークを付け、すでにその間にマークがあればそれを消す。
 */
	int AddBoneMotMark( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag );


/**
 * @fn
 * CalcLocalInfo
 * @breaf 指定モーションの指定時間の姿勢からローカルな回転クォータニオンと移動ベクトルを計算する。
 * @param (int motid) IN モーションのIDを指定する。
 * @param (double frameno) IN モーションのフレーム（時間）を指定する。
 * @param (CMotionPoint* pdstmp) OUT 計算結果を受け取るCMotionPointのポインタを指定する。
 * @return 成功したら０。
 */
	int CalcLocalInfo( int motid, double frameno, CMotionPoint* pdstmp );
	int CalcCurrentLocalInfo(CMotionPoint* pdstmp);
	int CalcBtLocalInfo(CMotionPoint* pdstmp);

	int CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp);
/**
 * @fn
 * GetBoneNum
 * @breaf このボーンを親とするボーンの数を取得する。
 * @return ボーンの数。
 * @detail 子供と子供の兄弟の数の合計を返す。
 */
	int GetBoneNum();

	/**
	* @fn
	* CalcAxisMatX_aft
	* @breaf ボーンの軸のための変換行列を計算する。初期状態がX軸を向いていると仮定して計算する。
	* @param (ChaVector3 curpos) IN ボーンの位置。
	* @param (ChaVector3 childpos) IN 子供のボーンの位置。
	* @param (ChaMatrix* destmat) OUT 変換行列を格納するデータへのポインタ。
	* @return 成功したら０。
	* @detail CalcAxisMatXから呼ばれる。
	*/
	//int CalcAxisMatX_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* destmat);

	int CalcAxisMatZ_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* destmat);


	int CalcFirstFrameBonePos(ChaMatrix srcmat);

	int CalcBoneDepth();

	ChaVector3 CalcLocalEulXYZ(int axiskind, int srcmotid, double srcframe, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)
	ChaVector3 CalcCurrentLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);
	ChaVector3 CalcBtLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul = 0);

	ChaMatrix CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe);
	//ChaMatrix CalcManipulatorPostureMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe);
	ChaMatrix CalcManipulatorPostureMatrix(int calccapsuleflag, int anglelimitaxisflag, int settraflag, int multworld, int calczeroframe);
	int SetWorldMatFromEul(int inittraflag, int setchildflag, ChaVector3 srceul, int srcmotid, double srcframe, int initscaleflag = 0);
	int SetBtWorldMatFromEul(int setchildflag, ChaVector3 srceul);
	ChaMatrix CalcWorldMatFromEul(int inittraflag, int setchildflag, ChaVector3 srceul, ChaVector3 befeul, int srcmotid, double srcframe, int initscaleflag);//initscaleflag = 1 : default
	int SetWorldMatFromEulAndScaleAndTra(int inittraflag, int setchildflag, ChaVector3 srceul, ChaVector3 srcscale, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetWorldMatFromEulAndTra(int setchildflag, ChaVector3 srceul, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetWorldMatFromQAndTra(int setchildflag, CQuaternion axisq, CQuaternion srcq, ChaVector3 srctra, int srcmotid, double srcframe);
	int SetLocalEul(int srcmotid, double srcframe, ChaVector3 srceul);
	ChaVector3 GetLocalEul(int srcmotid, double srcframe);
	int SetWorldMat(bool infooutflag, int setchildflag, int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck = 0);
	ChaMatrix GetWorldMat(int srcmotid, double srcframe);
	ChaVector3 CalcLocalTraAnim(int srcmotid, double srcframe);
	ChaVector3 CalcLocalScaleAnim(int srcmotid, double srcframe);
	ChaMatrix CalcLocalScaleRotMat(int rotcenterflag, int srcmotid, double srcframe);
	ChaMatrix CalcLocalSymScaleRotMat(int rotcenterflag, int srcmotid, double srcframe);
	ChaVector3 CalcLocalSymScaleVec(int srcmotid, double srcframe);
	ChaVector3 CalcLocalSymTraAnim(int srcmotid, double srcframe);
	ChaMatrix CalcSymXMat(int srcmotid, double srcframe);
	ChaMatrix CalcSymXMat2(int srcmotid, double srcframe, int symrootmode);
	int PasteMotionPoint(int srcmotid, double srcframe, CMotionPoint srcmp);

	ChaVector3 CalcFBXEulXYZ(int srcnotmodifyflag, int srcmotid, double srcframe, ChaVector3* befeulptr = 0);
	//ChaVector3 CalcFBXEulZXY(int srcmotid, double srcframe, ChaVector3* befeulptr = 0);
	ChaVector3 CalcFBXTra(int srcmotid, double srcframe);
	int QuaternionInOrder(int srcmotid, double srcframe, CQuaternion* srcdstq);
	int CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos);

	int LoadCapsuleShape(ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext);

	int ChkMovableEul(ChaVector3 srceul);


	int SetCurrentMotion(int srcmotid, double animleng);
	void ResetMotionCache();

	ChaMatrix GetCurrentZeroFrameMat(int updateflag);//current motionのframe 0のworldmat
	ChaMatrix GetCurrentZeroFrameInvMat(int updateflag);


	static CBone* GetNewBone(CModel* parmodel);
	static void InvalidateBone(CBone* srcmp);
	static void InitBones();
	static void DestroyBones();
	static void OnDelModel(CModel* parmodel);

	ChaMatrix CalcParentGlobalMat(int srcmotid, double srcframe);

	void InitAddLimitQAll();
	void RotQAddLimitQAll(int srcmotid, double srcframe);


	int CreateIndexedMotionPoint(int srcmotid, double animleng);

	int ResizeIndexedMotionPoint(int srcmotid, double animleng);
	void ResizeIndexedMotionPointReq(int srcmotid, double animleng);

	ChaVector3 LimitEul(ChaVector3 srceul);
	void SetBefWorldMatReq(int srcmotid, double srcframe);

	int ResetAngleLimit(int srcval);
	int AngleLimitReplace180to170();
	int AdditiveCurrentToAngleLimit();
	int AdditiveToAngleLimit(ChaVector3 cureul);

private:

/**
 * @fn
 * InitParams
 * @breaf クラスメンバの初期化をする。
 * @return ０。
 */
	int InitParams();
	int InitParamsForReUse(CModel* srcparmodel);

	int SetParams(CModel* parmodel);//コンストラクタのInitParamsでは足りない部分

/**
 * @fn
 * DestroyObjs
 * @breaf クラスでアロケートしたメモリを開放する。
 * @return ０。
 */
	int DestroyObjs();


/**
 * @fn
 * CalcAxisMatX
 * @breaf ボーンの軸のための変換行列を計算する。ボーンの軸を計算してCalcAxisMatX_aftに渡して計算する。
 * @return 成功したら０。
 * @detail CalcAxisMatから呼ばれる。
 */
	//int CalcAxisMatX();


	int CalcAxisMatZ();


/**
 * @fn
 * CalcAxisMatY
 * @breaf ボーンの軸のための変換行列を計算する。初期状態がZ軸を向いていると仮定して計算する。
 * @param (CBone* childbone) IN 子供のボーン。
 * @param (ChaMatrix* dstmat) OUT 変換行列を格納するデータへのポインタ。
 * @return 成功したら０。
 */
	int CalcAxisMatY( CBone* childbone, ChaMatrix* dstmat );

/**
 * @fn
 * CalcLocalAxisMat
 * @breaf ローカルなボーン軸の変換行列を計算する。
 * @param (ChaMatrix motmat) IN グローバルな姿勢行列。
 * @param (ChaMatrix axismatpar) IN グローバルなボーン軸変換行列。X軸基準。
 * @param (ChaMatrix gaxisy) IN グローバルなボーン軸変換行列。Y軸基準。
 * @return 成功したら０。
 * @detail 計算結果はGetAxisMatPar()で取得する。
 */
	int CalcLocalAxisMat( ChaMatrix motmat, ChaMatrix axismatpar, ChaMatrix gaxisy );

/**
 * @fn
 * SetGroupNoByName
 * @breaf 剛体のあたり判定用のグループ番号を設定する。ボーンの名前から判定して設定する。
 * @param (CRigidElem* curre) IN 剛体のCRigidElemへのポインタ。
 * @param (CBone* childbone) IN ボーンのCBoneへのポインタ。
 * @return 成功したら０。
 * @detail BT_が名前につくものにも剛体用のグループ番号が与えられる。それ以外のグループ番号は、name_G_*** 形式の名前で指定できる。
 */
	int SetGroupNoByName( CRigidElem* curre, CBone* childbone );

/**
 * @fn
 * AddBoneMarkIfNot
 * @breaf ボーンの編集マークを付ける。
 * @param (int motid) IN モーションのIDを指定する。
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN タイムラインを指定する。
 * @param (int curlineno) IN タイムラインのライン番号を指定する。
 * @param (double curframe) IN モーションのフレームを指定する。
 * @param (int flag) IN マークにセットする値、通常は１を指定する。
 * @return 成功したら０。
 */
	int AddBoneMarkIfNot( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double curframe, int flag );


/**
 * @fn
 * DelBoneMarkRange
 * @breaf 指定範囲内の編集マークを削除する。
 * @param (int motid) IN モーションのIDを指定する。
 * @param (OrgWinGUI::OWP_Timeline* owpTimeline) IN タイムラインを指定する。
 * @param (int curlineno) IN タイムラインのライン番号を指定する。
 * @param (double startframe) IN 削除開始フレーム。
 * @param (double endframe) IN 削除終了フレーム。
 * @return 戻り値の説明
 */
	int DelBoneMarkRange( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe );

	void SetStartMat2Req();
	//void CalcFirstAxisMatX();
	void CalcFirstAxisMatZ();

	void InitAngleLimit();
	float LimitAngle(enum tag_axiskind srckind, float srcval);
	int InitCustomRig();
	void CalcBtRootDiffMatFunc(CBone* srcbone);

	ChaMatrix GetCurrentZeroFrameMatFunc(int updateflag, int inverseflag);

	void CalcParentGlobalMatReq(ChaMatrix* dstmat, CBone* srcbone, int srcmotid, double srcframe);


	void InitAddLimitQReq(CBone* srcbone);
	void RotQAddLimitQReq(CBone* srcbone, int srcmotid, double srcframe);


public: //accesser
	int GetType(){ return m_type; };
	void SetType( int srctype ){ m_type = srctype; };

	int GetSelectFlag(){ return m_selectflag; };
	void SetSelectFlag( int srcflag ){ m_selectflag = srcflag; };

	int GetBoneCnt(){ return m_bonecnt; };
	void SetBoneCnt( int srccnt ){ m_bonecnt = srccnt; };

	int GetValidFlag(){ return m_validflag; };
	void SetValidFlag( int srcflag ){ m_validflag = srcflag; };

	int GetBoneNo(){ return m_boneno; };
	void SetBoneNo( int srcno ){ m_boneno = srcno; };

	int GetTopBoneFlag(){ return m_topboneflag; };
	void SetTopBoneFlag( int srcflag ){ m_topboneflag = srcflag; };

	const char* GetBoneName(){ return (const char*)m_bonename; };
	void SetBoneName( char* srcname ){ strcpy_s( m_bonename, 256, srcname ); };

	const WCHAR* GetWBoneName(){ return (const WCHAR*)m_wbonename; };
	void SetWBoneName( WCHAR* srcname ){ wcscpy_s( m_wbonename, 256, srcname ); };

	const char* GetEngBoneName(){ return (const char*)m_engbonename; };
	void SetEngBoneName( char* srcname ){ strcpy_s( m_engbonename, 256, srcname ); };

	ChaVector3 GetChildWorld();
	void SetChildWorld( ChaVector3 srcvec ){ m_childworld = srcvec; };

	ChaVector3 GetChildScreen(){ return m_childscreen; };
	void SetChildScreen( ChaVector3 srcvec ){ m_childscreen = srcvec; };

	int GetMotionKeySize(){ return (int)m_motionkey.size(); };
	CMotionPoint* GetMotionKey( int srccookie ){
		if ((srccookie >= 1) && (srccookie <= m_motionkey.size())) {
			return m_motionkey[srccookie - 1];
		}
		else {
			//_ASSERT(0);//OnAddMotionのときには有り得る
			return 0;
		}
	};
	void SetMotionKey( int srccookie, CMotionPoint* srcmk ){ 
		if ((srccookie >= 1) && (srccookie <= m_motionkey.size())) {
			m_motionkey[srccookie - 1] = srcmk;
		}
		else {
			_ASSERT(0);
		}
	};

	CMotionPoint GetCurMp(){ return m_curmp; };
	void SetCurMp( CMotionPoint srcmp ){ m_curmp = srcmp; };

	//CMotionPoint GetBefMp(){ return m_befmp; };
	//void SetBefMp(CMotionPoint srcmp){ m_befmp = srcmp; };


	CQuaternion GetAxisQ(){ return m_axisq; };
	void SetAxisQ( CQuaternion srcq ){ m_axisq = srcq; };

	ChaMatrix GetLAxisMat(){ return m_laxismat; };
	ChaMatrix GetAxisMatPar(){ return m_axismat_par; };
	ChaMatrix GetInvAxisMatPar(){
		ChaMatrix invaxis;
		ChaMatrixInverse(&invaxis, NULL, &m_axismat_par);
		return invaxis;
	};

	ChaMatrix GetStartMat2(){ return m_startmat2; };
	ChaMatrix GetInvStartMat2(){
		ChaMatrix retmat;
		ChaMatrixInverse(&retmat, NULL, &m_startmat2);
		return retmat;
	};
	void SetStartMat2(ChaMatrix srcmat){ m_startmat2 = srcmat; };

	int GetGetAnimFlag(){ return m_getanimflag; };
	void SetGetAnimFlag( int srcflag ){ m_getanimflag = srcflag; };

	ChaMatrix GetNodeMat(){ return m_nodemat; };
	ChaMatrix GetInvNodeMat(){
		ChaMatrix retmat;
		ChaMatrixInverse(&retmat, NULL, &m_nodemat);
		return retmat;
	};
	void SetNodeMat( ChaMatrix srcmat ){ m_nodemat = srcmat; };


	ChaMatrix GetFirstMat(){ return m_firstmat; };
	void SetFirstMat( ChaMatrix srcmat ){ m_firstmat = srcmat; };

	ChaMatrix GetInvFirstMat(){ ChaMatrixInverse(&m_invfirstmat, NULL, &m_firstmat); return m_invfirstmat; };
	void SetInvFirstMat( ChaMatrix srcmat ){ m_invfirstmat = srcmat; };

	ChaMatrix GetInitMat(){ return m_initmat; };
	void SetInitMat(ChaMatrix srcmat){ m_initmat = srcmat; };

	ChaMatrix GetInvInitMat(){ ChaMatrixInverse(&m_invinitmat, NULL, &m_initmat); return m_invinitmat; };
	void SetInvInitMat(ChaMatrix srcmat){ m_invinitmat = srcmat; };

	ChaMatrix GetTmpMat(){ return m_tmpmat; };
	void SetTmpMat(ChaMatrix srcmat){ m_tmpmat = srcmat; };

	CQuaternion GetTmpQ(){ return m_tmpq; };
	void SetTmpQ(CQuaternion srcq){ m_tmpq = srcq; };


	ChaVector3 GetJointFPos();
	void SetJointFPos(ChaVector3 srcpos);
	void SetOldJointFPos(ChaVector3 srcpos);

	ChaVector3 GetJointWPos(){ return m_jointwpos; };
	void SetJointWPos( ChaVector3 srcpos ){ m_jointwpos = srcpos; };

	FbxAMatrix GetGlobalPosMat(){ return m_globalpos; };
	void SetGlobalPosMat( FbxAMatrix srcmat ){ m_globalpos = srcmat; };

	int GetRigidElemSize(){
		std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
		itrremap = m_remap.find(m_rigidelemname);
		if (itrremap != m_remap.end()){
			return (int)itrremap->second.size();
		}
		else{
			return 0;
		}
	};
		
	CRigidElem* GetRigidElem( CBone* srcbone ){ 
		return GetRigidElemOfMap(m_rigidelemname, srcbone);
	};
	//std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapBegin(){
	//	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
	//	itrremap = m_remap.find(m_rigidelemname);
	//	if (itrremap != m_remap.end()){
	//		return itrremap->second.begin();
	//	}
	//	else{
	//		return itrremap->second.end();
	//	}
	//};
	//std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapEnd(){
	//	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
	//	itrremap = m_remap.find(m_rigidelemname);
	//	return itrremap->second.end();
	//};
	void SetRigidElem( CBone* srcbone, CRigidElem* srcre ){ 
		SetRigidElemOfMap(m_rigidelemname, srcbone, srcre);
	};

	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator FindRigidElemOfMap(std::string srcname){
	//	return m_remap.find(srcname);
	//};
	int GetReMapSize(){ return (int)m_remap.size(); };
	int GetReMapSize2(std::string srcstring){
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstring ];
		return (int)curmap.size();
	};
	CRigidElem* GetRigidElemOfMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
		itrremap = m_remap.find(srcstr);
		if (itrremap != m_remap.end()){
			//itrremap->second[srcbone];
			std::map<CBone*, CRigidElem*>::iterator itrmap2;
			itrmap2 = itrremap->second.find(srcbone);
			if (itrmap2 != itrremap->second.end()){
				return itrmap2->second;
			}
			else{
				return 0;
			}
		}
		else{
			return 0;
		}
	};
	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapBegin(){
	//	return m_remap.begin();
	//};
	//std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapEnd(){
	//	return m_remap.end();
	//};

	void SetRigidElemOfMap(std::string srcstr, CBone* srcbone, CRigidElem* srcre);

	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator FindImpMap(std::string srcname){
		return m_impmap.find(srcname);
	};
	int GetImpMapSize(){ return (int)m_impmap.size(); };
	int GetImpMapSize2(std::string srcstring){
		std::map<CBone*, ChaVector3> curmap = m_impmap[ srcstring ];
		return (int)curmap.size();
	};
	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator GetImpMapBegin(){
		return m_impmap.begin();
	};
	std::map<std::string, std::map<CBone*, ChaVector3>>::iterator GetImpMapEnd(){
		return m_impmap.end();
	};
	ChaVector3 GetImpMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, ChaVector3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, ChaVector3> curmap = itrimpmap->second;
			std::map<CBone*, ChaVector3>::iterator itrimp;
			itrimp = curmap.find(srcbone);
			if (itrimp != curmap.end()){
				return itrimp->second;
			}
			else{
				//_ASSERT(0);
				return ChaVector3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			_ASSERT(0);
			return ChaVector3(0.0f, 0.0f, 0.0f);
		}
	};
	void SetImpMap2( std::string srcstring, std::map<CBone*,ChaVector3>& srcmap ){
		m_impmap[ srcstring ] = srcmap;
	};
	void SetImpOfMap( std::string srcstr, CBone* srcbone, ChaVector3 srcimp ){
		std::map<std::string, std::map<CBone*, ChaVector3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, ChaVector3>::iterator itrsetmap;
			itrsetmap = itrimpmap->second.find(srcbone);
			if (itrsetmap != itrimpmap->second.end()){
				itrsetmap->second = srcimp;
			}
			else{
				itrimpmap->second[srcbone] = srcimp;
			}
		}
		else{
			std::map<CBone*, ChaVector3> newmap;
			newmap[srcbone] = srcimp;
			m_impmap[srcstr] = newmap;
		}
		//((m_impmap[ srcstr ])[ srcbone ]) = srcimp;
	};

	int GetBtKinFlag(){ return m_btkinflag; };
	void SetBtKinFlag( int srcflag ){ m_btkinflag = srcflag; };

	int GetBtForce(){ return m_btforce; };
	void SetBtForce( int srcval ){ m_btforce = srcval; };

	int GetBtObjectSize(){ return (int)m_btobject.size(); };
	CBtObject* GetBtObject( CBone* srcbone ){ return m_btobject[ srcbone ]; };
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapBegin(){
		return m_btobject.begin();
	};
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapEnd(){
		return m_btobject.end();
	};

	void SetBtObject( CBone* srcbone, CBtObject* srcbo ){ m_btobject[ srcbone ] = srcbo; };
	void ClearBtObject(){ m_btobject.clear(); };


	int GetMotMarkSize(){ return (int)m_motmark.size(); };
	int GetMotMarkSize2(int srcindex){
		std::map<double, int> curmap = m_motmark[ srcindex ];
		return (int)curmap.size();
	};
	int GetMotMarkOfMap( int srcindex, double srcframe ){
		std::map<double,int> curmap = m_motmark[ srcindex ];
		return curmap[ srcframe ];
	};
	void GetMotMarkOfMap2( int srcindex, std::map<double, int>& dstmap ){
		dstmap = m_motmark[ srcindex ];
	};
	std::map<int, std::map<double, int>>::iterator GetMotMarkOfMapBegin(){
		return m_motmark.begin();
	};
	std::map<int, std::map<double, int>>::iterator GetMotMarkOfMapEnd(){
		return m_motmark.end();
	};
	std::map<int, std::map<double, int>>::iterator FindMotMarkOfMap( int srcindex ){
		return m_motmark.find( srcindex );
	};
	void SetMotMarkOfMap( int srcindex, double srcframe, int srcmark ){
		((m_motmark[ srcindex ])[ srcframe ]) = srcmark;
	};
	void SetMotMarkOfMap2( int srcindex, std::map<double, int>& srcmap ){
		m_motmark[ srcindex ] = srcmap;
	};
	void ClearMotMarkOfMap2( int srcindex ){
		m_motmark[ srcindex ].clear();
	};

	ChaVector3 GetFirstFrameBonePos()
	{
		return m_firstframebonepos;
	};

	ChaVector3 GetWorldPos(int srcmotid, double srcframe);
	ChaMatrix GetLimitedWorldMat(int srcmotid, double srcframe, ChaVector3* dstneweul = 0);
	ChaMatrix GetCurrentLimitedWorldMat();


	CModel* GetParModel(){ return m_parmodel; };
	//void SetParModel( CModel* srcpar ){ m_parmodel = srcpar; };//parmodelごとのm_bonenoに注意！！！

	CBone* GetParent(){ return m_parent; };
	void SetParent( CBone* srcpar ){ m_parent = srcpar; };

	CBone* GetChild(){ return m_child; };
	void SetChild( CBone* srcchil ){ m_child = srcchil; };

	CBone* GetBrother(){ return m_brother; };
	void SetBrother( CBone* srcbro ){ m_brother = srcbro; };
	CBone* GetSister() {
		CBone* parbone = GetParent();
		if (parbone) {
			CBone* firstbrobone = parbone->GetChild();
			CBone* nextbone = firstbrobone;
			while (nextbone) {
				if (nextbone) {
					if (nextbone->GetBrother() == this) {
						return nextbone;//!!!!!!!!!!!!
					}
					nextbone = nextbone->GetBrother();
				}
				else {
					return 0;
				}
			}
			return 0;
		}
		else {
			return 0;
		}
		return 0;
	};
	bool IsBranchBone()
	{
		CBone* chksister = GetSister();
		CBone* chkbrother = GetBrother();
		if (chksister || chkbrother) {
			return true;
		}
		else {
			return false;
		}
	}
	CBone* GetUpperBranchBone() {
		CBone* curbone = GetParent();
		while (curbone) {
			if (curbone->IsBranchBone()) {
				return curbone;
			}
			curbone = curbone->GetParent();
		}
		return 0;
	};
	CBone* GetLowerBranchBone() {
		CBone* curbone = GetChild();
		while (curbone) {
			if (curbone->IsBranchBone()) {
				return curbone;
			}
			curbone = curbone->GetChild();
		}
		return 0;
	}
	

	bool GetPositionFound(){
		return m_posefoundflag;
	};
	void SetPositionFound(bool foundflag){
		m_posefoundflag = foundflag;
	};

	double GetBoneLeng(){
		CBone* parentbone = GetParent();
		if (parentbone){
			ChaVector3 bonevec = GetJointFPos() - parentbone->GetJointFPos();
			double boneleng = ChaVector3LengthDbl(&bonevec);
			return boneleng;
		}
		else{
			return 0.0;
		}
	};
	/*
	ChaMatrix GetFirstAxisMatX()
	{
		CalcFirstAxisMatX();
		return m_firstaxismatX;
	};
	*/
	ChaMatrix GetFirstAxisMatZ()
	{
		CalcFirstAxisMatZ();
		return m_firstaxismatZ;
	};

	CMotionPoint* GetMotionPoint(int srcmotid, double srcframe, bool onaddmotion = false){
		//存在するときだけ返す。
		CMotionPoint* pbef = 0;
		CMotionPoint* pnext = 0;
		int existflag = 0;
		GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, &existflag, onaddmotion);
		if (existflag == 1){
			return pbef;
		}
		else{
			return 0;
		}
	};

	ANGLELIMIT GetAngleLimit(int getchkflag);
	void SetAngleLimit(ANGLELIMIT srclimit);

	int GetFreeCustomRigNo();
	CUSTOMRIG CBone::GetFreeCustomRig();
	CUSTOMRIG GetCustomRig(int rigno);
	void SetCustomRig(CUSTOMRIG srccr);

	ChaMatrix GetTmpSymMat()
	{
		return m_tmpsymmat;
	};
	void SetTmpSymMat(ChaMatrix srcmat)
	{
		m_tmpsymmat = srcmat;
	};
	ChaVector3 GetBtparentpos(){
		return m_btparentpos;
	};
	ChaVector3 GetBtchildpos(){
		return m_btchildpos;
	};
	ChaMatrix GetBtDiffMat(){
		return m_btdiffmat;
	};


	ChaMatrix GetBefBtMat(){ return m_befbtmat; };
	void SetBefBtMat(ChaMatrix srcmat){ m_befbtmat = srcmat; };
	ChaMatrix GetBtMat(){ return m_btmat; };
	ChaMatrix GetInvBtMat(){ ChaMatrix retmat; ChaMatrixInverse(&retmat, NULL, &m_btmat); return retmat; };
	void SetBtMat(ChaMatrix srcmat){
		//if (GetBtFlag() == 0){
			SetBefBtMat(m_btmat);
		//}
		m_btmat = srcmat;
	};

	int GetBtFlag(){ return m_setbtflag; };
	void SetBtFlag(int srcflag){ m_setbtflag = srcflag; };

	CModel* GetCurColDisp(CBone* childbone);
	CModel* GetColDisp(CBone* childbone, int srcindex);
	void SetFirstCalcRigid(bool srcflag){
		m_firstcalcrigid = srcflag;
	};

	void SetPosConstraint(int srcval){
		m_posconstraint = srcval;
	};
	int GetPosConstraint(){
		return m_posconstraint;
	};

	void SetMass0(int srcval){
		m_mass0 = srcval;
	};
	int GetMass0(){
		return m_mass0;
	};

	void SetExcludeMv(int srcval){
		m_excludemv = srcval;
	};
	int GetExcludeMv(){
		return m_excludemv;
	};
	void SetCurMotID(int srcmotid);
	int GetCurMotID()
	{
		return m_curmotid;
	};
	void SetTmpKinematic(bool srcflag)
	{
		m_tmpkinematic = srcflag;
	};
	bool GetTmpKinematic()
	{
		return m_tmpkinematic;
	};
	

	int GetUseFlag()
	{
		return m_useflag;
	};
	void SetUseFlag(int srcflag)
	{
		m_useflag = srcflag;
	};
	int GetIndexOfPool()
	{
		return m_indexofpool;
	};
	void SetIndexOfPool(int srcindex)
	{
		m_indexofpool = srcindex;
	};
	int IsAllocHead()
	{
		return m_allocheadflag;
	};
	void SetIsAllocHead(int srcflag)
	{
		m_allocheadflag = srcflag;
	};


	void SetExtendFlag(bool srcflag) {
		m_extendflag = srcflag;
	};
	bool GetExtendFlag() {
		return m_extendflag;
	};

	void InitAddLimitQ()
	{
		m_addlimitq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}
	void Add2AddLimitQ(CQuaternion srcq);
	CQuaternion GetAddLimitQ()
	{
		return m_addlimitq;
	}

private:
	CRITICAL_SECTION m_CritSection_GetBefNext;
	CRITICAL_SECTION m_CritSection_AddMP;

	int m_useflag;//0: not use, 1: in use
	int m_indexofpool;//index of pool vector
	int m_allocheadflag;//1: head pointer at allocated


	int m_posconstraint;
	int m_mass0;
	int m_excludemv;
	bool m_firstcalcrigid;
	int m_type;
	int m_selectflag;
	int m_bonecnt;
	int m_validflag;
	int m_boneno;
	int m_topboneflag;
	char m_bonename[256];
	WCHAR m_wbonename[256];
	char m_engbonename[256];
	int m_curmotid;

	int m_upkind;//m_gaxismatXpar計算時のupvec

	bool m_posefoundflag;//BindPoseの中にこのボーンの位置情報があった場合true。

	ChaVector3 m_childworld;//ボーンの子供のモーション行列適用後の座標。
	ChaVector3 m_childscreen;//ボーンの子供のWVP適用後の座標。




	std::map<int, CMotionPoint*> m_motionkey;//m_motionkey[ モーションID ]でモーションの最初のフレームの姿勢にアクセスできる。
	CMotionPoint m_curmp;//現在のWVP適用後の姿勢データ。
	CMotionPoint m_befmp;//一回前の姿勢データ。
	CMotionPoint* m_cachebefmp[MAXMOTIONNUM + 1];//motidごとのキャッシュ

	//std::vector<CMotionPoint*> m_indexedmp;
	std::map<int, std::vector<CMotionPoint*>> m_indexedmotionpoint;
	std::map<int, bool> m_initindexedmotionpoint;
	CMotionPoint m_dummymp;

	CQuaternion m_axisq;//ボーンの軸のクォータニオン表現。

	ChaMatrix m_laxismat;//Zボーンのaxismat
	//ChaMatrix m_gaxismatXpar;//Xボーンのグローバルのaxismat
	ChaMatrix m_gaxismatYpar;//Yボーンのグローバルのaxismat
	ChaMatrix m_axismat_par;//Xボーンのローカルのaxismat
	//ChaMatrix m_firstaxismatX;//初期状態でのXボーンのグローバルaxismat
	ChaMatrix m_firstaxismatZ;//初期状態でのZボーンのグローバルaxismat


	ChaMatrix m_startmat2;//ワールド行列を保存しておくところ。剛体シミュレーションを始める際などに保存する。

	int m_getanimflag;//FBXファイルを読み込む際にアニメーションを読み込んだら１。
	ChaMatrix m_nodemat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	ChaMatrix m_firstmat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	ChaMatrix m_invfirstmat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	ChaVector3 m_jointfpos;//ジョイントの初期位置。
	ChaVector3 m_oldjointfpos;//ジョイント初期位置（旧データ互換）
	ChaVector3 m_jointwpos;//FBXにアニメーションが入っていない時のジョイントの初期位置。
	FbxAMatrix m_globalpos;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	ChaMatrix m_initmat;
	ChaMatrix m_invinitmat;
	ChaMatrix m_tmpmat;//一時使用目的
	CQuaternion m_tmpq;
	ChaMatrix m_tmpsymmat;

	CQuaternion m_addlimitq;

	ChaMatrix m_btmat;
	ChaMatrix m_befbtmat;
	int m_setbtflag;


	ChaVector3 m_firstframebonepos;

	ANGLELIMIT m_anglelimit;

	int m_initcustomrigflag;
	CUSTOMRIG m_customrig[MAXRIGNUM];

	//CBone*は子供ジョイントのポインタ。子供の数だけエントリーがある。
	//std::map<CBone*, CRigidElem*> m_rigidelem;
	std::string m_rigidelemname;


	//m_remapは、jointの名前でセットすればmap<string,CRigidElem*>で済む。
	//名前で一意なRigidElemを選択するために子供のボーンの名前を使用する。
	//そのため子供のボーンの名前とCRigidElemのセットを使う。
	//m_impmapについても同様のことがいえる。
	std::map<std::string, std::map<CBone*, CRigidElem*>> m_remap;//map<設定ファイル名, map<子供ボーン, 剛体設定>>
	std::map<std::string, std::map<CBone*, ChaVector3>> m_impmap;//map<設定ファイル名, map<子供ボーン, インパルス設定>>


	int m_btkinflag;//bullet kinematic flag。剛体シミュレーションの根元のボーンが固定モーションに追随する際は０を指定する。その他は１。
	int m_btforce;//bullet 強制フラグ。モデルに設定されているbtkinflagである。m_btkinflagはシミュレーション開始と終了で変化するが、このフラグは読み込み時に設定されるものである。
	std::map<CBone*,CBtObject*> m_btobject;//CBtObjectはbulletの剛体オブジェクトをラップしたクラスである。ボーンとCBtObjectのmap。

	//タイムラインのモーション編集マーク
	//最初のintはmotid。次のmapはframenoと更新フラグ。更新フラグは読み込み時のマークは０、それ以後の編集マークは１にする予定。色を変えるかも。
	std::map<int, std::map<double,int>> m_motmark;

	CModel* m_coldisp[COL_MAX];

	CModel* m_parmodel;

	int m_firstgetflag;//GetCurrentZeroFrameMat用
	ChaMatrix m_firstgetmatrix;//GetCurrentZeroFrameMat用
	ChaMatrix m_invfirstgetmatrix;//GetCurrentZeroFrameMat用

	bool m_tmpkinematic;

	bool m_extendflag;

	double m_befupdatetime;


	CBone* m_parent;
	CBone* m_child;
	CBone* m_brother;
};

#endif