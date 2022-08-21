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

#include <ThreadingUpdateMatrix.h>
#include <GlobalVar.h>

using namespace std;


CThreadingUpdateMatrix::CThreadingUpdateMatrix()
{
	InitParams();
}
int CThreadingUpdateMatrix::InitParams()
{
	CThreadingBase::InitParams();

	ClearBoneList();
	motid = 0;
	frame = 0.0;
	//wmat = 0;
	//vpmat = 0;
	ChaMatrixIdentity(&wmat);
	ChaMatrixIdentity(&vpmat);


	return 0;
}
CThreadingUpdateMatrix::~CThreadingUpdateMatrix()
{
	DestroyObjs();
}

void CThreadingUpdateMatrix::DestroyObjs()
{
	CThreadingBase::DestroyObjs();
	ClearBoneList();
}

int CThreadingUpdateMatrix::ThreadFunc()
{
	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hExitEvent == NULL) {
		_ASSERT(0);
		return 1;
	}

	while (InterlockedAdd(&m_exit_state, 0) != 1) {

		if (g_HighRpmMode == true) {

			//###########################
			// 高回転モード　: High rpm
			//###########################

			if (InterlockedAdd(&m_start_state, 0) == 1) {
				if (InterlockedAdd(&m_exit_state, 0) != 1) {//終了していない場合
					EnterCriticalSection(&m_CritSection);
					if ((m_bonenum >= 0) || (m_bonenum <= MAXBONEUPDATE)) {
						int bonecount;
						for (bonecount = 0; bonecount < m_bonenum; bonecount++) {
							CBone* curbone = m_bonelist[bonecount];
							if (curbone) {
								bool callingbythread = true;
								curbone->UpdateMatrix(motid, frame, &wmat, &vpmat, callingbythread);
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
			else {
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
			// eco モード
			//############################

			DWORD dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);
			ResetEvent(m_hEvent);
			switch (dwWaitResult)
			{
				// Event object was signaled
			case WAIT_OBJECT_0:
			{
				EnterCriticalSection(&m_CritSection);
				if ((m_bonenum >= 0) || (m_bonenum <= MAXBONEUPDATE)) {

					int bonecount;
					for (bonecount = 0; bonecount < m_bonenum; bonecount++) {
						CBone* curbone = m_bonelist[bonecount];
						if (curbone) {
							bool callingbythread = true;
							curbone->UpdateMatrix(motid, frame, &wmat, &vpmat, callingbythread);
						}
					}
				}

				InterlockedExchange(&m_start_state, 0L);
				LeaveCriticalSection(&m_CritSection);

			}
			break;

			// An error occurred
			default:
				//printf("Wait error (%d)\n", GetLastError());
				//return 0;
				break;
			}
		}
	}

	if (m_hExitEvent != NULL) {
		SetEvent(m_hExitEvent);
	}

	return 0;
}


int CThreadingUpdateMatrix::ClearBoneList()
{
	m_bonenum = 0;
	ZeroMemory(m_bonelist, sizeof(CBone*) * MAXBONEUPDATE);

	return 0;
}
int CThreadingUpdateMatrix::SetBoneList(int srcindex, CBone* srcbone)
{
	if ((srcindex < 0) || (srcindex >= MAXBONEUPDATE)) {
		_ASSERT(0);
		return -1;
	}

	if (srcindex != m_bonenum) {
		_ASSERT(0);
		return -1;
	}

	m_bonelist[srcindex] = srcbone;

	m_bonenum++;

	return m_bonenum;
}

void CThreadingUpdateMatrix::UpdateMatrix(int srcmotid, double srcframe, ChaMatrix* srcwmat, ChaMatrix* srcvpmat)
{

	//####################################################################
	//## g_limitdegflag == 1　の場合にはローカルの計算だけ並列化
	//####################################################################

	if (m_bonenum > 0) {
		EnterCriticalSection(&m_CritSection);
		motid = srcmotid;
		frame = srcframe;
		wmat = *srcwmat;
		vpmat = *srcvpmat;
		LeaveCriticalSection(&m_CritSection);
		InterlockedExchange(&m_start_state, 1L);
		SetEvent(m_hEvent);
	}
	else {
		InterlockedExchange(&m_start_state, 0L);
	}


}



