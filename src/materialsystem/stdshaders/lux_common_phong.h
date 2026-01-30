//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.02.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_PHONG
#define LUX_COMMON_PHONG

#include "lux_common_phong_data.h"
#include "lux_common_lighting.h"

// Direct Diffuse Lighting Function is in lux_common_lighting.h

//==========================================================================//
// Direct Specular Lighting Function
// Projected Textures and Brush Phong can't have a RimLight Term
//==========================================================================//
#if (!defined(BRUSH_SPECULAR) && !PROJTEX)
float3 ComputeDirectSpecularLight(float f1NdL, float3 f3Reflect, float3 f3LightDir, float3 f3LightColor, float f1SpecularExponent, float f1Fresnel, inout float3 f3RimLight)
#else
float3 ComputeDirectSpecularLight(float f1NdL, float3 f3Reflect, float3 f3LightDir, float3 f3LightColor, float f1SpecularExponent, float f1Fresnel)
#endif
{
	// L.R
	float f1RdL = saturate(dot(f3Reflect, -f3LightDir));
	float f1Specular = pow(f1RdL, f1SpecularExponent); // Raise to the Power of the Exponent

	// Copy it around a bunch of times so we get a float3
	float3	f3Specular = float3(f1Specular, f1Specular, f1Specular);

	// Warp as a function of *Specular and Fresnel
	if (g_bHasPhongWarpTexture)
	{
		// Shirodkxtro2: This must be the lod Instruction, as we use a Integer Register for the Loop.
		// If we didn't, it would error out, telling us blabla 'gradient instruction in a conditional statement'
		// It will attempt to fix the Issue by unrolling the Loop.
		// ( which is impossible, because it doesn't have a given Size, its a Register )
		// Luckily these Textures are LUT's, which in this Case shouldn't even have Mipmaps
		// mini_iridescence.vtf does have them though, but you can safely ignore that
		f3Specular *= tex2Dlod(Sampler_PhongWarpTexture, float4(f1Specular, f1Fresnel, 0, 0)).rgb;
	}
	else
	{	// If we didn't apply Fresnel through Warping, apply it manually.
//		f3Specular *= f1Fresnel;
		// This happens after sum!
	}

#if (!defined(BRUSH_SPECULAR) && !PROJTEX)
	if (g_bHasRimLight)
	{
		float3 f3LocalRimLight = pow(f1RdL, g_f1RimLightExponent);	// Raise to rim exponent
		f3LocalRimLight *= f1NdL;									// Mask with N.L
		f3LocalRimLight *= f3LightColor;

		f3RimLight += f3LocalRimLight;
	}
#endif

	return (f3Specular * f1NdL * f3LightColor) ; // Mask with N.L and Modulation ( attenuation and color )
}

#ifdef BRUSH_SPECULAR
float3 ComputeDirectBlinnSpecularLight(float3 f3NormalWS, float3 f3ViewDir, float3 f3LightDir, float3 f3LightColor, float f1SpecularExponent, float f1Fresnel)
{
	float3 f3HalfVector = normalize(f3LightDir + f3ViewDir);
	float f1NdH = saturate(dot(-f3NormalWS, f3HalfVector));
	float f1Specular = pow(f1NdH, f1SpecularExponent); // Raise to the Power of the Exponent

	// Copy it around a bunch of times so we get a float3
	float3 f3Specular = f1Specular;

	// Warp as a function of *Specular and Fresnel
	if (g_bHasPhongWarpTexture)
	{
		// Shirodkxtro2: This must be the lod Instruction, as we use a Integer Register for the Loop.
		// If we didn't, it would error out, telling us blabla 'gradient instruction in a conditional statement'
		// It will attempt to fix the Issue by unrolling the Loop.
		// ( which is impossible, because it doesn't have a given Size, its a Register )
		// Luckily these Textures are LUT's, which in this Case shouldn't even have Mipmaps
		// mini_iridescence.vtf does have them though, but you can safely ignore that
		f3Specular *= tex2Dlod(Sampler_PhongWarpTexture, float4(f1Specular, f1Fresnel, 0, 0)).rgb;
	}

	return (f3Specular * f3LightColor); // Mask with N.L and Modulation ( attenuation and color )
}
#endif

#if (!defined(BRUSH_SPECULAR) && !PROJTEX)
//==========================================================================//
// Easy to use Function to compute Direct Specular Lighting (Phong) for all 4 Lights
//==========================================================================//
float3 ComputeDirectSpecular(Phong_Data_t ph, float3 f3WorldPos, float3 f3NormalWS, float3 f3ViewDir, float4 f4LightAtten)
{
	// Prepare some Variables
	float3	f3Specular			= float3(0, 0, 0);
	float3	f3RimLight			= float3(0, 0, 0);
	float3	f3RimLightAmbient	= float3(0, 0, 0);

	// ViewDir must be an incident Vector for this to work.
	float3 f3Reflect = reflect(f3ViewDir, f3NormalWS);

	// Oof! Unrolling to avoid a Warning
	[unroll]
	for (int n = 0; n < NUM_LIGHTS_COMBO; n++)
	{
		float3 f3LightColor;
		float3 f3LightDir;
		if (n == 3)
		{
			f3LightColor = float3(cLightInfo[0].color.w, cLightInfo[0].pos.w, cLightInfo[1].color.w) * f4LightAtten[n];
			f3LightDir = normalize(f3WorldPos - float3(cLightInfo[1].pos.w, cLightInfo[2].color.w, cLightInfo[2].pos.w));
		}
		else
		{
			f3LightColor = cLightInfo[n].color.xyz * f4LightAtten[n];
			f3LightDir = normalize(f3WorldPos - cLightInfo[n].pos.xyz);
		}

		float f1L1NdL = saturate(dot(f3NormalWS, -f3LightDir));
		f3Specular += ComputeDirectSpecularLight(f1L1NdL, f3Reflect, f3LightDir, f3LightColor, ph.f1PhongExponent, ph.f1PhongFresnel, f3RimLight);
	}

	// Apply Fresnel after the Sum, only if not already done via PhongWarpTexture
	if (!g_bHasPhongWarpTexture)
	{
		f3Specular *= ph.f1PhongFresnel;
	}

	// Stock-Consistency:
	// PhongMask and PhongBoost applied only to Regular Lights
	f3Specular *= ph.f1AlphaMask * g_f1PhongBoost;

	// Urgh, I wish this was a Static Combo..
	if (g_bHasRimLight)
	{
		f3RimLightAmbient = ComputeAmbientCube(f3ViewDir, cAmbientCube);

		// Now that we added all those Lights together, mask RimLighting from them.
		// NOTE: Double Squared Fresnel with no ranges for RimLight. The Double-square is for a 'more subtle look'
		// .-Stock Consistency:
		float f1RimMultiply = ph.f1RimLightMask * ph.f1RimFresnel;

		f3RimLight *= f1RimMultiply;

		// This is what Valve did, so we have to do it... I couldn't think of a better way, that is.
		// This avoids everything blowing out from Brightness by just picking whatever is brighter
		// We have two seperate instances of Light information, and thats not very nice..
		f3Specular = max(f3Specular, f3RimLight);

		// This is probably the reason why on the VDC it says something about $RimLightExponent not working in HLMV
		// They probably meant $RimlightBoost. And it probably means there is no Ambient Cubes in HLMV.
		// This will add 0,0,0 if no ambient cubes exist.
		f3RimLightAmbient *= g_f1RimLightBoost * saturate(f1RimMultiply * f3NormalWS.z);
		f3Specular += f3RimLightAmbient;
	}

	// Tint / $PhongAlbedoTint / ..
	f3Specular *= ph.f3PhongModulation;

	return f3Specular;
}
#endif // #if !PROJTEX

#endif // End of LUX_COMMON_PHONG