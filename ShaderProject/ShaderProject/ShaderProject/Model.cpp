#include "Model.h"
#include "DirectXTex/TextureLoad.h"
#include "WinDialog.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#if _MSC_VER >= 1910
#pragma comment(lib, "assimp-vc141-mtd.lib")
#elif _MSC_VER >= 1930
#pragma comment(lib, "assimp-vc142-mtd.lib")
#endif

VertexShader* Model::m_pDefVS = nullptr;
PixelShader* Model::m_pDefPS = nullptr;
unsigned int Model::m_shaderRef = 0;

//--- プロトタイプ宣言
void MakeModelDefaultShader(VertexShader** vs, PixelShader** ps);

Model::Model()
	: m_meshNum(0)
	, m_pMeshes(nullptr)
	, m_materialNum(0)
	, m_pMaterials(nullptr)
{
	if (m_shaderRef == 0)
	{
		MakeModelDefaultShader(&m_pDefVS, &m_pDefPS);
	}
	m_pVS = m_pDefVS;
	m_pPS = m_pDefPS;
	++m_shaderRef;
}
Model::~Model()
{
	for (unsigned int i = 0; i < m_meshNum; ++i)
	{
		delete[] m_pMeshes[i].pVertices;
		delete[] m_pMeshes[i].pIndices;
		delete m_pMeshes[i].pMesh;
	}
	if (m_pMeshes) {
		delete[] m_pMeshes;
	}
	for (unsigned int i = 0; i < m_materialNum; ++i)
	{
		if (m_pMaterials[i].pTexture)
			m_pMaterials[i].pTexture->Release();
	}
	if (m_pMaterials) {
		delete[] m_pMaterials;
	}

	--m_shaderRef;
	if (m_shaderRef <= 0)
	{
		delete m_pDefPS;
		delete m_pDefVS;
	}
}

void Model::SetVertexShader(VertexShader* vs)
{
	m_pVS = vs;
}
void Model::SetPixelShader(PixelShader* ps)
{
	m_pPS = ps;
}
const Model::Mesh* Model::GetMesh(unsigned int index)
{
	if (index <= GetMeshNum())
	{
		return &m_pMeshes[index];
	}
	return nullptr;
}
uint32_t Model::GetMeshNum()
{
	return m_meshNum;
}

void MakeModelDefaultShader(VertexShader** vs, PixelShader** ps)
{
	const char* ModelVS = R"EOT(
struct VS_IN {
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
VS_OUT main(VS_IN vin) {
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos.z += 0.5f;
	vout.pos.y -= 0.8f;
	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
})EOT";
	const char* ModelPS = R"EOT(
struct PS_IN {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
float4 main(PS_IN pin) : SV_TARGET
{
	return float4(pin.normal, 1.0f);
})EOT";
	*vs = new VertexShader();
	(*vs)->Compile(ModelVS);
	*ps = new PixelShader();
	(*ps)->Compile(ModelPS);
}

bool Model::Load(const char* file, float scale, bool flip)
{
	// assimpの設定
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_JoinIdenticalVertices;
	flag |= aiProcess_FlipUVs;
	flag |= aiProcess_PreTransformVertices;
	if (flip) flag |= aiProcess_MakeLeftHanded;
	// assimpで読み込み
	const aiScene* pScene = importer.ReadFile(file, flag);
	if (!pScene) {
		DialogError(STACK_DEFAULT, importer.GetErrorString());
		return false;
	}


	// メッシュの作成
	m_meshNum = pScene->mNumMeshes;
	m_pMeshes = new Mesh[m_meshNum];
	for (unsigned int i = 0; i < m_meshNum; ++i)
	{
		// 頂点の作成
		aiVector3D zero(0.0f, 0.0f, 0.0f);
		m_pMeshes[i].vertexNum = pScene->mMeshes[i]->mNumVertices;
		m_pMeshes[i].pVertices = new Model::Vertex[m_pMeshes[i].vertexNum];
		for (unsigned int j = 0; j < m_pMeshes[i].vertexNum; ++j)
		{
			// 値の吸出し
			aiVector3D pos = pScene->mMeshes[i]->mVertices[j];
			aiVector3D uv = pScene->mMeshes[i]->HasTextureCoords(0) ?
				pScene->mMeshes[i]->mTextureCoords[0][j] : zero;
			aiVector3D normal = pScene->mMeshes[i]->HasNormals() ?
				pScene->mMeshes[i]->mNormals[j] : zero;
			// 値を設定
			m_pMeshes[i].pVertices[j] = {
				DirectX::XMFLOAT3(pos.x * scale, pos.y * scale, pos.z * scale),
				DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
				DirectX::XMFLOAT2(uv.x, uv.y),
			};
		}

		// インデックスの作成
		m_pMeshes[i].indexNum = pScene->mMeshes[i]->mNumFaces * 3; // faceはポリゴンの数を表す(１ポリゴンで3インデックス
		m_pMeshes[i].pIndices = new unsigned int[m_pMeshes[i].indexNum];
		for (unsigned int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j)
		{
			aiFace face = pScene->mMeshes[i]->mFaces[j];
			int idx = j * 3;
			m_pMeshes[i].pIndices[idx + 0] = face.mIndices[0];
			m_pMeshes[i].pIndices[idx + 1] = face.mIndices[1];
			m_pMeshes[i].pIndices[idx + 2] = face.mIndices[2];
		}

		// マテリアルの割り当て
		m_pMeshes[i].materialID = pScene->mMeshes[i]->mMaterialIndex;

		// メッシュを元に頂点バッファ作成
		MeshBuffer::Description desc = {};
		desc.pVtx = m_pMeshes[i].pVertices;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = m_pMeshes[i].vertexNum;
		desc.pIdx = m_pMeshes[i].pIndices;
		desc.idxSize = sizeof(unsigned int);
		desc.idxCount = m_pMeshes[i].indexNum;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pMeshes[i].pMesh = new MeshBuffer(desc);
	}

	//--- マテリアルの作成
	// ファイルの探索
	std::string dir = file;
	dir = dir.substr(0, dir.find_last_of('/') + 1);
	// マテリアル
	m_materialNum = pScene->mNumMaterials;
	m_pMaterials = new Material[m_materialNum];
	for (unsigned int i = 0; i < m_materialNum; ++i)
	{
		// 各種パラメーター
		aiColor3D color(0.0f, 0.0f, 0.0f);
		float shininess;
		m_pMaterials[i].diffuse = pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f) :
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pMaterials[i].ambient = pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f) :
			DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		shininess = pScene->mMaterials[i]->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS ? shininess : 0.0f;
		m_pMaterials[i].specular = pScene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS ?
			DirectX::XMFLOAT4(color.r, color.g, color.b, shininess) :
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, shininess);

		// テクスチャ
		HRESULT hr;
		aiString path;
		if (pScene->mMaterials[i]->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS) {
			// そのまま探索
			hr = LoadTextureFromFile(GetDevice(), path.C_Str(), &m_pMaterials[i].pTexture);
			// モデルと同じ階層を探索
			if (FAILED(hr)) {
				std::string file = dir;
				file += path.C_Str();
				hr = LoadTextureFromFile(GetDevice(), file.c_str(), &m_pMaterials[i].pTexture);
			}
			if (FAILED(hr)) {
				m_pMaterials[i].pTexture = nullptr;
			}
		}
		else {
			m_pMaterials[i].pTexture = nullptr;
		}
	}

	return true;
}

void Model::Draw()
{
	m_pVS->Bind();
	m_pPS->Bind();
	for (unsigned int i = 0; i < m_meshNum; ++i)
	{
		SetTexturePS(m_pMaterials[m_pMeshes[i].materialID].pTexture, 0);
		m_pMeshes[i].pMesh->Draw();
	}
}