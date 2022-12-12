#ifndef __SCENE_TOON_H__
#define __SCENE_TOON_H__

#include "SceneBase.hpp"

class SceneToon : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();
};

#endif // __SCENE_SHADING_H__