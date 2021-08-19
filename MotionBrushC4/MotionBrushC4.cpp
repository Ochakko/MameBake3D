// MotionBrushC4.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "MotionBrushC4.h"
#include "LocateMBDlg.h"

//#include <iostream.h>
#include <Shlwapi.h>

#include <crtdbg.h>

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

//static
static STARTUPINFO si1;
static PROCESS_INFORMATION pi1;
static STARTUPINFO si2;
static PROCESS_INFORMATION pi2;
static STARTUPINFO si3;
static PROCESS_INFORMATION pi3;
static STARTUPINFO si4;
static PROCESS_INFORMATION pi4;


// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOTIONBRUSHC4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);


    //HWND desktopwnd;
    //desktopwnd = ::GetDesktopWindow();
    //if (desktopwnd) {
    //    RECT desktoprect;
    //    ::GetClientRect(desktopwnd, &desktoprect);
    //    //(1216 + 450) * 2 + 16, (950) * 2 + 62
    //    DWORD neededw = (1216 + 450) * 2 + 16;
    //    DWORD neededh = (950) * 2 + 62;
    //    if ((desktoprect.right < neededw) && (desktoprect.bottom < neededh)) {
    //        WCHAR strmessage[MAX_PATH];
    //        swprintf_s(strmessage, MAX_PATH, L"デスクトップの解像度が(%d x %d)以上の場合に起動を試みます。\n終了します。", neededw, neededh);
    //        MessageBox(NULL, strmessage, L"解像度の条件", MB_OK);
    //        return 0;
    //    }
    //}



    ::SetPriorityClass(GetModuleHandle(NULL), REALTIME_PRIORITY_CLASS);
    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);


    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOTIONBRUSHC4));


    //HWND window = CreateWindowEx(
    //    WS_EX_LEFT, WINDOWS_CLASS_NAME, strwindowname,
    //    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    //    //CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    //    0, 0, (1216 + 450), 950,
    //    NULL, s_mainmenu, (HINSTANCE)GetModuleHandle(NULL), NULL
    //);
    //if (!window)
    //{
    //    OutputDebugString(TEXT("Error: ウィンドウが作成できません。\n"));
    //    return NULL;
    //}






    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MOTIONBRUSHC4);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   ZeroMemory(&si1, sizeof(si1));
   si1.cb = sizeof(si1);
   ZeroMemory(&si2, sizeof(si2));
   si2.cb = sizeof(si2);
   ZeroMemory(&si3, sizeof(si3));
   si3.cb = sizeof(si3);
   ZeroMemory(&si4, sizeof(si4));
   si4.cb = sizeof(si4);
   ZeroMemory(&pi1, sizeof(pi1));
   ZeroMemory(&pi2, sizeof(pi2));
   ZeroMemory(&pi3, sizeof(pi3));
   ZeroMemory(&pi4, sizeof(pi4));


   WCHAR path[MAX_PATH] = { 0L };

   WCHAR path1[MAX_PATH] = { 0L };
   wcscpy_s(path1, MAX_PATH, L".\\MotionBrush.exe");
   WCHAR path2[MAX_PATH] = { 0L };
   wcscpy_s(path2, MAX_PATH, L"..\\MameBake3D\\x64\\Release\\MameBake3D.exe");
   WCHAR path3[MAX_PATH] = { 0L };
   wcscpy_s(path3, MAX_PATH, L"..\\MameBake3D\\x64\\Debug\\MameBake3D.exe");
   WCHAR path4[MAX_PATH] = { 0L };
   wcscpy_s(path4, MAX_PATH, L"..\\MameBake3D\\MameBake3D.exe");



   CLocateMBDlg dlg;
   UINT result;
   result = (UINT)dlg.DoModal();
   if (result != IDOK) {
       return FALSE;
   }



   BOOL bexist1;
   BOOL bexist2;
   BOOL bexist3;
   BOOL bexist4;
   bexist1 = PathFileExists(path1);
   if (bexist1)
   {
       wcscpy_s(path, MAX_PATH, path1);
   }
   else {
       bexist2 = PathFileExists(path2);
       if (bexist2)
       {
           wcscpy_s(path, MAX_PATH, path2);
       }
       else {
           bexist3 = PathFileExists(path3);
           if (bexist3)
           {
               wcscpy_s(path, MAX_PATH, path3);
           }
           else {
               bexist4 = PathFileExists(path4);
               if (bexist4)
               {
                   wcscpy_s(path, MAX_PATH, path4);
               }
               else {
                   _ASSERT(0);
                   return FALSE;
               }
           }
       }
   }

   //HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   HWND hWnd = 0;
   if (dlg.m_w2 && dlg.m_h2) {
       hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
           0, 0, (1216 + 450) * 2 + 16, (950) * 2 + 62, nullptr, nullptr, hInstance, nullptr);
   }
   else if (dlg.m_w2) {
       hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
           0, 0, (1216 + 450) * 2 + 16, (950) + 62, nullptr, nullptr, hInstance, nullptr);
   }
   else if (dlg.m_h2) {
       hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
           0, 0, (1216 + 450) + 16, (950) * 2 + 62, nullptr, nullptr, hInstance, nullptr);
   }
   else {
       return FALSE;
   }
   //window = CreateWindowEx(
   //    WS_EX_LEFT, WINDOWS_CLASS_NAME, strwindowname,
   //    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
   //    //CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
   //    0, 0, (1216 + 450), 950,
   //    NULL, s_mainmenu, (HINSTANCE)GetModuleHandle(NULL), NULL
   //);
   //if (!window)
   //{
   //    OutputDebugString(TEXT("Error: ウィンドウが作成できません。\n"));
   //    return NULL;
   //}
   if (!hWnd)
   {
      _ASSERT(0);
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //STARTUPINFO si1;
   //PROCESS_INFORMATION pi1;
   //ZeroMemory(&pi1, sizeof(pi1));
   WCHAR strcmdline1[MAX_PATH] = { 0L };
   swprintf_s(strcmdline1, MAX_PATH, L"\"%s\" -progno 1", path);
   //si1.dwX = 0;
   //si1.dwY = 0;
   // Start the child process. 
   if (!CreateProcess(NULL,   // No module name (use command line)
       strcmdline1,        // Command line
       NULL,           // Process handle not inheritable
       NULL,           // Thread handle not inheritable
       FALSE,          // Set handle inheritance to FALSE
       0,              // No creation flags
       NULL,           // Use parent's environment block
       NULL,           // Use parent's starting directory 
       &si1,            // Pointer to STARTUPINFO structure
       &pi1)           // Pointer to PROCESS_INFORMATION structure
       )
   {
       wprintf(L"CreateProcess failed (%d).\n", GetLastError());
       _ASSERT(0);
       return FALSE;
   }
   //// Wait until child process exits.
   //WaitForSingleObject(pi1.hProcess, INFINITE);
   //// Close process and thread handles. 
   //CloseHandle(pi1.hProcess);
   //CloseHandle(pi1.hThread);

   if (dlg.m_w2) {
       //STARTUPINFO si2;
       //PROCESS_INFORMATION pi2;
       //ZeroMemory(&si2, sizeof(si2));
       //si2.cb = sizeof(si2);
       //ZeroMemory(&pi2, sizeof(pi2));
       WCHAR strcmdline2[MAX_PATH] = { 0L };
       swprintf_s(strcmdline2, MAX_PATH, L"\"%s\" -progno 2", path);
       //si2.dwX = (1216 + 450);
       //si2.dwY = 0;
       // Start the child process. 
       if (!CreateProcess(NULL,   // No module name (use command line)
           strcmdline2,        // Command line
           NULL,           // Process handle not inheritable
           NULL,           // Thread handle not inheritable
           FALSE,          // Set handle inheritance to FALSE
           0,              // No creation flags
           NULL,           // Use parent's environment block
           NULL,           // Use parent's starting directory 
           &si2,            // Pointer to STARTUPINFO structure
           &pi2)           // Pointer to PROCESS_INFORMATION structure
           )
       {
           wprintf(L"CreateProcess failed (%d).\n", GetLastError());
           _ASSERT(0);
           return FALSE;
       }
       //// Wait until child process exits.
       //WaitForSingleObject(pi2.hProcess, INFINITE);
       //// Close process and thread handles. 
       //CloseHandle(pi2.hProcess);
       //CloseHandle(pi2.hThread);
   }

   if (dlg.m_h2) {
       //STARTUPINFO si3;
       //PROCESS_INFORMATION pi3;
       //ZeroMemory(&si3, sizeof(si3));
       //si3.cb = sizeof(si3);
       //ZeroMemory(&pi3, sizeof(pi3));
       WCHAR strcmdline3[MAX_PATH] = { 0L };
       swprintf_s(strcmdline3, MAX_PATH, L"\"%s\" -progno 3", path);
       //si3.dwX = 0;
       //si3.dwY = 950;
       // Start the child process. 
       if (!CreateProcess(NULL,   // No module name (use command line)
           strcmdline3,        // Command line
           NULL,           // Process handle not inheritable
           NULL,           // Thread handle not inheritable
           FALSE,          // Set handle inheritance to FALSE
           0,              // No creation flags
           NULL,           // Use parent's environment block
           NULL,           // Use parent's starting directory 
           &si3,            // Pointer to STARTUPINFO structure
           &pi3)           // Pointer to PROCESS_INFORMATION structure
           )
       {
           wprintf(L"CreateProcess failed (%d).\n", GetLastError());
           _ASSERT(0);
           return FALSE;
       }
       //// Wait until child process exits.
       //WaitForSingleObject(pi3.hProcess, INFINITE);
       //// Close process and thread handles. 
       //CloseHandle(pi3.hProcess);
       //CloseHandle(pi3.hThread);
   }

   if (dlg.m_w2 && dlg.m_h2) {
       //STARTUPINFO si4;
       //PROCESS_INFORMATION pi4;
       //ZeroMemory(&si4, sizeof(si4));
       //si4.cb = sizeof(si4);
       //ZeroMemory(&pi4, sizeof(pi4));
       WCHAR strcmdline4[MAX_PATH] = { 0L };
       swprintf_s(strcmdline4, MAX_PATH, L"\"%s\" -progno 4", path);
       //si4.dwX = 0;
       //si4.dwY = 0;
       // Start the child process. 
       if (!CreateProcess(NULL,   // No module name (use command line)
           strcmdline4,        // Command line
           NULL,           // Process handle not inheritable
           NULL,           // Thread handle not inheritable
           FALSE,          // Set handle inheritance to FALSE
           0,              // No creation flags
           NULL,           // Use parent's environment block
           NULL,           // Use parent's starting directory 
           &si4,            // Pointer to STARTUPINFO structure
           &pi4)           // Pointer to PROCESS_INFORMATION structure
           )
       {
           wprintf(L"CreateProcess failed (%d).\n", GetLastError());
           _ASSERT(0);
           return FALSE;
       }
       //// Wait until child process exits.
       //WaitForSingleObject(pi4.hProcess, INFINITE);
       //// Close process and thread handles. 
       //CloseHandle(pi4.hProcess);
       //CloseHandle(pi4.hThread);
   }

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //// Wait until child process exits.
        //WaitForSingleObject(pi4.hProcess, INFINITE);
        //// Close process and thread handles. 
        //CloseHandle(pi4.hProcess);
        //CloseHandle(pi4.hThread);

        if (pi1.hProcess) {
            TerminateProcess(pi1.hProcess, 0);
            WaitForSingleObject(pi1.hProcess, INFINITE);
            CloseHandle(pi1.hProcess);
            CloseHandle(pi1.hThread);
        }
        pi1.hProcess = 0;
        pi1.hThread = 0;

        if (pi2.hProcess) {
            TerminateProcess(pi2.hProcess, 0);
            WaitForSingleObject(pi2.hProcess, INFINITE);
            CloseHandle(pi2.hProcess);
            CloseHandle(pi2.hThread);
        }
        pi2.hProcess = 0;
        pi2.hThread = 0;

        if (pi3.hProcess) {
            TerminateProcess(pi3.hProcess, 0);
            WaitForSingleObject(pi3.hProcess, INFINITE);
            CloseHandle(pi3.hProcess);
            CloseHandle(pi3.hThread);
        }
        pi3.hProcess = 0;
        pi3.hThread = 0;


        if (pi4.hProcess) {
            TerminateProcess(pi4.hProcess, 0);
            WaitForSingleObject(pi4.hProcess, INFINITE);
            CloseHandle(pi4.hProcess);
            CloseHandle(pi4.hThread);
        }
        pi4.hProcess = 0;
        pi4.hThread = 0;

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
