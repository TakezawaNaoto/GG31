struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};

cbuffer Light : register(b0)
{
	float4 lightDir;
};
cbuffer Camera : register(b1)
{
	float4 camPos;
}

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

	// 反射ベクトル
	// ある地点に入射した光が反射する方向を示したベクトル
	// 法線を用いて以下の式となる
	// R = L + 2 * dot(-L, N) * N
	// R …反射ベクトル / L…光の方向ベクトル / N…法線

	// HLSLにはreflect(ライトの向き、法線)で反射ベクトルを求める関数がある
	// 前の計算でライトの向きを反転させてたので、戻す
	float3 R = reflect(-L, N);

	// ピクセルからカメラへ向かうベクトル
	float3 V = normalize(camPos.xyz - pin.wPos.xyz);

	// 鏡面反射(スペキュラー:specular)
	// 反射ベクトルと視線ベクトルの一致具合で反射の強さを表す
	float specular = max(0, dot(R, V));
	// そのままだと全体が明るいので、累乗するといい感じになる
	specular = pow(specular, 30.0f);

	color.rgb = diffuse * 0.45f + ambient + specular;

	color.r = 1.0f;

	return color;
}