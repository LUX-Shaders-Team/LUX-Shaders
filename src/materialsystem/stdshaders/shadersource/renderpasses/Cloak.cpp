//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	CloakPass from Team Fortress 2
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "Cloak.h"

// Includes for Shaderfiles...
#include "lux_cloak_vs30.inc"
#include "lux_cloak_ps30.inc"

bool CloakBlend_IsOpaque(CBaseVSShader *pShader, IMaterialVar **params, Cloak_Vars_t &info)
{
	float f1CloakFactor = pShader->GetFloat(info.m_nCloakFactor);

	// If this math changes, update the Pixelshader!
	float f1Fresnel = 1.0f; // Assume V.N = 0.0f;
	float f1CloakLerpFactor = clamp(Lerp(clamp(f1CloakFactor, 0.0f, 1.0f), 1.0f, f1Fresnel - 1.35f), 0.0f, 1.0f);
	//flCloakLerpFactor = 1.0f - smoothstep( 0.4f, 0.425f, flCloakLerpFactor );

	if (f1CloakLerpFactor <= 0.4f)
		return true;

	return false;
}

void CloakBlend_Init_Params(CBaseVSShader *pShader, Cloak_Vars_t &info)
{
	if (!pShader->GetBool(info.m_nCloakEnabled))
		return;

	// Always a Model
	pShader->SetFlag(MATERIAL_VAR_MODEL);

	// Always need to set this, for animated Models
	pShader->SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// Always need Tangents for BumpMapping
	pShader->SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// We should have this..
//	SetFlag2(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE);
//	SetFlag2(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);

	pShader->DefaultFloat(info.m_nRefractAmount, 0.1f);
}

void CloakBlend_Shader_Init(CBaseVSShader* pShader, Cloak_Vars_t& info)
{
	// NOTE:	Stock uses LoadTexture, not LoadBumpMap
	//			Is that intentional?
	//			In either case we don't need it here!!
	//			If a BumpMap is set, the main Shader should already have loaded it
}

void CloakBlend_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, Cloak_Vars_t& info)
{
	bool bUseCloak = pShader->GetBool(info.m_nCloakEnabled);

	// Not using Cloak? Don't bother!
	if (!bUseCloak)
		return;

	// Additional Renderpasses never get Projected Textures
	// Indicate Draw(false) if we would otherwise have rendered Cloak
	if (pShader->UsingFlashlight() && bUseCloak)
	{
		pShader->Draw(false);
		return;
	}

	bool bHasNormalVars = info.Bump.m_nBumpMap != -1;
	bool bHasNormalMap = bHasNormalVars && pShader->IsTextureLoaded(info.Bump.m_nBumpMap);

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

		// Translucent
		pShader->EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

		// Stock Shader doesn't do this, but we don't want AlphaWrites
		pShaderShadow->EnableAlphaWrites(false);

		// "!!! We need to turn this back on because EnableAlphaBlending() above disables it!"
		pShaderShadow->EnableDepthWrites(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Stock shader sets a UserDataSize of 0, even though we want Tangents
		// That breaks Vertex Compression as vUserData contains the Tangent Data for uncompressed Vertices
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int TexCoords = 1;
		int nUserDataSize = bHasNormalMap ? 4 : 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, TexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - Framebuffer Texture - Always sRGB - NOTE: Why does this not consider HDR like the Core Shader?
		pShader->EnableSampler(SHADER_SAMPLER0, true);

		// s2 - $BumpMap - Never sRGB
		pShader->EnableSampler(bHasNormalMap, SHADER_SAMPLER2, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_cloak_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(BUMPMAPPED, bHasNormalMap);
		SET_STATIC_VERTEX_SHADER(lux_cloak_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_cloak_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(BUMPMAPPED, bHasNormalMap);
		SET_STATIC_PIXEL_SHADER_COMBO(LINEARTOGAMMA, g_pHardwareConfig->NeedsShaderSRGBConversion());
		SET_STATIC_PIXEL_SHADER(lux_cloak_ps30);
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
		
		// s0 - Framebuffer Texture
		pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

		// s2 - $BumpMap
		if (bHasNormalMap)
			pShader->BindTexture(SHADER_SAMPLER2, info.Bump.m_nBumpMap, info.Bump.m_nBumpFrame);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS - c225
		if(bHasNormalMap)
			pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.Bump.m_nBumpTransform);

		// PS - c11
		pShader->SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// Need Gamma when converting manually from Linear to Gamma
		if(g_pHardwareConfig->NeedsShaderSRGBConversion())
			pShader->SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// Get all the values first
		float f1CloakFactor = saturate(pShader->GetFloat(info.m_nCloakFactor));
		float f1RefractFactor = pShader->GetFloat(info.m_nRefractAmount);

		float4 f4CloakTint_Factor;
		f4CloakTint_Factor.xyz = pShader->GetFloat3(info.m_nCloakColorTint);

		// Precompute this
		// fColorTintStrength = saturate( ( saturate( g_flCloakFactor ) - 0.75f ) * 4.0f );
		// cRefract.rgb *= lerp( g_cCloakColorTint, 1.0f, fColorTintStrength );
		float f1TintStrength = saturate((f1CloakFactor - 0.75f) * 4.0f);
		f4CloakTint_Factor.xyz = lerp(f4CloakTint_Factor.xyz, float3(1.0f, 1.0f, 1.0f), f1TintStrength);
		f4CloakTint_Factor.w = f1CloakFactor;
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_032, f4CloakTint_Factor);

		// Precompute this:
		// saturate(g_flCloakFactor) * saturate(g_flCloakFactor)
		// and this:
		// lerp(g_flRefractAmount, 0.0f, saturate(g_flCloakFactor));
		// and this:
		// lerp(0.05f, 0.0f, saturate(f1CloakFactor))
		float4 f4Controls;
		f4Controls.x = f1CloakFactor * f1CloakFactor; // CloakFactor squared
		f4Controls.y = lerp(f1RefractFactor, 0.0f, f1CloakFactor);
		f4Controls.z = lerp(0.05f, 0.0f, saturate(f1CloakFactor));
		f4Controls.w = 0.0f;	
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_033, f4Controls);

		// Set c34 and c35 to contain first two rows of ViewProj matrix
		VMatrix mView, mProj;
		pShaderAPI->GetMatrix(MATERIAL_VIEW, mView.m[0]);
		pShaderAPI->GetMatrix(MATERIAL_PROJECTION, mProj.m[0]);
		VMatrix mViewProj = mView * mProj;
		mViewProj = mViewProj.Transpose3x3();
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_034, mViewProj.m[0], 2);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_cloak_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_cloak_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_cloak_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_cloak_ps30);
	}

	pShader->Draw();
}