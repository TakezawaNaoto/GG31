struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 screenPos : TEXCOORD0;
};

// ����������Ȃ�A�\���̂����قǂł��Ȃ��̂�
// main�֐��̈����ɏ��������Z�}���e�B�N�X���w�肷��
float main(PS_IN pin) : SV_TARGET
{
	float depth = pin.screenPos.z / pin.screenPos.w;
	return depth;
}