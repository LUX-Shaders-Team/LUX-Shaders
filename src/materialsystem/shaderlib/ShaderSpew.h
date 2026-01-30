//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	17.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Wrapper for IShaderDynamicAPI
//
//==========================================================================//

#ifndef SHADERSPEW_H
#define SHADERSPEW_H

#ifdef _WIN32		   
#pragma once
#endif

#include "ishadersystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/ishaderapi.h"

#include <bitset>
#include <vector>

#define PASS_COLOR    Color(175, 185, 25, 255)
#define HEADER_COLOR  Color(50, 165, 255, 255)
#define MESSAGE_COLOR Color(255, 255, 255, 255)
#define WARNING_COLOR Color(255, 0, 0, 255)

struct SamplerData_t
{
	bool m_bEnabled = false;
	bool m_bGammaRead = false;
};

struct LoggedShadowState_t
{
	// Sampler Data
	SamplerData_t m_Samplers[16];

	// Transparency Data
	bool m_bAlphaTest;
	bool m_bAlphaBlend;
	ShaderBlendFactor_t m_AlphaBlend_src;
	ShaderBlendFactor_t m_AlphaBlend_dst;
	ShaderAlphaFunc_t m_AlphaFunc;
	float m_f1AlphaTestReference;

	// Output Data
	bool m_bWritesColor;
	bool m_bWritesAlpha;
	bool m_bWritesGamma;
	bool m_bWritesDepth;
	bool m_bDepthTest;
	bool m_bCulling;

	// Vertex Shader Vertex Format
	// Ignoring TexCoord Dim for this one
	unsigned int m_nVertexFlags;
	int m_nTexCoords;
	int m_nUserDataSize;

	void Clear()
	{
		for (int n = 0; n < 16; ++n)
		{
			m_Samplers[n].m_bEnabled = false;
			m_Samplers[n].m_bGammaRead = false;
		}

		m_bAlphaTest = false;
		m_bAlphaBlend = false;
		m_AlphaBlend_src = SHADER_BLEND_ZERO;
		m_AlphaBlend_dst = SHADER_BLEND_ZERO;
		m_f1AlphaTestReference = 0.0f;
		m_AlphaFunc = SHADER_ALPHAFUNC_NEVER;
		m_bWritesColor = true;
		m_bWritesAlpha = true;
		m_bWritesGamma = false;
		m_bWritesDepth = true;
		m_bDepthTest = true;
		m_bCulling = true;
		m_nVertexFlags = 0;
		m_nTexCoords = 0;
		m_nUserDataSize = 0;
	}

	LoggedShadowState_t()
	{
		Clear();
	}
};

struct FloatShaderConstant_t
{
	unsigned char m_nRegister = 0;
	Vector4D m_Values = Vector4D(0.0f, 0.0f, 0.0f, 0.0f);

	// This is for EyePos, AmbientCube, Pixel Shader Light Info etc
	// Only valid when not nullptr!
	const char* m_ccConstantName = nullptr;
};

struct BooleanShaderConstant_t
{
	unsigned char m_nRegister = 0;
	BOOL m_Value = 0;
};

struct IntegerShaderConstant_t
{
	unsigned char m_nRegister = 0;

	// The actual Constant is a Size of 4,
	// but 3 makes the struct 16 bytes and the .w isn't used
	int m_Values[3] = { 0 };
};

enum TextureBindTextureType_t
{
	TEXTUREBIND_POINTER = 0,
	TEXTUREBIND_PARAMETER = 1,
	TEXTUREBIND_STANDARD = 2,
	TEXTUREBIND_HANDLE = 3,
};

struct TextureBinds_t
{
	int m_nSampler = 0;

	// Equal to TextureBindTextureType_t
	int m_nTextureType = 0;

	// TEXTUREBIND_POINTER		Texture Name
	// TEXTUREBIND_PARAMETER	Parameter Name
	// TEXTUREBIND_STANDARD		Standard Texture Name
	const char* m_ccBindName = nullptr;
};

struct LoggedDynamicState_t
{
	std::vector<TextureBinds_t>			 m_PSTextureBinds;
	std::vector<FloatShaderConstant_t>   m_PSFloatConstants;
	std::vector<BooleanShaderConstant_t> m_PSBooleanConstants;
	std::vector<IntegerShaderConstant_t> m_PSIntegerConstants;

	std::vector<TextureBinds_t>			 m_VSTextureBinds;
	std::vector<FloatShaderConstant_t>   m_VSFloatConstants;
	std::vector<BooleanShaderConstant_t> m_VSBooleanConstants;
	std::vector<IntegerShaderConstant_t> m_VSIntegerConstants;
};

class CShaderSpew
{
private:
	const char* m_ccShaderName = nullptr;
	const char* m_ccMaterialName = nullptr;
	std::string m_strLastMaterialName = "";

	// Don't need sorting Functions for this, it's in chronological Order
	std::vector<LoggedShadowState_t> m_ShadowStates;
	std::vector<LoggedDynamicState_t> m_DynamicStates;

	// References a State until Draw() is called
	// We have placeholder States at all times, since we don't know how many States we need.
	// ( Need a new State after Draw in case there is one, but don't know how many Draw()'s to expect )
	// It ( the last State after the last Draw ) will contain no data and will be discarded by End()
	LoggedShadowState_t* m_pCurShadowState = nullptr;
	LoggedDynamicState_t* m_pCurDynamicState = nullptr;

	void SortFloatConstants();
	void SortBooleanConstants();
	void SortIntegerConstants();
	void SortTextureBinds();

	bool m_bSpewShadowState = false;
	bool m_bSpewDynamicState = false;
	bool m_bAnySpew = false;
	bool m_bStrictSpew = false;
	bool m_bSpecificMaterial = false;
	bool m_bAllowSpew = false;
	
	bool m_bShaderShadow = false;
	bool m_bShaderAPI = false;

	bool m_bAllowShadowLog = false;
	bool m_bAllowDynamicLog = false;

	// Prints Information to the Console.
	void Spew();
public:

	// Feed the Shader Name and Material Name to the Class
	// Used when spewing
	void Start(const char* ccShader, const char* ccMaterial, bool bAllowed);

	// When reloading a Material we have only ShaderShadow
	// This is fine if we are spewing Shadow only
	// But if we want dynamic state, it's going to be empty during load
	// These Functions allow ProxyShaderAPI and ProxyShaderShadow to inform CShaderSpew
	void HasShaderShadow(bool bReal);
	void HasShaderAPI(bool bReal);

	// End Point for recordings, also prints Data
	void End();

	// Called by ProxyShaderShadow to determine if we need a fake ShadowState
	bool NeedsFakeShadowState();

	// Called by CBaseShader Param Init
	bool SpewSpecificMaterial();
	bool SpewAllMaterials();

	// Clears Data of current Buffers.
	// Called by pProxyShaderShadow and pProxyShaderAPI respectively
	void ResetShadowState();
	void ResetDynamicState();

	// Sets the Behaviour for spews
	void SetSpewBehaviour(bool bShadow, bool bDynamic, bool bStrict, bool bSpecific);

	// Draw Function must call this to allow Spews based on $Debug_True
	void AllowSpew(bool bEnable);
	
	// Logging Functions called by CBaseShader
	void LogDraw_ShadowState();
	void LogDraw_DynamicState();

	// Logging Functions called by CProxyShaderShadow and CProxyShaderAPI

	// Logs when a Sampler was enabled
	void LogSamplerEnabled(int nSampler, bool bEnabled);
	void LogSamplerGammaRead(int nSampler, bool bGammaRead);

	// Texture Logs
	void LogVSBindStandardTexture(int nSampler, StandardTextureId_t nTexture);
	void LogCommandBufferBindTexture(int nSampler, const char* ccParameterName);
	void LogBindTexture(int nSampler, ITexture* pTexture);
	void LogBindTexture(int nSampler, const char* ccParameterName);
	void LogBindStandardTexture(int nSampler, StandardTextureId_t nTexture);

	// Logs Constants for Pixel Shaders
	void LogPixelShaderRegister(int nRegister, int nConstants = 1, const char* ccType = nullptr);
	void LogPixelShaderRegisterF(int m_nRegister, float const* m_Values, int nConstants, bool bForce, const char* pConstantName = nullptr);
	void LogPixelShaderRegisterN(int nRegister, int const* Values, int nConstants = 1);
	void LogPixelShaderRegisterB(int nRegister, BOOL const* Values, int nConstants = 1);

	// Logs Constants for Pixel Shaders
	void LogVertexShaderRegister(int nRegister, int nConstants = 1, const char* ccType = nullptr);
	void LogVertexShaderRegisterF(int nRegister, float const* Values, int nConstants = 1);
	void LogVertexShaderRegisterN(int nRegister, int const* Values, int nConstants = 1);
	void LogVertexShaderRegisterB(int nRegister, BOOL const* Values, int nConstants = 1);

	// Helper Function to interpret a CommandBuffer
	// Note that this is needed because Command Buffers can have JSR Instructions
	// These Commands jump to secondary CommandBuffer that are referenced within the primary CommandBuffer
	int InterpretCommand(uint8_t** pBuffer);

	// Log a Command Buffer
	void LogCommandBuffer(uint8_t* pBuffer);

	// Logging Functions called by CProxyShaderShadow

	void LogColorWrites(bool bEnable);
	void LogAlphaWrites(bool bEnable);
	void LogGammaWrites(bool bEnable);
	void LogDepthWrites(bool bEnable);
	void LogDepthTests(bool bEnable);

	void LogVertexShaderVertexFormat(unsigned int nFlags, int nTexCoords, int* pTexCoordDimensions, int nUserDataSize);
	void LogCulling(bool bEnable);
	void LogAlphaTest(bool bEnable);
	void LogAlphaBlending(bool bEnable);
	void LogAlphaBlending(ShaderBlendFactor_t src, ShaderBlendFactor_t dst);
	void LogAlphaBlending(bool bEnable, ShaderBlendFactor_t src, ShaderBlendFactor_t dst);
	void LogAlphaFunc(ShaderAlphaFunc_t AlphaFunc, float AlphaTestRef);

	// Store the keyword from the latest concommand
	void SetCachedKeyword( const char *strKeyword ) { m_strLastMaterialName = strKeyword; }
	const char *GetCachedKeyword() { return m_strLastMaterialName.c_str(); }
};

extern CShaderSpew s_ShaderSpew;

#endif // SHADERSPEW_H