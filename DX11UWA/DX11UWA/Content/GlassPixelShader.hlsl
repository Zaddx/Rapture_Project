SamplerState SampleType;

texture2D colorTexture : register(t0);
texture2D normalTexture : register(t1);
texture2D refractionTexture : register(t2);

cbuffer GlassBuffer
{
	float refreactionScale;
	float3 padding;
}

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 uv : UV;
	float4 refractionPosition : REFRACTION_POSITION;
}

float4 GlassPixelShader(PixelInputType input) : SV_TARGET
{
	float2 refractTextCoord;
	float4 normalMap;
	float3 normal;
	float4 refractionColor;
	float4 textureColor;
	float4 color;

	// Calculate the projected refraction texture coordinates.
	refractTextCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
	refractTextCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

	// Sample the normal from the normal map texture.
	normalMap = normalTexture.Sample(SampleType, input.uv);

	// Expand the range of the normal from (0,1) to (-1, +1)
	normal = (normalMap.xyz * 2.0f) - 1.0f;

	// Re-position the texutre coordinates sampling position by the normal map value to simulate light distortion through glass.
	refractTextCoord = refractTextCoord + (normal.xy * refreactionScale);

	// Sample the texture pixel from the refreaction texture using the perturbed texture coordinates.
	refractionColor = refractionTexture.Sample(SampleType, refractTextCoord);

	// Sample the texture pixel from the glass color texture.
	textureColor = colorTexture.Sample(SampleType, input.uv);

	// Evenly combine the glass color and refraction value for the final color.
	color = lerp(refractionColor, textureColor, 0.5f);

	return color;
}