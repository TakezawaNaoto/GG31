#ifndef __SHADER_LIST_H__
#define __SHADER_LIST_H__

#include "Shader.h"

enum VertexShaderKind
{
	VS_OBJECT, // pos3-noraml3-uv2
	VS_BUMP,
	VS_OUTLINE,
	VS_KIND_MAX
};
enum PixelShaderKind
{
	PS_LAMBERT,
	PS_PHONG,
	PS_LIMLIGHT,
	PS_FOG,
	PS_DISSOLVE,
	PS_BUMPMAP,
	PS_TOON,
	PS_OUTLINE,
	PS_KIND_MAX
};

void InitShaderList();
void UninitShaderList();
VertexShader* GetVS(VertexShaderKind kind);
PixelShader* GetPS(PixelShaderKind kind);

#endif // __SHADER_LIST_H__