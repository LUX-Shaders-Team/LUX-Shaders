//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_teeth_vs30.inc"
#include "lux_teeth_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Teeth, LUX_Teeth)
DEFINE_FALLBACK_SHADER(SDK_Teeth_DX9, LUX_Teeth)
DEFINE_FALLBACK_SHADER(SDK_Teeth_DX8, LUX_Teeth)
DEFINE_FALLBACK_SHADER(SDK_Teeth_DX6, LUX_Teeth)
#endif 

#ifdef REPLACE_TEETH
DEFINE_FALLBACK_SHADER(Teeth,		LUX_Teeth)
DEFINE_FALLBACK_SHADER(Teeth_DX9,	LUX_Teeth)
DEFINE_FALLBACK_SHADER(Teeth_DX6,	LUX_Teeth)
DEFINE_FALLBACK_SHADER(Teeth_DX8,	LUX_Teeth)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class TeethContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	bool m_bHalfLambert = true;

	// Intended for Debugging at some Point
	// Should also be here since we have to evaluate this at the same Time,
	// as the Phong Variables below.
	int m_nEnvMapMode = 0;
	int m_nbBaseAlphaEnvMapMask = false;
	int m_nbNormalMapAlphaEnvMapMask = false;

	// This is for Phong.
	// We need a Cargoship worth of Logic to make sure Stock Materials look the same.
	bool m_bEnvMapFresnel = false;
	bool m_bPhong_BaseMapAlphaPhongMask = false;
	bool m_bPhong_AlbedoTint = false;
	bool m_bPhong_AlbedoTintBoost = false;
	bool m_bPhong_InvertEnvMapMask = false;
	bool m_bPhong_InvertPhongMask = false;
	bool m_bPhong_PhongExponentTextureMask = false;

	TeethContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Teeth, "A shader for teeth that is used to dynamically adjust the brightness of the teeth based on how open the mouth is.")
SHADER_INFO_GEOMETRY	("Models.")
SHADER_INFO_USAGE		("Create a model and apply the Material to the model.")
SHADER_INFO_LIMITATIONS	("$SelfIllum is not supported.-No glowing Teeth.")
SHADER_INFO_PERFORMANCE	("Cheap to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Teeth Shader Page: https://developer.valvesoftware.com/wiki/Teeth")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_NormalTextureParameters();
	Declare_EnvironmentMapParameters();
	Declare_EnvMapMaskParameters();
	Declare_PhongParameters();
	Declare_RimLightParameters();
	Declare_DetailTextureParameters();
	Declare_MiscParameters();
	SHADER_PARAM(IllumFactor,	SHADER_PARAM_TYPE_FLOAT, "", "Modulates the Amount that the Teeth are brightened or darkened by.\nDefault 1. ( Usually set by Proxies ).");
	SHADER_PARAM(Forward,		SHADER_PARAM_TYPE_VEC3,	 "", "Forward Direction for Teeth Lighting. ( Usually set by Proxies ).");
	SHADER_PARAM(Intro,			SHADER_PARAM_TYPE_BOOL,	 "", "Activates a special Variant used for the Episode 1 Intro. Adds another Shader Pass.");
	SHADER_PARAM(EntityOrigin,	SHADER_PARAM_TYPE_VEC3,	 "", "Requires $Intro 1. World-space location of the entity, required to correctly animate the Warp.");
	SHADER_PARAM(WarpParam,		SHADER_PARAM_TYPE_FLOAT, "", "Requires $Intro 1. How far into the Warp Animation we are.\nAnimation Parameter with a Range of 0 to 1.");
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// Always used on Models
	SetFlag(MATERIAL_VAR_MODEL);

	// Always, since this Shader's main purpose is to be used on Character Models that usually have Skinning.
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

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

//	DefaultInt(Intro, 0);
	
	// Only used when there is a $BumpMap
	// Stock Consistency : Phong Enabled by default when using $BumpMap
	// $BumpMap Alpha acts as PhongExponent unless $PhongExponent is above 0
	DefaultInt(Phong, 1);
	DefaultInt(BaseMapAlphaPhongMask, 1); // The Default!

	// Debugging Shenanigans and Default Stuff for Phong
	bool bUsesNewBehaviour = IsDefined(PhongNewBehaviour) && GetBool(PhongNewBehaviour);
	bool bHasPhongExponentTexture = IsDefined(PhongExponentTexture);
	if (CVarDeveloper.GetInt() > 0 && IsDefined(BumpMap))
	{
		// Phong related Caveats
		if (GetBool(Phong)) // $Phong on by default under the BumpMap
		{
			// All caveats that will be replicated when not under new behaviour should be warned about...
			if (!bUsesNewBehaviour)
			{
				// InvertPhongMask now actually does what its supposed to, so tell the user that old Materials will not look like they should
				if (IsDefined(InvertPhongMask) && GetBool(InvertPhongMask))
				{
					ShaderDebugMessage("uses InvertPhongMask, on the Stock Shader this would only flip the EnvMap... So thats reproduced by using $EnvMapMaskFlip \n"
					"You can use the 'correct' behaviour using $PhongNewBehaviour,\n $InvertPhongMask will flip the PhongMask and $EnvMapMaskFlip flips the EnvMapMask.\n");
				}

				// PhongAlbedoTint is disabled when using PhongTint without New Behaviour
				if (IsDefined(PhongTint) && IsDefined(PhongAlbedoTint) && GetBool(PhongAlbedoTint))
				{
					float3 f3PhongTint = GetFloat3(PhongTint);
					if (f3PhongTint[0] != 1.0f && f3PhongTint[1] != 1.0f && f3PhongTint[2] != 1.0f)
					{
						ShaderDebugMessage("uses $PhongAlbedoTint and PhongTint. PhongAlbedoTint will be disabled. This is fixed when using $PhongNewBehaviour! \n");
					}
				}

				// The following two caveats are related to PhongAlbedoTint
				if (IsDefined(PhongAlbedoTint) && GetBool(PhongAlbedoTint))
				{
					// PhongAlbedoTint can only be used with PhongExponentTexture
					if (!bHasPhongExponentTexture)
					{
						ShaderDebugMessage("wants to use $PhongAlbedoTint but has no $PhongExponentTexture. It may be used without under $PhongNewBehaviour. Without it, $PhongAlbedoTint will be disabled. \n\n");
					}

					// This is a caveat introduced in CS:GO, we replicate the bevaviour to not break existing Materials
					if (IsDefined(PhongAlbedoBoost) && GetFloat(PhongAlbedoBoost) != 1.0f && IsDefined(Detail))
					{
						ShaderDebugMessage(" has $PhongAlbedoBoost and $Detail, this is a Caveat that happens in CS:GO. PhongAlbedoBoost will be disabled. Use $PhongNewBehaviour to fix the issue. \n\n");
					}
				}

				// Only with New Behaviour, you may use the blue channel for masking phong.
				if (bHasPhongExponentTexture && IsDefined(PhongExponentTextureMask) && GetBool(PhongExponentTextureMask))
				{
					ShaderDebugMessage("has $PhongExpontentTextureMask but does not use $PhongNewBehaviour. The Parameter will not do anything. \n\n");
				}
				else if (!bHasPhongExponentTexture && IsDefined(PhongExponentTextureMask) && GetBool(PhongExponentTextureMask))
				{
					ShaderDebugMessage("has $PhongExponentTextureMask but has no $PhongExponentTexture. It also does not use $PhongNewBehaviour and thus cannot use that parameter. \n\n");
				}
			}

			// This doesn't make any sense and I want to tell the user
			if (HasFlag(MATERIAL_VAR_HALFLAMBERT) && IsDefined(PhongDisableHalfLambert) && GetBool(PhongDisableHalfLambert))
			{
				ShaderDebugMessage(" tries to enable $HalfLambert, but then disables it using $PhongDisableHalfLambert, YOU CAN'T HAVE BOTH, PICK ONE \n\n");
			}

			// NOTE: Usually an issue for VLG but not for Teeth
			// No BaseTexture but want to use the alpha.... riiiiight
//			if (!IsDefined(BaseTexture) && IsDefined(BaseMapAlphaPhongMask) && GetBool(BaseMapAlphaPhongMask))
//			{
//				ShaderDebugMessage("wants to use $BaseTexture's Alpha Channel for masking Phong, but no $BaseTexture was defined \n\n", pMaterialName);
//			}
		}
	}

	// Now actually enforce the Behaviour from above!
	if (GetBool(Phong) && !bUsesNewBehaviour)
	{	
		// InvertPhongMask does not flip the Phong Mask ironically
		if (IsDefined(InvertPhongMask) && GetBool(InvertPhongMask))
		{
			DefaultInt(EnvMapMaskFlip, 1);
			SetBool(InvertPhongMask, 0);
		}

		if (IsDefined(PhongTint) && IsDefined(PhongAlbedoTint) && GetBool(PhongAlbedoTint))
		{
			float3 f3PhongTint = GetFloat3(PhongTint);
			if (f3PhongTint[0] != 1.0f && f3PhongTint[1] != 1.0f && f3PhongTint[2] != 1.0f)
			{
				SetBool(PhongAlbedoTint, 0);
			}
		}

		// The following two caveats are related to PhongAlbedoTint
		if (IsDefined(PhongAlbedoTint) && GetBool(PhongAlbedoTint))
		{
			// PhongAlbedoTint can only be used with PhongExponentTexture
			if (!bHasPhongExponentTexture)
			{
				SetBool(PhongAlbedoTint, 0);
			}

			// This is a caveat introduced in CS:GO, we replicate the bevaviour to not break existing Materials
			if (IsDefined(PhongAlbedoBoost) && GetFloat(PhongAlbedoBoost) != 1.0f && IsDefined(Detail))
			{
				SetBool(PhongAlbedoTint, 0);
			}
		}
	}

	// Default Value is supposed to be 1.0f
	DefaultFloat(EnvMapSaturation, 1.0f);

	if (IsDefined(EnvMap) && !g_pConfig->UseSpecular() && IsDefined(BaseTexture))
		SetUndefined(EnvMap);

	// Default Value is supposed to be 1.0f
	DefaultFloat(DetailBlendFactor, 1.0f);

	// NOTE: No support for DetailTexCoord rn
	// Default Value is supposed to be 4.0f
//	DefaultFloat(DetailScale, 4.0f);

	// Default values are supposed to be 0, 0.5, 1
	DefaultFloat3(PhongFresnelRanges, 1.0f, 0.5f, 1.0f);

	// g_pConfig says "No Phong"? No Phong it is!
	if (GetBool(Phong) && !g_pConfig->UsePhong())
	{
		SetInt(Phong, 0);
	}
}

SHADER_FALLBACK
{
#ifndef REPLACE_TEETH
	if (lux_oldshaders.GetBool())
		return "Teeth";
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
	LoadBumpMap(BumpMap);
	LoadTexture(PhongExponentTexture);
	LoadTexture(PhongWarpTexture);

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
	SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube

	if (IsDefined(BumpMap))
	{
		SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
		SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting

		// This has to happen here since we can only access Flags after the Texture was loaded
		// BumpMap means Phong ( by default )
		// However it might be used for the Phong Mask, so only when not using BaseAlpha ( by default )
		// If there is a $PhongExponentTexture, we don't care
		// If there is no $PhongExponent, we have a problem as there is no Source for PhongExponent
		// If there is no Alpha Flag, we can't use the Alpha for PhongExponent can we?
		if (CVarDeveloper.GetInt() > 0 && !GetBool(BaseMapAlphaPhongMask))
		{
			// If using Phong, warn the User about not having a Phong Exponent Mask
			if (IsTextureLoaded(BumpMap) && GetInt(Phong) && !IsDefined(PhongExponentTexture) && !IsDefined(PhongExponent))
			{
				ITexture *pBumpMap = GetTexture(BumpMap);
				if (pBumpMap && !(pBumpMap->GetFlags() & TEXTUREFLAGS_EIGHTBITALPHA) && !(pBumpMap->GetFlags() & TEXTUREFLAGS_ONEBITALPHA))
				{
					ShaderDebugMessage("Uses $Phong ( probably by default ) but there is no Alpha on the $BumpMap and $PhongExponent is undefined.\nIf you don't want $Phong, set it to 0.\n");
				}
			}
		}
	}

	LoadCubeMap(EnvMap, 0);

	// This big block of if-statements is to determine if we even have any envmapmasking.
	// We don't want EnvMapMasking if we don't even have an envmap
	if (IsDefined(EnvMap))
	{
		// $EnvMapMask has Priority over other Masking
		if (IsDefined(EnvMapMask))
		{
			LoadTexture(EnvMapMask, 0);

			// We already have an $EnvMapMask, so remove these!
			ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			ClearFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
		}
		else
		{
			// NormalMapAlphaEnvMapMask takes priority, because its the go to one
			if (IsDefined(BumpMap) && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
			{
				if (GetTexture(BumpMap) && GetTexture(BumpMap)->IsError())
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

	if (HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK) || GetBool(BaseMapAlphaPhongMask))
	{
		ClearFlag(MATERIAL_VAR_ALPHATEST);
	}
}

// Virtual Void Override for Context Data
TeethContext* CreateMaterialContextData() override
{
	return new TeethContext(this);
}

SHADER_DRAW
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<TeethContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
//	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	bool bHasFlashlight = HasFlashlight();
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasNormalTexture = IsTextureLoaded(BumpMap);

	bool bHasEnvMap = !bHasFlashlight && IsTextureLoaded(EnvMap) && mat_specular.GetBool();
	bool bNormalMapAlphaEnvMapMask = !bHasEnvMap && HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
	bool bBaseAlphaEnvMapMask = !bHasEnvMap && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bHasEnvMapMask = bHasEnvMap &&	IsTextureLoaded(EnvMapMask); // No Envmapping under the flashlight
	bool bHasEnvMapFresnel = bHasEnvMap &&	GetBool(EnvMapFresnel);

	// 5&6 not done on this Shader
	int  nDetailBlendMode = GetBool(DetailBlendMode);
	bool bHasDetailTexture = (nDetailBlendMode != 5 && nDetailBlendMode != 6) && IsTextureLoaded(Detail);

	bool bHasPhong = bHasNormalTexture && GetBool(Phong); // Phong will be on by default!
	bool bHasPhongExponentTexture = bHasPhong && IsTextureLoaded(PhongExponentTexture);
	bool bHasPhongWarpTexture = bHasPhong && IsTextureLoaded(PhongWarpTexture);
	bool bHasPhongNewBehaviour = bHasPhong && GetBool(PhongNewBehaviour);
	bool bHasRimLight = bHasPhong && GetBool(RimLight);
	bool bHasBaseMapAlphaPhongMask = bHasPhong && GetBool(BaseMapAlphaPhongMask);

	// The RimMask is part of the PhongExponentTexture, therefore having one is a requirement...
	bool bHasRimMask = bHasRimLight && bHasPhongExponentTexture && GetBool(RimMask);

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
			// There is some whacky Behaviour using SelfIllumFresnel
			// No SelfIllum on this Shader ( no glowing Teeth )
			if (false) // bHasSelfIllumFresnel
			{
				// In this scenario Phong did "fEnvMapMask = lerp(BT.a, $InvertPhongMask, $NormalMapAlphaEnvMapMask)"
				// And then "lerp(EnvMapMask, 1-fEnvMapMask, $InvertPhongMask)"
				// Aka EnvMapMask = 0.0f
				pContextData->m_bEnvMapFresnel = false;
				pContextData->m_nEnvMapMode = 0;

				// Doesn't matter since we have no EnvMap here.
				pContextData->m_nbBaseAlphaEnvMapMask = false;
				pContextData->m_nbNormalMapAlphaEnvMapMask = false;
			}
			else
			{
				// Stock-Consistency:
				// Use BaseAlpha as EnvMapMask when using $BaseMapAlphaPhongMask with $NormalMapAlphaEnvMapMask
				pContextData->m_nEnvMapMode = bHasEnvMap;
				pContextData->m_nbBaseAlphaEnvMapMask = bHasBaseMapAlphaPhongMask;
				pContextData->m_nbNormalMapAlphaEnvMapMask = !bHasBaseMapAlphaPhongMask;
			}
		}
		else if (bHasPhong && !bHasPhongNewBehaviour)
		{
			// Set to true by Default, LUX Allows no Masking ( 1.0f ) so we need to enforce this
			// TF2C Demoman Cyclops for Example requires this
			pContextData->m_nbBaseAlphaEnvMapMask = true;
			pContextData->m_nbNormalMapAlphaEnvMapMask = false;
		}
		else
		{
			// Use what we actually want
			pContextData->m_nbBaseAlphaEnvMapMask = bBaseAlphaEnvMapMask;
			pContextData->m_nbNormalMapAlphaEnvMapMask = bNormalMapAlphaEnvMapMask;
		}

		if(bHasPhong)
		{
			if (GetBool(PhongDisableHalfLambert))
				pContextData->m_bHalfLambert = false;
			else if (lux_phong_defaulthalflambert.GetBool())
				pContextData->m_bHalfLambert = true;
		}
		else
			pContextData->m_bHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);
	}

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(pContextData->m_nBlendType);
	
		// Stock-Consistency: *shrug*
		FogToFogColor();

		// We always need this
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Always using this on Models. Which needs compressed verts
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;

		int nTexCoords = 1;
		int nUserDataSize = bHasNormalTexture ? 4 : 0;
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

		// s5 - $EnvMapMask. Stock-Consistency: Not sRGB
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		// s7 - $PhongWarpTexture. Never sRGB
		EnableSampler(bHasPhongWarpTexture, SAMPLER_PHONGWARP, false);

		// s8 - $PhongExponentTexture. Never sRGB
		EnableSampler(bHasPhongExponentTexture, SAMPLER_PHONGEXPONENT, false);

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		// Determine Lighting Mode
		int nLightingMode;
		if (bHasPhong)
			nLightingMode = bHasPhongExponentTexture ? 3 : 2;
		else
			nLightingMode = bHasNormalTexture ? 1 : 0;

		DECLARE_STATIC_VERTEX_SHADER(lux_teeth_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, !bHasFlashlight && !bHasNormalTexture && pContextData->m_bHalfLambert);
		SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, bHasNormalTexture);
		SET_STATIC_VERTEX_SHADER(lux_teeth_vs30);
	
		DECLARE_STATIC_PIXEL_SHADER(lux_teeth_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(PROJTEX, bHasFlashlight);
		SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, pContextData->m_nEnvMapMode);
		SET_STATIC_PIXEL_SHADER_COMBO(LIGHTING_MODE, bHasFlashlight ? 0 : nLightingMode);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
		SET_STATIC_PIXEL_SHADER(lux_teeth_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// Getting the light state
		// Always need this for lighting
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);

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

		#ifdef LUX_DEBUGCONVARS
		if (lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SAMPLER_NORMALMAP, TEXTURE_NORMALMAP_FLAT);
		}
		else
		#endif
			BindTexture(bHasNormalTexture, SAMPLER_NORMALMAP, BumpMap, BumpFrame);

		BindTexture(bHasDetailTexture, SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		if (bHasEnvMap)
		{
			BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);
			BindTexture(bHasEnvMapMask, SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);
		}

		BindTexture(bHasPhongExponentTexture, SAMPLER_PHONGEXPONENT, PhongExponentTexture, 0);
		BindTexture(bHasPhongWarpTexture, SAMPLER_PHONGWARP, PhongWarpTexture, 0);

		//==================================================================================================
		// Setup Constant Registers
		//==================================================================================================

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		if (bHasEnvMap || bHasPhong)
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);
		
		// c32 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// c33, c34
		if (bHasDetailTexture)
		{
			float4 f4Tint_Factor;
			f4Tint_Factor.rgb = GetFloat3(DetailTint);
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_FACTORS, f4Tint_Factor);

			float4 f4Blendmode = 0.0f;
			f4Blendmode.x = (float)nDetailBlendMode;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DETAIL_BLENDMODE, f4Blendmode);
		}

		// c37, c38, c39, c40
		if (bHasEnvMap)
		{
			// c37
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			#ifdef LUX_DEBUGCONVARS
			if (lux_disablefast_envmap.GetBool())
				f4EnvMapTint_LightScale.rgb = float3(0.0f, 0.0f, 0.0f);
			#endif
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// c38
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation);
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// c39
			float4 f4EnvMapControls = 0.0f;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = (float)GetBool(EnvMapMaskFlip);
//			FREE
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// c40, modified by $Phong
			// OOPS: PhongFresnelRanges should be used instead of EnvMapFresnel.
			// Not tragic but still tragic
			float4 f4EnvMapFresnelRanges = 0.0f;
			if (bHasEnvMapFresnel)
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
			else
				f4EnvMapFresnelRanges.y = 1.0f; // This will disable EnvMapFresnel
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapControls);
		}

		// c45, c46, c47, c48
		if (bHasPhong)
		{
			// $PhongTint, $InvertPhongMask
			float4 f4PhongTint_InvertMask;
			f4PhongTint_InvertMask.rgb = GetFloat3(PhongTint);
			f4PhongTint_InvertMask.w = (float)GetBool(InvertPhongMask);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_TINT, f4PhongTint_InvertMask);

			// $PhongFresnelRanges, $PhongExponentFactor
			float4 f4PhongFresnelRanges_ExponentFactor;
			f4PhongFresnelRanges_ExponentFactor.xyz = GetFloat3(PhongFresnelRanges);
			f4PhongFresnelRanges_ExponentFactor.x = (f4PhongFresnelRanges_ExponentFactor.y - f4PhongFresnelRanges_ExponentFactor.x) * 2;
			f4PhongFresnelRanges_ExponentFactor.z = (f4PhongFresnelRanges_ExponentFactor.z - f4PhongFresnelRanges_ExponentFactor.y) * 2;
			f4PhongFresnelRanges_ExponentFactor.w = GetFloat(PhongExponentFactor);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_FRESNEL, f4PhongFresnelRanges_ExponentFactor);

			float4 f4PhongControls;
			f4PhongControls.x = pContextData->m_bPhong_AlbedoTintBoost ? GetFloat(PhongAlbedoBoost) : 1.0f;
			f4PhongControls.y = bHasRimLight ? GetFloat(RimLightExponent) : 0.0f;
			f4PhongControls.z = bHasRimLight ? GetFloat(RimLightBoost) : 0.0f;
			f4PhongControls.w = GetFloat(PhongExponent);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_CONTROLS, f4PhongControls);

			float4 f4MinimumLight_PhongBoost;
			f4MinimumLight_PhongBoost.rgb = GetFloat3(PhongMinimumLight);
			f4MinimumLight_PhongBoost.w = GetFloat(PhongBoost);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PHONG_MINLIGHT_BOOST, f4MinimumLight_PhongBoost);
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b1
		if(pContextData->m_bHalfLambert)
			BBools[LUX_PS_BOOL_HALFLAMBERT] = true;

		// b3, b4, b5, b6, b7, b8, b9, b10
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

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque); // Heightfog instead of Range/Radial Fog
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// Handled below
		bool bFlashlightShadows = false;

		if (!bHasFlashlight && bHasNormalTexture)
		{
			// c13, c14, c15, c16, c17, c18
			pShaderAPI->SetPixelShaderStateAmbientLightCube(LUX_PS_FLOAT_AMBIENTCUBE, !LightState.m_bAmbientLight);

			// c20, c21, c22, c23, c24, c25
			pShaderAPI->CommitPixelShaderLighting(LUX_PS_FLOAT_LIGHTDATA);
		}
		else
		{
			// Binds Flashlight Textures and sends constants
			// returns bFlashlightShadows
			bFlashlightShadows = SetupFlashlight();
		}

		//==================================================================================================
		// Setup Vertex Shader Constant Registers
		//==================================================================================================
		float4 f4Lighting;
		f4Lighting.xyz = GetFloat3(Forward);
		f4Lighting.w = GetFloat(IllumFactor);
		pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET0_0, f4Lighting);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		bool bHasStaticPropLighting = 0;
		bool bHasDynamicPropLighting = 0;

		// Dynamic Prop Lighting here refers to dynamic vertex lighting, or ambient cubes via the vertex shader
		// We shouldn't have that on bumped or phonged models. Same for Static Vertex Lighting
		if (!bHasPhong && !bHasNormalTexture)
		{
			bHasStaticPropLighting = StaticLightVertex(LightState); // LightState varies between SP and MP so we use a function to reinterpret
			bHasDynamicPropLighting = LightState.m_bAmbientLight || (LightState.m_nNumLights > 0) ? 1 : 0;

			// Need to send this to the Vertex Shader manually in this scenario
			if (bHasDynamicPropLighting)
				SetAmbientCubeDynamicStateVertexShader();
		}

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_teeth_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, !bHasFlashlight && bHasStaticPropLighting);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, !bHasFlashlight && bHasDynamicPropLighting);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_teeth_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_teeth_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS_COMBO, LightState.m_nNumLights);
		SET_DYNAMIC_PIXEL_SHADER(lux_teeth_ps30);
	}

	Draw();
}
END_SHADER