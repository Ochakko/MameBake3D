#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include <crtdbg.h>

#define DBGH
#include <dbg.h>

#include "MBPlugin.h"
//#include "MB2Version.h"
#define MB2VERSION 1005

#include <math.h>

//////////////////////////////////
// extern
//////////////////////////////////


//////////////////////////////////


MBPLUGIN_EXPORT int MBGetVersion( int* verptr )
{
	*verptr = MB2VERSION;//MB2Version.h !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	return 0;
}

MBPLUGIN_EXPORT int MBDbgOut( WCHAR* lpFormat, ... )
{

	int ret;
	va_list Marker;
	WCHAR outchar[4098];
			
	ZeroMemory( outchar, 4098 );

	va_start( Marker, lpFormat );
	ret = vswprintf_s( outchar, 4098, lpFormat, Marker );
	va_end( Marker );

	if( ret < 0 )
		return 1;

	ret = DbgOut( outchar );
	_ASSERT( !ret );

	return ret;
}

