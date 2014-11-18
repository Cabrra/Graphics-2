#pragma pack_matrix( row_major )

texture2D heightmap : register(t0);
SamplerState filters[1] : register(s0);

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

cbuffer TIME : register(b2)
{
	float4 elapsedTime;
}


V_OUT main(V_IN input, uint instan : SV_INSTANCEID)
{
	V_OUT output = (V_OUT)0;
	// ensures translation is preserved during matrix multiply  

	/*float heightR = heightmap.SampleLevel(filters[0], input.UVL.xy, 0).x;
	float heightG = heightmap.SampleLevel(filters[0], input.UVL.xy, 0).y;
	float heightB = heightmap.SampleLevel(filters[0], input.UVL.xy, 0).z;*/

	//float H = /*1.0f -*/ ((heightR + heightG + heightB));// / 5.0f);

	input.posL.y = -0.5f; //* -10;

	output.unpos = input.posL;
	float4 localH = float4(input.posL, 1.0f);

		// move local space vertex from vertex buffer into world space.

		// TODO: Move into view space, then projection space
		//camera
	//localH.y = cos(0.5 * elapsedTime);
	//localH.y += cos(localH.x + elapsedTime) * localH.x *0.15f;
	/*localH.y += localH.x * cos(localH.y + elapsedTime) + sin(localH.y - elapsedTime) * 0.3f;
	localH.x +=  localH.y * sin(localH.z + elapsedTime) * -0.1f;*/

	localH.x += 2.0f * cos(localH.x + elapsedTime) * 0.01f;
	localH.y -= 2.0f * cos(localH.z + elapsedTime) *0.25f;

	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);

	output.position = localH;
	output.UVH = input.UVL;
	output.normH = mul(input.normL, (float3x3)worldMatrix);

	return output; // send projected vertex to the rasterizer stage
}