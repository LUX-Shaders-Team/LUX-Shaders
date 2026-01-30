//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	05.08.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_LIGHTWARP_H_
#define LUX_COMMON_LIGHTWARP_H_

// Disable Lightwarp on Projected Textures
#if (!PROJTEX && !defined(NOLIGHTWARP))
	#define LIGHTWARPTEXTURE 1
#elif (PROJTEX || defined(NOLIGHTWARP))
	#define LIGHTWARPTEXTURE 0
#endif

#if LIGHTWARPTEXTURE
	//==========================================================================//
	//	Declaring Samplers. We only have 16 on SM3.0. Ranging from 0-15
	//	Although s15 wants to be a gamma-lookup table, limiting us to 15/16
	//==========================================================================//
	#if !defined(MOVED_SAMPLERS_LIGHTWARP)
		sampler Sampler_LightWarpTexture : register(s6);
	#endif
	
	#if (!defined(MOVED_REGISTERS_LIGHTWARP))
		#define g_bLightWarpTexture		Bools[LUX_PS_BOOL_LIGHTWARPTEXTURE]
	#endif
#endif


#endif // LUX_COMMON_LIGHTWARP_H_