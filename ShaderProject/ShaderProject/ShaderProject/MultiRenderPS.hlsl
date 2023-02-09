struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD;
	float4 wPos : TEXCOORD1;
	float4 screenPos : TEXCOORD2;
};

struct PS_OUT
{
	float4 color : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 world : SV_TARGET2;
};

SamplerState samp : register(s0);
Texture2D tex : register(t0);

PS_OUT main(PS_IN pin)
{
	PS_OUT pout;
	// �F��񏑂�����
	pout.color = tex.Sample(samp, pin.uv);
	// �@����񏑂�����
	// �@���̏���-1�`1�����A�e�N�X�`����
	// 0�`1�ň������߁A-1�`1��0�`1�ւ̕ϊ����K�v
	pout.normal = float4(pin.normal * 0.5f + 0.5f, 1.0f);
	// ���[���h���W�̏�������
	// ���[�x�n���������ޕ����悢
	// ���[���h���W����x,y,z�̂R�������K�v�����A�������ݐ��
	// �F��rgb�ebyte�����Ȃ��B
	// �[�x�n���烏�[���h���W�𕜌����邱�Ƃ��ł���̂ŁA
	// 4byte�g���Đ��m�ɕۑ��ł���
	// pout.world = pin.wPos; �����[���h���W�̓_��
	float depth = pin.screenPos.z / pin.screenPos.w;
	pout.world = float4(depth, depth, depth, depth);

	return pout;
}