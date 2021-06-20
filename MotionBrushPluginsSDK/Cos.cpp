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

#include <crtdbg.h>// <--- _ASSERTマクロ


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
//	//ダイアログボックスの表示に必要なので、インスタンスを保存しておく
//	hInstance = (HINSTANCE)hModule;
//
//	//プラグインとしては特に必要な処理はないので、何もせずにTRUEを返す
//    return TRUE;
//}


//---------------------------------------------------------------------------
//  MBGetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBGetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い

	char* cproduct;
	cproduct = (char*)Product;
	*cproduct = 'o';
	*( cproduct + 1 ) = 'c';
	*( cproduct + 2 ) = 'h';
	*( cproduct + 3 ) = 'a';

	*ID      = 0x00000002;

	return 0;
}


//---------------------------------------------------------------------------
//  MBGetPlugInName
//    プラグイン名を返す。
//    RokDeBone2のプラグインメニューに、この名前が表示される。
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT const WCHAR* MBGetPlugInName(void)
{
	// プラグイン名
	return L"Cos";
}

//---------------------------------------------------------------------------
//  MBOnClose
//    RokDeBone2が終了する直前にRokDeBone2から呼ばれる。
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnClose(void)
{
	return 0;
}

//---------------------------------------------------------------------------
//  MBOnPose
//    RokDeBone2でユーザーがボーンの姿勢を編集し、マウスを離したときにRokDeBone2から呼ばれる。
//    モーションポイントは変更されているがFillUpはされていない状態で呼ばれる。
//---------------------------------------------------------------------------
MBPLUGIN_EXPORT int MBOnPose( int motid )
{
	return 0;
}


//----------------------------------------------------------------------------
//  MBOnSelectPlugin
//  RokDeBone2のプラグインメニューで、MBGetPlugInNameの文字列を選択したときに、
//  この関数が、一回、呼ばれます。
//----------------------------------------------------------------------------

MBPLUGIN_EXPORT int MBCreateMotionBrush(double srcstartframe, double srcendframe, double srcapplyframe, double srcframeleng, float* dstvalue)
{
	int MB2version;
	if (MBGetVersion) {
		MBGetVersion(&MB2version);
		//Write2File( L"MotionBrush version : %d\r\n", MB2version );
	}

	if ((srcstartframe >= 0.0) && (srcstartframe < 1e5) && (srcendframe >= srcstartframe) && (srcendframe < 1e5) &&
		(srcapplyframe >= srcstartframe) && (srcapplyframe <= srcendframe) &&
		(srcframeleng > srcendframe) && (srcframeleng < 1e5) && dstvalue) {

		double numframe = srcendframe - srcstartframe + 1;
		::memset(dstvalue, 0, sizeof(float) * srcframeleng);


		if (numframe >= 3) {
			double halfcnt1, halfcnt2;
			double tangent1, tangent2;

			int framecnt;
			halfcnt1 = (srcapplyframe - srcstartframe);
			halfcnt2 = (srcendframe - srcapplyframe);
			tangent1 = 1.0 / halfcnt1;
			tangent2 = 1.0 / halfcnt2;

			for (framecnt = 0; framecnt < srcframeleng; framecnt++) {
				float curscale;
				if ((framecnt >= (int)srcstartframe) && (framecnt <= srcendframe)) {
					if ((framecnt == srcstartframe) || (framecnt == srcendframe)) {
						//矩形以外　両端０
						curscale = 0.0;
					}
					else if (framecnt < srcapplyframe) {
						curscale = (1.0 + cos(PI + PI * (framecnt - srcstartframe) / halfcnt1)) * 0.5;
					}
					else if ((framecnt > srcapplyframe) && (framecnt < srcendframe)) {
						curscale = (1.0 + cos(PI + PI * (srcendframe - framecnt) / halfcnt2)) * 0.5;
					}
					else if (framecnt == srcapplyframe) {
						curscale = 1.0;
					}
					else {
						_ASSERT(0);
						curscale = 0.0;
					}
				}
				else {
					//選択範囲以外０
					curscale = 0.0;
				}
				*(dstvalue + (int)framecnt) = curscale;
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


OnSelectExit:
	if( hfile != INVALID_HANDLE_VALUE ){
		FlushFileBuffers( hfile );
		SetEndOfFile( hfile );
		CloseHandle( hfile );
		hfile = INVALID_HANDLE_VALUE;
	}

	//MessageBox( NULL, L"SawMotionBrushの処理を終わりました。", L"SawMotionBrush", MB_OK );

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
