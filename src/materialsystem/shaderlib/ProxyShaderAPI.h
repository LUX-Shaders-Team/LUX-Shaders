//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	17.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	'Wrapper' for IShaderDynamicAPI
//
//==========================================================================//

#ifndef PROXYSHADERAPI_H
#define PROXYSHADERAPI_H

#ifdef _WIN32		   
#pragma once
#endif

#include "shaderapi/ishaderdynamic.h"
#include "ShaderSpew.h"

// For the TF2SDK Define
#include "../stdshaders/lux_common_defines.h"

// Inherit the original class
class CProxyShaderDynamicAPI : IShaderDynamicAPI
{
	// NOTE: Do not put any Functions at the Top here, this needs to follow the Layout of IShaderDynamicAPI!
public:
	// Likely not intended to be used by a Shader
	void SetViewports( int nCount, const ShaderViewport_t* pViewports ) override;

	// Returns Information on the current ViewPort
	// For an Example of how this is used, see LUX_Cable_Spline
	int GetViewports( ShaderViewport_t* pViewports, int nMax ) const override;

	// Returns the Time in Seconds.
	// NOTE: This is not Game Time! The Value of the double WILL increase if the Game is paused
	double CurrentTime() const override;

	// Returns the Dimensions of the current Lightmap Page
	// Usually 1024x512
	void GetLightmapDimensions( int *w, int *h ) override;

	// Scene Fog State.
	// Returns one of 3 Fog Modes ( None, Linear, Below Z )
	// None means no Fog
	// Linear means Range/Radial Fog
	// Below Z means the Material is underwater and need's WaterFogToDestAlpha,
	MaterialFogMode_t GetSceneFogMode( ) override;

	// Returns the Fog Color 0-255
	// ????: Linear? Does this change with ShadowState Fog Modes?
	void GetSceneFogColor( unsigned char *rgb ) override;

	// Stuff related to Matrix Stacks
	// Likely not intended to be used by a Shader
	void MatrixMode( MaterialMatrixMode_t matrixMode ) override;
	void PushMatrix() override;
	void PopMatrix() override;
	void LoadMatrix( float *m ) override;
	void MultMatrix( float *m ) override;
	void MultMatrixLocal( float *m ) override;
	void GetMatrix( MaterialMatrixMode_t matrixMode, float *dst ) override;
	void LoadIdentity( void ) override;
	void LoadCameraToWorld( void ) override;
	void Ortho( double left, double right, double bottom, double top, double zNear, double zFar ) override;
	void PerspectiveX( double fovx, double aspect, double zNear, double zFar ) override;
	virtual	void PickMatrix( int x, int y, int width, int height ) override;
	void Rotate( float angle, float x, float y, float z ) override;
	void Translate( float x, float y, float z ) override;
	void Scale( float x, float y, float z ) override;
	void ScaleXY( float x, float y ) override;

	// Sets the Color to modulate by
	// Likely not intended to be used by a Shader
	void Color3f( float r, float g, float b ) override;
	void Color3fv( float const* pColor ) override;
	void Color4f( float r, float g, float b, float a ) override;
	void Color4fv( float const* pColor ) override;
	void Color3ub( unsigned char r, unsigned char g, unsigned char b ) override;
	void Color3ubv( unsigned char const* pColor ) override;
	void Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a ) override;
	void Color4ubv( unsigned char const* pColor ) override;

	// Sets one or more Float Constant Register for the Vertex Shader
	void SetVertexShaderConstant( int var, float const* pVec, int numConst = 1, bool bForce = false ) override;

	// Sets one or more Float Constant Register for the Pixel Shader
	void SetPixelShaderConstant( int var, float const* pVec, int numConst = 1, bool bForce = false ) override;

	// Sets the default *dynamic* state
	// Important for Shaders with more than one Draw() - Multipass Shaders
	void SetDefaultState() override;

	// Gets the current Camera Position in World Space. [float3]
	void GetWorldSpaceCameraPosition( float* pPos ) const override;

	// Returns the Bone Count for animated Models
	int GetCurrentNumBones( void ) const override;

	// ????: What does this return exactly?
	int GetCurrentLightCombo( void ) const override;

	// Although I have not confirmed this,
	// GetCurrentFogType() likely returns the Fog Type of the Material itself
	// GetSceneFogMode() would subsequently return the FogMode of the *Scene*
	// For Example, the Material might be under Water, so it indicates Height Fog
	// But the Scene may be using Range Fog
	MaterialFogMode_t GetCurrentFogType( void ) const override;

	// Outdated Methods
	// "fixme: move this to shadow state"
	void SetTextureTransformDimension( TextureStage_t textureStage, int dimension, bool projected ) override;
	void DisableTextureTransform( TextureStage_t textureStage ) override;
	void SetBumpEnvMatrix( TextureStage_t textureStage, float m00, float m01, float m10, float m11 ) override;

	// Sets the Dynamic Shader Combo
	// ????: Why is one default -1 the other 0?
	void SetVertexShaderIndex( int vshIndex = -1 ) override;
	void SetPixelShaderIndex( int pshIndex = 0) override;

	// Gets the Dimensions of the BackBuffer.
	// IMPORTANT:
	// This is not necessarily the Resolution of the Destination Rendertarget!
	// or use ITexture* GetRenderTargetEx() if you want to handle multiple Destination Targets
	void GetBackBufferDimensions( int& width, int& height ) const override;

	// "FIXME: The following 6 methods used to live in IShaderAPI
	// and were moved for stdshader_dx8. Let's try to move them back!"

	// Get the current maximum Number of Lights a Model receives
	// The Maximum for this is usually be 4, but it has been seen at 3
	int GetMaxLights( void ) const override;

	// Returns LightData for a specific Light Index
	const LightDesc_t& GetLight( int lightNum ) const override;

	// Sets a float4 to the specified Register containing the Fog Parameters
	// .x FogEndOverRange
	// .y WaterZ
	// .z FogMaxDensity
	// .w FogOORange
	// NOTE: This is missing Fog Color and '1.0f / DestAlphaDepthRange'
	// Those get set to c29 regardless of which Register this Function is used with.
	// NOTE2: WaterZ is set to the latest WaterZ even if the Water Surface is no longer in View
	void SetPixelShaderFogParams( int reg ) override;

	// Sets a float3[6] Ambient Color Cube to 6 Registers on the Vertex Shader
	// Specifically on c21, c22, c23, c24, c25, c26
	void SetVertexShaderStateAmbientLightCube() override;

	// Sets a float3[6] Ambient Color Cube to 6 Registers on the Pixels Shader
	void SetPixelShaderStateAmbientLightCube( int pshReg, bool bForceToBlack = false ) override;

	// Sets a float4[6] Light Data Struct on the Pixel Shader
	// 3 Lights are uncompressed. 4th Light is compressed onto the .w's
	void CommitPixelShaderLighting( int pshReg ) override;

	// MeshBuilder that allows to modify the Vertex Data
	// NOTE: This does not appear to be functional
	// Probably a dead DX8 Feature
	CMeshBuilder* GetVertexModifyBuilder() override;

	// Returns whether or not the Shader finds itself on an additive projected Texture Pass
	// You should probably use the CBaseShader Function 'UsingFlashlight' instead
	bool InFlashlightMode() const override;

	// Returns the FlashlightState_t and the assosciated Matrix of the current projected Texture
	// Use GetFlashlightStateEx() instead for DepthTexture ITexture*!
	const FlashlightState_t &GetFlashlightState( VMatrix &worldToTexture ) const override;

	// Returns whether or not the Shader finds itself in an Editor Tool like Hammer
	// You should probably use the CBaseShader Function 'UsingEditor' instead
	bool InEditorMode() const override;

	// "Gets the bound morph's vertex format returns 0 if no morph is bound"
	MorphFormat_t GetBoundMorphFormat() override;

	// Binds a Standard Texture
	// See also the CBaseShader Functions
	void BindStandardTexture( Sampler_t sampler, StandardTextureId_t id ) override;

	// Returns the ITexture of currently bound Rendertargets 0-3
	// Note that usually Source only has 1 Rendertarget
	// Note also that this can randomly return a nullptr. Might be better to go through the Materialsystem here
	ITexture *GetRenderTargetEx( int nRenderTargetID ) override;

	// Sets the ToneMappingScale, this should probably not be called from a Shader.
	void SetToneMappingScaleLinear( const Vector &scale ) override;

	// Gets the ToneMappingScale.
	// NOTE: This is a Vector, implying it returns more than one Value
	const Vector &GetToneMappingScaleLinear( void ) const override;

	// Returns the LightmapScaleFactor.
	// For HDR this will be 16.0f
	// LDR gets 4.594794f (?)
	// NOTE: This doesn't appear to exist anymore in Alien Swarm
	// If you are porting LUX to Alien Swarm or .. newer(?)
	// Reference Stock LightmappedGeneric Lightmap Code for more Information
	float GetLightMapScaleFactor( void ) const override;

	// This is likely not intended to be used on a Shader
	void LoadBoneMatrix( int boneIndex, const float *m ) override;

	// "Special off-center perspective matrix for DoF, MSAA jitter and poster rendering" - Alien Swarm imaterialsystem.h
	// This is likely not intended to be used on a Shader
	void PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right ) override;

	// Shaders shouldn't set Rendering Parameters during Rendering!!
	void SetFloatRenderingParameter(int parm_number, float value) override;
	void SetIntRenderingParameter(int parm_number, int value) override ;
	void SetVectorRenderingParameter(int parm_number, Vector const &value) override ;

	// See public/renderparm.h for more Information
	// RenderParamFloat_t, RenderParamInt_t, RenderParamVector_t
	// IMPORTANT: You can send *entire structs* to the Shader from the Client.
	// Use the Int RenderParam's to store a pointer to the Data.
	// Textures can be sent to the Shader this way too, by passing on the ITexture*
	float GetFloatRenderingParameter(int parm_number) const override ;
	int GetIntRenderingParameter(int parm_number) const override ;
	Vector GetVectorRenderingParameter(int parm_number) const override ;

	// Stencil Buffer Operations.
	// This is likely not intended to be used on a Shader
	void SetStencilEnable(bool onoff) override;
	void SetStencilFailOperation(StencilOperation_t op) override;
	void SetStencilZFailOperation(StencilOperation_t op) override;
	void SetStencilPassOperation(StencilOperation_t op) override;
	void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn) override;
	void SetStencilReferenceValue(int ref) override;
	void SetStencilTestMask(uint32 msk) override;
	void SetStencilWriteMask(uint32 msk) override;
	void ClearStencilBufferRectangle( int xmin, int ymin, int xmax, int ymax,int value) override;

	// Returns some DXLevel Information
	// NOTE: Shouldn't this be a g_pHardwareConfig Method? See also GetDXSupportLevel
	void GetDXLevelDefaults(uint &max_dxlevel,uint &recommended_dxlevel) override;

	// Returns the FlashlightState_t and the assosciated Matrix of the current projected Texture
	// If the Depth Texture ITexture* is NOT nullptr it means the projected Texture has a Shadows
	const FlashlightState_t &GetFlashlightStateEx( VMatrix &worldToTexture, ITexture **pFlashlightDepthTexture ) const override;

	// Returns the perceptual(?) Luminance of the AmbientLightCube
	float GetAmbientLightCubeLuminance() override;

	// Returns the LightState for a Model.
	// This is mostly useless for Brushes.
	void GetDX9LightState( LightState_t *state ) const override;

	// 0 = Range Fog / No Fog. Original Comment says 'or no fog simulated with rigged range fog values'
	// 1 = Height Fog
	// If you are on the TF2SDK, use GetPixelFogCombo1() instead
	// IMPORTANT: You can use == 1 to determine if the Material is underwater
	// Used by LUX to stop DecalModulate from having messed up Transparency underneath the Water Surface ( Ancient TF2 Issue )
	int GetPixelFogCombo( ) override;

	// Sets dedicated Vertex Standard Textures to the Vertex Shader
	// NOTE: Only TEXTURE_MORPH_ACCUMULATOR and TEXTURE_MORPH_WEIGHTS
	// appear to be actual Vertex Textures.
	// There does not appear to be a way to load or create custom Vertex Textures.
	// Additionally, there wouldn't even be a Function to allow binding them.
	void BindStandardVertexTexture( VertexTextureSampler_t sampler, StandardTextureId_t id ) override;

	// 'Is hardware morphing enabled?'
	// Note that to use this, Shaders also check HasFastVertexTextures()
	// Which appears to always return false..
	// Does this work in SFM or Alien Swarm?
	bool IsHWMorphingEnabled( ) const override;

	// Returns the Dimensions of a Standard Texture
	// Avoid running this Function in an early-stage Shader ( UnlitGeneric )
	// StandardTextures may not always be available during start-up
	// NOTE: Doesn't work too well for Framebuffer Textures
	void GetStandardTextureDimensions( int *pWidth, int *pHeight, StandardTextureId_t id ) override;

	// BOOL and Integer Vertex Shader Constants
	void SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numBools = 1, bool bForce = false ) override;
	void SetIntegerVertexShaderConstant( int var, int const* pVec, int numIntVecs = 1, bool bForce = false ) override;

	// BOOL and Integer Pixel Shader Constants
	void SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools = 1, bool bForce = false ) override;
	void SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs = 1, bool bForce = false ) override;
	
	// 'Are we in a configuration that needs access to depth data through the alpha channel later?'
	bool ShouldWriteDepthToDestAlpha( void ) const override;

	// Deformations
	// This is likely not intended to be used on a Shader
	void PushDeformation( DeformationBase_t const *Deformation ) override;
	void PopDeformation( ) override;
	int GetNumActiveDeformations() const override;

	// "for shaders to set vertex shader constants. returns a packed state which can be used to set
	// the dynamic combo. returns # of active deformations"
	int GetPackedDeformationInformation( int nMaskOfUnderstoodDeformations,
												 float *pConstantValuesOut,
												 int nBufferSize,
												 int nMaximumDeformations,
												 int *pNumDefsOut ) const override;

	// "This lets the lower level system that certain vertex fields requested 
	// in the shadow state aren't actually being read given particular state
	// known only at dynamic state time. It's here only to silence warnings."
	// Used by Shader Morphing Code.
	void MarkUnusedVertexFields( unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords ) override;

	// Executes a Command Buffer. Reference existing Shaders for how to use this ( it's a bit complicated )
	// You can also check ProxyShaderAPI.cpp to see how this *may* be interpreted by the ShaderAPI
	void ExecuteCommandBuffer( uint8 *pCmdBuffer ) override;

	// "interface for mat system to tell shaderapi about standard texture handles"
	// This is used by CBaseShader.
	// This is likely not intended to be used by a Shader directly
	void SetStandardTextureHandle( StandardTextureId_t nId, ShaderAPITextureHandle_t nHandle ) override;

	// "Interface for mat system to tell shaderapi about color correction"
	// Used in EnginePost
	void GetCurrentColorCorrection( ShaderColorCorrectionInfo_t* pInfo ) override;

	// Sets a float4 to the specified Register containing Near and FarZ
	// .x NearZ
	// .y FarZ
	// .zw Empty
	void SetPSNearAndFarZ( int pshReg ) override;

	// Sets (DepthRange / Scale), .yzw is empty(?)
	// See lux_common_depth.h for more Information.
	void SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale ) override;
	
#ifdef TF2SDK
	// Introduced by the TF2SDK. This is identical to GetPixelFogCombo,
	// Except, it returns 2 for Radial Fog.
	// Preferably use HasRadialFog() from CBaseVSShader,
	// It allows support for lux_radialfog and subsequently SDK2013SP
	int GetPixelFogCombo1( bool bSupportsRadial ) override;
#endif

	private:
	IShaderDynamicAPI* m_pShaderAPI = NULL;
	public:

	inline void SetShaderAPI(IShaderDynamicAPI* pOriginal)
	{
		m_pShaderAPI = pOriginal;
	}

	inline void ResetShaderAPI()
	{
		m_pShaderAPI = NULL;
	}

	inline IShaderDynamicAPI* GetProxy(IShaderDynamicAPI* pOriginal)
	{	
		if(pOriginal)
		{
			SetShaderAPI(pOriginal);
			return this;
		}
		else
		{
			return NULL;
		}
	}

	// This Class inherits from IShaderDynamicAPI and does not change the Function Layout
	// We can simply return this as a IShaderDynamicAPI*
	operator IShaderDynamicAPI*()
	{
		return this;
	}
};

extern CProxyShaderDynamicAPI s_ProxyShaderAPI;

#endif // PROXYSHADERAPI_H