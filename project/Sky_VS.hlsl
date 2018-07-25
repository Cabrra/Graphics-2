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

V_OUT main(V_IN input, uint instan : SV_INSTANCEID)
{
	V_OUT output = (V_OUT)0;
	// ensures translation is preserved during matrix multiply  
	float4 localH = float4(input.posL, 1.0f);
		output.unpos = input.posL;
	// move local space vertex from vertex buffer into world space.

	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.UVH = input.posL;
	output.position = localH;
	output.normH = mul(input.normL, (float3x3) worldMatrix);
	output.normH = normalize(output.normH);
	//output.normH = input.normL;

	return output; // send projected vertex to the rasterizer stage
}
