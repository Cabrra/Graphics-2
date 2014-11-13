#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture
texture2D noiseTex : register(t1); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer TIME : register(b0)
{
	float4 elapsedTime;
}

float4 main(float3 baseUV : UV, float3 normals : NORMAL,
	float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{

	float4 baseColor;

	baseColor = baseTexture.Sample(filters[0], baseUV.xy);

	//float4 finalColor = (baseColor.r + baseColor.g + baseColor.b) / 3.0f;
	//return baseColor;
	float luminanceThreshold = 0.1;
	float colorAmplification = 4;

	baseUV.x = 0.4*sin(elapsedTime.x*50.0);
	baseUV.y = 0.4*cos(elapsedTime.x*50.0);
	float3 n = noiseTex.Sample(filters[0], (baseUV*3.5) + baseUV).rgb;
	float3 c = noiseTex.Sample(filters[0], baseUV + n.xy*0.005).rgb;

	float lum = dot(float3(0.30, 0.59, 0.11), c);
	if (lum < luminanceThreshold)
	c *= colorAmplification;

	float3 visionColor = float3(0.1, 0.95, 0.2);
		return  float4((c + (n*0.2)) * visionColor, 1) * baseColor;
}