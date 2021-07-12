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

#include <crtdbg.h>// <--- _ASSERT�}�N��


#define PI          3.141592f


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

	*ID      = 0x00000007;

	return 0;
}


//---------------------------------------------------------------------------
//  MBGetPlugInName
//    �v���O�C������Ԃ��B
//    MotionBrush�̃u���V�̎�ނ̃R���{�{�b�N�X�ɁA���̖��O���\�������B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT const WCHAR* MBGetPlugInName(void)
{
	// �v���O�C����
	return L"Up_Cos";
}

//---------------------------------------------------------------------------
//  MBOnClose
//    MotionBrush���I�����钼�O��MotionBrush����Ă΂��B
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnClose(void)
{
	return 0;
}

//---------------------------------------------------------------------------
//  MBOnPose
//   ���ݖ��g�p
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnPose( int motid )
{
	return 0;
}


//----------------------------------------------------------------------------
//  MBOnSelectPlugin
//  MotionBrush�ŕ����t���[����I���������A�u���V�̎�ނ�I�񂾎��A�u���V�̃p�����[�^��ς����Ƃ��ɌĂ΂�܂��B
//  �I�����ꂽ�����t���[�����̃u���V�l���v�Z���Ĕz��dstvalue�ɃZ�b�g���܂��B
//----------------------------------------------------------------------------

MBPLUGIN_EXPORT int MBCreateMotionBrush(double srcstartframe, double srcendframe, double srcapplyframe, double srcframeleng, int srcrepeats, int srcmirroru, int srcmirrorv, int srcdiv2, float* dstvalue)
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
				bool div2;

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
					div2 = srcdiv2;
					if ((repeatscnt % 2) == 0) {
						minusv = false;
					}
					else {
						minusv = true;
					}
				}
				else {
					div2 = false;
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
						//if ((framecnt == startframe) || (framecnt == endframe)) {
						//	//��`�ȊO�@���[�O
						//	curscale = 0.0;
						//}
						if ((framecnt == startframe) && (framecnt != applyframe)) {
							if (invu) {
								if ((repeatscnt % 2) == 0) {
									curscale = 0.0f;
								}
								else {
									if (minusv) {
										if (div2) {
											curscale = 0.0f;
										}
										else {
											curscale = -1.0f;
										}
									}
									else {
										curscale = 1.0f;
									}
								}
							}
							else {
								if (repeatscnt == 0) {
									curscale = 0.0f;
								}
								else {
									if (div2) {
										curscale = 0.5f;
									}
									else {
										curscale = 0.0f;
									}
								}
							}
						}
						else if ((framecnt == endframe) && (framecnt != applyframe)) {
							if (invu) {
								if (minusv) {
									if (repeatscnt == (repeats - 1)) {
										curscale = 0.0f;
									}
									else {
										if ((repeatscnt % 2) == 0) {
											curscale = 1.0f;
										}
										else {
											if (div2) {
												curscale = 0.5f;
											}
											else {
												curscale = 1.0f;
											}
										}
									}
								}
								else {
									curscale = 0.0f;
								}
							}
							else {
								if (minusv) {
									if (div2) {
										curscale = 0.0f;
									}
									else {
										curscale = -1.0f;
									}
								}
								else {
									curscale = 1.0f;
								}
							}
						}
						else if (framecnt < applyframe) {
							if (repeatscnt == 0) {
								
								//repeatscnt == 0 ��minusv�͖���
								//curscale = (float)(pow(((double)framecnt - startframe) / halfcnt1, 2.0));
								//curscale = (cos(PI + PI * ((framecnt - startframe) / halfcnt1)));
								curscale = (1.0 + cos(PI + PI * ((framecnt - startframe) / halfcnt1))) * 0.5;
							}
							else {
								if (invu) {
									if ((repeatscnt % 2) == 0) {
										//curscale = (float)(pow(((double)framecnt - startframe) / halfcnt1, 2.0));
										//curscale = (cos(PI + PI * ((framecnt - startframe) / halfcnt1)));
										curscale = (1.0 + cos(PI + PI * ((framecnt - startframe) / halfcnt1))) * 0.5;
									}
									else {
										curscale = 1.0f;
									}
									if (minusv) {
										curscale *= -1.0f;
									}
									if (div2) {
										curscale = (curscale + 1.0) * 0.5f;
									}
								}
								else {
									//curscale = (float)(pow(((double)framecnt - startframe) / halfcnt1, 2.0));
									//curscale = (cos(PI + PI * ((framecnt - startframe) / halfcnt1)));
									curscale = (1.0 + cos(PI + PI * ((framecnt - startframe) / halfcnt1))) * 0.5;
									if (minusv) {
										curscale *= -1.0f;
									}
									if (div2) {
										curscale = (curscale + 1.0) * 0.5f;
									}
								}
							}
						}
						else if ((framecnt > applyframe) && (framecnt < endframe)) {
							if (repeatscnt == (repeats - 1)) {
								if (invu) {
									if ((repeatscnt % 2) == 0) {
										if (minusv) {
											if (div2) {
												curscale = 0.0f;
											}
											else {
												curscale = -1.0f;
											}
										}
										else {
											curscale = 1.0f;
										}
									}
									else {
										//curscale = 1.0f - (float)(pow(((double)endframe - framecnt) / halfcnt2, 2.0));
										curscale = (cos(PI + PI * ((endframe - framecnt) / halfcnt2)));
										curscale = (curscale + 1.0) * 0.5f;
										if (minusv) {
											curscale *= -1.0f;
										}
										if (div2) {
											curscale = (curscale + 1.0) * 0.5f;
										}
									}
								}
								else {
									if (minusv) {
										if (div2) {
											curscale = 0.0f;
										}
										else {
											curscale = -1.0f;
										}
									}
									else {
											curscale = 1.0f;
									}
								}
							}
							else {
								if (invu) {
									if ((repeatscnt % 2) == 0) {
										if (minusv) {
											if (div2) {
												curscale = 0.0f;
											}
											else {
												curscale = -1.0f;
											}
										}
										else {
											curscale = 1.0f;
										}
									}
									else {
										//curscale = 1.0f - (float)(pow(((double)endframe - framecnt) / halfcnt2, 2.0));
										//curscale = curscale * -1.0f + 1.0f;
										curscale = (cos(PI + PI * ((endframe - framecnt) / halfcnt2)));
										curscale = (curscale + 1.0) * 0.5f;
										if (minusv) {
											curscale *= -1.0f;
										}
										if (div2) {
											curscale = (curscale + 1.0) * 0.5f;
										}
									}
								}
								else {
									if (minusv) {
										if (div2) {
											curscale = 0.0f;
										}
										else {
											curscale = -1.0f;
										}
									}
									else {
										curscale = 1.0f;
									}
								}
							}
						}
						else if (framecnt == applyframe) {
							curscale = 1.0;
							if (minusv) {
								curscale *= -1.0f;
							}
							if (div2) {
								curscale = (curscale + 1.0) * 0.5f;
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

	//MessageBox( NULL, L"SawMotionBrush�̏������I���܂����B", L"SawMotionBrush", MB_OK );

	return 0;
}

