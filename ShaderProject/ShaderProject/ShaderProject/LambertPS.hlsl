struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

cbuffer Light : register(b0)
{
	float4 lightDir;
};

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	// 内積
	// 同じ方向のベクトル同士なら正の値
	// 垂直方向のベクトル同士なら0
	// 反対方向のベクトル同士なら負の値

	// オブジェクトの法線とライトのベクトルから内積を計算
	// そのまま計算すると明るくなる部分が負の値になるので
	// 「あえて」ライトのベクトルを反転させる
	float3 N = normalize(pin.normal);
	float3 L = normalize(-lightDir.xyz);

	// 拡散光(diffuse) 物体の表面に直接当たる光、物体の色を表す
	float diffuse = dot(N, L);
	// 環境光(ambient) 周りの物体から反射して間接的に当たる光、物体の影の部分の色を表す
	float ambient = 0.5f;

	color.rgb = diffuse * 0.45f + ambient;

	return color;
}