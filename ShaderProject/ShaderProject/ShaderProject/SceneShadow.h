#ifndef __SCENE_SHADOW_H__
#define __SCENE_SHADOW_H__

#include "SceneBase.hpp"

class SceneShadow : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();
};

#endif // __SCENE_SHADING_H__