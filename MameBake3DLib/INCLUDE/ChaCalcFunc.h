#ifndef CHACALCFUNCH
#define CHACALCFUNCH

#include <ChaVecCalc.h>



class ChaCalcFunc
{
	//#############################################################################################################
	//2023/10/17
	//このクラスの意味
	//マルチスレッドから　同期処理なしで呼び出し可能にするため
	//ChaCalcFuncのインスタンスをスレッドごとに作成し　インスタンス単位で実行中に再入しないようにするためのクラス
	//#############################################################################################################

public:
	ChaCalcFunc() {};
	~ChaCalcFunc() {};

	int ModifyEuler360(ChaVector3* eulerA, ChaVector3* eulerB, int notmodify180flag, float throundX, float throundY, float throundZ);
	int GetRoundThreshold(float srcval, float degth);


	int GetBefNextMP(CBone* srcbone, int srcmotid, double srcframe, CMotionPoint** ppbef, CMotionPoint** ppnext, int* existptr, bool onaddmotion = false);


	int IKRotateOneFrame(CModel* srcmodel, int limitdegflag, CEditRange* erptr,
		int keyno, CBone* rotbone, CBone* parentbone,
		int srcmotid, double curframe, double startframe, double applyframe,
		CQuaternion rotq0, bool keynum1flag, bool postflag, bool fromiktarget);
	int RotAndTraBoneQReq(CBone* srcbone, bool limitdegflag, int* onlycheckptr,
		double srcstartframe, bool infooutflag, CBone* parentbone, int srcmotid, double srcframe,
		CQuaternion qForRot, CQuaternion qForHipsRot, bool fromiktarget);


	int IKTargetVec(CModel* srcmodel, bool limitdegflag, CEditRange* erptr, int srcmotid, double srcframe, bool postflag);
	int IKRotateForIKTarget(CModel* srcmodel, bool limitdegflag, CEditRange* erptr,
		int srcboneno, int srcmotid, ChaVector3 targetpos, int maxlevel, double directframe, bool postflag);

	int AdjustBoneTra(CModel* srcmodel, bool limitdegflag, CEditRange* erptr, CBone* lastpar, int srcmotid);


	int FKBoneTra(CModel* srcmodel, bool limitdegflag, int onlyoneflag, CEditRange* erptr,
		int srcboneno, int srcmotid, ChaVector3 addtra, double onlyoneframe = 0.0);


	int CalcQForRot(bool limitdegflag, bool calcaplyflag,
		int srcmotid, double srcframe, double srcapplyframe, CQuaternion srcaddrot,
		CBone* srcrotbone, CBone* srcaplybone,
		CQuaternion* dstqForRot, CQuaternion* dstqForHipsRot);
	bool CalcAxisAndRotForIKRotateAxis(CModel* srcmodel, int limitdegflag,
		CBone* parentbone, CBone* firstbone,
		int srcmotid, double curframe, ChaVector3 targetpos,
		ChaVector3 srcikaxis,
		ChaVector3* dstaxis, float* dstrotrad);


	int CalcBoneEul(CModel* srcmodel, bool limitdegflag, int srcmotid);
	int CalcBoneEulOne(CModel* srcmodel, bool limitdegflag, CBone* curbone, int srcmotid, double startframe, double endframe);
	ChaVector3 CalcLocalEulXYZ(CBone* srcbone, bool limitdegflag, int axiskind,
		int srcmotid, double srcframe, tag_befeulkind befeulkind, ChaVector3* directbefeul);

	int ChkMovableEul(CBone* srcbone, ChaVector3 srceul);
	int SetLocalEul(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, ChaVector3 srceul, CMotionPoint* srcmp);
	ChaVector3 GetLocalEul(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, CMotionPoint* srcmp);


	ChaVector3 LimitEul(CBone* srcbone, ChaVector3 srceul);

	int SetWorldMat(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, ChaMatrix srcmat, CMotionPoint* srcmp);
	int SetWorldMat(CBone* srcbone, bool limitdegflag, bool directsetflag, bool infooutflag, int setchildflag, int srcmotid, double srcframe, ChaMatrix srcmat, int onlycheck, bool fromiktarget);
	int SetWorldMatFromEulAndScaleAndTra(CBone* srcbone, bool limitdegflag, int inittraflag, int setchildflag, ChaMatrix befwm, ChaVector3 srceul, ChaVector3 srcscale, ChaVector3 srctra, int srcmotid, double srcframe);

	CMotionPoint* GetMotionPoint(CBone* srcbone, int srcmotid, double srcframe, bool onaddmotion = false);
	ChaMatrix GetWorldMat(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe, CMotionPoint* srcmp, ChaVector3* dsteul = 0);

private:
	float LimitAngle(CBone* srcbone, enum tag_axiskind srckind, float srcval);




};




#endif
