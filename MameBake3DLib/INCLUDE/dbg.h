// platform sdk \ d3dim \ include ‚ÖˆÚ“®
#include <stdio.h>

#include <windows.h>
#include <wchar.h>
#include <coef.h>

#ifndef CallF
	#define WIDEN2(x) L ## x
	#define WIDEN(x) WIDEN2(x)
	#define __WFILE__ WIDEN(__FILE__)

	#define CallF(cfunc,f) if(cfunc){ DbgOut( L"file %s, line %d\r\n", __WFILE__, __LINE__ ); _ASSERT( 0 ); f; }
#endif

#ifndef DBGH
#define	DBGH
	HANDLE dbgfile = 0;
	HANDLE infofile = 0;

	int DbgOut( WCHAR* lpFormat, ... );

	int DbgOutB( void* lpvoid, int datsize );
	int DbgWrite2File( char* lpFormat, ... );
	int SetTabChar( int tabnum );		
	void ErrorMessage(char* szMessage, HRESULT hr);
	int OpenDbgFile();
	int CloseDbgFile();

	void OutputToInfoWnd(WCHAR* lpFormat, ...);

#else
	extern HANDLE dbgfile;
	extern HANDLE infofile;

	extern int DbgOut( WCHAR* lpFormat, ... );
	extern int DbgOutB( void* lpvoid, int datsize );
	extern int DbgWrite2File( char* lpFormat, ... );
	extern int SetTabChar( int tabnum );
	extern void ErrorMessage(char* szMessage, HRESULT hr);
	extern int OpenDbgFile();
	extern int CloseDbgFile();
	extern void OutputToInfoWnd(WCHAR* lpFormat, ...);
#endif

