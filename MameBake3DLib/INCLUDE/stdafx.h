

#include <Windows.h>

#ifdef CONVD3DX9
#include <d3dcommon.h>
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <d3dx10.h>
#endif

//fps�`�F�b�N�̎��ɗL���ɂ���B�������������Ȃ����[�h�B
//DXUT.cpp ��Create()����m_state.m_OverrideForceVsync = 0;//(�ʏ�1)
//DXUT.cpp ��DXUTRender3DEnvironment10()��hr = pSwapChain->Present(0, 0);//(�ʏ��Present(1, dwFlags);
//#define SPEEDTEST__

//�����O�^�C�����C���ƃI�C���[�O���t�̕`����X�L�b�v����Ƃ��ɗL���ɂ���B
//#define SKIP_EULERGRAPH__

//�����V�~�����X�L�b�v����B
//#define SKIP_BULLETSIMU__

//���[�V�����|�C���g�������ɃL���b�V����L���ɂ���B
//SPEEDTEST__��SKIP_EULERGRAPH__��SKIP_BULLETSIMU__�̂R��L���ɂ�����Ԃňȉ���L���ɂ���ƌ��ʂ�������B
//������̊��ł͂P�Q�L�����N�^�[�̃��[�V�����Đ����Ŕ�ׂāA�T�{����P�T�{���ɂȂ�B
#define USE_CACHE_ONGETMOTIONPOINT__

