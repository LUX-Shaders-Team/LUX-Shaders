//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Attempt at creating a more functional & useful DetailSprite/Particle Shader
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Needed for command line lookup
// Which uh, kinda useless for mods honestly
// You can't force users to launch with a specific launch argument...
// So checking for that here is only useful for debugging purposes
// Maybe for bugfixing on machines that aren't properly supported anyway
#include "tier0/icommandline.h"

// Includes for Shaderfiles...
#include "lux_spritecard_vs30.inc"
#include "lux_spritecard_spline_vs30.inc"
#include "lux_spritecard_ps30.inc"

// Function directly lifted from the Stock Shader
int GetDefaultDepthFeatheringValue(void) //Allow the command-line to go against the default soft-particle value
{
	static int iRetVal = -1;

	if (iRetVal == -1)
	{
#		if( DEFAULT_PARTICLE_FEATHERING_ENABLED == 1 )
		{
			if (CommandLine()->CheckParm("-softparticlesdefaultoff"))
				iRetVal = 0;
			else
				iRetVal = 1;
		}
#		else
		{
			if (CommandLine()->CheckParm("-softparticlesdefaulton"))
				iRetVal = 1;
			else
				iRetVal = 0;
		}
#		endif
	}

	// On low end parts on the Mac, we reduce particles and shut off depth blending here
	// FIXME PRE-RELEASE: Move to ConVars C++
	static ConVarRef mat_reduceparticles("mat_reduceparticles");
	if (mat_reduceparticles.GetBool())
	{
		iRetVal = 0;
	}

	return iRetVal;
}

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Spritecard, LUX_Spritecard)
DEFINE_FALLBACK_SHADER(SDK_Spritecard_DX8, LUX_Spritecard)
#endif

#ifdef REPLACE_SPRITECARD
DEFINE_FALLBACK_SHADER(Spritecard, LUX_Spritecard)
DEFINE_FALLBACK_SHADER(Spritecard_DX8, LUX_Spritecard)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Spritecard, "[Supported: Particles] -> A Shader used for sprite cards that are used with Source's particle system." LUX_DEFAULT_DESCRIPTION)

BEGIN_SHADER_PARAMS
	// These are the default parameters that (presumably) HAVE to be supported
	SHADER_PARAM(DepthBlend,			SHADER_PARAM_TYPE_BOOL,		"",	"Fade at Intersection with Geometry.")
	SHADER_PARAM(DepthBlendScale,		SHADER_PARAM_TYPE_FLOAT,	"", "Amplify or reduce DepthBlend fading. Lower Values cause more abrupt Transitions.")
	SHADER_PARAM(Orientation,			SHADER_PARAM_TYPE_INTEGER,	"",	"0 = Always face Camera.\n1 = Rotate around Z-Axis.\n2 = Parallel to Ground.")
	SHADER_PARAM(AddBaseTexture2,		SHADER_PARAM_TYPE_FLOAT,	"",	"Amount to blend second Texture into Frame by.")
	SHADER_PARAM(OverbrightFactor,		SHADER_PARAM_TYPE_FLOAT,	"",	"Overbright factor for Texture. For HDR effects.")
	SHADER_PARAM(DualSequence,			SHADER_PARAM_TYPE_INTEGER,	"",	"Blend two separate animated Sequences.")
	SHADER_PARAM(Sequence_Blend_Mode,	SHADER_PARAM_TYPE_INTEGER,	"",	"Defines the blend mode between the images un dual sequence particles. Modes: (0 = avg, 1 = alpha from first, rgb from 2nd, 2 = first over second).")
	SHADER_PARAM(MaxLumFrameBlend1,		SHADER_PARAM_TYPE_INTEGER,	"",	"Instead of blending between Animationframes for the 1st Sequence, select Pixels based upon max. Luminance.")
	SHADER_PARAM(MaxLumFrameBlend2,		SHADER_PARAM_TYPE_INTEGER,	"",	"Instead of blending between Animationframes for the 2nd Sequence, select Pixels based upon max. Luminance.")
	SHADER_PARAM(RampTexture,			SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] If specified, the Red Values of the Image is used to Index this Ramp, to produce the Output Color.\n[A] Unused.")
	SHADER_PARAM(ZoomAnimateSeq2,		SHADER_PARAM_TYPE_FLOAT,	"",	"Amount of gradual Zoom between Frames, on the Second sequence. 2.0 will *double* the Size of a Frame over its Lifetime.")
	SHADER_PARAM(ExtractGreenAlpha,		SHADER_PARAM_TYPE_BOOL,		"",	"Use Factors to blend Green/Alpha Channels.")
	SHADER_PARAM(AddOverBlend,			SHADER_PARAM_TYPE_BOOL,		"",	"Enables 'Over Operator' Blending also referred to as 'Standard Transparency', Does Result.rgb + (1.0 - Result.a) * Background")
	SHADER_PARAM(AddSelf,				SHADER_PARAM_TYPE_FLOAT,	"",	"Amount of BaseTexture to additively blend in.")
	SHADER_PARAM(BlendFrames,			SHADER_PARAM_TYPE_BOOL,		"",	"Whether or not to smoothly blend between animated Frames.")
	SHADER_PARAM(MinSize,				SHADER_PARAM_TYPE_FLOAT,	"",	"Minimum fractional Screensize of the Particle.")
	SHADER_PARAM(StartFadeSize,			SHADER_PARAM_TYPE_FLOAT,	"", "Fractional Screensize to start fading the Particle out.")
	SHADER_PARAM(EndFadeSize,			SHADER_PARAM_TYPE_FLOAT,	"", "Fractional Screensize to finish fading the Particle out.")
	SHADER_PARAM(MaxSize,				SHADER_PARAM_TYPE_FLOAT,	"", "Maximum fractional Screensize of the Particle.")
//	SHADER_PARAM(UseInstancing,			SHADER_PARAM_TYPE_BOOL,		"",	"Whether to use GPU Vertex Instancing, (submit 1 vert per particle quad).")
	SHADER_PARAM(SplineType,			SHADER_PARAM_TYPE_INTEGER,	"",	"0 = Not using Splines,  1 = Catmull-Rom.")
	SHADER_PARAM(MaxDistance,			SHADER_PARAM_TYPE_FLOAT,	"", "Maximum Distance the Particle will draw at.")
	SHADER_PARAM(FarFadeInterval,		SHADER_PARAM_TYPE_FLOAT,	"", "Interval over which to fade out far away Particles.")
	SHADER_PARAM(Opaque,				SHADER_PARAM_TYPE_BOOL,		"",	"Whether or not we should disable Transparency for good.")
	SHADER_PARAM(InverseDepthBlend,		SHADER_PARAM_TYPE_BOOL,		"",	"Flip DepthBlend so that Particles appear when they are NEAR Geometry instead of disappearing.")
	SHADER_PARAM(Mod2X,					SHADER_PARAM_TYPE_BOOL,		"", "Modulates the Background. Colors above Grey will brighten the Background, below they will darken them.")
	SHADER_PARAM(AlphaFuncValue,		SHADER_PARAM_TYPE_FLOAT,	"", "Overrides the default AlphaFunc Value for AlphaTesting")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	DefaultFloat(MaxDistance, 100000.0);
	DefaultFloat(FarFadeInterval, 400.0);
	DefaultFloat(MaxSize, 20.0);
	DefaultFloat(EndFadeSize, 20.0);
	DefaultFloat(StartFadeSize, 10.0);
	DefaultFloat(DepthBlendScale, 50.0);
	DefaultFloat(OverbrightFactor, 1.0);
	DefaultFloat(AddBaseTexture2, 0.0);
	DefaultFloat(AddSelf, 0.0);
	DefaultFloat(ZoomAnimateSeq2, 0.0);
	DefaultFloat(AlphaFuncValue, 0.01f);

	// Great so despite command line you can still force depth blending anyways
	DefaultInt(DepthBlend, GetDefaultDepthFeatheringValue());
	DefaultInt(BlendFrames, 1);

	// x360 not supported any longer. Farewell
//	DefaultInt(UseInstancing, IsX360() ? 1 : 0);

	// ????: 'What does it do?'
	SetFlag2(MATERIAL_VAR2_IS_SPRITECARD);
}

SHADER_FALLBACK
{
#ifndef REPLACE_SPRITECARD
	if (lux_oldshaders.GetBool())
		return "SpriteCard";
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
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

	bool bExtractGreenAlpha = IsDefined(ExtractGreenAlpha) && GetInt(ExtractGreenAlpha) != 0;

	LoadTexture(BaseTexture, !bExtractGreenAlpha ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(RampTexture, TEXTUREFLAGS_SRGB);
}

// I reworked aspects of the Draw function
// Moved various bools into the shadow state ( Which was the only place they've ever been used )
// Moved some bools OUT of shadow state ( AddSelf ) because their definitions were used later
// Removed dx8 references and xbox considerations
SHADER_DRAW
{
	bool bHasRampTexture = IsTextureLoaded(RampTexture);

	float f1ZoomAnimSeq2 = GetFloat(ZoomAnimateSeq2);
	float f1AddBaseTexture2 = GetFloat(AddBaseTexture2);
	float f1AddSelf = GetFloat(AddSelf);

	bool bZoomSeq2 = f1ZoomAnimSeq2 > 1.0f;
	bool bAdditive2ndTexture = f1AddBaseTexture2 != 0.0;
	bool bAddSelf = f1AddSelf != 0.0;
	bool bDepthBlend = GetBool(DepthBlend);
	bool bIsSplineType = GetInt(SplineType) != 0; // Stock Shader had an int that it used like a bool
	
	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		// These are only required during Snapshot
		bool bSecondSequence = GetBool(DualSequence);
		bool bAddOverBlend = GetBool(AddOverBlend);
		bool bExtractGreenAlpha = GetBool(ExtractGreenAlpha);
		bool bBlendFrames = !bIsSplineType && GetBool(BlendFrames);
		bool bMaxLumFrameBlend1 = GetBool(MaxLumFrameBlend1);
		// UseInstancing nuked since its x360 only

		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// "draw back-facing because of yaw spin"
		// I assume this comment is supposed to tell us that we always want backface culling,
		// Because when particles rotate you can probably see their backsides
		// So if we Culled the Backfaces you would just see nothing.
		pShaderShadow->EnableCulling(false);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		// "Be sure not to write to dest alpha"
		// Overwriting any existing Alpha would be problematic..
		pShaderShadow->EnableAlphaWrites(false);

		if (!GetBool(Opaque))
		{
			// Alphatest Support
			// ALPHATEST Flag check here is custom, that way you can just force it when you really really want it.
			// Hopefully no existing Material has that flag without it actually working..
			// FIXME: Consider bAddOverBlend(?), AlphaBlending is enabled under it...
			// FIXME2: Consider $Additive(?), do we always just have additive && alphatested? Seems rather wasteful.
			// It'd be better to multiply by Input Alpha...
			if (bAdditive2ndTexture || bAddSelf) // HasFlag(MATERIAL_VAR_ALPHATEST)
				pShaderShadow->EnableAlphaTest(false);
			else
				pShaderShadow->EnableAlphaTest(true);

			// Just always do this(?), this is usually only used in combination with $AlphaTest.
			// This should only affect AlphaTesting
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GREATER, GetFloat(AlphaFuncValue));

			// Mod2X adapted from ASW
			if(GetBool(Mod2X))
			{
				EnableAlphaBlending(SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR);
			}
			// Stock-Consistency: Have to do this,
			// !(bAdditive2ndTexture || bAddSelf) activates AlphaTesting
			else if (bAdditive2ndTexture || bAddOverBlend || bAddSelf)
			{
				EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				if (HasFlag(MATERIAL_VAR_ADDITIVE))
				{
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE);
				}
				else
				{
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
				}
			}
		} // !Opaque

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		unsigned int flags = VERTEX_POSITION | VERTEX_COLOR;

		// TODO: Update the comments here, just make a big box that explains exactly whats going on
		// Because this is extremely vague and tells you absolutely nothing
		// bounding uv's? How does that look like and work
		// for multiple frames? wha?
		// corner identifiers of 0,1 ? what is that supposed to mean and why we only need 2?
		static int s_TexCoordSize[8] =
		{
			4,				// 0 = sheet bounding uvs, frame0
			4,				// 1 = sheet bounding uvs, frame 1
			4,				// 2 = frame blend, rot, radius, yaw
			2,				// 3 = corner identifier ( 0/0,1/0,1/1, 1/0 )
			4,				// 4 = Texture 2 bounding uvs
			4,				// 5 = second sequence bounding uvs, frame0
			4,				// 6 = second sequence bounding uvs, frame1
			4,				// 7 = second sequence frame blend, ?,?,?
		};

		static int s_TexCoordSizeSpline[8] =
		{
			4,				// 0 = sheet bounding uvs, frame0
			4,				// 1 = sheet bounding uvs, frame 1
			4,				// 2 = frame blend, rot, radius, ???
			4,				// 3 = corner identifier ( 0/0,1/0,1/1, 1/0 )
			4,				// 4 = Texture 2 bounding uvs
			4,				// 5 = second sequence bounding uvs, frame0
			4,				// 6 = second sequence bounding uvs, frame1
			4,				// 7 = second sequence frame blend, ?,?,?
		};

		int numTexCoords = 5;

		if (bSecondSequence)
		{
			// "the whole shebang - 2 sequences, with a possible multi-image sequence first"
			numTexCoords = 8;
		}

		pShaderShadow->VertexShaderVertexFormat(flags,
			numTexCoords,
			bIsSplineType ? s_TexCoordSizeSpline : s_TexCoordSize, 0);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. sRGB when not extracting
		EnableSampler(SHADER_SAMPLER0, !bExtractGreenAlpha);

		// s1 - $RampTexture. Always sRGB
		EnableSampler(bHasRampTexture, SHADER_SAMPLER1, true);

		// s2 - Depth Buffer. Never sRGB ( we only read Alpha )
		EnableSampler(bDepthBlend, SHADER_SAMPLER2, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		if (bIsSplineType)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_spritecard_spline_vs30);
			SET_STATIC_VERTEX_SHADER(lux_spritecard_spline_vs30);

			// We need to make sure none of these are used
			bSecondSequence = false;
			bBlendFrames = false;
			bMaxLumFrameBlend1 = false;
			bExtractGreenAlpha = false;
		}
		else
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_spritecard_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(DUALSEQUENCE, bSecondSequence);
			SET_STATIC_VERTEX_SHADER_COMBO(ZOOM_ANIMATE_SEQ2, bZoomSeq2);
			SET_STATIC_VERTEX_SHADER_COMBO(EXTRACTGREENALPHA, bExtractGreenAlpha);
			SET_STATIC_VERTEX_SHADER_COMBO(ADDBASETEXTURE2, bAdditive2ndTexture && !bAddSelf);
			SET_STATIC_VERTEX_SHADER(lux_spritecard_vs30);
		}

		// All additional statements are safeguards for skipped combos!
		// See the shaderfile itself for why they are skipped ( Mututally exclusive by design )
		DECLARE_STATIC_PIXEL_SHADER(lux_spritecard_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(ANIMBLEND, bBlendFrames);
		SET_STATIC_PIXEL_SHADER_COMBO(MAXLUMFRAMEBLEND1, !bBlendFrames && bMaxLumFrameBlend1);
		SET_STATIC_PIXEL_SHADER_COMBO(EXTRACTGREENALPHA, bExtractGreenAlpha && !(!bAdditive2ndTexture && (bBlendFrames || GetBool(MaxLumFrameBlend1))));
		SET_STATIC_PIXEL_SHADER_COMBO(ADDBASETEXTURE2, bAdditive2ndTexture && !bAddSelf);
		SET_STATIC_PIXEL_SHADER_COMBO(ADDSELF, bAddSelf && !bAdditive2ndTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(DUALSEQUENCE, bSecondSequence);
		SET_STATIC_PIXEL_SHADER_COMBO(MAXLUMFRAMEBLEND2, bSecondSequence && GetBool(MaxLumFrameBlend1));
		SET_STATIC_PIXEL_SHADER_COMBO(SEQUENCE_BLEND_MODE, bSecondSequence ? GetInt(Sequence_Blend_Mode) : 0);
		SET_STATIC_PIXEL_SHADER_COMBO(COLORRAMP, bHasRampTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(DEPTHBLEND, bDepthBlend);
		SET_STATIC_PIXEL_SHADER_COMBO(MOD2X, GetBool(Mod2X));
		SET_STATIC_PIXEL_SHADER(lux_spritecard_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// Just always do Base even if we don't have one?
		// Kinda insane but I won't mind. Preferably there should be some logic here for binding some kinda debug Texture.
		// For example, bind a grey Texture 
		BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);
		BindTexture(bHasRampTexture, SHADER_SAMPLER1, RampTexture, Frame);
		BindTexture(bDepthBlend, SHADER_SAMPLER2, TEXTURE_FRAME_BUFFER_FULL_DEPTH);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Always needed
		LoadViewportTransformScaledIntoVertexShaderConstant(LUX_VS_FLOAT_SET0_0);

		// NOTE: Why clamp this every frame?
		// Is it possible or intended to change the Orientation type via proxy??
		// I.m.h.o. this should happen on Param Init, anyone using clearly out-of-boundary values shall receive the missing combo error!
		// Following pieced together :
		// 0 = Screen-Orienting
		// 1 = Z-Aligned
		// 2 = Z-Aligned cheap
		int nOrientation = clamp(GetInt(Orientation), 0, 2);

		// Only required when Screen Orienting
		if (nOrientation == 0)
		{
			// 48, 49, 50 is a float4x3
			LoadModelViewMatrixIntoVertexShaderConstant(LUX_VS_FLOAT_SET1_0);
			
			// 51, 52, 53, 54 is a float4x4
			LoadProjectionMatrixIntoVertexShaderConstant(LUX_VS_FLOAT_SET1_3);
		}

		if (bZoomSeq2)
		{
			// Neat, they already precomputed the RCP of this
			// NOTE: f1ZoomAnimteSeq2 is always above 1.0,
			// since that is the requirement for bZoomSeq2 to be true
			// Also: This used to be called c0 when its clearly c55
			float f1ZScale = 1.0 / f1ZoomAnimSeq2;
			float4 cZoomControl = 0.0f;
			cZoomControl.x = 0.5f * (1.0f + f1ZScale);
			cZoomControl.y = f1ZScale;

			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_7, cZoomControl);
		}

		// This is basically just stock code :
		// {
		float f1MaxDistance = GetFloat(MaxDistance);
		float flStartFade = f1MaxDistance - GetFloat(FarFadeInterval);
		flStartFade = MAX(1.0f, flStartFade);
	
		float4 cSizeControls1;
		cSizeControls1.x = GetFloat(MinSize);
		cSizeControls1.y = GetFloat(MaxSize);
		cSizeControls1.z = GetFloat(StartFadeSize);
		cSizeControls1.w = GetFloat(EndFadeSize);

		float4 cSizeControls2 = 0.0f;
		cSizeControls2.x = flStartFade;
		cSizeControls2.y = 1.0 / (f1MaxDistance - flStartFade); // RCP Precomputed
		// .zw free!

		pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_8, cSizeControls1);
		pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_9, cSizeControls2);
		// }

		float4 f4_PS_C0 = 0.0f;
		f4_PS_C0.x = f1AddBaseTexture2;
		f4_PS_C0.y = GetFloat(OverbrightFactor);
		f4_PS_C0.z = f1AddSelf;

		// "Set Mod2xIdentity to be 0.5 if we blend in linear space, or 0.5 Gamma->Linear if we blend in gamma space"
		if(GetBool(Mod2X))
			f4_PS_C0.w = g_pHardwareConfig->UsesSRGBCorrectBlending() ? 0.5f : SrgbGammaToLinear(0.5f);

		// In case of Custom Depth we can include our factor with c0,
		// Otherwise have to pack it on c2 using the stock function
#if defined(CUSTOM_DEPTH_RANGE)
		// See lux_common_depth.h for more information as to why this is done.
		f4_PS_C0.w = (DEPTH_RANGE_FAR_Z - DEPTH_RANGE_NEAR_Z) / GetFloat(DepthBlendScale);
#else
		// Always need this since we have to feather depth
		pShaderAPI->SetDepthFeatheringPixelShaderConstant(REGISTER_FLOAT_003, GetFloat(DepthBlendScale));
#endif
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4_PS_C0);

		// We just overwrite the alpha afterwards
		float4 f4_PS_C2 = ComputeTint(!GetBool(NoTint), Alpha);
		f4_PS_C2.w = (float)GetBool(InverseDepthBlend);

		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_002, f4_PS_C2);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		if (bIsSplineType)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_spritecard_spline_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_spritecard_spline_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_spritecard_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(ORIENTATION, nOrientation);
			SET_DYNAMIC_VERTEX_SHADER(lux_spritecard_vs30);
		}

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_spritecard_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_spritecard_ps30);
	}

	Draw();
}
END_SHADER