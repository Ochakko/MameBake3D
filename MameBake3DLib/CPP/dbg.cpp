#include "stdafx.h"
//#include <stdafx.h> //É_É~Å[

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <windows.h>

#include <crtdbg.h>

#ifdef DBGH
#undef DBGH
#endif
#include <dbg.h>

int DbgOut( WCHAR* lpFormat, ... )
{
	if( !dbgfile ){
		return 0;
	}

	int ret;
	va_list Marker;
	unsigned long wleng, writeleng;
	WCHAR outchar[7000];
			
	ZeroMemory( outchar, sizeof( WCHAR ) * 7000 );

	va_start( Marker, lpFormat );
	ret = vswprintf_s( outchar, 7000, lpFormat, Marker );
	va_end( Marker );

	if( ret < 0 )
		return 1;

	wleng = (unsigned long)wcslen( outchar );
	WriteFile( dbgfile, outchar, sizeof( WCHAR ) * wleng, &writeleng, NULL );

	return 0;
	
}

int DbgOutB( void* lpvoid, int datsize )
{
	if( !dbgfile )
		return 0;

	unsigned long writeleng;

	WriteFile( dbgfile, lpvoid, datsize, &writeleng, NULL );
	if( datsize != (long)writeleng ){
		return 1;
	}

	return 0;

}

void ErrorMessage(WCHAR* szMessage, HRESULT hr)
{
    if(HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
		hr = HRESULT_CODE(hr);

	WCHAR* szError;
	if(FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&szError, 0, NULL) != 0)
	{
		DbgOut( L"ErrorMessage : %s: %s\r\n", szMessage, szError);

		MessageBox( NULL, szError, szMessage, MB_OK );

		LocalFree(szError);
	}else{
		DbgOut( L"ErrorMessage : Error number not found\r\n");
	}
}

int OpenDbgFile()
{
	if( dbgfile )
		return 1;

	dbgfile = CreateFile( L"dbg.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( dbgfile == INVALID_HANDLE_VALUE ){
		return 1;
	}

	SetEndOfFile( dbgfile );

	DbgOut( L"InitFunc: dbgfile created\r\n" );
		
	return 0;
}

int CloseDbgFile()
{
	if( dbgfile ){
		
		DbgOut( L"CloseDbgFile\r\n" );

		FlushFileBuffers( dbgfile );
		SetEndOfFile( dbgfile );
		CloseHandle( dbgfile );
		dbgfile = 0;
	}
	return 0;
}

int DbgWrite2File( char* lpFormat, ... )
{
	if( dbgfile == INVALID_HANDLE_VALUE ){
		return 0;
	}

	int ret;
	va_list Marker;
	unsigned long wleng, writeleng;
	char outchar[2048];
			
	ZeroMemory( outchar, 2048 );

	va_start( Marker, lpFormat );
	ret = vsprintf_s( outchar, 2048, lpFormat, Marker );
	va_end( Marker );

	if( ret < 0 )
		return 1;

	wleng = (unsigned long)strlen( outchar );
	WriteFile( dbgfile, outchar, wleng, &writeleng, NULL );
	if( wleng != writeleng ){
		return 1;
	}

	return 0;	
}
