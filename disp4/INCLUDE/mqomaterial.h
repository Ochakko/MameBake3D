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

	int IsSame( CMQOMaterial* compmat, int compnameflag, int cmplevel = 0 );//compnameflag == 0 の時は、名前は比較しない。

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
	//accesser
	int GetMaterialNo(){
		return m_materialno;
	};
	void SetMaterialNo( int srcval ){
		m_materialno = srcval;
	};

	const char* GetName(){
		return m_name;
	};
	void SetName( char* srcname ){
		strcpy_s( m_name, 256, srcname );
	};

	D3DXVECTOR4 GetCol(){
		return m_col;
	};
	void SetCol( D3DXVECTOR4 srcval ){
		m_col = srcval;
	};

	float GetDif(){
		return m_dif;
	};
	void SetDif( float srcval ){
		m_dif = srcval;
	};

	float GetAmb(){
		return m_amb;
	};
	void SetAmb( float srcval ){
		m_amb = srcval;
	};

	float GetEmi(){
		return m_emi;
	};
	void SetEmi( float srcval ){
		m_emi = srcval;
	};

	float GetSpc(){
		return m_spc;
	};
	void SetSpc( float srcval ){
		m_spc = srcval;
	};

	float GetPower(){
		return m_power;
	};
	void SetPower( float srcval ){
		m_power = srcval;
	};

	int GetTexNum(){
		return m_texnum;
	};
	void SetTexNum( int srcval ){
		m_texnum = srcval;
	};

	const char* GetTex(){
		return m_tex;
	};
	void SetTex( char* srcname ){
		strcpy_s( m_tex, 256, srcname );
	};
	void Add2Tex( char* srcname ){
		strcat_s( m_tex, 256, srcname );
	};

	const char* GetAlpha(){
		return m_alpha;
	};
	void SetAlpha( char* srcname ){
		strcpy_s( m_alpha, 256, srcname );
	};

	const char* GetBump(){
		return m_bump;
	};
	void SetBump( char* srcname ){
		strcpy_s( m_bump, 256, srcname );
	};

	int GetVcolFlag(){
		return m_vcolflag;
	};
	void SetVcolFlag( int srcval ){
		m_vcolflag = srcval;
	};

	int GetShader(){
		return m_shader;
	};
	void SetShader( int srcval ){
		m_shader = srcval;
	};


	D3DXVECTOR4 GetDif4F(){
		return m_dif4f;
	};
	void SetDif4F( D3DXVECTOR4 srcval ){
		m_dif4f = srcval;
	};
	void SetDif4FW( float srcval ){
		m_dif4f.w = srcval;
	};

	D3DXVECTOR3 GetAmb3F(){
		return m_amb3f;
	};
	void SetAmb3F( D3DXVECTOR3 srcval ){
		m_amb3f = srcval;
	};

	D3DXVECTOR3 GetEmi3F(){
		return m_emi3f;
	};
	void SetEmi3F( D3DXVECTOR3 srcval ){
		m_emi3f = srcval;
	};

	D3DXVECTOR3 GetSpc3F(){
		return m_spc3f;
	};
	void SetSpc3F( D3DXVECTOR3 srcval ){
		m_spc3f = srcval;
	};

	D3DXVECTOR4 GetSceneAmb(){
		return m_sceneamb;
	};
	void SetSceneAmb( D3DXVECTOR4 srcval ){
		m_sceneamb = srcval;
	};
	
	int GetTransparent(){
		return m_transparent;
	};
	void SetTransparent( int srcval ){
		m_transparent = srcval;
	};

	int GetTexRule(){
		return m_texrule;
	};
	void SetTexRule( int srcval ){
		m_texrule = srcval;
	};

	int GetBlendMode(){
		return m_blendmode;
	};
	void SetBlendMode( int srcval ){
		m_blendmode = srcval;
	};

	float GetUAnime(){
		return m_uanime;
	};
	void SetUAnime( float srcval ){
		m_uanime = srcval;
	};

	float GetVAnime(){
		return m_vanime;
	};
	void SetVAnime( float srcval ){
		m_vanime = srcval;
	};

	const char* GetCurTexName(){
		return m_curtexname;
	};

	int GetTexID(){
		return m_texid;
	};
	void SetTexID( int srcval ){
		m_texid = srcval;
	};

private:
	int m_materialno;
	char m_name[256];

	D3DXVECTOR4 m_col;
	float m_dif;
	float m_amb;
	float m_emi;
	float m_spc;
	float m_power;
	int m_texnum;
	char m_tex[256];
	char m_alpha[256];
	char m_bump[256];

	int m_vcolflag;
////

	int m_shader;

////
	D3DXVECTOR4 m_dif4f;
	D3DXVECTOR3 m_amb3f;
	D3DXVECTOR3 m_emi3f;
	D3DXVECTOR3 m_spc3f;
	D3DXVECTOR4 m_sceneamb;

	int m_transparent;
	int m_texrule;
	int m_blendmode;

	float m_uanime;
	float m_vanime;

	int m_texid;

//以下、クラス外からアクセスしないのでアクセッサー無し。
	char* m_curtexname;

	char m_alphatest;
	unsigned char m_alphaval;

	float m_glowmult[3];
	float m_orgalpha;


	int m_convnamenum;
	char** m_ppconvname;

};


#endif