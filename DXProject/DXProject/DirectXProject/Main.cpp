#include "Main.h"
#include "DirectX.h"
#include "RenderTarget.h"
#include "TextureFactory.h"
#include <memory>	// スマートポインタ
#include "DXBuffer.h"	// 頂点バッファを作成
#include "Shader.h"	// シェーダーの読込
#include "ConstantBuffer.h"	// 定数バッファ
#include <DirectXMath.h>
#include "FBXParser.h"
#include <filesystem>
#include "FBXPlayer.h"
#include "Geometory.h"

// --- グローバル変数
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

	// レンダーターゲット作成
	g_pRTV = dynamic_cast<RenderTarget*>(TextureFactory::CreateRenderTargetFromScreen());
	// ビューポートの設定(描き込める領域の設定)
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetContext()->RSSetViewports(1, &vp);



	// 頂点バッファ作成
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
		// --- 「ファイルを開く」ダイアログの表示
			// ダイアログの初期設定
		//char fileName[MAX_PATH] = "";
		//OPENFILENAME ofn;
		//ZeroMemory(&ofn, sizeof(ofn));
		//ofn.lStructSize = sizeof(ofn);
		//ofn.hwndOwner = GetActiveWindow();
		//ofn.lpstrFile = fileName;	// 開くで選択したファイルのパスを格納
		//ofn.nMaxFile = sizeof(fileName);
		//// ダイアログ右下の、ファイル拡張子のフィルター設定
		//// [表示名\0拡張子\0]の組み合わせ、\0で区切る
		//ofn.lpstrFilter = "All Files\0*.*\0.fbx\0*.fbx\0";
		//ofn.nFilterIndex = 2;	// 最初に指定しているフィルター
		//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		//if (TRUE == GetOpenFileName(&ofn))
		//{
		//	// 正しく開けたかコンソール画面に出力して確認
		//	printf("開いたファイル：%s\n", fileName);

		//}
#endif

		// ダイアログを表示
		FBXParser fbx;
		if (!fbx.Load("cone.fbx"))
		{
			return E_FAIL;
		}

		// OpenGLではglNormal3fやglVertex3fで
		// 個別にデータを指定したが、DirectXでは
		// 頂点バッファを作成する際に、一つの
		// 頂点の情報を全て設定する。
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
			// 法線
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
			// 頂点
			data.pos[0] = pVtx[idx].x;
			data.pos[1] = pVtx[idx].y;
			data.pos[2] = pVtx[idx].z;
			// 動的配列へ追加
			vtx.push_back(data);
		}
		
		DXBuffer::Desc desc = {};
		desc.pVtx = vtx.data();	// 頂点のデータ
		desc.vtxCount = vtx.size();	// 頂点の数
		desc.vtxSize = sizeof(Vertex);	// 1頂点のデータ数
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	// 頂点の結び方
		// 頂点バッファ生成
		g_pTriangle = std::make_shared<DXBuffer>();
		hr = g_pTriangle->Create(desc);
		if (FAILED(hr)) { return hr; }

		vtx.clear();
		// vector
		auto nrm = fbx.DebugDrawNormal();
		// イテレーター
		auto it = nrm.begin();
		Vertex data;
		for (; it != nrm.end(); ++it)
		{
			data.pos[0] = (*it).x;
			data.pos[1] = (*it).y;
			data.pos[2] = (*it).z;
			vtx.push_back(data);
			
		}
		desc.pVtx = vtx.data();	// 頂点のデータ
		desc.vtxCount = vtx.size();	// 頂点の数
		desc.vtxSize = sizeof(Vertex);	// 1頂点のデータ数
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;	// 頂点の結び方
		// 頂点バッファ作成
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

		desc.pVtx = vtx.data();	// 頂点のデータ
		desc.vtxCount = vtx.size();	// 頂点の数
		desc.vtxSize = sizeof(Vertex);	// 1頂点のデータ数
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;	// 頂点の結び方
		// 頂点バッファ作成
		g_pGrid = std::make_shared<DXBuffer>();
		hr = g_pGrid->Create(desc);
		if (FAILED(hr)) { return hr; }
		//D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		
		//--- FBXSDKを利用してモデル読み込み
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

	// --- シェーダーの読込
	// 頂点シェーダー
	g_pSampleVS = std::make_shared<VertexShader>();
	hr = g_pSampleVS->Load("Assets/SampleVS.cso");
	if (FAILED(hr)) { return hr; }
	// アニメーションシェーダー
	g_pAnimationVS = std::make_shared<VertexShader>();
	hr = g_pAnimationVS->Load("Assets/AnimationVS.cso");
	if (FAILED(hr)) { return hr; }
	// ピクセルシェーダー
	g_pSamplePS = std::make_shared<PixelShader>();
	hr = g_pSamplePS->Load("Assets/SamplePS.cso");
	if (FAILED(hr)) { return hr; }
	// ランバートシェーダー
	g_pLambertPS = std::make_shared<PixelShader>();
	hr = g_pLambertPS->Load("Assets/LambertPS.cso");
	if (FAILED(hr)) { return hr; }

	// --- 定数バッファの作成
	// 変換行列
	g_pWVPBuf = std::make_shared<ConstantBuffer>();
	hr = g_pWVPBuf->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	if (FAILED(hr)) { return hr; }
	// ライト
	g_pLightBuf = std::make_shared<ConstantBuffer>();
	hr = g_pLightBuf->Create(sizeof(DirectX::XMFLOAT4));
	if (FAILED(hr)) { return hr; }
	// アニメーション用の定数バッファ
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
	// キー入力でアニメーションを切り替え
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

	// レンダーターゲットの設定(GPUの描画先を設定
	ID3D11RenderTargetView* pViews[] = {
		g_pRTV->GetView()
	};
	GetContext()->OMSetRenderTargets(
		1, pViews, nullptr
	);

	// 変換行列作成
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
	// 計算用の型から格納用の型に変換
	// シェーダー上では行列の順番が縦並びのため、
	// 行列を転置(斜めにひっくり返す)する
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	DirectX::XMStoreFloat4x4(&fMat[1], DirectX::XMMatrixTranspose(mat[1]));
	DirectX::XMStoreFloat4x4(&fMat[2], DirectX::XMMatrixTranspose(mat[2]));
	// ローカル変数の内容を定数バッファに描きこむ
	g_pWVPBuf->Write(fMat);
	// 定数バッファをGPUへ設定
	g_pWVPBuf->BindVS(0);

	// ライト
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

	// モデルの表示
	auto func = [](int mesh)	// メッシュの描画ごとに呼び出される関数
	{
		// メッシュごとに必要となる骨が異なる。(頭の時に下半身はいらない)
		DirectX::XMMATRIX bone;
		DirectX::XMFLOAT4X4 boneBuf[SHADER_BONE_NUM];

		// メッシュで必要になる骨の情報を取得
		std::vector<FBXPlayer::MeshInverse>& meshInv =
			g_fbxPlayer->GetMeshInverse();
		for (int i = 0; i < meshInv[mesh].num; ++i)
		{
			// メッシュで必要になる骨の情報を取得
			boneBuf[i] = g_fbxPlayer->GetBone(meshInv[mesh].pList[i].boneIndex);

			// 頂点座標を逆行列で一度原点に戻す
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
