//////////////// 
//   GLOBALS  //
////////////////
Texture2D shaderTexture;
SamplerState SampleType;

///////////////
// TYPEDEFS  //
///////////////
struct PixelInputType
{
	float4 pos : POS;
	float2 uv  : UV;
}

///////////////////////////////////////////
//              Pixel Shader             //
///////////////////////////////////////////
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;

	// Sample the pixel color from the texture using the sampler at this texture coordinte location.
	textureColor = shaderTexture.Sample(SampleType, input.uv);

	return textureColor;
}