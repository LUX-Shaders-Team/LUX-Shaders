//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	30.05.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Include Header for ConVars
//							Previously just used extern
//
//==========================================================================//

#ifndef CPP_CONVARS_H
#define CPP_CONVARS_H

#ifdef _WIN32
#pragma once
#endif

#include "convar.h"

extern ConVar lux_version;
extern ConVar lux_oldshaders;

//==========================================================================//
// Stock ConVars
//==========================================================================//

extern ConVar CVarDeveloper;
extern ConVar mat_fullbright;
extern ConVar mat_specular;

// Used in Water.cpp
extern ConVar r_waterforceexpensive;

#ifdef DEBUG_LUXELS
extern ConVar mat_luxels;
#endif

extern ConVar mat_disable_lightwarp;
extern ConVar r_lightmap_bicubic;
extern ConVar r_rimlight;

// Used on Sky Shaders
extern ConVar mat_use_compressed_hdr_textures;

//==========================================================================//
// General ConVars
//==========================================================================//

extern ConVar lux_general_shaderstatewrappers;
extern ConVar lux_general_fixdx9hpo;

#ifdef RADIALFOG
extern ConVar lux_general_radialfog;
#endif

extern ConVar lux_general_luminanceweights;
extern ConVar lux_general_gamma;

//==========================================================================//
// Debug ConVars
//==========================================================================//

#ifdef LUX_DEBUGCONVARS
extern ConVar lux_disablefast_envmap;
extern ConVar lux_disablefast_selfillum;
extern ConVar lux_disablefast_phong;
extern ConVar lux_disablefast_lightmap;
extern ConVar lux_disablefast_lightwarp;
extern ConVar lux_disablefast_normalmap;
extern ConVar lux_disablefast_diffuse;
#endif

//==========================================================================//
// LightmappedGeneric
//==========================================================================//

extern ConVar lux_lightmapped_phong_enable;
extern ConVar lux_lightmapped_phong_force;
extern ConVar lux_lightmapped_phong_force_boost;
extern ConVar lux_lightmapped_phong_force_exp;

//==========================================================================//
// UnlitGeneric / VertexLitGeneric
//==========================================================================//

extern ConVar lux_treesway_force_static;
extern ConVar lux_treesway_static_override;
extern ConVar lux_treesway_static_x;
extern ConVar lux_treesway_static_y;

//==========================================================================//
// VertexLitGeneric
//==========================================================================//

extern ConVar lux_phong_defaulthalflambert;

extern ConVar lux_envmap_forcelerp;
extern ConVar lux_envmap_lerptime;
extern ConVar lux_envmap_flipbasealpha;

//==========================================================================//
// Cable Shader
//==========================================================================//
extern ConVar lux_cable_forcespline;

//==========================================================================//
// Sky Shaders
//==========================================================================//

extern ConVar lux_sky_UseFilter;
extern ConVar lux_sky_BicubicFilter;
extern ConVar lux_sky_UseModelMatrix;

//==========================================================================//
// Water Shader
//==========================================================================//

extern ConVar lux_water_projectedtexturesupport;
extern ConVar lux_water_debugflowmaps;
extern ConVar lux_water_forcefogtype;

//==========================================================================//
// Engine_Post
//==========================================================================//

extern ConVar lux_enginepost_gamma;
extern ConVar lux_enginepost_linearbloom;
extern ConVar lux_enginepost_force_vomit;
extern ConVar lux_enginepost_force_contrast;
extern ConVar lux_enginepost_force_depthblur;
extern ConVar lux_enginepost_force_desaturate;
extern ConVar lux_enginepost_force_vignette;
extern ConVar lux_enginepost_force_noise;

// Engine Post Overrides
extern ConVar lux_enginepost_vomit_refractfactor;
extern ConVar lux_enginepost_noise_texturescale;
extern ConVar lux_enginepost_noise_factor;
extern ConVar lux_enginepost_desaturate_factor;
extern ConVar lux_enginepost_contrast_factor;
extern ConVar lux_enginepost_contrast_vignettestart;
extern ConVar lux_enginepost_contrast_vignetteend;
extern ConVar lux_enginepost_contrast_vignetteblur;
extern ConVar lux_enginepost_contrast_edgescale;
extern ConVar lux_enginepost_depthblur_blur;
extern ConVar lux_enginepost_depthblur_focalplane;
extern ConVar lux_enginepost_depthblur_factor;

//==========================================================================//
// Debug... Shaders
//==========================================================================//

extern ConVar lux_texturelist_octahedrons;
extern ConVar lux_texturelist_fixgamma;

//==========================================================================//
// Infected Shader
//==========================================================================//

extern ConVar lux_infected_forcerandomisation;

//==========================================================================//
// Others
//==========================================================================//

// LUX FIXME: Kill these before release, rn they are only used for tf2c's engine post fallback
// Default ConVars from Engine Post
// NOTE: Made these Cheats.
#ifdef NOLUX
extern ConVar mat_screen_blur_override;
extern ConVar mat_depth_blur_focal_distance_override;
extern ConVar mat_depth_blur_strength_override;
extern ConVar mat_grain_scale_override;
extern ConVar mat_local_contrast_scale_override;
extern ConVar mat_local_contrast_midtone_mask_override;
extern ConVar mat_local_contrast_vignette_start_override;
extern ConVar mat_local_contrast_vignette_end_override;
extern ConVar mat_local_contrast_edge_scale_override;
#endif

#endif // CPP_CONVARS_H
