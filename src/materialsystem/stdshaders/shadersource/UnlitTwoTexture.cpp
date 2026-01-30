//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Cloak Functions, because for some reason this shader has cloak
#include "renderpasses/Cloak.h"

// Includes for Shaderfiles...
#include "lux_combinetextures_vs30.inc"
#include "lux_combinetextures_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_UnlitTwoTexture, LUX_UnlitCombineTextures)
DEFINE_FALLBACK_SHADER(SDK_UnlitTwoTexture_DX9, LUX_UnlitCombineTextures)
#endif

#ifdef REPLACE_UNLITTWOTEXTURE
DEFINE_FALLBACK_SHADER(UnlitTwoTexture, LUX_UnlitCombineTextures)
DEFINE_FALLBACK_SHADER(UnlitTwoTexture_DX9, LUX_UnlitCombineTextures)
#endif

// Wanna multiply 13 Textures together for some odd Reason...? Here you go...
DEFINE_FALLBACK_SHADER(LUX_UnlitTwoTexture, LUX_UnlitCombineTextures)

#define MAX_TEXTURES 13
//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_UnlitCombineTextures, "Unlit Shader for multiplying independently transformed Textures together." LUX_DEFAULT_DESCRIPTION)
SHADER_INFO_GEOMETRY	("Brushes, Displacements, Overlays, Models. Pretty much all Geometry Types.\n")
SHADER_INFO_USAGE		("Two or more Textures must be defined in the VMT. $BaseTexture and $Texture1 to $Texture14.")
SHADER_INFO_LIMITATIONS ("This Shader doesn't really support much else, it's a very simple Shader.\n"
						 "The most complicated Feature here is probably Translucency, since it can combine many Textures together through multiplication")
SHADER_INFO_PERFORMANCE	("Very very cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.\n")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC)
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	// Don't change the Order of these, otherwise our Array-Math will break!

	// Texture 1 is $BaseTexture
	SHADER_PARAM(Texture2,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture3,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture4,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture5,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture6,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture7,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture8,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture9,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture10,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture11,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture12,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")
	SHADER_PARAM(Texture13,						SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Defines an Albedo Texture.")

	SHADER_PARAM(Frame2,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture2")
	SHADER_PARAM(Frame3,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture3")
	SHADER_PARAM(Frame4,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture4")
	SHADER_PARAM(Frame5,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture5")
	SHADER_PARAM(Frame6,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture6")
	SHADER_PARAM(Frame7,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture7")
	SHADER_PARAM(Frame8,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture8")
	SHADER_PARAM(Frame9,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture9")
	SHADER_PARAM(Frame10,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture10")
	SHADER_PARAM(Frame11,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture11")
	SHADER_PARAM(Frame12,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture12")
	SHADER_PARAM(Frame13,						SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Texture13")

	SHADER_PARAM(Texture2Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture2. Must include all Values!")
	SHADER_PARAM(Texture3Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture3. Must include all Values!")
	SHADER_PARAM(Texture4Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture4. Must include all Values!")
	SHADER_PARAM(Texture5Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture5. Must include all Values!")
	SHADER_PARAM(Texture6Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture6. Must include all Values!")
	SHADER_PARAM(Texture7Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture7. Must include all Values!")
	SHADER_PARAM(Texture8Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture8. Must include all Values!")
	SHADER_PARAM(Texture9Transform,				SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture9. Must include all Values!")
	SHADER_PARAM(Texture10Transform,			SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture10. Must include all Values!")
	SHADER_PARAM(Texture11Transform,			SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture11. Must include all Values!")
	SHADER_PARAM(Texture12Transform,			SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture12. Must include all Values!")
	SHADER_PARAM(Texture13Transform,			SHADER_PARAM_TYPE_MATRIX, "", "Transforms $Texture13. Must include all Values!")

	// LUX Addition, from UnlitGeneric
	SHADER_PARAM(LinearWrite,					SHADER_PARAM_TYPE_BOOL,	"",	"Disables SRGB Conversions of Shader Results.")

	// How many Textures are actually used
	SHADER_PARAM(TextureCount,					SHADER_PARAM_TYPE_INTEGER, "", "(INTERNAL PARAMETER) How many textures are actually used by the shader.")
	
	// For some Reason this shader has a Cloak Pass
	Declare_CloakParameters()
END_SHADER_PARAMS

void UTT_SetupCloakVars(Cloak_Vars_t& CloakVars)
{
	CloakVars.InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	CloakVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
	CloakVars.Bump.InitVars(-1);
}

// IMPORTANT: Virtual Function Override.
bool NeedsPowerOfTwoFrameBufferTexture(IMaterialVar** params, bool bCheckSpecificToThisFrame) const override
{
	// Need to use params directly here, otherwise we corrupt m_ppParams for Draw()
	if ( params[CloakPassEnabled]->GetIntValue() )
	{
		float flCloakFactor = params[CloakFactor]->GetFloatValue();
		if ( !bCheckSpecificToThisFrame || (flCloakFactor > 0.0f && flCloakFactor < 1.0f) )
			return true;
	}

	return params[Flags_Defined2]->GetIntValue() & MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE;
}

// IMPORTANT: Virtual Function Override.
bool IsTranslucent(IMaterialVar** params) const override
{
	// Need to use params directly here, otherwise we corrupt m_ppParams for Draw()
	if ( params[CloakPassEnabled]->GetIntValue() )
	{
		float flCloakFactor = params[CloakFactor]->GetFloatValue();
		if ( flCloakFactor > 0.0f && flCloakFactor < 1.0f )
			return true;
	}

	return params[Flags]->GetIntValue() & MATERIAL_VAR_TRANSLUCENT;
}

SHADER_INIT_PARAMS()
{
	Cloak_Vars_t CloakVars;
	UTT_SetupCloakVars(CloakVars);
	CloakBlend_Init_Params(this, CloakVars);

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	// We have to cycle through the Textures to see how many we actually have..
	// I'm not going to make *~13* separate Static Combos for this!! It's going to be one big Combo
	int nUsedTextureSlots = IsDefined(BaseTexture) ? 1 : 0;

	// This Shader supports MAX_TEXTURES-1 Total.
	// $BaseTexture + $Texture1 - $TextureX
	// We just always have s0, and then we only need to check the other n Textures
	// To do that, I abuse the Fact Parameters are an Array.
	for (int n = 0; n < (MAX_TEXTURES - 1); n++)
	{
		if (IsDefined(Texture2 + n))
		{
			// Increase counter
			nUsedTextureSlots++;
		}
	}

	// The final count of Textures
	SetInt(TextureCount, nUsedTextureSlots);
}

SHADER_FALLBACK
{
#ifdef REPLACE_UNLITTWOTEXTURE
	if (lux_oldshaders.GetBool())
		return "UnlitTwoTexture";
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
	Cloak_Vars_t CloakVars;
	UTT_SetupCloakVars(CloakVars);
	CloakBlend_Shader_Init(this, CloakVars);

	// This doesn't have to be impressively fast
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);

	// This should be a Loop using MAX_TEXTURES-1
	LoadTexture(Texture2, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture3, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture4, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture5, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture6, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture7, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture8, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture9, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture10, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture11, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture12, TEXTUREFLAGS_SRGB);
	LoadTexture(Texture13, TEXTUREFLAGS_SRGB);
}

// NOTE: DrawFunction is separate here because we Cloak
// It's just easier to do this way.
void DrawCombineTextures(IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, CBasePerMaterialContextData** pContextDataPtr)
{
	int nLoadedTextures = GetInt(TextureCount);

	// NOTE: We ignore every other Texture here ON PURPOSE
	// If you set $Translucent you better have Alpha!!!
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

		// Everything Transparency is packed into this Function
		EnableTransparency(nBlendType);
	
		// Need Fog. Apparently Left 4 Dead 2 Materials have some Issues if we don't.
		DefaultFog();
	
		// We always need this
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);
	
		// By default we write linear Values and need them converted to sRGB
		bool bSRGBWrite = !GetBool(LinearWrite); // Stock Consistency
		pShaderShadow->EnableSRGBWrite(bSRGBWrite);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Stock-Consistency. Asking for these 3 specifically.
		// Does going smaller cause the too-thin Vertex Format Issue? 
		// Don't really need Normals for anything.
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;

		// Stock Shader doesn't actually support this so neither do I
		/*
		if (HasFlag(MATERIAL_VAR_VERTEXCOLOR))
		{
			// Enables Vertex Color
			flags |= VERTEX_COLOR;
		}
		*/

		int nTexCoords = 1;
		int nUserDataSize = 0;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, 0, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// Simple. Always sRGB!
		for (int n = 0; n < nLoadedTextures; n++)
		{
			EnableSampler((Sampler_t)n, true);
		}

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_combinetextures_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nLoadedTextures);
		SET_STATIC_VERTEX_SHADER(lux_combinetextures_vs30);		

		DECLARE_STATIC_PIXEL_SHADER(lux_combinetextures_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(TEXTURES, nLoadedTextures);
		SET_STATIC_PIXEL_SHADER(lux_combinetextures_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		int nCurTexture = 0;

		// $BaseTexture is *always* s0
		if (IsDefined(BaseTexture))
		{
			BindTexture((Sampler_t)nCurTexture, BaseTexture, Frame);

			// Do TexCoord while we are already riding the looping coaster
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01 + (nCurTexture * 2), BaseTextureTransform);

			// Keep Track
			nCurTexture++;
		}
			
		// s1 to s13 we can handle like this
		for (int n = 0; n < (MAX_TEXTURES - 1); n++)
		{
			if (n == nLoadedTextures)
				break;

			if (IsDefined(Texture2 + n))
			{
				BindTexture((Sampler_t)nCurTexture, Texture2 + n, Frame2 + n);

				// Do TexCoord while we are already riding the looping coaster
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01 + (nCurTexture * 2), Texture2Transform + n);

				nCurTexture++;
			}
		}

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// c0 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		f4Tint.rgb = GammaToLinearTint(f4Tint.rgb);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4Tint);

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);

		// Always!
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_combinetextures_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_combinetextures_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_combinetextures_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_combinetextures_ps30);
	}

	Draw();
}

SHADER_DRAW
{
	bool bDrawBasePass = true;

	Cloak_Vars_t CloakVars;
	UTT_SetupCloakVars(CloakVars);
	bool bCloakEnabled = GetBool(CloakPassEnabled);

	if (bCloakEnabled && (pShaderShadow == NULL))
	{
		if (CloakBlend_IsOpaque(this, params, CloakVars))
			bDrawBasePass = false;
	}

	// If we know ahead of time because of the Spy Cloak, that we don't really have a base
	// Don't bother to even render it
	if (bDrawBasePass)
	{
		DrawCombineTextures(pShaderShadow, pShaderAPI, pContextDataPtr);
	}
	else
	{
		// We are cloaking, so stop doing the base pass
		// Otherwise the enemy team is going to cause a malfunction in our spy
		Draw(false);
	}

	// Now that we determined whether or not to draw the base
	// Draw spycloak if necessary
	if (bCloakEnabled)
	{
		float f1CloakFactor = GetFloat(CloakFactor);

		// If we are on snapshot we **really** have to set up the cloak shaders
		// Otherwise we will try to bind dynamic shaders to non-existant static ones
		// Also if we are at a cloakfactor of 0 there is no point in drawing the cloak
		if (pShaderShadow || ((f1CloakFactor > 0.0f) && (f1CloakFactor < 1.0f)))
			CloakBlend_Shader_Draw(this, pShaderShadow, pShaderAPI, CloakVars);
		else
			Draw(false);
	}
}
END_SHADER