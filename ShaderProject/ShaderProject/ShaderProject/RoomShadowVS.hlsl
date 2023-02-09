// --- ObjectVS.hlsl����R�s�[

struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
	float4 sunPos : TEXCOORD2;	// ���z���猩���ʒu
};
// �Q�[���̃��C���J�����ɕ\�����邽�߂̍s��
cbuffer WVP : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};
// ���z�̈ʒu�ɂ���J�����ɕ\�����邽�߂̍s��
cbuffer SunCamera : register(b1)
{
	float4x4 sunView;
	float4x4 sunProj;
}

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);

	// ���[�J�����W > ���[���h���W > �r���[���W > �v���W�F�N�V�������W �ƕϊ���������
	vout.pos = mul(vout.pos, world);
	vout.wPos = vout.pos;	// �s�N�Z���V�F�[�_�[�Ƀ��[���h���W��n�����߁A��U�R�s�[
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	// ���C���J�����Ƃ͕ʂɁA���z�p�Œ��_���v�Z
	float4 sunPos = vout.wPos;
	sunPos = mul(sunPos, sunView);
	sunPos = mul(sunPos, sunProj);
	vout.sunPos = sunPos;

	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
}