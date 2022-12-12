struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
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

	// ���_�̕����Ɍ������ă��f���̒��_���ړ�������
	vout.pos.xyz += normalize(vin.normal) * 0.001f;

	// ���[�J�����W > ���[���h���W > �r���[���W > �v���W�F�N�V�������W �ƕϊ���������
	vout.pos = mul(vout.pos, world);
	vout.wPos = vout.pos;	// �s�N�Z���V�F�[�_�[�Ƀ��[���h���W��n�����߁A��U�R�s�[
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);

	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
}