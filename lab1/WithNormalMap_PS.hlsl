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
	float3 SpecDir; //you don't need this you use each light direction and therefore you need to calculate the specular of each light
	float power; //use high power like 25
	float specInt; //1
	float3 padel;
	float4 color; // you don't need thois
}


cbuffer SCENE : register(b4)
{
	float4 viewPos;
}

float4 main(float3 baseUV : UV, float3 normals : NORMAL, float4 pos : SV_POSITION, float3 unpos : POSITION, float3 tang : TANGENT) : SV_TARGET
{

	float4 ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float4 baseColor = baseTexture.Sample(filters[0], baseUV);
	clip(baseColor.w < 0.35f ? -1 : 1);
		
	ambient = (baseColor * ambient);


	float3 norMap = normalMap.Sample(filters[0], baseUV);
	norMap = (2.0f * norMap) - 1.0f;
		//TANGENTS 


		// Build orthonormal basis.
		float3 N = normals;
		float3 T = normalize(tang - dot(tang, N)*N);
		float3 B = cross(N, T);

		float3x3 TBN = float3x3(T, B, N);

		// Transform from tangent space to world space.
		float3 bumpedNormal = mul(norMap, TBN);

		normals = normalize(bumpedNormal);

	////Directional Light 
		float dirRatio = saturate(dot(-DirlightDir, normals));
	float4 directional = (dirRatio * DirlightCol * baseColor);

	//Point light
	float len = length(lightPos - unpos);
	float4 pointL = float4 (0.0f, 0.0f, 0.0f, 1.0f);
		if (len <= range)
		{
		float3 dir = normalize(lightPos - unpos);
			float ratio = saturate(dot(dir, normals));
		float atten = 1.0 - saturate(length(lightPos - unpos) / radiusPoint);
		atten *= atten;
		pointL = ratio * lightCol * baseColor * atten;
		}

	//spot light
	float3 sDir = normalize(spotPos - unpos);
	float surfaceRat = saturate(dot(-sDir, spotlightDir));
	float spotRatio = saturate(dot(sDir, normals));
	float innerAttenua = 1.0f - saturate(length(spotPos - unpos) / radius);
	innerAttenua *= innerAttenua;
	float spotAtten = 1.0f - saturate((inner - surfaceRat) / (inner - outer));
	float4 spot = spotRatio * spotCol * baseColor * innerAttenua * spotAtten;

		////Specular
		//float4 viewdir = normalize(CamWorldPos - unpos);
		//	float4	halfvect = normalize(((-DirlightDir2), 0) + viewdir);
		//	float intense = max(pow(saturate(dot(normalize(normals), normalize(halfvect))), power), 0);

		//float4 spec = color * specInt * intense;

	//fog 
	float4 litColor = ambient + directional + pointL + spot;
	if (pad == 1.0f)// && length(viewPos) < 200) && length(viewPos.z) < 30))
	{
		float  fogStart = 0.0f;
		float  fogRange = 70.0f;
		float4 fogColor = float4 (0.7f, 0.7f, 0.7f, 1.0f);

			//1
			float fogLerp = 1.0f - saturate((length(float4(0, 0, 0, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.7f));
		//2
		fogStart = 0.0f;
		fogRange = 105.0f;
		fogLerp = 1.0f - saturate((length(float4(-100, 0, -100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.7f));
		//3
		fogLerp = 1.0f - saturate((length(float4(100, 0, -100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.7f));
		//4
		fogLerp = 1.0f - saturate((length(float4(-100, 0, 100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.7f));
		//5
		fogLerp = 1.0f - saturate((length(float4(100, 0, 100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.7f));
		//view
		fogStart = 5.0f;
		fogRange = 15.0f;
		fogLerp = /*1.0f -*/ saturate((length(viewPos - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
	}
	return litColor;

	//return ambient + directional + pointL + spot;
}
