//===================== File of the LUX Shader Project =====================//
//
//	Original D. :	29.10.2024 DMY
//	Initial D.	:	11.11.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_distancealpha_ps30.inc"
#include "lux_model_vs30.inc"
#include "lux_model_simplified_vs30.inc"

//==========================================================================//
// Model ONLY
//==========================================================================//
BEGIN_VS_SHADER(LUX_DistanceAlpha_Model, "A Shader for the 'Signed Distance Fields' Alpha Effect.\n" )
SHADER_INFO_GEOMETRY	("Models.")
SHADER_INFO_USAGE		("Have a SDF in the Alpha Channel of your BaseTexture.")
SHADER_INFO_LIMITATIONS	("Doesn't support a lot of the more complicated Shader Effects offered by the Original that routes here.\n"
						 "A lot of Effects based on the Alpha Channel of the BaseTexture are a bit limited on this Shader.")
SHADER_INFO_PERFORMANCE	("Decently fast.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						 "VDC DistanceAlpha Page: https://developer.valvesoftware.com/wiki/$distancealpha \n"
						 "Valves siggraph about SDF's https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	// Detail Textures are about the only Feature that DistanceAlpha ever supported
	Declare_DetailTextureParameters()
	Declare_LightmappingParameters()
	Declare_DistanceAlphaParameters()

	SHADER_PARAM(BlendTintByBaseAlpha, SHADER_PARAM_TYPE_BOOL, "", "Use the BaseTextures Alpha Channel to blend in Tint Parameters.")
	SHADER_PARAM(BlendTintColorOverBase, SHADER_PARAM_TYPE_FLOAT, "", "Blend between Tint acting as a Multiplier versus a Replacement, requires $BlendTintByBaseAlpha.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Brushes get Lightmaps, always
	SetFlag(MATERIAL_VAR_MODEL); // Always Model'ify UnlitGeneric's that goes here

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	DefaultFloat(DetailBlendFactor, 1.0f); // Default Value is supposed to be 1.0f

	// Regular Default value.. ( It still doesn't make sense to be 4..)
	DefaultFloat(DetailScale, 4.0f);

	DefaultFloat(EdgeSoftnessStart, 0.5);
	DefaultFloat(EdgeSoftnessEnd, 0.5);
	DefaultFloat(OutlineAlpha, 1.0);

	if (CVarDeveloper.GetInt() > 0)
	{
		if (!IsDefined(BaseTexture) && !GetBool(DistanceAlphaFromDetail))
		{
			ShaderDebugMessage("has no Basetexture to derive the Alpha from, and doesn't want to use the DetailTexture.\n\n");
		}

		if (HasFlag(MATERIAL_VAR_MODEL) && IsDefined(DetailBlendMode) && GetInt(DetailBlendMode) == 10)
		{
			ShaderDebugMessage("cannot uuse DetailBlendMode 10. Model Materials don't have Bumped Lightmaps.\n\n");
		}
	}

	// No Flashlight
	SetBool(ReceiveProjectedTextures, 0);
}

SHADER_FALLBACK
{
	// Don't have to check for this
	// When using ULG,VLG;LMG its gonna detour here
	// UNLESS lux_oldshaders
//	if (lux_oldshaders.GetBool())

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture);
	LoadTexture(Detail);

	// Portal 2's Panel Texture uses $DetailBlendMode 10 with an SSBump, however official Stock Shaders don't allow this.
	// It would only work if not a SSBump. LUX Adds support for it, so consider that!
	bool bModel = HasFlag(MATERIAL_VAR_MODEL);
	bool bDetail = IsTextureLoaded(Detail);
	if (!bModel && bDetail && GetTexture(Detail)->GetFlags() & TEXTUREFLAGS_SSBUMP)
	{
		// No $BumpMap so can only be $DetailBlendMode 11
		SetInt(DetailBlendMode, 11);
	}
}

SHADER_DRAW
{
	// Set up some booleans
	//===============================//
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasDetailTexture = IsTextureLoaded(Detail);
	bool bProjTex = HasFlashlight(); // Drawing Behaviour handled on Param Init!

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

		// This is now handled in the flashlight sampler setup
		// pShader->DefaultFog();

		// Everything Transparency is packed into this Function
		BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
		EnableTransparency(nBlendType);

		// We can never write to Alpha
		// Since we are always doing Transparency ( Although that isn't actually enforced )
		pShaderShadow->EnableAlphaWrites(false);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
//		bool bHasVertexColor = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);

		// We never need Normals for anything
		unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;

		if (bProjTex)
			flags |= VERTEX_NORMAL;

		int nTexCoords = 1; // 2 texcoords for brushes, 1 for models
		pShaderShadow->VertexShaderVertexFormat(flags, nTexCoords, 0, bProjTex ? 4 : 0);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//
		EnableSampler(SAMPLER_BASETEXTURE, true); // We always have a basetexture, and yes they should always be sRGB

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		int nDetailBlendMode = GetInt(DetailBlendMode);
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		// Model Lightmaps are never sRGB
		// Always need to enable this, $Lightmap is only available in Dynamic State
		EnableSampler(SAMPLER_LIGHTMAP, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);

		int nNeededTexCoords = bHasDetailTexture;
		if(bProjTex)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 1);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
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

		// 0 = Brush
		// 1 = Model
		// 2 = Unlit
		// 3 = ProjTex Pass
		int nMaterialType = bProjTex ? 3 : 1;

		// 0 = Nope
		// 1 = Detail Texture
		// 2 = Detail Texture + $DistanceAlphaFromDetail
		// 3 = Detail Texture + $DistanceAlphaFromDetail + $BlendTintByBaseAlpha
		int nDetailMode = bHasDetailTexture;
		if (GetBool(DistanceAlphaFromDetail))
		{
			// BlendTintByBaseAlpha is only supported with $DistanceAlphaFromDetail
			// Stock-Consistency / Stock-Consistent
			bool bBlendTintByBaseAlpha = bHasBaseTexture && GetBool(BlendTintByBaseAlpha);
			nDetailMode = bBlendTintByBaseAlpha ? 3 : 2;
		}

		DECLARE_STATIC_PIXEL_SHADER(lux_distancealpha_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(MATERIAL_TYPE, nMaterialType);
		SET_STATIC_PIXEL_SHADER_COMBO(VERTEX_COLORS, bHasVertexColors);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailMode);
		SET_STATIC_PIXEL_SHADER_COMBO(SOFT_MASK, GetBool(SoftEdges));
		SET_STATIC_PIXEL_SHADER_COMBO(OUTLINE, GetBool(Outline));
		SET_STATIC_PIXEL_SHADER_COMBO(OUTER_GLOW, GetBool(Glow));
		SET_STATIC_PIXEL_SHADER(lux_distancealpha_ps30);
	}
	else // Dynamic State
	{
		// Getting the light state
		// We always need this for Models
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		else
#endif
			BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_WHITE);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		if (IsTextureLoaded(Lightmap))
		{
			#ifdef LUX_DEBUGCONVARS
			if (lux_disablefast_lightmap.GetBool())
			{
				BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
			}
			else
			#endif
				BindTexture(SAMPLER_LIGHTMAP, Lightmap);

			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);
		}
		else
		{
			// Always need to bind something here, the Sampler is always enabled since $Lightmap is only set in Dynamic State
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
		}

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// VS c223, c224
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// VS c227, c228
		if(bHasDetailTexture)
		{
			if (HasTransform(true, DetailTextureTransform))
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, DetailTextureTransform, DetailScale);
			else
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform, DetailScale);
		}

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
		
		if (!bProjTex)
		{
			// c13, c14, c15, c16, c17, c18
			pShaderAPI->SetPixelShaderStateAmbientLightCube(LUX_PS_FLOAT_AMBIENTCUBE, !LightState.m_bAmbientLight);

			// c20, c21, c22, c23, c24, c25
			pShaderAPI->CommitPixelShaderLighting(LUX_PS_FLOAT_LIGHTDATA);
		}

		// c32 - $Color, $Color2, $sRGBTint
		// NOTE: $DesaturateWithBaseAlpha not supported on this Shader
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), BlendTintColorOverBase);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// c33, c34
		if (bHasDetailTexture)
		{
			int nDetailBlendMode = GetInt(DetailBlendMode);

			float4 f4Tint_Factor;
			f4Tint_Factor.rgb = GetFloat3(DetailTint);
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4Tint_Factor);

			float4 f4Blendmode = 0.0f;
			f4Blendmode.x = (float)nDetailBlendMode;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4Blendmode);
		}

		// Distance Alpha related Constants
		if (true)
		{
			// c5, c6
			if (GetBool(Glow))
			{
				// This is pretty much consistent with the Stock Constant Register Layout
				float4 f4GlowParameters;
				f4GlowParameters.x = GetFloat(GlowX);
				f4GlowParameters.y = GetFloat(GlowY);
				f4GlowParameters.z = GetFloat(GlowStart);
				f4GlowParameters.w = GetFloat(GlowEnd);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_005, f4GlowParameters);

				float4 f4GlowColor;
				f4GlowColor.rgb = GetFloat3(GlowColor);
				f4GlowColor.a = GetFloat(GlowAlpha);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_006, f4GlowColor);
			}

			float f1SoftStart = GetFloat(EdgeSoftnessStart);
			float f1SoftEnd = GetFloat(EdgeSoftnessEnd);

			float f1OutlineStart0 = GetFloat(OutlineStart0);
			float f1OutlineStart1 = GetFloat(OutlineStart1);
			float f1OutlineEnd0 = GetFloat(OutlineEnd0);
			float f1OutlineEnd1 = GetFloat(OutlineEnd1);

			// "set all line art shader parms"
			bool bScaleEdges = GetBool(ScaleEdgeSoftnessBasedOnScreenRes);
			bool bScaleOutline = GetBool(ScaleOutlineSoftnessBasedOnScreenRes);

			// Only do this when scaling anything based on screen res
			if (bScaleEdges || bScaleOutline)
			{
				int nWidth, nHeight;
				pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);

				// ..1: Up these resolutions, find a dynamic approach for these constants?
				// ..2: (float), those are integers right now
				float f1ResScale = max(0.5f, max(1024.0f / nWidth, 768.0f / nHeight));

				if (bScaleEdges)
				{
					float f1Mid = 0.5 * (f1SoftStart + f1SoftEnd);
					f1SoftStart = clamp(f1Mid + f1ResScale * (f1SoftStart - f1Mid), 0.05, 0.99);
					f1SoftEnd = clamp(f1Mid + f1ResScale * (f1SoftEnd - f1Mid), 0.05, 0.99);
				}


				if (bScaleOutline)
				{
					// shrink the soft part of the outline, enlarging hard part
					float f1MidS = 0.5 * (f1OutlineStart1 + f1OutlineStart0);
					f1OutlineStart1 = clamp(f1MidS + f1ResScale * (f1OutlineStart1 - f1MidS), 0.05f, 0.99f);
					float f1MidE = 0.5 * (f1OutlineEnd1 + f1OutlineEnd0);
					f1OutlineEnd1 = clamp(f1MidE + f1ResScale * (f1OutlineEnd1 - f1MidE), 0.05f, 0.99f);
				}

			}

			// c7
			float4 f4DA_Parameters = 0.0f;
			f4DA_Parameters.x = f1SoftStart;
			f4DA_Parameters.y = f1SoftEnd;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_007, f4DA_Parameters);

			// c8
			if (GetBool(Outline))
			{
				float4 f4OutlineColor;
				f4OutlineColor.rgb = GetFloat3(OutlineColor);
				f4OutlineColor.a = GetFloat(OutlineAlpha); // This isn't actually used anywhere on the Shader
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_008, f4OutlineColor);
			}

			// "c9 - outline parms. ordered for optimal ps20 .wzyx swizzling"
			float4 f4OutlineParameters;
			f4OutlineParameters.x = f1OutlineStart0;
			f4OutlineParameters.y = f1OutlineEnd1;
			f4OutlineParameters.z = f1OutlineEnd0;
			f4OutlineParameters.w = f1OutlineStart1;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_009, f4OutlineParameters);
		}

		// Binds Flashlight Textures and sends constants
		// returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		// Never Opaque!
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(false);

		// ALWAYS! Required for HalfLambert, LightWarpTexture, and more.
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// b4
		// Vertex Shader Booleans
		BOOL BHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);
		pShaderAPI->SetBooleanVertexShaderConstant(LUX_VS_BOOL_HALFLAMBERT, &BHalfLambert);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		bool bHasStaticPropLighting = 0;
		bool bHasDynamicPropLighting = 0;

		// Dynamic Prop Lighting here refers to dynamic vertex lighting, or ambient cubes via the vertex shader
		// We shouldn't have that on bumped or phonged models. Same for Static Vertex Lighting
		bHasStaticPropLighting = StaticLightVertex(LightState); // LightState varies between SP and MP so we use a function to reinterpret
		bHasDynamicPropLighting = LightState.m_bAmbientLight || (LightState.m_nNumLights > 0) ? 1 : 0;

		// Need to send this to the Vertex Shader manually in this scenario
		if (bHasDynamicPropLighting)
			pShaderAPI->SetVertexShaderStateAmbientLightCube();

		if(bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, !bProjTex && bHasStaticPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, !bProjTex && bHasDynamicPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
		}

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_distancealpha_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, IsTextureLoaded(Lightmap));
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER(lux_distancealpha_ps30);
	}

	Draw();
}
END_SHADER
