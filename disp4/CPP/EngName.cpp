#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#define ENGNAMECPP
#include <EngName.h>

#define DBGH
#include <dbg.h>


int ConvEngName( int type, char* srcname, int srcleng, char* dstname, int dstleng )
{
	static char headname[256] = "ReplacedName";
	static int replaceno = 0;

	char namepm[256];
	char tempname[256];
	char tempprinta[1024];
	WCHAR wtempprinta[1024];

	//文字列長は２５６を想定。
	if( (srcleng > 256) || (dstleng < 256) ){
		_ASSERT( 0 );
		return 1;
	}

	strcpy_s( namepm, 256, srcname );


	char* lptr;
	lptr = strstr( namepm, "[L]" );
	if( lptr ){
		strncpy( lptr, "_L_ ", 3 );
	}
	char* rptr;
	rptr = strstr( namepm, "[R]" );
	if( rptr ){
		strncpy( rptr, "_R_", 3 );
	}


	int namelen = (int)strlen( srcname );

	int nameno;
	int findilleagal = 0;
	int alnum;
	char curc;
	for( nameno = 0; nameno < namelen; nameno++ ){
		curc = namepm[nameno];
		if( ((curc >= 'a') && (curc <= 'z')) || ((curc >= 'A') && (curc <= 'Z')) || ((curc >= '0') && (curc <= '9')) || 
			(curc == '_') ){
			alnum = 1;
		}else{
			alnum = 0;
		}

		if( alnum == 0 ){
			findilleagal = 1;
			break;
		}
	}

	if( findilleagal != 0 ){
		if( type == ENGNAME_BONE ){
			sprintf_s( tempname, 256, "%s%d_Joint", headname, replaceno );
		}else if( type == ENGNAME_DISP ){
			sprintf_s( tempname, 256, "%s%d", headname, replaceno );
		}else if( type == ENGNAME_MOTION ){
			sprintf_s( tempname, 256, "Motion_%d", replaceno );
		}else{
			_ASSERT( 0 );
			return 1;
		}
			
		sprintf_s( tempprinta, 1024, "Xファイル用パーツ名変更\r\n\t%s ---> %s\r\n", srcname, tempname );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, tempprinta, 1024, wtempprinta, 1024 );
		DbgOut( wtempprinta );

		strcpy_s( dstname, dstleng, tempname );
		replaceno++;

	}else{
		if( type == ENGNAME_BONE ){
			sprintf_s( dstname, dstleng, "%s_Joint", namepm );
		}else{
			strcpy_s( dstname, dstleng, namepm );
		}
	}

	return 0;
}
