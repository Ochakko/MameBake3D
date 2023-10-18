#include "stdafx.h"
#include <windows.h>
#include <math.h>


//#ifdef CONVD3DX11
//#include <d3dcommon.h>
//#include <dxgi.h>
//#include <d3d10_1.h>
//#include <d3d10.h>
//#include <d3dcompiler.h>
//#include <d3dx10.h>
//#endif

#include <ChaCalcFunc.h>
#include <GlobalVar.h>
#include <Model.h>
#include <Bone.h>
#include <MotionPoint.h>
#include <EditRange.h>
#include <RIgidElem.h>
#include <Collision.h>

#define DBGH
#include <dbg.h>

#include "../Examples/CommonInterfaces/CommonExampleInterface.h"
#include "../Examples/CommonInterfaces/CommonGUIHelperInterface.h"
#include "../BTSRC/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "../BTSRC/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "../BTSRC/BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h"


#include "../BTSRC/LinearMath/btTransform.h"
#include "../BTSRC/LinearMath/btHashMap.h"
//#include "../Examples/MultiThreadedDemo/CommonRigidBodyMTBase.h"
//#include "../Examples/CommonInterfaces/CommonMultiBodyBase.h"

#include "../Examples/CommonInterfaces/CommonRigidBodyBase.h"
#include "../Examples/MultiThreadedDemo/CommonRigidBodyMTBase.h"


#include "../BTSRC/btBulletDynamicsCommon.h"
#include "../BTSRC/LinearMath/btIDebugDraw.h"

#include <math.h>

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

#include <crtdbg.h>

using namespace std;


//extern bool g_wmatDirectSetFlag;//!!!!!!!!!!!!
extern bool g_underIKRot;
extern bool g_x180flag;
extern CRITICAL_SECTION g_CritSection_FbxSdk;


#define EULLIMITPLAY	1


int ChaCalcFunc::ModifyEuler360(ChaVector3* eulerA, ChaVector3* eulerB, int notmodify180flag, float throundX, float throundY, float throundZ)
{
	//#########################################################
	// 2022/12/04
	//+-180dgree�ɐ��������Ɂ@�I�C���[�p��A�������邽�߂̊֐�
	//#########################################################

	//###########################################################################################
	//2023/02/04
	//������O�̂��Ƃ����@XYZEul(180, 0, 180)��XYZEul(0, 0, 0)�͈Ⴄ�p��
	//360�x�̃v���X�}�C�i�X�͗L�肾���@180�x�̃v���X�}�C�i�X�́@�Ⴄ�p���ɂ��邱��
	//�m�C�Y�΍�Ƃ���+-180�x�͗L�蓾�邪
	//�����p���̕ʕ\���Ƃ��Ă�+-180�x�́@XYZEul(0, 180, 0)��XYZEul(180, 0, 180)�ɂ���ȊO�Ɏv�����Ȃ�
	//360�̃v���X�}�C�i�X�ɖ߂��ā@�㏈���Ƃ��ĕ␳���s��
	//###########################################################################################


	float tmpX0, tmpY0, tmpZ0;
	if (notmodify180flag == 0) {
		if (eulerA->x >= 0.0f) {
			tmpX0 = eulerA->x + 360.0f * GetRoundThreshold((eulerB->x - eulerA->x) / 360.0f, throundX);
		}
		else {
			tmpX0 = eulerA->x - 360.0f * GetRoundThreshold((eulerA->x - eulerB->x) / 360.0f, throundX);
		}
		if (eulerA->y >= 0.0f) {
			tmpY0 = eulerA->y + 360.0f * GetRoundThreshold((eulerB->y - eulerA->y) / 360.0f, throundY);
		}
		else {
			tmpY0 = eulerA->y - 360.0f * GetRoundThreshold((eulerA->y - eulerB->y) / 360.0f, throundY);
		}
		if (eulerA->z >= 0.0f) {
			tmpZ0 = eulerA->z + 360.0f * GetRoundThreshold((eulerB->z - eulerA->z) / 360.0f, throundZ);
		}
		else {
			tmpZ0 = eulerA->z - 360.0f * GetRoundThreshold((eulerA->z - eulerB->z) / 360.0f, throundZ);
		}


		////�p�x�ω��̑傫��
		//double s0 = ((double)eulerB->x - eulerA->x) * ((double)eulerB->x - eulerA->x) +
		//	((double)eulerB->y - eulerA->y) * ((double)eulerB->y - eulerA->y) +
		//	((double)eulerB->z - eulerA->z) * ((double)eulerB->z - eulerA->z);
		//double s1 = ((double)eulerB->x - tmpX0) * ((double)eulerB->x - tmpX0) +
		//	((double)eulerB->y - tmpY0) * ((double)eulerB->y - tmpY0) +
		//	((double)eulerB->z - tmpZ0) * ((double)eulerB->z - tmpZ0);
		//
		//if (s0 <= s1) {
		//	//���̂܂�
		//}
		//else {
		//	eulerA->x = tmpX0;
		//	eulerA->y = tmpY0;
		//	eulerA->z = tmpZ0;
		//}

		eulerA->x = tmpX0;
		eulerA->y = tmpY0;
		eulerA->z = tmpZ0;
	}
	else {
		//���̂܂�
	}

	////############################################################################################
	////Q2EulXYZ��axisq���w�肵�ČĂяo�����ꍇ
	////invaxisq * *this * axisq�ɂ���ā@�P�W�O�x���I�C���[�p����]���邱�Ƃ�����̂ő΍�
	//// �������@befframe��0�t���[���̏ꍇ�ɂ́@�P�W�O�x����]�`�F�b�N�͂��Ȃ�(�P�W�O�x��]������)
	////############################################################################################
	//if (notmodify180flag == 0) {
	//	float thdeg = 165.0f;
	//	if ((tmpX0 - eulerB->x) >= thdeg) {
	//		tmpX0 -= 180.0f;
	//	}
	//	if ((eulerB->x - tmpX0) >= thdeg) {
	//		tmpX0 += 180.0f;
	//	}

	//	if ((tmpY0 - eulerB->y) >= thdeg) {
	//		tmpY0 -= 180.0f;
	//	}
	//	if ((eulerB->y - tmpY0) >= thdeg) {
	//		tmpY0 += 180.0f;
	//	}

	//	if ((tmpZ0 - eulerB->z) >= thdeg) {
	//		tmpZ0 -= 180.0f;
	//	}
	//	if ((eulerB->z - tmpZ0) >= thdeg) {
	//		tmpZ0 += 180.0f;
	//	}
	//}


	return 0;

	return 0;
}
int ChaCalcFunc::GetRoundThreshold(float srcval, float degth)
{
	//GetRound()�ł́@180�x�ȏ�̂�����P��]�ŕ␳���Ă���
	//ChaGetRoundThreshold()�ł́@�P��]���ǂꂾ���������p�x�ň��]�Ƃ݂Ȃ���(�g�����ő����̂͂R�U�O�x�P�ʂȂ̂Ŏp���͕ς��Ȃ�)���w�肷��(�����Ƃ�)

	float th360;
	th360 = degth / 360.0f;

	if (srcval > 0.0f) {
		return (int)(srcval + th360);
	}
	else {
		return (int)(srcval - th360);
	}

	return 0;
}
int ChaCalcFunc::GetBefNextMP(CBone* srcbone, int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr, bool onaddmotion)
{
	if (!srcbone || !ppbef || !ppnext || !existptr) {
		_ASSERT(0);
		return 1;
	}

	//EnterCriticalSection(&m_CritSection_GetBefNext);

	//2023/04/28 2023/05/23
	if (srcbone->IsNotSkeleton() && srcbone->IsNotCamera() && srcbone->IsNotNull()) {
		if (ppbef) {
			*ppbef = 0;
		}
		if (ppnext) {
			*ppnext = 0;
		}
		if (existptr) {
			*existptr = 0;
		}
		return 0;
	}



	CMotionPoint* pbef = 0;
	//CMotionPoint* pcur = m_motionkey[srcmotid -1];
	CMotionPoint* pcur = 0;
	//std::map<int, std::vector<CMotionPoint*>>::iterator itrvecmpmap;


	int curframeindex = IntTime(srcframe);
	int nextframeindex = curframeindex + 1;
	int mpmapleng = 0;//2022/11/01 STL��size()�͏d���炵���̂ŕϐ��ɑ�����Ďg���܂킵

	*existptr = 0;

	if ((srcmotid <= 0) || (srcmotid > srcbone->GetMotionKeySize())) {
		//AddMotionPoint����Ă΂��Ƃ��ɒʂ�ꍇ�͐���
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

			//if (srcmotid >= 1) {
			//	m_cachebefmp[srcmotid - 1] = NULL;
			//}
			////_ASSERT(0);
			////LeaveCriticalSection(&m_CritSection_GetBefNext);
			return 0;
		}
		else {
			pcur = srcbone->GetMotionKey(srcmotid);
		}
	}

	bool getbychain;
	getbychain = onaddmotion;


	//if (getbychain == false) {
	//	//get by indexed �̃t���O�w��̏ꍇ�ɂ�indexed�̏������o���Ă��Ȃ��ꍇ��get by chain�Ŏ擾����
	//	if (srcbone->GetInitIndexedMotionPointSize() <= srcmotid) {//�G���g���[���܂������Ƃ�
	//		getbychain = true;
	//	}
	//	else {
	//		getbychain = !(srcbone->ExistInitIndexedMotionPoint(srcmotid));
	//	}
	//}
	//if (getbychain == false) {
	//	//index��frame���̃`�F�b�N
	//	mpmapleng = srcbone->GetIndexedMotionPointFrameLeng(srcmotid);
	//	if ((mpmapleng <= 0) || (curframeindex >= mpmapleng)) {// ##### 2023/10/17_1  "&&" �ɂȂ��Ă��� #####
	//		getbychain = true;
	//	}
	//}


	std::vector<CMotionPoint*> mpvec;
	mpvec.clear();
	srcbone->GetIndexedMotionPointVec(srcmotid, mpvec);
	if (getbychain == false) {
		if (mpvec.empty()) {
			getbychain = true;
		}
		else {
			mpmapleng = (int)mpvec.size();
			if ((mpmapleng <= 0) || (curframeindex >= mpmapleng)) {
				getbychain = true;
			}
		}
	}


	if (getbychain == true) {
		//#ifdef USE_CACHE_ONGETMOTIONPOINT__
		//		//�L���b�V�����`�F�b�N����
		//		if ((srcmotid >= 1) && (srcmotid <= MAXMOTIONNUM) && m_cachebefmp[srcmotid - 1] &&
		//			((m_cachebefmp[srcmotid - 1])->GetUseFlag() == 1) &&
		//			//((m_cachebefmp[srcmotid - 1])->GetFrame() <= (srcframe + 0.0001))) {
		//			((m_cachebefmp[srcmotid - 1])->GetFrame() <= ((double)curframeindex + 0.0001))) {//2022/12/26
		//			//�������̂��ߓr������̌����ɂ���
		//			pcur = m_cachebefmp[srcmotid - 1];
		//		}
		//#endif

		while (pcur) {

			if (IsJustEqualTime(pcur->GetFrame(), srcframe)) {//�W���X�g����@�W���X�g�̏ꍇ��Ԗ���
				//if ((pcur->GetFrame() >= ((double)curframeindex - 0.0001)) && (pcur->GetFrame() <= ((double)curframeindex + 0.0001))) {//2022/12/26 ����ł͕�Ԃ������Ȃ�
				*existptr = 1;
				pbef = pcur;
				break;
			}
			else if (pcur->GetFrame() > srcframe) {//�߂��Ă��܂����ꍇ
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
		*ppbef = pbef;//�߂��Ă��܂����P�O�̃��[�V�����|�C���g���Z�b�g

		if (*existptr) {
			*ppnext = pbef->GetNext();
		}
		else {
			*ppnext = pcur;
		}

	}
	else {

		//### 2022/11/01 ################
		//�ŏ��̕��Ń`�F�b�N�ςȂ̂ŕs�v
		//###############################
		//if ((srcmotid <= 0) || (srcmotid > m_indexedmotionpoint.size())) {
		//	//AddMotionPoint����Ă΂��Ƃ��ɒʂ�ꍇ�͐���
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
			//*ppbef = srcbone->GetIndexedMotionPoint(srcmotid, curframeindex);
			*ppbef = mpvec[curframeindex];
		}
		else {
			if (mpmapleng >= 1) {
				//*ppbef = srcbone->GetIndexedMotionPoint(srcmotid, mpmapleng - 1);
				*ppbef = mpvec[mpmapleng - 1];
			}
			else {
				*ppbef = 0;
			}
		}

		if (*ppbef) {
			double mpframe = (*ppbef)->GetFrame();
			//if ((mpframe >= ((double)curframeindex - 0.0001)) && (mpframe <= ((double)curframeindex + 0.0001))) {

			//2023/08/02
			//��Ԍv�Z���ɂ�
			//GetBefNextMP�ɂ�(m_curmp)�ȊO�̏ꍇ�@�[���݂�̎��Ԃ�n���@just�̌v�Z���[���݂�ł���
			//if�����ȉ��̂悤�ɂ��Ȃ��Ɓ@���[�V�����ɂ���Ắ@0.007�{���ȂǂŃJ�N�J�N����@�ύX�O�ł����[�V�������Ԃ����܂���int�̏ꍇ�ɂ͊��炩������
			//��͕̂�Ԍv�Z���̘b�@���[�V�����̃L�[�̎��Ԃ�int�ɑ����ă��T���v�����O���ēǂݍ���ł���
			//���[�V�����f�[�^���C���f�b�N�X�����Ă��Ȃ��ꍇ�̊��炩�����̏C���́@��̕��̃R�[�h��2022/12/26�ɏC���ρ@����̏C���̓C���f�b�N�X�����ꂽ�f�[�^�ɂ��Ă̏C��
			if (IsJustEqualTime(mpframe, srcframe)) {
				*existptr = 1;
			}
			else {
				*existptr = 0;
			}


			if (nextframeindex < mpmapleng) {
				//*ppnext = srcbone->GetIndexedMotionPoint(srcmotid, nextframeindex);
				*ppnext = mpvec[nextframeindex];
			}
			else {
				if (mpmapleng >= 1) {
					//*ppnext = srcbone->GetIndexedMotionPoint(srcmotid, mpmapleng - 1);
					*ppnext = mpvec[mpmapleng - 1];
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


	//#ifdef USE_CACHE_ONGETMOTIONPOINT__
	//	//m_cachebefmp = pbef;
	//	if ((srcmotid >= 1) && (srcmotid <= MAXMOTIONNUM)) {
	//		if (*ppbef) {
	//			if ((*ppbef)->GetPrev()) {
	//				m_cachebefmp[srcmotid - 1] = (*ppbef)->GetPrev();
	//			}
	//			else {
	//				m_cachebefmp[srcmotid - 1] = (*ppbef);
	//			}
	//		}
	//		else {
	//			//m_cachebefmp[srcmotid - 1] = m_motionkey[srcmotid - 1];
	//			m_cachebefmp[srcmotid - 1] = NULL;
	//		}
	//	}
	//#endif


		//LeaveCriticalSection(&m_CritSection_GetBefNext);


	return 0;
}

int ChaCalcFunc::IKRotateOneFrame(CModel* srcmodel, int limitdegflag, CEditRange* erptr,
	int keyno, CBone* rotbone, CBone* parentbone,
	int srcmotid, double curframe, double startframe, double applyframe,
	CQuaternion rotq0, bool keynum1flag, bool postflag, bool fromiktarget)
{

	//for return value
	int ismovable = 1;


	if (!srcmodel || !erptr || !rotbone || !parentbone) {
		_ASSERT(0);
		return 0;//not move
	}

	if (rotbone->IsNotSkeleton()) {
		return 0;//not move
	}

	CQuaternion qForRot;
	CQuaternion qForHipsRot;

	if (fromiktarget == true) {
		qForRot = rotq0;
		qForHipsRot = rotq0;

		//bool calcaplyflag = false;
		//CalcQForRot(limitdegflag, calcaplyflag, 
		//	srcmotid, curframe, applyframe, rotq0,
		//	parentbone, parentbone,
		//	&qForRot, &qForHipsRot);

		//IKTraget�̏ꍇ�ɂ�
		//0.080�ō���Ł@���X�ɋ߂Â���
		//�߂Â�������Ȃ��ꍇ�́@������Ɂ@ConstExecute�{�^��������
		CQuaternion endq;
		CQuaternion curqForRot;
		CQuaternion curqForHipsRot;
		endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		qForRot.Slerp2(endq, 0.080f, &curqForRot);
		curqForHipsRot = curqForRot;
		bool infooutflag = true;
		ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
			infooutflag, 0, srcmotid, curframe, curqForRot, curqForHipsRot, fromiktarget);

		//bool infooutflag = true;
		//parentbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
		//	infooutflag, 0, srcmotid, curframe, qForRot, qForHipsRot, fromiktarget);

	}
	else if (keynum1flag) {
		qForRot = rotq0;
		qForHipsRot = rotq0;

		//bool calcaplyflag = false;
		//CalcQForRot(limitdegflag, calcaplyflag, 
		//	srcmotid, curframe, applyframe, rotq0,
		//	parentbone, parentbone,
		//	&qForRot, &qForHipsRot);

		bool infooutflag = true;
		ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
			infooutflag, 0, srcmotid, curframe, qForRot, qForHipsRot, fromiktarget);

		if ((fromiktarget != true) && (postflag != true)) {
			IKTargetVec(srcmodel, limitdegflag, erptr, srcmotid, curframe, postflag);
		}
	}
	else {
		if (g_pseudolocalflag == 1) {
			bool calcaplyflag = true;
			CalcQForRot(limitdegflag, calcaplyflag,
				srcmotid, curframe, applyframe, rotq0,
				rotbone, parentbone,
				&qForRot, &qForHipsRot);
		}
		else {
			qForRot = rotq0;
			qForHipsRot = rotq0;
		}

		double changerate;
		if (fromiktarget == false) {
			changerate = (double)(*(g_motionbrush_value + (int)curframe));
		}
		else {
			changerate = 1.0;
		}


		bool infooutflag;
		if (curframe == applyframe) {
			infooutflag = true;
		}
		else {
			infooutflag = false;
		}

		//double firstframe = 0.0;
		//if (keyno == 0) {
		//	firstframe = curframe;
		//}
		//if (g_absikflag == 0) {
		if (g_slerpoffflag == 0) {
			CQuaternion endq;
			CQuaternion curqForRot;
			CQuaternion curqForHipsRot;
			endq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
			qForRot.Slerp2(endq, 1.0 - changerate, &curqForRot);
			qForHipsRot.Slerp2(endq, 1.0 - changerate, &curqForHipsRot);

			ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
				infooutflag, 0, srcmotid, curframe, curqForRot, curqForHipsRot, fromiktarget);
		}
		else {
			ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
				infooutflag, 0, srcmotid, curframe, qForRot, qForHipsRot, fromiktarget);
		}
		//}
		//else {
		//	if (keyno == 0) {
		//		ismovable = rotbone->RotAndTraBoneQReq(limitdegflag, 0, RoundingTime(startframe),
		//			infooutflag, 0, srcmotid, curframe, qForRot, qForHipsRot, fromiktarget);
		//	}
		//	else {
		//		rotbone->SetAbsMatReq(limitdegflag, 0, srcmotid, curframe, firstframe);
		//	}
		//}


		if ((fromiktarget != true) && (postflag != true)) {
			IKTargetVec(srcmodel, limitdegflag, erptr, srcmotid, curframe, postflag);
		}
	}

	return ismovable;
}


int ChaCalcFunc::CalcQForRot(bool limitdegflag, bool calcaplyflag,
	int srcmotid, double srcframe, double srcapplyframe, CQuaternion srcaddrot,
	CBone* srcrotbone, CBone* srcaplybone,
	CQuaternion* dstqForRot, CQuaternion* dstqForHipsRot)
{
	if (!srcrotbone || !srcaplybone || !dstqForRot || !dstqForHipsRot) {
		_ASSERT(0);
		return 1;
	}

	if (srcrotbone->IsNotSkeleton()) {
		dstqForRot->SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		dstqForHipsRot->SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		return 0;
	}

	double roundingframe = RoundingTime(srcframe);
	double roundingapplyframe = RoundingTime(srcapplyframe);

	//ChaMatrix curparrotmat = curparmp->GetWorldMat();
	ChaMatrix curparrotmat = srcrotbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	curparrotmat.SetTranslationZero();
	//ChaMatrix invcurparrotmat = curparmp->GetInvWorldMat();
	ChaMatrix invcurparrotmat = ChaMatrixInv(curparrotmat);
	invcurparrotmat.SetTranslationZero();

	//ChaMatrix aplyparrotmat = aplyparmp->GetWorldMat();
	ChaMatrix aplyparrotmat = srcaplybone->GetWorldMat(limitdegflag, srcmotid, roundingapplyframe, 0);
	aplyparrotmat.SetTranslationZero();
	//ChaMatrix invaplyparrotmat = aplyparmp->GetInvWorldMat();
	ChaMatrix invaplyparrotmat = ChaMatrixInv(aplyparrotmat);
	invaplyparrotmat.SetTranslationZero();

	CQuaternion invcurparrotq, aplyparrotq, invaplyparrotq, curparrotq;
	invcurparrotq.RotationMatrix(invcurparrotmat);
	aplyparrotq.RotationMatrix(aplyparrotmat);
	invaplyparrotq.RotationMatrix(invaplyparrotmat);
	curparrotq.RotationMatrix(curparrotmat);


	//�Ӗ��FRotBoneQReq()��rotq��n���@currentworldmat�̌��Ɂ@invpivot * rotq * pivot���|����
	//�܂�@A = currentworldmat, B = localq.MakeRotMatX()�Ƃ���� A * (invA * B * A)
	ChaMatrix transmat2ForRot;
	ChaMatrix transmat2ForHipsRot;

	//hisp�ړ��͂��܂��������@��]���������� �@hips�ȊO�͗ǂ�
	//transmat2 = invcurparrotmat * aplyparrotmat * localq.MakeRotMatX() * invaplyparrotmat * curparrotmat;//bef

	//hips��]�͂��܂��������@�ړ�����������
	//transmat2 = localq.MakeRotMatX();//for hips edit

	//####################################################################
	//ToDo : RotQBoneReq2()������ā@�����Ƃ��ď�L�Q�̉�]����n��
	//####################################################################

	if (calcaplyflag == true) {
		transmat2ForRot = invcurparrotmat * aplyparrotmat * srcaddrot.MakeRotMatX() * invaplyparrotmat * curparrotmat;//bef
	}
	else {
		transmat2ForRot = invcurparrotmat * srcaddrot.MakeRotMatX() * curparrotmat;//bef
	}

	transmat2ForHipsRot = srcaddrot.MakeRotMatX();//for hips edit

	dstqForRot->RotationMatrix(transmat2ForRot);
	dstqForHipsRot->RotationMatrix(transmat2ForHipsRot);



	return 0;

}

bool ChaCalcFunc::CalcAxisAndRotForIKRotateAxis(CModel* srcmodel, int limitdegflag,
	CBone* parentbone, CBone* firstbone,
	int srcmotid, double curframe, ChaVector3 targetpos,
	ChaVector3 srcikaxis,
	ChaVector3* dstaxis, float* dstrotrad)
{
	//return nearflag : too near to move


	//########################################################
	//2023/03/24
	//model���W�n�Ōv�Z�Fmodel��WorldMat�̉e���𖳂����Čv�Z
	//########################################################


	if (!srcmodel || !parentbone || !firstbone || !dstaxis || !dstrotrad) {
		_ASSERT(0);
		return true;
	}

	if (!firstbone->GetParent(false)) {
		_ASSERT(0);
		return true;
	}

	ChaMatrix invmodelwm = ChaMatrixInv(srcmodel->GetWorldMat());

	ChaVector3 ikaxis = srcikaxis;//!!!!!!!!!!!!
	ChaVector3Normalize(&ikaxis, &ikaxis);

	ChaVector3 modelparentpos, modelchildpos;
	{
		ChaVector3 parentworld;
		parentworld = parentbone->GetWorldPos(limitdegflag, srcmotid, curframe);
		ChaVector3TransformCoord(&modelparentpos, &parentworld, &invmodelwm);

		ChaMatrix parentmat = firstbone->GetParent(false)->GetWorldMat(limitdegflag, srcmotid, curframe, 0);// *GetWorldMat();
		ChaVector3 tmpfirstfpos = firstbone->GetJointFPos();
		ChaVector3TransformCoord(&modelchildpos, &tmpfirstfpos, &parentmat);
	}

	ChaVector3 childtotarget = targetpos - modelchildpos;
	double distance = ChaVector3LengthDbl(&childtotarget);
	//if (distance <= 0.10f) {
	//	return true;
	//}


	ChaVector3 parbef, chilbef, tarbef;
	parbef = modelparentpos;
	CalcShadowToPlane(modelchildpos, ikaxis, modelparentpos, &chilbef);
	CalcShadowToPlane(targetpos, ikaxis, modelparentpos, &tarbef);

	ChaVector3 vec0, vec1;
	vec0 = chilbef - parbef;
	ChaVector3Normalize(&vec0, &vec0);
	vec1 = tarbef - parbef;
	ChaVector3Normalize(&vec1, &vec1);

	ChaVector3 rotaxis2;
	ChaVector3Cross(&rotaxis2, (const ChaVector3*)&vec0, (const ChaVector3*)&vec1);
	ChaVector3Normalize(&rotaxis2, &rotaxis2);

	float rotdot2, rotrad2;
	rotdot2 = ChaVector3Dot(&vec0, &vec1);
	rotdot2 = min(1.0f, rotdot2);
	rotdot2 = max(-1.0f, rotdot2);
	rotrad2 = (float)acos(rotdot2);

	*dstaxis = rotaxis2;
	*dstrotrad = rotrad2;

	return false;
}


int ChaCalcFunc::RotAndTraBoneQReq(CBone* srcbone, bool limitdegflag, int* onlycheckptr,
	double srcstartframe, bool infooutflag, CBone* parentbone, int srcmotid, double srcframe,
	CQuaternion qForRot, CQuaternion qForHipsRot, bool fromiktarget)
{
	//######################################
	//IK�p.�@Retarget��RotBoneQReq()���g�p
	//hips��tanim�� qForHipsRot �ŉ�]����
	//hips�ȊO��tanim�� qForRot�@�ŉ�]����
	//######################################

	//###################################################################
	//onlycheckptr != NULL�̏ꍇ�ɂ�
	//SetWorldMat��onlycheck�ŌĂяo���ā@��]�\���ǂ��������𒲂ׂ�
	//����Ăяo������CBone�ɑ΂��Ă����`�F�b�N�����ā@�����Ƀ��^�[������
	//###################################################################

	if (!srcbone) {
		_ASSERT(0);
		return 1;
	}


	int ismovable = 1;//for return value

	double roundingframe = RoundingTime(srcframe);

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}

	CMotionPoint* curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		_ASSERT(0);
		return 0;
	}

	ChaMatrix currentbefwm;
	ChaMatrix currentnewwm;
	currentbefwm.SetIdentity();
	currentnewwm.SetIdentity();
	currentbefwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	//����Ăяo��
	bool ishipsjoint;
	ishipsjoint = srcbone->IsHipsBone();


	ChaMatrix currentwm;
	//limitedworldmat = GetLimitedWorldMat(srcmotid, srcframe);//������GetLimitedWorldMat�ɂ���ƂP��ڂ�IK�������B�Q��ڂ�IK�ȍ~��OK�B
	currentwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
	ChaMatrix parentwm;
	CQuaternion parentq;
	CQuaternion invparentq;
	if (srcbone->GetParent(false)) {
		parentwm = srcbone->GetParent(false)->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		parentq.RotationMatrix(parentwm);
		invparentq.RotationMatrix(ChaMatrixInv(parentwm));
	}
	else {
		parentwm.SetIdentity();
		parentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		invparentq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
	}

	//Get startframeframe traanim : SRT�ۑ���CModel::IKRotate* ����Ăяo��CBone::SaveSRT()�ōs���Ă���
	//ChaVector3 startframetraanim = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaMatrix startframetraanimmat;
	startframetraanimmat.SetIdentity();
	{
		//CMotionPoint* zeromp = GetMotionPoint(srcmotid, 0.0);
		CMotionPoint* startframemp = srcbone->GetMotionPoint(srcmotid, RoundingTime(srcstartframe));
		if (startframemp) {
			ChaMatrix smat0, rmat0, tmat0, tanimmat0;
			smat0.SetIdentity();
			rmat0.SetIdentity();
			tmat0.SetIdentity();
			tanimmat0.SetIdentity();
			//CModel::IKRotate* ����Ăяo����CBone::SaveSRT()�ŕۑ�����SRT���擾
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
	//Hips�̂Ƃ��ɂ́@�ǉ����̉�]����납��|����
	//���̍ۂ�TraAnim������납��|���邱�Ƃɂ�� TraAnim����]����
	// 
	//Hips�ȊO�̎��ɂ́@qForRot�̓��e�Ƀg���b�N�������ā@�ǉ����̉�]�� "�����I�ɂ�"�O����|���Ă���
	//���̏ꍇ�@TraAnim������ʓr��]���ăZ�b�g����K�v������
	// 
	//�Ȃ��@Hips�ȊO�̎���Hips�Ɠ����V���v���Ȏ����g���Ȃ����Ƃ�����
	//�̑S�̂���]�������ȂǂɁ@�̂ɑ΂����]�̌������ێ�����K�v������̂Ł@��납��|���邱�Ƃ��o���Ȃ�����
	//###########################################################################################################


	if (ishipsjoint == true) {

		//############
		//Hips Joint
		//############


		//#############################################################################################################################
		//2022/12/27
		//hisp�ɂ��ā@�ړ�����]����ɂ́@�ɂ���
		//InvCurNodeTra * curS * curR * CurNodeTra * TAnim * ParentWM �ɑ΂��ā@��]qForHipsRot�������@curTAnim����]����ɂ�
		//�C���[�W�Ƃ��Ắ@curwm�̐e�̈ʒu�Ɂ@qForHipsRot������������C���[�W
		//���ۂɂ�
		//curTAnim��CurNodeTra�͗����Ƃ��ړ������݂̂ł��邩����ł��邩��
		//(InvCurNodeTra * curS * curR * TAnim * CurNodeTra) * InvCurNodeTra * qForHipsRot * CurNodeTra * ParentWM
		//currentwm * InvCurNode * qForHipsRot * CurNodeTra * ParentWM
		//#############################################################################################################################
		//newwm = currentwm * ChaMatrixInv(parentwm) *
		//	ChaMatrixInv(ChaMatrixTra(GetNodeMat())) * qForHipsRot.MakeRotMatX() * ChaMatrixTra(GetNodeMat()) *
		//	parentwm;

		newwm = currentwm * ChaMatrixInv(parentwm) *
			ChaMatrixInv(ChaMatrixTra(srcbone->GetNodeMat())) * ChaMatrixInv(startframetraanimmat) * qForHipsRot.MakeRotMatX() * ChaMatrixTra(srcbone->GetNodeMat()) * startframetraanimmat *
			parentwm;

		if (onlycheckptr) {
			bool directsetflag = false;
			int onlycheckflag = 1;
			ismovable = srcbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, 0,
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
			*onlycheckptr = ismovable;
			//if (ismovable == 0) {
			//	return curmp;// not movable�̏ꍇ�́@���t���ā@�����Ƀ��^�[������
			//}
			return ismovable;//onlycheckptr != NULL�̏ꍇ�́@����Ăяo����movable�`�F�b�N���Ē����Ƀ��^�[��
		}
		else {
			bool directsetflag = false;
			int onlycheckflag = 0;
			int setchildflag = 1;
			ismovable = srcbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag,
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
		}
		currentnewwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

	}
	else {

		//###############################################
		//other joints !!!! traanim�� qForRot �ŉ�]����
		//###############################################


			////�ȉ��R�s�@hips�Ɠ����悤�ɂ���Ɓ@traanim���ݒ肵�Ă���W���C���g�Ł@��]�����}�j�s�����[�^�ƍ���Ȃ�
			//newwm = currentwm * ChaMatrixInv(parentwm) *
			//	ChaMatrixInv(ChaMatrixTra(GetNodeMat())) * ChaMatrixInv(startframetraanimmat) * qForRot.MakeRotMatX() * ChaMatrixTra(GetNodeMat()) * startframetraanimmat *
			//	parentwm;



		//########################################################################
		//2023/01/14
		//�w��Rig�Ńe�X�g�����Ƃ��� �����p�x�L��̏ꍇ�Ɂ@traanim���s���ɂȂ���
		//�Q�i�K�ɕ����Čv�Z���邱�Ƃɂ�����
		//########################################################################

		//#############################################################
		//�Q�i�K�����̂P�i�ځF��]������ύX���Ċm�肷�邽�߂́@�P�i��
		//#############################################################
			////calc new local rot
		ChaMatrix newlocalrotmatForRot;
		ChaMatrix smatForRot, rmatForRot, tmatForRot, tanimmatForRot;
		newlocalrotmatForRot.SetIdentity();
		smatForRot.SetIdentity();
		rmatForRot.SetIdentity();
		tmatForRot.SetIdentity();
		tanimmatForRot.SetIdentity();
		newlocalrotmatForRot = srcbone->CalcNewLocalRotMatFromQofIK(limitdegflag, srcmotid, roundingframe, qForRot, &smatForRot, &rmatForRot, &tanimmatForRot);

		ChaMatrix newtanimmatrotated;
		newtanimmatrotated = tanimmatForRot;//�P�i�ڂł́@traanim�� ��]���Ȃ�

		////	//traanim�� ��]���Ȃ��Ƃ�
		////	newlocalrotmatForHipsRot = newlocalrotmatForRot;
		////	newtanimmatrotated = tanimmatForRot;


		//#### SRTAnim���烍�[�J���s��g�ݗ��� ####
		ChaMatrix newlocalmat;
		newlocalmat = ChaMatrixFromSRTraAnim(true, true, srcbone->GetNodeMat(),
			&smatForRot, &newlocalrotmatForRot, &newtanimmatrotated);//ForRot
		//newwm = newlocalmat * parentwmForRot;//global�ɂ���
		if (srcbone->GetParent(false)) {
			newwm = newlocalmat * parentwm;//global�ɂ���
		}
		else {
			newwm = newlocalmat;
		}

		if (onlycheckptr) {
			bool directsetflag = false;
			int onlycheckflag = 1;
			ismovable = srcbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, 0,
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
			*onlycheckptr = ismovable;
			//if (ismovable == 0) {
			//	return curmp;// not movable�̏ꍇ�́@���t���ā@�����Ƀ��^�[������
			//}


			//onlycheck�̏ꍇ�́@�����܂�
			return ismovable;

		}
		else {
			bool directsetflag = false;
			int onlycheckflag = 0;
			int setchildflag = 1;//2023/02/12 �Q�i�ڂ̑O�ɍċA����K�v
			ismovable = srcbone->SetWorldMat(limitdegflag, directsetflag, infooutflag, setchildflag,
				srcmotid, roundingframe, newwm, onlycheckflag, fromiktarget);
		}
		currentnewwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);

		////#####################################################################################################
		////�Q�i�K�����̂Q�i�ځF�p�x�����I���I�t���l�����@��]���m�肳������@�ړ��A�j������]���邽�߂́@�Q�i��
		////#####################################################################################################


			//2023/01/22 �����p�x�ƈꏏ�Ɏg���Ɓ@���삲�ƂɌ덷���~�ς���̂Ł@�I�v�V�����ɂ���
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
			GetSRTandTraAnim(tmplocalmat, srcbone->GetNodeMat(), &smatForRot2, &rmatForRot2, &tmatForRot2, &tanimmatForRot2);


			//�Q�i�ڂł́@�m�肵����]�ɂ��traanim����]����
			ChaMatrix newtanimmatrotated2;
			newtanimmatrotated2 = srcbone->CalcNewLocalTAnimMatFromSRTraAnim(rmatForRot2,
				smatForRot, rmatForRot, tanimmatForRot, ChaMatrixTraVec(startframetraanimmat));

			////	//traanim�� ��]���Ȃ��Ƃ�
			////	newlocalrotmatForHipsRot = newlocalrotmatForRot;
			////	newtanimmatrotated = tanimmatForRot;


			//#### SRTAnim���烍�[�J���s��g�ݗ��� ####
			ChaMatrix newlocalmat2;
			newlocalmat2 = ChaMatrixFromSRTraAnim(true, true, srcbone->GetNodeMat(),
				&smatForRot, &rmatForRot2, &newtanimmatrotated2);//ForRot
			//newwm = newlocalmat * parentwmForRot;//global�ɂ���
			if (srcbone->GetParent(false)) {
				newwm = newlocalmat2 * parentwm;//global�ɂ���
			}
			else {
				newwm = newlocalmat2;
			}

			bool directsetflag2 = true;
			int setchildflag2 = 1;//2023/02/12
			int onlycheck2 = 0;
			srcbone->SetWorldMat(limitdegflag, directsetflag2, infooutflag, setchildflag2,
				srcmotid, roundingframe, newwm, onlycheck2, fromiktarget);
			currentnewwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		}

	}

	curmp->SetAbsMat(srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp));

	//2023/02/12 setchildflag = 1�ŏ������邱�Ƃ�
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

int ChaCalcFunc::IKTargetVec(CModel* srcmodel, bool limitdegflag, CEditRange* erptr, int srcmotid, double srcframe, bool postflag)
{
	if (!srcmodel || !erptr) {
		_ASSERT(0);
		return 1;
	}

	int iktargetbonevecsize = (int)srcmodel->GetIKTargetBoneVecSize();

	int targetno;
	for (targetno = 0; targetno < iktargetbonevecsize; targetno++) {
		CBone* srcbone = srcmodel->GetIKTargetBone(targetno);
		if (srcbone && srcbone->IsSkeleton() && srcbone->GetParent(false) && srcbone->GetIKTargetFlag()) {
			ChaVector3 iktargetpos = srcbone->GetIKTargetPos();//model���W�n
			int calccount;
			const int calccountmax = 30;
			for (calccount = 0; calccount < calccountmax; calccount++) {
				int maxlevel = 0;
				IKRotateForIKTarget(srcmodel, limitdegflag, erptr, srcbone->GetBoneNo(), srcmotid,
					iktargetpos, maxlevel, srcframe, postflag);
			}
		}
	}

	return 0;
}

int ChaCalcFunc::IKRotateForIKTarget(CModel* srcmodel, bool limitdegflag, CEditRange* erptr,
	int srcboneno, int srcmotid, ChaVector3 targetpos, int maxlevel, double directframe, bool postflag)
{
	if (!srcmodel) {
		_ASSERT(0);
		return 1;
	}


	double curframe = directframe;

	CBone* firstbone = srcmodel->GetBoneByID(srcboneno);
	if (!firstbone) {
		_ASSERT(0);
		//g_underIKRot = false;//2023/01/14 parent limited or not
		return -1;
	}
	if (firstbone->IsNotSkeleton()) {
		return -1;
	}


	bool ishipsjoint = false;
	if (firstbone->GetParent(false)) {
		ishipsjoint = firstbone->GetParent(false)->IsHipsBone();
	}
	else {
		ishipsjoint = false;
	}

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);


	//if (postflag && (directframe == applyframe)) {
	//	return srcboneno;
	//}


	CBone* curbone = firstbone;
	CBone* lastpar = curbone;
	srcmodel->SetBefEditMat(limitdegflag, erptr, curbone, maxlevel);
	CBone* editboneforret = 0;
	if (firstbone->GetParent(false)) {
		editboneforret = firstbone->GetParent(false);
	}
	else {
		editboneforret = firstbone;
	}


	//For IKTraget
	//�J��������]�ƃJ�������ɐ����Ȏ���]�Ɓ@�Q����s����
	int calcnum = 3;

	int calccnt;
	for (calccnt = 1; calccnt <= calcnum; calccnt++) {
		curbone = firstbone;
		lastpar = curbone;

		int levelcnt = 0;
		float currate;
		//currate = g_ikrate;
		currate = 0.750f;

		while (curbone && lastpar && lastpar->GetParent(false) && ((maxlevel == 0) || (levelcnt < maxlevel)))
		{

			//IKTarget()�Ńt���O�����Z�b�g�����̂Ł@���[�v�擪�Ł@�Z�b�g������
			//g_underIKRot = true;

			//CBone* parentbone = curbone->GetParent();
			CBone* parentbone = lastpar->GetParent(false);
			if (parentbone && parentbone->IsSkeleton() && (curbone->GetJointFPos() != parentbone->GetJointFPos())) {
				//UpdateMatrix(limitdegflag, &m_matWorld, &m_matVP);//curmp�X�V

				CRigidElem* curre = srcmodel->GetRigidElem(lastpar->GetBoneNo());
				if (curre && (curre->GetForbidRotFlag() != 0)) {

					//_ASSERT(0);

					//��]�֎~�̏ꍇ�������X�L�b�v
					if (parentbone) {
						lastpar = parentbone;
					}
					levelcnt++;
					currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
					continue;
				}

				ChaVector3 rotaxis2;
				float rotrad2;
				//if ((calccnt % 2) != 0) {
				//	CalcAxisAndRotForIKRotate(limitdegflag,
				//		parentbone, firstbone,
				//		curframe, targetpos,
				//		&rotaxis2, &rotrad2);
				//}
				//else {
				//	CalcAxisAndRotForIKRotateVert(limitdegflag,
				//		parentbone, firstbone,
				//		curframe, targetpos,
				//		&rotaxis2, &rotrad2);
				//}


				ChaMatrix parentnodemat;
				parentnodemat = parentbone->GetNodeMat();
				ChaMatrix parentnoderot;
				parentnoderot = ChaMatrixRot(parentnodemat);
				ChaVector3 ikaxis;
				if ((calccnt % 3) == 0) {
					ikaxis = parentnoderot.GetRow(0);
				}
				else if ((calccnt % 2) == 0) {
					ikaxis = parentnoderot.GetRow(1);
				}
				else {
					ikaxis = parentnoderot.GetRow(2);
				}
				bool nearflag = CalcAxisAndRotForIKRotateAxis(srcmodel, limitdegflag,
					parentbone, firstbone,
					srcmotid, curframe, targetpos,
					ikaxis,
					&rotaxis2, &rotrad2);

				//rotrad2 *= currate;

				double firstframe = 0.0;
				if ((nearflag == false) && (fabs(rotrad2) > 1.0e-4)) {
					CQuaternion rotq0;
					rotq0.SetAxisAndRot(rotaxis2, rotrad2);

					//parentbone->SaveSRT(limitdegflag, m_curmotinfo->motid, startframe, endframe);
					// 
					//�ۑ����ʂ́@CBone::RotAndTraBoneQReq�ɂ����Ă����g���Ă��炸�@startframe�����g���Ă��Ȃ�
					parentbone->SaveSRT(limitdegflag, srcmotid, startframe);


					//IKRotate�͕ǂ���IK�ōs���̂Ł@��]�\���ǂ����̃`�F�b�N�͂����ł͂��Ȃ�


					int keyno = 0;
					bool keynum1flag = false;
					bool fromiktarget = true;
					IKRotateOneFrame(srcmodel, limitdegflag, erptr,
						keyno,
						parentbone, parentbone,
						srcmotid, curframe, startframe, applyframe,
						rotq0, keynum1flag, postflag, fromiktarget);
					keyno++;

					//if (g_applyendflag == 1) {
					//	//curmotinfo->curframe����Ō�܂�curmotinfo->curframe�̎p����K�p
					//	int tolast;
					//	for (tolast = (int)m_curmotinfo->curframe + 1; tolast < m_curmotinfo->frameleng; tolast++) {
					//		(m_bonelist[0])->PasteRotReq(limitdegflag, m_curmotinfo->motid, m_curmotinfo->curframe, tolast);
					//	}
					//}
				}

			}

			if (parentbone) {
				lastpar = parentbone;

				//�R���X�g���C���g�p��]���@IKStop�Ŏ~�߂�K�v�L
				//�̂̒��S�܂ŉ�]��`���������@�R���X�g���C���g���₷����
				//shoulder��IKStop�ŉ�]���~�߂Ȃ��ꍇ
				//�E��ƍ���̃R���X�g���C���g���Ԃɂ��@�ǂ��炩�ɂ����S���ł��Ȃ��Ȃ�
				//check ikstopflag
				if (parentbone->GetIKStopFlag()) {
					break;
				}
			}


			levelcnt++;

			//currate = (float)pow((double)g_ikrate, (double)g_ikfirst * (double)levelcnt);
		}

		//��΃��[�h�̏ꍇ
		if ((calccnt == calcnum) && g_absikflag && lastpar) {
			AdjustBoneTra(srcmodel, limitdegflag, erptr, lastpar, srcmotid);
		}
	}

	//if( lastpar ){
	//	return lastpar->GetBoneNo();
	//}else{
	//	return srcboneno;
	//}


	//g_underIKRot = false;//2023/01/14 parent limited or not
	if (editboneforret)
	{
		return editboneforret->GetBoneNo();
	}
	else {
		return srcboneno;
	}

}


int ChaCalcFunc::AdjustBoneTra(CModel* srcmodel, bool limitdegflag, CEditRange* erptr, CBone* lastpar, int srcmotid)
{
	int keynum = erptr->GetKeyNum();
	double startframe = erptr->GetStartFrame();
	double endframe;
	//if (g_applyendflag == 1) {
	//	endframe = m_curmotinfo->frameleng - 1.0;
	//}
	//else {
	endframe = erptr->GetEndFrame();
	//}

	if (lastpar && (keynum >= 2)) {
		int keyno = 0;
		double curframe;
		for (curframe = RoundingTime(startframe); curframe <= endframe; curframe += 1.0) {
			if (keyno >= 1) {
				CMotionPoint* pcurmp = 0;
				int curmotid = srcmotid;
				pcurmp = lastpar->GetMotionPoint(curmotid, curframe);
				if (pcurmp) {
					ChaVector3 orgpos;
					ChaVector3 tmplastfpos = lastpar->GetJointFPos();
					ChaMatrix tmpbefeditmat = pcurmp->GetBefEditMat();
					ChaVector3TransformCoord(&orgpos, &tmplastfpos, &tmpbefeditmat);

					ChaVector3 newpos;
					ChaMatrix tmpwm = lastpar->GetWorldMat(limitdegflag, curmotid, curframe, pcurmp);
					ChaVector3TransformCoord(&newpos, &tmplastfpos, &tmpwm);

					ChaVector3 diffpos;
					diffpos = orgpos - newpos;

					CEditRange tmper;
					KeyInfo tmpki;
					tmpki.time = curframe;
					list<KeyInfo> tmplist;
					tmplist.push_back(tmpki);
					tmper.SetRange(tmplist, curframe);
					//FKBoneTra( 0, &tmper, lastpar->GetBoneNo(), diffpos );
					FKBoneTra(srcmodel, limitdegflag, 1, &tmper, lastpar->GetBoneNo(), srcmotid, diffpos);//2022/11/07 FKBoneTra����frameno loop���Ȃ��悤�Ɂ@onlyoneflag = 1
				}
			}
			keyno++;
		}
	}

	return 0;
}

int ChaCalcFunc::FKBoneTra(CModel* srcmodel, bool limitdegflag, int onlyoneflag, CEditRange* erptr,
	int srcboneno, int srcmotid, ChaVector3 addtra, double onlyoneframe)
{

	if (!srcmodel || (srcboneno < 0)) {
		_ASSERT(0);
		return 1;
	}

	CBone* firstbone = srcmodel->GetBoneByID(srcboneno);
	if (!firstbone) {
		_ASSERT(0);
		return 1;
	}
	if (firstbone->IsNotSkeleton()) {
		return 1;
	}

	CBone* curbone = firstbone;
	srcmodel->SetBefEditMatFK(limitdegflag, erptr, curbone);

	CBone* lastpar = firstbone->GetParent(false);

	int keynum;
	double startframe, endframe, applyframe;
	erptr->GetRange(&keynum, &startframe, &endframe, &applyframe);
	if (onlyoneflag == 0) {
	}
	else {
		startframe = onlyoneframe;
		endframe = onlyoneframe;
	}


	curbone = firstbone;
	double firstframe = 0.0;

	ChaMatrix dummyparentwm;
	dummyparentwm.SetIdentity();//Req�֐��̍ŏ��̌Ăяo�����́@Identity��n���΂悢

	if (keynum >= 2) {
		//float changerate = 1.0f / (float)(endframe - startframe + 1);

		int keyno = 0;
		double curframe;
		for (curframe = startframe; curframe <= endframe; curframe += 1.0) {
			double changerate;
			//if( curframe <= applyframe ){
			//	changerate = 1.0 / (applyframe - startframe + 1);
			//}else{
			//	changerate = 1.0 / (endframe - applyframe + 1);
			//}
			changerate = (double)(*(g_motionbrush_value + (int)curframe));


			if (keyno == 0) {
				firstframe = curframe;
			}
			if (g_absikflag == 0) {
				if (g_slerpoffflag == 0) {
					//double currate2;
					//if( curframe <= applyframe ){
					//	currate2 = changerate * (curframe - startframe + 1);
					//}else{
					//	currate2 = changerate * (endframe - curframe + 1);
					//}
					//ChaVector3 curtra;
					//curtra = addtra * (float)currate2;
					ChaVector3 curtra;
					curtra = addtra * (float)changerate;

					//currate2 = changerate * keyno;
					//ChaVector3 curtra;
					//curtra = (1.0 - currate2) * addtra;

					curbone->AddBoneTraReq(limitdegflag, 0, srcmotid, curframe, curtra, dummyparentwm, dummyparentwm);
				}
				else {
					curbone->AddBoneTraReq(limitdegflag, 0, srcmotid, curframe, addtra, dummyparentwm, dummyparentwm);
				}
			}
			else {
				if (keyno == 0) {
					curbone->AddBoneTraReq(limitdegflag, 0, srcmotid, curframe, addtra, dummyparentwm, dummyparentwm);
				}
				else {
					curbone->SetAbsMatReq(limitdegflag, 0, srcmotid, curframe, firstframe);
				}
			}

			bool postflag = false;
			IKTargetVec(srcmodel, limitdegflag, erptr, srcmotid, curframe, postflag);


			keyno++;

		}
	}
	else {
		curbone->AddBoneTraReq(limitdegflag, 0, srcmotid, startframe, addtra, dummyparentwm, dummyparentwm);

		bool postflag = false;
		IKTargetVec(srcmodel, limitdegflag, erptr, srcmotid, startframe, postflag);
	}


	return curbone->GetBoneNo();
}


int ChaCalcFunc::CalcBoneEul(CModel* srcmodel, bool limitdegflag, int srcmotid)
{
	if (!srcmodel) {
		_ASSERT(0);
		return 1;
	}

	if (srcmodel->GetNoBoneFlag() == true) {
		return 0;
	}

	if (srcmotid >= 0) {
		map<int, CBone*>::iterator itrbone;
		for (itrbone = srcmodel->GetBoneListBegin(); itrbone != srcmodel->GetBoneListEnd(); itrbone++) {
			CBone* curbone = itrbone->second;
			if (curbone) {
				MOTINFO* mi = srcmodel->GetMotInfo(srcmotid);
				if (mi) {
					CalcBoneEulOne(srcmodel, limitdegflag, curbone, mi->motid, 0.0, mi->frameleng - 1.0);
				}
			}
		}
	}

	return 0;
}
int ChaCalcFunc::CalcBoneEulOne(CModel* srcmodel, bool limitdegflag, CBone* curbone, int srcmotid, double startframe, double endframe)
{
	if (!srcmodel || !curbone) {
		_ASSERT(0);
		return 1;
	}

	if (curbone->IsSkeleton()) {
		ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
		int paraxiskind = -1;
		double srcframe;
		for (srcframe = RoundingTime(startframe); srcframe <= RoundingTime(endframe); srcframe += 1.0) {
			cureul = curbone->CalcLocalEulXYZ(limitdegflag, paraxiskind, srcmotid, srcframe, BEFEUL_BEFFRAME);
			curbone->SetLocalEul(limitdegflag, srcmotid, srcframe, cureul, 0);
		}
	}
	return 0;
}

ChaVector3 ChaCalcFunc::CalcLocalEulXYZ(CBone* srcbone, bool limitdegflag, int axiskind,
	int srcmotid, double srcframe, tag_befeulkind befeulkind, ChaVector3* directbefeul)
{
	if (!srcbone) {
		_ASSERT(0);
		return ChaVector3(0.0f, 0.0f, 0.0f);
	}


	//###################################################################################################################
	//2022/12/17
	//���̊֐��̌Ăяo������LimitEul()������
	//Parent�̎p���Ɋւ��Ă͌v�Z�ς�GetParent()->GetWorldMat()���g�p : curwm�Ɋ|�����Ă���̂�Limited�ł͂Ȃ�parentwm
	//���[�V�����S�̂̃I�C���[�p�v�Z�������́@���̊֐��ł͂Ȃ��@UpdateMatrix���g�p
	//###################################################################################################################

	double roundingframe = RoundingTime(srcframe);


	//for debug
	//if ((roundingframe == 2.0) && (g_limitdegflag == true) && (strstr(GetBoneName(), "Root") != 0)) {
	//	_ASSERT(0);
	//}


	//axiskind : BONEAXIS_*  or  -1(CBone::m_anglelimit.boneaxiskind)

	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);
	BEFEUL befeul;
	befeul.Init();


	if (srcbone->IsNotSkeleton() && srcbone->IsNotCamera() && srcbone->IsNotNull()) {
		return cureul;
	}
	if (!srcbone->GetParModel()) {
		_ASSERT(0);
		return cureul;
	}


	const WCHAR* bonename = srcbone->GetWBoneName();
	//if (wcscmp(bonename, L"RootNode") == 0){
	//	return cureul;//!!!!!!!!!!!!!!!!!!!!!!!!
	//}

	if (befeulkind == BEFEUL_BEFFRAME) {
		//1�O�̃t���[����EUL�͂��łɌv�Z����Ă���Ɖ��肷��B
		//bool limitdegOnLimitEul = false;//2023/02/07 befeul��unlimited. ����]������ꍇ�ɃI�[�o�[�P�W�O�x�̊p�x�Ő������邽�߂�.
		//befeul = srcbone->GetBefEul(limitdegOnLimitEul, srcmotid, roundingframe);

		//2023/10/18
		//limitdeg true�̂Ƃ��̂��߂Ɂ@limitdegflag�͂��̂܂ܓn���K�v
		//�P�W�O�x�I�[�o�[�΍�Ƃ��Ắ@PostIK�̌�Ɂ@CalcBoneEul���Ă�
		befeul = srcbone->GetBefEul(limitdegflag, srcmotid, roundingframe);

	}
	else if ((befeulkind == BEFEUL_DIRECT) && directbefeul) {
		befeul.befframeeul = *directbefeul;
		befeul.currentframeeul = *directbefeul;
	}

	int isfirstbone = 0;
	int isendbone = 0;


	CBone* parentbone = srcbone->GetParent(false);//!!!!!!!!!!


	if (parentbone && parentbone->IsSkeleton()) {
		isfirstbone = 0;
	}
	else {
		isfirstbone = 1;
	}

	if (srcbone->GetChild(false) && srcbone->GetChild(false)->IsSkeleton()) {
		if (srcbone->GetChild(false)->GetChild(false) && srcbone->GetChild(false)->GetChild(false)->IsSkeleton()) {
			isendbone = 0;
		}
		else {
			isendbone = 1;
		}
	}
	else {
		isendbone = 1;
	}



	int notmodify180flag = srcbone->GetNotModify180Flag(srcmotid, roundingframe);
	CQuaternion axisq;
	axisq.RotationMatrix(srcbone->GetNodeMat());

	CQuaternion eulq;

	if (srcbone->IsSkeleton()) {

		//###########################
		//skeleton�̏ꍇ
		//###########################
		CMotionPoint* curmp = 0;
		curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			ChaMatrix curwm;
			curwm = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);

			if (parentbone) {
				isfirstbone = 0;

				ChaMatrix parentwm, eulmat;

				//parent��eNull�̏ꍇ�͂���
				if (parentbone->IsSkeleton()) {
					parentwm = parentbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
					eulq = ChaMatrix2Q(ChaMatrixInv(parentwm)) * ChaMatrix2Q(curwm);
				}
				else if (parentbone->IsNull() || parentbone->IsCamera()) {
					//2023/05/16 eNull�ɂ�Identity�ȊO��NodeMat���ݒ肳�ꂽ���ߏC��
					//parentwm = ChaMatrixInv(parentbone->GetNodeMat()) * parentbone->GetENullMatrix();//ENullMatrix�ɂ�NodeMat���|�����Ă���

					//2023/06/26 �����o������worldmat (InvNodeMat * EvaluateGlobalTransform)��enull�́@��]�́@�e���͓����Ă��Ȃ�? NodeMat�ɓ����Ă���H�@�H�H�H
					//���f����eNull��Y180�x��]�������f���̓ǂݏ����ǂݏ����ǂ݃e�X�g�Ŋm�F
					//���L�̂悤�ɕύX���Ȃ��Ɓ@eNull��Y180�x��]�������f���̓ǂݏ����ǂݎ��ɃI�C���[�p�\�����ώ����@�ǂݏ����ǂݏ����ǂ݃e�X�g�Ł@���f�����������΂�����
					//eulq = ChaMatrix2Q(curwm);




					//2023/06/29 eNull���A�j���[�V�����\�ɂ����̂�
					//GetENullMatrix���C������CalcEnullMatReq�Ōv�Z����悤�ɂ����Ƃ��낪2023/06/26����ς�����Ƃ���
					//SetWorldMat()���ɂ́@��]�v�Z�p�̃��[�J���s��擾���Ɂ@parenet��eNull�̏ꍇ�ւ��Ă�GetWorldMat[invNode * CalcENullMat]���g�p
					//�ړ��v�Z���ɂ́@CalcFbxLocalMatrix���ɂā@parent��eNull�̏ꍇ�@GetENullMatrix���g�p
					//
					//null�́@��]�́@�e���͓����Ă��Ȃ�? NodeMat�ɓ����Ă���H�Ɋւ���
					//eNull���A�j���[�V�������Ȃ��ꍇ�ɂ́@eNull�̏����s���NodeMat�Ɋ܂܂��@eNull�̃A�j���[�V��������InvNode * ENullMat = Indentity�ƂȂ�
					//eNull���A�j���[�V��������ꍇ�ɂ́@eNull�̃A�j���[�V��������InvNode * ENullMat != Indentity�ƂȂ���Ɋ|����@
					//�⑫�FNodeMat�̓W���C���g�̈ʒu�ł���@NodeMat��r���ŕς��邱�Ƃ̓W���C���g�ʒu��r���ŕς��邱�Ƃł���@�ʏ�NodeMat�͕ς��Ȃ�
					parentwm = parentbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
					eulq = ChaMatrix2Q(ChaMatrixInv(parentwm)) * ChaMatrix2Q(curwm);
				}
				else {
					eulq = ChaMatrix2Q(curwm);
				}
			}
			else {
				isfirstbone = 1;
				eulq = ChaMatrix2Q(curwm);
			}
		}
		else {
			//_ASSERT(0);
			eulq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
		}

		eulq.Q2EulXYZusingQ(&axisq, befeul, &cureul, isfirstbone, isendbone, notmodify180flag);

	}
	else if (srcbone->IsCamera()) {

		//########################
		//�J�����̏ꍇ
		//########################

		if (srcbone->GetParModel() && srcbone->GetParModel()->IsCameraLoaded()) {

			EnterCriticalSection(&g_CritSection_FbxSdk);
			cureul = srcbone->GetParModel()->CalcCameraFbxEulXYZ(srcmotid, roundingframe);
			//####  rotorder����  #####
			LeaveCriticalSection(&g_CritSection_FbxSdk);

		}
		else {
			_ASSERT(0);
			cureul = ChaVector3(0.0f, 0.0f, 0.0f);
		}

	}
	else if (srcbone->IsNull()) {

		//###########
		//eNull�̏ꍇ
		//###########

		if (srcbone->GetFbxNodeOnLoad()) {
			EnterCriticalSection(&g_CritSection_FbxSdk);
			FbxTime fbxtime;
			fbxtime.SetSecondDouble(roundingframe / 30.0);
			FbxVector4 orgfbxeul = srcbone->GetFbxNodeOnLoad()->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
			cureul = ChaVector3(orgfbxeul, false);
			//####  rotorder����  #####
			LeaveCriticalSection(&g_CritSection_FbxSdk);
		}
		else {
			return cureul;
		}
	}
	else {
		return cureul;
	}

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

int ChaCalcFunc::SetWorldMat(CBone* srcbone, bool limitdegflag, bool directsetflag,
	bool infooutflag, int setchildflag,
	int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck, bool fromiktarget)
{
	double roundingframe = RoundingTime(srcframe);

	if (!srcbone) {
		_ASSERT(0);
		return 1;
	}

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}


	//if pose is change, return 1 else return 0
	CMotionPoint* curmp;
	curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
	if (!curmp) {
		return 0;
	}

	curmp->SetCalcLimitedWM(0);//2023/01/14 limited�@�t���O�@���Z�b�g


	int ismovable = 0;


	//�ύX�O��ۑ�
	ChaMatrix saveworldmat;
	ChaVector3 saveeul;
	saveworldmat = srcbone->GetWorldMat(limitdegflag, srcmotid, roundingframe, curmp);
	saveeul = srcbone->GetLocalEul(limitdegflag, srcmotid, roundingframe, curmp);


	//if ((directsetflag == false) && (g_underRetargetFlag == false)){

	if (directsetflag == false) {
		ChaMatrix beflocalmat;
		ChaMatrix newlocalmat;
		beflocalmat.SetIdentity();
		newlocalmat.SetIdentity();
		if (srcbone->GetParent(false)) {
			//eNull���A�j���[�V�����\
			//GetENullMatrix���C������CalcEnullMatReq�Ōv�Z����悤�ɂ����Ƃ��낪2023/06/26����ς�����Ƃ���
			//SetWorldMat()���ɂ́@��]�v�Z�p�̃��[�J���s��擾���Ɂ@parenet��eNull�̏ꍇ�ւ��Ă�GetWorldMat[invNode * CalcENullMat]���g�p
			//Fbx��]�v�Z���ɂ́@CalcLocalEulXYZ()���ɂā@parent��eNull�̏ꍇ�@invNode * CalcENullMat���g�p
			//Fbx�ړ��v�Z���ɂ́@CalcFbxLocalMatrix���ɂā@parent��eNull�̏ꍇ�@GetENullMatrix���g�p
			ChaMatrix befparentwm;
			befparentwm = srcbone->GetParent(false)->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
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
		GetSRTandTraAnim(newlocalmat, srcbone->GetNodeMat(), &newsmat, &newrmat, &newtmat, &newtanimmat);

		//calc new eul
		srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, srcmat, curmp);//tmp time
		ChaVector3 neweul = ChaVector3(0.0f, 0.0f, 0.0f);
		neweul = srcbone->CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);


		//if (g_limitdegflag == true) {
		if ((limitdegflag == true) && (srcbone->GetBtForce() == 0)) {//2023/01/28 �����V�~���́@���O�ł͐������Ȃ�
			ismovable = srcbone->ChkMovableEul(neweul);
		}
		else {
			ismovable = 1;
		}

		if (infooutflag == true) {
			OutputToInfoWnd(L"CBone::SetWorldMat : %s : frame %5.1lf : neweul [%f, %f, %f] : ismovable %d", srcbone->GetWBoneName(), roundingframe, neweul.x, neweul.y, neweul.z, ismovable);
		}

		if (onlycheck == 0) {
			if (ismovable == 1) {
				int inittraflag0 = 0;
				//�q�W���C���g�ւ̔g�y�́@SetWorldMatFromEulAndScaleAndTra���ł��Ă���
				srcbone->SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
					saveworldmat, neweul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag�L��!!!!
				//curmp->SetBefWorldMat(saveworldmat);
				srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, neweul, curmp);
				if (limitdegflag == true) {
					curmp->SetCalcLimitedWM(2);
				}
			}
			else {
				if ((g_wallscrapingikflag == 1) || (fromiktarget == true)) {//PosConstraint�p�̉�]���ɂ́@�����ŕǂ��菈��������
					//############################################
					//�@�V�ѕt�����~�e�b�hIK
					//############################################
					ChaVector3 limiteul;
					bool limitdegOnLimitEul1 = false;//2023/02/07 befeul��unlimited. ����]������ꍇ�ɃI�[�o�[�P�W�O�x�̊p�x�Ő������邽�߂�.
					//limiteul = LimitEul(neweul, GetBefEul(limitdegOnLimitEul1, srcmotid, roundingframe));
					limiteul = srcbone->LimitEul(neweul);
					int inittraflag0 = 0;
					//�q�W���C���g�ւ̔g�y�́@SetWorldMatFromEulAndScaleAndTra���ł��Ă���
					srcbone->SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
						saveworldmat, limiteul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag�L��!!!!
					srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, limiteul, curmp);
					if (limitdegflag == true) {
						curmp->SetCalcLimitedWM(2);
					}
				}
				else {
					if (g_underIKRot == true) {
						srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
						srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
						if (limitdegflag == true) {
							curmp->SetCalcLimitedWM(2);
						}
					}
					else {
						ChaVector3 limiteul;
						bool limitdegOnLimitEul2 = false;//2023/02/07 befeul��unlimited. ����]������ꍇ�ɃI�[�o�[�P�W�O�x�̊p�x�Ő������邽�߂�.
						//limiteul = LimitEul(neweul, GetBefEul(limitdegOnLimitEul2, srcmotid, roundingframe));
						limiteul = srcbone->LimitEul(neweul);
						int inittraflag0 = 0;
						//�q�W���C���g�ւ̔g�y�́@SetWorldMatFromEulAndScaleAndTra���ł��Ă���
						srcbone->SetWorldMatFromEulAndScaleAndTra(limitdegflag, inittraflag0, setchildflag,
							saveworldmat, limiteul, befscalevec, ChaMatrixTraVec(newtanimmat), srcmotid, roundingframe);//setchildflag�L��!!!!
						srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, limiteul, curmp);
						if (limitdegflag == true) {
							curmp->SetCalcLimitedWM(2);
						}
					}
				}
			}
		}
		else {
			//only check : ���Z�b�g���Ă����̂����ɖ߂�
			srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
			srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
		}
	}
	else {

		//directsetflag == true

		ismovable = 1;
		if (onlycheck == 0) {
			srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, srcmat, curmp);

			ChaVector3 neweul = srcbone->CalcLocalEulXYZ(limitdegflag, -1, srcmotid, roundingframe, BEFEUL_BEFFRAME);
			srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, neweul, curmp);
			if (limitdegflag == true) {
				curmp->SetCalcLimitedWM(2);
			}

			if (setchildflag == 1) {
				if (srcbone->GetChild(false)) {
					bool setbroflag = true;
					srcbone->GetChild(false)->UpdateParentWMReq(limitdegflag, setbroflag, srcmotid, roundingframe,
						saveworldmat, srcmat);
				}
			}
		}
		else {
			//only check : ���Z�b�g���Ă����̂����ɖ߂�
			srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, saveworldmat, curmp);
			srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, saveeul, curmp);
		}
	}


	return ismovable;
}

int ChaCalcFunc::SetWorldMat(CBone* srcbone, bool limitdegflag,
	int srcmotid, double srcframe, ChaMatrix srcmat, CMotionPoint* srcmp)//default : srcmp = 0
{
	double roundingframe = RoundingTime(srcframe);

	if (!srcbone) {
		_ASSERT(0);
		return 1;
	}

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}

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
		curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
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

int ChaCalcFunc::SetWorldMatFromEulAndScaleAndTra(CBone* srcbone, bool limitdegflag, int inittraflag, int setchildflag,
	ChaMatrix befwm, ChaVector3 srceul, ChaVector3 srcscale, ChaVector3 srctra, int srcmotid, double srcframe)
{
	if (!srcbone) {
		_ASSERT(0);
		return 1;
	}


	//anglelimit��������̃I�C���[�p���n�����Banglelimit��CBone::SetWorldMat�ŏ�������B
	//if (!m_child) {
	//	return 0;
	//}

	double roundingframe = RoundingTime(srcframe);

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}


	ChaMatrix newscalemat;
	newscalemat.SetIdentity();//2023/02/12 ���������Ă�
	ChaMatrixScaling(&newscalemat, srcscale.x, srcscale.y, srcscale.z);//!!!!!!!!!!!!  new scale
	ChaMatrix newtramat;
	ChaMatrixIdentity(&newtramat);
	ChaMatrixTranslation(&newtramat, srctra.x, srctra.y, srctra.z);//TraAnim�����̂܂�

	CMotionPoint* curmp;
	curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
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


	ChaMatrix newlocalrotmat = srcbone->CalcLocalRotMatFromEul(srceul, srcmotid, roundingframe);
	ChaMatrix newlocalmat;
	bool sflag = true;
	bool tanimflag = (inittraflag == 0);
	newlocalmat = ChaMatrixFromSRTraAnim(sflag, tanimflag, srcbone->GetNodeMat(), &newscalemat, &newlocalrotmat, &newtramat);

	ChaMatrix newmat;
	if (srcbone->GetParent(false)) {
		ChaMatrix limitedparmat = srcbone->GetParent(false)->GetWorldMat(limitdegflag, srcmotid, roundingframe, 0);
		newmat = newlocalmat * limitedparmat;
	}
	else {
		newmat = newlocalmat;
	}

	if (curmp) {
		//curmp->SetBefWorldMat(curmp->GetWorldMat());
		srcbone->SetWorldMat(limitdegflag, srcmotid, roundingframe, newmat, curmp);
		srcbone->SetLocalEul(limitdegflag, srcmotid, roundingframe, srceul, curmp);
		if (limitdegflag == true) {
			curmp->SetCalcLimitedWM(2);
		}

		if (setchildflag == 1) {
			if (srcbone->GetChild(false)) {
				bool setbroflag = true;
				srcbone->GetChild(false)->UpdateParentWMReq(limitdegflag, setbroflag,
					srcmotid, roundingframe, befwm, newmat);
			}
		}
	}
	else {
		_ASSERT(0);
	}

	return 0;
}


CMotionPoint* ChaCalcFunc::GetMotionPoint(CBone* srcbone, int srcmotid, double srcframe, bool onaddmotion) {
	if (!srcbone) {
		_ASSERT(0);
		return 0;
	}

	//���݂���Ƃ������Ԃ��B
	CMotionPoint* pbef = 0;
	CMotionPoint* pnext = 0;
	int existflag = 0;
	GetBefNextMP(srcbone, srcmotid, srcframe, &pbef, &pnext, &existflag, onaddmotion);
	if (existflag == 1) {
		return pbef;
	}
	else {
		return 0;
	}
}

ChaMatrix ChaCalcFunc::GetWorldMat(CBone* srcbone, bool limitdegflag,
	int srcmotid, double srcframe, CMotionPoint* srcmp, ChaVector3* dsteul)//default : dsteul = 0
{

	double roundingframe = RoundingTime(srcframe);

	ChaMatrix curmat;
	ChaMatrixIdentity(&curmat);
	if (!srcbone) {
		_ASSERT(0);
		return curmat;
	}


	if (!srcbone->GetParModel()) {
		_ASSERT(0);
		curmat.SetIdentity();
		return curmat;
	}


	//2023/04/28
	if (srcbone->IsNull()) {
		//2023/05/07  
		//return GetENullMatrix();

		//2023/05/09
		//CalcLocalEulXYZ�̃��[�J���v�Z���������̗��R�ɂ��@eNull��WorldMat��Identity��Ԃ����Ƃɂ���
		//�������@fbx��scale��tra�o�͎��̐e�s��Ƃ��Ă�GetENullMatrix()���g��(CalcFbxLocalMatrix()�Q��)
		//Rot��tra,scale�̈Ⴂ�́@Rot��NodeMat�����Ń��[�J���v�Z���Ď��Ƃ���NodeMat���w��@tra,scale�̓��[�J���v�Z����NodeMat���|����
		//return curmat;


		//2023/05/16 eNull��NodeMat��Identity�ł͂Ȃ��Ȃ�������
		//return ChaMatrixInv(GetNodeMat()) * GetENullMatrix(srcframe);//!!!!!!!!!!!!

		//return ChaMatrixInv(GetNodeMat()) * GetTransformMat(srcframe, true);

		EnterCriticalSection(&g_CritSection_FbxSdk);
		ChaMatrix retmat = ChaMatrixInv(srcbone->GetNodeMat()) * srcbone->GetTransformMat(0.0, true);//!!!!!  1.2.0.26
		LeaveCriticalSection(&g_CritSection_FbxSdk);
		return retmat;
		//return ChaMatrixInv(GetNodeMat()) * GetTransformMat(0.0, true);//!!!!!  1.2.0.26

		//2023/06/27
		//CalcLocalEulXYZ()�̌��؂Ł@Parent��eNull�̂Ƃ��ɂ́@parentwm��Identity�ɂ���ׂ��������̂Ł@����ɍ��킹��
		//curmat.SetIdentity();
		//return curmat;
	}
	else if (srcbone->IsCamera()) {
		//bool multInvNodeMat = true;
		//return GetParModel()->GetCameraTransformMat(srcmotid, srcframe, g_cameraInheritMode, multInvNodeMat);
		//return ChaMatrixInv(GetNodeMat()) * GetTransformMat(0.0, true);//2023/07/05 Camera�̎q���̃X�L�����b�V���̌`���@�ǂݏ����ǂݏ����ǂ݃e�X�g�Ō`���ꂵ�Ȃ��悤��

		EnterCriticalSection(&g_CritSection_FbxSdk);
		ChaMatrix retmat = ChaMatrixInv(srcbone->GetNodeMat()) * srcbone->GetTransformMat(0.0, true);//!!!!!  1.2.0.26
		LeaveCriticalSection(&g_CritSection_FbxSdk);
		return retmat;

		//curmat.SetIdentity();
		//return curmat;
	}
	else if (srcbone->IsSkeleton()) {
		if (srcmp) {
			if (limitdegflag == false) {
				curmat = srcmp->GetWorldMat();
				if (dsteul) {
					*dsteul = srcmp->GetLocalEul();
				}
			}
			else {
				curmat = srcmp->GetLimitedWM();
				if (dsteul) {
					*dsteul = srcmp->GetLimitedLocalEul();
				}
			}
		}
		else {
			CMotionPoint* curmp;
			curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
			if (curmp) {
				if (limitdegflag == false) {
					curmat = curmp->GetWorldMat();
					if (dsteul) {
						*dsteul = curmp->GetLocalEul();
					}
				}
				else {
					curmat = curmp->GetLimitedWM();
					if (dsteul) {
						*dsteul = curmp->GetLimitedLocalEul();
					}
				}
			}
		}
		return curmat;

	}
	else {
		curmat.SetIdentity();
		return curmat;//!!!!!!!!!!!!  Skeleton�ł�Null�ł�Camera�ł������ꍇ�@identity��Ԃ�
	}

}

ChaVector3 ChaCalcFunc::LimitEul(CBone* srcbone, ChaVector3 srceul)
{
	if (!srcbone) {
		_ASSERT(0);
		return srceul;
	}

	const float thdeg = 165.0f;

	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);
	ChaVector3 tmpeul = ChaVector3(0.0f, 0.0f, 0.0f);


	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return reteul;
	}


	tmpeul.x = LimitAngle(srcbone, AXIS_X, srceul.x);
	tmpeul.y = LimitAngle(srcbone, AXIS_Y, srceul.y);
	tmpeul.z = LimitAngle(srcbone, AXIS_Z, srceul.z);


	//����]������ꍇ�Ɂ@�����Ɋp�x�Ő������邽�߂ɃR�����g�A�E�g
	//CQuaternion calcq;
	//calcq.ModifyEuler360(&tmpeul, &srcbefeul, 0);

	reteul = tmpeul;


	return reteul;
}

float ChaCalcFunc::LimitAngle(CBone* srcbone, enum tag_axiskind srckind, float srcval)
{
	if (!srcbone) {
		_ASSERT(0);
		return srcval;
	}


	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0.0f;
	}


	srcbone->SetAngleLimitOff();
	ANGLELIMIT tmpanglelimit;
	tmpanglelimit.Init();
	bool dummylimitdegflag = false;
	int setchkflag = 0;
	tmpanglelimit = srcbone->GetAngleLimit(dummylimitdegflag, setchkflag);

	if (tmpanglelimit.limitoff[srckind] == 1) {
		return srcval;
	}
	else {
		float newval = srcval;

		float cmpvalupper, cmpvallower;
		cmpvalupper = srcval;
		cmpvallower = srcval;

		if (abs(tmpanglelimit.upper[srckind] - tmpanglelimit.lower[srckind]) > EULLIMITPLAY) {
			//���~�b�g�t�߂ł�IK���������߂ɂ͗V�т̕������K�v

			if (cmpvalupper > tmpanglelimit.upper[srckind]) {
				newval = min(cmpvalupper, (float)(tmpanglelimit.upper[srckind] - EULLIMITPLAY));
			}
			if (cmpvallower < tmpanglelimit.lower[srckind]) {
				newval = max(cmpvallower, (float)(tmpanglelimit.lower[srckind] + EULLIMITPLAY));
			}
		}
		else {
			//lower��upper�̊Ԃ�EULLIMITPLAY��菬�����Ƃ�

			if (cmpvalupper > tmpanglelimit.upper[srckind]) {
				newval = (float)(tmpanglelimit.upper[srckind]);
			}
			if (cmpvallower < tmpanglelimit.lower[srckind]) {
				newval = (float)(tmpanglelimit.lower[srckind]);
			}
		}

		return newval;
	}
}

int ChaCalcFunc::SetLocalEul(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, ChaVector3 srceul, CMotionPoint* srcmp)
{
	if (!srcbone) {
		_ASSERT(0);
		return 1;
	}

	double roundingframe = RoundingTime(srcframe);

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}

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
		curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
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

ChaVector3 ChaCalcFunc::GetLocalEul(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, CMotionPoint* srcmp)
{
	double roundingframe = RoundingTime(srcframe);

	ChaVector3 reteul = ChaVector3(0.0f, 0.0f, 0.0f);
	if (!srcbone) {
		_ASSERT(0);
		return reteul;
	}

	//2023/04/28
	if (srcbone->IsNotSkeleton() && srcbone->IsNotCamera()) {//2023/06/05 eCamera�Ή�
		return reteul;
	}


	if (srcmp) {
		if (limitdegflag == false) {
			reteul = srcmp->GetLocalEul();
		}
		else {
			//2023/01/31
			//g_limitdegflag == true�̏ꍇ
			//GetWorldMat-->GetLimitedWorldMat-->CalcLocalEulAndSetLImitedEul
			//-->GetLocalEul(-->GetLimitedWorldMat)
			//��L�̂悤�ɌĂ΂��̂Ł@���̊֐�GetLocalEul����GetLimitedWorldMat���Ăяo�����Ƃ͏o���Ȃ�

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
		CMotionPoint* curmp = srcbone->GetMotionPoint(srcmotid, roundingframe);
		if (curmp) {
			if (limitdegflag == false) {
				reteul = curmp->GetLocalEul();
			}
			else {
				//2023/01/31
				//g_limitdegflag == true�̏ꍇ
				//GetWorldMat-->GetLimitedWorldMat-->CalcLocalEulAndSetLImitedEul
				//-->GetLocalEul(-->GetLimitedWorldMat)
				//��L�̂悤�ɌĂ΂��̂Ł@���̊֐�GetLocalEul����GetLimitedWorldMat���Ăяo�����Ƃ͏o���Ȃ�

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

int ChaCalcFunc::ChkMovableEul(CBone* srcbone, ChaVector3 srceul)
{

	//2023/01/10 : ���O�v�Z���Ă��������ꍇ�Ȃǂ�����̂Ł@�ȉ��R�s�̏����͂��̊֐��̌Ăяo�����ōs��
	//if (g_limitdegflag == false){
	//	return 1;//movable
	//}

	if (!srcbone) {
		_ASSERT(0);
		return 0;
	}

	//2023/04/28
	if (srcbone->IsNotSkeleton()) {
		return 0;
	}


	srcbone->SetAngleLimitOff();
	ANGLELIMIT tmpanglelimit;
	tmpanglelimit.Init();
	bool dummylimitdegflag = false;
	int setchkflag = 0;
	tmpanglelimit = srcbone->GetAngleLimit(dummylimitdegflag, setchkflag);


	int dontmove = 0;
	int axiskind;

	float chkval[3];
	chkval[0] = srceul.x;
	chkval[1] = srceul.y;
	chkval[2] = srceul.z;

	for (axiskind = AXIS_X; axiskind <= AXIS_Z; axiskind++) {
		if (tmpanglelimit.limitoff[axiskind] == 0) {
			//if (m_anglelimit.via180flag[axiskind] == 0) {
			if ((tmpanglelimit.lower[axiskind] > (int)chkval[axiskind]) || (tmpanglelimit.upper[axiskind] < (int)chkval[axiskind])) {
				dontmove++;
			}
			//}
			//else {
			//	//180�x��(-180�x��)���z����悤�ɓ����ꍇ
			//	if ((m_anglelimit.lower[axiskind] <= (int)chkval[axiskind]) && (m_anglelimit.upper[axiskind] >= (int)chkval[axiskind])) {
			//		dontmove++;
			//	}
			//}
		}
	}

	if (dontmove != 0) {
		return 0;
	}
	else {
		return 1;//movable
	}
}
