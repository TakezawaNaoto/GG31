struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float4 color : TEXCOORD2;
	float4 wPos : TEXCOORD3;
};

cbuffer Camera : register(b1)
{
	float4 camPos;
};

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1,1,1,1);

	float3 lightPos = float3(10, 10, 10);

	// ���̐F * �g�U�� * dot(�@��,���̃x�N�g��)
	float3 N = normalize(pin.normal);
	float3 L = normalize(lightPos);

	float NL = saturate(dot(N, L));
	float diffuse = 1.0f / 3.1415926535f;

	color.rgb = diffuse * NL;

	float3 V = normalize(camPos - pin.wPos);
	float3 H = normalize(L + V);

	float roughness = 0.5f; // �e���������p�����[�^(���炩(0)-�e��(1)
	float pi = 3.141592f;

	// �}�C�N���t�@�Z�b�g( GGX
	// D = a^2 / (�� * ((n�Eh)^2 * (a^2 - 1) + 1)^2)
	float a2 = roughness * roughness;
	float D = a2 / (pi * pow(
		pow(dot(N, H), 2.0f) * (a2 - 1.0f) + 1.0f,
		2.0f));

	// �􉽌��� ( Smith + Schlick
	// k = roughness / sqrt(2/pi);
	// Schlick(v) = (n�Ev) / ((n�Ev) * (1 - k) + k)
	// G = Schlick(L) * Schlick(V)
	float k = roughness * sqrt(2.0f/pi);
	float sl = dot(N, L) / (dot(N, L) * (1.0f - k) + k);
	float sv = dot(N, V) / (dot(N, V) * (1.0f - k) + k);
	float G = sl * sv;

	// �t���l��
	// F0 = 0.5f + 2.0f * (H, L)^2 * roughness;
	// F = F0+(1 - F0) * (1.0f - dot(v�Eh))^5
	float f0 = 0.5f + 2.0f * pow(dot(H, L), 2.0f) * roughness;
	float F = f0 + (1.0f - f0) * pow(1.0f - dot(V, H), 5.0f);


	float R = (D) / (4 * dot(N, L) * dot(N, V));

	color.rgb = R;

	return color;
};