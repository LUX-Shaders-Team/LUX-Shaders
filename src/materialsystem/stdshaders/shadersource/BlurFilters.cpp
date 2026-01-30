//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File : Rendering Blur
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_blurfilters_vs30.inc"
#include "lux_blurfilters_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_BlurFilterX, LUX_BlurFilterX)
DEFINE_FALLBACK_SHADER(SDK_BlurFilterY, LUX_BlurFilterY)
#endif

#ifdef REPLACE_BLURFILTERS
DEFINE_FALLBACK_SHADER(BlurFilterX, LUX_BlurFilterX)
DEFINE_FALLBACK_SHADER(BlurFilterY, LUX_BlurFilterY)
#endif

// Need to expose Draw Function to two separate Shaders.
// So we also need this struct.
// Mainly happens because Params are not carried over between two Shaders.
struct BlurVars_t
{
	int m_nBlurX = -1;
	int m_nBloomAmount = -1;
};

// Don't need Compression or ContextData for this one
// VertexCompressionType_t vertexCompression, CBasePerMaterialContextData** pContextDataPtr
void DrawBloom(CBaseVSShader* pShader, BlurVars_t& info, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI)
{
	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(pShader->IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//
		// Don't write any Depth, Screenshaders should probably never write Depth
		pShaderShadow->EnableDepthWrites(false);

		// We are blurring the Alpha too!
		pShaderShadow->EnableAlphaWrites(true);

		// Enable Sampler0, not sRGB
		// "Render targets are pegged as sRGB on POSIX, so just force these reads and writes"
//		bool bForceSRGBReadAndWrite = IsOSX() && g_pHardwareConfig->CanDoSRGBReadFromRTs();
		bool bForceSRGBReadAndWrite = false;

		// Weird name, what it actually means : We output linear Values
		pShaderShadow->EnableSRGBWrite(bForceSRGBReadAndWrite);

		// Never need Fog here.
		pShader->DisableFog();

		// Could be needed for some Effects like adding the second Layer of the Blur with Weights.
		// Useful if we don't want to swap Render Targets during blurring to store Results.
		// That however will almost certainly instantly break when using 16f Render Targets.
		// D3D9 does not appear to support AlphaBlending on HDR Rendertargets.. ?
		if (pShader->HasFlag(MATERIAL_VAR_ADDITIVE))
			pShader->EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We want only Position, one Texcoord and nothing else
		unsigned int nFlags = VERTEX_POSITION;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture
		pShader->EnableSampler(SAMPLER_BASETEXTURE, bForceSRGBReadAndWrite);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_blurfilters_vs30);
		SET_STATIC_VERTEX_SHADER(lux_blurfilters_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_blurfilters_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(SRGB_ADAPTER, bForceSRGBReadAndWrite);
		SET_STATIC_PIXEL_SHADER(lux_blurfilters_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(pShader->IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// We usually want -1 for the Frames since we often use Render Targets as the $BaseTexture
		// *however* this Shader might be applied to something that isn't a Render Target
		pShader->BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);

		//==================================================================================================
		// Setup Constant Registers
		//==================================================================================================

		// "The temp buffer is 1/4 back buffer size"
		// Stock-Consistency
		ITexture* pFramebuffer = pShader->GetTexture(BaseTexture);
		int nWidth = pFramebuffer->GetActualWidth();
		int nHeight = pFramebuffer->GetActualHeight();

		bool bBlurX = pShader->GetBool(info.m_nBlurX);
#if 0
		// NOTE:
		// nWidth here for both the X and Y Pass, Stock Shader does that.
		// The Aspect Ratio, unless you are using 4:3, is not very square?
		// Probably a non-issue though. And more of an oversight.
		float f1TexelSize = 1.0f / (float)nWidth;
#else
		// Use the correct Dimensions..
		float f1TexelSize = 1.0f / (bBlurX ? (float)nWidth : (float)nHeight);
#endif

		// Blur-Tap Offsets
		float4 f4Offsets[6];
		if (bBlurX) // BlurfilterX
		{
			f4Offsets[0].x = 1.3366f * f1TexelSize;
			f4Offsets[0].y = 0.0f;
			f4Offsets[1].x = 3.4295f * f1TexelSize;
			f4Offsets[1].y = 0.0f;
			f4Offsets[2].x = 5.4264f * f1TexelSize;
			f4Offsets[2].y = 0.0f;
			f4Offsets[3].x = 7.4359f * f1TexelSize;
			f4Offsets[3].y = 0.0f;
			f4Offsets[4].x = 9.4436f * f1TexelSize;
			f4Offsets[4].y = 0.0f;
			f4Offsets[5].x = 11.4401f * f1TexelSize;
			f4Offsets[5].y = 0.0f;
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_0, f4Offsets[0], 6);
		}
		else // BlurfilterY
		{
			f4Offsets[0].y = 1.3366f * f1TexelSize;
			f4Offsets[0].x = 0.0f;
			f4Offsets[1].y = 3.4295f * f1TexelSize;
			f4Offsets[1].x = 0.0f;
			f4Offsets[2].y = 5.4264f * f1TexelSize;
			f4Offsets[2].x = 0.0f;
			f4Offsets[3].y = 7.4359f * f1TexelSize;
			f4Offsets[3].x = 0.0f;
			f4Offsets[4].y = 9.4436f * f1TexelSize;
			f4Offsets[4].x = 0.0f;
			f4Offsets[5].y = 11.4401f * f1TexelSize;
			f4Offsets[5].x = 0.0f;
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_0, f4Offsets[0], 6);
		}

		// Original BlurFilterX forced a 1.0f Value
		// Not sure if it was Intentional or not, VMT's set a 1.0f Value regardless
		float4 f4BloomAmount = 0.0f;
		f4BloomAmount.xyz = pShader->GetFloat(info.m_nBloomAmount);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4BloomAmount);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_blurfilters_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_blurfilters_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_blurfilters_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_blurfilters_ps30);
	}

	pShader->Draw();
}

//==========================================================================//
// BlurX Fallback
//==========================================================================//
BEGIN_VS_SHADER(LUX_BlurFilterX, "A shader used for post-processing effects that smooths the screen in the X-axis." )
SHADER_INFO_GEOMETRY	("Screenspace Rectangles")
SHADER_INFO_USAGE		("Run through Client Code ( Post-Process Controller ) or make a Material and apply it via env_screenoverlay.")
SHADER_INFO_LIMITATIONS	("No mentionable Limitations.")
SHADER_INFO_PERFORMANCE	("Very fast, however any Screenspace Effect will stall rendering, so consider that.")
SHADER_INFO_FALLBACK	("No Fallbacks, aside from lux_oldshaders..")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(BlurX, SHADER_PARAM_TYPE_BOOL, "", "If set to true, Blur the X-Axis of the Screen.")
	SHADER_PARAM(BloomAmount, SHADER_PARAM_TYPE_FLOAT, "", "Multiplier for how much Blurring should apply")
END_SHADER_PARAMS

// Set Up Vars here

SHADER_INIT_PARAMS()
{
	// Set this bool so we can tell whether we are doing a x pass or a y pass
	SetBool(BlurX, true);
	DefaultFloat(BloomAmount, 1.0f);
}

SHADER_FALLBACK
{
#ifndef REPLACE_BLURFILTERS
	if (lux_oldshaders.GetBool())
		return "BlurFilterX";
#endif

	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture);
}

SHADER_DRAW
{
	BlurVars_t Vars;
	Vars.m_nBlurX = BlurX;
	Vars.m_nBloomAmount = BloomAmount;
	DrawBloom(this, Vars, pShaderShadow, pShaderAPI);
}
END_SHADER

//==========================================================================//
// BlurY Fallback
//==========================================================================//
BEGIN_VS_SHADER(LUX_BlurFilterY, "A shader used for post-processing effects that smooths the screen in the Y-axis" )
SHADER_INFO_GEOMETRY	("Screenspace Rectangles")
SHADER_INFO_USAGE		("Run through Client Code ( Post-Process Controller ) or make a Material and apply it via env_screenoverlay.")
SHADER_INFO_LIMITATIONS	("No mentionable Limitations.")
SHADER_INFO_PERFORMANCE	("Very fast, however any Screenspace Effect will stall rendering, so consider that.")
SHADER_INFO_FALLBACK	("No Fallbacks, aside from lux_oldshaders..")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(BlurX, SHADER_PARAM_TYPE_BOOL, "", "If set to false, Blur the Y-Axis of the Screen.")
	SHADER_PARAM(BloomAmount, SHADER_PARAM_TYPE_FLOAT, "", "Multiplier for how much Blurring should apply")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Set this bool so we can tell whether we are doing a x pass or a y pass
	SetBool(BlurX, false);
	DefaultFloat(BloomAmount, 1.0f);
}

SHADER_FALLBACK
{
#ifndef REPLACE_BLURFILTERS
	if (lux_oldshaders.GetBool())
		return "BlurFilterY";
#endif

	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture);
}

SHADER_DRAW
{
	BlurVars_t Vars;
	Vars.m_nBlurX = BlurX;
	Vars.m_nBloomAmount = BloomAmount;
	DrawBloom(this, Vars, pShaderShadow, pShaderAPI);
}
END_SHADER