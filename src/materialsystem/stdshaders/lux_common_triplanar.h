//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	27.03.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_TRIPLANAR_H_
#define LUX_COMMON_TRIPLANAR_H_

// Weights must Range [0..1]
//
// Stock Shaders do
// vNormal = normalize(vNormal)  : Unit Vector
// Weights = vNormal * vNormal   : [0..1]
// This works because -x * -x always returns a positive Value
// Gives us a nice Curve too!
float3 Triplanar_ComputeWeights(float3 vNormal)
{
	return vNormal * vNormal;
}

// uvw should be WorldPos * Triplanar_Scale + Triplanar_Offset 
float4 Triplanar_tex2D(sampler Sampler_Triplanar, float3 Weights, float3 uvw)
{
	// Only want the Fractions here
	// If we use exponents its gonna loop like crazy and not be very good
	float2 TexCoord_wu = frac(uvw.zy);
	float2 TexCoord_uw = frac(uvw.xz);
	float2 TexCoord_uv = frac(uvw.xy);

	// 3 Directions, 3 Samples
	float4 Color1 = tex2D(Sampler_Triplanar, TexCoord_wu);
	float4 Color2 = tex2D(Sampler_Triplanar, TexCoord_uw);
	float4 Color3 = tex2D(Sampler_Triplanar, TexCoord_uv);

	// Weight and return
	return (Color1 * Weights.xxxx) + (Color2 * Weights.yyyy) + (Color3 * Weights.zzzz);
}
#endif // LUX_COMMON_TRIPLANAR_H_