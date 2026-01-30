//===================== File of the LUX Shader Project =====================//
//
//	Original D. :	20.01.2023 DMY
//	Initial D.	:	10.12.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_brush_simplified_vs30.inc"
#include "lux_sky_simple_ps30.inc"
#include "lux_sky_rgbs_vs30.inc"
#include "lux_sky_rgbs_ps30.inc"
#include "lux_sky_compressed_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_SKY_HDR_DX9, LUX_Sky_Complex)
#endif

#ifdef REPLACE_SKY
DEFINE_FALLBACK_SHADER(SKY_HDR_DX9, LUX_Sky_Complex)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Sky_Complex, "Renders LDR and HDR Skybox Textures onto a Surface." )
SHADER_INFO_GEOMETRY	("Should only be applied using Map Properties.\nIf you must apply to World Geometry: Brushes.")
SHADER_INFO_USAGE		("Variable.\n"
						 "$HDRCompressedTexture, for RGBS Compressed Textures.\n"
						 "$HDRCompressedTexture0 to 2, for compression with multiple Textures.\n"
						 "$HDRBaseTexture, for no compression. ( HDR and LDR Formats supported )")
SHADER_INFO_LIMITATIONS	("Does not allow rotation of the Skybox.")
SHADER_INFO_PERFORMANCE	("Fast.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Sky Shader Page: https://developer.valvesoftware.com/wiki/Sky_(Source_1_shader)")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(HDRBaseTexture,		SHADER_PARAM_TYPE_TEXTURE, "", "Uncompressed Color Data. ( aka $BaseTexture ), despite the name this doesn't have to be an HDR [RGBA16161616F | RGBA16161616] Format")
	SHADER_PARAM(HDRCompressedTexture,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBa] RGBS (Read: RGBM) Compressed HDR Texture.\nTakes priority over $HDRBaseTexture unless mat_use_compressed_hdr_textures is 0.")
	SHADER_PARAM(HDRCompressedTexture0, SHADER_PARAM_TYPE_TEXTURE, "", "1/3 specially compressed Textures (Method B)")
	SHADER_PARAM(HDRCompressedTexture1, SHADER_PARAM_TYPE_TEXTURE, "", "2/3 specially compressed Textures (Method B)")
	SHADER_PARAM(HDRCompressedTexture2, SHADER_PARAM_TYPE_TEXTURE, "", "3/3 specially compressed Textures (Method B)")

	// Why would we block the editing of this?
	// SHADER_PARAM_OVERRIDE(Color, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE)
END_SHADER_PARAMS

// Set Up Vars here
SHADER_INIT_PARAMS()
{
	// Never render a Flashlight on the Sky
	SetBool(ReceiveProjectedTextures, false);

	// Don't want Fog
	SetFlag(MATERIAL_VAR_NOFOG);

	// Render it at the back at all times
	SetFlag(MATERIAL_VAR_IGNOREZ);
}

SHADER_FALLBACK
{
#ifndef REPLACE_SKY
	if (lux_oldshaders.GetBool())
		return "Sky_HDR_DX9";
#endif

	if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
		return "LUX_SKY";

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	// Replicating the Stock Shader
	// Find out if Sampler 0 is sRGB or HDR
	// However do it without 5 if-statements
	int nSamplerZeroFlags = 0;
	bool bUseCompressedHDRTexture = mat_use_compressed_hdr_textures.GetBool();
	if (!bUseCompressedHDRTexture && !IsDefined(HDRCompressedTexture))
	{
		bool bDefinedHDRCompressedTexture0 = IsDefined(HDRCompressedTexture0);
		if (!bDefinedHDRCompressedTexture0 && IsDefined(HDRBaseTexture) && IsTextureLoaded(HDRBaseTexture))
		{
			if (!IsHDRTexture(GetTexture(HDRBaseTexture)))
				nSamplerZeroFlags = TEXTUREFLAGS_SRGB;
		}
	}

	// Next find out, what Texture is gonna be Sampler0
	int nSampler0 = HDRCompressedTexture;
	if (!bUseCompressedHDRTexture && !IsDefined(HDRCompressedTexture))
	{
		if (IsDefined(HDRCompressedTexture0))
		{
			nSampler0 = HDRCompressedTexture0;
		}
		else
		{
			nSampler0 = HDRBaseTexture;
		}
	}

	// Load the Texture(s)
	if (IsDefined(HDRCompressedTexture) && bUseCompressedHDRTexture)
	{
		LoadTexture(HDRCompressedTexture, HDRCompressedTexture == nSampler0 ? nSamplerZeroFlags : 0);
	}
	else
	{
		if (IsDefined(HDRCompressedTexture0))
		{
			LoadTexture(HDRCompressedTexture0, HDRCompressedTexture0 == nSampler0 ? nSamplerZeroFlags : 0);
			if (IsDefined(HDRCompressedTexture1))
			{
				LoadTexture(HDRCompressedTexture1, HDRCompressedTexture1 == nSampler0 ? nSamplerZeroFlags : 0);
			}
			if (IsDefined(HDRCompressedTexture2))
			{
				LoadTexture(HDRCompressedTexture2, HDRCompressedTexture2 == nSampler0 ? nSamplerZeroFlags : 0);
			}
		}
		else if(IsDefined(HDRBaseTexture))
		{
			LoadTexture(HDRBaseTexture, HDRBaseTexture == nSampler0 ? nSamplerZeroFlags : 0);
		}
	}
}

SHADER_DRAW
{
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
	
		// Always write Alpha, used for Depth Values
		pShaderShadow->EnableAlphaWrites(true);
		
		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);
		
		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		
		// Pretty simple, one TexCoord and vPos for ProjPos
		unsigned int nFlags = VERTEX_POSITION;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);
		
		//==========================================================================//
		// Set Static Shaders and Samplers
		//==========================================================================//
		
		// 'RGBs'
		if (IsTextureLoaded(HDRCompressedTexture) && mat_use_compressed_hdr_textures.GetBool())
		{
			// Never sRGB, no LDR
			EnableSampler(SHADER_SAMPLER0, false);

			// We use the Brush VS since there is nothing different compared to what we have to do
//			bool bBicubic = lux_sky_BicubicFilter.GetBool();
			DECLARE_STATIC_VERTEX_SHADER(lux_sky_rgbs_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(USEMODELMATRIX, lux_sky_UseModelMatrix.GetBool());
//			SET_STATIC_VERTEX_SHADER_COMBO(BICUBIC, bBicubic);
			SET_STATIC_VERTEX_SHADER(lux_sky_rgbs_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_sky_rgbs_ps30);
//			SET_STATIC_PIXEL_SHADER_COMBO(BICUBIC, bBicubic);
			SET_STATIC_PIXEL_SHADER(lux_sky_rgbs_ps30);
		}
		else // 'Compressed HDR'
		{
			if (IsTextureLoaded(HDRCompressedTexture0))
			{
				// Never sRGB, no LDR
				EnableSampler(SHADER_SAMPLER0, false);
				EnableSampler(SHADER_SAMPLER1, false);
				EnableSampler(SHADER_SAMPLER2, false);

				// We use the Brush VS since there is nothing different compared to what we have to do
				DECLARE_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
				SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);		
				SET_STATIC_VERTEX_SHADER_COMBO(NOMODELMATRIX, !lux_sky_UseModelMatrix.GetBool());	
				SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);		
				SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, 0);	
				SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);	
				SET_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);

				DECLARE_STATIC_PIXEL_SHADER(lux_sky_compressed_ps30);
				SET_STATIC_PIXEL_SHADER(lux_sky_compressed_ps30);
			}
			else // Regular Sky Shader
			{
				EnableSampler(SHADER_SAMPLER0, !IsHDRTexture(GetTexture(HDRBaseTexture)));

				// We use the Brush VS since there is nothing different compared to what we have to do
				DECLARE_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
				SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);		
				SET_STATIC_VERTEX_SHADER_COMBO(NOMODELMATRIX, !lux_sky_UseModelMatrix.GetBool());	
				SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);		
				SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, 0);	
				SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);	
				SET_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);

				DECLARE_STATIC_PIXEL_SHADER(lux_sky_simple_ps30);
				SET_STATIC_PIXEL_SHADER(lux_sky_simple_ps30);
			}
		}
	}
	
	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures & Setup Constant Registers
		//==========================================================================//

		// VS c223, c224
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nFilter = 0;
		if(IsTextureLoaded(HDRCompressedTexture) && mat_use_compressed_hdr_textures.GetBool())
		{
			// s0 - $HDRCompressedTexture
			ITexture* pTexture = GetTexture(HDRCompressedTexture);
			BindTexture(SHADER_SAMPLER0, HDRCompressedTexture, Frame);

			float w = (float)pTexture->GetActualWidth();
			float h = (float)pTexture->GetActualHeight();

			nFilter = lux_sky_BicubicFilter.GetBool();
			if (!lux_sky_UseFilter.GetBool())
				nFilter = 2;

			// Fudge-Bilinear
			if(nFilter == 0)
			{
				float Fudge = 0.01f / max(w, h); // 'per ATI' wonder if thats still needed

				float4 f4Data;
				f4Data.x = 0.5f / (w - Fudge);
				f4Data.y = 0.5f / (h - Fudge);
				f4Data.z = w;
				f4Data.w = h;
				pShaderAPI->SetVertexShaderConstant(225, f4Data);
			}
			// Bicubic Filter
			else if (nFilter == 1)
			{
				float4 f4Bicubic;
				f4Bicubic.x = w;
				f4Bicubic.y = h;
				f4Bicubic.z = 1.0f / w;
				f4Bicubic.w = 1.0f / h;
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_002, f4Bicubic);
				pShaderAPI->SetVertexShaderConstant(225, f4Bicubic);
			}
		}
		else
		{
			// Compressed Textures
			if (IsTextureLoaded(HDRCompressedTexture0))
			{
				BindTexture(SHADER_SAMPLER0, HDRCompressedTexture0, Frame);
				BindTexture(SHADER_SAMPLER1, HDRCompressedTexture1, Frame);
				BindTexture(SHADER_SAMPLER2, HDRCompressedTexture2, Frame);
			}
			else
			{
				BindTexture(SHADER_SAMPLER0, HDRBaseTexture, Frame);
			}
		}

		// c1 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);

		if(IsTextureLoaded(HDRCompressedTexture) && mat_use_compressed_hdr_textures.GetBool())
			f4Tint.rgb *= 8.0f;
		else if(IsHDREnabled() && IsHDRTexture(GetTexture(HDRBaseTexture)))
			f4Tint.rgb *= 16.0f;

		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_001, f4Tint);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
		// I wonder if this is actually true at any time whatsoever
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = pShaderAPI->ShouldWriteDepthToDestAlpha();
		
		// Always!
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		// RGBS
		if (IsTextureLoaded(HDRCompressedTexture) && mat_use_compressed_hdr_textures.GetBool())
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_sky_rgbs_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(FILTER, nFilter);
			SET_DYNAMIC_VERTEX_SHADER(lux_sky_rgbs_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_sky_rgbs_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(FILTER, nFilter);
			SET_DYNAMIC_PIXEL_SHADER(lux_sky_rgbs_ps30);
		}
		else
		{
			// COMPRESSED
			if (IsTextureLoaded(HDRCompressedTexture0))
			{
				DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
				SET_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);

				DECLARE_DYNAMIC_PIXEL_SHADER(lux_sky_compressed_ps30);
				SET_DYNAMIC_PIXEL_SHADER(lux_sky_compressed_ps30);
			}
			// REGULAR
			else
			{
				DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
				SET_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);

				DECLARE_DYNAMIC_PIXEL_SHADER(lux_sky_simple_ps30);
				SET_DYNAMIC_PIXEL_SHADER(lux_sky_simple_ps30);
			}
		}
	}
	
	Draw();
}
END_SHADER