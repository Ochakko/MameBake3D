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


#include <lmtFile.h>

#include <Model.h>
#include <mqoobject.h>
#include <Bone.h>

//#include <BoneProp.h>

#define DBGH
#include <dbg.h>

#include <map>

#include <shlwapi.h>

using namespace std;

CLmtFile::CLmtFile()
{
	InitParams();
}

CLmtFile::~CLmtFile()
{
	DestroyObjs();
}

int CLmtFile::InitParams()
{
	CXMLIO::InitParams();

	m_model = 0;
	return 0;
}

int CLmtFile::DestroyObjs()
{
	CXMLIO::DestroyObjs();

	InitParams();

	return 0;
}


int CLmtFile::WriteLmtFile( WCHAR* strpath, CModel* srcmodel )
{
	m_model = srcmodel;
	m_mode = XMLIO_WRITE;

	if( !m_model->GetTopBone() ){
		return 0;
	}

	WCHAR wfilename[MAX_PATH] = {0L};
	WCHAR* lasten;
	lasten = wcsrchr( strpath, TEXT('\\') );
	if( !lasten ){
		_ASSERT( 0 );
		return 1;
	}
	wcscpy_s( wfilename, MAX_PATH, lasten + 1 );

	char mfilename[MAX_PATH] = {0};
	WideCharToMultiByte( CP_ACP, 0, wfilename, -1, mfilename, MAX_PATH, NULL, NULL );

	m_hfile = CreateFile(strpath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		DbgOut( L"LmtFile : WriteLmtFile : file open error !!! %s\n", strpath );
		_ASSERT( 0 );
		return 1;
	}


	CallF( Write2File( "<?xml version=\"1.0\" encoding=\"Shift_JIS\"?>\r\n<Lmt>\r\n" ), return 1 );  
	CallF( Write2File( "    <FileInfo>1001-01</FileInfo>\r\n" ), return 1 );

	WriteLmtReq( m_model->GetTopBone() );

	CallF( Write2File( "</Lmt>\r\n" ), return 1 );

	return 0;
}
void CLmtFile::WriteLmtReq( CBone* srcbone )
{
	WriteLmt( srcbone );

	if( srcbone->GetChild() ){
		WriteLmtReq( srcbone->GetChild() );
	}
	if( srcbone->GetBrother() ){
		WriteLmtReq( srcbone->GetBrother() );
	}
}


int CLmtFile::WriteLmt( CBone* srcbone )
{
/***
  <Bone>
    <Name>zentai_ido</Nama>
	<BoneAxisType>Current</BoneAxisType>        //Current or Parent or Global
	<NodeMatrix_11>1.0000</NodeMatrix_11>		//bind pose matrix : float
	<NodeMatrix_12>0.0000</NodeMatrix_12>
	<NodeMatrix_13>0.0000</NodeMatrix_13>
	<NodeMatrix_14>0.0000</NodeMatrix_14>
	<NodeMatrix_21>0.0000</NodeMatrix_21>
	<NodeMatrix_22>1.0000</NodeMatrix_22>
	<NodeMatrix_23>0.0000</NodeMatrix_23>
	<NodeMatrix_24>0.0000</NodeMatrix_24>
	<NodeMatrix_31>0.0000</NodeMatrix_31>
	<NodeMatrix_32>0.0000</NodeMatrix_32>
	<NodeMatrix_33>1.0000</NodeMatrix_33>
	<NodeMatrix_34>0.0000</NodeMatrix_34>
	<NodeMatrix_41>0.0000</NodeMatrix_41>
	<NodeMatrix_42>0.0000</NodeMatrix_42>
	<NodeMatrix_43>0.0000</NodeMatrix_43>
	<NodeMatrix_44>1.0000</NodeMatrix_44>
	<Via180_X>0</Via180_X>						// 0 or 1
	<Via180_Y>0</Via180_Y>
	<Via180_Z>0</Via180_Z>
	<Lower_X>-180</Lower_X>						//from -180 to 180 integer
	<Upper_X>180</Upper_X>
	<Lower_Y>-180</Lower_Y>
	<Upper_Y>180</Upper_Y>
	<Lower_Z>-180</Lower_Z>
	<Upper_Z>180</Upper_Z>
  </Bone>
***/
	CallF( Write2File( "  <Bone>\r\n" ), return 1);
	CallF( Write2File( "    <Name>%s</Name>\r\n", srcbone->GetBoneName() ), return 1);


	ANGLELIMIT anglelimit = srcbone->GetAngleLimit();

	char strboneaxistype[3][256] = {"Current", "Parent", "Global"};
	if ((anglelimit.boneaxiskind >= 0) && (anglelimit.boneaxiskind <= 2)){
		CallF(Write2File("    <BoneAxisType>%s</BoneAxisType>\r\n", strboneaxistype[anglelimit.boneaxiskind]), return 1);
	}

	ChaMatrix nodemat = srcbone->GetNodeMat();
	CallF(Write2File("      <NodeMatrix_11>%f</NodeMatrix_11>\r\n", nodemat._11), return 1);
	CallF(Write2File("      <NodeMatrix_12>%f</NodeMatrix_12>\r\n", nodemat._12), return 1);
	CallF(Write2File("      <NodeMatrix_13>%f</NodeMatrix_13>\r\n", nodemat._13), return 1);
	CallF(Write2File("      <NodeMatrix_14>%f</NodeMatrix_14>\r\n", nodemat._14), return 1);

	CallF(Write2File("      <NodeMatrix_21>%f</NodeMatrix_21>\r\n", nodemat._21), return 1);
	CallF(Write2File("      <NodeMatrix_22>%f</NodeMatrix_22>\r\n", nodemat._22), return 1);
	CallF(Write2File("      <NodeMatrix_23>%f</NodeMatrix_23>\r\n", nodemat._23), return 1);
	CallF(Write2File("      <NodeMatrix_24>%f</NodeMatrix_24>\r\n", nodemat._24), return 1);

	CallF(Write2File("      <NodeMatrix_31>%f</NodeMatrix_31>\r\n", nodemat._31), return 1);
	CallF(Write2File("      <NodeMatrix_32>%f</NodeMatrix_32>\r\n", nodemat._32), return 1);
	CallF(Write2File("      <NodeMatrix_33>%f</NodeMatrix_33>\r\n", nodemat._33), return 1);
	CallF(Write2File("      <NodeMatrix_34>%f</NodeMatrix_34>\r\n", nodemat._34), return 1);

	CallF(Write2File("      <NodeMatrix_41>%f</NodeMatrix_41>\r\n", nodemat._41), return 1);
	CallF(Write2File("      <NodeMatrix_42>%f</NodeMatrix_42>\r\n", nodemat._42), return 1);
	CallF(Write2File("      <NodeMatrix_43>%f</NodeMatrix_43>\r\n", nodemat._43), return 1);
	CallF(Write2File("      <NodeMatrix_44>%f</NodeMatrix_44>\r\n", nodemat._44), return 1);

	CallF(Write2File("      <Via180_X>%d</Via180_X>\r\n", anglelimit.via180flag[0]), return 1);
	CallF(Write2File("      <Via180_Y>%d</Via180_Y>\r\n", anglelimit.via180flag[1]), return 1);
	CallF(Write2File("      <Via180_Z>%d</Via180_Z>\r\n", anglelimit.via180flag[2]), return 1);

	CallF(Write2File("      <Lower_X>%d</Lower_X>\r\n", anglelimit.lower[0]), return 1);
	CallF(Write2File("      <Upper_X>%d</Upper_X>\r\n", anglelimit.upper[0]), return 1);

	CallF(Write2File("      <Lower_Y>%d</Lower_Y>\r\n", anglelimit.lower[1]), return 1);
	CallF(Write2File("      <Upper_Y>%d</Upper_Y>\r\n", anglelimit.upper[1]), return 1);

	CallF(Write2File("      <Lower_Z>%d</Lower_Z>\r\n", anglelimit.lower[2]), return 1);
	CallF(Write2File("      <Upper_Z>%d</Upper_Z>\r\n", anglelimit.upper[2]), return 1);

	CallF( Write2File( "  </Bone>\r\n" ), return 1);

	return 0;
}




int CLmtFile::LoadLmtFile( WCHAR* strpath, CModel* srcmodel )
{
	m_model = srcmodel;
	m_mode = XMLIO_LOAD;

	WCHAR wfilename[MAX_PATH] = {0L};
	WCHAR* lasten;
	lasten = wcsrchr( strpath, TEXT('\\') );
	if( !lasten ){
		_ASSERT( 0 );
		return 1;
	}
	wcscpy_s( wfilename, MAX_PATH, lasten + 1 );

	char mfilename[MAX_PATH] = {0};
	WideCharToMultiByte( CP_ACP, 0, wfilename, -1, mfilename, MAX_PATH, NULL, NULL );

	BOOL bexist;
	bexist = PathFileExists(strpath);
	if (!bexist)
	{
		_ASSERT(0);
		return 2;//not found
	}


	m_hfile = CreateFile(strpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( m_hfile == INVALID_HANDLE_VALUE ){
		//_ASSERT( 0 );
		return 1;
	}	

	CallF( SetBuffer(), return 1 );


	CBone* topbone = srcmodel->GetTopBone();

	int posstep = 0;
//	XMLIOBUF fileinfobuf;
//	ZeroMemory( &fileinfobuf, sizeof( XMLIOBUF ) );
//	CallF( SetXmlIOBuf( &m_xmliobuf, "<FileInfo>", "</FileInfo>", &fileinfobuf ), return 1 );
//	m_fileversion = CheckFileVersion( &fileinfobuf );
//	if( m_fileversion <= 0 ){
//		_ASSERT( 0 );
//		return 1;
//	}

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

int CLmtFile::ReadBone( XMLIOBUF* xmliobuf )
{
	/***
	<Bone>
	<Name>zentai_ido</Nama>
	<BoneAxisType>Current</BoneAxisType>        //Current or Parent or Global
	<NodeMatrix_11>1.0000</NodeMatrix_11>		//bind pose matrix : float
	<NodeMatrix_12>0.0000</NodeMatrix_12>
	<NodeMatrix_13>0.0000</NodeMatrix_13>
	<NodeMatrix_14>0.0000</NodeMatrix_14>
	<NodeMatrix_21>0.0000</NodeMatrix_21>
	<NodeMatrix_22>1.0000</NodeMatrix_22>
	<NodeMatrix_23>0.0000</NodeMatrix_23>
	<NodeMatrix_24>0.0000</NodeMatrix_24>
	<NodeMatrix_31>0.0000</NodeMatrix_31>
	<NodeMatrix_32>0.0000</NodeMatrix_32>
	<NodeMatrix_33>1.0000</NodeMatrix_33>
	<NodeMatrix_34>0.0000</NodeMatrix_34>
	<NodeMatrix_41>0.0000</NodeMatrix_41>
	<NodeMatrix_42>0.0000</NodeMatrix_42>
	<NodeMatrix_43>0.0000</NodeMatrix_43>
	<NodeMatrix_44>1.0000</NodeMatrix_44>
	<Via180_X>0</Via180_X>						// 0 or 1
	<Via180_Y>0</Via180_Y>
	<Via180_Z>0</Via180_Z>
	<Lower_X>-180</Lower_X>						//from -180 to 180 integer
	<Upper_X>180</Upper_X>
	<Lower_Y>-180</Lower_Y>
	<Upper_Y>180</Upper_Y>
	<Lower_Z>-180</Lower_Z>
	<Upper_Z>180</Upper_Z>
	</Bone>
	***/

	char bonename[256];
	char bonename1[256];//_Joint—L‚è
	char bonename2[256];//_Joint–³‚µ
	ZeroMemory(bonename, sizeof(char) * 256);
	CallF( Read_Str(xmliobuf, "<Name>", "</Name>", bonename, 256), return 1 );
	char* jointnameptr = strstr(bonename, "_Joint");
	if (!jointnameptr){
		sprintf_s(bonename1, 256, "%s_Joint", bonename);
		strcpy_s(bonename2, 256, bonename);
	}
	else{
		strcpy_s(bonename1, 256, bonename);
		strcpy_s(bonename2, 256, bonename);
		int headleng = (int)(jointnameptr - bonename);
		*(bonename2 + headleng) = 0;
	}
	CBone* curbone = m_model->GetBoneByName(bonename1);
	if (!curbone){
		curbone = m_model->GetBoneByName(bonename2);
		if (!curbone){
			_ASSERT(0);
			return 0;
		}
	}

	InitAngleLimit(&m_anglelimit);


	char boneaxistype[256] = { 0 };
	CallF(Read_Str(xmliobuf, "<BoneAxisType>", "</BoneAxisType>", boneaxistype, 256), return 1);
	int cmpcur, cmppar, cmpglobal;
	cmpcur = strncmp(boneaxistype, "Current", 7);
	cmppar = strncmp(boneaxistype, "Parent", 6);
	cmpglobal = strncmp(boneaxistype, "Global", 6);
	if (cmpcur == 0){
		m_anglelimit.boneaxiskind = 0;
	}
	else{
		if (cmppar == 0){
			m_anglelimit.boneaxiskind = 1;
		}
		else{
			if (cmpglobal == 0){
				m_anglelimit.boneaxiskind = 2;
			}
			else{
				_ASSERT(0);
				m_anglelimit.boneaxiskind = 0;
			}
		}
	}

	ChaMatrix nodemat;
	ChaMatrixIdentity(&nodemat);

	CallF(Read_Float(xmliobuf, "<NodeMatrix_11>", "</NodeMatrix_11>", &nodemat._11), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_12>", "</NodeMatrix_12>", &nodemat._12), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_13>", "</NodeMatrix_13>", &nodemat._13), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_14>", "</NodeMatrix_14>", &nodemat._14), return 1);

	CallF(Read_Float(xmliobuf, "<NodeMatrix_21>", "</NodeMatrix_21>", &nodemat._21), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_22>", "</NodeMatrix_22>", &nodemat._22), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_23>", "</NodeMatrix_23>", &nodemat._23), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_24>", "</NodeMatrix_24>", &nodemat._24), return 1);

	CallF(Read_Float(xmliobuf, "<NodeMatrix_31>", "</NodeMatrix_31>", &nodemat._31), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_32>", "</NodeMatrix_32>", &nodemat._32), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_33>", "</NodeMatrix_33>", &nodemat._33), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_34>", "</NodeMatrix_34>", &nodemat._34), return 1);

	CallF(Read_Float(xmliobuf, "<NodeMatrix_41>", "</NodeMatrix_41>", &nodemat._41), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_42>", "</NodeMatrix_42>", &nodemat._42), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_43>", "</NodeMatrix_43>", &nodemat._43), return 1);
	CallF(Read_Float(xmliobuf, "<NodeMatrix_44>", "</NodeMatrix_44>", &nodemat._44), return 1);

	curbone->SetNodeMat(nodemat);

	int tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Via180_X>", "</Via180_X>", &tmpint), return 1);
	if ((tmpint == 0) || (tmpint == 1)){
		m_anglelimit.via180flag[0] = tmpint;
	}
	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Via180_Y>", "</Via180_Y>", &tmpint), return 1);
	if ((tmpint == 0) || (tmpint == 1)){
		m_anglelimit.via180flag[1] = tmpint;
	}
	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Via180_Z>", "</Via180_Z>", &tmpint), return 1);
	if ((tmpint == 0) || (tmpint == 1)){
		m_anglelimit.via180flag[2] = tmpint;
	}


	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Lower_X>", "</Lower_X>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.lower[0] = tmpint;
	}
	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Upper_X>", "</Upper_X>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.upper[0] = tmpint;
	}

	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Lower_Y>", "</Lower_Y>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.lower[1] = tmpint;
	}
	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Upper_Y>", "</Upper_Y>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.upper[1] = tmpint;
	}

	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Lower_Z>", "</Lower_Z>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.lower[2] = tmpint;
	}
	tmpint = 0;
	CallF(Read_Int(xmliobuf, "<Upper_Z>", "</Upper_Z>", &tmpint), return 1);
	if ((tmpint >= -180) && (tmpint <= 180)){
		m_anglelimit.upper[2] = tmpint;
	}

	curbone->SetAngleLimit(m_anglelimit);


	return 0;
}


/***
int CLmtFile::CheckFileVersion( XMLIOBUF* xmlbuf )
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
