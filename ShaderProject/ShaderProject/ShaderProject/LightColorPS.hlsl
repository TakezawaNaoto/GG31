struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	// �_�����̍��W�Ƃ̈ʒu�֌W����A
	// �ǂ̕�������������ł��邩�v�Z����
	float4 wPos : TEXCOORD1;
};

cbuffer Light : register(b0)
{
	float4 color;
}

float4 main(PS_IN pin) : SV_TARGET
{
	return color;
}