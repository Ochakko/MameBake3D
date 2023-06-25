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

	m_cameranode.clear();
	m_cameramotion.clear();

	m_time = 0.0;

	m_pscene = 0;
	ZeroMemory(m_animname, sizeof(char) * 256);

	return 0;
}
int CCameraFbx::DestroyObjs()
{
	vector<CAMERANODE*>::iterator itrcameranode;
	for (itrcameranode = m_cameranode.begin(); itrcameranode != m_cameranode.end(); itrcameranode++) {
		CAMERANODE* curcn = *itrcameranode;
		if (curcn) {
			free(curcn);
		}
	}
	m_cameranode.clear();
	m_cameramotion.clear();

	InitParams();

	return 0;
}

int CCameraFbx::Clear()
{
	InitParams();
	return 0;
}


CAMERANODE* CCameraFbx::FindCameraNodeByNode(FbxNode* srcnode)
{
	vector<CAMERANODE*>::iterator itrcameranode;
	for (itrcameranode = m_cameranode.begin(); itrcameranode != m_cameranode.end(); itrcameranode++) {
		CAMERANODE* curcn = *itrcameranode;
		if (curcn) {
			if (curcn->pnode && curcn->pbone && (curcn->pnode == srcnode)) {
				return curcn;
			}
		}
	}
	return 0;
}
CAMERANODE* CCameraFbx::FindCameraNodeByBone(CBone* srcbone)
{
	vector<CAMERANODE*>::iterator itrcameranode;
	for (itrcameranode = m_cameranode.begin(); itrcameranode != m_cameranode.end(); itrcameranode++) {
		CAMERANODE* curcn = *itrcameranode;
		if (curcn) {
			if (curcn->pnode && curcn->pbone && (curcn->pbone == srcbone)) {
				return curcn;
			}
		}
	}
	return 0;
}

CAMERANODE* CCameraFbx::FindCameraNodeByMotId(int srcmotid)
{
	map<int, CAMERANODE*>::iterator itrcameranode;
	itrcameranode = m_cameramotion.find(srcmotid);
	if (itrcameranode != m_cameramotion.end()) {
		CAMERANODE* retcameranode = itrcameranode->second;
		if (retcameranode && retcameranode->IsValid()) {
			return retcameranode;
		}
		else {
			_ASSERT(0);
			return 0;
		}
	}
	else {
		return 0;
	}
}

int CCameraFbx::AddFbxCamera(FbxNode* pNode, CBone* pbone)
{
	if (!pNode || !pbone) {
		_ASSERT(0);
		return 1;
	}

	CAMERANODE* chkcameranode = FindCameraNodeByNode(pNode);
	if (chkcameranode) {
		//���ɑ��݂���̂Ń��^�[��
		_ASSERT(0);
		return 0;
	}

	CAMERANODE* newcameranode = (CAMERANODE*)malloc(sizeof(CAMERANODE));
	if (!newcameranode) {
		_ASSERT(0);
		return 1;
	}
	newcameranode->Init();
	m_cameranode.push_back(newcameranode);//!!!!!!!!!



	newcameranode->pnode = pNode;
	newcameranode->pbone = pbone;
	newcameranode->pcamera = pNode->GetCamera();//fbxsdk func


	{
		pbone->SaveFbxNodePosture(pNode);
		FbxDouble3 fbxlcltra = pbone->GetFbxLclPos();
		newcameranode->lcltra = ChaVector3((float)fbxlcltra[0], (float)fbxlcltra[1], (float)fbxlcltra[2]);

		FbxDouble3 fbxparentlcltra = FbxDouble3(0.0f, 0.0f, 0.0f);
		if (pbone->GetParent(false)) {
			pbone->GetParent(false)->SaveFbxNodePosture(pbone->GetParent(false)->GetFbxNodeOnLoad());
			fbxparentlcltra = pbone->GetParent(false)->GetFbxLclPos();
		}
		newcameranode->parentlcltra = ChaVector3((float)fbxparentlcltra[0], (float)fbxparentlcltra[1], (float)fbxparentlcltra[2]);
	}


	if (newcameranode->pcamera) {



		FbxVector4 upVector = newcameranode->pcamera->UpVector.Get();     // �A�b�v�x�N�g��
		FbxDouble aspectHeight = newcameranode->pcamera->AspectHeight.Get(); // �A�X�y�N�g��
		FbxDouble aspectWidth = newcameranode->pcamera->AspectWidth.Get();  // �A�X�y�N�g��

		//FbxDouble nearZ = newcameranode->pcamera->GetNearPlane();     // near���ʋ���
		//FbxDouble farZ = newcameranode->pcamera->GetFarPlane();      // far���ʋ���
		FbxDouble nearZ = newcameranode->pcamera->NearPlane.Get();     // near���ʋ���
		FbxDouble farZ = newcameranode->pcamera->FarPlane.Get();      // far���ʋ���
		
		FbxDouble aspectRatio = aspectWidth / aspectHeight; // �A�X�y�N�g��
		FbxDouble inch_mm = 25.4;    // �C���`���~��
		FbxDouble filmHeight = newcameranode->pcamera->FilmHeight.Get();   // �t�B�������i�C���`�j
		FbxDouble focalLength = newcameranode->pcamera->FocalLength.Get(); // ���ŋ����i�~���j
		FbxDouble filmHeight_mm = inch_mm * filmHeight;


		//FbxDouble fovY = atan2(filmHeight_mm, 2.0 * focalLength);
		FbxDouble fovY_Degree = newcameranode->pcamera->ComputeFieldOfView(focalLength);//degree
		FbxDouble fovY = fovY_Degree * 3.14159265358979 / 180.0;


		//newcameranode->upVector = ChaVector3((float)upVector[0], (float)upVector[1], (float)upVector[2]);     // �A�b�v�x�N�g��
		newcameranode->aspectHeight = aspectHeight; // �A�X�y�N�g��
		newcameranode->aspectWidth = aspectWidth;  // �A�X�y�N�g��
		newcameranode->nearZ = nearZ;     // near���ʋ���
		newcameranode->farZ = farZ;      // far���ʋ���
		newcameranode->aspectRatio = aspectRatio; // �A�X�y�N�g��
		newcameranode->inch_mm = inch_mm;    // �C���`���~��
		newcameranode->filmHeight = filmHeight;   // �t�B�������i�C���`�j
		newcameranode->focalLength = focalLength; // ���ŋ����i�~���j
		newcameranode->filmHeight_mm = filmHeight_mm;
		newcameranode->fovY = fovY;
		newcameranode->fovY_Degree = fovY_Degree;



		m_loadedflag = true;

	}


	return 0;

}

int CCameraFbx::PreLoadFbxAnim(CBone* srcbone, int srcmotid, ChaMatrix srcenullmat)
{

	CAMERANODE* cameranode = FindCameraNodeByBone(srcbone);
	if (!cameranode) {
		_ASSERT(0);
		return 1;
	}
	if (!cameranode->IsValid()) {
		_ASSERT(0);
		return 1;
	}

	//################################################################################
	//fbx�̃J�����g�A�j���[�V�������J�����A�j���ɂȂ��Ă���Ԃ�Evaluate���ʂ�ۑ�����
	//################################################################################
	FbxTime timezero;
	timezero.SetSecondDouble(m_time);
	FbxVector4 fbxpos = cameranode->pcamera->EvaluatePosition(timezero);
	cameranode->position = ChaVector3(fbxpos);

	FbxAMatrix fbxcameramat = cameranode->pnode->EvaluateGlobalTransform(timezero, FbxNode::eSourcePivot, true, true);
	ChaMatrix cameramat = ChaMatrixFromFbxAMatrix(fbxcameramat);
	cameranode->worldmat = cameramat;//!!!!!!

	cameranode->parentenullmat = srcenullmat;


	{
		srcbone->SaveFbxNodePosture(cameranode->pnode);
		FbxDouble3 fbxlcltra = srcbone->GetFbxLclPos();
		cameranode->lcltra = ChaVector3((float)fbxlcltra[0], (float)fbxlcltra[1], (float)fbxlcltra[2]);

		FbxDouble3 fbxparentlcltra = FbxDouble3(0.0f, 0.0f, 0.0f);
		if (srcbone->GetParent(false)) {
			srcbone->GetParent(false)->SaveFbxNodePosture(srcbone->GetParent(false)->GetFbxNodeOnLoad());
			fbxparentlcltra = srcbone->GetParent(false)->GetFbxLclPos();
		}
		cameranode->parentlcltra = ChaVector3((float)fbxparentlcltra[0], (float)fbxparentlcltra[1], (float)fbxparentlcltra[2]);
	}



	ChaMatrix rotmat = cameramat;
	CQuaternion cameraq;
	cameraq.RotationMatrix(rotmat);


	ChaVector3 firstdir = ChaVector3(1.0f, 0.0f, 0.0f);
	ChaVector3 cameradir = ChaVector3(1.0f, 0.0f, 0.0f);
	cameraq.Rotate(&cameradir, firstdir);
	ChaVector3Normalize(&cameradir, &cameradir);
	cameranode->dirvec = cameradir;


	ChaVector3 firstupdir = ChaVector3(0.0f, 1.0f, 0.0f);
	ChaVector3 cameraupdir = ChaVector3(0.0f, 1.0f, 0.0f);
	cameraq.Rotate(&cameraupdir, firstupdir);
	ChaVector3Normalize(&cameraupdir, &cameraupdir);
	cameranode->upvec = cameraupdir;


	//###################
	//cameramotion�ɓo�^
	//###################
	CAMERANODE* chkcameranode = FindCameraNodeByMotId(srcmotid);
	if (!chkcameranode) {
		m_cameramotion[srcmotid] = cameranode;
	}


	return 0;
}


//#################################################################
//inheritmode  0: writemode, 1:inherit all, 2:inherit cancel and Lclcancel
//#################################################################
int CCameraFbx::GetCameraAnimParams(int cameramotid, double nextframe, double camdist, 
	ChaVector3* pEyePos, ChaVector3* pTargetPos, ChaVector3* pcamupvec, ChaMatrix* protmat, int inheritmode)
{
	//if (!pEyePos || !pTargetPos || (cameramotid <= 0)) {
	if (!pEyePos || !pTargetPos || !pcamupvec) {//2023/05/29 cameramotid <= 0�̂Ƃ��ɂ́@zeroframe�J�����ʒu���Z�b�g
		//###################################################
		//protmat��NULL�̏ꍇ�����@rotmat���Z�b�g���Ȃ�����
		//###################################################
		_ASSERT(0);
		return 1;
	}

	CAMERANODE* curcamera = FindCameraNodeByMotId(cameramotid);
	if (!curcamera) {
		//fbx�ɃJ�������܂܂�Ă��Ȃ��ꍇ�@�����������^�[��
		_ASSERT(0);
		return 0;
	}
	if (!curcamera->IsValid()) {
		//�K�v�ȏ�񂪓ǂݍ��߂Ă��Ȃ��ꍇ�@�G���[
		_ASSERT(0);
		return 1;
	}


	double roundingframe = (double)((int)(nextframe + 0.0001));
	m_time = roundingframe;
	FbxTime fbxtime;
	fbxtime.SetSecondDouble(m_time / 30.0);
	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	if (IsLoaded()) {
		if (cameramotid > 0) {
			FbxNode* cameranode = curcamera->pnode;
			CBone* camerabone = curcamera->pbone;
			if (cameranode && camerabone) {
				//ChaMatrix cammat = camerabone->GetWorldMat(false, cameramotid, roundingframe, 0);
				//ChaMatrix nodemat = camerabone->GetNodeMat();

				ChaMatrix localnodemat, localnodeanimmat;
				localnodemat.SetIdentity();
				localnodeanimmat.SetIdentity();
				camerabone->CalcLocalNodePosture(0, roundingframe, &localnodemat, &localnodeanimmat);


				ChaMatrix rootmat;
				ChaMatrix parentlocalnodemat, parentlocalnodeanimmat;
				parentlocalnodemat.SetIdentity();
				parentlocalnodeanimmat.SetIdentity();
				if (camerabone->GetParent(false) && camerabone->GetParent(false)->IsNull()) {
					rootmat = curcamera->parentenullmat;
					//camerabone->GetParent(false)->CalcLocalNodePosture(0, 0.0, &parentlocalnodemat, &parentlocalnodeanimmat);
					camerabone->GetParent(false)->CalcLocalNodePosture(0, roundingframe, &parentlocalnodemat, &parentlocalnodeanimmat);
				}
				else {
					rootmat.SetIdentity();
				}

				ChaMatrix lcltramat;
				lcltramat.SetIdentity();
				lcltramat.SetTranslation(curcamera->lcltra);
				ChaMatrix parentlcltramat;
				parentlcltramat.SetIdentity();
				parentlcltramat.SetTranslation(curcamera->parentlcltra);


				//ChaVector3 nodepos = nodemat.GetTranslation();

	
				//ChaMatrix positionmat1;
				//positionmat1.SetIdentity();
				//positionmat1.SetTranslation(curcamera->position - curcamera->lcltra);
				//ChaMatrix positionmat2;
				//positionmat2.SetIdentity();
				//positionmat2.SetTranslation(curcamera->lcltra - curcamera->position);
				//ChaMatrix positionmat3;
				//positionmat3.SetIdentity();
				////positionmat.SetTranslation(curcamera->position);
				//positionmat3.SetTranslation(curcamera->position - curcamera->parentlcltra);
				ChaMatrix positionmat4;
				positionmat4.SetIdentity();
				positionmat4.SetTranslation(curcamera->parentlcltra - curcamera->position);


			//##########################################################################################################
			//2023/06/21�@����
			//Unity2022��LTS�����\���ꂽ�@�������ς�����H�悤�Ł@�J�����A�j���̕\�����ςɂȂ���
			//Unity2022.3.1f1�ŏo�͂����J�����A�j���t��fbx��\�����邽�߂Ɂ@���s����̗�
			//�Ȃ�Ƃ��Đ��ł���悤�ɂȂ���
			//�g����
			//�@�J�����̏����̈ʒu�ƌ����ݒ�pNull�m�[�h�̉��Ɂ@�J�����I�u�W�F�N�g(�ʒu�����͐ݒ肵�Ȃ�)��u��
			//�@�J�����I�u�W�F�N�g�ɃA�j���[�V�������R���|�[�l���g�Ƃ��Ēǉ�
			//�@!!!!!!!! �J�����I�u�W�F�N�g�̖��O�Ɓ@�A�j���[�V�����̖��O����v�����邱�Ƃ��ǂݍ��ݏ��� !!!!!!!!!!!!!!
			// 
			//  ��L�̂悤��Null�m�[�h-->�J�����I�u�W�F�N�g-->�A�j���P�̃Z�b�g���@�����ǂݍ��ݍĐ��\
			// 
			//2023/06/21���݁@���܂��Đ��ł�����̂Ɓ@�ςɂȂ���̂�������
			// 
			// 
			// �Ƃ肠�����@���s����̍���(���̎��̃R�~�b�g�ŏ����\��)���c�������̂��@��x�R�~�b�g
			// ����@����𑫊|����ɂ��Ē������Ă���
			//##########################################################################################################

			//##############
			//�J�����̈ʒu
			//##############
				//ChaMatrix transformmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
				//ChaVector3TransformCoord(pEyePos, &zeropos, &transformmat);
				 
				ChaMatrix transformmat;
				switch (inheritmode) {
				case CAMERA_INHERIT_ALL:
					//transformmat = nodemat * cammat;
					transformmat = localnodeanimmat * parentlocalnodeanimmat;//ParentRot�L��
					break;

				case CAMERA_INHERIT_CANCEL_NULL1:
					//transformmat = nodemat * cammat * ChaMatrixInv(rootmat);
					transformmat = localnodeanimmat;//�O ##########
					break;

				case CAMERA_INHERIT_CANCEL_NULL2:
					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					//transformmat = cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					transformmat = localnodeanimmat * positionmat4 * parentlocalnodemat;//###################### ParentRot���� TheHunt Street1 Camera1
					break;

				default:
					_ASSERT(0);
					transformmat = localnodeanimmat;
					break;
				}
				//ChaVector3TransformCoord(pEyePos, &nodepos, &transformmat);
				ChaVector3TransformCoord(pEyePos, &zeropos, &transformmat);
				//ChaVector3TransformCoord(pEyePos, &(curcamera->position), &transformmat);
				//ChaVector3TransformCoord(pEyePos, &(curcamera->parentlcltra), &transformmat);


			//##############
			//�J�����̌���
			//##############

			
				ChaVector3 dirvec0 = ChaVector3(-1.0f, 0.0f, 0.0f);
				ChaVector3 dirvec = ChaVector3(-1.0f, 0.0f, 0.0f);

				ChaMatrix rotmat;
				switch (inheritmode) {
				case CAMERA_INHERIT_ALL:
					//rotmat = nodemat * cammat;
					//rotmat = localnodeanimmat * parentlocalnodemat;//!!!!!!
					rotmat = localnodeanimmat * parentlocalnodeanimmat;//ParentRot�L��
					break;

				case CAMERA_INHERIT_CANCEL_NULL1:
					//rotmat = nodemat * cammat * ChaMatrixInv(rootmat);
					rotmat = localnodeanimmat;//!!!!!!!!!!!
					break;

				case CAMERA_INHERIT_CANCEL_NULL2:

					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					////rotmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					rotmat = localnodeanimmat * positionmat4 * parentlocalnodemat;//###################### ParentRot�����@TheHunt Street1 Camera1
					break;

				default:
					_ASSERT(0);
					//rotmat = nodemat * cammat;
					rotmat = localnodeanimmat;//!!!!!!!!!!!!!
					break;
				}


				ChaMatrix convmat = ChaMatrixRot(rotmat);
				//convmat.SetTranslationZero();
				//convmat.data[MATI_44] = 0;
				ChaVector3TransformCoord(&dirvec, &dirvec0, &convmat);


				ChaVector3Normalize(&dirvec, &dirvec);
				*pTargetPos = *pEyePos + dirvec * camdist;


				//2023/06/25
				//ChaVector3 upvec = ChaMatrixRot(convmat).GetRow(1);
				//ChaVector3Normalize(&upvec, &upvec);
				//*pcamupvec = upvec;
				ChaVector3 firstupdir = ChaVector3(0.01f, 0.99f, 0.0f);
				ChaVector3 cameraupdir = ChaVector3(0.01f, 0.99f, 0.0f);
				ChaVector3TransformCoord(&cameraupdir, &firstupdir, &convmat);
				ChaVector3Normalize(&cameraupdir, &cameraupdir);
				*pcamupvec = cameraupdir;


				if (protmat) {
					//*protmat = ChaMatrixRot(rotmat);
					//*protmat = rotq.MakeRotMatX();
					*protmat = convmat;
				}

			}
			else {
				_ASSERT(0);
				//ZeroFrameCamera
				*pEyePos = curcamera->position;
				*pTargetPos = *pEyePos + curcamera->dirvec * camdist;
				*pcamupvec = curcamera->upvec;

				if (protmat) {
					*protmat = ChaMatrixRot(curcamera->worldmat);
				}

			}
		}
		else {
			//ZeroFrameCamera
			*pEyePos = curcamera->position;
			*pTargetPos = *pEyePos + curcamera->dirvec * camdist;
			*pcamupvec = curcamera->upvec;

			if (protmat) {
				*protmat = ChaMatrixRot(curcamera->worldmat);
			}
		}
	}
	else {
		//�������Ȃ�
	}


	return 0;
}


ChaVector3 CCameraFbx::CalcCameraFbxEulXYZ(int cameramotid, double srcframe, ChaVector3 befeul)
{
	ChaVector3 cureul = ChaVector3(0.0f, 0.0f, 0.0f);


	CAMERANODE* curcamera = FindCameraNodeByMotId(cameramotid);
	if (!curcamera) {
		//fbx�ɃJ�������܂܂�Ă��Ȃ��ꍇ�@�����������^�[��
		_ASSERT(0);
		return cureul;
	}
	if (!curcamera->IsValid()) {
		//�K�v�ȏ�񂪓ǂݍ��߂Ă��Ȃ��ꍇ�@�G���[
		_ASSERT(0);
		return cureul;
	}


	double roundingframe = (double)((int)(srcframe + 0.0001));
	m_time = roundingframe;
	FbxTime fbxtime;
	fbxtime.SetSecondDouble(m_time / 30.0);
	ChaVector3 zeropos = ChaVector3(0.0f, 0.0f, 0.0f);

	int notmodify180flag;
	if (roundingframe <= 1.0) {
		notmodify180flag = 1;
	}
	else {
		notmodify180flag = 0;
	}


	if (IsLoaded()) {
		if (cameramotid > 0) {
			FbxNode* cameranode = curcamera->pnode;
			CBone* camerabone = curcamera->pbone;
			if (cameranode && camerabone) {

				//ChaMatrix cammat = camerabone->GetWorldMat(false, cameramotid, roundingframe, 0);
				//ChaMatrix nodemat = camerabone->GetNodeMat();
				//ChaMatrix rootmat;
				//if (camerabone->GetParent(false) && camerabone->GetParent(false)->IsNull()) {
				//	rootmat = curcamera->parentenullmat;
				//}
				//else {
				//	rootmat.SetIdentity();
				//}
				//ChaMatrix rotmat = nodemat * cammat * ChaMatrixInv(rootmat);
				//CQuaternion eulq;
				//eulq.RotationMatrix(rotmat);
				////CQuaternion rotq;
				////rotq.RotationMatrix(rotmat);
				////CQuaternion rotz2x;
				//////rotz2x.SetRotationXYZ(0, ChaVector3(0.0f, 90.0f, 0.0f));
				////rotz2x.SetRotationXYZ(0, ChaVector3(0.0f, -90.0f, 0.0f));
				////CQuaternion eulq;
				////eulq = rotq * rotz2x;//90�x��]���ā@�������������킹��
				////eulq = rotq;
				//eulq.Q2EulXYZusingQ(0, befeul, &cureul, 0, 0, notmodify180flag);//XYZ
				////eulq.Q2EulXYZusingMat((int)camerabone->GetFbxRotationOrder(), 0, befeul, &cureul, 0, 0, notmodify180flag);//rotationOrder
				////cureul.y += 90.0f;



				EFbxRotationOrder rotationorder;
				cameranode->GetRotationOrder(FbxNode::eSourcePivot, rotationorder);
				//FbxTime fbxtime;
				//fbxtime.SetSecondDouble(roundingframe / 30.0);
				FbxVector4 orgfbxeul = cameranode->EvaluateLocalRotation(fbxtime, FbxNode::eSourcePivot, true, true);
				cureul = ChaVector3((float)orgfbxeul[0], (float)orgfbxeul[1], (float)orgfbxeul[2]);

				//ChaVector3 orgeul = ChaVector3((float)orgfbxeul[0], (float)orgfbxeul[1], (float)orgfbxeul[2]);
				//CQuaternion rotq;
				//rotq.SetRotation(rotationorder, 0, orgeul);
				//CQuaternion rotz2x;
				//rotz2x.SetRotationXYZ(0, ChaVector3(0.0f, 90.0f, 0.0f));
				//CQuaternion eulq = rotq * rotz2x;//90�x��]���ā@�������������킹��
				////eulq = rotq;
				//int notmodify180flag2;
				//if (roundingframe <= 1.0) {
				//	notmodify180flag2 = 1;
				//}
				//else {
				//	notmodify180flag2 = 0;
				//}
				//eulq.Q2EulXYZusingMat(rotationorder, 0, befeul, &cureul, 0, 0, notmodify180flag2);//rotationOrder
				////eulq.Q2EulXYZusingQ(0, befeul, &cureul, 0, 0, notmodify180flag2);//XYZ

			}
		}
	}

	return cureul;
}





//CCameraFbx CCameraFbx::operator= (CCameraFbx srccamera)
//{
//	m_loadedflag = srccamera.m_loadedflag;
//
//	m_position = srccamera.m_position;
//	m_dirvec = srccamera.m_dirvec;
//	m_worldmat = srccamera.m_worldmat;
//
//	m_upVector = srccamera.m_upVector;
//	m_aspectHeight = srccamera.m_aspectHeight;
//	m_aspectWidth = srccamera.m_aspectWidth;
//	m_nearZ = srccamera.m_nearZ;
//	m_farZ = srccamera.m_farZ;
//	m_aspectRatio = srccamera.m_aspectRatio;
//	m_inch_mm = srccamera.m_inch_mm;
//	m_filmHeight = srccamera.m_filmHeight;
//	m_focalLength = srccamera.m_focalLength;
//	m_filmHeight_mm = srccamera.m_filmHeight_mm;
//	m_fovY = srccamera.m_fovY;
//	m_fovY_Degree = srccamera.m_fovY_Degree;
//
//
//	return *this;
//}


bool CCameraFbx::IsLoaded()
{
	return m_loadedflag;
}
