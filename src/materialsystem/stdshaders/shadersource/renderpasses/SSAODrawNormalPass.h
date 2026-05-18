//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	18.05.2026 DMY
//	Last Change :	18.05.2026 DMY
//
//==========================================================================//

#ifndef SSAODRAWNORMALPASS_H
#define SSAODRAWNORMALPASS_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

struct SSAODrawNormalPass_Vars_t
{
	SSAODrawNormalPass_Vars_t() { memset(this, 0xFF, sizeof(SSAODrawNormalPass_Vars_t)); }

	int m_bIsModel;	// ^ int for memset

	int m_nBumpMap;
	int m_nBumpMapFrame;
	int m_nBumpMapTransform;

	// For Models
	int m_nBumpCompress;
	int m_nBumpStretch;

	// For Displacements
	int m_nBumpMap2;
	int m_nBumpMapFrame2;
	int m_nBumpMapTransform2;
	int m_nBlendModulateTexture;
	int m_nBlendModulateFrame;
	int m_nBlendModulateTransform;

	Vars_Base_t BaseVars;
	TreeSway_Vars_t TreeSwayVars;
};

/*
	Use these Functions this Way:

	if(ShouldDrawNormalsForSSAO())
	{
		SetupVars ..
		SSAONormalPass_Shader_Draw( .. .. .. Vars )
		return;
	}
*/
void SSAONormalPass_Shader_Draw(CBaseVSShader* pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, SSAODrawNormalPass_Vars_t& info);

#endif // SSAODRAWNORMALPASS_H