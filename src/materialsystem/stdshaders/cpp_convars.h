//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	30.05.2024 DMY
//	Last Change :	06.02.2026 DMY
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

extern int CVarDeveloper();
extern int mat_fullbright();
extern int mat_specular();
extern int mat_luxels();
extern int mat_queue_mode();
extern int mat_reduceparticles();

extern ConVar rope_min_pixel_diameter;
extern ConVar r_waterforceexpensive;
extern ConVar mat_disable_lightwarp;
extern ConVar r_lightmap_bicubic;
extern ConVar mat_use_compressed_hdr_textures;

#ifdef ASWSDK
extern ConVar r_flashlightbrightness;
#endif

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

extern ConVar lux_emissiveblend_allowopacity;
extern ConVar lux_projtex_shadowfiltersizefix;

#endif // CPP_CONVARS_H
