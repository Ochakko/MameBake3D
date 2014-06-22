#ifndef FBXBONE0H
#define FBXBONE0H

#include <coef.h>

#include <D3DX9.h>
#include <crtdbg.h>
#include <fbxsdk.h>
#include <quaternion.h>

class CBone;
class CBVHElem;

class CFBXBone
{
public:
	CFBXBone();
	~CFBXBone();

	int AddChild( CFBXBone* childptr );

private:
	int InitParams();
	int DestroyObjs();

public:
	int type;
	CBone* bone;
	CBVHElem* pbe;
	FbxNode* skelnode;
	int bunkinum;

	CQuaternion axisq;

	CFBXBone* m_parent;
	CFBXBone* m_child;
	CFBXBone* m_brother;
};


#endif