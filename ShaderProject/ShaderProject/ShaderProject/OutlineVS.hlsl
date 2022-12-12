struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};
cbuffer WVP : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};


VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);

	// 頂点の方向に向かってモデルの頂点を移動させる
	vout.pos.xyz += normalize(vin.normal) * 0.001f;

	// ローカル座標 > ワールド座標 > ビュー座標 > プロジェクション座標 と変換をかける
	vout.pos = mul(vout.pos, world);
	vout.wPos = vout.pos;	// ピクセルシェーダーにワールド座標を渡すため、一旦コピー
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
}