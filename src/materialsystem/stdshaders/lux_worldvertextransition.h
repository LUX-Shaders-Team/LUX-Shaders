//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	30.07.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Constant Registers and Samplers of lux_WorldVertexTransition Files
//
//==========================================================================//

#include "lux_registermap_hlsl.h"
#include "lux_registermap_ps.h"

// We are remapping all of this.
// These defines make the Helper Headers not declare any Constants
#define MOVED_REGISTERS_DETAIL
#define MOVED_REGISTERS_SELFILLUM
#define MOVED_REGISTERS_ENVMAP
#define MOVED_REGISTERS_PHONG

// Only need to move $EnvMapMask Sampler
#define MOVED_SAMPLERS_ENVMAP
#define MOVED_SAMPLERS_PHONG
#define MOVED_SAMPLERS_LIGHTWARP
#define MOVED_SAMPLERS_SELFILLUM
#define MOVED_SAMPLERS_DETAIL

// c32	- Tint1 + AlphaVar
// c33	- DetailBlendMode
// c34	- DetailTint + BlendFactor
// c35	- SelfIllum Tint + MaskScale
// c36	- SelfIllum Fresnel
// c37	- EnvMap Tint + LightScale
// c38	- EnvMap Saturation + Contrast
// c39	- EnvMap Controls
// c40	- EnvMap Fresnel
// c41	- EnvMap Origin
// c42	- EnvMap OBB1
// c43	- EnvMap OBB2
// c44	- EnvMap OBB3
// c45	- Phong Tint + InvertPhongMask
// c46	- Phong Fresnel1 + ExponentFactor1
// c47	- Phong Controls
// c48	- MinLight PhongBoost
// c49	- Phong Fresnel2 + ExponentFactor2


// lux_common_ps_fxc.h
/*
const float4	cBaseTextureTint_Factor : register(c32);
#define			g_f3BaseTextureTint				(cBaseTextureTint_Factor.xyz)
#define			g_f1BlendTintFactor				(cBaseTextureTint_Factor.w)
*/

// Detail Texture
// ================================= //
const float4	cDetailTint_BlendFactor			: register(LUX_PS_FLOAT_DETAIL_BLENDMODE);
#define			g_f3DetailTextureTint				(cDetailTint_BlendFactor.rgb)
#define			g_f1DetailBlendFactor				(cDetailTint_BlendFactor.w)

const float4	cDetailFactors					: register(LUX_PS_FLOAT_DETAIL_FACTORS);
#define			g_f1DetailBlendMode					(cDetailFactors.x)

// SelfIllum
// ================================= //
const float4	cSelfIllumTint_Factor			: register(LUX_PS_FLOAT_SELFILLUM_FACTORS);
#define			g_f3SelfIllumTint					(cSelfIllumTint_Factor.rgb)
#define			g_f1SelfIllumMaskFactor				(cSelfIllumTint_Factor.w)

const float4	cSelfIllumFresnel				: register(LUX_PS_FLOAT_SELFILLUM_FRESNEL);
#define			g_f1SelfIllumFresnelScale			(cSelfIllumFresnel.x)
#define			g_f1SelfIllumFresnelBias			(cSelfIllumFresnel.y)
#define			g_f1SelfIllumFresnelExponent		(cSelfIllumFresnel.z)

// EnvMap
// ================================= //
const float4	cEnvMapTint_LightScale		: register(LUX_PS_FLOAT_ENVMAP_TINT); 
#define			g_f3EnvMapTint					(cEnvMapTint_LightScale.rgb)
#define			g_f1EnvMapLightScale			(cEnvMapTint_LightScale.w)

const float4	cEnvMapSaturation_Contrast	: register(LUX_PS_FLOAT_ENVMAP_FACTORS);
#define			g_f3EnvMapSaturation			(cEnvMapSaturation_Contrast.xyz)
#define			g_f1EnvMapContrast				(cEnvMapSaturation_Contrast.w)

const float4	cEnvMapMaskControls			: register(LUX_PS_FLOAT_ENVMAP_CONTROLS);
#define			g_f1LerpBaseAlpha				(cEnvMapMaskControls.x)
#define			g_f1LerpNormalAlpha				(cEnvMapMaskControls.y)
#define			g_f1EnvMapMaskFlip				(cEnvMapMaskControls.z) // 1 when flipped and 0 when not flipped.
#define			g_f1BaseTextureNoEnvMap			(cEnvMapMaskControls.w) // 0 when no EnvMap, 1 when EnvMap

const float4	cEnvMapFresnel				: register(LUX_PS_FLOAT_ENVMAP_FRESNEL);
#define			g_f1EnvMapFresnelScale			(cEnvMapFresnel.x)
#define			g_f1EnvMapFresnelBias			(cEnvMapFresnel.y)
#define			g_f1EnvMapFresnelExponent		(cEnvMapFresnel.z)
#define			g_f1BaseTexture2NoEnvMap		(cEnvMapFresnel.w)

const float3	g_f3CubeMapPos				: register(LUX_PS_FLOAT_ENVMAP_POSITION);
const float4x3	g_f4x3CorrectionMatrix		: register(LUX_PS_FLOAT_ENVMAP_MATRIX); // c43...c44

// Phong
// ================================= //
const float4	cPhongTint_InvertMask		: register(LUX_PS_FLOAT_PHONG_TINT);
#define			g_f3PhongTint					(cPhongTint_InvertMask.rgb)
#define			g_f1PhongInvertMask				(cPhongTint_InvertMask.w)

const float4	cPhongFresnelRanges_Exponent	: register(LUX_PS_FLOAT_PHONG_FRESNEL);
#define			g_f3PhongFresnelRanges				(cPhongFresnelRanges_Exponent.xyz)
#define			g_f1PhongExponentFactorParam		(cPhongFresnelRanges_Exponent.w)

const float4	cPhongControls					: register(LUX_PS_FLOAT_PHONG_CONTROLS);
#define			g_f1AlbedoTintBoost					(cPhongControls.x)
#define			g_f1PhongExponentParam				(cPhongControls.y)
#define			g_f1PhongExponentParam2				(cPhongControls.z)

const float4	cMinimumLight_PhongBoost		: register(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST);
#define			g_bBaseTexture1NoPhong				(cMinimumLight_PhongBoost.x)
#define			g_bBaseTexture2NoPhong				(cMinimumLight_PhongBoost.y)
#define			g_f1PhongBoost						(cMinimumLight_PhongBoost.w)

const float4	cPhongFresnelRanges_Exponent2	: register(c49);
#define			g_f3PhongFresnelRanges2				(cPhongFresnelRanges_Exponent2.xyz)
#define			g_f1PhongExponentFactorParam2		(cPhongFresnelRanges_Exponent2.w)

#define			g_bHasSelfIllumFresnel			Bools[12]

// Samplers
// ================================= //

// Commented are from other Headers ( lux_common_ps_fxc.h, lux_common_lightmapped.h, etc )
// sampler Sampler_BaseTexture : register(s0); 
sampler Sampler_BaseTexture2			: register(s1);
// sampler Sampler_NormalMap : register(s2); lux_common_ps_fxc.h
sampler Sampler_NormalMap2				: register(s3);
sampler Sampler_DetailTexture			: register(s4);
sampler Sampler_DetailTexture2			: register(s5);
sampler Sampler_PhongExpTexture			: register(s6);
sampler Sampler_PhongExpTexture2		: register(s7);
sampler Sampler_BlendModulate			: register(s8);

#if !PROJTEX
sampler Sampler_EnvMapMask				: register(s9);
sampler Sampler_EnvMapMask2				: register(s10);
#endif
// sampler Sampler_Lightmap				: register(s11);

#if (!PROJTEX && !defined(WVT_PHONGWARPTEXTURE))
sampler Sampler_LightWarpTexture		: register(s12);
#else
sampler Sampler_PhongWarpTexture		: register(s12);

// Need this as a Fallback..
#define Sampler_LightWarpTexture (Sampler_PhongWarpTexture)
#endif

#if !PROJTEX
sampler Sampler_SelfIllum				: register(s13);
sampler Sampler_EnvironmentMap			: register(s14);
sampler Sampler_SelfIllum2				: register(s15);
#endif

// lux_common_ps_fxc.h
// #define		g_bHalfLambert						Bools[0]
// #define		g_bLightWarpTexture					Bools[1]
// #define		g_bVertexColor						Bools[2]
#define			g_bHasBaseAlphaPhongMask			Bools[3]
#define			g_bHasPhongAlbedoTint				Bools[4]
#define			g_bUseFlatNormal					Bools[5]
#define			g_bHasRimLightMask					Bools[6]
#define			g_bHasBasemapLuminancePhongMask		Bools[7]
#define			g_bHasPhongExponentTextureMask		Bools[8]
#define			g_bHasRimLight						Bools[9]
#define			g_bHasPhongWarpTexture				Bools[10]
#define			g_bHasBlendModulateTransparency		Bools[11]
// #define		g_bHasSelfIllumFresnel				Bools[12] // Only when Phong defined.. lux_common_selfillum.h 
// #define		g_bHasEnvMapFresnel					Bools[12] // Only when defined.. lux_common_envmap.h
// #define		g_bHeightFog						Bools[13] // lux_common_ps_fxc.h
// #define		g_bRadialFog						Bools[14] // ^
// #define		g_bDepthToDestAlpha					Bools[15] // ^