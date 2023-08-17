#ifndef LIGHTSFOREDITFILEH
#define LIGHTSFOREDITFILEH

#include <ChaVecCalc.h>
#include <coef.h>
#include <XMLIO.h>

#include <vector>
#include <string>
#include <map>


class CLightsForEditFile : public CXMLIO
{
public:
	CLightsForEditFile();
	virtual ~CLightsForEditFile();

	int WriteLightsForEditFile(const WCHAR* srcfilepath);
	int LoadLightsForEditFile(const WCHAR* srcfilepath);

private:
	virtual int InitParams();
	virtual int DestroyObjs();

	int WriteFileInfo();
	int WriteLight(int colorindex);

	//int CheckFileVersion( XMLIOBUF* xmliobuf );
	//int ReadFileInfo( XMLIOBUF* xmliobuf, int* charanumptr );
	int ReadLight(int lightcnt, XMLIOBUF* xmlbuf);

};

#endif