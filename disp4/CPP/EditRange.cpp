#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>
#include <crtdbg.h>

#define DBGH
#include <dbg.h>

#include <EditRange.h>

using namespace std;



double CEditRange::s_applyrate = 100.0;


CEditRange::CEditRange()
{
	InitParams();
}
CEditRange::~CEditRange()
{
	DestroyObjs();
}

int CEditRange::InitParams()
{
	m_ki.clear();
	m_keynum = 0;
	m_startframe = 0.0;
	m_endframe = 0.0;
	m_applyframe = 0.0;
	return 0;
}
int CEditRange::DestroyObjs()
{
	m_ki.clear();
	return 0;
}

int CEditRange::Clear()
{
	m_ki.clear();

	m_keynum = 0;
	m_startframe = 0.0;
	m_endframe = 0.0;

	return 0;
}
int CEditRange::SetRange( std::list<KeyInfo> srcki, double srcframe )
{
	double minframe = 1e10;
	double maxframe = -1e10;

	m_ki = srcki;

	int num = 0;
	list<KeyInfo>::iterator itrki;
	for( itrki = m_ki.begin(); itrki != m_ki.end(); itrki++ ){
		if( itrki->lineIndex == 1 ){
			double curframe = itrki->time;
			if( minframe > curframe ){
				minframe = curframe;
			}
			if( maxframe < curframe ){
				maxframe = curframe;
			}
			num++;
		}
	}

	m_keynum = num;
	if( m_keynum >= 1 ){
		m_startframe = minframe;
		m_endframe = maxframe;
	}else{
		m_startframe = srcframe;
		m_endframe = srcframe;
	}

	return 0;
}
int CEditRange::GetRange( int* numptr, double* startptr, double* endptr )
{
	*numptr = m_keynum;
	*startptr = m_startframe;
	*endptr = m_endframe;
	return 0;
}

int CEditRange::GetRange( int* numptr, double* startptr, double* endptr, double* applyptr )
{
	*numptr = m_keynum;
	*startptr = m_startframe;
	*endptr = m_endframe;

	int offset = 0;
	if (s_applyrate != 0.0){
		offset = 1.0;
	}
	m_applyframe = (double)( (int)( m_startframe + (m_endframe - m_startframe) * s_applyrate / 100.0 + offset ) );
	*applyptr = m_applyframe;

	return 0;
}

