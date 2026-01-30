//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Shader Reference taken from Alien Swarm's Materialsystem
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_shatteredglass_vs30.inc"
#include "lux_shatteredglass_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_ShatteredGlass, LUX_ShatteredGlass)
#endif

#ifdef REPLACE_SHATTEREDGLASS
DEFINE_FALLBACK_SHADER(ShatteredGlass, LUX_ShatteredGlass)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_ShatteredGlass, "A shader used to simulate realtime breaking glass." )
SHADER_INFO_GEOMETRY	("func_breakable_surf.")
SHADER_INFO_USAGE		("https://developer.valvesoftware.com/wiki/Making_a_custom_breakable_surface")
SHADER_INFO_LIMITATIONS	("Can only be applied to a func_breakable_surf.")
SHADER_INFO_PERFORMANCE	("Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	// Detail and EnvMap were originally supported by this Shader
	// Lets expand on that with all the other Parameters!
	Declare_DetailTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_ParallaxCorrectionParameters()
	Declare_EnvMapMaskParameters()
	SHADER_PARAM(UnlitFactor, SHADER_PARAM_TYPE_FLOAT, "", "0.0 == Use Lightmap, 1.0 == Unlit")	
END_SHADER_PARAMS

// Set Up Vars here

SHADER_INIT_PARAMS()
{
	// Instead of making the Parameter uneditable
	// Simply set a Default instead
	if (!IsDefined(BaseTexture))
		SetString(BaseTexture, "Glass/glasswindowbreak070b");

	// Unlike other Shaders, it's 1.0f here. Stock-Consistency
	DefaultFloat(DetailScale, 1.0f);
	DefaultFloat(DetailBlendFactor, 1.0f);

	// Shader actually sends 1.0f - $UnlitFactor. So this is technically 0.7f unlit
	DefaultFloat(UnlitFactor, 0.3f);

	// 1.0f = Mirror so no Fresnel
	DefaultFloat(FresnelReflection, 1.0f);

	// Full Saturation
	DefaultFloat(EnvMapSaturation, 1.0f);

	if (IsDefined(EnvMap))
	{
		if (!g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		{
			SetUndefined(EnvMap);
			SetBool(EnvMapParallax, false);
		}

		// Stock-Consistency: Flipped EnvMapMask with $BaseAlphaEnvMapMask
		if (!IsDefined(EnvMapMaskFlip) && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
		{
			SetBool(EnvMapMaskFlip, true);
		}
	}

	if (!IsDefined(BaseTexture))
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);

	// If in decal mode, no debug override...
	if (HasFlag(MATERIAL_VAR_DECAL))
		SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

	// NOTE: Stock Shader does not set this Flag but Lightmaps are still available?
//	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

	// Projected Texture not supported.
	SetBool(ReceiveProjectedTextures, false);
}

SHADER_FALLBACK
{
#ifndef REPLACE_SHATTEREDGLASS
	if (lux_oldshaders.GetBool())
		return "ShatteredGlass";
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
	if (IsDefined(BaseTexture))
	{
		// LoadTexture specifically, we already checked for define
		LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);

		if (!GetTexture(BaseTexture)->IsTranslucent())
		{
			if (HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
				ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}
	}

	// Don't alpha test if the alpha channel is used for other purposes
	if (HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
		ClearFlag(MATERIAL_VAR_ALPHATEST);

	// NOTE: Detail is always sRGB
	LoadTexture(Detail, TEXTUREFLAGS_SRGB);

	LoadCubeMap(EnvMap, 0);

	// This big block of if-statements is to determine if we even have any envmapmasking.
	// We don't want EnvMapMasking if we don't even have an envmap
	if (IsDefined(EnvMap))
	{
		// $EnvMapMask has Priority over other Masking
		if (IsDefined(EnvMapMask))
		{
			// NOTE: Stock uses TEXTUREFLAGS_SRGB
			// Sampler is not sRGB, so don't do that.
			LoadTexture(EnvMapMask);

			// We already have an $EnvMapMask, so remove these!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}

		// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
		// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
		// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
		// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
		if(IsDefined(EnvMapParallaxOBB1) && !GetBool(EnvMapParallax))
			DefaultBool(EnvMapParallax, true);
	}
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasDetailTexture = IsTextureLoaded(Detail);
	bool bHasEnvMap = IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bBaseAlphaEnvMapMask = bHasEnvMap && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Alphatest Support
		if (HasFlag(MATERIAL_VAR_ALPHATEST))
		{
			pShaderShadow->EnableAlphaTest(true); // bAlphatest
			
			// AlphaTestReference not originally supported, so bring it here
			if (GetFloat(AlphaTestReference) > 0.0f) // 0 is default.
			{
				//	pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_EQUAL, GetFloat(AlphaTestReference));
				pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, GetFloat(AlphaTestReference));
			}
		}
		else
		{
			// Stock-Consistency, replicate original transparency behaviours
			// NOTE: BT_BLEND_ADD instead of regular BT_ADD
			bool bDetailIsTranslucent = TextureIsTranslucent(Detail, false);
			if (bDetailIsTranslucent)
			{
				// "Alpha blending, enable alpha blending if the detail Texture is translucent"
				if (HasFlag(MATERIAL_VAR_ADDITIVE))
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE);
				else
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				SetDefaultBlendingShadowState(BaseTexture, true);
			}
		}

		// We always output linear Values
		pShaderShadow->EnableSRGBWrite(true);

		// DO NOT Write Alpha, this Shader is translucent
		pShaderShadow->EnableAlphaWrites(false);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		unsigned int nFlags = VERTEX_POSITION;

		if (bHasEnvMap)
			nFlags |= VERTEX_NORMAL;

		// This might cause some Problems!!
		// The Shader does NOT use input Vertex Color, it outputs cModulationColor
		// So this is pointless!!
		// Not having it might mess with the Vertex Format though!
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			nFlags |= VERTEX_COLOR;

		// Shader always asks for 3 TexCoords
		// TexCoord0 = Fractured Glass BaseTexture UV ( Per Panel )
		// TexCoord1 = Lightmap UV
		// TexCoord2 = Actual UV ( Across the Surface )
		int nTexCoords = 3;

		// No Tangents..
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture
		EnableSampler(SAMPLER_BASETEXTURE, true);

		// s4 - $Detail. Stock-Consistency: Always sRGB
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, true);

		// s5 - $EnvMapMask. Not sRGB
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		// s11 - Lightmap. sRGB for LDR
		// Lightmap is sRGB in LDR, Linear in HDR
		EnableSampler(SAMPLER_LIGHTMAP, !IsHDREnabled());

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		int nNeededTexCoords = bHasDetailTexture + bHasEnvMapMask;
		DECLARE_STATIC_VERTEX_SHADER(lux_shatteredglass_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
		SET_STATIC_VERTEX_SHADER_COMBO(ENVMAP, bHasEnvMap);
		SET_STATIC_VERTEX_SHADER(lux_shatteredglass_vs30);

		bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR);
		int nEnvMapMode = bPCC * 2 + bHasEnvMapMask + bHasEnvMap;
		DECLARE_STATIC_PIXEL_SHADER(lux_shatteredglass_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
		SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
		SET_STATIC_PIXEL_SHADER(lux_shatteredglass_ps30);
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
		if(!bHasBaseTexture && bHasEnvMap)
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);		
		else if(!bHasBaseTexture)
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_WHITE);
		else
			BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5 - $EnvMapMask
		BindTexture(bHasEnvMapMask, SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s11 - Lightmap
		BindTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS c223, c224
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// VS c225, c226
		if (bHasEnvMapMask)
		{
			// Stock-Consistency, ASW Code always does DetailTextureTransform on it
			if (HasTransform(bHasEnvMapMask, EnvMapMaskTransform))
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, EnvMapMaskTransform, DetailScale);
			else
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, DetailTextureTransform, DetailScale);
		}

		// VS c225, c226 or c227, c228
		if(bHasDetailTexture)
		{
			int nDetailTexCoordRegister = bHasEnvMapMask ? LUX_VS_TEXTURETRANSFORM_03 : LUX_VS_TEXTURETRANSFORM_02;
			SetVertexShaderTextureScaledTransform(nDetailTexCoordRegister, DetailTextureTransform, DetailScale);
		}

		// c0
		float4 f4OverbrightFactors = 0.0f;
		f4OverbrightFactors.x = 2.0f;
		f4OverbrightFactors.y = GetFloat(UnlitFactor);
		f4OverbrightFactors.z = 1.0f - GetFloat(UnlitFactor);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4OverbrightFactors);

		// c1 - Modulation Constant
		// Function above, handles LightmapScaleFactor and Alpha Modulation
		SetModulationConstant();
			
		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
		
		// c32 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// c33 - $DetailTint, $DetailBlendFactor
		if (bHasDetailTexture)
		{
			float4 f4DetailTint;
			f4DetailTint.rgb = GetFloat3(DetailTint);
			f4DetailTint.w = GetFloat(DetailBlendFactor);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4DetailTint);
		}

		// c37, c38, c39, c40, c41, c42, c43, c44
		if (bHasEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes, this *is* a float3 Parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// $BaseAlphaEnvMapMask, $NormalMapAlphaEnvMapMask, $EnvMapMaskFlip
			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = 0.0f;
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip); // applied as abs($EnvMapMaskFlip - EnvMapMask)
			f4EnvMapControls.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// ~~$EnvMapFresnelMinMaxExp~~
			// Scratch that! Stock-Consistency
			// Shader Expects : Scale, Bias, Exponent
			float4 f4EnvMapFresnelRanges = 0.0f;
			f4EnvMapFresnelRanges.z = 5.0f; // Pow of 5
			f4EnvMapFresnelRanges.y = GetFloat(FresnelReflection); // Bias
			f4EnvMapFresnelRanges.x = 1.0f - f4EnvMapFresnelRanges.y; // Scale
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);

			// $EnvMapOrigin, $EnvMapParallaxOBB1, $EnvMapParallaxOBB2, $EnvMapParallaxOBB3
			if(bPCC)
			{
				float4 f4EnvMapOrigin = 0.0f;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX, f4Row1);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_2, f4Row2);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_3, f4Row3);
			}
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
		// b13, b14, b15
		// Never Opaque!
		if (true)
		{
			BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false); // Heightfog instead of Range/Radial Fog
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(false);
		}

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_shatteredglass_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_shatteredglass_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_shatteredglass_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_shatteredglass_ps30);
	}

	//==========================================================================//
	// ConVars
	//==========================================================================//
	if(IsDynamicState())
	{
		// mat_fullbright 2 binds a standard grey Texture...
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		{
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		}
#endif

#ifdef LUX_DEBUGCONVARS
		if(lux_disablefast_diffuse.GetBool())
		{
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);		
		}

		if (lux_disablefast_lightmap.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_WHITE);
		}

		if (lux_disablefast_envmap.GetBool())
		{
			BindTexture(SAMPLER_ENVMAPTEXTURE, TEXTURE_BLACK);
		}
#endif

#ifdef DEBUG_LUXELS
		if (mat_luxels.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
		}
#endif
	}

	Draw();
}
END_SHADER