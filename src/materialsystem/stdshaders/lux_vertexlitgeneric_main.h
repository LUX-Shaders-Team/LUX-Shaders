//===================== File of the LUX Shader Project =====================//
//
//  Original D. :	07.02.2023 DMY - VLG Simple Earliest Date
//	Initial D.	:	06.05.2026 DMY
//	Last Change :	18.05.2026 DMY
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

#if !defined(VLG_SIMPLE)
	#define VLG_SIMPLE 0
#endif

#if !defined(PROJTEX)
	#define PROJTEX 0
#endif

#if (!PROJTEX && !defined(ENVMAPSPHERE))
	#define ENVMAPSPHERE 0
#endif

#if !defined(ENVMAPLERP)
	#define ENVMAPLERP 0
#endif

#if !defined(PHONG)
	#define PHONG 0
	#define WRINKLEMAPS 0
#endif

#if !defined(LIGHTMAPPED_MODEL)
	#define LIGHTMAPPED_MODEL 0
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

#include "lux_common_detailtexture.h"
#include "lux_common_envmap.h"

	#include "lux_common_lightwarp.h"

#if !VLG_SIMPLE
	#include "lux_common_normalmap.h"
#endif

#if PROJTEX
	#if PHONG
		#include "lux_common_phong_flashlight.h"
	#else
		#include "lux_common_flashlight.h"
	#endif
#else
	#include "lux_common_selfillum.h"

	#if !VLG_SIMPLE
		#include "lux_common_lighting.h"

		#if PHONG
			#include "lux_common_phong.h"
		#endif
	#endif
#endif

// Some Constants and a Sampler specific to VLG Simple
#if (!PROJTEX && VLG_SIMPLE)

	#if !defined(ASWSDK)
		// Only for Model Lightmaps.
		sampler Sampler_Lightmap		: register(s11);
	#endif	
	// ASW Feature, specific to this Shader! Not found on bump and phong
	// Reusing EnvMap Pos since that's not a Thing on this Shader
	const float3 cAlphaEnvMapMaskParams : register(LUX_PS_FLOAT_ENVMAP_POSITION);
	#define g_f1BaseAlphaScale	(cAlphaEnvMapMaskParams.x)
	#define g_f1BaseAlphaBias	(cAlphaEnvMapMaskParams.y)
	#define g_f1BaseAlphaExp	(cAlphaEnvMapMaskParams.z)
#endif

#if (PHONG && WRINKLEMAPS)
	sampler Sampler_Compress		: register(s1);
	sampler Sampler_Stretch			: register(s3);
	sampler Sampler_BumpCompress	: register(s9);
	sampler Sampler_BumpStretch		: register(s10);
#endif

#if defined(ASWSDK)
	const float4 cScreenSizes		: register(LUX_PS_FLOAT_ASW_SCREENSIZE);
	#define g_f2ScreenTexelSize (cScreenSizes.xy)
	#define g_f2ScreenHalfTexel (cScreenSizes.zw)

	const float4 cSSAOControls		: register(LUX_PS_FLOAT_ASW_SSAOCONTROLS);
	#define g_f1SSAOStrength (cSSAOControls.x)

	sampler Sampler_SSAO			: register(s11);
#endif

//==========================================================================//
//	PS Input
//==========================================================================//
struct PS_INPUT
{
#if defined(ASWSDK)
	float2 ScreenPos				: VPOS;
#endif

	float4 WorldPos_ProjPosZ		: TEXCOORD0;
    float4 TexCoords1				: TEXCOORD1;
	float4 TexCoords2				: TEXCOORD2;

#if VLG_SIMPLE
	float3 VertexLighting			: TEXCOORD3;
	float2 SpecialTexCoord			: TEXCOORD4;
	float4 VertexColor				: COLOR0;
	float4 Normal					: NORMAL;
#else // BumpMapped & PHong
	#if !PROJTEX
		float4 LightAtten			: TEXCOORD3;
	#endif

	#if WRINKLEMAPS
		float WrinkleWeight			: TEXCOORD4;
	#endif

	float3 Tangent					: TANGENT;
	float3 Binormal					: BINORMAL;
	float4 Normal					: NORMAL;
#endif
};

//==========================================================================//
//	PS Entry Point
//==========================================================================//
float4 main(PS_INPUT i) : COLOR
{
	//	Getting our PS_INPUT
	//==========================================================================//

	// VPOS
	#if defined(ASWSDK)
		float2 f2ScreenUV = i.ScreenPos * g_f2ScreenTexelSize + g_f2ScreenHalfTexel;
	#endif

	// TEXCOORD0
	float3 f3WorldPos = i.WorldPos_ProjPosZ.xyz;
	float  f1Depth = i.WorldPos_ProjPosZ.w;

	#if VLG_SIMPLE
		// TEXCOORD1
		float2 f2BaseUV = i.TexCoords1.xy;

		// TEXCOORD1, TEXCOORD2
		#if ENVMAPMASK
			float2 f2EnvMapMaskUV = i.TexCoords1.zw;
			float2 f2DetailUV = i.TexCoords2.xy;
		#else
			float2 f2DetailUV = i.TexCoords1.zw;
		#endif

		#if !PROJTEX
			// TEXCOORD3
			float3 f3VertexLighting = i.VertexLighting;

			// TEXCOORD4
			float2 f2SpecialUV = i.SpecialTexCoord.xy;
		#endif

		// NORMAL
		float3 f3NormalWS = i.Normal.xyz;
	#else
		// TEXC00RD1
		float2 f2BaseUV = i.TexCoords1.xy;
		float2 f2NormalUV = i.TexCoords1.zw;

		// TEXCOORD2
		#if (!PROJTEX && ENVMAPMASK)
			float2 f2EnvMapMaskUV = i.TexCoords2.xy;
			float2 f2DetailUV = i.TexCoords2.zw;
		#else
			float2 f2DetailUV = i.TexCoords2.xy;
		#endif	// TEXC00RD1

		// TEXCOORD3
		float3x3 TBN_Matrix = float3x3(i.Tangent, i.Binormal, i.Normal.xyz);

		#if !PROJTEX
			// TEXCOORD4
			float4 f4LightAtten = i.LightAtten;
		#endif
	#endif

	//==========================================================================//
	//	$BaseTexture
	//==========================================================================//
	float4 f4BaseTexture = tex2D(Sampler_BaseTexture, f2BaseUV);

	// $BumpMap and $Phong paths want their Normal Maps about now
	#if !VLG_SIMPLE
		float4 f4NormalTS = tex2D(Sampler_NormalMap, f2NormalUV);

		#if (PHONG && WRINKLEMAPS)
			float f1WrinkleWeight = i.WrinkleWeight;
		
			// WrinkleWeight comes in [-1.0, +1.0f] Range
			// Compression will happen with negative Values and Stretching with positive Values.
			// Stock Shader does saturate(-WrinkleWeight) and saturate(WrinkleWeight)
			// I assume the Compiler will 'optimise' this as
			// min r0.x 0.0f f1WrinkleWeight
			// max r0.y 0.0f -f1WrinkleWeight
			// abs r0.y r0.y
			// Unless it can find an Instruction with _abs (doubt) or _sat
			// I think mad_sat is a thing?
			// I doubt the Compiler is smart enough to optimise the Equation we use the Weights on though.. ( it should have 1x MUL and 2x MAD )
			float f1CompressWeight = saturate(-f1WrinkleWeight); 
			float f1StretchWeight = saturate(f1WrinkleWeight);
		
			// Should sum to 1
			float f1TextureWeight = 1.0f - f1CompressWeight - f1StretchWeight;
		
			// The Textures:
			// If we use $BumpCompress and $BumpStretch but not $Compress and $Stretch, these two will just be the $BaseTexture
			float4 f4CompressTexture = tex2D(Sampler_Compress, f2BaseUV);
			float4 f4StretchTexture  = tex2D(Sampler_Stretch, f2BaseUV);
		
			// If we use $Compress and $Stretch but not $BumpCompress and $BumpStretch, these two will just be $BumpMap
			float4 f4BumpCompressTexture = tex2D(Sampler_BumpCompress, f2NormalUV);
			float4 f4BumpStretchTexture  = tex2D(Sampler_BumpStretch, f2NormalUV);
		
			// $BaseTexture WrinkleMapping
			// Stock-Consistency: Only apply to .rgb
			f4BaseTexture.rgb =  f1TextureWeight  * f4BaseTexture.rgb;
			f4BaseTexture.rgb += f1CompressWeight * f4CompressTexture.rgb;
			f4BaseTexture.rgb += f1StretchWeight  * f4StretchTexture.rgb;
		
			// $BumpMap WrinkleMapping
			f4NormalTS = f1TextureWeight  * f4NormalTS;
			f4NormalTS += f1CompressWeight * f4BumpCompressTexture;
			f4NormalTS += f1StretchWeight  * f4BumpStretchTexture;
		#endif
	#endif

	//==========================================================================//
	//	Detail Texture. Blendmodes 0 to 9 and 11
	//==========================================================================//
	#if DETAILTEXTURE
		int nDetailBlendMode = trunc(g_f1DetailBlendMode);

		float4 f4DetailTexture = tex2D(Sampler_DetailTexture, f2DetailUV);
		f4DetailTexture.xyz *= g_f3DetailTextureTint;

		f4BaseTexture = TextureCombine(f4BaseTexture, f4DetailTexture, nDetailBlendMode);
	#endif

	// Stock-Consistency: Apply Tint after Detail Texture
#if defined(ASWSDK)
	
	// This is where the above consistency becomes important.
	// Need this for SelfIllum and PhongAlbedoTint ( ASW Consistency )
	float3 f3UntintedBase = f4BaseTexture.rgb;
#endif

	// $BlendTintByBaseAlpha and $DesatureWithBaseAlpha
    ComputeTintAndXByBaseAlpha(f4BaseTexture, g_f3DefaultTint, g_f1DefaultAlphaFactor, BLENDTINTBYBASEALPHA, DESATURATEWITHBASEALPHA);
	
	//==========================================================================//
	//	Diffuse Lighting
	//==========================================================================//

	// Compute Diffuse Contributions
	float3 f3DirectDiffuse = 0.0f;
	float3 f3IndirectDiffuse = 0.0f;

	// We need a World-Space Normal when $BumpMap or $Phong
	#if !VLG_SIMPLE
		// Regular Tangent Space Normal Maps get perturbed
		// This scales the Normal Map to [-1..+1] Range
		float3 f3NormalTS = f4NormalTS.xyz * 2.0f - 1.0f;

		#if PHONG
			// Stock-Consistency.
			// $BaseMapAlphaPhongMask forces a flat Normal, even when using $BumpMap
			// This can be overriden now with $PhongFlatNormal
			if(g_bUseFlatNormal)
				f3NormalTS = float3(0.0f, 0.0f, 1.0f);
		#endif

		// Convert from Tangent Space to World Space, using the Tangent Matrix
		// NOTE: Matrix Vector Components are aligned horizontally, so Matrix goes into the mul() last
		float3 f3NormalWS = normalize(mul(f3NormalTS, TBN_Matrix));

		// Special Path for proj. Tex.'s
		#if PROJTEX
			#if defined(SFM_COMPATIBILITY)
				f3DirectDiffuse = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, f2ScreenUV, PROJTEXSHADOWS);
			#else // SDK and ASW Path
				f3DirectDiffuse = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, PROJTEXSHADOWS);
			#endif
		#else
			// f3DirectDiffuse is later used for $EnvMapLightScale
			f3DirectDiffuse = ComputeDirectDiffuse(f3WorldPos, f3NormalWS, i.LightAtten);
			f3IndirectDiffuse = ComputeIndirectDiffuse(f3NormalWS);
		#endif

	// If we use VLG Simple, we need Vertex Normal for the N.L Term
	#elif PROJTEX

		// FIXME: Same Code as above. Preferably split new NormalWS calc from this then calculate Lighting as
		// #if PROJTEX, #elif !VLG_SIMPLE #else .. #endif
		#if defined(SFM_COMPATIBILITY)
			f3DirectDiffuse = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, f2ScreenUV, PROJTEXSHADOWS);
		#else // SDK and ASW Path
			f3DirectDiffuse = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, PROJTEXSHADOWS);
		#endif

	// If we use VLG Simple but aren't using proj. Tex.'s, figure out what Lighting to apply
	#else
		// f3DiffuseLighting later used for EnvMapLightScale
		#if (!defined(ASWSDK) && LIGHTMAPPED_MODEL)
			// Multiply by cOverbright and convert from Gamma to Linear
			f3DirectDiffuse = GammaToLinear(tex2D(Sampler_Lightmap, f2SpecialUV).xyz * 2.0f); 
		#else
			// StaticPropLighting
			f3DirectDiffuse = f3VertexLighting;
		#endif

		// ShiroDkxtro2: This has to apply the same Way as LightmappedGeneric.
		// On the other Shaders we would be using the NdL for the UV, but we have RGB Values instead.
		#if LIGHTWARPTEXTURE
			f3DirectDiffuse *= 2.0f * tex1Dlod(Sampler_LightWarpTexture, float4(0.5f * length(f3DirectDiffuse), 0.0f, 0.0f, 0.0f)).rgb;
		#endif
	#endif

	//==========================================================================//
	//	Specular Lighting
	//==========================================================================//

	// Compute Specular Contributions
	float3 f3DirectSpecular = 0.0f;
	float3 f3IndirectSpecular = 0.0f;

	#if PHONG
		// Need these for Fresnel Terms
		float3	f3ViewDir = ComputeViewDir(f3WorldPos);
		float	f1NdotV = dot(f3NormalWS, -f3ViewDir); // Unsaturated!!

		// We prepare a Struct now with default Values
		// This will be Setup using a Function, drastically compressing the Code
		Phong_Data_t PhongData = Phong_Data_FakeConstructor(); // HLSL forced my Hand because it doesn't have constructors

		// Phong needs some special fresnel functions to look correctly
		SetupPhongFresnel(PhongData, f1NdotV);

		// Now saturate
		f1NdotV = saturate(f1NdotV);

		// Conditional Statement, yay!
		if(g_bHasBaseAlphaPhongMask)
			PhongData.f1AlphaMask = f4BaseTexture.w;
		else
			PhongData.f1AlphaMask = f4NormalTS.w;

		#if PHONGEXPONENTTEXTURE
			// Get the Pixel from the Texture into our float4.
			PhongData.f4PhongExponentTexture = tex2D(Sampler_PhongExpTexture, f2BaseUV);
		#endif

		// Helper Function that sets up all our Phong Data
		#if defined(ASWSDK)
			SetupPhongData(PhongData, f3UntintedBase);
		#else
			SetupPhongData(PhongData, f4BaseTexture.rgb);
		#endif
		
		// Diffuse + Specular. Perfect.
		#if PROJTEX
			f3DirectSpecular = ComputeProjectedTextureSpecular(PhongData, f3WorldPos, f3NormalWS, reflect(f3ViewDir, f3NormalWS), f3DirectDiffuse);
		#else
			f3DirectSpecular = ComputeDirectSpecular(PhongData, f3WorldPos, f3NormalWS, f3ViewDir, i.LightAtten);
		#endif

	// We only need ViewDir and NdotV for SelfIllumFresnel and EnvMaps
	#elif !PROJTEX
		// Data needed for SelfIllumFresnel and EnvMap
		#if (ENVMAP || SELFILLUM)
			float3 f3ViewDir = ComputeViewDir(f3WorldPos);
		
			// Phong already has this Variable. Otherwise it's only needed for $SelfIllumFresnel and EnvMapSphere
			#if (!PHONG && (SELFILLUM || (VLG_SIMPLE && ENVMAPSPHERE)))
				float f1NdotV = max(0, dot(f3NormalWS, -f3ViewDir));
			#endif
		#endif
	#endif

	#if ENVMAP
		// Use a dedicated per-Texel Mask for EnvMaps if specified
		#if ENVMAPMASK
			float4 f4EnvMapMask = tex2D(Sampler_EnvMapMask, f2EnvMapMaskUV);

		// Stock-Consistency: ASW+ has this for VLG simple, only here to make stock mats not break as bad
		// Seen on props_vehicles/flatnose_truck.vmt
		// TODO: Compiler complains about f not working with negative Values
		// Confused why this is a Problem here but not on EnvMapFresnel?
		// NOTE: Only available with VLG_SIMPLE
		#elif VLG_SIMPLE	
			float f1BaseAlphaEnvMapMask = saturate(g_f1BaseAlphaScale * pow(max(f4BaseTexture.a, 0.0f), g_f1BaseAlphaExp) + g_f1BaseAlphaBias);

		// BumpMapped Paths gets an unmodified $BaseAlphaEnvMapMask
		#else
			float f1BaseAlphaEnvMapMask = f4BaseTexture.a;
		#endif

		// Built together from Orange Box and SDK Code
		// NOTE: Originally : 0.5f * sample * DiffuseModulation * DiffuseLighting
		// We have f3SpecularLookUp * saturate(f3Lighting) below
		// So I simply default $EnvMapLightScale to 1 when using $EnvMapSphere
		// DiffuseModulation not considered but should be fine? Can just apply with $EnvMapTint..
		// 0.5f makes it appear too dark, so this should look pretty normal now!
		// Barney is happy now and he owes us a Beer.
		// INVESTIGATE: Barneys Helmet looks very desaturated, not sure why. Too much Beer?
		#if (VLG_SIMPLE && ENVMAPSPHERE)
			float3 f3EnvMap = SampleEnvMap_Sphere(f2SpecialUV);
			f3EnvMap = EnvMap_DefaultBehaviour(f3EnvMap, f1NdotV, f3DirectDiffuse);

			#if ENVMAPMASK
				f3EnvMap = EnvMapMasking(f3EnvMap, f4EnvMapMask.rgb);
			#else
				f3EnvMap = EnvMapMasking(f3EnvMap, f1BaseAlphaEnvMapMask);
			#endif

			f3IndirectSpecular = f3EnvMap;
		#else
			// Three different Paths here
			// 1. $EnvMapMask											- $BumpMap or $Phong or VLG Simple
			// 2. $BaseAlphaEnvMapMask and $NormalMapAlphaEnvMapMask	- $BumpMap or $Phong
			// 3. $BaseAlphaEnvMapMask									- Only VLG Simple
			#if ENVMAPMASK
				f3IndirectSpecular = ComputeEnvMap(f3DirectDiffuse, f3WorldPos, f3NormalWS, f3ViewDir, f4EnvMapMask.xyz);
			#elif !VLG_SIMPLE
				f3IndirectSpecular = ComputeEnvMap(f3DirectDiffuse, f3WorldPos, f3NormalWS, f3ViewDir, f1BaseAlphaEnvMapMask, f4NormalTS.w);
			#else
				f3IndirectSpecular = ComputeEnvMap(f3DirectDiffuse, f3WorldPos, f3NormalWS, f3ViewDir, f1BaseAlphaEnvMapMask);
			#endif

			#if PHONG
				// Stock-Consistency
				// EnvMap uses Phong Fresnel
				// By defining PHONG, ComputeEnvMap won't apply regular EnvMapFresnel
				f3IndirectSpecular *= lerp(1.0f, PhongData.f1PhongFresnel, g_f1EnvMapFresnelScale);
			#endif
		#endif
	#endif

	//==========================================================================//
	//	Diffuse Results
	//==========================================================================//

	float3 f3DiffuseTerm = f4BaseTexture.rgb * (f3DirectDiffuse + f3IndirectDiffuse);

	// Now on a second Pass
	/*
	#if DETAILTEXTURE
		f3Result = TextureCombinePostLighting(f3Result, f4DetailTexture.rgb, nDetailBlendMode);
	#endif
	*/

	// BaseAlpha $SelfIllum or $SelfIllumMask
	// Note that SelfIllum applies before we do Specular Results
	#if (!PROJTEX && SELFILLUM)
	
		// N.V computed above either in PHONG or after when !PROJTEX
		float f1SelfIllumFresnel = ComputeSelfIllumFresnel(f1NdotV);

		float3 f3UnlitColor;
		#if defined(ASWSDK)
			f3UnlitColor = f3UntintedBase;
		#else
			f3UnlitColor = f4BaseTexture.rgb;
		#endif

		#if SELFILLUM_ENVMAPMASK_ALPHA

			// $SelfIllum_EnvMapMask_Alpha
			// We only have one Mask and it's in the Alpha Channel of the $EnvMapMask
			f3DiffuseTerm = lerp(f3DiffuseTerm, f3UnlitColor * g_f3SelfIllumTint, f4EnvMapMask.a * f1SelfIllumFresnel);

		#else

			// Mask Selection
			float3 f3SelfIllumMask = tex2D(Sampler_SelfIllum, f2BaseUV).rgb;
			f3SelfIllumMask = lerp(f4BaseTexture.aaa, f3SelfIllumMask, g_f1SelfIllumMaskFactor) * f1SelfIllumFresnel;

			// Apply SelfIllum via Mask
			f3DiffuseTerm = lerp(f3DiffuseTerm, f3UnlitColor * g_f3SelfIllumTint, f3SelfIllumMask);
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

	//==========================================================================//
	//	SSAO ( in case of ASW )
	//==========================================================================//

#if defined(ASWSDK)
	float f1SSAO = tex2Dlod(Sampler_SSAO, float4(f2ScreenUV, 0.0f, 0.0f)).r;

	// Make it less strong if that is desired
	f1SSAO = lerp(1.0f, f1SSAO, g_f1SSAOStrength);

	f3CombinedTerms *= f1SSAO;
#endif

	float f1Alpha = f4BaseTexture.a; // Using $BaseTexture Alpha for transparency & translucency Effects
	
	// Needed for Hammer's Shaded Texture Polygons View
	// NOTE: This will look wrong with Reflections, but Hammer doesn't have Reflections
	// And the Game doesn't have Hammer. So this should be fine!
	// NOTE2: Not for Bumped Paths and not for Proj. Tex.'s either
	#if (!PROJTEX && VLG_SIMPLE)
		if(g_bVertexColor)
		{
			f3CombinedTerms *= i.VertexColor.rgb;
			f1Alpha *= i.VertexColor.a;
		}
	#endif

	return LUX_Finalise(float4(f3CombinedTerms, f1Alpha), f3WorldPos, f1Depth, g_f1AlphaModulation);
}