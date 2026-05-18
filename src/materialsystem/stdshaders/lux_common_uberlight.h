//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.05.2026 DMY
//	Last Change :	06.05.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_UBERLIGHT_H_
#define LUX_COMMON_UBERLIGHT_H_

// Vectorized smoothstep for doing three smoothsteps at once.  Used by uberlight
float3 smoothstep3(float3 edge0, float3 edge1, float3 OneOverWidth, float3 x)
{
	x = saturate((x - edge0) * OneOverWidth);	// Scale, bias and saturate x to the range of zero to one
	return x * x * (3.0f - 2.0f * x);			// Evaluate polynomial
}

// Superellipse soft clipping
//
// Input:
//   - Point Q on the x-y plane
//   - The equations of two superellipses (with major/minor axes given by
//     a,b and A,B for the inner and outer ellipses, respectively)
//   - This is changed a bit from the original RenderMan code to be better vectorized
//
// Return value:
//   - 0 if Q was inside the inner ellipse
//   - 1 if Q was outside the outer ellipse
//   - smoothly varying from 0 to 1 in between
float2 ClipSuperellipse(float2 Q, // Point on the xy plane
	float4 aAbB,				  // Dimensions of superellipses
	float2 rounds)				  // Same roundness for both ellipses
{
	float2 qr, Qabs = abs(Q);				// Project to +x +y quadrant

	float2 bx_Bx = Qabs.x * aAbB.zw;
	float2 ay_Ay = Qabs.y * aAbB.xy;

	// ShiroDkxtro2: Compiler complains because pow with negative f
	// Let's solve that
	bx_Bx = max(0.0f, bx_Bx);
	ay_Ay = max(0.0f, ay_Ay);

	qr.x = pow(pow(bx_Bx.x, rounds.x) + pow(ay_Ay.x, rounds.x), rounds.y);  // rounds.x = 2 / roundness
	qr.y = pow(pow(bx_Bx.y, rounds.x) + pow(ay_Ay.y, rounds.x), rounds.y);  // rounds.y = -roundness/2

	return qr * aAbB.xy * aAbB.zw;
}

// Volumetric light shaping
//
// Inputs:
//   - the point being shaded, in the local light space
//   - all information about the light shaping, including z smooth depth
//     clipping, superellipse xy shaping, and distance falloff.
// Return value:
//   - attenuation factor based on the falloff and shaping
float PerformUberlight(float3 PL,	// Point in light space
	float3 smoothEdge0,				// edge0 for three smooth steps
	float3 smoothEdge1,				// edge1 for three smooth steps
	float3 smoothOneOverWidth,		// width of three smooth steps
	float2 shear,					// shear in X and Y
	float4 aAbB,					// Superellipse dimensions
	float2 rounds)					// two functions of roundness packed together
{
	float2 qr = ClipSuperellipse((PL.xy / PL.zz) - shear, aAbB, rounds);

	smoothEdge0.x = qr.x;					// Fill in the dynamic parts of the smoothsteps
	smoothEdge1.x = qr.y;					// The other components are pre-computed outside of the shader
	smoothOneOverWidth.x = 1.0f / (qr.y - qr.x);
	float3 x = float3(1.0f, PL.z, PL.z);

	float3 atten3 = smoothstep3(smoothEdge0, smoothEdge1, smoothOneOverWidth, x);

	// Modulate the three resulting attenuations (flipping the sense of the attenuation from the superellipse and the far clip)
	return (1.0f - atten3.x) * atten3.y * (1.0f - atten3.z);
}

#endif