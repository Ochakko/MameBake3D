#ifndef RMENU1H
#define RMENU1H

class CRMenuMain
{
public:
	CRMenuMain( int srcmenuid );
	~CRMenuMain();

	int Create( HWND srchwnd );
	int Destroy();

	HMENU GetSubMenu();

	int TrackPopupMenu(POINT pt);

private:
	int InitParams();
	int CheckLevelMenu( int checkid );
	int Params2Dlg();

private:
	HWND m_menuwnd;
	HMENU m_rmenu;
	HMENU m_rsubmenu;
	DWORD m_menuid;

};

#endif