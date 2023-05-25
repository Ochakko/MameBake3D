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

#define DBGH
#include <dbg.h>

#include <CameraFbx.h>

using namespace std;


CCameraFbx::CCameraFbx()
{
	InitParams();
}
CCameraFbx::~CCameraFbx()
{
	DestroyObjs();
}

int CCameraFbx::InitParams()
{
	m_pcamera = 0;
	m_loadedflag = false;

	m_time = 0.0;

	m_worldmat.SetIdentity();

	m_position = ChaVector3(0.0f, 0.0f, 10.0f);
	m_targetpos = ChaVector3(0.0f, 0.0f, 0.0f);
	m_dirvec = ChaVector3(1.0f, 0.0f, 0.0f);

	m_upVector = ChaVector3(0.0f, 1.0f, 0.0f);     // �A�b�v�x�N�g��
	m_aspectHeight = 1.0; // �A�X�y�N�g��
	m_aspectWidth = 1.0;  // �A�X�y�N�g��
	m_nearZ = 0.1;     // near���ʋ���
	m_farZ = 1000;      // far���ʋ���
	m_aspectRatio = 1.0; // �A�X�y�N�g��
	m_inch_mm = 25.4;    // �C���`���~��
	m_filmHeight = 1.0;   // �t�B�������i�C���`�j
	m_focalLength = 100.0; // ���ŋ����i�~���j
	m_filmHeight_mm = m_inch_mm * m_filmHeight;
	m_fovY = atan2(m_filmHeight_mm, 2.0 * m_focalLength);
	m_fovY_Degree = m_fovY * 180 / 3.14159265358979;

	return 0;
}
int CCameraFbx::DestroyObjs()
{
	InitParams();

	return 0;
}

int CCameraFbx::Clear()
{
	InitParams();
	return 0;
}

int CCameraFbx::SetFbxCamera(FbxNode* pNode)
{
	if (!pNode) {
		_ASSERT(0);
		m_loadedflag = false;
		m_pcamera = 0;
		return 1;
	}

	m_pnode = pNode;
	m_pcamera = pNode->GetCamera();

	if (m_pcamera) {

		int result = 0;
		result = ProcessCameraAnim(0.0);//�w�莞�Ԃɂ�����@m_time, m_position, m_dirvec, m_worldmat�̃Z�b�g
		if (result != 0) {
			_ASSERT(0);
			return 1;
		}

		FbxVector4 upVector = m_pcamera->UpVector.Get();     // �A�b�v�x�N�g��
		FbxDouble aspectHeight = m_pcamera->AspectHeight.Get(); // �A�X�y�N�g��
		FbxDouble aspectWidth = m_pcamera->AspectWidth.Get();  // �A�X�y�N�g��

		//FbxDouble nearZ = m_pcamera->GetNearPlane();     // near���ʋ���
		//FbxDouble farZ = m_pcamera->GetFarPlane();      // far���ʋ���
		FbxDouble nearZ = m_pcamera->NearPlane.Get();     // near���ʋ���
		FbxDouble farZ = m_pcamera->FarPlane.Get();      // far���ʋ���
		
		FbxDouble aspectRatio = aspectWidth / aspectHeight; // �A�X�y�N�g��
		FbxDouble inch_mm = 25.4;    // �C���`���~��
		FbxDouble filmHeight = m_pcamera->FilmHeight.Get();   // �t�B�������i�C���`�j
		FbxDouble focalLength = m_pcamera->FocalLength.Get(); // ���ŋ����i�~���j
		FbxDouble filmHeight_mm = inch_mm * filmHeight;


		//FbxDouble fovY = atan2(filmHeight_mm, 2.0 * focalLength);
		FbxDouble fovY_Degree = m_pcamera->ComputeFieldOfView(focalLength);//degree
		FbxDouble fovY = fovY_Degree * 3.14159265358979 / 180.0;


		m_upVector = ChaVector3((float)upVector[0], (float)upVector[1], (float)upVector[2]);     // �A�b�v�x�N�g��
		m_aspectHeight = aspectHeight; // �A�X�y�N�g��
		m_aspectWidth = aspectWidth;  // �A�X�y�N�g��
		m_nearZ = nearZ;     // near���ʋ���
		m_farZ = farZ;      // far���ʋ���
		m_aspectRatio = aspectRatio; // �A�X�y�N�g��
		m_inch_mm = inch_mm;    // �C���`���~��
		m_filmHeight = filmHeight;   // �t�B�������i�C���`�j
		m_focalLength = focalLength; // ���ŋ����i�~���j
		m_filmHeight_mm = filmHeight_mm;
		m_fovY = fovY;
		m_fovY_Degree = fovY_Degree;

		m_loadedflag = true;



	}


	return 0;

}

int CCameraFbx::ProcessCameraAnim(double nextframe)
{

	//��������(SetFbxCamera())�ɂ��ĂԂ̂Ł@IsLoaded() == false�ł�����������



	//#########################################################
	//2023/05/25
	// �J�����A�j���́@CModel::GetCameraAnimParams()�őΉ�
	//�܂��́@���s����̐Ղ��R�~�b�g�@��Ń��t�@�N�^�����O�\��
	//#########################################################


	if (!m_pcamera || !m_pnode) {
		_ASSERT(0);
		return 1;
	}

	m_time = nextframe;


	FbxTime fbxtime;
	fbxtime.SetSecondDouble(m_time);


	FbxVector4 fbxpos = m_pcamera->EvaluatePosition(fbxtime);
	//FbxVector4 fbxpos = m_pnode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);
	if ((double)fbxpos[3] != 0.0) {
		float tmpx, tmpy, tmpz;
		tmpx = (float)((double)fbxpos[0] / (double)fbxpos[3]);
		tmpy = (float)((double)fbxpos[1] / (double)fbxpos[3]);
		tmpz = (float)((double)fbxpos[2] / (double)fbxpos[3]);
		m_position = ChaVector3(tmpx, tmpy, tmpz);
	}
	else {
		m_position = ChaVector3((float)fbxpos[0], (float)fbxpos[1], (float)fbxpos[2]);
	}

	FbxAMatrix fbxcameramat = m_pnode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot, true, true);
	ChaMatrix cameramat = ChaMatrixFromFbxAMatrix(fbxcameramat);
	//m_worldmat = cameramat;//!!!!!!

	CQuaternion cameraq;
	cameraq.RotationMatrix(cameramat);
	ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);//basevec +X��  �J�����A�j��������ꍇCModel::GetCameraAnimParams()�ł�basevec +Z�����H
	ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
	cameraq.Rotate(&cameradir, firstdir);
	ChaVector3Normalize(&cameradir, &cameradir);
	m_dirvec = cameradir;


	//ChaMatrix translatemat;
	//translatemat.SetIdentity();
	//translatemat.SetTranslation(m_position);
	//m_worldmat = cameraq.inverse().MakeRotMatX() * translatemat;

	m_worldmat = cameramat;


	return 0;
}

//int CCameraFbx::GetCameraAnimParams(double nextframe, double camdist, ChaVector3* pEyePos, ChaVector3* pTargetPos)
//{
//	if (!pEyePos || !pTargetPos) {
//		_ASSERT(0);
//		return 1;
//	}
//
//	m_time = nextframe;
//	FbxTime fbxtime;
//	fbxtime.SetSecondDouble(m_time);
//
//	//ChaVector3 chatra = ChaVector3(0.0f, 0.0f, 0.0f);
//	//FbxVector4 lcltra = m_pnode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);
//	//if ((double)lcltra[3] != 0.0) {
//	//	float tmpx, tmpy, tmpz;
//	//	tmpx = (float)((double)lcltra[0] / (double)lcltra[3]);
//	//	tmpy = (float)((double)lcltra[1] / (double)lcltra[3]);
//	//	tmpz = (float)((double)lcltra[2] / (double)lcltra[3]);
//	//	chatra = ChaVector3(tmpx, tmpy, tmpz);
//	//}
//	//else {
//	//	chatra = ChaVector3((float)lcltra[0], (float)lcltra[1], (float)lcltra[2]);
//	//}
//	//*pEyePos = GetPosition() + chatra;
//
//	//ChaVector3 chatra = ChaVector3(0.0f, 0.0f, 0.0f);
//	//FbxVector4 fbxpos = m_pcamera->EvaluatePosition(fbxtime);
//	//if ((double)fbxpos[3] != 0.0) {
//	//	float tmpx, tmpy, tmpz;
//	//	tmpx = (float)((double)fbxpos[0] / (double)fbxpos[3]);
//	//	tmpy = (float)((double)fbxpos[1] / (double)fbxpos[3]);
//	//	tmpz = (float)((double)fbxpos[2] / (double)fbxpos[3]);
//	//	chatra = ChaVector3(tmpx, tmpy, tmpz);
//	//}
//	//else {
//	//	chatra = ChaVector3((float)fbxpos[0], (float)fbxpos[1], (float)fbxpos[2]);
//	//}
//	//*pEyePos = chatra;
//
//
//	//FbxVector4 lclrot = m_pnode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
//	//CQuaternion lclq;
//	//lclq.SetRotationXYZ(0, ChaVector3((float)lclrot[0], (float)lclrot[1], (float)lclrot[2]));
//	//CQuaternion firstq;
//	//firstq.RotationMatrix(GetWorldMat());
//	//CQuaternion animq = firstq * lclq;
//	//ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);//basevec +X��
//	//ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
//	//animq.Rotate(&cameradir, firstdir);
//	//ChaVector3Normalize(&cameradir, &cameradir);
//
//
//	FbxAMatrix fbxcameramat = m_pnode->EvaluateGlobalTransform(fbxtime, FbxNode::eSourcePivot, true, true);
//	ChaMatrix cameramat = ChaMatrixFromFbxAMatrix(fbxcameramat);
//	ChaMatrix chaanimmat = ChaMatrixInv(GetWorldMat()) * cameramat;
//	ChaVector3 firsttargetpos;
//	firsttargetpos = GetPosition() + (GetDirVec() * camdist);
//	ChaVector3TransformCoord(pTargetPos, &firsttargetpos, &chaanimmat);
//
//
//	//CQuaternion cameraq;
//	//cameraq.RotationMatrix(chaanimmat);
//	//ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);//basevec +X��
//	//ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
//	//cameraq.Rotate(&cameradir, firstdir);
//	//ChaVector3Normalize(&cameradir, &cameradir);
//	//*pTargetPos = *pEyePos + cameradir * camdist;
//
//	return 0;
//}


//int CCameraFbx::GetCameraAnimParams(double nextframe, double camdist, ChaVector3* pEyePos, ChaVector3* pTargetPos)
//{
//	if (!pEyePos || !pTargetPos) {
//		_ASSERT(0);
//		return 1;
//	}
//
//	m_time = nextframe;
//	FbxTime fbxtime;
//	fbxtime.SetSecondDouble(m_time);
//
//	ChaVector3 chatra = ChaVector3(0.0f, 0.0f, 0.0f);
//	FbxVector4 lcltra = m_pnode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);
//	if ((double)lcltra[3] != 0.0) {
//		float tmpx, tmpy, tmpz;
//		tmpx = (float)((double)lcltra[0] / (double)lcltra[3]);
//		tmpy = (float)((double)lcltra[1] / (double)lcltra[3]);
//		tmpz = (float)((double)lcltra[2] / (double)lcltra[3]);
//		chatra = ChaVector3(tmpx, tmpy, tmpz);
//	}
//	else {
//		chatra = ChaVector3((float)lcltra[0], (float)lcltra[1], (float)lcltra[2]);
//	}
//	*pEyePos = GetPosition() + chatra;
//
//
//	FbxVector4 lclrot = m_pnode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
//	CQuaternion lclq;
//	lclq.SetRotationXYZ(0, ChaVector3((float)lclrot[0], (float)lclrot[1], (float)lclrot[2]));
//
//	CQuaternion firstq;
//	firstq.RotationMatrix(GetWorldMat());
//
//	CQuaternion animq = lclq * firstq;
//
//	ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);//basevec +X��
//	ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
//	animq.Rotate(&cameradir, firstdir);
//	ChaVector3Normalize(&cameradir, &cameradir);
//
//	*pTargetPos = *pEyePos + cameradir * camdist;
//
//	return 0;
//}







CCameraFbx CCameraFbx::operator= (CCameraFbx srccamera)
{
	m_loadedflag = srccamera.m_loadedflag;

	m_position = srccamera.m_position;
	m_dirvec = srccamera.m_dirvec;

	m_upVector = srccamera.m_upVector;
	m_aspectHeight = srccamera.m_aspectHeight;
	m_aspectWidth = srccamera.m_aspectWidth;
	m_nearZ = srccamera.m_nearZ;
	m_farZ = srccamera.m_farZ;
	m_aspectRatio = srccamera.m_aspectRatio;
	m_inch_mm = srccamera.m_inch_mm;
	m_filmHeight = srccamera.m_filmHeight;
	m_focalLength = srccamera.m_focalLength;
	m_filmHeight_mm = srccamera.m_filmHeight_mm;
	m_fovY = srccamera.m_fovY;
	m_fovY_Degree = srccamera.m_fovY_Degree;


	return *this;
}
