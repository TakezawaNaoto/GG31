#include "ShaderList.h"
#include "WinDialog.h"

const char* VertexShaderFile[] =
{
	"Assets/Shader/ObjectVS.cso",
	"Assets/Shader/BumpVS.cso",
	"Assets/Shader/OutlineVS.cso",
	"Assets/Shader/RoomShadowVS.cso",
	"Assets/Shader/DepthWriteVS.cso",
};
const char* PixelShaderFile[] =
{
	"Assets/Shader/LambertPS.cso",
	"Assets/Shader/PhongPS.cso",
	"Assets/Shader/LimlightPS.cso",
	"Assets/Shader/FogPS.cso",
	"Assets/Shader/DissolvePS.cso",
	"Assets/Shader/BumpmapPS.cso",
	"Assets/Shader/ToonPS.cso",
	"Assets/Shader/OutlinePS.cso",
	"Assets/Shader/ScreenPS.cso",
	"Assets/Shader/RoomShadowPS.cso",
	"Assets/Shader/DepthShadowPS.cso",
	"Assets/Shader/DepthWritePS.cso",
	"Assets/Shader/PointLightPS.cso",
	"Assets/Shader/MultiRenderPS.cso",
	"Assets/Shader/MultiRenderCheckPS.cso",
	"Assets/Shader/LightColorPS.cso",
	"Assets/Shader/DefferedPointLightPS.cso",
	"Assets/Shader/GradationPS.cso",
};





static_assert(VS_KIND_MAX == _countof(VertexShaderFile), "VS - ShaderList[.hの定義と.cppのファイル数が一致しません");
static_assert(PS_KIND_MAX == _countof(PixelShaderFile), "PS - ShaderList[.hの定義と.cppのファイル数が一致しません");
VertexShader* g_pVertexShaderList[VS_KIND_MAX];
PixelShader* g_pPixelShaderList[PS_KIND_MAX];
void InitShaderList()
{
	for (int i = 0; i < VS_KIND_MAX; ++i)
	{
		g_pVertexShaderList[i] = new VertexShader();
		if (FAILED(g_pVertexShaderList[i]->Load(VertexShaderFile[i])))
		{
			DialogError(STACK_DEFAULT, "%sが見つかりません", VertexShaderFile[i]);
		}
	}
	for (int i = 0; i < PS_KIND_MAX; ++i)
	{
		g_pPixelShaderList[i] = new PixelShader();
		if (FAILED(g_pPixelShaderList[i]->Load(PixelShaderFile[i])))
		{
			DialogError(STACK_DEFAULT, "%sが見つかりません", PixelShaderFile[i]);
		}
	}
}
void UninitShaderList()
{
	for (int i = 0; i < VS_KIND_MAX; ++i)
		delete g_pVertexShaderList[i];
	for (int i = 0; i < PS_KIND_MAX; ++i)
		delete g_pPixelShaderList[i];
}
VertexShader* GetVS(VertexShaderKind kind)
{
	return g_pVertexShaderList[kind];
}
PixelShader* GetPS(PixelShaderKind kind)
{
	return g_pPixelShaderList[kind];
}