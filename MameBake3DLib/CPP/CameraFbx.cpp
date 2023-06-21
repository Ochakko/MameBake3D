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


		newcameranode->upVector = ChaVector3((float)upVector[0], (float)upVector[1], (float)upVector[2]);     // �A�b�v�x�N�g��
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

int CCameraFbx::PostLoadFbxAnim(CBone* srcbone, int srcmotid, ChaMatrix srcenullmat)
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


	////FbxDouble3 fbxlcltra = cameranode->pnode->LclTranslation.Get();
	//FbxDouble3 fbxlcltra = cameranode->pnode->EvaluateLocalTranslation(timezero, FbxNode::eSourcePivot, true, true);;
	//cameranode->lcltra = ChaVector3((float)fbxlcltra[0], (float)fbxlcltra[1], (float)fbxlcltra[2]);

	//FbxDouble3 fbxlcltra = cameranode->pnode->EvaluateLocalTranslation(timezero, FbxNode::eSourcePivot, true, true);
	//FbxDouble3 fbxparentlcltra = FbxDouble3(0.0f, 0.0f, 0.0f);
	//if (cameranode->pnode->GetParent()) {
	//	fbxparentlcltra = cameranode->pnode->GetParent()->EvaluateLocalTranslation(timezero, FbxNode::eSourcePivot, true, true);
	//}
	////cameranode->lcltra = ChaVector3((float)(fbxlcltra[0] - fbxparentlcltra[0]), (float)(fbxlcltra[1] - fbxparentlcltra[1]), (float)(fbxlcltra[2] - fbxparentlcltra[2]));
	////cameranode->lcltra = ChaVector3((float)(fbxparentlcltra[0] - fbxlcltra[0]), (float)(fbxparentlcltra[1] - fbxlcltra[1]), (float)(fbxparentlcltra[2] - fbxlcltra[2]));


	//ChaMatrix chatra, chaparenttra;
	//chatra.SetIdentity();
	//chaparenttra.SetIdentity();
	//FbxAMatrix fbxtra = cameranode->pnode->EvaluateGlobalTransform(timezero, FbxNode::eSourcePivot, true, true);
	//chatra = ChaMatrixFromFbxAMatrix(fbxtra);
	//FbxAMatrix fbxparenttra;
	//fbxparenttra.SetIdentity();
	//if (cameranode->pnode->GetParent()) {
	//	fbxparenttra = cameranode->pnode->GetParent()->EvaluateGlobalTransform(timezero, FbxNode::eSourcePivot, true, true);
	//}
	//chaparenttra = ChaMatrixFromFbxAMatrix(fbxparenttra);

	////cameranode->adjusttra = chatra * ChaMatrixInv(chaparenttra);
	//ChaVector3 pos0 = cameranode->position;
	//ChaVector3 pos1 = ChaMatrixTraVec(chatra);
	//ChaVector3 adjustpos = pos0 - pos1;
	//cameranode->adjusttra.SetIdentity();
	//cameranode->adjusttra.SetTranslation(adjustpos);
	//int dbgflag = 1;

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
				ChaMatrix cammat = camerabone->GetWorldMat(false, cameramotid, roundingframe, 0);
				ChaMatrix nodemat = camerabone->GetNodeMat();

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


				//FbxDouble3 fbxlcltra = curcamera->pnode->EvaluateLocalTranslation(fbxtime, FbxNode::eSourcePivot, true, true);;
				////FbxDouble3 fbxlcltra = curcamera->pnode->LclTranslation.Get();
				//ChaVector3 chalcltra = ChaVector3((float)fbxlcltra[0], (float)fbxlcltra[1], (float)fbxlcltra[2]);
				//ChaMatrix lcltramat;
				//lcltramat.SetIdentity();
				//lcltramat.SetTranslation(chalcltra);


				ChaMatrix lcltramat;
				lcltramat.SetIdentity();
				lcltramat.SetTranslation(curcamera->lcltra);
				ChaMatrix parentlcltramat;
				parentlcltramat.SetIdentity();
				parentlcltramat.SetTranslation(curcamera->parentlcltra);

				//lcltramat.SetTranslation((curcamera->lcltra * 2.567f));
				//lcltramat.SetTranslation(curcamera->parentlcltra - curcamera->lcltra);
				//lcltramat.SetTranslation(curcamera->lcltra - curcamera->parentlcltra);
				//lcltramat.SetTranslation((curcamera->lcltra * 2.0f));
				//lcltramat = curcamera->localnodemat;
				//ChaVector3 svec, tvec;
				//ChaMatrix rmat;
				//GetSRTMatrix(curcamera->localnodemat, &svec, &rmat, &tvec);
				//ChaVector3 canceltravec = ChaVector3(tvec.x * svec.x, tvec.y * svec.y, tvec.z * svec.z);
				//lcltramat.SetTranslation(canceltravec);


				ChaVector3 nodepos = nodemat.GetTranslation();


				//ChaMatrix roottramat;
				//roottramat.SetIdentity();
				//roottramat = ChaMatrixTra(rootmat);
	
				ChaMatrix positionmat1;
				positionmat1.SetIdentity();
				//positionmat.SetTranslation(curcamera->position);
				positionmat1.SetTranslation(curcamera->position - curcamera->lcltra);
				ChaMatrix positionmat2;
				positionmat2.SetIdentity();
				positionmat2.SetTranslation(curcamera->lcltra - curcamera->position);

				ChaMatrix positionmat3;
				positionmat3.SetIdentity();
				//positionmat.SetTranslation(curcamera->position);
				positionmat3.SetTranslation(curcamera->position - curcamera->parentlcltra);
				ChaMatrix positionmat4;
				positionmat4.SetIdentity();
				positionmat4.SetTranslation(curcamera->parentlcltra - curcamera->position);


				//FbxVector4 fbxgeotra = cameranode->GetGeometricTranslation(FbxNode::eSourcePivot);
				//ChaVector3 chageotra = ChaVector3((float)fbxgeotra[0], (float)fbxgeotra[1], (float)fbxgeotra[2]);
				//ChaMatrix geotramat;
				//geotramat.SetIdentity();
				//geotramat.SetTranslation(chageotra);//identity



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
					//transformmat = localnodeanimmat * parentlocalnodeanimmat;//��
					//transformmat = localnodeanimmat * rootmat;//��
					transformmat = localnodeanimmat * parentlocalnodemat;//��
					
					
					//transformmat = localnodeanimmat * rootmat;//�� ########
					
					break;
				case CAMERA_INHERIT_CANCEL_NULL1:
					transformmat = localnodeanimmat;//�O ##########

					//transformmat = nodemat * cammat * ChaMatrixInv(rootmat);

					//transformmat = nodemat * cammat * positionmat4;//���n�_�͗ǂ��@�O��ړ����΂߂ɂȂ��Ă��܂�
					//transformmat = nodemat * cammat * positionmat3;//��������

					//transformmat = localnodeanimmat * ChaMatrixInv(parentlocalnodeanimmat);//�O�@�E
					//transformmat = localnodeanimmat * ChaMatrixInv(rootmat);//�O�@�E
					//transformmat = parentlocalnodeanimmat * localnodeanimmat;
					//transformmat = localnodeanimmat * positionmat1;//�X�ɏ�
					//transformmat = localnodeanimmat * positionmat3;//��
					
					//transformmat = nodemat * cammat * ChaMatrixInv(rootmat);
					//transformmat = cammat * ChaMatrixInv(rootmat) * roottramat;
					//transformmat = cammat * roottramat * ChaMatrixInv(rootmat);
					break;
				case CAMERA_INHERIT_CANCEL_NULL2:
					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					//transformmat = cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					//transformmat = cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat) * roottramat;
					//transformmat = cammat * ChaMatrixInv(curcamera->adjusttra) * ChaMatrixInv(rootmat);
					//transformmat = cammat * ChaMatrixInv(roottramat) * ChaMatrixInv(rootmat);
					//transformmat = cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat) * camerabone->GetParent(false)->GetNodeMat();//nodepos : ��߂�
					//transformmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat) * camerabone->GetParent(false)->GetNodeMat();//zeropos : ��߂�
					//transformmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat) * parentlcltramat;//zeropos : ��߂�
					//transformmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat) * ChaMatrixInv(parentlcltramat);//zeropos : �O�@��
				//{
				//	ChaMatrix localfbxmat = curcamera->pbone->CalcFbxLocalMatrix(false, cameramotid, roundingframe);
				//	transformmat = localfbxmat * ChaMatrixInv(parentlcltramat);//zeropos : �O�@��
				//}
				

					//transformmat = localnodeanimmat;//�O
					//transformmat = localnodeanimmat * ChaMatrixInv(parentlocalnodemat);//�O�@��
					//transformmat = localnodeanimmat * parentlocalnodeanimmat * ChaMatrixInv(parentlcltramat);//�O
					//transformmat = localnodeanimmat * ChaMatrixInv(positionmat);//�O�@��
					//transformmat = localnodeanimmat * positionmat;//��
					//transformmat = localnodeanimmat * ChaMatrixInv(lcltramat) * parentlocalnodemat;//�O�@��
					//transformmat = localnodeanimmat * lcltramat * parentlocalnodemat;//��

					//transformmat = localnodeanimmat * geotramat * parentlocalnodemat;

					//transformmat = localnodeanimmat * ChaMatrixInv(lcltramat);//lcltra 0 : �O
					//transformmat = ChaMatrixInv(localnodeanimmat) * ChaMatrixInv(parentlocalnodeanimmat);
					//transformmat = localnodeanimmat * ChaMatrixInv(parentlcltramat);
					//transformmat = localnodeanimmat * positionmat2;//��������
					//transformmat = localnodeanimmat * positionmat4;//��


					//transformmat = nodemat * cammat * ChaMatrixInv(rootmat) * positionmat4 * rootmat;//�E�@�O��ړ����΂߂ɂȂ�
					transformmat = localnodeanimmat * positionmat4 * parentlocalnodemat;//�ʒu�͗ǂ�����
					//transformmat = localnodeanimmat * positionmat4 * ChaMatrixTra(parentlocalnodemat);//�ʒu�͗ǂ����� #######


					//transformmat = localnodeanimmat * positionmat4 * parentlocalnodeanimmat;
					//transformmat = localnodeanimmat * parentlocalnodeanimmat * positionmat4;
					//transformmat = localnodeanimmat * parentlocalnodemat * positionmat4;
					//transformmat = localnodeanimmat * ChaMatrixInv(positionmat4) * ChaMatrixInv(parentlocalnodemat);//�������O

					//transformmat = nodemat * cammat * positionmat4 * ChaMatrixInv(rootmat);//��
					//transformmat = nodemat * cammat * ChaMatrixInv(rootmat) * positionmat4;//��
					//transformmat = nodemat * cammat * positionmat3 * ChaMatrixInv(rootmat);//��
					//transformmat = localnodeanimmat * positionmat3 * ChaMatrixInv(parentlocalnodemat);//�������O

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


				////FbxVector4 fbxpos = curcamera->pcamera->EvaluatePosition(fbxtime);
				////*pEyePos = ChaVector3(fbxpos);

				//ChaMatrix localfbxmat = curcamera->pbone->CalcFbxLocalMatrix(false, cameramotid, roundingframe);
				////ChaVector3 svec, tvec;
				////ChaMatrix rmat;
				////GetSRTMatrix(localfbxmat, &svec, &rmat, &tvec);
				////*pEyePos = tvec;
				//ChaVector3TransformCoord(pEyePos, &nodepos, &localfbxmat);


			//##############
			//�J�����̌���
			//##############
				//CQuaternion localq, parentlocalq;
				//localq.RotationMatrix(localnodeanimmat);
				////parentlocalq.RotationMatrix(parentlocalnodeanimmat);
				//parentlocalq.RotationMatrix(parentlocalnodemat);
				//CQuaternion rotq;

				//CQuaternion rotq, rotq1, rotq2;


				ChaMatrix rotmat;
				switch (inheritmode) {
				case CAMERA_INHERIT_ALL:
					//rotmat = nodemat * cammat;
					//rotmat = localnodeanimmat * parentlocalnodemat;//!!!!!!
					//rotmat = localnodeanimmat * rootmat;



					//rotmat = localnodeanimmat * parentlocalnodeanimmat;
					rotmat = localnodeanimmat * parentlocalnodemat;

					//rotq = parentlocalq * localq;
					//rotq1.RotationMatrix(localnodeanimmat);
					//rotq2.RotationMatrix(parentlocalnodemat);
					//rotq = rotq2 * rotq1;
					break;
				case CAMERA_INHERIT_CANCEL_NULL1:
					//rotmat = nodemat * cammat * ChaMatrixInv(rootmat);
					//rotmat = nodemat * cammat;
					//rotmat = localnodeanimmat;//!!!!!!!!!!!
					//rotmat = nodemat * cammat * positionmat4;
					rotmat = localnodeanimmat;


					//rotq = localq;
					//rotq1.RotationMatrix(localnodeanimmat);
					//rotq = rotq1;
					break;
				case CAMERA_INHERIT_CANCEL_NULL2:
					//################################################################################################
					//Unity�ɂ����Ă�RootMotion�`�F�b�N�I��. Maya�ɂ����Ă̓g�����X�t�H�[���̌p���`�F�b�N�I�t�@�ɑ���
					//################################################################################################
					////rotmat = nodemat * cammat * ChaMatrixInv(lcltramat) * ChaMatrixInv(rootmat);
					//rotmat = nodemat * cammat;
					////rotmat = nodemat * cammat * ChaMatrixInv(curcamera->adjusttra) * ChaMatrixInv(rootmat);
					//rotmat = localnodeanimmat * positionmat4 * parentlocalnodemat;//!!!!!!!!!!!!
					//rotmat = localnodeanimmat * positionmat4 * ChaMatrixTra(parentlocalnodemat);
					
					
					//rotmat = localnodeanimmat * ChaMatrixInv(positionmat4) * ChaMatrixInv(parentlocalnodemat);
					//rotmat = nodemat * cammat * positionmat4 * ChaMatrixInv(rootmat);
					//rotmat = nodemat * cammat * ChaMatrixInv(rootmat) * positionmat4;
					//rotmat = nodemat * cammat * positionmat3 * ChaMatrixInv(rootmat);
					//rotmat = localnodeanimmat * positionmat3 * ChaMatrixInv(parentlocalnodemat);

					rotmat = localnodeanimmat * positionmat4 * parentlocalnodemat;

					//rotmat = localnodeanimmat * positionmat4 * parentlocalnodeanimmat;
					//rotq = parentlocalq * localq;
					//rotq1.RotationMatrix(localnodeanimmat);
					////rotq2.RotationMatrix(positionmat4 * parentlocalnodemat);
					//rotq2.RotationMatrix(parentlocalnodemat);//!!!!!!!!!!!!!!!!!!!!
					////rotq2.RotationMatrix(parentlocalnodeanimmat);
					//rotq = rotq2 * rotq1;
					break;
				default:
					_ASSERT(0);
					//rotmat = nodemat * cammat;
					rotmat = localnodeanimmat;
					//rotq = localq;
					//rotq1.RotationMatrix(localnodeanimmat);
					//rotq = rotq1;
					break;
				}

				//CQuaternion rotq;
				//rotq.RotationMatrix(rotmat);
				//ChaVector3 dirvec0 = ChaVector3(1.0f, 0.0f, 0.0f);
				//ChaVector3 dirvec = ChaVector3(1.0f, 0.0f, 0.0f);
				ChaVector3 dirvec0 = ChaVector3(-1.0f, 0.0f, 0.0f);
				ChaVector3 dirvec = ChaVector3(-1.0f, 0.0f, 0.0f);
				//ChaVector3 dirvec0 = ChaVector3(0.0f, 0.0f, -1.0f);
				//ChaVector3 dirvec = ChaVector3(0.0f, 0.0f, -1.0f);
				//ChaVector3 dirvec0 = ChaVector3(0.0f, 0.0f, 1.0f);
				//ChaVector3 dirvec = ChaVector3(0.0f, 0.0f, 1.0f);
				//rotq.Rotate(&dirvec, dirvec0);


				//quaternion�ɂ��Ă����]������@ChaMatrixRot()���Ă����]����Ɓ@���E��U���㉺��U�肪�����Ȃ��Ȃ���
				//�ȉ��̂悤�Ƀx�N�g���ϊ��p�ɂ��đΉ�
				ChaMatrix convmat = rotmat;
				convmat.SetTranslationZero();
				convmat.data[MATI_44] = 0;
				ChaVector3TransformCoord(&dirvec, &dirvec0, &convmat);



				ChaVector3Normalize(&dirvec, &dirvec);
				*pTargetPos = *pEyePos + dirvec * camdist;

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

				if (protmat) {
					*protmat = ChaMatrixRot(curcamera->worldmat);
				}

			}
		}
		else {
			//ZeroFrameCamera
			*pEyePos = curcamera->position;
			*pTargetPos = *pEyePos + curcamera->dirvec * camdist;

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
