struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD;
	float4 wPos : TEXCOORD1;
	float4 screenPos : TEXCOORD2;
};

struct PS_OUT
{
	float4 color : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 world : SV_TARGET2;
};

SamplerState samp : register(s0);
Texture2D tex : register(t0);

PS_OUT main(PS_IN pin)
{
	PS_OUT pout;
	// 色情報書き込み
	pout.color = tex.Sample(samp, pin.uv);
	// 法線情報書き込み
	// 法線の情報は-1～1だが、テクスチャは
	// 0～1で扱うため、-1～1→0～1への変換が必要
	pout.normal = float4(pin.normal * 0.5f + 0.5f, 1.0f);
	// ワールド座標の書き込み
	// →深度地を書き込む方がよい
	// ワールド座標だとx,y,zの３成分が必要だが、書き込み先は
	// 色のrgb各byteしかない。
	// 深度地下らワールド座標を復元することができるので、
	// 4byte使って正確に保存できる
	// pout.world = pin.wPos; ←ワールド座標はダメ
	float depth = pin.screenPos.z / pin.screenPos.w;
	pout.world = float4(depth, depth, depth, depth);

	return pout;
}