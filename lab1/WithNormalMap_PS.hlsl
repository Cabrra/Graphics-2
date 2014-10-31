#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture
texture2D normalMap : register(t1);

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
	float padding;
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
	float3 SpecDir; //you don't need this you use each light direction and therefore you need to calculate the specular of each light
	float power; //use high power like 25
	float specInt; //1
	float3 padel;
	float4 color; // you don't need thois
}

float4 main(float3 baseUV : UV, float3 normals : NORMAL, float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{

	float4 ambient = float4(0.1f, 0.1f, 0.1f, 1.0f);
	float4 baseColor = baseTexture.Sample(filters[0], baseUV);
	clip(baseColor.w < 0.85f ? -1 : 1);
		
	ambient = (baseColor * ambient);

	//normalize
	//float3 N = (2.0 * (normalMap.Sample(filters[0], baseUV), baseUV)) - 1.0;
	float3 N = normalMap.Sample(filters[0], baseUV);
	// diffuse
	//float D = saturate(dot(N, normalize(DirlightDir)));

	////Directional Light 
	float dirRatio = saturate(dot(-DirlightDir, N));
	float4 directional = (dirRatio * DirlightCol * baseColor);

	//Point light
	float3 dir = normalize(lightPos - unpos);
	float ratio = saturate(dot(dir, N));
	float atten = 1.0 - saturate(length(lightPos - unpos) / radiusPoint);
	atten *= atten;
	float4 pointL = ratio * lightCol * baseColor * atten;

	//spot light
	float3 sDir = normalize(spotPos - unpos);
	float surfaceRat = saturate(dot(-sDir, spotlightDir));
	float spotRatio = saturate(dot(sDir, N));
	float innerAttenua = 1.0f - saturate(length(spotPos - unpos) / radius);
	innerAttenua *= innerAttenua;
	float spotAtten = 1.0f - saturate((inner - surfaceRat) / (inner - outer));
	float4 spot = spotRatio * spotCol * baseColor * innerAttenua * spotAtten;

		////Specular
		//float4 viewdir = normalize(CamWorldPos - unpos);
		//	float4	halfvect = normalize(((-DirlightDir2), 0) + viewdir);
		//	float intense = max(pow(saturate(dot(normalize(normals), normalize(halfvect))), power), 0);

		//float4 spec = color * specInt * intense;

		return ambient /*+ directional*/ + pointL + spot;
}
