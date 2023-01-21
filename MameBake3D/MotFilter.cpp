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

#include <Model.h>
#include <MotionPoint.h>
#include <Bone.h>
//#include <BoneProp.h>

#include "FilterType.h"
#include "FilterDlg.h"
#include "MotFilter.h"

using namespace std;


CMotFilter::CMotFilter()
{
	InitParams();
}

CMotFilter::~CMotFilter()
{
	DestroyObjs();
}

void CMotFilter::InitParams()
{
	m_filtertype = 0;
	m_filtersize = 0;

	m_eul = 0;
	m_smootheul = 0;
	m_tra = 0;
	m_smoothtra = 0;

}

void CMotFilter::DestroyObjs()
{
	if (m_eul){
		delete[] m_eul;
	}

	if (m_smootheul){
		delete[] m_smootheul;
	}

	if (m_tra){
		delete[] m_tra;
	}

	if (m_smoothtra){
		delete[] m_smoothtra;
	}

	m_eul = 0;
	m_smootheul = 0;
	m_tra = 0;
	m_smoothtra = 0;

}

int CMotFilter::GetFilterType()
{
	CFilterDlg dlg;
	int ret = (int)dlg.DoModal();
	if (ret == IDOK){
		m_filtertype = dlg.GetFilterType();
		m_filtersize = dlg.GetFilterSize();
	}

	return ret;
}


/***********************************************************

　　フィルタサイズが大きいほど、
  　たくさんのフレーム間で平滑がなされる。

   ・移動平均：
	通常の線形による平滑化。

	・加重移動平均：
	徐々に重みを小さくしていく平滑化。
	移動平均よりも、少しピークが残りやすい。

	・ガウシアン：
	ガウシアンフィルタによる平滑化。
	元のモーションの再現率が大きい。（といいなあ）
						   　
***********************************************************/
int CMotFilter::Filter(CModel* srcmodel, int srcmotid, int srcstartframe, int srcendframe)
{
	if (!srcmodel){
		return 0;//!!!!!!!!!!
	}

	DestroyObjs();//!!!!!!!!!!

	int dlgret = GetFilterType();
	if (dlgret != IDOK){
		return 0;//!!!!!!!!!!!!!!!
	}

	int frameleng = srcendframe - srcstartframe + 1;

	m_eul = new ChaVector3[frameleng];//frame番号でアクセスする
	if (!m_eul){
		_ASSERT(0);
		return 1;
	}
	m_smootheul = new ChaVector3[frameleng];
	if (!m_smootheul){
		_ASSERT(0);
		return 1;
	}

	m_tra = new ChaVector3[frameleng];
	if (!m_tra){
		_ASSERT(0);
		return 1;
	}
	m_smoothtra = new ChaVector3[frameleng];
	if (!m_smoothtra){
		_ASSERT(0);
		return 1;
	}

	FilterReq(srcmodel, srcmodel->GetTopBone(), srcmotid, srcstartframe, srcendframe);


	DestroyObjs();

	::MessageBox(NULL, L"平滑化を実行しました。", L"処理終了", MB_OK);

	return 0;
}


void CMotFilter::FilterReq(CModel* srcmodel, CBone* curbone, int srcmotid, int srcstartframe, int srcendframe)
{
	int frameleng = srcendframe - srcstartframe + 1;
	int half_filtersize = m_filtersize / 2;

	ChaVector3 tmp_vec3;
	ChaVector3 tmp_pos3;

	int frame;

	if (curbone){
		ZeroMemory(m_eul, sizeof(ChaVector3) * frameleng);
		ZeroMemory(m_smootheul, sizeof(ChaVector3) * frameleng);
		ZeroMemory(m_tra, sizeof(ChaVector3) * frameleng);
		ZeroMemory(m_smoothtra, sizeof(ChaVector3) * frameleng);

		ChaVector3 befeul = ChaVector3(0.0f, 0.0f, 0.0f);
		for (frame = (int)(srcstartframe + 0.0001); frame <= srcendframe; frame++){
			CMotionPoint curmp;
			curbone->CalcLocalInfo(srcmotid, (double)frame, &curmp);
			ChaVector3 cureul = curbone->CalcLocalEulXYZ(-1, srcmotid, (double)frame, BEFEUL_DIRECT, &befeul);// axiskind = -1 --> m_anglelimitの座標系
			//ChaVector3 cureul = curbone->CalcLocalEulXYZ(-1, srcmotid, (double)frame, BEFEUL_ZERO, 0);// axiskind = -1 --> m_anglelimitの座標系
			ChaVector3 curtra = curbone->CalcLocalTraAnim(srcmotid, (double)frame);

			*(m_eul + frame - (int)(srcstartframe + 0.0001)) = cureul;
			*(m_tra + frame - (int)(srcstartframe + 0.0001)) = curtra;
			//befeul = cureul;
			if ((frame == (int)(srcstartframe + 0.0001)) || IsValidNewEul(cureul, befeul)) {
				befeul = cureul;
			}
		}


		//平滑化処理
		switch (m_filtertype) {
		case AVGF_NONE:							//平滑処理なし
			MoveMemory((void*)m_smootheul, (void*)m_eul, sizeof(ChaVector3) * frameleng);
			MoveMemory((void*)m_smoothtra, (void*)m_tra, sizeof(ChaVector3) * frameleng);
			break;
		case AVGF_MOVING:					//移動平均
			//平滑化処理
			for (frame = (int)(srcstartframe + 0.0001); frame <= srcendframe; frame++){
				tmp_vec3.x = tmp_vec3.y = tmp_vec3.z = 0.0f;
				tmp_pos3.x = tmp_pos3.y = tmp_pos3.z = 0.0f;
				for (int k = -half_filtersize; k <= half_filtersize; k++){
					int index = frame + k - (int)(srcstartframe + 0.0001);
					if ((index < 0) || (index >= frameleng)){
						tmp_vec3 += m_eul[frame - (int)(srcstartframe + 0.0001)];
						tmp_pos3 += m_tra[frame - (int)(srcstartframe + 0.0001)];
					}
					else{
						tmp_vec3 += m_eul[index];
						tmp_pos3 += m_tra[index];
					}
				}
				m_smootheul[frame - (int)(srcstartframe + 0.0001)] = tmp_vec3 / (float)m_filtersize;
				m_smoothtra[frame - (int)(srcstartframe + 0.0001)] = tmp_pos3 / (float)m_filtersize;
			}
			break;

		case AVGF_WEIGHTED_MOVING:			//加重移動平均
		{
			double denomVal = 0.0;
			float weightVal = 0.0;

			int sumd = 0;
			int coef = 0;
			for (int i = 1; i <= m_filtersize; i++){
				sumd += i;
			}
			denomVal = 1.0 / (double)sumd;

			for (frame = (int)(srcstartframe + 0.0001); frame <= srcendframe; frame++){
				tmp_vec3.x = tmp_vec3.y = tmp_vec3.z = 0.0f;
				tmp_pos3.x = tmp_pos3.y = tmp_pos3.z = 0.0f;
				for (int k = -m_filtersize; k <= 0; k++){
					int index = frame + k - (int)(srcstartframe + 0.0001);
					coef = k + m_filtersize;
					weightVal = (float)(denomVal * (double)coef);
					if ((index < 0) || (index >= frameleng)){
						tmp_vec3 = tmp_vec3 + m_eul[frame - (int)(srcstartframe + 0.0001)] * weightVal;
						tmp_pos3 = tmp_pos3 + m_tra[frame - (int)(srcstartframe + 0.0001)] * weightVal;
					}
					else{
						tmp_vec3 = tmp_vec3 + m_eul[index] * weightVal;
						tmp_pos3 = tmp_pos3 + m_tra[index] * weightVal;
					}
				}
				m_smootheul[frame - (int)(srcstartframe + 0.0001)] = tmp_vec3;
				m_smoothtra[frame - (int)(srcstartframe + 0.0001)] = tmp_pos3;
			}
			break;
		}

		case AVGF_GAUSSIAN:					//ガウシアン 

			double normalizeVal = 0.0;
			float normalDistVal = 0.0;

			int N = m_filtersize - 1;
			int sum = 0;
			int r = 0;
			for (int i = 0; i <= N; i++){
				sum += Combi(N, i);
			}
			normalizeVal = 1.0 / (double)sum;

			for (frame = (int)(srcstartframe + 0.0001); frame <= srcendframe; frame++){
				tmp_vec3.x = tmp_vec3.y = tmp_vec3.z = 0.0f;
				tmp_pos3.x = tmp_pos3.y = tmp_pos3.z = 0.0f;
				for (int k = -half_filtersize; k <= half_filtersize; k++){
					int index = frame + k - (int)(srcstartframe + 0.0001);
					r = k + half_filtersize;
					if (r > N){
						r = N;//!!!!!!!!!!!!!!!!!
					}
					normalDistVal = (float)(normalizeVal * (double)Combi(N, r));
					if ((index < 0) || (index >= frameleng)){
						tmp_vec3 = tmp_vec3 + m_eul[frame - (int)(srcstartframe + 0.0001)] * normalDistVal;
						tmp_pos3 = tmp_pos3 + m_tra[frame - (int)(srcstartframe + 0.0001)] * normalDistVal;
					}
					else{
						tmp_vec3 = tmp_vec3 + m_eul[index] * normalDistVal;
						tmp_pos3 = tmp_pos3 + m_tra[index] * normalDistVal;
					}
				}
				m_smootheul[frame - (int)(srcstartframe + 0.0001)] = tmp_vec3;
				m_smoothtra[frame - (int)(srcstartframe + 0.0001)] = tmp_pos3;
			}

			break;
		}

		for (frame = srcstartframe; frame <= srcendframe; frame++){
			ChaMatrix befwm = curbone->GetWorldMat(srcmotid, (double)frame);
			curbone->SetWorldMatFromEulAndTra(1, befwm, m_smootheul[frame - srcstartframe], m_smoothtra[frame - srcstartframe], srcmotid, (double)frame);
			//curbone->SetWorldMatFromEul(0, 1, m_smootheul[frame - srcstartframe], srcmotid, (double)frame);
		}

		if (curbone->GetChild()){
			FilterReq(srcmodel, curbone->GetChild(), srcmotid, srcstartframe, srcendframe);
		}
		if (curbone->GetBrother()){
			FilterReq(srcmodel, curbone->GetBrother(), srcmotid, srcstartframe, srcendframe);
		}
	}
}



//ガウシアンフィルタ用
int CMotFilter::Combi(int n, int r) {
	if (n < 0){
		_ASSERT(0);
		return 0;
	}
	if ((r < 0) || (r > n)){
		_ASSERT(0);
		return 0;
	}
	if (n == r){
		return 1;
	}
	if (r == 0){
		return 1;
	}

	int val0 = n;
	int val1 = r;
	int calccntmax = r;
	int calccnt;
	for (calccnt = calccntmax; calccnt >= 2; calccnt--){
		val0 = val0 * (val0 - 1);
		val1 = val1 * (val1 - 1);
	}

	if (val1 != 0){
		return val0 / val1;
	}
	else{
		return 1;
	}
}

