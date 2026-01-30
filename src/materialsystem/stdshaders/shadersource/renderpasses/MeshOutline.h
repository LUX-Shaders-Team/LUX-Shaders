//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.03.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	
//
//==========================================================================//

#ifndef MESHOUTLINE_H
#define MESHOUTLINE_H

#ifdef _WIN32
#pragma once
#endif

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct Outline_Vars_t
{
	Outline_Vars_t() { memset(this, 0xFF, sizeof(Outline_Vars_t)); }

	int nOutlineEnable;
	int nOutlineColor;
	int nOutlineEnableFog;
	int nOutlineDistance;
	int nOutlineCutoffEnable;
	int nOutlineCutoffHeight;
	int nOutlineEnableAlphaWrites;
	int nOutlineAdditive;
	int nOutlineDepthWrite_Enable;
	int nOutlineDepthWrite_Bias;
	int nOutlineCullPower;
	int nOutlineCullFactor;

	// Instead of a Macro, just copy this.
	/*
	InitVars(MeshOutline_Enable, MeshOutline_Color, MeshOutline_Enable_Fog, MeshOutline_Distance,
			MeshOutline_Enable_Cutoff, MeshOutline_CutoffHeight, MeshOutline_EnableAlphaWrites);
	*/
	void InitVars(int Enabled)
	{
		nOutlineEnable = Enabled;
		nOutlineColor = Enabled + 1;
		nOutlineEnableFog = Enabled + 2;
		nOutlineDistance = Enabled + 3;
		nOutlineCutoffEnable = Enabled + 4;
		nOutlineCutoffHeight = Enabled + 5;
		nOutlineEnableAlphaWrites = Enabled + 6;
		nOutlineAdditive = Enabled + 7;
		nOutlineDepthWrite_Enable = Enabled + 8;
		nOutlineDepthWrite_Bias = Enabled + 9;
		nOutlineCullPower = Enabled + 10;
		nOutlineCullFactor = Enabled + 11;
	}
};

// FIXME: FIXME: Move the Parameter Macro to cpp_lux_shared.h
#define Declare_MeshOutlineParameters()\
SHADER_PARAM(MeshOutline_Enable,			SHADER_PARAM_TYPE_BOOL,  "", "Enables an Unlit Mesh Outline.")\
SHADER_PARAM(MeshOutline_Enable_Fog,		SHADER_PARAM_TYPE_BOOL,  "", "Allow the Outline to be affected by Fog.")\
SHADER_PARAM(MeshOutline_Enable_Cutoff,		SHADER_PARAM_TYPE_BOOL,  "", "Enables the Cutoff of the Outline at $MeshOutline_CutoffHeight.\nThis is better for Static Props..")\
SHADER_PARAM(MeshOutline_Distance,			SHADER_PARAM_TYPE_FLOAT, "", "The Size of the Outline. ( Offset from the Original Surface )")\
SHADER_PARAM(MeshOutline_CutoffHeight,		SHADER_PARAM_TYPE_FLOAT, "", "Anything below this Value will not be rendered when $MeshOutline_Enable_Cutoff is set.")\
SHADER_PARAM(MeshOutline_Color,				SHADER_PARAM_TYPE_COLOR, "", "Color of the Outline.")\
SHADER_PARAM(MeshOutline_EnableAlphaWrites, SHADER_PARAM_TYPE_BOOL,  "", "Enables DepthToDestAlpha for the Outline, makes it so Particles can blend with it.")\
SHADER_PARAM(MeshOutline_Additive,			SHADER_PARAM_TYPE_BOOL,	 "", "Renders the Outline additively instead of opaque.")\
SHADER_PARAM(MeshOutline_EnableDepthWrites, SHADER_PARAM_TYPE_BOOL,  "", "Allow DepthWrites. The Outline will occlude all Geometry rendered behind it.")\
SHADER_PARAM(MeshOutline_DepthBias,			SHADER_PARAM_TYPE_BOOL,	 "", "Biases Depth Values away from the Camera. Causing the main Geometry to not be occluded by it's own Outline.")\
SHADER_PARAM(MeshOutline_CullPower,			SHADER_PARAM_TYPE_FLOAT, "", "NdotL^CullPower. Bright Pixels will get discarded.")\
SHADER_PARAM(MeshOutline_CullFactor,		SHADER_PARAM_TYPE_FLOAT, "", "When NdotL is greated or equal to this Value, the Outline will get Culled.")

void LuxOutlinePass_Draw(CBaseVSShader* pShader, IMaterialVar** params, IShaderDynamicAPI* pShaderAPI,
	IShaderShadow* pShaderShadow, Outline_Vars_t& info);

#endif // MESHOUTLINE_H
