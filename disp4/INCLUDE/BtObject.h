#ifndef BTOBJECTH
#define BTOBJECTH

#include <d3dx9.h>
#include <wchar.h>
#include <Coef.h>
#include <string>
#include <map>
#include <vector>

#include <ConstraintElem.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

class CModel;
class CBone;
class CRigidElem;

class CBtObject
{
public:

/**
 * @fn
 * CBtObject
 * @breaf CBtObjectのコンストラクタ
 * @param (CBtObject* parbt) IN 親のCBtObjectへのポインタ。
 * @param (btDynamicsWorld* btWorld) IN bulletのbtDynamicsWorldへのポインタ。
 * @return なし。
 */
	CBtObject( CBtObject* parbt, btDynamicsWorld* btWorld );

/**
 * @fn
 * ~CBtObject
 * @breaf CBtObjectのデストラクタ。
 * @return なし。
 */
	~CBtObject();


/**
 * @fn
 * CreateObject
 * @breaf bulletの剛体オブジェクトを作成し設定する。
 * @param (CBtObject* parbt) IN 親のCBtObjectへのポインタ。
 * @param (CBone* parbone) IN 剛体の親側のボーンのそのまた親のボーン。
 * @param (CBone* curbone) IN 剛体の親側のボーン。
 * @param (CBone* chilbone) IN 剛体の子供側のボーン。
 * @return 成功したら０。
 */
	int CreateObject( CBtObject* parbt, CBone* parbone, CBone* curbone, CBone* chilbone );

/**
 * @fn
 * CreateBtConstraint
 * @breaf この剛体と接続する剛体との間のコンストレイントを作成する。
 * @return 成功したら０。
 */
	int CreateBtConstraint();


/**
 * @fn
 * SetBtMotion
 * @breaf 剛体シミュレーションの行列からシミュレーション適用後の行列を計算する。
 * @return 成功したら０。
 */
	int SetBtMotion();

	int SetCapsuleBtMotion(CRigidElem* srcre);



/**
 * @fn
 * Motion2Bt
 * @breaf 剛体シミュレーションのための設定をする。既存モーションからの変化分をスタート時の行列に掛ける。
 * @return 成功したら０。
 * @detail Motion2Btを呼んでからSetBtMotionを呼ぶ。
 */
	int Motion2Bt(CModel* srcmodel);

/**
 * @fn
 * AddChild
 * @breaf 剛体の階層構造を設定する。
 * @param (CBtObject* addbt) IN 子供にするCBtObjectへのポインタ。
 * @return 成功したら０。
 * @detail 子供の配列に加えるだけ。broは無い。
 */
	int AddChild(CBtObject* addbt);



	int SetEquilibriumPoint( int lflag, int aflag );
	int EnableSpring(bool angleflag, bool linearflag);
	int SetDofRotAxis(int srcaxiskind);


	int CreatePhysicsPosConstraint();
	int DestroyPhysicsPosConstraint();

private:

/**
 * @fn
 * InitParams
 * @breaf メンバを初期化する。
 * @return ０を返す。
 */
	int InitParams();

/**
 * @fn
 * DestroyObjs
 * @breaf アロケートしたメモリを開放する。
 * @return ０を返す。
 */
	int DestroyObjs();

/**
 * @fn
 * localCreateRigidBody
 * @breaf bulletのRigidBodyを作成する。CreateObjectから呼ばれる。
 * @param (CRigidElem* curre) IN 剛体設定パラメータクラスCRigidElemへのポインタ。
 * @param (const btTransform& startTransform) IN 剛体を初期位置に設定するための変換行列。
 * @param (btCollisionShape* shape) IN 剛体のあたり判定用オブジェクト。
 * @return 作成したbtRigidBodyへのポインタ。
 */
	btRigidBody* localCreateRigidBody( CRigidElem* curre, const btTransform& startTransform, btCollisionShape* shape );



/**
 * @fn
 * CalcConstraintTransform
 * @breaf コンストレイントを初期位置に置くための変換行列を計算する。
 * @param (int chilflag) IN 剛体の親側の場合は０、子供側の場合は１．
 * @param (CRigidElem* curre) IN 剛体パラメータ。
 * @param (CBtObject* curbto) IN 自分自身、または子供側の剛体オブジェクト。
 * @param (btTransform& dstmat) OUT 変換行列。
 * @return 戻り値の説明
 * @detail 親側のコンストレイントと子供側のコンストレイントと別々に両方呼び出す。
 */
	int CalcConstraintTransform( int chilflag, CRigidElem* curre, CBtObject* curbto, btTransform& dstmat );


	void DestroyGZObj();


public: //accesser
	btDynamicsWorld* GetbtWorld(){ return m_btWorld; };
	void SetBtWorld( btDynamicsWorld* srcworld ){ m_btWorld = srcworld; };

	int GetTopFlag(){ return m_topflag; };
	void SetTopFlag( int srcflag ){ m_topflag = srcflag; };

	float GetBoneLeng(){ return m_boneleng; };
	void SetBoneLeng( float srcleng ){ m_boneleng = srcleng; };

	D3DXMATRIX GetTransMat(){ return m_transmat; };
	void SetTransMat( D3DXMATRIX srcmat ){ m_transmat = srcmat; };

	D3DXMATRIX GetPar2Cen(){ return m_par2cen; };
	void SetPar2Cen( D3DXMATRIX srcvec ){ m_par2cen = srcvec; };

	D3DXMATRIX GetCen2ParY(){ return m_cen2parY; };
	void SetCen2ParY( D3DXMATRIX srcvec ){ m_cen2parY = srcvec; };

	CBone* GetBone(){ return m_bone; };
	void SetBone( CBone* srcbone ){ m_bone = srcbone; };

	CBone* GetParBone(){ return m_parbone; };
	void SetParBone( CBone* srcbone ){ m_parbone = srcbone; };

	CBone* GetEndBone(){ return m_endbone; };
	void SetEndBone( CBone* srcbone ){ m_endbone = srcbone; };

	btCollisionShape* GetColShape(){ return m_colshape; };
	void SetColShape( btCollisionShape* srcshape ){ m_colshape = srcshape; };

	btRigidBody* GetRigidBody(){ return m_rigidbody; };
	void SetRigidBody( btRigidBody* srcbody ){ m_rigidbody = srcbody; };


	CBtObject* GetParBt(){ return m_parbt; };
	void SetParBt( CBtObject* srcbt ){ m_parbt = srcbt; };

	int GetChilBtSize(){ return (int)m_chilbt.size(); };
	void PushBackChilBt( CBtObject* srcchil ){ m_chilbt.push_back( srcchil ); };
	CBtObject* GetChilBt( int srcindex ){ return m_chilbt[ srcindex ]; };
	void CopyChilBt( std::vector<CBtObject*>& dstbt ){ dstbt = m_chilbt; };


	float GetConstZRad(){ return m_constzrad; };
	void SetConstZRad( float srcval ){ m_constzrad = srcval; };

	int GetConnectFlag(){ return m_connectflag; };
	void SetConnectFlag( int srcflag ){ m_connectflag = srcflag; };

	void GetCurPivot( btVector3& dstpivot ){ dstpivot = m_curpivot; };
	void SetCurPivot( btVector3& srcpivot ){ m_curpivot = srcpivot; };

	void GetChilPivot( btVector3& dstpivot ){ dstpivot = m_chilpivot; };
	void SetChilPivot( btVector3& srcpivot ){ m_chilpivot = srcpivot; };

	void GetFrameA( btTransform& dstframe ){ dstframe = m_FrameA; };
	void SetFrameA( btTransform& srcval ){ m_FrameA = srcval; };

	void GetFrameB( btTransform& dstframe ){ dstframe = m_FrameB; };
	void SetFrameB( btTransform& srcval ){ m_FrameB = srcval; };


	D3DXMATRIX GetXWorld(){ return m_xworld; };
	void SetXWorld( D3DXMATRIX srcworld ){ m_xworld = srcworld; };

	int GetConstraintSize(){
		return (int)m_constraint.size();
	};
	void PushBackConstraint( CONSTRAINTELEM srcconstraint ){ m_constraint.push_back( srcconstraint ); };
	btGeneric6DofSpringConstraint* GetConstraint(int srcindex){
		if ((srcindex >= 0) && (srcindex < (int)m_constraint.size())){
			return m_constraint[srcindex].constraint;
		}
		else{
			//CONSTRAINTELEM initelem;
			//ZeroMemory(&initelem, sizeof(CONSTRAINTELEM));
			//return initelem;
			return 0;
		}
	};
	CONSTRAINTELEM GetConstraintElem(int srcindex){
		if ((srcindex >= 0) && (srcindex < (int)m_constraint.size())){
			return m_constraint[srcindex];
		}
		else{
			CONSTRAINTELEM initelem;
			ZeroMemory(&initelem, sizeof(CONSTRAINTELEM));
			return initelem;
		}
	};

	D3DXVECTOR3 GetBtPos()
	{
		return m_btpos;
	}

private:
	btDynamicsWorld* m_btWorld;

	int m_topflag;
	float m_boneleng;

	D3DXMATRIX m_transmat;//Y軸平行な剛体をボーンに合わせるための変換行列。
	D3DXMATRIX m_par2cen;//parent to centerのベクトル。
	D3DXMATRIX m_cen2parY;//center to parentのベクトル。剛体の初期状態がY軸に平行と仮定。
	D3DXMATRIX m_xworld;//bulletの剛体に設定された変換行列。

	CBone* m_bone;//剛体の親側のボーン
	CBone* m_parbone;//m_boneの親のボーン
	CBone* m_endbone;//剛体の子供側のボーン


	btCollisionShape* m_colshape;//bulletのあたり判定形状データ。
	btRigidBody* m_rigidbody;//ブレットの剛体データ。	
	std::vector<CONSTRAINTELEM> m_constraint;//thisと子供のBtObjectをつなぐコンストレイントのvector。
	
	btCollisionShape* m_gz_colshape;//bulletのあたり判定形状データ。
	btRigidBody* m_gz_rigidbody;//ブレットの剛体データ。	
	CONSTRAINTELEM m_gz_constraint;//thisと子供のBtObjectをつなぐコンストレイントのvector。



	CBtObject* m_parbt;//親のCBtObject
	std::vector<CBtObject*> m_chilbt;//子供のCBtObject

	float m_constzrad;//Constraintの軸の傾き(Z軸)
	int m_connectflag;//bunki剛体と同位置の剛体をつないだかどうかのフラグ。

	btVector3 m_curpivot;//剛体の親側の支点
	btVector3 m_chilpivot;//剛体の子供側の支点

	btTransform m_FrameA;//剛体設定時のA側変換行列。
	btTransform m_FrameB;//剛体設定時のB側変換行列。


	D3DXVECTOR3 m_btpos;//Motion2Btで計算した剛体の位置
};


#endif