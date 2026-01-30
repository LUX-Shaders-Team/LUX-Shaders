//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.02.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_PHONG_PROJTEX
#define LUX_COMMON_PHONG_PROJTEX

#include "lux_common_phong_data.h"

// Has Shadow Filters, constant register and everything else we need
#include "lux_common_flashlight.h"

// We use the Lighting Functions from here. ( They have been refactored to work with this since LUX v1.30 )
#include "lux_common_phong.h"

//==========================================================================//
// Direct Specular Projected Texture Function
// Requires Direct Diffuse to be passed manually!! Required when Diffuse Lighting is messed with separately!
//==========================================================================//
float3 ComputeProjectedTextureSpecular(Phong_Data_t ph, float3 f3WorldPos, float3 f3NormalWS, float3 f3Reflect, float3 f3DirectDiffuse)
{
	// This is technically duplicate Code.
	// I'm hoping the Compiler is smart enough to realise, that we compute this in ComputeProjectedTextureDiffuse too
	float3 f3LightDir = normalize(f3WorldPos - g_f3ProjTexPos);

	// ShiroDkxtro2: We have to use the Diffuse Light as our Shadowing Factor
	// Which already has NdL computed baked in ( Great, another Cookie Reference )
	float f1NdL = 1.0f;
	float3 f3DirectSpecular = ComputeDirectSpecularLight(f1NdL, f3Reflect, f3LightDir, f3DirectDiffuse, ph.f1PhongExponent, ph.f1PhongFresnel);

	// Apply Fresnel after the Sum, only if not already done via PhongWarpTexture
	if (!g_bHasPhongWarpTexture)
	{
		f3DirectSpecular *= ph.f1PhongFresnel;
	}

	// Stock-Consistency:
	// PhongMask and PhongBoost applied only to Regular Lights
	f3DirectSpecular *= ph.f1AlphaMask * g_f1PhongBoost;

	// Rimlight would usually be here

	// Tint / $PhongAlbedoTint / ..
	f3DirectSpecular *= ph.f3PhongModulation;

	return f3DirectSpecular;
}

//==========================================================================//
// Direct Diffuse and Direct Specular Projected Texture Function
//==========================================================================//
float3 ComputeProjectedTextureDiffuseAndSpecular(Phong_Data_t ph, float3 f3WorldPos, float3 f3NormalWS, float3 f3Reflect, float3 f3BaseTexture, const bool bDoShadows = false)
{
	// This is technically duplicate Code.
	// I'm hoping the Compiler is smart enough to realise, that we compute this in ComputeProjectedTextureDiffuse too
	float3 f3LightDir = normalize(g_f3ProjTexPos - f3WorldPos);

	float3 f3DirectDiffuse = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, bDoShadows);
	
	// ShiroDkxtro2: We have to use the Diffuse Light as our Shadowing Factor
	// Which already has NdL computed baked in ( Great, another Cookie Reference )
	float f1NdL = 1.0f;
	float3 f3DirectSpecular = ComputeDirectSpecularLight(f1NdL, f3Reflect, f3LightDir, f3DirectDiffuse, ph.f1PhongExponent, ph.f1PhongFresnel);
	
	// Apply Fresnel after the Sum, only if not already done via PhongWarpTexture
	if (!g_bHasPhongWarpTexture)
	{
		f3DirectSpecular *= ph.f1PhongFresnel;
	}

	// Stock-Consistency:
	// PhongMask and PhongBoost applied only to Regular Lights
	f3DirectSpecular *= ph.f1AlphaMask * g_f1PhongBoost;

	// Rimlight would usually be here

	// Tint / $PhongAlbedoTint / ..
	f3DirectSpecular *= ph.f3PhongModulation;

	return f3DirectDiffuse * f3BaseTexture + f3DirectSpecular;
}
#endif // End of LUX_COMMON_PHONG_PROJTEX