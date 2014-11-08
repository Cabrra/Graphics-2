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
	float4 posH : SV_POSITION;
};

V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;

	output.posH = float4(input.posL, 1.0f);
	return output;
}
