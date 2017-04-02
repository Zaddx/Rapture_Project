//////////////// 
//   GLOBALS  //
////////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
}

///////////////
// TYPEDEFS  //
///////////////
struct VertexInputType
{
	float4 pos : POS;
	float2 uv  : UV;
}

struct PixelInputType
{
	float4 pos : POS;
	float2 uv  : UV;
}

///////////////////////////////////////////
//             Vertex Shader             //
///////////////////////////////////////////
PixelInputType TextureVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change the postion vector to be 4 units for proper matrix calculations
	input.pos.w = 1.0f;

	// Calculate the postion of the vertex against the world, view, and projection matrices
	output.pos = mul(input.pos, worldMatrix);
	output.pos = mul(output.pos, viewMatrix);
	output.pos = mul(output.pos, projectionMatrix);

	// Store the texture coordinates for the pixel Shader
	output.uv = input.uv;

	return output;
}

