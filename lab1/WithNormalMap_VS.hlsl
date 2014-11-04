#pragma pack_matrix( row_major )

struct V_IN
{
	float3 UVL : UV;
	float3 normL : NORMAL;
	float3 posL : POSITION;
	float3 tang : TANGENT;
};

struct V_OUT
{
	float3 UVH : UV;
	float3 normH : NORMAL;  //needs to be in world space -> multiply by world
	float4 position : SV_POSITION; 
	float3 unpos : POSITION; // unaltered position
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


cbuffer INSTANCEMATRIX : register(b2)
{
	float4x4 position[100];
}

V_OUT main(V_IN input, uint instan : SV_INSTANCEID)
{
	V_OUT output = (V_OUT)0;
	// ensures translation is preserved during matrix multiply  
	float4 localH = float4(input.posL, 1.0f);

	// move local space vertex from vertex buffer into world space.

	// TODO: Move into view space, then projection space
	//camera


	localH = mul(localH, position[instan]);
	output.unpos = localH;

	localH = mul(localH, viewMatrix);

	localH = mul(localH, projectionMatrix);

	output.position = localH;
	output.UVH = input.UVL;
	output.normH = mul(input.normL, (float3x3)position[instan]);
	output.normH = normalize(output.normH);

	return output; // send projected vertex to the rasterizer stage
}