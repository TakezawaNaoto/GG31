#ifndef __SCENE_DEFFERED_H__
#define __SCENE_DEFFERED_H__

#include "SceneBase.hpp"

class SceneDeffered : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();
};

#endif // __SCENE_SHADING_H__