#ifndef __TEXTURE_FACTORY_H__
#define __TEXTURE_FACTORY_H__

#include "Texture.h"

/**
 * @brief �e�N�X�`������
 */
class TextureFactory
{
public:
	static Texture* CreateFromFile(const char* fileName);
	static Texture* CreateFromData(DXGI_FORMAT format, UINT width, UINT height, const void* pData);
	static Texture* CreateRenderTargetFromScreen();
};

#endif // __TEXTURE_FACTORY_H__