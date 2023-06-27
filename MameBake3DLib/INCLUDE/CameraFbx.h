﻿#ifndef CAMERAFBXH
#define CAMERAFBXH

#include <map>
#include <string>

#include <Coef.h>
#include <ChaVecCalc.h>
#include <MotionPoint.h>


#include <fbxsdk.h>

class CBone;


typedef struct tag_cameranode
{
	FbxNode* pnode;
	CBone* pbone;
	FbxCamera* pcamera;
	ChaVector3 position;
	ChaMatrix worldmat;
	ChaVector3 dirvec;
	ChaVector3 upvec;//2023/06/25

	ChaMatrix localnodemat;

	ChaVector3 lcltra;
	ChaVector3 parentlcltra;
	ChaMatrix parentenullmat;
	ChaVector3 adjustpos;

	double aspectHeight; // アスペクト高
	double aspectWidth;  // アスペクト幅
	double nearZ;     // near平面距離
	double farZ;      // far平面距離
	double aspectRatio; // アスペクト比
	double inch_mm;    // インチ→ミリ
	double filmHeight;   // フィルム高（インチ）
	double focalLength; // 合焦距離（ミリ）
	double filmHeight_mm;
	double fovY;
	double fovY_Degree;



	void Init() {
		pnode = 0;
		pbone = 0;
		pcamera = 0;
		position.SetZeroVec3();
		worldmat.SetIdentity();

		localnodemat.SetIdentity();

		lcltra.SetZeroVec3();
		parentlcltra.SetZeroVec3();
		parentenullmat.SetIdentity();
		dirvec = ChaVector3(0.0f, 0.0f, 1.0f);
		upvec = ChaVector3(0.0f, 1.0f, 0.0f);
		adjustpos.SetZeroVec3();

		aspectHeight = 600.0f; // アスペクト高
		aspectWidth = 800.0f;  // アスペクト幅
		nearZ = 1.0f;     // near平面距離
		farZ = 1000.0f;      // far平面距離
		aspectRatio = 1.0f; // アスペクト比
		inch_mm = 25.4;    // インチ→ミリ
		filmHeight = 100.0f;   // フィルム高（インチ）
		focalLength = 100.0f; // 合焦距離（ミリ）
		filmHeight_mm = inch_mm * filmHeight;
		fovY = atan2(filmHeight_mm, 2.0 * focalLength);
		fovY_Degree = fovY * 180.0 / 3.14159265358979;

	};
	tag_cameranode() {
		Init();
	};

	int IsValid() {
		if (pnode && pbone && pcamera) {
			return 1;
		}
		else {
			return 0;
		}
	};

}CAMERANODE;



class CCameraFbx
{
public:
	CCameraFbx();
	~CCameraFbx();
	int Clear();
	//int SetFbxCamera(FbxNode* pnode, CBone* pbone);
	int AddFbxCamera(FbxNode* pnode, CBone* pbone);

	//int SetZeroFrameCamera();

	int GetCameraAnimParams(int cameramotid, double nextframe, double camdist, 
		ChaVector3* pEyePos, ChaVector3* pTargetPos, ChaVector3* pcamupdir, ChaMatrix* protmat, int inheritmode);
	ChaVector3 CalcCameraFbxEulXYZ(int cameramotid, double srcframe, ChaVector3 befeul);

	CAMERANODE* FindCameraNodeByNode(FbxNode* srcnode);
	CAMERANODE* FindCameraNodeByBone(CBone* srcbone);
	CAMERANODE* FindCameraNodeByMotId(int srcmotid);

	int PreLoadFbxAnim(CBone* srcbone, int srcmotid, ChaMatrix srcenullmat);

	//CCameraFbx operator= (CCameraFbx srcrange);
	//bool operator== (const CCameraFbx &cmp) const {
	//	return (
	//		(m_position == cmp.m_position) &&
	//		(m_dirvec == cmp.m_dirvec) &&
	//		(m_upVector == cmp.m_upVector) && 
	//		(m_aspectHeight == cmp.m_aspectHeight) && 
	//		(m_aspectWidth == cmp.m_aspectWidth) &&
	//		(m_nearZ == cmp.m_nearZ) &&
	//		(m_farZ == cmp.m_farZ) &&
	//		(m_aspectRatio == cmp.m_aspectRatio) &&
	//		//(m_inch_mm == cmp.m_inch_mm) &&
	//		(m_filmHeight == cmp.m_filmHeight) &&
	//		(m_focalLength == cmp.m_focalLength) &&
	//		//(m_filmHeight_mm == cmp.m_filmHeight_mm) &&
	//		(m_fovY == cmp.m_fovY)// && 
	//		//(m_fovY_Degree == cmp.m_fovY_Degree)
	//		); 
	//};
	//bool operator!= (const CCameraFbx &cmp) const { 
	//	return !(*this == cmp); 
	//};


private:
	int InitParams();
	int DestroyObjs();

public:
	bool IsLoaded();
	//ChaMatrix GetWorldMat()
	//{
	//	return m_worldmat;
	//}


	//ChaVector3 GetPosition()
	//{
	//	return m_position;
	//}
	//ChaVector3 GetTargetPos()
	//{
	//	_ASSERT(0);//現在　未設定
	//	return m_targetpos;
	//}
	//ChaVector3 GetDirVec()
	//{
	//	return m_dirvec;
	//}

	//ChaVector3 GetUpVector()
	//{
	//	return m_upVector;
	//}
	//double GetAspectHeight()
	//{
	//	return m_aspectHeight; // アスペクト高
	//}
	//double GetAspectWidth()
	//{
	//	return m_aspectWidth;  // アスペクト幅
	//}
	//double GetNearPlane()
	//{
	//	return m_nearZ;     // near平面距離
	//}
	//double GetFarPlane()
	//{
	//	return m_farZ;      // far平面距離
	//}
	//double GetAspectRatio()
	//{
	//	return m_aspectRatio; // アスペクト比
	//}
	//double GetFilmHeight()
	//{
	//	return m_filmHeight;   // フィルム高（インチ）
	//}
	//double GetFocalLength()
	//{
	//	return m_focalLength; // 合焦距離（ミリ）
	//}
	//double GetFilmHeightMM()
	//{
	//	return m_filmHeight_mm;
	//}
	//double GetFovY()
	//{
	//	return m_fovY;
	//}
	//double GetFovYDegree()
	//{
	//	return m_fovY_Degree;
	//}

	//FbxCamera* GetFbxCamera()
	//{
	//	return m_pcamera;
	//}
	//FbxNode* GetFbxNode()
	//{
	//	return m_pnode;
	//}
	//CBone* GetBone()
	//{
	//	return m_pbone;
	//}

	//ChaVector3 GetCameraLclTra()
	//{
	//	return m_cameralcltra;
	//}
	//void SetCameraLclTra(FbxDouble3 srcval)
	//{
	//	m_cameralcltra = ChaVector3((float)srcval[0], (float)srcval[1], (float)srcval[2]);
	//}
	//ChaVector3 GetCameraParentLclTra()
	//{
	//	return m_cameraparentlcltra;
	//}
	//void SetCameraParentLclTra(ChaVector3 srcval)
	//{
	//	m_cameraparentlcltra = srcval;
	//}

	//char* GetAnimName()
	//{
	//	return &(m_animname[0]);
	//}
	//FbxScene* GetScene()
	//{
	//	return m_pscene;
	//}
	//void SetAnimLayer(FbxScene* pscene, char* animname)
	//{
	//	if (pscene) {
	//		m_pscene = pscene;
	//	}
	//	if (animname) {
	//		strcpy_s(m_animname, 256, animname);
	//	}
	//}
	//ChaMatrix GetCameraParentENullMat()
	//{
	//	return m_parentenullmat;
	//}
	//void SetCameraParentENullMat(ChaMatrix enullmat)
	//{
	//	m_parentenullmat = enullmat;
	//}

private:
	bool m_loadedflag;

	std::vector<CAMERANODE*> m_cameranode;
	std::map<int, CAMERANODE*> m_cameramotion;//<motid, cameranode>


	double m_time;

	FbxScene* m_pscene;
	char m_animname[256];
};

#endif



