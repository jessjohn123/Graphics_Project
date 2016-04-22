TextureCube m_texture: register(t0);
SamplerState m_sampleState: register(s0);

struct PS_IN
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXTURE;
	float4 localCoord : COORD;
};

float4 main(PS_IN input) : SV_TARGET
{
	float4 textureForSkybox;
	textureForSkybox = (m_texture.Sample(m_sampleState, input.localCoord.xyz));
	return textureForSkybox;
}