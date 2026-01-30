//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_model_simplified_vs30.inc"
#include "lux_brush_vs30.inc"
#include "lux_decalmodulate_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_DecalModulate, LUX_DecalModulate)
#endif

#ifdef REPLACE_DECALMODULATE
DEFINE_FALLBACK_SHADER(DecalModulate, LUX_DecalModulate)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_DecalModulate, "A Shader used for Decals, usually simulates Surface Damages.")
SHADER_INFO_GEOMETRY	("Decals. So this must be applied on-top of other Geometry.")
SHADER_INFO_USAGE		("Self-Explanatory.")
SHADER_INFO_LIMITATIONS	("This Shader REALLY does not like Fog. It Also cannot have Lighting or use BumpMaps.")
SHADER_INFO_PERFORMANCE	("Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC DecalModulate Shader Page: https://developer.valvesoftware.com/wiki/Decals#DecalModulate")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_DetailTextureParameters()
	Declare_MiscParameters()
	SHADER_PARAM(ModulateBias, SHADER_PARAM_TYPE_FLOAT ,"", "Biases the $BaseTexture's RGB Values Up (positive Values) or Down (negative Values).\nIntended for 8 Bit Textures that can't have a perfect 0.5f Value.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	DefaultFloat(DetailBlendFactor, 1.0f);	// Default Value is supposed to be 1.0f
	DefaultFloat(DetailScale, 4.0f);		// Default Value is supposed to be 4.0f
	
	// ASW Code does this so we do it too.
	SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// No projected Textures on Modulated Decals ( they will adapt Brightness from the Surface behind them )
	SetBool(ReceiveProjectedTextures, false);
}

SHADER_FALLBACK
{
#ifndef REPLACE_DECALMODULATE
	if (lux_oldshaders.GetBool())
		return "DecalModulate";
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
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadTexture(Detail);
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasDetailTexture = IsTextureLoaded(Detail);

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		FogToGrey();

		// This is a Decal, so avoid z-fights by offsetting
		pShaderShadow->EnablePolyOffset(SHADER_POLYOFFSET_DECAL);

		// We don't want to write to Dst. Alpha.
		pShaderShadow->EnableAlphaWrites(false);

		// We don't write linear Values ( Why? )
		pShaderShadow->EnableSRGBWrite(false);

		// We enable AlphaTest **AND** Blending
		// This doesn't make any Sense!!
		pShaderShadow->EnableAlphaTest(true);
		pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GREATER, 0.0f);

		// Mod2X Blending Method ( Dst*Src + Src*Dst )
		pShaderShadow->EnableBlending(true);
		pShaderShadow->BlendFunc(SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR);

		// Could just use EnableAlphaBlending() here ^ Does the same Thing
		pShaderShadow->EnableDepthWrites(false);

		// Stock-Consistency: Fog needs stay exactly middle grey
		// NOTE: We don't actually use the FogColor
		pShaderShadow->DisableFogGammaCorrection(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL;

		// This happens in ASW but not on the TF2SDK
		#ifndef TF2SDK
		bool bHasVertexAlpha = HasFlag(MATERIAL_VAR_VERTEXCOLOR) && HasFlag(MATERIAL_VAR_VERTEXALPHA);

		// Enables Vertex Color
		if (bHasVertexAlpha)
			nFlags |= VERTEX_COLOR;
		#else
		bool bHasVertexAlpha = false;
		#endif

		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture
		// "SRGB conversions hose the blend on some hardware, so keep everything in gamma space." -ASW Code
		EnableSampler(SAMPLER_BASETEXTURE, false);
		
		// s4 - $Detail. Blendmode 0 is always Linear.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, false);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, bHasDetailTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexAlpha);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0);
		SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_decalmodulate_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(VERTEXALPHA, bHasVertexAlpha);
		SET_STATIC_PIXEL_SHADER(lux_decalmodulate_ps30);
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
		BindTexture(bHasBaseTexture, SAMPLER_BASETEXTURE, BaseTexture, Frame, TEXTURE_GREY);

		// s4 - $Detail
		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// VS c225, c226 - $DetailTextureTransform
		if(bHasDetailTexture)
		{
			if (HasTransform(bHasDetailTexture, DetailTextureTransform))
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, DetailTextureTransform, DetailScale);
			else if (bHasDetailTexture)
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform, DetailScale);
		}

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c26 - Camera Pos
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c27 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c32 - $ModulateBias
		// The Rest is a bit empty but that's ok.
		float4 f4ModulationBias = 0.0f;
		f4ModulationBias.x = GetFloat(ModulateBias);
		f4ModulationBias.y = GetFloat(DetailBlendFactor);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_032, f4ModulationBias);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// NOTE: We don't *ever* want to write anything to Alpha
		// Alpha is needed for transparency!!
		// In case of Water and DepthToDestAlpha, we assume the Information already on the RT is correct.
		// Doing so solves the underwater-DecalModulate rendering Issue ( that's been plaguing TF2 forever )
		if(true)
		{
			// When this bool is true we are underwater, and don't want to do regular Fog.
			// Not true for cheap Water though, see .fxc for more Information.
			bool bHeightFog = false;
#ifdef TF2SDK
			bHeightFog = pShaderAPI->GetPixelFogCombo1(true) == 1;
#else
			bHeightFog = pShaderAPI->GetPixelFogCombo() == 1;
#endif
			// Never try writing Depth to DestAlpha, disabled on the Shader also
			BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false) || bHeightFog;
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = false;
		}

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==========================================================================//
		// Set Dynamic Shaders.
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_decalmodulate_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_decalmodulate_ps30);
	}

	Draw();
}
END_SHADER