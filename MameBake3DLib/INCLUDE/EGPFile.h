#ifndef EGPFILEH
#define EGPFILEH

#include <fbxsdk.h>
#include <stdio.h>

class CBVHFile;

class CModel;

#ifdef EGPFILECPP
	int WriteEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno);
	bool LoadEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno);
#else
	extern int WriteEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno);
	extern bool LoadEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno);
#endif

#endif

