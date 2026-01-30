//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_brush_decal_vs30.inc"
#include "lux_lightmappedgeneric_decal_ps30.inc"
#include "lux_lightmappedgeneric_flashlight_ps30.inc"

#ifdef REPLACE_LIGHTMAPPEDGENERIC_DECAL
DEFINE_FALLBACK_SHADER(LightmappedGeneric_Decal, LUX_LightmappedGeneric_Decal)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
// ShiroDkxtro2 NOTE: LUX_LightmappedGeneric will redirect to this Shader whenever the Decal flag is used
// I decided this should be done because Decals work drastically different from regular brushes
// For example, they can't receive Tangents on their own, so you have to compute them yourself
// LightmapTexcoord Offset is also very whacky
BEGIN_VS_SHADER(LUX_LightmappedGeneric_Decal, "A shader meant to be used for overlays.")
SHADER_INFO_GEOMETRY	("Decals but primarily Overlays.")
SHADER_INFO_USAGE		("Select a Material and create an Overlay.")
SHADER_INFO_LIMITATIONS	("BumpMaps only work with Overlays. This Shader cannot have EnvMaps.")
SHADER_INFO_PERFORMANCE	("Cheap to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_NormalTextureParameters()
	Declare_SelfIlluminationParameters()
	Declare_DetailTextureParameters()
	Declare_SelfIllumTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_MiscParameters()

END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Scale, Bias Exponent
	DefaultFloat3(EnvMapFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);
	DefaultFloat3(SelfIllumFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);

	DefaultFloat(DetailBlendFactor, 1.0f);
	DefaultFloat(DetailScale, 4.0f); 

	if(IsDefined(SelfIllumMask))
		DefaultFloat(SelfIllumMaskScale, 1.0f);

	// If in Decal Mode, no debug override...
	// And we are always in Decal Mode on this Shader
	SetFlag(MATERIAL_VAR_DECAL);
	SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

	// No BaseTexture? None of these.
	if (!IsDefined(BaseTexture))
	{
		ClearFlag(MATERIAL_VAR_SELFILLUM);
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
	if (g_pConfig->UseBumpmapping() && IsDefined(BumpMap))
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);

	// Default Value is supposed to be 1.0f
	DefaultFloat(EnvMapSaturation, 1.0f);

	if (IsDefined(EnvMap) && !g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		SetUndefined(EnvMap);

	// Tell the Shader to flip the EnvMap when set on AlphaEnvMapMask ( Consistency with Stock Shaders )
	if (!IsDefined(EnvMapMaskFlip) && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
		SetBool(EnvMapMaskFlip, true);

	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
}

SHADER_FALLBACK
{
	// LMG is the default fallback here
#ifndef REPLACE_LIGHTMAPPEDGENERIC
	if (lux_oldshaders.GetBool())
		return "LightmappedGeneric";
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
	// We don't check for Transparency we did so earlier and do so later...
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadBumpMap(BumpMap);

	// Stock-Consistency
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap) && IsTextureLoaded(BumpMap))
	{
		bHasBumpMap = true;
		ITexture *pBumpMap = GetTexture(BumpMap);
		if (pBumpMap->GetFlags() & TEXTUREFLAGS_SSBUMP ? true : false)
		{
			SetBool(SSBump, true);
		}
	}
	else
	{
		SetBool(SSBump, false);
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

	LoadTexture(SelfIllumMask);
	LoadTexture(SelfIllumTexture, TEXTUREFLAGS_SRGB);

	LoadCubeMap(EnvMap);

	if (IsDefined(EnvMap))
	{
		// $EnvMapMask has Priority over other Masking
		if (IsDefined(EnvMapMask))
		{
			LoadTexture(EnvMapMask);

			// We already have an $EnvMapMask, so remove these!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		else
		{
			// NormalMapAlphaEnvMapMask takes priority, because its the go to one
			if (IsDefined(BumpMap) && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
			{
				// If we use normal map alpha, don't use basetexture alpha.
				ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			}
			else if (!IsDefined(BaseTexture))
			{
				// If we have no Basetexture, can't use its alpha.
				ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			}
		}
	}

	// No Alphatesting/blendtint when we use the Alpha for other things
	// Valves Shaders ignore the fact you can use $envmapmask's alpha for selfillum...
	if ((HasFlag(MATERIAL_VAR_SELFILLUM) && !GetBool(SelfIllum_EnvMapMask_Alpha)))
	{
		ClearFlag(MATERIAL_VAR_ALPHATEST);
		SetBool(BlendTintByBaseAlpha, false);
	}
}

SHADER_DRAW
{
	// Check Flashlight first, important for a lot of other parameters
	bool bProjTex = HasFlashlight();

	// Texture related Boolean. We check for existing bools first because its faster
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bHasDetailTexture = IsTextureLoaded(Detail);
	int  nDetailBlendMode = GetInt(DetailBlendMode);

	// Important difference here between Has and Use EnvMap
	// If we have an EnvMap we apply everything but if we don't USE it, we apply a black Texture instead
	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bBaseAlphaEnvMapMask = bHasEnvMap && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bNormalMapAlphaEnvMapMask = bHasEnvMap && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
	bool bHasEnvMapFresnel = bHasEnvMap && GetBool(EnvMapFresnel);

	// BaseAlpha Modes
	bool bBlendTintByBaseAlpha = GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = !bBlendTintByBaseAlpha && GetBool(DesaturateWithBaseAlpha);

	// SelfIllum
	bool bSelfIllum = !bProjTex && HasFlag(MATERIAL_VAR_SELFILLUM);
	bool bHasSelfIllumMask = bSelfIllum && IsTextureLoaded(SelfIllumMask);

	// Others
	bool bHasVertexColor = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);
	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));

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

		// Never write Alpha. Surface we apply to already did this!
		pShaderShadow->EnableAlphaWrites(false);

		// Writing Linear Values that need to be converted
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Valve's shader does not check for this, which is odd.
		// Aka no VertexAlpha if not also using VertexColor? Is that a "bug"? Should we enable that Behaviour?
		// HasFlag(MATERIAL_VAR_VERTEXALPHA)

		// Just always ask for Normal... You pretty much need it 99% of the time
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL;

		if (bHasVertexColor)
			nFlags |= VERTEX_COLOR;

		// No Lightmap UV when using projected Textures
		// 1 TexCoord  = UV
		// 2 TexCoords = UV + Lightmap UV
		// 3 TexCoords = UV + Lightmap UV + Bumped Lightmap UV Offset
		int nTexCoords = bProjTex ? 1 : 2;
		if(!bProjTex && bHasNormalTexture) 
			nTexCoords = 3;

		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. Always sRGB
		EnableSampler(SAMPLER_BASETEXTURE, true);
		
		// s2 - $BumpMap. Never sRGB
		EnableSampler(bHasNormalTexture, SAMPLER_NORMALMAP, false);

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask. Not sRGB.
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		// s11 - Lightmap. sRGB when LDR
		EnableSampler(!bProjTex, SAMPLER_LIGHTMAP, !IsHDREnabled());

		// s13 - $SelfIllumMask. Never sRGB
		EnableSampler(bHasSelfIllumMask, SAMPLER_SELFILLUM, false);

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		int nNeededTexCoords = bHasDetailTexture + bHasEnvMapMask + bHasNormalTexture;
		DECLARE_STATIC_VERTEX_SHADER(lux_brush_decal_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColor);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
		SET_STATIC_VERTEX_SHADER_COMBO(BUMPMAPPED, bHasNormalTexture);
		SET_STATIC_VERTEX_SHADER(lux_brush_decal_vs30);

		if (bProjTex)
		{
			// We don't have Tangents so don't do Bump Mapping with the Flashlight
			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTING_MODE, 0);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else
		{
			int nEnvMapMode = bHasEnvMapMask ? 2 : bHasEnvMap;
			int nSelfIllumMode = bSelfIllum + (bHasEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha));

			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_decal_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(BUMPMAPPED, bHasNormalTexture + GetBool(SSBump));
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_decal_ps30);
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

		// s2 - $BumpMap
		BindTexture(bHasNormalTexture, SAMPLER_NORMALMAP, BumpMap, BumpFrame);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5 - $EnvMapMask
		BindTexture(bHasEnvMapMask, SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s11 - Lightmap. Decals kinda don't have them but we get a White one if we do
		// And there is no way to differentiate between Overlays and Decals
		BindTexture(!bProjTex, SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		// s13 - $SelfIllumMask
		BindTexture(bHasSelfIllumMask, SAMPLER_SELFILLUM, SelfIllumMask, SelfIllumMaskFrame);
		
		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		//==================================================================================================
		// Setup Constant Registers
		//==================================================================================================

		// VS c223, c224 - $BaseTextureTransform
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nRegisterShift = 0;
		if(bHasNormalTexture)
		{
			bool bNormalTextureTransform = HasTransform(bHasNormalTexture, BumpTransform);
			if (bNormalTextureTransform)
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform);
			else if (bHasNormalTexture)
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform);

			nRegisterShift += 2;
		}

		if(bHasEnvMapMask)
		{
			bool bEnvMapMaskTransform = HasTransform(bHasEnvMapMask, EnvMapMaskTransform);
			if (bEnvMapMaskTransform)
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, EnvMapMaskTransform);
			else if (bHasNormalTexture)
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform);

			nRegisterShift += 2;
		}

		if(bHasDetailTexture)
		{
			bool bDetailTextureTransform = HasTransform(bHasDetailTexture, DetailTextureTransform);
			if (bDetailTextureTransform)
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, DetailTextureTransform, DetailScale);
			else
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform, DetailScale);
		}

		// c1 - Modulation Constant
		// Function above, handles LightmapScaleFactor and Alpha Modulation
		SetModulationConstant(GetBool(SSBumpMathFix));
				
		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
		
		// c32 - $Color, $Color2, $sRGBTint
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

		// c35, c36
		if (bSelfIllum)
		{
			float4 f4SelfIllumTint_Scale = 0.0f;
			f4SelfIllumTint_Scale.xyz = GetFloat3(SelfIllumTint);
			f4SelfIllumTint_Scale.w = GetFloat(SelfIllumMaskScale);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FACTORS, f4SelfIllumTint_Scale);

			float4 f4SelfIllumFresnelMinMaxExp = 0.0f;
			if(GetBool(SelfIllumFresnel))
				f4SelfIllumFresnelMinMaxExp.xyz = GetFloat3(SelfIllumFresnelMinMaxExp); // .w empty
			else
				f4SelfIllumFresnelMinMaxExp.y = 1.0f; // This will disable SelfIllumFresnel
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FRESNEL, f4SelfIllumFresnelMinMaxExp);
		}

		// c37 c38 c39 c40
		if (bHasEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes. Yes this is a vec3 parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// $BaseAlphaEnvMapMask, $NormalMapAlphaEnvMapMask, $EnvMapMaskFlip
			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip); // applied as abs($EnvMapMaskFlip - EnvMapMask)
			f4EnvMapControls.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// $EnvMapFresnelMinMaxExp
			float4 f4EnvMapFresnelRanges = 0.0f;
			if(bHasEnvMapFresnel)
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
			else
				f4EnvMapFresnelRanges.y = 1.0f; // This will disable EnvMapFresnel

			// Always set this due to how EnvMapFresnel is setup in the Shader
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);
		}

		// This sets up flashlight  constants and returns bFlashlightShadow
		bool bFlashlightShadows = SetupFlashlight();

		//==================================================================================================
		// Setup Vertex Shader Constant Registers
		//==================================================================================================

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b12
		if(bHasVertexColor)
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		// Never draw to Alpha! Surface we draw to already did it.
		BBools[LUX_PS_BOOL_HEIGHTFOG] = false; 
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = false;

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_decal_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_brush_decal_vs30);

		if (bProjTex)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_decal_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, r_lightmap_bicubic.GetBool());
			SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_decal_ps30);
		}
	} 

	if(IsDynamicState())
	{
		#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		{
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		}
		#endif

		#ifdef LUX_DEBUGCONVARS
		if (lux_disablefast_diffuse.GetBool())
		{
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);		
		}

		if (lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		}

		if (lux_disablefast_lightmap.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
		}

		if (lux_disablefast_envmap.GetBool())
		{
			BindTexture(SAMPLER_ENVMAPTEXTURE, TEXTURE_BLACK);
		}
		#endif
	}

	Draw();

	// Don't DrawEqualDepthToDestAlpha here
	// The Surface we are applying this Decal too should have already done so!!
}
END_SHADER