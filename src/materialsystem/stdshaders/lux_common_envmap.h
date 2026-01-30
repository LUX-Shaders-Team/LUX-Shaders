//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_ENVMAP_H_
#define LUX_COMMON_ENVMAP_H_

// Cubemaps are not rendered under the flashlight.
#if !PROJTEX
//==========================================================================//
//	Constants found on all shaders with Environment Mapping.
//==========================================================================//
#if (ENVMAPMODE > 0)

// For use in Shaders via #if
#define ENVMAP 1

#if (!ENVMAPSPHERE && ENVMAPMODE > 2)
	#define ENVMAPPARALLAXCORRECTION 1
#endif

// by Defining this, it allows Shaders to move the Registers to something else
#if !defined(MOVED_REGISTERS_ENVMAP)
const float4	cEnvMapTint_LightScale			: register(LUX_PS_FLOAT_ENVMAP_TINT); 
#define			g_f3EnvMapTint					(cEnvMapTint_LightScale.rgb)
#define			g_f1EnvMapLightScale			(cEnvMapTint_LightScale.w)

const float4	cEnvMapSaturation_Contrast		: register(LUX_PS_FLOAT_ENVMAP_FACTORS);
#define			g_f3EnvMapSaturation			(cEnvMapSaturation_Contrast.rgb)
#define			g_f1EnvMapContrast				(cEnvMapSaturation_Contrast.w)

const float4	cEnvMapMaskControls				: register(LUX_PS_FLOAT_ENVMAP_CONTROLS);
#define			g_f1LerpBaseAlpha				(cEnvMapMaskControls.x)
#define			g_f1LerpNormalAlpha				(cEnvMapMaskControls.y)
#define			g_f1EnvMapMaskFlip				(cEnvMapMaskControls.z) // 1 when flipped and 0 when not flipped.
#define			g_f1EnvMapLerpFactor			(cEnvMapMaskControls.w)

const float4	cEnvMapFresnel					: register(LUX_PS_FLOAT_ENVMAP_FRESNEL);
#define			g_f1EnvMapFresnelScale			(cEnvMapFresnel.x)
#define			g_f1EnvMapFresnelBias			(cEnvMapFresnel.y)
#define			g_f1EnvMapFresnelExponent		(cEnvMapFresnel.z)

// On brushes ENVMAPMODE > 2 is PCC
// On models its envmap anisotropy ( exception unlitgeneric )
#if (ENVMAPMODE > 2 && defined(BRUSH))
const float3	g_f3CubeMapPos					: register(LUX_PS_FLOAT_ENVMAP_POSITION);
const float4x3	g_f4x3CorrectionMatrix			: register(LUX_PS_FLOAT_ENVMAP_MATRIX);
#endif // PCC


#endif // !MOVED_REGISTERS_ENVMAP

// On brushes ENVMAPMODE > 2 is PCC
// On models its envmap anisotropy
// To account for envmapmask we check for 4 too
#if (ENVMAPMODE == 2 || ENVMAPMODE == 4)

// For use in Shaders via #if
#define ENVMAPMASK 1

#if !defined(MOVED_SAMPLERS_ENVMAP)
sampler Sampler_EnvMapMask		: register(s5);
#endif
#endif

// Not using s15 due to Gamma LUT being there
#if !defined(MOVED_SAMPLERS_ENVMAP)

	// Second Sampler carrying the previous Cubemap for EnvMapLerp
	#if ENVMAPLERP
		sampler Sampler_PreviousEnvMap	: register(s12);
	#endif

sampler Sampler_EnvironmentMap	: register(s14);
#endif

// ================================================
// Reflect Vectors
// ================================================
float3 Reflect_Default(float3 f3ViewDir, float3 f3NormalWS)
{
	// This expects an incident ViewDir
	return reflect(f3ViewDir, f3NormalWS);
}

#if ENVMAPPARALLAXCORRECTION
float3 Reflect_ParallaxCorrect(float3 f3WorldPos, float3 f3ReflectionVector)
{
	// Parallax correction (2_0b and beyond)
	// Adapted from http://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
	float3 f3PositionLS = mul(float4(f3WorldPos, 1), g_f4x3CorrectionMatrix);

	// This is a float4x3, and the .w's are required but we set .w to 0.0f here? 
	// Weird, they should be ignored.
	float3	f3RayLS = mul(float4(f3ReflectionVector, 0.0f), g_f4x3CorrectionMatrix);
	float3	f3FirstPlaneIntersect = (float3(1.0f, 1.0f, 1.0f) - f3PositionLS) / f3RayLS;
	float3	f3SecondPlaneIntersect = (-f3PositionLS) / f3RayLS;
	float3	f3FurthestPlane = max(f3FirstPlaneIntersect, f3SecondPlaneIntersect);
	float	f1Distance = min(f3FurthestPlane.x, min(f3FurthestPlane.y, f3FurthestPlane.z));

	// Use distance in WS directly to recover intersection
	float3 f3IntersectPositionWS = f3WorldPos + f3ReflectionVector * f1Distance;

	return f3IntersectPositionWS - g_f3CubeMapPos;
}
#endif

// Map Reflect to Equirectangular Coordinates ( 2d )
// THIS IS NOT $SphereMap !!!
float2 ReflectToEquirectangular(float3 f3ReflectVector)
{
	float2 f2ReflectVector = float2(atan2(f3ReflectVector.y, f3ReflectVector.x), asin(f3ReflectVector.z));
	f2ReflectVector = f2ReflectVector * float2(1.0f / TWO_PI, 1.0f / PI) + 0.5f; // MAD
	
	return f2ReflectVector;
}

// LOD Version
float4 ReflectToEquirectangular(float4 f4ReflectVector)
{
	float2 f2ReflectVector = ReflectToEquirectangular(f4ReflectVector.xyz);

	// tex2Dlod instruction demands we put LOD in the last component
	return float4(f2ReflectVector.xy, 0.0f, f4ReflectVector.w);
}

// ================================================
// EnvMap Sampling
// ================================================

// Used this for PBR as it fixes the 'EnvMap MipMap Edge Seam' Issue
float3 SampleEnvMap_Equirectangular(float2 f2ReflectVector)
{
	float4 f4SpecularLookup;
	#if ENVMAPLERP
		float4 f4EnvMapA = tex2D(Sampler_PreviousEnvMap, f2ReflectVector);
		float4 f4EnvMapB = tex2D(Sampler_EnvironmentMap, f2ReflectVector);		

		f4SpecularLookup = lerp(f4EnvMapA, f4EnvMapB, g_f1EnvMapLerpFactor);
	#else
		f4SpecularLookup = tex2D(Sampler_EnvironmentMap, f2ReflectVector);
	#endif

	return f4SpecularLookup.rgb * ENV_MAP_SCALE;
}

// $SphereMap
float3 SampleEnvMap_Sphere(float2 f2ReflectVector)
{
	float4 f4SpecularLookup;
	#if ENVMAPLERP
		float4 f4EnvMapA = tex2D(Sampler_PreviousEnvMap, f2ReflectVector);
		float4 f4EnvMapB = tex2D(Sampler_EnvironmentMap, f2ReflectVector);		

		f4SpecularLookup = lerp(f4EnvMapA, f4EnvMapB, g_f1EnvMapLerpFactor);
	#else
		f4SpecularLookup = tex2D(Sampler_EnvironmentMap, f2ReflectVector);
	#endif

	return f4SpecularLookup.rgb * ENV_MAP_SCALE;
}

// EnvMap for regular Materials
float3 SampleEnvMap_Cube(float3 f3ReflectVector)
{
	float4 f4SpecularLookup;
	#if ENVMAPLERP
		float4 f4EnvMapA = texCUBE(Sampler_PreviousEnvMap, f3ReflectVector);
		float4 f4EnvMapB = texCUBE(Sampler_EnvironmentMap, f3ReflectVector);		

		f4SpecularLookup = lerp(f4EnvMapA, f4EnvMapB, g_f1EnvMapLerpFactor);
	#else
		f4SpecularLookup = texCUBE(Sampler_EnvironmentMap, f3ReflectVector);
	#endif

	return f4SpecularLookup.rgb * ENV_MAP_SCALE;
}

// In case someone implements something that makes use of the Mip Maps ( PBR )
float3 SampleEnvMap_Cube(float4 f4ReflectVector)
{
	float4 f4SpecularLookup;
	#if ENVMAPLERP
		float4 f4EnvMapA = texCUBElod(Sampler_PreviousEnvMap, f4ReflectVector);
		float4 f4EnvMapB = texCUBElod(Sampler_EnvironmentMap, f4ReflectVector);

		f4SpecularLookup = lerp(f4EnvMapA, f4EnvMapB, g_f1EnvMapLerpFactor);
	#else
		f4SpecularLookup = texCUBElod(Sampler_EnvironmentMap, f4ReflectVector);
	#endif

	return f4SpecularLookup.rgb * ENV_MAP_SCALE;
}

// ================================================
// Stock Behaviours
// ================================================

// Desaturation
float3 EnvMapSaturation(float3 f3SpecularLookup)
{
	float3 f3LookupHighSaturated = f3SpecularLookup * f3SpecularLookup;
	return lerp(f3SpecularLookup, f3LookupHighSaturated, g_f1EnvMapContrast);
}

// Contrast
float3 EnvMapContrast(float3 f3SpecularLookup)
{
	// NOTE: Must include lux_common_ps first, should really move Luminance Weights somewhere else
	float3 f3DesaturatedCubemap = PerceptualLuminance(f3SpecularLookup);
	return lerp(f3DesaturatedCubemap, f3SpecularLookup, g_f3EnvMapSaturation);
}

float3 EnvMapLightScale(float3 f3SpecularLookup, float3 f3Lighting)
{
	// Need to saturate, in case we use Additive Lighting
	return lerp(f3SpecularLookup, f3SpecularLookup * saturate(f3Lighting), g_f1EnvMapLightScale);
}

float3 EnvMapFresnel(float3 f3SpecularLookup, float f1NdotV)
{
	// ADD, MUL, MAD
	// Allow override of Fresnel for Phong
#if !defined(NO_ENVMAPFRESNEL)
	// Stock-Consistency: Squared Fresnel
	float f1Fresnel = 1.0f - f1NdotV;
	f1Fresnel *= f1Fresnel; // Squared
	f3SpecularLookup *= saturate(g_f1EnvMapFresnelScale * pow(f1Fresnel, g_f1EnvMapFresnelExponent) + g_f1EnvMapFresnelBias);
#endif
	return f3SpecularLookup;
}

float3 EnvMapMasking(float3 f3SpecularLookup, float3 f3Mask)
{
	// According to Shader Playground at optimization level 3,
	// the lerp() here will be compiled into 2 MAD instructions
	// Where as the abs version results in an ADD and an _abs extension
	// 
	// After talking some very helpful people from the DirectX Community ( Maraneshi, Devaniti, Skyth )
	// It turns out LRP Instructions are a lie, the fxc compiler will spit them out but they don't actually exist!
	// The Code from fxc.exe is fed to your drivers, it just turns into 2 fmas on the GPU. ( 2 MAD's )
	// Shader Playground gives us DXBC instructions hence why it shows 2 MAD's there
	// 
	// Overall _ABS + ADD *should* be faster.
	// They did find it kinda extreme how I'm trying to optimise a single Instruction..
	// 
	// lerp(f3CubemapMask, 1.0f - f3CubemapMask, f1EnvMapMaskFlip);
	return f3SpecularLookup * abs(g_f1EnvMapMaskFlip - f3Mask);
}

// ================================================
// Default Behaviours ( Tint, Contrast, .. )
// We always need a Normal for Reflect
// We always have a NdotV at our disposal for Fresnel!
// ================================================

float3 EnvMap_DefaultBehaviour(float3 f3SpecularLookup, float f1NdotV)
{
	f3SpecularLookup *= g_f3EnvMapTint;
	f3SpecularLookup = EnvMapContrast(f3SpecularLookup);
	f3SpecularLookup = EnvMapSaturation(f3SpecularLookup);
	f3SpecularLookup = EnvMapFresnel(f3SpecularLookup, f1NdotV);

	return f3SpecularLookup;
}

float3 EnvMap_DefaultBehaviour(float3 f3SpecularLookup, float f1NdotV, float3 f3Lighting)
{
	f3SpecularLookup *= g_f3EnvMapTint;
	f3SpecularLookup = EnvMapContrast(f3SpecularLookup);
	f3SpecularLookup = EnvMapSaturation(f3SpecularLookup);
	f3SpecularLookup = EnvMapFresnel(f3SpecularLookup, f1NdotV);
	f3SpecularLookup = EnvMapLightScale(f3SpecularLookup, f3Lighting);

	return f3SpecularLookup;
}

// ================================================
// Overloaded Functions for use in a PixelShader
// ================================================

// Sadly we need WorldPos here for PCC

// Mask [$BaseAlphaEnvMapMask], No Lighting ( UnlitGeneric for example )
float3 ComputeEnvMap(float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float f1Mask)
{
	float f1NdotV = max(0, dot(f3NormalWS, -f3ViewDir));

	// Create a Reflect Vector
	float3 f3Reflect = Reflect_Default(f3ViewDir, f3NormalWS);

#if ENVMAPPARALLAXCORRECTION
	f3Reflect = Reflect_ParallaxCorrect(f3WorldPos, f3Reflect);
#endif

	float3 f3EnvMap = SampleEnvMap_Cube(f3Reflect);
	f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV);
	f3EnvMap = EnvMapMasking(f3EnvMap, f1Mask);

	return f3EnvMap;
}

// Mask [$BaseAlphaEnvMapMask], Lighting ( VertexLit without $BumpMap )
float3 ComputeEnvMap(float3 f3Lighting, float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float f1Mask)
{
	float f1NdotV = max(0.0f, dot(f3NormalWS, -f3ViewDir));

	// Create a Reflect Vector
	float3 f3Reflect = Reflect_Default(f3ViewDir, f3NormalWS);

#if ENVMAPPARALLAXCORRECTION
	f3Reflect = Reflect_ParallaxCorrect(f3WorldPos, f3Reflect);
#endif

	float3 f3EnvMap = SampleEnvMap_Cube(f3Reflect);
	f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV, f3Lighting);
	f3EnvMap = EnvMapMasking(f3EnvMap, f1Mask);

	return f3EnvMap;
}

// Normalmapped Materials ( Lighting must exist if there is a Normal Map )
float3 ComputeEnvMap(float3 f3Lighting, float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float f1BaseAlpha, float f1NormalAlpha)
{
	float f1NdotV = max(0.0f, dot(f3NormalWS, -f3ViewDir));

	// Create a Reflect Vector
	float3 f3Reflect;
	
	f3Reflect = Reflect_Default(f3ViewDir, f3NormalWS);

#if ENVMAPPARALLAXCORRECTION
	f3Reflect = Reflect_ParallaxCorrect(f3WorldPos, f3Reflect);
#endif

	float f1Mask;
	f1Mask = lerp(1.0f, f1BaseAlpha, g_f1LerpBaseAlpha);
	f1Mask = lerp(f1Mask, f1NormalAlpha, g_f1LerpNormalAlpha);

	float3 f3EnvMap = SampleEnvMap_Cube(f3Reflect);
	f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV, f3Lighting);
	f3EnvMap = EnvMapMasking(f3EnvMap, f1Mask);

	return f3EnvMap;
}

// Mask [$EnvMapMask]
float3 ComputeEnvMap(float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float3 f3Mask)
{
	float f1NdotV = max(0.0f, dot(f3NormalWS, -f3ViewDir));

	// Create a Reflect Vector
	float3 f3Reflect = Reflect_Default(f3ViewDir, f3NormalWS);

#if ENVMAPPARALLAXCORRECTION
	f3Reflect = Reflect_ParallaxCorrect(f3WorldPos, f3Reflect);
#endif

	float3 f3EnvMap = SampleEnvMap_Cube(f3Reflect);
	f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV);
	f3EnvMap = EnvMapMasking(f3EnvMap, f3Mask);

	return f3EnvMap;
}

// Mask [$EnvMapMask], Lighting
float3 ComputeEnvMap(float3 f3Lighting, float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float3 f3Mask)
{
	float f1NdotV = max(0.0f, dot(f3NormalWS, -f3ViewDir));

	// Create a Reflect Vector
	float3 f3Reflect = Reflect_Default(f3ViewDir, f3NormalWS);

#if ENVMAPPARALLAXCORRECTION
	f3Reflect = Reflect_ParallaxCorrect(f3WorldPos, f3Reflect);
#endif

	float3 f3EnvMap = SampleEnvMap_Cube(f3Reflect);
	f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV, f3Lighting);
	f3EnvMap = EnvMapMasking(f3EnvMap, f3Mask);

	return f3EnvMap;
}
#endif // ENVMAPMODE != 0

#endif // !PROJTEX
#endif // End of LUX_COMMON_ENVMAP_H_