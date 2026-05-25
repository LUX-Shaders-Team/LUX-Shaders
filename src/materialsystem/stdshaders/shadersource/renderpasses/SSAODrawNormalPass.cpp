//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	18.05.2026 DMY
//	Last Change :	25.05.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "SSAODrawNormalPass.h"

// Includes for Shaderfiles...
#include "lux_ssao_drawnormal_vs30.inc"
#include "lux_ssao_drawnormal_ps30.inc"

void SSAONormalPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, SSAODrawNormalPass_Vars_t& info)
{
#ifndef ASWSDK
	return;
#else
	// We cannot Mask this Pass by Opacity since AlphaWrites are already used. Any non-Opaque Surface has to draw nothing
	// AlphaTested Materials could theoratically use Clip() to make use of Opacity while having AlphaWrites.
	// I'm worried about the previously written Results though.. Probably better not to do this
	bool bTranslucent = pShader->HasFlag(MATERIAL_VAR_TRANSLUCENT) && !pShader->GetBool(PretendTranslucent);
	if(bTranslucent || pShader->HasFlag(MATERIAL_VAR_ALPHATEST) || pShader->HasFlag(MATERIAL_VAR_ADDITIVE))
	{
		pShader->Draw(false);
		return;
	}

	bool bIsModel = (info.m_bIsModel == 1);
	bool bHasNormalMap = info.m_nBumpMap != -1 && pShader->IsTextureLoaded(info.m_nBumpMap);
	bool bHasNormalMap2 = !bIsModel && info.m_nBumpMap2 != -1 && pShader->IsTextureLoaded(info.m_nBumpMap2);
	bool bHasBlendModulate = !bIsModel && bHasNormalMap2 && info.m_nBlendModulateTexture != -1 && pShader->IsTextureLoaded(info.m_nBlendModulateTexture);
	bool bHasWrinkleMaps = bIsModel;
	bHasWrinkleMaps = bHasWrinkleMaps && (info.m_nBumpCompress != -1) && pShader->IsTextureLoaded(info.m_nBumpCompress);
	bHasWrinkleMaps = bHasWrinkleMaps && (info.m_nBumpStretch != -1) && pShader->IsTextureLoaded(info.m_nBumpStretch);

	// In Case of Displacements it's possible that there is a $BumpMap2 but not a $BumpMap
	if(bHasNormalMap2)
		bHasNormalMap = true;

	int nTreeSway = (info.TreeSwayVars.m_nTreeSway != -1) && pShader->GetInt(info.TreeSwayVars.m_nTreeSway);

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

		// Need Alpha for SSAO Factor
		pShaderShadow->EnableAlphaWrites(true);

		// Writing linear Values
		pShaderShadow->EnableSRGBWrite(true);

		// No Fog
		pShader->DisableFog();

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		
		if(bIsModel)
		{
			nUserDataSize = bHasNormalMap ? 4 : 0;
			nFlags |= VERTEX_FORMAT_COMPRESSED;
		}
		else
		{
			nTexCoords = 2;
			if(bHasNormalMap)
				nFlags |= VERTEX_TANGENT_SPACE;

			if(bHasNormalMap2)
				nFlags |= VERTEX_COLOR;
		}
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// First Normal Map
		if (bHasNormalMap)
			pShader->EnableSampler(SHADER_SAMPLER0, false);

		// Second Normal Map, or $BlendModulateTexture
		if(bHasNormalMap2)
		{
			pShader->EnableSampler(SHADER_SAMPLER1, false);
		
			if(bHasBlendModulate)
				pShader->EnableSampler(SHADER_SAMPLER2, false);
		}
		// Models can have $BumpCompress and $BumpStretch
		else if(bHasWrinkleMaps)
		{
			pShader->EnableSampler(SHADER_SAMPLER1, false);
			pShader->EnableSampler(SHADER_SAMPLER2, false);
		}

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_ssao_drawnormal_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(BRUSH_OR_DISPLACEMENT, !bIsModel);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALMAPS, bHasNormalMap + bHasNormalMap2);
		SET_STATIC_VERTEX_SHADER_COMBO(BLENDMODULATE, bHasBlendModulate);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, bHasNormalMap + bHasWrinkleMaps); // 1 for Tangents, 2 for WrinkleWeight
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
		SET_STATIC_VERTEX_SHADER(lux_ssao_drawnormal_vs30);

		// bNeedsBaseForOpacity
		DECLARE_STATIC_PIXEL_SHADER(lux_ssao_drawnormal_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(NORMALMAPS, bHasNormalMap + bHasNormalMap2);
		SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATE, bHasBlendModulate);
		SET_STATIC_PIXEL_SHADER_COMBO(WRINKLEMAPPING, bHasWrinkleMaps);
		SET_STATIC_PIXEL_SHADER(lux_ssao_drawnormal_ps30);
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

		// First Normal Map
		if (bIsModel)
		{
			if(bHasNormalMap)
				pShader->BindTexture(SHADER_SAMPLER0, info.m_nBumpMap, info.m_nBumpMapFrame);

			if(bHasWrinkleMaps)
			{
				pShader->BindTexture(SHADER_SAMPLER1, info.m_nBumpCompress, info.m_nBumpMapFrame);
				pShader->BindTexture(SHADER_SAMPLER2, info.m_nBumpStretch, info.m_nBumpMapFrame);
			}
		}
		else
		{
			// If no $BumpMap reuse $BumpMap2
			// Else use $BumpMap if it's loaded
			bool bNormalMap1Loaded = (info.m_nBumpMap != -1) && pShader->IsTextureLoaded(info.m_nBumpMap);
			if(bHasNormalMap2 && !bNormalMap1Loaded)
				pShader->BindTexture(SHADER_SAMPLER0, info.m_nBumpMap2, info.m_nBumpMapFrame2);
			else if(bNormalMap1Loaded)
				pShader->BindTexture(SHADER_SAMPLER0, info.m_nBumpMap, info.m_nBumpMapFrame);

			if(bHasNormalMap2)
				pShader->BindTexture(SHADER_SAMPLER1, info.m_nBumpMap2, info.m_nBumpMapFrame2);

			if(bHasBlendModulate)
				pShader->BindTexture(SHADER_SAMPLER2, info.m_nBlendModulateTexture, info.m_nBlendModulateFrame);
		}

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		if(!bIsModel)
		{
			// Account for BumpTransform2?
			if (!pShader->m_ppParams[info.m_nBumpMapTransform]->MatrixIsIdentity())
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.m_nBumpMapTransform);
			else
				pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.BaseVars.m_nBaseTextureTransform);

			if(bHasNormalMap2)
			{
				if (!pShader->m_ppParams[info.m_nBumpMapTransform2]->MatrixIsIdentity())
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, info.m_nBumpMapTransform2);
				else if (!pShader->m_ppParams[info.m_nBumpMapTransform]->MatrixIsIdentity())
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, info.m_nBumpMapTransform);
				else
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, info.BaseVars.m_nBaseTextureTransform);
			}

			if(bHasBlendModulate)
			{
				if (!pShader->m_ppParams[info.m_nBlendModulateTransform]->MatrixIsIdentity())
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, info.m_nBlendModulateTransform);
				else
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, info.BaseVars.m_nBaseTextureTransform);
			}
		}
		else
		{
			if (bHasNormalMap)
			{
				if (pShader->m_ppParams[info.m_nBumpMapTransform]->MatrixIsIdentity())
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.m_nBumpMapTransform);
				else
					pShader->SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, info.BaseVars.m_nBaseTextureTransform);
			}
		}

		// Always need this Data, this Shader is used for nothing else but this Data
//		int nLightingPreviewMode = m_pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING);
//		if (nLightingPreviewMode == ENABLE_FIXED_LIGHTING_OUTPUTNORMAL_AND_DEPTH)
		{
			float4 cViewDir = 0.0f;
			pShaderAPI->GetWorldSpaceCameraDirection(cViewDir);

			float flFarZ = pShaderAPI->GetFarZ();
			cViewDir.xyz /= flFarZ;
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET0_0, cViewDir);
		}

		// VS c48, c49, c50, c51, c52
		// Treesway Implementation
		if (nTreeSway != 0)
		{
			float4 f4SwayParams1; // c241
			f4SwayParams1.x = pShaderAPI->CurrentTime(); // f1Time
			f4SwayParams1.y = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayScrumbleFalloffExp);
			f4SwayParams1.z = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayFalloffExp);
			f4SwayParams1.w = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayScrumbleSpeed);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_01, f4SwayParams1);

			float4 f4SwayParams2; // c242
			f4SwayParams2.x = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwaySpeedHighWindMultiplier);
			f4SwayParams2.y = f4SwayParams2.x * 2.14f; // Precompute this
			if (pShader->GetBool(info.TreeSwayVars.m_nTreeSwayStatic) || lux_treesway_force_static.GetBool())
			{
				if (lux_treesway_static_override.GetBool())
				{
					f4SwayParams2.z = lux_treesway_static_x.GetFloat();
					f4SwayParams2.w = lux_treesway_static_y.GetFloat();
				}
				else
					f4SwayParams2.zw = pShader->GetFloat2(info.TreeSwayVars.m_nTreeSwayStaticValues);
			}
			else
			{
				const Vector& windDir = pShaderAPI->GetVectorRenderingParameter(VECTOR_RENDERPARM_WIND_DIRECTION);
				f4SwayParams2.z = windDir.x;
				f4SwayParams2.w = windDir.y;
			}
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_02, f4SwayParams2);

			// We need to precompute these into something else
			float f1Height		= pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayHeight);
			float f1StartHeight = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayStartHeight);
			float f1Radius		= pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayRadius);
			float f1StartRadius = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayStartRadius);

			// Compute Multiplication for the whole object
			// Same for inverse height and reciprocal
			float4 f4SwayParams3;
			f4SwayParams3.x = f1Height * f1StartHeight;						// f1Height_MUL
			f4SwayParams3.y = 1.0f / ((1.0f - f1StartHeight) * f1Height);	// f1Height_RCP
			f4SwayParams3.z = f1Radius * f1StartRadius;						// f1Radial_MUL
			f4SwayParams3.w = 1.0f / ((1.0f - f1StartRadius) * f1Radius);	// f1Radial_RCP
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_03, f4SwayParams3);

			float4 f4SwayParams4;
			f4SwayParams4.x = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwaySpeed);
			f4SwayParams4.y = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayStrength);
			f4SwayParams4.z = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayScrumbleFrequency);
			f4SwayParams4.w = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwayScrumbleStrength);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_04, f4SwayParams4);

			// More precomputation
			float4 f4SwayParams5;
			f4SwayParams5.x		= sqrtf(f4SwayParams2.z * f4SwayParams2.z + f4SwayParams2.w * f4SwayParams2.w); // Length of the Wind Direction
			float SpeedLerpLow	= pShader->GetFloat(info.TreeSwayVars.m_nTreeSwaySpeedLerpStart);
			float SpeedLerpHigh = pShader->GetFloat(info.TreeSwayVars.m_nTreeSwaySpeedLerpEnd);
			f4SwayParams5.y		= smoothstep(SpeedLerpLow, SpeedLerpHigh, f4SwayParams5.x);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_05, f4SwayParams5);
		}

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_ssao_drawnormal_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_ssao_drawnormal_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_ssao_drawnormal_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_ssao_drawnormal_ps30);
	}

	pShader->Draw();
#endif // ASWSDK
}