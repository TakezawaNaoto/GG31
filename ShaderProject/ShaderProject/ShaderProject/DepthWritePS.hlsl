struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 screenPos : TEXCOORD0;
};

// 引数一つだけなら、構造体を作るほどでもないので
// main関数の引数に書く時もセマンティクスを指定する
float main(PS_IN pin) : SV_TARGET
{
	float depth = pin.screenPos.z / pin.screenPos.w;
	return depth;
}