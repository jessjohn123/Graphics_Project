//#pragma pack_matrix(row_major)
#include "LightingCalcs.hlsli"

SamplerState SampleType;
Texture2D m_textureColor : register(t0);
Texture2D m_textureForNormalMap : register(t1);
Texture2D m_secondTextureForNormalMap : register(t2);

cbuffer DirectionalLightBuffer : register(b0)
{
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};

cbuffer PointLightBuffer : register(b1)
{
	float4 diffuseColorForPointLight;
	float3 lightPosForPointLight;
	float radius;
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

};

float4 main(PS_IN input) : SV_TARGET
{
	float4 textureColor, NormalMapColor, NormalMapColor1;
	float3 lightDir, normMap, normMap1;
	float lightIntensity;

	float4 color, color1, color2;

	//sample the texture pixel at this location
	textureColor = m_textureColor.Sample(SampleType, input.tex);
	
	//sample the pixel in the normal map
	NormalMapColor = m_textureForNormalMap.Sample(SampleType, input.tex);
	NormalMapColor1 = m_secondTextureForNormalMap.Sample(SampleType, input.tex);

	//expanding the range of the normal val from(0, +1) to (-1, +1).
	//normMap.xyz = normalize((normMap.xyz * 2.0f) - 1.0f);
	NormalMapColor = (NormalMapColor * 2.0f) - 1.0f;
	NormalMapColor1 = (NormalMapColor1 * 2.0f) - 1.0f;
	//NormalMapColor.y = -NormalMapColor.y;

	// create the tbn matrix
	float3x3 TBN;
	TBN[0] = input.tangent;
	TBN[1] = cross(float4(input.normal, 0), float4(input.tangent, 0));
	TBN[2] = input.normal;

	//TBN[0] = float3(1.0f, 0.0f, 0.0f);
	//TBN[1] = float3(0.0, 0.0f, 1.0f);
	//TBN[2] = float3(0.0f, 1.0f, 0.0f);

	//cal the bump normal from the data in the normal map
	//normMap.xyz = mul(normMap.xyz, input.TBN);
	normMap = mul(NormalMapColor.xyz, TBN);
	normMap1 = mul(NormalMapColor1.xyz, TBN);

	//Normalize the resulting bump normal 
	normMap = normalize(normMap);
	normMap1 = normalize(normMap1);

    //dot product btw normal vec & the light dir
	lightDir = -lightDirection;

	//Using the Directional Cals Func
	color = CalDirectionlLight(lightDirection, diffuseColor, normMap);
	color1 = CalPointLight(lightPosForPointLight, diffuseColorForPointLight, normMap, input.posW, radius);
	color2 = CalPointLight(lightPosForPointLight, diffuseColorForPointLight, normMap1, input.posW, radius);

	//Combine the final normal light color with the texture color
	color = color1 + color2 * textureColor;

	return color;
}