#include "SceneToon.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"

void SceneToon::Init()
{
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_TOON));

	Texture* pTex = TextureFactory::CreateFromFile("Assets/ramp.png");
	RegisterObj<Texture>("RampTex", pTex);
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
	// ŠK’²‚Ì‰e‚ð•\Ž¦
	SetCullingMode(D3D11_CULL_NONE);
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_TOON));
	pModel->Draw();
	// —ÖŠsü‚ð•\Ž¦
	SetCullingMode(D3D11_CULL_BACK);
	pModel->SetVertexShader(GetVS(VS_OUTLINE));
	pModel->SetPixelShader(GetPS(PS_OUTLINE));
	pModel->Draw();
}