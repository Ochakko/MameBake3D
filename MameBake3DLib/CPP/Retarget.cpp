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

#define RETARGETCPP
#include <Retarget.h>

#include <GlobalVar.h>
#include <Model.h>
#include <Bone.h>
//#include <ChaVecCalc.h>

#define DBGH
#include <dbg.h>

extern LONG g_retargetbatchflag;

namespace MameBake3DLibRetarget {

	static void RetargetReq(CModel* srcmodel, CModel* srcbvhmodel, CBone* modelbone, 
		double srcframe, CBone* befbvhbone, float hrate, std::map<CBone*, CBone*>& sconvbonemap);
	static int ConvBoneRotation(CModel* srcmodel, CModel* srcbvhmodel, int selfflag, 
		CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate);


	int Retarget(CModel* srcmodel, CModel* srcbvhmodel, ChaMatrix smatVP, 
		std::map<CBone*, CBone*>& sconvbonemap, 
		int (*srcAddMotionFunc)(const WCHAR* wfilename, double srcmotleng), int (*srcInitCurMotionFunc)(int selectflag, double expandmotion))
	{

		//retarget�́@unlimited�ɑ΂��čs���@unlimited�ɃZ�b�g����
		bool limitdegflag = false;

		if (!srcmodel || !srcbvhmodel || !srcAddMotionFunc || !srcInitCurMotionFunc) {
			return 0;
		}

		g_underRetargetFlag = true;//!!!!!!!!!!!!

		MOTINFO* bvhmi = srcbvhmodel->GetMotInfoBegin()->second;
		if (!bvhmi) {
			::MessageBox(NULL, L"motion of bvh is not found error.", L"error!!!", MB_OK);
			g_underRetargetFlag = false;
			return 1;
		}
		double motleng = bvhmi->frameleng;//2022/11/01
		//double motleng = bvhmi->frameleng - 1.0;//2021/10/13
		(srcAddMotionFunc)(0, motleng);
		//(srcInitCurMotionFunc)(0, 0);//CModel::AddMotion�ŏ��������邱�Ƃɂ����̂ŃR�����g�A�E�g�@2022/08/28


		MOTINFO* modelmi = srcmodel->GetCurMotInfo();
		CBone* modelbone;
		if (modelmi) {
			CBone* modeltopbone = srcmodel->GetTopBone();
			CBone* modelhipsbone = 0;
			if (!modeltopbone) {
				::MessageBox(NULL, L"modelside bone is not found error.", L"error!!!", MB_OK);
				g_underRetargetFlag = false;
				return 1;
			}
			else {
				srcmodel->GetHipsBoneReq(modeltopbone, &modelhipsbone);
				if (modelhipsbone) {
					modelbone = modelhipsbone;
				}
				else {
					modelbone = modeltopbone;
				}
			}
			//modelbone = srcmodel->GetTopBone();
		}
		else {
			::MessageBox(NULL, L"modelside motion is not found error.", L"error!!!", MB_OK);
			g_underRetargetFlag = false;
			return 1;
		}


		CBone* bvhtopbone = 0;
		CBone* bvhhipsbone = 0;
		CBone* befbvhbone = 0;
		bvhtopbone = srcbvhmodel->GetTopBone();
		if (bvhtopbone) {
			srcbvhmodel->GetHipsBoneReq(bvhtopbone, &bvhhipsbone);
			if (bvhhipsbone) {
				befbvhbone = bvhhipsbone;
			}
			else {
				befbvhbone = bvhtopbone;
			}
		}
		else {
			::MessageBox(NULL, L"bvhside motion is not found error.", L"error!!!", MB_OK);
			g_underRetargetFlag = false;
			return 1;
		}


		HINFO bvhhi;
		bvhhi.minh = 1e7;
		bvhhi.maxh = -1e7;
		bvhhi.height = 0.0f;
		srcbvhmodel->SetFirstFrameBonePos(&bvhhi, befbvhbone);//hips�w��

		HINFO modelhi;
		modelhi.minh = 1e7;
		modelhi.maxh = -1e7;
		modelhi.height = 0.0f;
		srcmodel->SetFirstFrameBonePos(&modelhi, modelbone);//hips�w��

		float hrate;
		if (bvhhi.height != 0.0f) {
			hrate = modelhi.height / bvhhi.height;
		}
		else {
			//hrate = 0.0f;
			hrate = 1.0f;
			_ASSERT(0);
		}

		//2023/02/08
		if (fabs(hrate) <= 0.0001f) {
			hrate = 1.0f;
		}


		double frame;
		//for (frame = 0.0; frame < motleng; frame += 1.0) {
		for (frame = 1.0; frame < motleng; frame += 1.0) {//2023/03/27 : 0�t���[����InitMP�̎p���̂܂܂ɂ���
			//s_sethipstra = 0;

			if (modelbone) {
				ChaMatrix dummyvpmat;
				ChaMatrixIdentity(&dummyvpmat);

				ChaMatrix tmpbvhwm = srcbvhmodel->GetWorldMat();
				srcbvhmodel->SetMotionFrame(frame);
				srcbvhmodel->UpdateMatrix(limitdegflag, &tmpbvhwm, &dummyvpmat);

				ChaMatrix tmpwm = srcmodel->GetWorldMat();
				srcmodel->SetMotionFrame(frame);
				srcmodel->UpdateMatrix(limitdegflag, &tmpwm, &dummyvpmat);

				if (bvhtopbone) {
					RetargetReq(srcmodel, srcbvhmodel, modelbone, frame, bvhtopbone, hrate, sconvbonemap);
				}
			}
		}

		ChaMatrix tmpwm = srcmodel->GetWorldMat();
		srcmodel->UpdateMatrix(limitdegflag, &tmpwm, &smatVP);

		g_underRetargetFlag = false;//!!!!!!!!!!!!

		//if (!g_retargetbatchflag) {
		if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {
			::MessageBox(NULL, L"Finish of convertion.", L"check!!!", MB_OK);
		}

		return 0;
	}




	void RetargetReq(CModel* srcmodel, CModel* srcbvhmodel, CBone* modelbone, 
		double srcframe, CBone* befbvhbone, float hrate, std::map<CBone*, CBone*>& sconvbonemap)
	{
		if (!srcmodel || !srcbvhmodel) {
			return;
		}

		if (!modelbone) {
			_ASSERT(0);
			return;
		}

		CBone* bvhbone = sconvbonemap[modelbone];
		if (bvhbone) {
			ConvBoneRotation(srcmodel, srcbvhmodel, 1, modelbone, bvhbone, srcframe, befbvhbone, hrate);
		}

		//2023/03/27 �R�����g�A�E�g : �Ή�bvhbone�������ꍇ�́@InitMP�̎p���̂܂܂ɂ���
		//else {
		//	ConvBoneRotation(srcmodel, srcbvhmodel, 0, modelbone, 0, srcframe, befbvhbone, hrate);
		//}



		if (modelbone->GetChild(true)) {
			if (bvhbone) {
				RetargetReq(srcmodel, srcbvhmodel, modelbone->GetChild(true), srcframe, bvhbone, hrate, sconvbonemap);
			}
			else {
				RetargetReq(srcmodel, srcbvhmodel, modelbone->GetChild(true), srcframe, befbvhbone, hrate, sconvbonemap);
			}
		}
		if (modelbone->GetBrother(true)) {
			//if (bvhbone){
			//	ConvBoneConvertReq(modelbone->GetBrother(true), srcframe, bvhbone, hrate);
			//}
			//else{
			RetargetReq(srcmodel, srcbvhmodel, modelbone->GetBrother(true), srcframe, befbvhbone, hrate, sconvbonemap);
			//}
		}

	}

	int ConvBoneRotation(CModel* srcmodel, CModel* srcbvhmodel, int selfflag, 
		CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate)
	{

		//retarget�́@unlimited�ɑ΂��čs���@unlimited�ɃZ�b�g����
		bool limitdegflag = false;


		//2023/03/27 : �Ή�bvhbone�������ꍇ�ɂ́@InitMP�̎p���̂܂܂ɂ���
		if (!bvhbone) {
			return 0;
		}



		if (selfflag && !bvhbone) {
			_ASSERT(0);
			return 1;
		}
		if ((selfflag == 0) && !befbvhbone) {
			_ASSERT(0);
			return 1;
		}

		if (!srcmodel || !srcbvhmodel || !srcbone) {
			_ASSERT(0);
			return 1;
		}


		//###################################################################
		//2023/02/02
		//GetCurMp().GetWorldMat�ɂ́@��O�I�Ƀ��f����worldmat���|�����Ă���
		//�A�j���p���̌v�Z�ɂ́@GetCurMp().GetAnimMat()���g�p
		//###################################################################


		double roundingframe = (double)((int)(srcframe + 0.0001));

		MOTINFO* bvhmi;
		int bvhmotid;

		if (srcbvhmodel->GetMotInfoBegin() != srcbvhmodel->GetMotInfoEnd()) {
			bvhmi = srcbvhmodel->GetMotInfoBegin()->second;
			if (bvhmi) {
				bvhmotid = bvhmi->motid;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			else {
				_ASSERT(0);
				return 1;
			}
		}
		else {
			_ASSERT(0);
			return 1;
		}


		bool onaddmotion = true;//for getbychain
		CMotionPoint bvhmp;
		if (bvhbone) {
			bvhmp = bvhbone->GetCurMp();
		}
		else {
			//bvhmp = befbvhbone->GetCurMp();
			return 0;
		}


		MOTINFO* modelmi = srcmodel->GetCurMotInfo();
		if (modelmi) {
			int modelmotid = modelmi->motid;
			CMotionPoint modelmp;
			CMotionPoint* pmodelmp = 0;
			pmodelmp = srcbone->GetMotionPoint(modelmotid, roundingframe, onaddmotion);
			if (pmodelmp) {
				modelmp = *pmodelmp;
			}
			else {
				_ASSERT(0);
				return 1;
			}

			CMotionPoint modelparmp;
			CMotionPoint* pmodelparmp = 0;
			if (srcbone->GetParent(true)) {
				pmodelparmp = srcbone->GetParent(true)->GetMotionPoint(modelmotid, roundingframe, onaddmotion);
				if (pmodelparmp) {
					modelparmp = *pmodelparmp;
				}
			}


			int curboneno = srcbone->GetBoneNo();


			CQuaternion rotq;
			ChaVector3 traanim;

			if (bvhbone) {
				//ChaMatrix curbvhmat;
				//ChaMatrix bvhmat;
				//bvhmat = bvhmp.GetWorldMat();

				//ChaMatrix modelinit, invmodelinit;
				//modelinit = modelmp.GetWorldMat();
				//invmodelinit = modelmp.GetInvWorldMat();

				CBone* modelfirstbone = 0;
				CBone* modeltopbone = srcmodel->GetTopBone();
				CBone* modelhipsbone = 0;
				if (modeltopbone) {
					srcmodel->GetHipsBoneReq(modeltopbone, &modelhipsbone);
					if (modelhipsbone) {
						modelfirstbone = modelhipsbone;
					}
					else {
						modelfirstbone = modeltopbone;
					}
				}

				CBone* bvhfirstbone = 0;
				CBone* bvhtopbone = srcbvhmodel->GetTopBone();
				CBone* bvhhipsbone = 0;
				if (bvhtopbone) {
					srcbvhmodel->GetHipsBoneReq(bvhtopbone, &bvhhipsbone);
					if (bvhhipsbone) {
						bvhfirstbone = bvhhipsbone;
					}
					else {
						bvhfirstbone = bvhtopbone;
					}
				}


				//if (srcbone == srcmodel->GetTopBone()) {//���f�����̍ŏ��̃{�[���̏�����
				//if (modelfirstbone && (srcbone == modelfirstbone)) {//���f�����̍ŏ��̃{�[���̏�����

				//	//firsthipbvhmat��firsthipmodelmat�́@���̊֐��̎Q�ƈ����@��x�Z�b�g���Ďg���܂킷
				//	
				//	//#######################################################################################
				//	//2022/12/21 ver1.1.0.10�֌�����
				//	//��10033�ƑO����������킹��
				//	//bvh����0�t���[���p����Identity�ɂȂ�悤�Ɂ@InvFirstMat * NodeMat ���|����
				//	//#######################################################################################
				//	firsthipbvhmat = ChaMatrixInv(bvhbone->GetFirstMat()) * bvhbone->GetNodeMat() * bvhmp.GetAnimMat();
				//	firsthipbvhmat.data[MATI_41] = 0.0f;
				//	firsthipbvhmat.data[MATI_42] = 0.0f;
				//	firsthipbvhmat.data[MATI_43] = 0.0f;

				//	firsthipmodelmat = modelmp.GetWorldMat();
				//	firsthipmodelmat.data[MATI_41] = 0.0f;
				//	firsthipmodelmat.data[MATI_42] = 0.0f;
				//	firsthipmodelmat.data[MATI_43] = 0.0f;
				//}


				//#########################################################################################
				//2023/03/26 ver1.2.0.18�֌�����
				//bvh��model���@�����Ƃ��O�t���[���ɃA�j�����݂��Ă��@���^�[�Q�b�g�����܂������悤�Ɂ@�C��
				//#########################################################################################
				ChaMatrix bvhparentmat, modelparentmat;
				bvhparentmat.SetIdentity();
				modelparentmat.SetIdentity();
				if (bvhbone->GetParent(true)) {
					bvhparentmat = ChaMatrixInv(bvhbone->GetParent(true)->GetWorldMat(false, bvhmotid, 0.0, 0)) * bvhbone->GetParent(true)->GetWorldMat(false, bvhmotid, roundingframe, 0);
				}
				else {
					//bvhparentmat.SetIdentity();
					bvhparentmat = ChaMatrixInv(bvhbone->GetWorldMat(false, bvhmotid, 0.0, 0)) * bvhbone->GetWorldMat(false, bvhmotid, roundingframe, 0);
				}
				if (srcbone->GetParent(true)) {
					modelparentmat = ChaMatrixInv(srcbone->GetParent(true)->GetWorldMat(false, modelmotid, 0.0, 0)) * srcbone->GetParent(true)->GetWorldMat(false, modelmotid, roundingframe, 0);
				}
				else {
					//modelparentmat.SetIdentity();
					modelparentmat = ChaMatrixInv(srcbone->GetWorldMat(false, modelmotid, 0.0, 0)) * srcbone->GetWorldMat(false, modelmotid, roundingframe, 0);
				}


				//curbvhmat = bvhbone->GetInvFirstMat() * invmodelinit * bvhmat;
				//curbvhmat = bvhbone->GetInvFirstMat() * sinvfirsthipmat * invmodelinit * bvhmat;
				//curbvhmat = sinvfirsthipmat * bvhbone->GetInvFirstMat() * sfirsthipmat * invmodelinit * bvhmat;//1.0.0.26�ɂȂ�O�܂ł̎��B�����p���̕ϊ���bvh�̑S�̉�]sfirsthipmat���l������B

				//#############################################################################################################################
				//1.0.0.26�����
				//bvh�͓ǂݍ��ݎ��ɂO�t���[���A�j����Identity�ɂȂ�悤�ɓǂݍ��ށBmodel����InvJonitPos * AnimMat�̂悤�ɓǂݍ��ނ悤�ɂ����B
				//model���͂O�t���[���ҏW�ɑΉ������B
				//�ȏ�̕ύX�ɑΉ����邽�߂�retarget�̐������C���B
				//#############################################################################################################################

				//###################################################################################################################
				//1.0.0.27����͂O�t���[���A�j���̕ҏW�ɑΉ��B
				//�O�t���[���ɑΉ��\�Ȃ͔̂�bvh�̃��f���B��bvh�̏ꍇ�A�O�t���[���A�j����Identity�ɂȂ�悤�ɂ͓ǂ܂Ȃ��B
				//��bvh�̏ꍇ�ɂ�BindPose��0�t���[���A�j���̗��������݂���B����ĂO�t���[���A�j���̕ҏW�����ď����o���Ă�����B
				//����Abvh�̏ꍇ�A�O�t���[���A�j����Identity�ɂȂ�悤�ɓǂݍ��ށB���̂��߃��^�[�Q�b�g�̐������ȗ��������B
				//###################################################################################################################
				//curbvhmat = sinvfirsthipmat * srcbone->GetFirstMat() * sfirsthipmat * invmodelinit * bvhmat;//��10027_1 ���܂��s��

				////####################################################################################
				////��10027_1�̍s��|���Z�������N�H�[�^�j�I���ɂ��ăW���o�����b�N���N����ɂ������Ă݂�
				////####################################################################################


				//FirstMat�ɂ���
				//SetFirstMat�́@CBone::InitMP�@�ōs���BInitMP��CModel::AddMotion����Ă΂��B
				//InitMP�͍ŏ��̃��[�V�����̂O�t���[���A�j���ŐV�K���[�V�����̑S�t���[��������������B


				////##############################################################################################################################
				////��10032(1033��)  bvh���̂O�t���[���Ή���model���̂O�t���[���Ή����C�����ā@���́I�I
				//// �O��P�F���^�[�Q�b�g������ bvh����model���̌�������̃|�[�Y�������ł��邱��
				//// �O��Q�Fbvh���͂O�t���[���p����identity(�O�t���[���ɃA�j�����t����ꍇ�̓W���C���g�ʒu�ɗ��Ƃ����ݎp����identity). 
				//// �O��R�Fmodel���͂O�t���[���ɃA�j���������c���Ă���
				//// �O��Q�ƑO��R�ɂ��Ắ@fbx�̓ǂݍ��ݕ������̂悤�ɂ��Ă���(bvh���ɂ̓o�C���h�|�[�Y���������Ƃ��������炱�̂悤�ɂ��Ă���)
				////##############################################################################################################################

				if (modelfirstbone && bvhfirstbone) {

					//#######################################################################################
					//2022/12/21 ver1.1.0.10�֌�����
					//��10033�ƑO����������킹��
					//bvh����0�t���[���p����Identity�ɂȂ�悤�Ɂ@InvFirstMat * NodeMat ���|����
					//#######################################################################################
					ChaMatrix offsetforbvhmat, offsetformodelmat;
					//offsetforbvhmat = ChaMatrixInv(bvhbone->GetFirstMat()) * bvhbone->GetNodeMat();
					//offsetformodelmat.SetIdentity();
					
					//#########################################################################################
					//2023/03/26 ver1.2.0.18�֌�����
					//bvh��model���@�����Ƃ��O�t���[���ɃA�j�����݂��Ă��@���^�[�Q�b�g�����܂������悤�Ɂ@�C��
					//#########################################################################################
					offsetforbvhmat = ChaMatrixInv(bvhbone->GetWorldMat(false, bvhmotid, 0.0, 0));
					offsetformodelmat = ChaMatrixInv(srcbone->GetWorldMat(false, modelmotid, 0.0, 0));


					//######
					//model
					//######
						//model parent
					CQuaternion modelparentQ, invmodelparentQ;
					modelparentQ.RotationMatrix(modelparentmat);
					invmodelparentQ.RotationMatrix(ChaMatrixInv(modelparentmat));

						//model current
					ChaMatrix invmodelcurrentmat;
					CQuaternion invmodelQ;
					invmodelcurrentmat = ChaMatrixInv(offsetformodelmat * modelmp.GetWorldMat());
					invmodelQ.RotationMatrix(invmodelcurrentmat);

						//model zeroframe anim
					ChaMatrix zeroframemodelmat;
					CQuaternion zeroframemodelQ;
					zeroframemodelmat = offsetformodelmat * srcbone->GetCurrentZeroFrameMat(limitdegflag, 1);
					zeroframemodelQ.RotationMatrix(zeroframemodelmat);


					//######
					//bvh
					//######
						//bvh parent
					CQuaternion bvhparentQ, invbvhparentQ;
					bvhparentQ.RotationMatrix(bvhparentmat);
					invbvhparentQ.RotationMatrix(ChaMatrixInv(bvhparentmat));

						//bvh current
					ChaMatrix bvhcurrentmat;
					CQuaternion bvhQ;
					bvhcurrentmat = offsetforbvhmat * bvhmp.GetAnimMat();
					bvhQ.RotationMatrix(bvhcurrentmat);


						////bvh zeroframe anim
					ChaMatrix zeroframebvhmat;
					CQuaternion invzeroframebvhQ;
					zeroframebvhmat = offsetforbvhmat * bvhbone->GetCurrentZeroFrameMat(limitdegflag, 1);
					invzeroframebvhQ.RotationMatrix(ChaMatrixInv(zeroframebvhmat));


				//10033�����̎�
					//ChaMatrix curbvhmat;
					//curbvhmat =
					//	(ChaMatrixInv(firsthipbvhmat) * ChaMatrixInv(bvhbone->GetCurrentZeroFrameMat(1)) * firsthipbvhmat) *
					//	(ChaMatrixInv(firsthipmodelmat) * (ChaMatrixInv(modelmp.GetWorldMat()) * zeroframemodelmat) * firsthipmodelmat) *
					//	bvhmp.GetWorldMat();//2022/10/30 �e�X�g(bvh120, bvh121, Rokoko)�ρ@OK
					//
					//�⑫�Finvhips * (inv)zeroframemat * hips�́@model���W�n�Ƃ�����hips���W�n��zeroframe�p���̌v�Z
					// 
					// 


				//###############################################################################################
				//2023/03/27 �C���F�@firsthipbvhmat��bvhparentmat�Ɂ@firsthipmodelmat��modelparentmat�ɒu������
				//hips���W�n�ł͂Ȃ��@parent���W�n�Ōv�Z
				//###############################################################################################

				//��10033 �ȉ��U�s
					ChaMatrix curbvhmat;
					CQuaternion convQ;
					convQ = bvhQ *
						(invmodelparentQ * (zeroframemodelQ * invmodelQ) * modelparentQ) *
						(invbvhparentQ * invzeroframebvhQ * bvhparentQ);
					curbvhmat = convQ.MakeRotMatX();
					//��10033
					//2022/10/30�e�X�g�̎����N�H�[�^�j�I��(�y�уN�H�[�^�j�I���̊|���Z�̏���)�ɂ��ā@�W���o�����b�N���N�����悤��


					rotq.RotationMatrix(curbvhmat);//��]�����̗p����

					//2023/03/26�@�⑫
					//FKRotate-->RotBoneQReq�ɉ�]��n���ā@�����̎p����rotq���|���邱�ƂɂȂ�
					//���^�[�Q�b�g���ʂ̑�(model��)�̃��[�V������
					//Identity�ł͂Ȃ��@�ŏ��̃��[�V�����̂O�t���[���̎p���ŏ��������Ă���
					//bvh����model���̂O�t���[���̌�������̎p���������ł��邱�Ƃ��@���^�[�Q�b�g����
					//�O�t���[���p������̕ω����𗘗p���ā@���̈Ⴂ�Ȃǂ��z�����Čv�Z����




					//traanim = bvhbone->CalcLocalTraAnim(bvhmotid, roundingframe);//�ړ��͂����炩��擾
					//if (!bvhbone->GetParent(true)) {
					//	ChaVector3 bvhbonepos = bvhbone->GetJointFPos();
					//	ChaVector3 firstframebonepos = bvhbone->GetFirstFrameBonePos();
					//	ChaVector3 firstdiff = firstframebonepos - bvhbonepos;
					//	traanim -= firstdiff;
					//}
					//traanim = traanim * hrate;


					//################################################################################
					//2023/01/08
					//Hips�W���C���g�ȊO��TraAnim���L����
					// 
					//���^�[�Q�b�g�����́@model��bvh�̂O�t���[���̌�������̎p�����������Ƃł��邩��
					//���^�[�Q�b�g����TraAnim�Ƃ��Čv�Z���ׂ��́@�O�t���[������̕ω����ł���
					//################################################################################

					ChaMatrix bvhsmat, bvhrmat, bvhtmat, bvhtanimmat;
					ChaMatrix bvhsmat0, bvhrmat0, bvhtmat0, bvhtanimmat0;

					//GetWorldMat() : limitedflag���[���ɂ��Ă����K�v�L !!!!
					if (bvhbone->GetParent(true)) {
						ChaMatrix parentwm = bvhbone->GetParent(true)->GetWorldMat(limitdegflag, bvhmotid, roundingframe, 0);
						GetSRTandTraAnim(bvhmp.GetAnimMat() * ChaMatrixInv(parentwm), bvhbone->GetNodeMat(), 
							&bvhsmat, &bvhrmat, &bvhtmat, &bvhtanimmat);

						//calc 0 frame
						ChaMatrix parentwm0 = bvhbone->GetParent(true)->GetWorldMat(limitdegflag, bvhmotid, 0.0, 0);
						GetSRTandTraAnim(bvhbone->GetWorldMat(limitdegflag, bvhmotid, 0.0, 0) * ChaMatrixInv(parentwm0), bvhbone->GetNodeMat(),
							&bvhsmat0, &bvhrmat0, &bvhtmat0, &bvhtanimmat0);
					}
					else {
						GetSRTandTraAnim(bvhmp.GetAnimMat(), bvhbone->GetNodeMat(), 
							&bvhsmat, &bvhrmat, &bvhtmat, &bvhtanimmat);

						//calc 0 frame
						GetSRTandTraAnim(bvhbone->GetWorldMat(limitdegflag, bvhmotid, 0.0, 0), bvhbone->GetNodeMat(),
							&bvhsmat0, &bvhrmat0, &bvhtmat0, &bvhtanimmat0);
					}

					traanim = ChaMatrixTraVec(bvhtanimmat) - ChaMatrixTraVec(bvhtanimmat0);//2023/01/08
					traanim = traanim * hrate;
				}
				else {
					rotq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
					traanim = ChaVector3(0.0f, 0.0f, 0.0f);
				}
			}
			else {
				//rotq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
				//traanim = ChaVector3(0.0f, 0.0f, 0.0f);

				return 0;
			}

			bool onretarget = true;
			if (bvhbone) {
				int reqflag = 1;//!!!!!!!!! �ҏW���ʂ��ċA�I�Ɏq���ɓ`����̂Ł@bvhbone�������ꍇ�ɂ͏��������Ȃ��ŗǂ�
				int traanimflag = 1;
				srcmodel->FKRotate(limitdegflag, onretarget, reqflag, bvhbone, 
					traanimflag, traanim, roundingframe, curboneno, rotq);
			}
			else {
				//srcmodel->FKRotate(limitdegflag, onretarget, 0, befbvhbone, 0, traanim, roundingframe, curboneno, rotq);
				return 0;
			}
		}

		return 0;
	}

}