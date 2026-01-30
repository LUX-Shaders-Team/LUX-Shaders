//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "EmissiveBlend.h"

// Includes for Shaderfiles...
#include "lux_emissive_vs30.inc"
#include "lux_emissive_ps30.inc"

void EmissiveBlend_Init_Params(CBaseVSShader* pShader, EmissiveBlend_Vars_t& info)
{
	if (!pShader->GetBool(info.m_nEmissiveBlendEnabled))
		return;

	// **NOT** always a Model
	// The Caller must set relevant Flags
//	pShader->SetFlag(MATERIAL_VAR_MODEL);
//	pShader->SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// Multiplier so 1.0f
	pShader->DefaultFloat(info.m_nEmissiveBlendStrength, 1.0f);

	// -123 now indicates the use of curtime, instead of 0. This way its possible to set 0
	pShader->DefaultFloat(info.m_nTime, -123.0f);
}

void EmissiveBlend_Shader_Init(CBaseVSShader* pShader, EmissiveBlend_Vars_t& info)
{
	// These are treated as sRGB but are not loaded as such..
	pShader->LoadTexture(info.m_nEmissiveBlendBaseTexture, 0);
	pShader->LoadTexture(info.m_nEmissiveBlendFlowTexture, 0);
	pShader->LoadTexture(info.m_nEmissiveBlendTexture, 0);

	// DetailTexture should be handled by the main Shader!
}

void EmissiveBlend_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, EmissiveBlend_Vars_t& info)
{
	// $Detail Parameters
	bool bDetailVars = info.Detail.m_nDetail != -1;
	int	nDetailBlendMode = 0;
	bool bHasDetail = false;
	if(bDetailVars)
	{
		// Only 5 and 6 are additive. So don't bother doing Detail otherwise
		nDetailBlendMode = pShader->GetInt(info.Detail.m_nDetailBlendmode);
		bHasDetail = pShader->IsTextureLoaded(info.Detail.m_nDetail) && IsSelfIllumDetailMode(nDetailBlendMode);
	}

	// $SelfIllumTexture Parameters
	bool bSelfIllumVars = info.SelfIllum.m_nSelfIllumTexture != -1;
	bool bHasSelfIllumTexture = bSelfIllumVars && pShader->IsTextureLoaded(info.SelfIllum.m_nSelfIllumTexture);

	// Base EmissiveBlend Parameters
	bool bEmissiveBlend = pShader->GetBool(info.m_nEmissiveBlendEnabled);
	bool bMinimumLight = pShader->GetBool(info.m_nMinimumLightAdditivePass);

	// Don't need to do anything here without either of these
	if (!bHasDetail && !bEmissiveBlend && !bHasSelfIllumTexture && !bMinimumLight)
	{
		return;
		// *** NOTHING ***
	}
	// Instantly abort if we are on any kind of projected Texture Pass
	// We can't use $ReceiveProjectedTextures since this is a second Pass
	else if (pShader->HasFlashlight())
	{
		pShader->Draw(false);
		return;
	}

	bool bHasFlowTexture = bEmissiveBlend && pShader->IsTextureLoaded(info.m_nEmissiveBlendFlowTexture);
	bool bHasNoFlow = bEmissiveBlend && !bHasFlowTexture;

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(pShader->IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// "Reset shadow state manually since we're drawing from two Materials"
		pShader->SetInitialShadowState();

		// Additive Blending
		// Setting this also disables DepthWrites.
		pShader->EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
		pShaderShadow->EnableAlphaWrites(false);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = 0; // Don't need Normal for anything
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0, s1, s2 - Emissive Blend
		if (bEmissiveBlend)
		{
			// We always have a EmissiveBlendBaseTexture, FlowTexture and Emissive Texture.
			// Only Flow is not sRGB
			pShader->EnableSampler(SHADER_SAMPLER0, true); // EmissiveBlendBaseTexture
			pShader->EnableSampler(SHADER_SAMPLER1, false); // Flow
			pShader->EnableSampler(SHADER_SAMPLER2, true); // Emissive Texture
		}

		// $SelfIllumTexture. always sRGB
		if(bHasSelfIllumTexture)
			pShader->EnableSampler(SHADER_SAMPLER3, true);

		// s4 - $Detail. Always sRGB for Blendmode 5 and 6
		if(bHasDetail)
			pShader->EnableSampler(SHADER_SAMPLER4, true);

		// $BaseTexture
		if(bMinimumLight)
			pShader->EnableSampler(SHADER_SAMPLER5, true);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_emissive_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(EMISSIVEBLENDMODE, bEmissiveBlend + bHasNoFlow);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE, bHasDetail);
		SET_STATIC_VERTEX_SHADER_COMBO(SELFILLUMTEXTURE, bHasSelfIllumTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(MINIMUMLIGHT, bMinimumLight);
		SET_STATIC_VERTEX_SHADER(lux_emissive_vs30);

		int nDetailMode = 0;
		if(bHasDetail && nDetailBlendMode == DETAILBLENDMODE_SELFILLUM_ADDITIVE)
			nDetailMode = 1;
		else if (bHasDetail && nDetailBlendMode == DETAILBLENDMODE_SELFILLUM_THRESHOLDFADE)
			nDetailMode = 2;

		DECLARE_STATIC_PIXEL_SHADER(lux_emissive_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVEBLEND, bEmissiveBlend + bEmissiveBlend * g_pHardwareConfig->NeedsShaderSRGBConversion());
		SET_STATIC_PIXEL_SHADER_COMBO(NOFLOW, bHasNoFlow);
		SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMTEXTURE, bHasSelfIllumTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailMode);
		SET_STATIC_PIXEL_SHADER_COMBO(MINIMUMLIGHT, bMinimumLight);
		SET_STATIC_PIXEL_SHADER(lux_emissive_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(pShader->IsDynamicState())
	{
		// Secondary Pass, reset.
		pShaderAPI->SetDefaultState();

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0, s1, s2 - Emissive Blend
		if (bEmissiveBlend)
		{
			bool bHasBaseTexture = pShader->IsTextureLoaded(info.m_nEmissiveBlendBaseTexture);
			bool bHasEmissiveTexture = pShader->IsTextureLoaded(info.m_nEmissiveBlendTexture);

			// Need Fallbacks.
			pShader->BindTexture(bHasBaseTexture, SHADER_SAMPLER0, info.m_nEmissiveBlendBaseTexture, -1, TEXTURE_WHITE);
			pShader->BindTexture(bHasFlowTexture, SHADER_SAMPLER1, info.m_nEmissiveBlendFlowTexture, -1, TEXTURE_BLACK);
			pShader->BindTexture(bHasEmissiveTexture, SHADER_SAMPLER2, info.m_nEmissiveBlendTexture, -1, TEXTURE_WHITE);
		}

		// s3 - $SelfIllumTexture
		if(bHasSelfIllumTexture)
			pShader->BindTexture(SHADER_SAMPLER3, info.SelfIllum.m_nSelfIllumTexture, info.SelfIllum.m_nSelfIllumTextureFrame);

		// s4 - $Detail
		if (bHasDetail)
			pShader->BindTexture(SHADER_SAMPLER4, info.Detail.m_nDetail, info.Detail.m_nDetailFrame);

		// s5 - $BaseTexture
		if (bMinimumLight)
			pShader->BindTexture(SHADER_SAMPLER5, info.Base.m_nBaseTexture, info.Base.m_nFrame);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS c223, c224, c225, c226
		if(bEmissiveBlend)
		{
			// Stock Shader had no Transform for this.
			pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.m_nEmissiveBlendTransform);

			// Makes it work like a second $DetailBlendMode 5.. or err third. Considering SelfIllumTexture
			if (bHasNoFlow)
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, info.m_nEmissiveBlend_NoFlowTransform);
		}

		// VS c227, c228
		if (bHasDetail)
		{
			if (pShader->HasTransform(true, info.Detail.m_nDetailTextureTransform))
				pShader->SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_03, info.Detail.m_nDetailTextureTransform, info.Detail.m_nDetailScale);
			else
				pShader->SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_03, info.Base.m_nBaseTextureTransform, info.Detail.m_nDetailScale);	
		}

		// VS c229, c230
		if (bHasSelfIllumTexture)
			pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, info.Base.m_nBaseTextureTransform);

		// VS c231, c232
		if(bMinimumLight)
			pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_05, info.Base.m_nBaseTextureTransform);

		// c19
		if(bEmissiveBlend)
			pShader->SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		if(bMinimumLight)
		{
			float4 f4MinimumLight = 0.0f;
			f4MinimumLight.rgb = pShader->GetFloat3(info.m_nMinimumLightTint);
		}

		// c33, c34
		if (bHasDetail)
		{
			// $DetailTint and $DetailBlendFactor
			float4 f4DetailTint_BlendFactor;
			f4DetailTint_BlendFactor.xyz = pShader->GetFloat3(info.Detail.m_nDetailTint);
			f4DetailTint_BlendFactor.w = pShader->GetFloat(info.Detail.m_nDetailBlendFactor);
			f4DetailTint_BlendFactor = pShader->PrecomputeDetail(f4DetailTint_BlendFactor, nDetailBlendMode);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4DetailTint_BlendFactor);

			float4 f4DetailBlendMode = 0.0f; // yzw empty
			f4DetailBlendMode.x = (float)nDetailBlendMode;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4DetailBlendMode);	
		}

		if(bEmissiveBlend)
		{
			// Need Gamma
			pShader->SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

			// "This brings in the electricity and the second base texture when the second base texture is present"
			// ShiroDkxtro2 :	The default $BlendStrength is 0, that's not very cool.
			//					However, it's important for stock Materials not to break..
			//					And by stock Materials I mean the single one Material that even used this...
			//					Vortigaunt_blue.vmt, where it's controlled by a Proxy
			float	f1BlendStrength = pShader->GetFloat(info.m_nEmissiveBlendStrength);
					f1BlendStrength = saturate(f1BlendStrength);
			
			// NOTE :	Stock Shader checks Param IsDefined() && > 0
			//			If you have a Sinewave hooked to your Time and it reaches 0 it will just randomly jump to CurTime.
			//			Congratulations, you are now forced to set a min value of 0.000001 so it doesn't do that...
			//			Additionally, the Defined check always returns true since this is after Shader Init
			//			Just check if it's *not* the default Value. I set it to -123
			//			If the Value was modified by a Proxy or User Defined, it will be != 123.
			float f1Time = pShader->GetFloat(info.m_nTime);
			if (f1Time == -123.0f)
				f1Time = pShaderAPI->CurrentTime();

			// Stock-Consistency "Time % 1000 for scrolling"
			f1Time -= (float)((int)(f1Time / 1000.0f)) * 1000.0f;

			// Stock-Consistency - Magic Numbers
			float4 f4EmissiveScroll = float4(0.11f, 0.124f, 0.0f, 0.0f);

			// This Check is pointless since it will always be true
			// Just force the Values from the Parameter
//			if (pShader->IsDefined(info.m_nEmissiveBlendScrollVector))
			f4EmissiveScroll.xy = pShader->GetFloat2(info.m_nEmissiveBlendScrollVector);

			// Precompute this, it's the same for all Pixels!
			f4EmissiveScroll.xy *= f1Time;

			// Stock Shader did some weird shenanigans checking if the Parameter is defined
			// Color Params have a default Value of [1 1 1], so don't bother checking.. just get the Values!
			float4 f4EmissiveTint = 0.0f;
			f4EmissiveTint.xyz = pShader->GetFloat3(info.m_nEmissiveBlendTint);
			f4EmissiveTint.xyz *= f1BlendStrength; // Precompute this. It's the same for all Pixels!

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_036, f4EmissiveTint);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_037, f4EmissiveScroll);
		}

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_emissive_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_emissive_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_emissive_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_emissive_ps30);
	}

	pShader->Draw();
}