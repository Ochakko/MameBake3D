#ifndef BONEH
#define BONEH

#include <d3dx9.h>
#include <Coef.h>
#include <MotionPoint.h>
#include <quaternion.h>

#include <fbxsdk.h>

#include <map>
#include <string>

#include <orgwindow.h>

class CMQOFace;
class CMotionPoint;
class CRigidElem;
class CBtObject;
class CModel;

class CBone
{
public:

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
 * @param (D3DXMATRIX* wmat) IN ワールド座標系の変換行列を指定する。
 * @param (D3DXMATRIX* vpmat) IN カメラプロジェクション座標系の変換行列を指定する。
 * @return 成功したら０。
 * @detail 指定モーションの指定時間の姿勢を計算する。グローバルな姿勢の計算である。
 */
	int UpdateMatrix( int srcmotid, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat );

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
	int GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr );

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
 * @param (CModel* cpslptr[COL_MAX]) INOUT　剛体の基本形状。カプセル、ボックス、球、コーンの種類がある。
 * @param (CBone* chilbone) IN　剛体を指定するための子供ボーン。
 * @param (int setstartflag) IN　剛体シミュレーション開始時の呼び出し時に１をセットする。
 * @return 成功したら０。
 * @detail 剛体をボーンの位置に表示するために、剛体表示用の形状をスケールするために呼ぶ。剛体はボーンの子供ジョイントと１対１で対応するため、指定にchilboneを使う。
 */
	int CalcRigidElemParams( CModel* cpslptr[COL_MAX], CBone* chilbone, int setstartflag );

/**
 * @fn
 * CalcAxisMat
 * @breaf ボーンの軸のための変換行列を計算する。初期状態がX軸を向いていると仮定して計算する。
 * @param (int firstflag) IN 変換開始の行列を保存したいときに１を指定する。
 * @param (float delta) IN 回転角度をしていする。現在は０でしか使用していない。
 * @return 成功したら０。
 * @detail 計算したデータは、ボーンの位置にマニピュレータを表示するための変換行列に使用する。現在はCalcAxisMatZ関数でボーンの変換行列を計算している。
 */
	int CalcAxisMat( int firstflag, float delta );
	

/**
 * @fn
 * CalcAxisMatZ
 * @breaf ボーンの親と子供の位置を指定して、その軸の変換行列を計算する。
 * @param (D3DXVECTOR3* srccur) IN ボーンの親の座標を指定する。
 * @param (D3DXVECTOR3* srcchil) IN ボーンの子供の座標を指定する。
 * @return 成功したら０。
 * @detail 初期状態がZ軸を向いていると仮定して計算する。ボーンの位置にマニピュレータを表示するために使用する。
 */
	int CalcAxisMatZ( D3DXVECTOR3* srccur, D3DXVECTOR3* srcchil );


/**
 * @fn
 * CreateRigidElem
 * @breaf 剛体シミュレーション用のパラメータを保持するCRigidElemを作成する。指定ボーンに関するCRigidElemを作成する。
 * @param (CBone* chil) IN 自分自身とここで指定する子供ジョイントで定義されるボーンに関するCRigidElemを作成する。
 * @param (int reflag) IN CRigidElemを作成する場合に１、しない場合に０を指定。
 * @param (std::string rename) IN reflagが１のとき、refファイルのファイル名を指定する。
 * @param (int impflag) IN インパルスを与えるための設定を作成する場合に１、しない場合に０を指定。
 * @param (std::string impname) IN impflagが１のとき、impファイルのファイル名を指定する。
 * @return 成功したら０。
 * @detail インパルス設定データも作成する。
 */
	int CreateRigidElem( CBone* chil, int reflag, std::string rename, int impflag, std::string impname );


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
 * @param (D3DXVECTOR3 srctra) IN 移動ベクトルを指定する。
 * @return 計算した姿勢を格納したCMotionPointのポインタを返すが再帰関数であることに注意。ポインタはチェインにセットされたものである。
 * @detail 想定している使い方としては、外部からの呼び出し時にはparmpを０にする。この関数内での再帰呼び出し時にparmpに親をセットする。
 */
	CMotionPoint* AddBoneTraReq( CMotionPoint* parmp, int srcmotid, double srcframe, D3DXVECTOR3 srctra );

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
	CMotionPoint* RotBoneQReq(CMotionPoint* parmp, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone = 0, D3DXVECTOR3 traanim = D3DXVECTOR3(0.0f, 0.0f, 0.0f), int setmatflag = 0, D3DXMATRIX* psetmat = 0);

/**

*/
	CMotionPoint* RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, D3DXMATRIX srcmat);

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

	int CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp);
/**
 * @fn
 * GetBoneNum
 * @breaf このボーンを親とするボーンの数を取得する。
 * @return ボーンの数。
 * @detail 子供と子供の兄弟の数の合計を返す。
 */
	int GetBoneNum();


	int CalcFirstFrameBonePos(D3DXMATRIX srcmat);


private:

/**
 * @fn
 * InitParams
 * @breaf クラスメンバの初期化をする。
 * @return ０。
 */
	int InitParams();

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
	int CalcAxisMatX();

/**
 * @fn
 * CalcAxisMatX_aft
 * @breaf ボーンの軸のための変換行列を計算する。初期状態がX軸を向いていると仮定して計算する。
 * @param (D3DXVECTOR3 curpos) IN ボーンの位置。
 * @param (D3DXVECTOR3 chilpos) IN 子供のボーンの位置。
 * @param (D3DXMATRIX* destmat) OUT 変換行列を格納するデータへのポインタ。
 * @return 成功したら０。
 * @detail CalcAxisMatXから呼ばれる。
 */
	int CalcAxisMatX_aft( D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* destmat );


/**
 * @fn
 * CalcAxisMatY
 * @breaf ボーンの軸のための変換行列を計算する。初期状態がZ軸を向いていると仮定して計算する。
 * @param (CBone* chilbone) IN 子供のボーン。
 * @param (D3DXMATRIX* dstmat) OUT 変換行列を格納するデータへのポインタ。
 * @return 成功したら０。
 */
	int CalcAxisMatY( CBone* chilbone, D3DXMATRIX* dstmat );

/**
 * @fn
 * CalcLocalAxisMat
 * @breaf ローカルなボーン軸の変換行列を計算する。
 * @param (D3DXMATRIX motmat) IN グローバルな姿勢行列。
 * @param (D3DXMATRIX axismatpar) IN グローバルなボーン軸変換行列。X軸基準。
 * @param (D3DXMATRIX gaxisy) IN グローバルなボーン軸変換行列。Y軸基準。
 * @return 成功したら０。
 * @detail 計算結果はGetAxisMatPar()で取得する。
 */
	int CalcLocalAxisMat( D3DXMATRIX motmat, D3DXMATRIX axismatpar, D3DXMATRIX gaxisy );

/**
 * @fn
 * SetGroupNoByName
 * @breaf 剛体のあたり判定用のグループ番号を設定する。ボーンの名前から判定して設定する。
 * @param (CRigidElem* curre) IN 剛体のCRigidElemへのポインタ。
 * @param (CBone* chilbone) IN ボーンのCBoneへのポインタ。
 * @return 成功したら０。
 * @detail BT_が名前につくものにも剛体用のグループ番号が与えられる。それ以外のグループ番号は、name_G_*** 形式の名前で指定できる。
 */
	int SetGroupNoByName( CRigidElem* curre, CBone* chilbone );

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

	D3DXVECTOR3 GetChildWorld(){ return m_childworld; };
	void SetChildWorld( D3DXVECTOR3 srcvec ){ m_childworld = srcvec; };

	D3DXVECTOR3 GetChildScreen(){ return m_childscreen; };
	void SetChildScreen( D3DXVECTOR3 srcvec ){ m_childscreen = srcvec; };

	int GetMotionKeySize(){ return m_motionkey.size(); };
	CMotionPoint* GetMotionKey( int srccookie ){ return m_motionkey[ srccookie ]; };
	void SetMotionKey( int srccookie, CMotionPoint* srcmk ){ m_motionkey[ srccookie ] = srcmk; };

	CMotionPoint GetCurMp(){ return m_curmp; };
	void SetCurMp( CMotionPoint srcmp ){ m_curmp = srcmp; };

	CQuaternion GetAxisQ(){ return m_axisq; };
	void SetAxisQ( CQuaternion srcq ){ m_axisq = srcq; };

	D3DXMATRIX GetLAxisMat(){ return m_laxismat; };
	D3DXMATRIX GetAxisMatPar(){ return m_axismat_par; };

	D3DXMATRIX GetStartMat2(){ return m_startmat2; };
	void SetStartMat2( D3DXMATRIX srcmat ){ m_startmat2 = srcmat; };

	int GetGetAnimFlag(){ return m_getanimflag; };
	void SetGetAnimFlag( int srcflag ){ m_getanimflag = srcflag; };

	D3DXMATRIX GetNodeMat(){ return m_nodemat; };
	void SetNodeMat( D3DXMATRIX srcmat ){ m_nodemat = srcmat; };

	D3DXMATRIX GetFirstMat(){ return m_firstmat; };
	void SetFirstMat( D3DXMATRIX srcmat ){ m_firstmat = srcmat; };

	D3DXMATRIX GetInvFirstMat(){ D3DXMatrixInverse(&m_invfirstmat, NULL, &m_firstmat); return m_invfirstmat; };
	void SetInvFirstMat( D3DXMATRIX srcmat ){ m_invfirstmat = srcmat; };

	D3DXMATRIX GetInitMat(){ return m_initmat; };
	void SetInitMat(D3DXMATRIX srcmat){ m_initmat = srcmat; };

	D3DXMATRIX GetInvInitMat(){ D3DXMatrixInverse(&m_invinitmat, NULL, &m_initmat); return m_invinitmat; };
	void SetInvInitMat(D3DXMATRIX srcmat){ m_invinitmat = srcmat; };

	D3DXMATRIX GetTmpMat(){ return m_tmpmat; };
	void SetTmpMat(D3DXMATRIX srcmat){ m_tmpmat = srcmat; };

	CQuaternion GetTmpQ(){ return m_tmpq; };
	void SetTmpQ(CQuaternion srcq){ m_tmpq = srcq; };


	D3DXVECTOR3 GetJointFPos(){ return m_jointfpos; };
	void SetJointFPos( D3DXVECTOR3 srcpos ){ m_jointfpos = srcpos; };

	D3DXVECTOR3 GetJointWPos(){ return m_jointwpos; };
	void SetJointWPos( D3DXVECTOR3 srcpos ){ m_jointwpos = srcpos; };

	FbxAMatrix GetGlobalPosMat(){ return m_globalpos; };
	void SetGlobalPosMat( FbxAMatrix srcmat ){ m_globalpos = srcmat; };

	int GetRigidElemSize(){ return m_rigidelem.size(); };
	void GetRigidElemMap( std::map<CBone*, CRigidElem*>& dstmap ){
		dstmap = m_rigidelem;
	};
	CRigidElem* GetRigidElem( CBone* srcbone ){ return m_rigidelem[ srcbone ]; };
	std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapBegin(){
		return m_rigidelem.begin();
	};
	std::map<CBone*, CRigidElem*>::iterator GetRigidElemMapEnd(){
		return m_rigidelem.end();
	};
	void SetRigidElem( CBone* srcbone, CRigidElem* srcre ){ m_rigidelem[ srcbone ] = srcre; };


	int GetReMapSize(){ return m_remap.size(); };
	int GetReMapSize2( std::string srcstring ){ 
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstring ];
		return curmap.size();
	};
	CRigidElem* GetRigidElemOfMap( std::string srcstr, CBone* srcbone ){
		std::map<CBone*, CRigidElem*> curmap = m_remap[ srcstr ];
		return curmap[ srcbone ];
	};
	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapBegin(){
		return m_remap.begin();
	};
	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator GetRigidElemOfMapEnd(){
		return m_remap.end();
	};

	void SetRigidElemOfMap( std::string srcstr, CBone* srcbone, CRigidElem* srcre ){
		((m_remap[ srcstr ])[ srcbone ]) = srcre;
	};


	int GetImpMapSize(){ return m_impmap.size(); };
	int GetImpMapSize2( std::string srcstring ){ 
		std::map<CBone*, D3DXVECTOR3> curmap = m_impmap[ srcstring ];
		return curmap.size();
	};
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator GetImpMapBegin(){
		return m_impmap.begin();
	};
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator GetImpMapEnd(){
		return m_impmap.end();
	};
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator FindImpMap( std::string strname ){
		return m_impmap.find( strname );
	};
	D3DXVECTOR3 GetImpMap( std::string srcstr, CBone* srcbone ){
		std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, D3DXVECTOR3> curmap = itrimpmap->second;
			std::map<CBone*, D3DXVECTOR3>::iterator itrimp;
			itrimp = curmap.find(srcbone);
			if (itrimp != curmap.end()){
				return itrimp->second;
			}
			else{
				//_ASSERT(0);
				return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			}
		}
		else{
			_ASSERT(0);
			return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		}
	};
	void SetImpMap2( std::string srcstring, std::map<CBone*,D3DXVECTOR3>& srcmap ){
		m_impmap[ srcstring ] = srcmap;
	};
	void SetImpOfMap( std::string srcstr, CBone* srcbone, D3DXVECTOR3 srcimp ){
		std::map<std::string, std::map<CBone*, D3DXVECTOR3>>::iterator itrimpmap;
		itrimpmap = m_impmap.find(srcstr);
		if (itrimpmap != m_impmap.end()){
			std::map<CBone*, D3DXVECTOR3>::iterator itrsetmap;
			itrsetmap = itrimpmap->second.find(srcbone);
			if (itrsetmap != itrimpmap->second.end()){
				itrsetmap->second = srcimp;
			}
			else{
				itrimpmap->second[srcbone] = srcimp;
			}
		}
		else{
			std::map<CBone*, D3DXVECTOR3> newmap;
			newmap[srcbone] = srcimp;
			m_impmap[srcstr] = newmap;
		}
		//((m_impmap[ srcstr ])[ srcbone ]) = srcimp;
	};

	int GetBtKinFlag(){ return m_btkinflag; };
	void SetBtKinFlag( int srcflag ){ m_btkinflag = srcflag; };

	int GetBtForce(){ return m_btforce; };
	void SetBtForce( int srcval ){ m_btforce = srcval; };

	int GetBtObjectSize(){ return m_btobject.size(); };
	CBtObject* GetBtObject( CBone* srcbone ){ return m_btobject[ srcbone ]; };
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapBegin(){
		return m_btobject.begin();
	};
	std::map<CBone*, CBtObject*>::iterator GetBtObjectMapEnd(){
		return m_btobject.end();
	};

	void SetBtObject( CBone* srcbone, CBtObject* srcbo ){ m_btobject[ srcbone ] = srcbo; };
	void ClearBtObject(){ m_btobject.clear(); };


	int GetMotMarkSize(){ return m_motmark.size(); };
	int GetMotMarkSize2( int srcindex ){ 
		std::map<double, int> curmap = m_motmark[ srcindex ];
		return curmap.size();
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

	D3DXVECTOR3 GetFirstFrameBonePos()
	{
		return m_firstframebonepos;
	};


	CModel* GetParModel(){ return m_parmodel; };
	void SetParModel( CModel* srcpar ){ m_parmodel = srcpar; };

	CBone* GetParent(){ return m_parent; };
	void SetParent( CBone* srcpar ){ m_parent = srcpar; };

	CBone* GetChild(){ return m_child; };
	void SetChild( CBone* srcchil ){ m_child = srcchil; };

	CBone* GetBrother(){ return m_brother; };
	void SetBrother( CBone* srcbro ){ m_brother = srcbro; };

private:
	int m_type;
	int m_selectflag;
	int m_bonecnt;
	int m_validflag;
	int m_boneno;
	int m_topboneflag;
	char m_bonename[256];
	WCHAR m_wbonename[256];
	char m_engbonename[256];

	D3DXVECTOR3 m_childworld;//ボーンの子供のモーション行列適用後の座標。
	D3DXVECTOR3 m_childscreen;//ボーンの子供のWVP適用後の座標。

	std::map<int, CMotionPoint*> m_motionkey;//m_motionkey[ モーションID ]でモーションの最初のフレームの姿勢にアクセスできる。
	CMotionPoint m_curmp;//現在のWVP適用後の姿勢データ。
	CQuaternion m_axisq;//ボーンの軸のクォータニオン表現。

	D3DXMATRIX m_laxismat;//Zボーンのaxismat
	D3DXMATRIX m_gaxismatXpar;//Xボーンのグローバルのaxismat
	D3DXMATRIX m_gaxismatYpar;//Yボーンのグローバルのaxismat
	D3DXMATRIX m_axismat_par;//Xボーンのローカルのaxismat

	D3DXMATRIX m_startmat2;//ワールド行列を保存しておくところ。剛体シミュレーションを始める際などに保存する。

	int m_getanimflag;//FBXファイルを読み込む際にアニメーションを読み込んだら１。
	D3DXMATRIX m_nodemat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	D3DXMATRIX m_firstmat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	D3DXMATRIX m_invfirstmat;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	D3DXVECTOR3 m_jointfpos;//ジョイントの初期位置。
	D3DXVECTOR3 m_jointwpos;//FBXにアニメーションが入っていない時のジョイントの初期位置。
	FbxAMatrix m_globalpos;//ジョイントの初期位置を計算するときに使用する。FBX読み込み時にセットして使用する。
	D3DXMATRIX m_initmat;
	D3DXMATRIX m_invinitmat;
	D3DXMATRIX m_tmpmat;//一時使用目的
	CQuaternion m_tmpq;

	D3DXVECTOR3 m_firstframebonepos;


	//CBone*は子供ジョイントのポインタ。子供の数だけエントリーがある。
	std::map<CBone*, CRigidElem*> m_rigidelem;


	//m_remapは、jointの名前でセットすればmap<string,CRigidElem*>で済む。
	//名前で一意なRigidElemを選択するために子供のボーンの名前を使用する。
	//そのため子供のボーンの名前とCRigidElemのセットを使う。
	//m_impmapについても同様のことがいえる。
	std::map<std::string, std::map<CBone*, CRigidElem*>> m_remap;//map<設定ファイル名, map<子供ボーン, 剛体設定>>
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>> m_impmap;//map<設定ファイル名, map<子供ボーン, インパルス設定>>


	int m_btkinflag;//bullet kinematic flag。剛体シミュレーションの根元のボーンが固定モーションに追随する際は０を指定する。その他は１。
	int m_btforce;//bullet 強制フラグ。モデルに設定されているbtkinflagである。m_btkinflagはシミュレーション開始と終了で変化するが、このフラグは読み込み時に設定されるものである。
	std::map<CBone*,CBtObject*> m_btobject;//CBtObjectはbulletの剛体オブジェクトをラップしたクラスである。ボーンとCBtObjectのmap。

	//タイムラインのモーション編集マーク
	//最初のintはmotid。次のmapはframenoと更新フラグ。更新フラグは読み込み時のマークは０、それ以後の編集マークは１にする予定。色を変えるかも。
	std::map<int, std::map<double,int>> m_motmark;

	CModel* m_parmodel;

	CBone* m_parent;
	CBone* m_child;
	CBone* m_brother;
};

#endif