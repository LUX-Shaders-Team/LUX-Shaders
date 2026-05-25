//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	13.05.2026 DMY
//	Last Change :	13.05.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "RimLightPass.h"
#include "../../lux_rimlightpass_registermap.h"

// Includes for Shaderfiles...
#include "lux_rimlightpass_vs30.inc"
#include "lux_rimlightpass_ps30.inc"

void RimLightPass_Init_Params(CBaseVSShader* pShader, RimLightPass_Vars_t& info)
{
	if (!pShader->GetBool(info.m_nEnabled))
		return;

	// NOTE: Caller must set HW Skinning and other Flags that apply	

	pShader->DefaultFloat3(info.m_nFresnelRanges, 0.0f, 0.5f, 1.0f);
}

void RimLightPass_Shader_Init(CBaseVSShader* pShader, RimLightPass_Vars_t& info)
{
	pShader->LoadTexture(info.m_nMask, TEXTUREFLAGS_SRGB); // I want this to be a per-Pixel Tint 
	pShader->LoadTexture(info.m_nCubemap, TEXTUREFLAGS_SRGB); // Expecting custom non-HDR EnvMaps
}

void RimLightPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, RimLightPass_Vars_t& info)
{
	if(!pShader->GetBool(info.m_nEnabled))
		return;

	// Instantly abort if we are on any kind of projected Texture Pass
	// We can't use $ReceiveProjectedTextures since this is a second Pass
	else if (pShader->HasFlashlight())
	{
		pShader->Draw(false);
		return;
	}

	// Determine whether $BaseTexture has Opacity.
	bool bNeedsBaseForOpacity = false;
	BlendType_t nBlendType = pShader->EvaluateBlendRequirements(info.Base.m_nBaseTexture, true);
	if (nBlendType == BT_BLEND || nBlendType == BT_BLENDADD)
	{
		bNeedsBaseForOpacity = true;
	}

	bool bHasNormalMap = pShader->GetBool(info.m_nUseBumpMap) && pShader->IsTextureLoaded(info.Normal.m_nBumpMap);
	bool bHasCubemap = pShader->IsTextureLoaded(info.m_nCubemap);
	bool bUseAmbientCube = !bHasCubemap;
	bool bHasMask = pShader->IsTextureLoaded(info.m_nMask);

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

		// Want regular Brightness
		pShaderShadow->EnableSRGBWrite(true);

		// This Shader is additive, make Fog Black so Things disappear into the Fog
		pShader->FogToBlack();

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = bHasNormalMap ? 4 : 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		if (bNeedsBaseForOpacity)
			pShader->EnableSampler(SHADER_SAMPLER0, true);

		// Mask should work like a Tint, Artists will probably make these in Photoshop and export as sRGB
		if(bHasMask)
			pShader->EnableSampler(SHADER_SAMPLER1, true);

		if(bHasNormalMap)
			pShader->EnableSampler(SHADER_SAMPLER2, false);

		// Always expecting custom sRGB Cubemaps
		if(!bUseAmbientCube)
			pShader->EnableSampler(SHADER_SAMPLER3, true);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		int nRequiredTexCoords = (bNeedsBaseForOpacity || bHasMask) + bHasNormalMap;

		DECLARE_STATIC_VERTEX_SHADER(lux_rimlightpass_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nRequiredTexCoords);
		SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bHasNormalMap);
		SET_STATIC_VERTEX_SHADER(lux_rimlightpass_vs30);

		// bNeedsBaseForOpacity
		DECLARE_STATIC_PIXEL_SHADER(lux_rimlightpass_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(NORMALMAP, bHasNormalMap);
		SET_STATIC_PIXEL_SHADER_COMBO(USEAMBIENTCUBE, bUseAmbientCube);
		SET_STATIC_PIXEL_SHADER_COMBO(BASETEXTUREOPACITY, bNeedsBaseForOpacity);
		SET_STATIC_PIXEL_SHADER_COMBO(MASKED, bHasMask);
		SET_STATIC_PIXEL_SHADER(lux_rimlightpass_ps30);

#ifdef ASWSDK
		if(bUseAmbientCube && !pShader->GetBool(info.m_nUseCustomAmbientCube))
		{
			pShader->PI_BeginCommandBuffer();
			pShader->PI_SetPixelShaderAmbientLightCube(LUX_PS_FLOAT_RIMLIGHTPASS_AMBIENTCUBE);
			pShader->PI_EndCommandBuffer();
		}
#endif
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

		if (bNeedsBaseForOpacity)
			pShader->BindTexture(SHADER_SAMPLER0, info.Base.m_nBaseTexture, info.Base.m_nFrame);

		// Mask should work like a Tint, Artists will probably make these in Photoshop and export as sRGB
		if (bHasMask)
			pShader->BindTexture(SHADER_SAMPLER1, info.m_nMask, info.m_nMaskFrame);

		if (bHasNormalMap)
			pShader->BindTexture(SHADER_SAMPLER2, info.Normal.m_nNormalTexture, info.Normal.m_nBumpFrame);

		if (bHasCubemap)
			pShader->BindTexture(SHADER_SAMPLER3, info.m_nCubemap, info.m_nCubemapFrame);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		// Also used for the Mask
		int nRegisterShift = 0;
		if(bNeedsBaseForOpacity || bHasMask)
		{
			pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.Base.m_nBaseTextureTransform);

			nRegisterShift += 2;

			// c225 used by cFlexScales, skip over it on SFM
			#ifdef SFM_COMPATIBILITY
				nRegisterShift += 1;
			#endif
		}

		// VS c223, c224, c225, c226
		if (bHasNormalMap)
		{
			bool bNormalTextureTransform = pShader->HasTransform(true, info.Normal.m_nBumpTransform);
			if (bNormalTextureTransform)
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01 + nRegisterShift, info.Normal.m_nBumpTransform);
			else
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01 + nRegisterShift, info.Base.m_nBaseTextureTransform);
		}

		float4 cTint = 1.0f;
		cTint.rgb = pShader->GetFloat3(info.m_nTint);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_RIMLIGHTPASS_TINT, cTint);

		// "Change fresnel range encoding from (min, mid, max) to ((mid-min)*2, mid, (max-mid)*2)"
		float4 cFresnelRanges = 0.0f;
		cFresnelRanges.xyz = pShader->GetFloat3(info.m_nFresnelRanges);
		cFresnelRanges.x = (cFresnelRanges.y - cFresnelRanges.x) * 2;
		cFresnelRanges.z = (cFresnelRanges.z - cFresnelRanges.y) * 2;
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_RIMLIGHTPASS_FRESNELRANGES, cFresnelRanges);

		if (bUseAmbientCube)
		{
			if(pShader->GetBool(info.m_nUseCustomAmbientCube))
			{
				float4 cAmbientCube[6];
				cAmbientCube[0].rgb = pShader->GetFloat3(info.m_nAmbientCubeXP);
				cAmbientCube[1].rgb = pShader->GetFloat3(info.m_nAmbientCubeXN);
				cAmbientCube[2].rgb = pShader->GetFloat3(info.m_nAmbientCubeYP);
				cAmbientCube[3].rgb = pShader->GetFloat3(info.m_nAmbientCubeYN);
				cAmbientCube[4].rgb = pShader->GetFloat3(info.m_nAmbientCubeZP);
				cAmbientCube[5].rgb = pShader->GetFloat3(info.m_nAmbientCubeZN);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_RIMLIGHTPASS_AMBIENTCUBE, *cAmbientCube, 6);
			}
#ifndef ASWSDK
			else
			{
				pShaderAPI->SetPixelShaderStateAmbientLightCube(LUX_PS_FLOAT_RIMLIGHTPASS_AMBIENTCUBE);
			}
#endif
		}

		// c26 - EyePos
		pShader->SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c27 - Fog Data
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c28
		if (bNeedsBaseForOpacity)
			pShader->SetModulationConstant(false, false);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_rimlightpass_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_rimlightpass_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_rimlightpass_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_rimlightpass_ps30);
	}

	pShader->Draw();
}