#ifndef __SCENE_SHADING_H__
#define __SCENE_SHADING_H__

#include "SceneBase.hpp"

class SceneShading : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	float m_rate;
};

#endif // __SCENE_SHADING_H__