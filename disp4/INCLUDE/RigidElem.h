#ifndef RIGIDELEMH
#define RIGIDELEMH

#include <d3dx9.h>
#include <wchar.h>
#include <Coef.h>
#include <string>
#include <map>
#include <vector>

#include <Bone.h>

class CRigidElem
{
public:
	CRigidElem();
	~CRigidElem();

	int GetColiID();


private:
	int InitParams();
	int DestroyObjs();

public: //accesser
	int GetColtype(){ return m_coltype; };
	void SetColtype( int srctype ){ m_coltype = srctype; };

	int GetSkipflag(){ return m_skipflag; };
	void SetSkipflag( int srcflag ){ m_skipflag = srcflag; };

	float GetMass(){ return m_mass; };
	void SetMass( float srcmass ){ m_mass = srcmass; };

	float GetSphr(){ return m_sphr; };
	void SetSphr( float srcsphr ){ m_sphr = srcsphr; };

	float GetSphrate(){ return m_sphrate; };
	void SetSphrate( float srcrate ){ m_sphrate = srcrate; };

	float GetBoxz(){ return m_boxz; };
	void SetBoxz( float srcboxz ){ m_boxz = srcboxz; };

	float GetBoxzrate(){ return m_boxzrate; };
	void SetBoxzrate( float srcrate ){ m_boxzrate = srcrate; };

	float GetCylileng(){ return m_cylileng; };
	void SetCylileng( float srcleng ){ m_cylileng = srcleng; };

	int GetLKindex(){ return m_l_kindex; };
	void SetLKindex( int srcindex ){ m_l_kindex = srcindex; };

	int GetAKindex(){ return m_a_kindex; };
	void SetAKindex( int srcindex ){ m_a_kindex = srcindex; };

	float GetCusLk(){ return m_cus_lk; };
	void SetCusLk( float srck ){ m_cus_lk = srck; };

	float GetCusAk(){ return m_cus_ak; };
	void SetCusAk( float srck ){ m_cus_ak = srck; };

	float GetLDamping(){ return m_l_damping; };
	void SetLDamping( float srcd ){ m_l_damping = srcd; };

	float GetADamping(){ return m_a_damping; };
	void SetADamping( float srcd ){ m_a_damping = srcd; };

	float GetDampanimL(){ return m_dampanim_l; };
	void SetDampanimL( float srcd ){ m_dampanim_l = srcd; };

	float GetDampanimA(){ return m_dampanim_a; };
	void SetDampanimA( float srcd ){ m_dampanim_a = srcd; };

	float GetBtg(){ return m_btg; };
	void SetBtg( float srcg ){ m_btg = srcg; };

	int GetGroupid(){ return m_groupid; };
	void SetGroupid( int srcid ){ m_groupid = srcid; };

	int GetColiidsSize(){ return (int)m_coliids.size(); };
	int GetColiids( int srcindex ){ return m_coliids[ srcindex ]; };
	void PushBackColiids( int srcid ){ m_coliids.push_back( srcid ); };
	void CopyColiids( CRigidElem* srcre ){ m_coliids = srcre->m_coliids; };
	void CopyColiids( std::vector<int>& srcids ){ m_coliids = srcids; };
	void CopyColiids2Dstvec( std::vector<int>& dstids ){ dstids = m_coliids; };


	int GetMyselfflag(){ return m_myselfflag; };
	void SetMyselfflag( int srcflag ){ m_myselfflag = srcflag; };

	float GetRestitution(){ return m_restitution; };
	void SetRestitution( float srcval ){ m_restitution = srcval; };

	float GetFriction(){ return m_friction; };
	void SetFriction( float srcval ){ m_friction = srcval; };

	ChaMatrix GetCapsulemat(){ return m_capsulemat; };
	void SetCapsulemat( ChaMatrix srcmat ){ m_capsulemat = srcmat; };

	ChaMatrix GetFirstcapsulemat(){ return m_firstcapsulemat; };
	void SetFirstcapsulemat( ChaMatrix srcmat ){ m_firstcapsulemat = srcmat; };

	ChaMatrix GetFirstWorldmat() { return m_firstworldmat; };
	ChaMatrix GetInvFirstWorldmat() { ChaMatrix retmat; ChaMatrixInverse(&retmat, NULL, &m_firstworldmat); return retmat; };
	void SetFirstWorldmat(ChaMatrix srcmat){ m_firstworldmat = srcmat; };

	CBone* GetBone(){ return m_bone; };
	void SetBone( CBone* srcbone ){ m_bone = srcbone; };

	CBone* GetEndbone(){ return m_endbone; };
	void SetEndbone( CBone* srcbone ){ m_endbone = srcbone; };

	float GetBoneLeng();

	int GetForbidRotFlag(){ return m_forbidrotflag; };
	void SetForbidRotFlag(int srcflag){
		if ((srcflag == 0) || (srcflag == 1)){
			m_forbidrotflag = srcflag;
		}
		else{
			m_forbidrotflag = 0;
		}
	};

private:
	int m_coltype;
	int m_skipflag;

	float m_boneleng;
	float m_mass;//質量

	float m_sphr;//球半径
	float m_sphrate;//球半径比率、つまり楕円の率
	float m_boxz;//箱の辺の長さ
	float m_boxzrate;//直方体の辺の比率
	float m_cylileng;//シリンダーの長さ

	int m_l_kindex;//移動ばね定数(親との接続)のプリセット用インデックス
	int m_a_kindex;//回転ばね定数のプリセット用インデックス

	float m_cus_lk;//カスタム時の移動ばね定数
	float m_cus_ak;//カスタム時の回転ばね定数

	float m_l_damping;//移動減衰率
	float m_a_damping;//回転減衰率
	float m_dampanim_l;
	float m_dampanim_a;

	float m_btg;//ラグドール時の重力

	int m_groupid;//コリジョン用　自分のグループID
	std::vector<int> m_coliids;//コリジョン用　衝突判定するグループIDの配列
	int m_myselfflag;//コリジョン用　自分と衝突判定するかどうかのフラグ

	float m_restitution;
	float m_friction;

	int m_forbidrotflag;

	ChaMatrix m_capsulemat;
	ChaMatrix m_firstcapsulemat;
	ChaMatrix m_firstworldmat;//m_firstcapsulemat設定時のworldmat


	CBone* m_bone;
	CBone* m_endbone;
};

#endif
