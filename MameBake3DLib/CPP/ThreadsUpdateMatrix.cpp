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

#include <ThreadsUpdateMatrix.h>

#define DBGH
#include <dbg.h>




unsigned __stdcall ThreadFunc_UpdateMatrix0(LPVOID lpThreadParam)
{
	if (!lpThreadParam) {
		return 0;
	}
	CThreadsUpdateMatrix* ptum = (CThreadsUpdateMatrix*)lpThreadParam;
	if (!ptum) {
		return 0;
	}

	while (1) {
		if ((InterlockedAdd(&(ptum->m_underupdatematrix0), 0) == 3)) {
			break;
		}
		if ((InterlockedAdd(&(ptum->m_underupdatematrix0), 0) == 4)) {
			timeBeginPeriod(1);
			SleepEx(0, TRUE);
			timeEndPeriod(1);
		}

		if ((InterlockedAdd(&(ptum->m_underupdatematrix0), 0) == 1)) {
			int modelnum = ptum->m_updatemodellist0.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix0), (LONG)2);
				continue;
			}

			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist0[modelno];
				if (curmodel) {
					ChaMatrix tmpwm = curmodel->GetWorldMat();
					curmodel->UpdateMatrix(&tmpwm, &(ptum->m_matVP));
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix0), (LONG)2);
		}

		if ((InterlockedAdd(&(ptum->m_underupdatematrix0), 0) == 10)) {
			if (!ptum->m_SetBtMotionFunc) {
				//InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)2);
				InterlockedExchange(&(ptum->m_underupdatematrix0), (LONG)2);
				continue;
			}
			int modelnum = ptum->m_updatemodellist0.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix0), (LONG)2);
				continue;
			}
			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist0[modelno];
				if (curmodel) {
					(ptum->m_SetBtMotionFunc)(ptum->m_nextframe, curmodel);
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix0), (LONG)2);
		}
	}

	return 0;
}
unsigned __stdcall ThreadFunc_UpdateMatrix1(LPVOID lpThreadParam)
{
	if (!lpThreadParam) {
		return 0;
	}
	CThreadsUpdateMatrix* ptum = (CThreadsUpdateMatrix*)lpThreadParam;
	if (!ptum) {
		return 0;
	}

	while (1) {
		if ((InterlockedAdd(&(ptum->m_underupdatematrix1), 0) == 3)) {
			break;
		}
		if ((InterlockedAdd(&(ptum->m_underupdatematrix1), 0) == 4)) {
			timeBeginPeriod(1);
			SleepEx(0, TRUE);
			timeEndPeriod(1);
		}
		if ((InterlockedAdd(&(ptum->m_underupdatematrix1), 0) == 1)) {
			int modelnum = ptum->m_updatemodellist1.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix1), (LONG)2);
				continue;
			}

			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist1[modelno];
				if (curmodel) {
					ChaMatrix tmpwm = curmodel->GetWorldMat();
					curmodel->UpdateMatrix(&tmpwm, &(ptum->m_matVP));
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix1), (LONG)2);
		}

		if ((InterlockedAdd(&(ptum->m_underupdatematrix1), 0) == 10)) {
			if (!ptum->m_SetBtMotionFunc) {
				//InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)2);
				InterlockedExchange(&(ptum->m_underupdatematrix1), (LONG)2);
				continue;
			}
			int modelnum = ptum->m_updatemodellist1.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix1), (LONG)2);
				continue;
			}
			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist1[modelno];
				if (curmodel) {
					(ptum->m_SetBtMotionFunc)(ptum->m_nextframe, curmodel);
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix1), (LONG)2);
		}


	}

	return 0;
}
unsigned __stdcall ThreadFunc_UpdateMatrix2(LPVOID lpThreadParam)
{
	if (!lpThreadParam) {
		return 0;
	}
	CThreadsUpdateMatrix* ptum = (CThreadsUpdateMatrix*)lpThreadParam;
	if (!ptum) {
		return 0;
	}

	while (1) {
		if ((InterlockedAdd(&(ptum->m_underupdatematrix2), 0) == 3)) {
			break;
		}
		if ((InterlockedAdd(&(ptum->m_underupdatematrix2), 0) == 4)) {
			timeBeginPeriod(1);
			SleepEx(0, TRUE);
			timeEndPeriod(1);
		}
		if ((InterlockedAdd(&(ptum->m_underupdatematrix2), 0) == 1)) {
			int modelnum = ptum->m_updatemodellist2.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix2), (LONG)2);
				continue;
			}

			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist2[modelno];
				if (curmodel) {
					ChaMatrix tmpwm = curmodel->GetWorldMat();
					curmodel->UpdateMatrix(&tmpwm, &(ptum->m_matVP));
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix2), (LONG)2);
		}

		if ((InterlockedAdd(&(ptum->m_underupdatematrix2), 0) == 10)) {
			if (!ptum->m_SetBtMotionFunc) {
				//InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)2);
				InterlockedExchange(&(ptum->m_underupdatematrix2), (LONG)2);
				continue;
			}
			int modelnum = ptum->m_updatemodellist2.size();
			if (modelnum <= 0) {
				InterlockedExchange(&(ptum->m_underupdatematrix2), (LONG)2);
				continue;
			}
			int modelno;
			for (modelno = 0; modelno < modelnum; modelno++) {
				CModel* curmodel = ptum->m_updatemodellist2[modelno];
				if (curmodel) {
					(ptum->m_SetBtMotionFunc)(ptum->m_nextframe, curmodel);
				}
			}
			InterlockedExchange(&(ptum->m_underupdatematrix2), (LONG)2);
		}

	}

	return 0;
}


unsigned __stdcall ThreadFunc_OnTimeLineCursor(LPVOID lpThreadParam)
{
	if (!lpThreadParam) {
		return 0;
	}
	CThreadsUpdateMatrix* ptum = (CThreadsUpdateMatrix*)lpThreadParam;
	if (!ptum) {
		return 0;
	}

	while (1) {
		if ((InterlockedAdd(&(ptum->m_underupdatetimeline), 0) == 3)) {
			break;
		}
		if ((InterlockedAdd(&(ptum->m_underupdatetimeline), 0) == 4)) {
			timeBeginPeriod(1);
			SleepEx(0, TRUE);
			//SleepEx(1, TRUE);
			timeEndPeriod(1);
		}
		if ((InterlockedAdd(&(ptum->m_underupdatetimeline), 0) == 1)) {
			if (!ptum->m_OnTimeLineCursor) {
				//InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)2);
				InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)4);
				continue;
			}

			//(ptum->m_UpdateEditedEulerFunc)();
			(ptum->m_OnTimeLineCursor)(ptum->m_mbuttonflag, ptum->m_newframe);


			//InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)2);
			InterlockedExchange(&(ptum->m_underupdatetimeline), (LONG)4);
		}
	}

	return 0;
}


CThreadsUpdateMatrix::CThreadsUpdateMatrix()
{
	InitParams();
}
CThreadsUpdateMatrix::~CThreadsUpdateMatrix()
{
	DestroyObjs();
}


void CThreadsUpdateMatrix::InitParams()
{
	m_updatethreadcreated = false;
	m_underupdatematrix0 = 0;
	m_underupdatematrix1 = 0;
	m_underupdatematrix2 = 0;
	//m_underupdatematrix3 = 0;
	m_updatehandle0 = INVALID_HANDLE_VALUE;
	m_updatehandle1 = INVALID_HANDLE_VALUE;
	m_updatehandle2 = INVALID_HANDLE_VALUE;
	//m_updatehandle3 = INVALID_HANDLE_VALUE;
	m_updatemodellist0.clear();
	m_updatemodellist1.clear();
	m_updatemodellist2.clear();
	//m_updatemodellist3.clear();

	m_underupdatetimeline = 0;
	m_updatehandle4 = INVALID_HANDLE_VALUE;

	m_OnTimeLineCursor = 0;
	m_SetBtMotionFunc = 0;

	ChaMatrixIdentity(&m_matVP);
	m_nextframe = 0.0;

	m_mbuttonflag = 1;
	m_newframe = 0.0;

}
void CThreadsUpdateMatrix::DestroyObjs()
{
	InterlockedExchange(&m_underupdatematrix0, (LONG)3);
	InterlockedExchange(&m_underupdatematrix1, (LONG)3);
	InterlockedExchange(&m_underupdatematrix2, (LONG)3);
	//InterlockedExchange(&m_underupdatematrix3, (LONG)3);
	InterlockedExchange(&m_underupdatetimeline, (LONG)3);

	Sleep(100);
}

int CThreadsUpdateMatrix::CreateThreads()
{


	//unsigned int threadaddr0 = 0;
	//m_updatehandle0 = (HANDLE)_beginthreadex(
	//	NULL, 0, &ThreadFunc_UpdateMatrix0,
	//	(void*)this,
	//	0, &threadaddr0);

	//unsigned int threadaddr1 = 0;
	//m_updatehandle1 = (HANDLE)_beginthreadex(
	//	NULL, 0, &ThreadFunc_UpdateMatrix1,
	//	(void*)this,
	//	0, &threadaddr1);

	//unsigned int threadaddr2 = 0;
	//m_updatehandle2 = (HANDLE)_beginthreadex(
	//	NULL, 0, &ThreadFunc_UpdateMatrix2,
	//	(void*)this,
	//	0, &threadaddr2);

	////unsigned int threadaddr3 = 0;
	////m_updatehandle3 = (HANDLE)_beginthreadex(
	////	NULL, 0, &ThreadFunc_UpdateMatrix3,
	////	(void*)this,
	////	0, &threadaddr3);

	unsigned int threadaddr4 = 0;
	m_updatehandle4 = (HANDLE)_beginthreadex(
		NULL, 0, &ThreadFunc_OnTimeLineCursor,
		(void*)this,
		0, &threadaddr4);



	//if (m_updatehandle0 && (m_updatehandle0 != INVALID_HANDLE_VALUE)) {
	//	CloseHandle(m_updatehandle0);
	//}
	//if (m_updatehandle1 && (m_updatehandle1 != INVALID_HANDLE_VALUE)) {
	//	CloseHandle(m_updatehandle1);
	//}
	//if (m_updatehandle2 && (m_updatehandle2 != INVALID_HANDLE_VALUE)) {
	//	CloseHandle(m_updatehandle2);
	//}
	////if (m_updatehandle3 && (m_updatehandle3 != INVALID_HANDLE_VALUE)) {
	////	CloseHandle(m_updatehandle3);
	////}
	if (m_updatehandle4 && (m_updatehandle4 != INVALID_HANDLE_VALUE)) {
		CloseHandle(m_updatehandle4);
	}

	//InterlockedExchange(&m_underupdatematrix0, (LONG)4);
	//InterlockedExchange(&m_underupdatematrix1, (LONG)4);
	//InterlockedExchange(&m_underupdatematrix2, (LONG)4);
	////InterlockedExchange(&m_underupdatematrix3, (LONG)4);
	InterlockedExchange(&m_underupdatetimeline, (LONG)4);

	m_updatethreadcreated = true;


	return 0;
}


int CThreadsUpdateMatrix::UpdateMatrix(std::vector<MODELELEM>& modelarray, ChaMatrix* vpmat)
{

	//if (m_updatethreadcreated == true) {

	//	//再入しない
	//	if ((InterlockedAdd(&m_underupdatematrix0, 0) == 1) || 
	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 1) ||
	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 1) ||
	//		(InterlockedAdd(&m_underupdatematrix0, 0) == 10) ||
	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 10) ||
	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 10)
	//		//(InterlockedAdd(&m_underupdatematrix3, 0) != 2)
	//		) {
	//		return 0;
	//	}


	//	m_matVP = *vpmat;

	//	m_updatemodellist0.clear();
	//	m_updatemodellist1.clear();
	//	m_updatemodellist2.clear();
	//	//m_updatemodellist3.clear();

	//	int listno = 0;
	//	std::vector<MODELELEM>::iterator itrmodelelem;
	//	for (itrmodelelem = modelarray.begin(); itrmodelelem != modelarray.end(); itrmodelelem++) {
	//		CModel* curmodel = (*itrmodelelem).modelptr;
	//		if (curmodel) {
	//			//curbone->UpdateMatrix( curmotid, curframe, wmat, vpmat );
	//			switch (listno) {
	//			case 0:
	//				m_updatemodellist0.push_back(curmodel);
	//				break;
	//			case 1:
	//				m_updatemodellist1.push_back(curmodel);
	//				break;
	//			case 2:
	//				m_updatemodellist2.push_back(curmodel);
	//				break;
	//			//case 3:
	//			//	m_updatemodellist3.push_back(curmodel);
	//			//	break;
	//			default:
	//				break;
	//			}

	//			listno++;
	//			//if (listno >= 4) {
	//			if (listno >= 3) {
	//				listno = 0;
	//			}
	//		}
	//	}

	//	InterlockedExchange(&m_underupdatematrix0, (LONG)1);
	//	InterlockedExchange(&m_underupdatematrix1, (LONG)1);
	//	InterlockedExchange(&m_underupdatematrix2, (LONG)1);
	//	//InterlockedExchange(&m_underupdatematrix3, (LONG)1);

	//	//while (1) {
	//	//	if ((InterlockedAdd(&m_underupdatematrix0, 0) == 2) &&
	//	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 2) &&
	//	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 2)){ // &&
	//	//		//(InterlockedAdd(&m_underupdatematrix3, 0) == 2)) {
	//	//		break;
	//	//	}
	//	//	timeBeginPeriod(1);
	//	//	SleepEx(0, TRUE);
	//	//	timeEndPeriod(1);
	//	//}

	//	//InterlockedExchange(&m_underupdatematrix0, (LONG)4);
	//	//InterlockedExchange(&m_underupdatematrix1, (LONG)4);
	//	//InterlockedExchange(&m_underupdatematrix2, (LONG)4);
	//	////InterlockedExchange(&m_underupdatematrix3, (LONG)4);

	//}
	//else {
	//	//map<int, CBone*>::iterator itrbone;
	//	//for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
	//	//	CBone* curbone = itrbone->second;
	//	//	if (curbone) {
	//	//		curbone->UpdateMatrix(curmotid, curframe, wmat, vpmat);
	//	//	}
	//	//}
	//}



	return 0;
}

int CThreadsUpdateMatrix::SetBtMotion(int (*srcsetbtmotionfunc)(double srcnextframe, CModel* curmodel), std::vector<MODELELEM>& modelarray, double nextframe)
{
	//if (m_updatethreadcreated == true) {

	//	//再入しない
	//	if ((InterlockedAdd(&m_underupdatematrix0, 0) == 1) ||
	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 1) ||
	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 1) ||
	//		(InterlockedAdd(&m_underupdatematrix0, 0) == 10) ||
	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 10) ||
	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 10)
	//		//(InterlockedAdd(&m_underupdatematrix3, 0) != 2)
	//		) {
	//		return 0;
	//	}

	//	if (!srcsetbtmotionfunc) {
	//		return 0;
	//	}


	//	m_SetBtMotionFunc = srcsetbtmotionfunc;
	//	m_nextframe = nextframe;

	//	m_updatemodellist0.clear();
	//	m_updatemodellist1.clear();
	//	m_updatemodellist2.clear();
	//	//m_updatemodellist3.clear();

	//	int listno = 0;
	//	std::vector<MODELELEM>::iterator itrmodelelem;
	//	for (itrmodelelem = modelarray.begin(); itrmodelelem != modelarray.end(); itrmodelelem++) {
	//		CModel* curmodel = (*itrmodelelem).modelptr;
	//		if (curmodel) {
	//			//curbone->UpdateMatrix( curmotid, curframe, wmat, vpmat );
	//			switch (listno) {
	//			case 0:
	//				m_updatemodellist0.push_back(curmodel);
	//				break;
	//			case 1:
	//				m_updatemodellist1.push_back(curmodel);
	//				break;
	//			case 2:
	//				m_updatemodellist2.push_back(curmodel);
	//				break;
	//				//case 3:
	//				//	m_updatemodellist3.push_back(curmodel);
	//				//	break;
	//			default:
	//				break;
	//			}

	//			listno++;
	//			//if (listno >= 4) {
	//			if (listno >= 3) {
	//				listno = 0;
	//			}
	//		}
	//	}

	//	InterlockedExchange(&m_underupdatematrix0, (LONG)10);
	//	InterlockedExchange(&m_underupdatematrix1, (LONG)10);
	//	InterlockedExchange(&m_underupdatematrix2, (LONG)10);
	//	//InterlockedExchange(&m_underupdatematrix3, (LONG)1);

	//	//while (1) {
	//	//	if ((InterlockedAdd(&m_underupdatematrix0, 0) == 2) &&
	//	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 2) &&
	//	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 2)){ // &&
	//	//		//(InterlockedAdd(&m_underupdatematrix3, 0) == 2)) {
	//	//		break;
	//	//	}
	//	//	timeBeginPeriod(1);
	//	//	SleepEx(0, TRUE);
	//	//	timeEndPeriod(1);
	//	//}

	//	//InterlockedExchange(&m_underupdatematrix0, (LONG)4);
	//	//InterlockedExchange(&m_underupdatematrix1, (LONG)4);
	//	//InterlockedExchange(&m_underupdatematrix2, (LONG)4);
	//	////InterlockedExchange(&m_underupdatematrix3, (LONG)4);

	//}
	//else {
	//	//map<int, CBone*>::iterator itrbone;
	//	//for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
	//	//	CBone* curbone = itrbone->second;
	//	//	if (curbone) {
	//	//		curbone->UpdateMatrix(curmotid, curframe, wmat, vpmat);
	//	//	}
	//	//}
	//}



	return 0;
}




int CThreadsUpdateMatrix::WaitUpdateMatrix()
{

	//while (1) {
	//	if ((InterlockedAdd(&m_underupdatematrix0, 0) == 2) &&
	//		(InterlockedAdd(&m_underupdatematrix1, 0) == 2) &&
	//		(InterlockedAdd(&m_underupdatematrix2, 0) == 2)) { // &&
	//		//(InterlockedAdd(&m_underupdatematrix3, 0) == 2)) {
	//		break;
	//	}
	//	timeBeginPeriod(1);
	//	SleepEx(0, TRUE);
	//	timeEndPeriod(1);
	//}

	//InterlockedExchange(&m_underupdatematrix0, (LONG)4);
	//InterlockedExchange(&m_underupdatematrix1, (LONG)4);
	//InterlockedExchange(&m_underupdatematrix2, (LONG)4);
	////InterlockedExchange(&m_underupdatematrix3, (LONG)4);

	return 0;
}



int CThreadsUpdateMatrix::UpdateTimeline(int (*srctimelinecursor)(int mbuttonflag, double newframe), int srcmbuttonflag, double srcnewframe)
{
	if (!srctimelinecursor) {
		return 0;
	}

	if (m_updatethreadcreated == true) {

		//実行中だった場合はすぐにリターン。再入しない。
		if (InterlockedAdd(&m_underupdatetimeline, 0) == 1) {
			return 0;
		}

		m_OnTimeLineCursor = srctimelinecursor;
		m_mbuttonflag = srcmbuttonflag;
		m_newframe = srcnewframe;


		InterlockedExchange(&m_underupdatetimeline, (LONG)1);

	}
	else {
		//map<int, CBone*>::iterator itrbone;
		//for (itrbone = m_bonelist.begin(); itrbone != m_bonelist.end(); itrbone++) {
		//	CBone* curbone = itrbone->second;
		//	if (curbone) {
		//		curbone->UpdateMatrix(curmotid, curframe, wmat, vpmat);
		//	}
		//}
	}



	return 0;
}
