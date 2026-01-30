//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.02.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_LIGHTMAPPED_H_
#define LUX_COMMON_LIGHTMAPPED_H_

// Could potentially be using a Normal Map for Bumped Lighting
#include "lux_common_normalmap.h"

// LightWarp if desired
#include "lux_common_lightwarp.h"

//==========================================================================//
//	BumpBasis for Bumped Lightmaps
//	Also See :
//	https://advances.realtimerendering.com/s2006/Mitchell-ShadingInValvesSourceEngine.pdf
//==========================================================================//

// ShiroDkxtro2 :	
//					During the Creation of the Dom. Dir. Function
//					I noticed that the first BumpBasis Value is not the same
//					as the one shown in Valves Siggraph presentation
//					
//					In the .pdf it shows an Image with this BumpBasis :
//					[	sqrt(3/2),	0,			1/sqrt(3) ]
//					[  -1/sqrt(6),  1/sqrt(2),  1/sqrt(3) ]
//					[  -1/sqrt(6), -1/sqrt(2),  1/sqrt(3) ]
//	The first Value in the BumpBasis here is
//	0.81649661064147949f, which is not the same as sqrt(3/2)... (1.22474487139158905f)
//  The same BumpBasis applies in the VRad Code ( bumpvects.h )
//	But THERE it has a Comment above it, specifically stating it to be sqrt(2/3)
//	Someone else pointed out that this Value is also the Reciprocal of sqrt(3/2)
//	Someone else pointed out that this Value is also sqrt(2) / sqrt(3), which can be found online.. referencing 'Directional Lightmaps'
//
//	This was probably just a Typo. For an In-Detail Description of how 'Directional Lightmaps & Radiosity Normal Mapping / Bumped Lightmaps' works
//	See http://www.decew.net/OSS/References/sem_ss06_07-Independant%20Explanation.pdf ( which has the correct x )
static const float3 mxBumpBasis[3] = {

	float3(  sqrt(2.0f / 3.0f),  0.0f,				1.0f / sqrt(3.0f)), // +x,  0, +z
	float3( -1.0f / sqrt(6.0f),  1.0f / sqrt(2.0f), 1.0f / sqrt(3.0f)), // -x, +y, +z
	float3( -1.0f / sqrt(6.0f), -1.0f / sqrt(2.0f), 1.0f / sqrt(3.0f))  // -x, -y, +z
};

// Everything below can't be used on Projected Textures
#if !PROJTEX

// For Bicubic Filtering, import this Header with the Weight Functions
#include "lux_common_bicubic.h"

//==========================================================================//
// PixelShader *Float* Constant Registers
//==========================================================================//

#if BICUBIC_FILTERING
	// ShiroDkxtro2: The Resolution shouldn't be hardcored like this.
	// It's consistent with Stock Shaders now but this is quite terrible.
	// 
	// If someone happens to adjusts the Lightmap Resolution in the Engine Code, 
	// they will have to find where this is and recompile all Shaders that use it..
	// We get Lightmap Page Res in the ShaderAPI why not use that!!
	static const float2 g_f2LightmapPageRes = float2(1024.0f, 512.0f);
	static const float2 g_f2LightmapTexel = 1.0f / g_f2LightmapPageRes;
#endif

//==========================================================================//
// Samplers
//==========================================================================//
#if !defined(MOVED_SAMPLERS_LIGHTMAP)
sampler Sampler_Lightmap		: register(s11);
#endif

//==========================================================================//
// 'Cleaned' Copy from Stock common_lightmappedgeneric_fxc.h
//==========================================================================//
float3 ComputeLightmap(float2 vTexCoord)
{
#if BICUBIC_FILTERING
	float2 Texcoord = vTexCoord * g_f2LightmapPageRes.xy + 0.5f;
	float2 TexCoord_Floored = floor(Texcoord);
	float2 TexCoord_Fraction = frac(Texcoord);

	float g0x = g0(TexCoord_Fraction.x);
	float g1x = g1(TexCoord_Fraction.x);
	float h0x = h0(TexCoord_Fraction.x);
	float h1x = h1(TexCoord_Fraction.x);
	float h0y = h0(TexCoord_Fraction.y);
	float h1y = h1(TexCoord_Fraction.y);
	float g0y = g0(TexCoord_Fraction.y);
	float g1y = g1(TexCoord_Fraction.y);

	// The four Texture Coordinates
	float2 TexCoord0 = (float2(TexCoord_Floored.x + h0x, TexCoord_Floored.y + h0y) - 0.5f) * g_f2LightmapTexel;
	float2 TexCoord1 = (float2(TexCoord_Floored.x + h1x, TexCoord_Floored.y + h0y) - 0.5f) * g_f2LightmapTexel;
	float2 TexCoord2 = (float2(TexCoord_Floored.x + h0x, TexCoord_Floored.y + h1y) - 0.5f) * g_f2LightmapTexel;
	float2 TexCoord3 = (float2(TexCoord_Floored.x + h1x, TexCoord_Floored.y + h1y) - 0.5f) * g_f2LightmapTexel;

	// We only want .rgb because the SDK doesn't allow us to utilise Lightmap Alpha
	return	g0y * (	g0x * tex2D(Sampler_Lightmap, TexCoord0).rgb  +
					g1x * tex2D(Sampler_Lightmap, TexCoord1).rgb) +
			g1y * (	g0x * tex2D(Sampler_Lightmap, TexCoord2).rgb  +
					g1x * tex2D(Sampler_Lightmap, TexCoord3).rgb);

#else // Regular Sample

	float3 Sample = tex2D(Sampler_Lightmap, vTexCoord).rgb;

	return Sample;
#endif
}

#if BICUBIC_FILTERING
// We do something smarter here for Bumped Lightmaps
// In the SDK it will evaluate the Bicubic Weights for all 3 Lightmaps
// But the Weights stay the same for all of them, since all that changes is their Horizontal Offset
// So reuse the weights!!!
void ComputeLightmapBumpedBicubic(float2 UV1, float2 UV2, float2 UV3,
	out float3 f3LightmapColor1, out float3 f3LightmapColor2, out float3 f3LightmapColor3)
{
	// The Offset between Lightmaps
	// In Theory, they should be identical going from 1 to 2 to 3
	// But I don't trust it, so do it like this instead
	float2 LightmapOffset_1to2 = UV2 - UV1;
	float2 LightmapOffset_2to3 = UV3 - UV2;

	float2 Texcoord = UV1 * g_f2LightmapPageRes.xy + 0.5f;
	float2 TexCoord_Floored = floor(Texcoord); // Integer Resolution
	float2 TexCoord_Fraction = frac(Texcoord); // Distance between the Integers

	float g0x = g0(TexCoord_Fraction.x);
	float g1x = g1(TexCoord_Fraction.x);
	float h0x = h0(TexCoord_Fraction.x);
	float h1x = h1(TexCoord_Fraction.x);
	float h0y = h0(TexCoord_Fraction.y);
	float h1y = h1(TexCoord_Fraction.y);
	float g0y = g0(TexCoord_Fraction.y);
	float g1y = g1(TexCoord_Fraction.y);

	// Lightmap1 not 0, 0 is the non-bumped Variant
	float2 Lightmap1_TexCoord0 = (float2(TexCoord_Floored.x + h0x, TexCoord_Floored.y + h0y) - 0.5f) * g_f2LightmapTexel;
	float2 Lightmap1_TexCoord1 = (float2(TexCoord_Floored.x + h1x, TexCoord_Floored.y + h0y) - 0.5f) * g_f2LightmapTexel;
	float2 Lightmap1_TexCoord2 = (float2(TexCoord_Floored.x + h0x, TexCoord_Floored.y + h1y) - 0.5f) * g_f2LightmapTexel;
	float2 Lightmap1_TexCoord3 = (float2(TexCoord_Floored.x + h1x, TexCoord_Floored.y + h1y) - 0.5f) * g_f2LightmapTexel;

	float2 Lightmap2_TexCoord0 = Lightmap1_TexCoord0 + LightmapOffset_1to2;
	float2 Lightmap2_TexCoord1 = Lightmap1_TexCoord1 + LightmapOffset_1to2;
	float2 Lightmap2_TexCoord2 = Lightmap1_TexCoord2 + LightmapOffset_1to2;
	float2 Lightmap2_TexCoord3 = Lightmap1_TexCoord3 + LightmapOffset_1to2;

	float2 Lightmap3_TexCoord0 = Lightmap2_TexCoord0 + LightmapOffset_2to3;
	float2 Lightmap3_TexCoord1 = Lightmap2_TexCoord1 + LightmapOffset_2to3;
	float2 Lightmap3_TexCoord2 = Lightmap2_TexCoord2 + LightmapOffset_2to3;
	float2 Lightmap3_TexCoord3 = Lightmap2_TexCoord3 + LightmapOffset_2to3;

	// Do the 12 Samples with the same Weights as Sample1
	f3LightmapColor1 = 	g0y * (	g0x * tex2D(Sampler_Lightmap, Lightmap1_TexCoord0).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap1_TexCoord1).rgb) +
						g1y * (	g0x * tex2D(Sampler_Lightmap, Lightmap1_TexCoord2).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap1_TexCoord3).rgb);

	f3LightmapColor2 = 	g0y * (	g0x * tex2D(Sampler_Lightmap, Lightmap2_TexCoord0).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap2_TexCoord1).rgb) +
						g1y * (	g0x * tex2D(Sampler_Lightmap, Lightmap2_TexCoord2).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap2_TexCoord3).rgb);

	f3LightmapColor3 = 	g0y * (	g0x * tex2D(Sampler_Lightmap, Lightmap3_TexCoord0).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap3_TexCoord1).rgb) +
						g1y * (	g0x * tex2D(Sampler_Lightmap, Lightmap3_TexCoord2).rgb +
								g1x * tex2D(Sampler_Lightmap, Lightmap3_TexCoord3).rgb);
}
#endif

//==========================================================================//
// Straight up cleaned copy from common_lightmappedgeneric_fxc.h
//==========================================================================//
void ComputeBumpedLightmapCoordinates(float4 f4LightmapTexCoord1, float4 f4LightmapTexCoord2And3, out float2 f2BumpCoord1, out float2 f2BumpCoord2, out float2 f2BumpCoord3)
{
	 // Swizzle? ( I'm being sarcastic )
	f2BumpCoord1 = f4LightmapTexCoord1.xy;
	f2BumpCoord2 = f4LightmapTexCoord2And3.xy;
	f2BumpCoord3 = f4LightmapTexCoord2And3.zw;
}

//==========================================================================//
// Somewhat modified stock bumped lightmap sampling function
//==========================================================================//
float3 ComputeBumpedLightmap(float3 f3TextureNormal, float4 f4LightmapTexCoord1, float4 f4LightmapTexCoord2And3, int nDetailBlendMode = 0, float3 f3DetailTexture = float3(0.5f, 0.5f, 0.5f))
{
	float2 f2BumpCoord1;
	float2 f2BumpCoord2;
	float2 f2BumpCoord3;

	ComputeBumpedLightmapCoordinates( f4LightmapTexCoord1, f4LightmapTexCoord2And3,	f2BumpCoord1, f2BumpCoord2, f2BumpCoord3);

#if BICUBIC_FILTERING
	float3 f3LightmapColor1;
	float3 f3LightmapColor2;
	float3 f3LightmapColor3;
	
	// Use this special Function for Bumped Bicubic so its cheaper.
	ComputeLightmapBumpedBicubic(f2BumpCoord1, f2BumpCoord2, f2BumpCoord3, f3LightmapColor1, f3LightmapColor2, f3LightmapColor3);

#else
	// Filter Bumped Lightmaps Regularly
	float3 f3LightmapColor1 = ComputeLightmap(f2BumpCoord1);
	float3 f3LightmapColor2 = ComputeLightmap(f2BumpCoord2);
	float3 f3LightmapColor3 = ComputeLightmap(f2BumpCoord3);
#endif


	//==================================//
	// Compute Bumped Lightmap
	//==================================//
	// Prepare this, will be replaced by respective normal basis
	float3 dp;
	float3 f3DiffuseLighting;
#if SSBUMP
	dp = f3TextureNormal;
#else
	// Regular bumped lightmap :
	// Check contribution on each axis
	// This only works because we assume both to be unit vectors
	dp.x = saturate(dot(f3TextureNormal, mxBumpBasis[0]));
	dp.y = saturate(dot(f3TextureNormal, mxBumpBasis[1]));
	dp.z = saturate(dot(f3TextureNormal, mxBumpBasis[2]));
	dp *= dp;
#endif

	// I have no idea what exactly p2-onward does here 
	// Referencing the original bumped lighting code ( and how it uses the ssbump ), this should make sense
	// In case of the Bumped Lightmap sample, it will multiply the sum of dp's with the detail Texture
	// And this would be the same place just that we multiply the SSBump... by the SSBump...
	// If this comment is still here then it probably looked the same as the reference ( Portal 2 Panel Material )
	// ShiroDkxtro2:	This works for both SSBumps and Regular Bumps the way I arranged the code
	//					Precompute the 2.0f* into $DetailTint
#if DETAILTEXTURE
	if(nDetailBlendMode == 10)
		dp *= f3DetailTexture; // 2.0f *
#endif

#if SSBUMP
	f3DiffuseLighting =	dp.xxx * f3LightmapColor1 +
		dp.yyy * f3LightmapColor2 +
		dp.zzz * f3LightmapColor3;
	f3DiffuseLighting *= g_f1LightmapScaleFactor;

#else
	f3DiffuseLighting =	dp.xxx * f3LightmapColor1 +
		dp.yyy * f3LightmapColor2 +
		dp.zzz * f3LightmapColor3;

	float sum = dot(dp, float3(1, 1, 1));
	f3DiffuseLighting *= g_f1LightmapScaleFactor / sum;
#endif
	return f3DiffuseLighting;
}

#if defined(BRUSH_SPECULAR)
//==========================================================================//
// Extracts the Dominant Light Direction from the Bumped Lightmaps
// Semi-based on this Thread, semi because the Code in this Thread does not work
// https://www.gamedev.net/forums/topic/673113-directional-lightmapped-specular/
//==========================================================================//
float3 ComputeBumpedLightmap_Directional(float3 f3TextureNormal, float4 f4LightmapTexCoord1, float4 f4LightmapTexCoord2And3, float3x3 TBN,
	out float3 f3DomDir, int nDetailBlendMode = 0, float3 f3DetailTexture = float3(0.5f, 0.5f, 0.5f))
{
	float2 f2BumpCoord1;
	float2 f2BumpCoord2;
	float2 f2BumpCoord3;

	ComputeBumpedLightmapCoordinates(f4LightmapTexCoord1, f4LightmapTexCoord2And3, f2BumpCoord1, f2BumpCoord2, f2BumpCoord3);

#if BICUBIC_FILTERING
	float3 f3LightmapColor1;
	float3 f3LightmapColor2;
	float3 f3LightmapColor3;

	// Use this special Function for Bumped Bicubic so its cheaper.
	ComputeLightmapBumpedBicubic(f2BumpCoord1, f2BumpCoord2, f2BumpCoord3, f3LightmapColor1, f3LightmapColor2, f3LightmapColor3);

#else
	// Filter Bumped Lightmaps Regularly
	float3 f3LightmapColor1 = ComputeLightmap(f2BumpCoord1);
	float3 f3LightmapColor2 = ComputeLightmap(f2BumpCoord2);
	float3 f3LightmapColor3 = ComputeLightmap(f2BumpCoord3);
#endif

	//==================================//
	// Compute Dominant Direction
	//==================================//
	// Average Luminance Value ( how strong, this direction )
	float3 f3Lightmap1Luminance = (float3)dot(f3LightmapColor1.rgb, (float3)1.0f / 3.0f);
	float3 f3Lightmap2Luminance = (float3)dot(f3LightmapColor2.rgb, (float3)1.0f / 3.0f);
	float3 f3Lightmap3Luminance = (float3)dot(f3LightmapColor3.rgb, (float3)1.0f / 3.0f);

	// ShiroDkxtro2
	// The contribution to the BumpBasis
	// The Compiler will probably optimise this by precomputing the Values:
	// We are running into the SSBumpMathFix Issue here again
	// the .z Contribution is too large! So we fix this up later.
	// We invert here so we get the Direction in which the Lights shine not from which they are coming.
	f3Lightmap1Luminance *= -mxBumpBasis[0]; // +x, __, z |
	f3Lightmap2Luminance *= -mxBumpBasis[1]; // -x, +y, z |
	f3Lightmap3Luminance *= -mxBumpBasis[2]; // -x, -y, z |
	
	float3 DomSum = f3Lightmap1Luminance + f3Lightmap2Luminance + f3Lightmap3Luminance;

	// ShiroDkxtro2:
	// Custom calculated Value that fixes Light glitching
	// Happens because .z Contributions are too large. ( SSBumpMathIssue 2.0 )
	// Note that this Value is negative, it must be! The Light Direction is calculated in Tangent Space,
	// Light can only ever come into the Surface, it can never come out of it. ( ignoring bounced Lighting )
	DomSum.z = -0.23f;

	// Like mentioned it's in Tangent Space, but Lighting happens in WorldSpace.
	// Need TBN to make it WS LightDir
	// We also abuse the Normalize here to Normalize the Direction ( so don't do it twice )
	f3DomDir = normalize(mul(DomSum, TBN));
	
	// Now compute the Rest of the original Function

	//==================================//
	// Compute Bumped Lightmap
	//==================================//
	// Prepare this, will be replaced by respective normal basis
	float3 dp;
	float3 f3DiffuseLighting;
#if SSBUMP
	dp = f3TextureNormal;
#else
	// Regular bumped lightmap :
	// Check contribution on each axis
	// This only works because we assume both to be unit vectors
	dp.x = saturate(dot(f3TextureNormal, mxBumpBasis[0]));
	dp.y = saturate(dot(f3TextureNormal, mxBumpBasis[1]));
	dp.z = saturate(dot(f3TextureNormal, mxBumpBasis[2]));
	dp *= dp;
#endif
	
	// I have no idea what exactly p2-onward does here 
	// Referencing the original bumped lighting code ( and how it uses the ssbump ), this should make sense
	// In case of the Bumped Lightmap sample, it will multiply the sum of dp's with the detail Texture
	// And this would be the same place just that we multiply the SSBump... by the SSBump...
	// If this comment is still here then it probably looked the same as the reference ( Portal 2 Panel Material )
	// ShiroDkxtro2: This works for both SSBumps and Regular Bumps the way I arranged the code
#if DETAILTEXTURE
		if(nDetailBlendMode == 10)
			dp *= 2.0f * f3DetailTexture;
#endif

#if SSBUMP
		f3DiffuseLighting =	dp.xxx * f3LightmapColor1 +
							dp.yyy * f3LightmapColor2 +
							dp.zzz * f3LightmapColor3;

		f3DiffuseLighting *= g_f1LightmapScaleFactor;
#else
		f3DiffuseLighting =	dp.xxx * f3LightmapColor1 +
							dp.yyy * f3LightmapColor2 +
							dp.zzz * f3LightmapColor3;

		float f1sumthing = f3LightmapColor1.z + f3LightmapColor2.z + f3LightmapColor3.z;


		float sum = dot(dp, float3(1, 1, 1));
		f3DiffuseLighting *= g_f1LightmapScaleFactor / sum;
#endif

		return f3DiffuseLighting;
}
#endif // BRUSH_SPECULAR

#endif // !PROJTEX

#endif // End of LUX_COMMON_LIGHTMAPPED_H_