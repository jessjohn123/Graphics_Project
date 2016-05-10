#pragma pack_matrix(row_major)

struct V_IN
{
	float4 posL : POSITION;
	float4 color : COLOR;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
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
	// move local space vertex from vertex buffer into world space.
	//localH = mul(localH, worldMatrix);

	// TODO: Move into view space, then projection space
	//localH = mul(localH, viewMatrix);
	//localH = mul(localH, projectionMatrix);

	output.posH = localH;

	output.color = input.color;

	return output;
}