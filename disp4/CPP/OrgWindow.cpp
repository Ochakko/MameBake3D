
#include <crtdbg.h>
#include "OrgWindow.h"

#include <windows.h>
#include <list>
#include <map>

using namespace std;

namespace OrgWinGUI{

	////////////////----------------------------------------////////////////
	////////////////										////////////////
	////////////////		ウィンドウ内部品クラス			////////////////
	////////////////										////////////////
	////////////////----------------------------------------////////////////

	/// Method : 再描画要求を送る
	void OrgWindowParts::callRewrite(){
		if( parentWindow==NULL ) return;

		//再描画領域
		RECT tmpRect;
		tmpRect.left=   pos.x+1;
		tmpRect.top=    pos.y+1;
		tmpRect.right=  pos.x+size.x-1;
		tmpRect.bottom= pos.y+size.y-1;
		InvalidateRect( parentWindow->getHWnd(), &tmpRect, false );
	}

	///<summary>
	///	ウィンドウ内部品"タイムラインテーブル"クラス
	///</summary>
	const double OWP_Timeline::LineData::TIME_ERROR_WIDTH= 0.0001;

	////////////////----------------------------------------////////////////
	////////////////										////////////////
	////////////////		オリジナルウィンドウクラス		////////////////
	////////////////										////////////////
	////////////////----------------------------------------////////////////

	////////////////////////// MemberVar /////////////////////////////

	//	MemberVar : ウィンドウハンドルとOrgWindowインスタンスポインタの対応マップ
	map<HWND,OrgWindow*> OrgWindow::hWndAndClassMap;

	//////////////////////////// Method //////////////////////////////

	//
	//	Method : ウィンドウプロシージャ
	//
	LRESULT CALLBACK OrgWindow::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
		map<HWND,OrgWindow*>::iterator owItr;
		owItr= hWndAndClassMap.find(hwnd);
		if( owItr==hWndAndClassMap.end() ){
			return (DefWindowProc(hwnd,message,wParam,lParam));
		}
		OrgWindow *owner= owItr->second;	//ウィンドウ作成元のクラスのインスタンスポインタ


		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;
		switch (message){				//イベント作成
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_MOUSEMOVE:
				POINT tmpPoint;
				GetCursorPos(&tmpPoint);

				mouseEvent.globalX= (int)tmpPoint.x;
				mouseEvent.globalY= (int)tmpPoint.y;
				mouseEvent.localX= mouseEvent.globalX- owner->pos.x;
				mouseEvent.localY= mouseEvent.globalY- owner->pos.y;
				mouseEvent.shiftKey= GetKeyState(VK_SHIFT)<0;
				mouseEvent.ctrlKey= GetKeyState(VK_CONTROL)<0;
				mouseEvent.altKey= GetKeyState(VK_MENU)<0;
				break;
			case WM_KEYDOWN:
			case WM_KEYUP:
				keyboardEvent.keyCode= (int)(unsigned int)wParam;
				keyboardEvent.shiftKey= GetKeyState(VK_SHIFT)<0;
				keyboardEvent.ctrlKey= GetKeyState(VK_CONTROL)<0;
				keyboardEvent.altKey= GetKeyState(VK_MENU)<0;
				keyboardEvent.repeat= (lParam&0x40000000)!=0;
				if( message==WM_KEYUP ){
					keyboardEvent.repeat=false;
					keyboardEvent.onDown=false;
				}else{
					keyboardEvent.onDown=true;
				}
				break;
		}

		switch (message){
			case WM_LBUTTONDOWN:		//左マウスボタンダウン
				owner->onLButtonDown(mouseEvent);
				return 0;
			case WM_LBUTTONUP:			//左マウスボタンアップ
				owner->onLButtonUp(mouseEvent);
				return 0;
			case WM_RBUTTONDOWN:		//右マウスボタンダウン
				owner->onRButtonDown(mouseEvent);
				return 0;
			case WM_RBUTTONUP:			//右マウスボタンアップ
				owner->onRButtonUp(mouseEvent);
				return 0;
			case WM_MOUSEMOVE:			//マウス移動
				owner->onMouseMove(mouseEvent);
				return 0;
			case WM_KEYDOWN:			//キーボードイベント
			case WM_KEYUP:
				owner->onKeyboard(keyboardEvent);
				return 0;
			case WM_MOVE:				//位置変更
				owner->refreshPosAndSize();
				return 0;
			case WM_SIZE:				//サイズ変更
				owner->refreshPosAndSize();
				owner->callRewrite();
				owner->nowChangingSize= true;
				return 0;
			case WM_PAINT:				//描画
				owner->allPaint();
				return 0;
			case WM_CLOSE:
				DestroyWindow(hwnd);
				return 0;
			case WM_DESTROY:
				//PostQuitMessage(0);
				return 0;
			default:
				if( owner->nowChangingSize ){
					owner->nowChangingSize= false;
					owner->autoResizeAllParts();
				}
				return (DefWindowProc(hwnd,message,wParam,lParam));
		}

		return 0;
	}

}