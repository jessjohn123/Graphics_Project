#pragma pack_matrix(row_major)
//#include "LightingCalcs.hlsi"

SamplerState SampleType;
Texture2D m_textureColor : register(t0);
Texture2D m_textureForNormalMap : register(t1);

cbuffer DirectionalLightBuffer : register(b0)
{
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float4 m_localCoord : COORD;
	float4 posW : WPOSITION;
	float3 tangent : TANGENT;
	float3x3 TBN : TBN;
};

float4 main(PS_IN input) : SV_TARGET
{
	float4 textureColor, NormalMapColor, color;
	float3 lightDir, normMap;
	float lightIntensity;

	//sample the texture pixel at this location
	textureColor = m_textureColor.Sample(SampleType, input.tex);
	
	//sample the pixel in the normal map
	NormalMapColor = m_textureForNormalMap.Sample(SampleType, input.tex);

	//expanding the range of the normal val from(0, +1) to (-1, +1).
	normMap.xyz = normalize((normMap.xyz * 2.0f) - 1.0f);

	//cal the bump normal from the data in the normal map
	normMap.xyz = mul(normMap.xyz, input.TBN);


	//Invert the light dir for cal
	lightDir = -lightDirection;

	//Cal the mt of light on this pixel based on the normal map normal val
	lightIntensity = saturate(dot(normMap, lightDir));

	//Determine the final diffuse color based on the diffuse color and the amt of light of light intensity
	color = saturate(diffuseColor * lightIntensity);

	//Combine the final normal light color with the texture color
	color = color * textureColor;

	return textureColor;
}