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

	// ����
	// ���������̃x�N�g�����m�Ȃ琳�̒l
	// ���������̃x�N�g�����m�Ȃ�0
	// ���Ε����̃x�N�g�����m�Ȃ畉�̒l

	// �I�u�W�F�N�g�̖@���ƃ��C�g�̃x�N�g��������ς��v�Z
	// ���̂܂܌v�Z����Ɩ��邭�Ȃ镔�������̒l�ɂȂ�̂�
	// �u�����āv���C�g�̃x�N�g���𔽓]������
	float3 N = normalize(pin.normal);
	float3 L = normalize(-lightDir.xyz);

	// �g�U��(diffuse) ���̂̕\�ʂɒ��ړ�������A���̂̐F��\��
	float diffuse = dot(N, L);
	// ����(ambient) ����̕��̂��甽�˂��ĊԐړI�ɓ�������A���̂̉e�̕����̐F��\��
	float ambient = 0.5f;

	color.rgb = diffuse * 0.45f + ambient;

	return color;
}