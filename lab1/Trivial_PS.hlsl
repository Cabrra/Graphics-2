#pragma pack_matrix( row_major )

texture2D baseTexture : register(t0); // first texture

SamplerState filters[1] : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer DIRECTIONALLIGHT : register(b0)
{
	float3 DirPos;
	float Dirpadding;
	float3 DirlightDir;
	float moarpadding;
	float4 DirlightCol;
}

cbuffer DIRECTIONALLIGHT2 : register(b1)
{
	float3 DirPos2;
	float Dirpadding2;
	float3 DirlightDir2;
	float moarpadding2;
	float4 DirlightCol2;
}

cbuffer SPECULAR : register(b2)
{
	float4 CamWorldPos;
	float3 SpecDir; //you don't need this you use each light direction and therefore you need to calculate the specular of each light
	float power; //use high power like 25
	float specInt; //1
	float3 padel;
	float4 color; // you don't need thois
}

float4 main(float3 baseUV : UV, float3 normals : NORMAL, float4 unpos : SV_POSITION) : SV_TARGET
{

	float4 baseColor;

	baseColor = baseTexture.Sample(filters[0], baseUV);

	clip(baseColor.w < 0.85f ? -1 : 1);

	return baseColor;
	////Directional Light 1
	//float dirRatio = saturate(dot(-DirlightDir, normals));
	//////Directional Light 2
	//float dirRatio2 = saturate(dot(-DirlightDir2, normals));
	////Specular

	//float4 viewdir = normalize(CamWorldPos - unpos);
	//	float4	halfvect = normalize(((-DirlightDir2), 0) + viewdir);
	//	float intense = max(pow(saturate(dot(normalize(normals), normalize(halfvect))), power), 0);

	//float4 spec = color * specInt * intense;


	//	return (dirRatio * DirlightCol * baseColor + spec) + (dirRatio2 * DirlightCol2 * baseColor);

}
