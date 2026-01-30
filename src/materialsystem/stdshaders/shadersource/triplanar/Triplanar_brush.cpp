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
#include "lux_brush_simplified_vs30.inc"
#include "lux_brush_vs30.inc"
#include "lux_triplanar_brush_ps30.inc"
#include "lux_triplanar_brush_flashlight_ps30.inc"

//==========================================================================//
// Triplanar for Model based Geometry
//==========================================================================//
BEGIN_VS_SHADER(LUX_Triplanar_Brush, "Pretty much LightmappedGeneric but with Triplanar Mapping.")
SHADER_INFO_GEOMETRY	("Brushes")
SHADER_INFO_USAGE		("Use at least one Triplanar Method.")
SHADER_INFO_LIMITATIONS	("Does not support Phong.\n"
						 "Does not support $LightWarpTexture.\n"
						 "Does not support $SelfIllum. Does support $DetailBlendMode 5.\n"
						 "Does not support a bunch of other Things compared to LightmappedGeneric.\n"
						 "However a lot more Features than Stock.")
SHADER_INFO_PERFORMANCE	("Expensive due to multiple Texture Samples for each Triplanar Texture.\n"
						 "The more Triplanar Methods are applied the more expensive it becomes.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"Reference $Seamless related Parameters.")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_NormalTextureParameters()
	Declare_NoDiffuseBumpLighting()
	Declare_DetailTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_ParallaxCorrectionParameters()
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
	bool bModelShader = false;

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

void SetTriplanarBrushFlags()
{
	// Always needed for the Flashlight
	// Well not really but I'm keeping it for Stock-Consistency
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES); 

	// We always want access to regular Lightmaps
	// If we have a $BumpMap, we wanted the Bumped ones with the regular ones as Fallback 
	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
	if (g_pConfig->UseBumpmapping() && IsDefined(BumpMap))
	{
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
	}
}

SHADER_INIT_PARAMS()
{
	SetTriplanarBrushFlags();
	HandleFallback();

	if (IsDefined(EnvMap))
	{
		if (!g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		{
			SetUndefined(EnvMap);
			SetBool(EnvMapParallax, false);
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
	// Stock-Consistency
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap) && IsTextureLoaded(BumpMap))
	{
		bHasBumpMap = true;

		// If the User didn't specifically imply this is a SSBump, check using the flags
		if (!IsDefined(SSBump))
		{
			ITexture* pBumpMap = GetTexture(BumpMap);
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
		else if (bHasBumpMap && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
		{
			// NormalMapAlphaEnvMapMask takes priority
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

	// Mapbase uses $EnvMapParallax as a bool, but the VDC Implementation checks if OBB's are defined.
	// Checking if they are defined is very unsafe, it might just randomly be true. So I'm checking GetBool() instead
	// That way if the entier OBB is just 0 it doesn't accidentally trigger PCC. ( Which it does if you use IsDefined() )
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

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

		// Just always ask for Normal... You pretty much need it 99% of the time
		unsigned int nFlags = VERTEX_POSITION;

		// *sigh* Another Hammer specific Issue
		// Vertex Colors are used for Shaded Texture Polygons..
		if (HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			nFlags |= VERTEX_COLOR;

		// EnvMap wants Tangents, ProjTex always needs Normals
		if(bProjTex || bHasEnvMap)
			nFlags |= VERTEX_NORMAL;

		// Normal Maps don't require the TBN Matrix actually.
		// ( Due to how Radiosity Normal Mapping works )
		// Projected Textures and EnvMaps do
		if (bProjTex && bHasNormalMap || bHasEnvMap)
			nFlags |= VERTEX_TANGENT_SPACE;

		// No Lightmap UV when using projected Textures
		// 1 TexCoord  = UV
		// 2 TexCoords = UV + Lightmap UV
		// 3 TexCoords = UV + Lightmap UV + Bumped Lightmap UV Offset
		int nTexCoords = bProjTex ? 1 : 2;
		if(!bProjTex && bHasNormalMap) 
			nTexCoords = 3;

		int nUserDataSize = 0;

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

		// s11 - Lightmap. sRGB on LDR
		EnableSampler(!bProjTex, SAMPLER_LIGHTMAP, !IsHDREnabled());

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
		nNeededTexCoords += bTriplanarBump;
		nNeededTexCoords += bTriplanarEnvMapMask;
		nNeededTexCoords += bTriplanarDetail;
		nNeededTexCoords = Clamp(nNeededTexCoords, 0, 3);

		bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);
		if(bProjTex)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);		
			SET_STATIC_VERTEX_SHADER_COMBO(NOMODELMATRIX, 0);	
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, bHasNormalMap ? 2 : 1);		
			SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, 0);	
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);	
			SET_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
		}
		else
		{
			bool bNeedsBumpedLightmaps = bHasNormalMap && !GetBool(NoDiffuseBumpLighting);
			DECLARE_STATIC_VERTEX_SHADER(lux_brush_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bHasNormalMap && bHasEnvMap);
			SET_STATIC_VERTEX_SHADER_COMBO(BUMPED_LIGHTMAP, bNeedsBumpedLightmaps);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_VERTEX_SHADER(lux_brush_vs30);
		}

		if (bProjTex)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_triplanar_brush_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_BASE, bTriplanarBase);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_BUMP, bHasNormalMap + bTriplanarBump);
			SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasNormalMap && GetBool(SSBump));
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_DETAIL, bHasDetailTexture + bTriplanarDetail);
			SET_STATIC_PIXEL_SHADER(lux_triplanar_brush_flashlight_ps30);
		}
		else
		{
			int nEnvMapMode = bHasEnvMap + bHasEnvMapMask + 2 * bPCC;
			DECLARE_STATIC_PIXEL_SHADER(lux_triplanar_brush_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_BASE, bTriplanarBase);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_BUMP, bHasNormalMap + bTriplanarBump);
			SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasNormalMap && GetBool(SSBump));
			SET_STATIC_PIXEL_SHADER_COMBO(TRIPLANAR_ENVMAPMASK, bTriplanarEnvMapMask);
			SET_STATIC_PIXEL_SHADER_COMBO(MODE_DETAIL, bHasDetailTexture + bTriplanarDetail);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER(lux_triplanar_brush_ps30);
		}
	}
	
	//==========================================================================//
	// Dynamic State
	//==========================================================================//
	if(IsDynamicState())
	{
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

		// s11 - Lightmap
		BindTexture(!bProjTex, SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bProjTexShadows = SetupFlashlight();

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

		// c1 - Modulation Constant
		// Function above, handles LightmapScaleFactor and Alpha Modulation
		SetModulationConstant(bHasNormalMap && GetBool(SSBumpMathFix));

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

			// $EnvMapOrigin, $EnvMapParallaxOBB1, $EnvMapParallaxOBB2, $EnvMapParallaxOBB3
			if(bPCC)
			{
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX, f4Row1);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_2, f4Row2);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_3, f4Row3);
			}
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


		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		if(bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
		}

		if (bProjTex)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_triplanar_brush_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bProjTexShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_triplanar_brush_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_triplanar_brush_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, r_lightmap_bicubic.GetBool());
			SET_DYNAMIC_PIXEL_SHADER(lux_triplanar_brush_ps30);
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

		if (lux_disablefast_lightmap.GetBool())
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
#endif

#ifdef DEBUG_LUXELS
		if (mat_luxels.GetBool())
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
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