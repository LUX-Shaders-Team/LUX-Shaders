//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"
#include "renderpasses/Cloak.h"

// Includes for Shaderfiles...
#include "lux_model_vs30.inc"
#include "lux_modulate_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Modulate,	 LUX_Modulate)
DEFINE_FALLBACK_SHADER(SDK_Modulate_DX6, LUX_Modulate)
DEFINE_FALLBACK_SHADER(SDK_Modulate_DX9, LUX_Modulate)
DEFINE_FALLBACK_SHADER(SDK_Modulate_DX8, LUX_Modulate)
#endif

#ifdef REPLACE_MODULATE
DEFINE_FALLBACK_SHADER(Modulate,		LUX_Modulate)
DEFINE_FALLBACK_SHADER(Modulate_DX6,	LUX_Modulate)
DEFINE_FALLBACK_SHADER(Modulate_DX9,	LUX_Modulate)
DEFINE_FALLBACK_SHADER(Modulate_DX8,	LUX_Modulate)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Modulate, "A Shader that modulates Areas behind it." )
SHADER_INFO_GEOMETRY	("Brushes, Models and Displacements.")
SHADER_INFO_USAGE		("By Default only $BaseTexture is used but the Shader has support for Vertex Lighting and Model Lightmaps.\n"
						 "By using $Mod2X the Shader can also brighten Areas instead of just darkening them.\n"
						 "The Alpha Channel of the BaseTexture masks the Effect.")
SHADER_INFO_LIMITATIONS	("Cannot do Per-Pixel Lighting. Only Per-Texel and Per-Vertex.\n"
						 "Output of the Shader used for Modulation is in Gamma Space. This might alter Lighting a little bit..")
SHADER_INFO_PERFORMANCE	("Decently fast.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Modulate Shader Page: https://developer.valvesoftware.com/wiki/Modulate")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_CloakParameters()
	Declare_DetailTextureParameters()
	Declare_LightmappingParameters()
	SHADER_PARAM(EnableVertexLighting, SHADER_PARAM_TYPE_BOOL, "", "Enables Vertex Lighting on the BaseTexture.")

	// Original Parameters
	SHADER_PARAM(WriteZ,	SHADER_PARAM_TYPE_BOOL, "0", "Enables Depth Writes.")
	SHADER_PARAM(Mod2X,		SHADER_PARAM_TYPE_BOOL, "0", "Output is treated as 0..2 Range. So Values below 0.5f will darken and above 0.5f will brighten the Background.")
END_SHADER_PARAMS

void Modulate_SetupCloakVars(Cloak_Vars_t& CloakVars)
{
	CloakVars.InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	CloakVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
	CloakVars.Bump.InitVars(-1);
}

// IMPORTANT: Virtual Function Override.
bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar** params, bool bCheckSpecificToThisFrame ) const override
{
	// Need to use params directly here, otherwise we corrupt m_ppParams for Draw()
	if ( params[CloakPassEnabled]->GetIntValue() )
	{
		float flCloakFactor = params[CloakFactor]->GetFloatValue();
		if ( !bCheckSpecificToThisFrame || (flCloakFactor > 0.0f && flCloakFactor < 1.0f) )
			return true;
	}

	return params[Flags_Defined2]->GetIntValue() & MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE;
}

// IMPORTANT: Virtual Function Override.
bool IsTranslucent( IMaterialVar** params ) const override
{
	// Need to use params directly here, otherwise we corrupt m_ppParams for Draw()
	if ( params[CloakPassEnabled]->GetIntValue() )
	{
		float flCloakFactor = params[CloakFactor]->GetFloatValue();
		if ( flCloakFactor > 0.0f && flCloakFactor < 1.0f )
			return true;
	}

	return params[Flags]->GetIntValue() & MATERIAL_VAR_TRANSLUCENT;
}

// Set Up Vars here
SHADER_INIT_PARAMS()
{
	Cloak_Vars_t CloakVars;
	Modulate_SetupCloakVars(CloakVars);
	CloakBlend_Init_Params(this, CloakVars);

	// No Flashlight ever
	SetBool(ReceiveProjectedTextures, false);

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}

SHADER_FALLBACK
{
#ifndef REPLACE_MODULATE
	if (lux_oldshaders.GetBool())
		return "Modulate_DX9";
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
	Cloak_Vars_t CloakVars;
	Modulate_SetupCloakVars(CloakVars);
	CloakBlend_Shader_Init(this, CloakVars);

	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadTexture(Lightmap);
	LoadTexture(Detail);
}

SHADER_DRAW
{
	bool bDrawBasePass = true;

	Cloak_Vars_t CloakVars;
	Modulate_SetupCloakVars(CloakVars);
	bool bCloakEnabled = GetBool(CloakPassEnabled);
	
	if (bCloakEnabled && (pShaderShadow == NULL))
	{
		if (CloakBlend_IsOpaque(this, params, CloakVars))
			bDrawBasePass = false;
	}

	if (bDrawBasePass)
	{
		bool bMod2X = GetBool(Mod2X);
		bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
		int  nDetailBlendMode = GetBool(DetailBlendMode);
		bool bHasDetailTexture = (nDetailBlendMode != 5 && nDetailBlendMode != 6) && IsTextureLoaded(Detail);
		bool bModelLightmap = IsTextureLoaded(Lightmap);
		BlendType_t nBlendType = ComputeBlendType(BaseTexture, true);
		bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

		//==========================================================================//
		// Static Snapshot of the Shader Settings
		//==========================================================================//
		if(IsSnapshotting())
		{
			bool bWriteZ = GetBool(WriteZ);
			//==========================================================================//
			// General Rendering Setup
			//==========================================================================//

			// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
			SetInitialShadowState();

			if (bMod2X)
			{
				EnableAlphaBlending(SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR);
			}
			else
			{
				EnableAlphaBlending(SHADER_BLEND_DST_COLOR, SHADER_BLEND_ZERO);
			}

			if (bWriteZ)
				pShaderShadow->EnableDepthWrites(true);

			// Only really putting Depth Values here
			pShaderShadow->EnableAlphaWrites(bWriteZ && bIsFullyOpaque);

			// This Shader does not write linear Values because we aren't convert our Textures to Linear
			pShaderShadow->EnableSRGBWrite(false);

			// Stock-Consistency
			// "We need to fog to *white* regardless of overbrighting..."
			if (bMod2X)
			{
				FogToGrey();
			}
			else
			{
				FogToOOOverbright();
			}

			//==========================================================================//
			// Vertex Shader - Vertex Format
			//==========================================================================//
			int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
			int nTexCoords = bHasBaseTexture;

			bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);
			if (bHasVertexColors)
				nFlags |= VERTEX_COLOR;
			else
				nTexCoords = 1; // 'add 1 texcoord if these verts are too thin' Aka Vertex Format is too small otherwise

			// Pretty simple, one TexCoord and vPos for ProjPos
			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, 0);

			//==========================================================================//
			// Sampler Setup
			//==========================================================================//

			// s0 - $BaseTexture. NOT sRGB
			EnableSampler(bHasBaseTexture, SAMPLER_BASETEXTURE, true);

			// s4 - $Detail. We use mod2x which is always Linear
			EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, false);

			// s11 - $Lightmap
			// Always need this Sampler, we only know in Dynamic State whether or not it's needed.
			EnableSampler(SAMPLER_LIGHTMAP, false);

			//==========================================================================//
			// Set Static Shaders
			//==========================================================================//

			// Using model_vs30 so we can get unmodified Lightmap UV's
			// Stock-Consistency: VertexColors*2 for Linear VertexColors
			DECLARE_STATIC_VERTEX_SHADER(lux_model_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, bHasDetailTexture ? 1 : 0); 
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors * 2);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, 0); 
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(SPECIALTEXCOORDS, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_modulate_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(VERTEXLIT, GetBool(EnableVertexLighting));
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_PIXEL_SHADER(lux_modulate_ps30);
		}

		//==========================================================================//
		// Entirely Dynamic Commands
		//==========================================================================//
		if(IsDynamicState())
		{
			bool bWriteZ = GetBool(WriteZ);
			//==========================================================================//
			// Bind Textures
			//==========================================================================//

			// s0 - $BaseTexture
			BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, Frame);

			// s11 - $Lightmap
			// Feature requested by siryodajedi ( Yui ) 
			// Model Lightmapping Support
			if(bModelLightmap)
				BindTexture(SAMPLER_LIGHTMAP, Lightmap);
			else
				BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
			
			// s4 - $Detail
			BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

			//==========================================================================//
			// Setup Constant Registers
			//==========================================================================//

			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

			if(bHasDetailTexture)
			{
				if (HasTransform(true, DetailTextureTransform))
					SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, DetailTextureTransform, DetailScale);
				else
					SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform, DetailScale);
			}

			// Need this for $Alpha/$Alpha2 and WaterFogFactorType
			SetModulationConstant(false, false);

			// c11 - Camera Position
			SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
			
			// c12 - Fog Params
			pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

			// c19
			// Need Gamma Constant for Lightmaps
			if(bModelLightmap)
				SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);
			
			// c32 - $Color, $Color2, $sRGBTint
			// Want $Alpha2 support here too. Could replace with GetAlpha() but we already got $Alpha through ComputeTint
			float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
			f4Tint.a *= GetFloat(Alpha2);

			// Shader previously happened in Gamma Space, but it no longer does ( it's in linear Space now )
			// So convert the Tint from Gamma->Linear to account for this
			f4Tint.rgb = GammaToLinearTint(f4Tint.rgb);

			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

			// c33, c34 - $Detail
			if (bHasDetailTexture)
			{
				float4 f4DetailTintFactor;
				f4DetailTintFactor.rgb = GetFloat3(DetailTint);
				f4DetailTintFactor.w = GetFloat(DetailBlendFactor);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4DetailTintFactor);
			}

			// Prepare boolean array, yes we need to use BOOL
			BOOL BBools[REGISTER_BOOL_MAX] = { false };

			// b13, b14, b15
			BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque && bWriteZ);
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque && bWriteZ);
			pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

			//==================================================================================================
			// Set Dynamic Shaders
			//==================================================================================================

			bool bHasStaticPropLighting = false;
			bool bHasDynamicPropLighting = false;
			if(GetBool(EnableVertexLighting))
			{
				LightState_t LightState;
				pShaderAPI->GetDX9LightState(&LightState);

				// LightState varies between SP and MP so we use a function to reinterpret
				bHasStaticPropLighting = StaticLightVertex(LightState);
				bHasDynamicPropLighting = (LightState.m_bAmbientLight || (LightState.m_nNumLights > 0)) ? 1 : 0;

				// Need to send this to the Vertex Shader manually in this scenario
				if (bHasDynamicPropLighting)
					pShaderAPI->SetVertexShaderStateAmbientLightCube();
			}

			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, bHasStaticPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, bHasDynamicPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_modulate_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bModelLightmap);
			SET_DYNAMIC_PIXEL_SHADER(lux_modulate_ps30);
		}

		Draw();
	}
	else
	{
		// We are cloaking, so stop doing the base pass
		// Otherwise the enemy team is going to cause a malfunction in our spy
		Draw(false);
	}

	// Now that we determined whether or not to draw the base
	// Draw spycloak if necessary
	if (bCloakEnabled)
	{
		float f1CloakFactor = GetFloat(CloakVars.m_nCloakFactor);

		// If we are on snapshot we **really** have to set up the cloak shaders
		// Otherwise we will try to bind dynamic shaders to non-existant static ones
		// Also if we are at a cloakfactor of 0 there is no point in drawing the cloak
		if ((pShaderShadow != NULL) || ((f1CloakFactor > 0.0f) && (f1CloakFactor < 1.0f)))
			CloakBlend_Shader_Draw(this, pShaderShadow, pShaderAPI, CloakVars);
	}
}
END_SHADER