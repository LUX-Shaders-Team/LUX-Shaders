//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	28.01.2026 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_debug_luxels_vs30.inc"
#include "lux_debug_luxels_ps30.inc"

// Throw these to Stock
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_DebugLuxels, LUX_DebugLuxels);
#endif

#ifdef REPLACE_DEBUGSHADERS
DEFINE_FALLBACK_SHADER(DebugLuxels, LUX_DebugLuxels);
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_DebugLuxels, "DebugShader used with mat_luxels.")
SHADER_INFO_GEOMETRY	("Applied to Geometry by the Engine.")
SHADER_INFO_USAGE		("Don't apply this yourself.")
SHADER_INFO_LIMITATIONS	("Probably can't handle lightmapped Props.")
SHADER_INFO_PERFORMANCE	("Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(NoScale,		SHADER_PARAM_TYPE_BOOL, "", "Disables scaling by the LightmapPage Dimensions.")
END_SHADER_PARAMS

// Set Up Vars here
void DebugLuxelsShaderFlags()
{
	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING); // Idk why this would be needed..
	SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
}

SHADER_INIT_PARAMS()
{
	DebugLuxelsShaderFlags();
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
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
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
		// ShiroDkxtro2: Causing this just in case for Overlays with $Decal ( which do get lightmapped )
		SetInitialShadowState();

		// Nope
		DisableFog();
	
		// Original Shader didn't convert the BaseTexture so they could just output in Gamma Anyways
		pShaderShadow->EnableSRGBWrite(true);

		// Addition here, don't do AlphaWrites.. Like what for?
		pShaderShadow->EnableAlphaWrites(false);

		// Translucency
		SetDefaultBlendingShadowState(BaseTexture);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Don't care about the first TexCoord but we want the second
		unsigned int nFlags = VERTEX_POSITION;
		int nTexCoords = 2;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. Always sRGB
		EnableSampler(SHADER_SAMPLER0, true);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_debug_luxels_vs30);
		SET_STATIC_VERTEX_SHADER(lux_debug_luxels_vs30);		

		DECLARE_STATIC_PIXEL_SHADER(lux_debug_luxels_ps30);
		SET_STATIC_PIXEL_SHADER(lux_debug_luxels_ps30);
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

		int nScaleX = 1;
		int nScaleY = 1;
		if(!GetBool(NoScale))
			pShaderAPI->GetLightmapDimensions(&nScaleX, &nScaleY);

		float4 cTransform = 0.0f;
		cTransform.x = nScaleX;
		pShaderAPI->SetVertexShaderConstant(LUX_VS_TEXTURETRANSFORM_01, cTransform);

		// Second Row of the Transform. Reuse float4
		cTransform.x = 0.0f;
		cTransform.y = nScaleY;
		pShaderAPI->SetVertexShaderConstant(LUX_VS_TEXTURETRANSFORM_01 + 1, cTransform);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_debug_luxels_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_debug_luxels_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_debug_luxels_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_debug_luxels_ps30);
	}

	Draw();
}
END_SHADER