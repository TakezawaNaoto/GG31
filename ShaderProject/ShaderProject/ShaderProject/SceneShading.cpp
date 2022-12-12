#include "SceneShading.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"
#include "Input.h"

void SceneShading::Init()
{
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LAMBERT));

	pModel = CreateObj<Model>("LandModel");
	pModel->Load("Assets/Model/Cotoon_land2.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_FOG));

	ConstantBuffer* pCamBuf = CreateObj<ConstantBuffer>("CameraCB");
	pCamBuf->Create(sizeof(DirectX::XMFLOAT4) * 2);

	m_rate = 0.0f;

	Texture* pTex = TextureFactory::CreateFromFile(
		"Assets/dissolve.png");
	RegisterObj<Texture>("DissolveTex", pTex);
}
void SceneShading::Uninit()
{
}
void SceneShading::Update()
{
	Model* pModel = GetObj<Model>("UnityModel");
	if (IsKeyTrigger('1'))
	{
		pModel->SetPixelShader(GetPS(PS_LAMBERT));
	}
	if (IsKeyTrigger('2'))
	{
		pModel->SetPixelShader(GetPS(PS_PHONG));
	}
	if (IsKeyTrigger('3'))
	{
		pModel->SetPixelShader(GetPS(PS_LIMLIGHT));
	}
	if (IsKeyTrigger('4'))
	{
		pModel->SetPixelShader(GetPS(PS_DISSOLVE));
	}
	if (IsKeyPress(VK_LEFT))
	{
		m_rate = max(0.0f, m_rate - 0.01f);
	}
	if (IsKeyPress(VK_RIGHT))
	{
		m_rate = min(1.0f, m_rate + 0.01f);
	}
}
void SceneShading::Draw()
{
	CameraBase* pCamera = GetObj<CameraBase>("CameraDCC");
	ConstantBuffer* pWVP = GetObj<ConstantBuffer>("CBWVP");
	ConstantBuffer* pLight = GetObj<ConstantBuffer>("CBLight");
	ConstantBuffer* pCamBuf = GetObj<ConstantBuffer>("CameraCB");

	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();
	pWVP->Write(mat);
	pWVP->BindVS(0);

	DirectX::XMFLOAT4 lightDir(-1.0f, -1.0f, 1.0f, 0.0f);
	pLight->Write(&lightDir);
	pLight->BindPS(0);

	DirectX::XMFLOAT4 camBuf[] = {
		DirectX::XMFLOAT4(pCamera->GetPos().x, pCamera->GetPos().y, pCamera->GetPos().z, 0.0f),
		DirectX::XMFLOAT4(10.0f, 30.0f, m_rate, 0.0f)
	};
	pCamBuf->Write(camBuf);
	pCamBuf->BindPS(1);

	Texture* pTex = GetObj<Texture>("DissolveTex");
	SetTexturePS(pTex->GetResource(), 1);

	GetObj<Model>("UnityModel")->Draw();

	GetObj<Model>("CarModel")->Draw();

	GetObj<Model>("TreeModel")->Draw();
}