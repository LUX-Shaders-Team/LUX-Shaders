//===================== File of the LUX Shader Project =====================//
//
//  Original D. :	23.01.2023 DMY - LMG Simple Earliest Date
//	Initial D.	:	21.05.2026 DMY
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

#if !defined(BICUBIC_FILTERING)
	#define BICUBIC_FILTERING 0
#endif

#if !defined(LMG_SIMPLE)
	#define LMG_SIMPLE 0
#endif

#if !defined(PROJTEX)
	#define PROJTEX 0
#endif

#if !defined(PHONG)
	#define PHONG 0
#endif

#if !defined(EXPONENTTEXTURE)
	#define EXPONENTTEXTURE 0
#elif EXPONENTTEXTURE
	#define PHONGEXPONENTTEXTURE 1
#endif

#if !defined(SSBUMP)
	#define SSBUMP 0
#endif

#if !defined(DETAILTEXTURE)
	#define DETAILTEXTURE 0
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

// Need this for PCC
#define BRUSH

#if PHONG
	// We use different Equations for Phong on Brush-based Geometry
	// This Define will enable that in the Header
	#define BRUSH_SPECULAR
#endif

//==========================================================================//
//	Constants, Functions, Includes
//==========================================================================//

#include "lux_common_ps_fxc.h"
#include "lux_common_detailtexture.h"
#include "lux_common_lightmapped.h"

#if PROJTEX
	#if PHONG
		#include "lux_common_phong_data.h"
		#include "lux_common_phong_flashlight.h"
	#else
		#include "lux_common_flashlight.h"
	#endif
#else
	#if PHONG
		#include "lux_common_phong_data.h"
		#include "lux_common_phong.h"
	#endif

	#include "lux_common_selfillum.h"
	#include "lux_common_envmap.h"
#endif

#if defined(ASWSDK)
	const float4 cScreenSizes		: register(LUX_PS_FLOAT_ASW_SCREENSIZE);
	#define g_f2ScreenTexelSize (cScreenSizes.xy)
	#define g_f2ScreenHalfTexel (cScreenSizes.zw)

	const float4 cSSAOControls		: register(LUX_PS_FLOAT_ASW_SSAOCONTROLS);
	#define g_f1SSAOStrength (cSSAOControls.x)

	sampler Sampler_SSAO			: register(s12);
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

#if PROJTEX
	// lux_brush_simplified_vs30
	float3 Tangent					: TANGENT;
	float3 Binormal					: BINORMAL;
	float3 Normal					: NORMAL;

#else // $BumpMap & $Phong

	// lux_brush_vs30
	float4 LightmapTexCoord			: TEXCOORD3;
	float4 LightmapTexCoord2And3	: TEXCOORD4;
	
	float3 VertexColors				: COLOR0;

	float3 Tangent					: TANGENT;
	float3 Binormal					: BINORMAL;
	float3 Normal					: NORMAL;
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

	// TEXCOORD1, TEXCOORD2
	float2 f2BaseUV = i.TexCoords1.xy;
	#if LMG_SIMPLE

		// $EnvMapMask TexCoord is on .zw unless proj. Tex. Pass which doesn't compute $EnvMap
		#if ENVMAPMASK
			float2 f2EnvMapMaskUV = i.TexCoords1.zw;
			float2 f2DetailUV = i.TexCoords2.xy;
		#else
			float2 f2DetailUV = i.TexCoords1.zw;
		#endif

		// NORMAL
		float3 f3NormalWS = i.Normal;
	#else
		// $BumpMap TexCoord is always on the .zw of TexCoord1
		float2 f2NormalUV = i.TexCoords1.zw;

		#if ENVMAPMASK
			float2 f2EnvMapMaskUV = i.TexCoords2.xy;
			float2 f2DetailUV = i.TexCoords2.zw;
		#else
			float2 f2DetailUV = i.TexCoords2.xy;
		#endif

		// TANGENT, BINORMAL, NORMAL
		float3x3 TBN_Matrix = float3x3(i.Tangent, i.Binormal, i.Normal);
	#endif

	#if !PROJTEX
		// TEXCOORD3, TEXCOORD4
		float4 f4LightmapUVs1 = i.LightmapTexCoord;
		float4 f4LightmapUVs2 = i.LightmapTexCoord2And3;
	#endif

	//==========================================================================//
	//	..
	//==========================================================================//

	float4 f4BaseTexture = tex2D(Sampler_BaseTexture, f2BaseUV.xy);

	// Apply $Detail first for ASW-Consistency
	#if DETAILTEXTURE
		int nDetailBlendMode = trunc(g_f1DetailBlendMode);
	
		float4 f4DetailTexture = tex2D(Sampler_DetailTexture, f2DetailUV.xy);
	
		f4DetailTexture.xyz *= g_f3DetailTextureTint;
		f4BaseTexture = TextureCombine(f4BaseTexture, f4DetailTexture, nDetailBlendMode);
	#endif

	// ASW-Consistency: Untinted Base for SelfIllum
	#if defined(ASWSDK)
		float3 f3UntintedBase = f4BaseTexture.rgb;
	#endif

	// $BlendTintByBaseAlpha and $DesatureWithBaseAlpha
	ComputeTintAndXByBaseAlpha(f4BaseTexture, g_f3DefaultTint, g_f1DefaultAlphaFactor, BLENDTINTBYBASEALPHA, DESATURATEWITHBASEALPHA);

	//==========================================================================//
	// Diffuse Lighting
	//==========================================================================//

	// Defaulting this to Red in case a Path is missed
	float3 f3DiffuseLighting = float3(1.0f, 0.0f, 0.0f);

	// Lightmaps only with !PROJTEX
	#if (!PROJTEX && LMG_SIMPLE)
		// Contains both Direct Diffuse and Indirect Diffuse
		float2 f2LightmapUV = f4LightmapUVs1.xy;
		f3DiffuseLighting = ComputeLightmap(f2LightmapUV) * g_f1LightmapScaleFactor;
	#elif (!LMG_SIMPLE)
		// Compute $BumpMap
		// Need f3 not f4
		float4 f4NormalTS = tex2D(Sampler_NormalMap, f2NormalUV.xy);
		float3 f3NormalTS = f4NormalTS.xyz;

		#if !SSBUMP
			// Regular Tangent Space Normal Maps get perturbed
			// This scales the Normal Map to [-1..+1] Range
			f3NormalTS = f3NormalTS.xyz * 2.0f - 1.0f;
		#endif
	#endif

	// Sample Lightmap
	#if (!PROJTEX && !LMG_SIMPLE)
		#if PHONG
			// Extract DomDir for Phong
			// This is Direct Diffuse *and* Indirect Diffuse
			// Which is bad since we get our Light Dir. from it...
			// Nothing we can do though and this is better than no Specular
			float3 f3LightDir;
			#if DETAILTEXTURE
				f3DiffuseLighting = ComputeBumpedLightmap_Directional(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2, TBN_Matrix,
				f3LightDir, nDetailBlendMode, f4DetailTexture.rgb);
			#else
				f3DiffuseLighting = ComputeBumpedLightmap_Directional(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2, TBN_Matrix, f3LightDir);
			#endif
		#else
			#if DETAILTEXTURE
				f3DiffuseLighting = ComputeBumpedLightmap(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2, nDetailBlendMode, f4DetailTexture.rgb);
			#else
				f3DiffuseLighting = ComputeBumpedLightmap(f3NormalTS, f4LightmapUVs1, f4LightmapUVs2);
			#endif
		#endif

		#if LIGHTWARPTEXTURE
			f3DiffuseLighting *= 2.0f * tex2D(Sampler_LightWarpTexture, float2(0.5f * length(f3DiffuseLighting), 0.0f)).rgb;
		#endif
	#endif

	// Also, the $BumpMap is still in Tangent Space at this Point.
	// We need to make it WorldSpace for SelfIllumFresnel, EnvMaps & Phong
	// SSBumps need an additional Conversion from SSBump to regular Bump
	// This wasn't done earlier because Lightmaps don't need 
	#if (!LMG_SIMPLE && (PROJTEX || PHONG || ENVMAP || SELFILLUM))

		// ShiroDkxtro2: This does not appear to be affected by the SSBumpMathIssue
		#if SSBUMP
			// Use the original f4 for the math, as the f3 is being overwritten
			f3NormalTS  = mxBumpBasis[0] * f4NormalTS.xxx;
			f3NormalTS += mxBumpBasis[1] * f4NormalTS.yyy;
			f3NormalTS += mxBumpBasis[2] * f4NormalTS.zzz;
			f3NormalTS  = normalize(f3NormalTS);
		#endif
	
		// Convert to WorldSpace
		float3 f3NormalWS = normalize(mul(f3NormalTS, TBN_Matrix));
	#endif

	// ViewDir is needed by all of these
	#if (PHONG || ENVMAP || SELFILLUM)
		float3 f3ViewDir = ComputeViewDir(f3WorldPos);

		#if !PHONG
			float f1NdotV = saturate(dot(f3NormalWS, -f3ViewDir));
		#endif
	#endif

	// Diffuse Lighting under PROJTEX needs NormalWS
	#if PROJTEX
		f3DiffuseLighting = ComputeProjectedTextureDiffuse(f3WorldPos, f3NormalWS, PROJTEXSHADOWS);
	#endif

	//==========================================================================//
	// Specular Lighting
	//==========================================================================//

	float3 f3DirectSpecular = 0.0f;
	#if PHONG
		// We prepare a Struct now with default Values
		// This will be Setup using a Function, drastically compressing the Code
		Phong_Data_t PhongData = Phong_Data_FakeConstructor(); // HLSL forced my Hand because it doesn't have constructors

		// Unsaturated
		float f1NdotV = dot(f3NormalWS, -f3ViewDir);
		
		// Phong needs some special fresnel functions to look correctly
		SetupPhongFresnel(PhongData, f1NdotV);
		
		// Now saturate.
		f1NdotV = saturate(f1NdotV);
		
		// Conditional Statement, yay!
		if(g_bHasBaseAlphaPhongMask)
			PhongData.f1AlphaMask = f4BaseTexture.w;
		else
			PhongData.f1AlphaMask = f4NormalTS.w;
		
		#if EXPONENTTEXTURE
			// Get the Pixel from the Texture into our float4.
			PhongData.f4PhongExponentTexture = tex2D(Sampler_PhongExpTexture, f2BaseUV.xy);
		#endif
		
		// Helper Function that sets up all our Phong Data
		SetupPhongData(PhongData, f4BaseTexture.rgb);
		
		// Direct *and* Indirect Specular(?)
		// This is technically 'wrong' (Read: Not physically based).
		// We abuse the Direct Diffuse Color as a Direct Specular Color
		// But it also contains Indirect Diffuse, so this is wrong. Nothing we can do though
		//
		// Reuse Function from lux_common_phong
		// TF2C: use blinn phong.
		#if PROJTEX
			f3DirectSpecular = ComputeProjectedTextureSpecular(PhongData, f3WorldPos, f3NormalWS, reflect(f3ViewDir, f3NormalWS), f3DiffuseLighting);
		#else 
			f3DirectSpecular = ComputeDirectBlinnSpecularLight(f3NormalWS, f3ViewDir, f3LightDir, f3DiffuseLighting, PhongData.f1PhongExponent, PhongData.f1PhongFresnel);
		#endif
		
		if (!g_bHasPhongWarpTexture)
			PhongData.f3PhongModulation *= PhongData.f1PhongFresnel;
		
		// f3PhongModulation is basically the Product of Luminance + $PhongTint + $PhongAlbedoTint + ..
		f3DirectSpecular *= PhongData.f3PhongModulation;
		f3DirectSpecular *= PhongData.f1AlphaMask;
		f3DirectSpecular *= g_f1PhongBoost; // Separate Term
	#endif

	float3 f3IndirectSpecular = 0.0f;
	#if ENVMAP
		#if ENVMAPMASK
			float4 f4EnvMapMask = tex2D(Sampler_EnvMapMask, f2EnvMapMaskUV);
			f3IndirectSpecular = ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4EnvMapMask.rgb);
		#elif LMG_SIMPLE
			f3IndirectSpecular = ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4BaseTexture.a);
		#else
			f3IndirectSpecular = ComputeEnvMap(f3DiffuseLighting, f3WorldPos, f3NormalWS, f3ViewDir, f4BaseTexture.a, f4NormalTS.a);
		#endif
	#endif

	//==========================================================================//
	//	Diffuse Results
	//==========================================================================//

	float3 f3DiffuseTerm = f4BaseTexture.rgb * f3DiffuseLighting;

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
	// Phong not available in Hammer, where Vertex Colors are needed for Texture Shaded Polygons View
	#if (!PROJTEX && !PHONG)
		if(g_bVertexColor)
		{
			f3CombinedTerms *= i.VertexColors.rgb;
		}
	#endif

	return LUX_Finalise(float4(f3CombinedTerms, f1Alpha), f3WorldPos, f1Depth, g_f1AlphaModulation);
}