#include "Main.h"
#include <memory>
#include "DirectX.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "TextureFactory.h"
#include "Geometory.h"
#include "Input.h"
#include "CameraDCC.h"
#include "SceneView.h"
#include "ShaderList.h"

//--- グローバル変数
std::shared_ptr<SceneView> g_pScene;

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	hr = InitDX(hWnd, width, height, false);
	if (FAILED(hr)) { return hr; }
	InitGeometory();
	InitInput();
	InitShaderList();

	// シーン作成
	g_pScene = std::make_shared<SceneView>();
	g_pScene->Init();
	// レンダーターゲット作成
	RenderTarget* pRTV = reinterpret_cast<RenderTarget*>(TextureFactory::CreateRenderTargetFromScreen());
	g_pScene->RegisterObj("DefRTV", pRTV);
	// 深度バッファ作成
	DepthStencil* pDSV = reinterpret_cast<DepthStencil*>(TextureFactory::CreateDepthStencil(width, height, false));
	g_pScene->RegisterObj("DefDSV", pDSV);

	ID3D11RenderTargetView* pViews[] = {
		pRTV->GetView()
	};
	GetContext()->OMSetRenderTargets(
		1, pViews, pDSV->GetView()
	);

	// ビューポートの設定(描き込める領域の設定
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);


	return hr;
}

void Uninit()
{
	g_pScene->Uninit();
	g_pScene.reset();

	UninitShaderList();
	UninitInput();
	UninitGeometory();
	UninitDX();
}

void Update()
{
	UpdateInput();
	g_pScene->_update();
	g_pScene->GetObj<CameraDCC>("CameraDCC")->Update();
}

void Draw()
{
	RenderTarget* pRTV = g_pScene->GetObj<RenderTarget>("DefRTV");
	DepthStencil* pDSV = g_pScene->GetObj<DepthStencil>("DefDSV");
	CameraDCC* pCamera = g_pScene->GetObj<CameraDCC>("CameraDCC");

	float color[4] = { 0.8f, 0.9f, 1.0f, 1.0f };
	GetContext()->ClearRenderTargetView(pRTV->GetView(), color);
	GetContext()->ClearDepthStencilView(pDSV->GetView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pScene->_draw();
	
	SetGeometoryVP(pCamera->GetView(), pCamera->GetProj());
	SetGeometoryColor(DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
	const int GridSize = 10;
	for (int i = 1; i <= GridSize; ++i)
	{
		float g = (float)i;
		AddLine(DirectX::XMFLOAT3( g, 0.0f, -GridSize), DirectX::XMFLOAT3( g, 0.0f, GridSize));
		AddLine(DirectX::XMFLOAT3(-g, 0.0f, -GridSize), DirectX::XMFLOAT3(-g, 0.0f, GridSize));
		AddLine(DirectX::XMFLOAT3(-GridSize,0.0f,  g), DirectX::XMFLOAT3(GridSize,0.0f, g));
		AddLine(DirectX::XMFLOAT3(-GridSize,0.0f, -g), DirectX::XMFLOAT3(GridSize,0.0f,-g));
	}
	SetGeometoryColor(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	AddLine(DirectX::XMFLOAT3(-GridSize, 0.0f, 0.0f), DirectX::XMFLOAT3(GridSize, 0.0f, 0.0f));
	SetGeometoryColor(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	AddLine(DirectX::XMFLOAT3(0.0f, -GridSize, 0.0f), DirectX::XMFLOAT3(0.0f, GridSize, 0.0f));
	SetGeometoryColor(DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	AddLine(DirectX::XMFLOAT3(0.0f, 0.0f, - GridSize), DirectX::XMFLOAT3(0.0f, 0.0f, GridSize));
	
	pCamera->Draw();
	DrawLines();

	SwapDX();
}

// EOF