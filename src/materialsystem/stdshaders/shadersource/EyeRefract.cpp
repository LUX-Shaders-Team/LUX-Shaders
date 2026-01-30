//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

#include "renderpasses/Cloak.h"
#include "renderpasses/EmissiveBlend.h"

// Includes for Shaderfiles...
#include "lux_eyes_vs30.inc"
#include "lux_eyes_ps30.inc"

// LUX Shaders will replace existing Shaders.
// Eyes Shader were reported to be obsolete by the VDC but that is wrong
// Due to some EyeRefract Bugs that affecti other Materials ( like Water ),
// it is commonly used as a less sophisticated Version that comes without the attached Issues
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Eyes, LUX_Eyes)
DEFINE_FALLBACK_SHADER(SDK_Eyes_dx8, LUX_Eyes)
DEFINE_FALLBACK_SHADER(SDK_EyeRefract, LUX_EyeRefract)
#endif

#ifdef REPLACE_EYES
DEFINE_FALLBACK_SHADER(Eyes, LUX_Eyes)
DEFINE_FALLBACK_SHADER(Eyes_dx8, LUX_Eyes)
#endif

#ifdef REPLACE_EYEREFRACT
DEFINE_FALLBACK_SHADER(EyeRefract, LUX_EyeRefract)
#endif

// Does this even exist?
// DEFINE_FALLBACK_SHADER(Eyeball, Wireframe) // This should work...

// TODO: This Shader isn't done yet
//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_EyeRefract, "A Shader that makes 'life-like' Eyes.")
SHADER_INFO_GEOMETRY	("Models.")
SHADER_INFO_USAGE		("..")
SHADER_INFO_LIMITATIONS	("..")
SHADER_INFO_PERFORMANCE	("..")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC EyeRefract Shader Page: https://developer.valvesoftware.com/wiki/EyeRefract")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

	SHADER_PARAM(Iris,			SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Iris Color.\n[A] Cornea Noise.")
	SHADER_PARAM(IrisFrame,		SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Iris")
	SHADER_PARAM(EyeOrigin,		SHADER_PARAM_TYPE_VEC3, "", "Center of the Eye Ball that this Material controls.\n Probably set automatically!")
	SHADER_PARAM(IrisU,			SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord U Matrix")
	SHADER_PARAM(IrisV,			SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord V Matrix")
	SHADER_PARAM(Dilation,		SHADER_PARAM_TYPE_FLOAT, "", "How far Pupils are dilated.\n0 = None.\n1 = Fully dilated\n")
	SHADER_PARAM(Intro,			SHADER_PARAM_TYPE_BOOL,  "", "Enables an Effect made for the Half-Life 2 Episode 1 Intro.")
	SHADER_PARAM(EntityOrigin,	SHADER_PARAM_TYPE_VEC3,  "", "Requires $Intro. Should be the Center of the Model in World Space.")
	SHADER_PARAM(WarpParam,		SHADER_PARAM_TYPE_FLOAT, "", "Intended to use CurTime.")

BEGIN_SHADER_PARAMS
	// All of these Parameters are found on the Eyes Shader
	SHADER_PARAM(Iris,					SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Iris Color.\n[A] Cornea Noise on EyeRefract.\n[A] Lerp Factor for $BaseTexture on Eyes.")
	SHADER_PARAM(IrisFrame,				SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Iris")
	SHADER_PARAM(EyeOrigin,				SHADER_PARAM_TYPE_VEC3, "", "Center of the Eye Ball that this Material controls.\n Probably set automatically!")	
	SHADER_PARAM(IrisU,					SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord U Matrix")
	SHADER_PARAM(IrisV,					SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord V Matrix")
	SHADER_PARAM(Dilation,				SHADER_PARAM_TYPE_FLOAT, "", "How far Pupils are dilated.\n0 = None.\n1 = Fully dilated\n")
	SHADER_PARAM(Intro,					SHADER_PARAM_TYPE_BOOL,  "", "Enables an Effect made for the Half-Life 2 Episode 1 Intro.")
	SHADER_PARAM(EntityOrigin,			SHADER_PARAM_TYPE_VEC3,  "", "Requires $Intro. Should be the Center of the Model in World Space.")
	SHADER_PARAM(WarpParam,				SHADER_PARAM_TYPE_FLOAT, "", "Intended to use CurTime.")	

	// EyeRefract specific Params
	SHADER_PARAM(Glossiness,			SHADER_PARAM_TYPE_FLOAT, "", "Brightness Factor for $EnvMap. Works exactly the same as $EnvMapTint on other Shaders.")
	SHADER_PARAM(RayTraceSphere,		SHADER_PARAM_TYPE_BOOL,  "", "Used for Eye Balls that are NOT spherical. Reduces Artifacts on Pixels outside the actual Sphere made from $EyeBallRadius.")
	SHADER_PARAM(SphereTexKillCombo,	SHADER_PARAM_TYPE_BOOL,  "", "Requires $RayTraceSphere, causes Pixels outside the Sphere to be discarded.")
	SHADER_PARAM(ParallaxStrength,		SHADER_PARAM_TYPE_FLOAT, "", "Modulation Factor for the Parallax Effect on the Iris. Should be >= 0")
	SHADER_PARAM(CorneaTexture,			SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Normal Map.\n[A] 'Highlight Mask' ( Intensity Factor for Lighting ).")
	SHADER_PARAM(CorneaBumpStrength,	SHADER_PARAM_TYPE_FLOAT, "", "Strength Factor for the Normal Map contained in $CorneaTexture. Should be >= 0")
	SHADER_PARAM(AmbientOcclTexture,	SHADER_PARAM_TYPE_TEXTURE, "", "[Grey or RGB] Ambient Occlusion Factor ( Only for **Diffuse** Lighting, Specular Lighting is unaffected by this Texture! )")
	SHADER_PARAM(AmbientOcclColor,		SHADER_PARAM_TYPE_VEC3,   "", "The Color that occluded Areas in the $AmbientOcclTexture will have.")
	SHADER_PARAM(EyeBallRadius,			SHADER_PARAM_TYPE_FLOAT,  "", "The desired Radius of the EyeBall Sphere, as measured from $EyeOrigin.")
	
	SHADER_PARAM(LightWarpTexture,		SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Tints texels depending on their brightness (See VDC for more Information).\n[A] Unused.")

	// Proper EnvMap Support
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()

	// Emissive Blend Support
	Declare_EmissiveBlendParameters()

	// Cloak Blended Pass Support
	Declare_CloakParameters()

	SHADER_PARAM(BumpMap,				SHADER_PARAM_TYPE_TEXTURE, "", "(INTERNAL PARAMETER) This Parameter is not used on this Shader, it's just here for some wizardry Reasons.")
END_SHADER_PARAMS

void ER_SetupCloakVars(Cloak_Vars_t& CloakVars)
{
	CloakVars.InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	CloakVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);

	// Errr ackchually, this is wrong.
	// CorneaTexture is a Normal Map that could be used with this.
	// However the Cloak Pass doesn't know how to handle the Eye Shaders projection Math.
	// Therefore we cannot use it and are forced to make do with the VNormal
	CloakVars.Bump.InitVars(-1);
}

void ER_SetupEmissiveBlendVars(EmissiveBlend_Vars_t& EmissiveVars)
{
	// Emissive Blend Params
	EmissiveVars.InitVars(EmissiveBlendEnabled);

	// Not supported
	EmissiveVars.SelfIllum.m_nSelfIllumTexture = -1;
	EmissiveVars.SelfIllum.m_nSelfIllumTextureFrame = -1;

	// Not supported
	EmissiveVars.Detail.m_nDetail = -1;

	// Minimum Light and Transform Fallbacks
	EmissiveVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
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

	return IS_FLAG2_DEFINED(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
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

	return IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT);
}

// Set Up Vars here
void EyeRefractShaderFlags()
{
	// EyeRefract sets these two Flags
	// Skinning is needed since this is used on Models ( that might be skinned )
	// LightingVertexLit is needed for Lighting
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

	// I'm setting these Flags too.
	// Stock EyeRefract is a Per-Pixel lit Shader ( like VertexLitGeneric with BumpMaps )
	// We should really also be setting these Flags. That's what VertexLitGeneric does.
	SetFlag(MATERIAL_VAR_MODEL);
	SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);
}

SHADER_INIT_PARAMS()
{
	Cloak_Vars_t CloakVars;
	ER_SetupCloakVars(CloakVars);
	CloakBlend_Init_Params(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	ER_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Init_Params(this, EmissiveVars);

	// Flags
	EyeRefractShaderFlags();

	// Should be set to *anything* for Per-Pixel Lighting
	SetString(BumpMap, "..");
}

SHADER_FALLBACK
{
	// OVERRIDE
	// TODO: REMOVE LATER WHEN THIS SHADER WAS DONE
	return "EyeRefract";

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
	ER_SetupCloakVars(CloakVars);
	CloakBlend_Shader_Init(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	ER_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Shader_Init(this, EmissiveVars);

	LoadTexture(Iris, TEXTUREFLAGS_SRGB);
	LoadTexture(AmbientOcclTexture, TEXTUREFLAGS_SRGB);

	LoadCubeMap(EnvMap);
	LoadTexture(EnvMapMask);
}

SHADER_DRAW
{
	
}
END_SHADER

// Eyes is not obsoleted like the VDC claims it to be, so we have it.

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Eyes, "A shader used to create life-like eyes. (For more complex effects, use EyeRefract).")
SHADER_INFO_GEOMETRY	("Models.")
SHADER_INFO_USAGE		("Apply a Material to the model mesh.")
SHADER_INFO_LIMITATIONS	("TODO: DES")
SHADER_INFO_PERFORMANCE	("Cheap to render.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Eye Shader Page: https://developer.valvesoftware.com/wiki/Eyes")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS


	// All of these Parameters are found on the EyeRefract Shader
	SHADER_PARAM(Iris,		SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Iris Color.\n[A] Cornea Noise on EyeRefract.\n[A] Lerp Factor for $BaseTexture on Eyes.")
	SHADER_PARAM(IrisFrame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $Iris")
	SHADER_PARAM(EyeOrigin,	SHADER_PARAM_TYPE_VEC3, "", "Center of the Eye Ball that this Material controls.\n Probably set automatically!")
	SHADER_PARAM(IrisU,		SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord U Matrix")
	SHADER_PARAM(IrisV,		SHADER_PARAM_TYPE_VEC4, "", "WorldSpace-Position TexCoord V Matrix")

	// Not used on the Eyes Shader
//	SHADER_PARAM(Dilation,		SHADER_PARAM_TYPE_FLOAT, "", "How far Pupils are dilated.\n0 = None.\n1 = Fully dilated\n")

	SHADER_PARAM(Intro,			SHADER_PARAM_TYPE_BOOL,  "", "Enables an Effect made for the Half-Life 2 Episode 1 Intro.")
	SHADER_PARAM(EntityOrigin,	SHADER_PARAM_TYPE_VEC3,  "", "Requires $Intro. Should be the Center of the Model in World Space.")
	SHADER_PARAM(WarpParam,		SHADER_PARAM_TYPE_FLOAT, "", "Intended to use CurTime.")

	SHADER_PARAM(Glint,			SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Additive Glint Color. ( This behaves similar to $DetailBlendMode 5 ).\n[A] Nothing.")
	SHADER_PARAM(GlintFrame,	SHADER_PARAM_TYPE_INTEGER,  "", "Frame Var for $GlintFrame.")
	SHADER_PARAM(GlintU,		SHADER_PARAM_TYPE_VEC4,		"", "WorldSpace-Position TexCoord U Matrix")
	SHADER_PARAM(GlintV,		SHADER_PARAM_TYPE_VEC4,		"", "WorldSpace-Position TexCoord V Matrix")
	SHADER_PARAM(EyeUp,			SHADER_PARAM_TYPE_VEC3,		"", "Up Vector for the Eyes. Changes how the Normal Map behaves.")

	// Emissive Blend Support
	Declare_EmissiveBlendParameters()

	// Cloak Blended Pass Support
	Declare_CloakParameters()
END_SHADER_PARAMS

void Eyes_SetupCloakVars(Cloak_Vars_t& CloakVars)
{
	CloakVars.InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	CloakVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);

	// Errr ackchually, this is wrong.
	// CorneaTexture is a Normal Map that could be used with this.
	// However the Cloak Pass doesn't know how to handle the Eye Shaders projection Math.
	// Therefore we cannot use it and are forced to make do with the VNormal
	CloakVars.Bump.InitVars(-1);
}

void Eyes_SetupEmissiveBlendVars(EmissiveBlend_Vars_t& EmissiveVars)
{
	// Emissive Blend Params
	EmissiveVars.InitVars(EmissiveBlendEnabled);

	// Not supported
	EmissiveVars.SelfIllum.m_nSelfIllumTexture = -1;
	EmissiveVars.SelfIllum.m_nSelfIllumTextureFrame = -1;

	// Not supported
	EmissiveVars.Detail.m_nDetail = -1;

	// Minimum Light and Transform Fallbacks
	EmissiveVars.Base.InitVars(BaseTexture, Frame, BaseTextureTransform);
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

	return IS_FLAG2_DEFINED(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
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

	return IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT);
}

// Set Up Vars here
void EyesShaderFlags()
{
	// Eyes sets these two Flags
	// Skinning is needed since this is used on Models ( that might be skinned )
	// LightingVertexLit is needed for Lighting
	SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
}

SHADER_INIT_PARAMS()
{
	Cloak_Vars_t CloakVars;
	Eyes_SetupCloakVars(CloakVars);
	CloakBlend_Init_Params(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	Eyes_SetupEmissiveBlendVars(EmissiveVars);

	EmissiveBlend_Init_Params(this, EmissiveVars);
	EyesShaderFlags();
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
	Cloak_Vars_t CloakVars;
	Eyes_SetupCloakVars(CloakVars);
	CloakBlend_Shader_Init(this, CloakVars);

	EmissiveBlend_Vars_t EmissiveVars;
	Eyes_SetupEmissiveBlendVars(EmissiveVars);
	EmissiveBlend_Shader_Init(this, EmissiveVars);

	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadTexture(Iris, TEXTUREFLAGS_SRGB);
	LoadTexture(Glint);
}

// NOTE: DrawFunction is separate here because we Cloak
// It's just easier to do this way.
void DrawEyes(IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, CBasePerMaterialContextData** pContextDataPtr)
{
	bool bProjTex = HasFlashlight();

	// NOTE: This Shader doesn't use $BaseTexture Alpha
	// We can safely check it for Opacity
	BlendType_t nBlendType = ComputeBlendType(BaseTexture, true);
	bool bIsFullyOpaque = IsFullyOpaque(nBlendType);

	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasIrisTexture = IsTextureLoaded(Iris);
	bool bHasGlintTexture = IsTextureLoaded(Glint);
	bool bUseIntro = GetBool(Intro);

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
	
		// Can do DefaultFog here since we allow for AlphaBlending
		DefaultFog();
	
		// Usually need it for DepthToDestAlpha
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);
	
		// By default we write linear Values and need them converted to sRGB
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Need Position ( of course ), Normals and those are allowed to be compressed
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
		int nTexCoords = 1;
		int nUserDataSize = 0;
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// "On DX9, get the gamma read and write correct"

		// s0 - $BaseTexture. Always sRGB
		EnableSampler(SHADER_SAMPLER0, true);

		// s1 - Iris. Always sRGB
		EnableSampler(SHADER_SAMPLER1, true);

		// s2 - Glint. Linear
		EnableSampler(SHADER_SAMPLER2, false);

		// s13, s14, s15
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_eyes_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(INTRO, bUseIntro);
		SET_STATIC_VERTEX_SHADER_COMBO(PROJTEX, bProjTex);
		SET_STATIC_VERTEX_SHADER(lux_eyes_vs30);		

		DECLARE_STATIC_PIXEL_SHADER(lux_eyes_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(PROJTEX, bProjTex);
		SET_STATIC_PIXEL_SHADER(lux_eyes_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		
		// Always bind Fallbacks!
		BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, Frame, TEXTURE_GREY);
		BindTexture(bHasIrisTexture, SHADER_SAMPLER1, Iris, IrisFrame, TEXTURE_GREY);
		BindTexture(bHasGlintTexture, SHADER_SAMPLER2, Glint, GlintFrame, TEXTURE_BLACK); // Gets added additively, so Black Fallback

		//==========================================================================//
		// Constant Registers
		//==========================================================================//

		// VS Constants
		// Always need these ones
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_0,EyeOrigin);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_1,EyeUp);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_2,IrisU);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_3,IrisV);

		if(bUseIntro)
		{
			float4 cIntroControls = 0.0f;
			cIntroControls.xyz = GetFloat3(EntityOrigin);
			cIntroControls.w = GetFloat(WarpParam);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_SET1_6, cIntroControls);
		}
		else
		{
			SetVertexShaderConstant(LUX_VS_FLOAT_SET1_4, GlintU);
			SetVertexShaderConstant(LUX_VS_FLOAT_SET1_5, GlintV);		
		}

		// c0
		// Stock-Consistency: Replicating this 1:1 to avoid visual Disparities
		float f1GlintDamping = max(0.0f, min(pShaderAPI->GetAmbientLightCubeLuminance(), 1.0f));
		const float f1DimGlint = 0.01f;

		// "Remap so that glint damping smooth steps to zero for low luminances"
		float4 cEyeGlint;
		if(f1GlintDamping > f1DimGlint)
			cEyeGlint = 1.0f;
		else
			cEyeGlint = f1GlintDamping * SimpleSplineRemapVal(f1GlintDamping, 0.0f, f1DimGlint, 0.0f, 1.0f);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, cEyeGlint);

		// c26 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
		
		// c27 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c28
		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c31 - $Color, $Color2, $sRGBTint
		float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);

		// disablefast Support
		if(lux_disablefast_diffuse.GetBool())
			f4Tint.rgb = 0.0f;

		pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(bIsFullyOpaque);

		// Always!
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		// Vertex Shader BOOLs
		BOOL BHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);
		pShaderAPI->SetBooleanVertexShaderConstant(LUX_VS_BOOL_HALFLAMBERT, &BHalfLambert);

		// Send ProjTex Data
		bool bProjTexShadows = SetupFlashlight();

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		bool bHasStaticPropLighting = 0;
		bool bHasDynamicPropLighting = 0;

		// Dynamic Prop Lighting here refers to dynamic vertex lighting, or ambient cubes via the vertex shader
		// We shouldn't have that on bumped or phonged models. Same for Static Vertex Lighting
		if (!bProjTex)
		{
			// LightState is always fully Dynamic, and we always need it.
			LightState_t LightState;
			pShaderAPI->GetDX9LightState(&LightState);

			// LightState varies between SP and MP so we use a function to reinterpret
			bHasStaticPropLighting = StaticLightVertex(LightState);
			bHasDynamicPropLighting = (LightState.m_bAmbientLight || (LightState.m_nNumLights > 0)) ? 1 : 0;

			// Need to send this to the Vertex Shader manually in this scenario
			if (bHasDynamicPropLighting)
				pShaderAPI->SetVertexShaderStateAmbientLightCube();
		}

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_eyes_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, bHasStaticPropLighting);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, bHasDynamicPropLighting);
		SET_DYNAMIC_VERTEX_SHADER(lux_eyes_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_eyes_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bProjTexShadows);
		SET_DYNAMIC_PIXEL_SHADER(lux_eyes_ps30);
	}

	Draw();
}

SHADER_DRAW
{
	bool bDrawBasePass = true;

	Cloak_Vars_t CloakVars;
	Eyes_SetupCloakVars(CloakVars);
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
		DrawEyes(pShaderShadow, pShaderAPI, pContextDataPtr);
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