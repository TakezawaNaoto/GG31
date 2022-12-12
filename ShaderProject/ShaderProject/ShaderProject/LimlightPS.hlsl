struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
};

cbuffer Light : register(b0)
{
	float4 lightDir;
};
cbuffer Camera : register(b1)
{
	float4 camPos;
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e��x�N�g��
	float3 L = normalize(-lightDir.xyz);
	float3 N = normalize(pin.normal);
	float3 V = normalize(camPos.xyz - pin.wPos.xyz);

	// �������̌v�Z
	float3 edgeColor = float3(1.0f, 0.0f, 0.0f);	// �������ɐF���������ăf�����Y�̈ŗ���ۂ�
	float edge = min(1.0f - dot(N, V), 1.0f);
	edge = pow(edge, 3);	// �����������ՂȂ��L����Ȃ��悤�ɍi��

	// �������C�g�̋��x�̌v�Z
	float lim = max(0, -dot(L, V));	// ���̒l���o�Ȃ��悤�Ɋۂ߂���

	// �g�U��
	float lambert = dot(L, N);

	color.rgb = lambert + edgeColor * edge * lim;

	return color;
}