//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	11.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	
//
//==========================================================================//

#ifndef SHEENPASS_H
#define SHEENPASS_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct SheenPass_Vars_t
{
	SheenPass_Vars_t() { memset(this, 0xFF, sizeof(SheenPass_Vars_t)); }

	int m_nSheenPassEnabled;
	int m_nSheenMap;
	int m_nSheenMapMask;
	int m_nSheenMapMaskFrame;
	int m_nSheenMapTint;
	int m_nSheenMapMaskOffsetX;
	int m_nSheenMapMaskOffsetY;
	int m_nSheenMapMaskScaleX;
	int m_nSheenMapMaskScaleY;
	int m_nSheenMapMaskDirection;
	int m_nSheenIndex;

	int m_nBumpMap;
	int m_nBumpFrame;
	int m_nBumpTransform;

	// Instead of a Macro, just copy this.
	/*
		InitVars(SheenPassEnabled, BumpMap, BumpFrame, BumpTransform);
	*/
	void InitVars(int SheenPassEnabled, int BumpMap, int BumpFrame, int BumpTransform)
	{
		m_nSheenPassEnabled = SheenPassEnabled;
		m_nSheenMap = SheenPassEnabled;
		m_nSheenMapMask = SheenPassEnabled + 1;
		m_nSheenMapMaskFrame = SheenPassEnabled + 2;
		m_nSheenMapTint = SheenPassEnabled + 3;
		m_nSheenMapMaskOffsetX = SheenPassEnabled + 4;
		m_nSheenMapMaskOffsetY = SheenPassEnabled + 5;
		m_nSheenMapMaskScaleX = SheenPassEnabled + 6;
		m_nSheenMapMaskScaleY = SheenPassEnabled + 7;
		m_nSheenMapMaskDirection = SheenPassEnabled + 8;
		m_nSheenIndex = SheenPassEnabled + 9;

		m_nBumpMap = BumpMap;
		m_nBumpFrame = BumpFrame;
		m_nBumpTransform = BumpTransform;
	}
};

void SheenPass_Init_Params(CBaseVSShader* pShader, SheenPass_Vars_t& info);
void SheenPass_Shader_Init(CBaseVSShader* pShader, SheenPass_Vars_t& info);
void SheenPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, SheenPass_Vars_t& info);

#endif // SHEENPASS_H