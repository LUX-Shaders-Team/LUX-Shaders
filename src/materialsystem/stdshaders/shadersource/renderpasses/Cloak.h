//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.03.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	
//
//==========================================================================//

#ifndef CLOAK_H
#define CLOAK_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct Cloak_Vars_t
{
	Cloak_Vars_t() { memset(this, 0xFF, sizeof(Cloak_Vars_t)); }

	Vars_Base_t Base;
	Vars_NormalMap_t Bump;

	int m_nCloakEnabled;
	int m_nCloakFactor;
	int m_nCloakColorTint;
	int m_nRefractAmount;

	// Instead of a Macro, just copy this.
	/*
		InitVars(CloakPassEnabled, CloakFactor, CloakColorTint, RefractAmount);
	*/
	void InitVars(int Enabled, int Factor, int Tint, int RefractAmount)
	{
		m_nCloakEnabled = Enabled;
		m_nCloakFactor = Factor;
		m_nCloakColorTint = Tint;
		m_nRefractAmount = RefractAmount;
	}
};

bool CloakBlend_IsOpaque(CBaseVSShader* pShader, IMaterialVar** params, Cloak_Vars_t& info);

void CloakBlend_Init_Params(CBaseVSShader* pShader, Cloak_Vars_t& info);
void CloakBlend_Shader_Init(CBaseVSShader* pShader, Cloak_Vars_t& info);
void CloakBlend_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, Cloak_Vars_t& info);

#endif // CLOAK_H