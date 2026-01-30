//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	29.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef TRIPLANAR_H
#define TRIPLANAR_H

#ifdef _WIN32
#pragma once
#endif 

// The Other Var Structs are in this Header
#include "../../cpp_lux_shared.h"

#define Declare_TriplanarParameters()\
SHADER_PARAM(TriPlanar_Base,			SHADER_PARAM_TYPE_BOOL, "", "Enables Triplanar Mapping for $BaseTexture.")\
SHADER_PARAM(TriPlanar_Bump,			SHADER_PARAM_TYPE_BOOL, "", "Enables Triplanar Mapping for $BumpMap.")\
SHADER_PARAM(TriPlanar_Detail,			SHADER_PARAM_TYPE_BOOL, "", "Enables Triplanar Mapping for $Detail.")\
SHADER_PARAM(TriPlanar_EnvMapMask,		SHADER_PARAM_TYPE_BOOL, "", "Enables Triplanar Mapping for $EnvMapMask.")\
/* Scale similar to Seamless */\
SHADER_PARAM(TriPlanar_Scale_Base,			SHADER_PARAM_TYPE_VEC3, "", "Scale of Triplanar Mapping for $BaseTexture.")\
SHADER_PARAM(TriPlanar_Scale_Bump,			SHADER_PARAM_TYPE_VEC3, "", "Scale of Triplanar Mapping for $BumpMap.")\
SHADER_PARAM(TriPlanar_Scale_Detail,		SHADER_PARAM_TYPE_VEC3, "", "Scale of Triplanar Mapping for $Detail.")\
SHADER_PARAM(TriPlanar_Scale_EnvMapMask,	SHADER_PARAM_TYPE_VEC3, "", "Scale of Triplanar Mapping for $EnvMapMask.")\
/* The so called 3d - xforms we've all been waiting for */\
SHADER_PARAM(TriPlanar_Offset_Base,			SHADER_PARAM_TYPE_VEC3,	"", "Offset of Triplanar Mapping for $BaseTexture.")\
SHADER_PARAM(TriPlanar_Offset_Bump,			SHADER_PARAM_TYPE_VEC3,	"", "Offset of Triplanar Mapping for $BumpMap.")\
SHADER_PARAM(TriPlanar_Offset_Detail,		SHADER_PARAM_TYPE_VEC3,	"", "Offset of Triplanar Mapping for $Detail.")\
SHADER_PARAM(TriPlanar_Offset_EnvMapMask,	SHADER_PARAM_TYPE_VEC3, "", "Offset of Triplanar Mapping for $EnvMapMask.")

#define Declare_TriplanarDisplacementParameters()\
SHADER_PARAM(TriPlanar_BlendModulate,			SHADER_PARAM_TYPE_BOOL, "", "Enables Triplanar Mapping for $BlendModulateTexture.")\
SHADER_PARAM(TriPlanar_Scale_BlendModulate,		SHADER_PARAM_TYPE_VEC3, "", "Scale of Triplanar Mapping for $BlendModulateTexture.")\
SHADER_PARAM(TriPlanar_Offset_BlendModulate,	SHADER_PARAM_TYPE_VEC3, "", "Offset of Triplanar Mapping for $BlendModulateTexture.")

#endif // TRIPLANAR_H