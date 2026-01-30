//===================== File of the LUX Shader Project =====================//
//
//	Original D. :	24.05.2025 DMY
//	Initial D.	:	06.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Infected Shader Constants
//
//==========================================================================//

#ifndef LUX_INFECTED_PS30_H_
#define LUX_INFECTED_PS30_H_

// Register Map for this Shader
#include "lux_infected_registermap.h"

// Don't want this
#define NOLIGHTWARP

// Remapping this
#define MOVED_REGISTERS_LIGHTING
#define MOVED_REGISTERS_PHONG
#define MOVED_SAMPLERS_NORMALMAP

//==========================================================================//
// Samplers
//==========================================================================//

// s0 - $BaseTexture defined in lux_common_ps_fxc.h
sampler Sampler_NormalMap			: register(s1); // Moved Sampler
sampler Sampler_WoundCutOutTexture	: register(s2);
sampler Sampler_GradientTexture		: register(s3);
sampler Sampler_DetailTexture		: register(s4);
sampler Sampler_BurnDetailTexture	: register(s5);

//==========================================================================//
// *Float* Constants
//==========================================================================//
const float4	cRandomisations				: register(INFECTED_RANDOMISATION);
#define			g_f1RandomisationOffset1	(cRandomisations.x)
#define			g_f1RandomisationOffset2	(cRandomisations.y)
#define			g_f2UVRandomisation			(cRandomisations.zw)

// c1
// Existing Register Definition
const float4	cBloodControls1			: register(INFECTED_BLOODCONTROLS1);
#define			g_f3BloodColor			(cBloodControls1.rgb)
#define			g_f1BloodPhongExponent	(cBloodControls1.w)

// c2 used to be shadow tweaks but we can move these down now!!

const float4	cBloodControls2			: register(INFECTED_BLOODCONTROLS2);
#define			g_f1BloodMaskMin		(cBloodControls2.x)
#define			g_f1BloodMaskMax		(cBloodControls2.y)
#define			g_f1BloodPhongBoost		(cBloodControls2.z)
#define			g_f1SkinPhongExponent	(cBloodControls2.w)

const float4	cPhongControls1			: register(INFECTED_PHONGCONTROLS1);
#define			g_f3PhongTint			(cPhongControls1.rgb)
#define			g_f1PhongBoost			(cPhongControls1.w)

// Need consistent Names with lux_common_phong_data.h
const float4	cPhongControls2				: register(INFECTED_PHONGCONTROLS2);
#define			g_f3PhongFresnelRanges		(cPhongControls2.xyz)
#define			g_f1DefaultPhongExponent	(cPhongControls2.w)

const float4	cEyeGlowControls		: register(INFECTED_EYEGLOWCONTROLS);
#define			g_f3EyeGlowColor		(cEyeGlowControls.rgb)
#define			g_f1DetailPhongExponent	(cEyeGlowControls.w)

const float4	cDetailControls			: register(INFECTED_DETAILCONTROLS);
#define			g_f3DetailTint			(cDetailControls.rgb)
#define			g_f1DetailBlendFactor	(cDetailControls.w)

const float4	cCutOutControls1		: register(INFECTED_CUTOUTCONTROLS1);
#define			g_f2Ellipsoid1_UVScale	(cCutOutControls1.xy)
#define			g_f2Ellipsoid2_UVScale	(cCutOutControls1.zw)

const float4	cCutOutControls2		: register(INFECTED_CUTOUTCONTROLS2);
#define			g_f1BurnStrength		(cCutOutControls2.x)
#define			g_f1CutOutTextureBias	(cCutOutControls2.y)
#define			g_f1Ellipsoid1_UVOffset	(cCutOutControls2.z)
#define			g_f1Ellipsoid2_UVOffset	(cCutOutControls2.w)


// Need consistent Names with lux_common_phong_data.h
const float4	cCutOutControls3				: register(INFECTED_CUTOUTCONTROLS3);
#define			g_f1Ellipsoid1_UVMappingScale	(cCutOutControls3.x)
#define			g_f1Ellipsoid2_UVMappingScale	(cCutOutControls3.y)
// .zw empty 

// Moved these Registers
const float3 cAmbientCube[6]				: register(INFECTED_AMBIENTCUBE);
//			 cAmbientCube[1]				: register(c14);
//			 cAmbientCube[2]				: register(c15);
//			 cAmbientCube[3]				: register(c16);
//			 cAmbientCube[4]				: register(c17);
//			 cAmbientCube[5]				: register(c18);

PixelShaderLightInfo	cLightInfo[4]		: register(INFECTED_LIGHTINFO); // "4th light spread across w's"
//						cLightInfo[1]		: register(c21);
//						cLightInfo[2]		: register(c22);
//						cLightInfo[2]		: register(c23);
//						cLightInfo[3]		: register(c24);
//						cLightInfo[3]		: register(c25);
//						cLightInfo[4]		: register(c26);

// Yet unhandled Remaps:
static const float g_f1PhongExponentFactorParam = 0.0f;
static const float g_f1AlbedoTintBoost = 1.0f;
static const float g_f1RimLightExponent = 1.0f;
static const float g_f1RimLightBoost = 1.0f;
static const float g_f1PhongExponentParam = 1.0f;
static const float g_f1PhongInvertMask = 0.0f;

//==========================================================================//
// *Boolean* Constants
//==========================================================================//

// All of these are Remaps

#if !PROJTEX
#define g_bHalfLambert	Bools[0]
#endif

// Disabling all of these instead of moving the bools
static const bool g_bHasBaseAlphaPhongMask			= false;	
static const bool g_bHasPhongAlbedoTint				= false;	
static const bool g_bUseFlatNormal					= false;	
static const bool g_bHasRimLightMask				= false;
static const bool g_bHasBasemapLuminancePhongMask	= false;
static const bool g_bHasPhongExponentTextureMask	= false;
static const bool g_bHasRimLight					= false;
static const bool g_bHasPhongWarpTexture			= false;

//==========================================================================//
// Functions
//==========================================================================//

// The Texture for this is called $BurnDetailTexture
// In TF2 they do Burns on Character Models using BlendMode 6
// So I'm simply assuming they adapted that from Left 4 Dead 2
// TCOMBINE_RGB_ADDITIVE_SELFILLUM_THRESHOLD_FADE
float3 BurnFactor(float3 f3Detail)
{
	/*
	// "fade in an unusual way - instead of fading out color, remap an increasing band of it from 0..1"
	// This is done the traditional way since the $BurnDetailTexture doesn't have a Tint
	float f = f1BurnStrength - 0.5;
	float fMult = (f >= 0) ? 1.0 / f1BurnStrength : 4 * f1BurnStrength;
	float fAdd = (f >= 0) ? 1.0 - fMult : -0.5*fMult;

	return saturate(f3Detail * fMult + fAdd);
	*/

	// Looks Whack
	// BlendMode 5 it is
	return f3Detail * g_f1BurnStrength;
}

// Used for blending out the CutOut Texture at its Borders due to infinitely stretching Border Pixels
float EdgeBlend(float2 f2TexCoord, float f1BorderWidth)
{
	// Distance to nearest Border, which is 0.0f or 1.0f
	// ( We expect saturated UV's )
	float2 f2EdgeDist = min(f2TexCoord, 1.0 - f2TexCoord);

	// Minimum Distance to *any* Border
	float f1Dist = min(f2EdgeDist.x, f2EdgeDist.y);

	// This will be 0.0f at the Edge and 1.0f towards the BorderWidth
	return pow(saturate(f1Dist / f1BorderWidth), 5.0f);
}

#endif