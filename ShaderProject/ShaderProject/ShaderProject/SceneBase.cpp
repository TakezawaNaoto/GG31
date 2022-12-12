#include "SceneBase.hpp"

SceneBase::SceneBase()
	: m_pParent(nullptr)
	, m_pSubScene(nullptr)
{
}
SceneBase::~SceneBase()
{
	if (m_pSubScene != nullptr)
	{
		m_pSubScene->Uninit();
		delete m_pSubScene;
	}
	if(m_pParent)
		m_pParent->m_pSubScene = nullptr;

	Map::iterator it = m_objects.begin();
	while (it != m_objects.end())
	{
		delete it->second;
		++it;
	}
}
void SceneBase::_update()
{
	if (m_pSubScene)
		m_pSubScene->_update();
	Update();
}
void SceneBase::_draw()
{
	if (m_pSubScene)
		m_pSubScene->_draw();
	Draw();
}
DeleterBase* SceneBase::FindObj(const char* name)
{
	// Ž©g‚Ì—Ìˆæ‚©‚ç’Tõ
	Map::iterator it = m_objects.find(name);
	if (it != m_objects.end())
	{
		return it->second;
	}
	// e—Ìˆæ‚Ì’Tõ
	SceneBase* pScene = m_pParent;
	while (pScene)
	{
		it = pScene->m_objects.find(name);
		if (it != pScene->m_objects.end())
		{
			return it->second;
		}
		pScene = pScene->m_pParent;
	}
	// Žq—Ìˆæ‚Ì’Tõ
	pScene = m_pSubScene;
	while (pScene)
	{
		it = pScene->m_objects.find(name);
		if (it != pScene->m_objects.end())
		{
			return it->second;
		}
		pScene = pScene->m_pSubScene;
	}
	return nullptr;
}
void SceneBase::RemoveSubScene()
{
	if (m_pSubScene)
	{
		m_pSubScene->Uninit();
		delete m_pSubScene;
	}
}