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
class CInfScope;
class CMotionPoint;
class CRigidElem;
class CBtObject;
class CModel;

class CBone
{
public:
	CBone( CModel* parmodel );
	~CBone();

	int SetName( char* srcname );
	int SetBoneTri( CMQOFace* srcface, D3DXVECTOR3* srcpoint );
	int AddChild( CBone* childptr );


	int AddInfScope( CInfScope* pinfscope );
	int CalcTargetCenter();

	int UpdateMatrix( int srcmotid, double srcframe, D3DXMATRIX* wmat, D3DXMATRIX* vpmat );

	int MakeFirstMP( int motid );
	CMotionPoint* AddMotionPoint( int srcmotid, double srcframe, int calcflag, int* existptr );

	int CalcFBXMotion( int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr );
	int CalcFBXFrame( double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr );

	int GetBefNextMP( int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr );
	int OrderMotionPoint( int srcmotid, double settime, CMotionPoint* putmp, int samedel );

	int DeleteMotion( int srcmotid );

	int DeleteMPOutOfRange( int motid, double srcleng );

	CBone* GetParbone( int searchnull );

	int CalcRigidElemParams( CModel* cpslptr[COL_MAX], CBone* chilbone, int setstartflag );
	int CalcAxisMat( int firstflag, float delta );
	int CalcAxisMatZ( D3DXVECTOR3* srccur, D3DXVECTOR3* srcchil );

	int CreateRigidElem( CBone* chil, int reflag, std::string rename, int impflag, std::string impname );
	int SetCurrentRigidElem( std::string curname );

	CMotionPoint* AddBoneEulReq( CMotionPoint* parmp, int srcmotid, double srcframe, D3DXMATRIX rotmat );
	CMotionPoint* AddBoneTraReq( CMotionPoint* parmp, int srcmotid, double srcframe, D3DXVECTOR3 srctra );
	CMotionPoint* RotBoneQReq( CMotionPoint* parmp, int srcmotid, double srcframe, CQuaternion rotq );
	CMotionPoint* PasteRotReq( int srcmotid, double srcframe, double dstframe );

	CMotionPoint* SetAbsMatReq( int broflag, int srcmotid, double srcframe, double firstframe );


	int DestroyMotionKey( int srcmotid );

	int AddBoneMotMark( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag );
	int CalcLocalInfo( int motid, double frameno, CMotionPoint** ppdstmp );
	int GetBoneNum();

private:
	int InitParams();
	int DestroyObjs();

	//int CalcAxisMatX( int parchilflag, D3DXMATRIX* dstmat );
	int CalcAxisMatX();
	int CalcAxisMatY( CBone* chilbone, D3DXMATRIX* dstmat );

	int CalcAxisMatX_aft( D3DXVECTOR3 curpos, D3DXVECTOR3 chilpos, D3DXMATRIX* destmat );
	int CalcLocalAxisMat( D3DXMATRIX motmat, D3DXMATRIX axismatpar, D3DXMATRIX gaxisy );

	int SetGroupNoByName( CRigidElem* curre, CBone* chilbone );
	int AddBoneMarkIfNot( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double curframe, int flag );
	int DelBoneMarkRange( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag );

public:
	int m_bonecnt;
	int m_validflag;
	int m_boneno;
	int m_topboneflag;
	char m_bonename[256];
	WCHAR m_wbonename[256];
	char m_engbonename[256];

	int m_bonematerialno;
	D3DXVECTOR3 m_vertpos[BT_MAX];
	CMQOFace* m_faceptr;

	D3DXVECTOR3 m_childworld;
	D3DXVECTOR3 m_childscreen;

	int m_isnum;
	CInfScope* m_isarray[ INFSCOPEMAX ];

	std::map<int, CMotionPoint*> m_motionkey;
	CMotionPoint m_curmp;
	CQuaternion m_axisq;
	D3DXMATRIX m_laxismat;//Zボーンのaxismat
	D3DXMATRIX m_gaxismatXpar;//Xボーンのaxismat
	D3DXMATRIX m_gaxismatYpar;//Yボーンのaxismat
	D3DXMATRIX m_axismat_par;
	D3DXMATRIX m_axismat_parY;

	D3DXMATRIX m_startmat2;
	D3DXVECTOR3 m_3rdvec;
	int m_3rdvecflag;

	int m_selectflag;

	D3DXMATRIX m_xoffsetmat;
	D3DXMATRIX m_xtransmat;
	D3DXMATRIX m_xconbinedtransmat;

	CQuaternion m_kinectq;

	CBone* m_parent;
	CBone* m_child;
	CBone* m_brother;

	D3DXMATRIX m_initmat;
	D3DXMATRIX m_invinitmat;
	D3DXMATRIX m_totalinitmat;

	int m_getanimflag;
	int m_type;
	D3DXMATRIX m_nodemat;
	D3DXMATRIX m_firstmat;
	D3DXMATRIX m_invfirstmat;

	D3DXVECTOR3 m_jointwpos;
	D3DXVECTOR3 m_jointfpos;

	FbxAMatrix m_globalpos;

	std::map<CBone*, CRigidElem*> m_rigidelem;
	std::map<std::string, std::map<CBone*, CRigidElem*>> m_remap;
	std::map<std::string, std::map<CBone*, D3DXVECTOR3>> m_impmap;

	int m_btkinflag;
	int m_btforce;

	D3DXMATRIX m_curfirstmat;

	D3DXMATRIX m_befworldmat;

	std::map<CBone*,CBtObject*> m_btobject;
	std::map<int, std::map<double,int>> m_motmark;//最初のintはmotid。次のmapはframenoと更新フラグ。更新フラグは読み込み時のマークは０、それ以後の編集マークは１にする予定。色を変えるかも。

	CModel* m_parmodel;
};

#endif