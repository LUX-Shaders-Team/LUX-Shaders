//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

#include "materialsystem/MaterialSystemUtil.h"

// This allows for additional passes for other effects
#include "renderpasses/Cloak.h"
#include "renderpasses/SheenPass.h"
#include "renderpasses/EmissiveBlend.h"
#include "renderpasses/FleshInterior.h"
#include "renderpasses/MeshOutline.h"

// Includes for Shaderfiles...
#include "lux_model_simplified_vs30.inc"
#include "lux_model_vs30.inc"
#include "lux_model_bump_vs30.inc"
#include "lux_vertexlitgeneric_simple_ps30.inc"
#include "lux_vertexlitgeneric_bump_ps30.inc"
#include "lux_vertexlitgeneric_phong_ps30.inc"
#include "lux_vertexlitgeneric_flashlight_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_VertexLitGeneric,		LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(SDK_Skin_DX9,				LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(SDK_VertexLitGeneric_DX9,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(SDK_VertexLitGeneric_DX8,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(SDK_VertexLitGeneric_DX7,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(SDK_VertexLitGeneric_DX6,	LUX_VertexLitGeneric)
#endif

#ifdef REPLACE_VERTEXLITGENERIC
DEFINE_FALLBACK_SHADER(VertexLitGeneric,		LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(Skin_DX9,				LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(VertexLitGeneric_DX9,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(VertexLitGeneric_DX8,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(VertexLitGeneric_DX7,	LUX_VertexLitGeneric)
DEFINE_FALLBACK_SHADER(VertexLitGeneric_DX6,	LUX_VertexLitGeneric)
#endif

CON_COMMAND_F(lux_toggle_envmaplerp, "Forces EnvMapLerp and reloads all Materials.\n", FCVAR_NONE)
{
	// Connect MatSys if it hasn't been
	if (!g_pMaterialSystem)
		g_pMaterialSystem = LoadInterface<IMaterialSystem>(MATERIALSYSTEM_DLL_NAME, MATERIAL_SYSTEM_INTERFACE_VERSION );

	// Turn on
	bool bCurrentValue = lux_envmap_forcelerp.GetBool();
	lux_envmap_forcelerp.SetValue(!bCurrentValue);

	// Reload all Materials
	g_pMaterialSystem->ReloadMaterials();
}

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class VertexLitGenericContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	// Intended for Debugging at some Point
	// Should also be here since we have to evaluate this at the same Time,
	// as the Phong Variables below.
	int m_nEnvMapMode = 0;
	int m_nSelfIllumMode = 0;
	int m_bBaseAlphaEnvMapMask = false;
	int m_bNormalMapAlphaEnvMapMask = false;


	// This is for Phong.
	// We need a Cargoship worth of Logic to make sure Stock Materials look the same.
	bool m_bEnvMapFresnel = false;
	bool m_bPhong_BaseMapAlphaPhongMask = false;
	bool m_bPhong_AlbedoTint = false;
	bool m_bPhong_AlbedoTintBoost = false;
	bool m_bPhong_InvertEnvMapMask = false;
	bool m_bPhong_InvertPhongMask = false;
	bool m_bPhong_PhongExponentTextureMask = false;

	bool m_bEnvMapLerp = false;
	bool m_bLerpLock = false;
	CTextureReference m_RefCubemapA;
	CTextureReference m_RefCubemapB;
	float m_f1LerpStart = 0.0f;

	VertexLitGenericContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_VertexLitGeneric, "A shader used to render models with support of variety of effects.")
SHADER_INFO_GEOMETRY	("Models. ( Static Props, Dynamic Props, Physics Props )")
SHADER_INFO_USAGE		("Apply to a Model. Use the Developer CVar and mat_reloadallMaterials to spew helpful Information about Caveats.")
SHADER_INFO_LIMITATIONS	("Models using $BumpMap or $Phong dynamically compute Lighting.\n"
						 "To be specific, this means Per-Vertex and Per-Texel ( Lightmaps ) Lighting will automatically be disabled.\n"
						 "Dynamically lit Models only receive *4 Lightsources* at any given Time. We've seen some Games force a Limit of 3 ( L4D2 ).\n\n"

						 "Dynamic Lighting ( with the exception of projected Textures ) cannot produce adequate Shadows.\n"
						 "Surface that face a Lightsource are illuminated, those facing away will be shadowed.\n"
						 "The Transition between lit and shadowed can be adjusted using Parameters like $HalfLambert and $LightWarpTexture.\n"
						 "$HalfLambert increases the Angle at which Surfaces count as illuminated.\n"
						 "$LightWarpTexture makes use of a Gradient Texture that remaps the Computed Lighting. For best Results use without $HalfLambert\n"
						 "Half-Lambertian Lighting is forced on with $Phong due to Stock-Consistency Reasons.\n"
						 "It can be disabled however, using $PhongDisableHalfLambert or the lux_phong_defaulthalflambert ConVar.\n\n"

						 "Physics Props that don't use $BumpMap or $Phong, rely heavily on Ambient Lighting ( Ambient Cubes ).\n"
						 "This can become problematic and lead to weird shadowing Situations, Ambient Cubes have a limited Density and Resolution.\n"
						 "The larger the Model the worse this Limitation, the whole model has to use a single Set of interpolated Values.\n"
						 "Applying $BumpMap can slightly alleviate this Problem, but it comes with it's own Share of Problems.\n\n"

						 "Dynamic Models with no $BumpMap or $Phong will be lit dynamically, but per-Vertex.\n"
						 "Named Lights will cause something similar on Static Props that don't use Lightmaps.\n\n"

						 "$DetailBlendMode 10 does not work on this Shader, as that requires Bumped Lightmaps.\n\n"
						 "Parameters using the same Alpha Channel are usually incompatible, but there are some Exceptions.\n"
						 "Exceptions include, but are not limited to:\n"
						 "$BaseAlphaEnvMapMask + $BlendTintByBaseAlpha\n"
						 "$AlphaTest or $Translucent + DetailBlendmode 3, 8 or 9\n"
						 "$BaseAlphaEnvMapMask + $BaseMapAlphaPhongMask\n"
						 "$Phong ( without any other Masking Parameters ) + $NormalMapAlphaEnvMapMask")
SHADER_INFO_PERFORMANCE	("Performance varies on the Parameter Combinations used.\n"
						 "Materials that are lit Per-Vertex are the cheapest.\n"
						 "Those with $BumpMap are a bit more expensive, and $Phong is above that.\n"
						 "Modern Hardware can handle this easily however.\n")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC VertexLitGeneric Shader Page: https://developer.valvesoftware.com/wiki/VertexLitGeneric")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_NormalTextureParameters()
	Declare_SelfIlluminationParameters()
	Declare_DetailTextureParameters()
	Declare_SelfIllumTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	SHADER_PARAM(EnvMapLerp, SHADER_PARAM_TYPE_BOOL, "", "When the local Cubemap changes it traditionally snaps, enabling this causes them to interpolate based on Time instead.")
	SHADER_PARAM(BaseAlphaEnvMapMaskMinMaxExp, SHADER_PARAM_TYPE_VEC3, "", "ASW+ Feature, only allowed on Materials without BumpMaps and Phong. Applies Scale, Bias, Exponent to BaseAlphaEnvMapMask.")
	Declare_LightmappingParameters()
	Declare_PhongParameters()
	Declare_RimLightParameters()
	Declare_SeamlessParameters()

	SHADER_PARAM(Compress, SHADER_PARAM_TYPE_TEXTURE, "", "Wrinklemapping, used with Face-Flexes, must be used with $Stretch. See VDC for more Information.")
	SHADER_PARAM(Stretch, SHADER_PARAM_TYPE_TEXTURE, "", "Wrinklemapping, used with Face-Flexes, must be used with $Compress. See VDC for more Information.")
	SHADER_PARAM(BumpCompress, SHADER_PARAM_TYPE_TEXTURE, "", "Wrinklemapping, used with Face-Flexes, must be used with $BumpStretch & $Compress & $Stretch. See VDC for more Information.")
	SHADER_PARAM(BumpStretch, SHADER_PARAM_TYPE_TEXTURE, "", "Wrinklemapping, used with Face-Flexes, must be used with $BumpStretch & $Compress & $Stretch. See VDC for more Information.")

	// Additional Renderpasses
	Declare_EmissiveBlendParameters()
	Declare_FleshInteriorParameters()
	Declare_CloakParameters()
	Declare_SheenPassParameters()
	Declare_MeshOutlineParameters()

	// Treesway Implementation
	Declare_TreeswayParameters()

	Declare_MiscParameters()
	SHADER_PARAM(DistanceAlpha, SHADER_PARAM_TYPE_BOOL, "", "(FALLBACK) This Parameter will cause the Shader to fallback to LUX_DistanceAlpha_Model.")
	SHADER_PARAM(UsesBumpMap, SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER) keeps track of whether or not $BumpMap is VMT Defined or not.")

	// We can't replicate this Parameter but we can at least make Materials from L4D1 not utterly broken
	SHADER_PARAM(ShinyBlood, SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER) Disables Phong to unbreak some L4D1 Materials.")
END_SHADER_PARAMS

void VLG_SetupCloakVars(Cloak_Vars_t &CloakVars)
{
	CloakVars.InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	CloakVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
	CloakVars.Bump.InitVars(BumpMap);
}

void VLG_SetupEmissiveBlendVars(EmissiveBlend_Vars_t& EmissiveVars)
{
	// Emissive Blend Params
	EmissiveVars.InitVars(EmissiveBlendEnabled);

	// Support for $SelfIllumTexture
	EmissiveVars.SelfIllum.InitVars(SelfIllumTexture, SelfIllumTextureFrame);

	// DetailBlendMode 5 and 6 are handled here, since it simplifies our Shaders
	EmissiveVars.Detail.InitVars(Detail, DetailFrame, DetailTextureTransform, DetailScale, DetailBlendMode, DetailTint, DetailBlendFactor);

	// Minimum Light and Transform Fallbacks
	EmissiveVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
}

void VLG_SetupSheenPassVars(SheenPass_Vars_t& SheenVars)
{
	SheenVars.InitVars(SheenPassEnabled, NormalTexture, BumpFrame, BumpTransform);
}

// IMPORTANT: Virtual Function Override.
bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar** params, bool bCheckSpecificToThisFrame ) const override
{
	// Need to use params directly here, otherwise we corrupt m_ppParams for Draw()
	if ( params[CloakPassEnabled]->GetIntValue() )
	{
		float flCloakFactor = params[CloakFactor]->GetFloatValue();
		if ( !bCheckSpecificToThisFrame || (flCloakFactor > 0.0f && flCloakFactor < 1.0f) )
			return true;
	}

	if ( params[SheenPassEnabled]->GetIntValue() )
		return true;

	return params[Flags_Defined2]->GetIntValue() & MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE;
}

// IMPORTANT: Virtual Function Override.
bool IsTranslucent( IMaterialVar** params ) const override
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

// ShiroDkxtro2 : Stock VLG Shader has a huge Amount of Caveats
// So I added this Function that spews Information regarding Caveats that are encountered
// This won't be able to get all of them, but it should make it a lot easier to figure out whats wrong with a Material.
void LuxVertexLitGeneric_ParamsDebugger()
{
	// MSAA Requirement for $AllowAlphaToCoverage
	if (g_pConfig->m_nAASamples == 0 && HasFlag(MATERIAL_VAR_ALLOWALPHATOCOVERAGE) && HasFlag(MATERIAL_VAR_ALPHATEST))
		ShaderDebugMessage("uses $AllowAlphaToCoverage, which requires MSAA. Current AASamples are 0. $AllowAlphaToCoverage won't work.\n");

	// All Textures
	bool bHasBaseTexture = IsDefined(BaseTexture);
	bool bHasNormalTexture = IsDefined(NormalTexture);
	bool bHasDetailTexture = IsDefined(Detail);
	bool bHasLightWarpTexture = IsDefined(LightWarpTexture);
	bool bHasLightWarpNoBump = GetBool(LightWarpNoBump);
	
	bool bSelfIllum = HasFlag(MATERIAL_VAR_SELFILLUM);
//	bool bhasSelfIllumMask = bSelfIllum && IsDefined(SelfIllumMask);

	bool bHasPhong = GetBool(Phong);
	bool bHasPhongExponentTexture = bHasPhong && IsDefined(PhongExponentTexture);
//	bool bHasPhongWarpTexture = bHasPhong && IsDefined(PhongWarpTexture);

	bool bHasEnvMap = IsDefined(EnvMap);
	bool bHasEnvMapMask = IsDefined(EnvMapMask);

	// All BaseTexture Alpha Parameters
	bool bAlphaTest = HasFlag(MATERIAL_VAR_ALPHATEST);
	bool bTranslucent = HasFlag(MATERIAL_VAR_TRANSLUCENT);
	bool bBlendTintByBaseAlpha = GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = GetBool(DesaturateWithBaseAlpha);
	bool bBaseMapAlphaPhongMask = GetBool(BaseMapAlphaPhongMask);
	bool bBaseAlphaEnvMapMask = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);

	// All NormalMap Alpha Parameters
	bool bNormalMapAlphaEnvMapMask = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);

	// EnvMapMask Alpha Parameters ( only one )
	bool bSelfIllum_EnvMapMask_Alpha = GetBool(SelfIllum_EnvMapMask_Alpha);

	// PhongExponentTexture can use the Blue Channel for Masking now.
	bool bPhongExponentTextureMask = GetBool(PhongExponentTextureMask);

	// BumpMap or Phong Path
	bool bBumpedShader = bHasNormalTexture || (bHasPhong && (bBaseMapAlphaPhongMask || bHasLightWarpTexture));

	// No BaseTexture means can't use it's Alpha
	if (!bHasBaseTexture)
	{
		if (bAlphaTest)
			ShaderDebugMessage("uses $AlphaTest, but there is no $BaseTexture to derive it from.\n");

		if (bTranslucent)
			ShaderDebugMessage("uses $Translucent, but there is no $BaseTexture to derive it from.\n");

		if (bBlendTintByBaseAlpha)
			ShaderDebugMessage("uses $BlendTintByBaseAlpha, but there is no $BaseTexture to derive it from.\n");

		if (bDesaturateWithBaseAlpha)
			ShaderDebugMessage("uses $DesaturateWithBaseAlpha, but there is no $BaseTexture to derive it from.\n");

		if(bHasEnvMap && bBaseAlphaEnvMapMask)
			ShaderDebugMessage("uses $BaseAlphaEnvMapMask, but there is no $BaseTexture to derive it from.\n");

		if (bHasPhong && bBaseMapAlphaPhongMask)
			ShaderDebugMessage("uses $BaseMapAlphaPhongMask, but there is no $BaseTexture to derive it from.\n");

		if(bSelfIllum && !IsDefined(SelfIllumMask) && !IsDefined(SelfIllumFresnel))
			ShaderDebugMessage("uses $SelfIllum, but there is no $BaseTexture to derive it from.\n"
			"This would be fine if it used one of the alternative SelfIllum Methods ( $SelfIllumMask or $SelfIllumFresnel )\n");
	}
	else
	{
		// Track how many times we use the Alpha Channel.
		int nAlphaUsed = 0;

		// Add all the Alpha Bools to the Counter
		nAlphaUsed += bAlphaTest;
		nAlphaUsed += bTranslucent;
		nAlphaUsed += bBlendTintByBaseAlpha;
		nAlphaUsed += bDesaturateWithBaseAlpha;
		nAlphaUsed += bBaseAlphaEnvMapMask;
		nAlphaUsed += bBaseMapAlphaPhongMask;
		nAlphaUsed += bSelfIllum;

		// This one doesn't count, we replicate L4D Behaviour with it..
		if (bBlendTintByBaseAlpha && bBaseAlphaEnvMapMask)
			nAlphaUsed -= 1;

		// This one doesn't matter.
		if(bHasEnvMap && bBaseAlphaEnvMapMask && bHasPhong && bBaseMapAlphaPhongMask)
			nAlphaUsed -= 1;

		// Huh I guess that means you can use $BaseMapAlphaEnvMapMask + $BaseMapAlphaPhongMask + $BlendTintByBaseAlpha now?
		// Hope no one does that.. $BaseMapAlphaEnvMapMask + $BlendTintByBaseAlpha is bad enough..
		
		// Conflicting Alpha Parameters
		if (nAlphaUsed > 1)
		{
			if (bAlphaTest && bTranslucent)
				ShaderDebugMessage("uses $AlphaTest and $Translucent. $Translucent will (probably) take Priority.\n");
			else if (bBlendTintByBaseAlpha && bDesaturateWithBaseAlpha)
				ShaderDebugMessage("uses $BlendTintByBaseAlpha and $DesaturateWithBaseAlpha. BlendTintByBaseAlpha will take Priority.\n");
			else if (bAlphaTest && bTranslucent && bBaseAlphaEnvMapMask)
				ShaderDebugMessage("uses $AlphaTest or $Translucent with $BaseAlphaEnvMapMask, .\n");
			else
				// This isn't very specific. Bad!
				// Good enough for now.
				ShaderDebugMessage("uses the BaseTexture's Alpha Channel multiple times for different Effects.\n");
		}
	}

	if (!bHasNormalTexture)
	{
		if(bHasEnvMap && bNormalMapAlphaEnvMapMask)
			ShaderDebugMessage("uses $NormalMapAlphaEnvMapMask, but there is no $BumpMap to derive it from.\n");

		if (bHasPhong && !bPhongExponentTextureMask && !bBaseMapAlphaPhongMask && !bHasLightWarpTexture)
			ShaderDebugMessage("uses $Phong, but no Mask can be derived. Phong will not work.\n"
				"You need any of these: $BumpMap, $PhongExponentTextureMask, $BaseMapAlphaPhongMask.\n");
	}
	else
	{
		// These are supported, so this won't be a Problem
		// bNormalMapAlphaEnvMapMask, bHasPhong

		// Wrinklemapping
		int nWrinkles = IsDefined(Stretch) + IsDefined(Compress);
		if (nWrinkles == 1)
		{
			ShaderDebugMessage("uses only one of $Stretch and $Compress. Both are needed for the Feature to be enabled.\n");
		}

		nWrinkles = IsDefined(BumpStretch) + IsDefined(BumpCompress);
		if (nWrinkles == 1)
		{
			ShaderDebugMessage("uses only one of $BumpStretch and $BumpCompress. Both are needed for the Feature to be enabled.\n");
		}
	}

	if (!bHasEnvMap)
	{
		if(bHasEnvMapMask)
			ShaderDebugMessage("uses $EnvMapMask without $EnvMap. $EnvMap is required.\n");
	}

	if (!bHasEnvMapMask)
	{
		if(bSelfIllum_EnvMapMask_Alpha)
			ShaderDebugMessage("uses $SelfIllum_EnvMapMask_Alpha, but there is no $EnvMapMask to derive it from.\n");

		if(bHasEnvMap && bBaseAlphaEnvMapMask && bNormalMapAlphaEnvMapMask)
			ShaderDebugMessage("has $BaseAlphaEnvMapMask and $NormalMapAlphaEnvMapMask. $NormalMapAlphaEnvMapMask will take priority.\n");
	}
	else
	{
		if(bHasEnvMap && bBaseAlphaEnvMapMask)
			ShaderDebugMessage("has $EnvMapMask and $BaseAlphaEnvMapMask. $EnvMapMask will take priority.\n");

		if (bHasEnvMap && bNormalMapAlphaEnvMapMask)
			ShaderDebugMessage("has $EnvMapMask and $NormalMapAlphaEnvMapMask. $EnvMapMask will take priority.\n");
	}

	// ASW+ Behaviour not supported beyond Simple VLG
	if(bHasEnvMap && bBaseAlphaEnvMapMask && bBumpedShader && IsDefined(BaseAlphaEnvMapMaskMinMaxExp))
	{
		ShaderDebugMessage("uses $BaseAlphaEnvMapMaskMinMaxExp on the bumpmapped Variant of this Shader. This is not supported.\n");	
	}

	// Not supported on Models
	if (bHasDetailTexture && GetInt(DetailBlendMode) == 10)
	{
		ShaderDebugMessage("tries to use $DetailBlendMode 10. This is not supported.\n");
	}

	// Almost every Material ever on TF2 does this!
	// It's so many Materials that the Console becomes useless with how often this gets printed.
	/*
	if (bHasLightWarpTexture && !bHasLightWarpNoBump && !bHasNormalTexture)
	{
		ShaderDebugMessage("uses $LightWarpTexture without $BumpMap.\nIf this is intentional, ignore this Message.\nOtherwise add a $BumpMap or use $LightWarpNoBump.\n");
	}
	*/

	// All Phong related Caveats
	if (GetBool(Phong) && (bHasNormalTexture || bBaseMapAlphaPhongMask || bHasLightWarpTexture && !bHasLightWarpNoBump))
	{
		bool bPhongNewBehaviour = GetBool(PhongNewBehaviour);

		// All caveats that will be replicated when not under new behaviour should be warned about...
		if (!bPhongNewBehaviour)
		{
			// Works on Stock but not on LUX. Very very specific Caveat that is probably never triggered.
			if (bNormalMapAlphaEnvMapMask && IsDefined(BaseMapLuminancePhongMask) && GetBool(BaseMapLuminancePhongMask))
			{
				ShaderDebugMessage("uses $NormalMapAlphaEnvMapMask and $BaseMapLuminancePhongMask.\n"
					"Stock Shaders would use the Luminance as the EnvMapMask, this is not supported on LUX.\n"
					"EnvMap will be masked using the $BumpMaps Alpha. Ignore this Message if the Material still looks ok.\n");
			}

			if (GetBool(InvertPhongMask))
			{
				ShaderDebugMessage("uses $InvertPhongMask.\n"
					"Without $PhongNewBehaviour this will actually flip the EnvMapMask instead, to reproduce the original Shaders Behaviour.\n");
			}

			if (GetBool(PhongAlbedoTint))
			{
				if (IsDefined(PhongTint))
				{
					float3 f3PhongTint = GetFloat3(PhongTint);
					if (f3PhongTint.x != 1.0f && f3PhongTint.y != 1.0f && f3PhongTint.z != 1.0f)
					{
						ShaderDebugMessage("uses $PhongAlbedoTint with $PhongTint.\n"
							"Without $PhongNewBehaviour this will be disabled $PhongAlbedoTint, to reproduce the original Shaders Behaviour.\n");
					}
				}

				if (!bHasPhongExponentTexture)
				{
					ShaderDebugMessage("uses $PhongAlbedoTint without $PhongExponentTexture.\n"
						"Without $PhongNewBehaviour this will be disabled, to reproduce the original Shaders Behaviour.\n"
						"The Original Shader requires you to have $PhongExponentTexture to be able to use $PhongAlbedoTint.\n");
				}

				// This is a Caveat introduced by CS:GO
				// The overall Bevaviour is replicated on LUX, so we don't break any existing Materials that might have been using it.
				if (IsDefined(PhongAlbedoBoost) && GetFloat(PhongAlbedoBoost) != 1.0f && IsDefined(Detail))
				{
					ShaderDebugMessage("uses $PhongAlbedoBoost with $Detail.\n"
						"Without $PhongNewBehaviour this will be disabled, a Caveat that LUX needs to replicate as it happens in CS:GO.\n"
						"If this is a CS:GO Material, ignore this Message. If its a new Material, use $PhongNewBehaviour.\n");
				}
			}

			// Only with New Behaviour, you may use the blue channel for masking phong.
			if (bHasPhongExponentTexture && GetBool(PhongExponentTextureMask))
			{
				ShaderDebugMessage("uses $PhongExponentTextureMask without $PhongNewBehaviour.\n"
					"This is a new Feature so you must use $PhongNewBehaviour.\n");
			}
		}
		else // Uses $PhongNewBehaviour
		{
			if (!bHasPhongExponentTexture && IsDefined(PhongExponentTextureMask) && GetBool(PhongExponentTextureMask))
			{
				ShaderDebugMessage("uses $PhongExponentTextureMask, but there is no $PhongExponentTexture to derive it from.\n");
			}

			if (HasFlag(MATERIAL_VAR_HALFLAMBERT) && IsDefined(PhongDisableHalfLambert) && GetBool(PhongDisableHalfLambert))
			{
				ShaderDebugMessage("uses $HalfLambert and $PhongDisableHalfLambert. $PhongDisableHalfLambert will take Priority.\n");
			}
		}

		if(bHasPhong && !bHasPhongExponentTexture && GetBool(RimMask))
			ShaderDebugMessage("uses $RimMask, but there is no $PhongExponentTexture to derive it from.\n");
	}
}

SHADER_INIT_PARAMS()
{
	// Let Developers know about potential Issues
	// This needs to happen first since we initialise Values afterwards and that makes them count as "defined"
	if (CVarDeveloper.GetInt() > 0)
		LuxVertexLitGeneric_ParamsDebugger();

	Cloak_Vars_t CloakVars;
	VLG_SetupCloakVars(CloakVars);
	CloakBlend_Init_Params(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	VLG_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Init_Params(this, EmissiveVars);

	SheenPass_Vars_t SheenVars;
	VLG_SetupSheenPassVars(SheenVars);
	SheenPass_Init_Params(this, SheenVars);

	// Always dealing with a Model!
	SetFlag(MATERIAL_VAR_MODEL);

	// We trick the engine into thinking we don't have a normal map. This is required for Model Lightmapping ( with BumpMaps )
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap))
	{
		// Needs this for ShaderInit
		SetBool(UsesBumpMap, true);

		bHasBumpMap = true;
		// If you have both $BumpMap and $normaltexture we use that as an indicator that you want model lightmapping
		if (IsDefined(NormalTexture))
		{
			SetString(NormalTexture, GetString(BumpMap));

			// 12.02.2023 ShiroDkxtro2 : NOTE, if you don't have data on $BumpMap, the engine will not send WorldLight data to the model!!! 
			SetUndefined(BumpMap);
		}
		else
		{
			SetString(NormalTexture, GetString(BumpMap));
		}
	}
	else
	{
		// If someone uses $NormalTexture now instead of $BumpMap, make sure $BumpMap has some kind of data, so we can actually get WorldLight data!
		// Note that Phong can be used with $BaseMapAlphaPhongMask and a default BumpMap is used for $LightWarpTexture without $LightWarpNoBump
		if (IsDefined(NormalTexture) || GetBool(Phong) && GetBool(BaseMapAlphaPhongMask) || IsDefined(LightWarpTexture) && !GetBool(LightWarpNoBump))
		{
			SetString(BumpMap, "..."); // Whats on $Bumpmap doesn't matter, it just has to... exist...
		}
	}

	// Only try to undefine if defined...
	if (IsDefined(LightWarpTexture) && mat_disable_lightwarp.GetBool())
	{
		SetUndefined(LightWarpTexture);
	}

	// Default Value is supposed to be 1.0f
	DefaultFloat(EnvMapSaturation, 1.0f);

	// If mat_specular 0, get rid of the EnvMap
	if (IsDefined(EnvMap))
	{
		if (!g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		{
			SetUndefined(EnvMap);
		}

		// Equation goes 0.5f * Sample * DiffuseModulation * Lighting
		// We ignore DiffuseModulation
		// So we only need to ensure that by default Lighting gets multiplied with the Sample
		// Set $EnvMapLightScale to 1.0f!!
		if (!IsDefined(NormalTexture) && !GetBool(Phong) && HasFlag(MATERIAL_VAR_ENVMAPSPHERE))
		{
			DefaultFloat(EnvMapLightScale, 1.0f);
		}

		if(lux_envmap_forcelerp.GetBool())
			SetBool(EnvMapLerp, true);

		// Disable EnvMapLerp if the Cubemap is NOT env_cubemap
		if(GetBool(EnvMapLerp) && V_stricmp(GetString(EnvMap), "env_cubemap"))
			SetBool(EnvMapLerp, false);
	}

	// Default Value is supposed to be 1.0f
	DefaultFloat(EnvMapSaturation, 1.0f);

	// Scale, Bias Exponent
	DefaultFloat3(EnvMapFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);
	DefaultFloat3(BaseAlphaEnvMapMaskMinMaxExp, 1.0f, 1.0f, 1.0f); // These Defaults will cause nothing to change by Default

	// Default Value is supposed to be 1.0f
	DefaultFloat(DetailBlendFactor, 1.0f);

	// Default Value is supposed to be 4.0f
	DefaultFloat(DetailScale, 4.0f);

	// Scale, Bias, Exponent.
	// This is a non-Stock Default Value. 1.0f for full Scale, 5.0f for Exponent
	DefaultFloat3(SelfIllumFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);

	// Funny issue we had: The Combine Elite's VMT uses $SelfIllumTint "[2 1 1]".
	// Caused the model to be rendered as pink...
	// Turns out, Valve forced a maskscale of 1.0 and I hadn't defaulted it at that point.
	// Apparently $SelfIllumMaskScale was a lie.
	// It doesn't actually exist on any of the stock shaders...
	// Thus Default Value has to be 1.0f. Otherwise you get 
	DefaultFloat(SelfIllumMaskScale, 1.0f);

	// No BaseTexture means you can't have any of these.
	if (!IsDefined(BaseTexture))
	{
		ClearFlag(MATERIAL_VAR_SELFILLUM);
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		ClearFlag(MATERIAL_VAR_ALPHATEST);
		SetBool(BaseMapAlphaPhongMask, false);
		SetBool(BaseMapLuminancePhongMask, false);
		SetBool(BlendTintByBaseAlpha, false);
		SetBool(BlendTintColorOverBase, false);
	}

	// L4D1 Common Infected Fix:
	// Can't reproduce the actual Feature but common Infected have broken Phong we can disable when detecting this Parameter.
	if(GetBool(ShinyBlood) && GetBool(Phong))
		SetBool(Phong, 0);

	if (IsDefined(Phong) && GetBool(Phong))
	{
		// g_pConfig says "No Phong"? No Phong it is!
		if (!g_pConfig->UsePhong())
			SetInt(Phong, 0);

		if (GetBool(BaseMapAlphaPhongMask) || bHasBumpMap || IsDefined(LightWarpTexture) && !GetBool(LightWarpNoBump))
		{
			// By Default, $BaseMapAlphaPhongMask forces a flat Normal even when $BumpMap is used.
			if (!GetBool(PhongNewBehaviour) && GetBool(BaseMapAlphaPhongMask))
				DefaultBool(PhongFlatNormal, true);

			// PhongFresnelRanges need this or Fresnel will be 0.0f
			DefaultFloat3(PhongFresnelRanges, 0.0f, 0.5f, 1.0f);

			// ShiroDkxtro2 Instruction Reduction :
			// On the Shader we'd do < $PhongExponentTexture.x * 149 + 1 >
			// On SDK2013MP this would be < $PhongExponentTexture.x * $PhongExponentFactor + 1 >
			// But we will do instead < $PhongExponentTexture.x * $PhongExponentFactor + $PhongExponent >
			// Without $PhongExponentTexture, this will just end up < $PhongExponent > on the Shader
			//
			// Stock Consistency : Override to $PhongExponent when its anything other than 0
			//
			// We use DefaultFloat's because maybe someone wants to do some really whacky stuff by combining Parameters 
			if (IsDefined(PhongExponent) && GetFloat(PhongExponent) > 0.0f)
			{
				DefaultFloat(PhongExponentFactor, 0.0f);
			}
			else if (IsDefined(PhongExponentTexture))
			{
				// Default Value is supposed to be ... Well in SDK2013mp it's 0...
				// It replaces the *149 of the calculation so that is what its default value SHOULD be
				DefaultFloat(PhongExponent, 1.0f);
				DefaultFloat(PhongExponentFactor, 149.0f);
			}
			else
			{
				// Default Value is supposed to be 5.0f
				DefaultFloat(PhongExponent, 5.0f);
			}
		}
	}

	// Treesway Implementation
	DefaultFloat(TreeSwayHeight, 1000.0f);
	DefaultFloat(TreeSwayStartHeight, 0.1f);
	DefaultFloat(TreeSwayRadius, 300.0f);
	DefaultFloat(TreeSwayStartRadius, 0.2f);
	DefaultFloat(TreeSwaySpeed, 1.0f);
	DefaultFloat(TreeSwaySpeedHighWindMultiplier, 2.0f);
	DefaultFloat(TreeSwayStrength, 10.0f);
	DefaultFloat(TreeSwayScrumbleSpeed, 5.0f);
	DefaultFloat(TreeSwayScrumbleStrength, 10.0f);
	DefaultFloat(TreeSwayScrumbleFrequency, 12.0f);
	DefaultFloat(TreeSwayFalloffExp, 1.5f);
	DefaultFloat(TreeSwayScrumbleFalloffExp, 1.0f);
	DefaultFloat(TreeSwaySpeedLerpStart, 3.0f);
	DefaultFloat(TreeSwaySpeedLerpEnd, 6.0f);
	DefaultFloat2(TreeSwayStaticValues, 0.5f, 0.5f);

	// Stock Consistency : Flip the $BaseAlphaEnvMapMask when not using $Phong or $BumpMap
	bool b1 = !IsDefined(BumpMap);
	bool b2 = !IsDefined(EnvMapMaskFlip);
	bool b3 = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	if (b1 && b2 && b3 && lux_envmap_flipbasealpha.GetBool())
	{
		SetBool(EnvMapMaskFlip, 1);
	}

	if (IsDefined(BumpMap))
	{
		// Required for dynamic Lighting
		SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
		SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
	}
}

SHADER_FALLBACK
{
#ifndef REPLACE_VERTEXLITGENERIC
	if (lux_oldshaders.GetBool())
		return "VertexLitGeneric";
#endif

	// Signed Distance Fields has its own dedicated Shader
	if (GetBool(DistanceAlpha))
		return "LUX_DistanceAlpha_Model";

	// Simple fallback for this Shader.
	if(GetBool(Seamless_Base) || GetBool(Seamless_Detail))
		return "LUX_Triplanar_Model";

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
	VLG_SetupCloakVars(CloakVars);
	CloakBlend_Shader_Init(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	VLG_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Shader_Init(this, EmissiveVars);

	SheenPass_Vars_t SheenVars;
	VLG_SetupSheenPassVars(SheenVars);
	SheenPass_Shader_Init(this, SheenVars);

	// Always needed...
	SetFlag(MATERIAL_VAR_MODEL);								// This has to be set here!!! Can't be set later
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);				// Required for skinning
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);				// Required for dynamic lighting
	SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);		// Required for ambient cube

	bool bBaseTextureIsTranslucent = false;
	if (IsDefined(BaseTexture))
	{
		LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);

		// IsTranslucent is not really reliable.
		if (GetTexture(BaseTexture)->IsTranslucent())
			bBaseTextureIsTranslucent = true;
	}

	// If the BaseTexture doesn't have an AlphaChannel, we can still use SelfIllum as long as it uses an alternate Method.
	bool bSelfIllum = HasFlag(MATERIAL_VAR_SELFILLUM);
	bool bHasSelfIllumMask = bSelfIllum && IsDefined(SelfIllumMask);
	bool bAlternativeSelfIllum = false;
	if (!bBaseTextureIsTranslucent)
	{
		bool bHasSelfIllumFresnel = bSelfIllum && GetBool(SelfIllumFresnel);
		bool bSelfIllumMask2 = bSelfIllum && GetBool(SelfIllum_EnvMapMask_Alpha);

		// Alternative Methods for SelfIllumination can still be used when the Basetexture has no Alpha Channel
		if (!bHasSelfIllumMask && !bSelfIllumMask2 && !bHasSelfIllumFresnel)
		{
			ClearFlag(MATERIAL_VAR_SELFILLUM);
		}

		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}
	else
		bAlternativeSelfIllum = GetBool(SelfIllumFresnel) || GetBool(SelfIllum_EnvMapMask_Alpha) || bHasSelfIllumMask;

	if (bHasSelfIllumMask)
		LoadTexture(SelfIllumMask,0);

	// Need to load this for the second Pass
	LoadTexture(SelfIllumTexture, TEXTUREFLAGS_SRGB);

	// Animated Texture Proxies will likely be running on $BumpMap
	// Load it, if $BumpMap was manually defined. That way it gets the Reference and Frame Params work.
	if (GetBool(UsesBumpMap))
		LoadBumpMap(BumpMap);

	LoadBumpMap(NormalTexture);

	if (IsDefined(Compress) && IsDefined(Stretch))
	{
		LoadTexture(Compress);
		LoadTexture(Stretch);
	}
	
	if (IsTextureLoaded(NormalTexture) && IsDefined(BumpStretch) && IsDefined(BumpCompress))
	{
		LoadTexture(BumpCompress);
		LoadTexture(BumpStretch);
	}

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	LoadTexture(LightWarpTexture, 0);

	// ShiroDkxtro2: $Lightmap appears to be totally dynamic.
	// This probably doesn't do anything, and won't allow for custom Lightmaps either.
	LoadTexture(Lightmap, 0);

	// SphereMap Support
	// Orange Box Code does this.
	// Unfortunately, it will result in Missing Textures when using "env_cubemap"
	// The 7th Face on a Cubemap will not end up here :/
	if (!IsDefined(NormalTexture) && !GetBool(Phong) && HasFlag(MATERIAL_VAR_ENVMAPSPHERE))
		LoadTexture(EnvMap, 0);
	else
		LoadCubeMap(EnvMap, 0);

	// This Block of if-Statements handles the Priority Chain of EnvMapMasks
	if (IsDefined(EnvMap))
	{
		LoadTexture(EnvMapMask);

		// $EnvMapMask has Priority over other Masking
		if (IsTextureLoaded(EnvMapMask))
		{
			// We already have an $EnvMapMask, so remove these!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		else
		{
			// NormalMapAlphaEnvMapMask takes priority, because its the go to one
			if (IsDefined(NormalTexture) && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
			{
				if (GetTexture(NormalTexture)->IsError())
					ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK); // No normal map, no masking.

				ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK); // If we use normal map alpha, don't use basetexture alpha.
			}
			else
			{
				if (IsDefined(BaseTexture) && GetTexture(BaseTexture) && GetTexture(BaseTexture)->IsError())
					ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK); // If we have no Basetexture, can't use its alpha.
			}
		}
	}

	if (GetBool(Phong))
	{
		LoadTexture(PhongWarpTexture);
		LoadTexture(PhongExponentTexture);
	}

	// No AlphaTest and BlendTintByBaseAlpha when we use the Alpha for something else.
	// Stock-Shader ignored that the $EnvMapMask's Alpha can be used for $SelfIllum...
	// 
	// IMPORTANT: || HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK) is not checked here
	// flatnose_truck.vmt ( A L4D Material ) uses $BaseAlphaEnvMapMask with $BlendTintByBaseAlpha
	// and that somehow works over there, so I'm excluding it from the Check here.
	if (HasFlag(MATERIAL_VAR_SELFILLUM) && !bAlternativeSelfIllum)
	{
		ClearFlag(MATERIAL_VAR_ALPHATEST);
		SetBool(BlendTintByBaseAlpha, false);
	}
}

// Virtual Void Override for Context Data
VertexLitGenericContext* CreateMaterialContextData() override
{
	return new VertexLitGenericContext(this);
}

// Exposed Function for Multipass Rendering
void LuxVertexLitGeneric_Shader_Draw(IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, CBasePerMaterialContextData** pContextDataPtr)
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<VertexLitGenericContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	// These don't have an Attachment to something else.
	bool bProjTex = HasFlashlight();
	int	 nTreeSway = GetInt(TreeSway);
	//	bool bIsDecal = HasFlag(MATERIAL_VAR_DECAL);
	bool bHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);

	// This used to be very unorderly. I sorted this by Texture now.
	// Let's start with BaseTexture Variables.
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);

	// All Parameters using the BaseTextures Alpha ( Except Phong ones )
	bool bSelfIllum = !bProjTex && HasFlag(MATERIAL_VAR_SELFILLUM);
	bool bBlendTintByBaseAlpha = GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = !bBlendTintByBaseAlpha && GetBool(DesaturateWithBaseAlpha);

	// Normal Map Variables
	bool bHasNormalTexture = IsTextureLoaded(NormalTexture);

	// Detail Texture Variables
	// 5 & 6 are now an additive Pass and will no longer be handled here.
	int  nDetailBlendMode = GetInt(DetailBlendMode);
	bool bHasDetailTexture = IsTextureLoaded(Detail) && (nDetailBlendMode != 5 && nDetailBlendMode != 6);

	// SelfIllum related Variables
	bool bHasSelfIllumMask = bSelfIllum && IsTextureLoaded(SelfIllumMask);
	bool bHasSelfIllumFresnel = bSelfIllum && GetBool(SelfIllumFresnel);

	// LightWarpTexture
	bool bHasLightWarpTexture = !bProjTex && IsTextureLoaded(LightWarpTexture); // No Lightwarp under the flashlight
	bool bHasLightWarpNoBump = bHasLightWarpTexture && GetBool(LightWarpNoBump);
	
	// Phong Variables, needed before EnvMap because of EnvMapSphere
	// NOTE: Phong can only be used with
	// A. $BumpMap
	// B. $BaseMapAlphaPhongMask
	// C. $LightWarpTexture
	bool bHasBaseMapAlphaPhongMask = GetBool(BaseMapAlphaPhongMask);
	bool bHasPhong = (bHasBaseMapAlphaPhongMask || bHasNormalTexture || bHasLightWarpTexture && !bHasLightWarpNoBump) && GetBool(Phong);
	bool bHasPhongExponentTexture = bHasPhong && IsTextureLoaded(PhongExponentTexture);
	bool bHasPhongWarpTexture = bHasPhong && IsTextureLoaded(PhongWarpTexture);
	bool bHasPhongNewBehaviour = bHasPhong && GetBool(PhongNewBehaviour);
	bool bHasRimLight = bHasPhong && GetBool(RimLight);
	bool bHasRimMask = bHasRimLight && bHasPhongExponentTexture && GetBool(RimMask); // Need PhongExponent for the Mask

	// Wrinklemapping
	bool bHasCompress = bHasPhong && IsTextureLoaded(Compress);
	bool bHasStretch = bHasPhong && IsTextureLoaded(Stretch);
	bool bHasBumpCompress = bHasPhong && bHasNormalTexture && IsTextureLoaded(BumpCompress);
	bool bHasBumpStretch = bHasPhong && bHasNormalTexture && IsTextureLoaded(BumpStretch);
	bool bWrinkleMappingBase = bHasBaseTexture && bHasCompress && bHasStretch; // Need both.
	bool bWrinkleMappingBump = bHasBumpCompress && bHasBumpStretch; // Need both.
	bool bAnyWrinkleMapping = bWrinkleMappingBase || bWrinkleMappingBump;

	// EnvMap Variables
	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bEnvMapSphere = bHasEnvMap && !bHasNormalTexture && !bHasPhong && HasFlag(MATERIAL_VAR_ENVMAPSPHERE);

	// We are on the BumpMapping Path if this is the case
	bool bBumpedShader = bHasPhong || bHasNormalTexture || bHasLightWarpTexture && !bHasLightWarpNoBump;
	bool bHasVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);

	bool bHasLightmapTexture = !bProjTex && IsTextureLoaded(Lightmap);

	//==========================================================================//
	// Pre-Snapshot Context Data Variables
	//==========================================================================//
	if(IsSnapshottingCommands())
	{
		pContextData->m_nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
		pContextData->m_bIsFullyOpaque = IsFullyOpaque(pContextData->m_nBlendType);
		pContextData->m_nEnvMapMode = bHasEnvMap + bHasEnvMapMask;

		// Evaluate the entire Phong Shenanigans and store it in the ContextData
		// ( Lots of branching )
		// Exception to this is Half-Lambert, it's a ConVar driven boolean Constant for Phong.

		// Stock Phong has EnvMapFresnel from the PhongFresnelRanges.
		if (bHasPhong && !bHasPhongNewBehaviour)
		{
			pContextData->m_bEnvMapFresnel = true;

			float3 f3PhongTint = GetFloat3(PhongTint);

			// Dirty Logic from the Stock Shader
			// Probably done because of Proxies not defining these Parameters.
			bool bHasPhongTint = (f3PhongTint.x != 1.0f) || (f3PhongTint.y != 1.0f) || (f3PhongTint.z != 1.0f);

			// Nothing weird here.
			pContextData->m_bPhong_BaseMapAlphaPhongMask = bHasBaseMapAlphaPhongMask;

			// PhongTint disables AlbedoTint. Only allowed with a PhongExponentTexture too ( For the Mask ).
			pContextData->m_bPhong_AlbedoTint = GetBool(PhongAlbedoTint) && bHasPhongExponentTexture && !bHasPhongTint;

			// Using $Detail disables $PhongAlbedoBoost
			pContextData->m_bPhong_AlbedoTintBoost = !bHasDetailTexture;

			// $InvertPhongMask didn't flip the PhongMask on Stock Shaders.
			// It flipped the EnvMapMask.. 
			pContextData->m_bPhong_InvertEnvMapMask = GetBool(InvertPhongMask);
			pContextData->m_bPhong_InvertPhongMask = false;

			// Stock Shader doesn't support this.
			pContextData->m_bPhong_PhongExponentTextureMask = false;
		}
		else if (bHasPhongNewBehaviour)
		{
			pContextData->m_bPhong_BaseMapAlphaPhongMask = bHasBaseMapAlphaPhongMask;
			pContextData->m_bPhong_AlbedoTint = GetBool(PhongAlbedoTint);
			pContextData->m_bPhong_AlbedoTintBoost = true;
			pContextData->m_bPhong_InvertEnvMapMask = GetBool(EnvMapMaskFlip);
			pContextData->m_bPhong_InvertPhongMask = GetBool(InvertPhongMask);
			pContextData->m_bPhong_PhongExponentTextureMask = GetBool(PhongExponentTextureMask);
		}
		else if (!bHasPhong && bHasEnvMap)
		{
			pContextData->m_bPhong_InvertEnvMapMask = GetBool(EnvMapMaskFlip);
			pContextData->m_bEnvMapFresnel = GetBool(EnvMapFresnel); // This is a float, not a bool..
		}

		// These Caveats are exclusive to $NormalMapAlphaEnvMapMask
		bool bBaseAlphaEnvMapMask = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		bool bNormalMapAlphaEnvMapMask = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		if (bHasPhong && !bHasPhongNewBehaviour && bNormalMapAlphaEnvMapMask)
		{
			// There is some whacky Behaviour using SelfIllumFresnel and $NMAEMM
			if (bHasSelfIllumFresnel)
			{
				// In this scenario Phong did "fEnvMapMask = lerp(BT.a, $InvertPhongMask, $NormalMapAlphaEnvMapMask)"
				// And then "lerp(EnvMapMask, 1-fEnvMapMask, $InvertPhongMask)"
				// Aka EnvMapMask = 0.0f
				pContextData->m_bEnvMapFresnel = false;
				pContextData->m_nEnvMapMode = 0;

				// Doesn't matter since we have no EnvMap here.
				pContextData->m_bBaseAlphaEnvMapMask = false;
				pContextData->m_bNormalMapAlphaEnvMapMask = false;
			}
			else
			{
				// Stock-Consistency:
				// Use BaseAlpha as EnvMapMask when using $BaseMapAlphaPhongMask with $NormalMapAlphaEnvMapMask
				pContextData->m_nEnvMapMode = bHasEnvMap;
				pContextData->m_bBaseAlphaEnvMapMask = bHasBaseMapAlphaPhongMask;
				pContextData->m_bNormalMapAlphaEnvMapMask = !bHasBaseMapAlphaPhongMask;
			}
		}
		else if (bHasPhong && !bHasPhongNewBehaviour)
		{
			// Set to true by Default, LUX Allows no Masking ( 1.0f ) so we need to enforce this
			// TF2C Demoman Cyclops for Example requires this
			pContextData->m_bBaseAlphaEnvMapMask = true;
			pContextData->m_bNormalMapAlphaEnvMapMask = false;
		}
		else
		{
			// Use what we actually want
			pContextData->m_bBaseAlphaEnvMapMask = bBaseAlphaEnvMapMask;
			pContextData->m_bNormalMapAlphaEnvMapMask = bNormalMapAlphaEnvMapMask;
		}
	}

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		// 1 means SelfIllum/SelfIllumMask. 2 is $SelfIllum_EnvMapMask_Alpha
		bool bSelfIllum_EnvMapMask_Alpha = bSelfIllum && bHasEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha);
		int nSelfIllumMode = bSelfIllum_EnvMapMask_Alpha ? 2 : bSelfIllum;

		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(pContextData->m_nBlendType);

		// We always need this
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We always want the Normal, even when we don't use it.
		// Otherwise we might get some Issues with too-thin Vertex Formats
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;

		// Not with Phong and Normal?
		// This seems a bit too generic
		if (bHasVertexColors)
			nFlags |= VERTEX_COLOR;

		// Always just one..
		int nTexCoords = 1;

		// Uncompressed Verts get Tangent + Binormal Sign through vUserData ( TANGENT Stream )
		int nUserDataSize = (bProjTex || bHasNormalTexture || bHasPhong) ? 4 : 0;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// Small Register Map since this Shader gets a bit cluttered...
		// ShiroDkxtro2: I moved these around a little bit to make this more organised
		// EnvMapMask is at the back here because PhongExponent will still be used under projected Textures
		// s0	- $BaseTexture
		// s1	- $Compress
		// s2	- $BumpMap
		// s3	- $Stretch
		// s4	- $Detail
		// s5	- $EnvMapMask
		// s6	- $LightWarpTexture
		// s7	- $PhongWarpTexture
		// s8	- $PhongExponentTexture
		// s9	- $BumpCompress
		// s10	- $BumpStretch
		// s11	- $Lightmap
		// s12	- $EnvMapLerp previous Cubemap
		// s13	- ProjTex / $SelfIllumMask
		// s14	- ProjTex / EnvMap 
		// s15	- ProjTex / GammaToLinear LUT

		// s0 - $BaseTexture
		// We always have a basetexture, and yes they should always be sRGB
		EnableSampler(SAMPLER_BASETEXTURE, true);

		// s1 - $Compress Map, always sRGB
		EnableSampler(bAnyWrinkleMapping, SHADER_SAMPLER1, true);

		// s2 - $BumpMap
		// Phong binds a Default Normal Map if there is no $BumpMap
		// LightWarp also does this, when $LightWarpNoBump isn't used.
		EnableSampler(bBumpedShader, SAMPLER_NORMALMAP, false);

		// s3 - $Stretch Map, always sRGB
		EnableSampler(bAnyWrinkleMapping, SHADER_SAMPLER3, true);

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false); // Stock Consistency, no sRGB read

		// s6 - $LightWarpTexture
		EnableSampler(bHasLightWarpTexture, SAMPLER_LIGHTWARP, false);

		// s7 - $PhongWarpTexture
		EnableSampler(bHasPhongWarpTexture, SAMPLER_PHONGWARP, false);

		// s8 - $PhongExponentTexture
		EnableSampler(bHasPhongExponentTexture, SAMPLER_PHONGEXPONENT, false);

		// s9 - $BumpCompress, only allowed with Base Wrinkle, not sRGB
		EnableSampler(bAnyWrinkleMapping, SHADER_SAMPLER9, false);

		// s10 - $BumpStretch, only allowed with Base Wrinkle, not sRGB
		EnableSampler(bAnyWrinkleMapping, SHADER_SAMPLER10, false);

		// s11 - $Lightmap
		if(!bHasPhong && !bHasNormalTexture)
			EnableSampler(SAMPLER_LIGHTMAP, false); // bHasLightmapTexture, 

		// s12 - Previous Envmap for $EnvMapLerp
		EnableSampler(bHasEnvMap && GetBool(EnvMapLerp), SHADER_SAMPLER12, !IsHDREnabled());

		// s13 - $SelfIllumMask
		EnableSampler(bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha), SAMPLER_SELFILLUM, false);

		// s14 - $EnvMap. sRGB when LDR.
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// s13, s14, s15 - Projected Texture Samplers
		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		// Normal Mapping Features were cut to their own Shader to improve compiletimes on the unbumped one
		// That way it will also be easier to add Normal Map specific Features..
		// Projected Texture always need a Shader without Lighting Shenanigans
		if (bProjTex)
		{
			// Can't have EnvMapMask ( EnvMap disabled with Projtex )
			int nNeededTexCoords = bBumpedShader + bHasDetailTexture;
			DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, bBumpedShader ? 2 : 1); // Need TBN or Normal
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
			SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		}
		else if (bBumpedShader)
		{
			int nNeededTexCoords =  bHasEnvMapMask + bHasDetailTexture;
			DECLARE_STATIC_VERTEX_SHADER(lux_model_bump_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(WRINKLEMAPS, bAnyWrinkleMapping);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
			SET_STATIC_VERTEX_SHADER(lux_model_bump_vs30);
		}
		else
		{
			int nNeededTexCoords =  bHasEnvMapMask + bHasDetailTexture;
			DECLARE_STATIC_VERTEX_SHADER(lux_model_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(SPECIALTEXCOORDS, bEnvMapSphere ? 2 : 0);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexColors);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, 0);
			SET_STATIC_VERTEX_SHADER(lux_model_vs30);
		}

		if (bProjTex)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTCOMBO, bBumpedShader + bHasPhong + bHasPhongExponentTexture); // Phong can be used without a BumpMap
			SET_STATIC_PIXEL_SHADER_COMBO(WRINKLEMAPS, bAnyWrinkleMapping);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
		}
		else if (bHasPhong)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_phong_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, pContextData->m_nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPLERP, bHasEnvMap && GetBool(EnvMapLerp));
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(EXPONENTTEXTURE, bHasPhongExponentTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(WRINKLEMAPS, bAnyWrinkleMapping);
			SET_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_phong_ps30);
		}
		else if (bBumpedShader)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_bump_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, pContextData->m_nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPLERP, bHasEnvMap && GetBool(EnvMapLerp));
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_bump_ps30);
		}
		else
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_simple_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, pContextData->m_nEnvMapMode + 2 * bEnvMapSphere);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPLERP, bHasEnvMap && GetBool(EnvMapLerp));
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_simple_ps30);
		}
	}

	//==========================================================================//
	// Post-Snapshot Context Data Static Commands
	//==========================================================================//
	if (IsSnapshottingCommands()) // IsSnapshottingCommands()
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
		if (!bHasBaseTexture)
		{
			if (bHasEnvMap)
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
			else
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_WHITE);
		}

		if (!bHasNormalTexture)
		{
			// LightWarp forces this without a BumpMap, $LightWarpNoBump allows using Vertex Lighting.
			if (bHasLightWarpTexture && !bHasLightWarpNoBump)
				SemiStaticCmds.BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);

			// This is allowed but we still use the Normal Map Sampler.
			else if (bHasBaseMapAlphaPhongMask)
				SemiStaticCmds.BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		}

		// When using regular SelfIllum the SelfIllumMask Sampler is on,
		// So we need to bind something to it.
		if (bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha) && !bHasSelfIllumMask)
			SemiStaticCmds.BindTexture(SAMPLER_SELFILLUM, TEXTURE_BLACK);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture
		if (bHasBaseTexture)
			SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);

		// s1 - $Compress
		if(bWrinkleMappingBase)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, Compress, Frame);
		else if(bWrinkleMappingBump)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, BaseTexture, Frame);

		// s2 - $BumpMap
		// Note that we use NormalTexture here, not $BumpMap.
		// This will be important later if we get an open-source Bumped Model Lightmapping Implementation
		if (bHasNormalTexture)
			SemiStaticCmds.BindTexture(SAMPLER_NORMALMAP, NormalTexture, BumpFrame);

		// s3 - $Stretch
		if (bWrinkleMappingBase)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER3, Stretch, Frame);
		else if (bWrinkleMappingBump)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER3, BaseTexture, Frame);

		// s4
		if (bHasDetailTexture)
			SemiStaticCmds.BindTexture(SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5
		if (bHasEnvMap && bHasEnvMapMask)
			SemiStaticCmds.BindTexture(SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s6
		if (bHasLightWarpTexture)
			SemiStaticCmds.BindTexture(SAMPLER_LIGHTWARP, LightWarpTexture, LightWarpTextureFrame);

		// s7, s8
		if (bHasPhong)
		{
			if (bHasPhongWarpTexture)
				SemiStaticCmds.BindTexture(SAMPLER_PHONGWARP, PhongWarpTexture, PhongWarpTextureFrame);

			if (bHasPhongExponentTexture)
				SemiStaticCmds.BindTexture(SAMPLER_PHONGEXPONENT, PhongExponentTexture, PhongExponentTextureFrame);
		}

		// s9 - $BumpCompress
		if (bWrinkleMappingBump)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER9, BumpCompress, BumpFrame);
		else if (bWrinkleMappingBase)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER9, NormalTexture, BumpFrame); // Fallback

		// s10 - $BumpStretch
		if (bWrinkleMappingBump)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER10, BumpStretch, BumpFrame);
		else if(bWrinkleMappingBase)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER10, NormalTexture, BumpFrame); // Fallback

		// s11
		// Lightmap needs to be bound dynamically, Instancing is done dynamically ( terrible ).

		// s13
		if (bHasSelfIllumMask)
			SemiStaticCmds.BindTexture(SAMPLER_SELFILLUM, SelfIllumMask, SelfIllumMaskFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// c1 - Modulation Constant
		bool bIsBrush = false;
		bool bApplySSBumpMathFix = false;
		float4 f4ModulationConstant = GetModulationConstant(bIsBrush, bApplySSBumpMathFix);
		SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_MODULATIONCONSTANTS, f4ModulationConstant);

		// c11
		SemiStaticCmds.SetPixelShaderConstant_EyePos(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12
		SemiStaticCmds.SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c13, c14, c15, c16, c17, c18
		if (!bProjTex && bBumpedShader)
			SemiStaticCmds.SetPixelShaderStateAmbientLightCube(LUX_PS_FLOAT_AMBIENTCUBE);

		// c20, c21, c22, c23, c24, c25
		if (!bProjTex && bBumpedShader)
			SemiStaticCmds.CommitPixelShaderLighting(LUX_PS_FLOAT_LIGHTDATA);

		// c32
		// XByBaseAlpha makes use of the same Constant, give it the right Parameter
		// Stock-Consistency: GammaToLinear Tint
		int nAlphaVar = bBlendTintByBaseAlpha ? BlendTintColorOverBase : DesaturateWithBaseAlpha;
		float4 f4BaseTextureTint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), nAlphaVar);
		f4BaseTextureTint.rgb = GammaToLinearTint(f4BaseTextureTint.rgb);
		SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4BaseTextureTint);

		// c33, c34
		if (bHasDetailTexture)
		{
			float4 f4Tint_Factor;
			f4Tint_Factor.rgb = GetFloat3(DetailTint);
			f4Tint_Factor.rgb = GammaToLinearTint(f4Tint_Factor.rgb); // Stock-Consistency: Gamma Tint
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4Tint_Factor);

			float4 f4Blendmode = 0.0f;
			f4Blendmode.x = (float)nDetailBlendMode;
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4Blendmode);
		}

		// c35, c36
		if (bSelfIllum)
		{
			// No SelfIllumMask means Alpha will be used for SelfIllumMask instead ( 0.0f )
			float4 f4SelfIllumTint_Scale;
			f4SelfIllumTint_Scale.rgb = GetFloat3(SelfIllumTint);
			f4SelfIllumTint_Scale.a = bHasSelfIllumMask ? GetFloat(SelfIllumMaskScale) : 0.0f;

			float4 f4SelfIllumFresnelTerms = 0.0f;
			if(GetBool(SelfIllumFresnel))
			{
				float3 f3ScaleBiasExp = GetFloat3(SelfIllumFresnelMinMaxExp);
				float f1Min = f3ScaleBiasExp.x;
				float f1Max = f3ScaleBiasExp.y;
				float f1Exp = f3ScaleBiasExp.z;

				// Stock-Consistency: Rearrange and add Brightness Term
				f4SelfIllumFresnelTerms.y = (f1Max != 0.0f) ? (f1Min / f1Max) : 0.0f;
				f4SelfIllumFresnelTerms.x = 1.0f - f4SelfIllumFresnelTerms.y;
				f4SelfIllumFresnelTerms.z = f1Exp;
				f4SelfIllumFresnelTerms.w = Max(f1Max, 0.0f);

				// This saves a multiply in the Shader
				f4SelfIllumTint_Scale.rgb *= f4SelfIllumFresnelTerms.w;
			}
			else
			{
				// SelfIllumFresnel is off, but we still compute it
				// This will disable it
				f4SelfIllumFresnelTerms.y = 1.0f;
			}

			// Always have to set both due to how Fresnel is setup in the Shader
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FACTORS, f4SelfIllumTint_Scale);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FRESNEL, f4SelfIllumFresnelTerms);
		}

		// c37, c38, c39, c40
		if (bHasEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.xyz = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.

			// Stock Consistency - Convert from Gamma to Linear
			// Not for Phong, because of course not
			if (!bHasPhong)
				f4EnvMapTint_LightScale.rgb = GammaToLinearTint(f4EnvMapTint_LightScale.rgb);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes, this *is* a float3 Parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// $EnvMapFresnelMinMaxExp
			float4 f4EnvMapFresnelRanges = 0.0f;
			if (pContextData->m_bEnvMapFresnel || bHasPhong)
			{
				if (pContextData->m_bEnvMapFresnel && !bHasPhong)
					f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
				else
					f4EnvMapFresnelRanges.x = GetFloat(EnvMapFresnel);
			}
			else
			{
				// EnvMapFresnel is off, but we still compute it
				// This will disable it
				f4EnvMapFresnelRanges.y = 1.0f;
			}

			// Always set this due to how EnvMapFresnel is setup in the Shader
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);

			// c41
			// ASW+ Feature for unbumped Shaders
			// Reusing EnvMap Position for this one
			if(!bBumpedShader)
			{
				float4 f4BaseAlphaParams = 0.0f;
				f4BaseAlphaParams.xyz = GetFloat3(BaseAlphaEnvMapMaskMinMaxExp);
				
				// Stock-Consistency: They wanted to replicate the 1-BaseAlpha Behaviour
				f4BaseAlphaParams.y -= f4BaseAlphaParams.x;

				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapFresnelRanges);
			}
		}

		// c41, c42, c43, c44
		// PCC. Not on this Shader.

		// c45, c46, c47, c48, c49
		if (bHasPhong)
		{
			// $PhongTint, $InvertPhongMask
			float4 f4PhongTint_InvertMask;
			f4PhongTint_InvertMask.xyz = GetFloat3(PhongTint);
			f4PhongTint_InvertMask.w = (float)pContextData->m_bPhong_InvertPhongMask;
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);

			// Get Fresnel Ranges for Phong and PhongExponentFactor
			// "Change fresnel range encoding from (min, mid, max) to ((mid-min)*2, mid, (max-mid)*2)"
			float4 f4PhongFresnelRanges_ExponentFactor;
			f4PhongFresnelRanges_ExponentFactor.xyz = GetFloat3(PhongFresnelRanges);
			f4PhongFresnelRanges_ExponentFactor.x = (f4PhongFresnelRanges_ExponentFactor.y - f4PhongFresnelRanges_ExponentFactor.x) * 2;
			f4PhongFresnelRanges_ExponentFactor.z = (f4PhongFresnelRanges_ExponentFactor.z - f4PhongFresnelRanges_ExponentFactor.y) * 2;
			f4PhongFresnelRanges_ExponentFactor.w = GetFloat(PhongExponentFactor);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_FRESNEL, f4PhongFresnelRanges_ExponentFactor);

			float4 f4PhongControls;
			f4PhongControls.x = pContextData->m_bPhong_AlbedoTintBoost ? GetFloat(PhongAlbedoBoost) : 1.0f;
			f4PhongControls.y = bHasRimLight ? GetFloat(RimLightExponent) : 0.0f;
			f4PhongControls.z = bHasRimLight ? GetFloat(RimLightBoost) : 0.0f;
			f4PhongControls.w = GetFloat(PhongExponent);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_CONTROLS, f4PhongControls);

			float4 f4MinimumLight_PhongBoost;
			f4MinimumLight_PhongBoost.rgb = GetFloat3(PhongMinimumLight);
			f4MinimumLight_PhongBoost.w = GetFloat(PhongBoost);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST, f4MinimumLight_PhongBoost);
		}

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nRegisterShift = 0;
		if(bBumpedShader)
		{
			bool bNormalTextureTransform = HasTransform(true, BumpTransform);
			if (bNormalTextureTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform);
			else
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform);

			nRegisterShift += 2;
		}

		if(bHasEnvMapMask)
		{
			bool bEnvMapMaskTransform = HasTransform(true, EnvMapMaskTransform);
			if (bEnvMapMaskTransform)
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, EnvMapMaskTransform);
			else
				SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform);

			nRegisterShift += 2;
		}

		if(bHasDetailTexture)
		{
			bool bDetailTextureTransform = HasTransform(true, DetailTextureTransform);
			if (bDetailTextureTransform)
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, DetailTextureTransform, DetailScale);
			else
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform, DetailScale);
		}

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s11 - $Lightmap
		// This is entirely Dynamic, so the Sampler is always enabled without bump and phong.
		// We MUST bind SOMETHING to it.
		if (!bHasPhong && !bHasNormalTexture)
		{
			if (bHasLightmapTexture)
				BindTexture(SAMPLER_LIGHTMAP, Lightmap);
			else
				BindTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);
		}

		// s14 - $EnvMap
		// EnvMapControls always dynamic because of EnvMapLerp
		if(bHasEnvMap)
		{
			float4 f4EnvMapControls = 0.0f;
			f4EnvMapControls.x = (float)pContextData->m_bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = (float)pContextData->m_bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = (float)pContextData->m_bPhong_InvertEnvMapMask;

			// Make really really sure ContextData is real here
			if(GetBool(EnvMapLerp) && pContextData)
			{
				// Initialise some kind of Cubemap Data
				if(!pContextData->m_bLerpLock && !pContextData->m_RefCubemapB.IsValid())
				{
					pContextData->m_RefCubemapA.Init(GetTexture(EnvMap));
					pContextData->m_RefCubemapB.Init(GetTexture(EnvMap));
				}

				// We aren't locked, check if the Cubemap changed
				ITexture* pCurrent = GetTexture(EnvMap);
				if(!pContextData->m_bLerpLock && pContextData->m_RefCubemapB != pCurrent)
				{
					// Store Reference to the current Cubemap in the ContextData
					pContextData->m_RefCubemapA.Init(GetTexture(EnvMap));

					// Lock.
					pContextData->m_bLerpLock = true;

					// Record the Target Time. Which is right now
					// Don't need more precision than a float
					pContextData->m_f1LerpStart = float(pShaderAPI->CurrentTime());
				}
				else if(pContextData->m_bLerpLock)
				{
					// Process the lerp Factor
					float f1CurrentTime = MAX(0, float(pShaderAPI->CurrentTime()) - pContextData->m_f1LerpStart - 2.0f);

					// Gives us a nice 0..1 Factor
					float f1TargetTime = saturate(f1CurrentTime / lux_envmap_lerptime.GetFloat());

					// If we reached the new Cubemap, unlock and set the previous Cubemap as the current Cubemap
					// Effectively reseting to the Start-Position of this System
					if (f1TargetTime >= 1.0f)
					{
						pContextData->m_bLerpLock = false;
						pContextData->m_RefCubemapB.Init(pContextData->m_RefCubemapA);
					}

					// Submit Target Time
					f4EnvMapControls.w = f1TargetTime;
				}
				else
				{
					f4EnvMapControls.w = 1.0f; // No Lerp
				}

				// FIXME: Macro
				BindTexture(SHADER_SAMPLER12, pContextData->m_RefCubemapB);
				BindTexture(SAMPLER_ENVMAPTEXTURE, pContextData->m_RefCubemapA);
			}
			else
				BindTexture(SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);
		}
			

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// VS c17, c18, c19, c20
		// World to View Matrix for Spheremapping
		if (bEnvMapSphere)
			LoadViewMatrixIntoVertexShaderConstant(LUX_VS_FLOAT_MV_MATRIX);

		// VS c48, c49, c50, c51, c52
		// Treesway Implementation
		if (nTreeSway != 0)
		{
			float4 f4SwayParams1; // c241
			f4SwayParams1.x = pShaderAPI->CurrentTime(); // f1Time
			f4SwayParams1.y = GetFloat(TreeSwayScrumbleFalloffExp);
			f4SwayParams1.z = GetFloat(TreeSwayFalloffExp);
			f4SwayParams1.w = GetFloat(TreeSwayScrumbleSpeed);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_01, f4SwayParams1);

			float4 f4SwayParams2; // c242
			f4SwayParams2.x = GetFloat(TreeSwaySpeedHighWindMultiplier);
			f4SwayParams2.y = f4SwayParams2.x * 2.14f; // Precompute this
			if (GetBool(TreeSwayStatic) || lux_treesway_force_static.GetBool())
			{
				if (lux_treesway_static_override.GetBool())
				{
					f4SwayParams2.z = lux_treesway_static_x.GetFloat();
					f4SwayParams2.w = lux_treesway_static_y.GetFloat();
				}
				else
					f4SwayParams2.zw = GetFloat2(TreeSwayStaticValues);
			}
			else
			{
				const Vector& windDir = pShaderAPI->GetVectorRenderingParameter(VECTOR_RENDERPARM_WIND_DIRECTION);
				f4SwayParams2.z = windDir.x;
				f4SwayParams2.w = windDir.y;
			}
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_02, f4SwayParams2);

			// We need to precompute these into something else
			float f1Height = GetFloat(TreeSwayHeight);
			float f1StartHeight = GetFloat(TreeSwayStartHeight);
			float f1Radius = GetFloat(TreeSwayRadius);
			float f1StartRadius = GetFloat(TreeSwayStartRadius);

			// Compute Multiplication for the whole object
			// Same for inverse height and reciprocal
			float4 f4SwayParams3;
			f4SwayParams3.x = f1Height * f1StartHeight;						// f1Height_MUL
			f4SwayParams3.y = 1.0f / ((1.0f - f1StartHeight) * f1Height);	// f1Height_RCP
			f4SwayParams3.z = f1Radius * f1StartRadius;						// f1Radial_MUL
			f4SwayParams3.w = 1.0f / ((1.0f - f1StartRadius) * f1Radius);	// f1Radial_RCP
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_03, f4SwayParams3);

			float4 f4SwayParams4; // c244
			f4SwayParams4.x = GetFloat(TreeSwaySpeed);
			f4SwayParams4.y = GetFloat(TreeSwayStrength);
			f4SwayParams4.z = GetFloat(TreeSwayScrumbleFrequency);
			f4SwayParams4.w = GetFloat(TreeSwayScrumbleStrength);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_04, f4SwayParams4);

			// More precomputation
			float4 f4SwayParams5; // c245
			f4SwayParams5.x = sqrtf(f4SwayParams2.z * f4SwayParams2.z + f4SwayParams2.w * f4SwayParams2.w); // Length of the Wind Direction
			float SpeedLerpLow = GetFloat(TreeSwaySpeedLerpStart);
			float SpeedLerpHigh = GetFloat(TreeSwaySpeedLerpEnd);
			f4SwayParams5.y = smoothstep(SpeedLerpLow, SpeedLerpHigh, f4SwayParams5.x);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_05, f4SwayParams5);
		}

		// PS c19
		// Need Luminance Weights in this Scenario
		if (bHasEnvMap || bDesaturateWithBaseAlpha || bHasLightmapTexture || bHasPhong && GetBool(BaseMapLuminancePhongMask))
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// Stock-Consistency: HalfLambert is on by Default
		// We allow overriding this with our ConVar,
		// $PhongDisableHalfLambert overrides *that* for existing Materials
		if (bHasPhong)
		{
			if (GetBool(PhongDisableHalfLambert))
				bHalfLambert = false;
			else if (lux_phong_defaulthalflambert.GetBool())
				bHalfLambert = true;

			// else it will be set to $HalfLambert. So that it can still be overriden by Materials.
		}

		// b0
		// Only have Dynamic Half-Lambert with BumpMapping
		if(bBumpedShader && bHalfLambert)
			BBools[LUX_PS_BOOL_HALFLAMBERT] = true;

		// b1
		if(bHasLightWarpTexture)
			BBools[LUX_PS_BOOL_LIGHTWARPTEXTURE] = true;

		// b4, b5, b6, b7, b8, b9, b10, b11
		if (bHasPhong)
		{
			// bHasBaseMapLuminancePhongMask overrides the mask used.
			bool bBaseMapLuminancePhongMask = GetBool(BaseMapLuminancePhongMask);
			BBools[LUX_PS_BOOL_PHONG_BASEMAPALPHAMASK] = pContextData->m_bPhong_BaseMapAlphaPhongMask && !bBaseMapLuminancePhongMask;
			BBools[LUX_PS_BOOL_PHONG_ALBEDOTINT] = pContextData->m_bPhong_AlbedoTint;
			BBools[LUX_PS_BOOL_PHONG_FLATNORMAL] = GetBool(PhongFlatNormal);
			BBools[LUX_PS_BOOL_PHONG_RIMLIGHTMASK] = bHasRimMask;
			BBools[LUX_PS_BOOL_PHONG_BASEMAPLUMINANCEMASK] = bBaseMapLuminancePhongMask;
			BBools[LUX_PS_BOOL_PHONG_EXPONENTTEXTUREMASK] = pContextData->m_bPhong_PhongExponentTextureMask;
			BBools[LUX_PS_BOOL_PHONG_RIMLIGHT] = bHasRimLight;
			BBools[LUX_PS_BOOL_PHONG_WARPTEXTURE] = bHasPhongWarpTexture;
		}

		// b12
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// b4
		// Vertex Shader Booleans
		if(!bBumpedShader)
		{
			BOOL BHalfLambert = bHalfLambert;
			pShaderAPI->SetBooleanVertexShaderConstant(LUX_VS_BOOL_HALFLAMBERT, &BHalfLambert);
		}

		// LightState is always fully Dynamic, and we always need it.
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		bool bHasStaticPropLighting = 0;
		bool bHasDynamicPropLighting = 0;

		// Dynamic Prop Lighting here refers to dynamic vertex lighting, or ambient cubes via the vertex shader
		// We shouldn't have that on bumped or phonged models. Same for Static Vertex Lighting
		if (!bProjTex && !bHasPhong && !bHasNormalTexture)
		{
			// LightState varies between SP and MP so we use a function to reinterpret
			bHasStaticPropLighting = StaticLightVertex(LightState);
			bHasDynamicPropLighting = (LightState.m_bAmbientLight || (LightState.m_nNumLights > 0)) ? 1 : 0;

			// Need to send this to the Vertex Shader manually in this scenario
			if (bHasDynamicPropLighting)
				pShaderAPI->SetVertexShaderStateAmbientLightCube();
		}

		if (bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		}
		else if (bBumpedShader)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_bump_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_bump_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, bHasStaticPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, bHasDynamicPropLighting);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
		}

		if (bProjTex)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
		}
		else if (bHasPhong)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_phong_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS_COMBO, LightState.m_nNumLights);
			SET_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_phong_ps30);
		}
		else if (bBumpedShader)
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_bump_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS_COMBO, LightState.m_nNumLights);
			SET_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_bump_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_simple_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bHasLightmapTexture);
			SET_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_simple_ps30);
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
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
#endif

#ifdef LUX_DEBUGCONVARS
		if (bHasNormalTexture && lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		}

		if (bHasLightmapTexture && lux_disablefast_lightmap.GetBool())
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
			if (lux_disablefast_phong.GetBool())
			{
				float4 f4PhongTint_InvertMask;
				f4PhongTint_InvertMask = 0.0f;
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);
			}
		}

		if (bSelfIllum && lux_disablefast_selfillum.GetBool())
		{
			float4 f4SelfIllumTint_Scale;
			f4SelfIllumTint_Scale.xyz = 0.0f;
			f4SelfIllumTint_Scale.w = 1.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_SELFILLUM_FACTORS, f4SelfIllumTint_Scale);

			BindTexture(SAMPLER_SELFILLUM, TEXTURE_BLACK);
		}

		if (bHasEnvMap && lux_disablefast_envmap.GetBool())
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);
		}

		if(bHasLightWarpTexture && lux_disablefast_lightwarp.GetBool())
		{
			BindTexture(SAMPLER_LIGHTWARP, TEXTURE_IDENTITY_LIGHTWARP);
		}
#endif
	}

	Draw();
}

SHADER_DRAW
{
	// Outline Support
	if (GetBool(MeshOutline_Enable))
	{
		Outline_Vars_t OutlineVars;
		OutlineVars.InitVars(MeshOutline_Enable);
		LuxOutlinePass_Draw(this, params, pShaderAPI, pShaderShadow, OutlineVars);
	}

	// Need to track whether or not we have drawn the BasePass
	bool bDrawBasePass = true;

	// Just always set this up
	Cloak_Vars_t CloakVars;
	VLG_SetupCloakVars(CloakVars);
	bool bCloakEnabled = GetBool(CloakVars.m_nCloakEnabled);

	// We want the regular Shader to be able to setup its Snapshot
	// Only do this without pShaderShadow
	if (bCloakEnabled && !pShaderShadow)
	{
		if (CloakBlend_IsOpaque(this, params, CloakVars))
			bDrawBasePass = false;
	}

	// Always need to Snapshot when pShaderShadow,
	// If we know there's a Spy Cloak, don't draw the BasePass
	// Don't bother to even render it
	if (pShaderShadow || bDrawBasePass)
	{
		LuxVertexLitGeneric_Shader_Draw(pShaderShadow, pShaderAPI, pContextDataPtr);
	}
	else
	{
		// We are cloaking, so stop doing the base pass
		// Otherwise the enemy team is going to cause a malfunction in our spy
		Draw(false);
	}

	if (pShaderShadow || bDrawBasePass)
	{
		EmissiveBlend_Vars_t EmissiveVars;
		VLG_SetupEmissiveBlendVars(EmissiveVars);
		EmissiveBlend_Shader_Draw(this, pShaderShadow, pShaderAPI, EmissiveVars);
	}
	else
	{
		int nDetailBlendMode = GetInt(DetailBlendMode);
		bool bAdditiveDetail = IsTextureLoaded(Detail) && IsSelfIllumDetailMode(nDetailBlendMode);
		bool bEmissiveBlend = GetBool(EmissiveBlendEnabled);
		bool bSelfIllumTexture = IsTextureLoaded(SelfIllumTexture);

		// Indicate we are not drawing anything in this case
		if (bAdditiveDetail || bEmissiveBlend || bSelfIllumTexture)
			Draw(false);
	}

	if (pShaderShadow || bDrawBasePass)
	{
		SheenPass_Vars_t SheenVars;
		VLG_SetupSheenPassVars(SheenVars);
		SheenPass_Shader_Draw(this, pShaderShadow, pShaderAPI, SheenVars);
	}
	else if (GetBool(SheenPassEnabled))
		Draw(false);

	// Only draw Spy Cloak if it's enabled
	// Should be drawn over anything else!
	if (bCloakEnabled)
	{
		float f1CloakFactor = GetFloat(CloakVars.m_nCloakFactor);

		// Snapshot if Enabled
		// If we are at a CloakFactor of 0, don't draw Anything. 100% Translucent
		// Is the < 1.0f redundant? This should always be normalized..
		if (pShaderShadow || ((f1CloakFactor > 0.0f) && (f1CloakFactor < 1.0f)))
			CloakBlend_Shader_Draw(this, pShaderShadow, pShaderAPI, CloakVars);
		else
			Draw(false); // The Engine expects us to tell it if we setup a Snapshot but don't want to draw anything.
	}
}
END_SHADER
