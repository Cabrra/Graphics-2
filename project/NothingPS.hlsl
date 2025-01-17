#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP


float4 main(float3 baseUV : UV, float3 normals : NORMAL,
	float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{

	float4 baseColor;

	baseColor = baseTexture.Sample(filters[0], baseUV.xy);

	//float4 finalColor = (baseColor.r + baseColor.g + baseColor.b) / 3.0f;
	return baseColor;
}