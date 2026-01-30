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
#include "lux_model_simplified_vs30.inc"
#include "lux_sky_hdri_ps30.inc"

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Sky_Equirectangular, "A Sky Shader with support for so called HDRI Textures.")
SHADER_INFO_GEOMETRY	("Can be applied via Map Properties, Brushes and Models.\n"
						 "Although a bit unconventional, may be used on animated Models as well.")
SHADER_INFO_USAGE		("$BaseTexture for simple Equirectangular Projection.\n"
						 "Textures cut in half to reduce Filesize may use $HalfHDRI."
						 "If there is a Resolution Limit to the VTFs, $SkyTextureX1Y1 etc can be used to use cut up versions of a larger Texture.\n"
						 "If the Equirectangular Projection appears flipped, use $FlipX.\n"
						 "The sRGBness of the Textures and Write Output can be modified using $WriteSRGB and $TextureSRGB.\n"
						 "For proper Depthtesting use $NoIgnoreZ.\n"
						 "The Texture(s) may be tinted with $HDRITint.")
SHADER_INFO_LIMITATIONS	("Does not support compressed HDR Formats like RGBM or RGBE.\n"
						 "Assuming a maximum VTF Resolution of 4k, the theoratical limit of this Shader is 16384x8192.")
SHADER_INFO_PERFORMANCE	("Fast.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Sky Shader Page: https://developer.valvesoftware.com/wiki/Sky_(Source_1_shader)")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

/*
	How m_nMode works :

	o o o o
	Mode 1		->	x o o o
	Maximum res of 4096x2048 ( 1x1 Textures )
	The aspect ratio of a single Texture will be 2:1
	
	o o o o
	Mode 2		->	x x o o
	Maximum res of 8192x4096 ( 2x1 Textures ) - Full res HDRI
	Maximum res of 8192x2048 ( 2x1 HalfHDRI ) - Cut in half to save filesize and performance.
	
	x x x x
	Mode 3		->	x x x x
	Maximum res of 16384x8192 ( 4x2 Textures ) - Full res HDRI
	Maximum res of 16384x4096 ( 4x1 HalfHDRI ) - Cut in half to save filesize and performance.
*/

BEGIN_SHADER_PARAMS
	SHADER_PARAM(SkyTextureX1Y1,	SHADER_PARAM_TYPE_TEXTURE,	"", "The entire Equirectangular Projection Texture or part of it.")
	SHADER_PARAM(SkyTextureX2Y1,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX3Y1,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX4Y1,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX1Y2,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX2Y2,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX3Y2,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(SkyTextureX4Y2,	SHADER_PARAM_TYPE_TEXTURE,	"", "Part of an Equirectangular Projection Texture ( HDRI )")
	SHADER_PARAM(HalfHDRI,			SHADER_PARAM_TYPE_BOOL,		"", "Cuts the Projcetion's in Half, useful for reducing Filesize by removing the lower half of the Projection.\n")
	SHADER_PARAM(Mode,				SHADER_PARAM_TYPE_INTEGER,	"", "(INTERNAL PARAMETER), don't use.")
	SHADER_PARAM(Stretch,			SHADER_PARAM_TYPE_BOOL,		"", "When using Half-HDRI's: Stretches the bottom end of the HDRI's across the missing lower half ( Similar to Clamp Flags on a regular Sky Shader )")
	SHADER_PARAM(HDR,				SHADER_PARAM_TYPE_BOOL,		"", "Set to 1 if your Texture(s) are HDR and need to be multiplied by 16.0f.")
	SHADER_PARAM(WritesRGB,			SHADER_PARAM_TYPE_BOOL,		"", "Determines how Shader Output is being used. If set to 1 it will convert from Linear to sRGB.")
	SHADER_PARAM(TexturesRGB,		SHADER_PARAM_TYPE_BOOL,		"", "Load Textures as sRGB.")
	SHADER_PARAM(Rotation,			SHADER_PARAM_TYPE_INTEGER,	"", "Rotation in degrees.")
	SHADER_PARAM(HDRITint,			SHADER_PARAM_TYPE_COLOR,	"", "Multiplied with the HDR Factor, it changes the Color of your HDRI's.")
	SHADER_PARAM(FlipX,				SHADER_PARAM_TYPE_BOOL,		"",	"Flips the X Axis, if for some reason some the HDRI is flipped.")
	SHADER_PARAM(NoIgnoreZ,			SHADER_PARAM_TYPE_BOOL,		"",	"Originally intended for building Cubemaps. Removes NoFog and IgnoreZ Rules.")
	SHADER_PARAM(AllowFog,			SHADER_PARAM_TYPE_BOOL,		"", "Enables Fogging Behaviours.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	if(!GetBool(AllowFog))
		SetFlag(MATERIAL_VAR_NOFOG);
		
	if (!IsDefined(NoIgnoreZ) && !GetBool(NoIgnoreZ))
		SetFlag(MATERIAL_VAR_IGNOREZ);

	if (IsDefined(SkyTextureX2Y1))
		SetInt(Mode, 1);

	if (IsDefined(SkyTextureX3Y1))
		SetInt(Mode, 2);
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
	// TextureFlags don't matter here
	LoadTexture(BaseTexture, 0);
	LoadTexture(SkyTextureX1Y1, 0);
	LoadTexture(SkyTextureX2Y1, 0);
	LoadTexture(SkyTextureX3Y1, 0);
	LoadTexture(SkyTextureX4Y1, 0);
	LoadTexture(SkyTextureX1Y2, 0);
	LoadTexture(SkyTextureX2Y2, 0);
	LoadTexture(SkyTextureX3Y2, 0);
	LoadTexture(SkyTextureX4Y2, 0);
}

SHADER_DRAW
{
	int nMode = GetInt(Mode);
	bool bIsHalfHDRI = GetBool(HalfHDRI);
	bool bHasStretch = GetBool(Stretch);
	bool bHDR = GetBool(HDR);
	bool bWriteSRGB = GetBool(WritesRGB);
	bool bTextureSRGB = GetBool(TexturesRGB);

	// Always required, minimum of the 2:1 ( Mode 0 )
	bool bHasTextureX1Y1 = IsTextureLoaded(SkyTextureX1Y1);

	// Only Available with 2 textures on X ( Mode 1 )
	bool bHasTextureX2Y1 = IsTextureLoaded(SkyTextureX2Y1) && (nMode == 1);

	// Only Available with 4 textures on X ( Mode 2 )
	bool bHasTextureX3Y1 = IsTextureLoaded(SkyTextureX3Y1) && (nMode == 2);
	bool bHasTextureX4Y1 = IsTextureLoaded(SkyTextureX4Y1) && (nMode == 2);

	// Only Available with 4 textures on X ( Mode 3 ). NOT usable with HalfHDRI's
	bool bHasTextureX1Y2 = IsTextureLoaded(SkyTextureX1Y2) && (nMode == 2) && !bIsHalfHDRI;
	bool bHasTextureX2Y2 = IsTextureLoaded(SkyTextureX2Y2) && (nMode == 2) && !bIsHalfHDRI;
	bool bHasTextureX3Y2 = IsTextureLoaded(SkyTextureX3Y2) && (nMode == 2) && !bIsHalfHDRI;
	bool bHasTextureX4Y2 = IsTextureLoaded(SkyTextureX4Y2) && (nMode == 2) && !bIsHalfHDRI;

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// Handles most Things
		SetInitialShadowState();

		// we are writing linear values from this shader.
		pShaderShadow->EnableSRGBWrite(true);

		// Always for DepthToDestAlpha
		pShaderShadow->EnableAlphaWrites(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//
		EnableSampler(SHADER_SAMPLER0, bTextureSRGB); // TextureX1Y1 and BaseTexture. Always needed
		EnableSampler(bHasTextureX2Y1, SHADER_SAMPLER1, bTextureSRGB);
		EnableSampler(bHasTextureX3Y1, SHADER_SAMPLER2, bTextureSRGB);
		EnableSampler(bHasTextureX4Y1, SHADER_SAMPLER3, bTextureSRGB);
		EnableSampler(bHasTextureX1Y2, SHADER_SAMPLER4, bTextureSRGB);
		EnableSampler(bHasTextureX2Y2, SHADER_SAMPLER5, bTextureSRGB);
		EnableSampler(bHasTextureX3Y2, SHADER_SAMPLER6, bTextureSRGB);
		EnableSampler(bHasTextureX4Y2, SHADER_SAMPLER7, bTextureSRGB);

		//==================================================================================================
		// Set Static Shaders
		//==================================================================================================

		// All we want is Position
		DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);
		SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_sky_hdri_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(MODE, nMode);
		SET_STATIC_PIXEL_SHADER_COMBO(HALF, bIsHalfHDRI);
		SET_STATIC_PIXEL_SHADER_COMBO(STRETCH, bHasStretch);
		SET_STATIC_PIXEL_SHADER_COMBO(TOSRGB, bWriteSRGB);
		SET_STATIC_PIXEL_SHADER_COMBO(INVERTX, GetBool(FlipX));
		SET_STATIC_PIXEL_SHADER(lux_sky_hdri_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		if (bHasTextureX1Y1)
			BindTexture(SHADER_SAMPLER0, SkyTextureX1Y1, 0);
		else
			BindTexture(IsTextureLoaded(BaseTexture), SHADER_SAMPLER0, BaseTexture, 0, TEXTURE_BLACK);

		BindTexture(bHasTextureX2Y1, SHADER_SAMPLER1, SkyTextureX2Y1, 0);
		BindTexture(bHasTextureX3Y1, SHADER_SAMPLER2, SkyTextureX3Y1, 0);
		BindTexture(bHasTextureX4Y1, SHADER_SAMPLER3, SkyTextureX4Y1, 0);
		BindTexture(bHasTextureX1Y2, SHADER_SAMPLER4, SkyTextureX1Y2, 0);
		BindTexture(bHasTextureX2Y2, SHADER_SAMPLER5, SkyTextureX2Y2, 0);
		BindTexture(bHasTextureX3Y2, SHADER_SAMPLER6, SkyTextureX3Y2, 0);
		BindTexture(bHasTextureX4Y2, SHADER_SAMPLER7, SkyTextureX4Y2, 0);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		if(GetBool(AllowFog))
			pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		float4 f4Controls;
		f4Controls.xyz = GetFloat3(HDRITint);

		// Rotational Factor.
		const float OneDividedBy360 = 1.0f / 360.0f;
		f4Controls[3] = GetInt(Rotation) * OneDividedBy360;
		if (bHDR)
		{
			// 8 seems more correct than Valve's 16...
			f4Controls.x *= 8.0f;
			f4Controls.y *= 8.0f;
			f4Controls.z *= 8.0f;
		}
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_001, f4Controls);

		// Use Boolean Registers instead of Dynamic Pixel Shader Combos
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// I wonder if this is actually true at any time whatsoever
		BBools[LUX_PS_BOOL_RADIALFOG] = GetBool(AllowFog) && HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = pShaderAPI->ShouldWriteDepthToDestAlpha();

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_sky_hdri_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_sky_hdri_ps30);
	}

	Draw();
}
END_SHADER