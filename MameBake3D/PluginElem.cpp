
#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "PluginElem.h"

#define DBGH
#include <dbg.h>

#include <crtdbg.h>

//#include "MB2Version.h"
#define MB2VERSION 10012

CPluginElem::CPluginElem()
{
	ZeroMemory( filepath, sizeof( char ) * _MAX_PATH );
	pluginversion = 0;
	product = 0;
	id = 0;

	menuid = 0;

	ZeroMemory( pluginname, sizeof( char ) * _MAX_PATH );
	
	validflag = 0;//!!!!!!!!!!!!!!!!!!!!!!

	MBCreateMotionBrush = NULL;
	MBOnClose = NULL;
	MBOnPose = NULL;

	hModule = NULL;

}
CPluginElem::~CPluginElem()
{
	UnloadPlugin();
}

int CPluginElem::CreateMotionBrush(double srcstartframe, double srcendframe, double srcapplyframe, double srcframeleng, int srcrepeats, int srcmirroru, int srcmirrorv, int srcdiv2, float* dstvalue)
{
	int ret = 0;

	if( (validflag == 1) && MBCreateMotionBrush){//！！！！！！！ validflagに注意！！！！！！
		//ret = MBOnSelectPlugin();
		//if ((srcstartframe >= 0.0) && (srcstartframe < 1e5) && (srcendframe >= srcstartframe) && (srcendframe < 1e5) &&
		//	(srcapplyframe >= srcstartframe) && (srcapplyframe <= srcendframe) &&
		//	(srcframeleng > srcendframe) && (srcframeleng < 1e5) && dstvalue) {

		if ((srcstartframe >= 0.0) && 
			(srcstartframe < 1e5) && (srcendframe >= srcstartframe) && 
			(srcapplyframe < 1e5) && (srcapplyframe <= srcendframe) &&
			(srcendframe < 1e5) && (srcframeleng >= srcendframe) &&
			(srcframeleng < 1e5) && dstvalue) {

			ret = MBCreateMotionBrush(srcstartframe, srcendframe, srcapplyframe, srcframeleng, srcrepeats, srcmirroru, srcmirrorv, srcdiv2, dstvalue);
		}
		else {
			_ASSERT(0);
			return 1;
		}
	}

	return ret;
}
int CPluginElem::CallOnClose()
{
	int ret = 0;

	if( (validflag == 1) && MBOnClose ){//！！！！！！！ validflagに注意！！！！！！
		ret = MBOnClose();
	}

	return ret;
}
int CPluginElem::CallOnPose( int motid )
{
	int ret = 0;

	if( (validflag == 1) && MBOnPose ){//！！！！！！！ validflagに注意！！！！！！
		ret = MBOnPose( motid );
	}

	return ret;
}

int CPluginElem::SetFilePath( WCHAR* pluginpath )
{
	int leng;
	leng = (int)wcslen( pluginpath );

	if( (leng <= 0) || (leng >= _MAX_PATH) ){
		DbgOut( L"pluginelem : SetFilePath : leng error !!!\n" );
		_ASSERT( 0 );
		return 1;
	}

	wcscpy_s( filepath, _MAX_PATH, pluginpath );

	return 0;
}


#define GPA(proc) \
	*(FARPROC *)&proc = GetProcAddress(hModule, #proc);


int CPluginElem::LoadPlugin()
{
	validflag = 0;//!!!!!!!!!!!!

	hModule = LoadLibrary( filepath );
	if(hModule == NULL){
		DbgOut( L"pluginelem : LoadPlugin : LoadLibrary error %s!!!\r\n", filepath );
		_ASSERT( 0 );
		return 0;
	}

	GPA(MBGetPlugInName);
	GPA(MBGetPlugInID);
	GPA(MBCheckVersion);
	GPA(MBInit);

	if( !MBGetPlugInName || !MBGetPlugInID || !MBCheckVersion || !MBInit ){
		DbgOut( L"pluginelem : LoadPlugin : this dll is not for MB2 %s\r\n", filepath );
		//DbgOut( L"%x %x %x %x\r\n", MBGetPlugInName, MBGetPlugInID, MBCheckVersion, MBInit );
		return 0;//!!!!!!!!!!!!!!!!!
	}

/////////////

	if( MBGetPlugInName ){
		const WCHAR* nameptr = MBGetPlugInName();
		if( !nameptr ){
			DbgOut( L"pluginelem : LoadPlugin : MBGetPlugInName return NULL error %s\r\n", filepath );
			_ASSERT( 0 );
			return 0;
		}

		int leng;
		leng = (int)wcslen( nameptr );

		if( leng >= _MAX_PATH )
			leng = _MAX_PATH - 1;

		wcsncpy_s( pluginname, _MAX_PATH, nameptr, leng );
		pluginname[leng] = 0;
	}

////////////
	if( MBGetPlugInID ){
		MBGetPlugInID( &product, &id );
	}
////////////
	if( MBCheckVersion ){
		MBCheckVersion( (DWORD)MB2VERSION, &pluginversion );
		if( pluginversion == 0 ){
			DbgOut( L"pluginelem : LoadPlugin : this plugin is not supported by MB2 ver%d : %s\r\n", MB2VERSION, filepath );
			_ASSERT( 0 );
			return 0;
		}
	}


/////////////
	int ret;
	if( MBInit ){

		WCHAR filename[2048];
		ZeroMemory( filename, 2048 );

		ret = GetModuleFileName( NULL, filename, 2048 );
		if( ret == 0 ){
			_ASSERT( 0 );
			DbgOut( L"pluginelem : LoadPlugin : GetModuleFileName error !!!\n" );
			return 1;
		}

		ret = MBInit( filename );
		if( ret ){
			DbgOut( L"pluginelem : LoadPlugin : MBInit error %s\r\n", filepath );
			_ASSERT( 0 );
			return 0;
		}
	}

////////////
	validflag = 1;//!!!!!!!!!!!!!!!!!!!!!!!!!	


////////////
	GPA(MBCreateMotionBrush);
	GPA(MBOnClose);
	GPA(MBOnPose);

	return 0;
}
int CPluginElem::UnloadPlugin()
{
	validflag = 0;//!!!!!!!!!!!!!!!!

	if( hModule ){
		FreeLibrary( hModule );
		hModule = NULL;
	}
	
	return 0;
}


