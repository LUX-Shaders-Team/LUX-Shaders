//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//==========================================================================//

#ifndef BASEVSSHADER_H
#define BASEVSSHADER_H

#ifdef _WIN32
#pragma once
#endif

// No longer uses shaderlib/ as a reference as it would point to the public/ headers with virtual functions
// Now uses the actual headers!
#include "../shaderlib/cshader.h"
#include "../shaderlib/BaseShader.h"

#include "convar.h"
#include <renderparm.h>

// LUX
#include "lux_common_defines.h"
#include "cpp_floatx.h"

//==========================================================================//
// Base Class for Shaders, contains Helper Methods.
//==========================================================================//
class CBaseVSShader : public CBaseShader
{
public:

	// Loads bump lightmap coordinates into the pixel shader
	void LoadBumpLightmapCoordinateAxes_PixelShader( int pixelReg );

	// Loads bump lightmap coordinates into the vertex shader
	void LoadBumpLightmapCoordinateAxes_VertexShader( int vertexReg );

	// Pixel and vertex shader constants....
	void SetPixelShaderConstant( int pixelReg, int constantVar );

	// Pixel and vertex shader constants....
	void SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar );

	// This version will put constantVar into x,y,z, and constantVar2 into the w
	void SetPixelShaderConstant( int pixelReg, int constantVar, int constantVar2 );
	void SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar, int constantVar2 );

	// Helpers for setting constants that need to be converted to linear space (from gamma space).
	void SetVertexShaderConstantGammaToLinear( int var, float const* pVec, int numConst = 1, bool bForce = false );
	void SetPixelShaderConstantGammaToLinear( int var, float const* pVec, int numConst = 1, bool bForce = false );

	void SetVertexShaderConstant( int vertexReg, int constantVar );

	// set rgb components of constant from a color parm and give an explicit w value
	void SetPixelShaderConstant_W( int pixelReg, int constantVar, float fWValue );

	// GR - fix for const/lerp issues
	void SetPixelShaderConstantFudge( int pixelReg, int constantVar );

	// Sets light direction for pixel shaders.
	void SetPixelShaderLightColors( int pixelReg );

	// Sets vertex shader texture transforms
	void SetVertexShaderTextureTranslation( int vertexReg, int translationVar );
	void SetVertexShaderTextureScale( int vertexReg, int scaleVar );
 	void SetVertexShaderTextureTransform( int vertexReg, int transformVar );
	void SetVertexShaderTextureScaledTransform( int vertexReg, 
											int transformVar, int scaleVar );

	// Set pixel shader texture transforms
	void SetPixelShaderTextureTranslation( int pixelReg, int translationVar );
	void SetPixelShaderTextureScale( int pixelReg, int scaleVar );
 	void SetPixelShaderTextureTransform( int pixelReg, int transformVar );
	void SetPixelShaderTextureScaledTransform( int pixelReg, 
											int transformVar, int scaleVar );

	// Moves a matrix into vertex shader constants 
	void SetVertexShaderMatrix3x4( int vertexReg, int matrixVar );
	void SetVertexShaderMatrix4x4( int vertexReg, int matrixVar );

	// Loads the view matrix into vertex shader constants
	void LoadViewMatrixIntoVertexShaderConstant( int vertexReg );

	// Loads the projection matrix into vertex shader constants
	void LoadProjectionMatrixIntoVertexShaderConstant( int vertexReg );

	// Loads the model->view matrix into vertex shader constants
	void LoadModelViewMatrixIntoVertexShaderConstant( int vertexReg );

	// Loads a scale/offset version of the viewport transform into the specified constant.
	void LoadViewportTransformScaledIntoVertexShaderConstant( int vertexReg );

	// Sets up ambient light cube...
	void SetAmbientCubeDynamicStateVertexShader( );
	float GetAmbientLightCubeLuminance( );

	// Helpers for dealing with envmaptint
	void SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar, int alphaVar, bool bConvertFromGammaToLinear = false );
	
	// Helper methods for pixel shader overbrighting
	void EnablePixelShaderOverbright( int reg, bool bEnable, bool bDivideByTwo );

	// Helper for dealing with modulation
	void SetModulationVertexShaderDynamicState();
	void SetModulationPixelShaderDynamicState( int modulationVar );
	void SetModulationPixelShaderDynamicState_LinearColorSpace( int modulationVar );
	void SetModulationPixelShaderDynamicState_LinearColorSpace_LinearScale( int modulationVar, float flScale );

	BlendType_t EvaluateBlendRequirements( int textureVar, bool isBaseTexture, int detailTextureVar = -1 );

	void HashShadow2DJitter( const float fJitterSeed, float *fU, float* fV );

	//Alpha tested Materials can end up leaving garbage in the dest alpha buffer if they write depth. 
	//This pass fills in the areas that passed the alpha test with depth in dest alpha 
	//by writing only equal depth pixels and only if we should be writing depth to dest alpha
	void DrawEqualDepthToDestAlpha( void );
	
	// Stock Functions
	// I changed the default Register Index here but it's not using the LUX_PS_FLOAT_PROJTEX_COLOR Macro!!
	void SetFlashLightColorFromState(FlashlightState_t const &state, int nPSRegister = 19, bool bFlashlightNoLambert = false);
	float ShadowAttenFromState( FlashlightState_t const &state );
	float ShadowFilterFromState( FlashlightState_t const &state );

	//==========================================================================//
	// LUX ADDITIONS
	//==========================================================================//
public:
	// Writes a Message to the Console.
	// The Material Name will be written in Orange 
	// After it, the Message will be written in Red
	// For Example ShaderDebugMessage("uses $Parameter with Invalid Value\n")
	// Will turn into "MaterialName uses $Parameter with Invalid Value\n"
	void ShaderDebugMessage(const char* pMessage);

	// Gets the Value from the lux_general_gamma ConVar
	float GetGammaValue();

	// Gets the Value from the lux_general_luminanceweights ConVars
	float3 GetLuminanceWeights();

	// Sets the Camera Position to a Register
	// Does not set a .w
	void SetPixelShaderCameraPosition(int nRegister);

	// Gets Values from ConVars and sets them to the given Register
	// If you need specific Values, manually pack to a float4
	void SetLuminanceGammaConstant(int nRegister);

	// Custom function to return the StaticLightVertex bool
	// The LightState_t struct varies between sdk2013sp and sdk2013mp
	// Avoids replacing existing Data in Headers outside of the MaterialSystem Folder (Such as a Custom Struct with a universal Name)
	bool StaticLightVertex(LightState_t &LightState);

	// This handles flashlight samplers, fog and blending state
	// var input is the texture that contains transparency in the alpha channel ( usually $BaseTexture )
	void SetupFlashlightSamplers();

	// Bind Samplers and send constants to the shader for the flashlight.
	// This is uniform on LUX, which is why we can do this in one function
	// Returns bFlashlightShadow and doesn't do anything if not in flashlightmode
	bool SetupFlashlight();

	// Compute the mipmap count of a texture
	// This Function assumes a *square* Texture
	int GetMipCount(const int var);

	// ComputeModulationColor is part of the BaseShader that is not in the sdk ( but in the orange box )
	// This function extends its functionality a little and makes it more in line with other LUX requirements
	// $Alpha is handled in SetupDefaultRegisters instead
	// var_Alpha will put that parameters float value into the .w component of the tint
	float4 ComputeTint(const bool bAllowDiffuseModulation, const int var_Alpha);

	// Computes LightmapScaleFactor * SSBumpMathFix and $Alpha * $Alpha2
	float4 GetModulationConstant(const bool bBrush, const bool bSSBumpMathFix);

	// Sets float4(LightmapScaleFactor, AlphaModulation) to c1
	// This is used in SetupDefaultRegisters()
	// Exposed as separate function as some shaders may want to set their own tint variables
	void SetModulationConstant(const bool bSSBumpMathFix = false, const bool bBrush = true);

	// Setup all the default things ( tint, lightmap factor, fog, eyepos )
	void SetupDefaultRegisters(const bool bFog, const bool bEyePos, const int var_Alpha = Alpha, const bool bSSBumpMathFix = false);

	// Only write Anything to Alpha, if we aren't transparent
	bool WriteDepthToDestAlpha(const bool bIsOpaque);
	bool WriteWaterFogToDestAlpha(const bool bIsOpaque);

	// Ported and Modified from Orange Box BaseShader Code
	// This one doesn't require 'params' to be passed on
	bool HasFlashlight();

	// Function to get the Flashlight Shadow Filter to use
	// Has a ConVar to force Nvidia PCF, as sometimes good gpu get forced to use bad filters.
	// ShiroDkxtro2: My AMD RX570 is supposed to use ATI_NO_PCF_FETCH4, like yeah no forget it.
	int GetDesiredShadowFilter();

	// Check if a transform is not an identity matrix
	// But only if the texture with it exists
	// var is the MATRIX parameter to check
	bool HasTransform(const bool bTexture, const int var);

	bool HasRadialFog();

	// We precompute a bunch of things into the detailtint and factor so we have to do less math on the GPU
	float4 PrecomputeDetail(const float4 &f4Tint_Factor, const int nBlendMode);
	
	// Returns current Framebuffer or Rendertarget Size
	// Useful because sometimes the Rendertarget Size is NOT the same as the Backbuffer Size.
	// 12.01.2026 NOTE: This Function does not work correctly because of nullptr ITexture* 's
	float2 GetCurrentRenderTargetSize() const;

	// Definitive Function that considers pretty much everything in determining Transparency
	// Including DetailBlendMode 3, 8 and 9 and
	// whether or not in a Flashlight Pass and thus requiring additive blending.
	// Pass -1 for when the Texture is not valid, for Blendmode use actual Blendmode
	BlendType_t ComputeBlendType(int nBaseTextureVar, bool bIsBaseTexture, int nDetailTextureVar = -1, int nDetailBlendMode = -1);

	// Enables Blending from the BlendType_t and handles AlphaTest
	void EnableTransparency(BlendType_t nBlendType);

	// Function above only handles $Additive, $Translucent and Flashlight
	// This one will return what was previously considered bIsFullyOpaque
	// which would be: bIsOpaque && !bAlphatest
	// This is essentially "WritesNoAlpha?", and can be used with WriteDepthToDestAlpha() and WriteWaterFogToDestAlpha()
	bool IsFullyOpaque(BlendType_t nBlendType);
	
	// Shouldn't be seen elsewhere
	private:

	// Used for reinterpreting the LightState for bStaticLight
	// The LightState definition varies between SDK2013SP and SDK2013MP
	// Reinterpreting avoids Compile Errors and #ifdef's
	struct LightState_Universal_t
	{
		int  Offset_NumLights;
		bool Offset_AmbientLightBool;
		bool bStaticLightVertex;
	//	bool bStaticLightTexel;		// On SDK2013MP
	};
};
#endif // BASEVSSHADER_H
