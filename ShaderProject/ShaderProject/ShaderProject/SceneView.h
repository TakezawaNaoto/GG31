#ifndef __SCENE_VIEW_H__
#define __SCENE_VIEW_H__

#include "SceneBase.hpp"

class SceneView : public SceneBase
{
public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	int m_index;
};

#endif // __SCENE_VIEW_H__