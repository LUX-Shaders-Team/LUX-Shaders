//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	13.05.2026 DMY
//	Last Change :	13.05.2026 DMY
//
//==========================================================================//

#ifndef RIMLIGHTPASS_H
#define RIMLIGHTPASS_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct RimLightPass_Vars_t
{
	RimLightPass_Vars_t() { memset(this, 0xFF, sizeof(RimLightPass_Vars_t)); }

	Vars_Base_t Base;
	Vars_NormalMap_t Normal;

	int m_nEnabled;
	int m_nUseBumpMap;
	int m_nTint;
	int m_nMask;
	int m_nMaskFrame;
	int m_nFresnelRanges;

	// Path A
	int m_nUseCustomAmbientCube;
	int m_nAmbientCubeXP;
	int m_nAmbientCubeXN;
	int m_nAmbientCubeYP;
	int m_nAmbientCubeYN;
	int m_nAmbientCubeZP;
	int m_nAmbientCubeZN;

	// Path B
	int m_nCubemap;
	int m_nCubemapFrame;

	// Instead of a Macro, just copy this.
	/*
		InitVars();
	*/
	void InitVars(int Enabled)
	{
		m_nEnabled				= Enabled + 0;
		m_nUseBumpMap			= Enabled + 1;
		m_nTint					= Enabled + 2;
		m_nMask					= Enabled + 3;
		m_nMaskFrame			= Enabled + 4;
		m_nFresnelRanges		= Enabled + 5;
		m_nUseCustomAmbientCube = Enabled + 6;
		m_nAmbientCubeXP		= Enabled + 7;
		m_nAmbientCubeXN		= Enabled + 8;
		m_nAmbientCubeYP		= Enabled + 9;
		m_nAmbientCubeYN		= Enabled + 10;
		m_nAmbientCubeZP		= Enabled + 11;
		m_nAmbientCubeZN		= Enabled + 12;
		m_nCubemap				= Enabled + 13;
		m_nCubemapFrame			= Enabled + 14;
	}
};

void RimLightPass_Init_Params(CBaseVSShader* pShader, RimLightPass_Vars_t& info);
void RimLightPass_Shader_Init(CBaseVSShader* pShader, RimLightPass_Vars_t& info);
void RimLightPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, RimLightPass_Vars_t& info);

#endif // RIMLIGHTPASS_H