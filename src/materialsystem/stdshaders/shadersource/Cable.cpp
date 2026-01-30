//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Cable Shaders
//	TODO: Figure out if using the Lightstate works for this Shader
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_cable_vs30.inc"
#include "lux_cable_ps30.inc"
#include "lux_cable_spline_vs30.inc"
#include "lux_cable_spline_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Cable, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(SDK_Cable_DX9, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(SDK_Cable_DX8, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(SDK_Cable_DX6, LUX_Rope_Router)
#endif

#ifdef REPLACE_CABLE
DEFINE_FALLBACK_SHADER(Cable, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(Cable_DX9, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(Cable_DX8, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(Cable_DX6, LUX_Rope_Router)
#endif

DEFINE_FALLBACK_SHADER(LUX_Cable_DX6, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(LUX_Cable_DX8, LUX_Rope_Router)
DEFINE_FALLBACK_SHADER(LUX_Cable_DX9, LUX_Rope_Router)

// L4D Uses this specific Shader Name for Spline Ropes
// Since we don't have it on the SDK, this is a safe replace
// Note that we send it to the router, so if you don't have splineropes it will use regular ones. 
DEFINE_FALLBACK_SHADER(SplineRope, LUX_Rope_Router)

//==========================================================================//
// Fallback Shader to control what variant we should actually be using
//==========================================================================//
BEGIN_VS_SHADER(LUX_Rope_Router, "A shader used for ropes such as those created by the keyframe_rope/move_rope entities.")
SHADER_INFO_GEOMETRY	("Rope entites. (move_rope or keyframe_rope).")
SHADER_INFO_USAGE		("See this Article for how to place Ropes: https://developer.valvesoftware.com/wiki/Cables_and_Ropes\n"
						 "As for Material Creation, it is important to mention some of the Issues that the stock Cable Shader has.\n"
						 "By default Vertex Alpha is applied to the $BaseTexture's Alpha.\n"
						 "The Value of the Vertex Alpha appears to be.. not 1.0f. This causes Alpha to always be modulated ( weaker )."
						 "This makes it impossible to use $Translucent on Ropes unless you want see through Ropes.\n"
						 "To fix this, LUX adds a new Parameter ($NoAlphaInfluence). It disables this Alpha Modulation, making $Translucent usabe.\n"
						 "Directional Shadows use a hardcoded Light Direction ( Up Vector ).\n"
						 "Areas on a Rope that face down will be shadowed, this is not necessarily an accurate Representation of Scene Lighting.\n"
						 "This was likely done since Cables are usually used in outdoor Areas ( Where Sunlight would come from above )\n"
						 "LUX adds $NoLightDirInfluence, which disables this directional Shadowing.")
SHADER_INFO_LIMITATIONS	("Only works correctly on Cables made with keyframe_rope and move_rope.")
SHADER_INFO_PERFORMANCE	("Very Cheap Shader.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(Legacy_Cable,	SHADER_PARAM_TYPE_BOOL, "0", "Forces to fallback to the LUX_Cable Shader specifically. (Old Cable Methods)")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
}
SHADER_FALLBACK
{
#ifndef REPLACE_CABLE
	if (lux_oldshaders.GetBool())
		return "Cable_DX9";
#endif

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}

	// Hammer Needs Regular Cable Shader
	if(g_pConfig->bEditMode)
		return "LUX_Cable";

	// This Parameter is only here for compatibility Reasons
	// ( If you want to keep a Shader as SDK_ but want it to really use the old Shader )
	if (GetBool(Legacy_Cable))
		return "LUX_Cable";

	// New default Expectation: We are running a non-Mapbase DLL and SPLINEROPES indicates whether we need spline or default Ropes.
	// lux_cable_forcespline is now 0 at default..
	// That makes more sense anyways, how would you force SplineRopes with a Parameter that is always 1...
	if (lux_cable_forcespline.GetBool())
		return "LUX_Cable_Spline";

	#ifdef SPLINEROPES
		return "LUX_Cable_Spline";
	#else
		return "LUX_Cable";
	#endif
}

SHADER_INIT
{

}

SHADER_DRAW
{
	Draw(false);
}
END_SHADER

//==========================================================================//
// Shader Start LUX_Cable
//==========================================================================//
BEGIN_VS_SHADER(LUX_Cable, "A Shader used for Ropes and Cables made using keyframe_rope/move_rope Entities. (Note: Use LUX_Rope_Router instead)." )
SHADER_INFO_GEOMETRY	("See LUX_Rope_Router for more Information.")
SHADER_INFO_USAGE		("See LUX_Rope_Router for more Information.")
SHADER_INFO_LIMITATIONS	("See LUX_Rope_Router for more Information.")
SHADER_INFO_PERFORMANCE	("See LUX_Rope_Router for more Information.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(BumpMap,				SHADER_PARAM_TYPE_TEXTURE,	"",	"[RGB] Bump Map or Normal Map.\n[A] Nothing.")
	SHADER_PARAM(BumpFrame,				SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Var for $BumpMap.")
	SHADER_PARAM(BumpTransform,			SHADER_PARAM_TYPE_MATRIX,	"",	"Transforms the $BumpMap. Must include all Values!")
	SHADER_PARAM(NoAlphaInfluence,		SHADER_PARAM_TYPE_BOOL,		"",	"Stop Vertex Lighting from affecting Alpha Output, makes Opacity Effects predictable.")
	SHADER_PARAM(NoLightDirInfluence,	SHADER_PARAM_TYPE_BOOL,		"",	"Stops the hardcoded Light Dir to affect this Material.")
	SHADER_PARAM(ShadowDepth,			SHADER_PARAM_TYPE_BOOL,		"",	"Whether or not to write Depth.")
	SHADER_PARAM(LightDir,				SHADER_PARAM_TYPE_VEC3,		"",	"Allows overriding the default Light Vector of [0 0 1]")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	DefaultFloat3(LightDir, 0.0f, 0.0f, 1.0f);
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
	LoadBumpMap(BumpMap);
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasBaseTextureTransform = HasTransform(bHasBaseTexture, BaseTextureTransform);
	bool bIgnoreLightDirInfluence = GetBool(NoLightDirInfluence);
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bBindDefaultNormal = !bHasNormalTexture && !bIgnoreLightDirInfluence;
	bHasNormalTexture = bHasNormalTexture && !bIgnoreLightDirInfluence;
	bool bHasNormalTextureTransform = HasTransform(!bBindDefaultNormal && bHasNormalTexture, BumpTransform);

	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, -1, -1);
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

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

		// Everything Transparency is packed into this Function
		EnableTransparency(nBlendType);

		// This is now handled in the Flashlight sampler setup
		DefaultFog();

		// We always need this
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		if (GetBool(ShadowDepth))
			pShaderShadow->EnableDepthWrites(false);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Stock Consistency : All the same here
		unsigned int flags = VERTEX_POSITION | VERTEX_COLOR | VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		int nTexCoordDim[2] = { 2, 2 };
		int nTexCoords = 2;

		pShaderShadow->VertexShaderVertexFormat(flags, nTexCoords, nTexCoordDim, 0);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		EnableSampler(SAMPLER_BASETEXTURE, true); // We always have a BaseTexture, and yes they should always be sRGB
		EnableSampler(!bIgnoreLightDirInfluence, SAMPLER_NORMALMAP, false); // No sRGB

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_cable_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(BUMPMAPPING, !bIgnoreLightDirInfluence);
		SET_STATIC_VERTEX_SHADER_COMBO(BASETRANSFORM, bHasBaseTextureTransform);	// 1 = normal, 2 = seamless
		SET_STATIC_VERTEX_SHADER_COMBO(BUMPTRANSFORM, bHasNormalTextureTransform);
		SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, 0);
		SET_STATIC_VERTEX_SHADER(lux_cable_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_cable_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(IGNORELIGHTDIR, bIgnoreLightDirInfluence);
		SET_STATIC_PIXEL_SHADER_COMBO(IGNOREVERTEXALPHA, GetBool(NoAlphaInfluence));
		SET_STATIC_PIXEL_SHADER(lux_cable_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		// if mat_fullbright 2. Bind a standard white Texture...
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		else
#endif
			BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_WHITE);

		if (bBindDefaultNormal)
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		else
			BindTexture(bHasNormalTexture, SAMPLER_NORMALMAP, BumpMap, BumpFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// c10 - $LightDir
		if(!GetBool(NoLightDirInfluence))
		{
			float4 f4LightDir;
			f4LightDir.xyz = GetFloat3(LightDir);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_010, f4LightDir);
		}

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
		
		// c32 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Setup Vertex Shader Constant Registers
		//==================================================================================================

		// VS c223, c224
		if (bHasBaseTexture)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// VS c225, c226
		if (bHasNormalTexture)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_cable_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_cable_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_cable_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_cable_ps30);
	}

	Draw();
}
END_SHADER

//==========================================================================//
// SplineRope Shader Start
//==========================================================================//
// FIXME PRE-RELEASE: Move to ConVars c++
static ConVar rope_min_pixel_diameter("rope_min_pixel_diameter", "2.0", FCVAR_CHEAT);

BEGIN_VS_SHADER(LUX_Cable_Spline, "A Shader used for Ropes and Cables made using keyframe_rope/move_rope Entities. (Note: Use LUX_Rope_Router instead)." )
SHADER_INFO_GEOMETRY	("See LUX_Rope_Router for more Information.")
SHADER_INFO_USAGE		("See LUX_Rope_Router for more Information.")
SHADER_INFO_LIMITATIONS	("See LUX_Rope_Router for more Information.")
SHADER_INFO_PERFORMANCE	("See LUX_Rope_Router for more Information.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(BumpMap,				SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGB] Bump Map or Normal Map.\n[A] Nothing.")
	SHADER_PARAM(BumpFrame,				SHADER_PARAM_TYPE_INTEGER,	"0",	"Frame number for $BumpMap.")
	SHADER_PARAM(BumpTransform,			SHADER_PARAM_TYPE_MATRIX,	"",		"Transforms the $BumpMap Texture. Must include all values!")
	SHADER_PARAM(NoAlphaInfluence,		SHADER_PARAM_TYPE_BOOL,		"",	"Stop Vertex Lighting from affecting Alpha Output, makes Opacity Effects predictable.")
	SHADER_PARAM(NoLightDirInfluence,	SHADER_PARAM_TYPE_BOOL,		"",	"Stops the hardcoded Light Dir to affect this Material.")
	SHADER_PARAM(ShadowDepth,			SHADER_PARAM_TYPE_BOOL,		"",	"Whether or not to write Depth.")
	SHADER_PARAM(LightDir,				SHADER_PARAM_TYPE_VEC3,		"",	"Allows overriding the default Light Vector of [0 0 1]")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	if (!IsDefined(BumpMap))
		SetString(BumpMap, "cable/cablenormalmap");

	DefaultFloat3(LightDir, 0.0f, 0.0f, 1.0f);

	// "What's this for?"
	// This is odd, what does it do here?
	SetFlag2(MATERIAL_VAR2_IS_SPRITECARD);
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
	// This is pretty much consistent with Mapbase
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadBumpMap(BumpMap);
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bIgnoreLightDirInfluence = GetBool(NoLightDirInfluence);
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bBindDefaultNormal = !bHasNormalTexture && !bIgnoreLightDirInfluence;
	bHasNormalTexture = bHasNormalTexture && !bIgnoreLightDirInfluence;
	bool bWriteShadowDepth = GetBool(ShadowDepth);

	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true);
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

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

		if (bWriteShadowDepth)
		{
			// Don't bother writing literally anything
			pShaderShadow->EnableColorWrites(false);
			pShaderShadow->EnableAlphaWrites(false);

			// "polyoffset for shadow maps."
			pShaderShadow->EnablePolyOffset(SHADER_POLYOFFSET_SHADOW_BIAS);

			// We probably don't want any Fog during Depth
			FogToBlack();
		}
		else
		{
			// This is now handled in the Flashlight sampler setup
			DefaultFog();

			// Everything Transparency is packed into this Function
			EnableTransparency(nBlendType);

			// We always need this? What about Transparency?
			// "We need to write to dest alpha for depth feathering."
			pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);

			// Weird name, what it actually means : We output linear values
			pShaderShadow->EnableSRGBWrite(true);

			//==========================================================================//
			// Sampler Setup
			//==========================================================================//

			EnableSampler(SAMPLER_BASETEXTURE, true); // We always have a BaseTexture, and yes they should always be sRGB
			EnableSampler(!bIgnoreLightDirInfluence, SAMPLER_NORMALMAP, false); // No sRGB
		}

		// "draw back-facing because of yaw spin"
		pShaderShadow->EnableCulling(false);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Stock Consistency : All the same here
		unsigned int nFlags = VERTEX_POSITION | VERTEX_COLOR;
		int nTexCoordDim[4] =
		{
			4, // (worldspace xyz) (radius (diameter?) of spline at this point) for first control point
			4, // (worldspace xyz) (radius of spline at this point) for second control point
			4, // (worldspace xyz) (radius of spline at this point) for third control point
			4  // (worldspace xyz) (radius of spline at this point) for fourth control point
		};
		int nTexCoords = 4;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, nTexCoordDim, nUserDataSize);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_cable_spline_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(DEPTH_WRITE, bWriteShadowDepth);
		SET_STATIC_VERTEX_SHADER(lux_cable_spline_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_cable_spline_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(DEPTH_WRITE, bWriteShadowDepth);
		SET_STATIC_PIXEL_SHADER_COMBO(IGNORELIGHTDIR, GetBool(NoLightDirInfluence));
		SET_STATIC_PIXEL_SHADER_COMBO(IGNOREVERTEXALPHA, GetBool(NoAlphaInfluence));
		SET_STATIC_PIXEL_SHADER(lux_cable_spline_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		if (!bWriteShadowDepth)
		{
			//==========================================================================//
			// Bind Textures
			//==========================================================================//
			// if mat_fullbright 2. Bind a standard white Texture...
#ifdef DEBUG_FULLBRIGHT2 
			if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
				BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
			else
#endif
				BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_WHITE);

			if (bBindDefaultNormal)
				BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
			else
				BindTexture(bHasNormalTexture, SAMPLER_NORMALMAP, BumpMap, BumpFrame);

			//==========================================================================//
			// Setup Constant Registers
			//==========================================================================//

			// c10 - $LightDir
			if(!GetBool(NoLightDirInfluence))
			{
				float4 f4LightDir;
				f4LightDir.xyz = GetFloat3(LightDir);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_010, f4LightDir);
			}

			// c11 - Camera Position
			SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
			
			// c12 - Fog Params
			pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
			
			// c32 - $Color, $Color2, $sRGBTint
			float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

			// b13, b14, b15
			BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);
		}
		else
		{
			// b13, b14, b15
			BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(true);
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(true);
		}

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Setup Vertex Shader Constant Registers
		//==================================================================================================

		// "We need these only when screen-orienting, which we are always in this shader."
		LoadModelViewMatrixIntoVertexShaderConstant(LUX_VS_FLOAT_SET1_0);
		LoadProjectionMatrixIntoVertexShaderConstant(LUX_VS_FLOAT_SET1_3);

		if (!bWriteShadowDepth)
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// Get viewport and render target dimensions and set shader constant to do a 2D mad
		ShaderViewport_t CurrentViewport;
		pShaderAPI->GetViewports(&CurrentViewport, 1);

		float4 f4c7;
		if (!g_pHardwareConfig->IsAAEnabled())
		{
			float flMinPixelDiameter = rope_min_pixel_diameter.GetFloat() / (float)CurrentViewport.m_nWidth;
			f4c7 = flMinPixelDiameter;
		}
		pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_7, f4c7);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_cable_spline_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_cable_spline_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_cable_spline_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_cable_spline_ps30);
	}

	Draw();
}
END_SHADER