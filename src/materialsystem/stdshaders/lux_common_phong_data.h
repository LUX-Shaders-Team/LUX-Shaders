//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	25.08.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_PHONG_DATA
#define LUX_COMMON_PHONG_DATA

// Need this for Luminance Weights
#include "lux_common_ps_fxc.h"

//==========================================================================//
// PixelShader *Float* Constant Registers
//==========================================================================//
// 38-41 are used by PCC. SO we avoid those for Brush Phong by using 42+
// 
// by Defining this, it allows Shaders to move the Registers to something else
#if !defined(MOVED_REGISTERS_PHONG)
	const float4	cPhongTint_InvertMask			: register(LUX_PS_FLOAT_PHONG_TINT);
	#define			g_f3PhongTint					(cPhongTint_InvertMask.rgb)
	#define			g_f1PhongInvertMask				(cPhongTint_InvertMask.w)
	const float4	cPhongFresnelRanges_Exponent	: register(LUX_PS_FLOAT_PHONG_FRESNEL);
	// FIXME: Split Fresnel Ranges into Min, Low, High or something like that
	#define			g_f3PhongFresnelRanges			(cPhongFresnelRanges_Exponent.xyz)
	#define			g_f1PhongExponentFactorParam	(cPhongFresnelRanges_Exponent.w)
	const float4	cPhongControls					: register(LUX_PS_FLOAT_PHONG_CONTROLS);
	#define			g_f1AlbedoTintBoost				(cPhongControls.x)				
	#define			g_f1RimLightExponent			(cPhongControls.y)			
	#define			g_f1RimLightBoost				(cPhongControls.z)			
	#define			g_f1PhongExponentParam			(cPhongControls.w)	
	const float4	cMinLight_PhongBoost			: register(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST);
	#define			g_f3MinimumLight				(cMinLight_PhongBoost.rgb)
	#define			g_f1PhongBoost					(cMinLight_PhongBoost.w)
#endif

//==========================================================================//
//	Declaring BOOLEAN PixelShader Constant Registers
//==========================================================================//

// Layout here is not very specific, has some PhongExponentTexture specific Things right in the Middle
#if !defined(MOVED_REGISTERS_PHONG)
#define			g_bHasBaseAlphaPhongMask		Bools[LUX_PS_BOOL_PHONG_BASEMAPALPHAMASK]
#define			g_bHasPhongAlbedoTint			Bools[LUX_PS_BOOL_PHONG_ALBEDOTINT]
#define			g_bUseFlatNormal				Bools[LUX_PS_BOOL_PHONG_FLATNORMAL]
#define			g_bHasRimLightMask				Bools[LUX_PS_BOOL_PHONG_RIMLIGHTMASK]
#define			g_bHasBasemapLuminancePhongMask	Bools[LUX_PS_BOOL_PHONG_BASEMAPLUMINANCEMASK]
#define			g_bHasPhongExponentTextureMask	Bools[LUX_PS_BOOL_PHONG_EXPONENTTEXTUREMASK]
#define			g_bHasRimLight					Bools[LUX_PS_BOOL_PHONG_RIMLIGHT]
#define			g_bHasPhongWarpTexture			Bools[LUX_PS_BOOL_PHONG_WARPTEXTURE]
#endif

//==========================================================================//
//	Declaring Samplers. We only have 16 on SM3.0. Ranging from 0-15
//	Although s15 wants to be a gamma-lookup table, limiting us to 15/16
//==========================================================================//

#if !defined(MOVED_SAMPLERS_PHONG)
// Always! Not going to pull a L4D2 disabling this..
sampler Sampler_PhongWarpTexture : register(s7);

// Put under an ifdef in case a Shader needs the Register for something else or isn't supposed to have a Texture
// Not using #if, STATIC name variable ( VLG uses PHONGEXPONENTTEXTURE, VLG Flashlight uses BUMPMAPPED, and Teeth uses LIGHTING_MODE )
#if defined(PHONGEXPONENTTEXTURE)
sampler Sampler_PhongExpTexture : register(s8);
#endif
#endif

//==========================================================================//
// Phong Data Struct. This compacts LUX Phong Code quite a bit..
// Has all necessary Variables used for Phong Calculations
//==========================================================================//
struct Phong_Data_t
{
	float4 f4PhongExponentTexture;
	// Phong Mask is exposed as some Shaders may not even have BaseAlpha and NormalAlpha Phong Masking
	// Gets overwritten when using bHasPhongExponentTextureMask
	float f1AlphaMask;
	float3 f3PhongModulation;
	float f1PhongExponent;
	float f1RimLightMask;

	float f1RimFresnel;
	float f1PhongFresnel;
};

// HLSL has forced my hand.
// No structure constructors! This is cursed!
Phong_Data_t Phong_Data_FakeConstructor()
{
	Phong_Data_t ph;
	ph.f4PhongExponentTexture = 1.0f;
	ph.f1AlphaMask = 1.0f;
	ph.f3PhongModulation = 1.0f;
	ph.f1PhongExponent = 1.0f;
	ph.f1RimLightMask = 1.0f;
	ph.f1RimFresnel = 1.0f;
	ph.f1PhongFresnel = 1.0f;
	return ph;
}

// Handy Setup Function
// ( Such as the Teeth Shader that doesn't use Normal Alpha for it )
void SetupPhongData(inout Phong_Data_t ph, float3 f3BaseTexture)
{
	// By Default we should receive $PhongExponent on f1PhongExponent
	// If we have a $PhongExponentFactor ( and a Texture )
	// $PhongExponent should be set to 1
	// Vice Versa if $PhongExponent has a value, $PhongExponentFactor should be 0
	// That way when we are using a PhongExponentTexture, we get..
	// [0..1] * $PhongExponentFactor (149) + $PhongExponent (1)
	ph.f1PhongExponent = g_f1PhongExponentParam;

	// Color * Mask
	ph.f3PhongModulation = g_f3PhongTint;

	// Handle overrides for $PhongExponentTexture first
	#if defined(PHONGEXPONENTTEXTURE)
		// Override PhongExponent
		ph.f1PhongExponent += ph.f4PhongExponentTexture.x * g_f1PhongExponentFactorParam;

		// Presumably the Compiler will automatically optimise these
		// In cases like the Teeth Shader where this would be, non-existant
		// This allows for the use of both $PhongAlbedoTint and $BaseMapLuminancePhongMask (f3SpecularTint) at the same time.

		// Neat trick here. Since ph.f4PhongExponentTexture.y is 1.0f by default, the compiler will strip it away,
		// That means we support $PhongAlbedoTint without $PhongExponentTexture!
//		if(bHasPhongAlbedoTint)

		// RimLightMask 
		// if $RimMask, use the Alpha-Channel of the $PhongExponentTexture
		// Default Value is 1.0f so it will just be nuked by the Compiler if not using PHONGEXPONENTTEXTURE
		// Stock Consistency : Not happening under the Flashlight
#if !PROJTEX
		if(g_bHasRimLightMask)
			ph.f1RimLightMask = ph.f4PhongExponentTexture.w;
#endif

		// If $PhongExponentTextureMask is set, use the blue channel as PhongMask so that Base and Normal can be used for other stuff.
		if(g_bHasPhongExponentTextureMask)
			ph.f1AlphaMask = ph.f4PhongExponentTexture.b;
//		else
//	 		ph.f1AlphaMask = ph.f1AlphaMask; // AlphaMask will stay AlphaMask
	#endif

	// No need for a conditional Statement
	// Just do abs(x - Mask) with x being 1 or 0
	ph.f1AlphaMask = abs(g_f1PhongInvertMask - ph.f1AlphaMask);

	// This should be correct, no AlbedoTint at Low Values and Tint at High Values
	if (g_bHasPhongAlbedoTint)
		ph.f3PhongModulation *= lerp(1.0f, f3BaseTexture * g_f1AlbedoTintBoost, ph.f4PhongExponentTexture.y);

	// $PhongAlbedoTint was already applied. We can still apply luminance though.
	// BUGBUG: There used to be a Parameter Combination that allowed you to mask the EnvMap using BaseMap Luminance
	// We are not reproducing that behaviour with the expectation that it was an oversight.
	// And the hopes that no one used that intentionally.
	// f3LumCoefficients is part of lux_common_ps_fxc.h - Which isn't included here.. Oops?
	if (g_bHasBasemapLuminancePhongMask)
		ph.f1AlphaMask *= PerceptualLuminance(f3BaseTexture);
}

void SetupPhongFresnel(inout Phong_Data_t ph, float f1UnsaturatedNdotV)
{
	// Stock-Consistency
	float Fresnel = saturate(1.0f - f1UnsaturatedNdotV);

	// "note: vRanges is now encoded as ((mid-min)*2, mid, (max-mid)*2) to optimize math"
	float FresnelRanges = Fresnel * Fresnel - 0.5f;
	ph.f1PhongFresnel = g_f3PhongFresnelRanges.y + (FresnelRanges >= 0.0f ? g_f3PhongFresnelRanges.z : g_f3PhongFresnelRanges.x) * FresnelRanges;

	// Double square for RimLighting
	float Fresnel4 = Fresnel * Fresnel;
	Fresnel4 = Fresnel4 * Fresnel4;
	ph.f1RimFresnel = Fresnel4;
}
#endif // End of LUX_COMMON_PHONG_DATA