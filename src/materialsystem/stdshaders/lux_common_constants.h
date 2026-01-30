//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	08.08.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_CONSTANTS_H_
#define LUX_COMMON_CONSTANTS_H_

//==========================================================================//
// Mathematical Constants
//==========================================================================//

#define PI (3.1415926535f)
#define TWO_PI (2.0f*PI)
#define ONE_OVER_PI (1.0f / PI)

//==========================================================================//
// Constant Graveyard
//==========================================================================//
#if 0
// Not used anywhere.
#define HDR_INPUT_MAP_SCALE 16.0f

// "If you change these, make the corresponding change in hardwareconfig.cpp"
// We cannot change the System behind this, and we replaced this with a 5x5 Filter.
// These are no longer needed.
#define NVIDIA_PCF_POISSON	0
#define ATI_NOPCF			1
#define ATI_NO_PCF_FETCH4	2

// Because the original Comments were a bit cryptic, let's explain how Fog Modes work(ed)
//
// SDK2013SP
// There are 3 Fog-Modes. None, Range and Height
// 
// TF2SDK
// There are 4 Fog-Modes. None, Range, Height and Radial
// 
// Except, this isn't actually true. NONE and RANGE are combined into one Mode
// This *forced* Fog Calculations to happen, *at all times*. Even on Shaders that don't need Fog.
// If a Map doesn't have Fog, or Fog isn't enabled, the Values for Fog are set so Fog doesn't show up. ( FogToBlack )
// 
// Height Fog is used for Water.
// Surfaces below the Water-Plane are rendered to a separate Rendertarget.
// Their Fog-Modes are ( usually ) automatically set to HEIGHT.
// Their RGB will ( also usually ) render the same as their above Water Counterparts.
// Their Alpha however will be the Height Fog Factor.
// 
// Radial Fog compared to Range Fog is distributed uniformly around the Camera.
// It solves the 'Projected Box Look' that Range Fog suffers from.
//
#define PIXEL_FOG_TYPE_NONE -1 // "MATERIAL_FOG_NONE is handled by PIXEL_FOG_TYPE_RANGE, this is for explicitly disabling fog in the shader"
#define PIXEL_FOG_TYPE_RANGE 0 // "range+none packed together in ps2b. Simply none in ps20 (instruction limits)"
#define PIXEL_FOG_TYPE_HEIGHT 1

#endif

#endif // End of LUX_COMMON_CONSTANTS_H_