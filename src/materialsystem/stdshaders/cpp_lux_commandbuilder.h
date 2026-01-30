//========== Copyright Valve Corporation, All rights reserved. =============//
//
//	Initial D.	:	16.07.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//===================== File of the LUX Shader Project =====================//

// Adapted from Stock Commandbuilder.h ( SDK2013 - TF2SDK specifically )
// 
// Improved Version of the CommandBuffer Structure by InevitablyDivinity
// With some LUX Additions ( Functions )

// FIXME: 1 Explanation that fits the current Implementation
// FIXME: 2 Make this not crash when creating the Class Structure in the virtual func on the Shader c++

#ifndef LUX_COMMANDBUILDER_H
#define LUX_COMMANDBUILDER_H

#ifdef _WIN32
#pragma once
#endif

#include "lux_common_defines.h"
#include "cpp_floatx.h"

// From Public Headers..
#ifndef COMMANDBUFFER_H
#include "shaderapi/commandbuffer.h"
#endif

#include "BaseVSShader.h"
#include "shaderapi/ishaderapi.h"

template<int N>
class CCommandStorageBuffer
{
public:

// SDK2013SP hates this
#ifdef TF2SDK
	alignas(8)
#endif
	CUtlVector<uint8> m_Data;

	uint8* m_pDataOut;
#ifdef DBGFLAG_ASSERT
	size_t m_nNumBytesRemaining;
#endif

	FORCEINLINE CCommandStorageBuffer()
	{
		Reset();
	}

	FORCEINLINE void EnsureCapacity(size_t sz)
	{
		Assert(m_nNumBytesRemaining >= sz);
	}

	template<class T> FORCEINLINE void Put(T const& nValue)
	{
		EnsureCapacity(sizeof(T));
		*(reinterpret_cast<T*>(m_pDataOut)) = nValue;
		m_pDataOut += sizeof(nValue);
#ifdef DBGFLAG_ASSERT
		m_nNumBytesRemaining -= sizeof(nValue);
#endif
	}

	FORCEINLINE void PutInt(int nValue)
	{
		Put(nValue);
	}

	FORCEINLINE void PutBool(bool bValue)
	{
		Put(bValue);
	}

	FORCEINLINE void PutFloat(float nValue)
	{
		Put(nValue);
	}

	FORCEINLINE void PutPtr(void* pPtr)
	{
		Put(pPtr);
	}

	FORCEINLINE void PutMemory(const void* pMemory, size_t nBytes)
	{
		EnsureCapacity(nBytes);
		memcpy(m_pDataOut, pMemory, nBytes);
		m_pDataOut += nBytes;
	}

	FORCEINLINE uint8* Base()
	{
		return m_Data.Base();
	}

	FORCEINLINE void Reset()
	{
		m_Data.Purge();
		m_Data.SetSize(N);
		m_pDataOut = m_Data.Base();
#ifdef DBGFLAG_ASSERT
		m_nNumBytesRemaining = N;
#endif
	}

	FORCEINLINE size_t Size() const
	{
		return m_Data.Size();
	}

	FORCEINLINE void Compact()
	{
		m_Data.Compact();
	}

};

template<int N>
class CFixedCommandStorageBuffer
{
public:
	uint8 m_Data[N];

	uint8* m_pDataOut;
#ifdef DBGFLAG_ASSERT
	size_t m_nNumBytesRemaining;
#endif

	FORCEINLINE CFixedCommandStorageBuffer()
	{
		Reset();
	}

	FORCEINLINE void EnsureCapacity(size_t sz)
	{
		Assert(m_nNumBytesRemaining >= sz);
	}

	template<class T> FORCEINLINE void Put(T const& nValue)
	{
		EnsureCapacity(sizeof(T));
		*(reinterpret_cast<T*>(m_pDataOut)) = nValue;
		m_pDataOut += sizeof(nValue);
#ifdef DBGFLAG_ASSERT
		m_nNumBytesRemaining -= sizeof(nValue);
#endif
	}

	FORCEINLINE void PutInt(int nValue)
	{
		Put(nValue);
	}

	FORCEINLINE void PutBool(bool bValue)
	{
		Put(bValue);
	}

	FORCEINLINE void PutFloat(float nValue)
	{
		Put(nValue);
	}

	FORCEINLINE void PutPtr(void* pPtr)
	{
		Put(pPtr);
	}

	FORCEINLINE void PutMemory(const void* pMemory, size_t nBytes)
	{
		EnsureCapacity(nBytes);
		memcpy(m_pDataOut, pMemory, nBytes);
		m_pDataOut += nBytes;
	}

	FORCEINLINE uint8* Base()
	{
		return m_Data;
	}

	FORCEINLINE void Reset()
	{
		m_pDataOut = m_Data;
#ifdef DBGFLAG_ASSERT
		m_nNumBytesRemaining = N;
#endif
	}

	FORCEINLINE size_t Size() const
	{
		return m_pDataOut - m_Data;
	}

	FORCEINLINE void Compact()
	{
	}

};

template<class S> class CCommandBufferBuilder
{
	IMaterialVar** m_ppParams = nullptr;
	CBaseShader* BaseShader;
public:
	S m_Storage;

	CCommandBufferBuilder(CBaseShader* pShader)
		: m_ppParams(pShader->m_ppParams)
	{
		BaseShader = pShader;
	}

	FORCEINLINE void End(void)
	{
		m_Storage.PutInt(CBCMD_END);
		m_Storage.Compact();
	}

	//========================//
	// Old Code
	//========================//
	// Access Structure for Param's
	FORCEINLINE IMaterialVar* Param(int nVar) const
	{
		return m_ppParams[nVar];
	}

	// Puts Structured Constants into the Buffer
	FORCEINLINE void OutputConstantData(float const* pSrcData)
	{
		m_Storage.PutFloat(pSrcData[0]);
		m_Storage.PutFloat(pSrcData[1]);
		m_Storage.PutFloat(pSrcData[2]);
		m_Storage.PutFloat(pSrcData[3]);
	}

	// Puts individual Constants into the Buffer
	FORCEINLINE void OutputConstantData4(float flVal0, float flVal1, float flVal2, float flVal3)
	{
		m_Storage.PutFloat(flVal0);
		m_Storage.PutFloat(flVal1);
		m_Storage.PutFloat(flVal2);
		m_Storage.PutFloat(flVal3);
	}

	//========================//
	// Parameter Interface
	//========================//
	FORCEINLINE bool GetBool(const int var)
	{
		// NOTE: This ignores negative values.
		// Material Creators shouldn't set negative values anyways
		return (CBaseShader::m_ppParams[var]->GetIntValue() != 0);
	}

	FORCEINLINE int GetInt(const int var)
	{
		return CBaseShader::m_ppParams[var]->GetIntValue();
	}

	FORCEINLINE float GetFloat(const int var)
	{
		return CBaseShader::m_ppParams[var]->GetFloatValue();
	}

	// floatx interface
	FORCEINLINE float2 GetFloat2(const int var)
	{
		float2 f2Result;
		CBaseShader::m_ppParams[var]->GetVecValue(f2Result, 2);
		return f2Result;
	}

	// floatx interface
	FORCEINLINE float3 GetFloat3(const int var)
	{
		float3 f3Result;
		CBaseShader::m_ppParams[var]->GetVecValue(f3Result, 3);
		return f3Result;
	}

	// floatx interface
	FORCEINLINE float4 GetFloat4(const int var)
	{
		float4 f4Result;
		CBaseShader::m_ppParams[var]->GetVecValue(f4Result, 4);
		return f4Result;
	}

	//==================================//
	// Pixel Shader Constants
	//==================================//

	// Throw floats into a Register..
	FORCEINLINE void SetPixelShaderConstants(int nFirstConstant, int nConstants)
	{
		m_Storage.PutInt(CBCMD_SET_PIXEL_SHADER_FLOAT_CONST);
		m_Storage.PutInt(nFirstConstant);
		m_Storage.PutInt(nConstants);
	}

	// Sends a float4 Constant into the Buffer that was computed on the Shader
	FORCEINLINE void SetPixelShaderConstant(int nFirstConstant, float const* pSrcData, int nNumConstantsToSet)
	{
		SetPixelShaderConstants(nFirstConstant, nNumConstantsToSet);
		m_Storage.PutMemory(pSrcData, 4 * sizeof(float) * nNumConstantsToSet);
	}

	// Sends a float4 Constant into the Buffer, from a Parameter
	FORCEINLINE void SetPixelShaderConstant(int nFirstConstant, int nVar)
	{
		SetPixelShaderConstant(nFirstConstant, Param(nVar)->GetVecValue());
	}

	// Sends a float4 Constant into the Buffer, from a Parameter, after converting it to Linear Values
	void SetPixelShaderConstantGammaToLinear(int pixelReg, int nVar)
	{
		float4 Value = GetFloat4(nVar);
		Value.x = Value.x > 1.0f ? Value.x : GammaToLinear(Value.x);
		Value.y = Value.y > 1.0f ? Value.y : GammaToLinear(Value.y);
		Value.z = Value.z > 1.0f ? Value.z : GammaToLinear(Value.z);

		// Stock Consistency, 1.0f for Alpha..
		Value.w = 1.0f;
		SetPixelShaderConstant(pixelReg, Value);
	}

	// Sends a float4 Constant into the Buffer that was computed on the Shader
	// Forces nNumConstantsToSet == 1
	FORCEINLINE void SetPixelShaderConstant(int nFirstConstant, float const* pSrcData)
	{
		// Just.. reuse the existing Function
		SetPixelShaderConstant(nFirstConstant, pSrcData, 1);

//		SetPixelShaderConstants(nFirstConstant, 1);
//		OutputConstantData(pSrcData);
	}

	// Sends a float4 Constant into the Buffer that was computed on the Shader
	// But its all separate Values..
	FORCEINLINE void SetPixelShaderConstant4(int nFirstConstant, float f1x, float f1y, float f1z, float f1w)
	{
		// Just reuse existing Functions!!
		SetPixelShaderConstant(nFirstConstant, float4(f1x, f1y, f1z, f1w), 1);

//		SetPixelShaderConstants(nFirstConstant, 1);
//		OutputConstantData4(flVal0, flVal1, flVal2, flVal3);
	}

	// Sends a float4 Constant into the Buffer that was *partially* (w) computed on the Shader
	// xyz is from Parameter Storage
	FORCEINLINE void SetPixelShaderConstant_W(int pixelReg, int constantVar, float f1w)
	{
		if (constantVar != -1)
		{
			float4 Value;
			Value.xyz = GetFloat3(constantVar);
			Value.w = f1w;
			SetPixelShaderConstant(pixelReg, Value, 1);

			/*
			float val[3];
			Param(constantVar)->GetVecValue(val, 3);
			SetPixelShaderConstant4(pixelReg, val[0], val[1], val[2], fWValue);
			*/
		}
	}

	//==================================//
	// Vertex Shader Constants
	//==================================//

	// Throw floats into a Register..
	FORCEINLINE void SetVertexShaderConstant(int nFirstConstant, float const* pSrcData)
	{
		m_Storage.PutInt(CBCMD_SET_VERTEX_SHADER_FLOAT_CONST);
		m_Storage.PutInt(nFirstConstant);
		m_Storage.PutInt(1);
		OutputConstantData(pSrcData);
	}

	// Sends a float4 Constant into the Buffer that was computed on the Shader
	FORCEINLINE void SetVertexShaderConstant( int nFirstConstant, float const *pSrcData, int nConsts )
	{
		m_Storage.PutInt( CBCMD_SET_VERTEX_SHADER_FLOAT_CONST );
		m_Storage.PutInt( nFirstConstant );
		m_Storage.PutInt( nConsts );
		m_Storage.PutMemory( pSrcData, 4 * nConsts * sizeof( float ) );
	}

	// Sends a float4 Constant into the Buffer, from a Parameter
	FORCEINLINE void SetVertexShaderConstant(int nFirstConstant, int nVar)
	{
		SetVertexShaderConstant(nFirstConstant, Param(nVar)->GetVecValue());
	}

	// Sends a float4 Constant into the Buffer that was computed on the Shader
	// But its all separate Values..
	FORCEINLINE void SetVertexShaderConstant4(int nFirstConstant, float f1x, float f1y, float f1z, float f1w)
	{
		// Just reuse existing Functions!!
		SetVertexShaderConstant(nFirstConstant, float4(f1x, f1y, f1z, f1w), 1);

//		SetPixelShaderConstants(nFirstConstant, 1);
//		OutputConstantData4(flVal0, flVal1, flVal2, flVal3);
	}

	/*
	FORCEINLINE void SetVertexShaderConstant4( int nFirstConstant, float flVal0, float flVal1, float flVal2, float flVal3 )
	{
		m_Storage.PutInt( CBCMD_SET_VERTEX_SHADER_FLOAT_CONST );
		m_Storage.PutInt( nFirstConstant );
		m_Storage.PutInt( 1 );
		m_Storage.PutFloat( flVal0 );
		m_Storage.PutFloat( flVal1 );
		m_Storage.PutFloat( flVal2 );
		m_Storage.PutFloat( flVal3 );
	}
	*/

	//==================================//
	// Stock-Consistency Functions
	//==================================//

	void SetVertexShaderTextureTransform( int vertexReg, int transformVar )
	{
		Vector4D transformation[2];
		IMaterialVar* pTransformationVar = Param( transformVar );
		if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
		{
			const VMatrix &mat = pTransformationVar->GetMatrixValue();
			transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
			transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
		}
		else
		{
			transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
			transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
		}
		SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
	}

	void SetVertexShaderTextureScaledTransform( int vertexReg, int transformVar, int scaleVar )
	{
		Vector4D transformation[2];
		IMaterialVar* pTransformationVar = Param( transformVar );
		if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
		{
			const VMatrix &mat = pTransformationVar->GetMatrixValue();
			transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
			transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
		}
		else
		{
			transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
			transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
		}

		Vector2D scale( 1, 1 );
		IMaterialVar* pScaleVar = Param( scaleVar );
		if (pScaleVar)
		{
			if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
				pScaleVar->GetVecValue( scale.Base(), 2 );
			else if (pScaleVar->IsDefined())
				scale[0] = scale[1] = pScaleVar->GetFloatValue();
		}

		// Apply the scaling
		transformation[0][0] *= scale[0];
		transformation[0][1] *= scale[1];
		transformation[1][0] *= scale[0];
		transformation[1][1] *= scale[1];
		transformation[0][3] *= scale[0];
		transformation[1][3] *= scale[1];
		SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
	}

	FORCEINLINE void SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar )
	{
		if( g_pConfig->bShowSpecular )
		{
			SetPixelShaderConstant( pixelReg, Param( tintVar)->GetVecValue() );
		}
		else
		{
			SetPixelShaderConstant4( pixelReg, 0.0, 0.0, 0.0, 0.0 );
		}
	}

	FORCEINLINE void SetEnvMapTintPixelShaderDynamicStateGammaToLinear( int pixelReg, int tintVar, float flAlphaValue = 1.0 )
	{
		if( ( tintVar != -1 ) && g_pConfig->bShowSpecular )
		{
			float color[4];
			color[3] = flAlphaValue;
			Param( tintVar)->GetLinearVecValue( color, 3 );
			SetPixelShaderConstant( pixelReg, color );
		}
		else
		{
			SetPixelShaderConstant4( pixelReg, 0.0, 0.0, 0.0, flAlphaValue );
		}
	}
	FORCEINLINE void StoreEyePosInPixelShaderConstant( int nConst )
	{
		m_Storage.PutInt( CBCMD_STORE_EYE_POS_IN_PSCONST );
		m_Storage.PutInt( nConst );
	}

	FORCEINLINE void CommitPixelShaderLighting( int nConst )
	{
		m_Storage.PutInt( CBCMD_COMMITPIXELSHADERLIGHTING );
		m_Storage.PutInt( nConst );
	}

	FORCEINLINE void SetPixelShaderStateAmbientLightCube( int nConst )
	{
		m_Storage.PutInt( CBCMD_SETPIXELSHADERSTATEAMBIENTLIGHTCUBE );
		m_Storage.PutInt( nConst );
	}

	FORCEINLINE void SetAmbientCubeDynamicStateVertexShader( void )
	{
		m_Storage.PutInt( CBCMD_SETAMBIENTCUBEDYNAMICSTATEVERTEXSHADER );
	}

	FORCEINLINE void SetPixelShaderFogParams( int nReg )
	{
		m_Storage.PutInt( CBCMD_SETPIXELSHADERFOGPARAMS );
		m_Storage.PutInt( nReg );
	}

	//==================================//
	// More consistent Naming Scheme..
	//==================================//
	
	FORCEINLINE void SetPixelShaderConstant_EyePos(int nReg)
	{
		StoreEyePosInPixelShaderConstant(nReg);
	}

	FORCEINLINE void SetPixelShaderConstant_Lighting(int nReg)
	{
		CommitPixelShaderLighting(nReg);
	}

	FORCEINLINE void SetPixelShaderConstant_AmbientCube(int nReg)
	{
		SetPixelShaderStateAmbientLightCube(nReg);
	}

	FORCEINLINE void SetVertexShaderConstant_AmbientCube()
	{
		SetAmbientCubeDynamicStateVertexShader();
	}

	FORCEINLINE void SetPixelShaderConstant_FogParams(int nReg)
	{
		SetPixelShaderFogParams(nReg);
	}

	//==================================//
	// Stock-Consistency Functions
	//==================================//

	FORCEINLINE void cmdBufferBindStandardTexture(Sampler_t nSampler, StandardTextureId_t nTextureId)
	{
		m_Storage.PutInt(CBCMD_BIND_STANDARD_TEXTURE);
		m_Storage.PutInt(nSampler);
		m_Storage.PutInt(nTextureId);
	}

	FORCEINLINE void cmdBufferBindTexture(Sampler_t nSampler, ShaderAPITextureHandle_t hTexture)
	{
		Assert(hTexture != INVALID_SHADERAPI_TEXTURE_HANDLE);
		if (hTexture != INVALID_SHADERAPI_TEXTURE_HANDLE)
		{
			m_Storage.PutInt(CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE);
			m_Storage.PutInt(nSampler);
			m_Storage.Put(hTexture);
		}
	}

	FORCEINLINE void cmdBindTexture(Sampler_t nSampler, int nTextureVar, int nFrameVar)
	{
		ShaderAPITextureHandle_t hTexture = BaseShader->GetShaderAPITextureBindHandle(nTextureVar, nFrameVar);
		cmdBufferBindTexture(nSampler, hTexture);
	}

	/*
	FORCEINLINE void BindMultiTexture(CBaseVSShader* pShader, Sampler_t nSampler1, Sampler_t nSampler2, int nTextureVar, int nFrameVar)
	{
		ShaderAPITextureHandle_t hTexture = pShader->GetShaderAPITextureBindHandle(nTextureVar, nFrameVar, 0);
		BindTexture(nSampler1, hTexture);
		hTexture = pShader->GetShaderAPITextureBindHandle(nTextureVar, nFrameVar, 1);
		BindTexture(nSampler2, hTexture);
	}
	*/

	//==================================//
	// LUX Bind Texture Functions
	//==================================//
	// FrameVars here must Default to -1, otherwise we get JunkData and that will crash the Game.

	FORCEINLINE void BindTexture(Sampler_t nSampler, int nTextureVar, int nFrameVar = -1)
	{
		// Stock Function here requires CBaseVSShader*
		// Previously I saw an instance where it uses CBaseShader::m_ppParams
		// Well, GetShaderAPITextureBindHandle turns out to be a CBaseShader Function. So we don't need the BaseVSShader Reference!
		ShaderAPITextureHandle_t hTexture = BaseShader->GetShaderAPITextureBindHandle(nTextureVar, nFrameVar);
		cmdBufferBindTexture(nSampler, hTexture);

		// ShiroDkxtro2: This is kinda odd?
		// We are Buffering Commands right? nFrameVar Value will change after we Buffer this..
		// Inless the ShaderAPITextureHandle is specific to this Shader, it will break Frame Vars!
	}

	// NOTE: I didn't make a ITexture* Variant. I don't know if the ITexture* Address changes, so I deemed that too unsafe.

	FORCEINLINE void BindTexture(bool bCheck, Sampler_t nSampler, int nTextureVar, int nFrameVar = -1)
	{
		if (bCheck)
		{
			cmdBindTexture(nSampler, nTextureVar, nFrameVar);
		}
	}

	FORCEINLINE void BindTexture(bool bCheck, Sampler_t nSampler, int nTextureVar, int nFrameVar, StandardTextureId_t StandardTexture)
	{
		if (bCheck)
		{
			cmdBindTexture(nSampler, nTextureVar, nFrameVar);
		}
		else
		{
			cmdBufferBindStandardTexture(nSampler, StandardTexture);
		}
	}

	FORCEINLINE void BindTexture(const bool bCheck, Sampler_t nSampler, StandardTextureId_t StandardTexture)
	{
		if (bCheck)
		{
			cmdBufferBindStandardTexture(nSampler, StandardTexture);
		}
	}

	FORCEINLINE void BindTexture(Sampler_t nSampler, StandardTextureId_t StandardTexture)
	{
		cmdBufferBindStandardTexture(nSampler, StandardTexture);
	}

	//==================================//
	// Remaining Stock Functions
	//==================================//

	FORCEINLINE void SetPixelShaderIndex( int nIndex )
	{
		m_Storage.PutInt( CBCMD_SET_PSHINDEX );
		m_Storage.PutInt( nIndex );
	}

	FORCEINLINE void SetVertexShaderIndex( int nIndex )
	{
		m_Storage.PutInt( CBCMD_SET_VSHINDEX );
		m_Storage.PutInt( nIndex );
	}

	FORCEINLINE void SetDepthFeatheringPixelShaderConstant( int iConstant, float fDepthBlendScale )
	{
		m_Storage.PutInt( CBCMD_SET_DEPTH_FEATHERING_CONST );
		m_Storage.PutInt( iConstant );
		m_Storage.PutFloat( fDepthBlendScale );
	}

	FORCEINLINE void Goto( uint8 *pCmdBuf )
	{
		m_Storage.PutInt( CBCMD_JUMP );
		m_Storage.PutPtr( pCmdBuf );
	}

	FORCEINLINE void Call( uint8 *pCmdBuf )
	{
		m_Storage.PutInt( CBCMD_JSR );
		m_Storage.PutPtr( pCmdBuf );
	}

	FORCEINLINE void Reset( CBaseShader* pShader )
	{
		BaseShader = pShader;
		m_Storage.Reset();
	}

	FORCEINLINE size_t Size( void ) const
	{
		return m_Storage.Size();
	}

	FORCEINLINE uint8 *Base( void )
	{
		return m_Storage.Base();
	}
};

template<int N>
using ShrinkableCommandBuilder_t = CCommandBufferBuilder<CCommandStorageBuffer<N>>;

template<int N>
using CommandBuilder_t = CCommandBufferBuilder<CFixedCommandStorageBuffer<N>>;

#endif // commandbuilder_h
