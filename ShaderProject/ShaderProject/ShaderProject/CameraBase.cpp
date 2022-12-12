#include "CameraBase.h"
#include "Geometory.h"

CameraBase::CameraBase()
	: m_pos(0.0f, 0.0f, -10.0f), m_look(0.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f)
	, m_fovy(DirectX::XMConvertToRadians(60.0f)), m_aspect(16.0f / 9.0f), m_near(0.2f), m_far(1000.0f)
{
}
CameraBase::~CameraBase()
{
}

#ifdef _DEBUG
void CameraBase::Draw()
{
	SetGeometoryColor(DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f));
	DrawBox(m_look, DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f));
	// DrawBox(m_pos, DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f));
	// AddLine(m_look, m_pos);

}
#endif

DirectX::XMFLOAT4X4 CameraBase::GetView()
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_pos), DirectX::XMLoadFloat3(&m_look), DirectX::XMLoadFloat3(&m_up))
	));
	return mat;
}
DirectX::XMFLOAT4X4 CameraBase::GetProj()
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH(m_fovy, m_aspect, m_near, m_far)
	));
	return mat;
}

DirectX::XMFLOAT3 CameraBase::GetPos()
{
	return m_pos;
}
DirectX::XMFLOAT3 CameraBase::GetLook()
{
	return m_look;
}
DirectX::XMFLOAT3 CameraBase::GetUp()
{
	return m_up;
}
void CameraBase::SetPos(DirectX::XMFLOAT3 pos)
{
	m_pos = pos;
}
void CameraBase::SetLook(DirectX::XMFLOAT3 look)
{
	m_look = look;
}
void CameraBase::SetUp(DirectX::XMFLOAT3 up)
{
	m_up = up;
}