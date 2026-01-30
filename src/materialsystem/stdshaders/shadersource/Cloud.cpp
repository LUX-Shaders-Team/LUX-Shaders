//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_cloud_vs30.inc"
#include "lux_cloud_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Cloud,		LUX_Cloud)
DEFINE_FALLBACK_SHADER(SDK_Cloud_DX9,	LUX_Cloud)
DEFINE_FALLBACK_SHADER(SDK_Cloud_DX8,	LUX_Cloud)
#endif

#ifdef REPLACE_CLOUD
DEFINE_FALLBACK_SHADER(Cloud,		LUX_Cloud)
DEFINE_FALLBACK_SHADER(Cloud_DX9,	LUX_Cloud)
DEFINE_FALLBACK_SHADER(Cloud_DX8,	LUX_Cloud)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Cloud, "A shader used for rendering animated cloud layers.")
SHADER_INFO_GEOMETRY	("Brushes and Models.")
SHADER_INFO_USAGE		("Need both $BaseTexture and $CloudAlphaTexture.")
SHADER_INFO_LIMITATIONS	("The Support for Models is only for static Props.")
SHADER_INFO_PERFORMANCE	("Very Cheap to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Cloud Shader Page: https://developer.valvesoftware.com/wiki/Cloud")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM_OVERRIDE(BaseTexture,			 SHADER_PARAM_TYPE_TEXTURE,	"",	 "[RGBA] Albedo Texture with Opacity.", 0)
	SHADER_PARAM(CloudAlphaTexture,				 SHADER_PARAM_TYPE_TEXTURE,	"",	 "[RGB] Modulation for Visible Color.\n[A] Modulation for Opacity.")
	SHADER_PARAM(CloudAlphaTextureTransform,	 SHADER_PARAM_TYPE_MATRIX,	"",	 "Transforms the $CloudAlphaTexture. Must include all Values!")
	SHADER_PARAM(CloudAlphaTexture_UseTexCoord0, SHADER_PARAM_TYPE_BOOL,	"",	 "Use TexCoord0 instead of TexCoord1.")
	SHADER_PARAM(CloudScale,					 SHADER_PARAM_TYPE_VEC2,	"",	 "Two-Dimensional Scalar for the $BaseTextureTransform. Works similar to $DetailScale on other Shaders.")
	SHADER_PARAM(MaskScale,						 SHADER_PARAM_TYPE_VEC2,	"",	 "Two-Dimensional Scalar for the $CloudAlphaTexture.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	DefaultFloat2(CloudScale, 1.0f, 1.0f);
	DefaultFloat2(MaskScale, 1.0f, 1.0f);

	// We NEVER want a Flashlight on this Thing
	SetBool(ReceiveProjectedTextures, false);
}

SHADER_FALLBACK
{
#ifndef REPLACE_CLOUD
	if (lux_oldshaders.GetBool())
		return "Cloud_dx9";
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
	LoadTexture(BaseTexture);
	LoadTexture(CloudAlphaTexture);
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

		// Current Default Fog for Occlusion
		DefaultFog();

		// Using EnableAlphaBlending disables Depthwrites!
		if (HasFlag(MATERIAL_VAR_ADDITIVE))
			EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // Additive
		else
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA); // Alpha Blending

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We want only Position, TWO Texcoords and nothing else
		unsigned int nFlags = VERTEX_POSITION;
		int nTexCoords = 2;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//
		EnableSampler(SHADER_SAMPLER0, false);
		EnableSampler(SHADER_SAMPLER1, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_cloud_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(ALPHATEX_USE_TEX0, GetBool(CloudAlphaTexture_UseTexCoord0));
		SET_STATIC_VERTEX_SHADER(lux_cloud_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_cloud_ps30);
		SET_STATIC_PIXEL_SHADER(lux_cloud_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture
		BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);

		// s1 - $CloudAlphaTexture
		BindTexture(SHADER_SAMPLER1, CloudAlphaTexture); // No FrameVar?

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Scroll $BaseTexture and $CloudAlphaTexture
		SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform, CloudScale);
		SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, CloudAlphaTextureTransform, MaskScale);

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c32 - Tints
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_COLOR_FACTORS, f4Tint);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		// Never Opaque!
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(false);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_cloud_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_cloud_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_cloud_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_cloud_ps30);
	}

	Draw();
}
END_SHADER