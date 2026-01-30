//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	07.02.2023 DMY
//	Last Change :	30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Version Number used to be based on amount of commits
// Since v1.00 its arbitrary
ConVar lux_version("lux_version", "1.80");
ConVar lux_oldshaders("lux_oldshaders", "0", FCVAR_RELOAD_MATERIALS);

//==========================================================================//
// Stock ConVars
//==========================================================================//

// FIXME PRE-RELEASE: Developer, fullbright & mat_specular should be ConVarRefs and since that can't be in global scope, int Functions
ConVar CVarDeveloper("developer", "0", 0);
ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
ConVar mat_specular("mat_specular", "1", 0);

// Used in Water.cpp
ConVar r_waterforceexpensive("r_waterforceexpensive", "0", FCVAR_ARCHIVE, "Forces the Water Shader to use Expensive Water. This is an Archive ConVar, use with Caution.");

// Note that this is separate from the DebugLuxels Shader
// This one is used for non-Brush Geometry
#ifdef DEBUG_LUXELS
ConVar mat_luxels("mat_luxels", "0", FCVAR_CHEAT);
#endif

ConVar mat_disable_lightwarp("mat_disable_lightwarp", "0");

// ~~ConVar Name dictated by the 20th Anniversary HL2 Update~~
// Now using a copy from the TF2-SDK
ConVar r_lightmap_bicubic("r_lightmap_bicubic", "0", FCVAR_NONE, "Enable bi-cubic (high quality) lightmap sampling.");

// FIXME: Is this set by the Options Menu or Config.cfg? Why is it a Cheat
ConVar r_rimlight("r_rimlight", "1", FCVAR_CHEAT);

// Used on Sky Shaders
ConVar mat_use_compressed_hdr_textures("mat_use_compressed_hdr_textures", "0", NULL);

//==========================================================================//
// General ConVars
//==========================================================================//

ConVar lux_general_shaderstatewrappers("lux_general_shaderstatewrappers", "0", FCVAR_CHEAT,
"Enables Wrappers/Proxies for Shaderstates.\n"
"This allows for Debugging Shader by spewing Information about them in the Console or reloading their .vcs Files.\n"
"See also ConCommands like lux_reloadmaterial and lux_debug_dynamicstate.");

ConVar lux_general_fixdx9hpo("lux_general_fixdx9hpo", "0", FCVAR_CHEAT, "(Currently not Functional) Fixes the notorious Half-Pixel Offset Bug that DirectX9 has.");

#ifdef RADIALFOG
ConVar lux_general_radialfog("lux_general_radialfog", "1", FCVAR_NONE, "Allows Radial Fog, setting this to 0 will disable Radial Fog dynamically.");
#endif

ConVar lux_general_luminanceweights("lux_general_luminanceweights", "0", FCVAR_NONE, "Changes which Luminance Weights the Shader will be using.\n"
	"0 = NTSC Analog Television Standard - [0.299f, 0.587f, 0.114f]\n"
	"1 = Rec. 709 HDTV - [0.2126f, 0.7152f, 0.0722f]");

ConVar lux_general_gamma("lux_general_gamma", "2.2", FCVAR_NONE, "Changes which Gamma Value Shaders use (With the Exception of Engine Post).\n"
	"The Default is 2.2, another Option here would be 2.4, Other Values might not make as much sense."
	"To change the Value Engine Post uses, change lux_enginepost_gamma.\n");

//==========================================================================//
// Debug ConVars
//==========================================================================//

// Some Helper and Debugging ConVars
#ifdef LUX_DEBUGCONVARS
ConVar lux_disablefast_envmap("lux_disablefast_envmap", "0", FCVAR_CHEAT, "Forces $EnvMapTint to 0 on Dynamic State, effectively disabling EnvMaps.");
ConVar lux_disablefast_selfillum("lux_disablefast_selfillum", "0", FCVAR_CHEAT, "Forces $SelfIllumTint to 0 on Dynamic State, effectively disabling SelfIllum.");
ConVar lux_disablefast_phong("lux_disablefast_phong", "0", FCVAR_CHEAT, "Forces $PhongTint to 0 on Dynamic State, effectively disabling Phong.");
ConVar lux_disablefast_lightmap("lux_disablefast_lightmap", "0", FCVAR_CHEAT, "Forces Lightmaps to Black, effectively disabling them. Different from Mat_Fullbright, which sets white textures ( Useful for debugging specular lighting )!");
ConVar lux_disablefast_lightwarp("lux_disablefast_lightwarp", "0", FCVAR_CHEAT, "Setting this forces $LightWarpTexture to white.");
ConVar lux_disablefast_normalmap("lux_disablefast_normalmap", "0", FCVAR_CHEAT, "Setting this forces $BumpMap to a Flat Texture in Dynamic State, effectively disabling Normal Mapping.");
ConVar lux_disablefast_diffuse("lux_disablefast_diffuse", "0", FCVAR_CHEAT, "Forces BaseTexture Tint Values to 0.0f. Effectively Removing Diffuse Lighting. Different from disablefast_lightmap as this also affects Models. ( works similar to mat_diffuse )");
#endif

//==========================================================================//
// LightmappedGeneric
//==========================================================================//

// Brush Phong
ConVar lux_lightmapped_phong_enable("lux_lightmapped_phong_enable", "1", FCVAR_NONE, "If 1, allow phong on world brushes. If 0, disallow. lux_lightmapped_phong_force does not work if this value is 0.");
ConVar lux_lightmapped_phong_force("lux_lightmapped_phong_force", "0", FCVAR_RELOAD_MATERIALS, "Forces the use of phong on all LightmappedAdv textures, regardless of setting in VMT.");
ConVar lux_lightmapped_phong_force_boost("lux_lightmapped_phong_force_boost", "1.0", FCVAR_CHEAT);
ConVar lux_lightmapped_phong_force_exp("lux_lightmapped_phong_force_exp", "5.0", FCVAR_CHEAT);

//==========================================================================//
// UnlitGeneric / VertexLitGeneric
//==========================================================================//

ConVar lux_treesway_force_static("lux_treesway_force_static", "0", FCVAR_CHEAT, "Forces $TreeSwayStaticValues for TreeSway");
ConVar lux_treesway_static_override("lux_treesway_static_override", "0", FCVAR_CHEAT, "Forces Static_X and Static_y for TreeSway ( useful for testing )");
ConVar lux_treesway_static_x("lux_treesway_static_x", "0", FCVAR_CHEAT, "You're trying to take me,\nYou're trying to make me,\nThis is the only..\n");
ConVar lux_treesway_static_y("lux_treesway_static_y", "0", FCVAR_CHEAT, "");

//==========================================================================//
// VertexLitGeneric
//==========================================================================//

// Requested by TF2C
ConVar lux_phong_defaulthalflambert("lux_phong_defaulthalflambert", "1", FCVAR_ARCHIVE, "Sets the default Value for Half-Lambert on VertexLitGeneric - Phong.");

ConVar lux_envmap_forcelerp("lux_envmap_forcelerp", "0", FCVAR_RELOAD_MATERIALS, "Force $EnvMapLerp on all Shaders that support it. Reload Materials after setting this.");
ConVar lux_envmap_lerptime("lux_envmap_lerptime", "1.5", FCVAR_NONE, "The Time it will take to transition between two Cubemaps, in Seconds.");
ConVar lux_envmap_flipbasealpha("lux_envmap_flipbasealpha", "1", FCVAR_NONE, "By default BaseMapAlpha is flipped for the EnvMapMask. This isn't the case since Alien Swarm. This disables the default flip Behaviour.");

//==========================================================================//
// Cable Shader
//==========================================================================//
ConVar lux_cable_forcespline("lux_cable_forcespline", "0", FCVAR_CHEAT, "Causes SDK_Cable Shaders to fallback to LUX_SplineRope instead of LUX_Cable ( unless Splineropes are already enforced )");

//==========================================================================//
// Sky Shaders
//==========================================================================//

// NOTE: Skybox Bicubic is experimental
ConVar lux_sky_UseFilter("lux_sky_usefilter", "1", FCVAR_RELOAD_MATERIALS, "By Default(1) RGBs Compressed Textures ( $HDRCompressedTexture ) are filtered, setting this ConVar to 0 disables the Filter.");
ConVar lux_sky_BicubicFilter("lux_sky_bicubic", "0", FCVAR_RELOAD_MATERIALS, "Use Bicubic instead of Bilinear. Please ClampS and ClampT your Textures when using this.");
ConVar lux_sky_UseModelMatrix("lux_sky_usemodelmatrix", "0", NULL, "Allows rotation and translation of the Mesh thats used by the Skybox Shader.");

//==========================================================================//
// Water Shader
//==========================================================================//

// Used in Water.cpp
ConVar lux_water_projectedtexturesupport("lux_waterflashlightsupport", "1", FCVAR_NONE, "0 = Water Fog can *not* be illuminated by projected Textures.\n"
	"1 = Waterfog can be illuminated by projected Textures.\n"
	"ConVar forces $ReceiveProjectedTextures Values.\n"); // FIXME PRE-RELEASE: No it doesn't

ConVar lux_water_debugflowmaps("lux_water_debugflowmaps", "0", FCVAR_CHEAT, "Force Draw the FlowMap Results ( Requires a Material Reload.");
ConVar lux_water_forcefogtype("lux_water_forcefogtype", "0", FCVAR_CHEAT, "Forces Shaders to calculate the HeightFogFactor for Water using the (1) SDK or (2) Alien Swarm Method.");

//==========================================================================//
// Engine_Post
//==========================================================================//

ConVar lux_enginepost_gamma("lux_enginepost_gamma", "2.2", FCVAR_NONE, "Changes which Gamma Value the Engine Post Shader will be using.");
ConVar lux_enginepost_linearbloom("lux_enginepost_linearbloom", "0", FCVAR_CHEAT, "Stock Engine Post reproduces some whacky behaviour where Bloom was done with additive blending using sRGB Data. This ConVar disables that Behaviour.");
ConVar lux_enginepost_force_vomit("lux_enginepost_force_vomit", "0", FCVAR_CHEAT, "Forces the Effect on.");
ConVar lux_enginepost_force_contrast("lux_enginepost_force_contrast", "0", FCVAR_CHEAT, "Forces the Effect on.");
ConVar lux_enginepost_force_depthblur("lux_enginepost_force_depthblur", "0", FCVAR_CHEAT, "Forces the Effect on.");
ConVar lux_enginepost_force_desaturate("lux_enginepost_force_desaturate", "0", FCVAR_CHEAT, "Forces the Effect on.");
ConVar lux_enginepost_force_vignette("lux_enginepost_force_vignette", "0", FCVAR_CHEAT, "Forces the Effect on.");
ConVar lux_enginepost_force_noise("lux_enginepost_force_noise", "0", FCVAR_CHEAT, "Forces the Effect on.");

// Engine Post Overrides
ConVar lux_enginepost_vomit_refractfactor("lux_enginepost_vomit_refractfactor", "-1.0", FCVAR_CHEAT, "Overrides the Vomit Refraction Amount.");
ConVar lux_enginepost_noise_texturescale("lux_enginepost_noise_texturescale", "4.0", FCVAR_CHEAT, "Changes the Scale of the Noise Texture.\nThis has to be variable as the SDK doesn't have the same Noise Texture as L4D.");
ConVar lux_enginepost_noise_factor("lux_enginepost_noise_factor", "-1.0", FCVAR_CHEAT, "Overrides the Noise Intensity.");
ConVar lux_enginepost_desaturate_factor("lux_enginepost_desaturate_factor", "-1.0", FCVAR_CHEAT, "Overrides the desaturation Level.");
ConVar lux_enginepost_contrast_factor("lux_enginepost_contrast_factor", "-1.0", FCVAR_CHEAT, "Overrides the Contrast Amount.");
ConVar lux_enginepost_contrast_vignettestart("lux_enginepost_contrast_vignettestart", "-1.0", FCVAR_CHEAT, "Overrides the Contrast Vignette Start. Relative to Screen Center.");
ConVar lux_enginepost_contrast_vignetteend("lux_enginepost_contrast_vignetteend", "-1.0", FCVAR_CHEAT, "Overrides the Contrast Vignette End. Relative to Screen Center.");
ConVar lux_enginepost_contrast_vignetteblur("lux_enginepost_contrast_vignetteblur", "-1.0", FCVAR_CHEAT, "Overrides the Contrast Vignette Blur Amount.");
ConVar lux_enginepost_contrast_edgescale("lux_enginepost_contrast_edgescale", "-1.0", FCVAR_CHEAT, "Overrides the Contrast Edge Scale.");
ConVar lux_enginepost_depthblur_blur("lux_enginepost_depthblur_blur", "-1", FCVAR_CHEAT, "Overrides the DepthBlur SscreenBlur Amount.");
ConVar lux_enginepost_depthblur_focalplane("lux_enginepost_depthblur_focalplane", "-1.0", FCVAR_CHEAT, "Overrides the DepthBlur Focal Plane Distance.");
ConVar lux_enginepost_depthblur_factor("lux_enginepost_depthblur_factor", "-1.0", FCVAR_CHEAT, "Overrides the DepthBlur Amount.");

//==========================================================================//
// Debug... Shaders
//==========================================================================//

ConVar lux_texturelist_octahedrons("lux_texturelist_octahedrons", "0", FCVAR_NONE, "Causes Cubemaps in mat_texture_list to show as Octahedrons.");
ConVar lux_texturelist_fixgamma("lux_texturelist_fixgamma", "1", FCVAR_NONE, "By Default HDR Textures arrive in the Shader as Linear. mat_texture_list displays everything as Gamma, making them too dark.\nThis fixes that Issue, by converting them to Gamma.");

//==========================================================================//
// Infected Shader
//==========================================================================//

ConVar lux_infected_forcerandomisation("lux_infected_forcerandomisation", "0", FCVAR_NONE, "Forces Infected Shader to always use randomisation. ( Most Static Props set a specific Variation, this disables that ).");

//==========================================================================//
// Others
//==========================================================================//

// LUX FIXME: Kill these before release, rn they are only used for tf2c's engine post fallback
// Default ConVars from Engine Post
// NOTE: Made these Cheats.
#ifdef NOLUX
ConVar mat_screen_blur_override("mat_screen_blur_override", "-1.0", FCVAR_CHEAT);
ConVar mat_depth_blur_focal_distance_override("mat_depth_blur_focal_distance_override", "-1.0", FCVAR_CHEAT);
ConVar mat_depth_blur_strength_override("mat_depth_blur_strength_override", "-1.0", FCVAR_CHEAT);
ConVar mat_grain_scale_override("mat_grain_scale_override", "-1.0", FCVAR_CHEAT);
ConVar mat_local_contrast_scale_override("mat_local_contrast_scale_override", "0.0", FCVAR_CHEAT);
ConVar mat_local_contrast_midtone_mask_override("mat_local_contrast_midtone_mask_override", "-1.0", FCVAR_CHEAT);
ConVar mat_local_contrast_vignette_start_override("mat_local_contrast_vignette_start_override", "-1.0", FCVAR_CHEAT);
ConVar mat_local_contrast_vignette_end_override("mat_local_contrast_vignette_end_override", "-1.0", FCVAR_CHEAT);
ConVar mat_local_contrast_edge_scale_override("mat_local_contrast_edge_scale_override", "-1000.0", FCVAR_CHEAT);
#endif

