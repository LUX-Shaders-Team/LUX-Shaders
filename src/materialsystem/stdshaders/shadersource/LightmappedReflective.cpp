//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	25.11.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	LUX_LightmappedReflective Shader for func_reflective_glass Entities
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_brush_vs30.inc"
#include "lux_lightmappedgeneric_flashlight_ps30.inc"
#include "lux_lightmappedreflective_vs30.inc"
#include "lux_lightmappedreflective_ps30.inc"

#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_LightmappedReflective_DX90, LUX_LightmappedReflective)
DEFINE_FALLBACK_SHADER(SDK_LightmappedReflective, LUX_LightmappedReflective)
#endif

#ifdef REPLACE_LIGHTMAPPEDREFLECTIVE
DEFINE_FALLBACK_SHADER(LightmappedReflective_DX90, LUX_LightmappedReflective)
DEFINE_FALLBACK_SHADER(LightmappedReflective, LUX_LightmappedReflective)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_LightmappedReflective, "A shader used to produce perfectly reflective glass that renders world + entities.")
SHADER_INFO_GEOMETRY	("func_reflective_glass.")
SHADER_INFO_USAGE		("Create a Brush, turn it into a func_reflective_glass and apply the Material.")
SHADER_INFO_LIMITATIONS	("Only Planar Reflections are suppored.\n"
						 "Does not support $Phong.\n"
						 "Does not support $BlendTintByBaseAlpha or $DesaturateWithBaseAlpha.\n"
						 "Does not support ")
SHADER_INFO_PERFORMANCE	("Expensive to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC LightmappedReflective Shader Page: https://developer.valvesoftware.com/wiki/LightmappedReflectivehttps://developer.valvesoftware.com/wiki/LightmappedReflective")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(RefractTexture,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Texture to use for refraction. For real-time refractions, use _rt_WaterRefraction.")
	SHADER_PARAM(ReflectTexture,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Texture to use for reflection. For real-time reflections, use _rt_WaterReflection.")
	SHADER_PARAM(RefractAmount,				SHADER_PARAM_TYPE_FLOAT,	"", "Amount of 'warp' for the Refraction. Higher values produce more Refraction.")
	SHADER_PARAM(RefractTint,				SHADER_PARAM_TYPE_COLOR,	"", "Tints the Diffuse Part of the Result ( Refracted Background ) Does not Tint Reflections!")
	SHADER_PARAM(ReflectAmount,				SHADER_PARAM_TYPE_FLOAT,	"", "Amount of 'warp' for the reflection. Higher values 'reflect more'. Also controls the strength of the $BumpMap.")
	SHADER_PARAM(ReflectTint,				SHADER_PARAM_TYPE_COLOR,	"", "Tints the Specular Part of the Result ( Reflected Foreground ) Does not Tint Refractions!")
	SHADER_PARAM(Reflectance,				SHADER_PARAM_TYPE_FLOAT,	"", "Controls how Reflective the Material is ( Opacity of Reflection ) works similar to $Alpha.")
	SHADER_PARAM(ReflectLightModulation,	SHADER_PARAM_TYPE_BOOL,		"" ,"$EnvMapLightScale but for LightmappedReflective, Tint the Surfaces Indirect Specular Component with the Lightmap.")

	// Keep support for $NormalMap
	// But sneakily add $BumpMap support
	// also all the other Bumpmap Stuff, because.. why not
	Declare_NormalTextureParameters()
	SHADER_PARAM(NormalMap, SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Same as $BumpMap.\n[A] Per-Texel Normal Mapping Scale for Reflections and Refractions.")

	Declare_DetailTextureParameters()
	Declare_SelfIlluminationParameters()
	Declare_SelfIllumTextureParameters()
	Declare_EnvMapMaskParameters()

	Declare_NoDiffuseBumpLighting()
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// We always just use $BumpMap Instead
	// I don't know what the in-engine behaviour is for this Parameter
	// So I will just always set $NormalMap to *something*
	// Similar to how Models need to fudge the $BumpMap Parameter,
	// I assume something similar is going on with this Shader
	if (!IsDefined(BumpMap) && IsDefined(NormalMap))
	{
		SetString(BumpMap, GetString(NormalMap));
	}
	else if (!IsDefined(NormalMap) && IsDefined(BumpMap))
	{
		SetString(NormalMap, GetString(BumpMap));
	}

	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// We could just always force this Flag..
	if (IsDefined(BaseTexture))
		SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

	if (g_pConfig->UseBumpmapping() && (IsDefined(BumpMap) || IsDefined(NormalMap)))
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);

	DefaultFloat(Reflectance, 0.25f);
	DefaultFloat(DetailBlendFactor, 1.0f);
	DefaultFloat(DetailScale, 4.0f);
}

SHADER_FALLBACK	
{
#ifndef REPLACE_LIGHTMAPPEDREFLECTIVE
	if (lux_oldshaders.GetBool())
		return "LightmappedReflective_DX90";
#endif

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}

	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadBumpMap(BumpMap);
	LoadBumpMap(NormalMap);
	LoadTexture(ReflectTexture);
	LoadTexture(RefractTexture);
	LoadTexture(EnvMapMask);

	// Stock-Consistency
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap) && IsTextureLoaded(BumpMap))
	{
		bHasBumpMap = true;

		// If the User didn't specifically imply this is a SSBump, check using the flags
		if (!IsDefined(SSBump))
		{
			ITexture *pBumpMap = GetTexture(BumpMap);
			bool bIsSSBump = pBumpMap->GetFlags() & TEXTUREFLAGS_SSBUMP ? true : false;
			SetBool(SSBump, bIsSSBump);
		}
	}

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	if (IsDefined(Detail) && IsTextureLoaded(Detail))
	{
		// Portal 2's Panel Texture uses $DetailBlendMode 10 with an SSBump, however official Stock Shaders don't allow this.
		// It would only work if not a SSBump. LUX Adds support for it, so consider that!
		if (GetTexture(Detail)->GetFlags() & TEXTUREFLAGS_SSBUMP)
		{
			if (bHasBumpMap)
				SetInt(DetailBlendMode, 10);
			else
				SetInt(DetailBlendMode, 11);
		}
	}
}

SHADER_DRAW
{
	// NOTE: We already made sure we don't have conflicting flags on Shader Init ( see above )
	bool bHasFlashlight = HasFlashlight();
	
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bHasSSBump = bHasNormalTexture && GetBool(SSBump);

	bool bHasReflectTexture = IsTextureLoaded(ReflectTexture);
	bool bHasRefractTexture = IsTextureLoaded(RefractTexture);
	bool bReflectLightScale = bHasReflectTexture && GetBool(ReflectLightModulation);
	bool bNeedsLightmap = (bHasBaseTexture || bReflectLightScale);
	bool bHasEnvMapMask = IsTextureLoaded(EnvMapMask);

	// FIXME: Right now only allowing Detail with BaseTexture
	// But DetailBlendMode 5 could be used without one..
	bool bHasDetailTexture = IsTextureLoaded(Detail) && bHasBaseTexture;

	bool bHasBaseTextureTransform = HasTransform(bHasBaseTexture, BaseTextureTransform);
	bool bEnvMapMaskTransform = HasTransform(bHasEnvMapMask, EnvMapMaskTransform);
	bool bNormalTextureTransform = HasTransform(bHasNormalTexture, BumpTransform);

	int nDetailBlendMode = 0;
	bool bDetailTextureTransform = false;
	if (bHasDetailTexture)
	{
		bDetailTextureTransform = HasTransform(true, DetailTextureTransform);
		nDetailBlendMode = GetInt(DetailBlendMode);
	}

	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(nBlendType);

		// We want Fog to Fog
		FogToFogColor();

		// We always need this
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Not currently using Tangent Spaces but Stock Shader always did for ViewDirTS
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_SPACE;

		// *sigh* Another Hammer specific Issue
		// Vertex Colors are used for Shaded Texture Polygons..
		if (HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			nFlags |= VERTEX_COLOR;

		// 1 Texcoord  = UV
		// 2 Texcoords = UV + Lightmap UV
		// 3 Texcoords = UV + Lightmap UV + Bumped Lightmap UV Offset
		int nTexCoords = bNeedsLightmap ? 3 : 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture
		// NOTE: Stock Shader has sRGBRead *OFF*, I'm not replicating that!
		EnableSampler(bHasBaseTexture, SAMPLER_BASETEXTURE, true);

		// s1 - $ReflectTexture. Always sRGB
		EnableSampler(bHasReflectTexture, SHADER_SAMPLER1, true);

		// s2 - $NormalMap / $BumpMap. Never sRGB
		EnableSampler(SAMPLER_NORMALMAP, false);
		
		// s3 - $RefractTexture. Always sRGB
		EnableSampler(bHasRefractTexture, SHADER_SAMPLER3, true);
		
		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false); // Stock Consistency, no sRGB

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		// s11 - Lightmap
		// H++ only runs LDR, so we have to consider that as well
		EnableSampler(bNeedsLightmap, SAMPLER_LIGHTMAP, !IsHDREnabled());

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		int nNeededTexCoords = 2; // Always need a BaseTexture and NormalMap Coordinate
		nNeededTexCoords += bHasDetailTexture;
		nNeededTexCoords += bHasEnvMapMask;

		if (bHasFlashlight)
		{
			// Reuse the Base LightmappedGeneric Shaders for this
			// Don't need anything different for the Flashlightpasses
			DECLARE_STATIC_VERTEX_SHADER(lux_brush_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bHasNormalTexture);
			SET_STATIC_VERTEX_SHADER_COMBO(BUMPED_LIGHTMAP, false);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, HasFlag(MATERIAL_VAR_VERTEXCOLOR));
			SET_STATIC_VERTEX_SHADER(lux_brush_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			int nLightingMode = bHasNormalTexture + bHasSSBump;
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTING_MODE, nLightingMode);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, 0); // NOPE
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else
		{
			bool bSelfIllumEMMAlpha = bHasEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha);
			DECLARE_STATIC_VERTEX_SHADER(lux_lightmappedreflective_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, bNeedsLightmap);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, HasFlag(MATERIAL_VAR_VERTEXCOLOR));
			SET_STATIC_VERTEX_SHADER(lux_lightmappedreflective_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedreflective_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSETEXTURE, bHasBaseTexture + bHasSSBump * bHasBaseTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(EMM, bHasEnvMapMask + bSelfIllumEMMAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(EFFECTMODE, bHasReflectTexture + bHasRefractTexture * 2); // Reflect, Refract, Both
			// Duplicate SSBump Scenario, DiffuseTexture(2) would be SSBump already here, so we skip ReflectionLightScale(2) which is also SSBump
			SET_STATIC_PIXEL_SHADER_COMBO(REFLECTIONLIGHTSCALE, bReflectLightScale + bHasSSBump * bReflectLightScale * !bHasBaseTexture);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedreflective_ps30);
		}
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture
		BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_WHITE);

		// s1 - $ReflectTexture
		BindTexture(bHasReflectTexture, SHADER_SAMPLER1, ReflectTexture, -1);

		// s2 - $BumpMap/$NormalMap
		BindTexture(bHasNormalTexture, SAMPLER_NORMALMAP, BumpMap, BumpFrame, TEXTURE_NORMALMAP_FLAT);

		// s3 - $RefractTexture
		BindTexture(bHasRefractTexture, SHADER_SAMPLER3, RefractTexture, -1);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5 - $EnvMapMask
		BindTexture(bHasEnvMapMask, SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s11 - Lightmap
		BindTexture(bNeedsLightmap, SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		//==================================================================================================
		// Setup Constant Registers
		//==================================================================================================

		// VS c223, c224
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// VS c225, c226
		if (bNormalTextureTransform)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform);
		else if (bHasNormalTexture && bHasBaseTextureTransform)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform);
		
		// VS c227, c228
		if (bEnvMapMaskTransform)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, EnvMapMaskTransform);
		else if (bHasEnvMapMask)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, BaseTextureTransform);

		// VS c229, c230
		if(bHasDetailTexture)
		{
			// Move up the Registers if there is an EnvMapMask
			int nDetailTransformReg = bHasEnvMapMask ? LUX_VS_TEXTURETRANSFORM_04 : LUX_VS_TEXTURETRANSFORM_03;
			if (bDetailTextureTransform)
				SetVertexShaderTextureScaledTransform(nDetailTransformReg, DetailTextureTransform, DetailScale);
			else
				SetVertexShaderTextureScaledTransform(nDetailTransformReg, BaseTextureTransform, DetailScale);			
		}

		// c1 - Modulation Constant
		// Function above, handles LightmapScaleFactor and Alpha Modulation
		SetModulationConstant(bHasSSBump && GetBool(SSBumpMathFix));
			
		// c3 - $ReflectTint & $Reflectance
		// Stock-Consistency: GammaToLinear
		SetPixelShaderConstantGammaToLinear(REGISTER_FLOAT_003, ReflectTint, Reflectance);

		// c4 - $RefractTint
		// Stock-Consistency: GammaToLinear
		SetPixelShaderConstantGammaToLinear(REGISTER_FLOAT_004, RefractTint);

		// c5 - $ReflectAmount and $RefractAmount
		float4 f4ReflectRefractScale;
		f4ReflectRefractScale.x = GetFloat(ReflectAmount);
		f4ReflectRefractScale.y = f4ReflectRefractScale.x;
		f4ReflectRefractScale.z = GetFloat(RefractAmount);
		f4ReflectRefractScale.w = f4ReflectRefractScale.z;
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_005, f4ReflectRefractScale);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
		
		// c31 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);
	
		// c33, c34
		if (bHasDetailTexture)
		{
			float4 f4Tint_Factor;
			f4Tint_Factor.rgb = GetFloat3(DetailTint);
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4Tint_Factor);

			float4 f4Blendmode = 0.0f;
			f4Blendmode.x = (float)nDetailBlendMode;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4Blendmode);
		}

		// c35
		if (bHasEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha))
		{
			float4 f4SelfIllumTint = 0.0f;
			f4SelfIllumTint.xyz = GetFloat3(SelfIllumTint);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FACTORS, f4SelfIllumTint);
		}

		// This sets up flashlight  constants and returns bFlashlightShadow
		bool bFlashlightShadows = SetupFlashlight();

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b12
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);

		if (bHasFlashlight)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_lightmappedreflective_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_lightmappedreflective_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedreflective_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, r_lightmap_bicubic.GetBool());
			SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedreflective_ps30);
		}
	}

	//==========================================================================//
	// ConVars
	//==========================================================================//
	if(IsDynamicState())
	{
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		{
			BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, TEXTURE_GREY);
		}
#endif

#ifdef LUX_DEBUGCONVARS
		if (lux_disablefast_diffuse.GetBool())
		{
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
		}

		if (bHasNormalTexture && lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		}

		if (bNeedsLightmap && lux_disablefast_lightmap.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
		}

		// Cannot disable EnvMapMask Alpha via ConVar ( lux_disablefast_selfillum )
		#endif

#ifdef DEBUG_LUXELS
		if (bNeedsLightmap && mat_luxels.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
		}
#endif
	}

	Draw();

	if (HasFlag(MATERIAL_VAR_ALPHATEST))
	{
		// "Alpha testing makes it so we can't write to dest alpha
		// Writing to depth makes it so later polygons can't write to dest alpha either
		// This leads to situations with garbage in dest alpha."
		// "Fix it now by converting depth to dest alpha for any pixels that just wrote."
		// ShiroDkxtro2 :	What this does is basically render the model again but replace the alpha
		//					with the alpha we *actually* wanted. This stalls quite a bit
		DrawEqualDepthToDestAlpha();
	}
}
END_SHADER