//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	11.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "SheenPass.h"

// Includes for Shaderfiles...
#include "lux_model_simplified_vs30.inc"
#include "lux_sheenpass_ps30.inc"

void SheenPass_Init_Params(CBaseVSShader* pShader, SheenPass_Vars_t& info)
{
	if (!pShader->GetBool(info.m_nSheenPassEnabled))
		return;

	// Always a Model
	pShader->SetFlag(MATERIAL_VAR_MODEL);

	// Always need to set this, for animated Models
	pShader->SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// Always need Tangents for BumpMapping
	pShader->SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	pShader->DefaultFloat(info.m_nSheenMapMaskScaleX, 1.0f);
	pShader->DefaultFloat(info.m_nSheenMapMaskScaleY, 1.0f);
}

void SheenPass_Shader_Init(CBaseVSShader* pShader, SheenPass_Vars_t& info)
{
	// NOTE:	Stock uses LoadTexture, not LoadBumpMap
	//			Is that intentional?
	//			In either case we don't need it here!!
	//			If a BumpMap is set, the main Shader should already have loaded it

	// We do need to load these two.
	bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;
	pShader->LoadCubeMap(info.m_nSheenMap, bHDR ? 0 : TEXTUREFLAGS_SRGB);
	pShader->LoadTexture(info.m_nSheenMapMask);
}

void SheenPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, SheenPass_Vars_t& info)
{
	bool bUseSheen = pShader->GetBool(info.m_nSheenPassEnabled);

	// Early exit.
	if (!bUseSheen)
		return;

	// Stock-Consistency: Only do Sheen with Non-Zero Frame
	// Must do SnapShot when pShaderShadow even when Frame is zero
	bUseSheen = pShaderShadow || pShader->GetInt(info.m_nSheenMapMaskFrame) > 0;

	// If Sheen is enabled, but doesn't meet the Requirements, draw nothing.
	if (!bUseSheen)
	{
		pShader->Draw(false);
		return;
	}

	// Instantly abort if we are on any kind of projected Texture Pass
	// We can't use $ReceiveProjectedTextures since this is a second Pass
	if (pShader->UsingFlashlight() && bUseSheen)
	{
		pShader->Draw(false);
	}
	else if (bUseSheen)
	{
		bool bHasNormalVars = info.m_nBumpMap != -1;
		bool bHasNormalMap = bHasNormalVars && pShader->IsTextureLoaded(info.m_nBumpMap);

		//==========================================================================//
		// Static Snapshot of the Shader Settings
		//==========================================================================//
		if (pShader->IsSnapshotting())
		{
			//==========================================================================//
			// General Rendering Setup
			//==========================================================================//

			// "Reset shadow state manually since we're drawing from two Materials"
			pShader->SetInitialShadowState();

			// Always Linear
			pShaderShadow->EnableSRGBWrite(true);

			// Blending
			pShader->EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			pShaderShadow->EnableAlphaWrites(false);

			// "!!! We need to turn this back on because EnableAlphaBlending() above disables it!"
			// Ok but why? the Depth written by the BasePass should already handle this, we just care about depth tests
			pShaderShadow->EnableDepthWrites(true);

			//==========================================================================//
			// Vertex Shader - Vertex Format
			//==========================================================================//

			// Stock shader states that it wants MATERIAL_VAR2_NEEDS_TANGENT_SPACES
			// But here we set a UserDataSize of 0
			// Does this only work with vertex compression? Might be a FIXME..
			unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			int TexCoords = 1;
			int nUserDataSize = 0;
			pShaderShadow->VertexShaderVertexFormat(nFlags, TexCoords, NULL, nUserDataSize);

			//==========================================================================//
			// Sampler Setup
			//==========================================================================//

			// Stock-Consistency:
			// The sRGBRead here doesn't make any sense.
			// SheenMap is a Cubemap, so I guess that is fine ( even if it's probably a stored-on-disk Texture )
			// But the SheenMapMask? It doesn't change based on HDR Type like Cubemaps, does it?
			// I'm replicating Stock-Behaviour here but it doesn't make a lot of sense.
			bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;

			// s0 - SheenMapMask
			pShader->EnableSampler(SHADER_SAMPLER0, !bHDR);

			// s1 - SheenMap
			pShader->EnableSampler(SHADER_SAMPLER1, !bHDR);

			// s2 - $BumpMap. Never sRGB
			pShader->EnableSampler(bHasNormalMap, SHADER_SAMPLER2, false);

			//==========================================================================//
			// Set Static Shaders
			//==========================================================================//
			DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, bHasNormalMap ? 2 : 1);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_sheenpass_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(BUMPMAPPED, bHasNormalMap);
			SET_STATIC_PIXEL_SHADER(lux_sheenpass_ps30);
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

			// No Checks, nothin'
			// s0, s1
			pShader->BindTexture(SHADER_SAMPLER0, info.m_nSheenMapMask, info.m_nSheenMapMaskFrame);
			pShader->BindTexture(SHADER_SAMPLER1, info.m_nSheenMap, -1);

			if (bHasNormalMap)
			{
				pShader->BindTexture(SHADER_SAMPLER2, info.m_nBumpMap, info.m_nBumpFrame);
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.m_nBumpTransform);
			}

			//==========================================================================//
			// Setup Constant Registers
			//==========================================================================//

			// c11
			pShader->SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

			// c19
			// Need Gamma when converting manually from Linear to Gamma
			if(g_pHardwareConfig->NeedsShaderSRGBConversion())
				pShader->SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

			// c32
			float4 f4SheenTint_Dir;
			f4SheenTint_Dir.rgb = pShader->GetFloat3(info.m_nSheenMapTint);
			f4SheenTint_Dir.w = pShader->GetFloat(info.m_nSheenMapMaskDirection);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_032, f4SheenTint_Dir);

			// c33
			float4 f4SheenOffsetScale;
			f4SheenOffsetScale.x = pShader->GetFloat(info.m_nSheenMapMaskOffsetX);
			f4SheenOffsetScale.y = pShader->GetFloat(info.m_nSheenMapMaskOffsetY);
			f4SheenOffsetScale.z = 1.0f / pShader->GetFloat(info.m_nSheenMapMaskScaleX); // Precompute rcp()
			f4SheenOffsetScale.w = 1.0f / pShader->GetFloat(info.m_nSheenMapMaskScaleY);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_033, f4SheenOffsetScale);

			//==================================================================================================
			// Set Dynamic Shaders
			//==================================================================================================
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_sheenpass_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_sheenpass_ps30);
		}

		pShader->Draw();
	}
}