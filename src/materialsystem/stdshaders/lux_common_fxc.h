//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	08.08.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#include "lux_common_constants.h"

#ifndef LUX_COMMON_FXC_H_
#define LUX_COMMON_FXC_H_

//==========================================================================//
// Color related Functions
//==========================================================================//

// Unweighted Mean of RGB
float Average(float3 f3Color)
{
	return dot(f3Color, 1.0f/3.0f); // dp3
}

// Gamma -> Linear. Piecewise OETF.
float3 SrgbGammaToLinear(float3 f3SrgbGammaValue)
{
	float3 x = saturate(f3SrgbGammaValue);
	return (x <= 0.04045f) ? (x / 12.92f) : (pow((x + 0.055f) / 1.055f, 2.4f));
}

// Linear -> Gamma. Piecewise OETF.
float3 SrgbLinearToGamma(float3 f3Linear)
{
	float3 x = saturate(f3Linear);
	return (x <= 0.0031308f) ? (x * 12.92f) : (1.055f * pow(x, (1.0f / 2.4f))) - 0.055f;
}

#endif // End of LUX_COMMON_FXC_H_