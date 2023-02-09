#ifndef __SCENE_BASE_HPP__
#define __SCENE_BASE_HPP__

#include <map>
#include <memory>
#include <string>
#include "WinDialog.h"

class DeleterBase {
public:
	virtual ~DeleterBase() {}
};
template<class T>
class Deleter : public DeleterBase {
public:
	Deleter(T* ptr) : m_pObj(ptr) {}
	virtual ~Deleter() {
		delete m_pObj;
	}
	T* m_pObj;
};

class SceneBase
{
private:
	using Map = std::map<std::string, DeleterBase*>;
public:
	SceneBase();
	virtual ~SceneBase();
	void RemoveSubScene();
	void _update();
	void _draw();

	virtual void Init() = 0;
	virtual void Uninit() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	template<class T> T* AddSubScene();
	template<class T> void RegisterObj(const char* name, T* ptr);
	template<class T> T* CreateObj(const char* name);
	template<class T> T* GetObj(const char* name);
private:
	DeleterBase* FindObj(const char* name);
protected:
	SceneBase* m_pParent;
	SceneBase* m_pSubScene;
	Map m_objects;
};

template<class T> T* SceneBase::AddSubScene()
{
	RemoveSubScene();
	T* pScene = new T;
	m_pSubScene = pScene;
	pScene->m_pParent = this;
	pScene->Init();
	return pScene;
}
template<class T>void SceneBase::RegisterObj(const char* name, T* ptr)
{
	if (FindObj(name))
		DialogError(STACK_DEFAULT + 1, "%sÇÕìoò^çœÇ›Ç≈Ç∑", name);
	else
		m_objects.insert(std::pair<std::string, DeleterBase*>(name, new Deleter<T>(ptr)));
}
template<class T> T* SceneBase::CreateObj(const char* name)
{
	if(FindObj(name))
		DialogError(STACK_DEFAULT + 1, "%sÇÕìoò^çœÇ›Ç≈Ç∑", name);

	T* pObj = new T;
	m_objects.insert(std::pair<std::string, DeleterBase*>(name, new Deleter<T>(pObj)));
	return pObj;
}
template<class T> T* SceneBase::GetObj(const char* name)
{
	Deleter<T>* ptr = reinterpret_cast<Deleter<T>*>(FindObj(name));
	if (!ptr) {
		DialogMessage(STACK_DEFAULT + 1, "%sÇ™å©Ç¬Ç©ÇËÇ‹ÇπÇÒ", name);
	}
	return ptr->m_pObj;
}

#endif // __SCENE_BASE_HPP__