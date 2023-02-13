// MultiRenderCheckPSからコピー

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PointLight
{
	float3 pos;	 // 配置されている位置
	float range;	// 光が届く距離(光の明るさ)
	float4 color;	// 光の色
};	

// --- 定数バッファ
cbuffer Inverse : register(b0)
{
	float4x4 invView;
	float4x4 invProj;
}

cbuffer Light : register(b1)
{
	PointLight lights[30];
}

SamplerState samp : register(s0);
Texture2D colorTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D worldTex : register(t3);
Texture2D pointLightTex : register(t4);

float4 main(PS_IN pin) : SV_TARGET
{
	// テクスチャから情報の取得
	float2 uv = pin.uv;
	uv.y = 1.0f - uv.y;
	float4 color = colorTex.Sample(samp, uv);
	float3 normal = normalTex.Sample(samp, uv).xyz;
	float depth = worldTex.Sample(samp, uv).x;
	float4 pointLightColor = pointLightTex.Sample(samp, pin.uv);

	// テクスチャに書き込まれた法線の情報の
	// 計算で使える形に変換する
	float3 N = normal * 2.0f - 1.0f;
	N = normalize(N);

	// テクスチャに書き込まれた深度値から
	// ワールド座標を復元する
	// ①スクリーンの座標を作成
	// ※スクリーンは-1～1なので、UV(0～1)から返還する
	float4 screenPos = float4(
		pin.uv * 2.0f - 1.0f, depth, 1.0f);
	// ②スクリーン座標からワールド座標へ変換
	// ワールド→スクリーンで行っている行列計算の
	// 逆行列を行えば変換できる。
	screenPos = mul(screenPos, invProj);	// プロジェクションをなかったことに
	screenPos = mul(screenPos, invView);	// ビューをなかったことに
	float3 worldPos = screenPos.xyz / screenPos.w;

	// 点光源の計算
	float4 lightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	for (int i = 0; i < 30; ++i)
	{
		float3 lightDir = lights[i].pos - worldPos;
		float lightLength = length(lightDir);

		float3 L = normalize(lightDir);
		float d = dot(L, N);
		d = d * 0.5f + 0.5f;

		float attenuation = lightLength / lights[i].range;
		attenuation = 1.0f - attenuation;
		attenuation = max(0.0f, attenuation);
		attenuation = pow(attenuation, 2.0f);

		lightColor += lights[i].color * d * attenuation;
	}

	return color + lightColor + pointLightColor;
}