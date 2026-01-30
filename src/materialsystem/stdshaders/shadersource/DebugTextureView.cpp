//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	11.11.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_debug_textureview_vs30.inc"
#include "lux_debug_textureview_ps30.inc"

// Throw these to Stock
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_DebugTextureView, LUX_DebugTextureView);
#endif

#ifdef REPLACE_DEBUGSHADERS
DEFINE_FALLBACK_SHADER(DebugTextureView, LUX_DebugTextureView);
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_DebugTextureView, "DebugShader used with mat_texture_list.")
SHADER_INFO_GEOMETRY	("Applied to Geometry by the Engine.")
SHADER_INFO_USAGE		("Don't apply this yourself.")
SHADER_INFO_LIMITATIONS	("Does not account for Alpha Modulation like $Alpha or $Alpha2.\n"
						 "Alpha Output may not be perceptually-linear for Anyone that cares.")
SHADER_INFO_PERFORMANCE	("Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(ShowAlpha, SHADER_PARAM_TYPE_BOOL, "", "Causes Alpha to be displayed instead of RGB Channels.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Nothing
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
	LoadTexture(BaseTexture);
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);

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

		// Nope
		DisableFog();
	
		// Never writing linear Values from here
		pShaderShadow->EnableSRGBWrite(false);

		// No Depth, we are in a UI Element
		pShaderShadow->EnableDepthWrites(false);

		// Stock-Consistency: Alphatesting, always
		pShaderShadow->EnableAlphaTest(false);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Applied to a Model so compressed Vertex Format flag
		unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. Might be sRGB but we don't care
		EnableSampler(SHADER_SAMPLER0, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_debug_textureview_vs30);
		SET_STATIC_VERTEX_SHADER(lux_debug_textureview_vs30);		

		DECLARE_STATIC_PIXEL_SHADER(lux_debug_textureview_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(SHOWALPHA, GetBool(ShowAlpha));
		SET_STATIC_PIXEL_SHADER(lux_debug_textureview_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		
		// Always bind Fallbacks!
		BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, Frame, TEXTURE_BLACK);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		const ITexture* pTexture = GetTexture(BaseTexture);
		int bHDR = false;
		bool bCubemap = false;
		if(pTexture)
		{
			ImageFormat Format = pTexture->GetImageFormat();
			bHDR += Format == IMAGE_FORMAT_RGBA16161616;
			bHDR += Format == IMAGE_FORMAT_RGBA16161616F;
			bHDR += Format == IMAGE_FORMAT_RGB323232F;
			bHDR += Format == IMAGE_FORMAT_RGBA32323232F;

			// Check if it's a Texture
			bCubemap = pTexture->IsCubeMap();
		}

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_debug_textureview_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_debug_textureview_vs30);

		int nCubemapMode = bCubemap + (bCubemap && lux_texturelist_octahedrons.GetBool());
		int nHDRTextureMode = bHDR + (bHDR && lux_texturelist_fixgamma.GetBool());
		DECLARE_DYNAMIC_PIXEL_SHADER(lux_debug_textureview_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(CUBEMAPMODE, nCubemapMode);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(HDRTEXTUREMODE, nHDRTextureMode);
		SET_DYNAMIC_PIXEL_SHADER(lux_debug_textureview_ps30);
	}

	Draw();
}
END_SHADER