#include "SceneToon.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

void SceneToon::Init()
{
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_TOON));

	pModel = CreateObj<Model>("Screen");
	pModel->Load("Assets/Model/screen.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_SCREEN));

	Texture* pTex = TextureFactory::CreateFromFile("Assets/ramp.png");
	RegisterObj<Texture>("RampTex", pTex);

	// 書き込み先を作成
	Texture* pCanvas = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 720.0f
	);
	RegisterObj<Texture>("Canvas", pCanvas);

	DepthStencil* pDepth = reinterpret_cast<DepthStencil*>(
		TextureFactory::CreateDepthStencil(1280, 720, true)
		);
	RegisterObj<DepthStencil>("CanvasDSV", pDepth);
}
void SceneToon::Uninit()
{
}
void SceneToon::Update()
{
}
void SceneToon::Draw()
{
	CameraBase* pCamera = GetObj<CameraBase>("CameraDCC");
	ConstantBuffer* pWVP = GetObj<ConstantBuffer>("CBWVP");
	ConstantBuffer* pLight = GetObj<ConstantBuffer>("CBLight");

	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();
	pWVP->Write(mat);
	pWVP->BindVS(0);

	DirectX::XMFLOAT4 lightDir(-1.0f, -1.0f, 1.0f, 0.0f);
	pLight->Write(&lightDir);
	pLight->BindPS(0);

	Texture* pTex = GetObj<Texture>("RampTex");
	SetTexturePS(pTex->GetResource(), 1);


	Model* pModel = GetObj<Model>("UnityModel");

	// 書き込み先の変更
	Texture* pCanvas = GetObj<Texture>("Canvas");
	DepthStencil* pDepth = GetObj<DepthStencil>("CanvasDSV");

	RenderTarget* pRTVCanvas = reinterpret_cast<RenderTarget*>(pCanvas);
	ID3D11RenderTargetView* pView = pRTVCanvas->GetView();
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// 描画先のクリア
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GetContext()->ClearRenderTargetView(pView, color);
	GetContext()->ClearDepthStencilView(pDepth->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 階調の影を表示
	SetCullingMode(D3D11_CULL_NONE);
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_TOON));
	pModel->Draw();
	// 輪郭線を表示
	SetCullingMode(D3D11_CULL_BACK);
	pModel->SetVertexShader(GetVS(VS_OUTLINE));
	pModel->SetPixelShader(GetPS(PS_OUTLINE));
	pModel->Draw();

	// 描画先を元の画面に戻す
	RenderTarget* pRTV = GetObj<RenderTarget>("DefRTV");
	pView = pRTV->GetView();
	pDepth = GetObj<DepthStencil>("DefDSV");
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// --- ゲーム空間上に改めて描画
	// 階調の影を表示
	SetCullingMode(D3D11_CULL_NONE);
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_TOON));
	pModel->Draw();
	// 輪郭線を表示
	SetCullingMode(D3D11_CULL_BACK);
	pModel->SetVertexShader(GetVS(VS_OUTLINE));
	pModel->SetPixelShader(GetPS(PS_OUTLINE));
	pModel->Draw();

	// スクリーンを表示
	SetTexturePS(pCanvas->GetResource(), 1);
	pModel = GetObj<Model>("Screen");
	pModel->Draw();
}