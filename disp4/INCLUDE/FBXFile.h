#ifndef FBXFILEH
#define FBXFILEH

#include <fbxsdk.h>
#include <stdio.h>

class CBVHFile;

class CModel;

#ifdef FBXFILECPP

/**
 * @fn
 * InitializeSdkObjects
 * @breaf FBXSDKのオブジェクトの作成と初期化。
 * @return 成功したら０。
 */
	int InitializeSdkObjects();

/**
 * @fn
 * DestroySdkObjects
 * @breaf FBXSDKのオブジェクトの開放。
 * @return 成功したら０。
 */
	int DestroySdkObjects();
	
/**
 * @fn
 * WriteFBXFile
 * @breaf FBXファイルを書き出す。
 * @param (FbxManager* psdk) IN FBXSDKのマネージャオブジェクト。
 * @param (CModel* pmodel) IN 書き出すモデルのデータ。
 * @param (char* pfilename) IN 書き出すFBXファイル名。
 * @return 成功したら０。
 * @detail ボーン影響度やアニメーションも書き出す。
 */	
	int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );

/**
 * @fn
 * BVH2FBXFile
 * @breaf BVHのアニメーションをFBXファイルに書き出す。
 * @param (FbxManager* psdk) IN FBXSDKのマネージャオブジェクト。
 * @param (CBVHFile* pbvhfile) IN 書き出すBVHのデータ。
 * @param (char* pfilename) IN 書き出すFBXファイル名。
 * @return 成功したら０。
 */	
	int BVH2FBXFile(FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename);

#else
	extern int InitializeSdkObjects();
	extern int DestroySdkObjects();
	extern int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );
	extern int BVH2FBXFile(FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename);
#endif

#endif

