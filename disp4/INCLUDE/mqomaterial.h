#ifndef MQOMATERIALH
#define MQOMATERIALH

#include <stdio.h>
#include <stdarg.h>

#include <d3dx9.h>

#include <coef.h>

#define DBGH
#include <dbg.h>

#include <wchar.h>

class CMQOMaterial
{
public:
	CMQOMaterial();
	~CMQOMaterial();

	int SetParams( int srcno, D3DXVECTOR4 srcsceneamb, char* srcchar, int srcleng );


	int SetName( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetCol( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetDif( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetAmb( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetEmi( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetSpc( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetPower( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetTex( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetAlpha( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetBump( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetShader( char* srcchar, int pos, int srcleng, int* stepnum );
	int SetVcolFlag( char* srcchar, int pos, int srcleng, int* stepnum );

	int GetFloat( float* dstfloat, char* srcchar, int pos, int srcleng, int* stepnum );
	int GetInt( int* dstint, char* srcchar, int pos, int srcleng, int* stepnum );


	int Dump();

	int IsSame( CMQOMaterial* compmat, int compnameflag, int cmplevel = 0 );//compnameflag == 0 ÇÃéûÇÕÅAñºëOÇÕî‰ärÇµÇ»Ç¢ÅB

	int GetName( int srcmatno, char** nameptr );
	int AddConvName( char** ppname );

	int SetCurTexname( int srcenabletex );

	int SetExtTex( int srctexnum, char** srctexname );
	int SetConvName( int srcconvnum, char** srcconvname );

	int GetColorrefDiffuse( COLORREF* dstdiffuse );
	int GetColorrefSpecular( COLORREF* dstspecular );
	int GetColorrefAmbient( COLORREF* dstambient );
	int GetColorrefEmissive( COLORREF* dstemissive );

	int CreateTexture( WCHAR* dirname, int texpool = D3DPOOL_DEFAULT );

private:
	int InitParams();
	int DestroyObjs();

	int ConvParamsTo3F();

public:
	int materialno;
	char name[256];

	D3DXVECTOR4 col;
	float dif;
	float amb;
	float emi;
	float spc;
	float power;
	int texnum;
	char tex[256];
	char alpha[256];
	char bump[256];

	int vcolflag;
////

	int shader;

////
	D3DXVECTOR4 dif4f;
	D3DXVECTOR3 amb3f;
	D3DXVECTOR3 emi3f;
	D3DXVECTOR3 spc3f;
	D3DXVECTOR4 sceneamb;

	int transparent;
	int texrule;
	int blendmode;

	float uanime;
	float vanime;

///
	char* curtexname;

	char alphatest;
	unsigned char alphaval;

	float glowmult[3];
	float orgalpha;

	int m_texid;

	int m_convnamenum;
	char** m_ppconvname;

};


#endif