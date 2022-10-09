#ifndef UNDOSPRITEH
#define UNDOSPRITEH

#include <windows.h>

#include <ChaVecCalc.h>

class CMySprite;

class CUndoSprite
{
public:
	CUndoSprite();
	~CUndoSprite();

	int CreateSprites(ID3D11Device* pdev, ID3D11DeviceContext* pd3dImmediateContext, WCHAR* mpath);
	void DestroySprites();
	int SetParams(int srcmainwidth, int srcmainheight);
	int Render(ID3D11DeviceContext* pd3dImmediateContext, int undoR, int undoW);

private:
	void InitParams();
	void DestroyObjs();

private:
	bool m_createdflag;
	CMySprite* m_sprite[12];
	int m_indexR[4];
	int m_indexW[4];
	POINT m_pointR[4];//dispcenter
	POINT m_pointW[4];//dispcenter
	ChaVector3 m_posR[4];
	ChaVector3 m_posW[4];
	ChaVector2 m_size;
};



#endif