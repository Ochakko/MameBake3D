#ifndef ENGNAMEH
#define ENGNAMEH

enum {
	ENGNAME_DISP,
	ENGNAME_BONE,
	ENGNAME_MOTION,
	ENGNAME_MAX
};

#ifdef ENGNAMECPP
	int ConvEngName( int type, char* srcname, int srcleng, char* dstname, int dstleng );
#else
	extern int ConvEngName( int type, char* srcname, int srcleng, char* dstname, int dstleng );
#endif

#endif