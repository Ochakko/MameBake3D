#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#include <BntFile.h>

#include <Model.h>
#include <MQOObject.h>
#include <MQOMaterial.h>
#include <polymesh4.h>
#include <Bone.h>
#include <MotionPoint.h>



//#include "..\\BTMANAGER\\BPWorld.h"
//#include <ImpFile.h>
//#include <GColiFile.h>
//#include <RigidElemFile.h>

#define DBGH
#include <dbg.h>

using namespace std;

#define BNTMAGICNO	2398475


CBntFile::CBntFile()
{
	InitParams();
}

CBntFile::~CBntFile()
{
	DestroyObjs();
}

int CBntFile::InitParams()
{
	CXMLIO::InitParams();

	m_me.modelptr = 0;
	m_me.tlarray.clear();
	m_me.motmenuindex = 0;
	m_me.lineno2boneno.clear();
	m_me.boneno2lineno.clear();

	return 0;
}
int CBntFile::DestroyObjs()
{
	CXMLIO::DestroyObjs();

	InitParams();

	return 0;
}

int CBntFile::WriteBntFile( WCHAR* wfilename, MODELELEM wme )
{
	m_me = wme;
	m_mode = XMLIO_WRITE;

	if( !m_me.modelptr ){
		_ASSERT( 0 );
		return 1;
	}

	m_hfile = CreateFile( wfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		DbgOut( L"BntFile : WriteChaFile : file open error !!!\n" );
		_ASSERT( 0 );
		return 1;
	}

	CallF( WriteHeader(), return 1 );
	CallF( WriteBone( wme.modelptr ), return 1 );
	CallF( WriteObject( wme.modelptr ), return 1 );
	CallF( WriteOneMotion( wme.modelptr ), return 1 );

	return 0;
}

int CBntFile::WriteHeader()
{
	CallF( WriteNumber( (int)BNTMAGICNO ), return 1 );
	return 0;
}
	
int CBntFile::WriteNumber( int srcnumber )
{
	int wnum = srcnumber;

	CallF( WriteVoid2File( (void*)&srcnumber, sizeof( int ) ), return 1 );
	return 0;
}

int CBntFile::WriteBone( CModel* srcmodel )
{
	int bonenum = srcmodel->GetBoneListSize();

	CallF( WriteNumber( bonenum ), return 1 );

	int wbonenum = 0;

	if( srcmodel->GetTopBone() ){
		WriteBntBoneReq( srcmodel->GetTopBone(), &wbonenum );
	}

	if( wbonenum != bonenum ){
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}

void CBntFile::WriteBntBoneReq( CBone* srcbone, int* pbonenum )
{
/*
typedef struct tag_bntbone
{
	int bonecnt;
	int validflag;
	int boneno;
	int topboneflag;
	char bonename[256];

	char parentname[256];
	char childname[256];
	char brothername[256];
}BNTBONE;
*/
	BNTBONE wdata;
	ZeroMemory( &wdata, sizeof( BNTBONE ) );
	wdata.bonecnt = srcbone->GetBoneCnt();
	wdata.validflag = srcbone->GetValidFlag();
	wdata.boneno = srcbone->GetBoneNo();
	wdata.topboneflag = srcbone->GetTopBoneFlag();
	strcpy_s( wdata.bonename, 256, srcbone->GetBoneName() );

	if( srcbone->GetParent() ){
		strcpy_s( wdata.parentname, 256, srcbone->GetParent()->GetBoneName() );
	}
	if( srcbone->GetChild() ){
		strcpy_s( wdata.childname, 256, srcbone->GetChild()->GetBoneName() );
	}
	if( srcbone->GetBrother() ){
		strcpy_s( wdata.brothername, 256, srcbone->GetBrother()->GetBoneName() );
	}
	
	CallF( WriteVoid2File( (void*)&wdata, sizeof( BNTBONE ) ), return );


	(*pbonenum)++;

/////
	if( srcbone->GetChild() ){
		WriteBntBoneReq( srcbone->GetChild(), pbonenum );
	}

	if( srcbone->GetBrother() ){
		WriteBntBoneReq( srcbone->GetBrother(), pbonenum );
	}

	return;
}

int CBntFile::WriteObject( CModel* srcmodel )
{
/*
typedef struct tag_bntobjinfo
{
	char objname[256];
	int vertexnum;
	int facenum;
	BNTMATERIAL bntmaterial;
}BNTOBJINFO;

typedef struct tag_bntvertex
{
	D3DXVECTOR4		pos;
	ChaVector3		normal;
	D3DXVECTOR2		uv;
	float weight[4];
	float boneindex[4];
}BNTVERTEX;
*/

	CallF( WriteNumber( srcmodel->GetMqoObjectSize() ), return 1 );

	map<int, CMQOObject*>::iterator itrobj;
	for( itrobj = srcmodel->GetMqoObjectBegin(); itrobj != srcmodel->GetMqoObjectEnd(); itrobj++ ){
		CMQOObject* curobj = itrobj->second;
		if( !curobj ){
			_ASSERT( 0 );
			return 1;
		}
		CallF( WriteBntObjInfo( curobj ), return 1 );

		CPolyMesh4* curpm4 = curobj->GetPm4();
		if( !curpm4 ){
			_ASSERT( 0 );
			return 1;
		}
//vertex
		int curvnum = curpm4->GetOptLeng();
		PM3DISPV* curdispv = curpm4->GetPm3Disp();
		PM3INF* curinf = curpm4->GetPm3Inf();
		int vno;
		for( vno = 0; vno < curvnum; vno++ ){
			CallF( WriteBntVertex( curdispv + vno, curinf + vno ), return 1 );
		}
//index
		int curinum = curpm4->GetFaceNum() * 3;
		int* curindex = curpm4->GetDispIndex();
		if( !curindex ){
			_ASSERT( 0 );
			return 1;
		}
		CallF( WriteBntIndex( curindex, curinum ), return 1 );
//cluster
		int ctnum = curobj->GetClusterSize();
		CallF( WriteNumber( ctnum ), return 1 );

		int ctno;
		for( ctno = 0; ctno < ctnum; ctno++ ){
			CBone* curctb = curobj->GetCluster( ctno );
			if( !curctb ){
				_ASSERT( 0 );
				return 1;
			}
			CallF( WriteBntCluster( curctb ), return 1 );
		}
	}

	return 0;
}

int CBntFile::WriteBntObjInfo( CMQOObject* srcobj )
{
/*
typedef struct tag_bntmaterial
{
	int materialno;
	char name[256];

	D3DXVECTOR4 dif4f;
	ChaVector3 amb3f;
	ChaVector3 emi3f;
	ChaVector3 spc3f;

	char tex[256];
}BNTMATERIAL;
*/
	CMQOMaterial* curmate = 0;
	map<int, CMQOMaterial*>::iterator itrmate = srcobj->GetMaterialBegin();
	if( itrmate != srcobj->GetMaterialEnd() ){
		curmate = itrmate->second;
	}else{
		_ASSERT( 0 );
		return 1;
	}
	if( !curmate ){
		_ASSERT( 0 );
		return 1;
	}
	BNTMATERIAL wmaterial;
	ZeroMemory( &wmaterial, sizeof( BNTMATERIAL ) );
	wmaterial.materialno = curmate->GetMaterialNo();
	strcpy_s( wmaterial.name, 256, curmate->GetName() );
	wmaterial.dif4f = curmate->GetDif4F();
	wmaterial.amb3f = curmate->GetAmb3F();
	wmaterial.emi3f = curmate->GetEmi3F();
	wmaterial.spc3f = curmate->GetSpc3F();
	if( *(curmate->GetTex()) != 0 ){
		strcpy_s( wmaterial.tex, 256, curmate->GetTex() );
	}

/*
typedef struct tag_bntobjinfo
{
	char objname[256];
	int vertexnum;
	int facenum;
	BNTMATERIAL bntmaterial;
}BNTOBJINFO;
*/

	BNTOBJINFO woi;
	ZeroMemory( &woi, sizeof( BNTOBJINFO ) );
	strcpy_s( woi.objname, 256, srcobj->GetName() );

	CPolyMesh4* curpm4 = srcobj->GetPm4();
	if( !curpm4 ){
		_ASSERT( 0 );
		return 1;
	}
//		pmvleng = m_pm4->m_optleng;
//		pmfleng = m_pm4->m_facenum;
	woi.vertexnum = curpm4->GetOptLeng();
	woi.facenum = curpm4->GetFaceNum();
	MoveMemory( &woi.bntmaterial, &wmaterial, sizeof( BNTMATERIAL ) );

	if( woi.bntmaterial.tex[0] == 0xFE ){
		_ASSERT( 0 );
	}


	CallF( WriteVoid2File( (void*)&woi, sizeof( BNTOBJINFO ) ), return 1 );

	return 0;
}


int CBntFile::WriteBntCluster( CBone* srcbone )
{
/*
typedef struct tag_bntcluster
{
	char bonename[256];
	int boneno;
}BNTCLUSTER;
*/
	BNTCLUSTER wct;
	ZeroMemory( &wct, sizeof( BNTCLUSTER ) );

	strcpy_s( wct.bonename, 256, srcbone->GetBoneName() );
	wct.boneno = srcbone->GetBoneNo();

	CallF( WriteVoid2File( (void*)&wct, sizeof( BNTCLUSTER ) ), return 1 );

	return 0;
}

int CBntFile::WriteBntVertex( PM3DISPV* srcpm3dispv, PM3INF* srcpm3inf )
{
/*
typedef struct tag_bntvertex
{
	PM3DISPV dispv;
	PM3INF inf;
}BNTVERTEX;
*/
	BNTVERTEX wv;
	ZeroMemory( &wv, sizeof( BNTVERTEX ) );

	wv.dispv = *srcpm3dispv;
	wv.inf = *srcpm3inf;

	CallF( WriteVoid2File( (void*)&wv, sizeof( BNTVERTEX ) ), return 1 );

	return 0;
}

int CBntFile::WriteBntIndex( int* srcindex, int indexnum )
{
	CallF( WriteVoid2File( (void*)srcindex, sizeof( int ) * indexnum ), return 1 );

	return 0;
}


int CBntFile::WriteOneMotion( CModel* srcmodel )
{
	if(	!srcmodel->GetCurMotInfo() ){
		_ASSERT( 0 );
		return 1;
	}


	int motionnum = 1;
	CallF( WriteVoid2File( (void*)&motionnum, sizeof( int ) ), return 1 );

	BNTMOTHEADER motheader;
	ZeroMemory( &motheader, sizeof( BNTMOTHEADER ) );
	strcpy_s( motheader.motname, 256, srcmodel->GetCurMotInfo()->motname );
	motheader.frameleng = (int)srcmodel->GetCurMotInfo()->frameleng;
	motheader.bonenum = srcmodel->GetBoneListSize();
	CallF( WriteVoid2File( (void*)&motheader, sizeof( BNTMOTHEADER ) ), return 1 );

	int bonecnt = 0;
	WriteMotionPointsReq( srcmodel->GetTopBone(), srcmodel->GetCurMotInfo()->motid, motheader.frameleng, &bonecnt );
	if( bonecnt != motheader.bonenum ){
		_ASSERT( 0 );
		return 1;
	}

	return 0;
}

void CBntFile::WriteMotionPointsReq( CBone* srcbone, int srcmotid, int frameleng, int* pcnt )
{
	if( !srcbone ){
		_ASSERT( 0 );
		return;
	}

	CallF( WriteVoid2File( (void*)srcbone->GetBoneName(), sizeof( char ) * 256 ), return );

	int frameno;
	for( frameno = 0; frameno < frameleng; frameno++ ){
		BNTMOTPOINT wmp;
		wmp.frameno = frameno;
		
		CMotionPoint curmp;
		int existflag = 0;
		CallF( srcbone->CalcFBXMotion( srcmotid, (double)frameno, &curmp, &existflag ), return );
		wmp.matrix = curmp.GetWorldMat();

		CallF( WriteVoid2File( (void*)&wmp, sizeof( BNTMOTPOINT ) ), return );
	}


	(*pcnt)++;

	if( srcbone->GetChild() ){
		WriteMotionPointsReq( srcbone->GetChild(), srcmotid, frameleng, pcnt );
	}
	if( srcbone->GetBrother() ){
		WriteMotionPointsReq( srcbone->GetBrother(), srcmotid, frameleng, pcnt );
	}

}



