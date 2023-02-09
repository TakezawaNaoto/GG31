struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	// 点光源の座標との位置関係から、
	// どの方向から光が飛んでくるか計算する
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