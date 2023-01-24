#ifndef SETDLGPOSH
#define SETDLGPOSH


#ifdef DECSETDLGPOSH
int SetDlgPosToDesktopCenter(HWND hDlgWnd, HWND hWndInsertAfter);
int SetDlgPosToCursorPos(HWND hDlgWnd, HWND hWndInsertAfter);
#else
extern int SetDlgPosToDesktopCenter(HWND hDlgWnd, HWND hWndInsertAfter);
extern int SetDlgPosToCursorPos(HWND hDlgWnd, HWND hWndInsertAfter);
#endif


#endif