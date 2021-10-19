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

	static void RetargetReq(CModel* srcmodel, CModel* srcbvhmodel, CBone* modelbone, double srcframe, CBone* befbvhbone, float hrate, ChaMatrix& sfirsthipmat, ChaMatrix& sinvfirsthipmat, std::map<CBone*, CBone*>& sconvbonemap);
	static int ConvBoneRotation(CModel* srcmodel, CModel* srcbvhmodel, int selfflag, CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate, ChaMatrix& sfirsthipmat, ChaMatrix& sinvfirsthipmat);


	int Retarget(CModel* srcmodel, CModel* srcbvhmodel, ChaMatrix smatVP, std::map<CBone*, CBone*>& sconvbonemap, int (*srcAddMotionFunc)(WCHAR* wfilename, double srcmotleng), int (*srcInitCurMotionFunc)(int selectflag, double expandmotion))
	{
		if (!srcmodel || !srcbvhmodel || !srcAddMotionFunc || !srcInitCurMotionFunc) {
			return 0;
		}

		g_underRetargetFlag = true;//!!!!!!!!!!!!

		MOTINFO* bvhmi = srcbvhmodel->GetMotInfoBegin()->second;
		if (!bvhmi) {
			::MessageBox(NULL, L"motion of bvh is not found error.", L"error!!!", MB_OK);
			return 1;
		}
		//double motleng = bvhmi->frameleng;
		double motleng = bvhmi->frameleng - 1.0;//2021/10/13
		(srcAddMotionFunc)(0, motleng);
		(srcInitCurMotionFunc)(0, 0);


		MOTINFO* modelmi = srcmodel->GetCurMotInfo();
		CBone* modelbone;
		if (modelmi) {
			modelbone = srcmodel->GetTopBone();
		}
		else {
			::MessageBox(NULL, L"modelside motion is not found error.", L"error!!!", MB_OK);
			return 1;
		}

		HINFO bvhhi;
		bvhhi.minh = 1e7;
		bvhhi.maxh = -1e7;
		bvhhi.height = 0.0f;
		srcbvhmodel->SetFirstFrameBonePos(&bvhhi);

		HINFO modelhi;
		modelhi.minh = 1e7;
		modelhi.maxh = -1e7;
		modelhi.height = 0.0f;
		srcmodel->SetFirstFrameBonePos(&modelhi);

		float hrate;
		if (bvhhi.height != 0.0f) {
			hrate = modelhi.height / bvhhi.height;
		}
		else {
			hrate = 0.0f;
			_ASSERT(0);
		}

		double frame;
		for (frame = 0.0; frame < motleng; frame += 1.0) {
			//s_sethipstra = 0;

			if (modelbone) {
				srcmodel->SetMotionFrame(frame);
				CBone* befbvhbone = srcbvhmodel->GetTopBone();

				ChaMatrix sfirsthipmat;
				ChaMatrix sinvfirsthipmat;
				ChaMatrixIdentity(&sfirsthipmat);
				ChaMatrixIdentity(&sinvfirsthipmat);

				if (befbvhbone) {
					RetargetReq(srcmodel, srcbvhmodel, modelbone, frame, befbvhbone, hrate, sfirsthipmat, sinvfirsthipmat, sconvbonemap);
				}
			}
		}

		srcmodel->UpdateMatrix(&srcmodel->GetWorldMat(), &smatVP);

		g_underRetargetFlag = false;//!!!!!!!!!!!!

		//if (!g_retargetbatchflag) {
		if (InterlockedAdd(&g_retargetbatchflag, 0) == 0) {
			::MessageBox(NULL, L"Finish of convertion.", L"check!!!", MB_OK);
		}

		return 0;
	}




	void RetargetReq(CModel* srcmodel, CModel* srcbvhmodel, CBone* modelbone, double srcframe, CBone* befbvhbone, float hrate, ChaMatrix& sfirsthipmat, ChaMatrix& sinvfirsthipmat, std::map<CBone*, CBone*>& sconvbonemap)
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
			ConvBoneRotation(srcmodel, srcbvhmodel, 1, modelbone, bvhbone, srcframe, befbvhbone, hrate, sfirsthipmat, sinvfirsthipmat);
		}
		else {
			ConvBoneRotation(srcmodel, srcbvhmodel, 0, modelbone, 0, srcframe, befbvhbone, hrate, sfirsthipmat, sinvfirsthipmat);
		}


		if (modelbone->GetChild()) {
			if (bvhbone) {
				RetargetReq(srcmodel, srcbvhmodel, modelbone->GetChild(), srcframe, bvhbone, hrate, sfirsthipmat, sinvfirsthipmat, sconvbonemap);
			}
			else {
				RetargetReq(srcmodel, srcbvhmodel, modelbone->GetChild(), srcframe, befbvhbone, hrate, sfirsthipmat, sinvfirsthipmat, sconvbonemap);
			}
		}
		if (modelbone->GetBrother()) {
			//if (bvhbone){
			//	ConvBoneConvertReq(modelbone->GetBrother(), srcframe, bvhbone, hrate);
			//}
			//else{
			RetargetReq(srcmodel, srcbvhmodel, modelbone->GetBrother(), srcframe, befbvhbone, hrate, sfirsthipmat, sinvfirsthipmat, sconvbonemap);
			//}
		}

	}

	int ConvBoneRotation(CModel* srcmodel, CModel* srcbvhmodel, int selfflag, CBone* srcbone, CBone* bvhbone, double srcframe, CBone* befbvhbone, float hrate, ChaMatrix& sfirsthipmat, ChaMatrix& sinvfirsthipmat)
	{
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

		//static ChaMatrix s_firsthipmat;
		//static ChaMatrix s_invfirsthipmat;

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


		CMotionPoint bvhmp;
		if (bvhbone) {
			CMotionPoint* pbvhmp = 0;
			pbvhmp = bvhbone->GetMotionPoint(bvhmotid, srcframe);
			if (pbvhmp) {
				bvhmp = *pbvhmp;
			}
			else {
				_ASSERT(0);
				return 1;
			}
		}
		else {
			CMotionPoint* pbvhmp = 0;
			pbvhmp = befbvhbone->GetMotionPoint(bvhmotid, srcframe);
			if (pbvhmp) {
				bvhmp = *pbvhmp;
			}
			else {
				_ASSERT(0);
				return 1;
			}
		}


		MOTINFO* modelmi = srcmodel->GetCurMotInfo();
		if (modelmi) {
			int modelmotid = modelmi->motid;
			CMotionPoint modelmp;
			CMotionPoint* pmodelmp = 0;
			pmodelmp = srcbone->GetMotionPoint(modelmotid, srcframe);
			if (pmodelmp) {
				modelmp = *pmodelmp;
			}
			else {
				_ASSERT(0);
				return 1;
			}

			CMotionPoint modelparmp;
			CMotionPoint* pmodelparmp = 0;
			if (srcbone->GetParent()) {
				pmodelparmp = srcbone->GetParent()->GetMotionPoint(modelmotid, srcframe);
				if (pmodelparmp) {
					modelparmp = *pmodelparmp;
				}
			}


			int curboneno = srcbone->GetBoneNo();


			CQuaternion rotq;
			ChaVector3 traanim;

			if (bvhbone) {
				ChaMatrix curbvhmat;
				ChaMatrix bvhmat;
				bvhmat = bvhmp.GetWorldMat();

				ChaMatrix modelinit, invmodelinit;
				modelinit = modelmp.GetWorldMat();
				invmodelinit = modelmp.GetInvWorldMat();

				if (srcbone == srcmodel->GetTopBone()) {//���f�����̍ŏ��̃{�[���̏�����
					sfirsthipmat = bvhmp.GetWorldMat();
					sfirsthipmat._41 = 0.0f;
					sfirsthipmat._42 = 0.0f;
					sfirsthipmat._43 = 0.0f;
					ChaMatrixInverse(&sinvfirsthipmat, NULL, &sfirsthipmat);
					sinvfirsthipmat._41 = 0.0f;
					sinvfirsthipmat._42 = 0.0f;
					sinvfirsthipmat._43 = 0.0f;
				}


				//curbvhmat = bvhbone->GetInvFirstMat() * invmodelinit * bvhmat;
				//curbvhmat = bvhbone->GetInvFirstMat() * sinvfirsthipmat * invmodelinit * bvhmat;
				curbvhmat = sinvfirsthipmat * bvhbone->GetInvFirstMat() * sfirsthipmat * invmodelinit * bvhmat;//�����p���̕ϊ���bvh�̑S�̉�]sfirsthipmat���l������B

				CMotionPoint curbvhrotmp;
				curbvhrotmp.CalcQandTra(curbvhmat, bvhbone);
				rotq = curbvhrotmp.GetQ();

				traanim = bvhbone->CalcLocalTraAnim(bvhmotid, srcframe);
				if (!bvhbone->GetParent()) {
					ChaVector3 bvhbonepos = bvhbone->GetJointFPos();
					ChaVector3 firstframebonepos = bvhbone->GetFirstFrameBonePos();
					ChaVector3 firstdiff = firstframebonepos - bvhbonepos;
					traanim -= firstdiff;
				}
				traanim = traanim * hrate;

			}
			else {
				rotq.SetParams(1.0f, 0.0f, 0.0f, 0.0f);
				traanim = ChaVector3(0.0f, 0.0f, 0.0f);
			}

			if (bvhbone) {
				srcmodel->FKRotate(1, bvhbone, 1, traanim, srcframe, curboneno, rotq);
			}
			else {
				srcmodel->FKRotate(0, befbvhbone, 0, traanim, srcframe, curboneno, rotq);
			}
		}

		return 0;
	}

}