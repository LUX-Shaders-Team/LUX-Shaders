//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// This is required for view matrices
#include "mathlib/vmatrix.h"

// Includes for Shaderfiles...
#include "lux_water_vs30.inc"
#include "lux_water_expensive_ps30.inc"
#include "lux_water_cheap_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Water,			LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DuDv,		LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DX9_HDR,	LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DX90,		LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DX81,		LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DX80,		LUX_Water)
DEFINE_FALLBACK_SHADER(SDK_Water_DX60,		LUX_Water)
#endif

#ifdef REPLACE_WATER
DEFINE_FALLBACK_SHADER(Water,			LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DuDv,		LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DX9_HDR,	LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DX90,		LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DX81,		LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DX80,		LUX_Water)
DEFINE_FALLBACK_SHADER(Water_DX60,		LUX_Water)
#endif

// TF2C
DEFINE_FALLBACK_SHADER(WaterFlow, LUX_Water)

BEGIN_VS_SHADER(LUX_Water, "Water Surface Rendering." LUX_DEFAULT_DESCRIPTION)

SHADER_INFO_GEOMETRY	("Brush Top-Faces")

SHADER_INFO_USAGE		("Materials using Cheap and/or Expensive Water may only be applied to a Brush's Top-Surface.\n"
						 "For Water on Models see LUX_Water_Model.\n"
						 "Materials require an Above Water and a Below Water Material. Specified using $AboveWater and $BottomMaterial\n"
						 "When not using $EnvMap and $RefractTexture, ( 'Cheap' Water ), the Shader renders the expensive Pass for the Background ( Refract )\n"
						 "then Alphablends the Cheap Water Pass (Cubemap) onto the rendered Background.")

SHADER_INFO_LIMITATIONS ("Brushes with Water must be Axis-Aligned, meaning the Brushes shouldn't be at an Angle or rotated.\n"
						 "Multiple Water Materials at different Heights in the same Vis are NOT allowed.\n"
						 "When using cheap Water: Background ( see through Water ) ALWAYS requires a $RefractTexture")

SHADER_INFO_PERFORMANCE ("Expensive Water might not be as expensive you think.\n"
						 "Cheap Water with Background requires a Refract Texture. The Background is displayed in the expensive Shader.\n"
						 "This causes the Shader to draw two separate Vertex and Pixel Shaders.\n"
						 "Its cheaper to just use Expensive Water and then not use a ReflectTexture or Cubemaps.\n"
						 "Aside from that..\n"
						 "Geometry below the Water Surface has to be rendered separately for Refraction.\n"
						 "Anything above the Water has to be rendered *AGAIN* for Reflections via $ReflectTexture.\n"
						 "In other words, the more expensive your Scene, the more expensive using $ReflectTexture will be.\n"
						 "$BlurRefract is VERY expensive!! It Samples a 5x5 Grid of Pixels to achieve the Blur.")

SHADER_INFO_FALLBACK    ("DXLEVEL < 90 -> Wireframe")

SHADER_INFO_WEBLINKS(WEBLINK_VDC)
SHADER_INFO_D3D(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(ForceCheap,		SHADER_PARAM_TYPE_BOOL, "", "Force the water to render the cheap Path.\nRegardless of the map's water_lod_control entity settings or the user's settings.\nThis will replace real-time Reflection with $EnvMap if it is defined.")
	SHADER_PARAM(ForceExpensive,	SHADER_PARAM_TYPE_BOOL, "", "Force the water to render the expensive Path.\nregardless of the map's water_lod_control entity settings or the user's settings.")
	
	// Let's declare Parameters that aren't actually used by the Shader!
	// Yes, read that correctly. These are used elsewhere but not in the Shader. ( Unless we can find a use for them )
	SHADER_PARAM(UnderWaterOverlay,		SHADER_PARAM_TYPE_STRING,	"",	"Material Overlay that will be applied to the Players entire Screen when submerged in the Water Volume.")
	SHADER_PARAM(BottomMaterial,		SHADER_PARAM_TYPE_STRING,	"",	"Material that will be rendered when submerged beneath the Water, needs $AboveWater to be 0.")
	SHADER_PARAM(FogEnable,				SHADER_PARAM_TYPE_BOOL,		"",	"Enables 'Volumetric Fog' ( Height Fog ) for the Water. If this is set to 0, regular Fog (Range/Radial) will be applied instead.")
	SHADER_PARAM(WaterDepth,			SHADER_PARAM_TYPE_INTEGER,	"",	"Some BSP Files come with Patch VMT's for Water Surfaces, with this as a Parameter.\nAccording to the VDC, it's present in CS:GO+ but I feel like I've seen it before.\nEither way... Unused by the Shader.")

	// Outdated Parameters
	SHADER_PARAM(NoLowEndLightmap,		SHADER_PARAM_TYPE_BOOL,		"", "(DEPRECATED) Used to disable Lightmaps on very bad Hardware, no longer used.")

	// Reflect
	SHADER_PARAM(ReflectTexture,		SHADER_PARAM_TYPE_TEXTURE,	"", "{Expensive Only} [RGB] Surfaces visible in Reflection of the Material. For real-time Reflection, use _rt_WaterReflection.")
	SHADER_PARAM(ReflectTint,			SHADER_PARAM_TYPE_COLOR,	"", "Tints the Specular Part of the Result ( Reflected Foreground ) Does not Tint Refractions!")
	SHADER_PARAM(ReflectAmount,			SHADER_PARAM_TYPE_FLOAT,	"", "{Expensive Only} Amount of 'warp' for the reflection. Higher values 'reflect more'. Also controls the strength of the $BumpMap.")
	SHADER_PARAM(AboveWater,			SHADER_PARAM_TYPE_BOOL,		"", "Whether this Material is used for Above or Below the Water Surface, this impacts how it will be rendered.")

	// Expensive Only
	SHADER_PARAM(FlashlightTint,		SHADER_PARAM_TYPE_FLOAT,	"", "{Expensive Only} Boosts the Results of Projected Textures on Water Fog.\nBy default the Flashlight might be very dark without this.")

	// Refract
	SHADER_PARAM(RefractTexture,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Surfaces visible from behind the Material. For real-time Refraction, use _rt_WaterRefraction.\n[A] Height-Fog Mask ")
	SHADER_PARAM(RefractTint,			SHADER_PARAM_TYPE_COLOR,	"", "{Expensive Only} Tints the Diffuse Part of the Result ( Refracted Background ) Does not Tint Reflections!")
	SHADER_PARAM(RefractAmount,			SHADER_PARAM_TYPE_FLOAT,	"", "{Expensive Only} Amount of 'warp' for the Refraction. Higher values produce more Refraction.")
	SHADER_PARAM(BlurRefract,			SHADER_PARAM_TYPE_BOOL,		"", "{Expensive Only} Blurs the Refraction. Very expensive! Samples a 5x5 Field around the selected Pixel.")

	// Fog
	// This used to be a Color Parameter, but we check if its undefined and set it to 1.0f, 0.0f, 0.0f. No longer a point to it being a Color Param.
	SHADER_PARAM(FogColor,				SHADER_PARAM_TYPE_VEC3,		"", "Color of the Water's Volumetric Fog. Generally this Value should match the Color used in the $BottomMaterial")
	SHADER_PARAM(FogStart,				SHADER_PARAM_TYPE_FLOAT,	"", "Distance in units/inches from the eye at which water fog starts.")
	SHADER_PARAM(FogEnd,				SHADER_PARAM_TYPE_FLOAT,	"", "Distance in units/inches from the eye at which water fog ends.")
	SHADER_PARAM(LightmapWaterFog,		SHADER_PARAM_TYPE_BOOL,		"", "Allows the fog to receive Lightmaps, so that static objects can cast shadows onto the Waterfog. This must be enabled when the Map is compiled!")
	SHADER_PARAM(HeightFogFactorType,	SHADER_PARAM_TYPE_INTEGER,	"", "Forces a specific Height Fog Factor.\n0 = Automatically detects based on ASW Shader Features.\n1 = SDK FogFactor.\n2 = ASW FogFactor.\n")
	// Normal Mapping and those that affect Normal Mapping
	//
	// Water shader has this special Parameter for whatever Reason
	// So we have to keep it for backwards compatability.
	// We will just copy this to $NormalTexture ( need to load the others for AnimatedTexture Vars )
	// The same goes for $BumpMap, there are occassionally people wondering why it doesn't work so lets add it for easy use
	// To avoid DuDv Maps being used on old VMT's, $NormalMap has Priority over $BumpMap
	SHADER_PARAM(NormalMap,				SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Normal Map.\n[A] Scaling Factor for Refract and Reflect Intensity.")
	Declare_NormalTextureParameters()
	SHADER_PARAM(Time,					SHADER_PARAM_TYPE_FLOAT,	"", "Custom Time Variable for the Shader. This is optional. Used when > 0.000")
	// ShiroDkxtro2: These two used to be Color parameters.
	// For Reasons unknown, they decided to do the exact opposite of $PhongTint here.
	// Scroll1 & 2 had [1 1 1] as Default, but they wanted [0 0 0]
	// $PhongTint had the inverse. [0 0 0] Default but intended [1 1 1]
	// I changed it to a VEC3 as they don't need non-zero Defaults
	// UPDATE: It's even worse than I thought, it doesn't even use the .z component. VEC2 it is
	SHADER_PARAM(Scroll1,				SHADER_PARAM_TYPE_VEC2, "", "Scrolling Values for MultiTexture Water, must be defined in the VMT not just in a Proxy if you use this.")
	SHADER_PARAM(Scroll2,				SHADER_PARAM_TYPE_VEC2, "", "Scrolling Values for MultiTexture Water")

	// Flowmapping from ASW, might as well just be the L4D2 Implementation
	// ( See ASW's Cheap Water PixelShader FXC, it lists Swamp VMT Values )
	SHADER_PARAM(FlowMap,						SHADER_PARAM_TYPE_TEXTURE,	"",	"[RG] BumpMap XY Directions. Defines Flow Velocity by skewing and scrolling the $NormalMap.\n[BA] Alternative XY Directions when $Flow_Stop > 0. Will set .xy to .zw when $Flow_Stop == 1.0f.")
	SHADER_PARAM(FlowMapFrame,					SHADER_PARAM_TYPE_INTEGER,	"",	"Frame Number for $FlowMap.")
	SHADER_PARAM(FlowMapScrollRate,				SHADER_PARAM_TYPE_VEC2,		"",	"2D Scroll Rate/Scalar for $Flowmap.")
	SHADER_PARAM(Flow_BumpStrength,				SHADER_PARAM_TYPE_FLOAT,	"", "How rough the surface of the water is.")
	SHADER_PARAM(Flow_WorldUVScale,				SHADER_PARAM_TYPE_FLOAT,	"", "The number of times the $FlowMap fits into the Material. Texture Scale affects this.")
	SHADER_PARAM(Flow_NormalUVScale,			SHADER_PARAM_TYPE_FLOAT,	"", "The number of Inches/Units covered by the Normal Map before it repeats. Typically in the 100s.")
	SHADER_PARAM(Flow_UVScrollDistance,			SHADER_PARAM_TYPE_FLOAT,	"", "How far along the flow map the normal map should be distorted. Higher values lead to more Distortion.")
	SHADER_PARAM(Flow_TimeIntervalInSeconds,	SHADER_PARAM_TYPE_FLOAT,	"", "Time needed for the normal map to cross the $flow_uvscrolldistance.")
	SHADER_PARAM(Flow_TimeScale,				SHADER_PARAM_TYPE_FLOAT,	"", "Modifies flow speed without affecting the amount of distortion.")
	SHADER_PARAM(Flow_Noise_Texture,			SHADER_PARAM_TYPE_TEXTURE,	"",	"[G] Noise Values used to break repetition of the Normal Map.\n[G] is not a typo.. This should really be using [R], but it isn't!")
	SHADER_PARAM(Flow_Noise_Scale,				SHADER_PARAM_TYPE_FLOAT,	"", "How many times to fit the noise Texture into the normal map. Typically around 0.01.")
	SHADER_PARAM(Flow_Debug,					SHADER_PARAM_TYPE_BOOL,		"", "Allows you to display the current $FlowMap. See also the ConVar: LUX_Water_DebugFlowmaps")
	// LUX additions to the existing $FlowMap Parameters :
	SHADER_PARAM(Flow_Stop_Enable,				SHADER_PARAM_TYPE_BOOL,		"", "Enable $Flow_Stop Feature. This is a Parameter for Performance Reasons and so you can set $Flow_Stop to 0.")
	SHADER_PARAM(Flow_Stop,						SHADER_PARAM_TYPE_FLOAT,	"", "Linear [0..1] Factor for Lerping the Flowmaps [RG] and [BA],\nintended for stopping Flowing Water ( Black BA Values ) or increasing/decreasing Flow Speed using Proxies")

	// Fresnel..
	SHADER_PARAM(NoFresnel,						SHADER_PARAM_TYPE_BOOL,		"", "Disable the Fresnel on the Water's Reflection.")
	SHADER_PARAM(ForceFresnel,					SHADER_PARAM_TYPE_FLOAT,	"", "{Expensive Only} Force this Amount of Fresnel on the water.\nHigher Values usually cause the Water to appear brighter.")

	// Surface Related Parameters
	SHADER_PARAM(Surface_BumpMap,				SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB]Separate BumpMap for the Surface Layer that Scrolls with the Surface Layer.\n[A]Reflection Mask. For Cheap Water use $NormalMapAlphaEnvMapMask.")
	SHADER_PARAM(Surface_SecondTexCoord,		SHADER_PARAM_TYPE_BOOL,		"", "Make use of a second Texture Coordinate Transform for the Surface Layer ($Surface_Transform).\n This gives you more control over Surface movement.")
	SHADER_PARAM(Surface_Transform,				SHADER_PARAM_TYPE_MATRIX,	"", "Only used when $Surface_SecondTexCoord is set, transforms the Textexture Coordinate of the Surface Layer.")

	SHADER_PARAM(Color_Flow_UVScale,				SHADER_PARAM_TYPE_FLOAT,"",	"Number of world Units/Inches covered by the BaseTexture before it repeats. Typically in the 100s.")
	SHADER_PARAM(Color_Flow_TimeScale,				SHADER_PARAM_TYPE_FLOAT,"", "Modifies the Flowspeed without affecting the Amount of Distortion.")
	SHADER_PARAM(Color_Flow_TimeIntervalInSeconds,	SHADER_PARAM_TYPE_FLOAT,"", "Time needed for the BaseTexture to cross the $Color_Flow_UVScrollDistance.")
	SHADER_PARAM(Color_Flow_UVScrollDistance,		SHADER_PARAM_TYPE_FLOAT,"", "How far along the FlowMap the BaseTexture should be distorted. Higher Values lead to more Distortion.")
	SHADER_PARAM(Color_Flow_LerpExp,				SHADER_PARAM_TYPE_FLOAT,"", "How sharp the Transition should be between Repeats, should be >= 1.0")

	// For Cheap Water we will support (most of) the other EnvMapping Parameters
	Declare_EnvironmentMapParameters()
	Declare_ParallaxCorrectionParameters() // PCC on cheap water!

	// Cheap Water Shenanigans
	SHADER_PARAM(ReflectBlendFactor,		SHADER_PARAM_TYPE_FLOAT, "", "{Cheap Only} Adds to the Fresnel Factor ( up to a maximum of 1.0f)")
	SHADER_PARAM(CheapWaterStartDistance,	SHADER_PARAM_TYPE_FLOAT, "", "{Cheap Only} Distance from the Camera, in Units/Inches to start transitioning to cheap water.\nExpensive Water will still render!! This just fades in Cheap Water.")
	SHADER_PARAM(CheapWaterEndDistance,		SHADER_PARAM_TYPE_FLOAT, "", "{Cheap Only} Distance from the Camera, in Units/Inches to finish transitioning to cheap water.\nExpensive Water will still render!! This just fades in Cheap Water.")

	// Stop checking every frame if some random abs() > 0.0f ???
	// Just do it once and store that somewhere?!
	SHADER_PARAM(InternalVar_MultiTexture,			SHADER_PARAM_TYPE_BOOL,	"", "(INTERNAL PARAMETER) Used internally as more or less static boolean.")

	// Stop converting the value every frame from gamma to linear ???
	// Just do it once and store that somewhere?!
	// NOTE: This will break Proxies but we don't care
	SHADER_PARAM(InternalVar_LinearReflectTint,		SHADER_PARAM_TYPE_VEC3, "",	"(INTERNAL PARAMETER) Used as storage, this should really be a command buffer instruction!!")
	SHADER_PARAM(InternalVar_LinearRefractTint,		SHADER_PARAM_TYPE_VEC3, "",	"(INTERNAL PARAMETER) Used as storage, this should really be a command buffer instruction!!")
	SHADER_PARAM(InternalVar_LinearFogColor,		SHADER_PARAM_TYPE_VEC3, "",	"(INTERNAL PARAMETER) Used as storage, this should really be a command buffer instruction!!")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	// We want to have $NormalMap when its there, tt has priority over $BumpMap
	// which some existing Materials still define as a DuDv Map
	// We also want to support $BumpMap because thats more consistent
	// But we also don't like $BumpMap, and $NormalMap is even worse
	// So we will be using $NormalTexture, in secret!
	if (IsDefined(NormalMap))
		SetString(NormalTexture, GetString(NormalMap));
	else if (IsDefined(BumpMap))
		SetString(NormalTexture, GetString(BumpMap));

	// Set NormalMap and BumpMap to the same String!!
	// This is necessary for the above because of the AnimatedTexture Proxy.
	// It checks for a specific Texture Parameter for the Texture Data
	SetString(BumpMap, GetString(NormalTexture));
	SetString(NormalMap, GetString(NormalTexture));

	if (!IsDefined(AboveWater))
	{
		ShaderDebugMessage("does not have $AboveWater set to any value. \n Using $AboveWater 1 by default for this Material.\n If your Water looks incorrect, now you know why.\n");
		SetBool(AboveWater, true);
	}

	if (!IsDefined(FogColor))
	{
		ShaderDebugMessage("does not have a $FogColor.\n The Fog will be Red (forcing you to fix this).\n A $FogColor MUST be defined!\n");
		DefaultFloat3(FogColor, 1.0f, 0.0f, 0.0f);
	}

	if (lux_water_debugflowmaps.GetBool() && IsDefined(FlowMap))
		SetBool(Flow_Debug, true);

	// Flow Map Parameters...
	DefaultFloat(Flow_WorldUVScale, 1.0f);
	DefaultFloat(Flow_NormalUVScale, 1.0f);
	DefaultFloat(Flow_TimeIntervalInSeconds, 0.4f);
	DefaultFloat(Flow_UVScrollDistance, 0.2f);
	DefaultFloat(Flow_BumpStrength, 1.0f);
	DefaultFloat(Flow_TimeScale, 1.0f);
	DefaultFloat(Flow_Noise_Scale, 0.0002f);

	DefaultFloat(Color_Flow_UVScale, 1.0f);
	DefaultFloat(Color_Flow_TimeScale, 1.0f);
	DefaultFloat(Color_Flow_TimeIntervalInSeconds, 0.4f);
	DefaultFloat(Color_Flow_UVScrollDistance, 0.2f);
	DefaultFloat(Color_Flow_LerpExp, 1.0f);
	DefaultFloat(FlashlightTint, 1.0f);
	DefaultFloat(ForceFresnel, -1.0f); // Why -1 and not just leave it at 0? I don't know...

	// We don't want to evaluate this *every* frame, even if we have a precomputed lookup-table for this...
	// So we convert from Gamma to Linear here. If someone used a Proxy to animate this, they will have to adjust.
	if (IsDefined(RefractTint))
	{
		float3 f3RefractTint = GetFloat3(RefractTint);

		// Stock-Consistency : GammaToLinear Conversion
		f3RefractTint = GammaToLinearTint(f3RefractTint);

		SetFloat3(InternalVar_LinearRefractTint, f3RefractTint);
	}
	// NOTE: Despite these being Color Params they default to 0
	// This likely happens because the Materials are loaded manually somewhere
	// ( For other pseudo Parameters like $FogEnable )
	else
	{
		SetFloat3(ReflectTint, 1.0f, 1.0f, 1.0f);
		SetFloat3(InternalVar_LinearReflectTint, 1.0f, 1.0f, 1.0f);
	}


	// We don't want to evaluate this *every* frame, even if we have a precomputed lookup-table for this...
	// So we convert from Gamma to Linear here. If someone used a Proxy to animate this, they will have to adjust.
	if (IsDefined(ReflectTint))
	{
		float3 f3ReflectTint = GetFloat3(ReflectTint);

		// Stock-Consistency : GammaToLinear Conversion
		f3ReflectTint = GammaToLinearTint(f3ReflectTint);

		SetFloat3(InternalVar_LinearReflectTint, f3ReflectTint);
	}
	// NOTE: Despite these being Color Params they default to 0
	// This likely happens because the Materials are loaded manually somewhere
	// ( For other pseudo Parameters like $FogEnable )
	else
	{
		SetFloat3(ReflectTint, 1.0f, 1.0f, 1.0f);
		SetFloat3(InternalVar_LinearReflectTint, 1.0f, 1.0f, 1.0f);
	}

	// We don't want to evaluate this *every* frame, even if we have a precomputed lookup-table for this...
	// So we convert from Gamma to Linear here. If someone used a Proxy to animate this, they will have to adjust.
	//	if (IsDefined(FOGCOLOR))
	{
		float3 f3FogColor = GetFloat3(FogColor);

		// Stock-Consistency : GammaToLinear Conversion
		// ASW-Consistency: Using Piecewise Approximation
//		f3FogColor = GammaToLinearTint(f3FogColor);
		// No specialised Function for this so do it manually per Component
		f3FogColor.x = SrgbGammaToLinear(f3FogColor.x);
		f3FogColor.y = SrgbGammaToLinear(f3FogColor.y);
		f3FogColor.z = SrgbGammaToLinear(f3FogColor.z);

		SetFloat3(InternalVar_LinearFogColor, f3FogColor);
	}

	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// ShiroDkxtro2: I don't want to force it, but I will simply force it.
	// We always want this available for $LightmapWaterFog and $BaseTexture Layers
	if (IsDefined(BaseTexture) || GetBool(LightmapWaterFog))
		SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

	// We replicate Valve Behaviour here, in ASW it does this....:
	if (IsDefined(BaseTexture) && g_pConfig->UseBumpmapping())
		SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);

	// Cheap Water
	DefaultFloat(CheapWaterStartDistance, 500.0f);
	DefaultFloat(CheapWaterEndDistance, 1000.0f);
	DefaultFloat(ReflectBlendFactor, 1.0f);

	if(!lux_water_projectedtexturesupport.GetBool())
		SetBool(ReceiveProjectedTextures, false);
}

SHADER_FALLBACK
{
#ifndef REPLACE_WATER
	if (lux_oldshaders.GetBool())
		return "Water";
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
	// Surface Layer
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadBumpMap(Surface_BumpMap);

	// Refraction for Cheap and Expensive Water
	LoadTexture(RefractTexture, TEXTUREFLAGS_SRGB);

	// Reflections for Expensive Water, and Cubemaps for Cheap Water
	LoadTexture(ReflectTexture, TEXTUREFLAGS_SRGB);

	// Always try and load the EnvMap for Cheap Fallbacks
	LoadCubeMap(EnvMap, 0);

	// Need to load all of them, but we only use $NormalTexture
	// ShiroDkxtro2: @#$%!&*?! WHYYYY??? It doesn't even make any sense??
	// Loading all these fixes the Texture-Scaling Issue I've been chasing FOR TWO YEARS
	// I figured this out when looking at Textures not Animating ( AnimatedTexture looks for specific Params )
	// Just.. Load all of them, all we need is the Texture Reference on these Params.
	LoadBumpMap(NormalTexture);
	LoadBumpMap(BumpMap);
	LoadBumpMap(NormalMap);

	LoadTexture(FlowMap, 0);
	LoadTexture(Flow_Noise_Texture, 0);

	if (!IsDefined(NormalTexture) && CVarDeveloper.GetInt() > 0)
		ShaderDebugMessage("Water Material %s has no Normal Map? Might be unintentional.\n");

	// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
	// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
	// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
	// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
	if(IsDefined(EnvMapParallaxOBB1) && !GetBool(EnvMapParallax))
		DefaultBool(EnvMapParallax, true);

	// Stock Shaders determined this *everytime* the Draw Function was executed
	// Just precompute it, this might change if someone can show me a proxy messing with this without the Parameter being defined
	if (IsDefined(Scroll1))
	{
		float2 f2Scroll1 = GetFloat2(Scroll1);
		SetBool(InternalVar_MultiTexture, fabs(f2Scroll1.x) > 0.0f ? true : false);
	}
}

// BaseVSShader allows us to strip most Inputs from the Draw Function
void DrawExpensive(IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, bool bReflection)
{
	bool bHasRefractTexture = IsTextureLoaded(RefractTexture);
	bool bHasBlurRefract = bHasRefractTexture && GetBool(BlurRefract);

	bool bHasReflectTexture = bReflection;

	// Flashlight support apparently first added in ASW, I don't see why you wouldn't want to have it.
	bool bHasFlashlight = HasFlashlight();
	bool bHasFlowTexture = IsTextureLoaded(FlowMap);
	bool bHasFlowNoiseTexture = bHasFlowTexture && IsTextureLoaded(Flow_Noise_Texture);
	bool bHasFlowStop = bHasFlowTexture && GetBool(Flow_Stop_Enable);
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasLightmapFog = GetBool(LightmapWaterFog);

	// Forced Fresnel replaces Fresnel with a Factor.
	// That Factor is used to lerp between Reflect & Fog OR Reflect & Refract.
	// So you can only have ForceFresnel *with* Reflect on this Path
	bool bForceFresnel = bHasReflectTexture && GetFloat(ForceFresnel) != -1.0f;
	bool bNoFresnel = !bForceFresnel && GetBool(NoFresnel);

	bool bHasNormalTexture = IsTextureLoaded(NormalTexture);
	bool bHasSurfaceBumpMap = bHasBaseTexture && IsTextureLoaded(Surface_BumpMap);

	// ShiroDkxtro2 :
	// On the original shader (SDK <> ASW) it was SKIP'd when FlowMaps are on and I decided to do the same thing
	// This behaviour is intentionally replicated, and thus not available in combination with FlowMaps.
	// It would get quite complicated for something not many people would ever want to use.
	bool bHasMultiTexture = GetBool(InternalVar_MultiTexture) && !bHasFlowTexture;

	// ASW Shader also had && bHasReflectTexture for the basetexture, an issue with their shader, not ours. More to that below.
	bool bUsingLightmap = bHasLightmapFog || bHasBaseTexture;
	
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

		// Stock-Consistency
		FogToFogColor();

		// Make projected Textures additive
		if(bHasFlashlight)
			EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);

		// We want this!
		// What this Shader Outputs is what the Player sees
		// The Geometry behind the Waterplane basically doesn't exist
		// ( See what happens if you render the Cheap Pass without also rendering the Expensive Pass )
		// We will Output new DestAlpha Values!
		pShaderShadow->EnableAlphaWrites(true);

		// Always Linear. We love Linear.
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We need Position for ProjPos, Normal for Fresnel and Tangents for BumpMapping
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		
		// ShiroDkxtro2 :
		// The original Shader ( from ASW ) also had a minimum requirement for && bReflect.
		// "Not sure where the bReflection restriction comes in." - ASW Water Shader
		// 
		// Well I can tell you why.
		// It's because the Water Shader on Stock is one giant Mess,
		// 
		// The $BaseTexture Code was only ever applied to the REFLECT and (REFLECT && REFRACT) Paths
		// if(REFLECT && REFRACT) { BaseTexture code } else if(REFLECT) { BaseTexture code } 
		// else if (REFRACT) { no Code here } else { no Code here }
		// I have removed this Restriction ( by not using else if chains )
		// You may also use Lightmaps on cheap Water now for EnvMapLightScale!
		int nTexCoords = 1;
		
		if (bHasLightmapFog)
			nTexCoords = 2;

		// Base TexCoord, Lightmap Coord, Bumped Lightmap Offset
		if (bHasBaseTexture)
			nTexCoords = 3;

		// Not a Model, so 0
		int nUserDataSize = 0;

		// No TexCoord Dimensions or UserDataSize for this Shaders
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 BaseTexture, always sRGB
		EnableSampler(bHasBaseTexture, SHADER_SAMPLER0,true);

		// s1 Reflect Texture, always sRGB
		EnableSampler(bHasReflectTexture, SHADER_SAMPLER1, true);

		// s2 Surface BumpMap
		EnableSampler(bHasSurfaceBumpMap, SHADER_SAMPLER2, false);

		// s3 Refract Texture, always sRGB
		EnableSampler(bHasRefractTexture, SHADER_SAMPLER3, true);

		// s4 Water Normal, always!
		EnableSampler(SHADER_SAMPLER4, false);

		// s5 and s6, $FlowMap and $Flow_Noise_Texture
		EnableSampler(bHasFlowTexture, SHADER_SAMPLER5, false);
		EnableSampler(bHasFlowTexture, SHADER_SAMPLER6, false);

		// s11, Lightmapping
		EnableSampler(bUsingLightmap, SAMPLER_LIGHTMAP, !IsHDREnabled());

		// Flashlight Support
		SetupFlashlightSamplers();

		// FIXME: SetupFlashlightSamplers sets FogToBlack
		// That's exactly what we DON'T want so we have to reset this here
		// This should not be handled down here!
		if(bHasFlashlight)
			FogToFogColor();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_water_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(MULTITEXTURE, bHasMultiTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAPPING, bHasBaseTexture ? 2 : (bHasLightmapFog ? 1 : 0)); // Basetexture == 2, LightmapWaterFog 1 
		SET_STATIC_VERTEX_SHADER_COMBO(SURFACELAYER, bHasBaseTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(REFLECTIONS, 1);
		SET_STATIC_VERTEX_SHADER_COMBO(REFRACTIONS, 1);
		SET_STATIC_VERTEX_SHADER(lux_water_vs30);

		int nFlowMapping = bHasFlowTexture + (bHasFlowTexture && GetBool(Flow_Debug));
		nFlowMapping += (bHasFlowTexture && bHasFlowStop) * 2;

		// Packed these together since they were skipped
		int nPackedCombo = bHasMultiTexture ? 5 : nFlowMapping;

		DECLARE_STATIC_PIXEL_SHADER(lux_water_expensive_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(FLOWMAPPING_MULTITEXTURE, nPackedCombo);
		SET_STATIC_PIXEL_SHADER_COMBO(BASETEXTURELAYER, bHasBaseTexture + bHasSurfaceBumpMap);
		SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPWATERFOG, bHasLightmapFog);
		SET_STATIC_PIXEL_SHADER_COMBO(REFLECT, bHasReflectTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(REFRACT, bHasRefractTexture + bHasBlurRefract);
		SET_STATIC_PIXEL_SHADER_COMBO(ABOVEWATER, GetBool(AboveWater));
		SET_STATIC_PIXEL_SHADER_COMBO(PROJTEX, bHasFlashlight);
		SET_STATIC_PIXEL_SHADER(lux_water_expensive_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// We call this since we draw from two separate Shaders
		// Technically not required on expensive Water since it draws first..
		pShaderAPI->SetDefaultState();

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0
		BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, Frame);

		// s1
		BindTexture(bHasReflectTexture, SHADER_SAMPLER1, ReflectTexture, -1);

		// s2
		BindTexture(bHasSurfaceBumpMap, SHADER_SAMPLER2, Surface_BumpMap, Frame);

		// s3
		BindTexture(bHasRefractTexture, SHADER_SAMPLER3, RefractTexture, -1);

		// s4
		BindTexture(bHasNormalTexture, SHADER_SAMPLER4, NormalTexture, BumpFrame, TEXTURE_NORMALMAP_FLAT);

		// s5 & s6
		BindTexture(bHasFlowTexture, SHADER_SAMPLER5, FlowMap, FlowMapFrame);
		if(bHasFlowTexture) // Only want to get Black Texture if FlowMapNoise Sampler is enabled.
			BindTexture(bHasFlowNoiseTexture, SHADER_SAMPLER6, Flow_Noise_Texture , -1, TEXTURE_BLACK);

		// s11
		BindTexture(bUsingLightmap, SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// We need the time in a couple of places, both VS and PS
		float f1CurrentTime = pShaderAPI->CurrentTime();

		// VS c49, c50
		// We always need the BumpTransform
		// Not doing so will mess up all Water Materials
		// Previous Issue LUX had. Since we checked for defined Transform Parameters
		// Most VMT's have them in the Proxies where they don't count as defined
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BumpTransform);

		// VS c51, c52
		// Allow the Surface to use its own Transform Parameter ( in case it should be standing still etc )
		// If there is no second Transform, use the BumpTransform
		// Unfortunately Proxies don't define the Parameter, so I added a bool to indicate the usage instead.
		if (bHasBaseTexture)
		{
			if (GetBool(Surface_SecondTexCoord))
			{
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, Surface_Transform);
			}
			else
			{
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform);
			}
		}

		// VS c53 - Scrolling Values for Multitexture
		if (bHasMultiTexture)
		{
			float4 f4Scrolls;
			f4Scrolls.xy = GetFloat2(Scroll1);
			f4Scrolls.zw = GetFloat2(Scroll2);
			f4Scrolls = f4Scrolls * f1CurrentTime;
			pShaderAPI->SetVertexShaderConstant(53, f4Scrolls);
		}

		// c1 - Modulation Constant
		if (bUsingLightmap)
			SetModulationConstant(false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - For Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c32 - $Color, $Color2, $sRGBTint
		if (bHasBaseTexture)
		{
			float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);
		}
		
		// c50, c51
		// Don't actually need these!!
		if (false)
		{
			// These Matrices are pretty much Stock ASW Code. Nothing to change here.
			// "These constants are used to rotate the world space water normals around the up axis to align the
			// normal with the camera and then give us a 2D offset vector to use for reflection and refraction uv's" - ASW Shader
			VMatrix mx_View;
			pShaderAPI->GetMatrix(MATERIAL_VIEW, mx_View.m[0]); // Use Base() instead?
			mx_View = mx_View.Transpose3x3(); // MATERIAL_VIEW comes Transposed, detranspose it

			// We use Vector4D instead of float4 because of Normalize() and Cross()
			Vector4D v4CameraRight(mx_View.m[0][0], mx_View.m[0][1], mx_View.m[0][2], 0.0f);
			v4CameraRight.z = 0.0f; // "Project onto the plane of water" - ASW Shader
			v4CameraRight.AsVector3D().NormalizeInPlace();

			// "I assume the water surface normal is pointing along z!" - ASW Shader
			// This references Water in Source always being a flat Plane
			// The Normal of which goes straight up ( along Z, [0 0 1] )
			Vector4D v4CameraForward;
			CrossProduct(Vector(0.0f, 0.0f, 1.0f), v4CameraRight.AsVector3D(), v4CameraForward.AsVector3D());

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_050, v4CameraRight.Base(), 1);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_051, v4CameraForward.Base(), 1);
		}

		// c53, c54, c55
		if (bHasFlowTexture)
		{
			// Sending Flowmap data to the shader

			// World UV Scale, Normal UV Scale, Bumpstrength, Timescale
			float4 f4FlowControls1;
			f4FlowControls1.x = 1.0f / GetFloat(Flow_WorldUVScale); // We want the Reciprocal
			f4FlowControls1.y = 1.0f / GetFloat(Flow_NormalUVScale); // We want the Reciprocal
			f4FlowControls1.z = GetFloat(Flow_BumpStrength);
			f4FlowControls1.w = f1CurrentTime * GetFloat(Flow_TimeScale); // * Time precomputed
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_053, f4FlowControls1);

			// Time Interval [s] ), UV Scrolldist., Noise Scale, Color_Flow_LerpExp
			float4 f4FlowControls2; 
			f4FlowControls2.x = 1.0f / (2.0f * GetFloat(Flow_TimeIntervalInSeconds)); // [s] - in Seconds. With precomputed Reciprocal and 2.0f*
			f4FlowControls2.y = GetFloat(Flow_UVScrollDistance);
			f4FlowControls2.z = GetFloat(Flow_Noise_Scale);
			f4FlowControls2.w = GetFloat(Color_Flow_LerpExp); // Only needed for bHasBaseTexture		
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_054, f4FlowControls2);		

			if (bHasBaseTexture)
			{
				// Color Flow (CF) UV Scale, CF Timescale, CF Time Inteval [s], CF Uv Scrolldist.
				float4 f4FlowControls3;
				f4FlowControls3.x = 1.0f / GetFloat(Color_Flow_UVScale); // We want the Reciprocal
				f4FlowControls3.y = f1CurrentTime * GetFloat(Color_Flow_TimeScale); // * Time precomputed
				f4FlowControls3.z = 1.0f / (2.0f * GetFloat(Color_Flow_TimeIntervalInSeconds));	// [s] - in Seconds. With precomputed Reciprocal and 2.0f*
				f4FlowControls3.w = GetFloat(Color_Flow_UVScrollDistance);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_055, f4FlowControls3);
			}
		}

		// c56
		if (bHasRefractTexture)
		{
			// Converted from Gamma to Linear on Param Init
			float4 f4RefractTint = 0.0f;
			f4RefractTint.rgb = GetFloat3(InternalVar_LinearRefractTint);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_056, f4RefractTint);
		}

		// c57
		if (bHasReflectTexture)
		{
			float4 f4ReflectTint = 0.0f;
			// g_pConfig Header says this is for debugging, but I assume its tied to the Options Menu Reflections Setting
			if (g_pConfig->bShowSpecular)
			{
				f4ReflectTint.rgb = GetFloat3(InternalVar_LinearReflectTint); // Converted from Gamma to Linear on Param Init

				// "Need to multiply by 4 in linear space since we premultiplied into
				// the render target by .25 to get overbright data in the reflection render target."
				if (IsHDREnabled())
					f4ReflectTint.rgb = f4ReflectTint.rgb * 4.0f;

				// Cheap Water does this for Reference..
				//	f4ReflectTint.rgb = GetFloat3(ReflectTint);
			}

			// .w Empty
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_057, f4ReflectTint);
		}

		// c58
		float4 f4FogColor_Start;
		f4FogColor_Start.rgb = GetFloat3(InternalVar_LinearFogColor); // Converted from Gamma to Linear on Param Init
		// Cheap Water does this for Reference..
//		f4FogColor_Start.rgb = GetFloat3(FogColor);
		f4FogColor_Start.w = GetFloat(FogStart);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_058, f4FogColor_Start);

		// c59
		if (true)
		{
			float4 f4VariousControls;
			f4VariousControls.x = GetFloat(FogEnd) - f4FogColor_Start.w;
			f4VariousControls.y = IsHDREnabled() ? 4.0f : 1.0f; // Water Overbright Factor

			// No Fresnel at all
			if(bNoFresnel)
			{
				f4VariousControls.z = 0.0f; // Multiplier
				f4VariousControls.w = 0.0f; // Additive
			}
			// Constant Fresnel Value
			else if (bForceFresnel)
			{
				f4VariousControls.z = 0.0f;
				f4VariousControls.w = GetFloat(ForceFresnel);
			}
			// Regular Fresnel
			else
			{
				f4VariousControls.z = 1.0f; // Multiplier
				f4VariousControls.w = 0.0f; // Additive
			}

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_059, f4VariousControls);
		}

		// c60
		float4 f4UVScalars;
		f4UVScalars.x = GetFloat(ReflectAmount);
		f4UVScalars.y = f4UVScalars.x;
		f4UVScalars.z = GetFloat(RefractAmount);
		f4UVScalars.w = f4UVScalars.z;
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_060, f4UVScalars);

		// c61
		// Extra Flashlight Tint for Waterfog
		// Need .w for FlowStop.. FIXME: Find a better spot for this. Preferably something thats consistent with Cheap Water
		if (bHasFlashlight || bHasFlowStop)
		{
			float4 f4FlashlightTint = 0.0f;

			if(bHasFlashlight)
				f4FlashlightTint.rgb = GetFloat3(FlashlightTint);

			if(bHasFlowStop)
				f4FlashlightTint.w = GetFloat(Flow_Stop);

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_061, f4FlashlightTint);
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
//		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();

		// We always write our Depth to Alpha
		// We have nothing else to write, and Destination Alpha is Empty.
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = true; // pShaderAPI->ShouldWriteDepthToDestAlpha();

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_water_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_water_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_water_expensive_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER(lux_water_expensive_ps30);
	}

	Draw();
}

void DrawCheap(IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, bool bInEditor, bool bExpensiveWaterEditorPreview)
{
	bool bBlend = !bInEditor; // Stock-Consistency
	bool bHasRefractTexture = bBlend && IsTextureLoaded(RefractTexture);

	bool bHasFlowTexture = IsTextureLoaded(FlowMap);
	bool bHasFlowNoiseTexture = bHasFlowTexture && IsTextureLoaded(Flow_Noise_Texture);
	bool bHasFlowStop = bHasFlowTexture && GetBool(Flow_Stop_Enable);
	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasLightmapFog = GetBool(LightmapWaterFog);
	bool bNoFresnel = GetBool(NoFresnel);

	bool bHasNormalTexture = IsTextureLoaded(NormalTexture);
	bool bHasSurfaceBumpMap = bHasBaseTexture && IsTextureLoaded(Surface_BumpMap);

	// HasEnvMap = Should enable the Feature
	// UseEnvMap = Should actually use the Feature
	bool bHasEnvMap = IsTextureLoaded(EnvMap);
	bool bUseEnvMap = mat_specular.GetBool() && bHasEnvMap;
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	// ShiroDkxtro2 :
	// On the original shader (SDK <> ASW) it was SKIP'd when FlowMaps are on and I decided to do the same thing
	// This behaviour is intentionally replicated, and thus not available in combination with FlowMaps.
	// It would get quite complicated for something not many people would ever want to use.
	bool bHasMultiTexture = GetBool(InternalVar_MultiTexture) && !bHasFlowTexture;

	// ASW Shader also had && bHasReflectTexture for the basetexture, an issue with their shader, not ours. More to that below.
	bool bUsingLightmap = bHasLightmapFog || bHasBaseTexture;
	bool bAboveWater = GetBool(AboveWater);

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Stock-Consistency
		FogToFogColor();

		// In Editor Mode, use nocull
		if (bInEditor)
			pShaderShadow->EnableCulling(false);

		// This doesn't actually work for !$AboveWater Materials
		if (bBlend)
		{
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
		}

		// Stock Shader doesn't set this at all, I assume the default is false?
		// Either way we don't want AlphaWrites, those were already done by Expensive Water
		pShaderShadow->EnableAlphaWrites(false);

		// Always Linear
		pShaderShadow->EnableSRGBWrite(true);
		
		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// We need Position for ProjPos, Normal for Fresnel and Tangents for BumpMapping
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S | VERTEX_TANGENT_T;

		// ShiroDkxtro2 :
		// The original Shader ( from ASW ) also had a minimum requirement for && bReflect.
		// "Not sure where the bReflection restriction comes in." - ASW Water Shader
		// 
		// Well I can tell you why.
		// It's because the Water Shader on Stock is one giant Mess,
		// 
		// The $BaseTexture Code was only ever applied to the REFLECT and (REFLECT && REFRACT) Paths
		// if(REFLECT && REFRACT) { BaseTexture code } else if(REFLECT) { BaseTexture code } 
		// else if (REFRACT) { no Code here } else { no Code here }
		// I have removed this Restriction ( by not using else if chains )
		// You may also use Lightmaps on cheap Water now for EnvMapLightScale!
		int nTexCoords = 1;

		if (bHasLightmapFog)
			nTexCoords = 2;

		// Base TexCoord, Lightmap Coord, Bumped Lightmap Offset
		if (bHasBaseTexture)
			nTexCoords = 3;

		// Not a Model so always 0
		int nUserDataSize = 0;

		// No TexCoord Dimensions or UserDataSize for this Shaders
		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// Hammer and H++ are always sRGB. EnvMaps and Lightmaps need to consider this!
		bool bSRGBRead = !IsHDREnabled();

		// s0 BaseTexture, always sRGB
		EnableSampler(bHasBaseTexture, SHADER_SAMPLER0, true);

		// s1 Reflect Texture, always sRGB
		// Cheap Water so EnvMap, that one goes to 
//		EnableSampler(bHasReflectTexture, SHADER_SAMPLER1, true);

		// s2 Surface BumpMap
		EnableSampler(bHasSurfaceBumpMap, SHADER_SAMPLER2, false);

		// s3 Refract Texture, always sRGB
		// Or not? Stock Code doesn't have sRGB Conversion for this
		// I assume this is the Case because we only ever want to read the Alpha from it ( which doesn't get converted )
		EnableSampler(bHasRefractTexture, SHADER_SAMPLER3, false);

		// s4 Water Normal, always!
		EnableSampler(SHADER_SAMPLER4, false);

		// s5 and s6, $FlowMap and $Flow_Noise_Texture
		EnableSampler(bHasFlowTexture, SHADER_SAMPLER5, false);
		EnableSampler(bHasFlowTexture, SHADER_SAMPLER6, false);

		// s11, Lightmapping
		EnableSampler(bUsingLightmap, SAMPLER_LIGHTMAP, bSRGBRead);

		// s14, EnvMap
		// The Original Water Shader forced non-sRGB read for the EnvMap.
		// If an LDR cubemap is bound while we say it isn't, we are going to have a Brightness Problem.
		// Check HDR to solve this issue.
		// Always enable, avoids Issues with L4D2 Mats!
		EnableSampler(true, SAMPLER_ENVMAPTEXTURE, bSRGBRead);

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_water_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(MULTITEXTURE, bHasMultiTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAPPING, bHasBaseTexture ? 2 : (bHasLightmapFog ? 1 : 0)); // Basetexture == 2, LightmapWaterFog 1 
		SET_STATIC_VERTEX_SHADER_COMBO(SURFACELAYER, bHasBaseTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(REFLECTIONS, false);
		SET_STATIC_VERTEX_SHADER_COMBO(REFRACTIONS, bHasRefractTexture);
		SET_STATIC_VERTEX_SHADER(lux_water_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_water_cheap_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(FLOWMAPPING, bHasFlowTexture + GetBool(Flow_Debug));
		SET_STATIC_PIXEL_SHADER_COMBO(FLOWSTOP, bHasFlowTexture && bHasFlowStop);
		SET_STATIC_PIXEL_SHADER_COMBO(MULTITEXTURE, bHasMultiTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(BASETEXTURELAYER, bHasBaseTexture + bHasSurfaceBumpMap);
		SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPWATERFOG, bHasLightmapFog);
		SET_STATIC_PIXEL_SHADER_COMBO(BLEND, bBlend + bHasRefractTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXCORRECT, bPCC);
		SET_STATIC_PIXEL_SHADER_COMBO(PRETTYHAMMER, bInEditor && bExpensiveWaterEditorPreview);
		SET_STATIC_PIXEL_SHADER(lux_water_cheap_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// We call this since we draw from two separate Shaders
		pShaderAPI->SetDefaultState();

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0
		BindTexture(bHasBaseTexture, SHADER_SAMPLER0, BaseTexture, Frame);

		// s1 Not for Cheap Water
//		BindTexture(bHasReflectTexture, SHADER_SAMPLER1, ReflectTexture, -1);

		// s2
		BindTexture(bHasSurfaceBumpMap, SHADER_SAMPLER2, Surface_BumpMap, Frame);

		// s3
		BindTexture(bHasRefractTexture, SHADER_SAMPLER3, RefractTexture, -1);

		// s4
		BindTexture(bHasNormalTexture, SHADER_SAMPLER4, NormalTexture, BumpFrame, TEXTURE_NORMALMAP_FLAT);

		// s5 & s6
		BindTexture(bHasFlowTexture, SHADER_SAMPLER5, FlowMap, FlowMapFrame);
		if(bHasFlowTexture) // Only want to get Black Texture if FlowMapNoise Sampler is enabled.
			BindTexture(bHasFlowNoiseTexture, SHADER_SAMPLER6, Flow_Noise_Texture , -1, TEXTURE_BLACK);

		// s11
		BindTexture(bUsingLightmap, SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP);

		// s14
		BindTexture(bUseEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		if (bUsingLightmap)
			SetModulationConstant(false);

		if (bHasBaseTexture)
		{
			float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), Alpha);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);
		}

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// We need the time in a couple of places, both VS and PS
		float f1CurrentTime = pShaderAPI->CurrentTime();

		// vc49, vc50
		// Lets start with setting up the VS.
		// We always need the BumpTransform
		// 
		// Stock-Consistency, no Transforms for Cheap Water
		// I allow it with $ExpensiveWaterEditorPreview though
		if (bInEditor && bExpensiveWaterEditorPreview)
		{
			SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BumpTransform);
		}
		else
		{
			float4 f4NoTransformX = float4(1.0f, 0.0f, 0.0f, 0.0f);
			float4 f4NoTransformY = float4(0.0f, 1.0f, 0.0f, 0.0f);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_TEXTURETRANSFORM_01, f4NoTransformX);
			pShaderAPI->SetVertexShaderConstant(LUX_VS_TEXTURETRANSFORM_01 + 1, f4NoTransformY);
		}

		// vc51, vc52
		// Allow the Surface to use its own Transform Parameter ( in case it should be standing still etc )
		// If there is no second Transform, use the BumpTransform
		// Unfortunately Proxies don't define the Parameter, so I added a bool to indicate the usage instead.
		if (bHasBaseTexture)
		{
			if (GetBool(Surface_SecondTexCoord))
			{
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, Surface_Transform);
			}
			else
			{
				SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform);
			}
		}

		// Scrolling values for Multitexture
		// vc53
		if (bHasMultiTexture)
		{
			float4 f4Scrolls;
			f4Scrolls.xy = GetFloat2(Scroll1);
			f4Scrolls.zw = GetFloat2(Scroll2);
			f4Scrolls = f4Scrolls * f1CurrentTime;
			pShaderAPI->SetVertexShaderConstant(LUX_VS_TEXTURETRANSFORM_03, f4Scrolls);
		}
		
		// c50, c51 <- c51 conflicts with EnvMap Constants
		// Don't actually need these!!
		if (false)
		{
			// These Matrices are pretty much Stock ASW Code. Nothing to change here.
			// "These constants are used to rotate the world space water normals around the up axis to align the
			// normal with the camera and then give us a 2D offset vector to use for reflection and refraction uv's" - ASW Shader
			VMatrix mx_View;
			pShaderAPI->GetMatrix(MATERIAL_VIEW, mx_View.m[0]); // Use Base() instead?
			mx_View = mx_View.Transpose3x3(); // MATERIAL_VIEW comes Transposed, detranspose it

			// We use Vector4D instead of float4 because of Normalize() and Cross()
			Vector4D v4CameraRight(mx_View.m[0][0], mx_View.m[0][1], mx_View.m[0][2], 0.0f);
			v4CameraRight.z = 0.0f; // "Project onto the plane of water" - ASW Shader
			v4CameraRight.AsVector3D().NormalizeInPlace();

			// "I assume the water surface normal is pointing along z!" - ASW Shader
			// This references Water in Source always being a flat Plane
			// The Normal of which goes straight up ( along Z, [0 0 1] )
			Vector4D v4CameraForward;
			CrossProduct(Vector(0.0f, 0.0f, 1.0f), v4CameraRight.AsVector3D(), v4CameraForward.AsVector3D());

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_050, v4CameraRight.Base(), 1);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_051, v4CameraForward.Base(), 1);
		}

		// c35, c38, c39, c40, c41, c42
		// c46, c51
		if (bUseEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.rgb = GetFloat3(ReflectTint); // Using Reflect Tint for this one
			f4EnvMapTint_LightScale.w = GetFloat(EnvMapLightScale); // We always need the LightScale.
			// Ugly ifdef..
			if (!g_pConfig->bShowSpecular
#ifdef LUX_DEBUGCONVARS
				|| lux_disablefast_envmap.GetBool()
#endif
				)
			{
				f4EnvMapTint_LightScale.rgb = float3(0.0f, 0.0f, 0.0f);
			}
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes. Yes this is a vec3 parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			// $BaseAlphaEnvMapMask, $NormalMapAlphaEnvMapMask, $EnvMapMaskFlip
			float4 f4EnvMapControls;
			f4EnvMapControls.x = 0.0f;
			f4EnvMapControls.y = HasFlag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip); // applied as abs($EnvMapMaskFlip - EnvMapMask)
			f4EnvMapControls.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// No EnvMap Fresnel Factors, Water has it's own Fresnel Stuff

			// $EnvMapOrigin, $EnvMapParallaxOBB1, $EnvMapParallaxOBB2, $EnvMapParallaxOBB3
			if(bPCC)
			{
				float4 f4EnvMapOrigin;
				f4EnvMapOrigin.xyz = GetFloat3(EnvMapOrigin);
				f4EnvMapOrigin.w = 0.0f;
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_POSITION, f4EnvMapOrigin);

				float4 f4Row1 = GetFloat4(EnvMapParallaxOBB1);
				float4 f4Row2 = GetFloat4(EnvMapParallaxOBB2);
				float4 f4Row3 = GetFloat4(EnvMapParallaxOBB3);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX, f4Row1);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_2, f4Row2);
				pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_MATRIX_3, f4Row3);
			}
		}

		// c53
		// c54
		// c55
		if (bHasFlowTexture)
		{
			// World UV Scale, Normal UV Scale, Bumpstrength, Timescale
			float4 f4FlowControls1;
			f4FlowControls1.x = 1.0f / GetFloat(Flow_WorldUVScale); // We want the Reciprocal
			f4FlowControls1.y = 1.0f / GetFloat(Flow_NormalUVScale); // We want the Reciprocal
			f4FlowControls1.z = GetFloat(Flow_BumpStrength);
			f4FlowControls1.w = f1CurrentTime * GetFloat(Flow_TimeScale); // * Time precomputed
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_053, f4FlowControls1);

			// Time Interval [s] ), UV Scrolldist., Noise Scale, Color_Flow_LerpExp
			float4 f4FlowControls2;
			f4FlowControls2.x = 1.0f / (2.0f * GetFloat(Flow_TimeIntervalInSeconds)); // [s] - in Seconds. With precomputed Reciprocal and 2.0f*
			f4FlowControls2.y = GetFloat(Flow_UVScrollDistance);
			f4FlowControls2.z = GetFloat(Flow_Noise_Scale);
			f4FlowControls2.w = GetFloat(Color_Flow_LerpExp); // Always need this for bHasBaseTexture
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_054, f4FlowControls2);

			if (bHasBaseTexture)
			{
				// Color Flow (CF) UV Scale, CF Timescale, CF Time Inteval [s], CF Uv Scrolldist.
				float4 f4FlowControls3;
				f4FlowControls3.x = 1.0f / GetFloat(Color_Flow_UVScale); // We want the Reciprocal
				f4FlowControls3.y = f1CurrentTime * GetFloat(Color_Flow_TimeScale); // * Time precomputed
				f4FlowControls3.z = 1.0f / (2.0f * GetFloat(Color_Flow_TimeIntervalInSeconds));	// [s] - in Seconds. With precomputed Reciprocal and 2.0f*
				f4FlowControls3.w = GetFloat(Color_Flow_UVScrollDistance);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_055, f4FlowControls3);
			}
		}

		// c56 Not on Cheap Water ( it only gets RefractTexture Alpha )
		// So we can use it for other Factors.
//		if (bHasRefractTexture)
//		{
//			// Converted from Gamma to Linear on Param Init
//			float4 f4RefractTint;
//			f4RefractTint.rgb = GetFloat3(InternalVar_LinearRefractTint);
//			pShaderAPI->SetPixelShaderConstant(56, f4RefractTint, 1);
//		}

		// c56
		float f1StartDistance = GetFloat(CheapWaterStartDistance);
		float f1EndDistance = GetFloat(CheapWaterEndDistance);
		float4 c56;
		c56.x = bNoFresnel ? 0.0f : 1.0f;									// f1NoFresnelMul, set to 0 for no Fresnel
		c56.y = bNoFresnel ? saturate(GetFloat(ReflectBlendFactor)) : 0.0f; // Forces Fresnel Values despite $NoFresnel
		c56.z = 1.0f / (f1EndDistance - f1StartDistance);					// f1DeltaRecip
		c56.w = f1StartDistance / (f1EndDistance - f1StartDistance);		// f1StartDivDelta
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_056, c56);

		// c57 Not on Cheap Water
		/*
		if (bHasReflectTexture)
		{
			float4 f4ReflectTint;
			// g_pConfig Header says this is for debugging, but I assume its tied to the Options Menu Reflections Setting
			if (g_pConfig->bShowSpecular)
			{
				f4ReflectTint.rgb = GetFloat3(InternalVar_LinearReflectTint); // Converted from Gamma to Linear on Param Init

				// "Need to multiply by 4 in linear space since we premultiplied into
				// the render target by .25 to get overbright data in the reflection render target."
				if (IsHDREnabled())
					f4ReflectTint.rgb = f4ReflectTint.rgb * 4.0f;

				// Cheap Water does this for Reference..
				//	f4ReflectTint.rgb = GetFloat3(ReflectTint);
			}
			// Sad empty .w

			pShaderAPI->SetPixelShaderConstant(57, "$ReflectTint", f4ReflectTint);
		}
		*/

		// c58
		float4 f4FogColor_Start;
		if (IsHDREnabled())
		{
			f4FogColor_Start.rgb = GetFloat3(InternalVar_LinearFogColor);
		}
		else
		{
			f4FogColor_Start.rgb = GetFloat3(FogColor);
		}
//		f4FogColor_Start.w = GetFloat(FogStart);
		f4FogColor_Start.w = bAboveWater ? 1.0f : -1.0f;
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_058, f4FogColor_Start);

		// c59
		float4 f4VariousControls;
		f4VariousControls.x = GetFloat(FogEnd) - GetFloat(FogStart);
		f4VariousControls.y = bNoFresnel ? 0.0f : GetFloat(ForceFresnel);
		f4VariousControls.z = IsHDREnabled() ? 4.0f : 1.0f; // Water Overbright Factor
		f4VariousControls.w = f1CurrentTime;
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_059, f4VariousControls);

		// c60
		// Not needed on Cheap Water
		/*
		float4 f4UVScalars;
		f4UVScalars.x = GetFloat(ReflectAmount);
		f4UVScalars.y = f4UVScalars.x;
		f4UVScalars.z = GetFloat(RefractAmount);
		f4UVScalars.w = f4UVScalars.z;
		pShaderAPI->SetPixelShaderConstant(60, "ReflectAmount & RefractAmount", f4UVScalars, 1);
		*/

		// c61
		// Need .w for FlowStop.. FIXME: Find a better spot for this. Preferably something thats consistent with Cheap Water
		if (bHasFlowStop)
		{
			float4 f4FlashlightTint = 0.0f;
			if(bHasFlowStop)
				f4FlashlightTint.w = GetFloat(Flow_Stop);

			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_061, f4FlashlightTint);
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };
		
//		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(false);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
//		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(false);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_water_vs30);
		SET_DYNAMIC_VERTEX_SHADER(lux_water_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_water_cheap_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_water_cheap_ps30);
	}

	Draw();
}

SHADER_DRAW
{
	// Stock-Consistency
	// We need to Draw Expensive first, then Cheap. ( Unless bReflection )
	// Its a Multipass Shader which is not obvious at first glance!!
	bool bForceExpensive = r_waterforceexpensive.GetBool();
	bool bInEditor = InHammer();
	bool bForceCheap = GetBool(ForceCheap) || bInEditor;

	if (bForceCheap)
	{
		bForceExpensive = false;
	}
	else
	{
		bForceExpensive = bForceExpensive || GetBool(ForceExpensive);
	}

	// HACKHACK:
	// The Fog Factor used in this Shader is computed in all the other Shaders.
	// The Code for it has been changed since Alien-Swarm.
	// All ASW+ Water Materials rendered with the SDK FogFactor will look wrong.
	// Vice Versa all SDK Water Materials rendered with the ASW+ FogFactor will look wrong.
	// I added a bool to switch the Behaviour ( and a ConVar to force ASW Water Fog )
	// But we still have to determine when to actually set it.
	// Water Materials should be authored with the currently dominant Fog Factor. ( SDK for TF2 etc )
	// But we need to determine when we need the new Factor and for that we can only use the new Parameters.
	// If you are making a new Material. Please set $HeightFogFactorType
	// 0 = Automatic ( ASW Features will trigger the ASW+ Factor )
	// 1 = SDK
	// 2 = ASW+
	if(!bInEditor && GetBool(AboveWater))
	{
		// FIXME: Checking this every Frame the Water is rendered is kinda cursed
		// Maybe find a way to better cache this
		int nFogFactorType = GetInt(HeightFogFactorType);

		// Automatic
		if(nFogFactorType == 0)
		{
			if(IsTextureLoaded(FlowMap) || GetBool(LightmapWaterFog))
			{
				g_bWaterAlienSwarmFogFactor = true;
			}
			else
			{
				g_bWaterAlienSwarmFogFactor = false;			
			}
		}
		// ASW
		else if (nFogFactorType == 2)
		{
			g_bWaterAlienSwarmFogFactor = true;
		}
		else // Assuming SDK (1)
		{
			g_bWaterAlienSwarmFogFactor = false;
		}
	}

	bool bRefraction = IsTextureLoaded(RefractTexture);;
	bool bReflection = bForceExpensive && IsTextureLoaded(ReflectTexture);
	bool bExpensiveEditorPreview = !GetBool(ForceCheap) && (bRefraction || bReflection);

	bool bDrewSomething = false;

	// Always draw Expensive Water, we need it for the Refract Texture
	// The only exception to this is Hammer and Forced Cheap Water Materials
	// Edit: Refract or Reflect! When you change the Water Detail Setting it will just crash your Game otherwise.
	if (!bForceCheap && !bInEditor && (bRefraction || bReflection))
	{
		bDrewSomething = true;
		DrawExpensive(pShaderShadow, pShaderAPI, bReflection);
	}

	// "Use $decal to see if we are a decal or not. . if we are, then don't bother
	// drawing the cheap version for now since we don't have access to env_cubemap"
	// 
	// Actually.. This breaks a lot of Materials, especially ASW/L4D2 ones
	// Flowmapped Materials from Swamp Levels for example never define $EnvMap.
	// Cheap Water will simply *never* render, guess what happens next?
	// Yes we just Draw(), nothing else. This will cause every Water Surface to become pure White.
	// A trip to Wireframe Land is also in if you select the Brushes :P
	// I force the Draw here, and just bind a Black Texture if no Cubemap is available!! 
	// && IsTextureLoaded(EnvMap)
	if (!bReflection && !HasFlag(MATERIAL_VAR_DECAL) && !HasFlashlight()) // NOTE: never do cheap projected Texture Water
	{
		bDrewSomething = true;
		DrawCheap(pShaderShadow, pShaderAPI, bInEditor, bExpensiveEditorPreview);
	}

	if (!bDrewSomething)
	{
		// "We are likely here because of the tools. . . draw something so that we won't go into wireframe-land."
		// ShiroDkxtro2: It's real! The Wireframe Land is real!! They weren't joking
		// It's Approx. ~12 550 821 Blocks and a Decal Flag away from Spawn.. But it's real!
		// 
		// Not calling this causes Hammer to Crash with a Message about a "corrupt" Water Material.
		// ASW doesn't do this, and it works fine.
		Draw();
	}
}
END_SHADER