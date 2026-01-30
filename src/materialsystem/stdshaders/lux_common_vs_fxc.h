//
//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_VS_FXC_H_
#define LUX_COMMON_VS_FXC_H_

// For disabling Warnings
#include "lux_common_pragmas.h"

// Feature List
#include "lux_common_defines.h"

// Register Map
#include "lux_registermap_hlsl.h"
#include "lux_registermap_vs.h"

// Constants and Functions that can be shared between VS and PS
#include "lux_common_fxc.h"

//==========================================================================//
//	Declaring #define's and custom errors/error ignores ( pragma )
//==========================================================================//

// Because 1/0 is already a compile Error.
// And you can't define new Errors like this either
// Stock-Consistency
#define COMPILE_ERROR ( 1/0; )

// Is this still required?
// Stock-Consistency.
// the def pragma allocates this register with the specified values
#pragma def ( vs, c0, 0.0f, 1.0f, 2.0f, 0.5f )

// Vertex Specific Light Info
struct LightInfo
{
	float4 color;	// w is light type code
	float4 dir;		// w is light type code
	float4 pos;
	float4 spotParams;
	float4 atten;
};

//==========================================================================//
//	Declaring BOOLEAN VertexShader Constant Registers
//==========================================================================//

// 'The g_bLightEnabled registers and g_nLightCountRegister hold the same information regarding
// enabling lights, but callers internal to this file tend to use the loops, while external
// callers will end up using the booleans'
const bool			cLightsEnabled[4]				: register(b0); // b0 - b3
//const bool		cLightsEnabled[1]				: register(b1);
//const bool		cLightsEnabled[2]				: register(b2);
//const bool		cLightsEnabled[3]				: register(b3);
const bool			g_bHalfLambert					: register(LUX_VS_BOOL_HALFLAMBERT);
#define				SHADER_SPECIFIC_BOOL_CONST_1			   b5
#define				SHADER_SPECIFIC_BOOL_CONST_2			   b6
#define				SHADER_SPECIFIC_BOOL_CONST_3			   b7
#define				SHADER_SPECIFIC_BOOL_CONST_4			   b8
#define				SHADER_SPECIFIC_BOOL_CONST_5			   b9
#define				SHADER_SPECIFIC_BOOL_CONST_6			   b10
#define				SHADER_SPECIFIC_BOOL_CONST_7			   b11
#define				SHADER_SPECIFIC_BOOL_CONST_8			   b12 // This and below are new.
#define				SHADER_SPECIFIC_BOOL_CONST_9			   b13
#define				SHADER_SPECIFIC_BOOL_CONST_10			   b14
#define				SHADER_SPECIFIC_BOOL_CONST_11			   b15


//==========================================================================//
//	Declaring INTEGER VertexShader Constant Registers
//==========================================================================//

const int			cLightCountRegister			: register(i0);
#define				g_nLightCount					cLightCountRegister.x

//==========================================================================//
//	Declaring FLOAT VertexShader Constant Registers
//==========================================================================//

//					Default Constants Pragma Above	: register(c0);
const float4		cConstants1						: register(c1);
const float4		cEyePosWaterZ					: register(c2);
const float4		cFlexScale						: register(c3); // Only cFlexScale.x used. Binary value for toggling addition of the flex delta stream.
const float4x4		cModelViewProj					: register(c4);
//					cModelViewProj[1]				: register(c5);
//					cModelViewProj[2]				: register(c6);
//					cModelViewProj[3]				: register(c7);
const float4x4		cViewProj						: register(c8);
//					cViewProj[1]					: register(c9);
//					cViewProj[2]					: register(c10); // 10
//					cViewProj[3]					: register(c11);
const float4		cModelViewProjZ					: register(c12); // "Used to compute projPosZ without skinning. Using cModelViewProj with FastClip generates incorrect results"
const float4		cViewProjZ						: register(c13); // "More constants working back from the top..."
const float4		cHPOFixValues					: register(LUX_VS_FLOAT_HPOFIX); // Viewport Data for fixing the D3D9 Half Pixel Offset Issue
//					LUX_VS_FLOAT_SET0_0
const float4		cFogParams						: register(c16);
const float4x4		cViewModel						: register(c17);
//					cViewModel[1]					: register(c18);
//					cViewModel[2]					: register(c19);
//					cViewModel[3]					: register(c20); // 20
const float3		cAmbientCubeX[2]				: register(c21);
//					cAmbientCubeX[1]				: register(c22);
const float3		cAmbientCubeY[2]				: register(c23);
//					cAmbientCubeY[1]				: register(c24);
const float3		cAmbientCubeZ[2]				: register(c25);
//					cAmbientCubeZ[1]				: register(c26);
LightInfo			cLightInfo[4]					: register(c27);
//					cLightInfo[0].dir				: register(c28);
//					cLightInfo[0].pos				: register(c29);
//					cLightInfo[0].spotParams		: register(c30); // 30
//					cLightInfo[0].atten				: register(c31);
//					cLightInfo[1].color				: register(c32);
//					cLightInfo[1].dir				: register(c33);
//					cLightInfo[1].pos				: register(c34);
//					cLightInfo[1].spotParams		: register(c35);
//					cLightInfo[1].atten				: register(c36);
//					cLightInfo[2].color				: register(c37);
//					cLightInfo[2].dir				: register(c38);
//					cLightInfo[2].pos				: register(c39);
//					cLightInfo[2].spotParams		: register(c40); // 40
//					cLightInfo[2].atten				: register(c41);
//					cLightInfo[3].color				: register(c42);
//					cLightInfo[3].dir				: register(c43);
//					cLightInfo[3].pos				: register(c44);
//					cLightInfo[3].spotParams		: register(c45);
//					cLightInfo[3].atten				: register(c46);
const float4		cModulationColor				: register(c47);
//					LUX_VS_FLOAT_SET1_0
//					LUX_VS_FLOAT_SET1_1
//					LUX_VS_FLOAT_SET1_2								  // 50
//					LUX_VS_FLOAT_SET1_3
//					LUX_VS_FLOAT_SET1_4
//					LUX_VS_FLOAT_SET1_5
//					LUX_VS_FLOAT_SET1_6
//					LUX_VS_FLOAT_SET1_7
//					LUX_VS_FLOAT_SET1_8
//					LUX_VS_FLOAT_SET1_9

// c58 to c213
const float4x3		cModel[53]						: register(c58);

//				  Used by Something 				: register(c214);
//				  Used by Something 				: register(c215);
//				  Used by Something 				: register(c216);
//						EMPTY?						: register(c217);
//						EMPTY?						: register(c218);
//					ToGL ClipPlane 0				: register(c219);
//					ToGL ClipPlane 1				: register(c220); // 220
//						EMPTY?						: register(c221);
//						EMPTY?						: register(c222);

// 223 and up don't seem to impact anything. These are probably free.
// c223 to c240
//					LUX_VS_TEXTURETRANSFORM_01	c223
//
//					LUX_VS_TEXTURETRANSFORM_02	c225
//
//					LUX_VS_TEXTURETRANSFORM_03	c227
//
//					LUX_VS_TEXTURETRANSFORM_04	c229
//
//					LUX_VS_TEXTURETRANSFORM_05	c231
//
//					LUX_VS_TEXTURETRANSFORM_06	c233
//
//					LUX_VS_TEXTURETRANSFORM_07	c235
//
//					LUX_VS_TEXTURETRANSFORM_08	c237
//
//					LUX_VS_TEXTURETRANSFORM_09	c239
//
//					LUX_VS_TEXTURETRANSFORM_10	c241
//
//					LUX_VS_TEXTURETRANSFORM_11	c243
//
//					LUX_VS_TEXTURETRANSFORM_12	c245
//
//					LUX_VS_TEXTURETRANSFORM_13	c247
//
//					LUX_VS_TEXTURETRANSFORM_14	c249
//

// Note that according to mat_info only 256 VS Registers are supported
// Meaning the last usable Register is c254
#if defined(MODEL)
// c1024 to c1534
const float4		cFlexWeights[512]				: register(c1024);
#endif

// c1
#define g_f1OOGamma			(cConstants1.x)
#define g_f1Overbright		2.0f
#define g_f1OneThird		(cConstants1.z)
#define g_f1OOOverbright	(1.0f / g_f1Overbright)

// c2
#define g_f3EyePos			(cEyePosWaterZ.xyz)

// c16
#define g_f1FogEndOverFogRange		(cFogParams.x)
#define g_f1FogOne					(cFogParams.y)
#define g_f1FogMaxDensity			(cFogParams.z)
#define g_f1OOFogRange				(cFogParams.w)

// c58 not used anywhere
static const int g_nModel0Index = 58;

//==========================================================================//
// Functions
//==========================================================================//

float4 FixHPO(float4 vProjPos)
{
	// See also baseshader.cpp for why this is commented
	/*
	// Fixes the D3D9 Half-Pixel Offset Issue ( HPO )
	// Using: https://aras-p.info/blog/2016/04/08/solving-dx9-half-pixel-offset/
	vProjPos.xy += cHPOFixValues.xy * vProjPos.ww;
	*/
	return vProjPos;
}

//==========================================================================//
//	Needed for SkinPositionAndNormal on both models and brushes.
//	NOTE:	Due to how matrix multiplication works, m has to be transposed
//			Look up matrix multiplication for further details.
//==========================================================================//
float3 mul4x3(float4 v, float4x3 m)
{
	return float3(dot(v, transpose(m)[0]), dot(v, transpose(m)[1]), dot(v, transpose(m)[2]));
}

// Original Comment for both mul4x3 and mul3x3 in common_fxc.h
// "Versions of matrix multiply functions which force HLSL compiler to explictly use DOTs, 
// not giving it the option of using MAD expansion.  In a perfect world, the compiler would
// always pick the best strategy, and these shouldn't be needed.. but.. well.. umm..
//
// lorenmcq"

//==========================================================================//
//	Needed for SkinPositionAndNormal on both models and brushes.
//==========================================================================//
float3 mul3x3(float3 v, float3x3 m)
{
	return float3(dot(v, transpose(m)[0]), dot(v, transpose(m)[1]), dot(v, transpose(m)[2]));
}

#endif //#ifndef LUX_COMMON_VS_FXC_H_
