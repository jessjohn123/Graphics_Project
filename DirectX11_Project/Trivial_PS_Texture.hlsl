//#pragma pack_matrix(row_major)
#include "LightingCalcs.hlsli"

SamplerState SampleType;
Texture2D diffuseTexture : register(t0);
Texture2D blendTexture: register(t1);

cbuffer LightBuffer : register(b0)
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

cbuffer SpotLightBuffer : register(b2)
{
	float ConeRatio;
	float3 m_padding;

	float3 LightPosForSpotLight;
	float m_padding2;

	float3 ConeDirection;
	float m_padding3;

	float4 SpotLightColor;
}

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXTURE;
	float4 m_localCoord : COORD;
	float3 normal : NORMAL;
	float4 posW: WPOSITION;
};

float4 main(PS_IN input) : SV_TARGET
{
	float4 textureColor, combineTwoTextures, finalTextures;
	float3 lightDir, lightDirForPointLight, lightDirForSpotLight;
	float lightIntensity, lightRatioForPointLight, Attenuation, SpotLightRatio, SpotLightSurfaceRatio, SpotFactor;
	float4 color, color1, color2;
	float4 blendColorTogether;

	//Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = diffuseTexture.Sample(SampleType, input.tex);

	combineTwoTextures = blendTexture.Sample(SampleType, input.tex);
	if (combineTwoTextures.x == 0.0f  && combineTwoTextures.y == 0.0f && combineTwoTextures.z == 0.0f )
	{
		finalTextures = textureColor;
	}
	else
	{
		finalTextures = textureColor * combineTwoTextures;
	}

	 /*Directional Light Cal*/
	/*//dot product btw normal vector & the light dir
	lightDir = -lightDirection;

	//amt of light on pixel
	lightIntensity = saturate(dot(input.normal, lightDir));

	//diffuse color combined with light intensity
	color = saturate(diffuseColor * lightIntensity);*/

	/*Point Light Cal*/
	/*lightDirForPointLight = normalize(lightPosForPointLight.xyz - input.posW.xyz);

	lightRatioForPointLight = saturate(dot(lightDirForPointLight, input.normal));

	color1 = saturate(diffuseColorForPointLight * lightRatioForPointLight);

	Attenuation = 1.0 - saturate(length((lightPosForPointLight.xyz - input.posW.xyz) / radius));

	color1 *= Attenuation;*/

	/*Spot Light Cal*/
	/*lightDirForSpotLight = normalize(LightPosForSpotLight - input.posW.xyz);

	SpotLightSurfaceRatio = saturate(dot(-lightDirForSpotLight, ConeDirection));

	SpotFactor = (SpotLightSurfaceRatio > ConeRatio) ? 1 : 0;

	SpotLightRatio = saturate(dot(lightDirForSpotLight, input.normal));

	color2 = SpotLightRatio * SpotLightColor * SpotFactor;*/
	
	// trying implement through function
	lightDir = -lightDirection;
	color = CalDirectionlLight(lightDirection, diffuseColor, input.normal);
	//color.w = 1.0f;

	color1 = CalPointLight(lightPosForPointLight, diffuseColorForPointLight, input.normal, input.posW, radius);
	//color1.w = 1.0f;

	color2 = CalSpotLight(LightPosForSpotLight, SpotLightColor, ConeDirection, ConeRatio, input.normal, input.posW);
	
	//multiply the texture pixel and the final diffuse color
	//color = color * textureColor;

	blendColorTogether = color + color1 + color2;
	blendColorTogether = saturate(blendColorTogether);

	return  blendColorTogether * finalTextures;
}