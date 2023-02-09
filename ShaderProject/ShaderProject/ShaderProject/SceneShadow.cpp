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
	// Unitychan���f��
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LAMBERT));
	// �������f��
	pModel = CreateObj<Model>("room");
	pModel->Load("Assets/Model/room.fbx");
	pModel->SetVertexShader(GetVS(VS_ROOMSHADOW));
	pModel->SetPixelShader(GetPS(PS_DEPTHSHADOW));

	// ���C�g
	pModel = CreateObj<Model>("lightModel");
	pModel->Load("Assets/Model/light.fbx");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_LIGHT_COLOR));

	// �������ݐ���쐬(SceneToon.cpp����R�s�[)
	Texture* pCanvas = TextureFactory::CreateRenderTarget(
		// DXGI_FORMAT_R8G8B8A8_UNORM, �F���������ޏꍇ�̃t�H�[�}�b�g
		DXGI_FORMAT_R32_FLOAT,	// 32bit = 4byte�ŕ`�����ރt�H�[�}�b�g
		1280.0f, 720.0f
	);
	RegisterObj<Texture>("Canvas", pCanvas);

	DepthStencil* pDepth = reinterpret_cast<DepthStencil*>(
		TextureFactory::CreateDepthStencil(1280, 720, true)
		);
	RegisterObj<DepthStencil>("CanvasDSV", pDepth);

	// ���z�̃J�����̍s����쐬
	ConstantBuffer* pSunBuf =
		CreateObj<ConstantBuffer>("SunBuf");
	pSunBuf->Create(sizeof(DirectX::XMFLOAT4X4) * 2);

	// �_�����̒萔�o�b�t�@���쐬
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

	// �����̐F�p�̒萔�o�b�t�@
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

	// �������ݐ�̕ύX(SceneToon.cpp����R�s�[)
	Texture* pCanvas = GetObj<Texture>("Canvas");
	DepthStencil* pDepth = GetObj<DepthStencil>("CanvasDSV");

	RenderTarget* pRTVCanvas = reinterpret_cast<RenderTarget*>(pCanvas);
	ID3D11RenderTargetView* pView = pRTVCanvas->GetView();
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// �`���̃N���A
	// ���s�̔�r�̂��߂ɁA��ԉ���1�������l�Őݒ�
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GetContext()->ClearRenderTargetView(pView, color);
	GetContext()->ClearDepthStencilView(pDepth->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ���z�̈ʒu�ɃJ����������Ƒz�肵�āA�e�N�X�`���ɏ�������
	// ���J�������߂����āA�e�N�X�`���ɏ������߂Ȃ��������o�Ă���ƁA�������\������Ȃ��̂�
	// �J������b���Ĉʒu��ݒ肷��
	DirectX::XMMATRIX sunView = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
	DirectX::XMMATRIX sunProj = DirectX::XMMatrixOrthographicLH(
		5.0f, 5.0f * (9.0f / 16.0f), 0.2f, 100.0f
	);

		// ���z�̌��͐^���������ł���̂ŁA�p�x���������e�ł͐������Ȃ������ڂɂȂ�B
		// DirectX::XMMatrixPerspectiveFovLH(
		// DirectX::XMConvertToRadians(60.0f), 16.0f / 9.0f, 0.2f, 100.0f
	// );
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(sunView));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(sunProj));
	pWVP->Write(mat);
	pWVP->BindVS(0);

	// �e�ɂȂ�I�u�W�F�N�g��`��
	// �����̏��ɁA���j�e�B�����̉e�𗎂Ƃ��̂ŁA���j�e�B����񂾂��`������
	// ���������������ނƁA�����̓V��̏������܂�āA���j�e�B�����̉e�������Ȃ��Ȃ�
	pUnitychan->SetVertexShader(GetVS(VS_DEPTHWRITE));
	pUnitychan->SetPixelShader(GetPS(PS_DEPTHWRITE));
	pUnitychan->Draw();

	// �`�������̉�ʂɖ߂�(SceneToon.cpp����R�s�[)
	RenderTarget* pRTV = GetObj<RenderTarget>("DefRTV");
	pView = pRTV->GetView();
	pDepth = GetObj<DepthStencil>("DefDSV");
	GetContext()->OMSetRenderTargets(1, &pView, pDepth->GetView());

	// --- �Q�[����ʂ̕\��
	// --- mat�̒��g���Q�[���̃��C���J�����̏���
	// �㏑�������O�ɁA�萔�o�b�t�@��b1�֐ݒ肷��
	ConstantBuffer* pSunBuf =
		GetObj<ConstantBuffer>("SunBuf");
	pSunBuf->Write(&mat[1]);
	pSunBuf->BindVS(1);
	// �Q�[���̃��C���J������ݒ�
	mat[1] = pCamera->GetView();
	mat[2] = pCamera->GetProj();
	pWVP->Write(mat);
	pWVP->BindVS(0);

	DirectX::XMFLOAT4 lightDir(-1.0f, -1.0f, 1.0f, 0.0f);
	pLight->Write(&lightDir);
	pLight->BindPS(0);

	// ���O�ɏ������񂾃e�N�X�`���f�[�^��hlsl��t1�ɐݒ�
	SetTexturePS(pCanvas->GetResource(), 1);

	// �e�N�X�`���������ނƂ͕ʂ̃V�F�[�_�[��ݒ肵�ĕ`��
	GetObj<ConstantBuffer>("PointLights")->BindPS(0);
	pUnitychan->SetVertexShader(GetVS(VS_OBJECT));
	pUnitychan->SetPixelShader(GetPS(PS_POINTLIGHT));
	pUnitychan->Draw();

	GetObj<Model>("room")->Draw();

	// ���C�g�̕`��
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
		// ���C�g�̐F
		pLightColor->Write(&m_lights[i].color);
		GetObj<Model>("lightModel")->Draw();
	}
}