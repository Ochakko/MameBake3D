#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <GlobalVar.h>

#include <Model.h>
#include <mqomaterial.h>
#include <mqoobject.h>

#define DBGH
#include <dbg.h>

#include <Bone.h>
#include <MQOFace.h>
#include <BtObject.h>

#include <InfScope.h>
#include <MotionPoint.h>

#include <ChaVecCalc.h>

#include <RigidElem.h>
#include <EngName.h>
//#include <BoneProp.h>


//for __nop()
#include <intrin.h>


using namespace std;
using namespace OrgWinGUI;


//制限角度に遊びを設ける
//#define EULLIMITPLAY	5

//大きすぎ？ギザギザの原因？
//#define EULLIMITPLAY	2

//2023/02/12
#define EULLIMITPLAY	1

map<CModel*,int> g_bonecntmap;
/*
extern WCHAR g_basedir[MAX_PATH];
extern int g_boneaxis;
extern bool g_limitdegflag;
extern bool g_wmatDirectSetFlag;
extern bool g_underRetargetFlag;
extern int g_previewFlag;
*/
//global
void InitCustomRig(CUSTOMRIG* dstcr, CBone* parentbone, int rigno);
int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parentbone);
//void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* childbone);
int IsValidRigElem(CModel* srcmodel, RIGELEM srcrigelem);


static std::vector<CBone*> s_bonepool;//allocate BONEPOOLBLKLEN motoinpoints at onse and pool 



void InitCustomRig(CUSTOMRIG* dstcr, CBone* parentbone, int rigno)
{
	//ZeroMemory(dstcr, sizeof(CUSTOMRIG));
	if (!dstcr) {
		_ASSERT(0);
		return;
	}
	dstcr->Init();

	dstcr->rigboneno = -1;
	int rigelemno;
	for (rigelemno = 0; rigelemno < MAXRIGELEMNUM; rigelemno++) {
		dstcr->rigelem[rigelemno].boneno = -1;
		dstcr->rigelem[rigelemno].rigrigboneno = -1;
		dstcr->rigelem[rigelemno].rigrigno = -1;
		int uvno;
		for (uvno = 0; uvno < 2; uvno++) {
			dstcr->rigelem[rigelemno].transuv[uvno].enable = 1;
		}
	}


	if ((rigno >= 0) && (rigno < MAXRIGNUM)) {
		dstcr->rigno = rigno;
		if (parentbone) {
			swprintf_s(dstcr->rigname, 256, L"%s_Rig%d", parentbone->GetWBoneName(), rigno);
		}
		else {
			swprintf_s(dstcr->rigname, 256, L"Rig%d", rigno);
		}
	}
	else {
		_ASSERT(0);
		dstcr->rigno = 0;
		swprintf_s(dstcr->rigname, 256, L"RigName_00");
	}

	if (parentbone) {
		dstcr->rigboneno = parentbone->GetBoneNo();
		dstcr->elemnum = 1;
		dstcr->rigelem[0].boneno = parentbone->GetBoneNo();
	}
}


int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parentbone)
{
	/*
	typedef struct tag_rigtrans
	{
	int axiskind;
	float applyrate;
	}RIGTRANS;

	typedef struct tag_rigelem
	{
	int boneno;
	RIGTRANS transuv[2];
	}RIGELEM;

	typedef struct tag_customrig
	{
	int useflag;//0 : free, 1 : rental, 2 : valid and in use
	int rigno;//CUSTOMRIGを配列で持つ側のためのCUSTOMRIGのindex
	int rigboneno;
	int elemnum;
	RIGELEM rigelem[4];
	}CUSTOMRIG;
	*/
	if (!parentbone) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。ownerbone NULL");
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		return 0;
	}

	if (parentbone && (srccr.rigboneno != parentbone->GetBoneNo())) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。rigboneno : %d", srccr.rigboneno);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.rigno < 0) || (srccr.rigno >= MAXRIGNUM)) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。rigno : %d", srccr.rigno);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.elemnum < 1) || (srccr.elemnum > MAXRIGELEMNUM)) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。elemnum : %d", srccr.elemnum);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}

	if ((srccr.dispaxis < 0) || (srccr.dispaxis > 2)) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。dispaxis : %d", srccr.dispaxis);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.disporder < 0) || (srccr.disporder > RIGPOSINDEXMAX)) {//INDEXMAXは有り
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。disporder : %d", srccr.disporder);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.shapemult < 0) || (srccr.shapemult > RIGMULTINDEXMAX)) {//INDEXMAXは有り
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。shapemult : %d", srccr.shapemult);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.shapekind < 0) || (srccr.shapekind >= RIGSHAPE_MAX)) {//MAXは無し
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。shapekind : %d", srccr.shapekind);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}
	if ((srccr.rigcolor < 0) || (srccr.rigcolor >= RIGCOLOR_MAX)) {//MAXは無し
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。rigcolor : %d", srccr.rigcolor);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		_ASSERT(0);
		return 0;
	}


	int elemno;
	for (elemno = 0; elemno < srccr.elemnum; elemno++) {
		RIGELEM currigelem = srccr.rigelem[elemno];
		int isvalid = IsValidRigElem(srcmodel, currigelem);
		if (isvalid == 0) {
			WCHAR strerr[256];
			swprintf_s(strerr, 256, L"エラー。bonename %s, elem %d", parentbone->GetWBoneName(), elemno);
			::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
			_ASSERT(0);
			return 0;//!!!!!!!!!!!!!
		}
	}

	return 1;
}


int IsValidRigElem(CModel* srcmodel, RIGELEM srcrigelem)
{
	if (srcrigelem.rigrigboneno >= 0) {
		CBone* ownerbone = srcmodel->GetBoneByID(srcrigelem.rigrigboneno);
		if (ownerbone) {
			CUSTOMRIG curcr = ownerbone->GetCustomRig(srcrigelem.rigrigno);
			int isvalid = IsValidCustomRig(srcmodel, curcr, ownerbone);
			if (isvalid == 0) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。ownerbone %s, rigrigno %d", ownerbone->GetWBoneName(), srcrigelem.rigrigno);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				_ASSERT(0);
				return 0;
			}
		}
		else {
			WCHAR strerr[256];
			swprintf_s(strerr, 256, L"エラー。ownerbone NULL");
			::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
			_ASSERT(0);
			return 0;
		}
	}
	else {
		CBone* chkbone = srcmodel->GetBoneByID(srcrigelem.boneno);
		if (!chkbone) {
			WCHAR strerr[256];
			swprintf_s(strerr, 256, L"エラー。boneno : %d", srcrigelem.boneno);
			::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
			_ASSERT(0);
			return 0;
		}

		int uvno;
		for (uvno = 0; uvno < 2; uvno++) {
			RIGTRANS currigtrans = srcrigelem.transuv[uvno];
			if ((currigtrans.axiskind < AXIS_X) || (currigtrans.axiskind > AXIS_Z)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : axiskind : %d", uvno, currigtrans.axiskind);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				_ASSERT(0);
				return 0;
			}
			if ((currigtrans.applyrate < -100.0f) || (currigtrans.applyrate > 100.0f)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : applyrate : %f", uvno, currigtrans.applyrate);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				_ASSERT(0);
				return 0;
			}
			if ((currigtrans.enable != 0) && (currigtrans.enable != 1)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : enable : %d", uvno, currigtrans.enable);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				_ASSERT(0);
				return 0;
			}
		}
	}

	return 1;
}




//class


CBone::CBone( CModel* parmodel )// : m_curmp(), m_axisq()
{
	InitializeCriticalSection(&m_CritSection_AddMP);
	InitializeCriticalSection(&m_CritSection_GetBefNext);
	InitParams();
	SetParams(parmodel);
}

CBone::~CBone()
{
	DeleteCriticalSection(&m_CritSection_AddMP);
	DeleteCriticalSection(&m_CritSection_GetBefNext);
	DestroyObjs();
}

int CBone::InitParams()
{
	//not use at allocated
	m_useflag = 0;//0: not use, 1: in use
	m_indexofpool = 0;
	m_allocheadflag = 0;//1: head pointer at allocated

	m_extendflag = false;

	m_fbxnodeonload = 0;//2022/11/01

	m_curmp.InitParams();
	m_calccurmp.InitParams();
	m_axisq.InitParams();

	m_motionkey.clear();
	m_motionkey[0] = 0;

	m_addlimitq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);

	ChaMatrixIdentity(&m_localS0);
	m_localR0.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	ChaMatrixIdentity(&m_localT0);
	ChaMatrixIdentity(&m_firstSRT);
	ChaMatrixIdentity(&m_firstGlobalSRT);


	m_tmpkinematic = false;
	//m_curmotid = -1;
	m_curmotid = 0;//有効なidは１から
	//m_excludemv = 0;
	//m_mass0 = 0;
	//m_posconstraint = 0;
	ZeroMemory(m_coldisp, sizeof(CModel*)* COL_MAX);

	ChaMatrixIdentity(&m_tmpsymmat);

	ChaMatrixIdentity(&m_btmat);
	ChaMatrixIdentity(&m_befbtmat);
	m_setbtflag = 0;
	m_bteul = ChaVector3(0.0f, 0.0f, 0.0f);

	m_btparentpos = ChaVector3(0.0f, 0.0f, 0.0f);
	m_btchildpos = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaMatrixIdentity(&m_btdiffmat);

	m_initcustomrigflag = 0;
	//InitCustomRig();//<-- after set name

	InitAngleLimit();
	m_upkind = UPVEC_NONE;
	m_motmark.clear();

	m_parmodel = 0;
	m_validflag = 1;
	m_rigidelemname.clear();
	m_btobject.clear();
	m_btkinflag = 1;
	m_btforce = 0;

	m_globalpos.SetIdentity();
	ChaMatrixIdentity( &m_invfirstmat );
	ChaMatrixIdentity( &m_firstmat );
	//ChaMatrixIdentity( &m_startmat2 );
	ChaMatrixIdentity( &m_axismat_par );
	ChaMatrixIdentity(&m_initmat);
	ChaMatrixIdentity(&m_invinitmat);
	ChaMatrixIdentity(&m_tmpmat);
	//ChaMatrixIdentity(&m_firstaxismatX);
	ChaMatrixIdentity(&m_firstaxismatZ);

	m_boneno = 0;
	m_topboneflag = 0;
	ZeroMemory( m_bonename, sizeof( char ) * 256 );
	ZeroMemory( m_wbonename, sizeof( WCHAR ) * 256 );
	ZeroMemory( m_engbonename, sizeof( char ) * 256 );

	m_childworld = ChaVector3( 0.0f, 0.0f, 0.0f );
	m_childscreen = ChaVector3( 0.0f, 0.0f, 0.0f );

	m_parent = 0;
	m_child = 0;
	m_brother = 0;

	m_selectflag = 0;

	m_getanimflag = 0;

	m_type = FBXBONE_NONE;

	ChaMatrixIdentity(&m_nodemat);
	ChaMatrixIdentity(&m_nodeanimmat);
	m_bindmat.SetIdentity();

	m_jointwpos = ChaVector3( 0.0f, 0.0f, 0.0f );
	m_jointfpos = ChaVector3( 0.0f, 0.0f, 0.0f );
	m_oldjointfpos = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaMatrixIdentity( &m_laxismat );
	//ChaMatrixIdentity( &m_gaxismatXpar );

	m_remap.clear();
	m_impmap.clear();

	m_tmpq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);

	m_firstframebonepos = ChaVector3(0.0f, 0.0f, 0.0f);

	m_posefoundflag = false;
	ZeroMemory(m_cachebefmp, sizeof(CMotionPoint*) * (MAXMOTIONNUM + 1));

	m_firstgetflag = 0;//GetCurrentZeroFrameMat用
	ChaMatrixIdentity(&m_firstgetmatrix);//GetCurrentZeroFrameMat用
	ChaMatrixIdentity(&m_invfirstgetmatrix);//GetCurrentZeroFrameMat用

	m_indexedmotionpoint.clear();
	m_initindexedmotionpoint.clear();
	m_dummymp.InitParams();


	m_befupdatetime = -1.0;
	m_skipRenderBoneMark = false;

	m_fbxLclPos = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxRotOff = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxRotPiv = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxPreRot = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxLclRot = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxPostRot = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxSclOff = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxSclPiv = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxLclScl = FbxDouble3(0.0, 0.0, 0.0);
	m_fbxrotationActive = false;

	m_ikstopflag = false;
	m_iktargetflag = false;
	m_iktargetpos = ChaVector3(0.0f, 0.0f, 0.0f);

	m_ikrotrec.clear();
	m_ikrotrec_u.clear();
	m_ikrotrec_v.clear();

	return 0;
}

int CBone::InitParamsForReUse(CModel* srcparmodel)
{
	int saveboneno = m_boneno;
	CModel* saveparmodel = m_parmodel;
	int saveindex = GetIndexOfPool();
	int saveallochead = IsAllocHead();

	DestroyObjs();
	InitParams();

	m_parmodel = saveparmodel;
	m_boneno = saveboneno;

	if (m_parmodel == 0) {//!!!!! モデルごと削除されたボーンの再利用
		SetParams(srcparmodel);//m_parmodel, m_boneno
	}


	m_firstcalcrigid = true;

	SetIndexOfPool(saveindex);
	SetIsAllocHead(saveallochead);
	SetUseFlag(1);

	return 0;
}


int CBone::SetParams(CModel* parmodel)
{
	m_parmodel = parmodel;
	//_ASSERT(m_parmodel);

	map<CModel*, int>::iterator itrcnt;
	itrcnt = g_bonecntmap.find(m_parmodel);
	if (itrcnt == g_bonecntmap.end()) {
		g_bonecntmap[m_parmodel] = 0;
	}

	int curno = g_bonecntmap[m_parmodel];
	m_boneno = curno;
	g_bonecntmap[m_parmodel] = m_boneno + 1;

	m_firstcalcrigid = true;

	return 0;
}

int CBone::InitCustomRig()
{
	int rigno;
	for (rigno = 0; rigno < MAXRIGNUM; rigno++){
		::InitCustomRig(&(m_customrig[rigno]), this, rigno);
	}
	return 0;
}

void CBone::InitAngleLimit()
{
	::InitAngleLimit(&m_anglelimit);
}



int CBone::DestroyObjs()
{
	ZeroMemory(m_cachebefmp, sizeof(CMotionPoint*) * (MAXMOTIONNUM + 1));

	int colindex;
	for (colindex = 0; colindex < COL_MAX; colindex++){
		CModel* curcol = m_coldisp[colindex];
		if (curcol){
			delete curcol;
			m_coldisp[colindex] = 0;
		}
	}



	m_motmark.clear();

	map<int, CMotionPoint*>::iterator itrmp;
	for( itrmp = m_motionkey.begin(); itrmp != m_motionkey.end(); itrmp++ ){
		CMotionPoint* topkey = itrmp->second;
		if( topkey ){
			CMotionPoint* curmp = topkey;
			CMotionPoint* nextmp = 0;
			while( curmp ){
				nextmp = curmp->GetNext();

				//delete curmp;
				CMotionPoint::InvalidateMotionPoint(curmp);

				curmp = nextmp;
			}
		}
	}
	m_motionkey.clear();


	map<string, std::map<CBone*, CRigidElem*>>::iterator itrmap;
	for( itrmap = m_remap.begin(); itrmap != m_remap.end(); itrmap++ ){
		map<CBone*, CRigidElem*>::iterator itrre;
		for( itrre = itrmap->second.begin(); itrre != itrmap->second.end(); itrre++ ){
			CRigidElem* curre = itrre->second;
			if (curre){
				//delete curre;
				CRigidElem::InvalidateRigidElem(curre);
			}
		}
		itrmap->second.clear();
	}
	m_remap.clear();
	m_impmap.clear();

	m_rigidelemname.clear();



	return 0;
}


int CBone::AddChild( CBone* childptr )
{
	if( !m_child ){
		m_child = childptr;
		m_child->m_parent = this;
	}
	else{
		CBone* broptr = m_child;
		if (broptr){
			while (broptr->m_brother){
				broptr = broptr->m_brother;
			}
			broptr->m_brother = childptr;
			broptr->m_brother->m_parent = this;//!!!!!!!!
		}
	}

	return 0;
}


int CBone::UpdateMatrix(bool limitdegflag, int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat, bool callingbythread)
{

	//2023/01/18 注意書修正
	//UpdateMatrixのsrcframeは経過時間計算を考慮した　浮動小数有りの時間が渡される
	//GetMotionPoint, GetWorldMatは intに丸めてからdoubleにして検索する justでtimeが一致しないとMotionPointが返らない
	//一方で　GetBefNextMPには　フレーム間姿勢の補間のために　小数有りの時間を渡す　justが無くても　befとnextを返す

	double roundingframe = (double)((int)(srcframe + 0.0001));


	int existflag = 0;

	if ((g_previewFlag != 5) || (m_parmodel && (m_parmodel->GetBtCnt() == 0))){
		if (srcframe >= 0.0) {
			ChaMatrix newworldmat;
			ChaMatrixIdentity(&newworldmat);

			//###################################
			//補間のためにroundingframeではない
			//###################################
			CallF(CalcFBXMotion(limitdegflag, srcmotid, srcframe, &m_curmp, &existflag), return 1);
			//newworldmat = m_curmp.GetWorldMat();// **wmat;
			newworldmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, &m_curmp);

			//2023/02/03
			//計算済を取得して補間するだけなので　m_curmp以外にはセットしない
			////2022/12/17
			////オイラー角情報更新
			////if (callingbythread == false) {//worldmatに変更が無いときにthreadで呼ばれる　よってローカル計算可能
			//CMotionPoint* mpptr = GetMotionPoint(srcmotid, roundingframe);
			//if (mpptr) {
			//	ChaMatrix wm = GetWorldMat(srcmotid, roundingframe, mpptr);
			//	ChaVector3 cureul = CalcLocalEulXYZ(-1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
			//	SetLocalEul(srcmotid, roundingframe, cureul, mpptr);
			//	if (g_limitdegflag == true) {
			//		mpptr->SetCalcLimitedWM(2);
			//	}
			//}
			//else {
			//	_ASSERT(0);
			//}


		//2023/02/02
		//modelのworldmatが掛かっていないアニメ姿勢も保存　GetCurrent..., CalcCurrent...用
			m_curmp.SetAnimMat(newworldmat);

		//modelのworldmatを掛ける
			ChaMatrix tmpmat = newworldmat * *wmat; // !!!!!!!!!!!!!!!!!!!!!!!!!!!
			SetWorldMat(limitdegflag, srcmotid, roundingframe, tmpmat, &m_curmp);//roundingframe!!!!
			if (limitdegflag == true) {
				m_curmp.SetCalcLimitedWM(2);
			}

			ChaVector3 jpos = GetJointFPos();
			ChaVector3TransformCoord(&m_childworld, &jpos, &tmpmat);
			ChaMatrix wvpmat = tmpmat * *vpmat;
			ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);//wmatで変換した位置に対して　vp変換
		}
		else {
			_ASSERT(0);
			m_curmp.InitParams();
			m_curmp.SetWorldMat(*wmat);
			m_curmp.SetFrame(roundingframe);
			SetWorldMat(limitdegflag, srcmotid, roundingframe, *wmat, &m_curmp);//roundingframe!!!!
		}

		if (m_parmodel && (m_parmodel->GetBtCnt() == 0)) {//2022/08/18 add checking m_parmodel
			SetBtMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, &m_curmp));
		}
	}
	else{
		//RagdollIK時のボーン選択対策
		ChaVector3 jpos = GetJointFPos();

		ChaMatrix wmat2, wvpmat;
		if (m_parent){
			wmat2 = m_parent->GetBtMat();// **wmat;
		}
		else{
			wmat2 = GetBtMat();// **wmat;
		}
		wvpmat = wmat2 * *vpmat;


		//ChaVector3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
		//ChaVector3TransformCoord(&m_childworld, &jpos, &wmat);
		ChaVector3TransformCoord(&m_childworld, &jpos, &wmat2);
		
		//ChaVector3TransformCoord(&m_childworld, &jpos, &(GetBtMat()));
		ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);
	}

	m_befupdatetime = srcframe;

	return 0;
}

int CBone::CopyLimitedWorldToWorld(int srcmotid, double srcframe)//制限角度有りの姿勢を制限無しの姿勢にコピーする
{
	double roundingframe = (double)((int)srcframe + 0.0001);

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		ChaMatrix limitedwm;
		limitedwm = curmp->GetLimitedWM();

		ChaMatrix newwm;
		newwm.SetIdentity();
		if (GetParent()) {
			CMotionPoint* parentmp;
			parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
			if (parentmp) {
				ChaMatrix limitedlocal;
				ChaMatrix parentlimitedwm;
				ChaMatrix parentunlimitedwm;
				limitedlocal.SetIdentity();
				parentunlimitedwm.SetIdentity();
				parentlimitedwm.SetIdentity();

				parentlimitedwm = parentmp->GetLimitedWM();
				parentunlimitedwm = parentmp->GetWorldMat();

				limitedlocal = limitedwm * ChaMatrixInv(parentlimitedwm);
				newwm = limitedlocal * parentunlimitedwm;
			}
			else {
				_ASSERT(0);
				newwm = limitedwm;
			}
		}
		else {
			newwm = limitedwm;
		}

		//bool limitdegflag = false;
		//bool directsetflag = false;
		////bool directsetflag = true;//2023/02/08 copyなのでdirectset.
		//bool infooutflag = false;
		//int setchildflag = 1;//setchildflagは directsetflag == falseのときしか働かない
		//SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, srcmotid, roundingframe, newwm);

		bool limitdegflag = false;
		UpdateCurrentWM(limitdegflag, srcmotid, roundingframe, newwm);

	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int CBone::CopyWorldToLimitedWorld(int srcmotid, double srcframe)//制限角度無しの姿勢を制限有りの姿勢にコピーする
{
	double roundingframe = (double)((int)srcframe + 0.0001);

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		ChaMatrix currentwm;
		currentwm = curmp->GetWorldMat();

		ChaMatrix newwm;
		newwm.SetIdentity();
		if (GetParent()) {
			CMotionPoint* parentmp;
			parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
			if (parentmp) {
				ChaMatrix unlimitedlocal;
				ChaMatrix parentunlimited;
				ChaMatrix parentlimited;
				unlimitedlocal.SetIdentity();
				parentunlimited.SetIdentity();
				parentlimited.SetIdentity();

				parentunlimited = parentmp->GetWorldMat();
				parentlimited = parentmp->GetLimitedWM();

				unlimitedlocal = currentwm * ChaMatrixInv(parentunlimited);
				newwm = unlimitedlocal * parentlimited;
			}
			else {
				_ASSERT(0);
				newwm = currentwm;
			}
		}
		else {
			newwm = currentwm;
		}

		//bool limitdegflag = true;
		//bool directsetflag = false;
		////bool directsetflag = true;//2023/02/08 copyなのでdirectset.
		//bool infooutflag = false;
		//int setchildflag = 1;//setchildflagは directsetflag == falseのときしか働かない
		//SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, srcmotid, roundingframe, newwm);

		bool limitdegflag = true;
		UpdateCurrentWM(limitdegflag, srcmotid, roundingframe, newwm);

	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}


int CBone::ApplyNewLimitsToWM(int srcmotid, double srcframe)
{
	//2023/02/03
	//この関数を実行する前に　limitedworldmatにworldmatをコピーしておく

	double roundingframe = (double)((int)srcframe + 0.0001);

	ChaMatrix curwm;
	curwm.SetIdentity();

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		
		//2023/02/03 LimitEulにチェックが入っていない場合にも　limitedに対して操作
		bool limitdegflag = true;
		curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
		bool directsetflag = false;
		int infooutflag = 0;
		int setchildflag = 1;//<-- 必須 RootNodeの回転を絞り込めば分かる
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, srcmotid, roundingframe, curwm, onlycheck, fromiktarget);
		
		//curmp->SetLimitedWM(newwm);
		//curmp->SetLimitedLocalEul(neweul);
	}
	else {
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int CBone::ClearLimitedWorldMat(int srcmotid, double srcframe0)
{
	int existflag = 0;

	//ChaMatrix newworldmat;
	//ChaMatrixIdentity(&newworldmat);

	//制限角度有り
	double srcframe = (double)((int)(srcframe0 + 0.0001));
	CMotionPoint* orgbefmp = 0;
	CMotionPoint* orgnextmp = 0;
	GetBefNextMP(srcmotid, srcframe, &orgbefmp, &orgnextmp, &existflag);
	//if ((existflag == 1) && orgbefmp) {
	if (orgbefmp) {
		orgbefmp->SetCalcLimitedWM(0);
	}

	return 0;

}


//int CBone::UpdateMatrixFromEul(int srcmotid, double srcframe, ChaVector3 neweul, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	int existflag = 0;
//
//	if ((g_previewFlag != 5) || (m_parmodel && (m_parmodel->GetBtCnt() == 0))) {
//
//
//		if (srcframe >= 0.0) {
//			CallF(CalcFBXMotion(srcmotid, srcframe, &m_curmp, &existflag), return 1);
//			//ChaMatrix tmpmat = m_curmp.GetWorldMat();// **wmat;
//
//			ChaMatrix newworldmat;
//			ChaMatrixIdentity(&newworldmat);
//			newworldmat = CalcWorldMatFromEul(0, 1, neweul, srcmotid, srcframe, 0);
//			//m_curmp.SetWorldMat(newworldmat);
//
//			//ChaMatrix tmpmat = m_curmp.GetWorldMat() * *wmat;
//			ChaMatrix tmpmat = newworldmat * *wmat;
//			m_curmp.SetWorldMat(tmpmat);
//
//
//			ChaVector3 jpos = GetJointFPos();
//			//ChaVector3TransformCoord(&m_childworld, &jpos, &m_curmp.GetWorldMat());
//			ChaVector3TransformCoord(&m_childworld, &jpos, &newworldmat);
//
//			//ChaMatrix wvpmat = m_curmp.GetWorldMat() * *vpmat;
//			ChaMatrix wvpmat = newworldmat * *vpmat;
//
//			ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);
//			//ChaVector3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
//		}
//		else {
//			m_curmp.InitParams();
//			m_curmp.SetWorldMat(*wmat);
//		}
//
//		if (m_parmodel->GetBtCnt() == 0) {
//			SetBtMat(m_curmp.GetWorldMat());
//		}
//
//	}
//	else {
//		//RagdollIK時のボーン選択対策
//		ChaVector3 jpos = GetJointFPos();
//
//		ChaMatrix wmat2, wvpmat;
//		if (m_parent) {
//			wmat2 = m_parent->GetBtMat();// **wmat;
//		}
//		else {
//			wmat2 = GetBtMat();// **wmat;
//		}
//		wvpmat = wmat2 * *vpmat;
//
//
//		//ChaVector3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
//		//ChaVector3TransformCoord(&m_childworld, &jpos, &wmat);
//		ChaVector3TransformCoord(&m_childworld, &jpos, &wmat2);
//
//		//ChaVector3TransformCoord(&m_childworld, &jpos, &(GetBtMat()));
//		ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);
//	}
//	return 0;
//}

//int CBone::AddMotionPointAll(int srcmotid, double animleng)
//{
//	EnterCriticalSection(&m_CritSection_AddMP);
//
//	if ((srcmotid <= 0) || (srcmotid > (m_motionkey.size() + 1))) {// on add : ひとつ大きくても可 : 他の部分でのチェックは motid > m_motionkey.size()
//		_ASSERT(0);
//		LeaveCriticalSection(&m_CritSection_AddMP);
//		return 1;
//	}
//
//
//	//indexedmotionpointが無ければ作成　内容クリア
//	std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
//	itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
//	if (itrvecmpmap == m_indexedmotionpoint.end()) {
//		std::vector<CMotionPoint*> newvecmp;
//		m_indexedmotionpoint[srcmotid] = newvecmp;//STL 参照されていれば無くならない？？？
//
//		std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap2;
//		itrvecmpmap2 = m_indexedmotionpoint.find(srcmotid);
//		if (itrvecmpmap2 == m_indexedmotionpoint.end()) {
//			_ASSERT(0);
//			LeaveCriticalSection(&m_CritSection_AddMP);
//			return 1;
//		}
//
//		itrvecmpmap = itrvecmpmap2;
//	}
//	(itrvecmpmap->second).clear();//!!!!!!!!!!!!!!!
//	m_initindexedmotionpoint[srcmotid] = false;
//
//
//
//	CMotionPoint* firstmp = 0;
//	if (srcmotid < m_motionkey.size()) {
//		firstmp = m_motionkey[srcmotid - 1];
//		if (firstmp) {
//			//既にモーションポイントが存在するのでリターン
//			LeaveCriticalSection(&m_CritSection_AddMP);
//			return 0;//この場合も正常とする
//		}
//	}
//	
//	firstmp = CMotionPoint::GetNewMP();
//	if (firstmp) {
//		firstmp->SetFrame(0.0);
//		m_motionkey[srcmotid - 1] = firstmp;
//		(itrvecmpmap->second).push_back(firstmp);//indexedmotionpointの設定
//
//		CMotionPoint* newmp = 0;
//		CMotionPoint* pbef = firstmp;
//
//		double srcframe;
//		for (srcframe = 1.0; srcframe < animleng; srcframe+=1.0) {
//			//newmp = new CMotionPoint();
//			newmp = CMotionPoint::GetNewMP();
//			if (!newmp) {
//				_ASSERT(0);
//				LeaveCriticalSection(&m_CritSection_AddMP);
//				return 1;
//			}
//			newmp->SetFrame(srcframe);
//
//			if (pbef) {
//				int result2 = pbef->AddToNext(newmp);
//				if (result2) {
//					LeaveCriticalSection(&m_CritSection_AddMP);
//					return 1;
//				}
//			}
//
//			(itrvecmpmap->second).push_back(newmp);//indexedmotionpointの設定
//		}
//	}
//	else {
//		_ASSERT(0);
//		LeaveCriticalSection(&m_CritSection_AddMP);
//		return 1;
//	}
//
//
//	//int result3 = CreateIndexedMotionPoint(srcmotid, animleng);
//	//if (result3 != 0) {
//	//	_ASSERT(0);
//	//	LeaveCriticalSection(&m_CritSection_AddMP);
//	//	return 1;
//	//}
//
//	m_initindexedmotionpoint[srcmotid] = true;
//
//
//	LeaveCriticalSection(&m_CritSection_AddMP);
//
//	return 0;
//}

CMotionPoint* CBone::AddMotionPoint(int srcmotid, double srcframe, int* existptr)
{
	EnterCriticalSection(&m_CritSection_AddMP);

	if ((srcmotid <= 0) || (srcmotid > (m_motionkey.size() + 1))) {// on add : ひとつ大きくても可 : 他の部分でのチェックは motid > m_motionkey.size()
		_ASSERT(0);
		LeaveCriticalSection(&m_CritSection_AddMP);
		return 0;
	}


	CMotionPoint* newmp = 0;
	CMotionPoint* pbef = 0;
	CMotionPoint* pnext = 0;
	bool onaddmotion = true;
	int result = GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, existptr, onaddmotion);
	if (result != 0) {
		LeaveCriticalSection(&m_CritSection_AddMP);
		return 0;
	}

	if (*existptr){
		pbef->SetFrame(srcframe);
		newmp = pbef;
	}
	else{
		//newmp = new CMotionPoint();
		newmp = CMotionPoint::GetNewMP();
		if (!newmp){
			_ASSERT(0);
			LeaveCriticalSection(&m_CritSection_AddMP);
			return 0;
		}
		newmp->SetFrame(srcframe);

		if (pbef){
			int result2 = pbef->AddToNext(newmp);
			if (result2) {
				LeaveCriticalSection(&m_CritSection_AddMP);
				return 0;
			}
		}
		else{
			m_motionkey[srcmotid - 1] = newmp;
			if (pnext){
				newmp->SetNext(pnext);
			}
		}


		//Comment out 2022/10/30 push_back対応が難しいので　AddMotionPoint, 長さが変わるInitMp処理時に　呼び出し側でCreateIndexedMotionPointを呼ぶ
		//GetMotionPointなどは　indexをチェックして　エントリーが無い場合には　チェインを辿る
		//std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
		//itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
		//if (itrvecmpmap != m_indexedmotionpoint.end()) {
		//	//(itrvecmpmap->second).clear();
		//	int frameindex = (int)(srcframe + 0.0001);
		//	(itrvecmpmap->second)[frameindex] = newmp;//indexedmotionpointはモーションポイントの実体管理用ではなくインデックス用、作成と破棄はチェインで行うので上書きしても良い。
		//}
	}



	LeaveCriticalSection(&m_CritSection_AddMP);

	return newmp;
}

//void CBone::ResizeIndexedMotionPointReq(int srcmotid, double animleng)
//{
//	ResizeIndexedMotionPoint(srcmotid, animleng);
//
//	if (GetChild()) {
//		GetChild()->ResizeIndexedMotionPointReq(srcmotid, animleng);
//	}
//	if (GetBrother()) {
//		GetBrother()->ResizeIndexedMotionPointReq(srcmotid, animleng);
//	}
//}


//int CBone::ResizeIndexedMotionPoint(int srcmotid, double animleng)
//{
//	//std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
//	//itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
//	//if (itrvecmpmap != m_indexedmotionpoint.end()) {
//	//	(itrvecmpmap->second).resize((int)(animleng + 0.0001));
//	//}
//	return 0;
//}


int CBone::CalcFBXMotion(bool limitdegflag, 
	int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr)
{
	CMotionPoint* befptr = 0;
	CMotionPoint* nextptr = 0;
	CallF( GetBefNextMP( srcmotid, srcframe, &befptr, &nextptr, existptr ), return 1 );
	CallF( CalcFBXFrame(limitdegflag, srcframe, befptr, nextptr, *existptr, dstmpptr), return 1 );

	return 0;
}

void CBone::ResetMotionCache()
{
	ZeroMemory(m_cachebefmp, sizeof(CMotionPoint*) * (MAXMOTIONNUM + 1));
}

int CBone::GetBefNextMP(int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr, bool onaddmotion)//default : onaddmotion = false
{
	//########################################################################################
	//並列化はボーン単位にするはずなので　クリティカルセクションにはエンターしないことにする
	//########################################################################################

	//EnterCriticalSection(&m_CritSection_GetBefNext);

	CMotionPoint* pbef = 0;
	//CMotionPoint* pcur = m_motionkey[srcmotid -1];
	CMotionPoint* pcur = 0;
	std::map<int, std::vector<CMotionPoint*>>::iterator itrvecmpmap;


	int curframeindex = (int)(srcframe + 0.0001);
	int nextframeindex = curframeindex + 1;
	int mpmapleng = 0;//2022/11/01 STLのsize()は重いらしいので変数に代入して使いまわし

	*existptr = 0;

	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
		//AddMotionPointから呼ばれるときに通る場合は正常
		*ppbef = 0;
		*ppnext = 0;
		//_ASSERT(0);
		//LeaveCriticalSection(&m_CritSection_GetBefNext);
		return 0;
	}
	else {
		if (curframeindex < 0) {
			*ppbef = 0;
			*ppnext = 0;

			if (srcmotid >= 1) {
				m_cachebefmp[srcmotid - 1] = NULL;
			}
			//_ASSERT(0);
			//LeaveCriticalSection(&m_CritSection_GetBefNext);
			return 0;
		}
		else {
			pcur = m_motionkey[srcmotid - 1];
		}
	}	

	bool getbychain;
	getbychain = onaddmotion;


	if (getbychain == false) {
		
		//get by indexed のフラグ指定の場合にもindexedの準備が出来ていない場合はget by chainで取得する

		if (m_initindexedmotionpoint.size() <= srcmotid) {//エントリーがまだ無いとき
			getbychain = true;
		}
		else {
			std::map<int, bool>::iterator itrinitflag;
			itrinitflag = m_initindexedmotionpoint.find(srcmotid);//initflag
			if (itrinitflag == m_initindexedmotionpoint.end()) {//エントリーがまだ無いとき
				getbychain = true;
			}
			else {
				if (itrinitflag->second == false) {//初期化フラグがfalseのとき　
					getbychain = true;
				}
			}
		}
	}

	if (getbychain == false) {
		//indexのframe長のチェック

		itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
		if (itrvecmpmap == m_indexedmotionpoint.end()) {
			getbychain = true;
		}
		else {

			mpmapleng = (int)(itrvecmpmap->second).size();

			if (curframeindex >= mpmapleng) {
				getbychain = true;
			}
		}
	}


	if (getbychain == true) {
#ifdef USE_CACHE_ONGETMOTIONPOINT__
		//キャッシュをチェックする
		if ((srcmotid >= 1) && (srcmotid <= MAXMOTIONNUM) && m_cachebefmp[srcmotid - 1] &&
			((m_cachebefmp[srcmotid - 1])->GetUseFlag() == 1) &&
			//((m_cachebefmp[srcmotid - 1])->GetFrame() <= (srcframe + 0.0001))) {
			((m_cachebefmp[srcmotid - 1])->GetFrame() <= ((double)curframeindex + 0.0001))) {//2022/12/26
			//高速化のため途中からの検索にする
			pcur = m_cachebefmp[srcmotid - 1];
		}
#endif

		while (pcur) {

			if ((pcur->GetFrame() >= srcframe - 0.0001) && (pcur->GetFrame() <= srcframe + 0.0001)) {//ジャスト判定　ジャストの場合補間無し
			//if ((pcur->GetFrame() >= ((double)curframeindex - 0.0001)) && (pcur->GetFrame() <= ((double)curframeindex + 0.0001))) {//2022/12/26 これでは補間が効かない
				*existptr = 1;
				pbef = pcur;
				break;
			}
			else if (pcur->GetFrame() > srcframe) {//過ぎてしまった場合
			//else if (pcur->GetFrame() > ((double)curframeindex + 0.0001)) {//2022/12/26
				*existptr = 0;
				break;
			}
			else {
				//for loop
				pbef = pcur;
				pcur = pcur->GetNext();
			}
		}
		*ppbef = pbef;//過ぎてしまった１つ前のモーションポイントをセット

		if (*existptr) {
			*ppnext = pbef->GetNext();
		}
		else {
			*ppnext = pcur;
		}


	}
	else {

		//### 2022/11/01 ################
		//最初の方でチェック済なので不要
		//###############################
		//if ((srcmotid <= 0) || (srcmotid > m_indexedmotionpoint.size())) {
		//	//AddMotionPointから呼ばれるときに通る場合は正常
		//	*ppbef = 0;
		//	*ppnext = 0;
		//	//_ASSERT(0);
		//	//LeaveCriticalSection(&m_CritSection_GetBefNext);
		//	return 0;
		//}
		//else {
		//	itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
		//	if (itrvecmpmap == m_indexedmotionpoint.end()) {
		//		*ppbef = 0;
		//		*ppnext = 0;
		//		//_ASSERT(0);
		//		//LeaveCriticalSection(&m_CritSection_GetBefNext);
		//		return 0;

		//	}
		//}

		//CMotionPoint* testmp = (itrvecmpmap->second)[curframeindex];



		if (curframeindex < mpmapleng) {
			*ppbef = (itrvecmpmap->second)[curframeindex];
		}
		else {
			if (mpmapleng >= 1) {
				*ppbef = (itrvecmpmap->second)[mpmapleng - 1];
			}
			else {
				*ppbef = 0;
			}
		}
		
		if (*ppbef) {
			double mpframe = (*ppbef)->GetFrame();
			if ((mpframe >= ((double)curframeindex - 0.0001)) && (mpframe <= ((double)curframeindex + 0.0001))) {
				*existptr = 1;
			}
			else {
				*existptr = 0;
			}


			if (nextframeindex < mpmapleng) {
				*ppnext = (itrvecmpmap->second)[nextframeindex];
			}
			else {
				if (mpmapleng >= 1) {
					*ppnext = (itrvecmpmap->second)[mpmapleng - 1];
				}
				else {
					*ppnext = 0;
				}
				
			}
		}
		else {
			*ppnext = 0;
			*existptr = 0;
		}
	}


#ifdef USE_CACHE_ONGETMOTIONPOINT__
	//m_cachebefmp = pbef;
	if ((srcmotid >= 1) && (srcmotid <= MAXMOTIONNUM)) {
		if (*ppbef) {
			if ((*ppbef)->GetPrev()) {
				m_cachebefmp[srcmotid - 1] = (*ppbef)->GetPrev();
			}
			else {
				m_cachebefmp[srcmotid - 1] = (*ppbef);
			}
		}
		else {
			//m_cachebefmp[srcmotid - 1] = m_motionkey[srcmotid - 1];
			m_cachebefmp[srcmotid - 1] = NULL;
		}
	}
#endif


	//LeaveCriticalSection(&m_CritSection_GetBefNext);

	return 0;
}


int CBone::CalcFBXFrame(bool limitdegflag, double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr)
{

	//GetWorldMat対策(未計算時の取得含む)のため　SetFrameには roundingframeを使用

	double roundingframe = (double)((int)(srcframe + 0.0001));


	if( existflag == 1 ){
		*dstmpptr = *befptr;
		return 0;
	}else if( !befptr ){
		dstmpptr->InitParams();
		dstmpptr->SetFrame(roundingframe);
		return 0;
	}else if( !nextptr ){
		*dstmpptr = *befptr;
		dstmpptr->SetFrame(roundingframe);
		return 0;
	}else{
		double diffframe = nextptr->GetFrame() - befptr->GetFrame();
		_ASSERT( diffframe != 0.0 );
		double t = ( srcframe - befptr->GetFrame() ) / diffframe;

		ChaMatrix tmpmat = GetWorldMat(limitdegflag, m_curmotid, befptr->GetFrame(), befptr) + 
			(GetWorldMat(limitdegflag, m_curmotid, nextptr->GetFrame(), nextptr) - GetWorldMat(limitdegflag, m_curmotid, befptr->GetFrame(), befptr)) * (float)t;

		SetWorldMat(limitdegflag, m_curmotid, roundingframe, tmpmat, dstmpptr);
		dstmpptr->SetFrame(roundingframe);

		//dstmpptr->SetPrev(befptr);
		//dstmpptr->SetNext(nextptr);

		//ChaMatrix newmat;
		//ChaMatrix orgparmat;
		//ChaMatrix newparmat;
		//if (GetParent()) {
		//	CMotionPoint* parentmp;
		//	newparmat = GetParent()->GetCurMp().GetWorldMat();
		//	parentmp = GetParent()->GetMotionPoint(m_curmotid, (double)((int)srcframe));
		//	orgparmat = parentmp->GetWorldMat();

		//	ChaVector3 orgpos;
		//	ChaVector3TransformCoord(&orgpos, &(GetJointFPos()), &orgparmat);
		//	ChaVector3 newpos;
		//	ChaVector3TransformCoord(&newpos, &(GetJointFPos()), &newparmat);
		//	ChaVector3 diffpos;
		//	diffpos = orgpos - newpos;
		//	//AddBoneTraReq()
		//	
		//}
		//else {
		//	newmat = tmpmat;
		//	//dstmpptr->SetWorldMat( tmpmat );
		//	dstmpptr->SetWorldMat(newmat);
		//	dstmpptr->SetFrame(srcframe);
		//	dstmpptr->SetPrev(befptr);
		//	dstmpptr->SetNext(nextptr);
		//}

		return 0;
	}
}

int CBone::DeleteMotion( int srcmotid )
{
	//#######################################################
	//削除によりm_motionkeyに空きが出来るがmotidは変わらない
	//#######################################################

	map<int, CMotionPoint*>::iterator itrmp;
	itrmp = m_motionkey.find( srcmotid - 1 );//2021/08/26
	if( itrmp != m_motionkey.end() ){
		CMotionPoint* topkey = itrmp->second;
		if( topkey ){
			CMotionPoint* curmp = topkey;
			CMotionPoint* nextmp = 0;
			while( curmp ){
				nextmp = curmp->GetNext();

				//delete curmp;
				CMotionPoint::InvalidateMotionPoint(curmp);

				curmp = nextmp;
			}
		}
	}

	//m_motionkey.erase( itrmp );
	m_motionkey[srcmotid - 1] = 0;////2021/08/26 eraseするとアクセスするためのインデックスがsrcmotid - 1ではなくなる



	std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
	itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
	if (itrvecmpmap != m_indexedmotionpoint.end()) {
		(itrvecmpmap->second).clear();
	}

	m_initindexedmotionpoint[srcmotid] = false;


	return 0;
}



int CBone::DeleteMPOutOfRange( int motid, double srcleng )
{
	if ((motid <= 0) || (motid > m_motionkey.size())) {
		_ASSERT(0);
		return 1;
	}

	CMotionPoint* curmp = m_motionkey[motid - 1];

	while( curmp ){
		CMotionPoint* nextmp = curmp->GetNext();

		if( curmp->GetFrame() > srcleng ){
			curmp->LeaveFromChain( motid, this );


			//delete curmp;
			CMotionPoint::InvalidateMotionPoint(curmp);

		}
		curmp = nextmp;
	}

	std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
	itrvecmpmap = m_indexedmotionpoint.find(motid);
	if (itrvecmpmap != m_indexedmotionpoint.end()) {
		//(itrvecmpmap->second).clear();

		size_t delframeno;
		size_t roundingleng;
		roundingleng = (size_t)(srcleng + 0.0001);
		for (delframeno = roundingleng; delframeno < (itrvecmpmap->second).size(); delframeno++) {
			(itrvecmpmap->second)[delframeno] = 0;
		}
		(itrvecmpmap->second).resize(roundingleng);
	}


	return 0;
}


int CBone::SetName( const char* srcname )
{
	strcpy_s( m_bonename, 256, srcname );
	TermJointRepeats(m_bonename);

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_bonename, 256, m_wbonename, 256 );

	if (m_initcustomrigflag == 0){
		InitCustomRig();
		m_initcustomrigflag = 1;
	}

	return 0;
}

int CBone::CalcAxisMatZ( ChaVector3* curpos, ChaVector3* childpos )
{
	if( *curpos == *childpos ){
		ChaMatrixIdentity( &m_laxismat );
		m_axisq.SetParams( 1.0f, 0.0f, 0.0f, 0.0f );
		return 0;
	}

	ChaVector3 startpos, endpos, upvec;

	ChaVector3 vecx0, vecy0, vecz0;
	ChaVector3 vecx1, vecy1, vecz1;

	startpos = *curpos;
	endpos = *childpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	ChaVector3 bonevec;
	bonevec = endpos - startpos;
	ChaVector3Normalize( &bonevec, &bonevec );

	//m_axisq.RotationArc(vecz0, bonevec);
	//m_laxismat = m_axisq.MakeRotMatX();

	if( (bonevec.x != 0.0f) || (bonevec.y != 0.0f) ){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
	}else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
	}

	vecz1 = bonevec;
		
	ChaVector3Cross( &vecx1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecz1 );
	ChaVector3Normalize( &vecx1, &vecx1 );

	ChaVector3Cross( &vecy1, (const ChaVector3*)&vecz1, (const ChaVector3*)&vecx1 );
	ChaVector3Normalize( &vecy1, &vecy1 );


	ChaMatrixIdentity( &m_laxismat );
	m_laxismat.data[MATI_11] = vecx1.x;
	m_laxismat.data[MATI_12] = vecx1.y;
	m_laxismat.data[MATI_13] = vecx1.z;

	m_laxismat.data[MATI_21] = vecy1.x;
	m_laxismat.data[MATI_22] = vecy1.y;
	m_laxismat.data[MATI_23] = vecy1.z;

	m_laxismat.data[MATI_31] = vecz1.x;
	m_laxismat.data[MATI_32] = vecz1.y;
	m_laxismat.data[MATI_33] = vecz1.z;

	m_axisq.RotationMatrix(m_laxismat);

	return 0;
}



float CBone::CalcAxisMatX_Manipulator(bool limitdegflag, int srcboneaxis, int bindflag, CBone* childbone, ChaMatrix* dstmat, int setstartflag)
{
	//############################################################################################
	//2022/11/04
	//マニピュレータの姿勢計算関数
	//以前のCalcAxisMatXを改造
	//CalcAxisMatXは　ボーンの位置だけからマニピュレータを計算していた
	//しかし　位置だけから計算すると　IK操作時のマニピュレータのIK平面がブレてしまう
	//これを解決するには　ボーンの姿勢行列を加工して　マニピュレータ行列を求めることが有効だった
	//ボーンの姿勢と　マニピュレータ行列の関係式が決まっていれば　IK中でもIK平面がブレない
	//############################################################################################


	//#########################################################################################################################
	//2023/01/11
	//AXISKIND_CURRENTについて
	//この関数は GetParent()->CalcAxisMatX_Manipulator( , childbone, ,,)のように呼び出される
	//つまり選択中のジョイントの親としての関数が呼び出されている
	//EditMot(MameBake3D)においては　回転用のマニピュレータの操作は　IK　つまり　子供ジョイントドラッグで親ジョイントを回転する
	//マニピュレータは　子供ジョイント位置に表示するが　その軸の向きは　Parentの姿勢を反映したものとなる
	//よってAXISKIND_CURRENTの場合　親からみた　currentのNodeMatにcurrentのGetLimitedWorldMatを掛けたものが基準となる
	//#########################################################################################################################


	bool ishipsjoint = IsHipsBone();


	ChaVector3 aftbonepos;
	ChaVector3 aftchildpos;
	ChaVector3 aftparentpos;

	if (!dstmat) {
		_ASSERT(0);
		return 0.0f;
	}
	if (!childbone) {
		_ASSERT(0);
		ChaMatrix inimat;
		ChaMatrixIdentity(&inimat);
		*dstmat = inimat;
		return 0.0f;
	}

	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaVector3 tmpfpos = GetJointFPos();
	ChaVector3 tmpchildfpos = childbone->GetJointFPos();
	ChaVector3 tmpparentfpos;
	if (GetParent()) {
		tmpparentfpos = GetParent()->GetJointFPos();
	}
	else {
		tmpparentfpos = ChaVector3(0.0f, 0.0f, 0.0f);
	}

	ChaMatrix tmpzerofm = GetNodeMat() * GetCurrentZeroFrameMat(limitdegflag, 1);
	ChaMatrix tmplimwm = GetNodeMat() * GetCurrentWorldMat();
	ChaMatrix tmpbtmat = GetNodeMat() * GetBtMat();
	ChaMatrix tmpchildzerofm = childbone->GetNodeMat() * childbone->GetCurrentZeroFrameMat(limitdegflag, 1);
	ChaMatrix tmpchildlimwm = childbone->GetNodeMat() * childbone->GetCurrentWorldMat();
	ChaMatrix tmpchildbtmat = childbone->GetNodeMat() * childbone->GetBtMat();
	//ChaMatrix tmpchildbtmat;
	ChaMatrix tmpparentzerofm;
	ChaMatrix tmpparentlimwm;
	ChaMatrix tmpparentbtmat;
	if (GetParent()) {
		tmpparentzerofm = GetParent()->GetNodeMat() * GetParent()->GetCurrentZeroFrameMat(limitdegflag, 1);
		tmpparentlimwm = GetParent()->GetNodeMat() * GetParent()->GetCurrentWorldMat();
		tmpparentbtmat = GetParent()->GetNodeMat() * GetParent()->GetBtMat();
	}
	else {
		tmpparentzerofm.SetIdentity();
		tmpparentlimwm.SetIdentity();
		tmpparentbtmat.SetIdentity();
	}


	ChaMatrix convmat;
	convmat.SetIdentity();

	if ((g_previewFlag != 4) && (g_previewFlag != 5)) {
		ChaVector3TransformCoord(&aftparentpos, &zeropos, &tmpparentlimwm);
		ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmplimwm);
		ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildlimwm);
	}
	else {
		ChaVector3TransformCoord(&aftparentpos, &zeropos, &tmpparentbtmat);
		ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpbtmat);
		ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildbtmat);
	}

	if (ishipsjoint == false) {

		//hipsjointではない場合

		if ((srcboneaxis == BONEAXIS_CURRENT) || (srcboneaxis == BONEAXIS_BINDPOSE)) {
			//current bone axis
			if ((g_previewFlag != 4) && (g_previewFlag != 5)) {
				convmat = tmplimwm;
			}
			else {
				convmat = tmpbtmat;
			}
			
		}
		else if (srcboneaxis == BONEAXIS_PARENT) {
			//parent bone axis
			if ((g_previewFlag != 4) && (g_previewFlag != 5)) {
				convmat = tmpparentlimwm;
			}
			else {
				convmat = tmpparentbtmat;
			}
		}
		else if (srcboneaxis == BONEAXIS_GLOBAL) {
			//global axis
			convmat.SetIdentity();
		}
		else {
			convmat = tmplimwm;
		}
	}
	else {
		//2023/01/14
		//hipsjointの場合には　IK回転として　Global回転するしかない(移動成分も回転する)ので　マニピュレータもそれに合わせる
		//global axis
		convmat.SetIdentity();
	}



	//カレント変換したボーン軸
	ChaVector3 vecforleng;
	vecforleng = aftchildpos - aftbonepos;
	float retleng = (float)ChaVector3LengthDbl(&vecforleng);



	if ((aftbonepos == aftchildpos) || (srcboneaxis == BONEAXIS_GLOBAL) || (ishipsjoint == true)) {
		//ボーンの長さが０のとき　Identity回転　ボーン軸の種類がグローバルの場合　Identity回転
		dstmat->SetIdentity();
		//#########################################################
		//位置は　ボーンの親の位置　つまりカレントジョイントの位置
		//#########################################################
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_41] = aftbonepos.z;

		return retleng;
	}

	ChaVector3 startpos, endpos;

	if ((srcboneaxis == BONEAXIS_CURRENT) || (srcboneaxis == BONEAXIS_BINDPOSE)) {
		//current bone axis
		startpos = aftbonepos;
		endpos = aftchildpos;
	}
	else if (srcboneaxis == BONEAXIS_PARENT) {
		//parent bone axis
		startpos = aftparentpos;
		endpos = aftbonepos;
	}
	else if (srcboneaxis == BONEAXIS_GLOBAL) {
		//global axis

		_ASSERT(0);//上方でIdentity回転をセットしてリターンしているので　ここは通らない
		dstmat->SetIdentity();
		//#########################################################
		//位置は　ボーンの親の位置　つまりカレントジョイントの位置
		//#########################################################
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_41] = aftbonepos.z;
		return retleng;
	}
	else {
		_ASSERT(0);//想定外
		dstmat->SetIdentity();
		//#########################################################
		//位置は　ボーンの親の位置　つまりカレントジョイントの位置
		//#########################################################
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_41] = aftbonepos.z;
		return retleng;
	}


	//カレント変換したボーン軸
	ChaVector3 bonevec;
	bonevec = endpos - startpos;
	ChaVector3Normalize(&bonevec, &bonevec);


	//###########################################################################################
	//convmatのvecxをbonevecにする　それに合わせて３軸が互いに垂直になるようにvecy, veczを求める
	//###########################################################################################
	//#########################################################
	//位置は　ボーンの親の位置　つまりカレントジョイントの位置
	//#########################################################

	if (srcboneaxis != BONEAXIS_BINDPOSE) {//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BINDPOSEの時は　オートフィットしない
		//ボーン軸をX軸に　オートフィット
		*dstmat = CalcAxisMatX(bonevec, aftbonepos, convmat);//ChaVecCalc.cpp
	}
	else {
		//BONEAXIS_BINDPOSEの場合には　ボーン軸をX軸には合わせずに　BindPoseそのままの向きを返す
		*dstmat = convmat;

		//#####################################################################################################
		//位置は　ボーンの(呼び出すときの)親の位置　つまり(呼び出されたインスタンスの)カレントジョイントの位置
		//#####################################################################################################
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_43] = aftbonepos.z;
	}
	

	return retleng;
}

float CBone::CalcAxisMatX_NodeMat(CBone* childbone, ChaMatrix* dstmat)
{
	ChaVector3 aftbonepos;
	ChaVector3 aftchildpos;

	if (!dstmat) {
		_ASSERT(0);
		return 0.0f;
	}
	if (!childbone) {
		_ASSERT(0);
		ChaMatrix inimat;
		ChaMatrixIdentity(&inimat);
		*dstmat = inimat;
		return 0.0f;
	}

	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaVector3 tmpfpos = GetJointFPos();
	ChaVector3 tmpchildfpos = childbone->GetJointFPos();
	ChaMatrix convmat;
	convmat.SetIdentity();

	ChaMatrix tmpnodefm = GetNodeMat();
	ChaMatrix tmpchildnodefm = childbone->GetNodeMat();
	ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpnodefm);
	ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildnodefm);
	convmat = tmpnodefm;


	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (aftbonepos == aftchildpos) {
		//長さ０ボーン対策
		*dstmat = retmat;
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_43] = aftbonepos.z;
		//_ASSERT(0);
		return 0.0f;
	}


	//カレント変換したボーン軸
	ChaVector3 bonevec;
	bonevec = aftchildpos - aftbonepos;
	ChaVector3Normalize(&bonevec, &bonevec);

	//###########################################################################################
	//convmatのvecxをbonevecにする　それに合わせて３軸が互いに垂直になるようにvecy, veczを求める
	//###########################################################################################
	//#########################################################
	//位置は　ボーンの親の位置　つまりカレントジョイントの位置
	//#########################################################

	*dstmat = CalcAxisMatX(bonevec, aftbonepos, convmat);//ChaVecCalc.cpp

	ChaVector3 diffvec = aftbonepos - aftchildpos;
	float retleng = (float)ChaVector3LengthDbl(&diffvec);

	return retleng;


}

float CBone::CalcAxisMatX_RigidBody(bool limitdegflag, bool dir2xflag, int bindflag, CBone* childbone, ChaMatrix* dstmat, int setstartflag)
{
	//#############################################################################################
	//2022/11/03
	//RigidBody用　basevecは マニピュレータと同じvecx(RigidBody形状はCapsule_dirX.mqo)
	//CalcAxisMatX_Manipulatorと違うところは　g_ikaxis_kindによらず　RigidBody用の計算をするところ
	//#############################################################################################

	//########################################################################################################################
	//2023/01/18
	//dir2xflagを追加
	//bullet physicsのrigidbodyの回転モーションの軸を　通常モーションの軸と合わせるためには　NodeMatをX軸に向けないことが必要
	//dir2xflag == falseのときには　NodeMatをX軸に向けない
	// 
	//カプセル形状の表示のためには　NodeMatをX軸に向ける必要がある　この時には dir2xflagをtrueにしてこの関数を呼ぶ
	//########################################################################################################################

	ChaVector3 aftbonepos;
	ChaVector3 aftchildpos;

	if (!dstmat) {
		_ASSERT(0);
		return 0.0f;
	}
	if (!childbone) {
		_ASSERT(0);
		ChaMatrix inimat;
		ChaMatrixIdentity(&inimat);
		*dstmat = inimat;
		return 0.0f;
	}

	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	ChaVector3 tmpfpos = GetJointFPos();
	ChaVector3 tmpchildfpos = childbone->GetJointFPos();
	ChaMatrix convmat;
	convmat.SetIdentity();

	if (bindflag == 1) {
		//bind pose
		ChaMatrix tmpzerofm = GetNodeMat() * GetCurrentZeroFrameMat(limitdegflag, 1);
		ChaMatrix tmpchildzerofm = childbone->GetNodeMat() * childbone->GetCurrentZeroFrameMat(limitdegflag, 1);
		ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpzerofm);
		ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildzerofm);

		convmat = tmpzerofm;
	}
	else {
		if ((g_previewFlag != 5) && (g_previewFlag != 4)) {
			ChaMatrix tmpzerofm = GetNodeMat() * GetCurrentZeroFrameMat(limitdegflag, 1);
			ChaMatrix tmplimwm = GetNodeMat() * GetCurrentWorldMat();
			ChaMatrix tmpchildzerofm = childbone->GetNodeMat() * childbone->GetCurrentZeroFrameMat(limitdegflag, 1);
			ChaMatrix tmpchildlimwm = childbone->GetNodeMat() * childbone->GetCurrentWorldMat();

			if (setstartflag == 1) {
				ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpzerofm);
				ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildzerofm);

				convmat = tmpzerofm;
			}
			else {
				ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmplimwm);
				ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildlimwm);

				convmat = tmplimwm;
			}
		}
		else {
			ChaMatrix tmpzerofm = GetNodeMat() * GetCurrentZeroFrameMat(limitdegflag, 1);
			ChaMatrix tmpbtmat = GetNodeMat() * GetBtMat();
			ChaMatrix tmpchildzerofm = childbone->GetNodeMat() * childbone->GetCurrentZeroFrameMat(limitdegflag, 1);
			ChaMatrix tmpchildbtmat = childbone->GetNodeMat() * childbone->GetBtMat();
			if (setstartflag == 1) {
				ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpzerofm);
				ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildzerofm);

				convmat = tmpzerofm;
			}
			else {
				ChaVector3TransformCoord(&aftbonepos, &zeropos, &tmpbtmat);
				ChaVector3TransformCoord(&aftchildpos, &zeropos, &tmpchildbtmat);

				convmat = tmpbtmat;
			}
		}
	}
	
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (aftbonepos == aftchildpos) {

		//長さ０ボーン対策
		*dstmat = retmat;
		dstmat->data[MATI_41] = aftbonepos.x;
		dstmat->data[MATI_42] = aftbonepos.y;
		dstmat->data[MATI_43] = aftbonepos.z;
		//_ASSERT(0);
		return 0.0f;
	}


	//カレント変換したボーン軸
	ChaVector3 bonevec;
	bonevec = aftchildpos - aftbonepos;
	ChaVector3Normalize(&bonevec, &bonevec);

	//###########################################################################################
	//convmatのvecxをbonevecにする　それに合わせて３軸が互いに垂直になるようにvecy, veczを求める
	//###########################################################################################
	//#########################################################
	//位置は　ボーンの親の位置　つまりカレントジョイントの位置
	//#########################################################
	

	//2023/01/18
	if (dir2xflag == true) {
		*dstmat = CalcAxisMatX(bonevec, aftbonepos, convmat);//ChaVecCalc.cpp
	}
	else {
		*dstmat = convmat;
	}
	
	ChaVector3 diffvec = aftbonepos - aftchildpos;
	float retleng = (float)ChaVector3LengthDbl(&diffvec);

	return retleng;
}




/*
int CBone::CalcAxisMatX()
{
	ChaVector3 curpos;
	ChaVector3 childpos;

	if (m_parent){
		ChaVector3TransformCoord(&curpos, &(m_parent->GetJointFPos()), &(m_parent->m_startmat2));
		ChaVector3TransformCoord(&childpos, &(GetJointFPos()), &m_startmat2);

		CalcAxisMatX_aft(curpos, childpos, &m_gaxismatXpar);
	}

	return 0;
}
*/

/*
int CBone::CalcAxisMatX_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* dstmat)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (curpos == childpos){
		*dstmat = retmat;
		return 0;
	}

	ChaVector3 startpos, endpos, upvec;

	ChaVector3 vecx0, vecy0, vecz0;
	ChaVector3 vecx1, vecy1, vecz1;

	startpos = curpos;
	endpos = childpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	ChaVector3 bonevec;
	bonevec = endpos - startpos;
	ChaVector3Normalize(&bonevec, &bonevec);

	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.y) >= 0.000001f)){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
		m_upkind = UPVEC_Z;//vecx1-->X(bone), vecy1-->Y, vecz1-->Z
	}
	else{
		upvec.x = 0.0f;
		upvec.y = 1.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_Y;//vecx1-->X(bone), vecy1-->Z, vecz1-->Y
	}

	vecx1 = bonevec;

	ChaVector3Cross(&vecy1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecx1);

	int illeagalflag = 0;
	float crleng = ChaVector3LengthDbl(&vecy1);
	if (crleng < 0.000001f){
		illeagalflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	ChaVector3Normalize(&vecy1, &vecy1);


	ChaVector3Cross(&vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1);
	ChaVector3Normalize(&vecz1, &vecz1);


	ChaMatrixIdentity(dstmat);
	if (illeagalflag == 0){
		dstmat->_11 = vecx1.x;
		dstmat->_12 = vecx1.y;
		dstmat->_13 = vecx1.z;

		dstmat->_21 = vecy1.x;
		dstmat->_22 = vecy1.y;
		dstmat->_23 = vecy1.z;

		dstmat->_31 = vecz1.x;
		dstmat->_32 = vecz1.y;
		dstmat->_33 = vecz1.z;
	}

	return 0;
}
*/

int CBone::CalcAxisMatZ_aft(ChaVector3 curpos, ChaVector3 childpos, ChaMatrix* dstmat)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (curpos == childpos){
		*dstmat = retmat;
		return 0;
	}

	ChaVector3 startpos, endpos, upvec;

	ChaVector3 vecx0, vecy0, vecz0;
	ChaVector3 vecx1, vecy1, vecz1;

	startpos = curpos;
	endpos = childpos;

	vecx0.x = 1.0;
	vecx0.y = 0.0;
	vecx0.z = 0.0;

	vecy0.x = 0.0;
	vecy0.y = 1.0;
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	ChaVector3 bonevec;
	bonevec = endpos - startpos;
	ChaVector3Normalize(&bonevec, &bonevec);

	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.z) >= 0.000001f)){
		upvec.x = 0.0f;
		upvec.y = 1.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_Y;//vecx1-->X, vecy1-->Y, vecz1-->Z(bone)
	}
	else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
		m_upkind = UPVEC_X;//vecx1-->Y, vecy1-->X, vecz1-->Z(bone)
	}

	vecz1 = bonevec;

	//ChaVector3Cross(&vecx1, &vecz1, &upvec);
	ChaVector3Cross(&vecx1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecz1);
	ChaVector3Normalize(&vecx1, &vecx1);

	ChaVector3Cross(&vecy1, (const ChaVector3*)&vecz1, (const ChaVector3*)&vecx1);
	ChaVector3Normalize(&vecy1, &vecy1);


	retmat.data[MATI_11] = vecx1.x;
	retmat.data[MATI_12] = vecx1.y;
	retmat.data[MATI_13] = vecx1.z;

	retmat.data[MATI_21] = vecy1.x;
	retmat.data[MATI_22] = vecy1.y;
	retmat.data[MATI_23] = vecy1.z;

	retmat.data[MATI_31] = vecz1.x;
	retmat.data[MATI_32] = vecz1.y;
	retmat.data[MATI_33] = vecz1.z;

	*dstmat = retmat;

	return 0;
}



//int CBone::CalcAxisMatY( CBone* childbone, ChaMatrix* dstmat )
//{
//
//	ChaVector3 curpos;
//	ChaVector3 childpos;
//	ChaVector3 tmpfpos = GetJointFPos();
//	ChaMatrix tmpwm = GetWorldMat(m_curmotid, m_curmp.GetFrame(), &m_curmp);
//	ChaVector3TransformCoord(&curpos, &tmpfpos, &tmpwm);
//	//ChaVector3TransformCoord(&childpos, &(childbone->GetJointFPos()), &(childbone->m_curmp.GetWorldMat()));
//	ChaVector3 tmpchildfpos = childbone->GetJointFPos();
//	ChaVector3TransformCoord(&childpos, &tmpchildfpos, &tmpwm);
//
//	ChaVector3 diff = curpos - childpos;
//	float leng;
//	leng = (float)ChaVector3LengthDbl( &diff );
//
//	if( leng <= 0.00001f ){
//		ChaMatrixIdentity( dstmat );
//		return 0;
//	}
//
//	ChaVector3 startpos, endpos, upvec;
//
//	ChaVector3 vecx0, vecy0, vecz0;
//	ChaVector3 vecx1, vecy1, vecz1;
//
//	startpos = curpos;
//	endpos = childpos;
//
//	vecx0.x = 1.0;
//	vecx0.y = 0.0;
//	vecx0.z = 0.0;
//
//	vecy0.x = 0.0;
//	vecy0.y = 1.0;//!!!!!!!!!!!!!!!!!!
//	vecy0.z = 0.0;
//
//	vecz0.x = 0.0;
//	vecz0.y = 0.0;
//	vecz0.z = 1.0;
//
//	ChaVector3 bonevec;
//	bonevec = endpos - startpos;
//	ChaVector3Normalize( &bonevec, &bonevec );
//
//	if( (bonevec.x != 0.0f) || (bonevec.y != 0.0f) ){
//		upvec.x = 0.0f;
//		upvec.y = 0.0f;
//		upvec.z = 1.0f;
//	}else{
//		upvec.x = 1.0f;
//		upvec.y = 0.0f;
//		upvec.z = 0.0f;
//	}
//
//	vecy1 = bonevec;
//		
//	ChaVector3Cross( &vecx1, (const ChaVector3*)&vecy1, (const ChaVector3*)&upvec );
//
//	int illeagalflag = 0;
//	float crleng = (float)ChaVector3LengthDbl( &vecx1 );
//	if( crleng < 0.000001f ){
//		illeagalflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	}
//
//	ChaVector3Normalize( &vecx1, &vecx1 );
//
//	ChaVector3Cross( &vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1 );
//	ChaVector3Normalize( &vecy1, &vecy1 );
//
//	//D3DXQUATERNION tmpxq;
//
//	ChaMatrixIdentity( dstmat );
//	if( illeagalflag == 0 ){
//		dstmat->data[0] = vecx1.x;
//		dstmat->data[1] = vecx1.y;
//		dstmat->data[2] = vecx1.z;
//
//		dstmat->data[4] = vecy1.x;
//		dstmat->data[5] = vecy1.y;
//		dstmat->data[6] = vecy1.z;
//
//		dstmat->data[8] = vecz1.x;
//		dstmat->data[9] = vecz1.y;
//		dstmat->data[10] = vecz1.z;
//	}
//
//	return 0;
//}

int CBone::CalcRigidElemParams( CBone* childbone, int setstartflag )
{
	
	//剛体の形状(m_coldisp)を複数の子供で使いまわしている。使用するたびにこの関数で大きさをセットしている。
	if (!childbone) {
		_ASSERT(0);
		return 0;
	}


	CRigidElem* curre = GetRigidElem(childbone);
	if( !curre ){
		_ASSERT( 0 );
		return 0;
	}

	//_ASSERT( colptr );
	_ASSERT( childbone );

	CModel* curcoldisp = m_coldisp[curre->GetColtype()];
	_ASSERT( curcoldisp );



	//ChaMatrix bmmat;
	//ChaMatrixIdentity(&bmmat);
	//CalcAxisMatZ( &aftbonepos, &aftchildpos );
	//CalcAxisMatY( childbone, &bmmat );			
	//float diffleng = CalcAxisMatX(0, childbone, &bmmat, 1);

	//ChaMatrix bindcapsulemat;
	//ChaMatrixIdentity(&bindcapsulemat);
	//bool dir2xflag = false;
	//float diffleng = CalcAxisMatX_RigidBody(dir2xflag, 1, childbone, &bindcapsulemat, 1);

	ChaVector3 jointpos, childjointpos, diffvec;
	jointpos = GetJointFPos();
	childjointpos = childbone->GetJointFPos();
	diffvec = jointpos - childjointpos;
	float diffleng = (float)ChaVector3LengthDbl(&diffvec);


	float cylileng = curre->GetCylileng();
	float sphr = curre->GetSphr();
	float boxz = curre->GetBoxz();

	//if ((setstartflag != 0) || (m_firstcalcrigid == true)){
		if (curre->GetColtype() == COL_CAPSULE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				if (strcmp(curobj->GetName(), "cylinder") == 0){
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 0, &cylileng), return 1);
				}
				else if (strcmp(curobj->GetName(), "sph_ue") == 0){
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 1, &sphr), return 1);
				}
				else{
					CallF(curobj->ScaleBtCapsule(curre, diffleng, 2, 0), return 1);
				}
			}
		}
		else if (curre->GetColtype() == COL_CONE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtCone(curre, diffleng, &cylileng, &sphr), return 1);
			}
		}
		else if (curre->GetColtype() == COL_SPHERE_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtSphere(curre, diffleng, &cylileng, &sphr), return 1);
			}
		}
		else if (curre->GetColtype() == COL_BOX_INDEX){
			map<int, CMQOObject*>::iterator itrobj;
			for (itrobj = curcoldisp->GetMqoObjectBegin(); itrobj != curcoldisp->GetMqoObjectEnd(); itrobj++){
				CMQOObject* curobj = itrobj->second;
				_ASSERT(curobj);
				CallF(curobj->ScaleBtBox(curre, diffleng, &cylileng, &sphr, &boxz), return 1);

				DbgOut(L"bonecpp : calcrigidelemparams : BOX : cylileng %f, sphr %f, boxz %f\r\n", cylileng, sphr, boxz);

			}
		}
		else{
			_ASSERT(0);
			return 1;
		}
	//}

	//bmmat._41 = ( aftbonepos.x + aftchildpos.x ) * 0.5f;
	//bmmat._42 = ( aftbonepos.y + aftchildpos.y ) * 0.5f;
	//bmmat._43 = ( aftbonepos.z + aftchildpos.z ) * 0.5f;

	//bmmat._41 = aftbonepos.x;
	//bmmat._42 = aftbonepos.y;
	//bmmat._43 = aftbonepos.z;

	//curre->SetBindcapsulemat(bindcapsulemat);
	//bmmat = curre->GetEndbone()->CalcManipulatorPostureMatrix(0, 1, 1);

	//curre->SetCapsulemat( bmmat );
	curre->SetCylileng( cylileng );
	curre->SetSphr( sphr );
	curre->SetBoxz( boxz );



	if( setstartflag != 0 ){
		//bmmat = curre->GetCapsulemat(1);
		//curre->SetFirstcapsulemat( bmmat );
		//curre->SetFirstWorldmat(childbone->GetCurrentZeroFrameMat(0));
		//curre->SetFirstWorldmat(GetCurMp().GetWorldMat());

		//if (setstartflag == 2){
		//	childbone->SetNodeMat(bmmat);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//}

		childbone->SetBtMat(childbone->GetCurMp().GetWorldMat());//!!!!!!!!!!!!!btmatの初期値
	}


	m_firstcalcrigid = false;

	return 0;
}

//void CBone::SetStartMat2Req()
//{
//	//SetStartMat2(m_curmp.GetWorldMat());
//	SetStartMat2(GetCurrentZeroFrameMat(0));
//	SetBtMat(GetWorldMat(m_curmotid, m_curmp.GetFrame(), &m_curmp));//!!!!!!!!!!!!!btmatの初期値
//
//	if (m_child){
//		m_child->SetStartMat2Req();
//	}
//	if (m_brother){
//		m_brother->SetStartMat2Req();
//	}
//}

/*
int CBone::CalcAxisMat( int firstflag, float delta )
{
	if( firstflag == 1 ){
		SetStartMat2Req();
		
		CalcAxisMatX();
	}

	if( delta != 0.0f ){
		CQuaternion multq;
		ChaVector3 gparentpos, gchildpos, gbonevec;
		if (m_parent){
			ChaVector3TransformCoord(&gparentpos, &(m_parent->GetJointFPos()), &(m_parent->m_startmat2));
			ChaVector3TransformCoord(&gchildpos, &(GetJointFPos()), &m_startmat2);
			gbonevec = gchildpos - gparentpos;
			ChaVector3Normalize(&gbonevec, &gbonevec);
		}
		else{
			_ASSERT(0);
			return 0;
		}


		multq.SetAxisAndRot( gbonevec, delta );
		ChaMatrix multmat = multq.MakeRotMatX();
		m_gaxismatXpar = m_gaxismatXpar * multmat;
	}

	ChaMatrix invpar;
	ChaMatrixInverse( &invpar, NULL, &m_gaxismatXpar );

	CalcLocalAxisMat( m_startmat2, m_gaxismatXpar, m_gaxismatYpar );

	return 0;
}
*/
int CBone::CalcLocalAxisMat( ChaMatrix motmat, ChaMatrix axismatpar, ChaMatrix gaxisy )
{
	ChaMatrix startpar0 = axismatpar;
	startpar0.data[MATI_41] = 0.0f;
	startpar0.data[MATI_42] = 0.0f;
	startpar0.data[MATI_43] = 0.0f;

	ChaMatrix starty = gaxisy;
	starty.data[MATI_41] = 0.0f;
	starty.data[MATI_42] = 0.0f;
	starty.data[MATI_43] = 0.0f;

	ChaMatrix motmat0 = motmat;
	motmat0.data[MATI_41] = 0.0f;
	motmat0.data[MATI_42] = 0.0f;
	motmat0.data[MATI_43] = 0.0f;

	ChaMatrix invmotmat;
	ChaMatrixInverse( &invmotmat, NULL, &motmat0 );

	m_axismat_par = startpar0 * invmotmat;

	return 0;
}

int CBone::CreateRigidElem( CBone* parentbone, int reflag, std::string rename, int impflag, std::string impname )
{
	if (!parentbone){
		return 0;
	}

	if (reflag){
		//CRigidElem* newre = new CRigidElem();
		CRigidElem* newre = CRigidElem::GetNewRigidElem();
		if (!newre){
			_ASSERT(0);
			return 1;
		}
		newre->SetBone(parentbone);
		newre->SetEndbone(this);
		SetGroupNoByName(newre, this);
		parentbone->SetRigidElemOfMap(rename, this, newre);
	}

//////////////
	if( impflag ){
		map<string, map<CBone*, ChaVector3>>::iterator findimpmap;
		findimpmap = parentbone->FindImpMap(impname);
		if (findimpmap != parentbone->GetImpMapEnd()){
			map<CBone*, ChaVector3>::iterator itrimp;
			itrimp = findimpmap->second.find(this);
			if (itrimp != findimpmap->second.end()){
				return 0;
			}
			findimpmap->second[this] = ChaVector3(0.0f, 0.0f, 0.0f);
		}else{
			map<CBone*, ChaVector3> curmap;

			curmap[this] = ChaVector3( 0.0f, 0.0f, 0.0f );
			parentbone->m_impmap[impname] = curmap;
		}
	}
	return 0;
}

int CBone::SetGroupNoByName( CRigidElem* curre, CBone* childbone )
{
	char* groupmark = strstr( childbone->m_bonename, "_G_" );
	if( groupmark ){
		char* numstart = groupmark + 3;
		char* numend = strstr( numstart, "_" );
		if( numend ){
			int numleng = (int)(numend - numstart);
			if( (numleng > 0) && (numleng <= 2) ){
				char strnum[5];
				ZeroMemory( strnum, sizeof( char ) * 5 );
				strncpy_s( strnum, 5, numstart, numleng );
				int gno = (int)atoi( strnum );
				if( (gno >= 1) && (gno <= COLIGROUPNUM) ){
					curre->SetGroupid( gno ); 
				}
			}
		}
	}
	int cmpbt = strncmp( childbone->m_bonename, "BT_", 3 );
	if( cmpbt == 0 ){
		m_btforce = 1;
	}

	return 0;
}

int CBone::SetCurrentRigidElem( std::string curname )
{
	m_rigidelemname = curname;

	/*
	m_rigidelem.clear();

	if( !m_child ){
		return 0;
	}

	map<string, map<CBone*, CRigidElem*>>::iterator itrmap;
	itrmap = m_remap.find( curname );
	if( itrmap == m_remap.end() ){
		_ASSERT( 0 );
		::MessageBoxA(NULL, "CBone SetCurrentRigidElem : map not found", "error", MB_OK);
		return 1;
	}

	m_rigidelem = itrmap->second;
	*/


	return 0;
}


CMotionPoint* CBone::AddBoneTraReq(bool limitdegflag, CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srctra, ChaMatrix befparentwm, ChaMatrix newparentwm)
{

	double roundingframe = (double)((int)(srcframe + 0.0001));


	int existflag = 0;
	//CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	//if( !curmp || !existflag ){
	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if(!curmp){
		_ASSERT( 0 );
		return 0;
	}

	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);


	bool infooutflag = false;

	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	if( parmp ){
		//ChaMatrix invbefpar;
		//ChaMatrix tmpparbefwm = parmp->GetBefWorldMat();//!!!!!!! 2022/12/23 引数にするべき
		//ChaMatrixInverse( &invbefpar, NULL, &tmpparbefwm );
		//ChaMatrix tmpmat = curmp->GetWorldMat() * invbefpar * parmp->GetWorldMat();
		ChaMatrix tmpmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp) * ChaMatrixInv(befparentwm) * newparentwm;
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, srcmotid, roundingframe, tmpmat, onlycheck, fromiktarget);

		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	}
	else{
		ChaMatrix tramat;
		tramat.SetIdentity();//2023/02/12
		ChaMatrixTranslation( &tramat, srctra.x, srctra.y, srctra.z );
		ChaMatrix tmpmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp) * tramat;
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, srcmotid, roundingframe, tmpmat, onlycheck, fromiktarget);

		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	}

	curmp->SetAbsMat( GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp) );

	if (m_child){
		m_child->AddBoneTraReq(limitdegflag, curmp, srcmotid, roundingframe, srctra, currentbefwm, currentnewwm);
	}
	if (m_brother && parmp){
		m_brother->AddBoneTraReq(limitdegflag, parmp, srcmotid, roundingframe, srctra, befparentwm, newparentwm);
	}

	return curmp;
}


CMotionPoint* CBone::AddBoneScaleReq(bool limitdegflag, CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srcscale, ChaMatrix befparentwm, ChaMatrix newparentwm)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	int existflag = 0;
	//CMotionPoint* curmp = AddMotionPoint(srcmotid, srcframe, &existflag);
	//if (!curmp || !existflag) {
	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if(!curmp){
		_ASSERT(0);
		return 0;
	}

	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);


	bool infooutflag = false;


	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	if (parmp) {
		//ChaMatrix invbefpar;
		//ChaMatrix tmpparbefwm = parmp->GetBefWorldMat();//!!!!!!! 2022/12/23 引数にするべき
		//ChaMatrixInverse(&invbefpar, NULL, &tmpparbefwm);
		//ChaMatrix tmpmat = curmp->GetWorldMat() * invbefpar * parmp->GetWorldMat();
		ChaMatrix tmpmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp) * ChaMatrixInv(befparentwm) * newparentwm;
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, srcmotid, roundingframe, tmpmat, onlycheck, fromiktarget);

		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	}
	else {
		ChaVector3 curpos = GetJointFPos();
		ChaMatrix beftramat;
		beftramat.SetIdentity();//2023/02/12
		ChaMatrixTranslation(&beftramat, -curpos.x, -curpos.y, -curpos.z);
		ChaMatrix afttramat;
		afttramat.SetIdentity();//2023/02/12
		ChaMatrixTranslation(&afttramat, curpos.x, curpos.y, curpos.z);
		ChaMatrix scalemat;
		scalemat.SetIdentity();//2023/02/12
		ChaMatrixScaling(&scalemat, srcscale.x, srcscale.y, srcscale.z);
		ChaMatrix tramat;
		tramat = beftramat * scalemat * afttramat;
		//ChaMatrix tmpmat = curmp->GetWorldMat() * tramat;
		ChaMatrix tmpmat = tramat * GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, srcmotid, roundingframe, tmpmat, onlycheck, fromiktarget);

		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

		////ChaVector3 cureul;
		////cureul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_BEFFRAME);
		//ChaMatrix parentmat;
		//if (GetParent()) {
		//	CMotionPoint* parentmp;
		//	parentmp = GetParent()->GetMotionPoint(srcmotid, srcframe);
		//	if (parentmp) {
		//		parentmat = parentmp->GetWorldMat();
		//	}
		//	else {
		//		ChaMatrixIdentity(&parentmat);
		//	}
		//}
		//else {
		//	ChaMatrixIdentity(&parentmat);
		//}

		//ChaMatrix curlocalmat;
		//curlocalmat = curmp->GetWorldMat() * ChaMatrixInv(parentmat);
		//ChaMatrix cursmat, currmat, curtmat;
		//GetSRTMatrix2(curlocalmat, &cursmat, &currmat, &curtmat);

		//ChaVector3 cursvec, curtvec;
		//GetSRTMatrix(curlocalmat, &cursvec, &currmat, &curtvec);

		//ChaVector3 curtraanim;
		//curtraanim = CalcLocalTraAnim(srcmotid, srcframe);
		//ChaMatrix curtanimmat;
		//ChaMatrixIdentity(&curtanimmat);
		//ChaMatrixTranslation(&curtanimmat, curtraanim.x, curtraanim.y, curtraanim.z);

		////CQuaternion rotq;
		////rotq.SetRotationXYZ(0, cureul);
		////ChaVector3 curtra;
		////curtra = CalcLocalTraAnim(srcmotid, srcframe);
		////ChaMatrix curtramat;
		////ChaMatrixIdentity(&curtramat);
		////ChaMatrixTranslation(&curtramat, curtra.x, curtra.y, curtra.z);


		//ChaVector3 curpos = GetJointFPos();
		//ChaMatrix beftramat;
		//ChaMatrixIdentity(&beftramat);
		//ChaMatrixTranslation(&beftramat, -curpos.x, -curpos.y, -curpos.z);
		//ChaMatrix afttramat;
		//ChaMatrixIdentity(&afttramat);
		//ChaMatrixTranslation(&afttramat, curpos.x, curpos.y, curpos.z);
		//ChaMatrix scalemat;
		//ChaMatrixIdentity(&scalemat);
		//ChaMatrixScaling(&scalemat, srcscale.x, srcscale.y, srcscale.z);
		////ChaMatrix tramat;
		////tramat = beftramat * scalemat * afttramat;
		//////ChaMatrix tmpmat = curmp->GetWorldMat() * tramat;
		////ChaMatrix tmpmat = tramat * curmp->GetWorldMat();
		////g_wmatDirectSetFlag = true;
		////SetWorldMat(infooutflag, 0, srcmotid, srcframe, tmpmat);
		////g_wmatDirectSetFlag = false;

		//ChaMatrix newmat;
		//newmat = beftramat * scalemat * cursmat * currmat * afttramat * curtanimmat * parentmat;

		////g_wmatDirectSetFlag = true;
		////SetWorldMat(infooutflag, 0, srcmotid, srcframe, newmat);
		////g_wmatDirectSetFlag = false;

		//curmp->SetWorldMat(newmat);


	}

	curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));

	if (m_child) {
		m_child->AddBoneScaleReq(limitdegflag, curmp, srcmotid, roundingframe, srcscale, currentbefwm, currentnewwm);
	}
	if (m_brother && parmp) {
		m_brother->AddBoneScaleReq(limitdegflag, parmp, srcmotid, roundingframe, srcscale, befparentwm, newparentwm);
	}

	return curmp;
}


CMotionPoint* CBone::PasteRotReq(bool limitdegflag, int srcmotid, double srcframe, double dstframe)
{
	//src : srcmp srcparmp
	//dst : curmp parmp

	double roundingframe = (double)((int)(srcframe + 0.0001));

	int existflag0 = 0;
	//CMotionPoint* srcmp = AddMotionPoint( srcmotid, srcframe, &existflag0 );
	//if( !existflag0 || !srcmp ){
	CMotionPoint* srcmp = GetMotionPoint(srcmotid, roundingframe);
	if (!srcmp) {
		_ASSERT( 0 );
		return 0;
	}

	int existflag = 0;
	//CMotionPoint* curmp = AddMotionPoint( srcmotid, dstframe, &existflag );
	//if( !existflag || !curmp ){
	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT( 0 );
		return 0;
	}
	
	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	curmp->SetWorldMat( GetWorldMat(limitdegflag, srcmotid, roundingframe, srcmp) );
	curmp->SetAbsMat( srcmp->GetAbsMat() );


	//オイラー角初期化
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	int paraxsiflag = 1;
	//int isfirstbone = 0;
	//cureul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO);
	cureul = CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
	SetLocalEul(limitdegflag, srcmotid, roundingframe, cureul, 0);


	if( m_child ){
		m_child->PasteRotReq(limitdegflag, srcmotid, roundingframe, dstframe);
	}
	if( m_brother ){
		m_brother->PasteRotReq(limitdegflag, srcmotid, roundingframe, dstframe);
	}
	return curmp;

}

ChaMatrix CBone::CalcNewLocalRotMatFromQofIK(bool limitdegflag, int srcmotid, double srcframe, CQuaternion qForRot, ChaMatrix* dstsmat, ChaMatrix* dstrmat, ChaMatrix* dsttanimmat)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix newlocalrotmat;
	newlocalrotmat.SetIdentity();
	if (!dstsmat || !dstrmat || !dsttanimmat) {
		_ASSERT(0);
		return newlocalrotmat;
	}

	ChaMatrix currentwm;
	//limitedworldmat = GetLimitedWorldMat(srcmotid, roundingframe);//ここをGetLimitedWorldMatにすると１回目のIKが乱れる。２回目のIK以降はOK。
	currentwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	ChaMatrix localmat;
	ChaMatrix parentwm;
	localmat.SetIdentity();
	parentwm.SetIdentity();
	CQuaternion parentq;
	CQuaternion invparentq;
	if (GetParent()) {
		parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parentq.RotationMatrix(parentwm);
		invparentq.RotationMatrix(ChaMatrixInv(parentwm));
		localmat = currentwm * ChaMatrixInv(parentwm);
	}
	else {
		parentwm.SetIdentity();
		parentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invparentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		localmat = currentwm;
	}
	
	ChaMatrix smat, rmat, tmat, tanimmat;
	//ChaMatrix zeroposmat;
	//zeroposmat.SetIdentity();
	GetSRTandTraAnim(localmat, GetNodeMat(), &smat, &rmat, &tmat, &tanimmat);//#### ローカル行列をSRTTAnim分解 ####
	//GetSRTandTraAnim(localmat, zeroposmat, &smat, &rmat, &tmat, &tanimmat);//#### ローカル行列をSRTTAnim分解 ####


	CQuaternion curq, newq;
	curq.RotationMatrix(rmat);

	CQuaternion globalq;
	globalq = parentq * curq;
	CQuaternion newglobalq;
	newglobalq = qForRot * globalq;//########### 引数rotqはグローバル姿勢 ############
	//newglobalq = globalq * rotq;
	CQuaternion newlocalrotq;
	newlocalrotq = invparentq * newglobalq;
	newlocalrotmat = newlocalrotq.MakeRotMatX();


	if (dstsmat) {
		*dstsmat = smat;
	}
	if (dstrmat) {
		*dstrmat = rmat;
	}
	if (dsttanimmat) {
		*dsttanimmat = tanimmat;
	}

	return newlocalrotmat;
}

ChaMatrix CBone::CalcNewLocalTAnimMatFromSRTraAnim(ChaMatrix srcnewlocalrotmat, 
	ChaMatrix srcsmat, ChaMatrix srcrmat, ChaMatrix srctanimmat, ChaVector3 zeroframetanim)
{
	//############# 引数traanimはローカル姿勢 ###########



	//2023/02/08
	//この関数は　RotAndTraBoneQReqからしか呼ばれていない
	//リターゲット時には　RotBoneQReqを呼ぶので　この関数は呼ばれない
	//グローバル変数による機能選択は避ける　必要な場合は引数にする


	ChaMatrix newtanimmatrotated;
	newtanimmatrotated.SetIdentity();

	//if (g_underRetargetFlag == false) {

		//traanimをローカル回転する　ただしリターゲット時以外 (IK時などに回転する)
		//newtanimvec = traanim + ChaMatrixTraVec(srctanimmat);//現在のtanimに引数で移動分を指定する場合
		ChaVector3 difftanimvec;
		difftanimvec = ChaMatrixTraVec(srctanimmat) - zeroframetanim;//現在のtanimと0frameのtanimの差分
		//difftanimvec = ChaMatrixTraVec(srctanimmat);


		CQuaternion nodeq;
		CQuaternion curq;
		CQuaternion newlocalrotq;
		CQuaternion additionallocalrotq;
		ChaMatrix additionallocalrotmat;
		nodeq.RotationMatrix(GetNodeMat());
		additionallocalrotmat.SetIdentity();
		curq.RotationMatrix(srcrmat);
		newlocalrotq.RotationMatrix(srcnewlocalrotmat);

		additionallocalrotq = curq.inverse() * newlocalrotq;//!!!!!!!
		//additionallocalrotq = nodeq * (curq.inverse() * newlocalrotq) * nodeq.inverse();//!!!!!!!
		//additionallocalrotq = nodeq.inverse() * (curq.inverse() * newlocalrotq) * nodeq;//!!!!!!!
		
		additionallocalrotmat = additionallocalrotq.MakeRotMatX();

		ChaVector3 diffanimvecrotated;
		ChaVector3TransformCoord(&diffanimvecrotated, &difftanimvec, &additionallocalrotmat);//vector回転のために　回転行列で変換
		newtanimmatrotated.SetIdentity();
		newtanimmatrotated.SetTranslation(diffanimvecrotated + zeroframetanim);
		//newtanimmatrotated.SetTranslation(diffanimvecrotated);
	//}
	//else {
	//	//リターゲット時には　traanimをそのままセットするので　traanimの回転はしない
	//	newtanimmatrotated.SetIdentity();
	//	newtanimmatrotated.SetTranslation(ChaMatrixTraVec(srctanimmat));
	//}

	return newtanimmatrotated;
}

void CBone::UpdateCurrentWM(bool limitdegflag, int srcmotid, double srcframe,
	ChaMatrix newwm)
{
	//directsetで　ツリーの姿勢を更新　再帰

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix befwm;
	befwm.SetIdentity();
	befwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	ChaMatrix befparentwm;
	befparentwm.SetIdentity();
	if (GetParent()) {
		befparentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	}
	else {
		befparentwm.SetIdentity();
	}

	bool directsetflag = true;//directset !!!
	bool infooutflag = false;
	int setchildflag = 0;
	int onlycheck = 0;
	bool fromiktarget = false;
	SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag,
		srcmotid, roundingframe, newwm, onlycheck, fromiktarget);

	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));
	}
	

	if (GetChild()) {
		bool setbroflag2 = true;
		GetChild()->UpdateParentWMReq(limitdegflag, setbroflag2, srcmotid, roundingframe,
			befwm, newwm);
	}
	//if (GetBrother() && (setbroflag == true)) {
	//	GetBrother()->UpdateParentWMReq(limitdegflag, setbroflag, srcmotid, roundingframe,
	//		befparentwm, befparentwm);
	//}
}



void CBone::UpdateParentWMReq(bool limitdegflag, bool setbroflag, int srcmotid, double srcframe, 
	ChaMatrix oldparentwm, ChaMatrix newparentwm)
{
	//directsetで　parentの姿勢を更新　再帰

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);


	currentnewwm = currentbefwm * ChaMatrixInv(oldparentwm) * newparentwm;
	

	bool directsetflag = true;//directset !!!
	bool infooutflag = false;
	int setchildflag = 0;
	int onlycheck = 0;
	bool fromiktarget = false;
	SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag,
		srcmotid, roundingframe, currentnewwm, onlycheck, fromiktarget);

	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));
	}

	if (GetChild()) {
		bool setbroflag2 = true;
		GetChild()->UpdateParentWMReq(limitdegflag, setbroflag2, srcmotid, roundingframe, 
			currentbefwm, currentnewwm);
	}
	if (GetBrother() && (setbroflag == true)) {
		GetBrother()->UpdateParentWMReq(limitdegflag, setbroflag, srcmotid, roundingframe,
			oldparentwm, newparentwm);
	}
}




//##########################################################
//CBone::RotBoneQReq()
//引数rotqはグローバル回転　引数traanimはローカル移動アニメ
//##########################################################
CMotionPoint* CBone::RotBoneQReq(bool limitdegflag, bool infooutflag, 
	CBone* parentbone, int srcmotid, double srcframe, 
	CQuaternion rotq, ChaMatrix srcbefparentwm, ChaMatrix srcnewparentwm,
	CBone* bvhbone, ChaVector3 traanim)// , int setmatflag, ChaMatrix* psetmat, bool onretarget)
{
	//##############################################################
	//Retarget専用. IK用にはRotAndTraBoneQReq()を使用
	//##############################################################

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT( 0 );
		return 0;
	}
	
	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	//初回呼び出し

	ChaMatrix newlocalrotmat;
	ChaMatrix smat, rmat, tmat, tanimmat;
	newlocalrotmat.SetIdentity();
	smat.SetIdentity();
	rmat.SetIdentity();
	tmat.SetIdentity();
	tanimmat.SetIdentity();
	newlocalrotmat = CalcNewLocalRotMatFromQofIK(limitdegflag, srcmotid, roundingframe, rotq, &smat, &rmat, &tanimmat);

	//ChaMatrix newtanimmatrotated;
	//newtanimmatrotated.SetIdentity();
	//newtanimmatrotated = CalcNewLocalTAnimMatFromQofIK(srcmotid, roundingframe, newlocalrotmat, smat, rmat, tanimmat, parentwm);

	ChaMatrix bvhtraanim;
	bvhtraanim.SetIdentity();
	bvhtraanim.SetTranslation(ChaMatrixTraVec(tanimmat) + traanim);//元のtanim + 引数traanim

	//#### SRTAnimからローカル行列組み立て ####
	ChaMatrix newlocalmat;
	//newlocalmat = ChaMatrixFromSRTraAnim(true, true, GetNodeMat(), &smat, &newlocalrotmat, &newtanimmatrotated);
	newlocalmat = ChaMatrixFromSRTraAnim(true, true, GetNodeMat(), &smat, &newlocalrotmat, &bvhtraanim);
	ChaMatrix newwm;
	if (GetParent()) {
		ChaMatrix parentwm;
		parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newwm = newlocalmat * parentwm;//globalにする
	}
	else {
		newwm = newlocalmat;
	}
	

	bool directsetflag = false;
	int setchildflag = 0;
	int onlycheck = 0;
	bool fromiktarget = false;
	SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, 
		srcmotid, roundingframe, newwm, onlycheck, fromiktarget);

	if (bvhbone){
		//bvhbone->SetTmpMat(tmpmat);
		bvhbone->SetTmpMat(newwm);
	}

	currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		

	curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));


	if (m_child && curmp){
		bool setbroflag2 = true;
		m_child->UpdateParentWMReq(limitdegflag, setbroflag2, srcmotid, roundingframe,
			currentbefwm, currentnewwm);
	}
	if (m_brother && parentbone){
		bool setbroflag3 = true;
		m_child->UpdateParentWMReq(limitdegflag, setbroflag3, srcmotid, roundingframe,
			srcbefparentwm, srcnewparentwm);
	}
	return curmp;
}


int CBone::SaveSRT(bool limitdegflag, int srcmotid, double srcframe)
{
	double curframe = (double)((int)(srcframe + 0.0001));
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, curframe);
	if (curmp) {
		ChaMatrix curwm, parentwm, localmat;
		curwm = GetWorldMat(limitdegflag, srcmotid, curframe, curmp);
		if (GetParent()) {
			parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, curframe, 0);
			localmat = curwm * ChaMatrixInv(parentwm);
		}
		else {
			parentwm.SetIdentity();
			localmat = curwm;
		}

		ChaMatrix smat, rmat, tmat, tanimmat;
		smat.SetIdentity();
		rmat.SetIdentity();
		tmat.SetIdentity();
		tanimmat.SetIdentity();
		GetSRTandTraAnim(localmat, GetNodeMat(), &smat, &rmat, &tmat, &tanimmat);
		curmp->SetSaveSRTandTraAnim(smat, rmat, tmat, tanimmat);
	}
	return 0;
}



//###############################
//2023/03/04 ismovableをリターン
//###############################
int CBone::RotAndTraBoneQReq(bool limitdegflag, int* onlycheckptr,
	double srcstartframe, bool infooutflag, CBone* parentbone, int srcmotid, double srcframe,
	CQuaternion qForRot, CQuaternion qForHipsRot, bool fromiktarget)
{
	//######################################
	//IK用.　RetargetはRotBoneQReq()を使用
	//hipsはtanimを qForHipsRot で回転する
	//hips以外はtanimを qForRot　で回転する
	//######################################

	//###################################################################
	//onlycheckptr != NULLの場合には
	//SetWorldMatをonlycheckで呼び出して　回転可能かどうかだけを調べる
	//初回呼び出し時のCBoneに対してだけチェックをして　直ちにリターンする
	//###################################################################


	
	int ismovable = 1;//for return value


	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 0;
	}

	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	//初回呼び出し
	bool ishipsjoint;
	ishipsjoint = IsHipsBone();


	ChaMatrix currentwm;
	//limitedworldmat = GetLimitedWorldMat(srcmotid, srcframe);//ここをGetLimitedWorldMatにすると１回目のIKが乱れる。２回目のIK以降はOK。
	currentwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	ChaMatrix parentwm;
	CQuaternion parentq;
	CQuaternion invparentq;
	if (GetParent()) {
		parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parentq.RotationMatrix(parentwm);
		invparentq.RotationMatrix(ChaMatrixInv(parentwm));
	}
	else {
		parentwm.SetIdentity();
		parentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invparentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}

	//Get startframeframe traanim : SRT保存はCModel::IKRotate* から呼び出すCBone::SaveSRT()で行っている
	//ChaVector3 startframetraanim = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaMatrix startframetraanimmat;
	startframetraanimmat.SetIdentity();
	{
		//CMotionPoint* zeromp = GetMotionPoint(srcmotid, 0.0);
		CMotionPoint* startframemp = GetMotionPoint(srcmotid, (double)((int)(srcstartframe + 0.0001)));
		if (startframemp) {
			ChaMatrix smat0, rmat0, tmat0, tanimmat0;
			smat0.SetIdentity();
			rmat0.SetIdentity();
			tmat0.SetIdentity();
			tanimmat0.SetIdentity();
			//CModel::IKRotate* から呼び出したCBone::SaveSRT()で保存したSRTを取得
			startframemp->GetSaveSRTandTraAnim(&smat0, &rmat0, &tmat0, &tanimmat0);
			startframetraanimmat = tanimmat0;
		}
		else {
			startframetraanimmat.SetIdentity();
		}
	}
	//ChaMatrix currenttraanimmat;
	//curmp->GetSaveSRTandTraAnim(0, 0, 0, &currenttraanimmat);



	ChaMatrix newwm;
	newwm.SetIdentity();


	//###########################################################################################################
	//2022/12/29 Memo
	//Hipsのときには　追加分の回転を後ろから掛ける
	//その際にTraAnimよりも後ろから掛けることにより TraAnimを回転する
	// 
	//Hips以外の時には　qForRotの内容にトリックがあって　追加分の回転を "実質的には"前から掛けている
	//この場合　TraAnimだけを別途回転してセットする必要がある
	// 
	//なぜ　Hips以外の時にHipsと同じシンプルな式を使えないかというと
	//体全体を回転した時などに　体に対する回転の向きを維持する必要があるので　後ろから掛けることが出来ないため
	//###########################################################################################################


	if (ishipsjoint == true) {

		//############
		//Hips Joint
		//############


		//#############################################################################################################################
		//2022/12/27
		//hispについて　移動も回転するには　について
		//InvCurNodeTra * curS * curR * CurNodeTra * TAnim * ParentWM に対して　回転qForHipsRotを加え　curTAnimも回転するには
		//イメージとしては　curwmの親の位置に　qForHipsRot処理を加えるイメージ
		//実際には
		//curTAnimとCurNodeTraは両方とも移動成分のみであるから可換であるから
		//(InvCurNodeTra * curS * curR * TAnim * CurNodeTra) * InvCurNodeTra * qForHipsRot * CurNodeTra * ParentWM
		//currentwm * InvCurNode * qForHipsRot * CurNodeTra * ParentWM
		//#############################################################################################################################
		//newwm = currentwm * ChaMatrixInv(parentwm) *
		//	ChaMatrixInv(ChaMatrixTra(GetNodeMat())) * qForHipsRot.MakeRotMatX() * ChaMatrixTra(GetNodeMat()) *
		//	parentwm;

		newwm = currentwm * ChaMatrixInv(parentwm) *
			ChaMatrixInv(ChaMatrixTra(GetNodeMat())) * ChaMatrixInv(startframetraanimmat) * qForHipsRot.MakeRotMatX() * ChaMatrixTra(GetNodeMat()) * startframetraanimmat * 
			parentwm;

		if (onlycheckptr) {
			bool directsetflag = false;
			int onlycheckflag = 1;
			ismovable = SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
			*onlycheckptr = ismovable;
			//if (ismovable == 0) {
			//	return curmp;// not movableの場合は　印を付けて　直ちにリターンする
			//}
			return ismovable;//onlycheckptr != NULLの場合は　初回呼び出しでmovableチェックして直ちにリターン
		}
		else {
			bool directsetflag = false;
			int onlycheckflag = 0;
			int setchildflag = 1;
			ismovable = SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, 
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
		}		
		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	}
	else {

	//###############################################
	//other joints !!!! traanimを qForRot で回転する
	//###############################################


		////以下３行　hipsと同じようにすると　traanimが設定してあるジョイントで　回転軸がマニピュレータと合わない
		//newwm = currentwm * ChaMatrixInv(parentwm) *
		//	ChaMatrixInv(ChaMatrixTra(GetNodeMat())) * ChaMatrixInv(startframetraanimmat) * qForRot.MakeRotMatX() * ChaMatrixTra(GetNodeMat()) * startframetraanimmat *
		//	parentwm;



	//########################################################################
	//2023/01/14
	//指のRigでテストしたところ 制限角度有りの場合に　traanimが不正になった
	//２段階に分けて計算することにより解決
	//########################################################################

	//#############################################################
	//２段階処理の１段目：回転だけを変更して確定するための　１段目
	//#############################################################
		////calc new local rot
		ChaMatrix newlocalrotmatForRot;
		ChaMatrix smatForRot, rmatForRot, tmatForRot, tanimmatForRot;
		newlocalrotmatForRot.SetIdentity();
		smatForRot.SetIdentity();
		rmatForRot.SetIdentity();
		tmatForRot.SetIdentity();
		tanimmatForRot.SetIdentity();
		newlocalrotmatForRot = CalcNewLocalRotMatFromQofIK(limitdegflag, srcmotid, roundingframe, qForRot, &smatForRot, &rmatForRot, &tanimmatForRot);

		ChaMatrix newtanimmatrotated;
		newtanimmatrotated = tanimmatForRot;//１段目では　traanimを 回転しない

		////	//traanimを 回転しないとき
		////	newlocalrotmatForHipsRot = newlocalrotmatForRot;
		////	newtanimmatrotated = tanimmatForRot;


		//#### SRTAnimからローカル行列組み立て ####
		ChaMatrix newlocalmat;
		newlocalmat = ChaMatrixFromSRTraAnim(true, true, GetNodeMat(),
			&smatForRot, &newlocalrotmatForRot, &newtanimmatrotated);//ForRot
		//newwm = newlocalmat * parentwmForRot;//globalにする
		if (GetParent()) {
			newwm = newlocalmat * parentwm;//globalにする
		}
		else {
			newwm = newlocalmat;
		}

		if (onlycheckptr) {
			bool directsetflag = false;
			int onlycheckflag = 1;
			ismovable = SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
			*onlycheckptr = ismovable;
			//if (ismovable == 0) {
			//	return curmp;// not movableの場合は　印を付けて　直ちにリターンする
			//}


			//onlycheckの場合は　ここまで
			return ismovable;

		}
		else {
			bool directsetflag = false;
			int onlycheckflag = 0;
			int setchildflag = 1;//2023/02/12 ２段目の前に再帰する必要
			ismovable = SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, 
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
		}
		currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	////#####################################################################################################
	////２段階処理の２段目：角度制限オンオフを考慮し　回転を確定させた後　移動アニメを回転するための　２段目
	////#####################################################################################################


		//2023/01/22 制限角度と一緒に使うと　操作ごとに誤差が蓄積するので　オプションにした
		if ((ismovable == 1) && g_rotatetanim) {
			////calc new local rot
			ChaMatrix tmplocalmat;
			tmplocalmat = currentnewwm * ChaMatrixInv(parentwm);

			//ChaMatrix newlocalrotmatForRot2;
			ChaMatrix smatForRot2, rmatForRot2, tmatForRot2, tanimmatForRot2;
			//newlocalrotmatForRot2.SetIdentity();
			smatForRot2.SetIdentity();
			rmatForRot2.SetIdentity();
			tmatForRot2.SetIdentity();
			tanimmatForRot2.SetIdentity();

			//newlocalrotmatForRot = CalcNewLocalRotMatFromQofIK(srcmotid, roundingframe, qForRot, &smatForRot, &rmatForRot, &tanimmatForRot);
			GetSRTandTraAnim(tmplocalmat, GetNodeMat(), &smatForRot2, &rmatForRot2, &tmatForRot2, &tanimmatForRot2);


			//２段目では　確定した回転によりtraanimを回転する
			ChaMatrix newtanimmatrotated2;
			newtanimmatrotated2 = CalcNewLocalTAnimMatFromSRTraAnim(rmatForRot2,
				smatForRot, rmatForRot, tanimmatForRot, ChaMatrixTraVec(startframetraanimmat));

			////	//traanimを 回転しないとき
			////	newlocalrotmatForHipsRot = newlocalrotmatForRot;
			////	newtanimmatrotated = tanimmatForRot;


			//#### SRTAnimからローカル行列組み立て ####
			ChaMatrix newlocalmat2;
			newlocalmat2 = ChaMatrixFromSRTraAnim(true, true, GetNodeMat(),
				&smatForRot, &rmatForRot2, &newtanimmatrotated2);//ForRot
			//newwm = newlocalmat * parentwmForRot;//globalにする
			if (GetParent()) {
				newwm = newlocalmat2 * parentwm;//globalにする
			}
			else {
				newwm = newlocalmat2;
			}
			
			bool directsetflag2 = true;
			int setchildflag2 = 1;//2023/02/12
			int onlycheck2 = 0;
			SetWorldMat(limitdegflag, directsetflag2, infooutflag, setchildflag2, 
				srcmotid, roundingframe, newwm, onlycheck2, fromiktarget);
			currentnewwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		}

	}

	curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));

	//2023/02/12 setchildflag = 1で処理することに
	//if (ismovable == 1) {
	//	if (m_child && curmp) {
	//		bool setbroflag2 = true;
	//		m_child->UpdateParentWMReq(limitdegflag, setbroflag2, srcmotid, roundingframe,
	//			currentbefwm, currentnewwm);
	//	}
	//	if (m_brother && parentbone) {
	//		bool setbroflag3 = true;
	//		m_child->UpdateParentWMReq(limitdegflag, setbroflag3, srcmotid, roundingframe,
	//			srcbefparentwm, srcnewparentwm);
	//	}
	//}

	return ismovable;
}


//CMotionPoint* CBone::RotBoneQCurrentReq(bool infooutflag, CBone* parbone, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone, ChaVector3 traanim, int setmatflag, ChaMatrix* psetmat)
//{
//	int existflag = 0;
//	//CMotionPoint* curmp = AddMotionPoint(srcmotid, srcframe, &existflag);
//	//if (!existflag || !curmp) {
//	//	_ASSERT(0);
//	//	return 0;
//	//}
//	CMotionPoint* curmp = &m_curmp;
//
//	ChaMatrix smat, rmat, tmat;
//	GetSRTMatrix2(curmp->GetWorldMat(), &smat, &rmat, &tmat);
//
//	if (parbone) {
//		//再帰から呼び出し
//		//ChaMatrix befparmat = parmp->GetBefWorldMat();
//		CMotionPoint* befparmp = parbone->GetMotionPoint(srcmotid, (double)((int)(srcframe + 0.1)));
//		if (befparmp) {
//			ChaMatrix befparmat = befparmp->GetWorldMat();
//			ChaMatrix newparmat = parbone->GetCurMp().GetWorldMat();
//			//if ((g_underRetargetFlag == true) || (IsSameMat(befparmat, newparmat) == 0)){
//			ChaMatrix invbefpar;
//			ChaMatrixInverse(&invbefpar, NULL, &befparmat);
//			ChaMatrix tmpmat = curmp->GetWorldMat() * invbefpar * newparmat;
//
//			//g_wmatDirectSetFlag = true;
//			//SetWorldMat(infooutflag, 0, srcmotid, srcframe, tmpmat);
//			//g_wmatDirectSetFlag = false;
//			curmp->SetWorldMat(tmpmat);
//		}
//		//}
//	//else{
//		//parmatに変化がないときは変更しない。
//	//	curmp->SetBefWorldMat( curmp->GetWorldMat() );
//	//}
//		//if (bvhbone) {
//		//	if (m_child) {
//		//		bvhbone->SetTmpMat(curmp->GetWorldMat());
//		//	}
//		//	else {
//		//		bvhbone->SetTmpMat(newparmat);
//		//	}
//		//}
//	}
//	else {
//		//初回呼び出し
//		ChaMatrix tramat;
//		ChaMatrixIdentity(&tramat);
//		ChaMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);
//
//		if (m_child) {
//			if (setmatflag == 0) {
//				ChaVector3 rotcenter;// = m_childworld;
//				ChaVector3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));
//				ChaMatrix befrot, aftrot;
//				ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//				ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//				ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
//
//				ChaMatrix tmpmat0 = curmp->GetWorldMat() * rotmat;// *tramat;
//				ChaVector3 tmppos;
//				ChaVector3TransformCoord(&tmppos, &(GetJointFPos()), &tmpmat0);
//				ChaVector3 diffvec;
//				diffvec = rotcenter - tmppos;
//				ChaMatrix tmptramat;
//				ChaMatrixIdentity(&tmptramat);
//				ChaMatrixTranslation(&tmptramat, diffvec.x, diffvec.y, diffvec.z);
//				ChaMatrix tmpmat;
//				tmpmat = tmpmat0 * tmptramat * tramat;
//
//				////directflagまたはunderRetargetFlagがないときはtramat成分は無視され、SetWorldMatFromEul中でbone::CalcLocalTraAnimの値が適用される。
//				//SetWorldMat(infooutflag, 0, srcmotid, srcframe, tmpmat);
//				curmp->SetWorldMat(tmpmat);
//
//				
//				//if (bvhbone) {
//				//	bvhbone->SetTmpMat(tmpmat);
//				//}
//			}
//			else {
//				ChaMatrix tmpmat = *psetmat;
//				//g_wmatDirectSetFlag = true;
//				//SetWorldMat(infooutflag, 0, srcmotid, srcframe, tmpmat);
//				//g_wmatDirectSetFlag = false;
//				curmp->SetWorldMat(tmpmat);
//
//				//if (bvhbone) {
//				//	bvhbone->SetTmpMat(tmpmat);
//				//}
//			}
//		}
//		else {
//			ChaVector3 rotcenter;// = m_childworld;
//			ChaVector3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));
//			ChaMatrix befrot, aftrot;
//			ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
//			ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
//			ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
//			//ChaMatrix tmpmat = curmp->GetWorldMat() * rotmat * tramat;
//			//ChaMatrix tmpmat = GetS0RTMatrix(curmp->GetWorldMat()) * rotmat * tramat;
//
//			ChaMatrix tmpmat0 = curmp->GetWorldMat() * rotmat;// *tramat;
//			ChaVector3 tmppos;
//			ChaVector3TransformCoord(&tmppos, &(GetJointFPos()), &tmpmat0);
//			ChaVector3 diffvec;
//			diffvec = rotcenter - tmppos;
//			ChaMatrix tmptramat;
//			ChaMatrixIdentity(&tmptramat);
//			ChaMatrixTranslation(&tmptramat, diffvec.x, diffvec.y, diffvec.z);
//			ChaMatrix tmpmat;
//			tmpmat = tmpmat0 * tmptramat * tramat;
//
//
//			//g_wmatDirectSetFlag = true;//!!!!!!!!
//			//SetWorldMat(infooutflag, 0, srcmotid, srcframe, tmpmat);
//			//g_wmatDirectSetFlag = false;//!!!!!!!
//
//			curmp->SetWorldMat(tmpmat);
//
//			//if (bvhbone) {
//			//	bvhbone->SetTmpMat(tmpmat);
//			//}
//
//
//		}
//	}
//
//
//	curmp->SetAbsMat(curmp->GetWorldMat());
//
//	if (m_child && curmp) {
//		m_child->RotBoneQCurrentReq(infooutflag, this, srcmotid, srcframe, rotq);
//	}
//	if (m_brother && parbone) {
//		m_brother->RotBoneQCurrentReq(infooutflag, parbone, srcmotid, srcframe, rotq);
//	}
//	return curmp;
//}





CMotionPoint* CBone::RotBoneQOne(bool limitdegflag, CBone* srcparentbone, CMotionPoint* parmp, int srcmotid, double srcframe, ChaMatrix srcmat)
{
	if (!srcparentbone) {
		_ASSERT(0);
		return 0;
	}


	double roundingframe = (double)((int)(srcframe + 0.0001));

	int existflag = 0;
	//CMotionPoint* curmp = AddMotionPoint(srcmotid, srcframe, &existflag);
	//if (!existflag || !curmp){
	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 0;
	}

	bool infooutflag = false;

	if (parmp){
		//parentの行列をセット !!!!!!!!!
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, srcmotid, roundingframe, 
			srcparentbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, parmp),
			onlycheck, fromiktarget);
	} else{
		bool directsetflag = true;
		int onlycheck = 0;
		bool fromiktarget = false;
		SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
			srcmotid, roundingframe, srcmat,
			onlycheck, fromiktarget);
	}

	curmp->SetAbsMat(GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));

	return curmp;
}


CMotionPoint* CBone::SetAbsMatReq(bool limitdegflag, int broflag, 
	int srcmotid, double srcframe, double firstframe )
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	int existflag = 0;
	//CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	//if( !existflag || !curmp ){
	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT( 0 );
		return 0;
	}

	int existflag2 = 0;
	//CMotionPoint* firstmp = AddMotionPoint( srcmotid, firstframe, &existflag2 );
	//if( !existflag2 || !firstmp ){
	CMotionPoint* firstmp = GetMotionPoint(srcmotid, roundingframe);
	if (!firstmp) {
		_ASSERT( 0 );
		return 0;
	}

	bool infooutflag = false;
	bool directsetflag = true;
	int onlycheck = 0;
	bool fromiktarget = false;
	SetWorldMat(limitdegflag, directsetflag, infooutflag, 0, 
		srcmotid, roundingframe, firstmp->GetAbsMat(),
		onlycheck, fromiktarget);

	if( m_child ){
		m_child->SetAbsMatReq(limitdegflag, 1, srcmotid, roundingframe, firstframe);
	}
	if( m_brother && broflag ){
		m_brother->SetAbsMatReq(limitdegflag, 1, srcmotid, roundingframe, firstframe);
	}
	return curmp;
}



int CBone::DestroyMotionKey( int srcmotid )
{
	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
		_ASSERT(0);
		return 1;
	}

	CMotionPoint* curmp = m_motionkey[srcmotid - 1];
	while( curmp ){
		CMotionPoint* nextmp = curmp->GetNext();
		
		//delete curmp;
		CMotionPoint::InvalidateMotionPoint(curmp);


		curmp = nextmp;
	}

	m_motionkey[srcmotid - 1] = NULL;



	std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
	itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
	if (itrvecmpmap != m_indexedmotionpoint.end()) {
		(itrvecmpmap->second).clear();
	}

	m_initindexedmotionpoint[srcmotid] = false;


	return 0;
}


int CBone::AddBoneMarkIfNot( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double curframe, int flag )
{
	if ((motid <= 0) || (motid > m_motionkey.size())) {
		_ASSERT(0);
		return 1;
	}

	map<double, int> curmark;
	map<int, map<double, int>>::iterator itrcur;
	itrcur = m_motmark.find( motid - 1 );//2021/08/26
	if( itrcur == m_motmark.end() ){
		curmark.clear();
	}else{
		curmark = itrcur->second;
	}
	map<double, int>::iterator itrmark;
	itrmark = curmark.find( curframe );
	if( itrmark == curmark.end() ){
		curmark[ curframe ] = flag;
		m_motmark[motid - 1] = curmark;
	}

	return 0;
}

int CBone::DelBoneMarkRange( int motid, OrgWinGUI::OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe )
{
	if ((motid <= 0) || (motid > m_motionkey.size())) {
		_ASSERT(0);
		return 1;
	}

	map<int, map<double, int>>::iterator itrcur;
	itrcur = m_motmark.find( motid - 1 );//2021/08/26
	if( itrcur == m_motmark.end() ){
		return 0;
	}

	map<double, int> curmark;
	curmark = itrcur->second;

	double frame;
	for( frame = (startframe + 1.0); frame <= (endframe - 1.0); frame += 1.0 ){
		map<double, int>::iterator itrfind;
		itrfind = curmark.find( frame );
		if( itrfind != curmark.end() ){
			curmark.erase( itrfind );
		}
	}

	m_motmark[motid - 1] = curmark;

	return 0;
}

int CBone::AddBoneMotMark( int motid, OWP_Timeline* owpTimeline, int curlineno, double startframe, double endframe, int flag )
{
	if( startframe != endframe ){
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, startframe, flag );
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, endframe, flag );
		DelBoneMarkRange( motid, owpTimeline, curlineno, startframe, endframe );
	}else{
		AddBoneMarkIfNot( motid, owpTimeline, curlineno, startframe, flag );
	}

	return 0;
}

int CBone::CalcLocalInfo(bool limitdegflag, int motid, double frameno, CMotionPoint* pdstmp )
{
	double roundingframe = (double)((int)(frameno + 0.0001));

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(motid, roundingframe);
	if(GetParent()){
		if( pcurmp ){
			pparmp = GetParent()->GetMotionPoint(motid, roundingframe);
			if( pparmp ){
				CMotionPoint setmp;
				ChaMatrix invpar = ChaMatrixInv(GetParent()->GetWorldMat(limitdegflag, motid, roundingframe, pparmp));
				ChaMatrix localmat = GetWorldMat(limitdegflag, motid, roundingframe, pcurmp) * invpar;
				//pcurmp->CalcQandTra(localmat, this);
				setmp.CalcQandTra(localmat, this);

				int inirotcur, inirotpar;
				inirotcur = IsInitRot(GetWorldMat(limitdegflag, motid, roundingframe, pcurmp));
				inirotpar = IsInitRot(GetParent()->GetWorldMat(limitdegflag, motid, roundingframe, pparmp));
				if (inirotcur && inirotpar){
					CQuaternion iniq;
					iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
					//pcurmp->SetQ(iniq);
					setmp.SetQ(iniq);
				}

				*pdstmp = setmp;
			}else{
				CMotionPoint inimp;
				*pdstmp = inimp;
				//_ASSERT( 0 );
				return 0;
			}
		}else{
			CMotionPoint inimp;
			*pdstmp = inimp;

			//_ASSERT( 0 );
			return 0;
		}
	}else{
		if( pcurmp ){
			CMotionPoint setmp;
			ChaMatrix localmat = GetWorldMat(limitdegflag, motid, roundingframe, pcurmp);
			setmp.CalcQandTra( localmat, this );

			int inirotcur;
			inirotcur = IsInitRot(GetWorldMat(limitdegflag, motid, roundingframe, pcurmp));
			if (inirotcur ){
				CQuaternion iniq;
				iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
				setmp.SetQ(iniq);
			}

			*pdstmp = setmp;

		}else{
			CMotionPoint inimp;
			*pdstmp = inimp;

			//_ASSERT( 0 );
			return 0;
		}
	}

	return 0;
}

int CBone::CalcCurrentLocalInfo(CMotionPoint* pdstmp)
{
	CMotionPoint curmp;
	CMotionPoint parmp;
	
	curmp = GetCurMp();

	//2023/02/02
	//m_curmp, GetCurMp()のworldmatには　例外的にモデルのworldmatが掛かっている
	//アニメーションの計算には　GetWorldMatではなく GetAnimMatを使う

	if (GetParent()) {
		parmp = GetParent()->GetCurMp();
		CMotionPoint setmp;
		ChaMatrix invpar = ChaMatrixInv(parmp.GetAnimMat());
		ChaMatrix localmat = curmp.GetAnimMat() * invpar;
		setmp.CalcQandTra(localmat, this);

		int inirotcur, inirotpar;
		inirotcur = IsInitRot(curmp.GetAnimMat());
		inirotpar = IsInitRot(parmp.GetAnimMat());
		if (inirotcur && inirotpar) {
			CQuaternion iniq;
			iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			//pcurmp->SetQ(iniq);
			setmp.SetQ(iniq);
		}

		setmp.SetFrame(m_curmp.GetFrame());
		*pdstmp = setmp;
	}
	else {
		CMotionPoint setmp;
		ChaMatrix localmat = curmp.GetAnimMat();//GetParent() == NULLのときのローカル
		setmp.CalcQandTra(localmat, this);

		int inirotcur;
		inirotcur = IsInitRot(localmat);
		if (inirotcur) {
			CQuaternion iniq;
			iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			setmp.SetQ(iniq);
		}

		setmp.SetFrame(m_curmp.GetFrame());
		*pdstmp = setmp;
	}

	return 0;
}

int CBone::CalcBtLocalInfo(CMotionPoint* pdstmp)
{
	if (m_parent) {
		CMotionPoint setmp;
		ChaMatrix parentbtmat;
		ChaMatrix currentbtmat;
		ChaMatrix localbtmat;
		parentbtmat = m_parent->GetBtMat();
		currentbtmat = GetBtMat();
		localbtmat = currentbtmat * ChaMatrixInv(parentbtmat);

		setmp.CalcQandTra(localbtmat, this);

		int inirotcur, inirotpar;
		inirotcur = IsInitRot(currentbtmat);
		inirotpar = IsInitRot(parentbtmat);
		if (inirotcur && inirotpar) {
			CQuaternion iniq;
			iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			//pcurmp->SetQ(iniq);
			setmp.SetQ(iniq);
		}

		*pdstmp = setmp;
	}
	else {
		CMotionPoint setmp;
		ChaMatrix localbtmat;
		localbtmat = GetBtMat();

		setmp.CalcQandTra(localbtmat, this);

		int inirotcur;
		inirotcur = IsInitRot(localbtmat);
		if (inirotcur) {
			CQuaternion iniq;
			iniq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			setmp.SetQ(iniq);
		}

		*pdstmp = setmp;

	}

	return 0;
}



//int CBone::CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp)
//{
//	double roundingframe = (double)((int)(frameno + 0.0001));
//
//	CMotionPoint* pcurmp = 0;
//	CMotionPoint* pparmp = 0;
//	pcurmp = GetMotionPoint(motid, roundingframe);
//	if (GetParent()){
//		if (pcurmp){
//			pparmp = GetParent()->GetMotionPoint(motid, roundingframe);
//			if (pparmp){
//				ChaMatrix invpar = ChaMatrixInv(GetParent()->GetWorldMat(motid, roundingframe, pparmp));
//				ChaMatrix invinitmat = GetInvInitMat();
//				ChaMatrix localmat = invinitmat * GetWorldMat(motid, roundingframe, pcurmp) * invpar;//world == init * local * parだからlocalを計算するには、invinit * world * invpar。
//				pcurmp->CalcQandTra(localmat, this);
//			}
//			else{
//				_ASSERT(0);
//				return 0;
//			}
//		}
//		else{
//			_ASSERT(0);
//			return 0;
//		}
//	}
//	else{
//		if (pcurmp){
//			ChaMatrix invinitmat = GetInvInitMat();
//			ChaMatrix localmat = invinitmat * GetWorldMat(motid, roundingframe, pcurmp);
//			pcurmp->CalcQandTra(localmat, this);
//		}
//		else{
//			_ASSERT(0);
//			return 0;
//		}
//	}
//
//	*pdstmp = *pcurmp;
//
//	return 0;
//}


int CBone::GetBoneNum()
{
	int retnum = 0;

	if( !m_child ){
		return 0;
	}else{
		retnum++;
	}

	CBone* cbro = m_child->m_brother;
	while( cbro ){
		retnum++;
		cbro = cbro->m_brother;
	}

	return retnum;
}

int CBone::CalcFirstFrameBonePos(ChaMatrix srcmat)
{
	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3TransformCoord(&m_firstframebonepos, &zeropos, &srcmat);


	//ChaVector3 jpos = GetJointFPos();
	//ChaVector3TransformCoord(&m_firstframebonepos, &jpos, &srcmat);
	
	
	//ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);//2022/07/29
	//ChaVector3TransformCoord(&m_firstframebonepos, &zeropos, &(GetFirstMat()));


	//if ((m_firstframebonepos.x == 0.0f) && (m_firstframebonepos.y == 0.0f) && (m_firstframebonepos.z == 0.0f)){
	//	_ASSERT(0);
	//}
	return 0;
}

/*
void CBone::CalcFirstAxisMatX()
{
	ChaVector3 curpos;
	ChaVector3 childpos;

	if (m_parent){
		CalcAxisMatX_aft(m_parent->GetJointFPos(), GetJointFPos(), &m_firstaxismatX);
	}

}
*/
void CBone::CalcFirstAxisMatZ()
{
	ChaVector3 curpos;
	ChaVector3 childpos;

	if (m_parent){
		CalcAxisMatZ_aft(m_parent->GetJointFPos(), GetJointFPos(), &m_firstaxismatZ);
	}

}

int CBone::CalcBoneDepth()
{
	int retdepth = 0;
	CBone* curbone = this;
	if (curbone){
		while (curbone->GetParent()){
			retdepth++;
			curbone = curbone->GetParent();
		}
	}
	return retdepth;
}

ChaVector3 CBone::GetJointFPos()
{
	if (m_parmodel) {
		if (m_parmodel->GetOldAxisFlagAtLoading() == 0) {
			return m_jointfpos;
		}
		else {
			return m_oldjointfpos;
		}
	}
	else {
		return ChaVector3(0.0f, 0.0f, 0.0f);
	}

}
void CBone::SetJointFPos(ChaVector3 srcpos)
{ 
	m_jointfpos = srcpos; 
}
void CBone::SetOldJointFPos(ChaVector3 srcpos){
	m_oldjointfpos = srcpos;
}

ChaVector3 CBone::GetBefEul(bool limitdegflag, int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);

	//1つ前のフレームのEULはすでに計算されていると仮定する。
	double befframe;
	befframe = roundingframe - 1.0;
	if (roundingframe <= 1.01) {
		//roundingframe が0.0または1.0の場合 
		//befeul = ChaVector3(0.0f, 0.0f, 0.0f);
		CMotionPoint* curmp;
		curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			befeul = GetLocalEul(limitdegflag, srcmotid, roundingframe, curmp);
		}
	}
	else {
		CMotionPoint* befmp;
		befmp = GetMotionPoint(srcmotid, befframe);
		if (befmp) {
			befeul = GetLocalEul(limitdegflag, srcmotid, befframe, befmp);
		}
	}

	//if (g_underIKRot == true) {
	//	if (roundingframe <= 1.01) {
	//		befeul = ChaVector3(0.0f, 0.0f, 0.0f);
	//	}
	//}

	return befeul;
}

ChaVector3 CBone::GetUnlimitedBefEul(int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);

	//1つ前のフレームのEULはすでに計算されていると仮定する。
	double befframe;
	befframe = roundingframe - 1.0;
	if (roundingframe <= 1.01) {
		//befeul = ChaVector3(0.0f, 0.0f, 0.0f);
		CMotionPoint* curmp;
		curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			befeul = curmp->GetLocalEul();//unlimited !!!
		}
	}
	else {
		CMotionPoint* befmp;
		befmp = GetMotionPoint(srcmotid, befframe);
		if (befmp) {
			befeul = befmp->GetLocalEul();//unlimited !!!
		}
	}

	//if (g_underIKRot == true) {
	//	if (roundingframe <= 1.01) {
	//		befeul = ChaVector3(0.0f, 0.0f, 0.0f);
	//	}
	//}

	return befeul;
}

int CBone::GetNotModify180Flag(int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	//2023/02/04
	//ModifyEuler360()の内容を変えたので　全フレームmodifyする
	//int notmodify180flag = 0;


	//2023/01/14
	//rootjointを２回転する場合など　180度補正は必要(１フレームにつき165度までの変化しか出来ない制限は必要)
	//しかし　bvh2fbxなど　１フレームにアニメが付いているデータでうまくいくようにするために　0フレームと１フレームは除外
	int notmodify180flag = 1;
	if (g_underIKRot == false) {
		if (roundingframe <= 1.01) {
			//0フレームと１フレームは　180度ずれチェックをしない
			notmodify180flag = 1;
		}
		else {
			notmodify180flag = 0;
		}
	}
	else {
		//2023/01/26
		//IKRot中は　０フレームも１フレームも　180度チェックをする
		notmodify180flag = 0;
	}


	////2023/02/03
	////CalcLocalEulXYZに渡すbefeulを変更した
	////カレントフレームが０フレームと１フレームのときには
	////befeulをカレントフレームのオイラー角として modifyeul180チェックをすることにした
	//int notmodify180flag = 0;

	//2023/02/03_2
	//１フレームでX:-180, Z:-180で後ろを向いているモーションがある
	//そのようなモーションで　180度を取り消してしまうと　体が反対を向く
	//よって　notmodify180flagは1にしてみる
	//notmodify180flagを1にすると
	//LimitEulオン時に　１フレームだけオイラー角が360度違うことがある副作用があるが　対策は後で
	//int notmodify180flag = 1;


	return notmodify180flag;
}


ChaVector3 CBone::CalcLocalEulXYZ(bool limitdegflag, int axiskind, 
	int srcmotid, double srcframe, tag_befeulkind befeulkind, ChaVector3* directbefeul)
{
	//###################################################################################################################
	//2022/12/17
	//この関数の呼び出し元でLimitEul()をする
	//Parentの姿勢に関しては計算済のGetParent()->GetWorldMat()を使用 : curwmに掛かっているのはLimitedではないparentwm
	//モーション全体のオイラー角計算し直しは　この関数ではなく　UpdateMatrixを使用
	//###################################################################################################################

	double roundingframe = (double)((int)(srcframe + 0.0001));


	//for debug
	//if ((roundingframe == 2.0) && (g_limitdegflag == true) && (strstr(GetBoneName(), "Root") != 0)) {
	//	_ASSERT(0);
	//}


	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)

	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);

	const WCHAR* bonename = GetWBoneName();
	//if (wcscmp(bonename, L"RootNode") == 0){
	//	return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
	//}

	if (befeulkind == BEFEUL_BEFFRAME){
		//1つ前のフレームのEULはすでに計算されていると仮定する。
		bool limitdegOnLimitEul = false;//2023/02/07 befeulはunlimited. 何回転もする場合にオーバー１８０度の角度で制限するために.
		befeul = GetBefEul(limitdegOnLimitEul, srcmotid, roundingframe);
	}
	else if ((befeulkind == BEFEUL_DIRECT) && directbefeul){
		befeul = *directbefeul;
	}

	int isfirstbone = 0;
	int isendbone = 0;

	if (GetParent()) {
		isfirstbone = 0;
	}
	else {
		isfirstbone = 1;
	}

	if (GetChild()) {
		if (GetChild()->GetChild()) {
			isendbone = 0;
		}
		else {
			isendbone = 1;
		}
	}
	else {
		isendbone = 1;
	}


	CQuaternion eulq;
	CMotionPoint* curmp = 0;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		if (GetParent()) {
			isfirstbone = 0;

			ChaMatrix curwm, parentwm, eulmat;
			CMotionPoint* parentmp = 0;
			parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
			if (parentmp) {
				curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
				parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, parentmp);
				eulq = ChaMatrix2Q(ChaMatrixInv(parentwm)) * ChaMatrix2Q(curwm);
			}
			else {
				//_ASSERT(0);
				curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
				eulq = ChaMatrix2Q(curwm);
			}
		}
		else {
			isfirstbone = 1;

			ChaMatrix curwm;
			curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
			eulq = ChaMatrix2Q(curwm);
		}
	}
	else {
		//_ASSERT(0);
		eulq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}

	int notmodify180flag = GetNotModify180Flag(srcmotid, roundingframe);

	CQuaternion axisq;
	axisq.RotationMatrix(GetNodeMat());
	//if (GetParent()) {
	//	axisq.RotationMatrix(GetParent()->GetNodeMat());
	//}
	//else {
	//	axisq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	//}

	eulq.Q2EulXYZusingQ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
	//eulq.Q2EulXYZusingMat(ROTORDER_XYZ, &axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);

	return cureul;


	//CMotionPoint* curmp;
	//curmp = GetMotionPoint(srcmotid, roundingframe);
	//if (curmp){
	//	ChaVector3 oldeul = curmp->GetLocalEul();
	//	if (IsSameEul(oldeul, cureul) == 0){
	//		return cureul;
	//	}
	//	else{
	//		return oldeul;
	//	}
	//}
	//else{
	//	return cureul;
	//}
}

//ChaVector3 CBone::CalcLocalLimitedEulXYZ(int srcmotid, double srcframe)
//{
//	//スレッド呼び出しのUpdateMatrixの後処理用
//	// 
//	//制限角度の制限を受ける姿勢のオイラー角を計算
//	//計算済のlimitedwmからオイラー角を計算
//
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);
//	//reteul = GetLimitedLocalEul(srcmotid, srcframe);
//	//reteul = CalcLocalEulAndSetLimitedEul(srcmotid, roundingframe);
//
//	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
//	if (curmp) {
//		ChaMatrix limitedlocal, curlimitedwm;
//		curlimitedwm = curmp->GetLimitedWM();
//		if (GetParent()) {
//			CMotionPoint* parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
//			if (parentmp) {
//				ChaMatrix parentlimitedwm;
//				parentlimitedwm = parentmp->GetLimitedWM();
//				limitedlocal = curlimitedwm * ChaMatrixInv(parentlimitedwm);
//			}
//			else {
//				limitedlocal = curlimitedwm;
//			}
//		}
//		else {
//			limitedlocal = curlimitedwm;
//		}
//
//		CQuaternion eulq;
//		eulq.RotationMatrix(limitedlocal);
//		CQuaternion axisq;
//		axisq.RotationMatrix(GetNodeMat());
//
//		ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//		double befframe;
//		befframe = roundingframe - 1.0;
//		if (befframe >= -0.0001) {
//			CMotionPoint* befmp;
//			befmp = GetMotionPoint(srcmotid, roundingframe);
//			if (befmp) {
//				befeul = befmp->GetLimitedLocalEul();
//			}
//		}
//
//		int isfirstbone = 0;
//		int isendbone = 0;
//		int notmodify180flag = 1;
//		if (g_underIKRot == false) {
//			if (roundingframe <= 1.01) {
//				//0フレームと１フレームは　180度ずれチェックをしない
//				notmodify180flag = 1;
//			}
//			else {
//				notmodify180flag = 0;
//			}
//		}
//		else {
//			//2023/01/26
//			//IKRot中は　０フレームも１フレームも　180度チェックをする
//			notmodify180flag = 0;
//			if (roundingframe <= 1.01) {
//				befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//			}
//		}
//		eulq.Q2EulXYZusingQ(&axisq, befeul, &reteul, isfirstbone, isendbone, notmodify180flag);
//
//		return reteul;
//	}
//	else {
//		return reteul;
//	}
//
//}


//ChaVector3 CBone::CalcLocalUnlimitedEulXYZ(int srcmotid, double srcframe)
//{
//	//制限角度の制限を受けない姿勢のオイラー角を計算
//	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
//	if (curmp) {
//		cureul = curmp->GetLocalEul();//unlimited !!!
//	}
//	else {
//		_ASSERT(0);
//		return cureul;
//	}
//
//
//	//CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
//	//if (curmp) {
//	//	ChaMatrix localmat;
//	//	ChaMatrix curwm, parentwm;
//	//	curwm = curmp->GetWorldMat();//unlimited !!!
//	//	if (GetParent()) {
//	//		CMotionPoint* parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
//	//		if (parentmp) {
//	//			parentwm = parentmp->GetWorldMat();//unlimited !!!
//	//			localmat = curwm * ChaMatrixInv(parentwm);
//	//		}
//	//		else {
//	//			localmat = curwm;
//	//		}
//	//	}
//	//	else {
//	//		localmat = curwm;
//	//	}
//	//	CQuaternion eulq;
//	//	eulq.RotationMatrix(localmat);
//
//	//	CQuaternion axisq;
//	//	axisq.RotationMatrix(GetNodeMat());
//
//	//	//1つ前のフレームのEULはすでに計算されていると仮定する。
//	//	ChaVector3 befeul = GetUnlimitedBefEul(srcmotid, roundingframe);
//	//	int notmodify180flag = GetNotModify180Flag(srcmotid, roundingframe);
//
//	//	int isfirstbone = 0;
//	//	int isendbone = 0;
//	//	eulq.Q2EulXYZusingQ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//	//	//eulq.Q2EulXYZusingMat(ROTORDER_XYZ, &axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//
//	//	return cureul;
//
//	//}
//	//else {
//	//	_ASSERT(0);
//	//	return cureul;
//	//}
//
//
//	//CMotionPoint* curmp;
//	//curmp = GetMotionPoint(srcmotid, roundingframe);
//	//if (curmp) {
//	//	ChaVector3 oldeul = curmp->GetLocalEul();
//	//	if (IsSameEul(oldeul, cureul) == 0) {
//	//		return cureul;
//	//	}
//	//	else {
//	//		return oldeul;
//	//	}
//	//}
//	//else {
//	//	return cureul;
//	//}
//}


ChaMatrix CBone::CalcLocalRotMatFromEul(ChaVector3 srceul, int srcmotid, double srcframe)
{
	CQuaternion noderot;
	//CQuaternion invnoderot;
	noderot.RotationMatrix(GetNodeMat());
	//noderot.inv(&invnoderot);
	//CQuaternion parentnoderot;
	//CQuaternion invparentnoderot;
	//if (GetParent()) {
	//	parentnoderot.RotationMatrix(GetParent()->GetNodeMat());
	//	parentnoderot.inv(&invparentnoderot);
	//}
	//else {
	//	parentnoderot.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	//	invparentnoderot.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	//}


	//const WCHAR* bonename = GetWBoneName();
	//bool ishipsjoint = false;
	//if ((wcscmp(bonename, L"Hips_Joint") == 0) || (wcscmp(bonename, L"hips_Joint") == 0) ||
	//	(wcscmp(bonename, L"Hips") == 0) || (wcscmp(bonename, L"hips") == 0)) {
	//	ishipsjoint = true;
	//}
	//else {
	//	ishipsjoint = false;
	//}

	CQuaternion newrot;
	//newrot0.SetRotationXYZ(0, srceul);//GetNodeMat() * (curwm * ChaMatrixInv(parentwm)) * ChaMatrixInv(GetParent()->GetNodeMat()) の　GLOBAL軸オイラー角
	////########### CalcLocalEulXYZ()におけるeulmatは式１################################################################
	////eulmat = GetNodeMat() * (curwm * ChaMatrixInv(parentwm)) * ChaMatrixInv(GetParent()->GetNodeMat());//式１　//CalcLocalEulXYZ()
	//// 式２で　curwm * invparentwm　の回転に修正する
	////#################################################################################################################
	////newrotmat = invnoderot.MakeRotMatX() * newrot.MakeRotMatX() * parentnoderot.MakeRotMatX();//式２　//curwm * invparentwmの回転
	//CQuaternion newrot;
	//newrot = parentnoderot * newrot0 * invnoderot;


	//(curwm * ChaMatrixInv(parentwm)　の　NodeMat軸のオイラー角 2022/12/20 こっちの方がMayaに近い
	//その後　NodeMatに０フレームアニメを含めないように計算して　Mayaのオイラー角と一致
	newrot.SetRotationXYZ(&noderot, srceul);


	ChaMatrix retlocalrotmat;
	retlocalrotmat = newrot.MakeRotMatX();

	return retlocalrotmat;

}


//ChaVector3 CBone::CalcCurrentLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul)
//{
//	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)
//
//	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//
//	const WCHAR* bonename = GetWBoneName();
//	if (wcscmp(bonename, L"RootNode") == 0) {
//		return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
//	}
//
//	int curmotid = 0;
//	double curframe = 0;
//	if (m_parmodel) {
//		MOTINFO* curmi = m_parmodel->GetCurMotInfo();
//		if (curmi) {
//			curmotid = curmi->motid;
//			curframe = (double)((int)(curmi->curframe + 0.0001));
//		}
//	}
//	else {
//		_ASSERT(0);
//		return cureul;
//	}
//
//
//	if (befeulkind == BEFEUL_BEFFRAME) {
//		//1つ前のフレームのEULはすでに計算されていると仮定する。
//		double befframe;
//		befframe = curframe - 1.0;
//		if (befframe >= -0.0001) {
//			CMotionPoint* befmp;
//			//befmp = GetCurMp();
//			//befeul = befmp.GetLocalEul();
//			befmp = GetMotionPoint(curmotid, befframe);
//			if (befmp) {
//				if (g_limitdegflag == 0) {
//					befeul = befmp->GetLocalEul();
//				}
//				else {
//					befeul = befmp->GetLimitedLocalEul();
//				}
//			}
//		}
//	}
//	else if ((befeulkind == BEFEUL_DIRECT) && directbefeul) {
//		befeul = *directbefeul;
//	}
//
//	int isfirstbone = 0;
//	int isendbone = 0;
//
//	if (GetParent()) {
//		isfirstbone = 0;
//	}
//	else {
//		isfirstbone = 1;
//	}
//
//	if (GetChild()) {
//		if (GetChild()->GetChild()) {
//			isendbone = 0;
//		}
//		else {
//			isendbone = 1;
//		}
//	}
//	else {
//		isendbone = 1;
//	}
//
//	//2023/01/14
//	//rootjointを２回転する場合など　180度補正は必要(１フレームにつき165度までの変化しか出来ない制限は必要)
//	//しかし　bvh2fbxなど　１フレームにアニメが付いているデータでうまくいくようにするために　0フレームと１フレームは除外
//	int notmodify180flag = 1;
//	if (g_underIKRot == false) {
//		if (curframe <= 1.01) {
//			//0フレームと１フレームは　180度ずれチェックをしない
//			notmodify180flag = 1;
//		}
//		else {
//			notmodify180flag = 0;
//		}
//	}
//	else {
//		//2023/01/26
//		//IKRot中は　０フレームも１フレームも　180度チェックをする
//		notmodify180flag = 0;
//		if (curframe <= 1.01) {
//			befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//		}
//	}
//
//	CQuaternion eulq;
//	if (GetParent()) {
//		isfirstbone = 0;
//
//		ChaMatrix curwm, parentwm, eulmat;
//		curwm = GetCurMp().GetWorldMat();
//		parentwm = GetParent()->GetCurMp().GetWorldMat();
//		//eulq = ChaMatrix2Q(ChaMatrixInv(GetParent()->GetNodeMat())) * ChaMatrix2Q(ChaMatrixInv(parentwm)) * ChaMatrix2Q(curwm) * ChaMatrix2Q(GetNodeMat());//2022/12/14 mesh付きのfbxでOK
//		eulq = ChaMatrix2Q(ChaMatrixInv(parentwm)) * ChaMatrix2Q(curwm);
//	}
//	else {
//		isfirstbone = 1;
//
//		ChaMatrix curwm, eulmat;
//		curwm = GetCurMp().GetWorldMat();
//		//eulq = ChaMatrix2Q(curwm) * ChaMatrix2Q(GetNodeMat());
//		eulq = ChaMatrix2Q(curwm);
//	}
//	//eulq.Q2EulXYZusingMat(ROTORDER_XYZ, 0, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//	//eulq.Q2EulXYZusingQ(0, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//
//	CQuaternion axisq;
//	axisq.RotationMatrix(GetNodeMat());
//	eulq.Q2EulXYZusingQ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//	//eulq.Q2EulXYZusingMat(ROTORDER_XYZ, &axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//
//
//	CMotionPoint* curmp;
//	curmp = GetMotionPoint(curmotid, curframe);
//	if (curmp) {
//		ChaVector3 oldeul = curmp->GetLocalEul();
//		if (IsSameEul(oldeul, cureul) == 0) {
//			return cureul;
//		}
//		else {
//			return oldeul;
//		}
//	}
//	else {
//		return cureul;
//	}
//
//
//	return cureul;
//}

//ChaMatrix CBone::CalcCurrentLocalRotMatFromEul(ChaVector3 srceul)
//{
//	CQuaternion noderot;
//	CQuaternion invnoderot;
//	noderot.RotationMatrix(GetNodeMat());
//	noderot.inv(&invnoderot);
//	CQuaternion parentnoderot;
//	CQuaternion invparentnoderot;
//	if (GetParent()) {
//		parentnoderot.RotationMatrix(GetParent()->GetNodeMat());
//		parentnoderot.inv(&invparentnoderot);
//	}
//	else {
//		parentnoderot.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//		invparentnoderot.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
//	}
//
//
//	const WCHAR* bonename = GetWBoneName();
//	bool ishipsjoint = false;
//	if ((wcscmp(bonename, L"Hips_Joint") == 0) || (wcscmp(bonename, L"hips_Joint") == 0) ||
//		(wcscmp(bonename, L"Hips") == 0) || (wcscmp(bonename, L"hips") == 0)) {
//		ishipsjoint = true;
//	}
//	else {
//		ishipsjoint = false;
//	}
//
//
//	//CQuaternion newrot0;
//	//newrot0.SetRotationXYZ(0, srceul);//GetNodeMat() * (curwm * ChaMatrixInv(parentwm)) * ChaMatrixInv(GetParent()->GetNodeMat()) の　GLOBAL軸オイラー角
//	////########### CalcLocalEulXYZ()におけるeulmatは式１################################################################
//	////eulmat = GetNodeMat() * (curwm * ChaMatrixInv(parentwm)) * ChaMatrixInv(GetParent()->GetNodeMat());//式１　//CalcLocalEulXYZ()
//	//// 式２で　curwm * invparentwm　の回転に修正する
//	////#################################################################################################################
//	////newrotmat = invnoderot.MakeRotMatX() * newrot.MakeRotMatX() * parentnoderot.MakeRotMatX();//式２　//curwm * invparentwmの回転
//	//CQuaternion newrot;
//	//newrot = parentnoderot * newrot0 * invnoderot;
//
//
//	//2022/12/26 CalcLocalRotMatFromEulと合わせる
//	CQuaternion newrot;
//	newrot.SetRotationXYZ(&noderot, srceul);
//
//
//	ChaMatrix retlocalrotmat;
//	retlocalrotmat = newrot.MakeRotMatX();
//
//	return retlocalrotmat;
//
//}


//ChaVector3 CBone::CalcBtLocalEulXYZ(int axiskind, tag_befeulkind befeulkind, ChaVector3* directbefeul)
//{
//	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)
//
//	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//
//	const WCHAR* bonename = GetWBoneName();
//	if (wcscmp(bonename, L"RootNode") == 0) {
//		return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
//	}
//
//	//if (befeulkind == BEFEUL_BEFFRAME) {
//	//	//1つ前のフレームのEULはすでに計算されていると仮定する。
//	//	double befframe;
//	//	befframe = srcframe - 1.0;
//	//	if (befframe >= -0.0001) {
//	//		CMotionPoint* befmp;
//	//		//befmp = GetCurMp();
//	//		//befeul = befmp.GetLocalEul();
//	//		befmp = GetMotionPoint(srcmotid, befframe);
//	//		if (befmp) {
//	//			befeul = befmp->GetLocalEul();
//	//		}
//	//	}
//	//}
//	//else if ((befeulkind == BEFEUL_DIRECT) && directbefeul) {
//	//	befeul = *directbefeul;
//	//}
//
//	CMotionPoint tmpmp;
//	//CalcLocalInfo(srcmotid, srcframe, &tmpmp);//local!!!
//	CalcBtLocalInfo(&tmpmp);//local!!!
//
//	ChaMatrix axismat;
//	CQuaternion axisq;
//	//int multworld = 0;//local!!!
//	//axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
//	//axisq.RotationMatrix(axismat);
//
//	int isfirstbone = 0;
//	int isendbone = 0;
//
//	if (GetParent()) {
//		CRigidElem* curre = GetParent()->GetRigidElem(this);
//		if (curre) {
//			axismat = curre->GetBindcapsulemat();
//		}
//		else {
//			//_ASSERT(0);
//			ChaMatrixIdentity(&axismat);
//		}
//		//axismat = GetParent()->GetCurMp().GetWorldMat();
//
//		axisq.RotationMatrix(axismat);
//
//		isfirstbone = 0;
//	}
//	else {
//		ChaMatrixIdentity(&axismat);
//		axisq.SetParams(1.0, 0.0, 0.0, 0.0);
//
//		isfirstbone = 1;
//	}
//
//	if (GetChild()) {
//		if (GetChild()->GetChild()) {
//			isendbone = 0;
//		}
//		else {
//			isendbone = 1;
//		}
//	}
//	else {
//		isendbone = 1;
//	}
//
//	//int notmodifyflag;
//	//if ((srcframe == 0.0) || (srcframe == 1.0)) {
//	//	notmodifyflag = 1;
//	//}
//	//else {
//	//	notmodifyflag = 0;
//	//}
//
//
//	int notmodify180flag = 1;//!!!! 165度以上のIK編集のために　180度チェックはしない
//
//
//	if (axiskind == -1) {
//		if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL) {
//			tmpmp.GetQ().Q2EulXYZ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//		}
//		else {
//			tmpmp.GetQ().Q2EulXYZ(0, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//		}
//	}
//	else if (axiskind != BONEAXIS_GLOBAL) {
//		tmpmp.GetQ().Q2EulXYZ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//	}
//	else {
//		tmpmp.GetQ().Q2EulXYZ(0, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);
//	}
//
//	//CMotionPoint* curmp;
//	//curmp = GetMotionPoint(srcmotid, srcframe);
//	//if (curmp) {
//	//	ChaVector3 oldeul = curmp->GetLocalEul();
//	//	if (IsSameEul(oldeul, cureul) == 0) {
//	//		return cureul;
//	//	}
//	//	else {
//	//		return oldeul;
//	//	}
//	//}
//	//else {
//	//	return cureul;
//	//}
//
//	return cureul;
//}



int CBone::SetWorldMatFromEul(bool limitdegflag, int inittraflag, int setchildflag, ChaMatrix befwm, ChaVector3 srceul, int srcmotid, double srcframe, int initscaleflag)//initscaleflag = 1 : default
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。


	//2022/11/23 CommentOut なぜこのif文があったのか？ 不具合によりエンドジョイントにモーションポイントが無かったから？
	//if (!m_child){
	//	return 0;
	//}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		//_ASSERT(0);
		return 1;
	}

	//ChaMatrix befwm = curmp->GetBefWorldMat();

	ChaMatrix newworldmat = CalcWorldMatFromEul(limitdegflag, inittraflag, setchildflag, srceul, srcmotid, roundingframe, initscaleflag);


	//CMotionPoint* curmp;
	//curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		SetWorldMat(limitdegflag, srcmotid, roundingframe, newworldmat, curmp);
		SetLocalEul(limitdegflag, srcmotid, roundingframe, srceul, curmp);
		if (limitdegflag == true) {
			curmp->SetCalcLimitedWM(2);
		}

		if (setchildflag == 1){
			if (GetChild()){
				bool setbroflag = true;
				GetChild()->UpdateParentWMReq(limitdegflag, setbroflag,
					srcmotid, roundingframe, befwm, newworldmat);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}


//int CBone::SetBtWorldMatFromEul(int setchildflag, ChaVector3 srceul)//initscaleflag = 1 : default
//{
//	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
//	if (!m_child) {
//		return 0;
//	}
//
//	ChaMatrix axismat;
//	CQuaternion axisq;
//	//int multworld = 0;//local!!!
//	//axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
//	//axisq.RotationMatrix(axismat);
//	if (GetParent()) {
//		CRigidElem* curre = GetParent()->GetRigidElem(this);
//		if (curre) {
//			axismat = curre->GetBindcapsulemat();
//		}
//		else {
//			//_ASSERT(0);
//			ChaMatrixIdentity(&axismat);
//		}
//		axisq.RotationMatrix(axismat);
//	}
//	else {
//		ChaMatrixIdentity(&axismat);
//		axisq.SetParams(1.0, 0.0, 0.0, 0.0);
//	}
//
//
//	CQuaternion newrot;
//	if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL) {
//		newrot.SetRotationXYZ(&axisq, srceul);
//	}
//	else {
//		newrot.SetRotationXYZ(0, srceul);
//	}
//
//
//	//QuaternionInOrder(srcmotid, srcframe, &newrot);
//
//
//	ChaMatrix curlocalmat;
//	if (GetParent()) {
//		curlocalmat = GetBtMat() * ChaMatrixInv(GetParent()->GetBtMat());
//	}
//	else {
//		curlocalmat = GetBtMat();
//	}
//	
//	ChaVector3 curtrapos;
//	ChaVector3 tmpfpos = GetJointFPos();
//	ChaVector3TransformCoord(&curtrapos, &tmpfpos, &curlocalmat);
//
//	ChaMatrix cursmat, currmat, curtmat;
//	GetSRTMatrix2(curlocalmat, &cursmat, &currmat, &curtmat);
//
//
//	ChaMatrix newlocalmat, newrotmat, befrotmat, aftrotmat;
//	newrotmat = newrot.MakeRotMatX();
//	ChaMatrixTranslation(&befrotmat, -tmpfpos.x, -tmpfpos.y, -tmpfpos.z);
//	ChaMatrixTranslation(&aftrotmat, tmpfpos.x, tmpfpos.y, tmpfpos.z);
//	//newlocalmat = befrotmat * newrotmat * aftrotmat;
//
//	//if (initscaleflag == 0) {
//	//	newlocalmat = befrotmat * cursmat * newrotmat * aftrotmat;
//	//}
//	//else {
//		newlocalmat = befrotmat * newrotmat * aftrotmat;
//	//}
//	//if (inittraflag == 0) {
//	//	//ChaVector3 tmppos;
//	//	//ChaVector3TransformCoord(&tmppos, &(GetJointFPos()), &newlocalmat);
//	//	//ChaVector3 diffvec;
//	//	//diffvec = curtrapos - tmppos;
//	//	//ChaMatrix tmptramat;
//	//	//ChaMatrixIdentity(&tmptramat);
//	//	//ChaMatrixTranslation(&tmptramat, diffvec.x, diffvec.y, diffvec.z);
//
//	//	ChaMatrix tmptramat;
//	//	ChaMatrixIdentity(&tmptramat);
//	//	ChaMatrixTranslation(&tmptramat, orgtraanim.x, orgtraanim.y, orgtraanim.z);
//
//	//	newlocalmat = newlocalmat * tmptramat;
//	//}
//
//	//if (inittraflag == 0){
//	//	ChaVector3 traanim = CalcLocalTraAnim(srcmotid, srcframe);
//	//	ChaMatrix tramat;
//	//	ChaMatrixIdentity(&tramat);
//	//	ChaMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);
//	//	newlocalmat = newlocalmat * tramat;
//	//}
//
//	ChaMatrix newmat;
//	if (m_parent) {
//		newmat = newlocalmat * GetParent()->GetBtMat();
//	}
//	else {
//		newmat = newlocalmat;
//	}
//
//	////curmp->SetBefWorldMat(curmp->GetWorldMat());
//	//curmp->SetWorldMat(newmat);
//	//curmp->SetLocalEul(srceul);
//
//	//if (setchildflag == 1) {
//	//	if (m_child) {
//	//		bool infooutflag = false;
//	//		CQuaternion dummyq;
//	//		m_child->RotBoneQReq(infooutflag, this, srcmotid, srcframe, dummyq);
//	//	}
//	//}
//
//	return 0;
//}

//int CBone::CalcWorldMatAfterThread(int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat)
//{
//	//############################################################
//	//g_limitdegflag == trueのときに並列化計算後の処理として呼ばれる
//	//############################################################
//
//	//ChaVector3 orgeul, neweul;
//	//GetTempLocalEul(&orgeul, &neweul);
//	//ChaMatrix newworldmat = CalcWorldMatFromEul(0, 1, neweul, orgeul, srcmotid, (double)((int)(srcframe + 0.1)), 0);
//
//	ChaMatrix limworldmat;
//	ChaMatrixIdentity(&limworldmat);
//
//	int callingstate = 2;//!!!!!!!!!!!!!!!!!!!
//
//	CMotionPoint* pbefmp = 0;
//
//	GetCalclatedLimitedWM(srcmotid, srcframe, &limworldmat, &pbefmp, callingstate);//srcframeのbefaftによる補間もしている
//
//	ChaMatrix tmpmat = limworldmat * *wmat;
//	m_curmp.SetWorldMat(tmpmat);//2021/12/21
//
//	//CMotionPoint* pmp = GetMotionPoint(srcmotid, (double)((int)(srcframe + 0.1)));
//	//if (pmp) {
//	//	pmp->SetLimitedWM(newworldmat);
//	//	pmp->SetCalcLimitedWM(1);
//	//}
// 
//	ChaVector3 jpos = GetJointFPos();
//	ChaMatrix tmpwm = m_curmp.GetWorldMat();
//	ChaVector3TransformCoord(&m_childworld, &jpos, &tmpwm);
//	//ChaVector3TransformCoord(&m_childworld, &jpos, &newworldmat);
//
//	ChaMatrix wvpmat = m_curmp.GetWorldMat() * *vpmat;
//	//ChaMatrix wvpmat = newworldmat * *vpmat;
//
//	ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);
//	//ChaVector3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
//
//	return 0;
//}


ChaMatrix CBone::CalcWorldMatFromEul(bool limitdegflag, int inittraflag, int setchildflag, ChaVector3 srceul, int srcmotid, double srcframe, int initscaleflag)//initscaleflag = 1 : default
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	//if (!m_child) {
	//	return retmat;
	//}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		//_ASSERT(0);
		return retmat;
	}


	ChaMatrix curwm;
	curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
	//if (g_limitdegflag == false) {
	//	curwm = curmp->GetWorldMat();
	//}
	//else {
	//	curwm = curmp->GetLimitedWM();
	//}

	ChaMatrix oldlocalmat;
	oldlocalmat.SetIdentity();
	if (GetParent()) {
		ChaMatrix parmat;
		parmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		oldlocalmat = curwm * ChaMatrixInv(parmat);
	}
	else {
		oldlocalmat = curwm;
	}
	ChaMatrix cursmat, currmat, curtmat, curtanimmat;
	GetSRTandTraAnim(oldlocalmat, GetNodeMat(), &cursmat, &currmat, &curtmat, &curtanimmat);


	ChaMatrix newlocalrotmat = CalcLocalRotMatFromEul(srceul, srcmotid, roundingframe);
	ChaMatrix newlocalmat;
	bool sflag = (initscaleflag == 0);
	bool tanimflag = (inittraflag == 0);
	newlocalmat = ChaMatrixFromSRTraAnim(sflag, tanimflag, GetNodeMat(), &cursmat, &newlocalrotmat, &curtanimmat);


	ChaMatrix newmat;
	if (GetParent()) {
		ChaMatrix limitedparmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newmat = newlocalmat * limitedparmat;
	}
	else {
		newmat = newlocalmat;
	}

	retmat = newmat;

	return retmat;
}



int CBone::SetWorldMatFromEulAndScaleAndTra(bool limitdegflag, int inittraflag, int setchildflag, 
	ChaMatrix befwm, ChaVector3 srceul, ChaVector3 srcscale, ChaVector3 srctra, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	//if (!m_child) {
	//	return 0;
	//}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix newscalemat;
	newscalemat.SetIdentity();//2023/02/12 抜け落ちてた
	ChaMatrixScaling(&newscalemat, srcscale.x, srcscale.y, srcscale.z);//!!!!!!!!!!!!  new scale
	ChaMatrix newtramat;
	ChaMatrixIdentity(&newtramat);
	ChaMatrixTranslation(&newtramat, srctra.x, srctra.y, srctra.z);//TraAnimをそのまま

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 1;
	}


	//ChaMatrix curwm;
	//curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
	//ChaMatrix curlocalmat;
	//if (GetParent()) {
	//	ChaMatrix parmat;
	//	parmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	//	curlocalmat = curwm * ChaMatrixInv(parmat);
	//}
	//else {
	//	curlocalmat = curwm;
	//}
	//ChaMatrix cursmat, currmat, curtmat, curtanimmat;
	//GetSRTandTraAnim(curlocalmat, GetNodeMat(), &cursmat, &currmat, &curtmat, &curtanimmat);


	ChaMatrix newlocalrotmat = CalcLocalRotMatFromEul(srceul, srcmotid, roundingframe);
	ChaMatrix newlocalmat;
	bool sflag = true;
	bool tanimflag = (inittraflag == 0);
	newlocalmat = ChaMatrixFromSRTraAnim(sflag, tanimflag, GetNodeMat(), &newscalemat, &newlocalrotmat, &newtramat);

	ChaMatrix newmat;
	if (GetParent()) {
		ChaMatrix limitedparmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newmat = newlocalmat * limitedparmat;
	}
	else {
		newmat = newlocalmat;
	}

	if (curmp) {
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		SetWorldMat(limitdegflag, srcmotid, roundingframe, newmat, curmp);
		SetLocalEul(limitdegflag, srcmotid, roundingframe, srceul, curmp);
		if (limitdegflag == true) {
			curmp->SetCalcLimitedWM(2);
		}

		if (setchildflag == 1) {
			if (GetChild()) {
				bool setbroflag = true;
				GetChild()->UpdateParentWMReq(limitdegflag, setbroflag,
					srcmotid, roundingframe, befwm, newmat);
			}
		}
	}
	else {
		_ASSERT(0);
	}

	return 0;
}




int CBone::SetWorldMatFromQAndTra(bool limitdegflag, int setchildflag, 
	ChaMatrix befwm, CQuaternion axisq, CQuaternion srcq, ChaVector3 srctra, int srcmotid, double srcframe)
{
	if (!m_child){
		return 0;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CQuaternion invaxisq;
	axisq.inv(&invaxisq);
	CQuaternion newrot = invaxisq * srcq * axisq;
	ChaMatrix newlocalrotmat;
	newlocalrotmat = newrot.MakeRotMatX();
	ChaMatrix newtramat;
	ChaMatrixIdentity(&newtramat);
	ChaMatrixTranslation(&newtramat, srctra.x, srctra.y, srctra.z);

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 1;
	}

	//ChaMatrix befwm = curmp->GetBefWorldMat();
	ChaMatrix curwm;
	curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);

	ChaMatrix parmat;
	ChaMatrix parnodemat;
	ChaMatrixIdentity(&parmat);
	ChaMatrixIdentity(&parnodemat);
	if (GetParent()) {
		parmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parnodemat = GetParent()->GetNodeMat();
	}

	ChaMatrix cursmat, currmat, curtmat, curtanimmat;
	GetSRTandTraAnim((curwm * ChaMatrixInv(parmat)), GetNodeMat(), &cursmat, &currmat, &curtmat, &curtanimmat);

	ChaMatrix newlocalmat;
	bool sflag = false;
	bool tanimflag = true;
	newlocalmat = ChaMatrixFromSRTraAnim(sflag, tanimflag, GetNodeMat(), 0, &newlocalrotmat, &newtramat);

	ChaMatrix newmat;
	if (GetParent()) {
		ChaMatrix limitedparmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newmat = newlocalmat * limitedparmat;
	}
	else {
		newmat = newlocalmat;
	}


	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		SetWorldMat(limitdegflag, srcmotid, roundingframe, newmat, curmp);
		//ChaVector3 neweul = CalcLocalEulXYZ(-1, srcmotid, roundingframe, BEFEUL_ZERO);
		ChaVector3 neweul = CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
		SetLocalEul(limitdegflag, srcmotid, roundingframe, neweul, curmp);
		if (limitdegflag == true) {
			curmp->SetCalcLimitedWM(2);
		}


		if (setchildflag == 1) {
			if (GetChild()) {
				bool setbroflag = true;
				GetChild()->UpdateParentWMReq(limitdegflag, setbroflag,
					srcmotid, roundingframe, befwm, newmat);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}


int CBone::SetWorldMatFromEulAndTra(bool limitdegflag, int setchildflag, 
	ChaMatrix befwm, ChaVector3 srceul, ChaVector3 srctra, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	


	//2022/11/23 CommentOut なぜこのif文があったのか？ 不具合によりエンドジョイントにモーションポイントが無かったから？
	//if (!m_child){
	//	return 0;
	//}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix newtramat;
	ChaMatrixIdentity(&newtramat);
	ChaMatrixTranslation(&newtramat, srctra.x, srctra.y, srctra.z);//TraAnimをそのまま

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 1;
	}

	//ChaMatrix befwm = curmp->GetBefWorldMat();
	ChaMatrix curwm;
	curwm = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);


	ChaMatrix parmat;
	ChaMatrix parnodemat;
	ChaMatrixIdentity(&parmat);
	ChaMatrixIdentity(&parnodemat);
	if (GetParent()) {
		parmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parnodemat = GetParent()->GetNodeMat();
	}

	ChaMatrix newlocalrotmat = CalcLocalRotMatFromEul(srceul, srcmotid, roundingframe);

	ChaMatrix cursmat, currmat, curtmat, curtanimmat;
	GetSRTandTraAnim((curwm * ChaMatrixInv(parmat)), GetNodeMat(), &cursmat, &currmat, &curtmat, &curtanimmat);

	ChaMatrix newlocalmat;
	bool sflag = false;
	bool tanimflag = true;
	newlocalmat = ChaMatrixFromSRTraAnim(sflag, tanimflag, GetNodeMat(), 0, &newlocalrotmat, &newtramat);


	ChaMatrix newmat;
	if (GetParent()) {
		ChaMatrix limitedparmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newmat = newlocalmat * limitedparmat;
	}
	else {
		newmat = newlocalmat;
	}

	//CMotionPoint* curmp;
	//curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		SetWorldMat(limitdegflag, srcmotid, roundingframe, newmat, curmp);
		SetLocalEul(limitdegflag, srcmotid, roundingframe, srceul, curmp);
		if (limitdegflag == true) {
			curmp->SetCalcLimitedWM(2);
		}


		if (setchildflag == 1) {
			if (GetChild()) {
				bool setbroflag = true;
				GetChild()->UpdateParentWMReq(limitdegflag, setbroflag,
					srcmotid, roundingframe, befwm, newmat);
			}
		}
	}
	else {
		_ASSERT(0);
	}

	return 0;
}


int CBone::SetLocalEul(bool limitdegflag, int srcmotid, double srcframe, ChaVector3 srceul, CMotionPoint* srcmp)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	if (srcmp) {
		if (limitdegflag == false) {
			srcmp->SetLocalEul(srceul);
		}
		else {
			srcmp->SetLimitedLocalEul(srceul);
		}
	}
	else {
		CMotionPoint* curmp;
		curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			if (limitdegflag == false) {
				curmp->SetLocalEul(srceul);
			}
			else {
				curmp->SetLimitedLocalEul(srceul);
			}
		}
		else {
			//_ASSERT(0);
			return 1;
		}
	}

	return 0;
}

ChaVector3 CBone::GetLocalEul(bool limitdegflag, int srcmotid, double srcframe, CMotionPoint* srcmp)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);
	if (srcmp) {
		if (limitdegflag == false) {
			reteul = srcmp->GetLocalEul();
		}
		else {
			//2023/01/31
			//g_limitdegflag == trueの場合
			//GetWorldMat-->GetLimitedWorldMat-->CalcLocalEulAndSetLImitedEul
			//-->GetLocalEul(-->GetLimitedWorldMat)
			//上記のように呼ばれるので　この関数GetLocalEul内でGetLimitedWorldMatを呼び出すことは出来ない

			//if (srcmp->GetCalcLimitedWM() == 2) {
			//	reteul = srcmp->GetLimitedLocalEul();
			//}
			//else {
			//	GetLimitedWorldMat(srcmotid, roundingframe, &reteul);
			//}

			reteul = srcmp->GetLimitedLocalEul();

		}
	}
	else {
		CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			if (limitdegflag == false) {
				reteul = curmp->GetLocalEul();
			}
			else {
				//2023/01/31
				//g_limitdegflag == trueの場合
				//GetWorldMat-->GetLimitedWorldMat-->CalcLocalEulAndSetLImitedEul
				//-->GetLocalEul(-->GetLimitedWorldMat)
				//上記のように呼ばれるので　この関数GetLocalEul内でGetLimitedWorldMatを呼び出すことは出来ない

				//if (curmp->GetCalcLimitedWM() == 2) {
				//	reteul = curmp->GetLimitedLocalEul();
				//}
				//else {
				//	GetLimitedWorldMat(srcmotid, roundingframe, &reteul);
				//}

				reteul = curmp->GetLimitedLocalEul();
			}
		}
		else {
			//_ASSERT(0);
			reteul = ChaVector3(0.0f, 0.0f, 0.0f);
		}
	}

	return reteul;
}

//onlycheck = 0
int CBone::SetWorldMat(bool limitdegflag, bool directsetflag, 
	bool infooutflag, int setchildflag, 
	int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck, bool fromiktarget)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	//if pose is change, return 1 else return 0
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (!curmp){
		return 0;
	}

	curmp->SetCalcLimitedWM(0);//2023/01/14 limited　フラグ　リセット


	int ismovable = 0;
	
	
	//変更前を保存
	ChaMatrix saveworldmat;
	ChaVector3 saveeul;
	saveworldmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
	saveeul = GetLocalEul(limitdegflag, srcmotid, roundingframe, curmp);


	//if ((directsetflag == false) && (g_underRetargetFlag == false)){

	if (directsetflag == false) {
		ChaMatrix beflocalmat;
		ChaMatrix newlocalmat;
		beflocalmat.SetIdentity();
		newlocalmat.SetIdentity();
		if (GetParent()) {
			ChaMatrix befparentwm;
			befparentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
			beflocalmat = saveworldmat * ChaMatrixInv(befparentwm);
			newlocalmat = srcmat * ChaMatrixInv(befparentwm);
		}
		else {
			beflocalmat = saveworldmat;
			newlocalmat = srcmat;
		}

		//calc bef SRT
		ChaMatrix befrmat;
		befrmat.SetIdentity();
		ChaVector3 befscalevec = ChaVector3(1.0f, 1.0f, 1.0f);
		ChaVector3 beftvec = ChaVector3(0.0f, 0.0f, 0.0f);
		GetSRTMatrix(beflocalmat, &befscalevec, &befrmat, &beftvec);

		//calc new SRT
		ChaMatrix newsmat, newrmat, newtmat, newtanimmat;
		newsmat.SetIdentity();
		newrmat.SetIdentity();
		newtmat.SetIdentity();
		newtanimmat.SetIdentity();
		GetSRTandTraAnim(newlocalmat, GetNodeMat(), &newsmat, &newrmat, &newtmat, &newtanimmat);

		//calc new eul
		SetWorldMat(limitdegflag, srcmotid, roundingframe, srcmat, curmp);//tmp time
		ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
		neweul = CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
		
		
		//if (g_limitdegflag == true) {
		if((limitdegflag == true) && (GetBtForce() == 0)){//2023/01/28 物理シミュは　自前では制限しない
			ismovable = ChkMovableEul(neweul);
		}
		else {
			ismovable = 1;
		}
		
		if (infooutflag == true) {
			OutputToInfoWnd(L"CBone::SetWorldMat : %s : frame %5.1lf : neweul [%f, %f, %f] : ismovable %d", GetWBoneName(), roundingframe, neweul.x, neweul.y, neweul.z, ismovable);
		}

		if (onlycheck == 0) {
			if (ismovable == 1) {
				int inittraflag0 = 0;
				//子ジョイントへの波及は　SetWorldMatFromEulAndScaleAndTra内でしている
				SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
					saveworldmat, neweul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag有り!!!!
					//curmp->SetBefWorldMat(saveworldmat);
				SetLocalEul(limitdegflag, srcmotid, roundingframe, neweul, curmp);
				if (limitdegflag == true) {
					curmp->SetCalcLimitedWM(2);
				}
			}
			else {
				if ((g_wallscrapingikflag == 1) || (fromiktarget == true)) {//PosConstraint用の回転時には　自動で壁すり処理をする
					//############################################
					//　遊び付きリミテッドIK
					//############################################
					ChaVector3 limiteul;
					bool limitdegOnLimitEul1 = false;//2023/02/07 befeulはunlimited. 何回転もする場合にオーバー１８０度の角度で制限するために.
					limiteul = LimitEul(neweul, GetBefEul(limitdegOnLimitEul1, srcmotid, roundingframe));
					int inittraflag0 = 0;
					//子ジョイントへの波及は　SetWorldMatFromEulAndScaleAndTra内でしている
					SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
						saveworldmat, limiteul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag有り!!!!
					SetLocalEul(limitdegflag, srcmotid, roundingframe, limiteul, curmp);
					if (limitdegflag == true) {
						curmp->SetCalcLimitedWM(2);
					}
				}
				else {
					if (g_underIKRot == true) {
						SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
						SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
						if (limitdegflag == true) {
							curmp->SetCalcLimitedWM(2);
						}
					}
					else {
						ChaVector3 limiteul;
						bool limitdegOnLimitEul2 = false;//2023/02/07 befeulはunlimited. 何回転もする場合にオーバー１８０度の角度で制限するために.
						limiteul = LimitEul(neweul, GetBefEul(limitdegOnLimitEul2, srcmotid, roundingframe));
						int inittraflag0 = 0;
						//子ジョイントへの波及は　SetWorldMatFromEulAndScaleAndTra内でしている
						SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
							saveworldmat, limiteul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag有り!!!!
						SetLocalEul(limitdegflag, srcmotid, roundingframe, limiteul, curmp);
						if (limitdegflag == true) {
							curmp->SetCalcLimitedWM(2);
						}
					}
				}
			}
		}
		else {
			//only check : 仮セットしていたのを元に戻す
			SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
			SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
		}
	}
	else{

		//directsetflag == true

		ismovable = 1;
		if (onlycheck == 0) {
			SetWorldMat(limitdegflag, srcmotid, roundingframe, srcmat, curmp);

			ChaVector3 neweul = CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
			SetLocalEul(limitdegflag, srcmotid, roundingframe, neweul, curmp);
			if (limitdegflag == true) {
				curmp->SetCalcLimitedWM(2);
			}
		
			if (setchildflag == 1) {
				if (GetChild()) {
					bool setbroflag = true;
					GetChild()->UpdateParentWMReq(limitdegflag, setbroflag, srcmotid, roundingframe,
						saveworldmat, srcmat);
				}
			}
		}
		else {
			//only check : 仮セットしていたのを元に戻す
			SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
			SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
		}
	}


	return ismovable;
}

int CBone::ChkMovableEul(ChaVector3 srceul)
{

	//2023/01/10 : 事前計算しておきたい場合などがあるので　以下３行の条件はこの関数の呼び出し元で行う
	//if (g_limitdegflag == false){
	//	return 1;//movable
	//}

	SetAngleLimitOff(&m_anglelimit);


	int dontmove = 0;
	int axiskind;

	float chkval[3];
	chkval[0] = srceul.x;
	chkval[1] = srceul.y;
	chkval[2] = srceul.z;

	for (axiskind = AXIS_X; axiskind <= AXIS_Z; axiskind++){
		if (m_anglelimit.limitoff[axiskind] == 0) {
			//if (m_anglelimit.via180flag[axiskind] == 0) {
				if ((m_anglelimit.lower[axiskind] > (int)chkval[axiskind]) || (m_anglelimit.upper[axiskind] < (int)chkval[axiskind])) {
					dontmove++;
				}
			//}
			//else {
			//	//180度線(-180度線)を越えるように動く場合
			//	if ((m_anglelimit.lower[axiskind] <= (int)chkval[axiskind]) && (m_anglelimit.upper[axiskind] >= (int)chkval[axiskind])) {
			//		dontmove++;
			//	}
			//}
		}
	}

	if (dontmove != 0){
		return 0;
	}
	else{
		return 1;//movable
	}
}


float CBone::LimitAngle(enum tag_axiskind srckind, float srcval)
{

	SetAngleLimitOff(&m_anglelimit);
	if (m_anglelimit.limitoff[srckind] == 1){
		return srcval;
	}
	else{
		float newval = srcval;

		float cmpvalupper, cmpvallower;
		cmpvalupper = srcval;
		cmpvallower = srcval;

		if (abs(m_anglelimit.upper[srckind] - m_anglelimit.lower[srckind]) > EULLIMITPLAY) {
			//リミット付近でもIKが動くためには遊びの部分が必要

			if (cmpvalupper > m_anglelimit.upper[srckind]) {
				newval = min(cmpvalupper, (float)(m_anglelimit.upper[srckind] - EULLIMITPLAY));
			}
			if (cmpvallower < m_anglelimit.lower[srckind]) {
				newval = max(cmpvallower, (float)(m_anglelimit.lower[srckind] + EULLIMITPLAY));
			}
		}
		else {
			//lowerとupperの間がEULLIMITPLAYより小さいとき
			
			if (cmpvalupper > m_anglelimit.upper[srckind]) {
				newval = (float)(m_anglelimit.upper[srckind]);
			}
			if (cmpvallower < m_anglelimit.lower[srckind]) {
				newval = (float)(m_anglelimit.lower[srckind]);
			}
		}

		return newval;
	}
}


ChaVector3 CBone::LimitEul(ChaVector3 srceul, ChaVector3 srcbefeul)
{
	const float thdeg = 165.0f;

	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 tmpeul = ChaVector3(0.0f, 0.0f, 0.0f);

	tmpeul.x = LimitAngle(AXIS_X, srceul.x);
	tmpeul.y = LimitAngle(AXIS_Y, srceul.y);
	tmpeul.z = LimitAngle(AXIS_Z, srceul.z);


	//何回転もする場合に　純粋に角度で制限するためにコメントアウト
	//CQuaternion calcq;
	//calcq.ModifyEuler360(&tmpeul, &srcbefeul, 0);

	reteul = tmpeul;


	return reteul;
}

ANGLELIMIT CBone::GetAngleLimit(bool limitdegflag, int getchkflag)
{
	::SetAngleLimitOff(&m_anglelimit);

	if (getchkflag == 1) {

		//int existflag = 0;
		//CalcFBXMotion(curmotid, curframe, &m_curmp, &existflag);

		//ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
		//ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
		////cureul = CalcLocalEulXYZ(m_anglelimit.boneaxiskind, curmotid, curframe, BEFEUL_BEFFRAME);
		//cureul = CalcCurrentLocalEulXYZ(m_anglelimit.boneaxiskind, curmotid, curframe, BEFEUL_BEFFRAME);

		if (m_parmodel) {
			MOTINFO* curmi = m_parmodel->GetCurMotInfo();
			if (curmi) {
				int curmotid = curmi->motid;
				int curframe = (int)(curmi->curframe + 0.0001);

				ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
				ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
				if ((g_previewFlag != 4) && (g_previewFlag != 5)) {
					////cureul = CalcCurrentLocalEulXYZ(-1, BEFEUL_BEFFRAME);
					//cureul = CalcLocalEulXYZ(-1, m_curmotid, curframe, BEFEUL_BEFFRAME, 0);
					//if (g_limitdegflag) {
					//	neweul = LimitEul(cureul);
					//}
					//else {
					//	neweul = cureul;
					//}
					
					neweul = GetLocalEul(limitdegflag, m_curmotid, curframe, 0);

				}
				else {
					neweul = GetBtEul();
				}
				m_anglelimit.chkeul[AXIS_X] = neweul.x;
				m_anglelimit.chkeul[AXIS_Y] = neweul.y;
				m_anglelimit.chkeul[AXIS_Z] = neweul.z;
			}
			else {
				_ASSERT(0);
			}
		}
		else {
			_ASSERT(0);
		}


		//m_anglelimit.chkeul[AXIS_X] = neweul.x;
		//m_anglelimit.chkeul[AXIS_Y] = neweul.y;
		//m_anglelimit.chkeul[AXIS_Z] = neweul.z;
	}

	return m_anglelimit;
};

int CBone::SwapAngleLimitUpperLowerIfRev()
{
	int axiskind;
	for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++) {
		if (m_anglelimit.lower[axiskind] > m_anglelimit.upper[axiskind]) {
			//_ASSERT(0);
			//swap
			int tmpval = m_anglelimit.lower[axiskind];
			m_anglelimit.lower[axiskind] = m_anglelimit.upper[axiskind];
			m_anglelimit.upper[axiskind] = tmpval;
		}
	}

	return 0;
}

void CBone::SetAngleLimit(bool limitdegflag, ANGLELIMIT srclimit)
{
	m_anglelimit = srclimit;

	//int axiskind;
	//for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++){
		//if (m_anglelimit.lower[axiskind] < -180){
		//	m_anglelimit.lower[axiskind] = -180;
		//}
		//else if (m_anglelimit.lower[axiskind] > 180){
		//	m_anglelimit.lower[axiskind] = 180;
		//}

		//if (m_anglelimit.upper[axiskind] < -180){
		//	m_anglelimit.upper[axiskind] = -180;
		//}
		//else if (m_anglelimit.upper[axiskind] > 180){
		//	m_anglelimit.upper[axiskind] = 180;
		//}
	//}

	SwapAngleLimitUpperLowerIfRev();
	SetAngleLimitOff(&m_anglelimit);

	ChaVector3 neweul;
	neweul.x = m_anglelimit.chkeul[AXIS_X];
	neweul.y = m_anglelimit.chkeul[AXIS_Y];
	neweul.z = m_anglelimit.chkeul[AXIS_Z];

	ChaVector3 limiteul = neweul;
	//if (g_limitdegflag) {
	//	limiteul = LimitEul(neweul);
	//}
	//else {
	//	limiteul = neweul;
	//}

	if (GetParModel()) {
		MOTINFO* curmi = GetParModel()->GetCurMotInfo();
		if (curmi) {
			int curmotid;
			double curframe;
			curmotid = curmi->motid;
			curframe = curmi->curframe;

			limiteul = GetLocalEul(limitdegflag, curmotid, curframe, 0);
		}
	}

	m_anglelimit.chkeul[AXIS_X] = limiteul.x;
	m_anglelimit.chkeul[AXIS_Y] = limiteul.y;
	m_anglelimit.chkeul[AXIS_Z] = limiteul.z;

	//chkチェックスライダーは操作用ではなくリファレンス用にする。元データを上書きしないため。
	//if ((srcmotid >= 1) && (srcframe >= 0.0)) {
	//	//int inittraflag = 0;
	//	//int setchildflag = 1;
	//	//int initscaleflag = 0;
	//	//SetWorldMatFromEul(inittraflag, setchildflag, neweul, srcmotid, srcframe, initscaleflag);
	//	ChaMatrix newworldmat;
	//	ChaMatrixIdentity(&newworldmat);
	//	//if (g_limitdegflag == true) {
	//	//制限角度有り
	//	ChaVector3 orgeul = CalcLocalEulXYZ(-1, srcmotid, (double)((int)(srcframe + 0.1)), BEFEUL_BEFFRAME);
	//	ChaVector3 neweul = LimitEul(orgeul);
	//	SetLocalEul(srcmotid, (double)((int)(srcframe + 0.1)), neweul);//!!!!!!!!!!!!
	//	newworldmat = CalcWorldMatFromEul(0, 1, neweul, orgeul, srcmotid, (double)((int)(srcframe + 0.1)), 0);
	//	//}
	//	//else {
	//	//	//制限角度無し
	//	//	CallF(CalcFBXMotion(srcmotid, srcframe, &m_curmp, &existflag), return 1);
	//	//	newworldmat = m_curmp.GetWorldMat();// **wmat;
	//	//}
	//	//ChaMatrix tmpmat = newworldmat * *wmat;
	//	//m_curmp.SetWorldMat(newworldmat);//underchecking
	//	m_curmp.SetWorldMat(newworldmat);
	//}

};


int CBone::GetFreeCustomRigNo()
{
	int rigno;
	int findrigno = -1;
	for (rigno = 0; rigno < MAXRIGNUM; rigno++){
		if (m_customrig[rigno].useflag == 0){//0 : free mark
			findrigno = rigno;
			break;
		}
	}
	
	if (findrigno == -1){
		//freeがなかった場合、rentalから
		for (rigno = 0; rigno < MAXRIGNUM; rigno++){
			if (m_customrig[rigno].useflag == 1){//1 : rental mark
				findrigno = rigno;
				break;
			}
		}
	}

	return findrigno;
}
CUSTOMRIG CBone::GetFreeCustomRig()
{
	int freerigno = GetFreeCustomRigNo();
	if ((freerigno >= 0) && (freerigno < MAXRIGNUM)){
		m_customrig[freerigno].useflag = 1;//1 : rental mark
		//_ASSERT(0);
		return m_customrig[freerigno];
	}
	else{
		_ASSERT(0);
		CUSTOMRIG dummycr;
		::InitCustomRig(&dummycr, 0, 0);
		return dummycr;
	}
}

int CBone::InvalidateCustomRig(int rigno)
{
	if ((rigno >= 0) && (rigno < MAXRIGNUM)) {
		::InitCustomRig(&m_customrig[rigno], this, rigno);
		m_customrig[rigno].useflag = 0;
	}

	return 0;
}


CUSTOMRIG CBone::GetCustomRig(int rigno)
{
	if ((rigno >= 0) && (rigno < MAXRIGNUM)){
		return m_customrig[rigno];
	}
	else{
		_ASSERT(0);
		CUSTOMRIG dummycr;
		::InitCustomRig(&dummycr, 0, 0);
		return dummycr;
	}
}
void CBone::SetCustomRig(CUSTOMRIG srccr)
{
	int isvalid = IsValidCustomRig(m_parmodel, srccr, this);
	if (isvalid == 1){
		m_customrig[srccr.rigno] = srccr;
		m_customrig[srccr.rigno].useflag = 2;//2 : valid mark
	}
}

ChaMatrix CBone::CalcSymXMat(bool limitdegflag, int srcmotid, double srcframe)
{
	return CalcSymXMat2(limitdegflag, srcmotid, srcframe, SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS);
}

ChaMatrix CBone::CalcSymXMat2(bool limitdegflag, int srcmotid, double srcframe, int symrootmode)
{
	/*
	enum
	{
		//for bit mask operation
		SYMROOTBONE_SAMEORG = 0,
		SYMROOTBONE_SYMDIR = 1,
		SYMROOTBONE_SYMPOS = 2
	};
	*/

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix directsetmat;
	ChaMatrixIdentity(&directsetmat);
	//ChaVector3 symscale = ChaVector3(1.0f, 1.0f, 1.0f);
	//symscale = CalcLocalScaleAnim(srcmotid, roundingframe);

	int rotcenterflag1 = 1;
	if (GetParent()){
		directsetmat = CalcLocalSymScaleRotMat(limitdegflag, rotcenterflag1, srcmotid, roundingframe);
	}
	else{
		//root bone
		if (symrootmode == SYMROOTBONE_SAMEORG){
			directsetmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
			return directsetmat;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		else if(symrootmode & SYMROOTBONE_SYMDIR){
			directsetmat = CalcLocalSymScaleRotMat(limitdegflag, rotcenterflag1, srcmotid, roundingframe);
		}
		else{
			directsetmat = CalcLocalScaleRotMat(limitdegflag, rotcenterflag1, srcmotid, roundingframe);
		}
	}

////tra anim
	//ChaVector3 curanimtra = CalcLocalTraAnim(srcmotid, roundingframe);
	ChaVector3 curanimtra = CalcLocalSymTraAnim(limitdegflag, srcmotid, roundingframe);//traanimもsym対応

	if (GetParent()){
		directsetmat.data[MATI_41] += -curanimtra.x;//inv signe
		directsetmat.data[MATI_42] += curanimtra.y;
		directsetmat.data[MATI_43] += curanimtra.z;
	}
	else{
		//root bone
		if (symrootmode & SYMROOTBONE_SYMPOS){
			directsetmat.data[MATI_41] += -curanimtra.x;//inv signe
			directsetmat.data[MATI_42] += curanimtra.y;
			directsetmat.data[MATI_43] += curanimtra.z;
		}
		else{
			directsetmat.data[MATI_41] += curanimtra.x;//same signe
			directsetmat.data[MATI_42] += curanimtra.y;
			directsetmat.data[MATI_43] += curanimtra.z;
		}
	}


	//directsetmat._11 *= symscale.x;
	//directsetmat._12 *= symscale.x;
	//directsetmat._13 *= symscale.x;
	//directsetmat._21 *= symscale.y;
	//directsetmat._22 *= symscale.y;
	//directsetmat._23 *= symscale.y;
	//directsetmat._31 *= symscale.z;
	//directsetmat._32 *= symscale.z;
	//directsetmat._33 *= symscale.z;

	return directsetmat;
}


ChaMatrix CBone::GetWorldMat(bool limitdegflag, 
	int srcmotid, double srcframe, CMotionPoint* srcmp, ChaVector3* dsteul)//default : dsteul = 0
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix curmat;
	ChaMatrixIdentity(&curmat);

	if (srcmp) {
		if (limitdegflag == false) {
			curmat = srcmp->GetWorldMat();
			if (dsteul) {
				*dsteul = srcmp->GetLocalEul();
			}
		}
		else {
			//ChaVector3 limitedeul = ChaVector3(0.0f, 0.0f, 0.0f);
			//curmat = GetLimitedWorldMat(srcmotid, roundingframe, &limitedeul);//roundingframe
			//if (dsteul) {
			//	*dsteul = limitedeul;
			//}
			 
			//if (srcmp->GetCalcLimitedWM() == 2) {
				curmat = srcmp->GetLimitedWM();
				if (dsteul) {
					*dsteul = srcmp->GetLimitedLocalEul();
				}
			//}
			//else {
			//	ChaVector3 neweul = CalcLocalEulAndSetLimitedEul(srcmotid, roundingframe);
			//	curmat = CalcWorldMatFromEul(0, 1, neweul, srcmotid, roundingframe, 0);
			//	if (dsteul) {
			//		*dsteul = neweul;
			//	}
			//}
		}
	}
	else {
		CMotionPoint* curmp;
		curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			if (limitdegflag == false) {
				curmat = curmp->GetWorldMat();
				if (dsteul) {
					*dsteul = curmp->GetLocalEul();
				}
			}
			else {
				//ChaVector3 limitedeul = ChaVector3(0.0f, 0.0f, 0.0f);
				//curmat = GetLimitedWorldMat(srcmotid, roundingframe, &limitedeul);//roundingframe
				//if (dsteul) {
				//	*dsteul = limitedeul;
				//}
				//if (srcmp->GetCalcLimitedWM() == 2) {
					curmat = curmp->GetLimitedWM();
					if (dsteul) {
						*dsteul = curmp->GetLimitedLocalEul();
					}
				//}
				//else {
				//	ChaVector3 neweul = CalcLocalEulAndSetLimitedEul(srcmotid, roundingframe);
				//	curmat = CalcWorldMatFromEul(0, 1, neweul, srcmotid, roundingframe, 0);
				//	if (dsteul) {
				//		*dsteul = neweul;
				//	}
				//}
			}
		}
	}

	return curmat;
}

ChaMatrix CBone::GetCurrentWorldMat()
{
	//CMotionPoint curmp;
	//curmp = GetCurMp();
	//return curmp.GetWorldMat();

	bool currentlimitdegflag = g_limitdegflag;

	MOTINFO* curmi = 0;
	if (GetParModel()) {
		curmi = GetParModel()->GetCurMotInfo();
		if (curmi) {
			int curmotid = curmi->motid;
			double curframe = curmi->curframe;

			ChaMatrix newworldmat;
			ChaMatrixIdentity(&newworldmat);
			//###################################
			//補間のためにroundingframeではない
			//###################################
			CMotionPoint tmpmp;
			int existflag = 0;
			CalcFBXMotion(currentlimitdegflag, curmotid, curframe, &tmpmp, &existflag);
			newworldmat = GetWorldMat(currentlimitdegflag, curmotid, curframe, &tmpmp) * GetParModel()->GetWorldMat();

			return newworldmat;
		}
		else {
			_ASSERT(0);
			return GetParModel()->GetWorldMat();
		}
	}
	else {
		_ASSERT(0);
		ChaMatrix initmat;
		initmat.SetIdentity();
		return initmat;
	}
}

int CBone::SetWorldMat(bool limitdegflag, 
	int srcmotid, double srcframe, ChaMatrix srcmat, CMotionPoint* srcmp)//default : srcmp = 0
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	if (srcmp) {
		if (limitdegflag == false) {
			srcmp->SetWorldMat(srcmat);
		}
		else {
			srcmp->SetLimitedWM(srcmat);
			//srcmp->SetCalcLimitedWM(2);
		}
	}
	else {
		CMotionPoint* curmp;
		curmp = GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			if (limitdegflag == false) {
				curmp->SetWorldMat(srcmat);
			}
			else {
				curmp->SetLimitedWM(srcmat);
				//curmp->SetCalcLimitedWM(2);
			}
		}
	}

	return 0;
}


ChaMatrix CBone::CalcLocalScaleRotMat(bool limitdegflag, int rotcenterflag, int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix curmat;
	curmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	ChaMatrix parmat, invparmat;
	ChaMatrixIdentity(&parmat);
	ChaMatrixIdentity(&invparmat);
	if (m_parent) {
		parmat = m_parent->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		invparmat = ChaMatrixInv(parmat);
	}
	ChaMatrix localmat;
	localmat = curmat * invparmat;

	ChaMatrix localsmat, localrmat, localtmat;
	GetSRTMatrix2(localmat, &localsmat, &localrmat, &localtmat);

	//CMotionPoint curlocalmp;
	//CalcLocalInfo(srcmotid, roundingframe, &curlocalmp);
	ChaMatrix currotmat;
	//currotmat = curlocalmp.GetQ().MakeRotMatX();

	//currotmat._41 = 0.0f;
	//currotmat._42 = 0.0f;
	//currotmat._43 = 0.0f;

	if (rotcenterflag == 1){
		ChaMatrix befrotmat, aftrotmat;
		ChaMatrixIdentity(&befrotmat);
		ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
		ChaMatrixIdentity(&aftrotmat);
		ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
		currotmat = befrotmat * localsmat * localrmat * aftrotmat;// smat有り　！！！！
	}
	else {
		_ASSERT(0);
		currotmat = localsmat * localrmat;
	}

	return currotmat;
}


ChaMatrix CBone::CalcLocalSymScaleRotMat(bool limitdegflag, int rotcenterflag, int srcmotid, double srcframe)
{
	ChaMatrix retmat;

	int symboneno = 0;
	int existflag = 0;
	m_parmodel->GetSymBoneNo(GetBoneNo(), &symboneno, &existflag);
	if (symboneno >= 0){
		CBone* symbone = m_parmodel->GetBoneByID(symboneno);
		_ASSERT(symbone);
		if (symbone){
			//if (symbone == this){
			//	WCHAR dbgmes[1024];
			//	swprintf_s(dbgmes, 1024, L"CalcLocalSymRotMat : frame %lf : samebone : this[%s]--sym[%s]", srcframe, GetWBoneName(), symbone->GetWBoneName());
			//	::MessageBox(NULL, dbgmes, L"check", MB_OK);
			//}

			CMotionPoint symlocalmp;
			symbone->CalcLocalInfo(limitdegflag, srcmotid, srcframe, &symlocalmp);
			retmat = symlocalmp.GetQ().CalcSymX2();

			ChaVector3 symscale = ChaVector3(1.0f, 1.0f, 1.0f);
			symscale = symbone->CalcLocalScaleAnim(limitdegflag, srcmotid, srcframe);
			ChaMatrix symscalemat;
			ChaMatrixIdentity(&symscalemat);
			ChaMatrixScaling(&symscalemat, symscale.x, symscale.y, symscale.z);

			retmat.data[MATI_41] = 0.0f;
			retmat.data[MATI_42] = 0.0f;
			retmat.data[MATI_43] = 0.0f;

			if (rotcenterflag == 1){
				ChaMatrix befrotmat, aftrotmat;
				ChaMatrixIdentity(&befrotmat);
				ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
				ChaMatrixIdentity(&aftrotmat);
				ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
				retmat = befrotmat * symscalemat * retmat * aftrotmat;
			}
			else {
				retmat = symscalemat * retmat;
			}
		}
		else{
			retmat = CalcLocalScaleRotMat(limitdegflag, rotcenterflag, srcmotid, srcframe);

			//CalcLocalRotMatはrotcenter対応、scale有り!!!!!!!!!!!!!!
			

			//ChaVector3 symscale = ChaVector3(1.0f, 1.0f, 1.0f);
			//symscale = CalcLocalScaleAnim(srcmotid, srcframe);
			//ChaMatrix symscalemat;
			//ChaMatrixIdentity(&symscalemat);
			//ChaMatrixScaling(&symscalemat, symscale.x, symscale.y, symscale.z);

			//if (rotcenterflag == 1) {
			//	ChaMatrix befrotmat, aftrotmat;
			//	ChaMatrixIdentity(&befrotmat);
			//	ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
			//	ChaMatrixIdentity(&aftrotmat);
			//	ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
			//	retmat = befrotmat * symscalemat * retmat * aftrotmat;
			//}
			//else {
			//	retmat = symscalemat * retmat;
			//}

			_ASSERT(0);
		}
	}
	else{
		retmat = CalcLocalScaleRotMat(limitdegflag, rotcenterflag, srcmotid, srcframe);

		//CalcLocalRotMatはrotcenter対応、scale有り!!!!!!!!!!!!!!


		//ChaVector3 symscale = ChaVector3(1.0f, 1.0f, 1.0f);
		//symscale = CalcLocalScaleAnim(srcmotid, srcframe);
		//ChaMatrix symscalemat;
		//ChaMatrixIdentity(&symscalemat);
		//ChaMatrixScaling(&symscalemat, symscale.x, symscale.y, symscale.z);

		//if (rotcenterflag == 1) {
		//	ChaMatrix befrotmat, aftrotmat;
		//	ChaMatrixIdentity(&befrotmat);
		//	ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
		//	ChaMatrixIdentity(&aftrotmat);
		//	ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
		//	retmat = befrotmat * symscalemat * retmat * aftrotmat;
		//}
		//else {
		//	retmat = symscalemat * retmat;
		//}

		_ASSERT(0);
	}

	return retmat;
}

ChaVector3 CBone::CalcLocalSymScaleVec(bool limitdegflag, int srcmotid, double srcframe)
{
	ChaVector3 retscale;

	int symboneno = 0;
	int existflag = 0;
	m_parmodel->GetSymBoneNo(GetBoneNo(), &symboneno, &existflag);
	if (symboneno >= 0) {
		CBone* symbone = m_parmodel->GetBoneByID(symboneno);
		_ASSERT(symbone);
		if (symbone) {
			//if (symbone == this){
			//	WCHAR dbgmes[1024];
			//	swprintf_s(dbgmes, 1024, L"CalcLocalSymRotMat : frame %lf : samebone : this[%s]--sym[%s]", srcframe, GetWBoneName(), symbone->GetWBoneName());
			//	::MessageBox(NULL, dbgmes, L"check", MB_OK);
			//}
			retscale = symbone->CalcLocalScaleAnim(limitdegflag, srcmotid, srcframe);
		}
		else {
			retscale = CalcLocalScaleAnim(limitdegflag, srcmotid, srcframe);
			_ASSERT(0);
		}
	}
	else {
		retscale = CalcLocalScaleAnim(limitdegflag, srcmotid, srcframe);
		_ASSERT(0);
	}

	return retscale;
}



ChaVector3 CBone::CalcLocalTraAnim(bool limitdegflag, int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix curmat;
	curmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	ChaMatrix localmat;
	localmat.SetIdentity();
	if (GetParent()){
		ChaMatrix parmat;
		parmat = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		localmat = curmat * ChaMatrixInv(parmat);
	}
	else {
		localmat = curmat;
	}

	ChaMatrix smat, rmat, tmat;
	GetSRTMatrix2(localmat, &smat, &rmat, &tmat);

	ChaVector3 jointpos;
	ChaMatrix beftramat, afttramat;
	ChaMatrix localSRmat;
	ChaMatrixIdentity(&beftramat);
	ChaMatrixIdentity(&afttramat);
	ChaMatrixIdentity(&localSRmat);
	jointpos = GetJointFPos();
	ChaMatrixTranslation(&beftramat, -jointpos.x, -jointpos.y, -jointpos.z);
	ChaMatrixTranslation(&afttramat, jointpos.x, jointpos.y, jointpos.z);
	localSRmat = beftramat * smat * rmat * afttramat;

	ChaVector3 srpos, localpos;
	ChaVector3TransformCoord(&srpos, &jointpos, &localSRmat);
	ChaVector3TransformCoord(&localpos, &jointpos, &localmat);

	ChaVector3 rettraanim;
	rettraanim = localpos - srpos;
	return rettraanim;

}

ChaVector3 CBone::CalcLocalSymTraAnim(bool limitdegflag, int srcmotid, double srcframe)
{

	ChaVector3 rettra = ChaVector3(0.0f, 0.0f, 0.0);//scaleに設定されていてもrotcenterの位置になる

	//int symboneno = 0;
	//int existflag = 0;
	//m_parmodel->GetSymBoneNo(GetBoneNo(), &symboneno, &existflag);
	//if (symboneno >= 0) {
	//	CBone* symbone = m_parmodel->GetBoneByID(symboneno);
	//	_ASSERT(symbone);
	//	if (symbone) {
	//		//if (symbone == this){
	//		//	WCHAR dbgmes[1024];
	//		//	swprintf_s(dbgmes, 1024, L"CalcLocalSymRotMat : frame %lf : samebone : this[%s]--sym[%s]", srcframe, GetWBoneName(), symbone->GetWBoneName());
	//		//	::MessageBox(NULL, dbgmes, L"check", MB_OK);
	//		//}
	//		rettra = symbone->CalcLocalTraAnim(srcmotid, srcframe);
	//		rettra.x *= -1.0f;
	//	}
	//	else {
	//		rettra = CalcLocalScaleAnim(srcmotid, srcframe);
	//		_ASSERT(0);
	//	}
	//}
	//else {
	//	rettra = CalcLocalScaleAnim(srcmotid, srcframe);
	//	_ASSERT(0);
	//}

	return rettra;

}

ChaVector3 CBone::CalcFbxScaleAnim(bool limitdegflag, int srcmotid, double srcframe)
{
	//############################
	//fbx書き出し専用
	//############################

	//############################################################################
	// 2022/09/12 
	// fromnobindpose : bindpose無しのfbx書き出し：NodeMatを掛けた姿勢を書き出す。
	//############################################################################

	//############################################################################
	// 2022/10/31 
	// NodeMatを掛けた姿勢を書き出す。
	//############################################################################

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 svec, tvec;
	ChaMatrix rmat;
	ChaVector3 iniscale = ChaVector3(1.0f, 1.0f, 1.0f);

	ChaMatrix wmanim = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	ChaMatrix fbxwm = GetNodeMat() * wmanim;
	ChaMatrix parentfbxwm;
	parentfbxwm.SetIdentity();
	if (GetParent()) {
		ChaMatrix parentwmanim = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parentfbxwm = GetParent()->GetNodeMat() * parentwmanim;
	}

	ChaMatrix localfbxmat = fbxwm * ChaMatrixInv(parentfbxwm);

	GetSRTMatrix(localfbxmat, &svec, &rmat, &tvec);
	return svec;
}



ChaVector3 CBone::CalcLocalScaleAnim(bool limitdegflag, int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 svec, tvec;
	ChaMatrix rmat;
	ChaVector3 iniscale = ChaVector3(1.0f, 1.0f, 1.0f);

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, roundingframe);
	if (GetParent()) {
		if (pcurmp) {
			pparmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
			if (pparmp) {
				CMotionPoint setmp;
				ChaMatrix invpar = ChaMatrixInv(GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, pparmp));
				ChaMatrix localmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, pcurmp) * invpar;

				GetSRTMatrix(localmat, &svec, &rmat, &tvec);
				return svec;
			}
			else {
				_ASSERT(0);
				return iniscale;
			}
		}
		else {
			return iniscale;
		}
	}
	else {
		if (pcurmp) {
			CMotionPoint setmp;
			ChaMatrix localmat = GetWorldMat(limitdegflag, srcmotid, roundingframe, pcurmp);

			GetSRTMatrix(localmat, &svec, &rmat, &tvec);
			return svec;

		}
		else {
			return iniscale;
		}
	}
}



int CBone::PasteMotionPoint(bool limitdegflag, int srcmotid, double srcframe, CMotionPoint srcmp)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* newmp = 0;
	newmp = GetMotionPoint(srcmotid, roundingframe);
	if (newmp){
		//ChaMatrix setmat = GetWorldMat(srcmotid, roundingframe, &srcmp);
		ChaMatrix localmat = srcmp.GetWorldMat();//2023/02/05 コピー情報はunlimitedの方に入っている

		ChaMatrix setmat;
		setmat.SetIdentity();
		if (GetParent()){
			ChaMatrix parentwm;
			parentwm = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
			setmat = localmat * parentwm;//copy情報はローカルなのでグロバールにする
		}
		else {
			setmat = localmat;
		}

		//bool directsetflag = false;
		//bool infooutflag = false;
		//int setchildflag = 1;//setchildflagは directsetflag == falseのときしか働かない
		//SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag, srcmotid, roundingframe, setmat);

		UpdateCurrentWM(limitdegflag, srcmotid, roundingframe, setmat);

	}

	return 0;
}

ChaVector3 CBone::CalcFBXEulXYZ(bool limitdegflag, int srcmotid, double srcframe, ChaVector3* befeulptr)
{

	//############################
	// fbx書き出し専用
	//############################

	//############################################################################
	// 2022/09/12 
	// fromnobindpose : bindpose無しのfbx書き出し：NodeMatを掛けた姿勢を書き出す。
	//############################################################################

	//################################################
	//必要ノイズ付与機能付き　: FBX書き出し時のみ使用
	//################################################

	//############################################################################
	// 2022/10/31 
	// NodeMatを掛けた姿勢を書き出す。
	//############################################################################

	//#####################################################################
	// 2023/02/16
	// オイラー角がMayaと同じなのだから　そのまま書き出す方が合っている
	// ジョイントの向きが設定されていても正しく書き出せる
	//#####################################################################

	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaVector3 cureul;
	cureul = CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);

	return cureul;
}
//ChaVector3 CBone::CalcFBXEulZXY(int srcmotid, double srcframe, ChaVector3* befeulptr)
//{
//	CMotionPoint tmpmp;
//	CalcLocalInfo(srcmotid, srcframe, &tmpmp);
//	int isfirstbone;
//	if (GetParent()){
//		isfirstbone = 0;
//	}
//	else{
//		isfirstbone = 1;
//	}
//
//	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
//	if (befeulptr){
//		befeul = *befeulptr;
//	}
//
//	//tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);
//	tmpmp.GetQ().CalcFBXEulZXY(0, befeul, &cureul, isfirstbone);
//
//	return cureul;
//
//}
ChaVector3 CBone::CalcFBXTra(bool limitdegflag, int srcmotid, double srcframe)
{
	//############################
	// fbx書き出し専用
	//############################

	//############################################################################
	// 2022/09/12 
	// fromnobindpose : bindpose無しのfbx書き出し：NodeMatを掛けた姿勢を書き出す。
	//############################################################################

	//############################################################################
	// 2022/10/31 
	// NodeMatを掛けた姿勢を書き出す。
	//############################################################################
	
	double roundingframe = (double)((int)(srcframe + 0.0001));

	ChaMatrix wmanim = GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	ChaMatrix fbxwm = GetNodeMat() * wmanim;
	ChaMatrix parentfbxwm;
	parentfbxwm.SetIdentity();
	if (GetParent()) {
		ChaMatrix parentwmanim = GetParent()->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parentfbxwm = GetParent()->GetNodeMat() * parentwmanim;
	}

	ChaMatrix localfbxmat = fbxwm * ChaMatrixInv(parentfbxwm);

	ChaVector3 fbxtra = ChaVector3(localfbxmat.data[MATI_41], localfbxmat.data[MATI_42], localfbxmat.data[MATI_43]);
	return fbxtra;

}

int CBone::QuaternionInOrder(bool limitdegflag, int srcmotid, double srcframe, CQuaternion* srcdstq)
{
	CQuaternion beflocalq;
	CMotionPoint befmp;
	double befframe = srcframe - 1.0;
	if (befframe >= 0.0001){
		CalcLocalInfo(limitdegflag, srcmotid, befframe, &befmp);
		beflocalq = befmp.GetQ();
	}

	beflocalq.InOrder(srcdstq);

	return 0;

}

/*
int CBone::CalcNewBtMat(CRigidElem* srcre, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos)
{
	ChaMatrixIdentity(dstmat);
	*dstpos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (!childbone || !dstmat || !dstpos){
		return 1;
	}

	ChaVector3 jointfpos;
	ChaMatrix firstworld;
	ChaMatrix invfirstworld;
	ChaMatrix curworld;
	ChaMatrix befworld;
	ChaMatrix invbefworld;
	ChaMatrix diffworld;
	ChaVector3 rigidcenter;
	ChaMatrix multmat;
	ChaMatrix tramat;


	firstworld = GetStartMat2();
	ChaMatrixInverse(&invfirstworld, NULL, &firstworld);


	//current
	if (GetBtKinFlag() == 1){
		diffworld = invfirstworld * GetCurMp().GetWorldMat();
		tramat = GetCurMp().GetWorldMat();
	}
	else{
		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
		if (GetCurMp().GetBtFlag() == 0){
			diffworld = invfirstworld * GetCurMp().GetWorldMat();
			tramat = GetCurMp().GetWorldMat();
		}
		else{
			//ここでのBtMatは一回前の姿勢。
			diffworld = invfirstworld * GetCurMp().GetBtMat();
			tramat = GetCurMp().GetBtMat();
		}
	}
	jointfpos = GetJointFPos();
	ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);

	//child
	jointfpos = childbone->GetJointFPos();
	ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);



	multmat = srcre->GetFirstcapsulemat() * diffworld;
	//rigidcenter = (m_btparentpos + m_btchildpos) * 0.5f;
	rigidcenter = m_btparentpos;


	*dstmat = multmat;
	*dstpos = rigidcenter;

	return 0;
}
*/



//#################################################
//2023/01/18
//通常モーションと物理剛体の軸合わせをしたので
//回転情報は通常モーションと物理とで共通となった
//#################################################
int CBone::CalcNewBtMat(CModel* srcmodel, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos)
{

	//srcframe : 時間補間有り

	ChaMatrixIdentity(dstmat);
	*dstpos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (!childbone || !dstmat || !dstpos){
		return 1;
	}

	ChaVector3 jointfpos;
	ChaMatrix curworld;
	ChaMatrix befworld;
	ChaMatrix invbefworld;
	ChaVector3 rigidcenter;
	ChaMatrix rotmat;
	ChaMatrix tramat, childtramat;

	ChaMatrix befbtmat;
	befbtmat = GetBtMat();

	//if (GetBtFlag() == 0){
		//再帰処理中のまだ未セットの状態の場合
		//befbtmat = GetBtMat();
	//}
	//else{
	//	//再帰処理中のすでにセットした状態の場合
	//	befbtmat = GetBefBtMat();
	//}


	curworld = GetCurrentWorldMat();


	//current
	if (GetBtKinFlag() == 1){
		//tramat = GetCurMp().GetWorldMat();
		tramat = curworld;
		rotmat = ChaMatrixRot(tramat);

		jointfpos = GetJointFPos();
		ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
		jointfpos = childbone->GetJointFPos();
		ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);

	}
	else{
		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
		if (srcmodel->GetBtCnt() == 0){
			//tramat = GetCurMp().GetWorldMat();
			tramat = curworld;
			rotmat = ChaMatrixRot(tramat);

			jointfpos = GetJointFPos();
			ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
			jointfpos = childbone->GetJointFPos();
			ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
		}
		else{
			//親方向に　Kinematicのボーンを探す
			CBone* kinematicbone = 0;
			CBone* findbone = GetParent();
			while (findbone) {
				if (findbone->GetBtKinFlag() != 0) {
					kinematicbone = findbone;
					break;
				}
				findbone = findbone->GetParent();
			}

			//2023/01/28
			//純粋な物理計算においては　Kinematic部分だけ手動で移動すれば良いのだが
			//計算が乱れやすく　大げさになり易いので
			//Kinematicとそうではない境目のKinematicの　全フレームからの移動分を　子供ジョイントに波及させる
			//この処理を加えることにより　ジャンプして着地した時の　乱れ方が　大きくなり過ぎないようになった
			if (kinematicbone) {
				ChaMatrix befparentwm, curparentwm;
				befparentwm = kinematicbone->GetBtMat();//実質一回前の　BtMat
				curparentwm = kinematicbone->GetCurrentWorldMat();//カレントのKinematic姿勢

				jointfpos = kinematicbone->GetJointFPos();
				ChaVector3 befparentpos, curparentpos;
				ChaVector3TransformCoord(&befparentpos, &jointfpos, &befparentwm);
				ChaVector3TransformCoord(&curparentpos, &jointfpos, &curparentwm);
				ChaVector3 movevec;
				movevec = curparentpos - befparentpos;//前回から今回への　位置移動分
				ChaMatrix movemat;
				movemat.SetIdentity();
				movemat.SetTranslation(movevec);

				tramat = befbtmat * movemat;
			}
			else {
				tramat = befbtmat;
			}

				
			rotmat = ChaMatrixRot(tramat);				

			jointfpos = GetJointFPos();
			ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
			jointfpos = childbone->GetJointFPos();
			ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
		}
	}

	rigidcenter = (m_btparentpos + m_btchildpos) * 0.5f;


	*dstmat = rotmat;
	*dstpos = rigidcenter;

	return 0;
}


//###################################################################################
//2023/01/18よりも前のバージョン
//通常モーションと物理剛体の軸が異なる場合の　diffを使ったやり方
//軸合わせをしたので　回転情報をそのままセットする方法に変えた　よってコメントアウト
//###################################################################################
//int CBone::CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos)
//{
//	ChaMatrixIdentity(dstmat);
//	*dstpos = ChaVector3(0.0f, 0.0f, 0.0f);
//
//	if (!childbone || !dstmat || !dstpos) {
//		return 1;
//	}
//
//	ChaVector3 jointfpos;
//	ChaMatrix firstmat;
//	ChaMatrix invfirstmat;
//	ChaMatrix curworld;
//	ChaMatrix befworld;
//	ChaMatrix invbefworld;
//	ChaMatrix diffworld;
//	ChaVector3 rigidcenter;
//	ChaMatrix multmat;
//	ChaMatrix tramat;
//
//
//	//firstmat = GetFirstMat();
//	firstmat = GetCurrentZeroFrameMat(0);
//	ChaMatrixInverse(&invfirstmat, NULL, &firstmat);
//
//	ChaMatrix befbtmat;
//	if (GetBtFlag() == 0) {
//		//再帰処理中のまだ未セットの状態の場合
//		befbtmat = GetBtMat();
//	}
//	else {
//		//再帰処理中のすでにセットした状態の場合
//		befbtmat = GetBefBtMat();
//	}
//
//	//current
//	if (GetBtKinFlag() == 1) {
//		diffworld = invfirstmat * GetCurMp().GetWorldMat();
//		tramat = GetCurMp().GetWorldMat();
//
//		jointfpos = GetJointFPos();
//		ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
//		jointfpos = childbone->GetJointFPos();
//		ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
//
//	}
//	else {
//		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
//		if (srcmodel->GetBtCnt() == 0) {
//			diffworld = invfirstmat * GetCurMp().GetWorldMat();
//			tramat = GetCurMp().GetWorldMat();
//
//			jointfpos = GetJointFPos();
//			ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
//			jointfpos = childbone->GetJointFPos();
//			ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
//		}
//		else {
//			if (GetParent() && (GetParent()->GetBtKinFlag() == 1)) {
//				//ここでのBtMatは一回前の姿勢。
//
//				//BtMatにアニメーションの移動成分のみを掛けたものを新しい姿勢行列として子供ジョイント位置を計算してシミュレーションに使用する。
//				curworld = GetCurMp().GetWorldMat();
//				//befworld = GetCurMp().GetBefWorldMat();
//				befworld = GetCurrentZeroFrameMat(0);
//
//				ChaVector3 befparentpos, curparentpos;
//				jointfpos = GetJointFPos();
//				ChaVector3TransformCoord(&befparentpos, &jointfpos, &befworld);
//				ChaVector3TransformCoord(&curparentpos, &jointfpos, &curworld);
//				ChaVector3 diffmv = curparentpos - befparentpos;
//
//				ChaMatrix diffmvmat;
//				ChaMatrixIdentity(&diffmvmat);
//				ChaMatrixTranslation(&diffmvmat, diffmv.x, diffmv.y, diffmv.z);
//
//				ChaMatrixInverse(&invbefworld, NULL, &befworld);
//				ChaMatrix newtramat = befbtmat * diffmvmat;
//
//				diffworld = invfirstmat * newtramat;
//
//				m_btparentpos = curparentpos;
//				jointfpos = childbone->GetJointFPos();
//				//ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &befbtmat);
//				ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &newtramat);
//			}
//			else {
//				diffworld = invfirstmat * befbtmat;
//				tramat = befbtmat;
//
//				jointfpos = GetJointFPos();
//				ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
//				jointfpos = childbone->GetJointFPos();
//				ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
//			}
//		}
//	}
//
//	//multmat = srcre->GetBindcapsulemat() * diffworld;
//	CBtObject* curbto = GetBtObject(childbone);
//	if (curbto) {
//		multmat = curbto->GetFirstTransformMatX() * diffworld;
//	}
//	else {
//		multmat = GetCurrentZeroFrameMat(0) * diffworld;
//		_ASSERT(0);
//	}
//	rigidcenter = (m_btparentpos + m_btchildpos) * 0.5f;
//
//	*dstmat = multmat;
//	*dstpos = rigidcenter;
//
//	return 0;
//}

ChaVector3 CBone::GetChildWorld(){
	if (g_previewFlag != 5){
		//ChaVector3TransformCoord(&m_childworld, &m_jointfpos, &m_curmp.GetWorldMat());
		ChaMatrix tmplimwm = GetCurrentWorldMat();
		ChaVector3TransformCoord(&m_childworld, &m_jointfpos, &tmplimwm);
	}
	else{
		ChaMatrix wmat;
		if (m_parent){
			wmat = m_parent->GetBtMat();
		}
		else{
			wmat = GetBtMat();
		}

		//ChaVector3TransformCoord(&m_childworld, &m_jointfpos, &(GetBtMat()));
		ChaVector3TransformCoord(&m_childworld, &m_jointfpos, &wmat);
	}
	return m_childworld;
};

int CBone::LoadCapsuleShape(ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext)
{
	WCHAR wfilename[MAX_PATH];
	WCHAR mpath[MAX_PATH];

	wcscpy_s(mpath, MAX_PATH, g_basedir);
	WCHAR* lasten = 0;
	WCHAR* last2en = 0;
	lasten = wcsrchr(mpath, TEXT('\\'));
	if (lasten){
		*lasten = 0L;
		last2en = wcsrchr(mpath, TEXT('\\'));
		if (last2en){
			*last2en = 0L;
			wcscat_s(mpath, MAX_PATH, L"\\Media\\MameMedia");
		}
	}

	m_coldisp[COL_CONE_INDEX] = new CModel();
	if (!m_coldisp[COL_CONE_INDEX]){
		_ASSERT(0);
		return 1;
	}

	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"cone_dirX.mqo");
	CallF(m_coldisp[COL_CONE_INDEX]->LoadMQO(pdev, pd3dImmediateContext, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_CONE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_CAPSULE_INDEX] = new CModel();
	if (!m_coldisp[COL_CAPSULE_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"capsule_dirX.mqo");
	CallF(m_coldisp[COL_CAPSULE_INDEX]->LoadMQO(pdev, pd3dImmediateContext, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_CAPSULE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_SPHERE_INDEX] = new CModel();
	if (!m_coldisp[COL_SPHERE_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"sphere_dirX.mqo");
	CallF(m_coldisp[COL_SPHERE_INDEX]->LoadMQO(pdev, pd3dImmediateContext, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_SPHERE_INDEX]->MakeDispObj(), return 1);

	m_coldisp[COL_BOX_INDEX] = new CModel();
	if (!m_coldisp[COL_BOX_INDEX]){
		_ASSERT(0);
		return 1;
	}
	swprintf_s(wfilename, MAX_PATH, L"%s\\%s", mpath, L"box.mqo");
	CallF(m_coldisp[COL_BOX_INDEX]->LoadMQO(pdev, pd3dImmediateContext, wfilename, 0, 1.0f, 0), return 1);
	CallF(m_coldisp[COL_BOX_INDEX]->MakeDispObj(), return 1);

	return 0;
}

CModel* CBone::GetColDisp(CBone* childbone, int srcindex)
{
	if ((srcindex < COL_CONE_INDEX) || (srcindex > COL_MAX)) {
		return 0;
	}
	if (!childbone) {
		return 0;
	}

	CRigidElem* curre = GetRigidElem(childbone);
	if (!curre) {
		_ASSERT(0);
		return 0;
	}

	//_ASSERT(colptr);
	_ASSERT(childbone);

	CModel* retcoldisp = m_coldisp[srcindex];
	_ASSERT(retcoldisp);

	return retcoldisp;


}


CModel* CBone::GetCurColDisp(CBone* childbone)
{
	if (!childbone) {
		return 0;
	}

	CRigidElem* curre = GetRigidElem(childbone);
	if (!curre){
		_ASSERT(0);
		return 0;
	}

	//_ASSERT(colptr);
	_ASSERT(childbone);

	CModel* curcoldisp = m_coldisp[curre->GetColtype()];
	_ASSERT(curcoldisp);

	return curcoldisp;
}

void CBone::SetRigidElemOfMap(std::string srcstr, CBone* srcbone, CRigidElem* srcre){
	
	std::map<std::string, std::map<CBone*, CRigidElem*>>::iterator itrremap;
	itrremap = m_remap.find(srcstr);
	if (itrremap != m_remap.end()){

		//itrremap->second[srcbone] = srcre;

		std::map<CBone*, CRigidElem*>::iterator itrsetmap;
		itrsetmap = itrremap->second.find(srcbone);
		if (itrsetmap != itrremap->second.end()){
			CRigidElem* delre = itrsetmap->second;
			if (delre){
				//delete itrsetmap->second;
				CRigidElem::InvalidateRigidElem(delre);
				itrsetmap->second = 0;
			}
			itrsetmap->second = srcre;
		}
		else{
			itrremap->second[srcbone] = srcre;
		}
	}
	else{
		std::map<CBone*, CRigidElem*> newmap;
		newmap[srcbone] = srcre;
		m_remap[srcstr] = newmap;
	}
	
	//((m_remap[ srcstr ])[ srcbone ]) = srcre;
}

int CBone::SetCurrentMotion(int srcmotid, double animleng)
{
	SetCurMotID(srcmotid);
	//ResetMotionCache();

	//int result;
	//result = CreateIndexedMotionPoint(srcmotid, animleng);
	//_ASSERT(result == 0);

	return 0;
}

//current motionのframe 0のworldmat
ChaMatrix CBone::GetCurrentZeroFrameMatFunc(bool limitdegflag, int updateflag, int inverseflag)
{
	//ZeroFrameの編集前と編集後のポーズのdiffをとる必要がある場合に対応する
	//updateflagが1の場合に最新情報。0の場合に前回の取得情報と同じものを返す。

	//static int s_firstgetflag = 0;
	//static ChaMatrix s_firstgetmatrix;
	//static ChaMatrix s_invfirstgetmatrix;

	if ((m_curmotid <= 0) || (m_curmotid > m_motionkey.size())) {
		//_ASSERT(0);
		ChaMatrix inimat;
		ChaMatrixIdentity(&inimat);
		return inimat;
	}

	if (m_curmotid >= 1) {//idは１から
		//CMotionPoint* pcur = m_motionkey[m_curmotid - 1];//idは１から
		CMotionPoint* pcur = m_motionkey[m_curmotid - 1];//idは１から !!!!!!!!!!!!!!
		if (pcur) {
			if ((updateflag == 1) || (m_firstgetflag == 0)) {
				m_firstgetflag = 1;
				m_firstgetmatrix = GetWorldMat(limitdegflag, m_curmotid, 0.0, pcur);
				m_invfirstgetmatrix = ChaMatrixInv(m_firstgetmatrix);
			}

			if (inverseflag == 0) {
				return m_firstgetmatrix;
			}
			else {
				return m_invfirstgetmatrix;
			}
		}
		else {
			ChaMatrix inimat;
			ChaMatrixIdentity(&inimat);
			return inimat;
		}
	}
	else {
		ChaMatrix inimat;
		ChaMatrixIdentity(&inimat);
		return inimat;
	}

}


ChaMatrix CBone::GetCurrentZeroFrameMat(bool limitdegflag, int updateflag)
{
	//ZeroFrameの編集前と編集後のポーズのdiffをとる必要がある場合に対応する
	//updateflagが1の場合に最新情報。0の場合に前回の取得情報と同じものを返す。

	int inverseflag = 0;
	return GetCurrentZeroFrameMatFunc(limitdegflag, updateflag, inverseflag);

}

ChaMatrix CBone::GetCurrentZeroFrameInvMat(bool limitdegflag, int updateflag)
{
	//ZeroFrameの編集前と編集後のポーズのdiffをとる必要がある場合に対応する
	//updateflagが1の場合に最新情報。0の場合に前回の取得情報と同じものを返す。

	int inverseflag = 1;
	return GetCurrentZeroFrameMatFunc(limitdegflag, updateflag, inverseflag);
}


//static func
CBone* CBone::GetNewBone(CModel* parmodel)
{
	//目的としてはメモリの使いまわしではなく、メモリを連続させることでキャッシュヒットの可能性を増すことである

	//parmodelごとの使いまわししか出来ない
	//モデルの削除と作成を繰り返すとメモリが増え続ける
	//しかし必要モデルをあらかじめ作成して、表示非表示を切り替えて（削除作成を繰り返さずに）やりくりすれば良い

	static int s_befheadno = -1;
	static int s_befelemno = -1;

	if (!parmodel) {
		_ASSERT(0);
	}


	int curpoollen;
	curpoollen = (int)s_bonepool.size();

	//if ((s_befheadno != (s_bonepool.size() - 1)) || (s_befelemno != (BONEPOOLBLKLEN - 1))) {//前回リリースしたポインタが最後尾ではない場合

	//前回リリースしたポインタの次のメンバーをチェックして未使用だったらリリース
		int chkheadno;
		chkheadno = s_befheadno;
		int chkelemno;
		chkelemno = s_befelemno + 1;
		//if ((chkheadno >= 0) && (chkheadno >= curpoollen) && (chkelemno >= BONEPOOLBLKLEN)) {
		if ((chkheadno >= 0) && (chkheadno < (curpoollen - 1)) && (chkelemno >= BONEPOOLBLKLEN)) {//2021/08/21
			chkelemno = 0;
			chkheadno++;
		}
		if ((chkheadno >= 0) && (chkheadno < curpoollen) && (chkelemno >= 0) && (chkelemno < BONEPOOLBLKLEN)) {
			CBone* curbonehead = s_bonepool[chkheadno];
			if (curbonehead) {
				CBone* chkbone;
				chkbone = curbonehead + chkelemno;
				if (chkbone && (chkbone->GetParModel() == parmodel)) {//parmodelが同じ必要有。
					if (chkbone->GetUseFlag() == 0) {
						chkbone->InitParamsForReUse(parmodel);//

						s_befheadno = chkheadno;
						s_befelemno = chkelemno;

						return chkbone;
					}
				}
				else if (chkbone && (chkbone->GetParModel() == 0)) {
					chkbone->InitParamsForReUse(parmodel);//

					s_befheadno = chkheadno;
					s_befelemno = chkelemno;

					return chkbone;
				}
			}
		}

		//if ((chkheadno >= 0) && (chkheadno < curpoollen)) {
			//プールを先頭から検索して未使用がみつかればそれをリリース
		int boneno;
		for (boneno = 0; boneno < curpoollen; boneno++) {
			CBone* curbonehead = s_bonepool[boneno];
			if (curbonehead) {
				int elemno;
				for (elemno = 0; elemno < BONEPOOLBLKLEN; elemno++) {
					CBone* curbone;
					curbone = curbonehead + elemno;
					if (curbone && (curbone->GetParModel() == parmodel)) {//parmodelが同じ必要有。
						if (curbone->GetUseFlag() == 0) {
							curbone->InitParamsForReUse(parmodel);

							s_befheadno = boneno;
							s_befelemno = elemno;

							return curbone;
						}
					}
					else if (curbone && (curbone->GetParModel() == 0)) {
						if (curbone->GetUseFlag() == 0) {
							curbone->InitParamsForReUse(parmodel);

							s_befheadno = boneno;
							s_befelemno = elemno;

							return curbone;
						}
					}
				}
			}
		}
		//}
	//}


	//未使用boneがpoolに無かった場合、アロケートしてアロケートした先頭のポインタをリリース
	CBone* allocbone;
	allocbone = new CBone[BONEPOOLBLKLEN];
	if (!allocbone) {
		_ASSERT(0);

		s_befheadno = -1;
		s_befelemno = -1;

		return 0;
	}
	int allocno;
	for (allocno = 0; allocno < BONEPOOLBLKLEN; allocno++) {
		CBone* curallocbone = allocbone + allocno;
		if (curallocbone) {
			//int indexofpool = curpoollen + allocno;
			int indexofpool = curpoollen;//pool[indexofpool] 2021/08/19
			curallocbone->InitParams();
			curallocbone->SetParams(parmodel);//!!!!!作成時にはparmodel以外にボーン番号なども決定
			curallocbone->SetUseFlag(0);
			curallocbone->SetIndexOfPool(indexofpool);

			if (allocno == 0) {
				curallocbone->SetIsAllocHead(1);
			}
			else {
				curallocbone->SetIsAllocHead(0);
			}
		}
		else {
			_ASSERT(0);

			s_befheadno = -1;
			s_befelemno = -1;

			return 0;
		}
	}
	s_bonepool.push_back(allocbone);//allocate block(アロケート時の先頭ポインタ)を格納

	allocbone->SetUseFlag(1);


	s_befheadno = (int)s_bonepool.size() - 1;
	if (s_befheadno < 0) {
		s_befheadno = 0;
	}
	s_befelemno = 0;

	return allocbone;
}

//static func
void CBone::InvalidateBone(CBone* srcbone)
{
	if (!srcbone) {
		_ASSERT(0);
		return;
	}

	int saveindex = srcbone->GetIndexOfPool();
	int saveallochead = srcbone->IsAllocHead();
	CModel* saveparmodel = srcbone->GetParModel();

	srcbone->DestroyObjs();

	srcbone->InitParams();
	srcbone->SetUseFlag(0);
	srcbone->SetIsAllocHead(saveallochead);
	srcbone->SetIndexOfPool(saveindex);
	srcbone->m_parmodel = saveparmodel;
}

//static func
void CBone::InitBones()
{
	s_bonepool.clear();
}

//static func
void CBone::DestroyBones() 
{
	int boneallocnum = (int)s_bonepool.size();
	int boneno;
	for (boneno = 0; boneno < boneallocnum; boneno++) {
		CBone* delbone;
		delbone = s_bonepool[boneno];
		//if (delbone && (delbone->IsAllocHead() == 1)) {
		if (delbone) {
			delete[] delbone;
		}
	}
	s_bonepool.clear();
}

void CBone::OnDelModel(CModel* srcparmodel)
{
	//if ((chkheadno >= 0) && (chkheadno < curpoollen)) {
		//プールを先頭から検索して未使用がみつかればそのparmodelを０にする

	int curpoollen;
	curpoollen = (int)s_bonepool.size();

	int boneno;
	for (boneno = 0; boneno < curpoollen; boneno++) {
		CBone* curbonehead = s_bonepool[boneno];
		if (curbonehead) {
			int elemno;
			for (elemno = 0; elemno < BONEPOOLBLKLEN; elemno++) {
				CBone* curbone;
				curbone = curbonehead + elemno;
				if (curbone && (curbone->GetParModel() == srcparmodel)) {//parmodelが同じ必要有。
					//if (curbone && (curbone->GetUseFlag() == 0)) {//srcparmodelに関して再利用を防ぐ
						curbone->m_parmodel = 0;
						curbone->SetUseFlag(0);
					//}
				}
			}
		}
	}
	//}

	map<CModel*, int>::iterator itrbonecnt;
	itrbonecnt = g_bonecntmap.find(srcparmodel);
	if (itrbonecnt != g_bonecntmap.end()) {
		g_bonecntmap.erase(itrbonecnt);//エントリー削除
	}

}


ChaMatrix CBone::CalcParentGlobalMat(int srcmotid, double srcframe)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	if (!GetParent()) {
		return retmat;
	}


	CalcParentGlobalMatReq(&retmat, GetParent(), srcmotid, srcframe);


	return retmat;
}


void CBone::CalcParentGlobalMatReq(ChaMatrix* dstmat, CBone* srcbone, int srcmotid, double srcframe)
{
	if (!srcbone) {
		return;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp;
	bool onaddmotion = true;
	curmp = srcbone->GetMotionPoint(srcmotid, roundingframe, onaddmotion);
	if (curmp) {
		ChaMatrix localmat = curmp->GetLocalMat();
		*dstmat = *dstmat * localmat;//childmat * currentmat   (currentmat * parentmat)
	}

	if (srcbone->GetParent()) {
		CalcParentGlobalMatReq(dstmat, srcbone->GetParent(), srcmotid, roundingframe);
	}

}


//SRT形式
ChaMatrix CBone::CalcParentGlobalSRT(int srcmotid, double srcframe)
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	if (!GetParent()) {
		return retmat;
	}


	CalcParentGlobalSRTReq(&retmat, GetParent(), srcmotid, srcframe);


	return retmat;
}

//SRT形式
void CBone::CalcParentGlobalSRTReq(ChaMatrix* dstmat, CBone* srcbone, int srcmotid, double srcframe)
{
	if (!srcbone || !dstmat) {
		return;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* mpptr = GetMotionPoint(srcmotid, roundingframe);
	if (!mpptr) {
		return;
	}

	ChaMatrix curSRT = mpptr->GetSRT();
	*dstmat = *dstmat * curSRT;//childmat * currentmat   (currentmat * parentmat)

	if (srcbone->GetParent()) {
		CalcParentGlobalSRTReq(dstmat, srcbone->GetParent(), srcmotid, roundingframe);
	}

}

//SRT形式
ChaMatrix CBone::CalcFirstParentGlobalSRT()
{
	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);

	if (!GetParent()) {
		return retmat;
	}


	CalcFirstParentGlobalSRTReq(&retmat, GetParent());


	return retmat;
}

//SRT形式
void CBone::CalcFirstParentGlobalSRTReq(ChaMatrix* dstmat, CBone* srcbone)
{
	if (!srcbone || !dstmat) {
		return;
	}

	ChaMatrix firstSRT = srcbone->GetFirstSRT();
	*dstmat = *dstmat * firstSRT;//childmat * currentmat   (currentmat * parentmat)

	if (srcbone->GetParent()) {
		CalcFirstParentGlobalSRTReq(dstmat, srcbone->GetParent());
	}

}

//int CBone::SetLimitedLocalEul(int srcmotid, double srcframe, ChaVector3 srceul)
//{
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	CMotionPoint* curmp;
//	curmp = GetMotionPoint(srcmotid, roundingframe);
//	if (curmp) {
//		curmp->SetLimitedLocalEul(srceul);
//	}
//	else {
//		_ASSERT(0);
//	}
//	return 0;
//}

ChaVector3 CBone::GetLimitedLocalEul(int srcmotid, double srcframe)
{
	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		reteul = curmp->GetLimitedLocalEul();
		return reteul;
	}
	else {
		_ASSERT(0);
		return reteul;
	}
}

ChaVector3 CBone::GetUnlimitedLocalEul(int srcmotid, double srcframe)
{
	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);

	double roundingframe = (double)((int)(srcframe + 0.0001));

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, roundingframe);
	if (curmp) {
		reteul = curmp->GetLocalEul();
		return reteul;
	}
	else {
		_ASSERT(0);
		return reteul;
	}
}

//ChaVector3 CBone::CalcLocalEulAndSetLimitedEul(int srcmotid, double srcframe)
//{
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	ChaVector3 orgeul = ChaVector3(0.0f, 0.0f, 0.0f);
//	ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
//
//	orgeul = CalcLocalEulXYZ(-1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
//	int ismovable;
//	if ((g_limitdegflag == true) && (GetBtForce() == 0)) {//2023/01/28 物理シミュは　自前では制限しない
//		ismovable = ChkMovableEul(orgeul);
//	}
//	else {
//		ismovable = 1;
//	}
//	if (ismovable == 1) {
//		
//		//可動判定
//		
//		neweul = orgeul;
//	}
//	else {
//		
//		//不動判定
//
//		if (g_wallscrapingikflag == 1) {
//			//可動な軸の数値だけ動かす場合
//			neweul = LimitEul(orgeul, orgeul);//壁すりIK用
//		}
//		else {
//			//どの軸の数値も動かさない場合
//				//角度制限ありの場合の場合しか　ここを通らない
//
//
//			//2023/01/29 制限付けの場合分け
//
//			ChaVector3 befeul = GetLocalEul(srcmotid, 0.0, 0);
//
//			if ((roundingframe >= (0.0 - 0.0001)) && (roundingframe < (0.0 + 0.0001))) {
//				//0frame
//					//0frameはいじらない
//				neweul = GetLocalEul(srcmotid, 0.0, 0);
//			}
//			else if ((roundingframe >= (1.0 - 0.0001)) && (roundingframe < (1.0 + 0.0001))) {
//				//1frame
//				neweul = LimitEul(GetLocalEul(srcmotid, 1.0, 0), GetLocalEul(srcmotid, 1.0, 0));//１フレーム姿勢の壁すり
//			}
//			else {
//				double befframe;
//				befframe = roundingframe - 1.0;
//				//befeul = GetLimitedLocalEul(srcmotid, befframe);//2023/01/28　１フレーム前は計算されていると仮定
//				befeul = LimitEul(GetLocalEul(srcmotid, befframe, 0), GetLocalEul(srcmotid, befframe, 0));
//
//				if (g_underIKRot == false) {
//					neweul = LimitEul(befeul, GetLocalEul(srcmotid, befframe, 0));//壁すりIK用
//				}
//				else {
//					//IK中でg_wallscrapingikflag == 0の場合には　動かさない
//					//可動姿勢を探す
//					neweul = LimitEul(GetLocalEul(srcmotid, 1.0, 0), GetLocalEul(srcmotid, 1.0, 0));
//					while (befframe >= (2.0 - 0.0001)) {
//						befeul = GetLocalEul(srcmotid, befframe, 0);
//						if (ChkMovableEul(befeul) == 1) {
//							neweul = befeul;
//							break;
//						}
//						befframe -= 1.0;
//					}
//				}
//			}
//		}
//	}
//
//	SetLocalEul(srcmotid, roundingframe, neweul, 0);//!!!!!!!!!!!!
//	
//
//	return neweul;
//}

//ChaMatrix CBone::CalcLimitedWorldMat(int srcmotid, double srcframe, ChaVector3* dstneweul)
//{
//
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//	ChaMatrix retmat;
//	retmat.SetIdentity();
//	ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
//
//	if (!dstneweul) {
//		_ASSERT(0);
//		return retmat;
//	}
//
//	CMotionPoint* curmp;
//	curmp = GetMotionPoint(srcmotid, roundingframe);
//	if (!curmp) {
//		_ASSERT(0);
//		*dstneweul = neweul;
//		return retmat;
//	}
//
//
//
//	if (GetBtForce() == 0) {
//		neweul = CalcLocalEulAndSetLimitedEul(srcmotid, roundingframe);
//		retmat = CalcWorldMatFromEul(0, 1, neweul, srcmotid, roundingframe, 0);
//	}
//	else {
//		//2023/01/28
//		//btシミュボーンの場合
//		//物理のシミュにおける制限角度は　クランプではなく　行きすぎたら戻るやり方
//		//物理シミュをRecord機能でベイクしたモーションに対して　クランプすると　動きがカクカクになる
//		//よって　物理シミュボーンに対しては　自前では角度を制限しないことにする
//
//		//2023/01/29
//		//ただし　kinematicの親が角度制限を受けるので　それを反映しなくてはならない
//		ChaMatrix curwm;
//		curwm = curmp->GetWorldMat();//unlimited!!!!!
//		ChaMatrix curparentwm, curlimitedparentwm;
//		if (GetParent()) {
//			CMotionPoint* parentmp = GetParent()->GetMotionPoint(srcmotid, roundingframe);
//			if (parentmp) {
//				curparentwm = parentmp->GetWorldMat();
//				curlimitedparentwm = parentmp->GetLimitedWM();
//			}
//			else {
//				curparentwm.SetIdentity();
//				curlimitedparentwm.SetIdentity();
//			}
//		}
//		else {
//			curparentwm.SetIdentity();
//			curlimitedparentwm.SetIdentity();
//		}
//
//		ChaMatrix curlocalmat = curwm * ChaMatrixInv(curparentwm);
//		retmat = curlocalmat * curlimitedparentwm;
//		CQuaternion eulq;
//		eulq.RotationMatrix(curlocalmat);
//
//		ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//		double befframe;
//		befframe = roundingframe - 1.0;
//		if (befframe >= -0.0001) {
//			CMotionPoint* befmp;
//			befmp = GetMotionPoint(srcmotid, roundingframe);
//			if (befmp) {
//				befeul = befmp->GetLocalEul();//unlimited!!!!!
//			}
//		}
//
//		CQuaternion axisq;
//		axisq.RotationMatrix(GetNodeMat());
//		int isfirstbone = 0;
//		int isendbone = 0;
//		int notmodify180flag = 1;
//		if (g_underIKRot == false) {
//			if (roundingframe <= 1.01) {
//				//0フレームと１フレームは　180度ずれチェックをしない
//				notmodify180flag = 1;
//			}
//			else {
//				notmodify180flag = 0;
//			}
//		}
//		else {
//			//2023/01/26
//			//IKRot中は　０フレームも１フレームも　180度チェックをする
//			notmodify180flag = 0;
//			if (roundingframe <= 1.01) {
//				befeul = ChaVector3(0.0f, 0.0f, 0.0f);
//			}
//		}
//		eulq.Q2EulXYZusingQ(&axisq, befeul, &neweul, isfirstbone, isendbone, notmodify180flag);
//
//	}
//
//	*dstneweul = neweul;
//	return retmat;
//
//}

//ChaMatrix CBone::GetLimitedWorldMat(int srcmotid, double srcframe, ChaVector3* dstneweul, int callingstate)//default : dstneweul = 0, default : callingstate = 0
//{
//
//	//###########################
//	//時間に関する姿勢の補間無し
//	//###########################
//
//
//	//callingstate : 0->fullcalc, 1->bythread only current calc, 2->after thread, use result by threading calc, 3->get calclated parents wm
//
//
//	//###################################################################################################################
//	//この関数はオイラーグラフ用またはキー位置のモーションポイントに対しての処理用なので、時間に対する補間処理は必要ない
//	//###################################################################################################################
//
//	double roundingframe = (double)((int)(srcframe + 0.0001));
//
//	ChaMatrix retmat;
//	ChaMatrixIdentity(&retmat);
//	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
//		return retmat;
//	}
//
//	if (g_limitdegflag == true) {
//		//制限角度有り
//
//		ChaVector3 neweul;
//
//		CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
//		if (curmp) {
//			if (curmp->GetCalcLimitedWM() == 2) {
//				//計算済の場合 物理では無い場合
//				//GetCalcLimitedWM() : 前処理済１，後処理済２
//				retmat = curmp->GetLimitedWM();
//				if (dstneweul) {
//					//*dstneweul = curmp->GetLocalEul();
//					*dstneweul = curmp->GetLimitedLocalEul();//2023/01/29 limited !!!
//				}
//			}
//			else {
//				//計算済で無い場合
//				if (callingstate == 1) {
//					//前処理が出来てない(ボーン構造順に呼び出していないのでLocal計算が出来ない)ので何もしない。
//					//callingstate = 2での呼び出しに任せる。
//
//					//前処理をするかもしれないが　現状：curmp->SetCalcLimitedWM(1);だけする。
//					curmp->SetCalcLimitedWM(1);//前処理済のフラグ。後処理済は２
//				}
//				else {
//					//未計算の場合
//					retmat = CalcLimitedWorldMat(srcmotid, roundingframe, &neweul);
//
//					curmp->SetLimitedWM(retmat);
//					curmp->SetLimitedLocalEul(neweul);//2023/01/28
//					curmp->SetCalcLimitedWM(2);//処理済
//
//					if (dstneweul) {
//						*dstneweul = neweul;
//					}
//				}
//
//				//else if (callingstate == 3) {
//				//	//2022/08/22
//				//	//物理時　計算済のparent(この関数内においてはcurrent)のwmを参照　不具合エフェクト：指の先が遅れて動く
//				//	if (GetChild()) {
//				//		retmat = GetBtMat();
//				//	}
//				//	else {
//				//		//endjoint対策　この対策をしない場合、指の先が遅れて動く
//				//		if (GetParent()) {
//				//			retmat = GetParent()->GetBtMat();
//				//		}
//				//		else {
//				//			retmat = GetBtMat();
//				//		}
//				//	}
//				//	
//				//}
//				//else {
//				//	ChaMatrixIdentity(&retmat);
//				//}
//			}
//		}
//	}
//	else {
//		//制限角度無し
//		//CMotionPoint calcmp;
//		//int existflag = 0;
//		//CallF(CalcFBXMotion(srcmotid, roundingframe, &calcmp, &existflag), return retmat);//コメントにあるように時間に関する補間は必要ないのでコメントアウト2023/01/14
//		//retmat = calcmp.GetWorldMat();// **wmat;
//
//		CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
//		if (curmp) {
//			retmat = curmp->GetWorldMat();
//		}
//		else {
//			retmat.SetIdentity();
//		}
//
//		if (dstneweul) {
//			ChaVector3 cureul = CalcLocalEulXYZ(-1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
//			*dstneweul = cureul;
//		}
//	}
//
//
//	return retmat;
//}

//ChaMatrix CBone::GetCurrentLimitedWorldMat()
//{
//	ChaMatrix retmat;
//	ChaMatrixIdentity(&retmat);
//
//	if (!GetParModel()) {
//		return retmat;
//	}
//
//	int srcmotid;
//	double srcframe;
//	MOTINFO* curmi;
//	curmi = GetParModel()->GetCurMotInfo();
//	if (!curmi) {
//		return retmat;
//	}
//	srcmotid = curmi->motid;
//	srcframe = curmi->curframe;
//
//	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
//		return retmat;
//	}
//
//	if (g_limitdegflag == true) {
//		//制限角度有り
//
////####################################################################
////滑らかにするために、後でsrcframeと+1で２セット計算して補間計算するかもしれない
////####################################################################
//		//ChaVector3 neweul;
//		//neweul = CalcLocalEulAndSetLimitedEul(srcmotid, (double)((int)(srcframe + 0.1)));
//		//retmat = CalcWorldMatFromEul(0, 1, neweul, srcmotid, (double)((int)(srcframe + 0.1)), 0);
//
////#####################################
////滑らかにするために　フレーム補間
////#####################################
//		GetCalclatedLimitedWM(srcmotid, srcframe, &retmat);
//
//	}
//	else {
//		//制限角度無し
//		CMotionPoint calcmp;
//		int existflag = 0;
//		CallF(CalcFBXMotion(srcmotid, srcframe, &calcmp, &existflag), return retmat);
//		retmat = calcmp.GetWorldMat();// **wmat;
//	}
//
//
//	return retmat;
//}


ChaVector3 CBone::GetWorldPos(bool limitdegflag, int srcmotid, double srcframe)
{
	double roundingframe = (double)((int)(srcframe + 0.0001));


	ChaVector3 retpos = ChaVector3(0.0f, 0.0f, 0.0f);
	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
		return retpos;
	}

	ChaVector3 jointpos;
	jointpos = GetJointFPos();
	ChaMatrix newworldmat = GetWorldMat(limitdegflag, srcmotid, srcframe, 0);


	////2022/01/07 マニピュレータをキャラクター位置に出すため
	if (GetParModel()) {
		ChaMatrix wm = GetParModel()->GetWorldMat();//(親ボーンではなく)modelのworld
		newworldmat = newworldmat * wm;
	}

	ChaVector3TransformCoord(&retpos, &jointpos, &newworldmat);

	return retpos;
}


void CBone::SetCurMotID(int srcmotid)
{
	m_curmotid = srcmotid;
};


int CBone::CreateIndexedMotionPoint(int srcmotid, double animleng)
{
	//###############################################
	//2022/11/01 AddMotionPointAll内で行うように変更
	//###############################################

	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
		//_ASSERT(0);
		return 1;
	}
	if (animleng < 1.0) {
		_ASSERT(0);
		return 1;
	}



	std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap;
	itrvecmpmap = m_indexedmotionpoint.find(srcmotid);
	if (itrvecmpmap == m_indexedmotionpoint.end()) {
		std::vector<CMotionPoint*> newvecmp;
		m_indexedmotionpoint[srcmotid] = newvecmp;//STL 参照されていれば無くならない？？？

		std::map<int, vector<CMotionPoint*>>::iterator itrvecmpmap2;
		itrvecmpmap2 = m_indexedmotionpoint.find(srcmotid);
		if (itrvecmpmap2 == m_indexedmotionpoint.end()) {
			_ASSERT(0);
			return 1;
		}

		itrvecmpmap = itrvecmpmap2;
	}

	(itrvecmpmap->second).clear();//!!!!!!!!!!!!!!!
	

	CMotionPoint* curmp = m_motionkey[srcmotid - 1];
	if (curmp) {
		double mpframe;// = curmp->GetFrame();

		double frameno;
		for (frameno = 0.0; frameno < animleng; frameno += 1.0) {
			if (curmp) {
				mpframe = curmp->GetFrame();

				if ((mpframe >= 0.0) && (mpframe < animleng) &&
					(mpframe >= (frameno - 0.0001)) && (mpframe <= (frameno + 0.0001))) {
					(itrvecmpmap->second).push_back(curmp);
				}
				else {
					//for safety
					(itrvecmpmap->second).push_back(&m_dummymp);
				}
				curmp = curmp->GetNext();
			}
			else {
				(itrvecmpmap->second).push_back(&m_dummymp);
			}
		}
	}
	else {
		return 0;
	}


	m_initindexedmotionpoint[srcmotid] = true;

	return 0;


}

int CBone::ResetAngleLimit(int srcval)
{
	int newlimit;
	//newlimit = min(180, srcval);
	//newlimit = max(0, newlimit);
	newlimit = srcval;


	m_anglelimit.lower[0] = -newlimit;
	m_anglelimit.lower[1] = -newlimit;
	m_anglelimit.lower[2] = -newlimit;

	m_anglelimit.upper[0] = newlimit;
	m_anglelimit.upper[1] = newlimit;
	m_anglelimit.upper[2] = newlimit;

	return 0;
}

int CBone::AngleLimitReplace180to170()
{

	//+180, -180だけ170, -170に変換

	if (m_anglelimit.lower[0] == -180) {
		m_anglelimit.lower[0] = -170;
	}
	if (m_anglelimit.lower[1] == -180) {
		m_anglelimit.lower[1] = -170;
	}
	if (m_anglelimit.lower[2] == -180) {
		m_anglelimit.lower[2] = -170;
	}


	if (m_anglelimit.upper[0] == 180) {
		m_anglelimit.upper[0] = 170;
	}
	if (m_anglelimit.upper[1] == 180) {
		m_anglelimit.upper[1] = 170;
	}
	if (m_anglelimit.upper[2] == 180) {
		m_anglelimit.upper[2] = 170;
	}

	return 0;
}

int CBone::AdditiveCurrentToAngleLimit()
{
	//#########################################################################################################################
	//2022/12/17
	//カレントフレームだけの計算だと　0リセットの影響や制限の影響のあるbefeulを参照するので　オーバー180の計算がうまくいかない
	//よって全フレームのオイラー角を計算して　最大値と最小値を求めて　それを制限としてセットする
	//#########################################################################################################################

	if (m_parmodel) {
		MOTINFO* curmi = m_parmodel->GetCurMotInfo();
		if (curmi) {
			int curmotid = curmi->motid;
			double frameleng = curmi->frameleng;
			double curframe;

			ChaVector3 calceul;
			float cureul[3];
			float maxeul[3] = { FLT_MIN, FLT_MIN, FLT_MIN };//必ず更新されるようにMIN
			float mineul[3] = { FLT_MAX, FLT_MAX, FLT_MAX };//必ず更新されるようにMAX
			for (curframe = 1.0; curframe < frameleng; curframe += 1.0) {
				//calceul = CalcLocalEulXYZ(-1, curmotid, curframe, BEFEUL_BEFFRAME, 0);

				//2023/01/28
				//currentもparentも　制限角度無しで計算する必要有
				calceul = GetUnlimitedLocalEul(curmotid, curframe);

				cureul[0] = calceul.x;
				cureul[1] = calceul.y;
				cureul[2] = calceul.z;

				int axiskind;
				for (axiskind = 0; axiskind < 3; axiskind++) {
					if (cureul[axiskind] > maxeul[axiskind]) {
						maxeul[axiskind] = cureul[axiskind];
					}
					if (cureul[axiskind] < mineul[axiskind]) {
						mineul[axiskind] = cureul[axiskind];
					}
				}
			}

			int axiskind2;
			for (axiskind2 = 0; axiskind2 < 3; axiskind2++) {
				//m_anglelimit.upper[axiskind2] = (int)(maxeul[axiskind2] + 0.0001f);
				//m_anglelimit.lower[axiskind2] = (int)(mineul[axiskind2] + 0.0001f);

				//2023/01/28
				//実角度ギリギリをintに丸めて制限をかけると　可動部分が制限に引っ掛かることがあったので対応
				float tempmax, tempmin;
				tempmax = maxeul[axiskind2] + 2.0f;
				tempmin = mineul[axiskind2] - 2.0f;
				if (tempmax > 0.0f) {
					m_anglelimit.upper[axiskind2] = (int)(tempmax + 0.0001f);
				}
				else {
					m_anglelimit.upper[axiskind2] = (int)(tempmax - 0.0001f);
				}
				if (tempmin > 0.0f) {
					m_anglelimit.lower[axiskind2] = (int)(tempmin + 0.0001f);
				}
				else {
					m_anglelimit.lower[axiskind2] = (int)(tempmin - 0.0001f);
				}
			}

		}
	}


	//if (m_parmodel) {
	//	MOTINFO* curmi = m_parmodel->GetCurMotInfo();
	//	if (curmi) {
	//		int curmotid = curmi->motid;
	//		int curframe = curmi->curframe;
	//		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//		cureul = CalcLocalEulXYZ(-1, m_curmotid, curframe, BEFEUL_BEFFRAME, 0);
	//		AdditiveToAngleLimit(cureul);
	//	}
	//	else {
	//		_ASSERT(0);
	//	}
	//}
	//else {
	//	_ASSERT(0);
	//}


	return 0;
}

int CBone::AdditiveAllMotionsToAngleLimit()
{
	if (m_parmodel) {

		float maxeul[3] = { FLT_MIN, FLT_MIN, FLT_MIN };//必ず更新されるようにMIN
		float mineul[3] = { FLT_MAX, FLT_MAX, FLT_MAX };//必ず更新されるようにMAX

		int motionnum = m_parmodel->GetMotInfoSize();
		if (motionnum >= 1) {
			std::map<int, MOTINFO*>::iterator itrmi;
			for (itrmi = m_parmodel->GetMotInfoBegin(); itrmi != m_parmodel->GetMotInfoEnd(); itrmi++) {
				MOTINFO* curmi = itrmi->second;
				if (curmi) {
					int curmotid = curmi->motid;
					double frameleng = curmi->frameleng;
					double curframe;

					ChaVector3 calceul;
					float cureul[3];

					for (curframe = 1.0; curframe < frameleng; curframe += 1.0) {
						//calceul = CalcLocalEulXYZ(-1, curmotid, curframe, BEFEUL_BEFFRAME, 0);

						//2023/01/28
						//currentもparentも　制限角度無しで計算する必要有
						calceul = GetUnlimitedLocalEul(curmotid, curframe);

						cureul[0] = calceul.x;
						cureul[1] = calceul.y;
						cureul[2] = calceul.z;

						int axiskind;
						for (axiskind = 0; axiskind < 3; axiskind++) {
							if (cureul[axiskind] > maxeul[axiskind]) {
								maxeul[axiskind] = cureul[axiskind];
							}
							if (cureul[axiskind] < mineul[axiskind]) {
								mineul[axiskind] = cureul[axiskind];
							}
						}
					}

				}
			}

			int axiskind2;
			for (axiskind2 = 0; axiskind2 < 3; axiskind2++) {
				//m_anglelimit.upper[axiskind2] = (int)(maxeul[axiskind2] + 0.0001f);
				//m_anglelimit.lower[axiskind2] = (int)(mineul[axiskind2] + 0.0001f);

				//2023/01/28
				//実角度ギリギリをintに丸めて制限をかけると　可動部分が制限に引っ掛かることがあったので対応
				float tempmax, tempmin;
				tempmax = maxeul[axiskind2] + 2.0f;
				tempmin = mineul[axiskind2] - 2.0f;
				if (tempmax > 0.0f) {
					m_anglelimit.upper[axiskind2] = (int)(tempmax + 0.0001f);
				}
				else {
					m_anglelimit.upper[axiskind2] = (int)(tempmax - 0.0001f);
				}
				if (tempmin > 0.0f) {
					m_anglelimit.lower[axiskind2] = (int)(tempmin + 0.0001f);
				}
				else {
					m_anglelimit.lower[axiskind2] = (int)(tempmin - 0.0001f);
				}
			}
		}
	}


	//if (m_parmodel) {
	//	MOTINFO* curmi = m_parmodel->GetCurMotInfo();
	//	if (curmi) {
	//		int curmotid = curmi->motid;
	//		int curframe = curmi->curframe;
	//		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//		cureul = CalcLocalEulXYZ(-1, m_curmotid, curframe, BEFEUL_BEFFRAME, 0);
	//		AdditiveToAngleLimit(cureul);
	//	}
	//	else {
	//		_ASSERT(0);
	//	}
	//}
	//else {
	//	_ASSERT(0);
	//}


	return 0;
}


//int CBone::AdditiveToAngleLimit(ChaVector3 cureul)
//{
//
//	if ( m_anglelimit.lower[AXIS_X] > (int)cureul.x) {
//		m_anglelimit.lower[AXIS_X] = (int)cureul.x;
//	}
//	if (m_anglelimit.lower[AXIS_Y] > (int)cureul.y) {
//		m_anglelimit.lower[AXIS_Y] = (int)cureul.y;
//	}
//	if (m_anglelimit.lower[AXIS_Z] > (int)cureul.z) {
//		m_anglelimit.lower[AXIS_Z] = (int)cureul.z;
//	}
//
//	if ((int)cureul.x > m_anglelimit.upper[AXIS_X]) {
//		m_anglelimit.upper[AXIS_X] = (int)cureul.x;
//	}
//	if ((int)cureul.y > m_anglelimit.upper[AXIS_Y]) {
//		m_anglelimit.upper[AXIS_Y] = (int)cureul.y;
//	}
//	if ((int)cureul.z > m_anglelimit.upper[AXIS_Z]) {
//		m_anglelimit.upper[AXIS_Z] = (int)cureul.z;
//	}
//
//
//	//#################################################################################################
//	//2022/12/06
//	//floatの角度にintの制限をかける際　差分が遊びより小さい場合に　モーションがぶるぶるすることがある
//	//遊びよりも大きな値分　絶対値の内側に折り込む
//	//#################################################################################################
//	//if ( (m_anglelimit.lower[0] - (int)cureul.x) > EULLIMITPLAY) {
//	//	m_anglelimit.lower[0] = cureul.x + EULLIMITPLAY;
//	//}
//	//if ((m_anglelimit.lower[1] - (int)cureul.y) > EULLIMITPLAY) {
//	//	m_anglelimit.lower[1] = cureul.y + EULLIMITPLAY;
//	//}
//	//if ((m_anglelimit.lower[2] - (int)cureul.z) > EULLIMITPLAY) {
//	//	m_anglelimit.lower[2] = cureul.z + EULLIMITPLAY;
//	//}
//	//if (((int)cureul.x - m_anglelimit.upper[0]) > EULLIMITPLAY) {
//	//	m_anglelimit.upper[0] = cureul.x - EULLIMITPLAY;
//	//}
//	//if (((int)cureul.y - m_anglelimit.upper[1]) > EULLIMITPLAY) {
//	//	m_anglelimit.upper[1] = cureul.y - EULLIMITPLAY;
//	//}
//	//if (((int)cureul.z - m_anglelimit.upper[2]) > EULLIMITPLAY) {
//	//	m_anglelimit.upper[2] = cureul.z - EULLIMITPLAY;
//	//}
//
//
//	SwapAngleLimitUpperLowerIfRev();
//
//
//
//	//#######################################################################################
//	//2022/12/06 Comment out
//	//floatの角度にintの制限をかける際　差分が遊びより小さい場合に　モーションがぶるぶるする 
//	//#######################################################################################
//	//if ((int)cureul.x < m_anglelimit.lower[0]) {
//	//	m_anglelimit.lower[0] = cureul.x;
//	//}
//	//if ((int)cureul.y < m_anglelimit.lower[1]) {
//	//	m_anglelimit.lower[1] = cureul.y;
//	//}
//	//if ((int)cureul.z < m_anglelimit.lower[2]) {
//	//	m_anglelimit.lower[2] = cureul.z;
//	//}
//
//	//if ((int)cureul.x > m_anglelimit.upper[0]) {
//	//	m_anglelimit.upper[0] = cureul.x;
//	//}
//	//if ((int)cureul.y > m_anglelimit.upper[1]) {
//	//	m_anglelimit.upper[1] = cureul.y;
//	//}
//	//if ((int)cureul.z > m_anglelimit.upper[2]) {
//	//	m_anglelimit.upper[2] = cureul.z;
//	//}
//
//	return 0;
//}

int CBone::GetFBXAnim(FbxNode* pNode, int animno, int motid, double animleng, bool callingbythread)
{
	if (GetGetAnimFlag() == 0) {
		SetGetAnimFlag(1);
	}

	FbxTime fbxtime;
	fbxtime.SetSecondDouble(0.0);
	FbxTime difftime;
	difftime.SetSecondDouble(1.0 / 30);
	double framecnt;
	//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
	//for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//関数呼び出し時にanimleng - 1している


	FbxAMatrix correctscalemat;
	correctscalemat.SetIdentity();
	FbxAMatrix currentmat;
	currentmat.SetIdentity();
	FbxAMatrix parentmat;
	parentmat.SetIdentity();
	//const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eDestinationPivot);
	//const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eDestinationPivot);
	//const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eDestinationPivot);
	//const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);
	//const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
	//const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot, true, true);
	//FbxAMatrix lSRT = pNode->EvaluateLocalTransform(fbxtime, FbxNode::eSourcePivot, true, true);
	//FbxAMatrix lGlobalSRT = pNode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot, true, true);

	if (pNode) {
		//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
		for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//2022/10/21 : 最終フレームにモーションポイントが無い問題対応

			FbxAMatrix lGlobalSRT;

			//#####  2022/11/01  ################################################################################################
			//サブスレッド１つだけで計算することにした(CriticalSection回数が多すぎて遅くなる)ので　CriticalSectionコメントアウト
			//スレッド数(LOADFBXANIMTHREAD)を１以外にする場合には　CriticalSection必須
			//###################################################################################################################
			//EnterCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################
			lGlobalSRT = pNode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot);
			//LeaveCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################

			ChaMatrix chaGlobalSRT;
			chaGlobalSRT = ChaMatrixFromFbxAMatrix(lGlobalSRT);

			////##############
			////Add MotionPoint
			////##############
			ChaMatrix localmat;
			ChaMatrixIdentity(&localmat);
			ChaMatrix globalmat;
			ChaMatrixIdentity(&globalmat);

			CMotionPoint* curmp = 0;
			int existflag = 0;
			//curmp = curbone->AddMotionPoint(motid, framecnt, &existflag);
			curmp = GetMotionPoint(motid, framecnt);
			if (!curmp) {
				//_ASSERT(0);
				//return 1;
				curmp = AddMotionPoint(motid, framecnt, &existflag);
				if (!curmp) {
					_ASSERT(0);
					return 1;
				}
			}

			//###############
			//calc globalmat
			//###############
			globalmat = (ChaMatrixInv(GetNodeMat()) * chaGlobalSRT);
			//globalmat = (ChaMatrixInv(curbone->GetNodeMat()) * chaGlobalSRT);
			curmp->SetWorldMat(globalmat);//anglelimit無し
			curmp->SetLimitedWM(globalmat);//初期値はそのまま

			//##############################################################################
			//SetCalcLimitedWM(2)は　CModel::PostLoadFbxで　LimitedLocalEulをセットしてから
			//##############################################################################


			//##########
			//FirstMot
			//##########
			if ((animno == 0) && (framecnt == 0.0)) {
				//SetFirstMat(globalmat);
				SetFirstMat(chaGlobalSRT);
			}


			fbxtime = fbxtime + difftime;
		}
	}

	//Sleep(0);

	//#####################################################################################################
	//念のために　ジョイントの向きを強制リセットしていたころの　ソースをコメントアウトして残す　2022/10/31
	//#####################################################################################################
	//if ((bvhflag == 0) &&
	//	GetParModel() && GetParModel()->GetHasBindPose()) {
	//
	//	//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
	//	for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//2022/10/21 : 最終フレームにモーションポイントが無い問題対応
	//
	//		for (bonecount = 0; bonecount < srcbonenum; bonecount++) {
	//			CBone* curbone = *(bonelist + bonecount);
	//			FbxNode* pNode = *(nodelist + bonecount);
	//			if (curbone && pNode) {
	//				FbxAMatrix lGlobalSRT;
	//
	//				EnterCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################
	//				const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot);
	//				const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot);
	//				const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot);
	//				LeaveCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################
	//
	//				ChaVector3 chatra = ChaVector3((float)lT2[0], (float)lT2[1], (float)lT2[2]);
	//				ChaVector3 chaeul = ChaVector3((float)lR2[0], (float)lR2[1], (float)lR2[2]);
	//				ChaVector3 chascale = ChaVector3((float)lS2[0], (float)lS2[1], (float)lS2[2]);
	//
	//				//####################
	//				//calc joint position
	//				//####################
	//				ChaVector3 jointpos;
	//				jointpos = curbone->GetJointFPos();
	//				ChaVector3 parentjointpos;
	//				if (curbone->GetParent()) {
	//					parentjointpos = curbone->GetParent()->GetJointFPos();
	//				}
	//				else {
	//					parentjointpos = ChaVector3(0.0f, 0.0f, 0.0f);
	//				}
	//
	//				//##############
	//				//calc rotation
	//				//##############
	//				CQuaternion chaq;
	//				chaq.SetRotationXYZ(0, chaeul);
	//				ChaMatrix charotmat;
	//				charotmat = chaq.MakeRotMatX();
	//
	//				ChaMatrix befrotmat, aftrotmat;
	//				ChaMatrixTranslation(&befrotmat, -jointpos.x, -jointpos.y, -jointpos.z);
	//				ChaMatrixTranslation(&aftrotmat, jointpos.x, jointpos.y, jointpos.z);
	//
	//				//#################
	//				//calc translation
	//				//#################
	//				ChaMatrix chatramat;
	//				ChaMatrixIdentity(&chatramat);
	//				ChaMatrixTranslation(&chatramat, chatra.x - jointpos.x + parentjointpos.x, chatra.y - jointpos.y + parentjointpos.y, chatra.z - jointpos.z + parentjointpos.z);
	//
	//				//##############
	//				//calc scalling
	//				//##############
	//				ChaMatrix chascalemat;
	//				ChaMatrixScaling(&chascalemat, chascale.x, chascale.y, chascale.z);
	//
	//				//Set Local frame0
	//				if (framecnt == 0.0) {
	//					curbone->SetLocalR0(chaq);
	//					curbone->SetLocalT0(chatramat);
	//					curbone->SetLocalS0(chascalemat);
	//					//curbone->SetFirstSRT(chaSRT);
	//				}
	//
	//
	//				//##############
	//				//calc localmat
	//				//##############
	//				ChaMatrix localmat;
	//				ChaMatrixIdentity(&localmat);
	//				ChaMatrix globalmat;
	//				ChaMatrixIdentity(&globalmat);
	//
	//				CMotionPoint* curmp = 0;
	//				int existflag = 0;
	//				curmp = curbone->AddMotionPoint(motid, framecnt, &existflag);
	//				if (!curmp) {
	//					_ASSERT(0);
	//					return 1;
	//				}
	//
	//				localmat = befrotmat * chascalemat * charotmat * aftrotmat * chatramat;
	//
	//				//#############
	//				//set localmat
	//				//#############
	//				curmp->SetLocalMat(localmat);//anglelimit無し
	//
	//			}
	//		}
	//		fbxtime = fbxtime + difftime;
	//	}
	//}


	return 0;
}


//int CBone::GetFBXAnim(int bvhflag, CBone** bonelist, FbxNode** nodelist, int srcbonenum, int animno, int motid, double animleng, bool callingbythread)
//{
//
//	//if (curbone && !curbone->GetGetAnimFlag()) {
//	//	curbone->SetGetAnimFlag(1);
//	int bonecount;
//	for (bonecount = 0; bonecount < srcbonenum; bonecount++) {
//		CBone* curbone = *(bonelist + bonecount);
//		if (curbone && !curbone->GetGetAnimFlag()) {
//			curbone->SetGetAnimFlag(1);
//		}
//	}
//
//
//	FbxTime fbxtime;
//	fbxtime.SetSecondDouble(0.0);
//	FbxTime difftime;
//	difftime.SetSecondDouble(1.0 / 30);
//	double framecnt;
//	//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
//	//for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//関数呼び出し時にanimleng - 1している
//
//
//	FbxAMatrix correctscalemat;
//	correctscalemat.SetIdentity();
//	FbxAMatrix currentmat;
//	currentmat.SetIdentity();
//	FbxAMatrix parentmat;
//	parentmat.SetIdentity();
//	//const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eDestinationPivot);
//	//const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eDestinationPivot);
//	//const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eDestinationPivot);
//	//const FbxVector4 lT2 = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);
//	//const FbxVector4 lR2 = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
//	//const FbxVector4 lS2 = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot, true, true);
//	//FbxAMatrix lSRT = pNode->EvaluateLocalTransform(fbxtime, FbxNode::eSourcePivot, true, true);
//	//FbxAMatrix lGlobalSRT = pNode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot, true, true);
//
//
//	//2022/11/01 boneloopをtimeloopより外側にして高速化
//	for (bonecount = 0; bonecount < srcbonenum; bonecount++) {
//		CBone* curbone = *(bonelist + bonecount);
//		FbxNode* pNode = *(nodelist + bonecount);
//
//		fbxtime.SetSecondDouble(0.0);
//
//		if (curbone && pNode) {
//			//for (framecnt = 0.0; framecnt < (animleng - 1); framecnt += 1.0) {
//			for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {//2022/10/21 : 最終フレームにモーションポイントが無い問題対応
//
//				FbxAMatrix lGlobalSRT;
//
//				EnterCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################
//				lGlobalSRT = pNode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot);
//				LeaveCriticalSection(&(GetParModel()->m_CritSection_Node));//#######################
//
//				ChaMatrix chaGlobalSRT;
//				chaGlobalSRT = ChaMatrixFromFbxAMatrix(lGlobalSRT);
//
//				////##############
//				////Add MotionPoint
//				////##############
//				ChaMatrix localmat;
//				ChaMatrixIdentity(&localmat);
//				ChaMatrix globalmat;
//				ChaMatrixIdentity(&globalmat);
//
//				CMotionPoint* curmp = 0;
//				int existflag = 0;
//				//curmp = curbone->AddMotionPoint(motid, framecnt, &existflag);
//				curmp = curbone->GetMotionPoint(motid, framecnt);
//				if (!curmp) {
//					//_ASSERT(0);
//					//return 1;
//					curmp = curbone->AddMotionPoint(motid, framecnt, &existflag);
//					if (!curmp) {
//						_ASSERT(0);
//						return 1;
//					}
//				}
//
//				//###############
//				//calc globalmat
//				//###############
//				globalmat = (ChaMatrixInv(curbone->GetNodeMat()) * chaGlobalSRT);
//				//globalmat = (ChaMatrixInv(curbone->GetNodeMat()) * chaGlobalSRT);
//				curmp->SetWorldMat(globalmat);//anglelimit無し
//
//				//##########
//				//FirstMot
//				//##########
//				if ((animno == 0) && (framecnt == 0.0)) {
//					curbone->SetFirstMat(globalmat);
//				}
//
//
//				fbxtime = fbxtime + difftime;
//			}
//		}
//	}
//
//	Sleep(0);
//
//	return 0;
//}


int CBone::InitMP(bool limitdegflag, int srcmotid, double srcframe)
{
	//###########################################################
	//InitMP 初期姿勢。リターゲットの初期姿勢に関わる。 
	//最初のモーション(firstmotid)の worldmat(firstanim)で初期化
	//###########################################################

	if (!GetParModel()) {
		return 0;
	}

	double roundingframe = (double)((int)(srcframe + 0.0001));

	//この関数は処理に時間が掛かる
	//CModel読み込み中で　読み込み中のモーション数が０以外の場合には　InitMPする必要は無い(モーションの値で上書きする)ので　リターンする
	//
	//2022/11/08
	//ただし　RootまたはReferenceが含まれる名前のボーンは　読み込み時に追加することがあるので　RootとReferenceについてはここではリターンしない
	if ((strstr(GetBoneName(), "Root") == 0) && (strstr(GetBoneName(), "Reference") == 0) &&
		(GetParModel()->GetLoadedFlag() == false) && (GetParModel()->GetLoadingMotionCount() > 0)) {//2022/10/20
		return 0;
	}

	//firstmpが無い場合のダミーの初期化モーションポイント
	//初期化されたworldmatがあれば良い
	CMotionPoint initmp;
	initmp.InitParams();


	//１つ目のモーションを削除する場合もあるので　motid = 1決め打ちは出来ない　2022/09/13
	//CMotionPoint* firstmp = GetMotionPoint(1, 0.0);//motid == 1は１つ目のモーション

	int firstmotid = 1;
	MOTINFO* firstmi = GetParModel()->GetFirstValidMotInfo();//１つ目のモーションを削除済の場合に対応
	if (!firstmi) {
		//MotionPointが無い場合にもいても　想定している使い方として　MOTINFOはAddされた状態でRetargetは呼ばれる
		//よってここを通る場合は　想定外エラー
		_ASSERT(0);
		return 1;
	}
	else {
		firstmotid = firstmi->motid;
	}
	

	CMotionPoint* firstmp = 0;
	if ((GetParModel()->GetLoadedFlag() == false) && (GetParModel()->GetLoadingMotionCount() <= 0)) {
		//Motionが１つも無いfbx読み込みのフォロー
		//読み込み中で　fbxにモーションが無い場合　モーションポイントを作成する　それ以外の場合で　モーションポイントが無い場合はエラー
		firstmp = &initmp;
	}
	else {
		firstmp = GetMotionPoint(firstmotid, 0.0);
	}

	if (!firstmp && ((strstr(GetBoneName(), "Root") != 0) || (strstr(GetBoneName(), "Reference") != 0))) {
		//2022/11/08
		//RootまたはReferenceが含まれる名前のボーンは　読み込み時に追加することがある
		//RootとReferenceボーンの内　モーションポイントが無い場合についても　ここで対応
		firstmp = &initmp;
	}


	if (firstmp) {
		ChaMatrix firstanim = firstmp->GetWorldMat();
		//SetFirstMat(firstanim);//リターゲット時のbvhbone->GetFirstMatで効果

		CMotionPoint* curmp = GetMotionPoint(srcmotid, roundingframe);
		if (!curmp) {
			int existflag = 0;
			curmp = AddMotionPoint(srcmotid, roundingframe, &existflag);
		}
		if (curmp) {

			//SetWorldMat(srcmotid, roundingframe, firstanim, curmp);
			curmp->SetWorldMat(firstanim);
			curmp->SetLimitedWM(firstanim);

			//SetInitMat(xmat);
			////オイラー角初期化
			//ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
			//int paraxsiflag = 1;
			//cureul = CalcLocalEulXYZ(paraxsiflag, 1, 0.0, BEFEUL_ZERO);


			//１つ目のモーションを削除する場合もあるので　motid = 1決め打ちは出来ない　2022/09/13
			//ChaVector3 cureul = GetLocalEul(firstmotid, 0.0, 0);//motid == 1は１つ目のモーション
			//SetLocalEul(srcmotid, roundingframe, cureul, curmp);
			ChaVector3 cureul = firstmp->GetLocalEul();
			curmp->SetLocalEul(cureul);
			curmp->SetLimitedLocalEul(cureul);
			//if (limitdegflag == true) {
				curmp->SetCalcLimitedWM(2);
			//}


			//2023/02/11
			//GetFbxAnimのif((animno == 0) && (srcframe == 0.0))を通らなかったRootジョイント用の初期化
			if ((srcmotid == firstmotid) && (roundingframe == 0.0)) {
				ChaMatrix firstmat;
				firstmat = GetNodeMat() * firstanim;
				SetFirstMat(firstmat);
			}

		}
	}
	else {
		_ASSERT(0);
		return 1;
	}


	////###################################################################################		
	////InitMP 初期姿勢。２つ目以降のモーションの初期姿勢。リターゲットの初期姿勢に関わる。
	////###################################################################################
	//if (newmp && (srcmotid != 1)) {
	//	ChaMatrix xmat = GetFirstMat();
	//	newmp->SetWorldMat(xmat);
	//	//SetInitMat(xmat);
	//	////オイラー角初期化
	//	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//	int paraxsiflag = 1;
	//	cureul = CalcLocalEulXYZ(paraxsiflag, srcmotid, roundingframe, BEFEUL_ZERO);
	//	SetLocalEul(srcmotid, roundingframe, cureul);
	//}



	//ChaMatrix parfirstmat, invparfirstmat;
	//ChaMatrixIdentity(&parfirstmat);
	//ChaMatrixIdentity(&invparfirstmat);
	//if (parentbone) {
	//	double zeroframe = 0.0;
	//	int existz = 0;
	//	CMotionPoint* parmp = parentbone->AddMotionPoint(motid, zeroframe, &existz);
	//	if (existz && parmp) {
	//		parfirstmat = parmp->GetWorldMat();//!!!!!!!!!!!!!! この時点ではm_matWorldが掛かっていないから後で修正必要かも？？
	//		ChaMatrixInverse(&invparfirstmat, NULL, &parfirstmat);
	//	}
	//	else {
	//		ChaMatrixIdentity(&parfirstmat);
	//		ChaMatrixIdentity(&invparfirstmat);
	//	}
	//}

	//double framecnt;
	//for (framecnt = 0.0; framecnt < animleng; framecnt += 1.0) {
	//	double frame = framecnt;

	//	ChaMatrix mvmat;
	//	ChaMatrixIdentity(&mvmat);

	//	CMotionPoint* pcurmp = 0;
	//	bool onaddmotion = true;
	//	pcurmp = curbone->GetMotionPoint(motid, frame, onaddmotion);
	//	if (!pcurmp) {
	//		int exist2 = 0;
	//		CMotionPoint* newmp = curbone->AddMotionPoint(motid, frame, &exist2);
	//		if (!newmp) {
	//			_ASSERT(0);
	//			return;
	//		}

	//		if (parentbone) {
	//			int exist3 = 0;
	//			CMotionPoint* parmp = parentbone->AddMotionPoint(motid, frame, &exist3);
	//			ChaMatrix tmpmat = parentbone->GetInvFirstMat() * parmp->GetWorldMat();//!!!!!!!!!!!!!!!!!! endjointはこれでうまく行くが、floatと分岐が不動になる。
	//			//newmp->SetBefWorldMat(tmpmat);
	//			newmp->SetWorldMat(tmpmat);//anglelimit無し

	//			//オイラー角初期化
	//			ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	//			int paraxiskind = -1;//2021/11/18
	//			//int isfirstbone = 0;
	//			cureul = curbone->CalcLocalEulXYZ(paraxiskind, motid, (double)framecnt, BEFEUL_ZERO);
	//			curbone->SetLocalEul(motid, (double)framecnt, cureul);

	//		}
	//	}
	//}


	return 0;
}


bool CBone::IsHipsBone()
{
	if (strlen(GetBoneName()) <= 0) {
		return false;
	}

	const char strpat[20] = "Hips";
	const char* hipsptr = strstr(GetBoneName(), strpat);//strstr
	if (hipsptr) {
		return true;
	}
	else {
		const char strpat2[20] = "Hip";
		if (strcmp(GetBoneName(), strpat2) == 0) {//strcmp
			return true;
		}
		else {
			const char strpat3[20] = "Hip_Joint";
			if (strcmp(GetBoneName(), strpat3) == 0) {//strcmp
				return true;
			}
		}
	}

	return false;
}


int CBone::SwapCurrentMotionPoint()
{
	m_curmp.CopyMP(&m_calccurmp);
	return 0;
}

void CBone::SaveFbxNodePosture(FbxNode* pNode)
{
	if (pNode) {
		//m_fbxLclPos = pNode->LclTranslation.Get();
		//m_fbxLclRot = pNode->LclRotation.Get();
		//m_fbxLclScl = pNode->LclScaling.Get();

		FbxTime fbxtime;
		fbxtime.SetSecondDouble(0.0);
		m_fbxLclPos = pNode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot);
		m_fbxLclRot = pNode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot);
		m_fbxLclScl = pNode->EvaluateLocalScaling(fbxtime, FbxNode::eSourcePivot);

		m_fbxRotOff = pNode->GetRotationOffset(FbxNode::eSourcePivot);
		m_fbxRotPiv = pNode->GetRotationPivot(FbxNode::eSourcePivot);
		m_fbxPreRot = pNode->GetPreRotation(FbxNode::eSourcePivot);
		m_fbxPostRot = pNode->GetPostRotation(FbxNode::eSourcePivot);
		m_fbxSclOff = pNode->GetScalingOffset(FbxNode::eSourcePivot);
		m_fbxSclPiv = pNode->GetScalingPivot(FbxNode::eSourcePivot);
		m_fbxrotationActive = pNode->GetRotationActive();
	}
}

//2023/02/16
//fbxの初期姿勢のジョイントの向きを書き出すために追加
void CBone::RestoreFbxNodePosture(FbxNode* pNode)
{

	if (pNode) {

		pNode->LclTranslation.Set(m_fbxLclPos);
		pNode->LclRotation.Set(m_fbxLclRot);
		pNode->LclScaling.Set(m_fbxLclScl);

		pNode->SetRotationOffset(FbxNode::eSourcePivot, m_fbxRotOff);
		//pNode->SetRotationOffset(FbxNode::eDestinationPivot, m_fbxRotOff);

		pNode->SetRotationPivot(FbxNode::eSourcePivot, m_fbxRotPiv);
		//pNode->SetRotationPivot(FbxNode::eDestinationPivot, m_fbxRotPiv);

		pNode->SetPreRotation(FbxNode::eSourcePivot, m_fbxPreRot);
		//pNode->SetPreRotation(FbxNode::eDestinationPivot, m_fbxPreRot);

		pNode->SetPostRotation(FbxNode::eSourcePivot, m_fbxPostRot);
		//pNode->SetPostRotation(FbxNode::eDestinationPivot, m_fbxPostRot);

		pNode->SetScalingOffset(FbxNode::eSourcePivot, m_fbxSclOff);
		//pNode->SetScalingOffset(FbxNode::eDestinationPivot, m_fbxSclOff);

		pNode->SetScalingPivot(FbxNode::eSourcePivot, m_fbxSclPiv);
		//pNode->SetScalingPivot(FbxNode::eDestinationPivot, m_fbxSclPiv);

		pNode->SetRotationActive(m_fbxrotationActive);
	}
}


void CBone::SetIKTargetFlag(bool srcflag)
{
	m_iktargetflag = srcflag;

	if (srcflag == true) {
		if (GetParModel()) {
			MOTINFO* curmi = GetParModel()->GetCurMotInfo();
			if (curmi) {
				CMotionPoint curmp = GetCurMp();
				ChaMatrix curwm = GetWorldMat(g_limitdegflag, curmi->motid, curmi->curframe, &curmp);
				ChaVector3 jointpos0, jointpos1;
				jointpos0 = GetJointFPos();
				ChaVector3TransformCoord(&jointpos1, &jointpos0, &curwm);
				SetIKTargetPos(jointpos1);
			}
			else {
				SetIKTargetPos(ChaVector3(0.0f, 0.0f, 0.0f));
			}
		}
		else {
			SetIKTargetPos(ChaVector3(0.0f, 0.0f, 0.0f));
		}
	}
	else {
		SetIKTargetPos(ChaVector3(0.0f, 0.0f, 0.0f));
	}

}
bool CBone::GetIKTargetFlag()
{
	return m_iktargetflag;
}

