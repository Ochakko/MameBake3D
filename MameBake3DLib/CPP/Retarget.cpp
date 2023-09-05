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

#include <vector>

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

		//retargetは　unlimitedに対して行い　unlimitedにセットする
		bool limitdegflag = false;

		if (!srcmodel || !srcbvhmodel || !srcAddMotionFunc || !srcInitCurMotionFunc) {
			return 0;
		}

		g_underRetargetFlag = true;//!!!!!!!!!!!!

		//MOTINFO* bvhmi = srcbvhmodel->GetMotInfoBegin()->second;
		//if (!bvhmi) {
		//	::MessageBox(NULL, L"motion of bvh is not found error.", L"error!!!", MB_OK);
		//	g_underRetargetFlag = false;
		//	return 1;
		//}

		//############################################################
		//2023/08/14 : bvh側の全モーションに関してリターゲットループ
		//############################################################
		std::map<int, MOTINFO*>::iterator itrbvhmi;
		for (itrbvhmi = srcbvhmodel->GetMotInfoBegin(); itrbvhmi != srcbvhmodel->GetMotInfoEnd(); itrbvhmi++) {
			MOTINFO* bvhmi = itrbvhmi->second;
			if (!bvhmi) {
				//::MessageBox(NULL, L"motion of bvh is not found error.", L"error!!!", MB_OK);
				//g_underRetargetFlag = false;
				//return 1;
				continue;
			}

			double motleng = bvhmi->frameleng;//2022/11/01
			//double motleng = bvhmi->frameleng - 1.0;//2021/10/13
			int bvhmotid = bvhmi->motid;//motion側のmotid
			srcbvhmodel->SetCurrentMotion(bvhmotid);


			//AddMotion内部でSetCurrentMotionされる
			(srcAddMotionFunc)(0, motleng);//model側のaddmotion　model側のSetCurrentMotionもされる

			//(srcInitCurMotionFunc)(0, 0);//CModel::AddMotionで初期化することにしたのでコメントアウト　2022/08/28


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
			srcbvhmodel->SetFirstFrameBonePos(&bvhhi, befbvhbone);//hips指定

			HINFO modelhi;
			modelhi.minh = 1e7;
			modelhi.maxh = -1e7;
			modelhi.height = 0.0f;
			srcmodel->SetFirstFrameBonePos(&modelhi, modelbone);//hips指定

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
			for (frame = 1.0; frame < motleng; frame += 1.0) {//2023/03/27 : 0フレームはInitMPの姿勢のままにする
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

		}


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

		//2023/03/27 コメントアウト : 対応bvhboneが無い場合は　InitMPの姿勢のままにする
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

		//retargetは　unlimitedに対して行い　unlimitedにセットする
		bool limitdegflag = false;


		//2023/03/27 : 対応bvhboneが無い場合には　InitMPの姿勢のままにする
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
		//GetCurMp().GetWorldMatには　例外的にモデルのworldmatが掛かっている
		//アニメ姿勢の計算には　GetCurMp().GetAnimMat()を使用
		//###################################################################


		double roundingframe = RoundingTime(srcframe);

		MOTINFO* bvhmi;
		int bvhmotid;
		bvhmi = srcbvhmodel->GetCurMotInfo();
		if (!bvhmi) {
			_ASSERT(0);
			return 1;
		}
		bvhmotid = bvhmi->motid;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


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


				//if (srcbone == srcmodel->GetTopBone()) {//モデル側の最初のボーンの処理時
				//if (modelfirstbone && (srcbone == modelfirstbone)) {//モデル側の最初のボーンの処理時

				//	//firsthipbvhmatとfirsthipmodelmatは　この関数の参照引数　一度セットして使いまわす
				//	
				//	//#######################################################################################
				//	//2022/12/21 ver1.1.0.10へ向けて
				//	//式10033と前提条件を合わせる
				//	//bvh側の0フレーム姿勢がIdentityになるように　InvFirstMat * NodeMat を掛ける
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
				//2023/03/26 ver1.2.0.18へ向けて
				//bvh側model側　両方とも０フレームにアニメが在っても　リターゲットがうまくいくように　修正
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
				//curbvhmat = sinvfirsthipmat * bvhbone->GetInvFirstMat() * sfirsthipmat * invmodelinit * bvhmat;//1.0.0.26になる前までの式。初期姿勢の変換にbvhの全体回転sfirsthipmatを考慮する。

				//#############################################################################################################################
				//1.0.0.26からは
				//bvhは読み込み時に０フレームアニメがIdentityになるように読み込む。model側はInvJonitPos * AnimMatのように読み込むようにした。
				//model側は０フレーム編集に対応した。
				//以上の変更に対応するためにretargetの数式も修正。
				//#############################################################################################################################

				//###################################################################################################################
				//1.0.0.27からは０フレームアニメの編集に対応。
				//０フレームに対応可能なのは非bvhのモデル。非bvhの場合、０フレームアニメがIdentityになるようには読まない。
				//非bvhの場合にはBindPoseと0フレームアニメの両方が存在する。よって０フレームアニメの編集をして書き出しても正常。
				//一方、bvhの場合、０フレームアニメがIdentityになるように読み込む。そのためリターゲットの数式が簡略化される。
				//###################################################################################################################
				//curbvhmat = sinvfirsthipmat * srcbone->GetFirstMat() * sfirsthipmat * invmodelinit * bvhmat;//式10027_1 うまく行く

				////####################################################################################
				////式10027_1の行列掛け算部分をクォータニオンにしてジンバルロックが起こりにくくしてみる
				////####################################################################################


				//FirstMatについて
				//SetFirstMatは　CBone::InitMP　で行う。InitMPはCModel::AddMotionから呼ばれる。
				//InitMPは最初のモーションの０フレームアニメで新規モーションの全フレームを初期化する。


				////##############################################################################################################################
				////式10032(1033も)  bvh側の０フレーム対応とmodel側の０フレーム対応を修正して　合体！！
				//// 前提１：リターゲット条件は bvh側とmodel側の見かけ上のポーズが同じであること
				//// 前提２：bvh側は０フレーム姿勢がidentity(０フレームにアニメが付いる場合はジョイント位置に落とし込み姿勢はidentity). 
				//// 前提３：model側は０フレームにアニメ成分を残している
				//// 前提２と前提３については　fbxの読み込み方をそのようにしてある(bvh側にはバインドポーズが無いことが多いからこのようにしてある)
				////##############################################################################################################################

				if (modelfirstbone && bvhfirstbone) {

					//#######################################################################################
					//2022/12/21 ver1.1.0.10へ向けて
					//式10033と前提条件を合わせる
					//bvh側の0フレーム姿勢がIdentityになるように　InvFirstMat * NodeMat を掛ける
					//#######################################################################################
					ChaMatrix offsetforbvhmat, offsetformodelmat;
					//offsetforbvhmat = ChaMatrixInv(bvhbone->GetFirstMat()) * bvhbone->GetNodeMat();
					//offsetformodelmat.SetIdentity();
					
					//#########################################################################################
					//2023/03/26 ver1.2.0.18へ向けて
					//bvh側model側　両方とも０フレームにアニメが在っても　リターゲットがうまくいくように　修正
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


				//10033準備の式
					//ChaMatrix curbvhmat;
					//curbvhmat =
					//	(ChaMatrixInv(firsthipbvhmat) * ChaMatrixInv(bvhbone->GetCurrentZeroFrameMat(1)) * firsthipbvhmat) *
					//	(ChaMatrixInv(firsthipmodelmat) * (ChaMatrixInv(modelmp.GetWorldMat()) * zeroframemodelmat) * firsthipmodelmat) *
					//	bvhmp.GetWorldMat();//2022/10/30 テスト(bvh120, bvh121, Rokoko)済　OK
					//
					//補足：invhips * (inv)zeroframemat * hipsは　model座標系というかhips座標系のzeroframe姿勢の計算
					// 
					// 


				//###############################################################################################
				//2023/03/27 修正：　firsthipbvhmatはbvhparentmatに　firsthipmodelmatはmodelparentmatに置き換え
				//hips座標系ではなく　parent座標系で計算
				//###############################################################################################

				//式10033 以下６行
					ChaMatrix curbvhmat;
					CQuaternion convQ;
					convQ = bvhQ *
						(invmodelparentQ * (zeroframemodelQ * invmodelQ) * modelparentQ) *
						(invbvhparentQ * invzeroframebvhQ * bvhparentQ);
					curbvhmat = convQ.MakeRotMatX();
					//式10033
					//2022/10/30テストの式をクォータニオン(及びクォータニオンの掛け算の順番)にして　ジンバルロックが起こり難いように


					rotq.RotationMatrix(curbvhmat);//回転だけ採用する

					//2023/03/26　補足
					//FKRotate-->RotBoneQReqに回転を渡して　既存の姿勢にrotqを掛けることになる
					//リターゲット結果の側(model側)のモーションは
					//Identityではなく　最初のモーションの０フレームの姿勢で初期化しておく
					//bvh側とmodel側の０フレームの見かけ上の姿勢が同じであることが　リターゲット条件
					//０フレーム姿勢からの変化分を利用して　軸の違いなどを吸収して計算する




					//traanim = bvhbone->CalcLocalTraAnim(bvhmotid, roundingframe);//移動はこちらから取得
					//if (!bvhbone->GetParent(true)) {
					//	ChaVector3 bvhbonepos = bvhbone->GetJointFPos();
					//	ChaVector3 firstframebonepos = bvhbone->GetFirstFrameBonePos();
					//	ChaVector3 firstdiff = firstframebonepos - bvhbonepos;
					//	traanim -= firstdiff;
					//}
					//traanim = traanim * hrate;


					//################################################################################
					//2023/01/08
					//Hipsジョイント以外のTraAnimも有効に
					// 
					//リターゲット条件は　modelとbvhの０フレームの見かけ上の姿勢が同じことであるから
					//リターゲット時にTraAnimとして計算すべきは　０フレームからの変化分である
					//################################################################################

					ChaMatrix bvhsmat, bvhrmat, bvhtmat, bvhtanimmat;
					ChaMatrix bvhsmat0, bvhrmat0, bvhtmat0, bvhtanimmat0;

					//GetWorldMat() : limitedflagをゼロにしておく必要有 !!!!
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
				int reqflag = 1;//!!!!!!!!! 編集結果を再帰的に子供に伝えるので　bvhboneが無い場合には処理をしないで良い
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