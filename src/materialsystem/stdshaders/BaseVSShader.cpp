//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//==========================================================================//
#if !defined(_STATIC_LINKED) || defined(STDSHADER_DX8_DLL_EXPORT) || defined(STDSHADER_DX9_DLL_EXPORT)

#include "BaseVSShader.h"
#include "mathlib/vmatrix.h"
#include "mathlib/bumpvects.h"
#include "convar.h"

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "cpp_lux_shared.h"

// For the DepthToDestAlpha Replacement
#include "lux_model_simplified_vs30.inc"
#include "lux_depthtodestalpha_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// These functions are to be called from the shaders.

//-----------------------------------------------------------------------------
// Pixel and vertex shader constants....
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar, int constantVar2 )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1) || (constantVar2 == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );
	IMaterialVar* pPixelVar2 = m_ppParams[constantVar2];
	Assert( pPixelVar2 );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 3 );
	}
	else
	{
		val[0] = val[1] = val[2] = pPixelVar->GetFloatValue();
	}

	val[3] = pPixelVar2->GetFloatValue();
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar, int constantVar2 )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1) || (constantVar2 == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );
	IMaterialVar* pPixelVar2 = m_ppParams[constantVar2];
	Assert( pPixelVar2 );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 3 );
	}
	else
	{
		val[0] = val[1] = val[2] = pPixelVar->GetFloatValue();
	}

	val[3] = pPixelVar2->GetFloatValue();
	val[0] = val[0] > 1.0f ? val[0] : GammaToLinear( val[0] );
	val[1] = val[1] > 1.0f ? val[1] : GammaToLinear( val[1] );
	val[2] = val[2] > 1.0f ? val[2] : GammaToLinear( val[2] );

	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstant_W( int pixelReg, int constantVar, float fWValue )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();
	val[3]=fWValue;
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();

	val[0] = val[0] > 1.0f ? val[0] : GammaToLinear( val[0] );
	val[1] = val[1] > 1.0f ? val[1] : GammaToLinear( val[1] );
	val[2] = val[2] > 1.0f ? val[2] : GammaToLinear( val[2] );

	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetVertexShaderConstantGammaToLinear( int var, float const* pVec, int numConst, bool bForce )
{
	int i;
	for( i = 0; i < numConst; i++ )
	{
		float vec[4];
		vec[0] = pVec[i*4+0] > 1.0f ? pVec[i*4+0] : GammaToLinear( pVec[i*4+0] );
		vec[1] = pVec[i*4+1] > 1.0f ? pVec[i*4+1] : GammaToLinear( pVec[i*4+1] );
		vec[2] = pVec[i*4+2] > 1.0f ? pVec[i*4+2] : GammaToLinear( pVec[i*4+2] );
		vec[3] = pVec[i*4+3];

		m_pShaderAPI->SetVertexShaderConstant( var + i, vec, 1, bForce );
	}
}

void CBaseVSShader::SetPixelShaderConstantGammaToLinear( int var, float const* pVec, int numConst, bool bForce )
{
	int i;
	for( i = 0; i < numConst; i++ )
	{
		float vec[4];
		vec[0] = pVec[i*4+0] > 1.0f ? pVec[i*4+0] : GammaToLinear( pVec[i*4+0] );
		vec[1] = pVec[i*4+1] > 1.0f ? pVec[i*4+1] : GammaToLinear( pVec[i*4+1] );
		vec[2] = pVec[i*4+2] > 1.0f ? pVec[i*4+2] : GammaToLinear( pVec[i*4+2] );

		vec[3] = pVec[i*4+3];

		m_pShaderAPI->SetPixelShaderConstant( var + i, vec, 1, bForce );
	}
}

// GR - special version with fix for const/lerp issue
void CBaseVSShader::SetPixelShaderConstantFudge( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = m_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 4 );
		val[0] = val[0] * 0.992f + 0.0078f;
		val[1] = val[1] * 0.992f + 0.0078f;
		val[2] = val[2] * 0.992f + 0.0078f;
		val[3] = val[3] * 0.992f + 0.0078f;
	}
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue() * 0.992f + 0.0078f;
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetVertexShaderConstant( int vertexReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!m_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pVertexVar = m_ppParams[constantVar];
	Assert( pVertexVar );

	float val[4];
	if (pVertexVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pVertexVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pVertexVar->GetFloatValue();
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, val );	
}

//-----------------------------------------------------------------------------
// Sets normalized light color for pixel shaders.
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderLightColors( int pixelReg )
{
	int i;
	int maxLights = m_pShaderAPI->GetMaxLights();
	for( i = 0; i < maxLights; i++ )
	{
		const LightDesc_t & lightDesc = m_pShaderAPI->GetLight( i );
		if( lightDesc.m_Type != MATERIAL_LIGHT_DISABLE )
		{
			Vector color( lightDesc.m_Color[0], lightDesc.m_Color[1], lightDesc.m_Color[2] );
			VectorNormalize( color );
			float val[4] = { color[0], color[1], color[2], 1.0f };
			m_pShaderAPI->SetPixelShaderConstant( pixelReg + i, val, 1 );
		}
		else
		{
			float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_pShaderAPI->SetPixelShaderConstant( pixelReg + i, zero, 1 );
		}
	}
}

//-----------------------------------------------------------------------------
// Sets vertex shader texture transforms
//-----------------------------------------------------------------------------
void CBaseVSShader::SetVertexShaderTextureTranslation( int vertexReg, int translationVar )
{
	float offset[2] = {0, 0};

	IMaterialVar* pTranslationVar = m_ppParams[translationVar];
	if (pTranslationVar)
	{
		if (pTranslationVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pTranslationVar->GetVecValue( offset, 2 );
		else
			offset[0] = offset[1] = pTranslationVar->GetFloatValue();
	}

	Vector4D translation[2];
	translation[0].Init( 1.0f, 0.0f, 0.0f, offset[0] );
	translation[1].Init( 0.0f, 1.0f, 0.0f, offset[1] );
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, translation[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureScale( int vertexReg, int scaleVar )
{
	float scale[2] = {1, 1};

	IMaterialVar* pScaleVar = m_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale, 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	Vector4D scaleMatrix[2];
	scaleMatrix[0].Init( scale[0], 0.0f, 0.0f, 0.0f );
	scaleMatrix[1].Init( 0.0f, scale[1], 0.0f, 0.0f );
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, scaleMatrix[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureTransform( int vertexReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = m_ppParams[transformVar];
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
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureScaledTransform( int vertexReg, int transformVar, int scaleVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = m_ppParams[transformVar];
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
	IMaterialVar* pScaleVar = m_ppParams[scaleVar];
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
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}

//-----------------------------------------------------------------------------
// Sets pixel shader texture transforms
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderTextureTranslation( int pixelReg, int translationVar )
{
	float offset[2] = {0, 0};

	IMaterialVar* pTranslationVar = m_ppParams[translationVar];
	if (pTranslationVar)
	{
		if (pTranslationVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pTranslationVar->GetVecValue( offset, 2 );
		else
			offset[0] = offset[1] = pTranslationVar->GetFloatValue();
	}

	Vector4D translation[2];
	translation[0].Init( 1.0f, 0.0f, 0.0f, offset[0] );
	translation[1].Init( 0.0f, 1.0f, 0.0f, offset[1] );
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, translation[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureScale( int pixelReg, int scaleVar )
{
	float scale[2] = {1, 1};

	IMaterialVar* pScaleVar = m_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale, 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	Vector4D scaleMatrix[2];
	scaleMatrix[0].Init( scale[0], 0.0f, 0.0f, 0.0f );
	scaleMatrix[1].Init( 0.0f, scale[1], 0.0f, 0.0f );
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, scaleMatrix[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureTransform( int pixelReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = m_ppParams[transformVar];
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
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureScaledTransform( int pixelReg, int transformVar, int scaleVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = m_ppParams[transformVar];
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
	IMaterialVar* pScaleVar = m_ppParams[scaleVar];
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
	m_pShaderAPI->SetPixelShaderConstant( pixelReg, transformation[0].Base(), 2 ); 
}

//-----------------------------------------------------------------------------
// Moves a matrix into vertex shader constants 
//-----------------------------------------------------------------------------
void CBaseVSShader::SetVertexShaderMatrix3x4( int vertexReg, int matrixVar )
{
	IMaterialVar* pTranslationVar = m_ppParams[matrixVar];
	if (pTranslationVar)
	{
		m_pShaderAPI->SetVertexShaderConstant( vertexReg, &pTranslationVar->GetMatrixValue( )[0][0], 3 ); 
	}
	else
	{
		VMatrix matrix;
		MatrixSetIdentity( matrix );
		m_pShaderAPI->SetVertexShaderConstant( vertexReg, &matrix[0][0], 3 ); 
	}
}

void CBaseVSShader::SetVertexShaderMatrix4x4( int vertexReg, int matrixVar )
{
	IMaterialVar* pTranslationVar = m_ppParams[matrixVar];
	if (pTranslationVar)
	{
		m_pShaderAPI->SetVertexShaderConstant( vertexReg, &pTranslationVar->GetMatrixValue( )[0][0], 4 ); 
	}
	else
	{
		VMatrix matrix;
		MatrixSetIdentity( matrix );
		m_pShaderAPI->SetVertexShaderConstant( vertexReg, &matrix[0][0], 4 ); 
	}
}

//-----------------------------------------------------------------------------
// Loads the view matrix into pixel shader constants
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadViewMatrixIntoVertexShaderConstant( int vertexReg )
{
	VMatrix mat, transpose;
	m_pShaderAPI->GetMatrix( MATERIAL_VIEW, mat.m[0] );

	MatrixTranspose( mat, transpose );
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, transpose.m[0], 3 );
}

//-----------------------------------------------------------------------------
// Loads the projection matrix into pixel shader constants
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadProjectionMatrixIntoVertexShaderConstant( int vertexReg )
{
	VMatrix mat, transpose;
	m_pShaderAPI->GetMatrix( MATERIAL_PROJECTION, mat.m[0] );

	MatrixTranspose( mat, transpose );
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, transpose.m[0], 4 );
}

//-----------------------------------------------------------------------------
// Loads the projection matrix into pixel shader constants
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadModelViewMatrixIntoVertexShaderConstant( int vertexReg )
{
	VMatrix view, model, modelView;
	m_pShaderAPI->GetMatrix( MATERIAL_MODEL, model.m[0] );
	MatrixTranspose( model, model );
	m_pShaderAPI->GetMatrix( MATERIAL_VIEW, view.m[0] );
	MatrixTranspose( view, view );

	MatrixMultiply( view, model, modelView );
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, modelView.m[0], 3 );
}

//-----------------------------------------------------------------------------
// Loads a scale/offset version of the viewport transform into the specified constant.
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadViewportTransformScaledIntoVertexShaderConstant( int vertexReg )
{
	ShaderViewport_t viewport;

	m_pShaderAPI->GetViewports( &viewport, 1 );

	int bbWidth = 0, 
		bbHeight = 0;

	m_pShaderAPI->GetBackBufferDimensions( bbWidth, bbHeight );

	// (x, y, z, w) = (Width / bbWidth, Height / bbHeight, MinX / bbWidth, MinY / bbHeight)
	Vector4D viewportTransform( 
		1.0f * viewport.m_nWidth / bbWidth,
		1.0f * viewport.m_nHeight / bbHeight,
		1.0f * viewport.m_nTopLeftX / bbWidth, 
		1.0f * viewport.m_nTopLeftY / bbHeight
	);

	m_pShaderAPI->SetVertexShaderConstant( vertexReg, viewportTransform.Base() );
}

//-----------------------------------------------------------------------------
// Loads bump lightmap coordinates into the pixel shader
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadBumpLightmapCoordinateAxes_PixelShader( int pixelReg )
{
	Vector4D basis[3];
	for (int i = 0; i < 3; ++i)
	{
		memcpy( &basis[i], &g_localBumpBasis[i], 3 * sizeof(float) );
		basis[i][3] = 0.0f;
	}
	m_pShaderAPI->SetPixelShaderConstant(pixelReg, (float*)basis, 3);
}


//-----------------------------------------------------------------------------
// Loads bump lightmap coordinates into the pixel shader
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadBumpLightmapCoordinateAxes_VertexShader( int vertexReg )
{
	Vector4D basis[3];

	// transpose
	int i;
	for (i = 0; i < 3; ++i)
	{
		basis[i][0] = g_localBumpBasis[0][i];
		basis[i][1] = g_localBumpBasis[1][i];
		basis[i][2] = g_localBumpBasis[2][i];
		basis[i][3] = 0.0f;
	}
	m_pShaderAPI->SetVertexShaderConstant( vertexReg, (float*)basis, 3 );
	for (i = 0; i < 3; ++i)
	{
		memcpy( &basis[i], &g_localBumpBasis[i], 3 * sizeof(float) );
		basis[i][3] = 0.0f;
	}
	m_pShaderAPI->SetVertexShaderConstant( vertexReg + 3, (float*)basis, 3 );
}


//-----------------------------------------------------------------------------
// Helper methods for pixel shader overbrighting
//-----------------------------------------------------------------------------
void CBaseVSShader::EnablePixelShaderOverbright( int reg, bool bEnable, bool bDivideByTwo )
{
	// can't have other overbright values with pixel shaders as it stands.
	float v[4];
	if( bEnable )
	{
		v[0] = v[1] = v[2] = v[3] = bDivideByTwo ? OVERBRIGHT / 2.0f : OVERBRIGHT;
	}
	else
	{
		v[0] = v[1] = v[2] = v[3] = bDivideByTwo ? 1.0f / 2.0f : 1.0f;
	}
	m_pShaderAPI->SetPixelShaderConstant( reg, v, 1 );
}


//-----------------------------------------------------------------------------
// Helper for dealing with modulation
//-----------------------------------------------------------------------------
void CBaseVSShader::SetModulationVertexShaderDynamicState()
{
 	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	m_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	m_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState_LinearColorSpace( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	color[0] = color[0] > 1.0f ? color[0] : GammaToLinear( color[0] );
	color[1] = color[1] > 1.0f ? color[1] : GammaToLinear( color[1] );
	color[2] = color[2] > 1.0f ? color[2] : GammaToLinear( color[2] );

	m_pShaderAPI->SetPixelShaderConstant(modulationVar, color);
}

void CBaseVSShader::SetModulationPixelShaderDynamicState_LinearColorSpace_LinearScale( int modulationVar, float flScale )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	color[0] = ( color[0] > 1.0f ? color[0] : GammaToLinear( color[0] ) ) * flScale;
	color[1] = ( color[1] > 1.0f ? color[1] : GammaToLinear( color[1] ) ) * flScale;
	color[2] = ( color[2] > 1.0f ? color[2] : GammaToLinear( color[2] ) ) * flScale;

	m_pShaderAPI->SetPixelShaderConstant(modulationVar, color);
}

//-----------------------------------------------------------------------------
// Helpers for dealing with envmap tint
//-----------------------------------------------------------------------------
// set alphaVar to -1 to ignore it.
void CBaseVSShader::SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar, int alphaVar, bool bConvertFromGammaToLinear )
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if( g_pConfig->bShowSpecular && mat_fullbright.GetInt() != 2 )
	{
		IMaterialVar* pAlphaVar = NULL;
		if( alphaVar >= 0 )
		{
			pAlphaVar = m_ppParams[alphaVar];
		}
		if( pAlphaVar )
		{
			color[3] = pAlphaVar->GetFloatValue();
		}

		IMaterialVar* pTintVar = m_ppParams[tintVar];

		if( bConvertFromGammaToLinear )
		{
			pTintVar->GetLinearVecValue( color, 3 );
		}
		else
		{
			pTintVar->GetVecValue( color, 3 );
		}
	}
	else
	{
		color[0] = color[1] = color[2] = color[3] = 0.0f;
	}
	m_pShaderAPI->SetPixelShaderConstant(pixelReg, color);
}

void CBaseVSShader::SetAmbientCubeDynamicStateVertexShader( )
{
	m_pShaderAPI->SetVertexShaderStateAmbientLightCube();
}

float CBaseVSShader::GetAmbientLightCubeLuminance( )
{
	return m_pShaderAPI->GetAmbientLightCubeLuminance();
}

//-----------------------------------------------------------------------------
// GR - translucency query
//-----------------------------------------------------------------------------
BlendType_t CBaseVSShader::EvaluateBlendRequirements( int textureVar, bool isBaseTexture,
													  int detailTextureVar )
{
	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha (for blending or alpha test)
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if ( ( detailTextureVar != -1 ) && ( ! isTranslucent ) )
	{
		isTranslucent = TextureIsTranslucent( detailTextureVar, isBaseTexture );
	}

	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{	
		return isTranslucent ? BT_BLENDADD : BT_ADD;	// Additive
	}
	else
	{
		return isTranslucent ? BT_BLEND : BT_NONE;		// Normal blending
	}
}
#endif // !_STATIC_LINKED || STDSHADER_DX8_DLL_EXPORT


// Take 0..1 seed and map to (u, v) coordinate to be used in shadow filter jittering...
void CBaseVSShader::HashShadow2DJitter( const float fJitterSeed, float *fU, float* fV )
{
	const int nTexRes = 32;
	int nSeed = fmod (fJitterSeed, 1.0f) * nTexRes * nTexRes;

	int nRow = nSeed / nTexRes;
	int nCol = nSeed % nTexRes;

	// Div and mod to get an individual texel in the fTexRes x fTexRes grid
	*fU = nRow / (float) nTexRes;	// Row
	*fV = nCol / (float) nTexRes;	// Column
}

void CBaseVSShader::DrawEqualDepthToDestAlpha( void )
{
	bool bActualDrawCall = false;

	//==========================================================================//
	// Static Snapshot of the Shader Settings
	//==========================================================================//
	if(m_pShaderShadow)
	{
		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// Make it CHEAP
		m_pShaderShadow->EnableColorWrites( false );
		m_pShaderShadow->EnableAlphaWrites( true );
		m_pShaderShadow->EnableDepthWrites( false );
		m_pShaderShadow->EnableAlphaTest( false );
		m_pShaderShadow->EnableBlending( false );
		m_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );

		// No Vertex Shader Vertex Format? Odd

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		// Used in the Macros
		IShaderShadow* pShaderShadow = m_pShaderShadow;
		DECLARE_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(TEXCOORDS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(NORMALS, 0);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_SWAY, 0); // Don't have Params to do this with
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, 0);
		SET_STATIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_depthtodestalpha_ps30);
		SET_STATIC_PIXEL_SHADER(lux_depthtodestalpha_ps30);
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(m_pShaderAPI)
	{
		// If we aren't writing Depth, don't do a DrawCall
		bActualDrawCall = m_pShaderAPI->ShouldWriteDepthToDestAlpha();

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		IShaderDynamicAPI* pShaderAPI = m_pShaderAPI;

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_model_simplified_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_depthtodestalpha_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_depthtodestalpha_ps30);
	}

	Draw(bActualDrawCall);
}

void CBaseVSShader::SetFlashLightColorFromState(FlashlightState_t const &state, int nPSRegister, bool bFlashlightNoLambert)
{
	// Old code
	//float flToneMapScale = ( pShaderAPI->GetToneMappingScaleLinear() ).x;
	//float flFlashlightScale = 1.0f / flToneMapScale;

	// Fix to old code to keep flashlight from ever getting brighter than 1.0
	//float flToneMapScale = ( pShaderAPI->GetToneMappingScaleLinear() ).x;
	//if ( flToneMapScale < 1.0f )
	//	flToneMapScale = 1.0f;
	//float flFlashlightScale = 1.0f / flToneMapScale;

	// Force flashlight to 25% bright always
	float flFlashlightScale = 0.25f;

	if ( !g_pHardwareConfig->GetHDREnabled() )
	{
		// Non-HDR path requires 2.0 flashlight
		flFlashlightScale = 2.0f;
	}

	// DX10 requires some hackery due to sRGB/blend ordering change from DX9
	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() )
	{
		flFlashlightScale *= 2.5f; // Magic number that works well on the NVIDIA 8800
	}

	// Generate pixel shader constant
	float const *pFlashlightColor = state.m_Color;
	float4 f4PsConst;
	f4PsConst.x = flFlashlightScale * pFlashlightColor[0];
	f4PsConst.y = flFlashlightScale * pFlashlightColor[1];
	f4PsConst.z = flFlashlightScale * pFlashlightColor[2];

	// This will be added to N.L before saturate to force a 1.0 N.L Term
	// NoLambert means NdL = 1.0f
	f4PsConst.w = bFlashlightNoLambert ? 2.0f : pFlashlightColor[3];

	// Red flashlight for testing
	//vPsConst[0] = 0.5f; vPsConst[1] = 0.0f; vPsConst[2] = 0.0f;

	m_pShaderAPI->SetPixelShaderConstant(nPSRegister, f4PsConst);
}

float CBaseVSShader::ShadowAttenFromState( FlashlightState_t const &state )
{
	// DX10 requires some hackery due to sRGB/blend ordering change from DX9, which makes the shadows too light
	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() )
		return state.m_flShadowAtten * 0.1f; // magic number

	return state.m_flShadowAtten;
}

float CBaseVSShader::ShadowFilterFromState( FlashlightState_t const &state )
{
	// We developed shadow maps at 1024, so we expect the penumbra size to have been tuned relative to that
	return state.m_flShadowFilterSize / state.m_flShadowMapResolution;
}

//==========================================================================//
// LUX ADDITIONS
//==========================================================================//

// These functions are to be called from the Shaders.
void CBaseVSShader::ShaderDebugMessage(const char* pMessage)
{
	if(!g_bSupressShaderWarnings)
	{
		ConColorMsg(Color(255, 191, 0, 255), "%s", CurrentMaterialName());
		ConColorMsg(Color(252, 83, 83, 255), " %s", pMessage);
	}
}

// Gets the Value from the lux_general_gamma ConVar
float CBaseVSShader::GetGammaValue()
{
	return lux_general_gamma.GetFloat();
}

// Gets the Value from the lux_general_luminanceweights ConVars
float3 CBaseVSShader::GetLuminanceWeights()
{
	if (lux_general_luminanceweights.GetBool())
	{
		// Rec. 709 HDTV
		return float3(0.2126f, 0.7152f, 0.0722f);
	}
	else // == 0
	{
		// NTSC Analog Television Standard
		return float3(0.299f, 0.587f, 0.114f);
	}
}

void CBaseVSShader::SetPixelShaderCameraPosition(int nRegister)
{
	float4 f4EyePos = 0.0f;
	m_pShaderAPI->GetWorldSpaceCameraPosition(f4EyePos);
	m_pShaderAPI->SetPixelShaderConstant(nRegister, f4EyePos);
}

// Gets Values from ConVars and sets them to the given Register
// If you need specific Values, manually pack to a float4
void CBaseVSShader::SetLuminanceGammaConstant(int nRegister)
{
	float4 f4LumGamma;
	f4LumGamma.rgb = GetLuminanceWeights();
	f4LumGamma.a = GetGammaValue();
	m_pShaderAPI->SetPixelShaderConstant(nRegister, f4LumGamma);
}

// Reinterpret the LightState to account for the difference between SDK2013SP and MP
bool CBaseVSShader::StaticLightVertex(LightState_t &LightState)
{
	uintptr_t LightStateAddress = reinterpret_cast<uintptr_t>(&LightState);

	return reinterpret_cast<LightState_Universal_t*>(LightStateAddress)->bStaticLightVertex;
}

void CBaseVSShader::SetupFlashlightSamplers()
{
	// Only enable these Samplers when the Flashlight flag is used
	if (HasFlag2(MATERIAL_VAR2_USE_FLASHLIGHT))
	{
		m_pShaderShadow->EnableAlphaWrites(false);
		m_pShaderShadow->EnableDepthWrites(false);
		m_pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);
		m_pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH);
		m_pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);
		m_pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);
		m_pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHTCOOKIE, true);
		m_pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHTCOOKIE, true);

		// We don't want to have Fog with the Flashlight
		// Your Flashlight Shader should already have #define'd NO_FOG
		// Compiler won't optimise out things from the FinalOutput Function,
		// As they come from Constants.
		FogToBlack();
	}
	else
	{
		// We want the usual Fog, this is probably set by default anyways
		DefaultFog();
	}
}

bool CBaseVSShader::SetupFlashlight()
{
	// No Flashlight, no Data.
	if (!m_pShaderAPI->InFlashlightMode())
		return false;

	// This is probably no longer needed
	// We should just be using the one from the flashlightstate instead
	//	BindTexture(SAMPLER_FLASHLIGHTCOOKIE, FLASHLIGHTTEXTURE, FLASHLIGHTTEXTUREFRAME);

	VMatrix xmWorldToTexture;
	ITexture* pProjTexDepthTexture;
	// Ex is the new function that was introduced, the old one doesn't want ITexture*
	FlashlightState_t ProjTexState = m_pShaderAPI->GetFlashlightStateEx(xmWorldToTexture, &pProjTexDepthTexture);

	// Bind flashlight texture ( spotlight )
	BindTexture(SAMPLER_FLASHLIGHTCOOKIE, ProjTexState.m_pSpotlightTexture, ProjTexState.m_nSpotlightTextureFrame);

	// We will return this at the end
	// existance of a depthtexture means we are doing shadows ( shadows can be disabled on the entity )
	// Investigate: Why does this not consider g_pConfig->ShadowDepthTexture()?
	// is g_pConfig->ShadowDepthTexture() linked to the flashlight_depth convar?
	bool bProjTexShadows = ProjTexState.m_bEnableShadows && g_pConfig->ShadowDepthTexture();
	
	// Adapted from Mapbase, I noticed some Decals not having Depth Textures, maybe this will help them?
	// If there is no DepthTexture but Shadows are enabled, find the Texture in the Fallback List!
	if(bProjTexShadows && !pProjTexDepthTexture)
	{
		// Crashed TF2C when I tried it, disabled now
		// ( nFirst was Valid, Index was 0 & it still crashed because pProjTexDepthTexture was invalid )
		#if 0
			const int nDepthTextureIndex = ( ProjTexState.m_nShadowQuality >> 16 ) - 1;
			const int nLast = INT_FLASHLIGHT_DEPTHTEXTURE_FALLBACK_LAST;
			const int nFirst = INT_FLASHLIGHT_DEPTHTEXTURE_FALLBACK_FIRST;

			// Valid Index Range? Interpret ITexture* from the IntRenderingParameter
			if (nDepthTextureIndex >= 0 && (nDepthTextureIndex <= (nLast - nFirst)))
				pProjTexDepthTexture = (ITexture*)m_pShaderAPI->GetIntRenderingParameter(nFirst + nDepthTextureIndex);

			BindTexture(SAMPLER_SHADOWDEPTH, pProjTexDepthTexture, 0);
			BindTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
		#else
			bProjTexShadows = false;
			m_pShaderAPI->BindStandardTexture(SAMPLER_SHADOWDEPTH, TEXTURE_BLACK);
			m_pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_BLACK);
		#endif
	}
	else if(bProjTexShadows && pProjTexDepthTexture)
	{
		BindTexture(SAMPLER_SHADOWDEPTH, pProjTexDepthTexture, 0);
		BindTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
	}
	else
	{
		// Always bind SOMETHING to enabled Samplers!!
		m_pShaderAPI->BindStandardTexture(SAMPLER_SHADOWDEPTH, TEXTURE_BLACK);
		m_pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_BLACK);
	}

	// Shadow filtering constants
	// Need Texel Size for the Shadow Filter
	// Z will be Shadow Attenuation
	float4 f4tweaks = 0.0f;
	if (bProjTexShadows && pProjTexDepthTexture)
	{
		// ShiroDkxtro2: Stock uses ShadowFilterFromState
		// This doesn't consider non 1:1 Depth Textures ( which no one will ever use anyways )
		// To replicate Stock Behaviour here, we have to divide the ShadowFilterSize by the Resolution
		#if 0
			f4tweaks.x = ShadowFilterFromState(ProjTexState);
			f4tweaks.y = f4tweaks.x;
		#else
			f4tweaks.x = ProjTexState.m_flShadowFilterSize / (float)pProjTexDepthTexture->GetActualWidth();
			f4tweaks.y = ProjTexState.m_flShadowFilterSize / (float)pProjTexDepthTexture->GetActualHeight();
		#endif
	}
	f4tweaks.z = ShadowAttenFromState(ProjTexState);

	// .w should be the No Lambert Value

	// Old Tweaks that we no longer need ( not using Noise-based Shadow Filter anymore )
	/*
	HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &f4tweaks.z, &f4tweaks.w);
	*/
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PROJTEX_TWEAKS, f4tweaks);

	// Attenuation Factors
	float4 f4ProjTexAttenuations;
	f4ProjTexAttenuations.x = ProjTexState.m_fConstantAtten;
	f4ProjTexAttenuations.y = ProjTexState.m_fLinearAtten;
	f4ProjTexAttenuations.z = ProjTexState.m_fQuadraticAtten;
	f4ProjTexAttenuations.w = ProjTexState.m_FarZ;
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PROJTEX_ATTEN, f4ProjTexAttenuations);

	// Send Flashlight Tint
	// There is a separate parameter on the Water Shader to boost fog brigthness under the flashlight
	// NOTE: $ProjectedTextureNoLambert is a new Base-Parameter. Previously the NoLambert Feature was entirely unused.
	SetFlashLightColorFromState(ProjTexState, LUX_PS_FLOAT_PROJTEX_COLOR, GetBool(ProjectedTextureNoLambert));

	// The position at which the flashlight is currently located
	float4 f4ProjTexPos = 0.0f;
	f4ProjTexPos.x = ProjTexState.m_vecLightOrigin[0];
	f4ProjTexPos.y = ProjTexState.m_vecLightOrigin[1];
	f4ProjTexPos.z = ProjTexState.m_vecLightOrigin[2];
//	f4pos.w; // .w is still free!
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PROJTEX_POSITION, f4ProjTexPos);

	// Send WorldToShadow Matrix
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_PROJTEX_MATRIX, xmWorldToTexture.Base(), 4);

	return bProjTexShadows;
}

int CBaseVSShader::GetMipCount(const int var)
{
	int nMips = 0;
	ITexture* pTexture = GetTexture(var);

	// Make sure this thing is not nullptr
	if (pTexture)
	{
		// Get power of 2 of texture width
		int nWidth = pTexture->GetMappingWidth();
		
		// NOTE: Textures don't necessarily are pow of 2 !!!!!
		while (nWidth >>= 1)
			++nMips;
	}
	else return 0;

	// Dealing with very high and low resolution textures
	if (nMips > 12)	nMips = 12;
	if (nMips < 4)	nMips = 4;

	return nMips;
}

float4 CBaseVSShader::ComputeTint(const bool bAllowDiffuseModulation, const int var_Alpha)
{
	// This is mostly been ported and adapted from BaseShader.cpp
	// The old (CBaseShader) Function is kept, for backwards compatability with pre-LUX Shaders
	// Note that this Function does not use GetAlpha(), GetAlpha() is used for Modulation.w

	// Prepare a container
	float4 f4Result = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (!m_ppParams)
		return f4Result;

	// Alpha can be filled with just about any float Parameter
	f4Result.w = GetFloat(var_Alpha);

	// We want both $Color and $Color2 ( RenderColor )
	f4Result.xyz = GetFloat3(Color1);

	if (bAllowDiffuseModulation)
	{
		f4Result.xyz *= GetFloat3(Color2);
	}

	// This is where $sRGBTint comes in
	if (g_pHardwareConfig->UsesSRGBCorrectBlending())
	{
		float3 f3sRGBTint = GetFloat3(sRGBTint);
		f4Result.xyz *= f3sRGBTint;
	}

	// ShiroDkxtro2: bShowDiffuse can be set using the mat_diffuse ConVar.
	if (!g_pConfig->bShowDiffuse)
		f4Result.xyz = 0.0f;

	// Shouldn't this happen before bShowDiffuse?
	// This will cancel bShowDiffuse...
	if (mat_fullbright.GetInt() == 2)
		f4Result.xyz = 1.0f;

	return f4Result;
}

// The Three Bump-Basis's used for Radiosity Normal Mapping
// See also :
// https://advances.realtimerendering.com/s2006/Mitchell-ShadingInValvesSourceEngine.pdf
// http://www.decew.net/OSS/References/sem_ss06_07-Independant%20Explanation.pdf 
// NOTE: The Image used in the Valve presentation has a possible typo
// For Specifics see lux_common_lightmapped.h
static const float3 BumpBasis1 = { sqrtf(3.0f/2.0f), 0.0f, 1.0f / sqrtf(3.0f) };
static const float3 BumpBasis2 = { -(1.0f / sqrtf(6.0f)), 1.0f / sqrtf(2.0f), 1.0f / sqrtf(3.0f) };
static const float3 BumpBasis3 = { -(1.0f / sqrtf(6.0f)), -(1.0f / sqrtf(2.0f)), 1.0f / sqrtf(3.0f) };

// Fixed Values :
//  0.816496,  0,		 0.577350
// -0.408248,  0.707107, 0.577350
// -0.408248, -0.707107, 0.577350

inline static const float SSBumpCoefficient()
{
	// ShiroDkxtro2 :
	// $SSBumpMathFix is a Parameter found in various Branches of the Source Engine
	// Here is some Statistic on that. Credits for collecting this Information goes to Mr.Kleiner
	//
	// +----------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
	// |                |  hl2  |  ep1  |  ep2  |  asw  |  bms  | csgo  | l4d2  |  p2   |  tf2  |
	// +----------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
	// | Searched VMT's |  5083 |  475  |  1389 |  3780 |  7876 | 15201 |  4645 |  3431 | 20995 |
	// +----------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
	// | Occurences     |   0   |   0   |   0   |   5   |   61  |   37  |   62  |   21  |   0   |
	// +----------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
	//
	// NOTE: The overwhelming Majority of $SSBumpMathFix's in CS:GO and BMS are commented.
	//		 The Parameter doesn't appear to do anything in CS:GO and SSBumps appear fine.
	//		 ( Implying this has been implemented on a Shader Level )
	//		 Note that this breaks SSBumps generated by Software like xNormal ( they don't need the fix )
	//
	// The Parameter's Name and Existence imply incorrect Mathematics, and that it can retroactively be fixed.
	// The SSBump Shader-Code is very simple. The only Difference to regular Normal Maps,
	// is the lack of Adjustment for the Bump Basis. It is already considered in the Texture.
	// In Theory, Bumped Lightmaps can be precomputed using the Normal Map and the 3 Lightmaps.
	// The resulting Lightmap needs to have the same Resolution as the original Normal Map..
	// And you cannot retroactively change the Normal Map.. But I digress!
	//
	// The Shader Code boils down to This. Very simple.
	// lighting = (ssbump.xxx * lightmap1) + (ssbump.yyy * lightmap2) + (ssbump.zzz * lightmap3)
	// lighting *= lightmapscalefactor
	// 
	// The Lightmaps and Scale Factor are 100% correct. Otherwise the same Issue would apply to regular Normal Maps
	// The Issue is with the SSBump itself
	//
	// Using the Parameter in Left 4 Dead 2, what it does is decrease the Brightness of the Result.
	// The decrease in Brightness is linear, meaning we are dealing with a Multiplier.
	// 
	// The Stock Shader combines the LightmapScaleFactor with the $Color and $Color2 Values
	// So I set $SSBumpMathFix to 1, then increased $Color until the Results were the identical.
	// ( Without Tonemapping and Bloom of course )
	// To get back to the original Brightness we have to apply a Value >=1.6 ( Meaning Lightmap * Fix * 1.6f )
	// I also did the inverse, set $SSBumpMathFix to 0 and tried to eyeball the Result with $Color.
	// A $Color Value of around 0.57 - 0.59 gives the same Results as $SSBumpMathFix.
	//
	// The Multiplier is pretty much identical to the sqrt(3) in the Bump Basis
	// In fact, applying it as $Color gives a result indistinguishable with $SSBumpMathFix.
	// This means other Source Branches, not using LUX, can apply $SSBumpMathFix using $Color instead.
	// Coincidentally, the sum of all z's of the Basis is 1.73205f
	// Which is in the exact Range of Values previously estimated to nullify $SSBumpMathFix
	// dot of the Basis .x and .y Values Results in 0.0f.
	// This isn't the case with .z, which causes the contribution to be too high per Lightmap. 
	// So we can undo this by accounting for the Overcontribution with a multiplier of 1.0f / 1.73205f

	float f1BasisOne	= BumpBasis1.x + BumpBasis1.y + BumpBasis1.z; //  0.816496f +  0.0f      + 0.577350f =  1.393846
	float f1BasisTwo	= BumpBasis2.x + BumpBasis2.y + BumpBasis2.z; // -0.408248f +  0.707107f + 0.577350f =  0.876209
	float f1BasisThree	= BumpBasis3.x + BumpBasis3.y + BumpBasis3.z; // -0.408248f + -0.707107f + 0.577350f = -0.538005

	// Should come out to..
	// 1.393846f + 0.876209f + -0.538005f = 1.73205f
	// 1.0f / 1.73205f = 0.57735f
	return 1.0f / (f1BasisOne + f1BasisTwo + f1BasisThree);
}

float4 CBaseVSShader::GetModulationConstant(const bool bBrush, const bool bSSBumpMathFix)
{
	float4 Result = 0.0f;

	if (HasFlag(MATERIAL_VAR_NOALPHAMOD))
	{
		Result.x = 1.0f;
	}
	else
	{
		float f1Alpha1 = GetFloat(Alpha);
		float f1Alpha2 = GetFloat(Alpha2);
		Result.x = clamp(f1Alpha1 * f1Alpha2, 0.0f, 1.0f); // Clamp this to 0-1 Range
	}

	// LightmapScale Factor is only used on brushes
	// Models won't need it even if they will support Bumped Lightmaps
	if (bBrush)
	{
		Result.y = m_pShaderAPI->GetLightMapScaleFactor();

		if(bSSBumpMathFix)
			Result.y *= SSBumpCoefficient();
	}	
	else
		Result.y = 1.0f;

	// Force Alien Swarm Fog Factor if desired.
	// g_bWaterAlienSwarmFogFactor will be indicated by  the Water Material itself when it renders in the Scene
	int nFogFactorType = lux_water_forcefogtype.GetInt();
	if(nFogFactorType == 1)
		Result.z = 0.0f;
	else if (nFogFactorType >= 2)
		Result.z = 1.0f;
	else
		Result.z = float(g_bWaterAlienSwarmFogFactor);

	return Result;
}

void CBaseVSShader::SetModulationConstant(const bool bSSBumpMathFix, const bool bBrush)
{
	float4 f4Modulation = GetModulationConstant(bBrush, bSSBumpMathFix);

	// Our LightmapScaleFactor doesn't include Color like the Stock one does
	// Doing so would interfer with Features like BlendTintbyBaseAlpha
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_MODULATIONCONSTANTS, f4Modulation);
}

void CBaseVSShader::SetupDefaultRegisters(const bool bFog,
	const bool bEyePos, const int var_Alpha, const bool bSSBumpMathFix)
{
	// Alpha unfortunately Empty
	if (bEyePos)
	{
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);
	}

	if (bFog)
	{
		m_pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);
	}

	// $BaseTexture Tint
	// This is sent to c32, and c1 will have the LightmapScaleFactor
	float4 f4Tint = ComputeTint(!GetBool(NoTint) && GetBool(AllowDiffuseModulation), var_Alpha);
	m_pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_DEFAULTCONTROLS, f4Tint);

	// Function above, handles LightmapScaleFactor and Alpha Modulation
	SetModulationConstant(bSSBumpMathFix);
}

bool CBaseVSShader::WriteDepthToDestAlpha(const bool bIsOpaque)
{
	// Don't write anything to alpha if transparent
	if (!bIsOpaque)
		return false;
	else
		return m_pShaderAPI->ShouldWriteDepthToDestAlpha();
}

bool CBaseVSShader::WriteWaterFogToDestAlpha(const bool bIsOpaque)
{
	// Don't write anything to alpha if transparent
	if (!bIsOpaque)
		return false;
	else
		return (m_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
}

bool CBaseVSShader::HasFlashlight()
{
	if (IsSnapshotting())
	{
		return HasFlag2(MATERIAL_VAR2_USE_FLASHLIGHT);
	}
	else
	{
		return m_pShaderAPI->InFlashlightMode();
	}
}

int CBaseVSShader::GetDesiredShadowFilter()
{
	// Allow for overriding the flashlight filter,
	// You may set this to any filter you like, or implement new filters
	// By Default :
	// 0 NVIDIA_PCF_POISSON	0
	// 1 ATI_NOPCF			1
	// 2 ATI_NO_PCF_FETCH4	2
#ifdef ALLOW_FLASHLIGHT_SHADOWFILTER_OVERRIDE
	int ShadowFilter = lux_force_flashlight_filter.GetInt();
	if (ShadowFilter > 0  ShadowFilter < 4)
	{
		return (ShadowFilter - 1);
	}
	else
#endif
	{
		return g_pHardwareConfig->GetShadowFilterMode();
	}
}

bool CBaseVSShader::HasTransform(const bool bTexture, const int var)
{
	// Can't check if the Transform was defined, also can't know if it's in a proxy
	// Just have to check if it's not Identity.
	// If you use this in Snapshot State, better force your Transforms because whether the Matrix is Identity could change due to a Proxy.
	// ( Different Story for Command Buffer !! )
	if (bTexture)
		return !m_ppParams[var]->MatrixIsIdentity();
	else
		return false;
}

bool CBaseVSShader::HasRadialFog()
{
#ifndef RADIALFOG
	return false;
#else
	bool bRadialFog = lux_general_radialfog.GetBool();

	#ifdef TF2SDK
		// The Mame of this function is pretty lazy
		// SDK Shaders <ps20b apparently just call GetPixelFogCombo anyways
		// So the boolean is pointless..
		// I don't like this function, they could have done better but we are forced to use it now
		// Anyways, it returns 2 for Radial Fog!
		return bRadialFog && (m_pShaderAPI->GetPixelFogCombo1(true) == 2);
	#else
		return bRadialFog;
	#endif
#endif // RADIALFOG
}

float4 CBaseVSShader::PrecomputeDetail(const float4 &f4Tint_Factor, const int nBlendMode)
{
	float4 f4Result = f4Tint_Factor; // Copy
	switch (nBlendMode)
	{
	case DETAILBLENDMODE_MOD2X:
	{
		f4Result.xyz *= 2.0f; // Precompute the 2.0f* of mod2x
		break;
	}

	case DETAILBLENDMODE_ADDITIVE:
	{
		f4Result.xyz *= f4Tint_Factor.w; // Precompute the blendfactor into the tint
		break;
	}

	case DETAILBLENDMODE_LERP_BY_DETAILALPHA:
		break; // Nothing to do here

	case DETAILBLENDMODE_LERP_BY_BLENDFACTOR:
		break; // Nothing to do here

	case DETAILBLENDMODE_LERP_BY_INVBASEALPHA:
		break; // Nothing to do here

	case DETAILBLENDMODE_SELFILLUM_ADDITIVE:
	{
		f4Result.xyz *= f4Tint_Factor.w; // Precompute the blendfactor into the tint
		break;
	}

	case DETAILBLENDMODE_SELFILLUM_THRESHOLDFADE:
	{
		float factor = f4Tint_Factor.w - 0.5f;
		float fMult = (factor >= 0.0f) ? 1.0f / f4Tint_Factor.w : 4.0f * f4Tint_Factor.w;
		float fadd = (factor >= 0.0f) ? 1.0f - fMult : -0.5f * fMult;

		f4Result.xyz *= fMult; // Precompute fMult into the $detailtint
		f4Result.w = fadd; // Now reuse blendfactor as add-factor
		break;
	}

	case DETAILBLENDMODE_MOD2X_TWOPATTERNS:
		break; // Nothing to do here, because the alpha doesn't receive a tint we can't precompute it

	case DETAILBLENDMODE_MULTIPLY:
		break; // Nothing to do here

	case DETAILBLENDMODE_MULTIPLY_ALPHA:
		break; // Nothing to do here

	case DETAILBLENDMODE_SSBUMP_MODULATE:
	{
		f4Result.xyz *= 2.0f; // Precompute the 2.0f* used on the ssbump
		break;
	}

	case DETAILBLENDMODE_SSBUMP_AO:
		break; // Nothing to do here

	default: // Do nothing
		break;
	}

	return f4Result;
}

// This function was initially created by InevitablyDivinity
float2 CBaseVSShader::GetCurrentRenderTargetSize() const
{
	static float2 s_f2CachedRenderTargetSize;
	static ITexture *s_pCachedRenderTarget = nullptr;

	ITexture *pRenderTarget = m_pShaderAPI->GetRenderTargetEx(0);

	// We usually cannot access the ITexture* of the Framebuffer during rendering.
	// Meaning if this IS valid we are rendering to something that ISN'T the main FB
	// We can't get the Resolution of a nullpointer, so its backbuffer size then.
	if (pRenderTarget != s_pCachedRenderTarget)
	{
		// We cache the rendertarget so we don't constantly re-evaluate the same resolution on every Shader.
		s_pCachedRenderTarget = pRenderTarget;

		// Use RT Res. if it's valid, otherwise BackBuffer Res.
		int nWidth = 0;
		int nHeight = 0;
		if (pRenderTarget)
		{
			nWidth = pRenderTarget->GetActualWidth();
			nHeight = pRenderTarget->GetActualHeight();
		}
		else
			m_pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);

		s_f2CachedRenderTargetSize.x = (float)nWidth;
		s_f2CachedRenderTargetSize.y = (float)nHeight;
	}

	return s_f2CachedRenderTargetSize;
}

BlendType_t CBaseVSShader::ComputeBlendType(int nBaseTextureVar, bool bIsBaseTexture, int nDetailTextureVar, int nDetailBlendMode)
{
	int nDetailVar = -1;
	if (nDetailTextureVar != -1)
	{
		// 11.06.2025: Turns out these specific Blendmodes allow the Alpha Channel to be Overriden
		// Making it possible to write to alpha even with $SelfIllum, $BlendTintByBaseAlpha, etc.
		if ((nDetailBlendMode == DETAILBLENDMODE_LERP_BY_BLENDFACTOR) || (nDetailBlendMode == DETAILBLENDMODE_MULTIPLY) || (nDetailBlendMode == DETAILBLENDMODE_MULTIPLY_ALPHA))
			nDetailVar = nDetailTextureVar;
	}
	
	// Determine and set BlendingShadowState
	BlendType_t nBlendType = EvaluateBlendRequirements(nBaseTextureVar, bIsBaseTexture, nDetailVar);

	// When using the Flashlight, override to $Additive
	if (GetBool(ReceiveProjectedTextures) && HasFlag2(MATERIAL_VAR2_USE_FLASHLIGHT))
	{
		// If we are using translucency already, we need to blend with additive Lighting
		// This is very important for Decals, since using additive Lighting causes
		// the Area around Translucent Decals to become an opaque White.
		if (nBlendType == BT_BLEND)
		{
			nBlendType = BT_BLENDADD;
		}
		else
		{
			nBlendType = BT_ADD;
		}
	}

	return nBlendType;
}

void CBaseVSShader::EnableTransparency(BlendType_t nBlendType)
{
	// Can use this existing Function to enable Blending
	SetBlendingShadowState(nBlendType);

	// Handle Alphatesting
	// Should we check against nBlendType to make sure we don't try to use multiple transparency methods?
	if (HasFlag(MATERIAL_VAR_ALPHATEST))
	{
		m_pShaderShadow->EnableAlphaTest(true); // bAlphatest
		if (GetFloat(AlphaTestReference) > 0.0f) // 0 is default.
		{
			m_pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, GetFloat(AlphaTestReference));
		}
	}
}

bool CBaseVSShader::IsFullyOpaque(BlendType_t nBlendType)
{
	// This is better than checking for all other 3 BT Modes..
	// BT_ADD is Flashlight so we got that covered here too. ( As long as you use the ComputeBlendType Function )
	return (nBlendType == BT_NONE) && !HasFlag(MATERIAL_VAR_ALPHATEST);
}