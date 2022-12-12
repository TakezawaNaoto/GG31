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
	float m_rad;	// ƒ‰ƒCƒg‚ÌŠp“x
};

#endif // __SCENE_SHADING_H__
