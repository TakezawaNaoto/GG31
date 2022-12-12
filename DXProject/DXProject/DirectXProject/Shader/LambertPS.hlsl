// 頂点シェーダーからデータを受け取るので
// 頂点シェーダーの出力と同じデータ形式になる
struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

// ライトの情報
cbuffer LightInfo : register(b0)
{
	float4 LightDir;
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	// オブジェクトの影は法線とライトの向きから計算する
	// 法線とライトのベクトルがぶつかるような向きだと明るくなる
	// 法線とライトのベクトルが同じ向きだと暗くなる
	// 2つのベクトルの関係が同じか正体しているかは内積から求められる
	float3 N = normalize(pin.normal);
	// ライトのそのままの向きでは、内積の結果から本来明るくなる
	// 部分が暗くなってしまうので、ライトそのものの向きを反転させる
	float3 L = normalize(-LightDir);
	float shadow = dot(N, L);

	color.rgb *= shadow;

	return color;
}