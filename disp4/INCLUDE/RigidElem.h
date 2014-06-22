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

public:
	int m_coltype;
	int m_skipflag;
	bool m_bcone;
	float m_shprate;
	float m_boxzrate;

	float m_mass;

//	D3DXVECTOR3 m_impulse;

	float m_damping;//Œ¸Š—¦
	int m_l_kindex;//ˆÚ“®‚Î‚Ë’è”(e‚Æ‚ÌÚ‘±)
	int m_a_kindex;//‰ñ“]‚Î‚Ë’è”

	float m_cus_lk;
	float m_cus_ak;

	float m_l_damping;//ˆÚ“®Œ¸Š—¦
	float m_a_damping;//‰ñ“]Œ¸Š—¦

	D3DXMATRIX m_capsulemat;
	D3DXMATRIX m_firstcapsulemat;
	float m_cylileng;
	float m_sphr;
	float m_boxz;

	CBone* m_bone;
	CBone* m_endbone;

	int m_colgflag;

	int m_groupid;
	std::vector<int> m_coliids;
	int m_myselfflag;

	float m_restitution;
	float m_friction;

	float m_btg;
	float m_dampanim_l;
	float m_dampanim_a;

};

#endif
