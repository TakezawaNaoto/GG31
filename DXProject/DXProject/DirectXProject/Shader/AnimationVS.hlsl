struct VS_IN
{
	float3 pos : POSITION0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 weight : WEIGHT0;
	uint4 index : INDEX0;
};

struct VS_OUT
{
	float4 pos : SV_POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

cbuffer Matrix : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

// ���̒萔�o�b�t�@
cbuffer	BoneMatrix : register(b1)
{
	float4x4 bone[200];
}

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	// �������������W�͂����ȍ��̉e���������Ă���
	// �����Ƃ̍s��Ɋ��������߂āA�ŏI�I�ɑS������
	float4x4 anime =
		bone[vin.index.x] * vin.weight.x +
		bone[vin.index.y] * vin.weight.y +
		bone[vin.index.z] * vin.weight.z +
		bone[vin.index.w] * vin.weight.w;
	vout.pos = mul(vout.pos, anime);

	vout.pos = mul(vout.pos, world);
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	vout.uv = vin.uv;
	vout.normal = mul(vin.normal, (float3x3)world);

	return vout;
}