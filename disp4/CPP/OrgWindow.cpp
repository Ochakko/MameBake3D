
#include <crtdbg.h>
#include "OrgWindow.h"

#include <windows.h>
#include <list>
#include <map>

using namespace std;

namespace OrgWinGUI{

	////////////////----------------------------------------////////////////
	////////////////										////////////////
	////////////////		�E�B���h�E�����i�N���X			////////////////
	////////////////										////////////////
	////////////////----------------------------------------////////////////

	/// Method : �ĕ`��v���𑗂�
	void OrgWindowParts::callRewrite(){
		if( parentWindow==NULL ) return;

		//�ĕ`��̈�
		RECT tmpRect;
		tmpRect.left=   pos.x+1;
		tmpRect.top=    pos.y+1;
		tmpRect.right=  pos.x+size.x-1;
		tmpRect.bottom= pos.y+size.y-1;
		InvalidateRect( parentWindow->getHWnd(), &tmpRect, false );
	}

	///<summary>
	///	�E�B���h�E�����i"�^�C�����C���e�[�u��"�N���X
	///</summary>
	const double OWP_Timeline::LineData::TIME_ERROR_WIDTH= 0.0001;

	////////////////----------------------------------------////////////////
	////////////////										////////////////
	////////////////		�I���W�i���E�B���h�E�N���X		////////////////
	////////////////										////////////////
	////////////////----------------------------------------////////////////

	////////////////////////// MemberVar /////////////////////////////

	//	MemberVar : �E�B���h�E�n���h����OrgWindow�C���X�^���X�|�C���^�̑Ή��}�b�v
	map<HWND,OrgWindow*> OrgWindow::hWndAndClassMap;

	//////////////////////////// Method //////////////////////////////

	//
	//	Method : �E�B���h�E�v���V�[�W��
	//
	LRESULT CALLBACK OrgWindow::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
		map<HWND,OrgWindow*>::iterator owItr;
		owItr= hWndAndClassMap.find(hwnd);
		if( owItr==hWndAndClassMap.end() ){
			return (DefWindowProc(hwnd,message,wParam,lParam));
		}
		OrgWindow *owner= owItr->second;	//�E�B���h�E�쐬���̃N���X�̃C���X�^���X�|�C���^


		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;
		switch (message){				//�C�x���g�쐬
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
			case WM_LBUTTONDOWN:		//���}�E�X�{�^���_�E��
				owner->onLButtonDown(mouseEvent);
				return 0;
			case WM_LBUTTONUP:			//���}�E�X�{�^���A�b�v
				owner->onLButtonUp(mouseEvent);
				return 0;
			case WM_RBUTTONDOWN:		//�E�}�E�X�{�^���_�E��
				owner->onRButtonDown(mouseEvent);
				return 0;
			case WM_RBUTTONUP:			//�E�}�E�X�{�^���A�b�v
				owner->onRButtonUp(mouseEvent);
				return 0;
			case WM_MOUSEMOVE:			//�}�E�X�ړ�
				owner->onMouseMove(mouseEvent);
				return 0;
			case WM_KEYDOWN:			//�L�[�{�[�h�C�x���g
			case WM_KEYUP:
				owner->onKeyboard(keyboardEvent);
				return 0;
			case WM_MOVE:				//�ʒu�ύX
				owner->refreshPosAndSize();
				return 0;
			case WM_SIZE:				//�T�C�Y�ύX
				owner->refreshPosAndSize();
				owner->callRewrite();
				owner->nowChangingSize= true;
				return 0;
			case WM_PAINT:				//�`��
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