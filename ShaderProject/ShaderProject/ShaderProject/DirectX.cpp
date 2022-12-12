#include "DirectX.h"

//--- グローバル変数
ID3D11Device *g_pDevice;
ID3D11DeviceContext *g_pContext;
IDXGISwapChain *g_pSwapChain;
ID3D11RasterizerState* g_pRasterizerState[3];


HRESULT InitDX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
	HRESULT	hr = E_FAIL;

	// スワップチェインの設定
	// スワップチェインとは、ウインドウへの表示ダブルバッファを管理する
	// マルチサンプリング、リフレッシュレートが設定できる
	// 複数のバックバッファが作成できる
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));					// ゼロクリア
	sd.BufferDesc.Width = width;						// バックバッファの幅
	sd.BufferDesc.Height = height;						// バックバッファの高さ
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// バックバッファフォーマット(R,G,B,A)
	sd.SampleDesc.Count = 1;		// マルチサンプルの数
	sd.BufferDesc.RefreshRate.Numerator = 1000;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バックバッファの使用方法
	sd.BufferCount = 1;									// バックバッファの数
	sd.OutputWindow = hWnd;			// 関連付けるウインドウ
	sd.Windowed = fullscreen ? FALSE : TRUE;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	// ドライバの種類
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	// GPUで描画
		D3D_DRIVER_TYPE_WARP,		// 高精度(低速
		D3D_DRIVER_TYPE_REFERENCE,	// CPUで描画
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	// 機能レベル
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,		// DirectX11.1対応GPUレベル
		D3D_FEATURE_LEVEL_11_0,		// DirectX11対応GPUレベル
		D3D_FEATURE_LEVEL_10_1,		// DirectX10.1対応GPUレベル
		D3D_FEATURE_LEVEL_10_0,		// DirectX10対応GPUレベル
		D3D_FEATURE_LEVEL_9_3,		// DirectX9.3対応GPUレベル
		D3D_FEATURE_LEVEL_9_2,		// DirectX9.2対応GPUレベル
		D3D_FEATURE_LEVEL_9_1		// Direct9.1対応GPUレベル
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,					// ディスプレイデバイスのアダプタ（NULLの場合最初に見つかったアダプタ）
			driverType,				// デバイスドライバのタイプ
			NULL,					// ソフトウェアラスタライザを使用する場合に指定する
			createDeviceFlags,		// デバイスフラグ
			featureLevels,			// 機能レベル
			numFeatureLevels,		// 機能レベル数
			D3D11_SDK_VERSION,		// 
			&sd,					// スワップチェインの設定
			&g_pSwapChain,			// IDXGIDwapChainインタフェース	
			&g_pDevice,				// ID3D11Deviceインタフェース
			&featureLevel,		// サポートされている機能レベル
			&g_pContext);		// デバイスコンテキスト
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	if (FAILED(hr)) {
		return hr;
	}

	D3D11_RASTERIZER_DESC rasterizer = {};
	D3D11_CULL_MODE cull[] = {
		D3D11_CULL_NONE,
		D3D11_CULL_FRONT,
		D3D11_CULL_BACK
	};
	rasterizer.FillMode = D3D11_FILL_SOLID;
	rasterizer.FrontCounterClockwise = true;
	for (int i = 0; i < 3; ++i)
	{
		rasterizer.CullMode = cull[i];
		hr = g_pDevice->CreateRasterizerState(&rasterizer, &g_pRasterizerState[i]);
		if (FAILED(hr)) { return hr; }
	}
	SetCullingMode(D3D11_CULL_FRONT);
	return S_OK;
}

void UninitDX()
{
	g_pContext->ClearState();
	SAFE_RELEASE(g_pContext);

	g_pSwapChain->SetFullscreenState(false, NULL);
	SAFE_RELEASE(g_pSwapChain);

	SAFE_RELEASE(g_pDevice);
}
void SwapDX()
{
	g_pSwapChain->Present(0, 0);
}

ID3D11Device *GetDevice()
{
	return g_pDevice;
}

ID3D11DeviceContext *GetContext()
{
	return g_pContext;
}
IDXGISwapChain* GetSwapChain()
{
	return g_pSwapChain;
}
void SetCullingMode(D3D11_CULL_MODE cull)
{
	switch (cull)
	{
	case D3D11_CULL_NONE: g_pContext->RSSetState(g_pRasterizerState[0]); break;
	case D3D11_CULL_FRONT: g_pContext->RSSetState(g_pRasterizerState[1]); break;
	case D3D11_CULL_BACK: g_pContext->RSSetState(g_pRasterizerState[2]); break;
	}
}