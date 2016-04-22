SamplerState SampleType;
Texture2D diffuseTexture;

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
	float4 textureColor;
	float3 lightDir, lightDirForPointLight;
	float lightIntensity, lightRatioForPointLight, Attenuation;
	float4 color, color1;
	float4 blendColorTogether;


	//Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = diffuseTexture.Sample(SampleType, input.tex);

	//dot product btw normal vector & the light dir
	lightDir = -lightDirection;

	//amt of light on pixel
	lightIntensity = saturate(dot(input.normal, lightDir));

	//pointLight cal
	lightDirForPointLight = normalize(lightPosForPointLight - input.posW);

	lightRatioForPointLight = saturate(dot(lightDirForPointLight, input.normal));

	color1 = saturate(diffuseColorForPointLight * lightRatioForPointLight);

	//color1 = color1 * textureColor;

	Attenuation = 1.0 - saturate(length((lightPosForPointLight - input.posW) / radius));

	color1 *= Attenuation;

	//diffuse color combined with light intensity
	color = saturate(diffuseColor * lightIntensity);

	//multiply the texture pixel and the final diffuse color
	//color = color * textureColor;

	blendColorTogether = color + color1;
	blendColorTogether = saturate(blendColorTogether);

	return  blendColorTogether * textureColor;
}