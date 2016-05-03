SamplerState SampleType;
Texture2D m_textureColor : register(t0);
Texture2D m_textureForNormalMap : register(t1);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float4 m_localCoord : COORD;
	float4 posW : WPOSITION;
	float4 tangent : TANGENT;
	float3 binormal :  BINORMAL;
};

float4 main(PS_IN input) : SV_TARGET
{
	float4 textureColor, textureNormalMap;

	textureColor = m_textureColor.Sample(SampleType, input.tex);
	textureNormalMap = m_textureForNormalMap.Sample(SampleType, input.tex);



	return textureColor;
}