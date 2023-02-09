// RoomShadowPSからコピー
struct PS_IN
{
	// スクリーンの座標-1～1はw成分で割ると
	// 正しく計算できる x' = x / w, y' = y / w
	// SV_POSITIONのパラメータは頂点シェーダーから
	// ピクセルシェーダーに渡される途中で勝手に計算されている
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
	float4 sunPos : TEXCOORD2;
};

// テクスチャ
// register(t0)はモデルに貼り付けるデフォルトのテクスチャ
Texture2D sunViewTex : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	// 座標をwで割る
	float2 sunUV = pin.sunPos.xy / pin.sunPos.w;
	// 画面の座標(sunUV)-1～1をテクスチャの座標0～1へ変換
	// 自分で考える
	sunUV = sunUV * 0.5f + 0.5f;
	// 画面のY座標は下から上に向かって増えるが、
	// テクスチャのY座標は上から下に向かって増えるので反転
	sunUV.y = 1.0f - sunUV.y;

	// 奥行情報を書き込んだテクスチャから、距離を取得
	float sunDepth = sunViewTex.Sample(samp, sunUV).r;
	// シャドウアクネ(マッハバンド)対策
	// 描画するピクセルの奥行きと、テクスチャの奥行きが一致している場合
	// テクスチャに書き込まれたデータとの差異が発生して影が出来ない部分に
	// 影が出来てしまう。テクスチャに書き込まれた距離は実際は少し遠い、
	// と置くことで回避できる。
	sunDepth += 0.001f;	// 数字は適宜調整
	// メインカメラで表示しようとしているピクセルの、
	// 太陽から見た距離を取得
	float pixelDepth = pin.sunPos.z / pin.sunPos.w;

	float4 color = float4(1, 1, 1, 1);
	// 現在のピクセルの奥行きが、テクスチャに書き込まれている奥行よりも
	// 遠い場合、影と判定する。
	// ※ピクセルシェーダーではピクセルごとに計算するので、あまりif文は使わない方が良い
	if (sunDepth < pixelDepth)
	{
		color.rgb = 0.0f;	// 黒色
	}

	return color;
}