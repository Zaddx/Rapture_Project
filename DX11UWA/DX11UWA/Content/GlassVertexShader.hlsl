cbuffer MatrixBuffer 
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
}

struct VertexInputType
{
	float4 pos : POSITION;
	float2 uv  : UV;
}

struct PixelInputType
{
	float4 pos   : SV_POSITION;
	float2 uv    : UV;
	float4 rPos  : REFRACTION_POSITION;
}

PixelInputType GlassVertexShader(VertexInputType input)
{
	PixelInputType output;
	matrix viewProjectWorld;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.pos.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.pos = mul(input.pos, worldMatrix);
	output.pos = mul(output.pos, viewMatrix);
	output.pos = mul(output.pos, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;

	// Create the view projection world matrix for refraction.
	viewProjectWorld = mul(viewMatrix, projectionMatrix);
	viewProjectWorld = mul(worldMatrix. viewProjectWorld);

	// Calculate the input position against the viewProjectWorld matrix.
	output.rPos = mul(input.pos, viewProjectWorld);

	return output;
}