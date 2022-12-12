// CPU����󂯎��Ƃ��̒��_�̍\��
struct VS_IN
{
	// �Z�}���e�B�N�X ���̃f�[�^���ǂ������f�[�^�Ȃ̂���
	// �����I�ɐ������Ă���B���ϐ����͍D������ς����邽��
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
// �s�N�Z���V�F�[�_�[�ɓn���f�[�^�̍\��
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

// �萔�o�b�t�@(���_�ȊO�̃f�[�^�͂����Ŏ󂯎��
cbuffer Matrix : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;

	vout.pos = float4(vin.pos, 1.0f);
	// ���[�J�����W���烏�[���h���W�֕ϊ�
	vout.pos = mul(vout.pos, world);
	// ���[���h���W����J���������_�̍��W�֕ϊ�
	vout.pos = mul(vout.pos, view);
	// �J��������ǂ̂悤�Ɍ�����̂��ϊ�
	vout.pos = mul(vout.pos, proj);

	vout.uv = vin.uv;

	vout.normal = mul(vin.normal, (float3x3)world);

	return vout;
}