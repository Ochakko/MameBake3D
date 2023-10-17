#include "stdafx.h"
//#include <stdafx.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <Model.h>
#include <Bone.h>

#define DBGH
#include <dbg.h>

#include <string>

#include <fbxsdk.h>
#include <fbxsdk/scene/shading/fbxlayeredtexture.h>
#include <fbxsdk/scene/animation/fbxanimevaluator.h>

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

#include <io.h>

//for __nop()
#include <intrin.h>

#include <ThreadingCalcEul.h>
#include <GlobalVar.h>

using namespace std;

extern int g_previewFlag;


CThreadingCalcEul::CThreadingCalcEul()
{
	InitParams();
}
int CThreadingCalcEul::InitParams()
{
	CThreadingBase::InitParams();

	m_model = 0;
	ClearBoneList();
	motid = 0;
	m_limitdegflag = false;

	return 0;
}
CThreadingCalcEul::~CThreadingCalcEul()
{
	DestroyObjs();
}

void CThreadingCalcEul::DestroyObjs()
{
	CThreadingBase::DestroyObjs();
	ClearBoneList();
}

int CThreadingCalcEul::ThreadFunc()
{
	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hExitEvent == NULL) {
		_ASSERT(0);
		return 1;
	}

	while (InterlockedAdd(&m_exit_state, 0) != 1) {

		//if ((g_HighRpmMode == true) && 
		//	((g_previewFlag != 0) || (g_underIKRot == true))) {//�v���r���[���@�܂��́@IK��]���@����


		if (g_HighRpmMode == true) {

			//###########################
			// ����]���[�h�@: High rpm
			//###########################

			if (InterlockedAdd(&m_start_state, 0) == 1) {//�v�Z�J�n���߂��L���b�`
				if (InterlockedAdd(&m_exit_state, 0) != 1) {//�X���b�h���I�����Ă��Ȃ��ꍇ

					if (m_model && (m_model->GetInView() == true)) {
						//EnterCriticalSection(&m_CritSection);//�ē��h�~ �Ăяo�����ŏ����I����҂̂ŕs�v
						if (!m_bonevec.empty()) {
							int bonenum = (int)m_bonevec.size();
							int bonecount;
							for (bonecount = 0; bonecount < bonenum; bonecount++) {
								CBone* curbone = m_bonevec[bonecount];
								if (curbone) {
									MOTINFO* mi = m_model->GetMotInfo(motid);
									if (mi) {
										ChaCalcFunc chacalcfunc;
										chacalcfunc.CalcBoneEulOne(m_model, m_limitdegflag, curbone, mi->motid, 0.0, mi->frameleng - 1.0);
									}
								}
							}
						}
					}
					
					InterlockedExchange(&m_start_state, 0L);
					//LeaveCriticalSection(&m_CritSection);
				}
				else {
					//�X���b�h�I���t���O�������Ă����ꍇ
					InterlockedExchange(&m_start_state, 0L);
				}
			}
			else {

				//�v�Z�͏I�����Ă��Čv�Z���ߑ҂����

				//__nop();
				//Sleep(0);
				timeBeginPeriod(1);
				SleepEx(0, TRUE);
				//SleepEx(1, TRUE);
				timeEndPeriod(1);
			}

		}
		else {

			//############################
			// eco ���[�h
			//############################

			DWORD dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);
			ResetEvent(m_hEvent);
			if (InterlockedAdd(&m_exit_state, 0) != 1) {//�X���b�h���I�����Ă��Ȃ��ꍇ
				switch (dwWaitResult)
				{
					// Event object was signaled
				case WAIT_OBJECT_0:
				{
					if (m_model && (m_model->GetInView() == true)) {
						EnterCriticalSection(&m_CritSection);
						if (!m_bonevec.empty()) {
							int bonenum = (int)m_bonevec.size();
							int bonecount;
							for (bonecount = 0; bonecount < bonenum; bonecount++) {
								CBone* curbone = m_bonevec[bonecount];
								if (curbone) {
									MOTINFO* mi = m_model->GetMotInfo(motid);
									if (mi) {
										ChaCalcFunc chacalcfunc;
										chacalcfunc.CalcBoneEulOne(m_model, m_limitdegflag, curbone, mi->motid, 0.0, mi->frameleng - 1.0);
									}
								}
							}
						}

						InterlockedExchange(&m_start_state, 0L);
						LeaveCriticalSection(&m_CritSection);
					}
					else {
						InterlockedExchange(&m_start_state, 0L);
					}

				}
				break;

				// An error occurred
				default:
					//printf("Wait error (%d)\n", GetLastError());
					//return 0;
					break;
				}
			}
			else {
				//�X���b�h�I���t���O�������Ă����ꍇ
				InterlockedExchange(&m_start_state, 0L);
			}
		}
	}

	if (m_hExitEvent != NULL) {
		SetEvent(m_hExitEvent);
	}

	return 0;
}


int CThreadingCalcEul::ClearBoneList()
{
	m_bonevec.clear();

	return 0;
}
int CThreadingCalcEul::SetModel(CModel* srcmodel)
{
	m_model = srcmodel;
	return 0;
}
int CThreadingCalcEul::AddBoneList(CBone* srcbone)
{
	if (srcbone) {
		m_bonevec.push_back(srcbone);
		return 0;
	}
	else {
		_ASSERT(0);
		return 1;
	}
	//return m_bonevec.size();
}

void CThreadingCalcEul::CalcBoneEul(CModel* srcmodel, bool limitdegflag, int srcmotid)
{

	if (!m_model) {
		return;
	}
	if (m_model && (m_model->GetInView() == false)) {
		return;
	}


	//####################################################################
	//## g_limitdegflag == true�@�̏ꍇ�ɂ̓��[�J���̌v�Z��������
	//####################################################################

	if (!m_bonevec.empty()) {
		EnterCriticalSection(&m_CritSection);
		motid = srcmotid;
		m_limitdegflag = limitdegflag;
		LeaveCriticalSection(&m_CritSection);
		InterlockedExchange(&m_start_state, 1L);
		SetEvent(m_hEvent);
	}
	else {
		InterlockedExchange(&m_start_state, 0L);
	}


}



