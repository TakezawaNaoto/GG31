#include "Geometory.h"
#include "MeshBuffer.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include <memory>

//--- 定数
const int GeometoryLineNum = 1000;
const int GeometoryLineVtxNum = GeometoryLineNum * 2;

//--- 構造体
struct GeometoryVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

//--- グローバル変数
UINT g_geometoryLineIndex;
GeometoryVertex g_geometoryLineVtx[GeometoryLineVtxNum];
std::shared_ptr<MeshBuffer> g_pGeometoryLinesMesh;
std::shared_ptr<MeshBuffer> g_pGeometoryBoxMesh;
std::shared_ptr<VertexShader> g_pGeometoryVS;
std::shared_ptr<PixelShader> g_pGeometoryPS;
std::shared_ptr<ConstantBuffer> g_pGeometoryWVP;
std::shared_ptr<ConstantBuffer> g_pGeometoryColor;
DirectX::XMFLOAT4X4 g_geometoryWVPMat[3];
DirectX::XMFLOAT4 g_geometoryColor;

void InitGeometory()
{
	// 直線
	MeshBuffer::Description lineDesc = {};
	lineDesc.pVtx = g_geometoryLineVtx;
	lineDesc.vtxSize = sizeof(GeometoryVertex);
	lineDesc.vtxCount = GeometoryLineVtxNum;
	lineDesc.isWrite = true;
	lineDesc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	g_geometoryLineIndex = 0;
	g_pGeometoryLinesMesh = std::make_shared<MeshBuffer>(lineDesc);

	// ボックス
	const float d = 0.5f;
	DirectX::XMFLOAT4 boxColor(1.0f, 1.0f, 1.0f, 1.0f);
	GeometoryVertex boxVtx[] = {
		{DirectX::XMFLOAT3(-d, d,-d), boxColor},
		{DirectX::XMFLOAT3( d, d,-d), boxColor},
		{DirectX::XMFLOAT3(-d,-d,-d), boxColor},
		{DirectX::XMFLOAT3( d,-d,-d), boxColor},
		{DirectX::XMFLOAT3(-d, d, d), boxColor},
		{DirectX::XMFLOAT3( d, d, d), boxColor},
		{DirectX::XMFLOAT3(-d,-d, d), boxColor},
		{DirectX::XMFLOAT3( d,-d, d), boxColor},
	};
	BYTE boxIdx[] = {
		0,1,2, 2,1,3,
		1,5,3, 3,5,7,
		5,4,7, 7,4,6,
		4,0,6, 6,0,2,
		4,5,0, 0,5,1,
		2,3,6, 6,3,7
	};
	MeshBuffer::Description boxDesc = {};
	boxDesc.pVtx = boxVtx;
	boxDesc.vtxSize = sizeof(GeometoryVertex);
	boxDesc.vtxCount = _countof(boxVtx);
	boxDesc.pIdx = boxIdx;
	boxDesc.idxSize = sizeof(boxIdx[0]);
	boxDesc.idxCount = _countof(boxIdx);
	boxDesc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	g_pGeometoryBoxMesh = std::make_shared<MeshBuffer>(boxDesc);

	// 頂点シェーダー
	g_pGeometoryVS = std::make_shared<VertexShader>();
	g_pGeometoryVS->Compile(R"EOT(
struct VS_IN {
	float3 pos : POSITION0;
	float4 color : COLOR0;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};
cbuffer WVP : register(b0) {
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};
cbuffer Color : register(b1) {
	float4 color;
};
VS_OUT main(VS_IN vin) {
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos = mul(vout.pos, world);
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);
	vout.color = vin.color * color;
	return vout;
}
)EOT");

	// ピクセルシェーダー
	g_pGeometoryPS = std::make_shared<PixelShader>();
	g_pGeometoryPS->Compile(R"EOT(
struct PS_IN {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};
float4 main(PS_IN pin) : SV_TARGET {
	return pin.color;
}
)EOT");

	// 定数バッファ
	g_pGeometoryWVP = std::make_shared<ConstantBuffer>();
	g_pGeometoryWVP->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	g_pGeometoryColor = std::make_shared<ConstantBuffer>();
	g_pGeometoryColor->Create(sizeof(DirectX::XMFLOAT4));
}
void UninitGeometory()
{
	g_pGeometoryLinesMesh.reset();
	g_pGeometoryBoxMesh.reset();
	g_pGeometoryVS.reset();
	g_pGeometoryPS.reset();
	g_pGeometoryWVP.reset();
	g_pGeometoryColor.reset();
}

void SetGeometoryVP(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 proj)
{
	g_geometoryWVPMat[1] = view;
	g_geometoryWVPMat[2] = proj;
}
void SetGeometoryColor(DirectX::XMFLOAT4 color)
{
	g_geometoryColor = color;
}

void DrawBox(DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3 size, DirectX::XMFLOAT3 rot)
{
	DirectX::XMStoreFloat4x4(&g_geometoryWVPMat[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(size.x, size.y, size.z) *
		DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rot.x)) *
		DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rot.y)) *
		DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rot.z)) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)
	));
	g_pGeometoryWVP->Write(g_geometoryWVPMat);
	g_pGeometoryWVP->BindVS(0);
	g_pGeometoryColor->Write(&g_geometoryColor);
	g_pGeometoryColor->BindVS(1);
	g_pGeometoryVS->Bind();
	g_pGeometoryPS->Bind();
	g_pGeometoryBoxMesh->Draw();
}
void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end)
{
	if (g_geometoryLineIndex < GeometoryLineNum)
	{
		g_geometoryLineVtx[g_geometoryLineIndex].pos = start;
		g_geometoryLineVtx[g_geometoryLineIndex].color = g_geometoryColor;
		++g_geometoryLineIndex;
		g_geometoryLineVtx[g_geometoryLineIndex].pos = end;
		g_geometoryLineVtx[g_geometoryLineIndex].color = g_geometoryColor;
		++g_geometoryLineIndex;
	}
}
void DrawLines()
{
	DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMStoreFloat4x4(&g_geometoryWVPMat[0], DirectX::XMMatrixIdentity());
	g_pGeometoryWVP->Write(g_geometoryWVPMat);
	g_pGeometoryWVP->BindVS(0);
	g_pGeometoryColor->Write(&color);
	g_pGeometoryColor->BindVS(1);
	g_pGeometoryVS->Bind();
	g_pGeometoryPS->Bind();
	g_pGeometoryLinesMesh->Write(g_geometoryLineVtx);
	g_pGeometoryLinesMesh->Draw();
	g_geometoryLineIndex = 0;
}