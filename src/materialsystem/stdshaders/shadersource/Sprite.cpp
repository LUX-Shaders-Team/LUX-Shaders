//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Shader for Sprite objects, this doesn't include some particles.
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// const.h has an enum for sprite Renderingmodes
#include "const.h"

// Includes for Shaderfiles...
#include "lux_sprite_ps30.inc"
#include "lux_sprite_vs30.inc"

// What does SPR Stand for? Sprite?
// VP? Vertex Projection?
#define SPR_VP_PARALLEL_UPRIGHT		0
#define SPR_FACING_UPRIGHT			1
#define SPR_VP_PARALLEL				2
#define SPR_ORIENTED				3
#define SPR_VP_PARALLEL_ORIENTED	4

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Sprite,		LUX_Sprite)
DEFINE_FALLBACK_SHADER(SDK_Sprite_DX9,	LUX_Sprite)
DEFINE_FALLBACK_SHADER(SDK_Sprite_DX8,	LUX_Sprite)
DEFINE_FALLBACK_SHADER(SDK_Sprite_DX6,	LUX_Sprite)
#endif

#ifdef REPLACE_SPRITE
DEFINE_FALLBACK_SHADER(Sprite,		LUX_Sprite)
DEFINE_FALLBACK_SHADER(Sprite_DX9,	LUX_Sprite)
DEFINE_FALLBACK_SHADER(Sprite_DX8,	LUX_Sprite)
DEFINE_FALLBACK_SHADER(Sprite_DX6,	LUX_Sprite)
#endif

// Here's a Stock Shader Funfact
// Sprite_dx6.cpp
// Sprite_dx9.cpp
// Sprite.cpp		<- This is Sprite_dx8..

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Sprite, "A Shader used for Sprites." )
SHADER_INFO_GEOMETRY	("Intended for Sprites but *can* be used for simple enough flat Geometry.")
SHADER_INFO_USAGE		("Set a $SpriteRenderMode ( usually through Hammer ) and a $SpriteOrientation. See VDC for more Information.")
SHADER_INFO_LIMITATIONS	("Does not receive Lighting.")
SHADER_INFO_PERFORMANCE	("Cheap, very cheap!")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Sprite Shader Page: https://developer.valvesoftware.com/wiki/Sprite")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(SpriteOrigin,			SHADER_PARAM_TYPE_VEC3,		"", "The Origin of the Sprite")
	SHADER_PARAM(SpriteOrientation,		SHADER_PARAM_TYPE_VEC3,		"", "How the Sprite should orient itself in the World.")
	SHADER_PARAM(SpriteRenderMode,		SHADER_PARAM_TYPE_STRING,	"", "Modes: 0 to 9. See Shadercode for more Information, this is a little bit complicated.")
	SHADER_PARAM(IgnoreVertexColors,	SHADER_PARAM_TYPE_BOOL,		"", "0 ( the Default ) forces Vertex Colors, this also includes Vertex Alpha!")
	SHADER_PARAM(NosRGB,				SHADER_PARAM_TYPE_BOOL,		"", "Disable SRGB Conversions for this Sprite. This is enabled by Default.")
	SHADER_PARAM(HDRColorScale,			SHADER_PARAM_TYPE_FLOAT,	"", "Scales Results if HDR is Enabled, Artist Input for Brightness Scale.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Do this, or your Sprites become invisible
	DefaultFloat(Alpha, 1.0f);
	DefaultFloat(HDRColorScale, 1.0f);
	DefaultInt(NosRGB, 1);

	SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	SetFlag(MATERIAL_VAR_VERTEXCOLOR);
	SetFlag(MATERIAL_VAR_VERTEXALPHA);

	// translate from a string orientation to an enumeration
	// ShiroDkxtro2 : This is kinda... cursed.... Why didn't they just turn it to ints to begin with? Luckily this only executes once
	if (IsDefined(SpriteOrientation))
	{
		const char* ccOrientationString = GetString(SpriteOrientation);
		if (stricmp(ccOrientationString, "parallel_upright") == 0)
		{
			SetInt(SpriteOrientation, SPR_VP_PARALLEL_UPRIGHT);
		}
		else if (stricmp(ccOrientationString, "facing_upright") == 0)
		{
			SetInt(SpriteOrientation, SPR_FACING_UPRIGHT);
		}
		else if (stricmp(ccOrientationString, "vp_parallel") == 0)
		{
			SetInt(SpriteOrientation, SPR_VP_PARALLEL);
		}
		else if (stricmp(ccOrientationString, "oriented") == 0)
		{
			SetInt(SpriteOrientation, SPR_ORIENTED);
		}
		else if (stricmp(ccOrientationString, "vp_parallel_oriented") == 0)
		{
			SetInt(SpriteOrientation, SPR_VP_PARALLEL_ORIENTED);
		}
		else
		{
			// ShiroDkxtro2 : LMAO What kind of warning is this? No information at all, don't even know what Material
//			Warning("error with $spriteOrientation\n");

			// This will tell you what Material, what valid Strings are and what it's currently set to.
			Warning("SpriteMaterial %s with bogus $SpriteOrientation '%s'\n", pMaterialName, ccOrientationString);
			Warning("Valid Strings are : parallel_upright, facing_upright, vp_parallel, oriented, vp_parallel_oriented\n");
			Warning("Forcing 'parallel_upright'... Fix This!!!\n");
			SetInt(SpriteOrientation, SPR_VP_PARALLEL_UPRIGHT);
		}
	}
	else
	{
		// default case
		SetInt(SpriteOrientation, SPR_VP_PARALLEL_UPRIGHT);
	}
}

SHADER_FALLBACK
{
	// There used to be a joke here about wanting a Sprite ( beverage )
#ifndef REPLACE_SPRITE
	if (lux_oldshaders.GetBool())
		return "Sprite_DX9";
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
	bool bsRGB = !GetBool(NosRGB);
	LoadTexture(BaseTexture, bsRGB);
}

struct SpriteBlendFactors_t
{
	ShaderBlendFactor_t SrcFactor = SHADER_BLEND_ZERO;
	ShaderBlendFactor_t DstFactor = SHADER_BLEND_ZERO;
};

// 16 Bytes
struct SpriteSettings_t
{
	bool bFogToFogColor = false;
	bool bDisableDepthWrites = false;
	bool bEnableBlending = false;
	bool bUseVertexColors = false;
	bool bDisableDepthTests = false;
	bool bErrorMessage = false;
	bool bSecondDraw = false;

	SpriteBlendFactors_t BlendFactors;
};

void Sprite_DetermineBools(int nRenderMode, SpriteSettings_t &Settings)
{
	const bool bUsingEditor = UsingEditor();

	// the enum is declared in const.h
	switch (nRenderMode)
	{
		// Normal Rendermode only gets FogToFog.
	case kRenderNormal:
		Settings.bFogToFogColor			= true;
		Settings.bDisableDepthWrites	= false;
		Settings.bEnableBlending		= false;
		Settings.bUseVertexColors		= false;
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= false;
		break;

	// These two get FogToFog, No DepthWrites, Blending and VertexColors
	case kRenderTransColor:
	case kRenderTransTexture:
		Settings.bFogToFogColor			= true;
		Settings.bDisableDepthWrites	= true;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true;
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= false;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE_MINUS_SRC_ALPHA;
		break;

	// These two get FogToFog, No DepthWrites, Blending and VertexColors
	case kRenderGlow:
	case kRenderWorldGlow:
		// don't disable depth writes in hammer, it uses this rendermode for spotlight sprites.. -azzy
		Settings.bFogToFogColor			= false;
		Settings.bDisableDepthWrites	= !bUsingEditor;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true;
		Settings.bDisableDepthTests		= !bUsingEditor;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= false;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE;
		break;

	// As the name implies, this AlphaBlends, thus doesn't need DepthWrites
	case kRenderTransAlpha:
		Settings.bFogToFogColor			= true;
		Settings.bDisableDepthWrites	= true;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true; // Is this true on sdk2013sp?
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= false;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE_MINUS_SRC_ALPHA;
		break;

	// As the name implies, this AlphaBlends, thus doesn't need DepthWrites
	// Second Pass
	case kRenderTransAlphaAdd:
		Settings.bFogToFogColor			= true;
		Settings.bDisableDepthWrites	= true;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true;
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= true;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE_MINUS_SRC_ALPHA;
		// second tBlendFactors will be determined later.
		// ( SHADEAR_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_ONE )
		break;

	// Transparency, no FogToFog
	case kRenderTransAdd:
		Settings.bFogToFogColor			= false;
		Settings.bDisableDepthWrites	= true;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true;
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= false;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE;
		break;

	// Transparency, no FogToFog and Second Pass
	case kRenderTransAddFrameBlend:
		Settings.bFogToFogColor			= false;
		Settings.bDisableDepthWrites	= true;
		Settings.bEnableBlending		= true;
		Settings.bUseVertexColors		= true;
		Settings.bDisableDepthTests		= false;
		Settings.bErrorMessage			= false;
		Settings.bSecondDraw			= true;
		Settings.BlendFactors.SrcFactor = SHADER_BLEND_SRC_ALPHA;
		Settings.BlendFactors.DstFactor = SHADER_BLEND_ONE;
		break;

	default:
		Settings.bErrorMessage = true;	// Spit a Warning
		break;
	}
}

SHADER_DRAW
{
	//	bool bsRGB = !GetBool(info.m_nNoSRGB);
	int nRenderMode = GetInt(SpriteRenderMode);

	// Set the bools to whatever they should be
	SpriteSettings_t SpriteSettings;
	Sprite_DetermineBools(nRenderMode, SpriteSettings);
	
	// this allows all rendering modes to ignore vertexcolors
	bool bUseVertexColors = SpriteSettings.bUseVertexColors && !GetBool(IgnoreVertexColors);
	bool bsRGB = !GetBool(NosRGB);
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bConstantColor = (nRenderMode & kRenderTransAdd) || (nRenderMode & kRenderTransAddFrameBlend);
	
	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		if (SpriteSettings.bErrorMessage)
			Warning("Sprite '%s' is using unknown rendermode '%i', this will probably mess up the sprites rendering!", CurrentMaterialName(), nRenderMode);
	
		if (!bHasBaseTexture)
			Warning("Sprite %s has no $BaseTexture! \n", CurrentMaterialName());

		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// We don't want culling, ever
		// ( This will overwrite $NoCull )
		pShaderShadow->EnableCulling(false);

		// If we do Fog it's FogToFog
		if (SpriteSettings.bFogToFogColor)
			FogToFogColor();
		else
			FogToBlack();

		// Usually used for Transparency Blending Modes
		if (SpriteSettings.bDisableDepthWrites)
			pShaderShadow->EnableDepthWrites(false);

		// Sprites going through Walls
		if (SpriteSettings.bDisableDepthTests)
			pShaderShadow->EnableDepthTest(false);
	
		// LinearWrite depends on bsRGB
		pShaderShadow->EnableSRGBWrite(bsRGB);

		// AlphaBlending
		// NOTE: We don't use EnableAlphaBlending() here
		// It automatically set's some things like EnableDepthWrites(false)
		if (SpriteSettings.bEnableBlending)
		{
			pShaderShadow->EnableBlending(true);
			pShaderShadow->BlendFunc(SpriteSettings.BlendFactors.SrcFactor, SpriteSettings.BlendFactors.DstFactor);
		}
	
		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION;
		if (bUseVertexColors)
			nFlags |= VERTEX_COLOR;

		int nTexCoords = 1;
		int nUserData = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserData);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// Only s0
		EnableSampler(SAMPLER_BASETEXTURE, bsRGB);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_sprite_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXRGBA, bUseVertexColors);
		SET_STATIC_VERTEX_SHADER_COMBO(TRANSFORM, false); // FIXME: Set to true and test
		SET_STATIC_VERTEX_SHADER(lux_sprite_vs30);
	
		DECLARE_STATIC_PIXEL_SHADER(lux_sprite_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(VERTEXRGBA, bUseVertexColors);
		SET_STATIC_PIXEL_SHADER_COMBO(CONSTANTCOLOR, bConstantColor);
		SET_STATIC_PIXEL_SHADER_COMBO(SRGB, bsRGB);
		SET_STATIC_PIXEL_SHADER(lux_sprite_ps30);
	}
	
	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Textures and Constants
		//==========================================================================//

		// c0, c1
		// ShiroDkxtro2: Note that $Color2 is **NOT** RenderColor
		// RenderColor appears to be handled by VertexColors in Hammer
		float4 f4Tint = 1.0f;
		f4Tint.rgb *= GetFloat3(Color1);
		f4Tint.rgb *= GetFloat3(Color2);
		f4Tint.rgb *= GetFloat3(sRGBTint);
		f4Tint.a = GetFloat(Alpha);

		float4 f4HDRColorScale = 1.0f;

		// Gamma Conversions
		if (bsRGB && (nRenderMode & kRenderTransAdd || nRenderMode & kRenderTransAddFrameBlend))
		{
			// Stock-Consistency : GammaToLinear Conversion
			f4Tint.rgb = GammaToLinearTint(f4Tint.rgb);
		}
		
		// Artist Brightness Factor for HDR Specifically
		if (IsHDREnabled())
		{
			// Only HDR!
			f4HDRColorScale.rgb = GetFloat3(HDRColorScale);
			if (bsRGB)
			{
				// Stock-Consistency : GammaToLinear Conversion
				f4HDRColorScale.rgb = GammaToLinearTint(f4HDRColorScale.rgb);
			}
		}
	
		// This Rendermode requires some rather specific Shenanigans
		if (nRenderMode & kRenderTransAddFrameBlend)
		{
			// Yes, we use this as a float, that's intentional, somehow
			float f1FrameVar = GetFloat(Frame);
			float f1FrameBlendAlpha = 1.0f - (f1FrameVar - (int)f1FrameVar);

			// s0
			// Magic
			int nFrame = (int)f1FrameBlendAlpha;

			// ShiroDkxtro2: IMPORTANT, this is NOT BaseVSShader's BindTexture
			// This has to give *the actual* Frame Number not a Parameter Index
			if(bHasBaseTexture)
				BindTexture(SAMPLER_BASETEXTURE, GetTexture(BaseTexture), nFrame);

			// We override the Tint here.
			// Not good! No tinting :/
			float f1Alpha = GetFloat(Alpha);
			if (bsRGB)
				f4Tint.rgb = GammaToLinear(f1Alpha * f1FrameBlendAlpha);
			else
				f4Tint.rgb = f1Alpha * f1FrameBlendAlpha;

			// Overwrite $Alpha here
			f4Tint.w = 1.0f;
		}
		else
		{
			// s0
			BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame);
		}

		// c0
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4Tint);

		// c31
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4HDRColorScale);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
	
		// c19
		if (bsRGB)
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		// Never handle DepthToDestAlpha and WaterFog
		BBools[LUX_PS_BOOL_HEIGHTFOG] = false;
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = false;

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// Tranform. // FIXME: Enable and test
		// SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_sprite_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_sprite_vs30);
		
		DECLARE_DYNAMIC_PIXEL_SHADER(lux_sprite_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_sprite_ps30);
	}

	// First Draw
	Draw();
	
	// Not done here yet. Second Pass is a thing for this Shader
	if (SpriteSettings.bSecondDraw)
	{
		//==========================================================================//
		// Static Snapshot of the Shader Settings
		//==========================================================================//
		if(IsSnapshotting())
		{
			// Multipass, so reset Shadow State.
			// ShiroDkxtro2: Actually, don't
			// This might be intentional.
			// pShaderShadow->SetDefaultState();

			//==========================================================================//
			// General Rendering Setup
			//==========================================================================//

			// Only this RenderMode gets a different BlendFunc for the second Pass
			if (nRenderMode & kRenderTransAlphaAdd)
			{
				SetInitialShadowState();
				pShaderShadow->EnableDepthWrites(false);
				pShaderShadow->EnableBlending(true);
				pShaderShadow->BlendFunc(SHADER_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_ONE);
			}

			// No Fog, ever!
			FogToBlack();

			pShaderShadow->EnableSRGBWrite(bsRGB);

			//==========================================================================//
			// Vertex Shader - Vertex Format
			//==========================================================================//
			unsigned int nFlags = VERTEX_POSITION;
			if (bUseVertexColors)
				nFlags |= VERTEX_COLOR;

			int nTexCoords = 1;
			int nUserData = 0;
			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserData);

			//==========================================================================//
			// Sampler Setup
			//==========================================================================//

			// Only s0
			EnableSampler(SAMPLER_BASETEXTURE, bsRGB);

			//==========================================================================//
			// Set Static Shaders
			//==========================================================================//
			DECLARE_STATIC_VERTEX_SHADER(lux_sprite_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXRGBA, bUseVertexColors);
			SET_STATIC_VERTEX_SHADER_COMBO(TRANSFORM, false); // FIXME: Set to true and test.
			SET_STATIC_VERTEX_SHADER(lux_sprite_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_sprite_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(VERTEXRGBA, bUseVertexColors);
			SET_STATIC_PIXEL_SHADER_COMBO(CONSTANTCOLOR, bConstantColor);
			SET_STATIC_PIXEL_SHADER_COMBO(SRGB, bsRGB);
			SET_STATIC_PIXEL_SHADER(lux_sprite_ps30);
		}

		//==========================================================================//
		// Entirely Dynamic Commands
		//==========================================================================//
		DYNAMIC_STATE
		{
			// Multipass, so reset Dynamic State.
			// Multipass, so reset Shadow State.
			// ShiroDkxtro2: Actually, don't
			// This might be intentional.
//			pShaderAPI->SetDefaultState();

			//==========================================================================//
			// Textures and Constants
			//==========================================================================//

			// We don't add $Color here,
			// Should we?
			float4 f4Tint = 1.0f;
//			f4Tint.rgb = GetFloat3(Color1);
			f4Tint.rgb = 1.0f;
			f4Tint.a = GetFloat(Alpha);

			// Artist Brightness Factor for HDR Specifically
			float4 f4HDRColorScale = 1.0f;
			if (IsHDREnabled())
			{
				// Only HDR!
				f4HDRColorScale.rgb = GetFloat3(HDRColorScale);
				if (bsRGB)
				{
					// Stock-Consistency : GammaToLinear Conversion
					f4HDRColorScale.rgb = GammaToLinearTint(f4HDRColorScale.rgb);
				}
			}

			if (nRenderMode & kRenderTransAddFrameBlend)
			{
				// This is the inverse of the first pass. The first pass does 1.0f - x
				float f1FrameVar = GetFloat(Frame);
				float f1FrameBlendAlpha = (f1FrameVar - (int)f1FrameVar);

				// s0
				// Magic
				int nFrame = ((int)f1FrameVar + 1) % GetTexture(BaseTexture)->GetNumAnimationFrames();

				// ShiroDkxtro2: IMPORTANT, this is NOT BaseVSShader's BindTexture
				// This has to give *the actual* Frame Number not a Parameter Index
				if(bHasBaseTexture)
					BindTexture(SAMPLER_BASETEXTURE, GetTexture(BaseTexture), nFrame);

				// We override the Tint here.
				// Not good! No tinting :/
				float f1Alpha = GetFloat(Alpha);
				if (bsRGB)
					f4Tint.rgb = GammaToLinear(f1Alpha * f1FrameBlendAlpha);
				else
					f4Tint.rgb = f1Alpha * f1FrameBlendAlpha;

				// Overwrite $Alpha here too
				f4Tint.a = 1.0f;
			}
			else
			{
				// s0
				BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame);
			}

			// c0
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4Tint);

			// c31
			// Always modulating, fill the Register
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4HDRColorScale);

			// c11 - Camera Position
			SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

			// c12 - Fog Params
			pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

			// c19
			if (bsRGB)
				SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

			// Prepare boolean array, yes we need to use BOOL
			BOOL BBools[REGISTER_BOOL_MAX] = { false };

			// b13, b14, b15
			// Never handle DepthToDestAlpha and WaterFog
			BBools[LUX_PS_BOOL_HEIGHTFOG] = false;
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = false;

			pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

			// Tranform. // FIXME: Enable and test
			// SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

			//==================================================================================================
			// Set Dynamic Shaders
			//==================================================================================================
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_sprite_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_sprite_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_sprite_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_sprite_ps30);
		}

		// Second Draw
		Draw();
	}

	// No more Passes!
}
END_SHADER
