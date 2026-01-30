//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	25.03.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
// From common_lightmappedgeneric_fxc.h on ValveSoftware/source-sdk-2013:
// "misyl:
// Bicubic lightmap code lovingly taken and adapted from Godot
// ( https://github.com/godotengine/godot/pull/89919 )
// Licensed under MIT."
//
//==========================================================================//

#ifndef LUX_COMMON_BICUBIC_H_
#define LUX_COMMON_BICUBIC_H_

float w0(float a)
{
	return (1.0 / 6.0) * (a * (a * (-a + 3.0) - 3.0) + 1.0);
}

float w1(float a)
{
	return (1.0 / 6.0) * (a * a * (3.0 * a - 6.0) + 4.0);
}

float w2(float a)
{
	return (1.0 / 6.0) * (a * (a * (-3.0 * a + 3.0) + 3.0) + 1.0);
}

float w3(float a)
{
	return (1.0 / 6.0) * (a * a * a);
}

// g0 and g1 are the two amplitude functions
float g0(float a)
{
	return w0(a) + w1(a);
}

float g1(float a)
{
	return w2(a) + w3(a);
}

// h0 and h1 are the two offset functions
float h0(float a)
{
	return -1.0 + w1(a) / (w0(a) + w1(a));
}

float h1(float a)
{
	return 1.0 + w3(a) / (w2(a) + w3(a));
}

#endif // LUX_COMMON_BICUBIC_H_