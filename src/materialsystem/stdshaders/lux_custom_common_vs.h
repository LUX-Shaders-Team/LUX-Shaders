//===================== File of the LUX Shader Project =====================//

#ifndef LUX_CUSTOM_VS_H_
#define LUX_CUSTOM_VS_H_

// Include Base LUX Functions and Register Map
#include "lux_common_vs_fxc.h"

//==========================================================================//
//	These are the Constants that will be available to this Shader
//==========================================================================//

const float4		cReg_15		: register(LUX_VS_FLOAT_SET0_0);
const float4		cReg_48		: register(LUX_VS_FLOAT_SET1_0);
const float4		cReg_49		: register(LUX_VS_FLOAT_SET1_1);
const float4		cReg_50		: register(LUX_VS_FLOAT_SET1_2);
const float4		cReg_51		: register(LUX_VS_FLOAT_SET1_3);
const float4		cReg_52		: register(LUX_VS_FLOAT_SET1_4);
const float4		cReg_53		: register(LUX_VS_FLOAT_SET1_5);
const float4		cReg_54		: register(LUX_VS_FLOAT_SET1_6);
const float4		cReg_55		: register(LUX_VS_FLOAT_SET1_7);
const float4		cReg_56		: register(LUX_VS_FLOAT_SET1_8);
const float4		cReg_57		: register(LUX_VS_FLOAT_SET1_9);

const float4		cTexCoordTransform1[2]		: register(LUX_VS_TEXTURETRANSFORM_01);
const float4		cTexCoordTransform2[2]		: register(LUX_VS_TEXTURETRANSFORM_02);
const float4		cTexCoordTransform3[2]		: register(LUX_VS_TEXTURETRANSFORM_03);
const float4		cTexCoordTransform4[2]		: register(LUX_VS_TEXTURETRANSFORM_04);

// Only with $Shader_Matrices
const float4		cViewportTransform			: register(REGISTER_FLOAT_231);
const float4x4		cView						: register(REGISTER_FLOAT_232);
//					cView[1]
//					cView[2]
//					cView[3]
const float4x4		cProj						: register(REGISTER_FLOAT_235);
//					cProj[1]
//					cProj[2]
//					cProj[3]
const float4x3		cModelView					: register(REGISTER_FLOAT_240);
//					cModelView[1]
//					cModelView[2]

// Only for $Shader_Particle
const float4		cSizeParams						: register(REGISTER_FLOAT_244);
#define MINIMUM_SIZE_FACTOR								(cSizeParams.x)
#define MAXIMUM_SIZE_FACTOR								(cSizeParams.y)
#define START_FADE_SIZE_FACTOR							(cSizeParams.z)
#define END_FADE_SIZE_FACTOR							(cSizeParams.w)

const float4		cSizeParams2					: register(REGISTER_FLOAT_245);
#define START_FAR_FADE									(cSizeParams2.x)
#define FAR_FADE_FACTOR									(cSizeParams2.y)
#define OLDFRM_SCALE_START								(cSizeParams2.z)
#define OLDFRM_SCALE_END								(cSizeParams2.w)

#endif // LUX_CUSTOM_VS_H_