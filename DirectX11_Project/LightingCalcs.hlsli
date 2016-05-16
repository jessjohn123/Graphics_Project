#pragma pack_matrix(row_major)


float4 CalDirectionlLight(float3 m_lightDir, float4 color, float3 s_normal)
{
	float lightIntensity = saturate(dot(-m_lightDir, s_normal));
	float4 _color = lightIntensity * color;
	return _color;
}

float4 CalPointLight(float3 m_lightPosForPointLight, float4 color, float3 s_normal, float4 m_posW, float m_radius)
{
	float3 PointLightDir = normalize(m_lightPosForPointLight - m_posW);
	float pointLightRatio = saturate(dot(PointLightDir, s_normal));
	float4 m_color = saturate(color * pointLightRatio);
	float attenuation = 1.0f - saturate(length((m_lightPosForPointLight - m_posW) / m_radius));
	m_color *= attenuation;
	return m_color;
}

float4 CalSpotLight(float3 lightPosForSpotLight, float4 color, float3 m_coneDir, float m_coneRatio ,float3 s_normal, float4 m_posW)
{
	float3 SpotLightDir = normalize(lightPosForSpotLight - m_posW);
	float SpotLightSurfaceRatio = saturate(dot(-SpotLightDir, m_coneDir));
	float SpotFactor = (SpotLightSurfaceRatio > m_coneRatio) ? 1 : 0;
	float SpotLightRatio = saturate(dot(SpotLightDir, s_normal));
	float4 m_color = SpotLightRatio * color * SpotFactor;
	return m_color;
}