//===================== File of the LUX Shader Project =====================//
//
//	Original D.	:	26.03.2025 DMY
//	Initial D.	:	28.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "Triplanar.h"

// Includes for Shaderfiles...
#include "lux_model_simplified_vs30.inc"
#include "lux_model_vs30.inc"
#include "lux_model_bump_vs30.inc"
#include "lux_triplanar_model_ps30.inc"
#include "lux_triplanar_model_flashlight_ps30.inc"

//==========================================================================//
// Triplanar for Model based Geometry
//==========================================================================//
BEGIN_VS_SHADER(LUX_Triplanar_Model, "Pretty much VertexLitGeneric but with Triplanar Mapping.")
SHADER_INFO_GEOMETRY	("Models")
SHADER_INFO_USAGE		("Use at least one Triplanar Method.")
SHADER_INFO_LIMITATIONS	("Does not support Phong.\n"
						 "Does not support $LightWarpTexture.\n"
						 "Does not support $Lightmap ( Model Lightmaps ).\n"
						 "Does not support $EnvMapSphere.\n"
						 "Does not support $TreeSway.\n"
						 "Does not support $SelfIllum. Does support $DetailBlendMode 5.\n"
						 "Does not support a bunch of other Things compared to VertexLitGeneric.\n"
						 "However a lot more Features than Stock.")
SHADER_INFO_PERFORMANCE	("Expensive due to multiple Texture Samples for each Triplanar Texture.\n"
						 "The more Triplanar Methods are applied the more expensive it becomes.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"Reference $Seamless related Parameters.")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_NormalTextureParameters()
	Declare_DetailTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_TriplanarParameters()
	SHADER_PARAM(TriPlanar_Count, SHADER_PARAM_TYPE_INTEGER,	"",		"(INTERNAL PARAMETER), dont use!")

	// Stock Parameters
	SHADER_PARAM(Seamless_Base,	SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER), dont use!")
	SHADER_PARAM(Seamless_Detail, SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER), dont use!")
	SHADER_PARAM(Seamless_Scale, SHADER_PARAM_TYPE_FLOAT, "", "(INTERNAL PARAMETER), dont use!")
END_SHADER_PARAMS

// Account for Changes to this in the other Triplanar Mapping Files, thanks.
void HandleFallback()
{
	bool bModelShader = true;

	// Seamless Fallback to Triplanar Routing
	bool bSkipTriplanarRouting = false;
	if (true)
	{
		// Stock-Consistency:
		// Using Seamless_Scale will enable $Seamless_Base
		// **Not on VLG**. It demands you use $Seamless_Base instead
		if (!bModelShader && IsDefined(Seamless_Scale) && GetFloat(Seamless_Scale) != 0.0f)
		{
			SetBool(TriPlanar_Base, true);
			SetBool(TriPlanar_Bump, true);
			SetBool(TriPlanar_EnvMapMask, true);
			bSkipTriplanarRouting = true;
		}

		if (GetBool(Seamless_Base))
		{
			SetBool(TriPlanar_Base, true);
			SetBool(TriPlanar_Bump, true);
			SetBool(TriPlanar_EnvMapMask, true);
			bSkipTriplanarRouting = true;
		}

		if (GetBool(Seamless_Detail))
		{
			SetBool(TriPlanar_Detail, true);
			bSkipTriplanarRouting = true;
		}
	}

	// Triplanar Routing
	// When we fallback from another Shaders, don't Route!
	if (!bSkipTriplanarRouting)
	{
		if (GetBool(TriPlanar_Base))
		{
			if (!IsDefined(TriPlanar_Detail))
				SetBool(TriPlanar_Detail, true);

			if (!IsDefined(TriPlanar_Bump))
				SetBool(TriPlanar_Bump, true);

			if (!IsDefined(TriPlanar_EnvMapMask))
				SetBool(TriPlanar_EnvMapMask, true);
		}
	}
	else // Some Default Behaviours we have to reproduce
	{
		// Seamless Base means Seamless EnvMapMask and Seamless Bump
		// Not the case with Seamless_Detail though
		if (GetBool(TriPlanar_Base))
		{
			SetBool(TriPlanar_Bump, true);
			SetBool(TriPlanar_EnvMapMask, true);
		}
	}

	// Sum up how much Triplanar we have right now
	int nTriplanarCount = GetBool(TriPlanar_Base) + GetBool(TriPlanar_Detail) + GetBool(TriPlanar_Bump) + GetBool(TriPlanar_EnvMapMask);
	if (nTriplanarCount == 0)
	{
		ShaderDebugMessage("a Triplanar Material does not use any Triplanar methods. Falling back to a suited Stock Shader.\n");
	}
	SetInt(TriPlanar_Count, nTriplanarCount);

	// Copy Scales from Seamless Mapping
	if (bSkipTriplanarRouting)
	{
		float3 Scale;
		Scale = GetFloat(Seamless_Scale);

		if (GetBool(Seamless_Base))
		{
			SetFloat3(TriPlanar_Scale_Base, Scale);
			SetFloat3(TriPlanar_Scale_Bump, Scale);
			SetFloat3(TriPlanar_Scale_EnvMapMask, Scale);
		}
		else if (GetBool(Seamless_Detail))
		{
			SetFloat3(TriPlanar_Scale_Detail, Scale);
		}
	}
	else
	{
		DefaultFloat3(TriPlanar_Scale_Base, 1.0f, 1.0f, 1.0f);
		DefaultFloat3(TriPlanar_Scale_Detail, 1.0f, 1.0f, 1.0f);
		DefaultFloat3(TriPlanar_Scale_Bump, 1.0f, 1.0f, 1.0f);
		DefaultFloat3(TriPlanar_Scale_EnvMapMask, 1.0f, 1.0f, 1.0f);

		DefaultFloat3(TriPlanar_Offset_Base, 0.0f, 0.0f, 0.0f);
		DefaultFloat3(TriPlanar_Offset_Detail, 0.0f, 0.0f, 0.0f);
		DefaultFloat3(TriPlanar_Offset_Bump, 0.0f, 0.0f, 0.0f);
		DefaultFloat3(TriPlanar_Offset_EnvMapMask, 0.0f, 0.0f, 0.0f);
	}
}

void SetTriplanarModelFlags()
{
	// HW Skinning is supported.
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// Always need Lighting
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

	// Do we actually have to set this?
	SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);

	if (IsDefined(BumpMap))
	{
		SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
		SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
	}
}

SHADER_INIT_PARAMS()
{
	SetTriplanarModelFlags();
	HandleFallback();

	if (IsDefined(EnvMap))
	{
		if (!g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		{
			SetUndefined(EnvMap);
		}

		DefaultFloat(EnvMapSaturation, 1.0f);

		// Scale, Bias Exponent
		DefaultFloat3(EnvMapFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);
	}

	if(IsDefined(Detail))
	{
		DefaultFloat(DetailBlendFactor, 1.0f);
		DefaultFloat(DetailScale, 4.0f);
	}

	// No BaseTexture means you can't have any of these.
	if (!IsDefined(BaseTexture))
	{
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		ClearFlag(MATERIAL_VAR_ALPHATEST);
	}

	// Stock-Consistency : Flip $BaseAlphaEnvMapMask when not using $Phong or $BumpMap
	bool b1 = !IsDefined(BumpMap);
	bool b2 = !IsDefined(EnvMapMaskFlip);
	bool b3 = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	if (b1 && b2 && b3)
	{
		SetBool(EnvMapMaskFlip, 1);
	}
}

SHADER_FALLBACK
{
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
	
	if (IsDefined(BaseTexture) && IsTextureLoaded(BaseTexture))
	{
		bool bTranslucent = GetTexture(BaseTexture)->IsTranslucent();
		if (!bTranslucent)
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	LoadBumpMap(BumpMap);
	
	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	if (IsDefined(EnvMap))
	{
		LoadCubeMap(EnvMap);
		LoadTexture(EnvMapMask);

		// $EnvMapMask has Priority over other Masking
		if (IsTextureLoaded(EnvMapMask))
		{
			// We already have an $EnvMapMask, so remove these!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		else if (IsDefined(BumpMap) && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
		{
			// NormalMapAlphaEnvMapMask takes priority
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}
	}
}

SHADER_DRAW
{
	bool bProjTex = HasFlashlight();

	// Texture related Boolean. Check for existing booleans first!
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasNormalMap = IsTextureLoaded(BumpMap);

	// $Detail. BlendMode 5 and 6 not done on ProjTex
	int nDetailBlendMode = GetBool(DetailBlendMode);
	bool bAdditiveDetail = IsSelfIllumDetailMode(nDetailBlendMode);
	bool bHasDetailTexture = !(bProjTex && bAdditiveDetail) && IsTextureLoaded(Detail);

	// $EnvMap
	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap &&	IsTextureLoaded(EnvMapMask);
	bool bHasEnvMapFresnel = bHasEnvMap &&	GetBool(EnvMapFresnel);

	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

	// Its going to be at least one of these
	bool bTriplanarBase			= bHasBaseTexture && GetBool(TriPlanar_Base);
	bool bTriplanarDetail		= bHasDetailTexture && GetBool(TriPlanar_Detail);
	bool bTriplanarBump			= bHasNormalMap && GetBool(TriPlanar_Bump);
	bool bTriplanarEnvMapMask	= bHasEnvMapMask && GetBool(TriPlanar_EnvMapMask);

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(nBlendType);

		// We want AlphaWrites for DepthToDestAlpha and WaterFogToDestAlpha
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);

		// We output linear Values that need to be converted
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		// This doesn't actually support VertexColors
		// Should it? The only usecase I can imagine is Detail Props
//		if (HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA))
//			nFlags |= VERTEX_COLOR;

		// This enables Tangent Data apparently
		int nUserDataSize = (bProjTex || bHasNormalMap) ? 4 : 0;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. Always sRGB
		EnableSampler(SAMPLER_BASETEXTURE, true);

		// s2 - $BumpMap. Never sRGB
		EnableSampler(bHasNormalMap, SAMPLER_NORMALMAP, false);

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask. Not sRGB
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
	
		// How many non-Triplanar TexCoords we need
		// Unfortunately we will always have at minimum one TexCoord
		// That's just due to how existing Vertex Shaders work
		int nNeededTexCoords = 0;
		nNeededTexCoords += bTriplanarBase;
//		nNeededTexCoords += bTriplanarBump; // Bumped Model Shader always has 2 TexCoords
		nNeededTexCoords += bTriplanarEnvMapMask;
		nNeededTexCoords += bTriplanarDetail;
		nNeededTexCoords = Clamp(nNeededTexCoords, 0, 3);

		bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);
		if(bProjTex)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 1 + bHasNormalMap); // Need TBN or Normal
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		}
		else if (bHasNormalMap)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_bump_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(WRINKLEMAPS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_bump_vs30);
		}
		else
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(SPECIALTEXCOORDS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_vs30);
		}

		if (bProjTex)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_triplanar_model_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_BASE, bTriplanarBase);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_BUMP, bHasNormalMap + bTriplanarBump);		
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_DETAIL, bHasDetailTexture + bTriplanarDetail);	
			SET_STATIC_PIXEL_SHADER(lux_triplanar_model_flashlight_ps30);
		}
		else
		{
			int nEnvMapMode = bHasEnvMap + bHasEnvMapMask;
			DECLARE_STATIC_PIXEL_SHADER(lux_triplanar_model_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_BASE, bTriplanarBase);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_BUMP, bHasNormalMap + bTriplanarBump);
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_ENVMAPMASK, bTriplanarEnvMapMask);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_DETAIL, bHasDetailTexture + bTriplanarDetail);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER(lux_triplanar_model_ps30);
		}
	}
	
	//==========================================================================//
	// Dynamic State
	//==========================================================================//
	if(IsDynamicState())
	{
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture
		// Stock-Consistency. No $BaseTexture and $Envmap means Black instead of White
		// This is so EnvMaps on these surfaces don't overbright
		if (!bHasBaseTexture && bHasEnvMap)
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
		else
			BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_WHITE);

		// s2 - $BumpMap
		BindTexture(bHasNormalMap, SAMPLER_NORMALMAP, BumpMap, BumpFrame);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5 - $EnvMapMask
		BindTexture(bHasEnvMapMask, SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// VS Registers
		int nTexCoordShift = 0;
		if(!bTriplanarBase)
		{
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);
			nTexCoordShift += 2;
		}

		if(bHasNormalMap && !bTriplanarBump)
		{
			if(HasTransform(true, BumpTransform))
				SetVertexShaderTextureTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, BumpTransform);
			else
				SetVertexShaderTextureTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);
			nTexCoordShift += 2;
		}

		if(bHasEnvMapMask && !bTriplanarEnvMapMask)
		{
			if(HasTransform(true, EnvMapMaskTransform))
				SetVertexShaderTextureTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, EnvMapMaskTransform);
			else
				SetVertexShaderTextureTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);
			nTexCoordShift += 2;
		}

		if(bHasDetailTexture && !bTriplanarDetail)
		{
			if(HasTransform(true, DetailTextureTransform))
				SetVertexShaderTextureScaledTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, DetailTextureTransform, DetailScale);
			else
				SetVertexShaderTextureScaledTransform(nTexCoordShift + LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform, DetailScale);
		}

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c13, c14, c15, c16, c17, c18
		if (!bProjTex && bHasNormalMap)
			pShaderAPI->SetPixelShaderStateAmbientLightCube(LUX_PS_FLOAT_AMBIENTCUBE, !LightState.m_bAmbientLight);

		// c20, c21, c22, c23, c24, c25
		if (!bProjTex && bHasNormalMap)
			pShaderAPI->CommitPixelShaderLighting(LUX_PS_FLOAT_LIGHTDATA);

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

		// c35, c36, c37 c38
		if (bHasEnvMap)
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale);

			// Stock Consistency - Convert from Gamma to Linear
			f4EnvMapTint_LightScale.rgb = GammaToLinearTint(f4EnvMapTint_LightScale.rgb);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation);
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			float4 f4EnvMapControls = 0.0f;
			f4EnvMapControls.x = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			f4EnvMapControls.y = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
			f4EnvMapControls.z = (float)GetBool(EnvMapMaskFlip); // The shader will do abs($EnvMapMaskFlip - EnvMapMask)
			// FREE
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			float4 f4EnvMapFresnelRanges = 0.0f;
			if (bHasEnvMapFresnel)
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);	
			else
				f4EnvMapFresnelRanges.y = 1.0f; // This will disable EnvMapFresnel
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);
		}
	
		// c52, c53
		if(bTriplanarBase)
		{
			float4 Triplanar_Scales_Base = 0.0f;
			float4 Triplanar_Offset_Base = 0.0f;
			Triplanar_Scales_Base.xyz = GetFloat3(TriPlanar_Scale_Base);
			Triplanar_Offset_Base.xyz = GetFloat3(TriPlanar_Offset_Base);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_052, Triplanar_Scales_Base);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_053, Triplanar_Offset_Base);
		}

		// c54, c55
		if(bTriplanarDetail)
		{
			float4 Triplanar_Scales_Detail = 0.0f;
			float4 Triplanar_Offset_Detail = 0.0f;
			Triplanar_Scales_Detail.xyz = GetFloat3(TriPlanar_Scale_Detail);
			Triplanar_Offset_Detail.xyz = GetFloat3(TriPlanar_Offset_Detail);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_054, Triplanar_Scales_Detail);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_055, Triplanar_Offset_Detail);
		}

		// c56, c57
		if(bTriplanarBump)
		{
			float4 Triplanar_Scales_Bump = 0.0f;
			float4 Triplanar_Offset_Bump = 0.0f;
			Triplanar_Scales_Bump.xyz = GetFloat3(TriPlanar_Scale_Bump);
			Triplanar_Offset_Bump.xyz = GetFloat3(TriPlanar_Offset_Bump);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_056, Triplanar_Scales_Bump);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_057, Triplanar_Offset_Bump);
		}

		// c58, c59
		if(bTriplanarEnvMapMask)
		{
			float4 Triplanar_Scales_EnvMapMask = 0.0f;
			float4 Triplanar_Offset_EnvMapMask = 0.0f;
			Triplanar_Scales_EnvMapMask.xyz = GetFloat3(TriPlanar_Scale_EnvMapMask);
			Triplanar_Offset_EnvMapMask.xyz = GetFloat3(TriPlanar_Offset_EnvMapMask);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_058, Triplanar_Scales_EnvMapMask);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_059, Triplanar_Offset_EnvMapMask);
		}

		// Binds Flashlight Textures and sends constants
		// returns bFlashlightShadows
		bool bProjTexShadows = SetupFlashlight();

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
		// b0
		if(HasFlag(MATERIAL_VAR_HALFLAMBERT))
			BBools[LUX_PS_BOOL_HALFLAMBERT] = true;

		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// b4
		// Vertex Shader Booleans
		if(!bHasNormalMap)
		{
			BOOL BHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);
			pShaderAPI->SetBooleanVertexShaderConstant(LUX_VS_BOOL_HALFLAMBERT, &BHalfLambert);
		}

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		bool bHasStaticPropLighting = 0; 
		bool bHasDynamicPropLighting = 0;

		// Dynamic Prop Lighting here refers to dynamic vertex lighting, or ambient cubes via the vertex shader
		// We shouldn't have that on bumped or phonged models. Same for Static Vertex Lighting
		if (!bProjTex && !bHasNormalMap)
		{
			// LightState varies between SP and MP so we use a function to reinterpret
			bHasStaticPropLighting = StaticLightVertex(LightState);
			bHasDynamicPropLighting = LightState.m_bAmbientLight || (LightState.m_nNumLights > 0) ? 1 : 0;

			// Need to send this to the Vertex Shader manually in this scenario
			if (bHasDynamicPropLighting)
				pShaderAPI->SetVertexShaderStateAmbientLightCube();
		}

		if(bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		}
		else if (bHasNormalMap)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_bump_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_bump_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, bHasStaticPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, bHasDynamicPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
		}

		if (bProjTex)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_triplanar_model_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bProjTexShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_triplanar_model_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_triplanar_model_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS_COMBO, LightState.m_nNumLights);
			SET_DYNAMIC_PIXEL_SHADER(lux_triplanar_model_ps30);
		}
	}

	//==========================================================================//
	// ConVars
	//==========================================================================//
	if(IsDynamicState())
	{
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
#endif

#ifdef LUX_DEBUGCONVARS
		if (bHasNormalMap && lux_disablefast_normalmap.GetBool())
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);

		if (lux_disablefast_diffuse.GetBool())
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);

		if (bHasEnvMap && lux_disablefast_envmap.GetBool())
			BindTexture(SAMPLER_ENVMAPTEXTURE, TEXTURE_BLACK);
#endif
	}

	Draw();
}
END_SHADER