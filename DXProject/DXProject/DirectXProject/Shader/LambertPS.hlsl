// ���_�V�F�[�_�[����f�[�^���󂯎��̂�
// ���_�V�F�[�_�[�̏o�͂Ɠ����f�[�^�`���ɂȂ�
struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
};

// ���C�g�̏��
cbuffer LightInfo : register(b0)
{
	float4 LightDir;
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	// �I�u�W�F�N�g�̉e�͖@���ƃ��C�g�̌�������v�Z����
	// �@���ƃ��C�g�̃x�N�g�����Ԃ���悤�Ȍ������Ɩ��邭�Ȃ�
	// �@���ƃ��C�g�̃x�N�g���������������ƈÂ��Ȃ�
	// 2�̃x�N�g���̊֌W�����������̂��Ă��邩�͓��ς��狁�߂���
	float3 N = normalize(pin.normal);
	// ���C�g�̂��̂܂܂̌����ł́A���ς̌��ʂ���{�����邭�Ȃ�
	// �������Â��Ȃ��Ă��܂��̂ŁA���C�g���̂��̂̌����𔽓]������
	float3 L = normalize(-LightDir);
	float shadow = dot(N, L);

	color.rgb *= shadow;

	return color;
}