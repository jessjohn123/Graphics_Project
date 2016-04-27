#pragma pack_matrix(row_major)

struct GS_IN
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct GS_OUTPUT
{
	float4 pos : SV_POSITION;
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

[maxvertexcount(6)]
void main(point GS_IN input[1], uint id : SV_GSInstanceID, inout TriangleStream < GS_OUTPUT > output)
{
	GS_OUTPUT verts[4] =
	{
		{float4(-0.5f + input[0].position.x, 0.5f + input[0].position.y, -0.5f + input[0].position.z,1), input[0].color},
		{float4(0.5f + input[0].position.x, 0.5f + input[0].position.y, -0.5f + input[0].position.z,1), input[0].color},
		{float4(-0.5f + input[0].position.x, -0.5f + input[0].position.y, -0.5f + input[0].position.z,1), input[0].color},
		{float4(0.5f + input[0].position.x, -0.5f + input[0].position.y, -0.5f + input[0].position.z,1), input[0].color}
	};

	//prep triangle for rasterization
	for (int i = 0; i < 4; i++)
	{
		verts[i].pos = mul(verts[i].pos, worldMatrix);
		verts[i].pos = mul(verts[i].pos, viewMatrix);
		verts[i].pos = mul(verts[i].pos, projectionMatrix);
	}

	//send verts to the rasterizer
	output.Append(verts[0]);
	output.Append(verts[1]);
	output.Append(verts[3]);
	output.RestartStrip();

	output.Append(verts[0]);
	output.Append(verts[3]);
	output.Append(verts[2]);
	output.RestartStrip();

}
