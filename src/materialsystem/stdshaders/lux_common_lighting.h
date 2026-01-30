//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.02.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_LIGHTING_H_
#define LUX_COMMON_LIGHTING_H_

// Always need NormalMaps
#include "lux_common_normalmap.h"

// LightWarp if desired
#include "lux_common_lightwarp.h"

//  Source gives us at maximum four LightSources,
//	Their Colors and Positions are compressed into this 6x4 Block
//	Since they were limited to 32 Registers, this was necessary at their Time.
//	Using the Compressed Registers will cause some MOV Instructions on the Shader,
//	it also makes the Code more spaghettified, so they are a bit inoptimal. 
//	I wish we could send uncompressed lighting data to the Shader.
// 
//       x		y		z      w
//    +------+------+------+------+
//    |       L0.rgb       |      |
//    +------+------+------+      |
//    |       L0.pos       |  L3  |
//    +------+------+------+  rgb |
//    |       L1.rgb       |      |
//    +------+------+------+------+
//    |       L1.pos       |      |
//    +------+------+------+      |
//    |       L2.rgb       |  L3  |
//    +------+------+------+  pos |
//    |       L2.pos       |      |
//    +------+------+------+------+


// Not actually used anywhere
#define cOverbright 2.0f
#define cOOOverbright (1.0f / cOverbright)
#define LIGHTTYPE_NONE			0
#define LIGHTTYPE_SPOT			1
#define LIGHTTYPE_POINT			2
#define LIGHTTYPE_DIRECTIONAL	3

//==========================================================================//
// PixelShader *Float* Constant Registers
//==========================================================================//
// Projected Textures can't use this Lighting Code
// FIXME: Move actual Lighting Functions to their own Header then have Helper Headers for Functions like ComputeDirectDiffuseLight
#if PROJTEX

#define g_bHalfLambert false

// by Defining this, it allows Shaders to move the Registers to something else
#elif (!defined(MOVED_REGISTERS_LIGHTING))
	const float3 cAmbientCube[6]				: register(LUX_PS_FLOAT_AMBIENTCUBE);
	PixelShaderLightInfo	cLightInfo[4]		: register(LUX_PS_FLOAT_LIGHTDATA);

#define g_bHalfLambert	Bools[LUX_PS_BOOL_HALFLAMBERT]
#endif

//==========================================================================//
// Function for computing Diffuse Lighting from a LightSource
//==========================================================================//
float3 ComputeDirectDiffuseLight(float3 f3LightDir, float3 f3LightColor, float3 f3WorldNormal)
{
	float f1NdL = dot(f3WorldNormal, -f3LightDir); // Potentially negative dot

	// bHalfLambert is Bools[0], defined in lux_common_ps_fxc.h
	if(g_bHalfLambert)
	{
		// Zero to One Range
		// [-1.0f to +1.0f] -> [-0.5f to +0.5f] -> [0.0f to +1.0f]
		f1NdL = saturate(f1NdL * 0.5f + 0.5f);

		// Square Curve
#if LIGHTWARPTEXTURE
		if (!g_bLightWarpTexture)
			f1NdL *= f1NdL;
#endif
	}
	else
	{
		// More realistic Attenuation Term
		f1NdL = saturate(f1NdL);
	}

	float3 f3LightAttenuation;

#if LIGHTWARPTEXTURE
	if (g_bLightWarpTexture)
	{
		// Shirodkxtro2: This *must* be tex1Dlod, if we use the Integer Register for Lighting Loops.
		// 
		// If this was tex1Dlod, the Compiler would error with 'blabla gradient instruction are illegal in conditional statements'
		// Now this usually wouldn't be a Problem because it could just unroll the Loop.
		// However in case of Integer Registers *that's impossible* they don't have a known Size during Compile.
		// These Textures shouldn't be using MipMaps anyways..
		f3LightAttenuation = 2.0f * tex1Dlod(Sampler_LightWarpTexture, float4(f1NdL,0,0,0)).xyz;

		// The 2.0f here could be precomputed into f3LightColor or f1Attenuation
		// First one in the c++ Portion of the Code and the Second one on the Vertex Shader
	}
	else
#endif
	{
		f3LightAttenuation = (float3)f1NdL;
	}

	return f3LightAttenuation * f3LightColor;
}

// Flashlight can't use anything from below.

#if (!defined(BRUSH_SPECULAR) && !PROJTEX)
//==========================================================================//
// Easy to use Function to compute Direct Diffuse Lighting on Models
//==========================================================================//
float3 ComputeDirectDiffuse(float3 f3WorldPos, float3 f3NormalWS, float4 f4LightAtten)
{
	float3 f3Diffuse = 0.0f;

	// I hope that the Compiler is smart enough optimises this.
	// Otherwise it will do two loops for Diffuse and Specular Paths
	// Oof! Unrolling to avoid a Warning
	[unroll]
	for (int n = 0; n < NUM_LIGHTS_COMBO; n++)
	{
		float3 f3LightColor;
		float3 f3LightDir;
		if (n == 3)
		{
			f3LightColor = float3(cLightInfo[0].color.w, cLightInfo[0].pos.w, cLightInfo[1].color.w) * f4LightAtten[n];
			f3LightDir = normalize(f3WorldPos - float3(cLightInfo[1].pos.w, cLightInfo[2].color.w, cLightInfo[2].pos.w));
		}
		else
		{
			f3LightColor = cLightInfo[n].color.xyz * f4LightAtten[n];
			f3LightDir = normalize(f3WorldPos - cLightInfo[n].pos.xyz);
		}

		f3Diffuse += ComputeDirectDiffuseLight(f3LightDir, f3LightColor, f3NormalWS);
	}

	return f3Diffuse;
}

//==========================================================================//
// Easy to use Function to compute Indirect Diffuse Lighting on Models
//==========================================================================//

// Taken from Stock common_vertexlitgeneric Header.
float3 ComputeAmbientCube(const float3 worldNormal, const float3 cAmbientCube[6])
{
	float3 linearColor, nSquared = worldNormal * worldNormal;
	float3 isNegative = (worldNormal >= 0.0) ? 0 : nSquared;
	float3 isPositive = (worldNormal >= 0.0) ? nSquared : 0;
	linearColor = isPositive.x * cAmbientCube[0] + isNegative.x * cAmbientCube[1] +
		isPositive.y * cAmbientCube[2] + isNegative.y * cAmbientCube[3] +
		isPositive.z * cAmbientCube[4] + isNegative.z * cAmbientCube[5];
	return linearColor;
}

float3 ComputeIndirectDiffuse(float3 f3NormalWS)
{
	// Do it regardless, all Maps should have Ambient Cubes.
	// We ignore HLMV where they don't work regardless..
	// If HLMV++ fixes that, it'll use them too!
	// if (bComputeAmbientCube)
	// cAmbientCube isn't declared in this File.. oops?
	return ComputeAmbientCube(f3NormalWS, cAmbientCube);
}
#endif // End of !PROJTEX
#endif // End of LUX_COMMON_LIGHTING_H_