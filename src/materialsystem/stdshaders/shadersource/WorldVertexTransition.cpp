//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_worldvertextransition_simple_ps30.inc"
#include "lux_worldvertextransition_bump_ps30.inc"
#include "lux_worldvertextransition_phong_ps30.inc"
#include "lux_displacement_vs30.inc"
#include "lux_worldvertextransition_flashlight_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_WorldVertexTransition,		LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(SDK_WorldVertexTransition_DX9,	LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(SDK_WorldVertexTransition_DX8,	LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(SDK_WorldVertexTransition_DX6,	LUX_WorldVertexTransition)
#endif

#ifdef REPLACE_WORLDVERTEXTRANSITION
DEFINE_FALLBACK_SHADER(WorldVertexTransition,		LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(WorldVertexTransition_DX9,	LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(WorldVertexTransition_DX8,	LUX_WorldVertexTransition)
DEFINE_FALLBACK_SHADER(WorldVertexTransition_DX6,	LUX_WorldVertexTransition)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class WorldVertexTransitionContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	// Everything related to constants

	WorldVertexTransitionContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_WorldVertexTransition, "A Shader functionally similar to LightmappedGeneric, primarily used to perform blending Operations between two Textures on a Displacement.")
SHADER_INFO_GEOMETRY	("Displacements.")
SHADER_INFO_USAGE		("In order to use the blending Feature, Materials must be used on a Displacement Surface with Painted Alpha.")
SHADER_INFO_LIMITATIONS	("SelfIllumMask may not work correctly due to the limited Sampler count of DX9.\n\n"

						 "Due to the Alpha Channel being used by too many Things a new Parameter called $BlendModulateTransparency was introduced,\n"
						 "It allows using the remaining two Channels on a $BlendModulateTexture for two separate (blended) Opacity Maps.\n\n"

						 "The Stock Variant of this Shader ( or what is effectively this Shader ) came with a lot of baked in Limitations that had to be reproduced.\n"
						 "This means that there are various quirks that can't all be listed here.\n"
						 "For Example, specifying $BumpMap but not $BumpMap2 will cause the Shader to use $BumpMap for both Sides.\n"
						 "However using $BumpMap2 without a $BumpMap will give you the logical Behaviour of only $BumpMap2 on the second Side.")
SHADER_INFO_PERFORMANCE	("VERY Performance intense. This Shader has 2x the Amount of Texture Samples compared to other Shaders.\n"
						 "Especially Phong should be used *very* sparingly. There is a LOT of Code that needs to be run.\n\n"
						 "Due to how projected Textures render ( Multipass ), this becomes significantly more draining in Scenes with Projected Textures.\n"
						 "The Flashlight is a projected Texture, so try to avoid dark scenes with additional Projected Textures.\n"
						 "The most expensive Feature Combinations should be: Parallax Corrected Cubemaps, $EnvMapMask, $Phong, $Detail and $BlendModulateTexture.")
SHADER_INFO_FALLBACK	("Using $Seamless_Scale fallbacks to LUX_Triplanar_Displacement.\n"
						 "A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC WorldVertexTransition Shader Page: https://developer.valvesoftware.com/wiki/WorldVertexTransition")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	// BaseTexture related
	Declare_DisplacementBase()
	Declare_MiscParameters()
	SHADER_PARAM(BaseTexture2Tint, SHADER_PARAM_TYPE_COLOR, "", "Tints $BaseTexture2 specifically.\nWhen used, $BaseTexture2 will not be affected by $Color.\n However it will be affected by $Color2 unless $AllowDiffuseModulation or $NoTint is used.")

	// Bump
	Declare_NormalTextureParameters()
	Declare_DisplacementBump()
	Declare_NoDiffuseBumpLighting()

	// Detail
	Declare_DetailTextureParameters()
	Declare_Detail2TextureParameters()

	// Blending
	Declare_DisplacementBlend()
	SHADER_PARAM(BlendModulateTransparency, SHADER_PARAM_TYPE_BOOL, "", "Use Blue and Alpha Channel of the Blendmodulate Texture for Opacity Results.")
	SHADER_PARAM(FlipBlendFactor, SHADER_PARAM_TYPE_BOOL, "", "BlendFactor is automatically flipped when using Hammer, this allows you to flip it whenever you want, even at runtime.")

	// EnvMap
	Declare_EnvironmentMapParameters()
	SHADER_PARAM(BaseTextureNoEnvMap, SHADER_PARAM_TYPE_BOOL, "", "Stops the first Texture from receiving EnvMaps. ( Sets the tint to 0 )")
	SHADER_PARAM(BaseTexture2NoEnvMap, SHADER_PARAM_TYPE_BOOL, "", "Stops the first Texture from receiving EnvMaps. ( Sets the tint to 0 )")
	Declare_EnvMapMaskParameters()
	Declare_EnvMapMask2Parameters()
	Declare_ParallaxCorrectionParameters()
	
	// SelfIllum
	Declare_SelfIlluminationParameters()
	SHADER_PARAM(SelfIllumMask2, SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Acts as a second seperate Mask instead of using an Alpha Channel.")
	SHADER_PARAM(SelfIllumMaskFrame2, SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $SelfIllumMask2.")
	SHADER_PARAM(SelfIllumTint2, SHADER_PARAM_TYPE_COLOR, "", "Tint for the second Selfillum Texture.")
	Declare_SelfIllumTextureParameters()

	// Displacement Phong
	Declare_PhongParameters()
	Declare_DisplacementPhong()

	// Misc..
	SHADER_PARAM(LinearWrite, SHADER_PARAM_TYPE_BOOL, "", "Disables SRGB conversion of Shader Results.")

	// Fallback
	Declare_SeamlessParameters()
	SHADER_PARAM(DistanceAlpha, SHADER_PARAM_TYPE_BOOL, "", "Cheap edge filtering technique for raster images, great for UI elements, foliage, chain link fences, grates, and more. (Note: $DistanceAlpha is not implemented in WVT, using this Parameter a causes fallback to the DistanAlpha shader).")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Only try to undefine if defined...
	if (mat_disable_lightwarp.GetBool() && IsDefined(LightWarpTexture))
	{
		SetUndefined(LightWarpTexture);
	}

	// Scale, Bias Exponent
	DefaultFloat3(EnvMapFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);
	DefaultFloat3(SelfIllumFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);

	// Detail related
	DefaultFloat(DetailBlendFactor, 1.0f); // Default Value is supposed to be 1.0f
	DefaultFloat(DetailScale, 4.0f); // Default Value is supposed to be 4.0f
	DefaultFloat(DetailScale2, 4.0f); // Default Value is supposed to be 4.0f

	// No BaseTexture? None of these.
	if (!IsDefined(BaseTexture) && !IsDefined(BaseTexture2))
	{
		ClearFlag(MATERIAL_VAR_SELFILLUM);
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// Always want regular Lightmaps unless BumpMaps are used.
	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
	if (g_pConfig->UseBumpmapping() && (IsDefined(BumpMap) || IsDefined(BumpMap2)))
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);


	// Force Enable
	// This must have a $BumpMap since Phong only works with Radiosity Normal Mapping.
	if (lux_lightmapped_phong_force.GetBool() && IsDefined(BumpMap))
	{
		SetBool(Phong, 1);
	}

	if (GetBool(Phong))
	{
		// ShiroDkxtro2, 31.07.2025: Compiletimes were unacceptable ( 13 seconds )
		// So I removed SelfIllum from the Phong Shader. Disable all of them.
		// It is acceptable 8 seconds now
		// This might sound kind of extreme, but you have to consider..
		// I'm compiling this on a Ryzen9 5950x.
		// 13 seconds might as well be a Minute or two for someone else.
		// Feel free to bring it back if you want, I suggest not to though
		// You can still do 'SelfIllum' with DetailBlendMode 5
		// I removed SelfIllum specifically because it seems like an unlikely thing to use.
		// It could have hit Detail2, PCC or XByBaseAlpha.. I can live with SelfIllum being gone
		// Kinda busted anyways. s15 is not very safe..
		ClearFlag(MATERIAL_VAR_SELFILLUM);
		SetUndefined(SelfIllumMask);
		SetUndefined(SelfIllumMask2);
		SetBool(SelfIllum_EnvMapMask_Alpha, false);

		// ShiroDkxtro2:
		// This is a new Feature to this Shader
		// So I'm not hacking in Support for Valve created Shenanigans.
		// Abide by these Rules and we won't have a Spaghetti.
		if (!(IsDefined(BumpMap) || IsDefined(BumpMap2)) && CVarDeveloper.GetInt() > 0)
		{
			if (IsDefined(BaseMapAlphaPhongMask) && GetBool(BaseMapAlphaPhongMask))
			{
				ShaderDebugMessage("uses $BaseMapAlphaPhongMask without a $BumpMap. This is not supported on LUX_LightmappedGeneric.\n");
			}
			else
			{
				ShaderDebugMessage("tries to use $Phong without a $BumpMap. This is not supported on LUX_LightmappedGeneric.\n");
			}
		}

		DefaultFloat(PhongBoost, 1.0f);
		DefaultFloat(PhongExponent, 5.0f);
		DefaultFloat(PhongExponent2, 5.0f);

		// PhongFresnelRanges need this or Fresnel will be 0.0f
		DefaultFloat3(PhongFresnelRanges, 0.0f, 0.5f, 1.0f);
		DefaultFloat3(PhongFresnelRanges2, 0.0f, 0.5f, 1.0f);

		// ShiroDkxtro2 Instruction Reduction :
		// On the Shader we'd do "$PhongExponentTexture.x * 149 + 1"
		// On SDK2013MP this would be "$PhongExponentTexture.x * $PhongExponentFactor + 1"
		// But we will do instead "$PhongExponentTexture.x * $PhongExponentFactor + $PhongExponent"
		// Without $PhongExponentTexture, this will just end up being "$PhongExponent" on the Shader
		//
		// Stock Consistency : Override to $PhongExponent when its anything other than 0
		//
		// We use DefaultFloat's as perhaps someone tries some whacky Stuff by combining these Parameters 
		if (IsDefined(PhongExponent) && GetFloat(PhongExponent) > 0.0f)
		{
			DefaultFloat(PhongExponentFactor, 0.0f);
		}
		else if (IsDefined(PhongExponentFactor) && GetFloat(PhongExponentFactor) > 0.0f)
		{
			DefaultFloat(PhongExponent, 1.0f);
		}
		else
		{
			// Default Value is... In SDK2013mp this was 0... that's too bad..
			// It replaces the *149 in the Shader, so that is what the default Value *should* be
			DefaultFloat(PhongExponentFactor, 149.0f);

			// Default Value is supposed to be 5.0f
			DefaultFloat(PhongExponent, 5.0f);
		}

		// Same thing for the Second Texture
		if (IsDefined(PhongExponent2) && GetFloat(PhongExponent2) > 0.0f)
		{
			DefaultFloat(PhongExponentFactor2, 0.0f);
		}
		else if (IsDefined(PhongExponentFactor2) && GetFloat(PhongExponentFactor2) > 0.0f)
		{
			DefaultFloat(PhongExponent2, 1.0f);
		}
		else
		{
			// Default Value is... In SDK2013mp this was 0... that's too bad..
			// It replaces the *149 in the Shader, so that is what the default Value *should* be
			DefaultFloat(PhongExponentFactor2, 149.0f);

			// Default Value is supposed to be 5.0f
			DefaultFloat(PhongExponent2, 5.0f);
		}
	}

	// If mat_specular 0, get rid of the EnvMap
	if (!g_pConfig->UseSpecular() && IsDefined(EnvMap))
	{
		SetUndefined(EnvMap);
		SetBool(EnvMapParallax, false);
	}

	// Tell the Shader to flip the EnvMap when set on AlphaEnvMapMask ( Consistency with Stock Shaders )
	if (!IsDefined(EnvMapMaskFlip) && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
	{
		SetBool(EnvMapMaskFlip, true);
	}
}

SHADER_FALLBACK
{
#ifndef REPLACE_LIGHTMAPPEDGENERIC
	if (lux_oldshaders.GetBool())
		return "WorldVertexTransition";
#endif

	if(GetFloat(Seamless_Scale) != 0.0f)
		return "LUX_Triplanar_Displacement";

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
	LoadTexture(BaseTexture2, TEXTUREFLAGS_SRGB);
	LoadBumpMap(BumpMap);
	LoadBumpMap(BumpMap2);

	// ~Stock-Consistency~
	// This is no longer done?
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap) && IsTextureLoaded(BumpMap))
	{
		bHasBumpMap = true;

		// If the User didn't specifically imply this is a SSBump, check using the flags
		if (!IsDefined(SSBump))
		{
			ITexture *pBumpMap = GetTexture(BumpMap);
			bool bIsSSBump = pBumpMap->GetFlags() & TEXTUREFLAGS_SSBUMP ? true : false;
			SetBool(SSBump, bIsSSBump);
		}
	}

	// Account for second Bumpmap
	if (IsDefined(BumpMap2) && IsTextureLoaded(BumpMap2))
	{
		bHasBumpMap = true;

		// If the User didn't specifically imply this is a SSBump, check using the flags
		if (!IsDefined(SSBump))
		{
			ITexture* pBumpMap2 = GetTexture(BumpMap2);
			bool bIsSSBump = pBumpMap2->GetFlags() & TEXTUREFLAGS_SSBUMP ? true : false;
			SetBool(SSBump, bIsSSBump);
		}
	}

	// Can only use these with BumpMapping
	// We require Radiosity Normal Mapping for it to work.
	if (bHasBumpMap && GetBool(Phong))
	{
		LoadTexture(PhongExponentTexture);
		LoadTexture(PhongExponentTexture2);
		
		// no second PhongWarpTexture, Shader is complicated enough as is
		// Blending in the Lighting Function is something I'm not looking forward to
		LoadTexture(PhongWarpTexture);
	}

	LoadTexture(BlendModulateTexture);

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	int nDetailLoadFlag = IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0;
	LoadTexture(Detail, nDetailLoadFlag);
	LoadTexture(Detail2, nDetailLoadFlag);

	// Stock-Consistency
	bool bDetail1 = IsDefined(Detail) && IsTextureLoaded(Detail);
	bool bDetail2 = IsDefined(Detail2) && IsTextureLoaded(Detail2);
	if (bDetail1 || bDetail2)
	{
		int nDetailParam = bDetail1 ? Detail : Detail2;
		if (GetTexture(nDetailParam)->GetFlags() & TEXTUREFLAGS_SSBUMP)
		{
			// Portal 2's Panel Texture uses $DetailBlendMode 10 with an SSBump, however official Stock Shaders don't allow this.
			// It would only work if not a SSBump. LUX Adds support for it, so consider that!
			if (bHasBumpMap)
				SetInt(DetailBlendMode, 10);
			else
				SetInt(DetailBlendMode, 11);
		}
	}

	LoadTexture(LightWarpTexture);

	// SelfIllum
	LoadTexture(SelfIllumMask);
	LoadTexture(SelfIllumMask2);
	LoadTexture(SelfIllumTexture, TEXTUREFLAGS_SRGB);


	LoadCubeMap(EnvMap);

	// This Block of if-Statements handles the Priority Chain of EnvMapMasks
	if (IsDefined(EnvMap))
	{
		if (IsDefined(EnvMapMask) || IsDefined(EnvMapMask2))
		{
			LoadTexture(EnvMapMask, 0);
			LoadTexture(EnvMapMask2, 0);

			// We already have an envmapmask now, so discard the others!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		else
		{
			// NormalMapAlphaEnvMapMask takes priority, I decided thats sensible because its the go to one
			if (HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
			{
				ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK); // If we use normal map alpha, don't use basetexture alpha.
			}
		}

		// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
		// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
		// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
		// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
		if(IsDefined(EnvMapParallaxOBB1))
			DefaultBool(EnvMapParallax, true);
	}

	// No AlphaTest and BlendTintByBaseAlpha when we use the Alpha for something else.
	// Stock-Shader ignored that the $EnvMapMask's Alpha can be used for $SelfIllum...
	// 
	// IMPORTANT: || HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK) is not checked here
	// flatnose_truck.vmt ( A L4D Material ) uses $BaseAlphaEnvMapMask with $BlendTintByBaseAlpha
	// and that somehow works over there, so I'm excluding it from the Check here.
	bool bAlternativeSelfIllum;
	bAlternativeSelfIllum = IsDefined(SelfIllumMask) || IsDefined(SelfIllumMask2);
	bAlternativeSelfIllum = bAlternativeSelfIllum || GetBool(SelfIllumFresnel) || GetBool(SelfIllum_EnvMapMask_Alpha);
	if (HasFlag(MATERIAL_VAR_SELFILLUM) && !bAlternativeSelfIllum)
	{
		ClearFlag(MATERIAL_VAR_ALPHATEST);
		SetBool(BlendTintByBaseAlpha, false);
	}
}

// Virtual Void Override for Context Data
WorldVertexTransitionContext* CreateMaterialContextData() override
{
	return new WorldVertexTransitionContext(this);
}

SHADER_DRAW
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<WorldVertexTransitionContext>(pContextDataPtr);
//		auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	// Flagstuff

	// NOTE: We already ensured conflicting Flags have been accounted for on ParamInit and ShaderInit

	// These don't have an Attachment to something else.
	bool bInHammer = InHammer();
	bool bProjTex = HasFlashlight();
	bool bSelfIllum = !bProjTex && HasFlag(MATERIAL_VAR_SELFILLUM); // No SelfIllum under projected Textures.

	// This used to be very unorderly. I sorted this by Texture now.
	// Let's start with BaseTexture Variables.
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasBaseTexture2 = IsTextureLoaded(BaseTexture2);
	bool bAnyBaseTexture = bHasBaseTexture || bHasBaseTexture2;

	// All Parameters using the BaseTextures Alpha ( Except Phong ones )
	bool bBaseAlphaEnvMapMask = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bBlendTintByBaseAlpha = GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = !bBlendTintByBaseAlpha && GetBool(DesaturateWithBaseAlpha);

	// Normal Map Variables
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bHasNormalTexture2 = IsTextureLoaded(BumpMap2);
	bool bAnyNormalTexture = bHasNormalTexture || bHasNormalTexture2;
	bool bHasSSBump = bAnyNormalTexture && GetBool(SSBump);

	// All Parameters using the BumpMaps Alpha
	bool bNormalMapAlphaEnvMapMask = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK); // No Envmapping under the flashlight

	// Detail Texture Variables
	bool bHasDetailTexture = IsTextureLoaded(Detail);
	bool bHasDetailTexture2 = IsTextureLoaded(Detail2);
	bool bAnyDetailTexture = bHasDetailTexture || bHasDetailTexture2;

	// Not checking BlendModes for Detail here. 5 and 6 have to happen on the Shader.
	// Ironically, EmissiveBlend does not support Blending Emissive.

	// SelfIllum related Variables
	// Needs $SelfIllum specifically.
	bool bHasSelfIllumMask = bSelfIllum && IsTextureLoaded(SelfIllumMask);
	bool bHasSelfIllumMask2 = bSelfIllum && IsTextureLoaded(SelfIllumMask2);
	bool bAnySelfIllumMask = bHasSelfIllumMask || bHasSelfIllumMask2;

	// LightWarpTexture
	bool bHasLightWarpTexture = !bProjTex && IsTextureLoaded(LightWarpTexture); // No Lightwarp under the Flashlight

	// ShiroDkxtro2: Hammer++ does not use Bumped Lightmaps in the Lighting Preview!
	// Phong will look wrong without Radiosity Normal Mapping ( due to how it works ).
	// & I had several Reports from People that said it's "distracting", so we have to disable it.
	bool bHasPhong = !bInHammer && bHasNormalTexture && GetBool(Phong);

	// ExponentTextures
	bool bHasPhongExponentTexture = bHasPhong && IsTextureLoaded(PhongExponentTexture);
	bool bHasPhongExponentTexture2 = bHasPhong && IsTextureLoaded(PhongExponentTexture2);
	bool bAnyPhongExponentTexture = bHasPhongExponentTexture || bHasPhongExponentTexture2;

	// Singular Warp Texture.
	bool bHasPhongWarpTexture = bHasPhong && IsTextureLoaded(PhongWarpTexture);

	// Important difference here between Has and Use EnvMap
	// If we have an EnvMap we apply everything but if we don't USE it, we apply TEXTURE_BLACK instead
	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap); // No EnvMap under projected Textures
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	// EnvMapMasks
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bHasEnvMapMask2 = bHasEnvMap && IsTextureLoaded(EnvMapMask2);
	bool bAnyEnvMapMask = bHasEnvMapMask || bHasEnvMapMask2;

	// The first real Feature of this Shader..
	bool bHasBlendModulateTexture = IsTextureLoaded(BlendModulateTexture);

	//==========================================================================//
	// Pre-Snapshot Context Data Variables
	//==========================================================================//
	if(IsSnapshottingCommands())
	{
		pContextData->m_nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
		pContextData->m_bIsFullyOpaque = IsFullyOpaque(pContextData->m_nBlendType);
	}

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if(IsSnapshotting())
	{
		//	1 = $EnvMap - Mask determined through lerps
		//	2 = $EnvMap + $EnvMapMask
		//  3 = Same as 1 + PCC
		//  4 = Same as 2 + PCC
		int nEnvMapMode = bHasEnvMap + bAnyEnvMapMask + 2 * bPCC;

		// 1 = SelfIllum + SelfIllumMask
		// 2 = $SelfIllum_EnvMapMask_Alpha
		int nSelfIllumMode = (bAnyEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha)) ? 2 : bSelfIllum;

		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(pContextData->m_nBlendType);

		// We always need this
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque);

		// Weird name, what it actually means : We output linear Values
		bool bSRGBWrite = !GetBool(LinearWrite); // Stock Consistency
		pShaderShadow->EnableSRGBWrite(bSRGBWrite);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We always need VertexColors for Blending.
		unsigned int nFlags = VERTEX_POSITION | VERTEX_COLOR;

		// EnvMap wants Tangents, ProjTex always needs Normals
		if(bProjTex || bHasEnvMap || bHasPhong)
			nFlags |= VERTEX_NORMAL;

		// Normal Maps don't require the TBN Matrix actually.
		// ( Due to how Radiosity Normal Mapping works )
		// Projected Textures and EnvMaps do
		if (bProjTex && bAnyNormalTexture || bHasEnvMap || bHasPhong)
			nFlags |= VERTEX_TANGENT_SPACE;

		// No Lightmap UV when using projected Textures
		// 1 TexCoord  = UV
		// 2 TexCoords = UV + Lightmap UV
		// 3 TexCoords = UV + Lightmap UV + Bumped Lightmap UV Offset
		int nTexCoords = bProjTex ? 1 : 2;
		if(!bProjTex && bHasNormalTexture) 
			nTexCoords = 3;

		int nUserDataSize = 0;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// Small Register Map since this Shader gets a bit cluttered...
		// ShiroDkxtro2: I moved these around a little bit to make this more organised
		// EnvMapMask is at the back here because PhongExponent will still be used under projected Textures
		// s0	- $BaseTexture
		// s1	- $BaseTexture2
		// s2	- $BumpMap
		// s3	- $BumpMap2
		// s4	- $Detail
		// s5	- $Detail2
 		// s6	- $PhongExponentTexture
		// s7	- $PhongExponentTexture2
		// s8	- $BlendModulateTexture
		// s9	- $EnvMapMask				- Free for ProjTex
		// s10	- $EnvMapMask2				- Free for ProjTex
		// s11	- Lightmap
		// s12	- LightWarp / PhongWarp
		// s13	- ProjTex / $SelfIllumMask
		// s14	- ProjTex / EnvMap 
		// s15	- ProjTex / GammaToLinear LUT / $SelfIllumMask2 ( likely broken because of the LUT )

		// Always having BaseTextures. ( And always sRGB )
		EnableSampler(SHADER_SAMPLER0, true);
		EnableSampler(SHADER_SAMPLER1, true);

		// Normal Maps are never sRGB
		if (bAnyNormalTexture)
		{
			EnableSampler(SHADER_SAMPLER2, false);
			EnableSampler(SHADER_SAMPLER3, false);
		}

		// s4, s5 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		if (bAnyDetailTexture)
		{
			int nDetailBlendMode = GetInt(DetailBlendMode);
			bool sRGBReadDetail = IsGammaDetailMode(nDetailBlendMode);
			EnableSampler(SHADER_SAMPLER4, sRGBReadDetail);
			EnableSampler(SHADER_SAMPLER5, sRGBReadDetail);

		}

		// PhongExponentTextures are always Linear
		if (bAnyPhongExponentTexture)
		{
			EnableSampler(SHADER_SAMPLER6, false);
			EnableSampler(SHADER_SAMPLER7, false);
		}

		EnableSampler(bHasBlendModulateTexture, SHADER_SAMPLER8, false);

		// Stock-Consistency: EnvMapMasks are sRGB
		if (bAnyEnvMapMask)
		{
			EnableSampler(SHADER_SAMPLER9, true);
			EnableSampler(SHADER_SAMPLER10, true);
		}

		// LDR needs sRGB Lightmaps
		EnableSampler(!bProjTex, SAMPLER_LIGHTMAP, !IsHDREnabled());

		// PhongWarp is never sRGB
		// Lightwarp is sRGB
		// These exclude eachother due to our Sampler Limitation
		if (bHasPhong)
			EnableSampler(bHasPhongWarpTexture, SHADER_SAMPLER12, false);
		else
			EnableSampler(bHasLightWarpTexture, SHADER_SAMPLER12, false);

		// s14 - $EnvMap. sRGB when LDR.
		EnableSampler(bHasEnvMap, SHADER_SAMPLER14, !IsHDREnabled());

		if (bAnySelfIllumMask)
		{
			EnableSampler(SHADER_SAMPLER13, true);
			EnableSampler(SHADER_SAMPLER15, true);
		}

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		int nNeededTexCoords = bAnyNormalTexture + bAnyDetailTexture + bAnyEnvMapMask;
		bool bNeedsTangents = bAnyNormalTexture || bHasEnvMap;

		// Phong always needs Bumped Lightmaps 
		bool bNeedsBumpedLightmaps = !bProjTex && (bAnyNormalTexture && !GetBool(NoDiffuseBumpLighting) || bHasPhong);

		DECLARE_STATIC_VERTEX_SHADER(lux_displacement_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(PROJTEX, bProjTex);
		SET_STATIC_VERTEX_SHADER_COMBO(BLENDMODULATE_UV, bHasBlendModulateTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
		SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bNeedsTangents);
		SET_STATIC_VERTEX_SHADER_COMBO(BUMPED_LIGHTMAP, bNeedsBumpedLightmaps);
		SET_STATIC_VERTEX_SHADER(lux_displacement_vs30);

		int nDetailCombo = bHasDetailTexture + bHasDetailTexture2;
		if (bProjTex)
		{
			int nExponentTexture = bHasPhongExponentTexture ? 1 : bHasPhongExponentTexture2 * 2;
			DECLARE_STATIC_PIXEL_SHADER(lux_worldvertextransition_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATETEXTURE, bHasBlendModulateTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(EXPONENTTEXTURE, nExponentTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(BUMPMAPPED, bAnyNormalTexture + bHasSSBump);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailCombo);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_worldvertextransition_flashlight_ps30);
		}
		else
		{
			if (bHasPhong)
			{
				int nExponentTexture = bHasPhongExponentTexture ? 1 : bHasPhongExponentTexture2 * 2;
				DECLARE_STATIC_PIXEL_SHADER(lux_worldvertextransition_phong_ps30);
				SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATETEXTURE, bHasBlendModulateTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(EXPONENTTEXTURE, nExponentTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
				SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
				SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailCombo);
				SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasSSBump);
				SET_STATIC_PIXEL_SHADER(lux_worldvertextransition_phong_ps30);
			}
			else if (bAnyNormalTexture)
			{
				DECLARE_STATIC_PIXEL_SHADER(lux_worldvertextransition_bump_ps30);
				SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATETEXTURE, bHasBlendModulateTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
				SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
				SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
				SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailCombo);
				SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasSSBump);
				SET_STATIC_PIXEL_SHADER(lux_worldvertextransition_bump_ps30);
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER(lux_worldvertextransition_simple_ps30);
				SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATETEXTURE, bHasBlendModulateTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
				SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
				SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
				SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailCombo);
				SET_STATIC_PIXEL_SHADER(lux_worldvertextransition_simple_ps30);
			}
		}
	}

	//==========================================================================//
	// Post-Snapshot Context Data Static Commands
	//==========================================================================//
	if(IsSnapshottingCommands())
	{
		// Set the Buffer back to its original ( Empty ) State
//		StaticCmds.Reset();

		// Instruct the Buffer to set an End Point
//		StaticCmds.End();

		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Pre-Dynamic Context Data Semi-Static Commands
	//==========================================================================//
	if(MaterialVarsChanged())
	{
		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		//==========================================================================//
		// Bind StandardTextures
		//==========================================================================//

		// s0, s1 - $BaseTexture, $BaseTexture2
		if (!bAnyBaseTexture)
		{
			if (bHasEnvMap)
			{
				// Weird Stock Scenario where we bind $BaseTexture2 to s0
				if(!bHasBaseTexture && !bHasNormalTexture2)
					SemiStaticCmds.BindTexture(SHADER_SAMPLER0, TEXTURE_BLACK);

				if (!bHasBaseTexture2)
					SemiStaticCmds.BindTexture(SHADER_SAMPLER1, TEXTURE_BLACK);
			}
			else
			{
				// Weird Stock Scenario where we bind $BaseTexture2 to s0
				if (!bHasBaseTexture && !bHasNormalTexture2)
					SemiStaticCmds.BindTexture(SHADER_SAMPLER0, TEXTURE_WHITE);

				if(!bHasBaseTexture2)
					SemiStaticCmds.BindTexture(SHADER_SAMPLER1, TEXTURE_WHITE);
			}
		}
		
		// s2
		// If using $BumpMap2 but not $BumpMap, the first BumpMap will be flat
		if(bAnyNormalTexture)
		{
			if(!bHasNormalTexture)
			{
				// Texture Grey should work as a SSBump Fallback
				if(bHasSSBump)
					SemiStaticCmds.BindTexture(SHADER_SAMPLER2, TEXTURE_GREY_ALPHA_ZERO);
				else
					SemiStaticCmds.BindTexture(SHADER_SAMPLER2, TEXTURE_NORMALMAP_FLAT);
			}
		}

		// s4 and s5 don't need StandardTexture Fallbacks. ( DetailTextures )

		// s6 and s7 don't need StandardTexture Fallbacks. ( PhongExponentTextures )

		// s8 doesn't need one either.

		// s9 and s10
		if (bAnyEnvMapMask)
		{
			// Any means we have at least one, so one might be false
			// In that case bind the *White* Texture *unless* we have $BaseTextureNoEnvMap or $BaseTextureNoEnvMap2
			// 
			// Default Behaviour of "no Mask" requires us to use a WhiteTexture
			// No Texture means we don't want any Texture.
			if(GetBool(BaseTextureNoEnvMap))
				SemiStaticCmds.BindTexture(SHADER_SAMPLER9, TEXTURE_BLACK);
			else if(!bHasEnvMapMask)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER9, TEXTURE_WHITE);

			if (GetBool(BaseTexture2NoEnvMap))
				SemiStaticCmds.BindTexture(SHADER_SAMPLER10, TEXTURE_BLACK);
			else if (!bHasEnvMapMask2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER10, TEXTURE_WHITE);
		}

		// s11 - Lightmap
		if(!bProjTex)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER11, TEXTURE_LIGHTMAP);

		// s12 doesn't require Anything.

		// ShiroDkxtro2: This might lead to some Issues with s15.
		// The Shader is forcing my Hand!
		if (bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha) && bAnySelfIllumMask)
		{
			if(!bHasSelfIllumMask)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER13, TEXTURE_BLACK);
			else if (!bHasSelfIllumMask2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER15, TEXTURE_BLACK);
		}

		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		if (bAnyBaseTexture)
		{
			if (bHasBaseTexture)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);

			// Stock-Consistency - Weird Scenario
			if (!bHasBaseTexture && bHasNormalTexture2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER1, BaseTexture2, Frame);
			else if (bHasBaseTexture2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER1, BaseTexture2, Frame2);
		}

		if (bAnyNormalTexture)
		{
			// if we don't have a $BumpMap just bind the $BumpMap2 again.
			// This will cancel out on the Blend in the Shader, reproducing Stock-Shader Behaviour
			if (bHasNormalTexture)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER2, BumpMap, BumpFrame);

			// Bump2 but no Bump1 means Flat Normal Fallback ( handled above )

			// Stock-Consistency:
			// Both Sides are $BumpMap if there is no $BumpMap2
			if (bHasNormalTexture2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER3, BumpMap2, BumpFrame2);
			else if(bHasNormalTexture)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER3, BumpMap, BumpFrame);
		}
		
		if (bAnyDetailTexture)
		{
			if(bHasDetailTexture)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER4, Detail, DetailFrame);

			if(bHasDetailTexture2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER5, Detail2, DetailFrame2);
		}

		if (bAnyPhongExponentTexture)
		{
			if(bHasPhongExponentTexture)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER6, PhongExponentTexture, PhongExponentTextureFrame);
			else
				SemiStaticCmds.BindTexture(SHADER_SAMPLER6, PhongExponentTexture2, PhongExponentTextureFrame2);

			if (bHasPhongExponentTexture2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER7, PhongExponentTexture2, PhongExponentTextureFrame2);
			else
				SemiStaticCmds.BindTexture(SHADER_SAMPLER7, PhongExponentTexture, PhongExponentTextureFrame);
		}

		if(bHasBlendModulateTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER8, BlendModulateTexture, BlendMaskFrame);

		if (bAnyEnvMapMask)
		{
			if(bHasEnvMapMask)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER9, EnvMapMask, EnvMapMaskFrame);

			if (bHasEnvMapMask2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER10, EnvMapMask2, EnvMapMaskFrame2);
		}

		if (bHasPhongWarpTexture)
		{
			SemiStaticCmds.BindTexture(SHADER_SAMPLER12, PhongWarpTexture, PhongWarpTextureFrame);
		}
		else if (bHasLightWarpTexture)
		{
			SemiStaticCmds.BindTexture(SHADER_SAMPLER12, LightWarpTexture, LightWarpTextureFrame);
		}

		if (bAnySelfIllumMask && !GetBool(SelfIllum_EnvMapMask_Alpha))
		{
			if(bHasSelfIllumMask)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER13, LightWarpTexture, LightWarpTextureFrame);

			// This probably doesn't work because of s15..
			if (bHasSelfIllumMask2)
				SemiStaticCmds.BindTexture(SHADER_SAMPLER15, LightWarpTexture, LightWarpTextureFrame);
		}

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//
		
		// A babillion Texture Transforms!
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		// Fallback to first Transform if no second Transform
		bool bTransform2 = HasTransform(bHasBaseTexture2, BaseTextureTransform2);
		if (bTransform2)
			SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform2);
		else
			SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform);

		int nRegisterShift = 0;
		if(bAnyNormalTexture)
		{
			bool bBumpTransform = HasTransform(bHasNormalTexture, BumpTransform);
			if (bBumpTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, BumpTransform);
			else
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, BaseTextureTransform);

			// If we have Bump2Transform, use that
			// If we have BumpTransform, use that 
			// We don't have that either? Well check Transform2 since this is for the Second Texture
			// Don't have it? Use Regular Transform.
			// A loooot of Logic
			bool bBump2Transform = HasTransform(bHasNormalTexture2, BumpTransform2);
			if (bBump2Transform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, BumpTransform2);
			else if (bBumpTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, BumpTransform);
			else if (bAnyNormalTexture && bTransform2)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform2);
			else
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform);

			nRegisterShift += 4;
		}

		if(bAnyEnvMapMask)
		{
			bool bEnvMapMaskTransform = HasTransform(bHasEnvMapMask, EnvMapMaskTransform);
			if (bEnvMapMaskTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_03, EnvMapMaskTransform);
			else
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_03, BaseTextureTransform);

			bool bEnvMapMaskTransform2 = HasTransform(bHasEnvMapMask2, EnvMapMaskTransform2);
			if (bEnvMapMaskTransform2)
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, EnvMapMaskTransform2);
			else if (bEnvMapMaskTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, EnvMapMaskTransform);
			else if (bTransform2)
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform2);
			else 
				SemiStaticCmds.SetVertexShaderTextureTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform);

			nRegisterShift += 4;
		}

		if(bAnyDetailTexture)
		{
			bool bDetailTransform = HasTransform(bHasDetailTexture, DetailTextureTransform);
			if (bDetailTransform)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_03, DetailTextureTransform, DetailScale);
			else if (bAnyDetailTexture)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_03, BaseTextureTransform, DetailScale);

			// BUG:BUG: We use $Detail2 here and we cannot check if $Detail is used
			// So when we fallback to other Transforms we might have a differing Scale!!
			bool bDetailTransform2 = HasTransform(bHasDetailTexture2, DetailTextureTransform2);
			if(bDetailTransform2)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, DetailTextureTransform2, DetailScale2);
			else if (bDetailTransform)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, DetailTextureTransform, DetailScale2);
			else if (bTransform2)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform2, DetailScale2);
			else
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(nRegisterShift + LUX_VS_TEXTURETRANSFORM_04, BaseTextureTransform, DetailScale2);
		}

		// Always at the bottom of the Registers
		if (bHasBlendModulateTexture)
		{
			bool bBlendTransform = HasTransform(bHasBlendModulateTexture, BlendMaskTransform);
			if (bBlendTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_09, BlendMaskTransform);
			else 
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_09, BaseTextureTransform);		
		}

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Register Map:
		//
		// c1	- Modulation Constant
		// .. bla bla
		// c32	- Tint1 + AlphaVar
		// c33	- DetailBlendMode
		// c34	- DetailTint + BlendFactor
		// c35	- SelfIllum Tint + MaskScale
		// c36	- SelfIllum Fresnel
		// c37	- EnvMap Tint + LightScale
		// c38	- EnvMap Saturation + Contrast
		// c39	- EnvMap Controls
		// c40	- EnvMap Fresnel
		// c41	- EnvMap Origin
		// c42	- EnvMap OBB1
		// c43	- EnvMap OBB2
		// c44	- EnvMap OBB3
		// c45	- Phong Tint + InvertPhongMask
		// c46	- Phong Fresnel1 + ExponentFactor1
		// c47	- Phong Controls
		// c48	- MinLight PhongBoost
		// c49	- Phong Fresnel2 + ExponentFactor2

		// c1 - Modulation Constant
		// ASW has this Comment regarding LightmapScaleFactor.
		// "Also note that if the lightmap scale factor changes
		// all shadow state blocks will be re-run, so that's ok"
		// So don't need to worry about LightmapScaleFactor,
		// different for $color2, Proxies won't cause a Command Buffer refresh?
		bool bIsBrush = true;
		bool bApplySSBumpMathFix = bHasSSBump && GetBool(SSBumpMathFix);
		float4 f4ModulationConstant = GetModulationConstant(bIsBrush, bApplySSBumpMathFix);
		SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_MODULATIONCONSTANTS, f4ModulationConstant);

		// c11 - Camera Position
		SemiStaticCmds.SetPixelShaderConstant_EyePos(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		SemiStaticCmds.SetPixelShaderConstant_FogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// WVT is a bit disorganised..
		// Let's try to fix that..

		// c32 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// c33, c34 - $DetailTint, $DetailBlendFactor, $DetailBlendMode
		if (bHasDetailTexture)
		{
			float4 f4Detail1Tint_Factor;
			f4Detail1Tint_Factor.xyz = GetFloat3(DetailTint);
			f4Detail1Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Detail1Tint_Factor = PrecomputeDetail(f4Detail1Tint_Factor, GetInt(DetailBlendMode));
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4Detail1Tint_Factor);

			float4 f4BlendMode = 0.0f;
			f4BlendMode.x = (float)GetInt(DetailBlendMode);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4BlendMode);
		}

		// c35 and c36
		if (bSelfIllum)
		{
			// No SelfIllumMask means Alpha will be used for SelfIllumMask instead ( 0.0f )
			float4 f4SelfIllumTint_Scale;
			f4SelfIllumTint_Scale.rgb = GetFloat3(SelfIllumTint);
			f4SelfIllumTint_Scale.a = bHasSelfIllumMask ? GetFloat(SelfIllumMaskScale) : 0.0f;
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FACTORS, f4SelfIllumTint_Scale);

			if (GetBool(SelfIllumFresnel))
			{
				float4 f4SelfIllumFresnelMinMaxExp;
				f4SelfIllumFresnelMinMaxExp.xyz = GetFloat3(SelfIllumFresnelMinMaxExp);
				f4SelfIllumFresnelMinMaxExp.a = 0.0f; // Empty
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FRESNEL, f4SelfIllumFresnelMinMaxExp);
			}
		}
		
		// c37, c38, c39, c40, c41, c42, c43, c44
		if (bHasEnvMap)
		{
			// Stock-Consistency
			// Linear EnvMapTint
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.xyz = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.xyz = GetFloat3(EnvMapSaturation); // Yes, this *is* a float3 Parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = (float)GetBool(EnvMapMaskFlip);
			f4EnvMapControls.w = (float)!GetBool(BaseTextureNoEnvMap);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// $EnvMapFresnelMinMaxExp
			if(true)
			{
				float4 f4EnvMapFresnelRanges = 0.0f;
				f4EnvMapFresnelRanges.w = (float)!GetBool(BaseTexture2NoEnvMap); // Empty
	
				if(GetBool(EnvMapFresnel))
					f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
				else
				{
					// EnvMapFresnel is off, but we still compute it
					// This will disable it
					f4EnvMapFresnelRanges.y = 1.0f;
				}
	
				// Always set this due to how EnvMapFresnel is setup in the Shader
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);		
			}

			if(bPCC)
			{
				// c38
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapOrigin);

				// c39, c40, c41
				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);

				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX, f4Row1);
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_2, f4Row2);
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_3, f4Row3);
			}
		}

		// c45, c46, c47, c48, c49
		if (bHasPhong)
		{
			bool bBaseTextureNoPhong = GetBool(BaseTextureNoPhong);
			bool bBaseTexture2NoPhong = GetBool(BaseTexture2NoPhong);
			
			// c45
			float4 f4PhongTint1;
			f4PhongTint1.rgb = GetFloat3(PhongTint);
			f4PhongTint1.w = (float)GetBool(InvertPhongMask);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint1);

			// c46
			// PhongFresnelRanges and PhongExponentFactor
			float4 FresnelAndFactor1;
			FresnelAndFactor1.xyz = GetFloat3(PhongFresnelRanges);
			FresnelAndFactor1.x = (FresnelAndFactor1.y - FresnelAndFactor1.x) * 2.0f;
			FresnelAndFactor1.z = (FresnelAndFactor1.z - FresnelAndFactor1.y) * 2.0f;
			FresnelAndFactor1.w = GetFloat(PhongExponentFactor);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_FRESNEL, FresnelAndFactor1);

			// c47
			float4 f4PhongControls;
			f4PhongControls.x = GetFloat(PhongAlbedoBoost);
			f4PhongControls.y = GetFloat(PhongExponent);
			f4PhongControls.z = IsDefined(PhongExponent2) ? GetFloat(PhongExponent2) : GetFloat(PhongExponent);
			f4PhongControls.w = 0.0f; // Free
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_CONTROLS, f4PhongControls);

			// c48
			float4 f4MinimumLight_PhongBoost;	  
			f4MinimumLight_PhongBoost.x = (float)!bBaseTextureNoPhong;
			f4MinimumLight_PhongBoost.y = (float)!bBaseTexture2NoPhong;
			f4MinimumLight_PhongBoost.z = 0.0f;
			f4MinimumLight_PhongBoost.w = GetFloat(PhongBoost);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST, f4MinimumLight_PhongBoost);

			// c49
			float4 FresnelAndFactor2;
			FresnelAndFactor2.xyz = IsDefined(PhongFresnelRanges2) ? GetFloat3(PhongFresnelRanges2) : GetFloat3(PhongFresnelRanges);
			FresnelAndFactor2.x = (FresnelAndFactor2.y - FresnelAndFactor2.x) * 2.0f;
			FresnelAndFactor2.z = (FresnelAndFactor2.z - FresnelAndFactor2.y) * 2.0f;
			FresnelAndFactor2.w = IsDefined(PhongExponentFactor2) ? GetFloat(PhongExponentFactor2) : GetFloat(PhongExponentFactor);
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_049, FresnelAndFactor2);
		}

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if (IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b1
		// Always! Required for Lightwarp
		// No Lightwarp under the Flashlight, so we can reuse the Register here
		if (!bHasPhong)
		{
#ifdef LUX_DEBUGCONVARS
			if (lux_disablefast_lightwarp.GetBool())
				bHasLightWarpTexture = false;
#endif
			if(bHasLightWarpTexture)
				BBools[LUX_PS_BOOL_LIGHTWARPTEXTURE] = true;
		}

		// b4, b5, b6, b7, b8, b9, b10, b11
		if (bHasPhong)
		{
			BBools[LUX_PS_BOOL_PHONG_BASEMAPALPHAMASK] = GetBool(BaseMapAlphaPhongMask) && !GetBool(BaseMapLuminancePhongMask); // bHasBaseMapLuminancePhongMask overrides the mask used.
			BBools[LUX_PS_BOOL_PHONG_ALBEDOTINT] = GetBool(PhongAlbedoTint);
			BBools[LUX_PS_BOOL_PHONG_FLATNORMAL] = GetBool(PhongFlatNormal);
			BBools[LUX_PS_BOOL_PHONG_RIMLIGHTMASK] = false; // No RimLightMask on Brushes!!
			BBools[LUX_PS_BOOL_PHONG_BASEMAPLUMINANCEMASK] = GetBool(BaseMapLuminancePhongMask);
			BBools[LUX_PS_BOOL_PHONG_EXPONENTTEXTUREMASK] = GetBool(PhongExponentTextureMask);
//			BBools[LUX_PS_BOOL_PHONG_RIMLIGHT] = false; // No RimLighting on Brushes!!
			BBools[LUX_PS_BOOL_PHONG_WARPTEXTURE] = bHasPhongWarpTexture;
		}

		// b12
		// *sigh* Another Hammer specific Issue
		// Vertex Colors are used for Shaded Texture Polygons..
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// Hammer Blendmodulation Fix
		// 13.01.2026: H++ does it's own Shenanigans to fix the BlendFactor on stock Shaders
		// Previously it would also fix it for Custom Shaders but it has since been disabled.
		// This should now work for both Stock and H++!
		bool bHammerBlendFix = /*!g_bHammerPlusPlus &&*/ pShaderAPI->InEditorMode();
		bool bFlipBlendFactor = bHammerBlendFix;
		if (GetBool(FlipBlendFactor))
			bFlipBlendFactor = !bFlipBlendFactor;

		float4 f4Editor = float(bFlipBlendFactor);
		pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET0_0, f4Editor);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_displacement_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_displacement_vs30);

		if (bProjTex)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_flashlight_ps30);
		}
		else
		{
			if (bHasPhong)
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_phong_ps30);
				SET_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_phong_ps30);
			}
			if (bHasNormalTexture)
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_bump_ps30);
				SET_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_bump_ps30);
			}
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_simple_ps30);
				SET_DYNAMIC_PIXEL_SHADER(lux_worldvertextransition_simple_ps30);
			}
		}

//		pShaderAPI->ExecuteCommandBuffer(StaticCmds.Base());
		pShaderAPI->ExecuteCommandBuffer(SemiStaticCmds.Base());
	}

	// We are not done here!
	// The Command Buffer blocks us from using ConVars.
	// Let's overwrite the Constants now. This is fine since we'd only ever use any of these for debugging.
	//==========================================================================//
	// ConVars
	//==========================================================================//
	if(IsDynamicState())
	{
#ifdef LUX_DEBUGCONVARS
		if (bAnyNormalTexture && lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SHADER_SAMPLER2, TEXTURE_NORMALMAP_FLAT);
			BindTexture(SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT);
		}

		if (lux_disablefast_lightmap.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
		}

		if (lux_disablefast_diffuse.GetBool())
		{
			float4 f4Tint;
			f4Tint = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		}

		if (bHasPhong)
		{
			// Compared to Mapbase, we won't force a Value if there already is one in the VMT
			if (lux_lightmapped_phong_force.GetBool() && !lux_disablefast_phong.GetBool())
			{
				if (!bAnyPhongExponentTexture)
				{
					float4 Tint1;
					Tint1.rgb = lux_lightmapped_phong_force_boost.GetFloat();
					Tint1.rgb *= (float)!GetBool(BaseTextureNoPhong);
					Tint1.w = (float)GetBool(InvertPhongMask);
					pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_045, Tint1);

					// If we don't have a second Tint or Boost, use the first
					float4 Tint2;
					Tint2.rgb = lux_lightmapped_phong_force_boost.GetFloat();

					Tint2.rgb *= (float)!GetBool(BaseTexture2NoPhong);
					Tint2.w = 0.0f; // No dual-Invert
					pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_046, Tint2);

					float4 f4PhongControls;
					f4PhongControls.x = 0.0f;
					f4PhongControls.yz = lux_lightmapped_phong_force_exp.GetFloat();
					f4PhongControls.w = 0.0f;
					pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_049, f4PhongControls);
				}
			}

			if (lux_disablefast_phong.GetBool())
			{
				float4 f4PhongTint_InvertMask;
				f4PhongTint_InvertMask = 0.0f;
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_045, f4PhongTint_InvertMask);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_046, f4PhongTint_InvertMask);
			}
		}

		if (bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha) && lux_disablefast_selfillum.GetBool())
		{
			float4 f4SelfIllumTint_Scale;
			f4SelfIllumTint_Scale.xyz = 0.0f;
			f4SelfIllumTint_Scale.w = 1.0f;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_050, f4SelfIllumTint_Scale);

			BindTexture(SHADER_SAMPLER13, TEXTURE_BLACK);
			BindTexture(SHADER_SAMPLER15, TEXTURE_BLACK);
		}

		if (bHasEnvMap && lux_disablefast_envmap.GetBool())
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);
		}
#endif

#ifdef DEBUG_LUXELS
		if (mat_luxels.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
		}
#endif

#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		{
			BindTexture(SHADER_SAMPLER0, TEXTURE_GREY);
			BindTexture(SHADER_SAMPLER1, TEXTURE_GREY);
		}
#endif
	}

	Draw();
}
END_SHADER