//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	01.01.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_core_vs30.inc"
#include "lux_core_ps30.inc"

#define MAXBLUR 1

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_CORE, LUX_CORE)
#endif

#ifdef REPLACE_CORE
DEFINE_FALLBACK_SHADER(CORE, LUX_CORE)
#endif

//==========================================================================//
// Shader Start LUX_Core
//==========================================================================//
BEGIN_VS_SHADER(LUX_Core, "A shader used to create a unstable, reflecting and refracting core of energy, used exclusively for the Core reactor in the Citadel in Episode One.")
SHADER_INFO_GEOMETRY	("prop_coreball entity.")
SHADER_INFO_USAGE		("Create a prop_coreball entity.")
SHADER_INFO_LIMITATIONS	("Designed around spherical Models with a set $SphereRadius.")
SHADER_INFO_PERFORMANCE	("Pretty ok Performance.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Core Shader Page: https://developer.valvesoftware.com/wiki/Core_(shader)")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	Declare_EnvironmentMapParameters()
	SHADER_PARAM_OVERRIDE(Color1, SHADER_PARAM_TYPE_COLOR,	"{255 255 255}", "(INTERNAL PARAMETER), dont use.", SHADER_PARAM_NOT_EDITABLE)
	SHADER_PARAM_OVERRIDE(Alpha, SHADER_PARAM_TYPE_FLOAT,	"1.0",			 "(INTERNAL PARAMETER), dont use.", SHADER_PARAM_NOT_EDITABLE)

	SHADER_PARAM(NormalMap,				SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Normal Map.\n[A] Refract Scale, EnvMapMask and Opacity.") 
	SHADER_PARAM(BumpFrame,				SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $NormalMap.")
	SHADER_PARAM(BumpTransform,			SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the $BumpMap or $NormalMap Texture. Must include all Values!")
	SHADER_PARAM(RefractAmount,			SHADER_PARAM_TYPE_FLOAT,	"", "How Strong Refractions should be")
	SHADER_PARAM(RefractTint,			SHADER_PARAM_TYPE_COLOR,	"", "Tints the Diffuse Part of the Result ( Refracted Background )")
	SHADER_PARAM(Time,					SHADER_PARAM_TYPE_FLOAT,	"", "Allows you to hook a (optional) custom Time-Variable to the Shader.")
	SHADER_PARAM(FlowMap,				SHADER_PARAM_TYPE_TEXTURE,	"", "[RG] Flow Velocity that skews and scrolls the $NormalMap.")
	SHADER_PARAM(FlowMapFrame,			SHADER_PARAM_TYPE_INTEGER,	"", "Frame number for $Flowmap.") 
	SHADER_PARAM(FlowMapScrollRate,		SHADER_PARAM_TYPE_VEC2,		"", "2D Rate/Scalar for $FlowMap scroll.")
	SHADER_PARAM(CoreColorTexture,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Resulting Color.\n[A] Lerp Factor, how much Result will be replaced with the RGB Values.")
	SHADER_PARAM(CoreColorTextureFrame, SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number for $CoreColorTexture.")
	SHADER_PARAM(FlowMapTexCoordOffset, SHADER_PARAM_TYPE_FLOAT,	"", "Offsets for the $FlowMap UV.")

	// This is technically a mapbase featured
	// However I need it for much different reasons than mapbase...
	SHADER_PARAM(SphereRadius, SHADER_PARAM_TYPE_FLOAT,	"", "Radius ( Not Diameter ) of the Core Prop, Default is 215.0f.\n Stock Shader uses 231.197f internally, which is NOT the actual Radius of the Model.")

END_SHADER_PARAMS

void CoreShaderFlags()
{
	// Stock Shader does not do this
//	SetFlag(MATERIAL_VAR_MODEL);

	// Always AlphaBlending in some Way
	SetFlag(MATERIAL_VAR_TRANSLUCENT);

	// Only need Tangents for EnvMaps but we just always indicate this
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// If we don't have a "BaseTexture" (RenderTarget) defined
	// We have to nicely ask for it (brute force)
	// core_sheet.vmt doesn't specify a $BaseTexture. So this is pretty much the Default.
	if (!IsDefined(BaseTexture))
		SetFlag2(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
}

SHADER_INIT_PARAMS()
{
	CoreShaderFlags();

	// This is the radius of the stock core model
//	DefaultFloat(SphereRadius, 231.197f)

	// Mapbase's VMT sets a Radius of 215
	// Likely using the hardcoded Value from the original Shader.
	// Which happens to be 2.0f * 215.0f
	// Where does this Number come from?
	// It's not the real Radius of the Model!
	// I account for this now, the real Radius is 231.196f
	if (IsDefined(SphereRadius) && GetFloat(SphereRadius) == 215.0f)
		SetFloat(SphereRadius, 231.196f);

	// This is the radius they used instead
	DefaultFloat(SphereRadius, 215.0f);

	// Don't want greyscale envmaps by default
	DefaultFloat(EnvMapSaturation, 1.0f);
}

SHADER_FALLBACK
{
#ifndef REPLACE_CORE
	if (lux_oldshaders.GetBool())
		return "Core";
#endif

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
		// Funfact, the stock coreball shader returns to "Core_dx90"...
		// WHICH DOESN'T EXIST
		// Whoever wrote that shader didn't bother to read their own code
	}
	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture);
	LoadBumpMap(NormalMap);
	LoadCubeMap(EnvMap);
	LoadTexture(FlowMap);
	LoadTexture(CoreColorTexture);
}

SHADER_DRAW
{
	// If we are using a flashlight
	// ( which this shader doesn't support )
	// Just don't draw anything for both passes
	if (UsingFlashlight())
	{
		Draw(false);
		Draw(false);
		return;
	}

	// Set up some booleans
	//===============================//
	bool bHasFramebuffer = IsTextureLoaded(BaseTexture);
	bool bHasNormalMap = IsTextureLoaded(NormalMap);
	bool bHasEnvMap = IsTextureLoaded(EnvMap);
	bool bHasFlowMap = IsTextureLoaded(FlowMap);
	bool bHasColorTexture = IsTextureLoaded(CoreColorTexture);
	bool bHasRefract = true;

	// Instead of outsourcing the code to a new function, just do it twice!!
	for (int nPass = 0; nPass != 1; nPass++)
	{
		//==========================================================================//
		// Static Snapshot of the Shader Settings
		//==========================================================================//
		if(IsSnapshotting())
		{
			// core_sheet doesn't even have $alphatest
			// What the hell is the point of this
			// What we are essentially doing right now is
			// layer (top)		= transparent
			// layer (bottom)	= opaque ( its not alphatested )
			if (nPass == 0)
				pShaderShadow->EnableAlphaTest(HasFlag(MATERIAL_VAR_ALPHATEST));
			else
			{
				pShaderShadow->DepthFunc(SHADER_DEPTHFUNC_EQUAL);
				EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
			}

			// Keeping this consistent with the pixel shader now
			bHasEnvMap			= bHasEnvMap && (nPass == 1);
			bHasRefract			= (nPass == 0);
			bHasColorTexture	= (nPass == 0) && bHasColorTexture;

			// "If envmap is not specified, the alpha channel is the translucency
			// (If envmap *is* specified, alpha channel is the reflection amount)"
			// What this is supposed to mean :
			// Since our "BaseTexture" is the framebuffer Texture
			// We cannot use it to determine overall transparency
			// Therefore we have to use the normal maps alpha for it instead.
			// Whats the alternative..................???
			// Well there is only one Material that uses this shader and it has $EnvMap
			// Meaning this code is utterly useless to be here and we have a potentially unconsidered scenario
			if (bHasNormalMap && !bHasEnvMap)
				SetDefaultBlendingShadowState(NormalMap, false);

			//===============================//
			// Enable Sampler
			//===============================//

			// s0 - $BaseTexture ( Framebuffer )
			// OVERSIGHT: Stock uses sRGBRead when HDR Type == INTEGER
			// This not only ignores float HDR but also doesn't make sense.
			// Framebuffers are always sRGB!!
			EnableSampler(SHADER_SAMPLER0, true);

			// s1 - FlowMap. Never sRGB
			EnableSampler(SHADER_SAMPLER1, false);

			// s2 - Normal Map. Never sRGB
			EnableSampler(SHADER_SAMPLER2, false);

			// s3 - $CoreColorTexture. Stock-Consistency: Despite being a Color, not sRGB
			EnableSampler(bHasColorTexture, SHADER_SAMPLER3, false);

			// EnvMaps on LDR are sRGB, HDR ones are Linear
			// Stock Shader does sRGBRead when HDR!!! OVERSIGHT!!	
			EnableSampler(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, !IsHDREnabled());

			//===============================//
			// Configure the Shader
			//===============================//

			if (g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE)
				pShaderShadow->EnableSRGBWrite(true);

			// This is only ever used on (A SINGLE) model(s), so vertex compression must be in the flags
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			
			// We need tangents, so throw a 4
			pShaderShadow->VertexShaderVertexFormat(flags, 1, NULL, 4);

			DefaultFog();

			//===============================//
			// Set static shaders
			//===============================//
			DECLARE_STATIC_VERTEX_SHADER(lux_core_vs30);
			SET_STATIC_VERTEX_SHADER(lux_core_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_core_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(FLOWMAPPING, bHasFlowMap);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, bHasEnvMap);
			SET_STATIC_PIXEL_SHADER_COMBO(CORECOLOR, bHasColorTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(REFRACTION, bHasRefract);
			SET_STATIC_PIXEL_SHADER(lux_core_ps30);
		}

		//==========================================================================//
		// Entirely Dynamic Commands
		//==========================================================================//
		if(IsDynamicState())
		{
			// We are rendering multiple passes so default the state
			pShaderAPI->SetDefaultState();

			//===============================//
			// Bind Textures
			//===============================//

			// NOTE: This is just $BaseTexture so animation via $Frame **IS** possible
			// However using that on a Framebuffer sounds like UB
			if (bHasFramebuffer)
				BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

			if (bHasFlowMap)
				BindTexture(SHADER_SAMPLER1, FlowMap, FlowMapFrame);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER1, TEXTURE_BLACK);

			// NormalMap Alpha used for Masking, so need a Fallback.
			if (bHasNormalMap)
				BindTexture(SHADER_SAMPLER2, NormalMap, BumpFrame);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER2, TEXTURE_NORMALMAP_FLAT);
			
			if (bHasColorTexture)
				BindTexture(SHADER_SAMPLER3, CoreColorTexture, CoreColorTextureFrame);

			if (bHasEnvMap)
				BindTexture(SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

			//==========================================================================//
			// Constant Registers
			//==========================================================================//

			// VS c223, c224 - $BumpTransform
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BumpTransform);

			// c11 - Camera Position
			SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

			// c12 - Fog Params
			pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

			// c32 - $RefractTint
			if (bHasRefract)
			{
				float4 f4RefractTint = 0.0f;
				f4RefractTint.rgb = GetFloat3(RefractTint);

				// Gamma To Linear... here we go again...
				if (IsHDREnabled())
					f4RefractTint.rgb = GammaToLinearTint(f4RefractTint.rgb);

				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_032, f4RefractTint);
			}

			// c33 - $FlowMapScrollRate, $FlowMapTexCoordOffset, $RefractAmount
			float4 f4Controls = 0.0f;
			if (bHasRefract || bHasFlowMap)
			{
				f4Controls.xy = GetFloat2(FlowMapScrollRate);
				f4Controls.z = GetFloat(FlowMapTexCoordOffset);
				f4Controls.xy *= pShaderAPI->CurrentTime(); // Precomputation, this would otherwise happen on the shader
	
				// Used for Refraction Offset Scale
				if (bHasRefract || bHasColorTexture)
					f4Controls.w = GetFloat(RefractAmount);

				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_033, f4Controls);
			}
			
			// c34 - $SphereRadus
			float4 f4SphereRadius = GetFloat(SphereRadius);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_034, f4SphereRadius);

			// FIXME: If we aren't using Functions from lux_common_envmap then fix the Register Layout.
			// c37, c38
			if (bHasEnvMap)
			{
				float4 f4EnvMapTint = 0.0f;
				f4EnvMapTint.rgb = GetFloat3(EnvMapTint);

				// Gamma To Linear... here we go again...
				if (IsHDREnabled())
					f4EnvMapTint.rgb = GammaToLinearTint(f4EnvMapTint.rgb);

				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint);

				float4 f4EnvMapSaturation_Contrast;
				f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation);
				f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);
			}

			// Prepare boolean array, yes we need to use BOOL
			BOOL BBools[REGISTER_BOOL_MAX] = { false };

			// b13, b14, b15
			// We don't have Alpha Writes
			BBools[LUX_PS_BOOL_HEIGHTFOG] = false;
			BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = false;
			pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

			//==================================================================================================
			// Set Dynamic Shaders
			//==================================================================================================
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_core_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_core_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_core_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_core_ps30);
		}

		Draw();
	}
}
END_SHADER