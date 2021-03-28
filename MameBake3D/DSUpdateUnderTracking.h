#pragma once
#ifndef DSUPDATEUNDERTRACKINGH
#define DSUPDATEUNDERTRACKINGH

#include <stdio.h>

#include <windows.h>
#include <wchar.h>
#include <coef.h>

class CDSUpdateUnderTracking
{
public:
	CDSUpdateUnderTracking();
	~CDSUpdateUnderTracking();

	int CreateDSUpdateUnderTracking();



	int OutputInfo(WCHAR* lpFormat, ...);


	void UpdateWindow();
	void UpdateWindowFunc();

	void OnPaint();
	//void onMouseWheel(const OrgWinGUI::MouseEvent& e);

	HWND GetHWnd() { return m_hWnd; };
	int GetDataIndex() { return m_dataindex; };
	bool IsFirstOutput() { return m_isfirstoutput; };
	int GetStrNum();
	int GetStr(int srcindex, int srcoutleng, WCHAR* strout);

private:
	void InitParams();
	void DestroyObjs();

private:
	HWND m_hWnd;
	HWND m_hParentWnd;
	//OrgWinGUI::HDCMaster* m_hdcM;

	//WCHAR m_stroutput[DSUpdateUnderTrackingLINEH][DSUpdateUnderTrackingLINEW];
	WCHAR* m_stroutput;
	
	int m_dataindex;//�f�[�^�i�[�ʒu
	int m_viewindex;//�\���J�n�ʒu
	bool m_isfirstoutput;//�ŏ��̃f�[�^�i�[���ǂ���
};

#endif