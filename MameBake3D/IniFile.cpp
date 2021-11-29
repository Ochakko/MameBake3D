#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include "IniFile.h"

#include <GlobalVar.h>

#define DBGH
#include <dbg.h>


CIniFile::CIniFile()
{
	InitParams();
}

CIniFile::~CIniFile()
{
	DestroyObjs();
}

int CIniFile::InitParams()
{
	CXMLIO::InitParams();

	return 0;
}

int CIniFile::DestroyObjs()
{
	CXMLIO::DestroyObjs();
	InitParams();

	return 0;
}

int CIniFile::WriteIniFile(WCHAR* srcpath)
{
	if (!srcpath) {
		_ASSERT(0);
		return 1;
	}

	m_hfile = CreateFile( srcpath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		DbgOut( L"IniFile : WriteIniFile : file open error !!!\n" );
		_ASSERT( 0 );
		return 1;
	}

	CallF( Write2File( "<?xml version=\"1.0\" encoding=\"Shift_JIS\"?>\r\n<INI>\r\n" ), return 1 );  
	CallF( WriteFileInfo(), return 1 );
	
	CallF(WriteIniInfo(), return 1);

	CallF(Write2File("\r\n</INI>\r\n"), return 1);


	return 0;
}

int CIniFile::WriteFileInfo()
{

	CallF( Write2File( "  <FileInfo>\r\n    <kind>MotionBrushIniFile</kind>\r\n    <version>1001</version>\r\n    <type>0</type>\r\n  </FileInfo>\r\n" ), return 1 );

	return 0;
}

int CIniFile::WriteIniInfo()
{
	CallF(Write2File("  <IniFileBody>\r\n"), return 1);

	CallF(Write2File("    <ClearColorIndex>%d</ClearColorIndex>\r\n", g_ClearColorIndex), return 1);
	CallF(Write2File("    <UsePhysIK>%d</UsePhysIK>\r\n", g_usephysik), return 1);
	CallF(Write2File("    <PrepCntOnPhysIK>%d</PrepCntOnPhysIK>\r\n", g_prepcntonphysik), return 1);

	CallF(Write2File("  </IniFileBody>\r\n"), return 1);

	return 0;
}

int CIniFile::LoadIniFile(WCHAR* srcpath)
{
	if (!srcpath) {
		_ASSERT(0);
		return 1;
	}

	m_hfile = CreateFile( srcpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		_ASSERT( 0 );
		return 1;
	}	

	CallF(SetBuffer(), return 1);

	int posstep = 0;
	XMLIOBUF fileinfobuf;
	ZeroMemory( &fileinfobuf, sizeof( XMLIOBUF ) );
	CallF( SetXmlIOBuf( &m_xmliobuf, "<FileInfo>", "</FileInfo>", &fileinfobuf ), return 1 );
	CallF( ReadFileInfo( &fileinfobuf ), return 1 );

	m_xmliobuf.pos = 0;
	XMLIOBUF jointbuf;
	ZeroMemory( &jointbuf, sizeof( XMLIOBUF ) );
	int result;
	result = SetXmlIOBuf( &m_xmliobuf, "<IniFileBody>", "</IniFileBody>", &jointbuf );
	if (result == 0) {
		CallF(ReadIniInfo(&jointbuf), return 1);
	}

	return 0;
}

/***
int CIniFile::CheckFileVersion( XMLIOBUF* xmlbuf )
{
	char kind[256];
	char version[256];
	char type[256];
	ZeroMemory( kind, sizeof( char ) * 256 );
	ZeroMemory( version, sizeof( char ) * 256 );
	ZeroMemory( type, sizeof( char ) * 256 );

	CallF( Read_Str( xmlbuf, "<kind>", "</kind>", kind, 256 ), return 1 );
	CallF( Read_Str( xmlbuf, "<version>", "</version>", version, 256 ), return 1 );
	CallF( Read_Str( xmlbuf, "<type>", "</type>", type, 256 ), return 1 );

	int cmpkind, cmpversion, cmptype;
	cmpkind = strcmp( kind, "OpenRDBProjectFile" );
	cmpversion = strcmp( version, "1001" );
	cmptype = strcmp( type, "0" );

	if( (cmpkind == 0) && (cmpversion == 0) && (cmptype == 0) ){
		return 0;
	}else{
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}
***/
int CIniFile::ReadFileInfo( XMLIOBUF* xmlbuf )
{

	return 0;
}
int CIniFile::ReadIniInfo(XMLIOBUF* xmlbuf )
{
	int clearcolorindex = 0;
	int usephysik = 0;
	int prepcntonphysik = 30;

	int result1 = 0;
	int result2 = 0;
	int result3 = 0;
	result1 = Read_Int( xmlbuf, "<ClearColorIndex>", "</ClearColorIndex>", &clearcolorindex );
	if (result1 == 0) {
		if ((clearcolorindex >= 0) && (clearcolorindex < BGCOL_MAX)) {
			g_ClearColorIndex = clearcolorindex;
		}
		else {
			g_ClearColorIndex = 0;
		}
	}
	else {
		g_ClearColorIndex = 0;
	}

	result2 = Read_Int(xmlbuf, "<UsePhysIK>", "</UsePhysIK>", &usephysik);
	if (result2 == 0) {
		if ((usephysik >= 0) && (usephysik <= 1)) {
			g_usephysik = usephysik;
		}
		else {
			g_usephysik = 0;
		}
	}
	else {
		g_usephysik = 0;
	}


	result3 = Read_Int( xmlbuf, "<PrepCntOnPhysIK>", "</PrepCntOnPhysIK>", &prepcntonphysik );
	if (result3 == 0) {
		if ((prepcntonphysik >= 0) && (prepcntonphysik <= 100)) {
			g_prepcntonphysik = prepcntonphysik;
		}
		else {
			g_prepcntonphysik = 30;
		}
	}
	else {
		g_prepcntonphysik = 30;
	}


	return 0;
}
