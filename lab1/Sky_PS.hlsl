#pragma pack_matrix( row_major )

cbuffer SCENE : register(b0)
{
	float4 viewPos;
}

textureCUBE baseTexture : register(t0); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

float4 main(float3 baseUV : UV, float4 position : SV_POSITION, float3 normals : NORMAL, float3 unpos : POSITION) : SV_TARGET
{
	float4 baseColor;

	return baseColor = baseTexture.Sample(filters[0], baseUV);

	float  fogStart = 25.0f;
	 float  fogRange = 25.0f;
	 float4 fogColor = float4 (0.5f, 0.5f, 0.5f, 1.0f);

		 float fogLerp = saturate((length(viewPos - unpos) - fogStart) / fogRange);

	float4 litColor = lerp(baseColor, fogColor, fogLerp);

	 return litColor;

}
