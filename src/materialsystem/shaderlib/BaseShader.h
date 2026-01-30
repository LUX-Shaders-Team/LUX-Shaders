//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all shaders inherit from.
//===========================================================================//

#ifndef BASESHADER_H
#define BASESHADER_H

#ifdef _WIN32		   
#pragma once
#endif

#include "materialsystem/IShader.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "convar.h"
#include "ProxyShaderShadow.h"
#include "ProxyShaderAPI.h"

// Helper Functions ( including FloatX ones ) were moved to CBaseShader
// So we need this include
#include "../stdshaders/cpp_floatx.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define MATERIALSYSTEM_DLL_NAME "materialsystem"
#define FILESYSTEM_DLL_NAME "filesystem_stdio"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
extern bool g_bSupressShaderWarnings;
extern bool g_bHammerPlusPlus;
extern bool g_bWaterAlienSwarmFogFactor;

//-----------------------------------------------------------------------------
// Helper Function for dll loading
//-----------------------------------------------------------------------------
template<typename T>
inline T *LoadInterface( const char *module, const char *version, Sys_Flags flags = SYS_NOFLAGS )
{
    CSysModule *pModule = Sys_LoadModule( module, flags );
    if ( pModule != nullptr )
    {
        CreateInterfaceFn factory = Sys_GetFactory( pModule );
        if ( factory != nullptr )
        {
            return reinterpret_cast<T *>( factory( version, nullptr ) );
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterialVar;

//-----------------------------------------------------------------------------
// Standard Material vars
//-----------------------------------------------------------------------------
// Note: if you add to these, add to s_StandardParams in CBaseShader.cpp
enum ShaderMaterialVars_t
{
	Flags = 0,
	Flags_Defined,
	Flags2,
	Flags_Defined2,
	Color1,
	Alpha,
	BaseTexture,
	Frame,
	BaseTextureTransform,
	FlashlightTexture,
	FlashlightTextureFrame,
	Color2,
	sRGBTint,
	Alpha2,
	AlphaTestReference,
	ReceiveProjectedTextures,
	ProjectedTextureNoLambert,
	AllowDiffuseModulation,
	NoTint,
	DynamicNoDraw,
	Debug_True,

	NUM_SHADER_MATERIAL_VARS
};

// Alpha belnd mode enums. Moved from basevsshader
enum BlendType_t
{
	// no alpha blending
	BT_NONE = 0,

	// src * srcAlpha + dst * (1-srcAlpha)
	// two passes for HDR:
	//		pass 1:
	//			color: src * srcAlpha + dst * (1-srcAlpha)
	//			alpha: srcAlpha * zero + dstAlpha * (1-srcAlpha)
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	//
	// ShiroDkxtro2: $Translucent
	BT_BLEND,

	// src * one + dst * one
	// one pass for HDR
	//
	// ShiroDkxtro2: $Additive, used by projected Textures for Example.
	BT_ADD,

	// Why do we ever use this instead of using premultiplied alpha?
	// src * srcAlpha + dst * one
	// two passes for HDR
	//		pass 1:
	//			color: src * srcAlpha + dst * one
	//			alpha: srcAlpha * one + dstAlpha * one
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	//
	// ShiroDkxtro2: $Additive but with Result * Alpha.
	// The best Example for why to do this is Translucent Decals receiving a projected Texture.
	// proj. Textures use BT_ADD and it will cause transparent Areas of a Decal/Overlay to become White (visible when they shouldn't).
	BT_BLENDADD

	// Could add some new ones here..
};

// ShiroDkxtro2: Used by the CommandBuffer/CommandBuilder.
// Allows Shaders to store Data specific to their Materials in the Context Data
// ( Draw Function Only )
// Useful for cutting down on the performance Impact of a Shader!
class CBasePerMaterialContextData								// shaders can keep per material data in classes descended from this
{
 public:
	uint32 m_nVarChangeID;
	bool m_bMaterialVarsChanged;							// set by mat system when material vars change. shader should rehtink and then clear the var

	FORCEINLINE CBasePerMaterialContextData( void )
	{
		m_bMaterialVarsChanged = true;
		m_nVarChangeID = 0xffffffff;
	}

	// virtual destructor so that derived classes can have their own data to be cleaned up on
	// delete of material
	virtual ~CBasePerMaterialContextData( void )
	{
	}
};

class LUXPerMaterialContextData : public CBasePerMaterialContextData
{
public:
	bool m_bSnapshottingCommands;;
	
	FORCEINLINE LUXPerMaterialContextData()
	{
		m_bSnapshottingCommands = true;
	}
};

//-----------------------------------------------------------------------------
// Base class for Shaders, contains Helper Methods.
//-----------------------------------------------------------------------------
class CBaseShader : public IShader
{
public:
	// Constructor
	CBaseShader();

	// Methods inherited from IShader
	virtual char const* GetFallbackShader( IMaterialVar** params ) const { return 0; }
	virtual int GetNumParams( ) const;
	virtual char const* GetParamName( int paramIndex ) const;
	virtual char const* GetParamHelp( int paramIndex ) const;
	virtual ShaderParamType_t GetParamType( int paramIndex ) const;
	virtual char const* GetParamDefault( int paramIndex ) const;
	virtual int GetParamFlags( int nParamIndex ) const;

	virtual void InitShaderParams( IMaterialVar** ppParams, const char *pMaterialName );
	virtual void InitShaderInstance( IMaterialVar** ppParams, IShaderInit *pShaderInit, const char *pMaterialName, const char *pTextureGroupName );
	virtual void DrawElements( IMaterialVar **params, int nModulationFlags, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI,
								VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContext );

	virtual	const SoftwareVertexShader_t GetSoftwareVertexShader() const { return m_SoftwareVertexShader; }

	virtual int ComputeModulationFlags( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI );
	virtual bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame = true ) const;
	virtual bool NeedsFullFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame = true ) const;
	virtual bool IsTranslucent( IMaterialVar **params ) const;

	// These functions must be implemented by the shader
	virtual void OnInitShaderParams( IMaterialVar** ppParams, const char *pMaterialName ) {}
	virtual void OnInitShaderInstance( IMaterialVar** ppParams, IShaderInit *pShaderInit, const char *pMaterialName ) = 0;
	virtual void OnDrawElements( IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr ) = 0;

	// Sets the default shadow state
	void SetInitialShadowState( );
 
	// Draws a snapshot
	void Draw( bool bMakeActualDrawCall = true );

	// Are we currently taking a Snapshot?
	bool IsSnapshotting() const;

	// "Gets at the current Materialvar flags"
	// ( Returns the int of Flags )
	int CurrentMaterialVarFlags() const;

	// Finds a particular Parameter	(works because the lowest Parameters match the Shader)
	int FindParamIndex( const char *pName ) const;

	// "Are we using graphics?"
	// ShiroDkxtro2: This does not work in the Fallback Stage of a Shader!!
	// Also, this will be true for Example when a Compiler is running
	bool IsUsingGraphics();

	// Are we using Editor Materials?
	bool CanUseEditorMaterials();

	// "Gets the builder..."
	// ShiroDkxtro2: This appears to be non-functional, likely a DX8 Leftover.
	CMeshBuilder* MeshBuilder();

	// Loads a Texture
	void LoadTexture( int nTextureVar, int nAdditionalCreationFlags = 0 );

	// Loads a Bumpmap
	void LoadBumpMap( int nTextureVar );

	// Loads a Cubemap
	// NOTE: Additional Creation Flags only work *if* the Parameter is NOT set to env_cubemap
	// Cubemaps on a BSP File are likely loaded before Shaders are initiated,
	// if you want a Flag you have to add it to the .vtf's in the .bsp File itself!!
	void LoadCubeMap( int nTextureVar, int nAdditionalCreationFlags = 0  );

	// "get the shaderapi handle for a texture. BE CAREFUL WITH THIS."
	// ShiroDkxtro2: This is used internally and for the CommandBuffer/CommandBuilder
	// There don't appear to be sources for ShaderAPITextureHandle_t, so this is useless for us.
	ShaderAPITextureHandle_t GetShaderAPITextureBindHandle( int nTextureVar, int nFrameVar, int nTextureChannel = 0 );

	// Binds a Texture
	// Second Samplers here are dead, I was told this was once used by some procedually generated Textures
//	void BindTexture( Sampler_t sampler1, Sampler_t sampler2, int nTextureVar, int nFrameVar = -1 );
	void BindTexture( Sampler_t sampler1, int nTextureVar, int nFrameVar = -1 );
	void BindTexture( Sampler_t sampler1, ITexture *pTexture, int nFrame = 0 );
//	void BindTexture( Sampler_t sampler1, Sampler_t sampler2, ITexture *pTexture, int nFrame = 0 );

	// Is the Texture translucent?
	// ShiroDkxtro2: I wouldn't trust this if I were you.
	// params[textureVar]->GetTextureValue()->IsTranslucent() is pretty bogus
	// If you want to make sure. Check for $Translucent etc
	// Then see if the Texture has the TEXTUREFLAGS_EIGHTBITALPHA Flag
	// It's set even for RGBA16161616F Textures
	// We keep and use this for Compatability with existing Textures only.
	bool TextureIsTranslucent( int textureVar, bool isBaseTexture );

	// "Returns the translucency..."
	// 
	float GetAlpha( IMaterialVar** params = NULL );

	// Is the color var white?
	bool IsWhite( int colorVar );

	// Helper methods for fog
	void FogToOOOverbright( void );
	void FogToWhite( void );
	void FogToBlack( void );
	void FogToGrey( void );
	void FogToFogColor( void );
	void DisableFog( void );
	void DefaultFog( void );
	
	// Helpers for alpha blending
	void EnableAlphaBlending( ShaderBlendFactor_t src, ShaderBlendFactor_t dst );
	void DisableAlphaBlending();

	void SetBlendingShadowState( BlendType_t nMode );

	void SetNormalBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );
	void SetAdditiveBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );
	void SetDefaultBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );

	// Helpers for color modulation
	void SetColorState( int colorVar, bool setAlpha = false );
	bool IsAlphaModulating();
	bool IsColorModulating();
	void ComputeModulationColor( float* color );
	void SetModulationShadowState( int tintVar = -1 );
	void SetModulationDynamicState( int tintVar = -1 );

	// Loads the identity matrix into the texture
	void LoadIdentity( MaterialMatrixMode_t matrixMode );

	// Loads the camera to world transform
	void LoadCameraToWorldTransform( MaterialMatrixMode_t matrixMode );
	void LoadCameraSpaceSphereMapTransform( MaterialMatrixMode_t matrixMode );

	// Both UsingFlashlight and UsingEdictor only work in the Draw Stage of the Shader
	bool UsingFlashlight() const;
	bool UsingEditor() const;

	void GetColorParameter( IMaterialVar** params, float *pColorOut ) const; // return tint color (color*color2)
	void ApplyColor2Factor( float *pColorOut ) const;		// (*pColorOut) *= Color2

	// These used to be Statics but it broke some Stuff,
	// they shouldn't be statics anyways!
	const char* m_pTextureGroupName = NULL;
	mutable IMaterialVar** m_ppParams = NULL;
	IShaderInit* m_pShaderInit = NULL;
	IShaderShadow* m_pShaderShadow = NULL;
	IShaderDynamicAPI* m_pShaderAPI = NULL;

	int m_nModulationFlags = 0;
	CMeshBuilder* m_pMeshBuilder = NULL;
	VertexCompressionType_t m_nVertexCompression = VERTEX_COMPRESSION_NONE;
	
	// This requires modification of CShader.h
	// This has to be filled in by the Shader Init Macro!
	virtual char const* GetDescription() = 0; // Wasn't exposed before ..
	virtual char const* GetSupportedGeometry() = 0;
	virtual char const* GetUsage() = 0;
	virtual char const* GetLimitations() = 0;
	virtual char const* GetPerformance() = 0;
	virtual char const* GetFallback() = 0;
	virtual char const* GetWebLinks() = 0;
	virtual char const* GetD3DInfo() = 0;

	const char* CurrentMaterialName() const;
	const char* CurrentShaderName() const;

	// For fixing the Half Pixel Offset Issue
	void SetHPOFixConstant();

	//========================//
	// Parameter Helpers
	//========================//
	
	//==============//
	// Checks
	//==============//

	// This does not work for Parameters only used in Proxies!!!
	// They get set by the Proxy so they don't count as defined!!
	inline bool IsDefined(const int var) const
	{
		return m_ppParams[var]->IsDefined();
	}

	// Whether a Texture Parameter has been loaded
	// This just returns if the Parameter Type is a Texture Type
	// Before loading a Texture, a Texture Type Parameters is treated as a String Type instead.
	inline bool IsTextureLoaded(const int var) const
	{
		return m_ppParams[var]->IsTexture();
	}

	//==============//
	// Get Values
	//==============//
	inline bool GetBool(const int var) const
	{
		return (m_ppParams[var]->GetIntValue() != 0);
	}

	inline int GetInt(const int var) const
	{
		return m_ppParams[var]->GetIntValue();
	}

	inline float GetFloat(const int var) const
	{
		return m_ppParams[var]->GetFloatValue();
	}

	inline bool MatrixIsIdentity(const int var) const
	{
		return m_ppParams[var]->MatrixIsIdentity();
	}

	// floatx interface
	float2 GetFloat2(const int var);
	float3 GetFloat3(const int var);
	float4 GetFloat4(const int var);
	// No Overloads for non floatx

	const char*	GetString(const int var);
	ITexture*	GetTexture(const int var);
	const char* GetTextureName(const int var);
	const char* GetTextureName(const ITexture* pTexture); // overload

	//==============//
	// Set Values
	//==============//
	void SetString(const int var, const char* string);
	void SetUndefined(const int var);
	void SetBool(const int var, const bool value);
	void SetInt(const int var, const int value);
	void SetFloat(const int var, const float value);
	
	// floatx interface
	void SetFloat2(const int var, const float2 &value);
	void SetFloat3(const int var, const float3 &value);
	void SetFloat4(const int var, const float4 &value);

	// Overloads for non floatx
	void SetFloat2(const int var, const float x, const float y);
	void SetFloat3(const int var, const float x, const float y, const float z);
	void SetFloat4(const int var, const float x, const float y, const float z, const float w);

	//==============//
	// Defaults
	//==============//
	void DefaultBool(const int var, const bool value);
	void DefaultInt(const int var, const int value);
	void DefaultFloat(const int var, const float value);

	// floatx interface
	void DefaultFloat2(const int var, const float2 &value);
	void DefaultFloat3(const int var, const float3 &value);
	void DefaultFloat4(const int var, const float4 &value);

	// Overloads for non floatx
	void DefaultFloat2(const int var, const float x, const float y);
	void DefaultFloat3(const int var, const float x, const float y, const float z);
	void DefaultFloat4(const int var, const float x, const float y, const float z, const float w);

	//========================//
	// Shader Helpers
	//========================//
	bool IsDynamicState() const { return m_pShaderAPI != nullptr; }

	void EnableSampler(const Sampler_t nSampler, const bool bSRGB);
	void EnableSampler(const bool bCheck, const Sampler_t nSampler, const bool bSRGB);

	// Overloads to regular BindTexture
//	void BindTexture(Sampler_t nSampler, const int var, const int framevar = 0); // Already a CBaseShader Function
//	void BindTexture(Sampler_t nSampler, ITexture* var, const int framevar = 0); // ^
	void BindTexture(const bool bCheck, Sampler_t nSampler, ITexture* var, const int framevar = -1);
	void BindTexture(const bool bCheck, Sampler_t nSampler, const int var, const int framevar = -1);
	void BindTexture(const bool bCheck, Sampler_t nSampler, const int var, const int framevar, StandardTextureId_t StandardTexture);
	void BindTexture(const bool bCheck, Sampler_t nSampler, StandardTextureId_t StandardTexture);
	void BindTexture(Sampler_t nSampler, StandardTextureId_t StandardTexture);

	// Converts GammaToLinear, except when Tint is > 1.0f 
	inline float3 GammaToLinearTint(float3 Tint)
	{
		return float3(
			Tint.r = Tint.r > 1.0f ? Tint.r : GammaToLinear(Tint.r),
			Tint.g = Tint.g > 1.0f ? Tint.g : GammaToLinear(Tint.g),
			Tint.b = Tint.b > 1.0f ? Tint.b : GammaToLinear(Tint.b)
		);
	}

	// CShader.h Functions check an Input Variable (ppParams)
	// CShader.h Macros use very specifically *params*
	// Exposing *params* to every Function just so the macro works is kinda weird
	// We can just check m_ppParams!
	inline bool HasFlag(MaterialVarFlags_t Flag) const
	{
		// Lifted from CShader.h
		return (m_ppParams[Flags]->GetIntValue() & Flag) != 0;
	}
	inline bool HasFlag(IMaterialVar** Params, MaterialVarFlags_t Flag) const // Safe for Multithreading
	{
		return (Params[Flags]->GetIntValue() & Flag) != 0;
	}
	inline bool HasFlag2(MaterialVarFlags2_t Flag2) const
	{
		// Lifted from CShader.h
		return (m_ppParams[Flags2]->GetIntValue() & Flag2) != 0;
	}
	inline bool HasFlag2(IMaterialVar** Params, MaterialVarFlags2_t Flag) const // Safe for Multithreading
	{
		return (Params[Flags2]->GetIntValue() & Flag) != 0;
	}
	inline void ClearFlag(MaterialVarFlags_t Flag)
	{
		m_ppParams[Flags]->SetIntValue(m_ppParams[Flags]->GetIntValue() & ~(Flag));
	}
	inline void ClearFlag(IMaterialVar** Params, MaterialVarFlags_t Flag) // Safe for Multithreading
	{
		Params[Flags]->SetIntValue(Params[Flags]->GetIntValue() & ~(Flag));
	}
	inline void ClearFlag2(MaterialVarFlags_t Flag2)
	{
		m_ppParams[Flags2]->SetIntValue(m_ppParams[Flags2]->GetIntValue() & ~(Flag2));
	}
	inline void ClearFlag2(IMaterialVar** Params, MaterialVarFlags_t Flag2) // Safe for Multithreading
	{
		Params[Flags2]->SetIntValue(Params[Flags2]->GetIntValue() & ~(Flag2));
	}
	inline bool IsFlagDefined(MaterialVarFlags_t Flag) const
	{
		return (m_ppParams[Flags_Defined]->GetIntValue() & Flag) != 0;
	}
	inline bool IsFlagDefined(IMaterialVar** Params, MaterialVarFlags_t Flag) const // Safe for Multithreading
	{
		return (Params[Flags_Defined]->GetIntValue() & Flag) != 0;
	}
	inline bool IsFlag2Defined(MaterialVarFlags2_t Flag2) const
	{
		return (m_ppParams[Flags_Defined2]->GetIntValue() & Flag2) != 0;
	}
	inline bool IsFlag2Defined(IMaterialVar** Params, MaterialVarFlags2_t Flag2) const // Safe for Multithreading
	{
		return (Params[Flags_Defined2]->GetIntValue() & Flag2) != 0;
	}
	inline void SetFlag(MaterialVarFlags_t Flag)
	{
		m_ppParams[Flags]->SetIntValue(m_ppParams[Flags]->GetIntValue() | (Flag));
	}
	inline void SetFlag(IMaterialVar** Params, MaterialVarFlags_t Flag) // Safe for Multithreading
	{
		Params[Flags]->SetIntValue(Params[Flags]->GetIntValue() | (Flag));
	}
	inline void SetFlag2(MaterialVarFlags2_t Flag2)
	{
		m_ppParams[Flags2]->SetIntValue(m_ppParams[Flags2]->GetIntValue() | (Flag2));
	}
	inline void SetFlag2(IMaterialVar** Params, MaterialVarFlags2_t Flag2) // Safe for Multithreading
	{
		Params[Flags2]->SetIntValue(Params[Flags2]->GetIntValue() | (Flag2));
	}
	// Safe for Multithreading

	// Only works in the Draw Stage of the Shader!
	inline bool InHammer()
	{
		if (this->IsSnapshotting())
			return HasFlag2(MATERIAL_VAR2_USE_EDITOR);
		else if (this->IsDynamicState())
			return m_pShaderAPI->InEditorMode();
		else
			return false;
	}

	// Use with the SKINNING Dynamic Combo
	inline bool HasSkinning()
	{
		return m_pShaderAPI->GetCurrentNumBones() > 0 ? true : false;
	}

	// Use with the MORPHING Dynamic Combo
	inline bool HasMorphing()
	{
		return false; // HasFastVertexTextures() never returned true..
//		return g_pHardwareConfig->HasFastVertexTextures() && m_pShaderAPI->IsHWMorphingEnabled();
	}

	// Use with the COMPRESSION Dynamic Combo
	inline bool HasVertexCompression()
	{
//		Assert(s_nVertexCompression != VERTEX_COMPRESSION_INVALID);
		return (m_nVertexCompression == VERTEX_COMPRESSION_ON);
	}

	void UpdateMaterialContextData(CBasePerMaterialContextData** ppContextData);
	virtual LUXPerMaterialContextData* CreateMaterialContextData() { return nullptr; }

	template<typename T>
	inline T* GetMaterialContextData(CBasePerMaterialContextData** ppContextData)
	{
		return ppContextData ? reinterpret_cast<T*>(*ppContextData) : nullptr;
	}
	
	inline bool IsSnapshottingCommands() const
	{
		return (m_pMaterialContextData && m_pShaderShadow) || (m_pMaterialContextData && m_pMaterialContextData->m_bSnapshottingCommands);
	}
	
	inline bool MaterialVarsChanged() const
	{
		return m_pShaderAPI && m_pMaterialContextData && m_pMaterialContextData->m_bMaterialVarsChanged;
	}

protected:
	SoftwareVertexShader_t m_SoftwareVertexShader;


private:
	LUXPerMaterialContextData *m_pMaterialContextData;
};


//-----------------------------------------------------------------------------
// Gets at the current Materialvar flags
//-----------------------------------------------------------------------------
inline int CBaseShader::CurrentMaterialVarFlags() const
{
	return m_ppParams[Flags]->GetIntValue();
}

//-----------------------------------------------------------------------------
// Are we currently taking a Snapshot?
//-----------------------------------------------------------------------------
inline bool CBaseShader::IsSnapshotting() const
{
	return (m_pShaderShadow != NULL);
}

//-----------------------------------------------------------------------------
// Is the color var white?
//-----------------------------------------------------------------------------
inline bool CBaseShader::IsWhite( int colorVar )
{
	if (colorVar < 0)
		return true;

	if (!m_ppParams[colorVar]->IsDefined())
		return true;

	float color[3];
	m_ppParams[colorVar]->GetVecValue( color, 3 );
	return (color[0] >= 1.0f) && (color[1] >= 1.0f) && (color[2] >= 1.0f);
}

inline bool IsHDRImageFormat(ImageFormat format)
{
	return format == IMAGE_FORMAT_RGBA16161616 || format == IMAGE_FORMAT_RGBA16161616F;
}

bool IsHDREnabled();
bool IsHDRTexture(const ITexture* pTexture);

#endif // BASESHADER_H
