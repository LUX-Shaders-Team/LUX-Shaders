//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_screenspace_vs30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Screenspace_General, LUX_Screenspace_General)
#endif

#ifdef REPLACE_SCREENSPACE_GENERAL
DEFINE_FALLBACK_SHADER(Screenspace_General, LUX_Screenspace_General)
#endif

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Screenspace_General, "A shader used for loading custom Screenspace Shaders." LUX_DEFAULT_DESCRIPTION)
SHADER_INFO_GEOMETRY	("Intended for Screenspace Materials.")
SHADER_INFO_USAGE		("Make a Shader and call the .vcs file inside the Material using the \"$PixShader\" Parameter.")
SHADER_INFO_LIMITATIONS	("NOT intended for other Geometry!! Use LUX_Custom instead!")
SHADER_INFO_PERFORMANCE	("Variable.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Screenspace_General Shader Page : https://developer.valvesoftware.com/wiki/Screenspace_General")
SHADER_INFO_D3D			("VS should always be SM3.0, but the PS could be either SM2.0 or SM3.0 depending on what is used as the $PixShader.")

BEGIN_SHADER_PARAMS
	SHADER_PARAM(PixShader,				SHADER_PARAM_TYPE_STRING,	"",		"Set a specific Pixel Shader .vcs File, do not insert the Filetype.")
	SHADER_PARAM(PixelShaderName,		SHADER_PARAM_TYPE_STRING,	"",		"(INTERNAL PARAMETER), dont use, carries the real Name used by the Shader.")
	SHADER_PARAM(Disable_Color_Writes,	SHADER_PARAM_TYPE_BOOL,		"",		"Stops the Shader from writing Color Data.")
	SHADER_PARAM(LinearRead_BaseTexture, SHADER_PARAM_TYPE_BOOL,	"",		"Disables sRGB Conversions for s0.")
	SHADER_PARAM(LinearRead_Texture1,	SHADER_PARAM_TYPE_BOOL,		"",		"Disables sRGB Conversions for s1.")
	SHADER_PARAM(LinearRead_Texture2,	SHADER_PARAM_TYPE_BOOL,		"",		"Disables sRGB Conversions for s2.")
	SHADER_PARAM(LinearRead_Texture3,	SHADER_PARAM_TYPE_BOOL,		"",		"Disables sRGB Conversions for s3.")
	SHADER_PARAM(LinearWrite,			SHADER_PARAM_TYPE_BOOL,		"",		"Setting this disables Gamma Conversions.")
	SHADER_PARAM(C0_X,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c0's x Component.")
	SHADER_PARAM(C0_Y,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c0's y Component.")
	SHADER_PARAM(C0_Z,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c0's z Component.")
	SHADER_PARAM(C0_W,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c0's w Component.")
	SHADER_PARAM(C1_X,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c1's x Component.")
	SHADER_PARAM(C1_Y,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c1's y Component.")
	SHADER_PARAM(C1_Z,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c1's z Component.")
	SHADER_PARAM(C1_W,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c1's w Component.")
	SHADER_PARAM(C2_X,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c2's x Component.")
	SHADER_PARAM(C2_Y,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c2's y Component.")
	SHADER_PARAM(C2_Z,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c2's z Component.")
	SHADER_PARAM(C2_W,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c2's w Component.")
	SHADER_PARAM(C3_X,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c3's x Component.")
	SHADER_PARAM(C3_Y,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c3's y Component.")
	SHADER_PARAM(C3_Z,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c3's z Component.")
	SHADER_PARAM(C3_W,					SHADER_PARAM_TYPE_FLOAT,	"",		"Sets PixelShader Constant c3's w Component.")
	SHADER_PARAM(Texture1,				SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGBA] Texture that will be on s1.")
	SHADER_PARAM(Texture2,				SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGBA] Texture that will be on s2.")
	SHADER_PARAM(Texture3,				SHADER_PARAM_TYPE_TEXTURE,	"",		"[RGBA] Texture that will be on s3.")

	// This is a weird one
	// We had $ALPHABLEND as a Parameter before but it wasn't implemented
	// TF2SDK has Alpha_Blend.. I just changed the Name but I thought this is worth mentioning.
	SHADER_PARAM(Alpha_Blend,			SHADER_PARAM_TYPE_BOOL,		"",		"Whether or not to use Alpha Blending ( $Translucent but different Name ).")

	// Needed for some ancient Materials that use plattform VCS Files like appchooser360movie_ps20b
	SHADER_PARAM(X360AppChooser, SHADER_PARAM_TYPE_BOOL, "", "Ancient Parameter only kept for backwards compatibility.\nEnables Vertex Colors and MVP Matrix.")

	// TF2SDK Parameters
	// These used to be Integer Params
	SHADER_PARAM(CopyAlpha, SHADER_PARAM_TYPE_BOOL, "", "Disables Blending Modes so Src Alpha reaches Dest Alpha. ( allows Alpha Output )")
	SHADER_PARAM(Alpha_Blend_Color_Overlay, SHADER_PARAM_TYPE_BOOL, "", "")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// There is no Param Init for Stock Screenspace_General
}

SHADER_FALLBACK
{
#ifndef REPLACE_SCREENSPACE_GENERAL
	if (lux_oldshaders.GetBool())
		return "Screenspace_General";
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
	SetFlag2(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE);

	const char *ccPixelShader = GetString(PixShader);
	size_t iLength = Q_strlen(ccPixelShader);

	// HACKHACK: Stock ASW+ Mats use blurgaussian_3x3_ps20b but I'm not going to have this as a File, use the lux_ one instead
	// NOTE: This is a Shader from Alien Swarm, Mapbase uses it for something so we have to account for the lack of this Shader
	if(V_stricmp(ccPixelShader, "blurgaussian_3x3_ps20b") == 0)
	{
		SetString(PixelShaderName, "lux_blurgaussian_ps30");
	}
	// Stock-Consistency:
	// Detect if we are trying to load a ps20 Shader. If yes, make it ps20b
	else if ((iLength > 5) && (Q_stricmp(&ccPixelShader[iLength - 5], "_ps20") == 0))
	{
		//replace it with the ps20b shader
		char *szNewName = (char *)stackalloc(sizeof(char) * (iLength + 2));
		memcpy(szNewName, ccPixelShader, sizeof(char) * iLength); // Linux Incompatible memcpy function?
		szNewName[iLength] = 'b';
		szNewName[iLength + 1] = '\0';

		SetString(PixelShaderName, szNewName);
	}
	else
	{
		// ps20b, ps30...
		SetString(PixelShaderName, ccPixelShader);
	}

	LoadTexture(BaseTexture);
	LoadTexture(Texture1);
	LoadTexture(Texture2);
	LoadTexture(Texture3);
}

SHADER_DRAW
{
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasTexture1 = IsTextureLoaded(Texture1);
	bool bHasTexture2 = IsTextureLoaded(Texture2);
	bool bHasTexture3 = IsTextureLoaded(Texture3);

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
		pShaderShadow->EnableDepthTest(false); // No Depth Testing either

		if (GetBool(Disable_Color_Writes))
			pShaderShadow->EnableColorWrites(false);

		pShaderShadow->EnableSRGBWrite(!GetBool(LinearWrite));

		if (HasFlag(MATERIAL_VAR_ADDITIVE))
			EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);

		// NOTE: The Things below should disable AlphaWrites !!!
		// But they don't !!!
		// Is this intentional? I doubt it..!!!

		// From TF2SDK:
		// "Used for adding L4D-style halos"
		if (GetBool(Alpha_Blend_Color_Overlay))
			EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

		// From TF2SDK:
		// "Used for adding L4D-style halos"
		// This is probably different from $Translucent for Render-order Reasons
		if (GetBool(Alpha_Blend))
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

		// Backwards Compat.
		bool bMovieBackwardsCompat = GetBool(X360AppChooser);
		if (bMovieBackwardsCompat)
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

		// Stock-Consistency: Whacky AlphaTesting Behaviour
		if (GetBool(CopyAlpha))
		{
			pShaderShadow->EnableBlending(false);
			pShaderShadow->EnableAlphaTest(true);
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_ALWAYS, 0.0f);
		}
		else
		{
			pShaderShadow->EnableAlphaTest(true);
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GREATER, 0.0);
		}

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		unsigned int nFlags = VERTEX_POSITION;
		int nTexCoords = 1;
		int nUserDataSize = 0;

		// Backwards Compat.
		if(bMovieBackwardsCompat)
			nFlags |= VERTEX_COLOR;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//
		if (bHasBaseTexture)
		{
			ITexture *pBaseTexture = GetTexture(BaseTexture);
			if (IsHDRTexture(pBaseTexture))
				EnableSampler(SHADER_SAMPLER0, false);
			else
				EnableSampler(SHADER_SAMPLER0, !GetBool(LinearRead_BaseTexture));
		}

		if (bHasTexture1)
		{
			ITexture *pTexture1 = GetTexture(Texture1);
			if (IsHDRTexture(pTexture1))
				EnableSampler(SHADER_SAMPLER1, false);
			else
				EnableSampler(SHADER_SAMPLER1, !GetBool(LinearRead_Texture1));
		}

		if (bHasTexture2)
		{
			ITexture *pTexture2 = GetTexture(Texture2);
			if (IsHDRTexture(pTexture2))
				EnableSampler(SHADER_SAMPLER2, false);
			else
				EnableSampler(SHADER_SAMPLER2, !GetBool(LinearRead_Texture2));
		}

		if (bHasTexture3)
		{
			ITexture *pTexture3 = GetTexture(Texture3);
			if (IsHDRTexture(pTexture3))
				EnableSampler(SHADER_SAMPLER3, false);
			else
				EnableSampler(SHADER_SAMPLER3, !GetBool(LinearRead_Texture3));
		}

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(X360APPCHOOSER, bMovieBackwardsCompat);
		SET_STATIC_VERTEX_SHADER(lux_screenspace_vs30);

		pShaderShadow->SetPixelShader(GetString(PixelShaderName), 0);
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
		BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, -1);
		BindTexture(bHasTexture1, SHADER_SAMPLER1, Texture1, -1);
		BindTexture(bHasTexture2, SHADER_SAMPLER2, Texture2, -1);
		BindTexture(bHasTexture3, SHADER_SAMPLER3, Texture3, -1);

		// Array of all the Parameters
		float4 c0 = { GetFloat(C0_X), GetFloat(C0_Y), GetFloat(C0_Z), GetFloat(C0_W) };
		float4 c1 = { GetFloat(C1_X), GetFloat(C1_Y), GetFloat(C1_Z), GetFloat(C1_W) };
		float4 c2 = { GetFloat(C2_X), GetFloat(C2_Y), GetFloat(C2_Z), GetFloat(C2_W) };
		float4 c3 = { GetFloat(C3_X), GetFloat(C3_Y), GetFloat(C3_Z), GetFloat(C3_W) };
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, c0);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_001, c1);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_002, c2);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_003, c3);

		// From TF2SDK:
		// "Use 4-7 for Pixel Size"
		// I made it cast to float for Texture Res
		// Since that's handed to us as an Integer, sometimes the Compiler will complain about it.
		if (bHasBaseTexture)
		{
			ITexture* pTex0 = GetTexture(BaseTexture);
			float4 c4 = 0.0f;
			c4.x = 1.0f / (float)pTex0->GetActualWidth();
			c4.y = 1.0f / (float)pTex0->GetActualHeight();
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_004, c4);
		}

		if (bHasTexture1)
		{
			ITexture* pTex1 = GetTexture(Texture1);
			float4 c5 = 0.0f;
			c5.x = 1.0f / (float)pTex1->GetActualWidth();
			c5.y = 1.0f / (float)pTex1->GetActualHeight();
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_005, c5);
		}

		if (bHasTexture2)
		{
			ITexture* pTex2 = GetTexture(Texture2);
			float4 c6 = 0.0f;
			c6.x = 1.0f / (float)pTex2->GetActualWidth();
			c6.y = 1.0f / (float)pTex2->GetActualHeight();
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_006, c6);
		}

		if (bHasTexture3)
		{
			ITexture* pTex3 = GetTexture(Texture3);
			float4 c7 = 0.0f;
			c7.x = 1.0f / (float)pTex3->GetActualWidth();
			c7.y = 1.0f / (float)pTex3->GetActualHeight();
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_007, c7);
		}
			
		// Stock-Consistency: This must be on c10
		SetPixelShaderCameraPosition(REGISTER_FLOAT_010);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_screenspace_vs30);

		// No Combos
		pShaderAPI->SetPixelShaderIndex(0);
	}

	Draw();
}
END_SHADER