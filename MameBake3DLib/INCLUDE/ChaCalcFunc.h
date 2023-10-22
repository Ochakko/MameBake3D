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
		int srcboneno, int srcmotid, ChaVector3 addtra, double onlyonefarme = 0.0);

	//for threading
	int FKBoneTraOneFrame(CModel* srcmodel, bool limitdegflag, CEditRange* erptr,
		int srcboneno, int srcmotid, double srcframe, ChaVector3 addtra);

	CMotionPoint* AddBoneTraReq(CBone* srcbone, bool limitdegflag, CMotionPoint* parmp, int srcmotid, double srcframe, ChaVector3 srctra, ChaMatrix befparentwm, ChaMatrix newparentwm);


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

	int CopyWorldToLimitedWorldOne(CModel* srcmodel, CBone* srcbone, int srcmotid, double srcframe);
	void CopyWorldToLimitedWorldReq(CModel* srcmodel, CBone* srcbone, int srcmotid, double srcframe);
	int CopyWorldToLimitedWorld(CBone* srcbone, int srcmotid, double srcframe);
	void UpdateCurrentWM(CBone* srcbone, bool limitdegflag, int srcmotid, double srcframe,
		ChaMatrix newwm);
	void UpdateParentWMReq(CBone* srcbone, bool limitdegflag, bool setbroflag, int srcmotid, double srcframe,
		ChaMatrix oldparentwm, ChaMatrix newparentwm);


//#################################################################
//Following Functions are Called From GlobalFunctions at ChaVecCalc
//#################################################################
	BOOL IsValidNewEul(ChaVector3 srcneweul, ChaVector3 srcbefeul);
	ChaMatrix ChaMatrixTranspose(ChaMatrix srcmat);
	double ChaVector3LengthDbl(ChaVector3* psrc);
	double ChaVector3DotDbl(const ChaVector3* psrc1, const ChaVector3* psrc2);
	//float ChaVector3LengthDbl(ChaVector3* psrc);
	void ChaVector3Normalize(ChaVector3* pdst, const ChaVector3* psrc);
	float ChaVector3Dot(const ChaVector3* psrc1, const ChaVector3* psrc2);
	void ChaVector3Cross(ChaVector3* pdst, const ChaVector3* psrc1, const ChaVector3* psrc2);
	void ChaVector3TransformCoord(ChaVector3* pdst, ChaVector3* psrc, ChaMatrix* pmat);
	double ChaVector3LengthSqDbl(ChaVector3* psrc);
	ChaVector3* ChaVector3TransformNormal(ChaVector3* pOut, const ChaVector3* pV, const ChaMatrix* pM);
	void ChaMatrixIdentity(ChaMatrix* pdst);
	ChaMatrix ChaMatrixScale(ChaMatrix srcmat);//スケール成分だけの行列にする
	ChaMatrix ChaMatrixRot(ChaMatrix srcmat);//回転成分だけの行列にする
	ChaMatrix ChaMatrixTra(ChaMatrix srcmat);//移動成分だけの行列にする
	ChaVector3 ChaMatrixScaleVec(ChaMatrix srcmat);//スケール成分のベクトルを取得
	ChaVector3 ChaMatrixRotVec(ChaMatrix srcmat, int notmodify180flag);//回転成分のベクトルを取得
	ChaVector3 ChaMatrixTraVec(ChaMatrix srcmat);//移動成分のベクトルを取得
	CQuaternion ChaMatrix2Q(ChaMatrix srcmat);//ChaMatrixを受け取って　CQuaternionを返す
	void ChaMatrixNormalizeRot(ChaMatrix* pdst);
	void ChaMatrixInverse(ChaMatrix* pdst, float* pdet, const ChaMatrix* psrc);
	void ChaMatrixTranslation(ChaMatrix* pdst, float srcx, float srcy, float srcz);
	void ChaMatrixTranspose(ChaMatrix* pdst, ChaMatrix* psrc);
	void ChaMatrixRotationAxis(ChaMatrix* pdst, ChaVector3* srcaxis, float srcrad);
	void ChaMatrixScaling(ChaMatrix* pdst, float srcx, float srcy, float srcz);
	void ChaMatrixLookAtRH(ChaMatrix* dstviewmat, ChaVector3* camEye, ChaVector3* camtar, ChaVector3* camUpVec);
	ChaMatrix* ChaMatrixOrthoOffCenterRH(ChaMatrix* pOut, float l, float r, float t, float b, float zn, float zf);
	ChaMatrix* ChaMatrixPerspectiveFovRH(ChaMatrix* pOut, float fovY, float Aspect, float zn, float zf);
	ChaMatrix* ChaMatrixRotationYawPitchRoll(ChaMatrix* pOut, float srcyaw, float srcpitch, float srcroll);
	ChaMatrix* ChaMatrixRotationX(ChaMatrix* pOut, float srcrad);
	ChaMatrix* ChaMatrixRotationY(ChaMatrix* pOut, float srcrad);
	ChaMatrix* ChaMatrixRotationZ(ChaMatrix* pOut, float srcrad);
	const ChaMatrix* ChaMatrixRotationQuaternion(ChaMatrix* dstmat, CQuaternion* srcq);
	void CQuaternionIdentity(CQuaternion* dstq);
	CQuaternion CQuaternionInv(CQuaternion srcq);
	double vecDotVec(ChaVector3* vec1, ChaVector3* vec2);
	double lengthVec(ChaVector3* vec);
	double aCos(double dot);
	int vec3RotateY(ChaVector3* dstvec, double deg, ChaVector3* srcvec);
	int vec3RotateX(ChaVector3* dstvec, double deg, ChaVector3* srcvec);
	int vec3RotateZ(ChaVector3* dstvec, double deg, ChaVector3* srcvec);
	int GetRound(float srcval);
	int IsInitRot(ChaMatrix srcmat);
	int IsSameMat(ChaMatrix srcmat1, ChaMatrix srcmat2);
	int IsSameEul(ChaVector3 srceul1, ChaVector3 srceul2);
	bool IsJustEqualTime(double srctime1, double srctime2);
	double RoundingTime(double srctime);
	int IntTime(double srctime);
	bool IsEqualRoundingTime(double srctime1, double srctime2);
	double VecLength(ChaVector3 srcvec);
	void GetSRTMatrix(ChaMatrix srcmat, ChaVector3* svecptr, ChaMatrix* rmatptr, ChaVector3* tvecptr);
	void GetSRTMatrix2(ChaMatrix srcmat, ChaMatrix* smatptr, ChaMatrix* rmatptr, ChaMatrix* tmatptr);
	void GetSRTandTraAnim(ChaMatrix srcmat, ChaMatrix srcnodemat, ChaMatrix* smatptr, ChaMatrix* rmatptr, ChaMatrix* tmatptr, ChaMatrix* tanimmatptr);//For Local Posture
	ChaMatrix ChaMatrixFromSRT(bool sflag, bool tflag, ChaMatrix srcnodemat, ChaMatrix* srcsmat, ChaMatrix* srcrmat, ChaMatrix* srctmat);//For Local Posture
	ChaMatrix ChaMatrixFromSRTraAnim(bool sflag, bool tanimflag, ChaMatrix srcnodemat, ChaMatrix* srcsmat, ChaMatrix* srcrmat, ChaMatrix* srctanimmat);//For Local Posture
	ChaMatrix GetS0RTMatrix(ChaMatrix srcmat);//拡大縮小を初期化したRT行列を返す
	ChaMatrix ChaMatrixKeepScale(ChaMatrix srcmat, ChaVector3 srcsvec);
	ChaMatrix TransZeroMat(ChaMatrix srcmat);
	ChaMatrix ChaMatrixFromBtMat3x3(btMatrix3x3* srcmat3x3);
	ChaMatrix ChaMatrixFromBtTransform(btMatrix3x3* srcmat3x3, btVector3* srcpipot);
	ChaMatrix ChaMatrixInv(ChaMatrix srcmat);
	CQuaternion QMakeFromBtMat3x3(btMatrix3x3* eulmat);
	ChaMatrix MakeRotMatFromChaMatrix(ChaMatrix srcmat);
	ChaMatrix ChaMatrixFromFbxAMatrix(FbxAMatrix srcmat);
	ChaMatrix CalcAxisMatX(ChaVector3 vecx, ChaVector3 srcpos, ChaMatrix srcmat);
	int IsInitMat(ChaMatrix srcmat);
	double ChaVector3LengthSq(ChaVector3* psrc);



private:
	float LimitAngle(CBone* srcbone, enum tag_axiskind srckind, float srcval);




};




#endif
