#ifndef __GEOMETORY_H__
#define __GEOMETORY_H__

#include <Windows.h>
#include <DirectXMath.h>

HRESULT InitGeometory();

void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4 color = { 1,1,1,1 });
void DrawLines();

#endif // __GEOMETORY_H__