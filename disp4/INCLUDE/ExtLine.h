#ifndef EXTLINEH
#define EXTLINEH

#include <coef.h>
#include <D3DX9.h>

class CMQOFace;

class CExtLine
{
public:
	CExtLine();
	~CExtLine();

	int CreateExtLine( int pointnum, int facenum, D3DXVECTOR3* pointptr, CMQOFace* faceptr, D3DXVECTOR4 srccol );
	int CalcBound();

private:
	int InitParams();
	int DestroyObjs();

	int CreateBuffer( EXTLINEV* lineptr, int arrayleng, int* setnum );

public:
	int m_linenum;
	EXTLINEV* m_linev;
	D3DXVECTOR4 m_color;
	MODELBOUND	m_bound;

private:
	int m_pointnum;
	int m_facenum;	 
	D3DXVECTOR3* m_pointptr;
	CMQOFace* m_faceptr;
};


#endif