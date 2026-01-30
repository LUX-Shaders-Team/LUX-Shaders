//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	19.12.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_DETAILTEXTURE_H_
#define LUX_COMMON_DETAILTEXTURE_H_

// by Defining this, it allows Shaders to move the Registers to something else
#if !defined(MOVED_REGISTERS_DETAIL)
	const float4	cDetailTint_BlendFactor		: register(LUX_PS_FLOAT_DETAIL_FACTORS);
	#define			g_f3DetailTextureTint		(cDetailTint_BlendFactor.rgb)
	#define			g_f1DetailBlendFactor		(cDetailTint_BlendFactor.w)
	
	const float4	cDetailFactors					: register(LUX_PS_FLOAT_DETAIL_BLENDMODE); // zyw empty
	#define			g_f1DetailBlendMode				(cDetailFactors.x)
#endif

//==========================================================================//
//	Declaring Samplers. We only have 16 on SM3.0. Ranging from 0-15
//	Although s15 wants to be a gamma-lookup table, limiting us to 15/16
//==========================================================================//
#if !defined(MOVED_SAMPLERS_DETAIL)
sampler Sampler_DetailTexture : register(s4);
#endif

// #define NEW_DETAILTEXTURES_COMBINEMODE
#if !defined(NEW_DETAILTEXTURES_COMBINEMODE)

/*	IMPORTANT:

The Order in which these are used is NOT 0-11 but actually		0, 7, 1, 4, 8, 2, 9, 3, 11, - 5,6 for p-L
Stock Shaders order them as										7, 0, 1, 2, 3, 4, 8, 9, 11, - 5,6 for p-L

Theoratically the Order of these doesn't actually matter but to decrease Compiletimes ( quite drastically I might add ),
LUX uses a float Register for the Blendmode Integer instead of a Static Combo like Stock Shaders.
The hopes now are that Drivers can optimise unused Branches somehow later ( using Magic )

Mr.Kleiner has VMT's from various Source Games for LUX so we can do some statistical Analysis on this. Games Searched:
Team Fortress 2, Portal 2, Left 4 Dead 2, Half Life 2, Episode 1, Episode 2, Alien Swarm, Counter-Strike: Global Offensive AND Operation Black Mesa 
( TF2 alone has thousands of VMT's so took quite some time. Thank you Kleiner )

By using advanced statistical Methods ( ordering them into neat Tables ), 
we determined that '0, 7, 1, 4, 8, 10, 2, 9, 3, 11 ; 5,6' is the highest probability for the SDK ( ~2023 )

NOTE: TF2 uses Blendmode 6 the most, 4466 VMT's. It's used for the burning Effect and there happen to be a looot of Cosmetic Items with it.

Our Results (23.02.2023) :

Some Blendmodes don't work in some Games ( marked as XXXXX ).
	NOTE1: This was determined by loading custom Materials and seeing whether they would Function as expected.
	NOTE2: SDK, TF2 and Alien Swarm have public Source Code from which this could be derived instead.
	NOTE3: There is some inaccuracy in these Results as Mistakes are Human. But the Search Results back our Method pretty well.
	With one Exception. CS:GO has a Result for Blendmode 9 which when tested did NOT work. Maybe an earlier Version of the Shader had it working?
	We did not check if this Material was used anywhere so that's also a possibility.
	NOTE4: WVT traditionally uses the same HLSL Code as LMG, on DX9.
	It's still listed separately here identical to LMG but has been 

sdk13,tf2	- LMG mode 2-11		 not supported.
CS:GO		- LMG mode 1-6 and 8 not supported.
asw			- LMG mode 6		 not supported. 7 not with $SoftEdges
sdk13,tf2	- WVT mode 2-9		 not supported.
CS:GO		- WVT mode 1-6, 8-9	 not supported.
asw			- WVT mode 6		 not supported. 7 not with $BlendModulateTexture, or $BumpMap2
sdk13,tf2	- VLG mode 10-11	 not supported. 5-9 not with $BumpMap, 5-6 only with $Phong
CS:GO		- VLG mode 10-11	 not supported. 5-6, 8-9 not with $BumpMap ( probably inherted from earlier Branches )
asw			- VLG mode 10-11	 not supported. 5-9 not with $BumpMap, 5-6 only with $Phong

LightmappedGeneric																	VertexLitGeneric
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
| Mode |  hl2  |  ep1  |  ep2  |  asw  |  bms  | csgo  | l4d2  |  p2   |  tf2  |	| Mode |  hl2  |  ep1  |  ep2  |  asw  |  bms  | csgo  | l4d2  |  p2   |  tf2  |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  0   |  943  |   2   |  96   |  67   |  897  |  164  |  22   |  52   |  308  |	|  0   |  42   |   3   |  27   |   0   |  566  |  161  |  41   |  15   |  177  |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  1   |   0   |   3   |   0   |   0   |   3   | XXXXX |   0   |   8   |   1   |	|  1   |   0   |   0   |   0   |  18   |   2   |   0   |   0   |   0   |  33   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  2   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  2   |   1   |   0   |   0   |   0   |   2   |   1   |   2   |  20   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  3   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  3   |   0   |   0   |   1   |   0   |   0   |   0   |   0   |   0   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  4   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  4   |   0   |   0   |   0   |   0   |   1   |  18   |   0   |   0   |   5   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  5   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   1   | XXXXX |	|  5   |   0   |   0   |   0   |  35   |   0   |   1   |   1   |   4   | 4466  | <--Bruh
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  6   | XXXXX | XXXXX | XXXXX | XXXXX |   0   |   0   |   0   |   0   | XXXXX |	|  6   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  7   | XXXXX | XXXXX | XXXXX |   2   |   0   |   4   |   1   |  28   | XXXXX |	|  7   |   0   |   0   |   0   |   0   |   4   |   0   |   1   |   9   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  8   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  8   |   0   |   0   |   0   |   0   |  16   |   2   |   0   |   0   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  9   | XXXXX | XXXXX | XXXXX |   0   |   0   |   1?  |   0   |   0   | XXXXX |	|  9   |   0   |   0   |   0   |   0   |   0   |   3   |   0   |   0   |   0   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
| 10   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |  14   | XXXXX |	| 10   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
| 11   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |	| 11   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+

WorldVertexTransition																Lightmapped_4WayBlend	We don't have this Shader on sdk13/asw so we didn't log it
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
| Mode |  hl2  |  ep1  |  ep2  |  asw  |  bms  | csgo  | l4d2  |  p2   |  tf2  |	| Mode | csgo  |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  0   |   0   |   0   |   1   |   1   |   0   |   4   |   0   |   0   |   4   |	|  0   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  1   |   0   |   0   |   0   |   0   |   0   | XXXXX |   0   |   0   |   0   |	|  1   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  2   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  2   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  3   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  3   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  4   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  4   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  5   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  5   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  6   | XXXXX | XXXXX | XXXXX |   0   |   0   |   0   |   0   |   0   | XXXXX |	|  6   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  7   | XXXXX | XXXXX | XXXXX |   0   |   0   |   0   |   0   |   0   | XXXXX |	|  7   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  8   | XXXXX | XXXXX | XXXXX |   0   |   0   | XXXXX |   0   |   0   | XXXXX |	|  8   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
|  9   | XXXXX | XXXXX | XXXXX |   0   |   0   |   0   |   0   |   0   | XXXXX |	|  9   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
| 10   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |	| 10   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
| 11   | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX | XXXXX |	| 11   |   ?   |
+------+-------+-------+-------+-------+-------+-------+-------+-------+-------+	+------+-------+
*/

// NOTE: These are different Macros than those from the Definitions LUX has in C++ ( see also cpp_lux_shared.h )
/*
#define TCOMBINE_RGB_EQUALS_BASE_x_DETAILx2 0				// original mode
#define TCOMBINE_RGB_ADDITIVE 1								// base.rgb+detail.rgb*fblend
#define TCOMBINE_DETAIL_OVER_BASE 2
#define TCOMBINE_FADE 3										// straight fade between base and detail.
#define TCOMBINE_BASE_OVER_DETAIL 4                         // use base alpha for blend over detail
#define TCOMBINE_RGB_ADDITIVE_SELFILLUM 5                   // add detail color post lighting
#define TCOMBINE_RGB_ADDITIVE_SELFILLUM_THRESHOLD_FADE 6
#define TCOMBINE_MOD2X_SELECT_TWO_PATTERNS 7				// use alpha channel of base to select between mod2x channels in r+a of detail
#define TCOMBINE_MULTIPLY 8
#define TCOMBINE_MASK_BASE_BY_DETAIL_ALPHA 9                // use alpha channel of detail to mask base
#define TCOMBINE_SSBUMP_BUMP 10								// use detail to modulate lighting as an ssbump
#define TCOMBINE_SSBUMP_NOBUMP 11							// detail is an ssbump but use it as an albedo. shader does the magic here - no user needs to specify mode 11
*/

//==========================================================================//
// Blendmodes first that way you can do whatever order you want easily later
//==========================================================================//

// Originally: TCOMBINE_RGB_EQUALS_BASE_x_DETAILx2
float4 TCombine_0(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// This blendmode is known as mod2x because of the 2*
	// I precomputed the *2 into the detailtint. -1 MUL Instruction
	// Alpha unmodified
	return float4(f3Base * lerp(float3(1.0f, 1.0f, 1.0f), f3Detail, g_f1DetailBlendFactor), f1BaseAlpha);
}

// Originally: TCOMBINE_RGB_ADDITIVE
float4 TCombine_1(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// NOTE : Blendfactor precomputed into DetailTint. -1 MUL Instruction
	// Alpha unmodified
	return float4(f3Base + f3Detail, f1BaseAlpha);
}

// Originally: TCOMBINE_DETAIL_OVER_BASE
float4 TCombine_2(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// Alpha unmodified
	float f1Blend = g_f1DetailBlendFactor * f1DetailAlpha;
	return float4(lerp(f3Base, f3Detail, f1Blend), f1BaseAlpha);
}

// Originally: TCOMBINE_FADE
float4 TCombine_3(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// NOTE : This is just TCOMBINE_DETAIL_OVER_BASE but with Alpha interpolated instead of multiplied
	// Alpha modified
	float4 Base = float4(f3Base, f1BaseAlpha);
	float4 Detail = float4(f3Detail, f1DetailAlpha);
	return lerp(Base, Detail, g_f1DetailBlendFactor);
}

// Originally: TCOMBINE_BASE_OVER_DETAIL
float4 TCombine_4(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	float f1Blend = g_f1DetailBlendFactor * (1.0f - f1BaseAlpha);

	// NOTE :	This is just TCOMBINE_DETAIL_OVER_BASE but with alpha
	// Alpha MODIFIED
	return float4(lerp(f3Base, f3Detail, f1Blend), f1DetailAlpha);
}

// Originally: TCOMBINE_RGB_ADDITIVE_SELFILLUM
float3 TCombine_5(float3 f3Base, float3 f3Detail)
{
	// NOTE :	This is literally just TCOMBINE_RGB_ADDITIVE now that blendfactor is in the tint
	// Alpha unmodified
	return f3Base + f3Detail;
}

// Originally: TCOMBINE_RGB_ADDITIVE_SELFILLUM_THRESHOLD_FADE
float3 TCombine_6(float3 f3Base, float3 f3Detail)
{
	// "fade in an unusual way - instead of fading out color, remap an increasing band of it from 0..1"
	// Traditionally :
	/*
	float f = fBlendFactor - 0.5;
	float fMult = (f >= 0) ? 1.0 / fBlendFactor : 4 * fBlendFactor;
	float fAdd = (f >= 0) ? 1.0 - fMult : -0.5*fMult;
	*/

	// We know f1DetailBlendFactor in the C++ portion so we can precompute this.
	// no ADD for -0.5
	// no rcp/mul for fMult
	// no ADD for 1.0f - fMult, no MUL for -0.5f * fMult
	// no Ternary
	// We also precompute fMult into $DetailTint and reuse $DetailBlendFactor for the fAdd
	//
	// Now	ADD, ADD. Instead of
	//		ADD, RCP|MUL, ADD|MUL, MAD, ADD
	return f3Base + saturate(f3Detail + g_f1DetailBlendFactor.xxx);
}

// Originally: TCOMBINE_MOD2X_SELECT_TWO_PATTERNS
float4 TCombine_7(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// This is probably my least favorite Blendmode..
	// Why bother with the alpha, you've got g & b ???
	// This is overall just TCOMBINE_RGB_EQUALS_BASE_x_DETAILx2 so I sent it there
	// Alpha unmodified
	float3 f3DetailMask = 2.0f * lerp(f3Detail.rrr, f1DetailAlpha, f1BaseAlpha);
	return TCombine_0(f3Base, f3DetailMask, f1BaseAlpha, f1DetailAlpha);
}

// Originally: TCOMBINE_MULTIPLY
float4 TCombine_8(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// Nothing that can be optimised here.
	// Alpha modified
	float4 Base = float4(f3Base, f1BaseAlpha);
	float4 Detail = float4(f3Detail, f1DetailAlpha);
	return lerp(Base, Base * Detail, g_f1DetailBlendFactor);
}

// Originally: TCOMBINE_MASK_BASE_BY_DETAIL_ALPHA
float4 TCombine_9(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// Nothing that can be optimised here.
	// Base unmodified
	return float4(f3Base, lerp(f1BaseAlpha, f1BaseAlpha * f1DetailAlpha, g_f1DetailBlendFactor));
}

// This doesn't exist here, its handled in lux_common_lightmapped as it modulates the dp
// Originally: TCOMBINE_SSBUMP_BUMP
/*
float4 TCombine_10(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// The 2.0f is precomputed into $DetailTint
	return float4(f3Base * 2.0f * f3Detail, 0.0f);
}
*/

// Originally: TCOMBINE_SSBUMP_NOBUMP
float4 TCombine_11(float3 f3Base, float3 f3Detail, float f1BaseAlpha, float f1DetailAlpha)
{
	// Alpha unmodified
	return float4(f3Base * dot(f3Detail, (float3)2.0f / 3.0f), f1BaseAlpha);
}

//==========================================================================//
// Combine Textures
//==========================================================================//
float4 TextureCombine(float4 f4BaseTexture, float4 f4DetailTexture, const int nBlendMode)
{
	// These blendmodes aren't handled here
	if (nBlendMode == 10 || nBlendMode == 5 || nBlendMode == 6)
		return f4BaseTexture;

	// Current order is 0, 7, 1, 4, 8, 2, 9, 3, 11
	// SDK optimised. Use Table above to make it fitting for your mod
	// Simply change the numbers in the if and Function-Name. This is the best way I could do this for ease of changing the order

	if (nBlendMode == 0)  return  TCombine_0(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a);
	if (nBlendMode == 7)  return  TCombine_7(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 1)  return  TCombine_1(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 4)  return  TCombine_4(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 8)  return  TCombine_8(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 2)  return  TCombine_2(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 9)  return  TCombine_9(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 3)  return  TCombine_3(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else
	if (nBlendMode == 11) return TCombine_11(f4BaseTexture.rgb, f4DetailTexture.rgb, f4BaseTexture.a, f4DetailTexture.a); else

	// Always need a return path
	return f4BaseTexture;
}

float3 TextureCombinePostLighting(float3 f3BaseTexture, float3 f3DetailTexture, const int nBlendMode)
{
	if (nBlendMode == 5) return  TCombine_5(f3BaseTexture.rgb, f3DetailTexture.rgb); else
	if (nBlendMode == 6) return  TCombine_6(f3BaseTexture.rgb, f3DetailTexture.rgb); else

	// Always need a return path
	return f3BaseTexture;
}

#else // NEW detailtexture combine modes

// Ideas : combine mod2x and multiply
// set detailtint to 1.0 then force blendfactor of 1.0
// same result, one less actual blendmode

// NOTE 11.01.2026: Don't have the Time for this atm. ( Release Crunch )
// If someone wants to handle this themselves:
// The Idea, combine all Blendmodes into one unified math formula.
// Remove the need for branching *and* reduce total Instruction Count by just being more smart about this.
// Needing more constant Registers to Control it would be acceptable, we have a lot of them left

#endif // END of NEW detailtexture combine modes

#endif // End of LUX_COMMON_DETAILTEXTURE_H_