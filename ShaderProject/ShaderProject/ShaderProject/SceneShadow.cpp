#include "SceneShadow.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

void SceneShadow::Init()
{
	// Unitychanモデル
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LAMBERT));
	// 部屋モデル
	pModel = CreateObj<Model>("room");
	pModel->Load("Assets/Model/room.fbx");
	pModel->SetVertexShader(GetVS(VS_ROOMSHADOW));
	pModel->SetPixelShader(GetPS(PS_DEPTHSHADOW));

	// ライト
	pModel = CreateObj<Model>("lightModel");
	pModel->Load("Assets/Model/light.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LIGHT_COLOR));

	// 書き込み先を作成(SceneToon.cppからコピー)
	Texture* pCanvas = TextureFactory::CreateRenderTarget(
		// DXGI_FORMAT_R8G8B8A8_UNORM, 色を書き込む場合のフォーマット
		DXGI_FORMAT_R32_FLOAT,	// 32bit = 4byteで描きこむフォーマット
		1280.0f, 720.0f
	);
	RegisterObj<Texture>("Canvas", pCanvas);

	DepthStencil* pDepth = reinterpret_cast<DepthStencil*>(
		TextureFactory::CreateDepthStencil(1280, 720, true)
		);
	RegisterObj<DepthStencil>("CanvasDSV", pDepth);

	// 太陽のカメラの行列を作成
	ConstantBuffer* pSunBuf =
		CreateObj<ConstantBuffer>("SunBuf");
	pSunBuf->Create(sizeof(DirectX::XMFLOAT4X4) * 2);

	// 点光源の定数バッファを作成
	ConstantBuffer* pPointLights =
		CreateObj<ConstantBuffer>("PointLights");
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
			(rand() % 5) * 0.1f + 0.6f,
			1.0f);
	}
	pPointLights->Write(m_lights);

	// 光源の色用の定数バッファ
	ConstantBuffer* pLightColor =
		CreateObj<ConstantBuffer>("LightColor");
	pLightColor->Create(sizeof(DirectX::XMFLOAT4));
}
void SceneShadow::Uninit()
{
}
void SceneShadow::Update()
{
	Model* pModel = GetObj<Model>("UnityModel");

}
void SceneShadow::Draw()
{
	CameraBase* pCamera = GetObj<CameraBase>("CameraDCC");
	ConstantBuffer* pWVP = GetObj<ConstantBuffer>("CBWVP");
	ConstantBuffer* pLight = GetObj<ConstantBuffer>("CBLight");
	Model* pUnitychan = GetObj<Model>("UnityModel");

	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());

	// 書き込み先の変更(SceneToon.cppからコピー)
	Texture* pCanvas = GetObj<Texture>("Canvas");
	DepthStencil* pDepth = GetObj<DepthStencil>("CanvasDSV");

	RenderTarget* pRTVCanvas = reinterpret_cast<RenderTarget*>(pCanvas);
	ID3D11RenderTargetView* pView = pRTVCanvas->GetView();
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// 描画先のクリア
	// 奥行の比較のために、一番遠い1を初期値で設定
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GetContext()->ClearRenderTargetView(pView, color);
	GetContext()->ClearDepthStencilView(pDepth->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 太陽の位置にカメラがあると想定して、テクスチャに書き込む
	// ※カメラが近すぎて、テクスチャに書き込めない部分が出てくると、正しく表示されないので
	// カメラを話して位置を設定する
	DirectX::XMMATRIX sunView = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
	DirectX::XMMATRIX sunProj = DirectX::XMMatrixOrthographicLH(
		5.0f, 5.0f * (9.0f / 16.0f), 0.2f, 100.0f
	);

		// 太陽の光は真っ直ぐ飛んでくるので、角度が透視投影では正しくない見た目になる。
		// DirectX::XMMatrixPerspectiveFovLH(
		// DirectX::XMConvertToRadians(60.0f), 16.0f / 9.0f, 0.2f, 100.0f
	// );
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(sunView));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(sunProj));
	pWVP->Write(mat);
	pWVP->BindVS(0);

	// 影になるオブジェクトを描画
	// 部屋の床に、ユニティちゃんの影を落とすので、ユニティちゃんだけ描きこむ
	// ※部屋も書き込むと、部屋の天井の書き込まれて、ユニティちゃんの影が見えなくなる
	pUnitychan->SetVertexShader(GetVS(VS_DEPTHWRITE));
	pUnitychan->SetPixelShader(GetPS(PS_DEPTHWRITE));
	pUnitychan->Draw();

	// 描画先を元の画面に戻す(SceneToon.cppからコピー)
	RenderTarget* pRTV = GetObj<RenderTarget>("DefRTV");
	pView = pRTV->GetView();
	pDepth = GetObj<DepthStencil>("DefDSV");
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// --- ゲーム画面の表示
	// --- matの中身がゲームのメインカメラの情報に
	// 上書きされる前に、定数バッファのb1へ設定する
	ConstantBuffer* pSunBuf =
		GetObj<ConstantBuffer>("SunBuf");
	pSunBuf->Write(&mat[1]);
	pSunBuf->BindVS(1);
	// ゲームのメインカメラを設定
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();
	pWVP->Write(mat);
	pWVP->BindVS(0);

	DirectX::XMFLOAT4 lightDir(-1.0f, -1.0f, 1.0f, 0.0f);
	pLight->Write(&lightDir);
	pLight->BindPS(0);

	// 事前に書き込んだテクスチャデータをhlslのt1に設定
	SetTexturePS(pCanvas->GetResource(), 1);

	// テクスチャ書き込むとは別のシェーダーを設定して描画
	GetObj<ConstantBuffer>("PointLights")->BindPS(0);
	pUnitychan->SetVertexShader(GetVS(VS_OBJECT));
	pUnitychan->SetPixelShader(GetPS(PS_POINTLIGHT));
	pUnitychan->Draw();

	GetObj<Model>("room")->Draw();

	// ライトの描画
	ConstantBuffer* pLightColor = GetObj<ConstantBuffer>("LightColor");
	pLightColor->BindPS(0);
	for (int i = 0; i < 30; ++i)
	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			m_lights[i].pos.x, m_lights[i].pos.y, m_lights[i].pos.z);
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(
			m_lights[i].range, m_lights[i].range, m_lights[i].range);
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(S * T));
		pWVP->Write(mat);
		// ライトの色
		pLightColor->Write(&m_lights[i].color);
		GetObj<Model>("lightModel")->Draw();
	}
}