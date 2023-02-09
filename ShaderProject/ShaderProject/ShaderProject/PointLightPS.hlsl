// --- LambertPS.hlslからコピー

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	// 点光源の座標との位置関係から、
	// どの方向から光が飛んでくるか計算する
	float4 wPos : TEXCOORD1;
};

// 点光源に必要な情報
struct PointLight
{
	float3 pos;		// 配置されている位置
	float range;	// 光が届く距離(光の明るさと捉えてもよい
	float4 color;	// 光の色
};

cbuffer Light : register(b0)
{
	PointLight lights[30];
};

float4 main(PS_IN pin) : SV_TARGET
{
	// 点光源の光を計算する前はピクセルに光が当たらない
	// はずなので、黒色(0,0,0)で初期化
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 全ての点光源の光が当たったらどんな色になるか計算
	// ※光の重ね合わせは、光の三原色から重ねるほど明るくなる
	for (int i = 0; i < 30; ++i)
	{
		// 光の向きを計算
		float3 lightDir = lights[i].pos - pin.wPos.xyz;
		// 光源までの距離
		float lightLength = length(lightDir);

		// Lambertの計算
		float3 L = normalize(lightDir);
		float3 N = normalize(pin.normal);
		float d = dot(L, N);
		d = d * 0.5f + 0.5f;

		// 光の届く距離に関して計算
		// 点光源に近ければ1、届かない距離では0と扱う
		float attenuation = lightLength / lights[i].range;
		// 上記の計算では距離が近いほど0、遠いほど1となって
		// しまうため、計算結果に手を加える
		attenuation = 1.0f - attenuation;
		// 光が届く距離よりも遠い場所は負の値になるので0にする
		attenuation = max(0.0f, attenuation);

		// 現実世界の光は距離の2乗で減衰する特徴がある
		// ※距離が離れるほど照らす範囲がものすごく増えるため
		attenuation = pow(attenuation, 2.0f);

		// 点光源の明るさは、Lambert * attenuation(減衰率)で
		// 求められる
		color += lights[i].color * d * attenuation;
	}

	return color;
}