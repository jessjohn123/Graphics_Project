#pragma pack_matrix(row_major)

struct V_IN
{
	float4 posL : POSITION;
	float4 color : COLOR;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};
struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float4 m_localCoord : COORD;
	float4 posW : WPOSITION;
	float3 tangent : TANGENT;
};
cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}
cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}
V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;
	float4 worldPosition;
	// ensures translation is preserved during matrix multiply  
	float4 localH = float4(input.posL.xyz, 1);
	output.m_localCoord = localH;
	// move local space vertex from vertex buffer into world space.
	localH = mul(localH, worldMatrix);
	//worldPosition = localH;
	//output.posW = worldPosition;

	// TODO: Move into view space, then projection space
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);
	output.posH = localH;
	output.tex = input.tex;

	//cal normal vec against the world matrix
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	//cal the pos of the vertex in the world
	output.posW = mul(input.posL, worldMatrix);

	//Cross product helps in finding the perpendicular vec of our TBN matrix
	//output.TBN[0] = mul(float4(normalize(input.tangent), 0), worldMatrix).xyz;
	//output.TBN[1] = mul(float4(cross(normalize(input.normal), normalize(input.tangent)), 0), worldMatrix).xyz;
	//output.TBN[2] = mul(float4(normalize(input.normal), 0), worldMatrix).xyz;

	////cal the tangent vec against the world matrix
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);

	////cal the bi-normal vec against the world matrix 
	//output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	//output.binormal = normalize(output.binormal);

	return output; // send projected vertex to the rasterizer stage
}
