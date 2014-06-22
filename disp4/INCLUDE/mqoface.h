#ifndef MQOFACEH
#define MQOFACEH

#include <stdio.h>
#include <stdarg.h>

#include <coef.h>
//#include <basedat.h>

#define DBGH
#include <dbg.h>


class CMQOFace
{
public:
	CMQOFace();
	~CMQOFace();

	int SetParams( char* srcchar, int srcleng );

	int Dump();

	int CheckSameLine( CMQOFace* chkface, int* findflag );
	int SetInvFace( CMQOFace* srcface, int offset );

	int SetMikoBoneName( char* srcname, int lrflag );
	int CheckLRFlag( D3DXVECTOR3* pointptr, int* lrflagptr );

private:
	void InitParams();

	int GetInt( int* dstint, char* srcchar, int pos, int srcleng, int* stepnum );
	int GetI64( __int64* dsti64, char* srcchar, int pos, int srcleng, int* stepnum );
	int GetFloat( float* dstfloat, char* srcchar, int pos, int srcleng, int* stepnum );

public:
	int m_faceno;
	int m_pointnum;
	int m_index[4];
	int m_materialno;
	
	int m_hasuv;
	D3DXVECTOR2 m_uv[4];

	__int64 m_col[4];
	int m_vcolsetflag;


//	CMQOFace* next;

// for mikoto bone
	int m_bonetype; //MIKOBONE_NONE, MIKOBONE_NORMAL, MIKOBONE_FLOAT, MIKOBONE_ILLEAGAL
	int m_parentindex;
	int m_childindex;
	int m_hindex;

	char m_bonename[256];
	CMQOFace* m_parent;
	CMQOFace* m_child;
	CMQOFace* m_brother;

	int m_dirtyflag;
	int m_serialno;

	int m_shapeno;
};

#endif
