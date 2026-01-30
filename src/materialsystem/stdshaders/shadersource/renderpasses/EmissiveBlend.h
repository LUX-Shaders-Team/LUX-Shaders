//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.03.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	
//
//==========================================================================//

#ifndef EMISSIVEBLEND_H
#define EMISSIVEBLEND_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct EmissiveBlend_Vars_t
{
	EmissiveBlend_Vars_t() { memset(this, 0xFF, sizeof(EmissiveBlend_Vars_t)); }

	Vars_Base_t Base;
	Vars_Detail_t Detail;
	Vars_SelfIllumTexture_t SelfIllum;

	int m_nEmissiveBlendEnabled;
	int m_nEmissiveBlendBaseTexture;
	int m_nEmissiveBlendTransform;
	int m_nEmissiveBlendScrollVector;
	int m_nEmissiveBlendStrength;
	int m_nEmissiveBlendTexture;
	int m_nEmissiveBlendTint;
	int m_nEmissiveBlendFlowTexture;
	int m_nEmissiveBlend_NoFlowTransform;
	int m_nTime;
	int m_nMinimumLightAdditivePass;
	int m_nMinimumLightTint;

	// Instead of a Macro, just copy this.
	/*
		InitVars(EmissiveBlendEnabled);
	*/
	void InitVars(int Enabled)
	{
		m_nEmissiveBlendEnabled = Enabled;
		m_nEmissiveBlendBaseTexture = Enabled + 1;
		m_nEmissiveBlendTransform = Enabled + 2;
		m_nEmissiveBlendScrollVector = Enabled + 3;
		m_nEmissiveBlendStrength = Enabled + 4;
		m_nEmissiveBlendTexture = Enabled + 5;
		m_nEmissiveBlendTint = Enabled + 6;
		m_nEmissiveBlendFlowTexture = Enabled + 7;
		m_nEmissiveBlend_NoFlowTransform = Enabled + 8;
		m_nTime = Enabled + 9;
		m_nMinimumLightAdditivePass = Enabled + 10;
		m_nMinimumLightTint  = Enabled + 11;
	}
};

void EmissiveBlend_Init_Params(CBaseVSShader* pShader, EmissiveBlend_Vars_t& info);
void EmissiveBlend_Shader_Init(CBaseVSShader* pShader, EmissiveBlend_Vars_t& info);
void EmissiveBlend_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, EmissiveBlend_Vars_t& info);

#endif // EMISSIVEBLEND_H