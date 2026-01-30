//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	11.11.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Throw these to Stock
#ifdef REPLACE_SDK_SHADERS
	// I don't expect anyone to have ever done custom SDK_ Variants of these Shaders
	// To avoid filling the ShaderList with useless Shaders that don't exist, I commented this
	// NOTE: I'm throwing these to Stock since we don't have new Versions of these Shaders
	// So it goes SDK -> Stock -> UnlitGeneric for now
	// If you opt to add your own Versions of these Shaders you will have to change this.
	#if 0
	DEFINE_FALLBACK_SHADER(SDK_DebugDepth,					DebugDepth);
	DEFINE_FALLBACK_SHADER(SDK_DebugModifyVertex,			DebugModifyVertex);
	DEFINE_FALLBACK_SHADER(SDK_DebugMorphAccumulator,		DebugMorphAccumulator);
	DEFINE_FALLBACK_SHADER(SDK_DebugMrtTexture,				DebugMrtTexture	);
	DEFINE_FALLBACK_SHADER(SDK_DebugNormalMap,				DebugNormalMap);
	DEFINE_FALLBACK_SHADER(SDK_DebugSoftwareVertexShader,	DebugSoftwareVertexShader);
	DEFINE_FALLBACK_SHADER(SDK_DebugTrangentSpace,			DebugTrangentSpace);
	DEFINE_FALLBACK_SHADER(SDK_DebugDrawDepth,				DebugDrawDepth);
	DEFINE_FALLBACK_SHADER(SDK_DebugDrawEnvMapMask,			DebugDrawEnvMapMask	);
	DEFINE_FALLBACK_SHADER(SDK_FillRate,					FillRate);
	#endif
#endif

// LUX does not provide rewritten Versions of these Shaders.
// So I'm opting to fallback to UnlitGeneric instead. ( This might break some Stuff, be aware )
#ifdef REPLACE_DEBUGSHADERS
DEFINE_FALLBACK_SHADER(DebugDepth,					UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugModifyVertex,			UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugMorphAccumulator,		UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugMrtTexture,				UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugNormalMap,				UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugSoftwareVertexShader,	UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugTrangentSpace,			UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugDrawDepth,				UnlitGeneric);
DEFINE_FALLBACK_SHADER(DebugDrawEnvMapMask,			UnlitGeneric);
DEFINE_FALLBACK_SHADER(FillRate,					UnlitGeneric);
#endif