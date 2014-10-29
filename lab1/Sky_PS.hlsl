#pragma pack_matrix( row_major )

textureCUBE baseTexture : register(t0); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

float4 main(float3 baseUV : UV, float4 position : SV_POSITION, float3 normals : NORMAL, float3 unpos : POSITION) : SV_TARGET
{
	float4 baseColor;

	return baseColor = baseTexture.Sample(filters[0], baseUV);
}
