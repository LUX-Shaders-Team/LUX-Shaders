//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	19.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Helper Function for logging and spewing Shader Data
//
//==========================================================================//
#include <algorithm>
#include "ShaderSpew.h"

// Need this for unpacking the Command Buffer
#include "shaderapi/commandbuffer.h"

// Need this for ConColorMsg
#include "Color.h"

// Need to set the fake shadow state for spew
#include "ProxyShaderShadow.h"

// NOTE: This must be the last include File in a .cpp File!
#include "tier0/memdbgon.h"

// Extern
CShaderSpew s_ShaderSpew;

void CShaderSpew::SortFloatConstants()
{
	std::sort(m_pCurDynamicState->m_VSFloatConstants.begin(), m_pCurDynamicState->m_VSFloatConstants.end(),
		[](const FloatShaderConstant_t& a, const FloatShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});

	std::sort(m_pCurDynamicState->m_PSFloatConstants.begin(), m_pCurDynamicState->m_PSFloatConstants.end(),
		[](const FloatShaderConstant_t& a, const FloatShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});
}

void CShaderSpew::SortBooleanConstants()
{
	std::sort(m_pCurDynamicState->m_VSBooleanConstants.begin(), m_pCurDynamicState->m_VSBooleanConstants.end(),
		[](const BooleanShaderConstant_t& a, const BooleanShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});

	std::sort(m_pCurDynamicState->m_PSBooleanConstants.begin(), m_pCurDynamicState->m_PSBooleanConstants.end(),
		[](const BooleanShaderConstant_t& a, const BooleanShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});
}

void CShaderSpew::SortIntegerConstants()
{
	std::sort(m_pCurDynamicState->m_VSIntegerConstants.begin(), m_pCurDynamicState->m_VSIntegerConstants.end(),
		[](const IntegerShaderConstant_t& a, const IntegerShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});

	std::sort(m_pCurDynamicState->m_PSIntegerConstants.begin(), m_pCurDynamicState->m_PSIntegerConstants.end(),
		[](const IntegerShaderConstant_t& a, const IntegerShaderConstant_t& b)
		{
			return a.m_nRegister < b.m_nRegister;
		});
}

void CShaderSpew::SortTextureBinds()
{
	std::sort(m_pCurDynamicState->m_VSTextureBinds.begin(), m_pCurDynamicState->m_VSTextureBinds.end(),
		[](const TextureBinds_t& a, const TextureBinds_t& b)
		{
			return a.m_nSampler < b.m_nSampler;
		});

	std::sort(m_pCurDynamicState->m_PSTextureBinds.begin(), m_pCurDynamicState->m_PSTextureBinds.end(),
		[](const TextureBinds_t& a, const TextureBinds_t& b)
		{
			return a.m_nSampler < b.m_nSampler;
		});
}

void CShaderSpew::Start(const char* ccShader, const char* ccMaterial, bool bAllowed)
{
	// Never do anything during a real pShaderShadow
	if(m_bShaderShadow)
		return;

	if(!m_bAnySpew)
		return;

	if(!bAllowed)
		return;	

	m_bAllowSpew = true;
	m_bAllowShadowLog = true;
	m_bAllowDynamicLog = true;

	// Clean this. Just in case.
	// These should be empty at the start of the Draw

	if (m_ShadowStates.size() > 0)
		m_ShadowStates.clear();

	if (m_DynamicStates.size() > 0)
		m_DynamicStates.clear();

	// Store the Name
	m_ccShaderName = ccShader;
	m_ccMaterialName = ccMaterial;

	// Allocate a Shadow and Dynamic State 
	m_ShadowStates.emplace_back();
	m_DynamicStates.emplace_back();

	// Reference the back
	m_pCurShadowState = &m_ShadowStates.back();
	m_pCurDynamicState = &m_DynamicStates.back();
}

void CShaderSpew::HasShaderShadow(bool bReal)
{
	m_bShaderShadow = bReal;
}

void CShaderSpew::HasShaderAPI(bool bReal)
{
	m_bShaderAPI = bReal;
}

void CShaderSpew::End()
{
	// Never do anything during a real pShaderShadow
	if(m_bShaderShadow)
		return;

	if(!m_bAnySpew)
		return;

	if(!m_bAllowSpew)
		return;	

	// Remove the back of the Vectors. They contain no Data!
	if (m_ShadowStates.size() > 0)
		m_ShadowStates.pop_back();

	if (m_DynamicStates.size() > 0)
		m_DynamicStates.pop_back();

	// Print the Data
	Spew();

	// Cleanup
	if (m_ShadowStates.size() > 0)
		m_ShadowStates.clear();

	if (m_DynamicStates.size() > 0)
		m_DynamicStates.clear();

	// No Names
	m_ccShaderName = nullptr;
	m_ccMaterialName = nullptr;

	// Leave no dangling Pointers
	m_pCurShadowState = nullptr;
	m_pCurDynamicState = nullptr;

	// This always needs to get refreshed
	m_bAllowSpew = false;
	m_bAllowShadowLog = false;
	m_bAllowDynamicLog = false;

	if(m_bSpecificMaterial)
	{
		m_bSpewShadowState = false;
		m_bSpewDynamicState = false;
		m_bAnySpew = false;
		m_bStrictSpew = false;
		s_ProxyShaderShadow.SetFakeShadowState(false);
	}
}

bool CShaderSpew::NeedsFakeShadowState()
{
	return m_bSpewDynamicState;
}

bool CShaderSpew::SpewSpecificMaterial()
{
	// Could be Dynamic or Shadow State
	return (m_bSpewDynamicState || m_bSpewShadowState) && m_bSpecificMaterial;
}

bool CShaderSpew::SpewAllMaterials()
{
	// Only with Dynamic State Spew
	return m_bSpewDynamicState && !m_bSpecificMaterial;
}

void CShaderSpew::ResetShadowState()
{
	if(!m_bAllowShadowLog)
		return;

	// We have a copy of the old state
	// Clear it
	m_pCurShadowState->Clear();
}

void CShaderSpew::ResetDynamicState()
{
	// Nothing to do here
}

void CShaderSpew::SetSpewBehaviour(bool bShadow, bool bDynamic, bool bStrict, bool bSpecific)
{
	m_bSpewShadowState = bShadow;
	m_bSpewDynamicState = bDynamic;
	m_bAnySpew = bShadow || bDynamic;
	m_bStrictSpew = bStrict;
	m_bSpecificMaterial = bSpecific;
}

void CShaderSpew::AllowSpew(bool bEnable)
{
	m_bAllowSpew = bEnable;	
}

bool BlendFactorUsesAlpha(ShaderBlendFactor_t nFactor)
{
	switch(nFactor)
	{
		case SHADER_BLEND_ZERO:
			return false;
		case SHADER_BLEND_ONE:
			return false;
		case SHADER_BLEND_DST_COLOR:
			return false;
		case SHADER_BLEND_ONE_MINUS_DST_COLOR:
			return false;
		case SHADER_BLEND_SRC_ALPHA:
			return true;
		case SHADER_BLEND_ONE_MINUS_SRC_ALPHA:
			return true;
		case SHADER_BLEND_DST_ALPHA:
			return true;
		case SHADER_BLEND_ONE_MINUS_DST_ALPHA:
			return true;
		case SHADER_BLEND_SRC_ALPHA_SATURATE:
			return true;
		case SHADER_BLEND_SRC_COLOR:
			return false;
		case SHADER_BLEND_ONE_MINUS_SRC_COLOR:
			return false;
		default:
			return false;
		}
}

void CShaderSpew::Spew()
{
	if(!m_bAllowSpew)
		return;

	// Header
	ConColorMsg(HEADER_COLOR, "%s\n%s - %s\n%s\n",
		"---------------------------------------",
		m_ccShaderName, m_ccMaterialName,
		"---------------------------------------");

	// Information..
	size_t nPS_Binds = 0;
	size_t nPS_Bools = 0;
	size_t nPS_Ints = 0;
	size_t nPS_Floats = 0;
	size_t nVS_Binds = 0;
	size_t nVS_Bools = 0;
	size_t nVS_Ints = 0;
	size_t nVS_Floats = 0;
	
	// Go through the States and count the overall Data
	for (size_t nDyn = 0; nDyn < m_DynamicStates.size(); ++nDyn)
	{
		LoggedDynamicState_t* pCur = &m_DynamicStates[nDyn];
		nVS_Binds   += pCur->m_VSTextureBinds.size();
		nVS_Bools   += pCur->m_VSBooleanConstants.size();
		nVS_Ints    += pCur->m_VSIntegerConstants.size();
		nVS_Floats  += pCur->m_VSFloatConstants.size();
		nPS_Binds   += pCur->m_PSTextureBinds.size();
		nPS_Bools   += pCur->m_PSBooleanConstants.size();
		nPS_Ints    += pCur->m_PSIntegerConstants.size();
		nPS_Floats  += pCur->m_PSFloatConstants.size();
	}

	// 8 size_t Variables
	if(!m_bStrictSpew)
	{
		ConColorMsg(MESSAGE_COLOR, "%s %zu\n%s %zu\n%s %zu\n%s %zu\n%s %zu\n%s %zu\n%s %zu\n%s %zu\n",
		"PS Constants (c)", nPS_Floats,
		"PS Constants (i)", nPS_Ints,
		"PS Constants (b)", nPS_Bools,
		"PS Texture Binds", nPS_Binds,
		"VS Constants (c)", nVS_Floats,
		"VS Constants (i)",	nVS_Ints,
		"VS Constants (b)", nVS_Bools,
		"VS Texture Binds", nVS_Binds);
	}

	// m_DynamicStates.size() should be the same as m_ShadowStates.size()
	// If it isn't.. Huge Problem
	if(m_ShadowStates.size() != m_DynamicStates.size())
	{
		ConColorMsg(WARNING_COLOR, "Missmatching State Count\n%zu Shadow States - %zu Dynamic States\n", m_ShadowStates.size(), m_DynamicStates.size());
		return;
	}

	// Go through all Dynamic States, print constants and warnings
	// IMPORTANT: All commands are sorted!! So if the previous Register/Sampler is the same as the current
	// It means thing's are set TWICE
	for (int nState = 0; nState < m_DynamicStates.size(); ++nState)
	{
		ConColorMsg(PASS_COLOR, "Shader Pass (%d):\n", nState);
		LoggedShadowState_t* pCurShadow = &m_ShadowStates[nState];
		LoggedDynamicState_t* pCurDynamic = &m_DynamicStates[nState];

		// ====================================== //
		// Shadow State
		// ====================================== //
		if(m_bSpewShadowState && !m_bStrictSpew)
		{
			ConColorMsg(HEADER_COLOR, "%s\n%s\n%s\n",
			"----------------------------",
			"Shadow State",
			"----------------------------");

			// Vertex Shader Vertex Format
			ConColorMsg(MESSAGE_COLOR, "Vertex Shader Vertex Format: %d %d %d\n",
			pCurShadow->m_nVertexFlags, pCurShadow->m_nTexCoords, pCurShadow->m_nUserDataSize);

			// Write Modes
			ConColorMsg(MESSAGE_COLOR, "Writes Color : %s\nWrites Alpha : %s\nWrites  sRGB : %s\nWrites Depth : %s\n",
			pCurShadow->m_bWritesColor ? "true" : "false",
			pCurShadow->m_bWritesAlpha ? "true" : "false",
			pCurShadow->m_bWritesGamma ? "true" : "false",
			pCurShadow->m_bWritesDepth ? "true" : "false");

			// Various
			ConColorMsg(MESSAGE_COLOR, "DepthTesting : %s\nCulling Back : %s\n",
			pCurShadow->m_bDepthTest ? "true" : "false",
			pCurShadow->m_bCulling? "true" : "false");

			if(pCurShadow->m_bAlphaTest)
				ConColorMsg(MESSAGE_COLOR, "AlphaTest : true\nAlphaFunc : %d\n AlphaTestReference : %f\n",
				pCurShadow->m_AlphaFunc, pCurShadow->m_f1AlphaTestReference);

			if(pCurShadow->m_bAlphaBlend)
				ConColorMsg(MESSAGE_COLOR, "Alpha Blend. : true\nSrcFactor : %d\nDstFactor : %d\n",
				pCurShadow->m_AlphaBlend_src, pCurShadow->m_AlphaBlend_dst);

			if(pCurShadow->m_bAlphaTest || pCurShadow->m_bAlphaBlend)
			{
				bool bUsesAlpha = true;

				if (pCurShadow->m_bAlphaBlend)
				{
					bool bBlendSrcUsesAlpha = pCurShadow->m_bAlphaBlend && BlendFactorUsesAlpha(pCurShadow->m_AlphaBlend_src);
					bool bBlendDstUsesAlpha = pCurShadow->m_bAlphaBlend && BlendFactorUsesAlpha(pCurShadow->m_AlphaBlend_dst);

					// Not a Problem if Alpha isn't used for the Blend Modes
					if(!bBlendSrcUsesAlpha && !bBlendDstUsesAlpha)
						bUsesAlpha = false;
				}

				if(bUsesAlpha && pCurShadow->m_bWritesAlpha)
					ConColorMsg(WARNING_COLOR, "AlphaWrites Enabled but Shader uses AlphaTesting or AlphaBlending.\n");
			}

			// Samplers
			for(int n = 0; n < 16; ++n)
			{
				if(pCurShadow->m_Samplers[n].m_bEnabled)
				{
					if(pCurShadow->m_Samplers[n].m_bGammaRead)
						ConColorMsg(MESSAGE_COLOR, "Sampler %02d : Enabled - sRGBRead\n", n);
					else
						ConColorMsg(MESSAGE_COLOR, "Sampler %02d : Enabled\n", n);
				}
				else
					ConColorMsg(MESSAGE_COLOR, "Sampler %02d : Off\n", n);
			}
		}

		if(!m_bStrictSpew && m_bSpewDynamicState)
			ConColorMsg(HEADER_COLOR, "%s\n%s\n%s\n",
			"----------------------------",
			"Dynamic State",
			"----------------------------");

		// ====================================== //
		// Vertex Shader Texture Binds
		// ====================================== //
		int nLastSampler = -1;
		for(size_t n = 0; n < pCurDynamic->m_VSTextureBinds.size(); n++)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			TextureBinds_t* pCurBind = &pCurDynamic->m_VSTextureBinds[n];
			int nCurrentSampler = pCurBind->m_nSampler;

			// Texture Bind Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// If the current Sampler is the same as the previous Sampler,
			// we have two binds on one Sampler. That's bad. Red Things are bad, make it red.
			if(nCurrentSampler == nLastSampler)
				rgbMessageColor = WARNING_COLOR;

			const char* ccTextureType = nullptr;
			switch(pCurBind->m_nTextureType)
			{
			case TEXTUREBIND_POINTER:
				ccTextureType = "ITexture*";
				break;
			case TEXTUREBIND_PARAMETER:
				ccTextureType = "Parameter";
				break;
			case TEXTUREBIND_STANDARD:
				ccTextureType = "StandardTexture";
				break;
			case TEXTUREBIND_HANDLE:
				ccTextureType = "Command Buffer Texture Handle";
				break;
			default:
				rgbMessageColor = WARNING_COLOR;
				ccTextureType = "INVALID TEXTURE TYPE";
				break;
			}

			ConColorMsg(rgbMessageColor, "VS  s%02d - %s %s\n", nCurrentSampler, ccTextureType, pCurBind->m_ccBindName);
			nLastSampler = nCurrentSampler;
		}

		// ====================================== //
		// Vertex Shader Boolean Constants
		// ====================================== //
		int nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_VSBooleanConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			BooleanShaderConstant_t* pCur = &pCurDynamic->m_VSBooleanConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			ConColorMsg(rgbMessageColor, "VS  b%02d - %s\n", nCurrentConstant, ((bool)pCur->m_Value) ? "true" : "false");
			nLastConstant = nCurrentConstant;
		}

		// ====================================== //
		// Vertex Shader Integer Constants
		// ====================================== //
		nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_VSIntegerConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			IntegerShaderConstant_t* pCur = &pCurDynamic->m_VSIntegerConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			ConColorMsg(rgbMessageColor, "VS  i%02d - %d %d %d\n",
				nCurrentConstant, pCur->m_Values[0], pCur->m_Values[1], pCur->m_Values[2]);
			nLastConstant = nCurrentConstant;
		}

		// ====================================== //
		// Vertex Shader Float Constants
		// ====================================== //
		nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_VSFloatConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			FloatShaderConstant_t* pCur = &pCurDynamic->m_VSFloatConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			// If this is a nullptr, the Shader set it's own m_Values
			// Otherwise this is a specific Type of Constant ( EyePos, Fog Params, Light Data, Ambient Cube.. and so on )
			if((pCur->m_ccConstantName == nullptr) ? true : false)
				ConColorMsg(rgbMessageColor, "VS c%03d - %s - %f %f %f %f\n", nCurrentConstant,
				"Custom Const.", pCur->m_Values.x, pCur->m_Values.y, pCur->m_Values.z, pCur->m_Values.w);
			else
				ConColorMsg(rgbMessageColor, "VS c%03d - %s\n", nCurrentConstant, pCur->m_ccConstantName);

			nLastConstant = nCurrentConstant;
		}

		// ====================================== //
		// Pixel Shader Texture Binds
		// ====================================== //
		std::bitset<16> bPSTextureBinds;
		bPSTextureBinds.none();

		nLastSampler = -1;
		for(size_t n = 0; n < pCurDynamic->m_PSTextureBinds.size(); ++n)
		{
			// Mark this Sampler as used.
			bPSTextureBinds.set(pCurDynamic->m_PSTextureBinds[n].m_nSampler);

			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			TextureBinds_t* pCurBind = &pCurDynamic->m_PSTextureBinds[n];
			int nCurrentSampler = pCurBind->m_nSampler;

			// Texture Bind Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// If the current Sampler is the same as the previous Sampler,
			// we have two binds on one Sampler. That's bad. Red Things are bad, make it red.
			if(nCurrentSampler == nLastSampler)
				rgbMessageColor = WARNING_COLOR;

			const char* ccTextureType = nullptr;
			switch(pCurBind->m_nTextureType)
			{
			case TEXTUREBIND_POINTER:
				ccTextureType = "ITexture*";
				break;
			case TEXTUREBIND_PARAMETER:
				ccTextureType = "Parameter";
				break;
			case TEXTUREBIND_STANDARD:
				ccTextureType = "StandardTexture";
				break;
			case TEXTUREBIND_HANDLE:
				ccTextureType = "Command Buffer Texture Handle";
				break;
			default:
				rgbMessageColor = WARNING_COLOR;
				ccTextureType = "INVALID TEXTURE TYPE";
				break;
			}

			ConColorMsg(rgbMessageColor, "PS  s%02d - %s %s\n", nCurrentSampler, ccTextureType, pCurBind->m_ccBindName);
			nLastSampler = nCurrentSampler;
		}

		// Check if all the enabled Samplers have a Texture,
		// Also check if all the Samplers with a Texture were actually enabled to begin with
		for(int nSampler = 0; nSampler < 16; ++nSampler)
		{
			if(pCurShadow->m_Samplers[nSampler].m_bEnabled && !bPSTextureBinds.test(nSampler))
				ConColorMsg(WARNING_COLOR, "PS  s%02d - Enabled but without Texture\n", nSampler);
			else if(!pCurShadow->m_Samplers[nSampler].m_bEnabled && bPSTextureBinds.test(nSampler))
				ConColorMsg(WARNING_COLOR, "PS  s%02d - Disabled but with Texture\n", nSampler);
		}

		// ====================================== //
		// Pixel Shader Boolean Constants
		// ====================================== //
		nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_PSBooleanConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			BooleanShaderConstant_t* pCur = &pCurDynamic->m_PSBooleanConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			ConColorMsg(rgbMessageColor, "PS  b%02d - %s\n", nCurrentConstant, ((bool)pCur->m_Value) ? "true" : "false");
			nLastConstant = nCurrentConstant;
		}

		// ====================================== //
		// Pixel Shader Integer Constants
		// ====================================== //
		nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_PSIntegerConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			IntegerShaderConstant_t* pCur = &pCurDynamic->m_PSIntegerConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			ConColorMsg(rgbMessageColor, "PS  i%02d - %d %d %d\n",
				nCurrentConstant, pCur->m_Values[0], pCur->m_Values[1], pCur->m_Values[2]);
			nLastConstant = nCurrentConstant;
		}

		// ====================================== //
		// Pixel Shader Float Constants
		// ====================================== //
		nLastConstant = -1;
		for(size_t n = 0; n < pCurDynamic->m_PSFloatConstants.size(); ++n)
		{
			if(m_bStrictSpew && !m_bSpewDynamicState)
				break;

			FloatShaderConstant_t* pCur = &pCurDynamic->m_PSFloatConstants[n];
			int nCurrentConstant = pCur->m_nRegister;

			// Boolean Const Default Color
			Color rgbMessageColor = MESSAGE_COLOR;

			// Mark as red when setting the same Register twice
			if(nCurrentConstant == nLastConstant)
				rgbMessageColor = WARNING_COLOR;

			// If this is a nullptr, the Shader set it's own m_Values
			// Otherwise this is a specific Type of Constant ( EyePos, Fog Params, Light Data, Ambient Cube.. and so on )
			if((pCur->m_ccConstantName == nullptr) ? true : false)
				ConColorMsg(rgbMessageColor, "PS c%03d - %3.6f %3.6f %3.6f %3.6f - %s\n", nCurrentConstant,
				pCur->m_Values.x, pCur->m_Values.y, pCur->m_Values.z, pCur->m_Values.w, "Unnammed Constant");
			else
				ConColorMsg(rgbMessageColor, "PS c%03d - %3.6f %3.6f %3.6f %3.6f - %s\n", nCurrentConstant, 
				pCur->m_Values.x, pCur->m_Values.y, pCur->m_Values.z, pCur->m_Values.w, pCur->m_ccConstantName);

			nLastConstant = nCurrentConstant;
		}
	}
}

void CShaderSpew::LogDraw_ShadowState()
{
	if(!m_bAllowShadowLog)
		return;

	// This is VERY important!
	// Multipass Shaders keep the same ShadowState as the previous Pass
	// IF they don't call pShaderShadow->DefaultState() or InitialShadowState(), that calls DefaultState()
	// To replicate this Behaviour, I will do this:
	size_t Size = m_ShadowStates.size();
	m_ShadowStates.emplace_back();
	m_ShadowStates.back() = m_ShadowStates[Size];

	// Reference it
	m_pCurShadowState = &m_ShadowStates.back();
}

void CShaderSpew::LogDraw_DynamicState()
{
	if(!m_bAllowDynamicLog)
		return;

	// Sort the current Dynamic State
	SortFloatConstants();
	SortIntegerConstants();
	SortBooleanConstants();
	SortTextureBinds();

	// Create a new Dynamic State
	m_DynamicStates.emplace_back();

	// Reference the last Element as the current Dynamic State
	m_pCurDynamicState = &m_DynamicStates.back();
}

void CShaderSpew::LogSamplerEnabled(int m_nSampler, bool bEnabled)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_Samplers[m_nSampler].m_bEnabled = bEnabled;
}

void CShaderSpew::LogSamplerGammaRead(int m_nSampler, bool bGammaRead)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_Samplers[m_nSampler].m_bGammaRead = bGammaRead;
}

void CShaderSpew::LogCommandBufferBindTexture(int m_nSampler, const char *ccParameterName)
{
	if(!m_bAllowDynamicLog)
		return;

	// Create new Texture Bind Set at the end of the Buffer
	m_pCurDynamicState->m_PSTextureBinds.emplace_back();
	TextureBinds_t* pBind = &m_pCurDynamicState->m_PSTextureBinds.back();

	// Fill with Data
	pBind->m_nSampler = m_nSampler;
	pBind->m_nTextureType = TEXTUREBIND_HANDLE;
	pBind->m_ccBindName = ccParameterName;
}

void CShaderSpew::LogBindTexture(int m_nSampler, ITexture* pTexture)
{
	if(!m_bAllowDynamicLog)
		return;

	// Create new Texture Bind Set at the end of the Buffer
	m_pCurDynamicState->m_PSTextureBinds.emplace_back();
	TextureBinds_t* pBind = &m_pCurDynamicState->m_PSTextureBinds.back();

	// Fill with Data
	pBind->m_nSampler = m_nSampler;
	pBind->m_nTextureType = TEXTUREBIND_POINTER;
	pBind->m_ccBindName = pTexture->GetName();
}

void CShaderSpew::LogBindTexture(int m_nSampler, const char *ccParameterName)
{
	if(!m_bAllowDynamicLog)
		return;

	// Create new Texture Bind Set at the end of the Buffer
	m_pCurDynamicState->m_PSTextureBinds.emplace_back();
	TextureBinds_t* pBind = &m_pCurDynamicState->m_PSTextureBinds.back();

	// Fill with Data
	pBind->m_nSampler = m_nSampler;
	pBind->m_nTextureType = TEXTUREBIND_PARAMETER;
	pBind->m_ccBindName = ccParameterName;
}

void EvaluateStandardTextureType(TextureBinds_t* pBack, StandardTextureId_t nTexture)
{
	switch (nTexture)
	{
	case TEXTURE_LIGHTMAP:
		pBack->m_ccBindName = "TEXTURE_LIGHTMAP";
		break;
	case TEXTURE_LIGHTMAP_FULLBRIGHT:
		pBack->m_ccBindName = "TEXTURE_LIGHTMAP_FULLBRIGHT";
		break;
	case TEXTURE_LIGHTMAP_BUMPED:
		pBack->m_ccBindName = "TEXTURE_LIGHTMAP_BUMPED";
		break;
	case TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT:
		pBack->m_ccBindName = "TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT";
		break;
	case TEXTURE_WHITE:
		pBack->m_ccBindName = "TEXTURE_WHITE";
		break;
	case TEXTURE_BLACK:
		pBack->m_ccBindName = "TEXTURE_BLACK";
		break;
	case TEXTURE_GREY:
		pBack->m_ccBindName = "TEXTURE_GREY";
		break;
	case TEXTURE_GREY_ALPHA_ZERO:
		pBack->m_ccBindName = "TEXTURE_GREY_ALPHA_ZERO";
		break;
	case TEXTURE_NORMALMAP_FLAT:
		pBack->m_ccBindName = "TEXTURE_NORMALMAP_FLAT";
		break;
	case TEXTURE_NORMALIZATION_CUBEMAP:
		pBack->m_ccBindName = "TEXTURE_NORMALIZATION_CUBEMAP";
		break;
	case TEXTURE_NORMALIZATION_CUBEMAP_SIGNED:
		pBack->m_ccBindName = "TEXTURE_NORMALIZATION_CUBEMAP_SIGNED";
		break;
	case TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0:
		pBack->m_ccBindName = "TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0";
		break;
	case TEXTURE_FRAME_BUFFER_FULL_TEXTURE_1:
		pBack->m_ccBindName = "TEXTURE_FRAME_BUFFER_FULL_TEXTURE_1";
		break;
	case TEXTURE_COLOR_CORRECTION_VOLUME_0:
		pBack->m_ccBindName = "TEXTURE_COLOR_CORRECTION_VOLUME_0";
		break;
	case TEXTURE_COLOR_CORRECTION_VOLUME_1:
		pBack->m_ccBindName = "TEXTURE_COLOR_CORRECTION_VOLUME_1";
		break;
	case TEXTURE_COLOR_CORRECTION_VOLUME_2:
		pBack->m_ccBindName = "TEXTURE_COLOR_CORRECTION_VOLUME_2";
		break;
	case TEXTURE_COLOR_CORRECTION_VOLUME_3:
		pBack->m_ccBindName = "TEXTURE_COLOR_CORRECTION_VOLUME_3";
		break;
	case TEXTURE_FRAME_BUFFER_ALIAS:
		pBack->m_ccBindName = "TEXTURE_FRAME_BUFFER_ALIAS";
		break;
	case TEXTURE_SHADOW_NOISE_2D:
		pBack->m_ccBindName = "TEXTURE_SHADOW_NOISE_2D";
		break;
	case TEXTURE_MORPH_ACCUMULATOR:
		pBack->m_ccBindName = "TEXTURE_MORPH_ACCUMULATOR";
		break;
	case TEXTURE_MORPH_WEIGHTS:
		pBack->m_ccBindName = "TEXTURE_MORPH_WEIGHTS";
		break;
	case TEXTURE_FRAME_BUFFER_FULL_DEPTH:
		pBack->m_ccBindName = "TEXTURE_FRAME_BUFFER_FULL_DEPTH";
		break;
	case TEXTURE_IDENTITY_LIGHTWARP:
		pBack->m_ccBindName = "TEXTURE_IDENTITY_LIGHTWARP";
		break;
	case TEXTURE_DEBUG_LUXELS:
		pBack->m_ccBindName = "TEXTURE_DEBUG_LUXELS";
		break;
	default:
		pBack->m_ccBindName = "INVALID";
		break;
	}
}

void CShaderSpew::LogVSBindStandardTexture(int m_nSampler, StandardTextureId_t nTexture)
{
	if(!m_bAllowDynamicLog)
		return;

	// Create new Texture Bind Set at the end of the Buffer
	m_pCurDynamicState->m_VSTextureBinds.emplace_back();
	TextureBinds_t* pBack = &m_pCurDynamicState->m_VSTextureBinds.back();

	// Fill with Data
	pBack->m_nSampler = m_nSampler;
	pBack->m_nTextureType = TEXTUREBIND_STANDARD;
	EvaluateStandardTextureType(pBack, nTexture);
}

void CShaderSpew::LogBindStandardTexture(int m_nSampler, StandardTextureId_t nTexture)
{
	if(!m_bAllowDynamicLog)
		return;

	// Create new Texture Bind Set at the end of the Buffer
	m_pCurDynamicState->m_PSTextureBinds.emplace_back();
	TextureBinds_t* pBack = &m_pCurDynamicState->m_PSTextureBinds.back();

	// Fill with Data
	pBack->m_nSampler = m_nSampler;
	pBack->m_nTextureType = TEXTUREBIND_STANDARD;
	EvaluateStandardTextureType(pBack, nTexture);
}

void CShaderSpew::LogPixelShaderRegister(int m_nRegister, int nConstants, const char* ccType)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_PSFloatConstants.emplace_back();
		FloatShaderConstant_t* pBack = &m_pCurDynamicState->m_PSFloatConstants.back();

		// Fill with Data, note that we do not have m_Values to set here
		pBack->m_nRegister = n;
		pBack->m_ccConstantName = ccType;
		pBack->m_Values.x = 0.0f;
		pBack->m_Values.y = 0.0f;
		pBack->m_Values.z = 0.0f;
		pBack->m_Values.w = 0.0f;
	}
}

void CShaderSpew::LogPixelShaderRegisterF(int m_nRegister, float const* m_Values, int nConstants, bool bForce, const char* pConstantName)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_PSFloatConstants.emplace_back();
		FloatShaderConstant_t* pBack = &m_pCurDynamicState->m_PSFloatConstants.back();

		// Fill with Data
		pBack->m_nRegister = n;
		pBack->m_Values.x = m_Values[n - m_nRegister + 0];
		pBack->m_Values.y = m_Values[n - m_nRegister + 1];
		pBack->m_Values.z = m_Values[n - m_nRegister + 2];
		pBack->m_Values.w = m_Values[n - m_nRegister + 3];
		pBack->m_ccConstantName = pConstantName;
	}
}

void CShaderSpew::LogPixelShaderRegisterN(int m_nRegister, int const* m_Values, int nConstants)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_PSIntegerConstants.emplace_back();
		IntegerShaderConstant_t* pBack = &m_pCurDynamicState->m_PSIntegerConstants.back();

		// Fill with Data
		pBack->m_nRegister = n;
		pBack->m_Values[0] = m_Values[n - m_nRegister + 0];
		pBack->m_Values[1] = m_Values[n - m_nRegister + 1];
		pBack->m_Values[2] = m_Values[n - m_nRegister + 2];
	}
}

void CShaderSpew::LogPixelShaderRegisterB(int m_nRegister, BOOL const* m_Values, int nConstants)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_PSBooleanConstants.emplace_back();
		BooleanShaderConstant_t* pBack = &m_pCurDynamicState->m_PSBooleanConstants.back();

		pBack->m_nRegister = n;
		pBack->m_Value = m_Values[n - m_nRegister];
	}
}

void CShaderSpew::LogVertexShaderRegister(int m_nRegister, int nConstants, const char* ccType)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_VSFloatConstants.emplace_back();
		FloatShaderConstant_t* pBack = &m_pCurDynamicState->m_VSFloatConstants.back();

		// Fill with Data, note that we do not have m_Values to set here
		pBack->m_nRegister = n;
		pBack->m_ccConstantName = ccType;
		pBack->m_Values.x = 0.0f;
		pBack->m_Values.y = 0.0f;
		pBack->m_Values.z = 0.0f;
		pBack->m_Values.w = 0.0f;
	}
}

void CShaderSpew::LogVertexShaderRegisterF(int m_nRegister, float const* m_Values, int nConstants)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_VSFloatConstants.emplace_back();
		FloatShaderConstant_t* pBack = &m_pCurDynamicState->m_VSFloatConstants.back();

		// Fill with Data
		pBack->m_nRegister = n;
		pBack->m_Values.x = m_Values[n - m_nRegister + 0];
		pBack->m_Values.y = m_Values[n - m_nRegister + 1];
		pBack->m_Values.z = m_Values[n - m_nRegister + 2];
		pBack->m_Values.w = m_Values[n - m_nRegister + 3];
		pBack->m_ccConstantName = nullptr;
	}
}

void CShaderSpew::LogVertexShaderRegisterN(int m_nRegister, int const* m_Values, int nConstants)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_VSIntegerConstants.emplace_back();
		IntegerShaderConstant_t* pBack = &m_pCurDynamicState->m_VSIntegerConstants.back();

		// Fill with Data
		pBack->m_nRegister = n;
		pBack->m_Values[0] = m_Values[n - m_nRegister + 0];
		pBack->m_Values[1] = m_Values[n - m_nRegister + 1];
		pBack->m_Values[2] = m_Values[n - m_nRegister + 2];
	}
}

void CShaderSpew::LogVertexShaderRegisterB(int m_nRegister, BOOL const* m_Values, int nConstants)
{
	if(!m_bAllowDynamicLog)
		return;

	for (int n = m_nRegister; n < (m_nRegister + nConstants); ++n)
	{
		m_pCurDynamicState->m_VSBooleanConstants.emplace_back();
		BooleanShaderConstant_t* pBack = &m_pCurDynamicState->m_VSBooleanConstants.back();

		pBack->m_nRegister = n;
		pBack->m_Value = m_Values[n - m_nRegister];
	}
}

int CShaderSpew::InterpretCommand(uint8_t** pBuffer)
{
	CommandBufferCommand_t Command = *(CommandBufferCommand_t*)(*pBuffer);
	size_t nCommandSize = sizeof(int);
	size_t nVariableSize = sizeof(int);

	// We will go until we find CBCMD_END
	// End Point of the Command Buffer
	if (Command == CBCMD_END)
		return 1;

	// Skip over the Command ( 4 bytes )
	(*pBuffer) += nCommandSize;

	int nReturnCode = 0;
	switch (Command)
	{
	case CBCMD_SET_PIXEL_SHADER_FLOAT_CONST:
	{
		// After Command comes the first Registers
		int m_nRegister = *(int*)(*pBuffer);

		// Advance, after Register comes the amount of Constants
		(*pBuffer) += nVariableSize;

		// Read Constants and jump again
		int nConstants = *(int*)(*pBuffer);
		(*pBuffer) += nVariableSize;

		// pBuffer is the location of the data
		float* pData = (float*)(*pBuffer); // reinterpret current location as float*
		LogPixelShaderRegisterF(m_nRegister, pData, nConstants, "CmdBuffer Constant");

		// Skip over the float Data.
		(*pBuffer) += sizeof(float) * 4 * nConstants;

		// Next Command
		break;
	}

	case CBCMD_SET_VERTEX_SHADER_FLOAT_CONST:
	{
		int m_nRegister = *(int*)(*pBuffer);

		// Advance, after Register comes the amount of Constants
		(*pBuffer) += nVariableSize;

		// Read Constants and jump again
		int nConstants = *(int*)(*pBuffer);
		(*pBuffer) += nVariableSize;

		// pBuffer is the location of the data
		float* pData = (float*)(*pBuffer); // reinterpret current location as float*
		LogVertexShaderRegisterF(m_nRegister, pData, nConstants);

		// Skip over the float Data.
		(*pBuffer) += sizeof(float) * 4 * nConstants;

		// Next Command
		break;
	}

	case CBCMD_SET_VERTEX_SHADER_FLOAT_CONST_REF:
	{
		int m_nRegister = *(int*)(*pBuffer);

		// Advance, after Register comes the amount of Constants
		(*pBuffer) += nVariableSize;

		// Read Constants and jump again
		int nConstants = *(int*)(*pBuffer);
		(*pBuffer) += nVariableSize;

		// pBuffer is the location of the address to the data
		float* pData = (float*)(*((uintptr_t*)pBuffer));
		LogVertexShaderRegisterF(m_nRegister, pData, nConstants);

		// Skip over the float Data. In this case it's an integer pointer to the data
		(*pBuffer) += sizeof(int);

		// Next Command
		break;
	}

	case CBCMD_SETPIXELSHADERFOGPARAMS:
	{
		int m_nRegister = *(int*)(*pBuffer);

		// Always this one Register
		LogPixelShaderRegister(m_nRegister, 1, "Fog Parameters");

		// Advance over the Register
		(*pBuffer) += nVariableSize;

		// Next Command
		break;
	}

	case CBCMD_STORE_EYE_POS_IN_PSCONST:
	{
		// After Command comes the first Register
		int m_nRegister = *(int*)(*pBuffer);

		// Always this one Register
		LogPixelShaderRegister(m_nRegister, 1, "Camera Position");

		// Advance over the Register
		(*pBuffer) += nVariableSize;

		// Next Command
		break;
	}

	case CBCMD_COMMITPIXELSHADERLIGHTING:
	{
		// After Command comes the first Register
		int m_nRegister = *(int*)(*pBuffer);

		// Always 6 Registers
		LogPixelShaderRegister(m_nRegister, 6, "Light Data");

		// Advance over the Register
		(*pBuffer) += nVariableSize;

		// Next Command
		break;
	}

	case CBCMD_SETPIXELSHADERSTATEAMBIENTLIGHTCUBE:
	{
		// After Command comes the first Register
		int m_nRegister = *(int*)(*pBuffer);

		// Always 6 Registers
		LogPixelShaderRegister(m_nRegister, 6, "Ambient Cube");

		// Advance over the Register
		(*pBuffer) += nVariableSize;

		// Next Command
		break;
	}

	case CBCMD_SETAMBIENTCUBEDYNAMICSTATEVERTEXSHADER:
	{
		// Always at the same Register, just CMD..
		// Always 6 Registers
		LogVertexShaderRegister(21, 6, "Ambient Cube");

		// Next Command!
		break;
	}

	case CBCMD_SET_DEPTH_FEATHERING_CONST:
	{
		// After Command comes the first Registers
		int m_nRegister = *(int*)(*pBuffer);

		// Always 1 Register
		LogPixelShaderRegister(m_nRegister, 1, "Depth Feathering Constant");

		// Skip over the float Data. Just one float
		(*pBuffer) += sizeof(float);

		// Next Command
		break;
	}

	case CBCMD_BIND_STANDARD_TEXTURE:
	{
		// After Command comes the Sampler
		int m_nSampler = *(int*)(*pBuffer);

		// Advance, now at the Type of Texture
		(*pBuffer) += nVariableSize;
		int m_nTextureType = *(int*)(*pBuffer);
		LogBindStandardTexture(m_nSampler, (StandardTextureId_t)m_nTextureType);

		// Next Command
		(*pBuffer) += nVariableSize;
		break;
	}

	case CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE:
	{
		// After Command comes the Sampler
		int m_nSampler = *(int*)(*pBuffer);

		// Don't know the Texture Name or the Parameter Name
		// Just a ShaderAPI Texture Bind Handle..
		LogCommandBufferBindTexture(m_nSampler, "..");

		// Advance, now at the Handle
		(*pBuffer) += nVariableSize;

		// Next Command, this one is a intp in the commandbuffer
		(*pBuffer) += sizeof(intp);
		break;
	}

	case CBCMD_SET_PSHINDEX:
	{
		// Next Command
		(*pBuffer) += nVariableSize;
		break;
	}

	case CBCMD_SET_VSHINDEX:
	{
		// Next Command
		(*pBuffer) += nVariableSize;
		break;
	}

	case CBCMD_SET_VERTEX_SHADER_FLASHLIGHT_STATE:
	{
		// Next Command
		(*pBuffer) += nVariableSize;
		break;
	}

	case CBCMD_SET_PIXEL_SHADER_FLASHLIGHT_STATE:
	{
		// Flashlight always uses Dynamic State.
		// Is this even usable?
		Warning("%s\n", "ShaderSpew: Unhandled Command Buffer Instruction! CBCMD_SET_PIXEL_SHADER_FLASHLIGHT_STATE");
		(*pBuffer) += nVariableSize; // color reg -> atten reg
		(*pBuffer) += nVariableSize; // atten reg -> origin reg
		(*pBuffer) += nVariableSize; // origin reg -> sampler
		(*pBuffer) += nVariableSize; // sampler -> Next Command
		break;
	}

	case CBCMD_SET_PIXEL_SHADER_UBERLIGHT_STATE:
	{
		// Alien Swarm Command..?
		Warning("%s\n", "ShaderSpew: Unhandled Command Buffer Instruction! CBCMD_SET_PIXEL_SHADER_UBERLIGHT_STATE");

		// Next Command
		break;
	}

	case CBCMD_SET_VERTEX_SHADER_NEARZFARZ_STATE:
	{
		// No idea what Register this goes to!
		Warning("%s\n", "ShaderSpew: Unhandled Command Buffer Instruction! CBCMD_SET_VERTEX_SHADER_NEARZFARZ_STATE");

		// Next Command
		break;
	}

	case CBCMD_JUMP:
	{
		// Interpret the Jump Instruction as another memory address
		pBuffer = (uint8_t**)pBuffer;

		// Next Command
		break;
	}

	case CBCMD_JSR:
	{
		// Interpret the Jump Instruction as another memory address
		pBuffer = (uint8_t**)pBuffer;

		nReturnCode = 2;
		break;
	}

	default:
	{
		Warning("%s %d\n", "Proxy ShaderAPI: Unknown Command Buffer Instruction!", Command);
		break;
	}
	}

	return nReturnCode;
}

void CShaderSpew::LogCommandBuffer(uint8_t* pBuffer)
{
	if(!m_bAllowDynamicLog)
		return;

	uint8_t* pMain = (uint8_t*)pBuffer;

	int nRet = 0;
	int nCommands = 0;
	while (true)
	{
		if (nCommands > 500)
		{
			Warning("%s\n", "ShaderSpew: Too many CommandBuffer Commands ( > 500 ).\n");
			break;
		}

		uint8_t* pTemp = (uint8_t*)pMain;
		nRet = InterpretCommand(&pMain);
		nCommands++;

		if (nRet == 2)
		{
			while (true)
			{
				nRet = InterpretCommand(&pMain);
				nCommands++;

				if (nRet == 2)
					Warning("ShaderSpew: Encountered a CommandBuffer, in a CommandBuffer, in a CommandBuffer.\n"
						"CommandBuffer Matryoshka not supported!\n");

				if (nRet == 1)
					break;
			}

			// JSR Instruction means we jump to a separate buffer.
			// Return to the old one!
			pMain = pTemp;
		}
		else if (nRet == 1)
		{
			break;
		}
	}
}

void CShaderSpew::LogColorWrites(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bWritesColor = bEnable;
}

void CShaderSpew::LogAlphaWrites(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bWritesAlpha = bEnable;
}

void CShaderSpew::LogGammaWrites(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bWritesGamma = bEnable;
}

void CShaderSpew::LogDepthWrites(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bWritesDepth = bEnable;
}

void CShaderSpew::LogDepthTests(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bDepthTest = bEnable;
}

void CShaderSpew::LogVertexShaderVertexFormat(unsigned int nFlags, int nTexCoords, int* pTexCoordDimensions, int nUserDataSize)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_nVertexFlags = nFlags;
	m_pCurShadowState->m_nTexCoords = nTexCoords;
	m_pCurShadowState->m_nUserDataSize = nUserDataSize;
}

void CShaderSpew::LogCulling(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bCulling = bEnable;
}

void CShaderSpew::LogAlphaTest(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bAlphaTest = bEnable;
}

void CShaderSpew::LogAlphaBlending(bool bEnable)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bAlphaBlend = bEnable;
}

void CShaderSpew::LogAlphaBlending(ShaderBlendFactor_t src, ShaderBlendFactor_t dst)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_AlphaBlend_src = src;
	m_pCurShadowState->m_AlphaBlend_dst = dst;
}

void CShaderSpew::LogAlphaBlending(bool bEnable, ShaderBlendFactor_t src, ShaderBlendFactor_t dst)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_bAlphaBlend = bEnable;
	m_pCurShadowState->m_AlphaBlend_src = src;
	m_pCurShadowState->m_AlphaBlend_dst = dst;
}

void CShaderSpew::LogAlphaFunc(ShaderAlphaFunc_t AlphaFunc, float AlphaTestRef)
{
	if(!m_bAllowShadowLog)
		return;

	m_pCurShadowState->m_AlphaFunc = AlphaFunc;
	m_pCurShadowState->m_f1AlphaTestReference = AlphaTestRef;
}
