//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_FLASHLIGHT_H_
#define LUX_COMMON_FLASHLIGHT_H_

//==========================================================================//
// PixelShader *Float* Constant Registers
//==========================================================================//
// by Defining this, it allows Shaders to move the Registers to something else
#if !defined(MOVED_REGISTERS_FLASHLIGHT)
	const float4	cProjTexShadowTweaks			: register(LUX_PS_FLOAT_PROJTEX_TWEAKS);
	#define g_f2ProjTexTexelSize						(cProjTexShadowTweaks.xy)
	#define g_f1ProjTexShadowAtten						(cProjTexShadowTweaks.z)
	#define g_f1ProjTexNoLambertValue					(cProjTexShadowTweaks.w)

	const float4	cProjTexAttenuationFactors		: register(LUX_PS_FLOAT_PROJTEX_ATTEN);
	#define g_f3ProjTexDistanceAtten 					(cProjTexAttenuationFactors.xyz)
	#define g_f1ProjTexFarZ								(cProjTexAttenuationFactors.w)

	const float3	g_f3ProjTexColor				: register(LUX_PS_FLOAT_PROJTEX_COLOR);
	const float3	g_f3ProjTexPos					: register(LUX_PS_FLOAT_PROJTEX_POSITION);
	const float4x4	g_f4x4ProjTexWorldToTexture		: register(LUX_PS_FLOAT_PROJTEX_MATRIX);
#endif

#define NVIDIA_PCF_POISSON	0
#define ATI_NOPCF			1
#define ATI_NO_PCF_FETCH4	2

//==========================================================================//
//	Samplers
//==========================================================================//
sampler Sampler_FlashlightCookie: register(s13);
sampler Sampler_ShadowDepth		: register(s14);
sampler Sampler_RandomRotation	: register(s15);

//==========================================================================//
//	Constants
//==========================================================================//
float2 g_PoissonOffsets8[8] = {	float2( 0.3475f,	0.0042f),
								float2( 0.8806f,	0.3430f),
								float2(-0.0041f,   -0.6197f),
								float2( 0.0472f,	0.4964f),
								float2(-0.3730f,	0.0874f),
								float2(-0.9217f,   -0.3177f),
								float2(-0.6289f,	0.7388f),
								float2( 0.5744f,   -0.7741f)	};

float2 g_PoissonOffsets16[16] = {
								float2(-0.94201624, -0.39906216),
								float2(0.94558609, -0.76890725),
								float2(-0.094184101, -0.92938870),
								float2(0.34495938, 0.29387760),
								float2(-0.91588581, 0.45771432),
								float2(-0.81544232, -0.87912464),
								float2(-0.38277543, 0.27676845),
								float2(0.97484398, 0.75648379),
								float2(0.44323325, -0.97511554),
								float2(0.53742981, -0.47373420),
								float2(-0.26496911, -0.41893023),
								float2(0.79197514, 0.19090188),
								float2(-0.24188840, 0.99706507),
								float2(-0.81409955, 0.91437590),
								float2(0.19984126, 0.78641367),
								float2(0.14383161, -0.14100790)	};

//==========================================================================//
//	Stock remapping Function.
//==========================================================================//
float RemapValClamped(float val, float A, float B, float C, float D)
{
	float cVal = (val - A) / (B - A);
	cVal = saturate(cVal);

	return C + (D - C) * cVal;
}

//==========================================================================//
//	Stock SDK Shadow Filters.
//==========================================================================//
#if 0
float FilterShadow(const int nFilterMode, float f1ObjectDepth, float3 RMatTop, float3 RMatBottom, float distance = 0.0f)
{
	// Prepare these..
	float f1Shadow = 0.0f;
	float4 f4LightDepths = 0.0f;
	float4 f4Accumulated = 0.0f;
	float2 f2RotationOffset = 0.0f;

	if (nFilterMode == NVIDIA_PCF_POISSON) // NVIDIA_PCF_POISSON
	{
		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[0].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[0].xy) + RMatBottom.z;
		f4LightDepths.x += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[1].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[1].xy) + RMatBottom.z;
		f4LightDepths.y += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[2].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[2].xy) + RMatBottom.z;
		f4LightDepths.z += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[3].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[3].xy) + RMatBottom.z;
		f4LightDepths.w += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[4].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[4].xy) + RMatBottom.z;
		f4LightDepths.x += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[5].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[5].xy) + RMatBottom.z;
		f4LightDepths.y += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[6].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[6].xy) + RMatBottom.z;
		f4LightDepths.z += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[7].xy) + RMatTop.z;
		f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[7].xy) + RMatBottom.z;
		f4LightDepths.w += tex2Dproj(Sampler_ShadowDepth, float4(f2RotationOffset, f1ObjectDepth, 1)).x;

		f1Shadow = dot(f4LightDepths, float4(0.25, 0.25, 0.25, 0.25));
	}
	else if (nFilterMode == ATI_NOPCF) // ATI_NOPCF
	{
		// 'Improved' version of the ATI_NOPCF Code from Stock Code
		// Variable Sample amount, removed the nested loop
		// Array Access optimisation 
		// Easier Readability
		for (int i = 0; i < 16; i++)
		{
			float2 rotationOffset;
			rotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets16[i % 8].xy) + RMatTop.z;
			rotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets16[i % 8].xy) + RMatBottom.z;
			float lightDepth = tex2D(Sampler_ShadowDepth, rotationOffset.xy).x;

			f4Accumulated += (lightDepth > f1ObjectDepth.xxxx) ? (1.0f / 16.0f) : 0.0;
		}

		f1Shadow = dot(f4Accumulated, float4(1.0, 1.0, 1.0, 1.0));
	}
	else if (nFilterMode == ATI_NO_PCF_FETCH4) // ATI_NO_PCF_FETCH4
	{
		// Fixed version of the contact hardening code
		// Issue was (presumably) division by zero and undefined values
		float flNumCloserSamples = 1;
		float flAccumulatedCloserSamples = f1ObjectDepth;
		float4 vBlockerDepths;

		// First, search for blockers
		for (int j = 0; j < 8; j++)
		{
			f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[j].xy) + RMatTop.z;
			f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[j].xy) + RMatBottom.z;
			vBlockerDepths = tex2D(Sampler_ShadowDepth, f2RotationOffset.xy);

			// Which samples are closer than the pixel we're rendering?
			float4 vCloserSamples = (vBlockerDepths < f1ObjectDepth.xxxx); // Binary comparison results
			flNumCloserSamples += dot(vCloserSamples, float4(1, 1, 1, 1)); // How many samples are closer than receiver?
			flAccumulatedCloserSamples += dot(vCloserSamples, vBlockerDepths); // Total depths from samples closer than receiver
		}

		float flBlockerDepth = flAccumulatedCloserSamples / flNumCloserSamples;
		float flContactHardeningScale = (f1ObjectDepth - flBlockerDepth) / (flBlockerDepth + 0.0001); // Avoid division by zero

		// Scale the kernel
		RMatTop.xy *= flContactHardeningScale;
		RMatBottom.xy *= flContactHardeningScale;

		for (int i = 0; i<8; i++)
		{
			f2RotationOffset.x = dot(RMatTop.xy, g_PoissonOffsets8[i].xy) + RMatTop.z;
			f2RotationOffset.y = dot(RMatBottom.xy, g_PoissonOffsets8[i].xy) + RMatBottom.z;
			f4LightDepths = tex2D(Sampler_ShadowDepth, f2RotationOffset.xy);
			f4Accumulated += (f4LightDepths > f1ObjectDepth.xxxx);
		}
		f1Shadow = dot(f4Accumulated, float4(1.0f / 32.0f, 1.0f / 32.0f, 1.0f / 32.0f, 1.0f / 32.0f));
	}

	return f1Shadow;
}
#endif

//	1	4	7	4	1
//	4	20	33	20	4
//	7	33	55	33	7
//	4	20	33	20	4
//	1	4	7	4	1
float ComputeShadowNvidiaPCF5x5Gaussian(const float2 f2ProjectedCenter, const float f1ProjectedDepth)
{
	float2 f2Epsilon = g_f2ProjTexTexelSize.xy;
	float2 f2TwoEpsilon = g_f2ProjTexTexelSize.xy * 2.0f;

	float4 vOneTaps;
	vOneTaps.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2TwoEpsilon.x, f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vOneTaps.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2TwoEpsilon.x, f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vOneTaps.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2TwoEpsilon.x, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vOneTaps.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2TwoEpsilon.x, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	float flOneTaps = dot(vOneTaps, float4(1.0f / 331.0f, 1.0f / 331.0f, 1.0f / 331.0f, 1.0f / 331.0f));

	float4 vSevenTaps;
	vSevenTaps.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2TwoEpsilon.x, 0), f1ProjectedDepth, 1)).x;
	vSevenTaps.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2TwoEpsilon.x, 0), f1ProjectedDepth, 1)).x;
	vSevenTaps.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(0, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vSevenTaps.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(0, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	float flSevenTaps = dot(vSevenTaps, float4(7.0f / 331.0f, 7.0f / 331.0f, 7.0f / 331.0f, 7.0f / 331.0f));

	float4 vFourTapsA, vFourTapsB;
	vFourTapsA.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2TwoEpsilon.x, f2Epsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsA.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2Epsilon.x, f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsA.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2Epsilon.x, f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsA.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2TwoEpsilon.x, f2Epsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsB.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2TwoEpsilon.x, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsB.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2Epsilon.x, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsB.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2Epsilon.x, -f2TwoEpsilon.y), f1ProjectedDepth, 1)).x;
	vFourTapsB.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2TwoEpsilon.x, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	float flFourTapsA = dot(vFourTapsA, float4(4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f));
	float flFourTapsB = dot(vFourTapsB, float4(4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f));

	float4 v20Taps;
	v20Taps.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2Epsilon.x, f2Epsilon.y), f1ProjectedDepth, 1)).x;
	v20Taps.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2Epsilon.x, f2Epsilon.y), f1ProjectedDepth, 1)).x;
	v20Taps.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2Epsilon.x, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	v20Taps.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2Epsilon.x, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	float fl20Taps = dot(v20Taps, float4(20.0f / 331.0f, 20.0f / 331.0f, 20.0f / 331.0f, 20.0f / 331.0f));

	float4 v33Taps;
	v33Taps.x = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(f2Epsilon.x, 0), f1ProjectedDepth, 1)).x;
	v33Taps.y = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(-f2Epsilon.x, 0), f1ProjectedDepth, 1)).x;
	v33Taps.z = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(0, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	v33Taps.w = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter + float2(0, -f2Epsilon.y), f1ProjectedDepth, 1)).x;
	float fl33Taps = dot(v33Taps, float4(33.0f / 331.0f, 33.0f / 331.0f, 33.0f / 331.0f, 33.0f / 331.0f));

	float flCenterTap = tex2Dproj(Sampler_ShadowDepth, float4(f2ProjectedCenter, f1ProjectedDepth, 1)).x * (55.0f / 331.0f);

	// Sum all 25 Taps
	return flOneTaps + flSevenTaps + +flFourTapsA + flFourTapsB + fl20Taps + fl33Taps + flCenterTap;
}

//==========================================================================//
// Computes Flashlight Shadow from Depth Textures
//==========================================================================//
float InternalProjectedTextureShadow(float2 f2DepthTextureUV, float f1ComparisonDepth, float f1DistanceFalloff, const bool bDoShadows = false)
{
	if (bDoShadows)
	{
		// We would usually have some alternative Shadow Filters here ( Their Functions remain above )
		// But we only do this for LUX since it drastically speeds up Compiles and just looks better
		float f1Shadow = ComputeShadowNvidiaPCF5x5Gaussian(f2DepthTextureUV, f1ComparisonDepth);

		// "Blend between fully attenuated and not attenuated"
		float f1Attenuated = lerp(f1Shadow, 1.0f, g_f1ProjTexShadowAtten);

		// "Blend between shadow and the above, according to light attenuation"
		f1Shadow = saturate(lerp(f1Attenuated, f1Shadow, f1DistanceFalloff));

		return f1Shadow;
	}
	else
		return 1.0f;
}

//==========================================================================//
// Direct Diffuse Projected Texture Function
//==========================================================================//
float3 ComputeProjectedTextureDiffuse(float3 f3WorldPos, float3 f3NormalWS, const bool bDoShadows = false)
{
	// ShiroDkxtro2:
	// Here's how this works, in case whoever reads this doesn't know how Projected Textures work.
	// This has a couple of Names so let's start with that. This Feature is known as
	// 'The Flashlight', 'Shadow Mapping', or just 'Projected Textures'
	// I will be using the Term 'Projected Texture' because I believe it makes the most sense.
	// This isn't exclusive to the Player Flashlight ( at least not anymore )
	// And it doesn't necessarily have Shadows ( Shadow Mapping implies it does )
	// 
	// To make it simpler we will assume Projected Textures always have Shadows ( for now ).
	// When a Projected Texture, it renders the Scene Depth to a Texture,
	// it uses a dedicated ViewProj Matrix (g_f4x4ProjTexWorldToTexture) just like a regular View.
	// Geometry will overdraw and what you are left with in the End is the closest Depth for each Pixel.
	// In the Shader we can apply the same Equation. .xy will be a Screen UV and .z will be the Depth of the Current Pixel in the Shadow View.
	// By comparing the current Depth to the Scene Depth on the Texture ( that has lots of overdraw ),
	// we can determine which Pixels have a blocked Sightline with the Lightsource ( are in a Shadow )
	//
	// After applying Perspective Divide we kind of have a Problem however.
	// When outputting the ProjPos from the Vertex Shader, it gets clipped to the -1.0f to 1.0f Range ( on x and y )
	// Here however, it doesn't. Areas outside the Shadow View will have Values > 1.0f or < -1.0f
	// And this is where it gets really smart ( imo ) and why this *SHOULD* be called Projected Textures.
	// 
	// The ViewProj we have ( !!! g_f4x4ProjTexWorldToTexture !!! ) has precomputed within it: * 0.5f + 0.5f
	// So instead of -1.0f to +1.0f we get 0.0f to 1.0f !! ( See where this is going? )
	// We can use this *UV* to Sample the so-called Flashlight *Cookie*
	// The Flashlight Cookie is 1.0f in the Center and 0.0f at the Border.
	// It has Clamp Flags applied so any Areas with a UV of < 0.0f or > 1.0f will be *black*
	// We use it as a Mask to determine which Areas are outside the Shadow View.
	// ( Another Option would be saturate() but it would mean rectangular Lightsources. )
	//
	// It's called a Flashlight Cookie, because the Results will be whatever is left over after we stencil the Dough with the Cookie Cutter
	// ( Look at some 'Cookie Cutter' Images if you still don't understand )
	//
	// To make the Light look nicer we apply some Distance Attenuation and some hacky Math
	float4 f4ProjTexPos = mul(float4(f3WorldPos, 1.0), g_f4x4ProjTexWorldToTexture);

	// Clip all Pixels behind the Spotlight
	clip(f4ProjTexPos.w);

	// Perspective Divide is a must
	float3 f3ProjPos = f4ProjTexPos.xyz / f4ProjTexPos.www;

	// Color * Cookie
	float3 f3ProjTexColor = g_f3ProjTexColor.rgb * tex2D(Sampler_FlashlightCookie, f3ProjPos.xy).rgb;

	// We want to normalize.
	// normalize(v) does
	// v / sqrt(dot(v, v))
	// sqrt(dot(v, v)) is the Magnitude/Distance of the Vector
	// And we want that for our Falloff Factors.
	// So instead of using the intrinsic normalize() Function, we do it manually!
	float3 f3Delta = f3WorldPos - g_f3ProjTexPos;
	float f1DistSquared = dot(f3Delta, f3Delta);
	float f1Dist = sqrt(f1DistSquared);
	float3 f3LightDir = f3Delta / f1Dist; // The true Nature of normalize()

	// The 0.6f here is probably a Magic Number..
	float f1EndFalloffFactor = RemapValClamped(f1Dist, g_f1ProjTexFarZ, 0.6f * g_f1ProjTexFarZ, 0.0f, 1.0f);

	// "Attenuation for light and to fade out shadow over distance"
	float f1Attenuation = saturate(dot(g_f3ProjTexDistanceAtten, float3(1.0f, 1.0f / f1Dist, 1.0f / f1DistSquared)));

	// Compute Projected Texture Shadows
	float f1Shadow = InternalProjectedTextureShadow(f3ProjPos.xy, min(f3ProjPos.z, 0.999999f), f1Attenuation, bDoShadows);

	float3 f3DirectDiffuseLighting = f3ProjTexColor;
	f3DirectDiffuseLighting *= f1Attenuation;
	f3DirectDiffuseLighting *= f1Shadow;
	f3DirectDiffuseLighting *= f1EndFalloffFactor;

	// ShiroDkxtro2: This is pretty Important to avoid Noise at grazing Angles
	// NoLambertValue is either 0 or 2
	// The dot could be -1 so it has to be +2 not +1 in order to force No-Lambert Lighting
	// Strangely enough, doesn't support Half-Lambert
	// NOTE: Incident Light Vector so flip it
	float f1NdL = saturate(dot(-f3LightDir.xyz, f3NormalWS) + g_f1ProjTexNoLambertValue);
	f3DirectDiffuseLighting *= f1NdL;
	
	return f3DirectDiffuseLighting;
}

#endif // End of LUX_COMMON_FLASHLIGHT_H_