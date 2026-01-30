//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	16.12.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Allows Users to write custom Shaders and use them. ( with limited Functionality )
// 							'Recently' it was discovered that Screenspace_General can be used to write custom Shaders.
//							I'm putting Recently in '' because this was done a long time ago, but no one noticed. ( CS:GO )
//							The Screenspace_General Shader is very limited and shouldn't be modified to accomodate custom Shaders
//							This Shader will be much more flexible, and actually designed for such a Purpose.
// 
//	Usage:					To write a custom Shader, copy and modify the Template VS and PS Files.
//							You cannot add or remove any of the Static or Dynamic Combos, the Index will be computed here in the C++ Code.
//
//==========================================================================//

// Need this for Macros and general Shader Stuff
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_custom_brush_vs30.inc"
#include "lux_custom_model_vs30.inc"
#include "lux_custom_particle_vs30.inc"
#include "lux_custom_ps30.inc"
#include "lux_custom_particle_ps30.inc"
#include "lux_custom_projtex_ps30.inc"

//==========================================================================//
// Shader Start LUX_Custom
//==========================================================================//
BEGIN_VS_SHADER(LUX_Custom, "A Shader of the LUX Project")
SHADER_INFO_GEOMETRY("Brushes, Overlays, Models, MeshBuilder Meshes, Particles, and more.")
SHADER_INFO_USAGE("Expand upon the Template .fxc Files from the LUX Project and use the compiled .vcs in a VMT File.")
SHADER_INFO_LIMITATIONS("Limited Set of Features. Supports stock Lighting Methods for Models and Brushes, including Model Lightmaps.")
SHADER_INFO_PERFORMANCE("Variable, depends on the Shaders used.")
SHADER_INFO_FALLBACK("No Fallbacks.")
SHADER_INFO_WEBLINKS(WEBLINK_VDC)
SHADER_INFO_D3D(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(Shader_VS, SHADER_PARAM_TYPE_STRING, "", "The Vertex Shader .vcs Filename for regular Geometry.")
	SHADER_PARAM(Shader_ProjTex_VS, SHADER_PARAM_TYPE_STRING, "", "The Vertex Shader .vcs Filename for additive projected Textures.")
	
	SHADER_PARAM(Shader_PS, SHADER_PARAM_TYPE_STRING, "", "The Pixel Shader .vcs Filename for regular Geometry.")
	SHADER_PARAM(Shader_ProjTex_PS, SHADER_PARAM_TYPE_STRING, "", "The Pixel Shader .vcs Filename for additive projected Textures.")
	
	//==========================================================================//
	// All valid Textures
	//==========================================================================//
	// s0 is always $BaseTexture and should be used for Albedos ( since Vrad uses the Reflectivity from it )
	SHADER_PARAM(BumpMap,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] BumpMap. Uses special Loading Function. bound to s1, overrides $Texture1")
	SHADER_PARAM(BumpMap2,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] BumpMap. Uses special Loading Function. bound to s2, overrides $Texture2")
	SHADER_PARAM(Texture1,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 1. Using $BumpMap overrides this Texture.")
	SHADER_PARAM(Texture2,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 2. Using $BumpMap2 overrides this Texture.")
	SHADER_PARAM(Texture3,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 3.")
	SHADER_PARAM(Texture4,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 4.")
	SHADER_PARAM(Texture5,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 5.")
	SHADER_PARAM(Texture6,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 6.")
	SHADER_PARAM(Texture7,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 7.")
	SHADER_PARAM(Texture8,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 8.")
	SHADER_PARAM(Texture9,	SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 9.")
	SHADER_PARAM(Texture10, SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 10.")
	SHADER_PARAM(Texture11, SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 11.")
	SHADER_PARAM(Texture12, SHADER_PARAM_TYPE_TEXTURE, "", "[RGBA] Texture bound to Sampler 12.")
	
	// s13, Brushes get TEXTURE_LIGHTMAP standard-texture for lightmap, models use the parameter
	SHADER_PARAM(Lightmap,			SHADER_PARAM_TYPE_TEXTURE, "", "Model Lightmaps. For Custom Lightmaps use Lightmap_Custom. Bound to Sampler 13.")
	SHADER_PARAM(Lightmap_Custom,	SHADER_PARAM_TYPE_TEXTURE, "", "For Custom $Lightmap's. Bound to Sampler 13. Use this over $Lightmap for Custom Lightmaps")
	
	// s14, EnvMap
	SHADER_PARAM(EnvMap,			SHADER_PARAM_TYPE_TEXTURE, "", "Set the Cubemap for the Material. Bound to Sampler 14")
	
	// s15, occupied by Gamma LUT. Don't use it.
	
	//==========================================================================//
	// sRGBRead Behaviour for Samplers
	// Brush Lightmap sRGB determined by LDR/HDR
	//==========================================================================//
	SHADER_PARAM(BaseTexture_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $BaseTexture is sRGB or Linear")
	SHADER_PARAM(Texture1_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture1 is sRGB or Linear")
	SHADER_PARAM(Texture2_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture2 is sRGB or Linear")
	SHADER_PARAM(Texture3_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture3 is sRGB or Linear")
	SHADER_PARAM(Texture4_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture4 is sRGB or Linear")
	SHADER_PARAM(Texture5_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture5 is sRGB or Linear")
	SHADER_PARAM(Texture6_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture6 is sRGB or Linear")
	SHADER_PARAM(Texture7_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture7 is sRGB or Linear")
	SHADER_PARAM(Texture8_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture8 is sRGB or Linear")
	SHADER_PARAM(Texture9_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture9 is sRGB or Linear")
	SHADER_PARAM(Texture10_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture10 is sRGB or Linear")
	SHADER_PARAM(Texture11_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture11 is sRGB or Linear")
	SHADER_PARAM(Texture12_sRGB, SHADER_PARAM_TYPE_BOOL, "", "Whether $Texture12 is sRGB or Linear")

	// Brush Lightmaps depend on HDR Type
	// Otherwise Lightmaps are *always* interpreted as linear.
	// EnvMaps depend on HDR Type
	// On LDR, Brush Lightmaps and EnvMaps use sRGBRead

	//==========================================================================//
	// All valid Frame Parameters
	//==========================================================================//
	SHADER_PARAM(Texture1_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture1")
	SHADER_PARAM(Texture2_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture2")
	SHADER_PARAM(Texture3_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture3")
	SHADER_PARAM(Texture4_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture4")
	SHADER_PARAM(Texture5_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture5")
	SHADER_PARAM(Texture6_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture6")
	SHADER_PARAM(Texture7_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture7")
	SHADER_PARAM(Texture8_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture8")
	SHADER_PARAM(Texture9_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture9")
	SHADER_PARAM(Texture10_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture10")
	SHADER_PARAM(Texture11_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture11")
	SHADER_PARAM(Texture12_Frame,	SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $Texture12")
	// No Lightmap Frames! $Lightmap cannot be animated, $Lightmap_Custom can be and TEXTURE_LIGHTMAP cannot
	SHADER_PARAM(EnvMapFrame,		SHADER_PARAM_TYPE_INTEGER, "", "Frame Var for $EnvMap")
	
	//==========================================================================//
	// Constant Registers
	//==========================================================================//

	// Vertex Shader Registers
	SHADER_PARAM(vsreg_c15, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c15")
	SHADER_PARAM(vsreg_c48, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c48")
	SHADER_PARAM(vsreg_c49, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c49")
	SHADER_PARAM(vsreg_c50, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c50")
	SHADER_PARAM(vsreg_c51, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c51")
	SHADER_PARAM(vsreg_c52, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c52")
	SHADER_PARAM(vsreg_c53, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c53")
	SHADER_PARAM(vsreg_c54, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c54")
	SHADER_PARAM(vsreg_c55, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c55")
	SHADER_PARAM(vsreg_c56, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c56")
	SHADER_PARAM(vsreg_c57, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c57")
	SHADER_PARAM(Texture_Transform0, SHADER_PARAM_TYPE_MATRIX, "", "Transform Parameter 0, set to c223 and c224")
	SHADER_PARAM(Texture_Transform1, SHADER_PARAM_TYPE_MATRIX, "", "Transform Parameter 1, set to c225 and c226")
	SHADER_PARAM(Texture_Transform2, SHADER_PARAM_TYPE_MATRIX, "", "Transform Parameter 2, set to c227 and c228")
	SHADER_PARAM(Texture_Transform3, SHADER_PARAM_TYPE_MATRIX, "", "Transform Parameter 3, set to c229 and c230")

	// Pixel Shader Registers
	SHADER_PARAM(psreg_c0, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c0")
	SHADER_PARAM(psreg_c1, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c1")

	// Will be Model, View and Projection Matrix from c2 to c13, when using $Shader_Matrices
	SHADER_PARAM(psreg_c2, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c2, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c3, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c3, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c4, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c4, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c5, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c5, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c6, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c6, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c7, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c7, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c8, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c8, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c9, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c9, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c10, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c10, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c11, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c11, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c12, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c12, only when not using $Shader_Matrices and $Shader_Particle")
	SHADER_PARAM(psreg_c13, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c13, only when not using $Shader_Matrices and $Shader_Particle")
	
	// On Models that request Light Data:
	// c14 - Light Data
	// c15 - Light Data
	// c16 - Light Data - PCC OBB Row
	// c17 - Light Data - PCC OBB Row 
	// c18 - Light Data - PCC OBB Row 
	// c19 - Light Data - PCC EnvMap Origin 
	SHADER_PARAM(psreg_c14, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c14, only when not using $Shader_WorldLightData")
	SHADER_PARAM(psreg_c15, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c15, only when not using $Shader_WorldLightData")
	SHADER_PARAM(psreg_c16, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c16, only when not using $Shader_WorldLightData and PCC")
	SHADER_PARAM(psreg_c17, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c17, only when not using $Shader_WorldLightData and PCC")
	SHADER_PARAM(psreg_c18, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c18, only when not using $Shader_WorldLightData and PCC")
	SHADER_PARAM(psreg_c19, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c19, only when not using $Shader_WorldLightData and PCC")
	
	// Always occupied Registers!
	// 
	// On Models that request Ambient Cubes, or Brushes/Models with Projected Textures
	// c20 - Ambient Cube - Flashlight AttenuationFactors
	// c21 - Ambient Cube - Flashlight Pos
	// c22 - Ambient Cube - Flashlight Matrix
	// c23 - Ambient Cube - Flashlight Matrix
	// c24 - Ambient Cube - Flashlight Matrix
	// c25 - Ambient Cube - Flashlight Matrix
	// c26 - Diffuse Modulation
	// c27 - Eye Pos
	// c28 - Fog Params
	// c29 - LinearFogColor
	// c30 - cLightScale
	// c31 - LightmapData ( rcp Resolution and LightmapScaleFactor )

	// sm3.0 Registers
	SHADER_PARAM(psreg_c32, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c32")
	SHADER_PARAM(psreg_c33, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c33")
	SHADER_PARAM(psreg_c34, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c34")
	SHADER_PARAM(psreg_c35, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c35")
	SHADER_PARAM(psreg_c36, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c36")
	SHADER_PARAM(psreg_c37, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c37")
	SHADER_PARAM(psreg_c38, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c38")
	SHADER_PARAM(psreg_c39, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c39")
	SHADER_PARAM(psreg_c40, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c40")
	SHADER_PARAM(psreg_c41, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c41")
	SHADER_PARAM(psreg_c42, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c42")
	SHADER_PARAM(psreg_c43, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c43")
	SHADER_PARAM(psreg_c44, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c44")
	SHADER_PARAM(psreg_c45, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c45")
	SHADER_PARAM(psreg_c46, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c46")
	SHADER_PARAM(psreg_c47, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c47")
	SHADER_PARAM(psreg_c48, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c48")
	SHADER_PARAM(psreg_c49, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c49")
	SHADER_PARAM(psreg_c50, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c50")
	SHADER_PARAM(psreg_c51, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c51")
	SHADER_PARAM(psreg_c52, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c52")
	SHADER_PARAM(psreg_c53, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c53")
	SHADER_PARAM(psreg_c54, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c54")
	SHADER_PARAM(psreg_c55, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c55")
	SHADER_PARAM(psreg_c56, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c56")
	SHADER_PARAM(psreg_c57, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c57")
	SHADER_PARAM(psreg_c58, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c58")
	SHADER_PARAM(psreg_c59, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c59")
	SHADER_PARAM(psreg_c60, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c60")
	SHADER_PARAM(psreg_c61, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c61")
	SHADER_PARAM(psreg_c62, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c62")
	SHADER_PARAM(psreg_c63, SHADER_PARAM_TYPE_VEC4, "", "[x y z w] for c63")

	// .. could go down all the way to c224 but that's a bit much

	//==========================================================================//
	// All valid input booleans, passed to boolean registers
	// The rest of them are reserved for other things ( such as bInEditor )
	//==========================================================================//
	SHADER_PARAM(psreg_b0, SHADER_PARAM_TYPE_BOOL, "", "false/true for b0")
	SHADER_PARAM(psreg_b1, SHADER_PARAM_TYPE_BOOL, "", "false/true for b1")
	SHADER_PARAM(psreg_b2, SHADER_PARAM_TYPE_BOOL, "", "false/true for b2")
	SHADER_PARAM(psreg_b3, SHADER_PARAM_TYPE_BOOL, "", "false/true for b3")
	SHADER_PARAM(psreg_b4, SHADER_PARAM_TYPE_BOOL, "", "false/true for b4")
	SHADER_PARAM(psreg_b5, SHADER_PARAM_TYPE_BOOL, "", "false/true for b5")
	SHADER_PARAM(psreg_b6, SHADER_PARAM_TYPE_BOOL, "", "false/true for b6")
	SHADER_PARAM(psreg_b7, SHADER_PARAM_TYPE_BOOL, "", "false/true for b7")
	SHADER_PARAM(psreg_b8, SHADER_PARAM_TYPE_BOOL, "", "false/true for b8")
	SHADER_PARAM(psreg_b9, SHADER_PARAM_TYPE_BOOL, "", "false/true for b9")
	SHADER_PARAM(psreg_b10, SHADER_PARAM_TYPE_BOOL, "", "false/true for b10")
	SHADER_PARAM(psreg_b11, SHADER_PARAM_TYPE_BOOL, "", "false/true for b11")

	//==========================================================================//
	// Shader Settings
	// Some things are considered automatically by default
	// Such as $VertexColor, $Translucent, $Decal, $Model, $AlphaTest ...
	// These, aren't
	//==========================================================================//
	SHADER_PARAM(Shader_WorldLightData, SHADER_PARAM_TYPE_BOOL, "", "Enables LightData for Models. c14 to c19. Only when not in the ProjTex Pass!")
	SHADER_PARAM(Shader_AmbientData, SHADER_PARAM_TYPE_BOOL, "", "Enables Ambient Cubes for Models. c20 to c25. Only when not in the ProjTex Pass!")
	SHADER_PARAM(Shader_EyePos, SHADER_PARAM_TYPE_BOOL, "", "Enables EyePos. c27.")
	SHADER_PARAM(Shader_FogData, SHADER_PARAM_TYPE_BOOL, "", "Enables FogData. c28.")
	SHADER_PARAM(Shader_Matrices, SHADER_PARAM_TYPE_BOOL, "", "Enables Matrices, replaces c2 to c13 with Model, View and Proj Matrices (4x4). Sets VS c223 to View and VS c227 to Proj Matrix.")

	// Fog modes
	// 0 = DefaultFog (default)
	// 1 = FogToBlack (default for Projected Texture Pass)
	// 2 = FogToGrey
	// 3 = FogToWhite
	// 4 = FogToOOOverbright
	// 5 = DisableFog ( What is the Difference to FogToBlack? )
	SHADER_PARAM(Shader_FogMode,			SHADER_PARAM_TYPE_INTEGER, "", "FogMode Override.\n0 = DefaultFog.\n1 = FogToBlack.\n2 = FogToGrey\n3 = FogToWhite\n4 = FogToOOOverbright\n5 = DisableFog")
	SHADER_PARAM(Shader_FogMode_ProjTex,	SHADER_PARAM_TYPE_INTEGER, "", "FogMode Override. See $Shader_FogMode for possible modes.")

	// Post Draw Behaviour
	SHADER_PARAM(Shader_Disable_sRGBWrites,  SHADER_PARAM_TYPE_BOOL, "", "sRGBWrite means conversion to sRGB. Disabling it means you write as is to the rt, useful for HDR Texture.")
	SHADER_PARAM(Shader_Disable_DepthWrites, SHADER_PARAM_TYPE_BOOL, "", "Disable DepthWrites.")
	SHADER_PARAM(Shader_Disable_AlphaWrites, SHADER_PARAM_TYPE_BOOL, "", "By Default Shaders will write Alpha to the rt, this disables that. AlphaBlending_Enable overwrites this.")
	SHADER_PARAM(Shader_Disable_DepthTests,	 SHADER_PARAM_TYPE_BOOL, "", "Disables DepthTesting, this forces the rendered Mesh to always draw in Front of everything else.")
	SHADER_PARAM(Shader_Disable_ColorWrites, SHADER_PARAM_TYPE_BOOL, "", "Disables ColorWrites.")

	// Not as complicated as Tangents
	SHADER_PARAM(Shader_Normals, SHADER_PARAM_TYPE_BOOL, "", "Indicate that the Shader requires Vertex Normals.")

	// Ask for Tangent Data. Sets UserDataSize to 4 for Models, VERTEX_TANGENT_SPACE for Brushes
	// ( also sets Material_Var2 )
	SHADER_PARAM(Shader_Tangents, SHADER_PARAM_TYPE_BOOL, "", "Indicate that the Shader requires Vertex Tangents.")	
	SHADER_PARAM(Shader_WrinkleWeights, SHADER_PARAM_TYPE_BOOL, "", "Set the Vertex Shader Static Combo for WrinkleMapping.")
	SHADER_PARAM(Shader_UncompressedModel, SHADER_PARAM_TYPE_BOOL, "", "Stops the Vertex Format from being marked as being Compressed.")
	
	// Internal bool technically but you could set it yourself if you have a shader set
	SHADER_PARAM(Shader_ProjTexAllowed, SHADER_PARAM_TYPE_BOOL, "", "Internal Parameter. Set to 0 when not defining ProjTex Shaders.")

	// Models only.
	SHADER_PARAM(Shader_Model_SecondTexCoord, SHADER_PARAM_TYPE_BOOL, "", "Sets 2 TexCoords for the Vertex Shader Vertex Format. Your Mesh must support this to be able to use it.")

	// Override to Particle specific Vertex Shader Vertex Format
	// ( Particle Support )
	SHADER_PARAM(Shader_Particle,					SHADER_PARAM_TYPE_BOOL, "", "Sets Vertex Format specific to Particles. Without this Custom Particles won't really be possible.")
	SHADER_PARAM(Shader_Particle_VS_Mode,			SHADER_PARAM_TYPE_INTEGER, "", "Alternative VS Static for a Particle Vertex Shader. Must range between 0 and 5.")
	SHADER_PARAM(Shader_Particle_PS_Mode,			SHADER_PARAM_TYPE_INTEGER, "", "Alternative PS Static for a Particle Pixel Shader. Must range between 0 and 5.")
	SHADER_PARAM(Shader_Particle_Spline,			SHADER_PARAM_TYPE_BOOL, "", "Sets Vertex Format specific to Spline Particles.")
	SHADER_PARAM(Shader_Particle_SecondSequence,	SHADER_PARAM_TYPE_BOOL, "", "overrides Particle TexCoord count for Second Sequence. 'The whole Shebang'")
	SHADER_PARAM(Shader_Particle_DepthBlend,		SHADER_PARAM_TYPE_BOOL,	 "", "Sets Static Combo, Binds RT to s0 and sets DepthFeathering Constant ( c0 )");
	SHADER_PARAM(Shader_Particle_DepthBlendScale,	SHADER_PARAM_TYPE_FLOAT, "", "Amplify or reduce DepthBlend fading. Lower Values cause more abrupt Transitions.")
	SHADER_PARAM(Shader_Particle_MaxDistance,		SHADER_PARAM_TYPE_FLOAT, "", "Default is 10000, maximum Distance of the Particle.");
	SHADER_PARAM(Shader_Particle_FarFadeInterval,	SHADER_PARAM_TYPE_FLOAT, "", "Default is 400, fade factor. MaxDistance - This clamped to minimum of 1.0f");
	SHADER_PARAM(Shader_Particle_MinSize,			SHADER_PARAM_TYPE_FLOAT, "", "Default is 0. Forces a minimum Size in Units.");
	SHADER_PARAM(Shader_Particle_MaxSize,			SHADER_PARAM_TYPE_FLOAT, "", "Default is 20. Forces a maximum Size in Units.");
	SHADER_PARAM(Shader_Particle_StartFadeSize,		SHADER_PARAM_TYPE_FLOAT, "", "Default is 10.");
	SHADER_PARAM(Shader_Particle_EndFadeSize,		SHADER_PARAM_TYPE_FLOAT, "", "Default is 0.");
	SHADER_PARAM(Shader_Particle_ZoomAnimateSeq2,	SHADER_PARAM_TYPE_FLOAT, "", "Amount of gradual Zoom between Frames, on the Second sequence. 2.0 will *double* the Size of a Frame over its Lifetime.")
	SHADER_PARAM(Shader_Particle_ExtractGreenAlpha, SHADER_PARAM_TYPE_BOOL, "", "Use Factors to blend Green/Alpha Channels.")
	SHADER_PARAM(Shader_Particle_AddBaseTexture2,	SHADER_PARAM_TYPE_BOOL, "", "Enables second Texture blend Path. Float must be set manually to a Register.")

	// Not adapting a new Name since I don't know where this usually gets set. ( I don't make Particles. )
	// I was told it happens 'in the renderer', I'm not taking any chances that it'll be impossible set the Orientation Mode in the Particle Editor..
	// There is technically a third mode of Orientation that aligns a Particle with a Control Point
	// But it doesn't appear Code for this is public so I will allow 3 as a custom Orientation Mode instead.
	SHADER_PARAM(Orientation, SHADER_PARAM_TYPE_INTEGER, "", "For Particles only.\n0 = Always face Camera.\n1 = Rotate around Z-Axis.\n2 = Parallel to Ground.\n3 = Custom.")

	// Custom AlphaBlending
	// See also:
	// ShaderBlendFactor_t
	SHADER_PARAM(Shader_AlphaBlending_Enable, SHADER_PARAM_TYPE_BOOL, "", "Enables AlphaBlending Modes, disables AlphaWrites and DepthWrites. When not used, $AlphaTest / $Translucent / $Additive are handled automatically.")
	SHADER_PARAM(Shader_AlphaBlending_Src, SHADER_PARAM_TYPE_INTEGER, "", "AlphaBlend Src Mode. Reference public ShaderBlendFactor_t struct for what to input here. Too complicated to explain.")
	SHADER_PARAM(Shader_AlphaBlending_Dst, SHADER_PARAM_TYPE_INTEGER, "", "AlphaBlend Dst Mode. Reference public ShaderBlendFactor_t struct for what to input here. Too complicated to explain.")

	SHADER_PARAM(Shader_DepthFunc_Enable, SHADER_PARAM_TYPE_BOOL, "", "Enables Custom DepthFuncs")
	SHADER_PARAM(Shader_DepthFunc, SHADER_PARAM_TYPE_INTEGER, "", "Depth Fund Enumeration to be used with $Shader_DepthFunc_Enable.")

	SHADER_PARAM(Shader_AlphaTestReference, SHADER_PARAM_TYPE_FLOAT, "", "Reference for AlphaTesting. Used only when defined.")
	SHADER_PARAM(Shader_AlphaTestFunc, SHADER_PARAM_TYPE_INTEGER, "", "AlphaTesting Function. By Default Greater Than.")

	//==========================================================================//
	// Custom BitMask for $Flags and $Flags2
	//==========================================================================//
	SHADER_PARAM(Shader_Flags, SHADER_PARAM_TYPE_INTEGER, "", "Adds Flags to $Flags.")
	SHADER_PARAM(Shader_Flags2, SHADER_PARAM_TYPE_INTEGER, "", "Adds Flags to $Flags2.")

	//==========================================================================//
	// Support for the dominant Source Parallax Corrected Cubemap Implementation
	// Brushes only.
	//==========================================================================//
	SHADER_PARAM(EnvMapParallax,		SHADER_PARAM_TYPE_BOOL, "0", "Enables Parallax Correction correction for Cubemaps. Set by vbsp via Patch VMT.")
	SHADER_PARAM(EnvMapParallaxOBB1,	SHADER_PARAM_TYPE_VEC4, "[1 0 0 0]", "Matrix Row. Set by vbsp via Patch VMT.")
	SHADER_PARAM(EnvMapParallaxOBB2,	SHADER_PARAM_TYPE_VEC4, "[0 1 0 0]", "Matrix Row. Set by vbsp via Patch VMT.")
	SHADER_PARAM(EnvMapParallaxOBB3,	SHADER_PARAM_TYPE_VEC4, "[0 0 1 0]", "Matrix Row. Set by vbsp via Patch VMT.")
	SHADER_PARAM(EnvMapOrigin,			SHADER_PARAM_TYPE_COLOR, "[0 0 0]", "World Space Position of the env_cubemap. Set by vbsp via Patch VMT.")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	bool bParticle = GetBool(Shader_Particle);

	// EnvMaps *usually* require WorldSpace Normal Maps
	// And those require Tangents.
	if (IsDefined(EnvMap) && (IsDefined(BumpMap) || IsDefined(BumpMap2)))
		SetBool(Shader_Tangents, true);

	// If we want Tangents we should indicate this using the Var2
	bool bWantsTangents = GetBool(Shader_Tangents);

	// Supposedly Brushes always need Tangents for Projected Textures so I'm replicating that Behaviour
	if(bWantsTangents || !HasFlag(MATERIAL_VAR_MODEL))
		SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// Brushes need different Flags than Models
	// Lightmap Flags are important for Brushes. Without them VRad will not compile Lightmaps for them.
	// If Lightmaps don't show up, make sure you are using a custom vbsp that has the ModInit() Code to allow for custom Shaders.
	if (bParticle)
	{
		DefaultFloat(Shader_Particle_MaxDistance, 100000.0);
		DefaultFloat(Shader_Particle_FarFadeInterval, 400.0);
		DefaultFloat(Shader_Particle_MaxSize, 20.0);
		DefaultFloat(Shader_Particle_EndFadeSize, 20.0);
		DefaultFloat(Shader_Particle_StartFadeSize, 10.0);
		DefaultFloat(Shader_Particle_DepthBlendScale, 50.0);

		// What exactly does this entail?
		SetFlag2(MATERIAL_VAR2_IS_SPRITECARD);

		// Consistent with SpriteCard
		SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

		// Ambient Cubes work on Particles, although they are a bit janky.
		if (GetBool(Shader_AmbientData))
			SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);

		// Don't know if this actually works but would be nice if it does
		if(GetBool(Shader_WorldLightData))
			SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL); // Required for dynamic Lighting
	}
	if (!HasFlag(MATERIAL_VAR_MODEL))
	{
		SetFlag2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

		// Brushes may want Directional Lightmaps
		if (IsDefined(BumpMap) || IsDefined(BumpMap2))
			SetFlag2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
	}
	else
	{
		// Always!
		SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
		SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING); // Required for Skinning

		if (IsDefined(BumpMap) || IsDefined(BumpMap2))
		{
			SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL); // Required for dynamic Lighting
		}

		if (GetBool(Shader_AmbientData))
			SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);
	}

	if (IsDefined(EnvMap))
		SetFlag2(MATERIAL_VAR2_USES_ENV_CUBEMAP);

	// Decals get this Flag
	// What it does down the line is force transparency and tint in debug views
	if (HasFlag(MATERIAL_VAR_DECAL))
		SetFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

	// Want this to fall back by default
	// That way we get FogToBlack and don't mess up flashlight fog
	if (!IsDefined(Shader_FogMode_ProjTex))
		SetInt(Shader_FogMode_ProjTex, 1);

	if (IsDefined(Shader_ProjTex_VS) && IsDefined(Shader_ProjTex_PS))
		SetBool(Shader_ProjTexAllowed, true);

	// Add any additional Flags if desired
	// I've seen some people manually apply Flags2 via Proxies and that's pretty cursed
	// It will also happen like every Frame I believe.. Let's make it once
	if (IsDefined(Shader_Flags))
	{
		SetFlag((MaterialVarFlags_t)GetInt(Shader_Flags));
	}
	if (IsDefined(Shader_Flags2))
	{
		SetFlag2((MaterialVarFlags2_t)GetInt(Shader_Flags2));
	}
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture);

	// If we have a $BumpMap or $BumpMap2 use that over $Texture1 and $Texture2
	if (g_pConfig->UseBumpmapping() && IsDefined(BumpMap))
	{
		LoadBumpMap(BumpMap);
	}
	else
		LoadTexture(Texture1, GetBool(Texture1_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	
	if (g_pConfig->UseBumpmapping() && IsDefined(BumpMap2))
	{
		LoadBumpMap(BumpMap2);
	}
	else
		LoadTexture(Texture2, GetBool(Texture2_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	
	// Could be optimised with a loop..
	LoadTexture(Texture3, GetBool(Texture3_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture4, GetBool(Texture4_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture5, GetBool(Texture5_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture6, GetBool(Texture6_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture7, GetBool(Texture7_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture8, GetBool(Texture8_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture9, GetBool(Texture9_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture10, GetBool(Texture10_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture11, GetBool(Texture11_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	LoadTexture(Texture12, GetBool(Texture12_sRGB) ? TEXTUREFLAGS_SRGB : 0);
	
	// 13 is lightmap, although I'm not sure if we even have to load it
	// The engine should construct it from bits and then send the Pointer here for us?
	if (IsDefined(Lightmap_Custom))
		LoadTexture(Lightmap_Custom);
	else
		LoadTexture(Lightmap);
	
	// This LoadFlag does not work on the SDK. I'm adding it just for completion sake.
	// SDK Users must *manually* flag the .vtf Files within .bsp's to use all MipMaps
	// My hope is that if this makes it somewhere else, it will work
	LoadCubeMap(EnvMap, TEXTUREFLAGS_ALL_MIPS);
	if(IsDefined(EnvMapParallax))
		DefaultBool(EnvMapParallax, true);
}

SHADER_DRAW
{
	bool bParticle = GetBool(Shader_Particle);
	bool bParticleDepthBlend = bParticle && GetBool(Shader_Particle_DepthBlend);

	// All Flags we have to manually support..

	// We have two separate Vertex Shaders for Brushes and Models
	// If a Shader is meant to be run on a Model, $Model must be set.
	bool bIsModel = HasFlag(MATERIAL_VAR_MODEL);

	// UsingFlashlight uses MATERIAL_VAR2_USE_FLASHLIGHT for Shadow State
	// So that's a flag we have to manually support
	// Note that this won't be allowed if Shader_ProjTex_VS and PS are null
	bool bProjectedTexture = UsingFlashlight() && GetBool(Shader_ProjTexAllowed);

	// VertexColors are required for Hammer Texture Shaded Polygons
	// Custom Shaders won't support Stock Hammer Lighting Preview but these modes will be very handy.
	// Lighting Preview in Hammer++ solves the DYN Combo Issue anyways
	bool bVertexColor = HasFlag(MATERIAL_VAR_VERTEXCOLOR);
	bool bVertexAlpha = HasFlag(MATERIAL_VAR_VERTEXALPHA);

	// Rendering Types
	bool bTranslucent = HasFlag(MATERIAL_VAR_TRANSLUCENT);
	bool bAdditive = HasFlag(MATERIAL_VAR_ADDITIVE);
	bool bAlphaTest	= HasFlag(MATERIAL_VAR_ALPHATEST);

	// Special Textures
	bool bBumpMap1 = !bParticle && IsTextureLoaded(BumpMap);
	bool bBumpMap2 = !bParticle && IsTextureLoaded(BumpMap2);
	bool bBumpMapped = (bBumpMap1 || bBumpMap2);

	// Textures. 
	bool bTexture0 = !bParticleDepthBlend && IsTextureLoaded(BaseTexture);
	bool bTexture1 = !bBumpMap1 && IsTextureLoaded(Texture1);
	bool bTexture2 = !bBumpMap2 && IsTextureLoaded(Texture2);
	bool bTexture3 = IsTextureLoaded(Texture3);
	bool bTexture4 = IsTextureLoaded(Texture4);
	bool bTexture5 = IsTextureLoaded(Texture5);
	bool bTexture6 = IsTextureLoaded(Texture6);
	bool bTexture7 = IsTextureLoaded(Texture7);
	bool bTexture8 = IsTextureLoaded(Texture8);
	bool bTexture9 = IsTextureLoaded(Texture9);
	bool bTexture10 = IsTextureLoaded(Texture10);
	bool bTexture11 = IsTextureLoaded(Texture11);
	bool bTexture12 = IsTextureLoaded(Texture12);

	// s13. When !bIsModel we use TEXTURE_LIGHTMAP
	bool bCustomLightmap = bIsModel && IsTextureLoaded(Lightmap_Custom);
	bool bModelLightmap = bIsModel && IsTextureLoaded(Lightmap);

	// s14
	bool bHasEnvMap = !bProjectedTexture && !bParticle && IsTextureLoaded(EnvMap);
	bool bPCC = !bIsModel && !bParticle && bHasEnvMap && GetBool(EnvMapParallax);

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		bool bAlphaWrites = !GetBool(Shader_Disable_AlphaWrites);
		bool bDepthWrites = !GetBool(Shader_Disable_DepthWrites);
	
		// This overrides over Blending Modes ( read: Flags like $Additive )
		if (GetBool(Shader_AlphaBlending_Enable))
		{
			ShaderBlendFactor_t SrcFactor = (ShaderBlendFactor_t)GetInt(Shader_AlphaBlending_Src);
			ShaderBlendFactor_t DstFactor = (ShaderBlendFactor_t)GetInt(Shader_AlphaBlending_Dst);

			// Clamp this just in case someone does something they really shouldn't
			if (SrcFactor < SHADER_BLEND_ZERO || SrcFactor > SHADER_BLEND_ONE_MINUS_SRC_COLOR)
				SrcFactor = SHADER_BLEND_ZERO;

			if (DstFactor < 0 || DstFactor > 10)
				DstFactor = SHADER_BLEND_ONE;

			EnableAlphaBlending(SrcFactor, DstFactor);

			bAlphaWrites = false;
			bDepthWrites = false;
		}
		else
		{
			// Replicating Behaviour of Stock Shaders
			if (bProjectedTexture)
			{
				// Always Additive
				EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // BT_ADD

				bAlphaWrites = false;
				bDepthWrites = false;
			}
			else if (bTranslucent)
			{
				if (bAdditive)
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE); // BT_BLENDADD
				else
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA); // BT_BLEND

				bAlphaWrites = false;
				bDepthWrites = false;
			}
			else if (bAdditive)
			{
				EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // BT_ADD

				bAlphaWrites = false;
				bDepthWrites = false;
			}
			else if (bAlphaTest)
			{
				pShaderShadow->EnableAlphaTest(true);

				float f1AlphaTestRef = GetFloat(Shader_AlphaTestReference);
				if (f1AlphaTestRef > 0.0f) // 0 is the default
				{
					ShaderAlphaFunc_t nAlphaFunc = SHADER_ALPHAFUNC_GEQUAL;

					// User Picked AlphaFunc
					if (IsDefined(Shader_AlphaTestFunc))
						nAlphaFunc = (ShaderAlphaFunc_t)GetInt(Shader_AlphaTestFunc);

					// Clamp this just in case someone does something they really shouldn't
					if (nAlphaFunc >= SHADER_ALPHAFUNC_NEVER && nAlphaFunc <= SHADER_ALPHAFUNC_ALWAYS)
						pShaderShadow->AlphaFunc(nAlphaFunc, f1AlphaTestRef);
				}
			}
		}

		// Projected Texture gets it's own FogMode
		int nFogMode = 0;

		// EnableAlphaBlending disables DepthWrites but not AlphaWrites
		// So make sure we don't do either when blending
		// Never do Alpha or DepthWrites for Projected Textures
		if (bProjectedTexture)
		{
			bAlphaWrites = false;
			bDepthWrites = false;

			nFogMode = GetInt(Shader_FogMode_ProjTex);
		}
		else
		{
			nFogMode = GetInt(Shader_FogMode);
		}

		pShaderShadow->EnableAlphaWrites(bAlphaWrites);
		pShaderShadow->EnableSRGBWrite(!GetBool(Shader_Disable_sRGBWrites));

		if (!bDepthWrites)
			pShaderShadow->EnableDepthWrites(false);

		if(GetBool(Shader_Disable_DepthTests))
			pShaderShadow->EnableDepthTest(false);

		if (GetBool(Shader_Disable_ColorWrites))
			pShaderShadow->EnableColorWrites(false);

		// Depth Func
		if (GetBool(Shader_DepthFunc_Enable))
		{
			ShaderDepthFunc_t nDepthFunc = (ShaderDepthFunc_t)GetInt(Shader_DepthFunc);
			
			// Clamp just in case
			if(nDepthFunc >= SHADER_DEPTHFUNC_NEVER && nDepthFunc <= SHADER_DEPTHFUNC_ALWAYS)
				pShaderShadow->DepthFunc(nDepthFunc);
		}

		// 0 = DefaultFog (default for regular Pass)
		// 1 = FogToBlack (default for projected Textures)
		// 2 = FogToGrey
		// 3 = FogToWhite
		// 4 = FogToOOOverbright
		// 5 = DisableFog ( NOT the same as FogToBlack )
		switch (nFogMode)
		{
			case 0:
				DefaultFog();
				break;

			case 1:
				FogToBlack();
				break;

			case 2:
				FogToGrey();
				break;

			case 3:
				FogToWhite();
				break;

			case 4:
				FogToOOOverbright();
				break;

			case 5:
				DisableFog();
				break;

			default:
				DefaultFog();
				break;
		}
		
		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		bool bVertexNormals = GetBool(Shader_Normals);
		bool bVertexTangents = GetBool(Shader_Tangents);
		bool bVertexRGBA = (bVertexColor || bVertexAlpha);

		// Particle specific Vertex Format
		if (bParticle)
		{
			// Default Flags for a Particle
			unsigned int nFlags = VERTEX_POSITION | VERTEX_COLOR;
			int nUserDataSize = 0;

			// I don't think this is actually possible.
			// I'm just going to allow it anyways in case it is possible or will be possible.
			if (bVertexNormals)
				nFlags |= VERTEX_NORMAL;

			// I'm assuming these are more similar to Models than Brushes
			if (bVertexTangents)
				nUserDataSize = 4;

			// TODO: Update the comments here, just make a big box that explains exactly whats going on
			static int vTexCoordSize[8] =
			{
				4,				// 0 = sheet bounding uvs, frame0
				4,				// 1 = sheet bounding uvs, frame 1
				4,				// 2 = frame blend, rot, radius, ???
				2,				// 3 = corner identifier ( 0/0,1/0,1/1, 1/0 )
				4,				// 4 = texture 2 bounding uvs
				4,				// 5 = second sequence bounding uvs, frame0
				4,				// 6 = second sequence bounding uvs, frame1
				4,				// 7 = second sequence frame blend, ?,?,?
			};

			static int vTexCoordSizeSpline[8] =
			{
				4,				// 0 = sheet bounding uvs, frame0
				4,				// 1 = sheet bounding uvs, frame 1
				4,				// 2 = frame blend, rot, radius, ???
				4,				// 3 = corner identifier ( 0/0,1/0,1/1, 1/0 )
				4,				// 4 = texture 2 bounding uvs
				4,				// 5 = second sequence bounding uvs, frame0
				4,				// 6 = second sequence bounding uvs, frame1
				4,				// 7 = second sequence frame blend, ?,?,?
			};

			// See above for what which TexCoord does and when
			int nTexCoords = 5;

			if (GetBool(Shader_Particle_SecondSequence))
			{
				// "the whole shebang - 2 sequences, with a possible multi-image sequence first"
				nTexCoords = 8;
			}

			int *pTexCoordDim;
			if (GetBool(Shader_Particle_Spline))
				pTexCoordDim = vTexCoordSizeSpline;
			else 
				pTexCoordDim = vTexCoordSize;
			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, pTexCoordDim, nUserDataSize);
		}
		// Models require vastly different Vertex Shader Vertex Format than Brushes
		else if (bIsModel)
		{
			unsigned int nFlags = VERTEX_POSITION;
			int nUserDataSize = 0;

			// Most regular Models ( Static Props etc ) have Compressed Vertices
			// There is a chance something might not have it ( Brush with special Shader, Decals, .. )
			// So by default we set Compressed Vertex Format but allow overriding.
			if (!GetBool(Shader_UncompressedModel))
				nFlags |= VERTEX_FORMAT_COMPRESSED;

			// Vertex Tangents come through the NORMAL Stream unless Compression is off,
			// in which case the Tangent comes through the TANGENT Stream, with Binormal Sign in the .w of the TANGENT Stream
			if (bVertexNormals || bVertexTangents)
				nFlags |= VERTEX_NORMAL;

			// This allows for the TANGENT Stream to be used during uncompressed Tangents
			// ( Unfortunately ) we cannot detect uncompressed Tangents here I believe
			// There is MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING, maybe it's set when $SoftwareSkin and Face Flexes ( which also get uncompressed Verts )
			if (bVertexTangents || bProjectedTexture)
				nUserDataSize = 4;

			// Vertex Colors are sometimes used by Hammer and this could be used on DetailSprites so support it
			if (bVertexRGBA)
				nFlags |= VERTEX_COLOR;

			int nTexCoords = 1;
			if (GetBool(Shader_Model_SecondTexCoord))
				nTexCoords = 2;

			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);
		}
		// Brushes
		else
		{
			unsigned int nFlags = VERTEX_POSITION;
			int nUserDataSize = 0;

			// I want to avoid a too small Vertex Data Size here so always have Normals with Tangents
			if (bVertexNormals || bVertexTangents)
				nFlags |= VERTEX_NORMAL;

			// Brushes get Tangents and Binormal through their own Stream ( TANGENT and BINORMAL )
			if (bVertexTangents || bProjectedTexture)
				nFlags |= VERTEX_TANGENT_SPACE;

			// Vertex Colors are used in Hammer for Texture Shaded Polygons. And Displacements use the Alpha for Blending.
			if (bVertexRGBA)
				nFlags |= VERTEX_COLOR;

			// TEXCOORD0 = BaseTexture UV
			// TEXCOORD1 = Lightmap UV
			// TEXCOORD2 = Lightmap UV .x Offset for Directional Lightmaps
			int nTexCoords = 2 + (bBumpMapped);

			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);
		}

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// This could just be a loop..
		// I'm unrolling it because that's easier to maintain.

		// s0
		if (bTexture0)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, GetBool(BaseTexture_sRGB));
		}
		else if (bParticleDepthBlend)
		{
			// Using s0 for the RT with Depth on it
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, false);
		}

		// s1
		if (bBumpMap1)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, false);
		}
		else if (bTexture1)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, GetBool(Texture1_sRGB));
		}
		// s2
		if (bBumpMap2)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, false);
		}
		else if (bTexture2)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, GetBool(Texture2_sRGB));
		}
		// s3
		if (bTexture3)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER3, GetBool(Texture3_sRGB));
		}
		// s4
		if (bTexture4)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER4, GetBool(Texture4_sRGB));
		}
		// s5
		if (bTexture5)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER5, GetBool(Texture5_sRGB));
		}
		// s6
		if (bTexture6)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER6, GetBool(Texture6_sRGB));
		}
		// s7
		if (bTexture7)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER7, GetBool(Texture7_sRGB));
		}
		// s8
		if (bTexture8)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER8, GetBool(Texture8_sRGB));
		}
		// s9
		if (bTexture9)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER9, GetBool(Texture9_sRGB));
		}
		// s10
		if (bTexture10)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER10, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER10, GetBool(Texture10_sRGB));
		}
		// s11
		if (bTexture11)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER11, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER11, GetBool(Texture11_sRGB));
		}
		// s12
		if (bTexture12)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER12, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER12, GetBool(Texture12_sRGB));
		}

		if (bProjectedTexture)
		{
			// s13
			// Depth
			pShaderShadow->EnableTexture(SHADER_SAMPLER13, true);
			pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER13);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER13, false);

			// s14
			// Noise
			pShaderShadow->EnableTexture(SHADER_SAMPLER14, true);

			// s15
			// Cookie Cutter
			pShaderShadow->EnableTexture(SHADER_SAMPLER15, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER15, true);
		}
		else
		{
			// Specifically != NONE to support Float HDR !!
			bool bHDR = g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;

			// s13
			if (bIsModel)
			{
				// Lightmaps are never sRGB
				// They are converted in the Shader
				// Always enable this Sampler, we don't know in Snapshot State whether there is one, except if there is a Custom Lightmap
//				if (bCustomLightmap || bModelLightmap)
				{
					pShaderShadow->EnableTexture(SHADER_SAMPLER13, true);
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER13, false);
				}
			}
			else
			{
				// Only HDR gets non-sRGB Lightmaps
				// It always Gets a Lightmap
				pShaderShadow->EnableTexture(SHADER_SAMPLER13, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER13, !bHDR);
			}

			// s14
			if (bHasEnvMap)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER14, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER14, !bHDR);
			}

			// Do not set 15. It's used for the Gamma LUT and will make the Texture bound to it randomly flash.
		}

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//
		int nDecalMode = HasFlag(MATERIAL_VAR_DECAL);

		// Overlays require some extra logic on the TF2SDK due to Lightmap Padding
#ifdef TF2SDK
		nDecalMode = nDecalMode * 2;
#endif

		if (bProjectedTexture)
		{
			const char* ccStaticVS = GetString(Shader_ProjTex_VS);
			const char* ccStaticPS = GetString(Shader_ProjTex_PS);

			int nIndexVS = 0;
			int nIndexPS = 0;

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			//
			// Separate VS Indices for Models and Brushes
			if (bIsModel)
			{
				lux_custom_model_vs30_Static_Index vsIndex;
				vsIndex.SetNORMALS(bVertexNormals + bVertexTangents);
				vsIndex.SetVERTEXCOLORS(bVertexRGBA);
				vsIndex.SetBUMPMAPPED(bBumpMapped + (bBumpMapped && GetBool(Shader_WrinkleWeights)));
				vsIndex.SetPROJTEX(true);
				vsIndex.SetDECALMODE(nDecalMode);

				nIndexVS = vsIndex.GetIndex();
			}
			else
			{
				lux_custom_brush_vs30_Static_Index vsIndex;
				vsIndex.SetNORMALS(bVertexNormals + bVertexTangents);
				vsIndex.SetVERTEXCOLORS(bVertexRGBA);
				vsIndex.SetBUMPMAPPED(bBumpMapped);
				vsIndex.SetPROJTEX(true);
				vsIndex.SetDECALMODE(nDecalMode);

				nIndexVS = vsIndex.GetIndex();
			}

			lux_custom_projtex_ps30_Static_Index psIndex;
			psIndex.SetPROJTEXFILTER(g_pHardwareConfig->GetShadowFilterMode());
			psIndex.SetBRUSH(!bIsModel);
			psIndex.SetBUMPMAPPED(bBumpMapped);
			psIndex.SetVERTEXCOLORS(bVertexRGBA);

			nIndexPS = psIndex.GetIndex();

			pShaderShadow->SetVertexShader(ccStaticVS, nIndexVS);
			pShaderShadow->SetPixelShader(ccStaticPS, nIndexPS);
		}
		else if (bParticle)
		{
			const char* ccStaticVS = GetString(Shader_VS);
			const char* ccStaticPS = GetString(Shader_PS);

			int nIndexVS = 0;
			int nIndexPS = 0;

			// Avoid missing Combos on ZoomAnimSeq2
			bool b2ndSequence = GetBool(Shader_Particle_SecondSequence);
			bool bZoomAnimSeq2 = b2ndSequence && (GetFloat(Shader_Particle_ZoomAnimateSeq2) > 1.0f ? true : false);

			bool bAddBase2 = GetBool(Shader_Particle_AddBaseTexture2);
			bool bExtractGreenAlpha = GetBool(Shader_Particle_ExtractGreenAlpha);

			// Make sure this is 0-5
			int nVSAltMode = GetInt(Shader_Particle_VS_Mode);
			nVSAltMode = nVSAltMode >= 0 ? nVSAltMode : 0;
			nVSAltMode = nVSAltMode <= 5 ? nVSAltMode : 5;

			// Make sure this is also 0-5
			int nPSAltMode = GetInt(Shader_Particle_PS_Mode);
			nPSAltMode = nPSAltMode >= 0 ? nPSAltMode : 0;
			nPSAltMode = nPSAltMode <= 5 ? nPSAltMode : 5;

			bool bSplineCard = GetBool(Shader_Particle_Spline);

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			lux_custom_particle_vs30_Static_Index vsIndex;
			vsIndex.SetSPLINE(bSplineCard);
			vsIndex.SetDUALSEQUENCE(b2ndSequence);
			vsIndex.SetZOOM_ANIMATE_SEQ2(bZoomAnimSeq2);
			vsIndex.SetADDBASETEXTURE2(bAddBase2);
			vsIndex.SetEXTRACTGREENALPHA(bExtractGreenAlpha);
			vsIndex.SetALTERNATIVE_MODE(nVSAltMode);
			nIndexVS = vsIndex.GetIndex();

			lux_custom_particle_ps30_Static_Index psIndex;
			psIndex.SetDUALSEQUENCE(b2ndSequence);
			psIndex.SetADDBASETEXTURE2(bAddBase2);
			psIndex.SetEXTRACTGREENALPHA(bExtractGreenAlpha);
			psIndex.SetALTERNATIVE_MODE(nPSAltMode);
			nIndexPS = psIndex.GetIndex();

			pShaderShadow->SetVertexShader(ccStaticVS, nIndexVS);
			pShaderShadow->SetPixelShader(ccStaticPS, nIndexPS);
		}
		else
		{	
			// Using string here because,
			// cc* will turn this into "staticClientDLLToolsPanel" during vcs reloads
			std::string strStaticVS = GetString(Shader_VS);
			std::string strStaticPS = GetString(Shader_PS); 

			int nIndexVS = 0;
			int nIndexPS = 0;

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			//
			// Separate VS Indices for Models and Brushes
			if (bIsModel)
			{
				lux_custom_model_vs30_Static_Index vsIndex;
				vsIndex.SetNORMALS(bVertexNormals + bVertexTangents);
				vsIndex.SetVERTEXCOLORS(bVertexRGBA);
				vsIndex.SetBUMPMAPPED(bBumpMapped + (bBumpMapped && GetBool(Shader_WrinkleWeights)));
				vsIndex.SetPROJTEX(false);
				vsIndex.SetDECALMODE(nDecalMode);

				nIndexVS = vsIndex.GetIndex();
			}
			else
			{
				lux_custom_brush_vs30_Static_Index vsIndex;
				vsIndex.SetNORMALS(bVertexNormals + bVertexTangents);
				vsIndex.SetVERTEXCOLORS(bVertexRGBA);
				vsIndex.SetBUMPMAPPED(bBumpMapped);
				vsIndex.SetPROJTEX(false);
				vsIndex.SetDECALMODE(nDecalMode);

				nIndexVS = vsIndex.GetIndex();
			}

			lux_custom_ps30_Static_Index psIndex;
			psIndex.SetBRUSH(!bIsModel);
			psIndex.SetLIGHTDATA(bIsModel && GetBool(Shader_WorldLightData));
			psIndex.SetAMBIENTCUBES(bIsModel && GetBool(Shader_AmbientData));
			psIndex.SetENVMAPCOMBO(bHasEnvMap + bPCC);
			psIndex.SetBUMPMAPPED(bBumpMapped);
			psIndex.SetVERTEXCOLORS(bVertexRGBA);

			nIndexPS = psIndex.GetIndex();

			pShaderShadow->SetVertexShader(strStaticVS.c_str(), nIndexVS);
			pShaderShadow->SetPixelShader(strStaticPS.c_str(), nIndexPS);
		}
	}

	//==========================================================================//
	// Dynamic State
	//==========================================================================//
	if (pShaderAPI)
	{
		//==========================================================================//
		// Constants
		//==========================================================================//

		// This doesn't use floatx in case someone ports it somewhere else.
		// Unrolled for easy of maintaining

		// c0 to c13 ( 14 Registers, size of 4 )
		float4 cSingleConstant = 0.0f;
		float4 cConstants[14] = { 0.0f };
		cConstants[0] = GetFloat4(psreg_c0);
		cConstants[1] = GetFloat4(psreg_c1);

		// Expose View and Projection Matrix to the Pixel Shader
		if (GetBool(Shader_Matrices))
		{
			// TODO: Which of these Matrices come transposed?
			// View Matrix is transposed, don't know about the others.
			
			// This only works because VMatrix uses vec_t, which is a typedef for float
			// M, V ,P
			pShaderAPI->GetMatrix(MATERIAL_MODEL, cConstants[2]); // c2, c3, c4, c5
			pShaderAPI->GetMatrix(MATERIAL_VIEW, cConstants[6]); // c6, c7, c8, c9
			pShaderAPI->GetMatrix(MATERIAL_PROJECTION, cConstants[10]); // c10, c11, c12, c13
		}
		else
		{
			cConstants[2] = GetFloat4(psreg_c2);
			cConstants[3] = GetFloat4(psreg_c3);
			cConstants[4] = GetFloat4(psreg_c4);
			cConstants[5] = GetFloat4(psreg_c5);
			cConstants[6] = GetFloat4(psreg_c6);
			cConstants[7] = GetFloat4(psreg_c7);
			cConstants[8] = GetFloat4(psreg_c8);
			cConstants[9] = GetFloat4(psreg_c9);
			cConstants[10] = GetFloat4(psreg_c10);
			cConstants[11] = GetFloat4(psreg_c11);
			cConstants[12] = GetFloat4(psreg_c12);
			cConstants[13] = GetFloat4(psreg_c13);
		}

		// Push all of these at once
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, cConstants[0], 14);

		bool bProjectedTextureShadows = false;
		if (bProjectedTexture)
		{
			VMatrix vmWorldToTexture;
			ITexture* pFlashlightDepthTexture;

			// Ex is the 'new' Function that was introduced.. at some Point. The old Function doesn't want ITexture*
			FlashlightState_t FlashlightState = pShaderAPI->GetFlashlightStateEx(vmWorldToTexture, &pFlashlightDepthTexture);

			// If a DepthTexture exists, Shadows.
			// Not sure how g_pConfig->ShadowDepthTexture() feeds into this.
			bProjectedTextureShadows = FlashlightState.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

			if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && FlashlightState.m_bEnableShadows)
			{
				BindTexture(SHADER_SAMPLER13, pFlashlightDepthTexture, 0);
				BindTexture(SHADER_SAMPLER14, TEXTURE_SHADOW_NOISE_2D);
			}
			else
			{
				// Always bind SOMETHING to enabled Samplers!!
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER13, TEXTURE_BLACK);
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER14, TEXTURE_BLACK);
			}

			// Bind ProjectedTexture Cookie
			BindTexture(SHADER_SAMPLER15, FlashlightState.m_pSpotlightTexture, FlashlightState.m_nSpotlightTextureFrame);

			// c14
			cSingleConstant.x = ShadowFilterFromState(FlashlightState);
			cSingleConstant.y = ShadowAttenFromState(FlashlightState);
			HashShadow2DJitter(FlashlightState.m_flShadowJitterSeed, &cSingleConstant.z, &cSingleConstant.w);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_014, cSingleConstant);

			// c15
			cSingleConstant.x = FlashlightState.m_fConstantAtten;
			cSingleConstant.y = FlashlightState.m_fLinearAtten;
			cSingleConstant.z = FlashlightState.m_fQuadraticAtten;
			cSingleConstant.w = FlashlightState.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_015, cSingleConstant);

			// c16
			cSingleConstant.x = FlashlightState.m_vecLightOrigin[0];
			cSingleConstant.y = FlashlightState.m_vecLightOrigin[1];
			cSingleConstant.z = FlashlightState.m_vecLightOrigin[2];
			cSingleConstant.w = 0.0f; // Free..
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_016, cSingleConstant);

			// c17, c18, c19, c20
			// Matrix isn't just used for Shadows, but for ProjTex Cookie too. Ignore bProjectedTextureShadows
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_017, vmWorldToTexture.Base(), 4);

			// c21
			// Water needs a separate Parameter for Boost in Fog
			SetFlashLightColorFromState(FlashlightState, REGISTER_FLOAT_021, GetBool(ProjectedTextureNoLambert));

			// c22
			// "Dimensions of screen, used for screen-space noise map sampling"
			// Resolution of the Noise Texture is 32x32 that's why it does /32 here
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
			cSingleConstant.x = (float)nWidth / 32.0f;
			cSingleConstant.y = (float)nHeight / 32.0f;
			cSingleConstant.z = 0.0f;
			cSingleConstant.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_022, cSingleConstant);
		}
		else
		{
			// c14 - c19
			// Models get LightData, Brushes get 2-6 more Constants or PCC
			if (bIsModel && GetBool(Shader_WorldLightData))
			{
				pShaderAPI->CommitPixelShaderLighting(REGISTER_FLOAT_014);
			}
			else
			{
				cSingleConstant = GetFloat4(psreg_c14);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_014, cSingleConstant);

				cSingleConstant = GetFloat4(psreg_c15);
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_015, cSingleConstant);

				if (bPCC)
				{				
					cConstants[0] = GetFloat4(EnvMapParallaxOBB1);
					cConstants[1] = GetFloat4(EnvMapParallaxOBB2);
					cConstants[2] = GetFloat4(EnvMapParallaxOBB3);
					cConstants[3] = GetFloat4(EnvMapOrigin);
				}
				else
				{
					cConstants[0] = GetFloat4(psreg_c16);
					cConstants[1] = GetFloat4(psreg_c17);
					cConstants[2] = GetFloat4(psreg_c18);
					cConstants[3] = GetFloat4(psreg_c19);
				}
				pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_016, cConstants[0], 4);
			}

			// c20 - c25
			if(GetBool(Shader_AmbientData))
				pShaderAPI->SetPixelShaderStateAmbientLightCube(REGISTER_FLOAT_020);
		}

		// c26
		// LUX Considers $NoTint and $AllowDiffuseModulation within ComputeModulationColor
		// If you are porting this elsewhere, account for it on your End. Thank you.
		// Also I'm doing LightmapScaleFactor later.
		// Multiplying it into DiffuseModulation breaks a bunch of stuff that no one ever bothered to consider
		ComputeModulationColor(cSingleConstant);
		pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_026, cSingleConstant);

		// c27
		if (GetBool(Shader_EyePos))
		{
			pShaderAPI->GetWorldSpaceCameraPosition(cSingleConstant);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_027, cSingleConstant);
		}

		// c28
		if (GetBool(Shader_FogData))
			pShaderAPI->SetPixelShaderFogParams(REGISTER_FLOAT_028);

		// c29 is (apparently) set automatically ( FogColor and OO_DESTALPHA_DEPTH_RANGE )

		// c30 is (apparently) set automatically ( cLightScale )

		// c31
		if (bParticleDepthBlend)
		{
			pShaderAPI->SetDepthFeatheringPixelShaderConstant(REGISTER_FLOAT_031, GetFloat(Shader_Particle_DepthBlendScale));
		}
		else if (bIsModel)
		{
			if (bModelLightmap || bCustomLightmap)
			{
				const int nLightmapVar = bModelLightmap ? Lightmap : Lightmap_Custom;
				ITexture* pTexture = params[nLightmapVar]->GetTextureValue();

				if (pTexture)
				{

					cSingleConstant = { 0.0f };
					cSingleConstant.x = 1.0f / (float)pTexture->GetActualWidth();
					cSingleConstant.y = 1.0f / (float)pTexture->GetActualHeight();

					// Requires modified Compilers ( It is possible however )
					// Same for Bumped Lightmaps below
					/*
					bool bHDRLightmap = pTexture->GetImageFormat() == IMAGE_FORMAT_RGBA16161616F || pTexture->GetImageFormat() == IMAGE_FORMAT_RGBA16161616;
					cSingleConstant.w = bHDRLightmap ? 16.0f : 1.0f;
					*/

					// Bumped Model Lightmap Scale and Offset
					cSingleConstant.z = 1.0f / 3.0f;
					cSingleConstant.w = 0.0f;
					pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_031, cSingleConstant);
				}
			}
		}
		else
		{
			int nWidth, nHeight;
			pShaderAPI->GetLightmapDimensions(&nWidth, &nHeight);
			cSingleConstant.x = 1.0f / (float)nWidth;
			cSingleConstant.y = 1.0f / (float)nHeight;
			cSingleConstant.z = pShaderAPI->GetLightMapScaleFactor();
			cSingleConstant.w = 0.0f; // Free
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_031, cSingleConstant);
		}

		// If this isn't supported and you try to set >c31,
		// the game *will* crash *if* the registers aren't enabled in the ShaderAPI
		// It doesn't matter for setting .vcs, they will just be invisible apparently
		if (g_pHardwareConfig->SupportsShaderModel_3_0())
		{
			// 32 Parameters, Size of 4
			float4 cPixelConstants[32] = { 0.0f };
			cPixelConstants[ 0] = GetFloat4(psreg_c32);
			cPixelConstants[ 1] = GetFloat4(psreg_c33);
			cPixelConstants[ 2] = GetFloat4(psreg_c34);
			cPixelConstants[ 3] = GetFloat4(psreg_c35);
			cPixelConstants[ 4] = GetFloat4(psreg_c36);
			cPixelConstants[ 5] = GetFloat4(psreg_c37);
			cPixelConstants[ 6] = GetFloat4(psreg_c38);
			cPixelConstants[ 7] = GetFloat4(psreg_c39);
			cPixelConstants[ 8] = GetFloat4(psreg_c40);
			cPixelConstants[ 9] = GetFloat4(psreg_c41);
			cPixelConstants[10] = GetFloat4(psreg_c42);
			cPixelConstants[11] = GetFloat4(psreg_c43);
			cPixelConstants[12] = GetFloat4(psreg_c44);
			cPixelConstants[13] = GetFloat4(psreg_c45);
			cPixelConstants[14] = GetFloat4(psreg_c46);
			cPixelConstants[15] = GetFloat4(psreg_c47);
			cPixelConstants[16] = GetFloat4(psreg_c48);
			cPixelConstants[17] = GetFloat4(psreg_c49);
			cPixelConstants[18] = GetFloat4(psreg_c50);
			cPixelConstants[19] = GetFloat4(psreg_c51);
			cPixelConstants[20] = GetFloat4(psreg_c52);
			cPixelConstants[21] = GetFloat4(psreg_c53);
			cPixelConstants[22] = GetFloat4(psreg_c54);
			cPixelConstants[23] = GetFloat4(psreg_c55);
			cPixelConstants[24] = GetFloat4(psreg_c56);
			cPixelConstants[25] = GetFloat4(psreg_c57);
			cPixelConstants[26] = GetFloat4(psreg_c58);
			cPixelConstants[27] = GetFloat4(psreg_c59);
			cPixelConstants[28] = GetFloat4(psreg_c60);
			cPixelConstants[29] = GetFloat4(psreg_c61);
			cPixelConstants[30] = GetFloat4(psreg_c62);
			cPixelConstants[31] = GetFloat4(psreg_c63);
		}

		//==========================================================================//
		// Vertex Shader Constant Registers
		//==========================================================================//

		// Can't check if they are defined. So just force them all.
		SetVertexShaderConstant(LUX_VS_FLOAT_SET0_0, vsreg_c15);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_0, vsreg_c48);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_1, vsreg_c49);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_2, vsreg_c50);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_3, vsreg_c51);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_4, vsreg_c52);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_5, vsreg_c53);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_6, vsreg_c54);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_7, vsreg_c55);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_8, vsreg_c56);
		SetVertexShaderConstant(LUX_VS_FLOAT_SET1_9, vsreg_c57);

		// Always forcing these
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, Texture_Transform0);
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, Texture_Transform1);
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_03, Texture_Transform2);
		SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_04, Texture_Transform3);

		// Very far down. Not sure if these Registers are free on other SDK's
		// They *should* be free on sdk2013sp and sdk2013mp ( TF2SDK ).
		// There are some things like opengl clip planes down there, it isn't well documented.
		// Particles need these Matrices for Orientation
		if (GetBool(Shader_Matrices) || bParticle)
		{
			// Already has cModel Matrix so we only need View and Proj
			VMatrix matView, matViewTranspose;
			pShaderAPI->GetMatrix(MATERIAL_VIEW, matView[0]);
			MatrixTranspose(matView, matViewTranspose);

			// Not a Matrix(?) still here though..
			LoadViewportTransformScaledIntoVertexShaderConstant(REGISTER_FLOAT_231);

			// Still no Register Map for this. Magic Numbers it is!
			pShaderAPI->SetVertexShaderConstant(REGISTER_FLOAT_232, matViewTranspose.m[0], 4); // 232, 233, 234, 235
			LoadProjectionMatrixIntoVertexShaderConstant(REGISTER_FLOAT_236); // 236, 237, 238, 239
			LoadModelViewMatrixIntoVertexShaderConstant(REGISTER_FLOAT_240); // 240, 241, 242, 243
		}

		// Need a bunch of very specific Things for Particles to work correctly
		if (bParticle)
		{
			float f1MaxDistance = GetFloat(Shader_Particle_MaxDistance);
			float flStartFade = f1MaxDistance - GetFloat(Shader_Particle_FarFadeInterval);

			// Clamp to a minimum of 1.0f
			flStartFade = 1.0f > flStartFade ? 1.0f : flStartFade;

			float4 cParticleParams1;
			cParticleParams1.x = GetFloat(Shader_Particle_MinSize);
			cParticleParams1.y = GetFloat(Shader_Particle_MaxSize);
			cParticleParams1.z = GetFloat(Shader_Particle_StartFadeSize);
			cParticleParams1.w = GetFloat(Shader_Particle_EndFadeSize);

			float4 cParticleParams2;
			cParticleParams2.x = flStartFade;
			cParticleParams2.y = 1.0 / (f1MaxDistance - flStartFade); // RCP Precomputed, nice.

			float f1ZoomAnimSeq2 = GetFloat(Shader_Particle_ZoomAnimateSeq2);
			if (f1ZoomAnimSeq2 > 1.0f)
			{
				float f1ZScale = 1.0 / f1ZoomAnimSeq2;
				cParticleParams2.z = 0.5f * (1.0f + f1ZScale);
				cParticleParams2.w = f1ZScale;
			}
			else
			{
				cParticleParams2.z = 0.0f; // Free
				cParticleParams2.w = 0.0f; // Free
			}

			pShaderAPI->SetVertexShaderConstant(REGISTER_FLOAT_244, cParticleParams1);
			pShaderAPI->SetVertexShaderConstant(REGISTER_FLOAT_245, cParticleParams2);
		}

		//==========================================================================//
		// Bind Textures
		//==========================================================================//
		
		if (bTexture0)
			BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);
		else if (bParticleDepthBlend)
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_DEPTH);

		if (bBumpMap1)
			BindTexture(SHADER_SAMPLER1, BumpMap, Texture1_Frame);
		else if (bTexture1)
			BindTexture(SHADER_SAMPLER1, Texture1, Texture1_Frame);

		if (bBumpMap2)
			BindTexture(SHADER_SAMPLER2, BumpMap2, Texture2_Frame);
		else if (bTexture2)
			BindTexture(SHADER_SAMPLER2, Texture2, Texture2_Frame);

		if (bTexture3)
			BindTexture(SHADER_SAMPLER3, Texture3, Texture3_Frame);

		if (bTexture4)
			BindTexture(SHADER_SAMPLER4, Texture4, Texture4_Frame);

		if (bTexture5)
			BindTexture(SHADER_SAMPLER5, Texture5, Texture5_Frame);

		if (bTexture6)
			BindTexture(SHADER_SAMPLER6, Texture6, Texture6_Frame);

		if (bTexture7)
			BindTexture(SHADER_SAMPLER7, Texture7, Texture7_Frame);

		if (bTexture8)
			BindTexture(SHADER_SAMPLER8, Texture8, Texture8_Frame);

		if (bTexture9)
			BindTexture(SHADER_SAMPLER9, Texture9, Texture9_Frame);

		if (bTexture10)
			BindTexture(SHADER_SAMPLER10, Texture10, Texture10_Frame);

		if (bTexture11)
			BindTexture(SHADER_SAMPLER11, Texture11, Texture11_Frame);

		if (bTexture12)
			BindTexture(SHADER_SAMPLER12, Texture12, Texture12_Frame);

		// s13
		if (!bIsModel)
		{
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER13, TEXTURE_LIGHTMAP);
		}
		else if (bIsModel)
		{
			// Always need to bind something here, the Sampler is always enabled since $Lightmap is only set in Dynamic State
			if (bCustomLightmap)
				BindTexture(SHADER_SAMPLER13, Lightmap_Custom);
			else if (bModelLightmap)
				BindTexture(SHADER_SAMPLER13, Lightmap);
			else
				BindTexture(SHADER_SAMPLER13, TEXTURE_BLACK);
		}

		// s14
		if (bHasEnvMap)
			BindTexture(SHADER_SAMPLER14, EnvMap, EnvMapFrame);

		// Need to recompute AlphaWrites here.
		// All of these Things disable AlphaWrites
		int nAlphaDisable = 0;
		nAlphaDisable += GetBool(Shader_AlphaBlending_Enable);
		nAlphaDisable += bProjectedTexture;
		nAlphaDisable += bTranslucent;
		nAlphaDisable += bAdditive;
		nAlphaDisable += bAlphaTest;
		nAlphaDisable += GetBool(Shader_Disable_AlphaWrites);

		bool bAlphaWrites = (nAlphaDisable == 0) ? true : false;
		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;

		int nPixelFogMode = 0;
		if (!HasFlag(MATERIAL_VAR_NOFOG) && GetBool(Shader_FogData))
		{
#ifdef TF2SDK_MODE
			nPixelFogMode = pShaderAPI->GetPixelFogCombo1(true);
#else
			nPixelFogMode = pShaderAPI->GetPixelFogCombo();
#endif
			
		}

		MaterialFogMode_t FogType = pShaderAPI->GetSceneFogMode();
		if (bAlphaWrites)
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();

			// nPixelFogMode == 1 will be true when something is Underwater, making it very useful. ( Height Fog )
			// You can manually check WorldPos.z < WaterZ but that will be true *outside* of Water Volumes.
			bWriteWaterFogToAlpha = (FogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) || nPixelFogMode == 1;
		}

		//==========================================================================//
		// Boolean Constant Registers
		//==========================================================================//

		// Prepare boolean array, yes this needs to use Microsoft's BOOL Typedef
		BOOL BBools[16] = { false };

		BBools[0] = GetBool(psreg_b0);
		BBools[1] = GetBool(psreg_b1);
		BBools[2] = GetBool(psreg_b2);
		BBools[3] = GetBool(psreg_b3);
		BBools[4] = GetBool(psreg_b4);
		BBools[5] = GetBool(psreg_b5);
		BBools[6] = GetBool(psreg_b6);
		BBools[7] = GetBool(psreg_b7);
		BBools[8] = GetBool(psreg_b8);
		BBools[9] = GetBool(psreg_b9);
		BBools[10] = GetBool(psreg_b10);
		BBools[11] = GetBool(psreg_b11);

		// Always these
		BBools[12] = pShaderAPI->InEditorMode();

		// Projected Textures disappear into the Fog!
//		if (!bProjectedTexture)
		if (true)
		{
			BBools[13] = bAlphaWrites && bWriteWaterFogToAlpha;
			BBools[14] = HasRadialFog();

			// Never try writing Depth to DestAlpha, disabled on the Shader also
			BBools[15] = bAlphaWrites && bWriteDepthToAlpha;
		}

		pShaderAPI->SetBooleanPixelShaderConstant(0, BBools, 16);

		//==========================================================================//
		// Some Model specific Shenanigans
		//==========================================================================//
		bool bHasStaticPropLighting = false;
		bool bHasDynamicPropLighting = false;
		int nNum_Lights = 0;
		bool bVertexCompression = (vertexCompression == VERTEX_COMPRESSION_ON);
		if (bIsModel)
		{
			LightState_t LightState;
			pShaderAPI->GetDX9LightState(&LightState);

			nNum_Lights = LightState.m_nNumLights;

			if (!bBumpMapped)
			{
				bHasStaticPropLighting = StaticLightVertex(LightState); // Name of the Constant different between SDK's
				bHasDynamicPropLighting = LightState.HasDynamicLight();

				// Need to send this to the Vertex Shader manually in this Scenario
				if (bHasDynamicPropLighting)
					pShaderAPI->SetVertexShaderStateAmbientLightCube();
			}
		}

		//==========================================================================//
		// Set Dynamic Shaders
		//==========================================================================//
		int nDecalMode = HasFlag(MATERIAL_VAR_DECAL);

		// Overlays require some extra logic on the TF2SDK due to Lightmap Padding
#ifdef TF2SDK
		nDecalMode = nDecalMode * 2;
#endif

		if (bProjectedTexture)
		{
			int nIndexVS = 0;
			int nIndexPS = 0;

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			//
			// Separate VS Indices for Models and Brushes
			if (bIsModel)
			{
				lux_custom_model_vs30_Dynamic_Index vsIndex;
				vsIndex.SetCOMPRESSION(bVertexCompression);
				vsIndex.SetSKINNING((pShaderAPI->GetCurrentNumBones() > 0) ? 1 : 0);
				vsIndex.SetSTATICPROPLIGHTING(0); // Avoid missing combo
				vsIndex.SetDYNAMICPROPLIGHTING(0);

				nIndexVS = vsIndex.GetIndex();
			}
			else
			{
				// No Dyn Combos so far
				lux_custom_brush_vs30_Dynamic_Index vsIndex;

				nIndexVS = vsIndex.GetIndex();
			}

			// Only one Combo
			lux_custom_projtex_ps30_Dynamic_Index psIndex;
			psIndex.SetPROJTEXSHADOWS(bProjectedTextureShadows);
			nIndexPS = psIndex.GetIndex();

			pShaderAPI->SetVertexShaderIndex(nIndexVS);
			pShaderAPI->SetPixelShaderIndex(nIndexPS);
		}
		else if (bParticle)
		{
			int nIndexVS = 0;
			int nIndexPS = 0;

			// No Orientation for SplineCard
			int nOrientation = GetInt(Orientation);
			if (GetBool(Shader_Particle_Spline))
				nOrientation = 0;

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			//
			// Separate VS Indices for Models and Brushes
			lux_custom_particle_vs30_Dynamic_Index vsIndex;
			vsIndex.SetORIENTATION(nOrientation);
			nIndexVS = vsIndex.GetIndex();

			lux_custom_particle_ps30_Dynamic_Index psIndex;
			nIndexPS = psIndex.GetIndex();

			pShaderAPI->SetVertexShaderIndex(nIndexVS);
			pShaderAPI->SetPixelShaderIndex(nIndexPS);
		}
		else
		{
			int nIndexVS = 0;
			int nIndexPS = 0;

			// We manually compute the Index here.
			// Do *not* expose the Index. A Licensee told me there is a security Issue with exposing it on SDK2013SP
			// TF2SDK no longer has this Issue apparently, but they didn't go into specifics.
			// Forcing a specific Index is safe!
			//
			// Separate VS Indices for Models and Brushes
			if (bIsModel)
			{
				lux_custom_model_vs30_Dynamic_Index vsIndex;
				vsIndex.SetCOMPRESSION(bVertexCompression);
				vsIndex.SetSKINNING((pShaderAPI->GetCurrentNumBones() > 0) ? 1 : 0);
				vsIndex.SetSTATICPROPLIGHTING(bHasStaticPropLighting);
				vsIndex.SetDYNAMICPROPLIGHTING(bHasDynamicPropLighting);
				nIndexVS = vsIndex.GetIndex();
			}
			else
			{
				// No Dyn Combos so far
				lux_custom_brush_vs30_Dynamic_Index vsIndex;
				nIndexVS = vsIndex.GetIndex();
			}

			// Two Combos
			lux_custom_ps30_Dynamic_Index psIndex;
			psIndex.SetNUM_LIGHTS(nNum_Lights);
			psIndex.SetLIGHTMAPPED_MODEL(bModelLightmap || bCustomLightmap);
			nIndexPS = psIndex.GetIndex();

			pShaderAPI->SetVertexShaderIndex(nIndexVS);
			pShaderAPI->SetPixelShaderIndex(nIndexPS);
		}
	}

	Draw();
}
END_SHADER