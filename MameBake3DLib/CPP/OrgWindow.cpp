#include "stdafx.h"

#include <crtdbg.h>
#include <OrgWindow.h>

#include <windows.h>
#include <list>
#include <map>

#include "GlobalVar.h"


using namespace std;

extern std::vector<void*> g_eulpool;//allocate MPPOOLBLKLEN motoinpoints at onse and pool 

namespace OrgWinGUI{

	////////////////----------------------------------------////////////////
	////////////////										////////////////
	////////////////		ウィンドウ内部品クラス			////////////////
	////////////////										////////////////
	////////////////----------------------------------------////////////////

	void OrgWindow::allPaint() {
		//static int s_paintcnt = 0;
		//s_paintcnt++;
		//if (g_previewFlag != 0) {
		//	if ((s_paintcnt % 60) != 0) {
		//		return;
		//	}
		//}

		beginPaint();
		paintTitleBar();
		for (std::list<OrgWindowParts*>::iterator itr = partsList.begin();
			itr != partsList.end();
			itr++) {
			if ((*itr)->getParent() && IsWindow((*itr)->getParent()->hWnd)) {
				(*itr)->draw();
			}
		}


		//{
		//	//if (g_dsmousewait == 1) {
		//	if (g_mouseherebmp) {
		//		POINT mousepoint;
		//		::GetCursorPos(&mousepoint);
		//		::ScreenToClient(hWnd, &mousepoint);

		//		//PAINTSTRUCT ps;
		//		//HDC hdc = BeginPaint(hWnd, &ps);
		//		// メモリデバイスコンテキストを作成する
		//		HDC hCompatDC = CreateCompatibleDC(hdcM.hDC);
		//		// ロードしたビットマップを選択する
		//		HBITMAP hPrevBitmap = (HBITMAP)SelectObject(hCompatDC, g_mouseherebmp);
		//		BITMAP bmp;
		//		GetObject(g_mouseherebmp, sizeof(BITMAP), &bmp);
		//		int BMP_W = (int)bmp.bmWidth;
		//		int BMP_H = (int)bmp.bmHeight;
		//		BitBlt(hdcM.hDC, mousepoint.x, mousepoint.y, BMP_W, BMP_H, hCompatDC, 0, 0, SRCCOPY);
		//		DeleteDC(hCompatDC);
		//		//EndPaint(hWnd, &ps);
		//	}
		//	//}
		//}


		//{
		//	//if (g_dsmousewait == 1) {
		//	POINT mousepoint;
		//	::GetCursorPos(&mousepoint);
		//	//if (getParent()) {
		//		::ScreenToClient(getHWnd(), &mousepoint);
		//		PAINTSTRUCT ps;
		//		HDC hdc = BeginPaint(getHWnd(), &ps);
		//		// メモリデバイスコンテキストを作成する
		//		HDC hCompatDC = CreateCompatibleDC(hdc);
		//		// ロードしたビットマップを選択する
		//		HBITMAP hPrevBitmap = (HBITMAP)SelectObject(hCompatDC, g_mouseherebmp);
		//		BITMAP bmp;
		//		GetObject(g_mouseherebmp, sizeof(BITMAP), &bmp);
		//		int BMP_W = (int)bmp.bmWidth;
		//		int BMP_H = (int)bmp.bmHeight;
		//		BitBlt(hdc, mousepoint.x, mousepoint.y, BMP_W, BMP_H, hCompatDC, 0, 0, SRCCOPY);
		//		DeleteDC(hCompatDC);
		//		//EndPaint(hWnd, &ps);
		//	//}
		//	//}
		//}

		endPaint();
	}

	void OrgWindowParts::draw()
	{
		drawEdge();
		{
			if (g_dsmousewait == 1) {
				POINT mousepoint;
				::GetCursorPos(&mousepoint);
				if (getParent() && getHDCMaster()) {
					::ScreenToClient(getParent()->getHWnd(), &mousepoint);
					int BMP_W = 52;
					int BMP_H = 50;
					Gdiplus::Graphics* gdipg = new Gdiplus::Graphics(hdcM->hDC);
					if (gdipg) {
						Gdiplus::ImageAttributes attr;
						Gdiplus::ColorMatrix cmat = {
							1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Red
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // Green
							0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Blue
							0.0f, 0.0f, 0.0f, g_mouseherealpha, 0.0f,   // Alpha (70%)
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f    // must be 1
						};
						attr.SetColorMatrix(&cmat);
						//Gdiplus::Image* imgptr = new Gdiplus::Image(L"E:\\PG\\MameBake3D_git\\MameBake3D\\Media\\MameMedia\\img_l105.png");
						if(g_mousehereimage){
							gdipg->DrawImage(g_mousehereimage, Gdiplus::Rect(mousepoint.x, mousepoint.y, BMP_W, BMP_H),
								0, 0, BMP_W, BMP_H,
								Gdiplus::UnitPixel, &attr, NULL, NULL);
						}
						delete gdipg;
					}
				}
			}
		}
		
	}

	void OWP_Separator::draw() {
		//static int s_paintcnt = 0;
		//s_paintcnt++;
		//if (g_previewFlag != 0) {
		//	if ((s_paintcnt % 2) != 0) {
		//		return;
		//	}
		//}

		//if (g_underloading == true) {
		//	return;
		//}

		//枠を書く
		if (hdcM) {
			int centerPos = getCenterLinePos();
			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
			if (divideSide) {
				MoveToEx(hdcM->hDC, pos.x + centerPos, pos.y + 1, NULL);
				LineTo(hdcM->hDC, pos.x + centerPos, pos.y + size.y - 1);
				if (canShift) {
					hdcM->setPenAndBrush(RGB(240, 240, 240), NULL);
					int centerPos2 = pos.y + size.y / 2;
					MoveToEx(hdcM->hDC, pos.x + centerPos, centerPos2 - HANDLE_MARK_SIZE / 2, NULL);
					LineTo(hdcM->hDC, pos.x + centerPos, centerPos2 + HANDLE_MARK_SIZE / 2);
				}
			}
			else {
				MoveToEx(hdcM->hDC, pos.x + 1, pos.y + centerPos, NULL);
				LineTo(hdcM->hDC, pos.x + size.x - 1, pos.y + centerPos);
				if (canShift) {
					hdcM->setPenAndBrush(RGB(240, 240, 240), NULL);
					int centerPos2 = pos.x + size.x / 2;
					MoveToEx(hdcM->hDC, centerPos2 - HANDLE_MARK_SIZE / 2, pos.y + centerPos, NULL);
					LineTo(hdcM->hDC, centerPos2 + HANDLE_MARK_SIZE / 2, pos.y + centerPos);
				}
			}

			//全ての内部パーツを描画
			for (std::list<OrgWindowParts*>::iterator itr = partsList1.begin();
				itr != partsList1.end(); itr++) {
				if(*itr){
					(*itr)->draw();
				}
			}
			for (std::list<OrgWindowParts*>::iterator itr = partsList2.begin();
				itr != partsList2.end(); itr++) {
				if (*itr) {
					(*itr)->draw();
				}
			}

			{
				if (g_dsmousewait == 1) {
					POINT mousepoint;
					::GetCursorPos(&mousepoint);
					if (getParent() && getHDCMaster()) {
						::ScreenToClient(getParent()->getHWnd(), &mousepoint);
						int BMP_W = 52;
						int BMP_H = 50;
						Gdiplus::Graphics* gdipg = new Gdiplus::Graphics(hdcM->hDC);
						if (gdipg) {
							Gdiplus::ImageAttributes attr;
							Gdiplus::ColorMatrix cmat = {
								1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Red
								0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // Green
								0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Blue
								0.0f, 0.0f, 0.0f, g_mouseherealpha, 0.0f,   // Alpha (70%)
								0.0f, 0.0f, 0.0f, 0.0f, 1.0f    // must be 1
							};
							attr.SetColorMatrix(&cmat);
							//Gdiplus::Image* imgptr = new Gdiplus::Image(L"E:\\PG\\MameBake3D_git\\MameBake3D\\Media\\MameMedia\\img_l105.png");
							if (g_mousehereimage){
								gdipg->DrawImage(g_mousehereimage, Gdiplus::Rect(mousepoint.x, mousepoint.y, BMP_W, BMP_H),
									0, 0, BMP_W, BMP_H,
									Gdiplus::UnitPixel, &attr, NULL, NULL);
							}

							delete gdipg;
						}
					}
				}
			}

		}


	}

	void OWP_Timeline::LineData::callRewrite()
	{
	}

	void OWP_Timeline::callRewrite()
	{
		if (parentWindow && IsWindow(parentWindow->getHWnd())) {
			RECT tmpRect;
			tmpRect.left = 0;
			tmpRect.top = 0;
			tmpRect.right = size.x;
			tmpRect.bottom = size.y;
			InvalidateRect(parentWindow->getHWnd(), &tmpRect, false);

			//draw();

		}
	}
	void OWP_Timeline::draw() {
		//static int s_paintcnt = 0;
		//s_paintcnt++;
		//if (g_previewFlag != 0) {
		//	if ((s_paintcnt % 2) != 0) {
		//		return;
		//	}
		//}

		drawEdge();

		//時間軸目盛り
		{
			const int AXIS_CURSOR_SIZE = 4;
			const int AXIS_LABEL_SIDE_MARGIN = 7;

			//目盛り線 & ラベル
			int x0 = pos.x + MARGIN + LABEL_SIZE_X;
			int x1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH;
			int y0 = pos.y + MARGIN;
			int y1 = y0 + AXIS_SIZE_Y + 1;
			for (int i = (int)showPos_time; i <= (int)maxTime; i++) {
				int xx = (int)(((double)i - showPos_time)*timeSize) + x0 + 1;

				if (x1 + AXIS_LABEL_SIDE_MARGIN <= xx) break;
				if (x0 - AXIS_LABEL_SIDE_MARGIN <= xx) {
					hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
					MoveToEx(hdcM->hDC, xx, y1 - 5, NULL);
					LineTo(hdcM->hDC, xx, y1);

					if (((i < 1000) && (i % 5 == 0)) || ((i >= 1000) && (i % 10 == 0))) {
						TCHAR tmpChar[20];
						_stprintf_s(tmpChar, 20, _T("%.3G"), (double)i);
						hdcM->setFont(12, _T("ＭＳ ゴシック"));
						SetTextColor(hdcM->hDC, RGB(240, 240, 240));
						TextOut(hdcM->hDC,
							xx - (int)((double)_tcslen(tmpChar)*2.0), y0,
							tmpChar, (int)_tcslen(tmpChar));
					}
				}
			}

			//カーソル
			int xx = (int)((currentTime - showPos_time)*timeSize) + x0 + 1;
			hdcM->setPenAndBrush(RGB(240, 240, 240), NULL);
			if (x0 - AXIS_CURSOR_SIZE <= xx && xx <= x1 + AXIS_CURSOR_SIZE) {
				for (int i = 0; i<AXIS_CURSOR_SIZE; i++) {
					MoveToEx(hdcM->hDC, xx - i, y1 - i - 2, NULL);
					LineTo(hdcM->hDC, xx + i + 1, y1 - i - 2);
				}
			}

			//枠
			hdcM->setPenAndBrush(NULL, RGB(baseColor.r, baseColor.g, baseColor.b));
			Rectangle(hdcM->hDC, pos.x, y0, x0, y1);
			Rectangle(hdcM->hDC, x1, y0, pos.x + size.x, y1);
			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
			Rectangle(hdcM->hDC, x0, y0, x1, y1);
		}

		drawEdge(false);

		//行データ
		int showLineNum = (size.y - SCROLL_BAR_WIDTH - AXIS_SIZE_Y - MARGIN * 2) / (LABEL_SIZE_Y - 1);
		for (int i = showPos_line, j = 0; i<(int)lineData.size() && j<showLineNum; i++, j++) {
			bool highLight = false;
			if (i == currentLine) highLight = true;
			if (i >= 0) {
				lineData[i]->draw(hdcM,
					pos.x + MARGIN,
					pos.y + MARGIN + AXIS_SIZE_Y + j*(LABEL_SIZE_Y - 1),
					size.x - SCROLL_BAR_WIDTH - MARGIN * 2,
					timeSize, showPos_time, highLight);
			}
		}

		//ドラッグによる選択範囲
		if (dragSelect && dragSelectTime1 != dragSelectTime2) {
			int xx0 = pos.x + MARGIN + LABEL_SIZE_X + 1;
			int yy0 = pos.y + MARGIN + AXIS_SIZE_Y;
			int xx1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH - 1;
			int yy1 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH;
			int x0 = xx0 + (int)((min(dragSelectTime1, dragSelectTime2) - showPos_time)* timeSize);
			int x1 = xx0 + (int)((max(dragSelectTime1, dragSelectTime2) - showPos_time)* timeSize);
			int y0 = yy0 + (min(dragSelectLine1, dragSelectLine2) - showPos_line)* (LABEL_SIZE_Y - 1) + 1;
			int y1 = yy0 + (max(dragSelectLine1, dragSelectLine2) - showPos_line + 1)* (LABEL_SIZE_Y - 1) - 1;

			{//枠描画
				hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
				if (xx0 <= x0) {		//左枠
					MoveToEx(hdcM->hDC, x0, max(yy0, y0 + 1), NULL);
					LineTo(hdcM->hDC, x0, min(y1, yy1));
				}
				if (x1 <= xx1) {		//右枠
					MoveToEx(hdcM->hDC, x1, max(yy0, y0 + 1), NULL);
					LineTo(hdcM->hDC, x1, min(y1, yy1));
				}
				if (yy0 <= y0) {		//上枠
					MoveToEx(hdcM->hDC, max(xx0, x0 + 1), y0, NULL);
					LineTo(hdcM->hDC, min(x1, xx1), y0);
				}
				if (y1 <= yy1) {		//下枠
					MoveToEx(hdcM->hDC, max(xx0, x0 + 1), y1, NULL);
					LineTo(hdcM->hDC, min(x1, xx1), y1);
				}
			}
		}

		//時間軸スクロールバー
		{
			int x0 = pos.x + MARGIN + LABEL_SIZE_X;
			int x1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH;
			int y0 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH;
			int y1 = y0 + SCROLL_BAR_WIDTH;

			//枠
			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
			Rectangle(hdcM->hDC, x0, y0, x1, y1);

			//中身
			double showTimeLength = ((double)(x1 - x0 - 3)) / timeSize;
			double barSize = ((double)(x1 - x0 - 4))*showTimeLength / maxTime;
			double barStart = ((double)(x1 - x0 - 4))*showPos_time / maxTime;
			if (showTimeLength<maxTime) {
				hdcM->setPenAndBrush(NULL, RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)));
				Rectangle(hdcM->hDC, x0 + 2 + (int)barStart, y0 + 2, x0 + 2 + (int)(barStart + barSize), y1 - 2);
			}
		}

		//ラベルスクロールバー
		{
			int x0 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH - 1;
			int x1 = x0 + SCROLL_BAR_WIDTH + 1;
			int y0 = pos.y + MARGIN + AXIS_SIZE_Y;
			int y1 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH + 1;

			//枠
			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
			Rectangle(hdcM->hDC, x0, y0, x1, y1);

			//中身
			if (lineData.size() > 0) {
				int barSize = (y1 - y0 - 4) * showLineNum / (int)lineData.size();
				int barStart = (y1 - y0 - 4) * showPos_line / (int)lineData.size();
				if (showLineNum < (int)lineData.size()) {
					hdcM->setPenAndBrush(NULL, RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)));
					Rectangle(hdcM->hDC, x0 + 2, y0 + 2 + barStart, x1 - 2, y0 + 2 + barStart + barSize + 1);
				}
			}
		}

		{
			if (g_dsmousewait == 1) {
				POINT mousepoint;
				::GetCursorPos(&mousepoint);
				if (getParent()) {
					::ScreenToClient(getParent()->getHWnd(), &mousepoint);
					int BMP_W = 52;
					int BMP_H = 50;
					Gdiplus::Graphics* gdipg = new Gdiplus::Graphics(hdcM->hDC);
					if (gdipg) {
						Gdiplus::ImageAttributes attr;
						Gdiplus::ColorMatrix cmat = {
							1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Red
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // Green
							0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Blue
							0.0f, 0.0f, 0.0f, g_mouseherealpha, 0.0f,   // Alpha (70%)
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f    // must be 1
						};
						attr.SetColorMatrix(&cmat);
						//Gdiplus::Image* imgptr = new Gdiplus::Image(L"E:\\PG\\MameBake3D_git\\MameBake3D\\Media\\MameMedia\\img_l105.png");
						if (g_mousehereimage) {
							gdipg->DrawImage(g_mousehereimage, Gdiplus::Rect(mousepoint.x, mousepoint.y, BMP_W, BMP_H),
								0, 0, BMP_W, BMP_H,
								Gdiplus::UnitPixel, &attr, NULL, NULL);
						}
						delete gdipg;
					}
				}
			}
		}
	}

	void OWP_EulerGraph::draw() {
		//static int s_paintcnt = 0;
		//s_paintcnt++;
		//if (g_previewFlag != 0) {
		//	if ((s_paintcnt % 2) != 0) {
		//		return;
		//	}
		//}

		//drawEdge();

		////時間軸目盛り
		//{
		//	const int AXIS_CURSOR_SIZE = 4;
		//	const int AXIS_LABEL_SIDE_MARGIN = 7;

		//	//目盛り線 & ラベル
		//	int x0 = pos.x + MARGIN + LABEL_SIZE_X;
		//	int x1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH;
		//	int y0 = pos.y + MARGIN;
		//	int y1 = y0 + AXIS_SIZE_Y + 1;
		//	for (int i = (int)showPos_time; i <= (int)maxTime; i++) {
		//		int xx = (int)(((double)i - showPos_time)*timeSize) + x0 + 1;

		//		if (x1 + AXIS_LABEL_SIDE_MARGIN <= xx) break;
		//		if (x0 - AXIS_LABEL_SIDE_MARGIN <= xx) {
		//			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
		//			MoveToEx(hdcM->hDC, xx, y1 - 5, NULL);
		//			LineTo(hdcM->hDC, xx, y1);

		//			if (((i < 1000) && (i % 5 == 0)) || ((i >= 1000) && (i % 10 == 0))) {
		//				TCHAR tmpChar[20];
		//				_stprintf_s(tmpChar, 20, _T("%.3G"), (double)i);
		//				hdcM->setFont(12, _T("ＭＳ ゴシック"));
		//				SetTextColor(hdcM->hDC, RGB(240, 240, 240));
		//				TextOut(hdcM->hDC,
		//					xx - (int)((double)_tcslen(tmpChar)*2.0), y0,
		//					tmpChar, (int)_tcslen(tmpChar));
		//			}
		//		}
		//	}

		//	//カーソル
		//	int xx = (int)((currentTime - showPos_time)*timeSize) + x0 + 1;
		//	hdcM->setPenAndBrush(RGB(240, 240, 240), NULL);
		//	if (x0 - AXIS_CURSOR_SIZE <= xx && xx <= x1 + AXIS_CURSOR_SIZE) {
		//		for (int i = 0; i<AXIS_CURSOR_SIZE; i++) {
		//			MoveToEx(hdcM->hDC, xx - i, y1 - i - 2, NULL);
		//			LineTo(hdcM->hDC, xx + i + 1, y1 - i - 2);
		//		}
		//	}

		//	//枠
		//	hdcM->setPenAndBrush(NULL, RGB(baseColor.r, baseColor.g, baseColor.b));
		//	Rectangle(hdcM->hDC, pos.x, y0, x0, y1);
		//	Rectangle(hdcM->hDC, x1, y0, pos.x + size.x, y1);
		//	hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
		//	Rectangle(hdcM->hDC, x0, y0, x1, y1);
		//}

		//drawEdge(false);

		//行データ
		//int showLineNum = (size.y - SCROLL_BAR_WIDTH - AXIS_SIZE_Y - MARGIN * 2) / (LABEL_SIZE_Y - 1);
		//int showLineNum = 3;
		int showLineNum = 4;
		//for (int i = showPos_line, j = 0; i<(int)lineData.size() && j<showLineNum; i++, j++) {
		
		if (lineData.size() >= 4) {//X, Y, Z, Brush

			int drawnum = min(lineData.size(), showLineNum);

			for (int i = 0; i < drawnum; i++) {
				bool highLight = false;
				//if (i == currentLine) highLight = true;
				lineData[i]->draw(hdcM,
					pos.x + MARGIN,
					//pos.y + MARGIN + AXIS_SIZE_Y + j*(LABEL_SIZE_Y - 1),
					pos.y + MARGIN + AXIS_SIZE_Y,
					size.x - SCROLL_BAR_WIDTH - MARGIN * 2,
					timeSize, showPos_time, highLight);
			}
		}


		////motionbrush ０フレームから書く
		//bool highLight = false;
		//lineData[3]->draw(hdcM,
		//pos.x + MARGIN,
		////pos.y + MARGIN + AXIS_SIZE_Y + j*(LABEL_SIZE_Y - 1),
		//pos.y + MARGIN + AXIS_SIZE_Y,
		//size.x - SCROLL_BAR_WIDTH - MARGIN * 2,
		//timeSize, 0.0, highLight);


		////ドラッグによる選択範囲
		//if (dragSelect && dragSelectTime1 != dragSelectTime2) {
		//	int xx0 = pos.x + MARGIN + LABEL_SIZE_X + 1;
		//	int yy0 = pos.y + MARGIN + AXIS_SIZE_Y;
		//	int xx1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH - 1;
		//	int yy1 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH;
		//	int x0 = xx0 + (int)((min(dragSelectTime1, dragSelectTime2) - showPos_time)* timeSize);
		//	int x1 = xx0 + (int)((max(dragSelectTime1, dragSelectTime2) - showPos_time)* timeSize);
		//	int y0 = yy0 + (min(dragSelectLine1, dragSelectLine2) - showPos_line)* (LABEL_SIZE_Y - 1) + 1;
		//	int y1 = yy0 + (max(dragSelectLine1, dragSelectLine2) - showPos_line + 1)* (LABEL_SIZE_Y - 1) - 1;

		//	{//枠描画
		//		hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
		//		if (xx0 <= x0) {		//左枠
		//			MoveToEx(hdcM->hDC, x0, max(yy0, y0 + 1), NULL);
		//			LineTo(hdcM->hDC, x0, min(y1, yy1));
		//		}
		//		if (x1 <= xx1) {		//右枠
		//			MoveToEx(hdcM->hDC, x1, max(yy0, y0 + 1), NULL);
		//			LineTo(hdcM->hDC, x1, min(y1, yy1));
		//		}
		//		if (yy0 <= y0) {		//上枠
		//			MoveToEx(hdcM->hDC, max(xx0, x0 + 1), y0, NULL);
		//			LineTo(hdcM->hDC, min(x1, xx1), y0);
		//		}
		//		if (y1 <= yy1) {		//下枠
		//			MoveToEx(hdcM->hDC, max(xx0, x0 + 1), y1, NULL);
		//			LineTo(hdcM->hDC, min(x1, xx1), y1);
		//		}
		//	}
		//}

		////時間軸スクロールバー
		//{
		//	int x0 = pos.x + MARGIN + LABEL_SIZE_X;
		//	int x1 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH;
		//	int y0 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH;
		//	int y1 = y0 + SCROLL_BAR_WIDTH;

		//	//枠
		//	hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
		//	Rectangle(hdcM->hDC, x0, y0, x1, y1);

		//	//中身
		//	double showTimeLength = ((double)(x1 - x0 - 3)) / timeSize;
		//	double barSize = ((double)(x1 - x0 - 4))*showTimeLength / maxTime;
		//	double barStart = ((double)(x1 - x0 - 4))*showPos_time / maxTime;
		//	if (showTimeLength<maxTime) {
		//		hdcM->setPenAndBrush(NULL, RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)));
		//		Rectangle(hdcM->hDC, x0 + 2 + (int)barStart, y0 + 2, x0 + 2 + (int)(barStart + barSize), y1 - 2);
		//	}
		//}

		////ラベルスクロールバー
		//{
		//	int x0 = pos.x + size.x - MARGIN - SCROLL_BAR_WIDTH - 1;
		//	int x1 = x0 + SCROLL_BAR_WIDTH + 1;
		//	int y0 = pos.y + MARGIN + AXIS_SIZE_Y;
		//	int y1 = pos.y + size.y - MARGIN - SCROLL_BAR_WIDTH + 1;

		//	//枠
		//	hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
		//	Rectangle(hdcM->hDC, x0, y0, x1, y1);

		//	//中身
		//	int barSize = (y1 - y0 - 4)*showLineNum / (int)lineData.size();
		//	int barStart = (y1 - y0 - 4)*showPos_line / (int)lineData.size();
		//	if (showLineNum<(int)lineData.size()) {
		//		hdcM->setPenAndBrush(NULL, RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)));
		//		Rectangle(hdcM->hDC, x0 + 2, y0 + 2 + barStart, x1 - 2, y0 + 2 + barStart + barSize + 1);
		//	}
		//}

		{
			if (g_dsmousewait == 1) {
				POINT mousepoint;
				::GetCursorPos(&mousepoint);
				if (getParent() && getHDCMaster()) {
					::ScreenToClient(getParent()->getHWnd(), &mousepoint);
					int BMP_W = 52;
					int BMP_H = 50;
					Gdiplus::Graphics* gdipg = new Gdiplus::Graphics(hdcM->hDC);
					if (gdipg) {
						Gdiplus::ImageAttributes attr;
						Gdiplus::ColorMatrix cmat = {
							1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Red
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // Green
							0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Blue
							0.0f, 0.0f, 0.0f, g_mouseherealpha, 0.0f,   // Alpha (70%)
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f    // must be 1
						};
						attr.SetColorMatrix(&cmat);
						//Gdiplus::Image* imgptr = new Gdiplus::Image(L"E:\\PG\\MameBake3D_git\\MameBake3D\\Media\\MameMedia\\img_l105.png");
						if (g_mousehereimage) {
							gdipg->DrawImage(g_mousehereimage, Gdiplus::Rect(mousepoint.x, mousepoint.y, BMP_W, BMP_H),
								0, 0, BMP_W, BMP_H,
								Gdiplus::UnitPixel, &attr, NULL, NULL);
						}
						delete gdipg;
					}
					
				}
			}
		}

	}

	//namespace global func
	void* GetNewEulKey()
	{
		static int s_befheadno = -1;
		static int s_befelemno = -1;

		int curpoollen;
		curpoollen = g_eulpool.size();


		if ((s_befheadno != (g_eulpool.size() - 1)) || (s_befelemno != (EULPOOLBLKLEN - 1))) {//前回リリースしたポインタが最後尾ではない場合

		//前回リリースしたポインタの次のメンバーをチェックして未使用だったらリリース
			int chkheadno;
			chkheadno = s_befheadno;
			int chkelemno;
			chkelemno = s_befelemno + 1;
			if ((chkheadno >= 0) && (chkheadno >= curpoollen) && (chkelemno >= EULPOOLBLKLEN)) {
				chkelemno = 0;
				chkheadno++;
			}
			if ((chkheadno >= 0) && (chkheadno < curpoollen) && (chkelemno >= 0) && (chkelemno < EULPOOLBLKLEN)) {
				OWP_EulerGraph::EulLineData::EulKey* cureulhead = (OWP_EulerGraph::EulLineData::EulKey*)g_eulpool[chkheadno];

				if (cureulhead) {
					OWP_EulerGraph::EulLineData::EulKey* chkeul;
					chkeul = cureulhead + chkelemno;
					if (chkeul) {
						if (chkeul->GetUseFlag() == 0) {
							int saveindex = chkeul->GetIndexOfPool();
							int saveallochead = chkeul->IsAllocHead();
							chkeul->InitParams();
							chkeul->SetUseFlag(1);
							chkeul->SetIndexOfPool(saveindex);
							chkeul->SetIsAllocHead(saveallochead);

							s_befheadno = chkheadno;
							s_befelemno = chkelemno;
							return (void*)chkeul;
						}
					}
				}
			}

			//if ((chkheadno >= 0) && (chkheadno < curpoollen)) {
				//プールを先頭から検索して未使用がみつかればそれをリリース
			int eulno;
			for (eulno = 0; eulno < curpoollen; eulno++) {
				OWP_EulerGraph::EulLineData::EulKey* cureulhead = (OWP_EulerGraph::EulLineData::EulKey*)g_eulpool[eulno];
				if (cureulhead) {
					int elemno;
					for (elemno = 0; elemno < EULPOOLBLKLEN; elemno++) {
						OrgWinGUI::OWP_EulerGraph::EulLineData::EulKey* cureul;
						cureul = cureulhead + elemno;
						if (cureul->GetUseFlag() == 0) {
							int saveindex = cureul->GetIndexOfPool();
							int saveallochead = cureul->IsAllocHead();
							cureul->InitParams();
							cureul->SetUseFlag(1);
							cureul->SetIndexOfPool(saveindex);
							cureul->SetIsAllocHead(saveallochead);

							s_befheadno = eulno;
							s_befelemno = elemno;
							return (void*)cureul;
						}
					}
				}
			}
			//}
		}

		//前回リリースしたポインタが最後尾または
		//未使用eulがpoolに無かった場合、アロケートしてアロケートした先頭のポインタをリリース
		OWP_EulerGraph::EulLineData::EulKey* alloceul;
		alloceul = new OWP_EulerGraph::EulLineData::EulKey[EULPOOLBLKLEN];
		if (!alloceul) {
			_ASSERT(0);

			s_befheadno = -1;
			s_befelemno = -1;

			return 0;
		}
		int allocno;
		for (allocno = 0; allocno < EULPOOLBLKLEN; allocno++) {
			OWP_EulerGraph::EulLineData::EulKey* curalloceul = alloceul + allocno;
			if (curalloceul) {
				int indexofpool = curpoollen + allocno;
				curalloceul->InitParams();
				curalloceul->SetUseFlag(0);
				curalloceul->SetIndexOfPool(indexofpool);

				if (allocno == 0) {
					curalloceul->SetIsAllocHead(1);
				}
				else {
					curalloceul->SetIsAllocHead(0);
				}
			}
			else {
				_ASSERT(0);

				s_befheadno = -1;
				s_befelemno = -1;

				return 0;
			}
		}
		g_eulpool.push_back((void*)alloceul);//allocate block(アロケート時の先頭ポインタ)を格納

		alloceul->SetUseFlag(1);


		s_befheadno = g_eulpool.size() - 1;
		s_befelemno = 0;

		return (void*)alloceul;
	}

	//member func
	void OWP_EulerGraph::EulLineData::EulKey::InvalidateEulKeys()
	{
		if (!this) {
			_ASSERT(0);
			return;
		}

		int saveindex = GetIndexOfPool();
		int saveallochead = IsAllocHead();

		InitParams();
		SetUseFlag(0);
		SetIsAllocHead(saveallochead);
		SetIndexOfPool(saveindex);
	}

	//namespace global func
	void InitEulKeys()
	{
		g_eulpool.clear();
	}

	//namespace global func
	void DestroyEulKeys() {
		int eulallocnum = g_eulpool.size();
		int eulno;
		for (eulno = 0; eulno < eulallocnum; eulno++) {
			//class OWP_EulerGraph::EulLineData::EulKey;
			OWP_EulerGraph::EulLineData::EulKey* deleul;
			deleul = (OWP_EulerGraph::EulLineData::EulKey*)g_eulpool[eulno];
			//if (deleul && (deleul->IsAllocHead() == 1)) {
			if (deleul) {
				delete[] deleul;
			}
		}
		g_eulpool.clear();
	}


	void OWP_CheckBoxA::draw() {
		if (!parentWindow) {
			return;
		}
		HWND parenthwnd = parentWindow->getHWnd();
		if (!IsWindow(parenthwnd)) {
			return;
		}


		if (hdcM) {
			drawEdge();

			//チェックボックス
			int pos1x = pos.x + BOX_POS_X;
			int pos1y = pos.y + size.y / 2 - BOX_WIDTH / 2;
			int pos2x = pos.x + BOX_POS_X + BOX_WIDTH - 1;
			int pos2y = pos.y + size.y / 2 + BOX_WIDTH / 2 - 1;
			hdcM->setPenAndBrush(RGB(240, 240, 240), NULL);
			Rectangle(hdcM->hDC, pos1x, pos1y, pos2x + 1, pos2y + 1);
			if (value) {
				MoveToEx(hdcM->hDC, pos1x + 2, pos1y + 2, NULL);
				LineTo(hdcM->hDC, pos2x - 1, pos2y - 1);
				MoveToEx(hdcM->hDC, pos2x - 2, pos1y + 2, NULL);
				LineTo(hdcM->hDC, pos1x + 1, pos2y - 1);
			}
			hdcM->setPenAndBrush(RGB(min(baseColor.r + 20, 255), min(baseColor.g + 20, 255), min(baseColor.b + 20, 255)), NULL);
			MoveToEx(hdcM->hDC, pos1x, pos1y + 2, NULL);
			LineTo(hdcM->hDC, pos1x, pos2y - 1);
			MoveToEx(hdcM->hDC, pos2x, pos1y + 2, NULL);
			LineTo(hdcM->hDC, pos2x, pos2y - 1);
			MoveToEx(hdcM->hDC, pos1x + 2, pos1y, NULL);
			LineTo(hdcM->hDC, pos2x - 1, pos1y);
			MoveToEx(hdcM->hDC, pos1x + 2, pos2y, NULL);
			LineTo(hdcM->hDC, pos2x - 1, pos2y);

			//名前
			pos1x = pos.x + BOX_POS_X + BOX_WIDTH + 3;
			pos1y = pos.y + size.y / 2 - 5;
			hdcM->setFont(12, _T("ＭＳ ゴシック"));
			SetTextColor(hdcM->hDC, RGB(240, 240, 240));
			TextOut(hdcM->hDC,
				pos1x, pos1y,
				name, (int)_tcslen(name));
		}
		{
			if (g_dsmousewait == 1) {
				POINT mousepoint;
				::GetCursorPos(&mousepoint);
				if (getParent() && getHDCMaster()) {
					::ScreenToClient(getParent()->getHWnd(), &mousepoint);
					int BMP_W = 52;
					int BMP_H = 50;
					Gdiplus::Graphics* gdipg = new Gdiplus::Graphics(hdcM->hDC);
					if (gdipg) {
						Gdiplus::ImageAttributes attr;
						Gdiplus::ColorMatrix cmat = {
							1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Red
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // Green
							0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Blue
							0.0f, 0.0f, 0.0f, g_mouseherealpha, 0.0f,   // Alpha (70%)
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f    // must be 1
						};
						attr.SetColorMatrix(&cmat);
						//Gdiplus::Image* imgptr = new Gdiplus::Image(L"E:\\PG\\MameBake3D_git\\MameBake3D\\Media\\MameMedia\\img_l105.png");
						if (g_mousehereimage) {
							gdipg->DrawImage(g_mousehereimage, Gdiplus::Rect(mousepoint.x, mousepoint.y, BMP_W, BMP_H),
								0, 0, BMP_W, BMP_H,
								Gdiplus::UnitPixel, &attr, NULL, NULL);
						}
						delete gdipg;
					}
				}
			}
		}

	}


	void OWP_Timeline::setCurrentTime(double _currentTime, bool CallListener) {
		static int s_paintcnt = 0;
		s_paintcnt++;


		int x1 = MARGIN + LABEL_SIZE_X;
		int x2 = size.x - MARGIN - SCROLL_BAR_WIDTH;

		if (timeSnapSize != 0) {
			_currentTime = floor(_currentTime / timeSnapSize + 0.5)*timeSnapSize;
		}

		currentTime = min(max(_currentTime, 0), maxTime);

		if (currentTime <= showPos_time) {
			showPos_time = currentTime;
		}
		if (showPos_time + ((double)(x2 - 3 - x1)) / timeSize <= currentTime) {
			showPos_time = currentTime - ((double)(x2 - 3 - x1)) / timeSize;
		}

		//リスナーコール
		if (CallListener && this->cursorListener != NULL) {
			(this->cursorListener)();
		}

		//再描画要求
		if (rewriteOnChange) {
			callRewrite();
		}
	}

	void OWP_EulerGraph::setCurrentTime(double _currentTime, bool CallListener) {
		static int s_paintcnt = 0;
		s_paintcnt++;

		int x1 = MARGIN + LABEL_SIZE_X;
		int x2 = size.x - MARGIN - SCROLL_BAR_WIDTH;

		if (timeSnapSize != 0) {
			_currentTime = floor(_currentTime / timeSnapSize + 0.5)*timeSnapSize;
		}

		currentTime = min(max(_currentTime, 0), maxTime);

		if (currentTime <= showPos_time) {
			showPos_time = currentTime;
		}
		if (showPos_time + ((double)(x2 - 3 - x1)) / timeSize <= currentTime) {
			showPos_time = currentTime - ((double)(x2 - 3 - x1)) / timeSize;
		}

		//リスナーコール
		if (CallListener && this->cursorListener != NULL) {
			(this->cursorListener)();
		}

		//再描画要求
		if (rewriteOnChange) {
			callRewrite();
		}
	}


	int OWP_EulerGraph::getEuler(double srcframe, ChaVector3* dsteul)
	{
		if (!dsteul) {
			return 1;
		}
		if (isseteulminmax == false) {
			return 1;
		}
		if ((int)lineData.size() != 3) {
			return 1;
		}

		double valx, valy, valz;
		int ret;
		ret = lineData[0]->getValue(srcframe, &valx);
		if (ret) {
			return 1;
		}
		ret = lineData[1]->getValue(srcframe, &valy);
		if (ret) {
			return 1;
		}
		ret = lineData[2]->getValue(srcframe, &valz);
		if (ret) {
			return 1;
		}

		dsteul->x = (float)valx;
		dsteul->y = (float)valy;
		dsteul->z = (float)valz;
	
		return 0;

	}

	int OWP_EulerGraph::EulLineData::getValue(double srcframe, double* dstvalue)
	{
		if (!dstvalue) {
			return 1;
		}
		if (parent->isseteulminmax == false) {
			return 1;
		}

		int srcindex = getKeyIndex(srcframe);
		if (srcindex >= 0) {
			*dstvalue = key[srcindex]->value;
			return 0;
		}
		else {
			return 1;
		}
	}


	/// Method : 再描画要求を送る
	void OrgWindowParts::callRewrite(){
		if( parentWindow==NULL ) return;

		//再描画領域
		RECT tmpRect;
		tmpRect.left=   pos.x+1;
		tmpRect.top=    pos.y+1;
		tmpRect.right=  pos.x+size.x-1;
		tmpRect.bottom= pos.y+size.y-1;
		//InvalidateRect( parentWindow->getHWnd(), &tmpRect, false );
		InvalidateRect(parentWindow->getHWnd(), &tmpRect, false);

		//draw();
	}

	///<summary>
	///	ウィンドウ内部品"タイムラインテーブル"クラス
	///</summary>
	//const double OWP_Timeline::LineData::TIME_ERROR_WIDTH= 0.0001;

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
			//if (message != WM_SETCURSOR) {
				return (DefWindowProc(hwnd, message, wParam, lParam));
			//}
			//else {
			//	return 0;
			//}
		}
		OrgWindow *owner= owItr->second;	//ウィンドウ作成元のクラスのインスタンスポインタ


		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;
		POINT tmpPoint;
		switch (message){				//イベント作成
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
			case WM_MOUSEHOVER:
			case WM_MOUSELEAVE:
				GetCursorPos(&tmpPoint);

				mouseEvent.globalX= (int)tmpPoint.x;
				mouseEvent.globalY= (int)tmpPoint.y;
				mouseEvent.localX= mouseEvent.globalX - owner->pos.x;
				mouseEvent.localY= mouseEvent.globalY - owner->pos.y;
				mouseEvent.shiftKey= GetKeyState(VK_SHIFT)<0;
				mouseEvent.ctrlKey= GetKeyState(VK_CONTROL)<0;
				mouseEvent.altKey= GetKeyState(VK_MENU)<0;
				mouseEvent.wheeldelta = GET_WHEEL_DELTA_WPARAM(wParam);
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
			case WM_MBUTTONDOWN:
				owner->onMButtonDown(mouseEvent);
				return 0;
			case WM_MBUTTONUP:
				owner->onMButtonUp(mouseEvent);
				return 0;
			case WM_MOUSEWHEEL:
				owner->onMouseWheel(mouseEvent);
				return 0;
			case WM_MOUSEHOVER:
				owner->onMouseHover(mouseEvent);
				return 0;
			case WM_MOUSELEAVE:
				owner->onMouseLeave(mouseEvent);
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
				if( (g_endappflag == 0 )&& owner->nowChangingSize ){
					owner->nowChangingSize= false;
					owner->autoResizeAllParts();
				}
				//if (message != WM_SETCURSOR) {
					return (DefWindowProc(hwnd, message, wParam, lParam));
				//}
				break;
		}

		return 0;
	}





}