//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_model_vs30.inc"
#include "lux_model_simplified_vs30.inc"
#include "lux_unlitgeneric_ps30.inc"
#include "lux_vertexlitgeneric_flashlight_ps30.inc"

// Register Map for this Shader
#include "../lux_unlitgeneric_registermap.h"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric_DX8, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric_DX6, LUX_UnlitGeneric)
#endif

#ifdef REPLACE_UNLITGENERIC
DEFINE_FALLBACK_SHADER(UnlitGeneric, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(UnlitGeneric_DX8, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(UnlitGeneric_DX6, LUX_UnlitGeneric)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class UnlitGenericContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	UnlitGenericContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_UnlitGeneric, "Unlit Shader, Diffuse with no Lighting.")
SHADER_INFO_GEOMETRY	("Brushes, Models and Displacements.")
SHADER_INFO_USAGE		("Apply to Geometry.")
SHADER_INFO_LIMITATIONS	("No Lighting. Use VertexLitGeneric instead for Models with Lighting.")
SHADER_INFO_PERFORMANCE	("Very Cheap.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC UnlitGeneric Shader Page: https://developer.valvesoftware.com/wiki/UnlitGeneric")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_MiscParameters()
	Declare_DetailTextureParameters()
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_ParallaxCorrectionParameters()
	Declare_DistanceAlphaParameters()

	// Treesway Implementation
	Declare_TreeswayParameters()

	// Important Parameters often used by hacky vmt's
	SHADER_PARAM(HDRColorScale,	 SHADER_PARAM_TYPE_FLOAT, "", "Linear Tint Multiplier specific to HDR.")
	SHADER_PARAM(GammaColorRead, SHADER_PARAM_TYPE_BOOL,  "", "Disables SRGB Conversions of the $BaseTexture when reading.")
	SHADER_PARAM(LinearWrite,	 SHADER_PARAM_TYPE_BOOL,  "", "Disables SRGB Conversions when writing Results.")

	// Used for some TF2 Hud Element apparently. Sadly can't TexCoord1 for regular Models..
	SHADER_PARAM(SeparateDetailUVs, SHADER_PARAM_TYPE_BOOL, "", "Make $Detail use TexCoord1. Only works with MeshBuilder Meshes.")

	// Fallback Parameters
	// FIXME: Not implemented yet
	SHADER_PARAM(DepthBlend, SHADER_PARAM_TYPE_INTEGER, "", "(FALLBACK) This Parameter will cause the Shader to fallback to LUX_DepthBlendGeneric.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	if (IsDefined(DistanceAlpha) && GetBool(DistanceAlpha))
		SetBool(ReceiveProjectedTextures, 0);
	else
		// Usually don't want it!
		DefaultInt(ReceiveProjectedTextures, 0);

	DefaultFloat(HDRColorScale, 1.0f);

	DefaultFloat(DetailBlendFactor, 1.0f); // Default Value is supposed to be 1.0f

	// Stock Shader does not allow for Transforms on secondary TexCoord
	// LUX does, but we need to replicate the unmodified Scale with this
	if(GetBool(SeparateDetailUVs))
		DefaultFloat(DetailScale, 1.0f);
	else
		DefaultFloat(DetailScale, 4.0f);

	DefaultFloat(EnvMapSaturation, 1.0f); // Default Value is supposed to be 1.0f

	// No BaseTexture ? None of that.
	if (!IsDefined(BaseTexture))
	{
		ClearFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// If mat_specular 0, then get rid of envmap
	if (!g_pConfig->UseSpecular() && IsDefined(EnvMap))
	{
		SetUndefined(EnvMap);
	}

	// Seamless 
	/*
	if (GetFloat(Seamless_Scale) != 0.0f)
	{
		// if we don't have DetailScale we want Seamless_Scale
		if (GetFloat(Seamless_DetailScale) == 0.0f)
		{
			SetFloat(Seamless_DetailScale, GetFloat(Seamless_Scale));
		}
		// Using Seamless_Scale will enable Seamless_Base
		// IMPORTANT: **Not on ULG**. Stock behaviour demands you use $seamless_base!
		// SetInt(info.m_nSeamless_Base, 1)
	}
	*/

	DefaultFloat(EdgeSoftnessStart, 0.5);
	DefaultFloat(EdgeSoftnessEnd, 0.5);
	DefaultFloat(OutlineAlpha, 1.0);

	if (CVarDeveloper.GetInt() > 0)
	{
		if (GetBool(DistanceAlpha) && !IsDefined(BaseTexture) && !GetBool(DistanceAlphaFromDetail))
		{
			ShaderDebugMessage("has no Basetexture to derive the Alpha from, and doesn't want to use the DetailTexture.\n\n");
		}

		if (HasFlag(MATERIAL_VAR_MODEL) && IsDefined(DetailBlendMode) && GetInt(DetailBlendMode) == 10)
		{
			ShaderDebugMessage("cannot uuse DetailBlendMode 10. Model Materials don't have Bumped Lightmaps.\n\n");
		}
	}

	// Always need to set this, for animated Models
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

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
}

SHADER_FALLBACK
{
#ifndef REPLACE_UNLITGENERIC
	if (lux_oldshaders.GetBool())
		return "UnlitGeneric";
#endif

	// Since c32+ crashes on quit we can't do this one
//	if (GetBool(DistanceAlpha))
//		return "LUX_DistanceAlpha_Unlit";

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

	// 0 = mod2x, Linear
	// 10 or 11 = SSBump, Linear
	int nDetailBlendMode = GetInt(DetailBlendMode);
	LoadTexture(Detail, IsGammaDetailMode(nDetailBlendMode) ? TEXTUREFLAGS_SRGB : 0);

	LoadCubeMap(EnvMap);

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
		}

		// Tell the Shader to flip the EnvMap when set on AlphaEnvMapMask ( Consistency with Stock Shaders )
		if (!GetBool(EnvMapMaskFlip) && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
		{
			SetBool(EnvMapMaskFlip, true);
		}

		// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
		// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
		// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
		// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
		if(IsDefined(EnvMapParallaxOBB1) && !GetBool(EnvMapParallax))
			DefaultBool(EnvMapParallax, true);
	}

	if (HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
	{
		// No Detail = Can't reuse Alpha
		if (!IsDefined(Detail))
		{
			ClearFlag(MATERIAL_VAR_ALPHATEST);
			SetBool(BlendTintByBaseAlpha, false);
		}
		else
		{
			// Can only Output Alpha if one of these Blendmodes
			int nDetailBlendMode = GetInt(DetailBlendMode);
			if (nDetailBlendMode != 3 && nDetailBlendMode != 8 && nDetailBlendMode != 9)
			{
				ClearFlag(MATERIAL_VAR_ALPHATEST);
				SetBool(BlendTintByBaseAlpha, false);
			}
		}

	}
}

// Virtual Void Override for Context Data
#if FIXED_COMMANDBUFFER
UnlitGenericContext* CreateMaterialContextData() override
{
	return new UnlitGenericContext(this);
}
#endif

SHADER_DRAW
{
#if FIXED_COMMANDBUFFER
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<UnlitGenericContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;
#endif

	bool bProjTex = HasFlashlight();
	bool bBlendTintByBaseAlpha = GetBool(BlendTintByBaseAlpha);
	bool bDesaturateWithBaseAlpha = !bBlendTintByBaseAlpha && GetBool(DesaturateWithBaseAlpha);
	bool bBaseAlphaEnvMapMask = !bProjTex && HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK);

	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasDetailTexture = IsTextureLoaded(Detail);

	bool bHasEnvMap = !bProjTex && IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bHasEnvMapFresnel = bHasEnvMap && GetBool(EnvMapFresnel);
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	bool bHasVertexRGBA = HasFlag(MATERIAL_VAR_VERTEXCOLOR) || HasFlag(MATERIAL_VAR_VERTEXALPHA);
	int nTreeSway = GetInt(TreeSway);

	bool bDistanceAlpha = GetBool(DistanceAlpha);

#if FIXED_COMMANDBUFFER
	//==========================================================================//
	// Pre-Snapshot Context Data Variables
	//==========================================================================//
	if(IsSnapshottingCommands())
	{
		BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
		bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

		pContextData->m_bIsFullyOpaque = bIsFullyOpaque;
		pContextData->m_nBlendType = nBlendType;
	}
#else
	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);
#endif

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if(IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

#if FIXED_COMMANDBUFFER
		// Everything Transparency is packed into this Function
		EnableTransparency(pContextData->m_nBlendType);

		// We always need this
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque);
#else
		// Everything Transparency is packed into this Function
		EnableTransparency(nBlendType);

		// We always need this
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);
#endif

		// Weird name, what it actually means : We output linear Values
		bool bSRGBWrite = !GetBool(LinearWrite); // Stock Consistency
		pShaderShadow->EnableSRGBWrite(bSRGBWrite);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;

		if (bHasEnvMap)
			nFlags |= VERTEX_NORMAL;

		if (bHasVertexRGBA)
			nFlags |= VERTEX_COLOR;

		int nTexCoords = 1;

		// Second TexCoord exists here. It's used by MeshBuilder Meshes in TF2 for some HUD Element for Example.
		// It cannot be used with Regular Models however.
		bool bSeparateDetailUVs = bHasDetailTexture && GetBool(SeparateDetailUVs);
		if (bSeparateDetailUVs)
			nTexCoords = 2;

		int pTexCoordDim[3] = { 2, 2, 3 };
		int nUserDataSize = bProjTex ? 4 : 0; // Not technically required, we don't use the Tangents.

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, pTexCoordDim, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture. sRGB by Default.
		EnableSampler(SAMPLER_BASETEXTURE, !GetBool(GammaColorRead));

		// s4 - $Detail.
		// Stock Shaders set sRGBRead when nDetailBlendMode != 0, which is probably a massive Oversight!
		// 0 is mod2X, that's always been linear.
		// 10 and 11 are SSBumps and Normal Maps, they should *never* be sRGB.
		int nDetailBlendMode = GetInt(DetailBlendMode);
		EnableSampler(bHasDetailTexture, SAMPLER_DETAILTEXTURE, IsGammaDetailMode(nDetailBlendMode));

		// s5 - $EnvMapMask. Never sRGB
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		// s14 - $EnvMap. sRGB when LDR
		EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		int nNeededTexCoords = bHasEnvMapMask + bHasDetailTexture;
		if (bProjTex)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 1);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bHasVertexRGBA);
			SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

			// No Normals so LightCombo 0
			DECLARE_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTCOMBO, 0);
			SET_STATIC_PIXEL_SHADER_COMBO(WRINKLEMAPS, 0);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
			SET_STATIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
		}
		else
		{
			bool bLinearVertexColors = bHasVertexRGBA && !bSRGBWrite;
			bool bSeparateDetailUV = bHasDetailTexture && GetBool(SeparateDetailUVs);

			DECLARE_STATIC_VERTEX_SHADER(lux_model_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, nNeededTexCoords);
			SET_STATIC_VERTEX_SHADER_COMBO(SPECIALTEXCOORDS, bSeparateDetailUV ? 1 : 0);
			SET_STATIC_VERTEX_SHADER_COMBO(TANGENTS, 0);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, nTreeSway);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bLinearVertexColors ? 2 : bHasVertexRGBA);
			SET_STATIC_VERTEX_SHADER(lux_model_vs30);

			int nDistanceAlphaMode = 0;
			if (bDistanceAlpha)
			{
				nDistanceAlphaMode += GetBool(SoftEdges);
				nDistanceAlphaMode += GetBool(Outline) * 2;
				nDistanceAlphaMode += GetBool(Glow) * 4;
			}

			int nDetailCombo = bHasDetailTexture + GetBool(DistanceAlphaFromDetail) + 2 * bSeparateDetailUV;
			int nEnvMapMode = bHasEnvMap + bHasEnvMapMask + bPCC * 2;
			DECLARE_STATIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, nDetailCombo);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
			SET_STATIC_PIXEL_SHADER_COMBO(XBYBASEALPHA, bBlendTintByBaseAlpha + 2 * bDesaturateWithBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHAMODE, nDistanceAlphaMode);
			SET_STATIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
		}
	}

#if FIXED_COMMANDBUFFER
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

	// Disabled for now since CMD's cause crashes on this Shader
	if(MaterialVarsChanged())
	{
		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		if(bHasBaseTexture)
			SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);
		else
		{
			// Using Standard Textures during launch, crashes H++ and Vulkan
			// They are probably not initialised early enough.
			// Works with the CmdBuffers though..
			if (bHasEnvMap)
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
			else
				SemiStaticCmds.BindTexture(SAMPLER_BASETEXTURE, TEXTURE_WHITE);
		}


		if (bHasDetailTexture)
			SemiStaticCmds.BindTexture(SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		if(bHasEnvMap && bHasEnvMapMask)
			SemiStaticCmds.BindTexture(SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//
		
		// c0
		// Don't use c0, it causes Issues in Hammer
		
		// c1 Modulation Constant(s).
		float4 Modul;
		Modul.rgb = 1.0f;
		Modul.a = GetAlpha();
		SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_001, Modul);

		// c2, c3, c4, c5, c6, c7, c8, c9
		// Not doing EnvMapTint ( c2 ) since it can be altered at runtime.
		if (bHasEnvMap)
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.xyz = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_002, f4EnvMapTint_LightScale);

			float4 f4EnvMapFresnelRanges = 0.0f;
			if (bHasEnvMapFresnel)
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
			else
				f4EnvMapFresnelRanges.y = 1.0f; // This will disable EnvMapFresnel
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_003, f4EnvMapFresnelRanges);

			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.xyz = GetFloat3(EnvMapSaturation);
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_004, f4EnvMapSaturation_Contrast);

			if (bPCC)
			{
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_005, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_006, f4Row1);
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_007, f4Row2);
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_008, f4Row3);
			}

			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = 0.0f; // No Normal Map
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip);
			f4EnvMapControls.w = 0.0f; // No Normal Map
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_009, f4EnvMapControls);
		}

		// c10 - Tint
		// $Color/$Color2, $AllowDiffuseModulation $NoTint, $BlendTintColorOverBase
		// and $sRGBTint is handled here
		int nAlphaVar = bBlendTintByBaseAlpha ? BlendTintColorOverBase : DesaturateWithBaseAlpha;
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), nAlphaVar);

		// Stock-Consistency : GammaToLinear Conversion
		f4Tint.rgb = GammaToLinearTint(f4Tint.rgb);

		if (IsHDREnabled())
			f4Tint.rgb *= GetFloat(HDRColorScale);

		// ComputeTint has a Line where it sets .rgb to 0.0f when g_pConfig->bShowDiffuse is false
		// I do this here because UI Elements heavily rely on UnlitGeneric, they shouldn't just go poof.
		if (!g_pConfig->bShowDiffuse)
			f4Tint.rgb = 1.0f;
			
		// c10
		SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_010, f4Tint);

		// c11
		SemiStaticCmds.SetPixelShaderConstant_EyePos(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12
		SemiStaticCmds.SetPixelShaderConstant_FogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c13 to c19 is Flashlight Constants

		// c20, c21
		if (bHasDetailTexture)
		{
			int nDetailBlendMode = GetInt(DetailBlendMode);
			float4 f4Tint_Factor;
			f4Tint_Factor.xyz = GetFloat3(DetailTint);
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_020, f4Tint_Factor);

			float4 f4Blendmode;
			f4Blendmode.x = (float)nDetailBlendMode;
			f4Blendmode.y = 0.0f;
			f4Blendmode.z = 0.0f;
			f4Blendmode.w = 0.0f;
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_021, f4Blendmode);
		}

		// c22+
		if (bDistanceAlpha)
		{
			if (GetBool(Glow))
			{
				// This is pretty much consistent with the Stock Constant Register Layout
				float4 f4GlowParameters;
				f4GlowParameters.x = GetFloat(GlowX);
				f4GlowParameters.y = GetFloat(GlowY);
				f4GlowParameters.z = GetFloat(GlowStart);
				f4GlowParameters.w = GetFloat(GlowEnd);
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_022, f4GlowParameters);

				float4 f4GlowColor;
				f4GlowColor.rgb = GetFloat3(GlowColor);
				f4GlowColor.a = GetFloat(GlowAlpha);
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_023, f4GlowColor);
			}

			float f1SoftStart = GetFloat(EdgeSoftnessStart);
			float f1SoftEnd = GetFloat(EdgeSoftnessEnd);

			float f1OutlineStart0 = GetFloat(OutlineStart0);
			float f1OutlineStart1 = GetFloat(OutlineStart1);
			float f1OutlineEnd0 = GetFloat(OutlineEnd0);
			float f1OutlineEnd1 = GetFloat(OutlineEnd1);

			// "set all line art shader parms"
			bool bScaleEdges = GetBool(ScaleEdgeSoftnessBasedOnScreenRes);
			bool bScaleOutline = GetBool(ScaleOutlineSoftnessBasedOnScreenRes);

			// Only do this when scaling anything based on screen res
			if (bScaleEdges || bScaleOutline)
			{
				int nWidth, nHeight;
				pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);

				// ..1: Up these resolutions, find a dynamic approach for these constants?
				// ..2: (float), those are integers right now
				float f1ResScale = max(0.5f, max(1024.0f / nWidth, 768.0f / nHeight));

				if (bScaleEdges)
				{
					float f1Mid = 0.5 * (f1SoftStart + f1SoftEnd);
					f1SoftStart = clamp(f1Mid + f1ResScale * (f1SoftStart - f1Mid), 0.05, 0.99);
					f1SoftEnd = clamp(f1Mid + f1ResScale * (f1SoftEnd - f1Mid), 0.05, 0.99);
				}


				if (bScaleOutline)
				{
					// shrink the soft part of the outline, enlarging hard part
					float f1MidS = 0.5 * (f1OutlineStart1 + f1OutlineStart0);
					f1OutlineStart1 = clamp(f1MidS + f1ResScale * (f1OutlineStart1 - f1MidS), 0.05f, 0.99f);
					float f1MidE = 0.5 * (f1OutlineEnd1 + f1OutlineEnd0);
					f1OutlineEnd1 = clamp(f1MidE + f1ResScale * (f1OutlineEnd1 - f1MidE), 0.05f, 0.99f);
				}
			}

			float4 f4DA_Parameters;
			f4DA_Parameters.x = f1SoftStart;
			f4DA_Parameters.y = f1SoftEnd;
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_024, f4DA_Parameters);

			if (GetBool(Outline))
			{
				float4 f4OutlineColor;
				f4OutlineColor.rgb = GetFloat3(OutlineColor);
				f4OutlineColor.a = GetFloat(OutlineAlpha); // This isn't actually used anywhere on the Shader
				SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_025, f4OutlineColor);
			}

			// "c9 - outline parms. ordered for optimal ps20 .wzyx swizzling"
			float4 f4OutlineParameters;
			f4OutlineParameters.x = f1OutlineStart0;
			f4OutlineParameters.y = f1OutlineEnd1;
			f4OutlineParameters.z = f1OutlineEnd0;
			f4OutlineParameters.w = f1OutlineStart1;
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_028, f4OutlineParameters);
		}

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nRegisterShift = 0;
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
			// Always doing this for Separate Detail
			if(GetBool(SeparateDetailUVs))
			{
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_05, DetailTextureTransform, DetailScale);
			}
			else
			{
				bool bDetailTextureTransform = HasTransform(true, DetailTextureTransform);
				if (bDetailTextureTransform)
					SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, DetailTextureTransform, DetailScale);
				else
					SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform, DetailScale);			
			}
		}

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}
#else
	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if (IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// ShiroDkxtro2:
		// Some of the default Textures have an invalid ShaderAPI Texture Handle. Great!
		// This was never an Issue on Stock Shaders because they use the Command Buffer
		// The Command Buffer ensures you only ever stuff VALID Texture Handles into it
		// Since we don't use the Command Buffer here ( due to Reasons ),
		// using BindTexture will crash Hammer on some default White Engine Texture.
		// For Some it doesn't have a valid ShaderAPI Texture Handle..
		if(bHasBaseTexture)
		{
			ITexture* pTexture = GetTexture(BaseTexture);
			if(pTexture)
			{
				ShaderAPITextureHandle_t hHandle = GetShaderAPITextureBindHandle(BaseTexture, Frame);
				if(hHandle != INVALID_SHADERAPI_TEXTURE_HANDLE)
					BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);
			}
			else
				BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
		}
		/*
		if(bHasBaseTexture)
			BindTexture(SAMPLER_BASETEXTURE, BaseTexture, Frame);
		*/
		else
		{
			// Using Standard Textures during launch, crashes H++ and Vulkan
			// They are probably not initialised early enough.
			// Works with the CmdBuffers though..
			if (bHasEnvMap)
				BindTexture(SAMPLER_BASETEXTURE, TEXTURE_BLACK);
			else
				BindTexture(SAMPLER_BASETEXTURE, TEXTURE_WHITE);
		}


		if (bHasDetailTexture)
			BindTexture(SAMPLER_DETAILTEXTURE, Detail, DetailFrame);

		if(bHasEnvMap && bHasEnvMapMask)
			BindTexture(SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//
		
		// c2, c3, c4, c5, c6, c7, c8, c9
		// Not doing EnvMapTint ( c2 ) since it can be altered at runtime.
		if (bHasEnvMap)
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.xyz = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_TINT, f4EnvMapTint_LightScale);

			float4 f4EnvMapFresnelRanges = 0.0f;
			if (bHasEnvMapFresnel)
				f4EnvMapFresnelRanges.xyz = GetFloat3(EnvMapFresnelMinMaxExp);
			else
				f4EnvMapFresnelRanges.y = 1.0f; // This will disable EnvMapFresnel
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);

			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.xyz = GetFloat3(EnvMapSaturation);
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			if (bPCC)
			{
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_POSITION, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_MATRIX +0, f4Row1);
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_MATRIX +1, f4Row2);
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_MATRIX +2, f4Row3);
			}

			float4 f4EnvMapControls;
			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
			f4EnvMapControls.y = 0.0f; // No Normal Map
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip);
			f4EnvMapControls.w = 0.0f; // No Normal Map
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_ENVMAP_CONTROLS, f4EnvMapControls);
		}

		// c11, c12
		if (bHasDetailTexture)
		{
			int nDetailBlendMode = GetInt(DetailBlendMode);
			float4 f4Tint_Factor;
			f4Tint_Factor.xyz = GetFloat3(DetailTint);
			f4Tint_Factor.w = GetFloat(DetailBlendFactor);
			f4Tint_Factor = PrecomputeDetail(f4Tint_Factor, nDetailBlendMode);
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DETAIL_TINT, f4Tint_Factor);

			float4 f4Blendmode;
			f4Blendmode.x = (float)nDetailBlendMode;
			f4Blendmode.y = 0.0f;
			f4Blendmode.z = 0.0f;
			f4Blendmode.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DETAIL_MODE, f4Blendmode);
		}

		// c13+
		if (bDistanceAlpha)
		{
			if (GetBool(Glow))
			{
				// This is pretty much consistent with the Stock Constant Register Layout
				float4 f4GlowParameters;
				f4GlowParameters.x = GetFloat(GlowX);
				f4GlowParameters.y = GetFloat(GlowY);
				f4GlowParameters.z = GetFloat(GlowStart);
				f4GlowParameters.w = GetFloat(GlowEnd);
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DA_GLOWPARAMS, f4GlowParameters);

				float4 f4GlowColor;
				f4GlowColor.rgb = GetFloat3(GlowColor);
				f4GlowColor.a = GetFloat(GlowAlpha);
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DA_GLOWCOLOR, f4GlowColor);
			}

			float f1SoftStart = GetFloat(EdgeSoftnessStart);
			float f1SoftEnd = GetFloat(EdgeSoftnessEnd);

			float f1OutlineStart0 = GetFloat(OutlineStart0);
			float f1OutlineStart1 = GetFloat(OutlineStart1);
			float f1OutlineEnd0 = GetFloat(OutlineEnd0);
			float f1OutlineEnd1 = GetFloat(OutlineEnd1);

			// "set all line art shader parms"
			bool bScaleEdges = GetBool(ScaleEdgeSoftnessBasedOnScreenRes);
			bool bScaleOutline = GetBool(ScaleOutlineSoftnessBasedOnScreenRes);

			// Only do this when scaling anything based on screen res
			if (bScaleEdges || bScaleOutline)
			{
				int nWidth, nHeight;
				pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);

				// ..1: Up these resolutions, find a dynamic approach for these constants?
				// ..2: (float), those are integers right now
				float f1ResScale = max(0.5f, max(1024.0f / nWidth, 768.0f / nHeight));

				if (bScaleEdges)
				{
					float f1Mid = 0.5 * (f1SoftStart + f1SoftEnd);
					f1SoftStart = clamp(f1Mid + f1ResScale * (f1SoftStart - f1Mid), 0.05f, 0.99f);
					f1SoftEnd = clamp(f1Mid + f1ResScale * (f1SoftEnd - f1Mid), 0.05f, 0.99f);
				}


				if (bScaleOutline)
				{
					// shrink the soft part of the outline, enlarging hard part
					float f1MidS = 0.5 * (f1OutlineStart1 + f1OutlineStart0);
					f1OutlineStart1 = clamp(f1MidS + f1ResScale * (f1OutlineStart1 - f1MidS), 0.05f, 0.99f);
					float f1MidE = 0.5 * (f1OutlineEnd1 + f1OutlineEnd0);
					f1OutlineEnd1 = clamp(f1MidE + f1ResScale * (f1OutlineEnd1 - f1MidE), 0.05f, 0.99f);
				}
			}

			float4 f4DA_Parameters;
			f4DA_Parameters.x = f1SoftStart;
			f4DA_Parameters.y = f1SoftEnd;
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DA_PARAMS, f4DA_Parameters);

			if (GetBool(Outline))
			{
				float4 f4OutlineColor;
				f4OutlineColor.rgb = GetFloat3(OutlineColor);
				f4OutlineColor.a = GetFloat(OutlineAlpha); // This isn't actually used anywhere on the Shader
				pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DA_OUTLINECOLOR, f4OutlineColor);
			}

			// "c9 - outline parms. ordered for optimal ps20 .wzyx swizzling"
			float4 f4OutlineParameters;
			f4OutlineParameters.x = f1OutlineStart0;
			f4OutlineParameters.y = f1OutlineEnd1;
			f4OutlineParameters.z = f1OutlineEnd0;
			f4OutlineParameters.w = f1OutlineStart1;
			pShaderAPI->SetPixelShaderConstant(UNLITGENERIC_DA_OUTLINEPARARMS, f4OutlineParameters);
		}

		// c25
		// Need Luminance Weights in this Scenario
		if (bHasEnvMap || bDesaturateWithBaseAlpha)
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// c26
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c27
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c28
		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c31 - Tint
		// $Color/$Color2, $AllowDiffuseModulation $NoTint, $BlendTintColorOverBase
		// and $sRGBTint is handled here
		int nAlphaVar = bBlendTintByBaseAlpha ? BlendTintColorOverBase : DesaturateWithBaseAlpha;
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), nAlphaVar);

		// Stock-Consistency : GammaToLinear Conversion
		f4Tint.rgb = GammaToLinearTint(f4Tint.rgb);

		if (IsHDREnabled())
			f4Tint.rgb *= GetFloat(HDRColorScale);

		// I do this here because UI Elements rely on UnlitGeneric
		// When mat_diffuse == 0, your UI will basically disappear.. Not good!
		if (!g_pConfig->bShowDiffuse)
			f4Tint.rgb = 1.0f;
			
		// c31
		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//

		// VS c223, c224 - $BaseTextureTransform
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		int nRegisterShift = 0;
		if(bHasEnvMapMask)
		{
			bool bEnvMapMaskTransform = HasTransform(true, EnvMapMaskTransform);
			if (bEnvMapMaskTransform)
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, EnvMapMaskTransform);
			else
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform);

			nRegisterShift += 2;
		}

		if(bHasDetailTexture)
		{
			// Always doing this for Separate Detail
			if(GetBool(SeparateDetailUVs))
			{
				SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_05, DetailTextureTransform, DetailScale);
			}
			else
			{
				bool bDetailTextureTransform = HasTransform(true, DetailTextureTransform);
				if (bDetailTextureTransform)
					SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, DetailTextureTransform, DetailScale);
				else
					SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02 + nRegisterShift, BaseTextureTransform, DetailScale);			
			}
		}
	}
#endif

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

			float4 f4SwayParams2;
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

			float4 f4SwayParams4;
			f4SwayParams4.x = GetFloat(TreeSwaySpeed);
			f4SwayParams4.y = GetFloat(TreeSwayStrength);
			f4SwayParams4.z = GetFloat(TreeSwayScrumbleFrequency);
			f4SwayParams4.w = GetFloat(TreeSwayScrumbleStrength);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_04, f4SwayParams4);

			// More precomputation
			float4 f4SwayParams5;
			f4SwayParams5.x = sqrtf(f4SwayParams2.z * f4SwayParams2.z + f4SwayParams2.w * f4SwayParams2.w); // Length of the Wind Direction
			float SpeedLerpLow = GetFloat(TreeSwaySpeedLerpStart);
			float SpeedLerpHigh = GetFloat(TreeSwaySpeedLerpEnd);
			f4SwayParams5.y = smoothstep(SpeedLerpLow, SpeedLerpHigh, f4SwayParams5.x);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_VERTEXSWAY_05, f4SwayParams5);
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b3
		if(HasFlag(MATERIAL_VAR_VERTEXALPHA))
			BBools[UNLITGENERIC_BOOL_VERTEXALPHA] = true;

		// b12
		if(HasFlag(MATERIAL_VAR_VERTEXCOLOR))
			BBools[LUX_PS_BOOL_VERTEXCOLOR] = true;

		// b13, b14, b15
#if FIXED_COMMANDBUFFER
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);
#else
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);
#endif
		// Always!
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		if (bProjTex)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(lux_vertexlitgeneric_flashlight_ps30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
		}

#if FIXED_COMMANDBUFFER
//		pShaderAPI->ExecuteCommandBuffer(StaticCmds.Base());
		pShaderAPI->ExecuteCommandBuffer(SemiStaticCmds.Base());
#endif
	}

	// We are not done here!
	// The Command Buffer blocks us from using ConVars.
	// Let's overwrite the Constants now. This is fine since we'd only ever use any of these for debugging.
	//==========================================================================//
	// ConVars
	//==========================================================================//
	if (IsDynamicState())
	{
		// Do not disable Diffuse! This is used on UI's

#ifdef LUX_DEBUGCONVARS
		if (bHasEnvMap && lux_disablefast_envmap.GetBool())
		{
			BindTexture(SAMPLER_ENVMAPTEXTURE, TEXTURE_BLACK);
		}
#endif
	}

	Draw();
}
END_SHADER