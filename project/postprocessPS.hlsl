#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture
texture2D secondTex : register(t1); // second texture
texture2D noiseTex : register(t2); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer DIRECTIONALLIGHT : register(b0)
{
	float3 DirPos;
	float Dirpadding;
	float3 DirlightDir;
	float moarpadding;
	float4 DirlightCol;
}

cbuffer POINTLIGHT : register(b1)
{
	float3 lightPos;
	float range;
	float3 lightDir;
	float radiusPoint;
	float4 lightCol;
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

cbuffer SPECULAR : register(b3)
{
	float4 CamWorldPos;
	float power; //use high power like 25
	float3 pid;
	float specInt; //1
	float3 padel;
}

cbuffer TIME : register(b4)
{
	float4 elapsedTime;
}

float4 main(float3 baseUV : UV, float3 normals : NORMAL,
	float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{

	float4 baseColor;
	float4 secondColor;

	baseColor = baseTexture.Sample(filters[0], baseUV.xy);
	secondColor = secondTex.Sample(filters[0], baseUV.xy);

	baseColor = baseColor * secondColor * 2.0f;

	//clip(baseColor.w < 0.85f ? -1 : 1);
	////Directional Light 
	float dirRatio = saturate(dot(-normalize(DirlightDir), normals));
	float4 directional = (dirRatio * DirlightCol * baseColor);

		//Point light
		float4 pointL = float4 (0.0f, 0.0f, 0.0f, 1.0f);

		float3 dir = normalize(lightPos - unpos);
		float ratio = saturate(dot(dir, normals));
	float atten = 1.0 - saturate(length(lightPos - unpos) / radiusPoint);
	atten *= atten;
	pointL = ratio * lightCol * float4(baseColor.xyz, 1.0f) * atten;

	////spot light
	//float3 sDir = normalize(spotPos - unpos);
	//float surfaceRat = saturate(dot(-sDir, spotlightDir));
	//float spotRatio = saturate(dot(sDir, normals));
	//float innerAttenua = 1.0f - saturate(length(lightPos - unpos) / radius);
	//innerAttenua *= innerAttenua;
	//float spotAtten = 1.0f - saturate((inner - surfaceRat) / (inner - outer));
	//float4 spot = spotRatio * spotCol * (baseColor, 1.0f) * innerAttenua * spotAtten;

	//Specular
	float4 viewdir = normalize(CamWorldPos - float4(unpos, 0.0f));
		float4	halfvect = normalize(float4(-DirlightDir, 0.0f) + viewdir);
		float intense = max(pow(saturate(dot(normalize(normals), normalize(halfvect))), power), 0);

	float4 spec = float4(0.0f, 0.2f, 1.0f, 1.0f) * specInt * intense;

		//return (baseColor *0.85) + directional + spec; // + pointL +spot;

	float luminanceThreshold = 0.1;
	float colorAmplification = 4;

	baseUV.x = 0.4*sin(elapsedTime.x*50.0);
	baseUV.y = 0.4*cos(elapsedTime.x*50.0);
	float3 n = noiseTex.Sample(filters[0],
		(baseUV*3.5) + baseUV).rgb;
	float3 c = noiseTex.Sample(filters[0], baseUV + n.xy*0.005).rgb;

		float lum = dot(float3(0.30, 0.59, 0.11), c);
	if (lum < luminanceThreshold)
		c *= colorAmplification;

	float3 visionColor = float3(0.1, 0.95, 0.2);
		return  float4((c + (n*0.2)) * visionColor, 1) * ((baseColor *0.85) + directional + spec);
}
