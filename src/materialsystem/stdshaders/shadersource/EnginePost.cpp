//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	27.11.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_screenspace_vs30.inc"
#include "lux_engine_post_ps30.inc"

// 
// NOTE:
//
// This Shader does not include the AA Code from ASW or the TF2SDK
// It was apparently made for XBOX, not PC!
//
// Also I renamed all of the ConVars and added New ConVars for this Shader ( see cpp_convars )

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Engine_Post, LUX_Engine_Post)
DEFINE_FALLBACK_SHADER(SDK_Engine_Post_dx9, LUX_Engine_Post)
#endif

#ifdef REPLACE_ENGINE_POST
DEFINE_FALLBACK_SHADER(Engine_Post, LUX_Engine_Post)
DEFINE_FALLBACK_SHADER(Engine_Post_dx9, LUX_Engine_Post)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Engine_Post, "Post-Process Shader that handles Effects like Color Correction, Bloom, Desaturation, etc." )
SHADER_INFO_GEOMETRY	("Screenspace.")
SHADER_INFO_USAGE		("Only used Internally.")
SHADER_INFO_LIMITATIONS	("None known.")
SHADER_INFO_PERFORMANCE	("~Ok.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	// Nuked Params:
	/*
	// AA is disabled on Stock Shaders for PC
	SHADER_PARAM(AAEnable,		SHADER_PARAM_TYPE_BOOL, "", "Enables Software Anti-Aliasing")
	SHADER_PARAM(AAInternal1,	SHADER_PARAM_TYPE_VEC4, "",	"(Internal Parameter) Internal Anti-Aliasing Values set via Material Proxy.")
	SHADER_PARAM(AAInternal2,	SHADER_PARAM_TYPE_VEC4, "",	"(Internal Parameter) Internal Anti-Aliasing Values set via Material Proxy.")
	SHADER_PARAM(AAInternal3,	SHADER_PARAM_TYPE_VEC4, "",	"(Internal Parameter) Internal Anti-Aliasing Values set via Material Proxy.")

	// Not actually used on the Stock Reference
	SHADER_PARAM(BlurredVignetteEnable,	SHADER_PARAM_TYPE_BOOL,	 "", "Enables blurred Vignette" )

	// Xbox Feature
	SHADER_PARAM(TV_GAmma, SHADER_PARAM_TYPE_INTEGER, "", "0 default, 1 used for laying off 360 movies" )
	*/

	// Main Rendertarget this Shader is sourcing from
	SHADER_PARAM(FBTexture, SHADER_PARAM_TYPE_TEXTURE, "", "Full Framebuffer Texture without Post-Processing applied. ( Source Texture for this Shader )")

	// L4D Vomit Overlay
	SHADER_PARAM(VomitEnable,			SHADER_PARAM_TYPE_BOOL,		"", "Enables the Vomit Overlay Effect. ( Also used for some Paint Effect apparently )")
	SHADER_PARAM(ScreenEffectTexture,	SHADER_PARAM_TYPE_TEXTURE,	"", "[RG] TexCoord Refraction Offsets\n[B] Mask")
	SHADER_PARAM(VomitColor1,			SHADER_PARAM_TYPE_COLOR,	"", "Lower End Color for the Vomit Effect.")
	SHADER_PARAM(VomitColor2,			SHADER_PARAM_TYPE_COLOR,	"", "Upper End Color for the Vomit Effect")
	SHADER_PARAM(VomitRefractScale,		SHADER_PARAM_TYPE_FLOAT,	"", "Strength of the [rg] Refraction Offset")

	// Contrast
	// MidToneMask is not actually used in the Shader.
	SHADER_PARAM(AllowLocalContrast,			SHADER_PARAM_TYPE_BOOL,		"", "Enables the Local Contrast Effect. Default 1")
	SHADER_PARAM(LocalContrastEnable,			SHADER_PARAM_TYPE_BOOL,		"", "Enables the Local Contrast Effect. Default 0")
	SHADER_PARAM(LocalContrastScale,			SHADER_PARAM_TYPE_FLOAT,	"", "Intensity of the Contrast Effect.")
//	SHADER_PARAM(LocalContratsMidToneMask,		SHADER_PARAM_TYPE_FLOAT,	"", "Local Contrast Midtone Mask.")
	SHADER_PARAM(LocalContrastVignetteStart,	SHADER_PARAM_TYPE_BOOL,		"", "Vignette Starting Point relative to Screen Center ( 0..1 )")
	SHADER_PARAM(LocalContrastVignetteEnd,		SHADER_PARAM_TYPE_FLOAT,	"", "Vignette End Point relative to Screen Center ( 0..1 ).")
	SHADER_PARAM(LocalContrastEdgeScale,		SHADER_PARAM_TYPE_FLOAT,	"", "Local Contrast Midtone Mask.")
	SHADER_PARAM(BlurredVignetteEnable,			SHADER_PARAM_TYPE_BOOL,		"", "Enables blurred Vignette. Requires $LocalContrastEnable.")
	SHADER_PARAM(BlurredVignetteScale,			SHADER_PARAM_TYPE_FLOAT,	"", "Blurred Vignette Strength for LocalContrast.")

	// DepthBlur
	SHADER_PARAM(DepthBlurEnable,			SHADER_PARAM_TYPE_BOOL,	 "", "Inexpensive Depth-of-Field Substitute.")
	SHADER_PARAM(ScreenBlurStrength,		SHADER_PARAM_TYPE_FLOAT, "", "Minimum Blur Factor for DepthBlur")
	SHADER_PARAM(DepthBlurFocalDistance,	SHADER_PARAM_TYPE_FLOAT, "", "Distance in Dest-Alpha Space for the Focal Plane.")
	SHADER_PARAM(DepthBlurStrength,			SHADER_PARAM_TYPE_FLOAT, "", "Strength of the Depth-Blur Effect.")

	// Bloom
	SHADER_PARAM(BloomEnable, SHADER_PARAM_TYPE_BOOL, "", "Enables Bloom")
	SHADER_PARAM(BloomAmount, SHADER_PARAM_TYPE_FLOAT, "", "How much Bloom should be applied.")

	// ViewFade
	// NOTE: This is a Vec4 *intentionally*. The .w is used for the lerp on the Shader
	SHADER_PARAM(FadeColor,	SHADER_PARAM_TYPE_VEC4, "", "ViewFade Color. ( Used with $Fade ). Not the same as Fade to Black! .w controls Fade Amount!")
	SHADER_PARAM(Fade,		SHADER_PARAM_TYPE_INTEGER, "", "ViewFade Type.\n0 = Off\n1 = Lerp\n2 = Modulate")

	// Color Correction
	SHADER_PARAM(ToolMode,				SHADER_PARAM_TYPE_BOOL,		"", "Makes the Shader use $ToolTime instead of ShaderAPI Time. That's all.")
	SHADER_PARAM(ToolColorCorrection,	SHADER_PARAM_TYPE_FLOAT,	"", "Causes Color Correction Weights to be overriden with Parameters.")
	SHADER_PARAM(Weight_Default,		SHADER_PARAM_TYPE_FLOAT,	"", "ToolMode Color Correction Default Weight.")
	SHADER_PARAM(Weight0,				SHADER_PARAM_TYPE_FLOAT,	"", "ToolMode Color Correction Weight0.")
	SHADER_PARAM(Weight1,				SHADER_PARAM_TYPE_FLOAT,	"", "ToolMode Color Correction Weight1.")
	SHADER_PARAM(Weight2,				SHADER_PARAM_TYPE_FLOAT,	"", "ToolMode Color Correction Weight2.")
	SHADER_PARAM(Weight3,				SHADER_PARAM_TYPE_FLOAT,	"", "ToolMode Color Correction Weight3.")
	SHADER_PARAM(Num_LookUps,			SHADER_PARAM_TYPE_INTEGER,	"", "Amount of Lookups to make for Color Correction.-Maximum you can have is 3.")
	SHADER_PARAM(ToolTime,				SHADER_PARAM_TYPE_FLOAT,	"", "Time Variable used for Noise when $ToolMode is 1.")

	// Desaturation
	SHADER_PARAM(DesaturateEnable,	SHADER_PARAM_TYPE_BOOL, "", "Enables Desaturation of the FrameBuffer.")
	SHADER_PARAM(Desaturation,		SHADER_PARAM_TYPE_FLOAT, "", "Desaturation Factor ( 0..1 )")

	// Vignette
	SHADER_PARAM(AllowVignette,		SHADER_PARAM_TYPE_BOOL, "", "Enables the Vignette Texture ($Internal_VignetteTexture). Default 1")
	SHADER_PARAM(VignetteEnable,	SHADER_PARAM_TYPE_BOOL, "", "Enables the Vignette Texture ($Internal_VignetteTexture). Default 0")
	SHADER_PARAM(Internal_VignetteTexture, SHADER_PARAM_TYPE_TEXTURE, "", "Defaults to dev/vignette\n**NOT** an internal Parameter. ( The Name is wrong! )\n")

	// Noise
	SHADER_PARAM(AllowNoise,	SHADER_PARAM_TYPE_BOOL,		"", "Enables the Filmnoise Effect. Default 1")
	SHADER_PARAM(NoiseEnable,	SHADER_PARAM_TYPE_BOOL,		"", "Enables the Filmnoise Effect. Default 0")
	SHADER_PARAM(NoiseScale,	SHADER_PARAM_TYPE_FLOAT,	"", "Noise Scale")
	SHADER_PARAM(NoiseTexture,	SHADER_PARAM_TYPE_TEXTURE,	"", "[R] Nothing?\n[G] Noise.")

	// FadeToBlack
	SHADER_PARAM(FadeToBlackScale, SHADER_PARAM_TYPE_FLOAT, "", "Fade Strength. ( 0..1 )")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Setting this even when $FBTexture?
	SetFlag2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );

	// L4D Vomit Overlay
	// These are not the *default* Values from the ASW Shader
	// They are the actual Values used in the engine_post.vmt from L4D
	// NOTE: The actual Effect in L4D is apparently rendered using a screenspace Particle
	// So these might not be the *actual* Values used. Just the first I found.
	DefaultFloat3(VomitColor1, 0.675f, 0.735f, 0.0f);
	DefaultFloat3(VomitColor2, 0.599f, 0.646f, 0.0f);
	DefaultFloat(VomitRefractScale, 0.22f);

	// NOTE: This Texture is not included with the SDK!
	// This is the Texture used by L4D
	if(!IsDefined(ScreenEffectTexture))
		SetString(ScreenEffectTexture, "particle/vomitscreensplash");

	// ASW Defaults:
//	DefaultFloat(VomitRefractScale, 0.15f);
//	DefaultFloat3(VomitColor1, 1.0f, 1.0f, 0.0f);
//	DefaultFloat3(VomitColor1, 0.0f, 1.0f, 0.0f);

	// Contrast
	// MidToneMask is not actually used in the Shader.
	DefaultBool(AllowLocalContrast, true);
	DefaultFloat(LocalContrastScale, 1.0f);
	DefaultFloat(LocalContrastVignetteStart, 0.7f);
	DefaultFloat(LocalContrastVignetteEnd, 1.0f);

	// DepthBlur
	// No Default Values other than 0.0f
	
	// Bloom
	DefaultFloat(BloomAmount, 1.0f);

	// ViewFade
	// No Defeault Values other than 0.0f

	// Desaturation
	// No Default Values other than 0.0f

	// Vignette
	DefaultBool(AllowVignette, true);

	// DefaultString is not a Thing yet..
	if(!IsDefined(Internal_VignetteTexture))
		SetString(Internal_VignetteTexture, "dev/vignette");

	// Noise
	DefaultBool(AllowNoise, true);
	DefaultFloat(NoiseScale, 1.0f);

	// LUX Addition: Default Noise Texture
	// ShiroDkxtro2: I checked TF2 and the TF2SDK ( SDK2013MP ) and they both had this Texture.
	// L4D uses "dev/noise_post" for it's engine_post.vmt, which we don't have as SDK Dwellers.
	if(!IsDefined(NoiseTexture))
		SetString(NoiseTexture, "engine/noise-blur-256x256");
}

SHADER_FALLBACK
{
#ifndef REPLACE_ENGINEPOST
	if (lux_oldshaders.GetBool())
		return "Engine_Post_dx9";
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
	LoadTexture(BaseTexture);
	LoadTexture(ScreenEffectTexture);
	LoadTexture(NoiseTexture);
	LoadTexture(Internal_VignetteTexture);
}

SHADER_DRAW
{
	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		// Original Comment from ASW:
		// "This shader uses opaque blending, but needs to match the behaviour of bloom_add/screen_spacegeneral,
		// which uses additive blending (and is used when bloom is enabled but col-correction and AA are not).
		//   BUT!
		// Hardware sRGB blending is incorrect (on pre-DX10 cards, sRGB values are added directly).
		//   SO...
		// When doing the bloom addition in the pixel shader, we need to emulate that incorrect
		// behaviour - by turning sRGB read OFF for the FB texture and by turning sRGB write OFF
		// (which is fine, since the AA process works better on an sRGB framebuffer than a linear
		// one; gamma colours more closely match luminance perception. The color-correction process
		// has always taken gamma-space values as input anyway)."

		// ShiroDkxtro2: 
		// This Comment is a tad hard to comprehend the way it is written. Allow me to clarify.
		// 
		// Previously additive blending was used to create the Bloom Effect done in this Shader.
		// However they did Frame += Bloom, without converting either from Gamma to Linear
		// 
		// To replicate this Behaviour, we call sRGBWrite(false)
		// This causes Output Values to not be converted from Linear to sRGB
		// 
		// Both the Framebuffer and Bloom Sampler have EnablesRGBRead(false) 
		// So when the Shader reads them, the resulting Values are sRGB
		// In the Shader, it can now do Frame += BloomAmount * Bloom. Reproducing the original Behaviour.
		// I exposed this as a ConVar if you want to know what it looks like without reproducing whacky Behaviour
		bool bLinearBloom = lux_enginepost_linearbloom.GetBool();

		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// Don't write any Depth, Screenshaders should probably never write Depth
		pShaderShadow->EnableDepthWrites(false);

		// No Alpha output from this Shader
		pShaderShadow->EnableAlphaWrites(false);

		// Stock-Consistency: Setting this although we never enabled blending, so this shouldn't be required.
		pShaderShadow->EnableBlending(false);

		// Read the Comment above for more Information about bLinearBloom
		pShaderShadow->EnableSRGBWrite(bLinearBloom ? true : false);

		// Never need Fog. Turn it off
		DisableFog();

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
		
		// Current SamplerMap
		// s0 - Sampler_VomitParams
		// s1 - Sampler_Frame
		// s2 - Sampler_Bloom
		// s3 - Sampler_ColorCorrection1
		// s4 - Sampler_ColorCorrection2
		// s5 - Sampler_ColorCorrection3
		// s6 - Sampler_ColorCorrection4
		// s7 - Sampler_Vignette
		// s8 - Sampler_Noise

		// NOTE: These must all, always be enabled!
		// Whether a Feature is used, is handled via Dynamic Combo.

		// s0 - $ScreenEffectTexture. Never sRGB as it doesn't contain Colors
		EnableSampler(SHADER_SAMPLER0, false);

		// s1 - $FBTexture
		EnableSampler(SHADER_SAMPLER1, bLinearBloom);

		// s2 - $BaseTexture.. ( This is the Bloom Texture )
		EnableSampler(SHADER_SAMPLER2, bLinearBloom);

		// s3, s4, s5, s6 - "Up to 4 (sRGB) color-correction lookup textures are bound to samplers 2-5:"
		EnableSampler(SHADER_SAMPLER3, bLinearBloom);
		EnableSampler(SHADER_SAMPLER4, bLinearBloom);
		EnableSampler(SHADER_SAMPLER5, bLinearBloom);
		EnableSampler(SHADER_SAMPLER6, bLinearBloom);

		// s7 - $Internal_VignetteTexture.. Should probably be treated as sRGB but it's mostly linear so I will keep it that way.
		EnableSampler(SHADER_SAMPLER7, false);

		// s8 - $NoiseTexture. Never sRGB as it doesn't contain Colors
		EnableSampler(SHADER_SAMPLER8, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(X360APPCHOOSER, 0);
		SET_STATIC_VERTEX_SHADER(lux_screenspace_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_engine_post_ps30);
		SET_STATIC_PIXEL_SHADER(lux_engine_post_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// Tool Stuff
		bool bToolMode = GetBool(ToolMode);
		bool bToolColorCorrection = GetBool(ToolColorCorrection);

		// Dynamically evaluated Feature-Set.
		bool bVomit = GetBool(VomitEnable);
		bool bContrast = GetBool(AllowLocalContrast) && GetBool(LocalContrastEnable);
		bool bBlurredVignette = bContrast && GetBool(BlurredVignetteEnable);
		bool bDepthBlur = GetBool(DepthBlurEnable);
		bool bDesaturate = GetBool(DesaturateEnable);
		bool bVignette = GetBool(AllowVignette) && GetBool(VignetteEnable);
		bool bNoise = GetBool(AllowNoise) && GetBool(NoiseEnable);
		bool bBloom = GetBool(BloomEnable);

		// Stock-Consistency:? "Disable noise at low resolutions"
		ITexture* pFrameBuffer = GetTexture(FBTexture);
		if(pFrameBuffer && pFrameBuffer->GetActualHeight() < 720)
			bNoise = false;

		//==========================================================================//
		// ConVar Feature Overrides
		//==========================================================================//

		if(lux_enginepost_force_vomit.GetBool())
			bVomit = true;

		if(lux_enginepost_force_contrast.GetBool())
		{
			bContrast = true;
			if(lux_enginepost_force_contrast.GetInt() == 2)
				bBlurredVignette = true;
		}

		if(lux_enginepost_force_depthblur.GetInt() > 0)
			bDepthBlur = true;

		if(lux_enginepost_force_desaturate.GetBool())
			bDesaturate = true;

		if(lux_enginepost_force_vignette.GetBool())
			bVignette = true;

		if(lux_enginepost_force_noise.GetBool())
			bNoise = true;

		bool bVomitTexture = bVomit && IsTextureLoaded(ScreenEffectTexture);
		bool bVignetteTexture = bVignette && IsTextureLoaded(Internal_VignetteTexture);
		bool bNoiseTexture = bNoise && IsTextureLoaded(NoiseTexture);

		//==========================================================================//
		// Get Color Correction Data
		//==========================================================================//

		// Not disabled under DESATURATE on LUX
		ShaderColorCorrectionInfo_t ccInfo = { false, 0, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f } };
		if(bToolColorCorrection)
		{
			ccInfo.m_bIsEnabled = true;
			
			ccInfo.m_nLookupCount = GetInt(Num_LookUps);
			ccInfo.m_flDefaultWeight = GetFloat(Weight_Default);
			ccInfo.m_pLookupWeights[0] = GetFloat(Weight0);
			ccInfo.m_pLookupWeights[1] = GetFloat(Weight1);
			ccInfo.m_pLookupWeights[2] = GetFloat(Weight2);
			ccInfo.m_pLookupWeights[3] = GetFloat(Weight3);
		}
		else
		{
			pShaderAPI->GetCurrentColorCorrection( &ccInfo );
		}

		// Make sure this doesn't go over 3 and is 0 when CC is disabled.
		if(!ccInfo.m_bIsEnabled)
			ccInfo.m_nLookupCount = 0;
		else
			ccInfo.m_nLookupCount = MIN( ccInfo.m_nLookupCount, 3 );

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// Always bind SOMETHING to enabled Samplers!
		BindTexture(bVomitTexture, SHADER_SAMPLER0, ScreenEffectTexture, -1, TEXTURE_BLACK);

		// -1 so we don't try to load any frames from the rt..
		BindTexture(SHADER_SAMPLER1, FBTexture, -1);
		BindTexture(SHADER_SAMPLER2, BaseTexture, -1);

		// Stock bind Code
		for (int n = 0; n < ccInfo.m_nLookupCount; n++)
		{
			pShaderAPI->BindStandardTexture((Sampler_t)(SHADER_SAMPLER3 + n), (StandardTextureId_t)(TEXTURE_COLOR_CORRECTION_VOLUME_0 + n));
		}

		BindTexture(bVignetteTexture, SHADER_SAMPLER7, Internal_VignetteTexture, -1, TEXTURE_BLACK);
		BindTexture(bNoiseTexture, SHADER_SAMPLER8, NoiseTexture, -1, TEXTURE_BLACK);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//
		
		// c0, c1 - Vomit Params
		// c2, c3 - Viewport Transforms for Vomit
		if(bVomit)
		{
			float4 f4VomitParams1 = 0.0f;
			f4VomitParams1.rgb = GetFloat3(VomitColor1);
			f4VomitParams1.w = GetFloat(VomitRefractScale);

			// Override with ConVar
			if(lux_enginepost_vomit_refractfactor.GetFloat() != -1.0f)
				f4VomitParams1.w = lux_enginepost_vomit_refractfactor.GetFloat();

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4VomitParams1);

			float4 f4VomitParams2 = 0.0f;
			f4VomitParams2.rgb = GetFloat3(VomitColor2);
			// .w Free
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_001, f4VomitParams2);

			// "Get viewport and render target dimensions and set shader constant to do a 2D mad"
			// The Code here is from Mapbase, made to work with the SDK.
			// ASW uses GetCurrentViewPort() and GetCurrentRenderTargetResolution()
			// Which are not Functions the SDK has.
			ShaderViewport_t vp;
			pShaderAPI->GetViewports(&vp, 1);
			int nViewportX = vp.m_nTopLeftX;
			int nViewportY = vp.m_nTopLeftY;
			int nViewportWidth = vp.m_nWidth;
			int nViewportHeight = vp.m_nHeight;

			int nRTWidth, nRTHeight;
			ITexture* pCurRenderTarget = pShaderAPI->GetRenderTargetEx(0);
			if (pCurRenderTarget != nullptr)
			{
				nRTWidth = pCurRenderTarget->GetActualWidth();
				nRTHeight = pCurRenderTarget->GetActualHeight();
			}
			else
			{
				pShaderAPI->GetBackBufferDimensions(nRTWidth, nRTHeight);
			}

			// Lifted from ASW
			float4 f4ViewPortTransforms = 0.0f;
			f4ViewPortTransforms.x = (float)nRTWidth / (float)nViewportWidth;
			f4ViewPortTransforms.y = (float)nRTHeight / (float)nViewportHeight;
			f4ViewPortTransforms.z = -(float)nViewportX / (float)nViewportWidth;
			f4ViewPortTransforms.w = -(float)nViewportY / (float)nViewportHeight;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_002, f4ViewPortTransforms);

			float4 f4ViewPortTransformsInv = 0.0f;
			f4ViewPortTransformsInv.x = (float)nViewportWidth / (float)nRTWidth;
			f4ViewPortTransformsInv.y = (float)nViewportHeight / (float)nRTHeight;
			f4ViewPortTransformsInv.z = (float)nViewportX / (float)nRTWidth;
			f4ViewPortTransformsInv.w = (float)nViewportY / (float)nRTHeight;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_003, f4ViewPortTransformsInv);
		}

		// c4, c5 - Contrast Parameters
		if(bContrast)
		{
			float4 f4ContrastParams1 = 0.0f;
			f4ContrastParams1.x = GetFloat(LocalContrastScale);
			f4ContrastParams1.y = GetFloat(LocalContrastVignetteStart);
			f4ContrastParams1.z = GetFloat(LocalContrastVignetteEnd);
			f4ContrastParams1.w = GetFloat(LocalContrastEdgeScale);

			// ConVar Overrides
			if (lux_enginepost_contrast_factor.GetFloat() != -1.0f)
				f4ContrastParams1.x = lux_enginepost_contrast_factor.GetFloat();

			if (lux_enginepost_contrast_vignettestart.GetFloat() != -1.0f)
				f4ContrastParams1.y = lux_enginepost_contrast_vignettestart.GetFloat();

			if (lux_enginepost_contrast_vignetteend.GetFloat() != -1.0f)
				f4ContrastParams1.z = lux_enginepost_contrast_vignetteend.GetFloat();

			if (lux_enginepost_contrast_edgescale.GetFloat() != -1.0f)
				f4ContrastParams1.w = lux_enginepost_contrast_edgescale.GetFloat();

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_004, f4ContrastParams1);

			float4 f4ContrastParams2 = 0.0f;
			f4ContrastParams2.x = GetFloat(BlurredVignetteScale);

			if(lux_enginepost_contrast_vignetteblur.GetFloat() != -1.0f)
				f4ContrastParams2.x = lux_enginepost_contrast_vignetteblur.GetFloat();

			// .yzw Free
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_005, f4ContrastParams2);
		}

		// c6 - DepthBlur and Bloom
		// Always need Bloom Amount!!
		if(true)
		{
			float4 f4BloomParams = 0.0f;
			if(bDepthBlur)
			{
				f4BloomParams.x = GetFloat(ScreenBlurStrength);	
				f4BloomParams.y = GetFloat(DepthBlurFocalDistance);
				f4BloomParams.z = GetFloat(DepthBlurStrength);		
			}

			// Always need Bloom ( 0..1 )
			f4BloomParams.w = bBloom ? GetFloat(BloomAmount) : 0.0f;

			// ConVar Overrides
			if(lux_enginepost_depthblur_blur.GetFloat() != -1.0f)
				f4BloomParams.x = lux_enginepost_depthblur_blur.GetFloat();

			if(lux_enginepost_depthblur_focalplane.GetFloat() != -1.0f)
				f4BloomParams.y = lux_enginepost_depthblur_focalplane.GetFloat();

			if(lux_enginepost_depthblur_factor.GetFloat() != -1.0f)
				f4BloomParams.z = lux_enginepost_depthblur_factor.GetFloat();

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_006, f4BloomParams);		
		}

		// c7, c8 - View Fade
		// Always enabled! But not always used..
		if(true)
		{
			float4 f4ViewFadeParams1 = GetFloat4(FadeColor);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_007, f4ViewFadeParams1);	

			float4 f4ViewFadeParams2 = 0.0f;
			f4ViewFadeParams2.x = GetInt(Fade) == 2 ? 1.0f : 0.0f; // Flips the Lerp to Result*FadeColor
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_008, f4ViewFadeParams2);	
		}
		
		// c9, c10 - Color Correction Weights
		// Always enabled!
		if(true)
		{
			// Upload color-correction weights:
			// NOTE: Setting this float to a float4.
			// Noticed during testing that everything went green because it will interpret the actual weights as GBA
			// Interpreting this as a &float is prone to break when changes are made to the Struct.
			// This only worked on Stock because it interpreted the entire Constant as a single float.
			float4 f4DefaultWeight = ccInfo.m_flDefaultWeight;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_009, f4DefaultWeight);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_010, ccInfo.m_pLookupWeights );
		}

		// c11 - Desaturation
		if(bDesaturate)
		{
			// ShiroDkxtro2: Flipped this on the Shader
			float4 f4Desaturate = 0.0f;
			f4Desaturate.x = GetFloat(Desaturation);

			if(lux_enginepost_desaturate_factor.GetFloat() != -1.0f)
				f4Desaturate.x = lux_enginepost_desaturate_factor.GetFloat();

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_011, f4Desaturate );
		}

		// c12 - Film Noise
		if(bNoise)
		{
			float4 f4NoiseParams = 0.0f;
			if(bToolMode)
				f4NoiseParams.x = GetFloat(ToolTime);
			else
				f4NoiseParams.x = pShaderAPI->CurrentTime();

			// Stock-Consistency: Replicate this Math
			f4NoiseParams.x -= (float)((int)(f4NoiseParams.x / 1000.0f)) * 1000.0f;

			// Now do NoiseScale
			f4NoiseParams.y = GetFloat(NoiseScale);

			// ConVar Override
			if(lux_enginepost_noise_factor.GetFloat() != -1.0f)
				f4NoiseParams.y = lux_enginepost_noise_factor.GetFloat();

			// Stock-Consistency: Disable Noise at negative Scales
			if(f4NoiseParams.y <= 0.0f)
				bNoise = false;

			// Noise Texture Scale, so Noise doesn't become so tiny that it's no longer visible.
			f4NoiseParams.z = lux_enginepost_noise_texturescale.GetFloat();

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_012, f4NoiseParams);
		}

		// c13 - Fade to Black
		if(true)
		{
			float4 f4FadeParams = 0.0f;
			f4FadeParams.x = GetFloat(FadeToBlackScale);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_013, f4FadeParams);
		}

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);

		// Blurred Vignette needs Contrast so it shares the Combo
		DECLARE_DYNAMIC_PIXEL_SHADER(lux_engine_post_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(L4D_VOMIT, bVomit);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(CONTRAST, bContrast + bContrast && bBlurredVignette);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DEPTH_BLUR, bDepthBlur);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(COLORCORRECT_LOOKUPS, ccInfo.m_nLookupCount);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DESATURATE, bDesaturate);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(VIGNETTE, bVignette);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(NOISE, bNoise);
		SET_DYNAMIC_PIXEL_SHADER(lux_engine_post_ps30);
	}

	Draw();
}
END_SHADER