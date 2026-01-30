//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	08.07.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	LUX_Black
// 
//	The 'Black' Shader can be found in Source Branches ever since L4D2
//	Luckily, it's incredibly simple. It's.. Black, with Fog on it.
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_model_simplified_vs30.inc"
#include "lux_black_ps30.inc"

// LUX Shaders will replace existing Shaders.
// Now I don'T think there ever was a SDK_Black,
// Let's have this.. just in case
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Black, LUX_Black)
#endif

#ifdef REPLACE_BLACK
DEFINE_FALLBACK_SHADER(Black, LUX_Black)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Black, "A shader used by tools/toolsblack_cheap and tools/toolsblack_noportal_skybox.")
SHADER_INFO_GEOMETRY	("Brushes and Models and Displacements.")
SHADER_INFO_USAGE		("Apply to a Surface, any Surface.")
SHADER_INFO_LIMITATIONS	("It is only affected by Fog, it does not take any Parameters.")
SHADER_INFO_PERFORMANCE	("Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Black Shader Page: https://developer.valvesoftware.com/wiki/Black")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS

// None?
// 
// We have a Reference VMT for this
// But I don't think any of these Parameters exist/work..
//
//	$basetexture "sign_warning_12"
//	$fogcolor	"[0.5 0.5 0.5]"
//	$fogstart	"100.0"
//	$fogend		"1000.0"

END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// No Flashlight ever
	SetBool(ReceiveProjectedTextures, false);

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}

SHADER_FALLBACK
{
#ifndef REPLACE_BLACK
	if (lux_oldshaders.GetBool())
		return "Black";
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
	// There really isn't much to do
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

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Never blend
		DisableAlphaBlending();
		
		// Yes
		pShaderShadow->EnableDepthWrites(true);

		// Need this for DepthToDestAlpha and Height FogFactor
		pShaderShadow->EnableAlphaWrites(true);

		// This.. Technically doesn't matter? Just Fog is affected
		pShaderShadow->EnableSRGBWrite(true);

		// Yeah.. Fog.. Nothin' else
		DefaultFog();

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = 0;

		// NOTE: We need Pos for ProjPos and Radial Fog
		// I flag VERTEX_FORMAT_COMPRESSED so it can be used on Models
		// ( Although we probably don't care since Normal isn't required for anything )
		// NOW, I do ask for *one* TexCoord. A 'too-small' Vertex Format is a Thing.
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// None

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		// We use the Brush VS since there is nothing different compared to what we have to do
		DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);
		SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_black_ps30);
		SET_STATIC_PIXEL_SHADER(lux_black_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// None
	
		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		// Always Opaque
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(true);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(true);
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);
	
		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
	
		DECLARE_DYNAMIC_PIXEL_SHADER(lux_black_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_black_ps30);
	}

	Draw();
}
END_SHADER