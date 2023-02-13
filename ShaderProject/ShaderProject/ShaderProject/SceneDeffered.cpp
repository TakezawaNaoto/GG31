#include "SceneDeffered.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

void SceneDeffered::Init()
{
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_MULTI_RENDER));

	pModel = CreateObj<Model>("room");
	pModel->Load("Assets/Model/room.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_MULTI_RENDER));

	pModel = CreateObj<Model>("Screen");
	pModel->Load("Assets/Model/screen2.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_DEFFEREDPOINTLIGHT));

	pModel = CreateObj<Model>("lightModel");
	pModel->Load("Assets/Model/light.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LIGHT_COLOR));

	// マルチレンダーターゲットの書き込み先を用意
	Texture* pCanvasColor = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 720.0f);
	Texture* pCanvasNormal = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 720.0f);
	Texture* pCanvasWorld = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R32_FLOAT, 1280.0f, 720.0f);
	RegisterObj<Texture>("CanvasColor", pCanvasColor);
	RegisterObj<Texture>("pCanvasNormal", pCanvasNormal);
	RegisterObj<Texture>("pCanvasWorld", pCanvasWorld);

	// 複数の描画に対して、奥行きの情報は一つでOK
	Texture* pCanvasDepth = TextureFactory::CreateDepthStencil(1280, 720, true);
	RegisterObj<Texture>("CanvasDSV", pCanvasDepth);

	// 点光源のみを書き込むバッファを用意
	Texture* pCanvasPointLight = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 128.0f, 72.0f);	// 画像を小さくしてぼかす
	RegisterObj<Texture>("CanvasPointLight", pCanvasPointLight);

	// ぼかし用のバッファを用意
	Texture* pCanvasGradation = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 72.0f);
	RegisterObj<Texture>("CanvasGradation", pCanvasGradation);

	// --- 定数バッファ作成
	ConstantBuffer* pInverse =
		CreateObj<ConstantBuffer>("invMat");
	pInverse->Create(sizeof(DirectX::XMFLOAT4X4) * 2);

	// 点光源
	ConstantBuffer* pPointLights = CreateObj<ConstantBuffer>("PointLight");
	pPointLights->Create(sizeof(DirectX::XMFLOAT4) * 2 * 30);

	for (int i = 0; i < 30; ++i)
	{
		m_lights[i].pos = DirectX::XMFLOAT3(
			(rand() % 11 - 5) * 0.2f,
			(rand() % 10) * 0.1f,
			(rand() % 11 - 5) * 0.2f);
		m_lights[i].range = 0.3f;
		m_lights[i].color = DirectX::XMFLOAT4(
			(rand() % 5) * 0.1f + 0.6f,
			(rand() % 5) * 0.1f + 0.6f,
			(rand() % 5) * 0.1f + 0.6f, 1.0f);
	}
	pPointLights->Write(m_lights);

	ConstantBuffer* pLightColor =
		CreateObj<ConstantBuffer>("LightColor");
	pLightColor->Create(sizeof(DirectX::XMFLOAT4));
}
void SceneDeffered::Uninit()
{
}
void SceneDeffered::Update()
{

}
void SceneDeffered::Draw()
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

	// ビューポートの作成
	D3D11_VIEWPORT vp = {};
	vp.Width = 1280.0f;
	vp.Height = 720.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

	// マルチレンダーターゲットの設定
	RenderTarget* pCanvasColor =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("CanvasColor"));
	RenderTarget* pCanvasNormal =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("pCanvasNormal"));
	RenderTarget* pCanvasWorld =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("pCanvasWorld"));
	DepthStencil* pCanvasDSV = 
		reinterpret_cast<DepthStencil*>(GetObj<Texture>("CanvasDSV"));

	ID3D11RenderTargetView*pView[3] = {
		pCanvasColor->GetView(), pCanvasNormal->GetView(), pCanvasWorld->GetView()
	};
	GetContext()->OMSetRenderTargets(3, pView, pCanvasDSV->GetView());

	// 描画先のクリア
	float color[][4] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f }
	};
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	GetContext()->ClearRenderTargetView(pView[1], color[0]);
	GetContext()->ClearRenderTargetView(pView[2], color[1]);
	GetContext()->ClearDepthStencilView(pCanvasDSV->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



	// テクスチャに書き込み
	GetObj<Model>("UnityModel")->Draw();
	GetObj<Model>("room")->Draw();

	// 別のテクスチャに書き込み
	RenderTarget* pCanvasPointLight =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("CanvasPointLight"));
	pView[0] = pCanvasPointLight->GetView();
	GetContext()->OMSetRenderTargets(1, pView, pCanvasDSV->GetView());
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	vp.Width = 128.0f;
	vp.Height = 72.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

	// 点光源描画
	ConstantBuffer* pLightColor = GetObj<ConstantBuffer>("LightColor");
	pLight->BindPS(0);
	for (int i = 0; i < 30; ++i)
	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			m_lights[i].pos.x, m_lights[i].pos.y, m_lights[i].pos.z);
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(
			m_lights[i].range, m_lights[i].range, m_lights[i].range);
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(S * T));
		pWVP->Write(mat);
		// ライトの色
		pLight->Write(&m_lights[i].color);
		GetObj<Model>("lightModel")->Draw();
	}

	// ぼかし処理を実施
	RenderTarget* pCanvasGradation =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("CanvasGradation"));
	pView[0] = pCanvasGradation->GetView();
	GetContext()->OMSetRenderTargets(1, pView, nullptr);
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	
	// 2D表示の行列を設定
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixOrthographicLH(1.6f, 0.9f, -1.0f, 1.0f)));
	pWVP->Write(mat);

	// レンダーターゲットに収まるサイズでモデルを表示
	Model* pScreen = GetObj<Model>("Screen");
	pScreen->SetPixelShader(GetPS(PS_GRADATION));
	SetTexturePS(pCanvasPointLight->GetResource(), 1);

	// 元の描画に戻す
	RenderTarget*pRTV = GetObj<RenderTarget>("DefRTV");
	pView[0] = pRTV->GetView();
	pCanvasDSV = GetObj<DepthStencil>("DefDSV");
	GetContext()->OMSetRenderTargets(1, pView, pCanvasDSV->GetView());
	vp.Width = 1280.0f;
	vp.Height = 720.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

#if 0	// マルチレンダーターゲット書き込み確認
	
	SetTexturePS(pCanvasColor->GetResource(), 1);
	SetTexturePS(pCanvasNormal->GetResource(), 2);
	SetTexturePS(pCanvasWorld->GetResource(), 3);
	GetObj<Model>("Screen")->Draw();
#else	// ディファードレンダリングで点光源
	ConstantBuffer* pInverse =
		GetObj<ConstantBuffer>("invMat");
	DirectX::XMFLOAT4X4 fView = pCamera->GetView();
	DirectX::XMFLOAT4X4 fProj = pCamera->GetProj();
	// カメラのビュー・プロジェクション行列は転置済みなので
	// 転置を戻して逆行列を計算
	DirectX::XMMATRIX matView =
		DirectX::XMLoadFloat4x4(&fView);
	DirectX::XMMATRIX matProj =
		DirectX::XMLoadFloat4x4(&fProj);
	matView = DirectX::XMMatrixTranspose(matView);
	matProj = DirectX::XMMatrixTranspose(matProj);
	matView = DirectX::XMMatrixInverse(nullptr, matView);
	matProj = DirectX::XMMatrixInverse(nullptr, matProj);
	// 逆行列をシェーダーに渡すために転置する
	DirectX::XMFLOAT4X4 fInvMat[2];
	DirectX::XMStoreFloat4x4(&fInvMat[0],
		DirectX::XMMatrixTranspose(matView));
	DirectX::XMStoreFloat4x4(&fInvMat[1],
		DirectX::XMMatrixTranspose(matProj));
	pInverse->Write(fInvMat);
	pInverse->BindPS(0);

	// 2D表示の行列を設定
	DirectX::XMStoreFloat4x4(&mat[0],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[2],
		DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicLH(1.6f, 0.9f, -1.0f, 1.0f)));
	pWVP->Write(mat);

	// スクリーンの描画
	GetObj<ConstantBuffer>("PointLight")->BindPS(1);
	SetTexturePS(pCanvasColor->GetResource(), 1);
	SetTexturePS(pCanvasNormal->GetResource(), 2);
	SetTexturePS(pCanvasWorld->GetResource(), 3);
	SetTexturePS(pCanvasGradation->GetResource(), 4);
	pScreen->SetPixelShader(GetPS(PS_DEFFEREDPOINTLIGHT));
	pScreen->Draw();
#endif
}