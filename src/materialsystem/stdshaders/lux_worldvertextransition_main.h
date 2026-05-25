//===================== File of the LUX Shader Project =====================//
//
//  Original D. :	04.03.2023 DMY - WVT Simple Earliest Date
//	Initial D.	:	04.03.2023 DMY
//	Last Change :	21.05.2026 DMY
//
//==========================================================================//

//==========================================================================//
//	Define non-existant Combos
//==========================================================================//

#if !defined(XBYBASEALPHA)
	#define XBYBASEALPHA 0
#endif

#if !defined(SELFILLUMMODE)
	#define SELFILLUMMODE 0
#endif

#if !defined(WVT_SIMPLE)
	#define WVT_SIMPLE 0
#endif

#if !defined(PROJTEX)
	#define PROJTEX 0
#endif

#if !defined(PHONG)
	#define PHONG 0
#endif

#if !defined(SSBUMP)
	#define SSBUMP 0
#endif

#if !defined(BUMPMASK)
	#define BUMPMASK 0
#endif

#if !defined ENVMAPMODE
	#define ENVMAPMODE 0
#endif

#if !defined(BLENDMODULATETEXTURE)
	#define BLENDMODULATETEXTURE 0
#endif

//==========================================================================//
//	Unpack Combos
//==========================================================================//
#if (XBYBASEALPHA > 0)
	#if (XBYBASEALPHA == 1)
		#define BLENDTINTBYBASEALPHA 1
		#define DESATURATEWITHBASEALPHA 0
	// == 2
	#else
		#define BLENDTINTBYBASEALPHA 0
		#define DESATURATEWITHBASEALPHA 1		
	#endif
#else
	#define BLENDTINTBYBASEALPHA 0
	#define DESATURATEWITHBASEALPHA 0
#endif

// $SelfIllum/$SelfIllumMask & $SelfIllum_EnvMapMask_Alpha
#if (SELFILLUMMODE > 0)
	#define SELFILLUM 1

	#if (SELFILLUMMODE == 2)
		#define SELFILLUM_ENVMAPMASK_ALPHA 1
	#else
		#define SELFILLUM_ENVMAPMASK_ALPHA 0
	#endif
#else
	#define SELFILLUM 0
	#define SELFILLUM_ENVMAPMASK_ALPHA 0
#endif

// Unpack Detail Combo
#if (DETAILTEXTURE > 0)
	#define ANYDETAILTEXTURE 1

	#if (DETAILTEXTURE == 2)
		#define DETAILTEXTURE2 1
	#else
		#define DETAILTEXTURE2 0
	#endif
#else
	#define ANYDETAILTEXTURE 0
	#define DETAILTEXTURE2 0
#endif

//==========================================================================//
//	Common Definitions
//==========================================================================//

//#define TONEMAP_SCALE_NONE
#define TONEMAP_SCALE_LINEAR
//#define TONEMAP_SCALE_GAMMA

//==========================================================================//
//	Constants, Functions, Includes
//==========================================================================//

// Include for all Pixel Shaders
#include "lux_common_ps_fxc.h"

// Remapped Constants and Samplers, and new Constants
#include "lux_worldvertextransition.h"
#include "lux_common_detailtexture.h"
#include "lux_common_lightmapped.h"
#include "lux_common_selfillum.h"
#include "lux_common_envmap.h"

#if PROJTEX
	#include "lux_common_flashlight.h"

	#if PHONG
		#include "lux_common_phong_flashlight.h"
	#endif
#elif PHONG
	#include "lux_common_phong.h"
#endif

// Only for $BumpMask
#if BUMPMASK
	sampler Sampler_BumpMask : register(s6);
#endif

//==========================================================================//
//	PS Input
//==========================================================================//	
struct PS_INPUT
{
	// lux_displacement_vs30
	float4 WorldPos_ProjPosZ		: TEXCOORD0;
	float4 BaseTexCoords			: TEXCOORD1;
	float4 TexCoords2				: TEXCOORD2;
	float4 TexCoords3				: TEXCOORD3;
	float4 TexCoords4				: TEXCOORD4;
	float4 TexCoords5				: TEXCOORD5;
	float3 BlendData				: TEXCOORD6;

	float3 Tangent					: TANGENT;
	float3 Binormal					: BINORMAL;
	float3 Normal					: NORMAL;
};

//==========================================================================//
//	PS Entry Point
//==========================================================================//
float4 main(PS_INPUT i) : COLOR
{
	//	Getting our PS_INPUT
	//==========================================================================//

	// TEXCOORD0
	float3 f3WorldPos = i.WorldPos_ProjPosZ.xyz;
	float  f1Depth = i.WorldPos_ProjPosZ.w; 

	// TEXCOORD1
	float2 f2Base1UV = i.BaseTexCoords.xy;
	float2 f2Base2UV = i.BaseTexCoords.zw;

	#if WVT_SIMPLE
		// TEXCOORD2, TEXCOORD3
		#if (ANYDETAILTEXTURE)
			#if ENVMAPMASK
				float2 f2Detail1UV = i.TexCoords3.xy;
				float2 f2Detail2UV = i.TexCoords3.zw;
			#else
				float2 f2Detail1UV = i.TexCoords2.xy;
				float2 f2Detail2UV = i.TexCoords2.zw;
			#endif
		#endif

		// TEXCOORD2
		#if ENVMAPMASK
			float2 f2EnvMapMask1UV = i.TexCoords2.xy;
			float2 f2EnvMapMask2UV = i.TexCoords2.zw;
		#endif
	#else
		// TEXCOORD2
		float2 f2Normal1UV = i.TexCoords2.xy;
		float2 f2Normal2UV = i.TexCoords2.zw;

		// TEXCOORD3
		#if (ANYDETAILTEXTURE)
			#if ENVMAPMASK
				float2 f2Detail1UV = i.TexCoords4.xy;
				float2 f2Detail2UV = i.TexCoords4.zw;
			#else
				float2 f2Detail1UV = i.TexCoords3.xy;
				float2 f2Detail2UV = i.TexCoords3.zw;
			#endif
		#endif

		// TEXCOORD4
		#if ENVMAPMASK
			float2 f2EnvMapMask1UV = i.TexCoords3.xy;
			float2 f2EnvMapMask2UV = i.TexCoords3.zw;
		#endif
	#endif

	#if !PROJTEX
		#if WVT_SIMPLE
			float2 f2LightmapUV = i.TexCoords5.xy;
		#else
			// TEXCOORD5
			// Packed Offsets into .zw so we need one less float2
			float2 f2Lightmap1UV = i.TexCoords5.xy;
			float2 f2Lightmap2UV = f2Lightmap1UV + i.TexCoords5.zw;
			float2 f2Lightmap3UV = f2Lightmap2UV + i.TexCoords5.zw;
		
			// Function expects it like this
			float4 f4LightmapUVs1 = float4(f2Lightmap1UV, 0.0f, 0.0f);
			float4 f4LightmapUVs2 = float4(f2Lightmap2UV, f2Lightmap3UV);
		#endif
	#endif

	// TEXCOORD6
	#if BLENDMODULATETEXTURE
		float2 f2BlendModulateUV = i.BlendData.xy;
	#endif

	// I saturate this to make sure the Compiler knows it can optimize (a == b)
	float f1BlendFactor = saturate(i.BlendData.z);

	// TANGENT, BINORMAL, NORMAL
	#if (!WVT_SIMPLE && (PROJTEX || ENVMAP))
		float3x3 TBN_Matrix = float3x3(i.Tangent, i.Binormal, i.Normal);
	#elif WVT_SIMPLE
		float3 f3NormalWS = i.Normal;
	#endif

	//==========================================================================//
	// Textures
	//==========================================================================//

	float4 f4BaseTexture = tex2D(Sampler_BaseTexture, f2Base1UV);
	float4 f4BaseTexture2 = tex2D(Sampler_BaseTexture2, f2Base2UV);

	#if !WVT_SIMPLE
		float4 f4NormalTexture = tex2D(Sampler_NormalMap, f2Normal1UV);
		float4 f4NormalTexture2 = tex2D(Sampler_NormalMap2, f2Normal2UV);
	#endif

	#if ANYDETAILTEXTURE
		float4 f4DetailTexture = tex2D(Sampler_DetailTexture, f2Detail1UV);
		float4 f4DetailTexture2 = f4DetailTexture;

		#if DETAILTEXTURE2
			f4DetailTexture2 = tex2D(Sampler_DetailTexture2, f2Detail2UV);
		#endif
	#endif

	#if BLENDMODULATETEXTURE
		float4 f4BlendModulateTexture = tex2D(Sampler_BlendModulate, f2BlendModulateUV);
	#endif

	#if !PROJTEX
		#if ENVMAPMASK
			float4 f4EnvMapMask = tex2D(Sampler_EnvMapMask, f2EnvMapMask1UV);
			float4 f4EnvMapMask2 = tex2D(Sampler_EnvMapMask2, f2EnvMapMask2UV);
		#endif
	
		// s13, s15
		#if (SELFILLUM && !SELFILLUM_ENVMAPMASK_ALPHA)
			float4 f4SelfIllum = tex2D(Sampler_SelfIllum, f2Base1UV);
			float4 f4SelfIllum2 = tex2D(Sampler_SelfIllum2, f2Base2UV);	
		#endif
	#endif

	//==========================================================================//
	// Texture Blending
	//==========================================================================//
	#if BLENDMODULATETEXTURE

		// Stock-Consistency
		float f1MinBlend = saturate(f4BlendModulateTexture.g - f4BlendModulateTexture.r);
		float f1MaxBlend = saturate(f4BlendModulateTexture.g + f4BlendModulateTexture.r);
		f1BlendFactor = smoothstep(f1MinBlend, f1MaxBlend, f1BlendFactor);
	#endif

	// Blended Versions of these Textures
	float4 f4BlendedBase = lerp(f4BaseTexture, f4BaseTexture2, f1BlendFactor);

	#if (!WVT_SIMPLE && !BUMPMASK)
		// Stock-Consistency:
		// Stock allows a singular $BumpMap on a Blended-Material
		// ( See also Cave Walls in Half Life 2 : Episode 2 )
		// We bind $BumpMap to $BumpMap2 if there wasn't a $BumpMap2.
		// This wastes a Texture Sample and a LRP, but replicates Stock Behavior
		float4 f4BlendedBump = lerp(f4NormalTexture, f4NormalTexture2, f1BlendFactor);
		float  f1NormalAlpha = f4BlendedBump.a;
	#endif

	#if ANYDETAILTEXTURE
		float4 f4BlendedDetail = lerp(f4DetailTexture, f4DetailTexture2, f1BlendFactor);
	#endif

	#if !PROJTEX
		#if ENVMAPMASK
			float4 f4BlendedEnvMapMask = lerp(f4EnvMapMask, f4EnvMapMask2, f1BlendFactor);
		#endif
	
		#if (SELFILLUM && !SELFILLUM_ENVMAPMASK_ALPHA)
			float4 f4BlendedSelfIllumMask = lerp(f4SelfIllum, f4SelfIllum2, f1BlendFactor);
		#endif
	#endif

	//==========================================================================//
	//	Tints ( This has to happen AFTER we Modulate! )
	//==========================================================================//

	#if ANYDETAILTEXTURE
		int nDetailBlendMode = trunc(g_f1DetailBlendMode);
		
		f4DetailTexture.rgb *= g_f3DetailTextureTint;
		f4BlendedBase = TextureCombine(f4BlendedBase, f4BlendedDetail, nDetailBlendMode);
	#endif

	// ASW uses a untinted Base for SelfIllum
	#if defined(ASWSDK)
		float3 f3UntintedBase = f4BlendedBase.rgb;
	#endif

	// $BlendTintByBaseAlpha and $DesatureWithBaseAlpha
    ComputeTintAndXByBaseAlpha(f4BlendedBase, g_f3DefaultTint, g_f1DefaultAlphaFactor, BLENDTINTBYBASEALPHA, DESATURATEWITHBASEALPHA);

	//==========================================================================//
	// Lighting
	//==========================================================================//

	#if !WVT_SIMPLE

		// Variable initialized here or when convert to WS
		float3 f3NormalTS;

		#if BUMPMASK
			// This MUST use BaseTexture UV to look the same.
			float4 f4BumpMask = tex2D(Sampler_BumpMask, f2Base1UV);
		
			// Decompress first, then sum, then normalize
			f4NormalTexture.xyz = f4NormalTexture.xyz * 2.0f - 1.0f;
			f4NormalTexture2.xyz = f4NormalTexture2.xyz * 2.0f - 1.0f;
			float3 f3CombinedNormals = normalize(f4NormalTexture.xyz + f4NormalTexture2.xyz);

			// Need to Decompress this too
			f4BumpMask.xyz = f4BumpMask.xyz * 2.0f - 1.0f;
		
			// Stock-Consistency: Very carefully replicate this:
			f3NormalTS = lerp(f4BumpMask.xyz, f3CombinedNormals, f4BumpMask.a);
		
			// This 1:1 reproduces Stock SpecularFactor for BumpMask
			float f1NormalAlpha = lerp(f4BumpMask.a, f4NormalTexture.a, g_f1LerpNormalAlpha); 
		#elif SSBUMP
			f3NormalTS = f4BlendedBump.xyz;
		#else
			// Decompress regular BumpMaps
			f3NormalTS = f4BlendedBump.xyz * 2.0f - 1.0f;
		#endif
	#endif

	float3 f3DiffuseLighting = float3(1.0f, 0.0f, 0.0f);
	#if !PROJTEX
		#if WVT_SIMPLE
			f3DiffuseLighting = ComputeLightmap(f2LightmapUV) * g_f1LightmapScaleFactor;
		#elif PHONG
			float3 f3LightDir;
			#if ANYDETAILTEXTURE
				f3DiffuseLighting = ComputeBumpedLightmap_Directional(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2,
					TBN_Matrix, f3LightDir, nDetailBlendMode, f4DetailTexture.xyz);
			#else
				f3DiffuseLighting = ComputeBumpedLightmap_Directional(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2,
					TBN_Matrix, f3LightDir);
			#endif
		#else
			#if ANYDETAILTEXTURE
				f3DiffuseLighting = ComputeBumpedLightmap(f3NormalTS, f4LightmapUVs1,f4LightmapUVs2, nDetailBlendMode, f4DetailTexture.xyz);
			#else
				f3DiffuseLighting = ComputeBumpedLightmap(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2);
			#endif
		#endif

		// Apply LightWarpTexture if desired.
		#if LIGHTWARPTEXTURE
			f3DiffuseLighting *= 2.0f * tex1Dlod(Sampler_LightWarpTexture, float4(0.5f * length(f3DiffuseLighting), 0.0f, 0.0f, 0.0f)).rgb;
		#endif
	#endif

	#if (!WVT_SIMPLE && (PROJTEX || ENVMAP))
		// If we used an SSBump we must convert it or bad Things happen to our Cubemaps...
		// ShiroDkxtro2: This does not appear to be affected by the SSBumpMathIssue
		#if SSBUMP
			// Use the original f4 for the math, as the f3 is being overwritten
			f3NormalTS  = mxBumpBasis[0] * f4BlendedBump.xxx;
			f3NormalTS += mxBumpBasis[1] * f4BlendedBump.yyy;
			f3NormalTS += mxBumpBasis[2] * f4BlendedBump.zzz;
			f3NormalTS  = normalize(f3NormalTS);
		#endif
	
		// Convert to WorldSpace
		float3 f3NormalWS = normalize(mul(f3NormalTS, TBN_Matrix));
	#endif

	#if PROJTEX
		f3DiffuseLighting = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, PROJTEXSHADOWS);
	#endif
		
	//==========================================================================//
	//	Specular Lighting ( Direct )
	//==========================================================================//

	#if (ENVMAP || PHONG)
		float3 f3ViewDir = ComputeViewDir(f3WorldPos);

		#if(ENVMAP && !PHONG)
			float f1NdotV = saturate(dot(f3NormalWS, -f3ViewDir));
		#endif
	#endif

	float3 f3DirectSpecular = 0.0f;
	#if PHONG
		// We prepare a Struct now with default Values
		Phong_Data_t PhongData = Phong_Data_FakeConstructor(); // HLSL forced my Hand because it doesn't have Constructors
		
		// Get Extra Data
		#if (EXPONENTTEXTURE > 0)
			float4 f4ExponentTexture;
		
			#if (EXPONENTTEXTURE == 1)
				float4 f4ExponentTexture1 = tex2D(Sampler_PhongExpTexture, f2Base1UV);
				float4 f4ExponentTexture2 = tex2D(Sampler_PhongExpTexture2, f2Base2UV);
		
				// Blend
				PhongData.f4PhongExponentTexture = lerp(f4ExponentTexture1, f4ExponentTexture2, f1BlendFactor);
			#elif (EXPONENTTEXTURE == 2)
				
				// We don't need to blend.. We just force the first Tint to 0
				// Apply Exponent as is.
				PhongData.f4PhongExponentTexture = tex2D(Sampler_PhongExpTexture2, f2Base2UV);
			#endif
		#endif

		float f1NdotV = dot(f3NormalWS, -f3ViewDir);

		// This Block Handles SetupPhongFresnel
		if(true)
		{
			// Stock-Consistency
			float Fresnel = saturate(1.0f - f1NdotV);
		
			// "note: vRanges is now encoded as ((mid-min)*2, mid, (max-mid)*2) to optimize math"
			float FresnelRanges = Fresnel * Fresnel - 0.5f;
		
			// Carve Tool specifically wanted this, so here you. I hate this and it will probably look wrong!
		    float Fresnel1 = g_f3PhongFresnelRanges.y + (FresnelRanges >= 0.0f ? g_f3PhongFresnelRanges.z : g_f3PhongFresnelRanges.x) * FresnelRanges;
		    float Fresnel2 = g_f3PhongFresnelRanges2.y + (FresnelRanges >= 0.0f ? g_f3PhongFresnelRanges2.z : g_f3PhongFresnelRanges2.x) * FresnelRanges;
		
			PhongData.f1PhongFresnel = lerp(Fresnel1, Fresnel2, f1BlendFactor);
		}

		// Now saturate.
		f1NdotV = saturate(f1NdotV);
		
		// Conditional Statement, yay!
		if(g_bHasBaseAlphaPhongMask)
			PhongData.f1AlphaMask = f4BlendedBase.w;
		else
			PhongData.f1AlphaMask = f1NormalAlpha;
		
		// This Block is WVT Compatible SetupPhongData()
		if(true)
		{
			// By Default we should receive $PhongExponent on f1PhongExponent
			// If we have a $PhongExponentFactor ( and a Texture )
			// $PhongExponent should be set to 1
			// Vice Versa if $PhongExponent has a value, $PhongExponentFactor should be 0
			// That way when we are using a PhongExponentTexture, we get..
			// [0..1] * $PhongExponentFactor (149) + $PhongExponent (1)
		    PhongData.f1PhongExponent = lerp(g_f1PhongExponentParam, g_f1PhongExponentParam2, f1BlendFactor);
			
			// This includes $BaseTextureNoPhong and $BaseTexture2NoPhong
			// Color * Mask
		    PhongData.f3PhongModulation = g_f3PhongTint;
		
			// Masking. This get's a little bit tricky.
			// Since we only get one Phong Sample here not two
			// We simply have to blend the NoPhong Factor instead.
		    float bNoPhong = lerp(g_bBaseTexture1NoPhong, g_bBaseTexture2NoPhong, f1BlendFactor);
			PhongData.f3PhongModulation *= bNoPhong;
		
			// Handle overrides for $PhongExponentTexture first
			#if defined(PHONGEXPONENTTEXTURE)
		
				// Override PhongExponent
				float f1PhongExponentFactor = lerp(g_f1PhongExponentFactorParam, g_f1PhongExponentFactorParam2, f1BlendFactor);
				PhongData.f1PhongExponent += PhongData.f4PhongExponentTexture.x * f1PhongExponentFactor;
		
				// If $PhongExponentTextureMask is set, use the blue channel as PhongMask so that Base and Normal can be used for other stuff.
				if(g_bHasPhongExponentTextureMask)
					PhongData.f1AlphaMask = PhongData.f4PhongExponentTexture.b;
			#endif
		
			// No need for a conditional Statement
			// Just do abs(x - Mask) with x being 1 or 0
		    PhongData.f3PhongModulation *= abs(g_f1PhongInvertMask - PhongData.f1AlphaMask);
		
			// This should be correct, no AlbedoTint at Low Values and Tint at High Values
		    if (g_bHasPhongAlbedoTint)
		        PhongData.f3PhongModulation *= lerp(1.0f, f4BlendedBase.rgb * g_f1AlbedoTintBoost, PhongData.f4PhongExponentTexture.y);
		
			// $PhongAlbedoTint was already applied. We can still apply luminance though.
			// BUGBUG: There used to be a Parameter Combination that allowed you to mask the EnvMap using BaseMap Luminance
			// We are not reproducing that behaviour with the expectation that it was an oversight.
			// And the hopes that no one used that intentionally.
		    if (g_bHasBasemapLuminancePhongMask)
				PhongData.f3PhongModulation *= PerceptualLuminance(f4BlendedBase.rgb);
		}
		
		// Direct *and* Indirect Specular(?)
		// This is technically 'wrong' (Read: Not physically based).
		// We abuse the Direct Diffuse Color as a Direct Specular Color
		// But it also contains Indirect Diffuse, so this is wrong. Nothing we can do though
		// Valve, Ples Fix! ( Make Bounced Lighting a 'fourth' Lightmap. )
		//
		// Reuse Function from lux_common_phong
		// TF2C: use blinn phong.
		// TF2C: not sure why the sign is needed here for blinn.
		#if PROJTEX
			f3DirectSpecular = ComputeProjectedTextureSpecular(PhongData, f3WorldPos, f3NormalWS, reflect(f3ViewDir, f3NormalWS), f3DiffuseLighting);
		#else
			f3DirectSpecular = ComputeDirectBlinnSpecularLight(f3NormalWS, -f3ViewDir, f3LightDir, f3DiffuseLighting, PhongData.f1PhongExponent, PhongData.f1PhongFresnel);

			if (!g_bHasPhongWarpTexture)
				PhongData.f3PhongModulation *= PhongData.f1PhongFresnel;
		
			// f3PhongModulation is basically the Product of Luminance + $PhongTint + $PhongAlbedoTint + ..
			f3DirectSpecular *= PhongData.f3PhongModulation;
			f3DirectSpecular *= PhongData.f1AlphaMask;
			f3DirectSpecular *= g_f1PhongBoost; // Separate Term
		#endif
	#endif

	//==========================================================================//
	//	Specular Lighting ( Indirect )
	//==========================================================================//

	// Indirect-Specular Lighting
	float3 f3IndirectSpecular = 0.0f;
	#if ENVMAP

		// Masking. This get's a little bit tricky.
		// Since we only get one EnvMap Sample here not two
		// We simply have to blend the NoEnvMap Factor instead.
		float bNoEnvMap = lerp(g_f1BaseTextureNoEnvMap, g_f1BaseTexture2NoEnvMap, f1BlendFactor);

		#if ENVMAPMASK
			f3IndirectSpecular += bNoEnvMap * ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4BlendedEnvMapMask.rgb);
		#elif !WVT_SIMPLE
			f3IndirectSpecular += bNoEnvMap * ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4BlendedBase.a, f1NormalAlpha);
		#else
			f3IndirectSpecular += bNoEnvMap * ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4BlendedBase.a);
		#endif
	#endif
	
	//==========================================================================//
	//	Diffuse Results
	//==========================================================================//

	float3 f3DiffuseTerm = f4BlendedBase.rgb * f3DiffuseLighting;

	//==========================================================================//
	// SelfIllumination Methods
	//==========================================================================//

	// Despite being called EmissiveBlend it does not support Blending
	// So we can't move this to a second Pass
	#if ANYDETAILTEXTURE
		f3DiffuseTerm = TextureCombinePostLighting(f3DiffuseTerm, f4DetailTexture.rgb, nDetailBlendMode);
	#endif

	// $SelfIllum and $SelfIllumMask
	#if SELFILLUM
		#if SELFILLUM_ENVMAPMASK_ALPHA
			// $SelfIllum_EnvMapMask_Alpha
			f3DiffuseTerm = lerp(f3DiffuseTerm, f4BlendedBase.rgb * g_f3SelfIllumTint, f4EnvMapMask.w);
		#else	
			f4SelfIllum.rgb = lerp(f4BlendedBase.aaa, f4BlendedSelfIllumMask.rgb, g_f1SelfIllumMaskFactor);
			f3DiffuseTerm = lerp(f3DiffuseTerm, f4BlendedBase.rgb * g_f3SelfIllumTint, f4SelfIllum.rgb);
		#endif
	#endif

	//==========================================================================//
	//	Specular Results
	//==========================================================================//

	float3 f3SpecularTerm = f3DirectSpecular + f3IndirectSpecular;

	//==========================================================================//
	//	Results
	//==========================================================================//

	float3 f3CombinedTerms = f3DiffuseTerm + f3SpecularTerm;

	// Pass BaseAlpha onwards
	float f1Alpha;

	// Make a use of the Blue and Alpha Channel in a BlendModulateTexture
	#if BLENDMODULATETEXTURE
	if(g_bHasBlendModulateTransparency)
	{
		// Suggestion made by MrFunreal!
		// Alpha of the BaseTexture(s) might be used for BlendTintByBaseAlpha, SelfIllum or AlphaEnvMapMask.
		// This will allow Artists to use the unused Channels of the BlendModulate Texture for Transparency instead!
		f1Alpha = lerp(f4BlendModulateTexture.b, f4BlendModulateTexture.a, f1BlendFactor);
	}
	else
	#endif
	if(true)
	{
		f1Alpha = f4BlendedBase.a; // Already Blended!
	}

	// NOTE: No VertexColors here
	// We should probably have them for Texture Shaded Polygons
	// But we don't have the TexCoords for it!!

    return LUX_Finalise(float4(f3CombinedTerms, f1Alpha), f3WorldPos, f1Depth, g_f1AlphaModulation);
}