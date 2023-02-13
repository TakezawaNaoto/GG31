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

	// �}���`�����_�[�^�[�Q�b�g�̏������ݐ��p��
	Texture* pCanvasColor = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 720.0f);
	Texture* pCanvasNormal = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 720.0f);
	Texture* pCanvasWorld = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R32_FLOAT, 1280.0f, 720.0f);
	RegisterObj<Texture>("CanvasColor", pCanvasColor);
	RegisterObj<Texture>("pCanvasNormal", pCanvasNormal);
	RegisterObj<Texture>("pCanvasWorld", pCanvasWorld);

	// �����̕`��ɑ΂��āA���s���̏��͈��OK
	Texture* pCanvasDepth = TextureFactory::CreateDepthStencil(1280, 720, true);
	RegisterObj<Texture>("CanvasDSV", pCanvasDepth);

	// �_�����݂̂��������ރo�b�t�@��p��
	Texture* pCanvasPointLight = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 128.0f, 72.0f);	// �摜�����������Ăڂ���
	RegisterObj<Texture>("CanvasPointLight", pCanvasPointLight);

	// �ڂ����p�̃o�b�t�@��p��
	Texture* pCanvasGradation = TextureFactory::CreateRenderTarget(
		DXGI_FORMAT_R8G8B8A8_UNORM, 1280.0f, 72.0f);
	RegisterObj<Texture>("CanvasGradation", pCanvasGradation);

	// --- �萔�o�b�t�@�쐬
	ConstantBuffer* pInverse =
		CreateObj<ConstantBuffer>("invMat");
	pInverse->Create(sizeof(DirectX::XMFLOAT4X4) * 2);

	// �_����
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

	// �r���[�|�[�g�̍쐬
	D3D11_VIEWPORT vp = {};
	vp.Width = 1280.0f;
	vp.Height = 720.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

	// �}���`�����_�[�^�[�Q�b�g�̐ݒ�
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

	// �`���̃N���A
	float color[][4] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f }
	};
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	GetContext()->ClearRenderTargetView(pView[1], color[0]);
	GetContext()->ClearRenderTargetView(pView[2], color[1]);
	GetContext()->ClearDepthStencilView(pCanvasDSV->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



	// �e�N�X�`���ɏ�������
	GetObj<Model>("UnityModel")->Draw();
	GetObj<Model>("room")->Draw();

	// �ʂ̃e�N�X�`���ɏ�������
	RenderTarget* pCanvasPointLight =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("CanvasPointLight"));
	pView[0] = pCanvasPointLight->GetView();
	GetContext()->OMSetRenderTargets(1, pView, pCanvasDSV->GetView());
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	vp.Width = 128.0f;
	vp.Height = 72.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

	// �_�����`��
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
		// ���C�g�̐F
		pLight->Write(&m_lights[i].color);
		GetObj<Model>("lightModel")->Draw();
	}

	// �ڂ������������{
	RenderTarget* pCanvasGradation =
		reinterpret_cast<RenderTarget*>(GetObj<Texture>("CanvasGradation"));
	pView[0] = pCanvasGradation->GetView();
	GetContext()->OMSetRenderTargets(1, pView, nullptr);
	GetContext()->ClearRenderTargetView(pView[0], color[0]);
	
	// 2D�\���̍s���ݒ�
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixOrthographicLH(1.6f, 0.9f, -1.0f, 1.0f)));
	pWVP->Write(mat);

	// �����_�[�^�[�Q�b�g�Ɏ��܂�T�C�Y�Ń��f����\��
	Model* pScreen = GetObj<Model>("Screen");
	pScreen->SetPixelShader(GetPS(PS_GRADATION));
	SetTexturePS(pCanvasPointLight->GetResource(), 1);

	// ���̕`��ɖ߂�
	RenderTarget*pRTV = GetObj<RenderTarget>("DefRTV");
	pView[0] = pRTV->GetView();
	pCanvasDSV = GetObj<DepthStencil>("DefDSV");
	GetContext()->OMSetRenderTargets(1, pView, pCanvasDSV->GetView());
	vp.Width = 1280.0f;
	vp.Height = 720.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);

#if 0	// �}���`�����_�[�^�[�Q�b�g�������݊m�F
	
	SetTexturePS(pCanvasColor->GetResource(), 1);
	SetTexturePS(pCanvasNormal->GetResource(), 2);
	SetTexturePS(pCanvasWorld->GetResource(), 3);
	GetObj<Model>("Screen")->Draw();
#else	// �f�B�t�@�[�h�����_�����O�œ_����
	ConstantBuffer* pInverse =
		GetObj<ConstantBuffer>("invMat");
	DirectX::XMFLOAT4X4 fView = pCamera->GetView();
	DirectX::XMFLOAT4X4 fProj = pCamera->GetProj();
	// �J�����̃r���[�E�v���W�F�N�V�����s��͓]�u�ς݂Ȃ̂�
	// �]�u��߂��ċt�s����v�Z
	DirectX::XMMATRIX matView =
		DirectX::XMLoadFloat4x4(&fView);
	DirectX::XMMATRIX matProj =
		DirectX::XMLoadFloat4x4(&fProj);
	matView = DirectX::XMMatrixTranspose(matView);
	matProj = DirectX::XMMatrixTranspose(matProj);
	matView = DirectX::XMMatrixInverse(nullptr, matView);
	matProj = DirectX::XMMatrixInverse(nullptr, matProj);
	// �t�s����V�F�[�_�[�ɓn�����߂ɓ]�u����
	DirectX::XMFLOAT4X4 fInvMat[2];
	DirectX::XMStoreFloat4x4(&fInvMat[0],
		DirectX::XMMatrixTranspose(matView));
	DirectX::XMStoreFloat4x4(&fInvMat[1],
		DirectX::XMMatrixTranspose(matProj));
	pInverse->Write(fInvMat);
	pInverse->BindPS(0);

	// 2D�\���̍s���ݒ�
	DirectX::XMStoreFloat4x4(&mat[0],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1],
		DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[2],
		DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicLH(1.6f, 0.9f, -1.0f, 1.0f)));
	pWVP->Write(mat);

	// �X�N���[���̕`��
	GetObj<ConstantBuffer>("PointLight")->BindPS(1);
	SetTexturePS(pCanvasColor->GetResource(), 1);
	SetTexturePS(pCanvasNormal->GetResource(), 2);
	SetTexturePS(pCanvasWorld->GetResource(), 3);
	SetTexturePS(pCanvasGradation->GetResource(), 4);
	pScreen->SetPixelShader(GetPS(PS_DEFFEREDPOINTLIGHT));
	pScreen->Draw();
#endif
}