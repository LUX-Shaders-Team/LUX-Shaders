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

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_SKY_DX9, LUX_Sky_Simple)
DEFINE_FALLBACK_SHADER(SDK_SKY_DX6, LUX_Sky_Simple)
#endif

#ifdef REPLACE_SKY
DEFINE_FALLBACK_SHADER(SKY_DX9, LUX_Sky_Simple)
DEFINE_FALLBACK_SHADER(SKY_DX6, LUX_Sky_Simple)
#endif

DEFINE_FALLBACK_SHADER(LUX_Sky, LUX_Sky_Simple)

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Sky_Simple, "Renders LDR and HDR Skybox Textures onto a Surface." )
SHADER_INFO_GEOMETRY	("Should only be applied using Map Properties.\nIf you must apply to World Geometry: Brushes.")
SHADER_INFO_USAGE		("Only needs a $BaseTexture. Supports all LDR Formats, RGBA16161616F and RGBA16161616.")
SHADER_INFO_LIMITATIONS	("Does not support compressed HDR Formats like RGBM or RGBE.\n"
						 "Does not allow rotation of the Skybox.")
SHADER_INFO_PERFORMANCE	("Fast.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Sky Shader Page: https://developer.valvesoftware.com/wiki/Sky_(Source_1_shader)")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(Internal_HDR, SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER), dont use!")
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

	if (CVarDeveloper.GetInt() > 0 && !IsDefined(BaseTexture))
	{
		ShaderDebugMessage("(A Skybox Material) has no BaseTexture, binding black one instead!\n\n");
	}
}

SHADER_FALLBACK
{
#ifndef REPLACE_SKY
	if (lux_oldshaders.GetBool())
		return "Sky";
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
	if (IsDefined(BaseTexture))
	{
		bool bHDR = IsHDRTexture(GetTexture(BaseTexture));
		SetBool(Internal_HDR, bHDR);

		// Use the original LoadTexture Variant instead of simplified one ( Load )
		// Texture is sRGB when not HDR
		LoadTexture(BaseTexture, bHDR ? 0 : TEXTUREFLAGS_SRGB);
	}
}

SHADER_DRAW
{
	// Always needed!
	bool bHDR = GetBool(Internal_HDR);

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
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. sRGB only when LDR
		EnableSampler(SHADER_SAMPLER0, !bHDR);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

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

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		BindTexture(IsTextureLoaded(BaseTexture), SHADER_SAMPLER0, BaseTexture, Frame, TEXTURE_BLACK);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// VS c223, c224
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// c1 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		if (bHDR)
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
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_sky_simple_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_sky_simple_ps30);
	}

	Draw();
}
END_SHADER