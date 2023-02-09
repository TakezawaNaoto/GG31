#ifndef __SCENE_SHADOW_H__
#define __SCENE_SHADOW_H__

#include "SceneBase.hpp"
#include <DirectXMath.h>

class SceneShadow : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	struct PointLight
	{
		DirectX::XMFLOAT3 pos;
		float range;
		DirectX::XMFLOAT4 color;
	};
	PointLight m_lights[30];
};

#endif // __SCENE_SHADING_H__