//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// You must define a TONEMAP_SCALE_X on your Shader
// #define TONEMAP_SCALE_NONE
// #define TONEMAP_SCALE_LINEAR
// #define TONEMAP_SCALE_GAMMA

// These 'Features' can be disabled by defining these Identifiers 
// Fog					- NO_FOG
// DepthToDestAlpha		- NO_DEPTHTODESTALPHA
// WaterFogTODestAlpha	- NO_WATERFOGTODESTALPHA

#ifndef LUX_COMMON_PS_FXC_H_
#define LUX_COMMON_PS_FXC_H_

// For disabling Warnings
#include "lux_common_pragmas.h"

// Feature List
#include "lux_common_defines.h"

// Remap for Macros
#include "lux_registermap_hlsl.h"

// PixelShader Register Map
#include "lux_registermap_ps.h"

// Constants and Functions that can be shared between VS and PS
#include "lux_common_fxc.h"

// This is here so when Lighting Registers are moved ( like with MOVED_REGISTERS_LIGHTING )
// The ShaderCompiler doesn't complain about PixelShaderLightInfo not being declared anywhere
struct PixelShaderLightInfo
{
	float4 color;
	float4 pos;
};

//==========================================================================//
// PixelShader *Float* Constants ( below c32 )
//==========================================================================//
//														: register(c0);
//														: register(c1);
//														: register(c2);
//														: register(c3);
//														: register(c4);
//														: register(c5);
//														: register(c6);
//														: register(c7);
//														: register(c8);
//														: register(c9);
//														: register(c10);
//														: register(c11);
//														: register(c12);
//									|	Ambient Cube	: register(c13);
//									|	Ambient Cube	: register(c14);
//									|	Ambient Cube	: register(c15);
//									|	Ambient Cube	: register(c16);
//	Projected Texture Filter Tweaks	|	Ambient Cube	: register(c17);
//	Projected Texture Attenuations	|	Ambient Cube	: register(c18);
//	Projected Texture Color			|	LightData		: register(c19);
//	Projected Texture Position		|	LightData		: register(c20);
//	Projected Texture Matrix[0]		|	LightData		: register(c21);
//	Projected Texture Matrix[1]		|	LightData		: register(c22);
//	Projected Texture Matrix[2]		|	LightData		: register(c23);
//	Projected Texture Matrix[4]		|	LightData		: register(c24);
const float4 cColorConstants							: register(LUX_PS_FLOAT_LUMINANCE_GAMMA);
const float4 cEyePos									: register(LUX_PS_FLOAT_CAMERAPOSITION);
const float4 cFogParams									: register(LUX_PS_FLOAT_FOGPARAMETERS);
const float4 cModulationConstants						: register(LUX_PS_FLOAT_MODULATIONCONSTANTS);
const float4 cLinearFogColor							: register(STOCK_PS_FLOAT_LINEARFOGCOLOR);
const float4 cLightScale								: register(STOCK_PS_FLOAT_LIGHTSCALE);
const float4 cDefaultControls							: register(LUX_PS_FLOAT_DEFAULTCONTROLS);

//==========================================================================//
// Unpacking Constants
//==========================================================================//

// Color Constants (LUX)
// .rgb = Luminance Weights
// .w = Gamma Value ( 2.2, 2.4, whichever is set via ConVar )
#define g_f3LuminanceWeights	(cColorConstants.rgb)
#define g_f1GammaValue			(cColorConstants.w)
#define g_f1RcpGammaValue		(1.0f / cColorConstants.w)

// Camera Position
#define g_f3EyePos		(cEyePos.xyz)

// Fog Parameters
#define g_f1FogEndOverRange		(cFogParams.x)
#define g_f1WaterZ				(cFogParams.y)
#define g_f1FogMaxDensity		(cFogParams.z)
#define g_f1FogOORange			(cFogParams.w)

// Modulation Constants (LUX)
#define g_f1AlphaModulation			(cModulationConstants.x)
#define g_f1LightmapScaleFactor		(cModulationConstants.y)
#define g_f1HeightFogSwitch			(cModulationConstants.z)

// Linear Fog Parameters
#define g_f3FogColor				(cLinearFogColor.rgb)
#define OO_DESTALPHA_DEPTH_RANGE	(cLinearFogColor.w)

// Figured to Document what these are set to, using RenderDoc on Vulkan:
// Linear Tonemap Scale?
// I set mat_force_tonemap_scale to 0.5f and 0.75f and this Value was set to 0.502612 and 0.753906f
// At a ConVar Value of 1.0f it was at 1.0f
// I'm not sure why the Value is ever so slightly higher than what is set on the ConVar.
#define LINEAR_LIGHT_SCALE					(cLightScale.x)

// Set to 16.0f on HDR Maps
// Set to 4.594794f on LDR Maps
#define LIGHT_MAP_SCALE						(cLightScale.y)

// Set to 16.0f in HDR and 1.0f in LDR
// Makes sense since we go from RGBA16161616F to LDR Texture Formats
#define ENV_MAP_SCALE						(cLightScale.z)

// Gamma Tonemap Scale ( TonemapScale ^ rcp(2.2f)
// I set mat_force_tonemap_scale to 0.5f and 0.75f and this Value was set to 0.73147f and 0.879499f
// Both of these are very close to their ^rcp(2.2f) Values. 0.729740f and 0.877424f respectively
#define GAMMA_LIGHT_SCALE					(cLightScale.w)

// Shaders still have to set this but it's a Solution to needing Tints on every Shader
// Some Shaders may reinterpret cDefaultControls for other Purposes ( cloak_ps30 for Example )
#define g_f3DefaultTint			(cDefaultControls.rgb)
#define g_f1DefaultAlphaFactor	(cDefaultControls.w)

//==========================================================================//
// PixelShader *Boolean* Constant Registers
//==========================================================================//
/*
	*Critical* Notes:

	1. I *had to* define the entire Array, and use #define to give each Slot a dedicated Name
	The Compiler spit out "non-proper semantics" when I didn't 
	( This might work differently if you make a new bool and set it to the Value of the Register )

	2. Booleans Constants *ONLY* work in if()-Statements.
	Ternary Statements such as ( bool ? x : y ) will error!
	( This might work differently if you make a new bool and set it to the Value of the Register )

	3. Must use the Microsoft BOOL typedef in the C++ File
	This is likely a pre-2004 Consistency Thing

	4. if()-Statements *always* have an *else* Part in the ASM Instructions, even if you didn't explicitly write it out.
	This means, it's preferable to use a Boolean Constant for Things that would result in else Statements!
*/
// Always creating the entire Array of 16 bools.
// Using Macros to then give each Slots a new Name whenever it is needed
const bool		Bools[16]						: register(b0);


#define				g_bVertexColor				Bools[LUX_PS_BOOL_VERTEXCOLOR]

// The Shader need to consider $AlphaTest, $Translucent, etc. here.
#define				g_bHeightFog				Bools[LUX_PS_BOOL_HEIGHTFOG]
#define				g_bRadialFog				Bools[LUX_PS_BOOL_RADIALFOG]
#define				g_bDepthToDestAlpha			Bools[LUX_PS_BOOL_DEPTHTODESTALPHA]

//==========================================================================//
// PixelShader *Integer* Constant Registers
//==========================================================================//

/*
	*Critical* Notes:

	1. Integer Registers can only be used in for loops and 'repeat blocks'(?)

	2. 
		.x = Iteration Count ( How often to iterate )
		.y = Initial Value to start at
		.z = Increment for each Iteration ( Can these be negative? )
		.w = Nothing, in fact MS Documentation says it **MUST** be empty.
*/

// This starts at 0 and the max is Num Lights + 1 ( due to Array Indexing )
// This caused Lights to flicker when swapping with new Lights, it's been disabled since
/*
const int			g_nLightCountRegister : register(i0);
#define NUM_LIGHTS	(g_nLightCountRegister.x)
*/

//==========================================================================//
// PixelShader *Samplers*
//==========================================================================//
sampler Sampler_BaseTexture				: register(s0);

//==========================================================================//
// Color related Functions
//==========================================================================//

// Returns Luminance
// Weighted using either Rec. 709 HDTV or the NTSC Analog Television Standard
// Depends on the Value of lux_
float PerceptualLuminance(float3 f3Color)
{
	return dot(f3Color, g_f3LuminanceWeights); // dp3
}

// Ha! Stock Code in common_fxc.h uses hungarian notation the same way LUX does for these Functions
// Everywhere else it's either no notation or fl prefix.

// Linear -> Gamma. ^(1.0f / Gamma)
float3 LinearToGamma(float3 f3Linear)
{
	return pow(max(f3Linear, 0.0f), g_f1RcpGammaValue);
}
float3 LinearToGamma(float f1Linear)
{
	return pow(max(f1Linear, 0.0f), g_f1RcpGammaValue);
}

// Gamma -> Linear. ^Gamma
float3 GammaToLinear(float3 f3Gamma)
{
	return pow(max(f3Gamma, 0.0f), g_f1GammaValue);
}
float3 GammaToLinear(float f1Gamma)
{
	return pow(max(f1Gamma, 0.0f), g_f1GammaValue);
}

// Piecewise OETF's found in common_fxc.h as they don't require a Gamma Constant

//==========================================================================//
// $BlendTintByBaseAlpha
//==========================================================================//
void ComputeBlendTintByBaseAlpha(inout float4 f4BaseTexture, const float3 f3Tint, const float f1AlphaFactor)
{
	// Tinted Version of the Texture
	float3 f3TintedBaseTexture = f4BaseTexture.rgb * f3Tint;
	
	// BlendTintColorOverBase
	// This replces the BaseTexture with the Tint
	f3TintedBaseTexture	= lerp(f3TintedBaseTexture, f3Tint, f1AlphaFactor);
	
	// The Result will be masked by the BaseTexture Alpha
	f4BaseTexture.rgb = lerp(f4BaseTexture.rgb, f3TintedBaseTexture, f4BaseTexture.a);
}

//==========================================================================//
// $DesaturateWithBaseAlpha
//==========================================================================//
void ComputeDesaturateWithBaseAlpha(inout float4 f4BaseTexture, const float f1AlphaFactor)
{
	// Desaturated Versions of the BaseTextures
	float3 f3BWBaseTexture = PerceptualLuminance(f4BaseTexture.rgb);
	
	// Lerp with BaseAlpha * $DesaturateWithBaseAlpha as the Factor
	f4BaseTexture.rgb = lerp(f4BaseTexture.rgb, f3BWBaseTexture, f4BaseTexture.a * f1AlphaFactor);
}

//==========================================================================//
// Helper Function for $BlendTintByBaseAlpha and $DesaturateWithBaseAlpha
//==========================================================================//
void ComputeTintAndXByBaseAlpha(inout float4 f4BaseTexture, const float3 f3Tint, const float f1AlphaFactor,
							const bool bBlendTintByBaseAlpha, const bool bDesaturateWithBaseAlpha)
{
	if(bBlendTintByBaseAlpha)
	{
		ComputeBlendTintByBaseAlpha(f4BaseTexture, f3Tint, f1AlphaFactor);
	}
	else
	{
		f4BaseTexture.rgb *= f3Tint;

		if(bDesaturateWithBaseAlpha)
		{		
			ComputeDesaturateWithBaseAlpha(f4BaseTexture, f1AlphaFactor);
		}
	}
}

//==========================================================================//
// Vectors
//==========================================================================//

// Creates an incident Vector ( goes from Camera to World Position )
// Vector faces TOWARDS the Surface not away from it
float3 ComputeViewDir(float3 f3WorldPos)
{
	return normalize(f3WorldPos - g_f3EyePos);
}

//==========================================================================//
// Range/Radial Fog Factor
//==========================================================================//
float ComputeRangeFogFactor(const float3 f3WorldPos, const float f1Depth)
{
	float f1FogFactor = 0.0f;

	float f1DistanceFactor;
#if defined(RADIALFOG)
	if (g_bRadialFog) // Radial Fog
	{
		// This is semi-copied from Mapbase, which has taken it from whereever
		// I modified it a little, now the saturate Instruction is after the else Statement.
		// Which allows regular Range Fog to also use it. ( Avoid a duplicate Instructions )
		// I don't have Value Range at Hand, so I'm *hoping* this doesn't break anything.
		// You probably shouldn't have a Max Fog-Density above 1.0f anyways, and thus this should math out just fine
		f1DistanceFactor = distance(g_f3EyePos, f3WorldPos);
	}
	else // Regular Range Fog
#endif
	{
		f1DistanceFactor = f1Depth;	
	}

	// Reciprocal of the Fog Range.
	// This formats the Distance [Inches] to Linear Values [0..1] for the Lerp
	// To ensure not going > 1.0f, Saturate() is applied
	f1DistanceFactor *= g_f1FogOORange;
	f1FogFactor = saturate(min(g_f1FogMaxDensity, f1DistanceFactor - g_f1FogEndOverRange));

	// Stock-Consistency :
	// "squaring the factor will get the middle range mixing closer to hardware fog" - common_ps_fxc.h
	f1FogFactor *= f1FogFactor;

	return f1FogFactor;
}

//==========================================================================//
// SDK Height Fog Factor
//==========================================================================//
float ComputeHeightFogFactor_SDK(const float3 f3WorldPos, const float f1Depth)
{
	float f1DepthFromWater = g_f1WaterZ - f3WorldPos.z;
	
	// Distance to the Camera
	float f1DepthFromEye = g_f3EyePos.z - f3WorldPos.z;
	float f = saturate(f1DepthFromWater * (1.0 / f1DepthFromEye));
	
	// "$tmp.w is now the distance that we see through water."
	return saturate(f * f1Depth * g_f1FogOORange);
}

//==========================================================================//
// ASW Height Fog Factor
//==========================================================================//
float ComputeHeightFogFactor_ASW(const float3 f3WorldPos)
{
	// ShiroDkxtro2:
	// ASW Port, it's a cheaper Variant and supposedly doesn't have the "hard line" Issue
	// 
	// "This version is simply using the depth of the water to determine fog factor,
	// which is cheaper than doing the line integral and also fixes some problems with having 
	// a hard line on the shore when the water surface is viewed tangentially.
	// hackity hack . .the 2.0 is for the DF_FUDGE_UP in view_scene.cpp"
	return saturate((g_f1WaterZ - f3WorldPos.z - 2.0f) * g_f1FogOORange);
}

//==========================================================================//
// LUX Height Fog Factor
//==========================================================================//
float ComputeHeightFogFactor_LUX(const float3 f3WorldPos, const float f1Depth)
{
	// g_f1HeightFogSwitch1 will be 1 for ASW Water Fog Factor and 0 for SDK Water Fog Factor
	float f1DepthFromWater = g_f1WaterZ - f3WorldPos.z - 2 * g_f1HeightFogSwitch;

	// For SDK Water
	float f1DepthFromEye = g_f3EyePos.z - f3WorldPos.z;
	
	// Same as SDK now but without FogOORange and saturate()
	float f1SDKFactor = saturate(f1DepthFromWater * (1.0 / f1DepthFromEye));
	f1SDKFactor *= f1Depth;

	// ASW Water will be the previously derived Function
	float f1ASWFactor = f1DepthFromWater;

	// Can now lerp between the two. This should result in three additional MAD ( FMA ) Instructions in Total
	float f1FogFactor = lerp(f1SDKFactor, f1ASWFactor, g_f1HeightFogSwitch);

	// Apply Range and Saturate
	return saturate(f1FogFactor * g_f1FogOORange);
}

//==========================================================================//
// Helper Function for Height Fog Factor
//==========================================================================//
float ComputeHeightFogFactor(float3 f3WorldPos, const float f1Depth)
{
	//	return ComputeHeightFogFactor_SDK(f3WorldPos, f1Depth);
	//	return ComputeHeightFogFactor_ASW(f3WorldPos);
	return ComputeHeightFogFactor_LUX(f3WorldPos, f1Depth);
}

//==========================================================================//
// Output Functions
//==========================================================================//
float4 LUX_Finalise(const float4 f4Result, const float3 f3WorldPos = float3(0.0f, 0.0f, 0.0f),
						const float f1Depth = 1.0f, const float f1AlphaScale = 1.0f)
{
	float4 f4Final = f4Result;

	// Entities like func_areaportalwindow pass the desired transparency via the $Alpha Parameter
	// Shaders should pass g_f1AlphaModulation to the return Function to allow for this Behaviour
	// If not set, the Compiler will strip this out since it will default to 1.0f
	// This will also be 1.0f if the MATERIAL_VAR_NOALPHAMOD Flag was set or or $Alpha wasn't modified.
	// This was moved here because it was annoying to change the Comments on every Shader or rename the Variable.
	f4Final.a *= f1AlphaScale;

	//==========================================================================//
	// Apply ToneMap Scale.
	// Yes this happens in the forward Pass for Source. ( bad )
	// Shader must define a TONEMAP_SCALE_ Variant otherwise it won't get Tonemapping
	// Note that TONEMAP_SCALE_NONE doesn't actually exist
	//==========================================================================//
#if defined(TONEMAP_SCALE_LINEAR)
	f4Final.rgb *= LINEAR_LIGHT_SCALE;
#elif defined(TONEMAP_SCALE_GAMMA)
	f4Final.rgb *= GAMMA_LIGHT_SCALE;
#endif

	//==========================================================================//
	// Apply DepthToDestAlpha.
	// Can be disabled by defining NO_DEPTHTODESTALPHA
	//==========================================================================//
#if !defined(NO_DEPTHTODESTALPHA)
	// Only do this if we want to put it into the alpha
	if (g_bDepthToDestAlpha)
	{
		// Standard Depth Range in Source is very limited
		// An extended range can be used, but at larger values will cause inaccuracies near the player
		// This is here in-case you are doing just that!
		// NOTE: 4096 fits into the default Framebuffer without issues
		// Larger values ( such as 16384 ) won't and will require a new Framebuffer Format
		// See lux_common_depth.h for more information
		#if defined(CUSTOM_DEPTH_RANGE)
			f4Final.a = f1Depth * CUSTOM_DEPTH_RANGE;
		#else
			// The range of this is so small, you could throw a rock further than this
			f4Final.a = f1Depth * OO_DESTALPHA_DEPTH_RANGE;
		#endif
	}
	// else = .a is Alpha from Texture already
#endif

	//==========================================================================//
	// Apply Fog.
	// Can be disabled by defining NO_FOG
	//==========================================================================//
#if !defined(NO_FOG)
	float f1FogFactor = 0.0f;
	
	#if !defined(NO_WATERFOGTODESTALPHA)
	if (g_bHeightFog)
	{
		// PIXEL_FOG_TYPE_HEIGHT
		f1FogFactor = ComputeHeightFogFactor(f3WorldPos, f1Depth);

		// Note that the Alpha must be overriden with the Fog Factor
		f4Final.a = f1FogFactor;
	}
	else
	#endif
	{
		// PIXEL_FOG_TYPE_NONE and PIXEL_FOG_TYPE_RANGE and PIXEL_FOG_TYPE_RANGE_RADIAL
		f1FogFactor = ComputeRangeFogFactor(f3WorldPos, f1Depth);
	}
	
	// Apply Fog Color.
	// IMPORTANT: This will apply the Fog Color from PIXEL_FOG_TYPE_HEIGHT as well!!
	f4Final.rgb = lerp(f4Final.rgb, g_f3FogColor, f1FogFactor);
#endif

	return f4Final;
}

#endif // End of LUX_COMMON_PS_FXC_H_