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

#include <Bone.h>

#define DBGH
#include <dbg.h>

#include <CameraFbx.h>
#include <GlobalVar.h>


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
	m_loadedflag = false;
	m_pnode = 0;
	m_pbone = 0;
	m_pcamera = 0;

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

	m_cameralcltra = ChaVector3(0.0f, 0.0f, 0.0f);
	m_cameraparentlcltra = ChaVector3(0.0f, 0.0f, 0.0f);

	m_pscene = 0;
	ZeroMemory(m_animname, sizeof(char) * 256);
	m_parentenullmat.SetIdentity();

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

int CCameraFbx::SetFbxCamera(FbxNode* pNode, CBone* pbone)
{
	if (!pNode || !pbone) {
		_ASSERT(0);
		m_loadedflag = false;
		m_pcamera = 0;
		return 1;
	}

	m_pnode = pNode;
	m_pbone = pbone;
	m_pcamera = pNode->GetCamera();//fbxsdk func


	SetCameraLclTra(pNode->LclTranslation);//2023/06/02


	if (m_pcamera) {

		//SetZeroFrameCamera();//InitMP�������. Main.cpp��CalcTotalBound()����Ă�


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

int CCameraFbx::SetZeroFrameCamera()
{

	//��������(SetFbxCamera())�ɂ��ĂԂ̂Ł@IsLoaded() == false�ł�����������

	//########################################################################################
	//2023/05/25
	// eCamera�����݂��AeCamera�ɃA�j���[�V�����R���|�[�l���g���A�^�b�`����Ă��Ȃ��ꍇ�̏���
	// �J�����A�j���́@CModel::GetCameraAnimParams()�őΉ�
	//########################################################################################


	if (!m_pcamera || !m_pnode || !m_pbone) {
		_ASSERT(0);
		return 1;
	}

	m_time = 0.0;
	FbxTime timezero;
	timezero.SetSecondDouble(m_time);


	//###########
	//�J�����ʒu
	//###########
	FbxVector4 fbxpos = m_pcamera->EvaluatePosition(timezero);
	m_position = ChaVector3(fbxpos);


	FbxAMatrix fbxcameramat = m_pnode->EvaluateGlobalTransform(timezero, FbxNode::eSourcePivot, true, true);
	ChaMatrix cameramat = ChaMatrixFromFbxAMatrix(fbxcameramat);
	m_worldmat = cameramat;//!!!!!!


	//###########
	//�J��������
	//###########
	//cameramat : EvaluateGlobalTransform �ɂ�nodemat�͊|�����Ă���
	ChaMatrix rotmat = cameramat;
	CQuaternion cameraq;
	cameraq.RotationMatrix(rotmat);
	ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);
	ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
	cameraq.Rotate(&cameradir, firstdir);
	ChaVector3Normalize(&cameradir, &cameradir);
	m_dirvec = cameradir;


	return 0;
}

//#################################################################
//inheritmode  0: writemode, 1:inherit all, 2:inherit cancel and Lclcancel
//#################################################################
int CCameraFbx::GetCameraAnimParams(int cameramotid, double nextframe, double camdist, 
	ChaVector3* pEyePos, ChaVector3* pTargetPos, ChaMatrix* protmat, int inheritmode)
{
	//if (!pEyePos || !pTargetPos || (cameramotid <= 0)) {
	if (!pEyePos || !pTargetPos) {//2023/05/29 cameramotid <= 0�̂Ƃ��ɂ́@zeroframe�J�����ʒu���Z�b�g
		//###################################################
		//protmat��NULL�̏ꍇ�����@rotmat���Z�b�g���Ȃ�����
		//###################################################
		_ASSERT(0);
		return 1;
	}


	double roundingframe = (double)((int)(nextframe + 0.0001));
	m_time = roundingframe;
	FbxTime fbxtime;
	fbxtime.SetSecondDouble(m_time);
	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (IsLoaded()) {
		if (cameramotid > 0) {
			FbxNode* cameranode = GetFbxNode();
			CBone* camerabone = GetBone();
			if (cameranode && camerabone) {
				ChaMatrix cammat = camerabone->GetWorldMat(false, cameramotid, roundingframe, 0);
				ChaMatrix nodemat = camerabone->GetNodeMat();

				ChaMatrix rootmat;
				if (camerabone->GetParent(false) && camerabone->GetParent(false)->IsNull()) {
					rootmat = GetCameraParentENullMat();
				}
				else {
					rootmat.SetIdentity();
				}

				ChaMatrix lcltramat;
				lcltramat.SetIdentity();
				lcltramat.SetTranslation(GetCameraLclTra());
				ChaMatrix parentlcltramat;
				parentlcltramat.SetIdentity();
				parentlcltramat.SetTranslation(GetCameraParentLclTra());

				ChaVector3 nodepos = nodemat.GetTranslation();

			//##############
			//�J�����̈ʒu
			//##############
				//ChaMatrix transformmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
				//ChaVector3TransformCoord(pEyePos, &zeropos, &transformmat);
				 
				ChaMatrix transformmat;
				switch (inheritmode) {
				case CAMERA_INHERIT_ALL:
					transformmat = cammat;
					break;
				case CAMERA_INHERIT_CANCEL_NULL1:
					transformmat = cammat * ChaMatrixInv(rootmat);
					break;
				case CAMERA_INHERIT_CANCEL_NULL2:
					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					transformmat = cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					break;
				default:
					_ASSERT(0);
					transformmat = cammat;
					break;
				}
				ChaVector3TransformCoord(pEyePos, &nodepos, &transformmat);

			//##############
			//�J�����̌���
			//##############
				ChaMatrix rotmat;
				switch (inheritmode) {
				case CAMERA_INHERIT_ALL:
					rotmat = nodemat * cammat;
					break;
				case CAMERA_INHERIT_CANCEL_NULL1:
					rotmat = nodemat * cammat * ChaMatrixInv(rootmat);
					break;
				case CAMERA_INHERIT_CANCEL_NULL2:
					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					rotmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					break;
				default:
					_ASSERT(0);
					rotmat = nodemat * cammat;
					break;
				}

				CQuaternion rotq;
				rotq.RotationMatrix(rotmat);
				ChaVector3 dirvec0 = ChaVector3(1.0f, 0.0f, 0.0f);
				ChaVector3 dirvec = ChaVector3(1.0f, 0.0f, 0.0f);
				rotq.Rotate(&dirvec, dirvec0);
				ChaVector3Normalize(&dirvec, &dirvec);
				*pTargetPos = *pEyePos + dirvec * camdist;

				if (protmat) {
					*protmat = ChaMatrixRot(rotmat);
				}

			}
			else {
				_ASSERT(0);
				//ZeroFrameCamera
				*pEyePos = m_position;
				*pTargetPos = *pEyePos + m_dirvec * camdist;

				if (protmat) {
					*protmat = ChaMatrixRot(GetWorldMat());
				}

			}
		}
		else {
			//ZeroFrameCamera
			*pEyePos = m_position;
			*pTargetPos = *pEyePos + m_dirvec * camdist;

			if (protmat) {
				*protmat = ChaMatrixRot(GetWorldMat());
			}
		}
	}
	else {
		//�������Ȃ�
	}


	return 0;
}







CCameraFbx CCameraFbx::operator= (CCameraFbx srccamera)
{
	m_loadedflag = srccamera.m_loadedflag;

	m_position = srccamera.m_position;
	m_dirvec = srccamera.m_dirvec;
	m_worldmat = srccamera.m_worldmat;

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


bool CCameraFbx::IsLoaded()
{
	//CModel::m_camerafbx��pointer�ł͖������߁@�I���t���O���`�F�b�N����

	if (g_endappflag == 0) {
		return m_loadedflag;
	}
	else {
		return false;
	}
}