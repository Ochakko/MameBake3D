#include "stdafx.h"
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

#include <RigFile.h>

#include <Model.h>
#include <Bone.h>
//#include <BoneProp.h>

#define DBGH
#include <dbg.h>
#include <map>
#include <vector>

using namespace std;



extern void InitCustomRig(CUSTOMRIG* dstcr, CBone* parentbone, int rigno);
extern int IsValidCustomRig(CModel* srcmodel, CUSTOMRIG srccr, CBone* parentbone);
//void SetCustomRigBone(CUSTOMRIG* dstcr, CBone* childbone);
extern int IsValidRigElem(CModel* srcmodel, RIGELEM srcrigelem);




CRigFile::CRigFile()
{
	InitParams();
}

CRigFile::~CRigFile()
{
	DestroyObjs();
}

int CRigFile::InitParams()
{
	CXMLIO::InitParams();

	m_model = 0;
	return 0;
}

int CRigFile::DestroyObjs()
{
	CXMLIO::DestroyObjs();

	InitParams();

	return 0;
}


int CRigFile::WriteRigFile( WCHAR* strpath, CModel* srcmodel )
{
	if (!srcmodel){
		_ASSERT(0);
		return 1;
	}

	m_model = srcmodel;
	m_mode = XMLIO_WRITE;
	if( !m_model->GetTopBone() ){
		return 0;
	}

	m_hfile = CreateFile( strpath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		DbgOut( L"RigFile : WriteRigFile : file open error !!!\n" );
		_ASSERT( 0 );
		return 1;
	}

	CallF( Write2File( "<?xml version=\"1.0\" encoding=\"Shift_JIS\"?>\r\n<RIGFILE>\r\n" ), return 1 );  
	CallF( Write2File( "    <FileInfo>1001-01</FileInfo>\r\n" ), return 1 );

	WriteRigReq( m_model->GetTopBone() );

	CallF( Write2File( "</RIGFILE>\r\n" ), return 1 );

	return 0;
}
void CRigFile::WriteRigReq( CBone* srcbone )
{
	if (!srcbone){
		return;
	}

	int rigno;
	for (rigno = 0; rigno < MAXRIGNUM; rigno++){
		m_customrig = srcbone->GetCustomRig(rigno);
		if (m_customrig.useflag == 2){
			int isvalid = IsValidCustomRig(m_model, m_customrig, srcbone);
			if (isvalid){
				WriteRig(srcbone);
			}
		}
	}

	if( srcbone->GetChild() ){
		WriteRigReq(srcbone->GetChild());
	}
	if( srcbone->GetBrother() ){
		WriteRigReq(srcbone->GetBrother());
	}
}


int CRigFile::WriteRig(CBone* srcbone)
{
/***
  <Bone>
    <RigName>RigName</RigName>
	<OwnerBone>koshi</OwnerBone>
	<RigNumber>0</RigNumber>
	<ElemNum>3</ElemNum>
    <RigElem>
      <RigRigName>Lte</RigRigName>
      <RigRigNo>0</RigRigNo>
	  <BoneName>koshi</BoneName>
	  <AxisU>X</AxisU>
	  <RateU>1.0000</RateU>
	  <EnableU>1</EnableU>
	  <AxisV>X</AxisV>
	  <RateV>1.0000</RateV>
	  <EnableV>1</EnableV>
	</RigElem>
	<RigElem>
	  <RigRigName>Rte</RigRigName>
	  <RigRigNo>0</RigRigNo>
	  <BoneName>mune</BoneName>
	  <AxisU>X</AxisU>
	  <RateU>1.0000</RateU>
	  <EnableU>1</EnableU>
	  <AxisV>X</AxisV>
	  <RateV>1.0000</RateV>
	  <EnableV>1</EnableV>
	</RigElem>
  </Bone>
***/

	CallF(Write2File("  <Bone>\r\n" ), return 1);

	char mrigname[256];
	WideCharToMultiByte(CP_ACP, 0, m_customrig.rigname, -1, mrigname, 256, NULL, NULL);
	CallF(Write2File("    <RigName>%s</RigName>\r\n", mrigname ), return 1);

	CBone* ownerbone = m_model->GetBoneByID(m_customrig.rigboneno);
	if (!ownerbone || (ownerbone != srcbone)){
		_ASSERT(0);
		return 1;
	}
	CallF(Write2File("    <OwnerBone>%s</OwnerBone>\r\n", ownerbone->GetBoneName()), return 1);

	CallF(Write2File("    <RigNumber>%d</RigNumber>\r\n", m_customrig.rigno), return 1);

	CallF(Write2File("    <ElemNum>%d</ElemNum>\r\n", m_customrig.elemnum), return 1);


	int elemno;
	for (elemno = 0; elemno < m_customrig.elemnum; elemno++){
		RIGELEM currigelem = m_customrig.rigelem[elemno];

		CallF(Write2File("    <RigElem>\r\n"), return 1);


		char mrigrigname[256] = "!noname!";
		if (currigelem.rigrigboneno >= 0){
			CBone* rigrigbone = m_model->GetBoneByID(currigelem.rigrigboneno);
			if (rigrigbone){
				strcpy_s(mrigrigname, 256, rigrigbone->GetBoneName());
			}
		}
		CallF(Write2File("    <RigRigName>%s</RigRigName>\r\n", mrigrigname), return 1);		
		CallF(Write2File("    <RigRigNo>%d</RigRigNo>\r\n", currigelem.rigrigno), return 1);

		CBone* rigelembone = m_model->GetBoneByID(currigelem.boneno);
		if (!rigelembone){
			_ASSERT(0);
			return 1;
		}
		CallF(Write2File("      <BoneName>%s</BoneName>\r\n", rigelembone->GetBoneName()), return 1);

		char straxis[3][10] = {"X", "Y", "Z"};
		int axiskind;

		axiskind = currigelem.transuv[0].axiskind;
		if ((axiskind < AXIS_X) || (axiskind > AXIS_Z)){
			_ASSERT(0);
			return 1;
		}
		CallF(Write2File("      <AxisU>%s</AxisU>\r\n", straxis[axiskind]), return 1);
		CallF(Write2File("      <RateU>%f</RateU>\r\n", currigelem.transuv[0].applyrate), return 1);
		CallF(Write2File("      <EnableU>%d</EnableU>\r\n", currigelem.transuv[0].enable), return 1);

		axiskind = currigelem.transuv[1].axiskind;
		if ((axiskind < AXIS_X) || (axiskind > AXIS_Z)){
			_ASSERT(0);
			return 1;
		}
		CallF(Write2File("      <AxisV>%s</AxisV>\r\n", straxis[axiskind]), return 1);
		CallF(Write2File("      <RateV>%f</RateV>\r\n", currigelem.transuv[1].applyrate), return 1);
		CallF(Write2File("      <EnableV>%d</EnableV>\r\n", currigelem.transuv[1].enable), return 1);

		CallF(Write2File("    </RigElem>\r\n"), return 1);
	}

	CallF(Write2File("  </Bone>\r\n"), return 1);

	return 0;
}




int CRigFile::LoadRigFile( WCHAR* strpath, CModel* srcmodel )
{
	m_model = srcmodel;
	m_mode = XMLIO_LOAD;

	m_hfile = CreateFile( strpath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		_ASSERT( 0 );
		return 1;
	}	

	CallF( SetBuffer(), return 1 );

	CBone* topbone = srcmodel->GetTopBone();
	if( !topbone ){
		return 0;
	}


	int findflag = 1;
	while( findflag ){
		XMLIOBUF bonebuf;
		ZeroMemory( &bonebuf, sizeof( XMLIOBUF ) );
		int ret;
		ret = SetXmlIOBuf( &m_xmliobuf, "<Bone>", "</Bone>", &bonebuf );
		if( ret == 0 ){
			CallF( ReadBone( &bonebuf ), return 1 );
		}else{
			findflag = 0;
		}
	}

	return 0;
}

int CRigFile::ReadBone(XMLIOBUF* xmliobuf)
{
	/***
	<Bone>
  	  <RigName>RigName</RigName>
	  <OwnerBone>koshi</OwnerBone>
	  <RigNumber>0</RigNumber>
	  <ElemNum>3</ElemNum>
	  <RigElem>
        <RigRigName>Lte</RigRigName>
	    <RigRigNo>0</RigRigNo>
	    <BoneName>koshi</BoneName>
	    <AxisU>X</AxisU>
	    <RateU>1.0000</RateU>
	    <AxisV>X</AxisV>
	    <RateV>1.0000</RateV>
	  </RigElem>
	  <RigElem>
	    <RigRigName>Rte</RigRigName>
	    <RigRigNo>0</RigRigNo>
	    <BoneName>mune</BoneName>
	    <AxisU>X</AxisU>
	    <RateU>1.0000</RateU>
	    <AxisV>X</AxisV>
	    <RateV>1.0000</RateV>
	  </RigElem>
	</Bone>
	***/

	ZeroMemory(&m_customrig, sizeof(CUSTOMRIG));

	char rigname[256] = { 0 };
	CallF(Read_Str(xmliobuf, "<RigName>", "</RigName>", rigname, 256), return 1);
	WCHAR wrigname[256];
	ZeroMemory(wrigname, sizeof(WCHAR) * 256);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, rigname, 256, wrigname, 256);
	wcscpy_s(m_customrig.rigname, 256, wrigname);

	char bonename[256] = { 0 };
	CallF(Read_Str(xmliobuf, "<OwnerBone>", "</OwnerBone>", bonename, 256), return 1);
	CBone* ownerbone = FindBoneByName(m_model, bonename, 256);
	if (!ownerbone){
		_ASSERT(0);
		return 1;
	}
	m_customrig.rigboneno = ownerbone->GetBoneNo();

	int rigno;
	CallF(Read_Int(xmliobuf, "<RigNumber>", "</RigNumber>", &rigno), return 1);
	m_customrig.rigno = rigno;

	int elemnum;
	CallF(Read_Int(xmliobuf, "<ElemNum>", "</ElemNum>", &elemnum), return 1);
	m_customrig.elemnum = elemnum;

	m_customrig.useflag = 2;

	int readnum = 0;
	int findflag = 1;
	while( findflag ){
		XMLIOBUF rebuf;
		ZeroMemory( &rebuf, sizeof( XMLIOBUF ) );
		int ret;
		ret = SetXmlIOBuf( xmliobuf, "<RigElem>", "</RigElem>", &rebuf );
		if( ret == 0 ){
			CallF(ReadRig(&rebuf, readnum), return 1);
			readnum++;
		}else{
			findflag = 0;
		}
	}

	if (readnum == m_customrig.elemnum){
		if (readnum < MAXRIGELEMNUM){
			int elemno;
			for (elemno = readnum; elemno < MAXRIGELEMNUM; elemno++){
				m_customrig.rigelem[elemno].boneno = -1;
			}
		}

		int isvalid = IsValidCustomRig(m_model, m_customrig, ownerbone);
		if (isvalid == 1){
			ownerbone->SetCustomRig(m_customrig);
		}
		else{
			_ASSERT(0);
			return 1;
		}
	}
	else{
		_ASSERT(0);
		return 1;
	}

	return 0;
}

int CRigFile::ReadRig(XMLIOBUF* xmlbuf, int elemno)
{
	/*
	<RigElem>
	  <RigRigName>Rte</RigRigName>
	  <RigRigNo>0</RigRigNo>
	  <BoneName>koshi</BoneName>
	  <AxisU>X</AxisU>
	  <RateU>1.0000</RateU>
	  <EnableU>1</EnableU>
	  <AxisV>X</AxisV>
	  <RateV>1.0000</RateV>
	  <EnableV>1</EnableV>
	</RigElem>
	*/

	if ((elemno < 0) || (elemno > MAXRIGELEMNUM)){
		_ASSERT(0);
		return 1;
	}
	RIGELEM* dstrigelem = &(m_customrig.rigelem[elemno]);

	int ret, ret2;

	char rigrigname[256] = { 0 };
	ret = Read_Str(xmlbuf, "<RigRigName>", "</RigRigName>", rigrigname, 256);
	if (ret == 0){
		int cmp0 = strcmp("!noname!", rigrigname);
		if (cmp0 == 0){
			dstrigelem->rigrigboneno = -1;
			dstrigelem->rigrigno = -1;
		}
		else{
			CBone* rigrigbone = 0;
			rigrigbone = m_model->GetBoneByName(rigrigname);
			if (rigrigbone){
				dstrigelem->rigrigboneno = rigrigbone->GetBoneNo();

				int rigrigno = -1;
				ret2 = Read_Int(xmlbuf, "<RigRigNo>", "</RigRigNo>", &rigrigno);
				if (ret2 == 0){
					if ((rigrigno >= 0) && (rigrigno < MAXRIGNUM)){
						dstrigelem->rigrigno = rigrigno;
					}
					else{
						dstrigelem->rigrigno = -1;
					}
				}
				else{
					dstrigelem->rigrigno = -1;
				}
			}
			else{
				dstrigelem->rigrigboneno = -1;
				dstrigelem->rigrigno = -1;
			}
		}
	}
	else{
		dstrigelem->rigrigboneno = -1;
		dstrigelem->rigrigno = -1;
	}

	char bonename[256] = { 0 };
	CallF( Read_Str( xmlbuf, "<BoneName>", "</BoneName>", bonename, 256 ), return 1 );
	CBone* elembone = FindBoneByName(m_model, bonename, 256);
	if (!elembone){
		_ASSERT(0);
		return 1;
	}
	dstrigelem->boneno = elembone->GetBoneNo();


	char str_startaxis[2][10] = { "<AxisU>", "<AxisV>" };
	char str_endaxis[2][10] = { "</AxisU>", "</AxisV>" };
	char str_startrate[2][10] = { "<RateU>", "<RateV>" };
	char str_endrate[2][10] = { "</RateU>", "</RateV>" };
	char str_startenable[2][15] = { "<EnableU>", "<EnableV>" };
	char str_endenable[2][15] = { "</EnableU>", "</EnableV>" };

	int uvno;
	for (uvno = 0; uvno < 2; uvno++){
		char straxis[256] = { 0 };
		CallF(Read_Str(xmlbuf, str_startaxis[uvno], str_endaxis[uvno], straxis, 256), return 1);
		int cmpx, cmpy, cmpz;
		cmpx = strncmp(straxis, "X", 1);
		cmpy = strncmp(straxis, "Y", 1);
		cmpz = strncmp(straxis, "Z", 1);
		if (cmpx == 0){
			dstrigelem->transuv[uvno].axiskind = AXIS_X;
		}
		else if (cmpy == 0){
			dstrigelem->transuv[uvno].axiskind = AXIS_Y;
		}
		else if (cmpz == 0){
			dstrigelem->transuv[uvno].axiskind = AXIS_Z;
		}
		float rate;
		CallF(Read_Float(xmlbuf, str_startrate[uvno], str_endrate[uvno], &rate), return 1);
		dstrigelem->transuv[uvno].applyrate = rate;
		int ret;
		int enable;
		ret = Read_Int(xmlbuf, str_startenable[uvno], str_endenable[uvno], &enable);
		if (ret != 0){
			enable = 1;
		}
		else{
			if ((enable != 0) && (enable != 1)){
				enable = 0;
			}
		}
		dstrigelem->transuv[uvno].enable = enable;
	}


	return 0;
}


/***
int CRigFile::CheckFileVersion( XMLIOBUF* xmlbuf )
{
	char kind[256];
	char version[256];
	char type[256];
	ZeroMemory( kind, sizeof( char ) * 256 );
	ZeroMemory( version, sizeof( char ) * 256 );
	ZeroMemory( type, sizeof( char ) * 256 );

	CallF( Read_Str( xmlbuf, "<kind>", "</kind>", kind, 256 ), return 1 );
	CallF( Read_Str( xmlbuf, "<version>", "</version>", version, 256 ), return 1 );
	CallF( Read_Str( xmlbuf, "<type>", "</type>", type, 256 ), return 1 );

	int cmpkind, cmpversion1, cmpversion2, cmptype;
	cmpkind = strcmp( kind, "OpenRDBMotionFile" );
	cmpversion1 = strcmp( version, "1001" );
	cmpversion2 = strcmp( version, "1002" );
	cmptype = strcmp( type, "0" );

	if( (cmpkind == 0) && (cmptype == 0) ){
		if( cmpversion1 == 0 ){
			return 1;
		}else if( cmpversion2 == 0 ){
			return 2;
		}else{
			return 0;
		}

		return 0;
	}else{
		_ASSERT( 0 );
		return 0;
	}

	return 0;
}
***/
