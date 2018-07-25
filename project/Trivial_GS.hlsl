

struct GSInput
{
	float3 UVH : UV;
	float3 normH : NORMAL;  //needs to be in world space -> multiply by world
	float4 positionH : SV_POSITION;
	float3 unposH : POSITION; // unaltered position
};

struct GSOutput
{
	float3 UV : UV;
	float3 norm : NORMAL;  //needs to be in world space -> multiply
	float4 position : SV_POSITION;
	float3 unpos : POSITION; // unaltered position
};

[maxvertexcount(3)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
	for (int i = 0; i < 3; i++)
	{
		GSOutput element;
		element.UV = input[i].UVH;
		element.position = input[i].positionH;
		element.unpos = input[i].unposH;

		element.norm = normalize(cross(input[0].positionH - input[1].positionH, input[0].positionH - input[2].positionH));

		output.Append(element);
	}
}