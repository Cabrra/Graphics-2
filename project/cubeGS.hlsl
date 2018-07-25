#pragma pack_matrix( row_major )

struct GSOutput
{
	float3 UVH : UV;
	float3 normH : NORMAL;  //needs to be in world space -> multiply by world
	float4 position : SV_POSITION; 
	float3 unpos : POSITION; // unaltered position
};


cbuffer SCENE : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

[maxvertexcount(36)]
void main(
	point float4 input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{

	GSOutput myCube[36];
	//front
//0
	myCube[0].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[0].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[0].normH = float3(0.0f, 1.0f, 0.0f);

//1
	myCube[1].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[1].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[1].normH = float3(0.0f, 1.0f, 0.0f);
	
//2
	myCube[2].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[2].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[2].normH = float3(0.0f, 1.0f, 0.0f);

//2
	myCube[3].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[3].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[3].normH = float3(0.0f, 1.0f, 0.0f);

//3
	myCube[4].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[4].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[4].normH = float3(0.0f, 1.0f, 0.0f);

//0
	myCube[5].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[5].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[5].normH = float3(0.0f, 1.0f, 0.0f);

	//bottom
//4
	myCube[6].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[6].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[6].normH = float3(0.0f, -1.0f, 0.0f);

//6
	myCube[7].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[7].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[7].normH = float3(0.0f, -1.0f, 0.0f);

//5
	myCube[8].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[8].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[8].normH = float3(0.0f, -1.0f, 0.0f);

//6
	myCube[9].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[9].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[9].normH = float3(0.0f, -1.0f, 0.0f);

//4
	myCube[10].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[10].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[10].normH = float3(0.0f, -1.0f, 0.0f);

//7
	myCube[11].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[11].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[11].normH = float3(0.0f, -1.0f, 0.0f);

	//left
//9
	myCube[12].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[12].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[12].normH = float3(-1.0f, 0.0f, 0.0f);

//8
	myCube[13].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[13].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[13].normH = float3(-1.0f, 0.0f, 0.0f);

//10
	myCube[14].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[14].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[14].normH = float3(-1.0f, 0.0f, 0.0f);

//10
	myCube[15].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[15].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[15].normH = float3(-1.0f, 0.0f, 0.0f);

//8
	myCube[16].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[16].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[16].normH = float3(-1.0f, 0.0f, 0.0f);

//11
	myCube[17].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[17].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[17].normH = float3(-1.0f, 0.0f, 0.0f);

	//right
//12
	myCube[18].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[18].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[18].normH = float3(1.0f, 0.0f, 0.0f);

//13
	myCube[19].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[19].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[19].normH = float3(1.0f, 0.0f, 0.0f);

//14
	myCube[20].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[20].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[20].normH = float3(1.0f, 0.0f, 0.0f);

//14
	myCube[21].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[21].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[21].normH = float3(1.0f, 0.0f, 0.0f);

//15
	myCube[22].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[22].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[22].normH = float3(1.0f, 0.0f, 0.0f);

//12
	myCube[23].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[23].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[23].normH = float3(1.0f, 0.0f, 0.0f);

	//back
//17
	myCube[24].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[24].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[24].normH = float3(0.0f, 0.0f, 1.0f);

//16
	myCube[25].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[25].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[25].normH = float3(0.0f, 0.0f, 1.0f);

//18
	myCube[26].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[26].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[26].normH = float3(0.0f, 0.0f, 1.0f);

//18
	myCube[27].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[27].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[27].normH = float3(0.0f, 0.0f, 1.0f);

//16
	myCube[28].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[28].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[28].normH = float3(0.0f, 0.0f, 1.0f);

//19
	myCube[29].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z + 5.0f, 1.0f);
	myCube[29].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[29].normH = float3(0.0f, 0.0f, 1.0f);

	//front
//20
	myCube[30].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[30].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[30].normH = float3(0.0f, 0.0f, -1.0f);

//21
	myCube[31].position = float4(input[0].x + 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[31].UVH = float3(1.0f, 0.0f, 0.0f);
	myCube[31].normH = float3(0.0f, 0.0f, -1.0f);

//22
	myCube[32].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[32].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[32].normH = float3(0.0f, 0.0f, -1.0f);

//22
	myCube[33].position = float4(input[0].x + 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[33].UVH = float3(1.0f, 1.0f, 0.0f);
	myCube[33].normH = float3(0.0f, 0.0f, -1.0f);

//23
	myCube[34].position = float4(input[0].x - 5.0f, input[0].y - 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[34].UVH = float3(0.0f, 1.0f, 0.0f);
	myCube[34].normH = float3(0.0f, 0.0f, -1.0f);

//20
	myCube[35].position = float4(input[0].x - 5.0f, input[0].y + 5.0f, input[0].z - 5.0f, 1.0f);
	myCube[35].UVH = float3(0.0f, 0.0f, 0.0f);
	myCube[35].normH = float3(0.0f, 0.0f, -1.0f);


	float4x4 mVP = mul(viewMatrix, projectionMatrix);
	for (uint i = 0; i < 36; ++i)
	{
		myCube[i].unpos = myCube[i].position.xyz;
		myCube[i].position = mul(myCube[i].position, mVP);
	}
	for (uint i = 0; i < 36; i+=3)
	{
		output.Append(myCube[i]);
		output.Append(myCube[i + 1]);
		output.Append(myCube[i + 2]);

		output.RestartStrip();
	}
}