SamplerState SampleType;
Texture2D firstTexture : register(t0);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;
};


float4 main(PS_IN input) : SV_TARGET
{
	float4 textureColor;
	textureColor = firstTexture.Sample(SampleType, input.uv);
	return textureColor;
	//return input.color;
}