//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Define what features should be used
//							Define If we use sdk2013 MP or SP
//							Define LUX specific features
//							Define Utility and Debug features/tools
//
//	How to use :	Things you want, uncomment
//					Things you don't want, comment
//
//	Some Features can be disabled on C++ only, meaning they can't be used but technically,
//	the Shader will still supports them ( sometimes still running instructions until recompiled )
//	A Note is added to each define to indicate when to recompile
//				
//==========================================================================//

// #define SDK2013SP
#define TF2SDK
// #define ASW - Not supported

// This is only required for Parabellum
// #define PARABELLUM

// No longer applicable, leave undefined
// #define HAMMERPLUSPLUS_CRASH_FIX

// Only for debugging Shaders!
// Disable for release builds if possible.
#define LUX_DEBUGCONVARS

// Only for builds with PBR:
#define PBR_ENABLE

// TF2SDK Only:
// Replace one or more existing Shaders with those available in LUX
// NOTE:
// This will break lux_oldshaders with those Shaders, as no fallback will be available
// *IF* you write a new Shader based on LUX ones, that replaces an existing Shader
// make sure lux_oldshaders is ifdef'd, otherwise you will fall trap to a fallback loop!
// NOTE:
// Will not disable Fallbacks from LUX Shaders to other LUX Shaders
// For Example, LUX_LightmappedGeneric will fallback to LUX_LightmappedGeneric_Decal
// Even if REPLACE_LIGHTMAPPEDGENERIC_DECAL is defined
// #define REPLACE_ALL_SHADERS

// If you only want to replace *some* of them, do it here
#ifndef REPLACE_ALL_SHADERS
#define REPLACE_LIGHTMAPPEDGENERIC
#define REPLACE_LIGHTMAPPEDGENERIC_DECAL
#define REPLACE_LIGHTMAPPEDREFLECTIVE
#define REPLACE_VERTEXLITGENERIC
#define REPLACE_UNLITGENERIC
#define REPLACE_WORLDVERTEXTRANSITION
#define REPLACE_CLOUD
#define REPLACE_TEETH
#define REPLACE_CABLE
#define REPLACE_CORE
#define REPLACE_MODULATE
#define REPLACE_SHATTEREDGLASS
#define REPLACE_SPRITE
#define REPLACE_DECALMODULATE
#define REPLACE_SKY
#define REPLACE_SPRITECARD
#define REPLACE_BLOOM
#define REPLACE_BLURFILTERS
#define REPLACE_SCREENSPACE_GENERAL
#define REPLACE_UNLITTWOTEXTURE
#define REPLACE_WATER
#define REPLACE_REFRACT
#define REPLACE_EYES

// !!! TF2C RELEASE !!! +FIXME:
// ShiroDkxtro2: If something goes horribly wrong and you have to disable this Shader,
// I used ifdef NOLUX for TF2C Engine Post to get rid of unused ConVars with regular Engine Post.
// If you comment this to use the Fallback YOU MUST ALSO remove the ifdef NOLUX in tf2c_engine_post.cpp !!!
// The FIXME for this is to remove tf2c_engine_post after release ( if nothing goes wrong )
// AND to clean-up both this comment + the cpp_convars from the stock engine_post convars
#define REPLACE_ENGINE_POST

// This is a very safe 'replace'
// This Shader only exists since L4D2 onwards ( P2, CS:GO )
#define REPLACE_BLACK

// Another safe 'replace'
// This Shader only exists in Left 4 Dead 2
#define REPLACE_INFECTED

// safe 'replace'(?)
// It doesn't appear the original Shaders can even be used ( their ConVars do nothing )
// This is intended as Stubs for Linux. Linux does not have stdshader_dbg, so all of the Shaders under this Directive are just missing on LINUX
#define REPLACE_DEBUGSHADERS

// Not functional right now:
// #define REPLACE_EYEREFRACT
// #define REPLACE_PARALLAXTEST
// #define REPLACE_VOLUMECLOUDS
// #define REPLACE_WINDOWIMPOSTER
// #define REPLACE_DEPTHWRITE
#endif

// Commonly people use a SDK_ Prefix
// There is hundreds if not thousands of Materials out there using it
// This makes LUX easily Compatible with Mapbase Projects
// However, some may want to compare LUX with Mapbase Shaders or others
// Disable SDK_ Fallbacks to LUX_ by commenting
#define REPLACE_SDK_SHADERS

// NOTE: This is forced-on for TF2SDK builds
// -- Changing this requires a Shader recompile !!!
#define RADIALFOG

// This will make Ropes use the Spline Shader instead.
// Mapbase used to force splineropes, if you're on MP Mapbase and your Ropes are broken, comment this!
// -- Changing this does NOT require a Shader recompile
#define SPLINEROPES

// Makes the Shaders use the Alien Swarm HeightFogFactor Code ( which is much simpler and cheaper )
// The resulting Mask does not look 1:1 with the SDK.
// Results were eyeballed on the Water Shader to make this match the SDK
// -- Changing this requires a Shader recompile !!!
// #define ALIENSWARM_HEIGHTFOGFACTOR

//==========================================================================//
//====	  Features to use, Undefine if you don't want a feature		=========//
//==========================================================================//

// Artificially extend the Depth Range
// Note that all your Materials **MUST** be using the custom shaders IF you use this
// By default the DEPTH_RANGE is ~196 Units.
// 4096 will fit into the existing Buffer/Alpha without too much noticable Banding ( Results may vary )
// If you changed your framebuffer format, you may be able to use a max range of 16384.0f, without problems.
// See lux_common_depth.h for more information
#define DEPTH_RANGE_NEAR_Z 7
#define DEPTH_RANGE_FAR_Z 4096
// -- Changing this requires a Shader recompile !!!
// #define CUSTOM_DEPTH_RANGE (1.0f / (DEPTH_RANGE_FAR_Z - DEPTH_RANGE_NEAR_Z))

// mat_fullbright 2 support
// -- Changing this does NOT require a Shader recompile
#define DEBUG_FULLBRIGHT2

// Only on SDK2013MP, if you define this and use SP it will automatically be undefined later.
// Displays Debug Luxel Texture for models and brushes.
// -- Changing this does NOT require a Shader recompile
#define DEBUG_LUXELS

//==========================================================================//
// Everything here must use #if defined() !!
// This will be run by the Shadercompiler and it really does not like #ifdef
//==========================================================================//

// Undefine because SP doesn't have this pShaderAPI Standard-Texture
#if defined(SDK2013SP) && defined(DEBUG_LUXELS)
#undef DEBUG_LUXELS
#endif

// Must have for TF2SDK, Base Feature for the Engine now
#ifdef TF2SDK
	#if !defined(RADIALFOG)
	#define RADIALFOG
	#endif
#endif

#ifdef REPLACE_ALL_SHADERS
#define REPLACE_LIGHTMAPPEDGENERIC
#define REPLACE_LIGHTMAPPEDGENERIC_DECAL
#define REPLACE_LIGHTMAPPEDREFLECTIVE
#define REPLACE_VERTEXLITGENERIC
#define REPLACE_UNLITGENERIC
#define REPLACE_WORLDVERTEXTRANSITION
#define REPLACE_CLOUD
#define REPLACE_TEETH
#define REPLACE_CABLE
#define REPLACE_CORE
#define REPLACE_MODULATE
#define REPLACE_SHATTEREDGLASS
#define REPLACE_SPRITE
#define REPLACE_DECALMODULATE
#define REPLACE_SKY
#define REPLACE_SPRITECARD
#define REPLACE_BLOOM
#define REPLACE_BLURFILTERS
#define REPLACE_SCREENSPACE_GENERAL
#define REPLACE_UNLITTWOTEXTURE
#define REPLACE_WATER
#define REPLACE_BLACK
#define REPLACE_INFECTED
#define REPLACE_DEBUGSHADERS
#define REPLACE_REFRACT
#define REPLACE_ENGINE_POST
#define REPLACE_EYES

// Not functional right now:
// 
// #define REPLACE_EYEREFRACT
// #define REPLACE_PARALLAXTEST
// #define REPLACE_VOLUMECLOUDS

// #define REPLACE_WINDOWIMPOSTER
// #define REPLACE_DEPTHWRITE
#endif