#include "Geometory.h"
#include "DXBuffer.h"

#define LINE_NUM (1000)
#define LINE_GEOMETORY_NUM (LINE_NUM * 2)

struct GeometoryVertex
{
	float pos[3];
	float color[4];
};

UINT g_lineIndex;
GeometoryVertex g_lineData[LINE_GEOMETORY_NUM];
DXBuffer g_lines;


HRESULT InitGeometory()
{
	HRESULT hr;
	const float d = 0.5f;

	DXBuffer::Desc desc = {};
	
	desc.pVtx = g_lineData;
	//--------------
	// ライン描画用の頂点バッファ
	//--------------
	desc.vtxSize = sizeof(GeometoryVertex);
	desc.vtxCount = LINE_GEOMETORY_NUM;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	desc.isWrite = true;
	//--------------
	hr = g_lines.Create(desc);
	if (FAILED(hr)) { return hr; }

	g_lineIndex = 0;
	return hr;
}

void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4 color)
{
	if (g_lineIndex < LINE_NUM)
	{
		//--------------
		// ライン情報の設定
		//--------------
		// 始点
		g_lineData[g_lineIndex].pos[0] = start.x;
		g_lineData[g_lineIndex].pos[1] = start.y;
		g_lineData[g_lineIndex].pos[2] = start.z;
		g_lineData[g_lineIndex].color[0] = color.x;
		g_lineData[g_lineIndex].color[1] = color.y;
		g_lineData[g_lineIndex].color[2] = color.z;
		g_lineData[g_lineIndex].color[3] = color.w;

			g_lineIndex++;
		// 終点
		g_lineData[g_lineIndex].pos[0] = end.x;
		g_lineData[g_lineIndex].pos[1] = end.y;
		g_lineData[g_lineIndex].pos[2] = end.z;
		g_lineData[g_lineIndex].color[0] = color.x;
		g_lineData[g_lineIndex].color[1] = color.y;
		g_lineData[g_lineIndex].color[2] = color.z;
		g_lineData[g_lineIndex].color[3] = color.w;

			g_lineIndex++;
		//--------------
	}
}
void DrawLines()
{
	g_lines.Write(g_lineData);
	g_lines.Draw();
	g_lineIndex = 0;
}