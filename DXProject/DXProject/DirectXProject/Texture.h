#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "DirectX.h"

/**
 * @brief テクスチャリソース
 */
class Texture
{
public:
	friend class TextureFactory;

public:
	Texture();
	virtual ~Texture();
	virtual void Release();

	UINT GetWidth() const;
	UINT GetHeight() const;
	ID3D11ShaderResourceView* GetResource() const;

protected:
	virtual HRESULT CreateResource(D3D11_TEXTURE2D_DESC &desc, const void* pData = nullptr);

private:
	//! テクスチャ横幅
	UINT m_width;
	//! テクスチャ縦幅
	UINT m_height;

	ID3D11ShaderResourceView *m_pSRV;

protected:
	ID3D11Texture2D* m_pTex;
};

#endif // __TEXTURE_H__