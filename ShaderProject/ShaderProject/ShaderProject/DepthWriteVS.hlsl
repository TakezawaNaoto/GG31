// ObjectVS����R�s�[
struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 screenPos : TEXCOORD0;
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

	// ���[�J�����W > ���[���h���W > �r���[���W > �v���W�F�N�V�������W �ƕϊ���������
	vout.pos = mul(vout.pos, world);
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	// SV_POSITION�Ƃ͕ʂɃf�[�^��n��
	vout.screenPos = vout.pos;

	return vout;
}