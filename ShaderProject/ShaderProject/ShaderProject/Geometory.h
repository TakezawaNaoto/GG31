#ifndef __GEOMETORY_H__
#define __GEOMETORY_H__

#include <Windows.h>
#include <DirectXMath.h>

void InitGeometory();
void UninitGeometory();

void SetGeometoryVP(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 proj);
void SetGeometoryColor(DirectX::XMFLOAT4 color);
void DrawBox(DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3 size = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3 rot = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end);
void DrawLines();

#endif // __GEOMETORY_H__