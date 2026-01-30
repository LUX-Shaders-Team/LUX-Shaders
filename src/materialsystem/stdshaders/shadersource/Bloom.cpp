//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Post-Process overbrightening blur
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_screenspace_vs30.inc"
#include "lux_bloom_ps30.inc"


// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Bloom, LUX_Bloom)
#endif

#ifdef REPLACE_BLOOM
DEFINE_FALLBACK_SHADER(Bloom, LUX_Bloom)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Bloom, "A shader used for post-processing effects that simulates the glow or light bleeding in a scene." )
SHADER_INFO_GEOMETRY	("Materials.")
SHADER_INFO_USAGE		("Run through Client Code ( Post-Process Controller ) or make a Material and apply it via env_screenoverlay.")
SHADER_INFO_LIMITATIONS	("Presumably None.")
SHADER_INFO_PERFORMANCE	("Cheap to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(FBTexture, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "The previous Rendering Results")
	SHADER_PARAM(BlurTexture, SHADER_PARAM_TYPE_TEXTURE, "_rt_SmallHDR0",	 "[RGBA] The Bloom that's going to be added. (Created separately)")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	//.. nothing here!
}

SHADER_FALLBACK
{
#ifndef REPLACE_BLOOM
	if (lux_oldshaders.GetBool())
		return "Bloom";
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
	LoadTexture(FBTexture);
	LoadTexture(BlurTexture);
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
		// Don't write any Depth, Screenshaders should probably never write Depth
		pShaderShadow->EnableDepthWrites(false);

		// No Alpha output
		pShaderShadow->EnableAlphaWrites(false);

		// Never need Fog. Turn it off
		DisableFog();

		// NOTE: Should we clarify EnableSRGBWrite(false)?

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We want only Position, one Texcoord and nothing else
		pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//
		EnableSampler(SHADER_SAMPLER0, false);
		EnableSampler(SHADER_SAMPLER1, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(X360APPCHOOSER, 0);
		SET_STATIC_VERTEX_SHADER(lux_screenspace_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_bloom_ps30);
		SET_STATIC_PIXEL_SHADER(lux_bloom_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		// -1 so we don't try to load any frames from the rt..
		BindTexture(SHADER_SAMPLER0, FBTexture, -1);
		BindTexture(SHADER_SAMPLER1, BlurTexture, -1);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_bloom_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_bloom_ps30);
	}
	Draw();
}
END_SHADER