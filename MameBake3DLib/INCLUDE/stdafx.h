

#include <Windows.h>

#ifdef CONVD3DX9
#include <d3dcommon.h>
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <d3dx10.h>
#endif

//fpsチェックの時に有効にする。垂直同期をしないモード。
//DXUT.cpp のCreate()内でm_state.m_OverrideForceVsync = 0;//(通常1)
//DXUT.cpp のDXUTRender3DEnvironment10()でhr = pSwapChain->Present(0, 0);//(通常はPresent(1, dwFlags);
//#define SPEEDTEST__

//ロングタイムラインとオイラーグラフの描画をスキップするときに有効にする。
//#define SKIP_EULERGRAPH__

//物理シミュをスキップする。
//#define SKIP_BULLETSIMU__

//モーションポイント検索時にキャッシュを有効にする。
//SPEEDTEST__とSKIP_EULERGRAPH__とSKIP_BULLETSIMU__の３つを有効にした状態で以下を有効にすると効果が分かる。
//こちらの環境では１２キャラクターのモーション再生時で比べて、５倍から１５倍速になる。
#define USE_CACHE_ONGETMOTIONPOINT__

