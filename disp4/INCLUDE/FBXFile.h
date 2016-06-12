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
 * @breaf FBXSDK�̃I�u�W�F�N�g�̍쐬�Ə������B
 * @return ����������O�B
 */
	int InitializeSdkObjects();

/**
 * @fn
 * DestroySdkObjects
 * @breaf FBXSDK�̃I�u�W�F�N�g�̊J���B
 * @return ����������O�B
 */
	int DestroySdkObjects();
	
/**
 * @fn
 * WriteFBXFile
 * @breaf FBX�t�@�C���������o���B
 * @param (FbxManager* psdk) IN FBXSDK�̃}�l�[�W���I�u�W�F�N�g�B
 * @param (CModel* pmodel) IN �����o�����f���̃f�[�^�B
 * @param (char* pfilename) IN �����o��FBX�t�@�C�����B
 * @return ����������O�B
 * @detail �{�[���e���x��A�j���[�V�����������o���B
 */	
	int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );

/**
 * @fn
 * BVH2FBXFile
 * @breaf BVH�̃A�j���[�V������FBX�t�@�C���ɏ����o���B
 * @param (FbxManager* psdk) IN FBXSDK�̃}�l�[�W���I�u�W�F�N�g�B
 * @param (CBVHFile* pbvhfile) IN �����o��BVH�̃f�[�^�B
 * @param (char* pfilename) IN �����o��FBX�t�@�C�����B
 * @return ����������O�B
 */	
	int BVH2FBXFile(FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename);

#else
	extern int InitializeSdkObjects();
	extern int DestroySdkObjects();
	extern int WriteFBXFile( FbxManager* psdk, CModel* pmodel, char* pfilename );
	extern int BVH2FBXFile(FbxManager* psdk, CBVHFile* pbvhfile, char* pfilename);
#endif

#endif

