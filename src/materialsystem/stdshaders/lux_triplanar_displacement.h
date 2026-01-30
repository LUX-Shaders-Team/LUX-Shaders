//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	29.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#include "lux_registermap_hlsl.h"
#include "lux_registermap_ps.h"

// Move Samplers
#define MOVED_SAMPLERS_ENVMAP
#define MOVED_SAMPLERS_DETAIL

// c32	- Tint1 + AlphaVar
// c33	- DetailBlendMode
// c34	- DetailTint + BlendFactor
// c35
// c36
// c37	- EnvMap Tint + LightScale
// c38	- EnvMap Saturation + Contrast
// c39	- EnvMap Controls
// c40	- EnvMap Fresnel
// c41	- EnvMap Origin
// c42	- EnvMap OBB1
// c43	- EnvMap OBB2
// c44	- EnvMap OBB3
// ..

// Samplers
// ================================= //

// Commented are from other Headers ( lux_common_ps_fxc.h, lux_common_lightmapped.h, etc )
// sampler Sampler_BaseTexture : register(s0); 
sampler Sampler_BaseTexture2			: register(s1);
// sampler Sampler_NormalMap : register(s2); lux_common_ps_fxc.h
sampler Sampler_NormalMap2				: register(s3);
sampler Sampler_DetailTexture			: register(s4);
sampler Sampler_DetailTexture2			: register(s5);
sampler Sampler_EnvMapMask				: register(s6);
sampler Sampler_EnvMapMask2				: register(s7);
sampler Sampler_BlendModulate			: register(s8);

#if !PROJTEX
// sampler Sampler_Lightmap				: register(s11);
sampler Sampler_EnvironmentMap			: register(s14);
#endif