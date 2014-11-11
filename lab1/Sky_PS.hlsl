#pragma pack_matrix( row_major )

cbuffer SCENE : register(b0)
{
	float4 viewPos;
}
cbuffer SPOTLIGHT : register(b2)
{
	float3 spotPos;
	float pad;
	float3 spotlightDir;
	float radius;
	float4 spotCol;
	float inner;
	float outer;
	float2 morepadding;
}

textureCUBE baseTexture : register(t0); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

//float4 main(float3 baseUV : UV, float4 position : SV_POSITION, float3 normals : NORMAL, float3 unpos : POSITION) : SV_TARGET
float4 main(float3 baseUV : UV, float3 normals : NORMAL,
float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{
	float4 baseColor;

	/*return*/ baseColor = baseTexture.Sample(filters[0], baseUV);

	//if (pad == 1.0f)
	//{
	//fog 

	if (pad == 1.0f) // && length(viewPos) < 200) //&& length(viewPos.z) < 30))
		{
		float  fogStart = 0.0f;
			float  fogRange = 100.0f;
			float4 fogColor = float4 (0.5f, 0.5f, 0.5f, 0.5f);

				float fogLerp = 1.0f - saturate((length(float4(0, 0, 0, 1) - unpos) - fogStart) / fogRange);

			baseColor = lerp(baseColor, fogColor, fogLerp);
		}
	return baseColor;

}
