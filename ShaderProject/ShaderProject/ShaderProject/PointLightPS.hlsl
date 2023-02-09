// --- LambertPS.hlsl����R�s�[

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	// �_�����̍��W�Ƃ̈ʒu�֌W����A
	// �ǂ̕�������������ł��邩�v�Z����
	float4 wPos : TEXCOORD1;
};

// �_�����ɕK�v�ȏ��
struct PointLight
{
	float3 pos;		// �z�u����Ă���ʒu
	float range;	// �����͂�����(���̖��邳�Ƒ����Ă��悢
	float4 color;	// ���̐F
};

cbuffer Light : register(b0)
{
	PointLight lights[30];
};

float4 main(PS_IN pin) : SV_TARGET
{
	// �_�����̌����v�Z����O�̓s�N�Z���Ɍ���������Ȃ�
	// �͂��Ȃ̂ŁA���F(0,0,0)�ŏ�����
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// �S�Ă̓_�����̌�������������ǂ�ȐF�ɂȂ邩�v�Z
	// �����̏d�ˍ��킹�́A���̎O���F����d�˂�قǖ��邭�Ȃ�
	for (int i = 0; i < 30; ++i)
	{
		// ���̌������v�Z
		float3 lightDir = lights[i].pos - pin.wPos.xyz;
		// �����܂ł̋���
		float lightLength = length(lightDir);

		// Lambert�̌v�Z
		float3 L = normalize(lightDir);
		float3 N = normalize(pin.normal);
		float d = dot(L, N);
		d = d * 0.5f + 0.5f;

		// ���̓͂������Ɋւ��Čv�Z
		// �_�����ɋ߂����1�A�͂��Ȃ������ł�0�ƈ���
		float attenuation = lightLength / lights[i].range;
		// ��L�̌v�Z�ł͋������߂��ق�0�A�����ق�1�ƂȂ���
		// ���܂����߁A�v�Z���ʂɎ��������
		attenuation = 1.0f - attenuation;
		// �����͂��������������ꏊ�͕��̒l�ɂȂ�̂�0�ɂ���
		attenuation = max(0.0f, attenuation);

		// �������E�̌��͋�����2��Ō����������������
		// �������������قǏƂ炷�͈͂����̂����������邽��
		attenuation = pow(attenuation, 2.0f);

		// �_�����̖��邳�́ALambert * attenuation(������)��
		// ���߂���
		color += lights[i].color * d * attenuation;
	}

	return color;
}