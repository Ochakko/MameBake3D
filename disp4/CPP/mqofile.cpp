//#include <stdafx.h>

#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>

#define DBGH
#include <dbg.h>

#include <crtdbg.h>

#include <mqofile.h>
#include <Model.h>
#include <mqomaterial.h>
#include <mqoobject.h>
#include <mqoface.h>

#include <algorithm>
#include <iostream>
#include <iterator>

#define BUFBLOCKLEN	2048


CMQOFile::CMQOFile()
{
	InitLoadParams();
}
CMQOFile::~CMQOFile()
{
	if( mqobuf.buf ){
		free( mqobuf.buf );
		mqobuf.buf = 0;
	}
	if( mqobuf.hfile != INVALID_HANDLE_VALUE ){
		//FlushFileBuffers( hfile );
		//SetEndOfFile( hfile );
		CloseHandle( mqobuf.hfile );
		mqobuf.hfile = INVALID_HANDLE_VALUE;
	}	
}

void CMQOFile::InitLoadParams()
{

	m_modelptr = 0;
	m_objcnt = 0;

	ZeroMemory( m_linechar, LINECHARLENG );
	ZeroMemory( m_wline, sizeof( WCHAR ) * LINECHARLENG );

	m_state = BEGIN_NONE;

	currentobj = 0;

	mqobuf.hfile = INVALID_HANDLE_VALUE;
	mqobuf.buf = 0;
	mqobuf.bufleng = 0;
	mqobuf.pos = 0;
	mqobuf.isend = 0;

	ZeroMemory( tempname, NAMELENG + 1 );

	m_multiple = 1.0f;
	m_groundflag = 0;

	D3DXMatrixIdentity( &m_offsetmat );


	m_pdev = 0;
}


int CMQOFile::LoadMQOFile( LPDIRECT3DDEVICE9 pdev, float multiple, WCHAR* filename, 
	D3DXVECTOR3 offsetpos, D3DXVECTOR3 rot, CModel* srcmodel )
{
	m_pdev = pdev;
	m_modelptr = srcmodel;

	int ret;

	if( !filename || !m_modelptr ){
		_ASSERT( 0 );
		return 1;
	}

	SetLastError( 0 );
	mqobuf.hfile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( mqobuf.hfile == INVALID_HANDLE_VALUE ){
		DWORD dwer;
		dwer = GetLastError();


		LPVOID lpMsgBuf = 0;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwer,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
		);
		
		WCHAR mes[4098];
		swprintf_s( mes, 4098, L"ファイルを開けませんでした。ファイル名があっているか確認してください。\n%s", filename );
		::MessageBox( NULL, mes, L"エラー", MB_OK );

		if( lpMsgBuf ){
			::MessageBox( NULL, (WCHAR*)lpMsgBuf, L"GetLastError", MB_OK );
			DbgOut( L"mqofile : LoadMQOFile : %s : error !!!\r\n", (WCHAR*)lpMsgBuf );
			LocalFree(lpMsgBuf);
		}

		ret = D3DAPPERR_MEDIANOTFOUND;
		return ret;
	}	

	CallF( LoadMQOFile_aft( multiple, offsetpos, rot ), return 1 );

	return 0;
}


int CMQOFile::LoadMQOFile_aft( float multiple, D3DXVECTOR3 offsetpos, D3DXVECTOR3 rot )
{
	int ret = 0;
	m_multiple = multiple;

	D3DXMATRIX scalemat, rotxmat, rotymat, rotzmat, shiftmat;
	D3DXMatrixIdentity( &scalemat );
	D3DXMatrixIdentity( &rotxmat );
	D3DXMatrixIdentity( &rotymat );
	D3DXMatrixIdentity( &rotzmat );
	D3DXMatrixIdentity( &shiftmat );

	scalemat._11 = m_multiple;
	scalemat._22 = m_multiple;
	scalemat._33 = -m_multiple;//!!!!!!!! 反転

	shiftmat._41 = offsetpos.x;
	shiftmat._42 = offsetpos.y;
	shiftmat._43 = offsetpos.z;

	D3DXMatrixRotationX( &rotxmat, rot.x * (float)DEG2PAI );
	D3DXMatrixRotationY( &rotymat, rot.y * (float)DEG2PAI );
	D3DXMatrixRotationZ( &rotzmat, rot.z * (float)DEG2PAI );

	m_offsetmat = scalemat * rotzmat * rotymat * rotxmat * shiftmat;


//////////
	CallF( SetBuffer(), return 1 );
	CallF( CheckFileVersion(), return 1 );
	m_state = BEGIN_FINDCHUNK;

	while( m_state != BEGIN_FINISH ){
		//map<int, CMQOMaterial*> curmaterial;
		//map<int, CMQOObject*> curobject;

		switch( m_state ){
		case BEGIN_FINDCHUNK:
			ret = FindChunk( &m_state );
			if( ret ){
				if( mqobuf.isend == 1 ){
					ret = 0;
				}else{
					_ASSERT( 0 );
					return 1;
				}
			}
			break;
		case BEGIN_SCENE:
			CallF( ReadScene( &m_state ), return 1 );
			break;
		case BEGIN_BACKIMAGE:
			CallF( SkipChunk(), return 1 );
			m_state = BEGIN_FINDCHUNK;
			break;
		case BEGIN_MATERIAL:
			//m_modelptr->GetMQOMaterial2( curmaterial );
			CallF( ReadMaterial( &m_state ), return 1 );
			break;
		case BEGIN_OBJECT://子チャンクをもつ
			//m_modelptr->GetMqoObject2( curobject );
			CallF( ReadObject( &m_state ), return 1 );
			break;
		case BEGIN_VERTEX:
			if( currentobj ){
				CallF( ReadVertex( &m_state ), return 1 );
			}else{
				DbgOut( L"MQOFile : LoadMQOFile : BEGIN_VERTEX : currentobj error !!!" );
				_ASSERT( 0 );

				CallF( SkipChunk(), return 1 );
				m_state = BEGIN_FINDCHUNK;
			}
			break;
		case BEGIN_FACE:
			if( currentobj ){
				CallF( ReadFace( &m_state ), return 1 );
			}else{
				DbgOut( L"MQOFile : LoadMQOFile : BEGIN_FACE : currentobj error !!!" );
				_ASSERT( 0 );

				CallF( SkipChunk(), return 1 );
				m_state = BEGIN_FINDCHUNK;
			}
			break;
		case BEGIN_BVERTEX://子チャンクをもつ
			if( currentobj ){
				CallF( ReadBVertex( &m_state ), return 1 );
			}else{
				DbgOut( L"MQOFile : LoadMQOFile : BEGIN_BVERTEX : currentobj error !!!" );
				_ASSERT( 0 );

				CallF( SkipChunk(), return 1 );
				m_state = BEGIN_FINDCHUNK;
			}			
			break;
		case BEGIN_COLOR:
			if( currentobj ){
				CallF( ReadColor( &m_state ), return 1 );
			}else{
				DbgOut( L"MQOFile : LoadMQOFile : BEGIN_COLOR : currentobj error !!!" );
				_ASSERT( 0 );

				CallF( SkipChunk(), return 1 );
				m_state = BEGIN_FINDCHUNK;
			}			
			break;
		case BEGIN_VERTEXATTR://子チャンクをもつ
			m_state = BEGIN_FINDCHUNK;
			break;

		case BEGIN_VECTOR:
		case BEGIN_WEIT:
		case BEGIN_BLOB:
		case BEGIN_UNKCHUNK:
			CallF( SkipChunk(), return 1 );
			m_state = BEGIN_FINDCHUNK;
			break;

		case BEGIN_TRIALNOISE:
		case BEGIN_FINISH:
		case BEGIN_NONE:
		default:
			m_state = BEGIN_FINISH;
			break;
		}
	}

	CallF( m_modelptr->AddDefMaterial(), return 1 );

	//map<int, CMQOObject*> curobject;
	//m_modelptr->GetMqoObject2( curobject );
	CallF( Multiple(), return 1 );
	CallF( m_modelptr->MakePolyMesh3(), return 1 );
	CallF( m_modelptr->MakeExtLine(), return 1 );
	CallF( m_modelptr->MakeObjectName(), return 1 );

	return 0;
}




int CMQOFile::SetBuffer()
{
	DWORD sizehigh;
	DWORD bufleng;
	bufleng = GetFileSize( mqobuf.hfile, &sizehigh );
	if( bufleng < 0 ){
		_ASSERT( 0 );
		return 1;
	}
	if( sizehigh != 0 ){
		_ASSERT( 0 );
		return 1;
	}

	unsigned char* newbuf;
	newbuf = (unsigned char*)malloc( sizeof( char ) * ( bufleng + 1 ) );
	if( !newbuf ){
		_ASSERT( 0 );
		return 1;
	}
	ZeroMemory( newbuf, sizeof( unsigned char ) * ( bufleng + 1 ) );


	DWORD rleng, readleng;
	rleng = bufleng;
	ReadFile( mqobuf.hfile, (void*)newbuf, rleng, &readleng, NULL );
	if( rleng != readleng ){
		_ASSERT( 0 );
		free( newbuf );
		return 1;
	}

	mqobuf.buf = newbuf;
	mqobuf.pos = 0;
	mqobuf.isend = 0;

	int validleng;
	char* endptr;
	endptr = strstr( (char*)newbuf, "Eof\r\n" );
	if( endptr ){
		validleng = (int)( endptr - (char*)newbuf );
	}else{
		validleng = bufleng;
	}
	mqobuf.bufleng = validleng;

	return 0;
}


int CMQOFile::CheckFileVersion()
{
	int ret;
	int leng1, leng2;

	ret = GetLine( &leng1 );
	DbgOut( L"mqofile : CheckFileVersion : filekind : %s\r\n", m_wline );
	if( ret ){
		::MessageBox( NULL, L"Metasequoia Documentではありません。\n読み込めません。", L"読み込みエラー", MB_OK );

		return 1;
	}
	
	char pat1[] = "Metasequoia Document";
	int pat1leng = (int)strlen( pat1 );
	int cmp1 = 1;//!!!

	if( pat1leng <= leng1 )
		cmp1 = strncmp( pat1, m_linechar, pat1leng );

	if( cmp1 ){
		::MessageBox( NULL, L"Metasequoia Documentではありません。\n読み込めません。", L"読み込みエラー", MB_OK );
		return 1;
	}

////////	
	ret = GetLine( &leng2 );
	DbgOut( L"mqofile : CheckFileVersion : fileversion : %s\r\n", m_wline );
	if( ret ){
		::MessageBox( NULL, L"このfile versionには、対応していません。\n読み込めません。", L"読み込みエラー", MB_OK );
		return 1;
	}

	char pat2[] = "Format Text Ver 1.0";
	int pat2leng = (int)strlen( pat2 );
	int cmp2 = 1;//!!!

	if( pat2leng <= leng2 )
		cmp2 = strncmp( pat2, m_linechar, pat2leng );

	if( cmp2 ){
		char pat3[] = "Format Text Ver 1.1";
		int pat3leng = (int)strlen(pat3);
		int cmp3 = 1;//!!!

		if (pat3leng <= leng2)
			cmp3 = strncmp(pat3, m_linechar, pat3leng);

		if (cmp3){
			::MessageBox(NULL, L"このfile versionには、対応していません。\n読み込めません。", L"読み込みエラー", MB_OK);
			return 1;
		}
	}


	return 0;
}

int CMQOFile::GetLine( int* getlen )
{
	if( (mqobuf.pos >= mqobuf.bufleng) || (mqobuf.isend == 1) ){
		mqobuf.isend = 1;
		m_state = BEGIN_FINISH;
		return 1;
	}

	int notfound = 1;
	int stepno = 0;
	while( (mqobuf.pos + stepno < mqobuf.bufleng) && (notfound = ( *(mqobuf.buf + mqobuf.pos + stepno) != '\n' )) ){
		stepno++;

		if( mqobuf.pos + stepno >= mqobuf.bufleng ){
			mqobuf.isend = 1;
			break;
		}
	}

	if( notfound == 0 ){
		stepno++; //\nの分
	}

	if( LINECHARLENG > stepno ){
		strncpy_s( m_linechar, LINECHARLENG, (const char*)(mqobuf.buf + mqobuf.pos), stepno );
		*getlen = stepno;
		mqobuf.pos += stepno;

		m_linechar[stepno] = 0;

	}else{
		_ASSERT( 0 );
		DbgOut( L"MQOFile : GetLine : dstlen short error !!!" );
		return 1;
	}
	
	ZeroMemory( m_wline, sizeof( WCHAR ) * LINECHARLENG );
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_linechar, LINECHARLENG, m_wline, LINECHARLENG );

	if( (notfound == 1) && (mqobuf.isend == 1) ){
		m_state = BEGIN_FINISH;
		return 0;
	}else{
		if( ((*getlen == 2) && (strcmp( m_linechar, "\r\n" ) == 0)) || ((*getlen == 1) && (*m_linechar == '\n')) ){
			int retgl;			
			retgl = GetLine( getlen );	
			return retgl;
		}else{
			return 0;
		}
	}

}

int CMQOFile::GetBytes( unsigned char** dstuc, int getlen )
{

	if( mqobuf.pos + getlen >= mqobuf.bufleng ){
		mqobuf.isend = 1;
		m_state = BEGIN_FINISH;
		return 0;
	}

	*dstuc = mqobuf.buf + mqobuf.pos;
	mqobuf.pos += getlen;

	return 0;
}

int CMQOFile::FindChunk( MQOSTATE* nextstate )
{
	int isfind = 0;
	int ret = 0;
	int getleng;
	while( isfind == 0 ){
		ret = GetLine( &getleng );
		if( ret ){
			*nextstate = BEGIN_FINISH;
			break;
		}
		if( (getleng >= 3) && (strstr( m_linechar, "{\r\n" ) != NULL) ){
			isfind = 1;
			ret = GetChunkType( nextstate, m_linechar, getleng );
			if( ret ){
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				break;
			}
		}else if( (getleng >= 5) && (strstr( m_linechar, "Eof\r\n" ) != NULL) ){
			mqobuf.isend = 1;
			*nextstate = BEGIN_FINISH;
			break;
		}
	}


	return ret;
}

int CMQOFile::GetChunkType( MQOSTATE* type, char* chunkname, int nameleng )
{
	char* namehead = chunkname;

	while( (*namehead == ' ') || (*namehead == '\t') ){
		namehead++;
	}

	char chunkpat[13][20] =
	{
		"TrialNoise ",
		"Scene ",
		"BackImage ",
		"Material ",
		"Object ",
		"vertexattr ",
		"vertex ", //vertexattrより後。
		"weit ",
		"color ",
		"BVertex ",
		"Vector ",
		"face ",
		"Blob "
	};

	int isfind = 0;
	int patno;
	for( patno = 0; patno < 13; patno++ ){
		if( isfind == 1 )
			break;

		int patleng;
		patleng = (int)strlen( chunkpat[patno] );
		int cmp = 1;
		if( nameleng >= patleng ){
			cmp = strncmp( chunkpat[patno], namehead, patleng );
			if( cmp == 0 ){
				isfind = 1;
				switch( patno ){
				case 0:
					*type = BEGIN_TRIALNOISE;
					break;
				case 1:
					*type = BEGIN_SCENE;
					break;
				case 2:
					*type = BEGIN_BACKIMAGE;
					break;
				case 3:
					*type = BEGIN_MATERIAL;
					break;
				case 4:
					*type = BEGIN_OBJECT;
					break;
				case 5:
					*type = BEGIN_VERTEXATTR;
					break;
				case 6:
					*type = BEGIN_VERTEX;
					break;
				case 7:
					*type = BEGIN_WEIT;
					break;
				case 8:
					*type = BEGIN_COLOR;
					break;
				case 9:
					*type = BEGIN_BVERTEX;
					break;
				case 10:
					*type = BEGIN_VECTOR;
					break;
				case 11:
					*type = BEGIN_FACE;
					break;
				case 12:
					*type = BEGIN_BLOB;
					break;
				default:
					break;
				}
			}
		}

	}

	if( isfind == 0 ){
		_ASSERT( 0 );
		*type = BEGIN_UNKCHUNK;
	}

	return 0;
}

int CMQOFile::SkipChunk()
{
	int depth = 1;
	int findend = 0;
	int ret;
	//char linechar[512];

	while( (findend == 0) || (depth != 0) ){
		
		findend = 0;

		int getleng;
		ret = GetLine( &getleng );
		if( ret ){
			_ASSERT( 0 );
			return ret;
		}

		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
			depth--;
		}else if( (getleng >= 3) && (strstr( m_linechar, "{\r\n" ) != NULL) ){
			depth++;
		}

	}

	

	return 0;
}

int CMQOFile::ReadColor( MQOSTATE* nextstate )
{
	int ret;
	int findend = 0;
	int getleng;

	_ASSERT( currentobj );
	ret = currentobj->CreateColor();
	if( ret ){
		DbgOut( L"MQOFile : ReadColor : CreateColor error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}			
	
	int matno = 0;
	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;
		
		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
		}else{
			ret = currentobj->SetColor( m_linechar, (int)strlen( m_linechar ) );
			if( ret ){
				DbgOut( L"MQOFile : ReadColor : SetColor error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}			
		}
	}
	
	*nextstate = BEGIN_FINDCHUNK;

	return 0;



}

int CMQOFile::ReadScene( MQOSTATE* nextstate )
{
	int ret;
	int findend = 0;
	int getleng;

	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;

		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
			//*nextstate = BEGIN_FINISH;
		}else{
			char ambpat[10] = "amb ";
			char* ambptr = 0;

			ambptr = strstr( m_linechar, ambpat );
			if( ambptr ){
				ARGBF ambient;
				ambient.a = 1.0f;
				ambient.r = 0.25f;
				ambient.g = 0.25f;
				ambient.b = 0.25f;
				
				int leng;
				leng = (int)strlen( m_linechar );

				int step;
				int cnt;
				float dstfloat;
				int stepnum;
				stepnum = 4;
				for( cnt = 0; cnt < 3; cnt++ ){		
					ret = GetFloat( &dstfloat, m_linechar, stepnum, leng, &step ); 
					if( ret )
						return ret;

					switch( cnt ){
					case 0:
						ambient.r = dstfloat;
						break;
					case 1:
						ambient.g = dstfloat;
						break;
					case 2:
						ambient.b = dstfloat;
						break;
					default:
						break;
					}

					stepnum += step;
				}
		
				m_scene_ambient.x = ambient.r;
				m_scene_ambient.y = ambient.g;
				m_scene_ambient.z = ambient.b;
				m_scene_ambient.w = ambient.a;
				DbgOut( L"mqofile : Scene : ambient %f %f %f\r\n", ambient.r, ambient.g, ambient.b );

			}


		}
	}

	*nextstate = BEGIN_FINDCHUNK;

	return 0;
}

int CMQOFile::GetFloat( float* dstfloat, char* srcchar, int pos, int srcleng, int* stepnum )
{
	int flstart;
	int flend;
	int step = 0;

	while( (pos + step < srcleng) && 
	( (*(srcchar + pos + step) == ' ') || (*(srcchar + pos + step) == '\t') )
	//( *(srcchar + pos + step) ) 
	){
		step++;
	}

	flstart = pos+ step;

	while( (pos + step < srcleng) && 
		( isdigit( *(srcchar + pos + step) ) || (*(srcchar + pos + step) == '.') || (*(srcchar + pos + step) == '-') )
	){
		step++;
	}
	flend = pos + step;

	char tempchar[256];
	if( flend - flstart < 256 ){
		strncpy_s( tempchar, 256, srcchar + flstart, flend - flstart );
		tempchar[flend - flstart] = 0;
		*dstfloat = (float)atof( tempchar );
	}else{
		_ASSERT( 0 );
		*dstfloat = 0.0f;
	}

	*stepnum = step;

	return 0;

}


int CMQOFile::ReadMaterial( MQOSTATE* nextstate )
{
	int ret;
	int findend = 0;
	//char linechar[1024];
	int getleng;

	int matno = 0;
	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;
		
		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
			//*nextstate = BEGIN_FINISH;
		}else{
			CMQOMaterial* newmat;
			newmat = new CMQOMaterial();
			if( !newmat ){
				DbgOut( L"MQOFile : ReadMaterial : newmat alloc error !!!" );
				*nextstate = BEGIN_FINISH;
				return 1;
			}
		


//DbgOut( L"MQOFile : ReadMaterial : SetParams : %s\n", m_linechar );
			matno = m_modelptr->GetMQOMaterialSize();
			ret = newmat->SetParams( matno, m_scene_ambient, m_linechar, (int)strlen( m_linechar ) );
			if( ret ){
				DbgOut( L"MQOFile : ReadMaterial : newmat SetParams error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}

			m_modelptr->SetMQOMaterial( newmat->GetMaterialNo(), newmat ); 
		}
	}
	
	*nextstate = BEGIN_FINDCHUNK;

	return 0;
}


int CMQOFile::ReadObject( MQOSTATE* nextstate )
{
	int ret;
	int findend = 0;
	int getleng;

	m_objcnt++;

	CMQOObject* newobj;
	newobj = new CMQOObject();
	if( !newobj ){
		DbgOut( L"MQOFile : ReadObject : newobj alloc error !!!" );
		*nextstate = BEGIN_FINISH;
		return 1;
	}
	m_modelptr->SetMqoObject( newobj->GetObjectNo(), newobj );
	currentobj = newobj;//!!!!

	//nameのセット
	ret = newobj->SetParams( m_linechar, (int)strlen( m_linechar ) );
	if( ret ){
		DbgOut( L"MQOFile : ReadObject : newobj SetParams error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}			


	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;
		
		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
			currentobj = 0;

		}else if( (getleng >= 3) && (strstr( m_linechar, "{\r\n" ) != NULL) ){
			//子チャンクをチェック
			ret = GetChunkType( nextstate, m_linechar, (int)strlen( m_linechar ) );
			_ASSERT( !ret );
			return 0;
		}else{
			//DbgOut( L"MQOFile : ReadObject : SetParams : %s\n", m_linechar );
			ret = newobj->SetParams( m_linechar, (int)strlen( m_linechar ) );
			if( ret ){
				DbgOut( L"MQOFile : ReadObject : newobj SetParams error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}			
		}
	}
	
	*nextstate = BEGIN_FINDCHUNK;

	return 0;

}

int CMQOFile::ReadBVertex( MQOSTATE* nextstate )
{
	int ret;
	int getleng;
	int vertnum;

	_ASSERT( currentobj );

	ret = GetLine( &getleng );
	if( ret ){
		_ASSERT( 0 );
		return ret;
	}

	ret = currentobj->SetVertex( &vertnum, m_linechar, (int)strlen( m_linechar ) );
	if( ret ){
		DbgOut( L"MQOFile : ReadBVertex : SetVertex error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}

	unsigned char* dataptr;
	ret = GetBytes( &dataptr, vertnum * sizeof( VEC3F ) );
	if( ret ){
		DbgOut( L"MQOFile : ReadBVertex : GetBytes error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}

	ret = currentobj->SetPointBuf( dataptr, vertnum );
	if( ret ){
		DbgOut( L"MQOFile : ReadBVertex : SetPointBuf error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}


	*nextstate = BEGIN_FINDCHUNK;

	return 0;
}

int CMQOFile::ReadVertex( MQOSTATE* nextstate )
{
	int ret;
	int vertnum;

	_ASSERT( currentobj );

	ret = currentobj->SetVertex( &vertnum, m_linechar, (int)strlen( m_linechar ) );
	if( ret ){
		DbgOut( L"MQOFile : ReadVertex : SetVertex error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}

	int findend = 0;
	int getleng;
	int vertno = 0;
	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;
		
		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
		}else{
			if( !currentobj || (vertno >= vertnum) ){
				DbgOut( L"MQOFile : ReadVertex : vertno error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}

			ret = currentobj->SetPointBuf( vertno, m_linechar, getleng );
			if( ret ){
				DbgOut( L"MQOFile : ReadVertex : SetPointBuf error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}
			
			vertno++;
		}
	}


	*nextstate = BEGIN_FINDCHUNK;

	return 0;
}


int CMQOFile::ReadFace( MQOSTATE* nextstate )
{
	int ret;
	int facenum;

	_ASSERT( currentobj );
	ret = currentobj->SetFace( &facenum, m_linechar, (int)strlen( m_linechar ) );
	if( ret ){
		DbgOut( L"MQOFile : ReadFace : SetFace error !!!" );
		_ASSERT( 0 );
		*nextstate = BEGIN_FINISH;
		return 1;
	}


	int findend = 0;
	int getleng;
	int faceno = 0;
	while( findend == 0 ){
		ret = GetLine( &getleng );
		if( ret )
			return ret;
		
		if( (getleng >= 3) && (strstr( m_linechar, "}\r\n" ) != NULL) ){
			findend = 1;
		}else{
			if( !currentobj || (faceno >= facenum) ){
				DbgOut( L"MQOFile : ReadFace : faceno error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}

			int materialoffset = 0;
			ret = currentobj->SetFaceBuf( faceno, m_linechar, getleng, materialoffset );
			if( ret ){
				DbgOut( L"MQOFile : ReadFace : SetFaceBuf error !!!" );
				_ASSERT( 0 );
				*nextstate = BEGIN_FINISH;
				return 1;
			}
			
			faceno++;
		}
	}



	*nextstate = BEGIN_FINDCHUNK;

	return 0;
}


int CMQOFile::Multiple()
{
	map<int,CMQOObject*>::iterator itr;
	for( itr = m_modelptr->GetMqoObjectBegin(); itr != m_modelptr->GetMqoObjectEnd(); itr++ ){
		CMQOObject* curobj = itr->second;
		if( curobj ){
			CallF( curobj->MultMat( m_offsetmat ), return 1 );
			CallF( curobj->MultVertex(), return 1; );
		}
	}

	return 0;
}
