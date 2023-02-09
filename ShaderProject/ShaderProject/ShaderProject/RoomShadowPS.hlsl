// ObjectVS��VS_OUT���R�s�[���ĉ���
struct PS_IN
{
	// �X�N���[���̍��W-1�`1��w�����Ŋ����
	// �������v�Z�ł��� x' = x / w, y' = y / w
	// SV_POSITION�̃p�����[�^�͒��_�V�F�[�_�[����
	// �s�N�Z���V�F�[�_�[�ɓn�����r���ŏ���Ɍv�Z����Ă���
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 wPos : TEXCOORD1;
	float4 sunPos : TEXCOORD2;
};

// �e�N�X�`��
// register(t0)�̓��f���ɓ\��t����f�t�H���g�̃e�N�X�`��
Texture2D sunViewTex : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	// ���W��w�Ŋ���
	float2 sunUV = pin.sunPos.xy / pin.sunPos.w;
	// ��ʂ̍��W(sunUV)-1�`1���e�N�X�`���̍��W0�`1�֕ϊ�
	// �����ōl����
	sunUV = sunUV * 0.5f + 0.5f;
	// ��ʂ�Y���W�͉������Ɍ������đ����邪�A
	// �e�N�X�`����Y���W�͏ォ�牺�Ɍ������đ�����̂Ŕ��]
	sunUV.y = 1.0f - sunUV.y;

	float4 sunColor = sunViewTex.Sample(samp, sunUV);
	sunColor.rgb = 1.0f - sunColor.a;
	sunColor.a = 1.0f;
	return sunColor;
}