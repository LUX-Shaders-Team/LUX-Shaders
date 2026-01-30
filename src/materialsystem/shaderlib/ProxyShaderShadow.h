//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	17.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	'Wrapper' for IShaderShadow
//
//==========================================================================//

#ifndef PROXYSHADERSHADOW_H
#define PROXYSHADERSHADOW_H

#ifdef _WIN32		   
#pragma once
#endif

#include "shaderapi/ishadershadow.h"
#include "VCSHotReload.h"
#include "ShaderSpew.h"

// For a more compact overview, look at the original ishadershadow.h
class CProxyShaderShadow : IShaderShadow
{
	// NOTE: Do not put any Functions at the Top here, this needs to follow the Layout of IShaderShadow!
public:

	// Resets the ShadowState.
	// This is called by CBaseShader's SetInitialShadowState() override;
	// If you want to support $NoCull, $AllowAlphaToCoverage, etc. you should call that instead
	void SetDefaultState() override;

	// DepthTesting is affected by the DepthFunc
	// By default Depth Tests will be enabled
	void EnableDepthTest(bool bEnable) override;

	// The DepthOperation used to determine whether the Surface is visible
	void DepthFunc(ShaderDepthFunc_t depthFunc) override;

	// Note that EnableBlending() override; and EnableAlphaBlending turn DepthWrites off
	// You can turn it again afterwards using this Function
	// DepthWrites are on by default.
	void EnableDepthWrites(bool bEnable) override;

	// Polygon Offset. D3D9 does not appear to have a dedicated Method for Face Offsets
	// OpenGL does however. It is not clear what this Function does
	void EnablePolyOffset(PolygonOffsetMode_t nOffsetMode) override;

	// 'These methods for controlling stencil are obsolete and stubbed to do nothing.  Stencil
	// control is via the shaderapi/material system now, not part of the shadow state.
	// Methods related to stencil'
	// Don't use these.
	void EnableStencil(bool bEnable) override;
	void StencilFunc(ShaderStencilFunc_t stencilFunc) override;
	void StencilPassOp(ShaderStencilOp_t stencilOp) override;
	void StencilFailOp(ShaderStencilOp_t stencilOp) override;
	void StencilDepthFailOp(ShaderStencilOp_t stencilOp) override;
	void StencilReference(int nReference) override;
	void StencilMask(int nMask) override;
	void StencilWriteMask(int nMask) override;

	// Allows the Shader to draw to the Destination RenderTargets RGB Values
	// ColorWrites are enabled by default.
	void EnableColorWrites(bool bEnable) override;

	// Allows the Shader to draw to the Destination RenderTargets Alpha Values
	// AlphaWrites are enabled by default(?) override;
	void EnableAlphaWrites(bool bEnable) override;

	// Methods related to Alpha Blending

	// After calling EnableBlending, BlendFunc should be called.
	// Preferably use EnableAlphaBlending from CBaseShader
	// Which also disables DepthWritse
	void EnableBlending(bool bEnable) override;
	void BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override;

	// AlphaTesting Methods should usually be called together.
	// Note that you cannot use AlphaTesting and AlphaBlending at the same time.
	void EnableAlphaTest(bool bEnable) override;
	void AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */) override;

	// Allows for Front/Back Faces to be rendered as Points or a Wireframe
	// Note that this is indirectly called when using MATERIAL_VAR_WIREFRAME
	// This is handled in SetInitialShadowState() override;
	void PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode) override;

	// Back Face culling is enabled by default
	// This Function allows disabling it.
	// Note that there is currently no dedicated Method of inversing the culling Behaviour
	// PS30 allows you to use the VFACE input register
	// If you want front-face culling, disable culling then use Discard() with VFACE
	// VFACE will be -1 for Backfaces and +1 for Frontfaces
	void EnableCulling(bool bEnable) override;

	// ????: What does this do, old FFP Function?
	// constant color + transparency
	void EnableConstantColor(bool bEnable) override;

	// Indicates the Vertex Format used by the Vertex Shader
	// Note that just setting Things you want here isn't going to work.
	// Only the Things provided to the Mesh can actually be used.
	// If you are writing a new Shader for an existing Geometry Type,
	// reference a Shader already used on that Geometry Type.
	// 
	// The Flags to pass in here come from the VertexFormatFlags_t enum
	// If pTexCoordDimensions is *not* specified, all Coordinates
	// are interpreted as float2's. ( 2-Dimensional ) override;
	void VertexShaderVertexFormat(unsigned int nFlags,
		int nTexCoordCount, int* pTexCoordDimensions, int nUserDataSize) override;

	// Tells the System which Shader to associate with the next Draw()
	// Important for Dynamic State..
	// This is called by SET_STATIC_VERTEX_SHADER and SET_STATIC_PIXEL_SHADER
	// You usually never have to manually call this unless you want to support custom Shaders
	// or have a Shader with varied Vertex/Pixel Shaders like Screenspace_General
	// ????: Why is SetPixelShader a virtual and why does it set a default Index?
	void SetVertexShader(const char* pFileName, int nStaticVshIndex) override;
	void SetPixelShader(const char* pFileName, int nStaticPshIndex = 0) override;

	// ????: What does this do, old FFP Function?
	// Indicates we're going to light the model
	void EnableLighting(bool bEnable) override;

	// ????: What does this do, old FFP Function?
	// Enables specular lighting (lighting has also got to be enabled) override;
	void EnableSpecular(bool bEnable) override;

	// "Convert from linear to gamma color space on writes to frame buffer."
	// true = Shader outputs Linear values that need to be converted
	// false = Don't convert the output
	// Note that you **MUST** call this Function
	// Calling this Function with true will make s15 unusable! ( L->G LUT ) override;
	void EnableSRGBWrite(bool bEnable) override;

	// Causes Textures bound to this Sampler to be converted from Gamma to Linear
	// This is using dedicated D3D9 Functionality.
	// HDR Textures ( RGBA16161616F, RGBA16161616, R32F, etc. ) override; usually don't need conversion.
	void EnableSRGBRead(Sampler_t sampler, bool bEnable) override;

	// ????: Does this do what it says it does, disable skinning?
	// Usually for Skinning you must set MATERIAL_VAR2_SUPPORTS_HW_SKINNING
	// Then check NumBones to know when Skinning is enabled.
	// Override Function?
	// 
	// "Activate/deactivate skinning. Indexed blending is automatically
	// enabled if it's available for this hardware. When blending is enabled,
	// we allocate enough room for 3 weights (max allowed) override;"
	void EnableVertexBlend(bool bEnable) override;

	// ????: What does this do, Function related to Procedural Textures?
	// per texture unit stuff
	void OverbrightValue(TextureStage_t stage, float value) override;

	// Enables a Sampler. Once enabled a Texture MUST be bound
	// See also EnableSRGBRead() override;, which has Relevance to this Function
	void EnableTexture(Sampler_t sampler, bool bEnable) override;

	// ????: What does this do, Function related to Procedural Textures?
	void EnableTexGen(TextureStage_t stage, bool bEnable) override;

	void TexGen(TextureStage_t stage, ShaderTexGenParam_t param) override;

	// Old FFP Stuff?
	// "alternate method of specifying per-texture unit stuff, more flexible and more complicated
	// Can be used to specify different operation per channel (alpha/color) override;..."
	void EnableCustomPixelPipe(bool bEnable) override;

	void CustomTextureStages(int stageCount) override;
	void CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel,
		ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2) override;

	// ????: What does this do?
	// "Indicates what per-vertex data we're providing"
	void DrawFlags(unsigned int drawFlags) override;

	// ????: What does this do, Functions related to Procedural Textures?
	// A simpler method of dealing with alpha modulation
	void EnableAlphaPipe(bool bEnable) override;
	void EnableConstantAlpha(bool bEnable) override;
	void EnableVertexAlpha(bool bEnable) override;
	void EnableTextureAlpha(TextureStage_t stage, bool bEnable) override;

	// ????: What does this do?
	// "GR - Separate alpha blending"
	void EnableBlendingSeparateAlpha(bool bEnable) override;
	void BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override;

	// Sets the FogMode ( as the name implies ) override;
	// This is used internally by CBaseShader Functions you should be using instead
	// like FogToBlack, DisableFog, etc.
	void FogMode(ShaderFogMode_t fogMode) override;

	// ????: What does this do?
	void SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource) override;

	// This is probably not Functional.
	// "Indicates the morph format for use with a vertex shader
	// The flags to pass in here come from the MorphFormatFlags_t enum"
	void SetMorphFormat(MorphFormat_t flags) override;

	// "some blending modes won't work properly with corrected fog"
	void DisableFogGammaCorrection(bool bDisable) override;

	// Enables AlphaToCoverage, this is used in SetInitialShadowState() override;
	// Which you should be using instead.
	void EnableAlphaToCoverage(bool bEnable) override;

	// Indicates that the Sampler is used for a Shadow Depth Texture
	void SetShadowDepthFiltering(Sampler_t stage) override;

	// ????: What does this do, old FFP Function?
	// More alpha blending state
	void BlendOp(ShaderBlendOp_t blendOp) override;
	void BlendOpSeparateAlpha(ShaderBlendOp_t blendOp) override;

private:
	IShaderShadow* m_pShaderShadow = NULL;

	// 3 States here
	// Fake = ProxyShaderShadow with FakeShadowState
	// Real = pShaderShadow with !bRealShaderShadow
	// Neither = nullptr
	bool m_bFakeShadowState = false;
	bool m_bRealShaderShadow = false;

public:

	operator IShaderShadow*()
	{
		return this;
	}

	inline void SetShaderShadow(IShaderShadow* pOriginal)
	{
		m_pShaderShadow = pOriginal;
	}

	inline void ResetShaderShadow()
	{
		m_pShaderShadow = NULL;
	}

	inline IShaderShadow* GetProxy(IShaderShadow* pOriginal)
	{
		SetShaderShadow(pOriginal);
	
		// Make sure we are dealing with a real ShaderShadow
		m_bRealShaderShadow = pOriginal ? true : false;
		s_ShaderSpew.HasShaderShadow(m_bRealShaderShadow);
		if(m_bFakeShadowState)
		{
			return this;
		}
		else
		{
			if(pOriginal)
			{
				return this;
			}
			else
			{
				return NULL;
			}
		}
	}
	
	// Causes Functions from CProxyShaderShadow to not do anything!
	// This is only for ShaderSpew!
	inline void SetFakeShadowState(bool bFake)
	{
		m_bFakeShadowState = bFake;
	}
};

extern CProxyShaderShadow s_ProxyShaderShadow;

#endif // PROXYSHADERSHADOW_H
