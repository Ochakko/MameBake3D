#ifndef FBXFILEH
#define FBXFILEH

#include <fbxsdk.h>
#include <stdio.h>

class CBVHFile;

class CModel;

#ifdef FBXFILECPP
	int InitializeSdkObjects();
	int DestroySdkObjects();
	int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );
	int BVH2FBXFile( FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename );
#else
	extern int InitializeSdkObjects();
	extern int DestroySdkObjects();
	extern int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );
	extern int BVH2FBXFile( FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename );
#endif

#endif

