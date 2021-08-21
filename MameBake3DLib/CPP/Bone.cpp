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

using namespace std;
using namespace OrgWinGUI;


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
	ZeroMemory(dstcr, sizeof(CUSTOMRIG));


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
		return 0;
	}
	if ((srccr.rigno < 0) || (srccr.rigno >= MAXRIGNUM)) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。rigno : %d", srccr.rigno);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
		return 0;
	}
	if ((srccr.elemnum < 1) || (srccr.elemnum > MAXRIGELEMNUM)) {
		WCHAR strerr[256];
		swprintf_s(strerr, 256, L"エラー。elemnum : %d", srccr.elemnum);
		::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
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
				return 0;
			}
		}
		else {
			WCHAR strerr[256];
			swprintf_s(strerr, 256, L"エラー。ownerbone NULL");
			::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
			return 0;
		}
	}
	else {
		CBone* chkbone = srcmodel->GetBoneByID(srcrigelem.boneno);
		if (!chkbone) {
			WCHAR strerr[256];
			swprintf_s(strerr, 256, L"エラー。boneno : %d", srcrigelem.boneno);
			::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
			return 0;
		}
		int uvno;
		for (uvno = 0; uvno < 2; uvno++) {
			RIGTRANS currigtrans = srcrigelem.transuv[uvno];
			if ((currigtrans.axiskind < AXIS_X) || (currigtrans.axiskind > AXIS_Z)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : axiskind : %d", uvno, currigtrans.axiskind);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				return 0;
			}
			if ((currigtrans.applyrate < -100.0f) || (currigtrans.applyrate > 100.0f)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : applyrate : %f", uvno, currigtrans.applyrate);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
				return 0;
			}
			if ((currigtrans.enable != 0) && (currigtrans.enable != 1)) {
				WCHAR strerr[256];
				swprintf_s(strerr, 256, L"エラー。UV %d : enable : %d", uvno, currigtrans.enable);
				::MessageBox(NULL, strerr, L"入力エラー", MB_OK);
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

	m_curmp.InitParams();
	m_axisq.InitParams();

	vecLocalTransform.clear();
	veclClusterGlobalCurrentPosition.clear();

	m_motionkey.clear();
	m_motionkey[0] = 0;


	m_tmpkinematic = false;
	//m_curmotid = -1;
	m_curmotid = 0;//有効なidは１から
	m_excludemv = 0;
	m_mass0 = 0;
	m_posconstraint = 0;
	ZeroMemory(m_coldisp, sizeof(CModel*)* COL_MAX);

	ChaMatrixIdentity(&m_tmpsymmat);

	ChaMatrixIdentity(&m_btmat);
	ChaMatrixIdentity(&m_befbtmat);
	m_setbtflag = 0;


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
	ChaMatrixIdentity( &m_startmat2 );
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

	ChaMatrixIdentity( &m_nodemat );

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

	int motcnt;
	for (motcnt = 0; motcnt < (MAXMOTIONNUM + 1); motcnt++) {
		lClusterMode[motcnt] = FbxCluster::ELinkMode::eNormalize;

		lReferenceGlobalInitPosition[motcnt].SetIdentity();
		lReferenceGlobalCurrentPosition[motcnt].SetIdentity();
		lAssociateGlobalInitPosition[motcnt].SetIdentity();
		lAssociateGlobalCurrentPosition[motcnt].SetIdentity();
		lClusterGlobalInitPosition[motcnt].SetIdentity();
		//lClusterGlobalCurrentPosition[motcnt].SetIdentity();
		lReferenceGeometry[motcnt].SetIdentity();
		lAssociateGeometry[motcnt].SetIdentity();
		lClusterGeometry[motcnt].SetIdentity();
		lClusterRelativeInitPosition[motcnt].SetIdentity();
		lClusterRelativeCurrentPositionInverse[motcnt].SetIdentity();
	}
	veclClusterGlobalCurrentPosition.clear();

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

	vecLocalTransform.clear();
	veclClusterGlobalCurrentPosition.clear();


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


int CBone::UpdateMatrix( int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat )
{
	int existflag = 0;

	if ((g_previewFlag != 5) || (m_parmodel && (m_parmodel->GetBtCnt() == 0))){


		if (srcframe >= 0.0){
			CallF(CalcFBXMotion(srcmotid, srcframe, &m_curmp, &existflag), return 1);
			//ChaMatrix tmpmat = m_curmp.GetWorldMat();// **wmat;
			ChaMatrix tmpmat = m_curmp.GetWorldMat() * *wmat;
			m_curmp.SetWorldMat(tmpmat);

			ChaVector3 jpos = GetJointFPos();
			ChaVector3TransformCoord(&m_childworld, &jpos, &m_curmp.GetWorldMat());

			ChaMatrix wvpmat = m_curmp.GetWorldMat() * *vpmat;

			ChaVector3TransformCoord(&m_childscreen, &m_childworld, vpmat);
			//ChaVector3TransformCoord(&m_childscreen, &m_childworld, &wvpmat);
		}
		else{
			m_curmp.InitParams();
			m_curmp.SetWorldMat(*wmat);
		}

		if (m_parmodel->GetBtCnt() == 0) {
			SetBtMat(m_curmp.GetWorldMat());
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
	return 0;
}


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
	int result = GetBefNextMP(srcmotid, srcframe, &pbef, &pnext, existptr);
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
			CallF(pbef->AddToNext(newmp), return 0);
		}
		else{
			m_motionkey[srcmotid - 1] = newmp;
			if (pnext){
				newmp->SetNext(pnext);
			}
		}
	}

	LeaveCriticalSection(&m_CritSection_AddMP);

	return newmp;
}



int CBone::CalcFBXMotion( int srcmotid, double srcframe, CMotionPoint* dstmpptr, int* existptr )
{
	CMotionPoint* befptr = 0;
	CMotionPoint* nextptr = 0;
	CallF( GetBefNextMP( srcmotid, srcframe, &befptr, &nextptr, existptr ), return 1 );
	CallF( CalcFBXFrame( srcframe, befptr, nextptr, *existptr, dstmpptr ), return 1 );

	return 0;
}

void CBone::ResetMotionCache()
{
	ZeroMemory(m_cachebefmp, sizeof(CMotionPoint*) * (MAXMOTIONNUM + 1));
}

int CBone::GetBefNextMP(int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr)
{
	EnterCriticalSection(&m_CritSection_GetBefNext);

	CMotionPoint* pbef = 0;
	//CMotionPoint* pcur = m_motionkey[srcmotid -1];
	CMotionPoint* pcur = 0;

	*existptr = 0;

	if ((srcmotid <= 0) || (srcmotid > m_motionkey.size())) {
		//AddMotionPointから呼ばれるときに通る場合は正常
		*ppbef = 0;
		*ppnext = 0;
		//_ASSERT(0);
		LeaveCriticalSection(&m_CritSection_GetBefNext);
		return 0;
	}
	else {
		pcur = m_motionkey[srcmotid - 1];
	}


#ifdef USE_CACHE_ONGETMOTIONPOINT__
	//キャッシュをチェックする
	if ((srcmotid >= 0) && (srcmotid <= MAXMOTIONNUM) && m_cachebefmp[srcmotid] &&
		((m_cachebefmp[srcmotid])->GetUseFlag() == 1) &&
		((m_cachebefmp[srcmotid])->GetFrame() <= (srcframe + 0.0001))) {
		//高速化のため途中からの検索にする
		pcur = m_cachebefmp[srcmotid];
	}
#endif

	while (pcur) {

		if ((pcur->GetFrame() >= srcframe - 0.0001) && (pcur->GetFrame() <= srcframe + 0.0001)) {
			*existptr = 1;
			pbef = pcur;
			break;
		}
		else if (pcur->GetFrame() > srcframe) {
			*existptr = 0;
			break;
		}
		else {
			pbef = pcur;
			pcur = pcur->GetNext();
		}
	}
	*ppbef = pbef;

	if (*existptr) {
		*ppnext = pbef->GetNext();
	}
	else {
		*ppnext = pcur;
	}

#ifdef USE_CACHE_ONGETMOTIONPOINT__
	//m_cachebefmp = pbef;
	if ((srcmotid >= 0) && (srcmotid <= MAXMOTIONNUM)){
		if (pbef) {
			m_cachebefmp[srcmotid] = pbef->GetPrev();
		}
		else {
			m_cachebefmp[srcmotid] = m_motionkey[srcmotid - 1];
		}
	}
#endif

	LeaveCriticalSection(&m_CritSection_GetBefNext);

	return 0;
}


int CBone::CalcFBXFrame( double srcframe, CMotionPoint* befptr, CMotionPoint* nextptr, int existflag, CMotionPoint* dstmpptr )
{

	if( existflag == 1 ){
		*dstmpptr = *befptr;
		return 0;
	}else if( !befptr ){
		dstmpptr->InitParams();
		dstmpptr->SetFrame( srcframe );
		return 0;
	}else if( !nextptr ){
		*dstmpptr = *befptr;
		dstmpptr->SetFrame( srcframe );
		return 0;
	}else{
		double diffframe = nextptr->GetFrame() - befptr->GetFrame();
		_ASSERT( diffframe != 0.0 );
		double t = ( srcframe - befptr->GetFrame() ) / diffframe;

		ChaMatrix tmpmat = befptr->GetWorldMat() + (nextptr->GetWorldMat() - befptr->GetWorldMat()) * (float)t;
		dstmpptr->SetWorldMat( tmpmat );
		dstmpptr->SetFrame( srcframe );

		dstmpptr->SetPrev( befptr );
		dstmpptr->SetNext( nextptr );

		return 0;
	}
}

int CBone::DeleteMotion( int srcmotid )
{
	map<int, CMotionPoint*>::iterator itrmp;
	itrmp = m_motionkey.find( srcmotid );
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

	m_motionkey.erase( itrmp );

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

	return 0;
}


int CBone::SetName( char* srcname )
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
	m_laxismat._11 = vecx1.x;
	m_laxismat._12 = vecx1.y;
	m_laxismat._13 = vecx1.z;

	m_laxismat._21 = vecy1.x;
	m_laxismat._22 = vecy1.y;
	m_laxismat._23 = vecy1.z;

	m_laxismat._31 = vecz1.x;
	m_laxismat._32 = vecz1.y;
	m_laxismat._33 = vecz1.z;

	m_axisq.RotationMatrix(m_laxismat);

	return 0;
}



float CBone::CalcAxisMatX(int bindflag, CBone* childbone, ChaMatrix* dstmat, int setstartflag)
{
	ChaVector3 aftbonepos;
	ChaVector3 aftchildpos;

	if (bindflag == 1) {
		//bind pose
		aftbonepos = GetJointFPos();
		aftchildpos = childbone->GetJointFPos();
	}
	else {
		if (g_previewFlag != 5) {
			if (setstartflag == 1) {
				ChaVector3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetCurrentZeroFrameMat(0)));
				ChaVector3TransformCoord(&aftchildpos, &(childbone->GetJointFPos()), &(GetCurrentZeroFrameMat(0)));
			}
			else {
				ChaVector3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetCurMp().GetWorldMat()));
				ChaVector3TransformCoord(&aftchildpos, &(childbone->GetJointFPos()), &(GetCurMp().GetWorldMat()));
			}
		}
		else {
			if (setstartflag == 1) {
				ChaVector3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetCurrentZeroFrameMat(0)));
				ChaVector3TransformCoord(&aftchildpos, &(childbone->GetJointFPos()), &(GetCurrentZeroFrameMat(0)));
			}
			else {
				ChaVector3TransformCoord(&aftbonepos, &(GetJointFPos()), &(GetBtMat()));
				ChaVector3TransformCoord(&aftchildpos, &(childbone->GetJointFPos()), &(GetBtMat()));
			}
		}
	}

	//ChaVector3 curpos;
	//ChaVector3 childpos;
	////ChaVector3TransformCoord(&curpos, &(m_parent->GetJointFPos()), &(m_parent->m_startmat2));
	////ChaVector3TransformCoord(&childpos, &(GetJointFPos()), &m_startmat2);

	//ChaVector3TransformCoord(&curpos, &(GetJointFPos()), &(m_curmp.GetWorldMat()));
	//////ChaVector3TransformCoord(&childpos, &(childbone->GetJointFPos()), &(childbone->m_curmp.GetWorldMat()));
	//ChaVector3TransformCoord(&childpos, &(childbone->GetJointFPos()), &(m_curmp.GetWorldMat()));

	ChaMatrix retmat;
	ChaMatrixIdentity(&retmat);
	if (aftbonepos == aftchildpos){
		*dstmat = retmat;
		dstmat->_41 = aftbonepos.x;
		dstmat->_42 = aftbonepos.y;
		dstmat->_43 = aftbonepos.z;
		//_ASSERT(0);
		return 0.0f;
	}

	ChaVector3 startpos, endpos, upvec;

	ChaVector3 vecx0, vecy0, vecz0;
	ChaVector3 vecx1, vecy1, vecz1;

	//startpos = curpos;
	//endpos = childpos;
	startpos = aftbonepos;
	endpos = aftchildpos;

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

	if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.y) <= 0.000001f)){
		//bonevecが実質Z軸
		//if (bonevec.z >= 0.0f){
			upvec.x = 0.0f;
			upvec.y = 1.0f;
			upvec.z = 0.0f;
		//}
		//else{
		//	upvec.x = 0.0f;
		//	upvec.y = -1.0f;
		//	upvec.z = 0.0f;
		//}
		m_upkind = UPVEC_Y;//vecx1-->X(bone), vecy1-->Z, vecz1-->Y
	}
	else if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.z) <= 0.000001f)){
		//bonevecが実質Y軸
		//if (bonevec.y >= 0.0f){
			upvec.x = 0.0f;
			upvec.y = 0.0f;
			upvec.z = 1.0f;
		//}
		//else{
		//	upvec.x = 0.0f;
		//	upvec.y = 0.0f;
		//	upvec.z = -1.0f;
		//}
		m_upkind = UPVEC_Z;//vecx1-->X(bone), vecy1-->Y, vecz1-->Z
	}
	else{
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
		m_upkind = UPVEC_Z;//vecx1-->X(bone), vecy1-->Y, vecz1-->Z
	}
	vecx1 = bonevec;


	if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.y) <= 0.000001f)){
		//bonevecが実質Z軸
		//if (bonevec.z >= 0.0f){
			ChaVector3Cross(&vecy1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecx1);
		//}
		//else{
		//	ChaVector3Cross(&vecy1, &vecx1, &upvec);
		//}
		//ChaVector3Cross(&vecy1, (const ChaVector3*)&vecx1, (const ChaVector3*)&upvec);
	}
	else if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.z) <= 0.000001f)){
		//bonevecが実質Y軸
		//if (bonevec.y >= 0.0f){
			ChaVector3Cross(&vecy1, (const ChaVector3*)&vecx1, (const ChaVector3*)&upvec);
		//}
		//else{
		//	ChaVector3Cross(&vecy1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecx1);
		//}
	}
	else{
		//float dotbonex = ChaVector3Dot(&vecx1, &vecx0);
		////if (dotbonex >= 0.0f){
			ChaVector3Cross(&vecy1, (const ChaVector3*)&upvec, (const ChaVector3*)&vecx1);
		////}
		////else{
		////	ChaVector3Cross(&vecy1, &vecx1, &upvec);
		////}
		//ChaVector3Cross(&vecy1, (const ChaVector3*)&vecx1, (const ChaVector3*)&upvec);

	}


	/*
	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.z) >= 0.000001f)){
		ChaVector3Cross(&vecy1, &upvec, &vecx1);
	}
	else{
		//bonevecがY軸
		ChaVector3Cross(&vecy1, (const ChaVector3*)&vecx1, (const ChaVector3*)&upvec);
	}
	*/

	int illeagalflag = 0;
	float crleng = ChaVector3Length(&vecy1);
	if (crleng < 0.000001f){
		illeagalflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	ChaVector3Normalize(&vecy1, &vecy1);


	if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.y) <= 0.000001f)){
		//bonevecが実質Z軸
		//if (bonevec.z >= 0.0f){
			ChaVector3Cross(&vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1);
		//}
		//else{
		//	ChaVector3Cross(&vecz1, &vecy1, &vecx1);
		//}

//		ChaVector3Cross(&vecz1, (const ChaVector3*)&vecy1, (const ChaVector3*)&vecx1);
	}
	else if ((fabs(bonevec.x) <= 0.000001f) && (fabs(bonevec.z) <= 0.000001f)){
		//bonevecが実質Y軸
		//if (bonevec.y >= 0.0f){
			ChaVector3Cross(&vecz1, (const ChaVector3*)&vecy1, (const ChaVector3*)&vecx1);
		//}
		//else{
		//	ChaVector3Cross(&vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1);
		//}
	}
	else{
		//float dotbonex = ChaVector3Dot(&vecx1, &vecx0);
		////if (dotbonex >= 0.0f){
			ChaVector3Cross(&vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1);
		////}
		////else{
		////	ChaVector3Cross(&vecz1, &vecy1, &vecx1);
		////}

		//ChaVector3Cross(&vecz1, (const ChaVector3*)&vecy1, (const ChaVector3*)&vecx1);
	}


	/*
	if ((fabs(bonevec.x) >= 0.000001f) || (fabs(bonevec.z) >= 0.000001f)){
		if (fabs(bonevec.y) >= 0.000001f){
			ChaVector3Cross(&vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1);
		}
		else{
			//bonevecがZ軸
			ChaVector3Cross(&vecz1, (const ChaVector3*)&vecy1, (const ChaVector3*)&vecx1);
		}
	}
	else{
		//bonevecがY軸
		ChaVector3Cross(&vecz1, (const ChaVector3*)&vecy1, (const ChaVector3*)&vecx1);
	}
	*/
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

	dstmat->_41 = aftbonepos.x;
	dstmat->_42 = aftbonepos.y;
	dstmat->_43 = aftbonepos.z;

	ChaVector3 diffvec = aftbonepos - aftchildpos;
	float retleng = ChaVector3Length(&diffvec);

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
	float crleng = ChaVector3Length(&vecy1);
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


	retmat._11 = vecx1.x;
	retmat._12 = vecx1.y;
	retmat._13 = vecx1.z;

	retmat._21 = vecy1.x;
	retmat._22 = vecy1.y;
	retmat._23 = vecy1.z;

	retmat._31 = vecz1.x;
	retmat._32 = vecz1.y;
	retmat._33 = vecz1.z;

	*dstmat = retmat;

	return 0;
}



int CBone::CalcAxisMatY( CBone* childbone, ChaMatrix* dstmat )
{

	ChaVector3 curpos;
	ChaVector3 childpos;

	ChaVector3TransformCoord(&curpos, &(GetJointFPos()), &(m_curmp.GetWorldMat()));
	//ChaVector3TransformCoord(&childpos, &(childbone->GetJointFPos()), &(childbone->m_curmp.GetWorldMat()));
	ChaVector3TransformCoord(&childpos, &(childbone->GetJointFPos()), &(m_curmp.GetWorldMat()));

	ChaVector3 diff = curpos - childpos;
	float leng;
	leng = ChaVector3Length( &diff );

	if( leng <= 0.00001f ){
		ChaMatrixIdentity( dstmat );
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
	vecy0.y = 1.0;//!!!!!!!!!!!!!!!!!!
	vecy0.z = 0.0;

	vecz0.x = 0.0;
	vecz0.y = 0.0;
	vecz0.z = 1.0;

	ChaVector3 bonevec;
	bonevec = endpos - startpos;
	ChaVector3Normalize( &bonevec, &bonevec );

	if( (bonevec.x != 0.0f) || (bonevec.y != 0.0f) ){
		upvec.x = 0.0f;
		upvec.y = 0.0f;
		upvec.z = 1.0f;
	}else{
		upvec.x = 1.0f;
		upvec.y = 0.0f;
		upvec.z = 0.0f;
	}

	vecy1 = bonevec;
		
	ChaVector3Cross( &vecx1, (const ChaVector3*)&vecy1, (const ChaVector3*)&upvec );

	int illeagalflag = 0;
	float crleng = ChaVector3Length( &vecx1 );
	if( crleng < 0.000001f ){
		illeagalflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	ChaVector3Normalize( &vecx1, &vecx1 );

	ChaVector3Cross( &vecz1, (const ChaVector3*)&vecx1, (const ChaVector3*)&vecy1 );
	ChaVector3Normalize( &vecy1, &vecy1 );

	//D3DXQUATERNION tmpxq;

	ChaMatrixIdentity( dstmat );
	if( illeagalflag == 0 ){
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

int CBone::CalcRigidElemParams( CBone* childbone, int setstartflag )
{
	
	//剛体の形状(m_coldisp)を複数の子供で使いまわしている。使用するたびにこの関数で大きさをセットしている。


	CRigidElem* curre = GetRigidElem(childbone);
	if( !curre ){
		_ASSERT( 0 );
		return 0;
	}

	//_ASSERT( colptr );
	_ASSERT( childbone );

	CModel* curcoldisp = m_coldisp[curre->GetColtype()];
	_ASSERT( curcoldisp );



	ChaMatrix bmmat;
	ChaMatrixIdentity(&bmmat);
	//CalcAxisMatZ( &aftbonepos, &aftchildpos );
	//CalcAxisMatY( childbone, &bmmat );			
	//float diffleng = CalcAxisMatX(0, childbone, &bmmat, 1);

	ChaMatrix bindcapsulemat;
	ChaMatrixIdentity(&bindcapsulemat);
	float diffleng = CalcAxisMatX(1, childbone, &bindcapsulemat, 1);


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

	curre->SetBindcapsulemat(bindcapsulemat);
	//bmmat = curre->GetEndbone()->CalcManipulatorPostureMatrix(0, 1, 1);

	//curre->SetCapsulemat( bmmat );
	curre->SetCylileng( cylileng );
	curre->SetSphr( sphr );
	curre->SetBoxz( boxz );



	if( setstartflag != 0 ){
		bmmat = curre->GetCapsulemat(1);
		curre->SetFirstcapsulemat( bmmat );
		curre->SetFirstWorldmat(childbone->GetCurrentZeroFrameMat(0));
		//curre->SetFirstWorldmat(GetCurMp().GetWorldMat());

		//if (setstartflag == 2){
		//	childbone->SetNodeMat(bmmat);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//}

		childbone->SetBtMat(childbone->GetCurMp().GetWorldMat());//!!!!!!!!!!!!!btmatの初期値
	}


	m_firstcalcrigid = false;

	return 0;
}

void CBone::SetStartMat2Req()
{
	//SetStartMat2(m_curmp.GetWorldMat());
	SetStartMat2(GetCurrentZeroFrameMat(0));
	SetBtMat(m_curmp.GetWorldMat());//!!!!!!!!!!!!!btmatの初期値

	if (m_child){
		m_child->SetStartMat2Req();
	}
	if (m_brother){
		m_brother->SetStartMat2Req();
	}
}

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
	startpar0._41 = 0.0f;
	startpar0._42 = 0.0f;
	startpar0._43 = 0.0f;

	ChaMatrix starty = gaxisy;
	starty._41 = 0.0f;
	starty._42 = 0.0f;
	starty._43 = 0.0f;

	ChaMatrix motmat0 = motmat;
	motmat0._41 = 0.0f;
	motmat0._42 = 0.0f;
	motmat0._43 = 0.0f;

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


CMotionPoint* CBone::AddBoneTraReq( CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srctra )
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !curmp || !existflag ){
		_ASSERT( 0 );
		return 0;
	}


	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	if( parmp ){
		ChaMatrix invbefpar;
		ChaMatrixInverse( &invbefpar, NULL, &parmp->GetBefWorldMat() );
		ChaMatrix tmpmat = curmp->GetWorldMat() * invbefpar * parmp->GetWorldMat();
		g_wmatDirectSetFlag = true;
		SetWorldMat( 0, srcmotid, srcframe, tmpmat );
		g_wmatDirectSetFlag = false;
	}
	else{
		ChaMatrix tramat;
		ChaMatrixTranslation( &tramat, srctra.x, srctra.y, srctra.z );
		ChaMatrix tmpmat = curmp->GetWorldMat() * tramat;
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, tmpmat);
		g_wmatDirectSetFlag = false;
	}

	curmp->SetAbsMat( curmp->GetWorldMat() );

	if (m_child){
		m_child->AddBoneTraReq(curmp, srcmotid, srcframe, srctra);
	}
	if (m_brother && parmp){
		m_brother->AddBoneTraReq(parmp, srcmotid, srcframe, srctra);
	}

	return curmp;
}


CMotionPoint* CBone::PasteRotReq( int srcmotid, double srcframe, double dstframe )
{
	//src : srcmp srcparmp
	//dst : curmp parmp

	int existflag0 = 0;
	CMotionPoint* srcmp = AddMotionPoint( srcmotid, srcframe, &existflag0 );
	if( !existflag0 || !srcmp ){
		_ASSERT( 0 );
		return 0;
	}

	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, dstframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}
	
	//curmp->SetBefWorldMat( curmp->GetWorldMat() );
	curmp->SetWorldMat( srcmp->GetWorldMat() );
	curmp->SetAbsMat( srcmp->GetAbsMat() );


	//オイラー角初期化
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	int paraxsiflag = 1;
	int isfirstbone = 0;
	cureul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);
	SetLocalEul(srcmotid, srcframe, cureul);


	if( m_child ){
		m_child->PasteRotReq( srcmotid, srcframe, dstframe );
	}
	if( m_brother ){
		m_brother->PasteRotReq( srcmotid, srcframe, dstframe );
	}
	return curmp;

}


CMotionPoint* CBone::RotBoneQReq(CMotionPoint* parmp, int srcmotid, double srcframe, CQuaternion rotq, CBone* bvhbone, ChaVector3 traanim, int setmatflag, ChaMatrix* psetmat)
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}
	

	if (parmp){
		//再帰から呼び出し
		ChaMatrix befparmat = parmp->GetBefWorldMat();
		ChaMatrix newparmat = parmp->GetWorldMat();
		//if ((g_underRetargetFlag == true) || (IsSameMat(befparmat, newparmat) == 0)){
			ChaMatrix invbefpar;
			ChaMatrixInverse(&invbefpar, NULL, &befparmat);
			ChaMatrix tmpmat = curmp->GetWorldMat() * invbefpar * newparmat;
			g_wmatDirectSetFlag = true;
			SetWorldMat(0, srcmotid, srcframe, tmpmat);
			g_wmatDirectSetFlag = false;
			//}
		//else{
			//parmatに変化がないときは変更しない。
		//	curmp->SetBefWorldMat( curmp->GetWorldMat() );
		//}
		if (bvhbone){
			if (m_child){
				bvhbone->SetTmpMat(curmp->GetWorldMat());
			}
			else{
				bvhbone->SetTmpMat(newparmat);
			}
		}
	}
	else{
		//初回呼び出し
		ChaMatrix tramat;
		ChaMatrixIdentity(&tramat);
		ChaMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);

		if (m_child){
			if (setmatflag == 0){
				ChaVector3 rotcenter;// = m_childworld;
				ChaVector3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));

				ChaMatrix befrot, aftrot;
				ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
				ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
				ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
				ChaMatrix tmpmat = curmp->GetWorldMat() * rotmat * tramat;
				//directflagまたはunderRetargetFlagがないときはtramat成分は無視され、SetWorldMatFromEul中でbone::CalcLocalTraAnimの値が適用される。
				SetWorldMat(0, srcmotid, srcframe, tmpmat);
				if (bvhbone){
					bvhbone->SetTmpMat(tmpmat);
				}
			}
			else{
				ChaMatrix tmpmat = *psetmat;
				g_wmatDirectSetFlag = true;
				SetWorldMat(0, srcmotid, srcframe, tmpmat);
				g_wmatDirectSetFlag = false;
				if (bvhbone){
					bvhbone->SetTmpMat(tmpmat);
				}
			}
		}
		else{
			ChaVector3 rotcenter;// = m_childworld;
			ChaVector3TransformCoord(&rotcenter, &(GetJointFPos()), &(curmp->GetWorldMat()));

			ChaMatrix befrot, aftrot;
			ChaMatrixTranslation(&befrot, -rotcenter.x, -rotcenter.y, -rotcenter.z);
			ChaMatrixTranslation(&aftrot, rotcenter.x, rotcenter.y, rotcenter.z);
			ChaMatrix rotmat = befrot * rotq.MakeRotMatX() * aftrot;
			ChaMatrix tmpmat = curmp->GetWorldMat() * rotmat * tramat;
			g_wmatDirectSetFlag = true;//!!!!!!!!
			SetWorldMat(0, srcmotid, srcframe, tmpmat);
			g_wmatDirectSetFlag = false;//!!!!!!!
			if (bvhbone){
				bvhbone->SetTmpMat(tmpmat);
			}
		}
	}


	curmp->SetAbsMat(curmp->GetWorldMat());

	if (m_child && curmp){
		m_child->RotBoneQReq(curmp, srcmotid, srcframe, rotq);
	}
	if (m_brother && parmp){
		m_brother->RotBoneQReq(parmp, srcmotid, srcframe, rotq);
	}
	return curmp;
}




CMotionPoint* CBone::RotBoneQOne(CMotionPoint* parmp, int srcmotid, double srcframe, ChaMatrix srcmat)
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint(srcmotid, srcframe, &existflag);
	if (!existflag || !curmp){
		_ASSERT(0);
		return 0;
	}

	if (parmp){
		//parentの行列をセット !!!!!!!!!
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, parmp->GetWorldMat());
		g_wmatDirectSetFlag = false;
	} else{
		g_wmatDirectSetFlag = true;
		SetWorldMat(0, srcmotid, srcframe, srcmat);
		g_wmatDirectSetFlag = false;
	}

	curmp->SetAbsMat(curmp->GetWorldMat());

	return curmp;
}


CMotionPoint* CBone::SetAbsMatReq( int broflag, int srcmotid, double srcframe, double firstframe )
{
	int existflag = 0;
	CMotionPoint* curmp = AddMotionPoint( srcmotid, srcframe, &existflag );
	if( !existflag || !curmp ){
		_ASSERT( 0 );
		return 0;
	}

	int existflag2 = 0;
	CMotionPoint* firstmp = AddMotionPoint( srcmotid, firstframe, &existflag2 );
	if( !existflag2 || !firstmp ){
		_ASSERT( 0 );
		return 0;
	}

	g_wmatDirectSetFlag = true;
	SetWorldMat(0, srcmotid, srcframe, firstmp->GetAbsMat());
	g_wmatDirectSetFlag = false;

	if( m_child ){
		m_child->SetAbsMatReq( 1, srcmotid, srcframe, firstframe );
	}
	if( m_brother && broflag ){
		m_brother->SetAbsMatReq( 1, srcmotid, srcframe, firstframe );
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
	itrcur = m_motmark.find( motid );
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
	itrcur = m_motmark.find( motid );
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

int CBone::CalcLocalInfo( int motid, double frameno, CMotionPoint* pdstmp )
{

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(motid, frameno);
	if( m_parent ){
		if( pcurmp ){
			pparmp = m_parent->GetMotionPoint(motid, frameno);
			if( pparmp ){
				CMotionPoint setmp;
				ChaMatrix invpar = pparmp->GetInvWorldMat();
				ChaMatrix localmat = pcurmp->GetWorldMat() * invpar;
				//pcurmp->CalcQandTra(localmat, this);
				setmp.CalcQandTra(localmat, this);

				int inirotcur, inirotpar;
				inirotcur = IsInitRot(pcurmp->GetWorldMat());
				inirotpar = IsInitRot(pparmp->GetWorldMat());
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
				_ASSERT( 0 );
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
			ChaMatrix localmat = pcurmp->GetWorldMat();
			setmp.CalcQandTra( localmat, this );

			int inirotcur;
			inirotcur = IsInitRot(pcurmp->GetWorldMat());
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

int CBone::CalcInitLocalInfo(int motid, double frameno, CMotionPoint* pdstmp)
{

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(motid, frameno);
	if (m_parent){
		if (pcurmp){
			pparmp = GetMotionPoint(motid, frameno);
			if (pparmp){
				ChaMatrix invpar = pparmp->GetInvWorldMat();
				ChaMatrix invinitmat = GetInvInitMat();
				ChaMatrix localmat = invinitmat * pcurmp->GetWorldMat() * invpar;//world == init * local * parだからlocalを計算するには、invinit * world * invpar。
				pcurmp->CalcQandTra(localmat, this);
			}
			else{
				_ASSERT(0);
				return 0;
			}
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}
	else{
		if (pcurmp){
			ChaMatrix invinitmat = GetInvInitMat();
			ChaMatrix localmat = invinitmat * pcurmp->GetWorldMat();
			pcurmp->CalcQandTra(localmat, this);
		}
		else{
			_ASSERT(0);
			return 0;
		}
	}

	*pdstmp = *pcurmp;

	return 0;
}


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
	ChaVector3 jpos = GetJointFPos();
	ChaVector3TransformCoord(&m_firstframebonepos, &jpos, &srcmat);

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
	
	if (m_parmodel->GetOldAxisFlagAtLoading() == 0){
		return m_jointfpos;
	}
	else{
		return m_oldjointfpos;
	}
}
void CBone::SetJointFPos(ChaVector3 srcpos)
{ 
	m_jointfpos = srcpos; 
}
void CBone::SetOldJointFPos(ChaVector3 srcpos){
	m_oldjointfpos = srcpos;
}


ChaVector3 CBone::CalcLocalEulXYZ(int axiskind, int srcmotid, double srcframe, enum tag_befeulkind befeulkind, int isfirstbone, ChaVector3* directbefeul)
{
	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)

	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);

	const WCHAR* bonename = GetWBoneName();
	if (wcscmp(bonename, L"RootNode") == 0){
		return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
	}

	if (befeulkind == BEFEUL_BEFFRAME){
		//1つ前のフレームのEULはすでに計算されていると仮定する。
		double befframe;
		befframe = srcframe - 1.0;
		if (befframe >= -0.0001){
			CMotionPoint* befmp;
			befmp = GetMotionPoint(srcmotid, befframe);
			if (befmp){
				befeul = befmp->GetLocalEul();
			}
		}
	}
	else if ((befeulkind == BEFEUL_DIRECT) && directbefeul){
		befeul = *directbefeul;
	}

	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);//local!!!
	
	ChaMatrix axismat;
	CQuaternion axisq;
	//int multworld = 0;//local!!!
	//axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	//axisq.RotationMatrix(axismat);

	if (GetParent()) {
		CRigidElem* curre = GetParent()->GetRigidElem(this);
		if (curre) {
			axismat = curre->GetBindcapsulemat();
		}
		else {
			//_ASSERT(0);
			ChaMatrixIdentity(&axismat);
		}
		axisq.RotationMatrix(axismat);
	}
	else {
		ChaMatrixIdentity(&axismat);
		axisq.SetParams(1.0, 0.0, 0.0, 0.0);
	}

	if (axiskind == -1){
		if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
			tmpmp.GetQ().Q2EulXYZ(&axisq, befeul, &cureul);
		}
		else{
			tmpmp.GetQ().Q2EulXYZ(0, befeul, &cureul);
		}
	}
	else if (axiskind != BONEAXIS_GLOBAL){
		tmpmp.GetQ().Q2EulXYZ(&axisq, befeul, &cureul);
	}
	else{
		tmpmp.GetQ().Q2EulXYZ(0, befeul, &cureul);
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		ChaVector3 oldeul = curmp->GetLocalEul();
		if (IsSameEul(oldeul, cureul) == 0){
			return cureul;
		}
		else{
			return oldeul;
		}
	}
	else{
		return cureul;
	}
}



ChaMatrix CBone::CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe)
{
	ChaMatrix selm;
	ChaMatrixIdentity(&selm);

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	CMotionPoint* pgparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, srcframe);
	if (!pcurmp){
		//_ASSERT(0);
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	if (m_parent){
		pparmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (!pparmp){
			_ASSERT(0);
			return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
	}
	else{
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	if (m_parent && m_parent->GetParent()) {
		pgparmp = m_parent->GetParent()->GetMotionPoint(srcmotid, srcframe);
	}
	else {
		pgparmp = 0;
	}


	CRigidElem* curre = m_parent->GetRigidElem(this);
	if (!curre){
		//_ASSERT(0);
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	ChaMatrix capsulemat;
	ChaMatrix invcapsulemat;
	if (m_parent){
		if (curre){
			//capsulemat = curre->GetFirstcapsulemat();
			capsulemat = curre->GetBindcapsulemat();
		}
		else{
			ChaMatrixIdentity(&capsulemat);
		}
	}
	else{
		ChaMatrixIdentity(&capsulemat);
	}
	invcapsulemat = ChaMatrixInv(capsulemat);



	CRigidElem* parre = 0;
	if (m_parent && m_parent->GetParent()) {
		parre = m_parent->GetParent()->GetRigidElem(m_parent);
	}
	ChaMatrix parcapsulemat;
	ChaMatrix invparcapsulemat;
	if (parre) {
		//parcapsulemat = parre->GetFirstcapsulemat();
		parcapsulemat = parre->GetBindcapsulemat();
	}
	else {
		ChaMatrixIdentity(&parcapsulemat);
	}
	invparcapsulemat = ChaMatrixInv(parcapsulemat);


	CRigidElem* childre = 0;
	if (GetChild()) {
		childre = GetRigidElem(GetChild());//複数子供がある場合にはそれを指定しなければならない。ここでは仮実装として長男を選択。
	}
	ChaMatrix childcapsulemat;
	ChaMatrix invchildcapsulemat;
	if (childre) {
		//childcapsulemat = childre->GetFirstcapsulemat();
		childcapsulemat = childre->GetBindcapsulemat();
	}
	else {
		ChaMatrixIdentity(&childcapsulemat);
	}
	invchildcapsulemat = ChaMatrixInv(childcapsulemat);


	ChaMatrix worldmat, parworldmat, gparworldmat;
	ChaMatrix diffworld, pardiffworld, gpardiffworld;
	if (pcurmp){
		if (g_previewFlag != 5){
			worldmat = pcurmp->GetWorldMat();
			//diffworld = curre->GetInvFirstWorldmat() * worldmat;
			//diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);
			diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);
		}
		else{
			worldmat = GetBtMat();
			////diffworld = GetInvStartMat2() * worldmat;
			////diffworld = GetInvStartMat2() * gparworldmat;//シミュレーション開始時からの変化分
			//if (childre) {
			//	diffworld = childre->GetInvFirstWorldmat() * worldmat;//初期状態からの変化分
			//}
			//else {
			//	diffworld = curre->GetInvFirstWorldmat() * worldmat;
			//}
			//diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);
			diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);

		}
		//diffworld = curre->GetInvFirstWorldmat() * worldmat;
		//ChaMatrixIdentity(&diffworld);
		//diffworld = curre->GetInvFirstWorldmat() * worldmat;//!!!!!!!!!!!!!!
	}
	else{
		ChaMatrixIdentity(&worldmat);
		ChaMatrixIdentity(&diffworld);
	}


	if (pparmp){
		if (g_previewFlag != 5){
			parworldmat = pparmp->GetWorldMat();
			//pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
			pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
		}
		else{
			parworldmat = m_parent->GetBtMat();
			//pardiffworld = m_parent->GetInvStartMat2() * parworldmat;//シミュレーション開始時からの変化分
			//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;//初期状態からの変化分
			//pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
			pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
		}
		//pardiffworld = m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat;//!!!!!!
		//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;
		//ChaMatrixIdentity(&pardiffworld);
	}
	else{
		ChaMatrixIdentity(&parworldmat);
		ChaMatrixIdentity(&pardiffworld);
	}


	if (pgparmp) {
		if (g_previewFlag != 5) {
			gparworldmat = pgparmp->GetWorldMat();
			gpardiffworld = MakeRotMatFromChaMatrix(m_parent->GetParent()->GetCurrentZeroFrameInvMat(0) * gparworldmat);
		}
		else {
			gparworldmat = m_parent->GetParent()->GetBtMat();
			//gpardiffworld = m_parent->GetParent()->GetInvStartMat2() * gparworldmat;//シミュレーション開始時からの変化分
			gpardiffworld = MakeRotMatFromChaMatrix(m_parent->GetParent()->GetCurrentZeroFrameInvMat(0) * gparworldmat);

			//if (parre) {
			//	gpardiffworld = parre->GetInvFirstWorldmat() * gparworldmat;//初期状態からの変化分
			//}
			//else {
			//	gpardiffworld = curre->GetInvFirstWorldmat() * gparworldmat;
			//}
		}
		//pardiffworld = m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat;//!!!!!!
		//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;
		//ChaMatrixIdentity(&pardiffworld);
	}
	else {
		ChaMatrixIdentity(&gparworldmat);
		ChaMatrixIdentity(&gpardiffworld);
	}



	//ChaMatrix curcapsulemat, parcapsulemat;
	//parcapsulemat = capsulemat;
	//curcapsulemat = capsulemat * curre->GetFirstWorldmat();


	if (g_boneaxis == 2){
		//global axis
		ChaMatrixIdentity(&selm);
	}
	else if (g_boneaxis == 0){
		//current bone axis
		if (GetBoneLeng() > 0.00001f){
			if (multworld == 1){
				//selm = capsulemat * pardiffworld;
				selm = childcapsulemat * pardiffworld;
			}
			else{
				selm = childcapsulemat;
			}
		}
		else{
			if (multworld == 1){
				selm = parworldmat;
			}
			else{
				ChaMatrixIdentity(&selm);
			}
		}
	}
	else if (g_boneaxis == 1){
		//parent bone axis
		if (GetBoneLeng() > 0.00001f){
			if (multworld == 1){
				//selm = parcapsulemat * gpardiffworld;
				selm = capsulemat * gpardiffworld;
			}
			else{
				//selm = parcapsulemat;
				selm = capsulemat;
			}
		}
		else{
			if (multworld == 1){
				selm = gparworldmat;
			}
			else{
				ChaMatrixIdentity(&selm);
			}
		}
	}
	else{
		_ASSERT(0);
		ChaMatrixIdentity(&selm);
	}

	if (settraflag == 0){
		selm._41 = 0.0f;
		selm._42 = 0.0f;
		selm._43 = 0.0f;
	}
	else{
		ChaVector3 aftjpos;
		//ChaVector3TransformCoord(&aftjpos, &(GetJointFPos()), &worldmat);
		ChaVector3TransformCoord(&aftjpos, &(GetParent()->GetJointFPos()), &parworldmat);

		selm._41 = aftjpos.x;
		selm._42 = aftjpos.y;
		selm._43 = aftjpos.z;
	}


	return selm;
}

ChaMatrix CBone::CalcManipulatorPostureMatrix(int calccapsuleflag, int anglelimitaxisflag, int settraflag, int multworld, int calczeroframe)
{

	//Posture

	ChaMatrix selm;
	ChaMatrixIdentity(&selm);
	if (!m_parmodel) {
		return selm;
	}

	int srcmotid = m_curmotid;
	double srcframe;
	if (calczeroframe == 0) {
		if (m_parmodel->GetCurMotInfo()) {
			srcframe = m_parmodel->GetCurMotInfo()->curframe;
		}
		else {
			srcframe = 0.0;
			_ASSERT(0);
		}
	}
	else {
		srcframe = 0.0;
	}
	
	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	CMotionPoint* pgparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, srcframe);
	if (!pcurmp) {
		//_ASSERT(0);
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	if (m_parent) {
		pparmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (!pparmp) {
			_ASSERT(0);
			return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
	}
	else {
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	if (m_parent && m_parent->GetParent()) {
		pgparmp = m_parent->GetParent()->GetMotionPoint(srcmotid, srcframe);
	}
	else {
		pgparmp = 0;
	}


	CRigidElem* curre = m_parent->GetRigidElem(this);
	if (!curre) {
		//_ASSERT(0);
		return selm;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	ChaMatrix capsulemat;
	ChaMatrix invcapsulemat;
	if (m_parent) {
		if (curre) {
			//capsulemat = curre->GetFirstcapsulemat();
			capsulemat = curre->GetBindcapsulemat();
		}
		else {
			ChaMatrixIdentity(&capsulemat);
		}
	}
	else {
		ChaMatrixIdentity(&capsulemat);
	}
	invcapsulemat = ChaMatrixInv(capsulemat);



	CRigidElem* parre = 0;
	if (m_parent && m_parent->GetParent()) {
		parre = m_parent->GetParent()->GetRigidElem(m_parent);
	}
	ChaMatrix parcapsulemat;
	ChaMatrix invparcapsulemat;
	if (parre) {
		//parcapsulemat = parre->GetFirstcapsulemat();
		parcapsulemat = parre->GetBindcapsulemat();
	}
	else {
		ChaMatrixIdentity(&parcapsulemat);
	}
	invparcapsulemat = ChaMatrixInv(parcapsulemat);


	CRigidElem* childre = 0;
	if (GetChild()) {
		childre = GetRigidElem(GetChild());//複数子供がある場合にはそれを指定しなければならない。ここでは仮実装として長男を選択。
	}
	ChaMatrix childcapsulemat;
	ChaMatrix invchildcapsulemat;
	if (childre) {
		//childcapsulemat = childre->GetFirstcapsulemat();
		childcapsulemat = childre->GetBindcapsulemat();
	}
	else {
		ChaMatrixIdentity(&childcapsulemat);
	}
	invchildcapsulemat = ChaMatrixInv(childcapsulemat);


	ChaMatrix worldmat, parworldmat, gparworldmat;
	ChaMatrix diffworld, pardiffworld, gpardiffworld;
	if (pcurmp) {
		if (g_previewFlag != 5) {
			worldmat = pcurmp->GetWorldMat();
			//diffworld = curre->GetInvFirstWorldmat() * worldmat;
			diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);
		}
		else {
			worldmat = GetBtMat();
			////diffworld = GetInvStartMat2() * worldmat;
			////diffworld = GetInvStartMat2() * gparworldmat;//シミュレーション開始時からの変化分
			//if (childre) {
			//	diffworld = childre->GetInvFirstWorldmat() * worldmat;//初期状態からの変化分
			//}
			//else {
			//	diffworld = curre->GetInvFirstWorldmat() * worldmat;
			//}
			diffworld = MakeRotMatFromChaMatrix(GetCurrentZeroFrameInvMat(0) * worldmat);

		}
		//diffworld = curre->GetInvFirstWorldmat() * worldmat;
		//ChaMatrixIdentity(&diffworld);
		//diffworld = curre->GetInvFirstWorldmat() * worldmat;//!!!!!!!!!!!!!!
	}
	else {
		ChaMatrixIdentity(&worldmat);
		ChaMatrixIdentity(&diffworld);
	}


	if (pparmp) {
		if (g_previewFlag != 5) {
			parworldmat = pparmp->GetWorldMat();
			pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
		}
		else {
			parworldmat = m_parent->GetBtMat();
			//pardiffworld = m_parent->GetInvStartMat2() * parworldmat;//シミュレーション開始時からの変化分
			//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;//初期状態からの変化分
			pardiffworld = MakeRotMatFromChaMatrix(m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat);
		}
		//pardiffworld = m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat;//!!!!!!
		//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;
		//ChaMatrixIdentity(&pardiffworld);
	}
	else {
		ChaMatrixIdentity(&parworldmat);
		ChaMatrixIdentity(&pardiffworld);
	}


	if (pgparmp) {
		if (g_previewFlag != 5) {
			gparworldmat = pgparmp->GetWorldMat();
			gpardiffworld = MakeRotMatFromChaMatrix(m_parent->GetParent()->GetCurrentZeroFrameInvMat(0) * gparworldmat);
		}
		else {
			gparworldmat = m_parent->GetParent()->GetBtMat();
			//gpardiffworld = m_parent->GetParent()->GetInvStartMat2() * gparworldmat;//シミュレーション開始時からの変化分
			gpardiffworld = MakeRotMatFromChaMatrix(m_parent->GetParent()->GetCurrentZeroFrameInvMat(0) * gparworldmat);

			//if (parre) {
			//	gpardiffworld = parre->GetInvFirstWorldmat() * gparworldmat;//初期状態からの変化分
			//}
			//else {
			//	gpardiffworld = curre->GetInvFirstWorldmat() * gparworldmat;
			//}
		}
		//pardiffworld = m_parent->GetCurrentZeroFrameInvMat(0) * parworldmat;//!!!!!!
		//pardiffworld = curre->GetInvFirstWorldmat() * parworldmat;
		//ChaMatrixIdentity(&pardiffworld);
	}
	else {
		ChaMatrixIdentity(&gparworldmat);
		ChaMatrixIdentity(&gpardiffworld);
	}

	if ((calccapsuleflag == 0) && (g_boneaxis == 2)) {
		//global axis
		ChaMatrixIdentity(&selm);
	}
	else if ((calccapsuleflag == 0) && (g_boneaxis == 0)) {
		//current bone axis
		if (GetBoneLeng() > 0.00001f) {
			if (multworld == 1) {
				selm = childcapsulemat * diffworld;
			}
			else {
				selm = childcapsulemat;
			}
		}
		else {
			if (multworld == 1) {
				selm = parworldmat;
			}
			else {
				ChaMatrixIdentity(&selm);
			}
		}
	}
	else if ((calccapsuleflag == 1) || (g_boneaxis == 1)) {
		//parent bone axis
		if (GetBoneLeng() > 0.00001f) {
			if (multworld == 1) {
				selm = capsulemat * pardiffworld;
			}
			else {
				selm = capsulemat;
			}
		}
		else {
			if (multworld == 1) {
				selm = parworldmat;
			}
			else {
				ChaMatrixIdentity(&selm);
			}
		}
	}
	else {
		_ASSERT(0);
		ChaMatrixIdentity(&selm);
	}

	if (settraflag == 0) {
		selm._41 = 0.0f;
		selm._42 = 0.0f;
		selm._43 = 0.0f;
		selm._44 = 1.0f;
	}
	else {
		ChaVector3 aftjpos;
		ChaVector3TransformCoord(&aftjpos, &(GetParent()->GetJointFPos()), &parworldmat);

		selm._41 = aftjpos.x;
		selm._42 = aftjpos.y;
		selm._43 = aftjpos.z;
		selm._44 = 1.0f;
	}


	return selm;
}

/*
ChaMatrix CBone::CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe)
{
	ChaMatrix selm;
	ChaMatrixIdentity(&selm);

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, srcframe);
	if (!pcurmp){
		//_ASSERT(0);
		return selm;
	}
	if (m_parent){
		pparmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (!pparmp){
			_ASSERT(0);
			return selm;
		}
	}

	ChaMatrix worldmat, parworldmat;
	if (pcurmp){
		if (g_previewFlag != 5){
			worldmat = pcurmp->GetWorldMat();
		}
		else{
			worldmat = GetBtMat();
		}
	}
	else{
		ChaMatrixIdentity(&worldmat);
	}
	if (pparmp){
		if (g_previewFlag != 5){
			parworldmat = pparmp->GetWorldMat();
		}
		else{
			parworldmat = m_parent->GetBtMat();
		}
	}
	else{
		ChaMatrixIdentity(&parworldmat);
	}


	if (anglelimitaxisflag == 0){
		if (g_boneaxis == 2){
			//global axis
			ChaMatrixIdentity(&selm);
		}
		else if (g_boneaxis == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else if (g_boneaxis == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			ChaMatrixIdentity(&selm);
		}
	}
	else{
		if (m_anglelimit.boneaxiskind == 2){
			//global axis
			ChaMatrixIdentity(&selm);
		}
		else if (m_anglelimit.boneaxiskind == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (multworld == 1){
						selm = GetNodeMat() * worldmat;
					}
					else{
						selm = GetNodeMat();
					}
				}
			}
			else{
				//endjoint
				if (multworld == 1){
					selm = worldmat;
				}
				else{
					ChaMatrixIdentity(&selm);
				}
			}
		}
		else if (m_anglelimit.boneaxiskind == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			ChaMatrixIdentity(&selm);
		}
	}

	if (settraflag == 0){
		selm._41 = 0.0f;
		selm._42 = 0.0f;
		selm._43 = 0.0f;
	}
	else{
		ChaVector3 aftjpos;
		ChaVector3TransformCoord(&aftjpos, &(GetJointFPos()), &worldmat);

		selm._41 = aftjpos.x;
		selm._42 = aftjpos.y;
		selm._43 = aftjpos.z;
	}


	return selm;
}
*/

/*
//org
ChaMatrix CBone::CalcManipulatorMatrix(int anglelimitaxisflag, int settraflag, int multworld, int srcmotid, double srcframe)
{
	ChaMatrix selm;
	ChaMatrixIdentity(&selm);

	CMotionPoint* pcurmp = 0;
	CMotionPoint* pparmp = 0;
	pcurmp = GetMotionPoint(srcmotid, srcframe);
	if (!pcurmp){
		//_ASSERT(0);
		return selm;
	}
	if (m_parent){
		pparmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (!pparmp){
			_ASSERT(0);
			return selm;
		}
	}

	ChaMatrix worldmat, parworldmat;
	if (pcurmp){
		if (g_previewFlag != 5){
			worldmat = pcurmp->GetWorldMat();
		}
		else{
			worldmat = GetBtMat();
		}
	}
	else{
		ChaMatrixIdentity(&worldmat);
	}
	if (pparmp){
		if (g_previewFlag != 5){
			parworldmat = pparmp->GetWorldMat();
		}
		else{
			parworldmat = m_parent->GetBtMat();
		}
	}
	else{
		ChaMatrixIdentity(&parworldmat);
	}


	if (anglelimitaxisflag == 0){
		if (g_boneaxis == 2){
			//global axis
			ChaMatrixIdentity(&selm);
		}
		else if (g_boneaxis == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = pcurmp->GetWorldMat();
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else if (g_boneaxis == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = pcurmp->GetWorldMat();
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = pcurmp->GetWorldMat();
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * worldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			ChaMatrixIdentity(&selm);
		}
	}
	else{
		if (m_anglelimit.boneaxiskind == 2){
			//global axis
			ChaMatrixIdentity(&selm);
		}
		else if (m_anglelimit.boneaxiskind == 0){
			//current bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * worldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (multworld == 1){
						selm = GetNodeMat() * worldmat;
					}
					else{
						selm = GetNodeMat();
					}
				}
			}
			else{
				//endjoint
				if (multworld == 1){
					selm = worldmat;
				}
				else{
					ChaMatrixIdentity(&selm);
				}
			}
		}
		else if (m_anglelimit.boneaxiskind == 1){
			//parent bone axis
			if (m_child){
				if (m_parmodel->GetOldAxisFlagAtLoading() == 1){
					//FBXの初期のボーンの向きがIdentityの場合
					if (m_parent){
						if (GetBoneLeng() > 0.00001f){
							if (multworld == 1){
								selm = GetFirstAxisMatZ() * parworldmat;
							}
							else{
								selm = GetFirstAxisMatZ();
							}
						}
						else{
							if (multworld == 1){
								selm = worldmat;
							}
							else{
								ChaMatrixIdentity(&selm);
							}
						}
					}
					else{
						if (multworld == 1){
							selm = worldmat;
						}
						else{
							ChaMatrixIdentity(&selm);
						}
					}
				}
				else{
					//FBXにボーンの初期の軸の向きが記録されている場合
					if (m_parent){
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
					else{
						if (multworld == 1){
							selm = GetNodeMat() * parworldmat;
						}
						else{
							selm = GetNodeMat();
						}
					}
				}
			}
			else{
				//endjoint
				if (m_parent){
					if (multworld == 1){
						selm = parworldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
				else{
					if (multworld == 1){
						selm = worldmat;
					}
					else{
						ChaMatrixIdentity(&selm);
					}
				}
			}
		}
		else{
			_ASSERT(0);
			ChaMatrixIdentity(&selm);
		}
	}

	if (settraflag == 0){
		selm._41 = 0.0f;
		selm._42 = 0.0f;
		selm._43 = 0.0f;
	}
	else{
		ChaVector3 aftjpos;
		ChaVector3TransformCoord(&aftjpos, &(GetJointFPos()), &worldmat);

		selm._41 = aftjpos.x;
		selm._42 = aftjpos.y;
		selm._43 = aftjpos.z;
	}


	return selm;
}
*/

int CBone::SetWorldMatFromEul(int inittraflag, int setchildflag, ChaVector3 srceul, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	if (!m_child){
		return 0;
	}

	ChaMatrix axismat;
	CQuaternion axisq;
	//int multworld = 0;//local!!!
	//axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	//axisq.RotationMatrix(axismat);
	if (GetParent()) {
		CRigidElem* curre = GetParent()->GetRigidElem(this);
		if (curre) {
			axismat = curre->GetBindcapsulemat();
		}
		else {
			_ASSERT(0);
			ChaMatrixIdentity(&axismat);
		}
		axisq.RotationMatrix(axismat);
	}
	else {
		ChaMatrixIdentity(&axismat);
		axisq.SetParams(1.0, 0.0, 0.0, 0.0);
	}


	CQuaternion newrot;
	if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
		newrot.SetRotationXYZ(&axisq, srceul);
	}
	else{
		newrot.SetRotationXYZ(0, srceul);
	}


	QuaternionInOrder(srcmotid, srcframe, &newrot);


	ChaMatrix newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	ChaMatrixIdentity(&befrotmat);
	ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	ChaMatrixIdentity(&aftrotmat);
	ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	if (inittraflag == 0){
		ChaVector3 traanim = CalcLocalTraAnim(srcmotid, srcframe);
		ChaMatrix tramat;
		ChaMatrixIdentity(&tramat);
		ChaMatrixTranslation(&tramat, traanim.x, traanim.y, traanim.z);
		newlocalmat = newlocalmat * tramat;
	}

	ChaMatrix newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			ChaMatrix parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		curmp->SetLocalEul(srceul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}

int CBone::SetWorldMatFromQAndTra(int setchildflag, CQuaternion axisq, CQuaternion srcq, ChaVector3 srctra, int srcmotid, double srcframe)
{
	if (!m_child){
		return 0;
	}

	CQuaternion invaxisq;
	axisq.inv(&invaxisq);
	CQuaternion newrot = invaxisq * srcq * axisq;

	ChaMatrix newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	ChaMatrixIdentity(&befrotmat);
	ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	ChaMatrixIdentity(&aftrotmat);
	ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	ChaMatrix tramat;
	ChaMatrixIdentity(&tramat);
	ChaMatrixTranslation(&tramat, srctra.x, srctra.y, srctra.z);
	newlocalmat = newlocalmat * tramat;


	ChaMatrix newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			ChaMatrix parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		ChaVector3 neweul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO, 0);
		curmp->SetLocalEul(neweul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;




}


int CBone::SetWorldMatFromEulAndTra(int setchildflag, ChaVector3 srceul, ChaVector3 srctra, int srcmotid, double srcframe)
{
	//anglelimitをした後のオイラー角が渡される。anglelimitはCBone::SetWorldMatで処理する。
	if (!m_child){
		return 0;
	}

	ChaMatrix axismat;
	CQuaternion axisq;
	//int multworld = 0;//local!!!
	//axismat = CalcManipulatorMatrix(1, 0, multworld, srcmotid, srcframe);
	//axisq.RotationMatrix(axismat);
	if (GetParent()) {
		CRigidElem* curre = GetParent()->GetRigidElem(this);
		if (curre) {
			axismat = curre->GetBindcapsulemat();
		}
		else {
			_ASSERT(0);
			ChaMatrixIdentity(&axismat);
		}
		axisq.RotationMatrix(axismat);
	}
	else {
		ChaMatrixIdentity(&axismat);
		axisq.SetParams(1.0, 0.0, 0.0, 0.0);
	}


	CQuaternion invaxisq;
	axisq.inv(&invaxisq);

	CQuaternion newrot;
	if (m_anglelimit.boneaxiskind != BONEAXIS_GLOBAL){
		newrot.SetRotationXYZ(&axisq, srceul);
	}
	else{
		newrot.SetRotationXYZ(0, srceul);
	}

	ChaMatrix newlocalmat, newrotmat, befrotmat, aftrotmat;
	newrotmat = newrot.MakeRotMatX();
	ChaMatrixIdentity(&befrotmat);
	ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
	ChaMatrixIdentity(&aftrotmat);
	ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
	newlocalmat = befrotmat * newrotmat * aftrotmat;

	ChaMatrix tramat;
	ChaMatrixIdentity(&tramat);
	ChaMatrixTranslation(&tramat, srctra.x, srctra.y, srctra.z);
	newlocalmat = newlocalmat * tramat;


	ChaMatrix newmat;
	if (m_parent){
		CMotionPoint* parmp;
		parmp = m_parent->GetMotionPoint(srcmotid, srcframe);
		if (parmp){
			ChaMatrix parmat;
			parmat = parmp->GetWorldMat();
			newmat = newlocalmat * parmat;
		}
		else{
			_ASSERT(0);
			newmat = newlocalmat;
		}
	}
	else{
		newmat = newlocalmat;
	}

	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		curmp->SetWorldMat(newmat);
		curmp->SetLocalEul(srceul);

		if (setchildflag == 1){
			if (m_child){
				CQuaternion dummyq;
				m_child->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
			}
		}
	}
	else{
		_ASSERT(0);
	}

	return 0;
}


int CBone::SetLocalEul(int srcmotid, double srcframe, ChaVector3 srceul)
{
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (!curmp){
		//_ASSERT(0);
		return 1;
	}

	curmp->SetLocalEul(srceul);

	return 0;

}

ChaVector3 CBone::GetLocalEul(int srcmotid, double srcframe)
{
	ChaVector3 reteul;
	CMotionPoint* curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp) {
		reteul = curmp->GetLocalEul();
	}
	else {
		reteul.x = 0.0;
		reteul.y = 0.0;
		reteul.z = 0.0;
	}

	return reteul;
}

//onlycheck = 0
int CBone::SetWorldMat(int setchildflag, int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck)
{
	//if pose is change, return 1 else return 0
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (!curmp){
		return 0;
	}

	int ismovable = 0;

	if ((g_wmatDirectSetFlag == false) && (g_underRetargetFlag == false)){
		ChaMatrix saveworldmat;
		saveworldmat = curmp->GetWorldMat();

		ChaVector3 oldeul = ChaVector3(0.0f, 0.0f, 0.0f);
		int paraxsiflag = 1;
		int isfirstbone = 0;
		oldeul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);


		curmp->SetWorldMat(srcmat);//tmp time
		ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
		neweul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);

		curmp->SetWorldMat(saveworldmat);

		ismovable = ChkMovableEul(neweul);
		OutputToInfoWnd(L"CBone::SetWorldMat : %s : neweul [%f, %f, %f] : ismovable %d", GetWBoneName(), neweul.x, neweul.y, neweul.z, ismovable);
		
		if (onlycheck == 0) {
			if (ismovable == 1) {
				if (IsSameEul(oldeul, neweul) == 0) {
					int inittraflag0 = 0;
					SetWorldMatFromEul(inittraflag0, setchildflag, neweul, srcmotid, srcframe);//setchildflag有り!!!!
				}
				else {
					curmp->SetBefWorldMat(curmp->GetWorldMat());
				}
			}
			else {
				curmp->SetBefWorldMat(curmp->GetWorldMat());
			}
		}
	}
	else{
		ismovable = 1;
		if (onlycheck == 0) {
			//curmp->SetBefWorldMat(curmp->GetWorldMat());
			curmp->SetWorldMat(srcmat);

			ChaVector3 neweul = CalcLocalEulXYZ(-1, srcmotid, srcframe, BEFEUL_ZERO, 0);
			curmp->SetLocalEul(neweul);
		}
	}
	/*
	if (setchildflag){
		if (GetChild()){
			CQuaternion dummyq;
			GetChild()->RotBoneQReq(curmp, srcmotid, srcframe, dummyq);
		}
	}
	*/


	return ismovable;
}

int CBone::ChkMovableEul(ChaVector3 srceul)
{
	if (g_limitdegflag == false){
		return 1;//movable
	}

	int dontmove = 0;
	int axiskind;

	float chkval[3];
	chkval[0] = srceul.x;
	chkval[1] = srceul.y;
	chkval[2] = srceul.z;

	for (axiskind = AXIS_X; axiskind <= AXIS_Z; axiskind++){
		if (m_anglelimit.via180flag[axiskind] == 0){
			if ((m_anglelimit.lower[axiskind] > (int)chkval[axiskind]) || (m_anglelimit.upper[axiskind] < (int)chkval[axiskind])){
				dontmove++;
			}
		}
		else{
			if ((m_anglelimit.lower[axiskind] <= (int)chkval[axiskind]) && (m_anglelimit.upper[axiskind] >= (int)chkval[axiskind])){
				dontmove++;
			}
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
		int newval;
		newval = min((int)srcval, m_anglelimit.upper[srckind]);
		newval = max(newval, m_anglelimit.lower[srckind]);
		return (float)newval;
	}
}


ChaVector3 CBone::LimitEul(ChaVector3 srceul)
{
	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);

	reteul.x = LimitAngle(AXIS_X, srceul.x);
	reteul.y = LimitAngle(AXIS_Y, srceul.y);
	reteul.z = LimitAngle(AXIS_Z, srceul.z);

	return reteul;
}

ANGLELIMIT CBone::GetAngleLimit()
{
	::SetAngleLimitOff(&m_anglelimit);
	return m_anglelimit;
};
void CBone::SetAngleLimit(ANGLELIMIT srclimit)
{
	m_anglelimit = srclimit;

	int axiskind;
	for (axiskind = AXIS_X; axiskind < AXIS_MAX; axiskind++){
		if (m_anglelimit.lower[axiskind] < -180){
			m_anglelimit.lower[axiskind] = -180;
		}
		else if (m_anglelimit.lower[axiskind] > 180){
			m_anglelimit.lower[axiskind] = 180;
		}

		if (m_anglelimit.upper[axiskind] < -180){
			m_anglelimit.upper[axiskind] = -180;
		}
		else if (m_anglelimit.upper[axiskind] > 180){
			m_anglelimit.upper[axiskind] = 180;
		}

		if (m_anglelimit.lower[axiskind] > m_anglelimit.upper[axiskind]){
			_ASSERT(0);
			//swap
			int tmpval = m_anglelimit.lower[axiskind];
			m_anglelimit.lower[axiskind] = m_anglelimit.upper[axiskind];
			m_anglelimit.upper[axiskind] = tmpval;
		}
	}
	SetAngleLimitOff(&m_anglelimit);
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

ChaMatrix CBone::CalcSymXMat(int srcmotid, double srcframe)
{
	return CalcSymXMat2(srcmotid, srcframe, SYMROOTBONE_SYMDIR | SYMROOTBONE_SYMPOS);
}

ChaMatrix CBone::CalcSymXMat2(int srcmotid, double srcframe, int symrootmode)
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

	ChaMatrix directsetmat;
	ChaMatrixIdentity(&directsetmat);

	int rotcenterflag1 = 1;
	if (GetParent()){
		directsetmat = CalcLocalSymRotMat(rotcenterflag1, srcmotid, srcframe);
	}
	else{
		//root bone
		if (symrootmode == SYMROOTBONE_SAMEORG){
			directsetmat = GetWorldMat(srcmotid, srcframe);
			return directsetmat;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
		else if(symrootmode & SYMROOTBONE_SYMDIR){
			directsetmat = CalcLocalSymRotMat(rotcenterflag1, srcmotid, srcframe);
		}
		else{
			directsetmat = CalcLocalRotMat(rotcenterflag1, srcmotid, srcframe);
		}
	}

////tra anim
	ChaVector3 curanimtra = CalcLocalTraAnim(srcmotid, srcframe);

	if (GetParent()){
		directsetmat._41 += -curanimtra.x;//inv signe
		directsetmat._42 += curanimtra.y;
		directsetmat._43 += curanimtra.z;
	}
	else{
		//root bone
		if (symrootmode & SYMROOTBONE_SYMPOS){
			directsetmat._41 += -curanimtra.x;//inv signe
			directsetmat._42 += curanimtra.y;
			directsetmat._43 += curanimtra.z;
		}
		else{
			directsetmat._41 += curanimtra.x;//same signe
			directsetmat._42 += curanimtra.y;
			directsetmat._43 += curanimtra.z;
		}
	}

	return directsetmat;
}


ChaMatrix CBone::GetWorldMat(int srcmotid, double srcframe)
{
	ChaMatrix curmat;
	ChaMatrixIdentity(&curmat);
	CMotionPoint* curmp;
	curmp = GetMotionPoint(srcmotid, srcframe);
	if (curmp){
		curmat = curmp->GetWorldMat();
	}
	return curmat;
}

ChaMatrix CBone::CalcLocalRotMat(int rotcenterflag, int srcmotid, double srcframe)
{
	ChaMatrix curmat;
	curmat = GetWorldMat(srcmotid, srcframe);
	CMotionPoint curlocalmp;
	CalcLocalInfo(srcmotid, srcframe, &curlocalmp);
	ChaMatrix currotmat;
	currotmat = curlocalmp.GetQ().MakeRotMatX();

	currotmat._41 = 0.0f;
	currotmat._42 = 0.0f;
	currotmat._43 = 0.0f;

	if (rotcenterflag == 1){
		ChaMatrix befrotmat, aftrotmat;
		ChaMatrixIdentity(&befrotmat);
		ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
		ChaMatrixIdentity(&aftrotmat);
		ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
		currotmat = befrotmat * currotmat * aftrotmat;
	}

	return currotmat;
}


ChaMatrix CBone::CalcLocalSymRotMat(int rotcenterflag, int srcmotid, double srcframe)
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
			symbone->CalcLocalInfo(srcmotid, srcframe, &symlocalmp);
			retmat = symlocalmp.GetQ().CalcSymX2();

			retmat._41 = 0.0f;
			retmat._42 = 0.0f;
			retmat._43 = 0.0f;

			if (rotcenterflag == 1){
				ChaMatrix befrotmat, aftrotmat;
				ChaMatrixIdentity(&befrotmat);
				ChaMatrixTranslation(&befrotmat, -GetJointFPos().x, -GetJointFPos().y, -GetJointFPos().z);
				ChaMatrixIdentity(&aftrotmat);
				ChaMatrixTranslation(&aftrotmat, GetJointFPos().x, GetJointFPos().y, GetJointFPos().z);
				retmat = befrotmat * retmat * aftrotmat;
			}
		}
		else{
			retmat = CalcLocalRotMat(rotcenterflag, srcmotid, srcframe);
			_ASSERT(0);
		}
	}
	else{
		retmat = CalcLocalRotMat(rotcenterflag, srcmotid, srcframe);
		_ASSERT(0);
	}

	return retmat;
}

ChaVector3 CBone::CalcLocalTraAnim(int srcmotid, double srcframe)
{

	ChaMatrix curmat;
	curmat = GetWorldMat(srcmotid, srcframe);

	int rotcenterflag1 = 1;
	ChaMatrix curlocalrotmat, invcurlocalrotmat;
	curlocalrotmat = CalcLocalRotMat(rotcenterflag1, srcmotid, srcframe);
	ChaMatrixInverse(&invcurlocalrotmat, NULL, &curlocalrotmat);
	ChaMatrix parmat, invparmat;
	ChaMatrixIdentity(&parmat);
	invparmat = parmat;
	if (GetParent()){
		parmat = GetParent()->GetWorldMat(srcmotid, srcframe);
		ChaMatrixInverse(&invparmat, NULL, &parmat);
	}

	ChaMatrix curmvmat;
	curmvmat = invcurlocalrotmat * curmat * invparmat;

	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 curanimtra;
	ChaVector3TransformCoord(&curanimtra, &zeropos, &curmvmat);

	return curanimtra;
}


int CBone::PasteMotionPoint(int srcmotid, double srcframe, CMotionPoint srcmp)
{
	CMotionPoint* newmp = 0;
	newmp = GetMotionPoint(srcmotid, srcframe);
	if (newmp){
		ChaMatrix setmat = srcmp.GetWorldMat();

		CBone* parentbone = GetParent();
		if (parentbone){
			CMotionPoint* parmp = parentbone->GetMotionPoint(srcmotid, srcframe);
			if (parmp){
				setmat = setmat * parmp->GetWorldMat();
			}
		}

		int setmatflag1 = 1;
		CQuaternion dummyq;
		ChaVector3 dummytra = ChaVector3(0.0f, 0.0f, 0.0f);
		g_underRetargetFlag = true;
		RotBoneQReq(0, srcmotid, srcframe, dummyq, 0, dummytra, setmatflag1, &setmat);
		g_underRetargetFlag = false;

		//オイラー角初期化
		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
		int paraxsiflag = 1;
		int isfirstbone = 0;
		cureul = CalcLocalEulXYZ(paraxsiflag, srcmotid, srcframe, BEFEUL_ZERO, isfirstbone);
		SetLocalEul(srcmotid, srcframe, cureul);

	}

	return 0;
}

ChaVector3 CBone::CalcFBXEul(int srcmotid, double srcframe, ChaVector3* befeulptr)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);
	int isfirstbone;
	if (GetParent()){
		isfirstbone = 0;
	}
	else{
		isfirstbone = 1;
	}

	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	if (befeulptr){
		befeul = *befeulptr;
	}

	//tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);
	tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);

	return cureul;

}
ChaVector3 CBone::CalcFBXEulZXY(int srcmotid, double srcframe, ChaVector3* befeulptr)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);
	int isfirstbone;
	if (GetParent()){
		isfirstbone = 0;
	}
	else{
		isfirstbone = 1;
	}

	ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	if (befeulptr){
		befeul = *befeulptr;
	}

	//tmpmp.GetQ().CalcFBXEul(0, befeul, &cureul, isfirstbone);
	tmpmp.GetQ().CalcFBXEulZXY(0, befeul, &cureul, isfirstbone);

	return cureul;

}
ChaVector3 CBone::CalcFBXTra(int srcmotid, double srcframe)
{
	CMotionPoint tmpmp;
	CalcLocalInfo(srcmotid, srcframe, &tmpmp);

	ChaVector3 orgtra;
	CBone* parentbone = GetParent();
	if (parentbone){
		orgtra = GetJointFPos() - parentbone->GetJointFPos();
	}
	else{
		orgtra = GetJointFPos();
	}

	ChaVector3 fbxtra = orgtra + tmpmp.GetTra();
	return fbxtra;

}

int CBone::QuaternionInOrder(int srcmotid, double srcframe, CQuaternion* srcdstq)
{
	CQuaternion beflocalq;
	CMotionPoint befmp;
	double befframe = srcframe - 1.0;
	if (befframe >= 0.0001){
		CalcLocalInfo(srcmotid, befframe, &befmp);
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

int CBone::CalcNewBtMat(CModel* srcmodel, CRigidElem* srcre, CBone* childbone, ChaMatrix* dstmat, ChaVector3* dstpos)
{
	ChaMatrixIdentity(dstmat);
	*dstpos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (!childbone || !dstmat || !dstpos){
		return 1;
	}

	ChaVector3 jointfpos;
	ChaMatrix firstmat;
	ChaMatrix invfirstmat;
	ChaMatrix curworld;
	ChaMatrix befworld;
	ChaMatrix invbefworld;
	ChaMatrix diffworld;
	ChaVector3 rigidcenter;
	ChaMatrix multmat;
	ChaMatrix tramat;


	//firstmat = GetFirstMat();
	firstmat = GetCurrentZeroFrameMat(0);
	ChaMatrixInverse(&invfirstmat, NULL, &firstmat);

	ChaMatrix befbtmat;
	if (GetBtFlag() == 0){
		//再帰処理中のまだ未セットの状態の場合
		befbtmat = GetBtMat();
	}
	else{
		//再帰処理中のすでにセットした状態の場合
		befbtmat = GetBefBtMat();
	}

	//current
	if (GetBtKinFlag() == 1){
		diffworld = invfirstmat * GetCurMp().GetWorldMat();
		tramat = GetCurMp().GetWorldMat();

		jointfpos = GetJointFPos();
		ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
		jointfpos = childbone->GetJointFPos();
		ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);

	}
	else{
		//シミュ結果をそのまま。アニメーションは考慮しなくてよい。
		if (srcmodel->GetBtCnt() == 0){
			diffworld = invfirstmat * GetCurMp().GetWorldMat();
			tramat = GetCurMp().GetWorldMat();

			jointfpos = GetJointFPos();
			ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
			jointfpos = childbone->GetJointFPos();
			ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
		}
		else{
			if (GetParent() && (GetParent()->GetBtKinFlag() == 1)){
				//ここでのBtMatは一回前の姿勢。

				//BtMatにアニメーションの移動成分のみを掛けたものを新しい姿勢行列として子供ジョイント位置を計算してシミュレーションに使用する。
				curworld = GetCurMp().GetWorldMat();
				//befworld = GetCurMp().GetBefWorldMat();
				befworld = GetCurrentZeroFrameMat(0);
				
				ChaVector3 befparentpos, curparentpos;
				jointfpos = GetJointFPos();
				ChaVector3TransformCoord(&befparentpos, &jointfpos, &befworld);
				ChaVector3TransformCoord(&curparentpos, &jointfpos, &curworld);
				ChaVector3 diffmv = curparentpos - befparentpos;

				ChaMatrix diffmvmat;
				ChaMatrixIdentity(&diffmvmat);
				ChaMatrixTranslation(&diffmvmat, diffmv.x, diffmv.y, diffmv.z);

				ChaMatrixInverse(&invbefworld, NULL, &befworld);
				ChaMatrix newtramat = befbtmat * diffmvmat;

				diffworld = invfirstmat * newtramat;

				m_btparentpos = curparentpos;
				jointfpos = childbone->GetJointFPos();
				//ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &befbtmat);
				ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &newtramat);
			}
			else{
				diffworld = invfirstmat * befbtmat;
				tramat = befbtmat;

				jointfpos = GetJointFPos();
				ChaVector3TransformCoord(&m_btparentpos, &jointfpos, &tramat);
				jointfpos = childbone->GetJointFPos();
				ChaVector3TransformCoord(&m_btchildpos, &jointfpos, &tramat);
			}
		}
	}

	//multmat = srcre->GetBindcapsulemat() * diffworld;
	CBtObject* curbto = GetBtObject(childbone);
	if (curbto) {
		multmat = curbto->GetFirstTransformMatX() * diffworld;
	}
	else {
		multmat = GetCurrentZeroFrameMat(0) * diffworld;
		_ASSERT(0);
	}
	rigidcenter = (m_btparentpos + m_btchildpos) * 0.5f;

	*dstmat = multmat;
	*dstpos = rigidcenter;

	return 0;
}


ChaVector3 CBone::GetChildWorld(){
	if (g_previewFlag != 5){
		ChaVector3TransformCoord(&m_childworld, &m_jointfpos, &m_curmp.GetWorldMat());
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

CModel* CBone::GetCurColDisp(CBone* childbone)
{
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

int CBone::SetCurrentMotion(int srcmotid)
{
	SetCurMotID(srcmotid);
	//ResetMotionCache();
	return 0;
}

//current motionのframe 0のworldmat
ChaMatrix CBone::GetCurrentZeroFrameMatFunc(int updateflag, int inverseflag)
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
				m_firstgetmatrix = pcur->GetWorldMat();
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


ChaMatrix CBone::GetCurrentZeroFrameMat(int updateflag)
{
	//ZeroFrameの編集前と編集後のポーズのdiffをとる必要がある場合に対応する
	//updateflagが1の場合に最新情報。0の場合に前回の取得情報と同じものを返す。

	int inverseflag = 0;
	return GetCurrentZeroFrameMatFunc(updateflag, inverseflag);

}

ChaMatrix CBone::GetCurrentZeroFrameInvMat(int updateflag)
{
	//ZeroFrameの編集前と編集後のポーズのdiffをとる必要がある場合に対応する
	//updateflagが1の場合に最新情報。0の場合に前回の取得情報と同じものを返す。

	int inverseflag = 1;
	return GetCurrentZeroFrameMatFunc(updateflag, inverseflag);
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
	curpoollen = s_bonepool.size();

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


	s_befheadno = s_bonepool.size() - 1;
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
	int boneallocnum = s_bonepool.size();
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
	curpoollen = s_bonepool.size();

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
