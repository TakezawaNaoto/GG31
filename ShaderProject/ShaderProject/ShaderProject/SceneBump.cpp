#include "SceneBump.h"
#include "Model.h"
#include "TextureFactory.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "CameraBase.h"
#include "Input.h"

struct BumpVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 tangent;	// �ڃx�N�g��
	// ���[���h��ԏ�Ńe�N�X�`����\��t���鉡�̕���
};

void SceneBump::Init()
{
	Model* pModel = GetObj<Model>("UnityModel");
	pModel->SetVertexShader(GetVS(VS_OBJECT));
	pModel->SetPixelShader(GetPS(PS_BUMPMAP));

	// �o���v�}�b�v�p�̃��f���𐶐�
	int meshNum = pModel->GetMeshNum();
	MeshBuffer** ppMesh = new MeshBuffer*[meshNum];
	for (int i = 0; i < meshNum; ++i)
	{
		MeshBuffer::Description desc = {};
		// ���b�V�����Ƃ̒��_�f�[�^���R�s�[
		int vtxNum = pModel->GetMesh(i)->vertexNum;
		Model::Vertex* pVtx = pModel->GetMesh(i)->pVertices;
		BumpVertex* pBumpVtx = new BumpVertex[vtxNum];
		for (int j = 0; j < vtxNum; ++j)
		{
			// �o���v�ɒ��_�f�[�^���R�s�[
			pBumpVtx[j].pos = pVtx[j].pos;
			pBumpVtx[j].normal = pVtx[j].normal;
			pBumpVtx[j].uv = pVtx[j].uv;
		}
		// �ʐ��ɍ��킹�Đڃx�N�g�����v�Z
		int idxNum = pModel->GetMesh(i)->indexNum;
		unsigned int* pIdx = pModel->GetMesh(i)->pIndices;
		int* tangentVtxCnt = new int[vtxNum];	// ��̒��_�������̖ʂɗ��p����Ă���ꍇ
		ZeroMemory(tangentVtxCnt, sizeof(int)*vtxNum);
		for (int j = 0; j < idxNum; j += 3)
		{
			// �ʂ��\������C���f�b�N�X
			int idx0 = pIdx[j + 0];
			int idx1 = pIdx[j + 1];
			int idx2 = pIdx[j + 2];
			// �ڃx�N�g�����̊i�[��
			DirectX::XMFLOAT3* pTangents[] = {
				&pBumpVtx[idx0].tangent,
				&pBumpVtx[idx1].tangent,
				&pBumpVtx[idx2].tangent,
			};
			DirectX::XMFLOAT3 v0(
				pBumpVtx[idx1].pos.x - pBumpVtx[idx0].pos.x,
				pBumpVtx[idx1].pos.y - pBumpVtx[idx0].pos.y,
				pBumpVtx[idx1].pos.z - pBumpVtx[idx0].pos.z
			);
			DirectX::XMFLOAT3 v1(
				pBumpVtx[idx2].pos.x - pBumpVtx[idx0].pos.x,
				pBumpVtx[idx2].pos.y - pBumpVtx[idx0].pos.y,
				pBumpVtx[idx2].pos.z - pBumpVtx[idx0].pos.z
			);
			DirectX::XMFLOAT2 t0(
				pBumpVtx[idx1].uv.x - pBumpVtx[idx0].uv.x,
				pBumpVtx[idx1].uv.y - pBumpVtx[idx0].uv.y
			);
			DirectX::XMFLOAT2 t1(
				pBumpVtx[idx2].uv.x - pBumpVtx[idx0].uv.x,
				pBumpVtx[idx2].uv.y - pBumpVtx[idx0].uv.y
			);

			float t = 1.0f / (t0.x * t1.y - t1.x * t0.y);
			DirectX::XMVECTOR T = DirectX::XMVectorSet(
				t*(t1.y*v0.x - t0.y*v1.x),
				t*(t1.y*v0.y - t0.y*v1.y),
				t*(t1.y*v0.z - t0.y*v1.z), 0.0f
			);
			T = DirectX::XMVector3Normalize(T);
			// �ڃx�N�g���͍��Z���čŌ�ɕ��ς��o��
			for (int k = 0; k < 3; ++k)
			{
				DirectX::XMStoreFloat3(pTangents[k],
					DirectX::XMVectorAdd(T, DirectX::XMLoadFloat3(pTangents[k])));
				tangentVtxCnt[pIdx[j + k]]++;
			}
		}
		// �ڃx�N�g���̕��ς��Z�o
		for (int j = 0; j < vtxNum; ++j)
		{
			pBumpVtx[j].tangent.x /= tangentVtxCnt[j];
			pBumpVtx[j].tangent.y /= tangentVtxCnt[j];
			pBumpVtx[j].tangent.z /= tangentVtxCnt[j];
		}

		// ���f���f�[�^���쐬
		desc.pVtx = pBumpVtx;
		desc.vtxSize = sizeof(BumpVertex);
		desc.vtxCount = vtxNum;
		desc.pIdx = pIdx;
		desc.idxSize = sizeof(unsigned int);
		desc.idxCount = idxNum;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		ppMesh[i] = new MeshBuffer(desc);

		// �s�v�f�[�^�폜
		delete[] pBumpVtx;
		delete[] tangentVtxCnt;
	}
	RegisterObj<MeshBuffer*>("UnityBump", ppMesh);

	// ��
	pModel = CreateObj<Model>("SpiderModel");
	// pModel->Load("Assets/Model/Cotoon_land2.fbx");
	pModel->Load("Assets/Model/Spider.fbx");
	pModel->SetVertexShader(GetVS(VS_BUMP));	// VS_OBJECT
	pModel->SetPixelShader(GetPS(PS_BUMPMAP));	// PS_LAMBERT

	// ��
	pModel = CreateObj<Model>("TreeModel");
	// pModel->Load("Assets/Model/Cotoon_land2.fbx");
	pModel->Load("Assets/Model/tree.fbx");
	pModel->SetVertexShader(GetVS(VS_BUMP));	// VS_OBJECT
	pModel->SetPixelShader(GetPS(PS_BUMPMAP));	// PS_LAMBERT

	// �@���}�b�v�̓ǂݍ���
	Texture* pTex = TextureFactory::CreateFromFile(
		"Assets/blockNormal.png");
	RegisterObj<Texture>("NormalMap", pTex);

	// �J�����̒萔�o�b�t�@
	ConstantBuffer* pCamBuf = CreateObj<ConstantBuffer>("CameraCB");
	pCamBuf->Create(sizeof(DirectX::XMFLOAT4) * 2);

	// ������
	m_rad = 0.0f;
}
void SceneBump::Uninit()
{
}
void SceneBump::Update()
{
	if (IsKeyPress(VK_LEFT)) m_rad -= 0.01f;
	if (IsKeyPress(VK_RIGHT)) m_rad += 0.01f;
}
void SceneBump::Draw()
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

	// ���C�g�̏���ݒ�
	DirectX::XMFLOAT4 lightDir(
		cosf(m_rad), -1.0f, sinf(m_rad), 0.0f);
	pLight->Write(&lightDir);
	pLight->BindPS(0);
	// �J�����̏���ݒ�
	DirectX::XMFLOAT4 camBuf[] = {
		DirectX::XMFLOAT4(pCamera->GetPos().x, pCamera->GetPos().y, pCamera->GetPos().z, 0.0f),
		DirectX::XMFLOAT4(10.0f, 30.0f, 0.0f, 0.0f)
	};

	// �@���}�b�v�ݒ�
	Texture* pTex = GetObj<Texture>("NormalMap");
	SetTexturePS(pTex->GetResource(), 1);

	Model* pModel = GetObj<Model>("UnityModel");
	pLight->BindVS(1);
	GetVS(VS_BUMP)->Bind();
	GetPS(PS_BUMPMAP)->Bind();

	MeshBuffer** pMeshes = GetObj<MeshBuffer*>("UnityBump");
	for (int i = 0; i < pModel->GetMeshNum(); ++i)
	{
		pMeshes[i]->Draw();
	}

	GetObj<Model>("SpiderModel")->Draw();
	GetObj<Model>("TreeModel")->Draw();
}