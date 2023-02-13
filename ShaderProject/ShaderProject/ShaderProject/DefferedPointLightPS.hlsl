// MultiRenderCheckPS����R�s�[

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PointLight
{
	float3 pos;	 // �z�u����Ă���ʒu
	float range;	// �����͂�����(���̖��邳)
	float4 color;	// ���̐F
};	

// --- �萔�o�b�t�@
cbuffer Inverse : register(b0)
{
	float4x4 invView;
	float4x4 invProj;
}

cbuffer Light : register(b1)
{
	PointLight lights[30];
}

SamplerState samp : register(s0);
Texture2D colorTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D worldTex : register(t3);
Texture2D pointLightTex : register(t4);

float4 main(PS_IN pin) : SV_TARGET
{
	// �e�N�X�`��������̎擾
	float2 uv = pin.uv;
	uv.y = 1.0f - uv.y;
	float4 color = colorTex.Sample(samp, uv);
	float3 normal = normalTex.Sample(samp, uv).xyz;
	float depth = worldTex.Sample(samp, uv).x;
	float4 pointLightColor = pointLightTex.Sample(samp, pin.uv);

	// �e�N�X�`���ɏ������܂ꂽ�@���̏���
	// �v�Z�Ŏg����`�ɕϊ�����
	float3 N = normal * 2.0f - 1.0f;
	N = normalize(N);

	// �e�N�X�`���ɏ������܂ꂽ�[�x�l����
	// ���[���h���W�𕜌�����
	// �@�X�N���[���̍��W���쐬
	// ���X�N���[����-1�`1�Ȃ̂ŁAUV(0�`1)����Ԋ҂���
	float4 screenPos = float4(
		pin.uv * 2.0f - 1.0f, depth, 1.0f);
	// �A�X�N���[�����W���烏�[���h���W�֕ϊ�
	// ���[���h���X�N���[���ōs���Ă���s��v�Z��
	// �t�s����s���Εϊ��ł���B
	screenPos = mul(screenPos, invProj);	// �v���W�F�N�V�������Ȃ��������Ƃ�
	screenPos = mul(screenPos, invView);	// �r���[���Ȃ��������Ƃ�
	float3 worldPos = screenPos.xyz / screenPos.w;

	// �_�����̌v�Z
	float4 lightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	for (int i = 0; i < 30; ++i)
	{
		float3 lightDir = lights[i].pos - worldPos;
		float lightLength = length(lightDir);

		float3 L = normalize(lightDir);
		float d = dot(L, N);
		d = d * 0.5f + 0.5f;

		float attenuation = lightLength / lights[i].range;
		attenuation = 1.0f - attenuation;
		attenuation = max(0.0f, attenuation);
		attenuation = pow(attenuation, 2.0f);

		lightColor += lights[i].color * d * attenuation;
	}

	return color + lightColor + pointLightColor;
}