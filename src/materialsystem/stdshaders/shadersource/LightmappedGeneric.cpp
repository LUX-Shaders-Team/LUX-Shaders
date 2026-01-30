//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

#include "renderpasses/EmissiveBlend.h"

// Includes for Shaderfiles...
#include "lux_brush_simplified_vs30.inc"
#include "lux_brush_vs30.inc"
#include "lux_lightmappedgeneric_simple_ps30.inc"
#include "lux_lightmappedgeneric_bump_ps30.inc"
#include "lux_lightmappedgeneric_flashlight_ps30.inc"
#include "lux_lightmappedgeneric_phong_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_LightmappedGeneric,		LUX_LightmappedGeneric)
DEFINE_FALLBACK_SHADER(SDK_LightmappedGeneric_DX9,	LUX_LightmappedGeneric)
DEFINE_FALLBACK_SHADER(SDK_LightmappedGeneric_DX8,	LUX_LightmappedGeneric)
#endif

#ifdef REPLACE_LIGHTMAPPEDGENERIC
DEFINE_FALLBACK_SHADER(LightmappedGeneric,		LUX_LightmappedGeneric)
DEFINE_FALLBACK_SHADER(LightmappedGeneric_DX9,	LUX_LightmappedGeneric)
DEFINE_FALLBACK_SHADER(LightmappedGeneric_DX8,	LUX_LightmappedGeneric)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class LightmappedGenericContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	// Everything related to constants

	LightmappedGenericContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_LightmappedGeneric, "A general Purpose Brush Shader with support for Lightmapping.")
SHADER_INFO_GEOMETRY	("Brushes, Displacements, Overlays (Without $BumpMap or $EnvMap).")
SHADER_INFO_USAGE		("Materials need to be applied to a Brush or Displacement Surface. Or be applied as an Overlay to a Brush/Displacement Surface.\n"
						 "For Parallax-Corrected Cubemaps, the Map must be compiled with a VBSP that supports PCC.")
SHADER_INFO_LIMITATIONS("$Phong requires the Map to be compiled with $BumpMap on the Surface.\n"
						"$BumpMap and $EnvMap can not be used on Overlays. It is possible however using the LUX_LightmappedGeneric_Decal Shader.\n"
						"Does not inherently support Blended Textures. (See Fallbacks).")
SHADER_INFO_PERFORMANCE	("$Detail is relatively expensive.\n"
						 "$Phong is pretty cheap.\n"
						 "$SSBump's are cheaper than regular $BumpMap's.\n"
						 "$EnvMap pushes a lot of Code ($EnvMapTint, $EnvMapContrast, $EnvMapSaturation, EnvMapLightScale, etc).")
SHADER_INFO_FALLBACK	("Using $DistanceAlpha, $SoftEdges or $Outline fallbacks to LUX_DistanceAlpha_Brush shader.\n"
						 "Using $BaseTexture2 or $BumpMap2 fallbacks to LUX_WorldVertexTransition shader.\n"
						 "Using $Seamless_Scale fallbacks to LUX_Triplanar_Brush.\n"
						 "A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC LightmappedGeneric Shader Page: https://developer.valvesoftware.com/wiki/LightmappedGeneric")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(BaseTexture2, SHADER_PARAM_TYPE_STRING, "", "(FALLBACK) This Parameter will cause the Shader to fallback to WorldVertexTransition.")
	SHADER_PARAM(BumpMap2, SHADER_PARAM_TYPE_STRING, "", "(FALLBACK) This Parameter will cause the Shader to fallback to WorldVertexTransition.")
	Declare_NormalTextureParameters()
	Declare_SelfIlluminationParameters()
	Declare_PhongParameters()

	Declare_DetailTextureParameters()
	Declare_SelfIllumTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_ParallaxCorrectionParameters()
	Declare_SeamlessParameters()
	Declare_MiscParameters()

	SHADER_PARAM(DistanceAlpha,		SHADER_PARAM_TYPE_BOOL, "", "(FALLBACK) This Parameter will cause the Shader to fallback to LUX_DistanceAlpha_Brush.")
	SHADER_PARAM(SoftEdges,			SHADER_PARAM_TYPE_BOOL, "", "(FALLBACK) This Parameter will cause the Shader to fallback to LUX_DistanceAlpha_Brush.")
	SHADER_PARAM(Outline,			SHADER_PARAM_TYPE_BOOL, "", "(FALLBACK) This Parameter will cause the Shader to fallback to LUX_DistanceAlpha_Brush.")

	Declare_NoDiffuseBumpLighting()
	SHADER_PARAM(LinearWrite,		SHADER_PARAM_TYPE_BOOL,	"", "Disables SRGB conversion of Shader Results.")
	Declare_EmissiveBlendParameters()
END_SHADER_PARAMS

void LMG_SetupEmissiveBlendVars(EmissiveBlend_Vars_t &EmissiveVars)
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

SHADER_INIT_PARAMS()
{
	EmissiveBlend_Vars_t EmissiveVars;
	LMG_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Init_Params(this, EmissiveVars);

	// Only try to undefine if defined...
	if (mat_disable_lightwarp.GetBool() && IsDefined(LightWarpTexture))
	{
		SetUndefined(LightWarpTexture);
	}

	// Scale, Bias Exponent
	DefaultFloat3(EnvMapFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);
	DefaultFloat3(SelfIllumFresnelMinMaxExp, 1.0f, 0.0f, 5.0f);

	// Detail Texture
	DefaultFloat(DetailBlendFactor, 1.0f); 	// Default Value is supposed to be 1.0f
	DefaultFloat(DetailScale, 4.0f); 		// Default Value is supposed to be 4.0f

	// We force to 0 when no $SelfIllumMask is specified
	// The Default is 1.0f, so that when there is a dedicated $SelfIllumMask it won't be removed.
	DefaultFloat(SelfIllumMaskScale, 1.0f);

	// Can't use the imaginary Alpha Channel.
	if (!IsDefined(BaseTexture))
	{
		ClearFlag(MATERIAL_VAR_SELFILLUM);
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// We always want access to Lightmaps
	// If we have a $BumpMap, we want the Bumped one. ( Radiosity Normal Mapping )
	SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

	if (g_pConfig->UseBumpmapping() && IsDefined(BumpMap))
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);


	// Default Value is supposed to be 1.0f
	DefaultFloat(EnvMapSaturation, 1.0f);

	// If mat_specular is 0, get rid of the $EnvMap
	// On Valve's Shader they bind TEXTURE_BLACK instead of TEXTURE_WHITE, *if* the Material uses $EnvMap without a $BaseTexture.
	// It makes sense if all that is wanted is the $EnvMap.
	// White + $EnvMap doesn't go so well.
	if (IsDefined(EnvMap))
	{
		if (!g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		{
			SetUndefined(EnvMap);
			SetBool(EnvMapParallax, false);
		}

		// Support for $FresnelReflection
		// This isn't ideal as you cannot animate this with Proxies.
		// But it will reproduce the same looks as $FresnelReflection while allowing us to support MinMaxExp
		if(!IsDefined(EnvMapFresnelMinMaxExp) && IsDefined(FresnelReflection))
		{
			float3 f3MinMaxExp = 0.0f;
			f3MinMaxExp.x = 1.0f - f3MinMaxExp.y; // Scale
			f3MinMaxExp.y = GetFloat(FresnelReflection); // Bias
			f3MinMaxExp.z = 5.0f; // Pow of 5
			SetFloat3(EnvMapFresnelMinMaxExp, f3MinMaxExp);
		}
	}

	// Stock-Consistency
	// Tell the Shader to flip the EnvMap when set on AlphaEnvMapMask
	if (!IsDefined(EnvMapMaskFlip) && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
		SetBool(EnvMapMaskFlip, true);

	// ConVar for disabling Phong.
	if (!lux_lightmapped_phong_enable.GetBool())
	{
		SetBool(Phong, false);
	}
	else
	{
		// Force Enable
		// This must have a $BumpMap since Phong only works with Radiosity Normal Mapping.
		if (lux_lightmapped_phong_force.GetBool() && IsDefined(BumpMap))
		{
			SetBool(Phong, true);
		}
	}

	if (GetBool(Phong))
	{
		// ShiroDkxtro2:
		// This is a new Feature to this Shader
		// So I'm not hacking in Support for Valve created Shenanigans.
		// Abide by these Rules and we won't have a Spaghetti.
		if (!IsDefined(BumpMap) && CVarDeveloper.GetInt() > 0)
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

		// PhongFresnelRanges need this or Fresnel will be 0.0f
		DefaultFloat3(PhongFresnelRanges, 0.0f, 0.5f, 1.0f);

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
	}

	// Seamless -> TriPlanar Fallback
	// need to check the Float..
	if (GetFloat(Seamless_Scale) != 0.0f)
	{
		// Using Seamless_Scale will enable Seamless_Base
		SetBool(Seamless_Base, true);
	}
}

SHADER_FALLBACK	
{
#ifndef REPLACE_LIGHTMAPPEDGENERIC
	// This has to come before routing to WVT. Stock LMG includes WVT
	if (lux_oldshaders.GetBool())
		return "LightmappedGeneric";
#endif

	// Overlays and Decals to this Shader
	if (HasFlag(MATERIAL_VAR_DECAL))
		return "LUX_LightmappedGeneric_Decal";

	// Signed Distance Fields has its own dedicated Shader
	// NOTE: SoftEdges and Outline were the only Parameters for DistanceAlpha on LightmappedGeneric
	// So we are accounting for those here. This doesn't happen on UnlitGeneric and VertexLitGeneric.
	if (GetBool(DistanceAlpha) || GetBool(SoftEdges) || GetInt(Outline))
		return "LUX_DistanceAlpha_Brush";

	// ShiroDkxtro2: WVT Parameters are commonly used on LMG ( since it's the same Shader behind the Scenes )
	// This is no longer the Case anymore, so send them back to the dedicated WVT Shader
	if (IsDefined(BaseTexture2))
		return "LUX_WorldVertexTransition";

	// This could happen without $BaseTexture2 ( see HL2 EP2 ), so also send that to WVT
	if (IsDefined(BumpMap2))
		return "LUX_WorldVertexTransition";

	if(GetFloat(Seamless_Scale) != 0.0f)
		return "LUX_Triplanar_Brush";

	if (HasFlag(MATERIAL_VAR_MODEL))
	{
		Warning("Material Using LUX_LightmappedGeneric with $Model flag!\nRendering Wireframe instead...\n");
		return "Wireframe";
	}

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}

	return 0;
}

SHADER_INIT
{
	EmissiveBlend_Vars_t EmissiveVars;
	LMG_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Shader_Init(this, EmissiveVars);

	// Always needed...
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

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

	LoadBumpMap(BumpMap);

	// ~Stock-Consistency~
	// This is no longer done?
	bool bHasBumpMap = false;
	if (IsDefined(BumpMap) && IsTextureLoaded(BumpMap))
	{
		bHasBumpMap = true;

		// Stock-Consistency
		// If the User didn't specifically imply this is a $SSBump, check the TextureFlags
		if (!IsDefined(SSBump))
		{
			ITexture *pBumpMap = GetTexture(BumpMap);
			bool bIsSSBump = pBumpMap->GetFlags() & TEXTUREFLAGS_SSBUMP ? true : false;
			SetBool(SSBump, bIsSSBump);
		}
	}

	if (bHasBumpMap && GetBool(Phong))
	{
		LoadTexture(PhongExponentTexture);
		LoadTexture(PhongWarpTexture);
	}

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	if (IsDefined(Detail) && IsTextureLoaded(Detail))
	{
		// Portal 2's Panel Texture uses $DetailBlendMode 10 with an SSBump, however official Stock Shaders don't allow this.
		// It would only work if not a SSBump. LUX Adds support for it, so consider that!
		if (GetTexture(Detail)->GetFlags() & TEXTUREFLAGS_SSBUMP)
		{
			if (bHasBumpMap)
				SetInt(DetailBlendMode, 10);
			else
				SetInt(DetailBlendMode, 11);
		}
	}

	LoadTexture(LightWarpTexture);
	LoadTexture(SelfIllumMask);

	// Need to load this for the second Pass
	LoadTexture(SelfIllumTexture, TEXTUREFLAGS_SRGB);

	LoadCubeMap(EnvMap);

	// We don't want EnvMapMasking if we don't even have an EnvMap
	if (IsDefined(EnvMap))
	{
		// $EnvMapMask has Priority over other Masking Features
		if (IsDefined(EnvMapMask))
		{
			LoadTexture(EnvMapMask);

			// Nope!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		// NormalMapAlphaEnvMapMask has Priority over $BaseAlphaEnvMapMask
		else if (IsDefined(BumpMap) && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
		{
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}

		// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
		// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
		// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
		// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
		if(IsDefined(EnvMapParallaxOBB1) && !GetBool(EnvMapParallax))
			DefaultBool(EnvMapParallax, true);
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
LightmappedGenericContext* CreateMaterialContextData() override
{
	return new LightmappedGenericContext(this);
}

SHADER_DRAW
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<LightmappedGenericContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	// NOTE: We already ensured conflicting Flags have been accounted for on ParamInit and ShaderInit

	bool bInHammer = InHammer();
	bool bProjTex = HasFlashlight();
	bool bSelfIllum = !bProjTex && HasFlag(MATERIAL_VAR_SELFILLUM); // No SelfIllum under the flashlight

	// This used to be very unorderly. I sorted this by Texture now.
	// Let's start with BaseTexture Variables.
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);

	// All Parameters using the BaseTextures Alpha ( Except Phong ones )
	bool bBaseAlphaEnvMapMask = HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bBlendTintByBaseAlpha = bHasBaseTexture && GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = bHasBaseTexture && !bBlendTintByBaseAlpha && GetBool(DesaturateWithBaseAlpha);

	// Normal Map Variables
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);
	bool bHasSSBump = bHasNormalTexture && GetBool(SSBump);

	// All Parameters using the BumpMaps Alpha
	bool bNormalMapAlphaEnvMapMask = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);

	// Detail Texture Variables
	// 5 & 6 are now an additive Pass and will no longer be handled here.
	int  nDetailBlendMode = GetInt(DetailBlendMode);
	bool bHasDetailTexture = IsTextureLoaded(Detail) && (nDetailBlendMode != DETAILBLENDMODE_SELFILLUM_ADDITIVE && nDetailBlendMode != DETAILBLENDMODE_SELFILLUM_THRESHOLDFADE);

	// SelfIllum related Variables
	bool bHasSelfIllumMask = bSelfIllum && IsTextureLoaded(SelfIllumMask);

	// LightWarpTexture
	bool bHasLightWarpTexture = !bProjTex && IsTextureLoaded(LightWarpTexture); // No LightWarpTexture under projected Textures

	// Important difference here between Has and Use EnvMap
	// If we have an EnvMap we apply everything but if we don't USE it, we apply TEXTURE_BLACK instead
	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap); // No EnvMap under projected Textures
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bHasEnvMapFresnel = bHasEnvMap && GetBool(EnvMapFresnel);
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	// ShiroDkxtro2: Hammer++ does not use Bumped Lightmaps in the Lighting Preview!
	// Phong will look wrong without Radiosity Normal Mapping ( due to how it works ).
	// & I had several Reports from People that said it's "distracting", so we have to disable it.
	bool bHasPhong = !bInHammer && bHasNormalTexture && GetBool(Phong);
	bool bHasPhongExponentTexture = bHasPhong && IsTextureLoaded(PhongExponentTexture);
	bool bHasPhongWarpTexture = bHasPhong && IsTextureLoaded(PhongWarpTexture);

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
		// Purpose : Int to tell the Shader what Mask to use.
		// 0 = Nothing
		// 1 = $EnvMap - Mask determined through abs(0||1 - Mask)
		// 2 = $EnvMap + $EnvMapMask
		int nEnvMapMode = bHasEnvMap + bHasEnvMapMask + 2 * bPCC;

		// 1 = SelfIllum + SelfIllumMask
		// 2 = $SelfIllum_EnvMapMask_Alpha
		int nSelfIllumMode = (bHasEnvMapMask && GetBool(SelfIllum_EnvMapMask_Alpha)) ? 2 : bSelfIllum;

		//==========================================================================//
		// General Rendering Setup
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
		// Just always ask for Normal... You pretty much need it 99% of the time
		unsigned int nFlags = VERTEX_POSITION;

		// *sigh* Another Hammer specific Issue
		// Vertex Colors are used for Shaded Texture Polygons..
		if (HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			nFlags |= VERTEX_COLOR;

		// EnvMap wants Tangents, ProjTex always needs Normals
		if(bProjTex || bHasEnvMap || bHasPhong)
			nFlags |= VERTEX_NORMAL;

		// Normal Maps don't require the TBN Matrix actually.
		// ( Due to how Radiosity Normal Mapping works )
		// Projected Textures and EnvMaps do
		if (bProjTex && bHasNormalTexture || bHasEnvMap || bHasPhong)
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

		// s0 - $BaseTexture. Always sRGB
		EnableSampler(SAMPLER_BASETEXTURE, true);

		// s2 - $BumpMap. Never sRGB
		EnableSampler(bHasNormalTexture, SAMPLER_NORMALMAP, false);

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask. Not sRGB.
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false); // Stock Consistency, no sRGB

		// s6 - $LightWarpTexture. Not sRGB despite being a Color Texture
		EnableSampler(bHasLightWarpTexture, SAMPLER_LIGHTWARP, false);

		// s7 - $PhongWarpTexture. Not sRGB
		EnableSampler(bHasPhong, SAMPLER_PHONGWARP, false);

		// s8 - $PhongExponentTexture. Not sRGB
		EnableSampler(bHasPhongExponentTexture, SAMPLER_PHONGEXPONENT, false);

		// s11 - Lightmap. sRGB when LDR
		EnableSampler(!bProjTex, SAMPLER_LIGHTMAP, !IsHDREnabled());

		// s13 - $SelfIllumMask. Not sRGB
		EnableSampler(bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha), SAMPLER_SELFILLUM, false);

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		int nNeededTexCoords = bHasNormalTexture + bHasEnvMapMask + bHasDetailTexture;
		bool bVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR);
		if(bProjTex)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);		
			SET_STATIC_VERTEX_SHADER_COMBO(NOMODELMATRIX, 0);	
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, bHasNormalTexture ? 2 : 1);		
			SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, 0);	
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bVertexColors);	
			SET_STATIC_VERTEX_SHADER(lux_brush_simplified_vs30);
		}
		else
		{
			// Phong always needs Bumped Lightmaps. Otherwise it's optional under $NoDiffuseBumpLighting
			bool bNeedsBumpedLightmap = bHasNormalTexture && !GetBool(NoDiffuseBumpLighting);
			bNeedsBumpedLightmap = bNeedsBumpedLightmap || bHasPhong;

			DECLARE_STATIC_VERTEX_SHADER(lux_brush_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bHasNormalTexture && (bHasEnvMap || bHasPhong));
			SET_STATIC_VERTEX_SHADER_COMBO(BUMPED_LIGHTMAP, bNeedsBumpedLightmap);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bVertexColors);
			SET_STATIC_VERTEX_SHADER(lux_brush_vs30);
		}

		if (bProjTex)
		{
			int nLightingMode = bHasNormalTexture + bHasSSBump;
			nLightingMode += bHasPhong * 2 + bHasPhongExponentTexture * 2; // Compacted the Statics even more using this
			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTING_MODE, nLightingMode);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else if (bHasPhong)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_phong_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasSSBump);
			SET_STATIC_PIXEL_SHADER_COMBO(EXPONENTTEXTURE, bHasPhongExponentTexture);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_phong_ps30);
		}
		else if (bHasNormalTexture)
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_bump_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(SSBUMP, bHasSSBump);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_bump_ps30);
		}
		else
		{
			DECLARE_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_simple_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMMODE, nSelfIllumMode);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER(lux_lightmappedgeneric_simple_ps30);
		}
	}

	//==========================================================================//
	// Post-Snapshot Context Data Static Commands
	//==========================================================================//
	if (IsSnapshottingCommands())
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
	SEMI_STATIC_COMMANDS
	{
		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);
		
		//==========================================================================//
		// Bind StandardTextures
		//==========================================================================//

		// White Texture for LightmappedGeneric, unless we have an EnvMap
		if (!bHasBaseTexture)
		{
			if (bHasEnvMap)
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
			else
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_WHITE);
		}

		// When using regular SelfIllum the SelfIllumMask Sampler is on,
		// So we need to bind something to it.
		if (bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha) && !bHasSelfIllumMask)
			SemiStaticCmds.BindTexture(SAMPLER_SELFILLUM, TEXTURE_BLACK);

		// We enable the PhongWarp Sampler when we have Phong
		// So we need to bind something to it.
		if (bHasPhong && !bHasPhongWarpTexture)
			SemiStaticCmds.BindTexture(SAMPLER_PHONGWARP, TEXTURE_BLACK);

		// Always without projTex
		if(!bProjTex)
			SemiStaticCmds.BindTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0
		if(bHasBaseTexture)
			SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);

		// s2
		if (bHasNormalTexture)
			SemiStaticCmds.BindTexture(SAMPLER_NORMALMAP, BumpMap, BumpFrame);

		// s4
		if (bHasDetailTexture)
			SemiStaticCmds.BindTexture(SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		// s5
		if (bHasEnvMap && bHasEnvMapMask)
			SemiStaticCmds.BindTexture(SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s6
		if(bHasLightWarpTexture)
			SemiStaticCmds.BindTexture(SAMPLER_LIGHTWARP, LightWarpTexture, LightWarpTextureFrame);

		// s7, s8
		if (bHasPhong)
		{
			if (bHasPhongWarpTexture)
				SemiStaticCmds.BindTexture(SAMPLER_PHONGWARP, PhongWarpTexture, PhongWarpTextureFrame);

			if (bHasPhongExponentTexture)
				SemiStaticCmds.BindTexture(SAMPLER_PHONGEXPONENT, PhongExponentTexture, PhongExponentTextureFrame);
		}

		// s13
		if(bHasSelfIllumMask)
			SemiStaticCmds.BindTexture(SAMPLER_SELFILLUM, SelfIllumMask, SelfIllumMaskFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// c1 - Modulation Constant
		// ASW has this Comment regarding LightmapScaleFactor.
		// "Also note that if the lightmap scale factor changes
		// all shadow state blocks will be re-run, so that's ok"
		// So don't need to worry about LightmapScaleFactor,
		// different for $color2, Proxies won't caues a Command Buffer refresh.
		bool bIsBrush = true;
		bool bApplySSBumpMathFix = bHasSSBump && GetBool(SSBumpMathFix);
		float4 f4ModulationConstant = GetModulationConstant(bIsBrush, bApplySSBumpMathFix);
		SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_MODULATIONCONSTANTS, f4ModulationConstant);

		// c11 - Camera Position
		SemiStaticCmds.SetPixelShaderConstant_EyePos(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		SemiStaticCmds.SetPixelShaderConstant_FogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c32
		// XByBaseAlpha makes use of the same Constant, give it the right Parameter
		int nAlphaVar = bBlendTintByBaseAlpha ? BlendTintColorOverBase : DesaturateWithBaseAlpha;
		float4 f4BaseTextureTint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), nAlphaVar);
		SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4BaseTextureTint);

		// c33, c34
		if (bHasDetailTexture)
		{
			float4 f4Tint_Factor;
			f4Tint_Factor.xyz = GetFloat3(DetailTint);
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

		// c37, c38, c39, c40, c41, c42, c43, c44
		if (bHasEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes, this *is* a float3 Parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// $BaseAlphaEnvMapMask, $NormalMapAlphaEnvMapMask, $EnvMapMaskFlip
			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip); // applied as abs($EnvMapMaskFlip - EnvMapMask)
			f4EnvMapControls.w = 0.0f;
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// $EnvMapFresnelMinMaxExp
			float4 f4EnvMapFresnelRanges = 0.0f;
			if (bHasEnvMapFresnel)
			{
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
			}
			else
			{
				// EnvMapFresnel is off, but we still compute it
				// This will disable it
				f4EnvMapFresnelRanges.y = 1.0f;
			}
			
			// Always set this due to how EnvMapFresnel is setup in the Shader
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);

			// $EnvMapOrigin, $EnvMapParallaxOBB1, $EnvMapParallaxOBB2, $EnvMapParallaxOBB3
			if(bPCC)
			{
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX, f4Row1);
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_2, f4Row2);
				SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_3, f4Row3);
			}
		}

		// c45, c46, c47, c48
		if (bHasPhong)
		{
			float4 f4PhongTint_InvertMask;
			f4PhongTint_InvertMask.rgb = GetFloat3(PhongTint);
			f4PhongTint_InvertMask.w = (float)GetBool(InvertPhongMask);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);

			// Get Fresnel Ranges for Phong and PhongExponentFactor
			float4 f4PhongFresnelRanges_ExponentFactor;
			f4PhongFresnelRanges_ExponentFactor.xyz = GetFloat3(PhongFresnelRanges);
			f4PhongFresnelRanges_ExponentFactor.x = (f4PhongFresnelRanges_ExponentFactor.y - f4PhongFresnelRanges_ExponentFactor.x) * 2.0f;
			f4PhongFresnelRanges_ExponentFactor.z = (f4PhongFresnelRanges_ExponentFactor.z - f4PhongFresnelRanges_ExponentFactor.y) * 2.0f;
			f4PhongFresnelRanges_ExponentFactor.w = GetFloat(PhongExponentFactor);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_FRESNEL, f4PhongFresnelRanges_ExponentFactor);

			// No RimLighting on Brushes
			float4 f4PhongControls;
			f4PhongControls.r = GetFloat(PhongAlbedoBoost);
			/*
			if (bHasRimLight)
			{
				f4PhongControls.y = GetFloat(RimLightExponent);
				f4PhongControls.z = GetFloat(RimLightBoost);
			}
			*/
			f4PhongControls.w = GetFloat(PhongExponent);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_CONTROLS, f4PhongControls);

			// PhongBoost packed separately
			float4 f4MinLight_PhongBoost = 0.0f;
			f4MinLight_PhongBoost.w = GetFloat(PhongBoost);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST, f4MinLight_PhongBoost);
		}

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nRegisterShift = 0;
		if(bHasNormalTexture)
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

		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Need Luminance Weights in this Scenario
		if (bHasEnvMap || bDesaturateWithBaseAlpha || bHasPhong && GetBool(BaseMapLuminancePhongMask))
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b1
		if(bHasLightWarpTexture)
			BBools[LUX_PS_BOOL_LIGHTWARPTEXTURE] = true;

		// b4, b5, b6, b7, b8, b9, b10, b11
		if (bHasPhong)
		{
			BBools[LUX_PS_BOOL_PHONG_BASEMAPALPHAMASK] = GetBool(BaseMapAlphaPhongMask) && !GetBool(BaseMapLuminancePhongMask); // bHasBaseMapLuminancePhongMask overrides the mask used.
			BBools[LUX_PS_BOOL_PHONG_ALBEDOTINT] = GetBool(PhongAlbedoTint);
			BBools[LUX_PS_BOOL_PHONG_FLATNORMAL] = false; // No Flat Normals on Brushes!
			BBools[LUX_PS_BOOL_PHONG_RIMLIGHTMASK] = false; // No RimLightMask on Brushes!!
			BBools[LUX_PS_BOOL_PHONG_BASEMAPLUMINANCEMASK] = GetBool(BaseMapLuminancePhongMask);
			BBools[LUX_PS_BOOL_PHONG_EXPONENTTEXTUREMASK] = GetBool(PhongExponentTextureMask);
			BBools[LUX_PS_BOOL_PHONG_RIMLIGHT] = false; // No RimLighting on Brushes!!
			BBools[LUX_PS_BOOL_PHONG_WARPTEXTURE] = bHasPhongWarpTexture;
		}

		// b12
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque); // Heightfog instead of Range/Radial Fog
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		// Always set Boolean Registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		if (bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_simplified_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);

			bool bBicubic = r_lightmap_bicubic.GetBool();
			
			if (bHasPhong)
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_phong_ps30);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, bBicubic);
				SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_phong_ps30);				
			}
			else if (bHasNormalTexture)
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_bump_ps30);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, bBicubic);
				SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_bump_ps30);
			}		
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_simple_ps30);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(BICUBIC_FILTERING, bBicubic);
				SET_DYNAMIC_PIXEL_SHADER(lux_lightmappedgeneric_simple_ps30);
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
		if (bHasNormalTexture && lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
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
			// We have to force the Values here.
			// IsDefined() will be set when we set a default Value
			if (lux_lightmapped_phong_force.GetBool() && !lux_disablefast_phong.GetBool())
			{
				float4 f4PhongTint_InvertMask;
				f4PhongTint_InvertMask.xyz = lux_lightmapped_phong_force_boost.GetFloat();
				f4PhongTint_InvertMask.w = 0.0f;
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);

				if (!bHasPhongExponentTexture)
				{
					float4 f4PhongControls;
					f4PhongControls.xyz = 0.0f;
					f4PhongControls.w = lux_lightmapped_phong_force_exp.GetFloat();
					pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_CONTROLS, f4PhongControls);
				}
			}

			if (lux_disablefast_phong.GetBool())
			{
				float4 f4PhongTint_InvertMask;
				f4PhongTint_InvertMask = 0.0f;
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);
			}
		}

		if (bSelfIllum && !GetBool(SelfIllum_EnvMapMask_Alpha) && lux_disablefast_selfillum.GetBool())
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

#ifdef DEBUG_LUXELS
		if (mat_luxels.GetBool())
		{
			BindTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
		}
#endif

#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
#endif
	}

	Draw();

	// ShiroDkxtro2: What about Translucent Materials and other AlphaBlends?
	// They should have the same Problem of not being able to write to DestAlpha,
	// due to AlphaWrites being disabled for them. They also can't write regular Depth.
	if (HasFlag(MATERIAL_VAR_ALPHATEST))
	{
		// "Alpha testing makes it so we can't write to dest alpha
		// Writing to depth makes it so later polygons can't write to dest alpha either
		// This leads to situations with garbage in dest alpha."
		// "Fix it now by converting depth to dest alpha for any pixels that just wrote."
		DrawEqualDepthToDestAlpha();
	}

	EmissiveBlend_Vars_t EmissiveVars;
	LMG_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Shader_Draw(this, pShaderShadow, pShaderAPI, EmissiveVars);
}
END_SHADER