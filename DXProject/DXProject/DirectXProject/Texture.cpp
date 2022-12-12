#include "Texture.h"

Texture::Texture()
	: m_width(0), m_height(0)
	, m_pTex(nullptr)
	, m_pSRV(nullptr)
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	SAFE_RELEASE(m_pSRV);
	SAFE_RELEASE(m_pTex);
}

UINT Texture::GetWidth() const
{
	return m_width;
}

UINT Texture::GetHeight() const
{
	return m_height;
}

ID3D11ShaderResourceView* Texture::GetResource() const
{
	return m_pSRV;
}

HRESULT Texture::CreateResource(D3D11_TEXTURE2D_DESC& desc, const void* pData)
{
	// テクスチャ作成
	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = pData;
	data.SysMemPitch = desc.Width * 4;
	GetDevice()->CreateTexture2D(&desc, pData ? &data : nullptr, &m_pTex);

	// 設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// 生成
	return GetDevice()->CreateShaderResourceView(m_pTex, &srvDesc, &m_pSRV);
}