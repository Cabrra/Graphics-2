#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture
texture2D heightmap : register(t1);

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

float4 main(float3 baseUV : UV, float3 normals : NORMAL,
	float4 pos : SV_POSITION, float3 unpos : POSITION) : SV_TARGET
{

	//calculate normals
	float2 texel0 = heightmap.Load((baseUV.x * 512, baseUV.y * 512));
	float myMinX = texel0.x - 1;
	float myMaxX = texel0.x + 1;
	float myMinY = texel0.y - 1;
	float myMaxY = texel0.y - 1;

	if (myMinX == -1)
		myMinX = 512;
	if (myMinY == -1)
		myMinY = 512;
	if (myMaxX == 512)
		myMaxX = 0;
	if (myMaxY == 512)
		myMaxY = 0;

	//vectors
	float3 texelTL = float3(-1.0f, 1.0f, heightmap.Load((myMinX, myMinY)).r);
		float3 texelTT = float3(0.0f, 1.0f, heightmap.Load((texel0.x, myMinY)).r);
		float3 texelTR = float3(1.0f, 1.0f, heightmap.Load((myMaxX, myMinY)).r);
		float3 texelML = float3(-1.0f, 0.0f, heightmap.Load((myMinX, texel0.y)).r);
		float3 texelCC = float3(0.0f, 0.0f, heightmap.Load((myMinX, texel0.y)).r);
		float3 texelMR = float3(1.0f, 0.0f, heightmap.Load((myMaxX, texel0.y)).r);
		float3 texelBL = float3(-1.0f, -1.0f, heightmap.Load((myMinX, myMaxY)).r);
		float3 texelBB = float3(0.0f, -1.0f, heightmap.Load((texel0.x, myMaxY)).r);
		float3 texelBR = float3(1.0f, -1.0f, heightmap.Load((myMaxX, myMaxY)).r);

		//averaging starting in TOP LEFT
		float3 calcul1 = cross((texelTL - texelCC), (texelTT - texelCC));
		float3 calcul2 = cross((texelTT - texelCC), (texelTR - texelCC));
		float3 calcul3 = cross((texelTR - texelCC), (texelMR - texelCC));
		float3 calcul4 = cross((texelMR - texelCC), (texelBR - texelCC));
		float3 calcul5 = cross((texelBR - texelCC), (texelBB - texelCC));
		float3 calcul6 = cross((texelBB - texelCC), (texelBL - texelCC));
		float3 calcul7 = cross((texelBL - texelCC), (texelML - texelCC));
		float3 calcul8 = cross((texelML - texelCC), (texelTL - texelCC));

		//set new normal

		normals = (calcul1 + calcul2 + calcul3 + calcul4 + calcul5 + calcul6 + calcul7 + calcul8) / 8;

	float2 sample1;
	float2 sample2;
	float2 sample3;
	float2 sample4;
	float2 sample5;
	float2 sample6;
	float2 sample7;
	float2 sample8;

	float4 baseColor;
	float4 ambient = float4(0.3f, 0.3f, 0.3f, 1.0f);
		baseColor = baseTexture.Sample(filters[0], baseUV);

	//clip(baseColor.w < 0.85f ? -1 : 1);

	ambient = (baseColor * ambient);

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
	float innerAttenua = 1.0f - saturate(length(lightPos - unpos) / radius);
	innerAttenua *= innerAttenua;
	float spotAtten = 1.0f - saturate((inner - surfaceRat) / (inner - outer));
	float4 spot = spotRatio * spotCol * baseColor * innerAttenua * spotAtten;

		//fog 
		float4 litColor = ambient + directional + pointL + spot;
		if (pad == 1.0f) // && length(viewPos) < 200) //&& length(viewPos.z) < 30))
		{
		float  fogStart = 0.0f;
		float  fogRange = 70.0f;
		float4 fogColor = float4(0.7f, 0.7f, 0.7f, 1.0f);

			//1
			float fogLerp = 1.0f - saturate((length(float4(0, 0, 0, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		//2
		fogStart = 0.0f;
		fogRange = 105.0f;
		fogLerp = 1.0f - saturate((length(float4(-100, 0, -100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		//3
		fogLerp = 1.0f - saturate((length(float4(100, 0, -100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		//4
		fogLerp = 1.0f - saturate((length(float4(-100, 0, 100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		//5
		fogLerp = 1.0f - saturate((length(float4(100, 0, 100, 1) - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		//view
		fogStart = 5.0f;
		fogRange = 15.0f;
		fogLerp = /*1.0f -*/ saturate((length(viewPos - unpos) - fogStart) / fogRange);
		litColor = lerp(litColor, fogColor, saturate(fogLerp - 0.3f));
		}
	return litColor;



}
