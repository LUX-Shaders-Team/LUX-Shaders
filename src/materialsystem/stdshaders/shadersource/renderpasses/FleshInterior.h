//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.03.2023 DMY
//	Last Change :	// SET THE DATE!
//
//	Purpose of this File :	
//
//==========================================================================//

#ifndef FLESHINTERIOR_H
#define FLESHINTERIOR_H

#ifdef _WIN32
#pragma once
#endif

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

//void LuxFleshInterior_Link_Params(FleshInterior_Vars_t &info);
void LuxFleshInterior_Init_Params(CBaseVSShader* pShader, FleshInterior_Vars_t& info);
void LuxFleshInterior_Shader_Init(CBaseVSShader* pShader, FleshInterior_Vars_t& info);
void LuxFleshInterior_Shader_Draw(CBaseVSShader* pShader, CProxyShaderShadow* pShaderShadow, CProxyShaderDynamicAPI* pShaderAPI, FleshInterior_Vars_t& info);

#endif // FLESHINTERIOR_H

