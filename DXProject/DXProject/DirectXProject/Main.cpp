#include "Main.h"
#include "DirectX.h"
#include "RenderTarget.h"
#include "TextureFactory.h"
#include <memory>	// �X�}�[�g�|�C���^
#include "DXBuffer.h"	// ���_�o�b�t�@���쐬
#include "Shader.h"	// �V�F�[�_�[�̓Ǎ�
#include "ConstantBuffer.h"	// �萔�o�b�t�@
#include <DirectXMath.h>
#include "FBXParser.h"
#include <filesystem>
#include "FBXPlayer.h"
#include "Geometory.h"

// --- �O���[�o���ϐ�
RenderTarget *g_pRTV;
std::shared_ptr<DXBuffer> g_pTriangle;
std::shared_ptr<DXBuffer> g_pGrid;
std::shared_ptr<DXBuffer> g_pNrm;
std::shared_ptr<VertexShader> g_pSampleVS;
std::shared_ptr<VertexShader> g_pAnimationVS;
std::shared_ptr<PixelShader> g_pSamplePS;
std::shared_ptr<PixelShader> g_pLambertPS;
std::shared_ptr<ConstantBuffer> g_pWVPBuf;	// WorldViewProj
std::shared_ptr<ConstantBuffer> g_pLightBuf;
std::shared_ptr<ConstantBuffer> g_boneBuf;
std::shared_ptr<FBXPlayer> g_fbxPlayer;
const int SHADER_BONE_NUM = 200;

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	hr = InitDX(hWnd, width, height, false);
	if (FAILED(hr)) { return hr; }
	ggfbx::Initialize();

	InitGeometory();

	// �����_�[�^�[�Q�b�g�쐬
	g_pRTV = dynamic_cast<RenderTarget*>(TextureFactory::CreateRenderTargetFromScreen());
	// �r���[�|�[�g�̐ݒ�(�`�����߂�̈�̐ݒ�)
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);



	// ���_�o�b�t�@�쐬
	struct Vertex 
	{
		float pos[3];
		float normal[3];
		float uv[2];
	};
#if 0
	Vertex vtx[] =
	{
		{{0.0f, 1.0f, 0.0f},  {0, 0, -1}, {0.5f, 0.0f}},
		{{1.0f, -1.0f, 0.0f}, {0, 0, -1}, {1.0f, 0.0f}},
		{{-1.0f, -1.0f, 0.0f},{0, 0, -1}, {0.0f, 0.0f}}
	};
#else
		// --- �u�t�@�C�����J���v�_�C�A���O�̕\��
			// �_�C�A���O�̏����ݒ�
		//char fileName[MAX_PATH] = "";
		//OPENFILENAME ofn;
		//ZeroMemory(&ofn, sizeof(ofn));
		//ofn.lStructSize = sizeof(ofn);
		//ofn.hwndOwner = GetActiveWindow();
		//ofn.lpstrFile = fileName;	// �J���őI�������t�@�C���̃p�X���i�[
		//ofn.nMaxFile = sizeof(fileName);
		//// �_�C�A���O�E���́A�t�@�C���g���q�̃t�B���^�[�ݒ�
		//// [�\����\0�g���q\0]�̑g�ݍ��킹�A\0�ŋ�؂�
		//ofn.lpstrFilter = "All Files\0*.*\0.fbx\0*.fbx\0";
		//ofn.nFilterIndex = 2;	// �ŏ��Ɏw�肵�Ă���t�B���^�[
		//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		//if (TRUE == GetOpenFileName(&ofn))
		//{
		//	// �������J�������R���\�[����ʂɏo�͂��Ċm�F
		//	printf("�J�����t�@�C���F%s\n", fileName);

		//}
#endif

		// �_�C�A���O��\��
		FBXParser fbx;
		if (!fbx.Load("cone.fbx"))
		{
			return E_FAIL;
		}

		// OpenGL�ł�glNormal3f��glVertex3f��
		// �ʂɃf�[�^���w�肵�����ADirectX�ł�
		// ���_�o�b�t�@���쐬����ۂɁA���
		// ���_�̏���S�Đݒ肷��B
		FBXParser::Vertex* pVtx = fbx.GetVertex();
		unsigned long* pIdx = fbx.GetIndex();
		FBXParser::Normal* pNormal = fbx.GetNormal();
		unsigned long* pNrmIdx = fbx.GetNormalIndex();
		FBXParser::UV* pUV = fbx.GetUV();
		unsigned long* pUVIdx = fbx.GetUVIndex();

		std::vector<Vertex> vtx;
		for (int i = 0; i < fbx.GetIndexNum(); ++i)
		{
			Vertex data;
			int idx = pIdx[i];
			// �@��
			int nrmIdx = fbx.IsNormalMappingPolygon() ? i : idx;
			if (fbx.IsNormalReferenceIndex()) { nrmIdx = pNrmIdx[nrmIdx]; }
			data.normal[0] = pNormal[nrmIdx].x;
			data.normal[1] = pNormal[nrmIdx].y;
			data.normal[2] = pNormal[nrmIdx].z;
			// UV
			int uvIdx = fbx.IsMappingPolygon() ? i : idx;
			if (fbx.IsUVReferenceIndex()) { uvIdx = pUVIdx[uvIdx]; }
			data.uv[0] = pUV[uvIdx].u;
			data.uv[1] = pUV[uvIdx].v;
			// ���_
			data.pos[0] = pVtx[idx].x;
			data.pos[1] = pVtx[idx].y;
			data.pos[2] = pVtx[idx].z;
			// ���I�z��֒ǉ�
			vtx.push_back(data);
		}
		
		DXBuffer::Desc desc = {};
		desc.pVtx = vtx.data();	// ���_�̃f�[�^
		desc.vtxCount = vtx.size();	// ���_�̐�
		desc.vtxSize = sizeof(Vertex);	// 1���_�̃f�[�^��
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	// ���_�̌��ѕ�
		// ���_�o�b�t�@����
		g_pTriangle = std::make_shared<DXBuffer>();
		hr = g_pTriangle->Create(desc);
		if (FAILED(hr)) { return hr; }

		vtx.clear();
		// vector
		auto nrm = fbx.DebugDrawNormal();
		// �C�e���[�^�[
		auto it = nrm.begin();
		Vertex data;
		for (; it != nrm.end(); ++it)
		{
			data.pos[0] = (*it).x;
			data.pos[1] = (*it).y;
			data.pos[2] = (*it).z;
			vtx.push_back(data);
			
		}
		desc.pVtx = vtx.data();	// ���_�̃f�[�^
		desc.vtxCount = vtx.size();	// ���_�̐�
		desc.vtxSize = sizeof(Vertex);	// 1���_�̃f�[�^��
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;	// ���_�̌��ѕ�
		// ���_�o�b�t�@�쐬
		g_pNrm = std::make_shared<DXBuffer>();
		hr = g_pNrm->Create(desc);
		if (FAILED(hr)) { return hr; }
		//D3D11_PRIMITIVE_TOPOLOGY_LINELIST

		// grid
		vtx.clear();

		for (int x = 0; x < 10; ++x) {

			//for (int y = 0; y < 10; ++y)
			{
				data.pos[0] = -5.0f + x * 1;
				data.pos[1] = 0.0f;
				data.pos[2] = -5.0f;
				vtx.push_back(data);

				data.pos[0] = -5.0f + x * 1;
				data.pos[1] = 0.0f;
				data.pos[2] = 5.0f;
				vtx.push_back(data);
			}
		}

		for (int x = 0; x < 10; ++x) {

			//for (int y = 0; y < 10; ++y)
			{
				data.pos[0] = -5.0f;
				data.pos[1] = 0.0f;
				data.pos[2] = -5.0f + x * 1;
				vtx.push_back(data);

				data.pos[0] = 5.0f;
				data.pos[1] = 0.0f;
				data.pos[2] = -5.0f + x * 1;
				vtx.push_back(data);
			}
		}

		data.pos[0] = 0.0f;
		data.pos[1] = 0.0f;
		data.pos[2] = 0.0f;
		vtx.push_back(data);

		data.pos[0] = 10.0f;
		data.pos[1] = 0.0f;
		data.pos[2] = 0.0f;
		vtx.push_back(data);

		desc.pVtx = vtx.data();	// ���_�̃f�[�^
		desc.vtxCount = vtx.size();	// ���_�̐�
		desc.vtxSize = sizeof(Vertex);	// 1���_�̃f�[�^��
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;	// ���_�̌��ѕ�
		// ���_�o�b�t�@�쐬
		g_pGrid = std::make_shared<DXBuffer>();
		hr = g_pGrid->Create(desc);
		if (FAILED(hr)) { return hr; }
		//D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		
		//--- FBXSDK�𗘗p���ă��f���ǂݍ���
		g_fbxPlayer = std::make_shared<FBXPlayer>();
		if (!g_fbxPlayer->Load("Assets/unitychan/unitychan.fbx"))
		{
			return E_FAIL;
		}

		g_fbxPlayer->LoadAnime("Assets/unitychan/run.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/jump.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/Capoeira.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/Dancing Twerk.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/Hip Hop Dancing.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/Rumba Dancing.fbx");
		g_fbxPlayer->LoadAnime("Assets/unitychan/Sitting Laughing.fbx");
		g_fbxPlayer->PlayAnime(0, true);

		hr=InitGeometory();

	// --- �V�F�[�_�[�̓Ǎ�
	// ���_�V�F�[�_�[
	g_pSampleVS = std::make_shared<VertexShader>();
	hr = g_pSampleVS->Load("Assets/SampleVS.cso");
	if (FAILED(hr)) { return hr; }
	// �A�j���[�V�����V�F�[�_�[
	g_pAnimationVS = std::make_shared<VertexShader>();
	hr = g_pAnimationVS->Load("Assets/AnimationVS.cso");
	if (FAILED(hr)) { return hr; }
	// �s�N�Z���V�F�[�_�[
	g_pSamplePS = std::make_shared<PixelShader>();
	hr = g_pSamplePS->Load("Assets/SamplePS.cso");
	if (FAILED(hr)) { return hr; }
	// �����o�[�g�V�F�[�_�[
	g_pLambertPS = std::make_shared<PixelShader>();
	hr = g_pLambertPS->Load("Assets/LambertPS.cso");
	if (FAILED(hr)) { return hr; }

	// --- �萔�o�b�t�@�̍쐬
	// �ϊ��s��
	g_pWVPBuf = std::make_shared<ConstantBuffer>();
	hr = g_pWVPBuf->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	if (FAILED(hr)) { return hr; }
	// ���C�g
	g_pLightBuf = std::make_shared<ConstantBuffer>();
	hr = g_pLightBuf->Create(sizeof(DirectX::XMFLOAT4));
	if (FAILED(hr)) { return hr; }
	// �A�j���[�V�����p�̒萔�o�b�t�@
	g_boneBuf = std::make_shared<ConstantBuffer>();
	hr = g_boneBuf->Create(sizeof(DirectX::XMFLOAT4X4) * SHADER_BONE_NUM);
	if (FAILED(hr)) { return hr; }

	return hr;
}

void Uninit()
{
	g_boneBuf.reset();
	g_pLightBuf.reset();
	g_pWVPBuf.reset();
	g_pLambertPS.reset();
	g_pSamplePS.reset();
	g_pAnimationVS.reset();
	g_pSampleVS.reset();
	g_pTriangle.reset();
	delete g_pRTV;
	ggfbx::Terminate();
	UninitDX();
}

void Update()
{
	// �L�[���͂ŃA�j���[�V������؂�ւ�
	// jump
	if (GetAsyncKeyState('1') & 0x8000)
	{
		g_fbxPlayer->PlayAnime(1);
	}
	// jump
	if (GetAsyncKeyState('2') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(1, 0.5f);
	}
	// Capoeira
	if (GetAsyncKeyState('3') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(2, 0.5f);
	}
	// Dancing Twerk
	if (GetAsyncKeyState('4') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(3, 0.5f);
	}
	// Hip Hop Dancing
	if (GetAsyncKeyState('5') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(4, 0.5f);
	}
	// Rumba Dancing
	if (GetAsyncKeyState('6') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(5, 0.5f);
	}
	// Sitting Laughing
	if (GetAsyncKeyState('7') & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(6, 0.5f);
	}
	// run
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		g_fbxPlayer->PlayBlendAnime(0, true, 1);
	}


	g_fbxPlayer->Step();
}

void Draw()
{
	float color[4] = { 0.8f, 0.8f, 0.9f, 1.0f };
	GetContext()->ClearRenderTargetView(g_pRTV->GetView(), color);

	// �����_�[�^�[�Q�b�g�̐ݒ�(GPU�̕`����ݒ�
	ID3D11RenderTargetView* pViews[] = {
		g_pRTV->GetView()
	};
	GetContext()->OMSetRenderTargets(
		1, pViews, nullptr
	);

	// �ϊ��s��쐬
	DirectX::XMFLOAT4X4 fMat[3];
	DirectX::XMMATRIX mat[3];
	mat[0] = DirectX::XMMatrixIdentity();
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(30.0f, 80.0f, -250.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 80.0f, 0.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.0f / 9.0f, 0.1f, 1000.0f
	);
	// �v�Z�p�̌^����i�[�p�̌^�ɕϊ�
	// �V�F�[�_�[��ł͍s��̏��Ԃ��c���т̂��߁A
	// �s���]�u(�΂߂ɂЂ�����Ԃ�)����
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	DirectX::XMStoreFloat4x4(&fMat[1], DirectX::XMMatrixTranspose(mat[1]));
	DirectX::XMStoreFloat4x4(&fMat[2], DirectX::XMMatrixTranspose(mat[2]));
	// ���[�J���ϐ��̓��e��萔�o�b�t�@�ɕ`������
	g_pWVPBuf->Write(fMat);
	// �萔�o�b�t�@��GPU�֐ݒ�
	g_pWVPBuf->BindVS(0);

	// ���C�g
	static float q;
	q += 0.03f;
	DirectX::XMFLOAT4 light(sin(q), -1.0f, cos(q), 0.0f);
	g_pLightBuf->Write(&light);
	g_pLightBuf->BindPS(0);

	g_pSampleVS->Bind();
	
	// g_pSamplePS->Bind();
	g_pLambertPS->Bind();
	g_pTriangle->Draw();
	// g_pGrid->Draw();
	// g_pNrm->Draw();

	// ���f���̕\��
	auto func = [](int mesh)	// ���b�V���̕`�悲�ƂɌĂяo�����֐�
	{
		// ���b�V�����ƂɕK�v�ƂȂ鍜���قȂ�B(���̎��ɉ����g�͂���Ȃ�)
		DirectX::XMMATRIX bone;
		DirectX::XMFLOAT4X4 boneBuf[SHADER_BONE_NUM];

		// ���b�V���ŕK�v�ɂȂ鍜�̏����擾
		std::vector<FBXPlayer::MeshInverse>& meshInv =
			g_fbxPlayer->GetMeshInverse();
		for (int i = 0; i < meshInv[mesh].num; ++i)
		{
			// ���b�V���ŕK�v�ɂȂ鍜�̏����擾
			boneBuf[i] = g_fbxPlayer->GetBone(meshInv[mesh].pList[i].boneIndex);

			// ���_���W���t�s��ň�x���_�ɖ߂�
			bone = DirectX::XMLoadFloat4x4(&boneBuf[i]);
			bone = DirectX::XMMatrixTranspose(meshInv[mesh].pList[i].invMat * bone);
			DirectX::XMStoreFloat4x4(&boneBuf[i], bone);
		}
		g_boneBuf->Write(boneBuf);
		g_boneBuf->BindVS(1);
	};

	g_pAnimationVS->Bind();
	g_fbxPlayer->Draw(func);
	g_fbxPlayer->DrawBone();

	SwapDX();
}

// EOF
