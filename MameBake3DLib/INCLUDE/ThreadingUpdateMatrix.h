#ifndef THREADINGUPDATEMATRIXH
#define THREADINGUPDATEMATRIXH


#include <ThreadingBase.h>
#include <ChaVecCalc.h>


//�X���b�h�P�̏ꍇ������̂�512�܂łɂ���
//#define MAXBONEUPDATE	256
#define MAXBONEUPDATE	512


class CBone;
class CThreadingUpdateMatrix : public CThreadingBase
{
public:
	CThreadingUpdateMatrix();
	virtual ~CThreadingUpdateMatrix();

	//int CreateThread();
	virtual int InitParams();
	virtual void DestroyObjs();

	int ClearBoneList();
	int SetBoneList(int srcindex, CBone* srcbone);
	void UpdateMatrix(int srcmotid, double srcframe, ChaMatrix* wmat, ChaMatrix* vpmat);
	//bool IsFinished();

private:
	//static unsigned __stdcall ThreadFuncCaller(LPVOID lpThreadParam);
	virtual int ThreadFunc();


private:
	//CRITICAL_SECTION m_CritSection_UpdateMatrix;
	//HANDLE m_hEvent; //�蓮���Z�b�g�C�x���g
	//HANDLE m_hExitEvent; //�蓮���Z�b�g�C�x���g

	//HANDLE m_hthread;
	//LONG m_exit_state;
	//LONG m_start_state;

	int m_bonenum;
	CBone* m_bonelist[MAXBONEUPDATE];

	int motid;
	double frame;
	ChaMatrix wmat;
	ChaMatrix vpmat;
};


#endif