#include "SceneView.h"
#include "Model.h"
#include "ShaderList.h"
#include "ConstantBuffer.h"
#include "SceneShading.h"
#include "SceneBump.h"
#include "SceneToon.h"
#include "SceneShadow.h"
#include "SceneDeffered.h"
#include "Input.h"
#include "CameraDCC.h"

enum SceneKind
{
	SCENE_NONE,
	SCENE_SHADING,
	SCENE_BUMP,
	SCENE_TOON,
	SCENE_SHADOW,
	SCENE_DEFFERED,
	SCENE_MAX
};

struct ViewSetting
{
	DirectX::XMFLOAT3 camPos;
	DirectX::XMFLOAT3 camLook;
	DirectX::XMFLOAT3 camUp;
	int index;
};

const char* SettingFileName = "Assets/setting.dat";

void SceneView::Init()
{
	Model* pModel = CreateObj<Model>("UnityModel");
	pModel->Load("Assets/Unitychan/unitychan.fbx", 0.01f);

	ConstantBuffer* pWVP = CreateObj<ConstantBuffer>("CBWVP");
	pWVP->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	ConstantBuffer* pLight = CreateObj<ConstantBuffer>("CBLight");
	pLight->Create(sizeof(DirectX::XMFLOAT4));

	ViewSetting setting =
	{
		DirectX::XMFLOAT3(0.0f, 1.0f, -5.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		SCENE_SHADING
	};
	FILE* fp;
	fopen_s(&fp, SettingFileName, "rb");
	if (fp)
	{
		fread(&setting, sizeof(ViewSetting), 1, fp);
		fclose(fp);
	}

	CameraBase* pCamera = CreateObj<CameraDCC>("CameraDCC");
	pCamera->SetPos(setting.camPos);
	pCamera->SetLook(setting.camLook);
	pCamera->SetUp(setting.camUp);
	m_index = setting.index;
	switch (m_index)
	{
	case SCENE_SHADING:		AddSubScene<SceneShading>(); break;
	case SCENE_BUMP:		AddSubScene<SceneBump>(); break;
	case SCENE_TOON:		AddSubScene<SceneToon>(); break;
	case SCENE_SHADOW:		AddSubScene<SceneShadow>(); break;
	case SCENE_DEFFERED:	AddSubScene<SceneDeffered>(); break;
	}
}
void SceneView::Uninit()
{
	CameraBase* pCamera = GetObj<CameraBase>("CameraDCC");
	ViewSetting setting =
	{
		pCamera->GetPos(),
		pCamera->GetLook(),
		pCamera->GetUp(),
		m_index
	};
	FILE* fp;
	fopen_s(&fp, SettingFileName, "wb");
	if (fp)
	{
		fwrite(&setting, sizeof(ViewSetting), 1, fp);
		fclose(fp);
	}
}
void SceneView::Update()
{
	int idx = SCENE_NONE;
	if (IsKeyPress(VK_SHIFT))
	{
		if (IsKeyTrigger('1')) idx = SCENE_SHADING;
		if (IsKeyTrigger('2')) idx = SCENE_BUMP;
		if (IsKeyTrigger('3')) idx = SCENE_TOON;
		if (IsKeyTrigger('4')) idx = SCENE_SHADOW;
		if (IsKeyTrigger('5')) idx = SCENE_DEFFERED;
	}
	if (idx != SCENE_NONE && idx != m_index)
	{
		m_index = idx;
		RemoveSubScene();
		switch (m_index)
		{
		case SCENE_SHADING:		AddSubScene<SceneShading>(); break;
		case SCENE_BUMP:		AddSubScene<SceneBump>(); break;
		case SCENE_TOON:		AddSubScene<SceneToon>(); break;
		case SCENE_SHADOW:		AddSubScene<SceneShadow>(); break;
		case SCENE_DEFFERED:	AddSubScene<SceneDeffered>(); break;
		}
	}
}
void SceneView::Draw()
{
}