#ifndef NODEONLOADH
#define NODEONLOADH

#include <fbxsdk.h>

#include <map>
#include <string>

#include <Coef.h>

#include <ChaVecCalc.h>

enum {
	NOL_NONE,
	NOL_NULL,
	NOL_SKELETON,
	NOL_MESH,
	NOL_OTHER,
	NOL_MAX
};

class CBone;
class CMQOObject;

class CNodeOnLoad
{
public:
	CNodeOnLoad(FbxNode* srcnode);
	~CNodeOnLoad();

	void InitParams();
	void DestroyObjes();


	void SetType(int srctype) 
	{
		m_type = srctype;
	}
	int GetType()
	{
		return m_type;
	}
	void SetNode(FbxNode* srcnode)
	{
		m_pnode = srcnode;
	}
	FbxNode* GetNode()
	{
		return m_pnode;
	}
	void SetBone(CBone* srcbone)
	{
		m_bone = srcbone;
	}
	CBone* GetBone()
	{
		return m_bone;
	}
	void SetMqoObject(CMQOObject* srcobj)
	{
		m_mqoobject = srcobj;
	}
	CMQOObject* GetMqoObject()
	{
		return m_mqoobject;
	}
private:
	int m_type;
	FbxNode* m_pnode;
	CBone* m_bone;
	CMQOObject* m_mqoobject;

	//chain‚à•K—v

};


#endif