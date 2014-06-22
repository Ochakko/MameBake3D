#ifndef EDITRANGEH
#define EDITRANGEH

#include <list>
#include <orgwindow.h>

class CEditRange
{
public:
	CEditRange();
	~CEditRange();

	int Clear();
	int SetRange( std::list<KeyInfo> srcki, double srcframe );
	int GetRange( int* numptr, double* startptr, double* endptr );
	int GetRange( int* numptr, double* startptr, double* endptr, double* applyptr );

private:
	int InitParams();
	int DestroyObjs();

public:
	std::list<KeyInfo> m_ki;
	int m_keynum;
	double m_startframe;
	double m_endframe;
	double m_applyframe;

	static double s_applyrate;
};

#endif



