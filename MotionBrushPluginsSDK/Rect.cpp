#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <Commdlg.h>

#include "MBPlugin.h"
//#include "../coef_r.h"

#include <crtdbg.h>// <--- _ASSERT�}�N��


extern HINSTANCE g_hInstance;
static HANDLE hfile = INVALID_HANDLE_VALUE;

static int Write2File( WCHAR* lpFormat, ... );

#define PI          3.14159265358979323846f


//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
//BOOL APIENTRY DllMain( HANDLE hModule, 
//                       DWORD  ul_reason_for_call, 
//                       LPVOID lpReserved
//					 )
//{
//	//�_�C�A���O�{�b�N�X�̕\���ɕK�v�Ȃ̂ŁA�C���X�^���X��ۑ����Ă���
//	hInstance = (HINSTANCE)hModule;
//
//	//�v���O�C���Ƃ��Ă͓��ɕK�v�ȏ����͂Ȃ��̂ŁA����������TRUE��Ԃ�
//    return TRUE;
//}


//---------------------------------------------------------------------------
//  MBGetPlugInID
//    �v���O�C��ID��Ԃ��B
//    ���̊֐��͋N�����ɌĂяo�����B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBGetPlugInID(DWORD *Product, DWORD *ID)
{
	// �v���_�N�g��(����Җ�)��ID���A�S����64bit�̒l�Ƃ��ĕԂ�
	// �l�͑��Əd�����Ȃ��悤�ȃ����_���Ȃ��̂ŗǂ�

	char* cproduct;
	cproduct = (char*)Product;
	*cproduct = 'o';
	*( cproduct + 1 ) = 'c';
	*( cproduct + 2 ) = 'h';
	*( cproduct + 3 ) = 'a';

	*ID      = 0x00000004;

	return 0;
}


//---------------------------------------------------------------------------
//  MBGetPlugInName
//    �v���O�C������Ԃ��B
//    RokDeBone2�̃v���O�C�����j���[�ɁA���̖��O���\�������B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT const WCHAR* MBGetPlugInName(void)
{
	// �v���O�C����
	return L"Rect";
}

//---------------------------------------------------------------------------
//  MBOnClose
//    RokDeBone2���I�����钼�O��RokDeBone2����Ă΂��B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnClose(void)
{
	return 0;
}

//---------------------------------------------------------------------------
//  MBOnPose
//    RokDeBone2�Ń��[�U�[���{�[���̎p����ҏW���A�}�E�X�𗣂����Ƃ���RokDeBone2����Ă΂��B
//    ���[�V�����|�C���g�͕ύX����Ă��邪FillUp�͂���Ă��Ȃ���ԂŌĂ΂��B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnPose( int motid )
{
	return 0;
}


//----------------------------------------------------------------------------
//  MBOnSelectPlugin
//  RokDeBone2�̃v���O�C�����j���[�ŁAMBGetPlugInName�̕������I�������Ƃ��ɁA
//  ���̊֐����A���A�Ă΂�܂��B
//----------------------------------------------------------------------------

MBPLUGIN_EXPORT int MBCreateMotionBrush(double srcstartframe, double srcendframe, double srcapplyframe, double srcframeleng, int srcrepeats, int srcmirroru, int srcmirrorv, float* dstvalue)
{
	int MB2version;
	if (MBGetVersion) {
		MBGetVersion(&MB2version);
		//Write2File( L"MotionBrush version : %d\r\n", MB2version );
	}

	if ((srcstartframe >= 0.0) && (srcstartframe < 1e5) && (srcendframe >= srcstartframe) && (srcendframe < 1e5) &&
		(srcapplyframe >= srcstartframe) && (srcapplyframe <= srcendframe) &&
		(srcframeleng > srcendframe) && (srcframeleng < 1e5) && 
		(srcrepeats >= 1) && (srcrepeats <= 10) &&
		dstvalue) {

		double numframe = srcendframe - srcstartframe + 1;
		::memset(dstvalue, 0, sizeof(float) * srcframeleng);


		//�P�������R�t���[���ȏ�ɂȂ�悤�Ɏ��ۂ̌J��Ԃ��񐔂Ǝ����𒲐�����
		int repeats;
		repeats = srcrepeats;
		int frameT;//����(�t���[����)
		frameT = (int)(numframe / srcrepeats);
		while ((frameT >= (srcapplyframe + 1)) && (frameT < 3) && (repeats >= 2)) {
			repeats--;
			frameT = (int)(numframe / srcrepeats);
		}

		if (frameT >= 3) {
			double halfcnt1, halfcnt2;
			double tangent1, tangent2;


			int repeatscnt;
			for (repeatscnt = 0; repeatscnt < repeats; repeatscnt++) {
				int startframe;
				int endframe;
				int applyframe;
				bool invu;
				bool minusv;

				startframe = srcstartframe + repeatscnt * frameT;
				endframe = startframe + frameT;
				applyframe = startframe + (srcapplyframe - srcstartframe);
				if (srcmirroru) {
					if ((repeatscnt % 2) == 0) {
						invu = false;
					}
					else {
						invu = true;
					}
				}
				else {
					invu = false;
				}
				if (srcmirrorv) {
					if ((repeatscnt % 2) == 0) {
						minusv = false;
					}
					else {
						minusv = true;
					}
				}
				else {
					minusv = false;
				}



				if (invu) {
					applyframe = startframe + (endframe - applyframe);
				}

				int framecnt;
				halfcnt1 = (applyframe - startframe);
				halfcnt2 = (endframe - applyframe);
				tangent1 = 1.0 / halfcnt1;
				tangent2 = 1.0 / halfcnt2;

				for (framecnt = startframe; framecnt <= endframe; framecnt++) {
					float curscale;
					if ((framecnt >= (int)startframe) && (framecnt <= endframe)) {
						if ((framecnt == startframe) || (framecnt == endframe)) {
							//��`�ȊO�@���[�O
							curscale = 0.0;
						}
						else if (framecnt < applyframe) {
							curscale = 1.0f;
							if (minusv) {
								curscale *= -1.0f;
							}
						}
						else if ((framecnt > applyframe) && (framecnt < endframe)) {
							curscale = 1.0f;
							if (minusv) {
								curscale *= -1.0f;
							}
						}
						else if (framecnt == applyframe) {
							curscale = 1.0;
							if (minusv) {
								curscale *= -1.0f;
							}
						}
						else {
							_ASSERT(0);
							curscale = 0.0;
						}
					}
					else {
						//�I��͈͈ȊO�O
						curscale = 0.0;
					}
					*(dstvalue + (int)framecnt) = curscale;
				}
			}
		}
		else {
			double framecnt;
			for (framecnt = 0.0; framecnt < srcframeleng; framecnt++) {
				float curscale;
				if ((framecnt >= (int)srcstartframe) && (framecnt <= srcendframe)) {
					curscale = 1.0;
				}
				else {
					curscale = 0.0;
				}
				*(dstvalue + (int)framecnt) = curscale;
			}
		}
	}
	else {
		_ASSERT(0);
		return 1;
	}


//OnSelectExit:
	if( hfile != INVALID_HANDLE_VALUE ){
		FlushFileBuffers( hfile );
		SetEndOfFile( hfile );
		CloseHandle( hfile );
		hfile = INVALID_HANDLE_VALUE;
	}

	//MessageBox( NULL, L"SawMotionBrush�̏������I���܂����B", L"SawMotionBrush", MB_OK );

	return 0;
}


int Write2File( WCHAR* lpFormat, ... )
{
	if( hfile == INVALID_HANDLE_VALUE ){
		return 0;
	}

	int ret;
	va_list Marker;
	unsigned long wleng, writeleng;
	WCHAR outchar[4098];
			
	va_start( Marker, lpFormat );
	ret = vswprintf_s( outchar, 4096, lpFormat, Marker );
	va_end( Marker );

	if( ret < 0 )
		return 1;

	wleng = (unsigned long)wcslen( outchar );
	WriteFile( hfile, outchar, wleng, &writeleng, NULL );
	if( wleng != writeleng ){
		return 1;
	}

	return 0;	
}
