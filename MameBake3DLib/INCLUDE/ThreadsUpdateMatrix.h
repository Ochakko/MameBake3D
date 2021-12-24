#pragma once

#ifndef THREADSUPDATEMATRIXH
#define THREADSUPDATEMATRIXH


#include <Coef.h>
#include <vector>


class CModel;
class CBone;

class CThreadsUpdateMatrix
{
public:
	CThreadsUpdateMatrix();
	~CThreadsUpdateMatrix();

	int CreateThreads();
	
	int UpdateMatrix(std::vector<MODELELEM>& modelarray, ChaMatrix* vpmat);
	int SetBtMotion(int (*srcsetbtmotionfunc)(double srcnextframe, CModel* curmodel), std::vector<MODELELEM>& modelarray, double nextframe);

	int WaitUpdateMatrix();

	//int UpdateEditedEuler(int (*srcupdatefunc)());
	int UpdateTimeline(int (*srctimelinecursor)(int mbuttonflag, double newframe), int srcmbuttonflag, double srcnewframe);

	void InitParams();
	void DestroyObjs();

public:
	bool m_updatethreadcreated;
	LONG m_underupdatematrix0;
	LONG m_underupdatematrix1;
	LONG m_underupdatematrix2;
	//LONG m_underupdatematrix3;
	HANDLE m_updatehandle0;
	HANDLE m_updatehandle1;
	HANDLE m_updatehandle2;
	//HANDLE m_updatehandle3;
	std::vector<CModel*> m_updatemodellist0;
	std::vector<CModel*> m_updatemodellist1;
	std::vector<CModel*> m_updatemodellist2;
	//std::vector<CModel*> m_updatemodellist3;


	LONG m_underupdatetimeline;
	HANDLE m_updatehandle4;
	//int (*m_UpdateEditedEulerFunc)();
	int (*m_OnTimeLineCursor)(int mbuttonflag, double newframe);
	int (*m_SetBtMotionFunc)(double srcnextframe, CModel* curmodel);

	ChaMatrix m_matVP;
	double m_nextframe;
	int m_mbuttonflag;
	double m_newframe;
};





#endif