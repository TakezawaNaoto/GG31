#ifndef __SCENE_BUMP_H__
#define __SCENE_BUMP_H__

#include "SceneBase.hpp"

class SceneBump : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	float m_rad;	// ���C�g�̊p�x
};

#endif // __SCENE_SHADING_H__
