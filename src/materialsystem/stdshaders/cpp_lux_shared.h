//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Every Shader should include this File!
//
//==========================================================================//

#ifndef CPP_LUX_SHARED_H
#define CPP_LUX_SHARED_H

#ifdef _WIN32
#pragma once
#endif

// This File determines which Features to use
// Its a dedicated File instead of C++ Specific Preprocessors
// Since Shaders also need to use them.
#include "lux_common_defines.h"

// Shaders use CBaseVSShader,
// CBaseVSShader inherits CBaseShader ( From ShaderLib )
// CBaseVSShader mainly consists of Utility Functions like those that compress reoccuring Code
// All Shaders need it, since they are using CBaseVSShader!
#include "BaseVSShader.h"

// Used for memset() on Structs
#include <string.h>

// Instead of having ConVar Duplicates scattered across Files,
// We use this Header to pull *all* ConVars from
#include "cpp_convars.h"

// HLSL-Style float-vector Structs (float2, float3, float4)
// This makes the C++ Shaders more consistent with the FXC Files & more readable ( by compacting a lot of Text )
// Replaces mixed Types (float[4], Vector, Vector4D, vector2D)
// Does *not* replace VMatrix.
#include "cpp_floatx.h"

// Modified Version of the CommandBuffer/CommandBuilder with more consistent Naming Scheme and some new Functions
#include "cpp_lux_commandbuilder.h"

// Used for verbose and more readable Console Messages. Shader Debugging via ConColorMsg()
#include "Color.h"

// Macro Register Map
#include "lux_registermap_cpp.h"

//==========================================================================//
// Available DetailBlendmodes
//==========================================================================//
enum DetailBlendModes_t
{
	DETAILBLENDMODE_MOD2X = 0,
	DETAILBLENDMODE_ADDITIVE,
	DETAILBLENDMODE_LERP_BY_DETAILALPHA,
	DETAILBLENDMODE_LERP_BY_BLENDFACTOR,
	DETAILBLENDMODE_LERP_BY_INVBASEALPHA,
	DETAILBLENDMODE_SELFILLUM_ADDITIVE,
	DETAILBLENDMODE_SELFILLUM_THRESHOLDFADE,
	DETAILBLENDMODE_MOD2X_TWOPATTERNS,
	DETAILBLENDMODE_MULTIPLY,
	DETAILBLENDMODE_MULTIPLY_ALPHA,
	DETAILBLENDMODE_SSBUMP_MODULATE,
	DETAILBLENDMODE_SSBUMP_AO,

	NUM_DETAILBLENDMODES
};

// Returns whether the DetailBlendMode is using a sRGB Texture
inline bool IsGammaDetailMode(int nDetailBlendMode)
{
	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	return (nDetailBlendMode > DETAILBLENDMODE_MOD2X && nDetailBlendMode < DETAILBLENDMODE_SSBUMP_MODULATE) ? true : false;		
}

// Returns whether DetailBlendMode is post-Lighting
inline bool IsSelfIllumDetailMode(int nDetailBlendMode)
{
	return (nDetailBlendMode == DETAILBLENDMODE_SELFILLUM_ADDITIVE || nDetailBlendMode == DETAILBLENDMODE_SELFILLUM_THRESHOLDFADE);			
}

//==========================================================================//
// Sampler definitions for all shaders
//==========================================================================//

// For Stock Shaders :
const Sampler_t SAMPLER_BASETEXTURE		= SHADER_SAMPLER0;
const Sampler_t SAMPLER_BASETEXTURE2	= SHADER_SAMPLER1;
const Sampler_t SAMPLER_NORMALMAP		= SHADER_SAMPLER2;
const Sampler_t SAMPLER_NORMALMAP2		= SHADER_SAMPLER3;
const Sampler_t SAMPLER_DETAILTEXTURE	= SHADER_SAMPLER4;
const Sampler_t SAMPLER_ENVMAPMASK		= SHADER_SAMPLER5;	// Not rendered under the flashlight
const Sampler_t SAMPLER_LIGHTWARP		= SHADER_SAMPLER6;	// Not rendered under the flashlight
const Sampler_t SAMPLER_PHONGWARP		= SHADER_SAMPLER7;
const Sampler_t SAMPLER_PHONGEXPONENT	= SHADER_SAMPLER8;
const Sampler_t SAMPLER_ENVMAPMASK2		= SHADER_SAMPLER9;	// Not rendered under the flashlight
const Sampler_t SAMPLER_DETAILTEXTURE2	= SHADER_SAMPLER10;
const Sampler_t SAMPLER_LIGHTMAP		= SHADER_SAMPLER11;
const Sampler_t SAMPLER_BLENDMODULATE	= SHADER_SAMPLER12;
const Sampler_t SAMPLER_SELFILLUM		= SHADER_SAMPLER13; // Not rendered under the flashlight
const Sampler_t SAMPLER_ENVMAPTEXTURE	= SHADER_SAMPLER14; // Not rendered under the flashlight

// CRITICAL NOTE FOR SAMPLER 15 :
// When EnableSRGBWrite() is used, a Linear->Gamma 1D LUT is bound to s15 ( making it unusable )
// ( It can be found on Stock common_ps_fxc.h Line 288 ) - sdk2013sp so might be out of date now
// I'm not sure if this LUT is actually used.
// In Theory, we could manually convert from Linear -> Gamma using approximations like ^2.4
// This remains untested however.
// 
// Also, somehow this does not appear to be an Issue in the Flashlight Pass..
// Use with Caution!
const Sampler_t SAMPLER_SELFILLUM2 = SHADER_SAMPLER15; // Not rendered under the Flashlight

// For the Flashlight :
const Sampler_t SAMPLER_FLASHLIGHTCOOKIE	= SHADER_SAMPLER13;
const Sampler_t SAMPLER_SHADOWDEPTH			= SHADER_SAMPLER14;
const Sampler_t SAMPLER_RANDOMROTATION		= SHADER_SAMPLER15;

//==========================================================================//
// Structs are used when exposing Shader Functions, such as OnDrawElements()
// That way these Functions can be called from other Shaders
// Stock Shaders use this. For Example, UnlitGeneric calls VertexLitGeneric Functions
// 
// This is commonly used for Additional Renderpasses in LUX
// Standalone Functions can't have their own Parameters, that is reserved to Shaders.
// A Shader stores the Index of its own Parameters in *pParams, on a Struct
// Then passes that Struct onward to the Function.
//==========================================================================//

// Previously, LUX used Macros to declare Struct-Members and link the Parameters to them.
// That was pretty gruesome, hard to debug and not very easy to modify.
// It's using Structs now that can be packed into other Structs
// Parameters are linked with Init Functions. Much easier to maintain!

struct Vars_Base_t
{
	Vars_Base_t() { memset(this, 0xFF, sizeof(Vars_Base_t)); }

	int m_nBaseTexture;
	int m_nFrame;
	int m_nBaseTextureTransform;

// Instead of a Macro, just copy this.
/*
	InitVars(BaseTexture, Frame, BaseTextureTransform);
*/
	void InitVars(int Texture, int Frame, int Transform)
	{
		m_nBaseTexture = Texture;
		m_nFrame = Frame;
		m_nBaseTextureTransform = Transform;
	}
};

struct Vars_NormalMap_t
{
	Vars_NormalMap_t() { memset(this, 0xFF, sizeof(Vars_NormalMap_t)); }

	int m_nBumpMap;
	int m_nBumpFrame;
	int m_nBumpTransform;
	int m_nNormalTexture;
	int m_nSSBump;
	int m_nSSBumpMathFix;
	int m_nLightWarpTexture;
	int m_nLightWarpTextureFrame;
	int m_nLightWarpNoBump;

// Instead of a Macro, just copy this.
/*
	InitVars(BumpMap);
*/
	void InitVars(int nBumpMap)
	{
		m_nBumpMap = nBumpMap;
		m_nBumpFrame = nBumpMap + 1;
		m_nBumpTransform = nBumpMap + 2;
		m_nNormalTexture = nBumpMap + 3;
		m_nSSBump = nBumpMap + 4;
		m_nSSBumpMathFix = nBumpMap + 5;
		m_nLightWarpTexture = nBumpMap + 6;
		m_nLightWarpTextureFrame = nBumpMap + 7;
		m_nLightWarpNoBump = nBumpMap + 8;	
	}
};

struct Vars_Detail_t
{
	Vars_Detail_t() { memset(this, 0xFF, sizeof(Vars_Detail_t)); }

	int m_nDetail;
	int m_nDetailFrame;
	int m_nDetailTextureTransform;
	int m_nDetailScale;
	int m_nDetailBlendmode;
	int m_nDetailTint;
	int m_nDetailBlendFactor;

// Instead of a Macro, just copy this.
/*
	InitVars(Detail, DetailFrame, DetailTextureTransform, DetailScale, DetailBlendMode, DetailTint, DetailBlendFactor);
*/
	void InitVars(int Texture, int Frame, int Transform, int Scale, int Mode, int Tint, int Factor)
	{
		m_nDetail = Texture;
		m_nDetailFrame = Frame;
		m_nDetailTextureTransform = Transform;
		m_nDetailScale = Scale;
		m_nDetailBlendmode = Mode;
		m_nDetailTint = Tint;
		m_nDetailBlendFactor = Factor;	
	}
};

struct Vars_EnvMap_t
{
	Vars_EnvMap_t() { memset(this, 0xFF, sizeof(Vars_EnvMap_t)); }

	int m_nEnvMap;
	int m_nEnvMapFrame;
	int m_nEnvMapMaskFlip;
	int m_nEnvMapDisable;
	int m_nEnvMapTint;
	int m_nEnvMapContrast;
	int m_nEnvMapSaturation;
	int m_nEnvMapLightScale;
	int m_nFresnelReflection;
	int m_nEnvMapFresnel;
	int m_nEnvMapFresnelMinMaxExp;

	// Instead of a Macro, just copy this.
	// ShiroDkxtro2: Use the Version below this, it's much shorter.
/*
	InitVars(EnvMap, EnvMapFrame, EnvMapMaskFlip, EnvMapDisable, EnvMapTint,
	EnvMapContrast, EnvMapSaturation, EnvMapLightScale, FresnelReflection, EnvMapFresnel,
	EnvMapFresnelMinMaxExp);
*/
	// This is a lot, maybe split it up or find a better solution ?
	/*
	void InitVars(int Texture, int EnvFrame, int Flip, int Disable, int Tint,
		int Contrast, int Saturation, int LightScale, int FresnelReflection, int Fresnel,
		int FresnelMinMaxExp, int Anisotropy, int AnisotropyScale)
	{
		m_nEnvMap = Texture;
		m_nEnvMapFrame = EnvFrame;
		m_nEnvMapMaskFlip = Flip;
		m_nEnvMapDisable = Disable;
		m_nEnvMapTint = Tint;
		m_nEnvMapContrast = Contrast;
		m_nEnvMapSaturation = Saturation;
		m_nEnvMapLightScale = LightScale;
		m_nFresnelReflection = FresnelReflection;
		m_nEnvMapFresnel = Fresnel;
		m_nEnvMapFresnelMinMaxExp = FresnelMinMaxExp;
	}
	*/

	// Instead of a Macro, just copy this.
/*
	InitVars(EnvMap);
*/
	// ShiroDkxtro2: ONLY USE THIS WITH Declare_EnvironmentMapParameters
	// This works only because we always have the same Parameter Order when using the Parameter Macro
	void InitVars(int Base)
	{
		m_nEnvMap					= Base + 0;
		m_nEnvMapFrame				= Base + 1;
		m_nEnvMapMaskFlip			= Base + 2;
		m_nEnvMapDisable			= Base + 3;
		m_nEnvMapTint				= Base + 4;
		m_nEnvMapContrast			= Base + 5;
		m_nEnvMapSaturation			= Base + 6;
		m_nEnvMapLightScale			= Base + 7;
		m_nFresnelReflection		= Base + 8;
		m_nEnvMapFresnel			= Base + 9;
		m_nEnvMapFresnelMinMaxExp	= Base + 10;
	}
};

struct EnvMapMask_Vars_t
{
	EnvMapMask_Vars_t() { memset(this, 0xFF, sizeof(EnvMapMask_Vars_t)); }

	int m_nEnvMapMask;
	int m_nEnvMapMaskFrame;
	int m_nEnvMapMaskTransform;

	// Instead of a Macro, just copy this.
/*
	InitVars(EnvMapMask, EnvMapMaskFrame, EnvMapMaskTransform);
*/
	void InitVars(int Texture, int Frame, int Transform)
	{
		m_nEnvMapMask = Texture;
		m_nEnvMapMaskFrame = Frame;
		m_nEnvMapMaskTransform = Transform;
	}
};

struct Vars_SelfIllumTexture_t
{
	Vars_SelfIllumTexture_t() { memset(this, 0xFF, sizeof(Vars_SelfIllumTexture_t)); }

	int m_nSelfIllumTexture;
	int m_nSelfIllumTextureFrame;

	// Instead of a Macro, just copy this.
	/*
		InitVars(SelfIllumTexture, SelfIllumTextureFrame);
	*/
	void InitVars(int Texture, int Frame)
	{
		m_nSelfIllumTexture = Texture;
		m_nSelfIllumTextureFrame = Frame;
	}
};

struct Vars_Seamless_t
{
	Vars_Seamless_t() { memset(this, 0xFF, sizeof(Vars_Seamless_t)); }

	int m_nSeamless_Base;
	int m_nSeamless_BaseScale;
	int m_nSeamless_Detail;
	int m_nSeamless_DetailScale;

// Instead of a Macro, just copy this.
/*
	InitVars(Seamless_Base, Seamless_Scale, Seamless_Detail, Seamless_DetailScale);
*/
	void InitVars(int Base, int BaseScale, int Detail, int DetailScale)
	{
		m_nSeamless_Base = Base;
		m_nSeamless_BaseScale = BaseScale;
		m_nSeamless_Detail = Detail;
		m_nSeamless_DetailScale  = DetailScale;
	}
};

struct Vars_DepthBlend_t
{
	Vars_DepthBlend_t() { memset(this, 0xFF, sizeof(Vars_DepthBlend_t)); }

	int m_nDepthBlend;
	int m_nDepthBlendScale;
	int m_nDepthBlendInvert;

	// Instead of a Macro, just copy this.
/*
	InitVars(DepthBlend, DepthBlendFactor, DepthBlendInvert);
*/
	void InitVars(int Blend, int Factor, int Invert)
	{
		m_nDepthBlend = Blend;
		m_nDepthBlendScale = Factor;
		m_nDepthBlendInvert = Invert;
	}
};

// Moved EmissiveBlend, Cloak and FleshInterior structs to their repsective headers

// TODO: Do FleshInterior Pass Shader
struct FleshInterior_Vars_t
{
	FleshInterior_Vars_t() { memset(this, 0xFF, sizeof(FleshInterior_Vars_t)); }

	int m_nFleshTexture;
	int m_nFleshNoiseTexture;
	int m_nFleshBorderTexture1D;
	int m_nFleshNormalTexture;
	int m_nFleshSubsurfaceTexture;
	int m_nFleshCubeTexture;

	int m_nBorderNoiseScale;
	int m_nDebugForceFleshOn;
	int m_nEffectCenterRadius1;
	int m_nEffectCenterRadius2;
	int m_nEffectCenterRadius3;
	int m_nEffectCenterRadius4;

	int m_nSubsurfaceTint;
	int m_nBorderWidth;
	int m_nBorderSoftness; // > 0.0f && < 0.5f !
	int m_nBorderTint;
	int m_nGlobalOpacity;
	int m_nGlossBrightness;
	int m_nScrollSpeed;

	int m_nTime;

	// TODO: Flesh Interior Pass
};

// TODO: Move Links to dedicated Link Macro or per Shader Links once we have a GitHub Wiki for this
#define LUX_DEFAULT_DESCRIPTION	"\nVDC: https://developer.valvesoftware.com/wiki/Category:Shaders\n"
#define WEBLINK_VDC "Valve Developer Community: https://developer.valvesoftware.com/wiki/Category:Shaders\n"
#define LUX_SHADERINFO_SM20 "Shader uses SM2.0, DXLevel >= 90\n"
#define LUX_SHADERINFO_SM30 "Shader uses SM3.0, DXLevel >= 95\n"

// TODO:
// Move the below to cpp_lux_parameters.h after Carlos's changes!

//==========================================================================//
// Parameter Declarations. Used on Shader Params to allow for certain Features
//==========================================================================//

#define Declare_NormalTextureParameters()\
SHADER_PARAM(BumpMap,					SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB] Bump Map or Normal Map.\n[A] $NormalMapAlphaEnvMapMask, Default Mask for $Phong, etc.")\
SHADER_PARAM(BumpFrame,					SHADER_PARAM_TYPE_INTEGER,	"0",	"Frame Number for $BumpMap.")\
SHADER_PARAM(BumpTransform,				SHADER_PARAM_TYPE_MATRIX,	"",		"Transforms the $BumpMap Texture. Must include all Values!")\
SHADER_PARAM(NormalTexture,				SHADER_PARAM_TYPE_TEXTURE,	"",		"(INTERNAL PARAMETER) Carrier Parameter to avoid some Issues.")\
SHADER_PARAM(SSBump,					SHADER_PARAM_TYPE_BOOL,		"0",	"If 1, $BumpMap is Self-Shadowing. Note that if you enable this, both BumpMaps for BlendTextures will be SS-Bumps!") \
SHADER_PARAM(SSBumpMathFix,				SHADER_PARAM_TYPE_BOOL,		"0",	"Fixes Brightness of uncorrected SSBump's (See VDC for more Information).")\
SHADER_PARAM(LightWarpTexture,			SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB] Tints texels depending on their brightness (See VDC for more Information).\n[A] Unused.")\
SHADER_PARAM(LightWarpTextureFrame,		SHADER_PARAM_TYPE_INTEGER,	"",		"Frame Number for $LightWarpTexture")\
SHADER_PARAM(LightWarpNoBump,			SHADER_PARAM_TYPE_BOOL,		"0",	"Allows lightwarp to be used with a forced $BumpMap (See VDC for more Information).")

#define Declare_NoDiffuseBumpLighting()\
SHADER_PARAM(NoDiffuseBumpLighting,		SHADER_PARAM_TYPE_BOOL,		"",		"Stops $BumpMap from affecting Diffuse Lighting.\nUseful for for distorted Reflections on flat Surfaces.\nDoes NOT improve Performance!!!")

#define Declare_DetailTextureParameters()\
SHADER_PARAM(Detail,					SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB or RGBA] Detail Texture (See VDC for more Information).")\
SHADER_PARAM(DetailBlendMode,			SHADER_PARAM_TYPE_INTEGER,	"0",	"Modes: 0 to 11. (See VDC for more Information).")\
SHADER_PARAM(DetailBlendFactor,			SHADER_PARAM_TYPE_FLOAT,	"",		"Controls the Amount that $Detail affects the $BaseTexture. Or itself.. In case of some Blendmodes..")\
SHADER_PARAM(DetailScale,				SHADER_PARAM_TYPE_FLOAT,	"4",	"Scale of the Detail Texture.")\
SHADER_PARAM(DetailTextureTransform,	SHADER_PARAM_TYPE_MATRIX,	"",		"Transforms $Detail. Must include all Values!")\
SHADER_PARAM(DetailTint,				SHADER_PARAM_TYPE_COLOR,	"",		"Tints $Detail. Note: expect a RGB Value, like [1 0 0] (See VDC for more Information)")\
SHADER_PARAM(DetailFrame,				SHADER_PARAM_TYPE_INTEGER,	"0",	"Frame Number for $Detail.")\

#define Declare_Detail2TextureParameters()\
SHADER_PARAM(Detail2,					SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB or RGBA] Second Detail Texture. Note: Does not support $Detailblendmode2.")\
SHADER_PARAM(DetailScale2,				SHADER_PARAM_TYPE_FLOAT,	"4",	"Scale of the Detail Texture.")\
SHADER_PARAM(DetailTint2,				SHADER_PARAM_TYPE_COLOR,	"",		"Tints $Detail. Note: expect a RGB Value, like [1 0 0] (See VDC for more Information)")\
SHADER_PARAM(DetailFrame2,				SHADER_PARAM_TYPE_INTEGER,	"0",	"Frame Number for $Detail.")\
SHADER_PARAM(DetailTextureTransform2,	SHADER_PARAM_TYPE_MATRIX,	"",		"Transforms $Detail2. Must include all Values!")

#define Declare_LightmappingParameters()\
SHADER_PARAM(Lightmap,					SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB or RGBA] Lightmap Texture. Manually specifying might not work, handled by the Engine in MP Branches.")

// Always, no ifdef
// Absolute must have for TF2SDK!
#define Declare_DistanceAlphaParameters()\
SHADER_PARAM(DistanceAlpha,				SHADER_PARAM_TYPE_BOOL,		"",		"Cheap edge filtering technique for raster images, great for UI elements, foliage, chain link fences, grates, and more.")\
SHADER_PARAM(DistanceAlphaFromDetail,	SHADER_PARAM_TYPE_BOOL,		"",		"Take the distance-coded alpha mask from the Detail Texture.")\
SHADER_PARAM(SoftEdges,					SHADER_PARAM_TYPE_BOOL,		"",		"Enable soft edges to distance coded Textures.")\
SHADER_PARAM(ScaleEdgeSoftnessBasedOnScreenRes, SHADER_PARAM_TYPE_BOOL, "", "Scale the size of the soft edges based upon resolution. 1024x768 = nominal.")\
SHADER_PARAM(EdgeSoftnessStart,			SHADER_PARAM_TYPE_FLOAT,	"",		"Start Value for soft edges for distancealpha.")\
SHADER_PARAM(EdgeSoftnessEnd,			SHADER_PARAM_TYPE_FLOAT,	"",		"End Value for soft edges for distancealpha.")\
SHADER_PARAM(Glow,						SHADER_PARAM_TYPE_BOOL,		"",		"Enable glow/shadow for distance coded Textures.")\
SHADER_PARAM(GlowColor,					SHADER_PARAM_TYPE_COLOR,	"",		"Color of outter glow for distance coded line art.")\
SHADER_PARAM(GlowAlpha,					SHADER_PARAM_TYPE_FLOAT,	"",		"Base glow alpha Amount for glows/shadows with distance alpha.")\
SHADER_PARAM(GlowStart,					SHADER_PARAM_TYPE_FLOAT,	"",		"Start Value for glow/shadow.")\
SHADER_PARAM(GlowEnd,					SHADER_PARAM_TYPE_FLOAT,	"",		"End Value for glow/shadow.")\
SHADER_PARAM(GlowX,						SHADER_PARAM_TYPE_FLOAT,	"",		"Texture offset x for glow mask.")\
SHADER_PARAM(GlowY,						SHADER_PARAM_TYPE_FLOAT,	"",		"Texture offset y for glow mask.")\
SHADER_PARAM(Outline,					SHADER_PARAM_TYPE_BOOL,		"",		"Enable outline for distance coded Textures.")\
SHADER_PARAM(OutlineColor,				SHADER_PARAM_TYPE_COLOR,	"",		"Color of outline for distance coded images.")\
SHADER_PARAM(OutlineAlpha,				SHADER_PARAM_TYPE_FLOAT,	"",		"Alpha Value for outline.")\
SHADER_PARAM(OutlineStart0,				SHADER_PARAM_TYPE_FLOAT,	"",		"Outer start Value for outline.")\
SHADER_PARAM(OutlineStart1,				SHADER_PARAM_TYPE_FLOAT,	"",		"Inner start Value for outline.")\
SHADER_PARAM(OutlineEnd0,				SHADER_PARAM_TYPE_FLOAT,	"",		"Inner end Value for outline.")\
SHADER_PARAM(OutlineEnd1,				SHADER_PARAM_TYPE_FLOAT,	"",		"Outer end Value for outline.")\
SHADER_PARAM(ScaleOutlineSoftnessBasedOnScreenRes, SHADER_PARAM_TYPE_BOOL, "", "Scale the size of the soft part of the outline based upon resolution. 1024x768 = nominal.")

#define Declare_EnvironmentMapParameters()\
SHADER_PARAM(EnvMap,					SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Set the cubemap for this Material.")\
SHADER_PARAM(EnvMapFrame,				SHADER_PARAM_TYPE_INTEGER,	"", "The frame to start an animated envmap on.")\
SHADER_PARAM(EnvMapMaskFlip,			SHADER_PARAM_TYPE_BOOL,		"", "Flips the EnvMapMask. This is done automatically when using $basealphaenvmapmask but can be overriden using this parameter!")\
SHADER_PARAM(EnvMapTint,				SHADER_PARAM_TYPE_COLOR,	"", "Envmap tint. Works like vector3 scale where each Number scales the specific color by that much.")\
SHADER_PARAM(EnvMapContrast,			SHADER_PARAM_TYPE_FLOAT,	"", "Expected range: 0 to 1, where 1 is full contrast and 0 is pure cubemap.")\
SHADER_PARAM(EnvMapSaturation,			SHADER_PARAM_TYPE_COLOR,	"", "Expected range: 0 to 1, where 1 is default pure cubemap and 0 is b/w.")\
SHADER_PARAM(EnvMapLightScale,			SHADER_PARAM_TYPE_FLOAT,	"", "Value of 0 will disable the behavior and is set by default.")\
SHADER_PARAM(FresnelReflection,			SHADER_PARAM_TYPE_FLOAT,	"", "1.0 == mirror, 0.0 == water.")\
SHADER_PARAM(EnvMapFresnel,				SHADER_PARAM_TYPE_FLOAT,	"", "Degree to which Fresnel should be applied to env map")\
SHADER_PARAM(EnvMapFresnelMinMaxExp,	SHADER_PARAM_TYPE_VEC3,		"", "Min/max fresnel range and exponent for VertexLitGeneric.")

// NOTE: $EnvMapParallax is not a Parameter on the VDC Article but it's used in Mapbase
// It's superior to checking if something is defined or if a Parameter is of a specific Type after Shader Init
#define Declare_ParallaxCorrectionParameters()\
SHADER_PARAM(EnvMapParallax,			SHADER_PARAM_TYPE_BOOL,		"", "(INTERNAL PARAMETER) Enables Parallax Mapping")\
SHADER_PARAM(EnvMapParallaxOBB1,		SHADER_PARAM_TYPE_VEC4,		"", "(INTERNAL PARAMETER) The first line of the parallax correction OBB matrix")\
SHADER_PARAM(EnvMapParallaxOBB2,		SHADER_PARAM_TYPE_VEC4,		"", "(INTERNAL PARAMETER) The second line of the parallax correction OBB matrix")\
SHADER_PARAM(EnvMapParallaxOBB3,		SHADER_PARAM_TYPE_VEC4,		"", "(INTERNAL PARAMETER) The third line of the parallax correction OBB matrix")\
SHADER_PARAM(EnvMapOrigin,				SHADER_PARAM_TYPE_COLOR,	"", "(INTERNAL PARAMETER) The world space position of the env_cubemap being corrected")

// Note that $normalmapalphaenvmapmask and $basealphaenvmapmask are global vars.-Doesn't need to be defined here.
// Note that some Shaders (LightmappedReflective) use $EnvMapMask not for EnvMaps but Planar Reflections
// Which is why we use *Reflections* in the Descriptions, not *EnvMap*
#define Declare_EnvMapMaskParameters()\
SHADER_PARAM(EnvMapMask,				SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] Per-Texel Mask Texture for Reflections, can be used as a Per-Texel Tint too!\n[A] SelfIllumMask when using $SelfIllum_EnvMapMask_Alpha")\
SHADER_PARAM(EnvMapMaskFrame,			SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $EnvMapMask.")\
SHADER_PARAM(EnvMapMaskTransform,		SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the $EnvMapMask Texture. Must include all Values!")

#define Declare_EnvMapMask2Parameters()\
SHADER_PARAM(EnvMapMask2,				SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] Per-Texel Mask Texture for Reflections, can be used as a Per-Texel Tint too!\n[A] SelfIllumMask when using $SelfIllum_EnvMapMask_Alpha")\
SHADER_PARAM(EnvMapMaskFrame2,			SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $EnvMapMask2.")\
SHADER_PARAM(EnvMapMaskTransform2,		SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the $EnvMapMask2 Texture. Must include all Values!")

#define Declare_DisplacementBase()\
SHADER_PARAM(BaseTexture2,				SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB or RGBA] Second BaseTexture.")\
SHADER_PARAM(Frame2,					SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $baseTexture2")\
SHADER_PARAM(BaseTextureTransform2,		SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the baseTexture2 Texture. Must include all Values!")\
SHADER_PARAM(BlendTintColorOverBase2,	SHADER_PARAM_TYPE_FLOAT,	"",	"Blend between tint acting as a multiplication versus a replace, requires $blendtintbybasealpha.")\
SHADER_PARAM(DesaturateWithBaseAlpha2,	SHADER_PARAM_TYPE_FLOAT,	"",	"Same as $DesaturateWithBaseAlpha, which is the one you need to use to enable the Feature. This one sets the Amount of desaturation for the second BaseTexture.")

#define Declare_DisplacementBump()\
SHADER_PARAM(BumpMap2,					SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB or RGBA] Bump Map or Normal Map.")\
SHADER_PARAM(BumpFrame2,				SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $BumpMap2.")\
SHADER_PARAM(BumpTransform2,			SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the bump map Texture. Must include all Values!")

#define Declare_DisplacementBlend()\
SHADER_PARAM(BlendModulateTexture,		SHADER_PARAM_TYPE_TEXTURE,	"",	"Modulates Blending.\n[R] Controls Sharpness, Black is Sharpest.\n[G] Bias Factor. Black biases towards 2nd Material, White towards 1st.\n[BA] Opacity for Material 1 and Material 2 when using $BlendModulateTransparency.")\
SHADER_PARAM(BlendMaskFrame,			SHADER_PARAM_TYPE_INTEGER,	"",	"Drame Number for $blendmodulateTexture.")\
SHADER_PARAM(BlendMaskTransform,		SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the blendmodulateTexture Texture. Must include all Values!")

#define Declare_DisplacementPhong()\
SHADER_PARAM(PhongExponent2,				SHADER_PARAM_TYPE_FLOAT,	"", "Phong Exponent for Specular Lighting.")\
SHADER_PARAM(PhongExponentTexture2,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB or RGBA] Defines a Texture which on a per texel bases defines the exponent Value for a surface.")\
SHADER_PARAM(PhongExponentTextureFrame2,	SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $PhongExponentTexture.")\
SHADER_PARAM(PhongExponentFactor2,			SHADER_PARAM_TYPE_FLOAT,	"", "The Exponent will be multiplied by this, useful for $PhongExponentTexture.")\
SHADER_PARAM(BaseTextureNoPhong,			SHADER_PARAM_TYPE_BOOL,		"", "Stops the first Texture from receiving Phong. ( Sets the Tint to 0 )")\
SHADER_PARAM(BaseTexture2NoPhong,			SHADER_PARAM_TYPE_BOOL,		"", "Stops the second Texture from receiving Phong. ( Sets the Tint to 0 )")\
SHADER_PARAM(PhongFresnelRanges2,			SHADER_PARAM_TYPE_VEC3,		"", "Factors for remapping Fresnel (See VDC for more Information).")

// Note that $selfillum is a global var.-Doesn't need to be defined here..
#define Declare_SelfIlluminationParameters()\
SHADER_PARAM(SelfIllumMask,				SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGBA] Acts as a seperate mask instead of using an alpha channel.")\
SHADER_PARAM(SelfIllumMaskScale,		SHADER_PARAM_TYPE_FLOAT,	"",	"Range from 0.01 to 9.99, trying any other Values might affect other effects.")\
SHADER_PARAM(SelfIllumMaskFrame,		SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $SelfIllumMask.")\
SHADER_PARAM(SelfIllum_EnvMapMask_Alpha,SHADER_PARAM_TYPE_BOOL,		"",	"Use the Envmapmask Alpha as the Selfillum Mask.")\
SHADER_PARAM(SelfIllumFresnel,			SHADER_PARAM_TYPE_BOOL,		"",	"Allows the Material to use Fresnel Ranges for Selfillum.")\
SHADER_PARAM(SelfIllumFresnelMinMaxExp, SHADER_PARAM_TYPE_VEC3,		"",	"Vector format: Minimum Illumination, Maximum Illumination, Fresnel Exponent.  Note: Range 0 to 1.")\
SHADER_PARAM(SelfIllumTint,				SHADER_PARAM_TYPE_COLOR,	"",	"Tint the entire Selfillum.")

#define Declare_SelfIllumTextureParameters()\
SHADER_PARAM(SelfIllumTexture,			SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] Emission/Additive Selfillum.\n[A] Unused.")\
SHADER_PARAM(SelfIllumTextureFrame,		SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $SelfIllumTexture.")

#define Declare_MiscParameters()\
SHADER_PARAM(DesaturateWithBaseAlpha,	SHADER_PARAM_TYPE_FLOAT, "", "If used, the $BaseTexture's Alpha Channel will be used as a Mask to desaturate the $BaseTexture's Color. 1.0 will result in 0% Saturation.")\
SHADER_PARAM(BlendTintByBaseAlpha,		SHADER_PARAM_TYPE_BOOL,  "", "Use the BaseTextures Alpha Channel to blend in Tint Parameters.")\
SHADER_PARAM(BlendTintColorOverBase,	SHADER_PARAM_TYPE_FLOAT, "", "Blend between Tint acting as a Multiplier versus a Replacement, requires $BlendTintByBaseAlpha.")

#define Declare_PhongParameters()\
SHADER_PARAM(Phong,						SHADER_PARAM_TYPE_BOOL,		"",	"Enables Phong Reflections.")\
SHADER_PARAM(PhongExponent,				SHADER_PARAM_TYPE_FLOAT,	"",	"Phong Exponent for Specular Lighting." )\
SHADER_PARAM(PhongTint,					SHADER_PARAM_TYPE_COLOR,	"",	"Tints the Specular Highlights.")\
SHADER_PARAM(PhongAlbedoTint,			SHADER_PARAM_TYPE_BOOL,		"",	"Apply Albedo as Tint ( Controlled by $PhongExponentTexture )")\
SHADER_PARAM(PhongWarpTexture,			SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] Used to create an iridescence Effect. U is the Strength of the Highlight, V is Fresnel. (See VDC for more Information).")\
SHADER_PARAM(PhongWarpTextureFrame,		SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $PhongWarpTexture.")\
SHADER_PARAM(PhongFresnelRanges,		SHADER_PARAM_TYPE_VEC3,		"",	"Factors for remapping Fresnel (See VDC for more Information).")\
SHADER_PARAM(PhongBoost,				SHADER_PARAM_TYPE_FLOAT,	"",	"Phong overbrightening Factor. Does not apply to $RimLight.")\
SHADER_PARAM(PhongExponentTexture,		SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB or RGBA] Defines a Texture which on a per texel bases defines the exponent Value for a surface.")\
SHADER_PARAM(PhongExponentTextureFrame, SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $PhongExponentTexture.")\
SHADER_PARAM(PhongExponentFactor,		SHADER_PARAM_TYPE_FLOAT,	"",	"The Exponent will be multiplied by this, useful for $PhongExponentTexture.")\
SHADER_PARAM(BaseMapAlphaPhongMask,		SHADER_PARAM_TYPE_BOOL,		"",	"Indicates that there is no normal map and that the phong mask is in base alpha.")\
SHADER_PARAM(InvertPhongMask,			SHADER_PARAM_TYPE_BOOL,		"",	"Invert the phong mask (0 = full phong, 1 = no phong).")\
SHADER_PARAM(PhongDisableHalfLambert,	SHADER_PARAM_TYPE_BOOL,		"",	"Disable half lambert for phong.")\
SHADER_PARAM(PhongAlbedoBoost,			SHADER_PARAM_TYPE_FLOAT,	"",	"Phong albedo overbrightening factor. Ranges 0 to 255. Will multiply $PhongAlbedoTint by this Value.")\
SHADER_PARAM(BaseMapLuminancePhongMask, SHADER_PARAM_TYPE_BOOL,		"",	"Mask Phong using the perceptual Luminance of the $BaseTexture.")\
SHADER_PARAM(PhongNewBehaviour,			SHADER_PARAM_TYPE_BOOL,		"",	"Use new masking behaviour. Aka what truly is in your vmt and not weird logic.")\
SHADER_PARAM(PhongExponentTextureMask,	SHADER_PARAM_TYPE_BOOL,		"",	"The blue channel of the exponentTexture is used for masking Phong.")\
SHADER_PARAM(PhongMinimumLight,			SHADER_PARAM_TYPE_COLOR,	"", "Minimum Light Level for Phong'd Models. Intended for Character Models that are too dark in Shadows.")\
SHADER_PARAM(PhongFlatNormal,			SHADER_PARAM_TYPE_BOOL,		"", "Stock Shaders use flat Normals for $BaseMapAlphaPhongMask Materials.\nOn LUX_VLG this Caveat can be controlled using this Parameter.\nSet to 1 when using $BaseMapAlphaPhongMask without $PhongNewBehaviour.")

#define Declare_RimLightParameters()\
SHADER_PARAM(RimLight,					SHADER_PARAM_TYPE_BOOL,		"",	"Adds a constant rim light to the model that appears to come indirectly from the environment.")\
SHADER_PARAM(RimLightExponent,			SHADER_PARAM_TYPE_FLOAT,	"",	"Exponent for phong component of rim lighting. The rim exponent defines the tightness of the highlight. A higher exponent results in a smaller, tighter highlight while a lower exponent results in a broader flatter one.")\
SHADER_PARAM(RimLightBoost,				SHADER_PARAM_TYPE_FLOAT,	"",	"Additive boost for ambient cube component of rim lighting.")\
SHADER_PARAM(RimMask,					SHADER_PARAM_TYPE_BOOL,		"",	"Indicates whether or not to use alpha channel of exponent Texture to mask the rim term.")

#define Declare_TreeswayParameters()\
SHADER_PARAM(TreeSway,					SHADER_PARAM_TYPE_INTEGER,	"",		"Modes: (0 =  No Sway, 1 =  Classic tree sway, 2 = Radial tree sway effect, intended for use on rectangular sheets of plastic/tarp attached at four corners) (See VDC for more Information)." )\
SHADER_PARAM(TreeSwayHeight,			SHADER_PARAM_TYPE_FLOAT,	"",		"The starting height in Hammer units where the effect should start being fully applied. Default 1000.")\
SHADER_PARAM(TreeSwayStartHeight,		SHADER_PARAM_TYPE_FLOAT,	"",		"The height from the model's origin in which the effect starts blending in. Default 0.2.")\
SHADER_PARAM(TreeSwayRadius,			SHADER_PARAM_TYPE_FLOAT,	"",		"The radius from the model's origin in Hammer units in where the effect should start being fully applied. Default 300.")\
SHADER_PARAM(TreeSwayStartRadius,		SHADER_PARAM_TYPE_FLOAT,	"",		"The radius from the model's origin in which the effect starts blending in. Default 0.1.")\
SHADER_PARAM(TreeSwaySpeed,				SHADER_PARAM_TYPE_FLOAT,	"",		"The speed multiplier of large movement such as the trunk. Default 1.")\
SHADER_PARAM(TreeSwaySpeedHighWindMultiplier, SHADER_PARAM_TYPE_FLOAT, "",	"Speed multiplier when env_wind triggers a gust. Default 2")\
SHADER_PARAM(TreeSwayStrength,			SHADER_PARAM_TYPE_FLOAT,	"",		"The distance multiplier of large movement such as the trunk. Default 10.")\
SHADER_PARAM(TreeSwayScrumbleSpeed,		SHADER_PARAM_TYPE_FLOAT,	"",		"The falloff of the effect on large movement such as the trunk. Higher means a more stable center. Default 0.1.")\
SHADER_PARAM(TreeSwayScrumbleStrength,	SHADER_PARAM_TYPE_FLOAT,	"",		"The speed multiplier of the small movement such as the leaves. Default 0.1.")\
SHADER_PARAM(TreeSwayScrumbleFrequency, SHADER_PARAM_TYPE_FLOAT,	"",		"The distance multiplier of the small movement such as the leaves. Default 0.1.")\
SHADER_PARAM(TreeSwayFalloffExp,		SHADER_PARAM_TYPE_FLOAT,	"",		"The frequency of the rippling of a sine wave in small movement such as the leaves. Default 1.5.")\
SHADER_PARAM(TreeSwayScrumbleFalloffExp,SHADER_PARAM_TYPE_FLOAT,	"",		"The falloff of the effect on small movement such as the leaves. Higher means a more stable center. Default 1.")\
SHADER_PARAM(TreeSwaySpeedLerpStart,	SHADER_PARAM_TYPE_FLOAT,	"",		"Minimum wind speed in which a gust triggered by env_wind will start affecting the Material. Default 3.")\
SHADER_PARAM(TreeSwaySpeedLerpEnd,		SHADER_PARAM_TYPE_FLOAT,	"",		"Minimum wind speed in which a gust triggered by env_wind will fully affect the Material. Default 6.")\
SHADER_PARAM(TreeSwayStatic,			SHADER_PARAM_TYPE_BOOL,		"",		"Use a static wind Value instead of the Values from env_wind. If enabled, env_wind is not required.")\
SHADER_PARAM(TreeSwayStaticValues,		SHADER_PARAM_TYPE_VEC2,		"",		"Sets the static wind Values used by $treeswaystatic")

#define Declare_SeamlessParameters()\
SHADER_PARAM(Seamless_Base,				SHADER_PARAM_TYPE_BOOL,		"", "Enables triplanar mapping.")\
SHADER_PARAM(Seamless_Scale,			SHADER_PARAM_TYPE_FLOAT,	"", "Prevents Texture stretching issues on displacement surfaces.")\
SHADER_PARAM(Seamless_Detail,			SHADER_PARAM_TYPE_BOOL,		"", "Enables the effect on Detail Textures with the VertexLitGeneric shader. Requires $seamless_scale to set the scale of the effect and only applies to the Detail Texture.")\
SHADER_PARAM(Seamless_DetailScale,		SHADER_PARAM_TYPE_FLOAT,	"", "Prevents Detail Texture stretching issues on displacement surfaces.")

#define Declare_DepthBlendParameters()\
SHADER_PARAM(DepthBlend,				SHADER_PARAM_TYPE_INTEGER,	"", "Fade Alpha when Geometry of the Material gets close to other Geometry ( Intersections ).")\
SHADER_PARAM(DepthBlendScale,			SHADER_PARAM_TYPE_FLOAT,	"", "Amplify or reduce $DepthBlend. With regular DepthBlend lower Values will have a harder Transitions.")\
SHADER_PARAM(DepthBlendInverse,			SHADER_PARAM_TYPE_BOOL,		"", "$Flip DepthBlend so that Surfaces *appear* near Geometry instead of disappearing.")

#define Declare_CloakParameters()\
SHADER_PARAM(CloakPassEnabled,			SHADER_PARAM_TYPE_BOOL,		"", "Enables cloak render in a second pass")\
SHADER_PARAM(CloakFactor,				SHADER_PARAM_TYPE_FLOAT,	"", "Range: 0.00 = fully visible, 1.00 = fully invisible.")\
SHADER_PARAM(CloakColorTint,			SHADER_PARAM_TYPE_COLOR,	"", "Colors the refraction effect.")\
SHADER_PARAM(RefractAmount,				SHADER_PARAM_TYPE_FLOAT,	"", "How strong the refraction effect should be when the Material is partially cloaked.")

// Emissive Scroll Pass
#define Declare_EmissiveBlendParameters()\
SHADER_PARAM(EmissiveBlendEnabled,		SHADER_PARAM_TYPE_BOOL,			"", "Enable emissive blend pass.")\
SHADER_PARAM(EmissiveBlendBaseTexture,	SHADER_PARAM_TYPE_TEXTURE,		"", "[RGB or RGBA] The Texture used for the self-illumination. Can be colored or grayscale. Does not adopt the $baseTexture's color.")\
SHADER_PARAM(EmissiveBlendTransform,	SHADER_PARAM_TYPE_MATRIX,		"", "Transforms the $emissiveblendbaseTexture Texture. Must include all Values!")\
SHADER_PARAM(EmissiveBlendScrollVector, SHADER_PARAM_TYPE_VEC2,			"", "A matrix that controls the direction and speed the $emissiveblendTexture scrolls in. X (horizontal) and Y (vertical) respectively. A setting of 1 results in the Texture tiling every 1 second, 0.1 every 10 seconds. 0 is no scroll.")\
SHADER_PARAM(EmissiveBlendStrength,		SHADER_PARAM_TYPE_FLOAT,		"", "Controls the opacity of the effect. Ranges from 0 to 1; at 0, the effect is invisible, at 1, it is at full strength.")\
SHADER_PARAM(EmissiveBlendTexture,		SHADER_PARAM_TYPE_TEXTURE,		"", "[RGB] The Texture that scrolls by based on the flowmap and the scrolling speed parameter. Not mapped to a UV.")\
SHADER_PARAM(EmissiveBlendTint,			SHADER_PARAM_TYPE_COLOR,		"", "Color tint of the effect.")\
SHADER_PARAM(EmissiveBlendFlowTexture,	SHADER_PARAM_TYPE_TEXTURE,		"", "[RGB] Flowmap used for the $emissiveblendTexture.")\
SHADER_PARAM(EmissiveBlend_NoFlowTransform, SHADER_PARAM_TYPE_MATRIX,	"", "Only usable without flowTexture. Uses actual UV for Emission Texture.")\
SHADER_PARAM(Time,						SHADER_PARAM_TYPE_FLOAT,		"", "Allows you to hook a custom time based variable to the shader.")\
SHADER_PARAM(MinimumLightAdditivePass,	SHADER_PARAM_TYPE_BOOL,			"", "Enables Minimum Light Additive Pass.")\
SHADER_PARAM(MinimumLightTint,			SHADER_PARAM_TYPE_VEC3,			"", "Tint for the Pass. By Default 0")

#define Declare_FleshInteriorParameters()\
SHADER_PARAM(FleshInteriorEnabled,		SHADER_PARAM_TYPE_BOOL,		"", "Enables or disables the flesh effect for the Material." )\
SHADER_PARAM(FleshInteriorTexture,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Flesh color Texture.")\
SHADER_PARAM(FleshInteriorNoiseTexture, SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Used on the border of the flesh effect, adding a distortion effect on the transition between exterior and interior. Scrolls based on the Value of $fleshscrollspeed")\
SHADER_PARAM(FleshBorderTexture1D,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Determines the colors used by the border between the flesh interior and exterior model.")\
SHADER_PARAM(FleshNormalTexture,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Flesh normal Texture.")\
SHADER_PARAM(FleshSubSurfaceTexture,	SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] A Texture which is multiplied with the $fleshinteriorTexture Texture.")\
SHADER_PARAM(FleshCubeTexture,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] The cubemap Texture to use for reflections on the flesh interior. Accepts Values similar to $envmap (including env_cubemap).")\
SHADER_PARAM(FleshBorderNoiseScale,		SHADER_PARAM_TYPE_FLOAT,	"", "Changes the scale of the $fleshinteriornoiseTexture.")\
SHADER_PARAM(FleshDebugForceFleshOn,	SHADER_PARAM_TYPE_BOOL,		"", "If set to 1, shows the flesh rendering pass on the entire model, regardless of the mask specified on the $fleshinteriorTexture or the effect targets.")\
SHADER_PARAM(FleshSubSurfaceTint,		SHADER_PARAM_TYPE_COLOR,	"", "Allows tinting the $fleshsubsurfaceTexture.")\
SHADER_PARAM(FleshBorderWidth,			SHADER_PARAM_TYPE_FLOAT,	"", "Determines the width of the border transition between interior and exterior. May change from which camera angles the effect is visible, as well. Must be greater than 0 for the effect to work at all.")\
SHADER_PARAM(FleshBorderSoftness,		SHADER_PARAM_TYPE_FLOAT,	"", "Changes how soft the transition between interior and exterior is. Values must be greater than 0 but no higher than 0.5.")\
SHADER_PARAM(FleshBorderTint,			SHADER_PARAM_TYPE_COLOR,	"", "Allows tinting the effect border.")\
SHADER_PARAM(FleshEffectCenterRadius1,	SHADER_PARAM_TYPE_VEC4,		"", "This is how the position of point_flesh_effect_targets is communicated to the shader. The flesh effect target functionality is applied at the specified world location using the specified radius.")\
SHADER_PARAM(FleshEffectCenterRadius2,	SHADER_PARAM_TYPE_VEC4,		"", "This is how the position of point_flesh_effect_targets is communicated to the shader. The flesh effect target functionality is applied at the specified world location using the specified radius.")\
SHADER_PARAM(FleshEffectCenterRadius3,	SHADER_PARAM_TYPE_VEC4,		"", "This is how the position of point_flesh_effect_targets is communicated to the shader. The flesh effect target functionality is applied at the specified world location using the specified radius.")\
SHADER_PARAM(FLESHEFFECTCENTERRADIUS4,	SHADER_PARAM_TYPE_VEC4,		"", "This is how the position of point_flesh_effect_targets is communicated to the shader. The flesh effect target functionality is applied at the specified world location using the specified radius.")\
SHADER_PARAM(FleshGlobalOpacity,		SHADER_PARAM_TYPE_FLOAT,	"", "Opacity of the entire flesh effect. Essentially a $alpha for the entire effect. Note: Range 0 to 1.")\
SHADER_PARAM(FleshGlobalBrightness,		SHADER_PARAM_TYPE_FLOAT,	"", "Changes the strength or brightness of the cubemap reflection (if $fleshcubeTexture is specified).")\
SHADER_PARAM(FleshScrollSpeed,			SHADER_PARAM_TYPE_FLOAT,	"", "Determines how fast the $fleshinteriornoiseTexture should scroll.")

#define Declare_SheenPassParameters()\
SHADER_PARAM(SheenPassEnabled,		SHADER_PARAM_TYPE_BOOL,		"", "Enables the Weapon Sheen Pass.")\
SHADER_PARAM(SheenMap,				SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Texture for the Sheen Effect ( Expects a Cubemap )")\
SHADER_PARAM(SheenMapMask,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Mask Texture for the $SheenMap. Red affects Opacity.")\
SHADER_PARAM(SheenMapMaskFrame,		SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $SheenMapMask")\
SHADER_PARAM(SheenMapTint,			SHADER_PARAM_TYPE_COLOR,	"", "Tint for $SheenMap")\
SHADER_PARAM(SheenMapMaskOffsetX,	SHADER_PARAM_TYPE_FLOAT,	"", "X Offset of the $SheenMapMask, relative to Position Coordinate in Model Space.")\
SHADER_PARAM(SheenMapMaskOffsetY,	SHADER_PARAM_TYPE_FLOAT,	"", "Y Offset of the $SheenMapMask, relative to Position Coordinate in Model Space.")\
SHADER_PARAM(SheenMapMaskScaleX,	SHADER_PARAM_TYPE_FLOAT,	"", "X Scale for the $SheenMapMask.")\
SHADER_PARAM(SheenMapMaskScaleY,	SHADER_PARAM_TYPE_FLOAT,	"", "Y Scale for the $SheenMapMask.")\
SHADER_PARAM(SheenMapMaskDirection, SHADER_PARAM_TYPE_INTEGER,	"", "Direction in which the $SheenMapMask should move.\n0 = ZY\n1 = ZX\n2 = YX")\
SHADER_PARAM(SheenIndex,			SHADER_PARAM_TYPE_INTEGER,	"", "Not Functional.")

#endif // CPP_LUX_SHARED_H